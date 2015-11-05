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
SDName: Boss_Maexxna
SD%Complete: 90
SDComment: Web wrap effect still needs more love and research.
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "naxxramas.h"

enum
{
    EMOTE_SPIN_WEB              = -1533146,
    EMOTE_SPIDERLING            = -1533147,
    EMOTE_SPRAY                 = -1533148,
    EMOTE_BOSS_GENERIC_FRENZY   = -1000005,

    SPELL_DOUBLE_ATTACK         = 19818,

    SPELL_WEBWRAP               = 28622,
    SPELL_WEBWRAP_2             = 28673,                    // purpose unknown

    SPELL_WEBSPRAY              = 29484,
    SPELL_POISONSHOCK           = 28741,
    SPELL_NECROTICPOISON        = 28776,
    SPELL_FRENZY                = 28747,

    // SPELL_SUMMON_SPIDERLING_1 = 29434,                   // removed from dbc. Summons 10 spiderlings
    // SPELL_SUMMON_SPIDERLING_2 = 30076,                   // removed from dbc. Summons 3 spiderlings
    // SPELL_SUMMON_WEB_WRAP     = 28627,                   // removed from dbc. Summons one web wrap and transforms it into creature 17286

    NPC_WEB_WRAP                = 16486,
    NPC_SPIDERLING              = 17055,

    MAX_SPIDERLINGS             = 8,
    MAX_WEB_WRAP_POSITIONS      = 3,
};

static const float aWebWrapLoc[MAX_WEB_WRAP_POSITIONS][3] =
{
    {3546.796f, -3869.082f, 296.450f},
    {3531.271f, -3847.424f, 299.450f},
    {3497.067f, -3843.384f, 302.384f}
};

struct npc_web_wrapAI : public ScriptedAI
{
    npc_web_wrapAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    ObjectGuid m_victimGuid;
    uint32 m_uiWebWrapTimer;

    void Reset() override
    {
        m_uiWebWrapTimer = 0;
    }

    void MoveInLineOfSight(Unit* /*pWho*/) override {}
    void AttackStart(Unit* /*pWho*/) override {}

    void SetVictim(Unit* pVictim)
    {
        if (pVictim && pVictim->GetTypeId() == TYPEID_PLAYER)
        {
            // Vanilla spell 28618, 28619, 28620, 28621 had effect SPELL_EFFECT_PLAYER_PULL with EffectMiscValue = 200, 300, 400 and 500
            // All these spells trigger 28622 after 1 or 2 seconds
            // the EffectMiscValue may have been based on the distance between the victim and the target

            // NOTE: This implementation may not be 100% correct, but it gets very close to the expected result

            float fDist = m_creature->GetDistance2d(pVictim);
            // Switch the speed multiplier based on the distance from the web wrap
            uint32 uiEffectMiscValue = 500;
            if (fDist < 25.0f)
                uiEffectMiscValue = 200;
            else if (fDist < 50.0f)
                uiEffectMiscValue = 300;
            else if (fDist < 75.0f)
                uiEffectMiscValue = 400;

            // This doesn't give the expected result in all cases
            ((Player*)pVictim)->KnockBackFrom(m_creature, -fDist, uiEffectMiscValue * 0.033f);

            // Jump movement not supported on 2.4.3
            // float fSpeed = fDist * (uiEffectMiscValue * 0.01f);
            // pVictim->GetMotionMaster()->MoveJump(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), fSpeed, 0.0f);

            m_victimGuid = pVictim->GetObjectGuid();
            m_uiWebWrapTimer = uiEffectMiscValue == 200 ? 1000 : 2000;
        }
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_victimGuid)
        {
            if (Player* pVictim = m_creature->GetMap()->GetPlayer(m_victimGuid))
            {
                if (pVictim->isAlive())
                    pVictim->RemoveAurasDueToSpell(SPELL_WEBWRAP);
            }
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiWebWrapTimer)
        {
            // Finally the player gets web wrapped and he should change the display id until the creature is killed
            if (m_uiWebWrapTimer <= uiDiff)
            {
                if (Player* pVictim = m_creature->GetMap()->GetPlayer(m_victimGuid))
                    pVictim->CastSpell(pVictim, SPELL_WEBWRAP, true, nullptr, nullptr, m_creature->GetObjectGuid());

                m_uiWebWrapTimer = 0;
            }
            else
                m_uiWebWrapTimer -= uiDiff;
        }
    }
};

struct boss_maexxnaAI : public ScriptedAI
{
    boss_maexxnaAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        Reset();

        DoCastSpellIfCan(m_creature, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    instance_naxxramas* m_pInstance;

    uint32 m_uiWebWrapTimer;
    uint32 m_uiWebSprayTimer;
    uint32 m_uiPoisonShockTimer;
    uint32 m_uiNecroticPoisonTimer;
    uint32 m_uiSummonSpiderlingTimer;
    bool   m_bEnraged;

    void Reset() override
    {
        m_uiWebWrapTimer            = 15000;
        m_uiWebSprayTimer           = 40000;
        m_uiPoisonShockTimer        = urand(10000, 20000);
        m_uiNecroticPoisonTimer     = urand(20000, 30000);
        m_uiSummonSpiderlingTimer   = 30000;
        m_bEnraged                  = false;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_MAEXXNA, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_MAEXXNA, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_MAEXXNA, FAIL);

        DoCastSpellIfCan(m_creature, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_WEB_WRAP)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1, SPELL_WEBWRAP, SELECT_FLAG_PLAYER))
            {
                if (npc_web_wrapAI* pWebAI = dynamic_cast<npc_web_wrapAI*>(pSummoned->AI()))
                    pWebAI->SetVictim(pTarget);
            }
        }
        else if (pSummoned->GetEntry() == NPC_SPIDERLING)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                pSummoned->AI()->AttackStart(pTarget);
        }
    }

    bool DoCastWebWrap()
    {
        // If we can't select a player for web wrap then skip the summoning
        if (!m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1, uint32(0), SELECT_FLAG_PLAYER))
            return false;

        uint8 uiPos1 = urand(0, MAX_WEB_WRAP_POSITIONS - 1);
        m_creature->SummonCreature(NPC_WEB_WRAP, aWebWrapLoc[uiPos1][0], aWebWrapLoc[uiPos1][1], aWebWrapLoc[uiPos1][2], 0, TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 60000);

        uint8 uiPos2 = (uiPos1 + urand(1, MAX_WEB_WRAP_POSITIONS - 1)) % MAX_WEB_WRAP_POSITIONS;
        m_creature->SummonCreature(NPC_WEB_WRAP, aWebWrapLoc[uiPos2][0], aWebWrapLoc[uiPos2][1], aWebWrapLoc[uiPos2][2], 0, TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 60000);

        return true;
    }

    // Summons spiderlings around the boss
    void SummonSpiderlings()
    {
        for (uint8 i = 0; i < MAX_SPIDERLINGS; ++i)
            m_creature->SummonCreature(NPC_SPIDERLING, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 60000);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Web Wrap
        if (m_uiWebWrapTimer < uiDiff)
        {
            if (DoCastWebWrap())
                DoScriptText(EMOTE_SPIN_WEB, m_creature);

            m_uiWebWrapTimer = 40000;
        }
        else
            m_uiWebWrapTimer -= uiDiff;

        // Web Spray
        if (m_uiWebSprayTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_WEBSPRAY) == CAST_OK)
            {
                DoScriptText(EMOTE_SPRAY, m_creature);
                m_uiWebSprayTimer = 40000;
            }
        }
        else
            m_uiWebSprayTimer -= uiDiff;

        // Poison Shock
        if (m_uiPoisonShockTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_POISONSHOCK) == CAST_OK)
                m_uiPoisonShockTimer = urand(10000, 20000);
        }
        else
            m_uiPoisonShockTimer -= uiDiff;

        // Necrotic Poison
        if (m_uiNecroticPoisonTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_NECROTICPOISON) == CAST_OK)
                m_uiNecroticPoisonTimer = urand(20000, 30000);
        }
        else
            m_uiNecroticPoisonTimer -= uiDiff;

        // Summon Spiderling
        if (m_uiSummonSpiderlingTimer < uiDiff)
        {
            SummonSpiderlings();
            DoScriptText(EMOTE_SPIDERLING, m_creature);
            m_uiSummonSpiderlingTimer = 30000;
        }
        else
            m_uiSummonSpiderlingTimer -= uiDiff;

        // Enrage if not already enraged and below 30%
        if (!m_bEnraged && m_creature->GetHealthPercent() < 30.0f)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FRENZY) == CAST_OK)
            {
                DoScriptText(EMOTE_BOSS_GENERIC_FRENZY, m_creature);
                m_bEnraged = true;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_web_wrap(Creature* pCreature)
{
    return new npc_web_wrapAI(pCreature);
}

CreatureAI* GetAI_boss_maexxna(Creature* pCreature)
{
    return new boss_maexxnaAI(pCreature);
}

void AddSC_boss_maexxna()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_maexxna";
    pNewScript->GetAI = &GetAI_boss_maexxna;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_web_wrap";
    pNewScript->GetAI = &GetAI_npc_web_wrap;
    pNewScript->RegisterSelf();
}
