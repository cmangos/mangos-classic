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
SDComment: Missing poison inside the tunnel in phase 2 ; worms and eyes behaviour is incorrect
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
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

    // Spells
    SPELL_DECREPIT_FEVER    = 29998,
    SPELL_MANA_BURN         = 29310,
    SPELL_TELEPORT_SELF     = 30211,
    SPELL_TELEPORT_PLAYERS  = 29273,
    SPELL_PLAGUE_CLOUD      = 29350,
    SPELL_PLAGUE_WAVE_SLOW  = 29351,                // Activates the traps during phase 1; triggers spell 30116, 30117, 30118, 30119 each 10 secs
    SPELL_PLAGUE_WAVE_FAST  = 30114,                // Activates the traps during phase 2; triggers spell 30116, 30117, 30118, 30119 each 3 secs

    MAX_PLAYERS_TELEPORT    = 3,

    NPC_WORLD_TRIGGER       = 15384                 // Control plague waves
};

struct boss_heiganAI : public ScriptedAI
{
    boss_heiganAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        Reset();
    }

    instance_naxxramas* m_pInstance;

    uint8 m_uiPhase;
    uint8 m_uiPhaseEruption;

    uint32 m_uiTeleportTimer;
    uint32 m_uiFeverTimer;
    uint32 m_uiManaBurnTimer;
    uint32 m_uiEruptionStartDelay;
    uint32 m_uiPhaseTimer;
    uint32 m_uiTauntTimer;
    uint32 m_uiStartChannelingTimer;
    uint32 m_uiEntranceDoorTimer;

    SelectAttackingTargetParams m_teleportParams;

    void ResetPhase()
    {
        m_uiPhaseEruption = 0;
        StopEruptions();

        m_uiFeverTimer = 4 * IN_MILLISECONDS;
        m_uiManaBurnTimer = 5 * IN_MILLISECONDS;
        m_uiTeleportTimer = urand(35, 45) * IN_MILLISECONDS;
        m_uiEruptionStartDelay = 100;                       // ASAP
        m_uiStartChannelingTimer = 100;                     // ASAP

        m_uiPhaseTimer = (m_uiPhase == PHASE_GROUND ? 90 : 45) * IN_MILLISECONDS;
    }

    void Reset() override
    {
        m_uiPhase = PHASE_GROUND;
        m_uiTauntTimer = urand(25, 90) * IN_MILLISECONDS;
        m_uiEntranceDoorTimer = 0;
        m_teleportParams.range.minRange = 0;
        m_teleportParams.range.maxRange = 40;
        ResetPhase();
        m_uiEruptionStartDelay = 5 * IN_MILLISECONDS;       // Override value from ResetPhase() on combat start only: 5 more seconds are given at that time

        m_uiEntranceDoorTimer = 15 * IN_MILLISECONDS;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_AGGRO2, m_creature); break;
            case 2: DoScriptText(SAY_AGGRO3, m_creature); break;
        }

        if (m_pInstance)
            m_pInstance->SetData(TYPE_HEIGAN, IN_PROGRESS);
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(SAY_SLAY, m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_HEIGAN, DONE);

        StopEruptions();
    }

    void EnterEvadeMode() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_HEIGAN, FAIL);

        StopEruptions();

        ScriptedAI::EnterEvadeMode();
    }

    void StartEruptions(uint32 spellId)
    {
        // Clear current plague waves controller spell before applying the new one
        if (Creature* trigger = GetClosestCreatureWithEntry(m_creature, NPC_WORLD_TRIGGER, 100.0f))
        {
            trigger->RemoveAllAuras();
            trigger->CastSpell(trigger, spellId, TRIGGERED_OLD_TRIGGERED);
        }
    }

    void StopEruptions()
    {
        // Reset Plague Waves
        if (Creature* trigger = GetClosestCreatureWithEntry(m_creature, NPC_WORLD_TRIGGER, 100.0f))
            trigger->RemoveAllAuras();
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiEntranceDoorTimer)
        {
            // Entrance Gate
            if (m_uiEntranceDoorTimer < uiDiff)
            {
                if (GameObject* door = m_pInstance->GetSingleGameObjectFromStorage(GO_PLAG_HEIG_ENTRY_DOOR))
                    door->SetGoState(GO_STATE_READY);
                m_uiEntranceDoorTimer = 0;
            }
            else
                m_uiEntranceDoorTimer -= uiDiff;
        }

        if (m_uiPhase == PHASE_GROUND)
        {
            // Teleport to platform
            if (m_uiPhaseTimer < uiDiff)
            {
                m_creature->GetMotionMaster()->MoveIdle();
                if (DoCastSpellIfCan(m_creature, SPELL_TELEPORT_SELF) == CAST_OK)
                {
                    m_uiPhase = PHASE_PLATFORM;
                    ResetPhase();
                    return;
                }
            }
            else
                m_uiPhaseTimer -= uiDiff;

            // Taunt
            if (m_uiTauntTimer < uiDiff)
            {
                switch (urand(0, 3))
                {
                    case 0: DoScriptText(SAY_TAUNT1, m_creature); break;
                    case 1: DoScriptText(SAY_TAUNT2, m_creature); break;
                    case 2: DoScriptText(SAY_TAUNT3, m_creature); break;
                    case 3: DoScriptText(SAY_TAUNT4, m_creature); break;
                }
                m_uiTauntTimer = urand(25, 90) * IN_MILLISECONDS;
            }
            else
                m_uiTauntTimer -= uiDiff;

            // Decrepit Fever
            if (m_uiFeverTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_DECREPIT_FEVER) == CAST_OK)
                    m_uiFeverTimer = 21 * IN_MILLISECONDS;
            }
            else
                m_uiFeverTimer -= uiDiff;

            // Mana Burn
            if (m_uiManaBurnTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_MANA_BURN) == CAST_OK)
                    m_uiManaBurnTimer = 10 * IN_MILLISECONDS;
            }
            else
                m_uiManaBurnTimer -= uiDiff;

            if (m_uiTeleportTimer)
            {
                if (m_uiTeleportTimer < uiDiff)
                {
                    std::vector<Unit*> targets;
                    m_creature->SelectAttackingTargets(targets, ATTACKING_TARGET_ALL_SUITABLE, 0, nullptr, SELECT_FLAG_PLAYER | SELECT_FLAG_SKIP_TANK, m_teleportParams);

                    if (targets.size() > MAX_PLAYERS_TELEPORT)
                    {
                        std::random_shuffle(targets.begin(), targets.end());
                        targets.resize(MAX_PLAYERS_TELEPORT);
                    }

                    if (!targets.empty())
                    {
                        for (auto& target : targets)
                            target->CastSpell(target, SPELL_TELEPORT_PLAYERS, TRIGGERED_OLD_TRIGGERED);
                    }
                    m_uiTeleportTimer = 0;                  // Only one player teleport per ground phase
                }
                else
                    m_uiTeleportTimer -= uiDiff;
            }

            DoMeleeAttackIfReady();
        }
        else                                                // Platform Phase
        {
            if (m_uiPhaseTimer <= uiDiff)                   // Return to fight
            {
                m_creature->InterruptNonMeleeSpells(true);
                m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());

                m_uiPhase = PHASE_GROUND;
                ResetPhase();
                return;
            }
            m_uiPhaseTimer -= uiDiff;

            if (m_uiStartChannelingTimer)
            {
                if (m_uiStartChannelingTimer <= uiDiff)
                {
                    DoScriptText(SAY_CHANNELING, m_creature);
                    DoCastSpellIfCan(m_creature, SPELL_PLAGUE_CLOUD);

                    // ToDo: fill the tunnel with poison - required further research
                    m_uiStartChannelingTimer = 0;           // no more
                }
                else
                    m_uiStartChannelingTimer -= uiDiff;
            }
        }

        // Handling of the plague waves by a trigger NPC, this is not related to melee attack or spell-casting
        if (m_uiEruptionStartDelay)
        {
            if (m_uiEruptionStartDelay < uiDiff)
            {
                if (m_uiPhase == PHASE_GROUND)
                    StartEruptions(SPELL_PLAGUE_WAVE_SLOW);
                else
                    StartEruptions(SPELL_PLAGUE_WAVE_FAST);
                m_uiEruptionStartDelay = 0;
            }
            else
                m_uiEruptionStartDelay -= uiDiff;
        }
    }
};

UnitAI* GetAI_boss_heigan(Creature* pCreature)
{
    return new boss_heiganAI(pCreature);
}

void AddSC_boss_heigan()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_heigan";
    pNewScript->GetAI = &GetAI_boss_heigan;
    pNewScript->RegisterSelf();
}
