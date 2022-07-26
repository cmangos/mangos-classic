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
SDName: instance_maraudon
SD%Complete: 100
SDComment:
SDCategory: Maraudon
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "maraudon.h"

instance_maraudon::instance_maraudon(Map* map) : ScriptedInstance(map),
    m_spewLarvaTimer(1 * MINUTE * IN_MILLISECONDS)
{
    Initialize();
}

void instance_maraudon::Initialize()
{
    memset(&m_encounter, 0, sizeof(m_encounter));
}

void instance_maraudon::OnObjectCreate(GameObject* go)
{
    switch (go->GetEntry())
    {
        case GO_LARVA_SPEWER:
            // If Noxxion encounter is done, stop spewing larvas
            if (GetData(TYPE_NOXXION) == DONE)
                go->SetLootState(GO_ACTIVATED);
            break;
        case GO_CORRUPTION_SPEWER:
            break;
    }
    m_goEntryGuidStore[go->GetEntry()] = go->GetObjectGuid();
}

void instance_maraudon::SetData(uint32 type, uint32 data)
{
    if (type == TYPE_NOXXION)
    {
        // Do nothing if we are receiving the same data
        if (m_encounter[TYPE_NOXXION] == data)
            return;

        m_encounter[type] = data;
        switch (data)
        {
            // If Noxxion encounter is done or spewer attacked, stop spewing larvas
            case DONE:
                // Destroy the corruption spewer near Noxxion
                if (GameObject* go = GetSingleGameObjectFromStorage(GO_CORRUPTION_SPEWER))
                    go->SetLootState(GO_ACTIVATED);
                // no break, the special part is also executed
            case SPECIAL:
                m_spewLarvaTimer = 0;
                // Destroy the larva spewer if not already done
                if (GameObject* go = GetSingleGameObjectFromStorage(GO_LARVA_SPEWER))
                {
                    if (go->GetLootState() != GO_ACTIVATED)
                        go->SetLootState(GO_ACTIVATED);
                }
        }
    }

    if (data == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;

        saveStream << m_encounter[0];

        m_strInstData = saveStream.str();
        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

uint32 instance_maraudon::GetData(uint32 type) const
{
    if (type < MAX_ENCOUNTER)
        return m_encounter[type];

    return 0;
}

void instance_maraudon::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_encounter[0];

    for (unsigned int& i : m_encounter)
    {
        if (i == IN_PROGRESS)
            i = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

void instance_maraudon::OnCreatureDeath(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_NOXXION:
            SetData(TYPE_NOXXION, DONE);
            break;
    }
}

void instance_maraudon::OnCreatureEvade(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_NOXXION:
            SetData(TYPE_NOXXION, FAIL);
            break;
    }
}

void instance_maraudon::Update(uint32 uiDiff)
{
    // Nicely spew a larva that will go toward Noxxion (pathing is handled by DB)
    if (m_spewLarvaTimer)
    {
        if (m_spewLarvaTimer <= uiDiff)
        {
            if (GameObject* pGo = GetSingleGameObjectFromStorage(GO_LARVA_SPEWER))
                pGo->SummonCreature(NPC_SPEWED_LARVA, spawnLocation.m_fX, spawnLocation.m_fY, spawnLocation.m_fZ, spawnLocation.m_fO, TEMPSPAWN_TIMED_DESPAWN, 10 * MINUTE * IN_MILLISECONDS); // Despawn is handled by DBScript we just want to avoid an infinite number of larvas near Noxxion
            m_spewLarvaTimer = 1 * MINUTE * IN_MILLISECONDS;
        }
        else
            m_spewLarvaTimer -= uiDiff;
    }
}

InstanceData* GetInstanceData_instance_maraudon(Map* map)
{
    return new instance_maraudon(map);
}

/*################
## go_larva_spewer
#################*/

struct go_ai_larva_spewer : public GameObjectAI
{
    go_ai_larva_spewer(GameObject* go) : GameObjectAI(go) {}

    void OnLootStateChange(Unit* /*user*/)
    {
        if (m_go->GetLootState() == GO_ACTIVATED)
        {
            ScriptedInstance* pInstance = (ScriptedInstance*)m_go->GetMap()->GetInstanceData();
            if (pInstance)
                pInstance->SetData(TYPE_NOXXION, SPECIAL);
        }
    }
};

GameObjectAI* GetAI_go_larva_spewer(GameObject* go)
{
    return new go_ai_larva_spewer(go);
}

void AddSC_instance_maraudon()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "instance_maraudon";
    pNewScript->GetInstanceData = &GetInstanceData_instance_maraudon;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_larva_spewer";
    pNewScript->GetGameObjectAI = &GetAI_go_larva_spewer;
    pNewScript->RegisterSelf();
}
