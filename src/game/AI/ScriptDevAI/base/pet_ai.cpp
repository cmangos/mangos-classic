/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

/* ScriptData
SDName: ScriptedPetAI
SD%Complete: 50
SDComment: Intended to be used with Guardian/Protector/Minipets. Little/no control over when pet enter/leave combat. Must be considered to be under development.
SDCategory: Npc
EndScriptData */

#include "AI/ScriptDevAI/include/precompiled.h"
#include "pet_ai.h"

ScriptedPetAI::ScriptedPetAI(Creature* creature) : CreatureAI(creature)
{}

void ScriptedPetAI::AttackStart(Unit* who)
{
    if (who && m_creature->Attack(who, m_meleeEnabled))
        m_creature->GetMotionMaster()->MoveChase(who);
}

void ScriptedPetAI::AttackedBy(Unit* attacker)
{
    if (m_creature->getVictim())
        return;

    if (!HasReactState(REACT_PASSIVE) && m_creature->CanReachWithMeleeAttack(attacker))
        AttackStart(attacker);
}

void ScriptedPetAI::ResetPetCombat()
{
    Unit* owner = m_creature->GetMaster();

    if (owner && m_creature->GetCharmInfo() && m_creature->GetCharmInfo()->HasCommandState(COMMAND_FOLLOW))
    {
        m_creature->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
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

void ScriptedPetAI::UpdatePetAI(const uint32 /*diff*/)
{
    DoMeleeAttackIfReady();
}

void ScriptedPetAI::UpdateAI(const uint32 diff)
{
    if (!m_creature->isAlive())                             // should not be needed, isAlive is checked in mangos before calling UpdateAI
        return;

    // UpdateAllies() is done in the generic PetAI in Mangos, but we can't do this from script side.
    // Unclear what side effects this has, but is something to be resolved from Mangos.

    if (m_creature->getVictim())                            // in combat
    {
        if (!m_creature->CanAttack(m_creature->getVictim()))
        {
            // target no longer valid for pet, so either attack stops or new target are selected
            // doesn't normally reach this, because of how petAi is designed in Mangos. CombatStop
            // are called before this update diff, and then pet will already have no victim.
            ResetPetCombat();
            return;
        }

        // update when in combat
        UpdatePetAI(diff);
    }
    else if (m_creature->GetCharmInfo())
    {
        if (m_creature->isInCombat())
            m_creature->CombatStop(true, true);

        Unit* owner = m_creature->GetMaster();

        if (!owner)
            return;

        if (owner->isInCombat() && !HasReactState(REACT_PASSIVE))
        {
            // Not correct in all cases.
            // When mob initiate attack by spell, pet should not start attack before spell landed.
            AttackStart(owner->getAttackerForHelper());
        }
        else if (m_creature->GetCharmInfo()->HasCommandState(COMMAND_FOLLOW))
        {
            // not following, so start follow
            if (!m_creature->hasUnitState(UNIT_STAT_FOLLOW))
                m_creature->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);

            // update when not in combat
            UpdatePetOOCAI(diff);
        }
    }
}

void ScriptedPetAI::CombatStop()
{
    ResetPetCombat();
}
