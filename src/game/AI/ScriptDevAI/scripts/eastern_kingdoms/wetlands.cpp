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
SDName: Wetlands
SD%Complete: 100
SDComment: Quest support: 1249
SDCategory: Wetlands
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"/* ContentData
npc_mikhail
npc_tapoke_slim_jahn
EndContentData */


#include "AI/ScriptDevAI/base/escort_ai.h"

/*######
## npc_tapoke_slim_jahn
######*/

enum
{
    SAY_SLIM_AGGRO              = -1000977,
    SAY_SLIM_DEFEAT             = -1000978,
    SAY_FRIEND_DEFEAT           = -1000979,
    SAY_SLIM_NOTES              = -1000980,

    QUEST_MISSING_DIPLOMAT11    = 1249,
    FACTION_ENEMY               = 168,                      // ToDo: faction needs to be confirmed!

    SPELL_STEALTH               = 1785,
    SPELL_CALL_FRIENDS          = 16457,                    // summon npc 4971

    NPC_SLIMS_FRIEND            = 4971,
    NPC_TAPOKE_SLIM_JAHN        = 4962
};

static const DialogueEntry aDiplomatDialogue[] =
{
    {SAY_SLIM_DEFEAT,           NPC_TAPOKE_SLIM_JAHN,   4000},
    {SAY_SLIM_NOTES,            NPC_TAPOKE_SLIM_JAHN,   7000},
    {QUEST_MISSING_DIPLOMAT11,  0,                      0},
    {0, 0, 0},
};

struct npc_tapoke_slim_jahnAI : public npc_escortAI, private DialogueHelper
{
    npc_tapoke_slim_jahnAI(Creature* pCreature) : npc_escortAI(pCreature),
        DialogueHelper(aDiplomatDialogue)
    {
        Reset();
    }

    bool m_bFriendSummoned;
    bool m_bEventComplete;

    void Reset() override
    {
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
        SetReactState(REACT_PASSIVE);

        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            m_bFriendSummoned = false;
            m_bEventComplete = false;
        }
    }

    void JustReachedHome() override
    {
        // after the npc is defeated, start the dialog right after it reaches the evade point
        if (m_bEventComplete)
        {
            if (Player* pPlayer = GetPlayerForEscort())
                m_creature->SetFacingToObject(pPlayer);

            StartNextDialogueText(SAY_SLIM_DEFEAT);
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 3:
                SetRun();
                m_creature->RemoveAurasDueToSpell(SPELL_STEALTH);
                m_creature->SetFactionTemporary(FACTION_ENEMY, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_RESTORE_COMBAT_STOP);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
                SetReactState(REACT_AGGRESSIVE);
                break;
            case 7:
                // fail the quest if he escapes
                if (Player* pPlayer = GetPlayerForEscort())
                    JustDied(pPlayer);
                break;
        }
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING) && !m_bFriendSummoned)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_CALL_FRIENDS) == CAST_OK)
            {
                DoScriptText(SAY_SLIM_AGGRO, m_creature);
                m_bFriendSummoned = true;
            }
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        // Note: may not work on guardian pets
        if (Player* pPlayer = GetPlayerForEscort())
            pSummoned->AI()->AttackStart(pPlayer);
    }

    void DamageTaken(Unit* /*dealer*/, uint32& damage, DamageEffectType /*damagetype*/, SpellEntry const* /*spellInfo*/) override
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
            return;

        if (m_creature->GetHealthPercent() < 20.0f || damage > m_creature->GetHealth())
        {
            if (Pet* pFriend = m_creature->FindGuardianWithEntry(NPC_SLIMS_FRIEND))
            {
                DoScriptText(SAY_FRIEND_DEFEAT, pFriend);
                pFriend->ForcedDespawn(1000);
            }

            // set escort on pause and evade
            damage = std::min(damage, m_creature->GetHealth() - 1);
            m_bEventComplete = true;

            SetEscortPaused(true);
            EnterEvadeMode();
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*pSender*/, Unit* pInvoker, uint32 uiMiscValue) override
    {
        // start escort
        if (eventType == AI_EVENT_START_ESCORT && pInvoker->GetTypeId() == TYPEID_PLAYER)
            Start(false, (Player*)pInvoker, GetQuestTemplateStore(uiMiscValue), true);
    }

    void JustDidDialogueStep(int32 iEntry) override
    {
        if (iEntry == QUEST_MISSING_DIPLOMAT11)
        {
            // complete quest
            if (Player* pPlayer = GetPlayerForEscort())
                pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_MISSING_DIPLOMAT11, m_creature);

            // despawn and respawn at inn
            m_creature->ForcedDespawn(1000);
            m_creature->SetRespawnDelay(2);
        }
    }

    Creature* GetSpeakerByEntry(uint32 uiEntry) override
    {
        if (uiEntry == NPC_TAPOKE_SLIM_JAHN)
            return m_creature;

        return nullptr;
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        DialogueUpdate(uiDiff);

        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_npc_tapoke_slim_jahn(Creature* pCreature)
{
    return new npc_tapoke_slim_jahnAI(pCreature);
}

/*######
## npc_mikhail
######*/

bool QuestAccept_npc_mikhail(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_MISSING_DIPLOMAT11)
    {
        Creature* pSlim = GetClosestCreatureWithEntry(pCreature, NPC_TAPOKE_SLIM_JAHN, 25.0f);
        if (!pSlim)
            return false;

        if (!pSlim->HasAura(SPELL_STEALTH))
            pSlim->CastSpell(pSlim, SPELL_STEALTH, TRIGGERED_OLD_TRIGGERED);

        pCreature->AI()->SendAIEvent(AI_EVENT_START_ESCORT, pPlayer, pSlim, pQuest->GetQuestId());
        return true;
    }

    return false;
}

enum
{
    NPC_PROSPECTOR_WHELGAR  = 1077,
    SAY_ITEM_LOOTED         = 1235
};

// Flagongut's Fossil ID: 9630

struct go_flagongut_fossil : public GameObjectAI
{
    go_flagongut_fossil(GameObject* go) : GameObjectAI(go) {}

    void OnLootStateChange(Unit* /*user*/)
    {
        if (m_go->GetLootState() == GO_JUST_DEACTIVATED)
        {
            if (Creature* whelgar = GetClosestCreatureWithEntry(m_go, NPC_PROSPECTOR_WHELGAR, 10.0f))
            {
                whelgar->HandleEmote(EMOTE_ONESHOT_TALK);
                DoBroadcastText(SAY_ITEM_LOOTED, whelgar);
            }
        }
    }
};

GameObjectAI* GetAI_go_flagongut_fossil(GameObject* go)
{
    return new go_flagongut_fossil(go);
}


/*######
## AddSC
######*/

void AddSC_wetlands()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_tapoke_slim_jahn";
    pNewScript->GetAI = &GetAI_npc_tapoke_slim_jahn;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_mikhail";
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_mikhail;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_flagongut_fossil";
    pNewScript->GetGameObjectAI = &GetAI_go_flagongut_fossil;
    pNewScript->RegisterSelf();
}
