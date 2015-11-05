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
SDName: Tanaris
SD%Complete: 80
SDComment: Quest support: 648, 1560, 2954, 4005, 10277.
SDCategory: Tanaris
EndScriptData */

/* ContentData
mob_aquementas
npc_oox17tn
npc_stone_watcher_of_norgannon
npc_tooga
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"
#include "follower_ai.h"

/*######
## mob_aquementas
######*/

enum
{
    AGGRO_YELL_AQUE         = -1000168,

    SPELL_AQUA_JET          = 13586,
    SPELL_FROST_SHOCK       = 15089,

    ITEM_SILVER_TOTEM       = 11522,
    ITEM_BOOK_AQUOR         = 11169,
    ITEM_SILVERY_CLAWS      = 11172,
    ITEM_IRONTREE_HEART     = 11173,

    FACTION_FRIENDLY        = 35,
    FACTION_ELEMENTAL       = 91,
};

struct mob_aquementasAI : public ScriptedAI
{
    mob_aquementasAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiSwitchFactionTimer;
    uint32 m_uiFrostShockTimer;
    uint32 m_uiAquaJetTimer;

    void Reset() override
    {
        m_uiSwitchFactionTimer  = 10000;
        m_uiAquaJetTimer        = 5000;
        m_uiFrostShockTimer     = 1000;

        m_creature->setFaction(FACTION_FRIENDLY);           // TODO: Either do this way, or might require a DB change
    }

    void SendItem(Player* pReceiver)
    {
        if (pReceiver->HasItemCount(ITEM_BOOK_AQUOR, 1) &&
                pReceiver->HasItemCount(ITEM_SILVERY_CLAWS, 11) &&
                pReceiver->HasItemCount(ITEM_IRONTREE_HEART, 1) &&
                !pReceiver->HasItemCount(ITEM_SILVER_TOTEM, 1))
        {
            if (Item* pItem = pReceiver->StoreNewItemInInventorySlot(ITEM_SILVER_TOTEM, 1))
                pReceiver->SendNewItem(pItem, 1, true, false);
        }
    }

    void Aggro(Unit* pWho) override
    {
        DoScriptText(AGGRO_YELL_AQUE, m_creature, pWho);

        Player* pInvokedPlayer = pWho->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (pInvokedPlayer)
            SendItem(pInvokedPlayer);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiSwitchFactionTimer)
        {
            if (m_uiSwitchFactionTimer <= uiDiff)
            {
                m_creature->setFaction(FACTION_ELEMENTAL);
                m_uiSwitchFactionTimer = 0;
            }
            else
                m_uiSwitchFactionTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiFrostShockTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->getVictim(), SPELL_FROST_SHOCK);
            m_uiFrostShockTimer = 15000;
        }
        else
            m_uiFrostShockTimer -= uiDiff;

        if (m_uiAquaJetTimer < uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_AQUA_JET);
            m_uiAquaJetTimer = 15000;
        }
        else
            m_uiAquaJetTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_aquementas(Creature* pCreature)
{
    return new mob_aquementasAI(pCreature);
}

/*######
## npc_oox17tn
######*/

enum
{
    SAY_OOX_START           = -1000287,
    SAY_OOX_AGGRO1          = -1000288,
    SAY_OOX_AGGRO2          = -1000289,
    SAY_OOX_AMBUSH          = -1000290,
    SAY_OOX17_AMBUSH_REPLY  = -1000291,
    SAY_OOX_END             = -1000292,

    QUEST_RESCUE_OOX_17TN   = 648,

    NPC_SCORPION            = 7803,
    NPC_SCOFFLAW            = 7805,
    NPC_SHADOW_MAGE         = 5617
};

struct npc_oox17tnAI : public npc_escortAI
{
    npc_oox17tnAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    void WaypointReached(uint32 i) override
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch (i)
        {
                // 1. Ambush: 3 scorpions
            case 22:
                DoScriptText(SAY_OOX_AMBUSH, m_creature);
                m_creature->SummonCreature(NPC_SCORPION, -8340.70f, -4448.17f, 9.17f, 3.10f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                m_creature->SummonCreature(NPC_SCORPION, -8343.18f, -4444.35f, 9.44f, 2.35f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                m_creature->SummonCreature(NPC_SCORPION, -8348.70f, -4457.80f, 9.58f, 2.02f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
                break;
                // 2. Ambush: 2 Rogues & 1 Shadow Mage
            case 28:
                DoScriptText(SAY_OOX_AMBUSH, m_creature);

                m_creature->SummonCreature(NPC_SCOFFLAW, -7488.02f, -4786.56f, 10.67f, 3.74f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                m_creature->SummonCreature(NPC_SHADOW_MAGE, -7486.41f, -4791.55f, 10.54f, 3.26f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);

                if (Creature* pCreature = m_creature->SummonCreature(NPC_SCOFFLAW, -7488.47f, -4800.77f, 9.77f, 2.50f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000))
                    DoScriptText(SAY_OOX17_AMBUSH_REPLY, pCreature);

                break;
            case 34:
                DoScriptText(SAY_OOX_END, m_creature);
                // Award quest credit
                pPlayer->GroupEventHappens(QUEST_RESCUE_OOX_17TN, m_creature);
                break;
        }
    }

    void Reset() override { }

    void Aggro(Unit* /*who*/) override
    {
        // For an small probability he say something when it aggros
        switch (urand(0, 9))
        {
            case 0: DoScriptText(SAY_OOX_AGGRO1, m_creature); break;
            case 1: DoScriptText(SAY_OOX_AGGRO2, m_creature); break;
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        summoned->AI()->AttackStart(m_creature);
    }
};

CreatureAI* GetAI_npc_oox17tn(Creature* pCreature)
{
    return new npc_oox17tnAI(pCreature);
}

bool QuestAccept_npc_oox17tn(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_RESCUE_OOX_17TN)
    {
        DoScriptText(SAY_OOX_START, pCreature);

        pCreature->SetStandState(UNIT_STAND_STATE_STAND);

        if (pPlayer->GetTeam() == ALLIANCE)
            pCreature->SetFactionTemporary(FACTION_ESCORT_A_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);

        if (pPlayer->GetTeam() == HORDE)
            pCreature->SetFactionTemporary(FACTION_ESCORT_H_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);

        if (npc_oox17tnAI* pEscortAI = dynamic_cast<npc_oox17tnAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer, pQuest);
    }
    return true;
}

/*######
## npc_stone_watcher_of_norgannon
######*/

#define GOSSIP_ITEM_NORGANNON_1     "What function do you serve?"
#define GOSSIP_ITEM_NORGANNON_2     "What are the Plates of Uldum?"
#define GOSSIP_ITEM_NORGANNON_3     "Where are the Plates of Uldum?"
#define GOSSIP_ITEM_NORGANNON_4     "Excuse me? We've been \"reschedueled for visitations\"? What does that mean?!"
#define GOSSIP_ITEM_NORGANNON_5     "So, what's inside Uldum?"
#define GOSSIP_ITEM_NORGANNON_6     "I will return when i have the Plates of Uldum."

bool GossipHello_npc_stone_watcher_of_norgannon(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->GetQuestStatus(2954) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NORGANNON_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    pPlayer->SEND_GOSSIP_MENU(1674, pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_stone_watcher_of_norgannon(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NORGANNON_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            pPlayer->SEND_GOSSIP_MENU(1675, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NORGANNON_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(1676, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NORGANNON_4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            pPlayer->SEND_GOSSIP_MENU(1677, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NORGANNON_5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            pPlayer->SEND_GOSSIP_MENU(1678, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_NORGANNON_6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
            pPlayer->SEND_GOSSIP_MENU(1679, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->AreaExploredOrEventHappens(2954);
            break;
    }
    return true;
}

/*####
# npc_tooga
####*/

enum
{
    SAY_TOOG_THIRST             = -1000391,
    SAY_TOOG_WORRIED            = -1000392,
    SAY_TOOG_POST_1             = -1000393,
    SAY_TORT_POST_2             = -1000394,
    SAY_TOOG_POST_3             = -1000395,
    SAY_TORT_POST_4             = -1000396,
    SAY_TOOG_POST_5             = -1000397,
    SAY_TORT_POST_6             = -1000398,

    QUEST_TOOGA                 = 1560,
    NPC_TORTA                   = 6015,

    POINT_ID_TO_WATER           = 1
};

const float m_afToWaterLoc[] = { -7032.664551f, -4906.199219f, -1.606446f};

struct npc_toogaAI : public FollowerAI
{
    npc_toogaAI(Creature* pCreature) : FollowerAI(pCreature) { Reset(); }

    uint32 m_uiCheckSpeechTimer;
    uint32 m_uiPostEventTimer;
    uint32 m_uiPhasePostEvent;

    Unit* pTorta;

    void Reset() override
    {
        m_uiCheckSpeechTimer = 2500;
        m_uiPostEventTimer = 1000;
        m_uiPhasePostEvent = 0;

        pTorta = nullptr;
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        FollowerAI::MoveInLineOfSight(pWho);

        if (!m_creature->getVictim() && !HasFollowState(STATE_FOLLOW_COMPLETE | STATE_FOLLOW_POSTEVENT) && pWho->GetEntry() == NPC_TORTA)
        {
            if (m_creature->IsWithinDistInMap(pWho, INTERACTION_DISTANCE))
            {
                if (Player* pPlayer = GetLeaderForFollower())
                {
                    if (pPlayer->GetQuestStatus(QUEST_TOOGA) == QUEST_STATUS_INCOMPLETE)
                        pPlayer->GroupEventHappens(QUEST_TOOGA, m_creature);
                }

                pTorta = pWho;
                SetFollowComplete(true);
            }
        }
    }

    void MovementInform(uint32 uiMotionType, uint32 uiPointId) override
    {
        FollowerAI::MovementInform(uiMotionType, uiPointId);

        if (uiMotionType != POINT_MOTION_TYPE)
            return;

        if (uiPointId == POINT_ID_TO_WATER)
            SetFollowComplete();
    }

    void UpdateFollowerAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        {
            // we are doing the post-event, or...
            if (HasFollowState(STATE_FOLLOW_POSTEVENT))
            {
                if (m_uiPostEventTimer < uiDiff)
                {
                    m_uiPostEventTimer = 5000;

                    if (!pTorta || !pTorta->isAlive())
                    {
                        // something happened, so just complete
                        SetFollowComplete();
                        return;
                    }

                    switch (m_uiPhasePostEvent)
                    {
                        case 1:
                            DoScriptText(SAY_TOOG_POST_1, m_creature);
                            break;
                        case 2:
                            DoScriptText(SAY_TORT_POST_2, pTorta);
                            break;
                        case 3:
                            DoScriptText(SAY_TOOG_POST_3, m_creature);
                            break;
                        case 4:
                            DoScriptText(SAY_TORT_POST_4, pTorta);
                            break;
                        case 5:
                            DoScriptText(SAY_TOOG_POST_5, m_creature);
                            break;
                        case 6:
                            DoScriptText(SAY_TORT_POST_6, pTorta);
                            m_creature->GetMotionMaster()->MovePoint(POINT_ID_TO_WATER, m_afToWaterLoc[0], m_afToWaterLoc[1], m_afToWaterLoc[2]);
                            break;
                    }

                    ++m_uiPhasePostEvent;
                }
                else
                    m_uiPostEventTimer -= uiDiff;
            }
            //...we are doing regular speech check
            else if (HasFollowState(STATE_FOLLOW_INPROGRESS))
            {
                if (m_uiCheckSpeechTimer < uiDiff)
                {
                    m_uiCheckSpeechTimer = 5000;

                    switch (urand(0, 50))
                    {
                        case 10: DoScriptText(SAY_TOOG_THIRST, m_creature); break;
                        case 25: DoScriptText(SAY_TOOG_WORRIED, m_creature); break;
                    }
                }
                else
                    m_uiCheckSpeechTimer -= uiDiff;
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_tooga(Creature* pCreature)
{
    return new npc_toogaAI(pCreature);
}

bool QuestAccept_npc_tooga(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_TOOGA)
    {
        if (npc_toogaAI* pToogaAI = dynamic_cast<npc_toogaAI*>(pCreature->AI()))
            pToogaAI->StartFollow(pPlayer, FACTION_ESCORT_N_FRIEND_PASSIVE, pQuest);
    }

    return true;
}

void AddSC_tanaris()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "mob_aquementas";
    pNewScript->GetAI = &GetAI_mob_aquementas;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_oox17tn";
    pNewScript->GetAI = &GetAI_npc_oox17tn;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_oox17tn;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_stone_watcher_of_norgannon";
    pNewScript->pGossipHello =  &GossipHello_npc_stone_watcher_of_norgannon;
    pNewScript->pGossipSelect = &GossipSelect_npc_stone_watcher_of_norgannon;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_tooga";
    pNewScript->GetAI = &GetAI_npc_tooga;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_tooga;
    pNewScript->RegisterSelf();
}
