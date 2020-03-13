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

#include "AI/ScriptDevAI/include/sc_common.h"
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
    npc_web_wrapAI(Creature* creature) : ScriptedAI(creature) { Reset(); }

    Player* m_player;
    uint32 m_checkVictimAliveTimer;

    void Reset() override
    {
        SetCombatMovement(false);
        m_player = nullptr;
        m_checkVictimAliveTimer = 1 * IN_MILLISECONDS;

        DoCastSpellIfCan(m_creature, SPELL_SELF_STUN, CAST_TRIGGERED);
    }

    void JustDied(Unit* /*killer*/) override
    {
        // Remove polymporph and DoT auras from web wrapped player
        if (m_player)
        {
            if (m_player->IsAlive())
                DoCastSpellIfCan(m_player, SPELL_CLEAR_WEB_WRAP_TARGET, CAST_TRIGGERED);
        }
        m_creature->ForcedDespawn(5000);
    }

    void UpdateAI(const uint32 diff) override
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
            if (m_checkVictimAliveTimer <= diff)
            {
                if (!m_player->IsAlive())
                    DoCastSpellIfCan(m_creature, SPELL_CLEAR_WEB_WRAP_SELF, CAST_TRIGGERED);
                m_checkVictimAliveTimer = 1 * IN_MILLISECONDS;
            }
            else
                m_checkVictimAliveTimer -= diff;
        }
    }
};

/*###################
#   boss_maexxna
###################*/

struct boss_maexxnaAI : public ScriptedAI
{
    boss_maexxnaAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (instance_naxxramas*)creature->GetInstanceData();
        Reset();

        DoCastSpellIfCan(m_creature, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    instance_naxxramas* m_instance;

    uint32 m_webWrapTimer;
    uint32 m_webSprayTimer;
    uint32 m_poisonShockTimer;
    uint32 m_necroticPoisonTimer;
    uint32 m_summonSpiderlingTimer;
    bool   m_isEnraged;

    CreatureList m_summoningTriggers;
    SelectAttackingTargetParams m_webWrapParams;

    void Reset() override
    {
        m_webWrapTimer            = 20 * IN_MILLISECONDS;
        m_webSprayTimer           = TIMER_40_SEC;
        m_poisonShockTimer        = urand(10, 20) * IN_MILLISECONDS;
        m_necroticPoisonTimer     = urand(20, 30) * IN_MILLISECONDS;
        m_summonSpiderlingTimer   = 30 * IN_MILLISECONDS;
        m_isEnraged                  = false;
        m_webWrapParams.range.minRange = 0;
        m_webWrapParams.range.maxRange = 100;
    }

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_MAEXXNA, IN_PROGRESS);
        else
            return;

        // Fill the list of summoning NPCs for the Web Wrap ability
        GetCreatureListWithEntryInGrid(m_summoningTriggers, m_creature, NPC_INVISIBLE_MAN, 100.0f);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_MAEXXNA, DONE);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_MAEXXNA, FAIL);

        DoCastSpellIfCan(m_creature, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_SPIDERLING)
            summoned->SetInCombatWithZone();
    }

    void SummonedJustReachedHome(Creature* summoned) override
    {
        summoned->ForcedDespawn();
    }

    void UpdateAI(const uint32 diff) override
    {
        // Do nothing if no target
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        // Web Wrap
        // ToDo: the targets and triggers selection is probably done through spells 29280, 29281, 29282, 29283, 29285 & 29287
        // But because we are walking in Guessland until more reliable data are available, let's do the selection manually here
        if (m_webWrapTimer < diff)
        {
            // Randomly pick 3 targets, excluding current victim (main tank)
            std::vector<Unit*> targets;
            m_creature->SelectAttackingTargets(targets, ATTACKING_TARGET_ALL_SUITABLE, 0, nullptr, SELECT_FLAG_PLAYER | SELECT_FLAG_SKIP_TANK, m_webWrapParams);

            if (targets.size() > MAX_PLAYERS_WEB_WRAP)
            {
                std::shuffle(targets.begin(), targets.end(), *GetRandomGenerator());
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
                std::shuffle(invisibleMen.begin(), invisibleMen.end(), *GetRandomGenerator());
                invisibleMen.resize(targets.size());

                for (uint8 i = 0; i < targets.size(); i++)
                    targets[i]->CastSpell(invisibleMen[i], SPELL_WEB_WRAP_INIT, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_creature->GetObjectGuid());

                m_webWrapTimer = TIMER_40_SEC;
            }
        }
        else
            m_webWrapTimer -= diff;

        // Web Spray
        if (m_webSprayTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_WEBSPRAY) == CAST_OK)
                m_webSprayTimer = TIMER_40_SEC;
        }
        else
            m_webSprayTimer -= diff;

        // Poison Shock
        if (m_poisonShockTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_POISONSHOCK) == CAST_OK)
                m_poisonShockTimer = urand(10, 20) * IN_MILLISECONDS;
        }
        else
            m_poisonShockTimer -= diff;

        // Necrotic Poison
        if (m_necroticPoisonTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_NECROTICPOISON) == CAST_OK)
                m_necroticPoisonTimer = urand(20, 30) * IN_MILLISECONDS;
        }
        else
            m_necroticPoisonTimer -= diff;

        // Summon 10 Spiderlings
        if (m_summonSpiderlingTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_SPIDERLING_1) == CAST_OK)
                m_summonSpiderlingTimer = TIMER_40_SEC;
        }
        else
            m_summonSpiderlingTimer -= diff;

        // Enrage if not already enraged and below 30%
        if (!m_isEnraged && m_creature->GetHealthPercent() < 30.0f)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FRENZY) == CAST_OK)
            {
                DoScriptText(EMOTE_BOSS_GENERIC_FRENZY, m_creature);
                m_isEnraged = true;
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
    npc_invisible_manAI(Creature* creature) : ScriptedAI(creature) { Reset(); }

    ObjectGuid m_victimGuid;
    uint32 m_doWebWrapVisualTimer;

    void Reset() override
    {
        m_victimGuid.Clear();
        m_doWebWrapVisualTimer = 0;
    }

    void MoveInLineOfSight(Unit* /*who*/) override {}
    void AttackStart(Unit* /*who*/) override {}

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
                m_doWebWrapVisualTimer = (spell->Id == SPELL_WEB_WRAP_200 ? 5 : 6) * IN_MILLISECONDS;
                break;
            }
            default:
                break;
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        if (m_doWebWrapVisualTimer)
        {
            if (m_doWebWrapVisualTimer < diff)
            {
                if (Player* victim = m_creature->GetMap()->GetPlayer(m_victimGuid))
                {
                    if (victim->IsAlive())
                    {
                        // Make the player cast the visual effects spells with a delay to ensure he/she has reach his/her destination
                        victim->CastSpell(victim, SPELL_WEB_WRAP_SUMMON, TRIGGERED_OLD_TRIGGERED);
                        m_doWebWrapVisualTimer = 0;
                    }
                    else
                        m_victimGuid.Clear();
                }
            }
            else
                m_doWebWrapVisualTimer -= diff;
        }
    }
};

UnitAI* GetAI_npc_web_wrap(Creature* creature)
{
    return new npc_web_wrapAI(creature);
}

UnitAI* GetAI_boss_maexxna(Creature* creature)
{
    return new boss_maexxnaAI(creature);
}

UnitAI* GetAI_npc_invible_man(Creature* creature)
{
    return new npc_invisible_manAI(creature);
}

void AddSC_boss_maexxna()
{
    Script* newScript = new Script;
    newScript->Name = "boss_maexxna";
    newScript->GetAI = &GetAI_boss_maexxna;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "npc_web_wrap";
    newScript->GetAI = &GetAI_npc_web_wrap;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "npc_invible_man";
    newScript->GetAI = &GetAI_npc_invible_man;
    newScript->RegisterSelf();
}
