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
SDName: Instance_Scarlet_Monastery
SD%Complete: 50
SDComment:
SDCategory: Scarlet Monastery
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/include/sc_instance.h"
#include "scarlet_monastery.h"

instance_scarlet_monastery::instance_scarlet_monastery(Map* pMap) : ScriptedInstance(pMap)
{
    Initialize();
}

void instance_scarlet_monastery::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

void instance_scarlet_monastery::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_SORCERER:
        case NPC_MYRMIDON:
        case NPC_DEFENDER:
        case NPC_CHAPLAIN: 
        case NPC_WIZARD:
        case NPC_CENTURION:
        case NPC_CHAMPION: 
        case NPC_ABBOT:    
        case NPC_MONK:
        case NPC_FAIRBANKS:
            m_sAshbringerFriendlyGuids.insert(pCreature->GetObjectGuid());
            break;
        case NPC_WHITEMANE:
            m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
        case NPC_MOGRAINE:
            m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            m_sAshbringerFriendlyGuids.insert(pCreature->GetObjectGuid());
            break;
        case NPC_VORREL:
            m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
    }
}

void instance_scarlet_monastery::OnCreatureDeath(Creature* pCreature)
{
    if (pCreature->GetEntry() == NPC_INTERROGATOR_VISHAS)
    {
        // Any other actions to do with Vorrel? setStandState?
        if (Creature* pVorrel = GetSingleCreatureFromStorage(NPC_VORREL))
            DoScriptText(SAY_TRIGGER_VORREL, pVorrel);
    }
}

void instance_scarlet_monastery::OnObjectCreate(GameObject* pGo)
{
    if (pGo->GetEntry() == GO_WHITEMANE_DOOR)
        m_goEntryGuidStore[GO_WHITEMANE_DOOR] = pGo->GetObjectGuid();
    else if (pGo->GetEntry() == GO_CHAPEL_DOOR)
        m_goEntryGuidStore[GO_CHAPEL_DOOR] = pGo->GetObjectGuid();
}

void instance_scarlet_monastery::OnCreatureRespawn(Creature* creature)
{
    if (GetData(TYPE_ASHBRINGER_EVENT) == IN_PROGRESS)
        if (creature->IsAlive() && !creature->IsInCombat() && creature->GetFaction() != 35)
            if (m_sAshbringerFriendlyGuids.find(creature->GetObjectGuid()) != m_sAshbringerFriendlyGuids.end())
                creature->setFaction(35);
}

void instance_scarlet_monastery::OnObjectSpawn(GameObject* go)
{
    if (go->GetEntry() == GO_CHAPEL_DOOR)
        if (GetData(TYPE_ASHBRINGER_EVENT) == IN_PROGRESS)
            DoUseDoorOrButton(GO_CHAPEL_DOOR);
}

void instance_scarlet_monastery::SetData(uint32 uiType, uint32 uiData)
{
    if (uiType == TYPE_MOGRAINE_AND_WHITE_EVENT)
    {
        if (uiData == IN_PROGRESS)
            DoUseDoorOrButton(GO_WHITEMANE_DOOR);
        if (uiData == FAIL)
            {
                Creature* pWhitemane = GetSingleCreatureFromStorage(NPC_WHITEMANE);
                if(!pWhitemane)
                    return;
                Creature* pMograine = GetSingleCreatureFromStorage(NPC_MOGRAINE);
                if(!pMograine)
                    return;
                if(pWhitemane->IsAlive() && pMograine->IsAlive())
                {
                    pWhitemane->SetRespawnDelay(30, true);
                    pWhitemane->ForcedDespawn();
                    pMograine->SetRespawnDelay(30, true);
                    pMograine->ForcedDespawn();
                    DoUseDoorOrButton(GO_WHITEMANE_DOOR);
                    m_auiEncounter[0] = NOT_STARTED;
                    return;
                }
                if(!pMograine->IsAlive() && pWhitemane->IsAlive())
                {
                    pWhitemane->SetRespawnDelay(30, true);
                    pWhitemane->ForcedDespawn();
                    DoUseDoorOrButton(GO_WHITEMANE_DOOR);
                    m_auiEncounter[0] = uiData;
                    return;
                }
                if(!pWhitemane->IsAlive() && pMograine->IsAlive())
                {
                    pMograine->ForcedDespawn();
                    m_auiEncounter[0] = uiData;
                    return;
                }
            }

        m_auiEncounter[0] = uiData;
    }
    else if (uiType == TYPE_ASHBRINGER_EVENT)
    {
        if (uiData == IN_PROGRESS)
        {
            DoUseDoorOrButton(GO_CHAPEL_DOOR);

            Creature* whitemane = GetSingleCreatureFromStorage(NPC_WHITEMANE, true);
            if (whitemane && whitemane->IsAlive() && !whitemane->IsInCombat())
                whitemane->ForcedDespawn();

            for (auto scarletCathedralNpcGuid : m_sAshbringerFriendlyGuids)
                if (Creature* scarletNpc = instance->GetCreature(scarletCathedralNpcGuid))
                    if (scarletNpc->IsAlive() && !scarletNpc->IsInCombat())
                        scarletNpc->setFaction(35);
        }
        m_auiEncounter[1] = uiData;
    }
}

uint32 instance_scarlet_monastery::GetData(uint32 uiData) const
{
    if (uiData == TYPE_MOGRAINE_AND_WHITE_EVENT)
        return m_auiEncounter[0];
    if (uiData == TYPE_ASHBRINGER_EVENT)
        return m_auiEncounter[1];

    return 0;
}

InstanceData* GetInstanceData_instance_scarlet_monastery(Map* pMap)
{
    return new instance_scarlet_monastery(pMap);
}

bool instance_scarlet_monastery::DoHandleAreaTrigger(AreaTriggerEntry const* areaTrigger)
{
    if (areaTrigger->id == AREATRIGGER_CATHEDRAL_ENTRANCE)
    {
        if (GetData(TYPE_ASHBRINGER_EVENT) == NOT_STARTED)
        {
            SetData(TYPE_ASHBRINGER_EVENT, IN_PROGRESS);
            DoOrSimulateScriptTextForThisInstance(SAY_ASHBRINGER_ENTRANCE, NPC_MOGRAINE);
            return true;
        }
    }
    return false;
}

bool AreaTrigger_at_cathedral_entrance(Player* player, AreaTriggerEntry const* areaTrigger)
{
    if (player->IsGameMaster() || !player->IsAlive() || !player->HasItemCount(ITEM_CORRUPTED_ASHRBRINGER, 1))
        return false;

    if (auto* instance = (instance_scarlet_monastery*)player->GetInstanceData())
        return instance->DoHandleAreaTrigger(areaTrigger);

    return false;
}

struct ABEffect000 : public SpellScript
{
    bool OnCheckTarget(const Spell* spell, Unit* target, SpellEffectIndex /*eff*/) const override
    {
        if (WorldObject* caster = spell->GetCastingObject())
            if (!target->IsWithinLOSInMap(caster, true))
                return false;
        return true;
    }
};

void AddSC_instance_scarlet_monastery()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "instance_scarlet_monastery";
    pNewScript->GetInstanceData = &GetInstanceData_instance_scarlet_monastery;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_cathedral_entrance";
    pNewScript->pAreaTrigger = &AreaTrigger_at_cathedral_entrance;
    pNewScript->RegisterSelf();

    RegisterSpellScript<ABEffect000>("spell_ab_effect_000");
}
