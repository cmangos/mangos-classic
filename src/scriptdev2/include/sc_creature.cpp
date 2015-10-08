/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#include "precompiled.h"
#include "Item.h"
#include "Spell.h"
#include "WorldPacket.h"
#include "ObjectMgr.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

// Spell summary for ScriptedAI::SelectSpell
struct TSpellSummary
{
    uint8 Targets;                                          // set of enum SelectTarget
    uint8 Effects;                                          // set of enum SelectEffect
}* SpellSummary;

ScriptedAI::ScriptedAI(Creature* pCreature) : CreatureAI(pCreature),
    m_uiEvadeCheckCooldown(2500)
{}

/// This function shows if combat movement is enabled, overwrite for more info
void ScriptedAI::GetAIInformation(ChatHandler& reader)
{
    reader.PSendSysMessage("ScriptedAI, combat movement is %s", reader.GetOnOffStr(IsCombatMovement()));
}

/// Return if the creature can "see" pWho
bool ScriptedAI::IsVisible(Unit* pWho) const
{
    if (!pWho)
        return false;

    return m_creature->IsWithinDist(pWho, VISIBLE_RANGE) && pWho->isVisibleForOrDetect(m_creature, m_creature, true);
}

/**
 * This function triggers the creature attacking pWho, depending on conditions like:
 * - Can the creature start an attack?
 * - Is pWho hostile to the creature?
 * - Can the creature reach pWho?
 * - Is pWho in aggro-range?
 * If the creature can attack pWho, it will if it has no victim.
 * Inside dungeons, the creature will get into combat with pWho, even if it has already a victim
 */
void ScriptedAI::MoveInLineOfSight(Unit* pWho)
{
    if (m_creature->CanInitiateAttack() && pWho->isTargetableForAttack() &&
            m_creature->IsHostileTo(pWho) && pWho->isInAccessablePlaceFor(m_creature))
    {
        if (!m_creature->CanFly() && m_creature->GetDistanceZ(pWho) > CREATURE_Z_ATTACK_RANGE)
            return;

        if (m_creature->IsWithinDistInMap(pWho, m_creature->GetAttackDistance(pWho)) && m_creature->IsWithinLOSInMap(pWho))
        {
            if (!m_creature->getVictim())
            {
                pWho->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
                AttackStart(pWho);
            }
            else if (m_creature->GetMap()->IsDungeon())
            {
                pWho->SetInCombatWith(m_creature);
                m_creature->AddThreat(pWho);
            }
        }
    }
}

/**
 * This function sets the TargetGuid for the creature if required
 * Also it will handle the combat movement (chase movement), depending on SetCombatMovement(bool)
 */
void ScriptedAI::AttackStart(Unit* pWho)
{
    if (pWho && m_creature->Attack(pWho, true))             // The Attack function also uses basic checks if pWho can be attacked
    {
        m_creature->AddThreat(pWho);
        m_creature->SetInCombatWith(pWho);
        pWho->SetInCombatWith(m_creature);

        HandleMovementOnAttackStart(pWho);
    }
}

/**
 * This function only calls Aggro, which is to be used for scripting purposes
 */
void ScriptedAI::EnterCombat(Unit* pEnemy)
{
    if (pEnemy)
        Aggro(pEnemy);
}

/**
 * Main update function, by default let the creature behave as expected by a mob (threat management and melee dmg)
 * Always handle here threat-management with m_creature->SelectHostileTarget()
 * Handle (if required) melee attack with DoMeleeAttackIfReady()
 * This is usally overwritten to support timers for ie spells
 */
void ScriptedAI::UpdateAI(const uint32 /*uiDiff*/)
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
    m_creature->RemoveAllAurasOnEvade();
    m_creature->DeleteThreatList();
    m_creature->CombatStop(true);

    if (m_creature->isAlive())
        m_creature->GetMotionMaster()->MoveTargetedHome();

    m_creature->SetLootRecipient(nullptr);

    Reset();
}

/// This function calls Reset() to reset variables as expected
void ScriptedAI::JustRespawned()
{
    Reset();
}

void ScriptedAI::DoStartMovement(Unit* pVictim, float fDistance, float fAngle)
{
    if (pVictim)
        m_creature->GetMotionMaster()->MoveChase(pVictim, fDistance, fAngle);
}

void ScriptedAI::DoStartNoMovement(Unit* pVictim)
{
    if (!pVictim)
        return;

    m_creature->GetMotionMaster()->MoveIdle();
    m_creature->StopMoving();
}

void ScriptedAI::DoStopAttack()
{
    if (m_creature->getVictim())
        m_creature->AttackStop();
}

void ScriptedAI::DoCast(Unit* pTarget, uint32 uiSpellId, bool bTriggered)
{
    if (m_creature->IsNonMeleeSpellCasted(false) && !bTriggered)
        return;

    m_creature->CastSpell(pTarget, uiSpellId, bTriggered);
}

void ScriptedAI::DoCastSpell(Unit* pTarget, SpellEntry const* pSpellInfo, bool bTriggered)
{
    if (m_creature->IsNonMeleeSpellCasted(false) && !bTriggered)
        return;

    m_creature->CastSpell(pTarget, pSpellInfo, bTriggered);
}

void ScriptedAI::DoPlaySoundToSet(WorldObject* pSource, uint32 uiSoundId)
{
    if (!pSource)
        return;

    if (!GetSoundEntriesStore()->LookupEntry(uiSoundId))
    {
        script_error_log("Invalid soundId %u used in DoPlaySoundToSet (Source: TypeId %u, GUID %u)", uiSoundId, pSource->GetTypeId(), pSource->GetGUIDLow());
        return;
    }

    pSource->PlayDirectSound(uiSoundId);
}

Creature* ScriptedAI::DoSpawnCreature(uint32 uiId, float fX, float fY, float fZ, float fAngle, uint32 uiType, uint32 uiDespawntime)
{
    return m_creature->SummonCreature(uiId, m_creature->GetPositionX() + fX, m_creature->GetPositionY() + fY, m_creature->GetPositionZ() + fZ, fAngle, (TempSummonType)uiType, uiDespawntime);
}

SpellEntry const* ScriptedAI::SelectSpell(Unit* pTarget, int32 uiSchool, int32 iMechanic, SelectTarget selectTargets, uint32 uiPowerCostMin, uint32 uiPowerCostMax, float fRangeMin, float fRangeMax, SelectEffect selectEffects)
{
    // No target so we can't cast
    if (!pTarget)
        return nullptr;

    // Silenced so we can't cast
    if (m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
        return nullptr;

    // Using the extended script system we first create a list of viable spells
    SpellEntry const* apSpell[4];
    memset(apSpell, 0, sizeof(SpellEntry*) * 4);

    uint32 uiSpellCount = 0;

    SpellEntry const* pTempSpell;
    SpellRangeEntry const* pTempRange;

    // Check if each spell is viable(set it to null if not)
    for (uint8 i = 0; i < 4; ++i)
    {
        pTempSpell = GetSpellStore()->LookupEntry(m_creature->m_spells[i]);

        // This spell doesn't exist
        if (!pTempSpell)
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
        if (iMechanic >= 0 && pTempSpell->Mechanic != (uint32)iMechanic)
            continue;

        // Make sure that the spell uses the requested amount of power
        if (uiPowerCostMin &&  pTempSpell->manaCost < uiPowerCostMin)
            continue;

        if (uiPowerCostMax && pTempSpell->manaCost > uiPowerCostMax)
            continue;

        // Continue if we don't have the mana to actually cast this spell
        if (pTempSpell->manaCost > m_creature->GetPower((Powers)pTempSpell->powerType))
            continue;

        // Get the Range
        pTempRange = GetSpellRangeStore()->LookupEntry(pTempSpell->rangeIndex);

        // Spell has invalid range store so we can't use it
        if (!pTempRange)
            continue;

        // Check if the spell meets our range requirements
        if (fRangeMin && pTempRange->maxRange < fRangeMin)
            continue;

        if (fRangeMax && pTempRange->maxRange > fRangeMax)
            continue;

        // Check if our target is in range
        if (m_creature->IsWithinDistInMap(pTarget, pTempRange->minRange) || !m_creature->IsWithinDistInMap(pTarget, pTempRange->maxRange))
            continue;

        // All good so lets add it to the spell list
        apSpell[uiSpellCount] = pTempSpell;
        ++uiSpellCount;
    }

    // We got our usable spells so now lets randomly pick one
    if (!uiSpellCount)
        return nullptr;

    return apSpell[urand(0, uiSpellCount - 1)];
}

bool ScriptedAI::CanCast(Unit* pTarget, SpellEntry const* pSpellEntry, bool bTriggered)
{
    // No target so we can't cast
    if (!pTarget || !pSpellEntry)
        return false;

    // Silenced so we can't cast
    if (!bTriggered && m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
        return false;

    // Check for power
    if (!bTriggered && m_creature->GetPower((Powers)pSpellEntry->powerType) < pSpellEntry->manaCost)
        return false;

    SpellRangeEntry const* pTempRange = GetSpellRangeStore()->LookupEntry(pSpellEntry->rangeIndex);

    // Spell has invalid range store so we can't use it
    if (!pTempRange)
        return false;

    // Unit is out of range of this spell
    if (!m_creature->IsInRange(pTarget, pTempRange->minRange, pTempRange->maxRange))
        return false;

    return true;
}

void FillSpellSummary()
{
    SpellSummary = new TSpellSummary[GetSpellStore()->GetNumRows()];

    SpellEntry const* pTempSpell;

    for (uint32 i = 0; i < GetSpellStore()->GetNumRows(); ++i)
    {
        SpellSummary[i].Effects = 0;
        SpellSummary[i].Targets = 0;

        pTempSpell = GetSpellStore()->LookupEntry(i);
        // This spell doesn't exist
        if (!pTempSpell)
            continue;

        for (uint8 j = 0; j < 3; ++j)
        {
            // Spell targets self
            if (pTempSpell->EffectImplicitTargetA[j] == TARGET_SELF)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SELF - 1);

            // Spell targets a single enemy
            if (pTempSpell->EffectImplicitTargetA[j] == TARGET_CHAIN_DAMAGE ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_CURRENT_ENEMY_COORDINATES)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SINGLE_ENEMY - 1);

            // Spell targets AoE at enemy
            if (pTempSpell->EffectImplicitTargetA[j] == TARGET_ALL_ENEMY_IN_AREA ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_ALL_ENEMY_IN_AREA_INSTANT ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_CASTER_COORDINATES ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_ALL_ENEMY_IN_AREA_CHANNELED)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_AOE_ENEMY - 1);

            // Spell targets an enemy
            if (pTempSpell->EffectImplicitTargetA[j] == TARGET_CHAIN_DAMAGE ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_CURRENT_ENEMY_COORDINATES ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_ALL_ENEMY_IN_AREA ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_ALL_ENEMY_IN_AREA_INSTANT ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_CASTER_COORDINATES ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_ALL_ENEMY_IN_AREA_CHANNELED)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_ANY_ENEMY - 1);

            // Spell targets a single friend(or self)
            if (pTempSpell->EffectImplicitTargetA[j] == TARGET_SELF ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_SINGLE_FRIEND ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_SINGLE_PARTY)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SINGLE_FRIEND - 1);

            // Spell targets aoe friends
            if (pTempSpell->EffectImplicitTargetA[j] == TARGET_ALL_PARTY_AROUND_CASTER ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_AREAEFFECT_PARTY ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_CASTER_COORDINATES)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_AOE_FRIEND - 1);

            // Spell targets any friend(or self)
            if (pTempSpell->EffectImplicitTargetA[j] == TARGET_SELF ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_SINGLE_FRIEND ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_SINGLE_PARTY ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_ALL_PARTY_AROUND_CASTER ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_AREAEFFECT_PARTY ||
                    pTempSpell->EffectImplicitTargetA[j] == TARGET_CASTER_COORDINATES)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_ANY_FRIEND - 1);

            // Make sure that this spell includes a damage effect
            if (pTempSpell->Effect[j] == SPELL_EFFECT_SCHOOL_DAMAGE ||
                    pTempSpell->Effect[j] == SPELL_EFFECT_INSTAKILL ||
                    pTempSpell->Effect[j] == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE ||
                    pTempSpell->Effect[j] == SPELL_EFFECT_HEALTH_LEECH)
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_DAMAGE - 1);

            // Make sure that this spell includes a healing effect (or an apply aura with a periodic heal)
            if (pTempSpell->Effect[j] == SPELL_EFFECT_HEAL ||
                    pTempSpell->Effect[j] == SPELL_EFFECT_HEAL_MAX_HEALTH ||
                    pTempSpell->Effect[j] == SPELL_EFFECT_HEAL_MECHANICAL ||
                    (pTempSpell->Effect[j] == SPELL_EFFECT_APPLY_AURA  && pTempSpell->EffectApplyAuraName[j] == 8))
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_HEALING - 1);

            // Make sure that this spell applies an aura
            if (pTempSpell->Effect[j] == SPELL_EFFECT_APPLY_AURA)
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
    for (ThreatList::const_iterator itr = tList.begin(); itr != tList.end(); ++itr)
    {
        Unit* pUnit = m_creature->GetMap()->GetUnit((*itr)->getUnitGuid());

        if (pUnit && m_creature->getThreatManager().getThreat(pUnit))
            m_creature->getThreatManager().modifyThreatPercent(pUnit, -100);
    }
}

void ScriptedAI::DoTeleportPlayer(Unit* pUnit, float fX, float fY, float fZ, float fO)
{
    if (!pUnit)
        return;

    if (pUnit->GetTypeId() != TYPEID_PLAYER)
    {
        script_error_log("%s tried to teleport non-player (%s) to x: %f y:%f z: %f o: %f. Aborted.", m_creature->GetGuidStr().c_str(), pUnit->GetGuidStr().c_str(), fX, fY, fZ, fO);
        return;
    }

    ((Player*)pUnit)->TeleportTo(pUnit->GetMapId(), fX, fY, fZ, fO, TELE_TO_NOT_LEAVE_COMBAT);
}

Unit* ScriptedAI::DoSelectLowestHpFriendly(float fRange, uint32 uiMinHPDiff)
{
    Unit* pUnit = nullptr;

    MaNGOS::MostHPMissingInRangeCheck u_check(m_creature, fRange, uiMinHPDiff);
    MaNGOS::UnitLastSearcher<MaNGOS::MostHPMissingInRangeCheck> searcher(pUnit, u_check);

    Cell::VisitGridObjects(m_creature, searcher, fRange);

    return pUnit;
}

std::list<Creature*> ScriptedAI::DoFindFriendlyCC(float fRange)
{
    std::list<Creature*> pList;

    MaNGOS::FriendlyCCedInRangeCheck u_check(m_creature, fRange);
    MaNGOS::CreatureListSearcher<MaNGOS::FriendlyCCedInRangeCheck> searcher(pList, u_check);

    Cell::VisitGridObjects(m_creature, searcher, fRange);

    return pList;
}

std::list<Creature*> ScriptedAI::DoFindFriendlyMissingBuff(float fRange, uint32 uiSpellId)
{
    std::list<Creature*> pList;

    MaNGOS::FriendlyMissingBuffInRangeCheck u_check(m_creature, fRange, uiSpellId);
    MaNGOS::CreatureListSearcher<MaNGOS::FriendlyMissingBuffInRangeCheck> searcher(pList, u_check);

    Cell::VisitGridObjects(m_creature, searcher, fRange);

    return pList;
}

Player* ScriptedAI::GetPlayerAtMinimumRange(float fMinimumRange)
{
    Player* pPlayer = nullptr;

    MaNGOS::AnyPlayerInObjectRangeCheck check(m_creature, fMinimumRange);
    MaNGOS::PlayerSearcher<MaNGOS::AnyPlayerInObjectRangeCheck> searcher(pPlayer, check);

    Cell::VisitWorldObjects(m_creature, searcher, fMinimumRange);

    return pPlayer;
}

void ScriptedAI::SetEquipmentSlots(bool bLoadDefault, int32 iMainHand, int32 iOffHand, int32 iRanged)
{
    if (bLoadDefault)
    {
        m_creature->LoadEquipment(m_creature->GetCreatureInfo()->EquipmentTemplateId, true);
        return;
    }

    if (iMainHand >= 0)
        m_creature->SetVirtualItem(VIRTUAL_ITEM_SLOT_0, iMainHand);

    if (iOffHand >= 0)
        m_creature->SetVirtualItem(VIRTUAL_ITEM_SLOT_1, iOffHand);

    if (iRanged >= 0)
        m_creature->SetVirtualItem(VIRTUAL_ITEM_SLOT_2, iRanged);
}

// Hacklike storage used for misc creatures that are expected to evade of outside of a certain area.
// It is assumed the information is found elswehere and can be handled by mangos. So far no luck finding such information/way to extract it.
enum
{
    NPC_BROODLORD               = 12017,
};

bool ScriptedAI::EnterEvadeIfOutOfCombatArea(const uint32 uiDiff)
{
    if (m_uiEvadeCheckCooldown < uiDiff)
        m_uiEvadeCheckCooldown = 2500;
    else
    {
        m_uiEvadeCheckCooldown -= uiDiff;
        return false;
    }

    if (m_creature->IsInEvadeMode() || !m_creature->getVictim())
        return false;

    float fX = m_creature->GetPositionX();
    float fY = m_creature->GetPositionY();
    float fZ = m_creature->GetPositionZ();

    switch (m_creature->GetEntry())
    {
        case NPC_BROODLORD:                                 // broodlord (not move down stairs)
            if (fZ > 448.60f)
                return false;
            break;
        default:
            script_error_log("EnterEvadeIfOutOfCombatArea used for creature entry %u, but does not have any definition.", m_creature->GetEntry());
            return false;
    }

    EnterEvadeMode();
    return true;
}

void Scripted_NoMovementAI::GetAIInformation(ChatHandler& reader)
{
    reader.PSendSysMessage("Subclass of Scripted_NoMovementAI");
}

void Scripted_NoMovementAI::AttackStart(Unit* pWho)
{
    if (pWho && m_creature->Attack(pWho, true))
    {
        m_creature->AddThreat(pWho);
        m_creature->SetInCombatWith(pWho);
        pWho->SetInCombatWith(m_creature);

        DoStartNoMovement(pWho);
    }
}
