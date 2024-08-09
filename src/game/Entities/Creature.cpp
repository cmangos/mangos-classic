/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Entities/Creature.h"
#include "Database/DatabaseEnv.h"
#include "Server/WorldPacket.h"
#include "World/World.h"
#include "Globals/ObjectMgr.h"
#include "Globals/ObjectAccessor.h"
#include "Entities/ObjectGuid.h"
#include "Server/SQLStorages.h"
#include "Spells/SpellMgr.h"
#include "Entities/GossipDef.h"
#include "Entities/Player.h"
#include "GameEvents/GameEventMgr.h"
#include "Pools/PoolManager.h"
#include "Server/Opcodes.h"
#include "Log/Log.h"
#include "Loot/LootMgr.h"
#include "Maps/MapManager.h"
#include "AI/BaseAI/CreatureAI.h"
#include "AI/CreatureAISelector.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"
#include "Maps/InstanceData.h"
#include "Maps/MapPersistentStateMgr.h"
#include "BattleGround/BattleGroundMgr.h"
#include "OutdoorPvP/OutdoorPvP.h"
#include "Spells/Spell.h"
#include "Util/Util.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "Movement/MoveSplineInit.h"
#include "Entities/CreatureLinkingMgr.h"
#include "Maps/SpawnManager.h"

// apply implementation of the singletons
#include "Policies/Singleton.h"

TrainerSpell const* TrainerSpellData::Find(uint32 spell_id) const
{
    TrainerSpellMap::const_iterator itr = spellList.find(spell_id);
    if (itr != spellList.end())
        return &itr->second;

    return nullptr;
}

bool VendorItemData::RemoveItem(uint32 item_id)
{
    for (VendorItemList::iterator i = m_items.begin(); i != m_items.end(); ++i)
    {
        if ((*i)->item == item_id)
        {
            m_items.erase(i);
            return true;
        }
    }
    return false;
}

size_t VendorItemData::FindItemSlot(uint32 item_id) const
{
    for (size_t i = 0; i < m_items.size(); ++i)
        if (m_items[i]->item == item_id)
            return i;
    return m_items.size();
}

VendorItem const* VendorItemData::FindItem(uint32 item_id) const
{
    for (auto m_item : m_items)
    {
        // Skip checking for conditions, condition system is powerfull enough to not require additional entries only for the conditions
        if (m_item->item == item_id)
            return m_item;
    }
    return nullptr;
}

bool ForcedDespawnDelayEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    if (m_onlyAlive && !m_owner.IsAlive())
        return true; // still successful, just not executed

    m_owner.ForcedDespawn();
    return true;
}

void CreatureCreatePos::SelectFinalPoint(Creature* cr, bool staticSpawn)
{
    // if object provided then selected point at specific dist/angle from object forward look
    if (m_closeObject)
    {
        if (m_dist == 0.0f)
        {
            m_pos.x = m_closeObject->GetPositionX();
            m_pos.y = m_closeObject->GetPositionY();
            m_pos.z = m_closeObject->GetPositionZ();
        }
        else
            m_closeObject->GetClosePoint(m_pos.x, m_pos.y, m_pos.z, cr->GetObjectBoundingRadius(), m_dist, m_angle);
    }
    else if (!staticSpawn)
        cr->UpdateAllowedPositionZ(m_pos.x, m_pos.y, m_pos.z);
}

bool CreatureCreatePos::Relocate(Creature* cr) const
{
    cr->Relocate(m_pos.x, m_pos.y, m_pos.z, m_pos.o);

    if (!cr->IsPositionValid())
    {
        sLog.outError("%s not created. Suggested coordinates isn't valid (X: %f Y: %f)", cr->GetGuidStr().c_str(), cr->GetPositionX(), cr->GetPositionY());
        return false;
    }

    return true;
}

Creature::Creature(CreatureSubtype subtype) : Unit(),
    m_gossipMenuId(0), m_lootMoney(0), m_lootGroupRecipientId(0),
    m_lootStatus(CREATURE_LOOT_STATUS_NONE),
    m_corpseAccelerationDecayDelay(MINIMUM_LOOTING_TIME),
    m_respawnTime(0), m_respawnDelay(25), m_respawnOverriden(false), m_respawnOverrideOnce(false), m_corpseDelay(60), m_canAggro(false),
    m_respawnradius(5.0f), m_checkForHelp(true), m_interactionPauseTimer(0), m_subtype(subtype), m_defaultMovementType(IDLE_MOTION_TYPE),
    m_equipmentId(0), m_detectionRange(20.f), m_AlreadyCallAssistance(false), m_canCallForAssistance(true),
    m_temporaryFactionFlags(TEMPFACTION_NONE),
    m_originalEntry(0), m_gameEventVendorId(0),
    m_immunitySet(UINT32_MAX), m_ai(nullptr),
    m_isInvisible(false), m_ignoreMMAP(false), m_forceAttackingCapability(false),
    m_settings(this),
    m_countSpawns(false),
    m_creatureGroup(nullptr), m_imposedCooldown(false),
    m_creatureInfo(nullptr), m_mountInfo(nullptr)
{
    m_valuesCount = UNIT_END;

    SetWalk(true, true);
}

Creature::~Creature()
{
    CleanupsBeforeDelete();
}

void Creature::CleanupsBeforeDelete()
{

    if (GetCreatureGroup() && GetCreatureGroup()->GetFormationData())
        GetCreatureGroup()->GetFormationData()->OnDelete(this);

    Unit::CleanupsBeforeDelete();
    m_vendorItemCounts.clear();
}

void Creature::AddToWorld()
{
    ///- Register the creature for guid lookup
    if (!IsInWorld())
    {
        if (IsUnit())
            GetMap()->GetObjectsStore().insert<Creature>(GetObjectGuid(), (Creature*)this);
        if (GetDbGuid())
            GetMap()->AddDbGuidObject(this);
    }

    switch (GetSubtype())
    {
        case CREATURE_SUBTYPE_PET:
        case CREATURE_SUBTYPE_TEMPORARY_SUMMON:
        {
            std::map<uint32, uint32>& targetArray = GetMap()->GetTempCreatures();
            if (GetSubtype() == CREATURE_SUBTYPE_PET)
                targetArray = GetMap()->GetTempPets();
            // If creature exists, add count
            if (targetArray.find(this->GetEntry()) != targetArray.end())
                ++targetArray[this->GetEntry()];
            else
                targetArray.insert(std::pair<uint32, uint32>(this->GetEntry(), 1));
            break;
        }
        default: break;
    }

    Unit::AddToWorld();

    // Make active if required
    if (sWorld.isForceLoadMap(GetMapId()) || (GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_ACTIVE))
        SetActiveObjectState(true);

    if (m_countSpawns)
        GetMap()->AddToSpawnCount(GetObjectGuid());
}

void Creature::RemoveFromWorld()
{
    ///- Remove the creature from the accessor
    if (IsInWorld())
    {
        if (IsUnit())
            GetMap()->GetObjectsStore().erase<Creature>(GetObjectGuid(), (Creature*)nullptr);
        if (GetDbGuid())
            GetMap()->RemoveDbGuidObject(this);

        switch (GetSubtype())
        {
            case CREATURE_SUBTYPE_PET:
            case CREATURE_SUBTYPE_TEMPORARY_SUMMON:
            {
                std::map<uint32, uint32>& targetArray = GetMap()->GetTempCreatures();
                if (GetSubtype() == CREATURE_SUBTYPE_PET)
                    targetArray = GetMap()->GetTempPets();
                if (targetArray.find(this->GetEntry()) != targetArray.end())
                {
                    --targetArray[this->GetEntry()];

                    if (targetArray[this->GetEntry()] <= 0)
                        targetArray.erase(this->GetEntry());
                }
                break;
            }
            default: break;
        }

        if (m_countSpawns)
            GetMap()->RemoveFromSpawnCount(GetObjectGuid());

        if (uint32 spellId = GetCreatedBySpellId())
            if (Unit* spawner = GetSpawner())
                spawner->RemoveCreature(spellId, false);

        if (GetUInt32Value(UNIT_CREATED_BY_SPELL)) // optimization
            if (Unit* owner = GetOwner())
                StartCooldown(owner);

        ClearCreatureGroup();

        if (AI())
            AI()->RelinquishFollow(ObjectGuid());
    }

    Unit::RemoveFromWorld();
}

void Creature::RemoveCorpse(bool inPlace)
{
    if (!inPlace && !IsInWorld())
       return;

    if ((GetDeathState() != CORPSE))
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "Removing corpse of %s ", GetGuidStr().c_str());

    m_corpseExpirationTime = TimePoint();
    SetDeathState(DEAD);
    UpdateObjectVisibility();

    delete m_loot;
    m_loot = nullptr;
    m_lootStatus = CREATURE_LOOT_STATUS_NONE;
    uint32 respawnDelay = 0;

    if (AI())
        AI()->CorpseRemoved(respawnDelay);

    if (m_isCreatureLinkingTrigger)
        GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_DESPAWN, this);

    if (InstanceData* mapInstance = GetInstanceData())
        mapInstance->OnCreatureDespawn(this);

    // script can set time (in seconds) explicit, override the original
    if (respawnDelay)
    {
        m_respawnTime = time(nullptr) + respawnDelay; // if we set a custom respawn time, we need to save it as well

        // always save boss respawn time at death to prevent crash cheating
        if (sWorld.getConfig(CONFIG_BOOL_SAVE_RESPAWN_TIME_IMMEDIATELY) || IsWorldBoss())
            SaveRespawnTime();
    }

    InterruptMoving();

    float x, y, z, o;
    GetRespawnCoord(x, y, z, &o);
    GetMap()->CreatureRelocation(this, x, y, z, o);

    // forced recreate creature object at clients
    UnitVisibility currentVis = GetVisibility();
    SetVisibility(VISIBILITY_REMOVE_CORPSE);
    UpdateObjectVisibility();
    SetVisibility(currentVis);                              // restore visibility state
    UpdateObjectVisibility();

    if (IsUsingNewSpawningSystem())
        AddObjectToRemoveList();
}

/**
 * change the entry of creature until respawn
 */
bool Creature::InitEntry(uint32 Entry, CreatureData const* data /*=nullptr*/, GameEventCreatureData const* eventData /*=nullptr*/)
{
    // use game event entry if any instead default suggested
    if (eventData && eventData->entry_id)
        Entry = eventData->entry_id;

    CreatureInfo const* normalInfo = ObjectMgr::GetCreatureTemplate(Entry);
    if (!normalInfo)
    {
        sLog.outErrorDb("Creature::UpdateEntry creature entry %u does not exist.", Entry);
        return false;
    }

    CreatureInfo const* cinfo = normalInfo;

    SetEntry(Entry);                                        // normal entry always
    m_creatureInfo = cinfo;                                 // map mode related always

    SetObjectScale(cinfo->Scale);

    // equal to player Race field, but creature does not have race
    SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_RACE, 0);

    // known valid are: CLASS_WARRIOR,CLASS_PALADIN,CLASS_ROGUE,CLASS_MAGE
    SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_CLASS, uint8(cinfo->UnitClass));

    uint32 display_id = ChooseDisplayId(GetCreatureInfo(), data, eventData);
    if (!display_id)                                        // Cancel load if no display id
    {
        sLog.outErrorDb("Creature (Entry: %u) has no model defined in table `creature_template`, can't load.", Entry);
        return false;
    }

    CreatureModelInfo const* minfo = sObjectMgr.GetCreatureModelRandomGender(display_id);
    if (!minfo)                                             // Cancel load if no model defined
    {
        sLog.outErrorDb("Creature (Entry: %u) has no model info defined in table `creature_model_info`, can't load.", Entry);
        return false;
    }

    display_id = minfo->modelid;                            // it can be different (for another gender)

    SetNativeDisplayId(display_id);

    // normally the same as native, but some has exceptions (Spell::DoSummonTotem)
    // also recalculates speed since speed is based on Model and/or template
    SetDisplayId(display_id);

    SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_GENDER, minfo->gender);

    // set PowerType based on unit class
    switch (cinfo->UnitClass)
    {
        case CLASS_WARRIOR:
            SetPowerType(POWER_RAGE);
            break;
        case CLASS_PALADIN:
        case CLASS_MAGE:
            SetPowerType(POWER_MANA);
            break;
        case CLASS_ROGUE:
            SetPowerType(POWER_ENERGY);
            break;
        default:
            sLog.outErrorDb("Creature (Entry: %u) has unhandled unit class. Power type will not be set!", Entry);
            break;
    }

    // Load creature equipment
    if (eventData)
    {
        if (eventData->equipment_id)
            LoadEquipment(eventData->equipment_id);             // use event equipment if any for active event
        else if (eventData->entry_id)
            LoadEquipment(cinfo->EquipmentTemplateId, true);     // use changed entry default template
    }
    else if (!data || (data->spawnTemplate->equipmentId == -1))
    {
        if (cinfo->EquipmentTemplateId == 0)
            LoadEquipment(normalInfo->EquipmentTemplateId, true); // use default from normal template if diff does not have any
        else
            LoadEquipment(cinfo->EquipmentTemplateId); // else use from diff template

        if (GetSettings().HasFlag(CreatureStaticFlags::CAN_WIELD_LOOT)) // override from loot if any
        {
            PrepareBodyLootState(nullptr);
            if (m_loot != nullptr)
            {
                auto [mh, oh, ranged] = m_loot->GetQualifiedWeapons();

                if (mh != 0)
                    SetVirtualItem(VIRTUAL_ITEM_SLOT_0, mh);

                if (oh != 0)
                    SetVirtualItem(VIRTUAL_ITEM_SLOT_1, oh);

                if (ranged != 0)
                    SetVirtualItem(VIRTUAL_ITEM_SLOT_2, ranged);
            }            
        }
    }
    else if (data)
    {
        // override, 0 means no equipment
        if (data->spawnTemplate->equipmentId != -1)
        {
            if (data->spawnTemplate->equipmentId != 0)
                LoadEquipment(data->spawnTemplate->equipmentId);
        }
    }

    SetName(normalInfo->Name);                              // at normal entry always

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    if (!IsPet() || !static_cast<Pet*>(this)->isControlled())
        SetLevitate((cinfo->InhabitType & INHABIT_AIR) != 0); // TODO: may not be correct to send opcode at this point (already handled by UPDATE_OBJECT createObject)

    // check if we need to add swimming movement. TODO: i thing movement flags should be computed automatically at each movement of creature so we need a sort of UpdateMovementFlags() method
    if (cinfo->InhabitType & INHABIT_WATER &&               // check inhabit type water
            !(cinfo->ExtraFlags & CREATURE_EXTRA_FLAG_WALK_IN_WATER) &&  // check if creature is forced to walk (crabs, giant,...)
            GetMap()->GetTerrain()->IsSwimmable(m_respawnPos.x, m_respawnPos.y, m_respawnPos.z, GetCollisionHeight()))  // check if creature is in water and have enough space to swim
        m_movementInfo.AddMovementFlag(MOVEFLAG_SWIMMING);  // add swimming movement

    // checked at loading
    if (data)
    {
        if (data->spawnTemplate->IsRunning())
            SetWalk(false);
        if (data->spawnTemplate->IsHovering())
            SetHover(true);
        m_defaultMovementType = MovementGeneratorType(data->movementType);
    }
    else
        m_defaultMovementType = MovementGeneratorType(cinfo->MovementType);

    SetMeleeDamageSchool(SpellSchools(cinfo->DamageSchool));

    SetCanParry(!(cinfo->ExtraFlags & CREATURE_EXTRA_FLAG_NO_PARRY));
    SetCanBlock(!(cinfo->ExtraFlags & CREATURE_EXTRA_FLAG_NO_BLOCK));
    SetCanDualWield((cinfo->ExtraFlags & CREATURE_EXTRA_FLAG_DUAL_WIELD_FORCED));
    SetForceAttackingCapability((cinfo->ExtraFlags & CREATURE_EXTRA_FLAG_FORCE_ATTACKING_CAPABILITY) != 0);
    SetCanCallForAssistance((cinfo->ExtraFlags & CREATURE_EXTRA_FLAG_NO_CALL_ASSIST) == 0);
    SetNoReputation(false);

    SetDetectionRange(cinfo->Detection);

    if (cinfo->CorpseDelay)
        SetCorpseDelay(cinfo->CorpseDelay);
    else if(sObjectMgr.IsEncounter(GetEntry(), GetMapId()))
    {
        // encounter boss forced decay timer to 1h
        m_corpseDelay = 3600;                               // TODO: maybe add that to config file
    }
    else
    {
        switch (cinfo->Rank)
        {
            case CREATURE_ELITE_RARE:
                m_corpseDelay = sWorld.getConfig(CONFIG_UINT32_CORPSE_DECAY_RARE);
                break;
            case CREATURE_ELITE_ELITE:
                m_corpseDelay = sWorld.getConfig(CONFIG_UINT32_CORPSE_DECAY_ELITE);
                break;
            case CREATURE_ELITE_RAREELITE:
                m_corpseDelay = sWorld.getConfig(CONFIG_UINT32_CORPSE_DECAY_RAREELITE);
                break;
            case CREATURE_ELITE_WORLDBOSS:
                m_corpseDelay = sWorld.getConfig(CONFIG_UINT32_CORPSE_DECAY_WORLDBOSS);
                break;
            default:
                m_corpseDelay = sWorld.getConfig(CONFIG_UINT32_CORPSE_DECAY_NORMAL);
                break;
        }
    }

    if (cinfo->InteractionPauseTimer != -1)
        m_interactionPauseTimer = cinfo->InteractionPauseTimer;
    else
        m_interactionPauseTimer = sWorld.getConfig(CONFIG_UINT32_INTERACTION_PAUSE_TIMER);

    return true;
}

bool Creature::UpdateEntry(uint32 Entry, const CreatureData* data /*=nullptr*/, GameEventCreatureData const* eventData /*=nullptr*/, bool preserveHPAndPower /*=true*/)
{
    if (!InitEntry(Entry, data, eventData))
        return false;

    // creatures always have melee weapon ready if any
    SetSheath(SHEATH_STATE_MELEE);
    SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_MISC_FLAGS, UNIT_BYTE2_FLAG_AURAS);

    if (preserveHPAndPower)
    {
        uint32 healthPercent = GetHealthPercent();
        SelectLevel();
        SetHealthPercent(healthPercent);
    }
    else
    {
        SelectLevel();
        if (data)
        {
            uint32 curhealth = data->spawnTemplate->curHealth > 0 ? data->spawnTemplate->curHealth : GetMaxHealth();
            SetHealth(m_deathState == ALIVE ? curhealth : 0);
            if (GetPowerType() == POWER_MANA)
            {
                uint32 curmana;
                uint32 newPossibleData = data->spawnTemplate->curMana;
                if (IsRegeneratingPower()) // bypass so that 0 mana is possible TODO: change this to -1 in DB
                    curmana = newPossibleData ? newPossibleData : GetMaxPower(POWER_MANA);
                else
                    curmana = newPossibleData;
                SetPower(POWER_MANA, curmana);
            }
        }
    }

    uint32 faction = GetCreatureInfo()->Faction;
    if (data && data->spawnTemplate->faction)
        faction = data->spawnTemplate->faction;
    // update entry can occur during player vehicle ride - ignore faction change then
    if (!HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && !hasUnitState(UNIT_STAT_POSSESSED))
        setFaction(faction);

    SetDefaultGossipMenuId(GetCreatureInfo()->GossipMenuId);

    uint32 npcFlags = GetCreatureInfo()->NpcFlags;
    if (data && data->spawnTemplate->npcFlags != -1)
        npcFlags = uint32(data->spawnTemplate->npcFlags);
    SetUInt32Value(UNIT_NPC_FLAGS, npcFlags);

    uint32 attackTimer = GetCreatureInfo()->MeleeBaseAttackTime;

    SetAttackTime(BASE_ATTACK,  attackTimer);
    SetAttackTime(OFF_ATTACK,   attackTimer);
    SetAttackTime(RANGED_ATTACK, GetCreatureInfo()->RangedBaseAttackTime);

    uint32 unitFlags = GetCreatureInfo()->UnitFlags;
    if (data && data->spawnTemplate->unitFlags != -1)
        unitFlags = uint32(data->spawnTemplate->unitFlags);

    // we may need to append or remove additional flags
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT))
        unitFlags |= UNIT_FLAG_IN_COMBAT;
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT))
        unitFlags |= UNIT_FLAG_PET_IN_COMBAT;
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        unitFlags |= UNIT_FLAG_PLAYER_CONTROLLED;
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED))
        unitFlags |= UNIT_FLAG_POSSESSED;

    // TODO: Get rid of this by fixing DB data, seems to be static
    if (m_movementInfo.HasMovementFlag(MOVEFLAG_SWIMMING))
        unitFlags |= UNIT_FLAG_SWIMMING;

    SetUInt32Value(UNIT_FIELD_FLAGS, unitFlags);

    // preserve all current dynamic flags if exist
    uint32 dynFlags = GetUInt32Value(UNIT_DYNAMIC_FLAGS);
    SetUInt32Value(UNIT_DYNAMIC_FLAGS, dynFlags ? dynFlags : GetCreatureInfo()->DynamicFlags);

    SetCreateResistance(SPELL_SCHOOL_HOLY, GetCreatureInfo()->ResistanceHoly);
    SetCreateResistance(SPELL_SCHOOL_FIRE, GetCreatureInfo()->ResistanceFire);
    SetCreateResistance(SPELL_SCHOOL_NATURE, GetCreatureInfo()->ResistanceNature);
    SetCreateResistance(SPELL_SCHOOL_FROST, GetCreatureInfo()->ResistanceFrost);
    SetCreateResistance(SPELL_SCHOOL_SHADOW, GetCreatureInfo()->ResistanceShadow);
    SetCreateResistance(SPELL_SCHOOL_ARCANE, GetCreatureInfo()->ResistanceArcane);

    m_isInvisible = (GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_INVISIBLE) != 0;
    m_ignoreMMAP = (GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_MMAP_FORCE_DISABLE) != 0;
    m_countSpawns = (GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_COUNT_SPAWNS) != 0;

    if (IsWorldBoss())
        ApplySpellImmune(nullptr, IMMUNITY_STATE, SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE, true);

    SetCanModifyStats(true);
    UpdateAllStats();

    // checked and error show at loading templates
    if (FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(faction))
    {
        if (factionTemplate->factionFlags & FACTION_TEMPLATE_FLAG_ASSIST_PLAYERS)
            SetPvP(true);
    }

    if (GetCreatureInfo()->SpellList)
        SetSpellList(GetCreatureInfo()->SpellList);
    else // legacy compatibility
        SetSpellList(Entry * 100 + 0);
    UpdateImmunitiesSet(0);
    if (IsCritter()) // meant to be also settable per creature immunity set
        SetChainImmune(true);

    // if eventData set then event active and need apply spell_start
    if (eventData)
        ApplyGameEventSpells(eventData, true);

    if (GetCreatureInfo()->StringID1)
        SetStringId(GetCreatureInfo()->StringID1, true);

    if (GetCreatureInfo()->StringID2)
        SetStringId(GetCreatureInfo()->StringID2, true);

    if (data && data->spawnTemplate->stringId)
        SetStringId(data->spawnTemplate->stringId, true);

    // static flags implementation
    m_settings.ResetStaticFlags(CreatureStaticFlags(GetCreatureInfo()->StaticFlags), CreatureStaticFlags2(GetCreatureInfo()->StaticFlags2), CreatureStaticFlags3(GetCreatureInfo()->StaticFlags3), CreatureStaticFlags4(GetCreatureInfo()->StaticFlags4));

    return true;
}

void Creature::ResetEntry(bool respawn)
{
    bool isPet = GetObjectGuid().GetHigh() == HIGHGUID_PET;
    CreatureData const* data = isPet ? nullptr : sObjectMgr.GetCreatureData(GetDbGuid());
    GameEventCreatureData const* eventData = isPet ? nullptr : sGameEventMgr.GetCreatureUpdateDataForActiveEvent(GetDbGuid());

    if (respawn)
    {
        uint32 newEntry = isPet ? 0 : sObjectMgr.GetRandomCreatureEntry(GetDbGuid());
        if (newEntry)
        {
            UpdateEntry(newEntry, data, eventData, false);
            AIM_Initialize();
        }
        else
            UpdateEntry(m_originalEntry, data, eventData, false);
    }
    else
        UpdateEntry(m_originalEntry, data, eventData, false);
}

uint32 Creature::ChooseDisplayId(const CreatureInfo* cinfo, const CreatureData* data /*= nullptr*/, GameEventCreatureData const* eventData /*=nullptr*/)
{
    // Use creature event model explicit, override any other static models
    if (eventData && eventData->modelid)
        return eventData->modelid;

    // Use creature model explicit, override template (creature.modelid)
    if (data)
    {
        if (data->spawnTemplate->modelId)
            return data->spawnTemplate->modelId;
    }

    // use defaults from the template
    uint32 display_id = 0;
    // pick based on probability
    uint32 chanceTotal = 0;
    for (uint32 i = 0; i < MAX_CREATURE_MODEL; ++i)
        if (cinfo->DisplayId[i])
            chanceTotal += cinfo->DisplayIdProbability[i];

    int32 roll = irand(0, std::max(int32(chanceTotal) - 1, 0)); // avoid 0
    for (uint32 i = 0; i < MAX_CREATURE_MODEL; ++i)
    {
        if (cinfo->DisplayId[i])
        {
            if (roll < int32(cinfo->DisplayIdProbability[i]))
            {
                display_id = cinfo->DisplayId[i];
                break;
            }
            else
                roll -= cinfo->DisplayIdProbability[i];
        }
    }

    // fail safe, we use creature entry 1 and make error
    if (!display_id)
    {
        sLog.outErrorDb("Call customer support, ChooseDisplayId can not select native model for creature entry %u, model from creature entry 1 will be used instead.", cinfo->Entry);

        if (const CreatureInfo* creatureDefault = ObjectMgr::GetCreatureTemplate(1))
            display_id = creatureDefault->DisplayId[0];
    }

    return display_id;
}

void Creature::Update(const uint32 diff)
{
    switch (m_deathState)
    {
        case JUST_ALIVED:
            // Don't must be called, see Creature::SetDeathState JUST_ALIVED -> ALIVE promoting.
            sLog.outError("Creature (GUIDLow: %u Entry: %u ) in wrong state: JUST_ALIVED (4)", GetGUIDLow(), GetEntry());
            break;
        case JUST_DIED:
            // Don't must be called, see Creature::SetDeathState JUST_DIED -> CORPSE promoting.
            sLog.outError("Creature (GUIDLow: %u Entry: %u ) in wrong state: JUST_DEAD (1)", GetGUIDLow(), GetEntry());
            break;
        case DEAD:
        {
            if (m_respawnTime <= time(nullptr) && (!m_isSpawningLinked || GetMap()->GetCreatureLinkingHolder()->CanSpawn(this)))
            {
                DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "Respawning...");
                m_respawnTime = 0;
                SetCanAggro(false);
                delete m_loot;
                m_loot = nullptr;

                // Clear possible auras having IsDeathPersistent() attribute
                RemoveAllAuras();

                SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_NONE);
                SetDeathState(JUST_ALIVED);

                // Call AI respawn virtual function
                if (AI())
                    AI()->JustRespawned();

                if (InstanceData* mapInstance = GetInstanceData())
                    mapInstance->OnCreatureRespawn(this);

                if (m_isCreatureLinkingTrigger)
                    GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_RESPAWN, this);

                if (GetCreatureGroup())
                    GetCreatureGroup()->TriggerLinkingEvent(CREATURE_GROUP_EVENT_RESPAWN, this);

                GetMap()->Add(this);

                if (GetObjectGuid().GetHigh() != HIGHGUID_PET)
                    if (uint16 poolid = sPoolMgr.IsPartOfAPool<Creature>(GetDbGuid()))
                        sPoolMgr.UpdatePool<Creature>(*GetMap()->GetPersistentState(), poolid, GetDbGuid());
            }
            break;
        }
        case CORPSE:
        {
            Unit::Update(diff);

            if (m_loot)
                m_loot->Update();

            if (IsCorpseExpired())
                RemoveCorpse();

            break;
        }
        case ALIVE:
        {
            Unit::Update(diff);

            // creature can be dead after Unit::Update call
            // CORPSE/DEAD state will processed at next tick (in other case death timer will be updated unexpectedly)
            if (!IsAlive())
                break;

            // Creature can be dead after unit update
            if (IsAlive())
                RegenerateAll(diff);

            break;
        }
        default:
            break;
    }
}

void Creature::RegenerateAll(uint32 diff)
{
    m_regenTimer += diff;
    if (m_regenTimer < REGEN_TIME_FULL_UNIT)
        return;

    if (!IsInCombat() || GetCombatManager().IsEvadeRegen())
        RegenerateHealth();

    RegeneratePower(REGEN_TIME_FULL_UNIT / 1000);

    m_regenTimer -= REGEN_TIME_FULL_UNIT;
}

void Creature::RegeneratePower(float timerMultiplier)
{
    if (!IsRegeneratingPower())
        return;

    Powers powerType = GetPowerType();
    uint32 curValue = GetPower(powerType);
    uint32 maxValue = GetMaxPower(powerType);

    if (curValue >= maxValue)
        return;

    float addValue = 0.0f;

    switch (powerType)
    {
        case POWER_MANA:
            // Combat and any controlled creature
            if (IsInCombat() || GetMasterGuid())
            {
                if (!IsUnderLastManaUseEffect())
                {
                    float ManaIncreaseRate = sWorld.getConfig(CONFIG_FLOAT_RATE_POWER_MANA);
                    float intellect = GetStat(STAT_INTELLECT);
                    addValue = sqrt(intellect) * OCTRegenMPPerSpirit() * ManaIncreaseRate / 5.f * timerMultiplier;
                    if (!IsPet() && !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && addValue == 0.f)
                        addValue = (GetMaxPower(POWER_MANA) / 20) / 5.f * timerMultiplier;
                }
            }
            else
                addValue = maxValue / 3.0f;
            break;
        case POWER_ENERGY:
            // ToDo: for vehicle this is different - NEEDS TO BE FIXED!
            addValue = 20 * sWorld.getConfig(CONFIG_FLOAT_RATE_POWER_ENERGY);
            break;
        case POWER_FOCUS:
            addValue = 24 * sWorld.getConfig(CONFIG_FLOAT_RATE_POWER_FOCUS);
            break;
        default:
            return;
    }

    // Apply modifiers (if any)
    AuraList const& ModPowerRegenAuras = GetAurasByType(SPELL_AURA_MOD_POWER_REGEN);
    for (auto ModPowerRegenAura : ModPowerRegenAuras)
    {
        Modifier const* modifier = ModPowerRegenAura->GetModifier();
        if (modifier->m_miscvalue == int32(powerType))
            addValue += modifier->m_amount / 5.f * timerMultiplier;
    }

    AuraList const& ModPowerRegenPCTAuras = GetAurasByType(SPELL_AURA_MOD_POWER_REGEN_PERCENT);
    for (auto ModPowerRegenPCTAura : ModPowerRegenPCTAuras)
    {
        Modifier const* modifier = ModPowerRegenPCTAura->GetModifier();
        if (modifier->m_miscvalue == int32(powerType))
            addValue *= (modifier->m_amount + 100) / 100.0f;
    }

    ModifyPower(powerType, int32(addValue));
}

void Creature::RegenerateHealth()
{
    if (!IsRegeneratingHealth())
        return;

    uint32 curValue = GetHealth();
    uint32 maxValue = GetMaxHealth();

    if (curValue >= maxValue)
        return;

    // regenerate 33% for npc and ~13% for player controlled npc (enslave etc) ToDo: Find Regenvalue based on Spirit, might differ due to mob types
    uint32 addvalue = IsPlayerControlled() ? maxValue * 0.13 : maxValue / 3;

    ModifyHealth(addvalue);
}

bool Creature::AIM_Initialize()
{
    i_motionMaster.Initialize();
    m_ai.reset(FactorySelector::selectAI(this));

    // Handle Spawned Events, also calls Reset()
    m_ai->SpellListChanged();
    m_ai->JustRespawned();

    if (InstanceData* mapInstance = GetInstanceData())
        mapInstance->OnCreatureRespawn(this);

    if (GetSettings().HasFlag(CreatureStaticFlags::CREATOR_LOOT))
        SetLootRecipient(GetCreator());

    return true;
}

bool Creature::Create(uint32 dbGuid, uint32 guidlow, CreatureCreatePos& cPos, CreatureInfo const* cinfo, const CreatureData* data /*= nullptr*/, GameEventCreatureData const* eventData /*= nullptr*/)
{
    SetMap(cPos.GetMap());
    SetRespawnCoord(cPos);

    if (!CreateFromProto(dbGuid, guidlow, cinfo, data, eventData))
        return false;

    cPos.SelectFinalPoint(this, data != nullptr);

    if (!cPos.Relocate(this))
        return false;

    // Notify the pvp script
    if (GetMap()->IsBattleGround())
        static_cast<BattleGroundMap*>(GetMap())->GetBG()->HandleCreatureCreate(this);
    else if (OutdoorPvP* outdoorPvP = sOutdoorPvPMgr.GetScript(GetZoneId()))
        outdoorPvP->HandleCreatureCreate(this);

    // Notify the map's instance data.
    // Only works if you create the object in it, not if it is moves to that map.
    // Normally non-players do not teleport to other maps.
    if (InstanceData * iData = GetMap()->GetInstanceData())
        iData->OnCreatureCreate(this);

    // Add to CreatureLinkingHolder if needed
    if (sCreatureLinkingMgr.GetLinkedTriggerInformation(this))
        cPos.GetMap()->GetCreatureLinkingHolder()->AddSlaveToHolder(this);
    if (sCreatureLinkingMgr.IsLinkedEventTrigger(this))
    {
        m_isCreatureLinkingTrigger = true;
        cPos.GetMap()->GetCreatureLinkingHolder()->AddMasterToHolder(this);
    }

    LoadCreatureAddon(false);

    return true;
}

bool Creature::IsTrainerOf(Player* pPlayer, bool msg) const
{
    if (!isTrainer())
        return false;

    TrainerSpellData const* cSpells = GetTrainerSpells();
    TrainerSpellData const* tSpells = GetTrainerTemplateSpells();

    // for not pet trainer expected not empty trainer list always
    if ((!cSpells || cSpells->spellList.empty()) && (!tSpells || tSpells->spellList.empty()))
    {
        sLog.outErrorDb("Creature %u (Entry: %u) have UNIT_NPC_FLAG_TRAINER but have empty trainer spell list.",
                        GetGUIDLow(), GetEntry());
        return false;
    }

    switch (GetCreatureInfo()->TrainerType)
    {
        case TRAINER_TYPE_CLASS:
            if (pPlayer->getClass() != GetCreatureInfo()->TrainerClass)
            {
                if (msg)
                {
                    pPlayer->GetPlayerMenu()->ClearMenus();
                    switch (GetCreatureInfo()->TrainerClass)
                    {
                        case CLASS_DRUID:  pPlayer->GetPlayerMenu()->SendGossipMenu(4913, GetObjectGuid()); break;
                        case CLASS_HUNTER: pPlayer->GetPlayerMenu()->SendGossipMenu(10090, GetObjectGuid()); break;
                        case CLASS_MAGE:   pPlayer->GetPlayerMenu()->SendGossipMenu(328, GetObjectGuid()); break;
                        case CLASS_PALADIN: pPlayer->GetPlayerMenu()->SendGossipMenu(1635, GetObjectGuid()); break;
                        case CLASS_PRIEST: pPlayer->GetPlayerMenu()->SendGossipMenu(4436, GetObjectGuid()); break;
                        case CLASS_ROGUE:  pPlayer->GetPlayerMenu()->SendGossipMenu(4797, GetObjectGuid()); break;
                        case CLASS_SHAMAN: pPlayer->GetPlayerMenu()->SendGossipMenu(5003, GetObjectGuid()); break;
                        case CLASS_WARLOCK: pPlayer->GetPlayerMenu()->SendGossipMenu(5836, GetObjectGuid()); break;
                        case CLASS_WARRIOR: pPlayer->GetPlayerMenu()->SendGossipMenu(4985, GetObjectGuid()); break;
                    }
                }
                return false;
            }
            break;
        case TRAINER_TYPE_PETS:
            if (pPlayer->getClass() != CLASS_HUNTER)
            {
                if (msg)
                {
                    pPlayer->GetPlayerMenu()->ClearMenus();
                    pPlayer->GetPlayerMenu()->SendGossipMenu(3620, GetObjectGuid());
                }
                return false;
            }
            break;
        case TRAINER_TYPE_MOUNTS:
            if (GetCreatureInfo()->TrainerRace && pPlayer->getRace() != GetCreatureInfo()->TrainerRace)
            {
                // Allowed to train if exalted
                if (FactionTemplateEntry const* faction_template = GetFactionTemplateEntry())
                {
                    if (pPlayer->GetReputationRank(faction_template->faction) == REP_EXALTED)
                        return true;
                }

                if (msg)
                {
                    pPlayer->GetPlayerMenu()->ClearMenus();
                    switch (GetCreatureInfo()->TrainerClass)
                    {
                        case RACE_DWARF:        pPlayer->GetPlayerMenu()->SendGossipMenu(5865, GetObjectGuid()); break;
                        case RACE_GNOME:        pPlayer->GetPlayerMenu()->SendGossipMenu(4881, GetObjectGuid()); break;
                        case RACE_HUMAN:        pPlayer->GetPlayerMenu()->SendGossipMenu(5861, GetObjectGuid()); break;
                        case RACE_NIGHTELF:     pPlayer->GetPlayerMenu()->SendGossipMenu(5862, GetObjectGuid()); break;
                        case RACE_ORC:          pPlayer->GetPlayerMenu()->SendGossipMenu(5863, GetObjectGuid()); break;
                        case RACE_TAUREN:       pPlayer->GetPlayerMenu()->SendGossipMenu(5864, GetObjectGuid()); break;
                        case RACE_TROLL:        pPlayer->GetPlayerMenu()->SendGossipMenu(5816, GetObjectGuid()); break;
                        case RACE_UNDEAD:       pPlayer->GetPlayerMenu()->SendGossipMenu(624, GetObjectGuid()); break;
                    }
                }
                return false;
            }
            break;
        case TRAINER_TYPE_TRADESKILLS:
            if (GetCreatureInfo()->TrainerSpell && !pPlayer->HasSpell(GetCreatureInfo()->TrainerSpell))
            {
                if (msg)
                {
                    pPlayer->GetPlayerMenu()->ClearMenus();
                    pPlayer->GetPlayerMenu()->SendGossipMenu(11031, GetObjectGuid());
                }
                return false;
            }
            break;
        default:
            return false;                                   // checked and error output at creature_template loading
    }
    return true;
}

bool Creature::CanInteractWithBattleMaster(Player* pPlayer, bool msg) const
{
    if (!isBattleMaster())
        return false;

    BattleGroundTypeId bgTypeId = GetMap()->GetMapDataContainer().GetBattleMasterBG(GetEntry());
    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
        return false;

    if (!msg)
        return pPlayer->GetBGAccessByLevel(bgTypeId);

    if (!pPlayer->GetBGAccessByLevel(bgTypeId))
    {
        pPlayer->GetPlayerMenu()->ClearMenus();
        switch (bgTypeId)
        {
            case BATTLEGROUND_AV:  pPlayer->GetPlayerMenu()->SendGossipMenu(7616, GetObjectGuid()); break;
            case BATTLEGROUND_WS:  pPlayer->GetPlayerMenu()->SendGossipMenu(7599, GetObjectGuid()); break;
            case BATTLEGROUND_AB:  pPlayer->GetPlayerMenu()->SendGossipMenu(7642, GetObjectGuid()); break;
            default: break;
        }
        return false;
    }
    return true;
}

bool Creature::CanTrainAndResetTalentsOf(Player* pPlayer) const
{
    return pPlayer->GetLevel() >= 10
           && GetCreatureInfo()->TrainerType == TRAINER_TYPE_CLASS
           && pPlayer->getClass() == GetCreatureInfo()->TrainerClass;
}

void Creature::PrepareBodyLootState(Unit* killer)
{
    // if can weild loot - already generated on spawn
    if (GetSettings().HasFlag(CreatureStaticFlags::CAN_WIELD_LOOT) && m_loot != nullptr && m_loot->GetLootType() == LOOT_CORPSE)
        return;

    // loot may already exist (pickpocket case)
    delete m_loot;
    m_loot = nullptr;

    if (IsNoLoot())
        SetLootStatus(CREATURE_LOOT_STATUS_LOOTED);
    else
    {
        Player* looter = nullptr;
        if (GetSettings().HasFlag(CreatureStaticFlags3::CAN_BE_MULTITAPPED))
            looter = dynamic_cast<Player*>(killer);
        else
            looter = GetLootRecipient();

        if (looter || GetSettings().HasFlag(CreatureStaticFlags::CAN_WIELD_LOOT))
            m_loot = new Loot(looter, this, LOOT_CORPSE);
    }

    if (m_lootStatus == CREATURE_LOOT_STATUS_LOOTED && !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE))
    {
        // there is no loot so we can degrade corpse decay timer
        ReduceCorpseDecayTimer();
    }
}

/**
 * Return original player who tap creature, it can be different from player/group allowed to loot so not use it for loot code
 */
Player* Creature::GetOriginalLootRecipient() const
{
    return m_lootRecipientGuid ? ObjectAccessor::FindPlayer(m_lootRecipientGuid) : nullptr;
}

/**
 * Return group if player tap creature as group member, independent is player after leave group or stil be group member
 */
Group* Creature::GetGroupLootRecipient() const
{
    // original recipient group if set and not disbanded
    return m_lootGroupRecipientId ? sObjectMgr.GetGroupById(m_lootGroupRecipientId) : nullptr;
}

/**
 * Return player who can loot tapped creature (member of group or single player)
 *
 * In case when original player tap creature as group member then group tap prefered.
 * This is for example important if player after tap leave group.
 * If group not exist or disbanded or player tap creature not as group member return player
 */
Player* Creature::GetLootRecipient() const
{
    // original recipient group if set and not disbanded
    Group* group = GetGroupLootRecipient();

    // original recipient player if online
    Player* player = GetOriginalLootRecipient();

    // if group not set or disbanded return original recipient player if any
    if (!group)
        return player;

    // group case

    // return player if it still be in original recipient group
    if (player && player->GetGroup() == group)
        return player;

    // find any in group
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        if (Player* p = itr->getSource())
            return p;

    return nullptr;
}

/**
 * Set player and group (if player group member) who tap creature
 */
void Creature::SetLootRecipient(Unit* unit)
{
    // set the player whose group should receive the right
    // to loot the creature after it dies
    // should be set to nullptr after the loot disappears

    if (!unit)
    {
        m_lootRecipientGuid.Clear();
        m_lootGroupRecipientId = 0;
        ForceValuesUpdateAtIndex(UNIT_DYNAMIC_FLAGS);       // needed to be sure tapping status is updated
        return;
    }

    if (GetSettings().HasFlag(CreatureStaticFlags3::CAN_BE_MULTITAPPED))
        return;

    Player* player = unit->GetBeneficiaryPlayer();
    if (!player)                                            // normal creature, no player involved
        return;

    // set player for non group case or if group will disbanded
    m_lootRecipientGuid = player->GetObjectGuid();

    // set group for group existing case including if player will leave group at loot time
    if (Group* group = player->GetGroup())
        m_lootGroupRecipientId = group->GetId();

    ForceValuesUpdateAtIndex(UNIT_DYNAMIC_FLAGS);           // needed to be sure tapping status is updated
}

void Creature::SaveToDB()
{
    // this should only be used when the creature has already been loaded
    // preferably after adding to map, because mapid may not be valid otherwise
    CreatureData const* data = sObjectMgr.GetCreatureData(GetDbGuid());
    if (!data)
    {
        sLog.outError("Creature::SaveToDB failed, cannot get creature data!");
        return;
    }

    SaveToDB(GetMapId());
}

void Creature::SaveToDB(uint32 mapid)
{
    // update in loaded data
    CreatureData& data = sObjectMgr.NewOrExistCreatureData(GetGUIDLow());

    uint32 displayId = GetNativeDisplayId();

    // check if it's a custom model and if not, use 0 for displayId
    CreatureInfo const* cinfo = GetCreatureInfo();
    if (cinfo)
    {
        // The following if-else assumes that there are 4 model fields and needs updating if this is changed.
        static_assert(MAX_CREATURE_MODEL == 4, "Need to update custom model check for new/removed model fields.");

        if (displayId != cinfo->DisplayId[0] && displayId != cinfo->DisplayId[1] &&
                displayId != cinfo->DisplayId[2] && displayId != cinfo->DisplayId[3])
        {
            for (int i = 0; i < MAX_CREATURE_MODEL && displayId; ++i)
                if (cinfo->DisplayId[i])
                    if (CreatureModelInfo const* minfo = sObjectMgr.GetCreatureModelInfo(cinfo->DisplayId[i]))
                        if (displayId == minfo->modelid_other_gender)
                            displayId = 0;
        }
        else
            displayId = 0;
    }

    // data->guid = guid don't must be update at save
    data.id = GetEntry();
    data.mapid = mapid;
    data.spawnMask = 1;
    data.posX = GetPositionX();
    data.posY = GetPositionY();
    data.posZ = GetPositionZ();
    data.orientation = GetOrientation();
    data.spawntimesecsmin = m_respawnDelay;
    data.spawntimesecsmax = m_respawnDelay;
    // prevent add data integrity problems
    data.spawndist = GetDefaultMovementType() == IDLE_MOTION_TYPE ? 0 : m_respawnradius;
    // prevent add data integrity problems
    data.movementType = !m_respawnradius && GetDefaultMovementType() == RANDOM_MOTION_TYPE
                        ? IDLE_MOTION_TYPE : GetDefaultMovementType();
    data.spawnTemplate = sObjectMgr.GetCreatureSpawnTemplate(0);

    // updated in DB
    WorldDatabase.BeginTransaction();

    WorldDatabase.PExecuteLog("DELETE FROM creature WHERE guid=%u", GetGUIDLow());

    std::ostringstream ss;
    ss << "INSERT INTO creature VALUES ("
       << GetGUIDLow() << ","
       << data.id << ","
       << data.mapid << ","
       << static_cast<uint32>(data.spawnMask) << ","       // cast to prevent save as symbol
       << data.posX << ","
       << data.posY << ","
       << data.posZ << ","
       << data.orientation << ","
       << data.spawntimesecsmin << ","                     // respawn time minimum
       << data.spawntimesecsmax << ","                     // respawn time maximum
       << static_cast<float>(data.spawndist) << ","        // spawn distance (float)
       << static_cast<uint32>(data.movementType) << ")";   // default movement generator type, cast to prevent save as symbol

    WorldDatabase.PExecuteLog("%s", ss.str().c_str());

    WorldDatabase.CommitTransaction();
}

void Creature::SelectLevel(uint32 forcedLevel /*= USE_DEFAULT_DATABASE_LEVEL*/)
{
    CreatureInfo const* cinfo = GetCreatureInfo();
    if (!cinfo)
        return;

    uint32 rank = IsPet() ? 0 : cinfo->Rank;                // TODO :: IsPet probably not needed here

    // level
    uint32 level = forcedLevel;
    uint32 const minlevel = cinfo->MinLevel;
    uint32 const maxlevel = cinfo->MaxLevel;

    if (level == USE_DEFAULT_DATABASE_LEVEL)
        level = minlevel == maxlevel ? minlevel : urand(minlevel, maxlevel);

    SetLevel(level);

    //////////////////////////////////////////////////////////////////////////
    // Calculate level dependent stats
    //////////////////////////////////////////////////////////////////////////

    uint32 health = static_cast<uint32>(-1);
    uint32 mana = static_cast<uint32>(-1);
    float armor = -1.f;
    float mainMinDmg = 0.f;
    float mainMaxDmg = 0.f;
    float offMinDmg = 0.f;
    float offMaxDmg = 0.f;
    float minRangedDmg = 0.f;
    float maxRangedDmg = 0.f;
    float meleeAttackPwr = 0.f;
    float rangedAttackPwr = 0.f;

    float healthMultiplier = 1.f;
    float powerMultiplier = 1.f;

    float strength = 0.f;
    float agility = 0.f;
    float stamina = 0.f;
    float intellect = 0.f;
    float spirit = 0.f;

    float damageMod = _GetDamageMod(rank);
    float damageMulti = cinfo->DamageMultiplier * damageMod;
    bool usedDamageMulti = false;

    if (CreatureClassLvlStats const* cCLS = sObjectMgr.GetCreatureClassLvlStats(level, cinfo->UnitClass))
    {
        // Use Creature Stats to calculate stat values

        if (cinfo->HealthMultiplier >= 0)
            health = cCLS->BaseHealth;
        // health
        if (cinfo->HealthMultiplier > 0)
            healthMultiplier = cinfo->HealthMultiplier;

        if (cinfo->PowerMultiplier >= 0)
            mana = cCLS->BaseMana;
        // mana
        if (cinfo->PowerMultiplier > 0)
            powerMultiplier = cinfo->PowerMultiplier;

        // armor
        if (cinfo->ArmorMultiplier >= 0)
            armor = std::round(cCLS->BaseArmor * cinfo->ArmorMultiplier);

        // damage
        if (cinfo->DamageMultiplier >= 0 && cinfo->ArmorMultiplier >= 0)
        {
            usedDamageMulti = true;
            mainMinDmg = ((cCLS->BaseDamage * cinfo->DamageVariance) + (cCLS->BaseMeleeAttackPower / 14.0f)) * (cinfo->MeleeBaseAttackTime / 1000.0f) * damageMulti;
            mainMaxDmg = ((cCLS->BaseDamage * cinfo->DamageVariance * 1.5f) + (cCLS->BaseMeleeAttackPower / 14.0f)) * (cinfo->MeleeBaseAttackTime / 1000.0f) * damageMulti;
            offMinDmg = mainMinDmg; // Unitmod handles 50%
            offMaxDmg = mainMaxDmg;
            minRangedDmg = ((cCLS->BaseDamage * cinfo->DamageVariance) + (cCLS->BaseRangedAttackPower / 14.0f)) * (cinfo->RangedBaseAttackTime / 1000.0f) * damageMulti;
            maxRangedDmg = ((cCLS->BaseDamage * cinfo->DamageVariance * 1.5f) + (cCLS->BaseRangedAttackPower / 14.0f)) * (cinfo->RangedBaseAttackTime / 1000.0f) * damageMulti;

            // attack power (not sure about the next line)
            meleeAttackPwr = cCLS->BaseMeleeAttackPower;
            rangedAttackPwr = cCLS->BaseRangedAttackPower;
        }

        // attributes
        if (cinfo->StrengthMultiplier >= 0)
            strength = cCLS->Strength * cinfo->StrengthMultiplier;
        if (cinfo->AgilityMultiplier >= 0)
            agility = cCLS->Agility * cinfo->AgilityMultiplier;
        if (cinfo->StaminaMultiplier >= 0)
            stamina = cCLS->Stamina * cinfo->StaminaMultiplier;
        if (cinfo->IntellectMultiplier >= 0)
            intellect = cCLS->Intellect * cinfo->IntellectMultiplier;
        if (cinfo->SpiritMultiplier >= 0)
            spirit = cCLS->Spirit * cinfo->SpiritMultiplier;
    }

    if (!usedDamageMulti || armor == -1.f) // some field needs to default to old db fields
    {
        if (forcedLevel == USE_DEFAULT_DATABASE_LEVEL || (forcedLevel >= minlevel && forcedLevel <= maxlevel))
        {
            // Use old style to calculate stat values
            float rellevel = maxlevel == minlevel ? 0 : (float(level - minlevel)) / (maxlevel - minlevel);

            // health, mana and armor
            if (armor == -1.f)
            {
                uint32 minhealth = std::min(cinfo->MaxLevelHealth, cinfo->MinLevelHealth);
                uint32 maxhealth = std::max(cinfo->MaxLevelHealth, cinfo->MinLevelHealth);
                health = uint32(minhealth + uint32(rellevel * (maxhealth - minhealth)));

                uint32 minmana = std::min(cinfo->MaxLevelMana, cinfo->MinLevelMana);
                uint32 maxmana = std::max(cinfo->MaxLevelMana, cinfo->MinLevelMana);
                mana = minmana + uint32(rellevel * (maxmana - minmana));

                armor = cinfo->Armor;
            }

            // damage
            if (!usedDamageMulti)
            {
                mainMinDmg = cinfo->MinMeleeDmg * damageMulti;
                mainMaxDmg = cinfo->MaxMeleeDmg * damageMulti;
                offMinDmg = cinfo->MinMeleeDmg * damageMulti / 2.0f;
                offMaxDmg = cinfo->MaxMeleeDmg * damageMulti / 2.0f;
                minRangedDmg = cinfo->MinRangedDmg * damageMulti;
                maxRangedDmg = cinfo->MaxRangedDmg * damageMulti;

                // attack power
                meleeAttackPwr = cinfo->MeleeAttackPower;
                rangedAttackPwr = cinfo->RangedAttackPower;
            }
        }
        else
        {
            sLog.outError("Creature::SelectLevel> Error trying to set level(%u) for creature %s without enough data to do it!", level, GetGuidStr().c_str());
            // probably wrong
            health = (cinfo->MaxLevelHealth / cinfo->MaxLevel) * level;
            mana = (cinfo->MaxLevelMana / cinfo->MaxLevel) * level;
            armor = 0;
        }
    }

    health *= _GetHealthMod(rank); // Apply custom config setting
    if (health < 1)
        health = 1;

    //////////////////////////////////////////////////////////////////////////
    // Set values
    //////////////////////////////////////////////////////////////////////////

    // health
    SetCreateHealth(health);
    SetMaxHealth(health);

    // all power types
    for (int i = POWER_MANA; i <= POWER_HAPPINESS; ++i)
    {
        uint32 maxValue = 0;

        switch (i)
        {
            case POWER_MANA:        maxValue = mana; break;
            case POWER_RAGE:        maxValue = 0; break;
            case POWER_FOCUS:       maxValue = POWER_FOCUS_DEFAULT; break;
            case POWER_ENERGY:      maxValue = 0; break;
            case POWER_HAPPINESS:   maxValue = POWER_HAPPINESS_DEFAULT; break;
        }

        uint32 value = maxValue;

        // For non regenerating powers set 0
        if (i == POWER_ENERGY && !IsRegeneratingPower())
            value = 0;

        // Mana requires an extra field to be set
        SetMaxPower(Powers(i), maxValue);

        if (i == POWER_MANA)
            SetCreateMana(value);
        else // do not set modifier for mana
            SetModifierValue(UnitMods(UNIT_MOD_POWER_START + i), BASE_VALUE, float(value));
    }

    // Armor
    SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, armor);

    // damage
    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, mainMinDmg);
    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, mainMaxDmg);
    SetBaseWeaponDamage(OFF_ATTACK, MINDAMAGE, offMinDmg);
    SetBaseWeaponDamage(OFF_ATTACK, MAXDAMAGE, offMaxDmg);
    SetBaseWeaponDamage(RANGED_ATTACK, MINDAMAGE, minRangedDmg);
    SetBaseWeaponDamage(RANGED_ATTACK, MAXDAMAGE, maxRangedDmg);

    // attack power
    SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, meleeAttackPwr * damageMod);
    SetModifierValue(UNIT_MOD_ATTACK_POWER_RANGED, BASE_VALUE, rangedAttackPwr * damageMod);

    // primary attributes
    SetCreateStat(STAT_STRENGTH, strength);
    SetCreateStat(STAT_AGILITY, agility);
    SetCreateStat(STAT_STAMINA, stamina);
    SetCreateStat(STAT_INTELLECT, intellect);
    SetCreateStat(STAT_SPIRIT, spirit);

    // multipliers
    SetModifierValue(UNIT_MOD_HEALTH, TOTAL_PCT, healthMultiplier);
    SetModifierValue(UnitMods(UNIT_MOD_MANA + (int)GetPowerType()), TOTAL_PCT, powerMultiplier);

    UpdateAllStats();

    SetHealth(GetMaxHealth());
    for (int i = POWER_MANA; i <= POWER_HAPPINESS; ++i)
        SetPower(Powers(i), GetMaxPower(Powers(i)));
}

float Creature::_GetHealthMod(int32 Rank)
{
    switch (Rank)                                           // define rates for each elite rank
    {
        case CREATURE_ELITE_NORMAL:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_NORMAL_HP);
        case CREATURE_ELITE_ELITE:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_ELITE_ELITE_HP);
        case CREATURE_ELITE_RAREELITE:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_ELITE_RAREELITE_HP);
        case CREATURE_ELITE_WORLDBOSS:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_ELITE_WORLDBOSS_HP);
        case CREATURE_ELITE_RARE:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_ELITE_RARE_HP);
        default:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_ELITE_ELITE_HP);
    }
}

float Creature::_GetDamageMod(int32 Rank)
{
    switch (Rank)                                           // define rates for each elite rank
    {
        case CREATURE_ELITE_NORMAL:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_NORMAL_DAMAGE);
        case CREATURE_ELITE_ELITE:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_ELITE_ELITE_DAMAGE);
        case CREATURE_ELITE_RAREELITE:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_ELITE_RAREELITE_DAMAGE);
        case CREATURE_ELITE_WORLDBOSS:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_ELITE_WORLDBOSS_DAMAGE);
        case CREATURE_ELITE_RARE:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_ELITE_RARE_DAMAGE);
        default:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_ELITE_ELITE_DAMAGE);
    }
}

float Creature::_GetSpellDamageMod(int32 Rank)
{
    switch (Rank)                                           // define rates for each elite rank
    {
        case CREATURE_ELITE_NORMAL:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_NORMAL_SPELLDAMAGE);
        case CREATURE_ELITE_ELITE:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_ELITE_ELITE_SPELLDAMAGE);
        case CREATURE_ELITE_RAREELITE:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_ELITE_RAREELITE_SPELLDAMAGE);
        case CREATURE_ELITE_WORLDBOSS:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_ELITE_WORLDBOSS_SPELLDAMAGE);
        case CREATURE_ELITE_RARE:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_ELITE_RARE_SPELLDAMAGE);
        default:
            return sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_ELITE_ELITE_SPELLDAMAGE);
    }
}

std::vector<uint32> Creature::GetCharmSpells() const
{
    std::vector<uint32> spells(CREATURE_MAX_SPELLS, 0);
    for (auto& data : m_spellList.Spells)
        spells[data.second.Position] = data.second.SpellId;
    return spells;
}

Creature::CooldownResult Creature::GetSpellCooldown(uint32 spellId, uint32& cooldown) const
{
    for (auto& data : m_spellList.Spells)
    {
        if (data.second.SpellId == spellId)
        {
            cooldown = urand(data.second.RepeatMin, data.second.RepeatMax);
            return (data.second.Flags & SPELL_LIST_FLAG_CATEGORY_COOLDOWN) ? COOLDOWN_RESULT_CATEGORY_FOUND : COOLDOWN_RESULT_FOUND;
        }
    }
    return COOLDOWN_RESULT_NOT_FOUND;
}

void Creature::SetCreatureGroup(CreatureGroup* group)
{
    m_creatureGroup = group;
    group->AddObject(GetDbGuid(), GetEntry());
}

void Creature::ClearCreatureGroup()
{
    if (m_creatureGroup)
    {
        if (m_creatureGroup->GetFormationData())
            m_creatureGroup->GetFormationData()->Remove(this);
        m_creatureGroup->RemoveObject(this);
    }
    m_creatureGroup = nullptr;
}

void Creature::SetMountInfo(CreatureInfo const* info)
{
    if (info)
    {
        if (info->SpeedRun)
            SetBaseRunSpeed(info->SpeedRun);
    }
    else if (GetCreatureInfo()->SpeedRun)
        SetBaseRunSpeed(GetCreatureInfo()->SpeedRun);
    else
        UpdateModelData();

    m_mountInfo = info;
}

bool Creature::CreateFromProto(uint32 dbGuid, uint32 guidlow, CreatureInfo const* cinfo, const CreatureData* data /*=nullptr*/, GameEventCreatureData const* eventData /*=nullptr*/)
{
    m_originalEntry = cinfo->Entry;

    uint32 newEntry = cinfo->Entry;

    Object::_Create(dbGuid, guidlow, newEntry, cinfo->GetHighGuid());

    return UpdateEntry(newEntry, data, eventData, false);
}

bool Creature::LoadFromDB(uint32 dbGuid, Map* map, uint32 newGuid, uint32 forcedEntry, GenericTransport* transport)
{
    CreatureData const* data = sObjectMgr.GetCreatureData(dbGuid);

    if (!data)
    {
        sLog.outErrorDb("Creature (GUID: %u) not found in table `creature`, can't load. ", dbGuid);
        return false;
    }

    // Creature can be loaded already in map if grid has been unloaded while creature walk to another grid
    {
        Creature* existing = map->GetCreature(dbGuid);
        if (existing && existing->IsAlive())
            return false;
    }

    uint32 entry = forcedEntry ? forcedEntry : data->id;

    SpawnGroupEntry* groupEntry = map->GetMapDataContainer().GetSpawnGroupByGuid(dbGuid, TYPEID_UNIT); // use dynguid by default \o/
    CreatureGroup* group = nullptr;
    if (groupEntry)
    {
        group = static_cast<CreatureGroup*>(map->GetSpawnManager().GetSpawnGroup(groupEntry->Id));
        if (!entry)
            entry = group->GetGuidEntry(dbGuid);
    }

    // get data for dual spawn instances - spawn group precedes it because
    if (entry == 0)
        entry = GetCreatureConditionalSpawnEntry(dbGuid, map);

    if (entry == 0)
        entry = sObjectMgr.GetRandomCreatureEntry(GetDbGuid());

    GameEventCreatureData const* eventData = sGameEventMgr.GetCreatureUpdateDataForActiveEvent(dbGuid);
    if (!entry && eventData)
        entry = eventData->entry_id;

    if (!entry)
        return false;

    CreatureInfo const* cinfo = ObjectMgr::GetCreatureTemplate(entry);
    if (!cinfo)
    {
        sLog.outErrorDb("Creature (Entry: %u) not found in table `creature_template`, can't load. ", entry);
        return false;
    }

    bool dynguid = false;
    if (map->IsDynguidForced())
        dynguid = true;
    if (!dynguid)
    {
        if (((cinfo->ExtraFlags & CREATURE_EXTRA_FLAG_DYNGUID) != 0 || groupEntry) && dbGuid == newGuid)
            dynguid = true;
    }

    if (dynguid || newGuid == 0)
        newGuid = map->GenerateLocalLowGuid(cinfo->GetHighGuid());

    CreatureCreatePos pos(map, data->posX, data->posY, data->posZ, data->orientation);

    if (!Create(dbGuid, newGuid, pos, cinfo, data, eventData))
        return false;

    if (groupEntry)
    {
        SetCreatureGroup(group);

        if (groupEntry->StringId)
            SetStringId(groupEntry->StringId, true);
    }

    SetRespawnCoord(pos);
    m_respawnradius = data->spawndist;

    m_respawnDelay = data->GetRandomRespawnTime();
    bool isUsingNewSpawningSystem = IsUsingNewSpawningSystem();
    if (!isUsingNewSpawningSystem)
        m_corpseDelay = std::min(m_respawnDelay * 9 / 10, m_corpseDelay); // set corpse delay to 90% of the respawn delay
    m_deathState = ALIVE;

    m_respawnTime  = map->GetPersistentState()->GetCreatureRespawnTime(dbGuid);

    if (m_respawnTime > time(nullptr))                         // not ready to respawn
    {
        if (isUsingNewSpawningSystem && !group) // only at this point we know if marked as dynguid per entry
        {
            return false;
        }
        m_deathState = DEAD;
        SetHealth(0);
        if (CanFly())
        {
            float tz = GetTerrain()->GetHeightStatic(data->posX, data->posY, data->posZ, false);
            if (data->posZ - tz > 0.1)
                Relocate(data->posX, data->posY, tz);
        }
    }
    else if (m_respawnTime)                                 // respawn time set but expired
    {
        m_respawnTime = 0;

        GetMap()->GetPersistentState()->SaveCreatureRespawnTime(dbGuid, 0);
    }

    if (sCreatureLinkingMgr.IsSpawnedByLinkedMob(this))
    {
        m_isSpawningLinked = true;
        if (m_deathState == ALIVE && !GetMap()->GetCreatureLinkingHolder()->CanSpawn(this))
        {
            m_deathState = DEAD;
            SetHealth(0);

            // Just set to dead, so need to relocate like above
            if (CanFly())
            {
                float tz = GetTerrain()->GetHeightStatic(data->posX, data->posY, data->posZ, false);
                if (data->posZ - tz > 0.1)
                    Relocate(data->posX, data->posY, tz);
            }
        }
    }

    map->Add(this);

    AIM_Initialize();

    if (data && data->spawnTemplate->relayId)
        GetMap()->ScriptsStart(SCRIPT_TYPE_RELAY, data->spawnTemplate->relayId, this, nullptr);

    // Creature Linking, Initial load is handled like respawn
    if (m_isCreatureLinkingTrigger && IsAlive())
        GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_RESPAWN, this);

    if (GetCreatureGroup())
    {
        auto fData = m_creatureGroup->GetFormationData();
        if (fData)
            fData->SetFormationSlot(this);
        GetCreatureGroup()->TriggerLinkingEvent(CREATURE_GROUP_EVENT_RESPAWN, this);
    }

    // check if it is rabbit day
    if (IsAlive() && sWorld.getConfig(CONFIG_UINT32_RABBIT_DAY))
    {
        time_t rabbit_day = time_t(sWorld.getConfig(CONFIG_UINT32_RABBIT_DAY));
        tm rabbit_day_tm = *localtime(&rabbit_day);
        tm now_tm = *localtime(&sWorld.GetGameTime());

        if (now_tm.tm_mon == rabbit_day_tm.tm_mon && now_tm.tm_mday == rabbit_day_tm.tm_mday)
            CastSpell(this, 10710 + urand(0, 2), TRIGGERED_OLD_TRIGGERED);
    }

    return true;
}

void Creature::LoadEquipment(uint32 equip_entry, bool force)
{
    if (equip_entry == 0)
    {
        if (force)
        {
            for (uint8 i = 0; i < MAX_VIRTUAL_ITEM_SLOT; ++i)
                SetVirtualItem(VirtualItemSlot(i), 0);
            m_equipmentId = 0;
        }
        return;
    }

    if (EquipmentInfo const* einfo = sObjectMgr.GetEquipmentInfo(equip_entry))
    {
        m_equipmentId = equip_entry;
        for (uint8 i = 0; i < MAX_VIRTUAL_ITEM_SLOT; ++i)
            SetVirtualItem(VirtualItemSlot(i), einfo->equipentry[i]);
    }
}

bool Creature::HasQuest(uint32 quest_id) const
{
    QuestRelationsMapBounds bounds = sObjectMgr.GetCreatureQuestRelationsMapBounds(GetEntry());
    for (QuestRelationsMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        if (itr->second == quest_id)
            return true;
    }
    return false;
}

bool Creature::HasInvolvedQuest(uint32 quest_id) const
{
    QuestRelationsMapBounds bounds = sObjectMgr.GetCreatureQuestInvolvedRelationsMapBounds(GetEntry());
    for (QuestRelationsMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        if (itr->second == quest_id)
            return true;
    }
    return false;
}

bool Creature::IsRegeneratingPower() const
{
    if (IsInCombat())
        return (GetCreatureInfo()->RegenerateStats & REGEN_FLAG_POWER_IN_COMBAT) != 0;
    return (GetCreatureInfo()->RegenerateStats & REGEN_FLAG_POWER) != 0;
}


struct CreatureRespawnDeleteWorker
{
    explicit CreatureRespawnDeleteWorker(uint32 guid) : i_guid(guid) {}

    void operator()(MapPersistentState* state)
    {
        state->SaveCreatureRespawnTime(i_guid, 0);
    }

    uint32 i_guid;
};

void Creature::DeleteFromDB()
{
    CreatureData const* data = sObjectMgr.GetCreatureData(GetGUIDLow());
    if (!data)
    {
        DEBUG_LOG("Trying to delete not saved creature!");
        return;
    }

    DeleteFromDB(GetGUIDLow(), data);
}

void Creature::DeleteFromDB(uint32 lowguid, CreatureData const* data)
{
    CreatureRespawnDeleteWorker worker(lowguid);
    sMapPersistentStateMgr.DoForAllStatesWithMapId(data->mapid, worker);

    sObjectMgr.DeleteCreatureData(lowguid);

    WorldDatabase.BeginTransaction();
    WorldDatabase.PExecuteLog("DELETE FROM creature WHERE guid=%u", lowguid);
    WorldDatabase.PExecuteLog("DELETE FROM creature_addon WHERE guid=%u", lowguid);
    WorldDatabase.PExecuteLog("DELETE FROM creature_movement WHERE Id=%u", lowguid);
    WorldDatabase.PExecuteLog("DELETE FROM game_event_creature WHERE guid=%u", lowguid);
    WorldDatabase.PExecuteLog("DELETE FROM game_event_creature_data WHERE guid=%u", lowguid);
    WorldDatabase.PExecuteLog("DELETE FROM creature_battleground WHERE guid=%u", lowguid);
    WorldDatabase.PExecuteLog("DELETE FROM creature_linking WHERE guid=%u OR master_guid=%u", lowguid, lowguid);
    WorldDatabase.CommitTransaction();
}

void Creature::SetDeathState(DeathState s)
{
    if (s == JUST_DIED)
    {
        if (!m_respawnOverriden)
        {
            if (GetCreatureGroup() && GetCreatureGroup()->IsRespawnOverriden())
                m_respawnDelay = GetCreatureGroup()->GetRandomRespawnTime();
            else if (CreatureData const* data = sObjectMgr.GetCreatureData(GetDbGuid()))
                m_respawnDelay = data->GetRandomRespawnTime();
        }
        else if (m_respawnOverrideOnce)
            m_respawnOverriden = false;

        if (m_settings.HasFlag(CreatureStaticFlags3::FOREVER_CORPSE_DURATION))
            m_corpseExpirationTime = GetMap()->GetCurrentClockTime() + std::chrono::hours(24*7);
        else
            m_corpseExpirationTime = GetMap()->GetCurrentClockTime() + std::chrono::seconds(m_corpseDelay); // the max/default time for corpse decay (before creature is looted/AllLootRemovedFromCorpse() is called)
        m_respawnTime = time(nullptr) + m_respawnDelay; // respawn delay (spawntimesecs)

        // always save boss respawn time at death to prevent crash cheating
        if (sWorld.getConfig(CONFIG_BOOL_SAVE_RESPAWN_TIME_IMMEDIATELY) || IsWorldBoss())
            SaveRespawnTime();

        if (IsUsingNewSpawningSystem())
        {
            m_respawnTime = std::numeric_limits<time_t>::max();
            if (m_respawnDelay && s == JUST_DIED && !GetCreatureGroup())
                GetMap()->GetSpawnManager().AddCreature(GetDbGuid());
        }
    }

    Unit::SetDeathState(s);

    if (s == JUST_DIED)
    {
        SetTargetGuid(ObjectGuid());                        // remove target selection in any cases (can be set at aura remove in Unit::SetDeathState)
        SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        Unmount();
        ClearCreatureGroup();

        if (GetUInt32Value(UNIT_CREATED_BY_SPELL)) // optimization
            if (Unit* owner = GetOwner())
                StartCooldown(owner);

        Unit::SetDeathState(CORPSE);
    }

    if (s == JUST_ALIVED)
    {
        clearUnitState(static_cast<uint32>(UNIT_STAT_ALL_STATE));

        Unit::SetDeathState(ALIVE);

        SetWalk(true, true);
        ResetEntry(true);

        m_killer = ObjectGuid();

        ResetSpellHitCounter();

        if (GetSettings().HasFlag(CreatureStaticFlags::CREATOR_LOOT))
            SetLootRecipient(GetCreator());
        else
            SetLootRecipient(nullptr);
        if (GetTemporaryFactionFlags() & TEMPFACTION_RESTORE_RESPAWN)
            ClearTemporaryFaction();

        SetMeleeDamageSchool(SpellSchools(GetCreatureInfo()->DamageSchool));

        // Dynamic flags may be adjusted by spells. Clear them
        // first and let spell from *addon apply where needed.
        SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_NONE);
        LoadCreatureAddon(true);

        // Flags after LoadCreatureAddon. Any spell in *addon
        // will not be able to adjust these.
        SetUInt32Value(UNIT_NPC_FLAGS, GetCreatureInfo()->NpcFlags);
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

        i_motionMaster.Initialize();
    }
}

void Creature::Respawn()
{
    RemoveCorpse();
    if (!IsInWorld())                                       // Could be removed as part of a pool (in which case respawn-time is handled with pool-system)
        return;

    if (IsDespawned())
    {
        if (HasStaticDBSpawnData())
            GetMap()->GetPersistentState()->SaveCreatureRespawnTime(GetDbGuid(), 0);
        m_respawnTime = time(nullptr);                         // respawn at next tick
    }
}

void Creature::ForcedDespawn(uint32 timeMSToDespawn, bool onlyAlive)
{
    if (timeMSToDespawn)
    {
        ForcedDespawnDelayEvent* pEvent = new ForcedDespawnDelayEvent(*this, onlyAlive);

        m_events.AddEvent(pEvent, m_events.CalculateTime(timeMSToDespawn));
        return;
    }

    if (IsDespawned())
        return;

    if (IsAlive())
        SetDeathState(JUST_DIED);

    RemoveCorpse(true);                                     // force corpse removal in the same grid

    SetHealth(0);                                           // just for nice GM-mode view
}

bool Creature::IsImmuneToSpell(SpellEntry const* spellInfo, bool castOnSelf, uint8 effectMask, WorldObject const* caster)
{
    if (!spellInfo)
        return false;

    if (!castOnSelf)
    {
        if (GetCreatureInfo()->MechanicImmuneMask & (1 << (spellInfo->Mechanic - 1)))
            return true;

        if (GetCreatureInfo()->SchoolImmuneMask & (1 << spellInfo->School))
            return true;
    }

    return Unit::IsImmuneToSpell(spellInfo, castOnSelf, effectMask, caster);
}

bool Creature::IsImmuneToDamage(SpellSchoolMask meleeSchoolMask)
{
    if (GetCreatureInfo()->SchoolImmuneMask & meleeSchoolMask)
        return true;

    return Unit::IsImmuneToDamage(meleeSchoolMask);
}

bool Creature::IsImmuneToSpellEffect(SpellEntry const* spellInfo, SpellEffectIndex index, bool castOnSelf) const
{
    if (!castOnSelf && GetCreatureInfo()->MechanicImmuneMask & (1 << (spellInfo->EffectMechanic[index] - 1)))
        return true;

    return Unit::IsImmuneToSpellEffect(spellInfo, index, castOnSelf);
}

SpellEntry const* Creature::ReachWithSpellAttack(Unit* pVictim)
{
    if (!pVictim)
        return nullptr;

    for (auto& data : m_spellList.Spells)
    {
        uint32 spellId = data.second.SpellId;
        if (!spellId)
            continue;
        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
        if (!spellInfo)
        {
            sLog.outError("WORLD: unknown spell id %i", spellId);
            continue;
        }

        bool bcontinue = true;
        for (unsigned int j : spellInfo->Effect)
        {
            if ((j == SPELL_EFFECT_SCHOOL_DAMAGE)       ||
                    (j == SPELL_EFFECT_INSTAKILL)            ||
                    (j == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE) ||
                    (j == SPELL_EFFECT_HEALTH_LEECH)
               )
            {
                bcontinue = false;
                break;
            }
        }
        if (bcontinue) continue;

        if (spellInfo->manaCost > GetPower(POWER_MANA))
            continue;
        SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);
        float range = GetSpellMaxRange(srange);
        float minrange = GetSpellMinRange(srange);

        float dist = GetDistance(pVictim, true, spellInfo->rangeIndex == SPELL_RANGE_IDX_COMBAT ? DIST_CALC_COMBAT_REACH_WITH_MELEE : DIST_CALC_COMBAT_REACH);

        // if(!isInFront( pVictim, range ) && spellInfo->AttributesEx )
        //    continue;
        if (dist > range || dist < minrange)
            continue;
        if (spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
            continue;
        if (spellInfo->PreventionType == SPELL_PREVENTION_TYPE_PACIFY && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED))
            continue;
        return spellInfo;
    }
    return nullptr;
}

SpellEntry const* Creature::ReachWithSpellCure(Unit* pVictim)
{
    if (!pVictim)
        return nullptr;

    for (auto& data : m_spellList.Spells)
    {
        uint32 spellId = data.second.SpellId;
        if (!spellId)
            continue;
        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
        if (!spellInfo)
        {
            sLog.outError("WORLD: unknown spell id %i", spellId);
            continue;
        }

        bool bcontinue = true;
        for (uint32 j : spellInfo->Effect)
        {
            if (j == uint32(SPELL_EFFECT_HEAL))
            {
                bcontinue = false;
                break;
            }
        }
        if (bcontinue)
            continue;

        if (spellInfo->manaCost > GetPower(POWER_MANA))
            continue;
        SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);
        float range = GetSpellMaxRange(srange);
        float minrange = GetSpellMinRange(srange);

        float dist = GetDistance(pVictim, true, spellInfo->rangeIndex == SPELL_RANGE_IDX_COMBAT ? DIST_CALC_COMBAT_REACH_WITH_MELEE : DIST_CALC_COMBAT_REACH);

        // if(!isInFront( pVictim, range ) && spellInfo->AttributesEx )
        //    continue;
        if (dist > range || dist < minrange)
            continue;
        if (spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
            continue;
        if (spellInfo->PreventionType == SPELL_PREVENTION_TYPE_PACIFY && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED))
            continue;
        return spellInfo;
    }
    return nullptr;
}

bool Creature::IsVisibleInGridForPlayer(Player* pl) const
{
    // gamemaster in GM mode see all, including ghosts
    if (pl->IsGameMaster())
        return true;

    if (IsInvisible())
        return false;

    // Live player (or with not release body see live creatures or death creatures with corpse disappearing time > 0
    if (pl->IsAlive() || !pl->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
    {
        return (IsAlive() || !IsCorpseExpired());
    }

    // Dead player see live creatures near own corpse
    if (IsAlive())
    {
        Corpse* corpse = pl->GetCorpse();
        if (corpse)
        {
            // 20 - aggro distance for same level, 25 - max additional distance if player level less that creature level
            if (corpse->IsWithinDistInMap(this, (20 + 25)*sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_AGGRO)))
                return true;
        }
    }

    // Dead player can see ghosts
    if (GetCreatureInfo()->HasFlag(CreatureTypeFlags::VISIBLE_TO_GHOSTS))
        return true;

    // and not see any other
    return false;
}

void Creature::CallAssistance()
{
    CallAssistance(GetVictim());
}

void Creature::CallAssistance(Unit* enemy)
{
    // FIXME: should player pets call for assistance?
    if (!m_AlreadyCallAssistance && enemy && !HasCharmer())
    {
        MANGOS_ASSERT(AI());

        SetNoCallAssistance(true);

        if (!CanCallForAssistance())
            return;

        float radius = sWorld.getConfig(CONFIG_FLOAT_CREATURE_FAMILY_ASSISTANCE_RADIUS);
        if (GetCreatureInfo()->CallForHelp > 0)
            radius = GetCreatureInfo()->CallForHelp;
        AI()->SendAIEventAround(AI_EVENT_CALL_ASSISTANCE, enemy, sWorld.getConfig(CONFIG_UINT32_CREATURE_FAMILY_ASSISTANCE_DELAY), radius);
    }
}

void Creature::CallForHelp(float radius)
{
    if (!GetVictim() || IsPet() || HasCharmer())
        return;

    if (radius <= 0.0f)
        radius = GetCreatureInfo()->CallForHelp;

    MaNGOS::CallOfHelpCreatureInRangeDo u_do(this, GetVictim(), radius);
    MaNGOS::CreatureWorker<MaNGOS::CallOfHelpCreatureInRangeDo> worker(this, u_do);
    Cell::VisitGridObjects(this, worker, radius);
}

/// if enemy provided, check for initial combat help against enemy
bool Creature::CanAssistTo(const Unit* u, const Unit* enemy, bool checkfaction /*= true*/) const
{
    // we don't need help from zombies :)
    if (!IsAlive())
        return false;

    // we don't need help from non-combatant ;)
    if (IsNoAggroOnSight())
        return false;

    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING | UNIT_FLAG_UNINTERACTIBLE | UNIT_FLAG_IMMUNE_TO_NPC))
        return false;

    // skip fighting creature
    if (enemy && IsInCombat())
        return false;

    // only free creature
    if (GetMasterGuid())
        return false;

    // only from same creature faction
    if (checkfaction)
    {
        if (GetFaction() != u->GetFaction())
            return false;
    }
    else
    {
        if (CanAttack(u))
            return false;
    }

    // skip non hostile to caster enemy creatures
    if (enemy && CanAttack(enemy) && !IsEnemy(enemy))
        return false;

    return true;
}

bool Creature::CanInitiateAttack() const
{
    if (hasUnitState(UNIT_STAT_STUNNED | UNIT_STAT_FEIGN_DEATH))
        return false;

    if (!m_forceAttackingCapability && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING | UNIT_FLAG_UNINTERACTIBLE))
        return false;

    if (!CanAggro())
        return false;

    return true;
}

void Creature::SaveRespawnTime()
{
    if (IsPet() || !HasStaticDBSpawnData())
        return;

    if (m_respawnTime > time(nullptr) || IsUsingNewSpawningSystem()) // dead (no corpse)
        GetMap()->GetPersistentState()->SaveCreatureRespawnTime(GetDbGuid(), m_respawnTime);
    else if (!IsCorpseExpired())                               // dead (corpse)
        GetMap()->GetPersistentState()->SaveCreatureRespawnTime(GetDbGuid(), std::chrono::system_clock::to_time_t(m_corpseExpirationTime));
}

CreatureDataAddon const* Creature::GetCreatureAddon() const
{
    if (GetObjectGuid().GetHigh() != HIGHGUID_PET) // pets have guidlow that is conflicting with normal guidlows hence GetGUIDLow() gives wrong info
        if (CreatureDataAddon const* addon = ObjectMgr::GetCreatureAddon(GetDbGuid()))
            return addon;

    return ObjectMgr::GetCreatureTemplateAddon(GetCreatureInfo()->Entry);
}

// creature_addon table
bool Creature::LoadCreatureAddon(bool reload)
{
    CreatureDataAddon const* cainfo = GetCreatureAddon();
    if (!cainfo)
        return false;

    if (cainfo->mount != 0)
        Mount(cainfo->mount);

    if (cainfo->standState != 0)
        SetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_STAND_STATE, uint8(cainfo->standState));

    // UNIT_FIELD_BYTES_2
    // 0 SheathState
    // 1 Bytes2Flags, in 3.x used UnitPVPStateFlags, that have different meaning
    // 2 UnitRename         Pet only, so always 0 for default creature
    // 3 Unknown
    SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHEATH_STATE, cainfo->sheath_state);

    // SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, 0);
    // SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_UNKNOWN, 0);

    if (cainfo->emote != 0)
        SetUInt32Value(UNIT_NPC_EMOTESTATE, cainfo->emote);

    if (cainfo->auras)
    {
        for (uint32 const* cAura = cainfo->auras; *cAura; ++cAura)
        {
            if (HasAura(*cAura))
            {
                if (!reload)
                    sLog.outErrorDb("Creature (GUIDLow: %u Entry: %u) has spell %u in `auras` field, but aura is already applied.", GetGUIDLow(), GetEntry(), *cAura);

                continue;
            }

            CastSpell(this, *cAura, TRIGGERED_OLD_TRIGGERED);
        }
    }
    return true;
}

float Creature::GetNativeScale() const
{
    return GetCreatureInfo()->Scale;
}

/// Sends a message to LocalDefense and WorldDefense channels for players of the other team
void Creature::SendZoneUnderAttackMessage(Player* attacker) const
{
    sWorld.SendZoneUnderAttackMessage(GetZoneId(), attacker->GetTeam() == ALLIANCE ? HORDE : ALLIANCE);
}

void Creature::SetInCombatWithZone(bool checkAttackability)
{
    if (!CanHaveThreatList())
    {
        sLog.outError("Creature entry %u call SetInCombatWithZone but creature cannot have threat list.", GetEntry());
        return;
    }

    if (!AI())
    {
        sLog.outError("Creature entry %u call SetInCombatWithZone but creature does not have AI. Possible call during create.", GetEntry());
        return;
    }

    Map* pMap = GetMap();

    if (!pMap->IsDungeon())
    {
        sLog.outError("Creature entry %u call SetInCombatWithZone for map (id: %u) that isn't an instance.", GetEntry(), pMap->GetId());
        return;
    }

    Map::PlayerList const& PlList = pMap->GetPlayers();

    if (PlList.isEmpty())
        return;

    for (const auto& i : PlList)
    {
        if (Player* pPlayer = i.getSource())
        {
            if (pPlayer->IsGameMaster())
                continue;

            if (pPlayer->IsAlive())
            {
                if (checkAttackability && !CanAttack(pPlayer))
                    continue;

                EngageInCombatWith(pPlayer);
            }
        }
    }
}

bool Creature::HasSpell(uint32 spellID) const
{
    for (auto& spell : m_spellList.Spells)
        if (spell.second.SpellId == spellID)
            return true;

    return false;
}

void Creature::UpdateSpell(int32 index, int32 newSpellId)
{
    auto itr = m_spellList.Spells.find(index);
    if (itr != m_spellList.Spells.end())
    {
        if (newSpellId == 0)
            m_spellList.Spells.erase(itr);
        else
            (*itr).second.SpellId = newSpellId;
    }
}

void Creature::SetSpellList(uint32 spellSet)
{
    if (spellSet == 0)
    {
        m_spellList.Disabled = true;
        m_spellList.Spells.clear();

        if (AI())
            AI()->SpellListChanged();
        return;
    }

    // Try difficulty dependent version before falling back to base entry
    auto spellList = GetMap()->GetMapDataContainer().GetCreatureSpellList(spellSet);
    if (!spellList)
        return;
    
    m_spellList = *spellList;
    for (auto itr = m_spellList.Spells.begin(); itr != m_spellList.Spells.end();)
    {
        auto& spell = (*itr).second;
        if (spell.Availability < 100)
        {
            if (urand(0, 100) > spell.Availability) // such spells will not be available during this spell list lifetime
            {
                itr = m_spellList.Spells.erase(itr);
                continue;
            }
        }
        ++itr;
    }

    if (AI()) // might not yet be initialized - dealt with at init
        AI()->SpellListChanged();
}

void Creature::UpdateImmunitiesSet(uint32 immunitySet)
{
    if (m_immunitySet == immunitySet)
        return;

    auto set = sObjectMgr.GetCreatureImmunitySet(GetCreatureInfo()->Entry, immunitySet);
    if (!set)
        return;

    if (auto oldSet = sObjectMgr.GetCreatureImmunitySet(GetCreatureInfo()->Entry, m_immunitySet))
    {
        for (auto& data : *oldSet)
            ApplySpellImmune(nullptr, data.type, data.value, false);
    }

    for (auto& data : *set)
        ApplySpellImmune(nullptr, data.type, data.value, true);

    m_immunitySet = immunitySet;
}

time_t Creature::GetRespawnTimeEx() const
{
    time_t now = time(nullptr);
    if (m_respawnTime > now)                                // dead (no corpse)
        return m_respawnTime;
    else if (!IsCorpseExpired())                        // dead (corpse)
        return std::chrono::system_clock::to_time_t(m_corpseExpirationTime);
    else
        return now;
}
void Creature::GetRespawnCoord(float& x, float& y, float& z, float* ori, float* dist) const
{
    x = m_respawnPos.x;
    y = m_respawnPos.y;
    z = m_respawnPos.z;

    if (ori)
        *ori = m_respawnPos.o;

    if (dist)
        *dist = GetRespawnRadius();

    // lets check if our creatures have valid spawn coordinates
    MANGOS_ASSERT(MaNGOS::IsValidMapCoord(x, y, z) || PrintCoordinatesError(x, y, z, "respawn"));
}

void Creature::ResetRespawnCoord()
{
    if (GetObjectGuid().GetHigh() == HIGHGUID_PET)
        return;

    if (CreatureData const* data = sObjectMgr.GetCreatureData(GetDbGuid()))
    {
        m_respawnPos.x = data->posX;
        m_respawnPos.y = data->posY;
        m_respawnPos.z = data->posZ;
        m_respawnPos.o = data->orientation;
    }
}

uint32 Creature::GetLevelForTarget(Unit const* target) const
{
    if (!IsWorldBoss())
        return Unit::GetLevelForTarget(target);

    uint32 level = target->GetLevel() + sWorld.getConfig(CONFIG_UINT32_WORLD_BOSS_LEVEL_DIFF);
    if (level < 1)
        return 1;
    if (level > 255)
        return 255;
    return level;
}

std::string Creature::GetAIName() const
{
    return ObjectMgr::GetCreatureTemplate(GetEntry())->AIName;
}

std::string Creature::GetScriptName() const
{
    return sScriptDevAIMgr.GetScriptName(GetScriptId());
}

uint32 Creature::GetScriptId() const
{
    return ObjectMgr::GetCreatureTemplate(GetEntry())->ScriptID;
}

VendorItemData const* Creature::GetVendorItems() const
{
    return sObjectMgr.GetNpcVendorItemList(GetEntry());
}

VendorItemData const* Creature::GetVendorTemplateItems() const
{
    uint32 vendorId = GetCreatureInfo()->VendorTemplateId;
    return vendorId ? sObjectMgr.GetNpcVendorTemplateItemList(vendorId) : nullptr;
}

uint32 Creature::GetVendorItemCurrentCount(VendorItem const* vItem)
{
    if (!vItem->maxcount)
        return vItem->maxcount;

    VendorItemCounts::iterator itr = m_vendorItemCounts.begin();
    for (; itr != m_vendorItemCounts.end(); ++itr)
        if (itr->itemId == vItem->item)
            break;

    if (itr == m_vendorItemCounts.end())
        return vItem->maxcount;

    VendorItemCount* vCount = &*itr;

    time_t ptime = time(nullptr);

    if (vCount->lastIncrementTime + vItem->incrtime <= ptime)
    {
        ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(vItem->item);

        uint32 diff = uint32((ptime - vCount->lastIncrementTime) / vItem->incrtime);
        if ((vCount->count + diff * pProto->BuyCount) >= vItem->maxcount)
        {
            m_vendorItemCounts.erase(itr);
            return vItem->maxcount;
        }

        vCount->count += diff * pProto->BuyCount;
        vCount->lastIncrementTime = ptime;
    }

    return vCount->count;
}

uint32 Creature::UpdateVendorItemCurrentCount(VendorItem const* vItem, uint32 used_count)
{
    if (!vItem->maxcount)
        return 0;

    VendorItemCounts::iterator itr = m_vendorItemCounts.begin();
    for (; itr != m_vendorItemCounts.end(); ++itr)
        if (itr->itemId == vItem->item)
            break;

    if (itr == m_vendorItemCounts.end())
    {
        uint32 new_count = vItem->maxcount > used_count ? vItem->maxcount - used_count : 0;
        m_vendorItemCounts.push_back(VendorItemCount(vItem->item, new_count));
        return new_count;
    }

    VendorItemCount* vCount = &*itr;

    time_t ptime = time(nullptr);

    if (vCount->lastIncrementTime + vItem->incrtime <= ptime)
    {
        ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(vItem->item);

        uint32 diff = uint32((ptime - vCount->lastIncrementTime) / vItem->incrtime);
        if ((vCount->count + diff * pProto->BuyCount) < vItem->maxcount)
            vCount->count += diff * pProto->BuyCount;
        else
            vCount->count = vItem->maxcount;
    }

    vCount->count = vCount->count > used_count ? vCount->count - used_count : 0;
    vCount->lastIncrementTime = ptime;
    return vCount->count;
}

TrainerSpellData const* Creature::GetTrainerTemplateSpells() const
{
    uint32 trainerId = GetCreatureInfo()->TrainerTemplateId;
    return trainerId ? sObjectMgr.GetNpcTrainerTemplateSpells(trainerId) : nullptr;
}

TrainerSpellData const* Creature::GetTrainerSpells() const
{
    return sObjectMgr.GetNpcTrainerSpells(GetEntry());
}

// overwrite WorldObject function for proper name localization
const char* Creature::GetNameForLocaleIdx(int32 loc_idx) const
{
    char const* name = GetName();
    sObjectMgr.GetCreatureLocaleStrings(GetEntry(), loc_idx, &name);
    return name;
}

void Creature::SetFactionTemporary(uint32 factionId, uint32 tempFactionFlags)
{
    m_temporaryFactionFlags = tempFactionFlags;
    setFaction(factionId);

    ForceHealthAndPowerUpdate();                            // update health and power for client needed to hide enemy real value

    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_NON_ATTACKABLE)
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_IMMUNE_TO_PLAYER)
        SetImmuneToPlayer(false);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_IMMUNE_TO_NPC)
        SetImmuneToNPC(false);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_PACIFIED)
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_NOT_SELECTABLE)
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
}

void Creature::ClearTemporaryFaction()
{
    // No restore if creature is charmed/possessed.
    // For later we may consider extend to restore to charmer faction where charmer is creature.
    // This can also be done by update any pet/charmed of creature at any faction change to charmer.
    if (HasCharmer())
        return;

    // Reset to original faction
    setFaction(GetCreatureInfo()->Faction);

    ForceHealthAndPowerUpdate();                            // update health and power for client needed to hide enemy real value

    // Reset UNIT_FLAG_SPAWNING, UNIT_FLAG_IMMUNE_TO_PLAYER, UNIT_FLAG_IMMUNE_TO_NPC, UNIT_FLAG_PACIFIED or UNIT_FLAG_UNINTERACTIBLE flags
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_NON_ATTACKABLE && GetCreatureInfo()->UnitFlags & UNIT_FLAG_SPAWNING)
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SPAWNING);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_IMMUNE_TO_PLAYER && GetCreatureInfo()->UnitFlags & UNIT_FLAG_IMMUNE_TO_PLAYER)
        SetImmuneToPlayer(true);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_IMMUNE_TO_NPC && GetCreatureInfo()->UnitFlags & UNIT_FLAG_IMMUNE_TO_NPC)
        SetImmuneToNPC(true);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_PACIFIED && GetCreatureInfo()->UnitFlags & UNIT_FLAG_PACIFIED)
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_NOT_SELECTABLE && GetCreatureInfo()->UnitFlags & UNIT_FLAG_UNINTERACTIBLE)
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);

    m_temporaryFactionFlags = TEMPFACTION_NONE;
}

void Creature::SendAreaSpiritHealerQueryOpcode(Player* pl)
{
    uint32 next_resurrect = 0;
    if (Spell* pcurSpell = GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        next_resurrect = pcurSpell->GetCastedTime();
    WorldPacket data(SMSG_AREA_SPIRIT_HEALER_TIME, 8 + 4);
    data << ObjectGuid(GetObjectGuid());
    data << uint32(next_resurrect);
    pl->SendDirectMessage(data);
}

void Creature::ApplyGameEventSpells(GameEventCreatureData const* eventData, bool activated)
{
    uint32 cast_spell = activated ? eventData->spell_id_start : eventData->spell_id_end;
    uint32 remove_spell = activated ? eventData->spell_id_end : eventData->spell_id_start;

    if (remove_spell)
        if (SpellEntry const* spellEntry = sSpellTemplate.LookupEntry<SpellEntry>(remove_spell))
            if (IsSpellAppliesAura(spellEntry))
                RemoveAurasDueToSpell(remove_spell);

    if (cast_spell)
        CastSpell(this, cast_spell, TRIGGERED_OLD_TRIGGERED);
}

void Creature::FillGuidsListFromThreatList(GuidVector& guids, uint32 maxamount /*= 0*/)
{
    if (!CanHaveThreatList())
        return;

    ThreatList const& threats = getThreatManager().getThreatList();

    maxamount = maxamount > 0 ? std::min(maxamount, uint32(threats.size())) : threats.size();

    guids.reserve(guids.size() + maxamount);

    for (ThreatList::const_iterator itr = threats.begin(); maxamount && itr != threats.end(); ++itr, --maxamount)
        guids.push_back((*itr)->getUnitGuid());
}

bool Creature::HasStaticDBSpawnData() const
{
    if (GetObjectGuid().GetHigh() == HIGHGUID_PET)
        return false;
    return sObjectMgr.GetCreatureData(GetDbGuid()) != nullptr;
}

void Creature::SetVirtualItem(VirtualItemSlot slot, uint32 item_id)
{
    if (item_id == 0)
    {
        SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + slot, 0);
        SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + (slot * 2) + 0, 0);
        SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + (slot * 2) + 1, 0);
        return;
    }

    ItemPrototype const* proto = ObjectMgr::GetItemPrototype(item_id);
    if (!proto)
    {
        sLog.outError("Not listed in 'item_template' item (ID:%u) used as virtual item for %s", item_id, GetGuidStr().c_str());
        return;
    }

    SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + slot, proto->DisplayInfoID);
    SetByteValue(UNIT_VIRTUAL_ITEM_INFO + (slot * 2) + 0, VIRTUAL_ITEM_INFO_0_OFFSET_CLASS,    proto->Class);
    SetByteValue(UNIT_VIRTUAL_ITEM_INFO + (slot * 2) + 0, VIRTUAL_ITEM_INFO_0_OFFSET_SUBCLASS, proto->SubClass);
    SetByteValue(UNIT_VIRTUAL_ITEM_INFO + (slot * 2) + 0, VIRTUAL_ITEM_INFO_0_OFFSET_MATERIAL, proto->Material);
    SetByteValue(UNIT_VIRTUAL_ITEM_INFO + (slot * 2) + 0, VIRTUAL_ITEM_INFO_0_OFFSET_INVENTORYTYPE, proto->InventoryType);

    SetByteValue(UNIT_VIRTUAL_ITEM_INFO + (slot * 2) + 1, VIRTUAL_ITEM_INFO_1_OFFSET_SHEATH,        proto->Sheath);
}

bool Creature::hasWeapon(WeaponAttackType type) const
{
    const uint8 slot = uint8(type);
    const uint8 itemClass = GetByteValue(UNIT_VIRTUAL_ITEM_INFO + (slot * 2) + 0, VIRTUAL_ITEM_INFO_0_OFFSET_CLASS);

    return ((itemClass == ITEM_CLASS_WEAPON) || (type == OFF_ATTACK && CanDualWield()));
}

void Creature::SetWalk(bool enable, bool asDefault)
{
    if (asDefault)
    {
        if (enable)
            clearUnitState(UNIT_STAT_RUNNING);
        else
            addUnitState(UNIT_STAT_RUNNING);
    }

    // Nothing changed?
    if (enable == m_movementInfo.HasMovementFlag(MOVEFLAG_WALK_MODE))
        return;

    if (enable)
        m_movementInfo.AddMovementFlag(MOVEFLAG_WALK_MODE);
    else
        m_movementInfo.RemoveMovementFlag(MOVEFLAG_WALK_MODE);

    WorldPacket data(enable ? SMSG_SPLINE_MOVE_SET_WALK_MODE : SMSG_SPLINE_MOVE_SET_RUN_MODE, 9);
    data << GetPackGUID();
    SendMessageToSet(data, true);
}

void Creature::InspectingLoot()
{
    // until multiple corpse for creature is not supported
    // this will not have effect after re spawn delay (corpse will be removed anyway)
    if (m_settings.HasFlag(CreatureStaticFlags3::FOREVER_CORPSE_DURATION))
        return;

    // check if player have enough time to inspect loot
    if (m_corpseExpirationTime < GetMap()->GetCurrentClockTime() + std::chrono::milliseconds(m_corpseAccelerationDecayDelay))
        m_corpseExpirationTime = GetMap()->GetCurrentClockTime() + std::chrono::milliseconds(m_corpseAccelerationDecayDelay);
}

// reduce decay timer for corpse if need (for a corpse without loot)
void Creature::ReduceCorpseDecayTimer()
{
    if (!IsInWorld())
        return;

    if (m_settings.HasFlag(CreatureStaticFlags3::FOREVER_CORPSE_DURATION))
        return;

    if (m_corpseExpirationTime > GetMap()->GetCurrentClockTime() + std::chrono::milliseconds(m_corpseAccelerationDecayDelay))
        m_corpseExpirationTime = GetMap()->GetCurrentClockTime() + std::chrono::milliseconds(m_corpseAccelerationDecayDelay);  // 2 minutes for a creature
}

// Set loot status. Also handle remove corpse timer
void Creature::SetLootStatus(CreatureLootStatus status, bool forced)
{
    if (!forced && status <= m_lootStatus)
        return;

    m_lootStatus = status;
    switch (status)
    {
        case CREATURE_LOOT_STATUS_LOOTED:
            if (m_creatureInfo->SkinningLootId)
                SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
            else
            {
                // there is no loot so we can degrade corpse decay
                ReduceCorpseDecayTimer();
                RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
            }
            break;
        case CREATURE_LOOT_STATUS_SKINNED:
            m_corpseExpirationTime = TimePoint(); // remove corpse at next update
            RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
            RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
            break;
        case CREATURE_LOOT_STATUS_SKIN_AVAILABLE:
            SetFlag(UNIT_FIELD_FLAGS, UNIT_DYNFLAG_LOOTABLE);
            RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
            break;
        case CREATURE_LOOT_STATUS_PICKPOCKETED:
            StartPickpocketRestockTimer();
            break;
        default:
            break;
    }
}

// simple tap system return true if player or his group tapped the creature
// TODO:: this is semi correct. For group situation need more work but its not a big issue
bool Creature::IsTappedBy(Player* plr) const
{
    if (Player* recipient = GetLootRecipient())
    {
        if (recipient == plr)
            return true;

        if (Group* grp = recipient->GetGroup())
        {
            if (Group* plrGroup = plr->GetGroup())
            {
                if (plrGroup == grp)
                    return true;
            }
        }
        return false;
    }
    return false;
}

void Creature::SetBaseWalkSpeed(float speed)
{
    float newSpeed = speed;
    if (m_creatureInfo->SpeedWalk) // Creature template should still override
        newSpeed = m_creatureInfo->SpeedWalk;

    if (newSpeed != m_baseSpeedWalk)
    {
        m_baseSpeedWalk = newSpeed;
        UpdateSpeed(MOVE_WALK, false);
    }
}

void Creature::SetBaseRunSpeed(float speed, bool force)
{
    float newSpeed = speed;
    if (!force)
    {
        if (m_mountInfo && m_mountInfo->SpeedRun) // mount precedes everything
            newSpeed = m_mountInfo->SpeedRun;
        else if (m_creatureInfo->SpeedRun) // Creature template should still override
            newSpeed = m_creatureInfo->SpeedRun;
        else if (m_modelRunSpeed > 0)
            newSpeed = m_modelRunSpeed;
    }

    if (newSpeed != m_baseSpeedRun)
    {
        m_baseSpeedRun = newSpeed;
        UpdateSpeed(MOVE_RUN, false);
    }
}

void Creature::LockOutSpells(SpellSchoolMask schoolMask, uint32 duration)
{
    if (m_settings.HasFlag(CreatureStaticFlags2::NO_INTERRUPT_SCHOOL_COOLDOWN))
        return;

    WorldObject::LockOutSpells(schoolMask, duration);
}

void Creature::SetNoRewards()
{
    SetNoXP(true);
    SetNoLoot(true);
    SetNoReputation(true);
}

bool Creature::IsNoXp()
{
    return m_settings.HasFlag(CreatureStaticFlags::NO_XP);
}

void Creature::SetNoXP(bool state)
{
    if (state)
        m_settings.SetFlag(CreatureStaticFlags::NO_XP);
    else
        m_settings.RemoveFlag(CreatureStaticFlags::NO_XP);
}

bool Creature::IsNoLoot()
{
    return m_settings.HasFlag(CreatureStaticFlags::NO_LOOT);
}

void Creature::SetNoLoot(bool state)
{
    if (state)
        m_settings.SetFlag(CreatureStaticFlags::NO_LOOT);
    else
        m_settings.RemoveFlag(CreatureStaticFlags::NO_LOOT);
}

bool Creature::IsIgnoringFeignDeath() const
{
    return m_settings.HasFlag(CreatureStaticFlags2::IGNORE_FEIGN_DEATH);
}

void Creature::SetIgnoreFeignDeath(bool state)
{
    if (state)
        m_settings.SetFlag(CreatureStaticFlags2::IGNORE_FEIGN_DEATH);
    else
        m_settings.RemoveFlag(CreatureStaticFlags2::IGNORE_FEIGN_DEATH);
}

void Creature::SetNoWoundedSlowdown(bool state)
{
    if (state)
        m_settings.SetFlag(CreatureStaticFlags2::NO_WOUNDED_SLOWDOWN);
    else
        m_settings.RemoveFlag(CreatureStaticFlags2::NO_WOUNDED_SLOWDOWN);
}

bool Creature::IsNoWoundedSlowdown() const
{
    return m_settings.HasFlag(CreatureStaticFlags2::NO_WOUNDED_SLOWDOWN);
}

bool Creature::IsSlowedInCombat() const
{
    return !IsNoWoundedSlowdown() && HasAuraState(AURA_STATE_HEALTHLESS_20_PERCENT);
}

void Creature::SetNoWeaponSkillGain(bool state)
{
    if (state)
        m_settings.SetFlag(CreatureStaticFlags2::NO_SKILL_GAINS);
    else
        m_settings.RemoveFlag(CreatureStaticFlags2::NO_SKILL_GAINS);
}

bool Creature::IsNoWeaponSkillGain() const
{
    return m_settings.HasFlag(CreatureStaticFlags2::NO_SKILL_GAINS);
}

bool Creature::IsPreventingDeath() const
{
    return m_settings.HasFlag(CreatureStaticFlags::UNKILLABLE);
}

bool Creature::IsCorpseExpired() const
{
    auto now = GetMap()->GetCurrentClockTime();
    if (now >= m_corpseExpirationTime)
        return true;
    return false;
}

void Creature::UnsummonCleanup()
{
    CombatStop();
    RemoveAllAurasOnDeath();
    BreakCharmOutgoing();
    BreakCharmIncoming();
    RemoveGuardians(false);
    RemoveMiniPet();
    UnsummonAllTotems();
    InterruptNonMeleeSpells(false);
}

uint32 Creature::GetCreatureConditionalSpawnEntry(uint32 guidlow, Map* map) const
{
    uint32 entry = 0;

    /* ToDo: check for Battlefield or Outdoor pvp in the future */
    if (map->IsDungeon())
    {
        DungeonMap* dungeon = (DungeonMap*)map;
        if (!dungeon)
            return 0;

        Team currentTeam = dungeon->GetInstanceTeam();
        if (currentTeam != ALLIANCE && currentTeam != HORDE)
            return 0;

        CreatureConditionalSpawn const* spawn = ObjectMgr::GetCreatureConditionalSpawn(guidlow);
        if (!spawn)
            return 0;

        if (currentTeam == ALLIANCE)
            entry = spawn->EntryAlliance;
        else if (currentTeam == HORDE)
            entry = spawn->EntryHorde;
    }

    return entry;
}

void Creature::SetCanDualWield(bool state)
{
    Unit::SetCanDualWield(state);
    UpdateDamagePhysical(OFF_ATTACK);
}

Unit::MmapForcingStatus Creature::IsIgnoringMMAP() const
{
    if (m_ignoreMMAP)
        return MmapForcingStatus::IGNORED;

    if (GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_MMAP_FORCE_ENABLE)
        return MmapForcingStatus::FORCED;

    return Unit::IsIgnoringMMAP();
}

bool Creature::CanRestockPickpocketLoot() const
{
    return GetMap()->GetCurrentClockTime() >= m_pickpocketRestockTime;
}

void Creature::StartPickpocketRestockTimer()
{
    m_pickpocketRestockTime = GetMap()->GetCurrentClockTime() + std::chrono::milliseconds(sWorld.getConfig(CONFIG_UINT32_CREATURE_PICKPOCKET_RESTOCK_DELAY) * IN_MILLISECONDS);
}

bool Creature::HasBeenHitBySpell(uint32 spellId)
{
    return m_hitBySpells.find(spellId) != m_hitBySpells.end();
}

void Creature::RegisterHitBySpell(uint32 spellId)
{
    m_hitBySpells.insert(spellId);
}

void Creature::ResetSpellHitCounter()
{
    m_hitBySpells.clear();
}

void Creature::Heartbeat()
{
    Unit::Heartbeat();

    if (GetDetectionRange() > MAX_CREATURE_ATTACK_RADIUS && CanAggro())
        ScheduleAINotify(0);
}

void Creature::AddCooldown(SpellEntry const& spellEntry, ItemPrototype const* /*itemProto*/, bool permanent, uint32 forcedDuration, bool ignoreCat /*= false*/)
{
    uint32 recTime = forcedDuration ? forcedDuration : spellEntry.RecoveryTime;
    uint32 categoryRecTime = ignoreCat ? 0 : spellEntry.CategoryRecoveryTime;
    if (!forcedDuration)
    {
        uint32 cooldown = 0;
        Creature::CooldownResult result = GetSpellCooldown(spellEntry.Id, cooldown);
        bool success = result != COOLDOWN_RESULT_NOT_FOUND;
        if (!success)
            success = sObjectMgr.GetCreatureCooldown(GetCreatureInfo()->Entry, spellEntry.Id, cooldown);
        if (success && cooldown) // lets see if this will one day become a problem, if it does, add -1 -1 defaults to creature spell lists
        {
            if (result == COOLDOWN_RESULT_FOUND)
                recTime = cooldown;
            else if (result == COOLDOWN_RESULT_CATEGORY_FOUND)
                categoryRecTime = cooldown;
        }
    }
    if (recTime || categoryRecTime)
    {
        if (Player* modOwner = GetSpellModOwner())
        {
            if (recTime)
                modOwner->ApplySpellMod(spellEntry.Id, SPELLMOD_COOLDOWN, recTime);
            else if (spellEntry.Category && categoryRecTime)
                modOwner->ApplySpellMod(spellEntry.Id, SPELLMOD_COOLDOWN, categoryRecTime);
        }

        m_cooldownMap.AddCooldown(GetMap()->GetCurrentClockTime(), spellEntry.Id, recTime, spellEntry.Category, categoryRecTime, 0, permanent);
    }
    if ((recTime || categoryRecTime))
    {
        if (Player const* player = dynamic_cast<Player const*>(GetMaster()))
        {
            if (spellEntry.HasAttribute(SPELL_ATTR_COOLDOWN_ON_EVENT) && !permanent)
            {
                WorldPacket data(SMSG_COOLDOWN_EVENT, (4 + 8));
                data << uint32(spellEntry.Id);
                data << GetObjectGuid();
                player->GetSession()->SendPacket(data);
            }
            else
            {
                // send to client
                WorldPacket data(SMSG_SPELL_COOLDOWN, 8 + 1 + 4);
                data << GetObjectGuid();
                data << uint32(spellEntry.Id);
                data << uint32(recTime);
                player->GetSession()->SendPacket(data);
            }
        }
    }
}

void Creature::StartCooldown(Unit* owner)
{
    if (!m_imposedCooldown)
    {
        m_imposedCooldown = true;
        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(GetUInt32Value(UNIT_CREATED_BY_SPELL));
        // Remove infinity cooldown
        if (spellInfo && spellInfo->HasAttribute(SPELL_ATTR_COOLDOWN_ON_EVENT))
            owner->AddCooldown(*spellInfo);
    }
}
