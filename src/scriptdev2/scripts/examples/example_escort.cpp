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
SDName: Example_Escort
SD%Complete: 100
SDComment: Script used for testing escortAI
SDCategory: Script Examples
EndScriptData */

#include "precompiled.h"
#include "escort_ai.h"

enum
{
    NPC_FELBOAR                 = 21878,

    SPELL_DEATH_COIL            = 33130,
    SPELL_ELIXIR_OF_FORTITUDE   = 3593,
    SPELL_BLUE_FIREWORK         = 11540,

    SAY_AGGRO1                  = -1999910,
    SAY_AGGRO2                  = -1999911,
    SAY_WP_1                    = -1999912,
    SAY_WP_2                    = -1999913,
    SAY_WP_3                    = -1999914,
    SAY_WP_4                    = -1999915,
    SAY_DEATH_1                 = -1999916,
    SAY_DEATH_2                 = -1999917,
    SAY_DEATH_3                 = -1999918,
    SAY_SPELL                   = -1999919,
    SAY_RAND_1                  = -1999920,
    SAY_RAND_2                  = -1999921
};

#define GOSSIP_ITEM_1   "Click to Test Escort(Attack, Run)"
#define GOSSIP_ITEM_2   "Click to Test Escort(NoAttack, Walk)"
#define GOSSIP_ITEM_3   "Click to Test Escort(NoAttack, Run)"

struct example_escortAI : public npc_escortAI
{
    // CreatureAI functions
    example_escortAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    uint32 m_uiDeathCoilTimer;
    uint32 m_uiChatTimer;

    // Is called after each combat, so usally only reset combat-stuff here
    void Reset() override
    {
        m_uiDeathCoilTimer = 4000;
        m_uiChatTimer = 4000;
    }

    void JustSummoned(Creature* pSummoned) override
    {
        pSummoned->AI()->AttackStart(m_creature);
    }

    // Pure Virtual Functions (Have to be implemented)
    void WaypointReached(uint32 uiWP) override
    {
        switch (uiWP)
        {
            case 1:
                DoScriptText(SAY_WP_1, m_creature);
                break;
            case 3:
                DoScriptText(SAY_WP_2, m_creature);
                m_creature->SummonCreature(NPC_FELBOAR, m_creature->GetPositionX() + 5.0f, m_creature->GetPositionY() + 7.0f, m_creature->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_OOC_DESPAWN, 3000);
                break;
            case 4:
                if (Player* pTmpPlayer = GetPlayerForEscort())
                {
                    // pTmpPlayer is the target of the text
                    DoScriptText(SAY_WP_3, m_creature, pTmpPlayer);
                    // pTmpPlayer is the source of the text
                    DoScriptText(SAY_WP_4, pTmpPlayer);
                }
                break;
        }
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
        {
            if (Player* pTemp = GetPlayerForEscort())
                DoScriptText(SAY_AGGRO1, m_creature, pTemp);
        }
        else
            DoScriptText(SAY_AGGRO2, m_creature);
    }

    // Only overwrite if there is something special
    void JustDied(Unit* pKiller) override
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
        {
            if (Player* pTemp = GetPlayerForEscort())
            {
                // not a likely case, code here for the sake of example
                if (pKiller == m_creature)
                {
                    // This is actually a whisper. You control the text type in database
                    DoScriptText(SAY_DEATH_1, m_creature, pTemp);
                }
                else
                    DoScriptText(SAY_DEATH_2, m_creature, pTemp);
            }
        }
        else
            DoScriptText(SAY_DEATH_3, m_creature);

        // Fail quest for group - if you don't implement JustDied in your script, this will automatically work
        npc_escortAI::JustDied(pKiller);
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        // Combat check
        if (m_creature->SelectHostileTarget() && m_creature->getVictim())
        {
            if (m_uiDeathCoilTimer < uiDiff)
            {
                DoScriptText(SAY_SPELL, m_creature);
                m_creature->CastSpell(m_creature->getVictim(), SPELL_DEATH_COIL, TRIGGERED_NONE);
                m_uiDeathCoilTimer = 4000;
            }
            else
                m_uiDeathCoilTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
        else
        {
            // Out of combat but being escorted
            if (HasEscortState(STATE_ESCORT_ESCORTING))
            {
                if (m_uiChatTimer < uiDiff)
                {
                    if (m_creature->HasAura(SPELL_ELIXIR_OF_FORTITUDE, EFFECT_INDEX_0))
                    {
                        DoScriptText(SAY_RAND_1, m_creature);
                        m_creature->CastSpell(m_creature, SPELL_BLUE_FIREWORK, TRIGGERED_NONE);
                    }
                    else
                    {
                        DoScriptText(SAY_RAND_2, m_creature);
                        m_creature->CastSpell(m_creature, SPELL_ELIXIR_OF_FORTITUDE, TRIGGERED_NONE);
                    }

                    m_uiChatTimer = 12000;
                }
                else
                    m_uiChatTimer -= uiDiff;
            }
        }
    }
};

CreatureAI* GetAI_example_escort(Creature* pCreature)
{
    return new example_escortAI(pCreature);
}

bool GossipHello_example_escort(Player* pPlayer, Creature* pCreature)
{
    pPlayer->TalkedToCreature(pCreature->GetEntry(), pCreature->GetObjectGuid());
    pPlayer->PrepareGossipMenu(pCreature, pPlayer->GetDefaultGossipMenuForSource(pCreature));

    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

    pPlayer->SendPreparedGossip(pCreature);

    return true;
}

bool GossipSelect_example_escort(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    example_escortAI* pEscortAI = dynamic_cast<example_escortAI*>(pCreature->AI());

    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->CLOSE_GOSSIP_MENU();

            if (pEscortAI)
                pEscortAI->Start(true, pPlayer);
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->CLOSE_GOSSIP_MENU();

            if (pEscortAI)
                pEscortAI->Start(false, pPlayer);
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->CLOSE_GOSSIP_MENU();

            if (pEscortAI)
                pEscortAI->Start(true, pPlayer);
            break;
        default:
            return false;                                   // nothing defined      -> mangos core handling
    }

    return true;                                            // no default handling  -> prevent mangos core handling
}

void AddSC_example_escort()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "example_escort";
    pNewScript->GetAI = &GetAI_example_escort;
    pNewScript->pGossipHello = &GossipHello_example_escort;
    pNewScript->pGossipSelect = &GossipSelect_example_escort;
    pNewScript->RegisterSelf(false);
}
