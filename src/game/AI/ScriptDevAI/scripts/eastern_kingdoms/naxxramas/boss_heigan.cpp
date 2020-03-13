/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
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

/* ScriptData
SDName: Boss_Heigan
SD%Complete: 80
SDComment: Missing poison inside the eye stalk tunnel in phase 2
           Candidate spell is 30122 (correct damage range and already used in encounter) but there is no evidence of this
           and of how the spell is cast and by who (no data found in sniffs as this part was removed in WotLK)
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "naxxramas.h"

enum
{
    PHASE_GROUND            = 1,
    PHASE_PLATFORM          = 2,

    SAY_AGGRO1              = -1533109,
    SAY_AGGRO2              = -1533110,
    SAY_AGGRO3              = -1533111,
    SAY_SLAY                = -1533112,
    SAY_TAUNT1              = -1533113,
    SAY_TAUNT2              = -1533114,
    SAY_TAUNT3              = -1533115,
    SAY_TAUNT4              = -1533117,
    SAY_CHANNELING          = -1533116,
    SAY_DEATH               = -1533118,

    // Heigan spells
    SPELL_DECREPIT_FEVER    = 29998,
    SPELL_MANA_BURN         = 29310,
    SPELL_TELEPORT_SELF     = 30211,
    SPELL_TELEPORT_PLAYERS  = 29273,
    SPELL_PLAGUE_CLOUD      = 29350,                // Channel spell periodically triggering spell 30122
    SPELL_PLAGUE_WAVE_SLOW  = 29351,                // Activates the traps during phase 1; triggers spell 30116, 30117, 30118, 30119 each 10 secs
    SPELL_PLAGUE_WAVE_FAST  = 30114,                // Activates the traps during phase 2; triggers spell 30116, 30117, 30118, 30119 each 3 secs

    MAX_PLAYERS_TELEPORT    = 3,

    NPC_PLAGUE_WAVE         = 17293,                // Control plague waves
    NPC_WORLD_TRIGGER       = 15384
};

struct boss_heiganAI : public ScriptedAI
{
    boss_heiganAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (instance_naxxramas*)creature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_instance;

    uint8 m_phase;
    uint8 m_phaseEruption;

    uint32 m_teleportTimer;
    uint32 m_feverTimer;
    uint32 m_manaBurnTimer;
    uint32 m_eruptionStartDelay;
    uint32 m_phaseTimer;
    uint32 m_tauntTimer;
    uint32 m_startChannelingTimer;
    uint32 m_entranceDoorTimer;

    SelectAttackingTargetParams m_teleportParams;

    void ResetPhase()
    {
        m_phaseEruption = 0;
        StopEruptions();

        m_feverTimer = 4 * IN_MILLISECONDS;
        m_manaBurnTimer = 5 * IN_MILLISECONDS;
        m_teleportTimer = urand(35, 45) * IN_MILLISECONDS;
        m_eruptionStartDelay = 100;                         // ASAP
        m_startChannelingTimer = 100;                       // ASAP

        m_phaseTimer = (m_phase == PHASE_GROUND ? 90 : 45) * IN_MILLISECONDS;
    }

    void Reset() override
    {
        m_phase = PHASE_GROUND;
        m_tauntTimer = urand(25, 90) * IN_MILLISECONDS;
        m_entranceDoorTimer = 0;
        m_teleportParams.range.minRange = 0;
        m_teleportParams.range.maxRange = 40;
        ResetPhase();
        m_eruptionStartDelay = 5 * IN_MILLISECONDS;         // Override value from ResetPhase() on combat start only: 5 more seconds are given at that time

        m_entranceDoorTimer = 15 * IN_MILLISECONDS;
    }

    void Aggro(Unit* /*who*/) override
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_AGGRO2, m_creature); break;
            case 2: DoScriptText(SAY_AGGRO3, m_creature); break;
        }

        if (m_instance)
            m_instance->SetData(TYPE_HEIGAN, IN_PROGRESS);
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        DoScriptText(SAY_SLAY, m_creature);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_instance)
            m_instance->SetData(TYPE_HEIGAN, DONE);

        StopEruptions();
    }

    void EnterEvadeMode() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_HEIGAN, FAIL);

        StopEruptions();

        ScriptedAI::EnterEvadeMode();
    }

    void StartEruptions(uint32 spellId)
    {
        // Clear current plague waves controller spell before applying the new one
        if (Creature* trigger = GetClosestCreatureWithEntry(m_creature, NPC_PLAGUE_WAVE, 100.0f))
        {
            trigger->RemoveAllAuras();
            trigger->CastSpell(trigger, spellId, TRIGGERED_OLD_TRIGGERED);
        }
    }

    void StopEruptions()
    {
        // Reset Plague Waves
        if (Creature* trigger = GetClosestCreatureWithEntry(m_creature, NPC_PLAGUE_WAVE, 100.0f))
            trigger->RemoveAllAuras();
    }

    void UpdateAI(const uint32 diff) override
    {
        // Do nothing if no target
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        if (m_entranceDoorTimer)
        {
            // Entrance Gate
            if (m_entranceDoorTimer < diff)
            {
                if (GameObject* door = m_instance->GetSingleGameObjectFromStorage(GO_PLAG_HEIG_ENTRY_DOOR))
                    door->SetGoState(GO_STATE_READY);
                m_entranceDoorTimer = 0;
            }
            else
                m_entranceDoorTimer -= diff;
        }

        if (m_phase == PHASE_GROUND)
        {
            // Teleport to platform
            if (m_phaseTimer < diff)
            {
                m_creature->GetMotionMaster()->MoveIdle();
                if (DoCastSpellIfCan(m_creature, SPELL_TELEPORT_SELF) == CAST_OK)
                {
                    m_phase = PHASE_PLATFORM;
                    ResetPhase();
                    return;
                }
            }
            else
                m_phaseTimer -= diff;

            // Taunt
            if (m_tauntTimer < diff)
            {
                switch (urand(0, 3))
                {
                    case 0: DoScriptText(SAY_TAUNT1, m_creature); break;
                    case 1: DoScriptText(SAY_TAUNT2, m_creature); break;
                    case 2: DoScriptText(SAY_TAUNT3, m_creature); break;
                    case 3: DoScriptText(SAY_TAUNT4, m_creature); break;
                }
                m_tauntTimer = urand(25, 90) * IN_MILLISECONDS;
            }
            else
                m_tauntTimer -= diff;

            // Decrepit Fever
            if (m_feverTimer < diff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_DECREPIT_FEVER) == CAST_OK)
                    m_feverTimer = 21 * IN_MILLISECONDS;
            }
            else
                m_feverTimer -= diff;

            // Mana Burn
            if (m_manaBurnTimer < diff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_MANA_BURN) == CAST_OK)
                    m_manaBurnTimer = 10 * IN_MILLISECONDS;
            }
            else
                m_manaBurnTimer -= diff;

            if (m_teleportTimer)
            {
                if (m_teleportTimer < diff)
                {
                    std::vector<Unit*> targets;
                    m_creature->SelectAttackingTargets(targets, ATTACKING_TARGET_ALL_SUITABLE, 0, nullptr, SELECT_FLAG_PLAYER | SELECT_FLAG_SKIP_TANK, m_teleportParams);

                    if (targets.size() > MAX_PLAYERS_TELEPORT)
                    {
                        std::shuffle(targets.begin(), targets.end(), *GetRandomGenerator());
                        targets.resize(MAX_PLAYERS_TELEPORT);
                    }

                    if (!targets.empty())
                    {
                        for (auto& target : targets)
                            target->CastSpell(target, SPELL_TELEPORT_PLAYERS, TRIGGERED_OLD_TRIGGERED);
                    }
                    m_teleportTimer = 0;                  // Only one player teleport per ground phase
                }
                else
                    m_teleportTimer -= diff;
            }

            DoMeleeAttackIfReady();
        }
        else                                                // Platform Phase
        {
            if (m_phaseTimer <= diff)                   // Return to fight
            {
                m_creature->InterruptNonMeleeSpells(true);
                m_creature->GetMotionMaster()->MoveChase(m_creature->GetVictim());

                m_phase = PHASE_GROUND;
                ResetPhase();
                return;
            }
            m_phaseTimer -= diff;

            if (m_startChannelingTimer)
            {
                if (m_startChannelingTimer <= diff)
                {
                    DoScriptText(SAY_CHANNELING, m_creature);
                    DoCastSpellIfCan(m_creature, SPELL_PLAGUE_CLOUD);

                    // ToDo: fill the tunnel with poison - required further research
                    m_startChannelingTimer = 0;           // no more
                }
                else
                    m_startChannelingTimer -= diff;
            }
        }

        // Handling of the plague waves by a trigger NPC, this is not related to melee attack or spell-casting
        if (m_eruptionStartDelay)
        {
            if (m_eruptionStartDelay < diff)
            {
                if (m_phase == PHASE_GROUND)
                    StartEruptions(SPELL_PLAGUE_WAVE_SLOW);
                else
                    StartEruptions(SPELL_PLAGUE_WAVE_FAST);
                m_eruptionStartDelay = 0;
            }
            else
                m_eruptionStartDelay -= diff;
        }
    }
};

UnitAI* GetAI_boss_heigan(Creature* creature)
{
    return new boss_heiganAI(creature);
}

/*###################
## npc_diseased_maggot
####################*/

struct npc_diseased_maggotAI : public ScriptedAI
{
    npc_diseased_maggotAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (instance_naxxramas*)creature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_instance;

    uint32 m_resetCheckTimer;

    void Reset() override
    {
        m_resetCheckTimer = 3 * IN_MILLISECONDS;
    }

    void UpdateAI(const uint32 diff) override
    {
        // Do nothing if no target
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        if (m_resetCheckTimer < diff)
        {
            // Check if we are in range of the trigger NPC in the middle of Heigan room, if so: force evade
            if (Creature* trigger = GetClosestCreatureWithEntry(m_creature, NPC_WORLD_TRIGGER, 45.0f))
                m_creature->AI()->EnterEvadeMode();
            m_resetCheckTimer = 3 * IN_MILLISECONDS;
        }
        else
            m_resetCheckTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_diseased_maggot(Creature* creature)
{
    return new npc_diseased_maggotAI(creature);
}

void AddSC_boss_heigan()
{
    Script* newScript = new Script;
    newScript->Name = "boss_heigan";
    newScript->GetAI = &GetAI_boss_heigan;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "npc_diseased_maggot";
    newScript->GetAI = &GetAI_npc_diseased_maggot;
    newScript->RegisterSelf();
    newScript = new Script;
}
