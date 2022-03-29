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
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "blackrock_spire.h"

enum
{
    AREATRIGGER_ENTER_UBRS      = 2046,
    AREATRIGGER_STADIUM         = 2026,
    AREATRIGGER_BEAST_AGGRO     = 2066,
    AREATRIGGER_BEAST_INTRO     = 2067,

    // Arena event dialogue intro and outro - handled by instance
    SAY_NEFARIUS_INTRO_1        = -1229004,
    SAY_NEFARIUS_INTRO_2        = -1229005,
    SAY_NEFARIUS_LOSE1          = -1229014,
    SAY_REND_ATTACK             = -1229015,
    SAY_NEFARIUS_WARCHIEF       = -1229016,
    SAY_NEFARIUS_VICTORY        = -1229018,

    // Arena event random taunt - handled on creature death
    SAY_NEFARIUS_TAUNT1         = -1229006,
    SAY_NEFARIUS_TAUNT2         = -1229008,
    SAY_NEFARIUS_TAUNT3         = -1229009,
    SAY_NEFARIUS_TAUNT4         = -1229010,
    SAY_NEFARIUS_TAUNT5         = -1229013,
    SAY_NEFARIUS_TAUNT6         = -1229021,
    SAY_NEFARIUS_TAUNT7         = -1229022,
    SAY_NEFARIUS_TAUNT8         = -1229023,
    SAY_REND_TAUNT1             = -1229007,
    SAY_REND_TAUNT2             = -1229011,
    SAY_REND_TAUNT3             = -1229012,
    SAY_REND_TAUNT4             = -1229024,

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
    {SAY_NEFARIUS_LOSE1,       NPC_LORD_VICTOR_NEFARIUS,   3000},
    {SAY_REND_ATTACK,           NPC_REND_BLACKHAND,         2000},
    {SAY_NEFARIUS_WARCHIEF,     NPC_LORD_VICTOR_NEFARIUS,   0},
    {SAY_NEFARIUS_VICTORY,      NPC_LORD_VICTOR_NEFARIUS,   5000},
    {NPC_REND_BLACKHAND,        0,                          0},
    {0, 0, 0},
};

static const float rookeryEventSpawnPos[3] = {51.11098f, -266.0549f, 92.87846f};

instance_blackrock_spire::instance_blackrock_spire(Map* pMap) : ScriptedInstance(pMap), DialogueHelper(aStadiumDialogue),
    m_bUpperDoorOpened(false),
    m_uiDragonspineDoorTimer(0),
    m_uiDragonspineGoCount(0),
    m_bBeastIntroDone(false),
    m_bBeastOutOfLair(false),
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
    m_goEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
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
        case NPC_THE_BEAST:
            m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
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
                        if (!pIncarcerator->IsAlive())
                            pIncarcerator->Respawn();
                        pIncarcerator->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
                        pIncarcerator->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
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
            {
                // Event complete: remove the summoned spectators
                for (GuidList::const_iterator itr = m_lStadiumSpectatorsGUIDList.begin(); itr != m_lStadiumSpectatorsGUIDList.end(); ++itr)
                {
                    Creature* pSpectator = instance->GetCreature(*itr);
                    if (pSpectator)
                        pSpectator->ForcedDespawn();
                }
                DoUseDoorOrButton(GO_GYTH_EXIT_DOOR);
            }
            else if (uiData == FAIL)
            {
                // Despawn Nefarius, Rend and the spectators on fail (the others are despawned OnCreatureEvade())
                if (Creature* pNefarius = GetSingleCreatureFromStorage(NPC_LORD_VICTOR_NEFARIUS))
                    pNefarius->ForcedDespawn();
                if (Creature* pRend = GetSingleCreatureFromStorage(NPC_REND_BLACKHAND))
                    pRend->ForcedDespawn();
                if (Creature* pGyth = GetSingleCreatureFromStorage(NPC_GYTH))
                    pGyth->ForcedDespawn();
                for (GuidList::const_iterator itr = m_lStadiumSpectatorsGUIDList.begin(); itr != m_lStadiumSpectatorsGUIDList.end(); ++itr)
                {
                    Creature* pSpectator = instance->GetCreature(*itr);
                    if (pSpectator)
                        pSpectator->ForcedDespawn();
                }

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

    for (uint32& i : m_auiEncounter)
    {
        if (i == IN_PROGRESS)
            i = NOT_STARTED;
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
                if (pCreature && pCreature->IsAlive() && pCreature->GetDistance(pRune) < 10.0f)
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

            // 5% chance for Rend or Lord Victor Nefarius to taunt players when one of the creature is killed (% is guesswork)
            // Lord Victor Nefarius
            if (urand(0, 100) < 5)
            {
                if (Creature* pNefarius = GetSingleCreatureFromStorage(NPC_LORD_VICTOR_NEFARIUS))
                {
                    switch (urand(0, 7))
                    {
                        case 0: DoScriptText(SAY_NEFARIUS_TAUNT1, pNefarius); break;
                        case 1: DoScriptText(SAY_NEFARIUS_TAUNT2, pNefarius); break;
                        case 2: DoScriptText(SAY_NEFARIUS_TAUNT3, pNefarius); break;
                        case 3: DoScriptText(SAY_NEFARIUS_TAUNT4, pNefarius); break;
                        case 4: DoScriptText(SAY_NEFARIUS_TAUNT5, pNefarius); break;
                        case 5: DoScriptText(SAY_NEFARIUS_TAUNT6, pNefarius); break;
                        case 6: DoScriptText(SAY_NEFARIUS_TAUNT7, pNefarius); break;
                        case 7: DoScriptText(SAY_NEFARIUS_TAUNT8, pNefarius); break;
                    }
                }
            }
            // Warchief Rend Blackhand
            if (urand(0, 100) < 5)
            {
                if (Creature* pRend = GetSingleCreatureFromStorage(NPC_REND_BLACKHAND))
                {
                    switch (urand(0, 3))
                    {
                        case 0: DoScriptText(SAY_REND_TAUNT1, pRend); break;
                        case 1: DoScriptText(SAY_REND_TAUNT2, pRend); break;
                        case 2: DoScriptText(SAY_REND_TAUNT3, pRend); break;
                        case 3: DoScriptText(SAY_REND_TAUNT4, pRend); break;
                    }
                }
            }
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
        pCreature->CastSpell(pCreature, SPELL_FINKLE_IS_EINHORN, TRIGGERED_OLD_TRIGGERED);
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
        pEmberseer->CastSpell(pEmberseer, SPELL_EMBERSEER_GROWING, TRIGGERED_OLD_TRIGGERED);
    }

    // remove the incarcerators flags and stop casting
    for (GuidList::const_iterator itr = m_lIncarceratorGUIDList.begin(); itr != m_lIncarceratorGUIDList.end(); ++itr)
    {
        if (Creature* pCreature = instance->GetCreature(*itr))
        {
            if (pCreature->IsAlive())
            {
                pCreature->InterruptNonMeleeSpells(false);
                pCreature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
                pCreature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
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
                pRend->SetFacingTo(aStadiumLocs[5].o);
            if (Creature* pNefarius = GetSingleCreatureFromStorage(NPC_LORD_VICTOR_NEFARIUS))
            {
                pNefarius->GetMotionMaster()->MovePoint(0, aStadiumLocs[5].x, aStadiumLocs[5].y, aStadiumLocs[5].z);
                // Summon the spectators and move them to the western balcony
                for (uint8 i = 0; i < 12; i++)
                {
                    Creature* pSpectator = pNefarius->SummonCreature(aStadiumSpectators[i], aSpectatorsSpawnLocs[i].x, aSpectatorsSpawnLocs[i].y, aSpectatorsSpawnLocs[i].z, aSpectatorsSpawnLocs[i].o, TEMPSPAWN_DEAD_DESPAWN, 0);
                    if (pSpectator)
                    {
                        pSpectator->SetFacingTo(aSpectatorsTargetLocs[i].o);
                        pSpectator->SetWalk(false);
                        pSpectator->GetMotionMaster()->MovePoint(0, aSpectatorsTargetLocs[i].x, aSpectatorsTargetLocs[i].y, aSpectatorsTargetLocs[i].z);
                        pSpectator->AI()->SetReactState(REACT_DEFENSIVE);
                        m_lStadiumSpectatorsGUIDList.push_back(pSpectator->GetObjectGuid());
                    }
                }
            }
            break;
        case SAY_NEFARIUS_WARCHIEF:
            // Prepare for Gyth
            if (Creature* pRend = GetSingleCreatureFromStorage(NPC_REND_BLACKHAND))
            {
                pRend->ForcedDespawn(5000);
                pRend->SetWalk(false);
                pRend->GetMotionMaster()->MovePoint(0, aStadiumLocs[6].x, aStadiumLocs[6].y, aStadiumLocs[6].z);
            }
            // Make Lord Nefarius walk back and forth while Rend is preparing Glyth
            if (Creature* pNefarius = GetSingleCreatureFromStorage(NPC_LORD_VICTOR_NEFARIUS))
                pNefarius->GetMotionMaster()->MoveWaypoint(0);
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
                pNefarius->GetMotionMaster()->MovePoint(0, aStadiumLocs[6].x, aStadiumLocs[6].y, aStadiumLocs[6].z);
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

                pNefarius->GetRandomPoint(aStadiumLocs[0].x, aStadiumLocs[0].y, aStadiumLocs[0].z, 7.0f, fX, fY, fZ);
                fX = std::min(aStadiumLocs[0].x, fX);    // Halfcircle - suits better the rectangular form
                if (Creature* pTemp = pNefarius->SummonCreature(aStadiumEventNpcs[m_uiStadiumWaves][i], fX, fY, fZ, 0.0f, TEMPSPAWN_DEAD_DESPAWN, 0))
                {
                    // Get some point in the center of the stadium
                    pTemp->GetRandomPoint(aStadiumLocs[2].x, aStadiumLocs[2].y, aStadiumLocs[2].z, 5.0f, fX, fY, fZ);
                    fX = std::min(aStadiumLocs[2].x, fX);// Halfcircle - suits better the rectangular form

                    pTemp->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                    ++m_uiStadiumMobsAlive;
                }
            }
        }

        DoUseDoorOrButton(GO_GYTH_COMBAT_DOOR);
    }
    // All waves are cleared - start Gyth intro
    else if (m_uiStadiumWaves == MAX_STADIUM_WAVES)
        StartNextDialogueText(SAY_NEFARIUS_LOSE1);
    else
    {
        // Send Gyth
        if (Creature* pNefarius = GetSingleCreatureFromStorage(NPC_LORD_VICTOR_NEFARIUS))
        {
            // Stop Lord Nefarius from moving and put him back in place
            pNefarius->GetMotionMaster()->MoveIdle();
            pNefarius->GetMotionMaster()->MovePoint(0, aStadiumLocs[5].x, aStadiumLocs[5].y, aStadiumLocs[5].z);

            if (Creature* pTemp = pNefarius->SummonCreature(NPC_GYTH, aStadiumLocs[1].x, aStadiumLocs[1].y, aStadiumLocs[1].z, 0.0f, TEMPSPAWN_DEAD_DESPAWN, 0))
                pTemp->GetMotionMaster()->MovePoint(0, aStadiumLocs[2].x, aStadiumLocs[2].y, aStadiumLocs[2].z);
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
            pSummoned = pSummoner->SummonCreature(urand(0, 1) && m_uiFlamewreathWaveCount ? NPC_ROOKERY_GUARDIAN : NPC_ROOKERY_HATCHER, fX, fY, fZ, 0.0, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 300000);
            if (pSummoned)
            {
                //pSummoner->GetContactPoint(pSummoned, fX, fY, fZ);
                //pSummoned->GetMotionMaster()->MovePoint(1, fX, fY, pSummoner->GetPositionZ());
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
        if (Creature* pSolakar = pSummoner->SummonCreature(NPC_SOLAKAR_FLAMEWREATH, rookeryEventSpawnPos[0], rookeryEventSpawnPos[1], rookeryEventSpawnPos[2], 0.0f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, HOUR * IN_MILLISECONDS))
            //pSolakar->GetMotionMaster()->MovePoint(1, pSummoner->GetPositionX(), pSummoner->GetPositionY(), pSummoner->GetPositionZ());
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
    if (!pPlayer->IsAlive() || pPlayer->IsGameMaster())
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
                if (Creature* pNefarius = pPlayer->SummonCreature(NPC_LORD_VICTOR_NEFARIUS, aStadiumLocs[3].x, aStadiumLocs[3].y, aStadiumLocs[3].z, aStadiumLocs[3].o, TEMPSPAWN_CORPSE_DESPAWN, 0))
                    pNefarius->SetFactionTemporary(FACTION_BLACK_DRAGON, TEMPFACTION_NONE);
                if (Creature* pRend = pPlayer->SummonCreature(NPC_REND_BLACKHAND, aStadiumLocs[4].x, aStadiumLocs[4].y, aStadiumLocs[4].z, aStadiumLocs[4].o, TEMPSPAWN_CORPSE_DESPAWN, 0))
                    pRend->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);

                pInstance->SetData(TYPE_STADIUM, IN_PROGRESS);
            }
            break;

        // Intro event when players enter The Furnace: three Blackhand Elite are spawned, flee from the Beast and are "killed" by it
        // The Blackhand Elite scripts are handled in database by waypoints movement
        case AREATRIGGER_BEAST_INTRO:
            if (instance_blackrock_spire* pInstance = (instance_blackrock_spire*) pPlayer->GetInstanceData())
            {
                if (pInstance->m_bBeastIntroDone)
                    return false;

                if (Creature* pBeast = pInstance->GetSingleCreatureFromStorage(NPC_THE_BEAST))
                {
                    for (uint8 i = 0; i < 3; i++)
                    {
                        if (Creature* pTemp = pBeast->SummonCreature(NPC_BLACKHAND_ELITE, aBeastLocs[i].x, aBeastLocs[i].y, aBeastLocs[i].z, aBeastLocs[i].o, TEMPSPAWN_DEAD_DESPAWN, 0))
                            pTemp->GetMotionMaster()->MoveWaypoint(i);
                    }
                    pInstance->m_bBeastIntroDone = true;
                }
            }
            break;

        // Make the Beast move around the room unless it finds some target.
        case AREATRIGGER_BEAST_AGGRO:
            if (instance_blackrock_spire* pInstance = (instance_blackrock_spire*) pPlayer->GetInstanceData())
            {
                if (pInstance->m_bBeastOutOfLair)
                    return false;

                if (Creature* pBeast = pInstance->GetSingleCreatureFromStorage(NPC_THE_BEAST))
                {
                    pBeast->GetMotionMaster()->MoveWaypoint(0);
                    pInstance->m_bBeastOutOfLair = true;

                    // Play the intro if not already done
                    if (!pInstance->m_bBeastIntroDone)
                    {
                        for (uint8 i = 0; i < 3; i++)
                        {
                            if (Creature* pTemp = pBeast->SummonCreature(NPC_BLACKHAND_ELITE, aBeastLocs[i].x, aBeastLocs[i].y, aBeastLocs[i].z, aBeastLocs[i].o, TEMPSPAWN_DEAD_DESPAWN, 0))
                                pTemp->GetMotionMaster()->MoveWaypoint(i);
                        }
                        pInstance->m_bBeastIntroDone = true;
                    }
                }
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

enum
{
    SPELL_STRIKE            = 15580,
    SPELL_SUNDER_ARMOR      = 15572,
    SPELL_DISTURB_EGGS      = 15746,
};

struct npc_rookery_hatcherAI : public ScriptedAI
{
    npc_rookery_hatcherAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_blackrock_spire*) pCreature->GetInstanceData();
        Reset();
    }

    instance_blackrock_spire* m_pInstance;

    uint32 uiStrikeTimer;
    uint32 uiSunderArmorTimer;
    uint32 uiDisturbEggsTimer;
    uint32 uiWaitTimer;

    bool m_bIsMovementActive;

    void Reset() override
    {
        uiStrikeTimer           = urand(5000, 7000);
        uiSunderArmorTimer      = 5000;
        uiDisturbEggsTimer      = urand(8000, 10000);
        uiWaitTimer             = 0;

        m_bIsMovementActive     = false;
    }

    void MovementInform(uint32 uiMoveType, uint32 uiPointId) override
    {
        if (uiMoveType != POINT_MOTION_TYPE || !uiPointId)
            return;

        m_creature->GetMotionMaster()->MoveIdle();
        m_bIsMovementActive  = false;
        uiWaitTimer = 2000;

        if (DoCastSpellIfCan(m_creature, SPELL_DISTURB_EGGS) == CAST_OK)
            uiDisturbEggsTimer = urand(7500, 10000);
    }

    // Function to search for new rookery egg in range
    void DoFindNewEgg()
    {
        GameObjectList lEggsInRange;
        GetGameObjectListWithEntryInGrid(lEggsInRange, m_creature, GO_ROOKERY_EGG, 20.0f);

        if (lEggsInRange.empty())   // No GO found
            return;

        lEggsInRange.sort(ObjectDistanceOrder(m_creature));
        GameObject* pNearestEgg = nullptr;

        // Always need to find new ones
        for (GameObjectList::const_iterator itr = lEggsInRange.begin(); itr != lEggsInRange.end(); ++itr)
        {
            if (!((*itr)->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE)))
            {
                pNearestEgg = *itr;
                break;
            }
        }

        if (!pNearestEgg)
            return;

        float fX, fY, fZ;
        pNearestEgg->GetContactPoint(m_creature, fX, fY, fZ, 1.0f);
        m_creature->SetWalk(false);
        m_creature->GetMotionMaster()->MovePoint(1, fX, fY, fZ);
        m_bIsMovementActive = true;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Return since we have no target or are disturbing an egg
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim() || m_bIsMovementActive)
            return;

        if (uiWaitTimer)
        {
            if (uiWaitTimer < uiDiff)
            {
                uiWaitTimer = 0;
                m_creature->GetMotionMaster()->MoveChase(m_creature->GetVictim());
            }
            else
                uiWaitTimer -= uiDiff;
        }

        //  Strike Timer
        if (uiStrikeTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_STRIKE) == CAST_OK)
                uiStrikeTimer = urand(4000, 6000);
        }
        else
            uiStrikeTimer -= uiDiff;

        // Sunder Armor timer
        if (uiSunderArmorTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_SUNDER_ARMOR) == CAST_OK)
                uiSunderArmorTimer = 5000;
        }
        else
            uiSunderArmorTimer -= uiDiff;

        // Disturb Rookery Eggs timer
        if (uiDisturbEggsTimer < uiDiff)
        {
            m_bIsMovementActive = false;
            DoFindNewEgg();
        }
        else
            uiDisturbEggsTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_rookery_hatcher(Creature* pCreature)
{
    return new npc_rookery_hatcherAI(pCreature);
}

void AddSC_instance_blackrock_spire()
{
    Script* pNewScript = new Script;
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

    pNewScript = new Script;
    pNewScript->Name = "npc_rookery_hatcher";
    pNewScript->GetAI = &GetAI_npc_rookery_hatcher;
    pNewScript->RegisterSelf();
}
