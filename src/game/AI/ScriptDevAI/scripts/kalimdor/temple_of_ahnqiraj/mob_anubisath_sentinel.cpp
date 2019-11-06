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
SDComment:
SDCategory: Temple of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"

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

    SPELL_ENRAGE                    = 8599,

    MAX_BUDDY                       = 4
};

struct npc_anubisath_sentinelAI : public ScriptedAI
{
    npc_anubisath_sentinelAI(Creature* creature) : ScriptedAI(creature)
    {
        m_assistList.clear();
        Reset();
    }

    uint32 m_myAbility;
    bool m_isEnraged;
    std::vector<uint32> m_abilities;

    GuidList m_assistList;

    void Reset() override
    {
        m_myAbility = 0;
        m_isEnraged = false;
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

    void JustReachedHome() override
    {
        for (GuidList::const_iterator itr = m_assistList.begin(); itr != m_assistList.end(); ++itr)
        {
            if (*itr == m_creature->GetObjectGuid())
                continue;

            if (Creature* buddy = m_creature->GetMap()->GetCreature(*itr))
            {
                if (buddy->isDead())
                    buddy->Respawn();
            }
        }
    }

    void Aggro(Unit* who) override
    {
        if (!m_myAbility)
        {
            std::random_shuffle(m_abilities.begin(), m_abilities.end());    // shuffle the abilities, they will be set to the current creature and its siblings
            SetAbility();
            InitSentinelsNear(who);
        }
        
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
        for (GuidList::const_iterator itr = m_assistList.begin(); itr != m_assistList.end(); ++itr)
        {
            if (Creature* buddy = m_creature->GetMap()->GetCreature(*itr))
            {
                if (*itr == m_creature->GetObjectGuid() || !buddy->isAlive())
                    continue;

                if (!hasDoneEmote)
                {
                    DoScriptText(EMOTE_SHARE_POWERS, m_creature);
                    hasDoneEmote = true;
                }
                m_creature->CastSpell(buddy, SPELL_TRANSFER_POWER, TRIGGERED_OLD_TRIGGERED);
                DoCastSpellIfCan(buddy, m_myAbility, CAST_TRIGGERED);
            }
        }
    }

    void InitSentinelsNear(Unit* target)
    {
        if (!m_assistList.empty())
        {
            int8 buddyCount = 1;
            for (GuidList::const_iterator itr = m_assistList.begin(); itr != m_assistList.end(); ++itr)
            {
                if (*itr == m_creature->GetObjectGuid())
                    continue;

                if (Creature* buddy = m_creature->GetMap()->GetCreature(*itr))
                {
                    if (buddy->isAlive())
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

    void UpdateAI(const uint32 /*uiDiff*/) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (!m_isEnraged && m_creature->GetHealthPercent() < 30.0f)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
            {
                DoScriptText(EMOTE_GENERIC_FRENZY, m_creature);
                m_isEnraged = true;
            }
        }

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_anubisath_sentinel(Creature* creature)
{
    return new npc_anubisath_sentinelAI(creature);
}

void AddSC_mob_anubisath_sentinel()
{
    Script* newScript = new Script;
    newScript->Name = "mob_anubisath_sentinel";
    newScript->GetAI = &GetAI_npc_anubisath_sentinel;
    newScript->RegisterSelf();
}
