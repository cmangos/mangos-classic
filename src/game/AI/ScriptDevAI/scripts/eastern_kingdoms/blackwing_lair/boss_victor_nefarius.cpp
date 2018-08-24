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

#include "AI/ScriptDevAI/include/precompiled.h"
#include "blackwing_lair.h"

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

struct boss_victor_nefariusAI : public ScriptedAI, private DialogueHelper
{
    boss_victor_nefariusAI(Creature* pCreature) : ScriptedAI(pCreature),
        DialogueHelper(aIntroDialogue)
    {
        // Select the 2 different drakonids that we are going to use for all instance lifetime
        DoCastSpellIfCan(m_creature, SPELL_TUNNEL_SELECTION);

        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        InitializeDialogueHelper(m_pInstance);
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiShadowBoltTimer;
    uint32 m_uiFearTimer;
    uint32 m_uiShadowboltVolleyTimer;
    uint32 m_uiSilenceTimer;
    uint32 m_uiShadowCommandTimer;
    uint32 m_uiShadowBlinkTimer;

    void Reset() override
    {
        // Check the map id because the same creature entry is involved in other scripted event in other instance
        if (m_creature->GetMapId() != MAP_ID_BWL)
            return;

        m_uiShadowBoltTimer         = 3 * IN_MILLISECONDS;
        m_uiFearTimer               = 8 * IN_MILLISECONDS;
        m_uiShadowboltVolleyTimer   = 13 * IN_MILLISECONDS;
        m_uiSilenceTimer            = 23 * IN_MILLISECONDS;
        m_uiShadowCommandTimer      = 30 * IN_MILLISECONDS;
        m_uiShadowBlinkTimer        = 40 * IN_MILLISECONDS;

        // set gossip flag to begin the event
        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_NEFARIAN, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_NEFARIAN, FAIL);
    }

    void JustDidDialogueStep(int32 iEntry) override
    {
        // Start combat after the dialogue is finished
        if (iEntry == SPELL_SHADOWBLINK)
        {
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);
            m_creature->SetFactionTemporary(FACTION_BLACK_DRAGON, TEMPFACTION_RESTORE_REACH_HOME | TEMPFACTION_RESTORE_RESPAWN);
            DoCastSpellIfCan(m_creature, SPELL_NEFARIUS_BARRIER, CAST_TRIGGERED);
            DoCastSpellIfCan(m_creature, SPELL_SHADOWBLINK, CAST_TRIGGERED);
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
            m_creature->SetInCombatWithZone();
            SetCombatMovement(false);   // Nefarius teleports via Shadowblink, he does not move

            // Spawn the two spawner NPCs that will handle the drakonids spawning (both brood and chromatic)
            if (m_pInstance)
            {
                // Left
                m_creature->SummonCreature(m_pInstance->GetData(TYPE_NEFA_LTUNNEL), aNefarianLocs[0].m_fX, aNefarianLocs[0].m_fY, aNefarianLocs[0].m_fZ, 5.000f, TEMPSPAWN_TIMED_OOC_OR_CORPSE_DESPAWN, HOUR * IN_MILLISECONDS);
                // Right
                m_creature->SummonCreature(m_pInstance->GetData(TYPE_NEFA_RTUNNEL), aNefarianLocs[1].m_fX, aNefarianLocs[1].m_fY, aNefarianLocs[1].m_fZ, 5.000f, TEMPSPAWN_TIMED_OOC_OR_CORPSE_DESPAWN, HOUR * IN_MILLISECONDS);
            }
        }
    }

    void DoStartIntro()
    {
        StartNextDialogueText(SAY_GAMESBEGIN_1);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_creature->GetMapId() != MAP_ID_BWL)
            return;

        DialogueUpdate(uiDiff);

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Shadowbolt Timer
        if (m_uiShadowBoltTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_SHADOWBOLT) == CAST_OK)
                    m_uiShadowBoltTimer = urand(2 * IN_MILLISECONDS, 4 * IN_MILLISECONDS);
            }
        }
        else
            m_uiShadowBoltTimer -= uiDiff;

        // Fear Timer
        if (m_uiFearTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_FEAR) == CAST_OK)
                    m_uiFearTimer = urand(10 * IN_MILLISECONDS, 20 * IN_MILLISECONDS);
            }
        }
        else
            m_uiFearTimer -= uiDiff;

        // Shadowbolt Volley
        if (m_uiShadowboltVolleyTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SHADOWBOLT_VOLLEY) == CAST_OK)
                m_uiShadowboltVolleyTimer = urand(19 * IN_MILLISECONDS, 28 * IN_MILLISECONDS);
        }
        else
            m_uiShadowboltVolleyTimer -= uiDiff;

        // Silence
        if (m_uiSilenceTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_SILENCE) == CAST_OK)
                    m_uiSilenceTimer = urand(14 * IN_MILLISECONDS, 23 * IN_MILLISECONDS);
            }
        }
        else
            m_uiSilenceTimer -= uiDiff;

        // Shadow Command
        if (m_uiShadowCommandTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_SHADOW_COMMAND) == CAST_OK)
                    m_uiShadowCommandTimer = urand(24 * IN_MILLISECONDS, 30 * IN_MILLISECONDS);
            }
        }
        else
            m_uiShadowCommandTimer -= uiDiff;

        // ShadowBlink
        if (m_uiShadowBlinkTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SHADOWBLINK) == CAST_OK)
                m_uiShadowBlinkTimer = urand(30 * IN_MILLISECONDS, 40 * IN_MILLISECONDS);
        }
        else
            m_uiShadowBlinkTimer -= uiDiff;
    }
};

UnitAI* GetAI_boss_victor_nefarius(Creature* pCreature)
{
    return new boss_victor_nefariusAI(pCreature);
}

bool GossipHello_boss_victor_nefarius(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->GetMapId() != MAP_ID_BWL)
        return true;

    pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NEFARIUS_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_NEFARIUS_1, pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_boss_victor_nefarius(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (pCreature->GetMapId() != MAP_ID_BWL)
        return true;

    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pCreature->HandleEmote(EMOTE_ONESHOT_TALK);
            pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NEFARIUS_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_NEFARIUS_2, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pCreature->HandleEmote(EMOTE_ONESHOT_TALK);
            pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NEFARIUS_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_NEFARIUS_3, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            pPlayer->CLOSE_GOSSIP_MENU();
            // Start the intro event
            if (boss_victor_nefariusAI* pBossAI = dynamic_cast<boss_victor_nefariusAI*>(pCreature->AI()))
                pBossAI->DoStartIntro();
            break;
    }
    return true;
}

void AddSC_boss_victor_nefarius()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_victor_nefarius";
    pNewScript->GetAI = &GetAI_boss_victor_nefarius;
    pNewScript->pGossipHello = &GossipHello_boss_victor_nefarius;
    pNewScript->pGossipSelect = &GossipSelect_boss_victor_nefarius;
    pNewScript->RegisterSelf();
}
