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
SDName: Boss_Rajaxx
SD%Complete: 100
SDComment: General Andorov script
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "ruins_of_ahnqiraj.h"

enum
{
    // Event yells
    SAY_ANDOROV_INTRO_1         = -1509004,
    SAY_ANDOROV_INTRO_2         = -1509031,
    SAY_ANDOROV_INTRO_3         = -1509003,
    SAY_ANDOROV_INTRO_4         = -1509029,
    SAY_ANDOROV_ATTACK_START    = -1509030,

    // Rajaxx kills Andorov
    SAY_KILLS_ANDOROV           = -1509016,

    // probably related to the opening of AQ event
    SAY_UNK1                    = -1509011,
    SAY_UNK2                    = -1509012,
    SAY_UNK3                    = -1509013,
    SAY_UNK4                    = -1509014,

    // gossip items
    GOSSIP_TEXT_ID_INTRO        = 7883,
    GOSSIP_TEXT_ID_TRADE        = 8305,

    GOSSIP_ITEM_START           = -3509000,
    GOSSIP_ITEM_TRADE           = -3509001,

    // Andorov spells
    SPELL_AURA_OF_COMMAND       = 25516,
    SPELL_BASH                  = 25515,
    SPELL_STRIKE                = 22591,

    // Kaldorei spell
    SPELL_CLEAVE                = 26350,
    SPELL_MORTAL_STRIKE         = 16856,

    POINT_ID_MOVE_INTRO         = 2,
    POINT_ID_MOVE_ATTACK        = 4,
};

static const DialogueEntry aIntroDialogue[] =
{
    {SAY_ANDOROV_INTRO_1,       NPC_GENERAL_ANDOROV,    7000},
    {SAY_ANDOROV_INTRO_2,       NPC_GENERAL_ANDOROV,    0},
    {SAY_ANDOROV_INTRO_3,       NPC_GENERAL_ANDOROV,    4000},
    {SAY_ANDOROV_INTRO_4,       NPC_GENERAL_ANDOROV,    6000},
    {SAY_ANDOROV_ATTACK_START,  NPC_GENERAL_ANDOROV,    0},
    {0, 0, 0},
};

struct npc_general_andorovAI : public ScriptedAI, private DialogueHelper
{
    npc_general_andorovAI(Creature* pCreature) : ScriptedAI(pCreature),
        DialogueHelper(aIntroDialogue)
    {
        m_pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData();
        InitializeDialogueHelper(m_pInstance);
        m_uiMoveTimer = 5000;
        m_uiPointId = 0;
        Reset();
    }

    instance_ruins_of_ahnqiraj* m_pInstance;

    uint32 m_uiCommandAuraTimer;
    uint32 m_uiBashTimer;
    uint32 m_uiStrikeTimer;
    uint32 m_uiMoveTimer;

    uint8 m_uiPointId;

    void Reset() override
    {
        m_uiCommandAuraTimer = urand(1000, 3000);
        m_uiBashTimer        = urand(8000, 11000);
        m_uiStrikeTimer      = urand(2000, 5000);
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        // If Rajaxx is in range attack him
        if (pWho->GetEntry() == NPC_RAJAXX && m_creature->IsWithinDistInMap(pWho, 50.0f))
            AttackStart(pWho);

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void JustDied(Unit* pKiller) override
    {
        if (pKiller->GetEntry() != NPC_RAJAXX)
            return;

        // Yell when killed by Rajaxx
        if (m_pInstance)
        {
            if (Creature* pRajaxx = m_pInstance->GetSingleCreatureFromStorage(NPC_RAJAXX))
                DoScriptText(SAY_KILLS_ANDOROV, pRajaxx);
        }
    }

    void JustDidDialogueStep(int32 iEntry) override
    {
        // Start the event when the dialogue is finished
        if (iEntry == SAY_ANDOROV_ATTACK_START)
        {
            if (m_pInstance)
                m_pInstance->SetData(TYPE_RAJAXX, IN_PROGRESS);
        }
    }

    void MovementInform(uint32 uiType, uint32 uiPointId) override
    {
        if (uiType != POINT_MOTION_TYPE)
            return;

        switch (uiPointId)
        {
            case 0:
            case 1:
            case 3:
                ++m_uiPointId;
                m_creature->GetMotionMaster()->MovePoint(m_uiPointId, aAndorovMoveLocs[m_uiPointId].m_fX, aAndorovMoveLocs[m_uiPointId].m_fY, aAndorovMoveLocs[m_uiPointId].m_fZ);
                break;
            case POINT_ID_MOVE_INTRO:
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                m_creature->SetFacingTo(aAndorovMoveLocs[3].m_fO);
                ++m_uiPointId;
                break;
            case POINT_ID_MOVE_ATTACK:
                // Start dialogue only the first time it reaches the point
                if (m_uiPointId == 4)
                {
                    StartNextDialogueText(SAY_ANDOROV_INTRO_3);
                    ++m_uiPointId;
                }
                break;
        }
    }

    void EnterEvadeMode() override
    {
        if (!m_pInstance)
            return;

        m_creature->RemoveAllAurasOnEvade();
        m_creature->DeleteThreatList();
        m_creature->CombatStop(true);

        if (m_creature->isAlive())
        {
            // reset to combat position
            if (m_uiPointId >= 4)
                m_creature->GetMotionMaster()->MovePoint(POINT_ID_MOVE_ATTACK, aAndorovMoveLocs[4].m_fX, aAndorovMoveLocs[4].m_fY, aAndorovMoveLocs[4].m_fZ);
            // reset to intro position
            else
                m_creature->GetMotionMaster()->MovePoint(POINT_ID_MOVE_INTRO, aAndorovMoveLocs[2].m_fX, aAndorovMoveLocs[2].m_fY, aAndorovMoveLocs[2].m_fZ);
        }

        m_creature->SetLootRecipient(NULL);

        Reset();
    }

    // Wrapper to start initialize Kaldorei followers
    void DoInitializeFollowers()
    {
        if (!m_pInstance)
            return;

        GuidList m_lKaldoreiGuids;
        m_pInstance->GetKaldoreiGuidList(m_lKaldoreiGuids);

        for (GuidList::const_iterator itr = m_lKaldoreiGuids.begin(); itr != m_lKaldoreiGuids.end(); ++itr)
        {
            if (Creature* pKaldorei = m_creature->GetMap()->GetCreature(*itr))
                pKaldorei->GetMotionMaster()->MoveFollow(m_creature, pKaldorei->GetDistance(m_creature), pKaldorei->GetAngle(m_creature));
        }
    }

    // Wrapper to start the event
    void DoMoveToEventLocation()
    {
        m_creature->GetMotionMaster()->MovePoint(m_uiPointId, aAndorovMoveLocs[m_uiPointId].m_fX, aAndorovMoveLocs[m_uiPointId].m_fY, aAndorovMoveLocs[m_uiPointId].m_fZ);
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        StartNextDialogueText(SAY_ANDOROV_INTRO_1);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        DialogueUpdate(uiDiff);

        if (m_uiMoveTimer)
        {
            if (m_uiMoveTimer <= uiDiff)
            {
                m_creature->SetWalk(false);
                m_creature->GetMotionMaster()->MovePoint(m_uiPointId, aAndorovMoveLocs[m_uiPointId].m_fX, aAndorovMoveLocs[m_uiPointId].m_fY, aAndorovMoveLocs[m_uiPointId].m_fZ);

                DoInitializeFollowers();
                m_uiMoveTimer = 0;
            }
            else
                m_uiMoveTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiBashTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_BASH) == CAST_OK)
                m_uiBashTimer = urand(12000, 15000);
        }
        else
            m_uiBashTimer -= uiDiff;

        if (m_uiStrikeTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_STRIKE) == CAST_OK)
                m_uiStrikeTimer = urand(4000, 6000);
        }
        else
            m_uiStrikeTimer -= uiDiff;

        if (m_uiCommandAuraTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_AURA_OF_COMMAND) == CAST_OK)
                m_uiCommandAuraTimer = urand(30000, 45000);
        }
        else
            m_uiCommandAuraTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_general_andorov(Creature* pCreature)
{
    return new npc_general_andorovAI(pCreature);
}

bool GossipHello_npc_general_andorov(Player* pPlayer, Creature* pCreature)
{
    if (instance_ruins_of_ahnqiraj* pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData())
    {
        if (pInstance->GetData(TYPE_RAJAXX) == IN_PROGRESS)
            return true;

        if (pInstance->GetData(TYPE_RAJAXX) == NOT_STARTED || pInstance->GetData(TYPE_RAJAXX) == FAIL)
            pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_START, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_VENDOR, GOSSIP_ITEM_TRADE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID_INTRO, pCreature->GetObjectGuid());
    }

    return true;
}

bool GossipSelect_npc_general_andorov(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        if (npc_general_andorovAI* pAndorovAI = dynamic_cast<npc_general_andorovAI*>(pCreature->AI()))
            pAndorovAI->DoMoveToEventLocation();

        pPlayer->CLOSE_GOSSIP_MENU();
    }

    if (uiAction == GOSSIP_ACTION_TRADE)
        pPlayer->SEND_VENDORLIST(pCreature->GetObjectGuid());

    return true;
}

struct npc_kaldorei_eliteAI : public ScriptedAI
{
    npc_kaldorei_eliteAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiCleaveTimer;
    uint32 m_uiStrikeTimer;

    void Reset() override
    {
        m_uiCleaveTimer      = urand(2000, 4000);
        m_uiStrikeTimer      = urand(8000, 11000);
    }

    void EnterEvadeMode() override
    {
        if (!m_pInstance)
            return;

        m_creature->RemoveAllAurasOnEvade();
        m_creature->DeleteThreatList();
        m_creature->CombatStop(true);

        // reset only to the last position
        if (m_creature->isAlive())
        {
            if (Creature* pAndorov = m_pInstance->GetSingleCreatureFromStorage(NPC_GENERAL_ANDOROV))
            {
                if (pAndorov->isAlive())
                    m_creature->GetMotionMaster()->MoveFollow(pAndorov, m_creature->GetDistance(pAndorov), m_creature->GetAngle(pAndorov));
            }
        }

        m_creature->SetLootRecipient(NULL);

        Reset();
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiCleaveTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                m_uiCleaveTimer = urand(5000, 7000);
        }
        else
            m_uiCleaveTimer -= uiDiff;

        if (m_uiStrikeTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_MORTAL_STRIKE) == CAST_OK)
                m_uiStrikeTimer = urand(9000, 13000);
        }
        else
            m_uiStrikeTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_kaldorei_elite(Creature* pCreature)
{
    return new npc_kaldorei_eliteAI(pCreature);
}

void AddSC_boss_rajaxx()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_general_andorov";
    pNewScript->GetAI = &GetAI_npc_general_andorov;
    pNewScript->pGossipHello = &GossipHello_npc_general_andorov;
    pNewScript->pGossipSelect = &GossipSelect_npc_general_andorov;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_kaldorei_elite";
    pNewScript->GetAI = &GetAI_npc_kaldorei_elite;
    pNewScript->RegisterSelf();
}
