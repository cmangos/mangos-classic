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
SDName: Instance_Scholomance
SD%Complete: 99
SDComment: Possible some D2 or other exotic missing
SDCategory: Scholomance
EndScriptData */

#include "precompiled.h"
#include "scholomance.h"

instance_scholomance::instance_scholomance(Map* pMap) : ScriptedInstance(pMap),
    m_uiGandlingEvent(0),
    m_bIsRoomReset(false)
{
    Initialize();
}

void instance_scholomance::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

    for (uint8 i = 0; i < MAX_EVENTS; ++i)
        m_mGandlingData[aGandlingEvents[i]] = GandlingEventData();
}

void instance_scholomance::OnPlayerEnter(Player* pPlayer)
{
    // Summon Gandling if can
    DoSpawnGandlingIfCan(true);

    if (GetData(TYPE_RATTLEGORE) == DONE)
        DoRespawnEntranceRoom(pPlayer);
}

void instance_scholomance::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        // Store the Guids of the NPCs in the entrance room to remove them
        // and spawn their replacement or Rattlegore's death
        // (only store those located in the room volume)
        case NPC_REANIMATED_CORPSE:
        case NPC_DISEASED_GHOUL:
        case NPC_RISEN_ABERRATION:
            if (GetData(TYPE_RATTLEGORE) != DONE && (pCreature->GetPositionZ() > aEntranceRoom->m_fCenterZ) && (pCreature->GetPositionX() - aEntranceRoom->m_fCornerX < aEntranceRoom->m_uiLength) && (pCreature->GetPositionY() - aEntranceRoom->m_fCornerY < aEntranceRoom->m_uiWidth))
                m_sEntranceRoomGuids.insert(pCreature->GetObjectGuid());
            break;
        case NPC_DARKMASTER_GANDLING:
            m_mNpcEntryGuidStore[NPC_DARKMASTER_GANDLING] = pCreature->GetObjectGuid();
            break;
        case NPC_RISEN_GUARDIAN:
            GandlingEventMap::iterator find = m_mGandlingData.find(m_uiGandlingEvent);
            if (find != m_mGandlingData.end())
                find->second.m_sAddGuids.insert(pCreature->GetGUIDLow());
            break;
    }
}

void instance_scholomance::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_VIEWING_ROOM_DOOR:
            // In normal flow of the instance, this door is opened by a dropped key
            if (m_auiEncounter[TYPE_RATTLEGORE] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
        case GO_GATE_KIRTONOS:
        case GO_GATE_RAS:
        case GO_GATE_GANDLING:
            m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
            break;

        case GO_GATE_MALICIA:  m_mGandlingData[EVENT_ID_MALICIA].m_doorGuid  = pGo->GetObjectGuid(); break;
        case GO_GATE_THEOLEN:  m_mGandlingData[EVENT_ID_THEOLEN].m_doorGuid  = pGo->GetObjectGuid(); break;
        case GO_GATE_POLKELT:  m_mGandlingData[EVENT_ID_POLKELT].m_doorGuid  = pGo->GetObjectGuid(); break;
        case GO_GATE_RAVENIAN: m_mGandlingData[EVENT_ID_RAVENIAN].m_doorGuid = pGo->GetObjectGuid(); break;
        case GO_GATE_BAROV:    m_mGandlingData[EVENT_ID_BAROV].m_doorGuid    = pGo->GetObjectGuid(); break;
        case GO_GATE_ILLUCIA:  m_mGandlingData[EVENT_ID_ILLUCIA].m_doorGuid  = pGo->GetObjectGuid(); break;
    }
}

void instance_scholomance::DoRespawnEntranceRoom(Player* pSummoner)
{
    // safety check to avoid the room being reset for each OnPlayerEnter() call if Rattlegore is dead
    if (m_bIsRoomReset)
        return;

    if (!pSummoner)
        return;

    // Despawn the mobs already in the room with the exception of the necrofiend (not stored, so not despawned)
    for (GuidSet::const_iterator itr = m_sEntranceRoomGuids.begin(); itr != m_sEntranceRoomGuids.end(); ++itr)
    {
        if (Creature* pMob = instance->GetCreature(*itr))
            pMob->ForcedDespawn();
    }
    // Spawn the new and less numerous groups instead
    // Four groups, one in each corner
    // The creatures in each point are random
    // but follow the generic rule for each group of 4 NPCs:
    // 2 risen aberrations, 1 diseased ghoul, 1 diseased ghoul/reanimated corpse
    for (uint8 i = 0; i < MAX_GROUPS; ++i)
    {
        std::vector<uint32> uiMobList;              // Vector holding the 4 creatures entries for each spawned group
        uiMobList.push_back(NPC_RISEN_ABERRATION);  // 3 static NPC entries
        uiMobList.push_back(NPC_RISEN_ABERRATION);
        uiMobList.push_back(NPC_DISEASED_GHOUL);

        uint32 uiMobEntry;                          // will hold the last random creature entry

        // Pick the fourth NPC in the group and randomize the four possible spawns
        switch (urand(0, 1))
        {
            case 0: uiMobEntry = NPC_REANIMATED_CORPSE; break;
            case 1: uiMobEntry = NPC_DISEASED_GHOUL;    break;
        }

        uiMobList.push_back(uiMobEntry);
        std::random_shuffle(uiMobList.begin(), uiMobList.end());

        for (uint8 j = 0; j < MAX_NPC_PER_GROUP; ++j)
            pSummoner->SummonCreature(uiMobList[j], aEntranceRoomSpawnLocs[4*i+j].m_fX, aEntranceRoomSpawnLocs[4*i+j].m_fY, aEntranceRoomSpawnLocs[4*i+j].m_fZ, aEntranceRoomSpawnLocs[4*i+j].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0);
    }
    // spawn also a patrolling necrofiend
    // the waypoints are handled in DB creature_movement_template table (shared with the other necrofiend in the room)
    // the two other necrofiends in the instance are using DB creature_movement table
    if (Creature* pNecrofiend = pSummoner->SummonCreature(NPC_NECROFIEND, aEntranceRoomSpawnLocs[16].m_fX, aEntranceRoomSpawnLocs[16].m_fY, aEntranceRoomSpawnLocs[16].m_fZ, aEntranceRoomSpawnLocs[16].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0))
        pNecrofiend->GetMotionMaster()->MoveWaypoint();

    m_bIsRoomReset = true;

    debug_log("SD2: Entrance room in Scholomance reset after Rattlegore's death");
    return;
}

void instance_scholomance::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_KIRTONOS:
            // This door is initially closed by DB-scripts, so only use it in case of FAIL, DONE, or on aggro after wipe
            if (m_auiEncounter[uiType] != FAIL && uiData == IN_PROGRESS)
                return;
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(GO_GATE_KIRTONOS);
            break;
        case TYPE_RATTLEGORE:
            m_auiEncounter[uiType] = uiData;
            if (uiData == DONE)
            {
                if (Player* pPlayer = GetPlayerInMap())
                    DoRespawnEntranceRoom(pPlayer);
            }
            break;
        case TYPE_RAS_FROSTWHISPER:
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(GO_GATE_RAS);
            break;
        case TYPE_MALICIA:                                  // TODO this code can be simplified, when it is known which event-ids correspond to which room
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(m_mGandlingData[EVENT_ID_MALICIA].m_doorGuid);
            break;
        case TYPE_THEOLEN:
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(m_mGandlingData[EVENT_ID_THEOLEN].m_doorGuid);
            break;
        case TYPE_POLKELT:
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(m_mGandlingData[EVENT_ID_POLKELT].m_doorGuid);
            break;
        case TYPE_RAVENIAN:
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(m_mGandlingData[EVENT_ID_RAVENIAN].m_doorGuid);
            break;
        case TYPE_ALEXEI_BAROV:
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(m_mGandlingData[EVENT_ID_BAROV].m_doorGuid);
            break;
        case TYPE_ILLUCIA_BAROV:
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(m_mGandlingData[EVENT_ID_ILLUCIA].m_doorGuid);
            break;
        case TYPE_GANDLING:
            m_auiEncounter[uiType] = uiData;
            // Close the door to main room, because the encounter will take place only in the main hall and random around all the 6 rooms
            DoUseDoorOrButton(GO_GATE_GANDLING);
            break;
    }

    // Summon Gandling
    if (uiData == DONE)
        DoSpawnGandlingIfCan(false);

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                   << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " "
                   << m_auiEncounter[6] << " " << m_auiEncounter[7] << " " << m_auiEncounter[8] << " " << m_auiEncounter[9];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_scholomance::DoSpawnGandlingIfCan(bool bByPlayerEnter)
{
    // Do not summon, if event finished
    if (m_auiEncounter[TYPE_GANDLING] == DONE)
        return;

    // Summon only once
    if (GetSingleCreatureFromStorage(NPC_DARKMASTER_GANDLING))
        return;

    Player* pPlayer = GetPlayerInMap();
    if (!pPlayer)
        return;

    // Check if all the six bosses are done first
    if (m_auiEncounter[TYPE_MALICIA] == DONE && m_auiEncounter[TYPE_THEOLEN] == DONE && m_auiEncounter[TYPE_POLKELT] == DONE &&
            m_auiEncounter[TYPE_RAVENIAN] == DONE && m_auiEncounter[TYPE_ALEXEI_BAROV] == DONE && m_auiEncounter[TYPE_ILLUCIA_BAROV] == DONE)
    {
        if (Creature* pGandling = pPlayer->SummonCreature(NPC_DARKMASTER_GANDLING, aGandlingSpawnLocs[0].m_fX, aGandlingSpawnLocs[0].m_fY, aGandlingSpawnLocs[0].m_fZ, aGandlingSpawnLocs[0].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0))
        {
            if (!bByPlayerEnter)
                DoScriptText(SAY_GANDLING_SPAWN, pGandling);
        }
    }
}

void instance_scholomance::HandlePortalEvent(uint32 uiEventId, uint32 uiData)
{
    GandlingEventMap::iterator find = m_mGandlingData.find(uiEventId);
    if (find == m_mGandlingData.end())
        return;

    if (uiData == SPECIAL)
    {
        // Set current Event index
        m_uiGandlingEvent = uiEventId;

        // Close door if needed
        if (!find->second.m_bIsActive)
        {
            find->second.m_bIsActive = true;
            DoUseDoorOrButton(find->second.m_doorGuid);
        }
    }
    // Toggle door and event state in case of state-switch
    else
    {
        if ((uiData == IN_PROGRESS && !find->second.m_bIsActive) ||
                (uiData == FAIL && find->second.m_bIsActive) ||
                (uiData == DONE && find->second.m_bIsActive))
        {
            find->second.m_bIsActive = !find->second.m_bIsActive;
            DoUseDoorOrButton(find->second.m_doorGuid);
        }
    }
}

uint32 instance_scholomance::GetData(uint32 uiType) const
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    return 0;
}

void instance_scholomance::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3] >> m_auiEncounter[4]
               >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7] >> m_auiEncounter[8] >> m_auiEncounter[9];

    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;

    OUT_LOAD_INST_DATA_COMPLETE;
}

void instance_scholomance::OnCreatureEnterCombat(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_KIRTONOS:            SetData(TYPE_KIRTONOS, IN_PROGRESS);         break;
        case NPC_RATTLEGORE:          SetData(TYPE_RATTLEGORE, IN_PROGRESS);       break;
        case NPC_RAS_FROSTWHISPER:    SetData(TYPE_RAS_FROSTWHISPER, IN_PROGRESS); break;
        case NPC_THEOLEN_KRASTINOV:   SetData(TYPE_THEOLEN, IN_PROGRESS);          break;
        case NPC_LOREKEEPER_POLKELT:  SetData(TYPE_POLKELT, IN_PROGRESS);          break;
        case NPC_RAVENIAN:            SetData(TYPE_RAVENIAN, IN_PROGRESS);         break;
        case NPC_ILLUCIA_BAROV:       SetData(TYPE_ILLUCIA_BAROV, IN_PROGRESS);    break;
        case NPC_ALEXEI_BAROV:        SetData(TYPE_ALEXEI_BAROV, IN_PROGRESS);     break;
        case NPC_INSTRUCTOR_MALICIA:  SetData(TYPE_MALICIA, IN_PROGRESS);          break;
        case NPC_DARKMASTER_GANDLING: SetData(TYPE_GANDLING, IN_PROGRESS);         break;

        case NPC_RISEN_GUARDIAN:
            for (GandlingEventMap::iterator itr = m_mGandlingData.begin(); itr != m_mGandlingData.end(); ++itr)
            {
                // if there are no minions for a room, skip it
                if (!itr->second.m_sAddGuids.empty())
                {
                    // set data to fail in case of player death
                    if (itr->second.m_sAddGuids.find(pCreature->GetGUIDLow()) != itr->second.m_sAddGuids.end())
                    {
                        HandlePortalEvent(itr->first, IN_PROGRESS);
                        break;
                    }
                }
            }
            break;
    }
}

void instance_scholomance::OnCreatureEvade(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_KIRTONOS:            SetData(TYPE_KIRTONOS, FAIL);         break;
        case NPC_RATTLEGORE:          SetData(TYPE_RATTLEGORE, FAIL);       break;
        case NPC_RAS_FROSTWHISPER:    SetData(TYPE_RAS_FROSTWHISPER, FAIL); break;
        case NPC_THEOLEN_KRASTINOV:   SetData(TYPE_THEOLEN, FAIL);          break;
        case NPC_LOREKEEPER_POLKELT:  SetData(TYPE_POLKELT, FAIL);          break;
        case NPC_RAVENIAN:            SetData(TYPE_RAVENIAN, FAIL);         break;
        case NPC_ILLUCIA_BAROV:       SetData(TYPE_ILLUCIA_BAROV, FAIL);    break;
        case NPC_ALEXEI_BAROV:        SetData(TYPE_ALEXEI_BAROV, FAIL);     break;
        case NPC_INSTRUCTOR_MALICIA:  SetData(TYPE_MALICIA, FAIL);          break;
        case NPC_DARKMASTER_GANDLING: SetData(TYPE_GANDLING, FAIL);         break;

        case NPC_RISEN_GUARDIAN:
            for (GandlingEventMap::iterator itr = m_mGandlingData.begin(); itr != m_mGandlingData.end(); ++itr)
            {
                // if there are no minions for a room, skip it
                if (!itr->second.m_sAddGuids.empty())
                {
                    // set data to fail in case of player death
                    if (itr->second.m_sAddGuids.find(pCreature->GetGUIDLow()) != itr->second.m_sAddGuids.end())
                    {
                        HandlePortalEvent(itr->first, FAIL);
                        break;
                    }
                }
            }
            break;
    }
}

void instance_scholomance::OnCreatureDeath(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_KIRTONOS:            SetData(TYPE_KIRTONOS, DONE);         break;
        case NPC_RATTLEGORE:          SetData(TYPE_RATTLEGORE, DONE);       break;
        case NPC_RAS_FROSTWHISPER:    SetData(TYPE_RAS_FROSTWHISPER, DONE); break;
        case NPC_THEOLEN_KRASTINOV:   SetData(TYPE_THEOLEN, DONE);          break;
        case NPC_LOREKEEPER_POLKELT:  SetData(TYPE_POLKELT, DONE);          break;
        case NPC_RAVENIAN:            SetData(TYPE_RAVENIAN, DONE);         break;
        case NPC_ILLUCIA_BAROV:       SetData(TYPE_ILLUCIA_BAROV, DONE);    break;
        case NPC_ALEXEI_BAROV:        SetData(TYPE_ALEXEI_BAROV, DONE);     break;
        case NPC_INSTRUCTOR_MALICIA:  SetData(TYPE_MALICIA, DONE);          break;
        case NPC_DARKMASTER_GANDLING: SetData(TYPE_GANDLING, DONE);         break;

        case NPC_RISEN_GUARDIAN:
            for (GandlingEventMap::iterator itr = m_mGandlingData.begin(); itr != m_mGandlingData.end(); ++itr)
            {
                // if there are no minions for a room, skip it
                if (!itr->second.m_sAddGuids.empty())
                {
                    // search for the dead minion and erase it
                    if (itr->second.m_sAddGuids.find(pCreature->GetGUIDLow()) != itr->second.m_sAddGuids.end())
                    {
                        itr->second.m_sAddGuids.erase(pCreature->GetGUIDLow());

                        // if the current list is empty; set event id as done
                        if (itr->second.m_sAddGuids.empty())
                        {
                            HandlePortalEvent(itr->first, DONE);
                            break;
                        }
                    }
                }
            }
            break;
    }
}

InstanceData* GetInstanceData_instance_scholomance(Map* pMap)
{
    return new instance_scholomance(pMap);
}

bool ProcessEventId_event_spell_gandling_shadow_portal(uint32 uiEventId, Object* pSource, Object* /*pTarget*/, bool /*bIsStart*/)
{
    if (pSource->GetTypeId() == TYPEID_UNIT)
    {
        if (instance_scholomance* pInstance = (instance_scholomance*)((Creature*)pSource)->GetInstanceData())
        {
            // Check if we are handling an event associated with the room events of gandling
            for (uint8 i = 0; i < MAX_EVENTS; ++i)
            {
                if (uiEventId == aGandlingEvents[i])
                {
                    // Set data in progress for the current event and store current event
                    pInstance->HandlePortalEvent(uiEventId, SPECIAL);
                    // return false, to allow the DB-scripts to summon some NPCSs
                    return false;
                }
            }
        }
    }
    return false;
}

void AddSC_instance_scholomance()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_scholomance";
    pNewScript->GetInstanceData = &GetInstanceData_instance_scholomance;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "event_spell_gandling_shadow_portal";
    pNewScript->pProcessEventId = &ProcessEventId_event_spell_gandling_shadow_portal;
    pNewScript->RegisterSelf();
}
