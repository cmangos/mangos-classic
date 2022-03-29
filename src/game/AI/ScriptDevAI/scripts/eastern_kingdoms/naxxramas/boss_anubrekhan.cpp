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
SD%Complete: 99
SDComment: Corpse Scarabs summon from dead Crypt Guard may need improvement: timer and spell logic
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
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
    SPELL_SPAWN_CORPSE_SCARABS  = 28961,                    // Trigger 28864 Summon 10 Corpse Scarabs from dead Crypt Guard
//  SPELL_DELAY_SUMMON_SCARABS1 = 28992,                    // Probably used to prevent two dead Crypt Guards to explode in a short time
//  SPELL_DELAY_SUMMON_SCARABS2 = 28994,                    // Probably used to prevent two dead Crypt Guards to explode in a short time by triggering same spell cooldown than 28961
};

static const DialogueEntry introDialogue[] =
{
    {SAY_GREET1,  NPC_ANUB_REKHAN,  7000},
    {SAY_GREET2,  NPC_ANUB_REKHAN,  13000},
    {SAY_GREET3,  NPC_ANUB_REKHAN,  11000},
    {SAY_GREET4,  NPC_ANUB_REKHAN,  10000},
    {SAY_GREET5,  NPC_ANUB_REKHAN,  0},
};

struct boss_anubrekhanAI : public ScriptedAI
{
    boss_anubrekhanAI(Creature* creature) : ScriptedAI(creature),
        m_introDialogue(introDialogue)
    {
        m_instance = (instance_naxxramas*)creature->GetInstanceData();
        m_introDialogue.InitializeDialogueHelper(m_instance);
        m_hasDoneIntro = false;
        Reset();

        DoCastSpellIfCan(m_creature, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    instance_naxxramas* m_instance;
    DialogueHelper m_introDialogue;

    uint32 m_impaleTimer;
    uint32 m_locustSwarmTimer;
    uint32 m_summonTimer;
    uint32 m_corpseScarabsTimer;
    bool   m_hasDoneIntro;

    void Reset() override
    {
        m_impaleTimer           = 15 * IN_MILLISECONDS;
        m_locustSwarmTimer      = urand(80, 120) * IN_MILLISECONDS;
        m_summonTimer           = 0;
        m_corpseScarabsTimer    = urand(65, 105) * IN_MILLISECONDS;
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        DoScriptText(SAY_SLAY, m_creature);
    }

    void Aggro(Unit* /*who*/) override
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_AGGRO2, m_creature); break;
            case 2: DoScriptText(SAY_AGGRO3, m_creature); break;
        }

        DoCastSpellIfCan(m_creature, SPELL_ANUB_AURA, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);

        if (m_instance)
            m_instance->SetData(TYPE_ANUB_REKHAN, IN_PROGRESS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_ANUB_REKHAN, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_ANUB_REKHAN, FAIL);

        DoCastSpellIfCan(m_creature, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void StartIntro()
    {
        if (!m_hasDoneIntro)
        {
            m_introDialogue.StartNextDialogueText(SAY_GREET1);
            m_hasDoneIntro = true;
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        summoned->SetInCombatWithZone();
    }

    void EnterEvadeMode() override
    {
        // We despawn the guardians before entering evade mode to prevent despawning also the static adds that are linked to respawn on evade
        DoCastSpellIfCan(m_creature, SPELL_DESPAWN_GUARDS, CAST_TRIGGERED);

        ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(const uint32 diff) override
    {
        m_introDialogue.DialogueUpdate(diff);

        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        // Impale
        if (m_impaleTimer < diff)
        {
            // Cast Impale on a random target
            // Do NOT cast it when we are afflicted by locust swarm
            if (!m_creature->HasAura(SPELL_LOCUSTSWARM))
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    DoCastSpellIfCan(target, SPELL_IMPALE);
            }

            m_impaleTimer = urand(12, 18) * IN_MILLISECONDS;
        }
        else
            m_impaleTimer -= diff;

        // Locust Swarm
        if (m_locustSwarmTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_LOCUSTSWARM) == CAST_OK)
            {
                // Summon a crypt guard
                m_summonTimer = 3 * IN_MILLISECONDS;
                m_locustSwarmTimer = 90 * IN_MILLISECONDS;
                m_impaleTimer += 23 * IN_MILLISECONDS;    // Delay next Impale by Locust Swarm duration (20 sec) + casting time (3 sec), this prevent Impale to be always cast right after Locust Swarm ends
            }
        }
        else
            m_locustSwarmTimer -= diff;

        // Summon Crypt Guard after Locust Swarm
        if (m_summonTimer)
        {
            if (m_summonTimer <= diff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_GUARD) == CAST_OK)
                    m_summonTimer = 0;
            }
            else
                m_summonTimer -= diff;
        }

        // Summon Corpse Scarabs from dead Crypt Guard
        if (m_corpseScarabsTimer)
        {
            if (m_corpseScarabsTimer <= diff)
            {
                if (DoCastSpellIfCan(nullptr, SPELL_SPAWN_CORPSE_SCARABS) == CAST_OK)
                    m_corpseScarabsTimer = urand(65, 105) * IN_MILLISECONDS;
            }
            else
                m_corpseScarabsTimer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_anubrekhan(Creature* creature)
{
    return new boss_anubrekhanAI(creature);
}

bool GOUse_go_anub_door(Player* /*pPlayer*/, GameObject* go)
{
    if (ScriptedInstance* instance = (ScriptedInstance*)go->GetInstanceData())
    {
        if (instance->GetData(TYPE_ANUB_REKHAN) == IN_PROGRESS || instance->GetData(TYPE_ANUB_REKHAN) == DONE) // GOs always open once used (or handled by script), so this check is only there for safety
            return false;
        else
        {
            if (Creature* anub = instance->GetSingleCreatureFromStorage(NPC_ANUB_REKHAN))
            {
                if (boss_anubrekhanAI* anubAI = dynamic_cast<boss_anubrekhanAI*>(anub->AI()))
                    anubAI->StartIntro();
            }
        }
    }
    return false;
}

void AddSC_boss_anubrekhan()
{
    Script* newScript = new Script;
    newScript->Name = "boss_anubrekhan";
    newScript->GetAI = &GetAI_boss_anubrekhan;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "go_anub_door";
    newScript->pGOUse = &GOUse_go_anub_door;
    newScript->RegisterSelf();
}
