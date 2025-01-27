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
#include "Server/WorldPacket.h"
#include "Server/WorldSession.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Server/Opcodes.h"
#include "Log/Log.h"
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
#include "Util/Util.h"
#include "Chat/Chat.h"
#include "Server/SQLStorages.h"
#include "MotionGenerators/PathFinder.h"
#include "Spells/Scripts/SpellScript.h"
#include "Entities/ObjectGuid.h"

#ifdef ENABLE_PLAYERBOTS
#include "playerbot/PlayerbotAI.h"
#endif

extern pEffect SpellEffects[MAX_SPELL_EFFECTS];

class PrioritizeManaUnitWraper
{
    public:
    explicit PrioritizeManaUnitWraper(Unit* unit) : i_unit(unit)
    {
        uint32 maxmana = unit->GetMaxPower(POWER_MANA);
        i_percent = maxmana ? unit->GetPower(POWER_MANA) * 100 / maxmana : 101;
    }
    Unit* getUnit() const { return i_unit; }
    uint32 getPercent() const { return i_percent; }
    private:
    Unit* i_unit;
    uint32 i_percent;
};

struct PrioritizeMana
{
    int operator()(PrioritizeManaUnitWraper const& x, PrioritizeManaUnitWraper const& y) const
    {
        return x.getPercent() > y.getPercent();
    }
};

typedef std::priority_queue<PrioritizeManaUnitWraper, std::vector<PrioritizeManaUnitWraper>, PrioritizeMana> PrioritizeManaUnitQueue;

class PrioritizeHealthUnitWraper
{
    public:
    explicit PrioritizeHealthUnitWraper(Unit* unit) : i_unit(unit)
    {
        i_percent = unit->GetHealthPercent();
    }
    Unit* getUnit() const { return i_unit; }
    uint32 getPercent() const { return i_percent; }
    private:
    Unit* i_unit;
    uint32 i_percent;
};

struct PrioritizeHealth
{
    int operator()(PrioritizeHealthUnitWraper const& x, PrioritizeHealthUnitWraper const& y) const
    {
        return x.getPercent() > y.getPercent();
    }
};

typedef std::priority_queue<PrioritizeHealthUnitWraper, std::vector<PrioritizeHealthUnitWraper>, PrioritizeHealth> PrioritizeHealthUnitQueue;

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
    m_CorpseTarget = nullptr;

    m_itemTargetEntry  = 0;

    m_strTarget.clear();
    m_targetMask = 0;

    m_mapId = UINT32_MAX;
}

SpellCastTargets::~SpellCastTargets()
{
}

void SpellCastTargets::setUnitTarget(Unit* target)
{
    if (!target)
        return;

    m_destPos = target->GetPosition();
    m_unitTarget = target;
    m_unitTargetGUID = target->GetObjectGuid();
    m_targetMask |= TARGET_FLAG_UNIT;
}

void SpellCastTargets::setDestination(float x, float y, float z)
{
    m_destPos.x = x;
    m_destPos.y = y;
    m_destPos.z = z;
    m_targetMask |= TARGET_FLAG_DEST_LOCATION;
}

void SpellCastTargets::setDestination(Position position)
{
    m_destPos = position;
    m_targetMask |= TARGET_FLAG_DEST_LOCATION;
}

void SpellCastTargets::setSource(float x, float y, float z)
{
    m_srcPos.x = x;
    m_srcPos.y = y;
    m_srcPos.z = z;
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

void SpellCastTargets::clearItemPointer()
{
    // keep the other information so its clear how the spell was targeted
    m_itemTarget = nullptr;
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

void SpellCastTargets::Update(WorldObject* caster)
{
    m_GOTarget   = m_GOTargetGUID ? caster->GetMap()->GetGameObject(m_GOTargetGUID) : nullptr;
    m_unitTarget = m_unitTargetGUID ?
                   (m_unitTargetGUID == caster->GetObjectGuid() ? static_cast<Unit*>(caster) : ObjectAccessor::GetUnit(*caster, m_unitTargetGUID)) :
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
        m_destPos.x = caster->GetPositionX();
        m_destPos.y = caster->GetPositionY();
        m_destPos.z = caster->GetPositionZ();
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
        data >> m_srcPos.x >> m_srcPos.y >> m_srcPos.z;
        if (!MaNGOS::IsValidMapCoord(m_srcPos.x, m_srcPos.y, m_srcPos.z))
            throw ByteBufferException(false, data.rpos(), 0, data.size());
    }

    if (m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        data >> m_destPos.x >> m_destPos.y >> m_destPos.z;
        if (!MaNGOS::IsValidMapCoord(m_destPos.x, m_destPos.y, m_destPos.z))
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
        data << m_srcPos.x << m_srcPos.y << m_srcPos.z;

    if (m_targetMask & TARGET_FLAG_DEST_LOCATION)
        data << m_destPos.x << m_destPos.y << m_destPos.z;

    if (m_targetMask & TARGET_FLAG_STRING)
        data << m_strTarget;
}

// SpellLog class
// **************

void SpellLog::Initialize()
{
    m_spellLogData.Initialize(SMSG_SPELLLOGEXECUTE);
    m_spellLogData << m_spell->GetTrueCaster()->GetPackGUID();
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
    m_spell->GetTrueCaster()->SendMessageToSet(m_spellLogData, true);

    // make it ready for another log if need
    Initialize();
}

// Spell class
// ***********

Spell::Spell(WorldObject* caster, SpellEntry const* info, uint32 triggeredFlags, ObjectGuid originalCasterGUID, SpellEntry const* triggeredBy) :
    m_partialApplicationMask(0), m_spellScript(SpellScriptMgr::GetSpellScript(info->Id)), m_auraScript(SpellScriptMgr::GetAuraScript(info->Id)),
    m_effectSkipMask(0),
    m_spellEvent(nullptr), m_spellLog(this), m_param1(0), m_param2(0), m_trueCaster(caster)
{
    MANGOS_ASSERT(caster != nullptr && info != nullptr);
    MANGOS_ASSERT(info == sSpellTemplate.LookupEntry<SpellEntry>(info->Id) && "`info` must be pointer to sSpellTemplate element");

    m_spellInfo = info;
    m_triggeredBySpellInfo = triggeredBy;
    m_caster = dynamic_cast<Unit*>(caster);
    m_referencedFromCurrentSpell = false;
    m_executedCurrently = false;
    m_delayStart = 0;
    m_delayAtDamageCount = 0;

    m_applyMultiplierMask = 0;
    for (float& value : m_damageDoneMultiplier)
        value = 1.f;

    // Get data for type of attack
    m_attackType = GetWeaponAttackType(m_spellInfo);

    m_spellSchoolMask = GetSpellSchoolMask(info);           // Can be override for some spell (wand shoot for example)

    // wand case
    if (m_attackType == RANGED_ATTACK && m_caster)
        if (!!(m_caster->getClassMask() & CLASSMASK_WAND_USERS) && m_caster->GetTypeId() == TYPEID_PLAYER)
            m_spellSchoolMask = m_caster->GetRangedDamageSchoolMask();

    // Set health leech amount to zero
    m_healthLeech = 0;

    m_originalCasterGUID = originalCasterGUID ? originalCasterGUID : m_trueCaster->GetObjectGuid();

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
    m_itemCastSpell = false;

    unitTarget = nullptr;
    itemTarget = nullptr;
    gameObjTarget = nullptr;
    corpseTarget = nullptr;
    m_eventTarget = nullptr;
    m_triggeredByAuraSpell  = nullptr;
    m_spellAuraHolder = nullptr;

    // Auto Shot & Shoot (wand)
    m_autoRepeat = IsAutoRepeatRangedSpell(m_spellInfo);

    m_powerCost = 0;                                        // setup to correct value in Spell::prepare, don't must be used before.
    m_casttime = 0;                                         // setup to correct value in Spell::prepare, don't must be used before.
    m_timer = 0;                                            // will set to cast time in prepare
    m_creationTime = m_trueCaster->GetMap()->GetCurrentMSTime();
    m_updated = false;
    m_duration = 0;
    m_maxRange = 0.f;

    m_needAliveTargetMask = 0;

    m_ignoreHitResult = (triggeredFlags & TRIGGERED_IGNORE_HIT_CALCULATION) != 0;
    m_ignoreUnselectableTarget = m_IsTriggeredSpell || ((triggeredFlags & TRIGGERED_IGNORE_UNSELECTABLE_FLAG) != 0);
    m_ignoreCastTime = (triggeredFlags & TRIGGERED_INSTANT_CAST) != 0;
    m_ignoreUnattackableTarget = (triggeredFlags & TRIGGERED_IGNORE_UNATTACKABLE_FLAG) != 0;
    m_triggerAutorepeat = (triggeredFlags & TRIGGERED_AUTOREPEAT) != 0;
    m_doNotProc = (triggeredFlags & TRIGGERED_DO_NOT_PROC) != 0;
    m_petCast = (triggeredFlags & TRIGGERED_PET_CAST) != 0;
    m_notifyAI = (triggeredFlags & TRIGGERED_NORMAL_COMBAT_CAST) != 0;
    m_ignoreGCD = m_IsTriggeredSpell || ((triggeredFlags & TRIGGERED_IGNORE_GCD) != 0);
    m_ignoreCosts = (m_IsTriggeredSpell && (triggeredFlags & TRIGGERED_FORCE_COSTS) == 0) || ((triggeredFlags & TRIGGERED_IGNORE_COSTS) != 0);
    m_ignoreCooldowns = m_IsTriggeredSpell || ((triggeredFlags & TRIGGERED_IGNORE_COOLDOWNS) != 0);
    m_ignoreConcurrentCasts = m_IsTriggeredSpell || ((triggeredFlags & TRIGGERED_IGNORE_CURRENT_CASTED_SPELL) != 0) || m_spellInfo->HasAttribute(SPELL_ATTR_EX4_ALLOW_CAST_WHILE_CASTING);
    m_ignoreCasterAuraState = m_IsTriggeredSpell || ((triggeredFlags & TRIGGERED_IGNORE_CASTER_AURA_STATE) != 0);
    m_hideInCombatLog = (m_IsTriggeredSpell && !IsAutoRepeatRangedSpell(m_spellInfo)) || ((triggeredFlags & TRIGGERED_HIDE_CAST_IN_COMBAT_LOG) != 0);
    m_resetLeash = (triggeredFlags & TRIGGERED_DO_NOT_RESET_LEASH) == 0;
    m_channelOnly = (triggeredFlags & TRIGGERED_CHANNEL_ONLY) != 0;

    m_clientCast = false;

    m_reflectable = IsReflectableSpell(m_spellInfo);

    m_affectedTargetCount = m_spellInfo->MaxAffectedTargets;

    m_jumpRadius = SpellTargetMgr::GetJumpRadius(m_spellInfo->Id);
    memcpy(m_filteringScheme, SpellTargetMgr::GetSpellTargetingData(m_spellInfo->Id).filteringScheme, sizeof(m_filteringScheme));

    m_scriptValue = 0;

    memset(m_effectTriggerChance, -1, sizeof(m_effectTriggerChance));
    memset(damagePerEffect, 0, sizeof(damagePerEffect));

    CleanupTargetList();

    m_spellLog.Initialize();
    m_needSpellLog = (m_spellInfo->Attributes & (SPELL_ATTR_DO_NOT_LOG | SPELL_ATTR_DO_NOT_DISPLAY)) == 0;

    m_travellingStart = UINT32_MAX;

    m_targetlessMask = 0;

    m_overrideSpeed = false;

    m_ignoreRoot = IsIgnoreRootSpell(m_spellInfo);

    OnInit();
}

Spell::~Spell()
{
    if (!m_IsTriggeredSpell && m_CastItem)
        m_CastItem->SetUsedInSpell(false);
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
    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        // not call for empty effect.
        // Also some spells use not used effect targets for store targets for dummy effect in triggered spells
        if (m_spellInfo->Effect[i] == SPELL_EFFECT_NONE)
            continue;

        auto& data = SpellTargetMgr::GetSpellTargetingData(m_spellInfo->Id);
        SpellTargetImplicitType effectTargetType = data.implicitType[i]; // prefilled data on load
        auto& targetMask = data.targetMask[i];
        auto& ignoredTargets = data.ignoredTargets[i];
        auto& filterScheme = m_filteringScheme[i];
        targetingData.chainTargetCount[i] = m_spellInfo->EffectChainTarget[SpellEffectIndex(i)];

        if (effectTargetType == TARGET_TYPE_SPECIAL_UNIT) // area auras need custom handling
        {
            uint32 targetA = m_spellInfo->EffectImplicitTargetA[i];
            uint32 targetB = m_spellInfo->EffectImplicitTargetB[i];
            bool hadTarget = false;
            // need to pick a single unit target if existant and use it for area aura owner
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
            if (!hadTarget && m_caster) // if no single targeting available use caster as default
                targetingData.data[i].tmpUnitList[false].push_back(m_caster);
        }
        else
        {
            uint32 targetA = m_spellInfo->EffectImplicitTargetA[i];
            uint32 targetB = m_spellInfo->EffectImplicitTargetB[i];
            if (targetA == TARGET_NONE && targetB == TARGET_NONE)
            {
                // if no targeting available, attempt to use entry mask
                if (m_spellInfo->Targets && SpellTargetMgr::CanEffectBeFilledWithMask(m_spellInfo->Id, i, m_spellInfo->Targets))
                    FillFromTargetFlags(targetingData, SpellEffectIndex(i));
                else if (uint32 defaultTarget = SpellEffectInfoTable[m_spellInfo->Effect[i]].defaultTarget) // else resort to default effect type if it exists
                    SetTargetMap(SpellEffectIndex(i), defaultTarget, false, targetingData);
            }
            else // normal case, use existing spell data
            {
                if (targetA && !ignoredTargets.first)
                    SetTargetMap(SpellEffectIndex(i), targetA, false, targetingData);
                if (targetB && !ignoredTargets.second)
                    SetTargetMap(SpellEffectIndex(i), targetB, true, targetingData);
                if (effectTargetType == TARGET_TYPE_UNIT_DEST) // special case - no unit target, but need to check for valid units
                    if (SpellTargetInfoTable[targetA].type != TARGET_TYPE_UNIT && SpellTargetInfoTable[targetB].type != TARGET_TYPE_UNIT) // no fill for unit out of targets
                        FillFromTargetFlags(targetingData, SpellEffectIndex(i)); // inefficient call, very rare, less code duplicity
            }
        }

        bool processedUnits = false; // TODO: Review area aura packet fills for area auras with AOE targets
        bool processedGOs = false; // likely purpose - animations
        switch (effectTargetType)
        {
            case TARGET_TYPE_NONE:
                m_targetlessMask |= (1 << i);
                break;
            case TARGET_TYPE_LOCATION_DEST:
            case TARGET_TYPE_SPECIAL_DEST:
                OnDestTarget();
                AddDestExecution(SpellEffectIndex(i));
                if (effectTargetType == TARGET_TYPE_SPECIAL_DEST)
                    if (!FillUnitTargets(targetingData, data, i))
                        return;
                break;
            case TARGET_TYPE_CORPSE: // can be unit and corpse
                if (!targetingData.data[i].tempCorpseList.empty())
                {
                    CorpseList& corpseTargetList = targetingData.data[i].tempCorpseList;
                    uint8 effectMask = targetMask[0] | targetMask[1];
                    for (Corpse* corpse : corpseTargetList)
                        AddCorpseTarget(corpse, effectMask);
                }
            case TARGET_TYPE_UNIT:
            case TARGET_TYPE_UNIT_DEST:
            case TARGET_TYPE_PLAYER: // for now player handled here
            case TARGET_TYPE_SPECIAL_UNIT:
                if (effectTargetType == TARGET_TYPE_UNIT_DEST)
                    OnDestTarget();
                processedUnits = true;
                if (!FillUnitTargets(targetingData, data, i))
                    return;
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
                for (uint8 rightTarget = 0; rightTarget < 2; ++rightTarget) // need to process target A and B separately due to effect masks
                {
                    bool ignored = rightTarget ? ignoredTargets.second : ignoredTargets.first;
                    if (ignored)
                        continue;
                    GameObjectList& goTargetList = targetingData.data[i].tmpGOList[rightTarget];
                    uint8 effectMask = targetMask[rightTarget];
                    if (!goTargetList.empty()) // GO case
                    {
                        for (auto iter = goTargetList.begin(); iter != goTargetList.end();)
                        {
                            if (OnCheckTarget(*iter, SpellEffectIndex(i)))
                                ++iter;
                            else
                                iter = goTargetList.erase(iter);
                        }
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
                        for (GameObject* go : goTargetList)
                            AddGOTarget(go, effectMask);
                    }
                }
                break;
            default:
                break;
        }
    }

    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_FAIL_ON_ALL_TARGETS_IMMUNE))
    {
        bool allImmune = true;
        for (auto& ihit : m_UniqueTargetInfo)
        {
            if (ihit.missCondition != SPELL_MISS_IMMUNE && ihit.missCondition != SPELL_MISS_IMMUNE2)
            {
                allImmune = false;
                break;
            }
        }

        if (allImmune)
        {
            SendCastResult(SPELL_FAILED_IMMUNE); // guessed error
            finish(false);
            return;
        }
    }

    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_REQUIRE_ALL_TARGETS))
    {
        for (auto& ihit : m_UniqueTargetInfo)
        {
            if (ihit.targetGUID == m_targets.getUnitTargetGuid() && ihit.missCondition != SPELL_MISS_NONE)
            {
                for (auto& ihit : m_UniqueTargetInfo)
                {
                    ihit.effectHitMask = 0;
                    ihit.missCondition = SPELL_MISS_IMMUNE2;
                    ihit.effectDuration = 0;
                }
                m_duration = 0;
            }
        }
    }
}

bool Spell::FillUnitTargets(TempTargetingData& targetingData, SpellTargetingData& data, uint32 i)
{
    auto& targetMask = data.targetMask[i];
    auto& ignoredTargets = data.ignoredTargets[i];
    auto& filterScheme = m_filteringScheme[i];
    for (uint8 rightTarget = 0; rightTarget < 2; ++rightTarget) // need to process target A and B separately due to effect masks
    {
        bool ignored = rightTarget ? ignoredTargets.second : ignoredTargets.first;
        if (ignored)
            continue;
        UnitList& unitTargetList = targetingData.data[i].tmpUnitList[rightTarget];
        uint8 effectMask = targetMask[rightTarget];
        SpellTargetFilterScheme scheme = filterScheme[rightTarget];
        uint32 target = rightTarget ? m_spellInfo->EffectImplicitTargetB[i] : m_spellInfo->EffectImplicitTargetA[i];
        SpellTargetImplicitType type = SpellTargetInfoTable[target].type;
        if (!unitTargetList.empty()) // Unit case
        {
            for (auto itr = unitTargetList.begin(); itr != unitTargetList.end();)
            {
                if (!CheckTarget(*itr, SpellEffectIndex(i), bool(rightTarget), CheckException(targetingData.magnet)))
                    itr = unitTargetList.erase(itr);
                else
                    ++itr;
            }

            // Special target filter before adding targets to list
            FilterTargetMap(unitTargetList, scheme, targetingData.chainTargetCount[i]);

            if (m_trueCaster->IsPlayer())
            {
                Player* me = static_cast<Player*>(m_trueCaster);
                for (auto itr = unitTargetList.begin(); itr != unitTargetList.end(); ++itr)
                {
                    Player* targetOwner = (*itr)->GetBeneficiaryPlayer();
                    if (targetOwner && targetOwner != me && targetOwner->IsPvP() && !me->IsInDuelWith(targetOwner))
                    {
                        me->UpdatePvP(true);
                        me->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_PVP_ACTIVE_CANCELS);
                        break;
                    }
                }
            }
        }

        if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_REQUIRE_ALL_TARGETS) &&
            (type == TARGET_TYPE_UNIT || type == TARGET_TYPE_PLAYER))
        {
            // spells which should only be cast if a target was found
            if (unitTargetList.size() <= 0)
            {
                SendCastResult(SPELL_FAILED_BAD_TARGETS);
                finish(false);
                return false;
            }
        }

        for (Unit* unit : unitTargetList)
            AddUnitTarget(unit, effectMask);
    }

    return true;
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
    m_canTrigger = true;

    if ((m_CastItem && m_spellInfo->SpellFamilyFlags == nullptr) || m_doNotProc)
        m_canTrigger = false;                               // Do not trigger from item cast spell

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
            procAttacker = PROC_FLAG_DEAL_MELEE_ABILITY;
            if (m_attackType == BASE_ATTACK)
                procAttacker |= PROC_FLAG_MAIN_HAND_WEAPON_SWING;
            if (m_attackType == OFF_ATTACK)
                procAttacker |= PROC_FLAG_OFF_HAND_WEAPON_SWING;
            procVictim = PROC_FLAG_TAKE_MELEE_ABILITY;
            break;
        case SPELL_DAMAGE_CLASS_RANGED:
            // Auto attack
            if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_AUTO_REPEAT))
            {
                procAttacker = PROC_FLAG_DEAL_RANGED_ATTACK;
                procVictim = PROC_FLAG_TAKE_RANGED_ATTACK;
            }
            else // Ranged spell attack
            {
                procAttacker = PROC_FLAG_DEAL_RANGED_ABILITY;
                procVictim = PROC_FLAG_TAKE_RANGED_ABILITY;
            }
            break;
        default:
            if (IsPositiveEffectMask(m_spellInfo, effectMask, caster, target))           // Check for positive spell
            {
                if (m_spellInfo->HasAttribute(SPELL_ATTR_IS_ABILITY))
                {
                    procAttacker = PROC_FLAG_DEAL_HELPFUL_ABILITY;
                    procVictim = PROC_FLAG_TAKE_HELPFUL_ABILITY;
                }
                else
                {
                    procAttacker = PROC_FLAG_DEAL_HELPFUL_SPELL;
                    procVictim = PROC_FLAG_TAKE_HELPFUL_SPELL;
                }
            }
            else if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_AUTO_REPEAT))   // Wands auto attack
            {
                procAttacker = PROC_FLAG_DEAL_RANGED_ATTACK;
                procVictim = PROC_FLAG_TAKE_RANGED_ATTACK;
            }
            else                                           // Negative spell
            {
                if (m_spellInfo->HasAttribute(SPELL_ATTR_IS_ABILITY))
                {
                    procAttacker = PROC_FLAG_DEAL_HARMFUL_ABILITY;
                    procVictim = PROC_FLAG_TAKE_HARMFUL_ABILITY;
                }
                else
                {
                    procAttacker = PROC_FLAG_DEAL_HARMFUL_SPELL;
                    procVictim = PROC_FLAG_TAKE_HARMFUL_SPELL;
                }
            }
            break;
    }

    // Hunter traps spells (for Entrapment trigger)
    // Gives your Immolation Trap, Frost Trap, Explosive Trap, and Snake Trap ....
    if (m_spellInfo->SpellFamilyName == SPELLFAMILY_HUNTER && m_spellInfo->SpellFamilyFlags & uint64(0x000020000000001C))
        procAttacker |= PROC_FLAG_ON_TRAP_ACTIVATION;

    if (IsNextMeleeSwingSpell(m_spellInfo))
    {
        procAttacker |= PROC_FLAG_DEAL_MELEE_SWING;
        procVictim |= PROC_FLAG_TAKE_MELEE_SWING;
    }

    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX3_SUPPRESS_CASTER_PROCS))
        procAttacker = 0;

    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX3_SUPPRESS_TARGET_PROCS))
        procVictim = 0;
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
    if (m_trueCaster != target)
        m_targets.m_targetMask |= TARGET_FLAG_UNIT; // all spells with unit target must have this flag

    effectMask &= (~m_partialApplicationMask); // remove partially immuned out effects

    uint8 executionlessMask = 0;
    auto& data = SpellTargetMgr::GetSpellTargetingData(m_spellInfo->Id);
    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (data.implicitType[i] == TARGET_TYPE_SPECIAL_DEST)
            executionlessMask |= (1 << i);

    // Check for effect immune skip if immuned
    uint8 notImmunedMask = 0;
    for (uint8 effIndex = 0; effIndex < MAX_EFFECT_INDEX; ++effIndex)
        if ((effectMask & (1 << effIndex)) != 0)
            if (!target->IsImmuneToSpellEffect(m_spellInfo, SpellEffectIndex(effIndex), target == m_trueCaster))
                notImmunedMask |= (1 << effIndex);

    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX4_NO_PARTIAL_IMMUNITY) && notImmunedMask != effectMask)
        notImmunedMask = 0; // if one effect immunes out, whole target immunes out

    ObjectGuid targetGUID = target->GetObjectGuid();

    // Lookup target in already in list
    for (auto& ihit : m_UniqueTargetInfo)
    {
        if (targetGUID == ihit.targetGUID)                 // Found in list
        {
            ihit.effectMask |= (effectMask &~ executionlessMask);
            ihit.effectHitMask |= (notImmunedMask &~ executionlessMask);
            return;
        }
    }

    // This is new target calculate data for him

    // Get spell hit result on target
    TargetInfo targetInfo;
    targetInfo.targetGUID = targetGUID;                         // Store target GUID
    targetInfo.effectHitMask = exception != EXCEPTION_MAGNET ? notImmunedMask : effectMask; // Store not immuned effects
    targetInfo.effectMask = effectMask;                         // Store index of effect
    targetInfo.effectMaskProcessed = 0;
    targetInfo.processed  = false;                              // Effects not applied on target
    targetInfo.magnet = (exception == EXCEPTION_MAGNET);
    targetInfo.procReflect = false;
    targetInfo.isCrit = false;
    targetInfo.heartbeatResistChance = 0;
    targetInfo.effectDuration = CalculateSpellDuration(m_spellInfo, m_caster, target, m_auraScript);
    targetInfo.diminishDuration = targetInfo.effectDuration;
    targetInfo.diminishLevel = DIMINISHING_LEVEL_1;
    targetInfo.diminishGroup = DIMINISHING_NONE;
    targetInfo.executionless = false;

    // Calculate hit result
    targetInfo.missCondition = m_ignoreHitResult ? SPELL_MISS_NONE : Unit::SpellHitResult(m_trueCaster, target, m_spellInfo, targetInfo.effectMask, m_reflectable, false, &targetInfo.heartbeatResistChance);

    if ((executionlessMask & targetInfo.effectMask) != 0)
    {
        targetInfo.effectMask &= ~executionlessMask;
        targetInfo.effectHitMask &= ~executionlessMask;
        targetInfo.executionless = true;
    }

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
        // Objects vs units case: traps and similar
        // Pre-TBC: Reflect negates the spell when not in combat with caster, otherwise reflect
        if (!m_originalCasterGUID.IsUnit() && (!m_caster->IsInCombat() || !m_caster->IsAttackedBy(target)))
        {
            Unit::ProcDamageAndSpell(ProcSystemArguments(nullptr, target, PROC_FLAG_NONE, PROC_FLAG_TAKE_HARMFUL_SPELL, PROC_EX_REFLECT, 1, 0, BASE_ATTACK, m_spellInfo));
            targetInfo.reflectResult = SPELL_MISS_REFLECT;
        }
        else
        {
            // need to recheck reflect immuned effects
            notImmunedMask = 0;
            for (uint8 effIndex = 0; effIndex < MAX_EFFECT_INDEX; ++effIndex)
                if ((effectMask & (1 << effIndex)) != 0)
                    if (!target->IsImmuneToSpellEffect(m_spellInfo, SpellEffectIndex(effIndex), target == m_trueCaster))
                        notImmunedMask |= (1 << effIndex);

            targetInfo.effectHitMask = notImmunedMask;
            // Calculate reflected spell result on caster
            if (m_caster)
                targetInfo.reflectResult = Unit::SpellHitResult(m_caster, m_caster, m_spellInfo, targetInfo.effectMask, m_reflectable, true, &targetInfo.heartbeatResistChance);
            // Caster reflects back spell which was already reflected by victim
            if (targetInfo.reflectResult == SPELL_MISS_REFLECT || !m_caster)
                // Full circle: it's impossible to reflect further, "Immune" shows up
                targetInfo.reflectResult = SPELL_MISS_IMMUNE;

            // Store time interval so we know how fast to return it back
            targetInfo.initialDelay = targetInfo.timeDelay;
            targetInfo.procReflect = true;
            if (targetInfo.timeDelay == 0) // if no time delay, proc reflect procs immediately
                ProcReflectProcs(targetInfo);
        }
    }
    else
        targetInfo.reflectResult = SPELL_MISS_NONE;

    // only check DR for units and unit owned GOs
    if (Unit* realCaster = GetAffectiveCasterOrOwner())
    {
        bool isReflected = targetInfo.missCondition == SPELL_MISS_REFLECT && targetInfo.reflectResult == SPELL_MISS_NONE;
        Unit* targetForDiminish = isReflected ? m_caster : target;
        if ((targetInfo.missCondition == SPELL_MISS_NONE || isReflected) && CanSpellDiminish())
        {
            targetInfo.diminishGroup = GetDiminishingReturnsGroupForSpell(m_spellInfo, m_triggeredByAuraSpell != nullptr || (m_IsTriggeredSpell && m_CastItem));
            targetInfo.diminishLevel = targetForDiminish->GetDiminishing(targetInfo.diminishGroup);

            if (targetInfo.effectDuration > 0)
            {
                int32 duration = targetInfo.effectDuration;
                targetForDiminish->ApplyDiminishingToDuration(targetInfo.diminishGroup, duration, realCaster, targetInfo.diminishLevel, isReflected, m_spellInfo, m_auraScript);
                targetInfo.diminishDuration = duration;
                if (targetInfo.diminishDuration == 0 && targetInfo.diminishLevel == DIMINISHING_LEVEL_IMMUNE)
                    targetInfo.effectHitMask &= (~GetAuraEffectMask(m_spellInfo));
            }
        }
    }

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

void Spell::AddCorpseTarget(Corpse* target, uint8 effectMask)
{
    m_targets.m_targetMask |= m_spellInfo->Targets;

    // Lookup target in already in list
    for (auto& ihit : m_uniqueCorpseTargetInfo)
    {
        if (target->GetObjectGuid() == ihit.corpseGUID) // Found in list
        {
            ihit.effectMask |= effectMask;            // Add only effect mask
            return;
        }
    }

    CorpseTargetInfo info;
    info.corpseGUID = target->GetObjectGuid();
    info.effectMask = effectMask;

    m_uniqueCorpseTargetInfo.push_back(info);
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

    uint32 effectMask = (1 << effIndex);

    effectMask &= (~m_partialApplicationMask);

    m_destTargetInfo.effectMask |= effectMask;
}

void Spell::DoAllEffectOnTarget(TargetInfo* target)
{
    if (target->processed || target->effectMask == 0)       // Check target
        return;
    target->processed = true;                               // Target checked in apply effects procedure

    // Get mask of effects for target
    uint32 effectMask = target->effectHitMask &~ target->effectMaskProcessed;

    Unit* unit = m_trueCaster->GetObjectGuid() == target->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_trueCaster, target->targetGUID);
    if (!unit)
        return;

    // Get original caster (if exist) and calculate damage/healing from him data
    Unit* affectiveCaster = GetAffectiveCasterOrOwner();
    // FIXME: in case wild GO heal/damage spells will be used target bonuses
    WorldObject* caster = affectiveCaster ? affectiveCaster : m_trueCaster;

    SpellMissInfo missInfo = target->missCondition;
    // Need init unitTarget by default unit (can changed in code on reflect)
    // Or on missInfo!=SPELL_MISS_NONE unitTarget undefined (but need in trigger subsystem)
    unitTarget = unit;
    Unit* reflectTarget = nullptr;

    // Reset damage/healing counter
    ResetEffectDamageAndHeal();

    // Fill base trigger info
    uint32 procAttacker;
    uint32 procVictim;
    uint32 procEx = PROC_EX_NONE;
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

    m_damage = target->damage;
    m_healing = target->healing;

    if (missInfo == SPELL_MISS_NONE)                        // In case spell hit target, do all effect on that target
        DoSpellHitOnUnit(unit, effectMask, target);
    else if (missInfo != SPELL_MISS_EVADE)
    {
        if (missInfo == SPELL_MISS_REFLECT)                // In case spell reflect from target, do all effect on caster (if hit)
        {
            if (target->reflectResult == SPELL_MISS_NONE)       // If reflected spell hit caster -> do all effect on him
            {
                DoSpellHitOnUnit(m_caster, effectMask, target, true);
                reflectTarget = unit;
                unitTarget = m_caster;
            }
        }

        // Failed hostile spell hits count as attack made against target (if detected)
        if (affectiveCaster && affectiveCaster != unit)
            m_caster->CasterHitTargetWithSpell(affectiveCaster, unit, m_spellInfo, m_IsTriggeredSpell, false);
    }

    // All calculated do it!
    // Do healing and triggers
    if (m_healing && affectiveCaster) // does not support GO casts atm
    {
        uint32 addhealth = m_healing;
        if (target->isCrit)
        {
            procEx |= PROC_EX_CRITICAL_HIT;
            addhealth = affectiveCaster->CalculateCritAmount(nullptr, addhealth, m_spellInfo, true);
        }
        else
            procEx |= PROC_EX_NORMAL_HIT;

        m_healing = addhealth; // update value so that script handler has access
        OnHit(missInfo); // TODO: After spell damage calc is moved to proper handler - move this before the first if

        int32 gain = affectiveCaster->DealHeal(unitTarget, addhealth, m_spellInfo, target->isCrit);

        if (affectiveCaster)
            unitTarget->getHostileRefManager().threatAssist(affectiveCaster, float(gain) * 0.5f * sSpellMgr.GetSpellThreatMultiplier(m_spellInfo), m_spellInfo, false, m_IsTriggeredSpell);

        // Do triggers for unit (reflect triggers passed on hit phase for correct drop charge)
        if (m_canTrigger && missInfo != SPELL_MISS_REFLECT)
        {
            Unit::ProcDamageAndSpell(ProcSystemArguments(affectiveCaster, unitTarget, affectiveCaster ? procAttacker : uint32(PROC_FLAG_NONE), procVictim, procEx, addhealth, 0, m_attackType, m_spellInfo, this, gain, true));
        }
    }
    // Do damage and triggers
    else if (m_damage)
    {
        // Fill base damage struct (unitTarget - is real spell target)
        SpellNonMeleeDamage spellDamageInfo(caster, unitTarget, m_spellInfo->Id, GetFirstSchoolInMask(m_spellSchoolMask), this);

        spellDamageInfo.damage = m_damage;
        spellDamageInfo.HitInfo = target->HitInfo;
        if (!m_spellInfo->HasAttribute(SPELL_ATTR_EX3_IGNORE_CASTER_MODIFIERS))
        {
            if (target->isCrit) // GOs cant crit
            {
                spellDamageInfo.HitInfo |= SPELL_HIT_TYPE_CRIT;
                spellDamageInfo.damage = affectiveCaster->CalculateCritAmount(unitTarget, spellDamageInfo.damage, m_spellInfo);
            }
        }
        // damage mitigation
        if (spellDamageInfo.damage > 0)
        {
            // physical damage => armor
            if (m_spellSchoolMask & SPELL_SCHOOL_MASK_NORMAL)
                spellDamageInfo.damage = Unit::CalcArmorReducedDamage(affectiveCaster ? affectiveCaster : m_trueCaster, unitTarget, spellDamageInfo.damage);
        }

        unitTarget->CalculateAbsorbResistBlock(affectiveCaster, &spellDamageInfo, m_spellInfo);

        Unit::DealDamageMods(affectiveCaster, spellDamageInfo.target, spellDamageInfo.damage, &spellDamageInfo.absorb, SPELL_DIRECT_DAMAGE, m_spellInfo);

        m_damage = spellDamageInfo.damage; // update value so that script handler has access
        OnHit(missInfo); // TODO: After spell damage calc is moved to proper handler - move this before the first if

        // Send log damage message to client
        Unit::SendSpellNonMeleeDamageLog(&spellDamageInfo);

        procEx |= createProcExtendMask(&spellDamageInfo, missInfo);
        procVictim |= PROC_FLAG_TAKE_ANY_DAMAGE;

        if (reflectTarget)
            Unit::DealSpellDamage(affectiveCaster, &spellDamageInfo, true, m_resetLeash);
        else
            Unit::DealSpellDamage(affectiveCaster, &spellDamageInfo, true, m_resetLeash);

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
            Unit::ProcDamageAndSpell(ProcSystemArguments(affectiveCaster, unitTarget, affectiveCaster ? procAttacker : uint32(PROC_FLAG_NONE), procVictim, procEx, spellDamageInfo.damage, spellDamageInfo.absorb, m_attackType, m_spellInfo, this));
    }
    // Passive spell hits/misses or active spells only misses (only triggers if proc flags set)
    else if (procAttacker || procVictim)
    {
        OnHit(missInfo); // TODO: After spell damage calc is moved to proper handler - move this before the first if
        // Fill base damage struct (unitTarget - is real spell target)
        SpellNonMeleeDamage damageInfo(caster, unitTarget, m_spellInfo->Id, GetFirstSchoolInMask(m_spellSchoolMask));
        procEx = createProcExtendMask(&damageInfo, missInfo);
        // Do triggers for unit (reflect triggers passed on hit phase for correct drop charge)
        if (m_canTrigger && missInfo != SPELL_MISS_REFLECT)
            // traps need to be procced at trap triggerer
            Unit::ProcDamageAndSpell(ProcSystemArguments(affectiveCaster, procAttacker & PROC_FLAG_ON_TRAP_ACTIVATION ? m_targets.getUnitTarget() : unit, affectiveCaster ? procAttacker : uint32(PROC_FLAG_NONE), procVictim, procEx, 0, 0, m_attackType, m_spellInfo, this));
    }

    OnAfterHit();

    if (unit->IsCreature())
        // cast at creature (or GO) quest objectives update at successful cast finished (+channel finished)
        // ignore pets or autorepeat/melee casts for speed (not exist quest for spells (hm... )
        if (affectiveCaster && !static_cast<Creature*>(unit)->IsPet() && !IsAutoRepeat() && !IsNextMeleeSwingSpell(m_spellInfo) && !IsChannelActive())
            if (Player* p = affectiveCaster->GetBeneficiaryPlayer())
                p->RewardPlayerAndGroupAtCast(unit, m_spellInfo->Id);

    // Call scripted function for AI if this spell is casted upon a creature
    if (unit->AI())
        unit->AI()->SpellHit(m_caster, m_spellInfo);

    // Call scripted function for AI if this spell is casted by a creature
    if (m_trueCaster->IsCreature() && static_cast<Creature*>(m_trueCaster)->AI())
        static_cast<Creature*>(m_trueCaster)->AI()->SpellHitTarget(unit, m_spellInfo, missInfo);
    if (affectiveCaster && affectiveCaster != m_caster && affectiveCaster->IsCreature() && static_cast<Creature*>(affectiveCaster)->AI())
        static_cast<Creature*>(affectiveCaster)->AI()->SpellHitTarget(unit, m_spellInfo, missInfo);

    if (m_spellAuraHolder && m_caster)
    {
        if (m_caster->IsSpellProccingHappening())
            m_caster->AddDelayedHolderDueToProc(m_spellAuraHolder);
        else
            m_spellAuraHolder->SetState(SPELLAURAHOLDER_STATE_READY);
    }
}

void Spell::DoSpellHitOnUnit(Unit* unit, uint32 effectMask, TargetInfo* target, bool isReflected)
{
    if (!unit)
        return;

    const bool traveling = m_spellState == SPELL_STATE_TRAVELING;

    // Recheck immune (only for delayed spells)
    if (traveling && !m_spellInfo->HasAttribute(SPELL_ATTR_NO_IMMUNITIES))
    {
        uint8 notImmunedMask = 0;
        for (uint8 effIndex = 0; effIndex < MAX_EFFECT_INDEX; ++effIndex)
            if ((target->effectHitMask & (1 << effIndex)) != 0)
                if (!unit->IsImmuneToSpellEffect(m_spellInfo, SpellEffectIndex(effIndex), unit == m_trueCaster))
                    notImmunedMask |= (1 << effIndex);
        effectMask = notImmunedMask & ~target->effectMaskProcessed;
        if (!notImmunedMask ||
            unit->IsImmuneToDamage(GetSpellSchoolMask(m_spellInfo)) ||
            unit->IsImmuneToSpell(m_spellInfo, unit == m_trueCaster, effectMask, m_trueCaster))
        {
            Unit::SendSpellMiss(m_trueCaster, unit, m_spellInfo->Id, SPELL_MISS_IMMUNE);

            ResetEffectDamageAndHeal();
            return;
        }
    }

    if (traveling && m_trueCaster != unit)
    {
        if (m_trueCaster->CanAttackSpell(unit, m_spellInfo))
        {
            // for delayed spells ignore not visible explicit target, if caster is dead, nothing is visible for him
            if (unit == m_targets.getUnitTarget() && ((m_trueCaster->IsGameObject() && !unit->IsAlive()) || (!unit->IsVisibleForOrDetect(m_caster, m_caster, false, false, true, true))))
            {
                ResetEffectDamageAndHeal();
                return;
            }
        }
        else
        {
            // for delayed spells ignore negative spells (after duel end) for friendly targets
            if (!IsPositiveSpell(m_spellInfo->Id, m_trueCaster, unit))
            {
                Unit::SendSpellMiss(m_trueCaster, unit, m_spellInfo->Id, SPELL_MISS_IMMUNE);
                ResetEffectDamageAndHeal();
                return;
            }
        }
    }

    // Apply additional spell effects to target
    CastPreCastSpells(unit);

    Unit* affectiveCaster = GetAffectiveCasterOrOwner();
    if (IsSpellAppliesAura(m_spellInfo, effectMask))
    {
        if (affectiveCaster)
        {
            if (traveling) // if travelling, need to recalculate diminishing level and duration
            {
                target->diminishLevel = unit->GetDiminishing(target->diminishGroup);
                if (target->effectDuration > 0)
                {
                    int32 duration = target->effectDuration;
                    unit->ApplyDiminishingToDuration(target->diminishGroup, duration, m_caster, target->diminishLevel, isReflected, m_spellInfo, m_auraScript);
                    target->diminishDuration = duration;
                }
            }

            if (m_duration != target->diminishDuration && target->diminishDuration == 0 && target->diminishLevel > DIMINISHING_LEVEL_1 && !IsSpellWithNonAuraEffect(m_spellInfo))
            {
                Unit::SendSpellMiss(m_trueCaster, unit, m_spellInfo->Id, SPELL_MISS_IMMUNE);
                ResetEffectDamageAndHeal();
                return;
            }

            const bool pvp = (unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && affectiveCaster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED));
            // Increase Diminishing on unit, actual durations are already calculated
            if (IsSubjectToDiminishingLevels(target->diminishGroup, pvp))
                unit->IncrDiminishing(target->diminishGroup, pvp);
        }

        // Only unit caster to be passed here
        m_spellAuraHolder = CreateSpellAuraHolder(m_spellInfo, unit, m_trueCaster, m_CastItem, m_triggeredBySpellInfo);
        m_spellAuraHolder->setDiminishGroup(target->diminishGroup);
    }
    else
        m_spellAuraHolder = nullptr;

    ExecuteEffects(unit, nullptr, nullptr, effectMask);

    if (affectiveCaster && affectiveCaster != unit && m_caster)
        Unit::CasterHitTargetWithSpell(affectiveCaster, unit, m_spellInfo, m_IsTriggeredSpell);

    // now apply all created auras
    if (m_spellAuraHolder)
    {
        // normally shouldn't happen
        if (!m_spellAuraHolder->IsEmptyHolder())
        {
            int32 duration = m_spellAuraHolder->GetAuraMaxDuration();
            int32 originalDuration = duration;

            if (duration > 0 && target->diminishGroup > DIMINISHING_NONE)
            {
                duration = target->diminishDuration;
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

            if (originalDuration > 0 && target && target->heartbeatResistChance && !m_spellAuraHolder->IsPositive())
            {
                // Pre-TBC: heartbeat is enabled in both pve and pvp
                if (bool heartbeat = true)
                    m_spellAuraHolder->SetHeartbeatResist(target->heartbeatResistChance, originalDuration, uint32(target->diminishLevel));
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

    uint32 effectMask;
    if (dest) // can have delay
    {
        effectMask = m_destTargetInfo.effectMask &~ m_destTargetInfo.effectMaskProcessed;
        m_destTargetInfo.effectMaskProcessed = m_destTargetInfo.effectMask;
        for (uint32 j = 0; j < MAX_EFFECT_INDEX; ++j)
        {
            if ((effectMask & (1 << j)) != 0)
                HandleEffect(nullptr, nullptr, nullptr, SpellEffectIndex(j));
        }
    }
    else // always immediate
    {
        effectMask = m_targetlessMask;
        for (uint32 j = 0; j < MAX_EFFECT_INDEX; ++j)
        {
            if ((effectMask & (1 << j)) != 0)
                HandleEffect(nullptr, nullptr, nullptr, SpellEffectIndex(j));
        }

        // dest effects that are immediate
        uint32 destMaskImmediate = m_destTargetInfo.effectMask;
        for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
            if (!IsEffectHandledImmediatelySpellLaunch(m_spellInfo, SpellEffectIndex(i)))
                destMaskImmediate &= ~(destMaskImmediate & (1 << i));

        for (uint32 j = 0; j < MAX_EFFECT_INDEX; ++j)
        {
            if ((destMaskImmediate & (1 << j)) != 0)
                HandleEffect(nullptr, nullptr, nullptr, SpellEffectIndex(j));
        }
        m_destTargetInfo.effectMaskProcessed = destMaskImmediate;
    }

    if (effectMask)
    {
        OnHit(SPELL_MISS_NONE);

        if (m_caster)
        {
            PrepareMasksForProcSystem(effectMask, procAttacker, procVictim, m_trueCaster, unitTarget);
            Unit::ProcDamageAndSpell(ProcSystemArguments(m_caster, procAttacker & PROC_FLAG_ON_TRAP_ACTIVATION ? m_targets.getUnitTarget() : nullptr, m_caster ? procAttacker : uint32(PROC_FLAG_NONE), procVictim, procEx, 0, 0, m_attackType, m_spellInfo, this));
        }
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

    GameObject* go = m_trueCaster->GetMap()->GetGameObject(target->targetGUID);
    if (!go)
        return;

    gameObjTarget = go; // redundant but for safety against future change for hook functionality

    ExecuteEffects(nullptr, nullptr, go, effectMask);

    OnHit(SPELL_MISS_NONE);

    // cast at creature (or GO) quest objectives update at successful cast finished (+channel finished)
    // ignore autorepeat/melee casts for speed (not exist quest for spells (hm... )
    if (!IsAutoRepeat() && !IsNextMeleeSwingSpell(m_spellInfo) && !IsChannelActive() && m_caster)
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

    itemTarget = target->item; // redundant but for safety against future change for hook functionality

    ExecuteEffects(nullptr, target->item, nullptr, effectMask);

    OnHit(SPELL_MISS_NONE);
}

void Spell::DoAllEffectOnTarget(CorpseTargetInfo* target)
{
    uint32 effectMask = target->effectMask;
    if (!target->corpseGUID || !effectMask)
        return;

    Corpse* corpse = ObjectAccessor::GetCorpseInMap(target->corpseGUID, m_trueCaster->GetMapId());
    if (!corpse)
        return;

    corpseTarget = corpse;
    ExecuteEffects(nullptr, nullptr, nullptr, effectMask);
}

void Spell::HandleImmediateEffectExecution(TargetInfo* target)
{
    if (target->effectMask == 0)
        return;

    // Get mask of effects for target
    uint32 mask = target->effectHitMask;

    Unit* unit = m_trueCaster->GetObjectGuid() == target->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_trueCaster, target->targetGUID);
    if (!unit)
        return;

    // Get original caster (if exist) and calculate damage/healing from him data
    Unit* affectiveCaster = GetAffectiveCaster();
    // FIXME: in case wild GO heal/damage spells will be used target bonuses
    Unit* caster = affectiveCaster ? affectiveCaster : m_caster;

    SpellMissInfo missInfo = target->missCondition;
    // Need init unitTarget by default unit (can changed in code on reflect)
    // Or on missInfo!=SPELL_MISS_NONE unitTarget undefined (but need in trigger subsystem)
    unitTarget = unit;

    // Reset damage/healing counter
    m_damage = 0;
    m_healing = 0; // healing maybe not needed at this point

    // keep damage amount for reflected spells
    if (missInfo == SPELL_MISS_NONE || (missInfo == SPELL_MISS_REFLECT && target->reflectResult == SPELL_MISS_NONE))
    {
        for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
            if (!IsEffectHandledImmediatelySpellLaunch(m_spellInfo, SpellEffectIndex(i)))
                mask &= ~(mask & (1 << i));
        ExecuteEffects(unit, nullptr, nullptr, mask);
    }

    target->damage = m_damage;
    target->healing = m_healing;
    target->HitInfo = 0;
    target->effectMaskProcessed = mask;
    if (m_damage > 0 || m_healing > 0)
        target->isCrit = Unit::RollSpellCritOutcome(affectiveCaster, unit, m_spellSchoolMask, m_spellInfo);
}

bool Spell::CanSpellDiminish() const
{
    return m_caster && m_caster->IsPlayerControlled() || IsCreatureDRSpell(m_spellInfo);
}

void Spell::InitializeDamageMultipliers()
{
    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (m_spellInfo->Effect[i] == 0)
            continue;

        uint32 EffectChainTarget = m_spellInfo->EffectChainTarget[i];
        if (Unit* realCaster = GetAffectiveCaster())
            if (Player* modOwner = realCaster->GetSpellModOwner())
                modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_JUMP_TARGETS, EffectChainTarget);

        m_damageMultipliers[i] = 1.0f;
        if ((m_spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_ENEMY || m_spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_FRIEND_CHAIN_HEAL) && (EffectChainTarget > 1))
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
            Unit* unit = m_trueCaster->GetObjectGuid() == ihit->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_trueCaster, ihit->targetGUID);

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
    if (target->IsAlive())
        return !IsDeathOnlySpell(m_spellInfo);
    if (IsAllowingDeadTarget(m_spellInfo))
        return true;
    return false;
}

// Helper for Chain Healing
// Spell target first
// Raidmates then descending by injury suffered (MaxHealth - Health)
// Other players/mobs then descending by injury suffered (MaxHealth - Health)
struct ChainHealingOrder
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

class ChainHealingFullHealth
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
    uint32 unMaxTargets = m_affectedTargetCount;  // Get spell max affected targets

    GetSpellRangeAndRadius(effIndex, radius, targetB, targetingData.chainTargetCount[effIndex]);
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
            WorldObject* caster = m_trueCaster;
            float angle = m_trueCaster->GetOrientation();
            switch (targetMode)
            {
                case TARGET_LOCATION_CASTER_FRONT_LEFT:  angle += M_PI_F * 0.25f; break;
                case TARGET_LOCATION_CASTER_BACK_LEFT:   angle += M_PI_F * 0.75f; break;
                case TARGET_LOCATION_CASTER_BACK_RIGHT:  angle += M_PI_F * 1.25f; break;
                case TARGET_LOCATION_CASTER_FRONT_RIGHT: angle += M_PI_F * 1.75f; break;
                case TARGET_LOCATION_CASTER_FRONT:                                break;
                case TARGET_LOCATION_CASTER_BACK:        angle += M_PI_F;         break;
                case TARGET_LOCATION_UNIT_MINION_POSITION: caster = GetCastingObject(); // projected from original caster if necessary
                case TARGET_LOCATION_CASTER_LEFT:        angle += M_PI_F / 2;     break;
                case TARGET_LOCATION_CASTER_RIGHT:       angle -= M_PI_F / 2;     break;
            }

            Position pos;
            caster->GetFirstCollisionPosition(pos, radius, angle);
            m_targets.setDestination(pos.x, pos.y, pos.z);
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
                m_targets.m_destPos.o = st->target_Orientation;
                m_targets.m_mapId = st->target_mapId;

                // far-teleport spells are handled in SpellEffect, elsewise report an error about an unexpected map (spells are always locally)
                if (st->target_mapId != m_trueCaster->GetMapId() && m_spellInfo->Effect[effIndex] != SPELL_EFFECT_TELEPORT_UNITS && m_spellInfo->Effect[effIndex] != SPELL_EFFECT_BIND)
                    sLog.outError("SPELL: wrong map (%u instead %u) target coordinates for spell ID %u", st->target_mapId, m_trueCaster->GetMapId(), m_spellInfo->Id);
            }
            else
                sLog.outError("SPELL: unknown target coordinates for spell ID %u", m_spellInfo->Id);
            break;
        }
        case TARGET_LOCATION_CASTER_HOME_BIND:
        {
            if (m_trueCaster->IsPlayer())
            {
                float x, y, z;
                static_cast<Player*>(m_trueCaster)->GetHomebindLocation(x, y, z, m_targets.m_mapId);
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
            if (!m_caster->GetMap()->GetHeightInRange(prevPos.x, prevPos.y, groundZ, 3.0f) && m_caster->m_movementInfo.HasMovementFlag(MOVEFLAG_JUMPING))
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

                if (fabs(prevPos.z - nextPos.z) > 40.f) // dont move too high - magical constant - might need verification
                {
                    nextPos.x = prevPos.x;
                    nextPos.y = prevPos.y;
                    nextPos.z = prevPos.z - 2.0f;
                }
                else // check any obstacle and fix coords
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
            break;
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
            SpellCastResult result = SPELL_CAST_OK;
            for (uint32 i = 0; i < 10; ++i)
            {
                float dis = rand_norm_f() * (max_dis - min_dis) + min_dis;
                // calculate angle variation for roughly equal dimensions of target area
                float max_angle = (max_dis - min_dis) / (max_dis + m_caster->GetObjectBoundingRadius());
                float angle_offset = max_angle * (rand_norm_f() - 0.5f);
                m_caster->GetNearPoint2d(x, y, dis + m_caster->GetObjectBoundingRadius(), m_caster->GetOrientation() + angle_offset);

                GridMapLiquidData liqData;
                if (!m_caster->GetTerrain()->IsInWater(x, y, m_caster->GetTerrain()->GetWaterLevel(x, y, m_caster->GetPositionZ()) - 1.0f, &liqData))
                {
                    result = SPELL_FAILED_NOT_FISHABLE;
                    continue;
                }

                z = liqData.level;
                // finally, check LoS
                if (!m_caster->IsWithinLOS(x, y, z + 1.f))
                {
                    result = SPELL_FAILED_LINE_OF_SIGHT;
                    continue;
                }
            }
            if (result != SPELL_CAST_OK)
            {
                SendCastResult(result);
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
            float max_range = radius + unMaxTargets * m_jumpRadius;

            UnitList tempTargetUnitMap;

            switch (targetMode)
            {
                case TARGET_UNIT_ENEMY_NEAR_CASTER:
                {
                    FillAreaTargets(tempTargetUnitMap, max_range, 0.f, PUSH_SELF_CENTER, SPELL_TARGETS_CHAIN_ATTACKABLE);
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
                CheckSpellScriptTargets(bounds, tempTargetUnitMap, tempUnitList, effIndex);
            else
                tempUnitList.splice(tempUnitList.end(), tempTargetUnitMap);
            break;
        }
        case TARGET_UNIT_CASTER_PET:
        {
            if (Pet* tmpUnit = m_caster->GetPet())
                tempUnitList.push_back(tmpUnit);
            else if (Unit* charm = m_caster->GetCharm())
                tempUnitList.push_back(charm);
            break;
        }
        case TARGET_UNIT:
        {
            Unit* newUnitTarget = m_targets.getUnitTarget();
            if (!newUnitTarget)
                break;

            if (!m_trueCaster->CanAssistSpell(newUnitTarget, m_spellInfo))
            {
                if (CheckAndAddMagnetTarget(newUnitTarget, effIndex, targetB, targetingData))
                    break;
            }

            tempUnitList.push_back(newUnitTarget);

            // More than one target
            if (targetingData.chainTargetCount[effIndex] > 1)
            {
                // Getting spell casting distance
                float minRadiusCaster = 0, maxRadiusTarget = 0;
                GetChainJumpRange(m_spellInfo, effIndex, minRadiusCaster, maxRadiusTarget);

                // Filling target map
                UnitList tempAoeList;
                {
                    FillAreaTargets(tempAoeList, maxRadiusTarget, cone, m_spellInfo->HasAttribute(SPELL_ATTR_EX2_CHAIN_FROM_CASTER) ? PUSH_SELF_CENTER : PUSH_TARGET_CENTER, SPELL_TARGETS_ALL);
                    tempAoeList.erase(std::remove(tempAoeList.begin(), tempAoeList.end(), newUnitTarget), tempAoeList.end());
                }

                tempUnitList.splice(tempUnitList.end(), tempAoeList);
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
            if (targetingData.chainTargetCount[effIndex] > 1)
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
                        me->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_PVP_ACTIVE_CANCELS);
                    }
                }

                // Getting spell casting distance
                float minRadiusCaster = 0, maxRadiusTarget = 0;
                GetChainJumpRange(m_spellInfo, effIndex, minRadiusCaster, maxRadiusTarget);

                // Filling target map
                UnitList tempAoeList;
                {
                    SpellNotifyPushType pushType = PUSH_TARGET_CENTER;
                    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_CHAIN_FROM_CASTER))
                        pushType = PUSH_SELF_CENTER;
                    FillAreaTargets(tempAoeList, maxRadiusTarget, cone, pushType, SPELL_TARGETS_CHAIN_ATTACKABLE);
                    tempAoeList.erase(std::remove(tempAoeList.begin(), tempAoeList.end(), newUnitTarget), tempAoeList.end());

                    for (auto itr = tempAoeList.begin(); itr != tempAoeList.end();)
                    {
                        if(!(*itr)->IsVisibleForOrDetect(m_caster, m_originalCaster, false))
                            itr = tempAoeList.erase(itr);
                        else
                            ++itr;
                    }
                }

                // No targets. No need to process.
                if (tempAoeList.empty())
                    break;

                if (minRadiusCaster)
                {
                    float x, y, z;
                    m_caster->GetPosition(x, y, z);
                    auto itr = tempAoeList.begin();
                    ++itr; // start from 2nd
                    for (; itr != tempAoeList.end();)
                    {
                        if ((*itr)->GetDistance(x, y, z, DIST_CALC_COMBAT_REACH) < minRadiusCaster)
                            itr = tempAoeList.erase(itr);
                        else
                            ++itr;
                    }
                }
                tempUnitList.splice(tempUnitList.end(), tempAoeList);
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
                Cell::VisitGridObjects(m_trueCaster, checker, radius);
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
                if (m_trueCaster->CanAssistSpell(target, m_spellInfo))
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
            Unit* target = m_targets.getUnitTarget();
            if (!target || !m_caster->CanAssistSpell(target, m_spellInfo))
                break;

            tempUnitList.push_back(target);

            Player* pTarget = const_cast<Player*>(target->GetControllingPlayer());
            if (!pTarget)
                break;

            if (pTarget != target)
                tempUnitList.push_back(pTarget);

            Group* pGroup = pTarget->GetGroup();

            if (pGroup)
            {
                uint8 subgroup = pTarget->GetSubGroup();

                for (GroupReference* itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
                {
                    Player* groupTarget = itr->getSource();

                    if (groupTarget == target || groupTarget == pTarget)
                        continue;

                    // CanAssist check duel and controlled by enemy
                    if (groupTarget && groupTarget->GetSubGroup() == subgroup && m_caster->CanAssistSpell(groupTarget, m_spellInfo))
                    {
                        if (pTarget->IsWithinDistInMap(groupTarget, radius))
                            tempUnitList.push_back(groupTarget);

                        if (Pet* pet = groupTarget->GetPet())
                            if (pTarget->IsWithinDistInMap(pet, radius))
                                tempUnitList.push_back(pet);
                    }
                }
            }
            else if (pTarget)
            {
                if (Pet* pet = pTarget->GetPet())
                    if (m_caster->IsWithinDistInMap(pet, radius))
                        tempUnitList.push_back(pet);
            }
            break;
        }
        case TARGET_UNIT_FRIEND_CHAIN_HEAL:
        {
            Unit* unitTarget = m_targets.getUnitTarget();
            if (!unitTarget)
                break;

            if (!m_caster->CanAssistSpell(unitTarget, m_spellInfo))
                break;

            tempUnitList.push_back(unitTarget);

            if (targetingData.chainTargetCount[effIndex] > 1)
            {
                float max_range = targetingData.chainTargetCount[effIndex] * m_jumpRadius;
                Group* group = nullptr;

                //if target and caster are members in the same group then apply member only filtering
                //in regards to mc by player concerns the mc'ed player has simple toolbar thus chain heal not available
                //TODO: in regards to mc by npc concerns this is something that needs to be answered
                if (m_caster->GetTypeId() == TYPEID_PLAYER)
                {
                    if (Group* casterGroup = static_cast<Player*>(m_caster)->GetGroup())
                    {
                        switch (unitTarget->GetTypeId())
                        {
                            case TYPEID_PLAYER:
                            {
                                if (casterGroup == static_cast<Player*>(unitTarget)->GetGroup())
                                    group = casterGroup;

                                break;
                            }
                            case TYPEID_UNIT:
                            {
                                Creature* creature = static_cast<Creature*>(unitTarget);

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

                UnitList tempAoeList;
                MaNGOS::AnyFriendlyOrGroupMemberUnitInUnitRangeCheck u_check(m_caster, unitTarget, group, m_spellInfo, max_range);
                MaNGOS::UnitListSearcher<MaNGOS::AnyFriendlyOrGroupMemberUnitInUnitRangeCheck> searcher(tempAoeList, u_check);
                Cell::VisitAllObjects(m_caster, searcher, max_range);
                tempAoeList.erase(std::remove(tempAoeList.begin(), tempAoeList.end(), unitTarget), tempAoeList.end());
                if (!tempAoeList.empty())
                    tempUnitList.splice(tempUnitList.end(), tempAoeList);
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
            SpellCastResult result = CheckScriptTargeting(effIndex, targetingData.chainTargetCount[effIndex], radius, targetMode, tempUnitList, tempGOList);
            if (result != SPELL_CAST_OK)
            {
                SendCastResult(result);
                finish(false);
                return;
            }
            break;
        }
        default:
            // sLog.outError( "SPELL: Unknown implicit target (%u) for spell ID %u", targetMode, m_spellInfo->Id );
            break;
    }

    // remove caster from the list if required by attribute
    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_EXCLUDE_CASTER))
        if (targetMode != TARGET_UNIT_CASTER)
            tempUnitList.remove(m_caster);
}

SpellCastResult Spell::CheckScriptTargeting(SpellEffectIndex effIndex, uint32 chainTargets, float radius, uint32 targetMode, UnitList& tempUnitList, GameObjectList& tempGOList)
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

        return SPELL_FAILED_BAD_TARGETS;
    }

    CreatureList foundScriptCreatureTargets;
    GameObjectList foundScriptGOTargets;
    std::set<uint32> entriesToUse;
    uint32 type = MAX_SPELL_TARGET_TYPE;
    bool foundButOutOfRange = false;
    // corrections for numerous spells missing it
    uint32 targetCount = std::max(chainTargets, uint32(1));

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
            case SPELL_TARGET_TYPE_GAMEOBJECT_GUID:
            default:
                entriesToUse.insert(i_spellST->targetEntry);
                break;
        }
    }

    if (radius == 50000.f)
    {
        if (type != SPELL_TARGET_TYPE_CREATURE_GUID && type != SPELL_TARGET_TYPE_GAMEOBJECT_GUID)
            radius = 200.f;
    }
    else if (radius == 0.f)
        radius = m_trueCaster->GetMap()->IsDungeon() ? DEFAULT_VISIBILITY_INSTANCE : DEFAULT_VISIBILITY_DISTANCE;

    WorldObject* caster = GetAffectiveCasterObject();

    switch (type) // TODO: Unify logic with all other spell_script_target uses
    {
        case SPELL_TARGET_TYPE_GAMEOBJECT:
        case SPELL_TARGET_TYPE_GAMEOBJECT_GUID:
        {
            if (type == SPELL_TARGET_TYPE_GAMEOBJECT_GUID)
            {
                for (uint32 guid : entriesToUse)
                    if (GameObject* go = m_trueCaster->GetMap()->GetGameObject(guid))
                        if (go->IsWithinDist(m_trueCaster, radius))
                            foundScriptGOTargets.push_back(go);
            }
            else
            {
                MaNGOS::AllGameObjectEntriesListInObjectRangeCheck go_check(*m_trueCaster, entriesToUse, radius);
                MaNGOS::GameObjectListSearcher<MaNGOS::AllGameObjectEntriesListInObjectRangeCheck> checker(foundScriptGOTargets, go_check);
                Cell::VisitGridObjects(m_trueCaster, checker, radius);
            }

            break;
        }
        case SPELL_TARGET_TYPE_CREATURE:
        case SPELL_TARGET_TYPE_DEAD:
        case SPELL_TARGET_TYPE_CREATURE_GUID:
        {
            if (Unit* target = m_targets.getUnitTarget())
            {
                if (target->GetTypeId() == TYPEID_UNIT && entriesToUse.find(type == SPELL_TARGET_TYPE_CREATURE_GUID ? target->GetDbGuid() : target->GetEntry()) != entriesToUse.end())
                {
                    if ((type == SPELL_TARGET_TYPE_CREATURE && target->IsAlive()) ||
                        (type == SPELL_TARGET_TYPE_DEAD && ((Creature*)target)->IsCorpse()))
                    {
                        if (target->IsWithinDistInMap(caster, radius) && OnCheckTarget(target, effIndex))
                            foundScriptCreatureTargets.push_back((Creature*)target);
                        else
                            foundButOutOfRange = true;
                    }
                }
            }
            if (foundScriptCreatureTargets.size() < targetCount)
            {
                if (radius == 50000.f) // only guid scripting
                {
                    for (uint32 guid : entriesToUse)
                        if (Creature* creature = m_trueCaster->GetMap()->GetCreature(guid))
                            foundScriptCreatureTargets.push_back(creature);
                }
                else
                {
                    MaNGOS::AllCreatureEntriesWithLiveStateInObjectRangeCheck u_check(*caster, entriesToUse, type != SPELL_TARGET_TYPE_DEAD, radius, type == SPELL_TARGET_TYPE_CREATURE_GUID, false, true);
                    MaNGOS::CreatureListSearcher<MaNGOS::AllCreatureEntriesWithLiveStateInObjectRangeCheck> searcher(foundScriptCreatureTargets, u_check);
                    Cell::VisitAllObjects(caster, searcher, radius); // Visit all, need to find also Pet* objects
                    if (u_check.FoundOutOfRange())
                        foundButOutOfRange = true;
                }
            }
            break;
        }
        case SPELL_TARGET_TYPE_STRING_ID:
        {
            for (uint32 stringId : entriesToUse)
            {
                auto creatures = m_trueCaster->GetMap()->GetCreatures(stringId);
                for (Creature* creature : *creatures)
                {
                    if (creature->IsWithinCombatDistInMap(caster, radius, true))
                        foundScriptCreatureTargets.push_back(creature);
                    else
                        foundButOutOfRange = true;
                }
                auto gameObjects = m_trueCaster->GetMap()->GetGameObjects(stringId);
                for (GameObject* gameObject : *gameObjects)
                {
                    if (gameObject->IsWithinDist(caster, radius, true))
                        foundScriptGOTargets.push_back(gameObject);
                    else
                        foundButOutOfRange = true;
                }
            }
            break;
        }
        default:
            break;
    }

    if (!foundScriptCreatureTargets.empty())
    {
        for (auto iter = foundScriptCreatureTargets.begin(); iter != foundScriptCreatureTargets.end();)
        {
            bool failed = false;
            if (!OnCheckTarget(*iter, effIndex))
                failed = true;
            else if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_EXCLUDE_CASTER) && m_trueCaster == (*iter))
                failed = true;
            if (failed)
                iter = foundScriptCreatureTargets.erase(iter);
            else
                ++iter;
        }

        foundScriptCreatureTargets.sort([=](const Creature* a, const Creature* b) -> bool
        {
            return caster->GetDistance(a, true, DIST_CALC_NONE) < caster->GetDistance(b, true, DIST_CALC_NONE);
        });

        if (foundScriptCreatureTargets.size() > targetCount) // if we have too many targets, we need to trim the list
            foundScriptCreatureTargets.resize(targetCount);
    }

    if (!foundScriptGOTargets.empty())
    {
        for (auto itr = foundScriptGOTargets.begin(); itr != foundScriptGOTargets.end();)
        {
            if (!OnCheckTarget(*itr, effIndex))
                itr = foundScriptGOTargets.erase(itr);
            else
                ++itr;
        }
        foundScriptGOTargets.sort([=](const GameObject* a, const GameObject* b) -> bool
        {
            return caster->GetDistance(a, true, DIST_CALC_NONE) < caster->GetDistance(b, true, DIST_CALC_NONE);
        });

        if (foundScriptGOTargets.size() > targetCount) // if we have too many targets, we need to trim the list
            foundScriptGOTargets.resize(targetCount);
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
            return foundButOutOfRange ? SPELL_FAILED_OUT_OF_RANGE : SPELL_FAILED_BAD_TARGETS;
        }
        else if (foundButOutOfRange)
            return SPELL_FAILED_OUT_OF_RANGE;
    }
    return SPELL_CAST_OK;
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
            if (spellST->type == SPELL_TARGET_TYPE_GAMEOBJECT || spellST->type == SPELL_TARGET_TYPE_GAMEOBJECT_GUID)
                continue;

            bool fail = false;
            switch (spellST->type)
            {
                case SPELL_TARGET_TYPE_CREATURE_GUID:
                    fail = target->GetDbGuid() != spellST->targetEntry;
                    break;
                case SPELL_TARGET_TYPE_STRING_ID:
                    break;
                default:
                    fail = target->GetEntry() != spellST->targetEntry;
                    break;
            }
            if (fail)
                continue;

            switch (spellST->type)
            {
                case SPELL_TARGET_TYPE_DEAD:
                    if (static_cast<Creature*>(target)->IsCorpse())
                        targetUnitMap.push_back(target);
                    break;
                case SPELL_TARGET_TYPE_CREATURE:
                    if (target->IsAlive())
                        targetUnitMap.push_back(target);
                    break;
                case SPELL_TARGET_TYPE_CREATURE_GUID:
                    targetUnitMap.push_back(target);
                    break;
                case SPELL_TARGET_TYPE_STRING_ID:
                    if (target->HasStringId(spellST->targetEntry))
                        targetUnitMap.push_back(target);
                    break;
                default:
                    break;
            }
            break;
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
    if (!m_trueCaster)
        m_trueCaster = m_caster;
    m_spellState = SPELL_STATE_TARGETING;
    m_targets = *targets;

    if (triggeredByAura)
        m_triggeredByAuraSpell = triggeredByAura->GetSpellProto();

    // create and add update event for this spell
    m_spellEvent = new SpellEvent(this);
    m_trueCaster->m_events.AddEvent(m_spellEvent, m_trueCaster->m_events.CalculateTime(1));

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
    m_spellState = SPELL_STATE_CASTING;

    // Prepare data for triggers
    prepareDataForTriggerSystem();

    // calculate cast time (calculated after first CheckCast check to prevent charge counting for first CheckCast fail)
    if (!m_ignoreCastTime)
    {
        SpellModRAII spellModController(this, m_trueCaster->GetSpellModOwner(), false, true);
        m_casttime = GetSpellCastTime(m_spellInfo, m_trueCaster, this, true);
    }

    // set timer base at cast time
    ReSetTimer();

    if (!m_IsTriggeredSpell && !m_trueCaster->IsGameObject())
    {
        if (!m_spellInfo->HasAttribute(SPELL_ATTR_EX2_NOT_AN_ACTION))
            m_caster->RemoveAurasOnCast(AURA_INTERRUPT_FLAG_ACTION, m_spellInfo);

        // Orientation changes inside
        if (m_notifyAI && m_caster->AI())
            m_caster->AI()->OnSpellCastStateChange(this, true, m_targets.getUnitTarget());
    }

    m_castPositionX = m_trueCaster->GetPositionX();
    m_castPositionY = m_trueCaster->GetPositionY();
    m_castPositionZ = m_trueCaster->GetPositionZ();
    m_castOrientation = m_trueCaster->GetOrientation();

    OnSuccessfulStart();

    // Unsummon active Warlock demons when trying to summon a new one - vanilla only location
    if (Unit* unitCaster = dynamic_cast<Unit*>(m_trueCaster))
        if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_DISMISS_PET_FIRST))
            if (Pet* pet = unitCaster->GetPet())
                pet->Unsummon(PET_SAVE_NOT_IN_SLOT, unitCaster);

    // add non-triggered (with cast time and without)
    if (!m_IsTriggeredSpell)
    {
        if (!m_trueCaster->IsGameObject())
        {
            // add to cast type slot
            if ((!m_ignoreConcurrentCasts || IsChanneledSpell(m_spellInfo)) && !m_triggerAutorepeat)
                m_caster->SetCurrentCastedSpell(this);

            // add gcd server side (client side is handled by client itself)
            m_caster->AddGCD(*m_spellInfo);
        }

        // will show cast bar
        SendSpellStart();

        // Execute instant spells immediate
        if (m_timer == 0 && !IsNextMeleeSwingSpell(m_spellInfo) && (!IsAutoRepeat() || m_triggerAutorepeat))
            cast();
    }
    // execute triggered without cast time explicitly in call point
    else
    {
        // Channeled spell is always one per caster and needs to be tracked and removed on death
        if (IsChanneledSpell(m_spellInfo)) // GO casters cant cast channeled spells
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

    if (Player* player = m_caster->GetSpellModOwner()) // reset casting time mods
        if (player->GetSpellModSpell() != this && !m_usedAuraCharges.empty())
            player->ResetSpellModsDueToCanceledSpell(m_usedAuraCharges);

    m_autoRepeat = false;
    switch (m_spellState)
    {
        case SPELL_STATE_CREATED:
        case SPELL_STATE_TARGETING:
        case SPELL_STATE_CASTING:
            if (m_caster)
                m_caster->ResetGCD(m_spellInfo);
            // [[fallthrough]]
        case SPELL_STATE_TRAVELING:
            SendInterrupted(SPELL_FAILED_INTERRUPTED);
            if (sendInterrupt)
                SendCastResult(SPELL_FAILED_INTERRUPTED);
            break;
        case SPELL_STATE_LANDING:
            sLog.outError("Spell [%u] is interrupted while processing", m_spellInfo->Id);
            // [[fallthrough]]
        case SPELL_STATE_CHANNELING:
            for (auto& ihit : m_UniqueTargetInfo)
            {
                if (ihit.missCondition == SPELL_MISS_NONE)
                {
                    Unit* unit = m_caster->GetObjectGuid() == ihit.targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit.targetGUID);
                    if (unit && unit->IsAlive())
                        unit->RemoveAurasByCasterSpell(m_spellInfo->Id, m_caster->GetObjectGuid());
                }
            }
            if (m_spellInfo->HasAttribute(SPELL_ATTR_COOLDOWN_ON_EVENT))
                m_caster->RemoveSpellCooldown(*m_spellInfo, true); // need to clear cooldown in client

            SendChannelUpdate(0);
            SendInterrupted(SPELL_FAILED_INTERRUPTED);
            if (sendInterrupt)
                SendCastResult(SPELL_FAILED_INTERRUPTED);
            break;
        case SPELL_STATE_FINISHED: break; // should not occur
    }

    finish(false);
    if (m_caster)
    {
        m_caster->RemoveDynObject(m_spellInfo->Id);
        m_caster->RemoveGameObject(m_spellInfo->Id, true);
        m_caster->RemoveCreature(m_spellInfo->Id, true);
    }
}

SpellCastResult Spell::cast(bool skipCheck)
{
    SetExecutedCurrently(true);
    SpellModRAII spellModController(this, m_trueCaster->GetSpellModOwner());

    if (!m_trueCaster->CheckAndIncreaseCastCounter())
    {
        if (m_triggeredByAuraSpell)
            sLog.outError("Spell %u triggered by aura spell %u too deep in cast chain for cast. Cast not allowed for prevent overflow stack crash.", m_spellInfo->Id, m_triggeredByAuraSpell->Id);
        else
            sLog.outError("Spell %u too deep in cast chain for cast. Cast not allowed for prevent overflow stack crash.", m_spellInfo->Id);

        SendCastResult(SPELL_FAILED_ERROR);
        finish(false);
        SetExecutedCurrently(false);
        return SPELL_FAILED_ERROR;
    }

    // update pointers base at GUIDs to prevent access to already nonexistent object
    UpdatePointers();

    // cancel at lost main target unit
    if (!m_targets.getUnitTarget() && m_targets.getUnitTargetGuid() && m_targets.getUnitTargetGuid() != m_trueCaster->GetObjectGuid())
    {
        cancel();
        m_trueCaster->DecreaseCastCounter();
        SetExecutedCurrently(false);
        return SPELL_FAILED_ERROR;
    }

    if (m_trueCaster->IsCreature() && m_targets.getUnitTarget() && m_targets.getUnitTarget() != m_caster)
    {
        Unit* charmer = m_caster->GetCharmer();
        if (charmer && !(charmer->GetTypeId() == TYPEID_PLAYER && ((Player*)charmer)->GetCamera().GetBody() == m_caster)) // need to check if target doesnt have a player controlling it
            m_caster->SetInFront(m_targets.getUnitTarget());
    }

    // triggered cast called from Spell::prepare where it was already checked
    SpellCastResult castResult = SPELL_CAST_OK;
    if (!skipCheck)
    {
        castResult = CheckCast(false);
        if (castResult != SPELL_CAST_OK)
        {
            StopCast(castResult);
            return castResult;
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
        default:
            break;
    }

    // traded items have trade slot instead of guid in m_itemTargetGUID
    // set to real guid to be sent later to the client
    m_targets.updateTradeSlotItem();

    m_duration = CalculateSpellDuration(m_spellInfo, m_caster, nullptr, m_auraScript);

    FillTargetMap();

    if (m_spellState == SPELL_STATE_FINISHED)               // stop cast if spell marked as finish somewhere in FillTargetMap
    {
        m_trueCaster->DecreaseCastCounter();
        SetExecutedCurrently(false);
        return SPELL_FAILED_ERROR; // currently not propagating right error here but it should not be needed
    }

    spellModController.SetSuccess();

    // CAST SPELL
    SendSpellCooldown();
    if (m_notifyAI && m_caster && m_caster->AI())
        m_caster->AI()->OnSpellCooldownAdded(m_spellInfo);

    TakePower();
    TakeReagents();                                         // we must remove reagents before HandleEffects to allow place crafted item in same slot
    TakeAmmo();

    SendCastResult(castResult);
    SendSpellGo();                                          // we must send smsg_spell_go packet before m_castItem delete in TakeCastItem()...

    InitializeDamageMultipliers();

    OnCast();

    if (!m_IsTriggeredSpell && !m_trueCaster->IsGameObject() && !m_spellInfo->HasAttribute(SPELL_ATTR_EX2_NOT_AN_ACTION))
        m_caster->RemoveAurasOnCast(AURA_INTERRUPT_FLAG_ACTION_LATE, m_spellInfo);

    // process immediate effects (items, ground, etc.) also initialize some variables
    _handle_immediate_phase();

    Unit* procTarget = m_targets.getUnitTarget();
    if (!procTarget)
        procTarget = m_caster;

    // Okay, everything is prepared. Now we need to distinguish between immediate and evented delayed spells
    if (IsDelayedSpell())
    {
        // For channels, delay starts at channel end
        if (m_spellState != SPELL_STATE_CHANNELING)
        {
            // Okay, maps created, now prepare flags
            m_spellState = SPELL_STATE_TRAVELING;
            SetDelayStart(0);
            SetSpellStartTravelling(m_caster->GetMap()->GetCurrentMSTime());

            if (!m_spellInfo->HasAttribute(SPELL_ATTR_EX_NO_THREAT) &&
                !m_spellInfo->HasAttribute(SPELL_ATTR_EX_THREAT_ONLY_ON_MISS) &&
                !m_spellInfo->HasAttribute(SPELL_ATTR_EX2_NO_INITIAL_THREAT)) // attribute checks
            {
                if (m_caster && m_caster->IsPlayerControlled()) // only player casters
                {
                    if (Unit* target = m_targets.getUnitTarget())
                    {
                        for (auto const& ihit : m_UniqueTargetInfo)
                        {
                            if (target->GetObjectGuid() == ihit.targetGUID)                 // Found in list
                            {
                                if (m_caster->CanAttack(target)) // can attack
                                    if ((!IsPositiveEffectMask(m_spellInfo, ihit.effectMask, m_caster, target)
                                        && m_caster->IsVisibleForOrDetect(target, target, false)
                                        && m_caster->CanEnterCombat() && target->CanEnterCombat())) // can see and enter combat
                                    {
                                        m_caster->SetInCombatWithVictim(target);
                                        m_caster->GetCombatManager().TriggerCombatTimer(uint32(ihit.timeDelay + 500));
                                    }
                                break;
                            }
                        }
                    }
                }
            }
        }

        uint32 procAttacker;
        uint32 procVictim;
        uint32 procEx = PROC_EX_NONE;
        PrepareMasksForProcSystem(EFFECT_MASK_ALL, procAttacker, procVictim, m_trueCaster, nullptr);
        // on spell cast end proc,
        // critical hit related part is currently done on hit so proc there,
        // 0 damage since any damage based procs should be on hit
        // 0 victim proc since there is no victim proc dependent on successfull cast for caster
        Unit::ProcDamageAndSpell(ProcSystemArguments(m_caster, procTarget, procAttacker, 0, PROC_EX_CAST_END, 0, 0, m_attackType, m_spellInfo));
    }
    else // Immediate spell, no big deal
    {
        uint32 procAttacker;
        uint32 procVictim;
        uint32 procEx = PROC_EX_NONE;
        PrepareMasksForProcSystem(EFFECT_MASK_ALL, procAttacker, procVictim, m_trueCaster, nullptr);
        Unit::ProcDamageAndSpell(ProcSystemArguments(m_caster, procTarget, procAttacker, 0, PROC_EX_CAST_END, 0, 0, m_attackType, m_spellInfo));
        handle_immediate();
    }

    m_trueCaster->DecreaseCastCounter();
    SetExecutedCurrently(false);
    return SPELL_CAST_OK;
}

void Spell::handle_immediate()
{
    for (auto& ihit : m_UniqueTargetInfo)
        DoAllEffectOnTarget(&ihit);

    for (auto& ihit : m_uniqueCorpseTargetInfo)
        DoAllEffectOnTarget(&ihit);

    for (auto& ihit : m_UniqueGOTargetInfo)
        DoAllEffectOnTarget(&ihit);

    // spell is finished, perform some last features of the spell here
    _handle_finish_phase();

    if (m_spellState != SPELL_STATE_CHANNELING)
        finish(true);                                       // successfully finish spell cast (not last in case autorepeat or channel spell)
}

uint64 Spell::handle_delayed(uint64 t_offset)
{
    SpellModRAII spellModController(this, m_trueCaster->GetSpellModOwner(), true);

    uint64 next_time = 0;

    if (m_destTargetInfo.effectMaskProcessed != m_destTargetInfo.effectMask)
    {
        if (m_destTargetInfo.timeDelay <= t_offset)
            DoAllTargetlessEffects(true);
        else if (next_time == 0 || m_destTargetInfo.timeDelay < next_time)
            next_time = m_destTargetInfo.timeDelay;
    }

    // now recheck units targeting correctness (need before any effects apply to prevent adding immunity at first effect not allow apply second spell effect and similar cases)
    for (auto& ihit : m_UniqueTargetInfo)
    {
        if (!ihit.processed)
        {
            if (ihit.timeDelay <= t_offset)
            {
                if (ihit.procReflect)
                {
                    ihit.timeDelay = ihit.initialDelay / 2; // return it at twice the speed
                    ihit.procReflect = false;
                    ProcReflectProcs(ihit);
                    if (next_time == 0 || ihit.timeDelay < next_time)
                        next_time = ihit.timeDelay;
                }
                else
                    DoAllEffectOnTarget(&ihit);
            }
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
    m_spellState = SPELL_STATE_LANDING;

    if (IsMeleeAttackResetSpell())
    {
        if (!m_spellInfo->HasAttribute(SPELL_ATTR_EX2_DO_NOT_RESET_COMBAT_TIMERS))
        {
            m_caster->resetAttackTimer(BASE_ATTACK);
            if (m_caster->hasOffhandWeaponForAttack())
                m_caster->resetAttackTimer(OFF_ATTACK);
        }
    }

    // handle some immediate features of the spell here
    HandleThreatSpells();

    // handle none targeted effects
    DoAllTargetlessEffects(false);

    if (!IsDelayedSpell())
        DoAllTargetlessEffects(true);

    // process items
    for (auto& ihit : m_UniqueItemInfo)
        DoAllEffectOnTarget(&ihit);

    // take cast item after processing items
    TakeCastItem();

    // fill initial spell damage from caster for immediate effects
    for (auto& ihit : m_UniqueTargetInfo)
        HandleImmediateEffectExecution(&ihit);

    // process self immediately
    for (auto& ihit : m_UniqueTargetInfo)
    {
        if (ihit.targetGUID == m_trueCaster->GetObjectGuid())
        {
            DoAllEffectOnTarget(&ihit);
            break;
        }
    }

    ProcSpellAuraTriggers();

    // start channeling if applicable (after _handle_immediate_phase for get persistent effect dynamic object for channel target
    if (IsChanneledSpell(m_spellInfo) && m_duration)
    {
        m_spellState = SPELL_STATE_CHANNELING;
        SendChannelStart(m_duration);
    }
}

void Spell::_handle_finish_phase()
{
    // spell log
    if (m_needSpellLog)
        m_spellLog.SendToSet();
}

void Spell::SetCastItem(Item* item)
{
    m_CastItem = item;
    if (item)
        m_itemCastSpell = true;
}

void Spell::SendSpellCooldown()
{
    // (SPELL_ATTR_DISABLED_WHILE_ACTIVE) have infinity cooldown, (SPELL_ATTR_PASSIVE) passive cooldown at triggering
    if (m_spellInfo->HasAttribute(SPELL_ATTR_PASSIVE) || m_channelOnly)
        return;

    m_trueCaster->AddCooldown(*m_spellInfo, m_CastItem ? m_CastItem->GetProto() : nullptr, m_spellInfo->HasAttribute(SPELL_ATTR_COOLDOWN_ON_EVENT));
}

void Spell::update(uint32 difftime)
{
    if (!m_updated)
    {
        m_updated = true;
        if (m_creationTime == m_trueCaster->GetMap()->GetCurrentMSTime()) // do not update in the same tick as created
            return;
    }

    // update pointers based at it's GUIDs
    UpdatePointers();

    if (m_targets.getUnitTargetGuid() && !m_targets.getUnitTarget())
    {
        cancel();
        return;
    }

    // check if the player or unit caster has moved before the spell finished (exclude casting on vehicles)
    if ((m_trueCaster->IsUnit() && m_timer != 0) &&
            (m_castPositionX != m_trueCaster->GetPositionX() || m_castPositionY != m_trueCaster->GetPositionY() || m_castPositionZ != m_trueCaster->GetPositionZ()) &&
            (m_spellInfo->Effect[EFFECT_INDEX_0] != SPELL_EFFECT_STUCK || !m_trueCaster->m_movementInfo.HasMovementFlag(MOVEFLAG_FALLINGFAR)))
    {
        // always cancel for channeled spells
        if (m_spellState == SPELL_STATE_CHANNELING && m_spellInfo->ChannelInterruptFlags & AURA_INTERRUPT_FLAG_MOVING)
            cancel();
        // don't cancel for melee, autorepeat, triggered and instant spells
        else if (!IsNextMeleeSwingSpell(m_spellInfo) && !IsAutoRepeat() && !m_IsTriggeredSpell && (m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_MOVEMENT))
            cancel();
    }

    switch (m_spellState)
    {
        case SPELL_STATE_CASTING:
        {
            if (m_timer)
            {
                if (m_targets.getUnitTarget() && !m_IsTriggeredSpell && !IsAllowingDeadTarget(m_spellInfo) && !m_targets.getUnitTarget()->IsAlive())
                {
                    cancel();
                    return;
                }

                if (difftime >= m_timer)
                    m_timer = 0;
                else
                    m_timer -= difftime;
            }

            if (m_timer == 0 && !IsNextMeleeSwingSpell(m_spellInfo) && !IsAutoRepeat())
                cast();
        } break;
        case SPELL_STATE_CHANNELING:
        {
            if (m_timer > 0)
            {
                if (m_trueCaster->IsUnit())
                {
                    // check if player has jumped before the channeling finished
                    if (m_caster->m_movementInfo.HasMovementFlag(MOVEFLAG_JUMPING))
                    {
                        cancel();
                        return;
                    }

                    // check for incapacitating player states
                    if (m_caster->IsCrowdControlled())
                    {
                        // certain channel spells are not interrupted
                        if (!m_spellInfo->HasAttribute(SPELL_ATTR_EX_IS_CHANNELED) && !m_spellInfo->HasAttribute(SPELL_ATTR_EX3_IGNORE_CASTER_AND_TARGET_RESTRICTIONS))
                        {
                            cancel();
                            return;
                        }
                    }

                    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_SPECIAL_TAMING_FLAG)) // these fail on lost target attention (aggro)
                    {
                        if (Unit* target = m_caster->GetChannelObject())
                        {
                            Unit* targetsTarget = target->GetTarget();
                            if (targetsTarget && targetsTarget != m_caster)
                            {
                                cancel();
                                return;
                            }
                        }
                    }

                    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_TRACK_TARGET_IN_CHANNEL) && m_UniqueTargetInfo.begin() != m_UniqueTargetInfo.end())
                    {
                        if (Unit* target = m_caster->GetChannelObject())
                        {
                            if (target != m_caster)
                            {
                                float orientation = m_caster->GetAngle(target);
                                if (!m_caster->IsClientControlled())
                                {
                                    m_caster->SetOrientation(orientation);
                                    m_caster->SetFacingTo(orientation);
                                }
                                m_castOrientation = orientation; // need to update cast orientation due to the attribute
                            }
                        }
                    }

                    // check if player has turned if flag is set
                    if (m_spellInfo->ChannelInterruptFlags & AURA_INTERRUPT_FLAG_TURNING)
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
                                if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_TRACK_TARGET_IN_CHANNEL)) // On start, client turns player to face object, need leeway so that we dont break unnecessarily
                                    targetOrientationDiff += M_PI_F * (std::max(1200 - (m_duration - int32(m_timer)), 0) / 1200.f);
                                if ((M_PI_F - std::abs(std::abs(m_castOrientation - m_caster->GetOrientation()) - M_PI_F)) > targetOrientationDiff)
                                    cancel();
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
                                cancel();
                                return;
                            }
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
                if (!IsAutoRepeat() && !IsNextMeleeSwingSpell(m_spellInfo))
                {
                    if (Player* p = m_caster->GetBeneficiaryPlayer())
                    {
                        for (TargetList::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
                        {
                            TargetInfo const& target = *ihit;
                            if (!target.targetGUID.IsCreature())
                                continue;

                            Unit* unit = m_trueCaster->GetObjectGuid() == target.targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_trueCaster, target.targetGUID);
                            if (unit == nullptr)
                                continue;

                            p->RewardPlayerAndGroupAtCast(unit, m_spellInfo->Id);
                        }

                        for (GOTargetList::const_iterator ihit = m_UniqueGOTargetInfo.begin(); ihit != m_UniqueGOTargetInfo.end(); ++ihit)
                        {
                            GOTargetInfo const& target = *ihit;

                            GameObject* go = m_trueCaster->GetMap()->GetGameObject(target.targetGUID);
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
                    SetSpellStartTravelling(m_trueCaster->GetMap()->GetCurrentMSTime());
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
    if (!m_trueCaster)
        return;

    if (m_spellState == SPELL_STATE_FINISHED)
        return;

    bool channeledChannel = m_spellState == SPELL_STATE_CHANNELING;

    m_spellState = SPELL_STATE_FINISHED;

    if (m_notifyAI && m_caster && m_caster->AI())
        m_caster->AI()->OnSpellCastStateChange(this, false);

    // other code related only to successfully finished spells
    if (!ok)
        return;

    // Heal caster for all health leech from all targets
    if (m_healthLeech)
        m_caster->DealHeal(m_caster, uint32(m_healthLeech), m_spellInfo);

    if (m_spellInfo->AttributesEx & SPELL_ATTR_EX_DISCOUNT_POWER_ON_MISS)
    {
        for (TargetList::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
        {
            switch (ihit->missCondition)
            {
                case SPELL_MISS_MISS:
                case SPELL_MISS_DODGE:
                case SPELL_MISS_PARRY:
                case SPELL_MISS_DEFLECT:
                    m_caster->ModifyPower(Powers(m_spellInfo->powerType), int32(float(m_powerCost) * 0.8f));
                    break;
                default:
                    break;
            }
        }
    }

    OnSuccessfulFinish();

    // Clear combo at finish state
    if (m_trueCaster->IsPlayer() && NeedsComboPoints(m_spellInfo))
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
            m_caster->ClearComboPoints();
    }

    // call triggered spell only at successful cast (after clear combo points -> for add some if need)
    if (!m_TriggerSpells.empty())
        CastTriggerSpells();

    // Stop Attack for some spells
    if (m_caster && m_spellInfo->HasAttribute(SPELL_ATTR_CANCELS_AUTO_ATTACK_COMBAT))
    {
        if (IsRangedSpell()) // blizzlike order
            m_caster->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);

        m_caster->AttackStop();

        if (m_caster->IsClientControlled())
            const_cast<Player*>(m_caster->GetClientControlling())->SendAttackSwingCancelAttack();
    }

    // update encounter state if needed
    if (m_trueCaster->IsInWorld()) // some teleport spells put caster in between maps, need to check
    {
        Map* map = m_trueCaster->GetMap();
        if (map->IsDungeon())
            ((DungeonMap*)map)->GetPersistanceState()->UpdateEncounterState(ENCOUNTER_CREDIT_CAST_SPELL, m_spellInfo->Id);
    }
}

void Spell::SendCastResult(SpellCastResult result) const
{
    if (m_triggeredByAuraSpell) // should not report these
        return;

    Player const* recipient;
    if (!m_trueCaster->IsPlayer())
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

    uint32 param1 = m_param1, param2 = m_param2;
    if (m_spellScript)
        m_spellScript->OnSpellCastResultOverride(result, param1, param2);
    SendCastResult(recipient, m_spellInfo, result, m_petCast, param1, param2);
}

void Spell::SendCastResult(Player const* caster, SpellEntry const* spellInfo, SpellCastResult result, bool isPetCastResult /*=false*/, uint32 param1 /*=0*/, uint32 param2 /*=0*/)
{
    WorldPacket data(SMSG_CAST_RESULT, (4 + 1 + 1));
    data << uint32(spellInfo->Id);

    if (result != SPELL_CAST_OK && !spellInfo->HasAttribute(SPELL_ATTR_EX2_DO_NOT_REPORT_SPELL_FAILURE))
    {
        data << uint8(2); // status = fail
        data << uint8(!IsPassiveSpell(spellInfo) ? result : SPELL_FAILED_DONT_REPORT); // do not report failed passive spells
        switch (result)
        {
            case SPELL_FAILED_NOT_READY:
                if (spellInfo->HasAttribute(SPELL_ATTR_COOLDOWN_ON_EVENT))
                    data << uint32(caster->IsSpellOnPermanentCooldown(*spellInfo));
                break;
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
            case SPELL_FAILED_TOTEMS:
                for (uint32 i : spellInfo->Totem)
                    if (i)
                        data << uint32(i);
                break;
            case SPELL_FAILED_EQUIPPED_ITEM_CLASS:
            case SPELL_FAILED_EQUIPPED_ITEM_CLASS_MAINHAND:
            case SPELL_FAILED_EQUIPPED_ITEM_CLASS_OFFHAND:
                data << uint32(spellInfo->EquippedItemClass);
                data << uint32(spellInfo->EquippedItemSubClassMask);
                break;
            case SPELL_FAILED_NEED_EXOTIC_AMMO:
                data << uint32(spellInfo->EquippedItemSubClassMask);
                break;
            case SPELL_FAILED_REAGENTS:
                data << param1;                                 // item id
                break;
            case SPELL_FAILED_PREVENTED_BY_MECHANIC:
                data << param1;
                break;
            case SPELL_FAILED_MIN_SKILL:
                data << uint32(0);                              // required skill value
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
    if (m_triggeredByAuraSpell || m_hideInCombatLog)
        castFlags |= CAST_FLAG_HIDDEN_COMBATLOG;

    if (IsRangedSpell())
        castFlags |= CAST_FLAG_AMMO;

    WorldPacket data(SMSG_SPELL_START, (8 + 8 + 4 + 2 + 4));
    if (m_CastItem)
        data << m_CastItem->GetPackGUID();
    else
        data << m_trueCaster->GetPackGUID();

    if (m_trueCaster->IsGameObject()) // write empty guid if GO
        data << ObjectGuid().WriteAsPacked();
    else
        data << m_caster->GetPackGUID();

    data << uint32(m_spellInfo->Id);                        // spellId
    data << uint16(castFlags);                              // cast flags
    data << uint32(m_timer);                                // delay?

    data << m_targets;

    if (castFlags & CAST_FLAG_AMMO)                         // projectile info
        WriteAmmoToPacket(data);

    m_trueCaster->SendMessageToSet(data, true);
}

void Spell::SendSpellGo()
{
    // not send invisible spell casting
    if (!IsNeedSendToClient())
        return;

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Sending SMSG_SPELL_GO id=%u", m_spellInfo->Id);

    uint32 castFlags = CAST_FLAG_UNKNOWN9;
    if (m_triggeredByAuraSpell || m_hideInCombatLog)
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
    else if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_RETAIN_ITEM_CAST) && m_forwardedCastItemGuid)
        data << m_forwardedCastItemGuid.WriteAsPacked();
    else
        data << m_trueCaster->GetPackGUID();

    if (m_trueCaster->IsGameObject()) // write empty guid if GO
        data << ObjectGuid().WriteAsPacked();
    else
        data << m_caster->GetPackGUID();
    data << uint32(m_spellInfo->Id);                        // spellId
    data << uint16(castFlags);                              // cast flags

    WriteSpellGoTargets(data);

    data << m_targets;

    if (castFlags & CAST_FLAG_AMMO)                         // projectile info
        WriteAmmoToPacket(data);

    m_trueCaster->SendMessageToSet(data, true);
}

void Spell::WriteAmmoToPacket(WorldPacket& data) const
{
    uint32 ammoInventoryType = 0;
    uint32 ammoDisplayID = 0;

    if (m_trueCaster->IsPlayer())
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

    if (m_UniqueTargetInfo.size() > 255) // 53010 confirmed to do this legally
        sLog.outError("Spell ID %u cast by %s hit/missed too many unit targets %u. General warning. Only 255 sent", m_spellInfo->Id, m_caster->GetObjectGuid().GetString().c_str(), (uint32)m_UniqueTargetInfo.size());
    else if (m_UniqueGOTargetInfo.size() > 255)
        sLog.outError("Spell ID %u cast by %s hit/missed too many GO targets %u. General warning. Only 255 sent", m_spellInfo->Id, m_caster->GetObjectGuid().GetString().c_str(), (uint32)m_UniqueGOTargetInfo.size());
    else if (m_UniqueGOTargetInfo.size() + m_UniqueTargetInfo.size() > 255)
        sLog.outError("Spell ID %u cast by %s hit/missed too many targets %u. General warning. Only 255 sent", m_spellInfo->Id, m_caster->GetObjectGuid().GetString().c_str(), (uint32)(m_UniqueGOTargetInfo.size() + m_UniqueTargetInfo.size()));

    // This function also fill data for channeled spells:
    // m_needAliveTargetMask req for stop channeling if one target die
    uint32 hit = m_UniqueGOTargetInfo.size();              // Always hits on GO
    uint32 miss = 0;

    for (auto& ihit : m_UniqueTargetInfo)
    {
        if (ihit.effectHitMask == 0 && ihit.effectMask != 0) // No effect apply - all immuned add state
        {
            // possibly SPELL_MISS_IMMUNE2 for this??
            if (IsChanneledSpell(m_spellInfo) && ihit.targetGUID == m_targets.getUnitTargetGuid()) // can happen due to DR
            {
                m_duration = 0;                              // cancel aura to avoid visual effect continue
                ihit.effectDuration = 0;
            }
            ihit.missCondition = SPELL_MISS_IMMUNE2;
            ++miss;
        }
        else if (ihit.missCondition == SPELL_MISS_NONE)    // Add only hits
        {
            if (hit < 255)
            {
                ++hit;
                data << ihit.targetGUID;
                m_needAliveTargetMask |= ihit.effectHitMask;
            }
        }
        else
        {
            if (IsChanneledSpell(m_spellInfo) && (ihit.missCondition == SPELL_MISS_RESIST || ihit.missCondition == SPELL_MISS_REFLECT) && ihit.targetGUID == m_targets.getUnitTargetGuid())
            {
                m_duration = 0;                              // cancel aura to avoid visual effect continue
                ihit.effectDuration = 0;
            }
            ++miss;
        }
    }

    for (auto& ighit : m_UniqueGOTargetInfo)
        data << ighit.targetGUID;                         // Always hits

    hit = std::min(hit, 255u);
    data.put<uint8>(count_pos, hit);

    miss = std::min(miss, 255u);
    data << (uint8)miss;

    uint32 missCounter = 0;
    for (auto& ihit : m_UniqueTargetInfo)
    {
        if (ihit.missCondition != SPELL_MISS_NONE)         // Add only miss
        {
            ++missCounter;
            data << ihit.targetGUID;
            data << uint8(ihit.missCondition);
            if (ihit.missCondition == SPELL_MISS_REFLECT)
                data << uint8(ihit.reflectResult);

            if (missCounter >= 255)
                break;
        }
    }
    // Reset m_needAliveTargetMask for non channeled spell
    if (!IsChanneledSpell(m_spellInfo))
        m_needAliveTargetMask = 0;
}

void Spell::SendInterrupted(SpellCastResult result) const
{
    WorldPacket data(SMSG_SPELL_FAILURE, (8 + 4 + 1));
    data << m_trueCaster->GetPackGUID();
    data << m_spellInfo->Id;
    data << uint8(result);
    m_trueCaster->SendMessageToSet(data, true);

    data.Initialize(SMSG_SPELL_FAILED_OTHER, (8 + 4));
    data << m_trueCaster->GetObjectGuid();
    data << m_spellInfo->Id;
    m_trueCaster->SendMessageToSet(data, true);
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

    uint32 diminishDuration = duration;
    DiminishingGroup diminishGroup = DIMINISHING_NONE;
    DiminishingLevels diminishLevel = DIMINISHING_LEVEL_1;

    // select dynobject created by first effect if any
    if (m_spellInfo->Effect[EFFECT_INDEX_0] == SPELL_EFFECT_PERSISTENT_AREA_AURA)
        target = m_caster->GetDynObject(m_spellInfo->Id, EFFECT_INDEX_0);
    // select first not resisted target from target list for _0_ effect
    else if (!m_UniqueTargetInfo.empty())
    {
        for (TargetList::const_iterator itr = m_UniqueTargetInfo.begin(); itr != m_UniqueTargetInfo.end(); ++itr)
        {
            if (((itr->effectHitMask & (1 << EFFECT_INDEX_0)) && itr->reflectResult == SPELL_MISS_NONE) || itr->targetGUID != m_caster->GetObjectGuid())
            {
                // when immune, duration is already 0, still need to fetch data for caster
                if (itr->diminishGroup > DIMINISHING_NONE && (itr->diminishDuration != duration || diminishLevel == DIMINISHING_LEVEL_IMMUNE))
                {
                    diminishDuration = itr->diminishDuration;
                    diminishGroup = itr->diminishGroup;
                    diminishLevel = itr->diminishLevel;
                }
                target = ObjectAccessor::GetUnit(*m_caster, itr->targetGUID);
                if (m_spellInfo->EffectRadiusIndex[EFFECT_INDEX_0] != 0 &&
                    (m_spellInfo->EffectApplyAuraName[EFFECT_INDEX_0] == SPELL_AURA_MOD_POSSESS || m_spellInfo->EffectApplyAuraName[EFFECT_INDEX_0] == SPELL_AURA_BIND_SIGHT))
                    m_maxRange = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[EFFECT_INDEX_0]));
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
    else if (m_spellInfo->Id == 27360) // override for research - is not used in targeting but supplied as unittarget
    {
        target = m_targets.getUnitTarget();
    }

    // change channel duration to account for DR if neccessary, also store in caster's targetInfo to use later for setting aura duration
    if (diminishDuration != duration || diminishLevel == DIMINISHING_LEVEL_IMMUNE)
    {
        duration = diminishDuration;
        for (auto& target : m_UniqueTargetInfo)
        {
            if (target.targetGUID == m_caster->GetObjectGuid())
            {
                target.diminishDuration = diminishDuration;
                target.diminishGroup = diminishGroup;
                target.diminishLevel = diminishLevel;
                break;
            }
        }
    }

    if (m_caster->IsPlayer())
    {
        WorldPacket data(MSG_CHANNEL_START, (4 + 4));
        data << uint32(m_spellInfo->Id);
        data << uint32(duration);
        ((Player*)m_caster)->SendDirectMessage(data);
    }

    m_timer = duration;

    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_IS_CHANNELED))
    {
        WorldPacket data(SMSG_SPELL_UPDATE_CHAIN_TARGETS);
        data << m_caster->GetObjectGuid();
        data << uint32(m_spellInfo->Id);
        size_t count_pos = data.wpos();
        data << uint32(0);
        uint32 hit = 0;
        for (TargetList::const_iterator itr = m_UniqueTargetInfo.begin(); itr != m_UniqueTargetInfo.end(); ++itr)
        {
            if (((itr->effectHitMask & (1 << EFFECT_INDEX_0)) && itr->reflectResult == SPELL_MISS_NONE &&
                m_CastItem) || itr->targetGUID != m_caster->GetObjectGuid())
            {
                if (Unit* target = ObjectAccessor::GetUnit(*m_caster, itr->targetGUID))
                {
                    ++hit;
                    data << target->GetObjectGuid();
                }
            }
        }
        if (hit)
        {
            data.put<uint32>(count_pos, hit);
            m_caster->SendMessageToSet(data, true);
        }
    }

    if (target)
        m_caster->SetChannelObject(target);

    m_caster->SetUInt32Value(UNIT_CHANNEL_SPELL, m_spellInfo->Id);
    m_caster->addUnitState(UNIT_STAT_CHANNELING);

    if (m_caster->AI())
        m_caster->AI()->OnChannelStateChange(this, true, target);
}

void Spell::TakeCastItem()
{
    if (!m_CastItem || !m_trueCaster->IsPlayer())
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

    // these effects change the item
    if (IsSpellHaveEffect(m_spellInfo, SPELL_EFFECT_SUMMON_CHANGE_ITEM))
        return;

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
                    m_CastItem->SetState(ITEM_CHANGED, static_cast<Player*>(m_caster));
                }

                // all charges used
                withoutCharges = (charges == 0);
            }
        }
    }

    if (expendable && withoutCharges)
    {
        if (m_CastItem)
            m_CastItem->SetUsedInSpell(false);
        uint32 count = 1;
        static_cast<Player*>(m_caster)->DestroyItemCount(m_CastItem, count, true);

        // prevent crash at access to deleted m_targets.getItemTarget
        ClearCastItem();
    }
}

void Spell::TakePower()
{
    if (m_CastItem || m_triggeredByAuraSpell || !m_caster)
        return;

    // health as power used
    if (m_spellInfo->powerType == POWER_HEALTH)
    {
        m_caster->ModifyHealth(-(int32)m_powerCost);
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
    if (powerType == POWER_MANA && m_powerCost > 0 && !m_spellInfo->HasAttribute(SPELL_ATTR_EX2_DONT_BLOCK_MANA_REGEN))
        m_caster->SetLastManaUse();
}

void Spell::TakeAmmo() const
{
    if (m_attackType == RANGED_ATTACK && m_caster->IsPlayer())
    {
        Player* player = static_cast<Player*>(m_caster);
        Item* pItem = player->GetWeaponForAttack(RANGED_ATTACK, true, false);

        // wands don't have ammo
        if (!pItem || pItem->GetProto()->SubClass == ITEM_SUBCLASS_WEAPON_WAND)
            return;

        if (pItem->GetProto()->InventoryType == INVTYPE_THROWN)
        {
            if (pItem->GetMaxStackCount() == 1)
            {
                // decrease durability for non-stackable throw weapon
                player->DurabilityPointLossForEquipSlot(EQUIPMENT_SLOT_RANGED);
            }
            else
            {
                // decrease items amount for stackable throw weapon
                uint32 count = 1;
                player->DestroyItemCount(pItem, count, true);
            }
        }
        else if (uint32 ammo = player->GetUInt32Value(PLAYER_AMMO_ID))
            player->DestroyItemCount(ammo, 1, true);
    }
}


void Spell::TakeReagents()
{
    if (!m_trueCaster->IsPlayer())
        return;

    if (IgnoreItemRequirements())                           // reagents used in triggered spell removed by original spell or don't must be removed.
        return;

    Player* p_caster = static_cast<Player*>(m_caster);
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

                m_CastItem->SetUsedInSpell(false);
                m_CastItem = nullptr;
            }
        }

        // if getItemTarget is also spell reagent
        if (m_targets.getItemTargetEntry() == itemid)
            m_targets.clearItemPointer();

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

    Unit* affectiveCaster = GetAffectiveCasterOrOwner();
    if (!affectiveCaster) // GO cast spells do not need threat processing
        return;

    float threat = threatEntry->threat;
    if (threatEntry->ap_bonus != 0.0f)
        threat += threatEntry->ap_bonus * affectiveCaster->GetTotalAttackPowerValue(GetWeaponAttackType(m_spellInfo));

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

        Unit* target = m_trueCaster->GetObjectGuid() == ihit->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_trueCaster, ihit->targetGUID);
        if (!target)
            continue;

        // positive spells distribute threat among all units that are in combat with target, like healing
        if (positive)
        {
            target->getHostileRefManager().threatAssist(affectiveCaster, threat, m_spellInfo, false, true);
        }
        // for negative spells threat gets distributed among affected targets
        else
        {
            if (!target->CanHaveThreatList())
                continue;

            target->AddThreat(affectiveCaster, threat, false, GetSpellSchoolMask(m_spellInfo), m_spellInfo);
        }
    }

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Spell %u added an additional %f threat for %s " SIZEFMTD " target(s)", m_spellInfo->Id, threat, positive ? "assisting" : "harming", m_UniqueTargetInfo.size());
}

void Spell::ExecuteEffects(Unit* unitTarget, Item* itemTarget, GameObject* GOTarget, uint32 effectMask)
{
    Unit* affectiveCaster = GetAffectiveCaster();
    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (effectMask & (1 << i))
        {
            HandleEffect(unitTarget, itemTarget, GOTarget, SpellEffectIndex(i), m_damageMultipliers[i]);
            if (m_applyMultiplierMask & (1 << i))
            {
                // Get multiplier
                float multiplier = m_spellInfo->DmgMultiplier[i];
                // Apply multiplier mods
                if (affectiveCaster)
                    if (Player* modOwner = affectiveCaster->GetSpellModOwner())
                        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_EFFECT_PAST_FIRST, multiplier);
                m_damageMultipliers[i] *= multiplier;
            }
        }
    }
}

void Spell::HandleEffect(Unit* pUnitTarget, Item* pItemTarget, GameObject* pGOTarget, SpellEffectIndex i, float DamageMultiplier)
{
    if (m_effectTriggerChance[i] != -1)
    {
        if (m_effectTriggerChance[i] == 0 || irand(1, 100) > m_effectTriggerChance[i])
            return;
    }

    unitTarget = pUnitTarget;
    itemTarget = pItemTarget;
    gameObjTarget = pGOTarget;

    uint8 eff = m_spellInfo->Effect[i];

    if (IsEffectWithImplementedMultiplier(eff))
    {
        m_healingPerEffect[i] = 0;
        m_damagePerEffect[i] = 0;
        damage = CalculateSpellEffectValue(i, unitTarget);
    }
    else
        damage = int32(CalculateSpellEffectValue(i, unitTarget) * DamageMultiplier);

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Spell %u Effect%d : %u Targets: %s, %s, %s",
                     m_spellInfo->Id, i, eff,
                     unitTarget ? unitTarget->GetGuidStr().c_str() : "-",
                     itemTarget ? itemTarget->GetGuidStr().c_str() : "-",
                     gameObjTarget ? gameObjTarget->GetGuidStr().c_str() : "-");

    if (eff < MAX_SPELL_EFFECTS)
    {
        OnEffectExecute(i);
        damagePerEffect[i] = damage;
        (*this.*SpellEffects[eff])(i);
    }
    else
        sLog.outError("WORLD: Spell FX %d > MAX_SPELL_EFFECTS ", eff);

    if (IsEffectWithImplementedMultiplier(eff))
    {
        if (m_healingPerEffect[i] > 0)
            m_healing += int32(m_healingPerEffect[i] * DamageMultiplier);
        else if (m_damagePerEffect[i] > 0)
            m_damage += int32(m_damagePerEffect[i] * DamageMultiplier);
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
        Spell* spell = new Spell(m_trueCaster, (*si), TRIGGERED_OLD_TRIGGERED, m_originalCasterGUID);
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
            return m_trueCaster->GetMap()->GetUnit(itr.targetGUID);

    return m_targets.getUnitTarget();
}

SpellCastResult Spell::CheckCast(bool strict)
{
    // check cooldowns to prevent cheating (ignore passive spells, that client side visual only)
    if (!m_ignoreCooldowns && !m_spellInfo->HasAttribute(SPELL_ATTR_PASSIVE)
            && !m_trueCaster->IsSpellReady(*m_spellInfo, m_CastItem ? m_CastItem->GetProto() : nullptr))
    {
        if (m_triggeredByAuraSpell)
            return SPELL_FAILED_DONT_REPORT;
        else
            return SPELL_FAILED_NOT_READY;
    }

    // check global cooldown
    if (strict && !m_ignoreGCD && m_trueCaster->HasGCD(m_spellInfo))
        return SPELL_FAILED_NOT_READY;

    if (m_caster)
    {
        if (!m_caster->IsAlive() && m_caster->GetTypeId() == TYPEID_PLAYER && !m_spellInfo->HasAttribute(SPELL_ATTR_ALLOW_CAST_WHILE_DEAD) && !m_spellInfo->HasAttribute(SPELL_ATTR_PASSIVE))
            return SPELL_FAILED_CASTER_DEAD;

        if (!m_IsTriggeredSpell && !m_caster->IsStandState() && m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && !m_spellInfo->HasAttribute(SPELL_ATTR_ALLOW_WHILE_SITTING))
            return SPELL_FAILED_NOT_STANDING;

        // only allow triggered spells if at an ended battleground
        if (!m_IsTriggeredSpell && m_caster->IsPlayer())
            if (BattleGround* bg = static_cast<Player*>(m_caster)->GetBattleGround())
                if (bg->GetStatus() == STATUS_WAIT_LEAVE)
                    return SPELL_FAILED_DONT_REPORT;

        if ((!m_IsTriggeredSpell || m_triggeredByAuraSpell) && IsNonCombatSpell(m_spellInfo) && m_caster->IsInCombat())
            return SPELL_FAILED_AFFECTING_COMBAT;

        if (m_caster->GetTypeId() == TYPEID_PLAYER && !((Player*)m_caster)->IsGameMaster() &&
            sWorld.getConfig(CONFIG_BOOL_VMAP_INDOOR_CHECK) &&
            VMAP::VMapFactory::createOrGetVMapManager()->isLineOfSightCalcEnabled())
        {
            if (m_spellInfo->HasAttribute(SPELL_ATTR_ONLY_OUTDOORS) &&
                !m_caster->GetTerrain()->IsOutdoors(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ()))
                return SPELL_FAILED_ONLY_OUTDOORS; // TODO: If at least one effect is SPELL_AURA_MOUNTED return mounts not allowed

            if (m_spellInfo->HasAttribute(SPELL_ATTR_ONLY_INDOORS) &&
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

        if (!m_IsTriggeredSpell && NeedsComboPoints(m_spellInfo) && (!m_targets.getUnitTarget() || m_targets.getUnitTarget()->GetObjectGuid() != m_caster->GetComboTargetGuid()))
            // warrior not have real combo-points at client side but use this way for mark allow Overpower use
            return m_caster->getClass() == CLASS_WARRIOR ? SPELL_FAILED_CASTER_AURASTATE : SPELL_FAILED_NO_COMBO_POINTS;
    }

    if (m_trueCaster->IsPlayer())
    {
        // cancel autorepeat spells if cast start when moving
        // (not wand currently autorepeat cast delayed to moving stop anyway in spell update code)
        if (m_caster->IsMoving())
        {
            // skip stuck spell to allow use it in falling case and apply spell limitations at movement
            if ((!m_caster->m_movementInfo.HasMovementFlag(MOVEFLAG_FALLINGFAR) || m_spellInfo->Effect[EFFECT_INDEX_0] != SPELL_EFFECT_STUCK) &&
                    (IsAutoRepeat() || (m_spellInfo->AuraInterruptFlags & AURA_INTERRUPT_FLAG_STANDING_CANCELS) != 0))
                return SPELL_FAILED_MOVING;
        }

        if (m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            // cancel autorepeat spells if cast start when moving
            // (not wand currently autorepeat cast delayed to moving stop anyway in spell update code)
            if (m_caster->IsMoving())
            {
                // skip stuck spell to allow use it in falling case and apply spell limitations at movement
                if ((!m_caster->m_movementInfo.HasMovementFlag(MOVEFLAG_FALLINGFAR) || m_spellInfo->Effect[EFFECT_INDEX_0] != SPELL_EFFECT_STUCK) &&
                    (IsAutoRepeat() || (m_spellInfo->AuraInterruptFlags & AURA_INTERRUPT_FLAG_STANDING_CANCELS) != 0))
                    return SPELL_FAILED_MOVING;
            }

            // Loatheb Corrupted Mind and Nefarian class calls spell failed
            switch (m_spellInfo->SpellFamilyName)
            {
                case SPELLFAMILY_DRUID:
                {
                    if (IsSpellHaveAura(m_spellInfo, SPELL_AURA_MOD_SHAPESHIFT))
                        if (m_caster->HasOverrideScript(3655))
                            return SPELL_FAILED_TARGET_AURASTATE;
                    //[[fallthrough]]
                }
                case SPELLFAMILY_PRIEST:
                case SPELLFAMILY_SHAMAN:
                case SPELLFAMILY_PALADIN:
                {
                    if (IsSpellHaveEffect(m_spellInfo, SPELL_EFFECT_HEAL) ||
                        IsSpellHaveAura(m_spellInfo, SPELL_AURA_PERIODIC_HEAL) ||
                        IsSpellHaveEffect(m_spellInfo, SPELL_EFFECT_DISPEL))
                    {
                        if (m_caster->HasOverrideScript(4327))
                            return SPELL_FAILED_FIZZLE;
                    }
                    break;
                }
                case SPELLFAMILY_WARRIOR:
                {
                    if (IsSpellHaveAura(m_spellInfo, SPELL_AURA_MOD_SHAPESHIFT))
                    {
                        if (m_caster->HasOverrideScript(3654))
                            return SPELL_FAILED_TARGET_AURASTATE;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    Unit* target = m_targets.getUnitTarget();
    uint32 affectedMask = GetCheckCastEffectMask(m_spellInfo);

    bool selfTargeting = false;
    if (!target)
    {
        uint32 selfImmuneMask = GetCheckCastSelfEffectMask(m_spellInfo);
        if (selfImmuneMask)
        {
            target = m_caster;
            affectedMask = selfImmuneMask;
            selfTargeting = true;
        }
    }

    if (target)
    {
        // TARGET_UNIT_SCRIPT_NEAR_CASTER fills unit target per client requirement but should not be checked against common things
        // TODO: Find a nicer and more efficient way to check for this
        if (!IsSpellWithScriptUnitTarget(m_spellInfo))
        {
            bool ignoreRestrictions = m_spellInfo->HasAttribute(SPELL_ATTR_EX3_IGNORE_CASTER_AND_TARGET_RESTRICTIONS);
            if (!m_IsTriggeredSpell && IsDeathOnlySpell(m_spellInfo) && target->IsAlive())
                return SPELL_FAILED_TARGET_NOT_DEAD;

            // totem immunity for channeled spells(needs to be before spell cast)
            // spell attribs for player channeled spells
            if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_TRACK_TARGET_IN_CHANNEL)
                && target->GetTypeId() == TYPEID_UNIT && ((Creature*)target)->IsTotem())
                return SPELL_FAILED_IMMUNE;

            bool non_caster_target = target != m_trueCaster && !IsSpellWithCasterSourceTargetsOnly(m_spellInfo);

            if (non_caster_target)
            {
                // Not allow casting on flying player
                if (!ignoreRestrictions && !m_spellInfo->HasAttribute(SPELL_ATTR_ALLOW_WHILE_MOUNTED) && target->IsTaxiFlying())
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

                if (!IsIgnoreLosSpell(m_spellInfo) && !m_IsTriggeredSpell && !m_trueCaster->IsWithinLOSInMap(target, true))
                    return SPELL_FAILED_LINE_OF_SIGHT;

                if (m_trueCaster->IsPlayer())
                {
                    // auto selection spell rank implemented in WorldSession::HandleCastSpellOpcode
                    // this case can be triggered if rank not found (too low-level target for first rank)
                    if (!m_CastItem && !m_IsTriggeredSpell && !m_spellInfo->HasAttribute(SPELL_ATTR_EX2_ALLOW_LOW_LEVEL_BUFF))
                        // spell expected to be auto-downranking in cast handle, so must be same
                        if (m_spellInfo != sSpellMgr.SelectAuraRankForLevel(m_spellInfo, target->GetLevel()))
                            return SPELL_FAILED_LOWLEVEL;

                    // Do not allow these spells to target creatures not tapped by us (Banish, Polymorph, many quest spells)
                    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_CANNOT_CAST_ON_TAPPED))
                        if (Creature const* targetCreature = dynamic_cast<Creature*>(target))
                            if ((!targetCreature->GetLootRecipientGuid().IsEmpty()) && !targetCreature->IsTappedBy(static_cast<Player*>(m_trueCaster)))
                                return SPELL_FAILED_CANT_CAST_ON_TAPPED;
                    
                    // Do not allow spells to complete which are targeting players that are invisible to the caster since the time of cast start
                    if (!m_trueCaster->IsGameObject() && target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && !IsPositiveEffectMask(m_spellInfo, affectedMask, m_trueCaster, target) && !target->IsVisibleForOrDetect(m_caster, m_trueCaster, false))
                        return SPELL_FAILED_BAD_TARGETS;
                }

                if (strict && m_spellInfo->HasAttribute(SPELL_ATTR_EX3_ONLY_ON_PLAYER) && target->GetTypeId() != TYPEID_PLAYER && !IsAreaOfEffectSpell(m_spellInfo))
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

            if (!selfTargeting && (m_targets.m_targetMask == TARGET_FLAG_SELF || m_trueCaster == target) && m_spellInfo->HasAttribute(SPELL_ATTR_EX_EXCLUDE_CASTER))
            {
                if (IsOnlySelfTargeting(m_spellInfo))
                    sLog.outCustomLog("Spell ID %u cast at self explicitly even though it has SPELL_ATTR_EX_EXCLUDE_CASTER", m_spellInfo->Id);

                return SPELL_FAILED_BAD_TARGETS;
            }

            if (!selfTargeting && !ignoreRestrictions && target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNTARGETABLE))
                return SPELL_FAILED_BAD_TARGETS;

            // check creature type
            // ignore self casts (including area casts when caster selected as target)
            if (non_caster_target)
            {
                if (!CheckTargetCreatureType(target, m_spellInfo))
                {
                    if (target->GetTypeId() == TYPEID_PLAYER)
                        return SPELL_FAILED_TARGET_IS_PLAYER;
                    return SPELL_FAILED_BAD_TARGETS;
                }
            }

            if (!ignoreRestrictions && IsPositiveSpell(m_spellInfo->Id, m_trueCaster, target) && affectedMask)
                if (target->IsImmuneToSpell(m_spellInfo, target == m_trueCaster, affectedMask, m_trueCaster))
                    return SPELL_FAILED_TARGET_AURASTATE;

            // Caster must be facing the targets back
            if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_INITIATE_COMBAT_POST_CAST) && m_spellInfo->HasAttribute(SPELL_ATTR_EX_INITIATES_COMBAT_ENABLES_AUTO_ATTACK) && !m_trueCaster->IsFacingTargetsBack(target))
            {
                // Exclusion for Pounce: Facing Limitation was removed in 2.0.1, but it still uses the same, old Ex-Flags
                if (!m_spellInfo->IsFitToFamily(SPELLFAMILY_DRUID, uint64(0x0000000000020000)))
                    return SPELL_FAILED_NOT_BEHIND;
            }

            // duplicate block to avoid previous block complex logic
            if (m_spellInfo->HasAttribute(SPELL_ATTR_SS_FACING_BACK) && !m_trueCaster->IsFacingTargetsBack(target))
                return SPELL_FAILED_NOT_BEHIND;

            // Caster must be facing the targets front
            if (((m_spellInfo->Attributes == (SPELL_ATTR_IS_ABILITY | SPELL_ATTR_NOT_SHAPESHIFT | SPELL_ATTR_DO_NOT_SHEATH | SPELL_ATTR_CANCELS_AUTO_ATTACK_COMBAT)) && !m_trueCaster->IsFacingTargetsFront(target)))
                return SPELL_FAILED_NOT_INFRONT;

            // check if target is in combat
            if (non_caster_target && m_spellInfo->HasAttribute(SPELL_ATTR_EX_ONLY_PEACEFUL_TARGETS) && target->IsInCombat())
                return SPELL_FAILED_TARGET_AFFECTING_COMBAT;

            // check if target is affected by Spirit of Redemption (Aura: 27827) unless death persistent
            if (target->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION) && !m_spellInfo->HasAttribute(SPELL_ATTR_EX3_ALLOW_AURA_WHILE_DEAD))
                return SPELL_FAILED_BAD_TARGETS;

            // Check if more powerful spell applied on target (if spell only contains non-aoe auras)
            if (IsAuraApplyEffects(m_spellInfo, SpellEffectIndexMask(affectedMask)) && !IsAreaOfEffectSpell(m_spellInfo) && !HasAreaAuraEffect(m_spellInfo) && !selfTargeting)
            {
                bool computed = false; // optimization
                int32 amounts[MAX_EFFECT_INDEX];
                for (auto const& pair : target->GetSpellAuraHolderMap())
                {
                    const SpellAuraHolder* existing = pair.second;
                    const SpellEntry* existingSpell = existing->GetSpellProto();

                    if (m_trueCaster->GetObjectGuid() != existing->GetCasterGuid())
                    {
                        if (sSpellMgr.IsSpellStackableWithSpellForDifferentCasters(m_spellInfo, existingSpell))
                            continue;
                    }
                    else if (sSpellMgr.IsSpellStackableWithSpell(m_spellInfo, existingSpell))
                        continue;

                    if (!computed)
                    {
                        for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
                        {
                            if (affectedMask & (1 << i))
                            {
                                amounts[i] = CalculateSpellEffectValue(SpellEffectIndex(i), target, true, false);
                                amounts[i] = Aura::CalculateAuraEffectValue(m_caster, target, m_spellInfo, SpellEffectIndex(i), amounts[i]);
                                if (m_auraScript)
                                {
                                    AuraCalcData data(nullptr, m_caster, target, m_spellInfo, SpellEffectIndex(i), m_CastItem);
                                    amounts[i] = m_auraScript->OnAuraValueCalculate(data, amounts[i]);
                                }
                            }
                        }
                        computed = true;
                    }

                    if (IsSimilarExistingAuraStronger(m_caster, m_spellInfo, existing, affectedMask, amounts))
                        return SPELL_FAILED_AURA_BOUNCED;

                    if (sSpellMgr.IsSpellAnotherRankOfSpell(m_spellInfo->Id, existingSpell->Id))
                        if (sSpellMgr.IsSpellHigherRankOfSpell(existingSpell->Id, m_spellInfo->Id))
                            return SPELL_FAILED_AURA_BOUNCED;
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

            if (m_spellInfo->MaxTargetLevel && target->GetLevel() > m_spellInfo->MaxTargetLevel)
                return SPELL_FAILED_HIGHLEVEL;

#ifdef ENABLE_PLAYERBOTS
            if (target->IsPlayer())
            {
                PlayerbotAI* bot = ((Player*)target)->GetPlayerbotAI();
                if (bot && bot->IsImmuneToSpell(m_spellInfo->Id))
                {
                    return SPELL_FAILED_IMMUNE;
                }
            }
#endif
        }
    }

    // check targets
    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        uint32 targetType = m_spellInfo->EffectImplicitTargetA[i];
        switch (targetType)
        {
            case TARGET_UNIT_ENEMY_NEAR_CASTER:
            case TARGET_UNIT_FRIEND_NEAR_CASTER:
            case TARGET_UNIT_NEAR_CASTER:
            case TARGET_UNIT_CASTER_MASTER:
            case TARGET_UNIT_CASTER: break; // never check anything
            case TARGET_UNIT_CASTER_PET: // special pet checks
            {
                Unit* pet = m_caster->GetPet();
                if (!pet)
                    pet = m_caster->GetCharm();
                if (!pet)
                    return SPELL_FAILED_NO_PET;
                else
                {
                    if (!pet->IsAlive())
                        return SPELL_FAILED_TARGETS_DEAD;
                    if (!IsIgnoreLosSpellEffect(m_spellInfo, SpellEffectIndex(i), false) && !m_caster->IsWithinLOSInMap(pet, true))
                        return SPELL_FAILED_LINE_OF_SIGHT;
                }
                break;
            }
            default: // needs target
            {
                auto& data = SpellTargetInfoTable[targetType];
                WorldObject* originalCaster = GetCastingObject();
                if (!originalCaster)
                    originalCaster = m_trueCaster;
                if (data.type == TARGET_TYPE_UNIT && data.filter != TARGET_SCRIPT && (data.enumerator == TARGET_ENUMERATOR_SINGLE || data.enumerator == TARGET_ENUMERATOR_CHAIN))
                {
                    if (!target)
                        return SPELL_FAILED_BAD_TARGETS;
                    switch (data.filter)
                    {
                        case TARGET_HARMFUL: if (!originalCaster->CanAttackSpell(target, m_spellInfo)) return SPELL_FAILED_BAD_TARGETS; break;
                        case TARGET_HELPFUL: if (!originalCaster->CanAssistSpell(target, m_spellInfo)) return SPELL_FAILED_BAD_TARGETS; break;
                        case TARGET_PARTY:
                        case TARGET_GROUP: if (!m_trueCaster->CanAssistSpell(target, m_spellInfo) || !m_caster->IsInGroup(target, targetType == TARGET_UNIT_PARTY)) return SPELL_FAILED_BAD_TARGETS; break;
                        default: break;
                    }
                }
            }
        }
    }

    // zone check
    uint32 zone, area;
    m_trueCaster->GetZoneAndAreaId(zone, area);

    SpellCastResult locRes = sSpellMgr.GetSpellAllowedInLocationError(m_spellInfo, m_trueCaster->GetMapId(), zone, area,
                             m_caster ? m_caster->GetBeneficiaryPlayer() : nullptr);
    if (locRes != SPELL_CAST_OK)
    {
        if (!m_IsTriggeredSpell)
            return locRes;
        return SPELL_FAILED_DONT_REPORT;
    }

    // not let players cast spells at mount (and let do it to creatures)
    if (m_trueCaster->IsPlayer() && m_caster->GetMountID() && !m_IsTriggeredSpell &&
            !IsPassiveSpell(m_spellInfo) && !m_spellInfo->HasAttribute(SPELL_ATTR_ALLOW_WHILE_MOUNTED))
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

    // check spell focus object
    if (m_spellInfo->RequiresSpellFocus)
    {
        GameObject* ok = nullptr;
        MaNGOS::GameObjectFocusCheck go_check(m_trueCaster, m_spellInfo->RequiresSpellFocus);
        MaNGOS::GameObjectSearcher<MaNGOS::GameObjectFocusCheck> checker(ok, go_check);
        Cell::VisitGridObjects(m_trueCaster, checker, m_trueCaster->GetMap()->GetVisibilityDistance());

        if (!ok)
            return SPELL_FAILED_REQUIRES_SPELL_FOCUS;

        m_eventTarget = ok;                                   // game object found in range
    }

    if (!m_IsTriggeredSpell)
    {
        SpellCastResult castResult;
        if (!m_triggeredByAuraSpell)
        {
            castResult = CheckRange(strict);
            if (castResult != SPELL_CAST_OK)
                return castResult;
        }
    }

    if (!m_ignoreCosts && !m_IsTriggeredSpell)
    {
        SpellCastResult castResult = CheckPower(strict);
        if (castResult != SPELL_CAST_OK)
            return castResult;

        // triggered spell not affected by stun/etc
        castResult = CheckCasterAuras(m_param1);
        if (castResult != SPELL_CAST_OK)
            return castResult;
    }

    if (m_channelOnly)
    {
        m_partialApplicationMask = EFFECT_MASK_ALL; // no effects to be executed but spell needs to go through
        return SPELL_CAST_OK;
    }

    uint32 availableEffectMask = 0;
    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (m_spellInfo->Effect[i] && (m_effectSkipMask & (1 << i)) == 0)
            availableEffectMask |= (1 << i);

    auto partialApplication = [&](uint32 i) -> SpellCastResult
    {
        availableEffectMask &= (~(1 << i));
        if (availableEffectMask == 0)
            return SPELL_FAILED_BAD_TARGETS;
        else
            m_partialApplicationMask |= (1 << i);

        return SPELL_CAST_OK;
    };

    m_partialApplicationMask |= m_effectSkipMask;

    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if ((m_effectSkipMask & (1 << i)) != 0)
            continue;

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

                    bool inCombat = true;
                    for (auto& itr : targetsCombat)
                    {
                        if (!itr->IsInCombat())
                        {
                            inCombat = false;
                            break;
                        }
                    }
                            
                    if (inCombat)
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
            case SPELL_EFFECT_CREATE_ITEM:
            {
                if (Unit* target = m_targets.getUnitTarget())
                {
                    if (!target->IsPlayer() && m_spellInfo->EffectImplicitTargetA[i] != TARGET_UNIT_CASTER && m_spellInfo->EffectImplicitTargetB[i] != TARGET_UNIT_CASTER)
                        return SPELL_FAILED_BAD_TARGETS;

                    if (!target->IsPlayer())
                        target = m_caster;

                    if (i != EFFECT_INDEX_0) // TODO: Partial application
                        break;

                    uint32 count = CalculateSpellEffectValue(SpellEffectIndex(i), target);
                    ItemPosCountVec dest;
                    uint32 no_space = 0;
                    InventoryResult msg = static_cast<Player*>(target)->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, m_spellInfo->EffectItemType[i], count, &no_space);
                    if (msg != EQUIP_ERR_OK)
                        return SPELL_FAILED_TOO_MANY_OF_ITEM;
                }

                break;
            }
            case SPELL_EFFECT_TAMECREATURE:
            {
                // Spell can be triggered, we need to check original caster prior to caster
                Unit* caster = GetAffectiveCaster();
                Unit* tameTarget = m_targets.getUnitTarget();
                bool gmmode = caster->IsPlayer() && static_cast<Player*>(caster)->IsGameMaster();

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

                if (target->GetLevel() > plrCaster->GetLevel() && !gmmode)
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

                if (learn_spellproto->spellLevel > pet->GetLevel())
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

                if (learn_spellproto->spellLevel > pet->GetLevel())
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

                if (pet->IsInCombat())
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
                if (!m_ignoreRoot && m_caster->hasUnitState(UNIT_STAT_ROOT))
                    return SPELL_FAILED_ROOTED;

                if (Unit* target = m_targets.getUnitTarget())
                {
                    float range = GetSpellMaxRange(sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex));

                    Position pos;
                    target->GetFirstCollisionPosition(pos, target->GetCombatReach(), target->GetAngle(m_caster));

                    // TODO: Implement jumpin case check
                    //if (!m_caster->m_movementInfo.HasMovementFlag(MovementFlags(MOVEFLAG_JUMPING | MOVEFLAG_FALLINGFAR)) && (pos.coord_z < m_caster->GetPositionZ()) && (fabs(pos.coord_z - m_caster->GetPositionZ()) < 3.0f))
                    //{
                    PathFinder pathFinder(m_caster);
                    pathFinder.setPathLengthLimit(range * 1.5f);
                    bool result = pathFinder.calculate(pos.x, pos.y, pos.z);

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

                if (creature->IsAlive())
                    return SPELL_FAILED_TARGET_NOT_DEAD;

                if (creature->GetLootStatus() != CREATURE_LOOT_STATUS_LOOTED)// || creature->GetCreatureType() != CREATURE_TYPE_CRITTER)
                    return SPELL_FAILED_TARGET_NOT_LOOTED;

                uint32 skill = creature->GetCreatureInfo()->GetRequiredLootSkill();

                int32 skillValue = ((Player*)m_caster)->GetSkillValue(skill);
                int32 TargetLevel = m_targets.getUnitTarget()->GetLevel();
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

                    if (!strict && go->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE))
                        return SPELL_FAILED_CHEST_IN_USE;

                    // done in client but we need to recheck anyway
                    if (go->GetGOInfo()->CannotBeUsedUnderImmunity() && m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE))
                        return SPELL_FAILED_DAMAGE_IMMUNE;
                }
                else if (Item* item = m_targets.getItemTarget())
                {
                    // not own (trade?)
                    Player* itemOwner = item->GetOwner();
                    Player* itemTrader = itemOwner->GetTrader();

                    if (itemOwner != m_caster && itemTrader != m_caster)
                        return SPELL_FAILED_ITEM_GONE;

                    lockId = item->GetProto()->LockID;

                    // if already unlocked
                    if (!lockId || item->HasFlag(ITEM_FIELD_FLAGS, ITEM_DYNFLAG_UNLOCKED))
                        return SPELL_FAILED_ALREADY_OPEN;
                }
                else
                    return SPELL_FAILED_BAD_TARGETS;

                // check lock compatibility
                SpellEffectIndex effIdx = SpellEffectIndex(i);
                SpellCastResult res = CanOpenLock(effIdx, lockId, m_effectSkillInfo[effIdx].skillId, m_effectSkillInfo[effIdx].reqSkillValue, m_effectSkillInfo[effIdx].skillValue);
                if (res != SPELL_CAST_OK)
                    return res;

                // chance for fail at orange mining/herb/LockPicking gathering attempt
                // second check prevent fail at rechecks
                if (!strict && m_effectSkillInfo[effIdx].skillId != SKILL_NONE)
                {
                    bool canFailAtMax = m_effectSkillInfo[effIdx].skillId != SKILL_HERBALISM && m_effectSkillInfo[effIdx].skillId != SKILL_MINING;

                    // chance for failure in orange gather / lockpick (gathering skill can't fail at maxskill)
                    if ((canFailAtMax || m_effectSkillInfo[effIdx].skillValue < sWorld.GetConfigMaxSkillValue())
                        && m_effectSkillInfo[effIdx].reqSkillValue > irand(m_effectSkillInfo[effIdx].skillValue - 25, m_effectSkillInfo[effIdx].skillValue + 37))
                        return SPELL_FAILED_TRY_AGAIN;
                }
                if (m_CastItem)
                    m_effectSkillInfo[effIdx].skillId = SKILL_NONE;
                break;
            }
            case SPELL_EFFECT_PICKPOCKET:
            {
                // should always fail above if not exists
                Unit* target = m_targets.getUnitTarget();
                if (!target->IsCreature())
                    return SPELL_FAILED_BAD_TARGETS;

                Creature* creatureTarget = static_cast<Creature*>(target);
                if (!creatureTarget->GetCreatureInfo()->PickpocketLootId)
                    return SPELL_FAILED_TARGET_NO_POCKETS;
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
                else if (pet->IsAlive())
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

                if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_DISMISS_PET_FIRST))
                    if (m_caster->FindGuardianWithEntry(m_spellInfo->EffectMiscValue[i]))
                        return SPELL_FAILED_ALREADY_HAVE_SUMMON;

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
                        if (!pet->IsAlive())
                        {
                            ((Player*)m_caster)->SendPetTameFailure(PETTAME_DEAD);
                            return SPELL_FAILED_DONT_REPORT;
                        }
                        else if (!m_spellInfo->HasAttribute(SPELL_ATTR_EX_DISMISS_PET_FIRST))
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
                    if (!m_spellInfo->HasAttribute(SPELL_ATTR_EX_DISMISS_PET_FIRST))
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
                    if (instance->levelMin > target->GetLevel())
                        return SPELL_FAILED_LOWLEVEL;
                    if (instance->levelMax && instance->levelMax < target->GetLevel())
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

                if (!m_ignoreRoot && m_caster->hasUnitState(UNIT_STAT_ROOT))
                    return SPELL_FAILED_ROOTED;

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

    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
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
                if (!m_trueCaster->IsPlayer())
                    return SPELL_FAILED_UNKNOWN;

                if (!m_spellInfo->HasAttribute(SPELL_ATTR_EX_DISMISS_PET_FIRST) && m_caster->HasCharm())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                if (m_caster->HasCharmer())
                    return SPELL_FAILED_CHARMED;

                if (expectedTarget) // target may not be known at CheckCast time - TODO: add support for these checks on CheckTarget
                {
                    if (expectedTarget == m_caster)
                        return SPELL_FAILED_BAD_TARGETS;

                    if (expectedTarget->HasCharmer())
                        return SPELL_FAILED_CHARMED;

                    if (int32(expectedTarget->GetLevel()) > CalculateSpellEffectValue(SpellEffectIndex(i), expectedTarget))
                        return SPELL_FAILED_HIGHLEVEL;

                    if (expectedTarget->GetOwner())
                        return SPELL_FAILED_CANT_BE_CHARMED;
                }
                break;
            }
            case SPELL_AURA_MOD_CHARM:
            {
                if (!m_spellInfo->HasAttribute(SPELL_ATTR_EX_DISMISS_PET_FIRST) && m_caster->GetTypeId() == TYPEID_PLAYER && m_caster->HasCharm())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                if (m_caster->HasCharmer())
                    return SPELL_FAILED_CHARMED;

                if (expectedTarget) // target may not be known at CheckCast time - TODO: add support for these checks on CheckTarget
                {
                    if (expectedTarget == m_caster)
                        return SPELL_FAILED_BAD_TARGETS;

                    if (expectedTarget->HasCharmer())
                        return SPELL_FAILED_CHARMED;

                    if (int32(expectedTarget->GetLevel()) > CalculateSpellEffectValue(SpellEffectIndex(i), expectedTarget))
                        return SPELL_FAILED_HIGHLEVEL;

                    if (expectedTarget->GetOwner())
                        return SPELL_FAILED_CANT_BE_CHARMED;
                }
                break;
            }
            case SPELL_AURA_MOD_POSSESS_PET:
            {
                if (!m_trueCaster->IsPlayer())
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
                if (expectedTarget)
                {
                    if (expectedTarget->GetPowerType() != POWER_MANA)
                    {
                        SpellCastResult result = partialApplication(i);
                        if (result != SPELL_CAST_OK)
                            return result;
                    }
                }

                break;
            }
            case SPELL_AURA_WATER_WALK:
            {
                if (expectedTarget && expectedTarget->GetTypeId() == TYPEID_PLAYER)
                {
                    Player const* player = static_cast<Player const*>(expectedTarget);

                    // Player is not allowed to cast water walk on shapeshifted/mounted player
                    if (player->IsShapeShifted() || player->IsMounted())
                        return SPELL_FAILED_BAD_TARGETS;
                }

                break;
            }
            case SPELL_AURA_MOD_DISARM:
            {
                if (expectedTarget)
                {
                    // Target must be a weapon wielder
                    if (!expectedTarget->hasMainhandWeapon())
                    {
                        SpellCastResult result = partialApplication(i);
                        if (result != SPELL_CAST_OK)
                            return SPELL_FAILED_TARGET_NO_WEAPONS;
                    }
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
        if (!m_trueCaster->IsPlayer())
            return SPELL_FAILED_NOT_TRADING;

        if (TradeSlots(m_targets.getItemTargetGuid().GetRawValue()) != TRADE_SLOT_NONTRADED)
            return SPELL_FAILED_ITEM_NOT_READY;

        // if trade not complete then remember it in trade data
        if (TradeData* my_trade = static_cast<Player*>(m_caster)->GetTradeData())
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

    if (m_trueCaster->IsPlayer() && m_spellInfo->HasAttribute(SPELL_ATTR_EX2_NO_ACTIVE_PETS))
    {
        Player* player = static_cast<Player*>(m_caster);
        if (player->GetPetGuid() || player->HasCharm())
        {
            player->SendPetTameFailure(PETTAME_ANOTHERSUMMONACTIVE);
            return SPELL_FAILED_DONT_REPORT;
        }
        SpellCastResult result = Pet::TryLoadFromDB(player);
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
    if (!m_caster->IsAlive())
        return SPELL_FAILED_CASTER_DEAD;

    if (m_caster->IsNonMeleeSpellCasted(false) && !m_ignoreConcurrentCasts)             // prevent spellcast interruption by another spellcast
        return SPELL_FAILED_SPELL_IN_PROGRESS;
    if (m_caster->IsInCombat() && IsNonCombatSpell(m_spellInfo))
        return SPELL_FAILED_AFFECTING_COMBAT;

    if (m_caster->IsCreature() && (static_cast<Creature*>(m_caster)->IsPet() || m_caster->HasCharmer()))
    {
        // dead owner (pets still alive when owners ressed?)
        if (m_caster->GetMaster() && !m_caster->GetMaster()->IsAlive())
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
            if (_target->IsDead() && !m_spellInfo->HasAttribute(SPELL_ATTR_EX2_ALLOW_DEAD_TARGET))
                return SPELL_FAILED_BAD_TARGETS;

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

SpellCastResult Spell::CheckCasterAuras(uint32& param1) const
{
    if (m_ignoreCasterAuraState)
        return SPELL_CAST_OK;

    if (!m_trueCaster->IsUnit())
        return SPELL_CAST_OK;

    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX3_IGNORE_CASTER_AND_TARGET_RESTRICTIONS))
        return SPELL_CAST_OK;

    // these attributes only show the spell as usable on the client when it has related aura applied
     // still they need to be checked against certain mechanics

    bool usableWhileStunned = false;

    bool usableWhileFeared = false;

    bool usableWhileConfused = false;

    // Check whether the cast should be prevented by any state you might have.
    SpellCastResult result = SPELL_CAST_OK;

    // Get unit state
    uint32 const unitflag = m_caster->GetUInt32Value(UNIT_FIELD_FLAGS);

    // this check should only be done when player does cast directly
    // (ie not when it's called from a script) Breaks for example PlayerAI when charmed
    if (m_clientCast)
        if (Unit* charmer = m_caster->GetCharmer())
            if (!CheckSpellCancelsCharm(param1))
                result = SPELL_FAILED_CHARMED;

    // spell has attribute usable while having a cc state, check if caster has allowed mechanic auras, another mechanic types must prevent cast spell
    auto mechanicCheck = [&](AuraType type) -> SpellCastResult
    {
        bool foundNotMechanic = false;
        Unit::AuraList const& auras = m_caster->GetAurasByType(type);
        for (Aura const* aura : auras)
        {
            uint32 const mechanicMask = aura->GetSpellProto()->GetAllEffectsMechanicMask();
            if (mechanicMask && !(mechanicMask & GetAllowedMechanicMask(m_spellInfo)))
            {
                foundNotMechanic = true;

                // fill up aura mechanic info to send client proper error message
                param1 = aura->GetSpellProto()->EffectMechanic[aura->GetEffIndex()];
                if (!param1)
                    param1 = aura->GetSpellProto()->Mechanic;

                break;
            }
        }

        if (foundNotMechanic)
        {
            switch (type)
            {
                case SPELL_AURA_MOD_STUN:
                    return SPELL_FAILED_STUNNED;
                case SPELL_AURA_MOD_FEAR:
                    return SPELL_FAILED_FLEEING;
                case SPELL_AURA_MOD_CONFUSE:
                    return SPELL_FAILED_CONFUSED;
                default:
                    MANGOS_ASSERT(false);
                    return SPELL_FAILED_NOT_KNOWN;
            }
        }

        return SPELL_CAST_OK;
    };

    if (unitflag & UNIT_FLAG_STUNNED && !usableWhileStunned && !CheckSpellCancelsStun(param1))
        result = SPELL_FAILED_STUNNED;
    else if (unitflag & UNIT_FLAG_SILENCED && m_spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE && !CheckSpellCancelsSilence(param1))
        result = SPELL_FAILED_SILENCED;
    else if (unitflag & UNIT_FLAG_PACIFIED && m_spellInfo->PreventionType == SPELL_PREVENTION_TYPE_PACIFY && !CheckSpellCancelsPacify(param1))
        result = SPELL_FAILED_PACIFIED;
    else if (unitflag & UNIT_FLAG_FLEEING && !usableWhileFeared && !CheckSpellCancelsFear(param1))
        result = SPELL_FAILED_FLEEING;
    else if (unitflag & UNIT_FLAG_CONFUSED && !usableWhileConfused && !CheckSpellCancelsConfuse(param1))
        result = SPELL_FAILED_CONFUSED;

    // Attr must make flag drop spell totally immune from all effects
    if (result != SPELL_CAST_OK)
        return param1 ? SPELL_FAILED_PREVENTED_BY_MECHANIC : result;

    return SPELL_CAST_OK;
}

bool Spell::CheckSpellCancelsAuraEffect(AuraType auraType, uint32& param1) const
{
    Unit* unitCaster = (m_trueCaster->IsGameObject() ? nullptr : m_caster);
    if (!unitCaster)
        return false;

    // Checking auras is needed now, because you are prevented by some state but the spell grants immunity.
    Unit::AuraList const& auraEffects = unitCaster->GetAurasByType(auraType);
    if (auraEffects.empty())
        return true;

    for (Aura const* aura : auraEffects)
    {
        SpellEntry const* auraInfo = aura->GetSpellProto();
        if (SpellCancelsAuraEffect(m_spellInfo, auraInfo, aura->GetEffIndex()))
            continue;

        param1 = auraInfo->EffectMechanic[aura->GetEffIndex()];
        if (!param1)
            param1 = auraInfo->Mechanic;

        return false;
    }

    return true;
}

bool Spell::CheckSpellCancelsCharm(uint32& param1) const
{
    return CheckSpellCancelsAuraEffect(SPELL_AURA_MOD_CHARM, param1) &&
        CheckSpellCancelsAuraEffect(SPELL_AURA_AOE_CHARM, param1) &&
        CheckSpellCancelsAuraEffect(SPELL_AURA_MOD_POSSESS, param1);
}

bool Spell::CheckSpellCancelsStun(uint32& param1) const
{
    return CheckSpellCancelsAuraEffect(SPELL_AURA_MOD_STUN, param1);
}

bool Spell::CheckSpellCancelsSilence(uint32& param1) const
{
    return CheckSpellCancelsAuraEffect(SPELL_AURA_MOD_SILENCE, param1) &&
        CheckSpellCancelsAuraEffect(SPELL_AURA_MOD_PACIFY_SILENCE, param1);
}

bool Spell::CheckSpellCancelsPacify(uint32& param1) const
{
    return CheckSpellCancelsAuraEffect(SPELL_AURA_MOD_PACIFY, param1) &&
        CheckSpellCancelsAuraEffect(SPELL_AURA_MOD_PACIFY_SILENCE, param1);
}

bool Spell::CheckSpellCancelsFear(uint32& param1) const
{
    return CheckSpellCancelsAuraEffect(SPELL_AURA_MOD_FEAR, param1);
}

bool Spell::CheckSpellCancelsConfuse(uint32& param1) const
{
    return CheckSpellCancelsAuraEffect(SPELL_AURA_MOD_CONFUSE, param1);
}

bool Spell::CanAutoCast(Unit* target)
{
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
        return true;
    return false;                                           // target invalid
}

std::pair<float, float> Spell::GetMinMaxRange(bool strict)
{
    float minRange = 0.0f, maxRange = 0.0f, rangeMod = 0.0f;

    if (strict && IsNextMeleeSwingSpell(m_spellInfo))
        return { 0.0f, 100.0f };

    Unit* caster = dynamic_cast<Unit*>(m_trueCaster); // preparation for GO casting

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
                rangeMod = m_trueCaster->GetCombatReach() + (target ? target->GetCombatReach() : m_trueCaster->GetCombatReach());

                if (minRange > 0.0f && !(spellRange->Flags & SPELL_RANGE_FLAG_RANGED))
                    minRange += rangeMod;
            }
        }

        if (target && caster && caster->IsMovingForward() && target->IsMovingForward() && !caster->IsWalking() && !target->IsWalking() &&
            ((spellRange->Flags & SPELL_RANGE_FLAG_MELEE) || target->GetTypeId() == TYPEID_PLAYER))
            rangeMod += MELEE_LEEWAY;
    }

    if (caster)
    {
        if (m_spellInfo->HasAttribute(SPELL_ATTR_USES_RANGED_SLOT) && caster->GetTypeId() == TYPEID_PLAYER)
            if (Item* ranged = static_cast<Player*>(caster)->GetWeaponForAttack(RANGED_ATTACK))
                maxRange *= ranged->GetProto()->RangedModRange * 0.01f;

        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_RANGE, maxRange);
    }

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

    if (!target && m_clientCast && HasSpellTarget(m_spellInfo, TARGET_UNIT_CASTER_PET))
        target = m_caster->GetPet() ? m_caster->GetPet() : m_caster->GetCharm();

    if (target && target != m_trueCaster)
    {
        // distance from target in checks
        float dist = m_trueCaster->GetDistance(target, true, DIST_CALC_NONE); // minRange/maxRange already contain everything
        if (dist > maxRange * maxRange)
           return SPELL_FAILED_OUT_OF_RANGE;
        if (minRange && dist < minRange * minRange)
            return SPELL_FAILED_TOO_CLOSE;
        if (m_trueCaster->IsPlayer() &&
                (sSpellMgr.GetSpellFacingFlag(m_spellInfo->Id) & SPELL_FACING_FLAG_INFRONT) && !m_trueCaster->HasInArc(target))
            return SPELL_FAILED_UNIT_NOT_INFRONT;
    }

    if (m_targets.m_targetMask == TARGET_FLAG_DEST_LOCATION)
    {
        float dist = m_trueCaster->GetDistance(m_targets.m_destPos.x, m_targets.m_destPos.y, m_targets.m_destPos.z, DIST_CALC_NONE);
        if (dist > maxRange * maxRange)
            return SPELL_FAILED_OUT_OF_RANGE;
        if (minRange && dist < minRange * minRange)
            return SPELL_FAILED_TOO_CLOSE;
        if (!IsIgnoreLosSpell(m_spellInfo))
            if (!m_trueCaster->IsWithinLOS(m_targets.m_destPos.x, m_targets.m_destPos.y, m_targets.m_destPos.z + 1.f))
                return SPELL_FAILED_LINE_OF_SIGHT;
    }

    if (m_targets.m_targetMask == TARGET_FLAG_SOURCE_LOCATION)
    {
        float dist = m_trueCaster->GetDistance(m_targets.m_srcPos.x, m_targets.m_srcPos.y, m_targets.m_srcPos.z, DIST_CALC_NONE);
        if (dist > maxRange* maxRange)
            return SPELL_FAILED_OUT_OF_RANGE;
        if (minRange && dist < minRange * minRange)
            return SPELL_FAILED_TOO_CLOSE;
        if (!IsIgnoreLosSpell(m_spellInfo))
            if (!m_trueCaster->IsWithinLOS(m_targets.m_srcPos.x, m_targets.m_srcPos.y, m_targets.m_srcPos.z + 1.f))
                return SPELL_FAILED_LINE_OF_SIGHT;
    }

    m_maxRange = maxRange; // need to save max range for some calculations

    return SPELL_CAST_OK;
}

int32 Spell::CalculateSpellEffectDamage(Unit* unitTarget, int32 damage, float damageDoneMod, SpellEffectIndex effectIndex)
{
    // damage bonus (per damage class)
    switch (m_spellInfo->DmgClass)
    {
        // Melee and Ranged Spells
        case SPELL_DAMAGE_CLASS_RANGED:
        case SPELL_DAMAGE_CLASS_MELEE:
        {
            // Calculate damage bonus
            if (!m_trueCaster->IsGameObject())
                damage = m_caster->MeleeDamageBonusDone(unitTarget, damage, m_attackType, m_spellSchoolMask, m_spellInfo, effectIndex, SPELL_DIRECT_DAMAGE);
            damage *= damageDoneMod;
            damage = unitTarget->MeleeDamageBonusTaken(m_trueCaster->IsGameObject() ? nullptr : m_caster, damage, m_attackType, m_spellSchoolMask, m_spellInfo, effectIndex, SPELL_DIRECT_DAMAGE);
        }
        break;
        // Magical Attacks
        case SPELL_DAMAGE_CLASS_NONE:
        case SPELL_DAMAGE_CLASS_MAGIC:
        {
            // Calculate damage bonus
            if (!m_trueCaster->IsGameObject())
                damage = m_caster->SpellDamageBonusDone(unitTarget, m_spellSchoolMask, m_spellInfo, effectIndex, damage, SPELL_DIRECT_DAMAGE);
            damage *= damageDoneMod;
            damage = unitTarget->SpellDamageBonusTaken(m_trueCaster->IsGameObject() ? nullptr : m_caster, m_spellSchoolMask, m_spellInfo, effectIndex, damage, SPELL_DIRECT_DAMAGE);
        }
        break;
    }
    return damage;
}

uint32 Spell::CalculatePowerCost(SpellEntry const* spellInfo, Unit* caster, Spell* spell, Item* castItem, bool finalUse)
{
    // item cast not used power
    if (castItem)
        return 0;

    // Spell drain all exist power on cast (Only paladin lay of Hands)
    if (spellInfo->HasAttribute(SPELL_ATTR_EX_USE_ALL_MANA))
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
            modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_COST, powerCost, finalUse);

    if (spellInfo->HasAttribute(SPELL_ATTR_SCALES_WITH_CREATURE_LEVEL))
        powerCost = int32(powerCost / (1.117f * spellInfo->spellLevel / caster->GetLevel() - 0.1327f));

    // PCT mod from user auras by school
    powerCost = int32(powerCost * (1.0f + caster->GetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + school)));
    if (powerCost < 0)
        powerCost = 0;
    return powerCost;
}

SpellCastResult Spell::CheckPower(bool strict)
{
    // item or GO cast does not use power
    if (m_CastItem || !m_caster)
        return SPELL_CAST_OK;

    m_powerCost = CalculatePowerCost(m_spellInfo, m_caster, this, m_CastItem, !strict);

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
    if (m_channelOnly || m_ignoreCosts)
        return true;

#ifdef ENABLE_PLAYERBOTS
    if (m_caster->IsPlayer())
    {
        PlayerbotAI* bot = ((Player*)m_caster)->GetPlayerbotAI();
        if (bot && bot->HasSpellItems(m_spellInfo->Id, m_CastItem))
        {
            return true;
        }
    }
#endif

    // Workaround for double shard problem
    if (m_IsTriggeredSpell || this->m_spellInfo->Id == 46546)
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
    if (!m_trueCaster->IsPlayer())
        return SPELL_CAST_OK;

    Player* p_caster = static_cast<Player*>(m_caster);
    Player* playerTarget = p_caster;
    if (Unit* target = m_targets.getUnitTarget())
        if (target->IsPlayer())
            playerTarget = static_cast<Player*>(target);

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

        for (uint32 i = 0; i < 5; ++i)
            if (proto->Spells[i].SpellCharges)
                if (m_CastItem->GetSpellCharges(i) == 0)
                    return SPELL_FAILED_NO_CHARGES_REMAIN;

        // consumable cast item checks
        if (proto->Class == ITEM_CLASS_CONSUMABLE)
        {
            // such items should only fail if there is no suitable effect at all - see Rejuvenation Potions for example
            SpellCastResult failReason = SPELL_CAST_OK;
            for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
            {
                // skip check, pet not required like checks, and for TARGET_UNIT_CASTER_PET m_targets.getUnitTarget() is not the real target but the caster
                if (m_spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_CASTER_PET)
                    continue;

                Unit* target = m_spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_CASTER ? m_caster : m_targets.getUnitTarget();
                if (!target)
                    continue;

                if (m_spellInfo->Effect[i] == SPELL_EFFECT_HEAL)
                {
                    if (target->GetHealth() == target->GetMaxHealth())
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
                    if (target->GetPower(power) == target->GetMaxPower(power))
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

        if (m_spellInfo->HasAttribute(SPELL_ATTR_HELD_ITEM_ONLY) && (m_targets.getItemTarget()->GetSlot() < EQUIPMENT_SLOT_MAINHAND || m_targets.getItemTarget()->GetSlot() > EQUIPMENT_SLOT_RANGED))
            return m_IsTriggeredSpell && !(m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
                   ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_MAINHAND_EMPTY;
    }
    // if not item target then required item must be equipped (for triggered case not report error)
    else
    {
        uint32 error = SPELL_FAILED_EQUIPPED_ITEM_CLASS;
        if (m_caster->IsPlayer() && !static_cast<Player*>(m_caster)->HasItemFitToSpellReqirements(m_spellInfo, nullptr, &error))
            return m_IsTriggeredSpell ? SPELL_FAILED_DONT_REPORT : SpellCastResult(error);
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

        m_eventTarget = ok;                                   // game object found in range
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
                        return SPELL_FAILED_REAGENTS;
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
                {
                    m_param1 = itemid;
                    return SPELL_FAILED_REAGENTS;
                }
            }
        }

        // check totem-item requirements (items presence in inventory)
        uint32 totems = MAX_SPELL_TOTEMS;
        for (auto i : m_spellInfo->Totem)
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
            return SPELL_FAILED_TOTEMS;

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
    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
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
                    uint32 count = CalculateSpellEffectValue(SpellEffectIndex(i), m_caster);
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
                    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_ENCHANT_OWN_ITEM_ONLY))
                        return SPELL_FAILED_NOT_TRADEABLE;

                    uint32 enchant_id = m_spellInfo->EffectMiscValue[i];
                    SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                    if (!pEnchant)
                        return SPELL_FAILED_ERROR;
                    if (pEnchant->flags & ENCHANTMENT_SOULBOUND)
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
                    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_ENCHANT_OWN_ITEM_ONLY))
                        return SPELL_FAILED_NOT_TRADEABLE;

                    uint32 enchant_id = m_spellInfo->EffectMiscValue[i];
                    SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                    if (!pEnchant)
                        return SPELL_FAILED_ERROR;
                    if (pEnchant->flags & ENCHANTMENT_SOULBOUND)
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
    if (!m_trueCaster->IsPlayer())
        return;

    if (m_spellState == SPELL_STATE_TRAVELING)
        return;                                             // spell is active and can't be time-backed

    // spells not loosing casting time ( slam, dynamites, bombs.. )
    if (!(m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_DAMAGE_PUSHBACK))
        return;

    // check resist chance
    int32 resistChance = 100;                               // must be initialized to 100 for percent modifiers
    static_cast<Player*>(m_caster)->ApplySpellMod(m_spellInfo->Id, SPELLMOD_NOT_LOSE_CASTING_TIME, resistChance);
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
    if (!m_trueCaster->IsPlayer() || getState() != SPELL_STATE_CHANNELING)
        return;

    // check resist chance
    int32 resistChance = 100;                               // must be initialized to 100 for percent modifiers
    static_cast<Player*>(m_caster)->ApplySpellMod(m_spellInfo->Id, SPELLMOD_NOT_LOSE_CASTING_TIME, resistChance);
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
    if (m_originalCasterGUID == m_trueCaster->GetObjectGuid() && m_trueCaster->IsUnit())
        m_originalCaster = static_cast<Unit*>(m_trueCaster);
    else if (m_originalCasterGUID.IsGameObject())
    {
        GameObject* go = m_trueCaster->IsInWorld() ? m_trueCaster->GetMap()->GetGameObject(m_originalCasterGUID) : nullptr;
        m_originalCaster = go ? go->GetOwner() : nullptr;
    }
    else
    {
        Unit* unit = ObjectAccessor::GetUnit(*m_trueCaster, m_originalCasterGUID);
        m_originalCaster = unit && unit->IsInWorld() ? unit : nullptr;
    }
}

void Spell::UpdatePointers()
{
    UpdateOriginalCasterPointer();

    m_targets.Update(m_trueCaster);
}

bool Spell::CheckTargetCreatureType(Unit* target, SpellEntry const* spellInfo)
{
    uint32 spellCreatureTargetMask = spellInfo->TargetCreatureType;

    // Dismiss Pet and Taming Lesson skipped
    if (spellInfo->Id == 2641 || spellInfo->Id == 23356)
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
    if (IsNextMeleeSwingSpell(m_spellInfo))
        return (CURRENT_MELEE_SPELL);
    if (IsAutoRepeat())
        return (CURRENT_AUTOREPEAT_SPELL);
    if (IsChanneledSpell(m_spellInfo))
        return (CURRENT_CHANNELED_SPELL);
    return (CURRENT_GENERIC_SPELL);
}

bool Spell::CheckTarget(Unit* target, SpellEffectIndex eff, bool targetB, CheckException exception) const
{
    // Check targets for creature type mask and remove not appropriate (skip explicit self target case, maybe need other explicit targets)
    if (exception != EXCEPTION_MAGNET && m_spellInfo->EffectImplicitTargetA[eff] != TARGET_UNIT_CASTER)
    {
        if (!CheckTargetCreatureType(target, m_spellInfo))
            return false;
    }

    WorldObject* affectiveCaster = GetAffectiveCaster();
    uint32 targetType;
    SpellTargetInfo info;
    if (!targetB)
        targetType = m_spellInfo->EffectImplicitTargetA[eff], info = SpellTargetInfoTable[m_spellInfo->EffectImplicitTargetA[eff]];
    else
        targetType = m_spellInfo->EffectImplicitTargetB[eff], info = SpellTargetInfoTable[m_spellInfo->EffectImplicitTargetB[eff]];
    bool scriptTarget = (info.type == TARGET_TYPE_UNIT && info.filter == TARGET_SCRIPT);

    if (target != affectiveCaster)
    {
        // Check targets for not_selectable unit flag and remove
        // A player can cast spells on his pet (or other controlled unit) though in any state
        if ((!affectiveCaster || target->GetMasterGuid() != affectiveCaster->GetObjectGuid()) && !scriptTarget)
        {
            // unselectable targets skipped in all cases except targets with TARGET_SCRIPT
            if (!m_ignoreUnselectableTarget && target != m_targets.getUnitTarget() &&
                    target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE))
                return false;
        }

        // Check player targets and remove if in GM mode or GM invisibility (for not self casting case)
        if (target->IsPlayer())
        {
            if (static_cast<Player*>(target)->GetVisibility() == VISIBILITY_OFF)
                return false;

            if (static_cast<Player*>(target)->IsGameMaster() && !IsPositiveEffect(m_spellInfo, eff, affectiveCaster, target))
                return false;
        }

        if (affectiveCaster && affectiveCaster->IsPlayer())
        {
            // Do not allow these spells to target creatures not tapped by us (Banish, Polymorph, many quest spells)
            if (m_spellInfo->HasAttribute(SPELL_ATTR_EX2_CANNOT_CAST_ON_TAPPED))
                if (Creature const* targetCreature = dynamic_cast<Creature*>(target))
                    if ((!targetCreature->GetLootRecipientGuid().IsEmpty()) && !targetCreature->IsTappedBy(static_cast<Player*>(affectiveCaster)))
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
                // fall through
            case SPELL_EFFECT_RESURRECT_NEW:
                // player far away, maybe his corpse near?
                if (target != m_trueCaster && !target->IsWithinLOSInMap(m_trueCaster, true))
                {
                    if (!m_targets.getCorpseTargetGuid())
                        return false;

                    Corpse* corpse = m_trueCaster->GetMap()->GetCorpse(m_targets.getCorpseTargetGuid());
                    if (!corpse)
                        return false;

                    if (target->GetObjectGuid() != corpse->GetOwnerGuid())
                        return false;

                    if (!corpse->IsWithinLOSInMap(m_trueCaster, true))
                        return false;
                }

                // all ok by some way or another, skip normal check
                break;
            default:                                            // normal case
                if (exception != EXCEPTION_MAGNET && !IsIgnoreLosSpellEffect(m_spellInfo, eff, targetB))
                {
                    float x, y, z;
                    switch (info.los)
                    {
                        case TARGET_LOS_DEST:
                            m_targets.getDestination(x, y, z);
                            if (!target->IsWithinLOS(x, y, z + target->GetCollisionHeight(), true))
                                return false;
                            break;
                        case TARGET_LOS_SRC:
                            m_targets.getSource(x, y, z);
                            if (!target->IsWithinLOS(x, y, z + target->GetCollisionHeight(), true))
                                return false;
                            break;
                        case TARGET_LOS_CASTER:
                            if (target != m_trueCaster && info.enumerator != TARGET_ENUMERATOR_CHAIN) // chain is checked on FilterTargetMap
                            {
                                if (WorldObject* caster = GetCastingObject())
                                {
                                    if (!target->IsWithinLOSInMap(caster, true))
                                        return false;
                                }
                            }
                            break;
                    }
                }
                break;
        }

        if (m_spellInfo->HasAttribute(SPELL_ATTR_EX3_ONLY_ON_GHOSTS) && !target->HasVisFlags(UNIT_VIS_FLAG_GHOST))
            return false;

        if (!IsAllowingDeadTarget(m_spellInfo) && !target->IsAlive())
        {
            if (m_trueCaster->IsPlayer())
                if (target != m_trueCaster || !m_spellInfo->HasAttribute(SPELL_ATTR_ALLOW_CAST_WHILE_DEAD))
                    return false;

            if (target != m_trueCaster)
				if (info.filter == TARGET_HELPFUL)
					return false;
        }
    }

    if (targetType != TARGET_UNIT_CASTER && targetType != TARGET_UNIT_CASTER_PET)
    {
        if (target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNTARGETABLE))
            return false;
        
        if (m_spellInfo->HasAttribute(SPELL_ATTR_EX_ONLY_PEACEFUL_TARGETS) && target->IsInCombat())
            return false;
    }    

    if (m_spellInfo->HasAttribute(SPELL_ATTR_EX3_NOT_ON_AOE_IMMUNE)) // rest done in aoe code
        if (target->IsAOEImmune())
            return false;

    if (target->GetTypeId() != TYPEID_PLAYER && m_spellInfo->HasAttribute(SPELL_ATTR_EX3_ONLY_ON_PLAYER)
        && targetType != TARGET_UNIT_CASTER)
        return false;

    if (m_spellInfo->MaxTargetLevel && target->GetLevel() > m_spellInfo->MaxTargetLevel)
        return false;

#ifdef ENABLE_PLAYERBOTS
    if (target->IsPlayer())
    {
        PlayerbotAI* bot = ((Player*)target)->GetPlayerbotAI();
        if (bot && bot->IsImmuneToSpell(m_spellInfo->Id))
        {
            return false;
        }
    }
#endif

    return OnCheckTarget(target, eff);
}

bool Spell::IsNeedSendToClient() const
{
    if (m_channelOnly)
        return false;

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
    m_Spell.reset(spell, [](Spell* toDelete)
    {
        if (toDelete->IsDeletable() || World::IsStopped())
        {
            delete toDelete;
        }
        else
        {
            sLog.outError("~SpellEvent: %s %u tried to delete non-deletable spell %u. Was not deleted, causes memory leak.",
                          (toDelete->GetCaster()->GetTypeId() == TYPEID_PLAYER ? "Player" : "Creature"), toDelete->GetCaster()->GetGUIDLow(), toDelete->m_spellInfo->Id);
        }
    });
}

SpellEvent::~SpellEvent()
{
    if (m_Spell->getState() != SPELL_STATE_FINISHED)
        m_Spell->cancel();
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
                        m_Spell->GetTrueCaster()->m_events.AddEvent(this, m_Spell->GetDelayStart() + n_offset, false);
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
                m_Spell->GetTrueCaster()->m_events.AddEvent(this, e_time + m_Spell->GetDelayMoment(), false);
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
    m_Spell->GetTrueCaster()->m_events.AddEvent(this, e_time + 1, false);
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

                SkillType tempSkillId = SkillByLockType(LockType(lockInfo->Index[j]));
                skillId = tempSkillId;

                bool oldCalc = true;
                if (tempSkillId == SKILL_NONE) // these must not be carried over to the reference variable - unless more research comes up
                {
                    SkillLineAbilityMapBounds bounds = sSpellMgr.GetSkillLineAbilityMapBoundsBySpellId(m_spellInfo->Id);
                    if (bounds.first != bounds.second)
                    {
                        SkillLineAbilityEntry const* skillInfo = bounds.first->second;
                        tempSkillId = SkillType(skillInfo->skillId);
                        oldCalc = false;
                    }
                }

                if (tempSkillId != SKILL_NONE)
                {
                    uint32 spellSkillBonus = 0;
                    if (oldCalc) // still correct but not usable for spell skill ids
                    {
                        // skill bonus provided by casting spell (mostly item spells)
                        // add the damage modifier from the spell casted (cheat lock / skeleton key etc.) (use m_currentBasePoints, CalculateDamage returns wrong value)
                        uint32 spellSkillBonus = uint32(m_currentBasePoints[effIndex]);
                        reqSkillValue = lockInfo->Skill[j];

                        // castitem check: rogue using skeleton keys. the skill values should not be added in this case.
                        skillValue = m_CastItem || !m_trueCaster->IsPlayer() ?
                            0 : static_cast<Player*>(m_trueCaster)->GetSkillValue(tempSkillId);

                        skillValue += spellSkillBonus;
                    }
                    else if (lockInfo->Index[j] == LOCKTYPE_DISARM_TRAP)
                    {
                        reqSkillValue = INT32_MAX;
                        if (GameObject* go = m_targets.getGOTarget())
                            reqSkillValue = go->GetLevel() * 5;
                        skillValue = CalculateSpellEffectValue(effIndex, nullptr);
                    }

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
    Cell::VisitAllObjects(notifier.GetCenterX(), notifier.GetCenterY(), m_trueCaster->GetMap(), notifier, radius);
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
        return m_trueCaster;

    if (m_originalCasterGUID.IsGameObject() && m_trueCaster->IsInWorld())
        return m_trueCaster->IsInWorld() ? m_trueCaster->GetMap()->GetGameObject(m_originalCasterGUID) : nullptr;
    else if (m_originalCasterGUID.IsDynamicObject())
        return m_trueCaster->IsInWorld() ? m_trueCaster->GetMap()->GetDynamicObject(m_originalCasterGUID) : nullptr;
    return m_originalCaster;
}

Unit* Spell::GetAffectiveCasterOrOwner() const
{
    return m_trueCaster->IsUnit() ? m_caster : static_cast<GameObject*>(m_trueCaster)->GetOwner();
}

WorldObject* Spell::GetCastingObject() const
{
    if (m_trueCaster->IsGameObject())
        return m_trueCaster;
    if (m_originalCasterGUID.IsGameObject())
        return m_trueCaster->IsInWorld() ? m_trueCaster->GetMap()->GetGameObject(m_originalCasterGUID) : nullptr;
    else if(m_originalCasterGUID.IsDynamicObject())
        return m_trueCaster->IsInWorld() ? m_trueCaster->GetMap()->GetDynamicObject(m_originalCasterGUID) : nullptr;

    return m_trueCaster;
}

float Spell::GetSpellSpeed() const
{
    if (m_trueCaster->IsGameObject()) // 4 spells in all of wotlk and doesnt seem like GO casting supports travelling delay from sniffs
        return 0.f;
    if (IsChanneledSpell(m_spellInfo))
        return 0.f;

    if (m_overrideSpeed)
        return m_overridenSpeed;
    
    return m_spellInfo->speed;
}

bool Spell::IsDelayedSpell() const
{
    return GetSpellSpeed() > 0.0f && !IsChanneledSpell(m_spellInfo);
}

void Spell::ResetEffectDamageAndHeal()
{
    m_damage = 0;
    m_healing = 0;
}

bool Spell::CanExecuteTriggersOnHit(uint8 effMask, SpellEntry const* triggeredByAura, bool auraTarget) const
{
    bool onlyOnTarget = (triggeredByAura && triggeredByAura->HasAttribute(SPELL_ATTR_EX4_CLASS_TRIGGER_ONLY_ON_TARGET));
    // If triggeredByAura has SPELL_ATTR4_PROC_ONLY_ON_CASTER then it can only proc on a cast spell with TARGET_UNIT_CASTER
    for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if ((effMask & (1 << i)) && (!onlyOnTarget || (onlyOnTarget && auraTarget)))
            return true;

    return false;
}

void Spell::ProcSpellAuraTriggers()
{
    if (!m_caster)
        return;

    Unit::AuraList const& targetTriggers = m_caster->GetAurasByType(SPELL_AURA_ADD_TARGET_TRIGGER);
    for (auto targetTrigger : targetTriggers)
    {
        if (!targetTrigger->isAffectedOnSpell(m_spellInfo))
            continue;
        for (TargetList::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
        {
            if (ihit->missCondition == SPELL_MISS_NONE)
            {
                Unit* target = m_caster->GetObjectGuid() == ihit->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit->targetGUID);
                if (!target || !target->IsAlive())
                    continue;
                SpellEntry const* auraSpellInfo = targetTrigger->GetSpellProto();
                if (!CanExecuteTriggersOnHit(ihit->effectHitMask, auraSpellInfo, targetTrigger->GetTarget()->GetObjectGuid() == ihit->targetGUID))
                    continue;
                SpellEffectIndex auraSpellIdx = targetTrigger->GetEffIndex();
                const uint32 procid = auraSpellInfo->EffectTriggerSpell[auraSpellIdx];
                int32 auraBasePoints = targetTrigger->GetBasePoints();
                // Calculate chance at that moment (can be depend for example from combo points)
                int32 chance = m_caster->CalculateSpellEffectValue(target, auraSpellInfo, auraSpellIdx, &auraBasePoints);
                if (roll_chance_i(chance))
                {
                    Spell* spell = new Spell(m_caster, sSpellTemplate.LookupEntry<SpellEntry>(procid), TRIGGERED_OLD_TRIGGERED, ObjectGuid(), nullptr);
                    SpellCastTargets targets;
                    targets.setUnitTarget(target);
                    spell->SetOverridenSpeed(GetSpellSpeed());
                    spell->SpellStart(&targets, targetTrigger);
                }
            }
        }
    }
}

bool Spell::IsInterruptible() const
{
    return (m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_COMBAT) && m_spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE && CanBeInterrupted();
}

void Spell::RegisterAuraProc(Aura* aura)
{
    m_procOnceHolder.insert(aura);
}

bool Spell::IsAuraProcced(Aura* aura)
{
    return m_procOnceHolder.find(aura) != m_procOnceHolder.end();
}

void Spell::ClearCastItem()
{
    if (m_CastItem == m_targets.getItemTarget())
        m_targets.clearItemPointer();

    m_CastItem = nullptr;
}

void Spell::GetSpellRangeAndRadius(SpellEffectIndex effIndex, float& radius, bool targetB, uint32& EffectChainTarget)
{
    if (m_spellInfo->EffectRadiusIndex[effIndex])
        radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(m_spellInfo->EffectRadiusIndex[effIndex]));
    else
        radius = GetSpellMaxRange(sSpellRangeStore.LookupEntry(m_spellInfo->rangeIndex));

    uint32 targetMode;
    if (!targetB)
        targetMode = m_spellInfo->EffectImplicitTargetA[effIndex];
    else
        targetMode = m_spellInfo->EffectImplicitTargetB[effIndex];
    SpellTargetInfo& data = SpellTargetInfoTable[targetMode];
    if (radius == 50000.f) // safety against bad data
    {
        if (data.filter != TARGET_SCRIPT)
        {
            switch (data.type)
            {
                case TARGET_TYPE_LOCATION_DEST:
                    if (data.filter == TARGET_SCRIPT) // TODO: Fix this whole shebang
                        radius = 100.f;
                    else
                        radius = 3.0f;
                    break;
                case TARGET_TYPE_UNIT:
                case TARGET_TYPE_GAMEOBJECT:
                    if (data.enumerator == TARGET_ENUMERATOR_CHAIN || data.enumerator == TARGET_ENUMERATOR_AOE || data.enumerator == TARGET_ENUMERATOR_CONE)
                        radius = 200.f;
                    break;
                default: break;
            }
        }
    }
    else if (radius == 0.f && data.type == TARGET_TYPE_LOCATION_DEST)
    {
        switch (targetMode) // TODO: move this to SQL
        {
            case TARGET_LOCATION_CASTER_FRONT_RIGHT:
            case TARGET_LOCATION_CASTER_BACK_RIGHT:
            case TARGET_LOCATION_CASTER_BACK_LEFT:
            case TARGET_LOCATION_CASTER_FRONT_LEFT:
            case TARGET_LOCATION_CASTER_FRONT:
            case TARGET_LOCATION_CASTER_BACK:
            case TARGET_LOCATION_CASTER_LEFT:
            case TARGET_LOCATION_CASTER_RIGHT:
                if (radius == 0.f && m_spellInfo->EffectRadiusIndex[effIndex] != 36) // All shaman totems have 0 radius - need to override with proper value
                    radius = 2.f;
                break;
        }
    }

    if (Unit* realCaster = GetAffectiveCaster())
    {
        if (Player* modOwner = realCaster->GetSpellModOwner())
        {
            modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_RADIUS, radius);
            if (!targetB)
                modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_JUMP_TARGETS, EffectChainTarget);
        }
    }

    // AOE leeway - 2f when caster is moving
    if (radius != 50000.f && m_caster && m_caster->IsMoving() && !m_caster->IsWalking())
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
                        radius = 0.3f * (60000 - auraHolder->GetAuraDuration()) * 0.001f;
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

    OnRadiusCalculate(effIndex, targetB, radius);
}

float Spell::GetCone()
{
    if (SpellCone const* coneData = sSpellCones.LookupEntry<SpellCone>(sSpellMgr.GetFirstSpellInChain(m_spellInfo->Id)))
        return G3D::toRadians(coneData->coneAngle);

    return M_PI_F / 3.f; // 60 degrees is default
}

void Spell::ProcReflectProcs(TargetInfo& targetInfo)
{
    if (!m_caster)
        return;

    Unit* target = m_caster->GetObjectGuid() == targetInfo.targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_trueCaster, targetInfo.targetGUID);
    if (!target)
        return;

    // Victim reflects, apply reflect procs
    Unit::ProcDamageAndSpell(ProcSystemArguments(m_caster, target, PROC_FLAG_NONE, PROC_FLAG_TAKE_HARMFUL_SPELL, PROC_EX_REFLECT, 1, 0, BASE_ATTACK, m_spellInfo));
    if (targetInfo.reflectResult == SPELL_MISS_REFLECT)
        // Apply reflect procs on self
        Unit::ProcDamageAndSpell(ProcSystemArguments(m_caster, m_caster, PROC_FLAG_NONE, PROC_FLAG_TAKE_HARMFUL_SPELL, PROC_EX_REFLECT, 1, 0, BASE_ATTACK, m_spellInfo));
}

void Spell::FilterTargetMap(UnitList& filterUnitList, SpellTargetFilterScheme scheme, uint32 chainTargetCount)
{
    switch (scheme)
    {
        case SCHEME_RANDOM:
        {
            if (m_affectedTargetCount && filterUnitList.size() > m_affectedTargetCount)
            {
                // remove random units from the map
                while (filterUnitList.size() > m_affectedTargetCount)
                {
                    uint32 poz = urand(0, filterUnitList.size() - 1);
                    auto itr = filterUnitList.begin();
                    std::advance(itr, poz);
                    itr = filterUnitList.erase(itr);
                }
            }
            break;
        }
        case SCHEME_CLOSEST:
        {
            if (m_affectedTargetCount && filterUnitList.size() > m_affectedTargetCount)
            {
                filterUnitList.sort(TargetDistanceOrderNear(m_trueCaster));
                filterUnitList.resize(m_affectedTargetCount);
            }
            break;
        }
        case SCHEME_FURTHEST:
        {
            if (m_affectedTargetCount && filterUnitList.size() > m_affectedTargetCount)
            {
                filterUnitList.sort(TargetDistanceOrderFarAway(m_trueCaster));
                filterUnitList.resize(m_affectedTargetCount);
            }
            break;
        }
        case SCHEME_HIGHEST_HP:
        {
            // Returns the target with the highest HP in melee range
            Unit* hatedTarget = nullptr;
            uint32 maxHP = 0;
            for (auto& unit : filterUnitList)
            {
                if (m_caster->CanReachWithMeleeAttack(unit))
                {
                    if (unit->GetHealth() > maxHP)
                    {
                        maxHP = unit->GetHealth();
                        hatedTarget = unit;
                    }
                }
            }
            if (!hatedTarget)
                return;

            filterUnitList.clear();
            filterUnitList.push_back(hatedTarget);
            break;
        }
        case SCHEME_RANDOM_CHAIN:
        {
            Unit* unitTarget = m_targets.getUnitTarget();
            if (filterUnitList.empty() || filterUnitList.front() != unitTarget)
                break;
            if (chainTargetCount > 1 && filterUnitList.size() > chainTargetCount)
            {
                // remove random units from the map
                while (filterUnitList.size() > chainTargetCount)
                {
                    uint32 poz = urand(1, filterUnitList.size() - 1);
                    auto itr = filterUnitList.begin();
                    std::advance(itr, poz);
                    itr = filterUnitList.erase(itr);
                }
            }
            break;
        }
        case SCHEME_CLOSEST_CHAIN:
        {
            Unit* unitTarget = m_targets.getUnitTarget();
            if (filterUnitList.empty() || filterUnitList.front() != unitTarget)
                break;
            UnitList newList;
            newList.push_back(unitTarget);
            filterUnitList.pop_front();
            filterUnitList.sort(TargetDistanceOrderNear(unitTarget));
            Unit* prev = unitTarget;
            UnitList::iterator next = filterUnitList.begin();
            chainTargetCount -= 1; // unit target is one

            while (chainTargetCount && next != filterUnitList.end())
            {
                if (prev->GetDistance(*next, true, DIST_CALC_NONE) > m_jumpRadius * m_jumpRadius)
                    break;

                if (!prev->IsWithinLOSInMap(*next, true))
                {
                    ++next;
                    continue;
                }
                prev = *next;
                newList.push_back(prev);
                filterUnitList.erase(next);
                filterUnitList.sort(TargetDistanceOrderNear(prev));
                next = filterUnitList.begin();

                --chainTargetCount;
            }
            std::swap(filterUnitList, newList);
            break;
        }
        case SCHEME_LOWEST_HP_CHAIN:
        {
            Unit* unitTarget = m_targets.getUnitTarget();
            if (filterUnitList.empty() || filterUnitList.front() != unitTarget)
                break;
            UnitList newList;
            newList.push_back(unitTarget);
            bool isInGroup = m_caster->IsInGroup(unitTarget);
            filterUnitList.pop_front();
            filterUnitList.sort(LowestHPNearestOrder(unitTarget));
            Unit* prev = unitTarget;
            UnitList::iterator next = filterUnitList.begin();
            chainTargetCount -= 1; // unit target is one

            while (chainTargetCount && next != filterUnitList.end())
            {
                // since we do not iterate through closest but through HP, we must check all units
                if (prev->GetDistance(*next, true, DIST_CALC_NONE) > m_jumpRadius * m_jumpRadius)
                {
                    ++next;
                    continue;
                }

                if (!prev->IsWithinLOSInMap(*next, true))
                {
                    ++next;
                    continue;
                }

                if (isInGroup && !m_caster->IsInGroup(*next))
                {
                    ++next;
                    continue;
                }

                prev = *next;
                newList.push_back(prev);
                filterUnitList.erase(next);
                filterUnitList.sort(LowestHPNearestOrder(prev));
                next = filterUnitList.begin();

                --chainTargetCount;
            }
            std::swap(filterUnitList, newList);
            break;
        }
        case SCHEME_PRIORITIZE_HEALTH:
        {
            PrioritizeHealthUnitQueue healthQueue;
            for (Unit* unit : filterUnitList)
                if (!unit->IsDead())
                    healthQueue.push(PrioritizeHealthUnitWraper(unit));

            filterUnitList.clear();
            while (!healthQueue.empty() && filterUnitList.size() < m_affectedTargetCount)
            {
                filterUnitList.push_back(healthQueue.top().getUnit());
                healthQueue.pop();
            }
            break;
        }
        case SCHEME_PRIORITIZE_MANA:
        {
            PrioritizeManaUnitQueue manaUsers;
            for (Unit* unit : filterUnitList)
                if (unit->GetPowerType() == POWER_MANA && !unit->IsDead())
                    manaUsers.push(PrioritizeManaUnitWraper(unit));

            filterUnitList.clear();
            while (!manaUsers.empty() && filterUnitList.size() < m_affectedTargetCount)
            {
                filterUnitList.push_back(manaUsers.top().getUnit());
                manaUsers.pop();
            }
            break;
        }
    }
}

void Spell::FillFromTargetFlags(TempTargetingData& targetingData, SpellEffectIndex effIdx)
{
    if (m_spellInfo->Targets & (TARGET_FLAG_UNIT_ALLY | TARGET_FLAG_UNIT | TARGET_FLAG_UNIT_ENEMY))
    {
        if (Unit* unit = m_targets.getUnitTarget())
            targetingData.data[effIdx].tmpUnitList[false].push_back(unit);
    }
    else if (m_spellInfo->Targets & (TARGET_FLAG_CORPSE_ENEMY | TARGET_FLAG_CORPSE_ALLY))
    {
        if (Unit* unit = m_targets.getUnitTarget())
            targetingData.data[effIdx].tmpUnitList[false].push_back(unit);
        else if (m_targets.getCorpseTargetGuid())
        {
            if (Corpse* corpse = m_targets.getCorpseTarget())
                targetingData.data[effIdx].tempCorpseList.push_back(corpse);
        }
    }
    else if (m_spellInfo->Targets & (TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM))
    {
        if (Item* item = m_targets.getItemTarget())
            targetingData.data[effIdx].tempItemList.push_back(item);
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

bool Spell::IsEffectWithImplementedMultiplier(uint32 effectId) const
{
    // TODO: extend this for all effects that do damage and healing
    switch (effectId)
    {
        case SPELL_EFFECT_SCHOOL_DAMAGE:
        case SPELL_EFFECT_HEAL:
        case SPELL_EFFECT_POWER_BURN:
        case SPELL_EFFECT_HEAL_MECHANICAL:
        // weapon based
        case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
        case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
        case SPELL_EFFECT_WEAPON_DAMAGE:
        case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
            return true;
        default:
            return false;
    }
}

void Spell::StopCast(SpellCastResult castResult)
{
    SendCastResult(castResult);
    SendInterrupted(castResult);
    finish(false);
    m_trueCaster->DecreaseCastCounter();
    SetExecutedCurrently(false);
}

void Spell::SetOverridenSpeed(float newSpeed)
{
    m_overrideSpeed = true;
    m_overridenSpeed = newSpeed;
}

void Spell::SetDamageDoneModifier(float mod, SpellEffectIndex effIdx)
{
    m_damageDoneMultiplier[effIdx] = mod;
}

void Spell::OnInit()
{
    if (SpellScript* script = GetSpellScript())
        script->OnInit(this);
}

void Spell::OnSuccessfulStart()
{
    if (SpellScript* script = GetSpellScript())
        script->OnSuccessfulStart(this);
}

void Spell::OnSuccessfulFinish()
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
        default:
            if (SpellScript* script = GetSpellScript())
                script->OnSuccessfulFinish(this);
            break;
    }
}

SpellCastResult Spell::OnCheckCast(bool strict)
{
    switch (m_spellInfo->Id)
    {
        case 7914: // Capture Spirit
        {
            if (ObjectGuid target = m_targets.getUnitTargetGuid()) // can be cast only on these targets
                if (target.GetEntry() != 4663 && target.GetEntry() != 4664 && target.GetEntry() != 4665 && target.GetEntry() != 4666 && target.GetEntry() != 4667
                    && target.GetEntry() != 4668 && target.GetEntry() != 4705 && target.GetEntry() != 13019)
                    return SPELL_FAILED_BAD_TARGETS;
            break;
        }
        case 12699: // Summon Screecher Spirit
        {
            Unit* target = m_targets.getUnitTarget();
            if (!target || !target->IsCreature() || static_cast<Creature*>(target)->HasBeenHitBySpell(m_spellInfo->Id))
                return SPELL_FAILED_BAD_TARGETS;
            if (!strict)
            {
                static_cast<Creature*>(target)->RegisterHitBySpell(m_spellInfo->Id);
                return SPELL_CAST_OK;
            }
            break;
        }
        default:
            if (SpellScript* script = GetSpellScript())
                return script->OnCheckCast(this, strict);
            break;
    }

    return SPELL_CAST_OK;
}

void Spell::OnEffectExecute(SpellEffectIndex effIndex)
{
    if (SpellScript* script = GetSpellScript())
        script->OnEffectExecute(this, effIndex);
}

void Spell::OnRadiusCalculate(SpellEffectIndex effIndex, bool targetB, float& radius)
{
    if (SpellScript* script = GetSpellScript())
        script->OnRadiusCalculate(this, effIndex, targetB, radius);
}

void Spell::OnDestTarget() // TODO
{
    if (SpellScript* script = GetSpellScript())
        script->OnDestTarget(this);
}

bool Spell::OnCheckTarget(GameObject* target, SpellEffectIndex eff) const
{
    switch (m_spellInfo->Id)
    {
        case 38054: // Random rocket missile
            if (target->GetLootState() == GO_ACTIVATED) // can only hit unused ones
                return false;
            break;
        default:
            if (SpellScript* script = GetSpellScript())
                return script->OnCheckTarget(this, target, eff);
            break;
    }
    return true;
}

bool Spell::OnCheckTarget(Unit* target, SpellEffectIndex eff) const
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
        case 14297: // Shadow Storm (exclude targets below 25 yards and above 40 yards. Max value is handled by spell effect radius)
        case 26546:
        case 26555:
        {
            float x, y, z;
            m_caster->GetPosition(x, y, z);
            if (target->GetDistance(x, y, z, DIST_CALC_COMBAT_REACH) < 25.f)
                return false;
            break;
        }
        case 16807:                                         // Mass Teleport
            if (target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED)) // spell targets only creatures
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
        default:
            if (SpellScript* script = GetSpellScript())
                return script->OnCheckTarget(this, target, eff);
            break;
    }

    return true;
}

void Spell::OnCast()
{
    if (SpellScript* script = GetSpellScript())
        return script->OnCast(this);
}

void Spell::OnHit(SpellMissInfo missInfo)
{
    if (SpellScript* script = GetSpellScript())
        return script->OnHit(this, missInfo);
}

void Spell::OnAfterHit()
{
    if (SpellScript* script = GetSpellScript())
        return script->OnAfterHit(this);
}

void Spell::OnSummon(GameObject* summon)
{
    if (SpellScript* script = GetSpellScript())
        return script->OnSummon(this, summon);
}

void Spell::OnSummon(Creature* summon)
{
    if (SpellScript* script = GetSpellScript())
        return script->OnSummon(this, summon);
}

void Spell::SetTotalTargetValueModifier(float modifier)
{
    m_damage = float(m_damage) * modifier;
}

SpellModRAII::SpellModRAII(Spell* spell, Player* modOwner, bool success, bool onlySave) : m_spell(spell), m_modOwner(modOwner), m_success(success), m_onlySave(onlySave)
{
    if (m_modOwner && !modOwner->GetSpellModSpell()) // only if first spell in depth
        m_modOwner->SetSpellModSpell(spell);
}

SpellModRAII::~SpellModRAII()
{
    if (m_modOwner && m_modOwner->GetSpellModSpell() == m_spell) // only if this spell is toplevel
    {
        if (!m_onlySave)
        {
            if (m_success)
                m_modOwner->RemoveSpellMods(m_spell->m_usedAuraCharges);
            else
                m_modOwner->ResetSpellModsDueToCanceledSpell(m_spell->m_usedAuraCharges);
        }
        m_modOwner->SetSpellModSpell(nullptr);
    }
}

MaNGOS::unique_weak_ptr<Spell> Spell::GetWeakPtr() const
{
    return m_spellEvent->GetSpellWeakPtr();
}
