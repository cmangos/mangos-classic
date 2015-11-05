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
SDName: Boss_Gluth
SD%Complete: 95
SDComment: Gluth should turn around to face the victim when he devours a Zombie
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "naxxramas.h"

enum
{
    EMOTE_ZOMBIE                    = -1533119,
    EMOTE_BOSS_GENERIC_ENRAGED      = -1000006,
    EMOTE_DECIMATE                  = -1533152,

    SPELL_DOUBLE_ATTACK             = 19818,
    SPELL_MORTALWOUND               = 25646,
    SPELL_DECIMATE                  = 28374,
    SPELL_ENRAGE                    = 28371,
    SPELL_BERSERK                   = 26662,
    SPELL_TERRIFYING_ROAR           = 29685,
    // SPELL_SUMMON_ZOMBIE_CHOW      = 28216,               // removed from dbc: triggers 28217 every 6 secs
    // SPELL_CALL_ALL_ZOMBIE_CHOW    = 29681,               // removed from dbc: triggers 29682
    // SPELL_ZOMBIE_CHOW_SEARCH      = 28235,               // removed from dbc: triggers 28236 every 3 secs

    NPC_ZOMBIE_CHOW                 = 16360,                // old vanilla summoning spell 28217

    MAX_ZOMBIE_LOCATIONS            = 3,
};

static const float aZombieSummonLoc[MAX_ZOMBIE_LOCATIONS][3] =
{
    {3267.9f, -3172.1f, 297.42f},
    {3253.2f, -3132.3f, 297.42f},
    {3308.3f, -3185.8f, 297.42f},
};

struct boss_gluthAI : public ScriptedAI
{
    boss_gluthAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        Reset();

        DoCastSpellIfCan(m_creature, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    instance_naxxramas* m_pInstance;

    uint32 m_uiMortalWoundTimer;
    uint32 m_uiDecimateTimer;
    uint32 m_uiEnrageTimer;
    uint32 m_uiRoarTimer;
    uint32 m_uiSummonTimer;
    uint32 m_uiZombieSearchTimer;

    uint32 m_uiBerserkTimer;

    GuidList m_lZombieChowGuidList;

    void Reset() override
    {
        m_uiMortalWoundTimer  = 10000;
        m_uiDecimateTimer     = 110000;
        m_uiEnrageTimer       = 25000;
        m_uiSummonTimer       = 6000;
        m_uiRoarTimer         = 15000;
        m_uiZombieSearchTimer = 3000;

        m_uiBerserkTimer      = MINUTE * 8 * IN_MILLISECONDS;
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GLUTH, DONE);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GLUTH, IN_PROGRESS);
    }

    void KilledUnit(Unit* pVictim) override
    {
        // Restore 5% hp when killing a zombie
        if (pVictim->GetEntry() == NPC_ZOMBIE_CHOW)
        {
            DoScriptText(EMOTE_ZOMBIE, m_creature);
            m_creature->SetHealth(m_creature->GetHealth() + m_creature->GetMaxHealth() * 0.05f);
        }
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GLUTH, FAIL);

        DoCastSpellIfCan(m_creature, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        pSummoned->GetMotionMaster()->MoveFollow(m_creature, ATTACK_DISTANCE, 0);
        m_lZombieChowGuidList.push_back(pSummoned->GetObjectGuid());
    }

    void SummonedCreatureDespawn(Creature* pSummoned) override
    {
        m_lZombieChowGuidList.remove(pSummoned->GetObjectGuid());
    }

    // Replaces missing spell 29682
    void DoCallAllZombieChow()
    {
        for (GuidList::const_iterator itr = m_lZombieChowGuidList.begin(); itr != m_lZombieChowGuidList.end(); ++itr)
        {
            if (Creature* pZombie = m_creature->GetMap()->GetCreature(*itr))
                pZombie->GetMotionMaster()->MoveFollow(m_creature, ATTACK_DISTANCE, 0);
        }
    }

    // Replaces missing spell 28236
    void DoSearchZombieChow()
    {
        for (GuidList::const_iterator itr = m_lZombieChowGuidList.begin(); itr != m_lZombieChowGuidList.end(); ++itr)
        {
            if (Creature* pZombie = m_creature->GetMap()->GetCreature(*itr))
            {
                if (!pZombie->isAlive())
                    continue;

                // Devour a Zombie
                if (pZombie->IsWithinDistInMap(m_creature, 15.0f))
                    m_creature->DealDamage(pZombie, pZombie->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
            }
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiZombieSearchTimer < uiDiff)
        {
            DoSearchZombieChow();
            m_uiZombieSearchTimer = 3000;
        }
        else
            m_uiZombieSearchTimer -= uiDiff;

        // Mortal Wound
        if (m_uiMortalWoundTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MORTALWOUND) == CAST_OK)
                m_uiMortalWoundTimer = 10000;
        }
        else
            m_uiMortalWoundTimer -= uiDiff;

        // Decimate
        if (m_uiDecimateTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_DECIMATE) == CAST_OK)
            {
                DoScriptText(EMOTE_DECIMATE, m_creature);
                DoCallAllZombieChow();
                m_uiDecimateTimer = 100000;
            }
        }
        else
            m_uiDecimateTimer -= uiDiff;

        // Enrage
        if (m_uiEnrageTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
            {
                DoScriptText(EMOTE_BOSS_GENERIC_ENRAGED, m_creature);
                m_uiEnrageTimer = urand(20000, 30000);
            }
        }
        else
            m_uiEnrageTimer -= uiDiff;

        // Terrifying Roar
        if (m_uiRoarTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_TERRIFYING_ROAR) == CAST_OK)
                m_uiRoarTimer = 20000;
        }
        else
            m_uiRoarTimer -= uiDiff;

        // Summon
        if (m_uiSummonTimer < uiDiff)
        {
            uint8 uiPos1 = urand(0, MAX_ZOMBIE_LOCATIONS - 1);
            m_creature->SummonCreature(NPC_ZOMBIE_CHOW, aZombieSummonLoc[uiPos1][0], aZombieSummonLoc[uiPos1][1], aZombieSummonLoc[uiPos1][2], 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0);

            uint8 uiPos2 = (uiPos1 + urand(1, MAX_ZOMBIE_LOCATIONS - 1)) % MAX_ZOMBIE_LOCATIONS;
            m_creature->SummonCreature(NPC_ZOMBIE_CHOW, aZombieSummonLoc[uiPos2][0], aZombieSummonLoc[uiPos2][1], aZombieSummonLoc[uiPos2][2], 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0);

            m_uiSummonTimer = 6000;
        }
        else
            m_uiSummonTimer -= uiDiff;

        // Berserk
        if (m_uiBerserkTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BERSERK) == CAST_OK)
                m_uiBerserkTimer = MINUTE * 5 * IN_MILLISECONDS;
        }
        else
            m_uiBerserkTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_gluth(Creature* pCreature)
{
    return new boss_gluthAI(pCreature);
}

void AddSC_boss_gluth()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_gluth";
    pNewScript->GetAI = &GetAI_boss_gluth;
    pNewScript->RegisterSelf();
}
