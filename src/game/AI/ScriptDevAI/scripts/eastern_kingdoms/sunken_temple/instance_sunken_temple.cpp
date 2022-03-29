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
SDName: instance_sunken_temple
SD%Complete: 90
SDComment: Hakkar Summon Event needs more sources to improve
SDCategory: Sunken Temple
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "sunken_temple.h"

instance_sunken_temple::instance_sunken_temple(Map* pMap) : ScriptedInstance(pMap),
    m_uiProtectorsRemaining(0),
    m_uiStatueCounter(0),
    m_uiFlameCounter(0),
    m_uiAvatarSummonTimer(0),
    m_uiSupressorTimer(0),
    m_suppressionTimer(0),
    m_bIsFirstHakkarWave(false),
    m_bCanSummonBloodkeeper(false)
{
    Initialize();
}

void instance_sunken_temple::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

void instance_sunken_temple::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_JAMMALAN_BARRIER:
            if (m_auiEncounter[TYPE_PROTECTORS] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_IDOL_OF_HAKKAR:
        case GO_HAKKAR_DOOR_1:
        case GO_HAKKAR_DOOR_2:
            break;

        case GO_ATALAI_LIGHT_BIG:
            m_luiBigLightGUIDs.push_back(pGo->GetObjectGuid());
            return;
        case GO_EVIL_CIRCLE:
            m_evilCircleGuidList.push_back(pGo->GetObjectGuid());
            return;
        case GO_ETERNAL_FLAME_1:
        case GO_ETERNAL_FLAME_2:
        case GO_ETERNAL_FLAME_3:
        case GO_ETERNAL_FLAME_4:
            m_luiFlameGUIDs.push_back(pGo->GetObjectGuid());
            return;

        default:
            return;
    }
    m_goEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
}

void instance_sunken_temple::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_ZOLO:
        case NPC_GASHER:
        case NPC_LORO:
        case NPC_HUKKU:
        case NPC_ZULLOR:
        case NPC_MIJAN:
            ++m_uiProtectorsRemaining;
            break;
        case NPC_JAMMALAN:
        case NPC_ATALARION:
            m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
        case NPC_SHADE_OF_ERANIKUS:
            // Make Shade of Eranikus attackable by players if Jammal'an is defeated
            if (GetData(TYPE_JAMMALAN) == DONE)
                pCreature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
            m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
    }
}

void instance_sunken_temple::OnCreatureEvade(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        // Hakkar Event Mobs: On Wipe set as failed!
        case NPC_BLOODKEEPER:
        case NPC_HAKKARI_MINION:
        case NPC_SUPPRESSOR:
        case NPC_AVATAR_OF_HAKKAR:
            SetData(TYPE_AVATAR, FAIL);
            break;
        // Shade of Eranikus: prevent it to become unattackable after a wipe
        case NPC_SHADE_OF_ERANIKUS:
            pCreature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
            break;
    }
}

void instance_sunken_temple::OnCreatureDeath(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_ATALARION:         SetData(TYPE_ATALARION, DONE); break;
        case NPC_JAMMALAN:          SetData(TYPE_JAMMALAN, DONE);  break;
        case NPC_AVATAR_OF_HAKKAR:  SetData(TYPE_AVATAR, DONE);    break;

        case NPC_SUPPRESSOR:
            m_bCanSummonBloodkeeper = true;
            break;

        // Jammalain mini-bosses
        case NPC_ZOLO:
        case NPC_GASHER:
        case NPC_LORO:
        case NPC_HUKKU:
        case NPC_ZULLOR:
        case NPC_MIJAN:
            SetData(TYPE_PROTECTORS, DONE);
            break;
    }
}

void instance_sunken_temple::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_ATALARION:
            if (uiData == SPECIAL)
                DoSpawnAtalarionIfCan();
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_PROTECTORS:
            if (uiData == DONE)
            {
                --m_uiProtectorsRemaining;
                if (!m_uiProtectorsRemaining)
                {
                    m_auiEncounter[uiType] = uiData;
                    DoUseDoorOrButton(GO_JAMMALAN_BARRIER);
                    // Intro yell
                    DoOrSimulateScriptTextForThisInstance(SAY_JAMMALAN_INTRO, NPC_JAMMALAN);
                }
            }
            break;
        case TYPE_JAMMALAN:
            if (uiData == DONE)
            {
                if (Creature* pEranikus = GetSingleCreatureFromStorage(NPC_SHADE_OF_ERANIKUS))
                    pEranikus->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
            }
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_MALFURION:
            if (uiData == IN_PROGRESS)
                m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_AVATAR:
            if (uiData == SPECIAL)
            {
                Creature* shadeofHakkar = GetSingleCreatureFromStorage(NPC_SHADE_OF_HAKKAR);
                if (!shadeofHakkar)
                    return;

                if (shadeofHakkar->HasAura(SPELL_SUPPRESSION) && !m_suppressionTimer)
                {
                    m_suppressionTimer = 20 * IN_MILLISECONDS;
                    return; // Return to avoid handling one of the flames dousing
                }

                ++m_uiFlameCounter;

                switch (m_uiFlameCounter)
                {
                    // Yells on each flame
                    // TODO It might be possible that these yells should be ordered randomly, however this is the seen state
                    case 1: DoScriptText(SAY_AVATAR_BRAZIER_1, shadeofHakkar); break;
                    case 2: DoScriptText(SAY_AVATAR_BRAZIER_2, shadeofHakkar); break;
                    case 3: DoScriptText(SAY_AVATAR_BRAZIER_3, shadeofHakkar); break;
                    // Summon the avatar when all flames are used
                    case MAX_FLAMES:
                        DoScriptText(SAY_AVATAR_BRAZIER_4, shadeofHakkar);
                        shadeofHakkar->CastSpell(shadeofHakkar, SPELL_SUMMON_AVATAR, TRIGGERED_OLD_TRIGGERED);
                        m_uiAvatarSummonTimer = 0;
                        m_uiSupressorTimer = 0;
                        break;
                }

                // Summon the suppressors only after the flames are doused
                // Summon timer is confusing random; timers were: 13, 39 and 52 secs;
                if (m_uiFlameCounter != MAX_FLAMES)
                    m_uiSupressorTimer = urand(15000, 45000);

                return;
            }

            // Prevent double processing
            if (m_auiEncounter[uiType] == uiData)
                return;

            if (uiData == IN_PROGRESS)
            {
                m_uiSupressorTimer = 0;
                DoUpdateFlamesFlags(false);

                // Summon timer; use a small delay
                m_uiAvatarSummonTimer = 3000;
                m_bIsFirstHakkarWave = true;

                // Summon the shade
                Player* pPlayer = GetPlayerInMap();
                if (!pPlayer)
                    return;

                if (Creature* pShade = pPlayer->SummonCreature(NPC_SHADE_OF_HAKKAR, aSunkenTempleLocation[1].m_fX, aSunkenTempleLocation[1].m_fY, aSunkenTempleLocation[1].m_fZ, aSunkenTempleLocation[1].m_fO, TEMPSPAWN_MANUAL_DESPAWN, 0))
                {
                    m_npcEntryGuidStore[NPC_SHADE_OF_HAKKAR] = pShade->GetObjectGuid();
                    pShade->SetRespawnDelay(DAY);
                }

                // Respawn evil circle visuals
                for (auto evilCircleGuid : m_evilCircleGuidList)
                    DoRespawnGameObject(evilCircleGuid, 30 * MINUTE);
            }
            else if (uiData == FAIL)
            {
                // In case of wipe during the summoning ritual the shade is despawned
                // The trash mobs stay in place, they are not despawned

                // Despawn the shade and the avatar if needed
                if (Creature* pShade = GetSingleCreatureFromStorage(NPC_SHADE_OF_HAKKAR))
                    pShade->ForcedDespawn();

                // Reset flames - remove no interact flag
                DoUpdateFlamesFlags(false);
                m_uiFlameCounter = 0;

                // Despawn the evil circle visuals
                for (auto evilCircleGuid : m_evilCircleGuidList)
                {
                    if (GameObject* evilCircle = instance->GetGameObject(evilCircleGuid))
                    {
                        evilCircle->SetForcedDespawn();
                        evilCircle->SetLootState(GO_JUST_DEACTIVATED);
                    }
                }
            }

            // Use combat doors
            DoUseDoorOrButton(GO_HAKKAR_DOOR_1);
            DoUseDoorOrButton(GO_HAKKAR_DOOR_2);

            m_auiEncounter[uiType] = uiData;

            break;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;

        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                   << m_auiEncounter[3] << " " << m_auiEncounter[4];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_sunken_temple::DoSpawnAtalarionIfCan()
{
    // Return if already summoned
    if (GetSingleCreatureFromStorage(NPC_ATALARION))
        return;

    Player* pPlayer = GetPlayerInMap();
    if (!pPlayer)
        return;

    pPlayer->SummonCreature(NPC_ATALARION, aSunkenTempleLocation[0].m_fX, aSunkenTempleLocation[0].m_fY, aSunkenTempleLocation[0].m_fZ, aSunkenTempleLocation[0].m_fO, TEMPSPAWN_DEAD_DESPAWN, 0);

    // Spawn the idol of Hakkar
    DoRespawnGameObject(GO_IDOL_OF_HAKKAR, 30 * MINUTE);

    // Spawn the big green lights
    for (GuidList::const_iterator itr = m_luiBigLightGUIDs.begin(); itr != m_luiBigLightGUIDs.end(); ++itr)
        DoRespawnGameObject(*itr, 30 * MINUTE);
}

bool instance_sunken_temple::ProcessStatueEvent(uint32 uiEventId)
{
    bool bEventStatus = false;

    // Check if the statues are activated correctly
    // Increase the counter when the correct statue is activated
    for (uint8 i = 0; i < MAX_STATUES; ++i)
    {
        if (uiEventId == m_aAtalaiStatueEvents[i] && m_uiStatueCounter == i)
        {
            // Right Statue activated
            ++m_uiStatueCounter;
            bEventStatus = true;
            break;
        }
    }

    if (!bEventStatus)
        return false;

    // Check if all statues are active
    if (m_uiStatueCounter == MAX_STATUES)
        SetData(TYPE_ATALARION, SPECIAL);

    return true;
}

void instance_sunken_temple::DoUpdateFlamesFlags(bool bRestore)
{
    for (GuidList::const_iterator itr = m_luiFlameGUIDs.begin(); itr != m_luiFlameGUIDs.end(); ++itr)
        DoToggleGameObjectFlags(*itr, GO_FLAG_NO_INTERACT, bRestore);
}

void instance_sunken_temple::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3] >> m_auiEncounter[4];

    for (uint32& i : m_auiEncounter)
    {
        // Here a bit custom, to have proper mechanics for the statue events
        if (i != DONE)
            i = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_sunken_temple::GetData(uint32 uiType) const
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    return 0;
}

void instance_sunken_temple::Update(uint32 uiDiff)
{
    if (m_auiEncounter[TYPE_AVATAR] != IN_PROGRESS)
        return;

    if (m_suppressionTimer)
    {
        if (m_suppressionTimer < uiDiff)
        {
            // At the end of the suppression timer, if Shade of Hakkar still has suppression aura : reset event
            if (Creature* shadeOfHakkar = GetSingleCreatureFromStorage(NPC_SHADE_OF_HAKKAR))
            {
                if (shadeOfHakkar->HasAura(SPELL_SUPPRESSION))
                {
                    DoScriptText(SAY_AVATAR_DESPAWN, shadeOfHakkar, nullptr);
                    SetData(TYPE_AVATAR, FAIL);
                }
            }
            m_suppressionTimer = 0;
        }
        else
            m_suppressionTimer -= uiDiff;
    }

    // Summon random mobs around the circles
    if (m_uiAvatarSummonTimer)
    {
        if (m_uiAvatarSummonTimer <= uiDiff)
        {
            Creature* pShade = GetSingleCreatureFromStorage(NPC_SHADE_OF_HAKKAR);
            if (!pShade)
                return;

            // If no summon circles are spawned then return
            if (m_evilCircleGuidList.empty())
                return;

            if (m_bIsFirstHakkarWave)                       // First wave summoned
            {
                // Summon at all circles
                for (GuidVector::const_iterator itr = m_evilCircleGuidList.begin(); itr != m_evilCircleGuidList.end(); ++itr)
                {
                    if (GameObject* pCircle = instance->GetGameObject(*itr))
                        pShade->SummonCreature(NPC_HAKKARI_MINION, pCircle->GetPositionX(), pCircle->GetPositionY(), pCircle->GetPositionZ(), 0, TEMPSPAWN_DEAD_DESPAWN, 0);
                }

                // Summon Bloodkeeper at random circle
                if (GameObject* pCircle = instance->GetGameObject(m_evilCircleGuidList[urand(0, m_evilCircleGuidList.size() - 1)]))
                    pShade->SummonCreature(NPC_BLOODKEEPER, pCircle->GetPositionX(), pCircle->GetPositionY(), pCircle->GetPositionZ(), 0, TEMPSPAWN_DEAD_DESPAWN, 0);

                m_bCanSummonBloodkeeper = false;
                m_bIsFirstHakkarWave = false;
                m_uiAvatarSummonTimer = 50000;
            }
            else                                            // Later wave
            {
                uint32 uiRoll = urand(0, 99);
                uint8 uiMaxSummons = uiRoll < 75 ? 1 : uiRoll < 95 ? 2 : 3;

                if (m_bCanSummonBloodkeeper && roll_chance_i(30))
                {
                    // Summon a Bloodkeeper
                    if (GameObject* pCircle = instance->GetGameObject(m_evilCircleGuidList[urand(0, m_evilCircleGuidList.size() - 1)]))
                        pShade->SummonCreature(NPC_BLOODKEEPER, pCircle->GetPositionX(), pCircle->GetPositionY(), pCircle->GetPositionZ(), 0, TEMPSPAWN_DEAD_DESPAWN, 0);

                    m_bCanSummonBloodkeeper = false;
                    --uiMaxSummons;
                }

                for (uint8 i = 0; i < uiMaxSummons; ++i)
                {
                    if (GameObject* pCircle = instance->GetGameObject(m_evilCircleGuidList[urand(0, m_evilCircleGuidList.size() - 1)]))
                        pShade->SummonCreature(NPC_HAKKARI_MINION, pCircle->GetPositionX(), pCircle->GetPositionY(), pCircle->GetPositionZ(), 0, TEMPSPAWN_DEAD_DESPAWN, 0);
                }
                m_uiAvatarSummonTimer = urand(3000, 15000);
            }
        }
        else
            m_uiAvatarSummonTimer -= uiDiff;
    }

    // Summon nightmare suppressor after flame used
    if (m_uiSupressorTimer)
    {
        if (m_uiSupressorTimer <= uiDiff)
        {
            Creature* pShade = GetSingleCreatureFromStorage(NPC_SHADE_OF_HAKKAR);
            if (!pShade)
            {
                // Something went very wrong!
                return;
            }

            // Summon npc at random door; movement and script handled in DB
            uint8 uiSummonLoc = urand(0, 1);
            if (Creature* suppressor = pShade->SummonCreature(NPC_SUPPRESSOR, aHakkariDoorLocations[uiSummonLoc].m_fX, aHakkariDoorLocations[uiSummonLoc].m_fY, aHakkariDoorLocations[uiSummonLoc].m_fZ, 0, TEMPSPAWN_DEAD_DESPAWN, 0))
            {
                suppressor->GetMotionMaster()->MoveWaypoint(uiSummonLoc);
                uint32 sayTextId;
                switch (urand(0, 3))
                {
                    case 0:
                        sayTextId = SAY_SUPPRESSOR_1;
                        break;
                    case 1:
                        sayTextId = SAY_SUPPRESSOR_2;
                        break;
                    case 2:
                        sayTextId = SAY_SUPPRESSOR_3;
                        break;
                    default:
                        sayTextId = SAY_SUPPRESSOR_4;
                        break;
                }
                DoScriptText(sayTextId, suppressor, nullptr);
            }

            // This timer is finished now
            m_uiSupressorTimer = 0;
        }
        else
            m_uiSupressorTimer -= uiDiff;
    }
}

InstanceData* GetInstanceData_instance_sunken_temple(Map* pMap)
{
    return new instance_sunken_temple(pMap);
}

void AddSC_instance_sunken_temple()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "instance_sunken_temple";
    pNewScript->GetInstanceData = &GetInstanceData_instance_sunken_temple;
    pNewScript->RegisterSelf();
}
