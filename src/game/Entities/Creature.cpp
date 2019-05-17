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
#include "WorldPacket.h"
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
#include "Log.h"
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
#include "Util.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "Movement/MoveSplineInit.h"
#include "Entities/CreatureLinkingMgr.h"

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
    if (m_onlyAlive && !m_owner.isAlive())
        return true; // still successful, just not executed

    m_owner.ForcedDespawn();
    return true;
}

void CreatureCreatePos::SelectFinalPoint(Creature* cr)
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
    m_lootMoney(0), m_lootGroupRecipientId(0),
    m_lootStatus(CREATURE_LOOT_STATUS_NONE),
    m_respawnTime(0), m_respawnDelay(25), m_corpseDelay(60), m_canAggro(false),
    m_respawnradius(5.0f), m_subtype(subtype), m_defaultMovementType(IDLE_MOTION_TYPE),
    m_equipmentId(0), m_AlreadyCallAssistance(false),
    m_AlreadySearchedAssistance(false), m_isDeadByDefault(false),
    m_temporaryFactionFlags(TEMPFACTION_NONE),
    m_originalEntry(0), m_ai(nullptr),
    m_isInvisible(false), m_ignoreMMAP(false), m_forceAttackingCapability(false), m_ignoreRangedTargets(false), m_countSpawns(false),
    m_creatureInfo(nullptr),
    m_noXP(false), m_noLoot(false), m_noReputation(false)
{
    m_regenTimer = 200;
    m_valuesCount = UNIT_END;

    for (unsigned int& m_spell : m_spells)
        m_spell = 0;

    SetWalk(true, true);
}

Creature::~Creature()
{
    CleanupsBeforeDelete();
}

void Creature::CleanupsBeforeDelete()
{
    Unit::CleanupsBeforeDelete();
    m_vendorItemCounts.clear();
}

void Creature::AddToWorld()
{
    ///- Register the creature for guid lookup
    if (!IsInWorld() && GetObjectGuid().GetHigh() == HIGHGUID_UNIT)
        GetMap()->GetObjectsStore().insert<Creature>(GetObjectGuid(), (Creature*)this);

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

    // Check if visibility distance different
    if (GetCreatureInfo()->visibilityDistanceType != VisibilityDistanceType::Normal)
        SetVisibilityDistanceOverride(GetCreatureInfo()->visibilityDistanceType);

    if (m_countSpawns)
        GetMap()->AddToSpawnCount(GetObjectGuid());
}

void Creature::RemoveFromWorld()
{
    ///- Remove the creature from the accessor
    if (IsInWorld())
    {
        if (GetObjectGuid().GetHigh() == HIGHGUID_UNIT)
            GetMap()->GetObjectsStore().erase<Creature>(GetObjectGuid(), (Creature*)nullptr);

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
    }

    Unit::RemoveFromWorld();
}

void Creature::RemoveCorpse(bool inPlace)
{
    if (!inPlace && !IsInWorld())
       return;

    if ((getDeathState() != CORPSE && !m_isDeadByDefault) || (getDeathState() != ALIVE && m_isDeadByDefault))
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "Removing corpse of %s ", GetGuidStr().c_str());

    m_corpseExpirationTime = TimePoint();
    SetDeathState(DEAD);
    UpdateObjectVisibility();

    delete loot;
    loot = nullptr;
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
    SetByteValue(UNIT_FIELD_BYTES_0, 0, 0);

    // known valid are: CLASS_WARRIOR,CLASS_PALADIN,CLASS_ROGUE,CLASS_MAGE
    SetByteValue(UNIT_FIELD_BYTES_0, 1, uint8(cinfo->UnitClass));

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

    SetByteValue(UNIT_FIELD_BYTES_0, 2, minfo->gender);

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
    else if (!data || data->equipmentId == 0)
    {
        // use default from the template
        LoadEquipment(cinfo->EquipmentTemplateId);
    }
    else if (data && data->equipmentId != -1)
    {
        // override, -1 means no equipment
        LoadEquipment(data->equipmentId);
    }

    SetName(normalInfo->Name);                              // at normal entry always

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    SetLevitate((cinfo->InhabitType & INHABIT_AIR) != 0); // TODO: may not be correct to send opcode at this point (already handled by UPDATE_OBJECT createObject)

    // check if we need to add swimming movement. TODO: i thing movement flags should be computed automatically at each movement of creature so we need a sort of UpdateMovementFlags() method
    if (cinfo->InhabitType & INHABIT_WATER &&               // check inhabit type water
            !(cinfo->ExtraFlags & CREATURE_EXTRA_FLAG_WALK_IN_WATER) &&  // check if creature is forced to walk (crabs, giant,...)
            data &&                                         // check if there is data to get creature spawn pos
            GetMap()->GetTerrain()->IsSwimmable(data->posX, data->posY, data->posZ, minfo->bounding_radius))  // check if creature is in water and have enough space to swim
        m_movementInfo.AddMovementFlag(MOVEFLAG_SWIMMING);  // add swimming movement

    // checked at loading
    if (data)
        m_defaultMovementType = MovementGeneratorType(data->movementType);
    else
        m_defaultMovementType = MovementGeneratorType(cinfo->MovementType);

    SetMeleeDamageSchool(SpellSchools(cinfo->DamageSchool));

    SetCanParry(!(cinfo->ExtraFlags & CREATURE_EXTRA_FLAG_NO_PARRY));
    SetCanBlock(!(cinfo->ExtraFlags & CREATURE_EXTRA_FLAG_NO_BLOCK));
    SetCanDualWield((cinfo->ExtraFlags & CREATURE_EXTRA_FLAG_DUAL_WIELD_FORCED) || hasOffhandWeapon());
    SetForceAttackingCapability((cinfo->ExtraFlags & CREATURE_EXTRA_FLAG_FORCE_ATTACKING_CAPABILITY) != 0);
    SetNoXP((cinfo->ExtraFlags & CREATURE_EXTRA_FLAG_NO_XP_AT_KILL) != 0);
    SetNoLoot(false);
    SetNoReputation(false);

    return true;
}

bool Creature::UpdateEntry(uint32 Entry, const CreatureData* data /*=nullptr*/, GameEventCreatureData const* eventData /*=nullptr*/, bool preserveHPAndPower /*=true*/)
{
    if (!InitEntry(Entry, data, eventData))
        return false;

    // creatures always have melee weapon ready if any
    SetSheath(SHEATH_STATE_MELEE);
    SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_AURAS);

    if (preserveHPAndPower)
    {
        uint32 healthPercent = GetHealthPercent();
        SelectLevel();
        UpdateAllStats(); // to be sure stats is correct regarding level of the creature
        SetHealthPercent(healthPercent);
    }
    else
    {
        SelectLevel();
        UpdateAllStats(); // to be sure stats is correct regarding level of the creature
        if (data)
        {
            uint32 curhealth = data->curhealth ? data->curhealth : GetMaxHealth();
            SetHealth(m_deathState == ALIVE ? curhealth : 0);
            if (GetPowerType() == POWER_MANA)
            {
                uint32 curmana;
                if (IsRegeneratingPower()) // bypass so that 0 mana is possible TODO: change this to -1 in DB
                    curmana = data->curmana ? data->curmana : GetMaxPower(POWER_MANA);
                else
                    curmana = data->curmana;
                SetPower(POWER_MANA, curmana);
            }
        }
    }

    setFaction(GetCreatureInfo()->Faction);

    SetUInt32Value(UNIT_NPC_FLAGS, GetCreatureInfo()->NpcFlags);

    uint32 attackTimer = GetCreatureInfo()->MeleeBaseAttackTime;

    SetAttackTime(BASE_ATTACK,  attackTimer);
    SetAttackTime(OFF_ATTACK,   attackTimer - attackTimer / 4);
    SetAttackTime(RANGED_ATTACK, GetCreatureInfo()->RangedBaseAttackTime);

    uint32 unitFlags = GetCreatureInfo()->UnitFlags;

    // we may need to append or remove additional flags
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT))
        unitFlags |= UNIT_FLAG_IN_COMBAT;

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

    SetCanModifyStats(true);
    UpdateAllStats();

    // checked and error show at loading templates
    if (FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(GetCreatureInfo()->Faction))
    {
        if (factionTemplate->factionFlags & FACTION_TEMPLATE_FLAG_PVP)
            SetPvP(true);
    }

    // Try difficulty dependent version before falling back to base entry
    CreatureTemplateSpells const* templateSpells = sCreatureTemplateSpellsStorage.LookupEntry<CreatureTemplateSpells>(GetCreatureInfo()->Entry);
    if (!templateSpells)
        templateSpells = sCreatureTemplateSpellsStorage.LookupEntry<CreatureTemplateSpells>(GetEntry());
    if (templateSpells)
        for (int i = 0; i < CREATURE_MAX_SPELLS; ++i)
            m_spells[i] = templateSpells->spells[i];

    // if eventData set then event active and need apply spell_start
    if (eventData)
        ApplyGameEventSpells(eventData, true);

    return true;
}

void Creature::ResetEntry(bool respawn)
{
    CreatureData const* data = sObjectMgr.GetCreatureData(GetGUIDLow());
    GameEventCreatureData const* eventData = sGameEventMgr.GetCreatureUpdateDataForActiveEvent(GetGUIDLow());

    if (respawn)
    {
        auto spawnEntriesMap = sObjectMgr.GetCreatureSpawnEntry();
        auto itr = spawnEntriesMap.find(GetGUIDLow());
        if (itr != spawnEntriesMap.end())
        {
            auto& spawnList = (*itr).second;
            uint32 newEntry = spawnList[irand(0, spawnList.size() - 1)];
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
    if (data && data->modelid_override)
        return data->modelid_override;

    // use defaults from the template
    uint32 display_id = 0;

    // The follow decision tree needs to be updated if MAX_CREATURE_MODEL is changed.
    static_assert(MAX_CREATURE_MODEL == 4, "Need to update model selection code for new or removed model fields");

    // model selected here may be replaced with other_gender using own function
    if (!cinfo->ModelId[1])
    {
        display_id = cinfo->ModelId[0];
    }
    else if (!cinfo->ModelId[2])
    {
        display_id = cinfo->ModelId[urand(0, 1)];
    }
    else if (!cinfo->ModelId[3])
    {
        display_id = cinfo->ModelId[urand(0, 2)];
    }
    else
    {
        display_id = cinfo->ModelId[urand(0, 3)];
    }

    // fail safe, we use creature entry 1 and make error
    if (!display_id)
    {
        sLog.outErrorDb("Call customer support, ChooseDisplayId can not select native model for creature entry %u, model from creature entry 1 will be used instead.", cinfo->Entry);

        if (const CreatureInfo* creatureDefault = ObjectMgr::GetCreatureTemplate(1))
            display_id = creatureDefault->ModelId[0];
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
                delete loot;
                loot = nullptr;

                // Clear possible auras having IsDeathPersistent() attribute
                RemoveAllAuras();

                SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_NONE);
                if (m_isDeadByDefault)
                {
                    SetDeathState(JUST_DIED);
                    SetHealth(0);
                    i_motionMaster.Clear();
                    clearUnitState(static_cast<uint32>(UNIT_STAT_ALL_STATE));
                    LoadCreatureAddon(true);
                }
                else
                    SetDeathState(JUST_ALIVED);

                // Call AI respawn virtual function
                if (AI())
                    AI()->JustRespawned();

                if (m_isCreatureLinkingTrigger)
                    GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_RESPAWN, this);

                GetMap()->Add(this);

                if (uint16 poolid = sPoolMgr.IsPartOfAPool<Creature>(GetGUIDLow()))
                    sPoolMgr.UpdatePool<Creature>(*GetMap()->GetPersistentState(), poolid, GetGUIDLow());
            }
            break;
        }
        case CORPSE:
        {
            Unit::Update(diff);

            if (loot)
                loot->Update();

            if (!m_isDeadByDefault)
                if (IsCorpseExpired())
                    RemoveCorpse();

            break;
        }
        case ALIVE:
        {
            if (m_isDeadByDefault)
            {
                if (IsCorpseExpired())
                {
                    RemoveCorpse();
                    break;
                }
            }

            Unit::Update(diff);

            // Creature can be dead after unit update
            if (isAlive())
                RegenerateAll(diff);

            break;
        }
        default:
            break;
    }
}

void Creature::RegenerateAll(uint32 update_diff)
{
    if (m_regenTimer > 0)
    {
        if (update_diff >= m_regenTimer)
            m_regenTimer = 0;
        else
            m_regenTimer -= update_diff;
    }
    if (m_regenTimer != 0)
        return;

    if (!isInCombat() || IsEvadeRegen())
        RegenerateHealth();

    RegeneratePower();

    m_regenTimer = REGEN_TIME_FULL;
}

void Creature::RegeneratePower()
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
            if (isInCombat() || GetMasterGuid())
            {
                if (!IsUnderLastManaUseEffect())
                {
                    float ManaIncreaseRate = sWorld.getConfig(CONFIG_FLOAT_RATE_POWER_MANA);
                    float intellect = GetStat(STAT_INTELLECT);
                    addValue = sqrt(intellect) * OCTRegenMPPerSpirit() * ManaIncreaseRate;
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
            addValue += modifier->m_amount;
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

    uint32 addvalue = maxValue / 3;

    ModifyHealth(addvalue);
}

void Creature::DoFleeToGetAssistance() // TODO: split this into flee and assistance
{
    if (!getVictim())
        return;

    float radius = sWorld.getConfig(CONFIG_FLOAT_CREATURE_FAMILY_FLEE_ASSISTANCE_RADIUS);
    if (radius > 0)
    {
        Creature* pCreature = nullptr;

        MaNGOS::NearestAssistCreatureInCreatureRangeCheck u_check(this, getVictim(), radius);
        MaNGOS::CreatureLastSearcher<MaNGOS::NearestAssistCreatureInCreatureRangeCheck> searcher(pCreature, u_check);
        Cell::VisitGridObjects(this, searcher, radius);

        SetNoSearchAssistance(true);
        // UpdateSpeed(MOVE_RUN, false); [-ZERO] not needed?

        if (!pCreature)
            SetFeared(true, getVictim()->GetObjectGuid(), 0, sWorld.getConfig(CONFIG_UINT32_CREATURE_FAMILY_FLEE_DELAY));
        else
        {
            SetTargetGuid(ObjectGuid());        // creature flee loose its target
            GetMotionMaster()->MoveSeekAssistance(pCreature->GetPositionX(), pCreature->GetPositionY(), pCreature->GetPositionZ());
        }
    }
}

bool Creature::AIM_Initialize()
{
    /*
    // TODO:: implement this lock with proper guard when we'll support multimap or such
    // make sure nothing can change the AI during AI update
    if (m_AI_locked)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "AIM_Initialize: failed to init, locked.");
        return false;
    }*/

    i_motionMaster.Initialize();
    m_ai.reset(FactorySelector::selectAI(this));

    // Handle Spawned Events, also calls Reset()
    m_ai->JustRespawned();
    return true;
}

bool Creature::Create(uint32 guidlow, CreatureCreatePos& cPos, CreatureInfo const* cinfo, const CreatureData* data /*= nullptr*/, GameEventCreatureData const* eventData /*= nullptr*/)
{
    SetMap(cPos.GetMap());

    if (!CreateFromProto(guidlow, cinfo, data, eventData))
        return false;

    cPos.SelectFinalPoint(this);

    if (!cPos.Relocate(this))
        return false;

    // Notify the outdoor pvp script
    if (OutdoorPvP* outdoorPvP = sOutdoorPvPMgr.GetScript(GetZoneId()))
        outdoorPvP->HandleCreatureCreate(this);

    // Notify the map's instance data.
    // Only works if you create the object in it, not if it is moves to that map.
    // Normally non-players do not teleport to other maps.
    if (InstanceData* iData = GetMap()->GetInstanceData())
        iData->OnCreatureCreate(this);

    if (sObjectMgr.IsEncounter(GetEntry(), GetMapId()))
    {
        // encounter boss forced decay timer to 1h
        m_corpseDelay = 3600;                               // TODO: maybe add that to config file
    }
    else
    {
        switch (GetCreatureInfo()->Rank)
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
                    pPlayer->PlayerTalkClass->ClearMenus();
                    switch (GetCreatureInfo()->TrainerClass)
                    {
                        case CLASS_DRUID:  pPlayer->PlayerTalkClass->SendGossipMenu(4913, GetObjectGuid()); break;
                        case CLASS_HUNTER: pPlayer->PlayerTalkClass->SendGossipMenu(10090, GetObjectGuid()); break;
                        case CLASS_MAGE:   pPlayer->PlayerTalkClass->SendGossipMenu(328, GetObjectGuid()); break;
                        case CLASS_PALADIN: pPlayer->PlayerTalkClass->SendGossipMenu(1635, GetObjectGuid()); break;
                        case CLASS_PRIEST: pPlayer->PlayerTalkClass->SendGossipMenu(4436, GetObjectGuid()); break;
                        case CLASS_ROGUE:  pPlayer->PlayerTalkClass->SendGossipMenu(4797, GetObjectGuid()); break;
                        case CLASS_SHAMAN: pPlayer->PlayerTalkClass->SendGossipMenu(5003, GetObjectGuid()); break;
                        case CLASS_WARLOCK: pPlayer->PlayerTalkClass->SendGossipMenu(5836, GetObjectGuid()); break;
                        case CLASS_WARRIOR: pPlayer->PlayerTalkClass->SendGossipMenu(4985, GetObjectGuid()); break;
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
                    pPlayer->PlayerTalkClass->ClearMenus();
                    pPlayer->PlayerTalkClass->SendGossipMenu(3620, GetObjectGuid());
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
                    pPlayer->PlayerTalkClass->ClearMenus();
                    switch (GetCreatureInfo()->TrainerClass)
                    {
                        case RACE_DWARF:        pPlayer->PlayerTalkClass->SendGossipMenu(5865, GetObjectGuid()); break;
                        case RACE_GNOME:        pPlayer->PlayerTalkClass->SendGossipMenu(4881, GetObjectGuid()); break;
                        case RACE_HUMAN:        pPlayer->PlayerTalkClass->SendGossipMenu(5861, GetObjectGuid()); break;
                        case RACE_NIGHTELF:     pPlayer->PlayerTalkClass->SendGossipMenu(5862, GetObjectGuid()); break;
                        case RACE_ORC:          pPlayer->PlayerTalkClass->SendGossipMenu(5863, GetObjectGuid()); break;
                        case RACE_TAUREN:       pPlayer->PlayerTalkClass->SendGossipMenu(5864, GetObjectGuid()); break;
                        case RACE_TROLL:        pPlayer->PlayerTalkClass->SendGossipMenu(5816, GetObjectGuid()); break;
                        case RACE_UNDEAD:       pPlayer->PlayerTalkClass->SendGossipMenu(624, GetObjectGuid()); break;
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
                    pPlayer->PlayerTalkClass->ClearMenus();
                    pPlayer->PlayerTalkClass->SendGossipMenu(11031, GetObjectGuid());
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

    BattleGroundTypeId bgTypeId = sBattleGroundMgr.GetBattleMasterBG(GetEntry());
    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
        return false;

    if (!msg)
        return pPlayer->GetBGAccessByLevel(bgTypeId);

    if (!pPlayer->GetBGAccessByLevel(bgTypeId))
    {
        pPlayer->PlayerTalkClass->ClearMenus();
        switch (bgTypeId)
        {
            case BATTLEGROUND_AV:  pPlayer->PlayerTalkClass->SendGossipMenu(7616, GetObjectGuid()); break;
            case BATTLEGROUND_WS:  pPlayer->PlayerTalkClass->SendGossipMenu(7599, GetObjectGuid()); break;
            case BATTLEGROUND_AB:  pPlayer->PlayerTalkClass->SendGossipMenu(7642, GetObjectGuid()); break;
            default: break;
        }
        return false;
    }
    return true;
}

bool Creature::CanTrainAndResetTalentsOf(Player* pPlayer) const
{
    return pPlayer->getLevel() >= 10
           && GetCreatureInfo()->TrainerType == TRAINER_TYPE_CLASS
           && pPlayer->getClass() == GetCreatureInfo()->TrainerClass;
}

void Creature::PrepareBodyLootState()
{
    // loot may already exist (pickpocket case)
    delete loot;
    loot = nullptr;

    if (IsNoLoot())
        SetLootStatus(CREATURE_LOOT_STATUS_LOOTED);
    else
    {
        Player* killer = GetLootRecipient();

        if (killer)
            loot = new Loot(killer, this, LOOT_CORPSE);
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
    CreatureData const* data = sObjectMgr.GetCreatureData(GetGUIDLow());
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

        if (displayId != cinfo->ModelId[0] && displayId != cinfo->ModelId[1] &&
                displayId != cinfo->ModelId[2] && displayId != cinfo->ModelId[3])
        {
            for (int i = 0; i < MAX_CREATURE_MODEL && displayId; ++i)
                if (cinfo->ModelId[i])
                    if (CreatureModelInfo const* minfo = sObjectMgr.GetCreatureModelInfo(cinfo->ModelId[i]))
                        if (displayId == minfo->modelid_other_gender)
                            displayId = 0;
        }
        else
            displayId = 0;
    }

    // data->guid = guid don't must be update at save
    data.id = GetEntry();
    data.mapid = mapid;
    data.modelid_override = displayId;
    data.equipmentId = GetEquipmentId();
    data.posX = GetPositionX();
    data.posY = GetPositionY();
    data.posZ = GetPositionZ();
    data.orientation = GetOrientation();
    data.spawntimesecsmin = m_respawnDelay;
    data.spawntimesecsmax = m_respawnDelay;
    // prevent add data integrity problems
    data.spawndist = GetDefaultMovementType() == IDLE_MOTION_TYPE ? 0 : m_respawnradius;
    data.currentwaypoint = 0;
    data.curhealth = 0;
    data.curmana = 0;
    data.is_dead = m_isDeadByDefault;
    // prevent add data integrity problems
    data.movementType = !m_respawnradius && GetDefaultMovementType() == RANDOM_MOTION_TYPE
                        ? IDLE_MOTION_TYPE : GetDefaultMovementType();

    // updated in DB
    WorldDatabase.BeginTransaction();

    WorldDatabase.PExecuteLog("DELETE FROM creature WHERE guid=%u", GetGUIDLow());

    std::ostringstream ss;
    ss << "INSERT INTO creature VALUES ("
       << GetGUIDLow() << ","
       << data.id << ","
       << data.mapid << ","
       << data.modelid_override << ","
       << data.equipmentId << ","
       << data.posX << ","
       << data.posY << ","
       << data.posZ << ","
       << data.orientation << ","
       << data.spawntimesecsmin << ","                     // respawn time minimum
       << data.spawntimesecsmax << ","                     // respawn time maximum
       << (float) data.spawndist << ","                    // spawn distance (float)
       << data.currentwaypoint << ","                      // currentwaypoint
       << data.curhealth << ","                            // curhealth
       << data.curmana << ","                              // curmana
       << (data.is_dead  ? 1 : 0) << ","                   // is_dead
       << uint32(data.movementType) << ")";                // default movement generator type, cast to prevent save as symbol

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

    float damageMod = _GetDamageMod(rank);
    float damageMulti = cinfo->DamageMultiplier * damageMod;
    bool usedDamageMulti = false;

    if (CreatureClassLvlStats const* cCLS = sObjectMgr.GetCreatureClassLvlStats(level, cinfo->UnitClass))
    {
        // Use Creature Stats to calculate stat values

        // health
        if (cinfo->ArmorMultiplier >= 0)
        {
            health = cCLS->BaseHealth * cinfo->HealthMultiplier;
            mana = cCLS->BaseMana * cinfo->PowerMultiplier;
            armor = cCLS->BaseArmor * cinfo->ArmorMultiplier;
        }

        // damage
        if (cinfo->DamageMultiplier >= 0 && cinfo->ArmorMultiplier >= 0)
        {
            usedDamageMulti = true;
            mainMinDmg = ((cCLS->BaseDamage * cinfo->DamageVariance) + (cCLS->BaseMeleeAttackPower / 14.0f)) * (cinfo->MeleeBaseAttackTime / 1000.0f) * damageMulti;
            mainMaxDmg = ((cCLS->BaseDamage * cinfo->DamageVariance *1.5f) + (cCLS->BaseMeleeAttackPower / 14.0f)) * (cinfo->MeleeBaseAttackTime / 1000.0f) * damageMulti;
            offMinDmg = mainMinDmg / 2.0f;
            offMaxDmg = mainMaxDmg / 2.0f;
            minRangedDmg = ((cCLS->BaseDamage * cinfo->DamageVariance) + (cCLS->BaseRangedAttackPower / 14.0f)) * (cinfo->RangedBaseAttackTime / 1000.0f) * damageMulti;
            maxRangedDmg = ((cCLS->BaseDamage * cinfo->DamageVariance * 1.5f) + (cCLS->BaseRangedAttackPower / 14.0f)) * (cinfo->RangedBaseAttackTime / 1000.0f) * damageMulti;

            // attack power (not sure about the next line)
            meleeAttackPwr = cCLS->BaseMeleeAttackPower;
            rangedAttackPwr = cCLS->BaseRangedAttackPower;
        }
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
    SetHealth(health);

    SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, float(health));

    // all power types
    for (int i = POWER_MANA; i <= POWER_HAPPINESS; ++i)
    {
        uint32 maxValue = 0;

        switch (i)
        {
            case POWER_MANA:        maxValue = mana; break;
            case POWER_RAGE:        maxValue = 0; break;
            case POWER_FOCUS:       maxValue = POWER_FOCUS_DEFAULT; break;
            case POWER_ENERGY:      maxValue = POWER_ENERGY_DEFAULT * cinfo->PowerMultiplier; break;
            case POWER_HAPPINESS:   maxValue = POWER_HAPPINESS_DEFAULT; break;
        }

        uint32 value = maxValue;

        // For non regenerating powers set 0
        if (i == POWER_ENERGY && !IsRegeneratingPower())
            value = 0;

        // Mana requires an extra field to be set
        if (i == POWER_MANA)
            SetCreateMana(value);

        SetMaxPower(Powers(i), maxValue);
        SetPower(Powers(i), value);
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

bool Creature::CreateFromProto(uint32 guidlow, CreatureInfo const* cinfo, const CreatureData* data /*=nullptr*/, GameEventCreatureData const* eventData /*=nullptr*/)
{
    m_originalEntry = cinfo->Entry;

    uint32 newEntry = cinfo->Entry;

    Object::_Create(guidlow, newEntry, cinfo->GetHighGuid());

    auto spawnEntriesMap = sObjectMgr.GetCreatureSpawnEntry();
    auto itr = spawnEntriesMap.find(guidlow);
    if (itr != spawnEntriesMap.end())
    {
        auto& spawnList = (*itr).second;
        newEntry = spawnList[irand(0, spawnList.size() - 1)];
    }

    return UpdateEntry(newEntry, data, eventData, false);
}

bool Creature::LoadFromDB(uint32 guidlow, Map* map)
{
    CreatureData const* data = sObjectMgr.GetCreatureData(guidlow);

    if (!data)
    {
        sLog.outErrorDb("Creature (GUID: %u) not found in table `creature`, can't load. ", guidlow);
        return false;
    }

    uint32 entry = data->id;

    // get data for dual spawn instances
    if (entry == 0)
        entry = GetCreatureConditionalSpawnEntry(guidlow, map);

    if (!entry)
        return false;

    CreatureInfo const* cinfo = ObjectMgr::GetCreatureTemplate(entry);
    if (!cinfo)
    {
        sLog.outErrorDb("Creature (Entry: %u) not found in table `creature_template`, can't load. ", entry);
        return false;
    }

    GameEventCreatureData const* eventData = sGameEventMgr.GetCreatureUpdateDataForActiveEvent(guidlow);

    // Creature can be loaded already in map if grid has been unloaded while creature walk to another grid
    if (map->GetCreature(cinfo->GetObjectGuid(guidlow)))
        return false;

    CreatureCreatePos pos(map, data->posX, data->posY, data->posZ, data->orientation);

    if (!Create(guidlow, pos, cinfo, data, eventData))
        return false;

    SetRespawnCoord(pos);
    m_respawnradius = data->spawndist;

    m_respawnDelay = data->GetRandomRespawnTime();
    m_corpseDelay = std::min(m_respawnDelay * 9 / 10, m_corpseDelay); // set corpse delay to 90% of the respawn delay
    m_isDeadByDefault = data->is_dead;
    m_deathState = m_isDeadByDefault ? DEAD : ALIVE;

    m_respawnTime  = map->GetPersistentState()->GetCreatureRespawnTime(GetGUIDLow());

    if (m_respawnTime > time(nullptr))                         // not ready to respawn
    {
        m_deathState = DEAD;
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

        GetMap()->GetPersistentState()->SaveCreatureRespawnTime(GetGUIDLow(), 0);
    }

    if (sCreatureLinkingMgr.IsSpawnedByLinkedMob(this))
    {
        m_isSpawningLinked = true;
        if (m_deathState == ALIVE && !GetMap()->GetCreatureLinkingHolder()->CanSpawn(this))
        {
            m_deathState = DEAD;

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

    // Creature Linking, Initial load is handled like respawn
    if (m_isCreatureLinkingTrigger && isAlive())
        GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_RESPAWN, this);

    // check if it is rabbit day
    if (isAlive() && sWorld.getConfig(CONFIG_UINT32_RABBIT_DAY))
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
    WorldDatabase.PExecuteLog("DELETE FROM creature_movement WHERE id=%u", lowguid);
    WorldDatabase.PExecuteLog("DELETE FROM game_event_creature WHERE guid=%u", lowguid);
    WorldDatabase.PExecuteLog("DELETE FROM game_event_creature_data WHERE guid=%u", lowguid);
    WorldDatabase.PExecuteLog("DELETE FROM creature_battleground WHERE guid=%u", lowguid);
    WorldDatabase.PExecuteLog("DELETE FROM creature_linking WHERE guid=%u OR master_guid=%u", lowguid, lowguid);
    WorldDatabase.CommitTransaction();
}

void Creature::SetDeathState(DeathState s)
{
    if ((s == JUST_DIED && !m_isDeadByDefault) || (s == JUST_ALIVED && m_isDeadByDefault))
    {
        if (CreatureData const* data = sObjectMgr.GetCreatureData(GetGUIDLow()))
            m_respawnDelay = data->GetRandomRespawnTime();

        m_corpseExpirationTime = GetMap()->GetCurrentClockTime() + std::chrono::milliseconds(m_corpseDelay * IN_MILLISECONDS); // the max/default time for corpse decay (before creature is looted/AllLootRemovedFromCorpse() is called)
        m_respawnTime = time(nullptr) + m_respawnDelay; // respawn delay (spawntimesecs)

        // always save boss respawn time at death to prevent crash cheating
        if (sWorld.getConfig(CONFIG_BOOL_SAVE_RESPAWN_TIME_IMMEDIATELY) || IsWorldBoss())
            SaveRespawnTime();
    }

    Unit::SetDeathState(s);

    if (s == JUST_DIED)
    {
        SetTargetGuid(ObjectGuid());                        // remove target selection in any cases (can be set at aura remove in Unit::SetDeathState)
        SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);

        if (HasSearchedAssistance())
        {
            SetNoSearchAssistance(false);
            UpdateSpeed(MOVE_RUN, false);
        }

        if (CanFly())
            i_motionMaster.MoveFall();

        Unit::SetDeathState(CORPSE);
    }

    if (s == JUST_ALIVED)
    {
        clearUnitState(static_cast<uint32>(UNIT_STAT_ALL_STATE));

        Unit::SetDeathState(ALIVE);

        ResetEntry(true);

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

        SetWalk(true, true);
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
            GetMap()->GetPersistentState()->SaveCreatureRespawnTime(GetGUIDLow(), 0);
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

    if (isAlive())
        SetDeathState(JUST_DIED);

    RemoveCorpse(true);                                     // force corpse removal in the same grid

    SetHealth(0);                                           // just for nice GM-mode view
}

bool Creature::IsImmuneToSpell(SpellEntry const* spellInfo, bool castOnSelf, uint8 effectMask)
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

    return Unit::IsImmuneToSpell(spellInfo, castOnSelf, effectMask);
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

    if (!spellInfo->HasAttribute(SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY)) // spells like 37017 bypass these invulnerabilities
    {
        switch (spellInfo->Effect[index])
        {
            case SPELL_EFFECT_APPLY_AURA:
            {
                switch (spellInfo->EffectApplyAuraName[index])
                {
                    case SPELL_AURA_MOD_TAUNT: // Taunt immunity special flag check
                        if (GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_NOT_TAUNTABLE)
                            return true;
                        break;
                    case SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK: // Haste spell aura immunity
                        if (GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_HASTE_SPELL_IMMUNITY)
                            return true;
                        break;
                    case SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE:
                        if (IsWorldBoss()) // All bosses are immune to vindication in 2.4.3, needs to be setting in future
                            return true;
                        break;
                    default: break;
                }
                break;
            }
            case SPELL_EFFECT_ATTACK_ME: // Taunt immunity special flag check
                if (GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_NOT_TAUNTABLE)
                    return true;
                break;
            default: break;
        }
    }

    return Unit::IsImmuneToSpellEffect(spellInfo, index, castOnSelf);
}

SpellEntry const* Creature::ReachWithSpellAttack(Unit* pVictim)
{
    if (!pVictim)
        return nullptr;

    for (unsigned int m_spell : m_spells)
    {
        if (!m_spell)
            continue;
        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(m_spell);
        if (!spellInfo)
        {
            sLog.outError("WORLD: unknown spell id %i", m_spell);
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

    for (unsigned int m_spell : m_spells)
    {
        if (!m_spell)
            continue;
        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(m_spell);
        if (!spellInfo)
        {
            sLog.outError("WORLD: unknown spell id %i", m_spell);
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
    if (pl->isGameMaster())
        return true;

    if (IsInvisible())
        return false;

    // Live player (or with not release body see live creatures or death creatures with corpse disappearing time > 0
    if (pl->isAlive() || !pl->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
    {
        return (isAlive() || !IsCorpseExpired() || (m_isDeadByDefault && m_deathState == CORPSE));
    }

    // Dead player see live creatures near own corpse
    if (isAlive())
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
    if (GetCreatureInfo()->CreatureTypeFlags & CREATURE_TYPEFLAGS_GHOST_VISIBLE)
        return true;

    // and not see any other
    return false;
}

void Creature::CallAssistance()
{
    // FIXME: should player pets call for assistance?
    if (!m_AlreadyCallAssistance && getVictim() && !HasCharmer())
    {
        SetNoCallAssistance(true);

        if (GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_NO_CALL_ASSIST)
            return;

        AI()->SendAIEventAround(AI_EVENT_CALL_ASSISTANCE, getVictim(), sWorld.getConfig(CONFIG_UINT32_CREATURE_FAMILY_ASSISTANCE_DELAY), sWorld.getConfig(CONFIG_FLOAT_CREATURE_FAMILY_ASSISTANCE_RADIUS));
    }
}

void Creature::CallForHelp(float fRadius)
{
    if (fRadius <= 0.0f || !getVictim() || IsPet() || HasCharmer())
        return;

    MaNGOS::CallOfHelpCreatureInRangeDo u_do(this, getVictim(), fRadius);
    MaNGOS::CreatureWorker<MaNGOS::CallOfHelpCreatureInRangeDo> worker(this, u_do);
    Cell::VisitGridObjects(this, worker, fRadius);
}

/// if enemy provided, check for initial combat help against enemy
bool Creature::CanAssistTo(const Unit* u, const Unit* enemy, bool checkfaction /*= true*/) const
{
    // we don't need help from zombies :)
    if (!isAlive())
        return false;

    // we don't need help from non-combatant ;)
    if (IsNoAggroOnSight())
        return false;

    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_NPC))
        return false;

    // skip fighting creature
    if (enemy && isInCombat())
        return false;

    // only free creature
    if (GetMasterGuid())
        return false;

    // only from same creature faction
    if (checkfaction)
    {
        if (getFaction() != u->getFaction())
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

    if (!m_forceAttackingCapability && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE))
        return false;

    if (!CanAggro())
        return false;

    return true;
}

void Creature::SaveRespawnTime()
{
    if (IsPet() || !HasStaticDBSpawnData())
        return;

    if (m_respawnTime > time(nullptr))                         // dead (no corpse)
        GetMap()->GetPersistentState()->SaveCreatureRespawnTime(GetGUIDLow(), m_respawnTime);
    else if (!IsCorpseExpired())                               // dead (corpse)
        GetMap()->GetPersistentState()->SaveCreatureRespawnTime(GetGUIDLow(), std::chrono::system_clock::to_time_t(m_corpseExpirationTime));
}

CreatureDataAddon const* Creature::GetCreatureAddon() const
{
    if (!(GetObjectGuid().GetHigh() == HIGHGUID_PET)) // pets have guidlow that is conflicting with normal guidlows hence GetGUIDLow() gives wrong info
        if (CreatureDataAddon const* addon = ObjectMgr::GetCreatureAddon(GetGUIDLow()))
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

    if (cainfo->bytes1 != 0)
    {
        // 0 StandState
        // 1 LoyaltyLevel  Pet only, so always 0 for default creature
        // 2 ShapeshiftForm     Must be determined/set by shapeshift spell/aura
        // 3 StandMiscFlags

        SetByteValue(UNIT_FIELD_BYTES_1, 0, uint8(cainfo->bytes1 & 0xFF));
        // SetByteValue(UNIT_FIELD_BYTES_1, 1, uint8((cainfo->bytes1 >> 8) & 0xFF));
        // SetByteValue(UNIT_FIELD_BYTES_1, 1, 0);
        // SetByteValue(UNIT_FIELD_BYTES_2, 2, 0);
        SetByteValue(UNIT_FIELD_BYTES_1, 3, uint8((cainfo->bytes1 >> 24) & 0xFF));
    }

    // UNIT_FIELD_BYTES_2
    // 0 SheathState
    // 1 Bytes2Flags, in 3.x used UnitPVPStateFlags, that have different meaning
    // 2 UnitRename         Pet only, so always 0 for default creature
    // 3 ShapeshiftForm     Must be determined/set by shapeshift spell/aura
    SetByteValue(UNIT_FIELD_BYTES_2, 0, cainfo->sheath_state);

    if (cainfo->flags != 0)
        SetByteValue(UNIT_FIELD_BYTES_2, 1, cainfo->flags);

    // SetByteValue(UNIT_FIELD_BYTES_2, 2, 0);
    // SetByteValue(UNIT_FIELD_BYTES_2, 3, 0);

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
            if (pPlayer->isGameMaster())
                continue;

            if (pPlayer->isAlive())
            {
                if (checkAttackability && !CanAttack(pPlayer))
                    continue;

                AddThreat(pPlayer);
                SetInCombatWith(pPlayer);
                pPlayer->SetInCombatWith(this);
            }
        }
    }
}

bool Creature::MeetsSelectAttackingRequirement(Unit* pTarget, SpellEntry const* pSpellInfo, uint32 selectFlags, SelectAttackingTargetParams params) const
{
    if (selectFlags)
    {
        if ((selectFlags & SELECT_FLAG_PLAYER) && pTarget->GetTypeId() != TYPEID_PLAYER)
            return false;

        if ((selectFlags & SELECT_FLAG_POWER_MANA) && pTarget->GetPowerType() != POWER_MANA)
            return false;

        if ((selectFlags & SELECT_FLAG_POWER_RAGE) && pTarget->GetPowerType() != POWER_RAGE)
            return false;

        if ((selectFlags & SELECT_FLAG_POWER_ENERGY) && pTarget->GetPowerType() != POWER_ENERGY)
            return false;

        if ((selectFlags & SELECT_FLAG_IN_MELEE_RANGE) && !CanReachWithMeleeAttack(pTarget))
            return false;

        if ((selectFlags & SELECT_FLAG_NOT_IN_MELEE_RANGE) && CanReachWithMeleeAttack(pTarget))
            return false;

        if ((selectFlags & SELECT_FLAG_IN_LOS) && !IsWithinLOSInMap(pTarget))
            return false;

        if (!pTarget->isAlive())
            return false;

        if ((selectFlags & SELECT_FLAG_RANGE_RANGE))
        {
            float dist = GetDistance(pTarget, true, DIST_CALC_COMBAT_REACH);
            if (dist > params.range.maxRange || dist < params.range.minRange)
                return false;
        }

        if ((selectFlags & SELECT_FLAG_RANGE_AOE_RANGE))
        {
            float dist = pTarget->GetDistance(GetPositionX(), GetPositionY(), GetPositionZ(), DIST_CALC_COMBAT_REACH);
            if (dist > params.range.maxRange || dist < params.range.minRange)
                return false;
        }

        if ((selectFlags & SELECT_FLAG_POWER_NOT_MANA) && pTarget->GetPowerType() == POWER_MANA)
            return false;

        if ((selectFlags & SELECT_FLAG_SKIP_TANK) && pTarget == getVictim())
            return false;
    }

    if (pSpellInfo)
    {
        if (selectFlags & (SELECT_FLAG_HAS_AURA | SELECT_FLAG_NOT_AURA))
        {
            if (selectFlags & SELECT_FLAG_HAS_AURA)
            {
                if (!pTarget->HasAura(pSpellInfo->Id))
                    return false;
            }

            if (selectFlags & SELECT_FLAG_NOT_AURA)
            {
                if (pTarget->HasAura(pSpellInfo->Id))
                    return false;
            }

            return true;
        }

        if (pSpellInfo->HasAttribute(SPELL_ATTR_EX3_TARGET_ONLY_PLAYER) && pTarget->GetTypeId() != TYPEID_PLAYER)
            return false;

        switch (pSpellInfo->rangeIndex)
        {
            case SPELL_RANGE_IDX_ANYWHERE:  return true;
            case SPELL_RANGE_IDX_COMBAT:    return CanReachWithMeleeAttack(pTarget);
        }

        if (selectFlags & SELECT_FLAG_USE_EFFECT_RADIUS)
        {
            SpellRadiusEntry const* srange = sSpellRadiusStore.LookupEntry(pSpellInfo->EffectRadiusIndex[0]);
            float max_range = GetSpellRadius(srange);
            float dist = pTarget->GetDistance(GetPositionX(), GetPositionY(), GetPositionZ(), DIST_CALC_COMBAT_REACH);
            return dist < max_range;
        }
        else
        {
            SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(pSpellInfo->rangeIndex);
            float max_range = GetSpellMaxRange(srange);
            float min_range = GetSpellMinRange(srange);
            float dist = GetDistance(pTarget, true, DIST_CALC_COMBAT_REACH);
            return dist < max_range && dist >= min_range;
        }
    }

    return true;
}

Unit* Creature::SelectAttackingTarget(AttackingTarget target, uint32 position, uint32 spellId, uint32 selectFlags, SelectAttackingTargetParams params /*= SelectAttackingTargetParams()*/) const
{
    return SelectAttackingTarget(target, position, sSpellTemplate.LookupEntry<SpellEntry>(spellId), selectFlags, params);
}

Unit* Creature::SelectAttackingTarget(AttackingTarget target, uint32 position, SpellEntry const* spellInfo /*= nullptr*/, uint32 selectFlags/*= 0*/, SelectAttackingTargetParams params /*= SelectAttackingTargetParams()*/) const
{
    if (!CanHaveThreatList())
        return nullptr;

    ThreatList const& threatlist = getThreatManager().getThreatList();
    if (threatlist.empty() || position >= threatlist.size())
        return nullptr;

    ThreatList::const_iterator itr = threatlist.begin();

    switch (target)
    {
        case ATTACKING_TARGET_RANDOM:
        {
            std::vector<Unit*> suitableUnits;
            suitableUnits.reserve(threatlist.size() - position);
            if (position)
                advance(itr, position);

            for (; itr != threatlist.end(); ++itr)
            {
                if (Unit* pTarget = GetMap()->GetUnit((*itr)->getUnitGuid()))
                {
                    if ((!selectFlags && !spellInfo) || MeetsSelectAttackingRequirement(pTarget, spellInfo, selectFlags, params))
                        suitableUnits.push_back(pTarget);
                }
            }

            if (!suitableUnits.empty())
                return suitableUnits[urand(0, suitableUnits.size() - 1)];

            break;
        }
        case ATTACKING_TARGET_TOPAGGRO:
        {
            if (position)
                advance(itr, position);

            for (; itr != threatlist.end(); ++itr)
            {
                if (Unit* pTarget = GetMap()->GetUnit((*itr)->getUnitGuid()))
                {
                    if ((!selectFlags && !spellInfo) || MeetsSelectAttackingRequirement(pTarget, spellInfo, selectFlags, params))
                        return pTarget;
                }
            }

            break;
        }
        case ATTACKING_TARGET_BOTTOMAGGRO:
        {
            ThreatList::const_reverse_iterator ritr = threatlist.rbegin();

            if (position)
                advance(ritr, position);

            for (; ritr != threatlist.rend(); ++ritr)
            {
                if (Unit* pTarget = GetMap()->GetUnit((*ritr)->getUnitGuid()))
                {
                    if ((!selectFlags && !spellInfo) || MeetsSelectAttackingRequirement(pTarget, spellInfo, selectFlags, params))
                        return pTarget;
                }
            }

            break;
        }
        case ATTACKING_TARGET_NEAREST_BY:
        case ATTACKING_TARGET_FARTHEST_AWAY:
        {
            UnitList suitableUnits;

            for (; itr != threatlist.end(); ++itr)
            {
                if (Unit* pTarget = GetMap()->GetUnit((*itr)->getUnitGuid()))
                {
                    if ((!selectFlags && !spellInfo) || MeetsSelectAttackingRequirement(pTarget, spellInfo, selectFlags, params))
                        suitableUnits.push_back(pTarget);
                }
            }

            if (suitableUnits.empty() || position >= suitableUnits.size())
                return nullptr;

            if (suitableUnits.size() > 1)
            {
                if (target == ATTACKING_TARGET_NEAREST_BY)
                    suitableUnits.sort(TargetDistanceOrderNear(this));
                else
                    suitableUnits.sort(TargetDistanceOrderFarAway(this));
            }

            UnitList::iterator itr2 = suitableUnits.begin();
            if (position)
                std::advance(itr2, position);
            return *itr2;
        }
        case ATTACKING_TARGET_ALL_SUITABLE:
            //TODO:: This should not happen
            sLog.outError("Creature::SelectAttackingTarget> Target have unimplemented value 'ATTACKING_TARGET_ALL_SUITABLE'!");
            break;
        default:
            break;
    }

    return nullptr;
}

void Creature::SelectAttackingTargets(std::vector<Unit*>& selectedTargets, AttackingTarget target, uint32 position, uint32 spellId, uint32 selectFlags/*= 0*/, SelectAttackingTargetParams params /*= SelectAttackingTargetParams()*/) const
{
    SelectAttackingTargets(selectedTargets, target, position, sSpellTemplate.LookupEntry<SpellEntry>(spellId), selectFlags, params);
}

void Creature::SelectAttackingTargets(std::vector<Unit*>& selectedTargets, AttackingTarget target, uint32 position, SpellEntry const* spellInfo /*= nullptr*/, uint32 selectFlags/*= 0*/, SelectAttackingTargetParams params /*= SelectAttackingTargetParams()*/) const
{
    if (!CanHaveThreatList())
        return;

    ThreatList const& threatlist = getThreatManager().getThreatList();
    if (threatlist.empty() || position >= threatlist.size())
        return;

    ThreatList::const_iterator itr = threatlist.begin();

    switch (target)
    {
        case ATTACKING_TARGET_ALL_SUITABLE:
        {
            if (position)
                advance(itr, position);

            for (; itr != threatlist.end(); ++itr)
            {
                if (Unit* pTarget = GetMap()->GetUnit((*itr)->getUnitGuid()))
                {
                    if ((!selectFlags && !spellInfo) || MeetsSelectAttackingRequirement(pTarget, spellInfo, selectFlags, params))
                        selectedTargets.push_back(pTarget);
                }
            }
            break;
        }
        case ATTACKING_TARGET_RANDOM:
        case ATTACKING_TARGET_TOPAGGRO:
        case ATTACKING_TARGET_BOTTOMAGGRO:
        case ATTACKING_TARGET_NEAREST_BY:
        case ATTACKING_TARGET_FARTHEST_AWAY:
        default:
            //TODO:: This should not happen
            sLog.outError("Creature::SelectAttackingTarget> Target have unimplemented value!");
            break;
    }
}

bool Creature::HasSpell(uint32 spellID) const
{
    uint8 i;
    for (i = 0; i < CREATURE_MAX_SPELLS; ++i)
        if (spellID == m_spells[i])
            break;
    return i < CREATURE_MAX_SPELLS;                         // break before end of iteration of known spells
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
    if (CreatureData const* data = sObjectMgr.GetCreatureData(GetGUIDLow()))
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

    uint32 level = target->getLevel() + sWorld.getConfig(CONFIG_UINT32_WORLD_BOSS_LEVEL_DIFF);
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
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_IMMUNE_TO_PLAYER)
        SetImmuneToPlayer(false);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_IMMUNE_TO_NPC)
        SetImmuneToNPC(false);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_PACIFIED)
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_NOT_SELECTABLE)
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
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

    // Reset UNIT_FLAG_NON_ATTACKABLE, UNIT_FLAG_IMMUNE_TO_PLAYER, UNIT_FLAG_IMMUNE_TO_NPC, UNIT_FLAG_PACIFIED or UNIT_FLAG_NOT_SELECTABLE flags
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_NON_ATTACKABLE && GetCreatureInfo()->UnitFlags & UNIT_FLAG_NON_ATTACKABLE)
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_IMMUNE_TO_PLAYER && GetCreatureInfo()->UnitFlags & UNIT_FLAG_IMMUNE_TO_PLAYER)
        SetImmuneToPlayer(false);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_IMMUNE_TO_NPC && GetCreatureInfo()->UnitFlags & UNIT_FLAG_IMMUNE_TO_NPC)
        SetImmuneToNPC(false);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_PACIFIED && GetCreatureInfo()->UnitFlags & UNIT_FLAG_PACIFIED)
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    if (m_temporaryFactionFlags & TEMPFACTION_TOGGLE_NOT_SELECTABLE && GetCreatureInfo()->UnitFlags & UNIT_FLAG_NOT_SELECTABLE)
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

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

struct AddCreatureToRemoveListInMapsWorker
{
    AddCreatureToRemoveListInMapsWorker(ObjectGuid guid) : i_guid(guid) {}

    void operator()(Map* map)
    {
        if (Creature* pCreature = map->GetCreature(i_guid))
            pCreature->AddObjectToRemoveList();
    }

    ObjectGuid i_guid;
};

void Creature::AddToRemoveListInMaps(uint32 db_guid, CreatureData const* data)
{
    AddCreatureToRemoveListInMapsWorker worker(data->GetObjectGuid(db_guid));
    sMapMgr.DoForAllMapsWithMapId(data->mapid, worker);
}

struct SpawnCreatureInMapsWorker
{
    SpawnCreatureInMapsWorker(uint32 guid, CreatureData const* data)
        : i_guid(guid), i_data(data) {}

    void operator()(Map* map)
    {
        // We use spawn coords to spawn
        if (map->IsLoaded(i_data->posX, i_data->posY))
        {
            Creature* pCreature = new Creature;
            // DEBUG_LOG("Spawning creature %u",*itr);
            if (!pCreature->LoadFromDB(i_guid, map))
            {
                delete pCreature;
            }
        }
    }

    uint32 i_guid;
    CreatureData const* i_data;
};

void Creature::SpawnInMaps(uint32 db_guid, CreatureData const* data)
{
    SpawnCreatureInMapsWorker worker(db_guid, data);
    sMapMgr.DoForAllMapsWithMapId(data->mapid, worker);
}

bool Creature::HasStaticDBSpawnData() const
{
    return sObjectMgr.GetCreatureData(GetGUIDLow()) != nullptr;
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

void Creature::SetLevitate(bool enable)
{
    if (enable)
        m_movementInfo.AddMovementFlag(MOVEFLAG_LEVITATING);
    else
        m_movementInfo.RemoveMovementFlag(MOVEFLAG_LEVITATING);

    // TODO: there should be analogic opcode for 2.43
    // WorldPacket data(enable ? SMSG_SPLINE_MOVE_GRAVITY_DISABLE : SMSG_SPLINE_MOVE_GRAVITY_ENABLE, 9);
    // data << GetPackGUID();
    // SendMessageToSet(data, true);
}

void Creature::SetSwim(bool enable)
{
    if (enable)
        m_movementInfo.AddMovementFlag(MOVEFLAG_SWIMMING);
    else
        m_movementInfo.RemoveMovementFlag(MOVEFLAG_SWIMMING);

    WorldPacket data(enable ? SMSG_SPLINE_MOVE_START_SWIM : SMSG_SPLINE_MOVE_STOP_SWIM);
    data << GetPackGUID();
    SendMessageToSet(data, true);
}

void Creature::SetCanFly(bool enable)
{
//     TODO: check if there is something similar for 1.12.x (dragons and other flying NPCs)
//     if (enable)
//         m_movementInfo.AddMovementFlag(MOVEFLAG_CAN_FLY);
//     else
//         m_movementInfo.RemoveMovementFlag(MOVEFLAG_CAN_FLY);
//
//     WorldPacket data(enable ? SMSG_SPLINE_MOVE_SET_FLYING : SMSG_SPLINE_MOVE_UNSET_FLYING, 9);
//     data << GetPackGUID();
//     SendMessageToSet(data, true);
}

void Creature::SetFeatherFall(bool enable)
{
    if (enable)
        m_movementInfo.AddMovementFlag(MOVEFLAG_SAFE_FALL);
    else
        m_movementInfo.RemoveMovementFlag(MOVEFLAG_SAFE_FALL);

    WorldPacket data(enable ? SMSG_SPLINE_MOVE_FEATHER_FALL : SMSG_SPLINE_MOVE_NORMAL_FALL);
    data << GetPackGUID();
    SendMessageToSet(data, true);
}

void Creature::SetHover(bool enable)
{
    if (enable)
        m_movementInfo.AddMovementFlag(MOVEFLAG_HOVER);
    else
        m_movementInfo.RemoveMovementFlag(MOVEFLAG_HOVER);

    WorldPacket data(enable ? SMSG_SPLINE_MOVE_SET_HOVER : SMSG_SPLINE_MOVE_UNSET_HOVER, 9);
    data << GetPackGUID();
    SendMessageToSet(data, false);
}

void Creature::SetRoot(bool enable)
{
    if (enable)
        m_movementInfo.AddMovementFlag(MOVEFLAG_ROOT);
    else
        m_movementInfo.RemoveMovementFlag(MOVEFLAG_ROOT);

    WorldPacket data(enable ? SMSG_SPLINE_MOVE_ROOT : SMSG_SPLINE_MOVE_UNROOT, 9);
    data << GetPackGUID();
    SendMessageToSet(data, true);
}

void Creature::SetWaterWalk(bool enable)
{
    if (enable)
        m_movementInfo.AddMovementFlag(MOVEFLAG_WATERWALKING);
    else
        m_movementInfo.RemoveMovementFlag(MOVEFLAG_WATERWALKING);

    WorldPacket data(enable ? SMSG_SPLINE_MOVE_WATER_WALK : SMSG_SPLINE_MOVE_LAND_WALK, 9);
    data << GetPackGUID();
    SendMessageToSet(data, true);
}

void Creature::InspectingLoot()
{
    // until multiple corpse for creature is not supported
    // this will not have effect after re spawn delay (corpse will be removed anyway)

    // check if player have enough time to inspect loot
    if (m_corpseExpirationTime < GetMap()->GetCurrentClockTime() + std::chrono::milliseconds(MINIMUM_LOOTING_TIME))
        m_corpseExpirationTime = GetMap()->GetCurrentClockTime() + std::chrono::milliseconds(MINIMUM_LOOTING_TIME);
}

// reduce decay timer for corpse if need (for a corpse without loot)
void Creature::ReduceCorpseDecayTimer()
{
    if (!IsInWorld())
        return;

    bool isDungeonEncounter = false;
    if (GetMap()->IsDungeon())
    {
        if (sObjectMgr.IsEncounter(GetEntry(), GetMapId()))
            isDungeonEncounter = true;
    }

    if (!isDungeonEncounter && m_corpseExpirationTime > GetMap()->GetCurrentClockTime() + std::chrono::milliseconds(2 * MINUTE * IN_MILLISECONDS))
        m_corpseExpirationTime = GetMap()->GetCurrentClockTime() + std::chrono::milliseconds(2 * MINUTE * IN_MILLISECONDS);  // 2 minutes for a creature
}

// Set loot status. Also handle remove corpse timer
void Creature::SetLootStatus(CreatureLootStatus status)
{
    if (status <= m_lootStatus)
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

void Creature::SetBaseRunSpeed(float speed)
{
    float newSpeed = speed;
    if (m_creatureInfo->SpeedRun) // Creature template should still override
        newSpeed = m_creatureInfo->SpeedRun;

    if (newSpeed != m_baseSpeedRun)
    {
        m_baseSpeedRun = newSpeed;
        UpdateSpeed(MOVE_RUN, false);
    }
}

void Creature::LockOutSpells(SpellSchoolMask schoolMask, uint32 duration)
{
    if (GetCreatureInfo()->MechanicImmuneMask & (1 << (MECHANIC_SILENCE - 1)))
        return;

    WorldObject::LockOutSpells(schoolMask, duration);
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
    RemoveGuardians();
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
