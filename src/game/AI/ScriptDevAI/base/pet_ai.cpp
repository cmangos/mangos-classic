/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

/* ScriptData
SDName: ScriptedPetAI
SD%Complete: 50
SDComment: Intended to be used with Guardian/Protector/Minipets. Little/no control over when pet enter/leave combat. Must be considered to be under development.
SDCategory: Npc
EndScriptData */

#include "precompiled.h"
#include "pet_ai.h"

ScriptedPetAI::ScriptedPetAI(Creature* pCreature) : CreatureAI(pCreature)
{}

bool ScriptedPetAI::IsVisible(Unit* pWho) const
{
    return pWho && m_creature->IsWithinDist(pWho, VISIBLE_RANGE)
           && pWho->isVisibleForOrDetect(m_creature, m_creature, true);
}

void ScriptedPetAI::MoveInLineOfSight(Unit* pWho)
{
    if (m_creature->getVictim())
        return;

    if (!m_creature->GetCharmInfo() || !m_creature->GetCharmInfo()->HasReactState(REACT_AGGRESSIVE))
        return;

    if (m_creature->CanInitiateAttack() && pWho->isTargetableForAttack() &&
            m_creature->IsHostileTo(pWho) && pWho->isInAccessablePlaceFor(m_creature))
    {
        if (!m_creature->CanFly() && m_creature->GetDistanceZ(pWho) > CREATURE_Z_ATTACK_RANGE)
            return;

        if (m_creature->IsWithinDistInMap(pWho, m_creature->GetAttackDistance(pWho)) && m_creature->IsWithinLOSInMap(pWho))
        {
            pWho->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
            AttackStart(pWho);
        }
    }
}

void ScriptedPetAI::AttackStart(Unit* pWho)
{
    if (pWho && m_creature->Attack(pWho, true))
        m_creature->GetMotionMaster()->MoveChase(pWho);
}

void ScriptedPetAI::AttackedBy(Unit* pAttacker)
{
    if (m_creature->getVictim())
        return;

    if (m_creature->GetCharmInfo() && !m_creature->GetCharmInfo()->HasReactState(REACT_PASSIVE) &&
            m_creature->CanReachWithMeleeAttack(pAttacker))
        AttackStart(pAttacker);
}

void ScriptedPetAI::ResetPetCombat()
{
    Unit* pOwner = m_creature->GetCharmerOrOwner();

    if (pOwner && m_creature->GetCharmInfo() && m_creature->GetCharmInfo()->HasCommandState(COMMAND_FOLLOW))
    {
        m_creature->GetMotionMaster()->MoveFollow(pOwner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
    }
    else
    {
        m_creature->GetMotionMaster()->Clear(false);
        m_creature->GetMotionMaster()->MoveIdle();
    }

    m_creature->AttackStop();

    debug_log("SD2: ScriptedPetAI reset pet combat and stop attack.");
    Reset();
}

void ScriptedPetAI::UpdatePetAI(const uint32 /*uiDiff*/)
{
    DoMeleeAttackIfReady();
}

void ScriptedPetAI::UpdateAI(const uint32 uiDiff)
{
    if (!m_creature->isAlive())                             // should not be needed, isAlive is checked in mangos before calling UpdateAI
        return;

    // UpdateAllies() is done in the generic PetAI in Mangos, but we can't do this from script side.
    // Unclear what side effects this has, but is something to be resolved from Mangos.

    if (m_creature->getVictim())                            // in combat
    {
        if (!m_creature->getVictim()->isTargetableForAttack())
        {
            // target no longer valid for pet, so either attack stops or new target are selected
            // doesn't normally reach this, because of how petAi is designed in Mangos. CombatStop
            // are called before this update diff, and then pet will already have no victim.
            ResetPetCombat();
            return;
        }

        // update when in combat
        UpdatePetAI(uiDiff);
    }
    else if (m_creature->GetCharmInfo())
    {
        Unit* pOwner = m_creature->GetCharmerOrOwner();

        if (!pOwner)
            return;

        if (pOwner->isInCombat() && !m_creature->GetCharmInfo()->HasReactState(REACT_PASSIVE))
        {
            // Not correct in all cases.
            // When mob initiate attack by spell, pet should not start attack before spell landed.
            AttackStart(pOwner->getAttackerForHelper());
        }
        else if (m_creature->GetCharmInfo()->HasCommandState(COMMAND_FOLLOW))
        {
            // not following, so start follow
            if (!m_creature->hasUnitState(UNIT_STAT_FOLLOW))
                m_creature->GetMotionMaster()->MoveFollow(pOwner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);

            // update when not in combat
            UpdatePetOOCAI(uiDiff);
        }
    }
}
