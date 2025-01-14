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
SDName: Instance_Ruins_of_Ahnqiraj
SD%Complete: 80
SDComment: It's not clear if the Rajaxx event should reset if Andorov dies, or party wipes.
SDCategory: Ruins of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "ruins_of_ahnqiraj.h"

instance_ruins_of_ahnqiraj::instance_ruins_of_ahnqiraj(Map* map) : ScriptedInstance(map),
    m_armyDelayTimer(0),
    m_currentArmyWave(0)
{
    Initialize();
}

void instance_ruins_of_ahnqiraj::Initialize()
{
    memset(&m_encounter, 0, sizeof(m_encounter));
}

bool instance_ruins_of_ahnqiraj::IsEncounterInProgress() const
{
    for (uint32 i : m_encounter)
    {
        if (i == IN_PROGRESS)
            return true;
    }
    return false;
}

void instance_ruins_of_ahnqiraj::OnPlayerEnter(Player* /*player*/)
{
    // Spawn andorov if necessary
    if (GetData(TYPE_KURINNAXX) == DONE)
        DoSpawnAndorovIfCan();
}

void instance_ruins_of_ahnqiraj::OnCreatureCreate(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_OSSIRIAN_TRIGGER:
        {
            GuidVector& vector = m_npcEntryGuidCollection[creature->GetEntry()];
            vector.push_back(creature->GetObjectGuid());
            std::sort(vector.begin(), vector.end(), [&](ObjectGuid const& a, ObjectGuid const& b) -> bool
            {
                return a.GetCounter() < b.GetCounter();
            });
            break;
        }
        case NPC_BURU:
        case NPC_OSSIRIAN:
        case NPC_RAJAXX:
        case NPC_GENERAL_ANDOROV:
        case NPC_COLONEL_ZERRAN:
        case NPC_MAJOR_PAKKON:
        case NPC_MAJOR_YEGGETH:
        case NPC_CAPTAIN_XURREM:
        case NPC_CAPTAIN_DRENN:
        case NPC_CAPTAIN_TUUBID:
        case NPC_CAPTAIN_QEEZ:
            m_npcEntryGuidStore[creature->GetEntry()] = creature->GetObjectGuid();
            break;
        case NPC_KALDOREI_ELITE:
            m_kaldoreiGuidList.push_back(creature->GetObjectGuid());
            break;
    }
}

void instance_ruins_of_ahnqiraj::OnObjectCreate(GameObject* go)
{
    if (go->GetEntry() == GO_OSSIRIAN_CRYSTAL)
        m_goEntryGuidCollection[go->GetEntry()].push_back(go->GetObjectGuid());
}

void instance_ruins_of_ahnqiraj::OnCreatureEnterCombat(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_KURINNAXX: SetData(TYPE_KURINNAXX, IN_PROGRESS); break;
        case NPC_MOAM:      SetData(TYPE_MOAM, IN_PROGRESS);      break;
        case NPC_BURU:      SetData(TYPE_BURU, IN_PROGRESS);      break;
        case NPC_AYAMISS:   SetData(TYPE_AYAMISS, IN_PROGRESS);   break;
        case NPC_OSSIRIAN:  SetData(TYPE_OSSIRIAN, IN_PROGRESS);  break;
    }
}

void instance_ruins_of_ahnqiraj::OnCreatureEvade(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_KURINNAXX: SetData(TYPE_KURINNAXX, FAIL); break;
        case NPC_MOAM:      SetData(TYPE_MOAM, FAIL);      break;
        case NPC_BURU:      SetData(TYPE_BURU, FAIL);      break;
        case NPC_AYAMISS:   SetData(TYPE_AYAMISS, FAIL);   break;
        case NPC_OSSIRIAN:  SetData(TYPE_OSSIRIAN, FAIL);  break;
        case NPC_RAJAXX:
            // Rajaxx yells on evade
            // DoScriptText(SAY_DEAGGRO, creature); - unconfirmed
        // no break;
        case NPC_COLONEL_ZERRAN:
        case NPC_MAJOR_PAKKON:
        case NPC_MAJOR_YEGGETH:
        case NPC_CAPTAIN_XURREM:
        case NPC_CAPTAIN_DRENN:
        case NPC_CAPTAIN_TUUBID:
        case NPC_CAPTAIN_QEEZ:
            SetData(TYPE_RAJAXX, FAIL);
            break;
    }
}

void instance_ruins_of_ahnqiraj::OnCreatureDeath(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_KURINNAXX: SetData(TYPE_KURINNAXX, DONE); break;
        case NPC_RAJAXX:    SetData(TYPE_RAJAXX, DONE);    break;
        case NPC_MOAM:      SetData(TYPE_MOAM, DONE);      break;
        case NPC_BURU:      SetData(TYPE_BURU, DONE);      break;
        case NPC_AYAMISS:   SetData(TYPE_AYAMISS, DONE);   break;
        case NPC_OSSIRIAN:  SetData(TYPE_OSSIRIAN, DONE);  break;
        case NPC_COLONEL_ZERRAN:
        case NPC_MAJOR_PAKKON:
        case NPC_MAJOR_YEGGETH:
        case NPC_CAPTAIN_XURREM:
        case NPC_CAPTAIN_DRENN:
        case NPC_CAPTAIN_TUUBID:
        case NPC_CAPTAIN_QEEZ:
        case NPC_QIRAJI_WARRIOR:
        case NPC_SWARMGUARD_NEEDLER:
        {
            // If event isn't started by Andorov, return
            if (GetData(TYPE_RAJAXX) != IN_PROGRESS)
                return;


            break;
        }
    }
}

void instance_ruins_of_ahnqiraj::OnCreatureRespawn(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_OSSIRIAN_TRIGGER:
        {
            if (Creature* ossirian = GetSingleCreatureFromStorage(NPC_OSSIRIAN))
                ossirian->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, creature, ossirian);
            break;
        }
        case NPC_QIRAJI_WARRIOR:
        case NPC_SWARMGUARD_NEEDLER:
        {
            creature->SetCorpseDelay(5);
            break;
        }
    }
}

void instance_ruins_of_ahnqiraj::OnCreatureGroupDespawn(CreatureGroup* group, Creature* creature)
{
    // last wave makes rajaxx attack immediately
    if (group->GetGroupEntry().StringId == instance->GetMapDataContainer().GetStringId("AQ20_ZERRAN"))
    {
        if (GetData(TYPE_RAJAXX) != IN_PROGRESS)
            return;

        m_armyDelayTimer = 1;
    }
}

void instance_ruins_of_ahnqiraj::SetData(uint32 type, uint32 data)
{
    switch (type)
    {
        case TYPE_KURINNAXX:
            if (data == DONE)
            {
                DoSpawnAndorovIfCan();
                // Yell after kurinnaxx
                if (Creature* ossirian = GetSingleCreatureFromStorage(NPC_OSSIRIAN))
                    DoScriptText(SAY_OSSIRIAN_INTRO, ossirian); // targets kurinnaxx in sniff
            }
            m_encounter[type] = data;
            break;
        case TYPE_RAJAXX:
            m_encounter[type] = data;
            if (data == IN_PROGRESS)
                DoSortArmyWaves();
            if (data == DONE)
            {
                if (Creature* andorov = GetSingleCreatureFromStorage(NPC_GENERAL_ANDOROV))
                    if (andorov->IsAlive())
                        andorov->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, andorov, andorov);
            }
            break;
        case TYPE_MOAM:
        case TYPE_BURU:
        case TYPE_AYAMISS:
        case TYPE_OSSIRIAN:
            if (type == FAIL)
            {
                GuidVector crystals;
                GetGameObjectGuidVectorFromStorage(GO_OSSIRIAN_CRYSTAL, crystals);
                for (uint32 i = 1; i < crystals.size(); ++i) // skip first
                {
                    if (GameObject* go = instance->GetGameObject(crystals[i]))
                    {
                        go->SetLootState(GO_JUST_DEACTIVATED);
                        go->SetForcedDespawn();
                    }
                }
                m_goEntryGuidCollection[GO_OSSIRIAN_CRYSTAL].resize(1); // keeps first
            }
            m_encounter[type] = data;
            break;
    }

    if (data == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_encounter[0] << " " << m_encounter[1] << " " << m_encounter[2]
                   << " " << m_encounter[3] << " " << m_encounter[4] << " " << m_encounter[5];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

uint32 instance_ruins_of_ahnqiraj::GetData(uint32 type) const
{
    if (type < MAX_ENCOUNTER)
        return m_encounter[type];

    return 0;
}

void instance_ruins_of_ahnqiraj::DoSpawnAndorovIfCan()
{
    if (GetSingleCreatureFromStorage(NPC_GENERAL_ANDOROV))
        return;

    Player* player = GetPlayerInMap();
    if (!player)
        return;

    for (const auto& aAndorovSpawnLoc : aAndorovSpawnLocs)
        player->SummonCreature(aAndorovSpawnLoc.m_uiEntry, aAndorovSpawnLoc.m_fX, aAndorovSpawnLoc.m_fY, aAndorovSpawnLoc.m_fZ, aAndorovSpawnLoc.m_fO, TEMPSPAWN_DEAD_DESPAWN, 0);
}

void instance_ruins_of_ahnqiraj::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);

    loadStream >> m_encounter[0] >> m_encounter[1] >> m_encounter[2]
               >> m_encounter[3] >> m_encounter[4] >> m_encounter[5];

    for (uint32& i : m_encounter)
    {
        if (i == IN_PROGRESS)
            i = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

void instance_ruins_of_ahnqiraj::Update(uint32 diff)
{
    if (GetData(TYPE_RAJAXX) == IN_PROGRESS)
    {
        if (m_armyDelayTimer)
        {
            if (m_armyDelayTimer <= diff)
            {
                DoSendNextArmyWave();
                m_armyDelayTimer = 3 * MINUTE * IN_MILLISECONDS;
            }
            else
                m_armyDelayTimer -= diff;
        }
    }
}

void instance_ruins_of_ahnqiraj::DoSortArmyWaves()
{
    // send the first wave
    m_currentArmyWave = 0;
    DoSendNextArmyWave();
}

void instance_ruins_of_ahnqiraj::DoSendNextArmyWave()
{
    // The next army wave is sent into battle after 2 min or after the previous wave is finished
    if (GetData(TYPE_RAJAXX) != IN_PROGRESS)
        return;

    // The last wave is General Rajaxx itself
    if (m_currentArmyWave == MAX_ARMY_WAVES)
    {
        if (Creature* rajaxx = GetSingleCreatureFromStorage(NPC_RAJAXX))
        {
            DoScriptText(SAY_INTRO, rajaxx);
            rajaxx->SetInCombatWithZone();
            rajaxx->AI()->AttackClosestEnemy();
        }
        m_armyDelayTimer = 0;
    }
    else
    {
        if (m_currentArmyWave == MAX_ARMY_WAVES)
        {
            script_error_log("Instance Ruins of Ahn'Qiraj: ERROR Something unexpected happened. Please report to SD2 team.");
            return;
        }

        auto waveInfo = aArmySortingParameters[m_currentArmyWave];
        auto waveMobs = instance->GetCreatures(waveInfo.m_stringId);
        if (waveMobs)
        {
            for (Creature* member : *waveMobs)
            {
                member->SetWalk(false);
                member->SetInCombatWithZone();
                member->AI()->AttackClosestEnemy();
            }
        }

        // Yell on each wave (except the first 2)
        if (aArmySortingParameters[m_currentArmyWave].m_uiYellEntry)
        {
            if (Creature* pRajaxx = GetSingleCreatureFromStorage(NPC_RAJAXX))
                DoScriptText(aArmySortingParameters[m_currentArmyWave].m_uiYellEntry, pRajaxx);
        }
        // on wowwiki it states that there were 3 min between the waves, but this was reduced in later patches
        m_armyDelayTimer = 3 * MINUTE * IN_MILLISECONDS;
    }

    ++m_currentArmyWave;
}

void AddSC_instance_ruins_of_ahnqiraj()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "instance_ruins_of_ahnqiraj";
    pNewScript->GetInstanceData = &GetNewInstanceScript<instance_ruins_of_ahnqiraj>;
    pNewScript->RegisterSelf();
}
