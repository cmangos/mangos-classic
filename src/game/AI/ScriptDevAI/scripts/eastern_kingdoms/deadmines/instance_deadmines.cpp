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
SDName: Instance_Deadmines
SD%Complete: 0
SDComment: Placeholder
SDCategory: Deadmines
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "deadmines.h"

instance_deadmines::instance_deadmines(Map* pMap) : ScriptedInstance(pMap),
    m_uiIronDoorTimer(0),
    m_uiDoorStep(0),
    m_firstEnter(true)
{
    Initialize();
}

void instance_deadmines::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

void instance_deadmines::OnPlayerEnter(Player* pPlayer)
{
    // Respawn the Mysterious chest if one of the players who enter the instance has the quest in his log
    if (pPlayer->GetQuestStatus(QUEST_FORTUNE_AWAITS) == QUEST_STATUS_COMPLETE &&
            !pPlayer->GetQuestRewardStatus(QUEST_FORTUNE_AWAITS))
        DoRespawnGameObject(GO_MYSTERIOUS_CHEST, HOUR);

    if (m_firstEnter)
    {
        m_firstEnter = false;

        if (m_auiEncounter[TYPE_RHAHKZOR] == DONE)
            SpawnFirstDeadminesPatrol();

        if (m_auiEncounter[TYPE_SNEED] == DONE)
            SpawnSecondDeadminesPatrol();

        if (m_auiEncounter[TYPE_GILNID] == DONE)
            SpawnThirdDeadminesPatrol();
    }
}

void instance_deadmines::SpawnFirstDeadminesPatrol()
{
    WorldObject::SpawnCreature(GUID_PREFIX + 200, instance); // Defias Overseer
    WorldObject::SpawnCreature(GUID_PREFIX + 201, instance); // Defias Evoker
}

void instance_deadmines::SpawnSecondDeadminesPatrol()
{
    WorldObject::SpawnCreature(GUID_PREFIX + 202, instance); // Defias Evoker
    WorldObject::SpawnCreature(GUID_PREFIX + 203, instance); // Defias Overseer
}

void instance_deadmines::SpawnThirdDeadminesPatrol()
{
    WorldObject::SpawnCreature(GUID_PREFIX + 204, instance); // Defias Wizard
    WorldObject::SpawnCreature(GUID_PREFIX + 205, instance); // Defias Taskmaster
    WorldObject::SpawnCreature(GUID_PREFIX + 206, instance); // Defias Taskmaster
}

void instance_deadmines::OnCreatureCreate(Creature* pCreature)
{
    if (pCreature->GetEntry() == NPC_MR_SMITE)
        m_npcEntryGuidStore[NPC_MR_SMITE] = pCreature->GetObjectGuid();
}

void instance_deadmines::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_FACTORY_DOOR:
            if (m_auiEncounter[TYPE_RHAHKZOR] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);

            break;
        case GO_MAST_ROOM_DOOR:
            if (m_auiEncounter[TYPE_SNEED] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);

            break;
        case GO_FOUNDRY_DOOR:
            if (m_auiEncounter[TYPE_GILNID] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);

            break;
        case GO_IRON_CLAD_DOOR:
            if (m_auiEncounter[TYPE_IRON_CLAD_DOOR] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);

            break;
        case GO_DEFIAS_CANNON:
        case GO_SMITE_CHEST:
        case GO_MYSTERIOUS_CHEST:
            break;

        default:
            return;
    }

    m_goEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
}

void instance_deadmines::OnCreatureDeath(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_RHAHKZOR: SetData(TYPE_RHAHKZOR, DONE); break;
        case NPC_SNEED:    SetData(TYPE_SNEED, DONE);    break;
        case NPC_GILNID:   SetData(TYPE_GILNID, DONE);   break;
    }
}

void instance_deadmines::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_RHAHKZOR:
        {
            if (uiData == DONE)
                DoUseDoorOrButton(GO_FACTORY_DOOR);

            SpawnFirstDeadminesPatrol();
            m_auiEncounter[uiType] = uiData;
            break;
        }
        case TYPE_SNEED:
        {
            if (uiData == DONE)
                DoUseDoorOrButton(GO_MAST_ROOM_DOOR);

            SpawnSecondDeadminesPatrol();
            m_auiEncounter[uiType] = uiData;
            break;
        }
        case TYPE_GILNID:
        {
            if (uiData == DONE)
                DoUseDoorOrButton(GO_FOUNDRY_DOOR);

            SpawnThirdDeadminesPatrol();
            m_auiEncounter[uiType] = uiData;
            break;
        }
        case TYPE_IRON_CLAD_DOOR:
        {
            // delayed door animation to sync with Defias Cannon animation
            if (uiData == DONE)
                m_uiIronDoorTimer = 500;

            m_auiEncounter[uiType] = uiData;
            break;
        }
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " " << m_auiEncounter[3];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

uint32 instance_deadmines::GetData(uint32 uiType) const
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    return 0;
}

void instance_deadmines::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3];

    for (uint32& i : m_auiEncounter)
    {
        if (i == IN_PROGRESS)
            i = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

void instance_deadmines::Update(uint32 uiDiff)
{
    if (m_uiIronDoorTimer)
    {
        if (m_uiIronDoorTimer <= uiDiff)
        {
            switch (m_uiDoorStep)
            {
                case 0:
                    DoUseDoorOrButton(GO_IRON_CLAD_DOOR, 0, true);

                    if (Creature* pMrSmite = GetSingleCreatureFromStorage(NPC_MR_SMITE))
                        DoScriptText(INST_SAY_ALARM1, pMrSmite);

                    if (GameObject* pDoor = GetSingleGameObjectFromStorage(GO_IRON_CLAD_DOOR))
                    {
                        for (uint32 i = 148u; i <= 149; ++i)
                            if (Creature* guardfast = pDoor->GetMap()->GetCreature(GUID_PREFIX + i))
                                guardfast->GetMotionMaster()->MovePoint(0, pDoor->GetPositionX(), pDoor->GetPositionY(), pDoor->GetPositionZ(), FORCED_MOVEMENT_RUN, true);

                        if (Creature* guardlow = pDoor->GetMap()->GetCreature(GUID_PREFIX + 150))
                            guardlow->GetMotionMaster()->MovePoint(0, pDoor->GetPositionX(), pDoor->GetPositionY(), pDoor->GetPositionZ(), FORCED_MOVEMENT_WALK, true);
                    }
                    ++m_uiDoorStep;
                    m_uiIronDoorTimer = 15000;
                    break;
                case 1:
                    if (Creature* pMrSmite = GetSingleCreatureFromStorage(NPC_MR_SMITE))
                        DoScriptText(INST_SAY_ALARM2, pMrSmite);

                    m_uiDoorStep = 0;
                    m_uiIronDoorTimer = 0;
                    break;
            }
        }
        else
            m_uiIronDoorTimer -= uiDiff;
    }
}

InstanceData* GetInstanceData_instance_deadmines(Map* pMap)
{
    return new instance_deadmines(pMap);
}

void AddSC_instance_deadmines()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "instance_deadmines";
    pNewScript->GetInstanceData = &GetInstanceData_instance_deadmines;
    pNewScript->RegisterSelf();
}
