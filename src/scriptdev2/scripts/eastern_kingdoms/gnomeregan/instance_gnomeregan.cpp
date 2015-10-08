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
SDName: Instance_Gnomeregan
SD%Complete: 90%
SDComment:  Support for Grubbis and Thermaplugg Encounters
SDCategory: Gnomeregan
EndScriptData */

#include "precompiled.h"
#include "gnomeregan.h"

instance_gnomeregan::instance_gnomeregan(Map* pMap) : ScriptedInstance(pMap)
{
    Initialize();
}

void instance_gnomeregan::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

    for (uint8 i = 0; i < MAX_GNOME_FACES; ++i)
    {
        m_asBombFaces[i].m_bActivated = false;
        m_asBombFaces[i].m_uiBombTimer = 0;
    }
}

void instance_gnomeregan::OnCreatureCreate(Creature* pCreature)
{
    if (pCreature->GetEntry() == NPC_BLASTMASTER_SHORTFUSE)
        m_mNpcEntryGuidStore[NPC_BLASTMASTER_SHORTFUSE] = pCreature->GetObjectGuid();
}

void instance_gnomeregan::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_CAVE_IN_NORTH:
        case GO_CAVE_IN_SOUTH:
        case GO_THE_FINAL_CHAMBER:
            m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
            break;

        case GO_RED_ROCKET:         m_lRedRocketGUIDs.push_back(pGo->GetObjectGuid());         return;
        case GO_EXPLOSIVE_CHARGE:   m_luiExplosiveChargeGUIDs.push_back(pGo->GetObjectGuid()); return;

        case GO_GNOME_FACE_1: m_asBombFaces[0].m_gnomeFaceGuid = pGo->GetObjectGuid(); return;
        case GO_GNOME_FACE_2: m_asBombFaces[1].m_gnomeFaceGuid = pGo->GetObjectGuid(); return;
        case GO_GNOME_FACE_3: m_asBombFaces[2].m_gnomeFaceGuid = pGo->GetObjectGuid(); return;
        case GO_GNOME_FACE_4: m_asBombFaces[3].m_gnomeFaceGuid = pGo->GetObjectGuid(); return;
        case GO_GNOME_FACE_5: m_asBombFaces[4].m_gnomeFaceGuid = pGo->GetObjectGuid(); return;
        case GO_GNOME_FACE_6: m_asBombFaces[5].m_gnomeFaceGuid = pGo->GetObjectGuid(); return;
    }
}

static bool sortFromEastToWest(GameObject* pFirst, GameObject* pSecond)
{
    return pFirst && pSecond && pFirst->GetPositionY() < pSecond->GetPositionY();
}

void instance_gnomeregan::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_GRUBBIS:
            m_auiEncounter[0] = uiData;
            if (uiData == IN_PROGRESS)
            {
                // Sort the explosive charges if needed
                if (!m_luiExplosiveChargeGUIDs.empty())
                {
                    std::list<GameObject*> lExplosiveCharges;
                    for (GuidList::const_iterator itr = m_luiExplosiveChargeGUIDs.begin(); itr != m_luiExplosiveChargeGUIDs.end(); ++itr)
                    {
                        if (GameObject* pCharge = instance->GetGameObject(*itr))
                            lExplosiveCharges.push_back(pCharge);
                    }
                    m_luiExplosiveChargeGUIDs.clear();

                    // Sort from east to west
                    lExplosiveCharges.sort(sortFromEastToWest);

                    // Sort to south and north
                    uint8 uiCounterSouth = 0;
                    uint8 uiCounterNorth = 0;
                    GameObject* pCaveInSouth = GetSingleGameObjectFromStorage(GO_CAVE_IN_SOUTH);
                    GameObject* pCaveInNorth = GetSingleGameObjectFromStorage(GO_CAVE_IN_NORTH);
                    if (pCaveInSouth && pCaveInNorth)
                    {
                        for (std::list<GameObject*>::iterator itr = lExplosiveCharges.begin(); itr != lExplosiveCharges.end(); ++itr)
                        {
                            if ((*itr)->GetDistanceOrder(pCaveInSouth, pCaveInNorth) && uiCounterSouth < MAX_EXPLOSIVES_PER_SIDE)
                            {
                                m_aExplosiveSortedGuids[0][uiCounterSouth] = (*itr)->GetObjectGuid();
                                ++uiCounterSouth;
                            }
                            else if (uiCounterNorth < MAX_EXPLOSIVES_PER_SIDE)
                            {
                                m_aExplosiveSortedGuids[1][uiCounterNorth] = (*itr)->GetObjectGuid();
                                ++uiCounterNorth;
                            }
                        }
                    }
                }
            }
            if (uiData == FAIL)
            {
                // Despawn possible spawned explosive charges
                SetData(TYPE_EXPLOSIVE_CHARGE, DATA_EXPLOSIVE_CHARGE_USE);
            }
            if (uiData == DONE)
            {
                for (GuidList::const_iterator itr = m_lRedRocketGUIDs.begin(); itr != m_lRedRocketGUIDs.end(); ++itr)
                    DoRespawnGameObject(*itr, HOUR);
            }
            break;
        case TYPE_EXPLOSIVE_CHARGE:
            switch (uiData)
            {
                case DATA_EXPLOSIVE_CHARGE_1:
                    DoRespawnGameObject(m_aExplosiveSortedGuids[0][0], HOUR);
                    m_luiSpawnedExplosiveChargeGUIDs.push_back(m_aExplosiveSortedGuids[0][0]);
                    break;
                case DATA_EXPLOSIVE_CHARGE_2:
                    DoRespawnGameObject(m_aExplosiveSortedGuids[0][1], HOUR);
                    m_luiSpawnedExplosiveChargeGUIDs.push_back(m_aExplosiveSortedGuids[0][1]);
                    break;
                case DATA_EXPLOSIVE_CHARGE_3:
                    DoRespawnGameObject(m_aExplosiveSortedGuids[1][0], HOUR);
                    m_luiSpawnedExplosiveChargeGUIDs.push_back(m_aExplosiveSortedGuids[1][0]);
                    break;
                case DATA_EXPLOSIVE_CHARGE_4:
                    DoRespawnGameObject(m_aExplosiveSortedGuids[1][1], HOUR);
                    m_luiSpawnedExplosiveChargeGUIDs.push_back(m_aExplosiveSortedGuids[1][1]);
                    break;
                case DATA_EXPLOSIVE_CHARGE_USE:
                    Creature* pBlastmaster = GetSingleCreatureFromStorage(NPC_BLASTMASTER_SHORTFUSE);
                    if (!pBlastmaster)
                        break;
                    for (GuidList::const_iterator itr = m_luiSpawnedExplosiveChargeGUIDs.begin(); itr != m_luiSpawnedExplosiveChargeGUIDs.end(); ++itr)
                    {
                        if (GameObject* pExplosive = instance->GetGameObject(*itr))
                            pExplosive->Use(pBlastmaster);
                    }
                    m_luiSpawnedExplosiveChargeGUIDs.clear();
                    break;
            }
            return;
        case TYPE_THERMAPLUGG:
            m_auiEncounter[1] = uiData;
            if (uiData == IN_PROGRESS)
            {
                // Make Door locked
                if (GameObject* pDoor = GetSingleGameObjectFromStorage(GO_THE_FINAL_CHAMBER))
                {
                    pDoor->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED);
                    if (pDoor->getLootState() == GO_ACTIVATED)
                        pDoor->ResetDoorOrButton();
                }

                // Always directly activates this bomb-face
                DoActivateBombFace(2);
            }
            else if (uiData == DONE || uiData == FAIL)
            {
                // Make Door unlocked again
                if (GameObject* pDoor = GetSingleGameObjectFromStorage(GO_THE_FINAL_CHAMBER))
                {
                    pDoor->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED);
                    if (pDoor->getLootState() == GO_READY)
                        pDoor->UseDoorOrButton();
                }

                // Deactivate all remaining BombFaces
                for (uint8 i = 0; i < MAX_GNOME_FACES; ++i)
                    DoDeactivateBombFace(i);
            }
            break;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_gnomeregan::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1];

    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_gnomeregan::GetData(uint32 uiType) const
{
    switch (uiType)
    {
        case TYPE_GRUBBIS:      return m_auiEncounter[0];
        case TYPE_THERMAPLUGG:  return m_auiEncounter[1];
        default:
            return 0;
    }
}

sBombFace* instance_gnomeregan::GetBombFaces()
{
    return m_asBombFaces;
}

void instance_gnomeregan::DoActivateBombFace(uint8 uiIndex)
{
    if (uiIndex >= MAX_GNOME_FACES)
        return;

    if (!m_asBombFaces[uiIndex].m_bActivated)
    {
        DoUseDoorOrButton(m_asBombFaces[uiIndex].m_gnomeFaceGuid);
        m_asBombFaces[uiIndex].m_bActivated = true;
        m_asBombFaces[uiIndex].m_uiBombTimer = 3000;
    }
}

void instance_gnomeregan::DoDeactivateBombFace(uint8 uiIndex)
{
    if (uiIndex >= MAX_GNOME_FACES)
        return;

    if (m_asBombFaces[uiIndex].m_bActivated)
    {
        DoUseDoorOrButton(m_asBombFaces[uiIndex].m_gnomeFaceGuid);
        m_asBombFaces[uiIndex].m_bActivated = false;
        m_asBombFaces[uiIndex].m_uiBombTimer = 0;
    }
}

InstanceData* GetInstanceData_instance_gnomeregan(Map* pMap)
{
    return new instance_gnomeregan(pMap);
}

void AddSC_instance_gnomeregan()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_gnomeregan";
    pNewScript->GetInstanceData = &GetInstanceData_instance_gnomeregan;
    pNewScript->RegisterSelf();
}
