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
SDName: mob_anubisath_sentinel
SD%Complete: 100
SDComment: mob_anubisath_sentinel, mob_anubisath_defender
SDCategory: Temple of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/base/CombatAI.h"
#include "AI/ScriptDevAI/include/sc_common.h"

/*######
## npc_anubisath_sentinel
######*/

enum
{
    EMOTE_GENERIC_FRENZY            = -1000002,
    EMOTE_SHARE_POWERS              = -1531047,

    SPELL_PERIODIC_MANA_BURN        = 812,
    SPELL_MENDING                   = 2147,
    SPELL_PERIODIC_SHADOW_STORM     = 2148,
    SPELL_PERIODIC_THUNDERCLAP      = 2834,
    SPELL_MORTAL_STRIKE             = 9347,
    SPELL_FIRE_ARCANE_REFLECT       = 13022,
    SPELL_SHADOW_FROST_REFLECT      = 19595,
    SPELL_PERIODIC_KNOCK_AWAY       = 21737,
    SPELL_THORNS                    = 25777,
    SPELL_TRANSFER_POWER            = 2400,
    SPELL_HEAL_BRETHEN              = 26565,

    SPELL_ENRAGE                    = 8599,

    MAX_BUDDY                       = 4
};

enum AnubSentinelActions
{
    ANUBSENTINEL_BERSERK_HP_CHECK,
    ANUBSENTINEL_ACTION_MAX,
};

struct npc_anubisath_sentinelAI : public CombatAI
{
    npc_anubisath_sentinelAI(Creature* creature) : CombatAI(creature, ANUBSENTINEL_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddTimerlessCombatAction(ANUBSENTINEL_BERSERK_HP_CHECK, true);         // Soft enrage at 30% HP
        m_assistList.clear();
    }

    ScriptedInstance* m_instance;

    int32 m_myAbility;
    std::vector<uint32> m_abilities;

    GuidList m_assistList;

    void Reset() override
    {
        CombatAI::Reset();
        m_myAbility = 0;
        m_abilities = { SPELL_PERIODIC_MANA_BURN, SPELL_MENDING, SPELL_PERIODIC_SHADOW_STORM, SPELL_PERIODIC_THUNDERCLAP, SPELL_MORTAL_STRIKE, SPELL_FIRE_ARCANE_REFLECT, SPELL_SHADOW_FROST_REFLECT, SPELL_PERIODIC_KNOCK_AWAY, SPELL_THORNS };
    }

    void GetAIInformation(ChatHandler& reader) override
    {
        if (m_assistList.empty())
            reader.PSendSysMessage("Anubisath Sentinel - group not assigned, will be assigned OnAggro");
        if (m_assistList.size() == MAX_BUDDY)
            reader.PSendSysMessage("Anubisath Sentinel - proper group found, own ability is %u", m_myAbility);
        else
            reader.PSendSysMessage("Anubisath Sentinel - not correct number of mobs for group found. Number found %u, should be %u", uint32(m_assistList.size()), MAX_BUDDY);
    }

    void EnterEvadeMode() override
    {
        CombatAI::EnterEvadeMode();

        for (auto guid : m_assistList)
        {
            // Skip self as we are obviously alive and in world
            if (guid == m_creature->GetObjectGuid())
                continue;

            // Respawn fallen Anubisath Sentinel
            if (Creature* buddy = m_creature->GetMap()->GetCreature(guid))
            {
                if (buddy->IsDead())
                    buddy->Respawn();
            }
        }
    }

    void Aggro(Unit* who) override
    {
        // Find all buddies
        if (m_assistList.empty())
        {
            CreatureList assistList;
            GetCreatureListWithEntryInGrid(assistList, m_creature, m_creature->GetEntry(), 80.0f);

            for (auto& iter : assistList)
                m_assistList.push_back(iter->GetObjectGuid());

            if (m_assistList.size() != MAX_BUDDY)
                script_error_log("npc_anubisath_sentinel for %s found too few/too many buddies, expected %u.", m_creature->GetGuidStr().c_str(), MAX_BUDDY);
        }

        if (!m_myAbility)
        {
            std::shuffle(m_abilities.begin(), m_abilities.end(), *GetRandomGenerator()); // shuffle the abilities, they will be set to the current creature and its siblings
            SetAbility();
            InitSentinelsNear(who);
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoTransferAbility();
    }

    void SetAbility(bool forced=false, uint32 abilityId=0)
    {
        if (m_myAbility) // Do not set ability if already force set by the first Anubisath Sentinel to get aggro
            return;

        // This Anubisath Sentinel is the first one to set his ability as it is the one that got initial aggro
        // Pick the first ability in the (randomised) list of abilities
        if (!forced)
            m_myAbility = m_abilities[0];
        // This Anubisath Sentinel is forced to set his ability by the first sentinel to get aggro
        // Check that the requested ability is valid
        else if (std::find(m_abilities.begin(), m_abilities.end(), abilityId) != m_abilities.end())
            m_myAbility = abilityId;
        else
        {
            script_error_log("npc_anubisath_sentinel for %s: request illegal spell ID as ability %u.", m_creature->GetGuidStr().c_str(), abilityId);
            return;
        }

        DoCastSpellIfCan(m_creature, m_myAbility, CAST_TRIGGERED);
    }

    void DoTransferAbility()
    {
        bool hasDoneEmote = false;
        for (auto guid : m_assistList)
        {
            if (Creature* buddy = m_creature->GetMap()->GetCreature(guid))
            {
                // Don't transfer ability to dead buddies
                if (guid == m_creature->GetObjectGuid() || !buddy->IsAlive())
                    continue;

                // Only emote on the first ability transfert
                if (!hasDoneEmote)
                {
                    DoScriptText(EMOTE_SHARE_POWERS, m_creature);
                    hasDoneEmote = true;
                }
                m_creature->CastCustomSpell(buddy, SPELL_TRANSFER_POWER, &m_myAbility, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr);
            }
        }
    }

    void InitSentinelsNear(Unit* target)
    {
        if (!m_assistList.empty())
        {
            int8 buddyCount = 1;
            for (auto guid : m_assistList)
            {
                // Don't init ourselves
                if (guid == m_creature->GetObjectGuid())
                    continue;

                if (Creature* buddy = m_creature->GetMap()->GetCreature(guid))
                {
                    if (buddy->IsAlive())
                    {
                        npc_anubisath_sentinelAI* buddyAI = static_cast<npc_anubisath_sentinelAI*>(buddy->AI());
                        buddyAI->SetAbility(true, m_abilities[buddyCount]);
                        ++buddyCount;
                        buddy->AI()->AttackStart(target);
                    }
                }
            }
        }
    }

    void ExecuteAction(uint32 action) override
    {
        if (action == ANUBSENTINEL_BERSERK_HP_CHECK)
        {
            if (m_creature->GetHealthPercent() < 30.0f)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
                {
                    DoScriptText(EMOTE_GENERIC_FRENZY, m_creature);
                    DisableCombatAction(action);
                }
            }
        }
    }
};

struct SharePowers : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            if (Unit* caster = spell->GetCaster())
            {
                uint32 spellId = spell->m_currentBasePoints[EFFECT_INDEX_0];
                if (Unit* unitTarget = spell->GetUnitTarget())
                {
                    caster->CastSpell(unitTarget, spellId, TRIGGERED_OLD_TRIGGERED);              // Cast own ability on target
                    caster->CastSpell(unitTarget, SPELL_HEAL_BRETHEN, TRIGGERED_OLD_TRIGGERED);   // Heal Brethren
                }
            }
        }
    }
};

/*######
## npc_anubisath_defender
######*/

enum
{
    SPELL_SHADOW_STORM      = 26555,
    SPELL_METEOR            = 26558,
    SPELL_ENRAGE_DEFENDER   = 8269,
    SPELL_EXPLODE           = 25698,
    // SPELL_SHADOW_FROST_REFLECT      = 19595, // Same as Anubisath Sentinel
    // SPELL_FIRE_ARCANE_REFLECT       = 13022, // Same as Anubisath Sentinel
    SPELL_THUNDERCLAP       = 26554,
    SPELL_PLAGUE            = 26556,
    SPELL_SUMMON_WARRIOR    = 17431,
    SPELL_SUMMON_SWARMGUARD = 17430

};

enum AnubDefenderActions
{
    ANUBDEFENDER_HP_CHECK,
    ANUBDEFENDER_METEOR,
    ANUBDEFENDER_PLAGUE,
    ANUBDEFENDER_THUNDERCLAP,
    ANUBDEFENDER_SHADOW_STORM,
    ANUBDEFENDER_SUMMON,
    ANUBDEFENDER_ACTION_MAX,
};

struct npc_anubisath_defenderAI : public CombatAI
{
    npc_anubisath_defenderAI(Creature* creature) : CombatAI(creature, ANUBDEFENDER_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddTimerlessCombatAction(ANUBDEFENDER_HP_CHECK, true);         // Enrage or explode at 10% HP
        AddCombatAction(ANUBDEFENDER_METEOR, true);
        AddCombatAction(ANUBDEFENDER_PLAGUE, true);
        AddCombatAction(ANUBDEFENDER_THUNDERCLAP, true);
        AddCombatAction(ANUBDEFENDER_SHADOW_STORM, true);
        AddCombatAction(ANUBDEFENDER_SUMMON, 3 * IN_MILLISECONDS, 5 * IN_MILLISECONDS);
    }

    ScriptedInstance* m_instance;

    void Aggro(Unit* /*who*/) override
    {
        // At each aggro Anubisath Defender randomises its four abilities
        m_creature->RemoveAllAuras();
        // Pick between the two reflect spells
        uint32 reflectSpellId = (urand(0, 1) ? SPELL_FIRE_ARCANE_REFLECT : SPELL_SHADOW_FROST_REFLECT);
        DoCastSpellIfCan(m_creature, reflectSpellId, CAST_TRIGGERED);
        // Pick between Meteor and Plague
        if (urand(0, 1))
            ResetCombatAction(ANUBDEFENDER_METEOR, urand(6, 10) * IN_MILLISECONDS);
        else
            ResetCombatAction(ANUBDEFENDER_PLAGUE, urand(6, 10) * IN_MILLISECONDS);
        // Pick between Shadow Storm and Thunderclap
        if (urand(0, 1))
            ResetCombatAction(ANUBDEFENDER_THUNDERCLAP, urand(5, 8) * IN_MILLISECONDS);
        else
            ResetCombatAction(ANUBDEFENDER_SHADOW_STORM, urand(5, 8) * IN_MILLISECONDS);
        // The ability at 10% is randomised when the event triggers
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case ANUBDEFENDER_HP_CHECK:
            {
                if (m_creature->GetHealthPercent() < 10.0f)
                {
                    if (urand(0, 1))
                    {
                        if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE_DEFENDER) == CAST_OK)
                        {
                            DoScriptText(EMOTE_GENERIC_FRENZY, m_creature);
                            DisableCombatAction(action);
                        }
                    }
                    else
                    {
                        if (DoCastSpellIfCan(m_creature, SPELL_EXPLODE) == CAST_OK)
                            DisableCombatAction(action);
                    }
                }
                break;
            }
            case ANUBDEFENDER_METEOR:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_PLAYER ))
                {
                    if (DoCastSpellIfCan(target, SPELL_METEOR, TRIGGERED_OLD_TRIGGERED) == CAST_OK)
                        ResetCombatAction(action, urand(8, 12) * IN_MILLISECONDS);
                }
                break;
            }
            case ANUBDEFENDER_PLAGUE:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_SKIP_TANK))
                {
                    if (DoCastSpellIfCan(target, SPELL_PLAGUE, TRIGGERED_OLD_TRIGGERED) == CAST_OK)
                        ResetCombatAction(action, urand(8, 12) * IN_MILLISECONDS);
                }
                break;
            }
            case ANUBDEFENDER_SHADOW_STORM:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_SHADOW_STORM, TRIGGERED_OLD_TRIGGERED) == CAST_OK)
                    ResetCombatAction(action, urand(6, 10) * IN_MILLISECONDS);
                break;
            }
            case ANUBDEFENDER_THUNDERCLAP:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_THUNDERCLAP, TRIGGERED_OLD_TRIGGERED) == CAST_OK)
                    ResetCombatAction(action, urand(6, 10) * IN_MILLISECONDS);
                break;
            }
            case ANUBDEFENDER_SUMMON:
            {
                if (urand(0, 1))
                {
                    if (DoCastSpellIfCan(nullptr, SPELL_SUMMON_SWARMGUARD, TRIGGERED_OLD_TRIGGERED) == CAST_OK)
                        ResetCombatAction(action, urand(12, 16) * IN_MILLISECONDS);
                }
                else
                {
                    if (DoCastSpellIfCan(nullptr, SPELL_SUMMON_WARRIOR, TRIGGERED_OLD_TRIGGERED) == CAST_OK)
                        ResetCombatAction(action, urand(12, 16) * IN_MILLISECONDS);
                }
                break;
            }
            default:
                break;
        }
    }
};

void AddSC_mob_anubisath_sentinel()
{
    Script* newScript = new Script;
    newScript->Name = "mob_anubisath_sentinel";
    newScript->GetAI = &GetNewAIInstance<npc_anubisath_sentinelAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "mob_anubisath_defender";
    newScript->GetAI = &GetNewAIInstance<npc_anubisath_defenderAI>;
    newScript->RegisterSelf();

    RegisterSpellScript<SharePowers>("spell_anubisath_share_powers");
}
