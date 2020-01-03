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
SD%Complete: 100
SDComment:
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "naxxramas.h"

enum
{
    EMOTE_BOSS_GENERIC_FRENZY   = -1000005,

    // Maexxna Spells
    SPELL_DOUBLE_ATTACK         = 19818,

    SPELL_WEBSPRAY              = 29484,
    SPELL_POISONSHOCK           = 28741,
    SPELL_NECROTICPOISON        = 28776,
    SPELL_FRENZY                = 28747,
    SPELL_WEB_WRAP_INIT         = 28617,

    SPELL_SUMMON_SPIDERLING_1   = 29434,                    // Summons 10 spiderlings
//    SPELL_SUMMON_SPIDERLING_2 = 30076,                   // Summons 3 spiderlings

    // Web Wrap spells
    SPELL_WEB_WRAP_200          = 28618,
    SPELL_WEB_WRAP_300          = 28619,
    SPELL_WEB_WRAP_400          = 28620,
    SPELL_WEB_WRAP_500          = 28621,
//    SPELL_WEBWRAP_STUN          = 28622,                    // Triggered by spells 28618 - 28621
    SPELL_WEB_WRAP_SUMMON       = 28627,
    SPELL_CLEAR_WEB_WRAP_TARGET = 28628,
    SPELL_CLEAR_WEB_WRAP_SELF   = 28629,
    SPELL_SELF_STUN             = 29826,                    // Spell Id unsure

    NPC_SPIDERLING              = 17055,
    NPC_INVISIBLE_MAN           = 17286,                    // Handle the summoning of the players and Web Wrap NPCs

    MAX_PLAYERS_WEB_WRAP        = 3,

    TIMER_40_SEC                = 40000,                    // Used by all main abilities
};

/*###################
#   npc_web_wrap
###################*/

// This NPC is summoned by the web wrapped player and act as a visual target for other raid members to free the player
struct npc_web_wrapAI : public ScriptedAI
{
    npc_web_wrapAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    Player* m_player;
    uint32 m_uiCheckVictimAliveTimer;

    void Reset() override
    {
        SetCombatMovement(false);
        m_player = nullptr;
        m_uiCheckVictimAliveTimer = 1 * IN_MILLISECONDS;

        DoCastSpellIfCan(m_creature, SPELL_SELF_STUN, CAST_TRIGGERED);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        // Remove polymporph and DoT auras from web wrapped player
        if (m_player)
        {
            if (m_player->isAlive())
                DoCastSpellIfCan(m_player, SPELL_CLEAR_WEB_WRAP_TARGET, CAST_TRIGGERED);
        }
        m_creature->ForcedDespawn(5000);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // The nearest player is the one that summoned the NPC
        if (!m_player)
        {
            PlayerList closestPlayersList;
            GetPlayerListWithEntryInWorld(closestPlayersList, m_creature, 2.0f);
            if (!closestPlayersList.empty())
                m_player = closestPlayersList.front();
        }

        if (m_player)
        {
            // Check if the web wrapped player is still alive, if not, clear ourselves
            if (m_uiCheckVictimAliveTimer <= uiDiff)
            {
                if (!m_player->isAlive())
                    DoCastSpellIfCan(m_creature, SPELL_CLEAR_WEB_WRAP_SELF, CAST_TRIGGERED);
                m_uiCheckVictimAliveTimer = 1 * IN_MILLISECONDS;
            }
            else
                m_uiCheckVictimAliveTimer -= uiDiff;
        }
    }
};

/*###################
#   boss_maexxna
###################*/

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

    CreatureList m_summoningTriggers;
    SelectAttackingTargetParams m_webWrapParams;

    void Reset() override
    {
        m_uiWebWrapTimer            = 20 * IN_MILLISECONDS;
        m_uiWebSprayTimer           = TIMER_40_SEC;
        m_uiPoisonShockTimer        = urand(10, 20) * IN_MILLISECONDS;
        m_uiNecroticPoisonTimer     = urand(20, 30) * IN_MILLISECONDS;
        m_uiSummonSpiderlingTimer   = 30 * IN_MILLISECONDS;
        m_bEnraged                  = false;
        m_webWrapParams.range.minRange = 0;
        m_webWrapParams.range.maxRange = 100;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_MAEXXNA, IN_PROGRESS);
        else
            return;

        // Fill the list of summoning NPCs for the Web Wrap ability
        GetCreatureListWithEntryInGrid(m_summoningTriggers, m_creature, NPC_INVISIBLE_MAN, 100.0f);
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
        if (pSummoned->GetEntry() == NPC_SPIDERLING)
            pSummoned->SetInCombatWithZone();
    }

    void SummonedJustReachedHome(Creature* summoned) override
    {
        summoned->ForcedDespawn();
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Do nothing if no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Web Wrap
        // ToDo: the targets and triggers selection is probably done through spells 29280, 29281, 29282, 29283, 29285 & 29287
        // But because we are walking in Guessland until more reliable data are available, let's do the selection manually here
        if (m_uiWebWrapTimer < uiDiff)
        {
            // Randomly pick 3 targets, excluding current victim (main tank)
            std::vector<Unit*> targets;
            m_creature->SelectAttackingTargets(targets, ATTACKING_TARGET_ALL_SUITABLE, 0, nullptr, SELECT_FLAG_PLAYER | SELECT_FLAG_SKIP_TANK, m_webWrapParams);

            if (targets.size() > MAX_PLAYERS_WEB_WRAP)
            {
                std::random_shuffle(targets.begin(), targets.end());
                targets.resize(MAX_PLAYERS_WEB_WRAP);
            }

            if (!targets.empty())
            {
                // Check we have enough summoning NPCs spawned in regards of player targets
                if (m_summoningTriggers.size() < targets.size())
                {
                    script_error_log("Error in script Naxxramas::boss_maexxna: less summoning NPCs (entry %u) than expected targets (%u) for Web Wrap ability. Check your DB", NPC_INVISIBLE_MAN, MAX_PLAYERS_WEB_WRAP);
                    return;
                }

                // Randomly pick up to three trigger NPCs
                std::vector<Unit*> invisibleMen(m_summoningTriggers.begin(), m_summoningTriggers.end());
                std::random_shuffle(invisibleMen.begin(), invisibleMen.end());
                invisibleMen.resize(targets.size());

                for (uint8 i = 0; i < targets.size(); i++)
                    targets[i]->CastSpell(invisibleMen[i], SPELL_WEB_WRAP_INIT, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_creature->GetObjectGuid());

                m_uiWebWrapTimer = TIMER_40_SEC;
            }
        }
        else
            m_uiWebWrapTimer -= uiDiff;

        // Web Spray
        if (m_uiWebSprayTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_WEBSPRAY) == CAST_OK)
                m_uiWebSprayTimer = TIMER_40_SEC;
        }
        else
            m_uiWebSprayTimer -= uiDiff;

        // Poison Shock
        if (m_uiPoisonShockTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_POISONSHOCK) == CAST_OK)
                m_uiPoisonShockTimer = urand(10, 20) * IN_MILLISECONDS;
        }
        else
            m_uiPoisonShockTimer -= uiDiff;

        // Necrotic Poison
        if (m_uiNecroticPoisonTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_NECROTICPOISON) == CAST_OK)
                m_uiNecroticPoisonTimer = urand(20, 30) * IN_MILLISECONDS;
        }
        else
            m_uiNecroticPoisonTimer -= uiDiff;

        // Summon 10 Spiderlings
        if (m_uiSummonSpiderlingTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_SPIDERLING_1) == CAST_OK)
                m_uiSummonSpiderlingTimer = TIMER_40_SEC;
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

/*###################
#   npc_invible_man
###################*/

// This NPC handles the spell sync between the player that is web wrapped (with a DoT) and the related Web Wrap NPC
struct npc_invisible_manAI : public ScriptedAI
{
    npc_invisible_manAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    ObjectGuid m_victimGuid;
    uint32 m_uiDoWebWrapVisualTimer;

    void Reset() override
    {
        m_victimGuid.Clear();
        m_uiDoWebWrapVisualTimer = 0;
    }

    void MoveInLineOfSight(Unit* /*pWho*/) override {}
    void AttackStart(Unit* /*pWho*/) override {}

    // Store the GUID of the player that was pulled for later use
    void SpellHitTarget(Unit* target, const SpellEntry* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_WEB_WRAP_200:
            case SPELL_WEB_WRAP_300:
            case SPELL_WEB_WRAP_400:
            case SPELL_WEB_WRAP_500:
            {
                m_victimGuid = target->GetObjectGuid();
                m_uiDoWebWrapVisualTimer = (spell->Id == SPELL_WEB_WRAP_200 ? 5 : 6) * IN_MILLISECONDS;
                break;
            }
            default:
                break;
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiDoWebWrapVisualTimer)
        {
            if (m_uiDoWebWrapVisualTimer < uiDiff)
            {
                if (Player* pVictim = m_creature->GetMap()->GetPlayer(m_victimGuid))
                {
                    if (pVictim->isAlive())
                    {
                        // Make the player cast the visual effects spells with a delay to ensure he/she has reach his/her destination
                        pVictim->CastSpell(pVictim, SPELL_WEB_WRAP_SUMMON, TRIGGERED_OLD_TRIGGERED);
                        m_uiDoWebWrapVisualTimer = 0;
                    }
                    else
                        m_victimGuid.Clear();
                }
            }
            else
                m_uiDoWebWrapVisualTimer -= uiDiff;
        }
    }
};

UnitAI* GetAI_npc_web_wrap(Creature* pCreature)
{
    return new npc_web_wrapAI(pCreature);
}

UnitAI* GetAI_boss_maexxna(Creature* pCreature)
{
    return new boss_maexxnaAI(pCreature);
}

UnitAI* GetAI_npc_invible_man(Creature* pCreature)
{
    return new npc_invisible_manAI(pCreature);
}

void AddSC_boss_maexxna()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_maexxna";
    pNewScript->GetAI = &GetAI_boss_maexxna;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_web_wrap";
    pNewScript->GetAI = &GetAI_npc_web_wrap;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_invible_man";
    pNewScript->GetAI = &GetAI_npc_invible_man;
    pNewScript->RegisterSelf();
}
