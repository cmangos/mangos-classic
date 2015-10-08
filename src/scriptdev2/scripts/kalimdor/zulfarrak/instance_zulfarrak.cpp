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
SDName: instance_zulfarrak
SD%Complete: 80%
SDComment:
SDCategory: Zul'Farrak
EndScriptData */

#include "precompiled.h"
#include "zulfarrak.h"

instance_zulfarrak::instance_zulfarrak(Map* pMap) : ScriptedInstance(pMap),
    m_uiPyramidEventTimer(0)
{
    Initialize();
}

void instance_zulfarrak::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

void instance_zulfarrak::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_ANTUSUL:
        case NPC_SERGEANT_BLY:
            m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
        case NPC_SANDFURY_SLAVE:
        case NPC_SANDFURY_DRUDGE:
        case NPC_SANDFURY_CRETIN:
        case NPC_SANDFURY_ACOLYTE:
        case NPC_SANDFURY_ZEALOT:
            m_lPyramidTrollsGuidList.push_back(pCreature->GetObjectGuid());
            break;
    }
}

void instance_zulfarrak::OnObjectCreate(GameObject* pGo)
{
    if (pGo->GetEntry() == GO_SHALLOW_GRAVE)
        m_lShallowGravesGuidList.push_back(pGo->GetObjectGuid());
    else if (pGo->GetEntry() == GO_END_DOOR)
    {
        if (GetData(TYPE_PYRAMID_EVENT) == DONE)
            pGo->SetGoState(GO_STATE_ACTIVE);
    }
}

void instance_zulfarrak::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_VELRATHA:
        case TYPE_GAHZRILLA:
        case TYPE_ANTUSUL:
        case TYPE_THEKA:
        case TYPE_ZUMRAH:
        case TYPE_CHIEF_SANDSCALP:
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_NEKRUM:
            m_auiEncounter[uiType] = uiData;
            if (uiData == DONE && GetData(TYPE_SEZZZIZ) == DONE)
                SetData(TYPE_PYRAMID_EVENT, DONE);
            break;
        case TYPE_SEZZZIZ:
            m_auiEncounter[uiType] = uiData;
            if (uiData == DONE && GetData(TYPE_NEKRUM) == DONE)
                SetData(TYPE_PYRAMID_EVENT, DONE);
            break;
        case TYPE_PYRAMID_EVENT:
            m_auiEncounter[uiType] = uiData;
            if (uiData == IN_PROGRESS)
                m_uiPyramidEventTimer = 20000;
            else if (uiData == DONE)
                m_uiPyramidEventTimer = 0;
            break;
        default:
            return;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " " << m_auiEncounter[3]
                   << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " " << m_auiEncounter[6] << " " << m_auiEncounter[7]
                   << " " << m_auiEncounter[8];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_zulfarrak::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
               >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7]
               >> m_auiEncounter[8];

    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_zulfarrak::GetData(uint32 uiType) const
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    return 0;
}

void instance_zulfarrak::OnCreatureEnterCombat(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_VELRATHA: SetData(TYPE_VELRATHA, IN_PROGRESS); break;
        case NPC_GAHZRILLA: SetData(TYPE_GAHZRILLA, IN_PROGRESS); break;
        case NPC_ANTUSUL: SetData(TYPE_ANTUSUL, IN_PROGRESS); break;
        case NPC_THEKA: SetData(TYPE_THEKA, IN_PROGRESS); break;
        case NPC_ZUMRAH: SetData(TYPE_ZUMRAH, IN_PROGRESS); break;
        case NPC_NEKRUM: SetData(TYPE_NEKRUM, IN_PROGRESS); break;
        case NPC_SEZZZIZ: SetData(TYPE_SEZZZIZ, IN_PROGRESS); break;
        case NPC_CHIEF_SANDSCALP: SetData(TYPE_CHIEF_SANDSCALP, IN_PROGRESS); break;
    }
}

void instance_zulfarrak::OnCreatureEvade(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_VELRATHA: SetData(TYPE_VELRATHA, FAIL); break;
        case NPC_GAHZRILLA: SetData(TYPE_GAHZRILLA, FAIL); break;
        case NPC_ANTUSUL: SetData(TYPE_ANTUSUL, FAIL); break;
        case NPC_THEKA: SetData(TYPE_THEKA, FAIL); break;
        case NPC_ZUMRAH: SetData(TYPE_ZUMRAH, FAIL); break;
        case NPC_NEKRUM: SetData(TYPE_NEKRUM, FAIL); break;
        case NPC_SEZZZIZ: SetData(TYPE_SEZZZIZ, FAIL); break;
        case NPC_CHIEF_SANDSCALP: SetData(TYPE_CHIEF_SANDSCALP, FAIL); break;
    }
}

void instance_zulfarrak::OnCreatureDeath(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_VELRATHA: SetData(TYPE_VELRATHA, DONE); break;
        case NPC_GAHZRILLA: SetData(TYPE_GAHZRILLA, DONE); break;
        case NPC_ANTUSUL: SetData(TYPE_ANTUSUL, DONE); break;
        case NPC_THEKA: SetData(TYPE_THEKA, DONE); break;
        case NPC_ZUMRAH: SetData(TYPE_ZUMRAH, DONE); break;
        case NPC_NEKRUM: SetData(TYPE_NEKRUM, DONE); break;
        case NPC_SEZZZIZ: SetData(TYPE_SEZZZIZ, DONE); break;
        case NPC_CHIEF_SANDSCALP: SetData(TYPE_CHIEF_SANDSCALP, DONE); break;
    }
}

void instance_zulfarrak::Update(uint32 uiDiff)
{
    if (m_uiPyramidEventTimer)
    {
        if (m_uiPyramidEventTimer <= uiDiff)
        {
            if (m_lPyramidTrollsGuidList.empty())
            {
                m_uiPyramidEventTimer = urand(3000, 10000);
                return;
            }

            GuidList::iterator iter = m_lPyramidTrollsGuidList.begin();
            advance(iter, urand(0, m_lPyramidTrollsGuidList.size() - 1));

            // Remove the selected troll
            ObjectGuid selectedGuid = *iter;
            m_lPyramidTrollsGuidList.erase(iter);

            // Move the selected troll to the top of the pyramid. Note: the algorythm may be more complicated than this, but for the moment this will do.
            if (Creature* pTroll = instance->GetCreature(selectedGuid))
            {
                // Pick another one if already in combat or already killed
                if (pTroll->getVictim() || !pTroll->isAlive())
                {
                    m_uiPyramidEventTimer = urand(0, 2) ? urand(3000, 10000) : 1000;
                    return;
                }

                float fX, fY, fZ;
                if (Creature* pBly = GetSingleCreatureFromStorage(NPC_SERGEANT_BLY))
                {
                    // ToDo: research if there is anything special if these guys die
                    if (!pBly->isAlive())
                    {
                        m_uiPyramidEventTimer = 0;
                        return;
                    }

                    pBly->GetRandomPoint(pBly->GetPositionX(), pBly->GetPositionY(), pBly->GetPositionZ(), 4.0f, fX, fY, fZ);
                    pTroll->SetWalk(false);
                    pTroll->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                }
            }
            m_uiPyramidEventTimer = urand(0, 2) ? urand(3000, 10000) : 1000;
        }
        else
            m_uiPyramidEventTimer -= uiDiff;
    }
}

InstanceData* GetInstanceData_instance_zulfarrak(Map* pMap)
{
    return new instance_zulfarrak(pMap);
}

void AddSC_instance_zulfarrak()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_zulfarrak";
    pNewScript->GetInstanceData = &GetInstanceData_instance_zulfarrak;
    pNewScript->RegisterSelf();
}
