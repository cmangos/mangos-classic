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
    m_uiSilverHandKilled(0)
{
    Initialize();
}

void instance_stratholme::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
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
        case NPC_BARON:
        case NPC_YSIDA_TRIGGER:
        case NPC_BARTHILAS:
        case NPC_PALADIN_QUEST_CREDIT:
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
            // Only store those in the yard
            if (pCreature->IsWithinDist2d(aTimmyLocation[1].m_fX, aTimmyLocation[1].m_fY, 40.0f))
                m_suiCrimsonLowGuids.insert(pCreature->GetGUIDLow());
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

                    DoOrSimulateScriptTextForThisInstance(SAY_ANNOUNCE_RUN_START, NPC_BARON);

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
                // Reached the Baron within time-limit
                if (m_auiEncounter[TYPE_BARON_RUN] == IN_PROGRESS)
                    SetData(TYPE_BARON_RUN, DONE);

                // Close Slaughterhouse door if needed
                if (m_auiEncounter[uiType] == FAIL)
                    DoUseDoorOrButton(GO_PORT_GAUNTLET);
            }
            if (uiData == DONE)
            {
                if (m_auiEncounter[TYPE_BARON_RUN] == DONE)
                {
                    Map::PlayerList const& players = instance->GetPlayers();

                    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    {
                        if (Player* pPlayer = itr->getSource())
                        {
                            if (pPlayer->HasAura(SPELL_BARON_ULTIMATUM))
                                pPlayer->RemoveAurasDueToSpell(SPELL_BARON_ULTIMATUM);

                            if (pPlayer->GetQuestStatus(QUEST_DEAD_MAN_PLEA) == QUEST_STATUS_INCOMPLETE)
                                pPlayer->AreaExploredOrEventHappens(QUEST_DEAD_MAN_PLEA);
                        }
                    }

                    // Open cage and finish rescue event
                    if (Creature* pYsidaT = GetSingleCreatureFromStorage(NPC_YSIDA_TRIGGER))
                    {
                        if (Creature* pYsida = pYsidaT->SummonCreature(NPC_YSIDA, pYsidaT->GetPositionX(), pYsidaT->GetPositionY(), pYsidaT->GetPositionZ(), pYsidaT->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 1800000))
                        {
                            DoScriptText(SAY_EPILOGUE, pYsida);
                            pYsida->GetMotionMaster()->MovePoint(0, aStratholmeLocation[7].m_fX, aStratholmeLocation[7].m_fY, aStratholmeLocation[7].m_fZ);
                        }
                        DoUseDoorOrButton(GO_YSIDA_CAGE);
                    }
                }

                // Open Slaughterhouse door again
                DoUseDoorOrButton(GO_PORT_GAUNTLET);
            }
            if (uiData == FAIL)
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
        case TYPE_TRUE_MASTERS:
            m_auiEncounter[uiType] = uiData;
            if (uiData == SPECIAL)
            {
                ++m_uiSilverHandKilled;

                // When the 5th paladin is killed set data to DONE in order to give the quest credit for the last paladin
                if (m_uiSilverHandKilled == MAX_SILVERHAND)
                    SetData(TYPE_TRUE_MASTERS, DONE);
            }
            // No need to save anything here, so return
            return;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                   << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " " << m_auiEncounter[6];

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
               >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6];

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
        case TYPE_BARTHILAS_RUN:
        case TYPE_POSTMASTER:
        case TYPE_TRUE_MASTERS:
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
            // TODO - uncomment when proper working within core! case NPC_BARON:             SetData(TYPE_BARON, IN_PROGRESS);    break;

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
            // TODO - uncomment when proper working within core! case NPC_BARON:             SetData(TYPE_BARON, FAIL);    break;

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

            // Timmy spawn support
        case NPC_CRIMSON_INITIATE:
        case NPC_CRIMSON_GALLANT:
        case NPC_CRIMSON_GUARDSMAN:
        case NPC_CRIMSON_CONJURER:
            if (m_suiCrimsonLowGuids.find(pCreature->GetGUIDLow()) != m_suiCrimsonLowGuids.end())
            {
                m_suiCrimsonLowGuids.erase(pCreature->GetGUIDLow());

                // If all courtyard mobs are dead then summon Timmy
                if (m_suiCrimsonLowGuids.empty())
                    pCreature->SummonCreature(NPC_TIMMY_THE_CRUEL, aTimmyLocation[0].m_fX, aTimmyLocation[0].m_fY, aTimmyLocation[0].m_fZ, aTimmyLocation[0].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0);
            }
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
                pCrystal->DealDamage(pCrystal, pCrystal->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);

            switch (i)
            {
                case 0: SetData(TYPE_BARONESS, SPECIAL); break;
                case 1: SetData(TYPE_NERUB, SPECIAL);    break;
                case 2: SetData(TYPE_PALLID, SPECIAL);   break;
            }
        }
    }
}

void instance_stratholme::Update(uint32 uiDiff)
{
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

    if (m_uiBaronRunTimer)
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

        if (m_uiBaronRunTimer <= uiDiff)
        {
            SetData(TYPE_BARON_RUN, FAIL);

            DoOrSimulateScriptTextForThisInstance(SAY_ANNOUNCE_RUN_FAIL, NPC_BARON);

            m_uiBaronRunTimer = 0;
            debug_log("SD2: Instance Stratholme: Baron run event reached end. Event has state %u.", GetData(TYPE_BARON_RUN));
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
