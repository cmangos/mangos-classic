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
SDName: Boss_Victor_Nefarius
SD%Complete: 100
SDComment:
SDCategory: Blackwing Lair
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "blackwing_lair.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    SAY_GAMESBEGIN_1                = -1469004,
    SAY_GAMESBEGIN_2                = -1469005,

    GOSSIP_ITEM_NEFARIUS_1          = -3469000,
    GOSSIP_ITEM_NEFARIUS_2          = -3469001,
    GOSSIP_ITEM_NEFARIUS_3          = -3469002,
    GOSSIP_TEXT_NEFARIUS_1          = 7134,
    GOSSIP_TEXT_NEFARIUS_2          = 7198,
    GOSSIP_TEXT_NEFARIUS_3          = 7199,

    SPELL_NEFARIUS_BARRIER          = 22663,                // immunity in phase 1
    SPELL_SHADOWBLINK               = 22664,                // triggers a random from spells (22668 - 22676)
    SPELL_SHADOWBOLT_VOLLEY         = 22665,
    SPELL_SILENCE                   = 22666,
    SPELL_SHADOW_COMMAND            = 22667,
    SPELL_SHADOWBOLT                = 22677,
    SPELL_FEAR                      = 22678,
    SPELL_TUNNEL_SELECTION          = 23343,
    // wotlk + uses weird 65634 spell

    MAP_ID_BWL                      = 469,

    FACTION_BLACK_DRAGON            = 103
};

static const DialogueEntry aIntroDialogue[] =
{
    {SAY_GAMESBEGIN_1,    NPC_LORD_VICTOR_NEFARIUS,     4000},
    {SAY_GAMESBEGIN_2,    NPC_LORD_VICTOR_NEFARIUS,     5000},
    {SPELL_SHADOWBLINK,   0,                            0},
    {0, 0, 0},
};

enum VictorNefariusActions
{
    NEFARIUS_SHADOW_BOLT,
    NEFARIUS_FEAR,
    NEFARIUS_SHADOWBOLT_VOLLEY,
    NEFARIUS_SILENCE,
    NEFARIUS_SHADOW_COMMAND,
    NEFARIUS_SHADOW_BLINK,
    NEFARIUS_ACTION_MAX,
};

struct boss_victor_nefariusAI : public CombatAI, private DialogueHelper
{
    boss_victor_nefariusAI(Creature* creature) : CombatAI(creature, NEFARIUS_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData())),
        DialogueHelper(aIntroDialogue)
    {
        if (m_creature->GetMapId() != MAP_ID_BWL)
            return;
        // Select the 2 different drakonids that we are going to use for all instance lifetime
        m_creature->CastSpell(nullptr, SPELL_TUNNEL_SELECTION, TRIGGERED_NONE);
        InitializeDialogueHelper(m_instance);
        AddCombatAction(NEFARIUS_SHADOW_BOLT, uint32(3 * IN_MILLISECONDS));
        AddCombatAction(NEFARIUS_FEAR, uint32(8 * IN_MILLISECONDS));
        AddCombatAction(NEFARIUS_SHADOWBOLT_VOLLEY, uint32(13 * IN_MILLISECONDS));
        AddCombatAction(NEFARIUS_SILENCE, uint32(23 * IN_MILLISECONDS));
        AddCombatAction(NEFARIUS_SHADOW_COMMAND, uint32(30 * IN_MILLISECONDS));
        AddCombatAction(NEFARIUS_SHADOW_BLINK, uint32(40 * IN_MILLISECONDS));
    }

    ScriptedInstance* m_instance;

    void Reset() override
    {
        if (m_creature->GetMapId() != MAP_ID_BWL)
            return;
        CombatAI::Reset();
        // set gossip flag to begin the event
        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
        SetCombatScriptStatus(false);
    }

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_NEFARIAN, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_NEFARIAN, FAIL);
    }

    void JustDidDialogueStep(int32 entry) override
    {
        // Start combat after the dialogue is finished
        if (entry == SPELL_SHADOWBLINK)
        {
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);
            m_creature->SetFactionTemporary(FACTION_BLACK_DRAGON, TEMPFACTION_RESTORE_REACH_HOME | TEMPFACTION_RESTORE_RESPAWN);
            DoCastSpellIfCan(nullptr, SPELL_NEFARIUS_BARRIER, CAST_TRIGGERED);
            DoCastSpellIfCan(nullptr, SPELL_SHADOWBLINK);
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
            m_creature->SetInCombatWithZone();
            SetCombatMovement(false); // Nefarius teleports via Shadowblink, he does not move

            // Spawn the two spawner NPCs that will handle the drakonids spawning (both brood and chromatic)
            if (m_instance)
            {
                // Left
                m_creature->SummonCreature(m_instance->GetData(TYPE_NEFA_LTUNNEL), aNefarianLocs[0].m_fX, aNefarianLocs[0].m_fY, aNefarianLocs[0].m_fZ, 5.000f, TEMPSPAWN_TIMED_OOC_OR_CORPSE_DESPAWN, HOUR * IN_MILLISECONDS);
                // Right
                m_creature->SummonCreature(m_instance->GetData(TYPE_NEFA_RTUNNEL), aNefarianLocs[1].m_fX, aNefarianLocs[1].m_fY, aNefarianLocs[1].m_fZ, 5.000f, TEMPSPAWN_TIMED_OOC_OR_CORPSE_DESPAWN, HOUR * IN_MILLISECONDS);
            }
        }
    }

    void DoStartIntro()
    {
        StartNextDialogueText(SAY_GAMESBEGIN_1);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case NEFARIUS_SHADOW_BOLT:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_SHADOWBOLT, SELECT_FLAG_PLAYER))
                    if (DoCastSpellIfCan(target, SPELL_SHADOWBOLT) == CAST_OK)
                        ResetCombatAction(action, urand(2 * IN_MILLISECONDS, 4 * IN_MILLISECONDS));
                break;
            }
            case NEFARIUS_FEAR:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1, SPELL_FEAR, SELECT_FLAG_PLAYER))
                    if (DoCastSpellIfCan(target, SPELL_FEAR) == CAST_OK)
                        ResetCombatAction(action, urand(10 * IN_MILLISECONDS, 20 * IN_MILLISECONDS));
                break;
            }
            case NEFARIUS_SHADOWBOLT_VOLLEY:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_SHADOWBOLT_VOLLEY) == CAST_OK)
                    ResetCombatAction(action, urand(19 * IN_MILLISECONDS, 28 * IN_MILLISECONDS));
                break;
            }
            case NEFARIUS_SILENCE:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_SILENCE, SELECT_FLAG_PLAYER))
                    if (DoCastSpellIfCan(target, SPELL_SILENCE) == CAST_OK)
                        ResetCombatAction(action, urand(14 * IN_MILLISECONDS, 23 * IN_MILLISECONDS));
                break;
            }
            case NEFARIUS_SHADOW_COMMAND:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1, SPELL_SHADOW_COMMAND, SELECT_FLAG_PLAYER))
                    if (DoCastSpellIfCan(target, SPELL_SHADOW_COMMAND) == CAST_OK)
                        ResetCombatAction(action, urand(24 * IN_MILLISECONDS, 30 * IN_MILLISECONDS));
                break;
            }
            case NEFARIUS_SHADOW_BLINK:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_SHADOWBLINK) == CAST_OK)
                    ResetCombatAction(action, urand(30 * IN_MILLISECONDS, 40 * IN_MILLISECONDS));
                break;
            }
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        if (m_creature->GetMapId() != MAP_ID_BWL)
            return;

        DialogueUpdate(diff);
        CombatAI::UpdateAI(diff);
    }
};

bool GossipHello_boss_victor_nefarius(Player* player, Creature* creature)
{
    if (creature->GetMapId() != MAP_ID_BWL)
        return true;

    player->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NEFARIUS_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->SEND_GOSSIP_MENU(GOSSIP_TEXT_NEFARIUS_1, creature->GetObjectGuid());
    return true;
}

bool GossipSelect_boss_victor_nefarius(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 action)
{
    if (creature->GetMapId() != MAP_ID_BWL)
        return true;

    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            creature->HandleEmote(EMOTE_ONESHOT_TALK);
            player->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NEFARIUS_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(GOSSIP_TEXT_NEFARIUS_2, creature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            creature->HandleEmote(EMOTE_ONESHOT_TALK);
            player->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NEFARIUS_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(GOSSIP_TEXT_NEFARIUS_3, creature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            player->CLOSE_GOSSIP_MENU();
            // Start the intro event
            if (boss_victor_nefariusAI* bossAI = dynamic_cast<boss_victor_nefariusAI*>(creature->AI()))
                bossAI->DoStartIntro();
            break;
    }
    return true;
}

void AddSC_boss_victor_nefarius()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_victor_nefarius";
    pNewScript->GetAI = &GetNewAIInstance<boss_victor_nefariusAI>;
    pNewScript->pGossipHello = &GossipHello_boss_victor_nefarius;
    pNewScript->pGossipSelect = &GossipSelect_boss_victor_nefarius;
    pNewScript->RegisterSelf();
}
