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
SDName: Westfall
SD%Complete: 90
SDComment: Quest support: 155, 1651
SDCategory: Westfall
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"/* ContentData
npc_daphne_stilwell
npc_defias_traitor
EndContentData */


#include "AI/ScriptDevAI/base/escort_ai.h"

/*######
## npc_daphne_stilwell
######*/

enum
{
    SAY_DS_START        = -1000293,
    SAY_DS_DOWN_1       = -1000294,
    SAY_DS_DOWN_2       = -1000295,
    SAY_DS_DOWN_3       = -1000296,
    SAY_DS_PROLOGUE     = -1000297,

    SPELL_SHOOT         = 6660,
    QUEST_TOME_VALOR    = 1651,
    NPC_DEFIAS_RAIDER   = 6180,
    EQUIP_ID_RIFLE      = 2511,

    DAPHNE_SHOOT_CD     = 2000
};

enum Wave
{
    FIRST,
    SECOND,
    THIRD
};

float RaiderCoords[15][3] =
{
    {-11428.520f, 1612.757f, 72.241f}, // Spawn1
    {-11422.998f, 1616.106f, 74.153f}, // Spawn2
    {-11430.354f, 1618.334f, 72.632f}, // Spawn3
    {-11423.307f, 1621.033f, 74.224f}, // Spawn4
    {-11427.141f, 1623.220f, 73.168f}, // Spawn5

    {-11453.118f, 1554.380f, 53.100f}, // WP1a
    {-11449.692f, 1554.672f, 53.598f}, // WP2a
    {-11454.533f, 1558.679f, 52.497f}, // WP3a
    {-11449.488f, 1557.817f, 53.443f}, // WP4a
    {-11452.123f, 1559.800f, 52.890f}, // WP5a

    {-11475.067f, 1534.259f, 50.199f}, // WP1b
    {-11470.306f, 1533.835f, 50.267f}, // WP2b
    {-11471.954f, 1539.599f, 50.273f}, // WP3b
    {-11465.560f, 1534.399f, 50.649f}, // WP4b
    {-11467.391f, 1537.989f, 50.726f}  // WP5b
};

struct npc_daphne_stilwellAI : public npc_escortAI
{
    npc_daphne_stilwellAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_uiWPHolder = 0;
        Reset();
    }

    uint32 m_uiWPHolder;
    uint32 m_uiShootTimer;

    GuidList m_lSummonedRaidersGUIDs;

    void Reset() override
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
        {
            if (m_uiWPHolder >= 5)
                m_creature->SetSheath(SHEATH_STATE_RANGED);

            switch (m_uiWPHolder)
            {
                case 7: DoScriptText(SAY_DS_DOWN_1, m_creature); break;
                case 8: DoScriptText(SAY_DS_DOWN_2, m_creature); break;
                case 9:
                    if (m_lSummonedRaidersGUIDs.empty())
                        DoScriptText(SAY_DS_DOWN_3, m_creature);
                    break;
            }
        }
        else
            m_uiWPHolder = 0;

        m_uiShootTimer = 0;
    }

    void WaypointReached(uint32 uiPointId) override
    {
        m_uiWPHolder = uiPointId;

        switch (uiPointId)
        {
            case 4:
                m_creature->HandleEmote(EMOTE_STATE_USESTANDING_NOSHEATHE);
                break;
            case 5:
                SetEquipmentSlots(false, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE, EQUIP_ID_RIFLE);
                m_creature->SetSheath(SHEATH_STATE_RANGED);
                m_creature->HandleEmote(EMOTE_STATE_STAND);
                break;
            case 7:
                DoSendWave(Wave::FIRST);
                break;
            case 8:
                DoSendWave(Wave::SECOND);
                break;
            case 9:
                DoSendWave(Wave::THIRD);
                SetEscortPaused(true);
                break;
            case 10:
                SetRun(false);
                break;
            case 11:
                DoScriptText(SAY_DS_PROLOGUE, m_creature);
                break;
            case 13:
                SetEquipmentSlots(true);
                m_creature->SetSheath(SHEATH_STATE_UNARMED);
                m_creature->HandleEmote(EMOTE_STATE_USESTANDING);
                break;
            case 14:
                m_creature->HandleEmote(EMOTE_STATE_STAND);
                break;
            case 17:
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_TOME_VALOR, m_creature);
                break;
            case 18:
                DoEndEscort();
                break;
        }
    }

    void DoSendWave(uint32 wave)
    {
        uint8 uiFirstWPOffset = 5;

        switch (wave)
        {
            case Wave::FIRST:
                for (int counter = 0; counter < 3; counter++)
                    if (Creature* pAdd = m_creature->SummonCreature(NPC_DEFIAS_RAIDER, RaiderCoords[counter][0], RaiderCoords[counter][1], RaiderCoords[counter][2], 0, TEMPSPAWN_TIMED_OOC_DESPAWN, 30000, false, true))
                        pAdd->GetMotionMaster()->MovePoint(counter, RaiderCoords[uiFirstWPOffset + counter][0], RaiderCoords[uiFirstWPOffset + counter][1], RaiderCoords[uiFirstWPOffset + counter][2]);
                break;
            case Wave::SECOND:
                for (int counter = 0; counter < 4; counter++)
                    if (Creature* pAdd = m_creature->SummonCreature(NPC_DEFIAS_RAIDER, RaiderCoords[counter][0], RaiderCoords[counter][1], RaiderCoords[counter][2], 0, TEMPSPAWN_TIMED_OOC_DESPAWN, 30000, false, true))
                        pAdd->GetMotionMaster()->MovePoint(counter, RaiderCoords[uiFirstWPOffset + counter][0], RaiderCoords[uiFirstWPOffset + counter][1], RaiderCoords[uiFirstWPOffset + counter][2]);
                break;
            case Wave::THIRD:
                for (int counter = 0; counter < 5; counter++)
                    if (Creature* pAdd = m_creature->SummonCreature(NPC_DEFIAS_RAIDER, RaiderCoords[counter][0], RaiderCoords[counter][1], RaiderCoords[counter][2], 0, TEMPSPAWN_TIMED_OOC_DESPAWN, 30000, false, true))
                        pAdd->GetMotionMaster()->MovePoint(counter, RaiderCoords[uiFirstWPOffset + counter][0], RaiderCoords[uiFirstWPOffset + counter][1], RaiderCoords[uiFirstWPOffset + counter][2]);
                break;
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiMotionType, uint32 uiData) override
    {
        uint8 uiSecondWPOffset = 10;

        if (pSummoned->GetEntry() == NPC_DEFIAS_RAIDER && uiMotionType == POINT_MOTION_TYPE) // sanity check
        {
            switch (uiData)
            {
                case 0:
                    pSummoned->GetMotionMaster()->MovePoint(5, RaiderCoords[uiSecondWPOffset][0], RaiderCoords[uiSecondWPOffset][1], RaiderCoords[uiSecondWPOffset][2]);
                    break;
                case 1:
                    pSummoned->GetMotionMaster()->MovePoint(5, RaiderCoords[uiSecondWPOffset + 1][0], RaiderCoords[uiSecondWPOffset + 1][1], RaiderCoords[uiSecondWPOffset + 1][2]);
                    break;
                case 2:
                    pSummoned->GetMotionMaster()->MovePoint(5, RaiderCoords[uiSecondWPOffset + 2][0], RaiderCoords[uiSecondWPOffset + 2][1], RaiderCoords[uiSecondWPOffset + 2][2]);
                    break;
                case 3:
                    pSummoned->GetMotionMaster()->MovePoint(5, RaiderCoords[uiSecondWPOffset + 3][0], RaiderCoords[uiSecondWPOffset + 3][1], RaiderCoords[uiSecondWPOffset + 3][2]);
                    break;
                case 4:
                    pSummoned->GetMotionMaster()->MovePoint(5, RaiderCoords[uiSecondWPOffset + 4][0], RaiderCoords[uiSecondWPOffset + 4][1], RaiderCoords[uiSecondWPOffset + 4][2]);
                    break;
                default:
                    pSummoned->GetMotionMaster()->MoveIdle();
                    break;
            }
        }
    }

    void Aggro(Unit* /*who*/) override
    {
        SetCombatMovement(false);
    }

    void JustReachedHome() override
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING) && m_uiWPHolder >= 5)
            m_creature->SetSheath(SHEATH_STATE_RANGED);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_DEFIAS_RAIDER)
        {
            m_lSummonedRaidersGUIDs.push_back(pSummoned->GetObjectGuid());
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        m_lSummonedRaidersGUIDs.remove(pSummoned->GetObjectGuid());

        if (m_uiWPHolder >= 9 && m_lSummonedRaidersGUIDs.empty())
            SetEscortPaused(false);
    }

    void SummonedCreatureDespawn(Creature* pSummoned) override // just in case this happens somehow
    {
        if (pSummoned->isAlive())
            m_lSummonedRaidersGUIDs.remove(pSummoned->GetObjectGuid());
    }

    void JustDied(Unit* pKiller) override
    {
        m_lSummonedRaidersGUIDs.clear();

        npc_escortAI::JustDied(pKiller);
    }

    void DoEndEscort()
    {
        m_creature->ForcedDespawn();
        m_creature->Respawn();
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiShootTimer < uiDiff)
        {
            m_uiShootTimer = DAPHNE_SHOOT_CD;

            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHOOT) != CanCastResult::CAST_OK && !IsCombatMovement())
            {
                SetCombatMovement(true);
                DoStartMovement(m_creature->getVictim());
            }
        }
        else
            m_uiShootTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_daphne_stilwell(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_TOME_VALOR)
    {
        DoScriptText(SAY_DS_START, pCreature);

        if (npc_daphne_stilwellAI* pEscortAI = dynamic_cast<npc_daphne_stilwellAI*>(pCreature->AI()))
            pEscortAI->Start(true, pPlayer, pQuest);
    }

    return true;
}

UnitAI* GetAI_npc_daphne_stilwell(Creature* pCreature)
{
    return new npc_daphne_stilwellAI(pCreature);
}

/*######
## npc_defias_traitor
######*/

enum
{
    SAY_START                = -1000101,
    SAY_PROGRESS             = -1000102,
    SAY_END                  = -1000103,
    SAY_AGGRO_1              = -1000104,
    SAY_AGGRO_2              = -1000105,

    QUEST_DEFIAS_BROTHERHOOD = 155
};

struct npc_defias_traitorAI : public npc_escortAI
{
    npc_defias_traitorAI(Creature* creature) : npc_escortAI(creature)
    {
        SetReactState(REACT_DEFENSIVE);
        Reset();
    }

    void WaypointReached(uint32 pointId) override
    {
        switch (pointId)
        {
            case 35:
                SetRun(false);
                break;
            case 36:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_PROGRESS, m_creature, pPlayer);
                break;
            case 44:
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    DoScriptText(SAY_END, m_creature, pPlayer);
                    pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_DEFIAS_BROTHERHOOD, m_creature);
                }
                break;
        }
    }

    void Aggro(Unit* who) override
    {
        DoScriptText(urand(0, 1) ? SAY_AGGRO_1 : SAY_AGGRO_2, m_creature, who);
    }

    void Reset() override { }
};

bool QuestAccept_npc_defias_traitor(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_DEFIAS_BROTHERHOOD)
    {
        DoScriptText(SAY_START, creature, player);

        if (npc_defias_traitorAI* escortAI = dynamic_cast<npc_defias_traitorAI*>(creature->AI()))
            escortAI->Start(true, player, quest);
    }

    return true;
}

UnitAI* GetAI_npc_defias_traitor(Creature* creature)
{
    return new npc_defias_traitorAI(creature);
}

void AddSC_westfall()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_daphne_stilwell";
    pNewScript->GetAI = &GetAI_npc_daphne_stilwell;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_daphne_stilwell;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_defias_traitor";
    pNewScript->GetAI = &GetAI_npc_defias_traitor;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_defias_traitor;
    pNewScript->RegisterSelf();
}
