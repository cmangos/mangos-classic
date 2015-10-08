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
SDName: Instance_Uldaman
SD%Complete: 60
SDComment:
SDCategory: Uldaman
EndScriptData
*/

#include "precompiled.h"
#include "uldaman.h"

instance_uldaman::instance_uldaman(Map* pMap) : ScriptedInstance(pMap),
    m_uiKeeperCooldown(0),
    m_uiStoneKeepersFallen(0)
{
    Initialize();
}

void instance_uldaman::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

void instance_uldaman::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_TEMPLE_DOOR_UPPER:
        case GO_TEMPLE_DOOR_LOWER:
            if (GetData(TYPE_ALTAR_EVENT) == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_ANCIENT_VAULT:
            if (GetData(TYPE_ARCHAEDAS) == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_ANCIENT_TREASURE:
            break;
        default:
            return;
    }
    m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
}

void instance_uldaman::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_HALLSHAPER:
        case NPC_CUSTODIAN:
            m_lWardens.push_back(pCreature->GetObjectGuid());
            break;
        case NPC_STONE_KEEPER:
            m_lKeepers.push_back(pCreature->GetObjectGuid());
            break;
        case NPC_ARCHAEDAS:
            m_mNpcEntryGuidStore[NPC_ARCHAEDAS] = pCreature->GetObjectGuid();
            break;
        default:
            break;
    }
}

void instance_uldaman::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_ALTAR_EVENT:
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_TEMPLE_DOOR_UPPER);
                DoUseDoorOrButton(GO_TEMPLE_DOOR_LOWER);
            }
            else if (uiData == IN_PROGRESS)
            {
                // Also do a reset before starting the event - this will respawn dead Keepers
                DoResetKeeperEvent();
                m_uiKeeperCooldown = 5000;
            }
            else if (uiData == NOT_STARTED)
            {
                DoResetKeeperEvent();
                m_uiStoneKeepersFallen = 0;
            }
            m_auiEncounter[0] = uiData;
            break;

        case TYPE_ARCHAEDAS:
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_ANCIENT_VAULT);
                DoRespawnGameObject(GO_ANCIENT_TREASURE, HOUR);
            }
            m_auiEncounter[1] = uiData;
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

void instance_uldaman::Load(const char* chrIn)
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

void instance_uldaman::SetData64(uint32 uiData, uint64 uiGuid)
{
    switch (uiData)
    {
            // ToDo: check if this one is used in ACID. Otherwise it can be dropped
        case DATA_EVENT_STARTER:
            m_playerGuid = ObjectGuid(uiGuid);
            break;
    }
}

uint32 instance_uldaman::GetData(uint32 uiType) const
{
    switch (uiType)
    {
        case TYPE_ALTAR_EVENT:
            return m_auiEncounter[0];
        case TYPE_ARCHAEDAS:
            return m_auiEncounter[1];
    }
    return 0;
}

uint64 instance_uldaman::GetData64(uint32 uiData) const
{
    switch (uiData)
    {
        case DATA_EVENT_STARTER:
            return m_playerGuid.GetRawValue();
    }
    return 0;
}

void instance_uldaman::StartEvent(uint32 uiEventId, Player* pPlayer)
{
    m_playerGuid = pPlayer->GetObjectGuid();

    if (uiEventId == EVENT_ID_ALTAR_KEEPER)
    {
        if (GetData(TYPE_ALTAR_EVENT) == NOT_STARTED)
            SetData(TYPE_ALTAR_EVENT, IN_PROGRESS);
    }
    else if (uiEventId == EVENT_ID_ALTAR_ARCHAEDAS)
    {
        if (GetData(TYPE_ARCHAEDAS) == NOT_STARTED || GetData(TYPE_ARCHAEDAS) == FAIL)
            SetData(TYPE_ARCHAEDAS, SPECIAL);
    }
}

void instance_uldaman::DoResetKeeperEvent()
{
    if (m_lKeepers.empty())
    {
        script_error_log("Instance Uldaman: ERROR creature %u couldn't be found or something really bad happened.", NPC_STONE_KEEPER);
        return;
    }

    // Force reset all keepers to the original state
    for (GuidList::const_iterator itr = m_lKeepers.begin(); itr != m_lKeepers.end(); ++itr)
    {
        if (Creature* pKeeper = instance->GetCreature(*itr))
        {
            if (!pKeeper->isAlive())
                pKeeper->Respawn();
        }
    }
}

Creature* instance_uldaman::GetClosestDwarfNotInCombat(Creature* pSearcher)
{
    std::list<Creature*> lTemp;

    for (GuidList::const_iterator itr = m_lWardens.begin(); itr != m_lWardens.end(); ++itr)
    {
        Creature* pTemp = instance->GetCreature(*itr);

        if (pTemp && pTemp->isAlive() && !pTemp->getVictim())
            lTemp.push_back(pTemp);
    }

    if (lTemp.empty())
        return nullptr;

    lTemp.sort(ObjectDistanceOrder(pSearcher));
    return lTemp.front();
}

void instance_uldaman::OnCreatureEvade(Creature* pCreature)
{
    // Reset Altar event
    if (pCreature->GetEntry() == NPC_STONE_KEEPER)
        SetData(TYPE_ALTAR_EVENT, NOT_STARTED);
}

void instance_uldaman::OnCreatureDeath(Creature* pCreature)
{
    if (pCreature->GetEntry() == NPC_STONE_KEEPER)
    {
        ++m_uiStoneKeepersFallen;

        if (m_lKeepers.size() == m_uiStoneKeepersFallen)
            SetData(TYPE_ALTAR_EVENT, DONE);
        else
            m_uiKeeperCooldown = 5000;
    }
}

void instance_uldaman::Update(uint32 uiDiff)
{
    if (GetData(TYPE_ALTAR_EVENT) != IN_PROGRESS)
        return;

    if (!m_uiKeeperCooldown)
        return;

    if (m_uiKeeperCooldown <= uiDiff)
    {
        for (GuidList::const_iterator itr = m_lKeepers.begin(); itr != m_lKeepers.end(); ++itr)
        {
            // Get Keeper which is alive and out of combat
            Creature* pKeeper = instance->GetCreature(*itr);
            if (!pKeeper || !pKeeper->isAlive() || pKeeper->getVictim())
                continue;

            // Get starter player for attack
            Player* pPlayer = pKeeper->GetMap()->GetPlayer(m_playerGuid);
            if (!pPlayer || !pPlayer->isAlive())
            {
                // If he's not available, then get a random player, within a reasonamble distance in map
                pPlayer = GetPlayerInMap(true, false);
                if (!pPlayer || !pPlayer->IsWithinDistInMap(pKeeper, 50.0f))
                {
                    SetData(TYPE_ALTAR_EVENT, NOT_STARTED);
                    return;
                }
            }

            // Attack the player
            pKeeper->RemoveAurasDueToSpell(SPELL_STONED);
            pKeeper->AI()->AttackStart(pPlayer);
            break;
        }

        m_uiKeeperCooldown = 0;
    }
    else
        m_uiKeeperCooldown -= uiDiff;
}

InstanceData* GetInstanceData_instance_uldaman(Map* pMap)
{
    return new instance_uldaman(pMap);
}

bool ProcessEventId_event_spell_altar_boss_aggro(uint32 uiEventId, Object* pSource, Object* /*pTarget*/, bool bIsStart)
{
    if (bIsStart && pSource->GetTypeId() == TYPEID_PLAYER)
    {
        if (instance_uldaman* pInstance = (instance_uldaman*)((Player*)pSource)->GetInstanceData())
        {
            pInstance->StartEvent(uiEventId, (Player*)pSource);
            return true;
        }
    }
    return false;
}

void AddSC_instance_uldaman()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_uldaman";
    pNewScript->GetInstanceData = &GetInstanceData_instance_uldaman;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "event_spell_altar_boss_aggro";
    pNewScript->pProcessEventId = &ProcessEventId_event_spell_altar_boss_aggro;
    pNewScript->RegisterSelf();
}
