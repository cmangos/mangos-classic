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
SDName: Boss_Anubrekhan
SD%Complete: 98
SDComment: Corpse Scarabs summon from dead Crypt Guard may need improvement as it currently does not work on the two initial ones
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "naxxramas.h"

enum
{
    SAY_GREET1                  = -1533000,
    SAY_GREET2                  = -1533004,
    SAY_GREET3                  = -1533005,
    SAY_GREET4                  = -1533006,
    SAY_GREET5                  = -1533007,
    SAY_AGGRO1                  = -1533001,
    SAY_AGGRO2                  = -1533002,
    SAY_AGGRO3                  = -1533003,
    SAY_SLAY                    = -1533008,

    SPELL_DOUBLE_ATTACK         = 18943,
    SPELL_IMPALE                = 28783,                    // May be wrong spell id. Causes more dmg than I expect
    SPELL_LOCUSTSWARM           = 28785,                    // This is a self buff that triggers the dmg debuff
    SPELL_ANUB_AURA             = 29103,                    // Periodically apply aura 29104 onto players to handle the corpse scarabs summon when they die (spell 29105)

    SPELL_SUMMON_GUARD          = 29508,                    // Summons 1 Crypt Guard at targeted location
    SPELL_DESPAWN_GUARDS        = 29379,                    // Remove all Crypt Guards and Corpse Scarabs
    SPELL_SELF_SPAWN_10         = 28864,                    // This is used by the crypt guards when they die

    NPC_CRYPT_GUARD             = 16573
};

static const DialogueEntry aIntroDialogue[] =
{
    {SAY_GREET1,  NPC_ANUB_REKHAN,  7000},
    {SAY_GREET2,  NPC_ANUB_REKHAN,  13000},
    {SAY_GREET3,  NPC_ANUB_REKHAN,  11000},
    {SAY_GREET4,  NPC_ANUB_REKHAN,  10000},
    {SAY_GREET5,  NPC_ANUB_REKHAN,  0},
};

struct boss_anubrekhanAI : public ScriptedAI
{
    boss_anubrekhanAI(Creature* pCreature) : ScriptedAI(pCreature),
        m_introDialogue(aIntroDialogue)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        m_introDialogue.InitializeDialogueHelper(m_pInstance);
        m_bHasDoneIntro = false;
        Reset();

        DoCastSpellIfCan(m_creature, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    instance_naxxramas* m_pInstance;
    DialogueHelper m_introDialogue;

    uint32 m_uiImpaleTimer;
    uint32 m_uiLocustSwarmTimer;
    uint32 m_uiSummonTimer;
    bool   m_bHasDoneIntro;

    void Reset() override
    {
        m_uiImpaleTimer         = 15 * IN_MILLISECONDS;
        m_uiLocustSwarmTimer	= urand(80, 120) * IN_MILLISECONDS;
        m_uiSummonTimer         = 0;
    }

    void KilledUnit(Unit* pVictim) override
    {
        DoScriptText(SAY_SLAY, m_creature);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_AGGRO2, m_creature); break;
            case 2: DoScriptText(SAY_AGGRO3, m_creature); break;
        }

        DoCastSpellIfCan(m_creature, SPELL_ANUB_AURA, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_ANUB_REKHAN, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_ANUB_REKHAN, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_ANUB_REKHAN, FAIL);

        DoCastSpellIfCan(m_creature, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void StartIntro()
    {
        if (!m_bHasDoneIntro)
        {
            m_introDialogue.StartNextDialogueText(SAY_GREET1);
            m_bHasDoneIntro = true;
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        pSummoned->SetInCombatWithZone();
    }

    void SummonedCreatureDespawn(Creature* pSummoned) override
    {
        // If creature despawns on out of combat, skip this
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (pSummoned->GetEntry() == NPC_CRYPT_GUARD)
            pSummoned->CastSpell(pSummoned, SPELL_SELF_SPAWN_10, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_creature->GetObjectGuid());
    }

    void EnterEvadeMode() override
    {
        // We despawn the guardians before entering evade mode to prevent despawning also the static adds that are linked to respawn on evade
        DoCastSpellIfCan(m_creature, SPELL_DESPAWN_GUARDS, CAST_TRIGGERED);

        ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        m_introDialogue.DialogueUpdate(uiDiff);

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Impale
        if (m_uiImpaleTimer < uiDiff)
        {
            // Cast Impale on a random target
            // Do NOT cast it when we are afflicted by locust swarm
            if (!m_creature->HasAura(SPELL_LOCUSTSWARM))
            {
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    DoCastSpellIfCan(pTarget, SPELL_IMPALE);
            }

            m_uiImpaleTimer = urand(12, 18) * IN_MILLISECONDS;
        }
        else
            m_uiImpaleTimer -= uiDiff;

        // Locust Swarm
        if (m_uiLocustSwarmTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_LOCUSTSWARM) == CAST_OK)
            {
                // Summon a crypt guard
                m_uiSummonTimer = 3 * IN_MILLISECONDS;
                m_uiLocustSwarmTimer = 90 * IN_MILLISECONDS;
                m_uiImpaleTimer += 23 * IN_MILLISECONDS;    // Delay next Impale by Locust Swarm duration (20 sec) + casting time (3 sec), this prevent Impale to be always cast right after Locust Swarm ends
            }
        }
        else
            m_uiLocustSwarmTimer -= uiDiff;

        // Summon Crypt Guard after Locust Swarm
        if (m_uiSummonTimer)
        {
            if (m_uiSummonTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_GUARD) == CAST_OK)
                    m_uiSummonTimer = 0;
            }
            else
                m_uiSummonTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_anubrekhan(Creature* pCreature)
{
    return new boss_anubrekhanAI(pCreature);
}

bool GOUse_go_anub_door(Player* /*pPlayer*/, GameObject* pGo)
{
    if (ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData())
    {
        if (pInstance->GetData(TYPE_ANUB_REKHAN) == IN_PROGRESS || pInstance->GetData(TYPE_ANUB_REKHAN) == DONE) // GOs always open once used (or handled by script), so this check is only there for safety
            return false;
        else
        {
            if (Creature* pAnub = pInstance->GetSingleCreatureFromStorage(NPC_ANUB_REKHAN))
            {
                if (boss_anubrekhanAI* pAnubAI = dynamic_cast<boss_anubrekhanAI*>(pAnub->AI()))
                    pAnubAI->StartIntro();
            }
        }
    }
    return false;
}

void AddSC_boss_anubrekhan()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_anubrekhan";
    pNewScript->GetAI = &GetAI_boss_anubrekhan;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_anub_door";
    pNewScript->pGOUse = &GOUse_go_anub_door;
    pNewScript->RegisterSelf();
}
