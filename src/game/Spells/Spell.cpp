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

#include "Spells/Spell.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "Server/WorldSession.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Server/Opcodes.h"
#include "Log.h"
#include "World/World.h"
#include "Globals/ObjectMgr.h"
#include "Spells/SpellMgr.h"
#include "Entities/Player.h"
#include "Entities/Pet.h"
#include "Entities/Unit.h"
#include "Entities/DynamicObject.h"
#include "Groups/Group.h"
#include "Entities/UpdateData.h"
#include "Globals/ObjectAccessor.h"
#include "Grids/CellImpl.h"
#include "Policies/Singleton.h"
#include "Globals/SharedDefines.h"
#include "Loot/LootMgr.h"
#include "VMapFactory.h"
#include "BattleGround/BattleGround.h"
#include "Util.h"
#include "Chat/Chat.h"
#include "Server/SQLStorages.h"
#include "MotionGenerators/PathFinder.h"
#include "Entities/ObjectGuid.h"

extern pEffect SpellEffects[MAX_SPELL_EFFECTS];

bool IsQuestTameSpell(uint32 spellId)
{
    SpellEntry const* spellproto = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellproto)
        return false;

    return spellproto->Effect[EFFECT_INDEX_0] == SPELL_EFFECT_THREAT
           && spellproto->Effect[EFFECT_INDEX_1] == SPELL_EFFECT_APPLY_AURA && spellproto->EffectApplyAuraName[EFFECT_INDEX_1] == SPELL_AURA_DUMMY;
}

// SpellCastTargets class
// **********************

SpellCastTargets::SpellCastTargets()
{
    m_unitTarget = nullptr;
    m_itemTarget = nullptr;
    m_GOTarget   = nullptr;

    m_itemTargetEntry  = 0;

    m_srcX = m_srcY = m_srcZ = m_destX = m_destY = m_destZ = 0.0f;
    m_strTarget.clear();
    m_targetMask = 0;
}

SpellCastTargets::~SpellCastTargets()
{
}

void SpellCastTargets::setUnitTarget(Unit* target)
{
    if (!target)
        return;

    m_destX = target->GetPositionX();
    m_destY = target->GetPositionY();
    m_destZ = target->GetPositionZ();
    m_unitTarget = target;
    m_unitTargetGUID = target->GetObjectGuid();
    m_targetMask |= TARGET_FLAG_UNIT;
}

void SpellCastTargets::setDestination(float x, float y, float z)
{
    m_destX = x;
    m_destY = y;
    m_destZ = z;
    m_targetMask |= TARGET_FLAG_DEST_LOCATION;
}

void SpellCastTargets::setSource(float x, float y, float z)
{
    m_srcX = x;
    m_srcY = y;
    m_srcZ = z;
    m_targetMask |= TARGET_FLAG_SOURCE_LOCATION;
}

void SpellCastTargets::setGOTarget(GameObject* target)
{
    m_GOTarget = target;
    m_GOTargetGUID = target->GetObjectGuid();
    //    m_targetMask |= TARGET_FLAG_GAMEOBJECT;
}

void SpellCastTargets::setItemTarget(Item* item)
{
    if (!item)
        return;

    m_itemTarget = item;
    m_itemTargetGUID = item->GetObjectGuid();
    m_itemTargetEntry = item->GetEntry();
    m_targetMask |= TARGET_FLAG_ITEM;
}

void SpellCastTargets::setTradeItemTarget(Player* caster)
{
    m_itemTargetGUID = ObjectGuid(uint64(TRADE_SLOT_NONTRADED));
    m_itemTargetEntry = 0;
    m_targetMask |= TARGET_FLAG_TRADE_ITEM;

    Update(caster);
}

void SpellCastTargets::setCorpseTarget(Corpse* corpse)
{
    m_CorpseTarget = corpse;
    m_CorpseTargetGUID = corpse->GetObjectGuid();
}

void SpellCastTargets::Update(Unit* caster)
{
    m_GOTarget   = m_GOTargetGUID ? caster->GetMap()->GetGameObject(m_GOTargetGUID) : nullptr;
    m_unitTarget = m_unitTargetGUID ?
                   (m_unitTargetGUID == caster->GetObjectGuid() ? caster : ObjectAccessor::GetUnit(*caster, m_unitTargetGUID)) :
                   nullptr;

    m_itemTarget = nullptr;
    m_CorpseTarget = ObjectAccessor::GetCorpseInMap(m_CorpseTargetGUID, caster->GetMapId());
    if (caster->GetTypeId() == TYPEID_PLAYER)
    {
        Player* player = ((Player*)caster);

        if (m_targetMask & TARGET_FLAG_ITEM)
            m_itemTarget = player->GetItemByGuid(m_itemTargetGUID);
        else if (m_targetMask & TARGET_FLAG_TRADE_ITEM)
        {
            if (TradeData* pTrade = player->GetTradeData())
                if (m_itemTargetGUID.GetRawValue() < TRADE_SLOT_COUNT)
                    m_itemTarget = pTrade->GetTraderData()->GetItem(TradeSlots(m_itemTargetGUID.GetRawValue()));
        }

        if (m_itemTarget)
            m_itemTargetEntry = m_itemTarget->GetEntry();
    }
}

void SpellCastTargets::read(ByteBuffer& data, Unit* caster)
{
    data >> m_targetMask;

    if (m_targetMask == TARGET_FLAG_SELF)
    {
        m_destX = caster->GetPositionX();
        m_destY = caster->GetPositionY();
        m_destZ = caster->GetPositionZ();
        return;
    }

    // TARGET_FLAG_UNK2 is used for non-combat pets, maybe other?
    if (m_targetMask & (TARGET_FLAG_UNIT | TARGET_FLAG_UNIT_MINIPET | TARGET_FLAG_UNIT_ENEMY))
        data >> m_unitTargetGUID.ReadAsPacked();

    if (m_targetMask & (TARGET_FLAG_GAMEOBJECT | TARGET_FLAG_LOCKED))
        data >> m_GOTargetGUID.ReadAsPacked();

    if ((m_targetMask & (TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM)) && caster->GetTypeId() == TYPEID_PLAYER)
        data >> m_itemTargetGUID.ReadAsPacked();

    if (m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
    {
        data >> m_srcX >> m_srcY >> m_srcZ;
        if (!MaNGOS::IsValidMapCoord(m_srcX, m_srcY, m_srcZ))
            throw ByteBufferException(false, data.rpos(), 0, data.size());
    }

    if (m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        data >> m_destX >> m_destY >> m_destZ;
        if (!MaNGOS::IsValidMapCoord(m_destX, m_destY, m_destZ))
            throw ByteBufferException(false, data.rpos(), 0, data.size());
    }

    if (m_targetMask & TARGET_FLAG_STRING)
        data >> m_strTarget;

    if (m_targetMask & (TARGET_FLAG_CORPSE_ALLY | TARGET_FLAG_CORPSE_ENEMY))
        data >> m_CorpseTargetGUID.ReadAsPacked();

    // find real units/GOs
    Update(caster);
}

void SpellCastTargets::write(ByteBuffer& data) const
{
    data << uint16(m_targetMask);

    if (m_targetMask & (TARGET_FLAG_UNIT | TARGET_FLAG_CORPSE_ENEMY | TARGET_FLAG_GAMEOBJECT | TARGET_FLAG_CORPSE_ALLY | TARGET_FLAG_UNIT_MINIPET))
    {
        if (m_targetMask & TARGET_FLAG_UNIT)
        {
            if (m_unitTarget)
                data << m_unitTarget->GetPackGUID();
            else
                data << uint8(0);
        }
        else if (m_targetMask & (TARGET_FLAG_GAMEOBJECT | TARGET_FLAG_LOCKED))
        {
            if (m_GOTarget)
                data << m_GOTarget->GetPackGUID();
            else
                data << uint8(0);
        }
        else if (m_targetMask & (TARGET_FLAG_CORPSE_ALLY | TARGET_FLAG_CORPSE_ENEMY))
            data << m_CorpseTargetGUID.WriteAsPacked();
        else
            data << uint8(0);
    }

    if (m_targetMask & (TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM))
    {
        if (m_itemTarget)
            data << m_itemTarget->GetPackGUID();
        else
            data << uint8(0);
    }

    if (m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
        data << m_srcX << m_srcY << m_srcZ;

    if (m_targetMask & TARGET_FLAG_DEST_LOCATION)
        data << m_destX << m_destY << m_destZ;

    if (m_targetMask & TARGET_FLAG_STRING)
        data << m_strTarget;
}

// SpellLog class
// **************

void SpellLog::Initialize()
{
    m_spellLogData.Initialize(SMSG_SPELLLOGEXECUTE);
    m_spellLogData << m_spell->GetCaster()->GetPackGUID();
    m_spellLogData << uint32(m_spell->m_spellInfo->Id);
    m_spellLogDataEffectsCounterPos = m_spellLogData.wpos();
    m_spellLogData << uint32(0);                            //placeholder
    m_spellLogDataEffectsCounter = 0;
}

void SpellLog::FinalizePrevious()
{
    if (m_currentEffect < uint32(MAX_SPELL_EFFECTS))
    {
        // not first effect processed so we have to finalize the previous one by putting correct amount of targets
        m_spellLogData.put<uint32>(m_spellLogDataTargetsCounterPos, m_spellLogDataTargetsCounter);
        m_currentEffect = uint32(MAX_SPELL_EFFECTS);
    }
}

void SpellLog::SetCurrentEffect(uint32 effect)
{
    // check if its same effect so we don't have to put header
    if (effect == m_currentEffect)
    {
        ++m_spellLogDataTargetsCounter;
        return;
    }

    FinalizePrevious();

    m_spellLogData << effect;
    m_spellLogDataTargetsCounterPos = m_spellLogData.wpos();
    m_spellLogData << uint32(0);                            // placeholder for target counter
    m_currentEffect = effect;
    m_spellLogDataTargetsCounter = 1;
}

void SpellLog::SendToSet()
{
    if (!m_spellLogDataEffectsCounter)
        return;

    // check if one of previous target is not finalized
    FinalizePrevious();

    // put total effect counter in packet
    m_spellLogData.put<uint32>(m_spellLogDataEffectsCounterPos, m_spellLogDataEffectsCounter);
    m_spell->GetCaster()->SendMessageToSet(m_spellLogData, true);

    // make it ready for another log if need
    Initialize();
}

// Spell class
// ***********

Spell::Spell(Unit* caster, SpellEntry const* info, uint32 triggeredFlags, ObjectGuid originalCasterGUID, SpellEntry const* triggeredBy) :
    m_spellLog(this)
{
    MANGOS_ASSERT(caster != nullptr && info != nullptr);
    MANGOS_ASSERT(info == sSpellTemplate.LookupEntry<SpellEntry>(info->Id) && "`info` must be pointer to sSpellTemplate element");

    m_spellInfo = info;
    m_triggeredBySpellInfo = triggeredBy;
    m_caster = caster;
    m_referencedFromCurrentSpell = false;
    m_executedCurrently = false;
    m_delayStart = 0;
    m_delayAtDamageCount = 0;

    m_applyMultiplierMask = 0;

    // Get data for type of attack
    m_attackType = GetWeaponAttackType(m_spellInfo);

    m_spellSchoolMask = GetSpellSchoolMask(info);           // Can be override for some spell (wand shoot for example)

    // wand case
    if (m_attackType == RANGED_ATTACK)
        if (!!(m_caster->getClassMask() & CLASSMASK_WAND_USERS) && m_caster->GetTypeId() == TYPEID_PLAYER)
            m_spellSchoolMask = m_caster->GetRangedDamageSchoolMask();

    // Set health leech amount to zero
    m_healthLeech = 0;

    m_originalCasterGUID = originalCasterGUID ? originalCasterGUID : m_caster->GetObjectGuid();

    UpdateOriginalCasterPointer();

    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        m_currentBasePoints[i] = m_spellInfo->CalculateSimpleValue(SpellEffectIndex(i));

    m_spellState = SPELL_STATE_CREATED;

    m_castPositionX = m_castPositionY = m_castPositionZ = 0;
    m_TriggerSpells.clear();
    m_preCastSpells.clear();
    m_IsTriggeredSpell = triggeredFlags & TRIGGERED_OLD_TRIGGERED;
    // m_AreaAura = false;
    m_CastItem = nullptr;

    unitTarget = nullptr;
    itemTarget = nullptr;
    gameObjTarget = nullptr;
    focusObject = nullptr;
    m_triggeredByAuraSpell  = nullptr;
    m_spellAuraHolder = nullptr;

    // Auto Shot & Shoot (wand)
    m_autoRepeat = IsAutoRepeatRangedSpell(m_spellInfo);

    m_powerCost = 0;                                        // setup to correct value in Spell::prepare, don't must be used before.
    m_casttime = 0;                                         // setup to correct value in Spell::prepare, don't must be used before.
    m_timer = 0;                                            // will set to cast time in prepare
    m_duration = 0;
    m_maxRange = 0.f;

    m_needAliveTargetMask = 0;

    m_ignoreHitResult = (triggeredFlags & TRIGGERED_IGNORE_HIT_CALCULATION) != 0;
    m_ignoreUnselectableTarget = m_IsTriggeredSpell | ((triggeredFlags & TRIGGERED_IGNORE_UNSELECTABLE_FLAG) != 0);
    m_ignoreCastTime = (triggeredFlags & TRIGGERED_INSTANT_CAST) != 0;
    m_ignoreUnattackableTarget = (triggeredFlags & TRIGGERED_IGNORE_UNATTACKABLE_FLAG) != 0;
    m_triggerAutorepeat = (triggeredFlags & TRIGGERED_AUTOREPEAT) != 0;
    m_doNotProc = (triggeredFlags & TRIGGERED_DO_NOT_PROC) != 0;
    m_petCast = (triggeredFlags & TRIGGERED_PET_CAST) != 0;
    m_notifyAI = (triggeredFlags & TRIGGERED_NORMAL_COMBAT_CAST) != 0;

    m_reflectable = IsReflectableSpell(m_spellInfo);

    m_affectedTargetCount = GetAffectedTargets(m_spellInfo);

    m_scriptValue = 0;

    CleanupTargetList();

    m_spellLog.Initialize();
    m_needSpellLog = (m_spellInfo->Attributes & (SPELL_ATTR_HIDE_IN_COMBAT_LOG | SPELL_ATTR_HIDDEN_CLIENTSIDE)) == 0;

    m_targetlessMask = 0;
}

Spell::~Spell()
{
}

template<typename T>
WorldObject* Spell::FindCorpseUsing()
{
    // non-standard target selection
    SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex);
    float max_range = GetSpellMaxRange(srange);

    WorldObject* result = nullptr;

    T u_check(m_caster, max_range);
    MaNGOS::WorldObjectSearcher<T> searcher(result, u_check);

    Cell::VisitGridObjects(m_caster, searcher, max_range);

    if (!result)
        Cell::VisitWorldObjects(m_caster, searcher, max_range);

    return result;
}

void Spell::FillTargetMap()
{
    // TODO: ADD the correct target FILLS!!!!!!
    TempTargetingData targetingData;
    uint8 effToIndex[MAX_EFFECT_INDEX] = {0, 1, 2};         // Helper array, to link to another tmpUnitList, if the targets for both effects match
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        // not call for empty effect.
        // Also some spells use not used effect targets for store targets for dummy effect in triggered spells
        if (m_spellInfo->Effect[i] == SPELL_EFFECT_NONE)
            continue;

        auto& data = SpellTargetMgr::GetSpellTargetingData(m_spellInfo->Id);
        SpellTargetImplicitType effectTargetType = data.implicitType[i];
        auto& targetMask = data.targetMask[i];
        auto& ignoredTargets = data.ignoredTargets[i];

        if (effectTargetType == TARGET_TYPE_SPECIAL_UNIT) // area auras need custom handling
        {
            uint32 targetA = m_spellInfo->EffectImplicitTargetA[i];
            uint32 targetB = m_spellInfo->EffectImplicitTargetB[i];
            bool hadTarget = false;
            if (targetA && !ignoredTargets.first)
            {
                if (SpellTargetInfoTable[targetA].type == TARGET_TYPE_UNIT && SpellTargetInfoTable[targetA].enumerator == TARGET_ENUMERATOR_SINGLE)
                    hadTarget = true;
                SetTargetMap(SpellEffectIndex(i), targetA, false, targetingData);
            }
            if (targetB && !ignoredTargets.second)
            {
                if (SpellTargetInfoTable[targetB].type == TARGET_TYPE_UNIT && SpellTargetInfoTable[targetB].enumerator == TARGET_ENUMERATOR_SINGLE)
                    hadTarget = true;
                if (SpellTargetInfoTable[targetB].enumerator != TARGET_ENUMERATOR_AOE)
                    SetTargetMap(SpellEffectIndex(i), targetB, true, targetingData);
            }
            if (!hadTarget)
                targetingData.data[i].tmpUnitList[false].push_back(m_caster);
        }
        else
        {
            uint32 targetA = m_spellInfo->EffectImplicitTargetA[i];
            uint32 targetB = m_spellInfo->EffectImplicitTargetB[i];
            if (targetA == TARGET_NONE && targetB == TARGET_NONE)
            {
                if (m_spellInfo->Targets && SpellTargetMgr::CanEffectBeFilledWithMask(m_spellInfo->Id, i, m_spellInfo->Targets))
                {
                    if (m_spellInfo->Targets & (TARGET_FLAG_UNIT_ALLY | TARGET_FLAG_UNIT | TARGET_FLAG_UNIT_ENEMY))
                    {
                        if (Unit* unit = m_targets.getUnitTarget())
                            targetingData.data[i].tmpUnitList[false].push_back(unit);
                    }
                    else if (m_spellInfo->Targets & (TARGET_FLAG_CORPSE_ENEMY | TARGET_FLAG_CORPSE_ALLY))
                    {
                        if (Unit* unit = m_targets.getUnitTarget())
                            targetingData.data[i].tmpUnitList[false].push_back(unit);
                        else if (m_targets.getCorpseTargetGuid())
                        {
                            if (Corpse* corpse = m_targets.getCorpseTarget())
                                if (Player* owner = ObjectAccessor::FindPlayer(corpse->GetOwnerGuid()))
                                    targetingData.data[i].tmpUnitList[false].push_back(owner);
                        }
                    }
                    else if (m_spellInfo->Targets & (TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM))
                    {
                        if (Item* item = m_targets.getItemTarget())
                            targetingData.data[i].tempItemList.push_back(item);
                    }
                    else if (m_spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
                    {
                        if ((m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION) == 0)
                        {
                            sLog.outError("No destination for spell with flag TARGET_FLAG_DEST_LOCATION, spell ID: %u", m_spellInfo->Id); // should never occur
                            if (WorldObject* caster = GetCastingObject())
                                m_targets.setDestination(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ());
                        }
                    }
                }
                else if (uint32 defaultTarget = SpellEffectInfoTable[m_spellInfo->Effect[i]].defaultTarget)
                    SetTargetMap(SpellEffectIndex(i), defaultTarget, false, targetingData);
            }
            else
            {
                if (targetA && !ignoredTargets.first)
                    SetTargetMap(SpellEffectIndex(i), targetA, false, targetingData);
                if (targetB && !ignoredTargets.second)
                    SetTargetMap(SpellEffectIndex(i), targetB, true, targetingData);
            }
        }

        bool processedUnits = false; // TODO: implement adding of executionless targets which are not immune to SMSG_SPELL_GO
        bool processedGOs = false; // likely purpose - animations
        switch (effectTargetType)
        {
            case TARGET_TYPE_NONE:
                m_targetlessMask |= (1 << i);
                break;
            case TARGET_TYPE_LOCATION_DEST:
            case TARGET_TYPE_SPECIAL_DEST:
                AddDestExecution(SpellEffectIndex(i));
                break;
            case TARGET_TYPE_UNIT:
            case TARGET_TYPE_UNIT_DEST:
            case TARGET_TYPE_PLAYER: // for now player handled here
            case TARGET_TYPE_SPECIAL_UNIT:
            case TARGET_TYPE_CORPSE:
                processedUnits = true;
                for (uint8 rightTarget = 0; rightTarget < 2; ++rightTarget)
                {
                    UnitList& unitTargetList = targetingData.data[i].tmpUnitList[rightTarget];
                    uint8 effectMask = targetMask[rightTarget];
                    if (!unitTargetList.empty()) // Unit case
                    {
                        for (auto itr = unitTargetList.begin(); itr != unitTargetList.end();)
                        {
                            if (!CheckTarget(*itr, SpellEffectIndex(i), bool(rightTarget), CheckException(targetingData.magnet)))
                                itr = unitTargetList.erase(itr);
                            else
                                ++itr;
                        }

                        // Secial target filter before adding targets to list
                        FilterTargetMap(unitTargetList, SpellEffectIndex(i));

                        if (m_affectedTargetCount && unitTargetList.size() > m_affectedTargetCount)
                        {
                            // remove random units from the map
                            while (unitTargetList.size() > m_affectedTargetCount)
                            {
                                uint32 poz = urand(0, unitTargetList.size() - 1);
                                for (auto itr = unitTargetList.begin(); itr != unitTargetList.end(); ++itr, --poz)
                                {
                                    if (!*itr) continue;

                                    if (!poz)
                                    {
                                        itr = unitTargetList.erase(itr);
                                        break;
                                    }
                                }
                            }
                        }

                        if (m_caster->GetTypeId() == TYPEID_PLAYER)
                        {
                            Player* me = (Player*)m_caster;
                            for (auto itr = unitTargetList.begin(); itr != unitTargetList.end(); ++itr)
                            {
                                Player* targetOwner = (*itr)->GetBeneficiaryPlayer();
                                if (targetOwner && targetOwner != me && targetOwner->IsPvP() && !me->IsInDuelWith(targetOwner))
                                {
                                    me->UpdatePvP(true);
                                    me->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
                                    break;
                                }
                            }
                        }
                    }
                    for (Unit* unit : unitTargetList)
                        AddUnitTarget(unit, effectMask);
                }
                break;
            case TARGET_TYPE_LOCK:
            case TARGET_TYPE_ITEM:
                if (targetingData.data[i].tempItemList.size() > 0) // Item case
                    for (Item* item : targetingData.data[i].tempItemList)
                        AddItemTarget(item, targetMask[0]); // no spell in all of vanilla through wotlk has item target in B
                if (effectTargetType == TARGET_TYPE_ITEM)
                    break;
                // [[fallthrough]]
            case TARGET_TYPE_GAMEOBJECT:
                processedGOs = true;
                for (uint8 rightTarget = 0; rightTarget < 2; ++rightTarget)
                {
                    GameObjectList& goTargetList = targetingData.data[i].tmpGOList[rightTarget];
                    uint8 effectMask = targetMask[rightTarget];
                    if (!goTargetList.empty()) // GO case
                    {
                        if (m_affectedTargetCount && goTargetList.size() > m_affectedTargetCount)
                        {
                            // remove random units from the map
                            while (goTargetList.size() > m_affectedTargetCount)
                            {
                                uint32 poz = urand(0, goTargetList.size() - 1);
                                for (auto itr = goTargetList.begin(); itr != goTargetList.end(); ++itr, --poz)
                                {
                                    if (!*itr) continue;

                                    if (!poz)
                                    {
                                        goTargetList.erase(itr);
                                        break;
                                    }
                                }
                            }
                        }

                        for (auto iter = goTargetList.begin(); iter != goTargetList.end();)
                        {
                            if (CheckTargetGOScript(*iter, SpellEffectIndex(i)))
                                ++iter;
                            else
                                iter = goTargetList.erase(iter);
                        }

                        // Add resulting GOs as GOTargets
                        for (GameObject* go : goTargetList)
                            AddGOTarget(go, effectMask);
                    }
                }
                break;
        }
    }
}

void Spell::prepareDataForTriggerSystem()
{
    //==========================================================================================
    // Now fill data for trigger system, need know:
    // an spell trigger another or not ( m_canTrigger )
    // Create base triggers flags for Attacker and Victim ( m_procAttacker and  m_procVictim)
    //==========================================================================================
    // Fill flag can spell trigger or not
    // TODO: possible exist spell attribute for this
    m_canTrigger = false;

    if ((m_CastItem && m_spellInfo->SpellFamilyFlags == nullptr) || m_spellInfo->HasAttribute(SPELL_ATTR_EX3_CANT_TRIGGER_PROC) || m_doNotProc)
        m_canTrigger = false;                               // Do not trigger from item cast spell
    else if (!m_IsTriggeredSpell)
        m_canTrigger = true;                                // Normal cast - can trigger
    else if (!m_triggeredByAuraSpell)
        m_canTrigger = true;                                // Triggered from SPELL_EFFECT_TRIGGER_SPELL - can trigger
    else if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_TRIGGERED_CAN_TRIGGER_PROC) || m_spellInfo->HasAttribute(SPELL_ATTR_EX3_TRIGGERED_CAN_TRIGGER_SPECIAL))
        m_canTrigger = true;                                // Spells with these special attributes can trigger even if triggeredByAuraSpell

    if (!m_canTrigger)                                      // Exceptions (some periodic triggers)
    {
        switch (m_spellInfo->SpellFamilyName)
        {
            case SPELLFAMILY_MAGE:
                // Arcane Missiles / Blizzard triggers need do it
                if (m_spellInfo->IsFitToFamilyMask(uint64(0x0000000000200080)))
                    m_canTrigger = true;
                break;
            case SPELLFAMILY_WARLOCK:
                // For Hellfire Effect / Rain of Fire / Seed of Corruption triggers need do it
                if (m_spellInfo->IsFitToFamilyMask(uint64(0x0000800000000060)))
                    m_canTrigger = true;
                break;
            case SPELLFAMILY_HUNTER:
                // Hunter Explosive Trap Effect/Immolation Trap Effect/Frost Trap Aura/Snake Trap Effect
                if (m_spellInfo->IsFitToFamilyMask(uint64(0x0000200000000014)))
                    m_canTrigger = true;
                break;
            case SPELLFAMILY_PALADIN:
                // For Holy Shock triggers need do it
                if (m_spellInfo->IsFitToFamilyMask(uint64(0x0001000000200000)))
                    m_canTrigger = true;
                break;
            default:
                break;
        }
    }

    // some negative spells have positive effects to another or same targets
    // avoid triggering negative hit for only positive targets
    m_negativeEffectMask = 0x0;
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (!IsPositiveEffect(m_spellInfo, SpellEffectIndex(i)))
            m_negativeEffectMask |= (1 << i);
}

void Spell::PrepareMasksForProcSystem(uint8 effectMask, uint32& procAttacker, uint32& procVictim, WorldObject* caster, WorldObject* target)
{
    // Get data for type of attack and fill base info for trigger
    switch (m_spellInfo->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:
            procAttacker = PROC_FLAG_SUCCESSFUL_MELEE_SPELL_HIT;
            if (m_attackType == OFF_ATTACK)
                procAttacker |= PROC_FLAG_SUCCESSFUL_OFFHAND_HIT;
            procVictim = PROC_FLAG_TAKEN_MELEE_SPELL_HIT;
            break;
        case SPELL_DAMAGE_CLASS_RANGED:
            // Auto attack
            if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_AUTOREPEAT_FLAG))
            {
                procAttacker = PROC_FLAG_SUCCESSFUL_RANGED_HIT;
                procVictim = PROC_FLAG_TAKEN_RANGED_HIT;
            }
            else // Ranged spell attack
            {
                procAttacker = PROC_FLAG_SUCCESSFUL_RANGED_SPELL_HIT;
                procVictim = PROC_FLAG_TAKEN_RANGED_SPELL_HIT;
            }
            break;
        default:
            if (IsPositiveEffectMask(m_spellInfo, effectMask, caster, target))           // Check for positive spell
            {
                if (m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_NONE) // if dmg class none
                {
                    procAttacker = PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS;
                    procVictim = PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_POS;
                }
                else
                {
                    procAttacker = PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS;
                    procVictim = PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_POS;
                }
            }
            else if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_AUTOREPEAT_FLAG))   // Wands auto attack
            {
                procAttacker = PROC_FLAG_SUCCESSFUL_RANGED_HIT;
                procVictim = PROC_FLAG_TAKEN_RANGED_HIT;
            }
            else                                           // Negative spell
            {
                if (m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_NONE) // if dmg class none
                {
                    procAttacker = PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG;
                    procVictim = PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG;
                }
                else
                {
                    procAttacker = PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG;
                    procVictim = PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG;
                }
            }
            break;
    }

    // Hunter traps spells (for Entrapment trigger)
    // Gives your Immolation Trap, Frost Trap, Explosive Trap, and Snake Trap ....
    if (m_spellInfo->SpellFamilyName == SPELLFAMILY_HUNTER && m_spellInfo->SpellFamilyFlags & uint64(0x000020000000001C))
        procAttacker |= PROC_FLAG_ON_TRAP_ACTIVATION;

    if (IsNextMeleeSwingSpell())
    {
        procAttacker |= PROC_FLAG_SUCCESSFUL_MELEE_HIT;
        procVictim |= PROC_FLAG_TAKEN_MELEE_HIT;
    }
}

void Spell::CleanupTargetList()
{
    m_UniqueTargetInfo.clear();
    m_UniqueGOTargetInfo.clear();
    m_UniqueItemInfo.clear();
    m_delayMoment = 0;
}

void Spell::AddUnitTarget(Unit* target, uint8 effectMask, CheckException exception)
{
    if (m_caster != target)
        m_targets.m_targetMask |= TARGET_FLAG_UNIT; // all spells with unit target must have this flag

    // Check for effect immune skip if immuned
    uint8 notImmunedMask = 0;
    for (uint8 effIndex = 0; effIndex < MAX_EFFECT_INDEX; ++effIndex)
        if ((effectMask & (1 << effIndex)) != 0)
            if (!target->IsImmuneToSpellEffect(m_spellInfo, SpellEffectIndex(effIndex), target == m_caster))
                notImmunedMask |= (1 << effIndex);

    ObjectGuid targetGUID = target->GetObjectGuid();

    // Lookup target in already in list
    for (auto& ihit : m_UniqueTargetInfo)
    {
        if (targetGUID == ihit.targetGUID)                 // Found in list
        {
            ihit.effectMask |= effectMask;
            ihit.effectHitMask |= notImmunedMask;
            return;
        }
    }

    // This is new target calculate data for him

    // Get spell hit result on target
    TargetInfo targetInfo;
    targetInfo.targetGUID = targetGUID;                         // Store target GUID
    targetInfo.effectHitMask = exception != EXCEPTION_MAGNET ? notImmunedMask : effectMask; // Store not immuned effects
    targetInfo.effectMask = effectMask;                         // Store index of effect
    targetInfo.processed = false;                              // Effects not applied on target
    targetInfo.magnet = (exception == EXCEPTION_MAGNET);

    // Calculate hit result
    targetInfo.missCondition = m_ignoreHitResult ? SPELL_MISS_NONE : m_caster->SpellHitResult(target, m_spellInfo, targetInfo.effectMask, m_reflectable);

    // spell fly from visual cast object
    WorldObject* affectiveObject = GetAffectiveCasterObject();

    // Spell have speed (possible inherited from triggering spell) - need calculate incoming time
    float speed = GetSpellSpeed();
    if (speed > 0.0f && affectiveObject && target != affectiveObject)
    {
        // calculate spell incoming interval
        float dist = affectiveObject->GetDistance(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), DIST_CALC_NONE);
        dist = sqrt(dist); // default distance calculation is raw, apply sqrt before the next step

        if (dist < 5.0f)
            dist = 5.0f;
        targetInfo.timeDelay = (uint64)floor(dist / speed * 1000.0f);

        // Calculate minimum incoming time
        if (m_delayMoment == 0 || m_delayMoment > targetInfo.timeDelay)
            m_delayMoment = targetInfo.timeDelay;
    }
    else
        targetInfo.timeDelay = uint64(0);

    // If target reflect spell back to caster
    if (targetInfo.missCondition == SPELL_MISS_REFLECT)
    {
        // Victim reflects, apply reflect procs
        m_caster->ProcDamageAndSpell(ProcSystemArguments(target, PROC_FLAG_NONE, PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG, PROC_EX_REFLECT, 1, BASE_ATTACK, m_spellInfo));
        // Calculate reflected spell result on caster
        targetInfo.reflectResult = m_caster->SpellHitResult(m_caster, m_spellInfo, targetInfo.effectMask, m_reflectable);
        // Caster reflects back spell which was already reflected by victim
        if (targetInfo.reflectResult == SPELL_MISS_REFLECT)
        {
            // Apply reflect procs on self
            m_caster->ProcDamageAndSpell(ProcSystemArguments(m_caster, PROC_FLAG_NONE, PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG, PROC_EX_REFLECT, 1, BASE_ATTACK, m_spellInfo));
            // Full circle: it's impossible to reflect further, "Immune" shows up
            targetInfo.reflectResult = SPELL_MISS_IMMUNE;
        }
        // Increase time interval for reflected spells by 1.5
        targetInfo.timeDelay += targetInfo.timeDelay >> 1;
    }
    else
        targetInfo.reflectResult = SPELL_MISS_NONE;

    // Add target to list
    m_UniqueTargetInfo.push_back(targetInfo);
}

void Spell::AddGOTarget(GameObject* target, uint8 effectMask)
{
    m_targets.m_targetMask |= TARGET_FLAG_GAMEOBJECT; // all spells with GO target must have this flag TODO: do this based on target type

    ObjectGuid targetGUID = target->GetObjectGuid();

    // Lookup target in already in list
    for (auto& ihit : m_UniqueGOTargetInfo)
    {
        if (targetGUID == ihit.targetGUID)                 // Found in list
        {
            ihit.effectMask |= effectMask;                 // Add only effect mask
            return;
        }
    }

    // This is new target calculate data for him

    GOTargetInfo targetInfo;
    targetInfo.targetGUID = targetGUID;
    targetInfo.effectMask = effectMask;
    targetInfo.processed = false;                              // Effects not apply on target

    // spell fly from visual cast object
    WorldObject* affectiveObject = GetAffectiveCasterObject();

    // Spell can have speed - need calculate incoming time
    float speed = GetSpellSpeed();
    if (speed > 0.0f && affectiveObject && target != affectiveObject)
    {
        // calculate spell incoming interval
        float dist = affectiveObject->GetDistance(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), DIST_CALC_NONE);
        dist = sqrt(dist); // default distance calculation is raw, apply sqrt before the next step
        if (dist < 5.0f)
            dist = 5.0f;
        targetInfo.timeDelay = (uint64)floor(dist / speed * 1000.0f);
        if (m_delayMoment == 0 || m_delayMoment > targetInfo.timeDelay)
            m_delayMoment = targetInfo.timeDelay;
    }
    else
        targetInfo.timeDelay = uint64(0);

    // Add target to list
    m_UniqueGOTargetInfo.push_back(targetInfo);
}

void Spell::AddItemTarget(Item* item, uint8 effectMask)
{
    // Lookup target in already in list
    for (auto& ihit : m_UniqueItemInfo)
    {
        if (item == ihit.item)                        // Found in list
        {
            ihit.effectMask |= effectMask;            // Add only effect mask
            return;
        }
    }

    // This is new target add data

    ItemTargetInfo target;
    target.item = item;
    target.effectMask = effectMask;
    m_UniqueItemInfo.push_back(target);
}

void Spell::AddDestExecution(SpellEffectIndex effIndex)
{
    m_destTargetInfo.effectMask |= (1 << effIndex);
    if (m_destTargetInfo.effectMask == 0)
    {
        // spell fly from visual cast object
        WorldObject* affectiveObject = GetAffectiveCasterObject();
        float speed = GetSpellSpeed();
        if (speed > 0.0f)
        {
            // calculate spell incoming interval
            float x, y, z;
            m_targets.getDestination(x, y, z);
            float dist = affectiveObject->GetDistance(x, y, z, DIST_CALC_NONE);
            dist = sqrt(dist); // default distance calculation is raw, apply sqrt before the next step
            if (dist < 5.0f)
                dist = 5.0f;
            m_destTargetInfo.timeDelay = (uint64)floor(dist / speed * 1000.0f);
            if (m_delayMoment == 0 || m_delayMoment > m_destTargetInfo.timeDelay)
                m_delayMoment = m_destTargetInfo.timeDelay;
        }
        else
            m_destTargetInfo.timeDelay = uint64(0);
    }
}

void Spell::DoAllEffectOnTarget(TargetInfo* target)
{
    if (target->processed)                                  // Check target
        return;
    target->processed = true;                               // Target checked in apply effects procedure

    // Get mask of effects for target
    uint32 effectMask = target->effectHitMask;

    Unit* unit = m_caster->GetObjectGuid() == target->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, target->targetGUID);
    if (!unit)
        return;

    // Get original caster (if exist) and calculate damage/healing from him data
    Unit* real_caster = GetAffectiveCaster();
    // FIXME: in case wild GO heal/damage spells will be used target bonuses
    Unit* caster = real_caster ? real_caster : m_caster;

    SpellMissInfo missInfo = target->missCondition;
    // Need init unitTarget by default unit (can changed in code on reflect)
    // Or on missInfo!=SPELL_MISS_NONE unitTarget undefined (but need in trigger subsystem)
    unitTarget = unit;

    // Reset damage/healing counter
    ResetEffectDamageAndHeal();

    // Fill base trigger info
    uint32 procAttacker;
    uint32 procVictim;
    uint32 procEx       = PROC_EX_NONE;
    PrepareMasksForProcSystem(target->effectMask, procAttacker, procVictim, caster, unitTarget);
    if (target->magnet)
        procEx |= PROC_EX_MAGNET;

    // drop proc flags in case target not affected negative effects in negative spell
    // for example caster bonus or animation,
    // except miss case where will assigned PROC_EX_* flags later
    if (((procAttacker | procVictim) & NEGATIVE_TRIGGER_MASK) &&
            !(target->effectHitMask & m_negativeEffectMask) && missInfo == SPELL_MISS_NONE)
    {
        procAttacker = PROC_FLAG_NONE;
        procVictim   = PROC_FLAG_NONE;
    }

    float speed = GetSpellSpeed();
    if (speed > 0.0f)
    {
        // mark effects that were already handled in Spell::HandleDelayedSpellLaunch on spell launch as processed
        for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
            if (IsEffectHandledOnDelayedSpellLaunch(m_spellInfo, SpellEffectIndex(i)))
                effectMask &= ~(1 << i);

        // maybe used in effects that are handled on hit
        m_damage += target->damage;
    }

    if (missInfo == SPELL_MISS_NONE)                        // In case spell hit target, do all effect on that target
        DoSpellHitOnUnit(unit, effectMask);
    else if (missInfo != SPELL_MISS_EVADE)
    {
        if (missInfo == SPELL_MISS_REFLECT)                // In case spell reflect from target, do all effect on caster (if hit)
        {
            if (target->reflectResult == SPELL_MISS_NONE)       // If reflected spell hit caster -> do all effect on him
            {
                DoSpellHitOnUnit(m_caster, effectMask, true);
                unitTarget = m_caster;
            }
        }

        // Failed hostile spell hits count as attack made against target (if detected)
        if (real_caster && real_caster != unit)
            m_caster->CasterHitTargetWithSpell(real_caster, unit, m_spellInfo, false);
    }

    // All calculated do it!
    // Do healing and triggers
    if (m_healing)
    {
        bool crit = real_caster && real_caster->RollSpellCritOutcome(unitTarget, m_spellSchoolMask, m_spellInfo);
        uint32 addhealth = m_healing;
        if (crit)
        {
            procEx |= PROC_EX_CRITICAL_HIT;
            addhealth = caster->CalculateCritAmount(nullptr, addhealth, m_spellInfo, true);
        }
        else
            procEx |= PROC_EX_NORMAL_HIT;

        int32 gain = caster->DealHeal(unitTarget, addhealth, m_spellInfo, crit);

        if (real_caster)
            unitTarget->getHostileRefManager().threatAssist(real_caster, float(gain) * 0.5f * sSpellMgr.GetSpellThreatMultiplier(m_spellInfo), m_spellInfo);

        // Do triggers for unit (reflect triggers passed on hit phase for correct drop charge)
        if (m_canTrigger && missInfo != SPELL_MISS_REFLECT)
        {
            // Some spell expected send main spell info to triggered system
            SpellEntry const* spellInfo = m_spellInfo;
            switch (m_spellInfo->Id)
            {
                case 19968:                                 // Holy Light triggered heal
                case 19993:                                 // Flash of Light triggered heal
                {
                    // stored in unused spell effect basepoints in main spell code
                    uint32 spellid = m_currentBasePoints[EFFECT_INDEX_1];
                    spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
                }
            }

            caster->ProcDamageAndSpell(ProcSystemArguments(unitTarget, real_caster ? procAttacker : uint32(PROC_FLAG_NONE), procVictim, procEx, addhealth, m_attackType, m_spellInfo, this, gain));
        }
    }
    // Do damage and triggers
    else if (m_damage)
    {
        // Fill base damage struct (unitTarget - is real spell target)
        SpellNonMeleeDamage damageInfo(caster, unitTarget, m_spellInfo->Id, GetFirstSchoolInMask(m_spellSchoolMask));

        if (speed > 0.0f)
        {
            damageInfo.damage = m_damage;
            damageInfo.HitInfo = target->HitInfo;
        }
        // Add bonuses and fill damageInfo struct
        else
            caster->CalculateSpellDamage(&damageInfo, m_damage, m_spellInfo, m_attackType);

        unitTarget->CalculateAbsorbResistBlock(caster, &damageInfo, m_spellInfo);

        caster->DealDamageMods(damageInfo.target, damageInfo.damage, &damageInfo.absorb, SPELL_DIRECT_DAMAGE, m_spellInfo);

        // Send log damage message to client
        caster->SendSpellNonMeleeDamageLog(&damageInfo);

        procEx |= createProcExtendMask(&damageInfo, missInfo);
        procVictim |= PROC_FLAG_TAKEN_ANY_DAMAGE;

        caster->DealSpellDamage(&damageInfo, true);

        // Bloodthirst
        if (m_spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR && m_spellInfo->SpellFamilyFlags & uint64(0x0000000002000000))
        {
            uint32 BTAura = 0;
            switch (m_spellInfo->Id)
            {
                case 23881: BTAura = 23885; break;
                case 23892: BTAura = 23886; break;
                case 23893: BTAura = 23887; break;
                case 23894: BTAura = 23888; break;
                default:
                    sLog.outError("Spell::EffectSchoolDMG: Spell %u not handled in BTAura", m_spellInfo->Id);
                    break;
            }
            if (BTAura)
                m_caster->CastSpell(m_caster, BTAura, TRIGGERED_OLD_TRIGGERED);
        }

        // Do triggers for unit (reflect triggers passed on hit phase for correct drop charge)
        if (m_canTrigger && missInfo != SPELL_MISS_REFLECT)
            caster->ProcDamageAndSpell(ProcSystemArguments(unitTarget, real_caster ? procAttacker : uint32(PROC_FLAG_NONE), procVictim, procEx, damageInfo.damage, m_attackType, m_spellInfo, this));

        // trigger weapon enchants for weapon based spells; exclude spells that stop attack, because may break CC
        if (m_caster->GetTypeId() == TYPEID_PLAYER && m_spellInfo->EquippedItemClass == ITEM_CLASS_WEAPON &&
            !m_spellInfo->HasAttribute(SPELL_ATTR_STOP_ATTACK_TARGET))
            ((Player*)m_caster)->CastItemCombatSpell(unitTarget, m_attackType, !IsNextMeleeSwingSpell());
    }
    // Passive spell hits/misses or active spells only misses (only triggers if proc flags set)
    else if (procAttacker || procVictim)
    {
        // Fill base damage struct (unitTarget - is real spell target)
        SpellNonMeleeDamage damageInfo(caster, unitTarget, m_spellInfo->Id, GetFirstSchoolInMask(m_spellSchoolMask));
        procEx = createProcExtendMask(&damageInfo, missInfo);
        // Do triggers for unit (reflect triggers passed on hit phase for correct drop charge)
        if (m_canTrigger && missInfo != SPELL_MISS_REFLECT)
            // traps need to be procced at trap triggerer
            caster->ProcDamageAndSpell(ProcSystemArguments(procAttacker & PROC_FLAG_ON_TRAP_ACTIVATION ? m_targets.getUnitTarget() : unit, real_caster ? procAttacker : uint32(PROC_FLAG_NONE), procVictim, procEx, 0, m_attackType, m_spellInfo, this));
    }

    // Call scripted function for AI if this spell is casted upon a creature
    if (unit->GetTypeId() == TYPEID_UNIT)
    {
        // cast at creature (or GO) quest objectives update at successful cast finished (+channel finished)
        // ignore pets or autorepeat/melee casts for speed (not exist quest for spells (hm... )
        if (real_caster && !((Creature*)unit)->IsPet() && !IsAutoRepeat() && !IsNextMeleeSwingSpell() && !IsChannelActive())
            if (Player* p = real_caster->GetBeneficiaryPlayer())
                p->RewardPlayerAndGroupAtCast(unit, m_spellInfo->Id);

        if (((Creature*)unit)->AI())
            ((Creature*)unit)->AI()->SpellHit(m_caster, m_spellInfo);
    }

    // Call scripted function for AI if this spell is casted by a creature
    if (m_caster->GetTypeId() == TYPEID_UNIT && ((Creature*)m_caster)->AI())
        ((Creature*)m_caster)->AI()->SpellHitTarget(unit, m_spellInfo, missInfo);
    if (real_caster && real_caster != m_caster && real_caster->GetTypeId() == TYPEID_UNIT && ((Creature*)real_caster)->AI())
        ((Creature*)real_caster)->AI()->SpellHitTarget(unit, m_spellInfo, missInfo);

    if (m_spellAuraHolder)
    {
        if (m_caster->IsSpellProccingHappening())
            m_caster->AddDelayedHolderDueToProc(m_spellAuraHolder);
        else
            m_spellAuraHolder->SetState(SPELLAURAHOLDER_STATE_READY);
    }
}

void Spell::DoSpellHitOnUnit(Unit* unit, uint32 effectMask, bool isReflected)
{
    if (!unit)
        return;

    Unit* realCaster = GetAffectiveCaster();

    const bool traveling = (GetSpellSpeed() > 0.0f);

    // Recheck immune (only for delayed spells)
    if (traveling && !m_spellInfo->HasAttribute(SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY))
    {
        if (unit->IsImmuneToDamage(GetSpellSchoolMask(m_spellInfo)) ||
            unit->IsImmuneToSpell(m_spellInfo, unit == realCaster, effectMask))
        {
            if (realCaster)
                realCaster->SendSpellMiss(unit, m_spellInfo->Id, SPELL_MISS_IMMUNE);

            ResetEffectDamageAndHeal();
            return;
        }
    }

    if (traveling && realCaster && realCaster != unit)
    {
        // Recheck  UNIT_FLAG_NON_ATTACKABLE for delayed spells
        if (unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE) && unit->GetMasterGuid() != m_caster->GetObjectGuid())
        {
            realCaster->SendSpellMiss(unit, m_spellInfo->Id, SPELL_MISS_EVADE);
            ResetEffectDamageAndHeal();
            return;
        }

        if (realCaster->CanAttack(unit))
        {
            // for delayed spells ignore not visible explicit target, if caster is dead, nothing is visible for him
            if (traveling && unit == m_targets.getUnitTarget() &&
                !unit->IsVisibleForOrDetect(m_caster, m_caster, false, false, true, true) && m_caster->isAlive())
            {
                // Workaround: do not send evade if caster/unit are dead to prevent combat log errors
                // TODO: Visibility check clearly lackluster if we end up here like this, to be fixed later
                if (unit->isAlive() && realCaster->isAlive())
                    realCaster->SendSpellMiss(unit, m_spellInfo->Id, SPELL_MISS_EVADE);
                ResetEffectDamageAndHeal();
                return;
            }
        }
        else
        {
            // for delayed spells ignore negative spells (after duel end) for friendly targets
            if (!IsPositiveSpell(m_spellInfo->Id, realCaster, unit))
            {
                realCaster->SendSpellMiss(unit, m_spellInfo->Id, SPELL_MISS_IMMUNE);
                ResetEffectDamageAndHeal();
                return;
            }
        }
    }

    // Get Data Needed for Diminishing Returns, some effects may have multiple auras, so this must be done on spell hit, not aura add
    if (m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) || IsCreatureDRSpell(m_spellInfo))
    {
        m_diminishGroup = GetDiminishingReturnsGroupForSpell(m_spellInfo, m_triggeredByAuraSpell != nullptr || (m_IsTriggeredSpell && m_CastItem));
        m_diminishLevel = unit->GetDiminishing(m_diminishGroup);
        // Increase Diminishing on unit, current informations for actually casts will use values above
        if ((GetDiminishingReturnsGroupType(m_diminishGroup) == DRTYPE_PLAYER && unit->GetTypeId() == TYPEID_PLAYER) ||
            GetDiminishingReturnsGroupType(m_diminishGroup) == DRTYPE_ALL)
            unit->IncrDiminishing(m_diminishGroup);
    }

    // Apply additional spell effects to target
    CastPreCastSpells(unit);

    if (IsSpellAppliesAura(m_spellInfo, effectMask))
    {
        m_spellAuraHolder = CreateSpellAuraHolder(m_spellInfo, unit, realCaster, m_CastItem, m_triggeredBySpellInfo);
        m_spellAuraHolder->setDiminishGroup(m_diminishGroup);
    }
    else
        m_spellAuraHolder = nullptr;

    for (int effectNumber = 0; effectNumber < MAX_EFFECT_INDEX; ++effectNumber)
    {
        if (effectMask & (1 << effectNumber))
        {
            HandleEffects(unit, nullptr, nullptr, SpellEffectIndex(effectNumber), m_damageMultipliers[effectNumber]);
            if (m_applyMultiplierMask & (1 << effectNumber))
            {
                // Get multiplier
                float multiplier = m_spellInfo->DmgMultiplier[effectNumber];
                // Apply multiplier mods
                if (realCaster)
                    if (Player* modOwner = realCaster->GetSpellModOwner())
                        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_EFFECT_PAST_FIRST, multiplier, this);
                m_damageMultipliers[effectNumber] *= multiplier;
            }
        }
    }

    if (realCaster && realCaster != unit)
        m_caster->CasterHitTargetWithSpell(realCaster, unit, m_spellInfo);

    // now apply all created auras
    if (m_spellAuraHolder)
    {
        // normally shouldn't happen
        if (!m_spellAuraHolder->IsEmptyHolder())
        {
            int32 duration = m_spellAuraHolder->GetAuraMaxDuration();
            int32 originalDuration = duration;

            if (duration > 0)
            {
                unit->ApplyDiminishingToDuration(m_diminishGroup, duration, m_caster, m_diminishLevel, isReflected);

                // Fully diminished
                if (duration == 0)
                {
                    delete m_spellAuraHolder;
                    m_spellAuraHolder = nullptr;
                    return;
                }
            }

            if (duration != originalDuration)
            {
                m_spellAuraHolder->SetAuraMaxDuration(duration);
                m_spellAuraHolder->SetAuraDuration(duration);
            }

            if (!unit->AddSpellAuraHolder(m_spellAuraHolder))
            {
                delete m_spellAuraHolder;
                m_spellAuraHolder = nullptr;
            }
        }
        else
        {
            delete m_spellAuraHolder;
            m_spellAuraHolder = nullptr;
        }
    }
}

void Spell::DoAllTargetlessEffects(bool dest)
{
    uint32 procAttacker;
    uint32 procVictim;
    uint32 procEx = PROC_EX_NONE;
    WorldObject* caster = m_caster; // preparation for GO casting
    Unit* unitCaster = caster->GetTypeId() == TYPEID_UNIT ? static_cast<Unit*>(caster) : nullptr;

    uint32 effectMask;
    if (dest) // can have delay
    {
        effectMask = m_destTargetInfo.effectMask;
        m_destTargetInfo.processed = true;
        for (uint32 j = 0; j < MAX_EFFECT_INDEX; ++j)
        {
            if ((effectMask & (1 << j)) != 0)
                HandleEffects(nullptr, nullptr, nullptr, SpellEffectIndex(j));
        }
    }
    else // always immediate
    {
        effectMask = m_targetlessMask;
        for (uint32 j = 0; j < MAX_EFFECT_INDEX; ++j)
        {
            if ((effectMask & (1 << j)) != 0)
                HandleEffects(nullptr, nullptr, nullptr, SpellEffectIndex(j));
        }
    }

    if (effectMask && unitCaster)
    {
        PrepareMasksForProcSystem(effectMask, procAttacker, procVictim, caster, unitTarget);
        unitCaster->ProcDamageAndSpell(ProcSystemArguments(procAttacker & PROC_FLAG_ON_TRAP_ACTIVATION ? m_targets.getUnitTarget() : nullptr, unitCaster ? procAttacker : uint32(PROC_FLAG_NONE), procVictim, procEx, 0, m_attackType, m_spellInfo, this));
    }
}

void Spell::DoAllEffectOnTarget(GOTargetInfo* target)
{
    if (target->processed)                                  // Check target
        return;
    target->processed = true;                               // Target checked in apply effects procedure

    uint32 effectMask = target->effectMask;
    if (!effectMask)
        return;

    GameObject* go = m_caster->GetMap()->GetGameObject(target->targetGUID);
    if (!go)
        return;

    for (int effectNumber = 0; effectNumber < MAX_EFFECT_INDEX; ++effectNumber)
        if (effectMask & (1 << effectNumber))
            HandleEffects(nullptr, nullptr, go, SpellEffectIndex(effectNumber));

    // cast at creature (or GO) quest objectives update at successful cast finished (+channel finished)
    // ignore autorepeat/melee casts for speed (not exist quest for spells (hm... )
    if (!IsAutoRepeat() && !IsNextMeleeSwingSpell() && !IsChannelActive())
    {
        if (Player* p = m_caster->GetBeneficiaryPlayer())
            p->RewardPlayerAndGroupAtCast(go, m_spellInfo->Id);
    }
}

void Spell::DoAllEffectOnTarget(ItemTargetInfo* target)
{
    uint32 effectMask = target->effectMask;
    if (!target->item || !effectMask)
        return;

    for (int effectNumber = 0; effectNumber < MAX_EFFECT_INDEX; ++effectNumber)
        if (effectMask & (1 << effectNumber))
            HandleEffects(nullptr, target->item, nullptr, SpellEffectIndex(effectNumber));
}

void Spell::HandleDelayedSpellLaunch(TargetInfo* target)
{
    // Get mask of effects for target
    uint32 mask = target->effectHitMask;

    Unit* unit = m_caster->GetObjectGuid() == target->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, target->targetGUID);
    if (!unit)
        return;

    // Get original caster (if exist) and calculate damage/healing from him data
    Unit* real_caster = GetAffectiveCaster();
    // FIXME: in case wild GO heal/damage spells will be used target bonuses
    Unit* caster = real_caster ? real_caster : m_caster;

    SpellMissInfo missInfo = target->missCondition;
    // Need init unitTarget by default unit (can changed in code on reflect)
    // Or on missInfo!=SPELL_MISS_NONE unitTarget undefined (but need in trigger subsystem)
    unitTarget = unit;

    // Reset damage/healing counter
    m_damage = 0;
    m_healing = 0; // healing maybe not needed at this point

    // Fill base damage struct (unitTarget - is real spell target)
    SpellNonMeleeDamage damageInfo(caster, unitTarget, m_spellInfo->Id, GetFirstSchoolInMask(m_spellSchoolMask));

    // keep damage amount for reflected spells
    if (missInfo == SPELL_MISS_NONE || (missInfo == SPELL_MISS_REFLECT && target->reflectResult == SPELL_MISS_NONE))
    {
        for (int32 effectNumber = 0; effectNumber < MAX_EFFECT_INDEX; ++effectNumber)
        {
            if (mask & (1 << effectNumber) && IsEffectHandledOnDelayedSpellLaunch(m_spellInfo, SpellEffectIndex(effectNumber)))
            {
                HandleEffects(unit, nullptr, nullptr, SpellEffectIndex(effectNumber), m_damageMultipliers[effectNumber]);
                if (m_applyMultiplierMask & (1 << effectNumber))
                {
                    // Get multiplier
                    float multiplier = m_spellInfo->DmgMultiplier[effectNumber];
                    // Apply multiplier mods
                    if (real_caster)
                        if (Player* modOwner = real_caster->GetSpellModOwner())
                            modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_EFFECT_PAST_FIRST, multiplier, this);
                    m_damageMultipliers[effectNumber] *= multiplier;
                }
            }
        }

        if (m_damage > 0)
            caster->CalculateSpellDamage(&damageInfo, m_damage, m_spellInfo, m_attackType);
    }

    target->damage = damageInfo.damage;
    target->HitInfo = damageInfo.HitInfo;
}

void Spell::InitializeDamageMultipliers()
{
    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (m_spellInfo->Effect[i] == 0)
            continue;

        uint32 EffectChainTarget = m_spellInfo->EffectChainTarget[i];
        if (Unit* realCaster = GetAffectiveCaster())
            if (Player* modOwner = realCaster->GetSpellModOwner())
                modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_JUMP_TARGETS, EffectChainTarget, this);

        m_damageMultipliers[i] = 1.0f;
        if ((m_spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_ENEMY || m_spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_FRIEND_CHAIN_HEAL) &&
                (EffectChainTarget > 1))
            m_applyMultiplierMask |= (1 << i);
    }
}

bool Spell::IsAliveUnitPresentInTargetList()
{
    // Not need check return true
    if (m_needAliveTargetMask == 0)
        return true;

    uint8 channelTargetEffectMask = m_needAliveTargetMask;
    uint8 channelAuraMask = 0;
    for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (m_spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA)
            channelAuraMask |= 1 << i;

    channelAuraMask &= channelTargetEffectMask;

    for (TargetList::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
    {
        if (ihit->missCondition == SPELL_MISS_NONE && (channelAuraMask & ihit->effectHitMask))
        {
            Unit* unit = m_caster->GetObjectGuid() == ihit->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit->targetGUID);

            if (!unit)
                continue;

            // either unit is alive and normal spell, or unit dead and deathonly-spell
            if (IsValidDeadOrAliveTarget(unit))
                channelAuraMask &= ~ihit->effectHitMask;   // remove from need alive mask effect that have alive target
        }
    }

    // is all effects from m_needAliveTargetMask have alive targets
    return channelAuraMask == 0;
}

bool Spell::IsValidDeadOrAliveTarget(Unit const* target) const
{
    if (target->isAlive())
        return !IsDeathOnlySpell(m_spellInfo);
    if (IsAllowingDeadTarget(m_spellInfo))
        return true;
    return false;
}

// Helper for Chain Healing
// Spell target first
// Raidmates then descending by injury suffered (MaxHealth - Health)
// Other players/mobs then descending by injury suffered (MaxHealth - Health)
struct ChainHealingOrder : public std::binary_function<const Unit*, const Unit*, bool>
{
    const Unit* MainTarget;
    ChainHealingOrder(Unit const* Target) : MainTarget(Target) {};
    // functor for operator ">"
    bool operator()(Unit const* _Left, Unit const* _Right) const
    {
        return (ChainHealingHash(_Left) < ChainHealingHash(_Right));
    }
    int32 ChainHealingHash(Unit const* Target) const
    {
        if (Target == MainTarget)
            return 0;
        if (Target->IsInGroup(MainTarget))
        {
            if (Target->GetHealth() == Target->GetMaxHealth())
                return 40000;
            return 20000 - Target->GetMaxHealth() + Target->GetHealth();
        }
        return 40000 - Target->GetMaxHealth() + Target->GetHealth();
    }
};

class ChainHealingFullHealth: std::unary_function<const Unit*, bool>
{
    public:
        const Unit* MainTarget;
        ChainHealingFullHealth(const Unit* Target) : MainTarget(Target) {};

        bool operator()(const Unit* Target)
        {
            return (Target != MainTarget && Target->GetHealth() == Target->GetMaxHealth());
        }
};

bool Spell::CheckAndAddMagnetTarget(Unit* target, SpellEffectIndex effIndex, bool targetB, TempTargetingData& data)
{
    if (Unit* magnetTarget = m_caster->SelectMagnetTarget(target, this))
    {
        // Found. Push totem as target instead.
        m_targets.setUnitTarget(magnetTarget);
        data.data[effIndex].tmpUnitList[targetB].push_back(magnetTarget);
        data.magnet = true;
        return true;
    }

    return false;
}

void Spell::SetTargetMap(SpellEffectIndex effIndex, uint32 targetMode, bool targetB, TempTargetingData& targetingData)
{
    TempTargetData& data = targetingData.data[effIndex];
    float radius;
    uint32 EffectChainTarget = m_spellInfo->EffectChainTarget[effIndex];
    uint32 unMaxTargets = m_affectedTargetCount;  // Get spell max affected targets

    GetSpellRangeAndRadius(effIndex, radius, targetB, EffectChainTarget);
    float cone = GetCone();

    UnitList& tempUnitList = data.tmpUnitList[targetB];
    GameObjectList& tempGOList = data.tmpGOList[targetB];

    switch (targetMode)
    {
        case TARGET_LOCATION_UNIT_MINION_POSITION: // unknown how pet summon is different - maybe some formation support?
        case TARGET_LOCATION_CASTER_FRONT_RIGHT:
        case TARGET_LOCATION_CASTER_BACK_RIGHT:
        case TARGET_LOCATION_CASTER_BACK_LEFT:
        case TARGET_LOCATION_CASTER_FRONT_LEFT:
        case TARGET_LOCATION_CASTER_FRONT:
        case TARGET_LOCATION_CASTER_BACK:
        case TARGET_LOCATION_CASTER_LEFT:
        case TARGET_LOCATION_CASTER_RIGHT:
        {
            float angle = m_caster->GetOrientation();
            switch (targetMode)
            {
                case TARGET_LOCATION_UNIT_MINION_POSITION:
                case TARGET_LOCATION_CASTER_FRONT_LEFT:  angle += M_PI_F * 0.25f; break;
                case TARGET_LOCATION_CASTER_BACK_LEFT:   angle += M_PI_F * 0.75f; break;
                case TARGET_LOCATION_CASTER_BACK_RIGHT:  angle += M_PI_F * 1.25f; break;
                case TARGET_LOCATION_CASTER_FRONT_RIGHT: angle += M_PI_F * 1.75f; break;
                case TARGET_LOCATION_CASTER_FRONT:                                break;
                case TARGET_LOCATION_CASTER_BACK:        angle += M_PI_F;         break;
                case TARGET_LOCATION_CASTER_LEFT:        angle += M_PI_F / 2;     break;
                case TARGET_LOCATION_CASTER_RIGHT:       angle -= M_PI_F / 2;     break;
            }

            float x, y;
            float z = m_caster->GetPositionZ();
            if (radius == 0.f) // All shaman totems have 0 radius - need to override with proper value
                radius = 2.f;

            m_caster->GetNearPoint(nullptr, x, y, z, m_caster->GetObjectBoundingRadius(), radius, angle, m_caster->IsInWater());
            m_targets.setDestination(x, y, z);
            break;
        }
        case TARGET_LOCATION_CASTER_DEST:
        {
            if (!(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION))
                if (WorldObject* caster = GetCastingObject())
                    m_targets.setDestination(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ());
            break;
        }
        case TARGET_LOCATION_DATABASE:
        {
            if (SpellTargetPosition const* st = sSpellMgr.GetSpellTargetPosition(m_spellInfo->Id))
            {
                m_targets.setDestination(st->target_X, st->target_Y, st->target_Z);
                // TODO - maybe use an (internal) value for the map for neat far teleport handling

                // far-teleport spells are handled in SpellEffect, elsewise report an error about an unexpected map (spells are always locally)
                if (st->target_mapId != m_caster->GetMapId() && m_spellInfo->Effect[effIndex] != SPELL_EFFECT_TELEPORT_UNITS && m_spellInfo->Effect[effIndex] != SPELL_EFFECT_BIND)
                    sLog.outError("SPELL: wrong map (%u instead %u) target coordinates for spell ID %u", st->target_mapId, m_caster->GetMapId(), m_spellInfo->Id);
            }
            else
                sLog.outError("SPELL: unknown target coordinates for spell ID %u", m_spellInfo->Id);
            break;
        }
        case TARGET_LOCATION_CASTER_HOME_BIND:
        {
            if (m_caster->GetTypeId() == TYPEID_PLAYER)
            {
                float x, y, z;
                static_cast<Player*>(m_caster)->GetHomebindLocation(x, y, z);
                m_targets.setDestination(x, y, z);
            }
            break;
        }
        case TARGET_LOCATION_CASTER_FRONT_LEAP:
        {
            float dist = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[effIndex]));
            const float IN_OR_UNDER_LIQUID_RANGE = 0.8f;                // range to make player under liquid or on liquid surface from liquid level

            G3D::Vector3 prevPos, nextPos;
            float orientation = m_caster->GetOrientation();

            prevPos.x = m_caster->GetPositionX();
            prevPos.y = m_caster->GetPositionY();
            prevPos.z = m_caster->GetPositionZ();

            float groundZ = prevPos.z;
            bool isPrevInLiquid = false;

            // falling case
            if (!m_caster->GetMap()->GetHeightInRange(prevPos.x, prevPos.y, groundZ, 3.0f) && m_caster->m_movementInfo.HasMovementFlag(MOVEFLAG_FALLING))
            {
                nextPos.x = prevPos.x + dist * cos(orientation);
                nextPos.y = prevPos.y + dist * sin(orientation);
                nextPos.z = prevPos.z - 2.0f; // little hack to avoid the impression to go up when teleporting instead of continue to fall. This value may need some tweak

                //
                GridMapLiquidData liquidData;
                if (m_caster->GetMap()->GetTerrain()->IsInWater(nextPos.x, nextPos.y, nextPos.z, &liquidData))
                {
                    if (fabs(nextPos.z - liquidData.level) < 10.0f)
                        nextPos.z = liquidData.level - IN_OR_UNDER_LIQUID_RANGE;
                }
                else
                {
                    // fix z to ground if near of it
                    m_caster->GetMap()->GetHeightInRange(nextPos.x, nextPos.y, nextPos.z, 10.0f);
                }

                // check any obstacle and fix coords
                m_caster->GetMap()->GetHitPosition(prevPos.x, prevPos.y, prevPos.z + 0.5f, nextPos.x, nextPos.y, nextPos.z, -0.5f);
            }
            else
            {
                // fix origin position if player was jumping and near of the ground but not in ground
                if (fabs(prevPos.z - groundZ) > 0.5f)
                    prevPos.z = groundZ;

                //check if in liquid
                isPrevInLiquid = m_caster->GetMap()->GetTerrain()->IsInWater(prevPos.x, prevPos.y, prevPos.z);

                const float step = 2.0f;                                    // step length before next check slope/edge/water
                const float maxSlope = 50.0f;                               // 50(degree) max seem best value for walkable slope
                const float MAX_SLOPE_IN_RADIAN = maxSlope / 180.0f * M_PI_F;
                float nextZPointEstimation = 1.0f;
                float destx = prevPos.x + dist * cos(orientation);
                float desty = prevPos.y + dist * sin(orientation);
                const uint32 numChecks = ceil(fabs(dist / step));
                const float DELTA_X = (destx - prevPos.x) / numChecks;
                const float DELTA_Y = (desty - prevPos.y) / numChecks;

                for (uint32 i = 1; i < numChecks + 1; ++i)
                {
                    // compute next point average position
                    nextPos.x = prevPos.x + DELTA_X;
                    nextPos.y = prevPos.y + DELTA_Y;
                    nextPos.z = prevPos.z + nextZPointEstimation;

                    bool isInLiquid = false;
                    bool isInLiquidTested = false;
                    bool isOnGround = false;
                    GridMapLiquidData liquidData = {};

                    // try fix height for next position
                    if (!m_caster->GetMap()->GetHeightInRange(nextPos.x, nextPos.y, nextPos.z))
                    {
                        // we cant so test if we are on water
                        if (!m_caster->GetMap()->GetTerrain()->IsInWater(nextPos.x, nextPos.y, nextPos.z, &liquidData))
                        {
                            // not in water and cannot get correct height, maybe flying?
                            //sLog.outString("Can't get height of point %u, point value %s", i, nextPos.toString().c_str());
                            nextPos = prevPos;
                            break;
                        }
                        isInLiquid = true;
                        isInLiquidTested = true;
                    }
                    else
                        isOnGround = true;                                  // player is on ground

                    if (isInLiquid || (!isInLiquidTested && m_caster->GetMap()->GetTerrain()->IsInWater(nextPos.x, nextPos.y, nextPos.z, &liquidData)))
                    {
                        if (!isPrevInLiquid && fabs(liquidData.level - prevPos.z) > 2.0f)
                        {
                            // on edge of water with difference a bit to high to continue
                            //sLog.outString("Ground vs liquid edge detected!");
                            nextPos = prevPos;
                            break;
                        }

                        if ((liquidData.level - IN_OR_UNDER_LIQUID_RANGE) > nextPos.z)
                            nextPos.z = prevPos.z;                                      // we are under water so next z equal prev z
                        else
                            nextPos.z = liquidData.level - IN_OR_UNDER_LIQUID_RANGE;    // we are on water surface, so next z equal liquid level

                        isInLiquid = true;

                        float ground = nextPos.z;
                        if (m_caster->GetMap()->GetHeightInRange(nextPos.x, nextPos.y, ground))
                        {
                            if (nextPos.z < ground)
                            {
                                nextPos.z = ground;
                                isOnGround = true;                          // player is on ground of the water
                            }
                        }
                    }

                    //unitTarget->SummonCreature(VISUAL_WAYPOINT, nextPos.x, nextPos.y, nextPos.z, 0, TEMPSUMMON_TIMED_DESPAWN, 15000);
                    float hitZ = nextPos.z + 1.5f;
                    if (m_caster->GetMap()->GetHitPosition(prevPos.x, prevPos.y, prevPos.z + 1.5f, nextPos.x, nextPos.y, hitZ, -1.0f))
                    {
                        //sLog.outString("Blink collision detected!");
                        nextPos = prevPos;
                        break;
                    }

                    if (isOnGround)
                    {
                        // project vector to get only positive value
                        float ac = fabs(prevPos.z - nextPos.z);

                        // compute slope (in radian)
                        float slope = atan(ac / step);

                        // check slope value
                        if (slope > MAX_SLOPE_IN_RADIAN)
                        {
                            //sLog.outString("bad slope detected! %4.2f max %4.2f, ac(%4.2f)", slope * 180 / M_PI_F, maxSlope, ac);
                            nextPos = prevPos;
                            break;
                        }
                        //sLog.outString("slope is ok! %4.2f max %4.2f, ac(%4.2f)", slope * 180 / M_PI_F, maxSlope, ac);
                    }

                    //sLog.outString("point %u is ok, coords %s", i, nextPos.toString().c_str());
                    nextZPointEstimation = (nextPos.z - prevPos.z) / 2.0f;
                    isPrevInLiquid = isInLiquid;
                    prevPos = nextPos;
                }
            }
            m_targets.setDestination(nextPos.x, nextPos.y, nextPos.z);
        }
        case TARGET_LOCATION_UNIT_POSITION:
        {
            if (Unit* currentTarget = m_targets.getUnitTarget())
                m_targets.setDestination(currentTarget->GetPositionX(), currentTarget->GetPositionY(), currentTarget->GetPositionZ());
            break;
        }
        case TARGET_LOCATION_CASTER_FISHING_SPOT:
        {
            float x, y, z;
            // special code for fishing bobber (TARGET_LOCATION_CASTER_FISHING_SPOT), should not try to avoid objects
            // nor try to find ground level, but randomly vary in angle
            float min_dis = GetSpellMinRange(sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex));
            float max_dis = GetSpellMaxRange(sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex));
            float dis = rand_norm_f() * (max_dis - min_dis) + min_dis;
            // calculate angle variation for roughly equal dimensions of target area
            float max_angle = (max_dis - min_dis) / (max_dis + m_caster->GetObjectBoundingRadius());
            float angle_offset = max_angle * (rand_norm_f() - 0.5f);
            m_caster->GetNearPoint2D(x, y, dis + m_caster->GetObjectBoundingRadius(), m_caster->GetOrientation() + angle_offset);

            GridMapLiquidData liqData;
            if (!m_caster->GetTerrain()->IsInWater(x, y, m_caster->GetPositionZ() + 1.f, &liqData))
            {
                SendCastResult(SPELL_FAILED_NOT_FISHABLE);
                finish(false);
                return;
            }

            z = liqData.level;
            // finally, check LoS
            if (!m_caster->IsWithinLOS(x, y, z))
            {
                SendCastResult(SPELL_FAILED_LINE_OF_SIGHT);
                finish(false);
                return;
            }
            m_targets.setDestination(x, y, z);
            break;
        }
        case TARGET_LOCATION_CASTER_TARGET_POSITION:
        {
            Unit* currentTarget = m_targets.getUnitTarget();
            if (currentTarget)
                m_targets.setDestination(currentTarget->GetPositionX(), currentTarget->GetPositionY(), currentTarget->GetPositionZ());
            break;
        }
        case TARGET_UNIT_CASTER:
            tempUnitList.push_back(m_caster);
            break;
        case TARGET_UNIT_ENEMY_NEAR_CASTER:
        case TARGET_UNIT_FRIEND_NEAR_CASTER:
        case TARGET_UNIT_NEAR_CASTER:
        {
            m_targets.m_targetMask = 0;
            unMaxTargets = EffectChainTarget;
            float max_range = radius + unMaxTargets * CHAIN_SPELL_JUMP_RADIUS;

            UnitList tempTargetUnitMap;

            switch (targetMode)
            {
                case TARGET_UNIT_ENEMY_NEAR_CASTER:
                {
                    FillAreaTargets(tempTargetUnitMap, max_range, 0.f, PUSH_SELF_CENTER, SPELL_TARGETS_AOE_ATTACKABLE);
                    break;
                }
                case TARGET_UNIT_NEAR_CASTER: // TODO: Rename TARGET_UNIT_NEAR_CASTER to something better and find real difference with TARGET_UNIT_FRIEND_NEAR_CASTER.
                {
                    FillAreaTargets(tempTargetUnitMap, max_range, cone, PUSH_SELF_CENTER, SPELL_TARGETS_ALL);
                    break;
                }
                case TARGET_UNIT_FRIEND_NEAR_CASTER:
                {
                    FillAreaTargets(tempTargetUnitMap, max_range, cone, PUSH_SELF_CENTER, SPELL_TARGETS_ASSISTABLE);
                    break;
                }
            }

            if (tempTargetUnitMap.empty())
                break;

            SQLMultiStorage::SQLMSIteratorBounds<SpellTargetEntry> bounds = sSpellScriptTargetStorage.getBounds<SpellTargetEntry>(m_spellInfo->Id);
            if (bounds.first != bounds.second)
            {
                UnitList sourceTempTargetUnitMap = tempTargetUnitMap;
                tempTargetUnitMap.clear();
                CheckSpellScriptTargets(bounds, sourceTempTargetUnitMap, tempTargetUnitMap, effIndex);
            }

            tempTargetUnitMap.sort(TargetDistanceOrderNear(m_caster));

            // Now to get us a random target that's in the initial range of the spell
            uint32 t = 0;
            UnitList::iterator itr = tempTargetUnitMap.begin();
            while (itr != tempTargetUnitMap.end() && (*itr)->IsWithinDist(m_caster, radius))
                ++t, ++itr;

            if (!t)
                break;

            itr = tempTargetUnitMap.begin();
            std::advance(itr, urand() % t);
            Unit* pUnitTarget = *itr;
            tempUnitList.push_back(pUnitTarget);

            tempTargetUnitMap.erase(itr);

            tempTargetUnitMap.sort(TargetDistanceOrderNear(pUnitTarget));

            t = unMaxTargets - 1;
            Unit* prev = pUnitTarget;
            UnitList::iterator next = tempTargetUnitMap.begin();

            while (t && next != tempTargetUnitMap.end())
            {
                if (!prev->IsWithinDist(*next, CHAIN_SPELL_JUMP_RADIUS))
                    break;

                if (!prev->IsWithinLOSInMap(*next))
                {
                    ++next;
                    continue;
                }
                prev = *next;
                tempUnitList.push_back(prev);
                tempTargetUnitMap.erase(next);
                tempTargetUnitMap.sort(TargetDistanceOrderNear(prev));
                next = tempTargetUnitMap.begin();
                --t;
            }
            break;
        }
        case TARGET_UNIT_CASTER_PET:
        {
            Pet* tmpUnit = m_caster->GetPet();
            if (tmpUnit)
            {
                tempUnitList.push_back(tmpUnit);
            }
            break;
        }
        case TARGET_UNIT:
        {
            Unit* newUnitTarget = m_targets.getUnitTarget();
            if (!newUnitTarget)
                break;

            if (m_caster->CanAssistSpell(newUnitTarget, m_spellInfo))
                tempUnitList.push_back(newUnitTarget);
            else
            {
                if (!CheckAndAddMagnetTarget(newUnitTarget, effIndex, targetB, targetingData))
                    tempUnitList.push_back(newUnitTarget);
            }
            break;
        }
        case TARGET_UNIT_ENEMY:
        {
            Unit* newUnitTarget = m_targets.getUnitTarget();
            if (!newUnitTarget)
                break;

            // Check if target have Grounding Totem Aura(Magnet target). Check for physical school inside included.
            if (CheckAndAddMagnetTarget(newUnitTarget, effIndex, targetB, targetingData))
                break;

            tempUnitList.push_back(newUnitTarget);

            // More than one target
            if (EffectChainTarget > 1)
            {
                WorldObject* originalCaster = GetAffectiveCasterObject();
                if (!originalCaster)
                    break;

                if (m_caster->GetTypeId() == TYPEID_PLAYER)
                {
                    Player* me = (Player*)m_caster;
                    Player* targetOwner = newUnitTarget->GetBeneficiaryPlayer();
                    if (targetOwner && targetOwner != me && targetOwner->IsPvP() && !me->IsInDuelWith(targetOwner))
                    {
                        me->UpdatePvP(true);
                        me->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
                    }
                }

                // Getting spell casting distance
                float minRadiusCaster = 0, maxRadiusTarget = 0, jumpRadius = CHAIN_SPELL_JUMP_RADIUS;
                GetChainJumpRange(m_spellInfo, effIndex, minRadiusCaster, maxRadiusTarget, jumpRadius);

                // Filling target map
                UnitList unsteadyTargetMap;
                {
                    SpellNotifyPushType pushType = PUSH_TARGET_CENTER;
                    FillAreaTargets(unsteadyTargetMap, maxRadiusTarget, cone, pushType, SPELL_TARGETS_AOE_ATTACKABLE);
                    unsteadyTargetMap.remove(m_caster);
                    unsteadyTargetMap.remove(newUnitTarget);
                }

                // No targets. No need to process.
                if (unsteadyTargetMap.empty())
                    break;

                if (minRadiusCaster)
                {
                    for (auto iter = unsteadyTargetMap.begin(); iter != unsteadyTargetMap.end();)
                    {
                        if ((*iter)->IsWithinDist(m_caster, minRadiusCaster))
                            unsteadyTargetMap.erase(iter++);
                        else
                            ++iter;
                    }
                }

                bool ignoreLos = IsIgnoreLosSpellEffect(m_spellInfo, effIndex); // optimization

                // Removing not matched units
                for (UnitList::iterator activeUnit = unsteadyTargetMap.begin(); activeUnit != unsteadyTargetMap.end(); )
                {
                    if (!m_caster->CanAttackSpell((*activeUnit), m_spellInfo, true))
                    {
                        unsteadyTargetMap.erase(activeUnit++);
                        continue;
                    }

                    // Remove not LOS(Line of Sight) targets
                    if (!ignoreLos && !originalCaster->IsWithinLOSInMap(static_cast<WorldObject*>(*activeUnit)))
                    {
                        unsteadyTargetMap.erase(activeUnit++);
                        continue;
                    }

                    // If spell targets only players
                    if ((m_spellInfo->AttributesEx3 & SPELL_ATTR_EX3_TARGET_ONLY_PLAYER) && ((*activeUnit)->GetTypeId() != TYPEID_PLAYER))
                    {
                        unsteadyTargetMap.erase(activeUnit++);
                        continue;
                    }

                    // TODO: Mother Shahraz beams can target totems.
                    switch ((*activeUnit)->GetTypeId())
                    {
                        case TYPEID_UNIT:
                        {
                            Creature* target = static_cast<Creature*>(*activeUnit);
                            if (target->IsCritter())
                            {
                                unsteadyTargetMap.erase(activeUnit++);
                                continue;
                            }
                            break;
                        }
                        default: break;
                    }

                    ++activeUnit;
                }

                uint32 t = m_spellInfo->EffectChainTarget[effIndex] - 1;
                unsteadyTargetMap.sort(TargetDistanceOrderNear(newUnitTarget));

                if (IsChainAOESpell(m_spellInfo)) // Spell like Multi-Shot
                {
                    // Fill TargetUnitMap
                    for (UnitList::iterator activeUnit = unsteadyTargetMap.begin(); t && activeUnit != unsteadyTargetMap.end(); ++activeUnit, --t)
                        tempUnitList.push_back((*activeUnit));
                }
                else // spell like Chain Lightning
                {
                    Unit* prev = newUnitTarget;
                    UnitList::iterator next = unsteadyTargetMap.begin();

                    while (t && next != unsteadyTargetMap.end())
                    {
                        if (!prev->IsWithinDist(*next, jumpRadius))
                            break;

                        if (!prev->IsWithinLOSInMap(*next))
                        {
                            ++next;
                            continue;
                        }
                        prev = *next;
                        tempUnitList.push_back(prev);
                        unsteadyTargetMap.erase(next);
                        unsteadyTargetMap.sort(TargetDistanceOrderNear(prev));
                        next = unsteadyTargetMap.begin();

                        --t;
                    }
                }
            }
            break;
        }
        case TARGET_ENUM_UNITS_ENEMY_AOE_AT_SRC_LOC:
            FillAreaTargets(tempUnitList, radius, cone, PUSH_SRC_CENTER, SPELL_TARGETS_AOE_ATTACKABLE);
            break;
        case TARGET_ENUM_UNITS_SCRIPT_AOE_AT_SRC_LOC:
        {
            UnitList tempTargetUnitMap;
            SQLMultiStorage::SQLMSIteratorBounds<SpellTargetEntry> bounds = sSpellScriptTargetStorage.getBounds<SpellTargetEntry>(m_spellInfo->Id);

            // fill real target list if no spell script target defined
            FillAreaTargets(bounds.first != bounds.second ? tempTargetUnitMap : tempUnitList,
                            radius, cone, PUSH_SRC_CENTER, SPELL_TARGETS_ALL);

            if (!tempTargetUnitMap.empty())
                CheckSpellScriptTargets(bounds, tempTargetUnitMap, tempUnitList, effIndex);

            break;
        }
        case TARGET_ENUM_UNITS_SCRIPT_AOE_AT_DEST_LOC:
        {
            UnitList tempTargetUnitMap;
            SQLMultiStorage::SQLMSIteratorBounds<SpellTargetEntry> bounds = sSpellScriptTargetStorage.getBounds<SpellTargetEntry>(m_spellInfo->Id);
            // fill real target list if no spell script target defined
            FillAreaTargets(bounds.first != bounds.second ? tempTargetUnitMap : tempUnitList, radius, cone, PUSH_DEST_CENTER, SPELL_TARGETS_ALL);

            if (!tempTargetUnitMap.empty())
                CheckSpellScriptTargets(bounds, tempTargetUnitMap, tempUnitList, effIndex);

            break;
        }
        case TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_SRC_LOC:
        case TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_DEST_LOC:
        {
            float x, y, z;
            if (targetMode == TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_SRC_LOC)
                m_targets.getSource(x, y, z);
            else
                m_targets.getDestination(x, y, z);

            // It may be possible to fill targets for some spell effects
            // automatically (SPELL_EFFECT_WMO_REPAIR(88) for example) but
            // for some/most spells we clearly need/want to limit with spell_target_script

            // Some spells untested, for affected GO type 33. May need further adjustments for spells related.

            std::set<uint32> entriesToUse;

            SQLMultiStorage::SQLMSIteratorBounds<SpellTargetEntry> bounds = sSpellScriptTargetStorage.getBounds<SpellTargetEntry>(m_spellInfo->Id);
            for (SQLMultiStorage::SQLMultiSIterator<SpellTargetEntry> i_spellST = bounds.first; i_spellST != bounds.second; ++i_spellST)
            {
                if (i_spellST->CanNotHitWithSpellEffect(effIndex))
                    continue;

                if (i_spellST->type == SPELL_TARGET_TYPE_GAMEOBJECT)
                    entriesToUse.insert(i_spellST->targetEntry);
            }

            if (!entriesToUse.empty())
            {
                MaNGOS::AllGameObjectEntriesListInPosRangeCheck go_check(x, y, z, entriesToUse, radius);
                MaNGOS::GameObjectListSearcher<MaNGOS::AllGameObjectEntriesListInPosRangeCheck> checker(tempGOList, go_check);
                Cell::VisitGridObjects(m_caster, checker, radius);
            }

            break;
        }
        case TARGET_ENUM_UNITS_ENEMY_AOE_AT_DEST_LOC:
        {
            // targets the ground, not the units in the area
            FillAreaTargets(tempUnitList, radius, cone, PUSH_DEST_CENTER, SPELL_TARGETS_AOE_ATTACKABLE);
            break;
        }
        case TARGET_ENUM_UNITS_PARTY_WITHIN_CASTER_RANGE:
        case TARGET_ENUM_UNITS_PARTY_AOE_AT_DEST_LOC:
        case TARGET_ENUM_UNITS_PARTY_AOE_AT_SRC_LOC:
        {
            FillRaidOrPartyTargets(tempUnitList, m_caster, radius, false, true, true);
            break;
        }
        case TARGET_ENUM_UNITS_RAID_WITHIN_CASTER_RANGE:
        {
            FillRaidOrPartyTargets(tempUnitList, m_caster, radius, true, true, IsPositiveSpell(m_spellInfo->Id));
            break;
        }
        case TARGET_UNIT_FRIEND:
        case TARGET_UNIT_RAID:
        {
            if (Unit* target = m_targets.getUnitTarget())
            {
                if (m_caster->CanAssistSpell(target, m_spellInfo))
                    tempUnitList.push_back(target);
            }
            break;
        }
        case TARGET_LOCATION_CASTER_SRC:
        {
            // Check original caster is GO - set its coordinates as src cast
            if (WorldObject* caster = GetCastingObject())
                m_targets.setSource(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ());
            break;
        }
        case TARGET_ENUM_UNITS_ENEMY_WITHIN_CASTER_RANGE:
            FillAreaTargets(tempUnitList, radius, cone, PUSH_SELF_CENTER, SPELL_TARGETS_AOE_ATTACKABLE);
            break;
        case TARGET_ENUM_UNITS_FRIEND_AOE_AT_SRC_LOC:
            // selected friendly units (for casting objects) around casting object
            FillAreaTargets(tempUnitList, radius, cone, PUSH_SRC_CENTER, SPELL_TARGETS_ASSISTABLE, GetCastingObject());
            break;
        case TARGET_ENUM_UNITS_FRIEND_AOE_AT_DEST_LOC:
            FillAreaTargets(tempUnitList, radius, cone, PUSH_DEST_CENTER, SPELL_TARGETS_ASSISTABLE);
            break;
        // TARGET_UNIT_PARTY means that the spells can only be casted on a party member and not on the caster (some seals, fire shield from imp, etc..)
        case TARGET_UNIT_PARTY:
        {
            Unit* target = m_targets.getUnitTarget();
            // Those spells apparently can't be casted on the caster.
            if (target && target != m_caster)
            {
                // Can only be casted on group's members or its pets
                Group*  pGroup = nullptr;

                Unit* owner = m_caster->GetMaster();
                Unit* targetOwner = target->GetMaster();
                if (owner)
                {
                    if (owner->GetTypeId() == TYPEID_PLAYER)
                    {
                        if (target == owner)
                        {
                            tempUnitList.push_back(target);
                            break;
                        }
                        pGroup = ((Player*)owner)->GetGroup();
                    }
                }
                else if (m_caster->GetTypeId() == TYPEID_PLAYER)
                {
                    if (targetOwner == m_caster && target->GetTypeId() == TYPEID_UNIT && ((Creature*)target)->IsPet())
                    {
                        tempUnitList.push_back(target);
                        break;
                    }
                    pGroup = ((Player*)m_caster)->GetGroup();
                }

                if (pGroup)
                {
                    // Our target can also be a player's pet who's grouped with us or our pet. But can't be controlled player
                    if (targetOwner)
                    {
                        if (targetOwner->GetTypeId() == TYPEID_PLAYER &&
                                target->GetTypeId() == TYPEID_UNIT && (((Creature*)target)->IsPet()) &&
                                target->GetOwnerGuid() == targetOwner->GetObjectGuid() &&
                                pGroup->IsMember(((Player*)targetOwner)->GetObjectGuid()))
                        {
                            tempUnitList.push_back(target);
                        }
                    }
                    // 1Our target can be a player who is on our group
                    else if (target->GetTypeId() == TYPEID_PLAYER && pGroup->IsMember(((Player*)target)->GetObjectGuid()))
                    {
                        tempUnitList.push_back(target);
                    }
                }
            }
            break;
        }
        case TARGET_GAMEOBJECT:
            if (GameObject* go = m_targets.getGOTarget())
                tempGOList.push_back(go);
            break;
        case TARGET_ENUM_UNITS_ENEMY_IN_CONE_24:
        case TARGET_ENUM_UNITS_ENEMY_IN_CONE_54:
        case TARGET_ENUM_UNITS_FRIEND_IN_CONE:
        case TARGET_ENUM_UNITS_SCRIPT_IN_CONE_60:
        {
            SpellTargets targetType = SPELL_TARGETS_ALL;
            switch (targetMode)
            {
                case TARGET_ENUM_UNITS_ENEMY_IN_CONE_24:
                case TARGET_ENUM_UNITS_ENEMY_IN_CONE_54: targetType = SPELL_TARGETS_AOE_ATTACKABLE; break;
                case TARGET_ENUM_UNITS_FRIEND_IN_CONE: targetType = SPELL_TARGETS_ASSISTABLE; break;
            }

            switch (targetMode)
            {
                case TARGET_ENUM_UNITS_SCRIPT_IN_CONE_60:
                {
                    UnitList tempTargetUnitMap;
                    SQLMultiStorage::SQLMSIteratorBounds<SpellTargetEntry> bounds = sSpellScriptTargetStorage.getBounds<SpellTargetEntry>(m_spellInfo->Id);
                    // fill real target list if no spell script target defined
                    FillAreaTargets(bounds.first != bounds.second ? tempTargetUnitMap : tempUnitList, radius, cone, PUSH_CONE, targetType);
                    if (!tempTargetUnitMap.empty())
                        CheckSpellScriptTargets(bounds, tempTargetUnitMap, tempUnitList, effIndex);
                    break;
                }
                default:
                    FillAreaTargets(tempUnitList, radius, cone, PUSH_CONE, targetType);
                    break;
            }
            break;
        }
        case TARGET_LOCKED:
            if (GameObject* go = m_targets.getGOTarget())
                tempGOList.push_back(go);
            else if (Item* item = m_targets.getItemTarget())
                data.tempItemList.push_back(item);
            break;
        case TARGET_UNIT_CASTER_MASTER:
            if (Unit* owner = m_caster->GetMaster())
                tempUnitList.push_back(owner);
            break;
        case TARGET_ENUM_UNITS_ENEMY_AOE_AT_DYNOBJ_LOC:
            // targets the ground, not the units in the area
            FillAreaTargets(tempUnitList, radius, cone, PUSH_DEST_CENTER, SPELL_TARGETS_AOE_ATTACKABLE);
            break;
        case TARGET_UNIT_FRIEND_AND_PARTY:
        {
            Unit* owner = m_caster->GetMaster();
            Player* pTarget = nullptr;

            if (owner)
            {
                tempUnitList.push_back(m_caster);
                if (owner->GetTypeId() == TYPEID_PLAYER)
                    pTarget = (Player*)owner;
            }
            else if (m_caster->GetTypeId() == TYPEID_PLAYER)
            {
                if (Unit* target = m_targets.getUnitTarget())
                {
                    if (target->GetTypeId() != TYPEID_PLAYER)
                    {
                        if (((Creature*)target)->IsPet())
                        {
                            Unit* targetOwner = target->GetOwner();
                            if (targetOwner->GetTypeId() == TYPEID_PLAYER)
                                pTarget = (Player*)targetOwner;
                        }
                    }
                    else
                        pTarget = (Player*)target;
                }
            }

            Group* pGroup = pTarget ? pTarget->GetGroup() : nullptr;

            if (pGroup)
            {
                uint8 subgroup = pTarget->GetSubGroup();

                for (GroupReference* itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
                {
                    Player* Target = itr->getSource();

                    // CanAssist check duel and controlled by enemy
                    if (Target && Target->GetSubGroup() == subgroup && m_caster->CanAssistSpell(Target, m_spellInfo))
                    {
                        if (pTarget->IsWithinDistInMap(Target, radius))
                            tempUnitList.push_back(Target);

                        if (Pet* pet = Target->GetPet())
                            if (pTarget->IsWithinDistInMap(pet, radius))
                                tempUnitList.push_back(pet);
                    }
                }
            }
            else if (owner)
            {
                if (m_caster->IsWithinDistInMap(owner, radius))
                    tempUnitList.push_back(owner);
            }
            else if (pTarget)
            {
                tempUnitList.push_back(pTarget);

                if (Pet* pet = pTarget->GetPet())
                    if (m_caster->IsWithinDistInMap(pet, radius))
                        tempUnitList.push_back(pet);
            }
            break;
        }
        case TARGET_UNIT_FRIEND_CHAIN_HEAL:
        {
            Unit* pUnitTarget = m_targets.getUnitTarget();
            if (!pUnitTarget)
                break;

            if (!m_caster->CanAssistSpell(pUnitTarget, m_spellInfo))
                break;

            if (EffectChainTarget <= 1)
                tempUnitList.push_back(pUnitTarget);
            else
            {
                unMaxTargets = EffectChainTarget;
                float max_range = radius + unMaxTargets * CHAIN_SPELL_JUMP_RADIUS;

                UnitList tempTargetUnitMap;

                Group* group = nullptr;

                //if target and caster are members in the same group then apply member only filtering
                //in regards to mc by player concerns the mc'ed player has simple toolbar thus chain heal not available
                //TODO: in regards to mc by npc concerns this is something that needs to be answered
                if (m_caster->GetTypeId() == TYPEID_PLAYER)
                {
                    if (Group* casterGroup = static_cast<Player*>(m_caster)->GetGroup())
                    {
                        switch (pUnitTarget->GetTypeId())
                        {
                            case TYPEID_PLAYER:
                            {
                                if (casterGroup == static_cast<Player*>(pUnitTarget)->GetGroup())
                                    group = casterGroup;

                                break;
                            }
                            case TYPEID_UNIT:
                            {
                                Creature* creature = static_cast<Creature*>(pUnitTarget);

                                if (Unit* owner = creature->GetOwner(nullptr, true))
                                {
                                    if (owner->GetTypeId() == TYPEID_PLAYER && casterGroup == static_cast<Player*>(owner)->GetGroup())
                                    {
                                        group = casterGroup;
                                    }
                                }

                                break;
                            }
                        }

                    }
                }

                MaNGOS::AnyFriendlyOrGroupMemberUnitInUnitRangeCheck u_check(m_caster, group, m_spellInfo, max_range);
                MaNGOS::UnitListSearcher<MaNGOS::AnyFriendlyOrGroupMemberUnitInUnitRangeCheck> searcher(tempTargetUnitMap, u_check);
                Cell::VisitAllObjects(m_caster, searcher, max_range);

                tempTargetUnitMap.sort(LowestHPNearestOrder(pUnitTarget, DIST_CALC_COMBAT_REACH_WITH_MELEE));

                if (tempTargetUnitMap.empty())
                    break;

                //remove root target unit since it already defaults to the first target
                tempTargetUnitMap.remove(pUnitTarget);

                tempUnitList.push_back(pUnitTarget);

                uint32 t = unMaxTargets - 1;
                Unit* prev = pUnitTarget;
                UnitList::iterator next = tempTargetUnitMap.begin();

                while (t && next != tempTargetUnitMap.end())
                {
                    if (!prev->IsWithinCombatDist(*next, CHAIN_SPELL_JUMP_RADIUS))
                    {
                        ++next;
                        continue;
                    }

                    if (!prev->IsWithinLOSInMap(*next, true))
                    {
                        ++next;
                        continue;
                    }

                    if ((*next)->GetHealth() == (*next)->GetMaxHealth())
                    {
                        next = tempTargetUnitMap.erase(next);
                        continue;
                    }

                    prev = *next;
                    tempUnitList.push_back(prev);
                    tempTargetUnitMap.erase(next);
                    tempTargetUnitMap.sort(LowestHPNearestOrder(prev, DIST_CALC_COMBAT_REACH_WITH_MELEE));
                    next = tempTargetUnitMap.begin();

                    --t;
                }
            }
            break;
        }
        case TARGET_UNIT_RAID_AND_CLASS:
        {
            Player* targetPlayer = m_targets.getUnitTarget() && m_targets.getUnitTarget()->GetTypeId() == TYPEID_PLAYER
                                   ? (Player*)m_targets.getUnitTarget() : nullptr;

            Group* pGroup = targetPlayer ? targetPlayer->GetGroup() : nullptr;
            if (pGroup)
            {
                float x, y, z;
                targetPlayer->GetPosition(x, y, z);
                for (GroupReference* itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
                {
                    Player* target = itr->getSource();

                    // CanAssist check duel and controlled by enemy
                    if (target)
                    {
                        if (target->GetDistance(x, y, z, DIST_CALC_COMBAT_REACH) <= radius &&
                            targetPlayer->getClass() == target->getClass() &&
                            m_caster->CanAssistSpell(target, m_spellInfo))
                        {
                            tempUnitList.push_back(target);
                        }
                        if (Pet* pet = target->GetPet())
                        {
                            if (pet->GetDistance(x, y, z, DIST_CALC_COMBAT_REACH) <= radius &&
                                targetPlayer->getClass() == pet->getClass() &&
                                m_caster->CanAssistSpell(pet, m_spellInfo))
                                tempUnitList.push_back(pet);
                        }
                    }
                }
            }
            else if (m_targets.getUnitTarget())
                tempUnitList.push_back(m_targets.getUnitTarget());
            break;
        }
        case TARGET_UNIT_SCRIPT_NEAR_CASTER:
        case TARGET_GAMEOBJECT_SCRIPT_NEAR_CASTER:
        case TARGET_LOCATION_SCRIPT_NEAR_CASTER:
        {
            SQLMultiStorage::SQLMSIteratorBounds<SpellTargetEntry> bounds = sSpellScriptTargetStorage.getBounds<SpellTargetEntry>(m_spellInfo->Id);

            if (bounds.first == bounds.second)
            {
                if (m_spellInfo->EffectImplicitTargetA[effIndex] == TARGET_UNIT_SCRIPT_NEAR_CASTER || m_spellInfo->EffectImplicitTargetB[effIndex] == TARGET_UNIT_SCRIPT_NEAR_CASTER)
                    sLog.outErrorDb("Spell entry %u, effect %i has EffectImplicitTargetA/EffectImplicitTargetB = TARGET_UNIT_SCRIPT_NEAR_CASTER, but creature are not defined in `spell_script_target`", m_spellInfo->Id, effIndex);

                if (m_spellInfo->EffectImplicitTargetA[effIndex] == TARGET_LOCATION_SCRIPT_NEAR_CASTER || m_spellInfo->EffectImplicitTargetB[effIndex] == TARGET_LOCATION_SCRIPT_NEAR_CASTER)
                    sLog.outErrorDb("Spell entry %u, effect %i has EffectImplicitTargetA/EffectImplicitTargetB = TARGET_LOCATION_SCRIPT_NEAR_CASTER, but gameobject or creature are not defined in `spell_script_target`", m_spellInfo->Id, effIndex);

                if (m_spellInfo->EffectImplicitTargetA[effIndex] == TARGET_GAMEOBJECT_SCRIPT_NEAR_CASTER || m_spellInfo->EffectImplicitTargetB[effIndex] == TARGET_GAMEOBJECT_SCRIPT_NEAR_CASTER)
                    sLog.outErrorDb("Spell entry %u, effect %i has EffectImplicitTargetA/EffectImplicitTargetB = TARGET_GAMEOBJECT_SCRIPT_NEAR_CASTER, but gameobject are not defined in `spell_script_target`", m_spellInfo->Id, effIndex);

                break;
            }

            CreatureList foundScriptCreatureTargets;
            GameObjectList foundScriptGOTargets;
            std::set<uint32> entriesToUse;
            uint32 type = MAX_SPELL_TARGET_TYPE;
            bool foundButOutOfRange = false;
            // corrections for numerous spells missing it
            uint32 targetCount = std::max(EffectChainTarget, uint32(1));
            if (radius == 0.f)
                radius = m_caster->GetMap()->IsDungeon() ? DEFAULT_VISIBILITY_INSTANCE : DEFAULT_VISIBILITY_DISTANCE;

            for (SQLMultiStorage::SQLMultiSIterator<SpellTargetEntry> i_spellST = bounds.first; i_spellST != bounds.second; ++i_spellST)
            {
                if (i_spellST->CanNotHitWithSpellEffect(effIndex))
                    continue;

                type = i_spellST->type;
                switch (i_spellST->type)
                {
                    case SPELL_TARGET_TYPE_GAMEOBJECT:
                    case SPELL_TARGET_TYPE_CREATURE:
                    case SPELL_TARGET_TYPE_DEAD:
                    case SPELL_TARGET_TYPE_CREATURE_GUID:
                    default:
                        entriesToUse.insert(i_spellST->targetEntry);
                        break;
                }
            }

            WorldObject* caster = GetAffectiveCasterObject();

            switch (type) // TODO: Unify logic with all other spell_script_target uses
            {
                case SPELL_TARGET_TYPE_GAMEOBJECT:
                {
                    MaNGOS::AllGameObjectEntriesListInObjectRangeCheck go_check(*m_caster, entriesToUse, radius);
                    MaNGOS::GameObjectListSearcher<MaNGOS::AllGameObjectEntriesListInObjectRangeCheck> checker(foundScriptGOTargets, go_check);
                    Cell::VisitGridObjects(m_caster, checker, radius);
                    for (auto itr = foundScriptGOTargets.begin(); itr != foundScriptGOTargets.end();)
                    {
                        if (!CheckTargetGOScript(*itr, effIndex))
                            itr = foundScriptGOTargets.erase(itr);
                        else
                            ++itr;
                    }
                    foundScriptGOTargets.sort([=](const GameObject * a, const GameObject * b) -> bool
                    {
                        return caster->GetDistance(a, true, DIST_CALC_NONE) < caster->GetDistance(b, true, DIST_CALC_NONE);
                    });

                    if (foundScriptGOTargets.size() > targetCount) // if we have too many targets, we need to trim the list
                        foundScriptGOTargets.resize(targetCount);
                    break;
                }
                case SPELL_TARGET_TYPE_CREATURE:
                case SPELL_TARGET_TYPE_DEAD:
                case SPELL_TARGET_TYPE_CREATURE_GUID:
                {
                    if (Unit* target = m_targets.getUnitTarget())
                    {
                        if (target->GetTypeId() == TYPEID_UNIT && entriesToUse.find(type == SPELL_TARGET_TYPE_CREATURE_GUID ? target->GetGUIDLow() : target->GetEntry()) != entriesToUse.end())
                        {
                            if ((type == SPELL_TARGET_TYPE_CREATURE && target->isAlive()) ||
                                (type == SPELL_TARGET_TYPE_DEAD && ((Creature*)target)->IsCorpse()))
                            {
                                if (target->IsWithinDistInMap(caster, radius))
                                    foundScriptCreatureTargets.push_back((Creature*)target);
                                else
                                    foundButOutOfRange = true;
                            }
                        }
                    }
                    if (foundScriptCreatureTargets.size() < targetCount)
                    {
                        MaNGOS::AllCreatureEntriesWithLiveStateInObjectRangeCheck u_check(*caster, entriesToUse, type != SPELL_TARGET_TYPE_DEAD, radius, type == SPELL_TARGET_TYPE_CREATURE_GUID, false, true);
                        MaNGOS::CreatureListSearcher<MaNGOS::AllCreatureEntriesWithLiveStateInObjectRangeCheck> searcher(foundScriptCreatureTargets, u_check);

                        // Visit all, need to find also Pet* objects
                        Cell::VisitAllObjects(caster, searcher, radius);
                        for (auto iter = foundScriptCreatureTargets.begin(); iter != foundScriptCreatureTargets.end();)
                        {
                            bool failed = false;
                            if (!CheckTargetScript(*iter, effIndex))
                                failed = true;
                            else if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_CANT_TARGET_SELF) && m_caster == (*iter))
                                failed = true;
                            if (failed)
                                iter = foundScriptCreatureTargets.erase(iter);
                            else
                                ++iter;
                        }

                        foundScriptCreatureTargets.sort([=](const Creature * a, const Creature * b) -> bool
                        {
                            return caster->GetDistance(a, true, DIST_CALC_NONE) < caster->GetDistance(b, true, DIST_CALC_NONE);
                        });

                        if (foundScriptCreatureTargets.size() > targetCount) // if we have too many targets, we need to trim the list
                            foundScriptCreatureTargets.resize(targetCount);

                        if (u_check.FoundOutOfRange())
                            foundButOutOfRange = true;
                    }
                    break;
                }
                default:
                    break;
            }

            if (!foundScriptCreatureTargets.empty())
            {
                // store coordinates for TARGET_LOCATION_SCRIPT_NEAR_CASTER
                if (targetMode == TARGET_LOCATION_SCRIPT_NEAR_CASTER)
                {
                    Creature* creatureScriptTarget = foundScriptCreatureTargets.front(); // can only have one target in this case
                    m_targets.setDestination(creatureScriptTarget->GetPositionX(), creatureScriptTarget->GetPositionY(), creatureScriptTarget->GetPositionZ());
                }
                // store explicit target for TARGET_UNIT_SCRIPT_NEAR_CASTER
                else
                    for (Creature* creature : foundScriptCreatureTargets)
                        tempUnitList.push_back(creature);
            }
            else if (!foundScriptGOTargets.empty())
            {
                // store coordinates for TARGET_LOCATION_SCRIPT_NEAR_CASTER
                if (targetMode == TARGET_LOCATION_SCRIPT_NEAR_CASTER)
                {
                    GameObject* goScriptTarget = foundScriptGOTargets.front(); // can only have one target in this case
                    m_targets.setDestination(goScriptTarget->GetPositionX(), goScriptTarget->GetPositionY(), goScriptTarget->GetPositionZ());
                }
                // store explicit target for TARGET_GAMEOBJECT_SCRIPT_NEAR_CASTER
                else
                    tempGOList.splice(tempGOList.end(), foundScriptGOTargets);
            }
            // Missing DB Entry or targets for this spellEffect.
            else
            {
                /* For TARGET_GAMEOBJECT_SCRIPT_NEAR_CASTER makes DB targets optional not required for now
                 * TODO: Makes more research for this target type
                 */
                if (targetMode != TARGET_GAMEOBJECT_SCRIPT_NEAR_CASTER)
                {
                    // not report target not existence for triggered spells
                    SpellCastResult result = SPELL_CAST_OK;
                    if (m_triggeredByAuraSpell || m_IsTriggeredSpell)
                        result = SPELL_FAILED_DONT_REPORT;
                    result = foundButOutOfRange ? SPELL_FAILED_OUT_OF_RANGE : SPELL_FAILED_BAD_TARGETS;
                    SendCastResult(result);
                    finish(false);
                    return;
                }
            }
            break;
        }
        default:
            // sLog.outError( "SPELL: Unknown implicit target (%u) for spell ID %u", targetMode, m_spellInfo->Id );
            break;
    }

    // remove caster from the list if required by attribute
    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_CANT_TARGET_SELF))
        if (targetMode != TARGET_UNIT_CASTER)
            tempUnitList.remove(m_caster);
}

void Spell::CheckSpellScriptTargets(SQLMultiStorage::SQLMSIteratorBounds<SpellTargetEntry>& bounds, UnitList& tempTargetUnitMap, UnitList& targetUnitMap, SpellEffectIndex effIndex)
{
    for (UnitList::const_iterator iter = tempTargetUnitMap.begin(); iter != tempTargetUnitMap.end(); ++iter)
    {
        Unit* target = (*iter);
        if (target->GetTypeId() != TYPEID_UNIT)
            continue;

        for (SQLMultiStorage::SQLMultiSIterator<SpellTargetEntry> i_spellST = bounds.first; i_spellST != bounds.second; ++i_spellST)
        {
            SpellTargetEntry const* spellST = (*i_spellST);
            if (spellST->CanNotHitWithSpellEffect(effIndex))
                continue;

            // only creature entries supported for this target type
            if (spellST->type == SPELL_TARGET_TYPE_GAMEOBJECT)
                continue;

            if (spellST->type == SPELL_TARGET_TYPE_CREATURE_GUID ? target->GetGUIDLow() == spellST->targetEntry : target->GetEntry() == spellST->targetEntry)
            {
                switch (spellST->type)
                {
                    case SPELL_TARGET_TYPE_DEAD:
                        if (static_cast<Creature*>(target)->IsCorpse())
                            targetUnitMap.push_back(target);
                        break;
                    case SPELL_TARGET_TYPE_CREATURE:
                        if (target->isAlive())
                            targetUnitMap.push_back(target);
                        break;
                    case SPELL_TARGET_TYPE_CREATURE_GUID:
                        targetUnitMap.push_back(target);
                        break;
                    default:
                        break;
                }
                break;
            }
        }
    }
}

SpellCastResult Spell::PreCastCheck(Aura* triggeredByAura /*= nullptr*/)
{
    SpellCastResult result = CheckCast(true);
    if (result != SPELL_CAST_OK && (!IsAutoRepeat() || m_triggerAutorepeat)) // always cast autorepeat dummy for triggering
    {
        if (triggeredByAura)
        {
            SendChannelUpdate(0);
            triggeredByAura->GetHolder()->SetAuraDuration(0);
        }
        return result;
    }

    return SPELL_CAST_OK;
}

SpellCastResult Spell::SpellStart(SpellCastTargets const* targets, Aura* triggeredByAura)
{
    m_spellState = SPELL_STATE_TARGETING;
    m_targets = *targets;

    if (m_CastItem)
        m_CastItemGuid = m_CastItem->GetObjectGuid();


    m_castPositionX = m_caster->GetPositionX();
    m_castPositionY = m_caster->GetPositionY();
    m_castPositionZ = m_caster->GetPositionZ();
    m_castOrientation = m_caster->GetOrientation();

    if (triggeredByAura)
        m_triggeredByAuraSpell = triggeredByAura->GetSpellProto();

    // create and add update event for this spell
    SpellEvent* Event = new SpellEvent(this);
    m_caster->m_events.AddEvent(Event, m_caster->m_events.CalculateTime(1));

    // Fill cost data
    m_powerCost = m_IsTriggeredSpell ? 0 : CalculatePowerCost(m_spellInfo, m_caster, this, m_CastItem);

    SpellCastResult result = PreCastCheck();
    if (result != SPELL_CAST_OK)
    {
        SendCastResult(result);
        finish(false);
        return result;
    }
    Prepare();

    return SPELL_CAST_OK;
}

void Spell::Prepare()
{
    OnSuccessfulSpellStart();

    m_spellState = SPELL_STATE_CASTING;

    // Prepare data for triggers
    prepareDataForTriggerSystem();

    // calculate cast time (calculated after first CheckCast check to prevent charge counting for first CheckCast fail)
    if (!m_ignoreCastTime)
        m_casttime = GetSpellCastTime(m_spellInfo, m_caster, this);

    m_duration = CalculateSpellDuration(m_spellInfo, m_caster);

    // set timer base at cast time
    ReSetTimer();

    if (!m_IsTriggeredSpell)
    {
        m_caster->RemoveAurasOnCast(m_spellInfo);

        // Orientation changes inside
        if (m_notifyAI && m_caster->AI())
            m_caster->AI()->OnSpellCastStateChange(this, true, m_targets.getUnitTarget());
    }

    // add non-triggered (with cast time and without)
    if (!m_IsTriggeredSpell)
    {
        // add to cast type slot
        if((!m_spellInfo->HasAttribute(SPELL_ATTR_EX4_CAN_CAST_WHILE_CASTING) || IsChanneledSpell(m_spellInfo)) && !m_triggerAutorepeat)
            m_caster->SetCurrentCastedSpell(this);

        // will show cast bar
        SendSpellStart();

        // add gcd server side (client side is handled by client itself)
        m_caster->AddGCD(*m_spellInfo);

        // Execute instant spells immediate
        if (m_timer == 0 && !IsNextMeleeSwingSpell() && (!IsAutoRepeat() || m_triggerAutorepeat))
            cast();
    }
    // execute triggered without cast time explicitly in call point
    else
    {
        // Channeled spell is always one per caster and needs to be tracked and removed on death
        if (IsChanneledSpell(m_spellInfo))
            m_caster->SetCurrentCastedSpell(this);

        if (m_timer == 0)
            cast(true);
    }
    // else triggered with cast time will execute execute at next tick or later
    // without adding to cast type slot
    // will not show cast bar but will show effects at casting time etc
}

void Spell::cancel()
{
    if (m_spellState == SPELL_STATE_FINISHED)
        return;

    // channeled spells don't display interrupted message even if they are interrupted, possible other cases with no "Interrupted" message
    bool sendInterrupt = !(IsChanneledSpell(m_spellInfo) || m_autoRepeat);

    m_autoRepeat = false;
    switch (m_spellState)
    {
        case SPELL_STATE_CREATED:
        case SPELL_STATE_TARGETING:
        case SPELL_STATE_CASTING:
            m_caster->ResetGCD(m_spellInfo);

        //(no break)
        case SPELL_STATE_TRAVELING:
        {
            SendInterrupted(SPELL_FAILED_INTERRUPTED);
            if (sendInterrupt)
                SendCastResult(SPELL_FAILED_INTERRUPTED);
        } break;

        case SPELL_STATE_LANDING:
            sLog.outError("Spell [%u] is interrupted while processing", m_spellInfo->Id);
        // no break

        case SPELL_STATE_CHANNELING:
        {
            for (auto& ihit : m_UniqueTargetInfo)
            {
                if (ihit.missCondition == SPELL_MISS_NONE)
                {
                    Unit* unit = m_caster->GetObjectGuid() == ihit.targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit.targetGUID);
                    if (unit && unit->isAlive())
                        unit->RemoveAurasByCasterSpell(m_spellInfo->Id, m_caster->GetObjectGuid());
                }
            }
            if (m_spellInfo->HasAttribute(SPELL_ATTR_DISABLED_WHILE_ACTIVE))
                m_caster->RemoveSpellCooldown(*m_spellInfo, true); // need to clear cooldown in client

            SendChannelUpdate(0);
            SendInterrupted(SPELL_FAILED_INTERRUPTED);
            if (sendInterrupt)
                SendCastResult(SPELL_FAILED_INTERRUPTED);
        } break;

        case SPELL_STATE_FINISHED: break; // should not occur
    }

    finish(false);
    m_caster->RemoveDynObject(m_spellInfo->Id);
    m_caster->RemoveGameObject(m_spellInfo->Id, true);
}

void Spell::cast(bool skipCheck)
{
    SetExecutedCurrently(true);

    if (m_notifyAI && m_caster->AI())
        m_caster->AI()->OnSpellCastStateChange(this, false);

    if (!m_caster->CheckAndIncreaseCastCounter())
    {
        if (m_triggeredByAuraSpell)
            sLog.outError("Spell %u triggered by aura spell %u too deep in cast chain for cast. Cast not allowed for prevent overflow stack crash.", m_spellInfo->Id, m_triggeredByAuraSpell->Id);
        else
            sLog.outError("Spell %u too deep in cast chain for cast. Cast not allowed for prevent overflow stack crash.", m_spellInfo->Id);

        SendCastResult(SPELL_FAILED_ERROR);
        finish(false);
        SetExecutedCurrently(false);
        return;
    }

    // update pointers base at GUIDs to prevent access to already nonexistent object
    UpdatePointers();

    // cancel at lost main target unit
    if (!m_targets.getUnitTarget() && m_targets.getUnitTargetGuid() && m_targets.getUnitTargetGuid() != m_caster->GetObjectGuid())
    {
        cancel();
        m_caster->DecreaseCastCounter();
        SetExecutedCurrently(false);
        return;
    }

    if (m_caster->GetTypeId() != TYPEID_PLAYER && m_targets.getUnitTarget() && m_targets.getUnitTarget() != m_caster)
        m_caster->SetInFront(m_targets.getUnitTarget());

    SpellCastResult castResult = CheckPower();
    if (castResult != SPELL_CAST_OK)
    {
        StopCast(castResult);
        return;
    }

    // triggered cast called from Spell::prepare where it was already checked
    if (!skipCheck)
    {
        castResult = CheckCast(false);
        if (castResult != SPELL_CAST_OK)
        {
            StopCast(castResult);
            return;
        }
    }

    // different triggered (for caster) and pre-cast (casted before apply effect to each target) cases
    switch (m_spellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            // Bandages
            if (m_spellInfo->Mechanic == MECHANIC_BANDAGE)
                AddPrecastSpell(11196);                     // Recently Bandaged
            // Divine Shield, Divine Protection (Blessing of Protection in paladin switch case)
            else if (m_spellInfo->Mechanic == MECHANIC_INVULNERABILITY)
                AddPrecastSpell(25771);                     // Forbearance
            break;
        }
        case SPELLFAMILY_WARRIOR:
            break;
        case SPELLFAMILY_PRIEST:
        {
            // Power Word: Shield
            if (m_spellInfo->SpellFamilyName == SPELLFAMILY_PRIEST && m_spellInfo->SpellFamilyFlags & uint64(0x0000000000000001))
                AddPrecastSpell(6788);                      // Weakened Soul

            switch (m_spellInfo->Id)
            {
                case 15237: AddTriggeredSpell(23455); break;// Holy Nova, rank 1
                case 15430: AddTriggeredSpell(23458); break;// Holy Nova, rank 2
                case 15431: AddTriggeredSpell(23459); break;// Holy Nova, rank 3
                case 27799: AddTriggeredSpell(27803); break;// Holy Nova, rank 4
                case 27800: AddTriggeredSpell(27804); break;// Holy Nova, rank 5
                case 27801: AddTriggeredSpell(27805); break;// Holy Nova, rank 6
                case 25331: AddTriggeredSpell(25329); break;// Holy Nova, rank 7
                default: break;
            }
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            // Blessing of Protection (Divine Shield, Divine Protection in generic switch case)
            if (m_spellInfo->Mechanic == MECHANIC_INVULNERABILITY && m_spellInfo->Id != 25771)
                AddPrecastSpell(25771);                     // Forbearance
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            if (m_spellInfo->IsFitToFamilyMask(uint64(0x0000000000000800)) && m_spellInfo->Reagent[0] == 5140) // vanish base spell
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER)
                    break;

                // get highest rank of the Stealth spell
                SpellEntry const* stealthSpellEntry = nullptr;
                const PlayerSpellMap& sp_list = ((Player*)m_caster)->GetSpellMap();
                for (const auto& itr : sp_list)
                {
                    // only highest rank is shown in spell book, so simply check if shown in spell book
                    if (!itr.second.active || itr.second.disabled || itr.second.state == PLAYERSPELL_REMOVED)
                        continue;

                    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(itr.first);
                    if (!spellInfo)
                        continue;

                    if (spellInfo->IsFitToFamily(SPELLFAMILY_ROGUE, uint64(0x0000000000400000)))
                    {
                        stealthSpellEntry = spellInfo;
                        break;
                    }
                }

                // no Stealth spell found
                if (!stealthSpellEntry)
                    return;

                // reset cooldown on it if needed
                if (!m_caster->IsSpellReady(*stealthSpellEntry))
                    m_caster->RemoveSpellCooldown(*stealthSpellEntry);

                m_caster->CastSpell(m_caster, stealthSpellEntry, TRIGGERED_OLD_TRIGGERED);
            }
            break;
        }
        default:
            break;
    }

    // traded items have trade slot instead of guid in m_itemTargetGUID
    // set to real guid to be sent later to the client
    m_targets.updateTradeSlotItem();

    FillTargetMap();

    if (m_spellState == SPELL_STATE_FINISHED)               // stop cast if spell marked as finish somewhere in FillTargetMap
    {
        m_caster->DecreaseCastCounter();
        SetExecutedCurrently(false);
        return;
    }

    // CAST SPELL
    SendSpellCooldown();
    if (m_notifyAI && m_caster->AI())
        m_caster->AI()->OnSpellCooldownAdded(m_spellInfo);

    TakePower();
    TakeReagents();                                         // we must remove reagents before HandleEffects to allow place crafted item in same slot
    TakeAmmo();

    SendCastResult(castResult);
    SendSpellGo();                                          // we must send smsg_spell_go packet before m_castItem delete in TakeCastItem()...

    InitializeDamageMultipliers();

    // process immediate effects (items, ground, etc.) also initialize some variables
    _handle_immediate_phase();

    // Okay, everything is prepared. Now we need to distinguish between immediate and evented delayed spells
    if (GetSpellSpeed() > 0.0f && !IsChanneledSpell(m_spellInfo))
    {
        // Remove used for cast item if need (it can be already nullptr after TakeReagents call
        // in case delayed spell remove item at cast delay start
        TakeCastItem();

        // fill initial spell damage from caster for delayed casted spells
        for (auto& ihit : m_UniqueTargetInfo)
            HandleDelayedSpellLaunch(&ihit);

        // For channels, delay starts at channel end
        if (m_spellState != SPELL_STATE_CHANNELING)
        {
            // Okay, maps created, now prepare flags
            m_spellState = SPELL_STATE_TRAVELING;
            SetDelayStart(0);
        }
    }
    else
    {
        // Immediate spell, no big deal
        handle_immediate();
    }

    m_caster->DecreaseCastCounter();
    SetExecutedCurrently(false);
}

void Spell::handle_immediate()
{
    if (m_spellState != SPELL_STATE_CHANNELING)
        m_spellState = SPELL_STATE_LANDING;

    DoAllTargetlessEffects(true);

    for (auto& ihit : m_UniqueTargetInfo)
        DoAllEffectOnTarget(&ihit);

    for (auto& ihit : m_UniqueGOTargetInfo)
        DoAllEffectOnTarget(&ihit);

    // spell is finished, perform some last features of the spell here
    _handle_finish_phase();

    // Remove used for cast item if need (it can be already nullptr after TakeReagents call
    TakeCastItem();

    if (m_spellState != SPELL_STATE_CHANNELING)
        finish(true);                                       // successfully finish spell cast (not last in case autorepeat or channel spell)
}

uint64 Spell::handle_delayed(uint64 t_offset)
{
    uint64 next_time = 0;

    if (!m_destTargetInfo.processed)
        if (m_destTargetInfo.timeDelay <= t_offset)
            DoAllTargetlessEffects(true);
        else if (next_time == 0 || m_destTargetInfo.timeDelay < next_time)
            next_time = m_destTargetInfo.timeDelay;

    // now recheck units targeting correctness (need before any effects apply to prevent adding immunity at first effect not allow apply second spell effect and similar cases)
    for (auto& ihit : m_UniqueTargetInfo)
    {
        if (!ihit.processed)
        {
            if (ihit.timeDelay <= t_offset)
                DoAllEffectOnTarget(&ihit);
            else if (next_time == 0 || ihit.timeDelay < next_time)
                next_time = ihit.timeDelay;
        }
    }

    // now recheck gameobject targeting correctness
    for (auto& ighit : m_UniqueGOTargetInfo)
    {
        if (!ighit.processed)
        {
            if (ighit.timeDelay <= t_offset)
                DoAllEffectOnTarget(&ighit);
            else if (next_time == 0 || ighit.timeDelay < next_time)
                next_time = ighit.timeDelay;
        }
    }
    // All targets passed - need finish phase
    if (next_time == 0)
    {
        // spell is finished, perform some last features of the spell here
        _handle_finish_phase();

        finish(true);                                       // successfully finish spell cast

        // return zero, spell is finished now
        return 0;
    }
    // spell is unfinished, return next execution time
    return next_time;
}

void Spell::_handle_immediate_phase()
{
    if (IsMeleeAttackResetSpell())
    {
        if (!m_spellInfo->HasAttribute(SPELL_ATTR_EX2_NOT_RESET_AUTO_ACTIONS))
        {
            m_caster->resetAttackTimer(BASE_ATTACK);
            if (m_caster->hasOffhandWeaponForAttack())
                m_caster->resetAttackTimer(OFF_ATTACK);
        }
    }

    // handle some immediate features of the spell here
    HandleThreatSpells();

    // initialize Diminishing Returns Data
    m_diminishLevel = DIMINISHING_LEVEL_1;
    m_diminishGroup = DIMINISHING_NONE;

    // handle none and dest targeted effects
    DoAllTargetlessEffects(false);

    // process items
    for (auto& ihit : m_UniqueItemInfo)
        DoAllEffectOnTarget(&ihit);

    // start channeling if applicable (after _handle_immediate_phase for get persistent effect dynamic object for channel target
    if (IsChanneledSpell(m_spellInfo) && m_duration)
    {
        m_spellState = SPELL_STATE_CHANNELING;
        SendChannelStart(m_duration);

        // Proc spell aura triggers on start of channeled spell
        ProcSpellAuraTriggers();
    }
}

void Spell::_handle_finish_phase()
{
    // spell log
    if (m_needSpellLog)
        m_spellLog.SendToSet();

    if (m_caster->m_extraAttacks && IsSpellHaveEffect(m_spellInfo, SPELL_EFFECT_ADD_EXTRA_ATTACKS))
    {
        if (Unit* target = m_caster->GetMap()->GetUnit(m_caster->GetSelectionGuid()))
            m_caster->DoExtraAttacks(target);
        else
            m_caster->m_extraAttacks = 0;
    }
}

void Spell::SendSpellCooldown()
{
    // (SPELL_ATTR_DISABLED_WHILE_ACTIVE) have infinity cooldown, (SPELL_ATTR_PASSIVE) passive cooldown at triggering
    if (m_spellInfo->HasAttribute(SPELL_ATTR_PASSIVE))
        return;

    m_caster->AddCooldown(*m_spellInfo, m_CastItem ? m_CastItem->GetProto() : nullptr, m_spellInfo->HasAttribute(SPELL_ATTR_DISABLED_WHILE_ACTIVE));
}

void Spell::update(uint32 difftime)
{
    // update pointers based at it's GUIDs
    UpdatePointers();

    if (m_targets.getUnitTargetGuid() && !m_targets.getUnitTarget())
    {
        cancel();
        return;
    }

    // check if the player or unit caster has moved before the spell finished (exclude casting on vehicles)
    if (((m_caster->GetTypeId() == TYPEID_PLAYER || m_caster->GetTypeId() == TYPEID_UNIT) && m_timer != 0) &&
            (m_castPositionX != m_caster->GetPositionX() || m_castPositionY != m_caster->GetPositionY() || m_castPositionZ != m_caster->GetPositionZ()) &&
            (m_spellInfo->Effect[EFFECT_INDEX_0] != SPELL_EFFECT_STUCK || !m_caster->m_movementInfo.HasMovementFlag(MOVEFLAG_FALLINGFAR)))
    {
        // always cancel for channeled spells
        if (m_spellState == SPELL_STATE_CHANNELING && m_spellInfo->ChannelInterruptFlags & CHANNEL_FLAG_MOVEMENT)
            cancel();
        // don't cancel for melee, autorepeat, triggered and instant spells
        else if (!IsNextMeleeSwingSpell() && !IsAutoRepeat() && !m_IsTriggeredSpell && (m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_MOVEMENT))
            cancel();
    }

    switch (m_spellState)
    {
        case SPELL_STATE_CASTING:
        {
            if (m_timer)
            {
                if (difftime >= m_timer)
                    m_timer = 0;
                else
                    m_timer -= difftime;
            }

            if (m_timer == 0 && !IsNextMeleeSwingSpell() && !IsAutoRepeat())
                cast();
        } break;
        case SPELL_STATE_CHANNELING:
        {
            if (m_timer > 0)
            {
                if (m_caster->GetTypeId() == TYPEID_PLAYER || m_caster->GetTypeId() == TYPEID_UNIT)
                {
                    // check if player has jumped before the channeling finished
                    if (m_caster->m_movementInfo.HasMovementFlag(MOVEFLAG_FALLING))
                        cancel();

                    // check for incapacitating player states
                    if (m_caster->hasUnitState(UNIT_STAT_CAN_NOT_REACT))
                    {
                        // certain channel spells are not interrupted
                        if (!m_spellInfo->HasAttribute(SPELL_ATTR_EX_CHANNELED_1) && !m_spellInfo->HasAttribute(SPELL_ATTR_EX3_UNK28))
                            cancel();
                    }

                    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_CHANNEL_TRACK_TARGET) && m_UniqueTargetInfo.begin() != m_UniqueTargetInfo.end())
                    {
                        if (Unit* target = m_caster->GetChannelObject())
                        {
                            if (target != m_caster)
                            {
                                float orientation = m_caster->GetAngle(target);
                                if (m_caster->GetTypeId() == TYPEID_UNIT)
                                {
                                    m_caster->SetOrientation(orientation);
                                    m_caster->SetFacingTo(orientation);
                                }
                                m_castOrientation = orientation; // need to update cast orientation due to the attribute
                            }
                        }
                    }

                    // check if player has turned if flag is set
                    if (m_spellInfo->ChannelInterruptFlags & CHANNEL_FLAG_TURNING)
                    {
                        switch (m_caster->GetTypeId())
                        {
                            case TYPEID_UNIT:
                                if (m_castOrientation != m_caster->GetOrientation())
                                    cancel();
                                break;
                            case TYPEID_PLAYER:
                            {
                                float targetOrientationDiff = 0.10f; // Diff due to client server mismatch
                                if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_CHANNEL_TRACK_TARGET)) // On start, client turns player to face object, need leeway so that we dont break unnecessarily
                                    targetOrientationDiff += M_PI_F * (std::max(1200 - (m_duration - int32(m_timer)), 0) / 1200.f);
                                if ((M_PI_F - std::abs(std::abs(m_castOrientation - m_caster->GetOrientation()) - M_PI_F)) > targetOrientationDiff)
                                    cancel();
                            }
                        }
                    }
                }

                // need check distance for channeled target only - but only when its set
                if (m_maxRange && !m_caster->HasChannelObject(m_caster->GetObjectGuid()))
                {
                    if (Unit* channelTarget = m_caster->GetChannelObject())
                    {
                        if (!m_caster->IsWithinCombatDistInMap(channelTarget, m_maxRange * 1.5f))
                        {
                            SendChannelUpdate(0);
                            finish();
                        }
                    }
                }

                // check if there are alive targets left
                if (!IsAliveUnitPresentInTargetList())
                {
                    SendChannelUpdate(0);
                    finish();
                }

                if (difftime >= m_timer)
                    m_timer = 0;
                else
                    m_timer -= difftime;
            }

            if (m_timer == 0)
            {
                SendChannelUpdate(0, difftime);

                // channeled spell processed independently for quest targeting
                // cast at creature (or GO) quest objectives update at successful cast channel finished
                // ignore autorepeat/melee casts for speed (not exist quest for spells (hm... )
                if (!IsAutoRepeat() && !IsNextMeleeSwingSpell())
                {
                    if (Player* p = m_caster->GetBeneficiaryPlayer())
                    {
                        for (TargetList::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
                        {
                            TargetInfo const& target = *ihit;
                            if (!target.targetGUID.IsCreature())
                                continue;

                            Unit* unit = m_caster->GetObjectGuid() == target.targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, target.targetGUID);
                            if (unit == nullptr)
                                continue;

                            p->RewardPlayerAndGroupAtCast(unit, m_spellInfo->Id);
                        }

                        for (GOTargetList::const_iterator ihit = m_UniqueGOTargetInfo.begin(); ihit != m_UniqueGOTargetInfo.end(); ++ihit)
                        {
                            GOTargetInfo const& target = *ihit;

                            GameObject* go = m_caster->GetMap()->GetGameObject(target.targetGUID);
                            if (!go)
                                continue;

                            p->RewardPlayerAndGroupAtCast(go, m_spellInfo->Id);
                        }
                    }
                }

                if (GetSpellSpeed() > 0.0f)
                {
                    // Okay, maps created, now prepare flags
                    m_spellState = SPELL_STATE_TRAVELING;
                    SetDelayStart(0);
                }
                else
                    finish();
            }
        } break;
        default:
        {
        } break;
    }
}

void Spell::finish(bool ok)
{
    if (!m_caster)
        return;

    if (m_spellState == SPELL_STATE_FINISHED)
        return;

    // remove/restore spell mods before m_spellState update
    if (Player* modOwner = m_caster->GetSpellModOwner())
    {
        if (ok || (m_spellState > uint32(SPELL_STATE_CASTING))) // fail after start channeling or throw to target not affect spell mods
            modOwner->RemoveSpellMods(this);
        else
            modOwner->ResetSpellModsDueToCanceledSpell(this);
    }

    bool channeledChannel = m_spellState == SPELL_STATE_CHANNELING;

    m_spellState = SPELL_STATE_FINISHED;

    // other code related only to successfully finished spells
    if (!ok)
        return;

    // Normal spells proc on finish, channeled spells proc on start when they have duration, thats when channeledChannel is true
    if (!IsChanneledSpell(m_spellInfo) && !channeledChannel)
        ProcSpellAuraTriggers();

    // Heal caster for all health leech from all targets
    if (m_healthLeech)
        m_caster->DealHeal(m_caster, uint32(m_healthLeech), m_spellInfo);

    if (m_spellInfo->AttributesEx & SPELL_ATTR_EX_REFUND_POWER)
    {
        for (TargetList::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
        {
            switch (ihit->missCondition)
            {
                case SPELL_MISS_MISS:
                case SPELL_MISS_DODGE:
                    if (m_spellInfo->powerType == POWER_RAGE) // For Warriors only refund on parry/deflect, for rogues on all 4
                        break;
                case SPELL_MISS_PARRY:
                case SPELL_MISS_DEFLECT:
                    m_caster->ModifyPower(Powers(m_spellInfo->powerType), int32(float(m_powerCost) * 0.8f));
                    break;
                default:
                    break;
            }
        }
    }

    OnSuccessfulSpellFinish();

    // Clear combo at finish state
    if (m_caster->GetTypeId() == TYPEID_PLAYER && NeedsComboPoints(m_spellInfo))
    {
        // Not drop combopoints if negative spell and if any miss on enemy exist
        bool needDrop = true;
        if (!IsPositiveSpell(m_spellInfo->Id))
        {
            for (TargetList::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
            {
                if (ihit->missCondition != SPELL_MISS_NONE && ihit->targetGUID != m_caster->GetObjectGuid())
                {
                    needDrop = false;
                    break;
                }
            }
        }
        if (needDrop)
            ((Player*)m_caster)->ClearComboPoints();
    }

    // call triggered spell only at successful cast (after clear combo points -> for add some if need)
    if (!m_TriggerSpells.empty())
        CastTriggerSpells();

    // Stop Attack for some spells
    if (m_spellInfo->HasAttribute(SPELL_ATTR_STOP_ATTACK_TARGET))
    {
        if (IsRangedSpell()) // blizzlike order
            m_caster->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);

        m_caster->AttackStop();

        if (m_caster->GetTypeId() == TYPEID_PLAYER)
            ((Player*)m_caster)->SendAttackSwingCancelAttack();
    }

    // update encounter state if needed
    if (m_caster->IsInWorld()) // some teleport spells put caster in between maps, need to check
    {
        Map* map = m_caster->GetMap();
        if (map->IsDungeon())
            ((DungeonMap*)map)->GetPersistanceState()->UpdateEncounterState(ENCOUNTER_CREDIT_CAST_SPELL, m_spellInfo->Id);
    }
}

void Spell::SendCastResult(SpellCastResult result) const
{
    if (m_triggeredByAuraSpell) // should not report these
        return;

    Player const* recipient;
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
    {
        if (m_petCast)
        {
            Player const* master = m_caster->GetControllingPlayer();
            if (!master)
                return;

            recipient = master;
        }
        else
            return;
    }
    else
        recipient = static_cast<Player*>(m_caster);

    if (recipient->GetSession()->PlayerLoading()) // don't send cast results at loading time
        return;

    SendCastResult(recipient, m_spellInfo, result, m_petCast);
}

void Spell::SendCastResult(Player const* caster, SpellEntry const* spellInfo, SpellCastResult result, bool /*isPetCastResult =false*/)
{
    WorldPacket data(SMSG_CAST_RESULT, (4 + 1 + 1));
    data << uint32(spellInfo->Id);

    if (result != SPELL_CAST_OK)
    {
        data << uint8(2); // status = fail
        data << uint8(!IsPassiveSpell(spellInfo) ? result : SPELL_FAILED_DONT_REPORT); // do not report failed passive spells
        switch (result)
        {
            case SPELL_FAILED_REQUIRES_SPELL_FOCUS:
                data << uint32(spellInfo->RequiresSpellFocus);
                break;
            case SPELL_FAILED_REQUIRES_AREA:
                /* [-ZERO]    // hardcode areas limitation case     switch(spellInfo->Id)
                    {
                        default:                            // default case             data << uint32(spellInfo->AreaId);
                            break;
                    } */
                break;
            case SPELL_FAILED_EQUIPPED_ITEM_CLASS:
                data << uint32(spellInfo->EquippedItemClass);
                data << uint32(spellInfo->EquippedItemSubClassMask);
                data << uint32(spellInfo->EquippedItemInventoryTypeMask);
                break;
            default:
                break;
        }
    }
    else
        data << uint8(0);

    caster->GetSession()->SendPacket(data);
}

void Spell::SendSpellStart() const
{
    if (!IsNeedSendToClient())
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Sending SMSG_SPELL_START id=%u", m_spellInfo->Id);

    uint32 castFlags = CAST_FLAG_UNKNOWN2;
    if ((m_IsTriggeredSpell && !IsAutoRepeatRangedSpell(m_spellInfo)) || m_triggeredByAuraSpell)
        castFlags |= CAST_FLAG_HIDDEN_COMBATLOG;

    if (IsRangedSpell())
        castFlags |= CAST_FLAG_AMMO;

    WorldPacket data(SMSG_SPELL_START, (8 + 8 + 4 + 2 + 4));
    if (m_CastItem)
        data << m_CastItem->GetPackGUID();
    else
        data << m_caster->GetPackGUID();

    data << m_caster->GetPackGUID();
    data << uint32(m_spellInfo->Id);                        // spellId
    data << uint16(castFlags);                              // cast flags
    data << uint32(m_timer);                                // delay?

    data << m_targets;

    if (castFlags & CAST_FLAG_AMMO)                         // projectile info
        WriteAmmoToPacket(data);

    m_caster->SendMessageToSet(data, true);
}

void Spell::SendSpellGo()
{
    // not send invisible spell casting
    if (!IsNeedSendToClient())
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Sending SMSG_SPELL_GO id=%u", m_spellInfo->Id);

    uint32 castFlags = CAST_FLAG_UNKNOWN9;
    if ((m_IsTriggeredSpell && !IsAutoRepeatRangedSpell(m_spellInfo)) || m_triggeredByAuraSpell)
        castFlags |= CAST_FLAG_HIDDEN_COMBATLOG;

    if (IsRangedSpell())
        castFlags |= CAST_FLAG_AMMO;                        // arrows/bullets visual

    if (HasPersistentAuraEffect(m_spellInfo))
        castFlags |= CAST_FLAG_PERSISTENT_AA;

    if (m_CastItem)
        castFlags |= CAST_FLAG_UNKNOWN7;

    WorldPacket data(SMSG_SPELL_GO, 53);                    // guess size

    if (m_CastItem)
        data << m_CastItem->GetPackGUID();
    else
        data << m_caster->GetPackGUID();

    data << m_caster->GetPackGUID();
    data << uint32(m_spellInfo->Id);                        // spellId
    data << uint16(castFlags);                              // cast flags

    WriteSpellGoTargets(data);

    data << m_targets;

    if (castFlags & CAST_FLAG_AMMO)                         // projectile info
        WriteAmmoToPacket(data);

    m_caster->SendMessageToSet(data, true);
}

void Spell::WriteAmmoToPacket(WorldPacket& data) const
{
    uint32 ammoInventoryType = 0;
    uint32 ammoDisplayID = 0;

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        Item* pItem = ((Player*)m_caster)->GetWeaponForAttack(RANGED_ATTACK);
        if (pItem)
        {
            ammoInventoryType = pItem->GetProto()->InventoryType;
            if (ammoInventoryType == INVTYPE_THROWN)
                ammoDisplayID = pItem->GetProto()->DisplayInfoID;
            else
            {
                uint32 ammoID = ((Player*)m_caster)->GetUInt32Value(PLAYER_AMMO_ID);
                if (ammoID)
                {
                    ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(ammoID);
                    if (pProto)
                    {
                        ammoDisplayID = pProto->DisplayInfoID;
                        ammoInventoryType = pProto->InventoryType;
                    }
                }
            }
        }
    }
    else
    {
        for (uint8 i = 0; i < MAX_VIRTUAL_ITEM_SLOT; ++i)
        {
            // see Creature::SetVirtualItem for structure data
            if (uint32 item_class = m_caster->GetByteValue(UNIT_VIRTUAL_ITEM_INFO + (i * 2) + 0, VIRTUAL_ITEM_INFO_0_OFFSET_CLASS))
            {
                if (item_class == ITEM_CLASS_WEAPON)
                {
                    switch (m_caster->GetByteValue(UNIT_VIRTUAL_ITEM_INFO + (i * 2) + 0, VIRTUAL_ITEM_INFO_0_OFFSET_SUBCLASS))
                    {
                        case ITEM_SUBCLASS_WEAPON_THROWN:
                            ammoDisplayID = m_caster->GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + i);
                            ammoInventoryType = m_caster->GetByteValue(UNIT_VIRTUAL_ITEM_INFO + (i * 2) + 0, VIRTUAL_ITEM_INFO_0_OFFSET_INVENTORYTYPE);
                            break;
                        case ITEM_SUBCLASS_WEAPON_BOW:
                        case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                            ammoDisplayID = 5996;           // is this need fixing?
                            ammoInventoryType = INVTYPE_AMMO;
                            break;
                        case ITEM_SUBCLASS_WEAPON_GUN:
                            ammoDisplayID = 5998;           // is this need fixing?
                            ammoInventoryType = INVTYPE_AMMO;
                            break;
                    }

                    if (ammoDisplayID)
                        break;
                }
            }
        }
    }

    data << uint32(ammoDisplayID);
    data << uint32(ammoInventoryType);
}

void Spell::WriteSpellGoTargets(WorldPacket& data)
{
    size_t count_pos = data.wpos();
    data << uint8(0);                                      // placeholder

    // This function also fill data for channeled spells:
    // m_needAliveTargetMask req for stop channeling if one target die
    uint32 hit  = m_UniqueGOTargetInfo.size();              // Always hits on GO
    uint32 miss = 0;

    for (auto& ihit : m_UniqueTargetInfo)
    {
        if (ihit.effectHitMask == 0)                       // No effect apply - all immuned add state
        {
            // possibly SPELL_MISS_IMMUNE2 for this??
            ihit.missCondition = SPELL_MISS_IMMUNE2;
            ++miss;
        }
        else if (ihit.missCondition == SPELL_MISS_NONE)    // Add only hits
        {
            ++hit;
            data << ihit.targetGUID;
            m_needAliveTargetMask |= ihit.effectHitMask;
        }
        else
        {
            if (IsChanneledSpell(m_spellInfo) && (ihit.missCondition == SPELL_MISS_RESIST || ihit.missCondition == SPELL_MISS_REFLECT))
                m_duration = 0;                             // cancel aura to avoid visual effect continue
            ++miss;
        }
    }

    for (GOTargetList::const_iterator ighit = m_UniqueGOTargetInfo.begin(); ighit != m_UniqueGOTargetInfo.end(); ++ighit)
        data << ighit->targetGUID;                         // Always hits

    data.put<uint8>(count_pos, hit);

    data << (uint8)miss;
    for (TargetList::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
    {
        if (ihit->missCondition != SPELL_MISS_NONE)         // Add only miss
        {
            data << ihit->targetGUID;
            data << uint8(ihit->missCondition);
            if (ihit->missCondition == SPELL_MISS_REFLECT)
                data << uint8(ihit->reflectResult);
        }
    }
    // Reset m_needAliveTargetMask for non channeled spell
    if (!IsChanneledSpell(m_spellInfo))
        m_needAliveTargetMask = 0;
}

void Spell::SendInterrupted(SpellCastResult result) const
{
    WorldPacket data(SMSG_SPELL_FAILURE, (8 + 4 + 1));
    data << m_caster->GetPackGUID();
    data << m_spellInfo->Id;
    data << uint8(result);
    m_caster->SendMessageToSet(data, true);

    data.Initialize(SMSG_SPELL_FAILED_OTHER, (8 + 4));
    data << m_caster->GetObjectGuid();
    data << m_spellInfo->Id;
    m_caster->SendMessageToSet(data, true);
}

void Spell::SendChannelUpdate(uint32 time, uint32 lastTick) const
{
    if (time == 0)
    {
        if (lastTick)
        {
            if (SpellAuraHolder* holder = m_caster->GetSpellAuraHolder(m_spellInfo->Id, m_caster->GetObjectGuid()))
                holder->UpdateHolder(lastTick);
        }

        bool stackable = m_spellInfo->StackAmount > 0;

        // Channel aura is removed during its update when channel ends properly
        if (stackable)
            m_caster->RemoveAuraStack(m_spellInfo->Id);
        else
            m_caster->RemoveAurasByCasterSpell(m_spellInfo->Id, m_caster->GetObjectGuid(), m_timer == 0 ? AURA_REMOVE_BY_EXPIRE : AURA_REMOVE_BY_CANCEL);

        if (!m_caster->HasChannelObject(m_caster->GetObjectGuid()))
        {
            if (Unit* target = m_caster->GetChannelObject())
            {
                if (lastTick)
                {
                    if (SpellAuraHolder* holder = target->GetSpellAuraHolder(m_spellInfo->Id, m_caster->GetObjectGuid()))
                        holder->UpdateHolder(lastTick);
                }

                if (stackable)
                    target->RemoveAuraStack(m_spellInfo->Id);
                else
                    target->RemoveAurasByCasterSpell(m_spellInfo->Id, m_caster->GetObjectGuid(), m_timer == 0 ? AURA_REMOVE_BY_EXPIRE : AURA_REMOVE_BY_CANCEL);
            }
        }

        // Only finish channeling when latest channeled spell finishes
        if (m_caster->GetUInt32Value(UNIT_CHANNEL_SPELL) != m_spellInfo->Id)
            return;

        m_caster->SetChannelObject(nullptr);
        m_caster->SetUInt32Value(UNIT_CHANNEL_SPELL, 0);
        m_caster->clearUnitState(UNIT_STAT_CHANNELING);
        if (m_caster->AI())
            m_caster->AI()->OnChannelStateChange(this, false);
    }

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data(MSG_CHANNEL_UPDATE, 4);
        data << uint32(time);
        ((Player*)m_caster)->SendDirectMessage(data);
    }
}

void Spell::SendChannelStart(uint32 duration)
{
    WorldObject* target = nullptr;

    // select dynobject created by first effect if any
    if (m_spellInfo->Effect[EFFECT_INDEX_0] == SPELL_EFFECT_PERSISTENT_AREA_AURA)
        target = m_caster->GetDynObject(m_spellInfo->Id, EFFECT_INDEX_0);
    // select first not resisted target from target list for _0_ effect
    else if (!m_UniqueTargetInfo.empty())
    {
        for (TargetList::const_iterator itr = m_UniqueTargetInfo.begin(); itr != m_UniqueTargetInfo.end(); ++itr)
        {
            if (((itr->effectHitMask & (1 << EFFECT_INDEX_0)) && itr->reflectResult == SPELL_MISS_NONE &&
                    m_CastItem) || itr->targetGUID != m_caster->GetObjectGuid())
            {
                target = ObjectAccessor::GetUnit(*m_caster, itr->targetGUID);
                break;
            }
        }
    }
    else if (!m_UniqueGOTargetInfo.empty())
    {
        for (GOTargetList::const_iterator itr = m_UniqueGOTargetInfo.begin(); itr != m_UniqueGOTargetInfo.end(); ++itr)
        {
            if (itr->effectMask & (1 << EFFECT_INDEX_0))
            {
                target = m_caster->GetMap()->GetGameObject(itr->targetGUID);
                break;
            }
        }
    }

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data(MSG_CHANNEL_START, (4 + 4));
        data << uint32(m_spellInfo->Id);
        data << uint32(duration);
        ((Player*)m_caster)->SendDirectMessage(data);
    }

    m_timer = duration;

    if (target)
        m_caster->SetChannelObject(target);

    m_caster->SetUInt32Value(UNIT_CHANNEL_SPELL, m_spellInfo->Id);
    m_caster->addUnitState(UNIT_STAT_CHANNELING);

    if (m_caster->AI())
        m_caster->AI()->OnChannelStateChange(this, true, target);
}

void Spell::SendResurrectRequest(Player* target) const
{
    // Both players and NPCs can resurrect using spells - have a look at creature 28487 for example
    // However, the packet structure differs slightly

    const char* sentName = m_caster->GetTypeId() == TYPEID_PLAYER ? "" : m_caster->GetNameForLocaleIdx(target->GetSession()->GetSessionDbLocaleIndex());

    WorldPacket data(SMSG_RESURRECT_REQUEST, (8 + 4 + strlen(sentName) + 1 + 1 + 1));
    data << m_caster->GetObjectGuid();
    data << uint32(strlen(sentName) + 1);

    data << sentName;
    data << uint8(m_caster->isSpiritHealer());
    // override delay sent with SMSG_CORPSE_RECLAIM_DELAY, set instant resurrection for spells with this attribute
    data << uint8(!m_spellInfo->HasAttribute(SPELL_ATTR_EX3_IGNORE_RESURRECTION_TIMER));
    target->GetSession()->SendPacket(data);
}

void Spell::TakeCastItem()
{
    if (!m_CastItem || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    // not remove cast item at triggered spell (equipping, weapon damage, etc)
    if (m_IsTriggeredSpell && !(m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM))
        return;

    ItemPrototype const* proto = m_CastItem->GetProto();

    if (!proto)
    {
        // This code is to avoid a crash
        // I'm not sure, if this is really an error, but I guess every item needs a prototype
        sLog.outError("Cast item (%s) has no item prototype", m_CastItem->GetGuidStr().c_str());
        return;
    }

    bool expendable = false;
    bool withoutCharges = false;

    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (proto->Spells[i].SpellId)
        {
            // item has limited charges
            if (proto->Spells[i].SpellCharges)
            {
                if (proto->Spells[i].SpellCharges < 0)
                    expendable = true;

                int32 charges = m_CastItem->GetSpellCharges(i);

                // item has charges left
                if (charges)
                {
                    (charges > 0) ? --charges : ++charges;  // abs(charges) less at 1 after use
                    if (proto->Stackable == 1)
                        m_CastItem->SetSpellCharges(i, charges);
                    m_CastItem->SetState(ITEM_CHANGED, (Player*)m_caster);
                }

                // all charges used
                withoutCharges = (charges == 0);
            }
        }
    }

    if (expendable && withoutCharges)
    {
        uint32 count = 1;
        ((Player*)m_caster)->DestroyItemCount(m_CastItem, count, true);

        // prevent crash at access to deleted m_targets.getItemTarget
        ClearCastItem();
    }
}

void Spell::TakePower()
{
    if (m_CastItem || m_triggeredByAuraSpell)
        return;

    // health as power used
    if (m_spellInfo->powerType == POWER_HEALTH)
    {
        m_caster->ModifyHealth(-(int32)m_powerCost);
        m_caster->SendSpellNonMeleeDamageLog(m_caster, m_spellInfo->Id, m_powerCost, GetSpellSchoolMask(m_spellInfo), 0, 0, false, 0, false);
        return;
    }

    if (m_spellInfo->powerType >= MAX_POWERS)
    {
        sLog.outError("Spell::TakePower: Unknown power type '%d'", m_spellInfo->powerType);
        return;
    }

    Powers powerType = Powers(m_spellInfo->powerType);

    m_caster->ModifyPower(powerType, -(int32)m_powerCost);

    // Set the five second timer
    if (powerType == POWER_MANA && m_powerCost > 0)
        m_caster->SetLastManaUse();
}

void Spell::TakeAmmo() const
{
    if (m_attackType == RANGED_ATTACK && m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        Item* pItem = ((Player*)m_caster)->GetWeaponForAttack(RANGED_ATTACK, true, false);

        // wands don't have ammo
        if (!pItem || pItem->GetProto()->SubClass == ITEM_SUBCLASS_WEAPON_WAND)
            return;

        if (pItem->GetProto()->InventoryType == INVTYPE_THROWN)
        {
            if (pItem->GetMaxStackCount() == 1)
            {
                // decrease durability for non-stackable throw weapon
                ((Player*)m_caster)->DurabilityPointLossForEquipSlot(EQUIPMENT_SLOT_RANGED);
            }
            else
            {
                // decrease items amount for stackable throw weapon
                uint32 count = 1;
                ((Player*)m_caster)->DestroyItemCount(pItem, count, true);
            }
        }
        else if (uint32 ammo = ((Player*)m_caster)->GetUInt32Value(PLAYER_AMMO_ID))
            ((Player*)m_caster)->DestroyItemCount(ammo, 1, true);
    }
}


void Spell::TakeReagents()
{
    if (!m_caster->IsPlayer())
        return;

    if (IgnoreItemRequirements())                           // reagents used in triggered spell removed by original spell or don't must be removed.
        return;

    Player* p_caster = (Player*)m_caster;
    if (p_caster->CanNoReagentCast(m_spellInfo))
        return;

    for (uint32 x = 0; x < MAX_SPELL_REAGENTS; ++x)
    {
        if (m_spellInfo->Reagent[x] <= 0)
            continue;

        uint32 itemid = m_spellInfo->Reagent[x];
        uint32 itemcount = m_spellInfo->ReagentCount[x];

        // if CastItem is also spell reagent
        if (m_CastItem)
        {
            ItemPrototype const* proto = m_CastItem->GetProto();
            if (proto && proto->ItemId == itemid)
            {
                for (int s = 0; s < MAX_ITEM_PROTO_SPELLS; ++s)
                {
                    // CastItem will be used up and does not count as reagent
                    int32 charges = m_CastItem->GetSpellCharges(s);
                    if (proto->Spells[s].SpellCharges < 0 && abs(charges) < 2)
                    {
                        ++itemcount;
                        break;
                    }
                }

                m_CastItem = nullptr;
                m_CastItemGuid.Clear();
            }
        }

        // if getItemTarget is also spell reagent
        if (m_targets.getItemTargetEntry() == itemid)
            m_targets.setItemTarget(nullptr);

        p_caster->DestroyItemCount(itemid, itemcount, true);
    }
}

void Spell::HandleThreatSpells()
{
    if (m_UniqueTargetInfo.empty())
        return;

    SpellThreatEntry const* threatEntry = sSpellMgr.GetSpellThreatEntry(m_spellInfo->Id);

    if (!threatEntry || (!threatEntry->threat && threatEntry->ap_bonus == 0.0f))
        return;

    float threat = threatEntry->threat;
    if (threatEntry->ap_bonus != 0.0f)
        threat += threatEntry->ap_bonus * m_caster->GetTotalAttackPowerValue(GetWeaponAttackType(m_spellInfo));

    bool positive = true;
    uint8 effectMask = 0;
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (m_spellInfo->Effect[i])
            effectMask |= (1 << i);

    if (m_negativeEffectMask & effectMask)
    {
        // can only handle spells with clearly defined positive/negative effect, check at spell_threat loading probably not perfect
        // so abort when only some effects are negative.
        if ((m_negativeEffectMask & effectMask) != effectMask)
        {
            DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Spell %u, rank %u, is not clearly positive or negative, ignoring bonus threat", m_spellInfo->Id, sSpellMgr.GetSpellRank(m_spellInfo->Id));
            return;
        }
        positive = false;
    }

    // before 2.0.1 threat from positive effects not dependent from targets amount
    if (!positive)
        threat /= m_UniqueTargetInfo.size();

    for (TargetList::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
    {
        if (ihit->missCondition != SPELL_MISS_NONE)
            continue;

        Unit* target = m_caster->GetObjectGuid() == ihit->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit->targetGUID);
        if (!target)
            continue;

        // positive spells distribute threat among all units that are in combat with target, like healing
        if (positive)
        {
            target->getHostileRefManager().threatAssist(m_caster /*real_caster ??*/, threat, m_spellInfo);
        }
        // for negative spells threat gets distributed among affected targets
        else
        {
            if (!target->CanHaveThreatList())
                continue;

            target->AddThreat(m_caster, threat, false, GetSpellSchoolMask(m_spellInfo), m_spellInfo);
        }
    }

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Spell %u added an additional %f threat for %s " SIZEFMTD " target(s)", m_spellInfo->Id, threat, positive ? "assisting" : "harming", m_UniqueTargetInfo.size());
}

void Spell::HandleEffects(Unit* pUnitTarget, Item* pItemTarget, GameObject* pGOTarget, SpellEffectIndex i, float DamageMultiplier)
{
    unitTarget = pUnitTarget;
    itemTarget = pItemTarget;
    gameObjTarget = pGOTarget;

    uint8 eff = m_spellInfo->Effect[i];

    if (IsEffectWithImplementedMultiplier(eff))
    {
        m_healingPerEffect[i] = 0;
        m_damagePerEffect[i] = 0;
        damage = CalculateDamage(i, unitTarget);
    }
    else
        damage = int32(CalculateDamage(i, unitTarget) * DamageMultiplier);

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Spell %u Effect%d : %u Targets: %s, %s, %s",
                     m_spellInfo->Id, i, eff,
                     unitTarget ? unitTarget->GetGuidStr().c_str() : "-",
                     itemTarget ? itemTarget->GetGuidStr().c_str() : "-",
                     gameObjTarget ? gameObjTarget->GetGuidStr().c_str() : "-");

    if (eff < MAX_SPELL_EFFECTS)
        (*this.*SpellEffects[eff])(i);
    else
        sLog.outError("WORLD: Spell FX %d > MAX_SPELL_EFFECTS ", eff);

    if (IsEffectWithImplementedMultiplier(eff))
    {
        if (m_healingPerEffect[i])
            m_healing = int32(m_healingPerEffect[i] * DamageMultiplier);
        else if (m_damagePerEffect[i])
            m_damage = int32(m_damagePerEffect[i] * DamageMultiplier);
    }
}

void Spell::AddTriggeredSpell(uint32 spellId)
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

    if (!spellInfo)
    {
        sLog.outError("Spell::AddTriggeredSpell: unknown spell id %u used as triggred spell for spell %u)", spellId, m_spellInfo->Id);
        return;
    }

    m_TriggerSpells.push_back(spellInfo);
}

void Spell::AddPrecastSpell(uint32 spellId)
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

    if (!spellInfo)
    {
        sLog.outError("Spell::AddPrecastSpell: unknown spell id %u used as pre-cast spell for spell %u)", spellId, m_spellInfo->Id);
        return;
    }

    m_preCastSpells.push_back(spellInfo);
}

void Spell::CastTriggerSpells()
{
    for (SpellInfoList::const_iterator si = m_TriggerSpells.begin(); si != m_TriggerSpells.end(); ++si)
    {
        Spell* spell = new Spell(m_caster, (*si), TRIGGERED_OLD_TRIGGERED, m_originalCasterGUID);
        spell->SpellStart(&m_targets);                      // use original spell original targets
    }
}

void Spell::CastPreCastSpells(Unit* target)
{
    for (SpellInfoList::const_iterator si = m_preCastSpells.begin(); si != m_preCastSpells.end(); ++si)
        m_caster->CastSpell(target, (*si), TRIGGERED_OLD_TRIGGERED, m_CastItem);
}

Unit* Spell::GetPrefilledUnitTargetOrUnitTarget(SpellEffectIndex effIndex) const
{
    for (const auto& itr : m_UniqueTargetInfo)
        if (itr.effectHitMask & (1 << effIndex))
            return m_caster->GetMap()->GetUnit(itr.targetGUID);

    return m_targets.getUnitTarget();
}

SpellCastResult Spell::CheckCast(bool strict)
{
    // check cooldowns to prevent cheating (ignore passive spells, that client side visual only)
    if (m_caster->GetTypeId() == TYPEID_PLAYER && !m_spellInfo->HasAttribute(SPELL_ATTR_PASSIVE) &&
            !m_IsTriggeredSpell && !m_caster->IsSpellReady(*m_spellInfo, m_CastItem ? m_CastItem->GetProto() : nullptr))
    {
        return m_triggeredByAuraSpell ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_NOT_READY;
    }

    if (!m_caster->isAlive() && m_caster->GetTypeId() == TYPEID_PLAYER && !m_spellInfo->HasAttribute(SPELL_ATTR_CASTABLE_WHILE_DEAD) && !m_spellInfo->HasAttribute(SPELL_ATTR_PASSIVE))
        return SPELL_FAILED_CASTER_DEAD;

    if (!m_caster->IsStandState() && m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && !m_spellInfo->HasAttribute(SPELL_ATTR_CASTABLE_WHILE_SITTING))
        return SPELL_FAILED_NOT_STANDING;

    // check global cooldown
    if (strict && !m_IsTriggeredSpell && m_caster->HasGCD(m_spellInfo))
        return m_spellInfo->HasAttribute(SPELL_ATTR_DISABLED_WHILE_ACTIVE) ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_NOT_READY;

    // only allow triggered spells if at an ended battleground
    if (!m_IsTriggeredSpell && m_caster->GetTypeId() == TYPEID_PLAYER)
        if (BattleGround* bg = ((Player*)m_caster)->GetBattleGround())
            if (bg->GetStatus() == STATUS_WAIT_LEAVE)
                return SPELL_FAILED_DONT_REPORT;

    if (!m_IsTriggeredSpell && IsNonCombatSpell(m_spellInfo) && m_caster->isInCombat())
        return SPELL_FAILED_AFFECTING_COMBAT;

    if (m_caster->GetTypeId() == TYPEID_PLAYER && !((Player*)m_caster)->isGameMaster() &&
            sWorld.getConfig(CONFIG_BOOL_VMAP_INDOOR_CHECK) &&
            VMAP::VMapFactory::createOrGetVMapManager()->isLineOfSightCalcEnabled())
    {
        if (m_spellInfo->HasAttribute(SPELL_ATTR_OUTDOORS_ONLY) &&
                !m_caster->GetTerrain()->IsOutdoors(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ()))
            return SPELL_FAILED_ONLY_OUTDOORS;

        if (m_spellInfo->HasAttribute(SPELL_ATTR_INDOORS_ONLY) &&
                m_caster->GetTerrain()->IsOutdoors(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ()))
            return SPELL_FAILED_ONLY_INDOORS;
    }

    // only check at first call, Stealth auras are already removed at second call
    // for now, ignore triggered spells
    if (strict && !m_IsTriggeredSpell)
    {
        // Cannot be used in this stance/form
        SpellCastResult shapeError = GetErrorAtShapeshiftedCast(m_spellInfo, m_caster->GetShapeshiftForm());
        if (shapeError != SPELL_CAST_OK)
            return shapeError;

        if (m_spellInfo->HasAttribute(SPELL_ATTR_ONLY_STEALTHED) && !(m_caster->HasStealthAura()))
            return SPELL_FAILED_ONLY_STEALTHED;
    }

    // caster state requirements
    if (m_spellInfo->CasterAuraState && !m_caster->HasAuraState(AuraState(m_spellInfo->CasterAuraState)))
        return SPELL_FAILED_CASTER_AURASTATE;


    if (m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        // cancel autorepeat spells if cast start when moving
        // (not wand currently autorepeat cast delayed to moving stop anyway in spell update code)
        if (((Player*)m_caster)->IsMoving())
        {
            // skip stuck spell to allow use it in falling case and apply spell limitations at movement
            if ((!((Player*)m_caster)->m_movementInfo.HasMovementFlag(MOVEFLAG_FALLINGFAR) || m_spellInfo->Effect[EFFECT_INDEX_0] != SPELL_EFFECT_STUCK) &&
                    (IsAutoRepeat() || (m_spellInfo->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED) != 0))
                return SPELL_FAILED_MOVING;
        }

        if (!m_IsTriggeredSpell && NeedsComboPoints(m_spellInfo) &&
                (!m_targets.getUnitTarget() || m_targets.getUnitTarget()->GetObjectGuid() != ((Player*)m_caster)->GetComboTargetGuid()))
            // warrior not have real combo-points at client side but use this way for mark allow Overpower use
            return m_caster->getClass() == CLASS_WARRIOR ? SPELL_FAILED_CASTER_AURASTATE : SPELL_FAILED_NO_COMBO_POINTS;

        // Loatheb Corrupted Mind and Nefarian class calls spell failed
        switch (m_spellInfo->SpellFamilyName)
        {
            case SPELLFAMILY_DRUID:
            {
                if (IsSpellHaveAura(m_spellInfo, SPELL_AURA_MOD_SHAPESHIFT))
                {
                    Unit::AuraList const& auraClassScripts = m_caster->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                    for (Unit::AuraList::const_iterator itr = auraClassScripts.begin(); itr != auraClassScripts.end();)
                    {
                        if ((*itr)->GetModifier()->m_miscvalue == 3655)
                            return SPELL_FAILED_TARGET_AURASTATE;
                        else
                            ++itr;
                    }
                }
                // No break
            }
            case SPELLFAMILY_PRIEST:
            case SPELLFAMILY_SHAMAN:
            case SPELLFAMILY_PALADIN:
            {
                if (IsSpellHaveEffect(m_spellInfo, SPELL_EFFECT_HEAL) || IsSpellHaveAura(m_spellInfo, SPELL_AURA_PERIODIC_HEAL) ||
                        IsSpellHaveEffect(m_spellInfo, SPELL_EFFECT_DISPEL))
                {
                    Unit::AuraList const& auraClassScripts = m_caster->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                    for (Unit::AuraList::const_iterator itr = auraClassScripts.begin(); itr != auraClassScripts.end();)
                    {
                        if ((*itr)->GetModifier()->m_miscvalue == 4327)
                            return SPELL_FAILED_FIZZLE;
                        else
                            ++itr;
                    }
                }
                break;
            }
            case SPELLFAMILY_WARRIOR:
            {
                if (IsSpellHaveAura(m_spellInfo, SPELL_AURA_MOD_SHAPESHIFT))
                {
                    Unit::AuraList const& auraClassScripts = m_caster->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                    for (Unit::AuraList::const_iterator itr = auraClassScripts.begin(); itr != auraClassScripts.end();)
                    {
                        if ((*itr)->GetModifier()->m_miscvalue == 3654)
                            return SPELL_FAILED_TARGET_AURASTATE;
                        else
                            ++itr;
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    // following code block is supposed to be applied to single target spell
    // TODO: above assumption need to be verified
    if (Unit* target = m_targets.getUnitTarget())
    {
        // TARGET_UNIT_SCRIPT_NEAR_CASTER fills unit target per client requirement but should not be checked against common things
        // TODO: Find a nicer and more efficient way to check for this
        if (!IsSpellWithScriptUnitTarget(m_spellInfo))
        {
            // Swiftmend
            if (m_spellInfo->Id == 18562)                       // future versions have special aura state for this
            {
                if (!target->GetAura(SPELL_AURA_PERIODIC_HEAL, SPELLFAMILY_DRUID, uint64(0x50)))
                    return SPELL_FAILED_TARGET_AURASTATE;
            }

            if (!m_IsTriggeredSpell && IsDeathOnlySpell(m_spellInfo) && target->isAlive())
                return SPELL_FAILED_TARGET_NOT_DEAD;

            // totem immunity for channeled spells(needs to be before spell cast)
            // spell attribs for player channeled spells
            if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_CHANNEL_TRACK_TARGET)
                && target->GetTypeId() == TYPEID_UNIT && ((Creature*)target)->IsTotem())
                return SPELL_FAILED_IMMUNE;

            bool non_caster_target = target != m_caster && !IsSpellWithCasterSourceTargetsOnly(m_spellInfo);

            if (non_caster_target)
            {
                // Not allow casting on flying player
                if (target->IsTaxiFlying())
                {
                    switch (m_spellInfo->Id)
                    {
                        // Except some spells from Taxi Flying cast
                        case 7720:                              // Ritual of Summoning Effect
                            break;
                        default:
                            return SPELL_FAILED_BAD_TARGETS;
                    }
                }

                if (!IsIgnoreLosSpell(m_spellInfo) && !m_IsTriggeredSpell && VMAP::VMapFactory::checkSpellForLoS(m_spellInfo->Id) && !m_caster->IsWithinLOSInMap(target, true))
                    return SPELL_FAILED_LINE_OF_SIGHT;

                if (m_caster->GetTypeId() == TYPEID_PLAYER)
                {
                    // auto selection spell rank implemented in WorldSession::HandleCastSpellOpcode
                    // this case can be triggered if rank not found (too low-level target for first rank)
                    if (!m_CastItem && !m_IsTriggeredSpell)
                        // spell expected to be auto-downranking in cast handle, so must be same
                        if (m_spellInfo != sSpellMgr.SelectAuraRankForLevel(m_spellInfo, target->getLevel()))
                            return SPELL_FAILED_LOWLEVEL;

                    // Do not allow these spells to target creatures not tapped by us (Banish, Polymorph, many quest spells)
                    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_CANT_TARGET_TAPPED))
                        if (Creature const* targetCreature = dynamic_cast<Creature*>(target))
                            if ((!targetCreature->GetLootRecipientGuid().IsEmpty()) && !targetCreature->IsTappedBy((Player*)m_caster))
                                return SPELL_FAILED_CANT_CAST_ON_TAPPED;
                }

                if (strict && m_spellInfo->HasAttribute(SPELL_ATTR_EX3_TARGET_ONLY_PLAYER) && target->GetTypeId() != TYPEID_PLAYER && !IsAreaOfEffectSpell(m_spellInfo))
                    return SPELL_FAILED_BAD_TARGETS;
            }
            else if (m_caster == target)
            {
                if (m_caster->GetTypeId() == TYPEID_PLAYER && m_caster->IsInWorld())
                {
                    // Additional check for some spells
                    // If 0 spell effect empty - client not send target data (need use selection)
                    // TODO: check it on next client version
                    if (m_targets.m_targetMask == TARGET_FLAG_SELF &&
                        m_spellInfo->EffectImplicitTargetA[EFFECT_INDEX_1] == TARGET_UNIT_ENEMY)
                    {
                        target = m_caster->GetMap()->GetUnit(((Player*)m_caster)->GetSelectionGuid());
                        if (!target)
                            return SPELL_FAILED_BAD_TARGETS;

                        m_targets.setUnitTarget(target);
                    }
                }

                // Some special spells with non-caster only mode

                // Fire Shield
                if (m_spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK &&
                    m_spellInfo->SpellIconID == 16)
                    return SPELL_FAILED_BAD_TARGETS;
            }

            // check pet presents
            for (unsigned int j : m_spellInfo->EffectImplicitTargetA)
            {
                if (j == TARGET_UNIT_CASTER_PET)
                {
                    Pet* pet = m_caster->GetPet();
                    if (!pet)
                    {
                        if (m_triggeredByAuraSpell)             // not report pet not existence for triggered spells
                            return SPELL_FAILED_DONT_REPORT;
                        return SPELL_FAILED_NO_PET;
                    }
                    else
                    {
                        if (!pet->isAlive())
                            return SPELL_FAILED_TARGETS_DEAD;
                        if (!IsIgnoreLosSpellEffect(m_spellInfo, SpellEffectIndex(j)) && !m_caster->IsWithinLOSInMap(pet))
                            return SPELL_FAILED_LINE_OF_SIGHT;
                    }
                    break;
                }
            }

            // check creature type
            // ignore self casts (including area casts when caster selected as target)
            if (non_caster_target)
            {
                if (!CheckTargetCreatureType(target))
                {
                    if (target->GetTypeId() == TYPEID_PLAYER)
                        return SPELL_FAILED_TARGET_IS_PLAYER;
                    return SPELL_FAILED_BAD_TARGETS;
                }
            }

            for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
            {
                uint32 targetType = m_spellInfo->EffectImplicitTargetA[i];
                if (targetType == TARGET_UNIT_CASTER || targetType == TARGET_UNIT_CASTER_PET) // fits the description but should never be checked
                    continue;
                auto& data = SpellTargetInfoTable[targetType];
                if (data.type == TARGET_TYPE_UNIT && (data.enumerator == TARGET_ENUMERATOR_SINGLE || data.enumerator == TARGET_ENUMERATOR_CHAIN))
                {
                    switch (data.filter)
                    {
                        case TARGET_HARMFUL: if (!m_caster->CanAttackSpell(target, m_spellInfo)) return SPELL_FAILED_BAD_TARGETS; break;
                        case TARGET_HELPFUL: if (!m_caster->CanAssistSpell(target, m_spellInfo)) return SPELL_FAILED_BAD_TARGETS; break;
                        case TARGET_PARTY:
                        case TARGET_GROUP: if (!m_caster->CanAssistSpell(target, m_spellInfo) || !m_caster->IsInGroup(target, targetType == TARGET_UNIT_PARTY)) return SPELL_FAILED_BAD_TARGETS; break;
                        default: break;
                    }
                }
            }

            if (IsPositiveSpell(m_spellInfo->Id, m_caster, target))
                if (target->IsImmuneToSpell(m_spellInfo, target == m_caster, 7) && !target->hasUnitState(UNIT_STAT_ISOLATED))
                    return SPELL_FAILED_TARGET_AURASTATE;

            // Must be behind the target.
            if (m_spellInfo->AttributesEx2 == SPELL_ATTR_EX2_FACING_TARGETS_BACK && m_spellInfo->HasAttribute(SPELL_ATTR_EX_FACING_TARGET) && target->HasInArc(m_caster))
            {
                // Exclusion for Pounce: Facing Limitation was removed in 2.0.1, but it still uses the same, old Ex-Flags
                if (!m_spellInfo->IsFitToFamily(SPELLFAMILY_DRUID, uint64(0x0000000000020000)))
                    return SPELL_FAILED_NOT_BEHIND;
            }

            // Caster must be facing the targets front
            if (((m_spellInfo->Attributes == (SPELL_ATTR_ABILITY | SPELL_ATTR_NOT_SHAPESHIFT | SPELL_ATTR_DONT_AFFECT_SHEATH_STATE | SPELL_ATTR_STOP_ATTACK_TARGET)) && !m_caster->IsFacingTargetsFront(target))
                // Caster must be facing the target!
                || (m_spellInfo->HasAttribute(SPELL_ATTR_EX_FACING_TARGET) && !m_caster->HasInArc(target)))
                return SPELL_FAILED_NOT_INFRONT;

            // check if target is in combat
            if (non_caster_target && m_spellInfo->HasAttribute(SPELL_ATTR_EX_NOT_IN_COMBAT_TARGET) && target->isInCombat())
                return SPELL_FAILED_TARGET_AFFECTING_COMBAT;

            // check if target is affected by Spirit of Redemption (Aura: 27827)
            if (target->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
                return SPELL_FAILED_BAD_TARGETS;

            // Check if more powerful spell applied on target (if spell only contains non-aoe auras)
            if (IsAuraApplyEffects(m_spellInfo, EFFECT_MASK_ALL) && !IsAreaOfEffectSpell(m_spellInfo) && !HasAreaAuraEffect(m_spellInfo))
            {
                const ObjectGuid casterGuid = m_caster->GetObjectGuid();
                Unit::SpellAuraHolderMap const& spair = target->GetSpellAuraHolderMap();
                for (Unit::SpellAuraHolderMap::const_iterator iter = spair.begin(); iter != spair.end(); ++iter)
                {
                    const SpellAuraHolder* existing = iter->second;
                    // We can overwrite own auras at all times
                    if (casterGuid == existing->GetCasterGuid())
                        continue;
                    const SpellEntry* entry = existing->GetSpellProto();
                    // Cannot overwrite someone else's auras
                    if (!sSpellMgr.IsSpellStackableWithSpellForDifferentCasters(m_spellInfo, entry))
                    {
                        bool bounce = false;
                        if (IsSimilarExistingAuraStronger(m_caster, m_spellInfo->Id, existing))
                            bounce = true;
                        if (!bounce && sSpellMgr.IsSpellAnotherRankOfSpell(m_spellInfo->Id, entry->Id) && sSpellMgr.IsSpellHigherRankOfSpell(entry->Id, m_spellInfo->Id))
                            bounce = true;
                        if (bounce)
                            return SPELL_FAILED_AURA_BOUNCED;
                    }
                }
            }

            // Classic only (seems to be handled client side tbc and later (not verified)) - Only check effect 0
            // Spells such as Abolish Poison have dispel on effect 1 but should be castable even if target not poisoned
            // shield slam has dispel on effect 0 which should be castable even if nothing to dispel
            if (m_spellInfo->Effect[0] == SPELL_EFFECT_DISPEL
                && m_spellInfo->Effect[1] == SPELL_EFFECT_NONE
                && m_spellInfo->Effect[2] == SPELL_EFFECT_NONE)
            {
                bool dispelTarget = false;
                uint32 mechanic = m_spellInfo->EffectMiscValue[0];
                SpellEntry const* spell = nullptr;

                Unit::SpellAuraHolderMap& Auras = target->GetSpellAuraHolderMap();
                for (Unit::SpellAuraHolderMap::iterator iter = Auras.begin(); iter != Auras.end(); ++iter)
                {
                    spell = iter->second->GetSpellProto();
                    if (spell->Dispel == mechanic)
                    {
                        dispelTarget = true;
                        break;
                    }
                }

                if (!dispelTarget)
                {
                    // Don't report triggered spells
                    if (m_triggeredByAuraSpell || m_IsTriggeredSpell)
                        return SPELL_FAILED_DONT_REPORT;
                    else
                        return SPELL_FAILED_NOTHING_TO_DISPEL;
                }
            }

            if (m_spellInfo->MaxTargetLevel && target->getLevel() > m_spellInfo->MaxTargetLevel)
                return SPELL_FAILED_HIGHLEVEL;
        }
    }
    else // some spells always require targets - generally checked in client - need to check serverside as well
        for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
            if (IsUnitTargetTarget(m_spellInfo->EffectImplicitTargetA[i]))
                return SPELL_FAILED_BAD_TARGETS;

    // zone check
    uint32 zone, area;
    m_caster->GetZoneAndAreaId(zone, area);

    SpellCastResult locRes = sSpellMgr.GetSpellAllowedInLocationError(m_spellInfo, m_caster->GetMapId(), zone, area,
                             m_caster->GetBeneficiaryPlayer());
    if (locRes != SPELL_CAST_OK)
    {
        if (!m_IsTriggeredSpell)
            return locRes;
        return SPELL_FAILED_DONT_REPORT;
    }

    // not let players cast spells at mount (and let do it to creatures)
    if (m_caster->IsMounted() && m_caster->GetTypeId() == TYPEID_PLAYER && !m_IsTriggeredSpell &&
            !IsPassiveSpell(m_spellInfo) && !m_spellInfo->HasAttribute(SPELL_ATTR_CASTABLE_WHILE_MOUNTED))
    {
        if (m_caster->IsTaxiFlying())
            return SPELL_FAILED_NOT_ON_TAXI;
        return SPELL_FAILED_NOT_MOUNTED;
    }

    // always (except passive spells) check items (focus object can be required for any type casts)
    if (!IsPassiveSpell(m_spellInfo))
    {
        SpellCastResult castResult = CheckItems();
        if (castResult != SPELL_CAST_OK)
            return castResult;
    }

    if (!m_IsTriggeredSpell)
    {
        if (!m_triggeredByAuraSpell)
        {
            SpellCastResult castResult = CheckRange(strict);
            if (castResult != SPELL_CAST_OK)
                return castResult;
        }
    }

    if (!m_IsTriggeredSpell)
    {
        SpellCastResult castResult = CheckPower();
        if (castResult != SPELL_CAST_OK)
            return castResult;
    }

    if (!m_IsTriggeredSpell)                                // triggered spell not affected by stun/etc
    {
        SpellCastResult castResult = CheckCasterAuras();
        if (castResult != SPELL_CAST_OK)
            return castResult;
    }

    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        // for effects of spells that have only one target
        switch (m_spellInfo->Effect[i])
        {
            case SPELL_EFFECT_DUMMY:
            {
                // By Spell ID
                if (m_spellInfo->Id == 19938)               // Awaken Lazy Peon
                {
                    Unit* target = m_targets.getUnitTarget();
                    // 17743 = Lazy Peon Sleep | 10556 = Lazy Peon
                    if (!target || !target->HasAura(17743) || target->GetEntry() != 10556)
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else if (m_spellInfo->Id == 20577)
                {
                    WorldObject* result = FindCorpseUsing<MaNGOS::CannibalizeObjectCheck>();
                    if (result)
                    {
                        switch (result->GetTypeId())
                        {
                            case TYPEID_UNIT:
                            case TYPEID_PLAYER:
                                if (!CheckTarget(static_cast<Unit*>(result), SpellEffectIndex(i), false, EXCEPTION_NONE))
                                    return SPELL_FAILED_NO_EDIBLE_CORPSES;
                                break;
                            case TYPEID_CORPSE:
                                if (Player* owner = ObjectAccessor::FindPlayer(static_cast<Corpse*>(result)->GetOwnerGuid()))
                                    if (!CheckTarget(owner, SpellEffectIndex(i), false, EXCEPTION_NONE))
                                        return SPELL_FAILED_NO_EDIBLE_CORPSES;
                                break;
                        }
                    }
                    else
                        return SPELL_FAILED_NO_EDIBLE_CORPSES;
                }
                // By SpellIconID
                else if (m_spellInfo->SpellIconID == 1648)       // Execute
                {
                    Unit* target = m_targets.getUnitTarget();
                    if (!target || target->GetHealth() > target->GetMaxHealth() * 0.2)
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else if (m_spellInfo->SpellIconID == 156)   // Holy Shock
                {
                    Unit* target = m_targets.getUnitTarget();
                    if (!target)
                        return SPELL_FAILED_BAD_TARGETS;

                    // Prevents usage when cant neither attack or assist and not in front for shock attack
                    if (m_caster->CanAttack(target))
                    {
                        if (!m_caster->HasInArc(target))
                            return SPELL_FAILED_UNIT_NOT_INFRONT;
                    }
                    else if (!m_caster->CanAssistSpell(target, m_spellInfo))
                        return SPELL_FAILED_BAD_TARGETS;
                }
                break;
            }
            case SPELL_EFFECT_DISTRACT:                     // All nearby enemies must not be in combat
            {
                if (m_targets.m_targetMask & (TARGET_FLAG_DEST_LOCATION | TARGET_FLAG_SOURCE_LOCATION))
                {
                    UnitList targetsCombat;
                    float radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[i]));

                    FillAreaTargets(targetsCombat, radius, 0.f, PUSH_DEST_CENTER, SPELL_TARGETS_AOE_ATTACKABLE);

                    if (targetsCombat.empty())
                        break;

                    for (auto& itr : targetsCombat)
                        if (itr->isInCombat())
                            return SPELL_FAILED_TARGET_IN_COMBAT;
                }
                break;
            }
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            {
                // Hammer of Wrath
                if (m_spellInfo->SpellVisual == 7250)
                {
                    if (!m_targets.getUnitTarget())
                        return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                    if (m_targets.getUnitTarget()->GetHealth() > m_targets.getUnitTarget()->GetMaxHealth() * 0.2)
                        return SPELL_FAILED_BAD_TARGETS;
                }
                // Conflagrate
                else if (m_spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK && m_spellInfo->SpellFamilyFlags & uint64(0x0000000000000200))
                {
                    if (!m_targets.getUnitTarget())
                        return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                    // for caster applied auras only
                    bool found = false;
                    Unit::AuraList const& mPeriodic = m_targets.getUnitTarget()->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for (Unit::AuraList::const_iterator i = mPeriodic.begin(); i != mPeriodic.end(); ++i)
                    {
                        if ((*i)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_WARLOCK &&
                                (*i)->GetCasterGuid() == m_caster->GetObjectGuid() &&
                                // Immolate
                                ((*i)->GetSpellProto()->SpellFamilyFlags & uint64(0x0000000000000004)))
                        {
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                        return SPELL_FAILED_TARGET_AURASTATE;
                }
                break;
            }
            case SPELL_EFFECT_TAMECREATURE:
            {
                // Spell can be triggered, we need to check original caster prior to caster
                Unit* caster = GetAffectiveCaster();
                Unit* tameTarget;

                bool gmmode = m_triggeredBySpellInfo == nullptr;
                if (gmmode)
                    tameTarget = caster->GetTarget();
                else
                    tameTarget = m_caster->GetChannelObject();

                if (!caster || caster->GetTypeId() != TYPEID_PLAYER ||
                        !tameTarget || tameTarget->GetTypeId() == TYPEID_PLAYER)
                    return SPELL_FAILED_BAD_TARGETS;

                Player* plrCaster = (Player*)caster;

                if (gmmode && !ChatHandler(plrCaster).FindCommand("npc tame"))
                {
                    plrCaster->SendPetTameFailure(PETTAME_UNKNOWNERROR);
                    return SPELL_FAILED_DONT_REPORT;
                }

                if (plrCaster->getClass() != CLASS_HUNTER && !gmmode)
                {
                    plrCaster->SendPetTameFailure(PETTAME_UNITSCANTTAME);
                    return SPELL_FAILED_DONT_REPORT;
                }

                Creature* target = (Creature*)tameTarget;

                if (target->IsPet() || target->HasCharmer())
                {
                    plrCaster->SendPetTameFailure(PETTAME_CREATUREALREADYOWNED);
                    return SPELL_FAILED_DONT_REPORT;
                }

                if (target->getLevel() > plrCaster->getLevel() && !gmmode)
                {
                    plrCaster->SendPetTameFailure(PETTAME_TOOHIGHLEVEL);
                    return SPELL_FAILED_DONT_REPORT;
                }

                if (!target->GetCreatureInfo()->isTameable())
                {
                    plrCaster->SendPetTameFailure(PETTAME_NOTTAMEABLE);
                    return SPELL_FAILED_DONT_REPORT;
                }

                if (plrCaster->GetPetGuid() || plrCaster->HasCharm())
                {
                    plrCaster->SendPetTameFailure(PETTAME_ANOTHERSUMMONACTIVE);
                    return SPELL_FAILED_DONT_REPORT;
                }

                break;
            }
            case SPELL_EFFECT_LEARN_SPELL:
            {
                if (m_spellInfo->EffectImplicitTargetA[i] != TARGET_UNIT_CASTER_PET)
                    break;

                Pet* pet = m_caster->GetPet();

                if (!pet)
                    return SPELL_FAILED_NO_PET;

                SpellEntry const* learn_spellproto = sSpellTemplate.LookupEntry<SpellEntry>(m_spellInfo->EffectTriggerSpell[i]);

                if (!learn_spellproto)
                    return SPELL_FAILED_NOT_KNOWN;

                if (!pet->CanTakeMoreActiveSpells(learn_spellproto->Id))
                    return SPELL_FAILED_TOO_MANY_SKILLS;

                if (m_spellInfo->spellLevel > pet->getLevel())
                    return SPELL_FAILED_LOWLEVEL;

                if (!pet->HasTPForSpell(learn_spellproto->Id))
                    return SPELL_FAILED_TRAINING_POINTS;

                break;
            }
            case SPELL_EFFECT_LEARN_PET_SPELL:
            {
                Pet* pet = m_caster->GetPet();

                if (!pet)
                    return SPELL_FAILED_NO_PET;

                SpellEntry const* learn_spellproto = sSpellTemplate.LookupEntry<SpellEntry>(m_spellInfo->EffectTriggerSpell[i]);

                if (!learn_spellproto)
                    return SPELL_FAILED_NOT_KNOWN;

                if (!pet->CanTakeMoreActiveSpells(learn_spellproto->Id))
                    return SPELL_FAILED_TOO_MANY_SKILLS;

                if (m_spellInfo->spellLevel > pet->getLevel())
                    return SPELL_FAILED_LOWLEVEL;

                if (!pet->HasTPForSpell(learn_spellproto->Id))
                    return SPELL_FAILED_TRAINING_POINTS;

                break;
            }
            case SPELL_EFFECT_FEED_PET:
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER)
                    return SPELL_FAILED_BAD_TARGETS;

                Item* foodItem = m_targets.getItemTarget();
                if (!foodItem)
                    return SPELL_FAILED_BAD_TARGETS;

                Pet* pet = m_caster->GetPet();

                if (!pet)
                    return SPELL_FAILED_NO_PET;

                if (!pet->HaveInDiet(foodItem->GetProto()))
                    return SPELL_FAILED_WRONG_PET_FOOD;

                if (!pet->GetCurrentFoodBenefitLevel(foodItem->GetProto()->ItemLevel))
                    return SPELL_FAILED_FOOD_LOWLEVEL;

                if (pet->isInCombat())
                    return SPELL_FAILED_AFFECTING_COMBAT;

                break;
            }
            case SPELL_EFFECT_POWER_BURN:
            case SPELL_EFFECT_POWER_DRAIN:
            {
                // Can be area effect, Check only for players and not check if target - caster (spell can have multiply drain/burn effects)
                if (m_caster->GetTypeId() == TYPEID_PLAYER)
                    if (Unit* target = m_targets.getUnitTarget())
                        if (target != m_caster && int32(target->GetPowerType()) != m_spellInfo->EffectMiscValue[i])
                            return SPELL_FAILED_BAD_TARGETS;
                break;
            }
            case SPELL_EFFECT_CHARGE:
            {
                if (m_caster->hasUnitState(UNIT_STAT_ROOT))
                    return SPELL_FAILED_ROOTED;

                if (Unit* target = m_targets.getUnitTarget())
                {
                    float range = GetSpellMaxRange(sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex));

                    WorldLocation pos;
                    target->GetFirstCollisionPosition(pos, target->GetCombatReach(), target->GetAngle(m_caster));

                    // TODO: Implement jumpin case check
                    //if (!m_caster->m_movementInfo.HasMovementFlag(MovementFlags(MOVEFLAG_FALLING | MOVEFLAG_FALLINGFAR)) && (pos.coord_z < m_caster->GetPositionZ()) && (fabs(pos.coord_z - m_caster->GetPositionZ()) < 3.0f))
                    //{
                    PathFinder pathFinder(m_caster);
                    pathFinder.setPathLengthLimit(range * 1.5f);
                    bool result = pathFinder.calculate(pos.coord_x, pos.coord_y, pos.coord_z);

                    if (pathFinder.getPathType() & PATHFIND_SHORT)
                        return SPELL_FAILED_OUT_OF_RANGE;
                    if (!result || pathFinder.getPathType() & PATHFIND_NOPATH)
                        return SPELL_FAILED_NOPATH;
                    //}

                }
                break;
            }
            case SPELL_EFFECT_SKINNING:
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER || !m_targets.getUnitTarget() || m_targets.getUnitTarget()->GetTypeId() != TYPEID_UNIT)
                    return SPELL_FAILED_BAD_TARGETS;

                if (!m_targets.getUnitTarget()->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE))
                    return SPELL_FAILED_TARGET_UNSKINNABLE;

                Creature* creature = (Creature*)m_targets.getUnitTarget();

                if (creature->isAlive())
                    return SPELL_FAILED_TARGET_NOT_DEAD;

                if (creature->GetLootStatus() != CREATURE_LOOT_STATUS_LOOTED)// || creature->GetCreatureType() != CREATURE_TYPE_CRITTER)
                    return SPELL_FAILED_TARGET_NOT_LOOTED;

                uint32 skill = creature->GetCreatureInfo()->GetRequiredLootSkill();

                int32 skillValue = ((Player*)m_caster)->GetSkillValue(skill);
                int32 TargetLevel = m_targets.getUnitTarget()->getLevel();
                int32 ReqValue = (skillValue < 100 ? (TargetLevel - 10) * 10 : TargetLevel * 5);
                if (ReqValue > skillValue)
                    return SPELL_FAILED_LOW_CASTLEVEL;

                // chance for fail at orange skinning attempt
                if (!strict && skillValue < sWorld.GetConfigMaxSkillValue() && (ReqValue < 0 ? 0 : ReqValue) > irand(skillValue - 25, skillValue + 37))
                    return SPELL_FAILED_TRY_AGAIN;

                break;
            }
            case SPELL_EFFECT_OPEN_LOCK_ITEM:
            case SPELL_EFFECT_OPEN_LOCK:
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER) // only players can open locks, gather etc.
                    return SPELL_FAILED_BAD_TARGETS;

                // we need a go target in case of TARGET_GAMEOBJECT (for other targets acceptable GO and items)
                if (m_spellInfo->EffectImplicitTargetA[i] == TARGET_GAMEOBJECT)
                {
                    if (!m_targets.getGOTarget())
                        return SPELL_FAILED_BAD_TARGETS;
                }

                // get the lock entry
                uint32 lockId;
                if (GameObject* go = m_targets.getGOTarget())
                {
                    // In BattleGround players can use only flags and banners
                    if (((Player*)m_caster)->InBattleGround() &&
                            !((Player*)m_caster)->CanUseBattleGroundObject())
                        return SPELL_FAILED_TRY_AGAIN;

                    lockId = go->GetGOInfo()->GetLockId();
                    if (!lockId)
                        return SPELL_FAILED_ALREADY_OPEN;

                    // check if its in use only when cast is finished (called from spell::cast() with strict = false)
                    if (!strict && go->IsInUse())
                        return SPELL_FAILED_CHEST_IN_USE;
                }
                else if (Item* item = m_targets.getItemTarget())
                {
                    // not own (trade?)
                    if (item->GetOwner() != m_caster)
                        return SPELL_FAILED_ITEM_GONE;

                    lockId = item->GetProto()->LockID;

                    // if already unlocked
                    if (!lockId || item->HasFlag(ITEM_FIELD_FLAGS, ITEM_DYNFLAG_UNLOCKED))
                        return SPELL_FAILED_ALREADY_OPEN;
                }
                else
                    return SPELL_FAILED_BAD_TARGETS;

                SkillType skillId = SKILL_NONE;
                int32 reqSkillValue = 0;
                int32 skillValue = 0;

                // check lock compatibility
                SpellCastResult res = CanOpenLock(SpellEffectIndex(i), lockId, skillId, reqSkillValue, skillValue);
                if (res != SPELL_CAST_OK)
                    return res;

                // chance for fail at orange mining/herb/LockPicking gathering attempt
                // second check prevent fail at rechecks
                if (!strict && skillId != SKILL_NONE)
                {
                    bool canFailAtMax = skillId != SKILL_HERBALISM && skillId != SKILL_MINING;

                    // chance for failure in orange gather / lockpick (gathering skill can't fail at maxskill)
                    if ((canFailAtMax || skillValue < sWorld.GetConfigMaxSkillValue()) && reqSkillValue > irand(skillValue - 25, skillValue + 37))
                        return SPELL_FAILED_TRY_AGAIN;
                }
                break;
            }
            case SPELL_EFFECT_SUMMON_DEAD_PET:
            {
                Creature* pet = m_caster->GetPet();
                if (!pet)
                {
                    SpellCastResult result = Pet::TryLoadFromDB(m_caster);
                    if (result == SPELL_FAILED_NO_PET)
                        return SPELL_FAILED_NO_PET;

                    if (result == SPELL_CAST_OK)
                    {
                        ((Player*)m_caster)->SendPetTameFailure(PETTAME_NOTDEAD);
                        return SPELL_FAILED_DONT_REPORT;
                    }

                    if (result != SPELL_FAILED_TARGETS_DEAD)
                        return SPELL_FAILED_UNKNOWN;
                }
                else if (pet->isAlive())
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                break;
            }
            // Don't make this check for SPELL_EFFECT_SUMMON_CRITTER, SPELL_EFFECT_SUMMON_WILD or SPELL_EFFECT_SUMMON_GUARDIAN.
            // These won't show up in m_caster->GetPetGUID()
            case SPELL_EFFECT_SUMMON:
            case SPELL_EFFECT_SUMMON_POSSESSED:
            case SPELL_EFFECT_SUMMON_PHANTASM:
            case SPELL_EFFECT_SUMMON_DEMON:
            {
                if (m_caster->GetPetGuid())
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                if (m_caster->HasCharm())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                break;
            }
            case SPELL_EFFECT_SUMMON_PET:
            {
                if (m_caster->HasCharm())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                uint32 plClass = m_caster->getClass();
                if (plClass == CLASS_HUNTER)
                {
                    if (Creature* pet = m_caster->GetPet())
                    {
                        if (!pet->isAlive())
                        {
                            ((Player*)m_caster)->SendPetTameFailure(PETTAME_DEAD);
                            return SPELL_FAILED_DONT_REPORT;
                        }
                        return SPELL_FAILED_ALREADY_HAVE_SUMMON;
                    }
                    SpellCastResult result = Pet::TryLoadFromDB(m_caster);
                    if (result == SPELL_FAILED_TARGETS_DEAD)
                    {
                        ((Player*)m_caster)->SendPetTameFailure(PETTAME_DEAD);
                        return SPELL_FAILED_DONT_REPORT;
                    }
                    if (result != SPELL_CAST_OK)
                        return result;
                }
                else if (m_caster->GetPetGuid())
                {
                    if (plClass == CLASS_WARLOCK)                  // let warlock do a replacement summon
                        static_cast<Player*>(m_caster)->UnsummonPetIfAny();
                    else
                        return SPELL_FAILED_ALREADY_HAVE_SUMMON;
                }

                break;
            }
            case SPELL_EFFECT_SUMMON_PLAYER:
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER)
                    return SPELL_FAILED_BAD_TARGETS;
                Player* caster = static_cast<Player*>(m_caster);
                if (!caster->GetSelectionGuid())
                    return SPELL_FAILED_BAD_TARGETS;

                Player* target = sObjectMgr.GetPlayer(caster->GetSelectionGuid());
                if (!target || caster == target || !target->IsInGroup(m_caster))
                    return SPELL_FAILED_BAD_TARGETS;

                // check if our map is dungeon
                uint32 mapId = m_caster->GetMapId();
                const MapEntry* map = sMapStore.LookupEntry(mapId);
                if (map->IsDungeon())
                {
                    InstanceTemplate const* instance = ObjectMgr::GetInstanceTemplate(mapId);
                    if (!instance)
                        return SPELL_FAILED_TARGET_NOT_IN_INSTANCE;
                    if (instance->levelMin > target->getLevel())
                        return SPELL_FAILED_LOWLEVEL;
                    if (instance->levelMax && instance->levelMax < target->getLevel())
                        return SPELL_FAILED_HIGHLEVEL;

                    if (InstancePlayerBind* targetBind = target->GetBoundInstance(mapId))
                        if (InstancePlayerBind* casterBind = caster->GetBoundInstance(mapId))
                            if (targetBind->perm && targetBind->state != casterBind->state)
                                return SPELL_FAILED_UNKNOWN; // TODO: Error was added in TBC maybe wrong

                    if (AreaTrigger const* at = sObjectMgr.GetMapEntranceTrigger(mapId))
                    {
                        uint32 miscRequirement;
                        if (AREA_LOCKSTATUS_OK != target->GetAreaTriggerLockStatus(at, miscRequirement))
                            return SPELL_FAILED_BAD_TARGETS; // TODO: Verify this result
                    }
                }
                break;
            }
            case SPELL_EFFECT_LEAP:
            case SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER:
            {
                if (!m_caster || m_caster->IsTaxiFlying())
                    return SPELL_FAILED_NOT_ON_TAXI;

                // Blink has leap first and then removing of auras with root effect
                // need further research with this
                if (m_spellInfo->Effect[i] != SPELL_EFFECT_LEAP)
                {
                    if (m_caster->hasUnitState(UNIT_STAT_ROOT))
                        return SPELL_FAILED_ROOTED;
                }

                if (m_caster->GetTypeId() == TYPEID_PLAYER)
                {
                    if (((Player*)m_caster)->HasMovementFlag(MOVEFLAG_ONTRANSPORT))
                        return SPELL_FAILED_NOT_ON_TRANSPORT;

                    // not allow use this effect at battleground until battleground start
                    if (BattleGround const* bg = ((Player*)m_caster)->GetBattleGround())
                        if (bg->GetStatus() != STATUS_IN_PROGRESS)
                            return SPELL_FAILED_TRY_AGAIN;
                }

                break;
            }
            default: break;
        }
    }

    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        // Do not check in case of junk in DBC
        if (!IsAuraApplyEffect(m_spellInfo, SpellEffectIndex(i)))
            continue;

        // Possible Unit-target for the spell
        Unit* expectedTarget = GetPrefilledUnitTargetOrUnitTarget(SpellEffectIndex(i));

        switch (m_spellInfo->EffectApplyAuraName[i])
        {
            case SPELL_AURA_MOD_POSSESS:
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER)
                    return SPELL_FAILED_UNKNOWN;

                if (m_caster->HasCharm())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                if (m_caster->HasCharmer())
                    return SPELL_FAILED_CHARMED;

                if (expectedTarget) // target may not be known at CheckCast time - TODO: add support for these checks on CheckTarget
                {
                    if (expectedTarget == m_caster)
                        return SPELL_FAILED_BAD_TARGETS;

                    if (expectedTarget->HasCharmer())
                        return SPELL_FAILED_CHARMED;

                    if (int32(expectedTarget->getLevel()) > CalculateDamage(SpellEffectIndex(i), expectedTarget))
                        return SPELL_FAILED_HIGHLEVEL;
                }
                break;
            }
            case SPELL_AURA_MOD_CHARM:
            {
                if (expectedTarget == m_caster)
                    return SPELL_FAILED_BAD_TARGETS;

                if (m_caster->GetTypeId() == TYPEID_PLAYER && m_caster->HasCharm())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                if (m_caster->HasCharmer())
                    return SPELL_FAILED_CHARMED;

                if (expectedTarget) // target may not be known at CheckCast time - TODO: add support for these checks on CheckTarget
                {
                    if (expectedTarget == m_caster)
                        return SPELL_FAILED_BAD_TARGETS;

                    if (expectedTarget->HasCharmer())
                        return SPELL_FAILED_CHARMED;

                    if (int32(expectedTarget->getLevel()) > CalculateDamage(SpellEffectIndex(i), expectedTarget))
                        return SPELL_FAILED_HIGHLEVEL;
                }
                break;
            }
            case SPELL_AURA_MOD_POSSESS_PET:
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER)
                    return SPELL_FAILED_UNKNOWN;

                if (m_caster->HasCharm())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                if (m_caster->HasCharmer())
                    return SPELL_FAILED_CHARMED;

                Pet* pet = m_caster->GetPet();
                if (!pet)
                    return SPELL_FAILED_NO_PET;

                if (pet->HasCharmer())
                    return SPELL_FAILED_CHARMED;

                break;
            }
            case SPELL_AURA_AOE_CHARM:
            {
                if (m_caster->HasCharmer())
                    return SPELL_FAILED_CHARMED;

                break;
            }
            case SPELL_AURA_MOUNTED:
            {
                if (m_caster->IsInWater() && (m_caster->GetTypeId() != TYPEID_PLAYER || static_cast<Player*>(m_caster)->IsInHighLiquid()))
                    return SPELL_FAILED_ONLY_ABOVEWATER;

                if (m_caster->GetTypeId() == TYPEID_PLAYER && ((Player*)m_caster)->GetTransport())
                    return SPELL_FAILED_NO_MOUNTS_ALLOWED;

                /// Specific case for Temple of Ahn'Qiraj mounts as they are usable only in AQ40 and are the only mounts allowed here
                /// TBC and above handle this by using m_spellInfo->AreaId
                bool isAQ40Mounted = false;

                switch (m_spellInfo->Id)
                {
                    case 25863:    // spell used by ingame item for Black Qiraji mount (legendary reward)
                    case 26655:    // spells also related to Black Qiraji mount but use/trigger unknown
                    case 26656:
                    case 31700:
                        if (m_caster->GetMapId() == 531)
                            isAQ40Mounted = true;
                        break;
                    case 25953:    // spells of the 4 regular AQ40 mounts
                    case 26054:
                    case 26055:
                    case 26056:
                        if (m_caster->GetMapId() == 531)
                        {
                            isAQ40Mounted = true;
                            break;
                        }
                        else
                            return SPELL_FAILED_NOT_HERE;
                    default:
                        break;
                }

                // Ignore map check if spell have AreaId. AreaId already checked and this prevent special mount spells
                if (m_caster->GetTypeId() == TYPEID_PLAYER &&
                        !m_IsTriggeredSpell &&
                        !isAQ40Mounted &&   // [-ZERO] && !m_spellInfo->AreaId)
                        (m_caster->GetMap() && !m_caster->GetMap()->IsMountAllowed()))
                {
                    return SPELL_FAILED_NO_MOUNTS_ALLOWED;
                }

                if (m_caster->GetAreaId() == 35)
                    return SPELL_FAILED_NO_MOUNTS_ALLOWED;

                if (m_caster->IsInDisallowedMountForm())
                    return SPELL_FAILED_NOT_SHAPESHIFT;

                break;
            }
            case SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS:
            {
                if (!expectedTarget)
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                // can be casted at non-friendly unit or own pet/charm
                if (!m_caster->CanAttack(expectedTarget))
                    return SPELL_FAILED_TARGET_FRIENDLY;

                break;
            }
            case SPELL_AURA_PERIODIC_MANA_LEECH:
            {
                if (!expectedTarget)
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                if (m_caster->GetTypeId() != TYPEID_PLAYER || m_CastItem)
                    break;

                if (expectedTarget->GetPowerType() != POWER_MANA)
                    return SPELL_FAILED_BAD_TARGETS;

                break;
            }
            case SPELL_AURA_WATER_WALK:
            {
                if (expectedTarget && expectedTarget->GetTypeId() == TYPEID_PLAYER)
                {
                    Player const* player = static_cast<Player const*>(expectedTarget);

                    // Player is not allowed to cast water walk on shapeshifted/mounted player
                    if (player->IsInDisallowedMountForm() || player->IsMounted())
                        return SPELL_FAILED_BAD_TARGETS;
                }
            }
            case SPELL_AURA_MOD_DISARM:
            {
                if (expectedTarget)
                {
                    // Target must be a weapon wielder
                    if (!expectedTarget->hasMainhandWeapon())
                        return SPELL_FAILED_BAD_TARGETS;
                }
                break;
            }
            default:
                break;
        }
    }

    // check trade slot case (last, for allow catch any another cast problems)
    if (m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
    {
        if (m_caster->GetTypeId() != TYPEID_PLAYER)
            return SPELL_FAILED_NOT_TRADING;

        if (TradeSlots(m_targets.getItemTargetGuid().GetRawValue()) != TRADE_SLOT_NONTRADED)
            return SPELL_FAILED_ITEM_NOT_READY;

        // if trade not complete then remember it in trade data
        if (TradeData* my_trade = ((Player*)m_caster)->GetTradeData())
        {
            if (!my_trade->IsInAcceptProcess())
            {
                // Spell will be casted at completing the trade. Silently ignore at this place
                my_trade->SetSpell(m_spellInfo->Id, m_CastItem);
                return SPELL_FAILED_DONT_REPORT;
            }
        }
        else
            return SPELL_FAILED_NOT_TRADING;
    }

    if (m_caster->GetTypeId() == TYPEID_PLAYER && m_spellInfo->HasAttribute(SPELL_ATTR_EX2_TAME_BEAST))
    {
        Player* player = (Player*)m_caster;
        if (player->GetPetGuid() || player->HasCharm())
        {
            player->SendPetTameFailure(PETTAME_ANOTHERSUMMONACTIVE);
            return SPELL_FAILED_DONT_REPORT;
        }
        SpellCastResult result = Pet::TryLoadFromDB((Player*)m_caster);
        if (result == SPELL_FAILED_TARGETS_DEAD || result == SPELL_CAST_OK)
        {
            player->SendPetTameFailure(PETTAME_ANOTHERSUMMONACTIVE);
            return SPELL_FAILED_DONT_REPORT;
        }
    }

    // all ok
    return OnCheckCast(strict);
}

SpellCastResult Spell::CheckPetCast(Unit* target)
{
    if (!m_caster->isAlive())
        return SPELL_FAILED_CASTER_DEAD;

    if (m_caster->IsNonMeleeSpellCasted(false) && !m_spellInfo->HasAttribute(SPELL_ATTR_EX4_CAN_CAST_WHILE_CASTING)) // prevent spellcast interruption by another spellcast
        return SPELL_FAILED_SPELL_IN_PROGRESS;
    if (m_caster->isInCombat() && IsNonCombatSpell(m_spellInfo))
        return SPELL_FAILED_AFFECTING_COMBAT;

    if (m_caster->GetTypeId() == TYPEID_UNIT && (((Creature*)m_caster)->IsPet() || m_caster->HasCharmer()))
    {
        // dead owner (pets still alive when owners ressed?)
        if (m_caster->GetMaster() && !m_caster->GetMaster()->isAlive())
            return SPELL_FAILED_CASTER_DEAD;

        if (!target && m_targets.getUnitTarget())
            target = m_targets.getUnitTarget();

        bool need = false;
        bool script = false;
        for (unsigned int i : m_spellInfo->EffectImplicitTargetA)
        {
            if (i == TARGET_UNIT_ENEMY ||
                i == TARGET_UNIT_FRIEND ||
                i == TARGET_UNIT_RAID ||
                i == TARGET_UNIT ||
                i == TARGET_UNIT_PARTY ||
                i == TARGET_LOCATION_CASTER_TARGET_POSITION)
            {
                need = true;
                if (!target)
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;
                break;
            }
            else if (i == TARGET_LOCATION_SCRIPT_NEAR_CASTER)
            {
                script = true;
            }
        }
        if (need)
            m_targets.setUnitTarget(target);
        else if (script)
            return CheckCast(true);

        if (Unit* _target = m_targets.getUnitTarget())      // for target dead/target not valid
        {
            if (IsPositiveSpell(m_spellInfo->Id, m_caster, _target))
            {
                if (!m_caster->CanAssistSpell(_target, m_spellInfo))
                    return SPELL_FAILED_BAD_TARGETS;
            }
            else
            {
                bool duelvsplayertar = false;
                for (unsigned int j : m_spellInfo->EffectImplicitTargetA)
                {
                    // TARGET_UNIT is positive AND negative
                    duelvsplayertar |= (j == TARGET_UNIT);
                }
                if (!m_caster->CanAttack(target) && !duelvsplayertar)
                {
                    return SPELL_FAILED_BAD_TARGETS;
                }
            }
        }
        // cooldown
        if (!m_caster->IsSpellReady(*m_spellInfo))
            return SPELL_FAILED_NOT_READY;
    }

    return CheckCast(true);
}

SpellCastResult Spell::CheckCasterAuras() const
{
    // Flag drop spells totally immuned to caster auras
    // FIXME: find more nice check for all totally immuned spells
    // HasAttribute(SPELL_ATTR_EX3_UNK28) ?
    if (m_spellInfo->Id == 23336 ||                         // Alliance Flag Drop
            m_spellInfo->Id == 23334)                       // Horde Flag Drop
        return SPELL_CAST_OK;

    uint8 school_immune = 0;
    uint32 mechanic_immune = 0;
    uint32 dispel_immune = 0;

    // Check if the spell grants school or mechanic immunity.
    // We use bitmasks so the loop is done only once and not on every aura check below.
    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY))
    {
        for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (m_spellInfo->EffectApplyAuraName[i] == SPELL_AURA_SCHOOL_IMMUNITY)
                school_immune |= uint32(m_spellInfo->EffectMiscValue[i]);
            else if (m_spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MECHANIC_IMMUNITY)
                mechanic_immune |= 1 << uint32(m_spellInfo->EffectMiscValue[i] - 1);
            else if (m_spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MECHANIC_IMMUNITY_MASK)
                mechanic_immune |= uint32(m_spellInfo->EffectMiscValue[i]);
            else if (m_spellInfo->EffectApplyAuraName[i] == SPELL_AURA_DISPEL_IMMUNITY)
                dispel_immune |= GetDispellMask(DispelType(m_spellInfo->EffectMiscValue[i]));
        }
    }

    // Check whether the cast should be prevented by any state you might have.
    SpellCastResult prevented_reason = SPELL_CAST_OK;
    // Have to check if there is a stun aura. Otherwise will have problems with ghost aura apply while logging out
    uint32 unitflag = m_caster->GetUInt32Value(UNIT_FIELD_FLAGS);     // Get unit state
    /*[-ZERO]    if (unitflag & UNIT_FLAG_STUNNED && !(m_spellInfo->AttributesEx5 & SPELL_ATTR_EX5_USABLE_WHILE_STUNNED))
            prevented_reason = SPELL_FAILED_STUNNED;
        else if (unitflag & UNIT_FLAG_CONFUSED && !(m_spellInfo->AttributesEx5 & SPELL_ATTR_EX5_USABLE_WHILE_CONFUSED))
            prevented_reason = SPELL_FAILED_CONFUSED;
        else if (unitflag & UNIT_FLAG_FLEEING && !(m_spellInfo->AttributesEx5 & SPELL_ATTR_EX5_USABLE_WHILE_FEARED))
            prevented_reason = SPELL_FAILED_FLEEING;
        else */
    if (unitflag & UNIT_FLAG_SILENCED && m_spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE)
        prevented_reason = SPELL_FAILED_SILENCED;
    else if (unitflag & UNIT_FLAG_PACIFIED && m_spellInfo->PreventionType == SPELL_PREVENTION_TYPE_PACIFY)
        prevented_reason = SPELL_FAILED_PACIFIED;

    // Attr must make flag drop spell totally immune from all effects
    if (prevented_reason != SPELL_CAST_OK)
    {
        if (school_immune || mechanic_immune || dispel_immune)
        {
            // Checking auras is needed now, because you are prevented by some state but the spell grants immunity.
            Unit::SpellAuraHolderMap const& auras = m_caster->GetSpellAuraHolderMap();
            for (const auto& itr : auras)
            {
                SpellAuraHolder* holder = itr.second;
                SpellEntry const* pEntry = holder->GetSpellProto();

                if ((GetSpellSchoolMask(pEntry) & school_immune) && !pEntry->HasAttribute(SPELL_ATTR_EX_UNAFFECTED_BY_SCHOOL_IMMUNE))
                    continue;
                if ((1 << (pEntry->Dispel)) & dispel_immune)
                    continue;

                for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
                {
                    Aura* aura = holder->GetAuraByEffectIndex(SpellEffectIndex(i));
                    if (!aura)
                        continue;

                    if (GetSpellMechanicMask(pEntry, 1 << i) & mechanic_immune)
                        continue;
                    // Make a second check for spell failed so the right SPELL_FAILED message is returned.
                    // That is needed when your casting is prevented by multiple states and you are only immune to some of them.
                    switch (aura->GetModifier()->m_auraname)
                    {
                        /* Zero
                        case SPELL_AURA_MOD_STUN:
                            if (!(m_spellInfo->AttributesEx5 & SPELL_ATTR_EX5_USABLE_WHILE_STUNNED))
                                return SPELL_FAILED_STUNNED;
                            break;
                        case SPELL_AURA_MOD_CONFUSE:
                            if (!(m_spellInfo->AttributesEx5 & SPELL_ATTR_EX5_USABLE_WHILE_CONFUSED))
                                return SPELL_FAILED_CONFUSED;
                            break;
                        case SPELL_AURA_MOD_FEAR:
                            if (!(m_spellInfo->AttributesEx5 & SPELL_ATTR_EX5_USABLE_WHILE_FEARED))
                                return SPELL_FAILED_FLEEING;
                            break;
                        */
                        case SPELL_AURA_MOD_SILENCE:
                        case SPELL_AURA_MOD_PACIFY:
                        case SPELL_AURA_MOD_PACIFY_SILENCE:
                        {
                            if (m_spellInfo->PreventionType == SPELL_PREVENTION_TYPE_PACIFY)
                                return SPELL_FAILED_PACIFIED;
                            if (m_spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE)
                                return SPELL_FAILED_SILENCED;
                        }
                            break;
                        default: break;
                    }
                }
            }
        }
        // You are prevented from casting and the spell casted does not grant immunity. Return a failed error.
        else
            return prevented_reason;
    }
    return SPELL_CAST_OK;
}

bool Spell::CanAutoCast(Unit* target)
{
    ObjectGuid targetguid = target->GetObjectGuid();

    for (int j = 0; j < MAX_EFFECT_INDEX; ++j)
    {
        if (m_spellInfo->Effect[j] == SPELL_EFFECT_APPLY_AURA)
        {
            if (m_spellInfo->StackAmount <= 1)
            {
                if (target->HasAura(m_spellInfo->Id, SpellEffectIndex(j)))
                    return false;
            }
            else
            {
                if (Aura* aura = target->GetAura(m_spellInfo->Id, SpellEffectIndex(j)))
                    if (aura->GetStackAmount() >= m_spellInfo->StackAmount)
                        return false;
            }
        }
        else if (IsAreaAuraEffect(m_spellInfo->Effect[j]))
        {
            if (target->HasAura(m_spellInfo->Id, SpellEffectIndex(j)))
                return false;
        }
    }

    SpellCastResult result = CheckPetCast(target);

    if (result == SPELL_CAST_OK || result == SPELL_FAILED_UNIT_NOT_INFRONT)
    {
        FillTargetMap();
        // check if among target units, our WANTED target is as well (->only self cast spells return false)
        for (TargetList::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
            if (ihit->targetGUID == targetguid)
                return true;
    }
    return false;                                           // target invalid
}

std::pair<float, float> Spell::GetMinMaxRange(bool strict)
{
    float minRange = 0.0f, maxRange = 0.0f, rangeMod = 0.0f;

    if (strict && IsNextMeleeSwingSpell())
        return { 0.0f, 100.0f };

    Unit* caster = dynamic_cast<Unit*>(m_caster); // preparation for GO casting

    SpellRangeEntry const* spellRange = sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex);
    if (spellRange)
    {
        Unit* target = m_targets.getUnitTarget();
        if (spellRange->Flags & SPELL_RANGE_FLAG_MELEE)
        {
            if (caster)
                rangeMod = caster->GetCombinedCombatReach(target ? target : caster, true, 0.f);
        }
        else
        {
            float meleeRange = 0.f;
            if (spellRange->Flags & SPELL_RANGE_FLAG_RANGED)
            {
                if (caster)
                    meleeRange = caster->GetCombinedCombatReach(target ? target : caster, true, 0.f);
            }

            minRange = GetSpellMinRange(spellRange) + meleeRange;
            maxRange = GetSpellMaxRange(spellRange);

            if (target || m_targets.getCorpseTarget())
            {
                rangeMod = m_caster->GetCombatReach() + (target ? target->GetCombatReach() : m_caster->GetCombatReach());

                if (minRange > 0.0f && !(spellRange->Flags & SPELL_RANGE_FLAG_RANGED))
                    minRange += rangeMod;
            }
        }

        if (target && caster && caster->IsMovingForward() && target->IsMovingForward() && !caster->IsWalking() && !target->IsWalking() &&
            ((spellRange->Flags & SPELL_RANGE_FLAG_MELEE) || target->GetTypeId() == TYPEID_PLAYER))
            rangeMod += MELEE_LEEWAY;
    }

    if (m_spellInfo->HasAttribute(SPELL_ATTR_RANGED) && m_caster->GetTypeId() == TYPEID_PLAYER)
        if (Item* ranged = static_cast<Player*>(m_caster)->GetWeaponForAttack(RANGED_ATTACK))
            maxRange *= ranged->GetProto()->RangedModRange * 0.01f;

    if (Player* modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_RANGE, maxRange, this);

    maxRange += rangeMod;

    return { minRange, maxRange };
}

SpellCastResult Spell::CheckRange(bool strict)
{
    // Don't check for instant cast spells
    if (!strict && m_casttime == 0)
        return SPELL_CAST_OK;

    WorldObject* target = m_targets.getUnitTarget();

    float minRange, maxRange;
    std::tie(minRange, maxRange) = GetMinMaxRange(strict);

    // non strict spell tolerance
    if (SpellRangeEntry const* spellRange = sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex))
        if ((spellRange->Flags & SPELL_RANGE_FLAG_MELEE) == 0 && !strict)
            maxRange += std::min(3.f, maxRange * 0.1f); // 10% but no more than MAX_SPELL_RANGE_TOLERANCE

    if (!target && m_spellInfo->Targets & (TARGET_FLAG_LOCKED | TARGET_FLAG_GAMEOBJECT))
        target = m_targets.getGOTarget();

    if (target && target != m_caster)
    {
        // distance from target in checks
        float dist = m_caster->GetDistance(target, true, DIST_CALC_NONE); // minRange/maxRange already contain everything
        if (dist > maxRange * maxRange)
           return SPELL_FAILED_OUT_OF_RANGE;
        if (minRange && dist < minRange * minRange)
            return SPELL_FAILED_TOO_CLOSE;
        if (m_caster->GetTypeId() == TYPEID_PLAYER &&
                (sSpellMgr.GetSpellFacingFlag(m_spellInfo->Id) & SPELL_FACING_FLAG_INFRONT) && !m_caster->HasInArc(target))
            return SPELL_FAILED_UNIT_NOT_INFRONT;
    }

    if (m_targets.m_targetMask == TARGET_FLAG_DEST_LOCATION)
    {
        float dist = m_caster->GetDistance(m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, DIST_CALC_NONE);
        if (dist > maxRange * maxRange)
            return SPELL_FAILED_OUT_OF_RANGE;
        if (minRange && dist < minRange * minRange)
            return SPELL_FAILED_TOO_CLOSE;
        if (!m_caster->IsWithinLOS(m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ))
            return SPELL_FAILED_LINE_OF_SIGHT;
    }

    m_maxRange = maxRange; // need to save max range for some calculations

    return SPELL_CAST_OK;
}

uint32 Spell::CalculatePowerCost(SpellEntry const* spellInfo, Unit* caster, Spell const* spell, Item* castItem)
{
    // item cast not used power
    if (castItem)
        return 0;

    // Spell drain all exist power on cast (Only paladin lay of Hands)
    if (spellInfo->HasAttribute(SPELL_ATTR_EX_DRAIN_ALL_POWER))
    {
        // If power type - health drain all
        if (spellInfo->powerType == POWER_HEALTH)
            return caster->GetHealth();
        // Else drain all power
        if (spellInfo->powerType < MAX_POWERS)
            return caster->GetPower(Powers(spellInfo->powerType));
        sLog.outError("Spell::CalculateManaCost: Unknown power type '%d' in spell %d", spellInfo->powerType, spellInfo->Id);
        return 0;
    }

    // Base powerCost
    int32 powerCost = spellInfo->manaCost + spellInfo->manaCostPerlevel * (int32(caster->GetSpellRank(spellInfo)) / 5  - spellInfo->baseLevel);
    // PCT cost from total amount
    if (spellInfo->ManaCostPercentage)
    {
        switch (spellInfo->powerType)
        {
            // health as power used
            case POWER_HEALTH:
                powerCost += spellInfo->ManaCostPercentage * caster->GetCreateHealth() / 100;
                break;
            case POWER_MANA:
                powerCost += spellInfo->ManaCostPercentage * caster->GetCreateMana() / 100;
                break;
            case POWER_RAGE:
            case POWER_FOCUS:
            case POWER_ENERGY:
            case POWER_HAPPINESS:
                powerCost += spellInfo->ManaCostPercentage * caster->GetMaxPower(Powers(spellInfo->powerType)) / 100;
                break;
            default:
                sLog.outError("Spell::CalculateManaCost: Unknown power type '%d' in spell %d", spellInfo->powerType, spellInfo->Id);
                return 0;
        }
    }
    SpellSchools school = GetFirstSchoolInMask(spell ? spell->m_spellSchoolMask : GetSpellSchoolMask(spellInfo));
    // Flat mod from caster auras by spell school
    powerCost += caster->GetInt32Value(UNIT_FIELD_POWER_COST_MODIFIER + school);
    // Apply cost mod by spell
    if (spell)
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_COST, powerCost, spell);

    if (spellInfo->HasAttribute(SPELL_ATTR_LEVEL_DAMAGE_CALCULATION))
        powerCost = int32(powerCost / (1.117f * spellInfo->spellLevel / caster->getLevel() - 0.1327f));

    // PCT mod from user auras by school
    powerCost = int32(powerCost * (1.0f + caster->GetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + school)));
    if (powerCost < 0)
        powerCost = 0;
    return powerCost;
}

SpellCastResult Spell::CheckPower()
{
    // item cast not used power
    if (m_CastItem)
        return SPELL_CAST_OK;

    // health as power used - need check health amount
    if (m_spellInfo->powerType == POWER_HEALTH)
    {
        if (m_caster->GetHealth() <= m_powerCost)
            return SPELL_FAILED_CASTER_AURASTATE;
        return SPELL_CAST_OK;
    }

    // Check valid power type
    if (m_spellInfo->powerType >= MAX_POWERS)
    {
        sLog.outError("Spell::CheckMana: Unknown power type '%d'", m_spellInfo->powerType);
        return SPELL_FAILED_UNKNOWN;
    }

    // Check power amount
    Powers powerType = Powers(m_spellInfo->powerType);
    if (m_caster->GetPower(powerType) < m_powerCost)
        return SPELL_FAILED_NO_POWER;

    return SPELL_CAST_OK;
}

bool Spell::IgnoreItemRequirements() const
{
    if (m_IsTriggeredSpell)
    {
        /// Not own traded item (in trader trade slot) req. reagents including triggered spell case
        if (Item* targetItem = m_targets.getItemTarget())
            if (targetItem->GetOwnerGuid() != m_caster->GetObjectGuid())
                return false;

        /// Some triggered spells have same reagents that have master spell
        /// expected in test: master spell have reagents in first slot then triggered don't must use own
        return !(m_triggeredBySpellInfo && !m_triggeredBySpellInfo->Reagent[0]);
    }

    return false;
}

SpellCastResult Spell::CheckItems()
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return SPELL_CAST_OK;

    Player* p_caster = (Player*)m_caster;
    Player* playerTarget = p_caster;
    if (Unit* target = m_targets.getUnitTarget())
        if (target->GetTypeId() == TYPEID_PLAYER)
            playerTarget = (Player*)target;

    // cast item checks
    if (m_CastItem)
    {
        if (m_CastItem->IsInTrade())
            return SPELL_FAILED_ITEM_GONE;

        uint32 itemid = m_CastItem->GetEntry();
        if (!p_caster->HasItemCount(itemid, 1))
            return SPELL_FAILED_ITEM_NOT_READY;

        ItemPrototype const* proto = m_CastItem->GetProto();
        if (!proto)
            return SPELL_FAILED_ITEM_NOT_READY;

        for (int i = 0; i < 5; ++i)
            if (proto->Spells[i].SpellCharges)
                if (m_CastItem->GetSpellCharges(i) == 0)
                    return SPELL_FAILED_NO_CHARGES_REMAIN;

        // consumable cast item checks
        if (proto->Class == ITEM_CLASS_CONSUMABLE && m_targets.getUnitTarget())
        {
            // such items should only fail if there is no suitable effect at all - see Rejuvenation Potions for example
            SpellCastResult failReason = SPELL_CAST_OK;
            for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
            {
                // skip check, pet not required like checks, and for TARGET_UNIT_CASTER_PET m_targets.getUnitTarget() is not the real target but the caster
                if (m_spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_CASTER_PET)
                    continue;

                if (m_spellInfo->Effect[i] == SPELL_EFFECT_HEAL)
                {
                    if (m_targets.getUnitTarget()->GetHealth() == m_targets.getUnitTarget()->GetMaxHealth())
                    {
                        failReason = SPELL_FAILED_ALREADY_AT_FULL_HEALTH;
                        continue;
                    }
                    failReason = SPELL_CAST_OK;
                    break;
                }

                // Mana Potion, Rage Potion, Thistle Tea(Rogue), ...
                if (m_spellInfo->Effect[i] == SPELL_EFFECT_ENERGIZE)
                {
                    if (m_spellInfo->EffectMiscValue[i] < 0 || m_spellInfo->EffectMiscValue[i] >= MAX_POWERS)
                    {
                        failReason = SPELL_FAILED_ALREADY_AT_FULL_POWER;
                        continue;
                    }

                    Powers power = Powers(m_spellInfo->EffectMiscValue[i]);
                    if (m_targets.getUnitTarget()->GetPower(power) == m_targets.getUnitTarget()->GetMaxPower(power))
                    {
                        failReason = SPELL_FAILED_ALREADY_AT_FULL_POWER;
                    }
                    else
                    {
                        failReason = SPELL_CAST_OK;
                        break;
                    }
                }
            }
            if (failReason != SPELL_CAST_OK)
                return failReason;
        }
    }

    // check target item (for triggered case not report error)
    if (m_targets.getItemTargetGuid())
    {
        if (m_caster->GetTypeId() != TYPEID_PLAYER)
            return m_IsTriggeredSpell && !(m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
                   ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_BAD_TARGETS;

        if (!m_targets.getItemTarget())
            return m_IsTriggeredSpell  && !(m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
                   ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_ITEM_GONE;

        if (!m_targets.getItemTarget()->IsFitToSpellRequirements(m_spellInfo))
            return m_IsTriggeredSpell  && !(m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
                   ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_EQUIPPED_ITEM_CLASS;
    }
    // if not item target then required item must be equipped (for triggered case not report error)
    else
    {
        if (m_caster->GetTypeId() == TYPEID_PLAYER && !((Player*)m_caster)->HasItemFitToSpellReqirements(m_spellInfo))
            return m_IsTriggeredSpell ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_EQUIPPED_ITEM_CLASS;
    }

    // check spell focus object
    if (m_spellInfo->RequiresSpellFocus)
    {
        GameObject* ok = nullptr;
        MaNGOS::GameObjectFocusCheck go_check(m_caster, m_spellInfo->RequiresSpellFocus);
        MaNGOS::GameObjectSearcher<MaNGOS::GameObjectFocusCheck> checker(ok, go_check);
        Cell::VisitGridObjects(m_caster, checker, m_caster->GetMap()->GetVisibilityDistance());

        if (!ok)
            return SPELL_FAILED_REQUIRES_SPELL_FOCUS;

        focusObject = ok;                                   // game object found in range
    }

    // check reagents (ignore triggered spells with reagents processed by original spell) and special reagent ignore case.
    if (!IgnoreItemRequirements())
    {
        if (!p_caster->CanNoReagentCast(m_spellInfo))
        {
            for (uint32 i = 0; i < MAX_SPELL_REAGENTS; ++i)
            {
                if (m_spellInfo->Reagent[i] <= 0)
                    continue;

                uint32 itemid    = m_spellInfo->Reagent[i];
                uint32 itemcount = m_spellInfo->ReagentCount[i];

                // if CastItem is also spell reagent
                if (m_CastItem && m_CastItem->GetEntry() == itemid)
                {
                    ItemPrototype const* proto = m_CastItem->GetProto();
                    if (!proto)
                        return SPELL_FAILED_ITEM_NOT_READY;
                    for (int s = 0; s < MAX_ITEM_PROTO_SPELLS; ++s)
                    {
                        // CastItem will be used up and does not count as reagent
                        int32 charges = m_CastItem->GetSpellCharges(s);
                        if (proto->Spells[s].SpellCharges < 0 && abs(charges) < 2)
                        {
                            ++itemcount;
                            break;
                        }
                    }
                }

                if (!p_caster->HasItemCount(itemid, itemcount))
                    return SPELL_FAILED_ITEM_NOT_READY;
            }
        }

        // check totem-item requirements (items presence in inventory)
        uint32 totems = MAX_SPELL_TOTEMS;
        for (unsigned int i : m_spellInfo->Totem)
        {
            if (i != 0)
            {
                if (p_caster->HasItemCount(i, 1))
                {
                    totems -= 1;
                }
            }
            else
                totems -= 1;
        }

        if (totems != 0)
            return SPELL_FAILED_ITEM_GONE;                  //[-ZERO] not sure of it

        /*[-ZERO] to rewrite?
        // Check items for TotemCategory  (items presence in inventory)
        uint32 TotemCategory = MAX_SPELL_TOTEM_CATEGORIES;
        for (unsigned int i : m_spellInfo->TotemCategory)
        {
            if (i != 0)
            {
                if (p_caster->HasItemTotemCategory(i))
                {
                    TotemCategory -= 1;
                }
            }
            else
                TotemCategory -= 1;
        }

        if (TotemCategory != 0)
            return SPELL_FAILED_TOTEM_CATEGORY;             // 0x7B
        */
    }

    // special checks for spell effects
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        switch (m_spellInfo->Effect[i])
        {
            case SPELL_EFFECT_CREATE_ITEM:
            {
                // check requirement at spell end
                if (m_spellState == SPELL_STATE_TARGETING)
                    break;

                if (!m_IsTriggeredSpell && m_spellInfo->EffectItemType[i])
                {
                    ItemPrototype const* itemProto = ObjectMgr::GetItemPrototype(m_spellInfo->EffectItemType[i]);
                    if (!itemProto)
                        return SPELL_FAILED_ITEM_NOT_FOUND; // custom error in case item template is missing
                    ItemPosCountVec dest;
                    uint32 count = CalculateDamage(SpellEffectIndex(i), m_caster);
                    count = count > itemProto->Stackable ? itemProto->Stackable : count;
                    InventoryResult msg = playerTarget->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, m_spellInfo->EffectItemType[i], count);
                    if (msg != EQUIP_ERR_OK)
                    {
                        p_caster->SendEquipError(msg, nullptr, nullptr, m_spellInfo->EffectItemType[i]);
                        return SPELL_FAILED_DONT_REPORT;
                    }
                }
                break;
            }
            case SPELL_EFFECT_ENCHANT_ITEM:
            {
                Item* targetItem = m_targets.getItemTarget();
                if (!targetItem)
                    return SPELL_FAILED_ITEM_GONE;

                if (targetItem->GetProto()->ItemLevel < m_spellInfo->baseLevel)
                    return SPELL_FAILED_LOWLEVEL;
                // Not allow enchant in trade slot for some enchant type
                if (targetItem->GetOwner() != m_caster)
                {
                    uint32 enchant_id = m_spellInfo->EffectMiscValue[i];
                    SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                    if (!pEnchant)
                        return SPELL_FAILED_ERROR;
                    if (pEnchant->slot & ENCHANTMENT_CAN_SOULBOUND)
                        return SPELL_FAILED_NOT_TRADEABLE;
                }
                break;
            }
            case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
            {
                Item* item = m_targets.getItemTarget();
                if (!item)
                    return SPELL_FAILED_ITEM_GONE;
                // Not allow enchant in trade slot for some enchant type
                if (item->GetOwner() != m_caster)
                {
                    uint32 enchant_id = m_spellInfo->EffectMiscValue[i];
                    SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                    if (!pEnchant)
                        return SPELL_FAILED_ERROR;
                    if (pEnchant->slot & ENCHANTMENT_CAN_SOULBOUND)
                        return SPELL_FAILED_NOT_TRADEABLE;
                }
                break;
            }
            case SPELL_EFFECT_ENCHANT_HELD_ITEM:
                // check item existence in effect code (not output errors at offhand hold item effect to main hand for example
                break;
            case SPELL_EFFECT_DISENCHANT:
            {
                if (!m_targets.getItemTarget())
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;

                // prevent disenchanting in trade slot
                if (m_targets.getItemTarget()->GetOwnerGuid() != m_caster->GetObjectGuid())
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;

                ItemPrototype const* itemProto = m_targets.getItemTarget()->GetProto();
                if (!itemProto)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;

                // must have disenchant loot (other static req. checked at item prototype loading)
                if (!itemProto->DisenchantID)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
                break;
            }
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER) return SPELL_FAILED_TARGET_NOT_PLAYER;
                if (m_attackType != RANGED_ATTACK)
                    break;
                Item* pItem = ((Player*)m_caster)->GetWeaponForAttack(m_attackType, true, false);
                if (!pItem)
                    return SPELL_FAILED_EQUIPPED_ITEM;

                switch (pItem->GetProto()->SubClass)
                {
                    case ITEM_SUBCLASS_WEAPON_THROWN:
                    {
                        uint32 ammo = pItem->GetEntry();
                        if (!((Player*)m_caster)->HasItemCount(ammo, 1))
                            return SPELL_FAILED_NO_AMMO;
                    }
                    break;
                    case ITEM_SUBCLASS_WEAPON_GUN:
                    case ITEM_SUBCLASS_WEAPON_BOW:
                    case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    {
                        uint32 ammo = ((Player*)m_caster)->GetUInt32Value(PLAYER_AMMO_ID);
                        if (!ammo)
                        {
                            // Requires No Ammo
                            if (m_caster->GetDummyAura(46699))
                                break;                      // skip other checks

                            return SPELL_FAILED_NO_AMMO;
                        }

                        ItemPrototype const* ammoProto = ObjectMgr::GetItemPrototype(ammo);
                        if (!ammoProto)
                            return SPELL_FAILED_NO_AMMO;

                        if (ammoProto->Class != ITEM_CLASS_PROJECTILE)
                            return SPELL_FAILED_NO_AMMO;

                        // check ammo ws. weapon compatibility
                        switch (pItem->GetProto()->SubClass)
                        {
                            case ITEM_SUBCLASS_WEAPON_BOW:
                            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                                if (ammoProto->SubClass != ITEM_SUBCLASS_ARROW)
                                    return SPELL_FAILED_NO_AMMO;
                                break;
                            case ITEM_SUBCLASS_WEAPON_GUN:
                                if (ammoProto->SubClass != ITEM_SUBCLASS_BULLET)
                                    return SPELL_FAILED_NO_AMMO;
                                break;
                            default:
                                return SPELL_FAILED_NO_AMMO;
                        }

                        if (!((Player*)m_caster)->HasItemCount(ammo, 1))
                            return SPELL_FAILED_NO_AMMO;
                    }
                        break;
                    case ITEM_SUBCLASS_WEAPON_WAND:
                        break;
                    default:
                        break;
                }
                break;
            }
            default: break;
        }
    }

    return SPELL_CAST_OK;
}

void Spell::Delayed()
{
    if (!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if (m_spellState == SPELL_STATE_TRAVELING)
        return;                                             // spell is active and can't be time-backed

    // spells not loosing casting time ( slam, dynamites, bombs.. )
    if (!(m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_DAMAGE))
        return;

    // check resist chance
    int32 resistChance = 100;                               // must be initialized to 100 for percent modifiers
    ((Player*)m_caster)->ApplySpellMod(m_spellInfo->Id, SPELLMOD_NOT_LOSE_CASTING_TIME, resistChance, this);
    resistChance += m_caster->GetTotalAuraModifier(SPELL_AURA_RESIST_PUSHBACK) - 100;
    if (roll_chance_i(resistChance))
        return;

    int32 delaytime = GetNextDelayAtDamageMsTime();

    if (int32(m_timer) + delaytime > m_casttime)
    {
        delaytime = m_casttime - m_timer;
        m_timer = m_casttime;
    }
    else
        m_timer += delaytime;

    DETAIL_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Spell %u partially interrupted for (%d) ms at damage", m_spellInfo->Id, delaytime);

    WorldPacket data(SMSG_SPELL_DELAYED, 8 + 4);
    data << ObjectGuid(m_caster->GetObjectGuid());
    data << uint32(delaytime);

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_caster)->SendDirectMessage(data);
}

void Spell::DelayedChannel()
{
    if (!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER || getState() != SPELL_STATE_CHANNELING)
        return;

    // check resist chance
    int32 resistChance = 100;                               // must be initialized to 100 for percent modifiers
    ((Player*)m_caster)->ApplySpellMod(m_spellInfo->Id, SPELLMOD_NOT_LOSE_CASTING_TIME, resistChance, this);
    resistChance += m_caster->GetTotalAuraModifier(SPELL_AURA_RESIST_PUSHBACK) - 100;
    if (roll_chance_i(resistChance))
        return;

    int32 delaytime = GetNextDelayAtDamageMsTime();

    if (int32(m_timer) < delaytime)
    {
        delaytime = m_timer;
        m_timer = 0;
    }
    else
        m_timer -= delaytime;

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Spell %u partially interrupted for %i ms, new duration: %u ms", m_spellInfo->Id, delaytime, m_timer);

    for (TargetList::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
    {
        if ((*ihit).missCondition == SPELL_MISS_NONE)
        {
            if (Unit* unit = m_caster->GetObjectGuid() == ihit->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit->targetGUID))
                unit->DelaySpellAuraHolder(m_spellInfo->Id, delaytime, m_caster->GetObjectGuid());
        }
    }

    for (int j = 0; j < MAX_EFFECT_INDEX; ++j)
    {
        // partially interrupt persistent area auras
        if (DynamicObject* dynObj = m_caster->GetDynObject(m_spellInfo->Id, SpellEffectIndex(j)))
            dynObj->Delay(delaytime);
    }

    SendChannelUpdate(m_timer);
}

void Spell::UpdateOriginalCasterPointer()
{
    if (m_originalCasterGUID == m_caster->GetObjectGuid())
        m_originalCaster = m_caster;
    else if (m_originalCasterGUID.IsGameObject())
    {
        GameObject* go = m_caster->IsInWorld() ? m_caster->GetMap()->GetGameObject(m_originalCasterGUID) : nullptr;
        m_originalCaster = go ? go->GetOwner() : nullptr;
    }
    else
    {
        Unit* unit = ObjectAccessor::GetUnit(*m_caster, m_originalCasterGUID);
        m_originalCaster = unit && unit->IsInWorld() ? unit : nullptr;
    }
}

void Spell::UpdatePointers()
{
    UpdateOriginalCasterPointer();

    m_targets.Update(m_caster);

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
        m_CastItem = ((Player*)m_caster)->GetItemByGuid(m_CastItemGuid);
    else
        m_CastItem = nullptr;
}

bool Spell::CheckTargetCreatureType(Unit* target) const
{
    uint32 spellCreatureTargetMask = m_spellInfo->TargetCreatureType;

    // Dismiss Pet and Taming Lesson skipped
    if (m_spellInfo->Id == 2641 || m_spellInfo->Id == 23356)
        spellCreatureTargetMask =  0;

    if (spellCreatureTargetMask)
    {
        uint32 TargetCreatureType = target->GetCreatureTypeMask();

        return !TargetCreatureType || (spellCreatureTargetMask & TargetCreatureType);
    }
    return true;
}

CurrentSpellTypes Spell::GetCurrentContainer() const
{
    if (IsNextMeleeSwingSpell())
        return (CURRENT_MELEE_SPELL);
    if (IsAutoRepeat())
        return (CURRENT_AUTOREPEAT_SPELL);
    if (IsChanneledSpell(m_spellInfo))
        return (CURRENT_CHANNELED_SPELL);
    return (CURRENT_GENERIC_SPELL);
}

bool Spell::CheckTargetGOScript(GameObject* /*target*/, SpellEffectIndex /*eff*/) const
{
    /*switch (m_spellInfo->Id)
    {
        default: break;
    }*/
    return true;
}

bool Spell::CheckTargetScript(Unit* target, SpellEffectIndex eff) const
{
    switch (m_spellInfo->Id)
    {
        case 5246:                                          // Intimidating Shout
            if (eff != EFFECT_INDEX_0 && target == m_targets.getUnitTarget())
                return false;
            break;
        case 9082:                                          // Create Containment Coffer
            if (!target->HasAura(9032))
                return false;
            break;
        // Exceptions for Nefarian class calls
        // as it is not possible to filter out targets based on their class from data in spells template (like spell family masks for example)
        case 23397:                                         // Berserk
            if (target->getClass() != CLASS_WARRIOR)
                return false;
            break;
        case 23398:                                         // Involuntary Transformation
            if (target->getClass() != CLASS_DRUID)
                return false;
            break;
        case 23401:                                         // Corrupted Healing
            if (target->getClass() != CLASS_PRIEST)
                return false;
            break;
        case 23410:                                         // Wild Magic
            if (target->getClass() != CLASS_MAGE)
                return false;
            break;
        case 23414:                                         // Paralyze
            if (target->getClass() != CLASS_ROGUE)
                return false;
            break;
        case 23418:                                         // Siphon Blessing
            if (target->getClass() != CLASS_PALADIN)
                return false;
            break;
        case 23425:                                         // Corrupted Totems
            if (target->getClass() != CLASS_SHAMAN)
                return false;
            break;
        case 23427:                                         // Summon Infernals
            if (target->getClass() != CLASS_WARLOCK)
                return false;
            break;
        case 23436:                                         // Corrupted Weapon
            if (target->getClass() != CLASS_HUNTER)
                return false;
            break;
        case 25676:                                         // Drain Mana
        case 25754:
        case 26457:
        case 26559:
            if (target->GetPowerType() != POWER_MANA)
                return false;
            break;
        case 28062:                                         // Positive Charge
            if (!target->HasAura(29660))                    // Only deal damage if target has Negative Charge
                return false;
            break;
        case 28085:                                         // Negative Charge
            if (!target->HasAura(29659))                    // Only deal damage if target has Positive Charge
                return false;
            break;
        default:
            break;
    }
    return true;
}

bool Spell::CheckTarget(Unit* target, SpellEffectIndex eff, bool targetB, CheckException exception) const
{
    // Check targets for creature type mask and remove not appropriate (skip explicit self target case, maybe need other explicit targets)
    if (exception != EXCEPTION_MAGNET && m_spellInfo->EffectImplicitTargetA[eff] != TARGET_UNIT_CASTER)
    {
        if (!CheckTargetCreatureType(target))
            return false;
    }

    Unit* realCaster = m_originalCaster ? m_originalCaster : m_caster;
    uint32 targetType;
    SpellTargetInfo info;
    if (!targetB)
        targetType = m_spellInfo->EffectImplicitTargetA[eff], info = SpellTargetInfoTable[m_spellInfo->EffectImplicitTargetA[eff]];
    else
        targetType = m_spellInfo->EffectImplicitTargetB[eff], info = SpellTargetInfoTable[m_spellInfo->EffectImplicitTargetB[eff]];
    bool scriptTarget = (info.type == TARGET_TYPE_UNIT && info.filter == TARGET_SCRIPT);

    if (target != realCaster)
    {
        // Check targets for not_selectable unit flag and remove
        // A player can cast spells on his pet (or other controlled unit) though in any state
        if (target->GetMasterGuid() != realCaster->GetObjectGuid() && !scriptTarget)
        {
            // any unattackable target skipped
            if (!m_ignoreUnattackableTarget && target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                return false;

            // unselectable targets skipped in all cases except targets with TARGET_SCRIPT
            if (!m_ignoreUnselectableTarget && target != m_targets.getUnitTarget() &&
                target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
                return false;
        }

        // Check player targets and remove if in GM mode or GM invisibility (for not self casting case)
        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            if (((Player*)target)->GetVisibility() == VISIBILITY_OFF)
                return false;

            if (((Player*)target)->isGameMaster() && !IsPositiveSpell(m_spellInfo->Id, realCaster, target))
                return false;
        }

        if (realCaster->GetTypeId() == TYPEID_PLAYER)
        {
            // Do not allow these spells to target creatures not tapped by us (Banish, Polymorph, many quest spells)
            if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_CANT_TARGET_TAPPED))
                if (Creature const* targetCreature = dynamic_cast<Creature*>(target))
                    if ((!targetCreature->GetLootRecipientGuid().IsEmpty()) && !targetCreature->IsTappedBy((Player*)m_caster))
                        return false;
        }
    }

    if (!scriptTarget)
    {
        // Check targets for LOS visibility (except spells without range limitations )
        switch (m_spellInfo->Effect[eff])
        {
            case SPELL_EFFECT_SUMMON_PLAYER:                    // from anywhere
                break;
            case SPELL_EFFECT_RESURRECT_NEW:
                // player far away, maybe his corpse near?
                if (target != m_caster && !target->IsWithinLOSInMap(m_caster, true))
                {
                    if (!m_targets.getCorpseTargetGuid())
                        return false;

                    Corpse* corpse = m_caster->GetMap()->GetCorpse(m_targets.getCorpseTargetGuid());
                    if (!corpse)
                        return false;

                    if (target->GetObjectGuid() != corpse->GetOwnerGuid())
                        return false;

                    if (!corpse->IsWithinLOSInMap(m_caster))
                        return false;
                }

                // all ok by some way or another, skip normal check
                break;
            default:                                            // normal case
                // Get GO cast coordinates if original caster -> GO
                if (!IsIgnoreLosSpellEffect(m_spellInfo, eff) && target != m_caster)
                    if (WorldObject* caster = GetCastingObject())
                        if (!target->IsWithinLOSInMap(caster, true))
                            return false;
                break;
        }

        if (m_spellInfo->HasAttribute(SPELL_ATTR_EX3_CAST_ON_DEAD) && target->isAlive())
            return false;

        if (!IsAllowingDeadTarget(m_spellInfo) && !target->isAlive() && !(target == m_caster && m_spellInfo->HasAttribute(SPELL_ATTR_CASTABLE_WHILE_DEAD)) && m_caster->GetTypeId() == TYPEID_PLAYER)
            return false;
    }

    if (target->GetTypeId() != TYPEID_PLAYER && m_spellInfo->HasAttribute(SPELL_ATTR_EX3_TARGET_ONLY_PLAYER)
        && targetType != TARGET_UNIT_CASTER)
        return false;

    return CheckTargetScript(target, eff);
}

bool Spell::IsNeedSendToClient() const
{
    return m_spellInfo->SpellVisual != 0 || IsChanneledSpell(m_spellInfo) ||
           m_spellInfo->speed > 0.0f || (!m_triggeredByAuraSpell && !m_IsTriggeredSpell);
}

bool Spell::IsTriggeredSpellWithRedundantCastTime() const
{
    return m_IsTriggeredSpell && (m_spellInfo->manaCost || m_spellInfo->ManaCostPercentage);
}

bool Spell::HaveTargetsForEffect(SpellEffectIndex effect) const
{
    for (const auto& itr : m_UniqueTargetInfo)
        if (itr.effectHitMask & (1 << effect))
            return true;

    for (const auto& itr : m_UniqueGOTargetInfo)
        if (itr.effectMask & (1 << effect))
            return true;

    for (auto itr : m_UniqueItemInfo)
        if (itr.effectMask & (1 << effect))
            return true;

    return false;
}

SpellEvent::SpellEvent(Spell* spell) : BasicEvent()
{
    m_Spell = spell;
}

SpellEvent::~SpellEvent()
{
    if (m_Spell->getState() != SPELL_STATE_FINISHED)
        m_Spell->cancel();

    if (m_Spell->IsDeletable())
    {
        delete m_Spell;
    }
    else
    {
        sLog.outError("~SpellEvent: %s %u tried to delete non-deletable spell %u. Was not deleted, causes memory leak.",
                      (m_Spell->GetCaster()->GetTypeId() == TYPEID_PLAYER ? "Player" : "Creature"), m_Spell->GetCaster()->GetGUIDLow(), m_Spell->m_spellInfo->Id);
    }
}

bool SpellEvent::Execute(uint64 e_time, uint32 p_time)
{
    // update spell if it is not finished
    if (m_Spell->getState() != SPELL_STATE_FINISHED)
        m_Spell->update(p_time);

    // check spell state to process
    switch (m_Spell->getState())
    {
        case SPELL_STATE_FINISHED:
        {
            // spell was finished, check deletable state
            if (m_Spell->IsDeletable())
            {
                // check, if we do have unfinished triggered spells
                return true;                                // spell is deletable, finish event
            }
            // event will be re-added automatically at the end of routine)
        } break;

        case SPELL_STATE_CHANNELING:
        {
            // this spell is in channeled state, process it on the next update
            // event will be re-added automatically at the end of routine)
        } break;

        case SPELL_STATE_TRAVELING:
        {
            // first, check, if we have just started
            if (m_Spell->GetDelayStart() != 0)
            {
                // no, we aren't, do the typical update
                // check, if we have channeled spell on our hands
                if (IsChanneledSpell(m_Spell->m_spellInfo))
                {
                    // evented channeled spell is processed separately, casted once after delay, and not destroyed till finish
                    // check, if we have casting anything else except this channeled spell and autorepeat
                    if (m_Spell->GetCaster()->IsNonMeleeSpellCasted(false, true, true))
                    {
                        // another non-melee non-delayed spell is casted now, abort
                        m_Spell->cancel();
                    }
                    else
                    {
                        // do the action (pass spell to channeling state)
                        m_Spell->handle_immediate();
                    }
                    // event will be re-added automatically at the end of routine)
                }
                else
                {
                    // run the spell handler and think about what we can do next
                    uint64 t_offset = e_time - m_Spell->GetDelayStart();
                    uint64 n_offset = m_Spell->handle_delayed(t_offset);
                    if (n_offset)
                    {
                        // re-add us to the queue
                        m_Spell->GetCaster()->m_events.AddEvent(this, m_Spell->GetDelayStart() + n_offset, false);
                        return false;                       // event not complete
                    }
                    // event complete
                    // finish update event will be re-added automatically at the end of routine)
                }
            }
            else
            {
                // delaying had just started, record the moment
                m_Spell->SetDelayStart(e_time);
                // re-plan the event for the delay moment
                m_Spell->GetCaster()->m_events.AddEvent(this, e_time + m_Spell->GetDelayMoment(), false);
                return false;                               // event not complete
            }
        } break;

        default:
        {
            // all other states
            // event will be re-added automatically at the end of routine)
        } break;
    }

    // spell processing not complete, plan event on the next update interval
    m_Spell->GetCaster()->m_events.AddEvent(this, e_time + 1, false);
    return false;                                           // event not complete
}

void SpellEvent::Abort(uint64 /*e_time*/)
{
    // oops, the spell we try to do is aborted
    if (m_Spell->getState() != SPELL_STATE_FINISHED)
        m_Spell->cancel();
}

bool SpellEvent::IsDeletable() const
{
    return m_Spell->IsDeletable();
}

SpellCastResult Spell::CanOpenLock(SpellEffectIndex effIndex, uint32 lockId, SkillType& skillId, int32& reqSkillValue, int32& skillValue)
{
    if (!lockId)                                            // possible case for GO and maybe for items.
        return SPELL_CAST_OK;

    // Get LockInfo
    LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);

    if (!lockInfo)
        return SPELL_FAILED_BAD_TARGETS;

    bool reqKey = false;                                    // some locks not have reqs

    for (int j = 0; j < 8; ++j)
    {
        switch (lockInfo->Type[j])
        {
            // check key item (many fit cases can be)
            case LOCK_KEY_ITEM:
                if (lockInfo->Index[j] && m_CastItem && m_CastItem->GetEntry() == lockInfo->Index[j])
                    return SPELL_CAST_OK;
                reqKey = true;
                break;
            // check key skill (only single first fit case can be)
            case LOCK_KEY_SKILL:
            {
                reqKey = true;

                // wrong locktype, skip
                if (uint32(m_spellInfo->EffectMiscValue[effIndex]) != lockInfo->Index[j])
                    continue;

                skillId = SkillByLockType(LockType(lockInfo->Index[j]));

                if (skillId != SKILL_NONE)
                {
                    // skill bonus provided by casting spell (mostly item spells)
                    // add the damage modifier from the spell casted (cheat lock / skeleton key etc.) (use m_currentBasePoints, CalculateDamage returns wrong value)
                    uint32 spellSkillBonus = uint32(m_currentBasePoints[effIndex]);
                    reqSkillValue = lockInfo->Skill[j];

                    // castitem check: rogue using skeleton keys. the skill values should not be added in this case.
                    skillValue = m_CastItem || m_caster->GetTypeId() != TYPEID_PLAYER ?
                                 0 : ((Player*)m_caster)->GetSkillValue(skillId);

                    skillValue += spellSkillBonus;

                    if (skillValue < reqSkillValue)
                        return SPELL_FAILED_LOW_CASTLEVEL;
                }

                return SPELL_CAST_OK;
            }
        }
    }

    if (reqKey)
        return SPELL_FAILED_BAD_TARGETS;

    return SPELL_CAST_OK;
}

/**
 * Fill target list by units around (x,y) points at radius distance

 * @param targetUnitMap        Reference to target list that filled by function
 * @param x                    X coordinates of center point for target search
 * @param y                    Y coordinates of center point for target search
 * @param radius               Radius around (x,y) for target search
 * @param pushType             Additional rules for target area selection (in front, angle, etc)
 * @param spellTargets         Additional rules for target selection base at hostile/friendly state to original spell caster
 * @param originalCaster       If provided set alternative original caster, if =nullptr then used Spell::GetAffectiveObject() return
 */
void Spell::FillAreaTargets(UnitList& targetUnitMap, float radius, float cone, SpellNotifyPushType pushType, SpellTargets spellTargets, WorldObject* originalCaster /*=nullptr*/)
{
    MaNGOS::SpellNotifierCreatureAndPlayer notifier(*this, targetUnitMap, radius, cone, pushType, spellTargets, originalCaster);
    Cell::VisitAllObjects(notifier.GetCenterX(), notifier.GetCenterY(), m_caster->GetMap(), notifier, radius);
}

void Spell::FillRaidOrPartyTargets(UnitList& targetUnitMap, Unit* member, float radius, bool raid, bool withPets, bool withcaster) const
{
    Player* pMember = member->GetBeneficiaryPlayer();
    Group* pGroup = pMember ? pMember->GetGroup() : nullptr;

    if (pGroup)
    {
        uint8 subgroup = pMember->GetSubGroup();

        for (GroupReference* itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* Target = itr->getSource();

            // CanAssist check duel and controlled by enemy
            if (Target && (raid || subgroup == Target->GetSubGroup())
                    && m_caster->CanAssistSpell(Target, m_spellInfo))
            {
                if (((Target == m_caster) && withcaster) ||
                        ((Target != m_caster) && m_caster->IsWithinDistInMap(Target, radius)))
                    targetUnitMap.push_back(Target);

                if (withPets)
                    if (Pet* pet = Target->GetPet())
                        if (((pet == m_caster) && withcaster) ||
                                ((pet != m_caster) && m_caster->IsWithinDistInMap(pet, radius)))
                            targetUnitMap.push_back(pet);
            }
        }
    }
    else
    {
        Unit* ownerOrSelf = pMember ? pMember : member->GetBeneficiary();
        if (((ownerOrSelf == m_caster) && withcaster) ||
                ((ownerOrSelf != m_caster) && m_caster->IsWithinDistInMap(ownerOrSelf, radius)))
            targetUnitMap.push_back(ownerOrSelf);

        if (withPets)
            if (Pet* pet = ownerOrSelf->GetPet())
                if (((pet == m_caster) && withcaster) ||
                        ((pet != m_caster) && m_caster->IsWithinDistInMap(pet, radius)))
                    targetUnitMap.push_back(pet);
    }
}

WorldObject* Spell::GetAffectiveCasterObject() const
{
    if (!m_originalCasterGUID)
        return m_caster;

    if (m_originalCasterGUID.IsGameObject() && m_caster->IsInWorld())
        return m_caster->GetMap()->GetGameObject(m_originalCasterGUID);
    return m_originalCaster;
}

WorldObject* Spell::GetCastingObject() const
{
    if (m_originalCasterGUID.IsGameObject())
        return m_caster->IsInWorld() ? m_caster->GetMap()->GetGameObject(m_originalCasterGUID) : nullptr;
    else if (m_originalCasterGUID.IsDynamicObject())
        return m_caster->IsInWorld() ? m_caster->GetMap()->GetDynamicObject(m_originalCasterGUID) : nullptr;

    return m_caster;
}

float Spell::GetSpellSpeed() const
{
    if (IsChanneledSpell(m_spellInfo)) return 0.f; return m_spellInfo->speed;
}

void Spell::ResetEffectDamageAndHeal()
{
    m_damage = 0;
    m_healing = 0;
}

// handle SPELL_AURA_ADD_TARGET_TRIGGER auras
void Spell::ProcSpellAuraTriggers()
{
    Unit::AuraList const& targetTriggers = m_caster->GetAurasByType(SPELL_AURA_ADD_TARGET_TRIGGER);
    for (auto targetTrigger : targetTriggers)
    {
        if (!targetTrigger->isAffectedOnSpell(m_spellInfo))
            continue;
        for (TargetList::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
        {
            if (ihit->missCondition == SPELL_MISS_NONE)
            {
                // check m_caster->GetGUID() let load auras at login and speedup most often case
                Unit* unit = m_caster->GetObjectGuid() == ihit->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit->targetGUID);
                if (unit && unit->isAlive())
                {
                    SpellEntry const* auraSpellInfo = targetTrigger->GetSpellProto();
                    SpellEffectIndex auraSpellIdx = targetTrigger->GetEffIndex();
                    const uint32 procid = auraSpellInfo->EffectTriggerSpell[auraSpellIdx];
                    // Quick target mode check for procs and triggers (do not cast at friendly targets stuff against hostiles only)
                    if (IsPositiveSpellTargetModeForSpecificTarget(m_spellInfo, ihit->effectMask, m_caster, unit) != IsPositiveSpellTargetModeForSpecificTarget(procid, ihit->effectMask, m_caster, unit))
                        continue;
                    // Calculate chance at that moment (can be depend for example from combo points)
                    int32 auraBasePoints = targetTrigger->GetBasePoints();
                    int32 chance = m_caster->CalculateSpellDamage(unit, auraSpellInfo, auraSpellIdx, &auraBasePoints);
                    if (roll_chance_i(chance))
                        m_caster->CastSpell(unit, procid, TRIGGERED_OLD_TRIGGERED, nullptr, targetTrigger);
                }
            }
        }
    }
}

void Spell::ClearCastItem()
{
    if (m_CastItem == m_targets.getItemTarget())
        m_targets.setItemTarget(nullptr);

    m_CastItem = nullptr;
    m_CastItemGuid.Clear();
}

void Spell::GetSpellRangeAndRadius(SpellEffectIndex effIndex, float& radius, bool targetB, uint32& EffectChainTarget) const
{
    if (m_spellInfo->EffectRadiusIndex[effIndex])
        radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[effIndex]));
    else
        radius = GetSpellMaxRange(sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex));

    if (radius == 50000.f) // safety against bad data
    {
        uint32 targetMode;
        if (!targetB)
            targetMode = m_spellInfo->EffectImplicitTargetA[effIndex];
        else
            targetMode = m_spellInfo->EffectImplicitTargetB[effIndex];
        SpellTargetInfo& data = SpellTargetInfoTable[targetMode];
        switch (data.type)
        {
            case TARGET_TYPE_LOCATION_DEST:
                if (radius == 50000.f)
                    radius = 3.0f;
                break;
            case TARGET_TYPE_UNIT:
            case TARGET_TYPE_GAMEOBJECT:
                if (radius == 50000.f)
                    if (data.enumerator == TARGET_ENUMERATOR_CHAIN || data.enumerator == TARGET_ENUMERATOR_AOE || data.enumerator == TARGET_ENUMERATOR_CONE)
                        radius = 200.f;
                break;
        }
    }

    if (Unit* realCaster = GetAffectiveCaster())
    {
        if (Player* modOwner = realCaster->GetSpellModOwner())
        {
            modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_RADIUS, radius, this);
            modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_JUMP_TARGETS, EffectChainTarget, this);
        }
    }

    // AOE leeway - 2f when caster is moving
    if (m_caster->IsMoving() && !m_caster->IsWalking())
        radius += AOE_LEEWAY;

    // custom radius cases
    switch (m_spellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch (m_spellInfo->Id)
            {
                case 28241:                                 // Poison (Naxxramas, Grobbulus Cloud)
                {
                    if (SpellAuraHolder* auraHolder = m_caster->GetSpellAuraHolder(28158))
                        radius = 0.5f * (60000 - auraHolder->GetAuraDuration()) * 0.001f;
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}

float Spell::GetCone()
{
    if (SpellCone const* coneData = sSpellCones.LookupEntry<SpellCone>(sSpellMgr.GetFirstSpellInChain(m_spellInfo->Id)))
        return G3D::toRadians(coneData->coneAngle);

    return M_PI_F / 3.f; // 60 degrees is default
}

void Spell::FilterTargetMap(UnitList& /*filterUnitList*/, SpellEffectIndex /*effIndex*/)
{

}

bool Spell::IsEffectWithImplementedMultiplier(uint32 effectId) const
{
    // TODO: extend this for all effects that do damage and healing
    switch (effectId)
    {
        case SPELL_EFFECT_SCHOOL_DAMAGE:
        case SPELL_EFFECT_HEAL:
            return true;
        default:
            return false;
    }
}

void Spell::OnSuccessfulSpellStart()
{
    switch (m_spellInfo->Id)
    {
        case 5308: // Warrior - Execute
        case 20658:
        case 20660:
        case 20661:
        case 20662:
        {
            int32 basePoints0 = m_caster->CalculateSpellDamage(m_targets.getUnitTarget(), m_spellInfo, SpellEffectIndex(0)) + int32(m_caster->GetPower(POWER_RAGE) * m_spellInfo->DmgMultiplier[0]);
            SpellCastResult result = m_caster->CastCustomSpell(m_targets.getUnitTarget(), 20647, &basePoints0, nullptr, nullptr, TRIGGERED_NONE, nullptr);
            m_powerCost = 0;
            break;
        }
    }
}

void Spell::OnSuccessfulSpellFinish()
{
    switch (m_spellInfo->Id)
    {
        case 32053: // Soul Charge - drop either stack or charge
        case 32054:
        case 32057:
        {
            uint32 parentSpell;
            switch (m_spellInfo->Id)
            {
                case 32053: parentSpell = 32052; break;
                case 32054: parentSpell = 32045; break;
                case 32057: parentSpell = 32051; break;
            }
            m_caster->RemoveAuraStack(parentSpell);
            break;
        }
    }
}

SpellCastResult Spell::OnCheckCast(bool /*strict*/)
{
    switch (m_spellInfo->Id)
    {
        case 603: // Curse of Doom
        {
            // not allow cast at player
            Unit* target = m_targets.getUnitTarget();
            if (!target || target->GetTypeId() == TYPEID_PLAYER)
                return SPELL_FAILED_BAD_TARGETS;
            break;
        }
        case 27230: // Health Stone
        case 11730:
        case 11729:
        case 6202:
        case 6201:
        case 5699:
        {
            // check if we already have a healthstone
            uint32 itemType = GetUsableHealthStoneItemType(m_caster);
            if (itemType && m_caster->IsPlayer() && ((Player*)m_caster)->GetItemCount(itemType) > 0)
                return SPELL_FAILED_TOO_MANY_OF_ITEM;
            break;
        }
    }

    return SPELL_CAST_OK;
}

void Spell::StopCast(SpellCastResult castResult)
{
    SendCastResult(castResult);
    SendInterrupted(castResult);
    finish(false);
    m_caster->DecreaseCastCounter();
    SetExecutedCurrently(false);
}
