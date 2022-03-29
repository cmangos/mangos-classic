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
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "ruins_of_ahnqiraj.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    // Event yells
    SAY_ANDOROV_INTRO_1         = -1509004,
    SAY_ANDOROV_INTRO_2         = -1509031,
    SAY_ANDOROV_INTRO_3         = -1509003,
    SAY_ANDOROV_INTRO_4         = -1509029,
    SAY_ANDOROV_ATTACK_START    = -1509030,
    SAY_ANDOROV_DESPAWN         = -1509032,

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

enum AndorovActions
{
    ANDOROV_COMMAND_AURA,
    ANDOROV_BASH,
    ANDOROV_STRIKE,
    ANDOROV_ACTION_MAX,
    ANDOROV_MOVE,
    ANDOROV_DESPAWN,
};

struct npc_general_andorovAI : public CombatAI, private DialogueHelper
{
    npc_general_andorovAI(Creature* creature) : CombatAI(creature, ANDOROV_ACTION_MAX), m_instance(static_cast<instance_ruins_of_ahnqiraj*>(creature->GetInstanceData())),
        DialogueHelper(aIntroDialogue)
    {
        InitializeDialogueHelper(m_instance);
        m_pointId = 0;
        AddCombatAction(ANDOROV_COMMAND_AURA, 1000, 3000);
        AddCombatAction(ANDOROV_BASH, 8000, 11000);
        AddCombatAction(ANDOROV_STRIKE, 2000, 5000);
        AddCustomAction(ANDOROV_MOVE, true, [&]() { HandleMove(); });
        AddCustomAction(ANDOROV_DESPAWN, true, [&]() { HandleDespawn(); });
    }

    instance_ruins_of_ahnqiraj* m_instance;

    uint8 m_pointId;

    void JustRespawned() override
    {
        ResetTimer(ANDOROV_MOVE, 5000);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        // If Rajaxx is in range attack him
        if (who->GetEntry() == NPC_RAJAXX && m_creature->IsWithinDistInMap(who, 50.0f))
            AttackStart(who);

        ScriptedAI::MoveInLineOfSight(who);
    }

    void JustDied(Unit* killer) override
    {
        if (killer->GetEntry() != NPC_RAJAXX)
            return;

        // Yell when killed by Rajaxx
        if (m_instance)
        {
            if (Creature* pRajaxx = m_instance->GetSingleCreatureFromStorage(NPC_RAJAXX))
                DoScriptText(SAY_KILLS_ANDOROV, pRajaxx);
        }
    }

    void JustDidDialogueStep(int32 entry) override
    {
        // Start the event when the dialogue is finished
        if (entry == SAY_ANDOROV_ATTACK_START)
        {
            if (m_instance)
                m_instance->SetData(TYPE_RAJAXX, IN_PROGRESS);
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
                ++m_pointId;
                m_creature->GetMotionMaster()->MovePoint(m_pointId, aAndorovMoveLocs[m_pointId].m_fX, aAndorovMoveLocs[m_pointId].m_fY, aAndorovMoveLocs[m_pointId].m_fZ);
                break;
            case POINT_ID_MOVE_INTRO:
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                m_creature->SetFacingTo(aAndorovMoveLocs[3].m_fO);
                ++m_pointId;
                break;
            case POINT_ID_MOVE_ATTACK:
                // Start dialogue only the first time it reaches the point
                if (m_pointId == 4)
                {
                    StartNextDialogueText(SAY_ANDOROV_INTRO_3);
                    ++m_pointId;
                }
                break;
        }
    }

    void EnterEvadeMode() override
    {
        if (!m_instance)
            return;

        m_creature->RemoveAllAurasOnEvade();
        m_creature->CombatStop(true);

        if (m_creature->IsAlive())
        {
            // reset to combat position
            if (m_pointId >= 4)
                m_creature->GetMotionMaster()->MovePoint(POINT_ID_MOVE_ATTACK, aAndorovMoveLocs[4].m_fX, aAndorovMoveLocs[4].m_fY, aAndorovMoveLocs[4].m_fZ);
            // reset to intro position
            else
                m_creature->GetMotionMaster()->MovePoint(POINT_ID_MOVE_INTRO, aAndorovMoveLocs[2].m_fX, aAndorovMoveLocs[2].m_fY, aAndorovMoveLocs[2].m_fZ);
        }

        m_creature->SetLootRecipient(nullptr);

        Reset();
    }

    // Wrapper to start initialize Kaldorei followers
    void DoInitializeFollowers()
    {
        if (!m_instance)
            return;

        GuidList m_lKaldoreiGuids;
        m_instance->GetKaldoreiGuidList(m_lKaldoreiGuids);

        for (GuidList::const_iterator itr = m_lKaldoreiGuids.begin(); itr != m_lKaldoreiGuids.end(); ++itr)
            if (Creature* kaldorei = m_creature->GetMap()->GetCreature(*itr))
                kaldorei->GetMotionMaster()->MoveFollow(m_creature, kaldorei->GetDistance(m_creature), kaldorei->GetAngle(m_creature));
    }

    // Wrapper to start the event
    void DoMoveToEventLocation()
    {
        m_creature->GetMotionMaster()->MovePoint(m_pointId, aAndorovMoveLocs[m_pointId].m_fX, aAndorovMoveLocs[m_pointId].m_fY, aAndorovMoveLocs[m_pointId].m_fZ);
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        GuidList m_lKaldoreiGuids;
        m_instance->GetKaldoreiGuidList(m_lKaldoreiGuids);
        for (GuidList::const_iterator itr = m_lKaldoreiGuids.begin(); itr != m_lKaldoreiGuids.end(); ++itr)
            if (Creature* kaldorei = m_creature->GetMap()->GetCreature(*itr))
                kaldorei->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        StartNextDialogueText(SAY_ANDOROV_INTRO_1);
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/) override
    {
        if (eventType == AI_EVENT_CUSTOM_A)
        {
            m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_VENDOR);
            ResetTimer(ANDOROV_DESPAWN, 135000u);
        }
    }

    void HandleDespawn()
    {
        DoScriptText(SAY_ANDOROV_DESPAWN, m_creature);
        m_creature->ForcedDespawn(2500);
    }

    void HandleMove()
    {
        m_creature->SetWalk(false);
        m_creature->GetMotionMaster()->MovePoint(m_pointId, aAndorovMoveLocs[m_pointId].m_fX, aAndorovMoveLocs[m_pointId].m_fY, aAndorovMoveLocs[m_pointId].m_fZ);

        DoInitializeFollowers();
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case ANDOROV_COMMAND_AURA:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_AURA_OF_COMMAND) == CAST_OK)
                    ResetCombatAction(action, urand(30000, 45000));
                break;
            }
            case ANDOROV_BASH:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_BASH) == CAST_OK)
                    ResetCombatAction(action, urand(12000, 15000));
                break;
            }
            case ANDOROV_STRIKE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_STRIKE) == CAST_OK)
                    ResetCombatAction(action, urand(4000, 6000));
                break;
            }
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        DialogueUpdate(diff);
        CombatAI::UpdateAI(diff);
    }
};

bool GossipHello_npc_general_andorov(Player* player, Creature* creature)
{
    if (instance_ruins_of_ahnqiraj* instance = static_cast<instance_ruins_of_ahnqiraj*>(creature->GetInstanceData()))
    {
        if (instance->GetData(TYPE_RAJAXX) == IN_PROGRESS)
            return true;

        if (instance->GetData(TYPE_RAJAXX) == NOT_STARTED || instance->GetData(TYPE_RAJAXX) == FAIL)
            player->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_START, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        if (creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR))
            player->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_VENDOR, GOSSIP_ITEM_TRADE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

        player->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID_INTRO, creature->GetObjectGuid());
    }

    return true;
}

bool GossipSelect_npc_general_andorov(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        if (npc_general_andorovAI* andorovAI = dynamic_cast<npc_general_andorovAI*>(creature->AI()))
            andorovAI->DoMoveToEventLocation();

        player->CLOSE_GOSSIP_MENU();
    }

    if (action == GOSSIP_ACTION_TRADE)
        player->SEND_VENDORLIST(creature->GetObjectGuid());

    return true;
}

enum KaldoreiEliteActions
{
    KALDOREI_CLEAVE,
    KALDOREI_STRIKE,
    KALDOREI_ACTION_MAX,
};

struct npc_kaldorei_eliteAI : public CombatAI
{
    npc_kaldorei_eliteAI(Creature* creature) : CombatAI(creature, KALDOREI_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(KALDOREI_CLEAVE, 2000, 4000);
        AddCombatAction(KALDOREI_STRIKE, 8000, 11000);
    }

    ScriptedInstance* m_instance;

    void EnterEvadeMode() override
    {
        if (!m_instance)
            return;

        m_creature->RemoveAllAurasOnEvade();
        m_creature->CombatStop(true);

        // reset only to the last position
        if (m_creature->IsAlive())
        {
            if (Creature* andorov = m_instance->GetSingleCreatureFromStorage(NPC_GENERAL_ANDOROV))
            {
                if (andorov->IsAlive())
                    m_creature->GetMotionMaster()->MoveFollow(andorov, m_creature->GetDistance(andorov), m_creature->GetAngle(andorov));
            }
        }

        m_creature->SetLootRecipient(nullptr);

        Reset();
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case KALDOREI_CLEAVE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_CLEAVE) == CAST_OK)
                    ResetCombatAction(action, urand(5000, 7000));
                break;
            }
            case KALDOREI_STRIKE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_MORTAL_STRIKE) == CAST_OK)
                    ResetCombatAction(action, urand(9000, 13000));
                break;
            }
        }
    }
};

void AddSC_boss_rajaxx()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_general_andorov";
    pNewScript->GetAI = &GetNewAIInstance<npc_general_andorovAI>;
    pNewScript->pGossipHello = &GossipHello_npc_general_andorov;
    pNewScript->pGossipSelect = &GossipSelect_npc_general_andorov;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_kaldorei_elite";
    pNewScript->GetAI = &GetNewAIInstance<npc_kaldorei_eliteAI>;
    pNewScript->RegisterSelf();
}
