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
SDName: Instance_Blackwing_Lair
SD%Complete: 100
SDComment:
SDCategory: Blackwing Lair
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "blackwing_lair.h"

instance_blackwing_lair::instance_blackwing_lair(Map* pMap) : ScriptedInstance(pMap),
    m_uiResetTimer(0),
    m_uiDefenseTimer(0),
    m_uiScepterEpicTimer(0),
    m_uiNefarianSpawnTimer(0),
    m_uiScepterQuestStep(0),
    m_uiDragonspawnCount(0),
    m_uiBlackwingDefCount(0),
    m_uiDeadDrakonidsCount(0),
    m_bIsMainGateOpen(true)
{
    Initialize();
}

void instance_blackwing_lair::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

bool instance_blackwing_lair::IsEncounterInProgress() const
{
    for (uint32 i : m_auiEncounter)
    {
        if (i == IN_PROGRESS)
            return true;
    }
    return false;
}

void instance_blackwing_lair::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_MONSTER_GENERATOR:
            m_vGeneratorGuids.push_back(pCreature->GetObjectGuid());
            break;
        case NPC_BLACKWING_LEGIONNAIRE:
        case NPC_BLACKWING_MAGE:
        case NPC_DRAGONSPAWN:
            // Increase the NPC counter depending on dragonspawn or not
            if (pCreature->GetEntry() == NPC_DRAGONSPAWN)
                m_uiDragonspawnCount++;
            else
                m_uiBlackwingDefCount++;
            // Egg room defenders attack players and Razorgore on spawn
            pCreature->SetInCombatWithZone();
            m_lDefendersGuids.push_back(pCreature->GetObjectGuid());
            break;
        // Nefarian encounter
        case NPC_BLACK_SPAWNER:
        case NPC_RED_SPAWNER:
        case NPC_GREEN_SPAWNER:
        case NPC_BRONZE_SPAWNER:
        case NPC_BLUE_SPAWNER:
            m_lDrakonidSpawnerGuids.push_back(pCreature->GetObjectGuid());
            break;
        case NPC_BLACK_DRAKONID:
        case NPC_RED_DRAKONID:
        case NPC_GREEN_DRAKONID:
        case NPC_BLUE_DRAKONID:
        case NPC_BRONZE_DRAKONID:
        case NPC_CHROMATIC_DRAKONID:
            pCreature->SetInCombatWithZone();
            break;
        case NPC_RAZORGORE:
        case NPC_NEFARIANS_TROOPS:
        case NPC_BLACKWING_ORB_TRIGGER:
        case NPC_VAELASTRASZ:
        case NPC_LORD_VICTOR_NEFARIUS:
            m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
    }
}

void instance_blackwing_lair::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_DOOR_RAZORGORE_ENTER:
        case GO_ORB_OF_DOMINATION:
        case GO_DOOR_NEFARIAN:
            break;
        case GO_DOOR_RAZORGORE_EXIT:
            if (m_auiEncounter[TYPE_RAZORGORE] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_DOOR_CHROMAGGUS_EXIT:
            if (m_auiEncounter[TYPE_CHROMAGGUS] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_DOOR_VAELASTRASZ:
            if (m_auiEncounter[TYPE_VAELASTRASZ] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_DOOR_LASHLAYER:
            if (m_auiEncounter[TYPE_LASHLAYER] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_BLACK_DRAGON_EGG:
            m_lDragonEggsGuids.push_back(pGo->GetObjectGuid());
            return;
        case GO_DRAKONID_BONES:
            m_lDrakonidBonesGuids.push_back(pGo->GetObjectGuid());
            return;
        case GO_SUPPRESSION_DEVICE:
            // Do not spawn the Suppression Device GOs if Broodlord Lashlayer is dead
            if (GetData(TYPE_LASHLAYER) == DONE)
                pGo->SetLootState(GO_JUST_DEACTIVATED);
            return;

        default:
            return;
    }
    m_goEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
}

void instance_blackwing_lair::SetData(uint32 uiType, uint32 uiData)
{
    // Close de the main gate whenever an event starts (if it is not already open)
    if (m_bIsMainGateOpen && (uiData == IN_PROGRESS || uiData == SPECIAL))
    {
        DoUseDoorOrButton(GO_DOOR_RAZORGORE_ENTER);
        m_bIsMainGateOpen = false;
    }
    // If an encounter is failed or won, open the main gate only if it is currently closed and no other event is in progress
    else if (!m_bIsMainGateOpen && (uiData == FAIL || uiData == DONE))
    {
        bool ShouldKeepGateClosed = false;
        for (uint8 i = 0; i < TYPE_NEFARIAN; i++)
        {
            if (uiType != i && (m_auiEncounter[i] == IN_PROGRESS || m_auiEncounter[i] == SPECIAL))
                ShouldKeepGateClosed = true;
        }

        if (!ShouldKeepGateClosed)
        {
            DoUseDoorOrButton(GO_DOOR_RAZORGORE_ENTER);
            m_bIsMainGateOpen = true;
        }
    }

    switch (uiType)
    {
        case TYPE_RAZORGORE:
            m_auiEncounter[uiType] = uiData;
            if (uiData == DONE)
                DoUseDoorOrButton(GO_DOOR_RAZORGORE_EXIT);
            else if (uiData == FAIL)
            {
                m_uiResetTimer = 30000;

                // Reset the Orb of Domination and the eggs
                DoToggleGameObjectFlags(GO_ORB_OF_DOMINATION, GO_FLAG_NO_INTERACT, true);
                if (Creature* pOrb = GetSingleCreatureFromStorage(NPC_BLACKWING_ORB_TRIGGER))
                {
                    if (pOrb->isAlive())
                        pOrb->AI()->EnterEvadeMode();
                }

                // Reset defenders
                for (GuidList::const_iterator itr = m_lDefendersGuids.begin(); itr != m_lDefendersGuids.end(); ++itr)
                {
                    if (Creature* pDefender = instance->GetCreature(*itr))
                        pDefender->ForcedDespawn();
                }

                m_lUsedEggsGuids.clear();
                m_lDefendersGuids.clear();
                m_uiBlackwingDefCount = 0;
                m_uiDragonspawnCount = 0;
            }
            break;
        case TYPE_VAELASTRASZ:
            m_auiEncounter[uiType] = uiData;
            // Prevent the players from running back to the first room; use if the encounter is not special
            if (uiData != SPECIAL)
                DoUseDoorOrButton(GO_DOOR_RAZORGORE_EXIT);
            if (uiData == DONE)
                DoUseDoorOrButton(GO_DOOR_VAELASTRASZ);
            break;
        case TYPE_LASHLAYER:
            m_auiEncounter[uiType] = uiData;
            if (uiData == DONE)
                DoUseDoorOrButton(GO_DOOR_LASHLAYER);
            break;
        case TYPE_FIREMAW:
        case TYPE_EBONROC:
        case TYPE_FLAMEGOR:
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_CHROMAGGUS:
            m_auiEncounter[uiType] = uiData;
            if (uiData == DONE)
                DoUseDoorOrButton(GO_DOOR_CHROMAGGUS_EXIT);
            break;
        case TYPE_NEFARIAN:
            // If epic quest for Scepter of the Shifting Sands is in progress when Nefarian is defeated mark it as complete
            if (uiData == DONE && GetData(TYPE_QUEST_SCEPTER) == IN_PROGRESS)
                SetData(TYPE_QUEST_SCEPTER, DONE);

            // Don't store the same thing twice
            if (m_auiEncounter[uiType] == uiData)
                break;

            if (uiData == SPECIAL)
            {
                m_auiEncounter[uiType] = uiData;
                m_uiNefarianSpawnTimer = 5 * IN_MILLISECONDS;   // End of phase 1: spawn Nefarian 5 seconds later

                // Remove drakonids spawners and Lord Victor Nefarius
                CleanupNefarianStage(false);
                break;
            }
            if (uiData == FAIL)
                CleanupNefarianStage(true); // Cleanup the drakonid bones, dead drakonids count and spawners
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(GO_DOOR_NEFARIAN);
            break;
        case TYPE_QUEST_SCEPTER:
            m_auiEncounter[uiType] = uiData;
            // Start 5 hours timer (various steps are handled in Update()
            if (uiData == IN_PROGRESS)
            {
                m_uiScepterEpicTimer = 2000;
                m_uiScepterQuestStep = 0;
            }
            // Stop timer
            if (uiData == DONE)
                m_uiScepterEpicTimer = 0;
            break;
        case TYPE_CHROMA_LBREATH:
        case TYPE_CHROMA_RBREATH:
        case TYPE_NEFA_LTUNNEL:
        case TYPE_NEFA_RTUNNEL:
            m_auiEncounter[uiType] = uiData;    // Store the spell IDs/NPC entries of the two breaths/drakonids used by Chromaggus/Nefarian for all the instance lifetime. Breaths/Drakonids are picked randomly in Chromaggus/Nefarian script
            break;
    }

    if (uiData >= DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                   << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " "
                   << m_auiEncounter[6] << " " << m_auiEncounter[7] << " " << m_auiEncounter[8] << " "
                   << m_auiEncounter[9] << " " << m_auiEncounter[10]<< " " << m_auiEncounter[11] << " "
                   << m_auiEncounter[12];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_blackwing_lair::Load(const char* chrIn)
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
               >> m_auiEncounter[8] >> m_auiEncounter[9] >> m_auiEncounter[10]>> m_auiEncounter[11]
               >> m_auiEncounter[12];

    for (uint32& i : m_auiEncounter)
    {
        if (i == IN_PROGRESS)
            i = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_blackwing_lair::GetData(uint32 uiType) const
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    return 0;
}

void instance_blackwing_lair::SetData64(uint32 uiData, uint64 uiGuid)
{
    if (uiData == DATA_DRAGON_EGG)
    {
        if (GameObject* pEgg = instance->GetGameObject(ObjectGuid(uiGuid)))
            m_lUsedEggsGuids.push_back(pEgg->GetObjectGuid());

        // If all eggs are destroyed, then allow Razorgore to be attacked
        if (m_lUsedEggsGuids.size() == m_lDragonEggsGuids.size())
        {
            SetData(TYPE_RAZORGORE, SPECIAL);
            DoToggleGameObjectFlags(GO_ORB_OF_DOMINATION, GO_FLAG_NO_INTERACT, true);

            // Emote for the start of the second phase
            if (Creature* pTrigger = GetSingleCreatureFromStorage(NPC_NEFARIANS_TROOPS))
            {
                DoScriptText(EMOTE_ORB_SHUT_OFF, pTrigger);
                DoScriptText(EMOTE_TROOPS_FLEE, pTrigger);
            }

            // Break mind control and set max health
            if (Creature* pRazorgore = GetSingleCreatureFromStorage(NPC_RAZORGORE))
            {
                pRazorgore->RemoveAllAuras();
                pRazorgore->CastSpell(pRazorgore, SPELL_WARMING_FLAMES, TRIGGERED_OLD_TRIGGERED);
            }

            // All defenders evade and despawn
            for (GuidList::const_iterator itr = m_lDefendersGuids.begin(); itr != m_lDefendersGuids.end(); ++itr)
            {
                if (Creature* pDefender = instance->GetCreature(*itr))
                {
                    pDefender->AI()->EnterEvadeMode();
                    pDefender->ForcedDespawn(10000);
                }
            }
            m_uiBlackwingDefCount = 0;
            m_uiDragonspawnCount = 0;
        }
    }
}

void instance_blackwing_lair::OnCreatureEnterCombat(Creature* pCreature)
{
    if (pCreature->GetEntry() == NPC_GRETHOK_CONTROLLER)
    {
        SetData(TYPE_RAZORGORE, IN_PROGRESS);
        m_uiDefenseTimer = 40000;
    }
}

void instance_blackwing_lair::OnCreatureDeath(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_GRETHOK_CONTROLLER:
            // Allow orb to be used
            DoToggleGameObjectFlags(GO_ORB_OF_DOMINATION, GO_FLAG_NO_INTERACT, false);

            if (Creature* pOrbTrigger = GetSingleCreatureFromStorage(NPC_BLACKWING_ORB_TRIGGER))
                pOrbTrigger->InterruptNonMeleeSpells(false);
            break;
        case NPC_RAZORGORE:
            // Only set the event as done if Razorgore dies in last phase
            if (GetData(TYPE_RAZORGORE) == SPECIAL)
            {
                SetData(TYPE_RAZORGORE, DONE);
                break;
            }

            // If the event is not already failed in Razorgore script, then force group wipe by making the boss trigger an AoE
            // this is basically a duplicate of what is in Razorgore script because when the boss is Mind Controlled the AI is overriden
            // So we have to handle it in the instance script instead to prevent the event to be stucked or exploited
            if (GetData(TYPE_RAZORGORE) != FAIL)
            {
                if (Creature* pRazorgore = GetSingleCreatureFromStorage(NPC_RAZORGORE))
                {
                    pRazorgore->CastSpell(pRazorgore, SPELL_FIREBALL, TRIGGERED_OLD_TRIGGERED);
                    SetData(TYPE_RAZORGORE, FAIL);
                    DoScriptText(SAY_RAZORGORE_DEATH, pRazorgore);
                    pRazorgore->ForcedDespawn();
                }
                if (Creature* pOrbTrigger = GetSingleCreatureFromStorage(NPC_BLACKWING_ORB_TRIGGER))
                {
                    if (Creature* pTemp = pOrbTrigger->SummonCreature(NPC_ORB_DOMINATION, pOrbTrigger->GetPositionX(), pOrbTrigger->GetPositionY(), pOrbTrigger->GetPositionZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 5 * IN_MILLISECONDS))
                        DoScriptText(EMOTE_ORB_SHUT_OFF, pTemp);
                    pOrbTrigger->CastSpell(pOrbTrigger, SPELL_EXPLODE_ORB, TRIGGERED_IGNORE_UNATTACKABLE_FLAG);
                }
            }
            break;
        case NPC_BLACKWING_LEGIONNAIRE:
        case NPC_BLACKWING_MAGE:
            m_uiBlackwingDefCount--;
            break;
        case NPC_DRAGONSPAWN:
            m_uiDragonspawnCount--;
            break;
        case NPC_BLUE_DRAKONID:
        case NPC_GREEN_DRAKONID:
        case NPC_BRONZE_DRAKONID:
        case NPC_RED_DRAKONID:
        case NPC_BLACK_DRAKONID:
        case NPC_CHROMATIC_DRAKONID:
            m_uiDeadDrakonidsCount++;
            // If the requiered amount of drakonids are killed, start phase 2
            if (m_uiDeadDrakonidsCount >= MAX_DRAKONID_SUMMONS && GetData(TYPE_NEFARIAN) == IN_PROGRESS)
                SetData(TYPE_NEFARIAN, SPECIAL);
            break;
    }
}

bool instance_blackwing_lair::CheckConditionCriteriaMeet(Player const* pPlayer, uint32 uiInstanceConditionId, WorldObject const* pConditionSource, uint32 conditionSourceType) const
{
    switch (uiInstanceConditionId)
    {
        case INSTANCE_CONDITION_ID_HARD_MODE:               // Event failed
            return (GetData(TYPE_QUEST_SCEPTER) == FAIL);
        case INSTANCE_CONDITION_ID_HARD_MODE_2:             // Event succeeded
            return (GetData(TYPE_QUEST_SCEPTER) == DONE);
    }

    script_error_log("instance_blackwing_lair::CheckConditionCriteriaMeet called with unsupported Id %u. Called with param plr %s, src %s, condition source type %u",
                     uiInstanceConditionId, pPlayer ? pPlayer->GetGuidStr().c_str() : "NULL", pConditionSource ? pConditionSource->GetGuidStr().c_str() : "NULL", conditionSourceType);
    return false;
}

void instance_blackwing_lair::Update(uint32 uiDiff)
{
    // Scepter of the Shifting Sand epic quest line
    if (m_uiScepterEpicTimer)
    {
        if (m_uiScepterEpicTimer <= uiDiff)
        {
            switch (m_uiScepterQuestStep)
            {
                case 0:     // On quest acceptance
                    DoOrSimulateScriptTextForThisInstance(YELL_REDSHARD_TAUNT_1, NPC_LORD_VICTOR_NEFARIUS);
                    m_uiScepterEpicTimer = 2 * HOUR * IN_MILLISECONDS;
                    break;
                case 1:     // 2 hours time mark
                    switch (urand(0, 1))
                    {
                        case 0:
                            DoOrSimulateScriptTextForThisInstance(YELL_REDSHARD_TAUNT_2, NPC_LORD_VICTOR_NEFARIUS);
                            DoOrSimulateScriptTextForThisInstance(EMOTE_REDSHARD_TAUNT_1, NPC_LORD_VICTOR_NEFARIUS);
                            break;
                        case 1:
                            DoOrSimulateScriptTextForThisInstance(YELL_REDSHARD_TAUNT_3, NPC_LORD_VICTOR_NEFARIUS);
                            break;
                    }
                    m_uiScepterEpicTimer = 2 * HOUR * IN_MILLISECONDS;
                    break;
                case 2:     // 1 hour left
                    switch (urand(0, 1))
                    {
                        case 0:
                            DoOrSimulateScriptTextForThisInstance(YELL_REDSHARD_TAUNT_4, NPC_LORD_VICTOR_NEFARIUS);
                            break;
                        case 1:
                            DoOrSimulateScriptTextForThisInstance(YELL_REDSHARD_TAUNT_5, NPC_LORD_VICTOR_NEFARIUS);
                            break;
                    }
                    m_uiScepterEpicTimer = 30 * MINUTE * IN_MILLISECONDS;
                    break;
                case 3:     // 30 min left
                    DoOrSimulateScriptTextForThisInstance(YELL_REDSHARD_TAUNT_6, NPC_LORD_VICTOR_NEFARIUS);
                    m_uiScepterEpicTimer = 30 * MINUTE * IN_MILLISECONDS;
                    break;
                case 4:     // Failure
                    SetData(TYPE_QUEST_SCEPTER, FAIL);
                    if (GetData(TYPE_NEFARIAN) == NOT_STARTED)
                    {
                        DoOrSimulateScriptTextForThisInstance(EMOTE_REDSHARD_TAUNT_2, NPC_LORD_VICTOR_NEFARIUS);
                        DoOrSimulateScriptTextForThisInstance(YELL_REDSHARD_TAUNT_7, NPC_LORD_VICTOR_NEFARIUS);
                    }
                default:    // Something weird happened: stop timer and fail the event
                    m_uiScepterEpicTimer = 0;
                    SetData(TYPE_QUEST_SCEPTER, FAIL);
                    break;
            }
            m_uiScepterQuestStep++;
        }
        else
            m_uiScepterEpicTimer -= uiDiff;
    }
    // Spawn Nefarian
    if (m_uiNefarianSpawnTimer)
    {
        if (m_uiNefarianSpawnTimer <= uiDiff)
        {
            if (Creature* pNefarius = GetSingleCreatureFromStorage(NPC_LORD_VICTOR_NEFARIUS))
            {
                if (Creature* pNefarian = pNefarius->SummonCreature(NPC_NEFARIAN, aNefarianLocs[2].m_fX, aNefarianLocs[2].m_fY, aNefarianLocs[2].m_fZ, 0, TEMPSPAWN_DEAD_DESPAWN, 0, true))
                {
                    pNefarian->SetWalk(false);

                    // see boss_onyxia (also note the removal of this in boss_nefarian)
                    pNefarian->SetByteValue(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND);
                    pNefarian->SetLevitate(true);
                    pNefarian->GetMotionMaster()->MoveWaypoint(0);
                }
            }
            m_uiNefarianSpawnTimer = 0;
        }
        else
            m_uiNefarianSpawnTimer -= uiDiff;
    }

    // Reset Razorgore in case of wipe
    if (m_uiResetTimer)
    {
        if (m_uiResetTimer <= uiDiff)
        {
            // Respawn Razorgore
            if (Creature* pRazorgore = GetSingleCreatureFromStorage(NPC_RAZORGORE))
            {
                if (!pRazorgore->isAlive())
                    pRazorgore->Respawn();
            }

            // Respawn the Dragon Eggs
            for (GuidList::const_iterator itr = m_lDragonEggsGuids.begin(); itr != m_lDragonEggsGuids.end(); ++itr)
            {
                if (GameObject* pEgg = instance->GetGameObject(*itr))
                {
                    if (!pEgg->IsSpawned())
                        pEgg->Respawn();
                }
            }

            m_uiResetTimer = 0;
        }
        else
            m_uiResetTimer -= uiDiff;
    }

    if (GetData(TYPE_RAZORGORE) != IN_PROGRESS)
        return;

    if (m_uiDefenseTimer < uiDiff)
    {
        // Randomize generators
        std::random_shuffle(m_vGeneratorGuids.begin(), m_vGeneratorGuids.end());

        // Spawn the defenders
        for (uint8 i = 0; i < MAX_EGGS_DEFENDERS; ++i)
        {
            Creature* pGenerator = instance->GetCreature(m_vGeneratorGuids[i]);
            if (!pGenerator)
                continue;

            // Defenders are spawned randomly, 4 at a time
            // There can be up to 12 Dragonspawns spawned and 40 Orcs (either mage or legionnaire)
            // If one of the cap is reached, only the remaining type of NPC is spawned until the second cap is also reached

            uint32 uiSpellId = 0;                                                   // Hold the spell summoning the NPC defender for that generator
            uint8 uiMaxRange = (m_uiDragonspawnCount < MAX_DRAGONSPAWN ? 3 : 1);    // If all dragonspawns are already spawned, don't roll for them below

            switch (urand(0, uiMaxRange))
            {
                case 0:
                    if (m_uiBlackwingDefCount < MAX_BLACKWING_DEFENDER)
                        uiSpellId = SPELL_SUMMON_LEGIONNAIRE;
                    break;
                case 1:
                    if (m_uiBlackwingDefCount < MAX_BLACKWING_DEFENDER)
                        uiSpellId = SPELL_SUMMON_MAGE;
                    break;
                case 2:
                case 3:
                    uiSpellId = SPELL_SUMMON_DRAGONSPAWN;
                    break;
            }

            if (uiSpellId != 0)
                pGenerator->CastSpell(pGenerator, uiSpellId, TRIGGERED_NONE);
        }

        m_uiDefenseTimer = 15000;
    }
    else
        m_uiDefenseTimer -= uiDiff;
}

void instance_blackwing_lair::InitiateBreath(uint32 uiEventId)
{
    uint32 leftBreath = 0;
    uint32 rightBreath = 0;
    switch (uiEventId)
    {
        // Left Chromaggus breath
        case 8446: leftBreath = 23187; break;  // Frost Burn
        case 8447: leftBreath = 23308; break;  // Incinerate
        case 8448: leftBreath = 23310; break;  // Time Lapse
        case 8449: leftBreath = 23313; break;  // Corrosive Acid
        case 8450: leftBreath = 23315; break;  // Ignite Flesh
        // Right Chromaggus breath
        case 8451: rightBreath = 23189; break;  // Frost Burn
        case 8452: rightBreath = 23309; break;  // Incinerate
        case 8453: rightBreath = 23312; break;  // Time Lapse
        case 8454: rightBreath = 23314; break;  // Corrosive Acid
        case 8455: rightBreath = 23316; break;  // Ignite Flesh
    }
    if (leftBreath)
    {
        debug_log("SD2 Instance Blackwing Lair: initiating Chromaggus' left breath");
        if (GetData(TYPE_CHROMA_LBREATH) == NOT_STARTED)
            SetData(TYPE_CHROMA_LBREATH, leftBreath);
        debug_log("SD2 Instance Blackwing Lair: Chromaggus' left breath set to spell %u", GetData(TYPE_CHROMA_LBREATH));
    }
    if (rightBreath)
    {
        debug_log("SD2 Instance Blackwing Lair: initiating Chromaggus' right breath");
        if (GetData(TYPE_CHROMA_RBREATH) == NOT_STARTED)
            SetData(TYPE_CHROMA_RBREATH, rightBreath);
        debug_log("SD2 Instance Blackwing Lair: Chromaggus' right breath set to spell %u", GetData(TYPE_CHROMA_RBREATH));
    }
}

void instance_blackwing_lair::InitiateDrakonid(uint32 uiEventId)
{
    uint32 leftTunnel = 0;
    uint32 rightTunnel = 0;
    switch (uiEventId)
    {
        // Left tunnel
        case 8520: leftTunnel = NPC_BLACK_SPAWNER; break;
        case 8521: leftTunnel = NPC_RED_SPAWNER; break;
        case 8522: leftTunnel = NPC_GREEN_SPAWNER; break;
        case 8523: leftTunnel = NPC_BRONZE_SPAWNER; break;
        case 8524: leftTunnel = NPC_BLUE_SPAWNER; break;
        // Right tunnel
        case 8525: rightTunnel = NPC_BLACK_SPAWNER; break;
        case 8526: rightTunnel = NPC_RED_SPAWNER; break;
        case 8527: rightTunnel = NPC_GREEN_SPAWNER; break;
        case 8528: rightTunnel = NPC_BRONZE_SPAWNER; break;
        case 8529: rightTunnel = NPC_BLUE_SPAWNER; break;
    }
    if (leftTunnel)
    {
        debug_log("SD2 Instance Blackwing Lair: initiating drakonid for left tunnel in Nefarian's lair");
        if (GetData(TYPE_NEFA_LTUNNEL) == NOT_STARTED)
            SetData(TYPE_NEFA_LTUNNEL, leftTunnel);
        debug_log("SD2 Instance Blackwing Lair: Nefarian's lair left tunnel set with drakonid spawner %u", GetData(TYPE_NEFA_LTUNNEL));
    }
    if (rightTunnel)
    {
        debug_log("SD2 Instance Blackwing Lair: initiating drakonid for right tunnel in Nefarian's lair");
        if (GetData(TYPE_NEFA_RTUNNEL) == NOT_STARTED)
            SetData(TYPE_NEFA_RTUNNEL, rightTunnel);
        debug_log("SD2 Instance Blackwing Lair: Nefarian's lair left tunnel set with drakonid spawner %u", GetData(TYPE_NEFA_RTUNNEL));
    }
}

void instance_blackwing_lair::CleanupNefarianStage(bool fullCleanup)
{
    for (GuidList::const_iterator itr = m_lDrakonidSpawnerGuids.begin(); itr != m_lDrakonidSpawnerGuids.end(); ++itr)
    {
        if (Creature* pTemp = instance->GetCreature(*itr))
            pTemp->ForcedDespawn();
    }
    m_lDrakonidSpawnerGuids.clear();

    if (Creature* pNefarius = GetSingleCreatureFromStorage(NPC_LORD_VICTOR_NEFARIUS))
    {
        // Despawn Nefarius if phase 2 is started
        if (!pNefarius->IsDespawned() && GetData(TYPE_NEFARIAN) == SPECIAL)
        {
            pNefarius->CastSpell(pNefarius, SPELL_SHADOWBLINK_OUTRO, TRIGGERED_OLD_TRIGGERED);
            pNefarius->ForcedDespawn(2000);
        }
    }
    // Stop the cleanup here if we are only moving from P1 to P2
    if (!fullCleanup)
        return;

    m_uiDeadDrakonidsCount = 0;

    for (GuidList::const_iterator itr = m_lDrakonidBonesGuids.begin(); itr != m_lDrakonidBonesGuids.end(); ++itr)
    {
        if (GameObject* pGo = instance->GetGameObject(*itr))
            pGo->SetLootState(GO_JUST_DEACTIVATED);
    }
    m_lDrakonidBonesGuids.clear();

    if (Creature* pNefarius = GetSingleCreatureFromStorage(NPC_LORD_VICTOR_NEFARIUS))
    {
        // Respawn Nefarius if wipe in Phase 2
        if (pNefarius->IsDespawned())
            pNefarius->Respawn();
    }
}

InstanceData* GetInstanceData_instance_blackwing_lair(Map* pMap)
{
    return new instance_blackwing_lair(pMap);
}

/*###############
## go_suppression
################*/

struct go_ai_suppression : public GameObjectAI
{
    go_ai_suppression(GameObject* go) : GameObjectAI(go), m_uiFumeTimer(urand(0, 5 * IN_MILLISECONDS)) {}

    uint32 m_uiFumeTimer;

    void OnLootStateChange() override
    {
        ScriptedInstance* pInstance = (ScriptedInstance*)m_go->GetMap()->GetInstanceData();
        if (!pInstance)
            return;

        // As long as Broodlord Lashlayer is alive, the GO will rearm on a random timer from 30 sec to 2 min
        // It will not rearm for the instance lifetime after Broodlord Lashlayer death
        if (m_go->GetLootState() == GO_ACTIVATED)
        {
            if (pInstance->GetData(TYPE_LASHLAYER) != DONE)
                m_go->SetRespawnTime(urand(30, 2 * MINUTE));
            else
                m_go->SetRespawnTime(7 * 24 * HOUR);
        }
    }

    // Visual effects for each GO is played on a 5 seconds timer. Sniff show that the GO should also be used (trap spell is cast)
    // but we need core support for GO casting for that
    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiFumeTimer)
        {
            if (m_uiFumeTimer <= uiDiff)
            {
                // TODO replace by go->Use(go) or go->Use(nullptr) once GO casting is added in core
                // The loot state check may be removed in that case because it should probably be handled in the Gameobject::Use() code
                if (m_go->GetLootState() == GO_READY)
                    m_go->SendGameObjectCustomAnim(m_go->GetObjectGuid());
                m_uiFumeTimer = 5 * IN_MILLISECONDS;
            }
            else
                m_uiFumeTimer -= uiDiff;
        }
    }
};


GameObjectAI* GetAI_go_suppression(GameObject* go)
{
    return new go_ai_suppression(go);
}

/*###################################################
## Chromaggus' breaths and Nefarian tunnels selection
####################################################*/

bool ProcessEventId_event_weekly_chromatic_selection(uint32 uiEventId, Object* pSource, Object* /*pTarget*/, bool /*bIsStart*/)
{
    if (pSource->GetTypeId() == TYPEID_UNIT)
    {
        if (instance_blackwing_lair* pInstance = (instance_blackwing_lair*)((Creature*)pSource)->GetInstanceData())
        {
            switch (uiEventId)
            {
                // Left Chromaggus breath
                case 8446:
                case 8447:
                case 8448:
                case 8449:
                case 8450:
                // Right Chromaggus breath
                case 8451:
                case 8452:
                case 8453:
                case 8454:
                case 8455:
                    pInstance->InitiateBreath(uiEventId);
                    break;
                // Left tunnel in Nefarian's lair
                case 8520:
                case 8521:
                case 8522:
                case 8523:
                case 8524:
                // Right tunnel in Nefarian's lair
                case 8525:
                case 8526:
                case 8527:
                case 8528:
                case 8529:
                    pInstance->InitiateDrakonid(uiEventId);
                    break;
            }
        }
    }
    return false;
}

void AddSC_instance_blackwing_lair()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "instance_blackwing_lair";
    pNewScript->GetInstanceData = &GetInstanceData_instance_blackwing_lair;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_suppression";
    pNewScript->GetGameObjectAI = &GetAI_go_suppression;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "event_weekly_chromatic_selection";
    pNewScript->pProcessEventId = &ProcessEventId_event_weekly_chromatic_selection;
    pNewScript->RegisterSelf();
}
