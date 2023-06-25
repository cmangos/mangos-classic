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
SDName: Searing_Gorge
SD%Complete: 80
SDComment: Quest support: 3367.
SDCategory: Searing Gorge
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"/* ContentData
npc_dorius_stonetender
EndContentData */


#include "AI/ScriptDevAI/base/escort_ai.h"

/*######
## npc_dorius_stonetender
######*/

enum
{
    SAY_DORIUS_AGGRO_1              = 4351,
    SAY_DORIUS_AGGRO_2              = 4352,
    SAY_DORIUS_AGGRO_3              = 4353,

    SAY_DORIUS_TO_PLAYER            = 4354,
    SAY_DORIUS_FINISH               = 4359,

    NPC_DARK_IRON_STEELSHIFTER      = 8337,

    SPELL_MARKSMAN_HIT              = 12198,
    
    RELAY_SCRIPT_FINISHED           = 8284,

    PATH_ID                         = 8284,        // Sniffed Waypoints for Escort Event
    QUEST_ID_SUNTARA_STONES         = 3367,
};

struct npc_dorius_stonetenderAI : public npc_escortAI
{
    npc_dorius_stonetenderAI(Creature* pCreature) : npc_escortAI(pCreature) 
    { 
        Reset(); 
    }

    void Reset() override
    {
        m_creature->SetStandState(UNIT_STAND_STATE_DEAD, true);
    }

    void Aggro(Unit* who) override
    {
        switch (urand(0, 2))
        {
            case 0: DoBroadcastText(SAY_DORIUS_AGGRO_1, m_creature, who); break;
            case 1: DoBroadcastText(SAY_DORIUS_AGGRO_2, m_creature, who); break;
            case 2: DoBroadcastText(SAY_DORIUS_AGGRO_3, m_creature, who); break;
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*pSender*/, Unit* pInvoker, uint32 uiMiscValue) override
    {
        if (eventType == AI_EVENT_START_ESCORT && pInvoker->GetTypeId() == TYPEID_PLAYER)
        {
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);
            m_creature->SetFactionTemporary(FACTION_ESCORT_A_NEUTRAL_ACTIVE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_NPC);
            Start(false, (Player*)pInvoker, GetQuestTemplateStore(uiMiscValue), true, false, PATH_ID);
        }
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell) override
    {        
        if (pSpell->Id == SPELL_MARKSMAN_HIT)
        { 
            if (Player* pPlayer = GetPlayerForEscort())
                pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_ID_SUNTARA_STONES, m_creature);
            // all rp things handled via relayscript
            m_creature->GetMap()->ScriptsStart(SCRIPT_TYPE_RELAY, RELAY_SCRIPT_FINISHED, m_creature, pCaster);
            End();
        }            
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 /*moveType*/, uint32 uiPointId) override
    {
        if (pSummoned->GetEntry() == NPC_DARK_IRON_STEELSHIFTER && uiPointId == 2)
            pSummoned->AI()->AttackStart(m_creature);
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {         
            case 37:
                // Have to do it via core because we need Player as target
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    DoBroadcastText(SAY_DORIUS_TO_PLAYER, m_creature, pPlayer);
                    m_creature->HandleEmote(EMOTE_ONESHOT_POINT);
                }
                break;
            case 53:
                if (Player* pPlayer = GetPlayerForEscort())                
                    m_creature->SetFacingToObject(pPlayer);
                break;
        }
    }

    void UpdateEscortAI(const uint32 /*uiDiff*/) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_dorius_stonetender(Creature* pCreature)
{
    return new npc_dorius_stonetenderAI(pCreature);
}

bool QuestAccept_npc_dorius_stonetender(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ID_SUNTARA_STONES)
    {
        pCreature->AI()->SendAIEvent(AI_EVENT_START_ESCORT, pPlayer, pCreature, pQuest->GetQuestId());
        return true;
    }

    return false;
}

void AddSC_searing_gorge()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_dorius_stonetender";
    pNewScript->GetAI = &GetAI_npc_dorius_stonetender;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_dorius_stonetender;
    pNewScript->RegisterSelf();
}
