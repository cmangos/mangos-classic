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
#include "AI/ScriptDevAI/base/BossAI.h"
#include "naxxramas.h"

enum
{
    EMOTE_BOSS_GENERIC_FRENZY   = 1191,

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

    uint32 m_checkVictimAliveTimer;

    void Reset() override
    {
        SetCombatMovement(false);
        m_checkVictimAliveTimer = 1 * IN_MILLISECONDS;

        DoCastSpellIfCan(m_creature, SPELL_SELF_STUN, CAST_TRIGGERED);
    }

    void JustDied(Unit* /*killer*/) override
    {
        // Remove polymporph and DoT auras from web wrapped player
        if (Unit* spawner = m_creature->GetSpawner())
        {
            if (spawner->IsAlive())
                DoCastSpellIfCan(spawner, SPELL_CLEAR_WEB_WRAP_TARGET, CAST_TRIGGERED);
        }
        m_creature->ForcedDespawn(5000);
    }

    void UpdateAI(const uint32 diff) override
    {
        if (Unit* spawner = m_creature->GetSpawner())
        {
            // Check if the web wrapped player is still alive, if not, clear ourselves
            if (m_checkVictimAliveTimer <= diff)
            {
                if (!spawner->IsAlive())
                    DoCastSpellIfCan(nullptr, SPELL_CLEAR_WEB_WRAP_SELF, CAST_TRIGGERED);
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
    MAEXXNA_ACTION_MAX,
};

struct boss_maexxnaAI : public BossAI
{
    boss_maexxnaAI(Creature* creature) : BossAI(creature, MAEXXNA_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        SetDataType(TYPE_MAEXXNA);
    }

    ScriptedInstance* m_instance;

    CreatureList m_summoningTriggers;

    void Reset() override
    {
        BossAI::Reset();
        DoCastSpellIfCan(nullptr, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
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
};

/*###################
#   npc_invible_man
###################*/

// This NPC handles the spell sync between the player that is web wrapped (with a DoT) and the related Web Wrap NPC
struct npc_invisible_manAI : public ScriptedAI
{
    npc_invisible_manAI(Creature* creature) : ScriptedAI(creature)
    {
        AddCustomAction(1, true, [&]() { HandleVictimCheck(); });
        SetReactState(REACT_PASSIVE);
        Reset();
    }

    ObjectGuid m_victimGuid;

    void Reset() override
    {
        m_victimGuid.Clear();
    }

    void HandleVictimCheck()
    {
        if (Player* victim = m_creature->GetMap()->GetPlayer(m_victimGuid))
        {
            if (victim->IsAlive())
            {
                // Make the player cast the visual effects spells with a delay to ensure he/she has reach his/her destination
                victim->CastSpell(nullptr, SPELL_WEB_WRAP_SUMMON, TRIGGERED_OLD_TRIGGERED);
                DisableTimer(1);
            }
            else
                m_victimGuid.Clear();
        }
    };

    // Store the GUID of the player that was pulled for later use
    void SpellHitTarget(Unit* target, const SpellEntry* spellInfo) override
    {
        switch (spellInfo->Id)
        {
            case SPELL_WEB_WRAP_200:
            case SPELL_WEB_WRAP_300:
            case SPELL_WEB_WRAP_400:
            case SPELL_WEB_WRAP_500:
            {
                m_victimGuid = target->GetObjectGuid();
                ResetTimer(1, (spellInfo->Id == SPELL_WEB_WRAP_200 ? 5 : 6) * IN_MILLISECONDS);
                break;
            }
            default:
                break;
        }
    }
};

// 28673 - Web Wrap
struct WebWrapMaexxna : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        uint32 targetCount = MAX_PLAYERS_WEB_WRAP;
        std::vector<uint32> targetSpells{ 29280, 29281, 29282, 29283, 29285, 29287 };
        std::vector<Unit*> unitList;
        spell->GetCaster()->SelectAttackingTargets(unitList, ATTACKING_TARGET_ALL_SUITABLE, 1, nullptr, SELECT_FLAG_PLAYER | SELECT_FLAG_SKIP_TANK);
        std::shuffle(unitList.begin(), unitList.end(), *GetRandomGenerator());
        unitList.resize(targetCount);
        for (Unit* target : unitList)
        {
            uint32 spellId = targetSpells[urand(0, targetSpells.size() - 1)];
            targetSpells.erase(std::remove(targetSpells.begin(), targetSpells.end(), spellId), targetSpells.end());
            target->CastSpell(nullptr, spellId, TRIGGERED_OLD_TRIGGERED);
        }
    }
};

// 29280, 29281, 29282, 29283, 29285, 29287 - Web Wrap
struct WebWrapMessage : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        float dist = spell->GetCaster()->GetDistance(spell->GetUnitTarget(), true, DIST_CALC_NONE);
        uint32 spellId = 0;
        if (dist <= 20.f)
            spellId = SPELL_WEB_WRAP_200;
        else if (dist <= 30.f)
            spellId = SPELL_WEB_WRAP_300;
        else if (dist <= 40.f)
            spellId = SPELL_WEB_WRAP_400;
        else
            spellId = SPELL_WEB_WRAP_500;

        if (spellId)
            spell->GetUnitTarget()->CastSpell(spell->GetCaster(), spellId, TRIGGERED_OLD_TRIGGERED);
    }
};

// 28628, 28629 - Clear Web Wrap 
struct ClearWebWrap : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        // Clear Web Wrap
        if (effIdx == EFFECT_INDEX_0)
        {
            Unit* unitTarget = spell->GetUnitTarget();
            switch (spell->m_spellInfo->Id)
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

    RegisterSpellScript<WebWrapMaexxna>("spell_web_wrap_maexxna");
    RegisterSpellScript<WebWrapMessage>("spell_web_wrap_message");
    RegisterSpellScript<ClearWebWrap>("spell_clear_web_wrap");
}
