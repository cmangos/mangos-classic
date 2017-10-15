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
SD%Complete: 95
SDComment:
SDCategory: Blackwing Lair
EndScriptData

*/

#include "AI/ScriptDevAI/PreCompiledHeader.h"
#include "blackwing_lair.h"

instance_blackwing_lair::instance_blackwing_lair(Map* pMap) : ScriptedInstance(pMap),
    m_uiResetTimer(0),
    m_uiDefenseTimer(0),
    m_uiScepterEpicTimer(0),
    m_uiScepterQuestStep(0),
    m_uiDragonspawnCount(0),
    m_uiBlackwingDefCount(0),
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
    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
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
                // handle missing spell 23362
                Creature* pNefarius = GetSingleCreatureFromStorage(NPC_LORD_VICTOR_NEFARIUS);
                if (!pNefarius)
                    break;

                for (GuidList::const_iterator itr = m_lDrakonidBonesGuids.begin(); itr != m_lDrakonidBonesGuids.end(); ++itr)
                {
                    // The Go script will handle the missing spell 23361
                    if (GameObject* pGo = instance->GetGameObject(*itr))
                        pGo->Use(pNefarius);
                }
                // Don't store special data
                break;
            }
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(GO_DOOR_NEFARIAN);
            // Cleanup the drakonid bones
            if (uiData == FAIL)
            {
                for (GuidList::const_iterator itr = m_lDrakonidBonesGuids.begin(); itr != m_lDrakonidBonesGuids.end(); ++itr)
                {
                    if (GameObject* pGo = instance->GetGameObject(*itr))
                        pGo->SetLootState(GO_JUST_DEACTIVATED);
                }

                m_lDrakonidBonesGuids.clear();
            }
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
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                   << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " "
                   << m_auiEncounter[6] << " " << m_auiEncounter[7] << " " << m_auiEncounter[8];

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
               >> m_auiEncounter[8];

    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
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

    script_error_log("instance_dire_maul::CheckConditionCriteriaMeet called with unsupported Id %u. Called with param plr %s, src %s, condition source type %u",
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
                    if (!pEgg->isSpawned())
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

InstanceData* GetInstanceData_instance_blackwing_lair(Map* pMap)
{
    return new instance_blackwing_lair(pMap);
}

/*###############
## go_suppression
################*/

struct go_ai_suppression : public GameObjectAI
{
    go_ai_suppression (GameObject* go) : GameObjectAI(go), m_uiFumeTimer(urand(0, 5 * IN_MILLISECONDS)) {}

    uint32 m_uiFumeTimer;

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
                if (m_go->getLootState() == GO_READY)
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
    return new go_ai_suppression (go);
}

void AddSC_instance_blackwing_lair()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_blackwing_lair";
    pNewScript->GetInstanceData = &GetInstanceData_instance_blackwing_lair;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_suppression";
    pNewScript->GetGameObjectAI = &GetAI_go_suppression;
    pNewScript->RegisterSelf();
}
