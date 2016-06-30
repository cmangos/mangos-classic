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
SD%Complete: 70
SDComment: Undead side 90% implemented, event needs better implementation, Barthildas relocation for reload case is missing, Baron Combat handling is buggy.
SDCategory: Stratholme
EndScriptData */

#include "precompiled.h"
#include "stratholme.h"

instance_stratholme::instance_stratholme(Map* pMap) : ScriptedInstance(pMap),
    m_uiBaronRunTimer(0),
    m_uiBarthilasRunTimer(0),
    m_uiMindlessSummonTimer(0),
    m_uiSlaugtherSquareTimer(0),
    m_uiYellCounter(0),
    m_uiMindlessCount(0),
    m_uiPostboxesUsed(0),
    m_uiAuriusSummonTimer(0)
{
    Initialize();
}

void instance_stratholme::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

void instance_stratholme::OnPlayerEnter(Player* pPlayer)
{
    // Baron ultimatum succeed: summon Ysida outside the cage alive
    if (GetData(TYPE_BARON_RUN) == DONE)
        pPlayer->SummonCreature(NPC_YSIDA, aStratholmeLocation[8].m_fX, aStratholmeLocation[8].m_fY, aStratholmeLocation[8].m_fZ, aStratholmeLocation[8].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0);
    // Baron ultimatum failed: summon Ysida outside the cage dead
    else if (GetData(TYPE_BARON_RUN) == FAIL)
    {
        if (Creature* pYsida = pPlayer->SummonCreature(NPC_YSIDA, aStratholmeLocation[8].m_fX, aStratholmeLocation[8].m_fY, aStratholmeLocation[8].m_fZ, aStratholmeLocation[8].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0))
            pYsida->SetDeadByDefault(true);
    }
}

bool instance_stratholme::StartSlaugtherSquare()
{
    if (m_auiEncounter[TYPE_BARONESS] == SPECIAL && m_auiEncounter[TYPE_NERUB] == SPECIAL && m_auiEncounter[TYPE_PALLID] == SPECIAL)
    {
        DoOrSimulateScriptTextForThisInstance(SAY_ANNOUNCE_RIVENDARE, NPC_BARON);

        DoUseDoorOrButton(GO_PORT_GAUNTLET);
        DoUseDoorOrButton(GO_PORT_SLAUGTHER);

        debug_log("SD2: Instance Stratholme: Open slaugther square.");

        return true;
    }

    return false;
}

void instance_stratholme::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_AURIUS:
            if (m_auiEncounter[TYPE_AURIUS] == DONE)
                pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            // no break here
        case NPC_BARON:
        case NPC_YSIDA:
        case NPC_BARTHILAS:
            m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;

        case NPC_CRYSTAL:
            m_luiCrystalGUIDs.push_back(pCreature->GetObjectGuid());
            break;
        case NPC_ABOM_BILE:
        case NPC_ABOM_VENOM:
            m_sAbomnationGUID.insert(pCreature->GetObjectGuid());
            break;
        case NPC_THUZADIN_ACOLYTE:
            m_luiAcolyteGUIDs.push_back(pCreature->GetObjectGuid());
            break;
        case NPC_CRIMSON_INITIATE:
        case NPC_CRIMSON_GALLANT:
        case NPC_CRIMSON_GUARDSMAN:
        case NPC_CRIMSON_CONJURER:
        case NPC_CRIMSON_BATTLE_MAGE:
            // Store GUID of NPCs in the courtyard to spawn Timmy once they are all dead
            if (pCreature->IsWithinDist2d(aDefensePoints[TIMMY].m_fX, aDefensePoints[TIMMY].m_fY, 40.0f))
                m_suiCrimsonDefendersLowGuids[TIMMY].push_back(pCreature->GetObjectGuid());
            // Iterate also over all the defense points where those NPCs are possibly spawned
            for (uint8 i = BARRICADE; i <= FIRST_BARRICADES; i++)
            {
                // Do not store - again - GUIDs for Timmy spawn point, they were done previously with a different range
                if (i == TIMMY)
                    continue;
                // Store the GUID of the nearby NPCs for each defense point
                if (pCreature->IsWithinDist2d(aDefensePoints[i].m_fX, aDefensePoints[i].m_fY, 8.0f))
                {
                    m_suiCrimsonDefendersLowGuids[i].push_back(pCreature->GetObjectGuid());
                    break;
                }
            }
            break;
    }
}

void instance_stratholme::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_SERVICE_ENTRANCE:
            break;
        case GO_GAUNTLET_GATE1:
            // TODO
            // weird, but unless flag is set, client will not respond as expected. DB bug?
            pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED);
            break;

        case GO_ZIGGURAT_DOOR_1:
            m_zigguratStorage[0].m_doorGuid = pGo->GetObjectGuid();
            if (m_auiEncounter[TYPE_BARONESS] == DONE || m_auiEncounter[TYPE_BARONESS] == SPECIAL)
                pGo->SetGoState(GO_STATE_ACTIVE);
            return;
        case GO_ZIGGURAT_DOOR_2:
            m_zigguratStorage[1].m_doorGuid = pGo->GetObjectGuid();
            if (m_auiEncounter[TYPE_NERUB] == DONE || m_auiEncounter[TYPE_NERUB] == SPECIAL)
                pGo->SetGoState(GO_STATE_ACTIVE);
            return;
        case GO_ZIGGURAT_DOOR_3:
            m_zigguratStorage[2].m_doorGuid = pGo->GetObjectGuid();
            if (m_auiEncounter[TYPE_PALLID] == DONE || m_auiEncounter[TYPE_PALLID] == SPECIAL)
                pGo->SetGoState(GO_STATE_ACTIVE);
            return;

        case GO_ZIGGURAT_DOOR_4:
            if (m_auiEncounter[TYPE_RAMSTEIN] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_ZIGGURAT_DOOR_5:
            if (m_auiEncounter[TYPE_RAMSTEIN] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PORT_GAUNTLET:
            if (m_auiEncounter[TYPE_BARONESS] == SPECIAL && m_auiEncounter[TYPE_NERUB] == SPECIAL && m_auiEncounter[TYPE_PALLID] == SPECIAL)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PORT_SLAUGTHER:
            if (m_auiEncounter[TYPE_BARONESS] == SPECIAL && m_auiEncounter[TYPE_NERUB] == SPECIAL && m_auiEncounter[TYPE_PALLID] == SPECIAL)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PORT_SLAUGHTER_GATE:
            if (m_auiEncounter[TYPE_RAMSTEIN] == DONE)      // Might actually be uneeded
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PORT_ELDERS:
        case GO_YSIDA_CAGE:
            break;

        default:
            return;
    }
    m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
}

void instance_stratholme::SetData(uint32 uiType, uint32 uiData)
{
    // TODO: Remove the hard-coded indexes from array accessing
    switch (uiType)
    {
        case TYPE_BARON_RUN:
            switch (uiData)
            {
                case IN_PROGRESS:
                    if (m_auiEncounter[uiType] == IN_PROGRESS || m_auiEncounter[uiType] == FAIL)
                        break;

                    // Baron ultimatum starts: summon Ysida in the cage
                    if (Creature* pBaron = GetSingleCreatureFromStorage(NPC_BARON))
                    {
                        DoOrSimulateScriptTextForThisInstance(SAY_ANNOUNCE_RUN_START, NPC_BARON);
                        if (Creature* pYsida = pBaron->SummonCreature(NPC_YSIDA, aStratholmeLocation[7].m_fX, aStratholmeLocation[7].m_fY, aStratholmeLocation[7].m_fZ, aStratholmeLocation[7].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0))
                            pYsida->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
                    }

                    m_uiBaronRunTimer = 45 * MINUTE * IN_MILLISECONDS;
                    debug_log("SD2: Instance Stratholme: Baron run in progress.");
                    break;
                case FAIL:
                    // may add code to remove aura from players, but in theory the time should be up already and removed.
                    break;
                case DONE:
                    m_uiBaronRunTimer = 0;
                    break;
            }
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_BARONESS:
        case TYPE_NERUB:
        case TYPE_PALLID:
            m_auiEncounter[uiType] = uiData;
            if (uiData == DONE)
            {
                DoSortZiggurats();
                DoUseDoorOrButton(m_zigguratStorage[uiType - TYPE_BARONESS].m_doorGuid);
            }
            if (uiData == SPECIAL)
                StartSlaugtherSquare();
            break;
        case TYPE_RAMSTEIN:
            if (uiData == SPECIAL)
            {
                if (m_auiEncounter[uiType] != SPECIAL && m_auiEncounter[uiType] != DONE)
                {
                    m_uiSlaugtherSquareTimer = 20000;       // TODO - unknown, also possible that this is not the very correct place..
                    DoUseDoorOrButton(GO_PORT_GAUNTLET);
                }

                uint32 uiCount = m_sAbomnationGUID.size();
                for (GuidSet::iterator itr = m_sAbomnationGUID.begin(); itr != m_sAbomnationGUID.end();)
                {
                    if (Creature* pAbom = instance->GetCreature(*itr))
                    {
                        ++itr;
                        if (!pAbom->isAlive())
                            --uiCount;
                    }
                    else
                    {
                        // Remove obsolete guid from set and decrement count
                        m_sAbomnationGUID.erase(itr++);
                        --uiCount;
                    }
                }

                if (!uiCount)
                {
                    // Old Comment: a bit itchy, it should close GO_ZIGGURAT_DOOR_4 door after 10 secs, but it doesn't. skipping it for now.
                    // However looks like that this door is no more closed
                    DoUseDoorOrButton(GO_ZIGGURAT_DOOR_4);

                    // No more handlng of Abomnations
                    m_uiSlaugtherSquareTimer = 0;

                    if (Creature* pBaron = GetSingleCreatureFromStorage(NPC_BARON))
                    {
                        DoScriptText(SAY_ANNOUNCE_RAMSTEIN, pBaron);
                        if (Creature* pRamstein = pBaron->SummonCreature(NPC_RAMSTEIN, aStratholmeLocation[2].m_fX, aStratholmeLocation[2].m_fY, aStratholmeLocation[2].m_fZ, aStratholmeLocation[2].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0))
                            pRamstein->GetMotionMaster()->MovePoint(0, aStratholmeLocation[3].m_fX, aStratholmeLocation[3].m_fY, aStratholmeLocation[3].m_fZ);

                        debug_log("SD2: Instance Stratholme - Slaugther event: Ramstein spawned.");
                    }
                }
                else
                    debug_log("SD2: Instance Stratholme - Slaugther event: %u Abomnation left to kill.", uiCount);
            }
            // After fail aggroing Ramstein means wipe on Ramstein, so close door again
            if (uiData == IN_PROGRESS && m_auiEncounter[uiType] == FAIL)
                DoUseDoorOrButton(GO_PORT_GAUNTLET);
            if (uiData == DONE)
            {
                // Open side gate and start summoning skeletons
                DoUseDoorOrButton(GO_PORT_SLAUGHTER_GATE);
                // use this timer as a bool just to start summoning
                m_uiMindlessSummonTimer = 500;
                m_uiMindlessCount = 0;
                m_luiUndeadGUIDs.clear();

                // Summon 5 guards
                if (Creature* pBaron = GetSingleCreatureFromStorage(NPC_BARON))
                {
                    for (uint8 i = 0; i < 5; ++i)
                    {
                        float fX, fY, fZ;
                        pBaron->GetRandomPoint(aStratholmeLocation[6].m_fX, aStratholmeLocation[6].m_fY, aStratholmeLocation[6].m_fZ, 5.0f, fX, fY, fZ);
                        if (Creature* pTemp = pBaron->SummonCreature(NPC_BLACK_GUARD, aStratholmeLocation[6].m_fX, aStratholmeLocation[6].m_fY, aStratholmeLocation[6].m_fZ, aStratholmeLocation[6].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0))
                            m_luiGuardGUIDs.push_back(pTemp->GetObjectGuid());
                    }

                    debug_log("SD2: Instance Stratholme - Slaugther event: Summoned 5 guards.");
                }
            }
            // Open Door again and stop Abomnation
            if (uiData == FAIL && m_auiEncounter[uiType] != FAIL)
            {
                DoUseDoorOrButton(GO_PORT_GAUNTLET);
                m_uiSlaugtherSquareTimer = 0;

                // Let already moving Abomnations stop
                for (GuidSet::const_iterator itr = m_sAbomnationGUID.begin(); itr != m_sAbomnationGUID.end(); ++itr)
                {
                    Creature* pAbom = instance->GetCreature(*itr);
                    if (pAbom && pAbom->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
                        pAbom->GetMotionMaster()->MovementExpired();
                }
            }

            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_BARON:
            if (uiData == IN_PROGRESS)
            {
                // Close Slaughterhouse door if needed
                if (m_auiEncounter[uiType] == FAIL)
                    DoUseDoorOrButton(GO_PORT_GAUNTLET);

                // If Aurius was given the medaillon wait 5s before summoning him
                if (GetData(TYPE_AURIUS) == DONE)
                    m_uiAuriusSummonTimer = 5000;
            }
            else if (uiData == DONE)
            {
                // Players successfully engaged Baron within the time-limit of his ultimatum
                //     Note: UpdateAI() prevents TYPE_BARON_RUN to be marked as FAILED if the
                //     Baron is already engaged (in progress) when the ultimatum timer expires
                if (m_auiEncounter[TYPE_BARON_RUN] == IN_PROGRESS)
                {
                    SetData(TYPE_BARON_RUN, DONE);
                    Map::PlayerList const& players = instance->GetPlayers();

                    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    {
                        if (Player* pPlayer = itr->getSource())
                        {
                            if (pPlayer->HasAura(SPELL_BARON_ULTIMATUM))
                                pPlayer->RemoveAurasDueToSpell(SPELL_BARON_ULTIMATUM);

                            if (pPlayer->GetQuestStatus(QUEST_DEAD_MAN_PLEA) == QUEST_STATUS_INCOMPLETE)
                                pPlayer->KilledMonsterCredit(NPC_YSIDA);

                            // Argent Dawn reputation reward
                            pPlayer->CastSpell(pPlayer, SPELL_YSIDA_FREED, true);
                        }
                    }

                    // Open cage, finish rescue event
                    if (Creature* pYsida = GetSingleCreatureFromStorage(NPC_YSIDA))
                    {
                        DoScriptText(SAY_EPILOGUE, pYsida);
                        DoUseDoorOrButton(GO_YSIDA_CAGE);
                        pYsida->GetMotionMaster()->MovePoint(0, aStratholmeLocation[8].m_fX, aStratholmeLocation[8].m_fY, aStratholmeLocation[8].m_fZ);
                        pYsida->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
                    }
                }

                // If Aurius was spawned to help fight the Baron
                if (GetData(TYPE_AURIUS) == DONE)
                {
                    // Baron killed and Aurius is alive: give him his NPC Flags back, so players can complete the quest and fake his death
                    if (Creature* pAurius = GetSingleCreatureFromStorage(NPC_AURIUS))
                    {
                        if (pAurius->isAlive())
                        {
                            DoScriptText(SAY_AURIUS_DEATH, pAurius);
                            pAurius->StopMoving();
                            pAurius->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
                            pAurius->InterruptNonMeleeSpells(true);
                            pAurius->SetHealth(1);
                            pAurius->GetMotionMaster()->MovementExpired();
                            pAurius->GetMotionMaster()->MoveIdle();
                            pAurius->RemoveAllAurasOnDeath();
                            pAurius->SetStandState(UNIT_STAND_STATE_DEAD);
                        }
                    }
                }

                // Open Slaughterhouse door again
                DoUseDoorOrButton(GO_PORT_GAUNTLET);
            }
            else if (uiData == FAIL)
                DoUseDoorOrButton(GO_PORT_GAUNTLET);

            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_BARTHILAS_RUN:
            if (uiData == IN_PROGRESS)
            {
                Creature* pBarthilas = GetSingleCreatureFromStorage(NPC_BARTHILAS);
                if (pBarthilas && pBarthilas->isAlive() && !pBarthilas->isInCombat())
                {
                    DoScriptText(SAY_WARN_BARON, pBarthilas);
                    pBarthilas->SetWalk(false);
                    pBarthilas->GetMotionMaster()->MovePoint(0, aStratholmeLocation[0].m_fX, aStratholmeLocation[0].m_fY, aStratholmeLocation[0].m_fZ);

                    m_uiBarthilasRunTimer = 8000;
                }
            }
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_BLACK_GUARDS:
            // Prevent double action
            if (m_auiEncounter[uiType] == uiData)
                return;

            // Restart after failure, close Gauntlet
            if (uiData == IN_PROGRESS && m_auiEncounter[uiType] == FAIL)
                DoUseDoorOrButton(GO_PORT_GAUNTLET);
            // Wipe case - open gauntlet
            if (uiData == FAIL)
                DoUseDoorOrButton(GO_PORT_GAUNTLET);
            if (uiData == DONE)
            {
                if (Creature* pBaron = GetSingleCreatureFromStorage(NPC_BARON))
                    DoScriptText(SAY_UNDEAD_DEFEAT, pBaron);
                DoUseDoorOrButton(GO_ZIGGURAT_DOOR_5);
            }
            m_auiEncounter[uiType] = uiData;

            // No need to save anything here, so return
            return;
        case TYPE_POSTMASTER:
            m_auiEncounter[uiType] = uiData;
            if (uiData == IN_PROGRESS)
            {
                ++m_uiPostboxesUsed;

                // After the second post box prepare to spawn the Post Master
                if (m_uiPostboxesUsed == 2)
                    SetData(TYPE_POSTMASTER, SPECIAL);
            }
            // No need to save anything here, so return
            return;
        case TYPE_AURIUS:
            // Prevent further players to complete the quest in that instance or autocomplete the follow-up quest. the flag will be set back if event is succeed
            if (uiData == DONE)
            {
                if (Creature* pAurius = GetSingleCreatureFromStorage(NPC_AURIUS))
                    pAurius->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            }
            m_auiEncounter[uiType] = uiData;
            break;
    }

    if (uiData == DONE)
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

    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
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

uint32 instance_stratholme::GetData(uint32 uiType) const
{
    switch (uiType)
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
            return m_auiEncounter[uiType];
        default:
            return 0;
    }
}

static bool sortByHeight(Creature* pFirst, Creature* pSecond)
{
    return pFirst && pSecond && pFirst->GetPositionZ() > pSecond->GetPositionZ();
}

void instance_stratholme::DoSortZiggurats()
{
    if (m_luiAcolyteGUIDs.empty())
        return;

    std::list<Creature*> lAcolytes;                         // Valid pointers, only used locally
    for (GuidList::const_iterator itr = m_luiAcolyteGUIDs.begin(); itr != m_luiAcolyteGUIDs.end(); ++itr)
    {
        if (Creature* pAcolyte = instance->GetCreature(*itr))
            lAcolytes.push_back(pAcolyte);
    }
    m_luiAcolyteGUIDs.clear();

    if (lAcolytes.empty())
        return;

    if (!GetSingleCreatureFromStorage(NPC_THUZADIN_ACOLYTE, true))
    {
        // Sort the acolytes by height, and the one with the biggest height is the announcer (a bit outside the map)
        lAcolytes.sort(sortByHeight);
        m_mNpcEntryGuidStore[NPC_THUZADIN_ACOLYTE] = (*lAcolytes.begin())->GetObjectGuid();
        lAcolytes.erase(lAcolytes.begin());
    }

    // Sort Acolytes
    for (std::list<Creature*>::iterator itr = lAcolytes.begin(); itr != lAcolytes.end();)
    {
        bool bAlreadyIterated = false;
        for (uint8 i = 0; i < MAX_ZIGGURATS; ++i)
        {
            if (GameObject* pZigguratDoor = instance->GetGameObject(m_zigguratStorage[i].m_doorGuid))
            {
                if ((*itr)->isAlive() && (*itr)->IsWithinDistInMap(pZigguratDoor, 35.0f, false))
                {
                    m_zigguratStorage[i].m_lZigguratAcolyteGuid.push_back((*itr)->GetObjectGuid());
                    itr = lAcolytes.erase(itr);
                    bAlreadyIterated = true;
                    break;
                }
            }
        }

        if (itr != lAcolytes.end() && !bAlreadyIterated)
            ++itr;
    }

    // In case some mobs have not been able to be sorted, store their GUIDs again
    for (std::list<Creature*>::const_iterator itr = lAcolytes.begin(); itr != lAcolytes.end(); ++itr)
        m_luiAcolyteGUIDs.push_back((*itr)->GetObjectGuid());

    // Sort Crystal
    for (GuidList::iterator itr = m_luiCrystalGUIDs.begin(); itr != m_luiCrystalGUIDs.end();)
    {
        Creature* pCrystal = instance->GetCreature(*itr);
        if (!pCrystal)
        {
            itr = m_luiCrystalGUIDs.erase(itr);
            continue;
        }

        bool bAlreadyIterated = false;
        for (uint8 i = 0; i < MAX_ZIGGURATS; ++i)
        {
            if (GameObject* pZigguratDoor = instance->GetGameObject(m_zigguratStorage[i].m_doorGuid))
            {
                if (pCrystal->IsWithinDistInMap(pZigguratDoor, 50.0f, false))
                {
                    m_zigguratStorage[i].m_crystalGuid = pCrystal->GetObjectGuid();
                    itr = m_luiCrystalGUIDs.erase(itr);
                    bAlreadyIterated = true;
                    break;
                }
            }
        }

        if (itr != m_luiCrystalGUIDs.end() && !bAlreadyIterated)
            ++itr;
    }
}

void instance_stratholme::OnCreatureEnterCombat(Creature* pCreature)
{
    switch (pCreature->GetEntry())
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

void instance_stratholme::OnCreatureEvade(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_BARONESS_ANASTARI: SetData(TYPE_BARONESS, FAIL); break;
        case NPC_MALEKI_THE_PALLID: SetData(TYPE_PALLID, FAIL);   break;
        case NPC_NERUBENKAN:        SetData(TYPE_NERUB, FAIL);    break;
        case NPC_RAMSTEIN:          SetData(TYPE_RAMSTEIN, FAIL); break;
        case NPC_BARON:             SetData(TYPE_BARON, FAIL);    break;

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

void instance_stratholme::OnCreatureDeath(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_BARONESS_ANASTARI: SetData(TYPE_BARONESS, DONE); break;
        case NPC_MALEKI_THE_PALLID: SetData(TYPE_PALLID, DONE);   break;
        case NPC_NERUBENKAN:        SetData(TYPE_NERUB, DONE);    break;
        case NPC_RAMSTEIN:          SetData(TYPE_RAMSTEIN, DONE); break;
        case NPC_BARON:             SetData(TYPE_BARON, DONE);    break;

        case NPC_AURIUS:
            DoScriptText(SAY_AURIUS_DEATH, pCreature);
            break;

        case NPC_THUZADIN_ACOLYTE:
            ThazudinAcolyteJustDied(pCreature);
            break;

        case NPC_ABOM_BILE:
        case NPC_ABOM_VENOM:
            // Start Slaughterhouse Event
            SetData(TYPE_RAMSTEIN, SPECIAL);
            break;

        case NPC_MINDLESS_UNDEAD:
            m_luiUndeadGUIDs.remove(pCreature->GetObjectGuid());
            if (m_luiUndeadGUIDs.empty())
            {
                // Let the black Guards move out of the citadel
                for (GuidList::const_iterator itr = m_luiGuardGUIDs.begin(); itr != m_luiGuardGUIDs.end(); ++itr)
                {
                    Creature* pGuard = instance->GetCreature(*itr);
                    if (pGuard && pGuard->isAlive() && !pGuard->isInCombat())
                    {
                        float fX, fY, fZ;
                        pGuard->GetRandomPoint(aStratholmeLocation[5].m_fX, aStratholmeLocation[5].m_fY, aStratholmeLocation[5].m_fZ, 10.0f, fX, fY, fZ);
                        pGuard->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                    }
                }
            }
            break;
        case NPC_BLACK_GUARD:
            m_luiGuardGUIDs.remove(pCreature->GetObjectGuid());
            if (m_luiGuardGUIDs.empty())
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
            	if (m_suiCrimsonDefendersLowGuids[i].empty())
            		continue;

                m_suiCrimsonDefendersLowGuids[i].remove(pCreature->GetObjectGuid());
                // If all mobs from a defense group are dead then activate the related defense event
                if (m_suiCrimsonDefendersLowGuids[i].empty() && i != FIRST_BARRICADES)
                    DoScarletBastionDefense(i, pCreature);
            }
            break;
        case NPC_BALNAZZAR:
            DoScarletBastionDefense(CRIMSON_THRONE, pCreature);
            break;
    }
}

void instance_stratholme::ThazudinAcolyteJustDied(Creature* pCreature)
{
    for (uint8 i = 0; i < MAX_ZIGGURATS; ++i)
    {
        if (m_zigguratStorage[i].m_lZigguratAcolyteGuid.empty())
            continue;                               // nothing to do anymore for this ziggurat

        m_zigguratStorage[i].m_lZigguratAcolyteGuid.remove(pCreature->GetObjectGuid());
        if (m_zigguratStorage[i].m_lZigguratAcolyteGuid.empty())
        {
            // A random zone yell after one is cleared
            int32 aAnnounceSay[MAX_ZIGGURATS] = {SAY_ANNOUNCE_ZIGGURAT_1, SAY_ANNOUNCE_ZIGGURAT_2, SAY_ANNOUNCE_ZIGGURAT_3};
            DoOrSimulateScriptTextForThisInstance(aAnnounceSay[i], NPC_THUZADIN_ACOLYTE);

            // Kill Crystal
            if (Creature* pCrystal = instance->GetCreature(m_zigguratStorage[i].m_crystalGuid))
                pCrystal->DealDamage(pCrystal, pCrystal->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);

            switch (i)
            {
                case 0: SetData(TYPE_BARONESS, SPECIAL); break;
                case 1: SetData(TYPE_NERUB, SPECIAL);    break;
                case 2: SetData(TYPE_PALLID, SPECIAL);   break;
            }
        }
    }
}

void instance_stratholme::DoSpawnScarletGuards(uint8 uiStep, Player* pSummoner)
{
    if (!pSummoner)
        return;

    uint32 uiNPCEntry[2];
    uint8  uiIndex;

    switch (uiStep)
    {
        case HALL_OF_LIGHTS:
            uiNPCEntry[0] = NPC_CRIMSON_GALLANT;
            uiNPCEntry[1] = NPC_CRIMSON_GALLANT;
            uiIndex = 0;
            break;
        case INNER_BASTION_2:
            uiNPCEntry[0] = NPC_CRIMSON_MONK;
            uiNPCEntry[1] = NPC_CRIMSON_SORCERER;
            uiIndex = 3;
            break;
        default:
            return;    // avoid indexing the following tables with a wrong index. Should never happen.
    }

    // Spawn the two guards and move them to where they will guard (each side of a door)
    for (uint8 i = 0; i < 2; i++)
    {
        if (Creature* pTemp = pSummoner->SummonCreature(uiNPCEntry[i], aScarletGuards[uiIndex].m_fX, aScarletGuards[uiIndex].m_fY, aScarletGuards[uiIndex].m_fZ, aScarletGuards[uiIndex].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0))
        {
            pTemp->SetWalk(false);
            pTemp->GetMotionMaster()->MovePoint(0, aScarletGuards[uiIndex + i + 1].m_fX, aScarletGuards[uiIndex + i + 1].m_fY, aScarletGuards[uiIndex + i + 1].m_fZ);
        }
    }

    return;
}

void instance_stratholme::DoSpawnScourgeInvaders(uint8 uiStep, Player* pSummoner)
{
    if (!pSummoner)
        return;

    // Define the group of 5 Scourge invaders
    std::vector<uint32> uiMobList;                  // Vector holding the 5 creatures entries for each Scourge invaders group
    uiMobList.push_back(NPC_SKELETAL_GUARDIAN);     // 4 static NPC entries
    uiMobList.push_back(NPC_SKELETAL_GUARDIAN);
    uiMobList.push_back(NPC_SKELETAL_BERSERKER);
    uiMobList.push_back(NPC_SKELETAL_BERSERKER);

    uint32 uiMobEntry;                              // will hold the last random creature entry
    uint8  uiIndex;

    // Pick the fifth NPC in the group and randomize the five possible spawns
    switch (urand(0, 1))
    {
        case 0: uiMobEntry = NPC_SKELETAL_GUARDIAN;     break;
        case 1: uiMobEntry = NPC_SKELETAL_BERSERKER;    break;
    }

    uiMobList.push_back(uiMobEntry);
    std::random_shuffle(uiMobList.begin(), uiMobList.end());

    // Define the correct index for the spawn/move coords table
    switch (uiStep)
    {
        case ENTRANCE:          uiIndex = 1; break;
        case INNER_BASTION_1:   uiIndex = 3; break;
        case CRIMSON_THRONE:    uiIndex = 5; break;
        default: return;    // avoid indexing the following table with a wrong index. Should never happen.
    }

    // Summon the five invaders and make them run into the room
    for (uint8 i = 0; i < 5; i++)
    {
        float fTargetPosX, fTargetPosY, fTargetPosZ;

        if (Creature* pTemp = pSummoner->SummonCreature(uiMobList[i], aScourgeInvaders[uiIndex].m_fX, aScourgeInvaders[uiIndex].m_fY, aScourgeInvaders[uiIndex].m_fZ, aScourgeInvaders[uiIndex].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0))
        {
            pTemp->SetWalk(false);
            pTemp->GetRandomPoint(aScourgeInvaders[uiIndex + 1].m_fX, aScourgeInvaders[uiIndex + 1].m_fY, aScourgeInvaders[uiIndex + 1].m_fZ, 6.0f, fTargetPosX, fTargetPosY, fTargetPosZ);
            pTemp->GetMotionMaster()->MovePoint(0, fTargetPosX, fTargetPosY, fTargetPosZ);
        }
    }

    return;
}

void instance_stratholme::DoMoveBackDefenders(uint8 uiStep, Creature* pCreature)
{
    uint8 uiIndex;
    uint8 uiTreshold = 0;
    uint8 uiFoundGuards = 0;

    switch (uiStep)
    {
        case BARRICADE:
            uiIndex = FIRST_BARRICADES;
            break;
        case STAIRS:
            uiIndex = BARRICADE;
            uiTreshold = 3;
            break;
        default:
            return;     // avoid indexing the following table with a wrong index. Should never happen.
    }

    // Check that there are still defenders to move to the stairs/last barricade
    if (m_suiCrimsonDefendersLowGuids[uiIndex].empty())
        return;
    if (pCreature)
        DoScriptText(ScarletEventYells[uiStep], pCreature);

    for (GuidList::const_iterator itr = m_suiCrimsonDefendersLowGuids[uiIndex].begin(); itr != m_suiCrimsonDefendersLowGuids[uiIndex].end(); ++itr)
    {
        Creature* pGuard = instance->GetCreature(*itr);
        if (pGuard && pGuard->isAlive() && !pGuard->isInCombat())
        {
			pGuard->GetMotionMaster()->MoveIdle();
        	pGuard->SetWalk(false);
            pGuard->GetMotionMaster()->MovePoint(0, aScarletLastStand[uiTreshold + uiFoundGuards].m_fX, aScarletLastStand[uiTreshold + uiFoundGuards].m_fY, aScarletLastStand[uiTreshold + uiFoundGuards].m_fZ);
            uiFoundGuards++;
        }

        if (uiFoundGuards == 3)
            return;
    }

    return;
}

void instance_stratholme::DoScarletBastionDefense(uint8 uiStep, Creature* pCreature)
{
    if (!pCreature)
        return;

    switch (uiStep)
    {
        case BARRICADE:
        case STAIRS:
            DoMoveBackDefenders(uiStep, pCreature);
            return;
        case TIMMY:
            pCreature->SummonCreature(NPC_TIMMY_THE_CRUEL, aScourgeInvaders[0].m_fX, aScourgeInvaders[0].m_fY, aScourgeInvaders[0].m_fZ, aScourgeInvaders[0].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0);
            return;
        // Scarlet guards spawned
        case HALL_OF_LIGHTS:
        case INNER_BASTION_2:
            DoScriptText(ScarletEventYells[uiStep], pCreature);
            if (Player* pPlayer = GetPlayerInMap())
                DoSpawnScarletGuards(uiStep, pPlayer);
            return;
        // Scourge invading
        case ENTRANCE:
        case INNER_BASTION_1:
            DoScriptText(ScarletEventYells[uiStep], pCreature);
        case CRIMSON_THRONE:
            if (Player* pPlayer = GetPlayerInMap())
                DoSpawnScourgeInvaders(uiStep, pPlayer);
            return;
    }
    return;
}

void instance_stratholme::Update(uint32 uiDiff)
{
    // Timer to teleport Barthilas
    if (m_uiBarthilasRunTimer)
    {
        if (m_uiBarthilasRunTimer <= uiDiff)
        {
            Creature* pBarthilas = GetSingleCreatureFromStorage(NPC_BARTHILAS);
            if (pBarthilas && pBarthilas->isAlive() && !pBarthilas->isInCombat())
                pBarthilas->NearTeleportTo(aStratholmeLocation[1].m_fX, aStratholmeLocation[1].m_fY, aStratholmeLocation[1].m_fZ, aStratholmeLocation[1].m_fO);

            SetData(TYPE_BARTHILAS_RUN, DONE);
            m_uiBarthilasRunTimer = 0;
        }
        else
            m_uiBarthilasRunTimer -= uiDiff;
    }

    // Timer to summon Aurius into the Slaughter House once Baron is engaged
    if (m_uiAuriusSummonTimer)
    {
        if (m_uiAuriusSummonTimer <= uiDiff)
        {
            // Teleport Aurius from the Chapel and spawn it in the Slaughter House to engage Baron
            Creature* pAurius = GetSingleCreatureFromStorage(NPC_AURIUS);
            if (pAurius && pAurius->isAlive() && !pAurius->isInCombat())
            {
                if (Creature* pBaron = GetSingleCreatureFromStorage(NPC_BARON))
                {
                    float fX, fY, fZ;
                    pBaron->GetRandomPoint(pBaron->GetPositionX(), pBaron->GetPositionY(), pBaron->GetPositionZ(), 4.0f, fX, fY, fZ);
                    pAurius->NearTeleportTo(fX, fY, fZ, pAurius->GetOrientation());
                    pAurius->SetRespawnCoord(fX, fY, fZ, pAurius->GetOrientation());
                    DoScriptText(SAY_AURIUS_AGGRO, pAurius);
                    pAurius->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
                    pAurius->AI()->AttackStart(pBaron);
                }
            }
            m_uiAuriusSummonTimer = 0;
        }
        else
            m_uiAuriusSummonTimer -= uiDiff;
    }

    // Check changes for Baron ultimatum timer only if Baron is not already in combat
    if (m_uiBaronRunTimer && GetData(TYPE_BARON) != IN_PROGRESS)
    {
        if (m_uiYellCounter == 0 && m_uiBaronRunTimer <= 10 * MINUTE * IN_MILLISECONDS)
        {
            DoOrSimulateScriptTextForThisInstance(SAY_ANNOUNCE_RUN_10_MIN, NPC_BARON);
            ++m_uiYellCounter;
        }
        else if (m_uiYellCounter == 1 && m_uiBaronRunTimer <= 5 * MINUTE * IN_MILLISECONDS)
        {
            DoOrSimulateScriptTextForThisInstance(SAY_ANNOUNCE_RUN_5_MIN, NPC_BARON);
            ++m_uiYellCounter;
        }
        // Used to create a delay of 10s between Baron speech and Ysida's answer
        else if (m_uiYellCounter == 2 && m_uiBaronRunTimer <= (5 * MINUTE - 10) * IN_MILLISECONDS)
        {
            DoOrSimulateScriptTextForThisInstance(SAY_YSIDA_RUN_5_MIN, NPC_YSIDA);
            ++m_uiYellCounter;
        }

        if (m_uiBaronRunTimer <= uiDiff)
        {
            if (GetData(TYPE_BARON_RUN) != FAIL)
            {
                SetData(TYPE_BARON_RUN, FAIL);

                // Open the cage and let Ysida face her doom
                if (Creature* pYsida = GetSingleCreatureFromStorage(NPC_YSIDA))
                {
                    pYsida->GetMotionMaster()->MovePoint(0, aStratholmeLocation[8].m_fX, aStratholmeLocation[8].m_fY, aStratholmeLocation[8].m_fZ);
                    DoUseDoorOrButton(GO_YSIDA_CAGE);
                }

                DoOrSimulateScriptTextForThisInstance(SAY_ANNOUNCE_RUN_FAIL, NPC_BARON);

                m_uiBaronRunTimer = 8000;  // We reset the timer so the speech of Ysida is not said at the same time than the Baron's one
            }
            else
            {
                // Baron ultimatum failed: let the Baron kill her
                if (Creature* pYsida = GetSingleCreatureFromStorage(NPC_YSIDA))
                {
                    if (Creature* pBaron = GetSingleCreatureFromStorage(NPC_BARON))
                        pBaron->CastSpell(pYsida, SPELL_BARON_SOUL_DRAIN, false);
                }

                DoOrSimulateScriptTextForThisInstance(SAY_YSIDA_RUN_FAIL, NPC_YSIDA);

                m_uiBaronRunTimer = 0;  // event done for good, no more speech
                debug_log("SD2: Instance Stratholme: Baron run event reached end. Event has state %u.", GetData(TYPE_BARON_RUN));
            }
        }
        else
            m_uiBaronRunTimer -= uiDiff;
    }

    if (m_uiMindlessSummonTimer)
    {
        if (m_uiMindlessCount < 30)
        {
            if (m_uiMindlessSummonTimer <= uiDiff)
            {
                if (Creature* pBaron = GetSingleCreatureFromStorage(NPC_BARON))
                {
                    // Summon mindless skeletons and move them to random point in the center of the square
                    if (Creature* pTemp = pBaron->SummonCreature(NPC_MINDLESS_UNDEAD, aStratholmeLocation[4].m_fX, aStratholmeLocation[4].m_fY, aStratholmeLocation[4].m_fZ, aStratholmeLocation[4].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0))
                    {
                        float fX, fY, fZ;
                        pBaron->GetRandomPoint(aStratholmeLocation[5].m_fX, aStratholmeLocation[5].m_fY, aStratholmeLocation[5].m_fZ, 20.0f, fX, fY, fZ);
                        pTemp->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                        m_luiUndeadGUIDs.push_back(pTemp->GetObjectGuid());
                        ++m_uiMindlessCount;
                    }
                }
                m_uiMindlessSummonTimer = 400;
            }
            else
                m_uiMindlessSummonTimer -= uiDiff;
        }
        else
            m_uiMindlessSummonTimer = 0;
    }

    if (m_uiSlaugtherSquareTimer)
    {
        if (m_uiSlaugtherSquareTimer <= uiDiff)
        {
            // Call next Abomnations
            for (GuidSet::const_iterator itr = m_sAbomnationGUID.begin(); itr != m_sAbomnationGUID.end(); ++itr)
            {
                Creature* pAbom = instance->GetCreature(*itr);
                // Skip killed and already walking Abomnations
                if (!pAbom || !pAbom->isAlive() || pAbom->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
                    continue;

                // Let Move to somewhere in the middle
                if (!pAbom->isInCombat())
                {
                    if (GameObject* pDoor = GetSingleGameObjectFromStorage(GO_PORT_SLAUGTHER))
                    {
                        float fX, fY, fZ;
                        pAbom->GetRandomPoint(pDoor->GetPositionX(), pDoor->GetPositionY(), pDoor->GetPositionZ(), 10.0f, fX, fY, fZ);
                        pAbom->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                    }
                }
                break;
            }

            // TODO - how fast are they called?
            m_uiSlaugtherSquareTimer = urand(15000, 30000);
        }
        else
            m_uiSlaugtherSquareTimer -= uiDiff;
    }
}

InstanceData* GetInstanceData_instance_stratholme(Map* pMap)
{
    return new instance_stratholme(pMap);
}

void AddSC_instance_stratholme()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_stratholme";
    pNewScript->GetInstanceData = &GetInstanceData_instance_stratholme;
    pNewScript->RegisterSelf();
}
