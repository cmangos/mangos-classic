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
SDName: instance_blackrock_spire
SD%Complete: 75
SDComment: The Stadium event is missing some yells. Seal of Ascension related event NYI
SDCategory: Blackrock Spire
EndScriptData */

#include "precompiled.h"
#include "blackrock_spire.h"

enum
{
    AREATRIGGER_ENTER_UBRS      = 2046,
    AREATRIGGER_STADIUM         = 2026,

    // Arena event dialogue - handled by instance
    SAY_NEFARIUS_INTRO_1        = -1229004,
    SAY_NEFARIUS_INTRO_2        = -1229005,
    SAY_NEFARIUS_ATTACK_1       = -1229006,
    SAY_REND_JOIN               = -1229007,
    SAY_NEFARIUS_ATTACK_2       = -1229008,
    SAY_NEFARIUS_ATTACK_3       = -1229009,
    SAY_NEFARIUS_ATTACK_4       = -1229010,
    SAY_REND_LOSE_1             = -1229011,
    SAY_REND_LOSE_2             = -1229012,
    SAY_NEFARIUS_LOSE_3         = -1229013,
    SAY_NEFARIUS_LOSE_4         = -1229014,
    SAY_REND_ATTACK             = -1229015,
    SAY_NEFARIUS_WARCHIEF       = -1229016,
    SAY_NEFARIUS_VICTORY        = -1229018,

    // Emberseer event
    EMOTE_BEGIN                 = -1229000,
    SPELL_EMBERSEER_GROWING     = 16048,

    // Solakar Flamewreath Event
    SAY_ROOKERY_EVENT_START     = -1229020,
    NPC_ROOKERY_GUARDIAN        = 10258,
    NPC_ROOKERY_HATCHER         = 10683,

    // Spells
    SPELL_FINKLE_IS_EINHORN     = 16710,
};

/* Areatrigger
1470 Instance Entry
1628 LBRS, between Spiders and Ogres
1946 LBRS, ubrs pre-quest giver (1)
1986 LBRS, ubrs pre-quest giver (2)
1987 LBRS, ubrs pre-quest giver (3)
2026 UBRS, stadium event trigger
2046 UBRS, way to upper
2066 UBRS, The Beast - Exit (to the dark chamber)
2067 UBRS, The Beast - Entry
2068 LBRS, fall out of map
3726 UBRS, entrance to BWL
*/

static const DialogueEntry aStadiumDialogue[] =
{
    {NPC_LORD_VICTOR_NEFARIUS,  0,                          1000},
    {SAY_NEFARIUS_INTRO_1,      NPC_LORD_VICTOR_NEFARIUS,   7000},
    {SAY_NEFARIUS_INTRO_2,      NPC_LORD_VICTOR_NEFARIUS,   5000},
    {NPC_BLACKHAND_HANDLER,     0,                          0},
    {SAY_NEFARIUS_LOSE_4,       NPC_LORD_VICTOR_NEFARIUS,   3000},
    {SAY_REND_ATTACK,           NPC_REND_BLACKHAND,         2000},
    {SAY_NEFARIUS_WARCHIEF,     NPC_LORD_VICTOR_NEFARIUS,   0},
    {SAY_NEFARIUS_VICTORY,      NPC_LORD_VICTOR_NEFARIUS,   5000},
    {NPC_REND_BLACKHAND,        0,                          0},
    {0, 0, 0},
};

static const float rookeryEventSpawnPos[3] = {43.7685f, -259.82f, 91.6483f};

instance_blackrock_spire::instance_blackrock_spire(Map* pMap) : ScriptedInstance(pMap), DialogueHelper(aStadiumDialogue),
    m_bUpperDoorOpened(false),
    m_uiDragonspineDoorTimer(0),
    m_uiDragonspineGoCount(0),
    m_uiFlamewreathEventTimer(0),
    m_uiFlamewreathWaveCount(0),
    m_uiStadiumEventTimer(0),
    m_uiStadiumWaves(0),
    m_uiStadiumMobsAlive(0)
{
    Initialize();
}

void instance_blackrock_spire::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
    memset(&m_aRoomRuneGuid, 0, sizeof(m_aRoomRuneGuid));
    InitializeDialogueHelper(this);
}

void instance_blackrock_spire::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_EMBERSEER_IN:
            if (GetData(TYPE_ROOM_EVENT) == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_DOORS:
            break;
        case GO_EMBERSEER_OUT:
            if (GetData(TYPE_EMBERSEER) == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_FATHER_FLAME:
        case GO_GYTH_ENTRY_DOOR:
        case GO_GYTH_COMBAT_DOOR:
        case GO_DRAKKISATH_DOOR_1:
        case GO_DRAKKISATH_DOOR_2:
            break;
        case GO_GYTH_EXIT_DOOR:
            if (GetData(TYPE_STADIUM) == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;

        case GO_BRAZIER_1:
        case GO_BRAZIER_2:
        case GO_BRAZIER_3:
        case GO_BRAZIER_4:
        case GO_BRAZIER_5:
        case GO_BRAZIER_6:
        case GO_DRAGONSPINE:
            break;

        case GO_ROOM_1_RUNE: m_aRoomRuneGuid[0] = pGo->GetObjectGuid(); return;
        case GO_ROOM_2_RUNE: m_aRoomRuneGuid[1] = pGo->GetObjectGuid(); return;
        case GO_ROOM_3_RUNE: m_aRoomRuneGuid[2] = pGo->GetObjectGuid(); return;
        case GO_ROOM_4_RUNE: m_aRoomRuneGuid[3] = pGo->GetObjectGuid(); return;
        case GO_ROOM_5_RUNE: m_aRoomRuneGuid[4] = pGo->GetObjectGuid(); return;
        case GO_ROOM_6_RUNE: m_aRoomRuneGuid[5] = pGo->GetObjectGuid(); return;
        case GO_ROOM_7_RUNE: m_aRoomRuneGuid[6] = pGo->GetObjectGuid(); return;

        case GO_EMBERSEER_RUNE_1:
        case GO_EMBERSEER_RUNE_2:
        case GO_EMBERSEER_RUNE_3:
        case GO_EMBERSEER_RUNE_4:
        case GO_EMBERSEER_RUNE_5:
        case GO_EMBERSEER_RUNE_6:
        case GO_EMBERSEER_RUNE_7:
            m_lEmberseerRunesGUIDList.push_back(pGo->GetObjectGuid());
            return;

        default:
            return;
    }
    m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
}

void instance_blackrock_spire::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_PYROGUARD_EMBERSEER:
        case NPC_SOLAKAR_FLAMEWREATH:
        case NPC_LORD_VICTOR_NEFARIUS:
        case NPC_GYTH:
        case NPC_REND_BLACKHAND:
        case NPC_SCARSHIELD_INFILTRATOR:
            m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;

        case NPC_BLACKHAND_SUMMONER:
        case NPC_BLACKHAND_VETERAN:      m_lRoomEventMobGUIDList.push_back(pCreature->GetObjectGuid()); break;
        case NPC_BLACKHAND_INCARCERATOR: m_lIncarceratorGUIDList.push_back(pCreature->GetObjectGuid()); break;
    }
}

void instance_blackrock_spire::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_ROOM_EVENT:
            if (uiData == DONE)
                DoUseDoorOrButton(GO_EMBERSEER_IN);
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_EMBERSEER:
            // Don't set the same data twice
            if (m_auiEncounter[uiType] == uiData)
                break;
            // Combat door
            DoUseDoorOrButton(GO_DOORS);
            // Respawn all incarcerators and reset the runes on FAIL
            if (uiData == FAIL)
            {
                for (GuidList::const_iterator itr = m_lIncarceratorGUIDList.begin(); itr != m_lIncarceratorGUIDList.end(); ++itr)
                {
                    if (Creature* pIncarcerator = instance->GetCreature(*itr))
                    {
                        if (!pIncarcerator->isAlive())
                            pIncarcerator->Respawn();
                        pIncarcerator->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
                    }
                }

                DoUseEmberseerRunes(true);
            }
            else if (uiData == DONE)
            {
                DoUseEmberseerRunes();
                DoUseDoorOrButton(GO_EMBERSEER_OUT);
            }
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_FLAMEWREATH:
            if (uiData == FAIL)
            {
                m_uiFlamewreathEventTimer = 0;
                m_uiFlamewreathWaveCount = 0;
            }
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_STADIUM:
            // Don't set the same data twice
            if (m_auiEncounter[uiType] == uiData)
                break;
            // Combat door
            DoUseDoorOrButton(GO_GYTH_ENTRY_DOOR);
            // Start event
            if (uiData == IN_PROGRESS)
                StartNextDialogueText(SAY_NEFARIUS_INTRO_1);
            else if (uiData == DONE)
                DoUseDoorOrButton(GO_GYTH_EXIT_DOOR);
            else if (uiData == FAIL)
            {
                // Despawn Nefarius and Rend on fail (the others are despawned OnCreatureEvade())
                if (Creature* pNefarius = GetSingleCreatureFromStorage(NPC_LORD_VICTOR_NEFARIUS))
                    pNefarius->ForcedDespawn();
                if (Creature* pRend = GetSingleCreatureFromStorage(NPC_REND_BLACKHAND))
                    pRend->ForcedDespawn();
                if (Creature* pGyth = GetSingleCreatureFromStorage(NPC_GYTH))
                    pGyth->ForcedDespawn();

                m_uiStadiumEventTimer = 0;
                m_uiStadiumMobsAlive = 0;
                m_uiStadiumWaves = 0;
            }
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_DRAKKISATH:
        case TYPE_VALTHALAK:
            m_auiEncounter[uiType] = uiData;
            break;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " " << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_blackrock_spire::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3] >> m_auiEncounter[4] >> m_auiEncounter[5];

    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_blackrock_spire::GetData(uint32 uiType) const
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    return 0;
}

void instance_blackrock_spire::DoSortRoomEventMobs()
{
    if (GetData(TYPE_ROOM_EVENT) != NOT_STARTED)
        return;

    for (uint8 i = 0; i < MAX_ROOMS; ++i)
    {
        if (GameObject* pRune = instance->GetGameObject(m_aRoomRuneGuid[i]))
        {
            for (GuidList::const_iterator itr = m_lRoomEventMobGUIDList.begin(); itr != m_lRoomEventMobGUIDList.end(); ++itr)
            {
                Creature* pCreature = instance->GetCreature(*itr);
                if (pCreature && pCreature->isAlive() && pCreature->GetDistance(pRune) < 10.0f)
                    m_alRoomEventMobGUIDSorted[i].push_back(*itr);
            }
        }
    }

    SetData(TYPE_ROOM_EVENT, IN_PROGRESS);
}

void instance_blackrock_spire::OnCreatureDeath(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_BLACKHAND_SUMMONER:
        case NPC_BLACKHAND_VETERAN:
            // Handle Runes
            if (m_auiEncounter[TYPE_ROOM_EVENT] == IN_PROGRESS)
            {
                uint8 uiNotEmptyRoomsCount = 0;
                for (uint8 i = 0; i < MAX_ROOMS; ++i)
                {
                    if (m_aRoomRuneGuid[i])                 // This check is used, to ensure which runes still need processing
                    {
                        m_alRoomEventMobGUIDSorted[i].remove(pCreature->GetObjectGuid());
                        if (m_alRoomEventMobGUIDSorted[i].empty())
                        {
                            DoUseDoorOrButton(m_aRoomRuneGuid[i]);
                            m_aRoomRuneGuid[i].Clear();
                        }
                        else
                            ++uiNotEmptyRoomsCount;         // found an not empty room
                    }
                }
                if (!uiNotEmptyRoomsCount)
                    SetData(TYPE_ROOM_EVENT, DONE);
            }
            break;
        case NPC_SOLAKAR_FLAMEWREATH:
            SetData(TYPE_FLAMEWREATH, DONE);
            break;
        case NPC_DRAKKISATH:
            SetData(TYPE_DRAKKISATH, DONE);
            DoUseDoorOrButton(GO_DRAKKISATH_DOOR_1);
            DoUseDoorOrButton(GO_DRAKKISATH_DOOR_2);
            break;
        case NPC_CHROMATIC_WHELP:
        case NPC_CHROMATIC_DRAGON:
        case NPC_BLACKHAND_HANDLER:
            // check if it's summoned - some npcs with the same entry are already spawned in the instance
            if (!pCreature->IsTemporarySummon())
                break;
            --m_uiStadiumMobsAlive;
            if (m_uiStadiumMobsAlive == 0)
                DoSendNextStadiumWave();
            break;
        case NPC_GYTH:
        case NPC_REND_BLACKHAND:
            --m_uiStadiumMobsAlive;
            if (m_uiStadiumMobsAlive == 0)
                StartNextDialogueText(SAY_NEFARIUS_VICTORY);
            break;
    }
}

void instance_blackrock_spire::OnCreatureEvade(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
            // Emberseer should evade if the incarcerators evade
        case NPC_BLACKHAND_INCARCERATOR:
            if (Creature* pEmberseer = GetSingleCreatureFromStorage(NPC_PYROGUARD_EMBERSEER))
                pEmberseer->AI()->EnterEvadeMode();
            break;
        case NPC_SOLAKAR_FLAMEWREATH:
        case NPC_ROOKERY_GUARDIAN:
        case NPC_ROOKERY_HATCHER:
            SetData(TYPE_FLAMEWREATH, FAIL);
            break;
        case NPC_CHROMATIC_WHELP:
        case NPC_CHROMATIC_DRAGON:
        case NPC_BLACKHAND_HANDLER:
        case NPC_GYTH:
        case NPC_REND_BLACKHAND:
            // check if it's summoned - some npcs with the same entry are already spawned in the instance
            if (!pCreature->IsTemporarySummon())
                break;
            SetData(TYPE_STADIUM, FAIL);
            pCreature->ForcedDespawn();
            break;
    }
}

void instance_blackrock_spire::OnCreatureEnterCombat(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
            // Once one of the Incarcerators gets Aggro, the door should close
        case NPC_BLACKHAND_INCARCERATOR:
            SetData(TYPE_EMBERSEER, IN_PROGRESS);
            break;
    }
}

void instance_blackrock_spire::OnCreatureDespawn(Creature* pCreature)
{
    if (pCreature->GetEntry() == NPC_THE_BEAST)
        pCreature->CastSpell(pCreature, SPELL_FINKLE_IS_EINHORN, true);
}

void instance_blackrock_spire::DoOpenUpperDoorIfCan(Player* pPlayer)
{
    if (m_bUpperDoorOpened)
        return;

    if (pPlayer->HasItemCount(ITEM_SEAL_OF_ASCENSION, 1))
    {
        m_uiDragonspineDoorTimer = 100;
        m_uiDragonspineGoCount = 0;
        m_bUpperDoorOpened = true;
    }
}

void instance_blackrock_spire::DoProcessEmberseerEvent()
{
    if (GetData(TYPE_EMBERSEER) == DONE || GetData(TYPE_EMBERSEER) == IN_PROGRESS)
        return;

    if (m_lIncarceratorGUIDList.empty())
    {
        script_error_log("Npc %u couldn't be found. Please check your DB content!", NPC_BLACKHAND_INCARCERATOR);
        return;
    }

    // start to grow
    if (Creature* pEmberseer = GetSingleCreatureFromStorage(NPC_PYROGUARD_EMBERSEER))
    {
        // If already casting, return
        if (pEmberseer->HasAura(SPELL_EMBERSEER_GROWING))
            return;

        DoScriptText(EMOTE_BEGIN, pEmberseer);
        pEmberseer->CastSpell(pEmberseer, SPELL_EMBERSEER_GROWING, true);
    }

    // remove the incarcerators flags and stop casting
    for (GuidList::const_iterator itr = m_lIncarceratorGUIDList.begin(); itr != m_lIncarceratorGUIDList.end(); ++itr)
    {
        if (Creature* pCreature = instance->GetCreature(*itr))
        {
            if (pCreature->isAlive())
            {
                pCreature->InterruptNonMeleeSpells(false);
                pCreature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
            }
        }
    }
}

void instance_blackrock_spire::DoUseEmberseerRunes(bool bReset)
{
    if (m_lEmberseerRunesGUIDList.empty())
        return;

    for (GuidList::const_iterator itr = m_lEmberseerRunesGUIDList.begin(); itr != m_lEmberseerRunesGUIDList.end(); ++itr)
    {
        if (bReset)
        {
            if (GameObject* pRune = instance->GetGameObject(*itr))
                pRune->ResetDoorOrButton();
        }
        else
            DoUseDoorOrButton(*itr);
    }
}

void instance_blackrock_spire::JustDidDialogueStep(int32 iEntry)
{
    switch (iEntry)
    {
        case NPC_BLACKHAND_HANDLER:
            m_uiStadiumEventTimer = 1000;
            // Move the two near the balcony
            if (Creature* pRend = GetSingleCreatureFromStorage(NPC_REND_BLACKHAND))
                pRend->SetFacingTo(aStadiumLocs[5].m_fO);
            if (Creature* pNefarius = GetSingleCreatureFromStorage(NPC_LORD_VICTOR_NEFARIUS))
                pNefarius->GetMotionMaster()->MovePoint(0, aStadiumLocs[5].m_fX, aStadiumLocs[5].m_fY, aStadiumLocs[5].m_fZ);
            break;
        case SAY_NEFARIUS_WARCHIEF:
            // Prepare for Gyth - note: Nefarius should be moving around the balcony
            if (Creature* pRend = GetSingleCreatureFromStorage(NPC_REND_BLACKHAND))
            {
                pRend->ForcedDespawn(5000);
                pRend->SetWalk(false);
                pRend->GetMotionMaster()->MovePoint(0, aStadiumLocs[6].m_fX, aStadiumLocs[6].m_fY, aStadiumLocs[6].m_fZ);
            }
            m_uiStadiumEventTimer = 30000;
            break;
        case SAY_NEFARIUS_VICTORY:
            SetData(TYPE_STADIUM, DONE);
            break;
        case NPC_REND_BLACKHAND:
            // Despawn Nefarius
            if (Creature* pNefarius = GetSingleCreatureFromStorage(NPC_LORD_VICTOR_NEFARIUS))
            {
                pNefarius->ForcedDespawn(5000);
                pNefarius->GetMotionMaster()->MovePoint(0, aStadiumLocs[6].m_fX, aStadiumLocs[6].m_fY, aStadiumLocs[6].m_fZ);
            }
            break;
    }
}

void instance_blackrock_spire::DoSendNextStadiumWave()
{
    if (m_uiStadiumWaves < MAX_STADIUM_WAVES)
    {
        // Send current wave mobs
        if (Creature* pNefarius = GetSingleCreatureFromStorage(NPC_LORD_VICTOR_NEFARIUS))
        {
            float fX, fY, fZ;
            for (uint8 i = 0; i < MAX_STADIUM_MOBS_PER_WAVE; ++i)
            {
                if (aStadiumEventNpcs[m_uiStadiumWaves][i] == 0)
                    continue;

                pNefarius->GetRandomPoint(aStadiumLocs[0].m_fX, aStadiumLocs[0].m_fY, aStadiumLocs[0].m_fZ, 7.0f, fX, fY, fZ);
                fX = std::min(aStadiumLocs[0].m_fX, fX);    // Halfcircle - suits better the rectangular form
                if (Creature* pTemp = pNefarius->SummonCreature(aStadiumEventNpcs[m_uiStadiumWaves][i], fX, fY, fZ, 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0))
                {
                    // Get some point in the center of the stadium
                    pTemp->GetRandomPoint(aStadiumLocs[2].m_fX, aStadiumLocs[2].m_fY, aStadiumLocs[2].m_fZ, 5.0f, fX, fY, fZ);
                    fX = std::min(aStadiumLocs[2].m_fX, fX);// Halfcircle - suits better the rectangular form

                    pTemp->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                    ++m_uiStadiumMobsAlive;
                }
            }
        }

        DoUseDoorOrButton(GO_GYTH_COMBAT_DOOR);
    }
    // All waves are cleared - start Gyth intro
    else if (m_uiStadiumWaves == MAX_STADIUM_WAVES)
        StartNextDialogueText(SAY_NEFARIUS_LOSE_4);
    else
    {
        // Send Gyth
        if (Creature* pNefarius = GetSingleCreatureFromStorage(NPC_LORD_VICTOR_NEFARIUS))
        {
            if (Creature* pTemp = pNefarius->SummonCreature(NPC_GYTH, aStadiumLocs[1].m_fX, aStadiumLocs[1].m_fY, aStadiumLocs[1].m_fZ, 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0))
                pTemp->GetMotionMaster()->MovePoint(0, aStadiumLocs[2].m_fX, aStadiumLocs[2].m_fY, aStadiumLocs[2].m_fZ);
        }

        // Set this to 2, because Rend will be summoned later during the fight
        m_uiStadiumMobsAlive = 2;

        DoUseDoorOrButton(GO_GYTH_COMBAT_DOOR);
    }

    ++m_uiStadiumWaves;

    // Stop the timer when all the waves have been sent
    if (m_uiStadiumWaves >= MAX_STADIUM_WAVES)
        m_uiStadiumEventTimer = 0;
    else
        m_uiStadiumEventTimer = 60000;
}

void instance_blackrock_spire::DoSendNextFlamewreathWave()
{
    GameObject* pSummoner = GetSingleGameObjectFromStorage(GO_FATHER_FLAME);
    if (!pSummoner)
        return;

    // TODO - The npcs would move nicer if they had DB waypoints, so i suggest to change their default movement to DB waypoints, and random movement when they reached their goal

    if (m_uiFlamewreathWaveCount < 6)                       // Send two adds (6 waves, then boss)
    {
        Creature* pSummoned = nullptr;
        for (uint8 i = 0; i < 2; ++i)
        {
            float fX, fY, fZ;
            pSummoner->GetRandomPoint(rookeryEventSpawnPos[0], rookeryEventSpawnPos[1], rookeryEventSpawnPos[2], 2.5f, fX, fY, fZ);
            // Summon Rookery Hatchers in first wave, else random
            pSummoned = pSummoner->SummonCreature(urand(0, 1) && m_uiFlamewreathWaveCount ? NPC_ROOKERY_GUARDIAN : NPC_ROOKERY_HATCHER, fX, fY, fZ, 0.0, TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 300000);
            if (pSummoned)
            {
                pSummoner->GetContactPoint(pSummoned, fX, fY, fZ);
                pSummoned->GetMotionMaster()->MovePoint(1, fX, fY, pSummoner->GetPositionZ());
            }
        }
        if (pSummoned && m_uiFlamewreathWaveCount == 0)
            DoScriptText(SAY_ROOKERY_EVENT_START, pSummoned);

        if (m_uiFlamewreathWaveCount < 4)
            m_uiFlamewreathEventTimer = 30000;
        else if (m_uiFlamewreathWaveCount < 6)
            m_uiFlamewreathEventTimer = 40000;
        else
            m_uiFlamewreathEventTimer = 10000;

        ++m_uiFlamewreathWaveCount;
    }
    else                                                    // Send Flamewreath
    {
        if (Creature* pSolakar = pSummoner->SummonCreature(NPC_SOLAKAR_FLAMEWREATH, rookeryEventSpawnPos[0], rookeryEventSpawnPos[1], rookeryEventSpawnPos[2], 0.0f, TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, HOUR * IN_MILLISECONDS))
            pSolakar->GetMotionMaster()->MovePoint(1, pSummoner->GetPositionX(), pSummoner->GetPositionY(), pSummoner->GetPositionZ());
        SetData(TYPE_FLAMEWREATH, SPECIAL);
        m_uiFlamewreathEventTimer = 0;
    }
}

void instance_blackrock_spire::Update(uint32 uiDiff)
{
    DialogueUpdate(uiDiff);

    if (m_uiStadiumEventTimer)
    {
        if (m_uiStadiumEventTimer <= uiDiff)
            DoSendNextStadiumWave();
        else
            m_uiStadiumEventTimer -= uiDiff;
    }

    if (m_uiFlamewreathEventTimer)
    {
        if (m_uiFlamewreathEventTimer <= uiDiff)
            DoSendNextFlamewreathWave();
        else
            m_uiFlamewreathEventTimer -= uiDiff;
    }

    // unlock dragon spine door
    if (m_uiDragonspineDoorTimer)
    {
        if (m_uiDragonspineDoorTimer <= uiDiff)
        {
            switch (m_uiDragonspineGoCount)
            {
                case 0:
                    DoUseDoorOrButton(GO_BRAZIER_1);
                    DoUseDoorOrButton(GO_BRAZIER_2);
                    break;
                case 1:
                    DoUseDoorOrButton(GO_BRAZIER_3);
                    DoUseDoorOrButton(GO_BRAZIER_4);
                    break;
                case 2:
                    DoUseDoorOrButton(GO_BRAZIER_5);
                    DoUseDoorOrButton(GO_BRAZIER_6);
                    break;
                case 3:
                    DoUseDoorOrButton(GO_DRAGONSPINE);
                    break;
            }
            ++m_uiDragonspineGoCount;

            if (m_uiDragonspineGoCount >= 4)
                m_uiDragonspineDoorTimer = 0;
            else
                m_uiDragonspineDoorTimer = 1000;
        }
        else
            m_uiDragonspineDoorTimer -= uiDiff;
    }
}

void instance_blackrock_spire::StartflamewreathEventIfCan()
{
    // Already done or currently in progress - or endboss done
    if (m_auiEncounter[TYPE_FLAMEWREATH] == DONE || m_auiEncounter[TYPE_FLAMEWREATH] == IN_PROGRESS || m_auiEncounter[TYPE_DRAKKISATH] == DONE)
        return;

    // Boss still around
    if (GetSingleCreatureFromStorage(NPC_SOLAKAR_FLAMEWREATH, true))
        return;

    // Start summoning of mobs
    m_uiFlamewreathEventTimer = 1;
    m_uiFlamewreathWaveCount = 0;
}

InstanceData* GetInstanceData_instance_blackrock_spire(Map* pMap)
{
    return new instance_blackrock_spire(pMap);
}

bool AreaTrigger_at_blackrock_spire(Player* pPlayer, AreaTriggerEntry const* pAt)
{
    if (!pPlayer->isAlive() || pPlayer->isGameMaster())
        return false;

    switch (pAt->id)
    {
        case AREATRIGGER_ENTER_UBRS:
            if (instance_blackrock_spire* pInstance = (instance_blackrock_spire*) pPlayer->GetInstanceData())
            {
                pInstance->DoOpenUpperDoorIfCan(pPlayer);
                pInstance->DoSortRoomEventMobs();
            }
            break;
        case AREATRIGGER_STADIUM:
            if (instance_blackrock_spire* pInstance = (instance_blackrock_spire*) pPlayer->GetInstanceData())
            {
                if (pInstance->GetData(TYPE_STADIUM) == IN_PROGRESS || pInstance->GetData(TYPE_STADIUM) == DONE)
                    return false;

                // Summon Nefarius and Rend for the dialogue event
                // Note: Nefarius and Rend need to be hostile and not attackable
                if (Creature* pNefarius = pPlayer->SummonCreature(NPC_LORD_VICTOR_NEFARIUS, aStadiumLocs[3].m_fX, aStadiumLocs[3].m_fY, aStadiumLocs[3].m_fZ, aStadiumLocs[3].m_fO, TEMPSUMMON_CORPSE_DESPAWN, 0))
                    pNefarius->SetFactionTemporary(FACTION_BLACK_DRAGON, TEMPFACTION_NONE);
                if (Creature* pRend = pPlayer->SummonCreature(NPC_REND_BLACKHAND, aStadiumLocs[4].m_fX, aStadiumLocs[4].m_fY, aStadiumLocs[4].m_fZ, aStadiumLocs[4].m_fO, TEMPSUMMON_CORPSE_DESPAWN, 0))
                    pRend->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);

                pInstance->SetData(TYPE_STADIUM, IN_PROGRESS);
            }
            break;
    }
    return false;
}

bool ProcessEventId_event_spell_altar_emberseer(uint32 /*uiEventId*/, Object* pSource, Object* /*pTarget*/, bool bIsStart)
{
    if (bIsStart && pSource->GetTypeId() == TYPEID_PLAYER)
    {
        if (instance_blackrock_spire* pInstance = (instance_blackrock_spire*)((Player*)pSource)->GetInstanceData())
        {
            pInstance->DoProcessEmberseerEvent();
            return true;
        }
    }
    return false;
}

bool GOUse_go_father_flame(Player* /*pPlayer*/, GameObject* pGo)
{
    if (instance_blackrock_spire* pInstance = (instance_blackrock_spire*)pGo->GetInstanceData())
        pInstance->StartflamewreathEventIfCan();

    return true;
}

void AddSC_instance_blackrock_spire()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_blackrock_spire";
    pNewScript->GetInstanceData = &GetInstanceData_instance_blackrock_spire;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_blackrock_spire";
    pNewScript->pAreaTrigger = &AreaTrigger_at_blackrock_spire;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "event_spell_altar_emberseer";
    pNewScript->pProcessEventId = &ProcessEventId_event_spell_altar_emberseer;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_father_flame";
    pNewScript->pGOUse = &GOUse_go_father_flame;
    pNewScript->RegisterSelf();
}
