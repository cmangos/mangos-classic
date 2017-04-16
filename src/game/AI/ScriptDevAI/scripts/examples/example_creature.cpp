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
SDName: Example_Creature
SD%Complete: 100
SDComment: Short custom scripting example
SDCategory: Script Examples
EndScriptData */

#include "precompiled.h"

// **** This script is designed as an example for others to build on ****
// **** Please modify whatever you'd like to as this script is only for developement ****

// **** Script Info ****
// This script is written in a way that it can be used for both friendly and hostile monsters
// Its primary purpose is to show just how much you can really do with scripts
// I recommend trying it out on both an agressive NPC and on friendly npc

// **** Quick Info ****
// Functions with Handled Function marked above them are functions that are called automatically by the core
// Functions that are marked Custom Function are functions I've created to simplify code

enum
{
    // List of text id's. The text is stored in database, also in a localized version
    // (if translation not exist for the textId, default english text will be used)
    SAY_AGGRO       = -1999900,
    SAY_RANDOM_0    = -1999901,
    SAY_RANDOM_1    = -1999902,
    SAY_RANDOM_2    = -1999903,
    SAY_RANDOM_3    = -1999904,
    SAY_RANDOM_4    = -1999905,
    SAY_BESERK      = -1999906,
    SAY_PHASE       = -1999907,
    SAY_DANCE       = -1999908,
    SAY_SALUTE      = -1999909,

    // List of used spells
    SPELL_BUFF      = 25661,
    SPELL_ONE       = 12555,
    SPELL_ONE_ALT   = 24099,
    SPELL_TWO       = 10017,
    SPELL_THREE     = 26027,
    SPELL_ENRAGE    = 23537,
    SPELL_BESERK    = 32309,

    // Some other information we need to store
    TEXT_ID_GREET   = 907,
    FACTION_WORGEN  = 24
};

// List of gossip item texts. Items will appear in the gossip window.
// Actually such gossip can already be handled in normal World-Database
// If (and only if) a gossip must be handled within SD2, then it should be moved to SD2-database!
#define GOSSIP_ITEM     "I'm looking for a fight"

struct example_creatureAI : public ScriptedAI
{
    // *** HANDLED FUNCTION ***
    // This is the constructor, called only once when the creature is first created
    example_creatureAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    // *** CUSTOM VARIABLES ****
    // These variables are for use only by this individual script.
    // Nothing else will ever call them but us.

    uint32 m_uiSayTimer;                                    // Timer for random chat
    uint32 m_uiRebuffTimer;                                 // Timer for rebuffing
    uint32 m_uiSpellOneTimer;                               // Timer for spell 1 when in combat
    uint32 m_uiSpellTwoTimer;                               // Timer for spell 1 when in combat
    uint32 m_uiSpellThreeTimer;                             // Timer for spell 1 when in combat
    uint32 m_uiBeserkTimer;                                 // Timer until we go into Beserk (enraged) mode
    uint32 m_uiPhase;                                       // The current battle phase we are in
    uint32 m_uiPhaseTimer;                                  // Timer until phase transition

    // *** HANDLED FUNCTION ***
    // This is called whenever the core decides we need to evade
    void Reset() override
    {
        m_uiPhase = 1;                                      // Start in phase 1
        m_uiPhaseTimer = 60000;                             // 60 seconds
        m_uiSpellOneTimer = 5000;                           // 5 seconds
        m_uiSpellTwoTimer = 37000;                          // 37 seconds
        m_uiSpellThreeTimer = 19000;                        // 19 seconds
        m_uiBeserkTimer = 120000;                           // 2 minutes
    }

    // *** HANDLED FUNCTION ***
    // Aggro is called when we enter combat, against an enemy, and haven't been in combat before
    void Aggro(Unit* pWho) override
    {
        // Say some stuff
        DoScriptText(SAY_AGGRO, m_creature, pWho);
    }

    // *** HANDLED FUNCTION ***
    // Our Recive emote function
    void ReceiveEmote(Player* /*pPlayer*/, uint32 uiTextEmote) override
    {
        m_creature->HandleEmote(uiTextEmote);

        switch (uiTextEmote)
        {
            case TEXTEMOTE_DANCE:
                DoScriptText(SAY_DANCE, m_creature);
                break;
            case TEXTEMOTE_SALUTE:
                DoScriptText(SAY_SALUTE, m_creature);
                break;
        }
    }

    // *** HANDLED FUNCTION ***
    // Update AI is called Every single map update (roughly once every 100ms if a player is within the grid)
    void UpdateAI(const uint32 uiDiff) override
    {
        // Out of combat timers
        if (!m_creature->getVictim())
        {
            // Random Say timer
            if (m_uiSayTimer < uiDiff)
            {
                // Random switch between 5 outcomes
                switch (urand(0, 4))
                {
                    case 0: DoScriptText(SAY_RANDOM_0, m_creature); break;
                    case 1: DoScriptText(SAY_RANDOM_1, m_creature); break;
                    case 2: DoScriptText(SAY_RANDOM_2, m_creature); break;
                    case 3: DoScriptText(SAY_RANDOM_3, m_creature); break;
                    case 4: DoScriptText(SAY_RANDOM_4, m_creature); break;
                }

                m_uiSayTimer = 45 * IN_MILLISECONDS;        // Say something agian in 45 seconds
            }
            else
                m_uiSayTimer -= uiDiff;

            // Rebuff timer
            if (m_uiRebuffTimer < uiDiff)
            {
                DoCastSpellIfCan(m_creature, SPELL_BUFF);
                // Rebuff agian in 15 minutes
                m_uiRebuffTimer = 15 * MINUTE * IN_MILLISECONDS;
            }
            else
                m_uiRebuffTimer -= uiDiff;
        }

        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Abilities of all phases
        // Spell One timer
        if (m_uiSpellOneTimer < uiDiff)
        {
            // Cast spell one on our current target.
            if (rand() % 50 > 10)
                DoCastSpellIfCan(m_creature->getVictim(), SPELL_ONE_ALT);
            else if (m_creature->IsWithinDist(m_creature->getVictim(), 25.0f))
                DoCastSpellIfCan(m_creature->getVictim(), SPELL_ONE);

            m_uiSpellOneTimer = 5000;
        }
        else
            m_uiSpellOneTimer -= uiDiff;

        // Spell Two timer
        if (m_uiSpellTwoTimer < uiDiff)
        {
            // Cast spell two on self (AoE spell with only self-target) if we can
            if (DoCastSpellIfCan(m_creature, SPELL_TWO) == CAST_OK)
                m_uiSpellTwoTimer = 37 * IN_MILLISECONDS;   // Only Update Timer, if we could start casting
        }
        else
            m_uiSpellTwoTimer -= uiDiff;

        // End of abliities of all phases

        // Phase 1 abilities
        if (m_uiPhase == 1)
        {
            // Phase timer
            if (m_uiPhaseTimer < uiDiff)
            {
                // Only switch phase and display phase-switich text, if out cast was started sucessfull
                if (DoCastSpellIfCan(m_creature, SPELL_ENRAGE) == CAST_OK)
                {
                    // Go to next phase
                    ++m_uiPhase;
                    DoScriptText(SAY_PHASE, m_creature);
                }
            }
            else
                m_uiPhaseTimer -= uiDiff;
        }
        // Phase 2 abilities
        else if (m_uiPhase > 1)
        {
            // Spell Three timer
            if (m_uiSpellThreeTimer < uiDiff)
            {
                // Cast spell three on self (AoE spell with only self-target)
                if (DoCastSpellIfCan(m_creature, SPELL_THREE) == CAST_OK)
                    m_uiSpellThreeTimer = 19000;
            }
            else
                m_uiSpellThreeTimer -= uiDiff;

            // Beserk timer
            if (m_uiBeserkTimer < uiDiff)
            {
                // Cast uber death spell if possible
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_BESERK) == CAST_OK)
                {
                    // Say our line if we cast
                    DoScriptText(SAY_BESERK, m_creature, m_creature->getVictim());

                    // Cast our beserk spell agian in 12 seconds (if we didn't kill everyone)
                    m_uiBeserkTimer = 12000;
                }
            }
            else
                m_uiBeserkTimer -= uiDiff;
        }

        // Normal behaviour: if possible mobs do attack with melee
        DoMeleeAttackIfReady();
    }
};

// This is the GetAI method used by all scripts that involve AI
// It is called every time a new creature using this script is created
CreatureAI* GetAI_example_creature(Creature* pCreature)
{
    return new example_creatureAI(pCreature);
}

// This function is called when the player opens the gossip menu
// In this case as there is nothing special about this gossip dialogue, it should be moved to world-DB
bool GossipHello_example_creature(Player* pPlayer, Creature* pCreature)
{
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    pPlayer->SEND_GOSSIP_MENU(TEXT_ID_GREET, pCreature->GetObjectGuid());

    return true;
}

// This function is called when the player clicks an option on the gossip menu
// In this case here the faction change could be handled by world-DB gossip, hence it should be handled there!
bool GossipSelect_example_creature(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        // Set our faction to hostile towards all
        pCreature->SetFactionTemporary(FACTION_WORGEN, TEMPFACTION_RESTORE_RESPAWN);
        pCreature->AI()->AttackStart(pPlayer);
    }

    return true;
}

// This is the actual function called only once durring InitScripts()
// It must define all handled functions that are to be run in this script
void AddSC_example_creature()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "example_creature";
    pNewScript->GetAI = &GetAI_example_creature;
    pNewScript->pGossipHello = &GossipHello_example_creature;
    pNewScript->pGossipSelect = &GossipSelect_example_creature;
    pNewScript->RegisterSelf(false);
}
