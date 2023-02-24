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
SDName: Instance_Stratholme
SD%Complete: 100
SDComment:
SDCategory: Stratholme
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "stratholme.h"

instance_stratholme::instance_stratholme(Map* map) : ScriptedInstance(map),
    m_baronRunTimer(0),
    m_barthilasRunTimer(0),
    m_mindlessSummonTimer(0),
    m_slaughterSquareTimer(0),
    m_slaughterDoorTimer(0),
    m_blackGuardsTimer(0),
    m_auriusSummonTimer(0),
    m_isSlaughterDoorOpen(false),
    m_yellCounter(0),
    m_mindlessCount(0),
    m_postboxesUsed(0),
    m_jarienKilled(false),
    m_sothosKilled(false)
{
    Initialize();
}

void instance_stratholme::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

    for (auto& m_uiGateTrapTimer : m_gateTrapTimers)
    {
        for (uint8 j = 0; j < 3; ++j)
            m_uiGateTrapTimer[j] = 0;
    }
}

void instance_stratholme::OnPlayerEnter(Player* player)
{
    // Baron ultimatum succeeded: summon Ysida outside the cage alive
    if (GetData(TYPE_BARON_RUN) == DONE)
        player->SummonCreature(NPC_YSIDA, stratholmeLocation[8].m_fX, stratholmeLocation[8].m_fY, stratholmeLocation[8].m_fZ, stratholmeLocation[8].m_fO, TEMPSPAWN_DEAD_DESPAWN, 0);
    // Baron ultimatum failed: summon Ysida outside the cage dead
    else if (GetData(TYPE_BARON_RUN) == FAIL)
    {
        if (Creature* ysida = player->SummonCreature(NPC_YSIDA, stratholmeLocation[8].m_fX, stratholmeLocation[8].m_fY, stratholmeLocation[8].m_fZ, stratholmeLocation[8].m_fO, TEMPSPAWN_DEAD_DESPAWN, 0))
            ysida->CastSpell(nullptr, 29266, TRIGGERED_OLD_TRIGGERED); // Guesswork - Feign Death
    }
}

// This function will prevent to lock players/monsters out/in
// the Slaughterhouse when toggling the door:
// this could happen when wiping before the end of the whole event
// The door will be toggled only if the current state is
// different of the one requested by open (true = open, false  = closed)
void instance_stratholme::DoOpenSlaughterhouseDoor(bool open)
{
    if (open != m_isSlaughterDoorOpen)
    {
        DoUseDoorOrButton(GO_ZIGGURAT_DOOR_4);
        m_isSlaughterDoorOpen = open;
    }
}

bool instance_stratholme::StartSlaughterSquare()
{
    if (m_auiEncounter[TYPE_BARONESS] == SPECIAL && m_auiEncounter[TYPE_NERUB] == SPECIAL && m_auiEncounter[TYPE_PALLID] == SPECIAL)
    {
        DoOrSimulateScriptTextForThisInstance(SAY_ANNOUNCE_RIVENDARE, NPC_BARON);

        DoUseDoorOrButton(GO_PORT_GAUNTLET);
        DoUseDoorOrButton(GO_PORT_SLAUGHTER);

        debug_log("SD2: Instance Stratholme: Open Slaughter square.");
        return true;
    }

    return false;
}

void instance_stratholme::OnCreatureCreate(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_AURIUS:
            creature->SetStandState(UNIT_STAND_STATE_KNEEL);
            if (m_auiEncounter[TYPE_AURIUS] == DONE)
                creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        // no break here
        case NPC_BARON:
        case NPC_YSIDA:
        case NPC_BARTHILAS:
            m_npcEntryGuidStore[creature->GetEntry()] = creature->GetObjectGuid();
            break;

        case NPC_CRYSTAL:
            m_crystalGUIDs.push_back(creature->GetObjectGuid());
            break;
        case NPC_ABOM_BILE:
        case NPC_ABOM_VENOM:
            m_abominationGUIDs.insert(creature->GetObjectGuid());
            break;
        case NPC_THUZADIN_ACOLYTE:
            m_acolyteGUIDs.push_back(creature->GetObjectGuid());
            break;
        case NPC_CRIMSON_INITIATE:
        case NPC_CRIMSON_GALLANT:
        case NPC_CRIMSON_GUARDSMAN:
        case NPC_CRIMSON_CONJURER:
        case NPC_CRIMSON_BATTLE_MAGE:
            // Store GUID of NPCs in the courtyard to spawn Timmy once they are all dead
            if (creature->IsWithinDist2d(defensePoints[TIMMY].m_fX, defensePoints[TIMMY].m_fY, 40.0f))
                m_crimsonDefendersLowGuids[TIMMY].push_back(creature->GetObjectGuid());
            // Iterate also over all the defense points where those NPCs are possibly spawned
            for (uint8 i = BARRICADE; i <= FIRST_BARRICADES; i++)
            {
                // Do not store - again - GUIDs for Timmy spawn point, they were done previously with a different range
                if (i == TIMMY)
                    continue;
                // Store the GUID of the nearby NPCs for each defense point
                if (creature->IsWithinDist2d(defensePoints[i].m_fX, defensePoints[i].m_fY, 8.0f))
                {
                    m_crimsonDefendersLowGuids[i].push_back(creature->GetObjectGuid());
                    break;
                }
            }
            break;
    }
}

void instance_stratholme::OnObjectCreate(GameObject* go)
{
    switch (go->GetEntry())
    {
        case GO_SERVICE_ENTRANCE:
            break;
        case GO_GAUNTLET_GATE1:
            // TODO
            // weird, but unless flag is set, client will not respond as expected. DB bug?
            go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED);
            break;

        case GO_ZIGGURAT_DOOR_1:
            m_zigguratStorage[0].m_doorGuid = go->GetObjectGuid();
            if (m_auiEncounter[TYPE_BARONESS] == DONE || m_auiEncounter[TYPE_BARONESS] == SPECIAL)
                go->SetGoState(GO_STATE_ACTIVE);
            return;
        case GO_ZIGGURAT_DOOR_2:
            m_zigguratStorage[1].m_doorGuid = go->GetObjectGuid();
            if (m_auiEncounter[TYPE_NERUB] == DONE || m_auiEncounter[TYPE_NERUB] == SPECIAL)
                go->SetGoState(GO_STATE_ACTIVE);
            return;
        case GO_ZIGGURAT_DOOR_3:
            m_zigguratStorage[2].m_doorGuid = go->GetObjectGuid();
            if (m_auiEncounter[TYPE_PALLID] == DONE || m_auiEncounter[TYPE_PALLID] == SPECIAL)
                go->SetGoState(GO_STATE_ACTIVE);
            return;

        case GO_ZIGGURAT_DOOR_4:
            if (m_auiEncounter[TYPE_RAMSTEIN] == DONE || m_auiEncounter[TYPE_BLACK_GUARDS] == DONE)
                go->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_ZIGGURAT_DOOR_5:
            if (m_auiEncounter[TYPE_BLACK_GUARDS] == DONE)
                go->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PORT_GAUNTLET:
        case GO_PORT_SLAUGHTER:
            if (m_auiEncounter[TYPE_BARONESS] == SPECIAL && m_auiEncounter[TYPE_NERUB] == SPECIAL && m_auiEncounter[TYPE_PALLID] == SPECIAL)
                go->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PORT_SLAUGHTER_GATE:
            if (m_auiEncounter[TYPE_RAMSTEIN] == DONE)      // Might actually be unneeded
                go->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PORT_ELDERS:
        case GO_YSIDA_CAGE:
        case GO_PORT_TRAP_GATE_1:
        case GO_PORT_TRAP_GATE_2:
        case GO_PORT_TRAP_GATE_3:
        case GO_PORT_TRAP_GATE_4:
            break;

        default:
            return;
    }
    m_goEntryGuidStore[go->GetEntry()] = go->GetObjectGuid();
}

void instance_stratholme::SetData(uint32 type, uint32 data)
{
    // TODO: Remove the hard-coded indexes from array accessing
    switch (type)
    {
        case TYPE_BARON_RUN:
            switch (data)
            {
                case IN_PROGRESS:
                    if (m_auiEncounter[type] == IN_PROGRESS || m_auiEncounter[type] == FAIL)
                        break;

                    // Baron ultimatum starts: summon Ysida in the cage
                    if (Creature* pBaron = GetSingleCreatureFromStorage(NPC_BARON))
                    {
                        DoOrSimulateScriptTextForThisInstance(SAY_ANNOUNCE_RUN_START, NPC_BARON);
                        if (Creature* pYsida = pBaron->SummonCreature(NPC_YSIDA, stratholmeLocation[7].m_fX, stratholmeLocation[7].m_fY, stratholmeLocation[7].m_fZ, stratholmeLocation[7].m_fO, TEMPSPAWN_DEAD_DESPAWN, 0))
                            pYsida->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
                    }

                    m_baronRunTimer = 45 * MINUTE * IN_MILLISECONDS;
                    debug_log("SD2: Instance Stratholme: Baron run in progress.");
                    break;
                case FAIL:
                    // may add code to remove aura from players, but in theory the time should be up already and removed.
                    break;
                case DONE:
                    m_baronRunTimer = 0;
                    break;
                default:
                    break;
            }
            m_auiEncounter[type] = data;
            break;
        case TYPE_BARONESS:
        case TYPE_NERUB:
        case TYPE_PALLID:
            m_auiEncounter[type] = data;
            if (data == DONE)
            {
                DoSortZiggurats();
                DoUseDoorOrButton(m_zigguratStorage[type - TYPE_BARONESS].m_doorGuid);
            }
            if (data == SPECIAL)
                StartSlaughterSquare();
            break;
        case TYPE_RAMSTEIN:
            if (data == SPECIAL)
            {
                if (m_auiEncounter[type] != SPECIAL && m_auiEncounter[type] != DONE)
                {
                    m_slaughterSquareTimer = 20 * IN_MILLISECONDS;       // TODO - unknown, also possible that this is not the very correct place..
                    DoUseDoorOrButton(GO_PORT_GAUNTLET);
                }

                uint32 abomCount = m_abominationGUIDs.size();
                for (auto abomGuid : m_abominationGUIDs)
                {
                    if (Creature* abom = instance->GetCreature(abomGuid))
                    {
                        if (!abom->IsAlive())
                            --abomCount;
                    }
                    else
                    {
                        // Remove obsolete guid from set and decrement count
                        m_abominationGUIDs.erase(abomGuid);
                        --abomCount;
                    }
                }

                if (!abomCount)
                {
                    // Open the Slaughterhouse door and set a timer to close it after 10 sec to let some time to Ramstein to move out
                    DoOpenSlaughterhouseDoor(true);
                    m_slaughterDoorTimer = 10 * IN_MILLISECONDS;

                    // No more handlng of Abominations
                    m_slaughterSquareTimer = 0;

                    if (Creature* baron = GetSingleCreatureFromStorage(NPC_BARON))
                    {
                        DoScriptText(SAY_ANNOUNCE_RAMSTEIN, baron);
                        if (Creature* ramstein = baron->SummonCreature(NPC_RAMSTEIN, stratholmeLocation[2].m_fX, stratholmeLocation[2].m_fY, stratholmeLocation[2].m_fZ, stratholmeLocation[2].m_fO, TEMPSPAWN_DEAD_DESPAWN, 0))
                            ramstein->GetMotionMaster()->MovePoint(0, stratholmeLocation[5].m_fX, stratholmeLocation[5].m_fY, stratholmeLocation[5].m_fZ);

                        debug_log("SD2: Instance Stratholme - Slaughter event: Ramstein spawned.");
                    }
                }
                else
                    debug_log("SD2: Instance Stratholme - Slaughter event: %u Abomination left to kill.", abomCount);
            }
            // After fail aggroing Ramstein means wipe on Ramstein, so close door again
            if (data == IN_PROGRESS && m_auiEncounter[type] == FAIL)
                DoUseDoorOrButton(GO_PORT_GAUNTLET);
            if (data == DONE)
            {
                // Open side gate and start summoning skeletons
                DoUseDoorOrButton(GO_PORT_SLAUGHTER_GATE);
                // Close the Slaughterhouse door in case it was open again after a wipe on Ramstein
                DoOpenSlaughterhouseDoor(false);
                // use this timer as a bool just to start summoning
                m_mindlessSummonTimer = 500;
                m_mindlessCount = 0;
                m_undeadGUIDs.clear();

                // Summon 5 guards
                if (Creature* baron = GetSingleCreatureFromStorage(NPC_BARON))
                {
                    for (uint8 i = 0; i < 5; ++i)
                    {
                        float fX, fY, fZ;
                        baron->GetRandomPoint(stratholmeLocation[6].m_fX, stratholmeLocation[6].m_fY, stratholmeLocation[6].m_fZ, 5.0f, fX, fY, fZ);
                        if (Creature* temp = baron->SummonCreature(NPC_BLACK_GUARD, stratholmeLocation[6].m_fX, stratholmeLocation[6].m_fY, stratholmeLocation[6].m_fZ, stratholmeLocation[6].m_fO, TEMPSPAWN_DEAD_DESPAWN, 0))
                            m_guardGUIDs.push_back(temp->GetObjectGuid());
                    }

                    debug_log("SD2: Instance Stratholme - Slaughter event: Summoned 5 guards.");
                }
            }
            // Open Door again and stop Abomination
            if (data == FAIL && m_auiEncounter[type] != FAIL)
            {
                DoUseDoorOrButton(GO_PORT_GAUNTLET);
                m_slaughterSquareTimer = 0;

                // Let already moving Abominations stop
                for (auto abomGuid : m_abominationGUIDs)
                {
                    Creature* abom = instance->GetCreature(abomGuid);
                    if (abom && abom->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
                        abom->GetMotionMaster()->MovementExpired();
                }
            }

            m_auiEncounter[type] = data;
            break;
        case TYPE_BARON:
            if (data == IN_PROGRESS)
            {
                DoUseOpenableObject(GO_ZIGGURAT_DOOR_5, false);

                // Close Slaughterhouse door if needed
                if (m_auiEncounter[type] == FAIL)
                    DoUseOpenableObject(GO_PORT_GAUNTLET, false);

                // If Aurius was given the medaillon wait 5s before summoning him
                if (GetData(TYPE_AURIUS) == DONE)
                    m_auriusSummonTimer = 5 * IN_MILLISECONDS;
            }
            else if (data == DONE)
            {
                // Players successfully engaged Baron within the time-limit of his ultimatum
                //     Note: UpdateAI() prevents TYPE_BARON_RUN to be marked as FAILED if the
                //     Baron is already engaged (in progress) when the ultimatum timer expires
                if (m_auiEncounter[TYPE_BARON_RUN] == IN_PROGRESS)
                {
                    SetData(TYPE_BARON_RUN, DONE);
                    Map::PlayerList const& players = instance->GetPlayers();

                    for (const auto& player : players)
                    {
                        if (Player* sourcePlayer = player.getSource())
                        {
                            if (sourcePlayer->HasAura(SPELL_BARON_ULTIMATUM))
                                sourcePlayer->RemoveAurasDueToSpell(SPELL_BARON_ULTIMATUM);

                            if (sourcePlayer->GetQuestStatus(QUEST_DEAD_MAN_PLEA) == QUEST_STATUS_INCOMPLETE)
                                sourcePlayer->KilledMonsterCredit(NPC_YSIDA);

                            // Argent Dawn reputation reward
                            sourcePlayer->CastSpell(sourcePlayer, SPELL_YSIDA_FREED, TRIGGERED_OLD_TRIGGERED);
                        }
                    }

                    // Open cage, finish rescue event
                    if (Creature* ysida = GetSingleCreatureFromStorage(NPC_YSIDA))
                    {
                        DoScriptText(SAY_EPILOGUE, ysida);
                        DoUseDoorOrButton(GO_YSIDA_CAGE);
                        ysida->GetMotionMaster()->MovePoint(0, stratholmeLocation[8].m_fX, stratholmeLocation[8].m_fY, stratholmeLocation[8].m_fZ);
                        ysida->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
                    }
                }

                // If Aurius was spawned to help fight the Baron
                if (GetData(TYPE_AURIUS) == DONE)
                {
                    // Baron killed and Aurius is alive: give him his NPC Flags back, so players can complete the quest and fake his death
                    if (Creature* aurius = GetSingleCreatureFromStorage(NPC_AURIUS))
                    {
                        if (aurius->IsAlive())
                        {
                            DoScriptText(SAY_AURIUS_DEATH, aurius);
                            aurius->StopMoving();
                            aurius->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
                            aurius->InterruptNonMeleeSpells(true);
                            aurius->SetHealth(1);
                            aurius->GetMotionMaster()->MovementExpired();
                            aurius->GetMotionMaster()->MoveIdle();
                            aurius->RemoveAllAurasOnDeath();
                            aurius->SetStandState(UNIT_STAND_STATE_DEAD);
                        }
                    }
                }
            }

            if (data != IN_PROGRESS)
            {
                // Open Slaughterhouse door again
                DoUseOpenableObject(GO_PORT_GAUNTLET, true);
                // Open Baron door
                DoUseOpenableObject(GO_ZIGGURAT_DOOR_5, true);
            }

            m_auiEncounter[type] = data;
            break;
        case TYPE_BARTHILAS_RUN:
            if (data == IN_PROGRESS)
            {
                Creature* barthilas = GetSingleCreatureFromStorage(NPC_BARTHILAS);
                if (barthilas && barthilas->IsAlive() && !barthilas->IsInCombat())
                {
                    DoScriptText(SAY_WARN_BARON, barthilas);
                    barthilas->SetWalk(false);
                    barthilas->GetMotionMaster()->MovePoint(0, stratholmeLocation[0].m_fX, stratholmeLocation[0].m_fY, stratholmeLocation[0].m_fZ);

                    m_barthilasRunTimer = 8 * IN_MILLISECONDS;
                }
            }
            m_auiEncounter[type] = data;
            break;
        case TYPE_BLACK_GUARDS:
            // Prevent double action
            if (m_auiEncounter[type] == data)
                return;

            // Restart after failure, close Gauntlet
            if (data == IN_PROGRESS && m_auiEncounter[type] == FAIL)
                DoUseDoorOrButton(GO_PORT_GAUNTLET);
            // Wipe case - open gauntlet
            if (data == FAIL)
                DoUseDoorOrButton(GO_PORT_GAUNTLET);
            if (data == DONE)
            {
                if (Creature* baron = GetSingleCreatureFromStorage(NPC_BARON))
                    DoScriptText(SAY_UNDEAD_DEFEAT, baron);
                DoUseDoorOrButton(GO_ZIGGURAT_DOOR_5);
            }
            m_auiEncounter[type] = data;

            // No need to save anything here, so return
            return;
        case TYPE_POSTMASTER:
            m_auiEncounter[type] = data;
            if (data == IN_PROGRESS)
            {
                ++m_postboxesUsed;

                // After the second post box prepare to spawn the Post Master
                if (m_postboxesUsed == 2)
                    SetData(TYPE_POSTMASTER, SPECIAL);
            }
            // No need to save anything here, so return
            return;
        case TYPE_AURIUS:
            // Prevent further players to complete the quest in that instance or autocomplete the follow-up quest. the flag will be set back if event is succeed
            if (data == DONE)
            {
                if (Creature* aurius = GetSingleCreatureFromStorage(NPC_AURIUS))
                    aurius->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            }
            m_auiEncounter[type] = data;
            break;
        default:
            m_auiEncounter[type] = data;
            break;
    }

    if (data == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                   << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " "
                   << m_auiEncounter[6] << " " << m_auiEncounter[7];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_stratholme::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
               >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7];

    for (uint32& i : m_auiEncounter)
    {
        if (i == IN_PROGRESS)
            i = NOT_STARTED;
    }

    // Special Treatment for the Ziggurat-Bosses, as otherwise the event couldn't reload
    if (m_auiEncounter[TYPE_BARONESS] == DONE)
        m_auiEncounter[TYPE_BARONESS] = SPECIAL;
    if (m_auiEncounter[TYPE_NERUB] == DONE)
        m_auiEncounter[TYPE_NERUB] = SPECIAL;
    if (m_auiEncounter[TYPE_PALLID] == DONE)
        m_auiEncounter[TYPE_PALLID] = SPECIAL;

    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_stratholme::GetData(uint32 type) const
{
    switch (type)
    {
        case TYPE_BARON_RUN:
        case TYPE_BARONESS:
        case TYPE_NERUB:
        case TYPE_PALLID:
        case TYPE_RAMSTEIN:
        case TYPE_BARON:
        case TYPE_AURIUS:
        case TYPE_BARTHILAS_RUN:
        case TYPE_POSTMASTER:
            return m_auiEncounter[type];
        default:
            return 0;
    }
}

static bool sortByHeight(Creature* first, Creature* second)
{
    return first && second && first->GetPositionZ() > second->GetPositionZ();
}

void instance_stratholme::DoSortZiggurats()
{
    if (m_acolyteGUIDs.empty())
        return;

    CreatureList acolytesList;                         // Valid pointers, only used locally
    for (auto acolyteGUID : m_acolyteGUIDs)
    {
        if (Creature* acolyte = instance->GetCreature(acolyteGUID))
            acolytesList.push_back(acolyte);
    }
    m_acolyteGUIDs.clear();

    if (acolytesList.empty())
        return;

    if (!GetSingleCreatureFromStorage(NPC_THUZADIN_ACOLYTE, true))
    {
        // Sort the acolytes by height, and the one with the biggest height is the announcer (a bit outside the map)
        acolytesList.sort(sortByHeight);
        m_npcEntryGuidStore[NPC_THUZADIN_ACOLYTE] = (*acolytesList.begin())->GetObjectGuid();
        acolytesList.erase(acolytesList.begin());
    }

    // Sort Acolytes
    for (auto itr = acolytesList.begin(); itr != acolytesList.end();)
    {
        bool isAlreadyIterated = false;
        for (auto& i : m_zigguratStorage)
        {
            if (GameObject* pZigguratDoor = instance->GetGameObject(i.m_doorGuid))
            {
                if ((*itr)->IsAlive() && (*itr)->IsWithinDistInMap(pZigguratDoor, 35.0f, false))
                {
                    i.m_zigguratAcolyteGuid.push_back((*itr)->GetObjectGuid());
                    itr = acolytesList.erase(itr);
                    isAlreadyIterated = true;
                    break;
                }
            }
        }

        if (itr != acolytesList.end() && !isAlreadyIterated)
            ++itr;
    }

    // In case some mobs have not been able to be sorted, store their GUIDs again
    for (CreatureList::const_iterator itr = acolytesList.begin(); itr != acolytesList.end(); ++itr)
        m_acolyteGUIDs.push_back((*itr)->GetObjectGuid());

    // Sort Crystal
    for (auto itr = m_crystalGUIDs.begin(); itr != m_crystalGUIDs.end();)
    {
        Creature* crystal = instance->GetCreature(*itr);
        if (!crystal)
        {
            itr = m_crystalGUIDs.erase(itr);
            continue;
        }

        bool isAlreadyIterated = false;
        for (auto& i : m_zigguratStorage)
        {
            if (GameObject* pZigguratDoor = instance->GetGameObject(i.m_doorGuid))
            {
                if (crystal->IsWithinDistInMap(pZigguratDoor, 50.0f, false))
                {
                    i.m_crystalGuid = crystal->GetObjectGuid();
                    itr = m_crystalGUIDs.erase(itr);
                    isAlreadyIterated = true;
                    break;
                }
            }
        }

        if (itr != m_crystalGUIDs.end() && !isAlreadyIterated)
            ++itr;
    }
}

void instance_stratholme::OnCreatureEnterCombat(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_BARONESS_ANASTARI: SetData(TYPE_BARONESS, IN_PROGRESS); break;
        case NPC_MALEKI_THE_PALLID: SetData(TYPE_PALLID, IN_PROGRESS);   break;
        case NPC_NERUBENKAN:        SetData(TYPE_NERUB, IN_PROGRESS);    break;
        case NPC_RAMSTEIN:          SetData(TYPE_RAMSTEIN, IN_PROGRESS); break;
        case NPC_BARON:             SetData(TYPE_BARON, IN_PROGRESS);    break;

        case NPC_ABOM_BILE:
        case NPC_ABOM_VENOM:
            // Start Slaughterhouse Event
            SetData(TYPE_RAMSTEIN, SPECIAL);
            break;

        case NPC_MINDLESS_UNDEAD:
        case NPC_BLACK_GUARD:
            // Aggro in Slaughterhouse after Ramstein
            SetData(TYPE_BLACK_GUARDS, IN_PROGRESS);
            break;
    }
}

void instance_stratholme::OnCreatureEvade(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_BARONESS_ANASTARI: SetData(TYPE_BARONESS, FAIL); break;
        case NPC_MALEKI_THE_PALLID: SetData(TYPE_PALLID, FAIL);   break;
        case NPC_NERUBENKAN:        SetData(TYPE_NERUB, FAIL);    break;
        case NPC_BARON:             SetData(TYPE_BARON, FAIL);    break;

        case NPC_RAMSTEIN:
            SetData(TYPE_RAMSTEIN, FAIL);
            // Open Slaughterhouse door again because Ramstein will move back in
            DoOpenSlaughterhouseDoor(true);
            break;
        case NPC_ABOM_BILE:
        case NPC_ABOM_VENOM:
            // Fail in Slaughterhouse Event before Ramstein
            SetData(TYPE_RAMSTEIN, FAIL);
            break;
        case NPC_MINDLESS_UNDEAD:
        case NPC_BLACK_GUARD:
            // Fail in Slaughterhouse after Ramstein
            SetData(TYPE_BLACK_GUARDS, FAIL);
            break;
    }
}

void instance_stratholme::OnCreatureDeath(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_BARONESS_ANASTARI: SetData(TYPE_BARONESS, DONE); break;
        case NPC_MALEKI_THE_PALLID: SetData(TYPE_PALLID, DONE);   break;
        case NPC_NERUBENKAN:        SetData(TYPE_NERUB, DONE);    break;
        case NPC_RAMSTEIN:          SetData(TYPE_RAMSTEIN, DONE); break;
        case NPC_BARON:             SetData(TYPE_BARON, DONE);    break;

        case NPC_AURIUS:
            DoScriptText(SAY_AURIUS_DEATH, creature);
            break;

        case NPC_THUZADIN_ACOLYTE:
            ThazudinAcolyteJustDied(creature);
            break;

        case NPC_ABOM_BILE:
        case NPC_ABOM_VENOM:
            // Start Slaughterhouse Event
            SetData(TYPE_RAMSTEIN, SPECIAL);
            break;

        case NPC_MINDLESS_UNDEAD:
            m_undeadGUIDs.remove(creature->GetObjectGuid());
            if (m_undeadGUIDs.empty())
                m_blackGuardsTimer = 60 * IN_MILLISECONDS; // Wait 1 min before sending the Black Guards outside. In Classic, this allows players to resplenish health and mana
            break;
        case NPC_BLACK_GUARD:
            m_guardGUIDs.remove(creature->GetObjectGuid());
            if (m_guardGUIDs.empty())
                SetData(TYPE_BLACK_GUARDS, DONE);
            break;

        // Scarlet Bastion defense and Timmy spawn support
        case NPC_CRIMSON_INITIATE:
        case NPC_CRIMSON_GALLANT:
        case NPC_CRIMSON_GUARDSMAN:
        case NPC_CRIMSON_CONJURER:
        case NPC_CRIMSON_BATTLE_MAGE:
            for (uint8 i = BARRICADE; i <= FIRST_BARRICADES; i++)
            {
                if (m_crimsonDefendersLowGuids[i].empty())
                    continue;

                m_crimsonDefendersLowGuids[i].remove(creature->GetObjectGuid());
                // If all mobs from a defense group are dead then activate the related defense event
                if (m_crimsonDefendersLowGuids[i].empty() && i != FIRST_BARRICADES)
                    DoScarletBastionDefense(i, creature);
            }
            break;
        case NPC_BALNAZZAR:
            DoScarletBastionDefense(CRIMSON_THRONE, creature);
            break;
        case NPC_JARIEN:
            m_jarienKilled = true;
            if (m_sothosKilled)
                if (Unit* spawner = creature->GetSpawner())
                    spawner->CastSpell(nullptr, SPELL_SUMMON_WINNER_BOX, TRIGGERED_OLD_TRIGGERED);
            break;
        case NPC_SOTHOS:
            m_sothosKilled = true;
            if (m_jarienKilled)
                if (Unit* spawner = creature->GetSpawner())
                    spawner->CastSpell(nullptr, SPELL_SUMMON_WINNER_BOX, TRIGGERED_OLD_TRIGGERED);
            break;
    }
}

void instance_stratholme::OnCreatureRespawn(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_BARTHILAS:
            if (GetData(TYPE_BARTHILAS_RUN) != NOT_STARTED)
                creature->NearTeleportTo(stratholmeLocation[1].m_fX, stratholmeLocation[1].m_fY, stratholmeLocation[1].m_fZ, stratholmeLocation[1].m_fO);
            break;
        case NPC_JARIEN:
            m_jarienKilled = false;
            break;
        case NPC_SOTHOS:
            m_sothosKilled = false;
            break;
    }
}

void instance_stratholme::ThazudinAcolyteJustDied(Creature* creature)
{
    for (uint8 i = 0; i < MAX_ZIGGURATS; ++i)
    {
        if (m_zigguratStorage[i].m_zigguratAcolyteGuid.empty())
            continue;                               // nothing to do anymore for this ziggurat

        m_zigguratStorage[i].m_zigguratAcolyteGuid.remove(creature->GetObjectGuid());
        if (m_zigguratStorage[i].m_zigguratAcolyteGuid.empty())
        {
            // A random zone yell after one is cleared
            int32 aAnnounceSay[MAX_ZIGGURATS] = {SAY_ANNOUNCE_ZIGGURAT_1, SAY_ANNOUNCE_ZIGGURAT_2, SAY_ANNOUNCE_ZIGGURAT_3};
            DoOrSimulateScriptTextForThisInstance(aAnnounceSay[i], NPC_THUZADIN_ACOLYTE);

            // Kill Crystal
            if (Creature* crystal = instance->GetCreature(m_zigguratStorage[i].m_crystalGuid))
                crystal->CastSpell(nullptr, SPELL_SUICIDE, TRIGGERED_OLD_TRIGGERED);

            switch (i)
            {
                case 0: SetData(TYPE_BARONESS, SPECIAL); break;
                case 1: SetData(TYPE_NERUB, SPECIAL);    break;
                case 2: SetData(TYPE_PALLID, SPECIAL);   break;
                default: return;
            }
        }
    }
}

void instance_stratholme::DoSpawnScarletGuards(uint8 step, Player* summoner)
{
    if (!summoner)
        return;

    uint32 npcEntry[2];
    uint8  index;

    switch (step)
    {
        case HALL_OF_LIGHTS:
            npcEntry[0] = NPC_CRIMSON_GALLANT;
            npcEntry[1] = NPC_CRIMSON_GALLANT;
            index = 0;
            break;
        case INNER_BASTION_2:
            npcEntry[0] = NPC_CRIMSON_MONK;
            npcEntry[1] = NPC_CRIMSON_SORCERER;
            index = 3;
            break;
        default:
            return;    // avoid indexing the following tables with a wrong index. Should never happen.
    }

    // Spawn the two guards and move them to where they will guard (each side of a door)
    for (uint8 i = 0; i < 2; i++)
    {
        if (Creature* temp = summoner->SummonCreature(npcEntry[i], scarletGuards[index].m_fX, scarletGuards[index].m_fY, scarletGuards[index].m_fZ, scarletGuards[index].m_fO, TEMPSPAWN_DEAD_DESPAWN, 0))
        {
            temp->SetWalk(false);
            temp->GetMotionMaster()->MovePoint(0, scarletGuards[index + i + 1].m_fX, scarletGuards[index + i + 1].m_fY, scarletGuards[index + i + 1].m_fZ);
        }
    }
}

void instance_stratholme::DoSpawnScourgeInvaders(uint8 step, Player* summoner)
{
    if (!summoner)
        return;

    // Define the group of 5 Scourge invaders
    std::vector<uint32> mobList;                  // Vector holding the 5 creatures entries for each Scourge invaders group
    mobList.push_back(NPC_SKELETAL_GUARDIAN);     // 4 static NPC entries
    mobList.push_back(NPC_SKELETAL_GUARDIAN);
    mobList.push_back(NPC_SKELETAL_BERSERKER);
    mobList.push_back(NPC_SKELETAL_BERSERKER);

    uint32 mobEntry;                              // will hold the last random creature entry
    uint8  index;

    // Pick the fifth NPC in the group and randomize the five possible spawns
    switch (urand(0, 1))
    {
        case 0: mobEntry = NPC_SKELETAL_GUARDIAN;     break;
        case 1: mobEntry = NPC_SKELETAL_BERSERKER;    break;
    }

    mobList.push_back(mobEntry);
    std::shuffle(mobList.begin(), mobList.end(), *GetRandomGenerator());

    // Define the correct index for the spawn/move coords table
    switch (step)
    {
        case ENTRANCE:        index = 1; break;
        case INNER_BASTION_1: index = 3; break;
        case CRIMSON_THRONE:  index = 5; break;
        default: return;    // avoid indexing the following table with a wrong index. Should never happen.
    }

    // Summon the five invaders and make them run into the room
    for (uint8 i = 0; i < 5; i++)
    {
        float targetPosX, targetPosY, targetPosZ;

        if (Creature* temp = summoner->SummonCreature(mobList[i], scourgeInvaders[index].m_fX, scourgeInvaders[index].m_fY, scourgeInvaders[index].m_fZ, scourgeInvaders[index].m_fO, TEMPSPAWN_DEAD_DESPAWN, 0))
        {
            temp->SetWalk(false);
            temp->GetRandomPoint(scourgeInvaders[index + 1].m_fX, scourgeInvaders[index + 1].m_fY, scourgeInvaders[index + 1].m_fZ, 6.0f, targetPosX, targetPosY, targetPosZ);
            temp->GetMotionMaster()->MovePoint(0, targetPosX, targetPosY, targetPosZ);
        }
    }
}

void instance_stratholme::DoMoveBackDefenders(uint8 step, Creature* creature)
{
    uint8 index;
    uint8 treshold = 0;
    uint8 foundGuards = 0;

    switch (step)
    {
        case BARRICADE:
            index = FIRST_BARRICADES;
            break;
        case STAIRS:
            index = BARRICADE;
            treshold = 3;
            break;
        default:
            return;     // avoid indexing the following table with a wrong index. Should never happen.
    }

    // Check that there are still defenders to move to the stairs/last barricade
    if (m_crimsonDefendersLowGuids[index].empty())
        return;
    if (creature)
        DoScriptText(ScarletEventYells[step], creature);

    for (auto guardGUID : m_crimsonDefendersLowGuids[index])
    {
        Creature* guard = instance->GetCreature(guardGUID);
        if (guard && guard->IsAlive() && !guard->IsInCombat())
        {
            guard->GetMotionMaster()->MoveIdle();
            guard->SetWalk(false);
            guard->GetMotionMaster()->MovePoint(0, scarletLastStand[treshold + foundGuards].m_fX, scarletLastStand[treshold + foundGuards].m_fY, scarletLastStand[treshold + foundGuards].m_fZ);
            foundGuards++;
        }

        if (foundGuards == 3)
            return;
    }
}

void instance_stratholme::DoScarletBastionDefense(uint8 step, Creature* creature)
{
    if (!creature)
        return;

    switch (step)
    {
        case BARRICADE:
        case STAIRS:
            DoMoveBackDefenders(step, creature);
            return;
        case TIMMY:
            creature->SummonCreature(NPC_TIMMY_THE_CRUEL, scourgeInvaders[0].m_fX, scourgeInvaders[0].m_fY, scourgeInvaders[0].m_fZ, scourgeInvaders[0].m_fO, TEMPSPAWN_DEAD_DESPAWN, 0);
            return;
        // Scarlet guards spawned
        case HALL_OF_LIGHTS:
        case INNER_BASTION_2:
            DoScriptText(ScarletEventYells[step], creature);
            if (Player* player = GetPlayerInMap())
                DoSpawnScarletGuards(step, player);
            return;
        // Scourge invading
        case ENTRANCE:
        case INNER_BASTION_1:
            DoScriptText(ScarletEventYells[step], creature);
        case CRIMSON_THRONE:
            if (Player* player = GetPlayerInMap())
                DoSpawnScourgeInvaders(step, player);
        default:
            return;
    }
}

void instance_stratholme::DoGateTrap(uint8 gate)
{
    // Check if timer was not already set by another player/pet a few milliseconds before
    if (m_gateTrapTimers[gate][0])
        return;

    debug_log("SD2: Instance Stratholme - Rat Trap activated %i.", gate);
    // close the gates
    DoUseDoorOrButton(gates[2 * gate]);
    DoUseDoorOrButton(gates[2 * gate + 1]);

    // set timer to reset the trap
    m_gateTrapTimers[gate][0] = 30 * MINUTE * IN_MILLISECONDS;
    // set timer to reopen gates
    m_gateTrapTimers[gate][1] = 20 * IN_MILLISECONDS;
    // set timer to spawn the plagued critters
    m_gateTrapTimers[gate][2] = 2 * IN_MILLISECONDS;
}

void instance_stratholme::DoSpawnPlaguedCritters(uint8 gate, Player* player)
{
    if (!player)
        return;

    uint32 entry = plaguedCritters[urand(0, 2)];
    for (uint8 i = 0; i < 30; ++i)
    {
        float fX, fY, fZ;
        player->GetRandomPoint(gateTrap[gate].m_fX, gateTrap[gate].m_fY, gateTrap[gate].m_fZ, 8.0f, fX, fY, fZ);
        if (Creature* critter = player->SummonCreature(entry, fX, fY, fZ, 0, TEMPSPAWN_DEAD_DESPAWN, 0))
            critter->Attack(player, true);
    }
}

void instance_stratholme::Update(uint32 diff)
{
    // Loop over the two Gate traps, each one has up to three timers (trap reset, gate opening delay, critters spawning delay)
    for (uint8 i = 0; i < 2; i++)
    {
        // Check that the trap is not on cooldown, if so check if player/pet is in range
        if (m_gateTrapTimers[i][0])
        {
            m_gateTrapTimers[i][0] -= diff;
            if (m_gateTrapTimers[i][0] <= diff)
            {
                debug_log("SD2: Instance Stratholme - Rat Trap reseted %u.", i);
                m_gateTrapTimers[i][0] = 0;
            }
        }
        else
        {
            Map::PlayerList const& players = instance->GetPlayers();
            for (const auto& player : players)
            {
                if (Player* playerSource = player.getSource())
                {
                    if (!playerSource->IsGameMaster() && playerSource->IsWithinDist2d(gateTrap[i].m_fX, gateTrap[i].m_fY, 5.5f))
                        DoGateTrap(i);

                    Pet* pet = playerSource->GetPet();
                    if (!playerSource->IsGameMaster() && pet && pet->IsWithinDist2d(gateTrap[i].m_fX, gateTrap[i].m_fY, 5.5f))
                        DoGateTrap(i);
                }
            }
        }
        // Timer to reopen the gates
        if (m_gateTrapTimers[i][1])
        {
            if (m_gateTrapTimers[i][1] <= diff)
            {
                DoUseDoorOrButton(gates[2 * i]);
                DoUseDoorOrButton(gates[2 * i + 1]);
                m_gateTrapTimers[i][1] = 0;
            }
            else
                m_gateTrapTimers[i][1] -= diff;
        }
        // Delay timer to spawn the plagued critters once the gate are closing
        if (m_gateTrapTimers[i][2])
        {
            if (m_gateTrapTimers[i][2] <= diff)
            {
                if (Player* player = GetPlayerInMap())
                    DoSpawnPlaguedCritters(i, player);
                m_gateTrapTimers[i][2] = 0;
            }
            else
                m_gateTrapTimers[i][2] -= diff;
        }
    }

    // Timer to send the Black Guard Sentries out of the Slaughterhouse before Baron Rivendare
    if (m_blackGuardsTimer)
    {
        if (m_blackGuardsTimer <= diff)
        {
            // Open the Slaughterhouse door to let the 5 Black Guards to move out
            DoOpenSlaughterhouseDoor(true);

            for (auto guardGUID : m_guardGUIDs)
            {
                Creature* guard = instance->GetCreature(guardGUID);
                if (guard && guard->IsAlive() && !guard->IsInCombat())
                {
                    float fX, fY, fZ;
                    guard->GetRandomPoint(stratholmeLocation[5].m_fX, stratholmeLocation[5].m_fY, stratholmeLocation[5].m_fZ, 10.0f, fX, fY, fZ);
                    guard->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                }
            }
            m_blackGuardsTimer = 0;
        }
        else
            m_blackGuardsTimer -= diff;
    }
    // Timer to close the gate of the Slaughterhouse before Baron Rivendare after Ramstein/Black Guards moved out
    if (m_slaughterDoorTimer)
    {
        if (m_slaughterDoorTimer <= diff)
        {
            DoOpenSlaughterhouseDoor(false);
            m_slaughterDoorTimer = 0;
        }
        else
            m_slaughterDoorTimer -= diff;
    }
    // Timer to teleport Barthilas
    if (m_barthilasRunTimer)
    {
        if (m_barthilasRunTimer <= diff)
        {
            Creature* barthilas = GetSingleCreatureFromStorage(NPC_BARTHILAS);
            if (barthilas && barthilas->IsAlive() && !barthilas->IsInCombat())
            {
                barthilas->GetMotionMaster()->Clear(false, true);
                barthilas->GetMotionMaster()->MoveIdle();
                barthilas->NearTeleportTo(stratholmeLocation[1].m_fX, stratholmeLocation[1].m_fY, stratholmeLocation[1].m_fZ, stratholmeLocation[1].m_fO);
            }

            SetData(TYPE_BARTHILAS_RUN, DONE);
            m_barthilasRunTimer = 0;
        }
        else
            m_barthilasRunTimer -= diff;
    }

    // Timer to summon Aurius into the Slaughter House once Baron is engaged
    if (m_auriusSummonTimer)
    {
        if (m_auriusSummonTimer <= diff)
        {
            // Teleport Aurius from the Chapel and spawn it in the Slaughter House to engage Baron
            Creature* aurius = GetSingleCreatureFromStorage(NPC_AURIUS);
            if (aurius && aurius->IsAlive() && !aurius->IsInCombat())
            {
                if (Creature* baron = GetSingleCreatureFromStorage(NPC_BARON))
                {
                    float fX, fY, fZ;
                    baron->GetRandomPoint(baron->GetPositionX(), baron->GetPositionY(), baron->GetPositionZ(), 4.0f, fX, fY, fZ);
                    aurius->SetStandState(UNIT_STAND_STATE_STAND);
                    aurius->NearTeleportTo(fX, fY, fZ, aurius->GetOrientation());
                    aurius->SetRespawnCoord(fX, fY, fZ, aurius->GetOrientation());
                    DoScriptText(SAY_AURIUS_AGGRO, aurius);
                    aurius->SetFactionTemporary(FACTION_VICTIM, TEMPFACTION_RESTORE_RESPAWN);
                    aurius->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
                    aurius->AI()->AttackStart(baron);
                }
            }
            m_auriusSummonTimer = 0;
        }
        else
            m_auriusSummonTimer -= diff;
    }

    // Check changes for Baron ultimatum timer only if Baron is not already in combat
    if (m_baronRunTimer && GetData(TYPE_BARON) != IN_PROGRESS)
    {
        if (m_yellCounter == 0 && m_baronRunTimer <= 10 * MINUTE * IN_MILLISECONDS)
        {
            DoOrSimulateScriptTextForThisInstance(SAY_ANNOUNCE_RUN_10_MIN, NPC_BARON);
            ++m_yellCounter;
        }
        else if (m_yellCounter == 1 && m_baronRunTimer <= 5 * MINUTE * IN_MILLISECONDS)
        {
            DoOrSimulateScriptTextForThisInstance(SAY_ANNOUNCE_RUN_5_MIN, NPC_BARON);
            ++m_yellCounter;
        }
        // Used to create a delay of 10s between Baron speech and Ysida's answer
        else if (m_yellCounter == 2 && m_baronRunTimer <= (5 * MINUTE - 10) * IN_MILLISECONDS)
        {
            DoOrSimulateScriptTextForThisInstance(SAY_YSIDA_RUN_5_MIN, NPC_YSIDA);
            ++m_yellCounter;
        }

        if (m_baronRunTimer <= diff)
        {
            if (GetData(TYPE_BARON_RUN) != FAIL)
            {
                SetData(TYPE_BARON_RUN, FAIL);

                // Open the cage and let Ysida face her doom
                if (Creature* ysida = GetSingleCreatureFromStorage(NPC_YSIDA))
                {
                    ysida->GetMotionMaster()->MovePoint(0, stratholmeLocation[8].m_fX, stratholmeLocation[8].m_fY, stratholmeLocation[8].m_fZ);
                    DoUseDoorOrButton(GO_YSIDA_CAGE);
                }

                DoOrSimulateScriptTextForThisInstance(SAY_ANNOUNCE_RUN_FAIL, NPC_BARON);

                m_baronRunTimer = 8 * IN_MILLISECONDS;  // We reset the timer so the speech of Ysida is not said at the same time than the Baron's one
            }
            else
            {
                // Baron ultimatum failed: let the Baron kill her
                if (Creature* ysida = GetSingleCreatureFromStorage(NPC_YSIDA))
                {
                    if (Creature* baron = GetSingleCreatureFromStorage(NPC_BARON))
                        baron->CastSpell(ysida, SPELL_BARON_SOUL_DRAIN, TRIGGERED_NONE);
                }

                DoOrSimulateScriptTextForThisInstance(SAY_YSIDA_RUN_FAIL, NPC_YSIDA);

                m_baronRunTimer = 0;  // event done for good, no more speech
                debug_log("SD2: Instance Stratholme: Baron run event reached end. Event has state %u.", GetData(TYPE_BARON_RUN));
            }
        }
        else
            m_baronRunTimer -= diff;
    }

    if (m_mindlessSummonTimer)
    {
        if (m_mindlessCount < 30)
        {
            if (m_mindlessSummonTimer <= diff)
            {
                if (Creature* baron = GetSingleCreatureFromStorage(NPC_BARON))
                {
                    // Summon mindless skeletons and move them to random point in the center of the square
                    if (Creature* temp = baron->SummonCreature(NPC_MINDLESS_UNDEAD, stratholmeLocation[4].m_fX, stratholmeLocation[4].m_fY, stratholmeLocation[4].m_fZ, stratholmeLocation[4].m_fO, TEMPSPAWN_DEAD_DESPAWN, 0))
                    {
                        float fX, fY, fZ;
                        baron->GetRandomPoint(stratholmeLocation[5].m_fX, stratholmeLocation[5].m_fY, stratholmeLocation[5].m_fZ, 20.0f, fX, fY, fZ);
                        temp->SetWalk(false);
                        temp->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                        m_undeadGUIDs.push_back(temp->GetObjectGuid());
                        ++m_mindlessCount;
                    }
                }
                m_mindlessSummonTimer = 400;
            }
            else
                m_mindlessSummonTimer -= diff;
        }
        else
            m_mindlessSummonTimer = 0;
    }

    if (m_slaughterSquareTimer)
    {
        if (m_slaughterSquareTimer <= diff)
        {
            // Call next Abominations
            for (auto abomGUID : m_abominationGUIDs)
            {
                Creature* abom = instance->GetCreature(abomGUID);
                // Skip killed and already walking Abominations
                if (!abom || !abom->IsAlive() || abom->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
                    continue;

                // Let Move to somewhere in the middle
                if (!abom->IsInCombat())
                {
                    if (GameObject* door = GetSingleGameObjectFromStorage(GO_PORT_SLAUGHTER))
                    {
                        float fX, fY, fZ;
                        abom->GetRandomPoint(door->GetPositionX(), door->GetPositionY(), door->GetPositionZ(), 10.0f, fX, fY, fZ);
                        abom->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                    }
                }
                break;
            }

            // TODO - how fast are they called?
            m_slaughterSquareTimer = urand(30, 45) * IN_MILLISECONDS;
        }
        else
            m_slaughterSquareTimer -= diff;
    }
}

bool instance_stratholme::CheckConditionCriteriaMeet(Player const* player, uint32 instanceConditionId, WorldObject const* conditionSource, uint32 conditionSourceType) const
{
    if (instanceConditionId == INSTANCE_CONDITION_ID_NORMAL_MODE)   // Aurius event during fight against Baron Rivendare
        return GetData(TYPE_AURIUS) == DONE;
    else
        script_error_log("instance_stratholme::CheckConditionCriteriaMeet called with unsupported Id %u. Called with param plr %s, src %s, condition source type %u",
                     instanceConditionId, player ? player->GetGuidStr().c_str() : "nullptr", conditionSource ? conditionSource->GetGuidStr().c_str() : "nullptr", conditionSourceType);
    return false;
}

InstanceData* GetInstanceData_instance_stratholme(Map* map)
{
    return new instance_stratholme(map);
}

void AddSC_instance_stratholme()
{
    Script* newScript = new Script;
    newScript->Name = "instance_stratholme";
    newScript->GetInstanceData = &GetInstanceData_instance_stratholme;
    newScript->RegisterSelf();
}
