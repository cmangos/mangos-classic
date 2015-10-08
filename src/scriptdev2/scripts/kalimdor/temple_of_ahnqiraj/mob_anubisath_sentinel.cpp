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
SD%Complete: 75
SDComment: Abilities selection needs further improvements. Shadow storm is not properly implemented in core it should only target ppl outside of melee range.
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"

enum
{
    EMOTE_GENERIC_FRENZY            = -1000002,

    SPELL_PERIODIC_MANA_BURN        = 812,
    SPELL_MENDING                   = 2147,
    SPELL_PERIODIC_SHADOW_STORM     = 2148,
    SPELL_PERIODIC_THUNDERCLAP      = 2834,
    SPELL_MORTAL_STRIKE             = 9347,
    SPELL_FIRE_ARCANE_REFLECT       = 13022,
    SPELL_SHADOW_FROST_REFLECT      = 19595,
    SPELL_PERIODIC_KNOCK_AWAY       = 21737,
    SPELL_THORNS                    = 25777,

    SPELL_ENRAGE                    = 8599,

    MAX_BUDDY                       = 4
};

struct npc_anubisath_sentinelAI : public ScriptedAI
{
    npc_anubisath_sentinelAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_lAssistList.clear();
        Reset();
    }

    uint32 m_uiMyAbility;
    bool m_bEnraged;

    GuidList m_lAssistList;

    void Reset() override
    {
        m_uiMyAbility = 0;
        m_bEnraged = false;
    }

    void GetAIInformation(ChatHandler& reader) override
    {
        if (m_lAssistList.empty())
            reader.PSendSysMessage("Anubisath Sentinel - group not assigned, will be assigned OnAggro");
        if (m_lAssistList.size() == MAX_BUDDY)
            reader.PSendSysMessage("Anubisath Sentinel - proper group found");
        else
            reader.PSendSysMessage("Anubisath Sentinel - not correct number of mobs for group found. Number found %u, should be %u", m_lAssistList.size(), MAX_BUDDY);
    }

    void JustReachedHome() override
    {
        for (GuidList::const_iterator itr = m_lAssistList.begin(); itr != m_lAssistList.end(); ++itr)
        {
            if (*itr == m_creature->GetObjectGuid())
                continue;

            if (Creature* pBuddy = m_creature->GetMap()->GetCreature(*itr))
            {
                if (pBuddy->isDead())
                    pBuddy->Respawn();
            }
        }
    }

    void Aggro(Unit* pWho) override
    {
        SetAbility();
        InitSentinelsNear(pWho);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoTransferAbility();
    }

    // this way will make it quite possible that sentinels get the same buff as others, need to fix that, it should be one unique each
    void SetAbility()
    {
        switch (urand(0, 8))
        {
            case 0: m_uiMyAbility = SPELL_MENDING; break;
            case 1: m_uiMyAbility = SPELL_PERIODIC_KNOCK_AWAY; break;
            case 2: m_uiMyAbility = SPELL_PERIODIC_MANA_BURN; break;
            case 3: m_uiMyAbility = SPELL_FIRE_ARCANE_REFLECT; break;
            case 4: m_uiMyAbility = SPELL_SHADOW_FROST_REFLECT; break;
            case 5: m_uiMyAbility = SPELL_THORNS; break;
            case 6: m_uiMyAbility = SPELL_PERIODIC_THUNDERCLAP; break;
            case 7: m_uiMyAbility = SPELL_MORTAL_STRIKE; break;
            case 8: m_uiMyAbility = SPELL_PERIODIC_SHADOW_STORM; break;
        }

        DoCastSpellIfCan(m_creature, m_uiMyAbility, CAST_TRIGGERED);
    }

    void DoTransferAbility()
    {
        for (GuidList::const_iterator itr = m_lAssistList.begin(); itr != m_lAssistList.end(); ++itr)
        {
            if (Creature* pBuddy = m_creature->GetMap()->GetCreature(*itr))
            {
                if (*itr == m_creature->GetObjectGuid())
                    continue;

                if (!pBuddy->isAlive())
                    continue;

                pBuddy->SetHealth(pBuddy->GetMaxHealth());
                DoCastSpellIfCan(pBuddy, m_uiMyAbility, CAST_TRIGGERED);
            }
        }
    }

    void InitSentinelsNear(Unit* pTarget)
    {
        if (!m_lAssistList.empty())
        {
            for (GuidList::const_iterator itr = m_lAssistList.begin(); itr != m_lAssistList.end(); ++itr)
            {
                if (*itr == m_creature->GetObjectGuid())
                    continue;

                if (Creature* pBuddy = m_creature->GetMap()->GetCreature(*itr))
                {
                    if (pBuddy->isAlive())
                        pBuddy->AI()->AttackStart(pTarget);
                }
            }

            return;
        }

        std::list<Creature*> lAssistList;
        GetCreatureListWithEntryInGrid(lAssistList, m_creature, m_creature->GetEntry(), 80.0f);

        for (std::list<Creature*>::iterator iter = lAssistList.begin(); iter != lAssistList.end(); ++iter)
        {
            m_lAssistList.push_back((*iter)->GetObjectGuid());

            if ((*iter)->GetObjectGuid() == m_creature->GetObjectGuid())
                continue;

            (*iter)->AI()->AttackStart(pTarget);
        }

        if (m_lAssistList.size() != MAX_BUDDY)
            script_error_log("npc_anubisath_sentinel for %s found too few/too many buddies, expected %u.", m_creature->GetGuidStr().c_str(), MAX_BUDDY);
    }

    void UpdateAI(const uint32 /*uiDiff*/) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (!m_bEnraged && m_creature->GetHealthPercent() < 30.0f)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
            {
                DoScriptText(EMOTE_GENERIC_FRENZY, m_creature);
                m_bEnraged = true;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_anubisath_sentinel(Creature* pCreature)
{
    return new npc_anubisath_sentinelAI(pCreature);
}

void AddSC_mob_anubisath_sentinel()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "mob_anubisath_sentinel";
    pNewScript->GetAI = &GetAI_npc_anubisath_sentinel;
    pNewScript->RegisterSelf();
}
