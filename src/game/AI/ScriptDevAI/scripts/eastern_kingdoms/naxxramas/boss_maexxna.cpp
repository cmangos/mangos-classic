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

#include "AI/ScriptDevAI/base/CombatAI.h"
#include "AI/ScriptDevAI/base/TimerAI.h"
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
    SPELL_WEBWRAP_STUN          = 28622,                    // Triggered by spells 28618 - 28621
    SPELL_WEB_WRAP_SUMMON       = 28627,
    SPELL_CLEAR_WEB_WRAP_TARGET = 28628,
    SPELL_CLEAR_WEB_WRAP_SELF   = 28629,
    SPELL_SELF_STUN             = 29826,                    // Spell Id unsure
    SPELL_KILL_WEBWRAP          = 29108,

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

enum MaexxnaActions
{
    MAEXXNA_WEBWRAP,
    MAEXXNA_WEBSPRAY,
    MAEXXNA_POISON_SHOCK,
    MAEXXNA_NECROTIC_POISON,
    MAEXXNA_SUMMON_SPIDERLING,
    MAEXXNA_ENRAGE_HP_CHECK,
    MAEXXNA_ACTION_MAX,
};

struct boss_maexxnaAI : public CombatAI
{
    boss_maexxnaAI(Creature* creature) : CombatAI(creature, MAEXXNA_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddTimerlessCombatAction(MAEXXNA_ENRAGE_HP_CHECK, true);         // Soft enrage à 30%
        AddCombatAction(MAEXXNA_WEBWRAP, 20000u);
        AddCombatAction(MAEXXNA_WEBSPRAY, 40000u);
        AddCombatAction(MAEXXNA_POISON_SHOCK, 10000u, 20000u);
        AddCombatAction(MAEXXNA_NECROTIC_POISON, 20000u, 30000u);
        AddCombatAction(MAEXXNA_SUMMON_SPIDERLING, 30000u);
    }

    ScriptedInstance* m_instance;

    CreatureList m_summoningTriggers;
    SelectAttackingTargetParams m_webWrapParams;

    void Reset() override
    {
        CombatAI::Reset();

        m_webWrapParams.range.minRange = 0;
        m_webWrapParams.range.maxRange = 100;

        DoCastSpellIfCan(m_creature, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
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

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case MAEXXNA_ENRAGE_HP_CHECK:
            {
                if (m_creature->GetHealthPercent() < 30.0f)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_FRENZY) == CAST_OK)
                    {
                        DoScriptText(EMOTE_BOSS_GENERIC_FRENZY, m_creature);
                        DisableCombatAction(action);
                    }
                }
                break;
            }
            case MAEXXNA_WEBWRAP:
            {
                // Web Wrap
                // ToDo: the targets and triggers selection is probably done through spells 29280, 29281, 29282, 29283, 29285 & 29287
                // But because we are walking in Guessland until more reliable data are available, let's do the selection manually here
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
                        break;
                    }

                    // Randomly pick up to three trigger NPCs
                    std::vector<Unit*> invisibleMen(m_summoningTriggers.begin(), m_summoningTriggers.end());
                    std::shuffle(invisibleMen.begin(), invisibleMen.end(), *GetRandomGenerator());
                    invisibleMen.resize(targets.size());

                    for (uint8 i = 0; i < targets.size(); i++)
                        targets[i]->CastSpell(invisibleMen[i], SPELL_WEB_WRAP_INIT, TRIGGERED_IGNORE_UNSELECTABLE_FLAG, nullptr, nullptr, m_creature->GetObjectGuid());

                    ResetCombatAction(action, TIMER_40_SEC);
                    break;
                }
            }
            case MAEXXNA_WEBSPRAY:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_WEBSPRAY) == CAST_OK)
                    ResetCombatAction(action, TIMER_40_SEC);
                break;
            }
            case MAEXXNA_POISON_SHOCK:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_POISONSHOCK) == CAST_OK)
                    ResetCombatAction(action, urand(10, 20) * IN_MILLISECONDS);
                break;
            }
            case MAEXXNA_NECROTIC_POISON:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_NECROTICPOISON) == CAST_OK)
                    ResetCombatAction(action, urand(20, 30) * IN_MILLISECONDS);
                break;
            }
            // Summon 10 Spiderlings
            case MAEXXNA_SUMMON_SPIDERLING:
            {
                if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_SPIDERLING_1) == CAST_OK)
                    ResetCombatAction(action, TIMER_40_SEC);
                break;
            }
            default:
                break;
        }
    }
};

/*###################
#   npc_invible_man
###################*/

// This NPC handles the spell sync between the player that is web wrapped (with a DoT) and the related Web Wrap NPC
struct npc_invisible_manAI : public ScriptedAI
{
    npc_invisible_manAI(Creature* creature) : ScriptedAI(creature) {
        AddCustomAction(1, true, [&]() { HandleVictimCheck(); });
        SetReactState(REACT_PASSIVE);
        Reset();
    }

    ObjectGuid m_victimGuid;

    void Reset() override
    {
        m_victimGuid.Clear();
    }

    void HandleVictimCheck() {
            if (Player* victim = m_creature->GetMap()->GetPlayer(m_victimGuid))
            {
                if (victim->IsAlive())
                {
                    // Make the player cast the visual effects spells with a delay to ensure he/she has reach his/her destination
                    victim->CastSpell(victim, SPELL_WEB_WRAP_SUMMON, TRIGGERED_OLD_TRIGGERED);
                    DisableTimer(1);
                }
                else
                    m_victimGuid.Clear();
            }
    };

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
                ResetTimer(1, (spell->Id == SPELL_WEB_WRAP_200 ? 5 : 6) * IN_MILLISECONDS);
                break;
            }
            default:
                break;
        }
    }
};

// Web Wrap (Maexxna: pull spell initialiser)
struct WebWrap : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            if (!spell->GetAffectiveCasterObject())
                return;

            Unit* unitTarget = spell->GetUnitTarget();
            if (!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT || spell->GetCaster()->GetTypeId() != TYPEID_PLAYER)
                return;

            float dist = spell->GetCaster()->GetDistance(unitTarget, false);
            // Switch the pull target spell based on the distance from the web wrap position
            uint32 pullSpellId = SPELL_WEB_WRAP_500;
            if (dist < 25.0f)
                pullSpellId = SPELL_WEB_WRAP_200;
            else if (dist < 50.0f)
                pullSpellId = SPELL_WEB_WRAP_300;
            else if (dist < 75.0f)
                pullSpellId = SPELL_WEB_WRAP_400;

            unitTarget->CastSpell(spell->GetCaster(), pullSpellId, TRIGGERED_INSTANT_CAST, nullptr, nullptr, spell->GetAffectiveCasterObject()->GetObjectGuid());
        }
    }
};

// Clear Web Wrap
struct ClearWebWrap : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            Unit* unitTarget = spell->GetUnitTarget();
            switch(spell->m_spellInfo->Id)
            {
                case SPELL_CLEAR_WEB_WRAP_TARGET:   // Clear Web Wrap (Maexxna: clear effects on player)
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_PLAYER)
                    {
                        unitTarget->RemoveAurasDueToSpell(SPELL_WEB_WRAP_SUMMON);   // Web Wrap polymorph
                        unitTarget->RemoveAurasDueToSpell(SPELL_WEBWRAP_STUN);      // Web Wrap stun and DoT
                    }
                    break;
                }
                case SPELL_CLEAR_WEB_WRAP_SELF:     // Clear Web Wrap (Maexxna: kill Web Wrap NPC)
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT)
                        unitTarget->CastSpell(nullptr, SPELL_KILL_WEBWRAP, TRIGGERED_OLD_TRIGGERED);  // Kill Web Wrap
                    break;
                }
                default:
                    break;
            }
        }
    }
};

void AddSC_boss_maexxna()
{
    Script* newScript = new Script;
    newScript->Name = "boss_maexxna";
    newScript->GetAI = &GetNewAIInstance<boss_maexxnaAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "npc_web_wrap";
    newScript->GetAI = &GetNewAIInstance<npc_web_wrapAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "npc_invible_man";
    newScript->GetAI = &GetNewAIInstance<npc_invisible_manAI>;
    newScript->RegisterSelf();

    RegisterSpellScript<WebWrap>("spell_web_wrap");
    RegisterSpellScript<ClearWebWrap>("spell_clear_web_wrap");
}
