/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#include "AI/ScriptDevAI/include/precompiled.h"
#include "Entities/Item.h"
#include "Spells/Spell.h"
#include "WorldPacket.h"
#include "Globals/ObjectMgr.h"
#include "Grids/Cell.h"
#include "Grids/CellImpl.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"

// Spell summary for ScriptedAI::SelectSpell
struct TSpellSummary
{
    uint8 Targets;                                          // set of enum SelectTarget
    uint8 Effects;                                          // set of enum SelectEffect
}* SpellSummary;

ScriptedAI::ScriptedAI(Creature* creature) : CreatureAI(creature),
    m_uiEvadeCheckCooldown(2500)
{}

/// This function shows if combat movement is enabled, overwrite for more info
void ScriptedAI::GetAIInformation(ChatHandler& reader)
{
    reader.PSendSysMessage("ScriptedAI, combat movement is %s", reader.GetOnOffStr(IsCombatMovement()));
}

/**
 * This function only calls Aggro, which is to be used for scripting purposes
 */
void ScriptedAI::EnterCombat(Unit* enemy)
{
    if (enemy)
        Aggro(enemy);
}

/**
 * Main update function, by default let the creature behave as expected by a mob (threat management and melee dmg)
 * Always handle here threat-management with m_creature->SelectHostileTarget()
 * Handle (if required) melee attack with DoMeleeAttackIfReady()
 * This is usally overwritten to support timers for ie spells
 */
void ScriptedAI::UpdateAI(const uint32 /*diff*/)
{
    // Check if we have a current target
    if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        return;

    DoMeleeAttackIfReady();
}

/**
 * This function cleans up the combat state if the creature evades
 * It will:
 * - Drop Auras
 * - Drop all threat
 * - Stop combat
 * - Move the creature home
 * - Clear tagging for loot
 * - call Reset()
 */
void ScriptedAI::EnterEvadeMode()
{
    UnitAI::EnterEvadeMode();

    Reset();
}

/// This function calls Reset() to reset variables as expected
void ScriptedAI::JustRespawned()
{
    Reset();
}

void ScriptedAI::DoStartMovement(Unit* victim)
{
    if (victim)
        m_creature->GetMotionMaster()->MoveChase(victim, m_attackDistance, m_attackAngle, m_moveFurther);
}

void ScriptedAI::DoStartNoMovement(Unit* victim)
{
    if (!victim)
        return;

    m_creature->GetMotionMaster()->MoveIdle();
    m_creature->StopMoving();
}

void ScriptedAI::DoStopAttack()
{
    if (m_creature->getVictim())
        m_creature->AttackStop();
}

void ScriptedAI::DoPlaySoundToSet(WorldObject* source, uint32 soundId)
{
    if (!source)
        return;

    if (!GetSoundEntriesStore()->LookupEntry(soundId))
    {
        script_error_log("Invalid soundId %u used in DoPlaySoundToSet (Source: TypeId %u, GUID %u)", soundId, source->GetTypeId(), source->GetGUIDLow());
        return;
    }

    source->PlayDirectSound(soundId);
}

SpellEntry const* ScriptedAI::SelectSpell(Unit* target, int32 school, int32 mechanic, SelectTarget selectTargets, uint32 powerCostMin, uint32 powerCostMax, float rangeMin, float rangeMax, SelectEffect selectEffects)
{
    // No target so we can't cast
    if (!target)
        return nullptr;

    // Silenced so we can't cast
    if (m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
        return nullptr;

    // Using the extended script system we first create a list of viable spells
    SpellEntry const* spellInfos[4];
    memset(spellInfos, 0, sizeof(SpellEntry*) * 4);

    uint32 spellCount = 0;

    // Check if each spell is viable(set it to null if not)
    for (uint8 i = 0; i < 4; ++i)
    {
        SpellEntry const* tempSpellInfo = GetSpellStore()->LookupEntry<SpellEntry>(m_creature->m_spells[i]);

        // This spell doesn't exist
        if (!tempSpellInfo)
            continue;

        // Targets and Effects checked first as most used restrictions
        // Check the spell targets if specified
        if (selectTargets && !(SpellSummary[m_creature->m_spells[i]].Targets & (1 << (selectTargets - 1))))
            continue;

        // Check the type of spell if we are looking for a specific spell type
        if (selectEffects && !(SpellSummary[m_creature->m_spells[i]].Effects & (1 << (selectEffects - 1))))
            continue;

        // Check for school if specified
        // if (uiSchool >= 0 && pTempSpell->SchoolMask & uiSchool)
        //    continue;

        // Check for spell mechanic if specified
        if (mechanic >= 0 && tempSpellInfo->Mechanic != (uint32)mechanic)
            continue;

        // Make sure that the spell uses the requested amount of power
        if (powerCostMin &&  tempSpellInfo->manaCost < powerCostMin)
            continue;

        if (powerCostMax && tempSpellInfo->manaCost > powerCostMax)
            continue;

        // Continue if we don't have the mana to actually cast this spell
        if (tempSpellInfo->manaCost > m_creature->GetPower((Powers)tempSpellInfo->powerType))
            continue;

        // Get the Range
        SpellRangeEntry const* tempRange = GetSpellRangeStore()->LookupEntry(tempSpellInfo->rangeIndex);

        // Spell has invalid range store so we can't use it
        if (!tempRange)
            continue;

        // Check if the spell meets our range requirements
        if (rangeMin && tempRange->maxRange < rangeMin)
            continue;

        if (rangeMax && tempRange->maxRange > rangeMax)
            continue;

        // Check if our target is in range
        if (m_creature->IsWithinDistInMap(target, tempRange->minRange) || !m_creature->IsWithinDistInMap(target, tempRange->maxRange))
            continue;

        // All good so lets add it to the spell list
        spellInfos[spellCount] = tempSpellInfo;
        ++spellCount;
    }

    // We got our usable spells so now lets randomly pick one
    if (!spellCount)
        return nullptr;

    return spellInfos[urand(0, spellCount - 1)];
}

bool ScriptedAI::CanCast(Unit* target, SpellEntry const* spellInfo, bool triggered)
{
    // No target so we can't cast
    if (!target || !spellInfo)
        return false;

    // Silenced so we can't cast
    if (!triggered && m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
        return false;

    // Check for power
    if (!triggered && m_creature->GetPower((Powers)spellInfo->powerType) < spellInfo->manaCost)
        return false;

    SpellRangeEntry const* tempRange = GetSpellRangeStore()->LookupEntry(spellInfo->rangeIndex);

    // Spell has invalid range store so we can't use it
    if (!tempRange)
        return false;

    // Unit is out of range of this spell
    if (!m_creature->IsInRange(target, tempRange->minRange, tempRange->maxRange))
        return false;

    return true;
}

void FillSpellSummary()
{
    SpellSummary = new TSpellSummary[GetSpellStore()->GetMaxEntry()];

    for (uint32 i = 0; i < GetSpellStore()->GetMaxEntry(); ++i)
    {
        SpellSummary[i].Effects = 0;
        SpellSummary[i].Targets = 0;

        SpellEntry const* tempSpell = GetSpellStore()->LookupEntry<SpellEntry>(i);
        // This spell doesn't exist
        if (!tempSpell)
            continue;

        for (uint8 j = 0; j < 3; ++j)
        {
            // Spell targets self
            if (tempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_CASTER)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SELF - 1);

            // Spell targets a single enemy
            if (tempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_ENEMY ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_LOCATION_CASTER_TARGET_POSITION)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SINGLE_ENEMY - 1);

            // Spell targets AoE at enemy
            if (tempSpell->EffectImplicitTargetA[j] == TARGET_ENUM_UNITS_ENEMY_AOE_AT_SRC_LOC ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_ENUM_UNITS_ENEMY_AOE_AT_DEST_LOC ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_LOCATION_CASTER_SRC ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_ENUM_UNITS_ENEMY_AOE_AT_DYNOBJ_LOC)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_AOE_ENEMY - 1);

            // Spell targets an enemy
            if (tempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_ENEMY ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_LOCATION_CASTER_TARGET_POSITION ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_ENUM_UNITS_ENEMY_AOE_AT_SRC_LOC ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_ENUM_UNITS_ENEMY_AOE_AT_DEST_LOC ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_LOCATION_CASTER_SRC ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_ENUM_UNITS_ENEMY_AOE_AT_DYNOBJ_LOC)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_ANY_ENEMY - 1);

            // Spell targets a single friend(or self)
            if (tempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_CASTER ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_FRIEND ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_PARTY)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SINGLE_FRIEND - 1);

            // Spell targets aoe friends
            if (tempSpell->EffectImplicitTargetA[j] == TARGET_ENUM_UNITS_PARTY_WITHIN_CASTER_RANGE ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_FRIEND_AND_PARTY ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_LOCATION_CASTER_SRC)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_AOE_FRIEND - 1);

            // Spell targets any friend(or self)
            if (tempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_CASTER ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_FRIEND ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_PARTY ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_ENUM_UNITS_PARTY_WITHIN_CASTER_RANGE ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_FRIEND_AND_PARTY ||
                    tempSpell->EffectImplicitTargetA[j] == TARGET_LOCATION_CASTER_SRC)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_ANY_FRIEND - 1);

            // Make sure that this spell includes a damage effect
            if (tempSpell->Effect[j] == SPELL_EFFECT_SCHOOL_DAMAGE ||
                    tempSpell->Effect[j] == SPELL_EFFECT_INSTAKILL ||
                    tempSpell->Effect[j] == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE ||
                    tempSpell->Effect[j] == SPELL_EFFECT_HEALTH_LEECH)
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_DAMAGE - 1);

            // Make sure that this spell includes a healing effect (or an apply aura with a periodic heal)
            if (tempSpell->Effect[j] == SPELL_EFFECT_HEAL ||
                    tempSpell->Effect[j] == SPELL_EFFECT_HEAL_MAX_HEALTH ||
                    tempSpell->Effect[j] == SPELL_EFFECT_HEAL_MECHANICAL ||
                    (tempSpell->Effect[j] == SPELL_EFFECT_APPLY_AURA  && tempSpell->EffectApplyAuraName[j] == 8))
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_HEALING - 1);

            // Make sure that this spell applies an aura
            if (tempSpell->Effect[j] == SPELL_EFFECT_APPLY_AURA)
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_AURA - 1);
        }
    }
}

void ScriptedAI::DoResetThreat()
{
    if (!m_creature->CanHaveThreatList() || m_creature->getThreatManager().isThreatListEmpty())
    {
        script_error_log("DoResetThreat called for creature that either cannot have threat list or has empty threat list (m_creature entry = %d)", m_creature->GetEntry());
        return;
    }

    ThreatList const& tList = m_creature->getThreatManager().getThreatList();
    for (auto itr : tList)
    {
        Unit* unit = m_creature->GetMap()->GetUnit(itr->getUnitGuid());

        if (unit && m_creature->getThreatManager().getThreat(unit))
            m_creature->getThreatManager().modifyThreatPercent(unit, -100);
    }
}

void ScriptedAI::DoTeleportPlayer(Unit* unit, float x, float y, float z, float ori)
{
    if (!unit)
        return;

    if (unit->GetTypeId() != TYPEID_PLAYER)
    {
        script_error_log("%s tried to teleport non-player (%s) to x: %f y:%f z: %f o: %f. Aborted.", m_creature->GetGuidStr().c_str(), unit->GetGuidStr().c_str(), x, y, z, ori);
        return;
    }

    ((Player*)unit)->TeleportTo(unit->GetMapId(), x, y, z, ori, TELE_TO_NOT_LEAVE_COMBAT);
}

CreatureList ScriptedAI::DoFindFriendlyCC(float range)
{
    CreatureList creatureList;

    MaNGOS::FriendlyCCedInRangeCheck u_check(m_creature, range);
    MaNGOS::CreatureListSearcher<MaNGOS::FriendlyCCedInRangeCheck> searcher(creatureList, u_check);

    Cell::VisitGridObjects(m_creature, searcher, range);

    return creatureList;
}

CreatureList ScriptedAI::DoFindFriendlyMissingBuff(float range, uint32 spellId)
{
    CreatureList creatureList;

    MaNGOS::FriendlyMissingBuffInRangeCheck u_check(m_creature, range, spellId);
    MaNGOS::CreatureListSearcher<MaNGOS::FriendlyMissingBuffInRangeCheck> searcher(creatureList, u_check);

    Cell::VisitGridObjects(m_creature, searcher, range);

    return creatureList;
}

Player* ScriptedAI::GetPlayerAtMinimumRange(float minimumRange) const
{
    Player* player = nullptr;

    MaNGOS::AnyPlayerInObjectRangeCheck check(m_creature, minimumRange);
    MaNGOS::PlayerSearcher<MaNGOS::AnyPlayerInObjectRangeCheck> searcher(player, check);

    Cell::VisitWorldObjects(m_creature, searcher, minimumRange);

    return player;
}

void ScriptedAI::SetEquipmentSlots(bool loadDefault, int32 mainHand, int32 offHand, int32 ranged)
{
    if (loadDefault)
    {
        m_creature->LoadEquipment(m_creature->GetCreatureInfo()->EquipmentTemplateId, true);
        return;
    }

    if (mainHand >= 0)
        m_creature->SetVirtualItem(VIRTUAL_ITEM_SLOT_0, mainHand);

    if (offHand >= 0)
        m_creature->SetVirtualItem(VIRTUAL_ITEM_SLOT_1, offHand);

    if (ranged >= 0)
        m_creature->SetVirtualItem(VIRTUAL_ITEM_SLOT_2, ranged);
}

// Hacklike storage used for misc creatures that are expected to evade of outside of a certain area.
// It is assumed the information is found elswehere and can be handled by mangos. So far no luck finding such information/way to extract it.
enum
{
    NPC_BROODLORD               = 12017,
};

bool ScriptedAI::EnterEvadeIfOutOfCombatArea(const uint32 diff)
{
    if (m_uiEvadeCheckCooldown < diff)
        m_uiEvadeCheckCooldown = 2500;
    else
    {
        m_uiEvadeCheckCooldown -= diff;
        return false;
    }

    if (m_creature->IsInEvadeMode() || !m_creature->getVictim())
        return false;

    float x = m_creature->GetPositionX();
    float y = m_creature->GetPositionY();
    float z = m_creature->GetPositionZ();

    switch (m_creature->GetEntry())
    {
        case NPC_BROODLORD:                                 // broodlord (not move down stairs)
            if (z > 448.60f)
                return false;
            break;

        default:
            script_error_log("EnterEvadeIfOutOfCombatArea used for creature entry %u, but does not have any definition.", m_creature->GetEntry());
            return false;
    }

    EnterEvadeMode();
    return true;
}

void ScriptedAI::DespawnGuids(GuidVector& spawns)
{
    for (ObjectGuid& guid : spawns)
        if (Creature* spawn = m_creature->GetMap()->GetCreature(guid))
            spawn->ForcedDespawn();
    spawns.clear();
}

void Scripted_NoMovementAI::GetAIInformation(ChatHandler& reader)
{
    reader.PSendSysMessage("Subclass of Scripted_NoMovementAI");
}

void Scripted_NoMovementAI::AttackStart(Unit* who)
{
    if (who && m_creature->Attack(who, m_meleeEnabled))
    {
        m_creature->AddThreat(who);
        m_creature->SetInCombatWith(who);
        who->SetInCombatWith(m_creature);

        DoStartNoMovement(who);
    }
}
