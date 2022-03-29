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
SDName: Swamp_of_Sorrows
SD%Complete: 100
SDComment: Quest support: 1393
SDCategory: Swamp of Sorrows
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"/* ContentData
npc_galen_goodward
EndContentData */


#include "AI/ScriptDevAI/base/escort_ai.h"

/*######
## npc_galen_goodward
######*/

enum Galen
{
    QUEST_GALENS_ESCAPE     = 1393,

    GO_GALENS_CAGE          = 37118,

    SAY_PERIODIC            = -1000582,
    SAY_QUEST_ACCEPTED      = -1000583,
    SAY_ATTACKED_1          = -1000584,
    SAY_ATTACKED_2          = -1000585,
    SAY_QUEST_COMPLETE      = -1000586,
    EMOTE_WHISPER           = -1000587,
    EMOTE_DISAPPEAR         = -1000588
};

struct npc_galen_goodwardAI : public npc_escortAI
{
    npc_galen_goodwardAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

    ObjectGuid m_galensCageGuid;
    uint32 m_periodicSay;

    void Reset() override
    {
        m_periodicSay = 6000;
    }

    void Aggro(Unit* who) override
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
            DoScriptText(urand(0, 1) ? SAY_ATTACKED_1 : SAY_ATTACKED_2, m_creature, who);
    }

    void WaypointStart(uint32 pointId) override
    {
        if (pointId == 1)
        {
            GameObject* cage;
            if (m_galensCageGuid)
                cage = m_creature->GetMap()->GetGameObject(m_galensCageGuid);
            else
                cage = GetClosestGameObjectWithEntry(m_creature, GO_GALENS_CAGE, INTERACTION_DISTANCE);

            if (cage)
            {
                cage->UseDoorOrButton();
                m_galensCageGuid = cage->GetObjectGuid();
            }
        }
    }

    void WaypointReached(uint32 pointId) override
    {
        switch (pointId)
        {
            case 1:
                if (GameObject* cage = m_creature->GetMap()->GetGameObject(m_galensCageGuid))
                    cage->ResetDoorOrButton();
                break;
            case 20:
                SetRun(true);
                break;
            case 21:

                if (Player* player = GetPlayerForEscort())
                {
                    DoScriptText(SAY_QUEST_COMPLETE, m_creature, player);
                    DoScriptText(EMOTE_WHISPER, m_creature, player);
                    player->RewardPlayerAndGroupAtEventExplored(QUEST_GALENS_ESCAPE, m_creature);
                }
                break;
            case 22:
                DoScriptText(EMOTE_DISAPPEAR, m_creature);
                break;
            case 23:
                SetEscortPaused(true);
                m_creature->ForcedDespawn(1000);
                break;
            default:
                break;
        }
    }

    void UpdateEscortAI(const uint32 diff) override
    {

        if (m_periodicSay < diff)
        {
            if (HasEscortState(STATE_ESCORT_NONE))
                DoScriptText(SAY_PERIODIC, m_creature);
            m_periodicSay = 6000;
        }
        else
            m_periodicSay -= diff;

        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_galen_goodward(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_GALENS_ESCAPE)
    {

        if (auto* escortAI = dynamic_cast<npc_galen_goodwardAI*>(creature->AI()))
        {
            escortAI->Start(false, player, quest);
            creature->SetFactionTemporary(FACTION_ESCORT_N_NEUTRAL_ACTIVE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_NPC);
            DoScriptText(SAY_QUEST_ACCEPTED, creature);
        }
    }
    return true;
}

UnitAI* GetAI_npc_galen_goodward(Creature* creature)
{
    return new npc_galen_goodwardAI(creature);
}

void AddSC_swamp_of_sorrows()
{
    Script* newScript = new Script;
    newScript->Name = "npc_galen_goodward";
    newScript->GetAI = &GetAI_npc_galen_goodward;
    newScript->pQuestAcceptNPC = &QuestAccept_npc_galen_goodward;
    newScript->RegisterSelf();
}
