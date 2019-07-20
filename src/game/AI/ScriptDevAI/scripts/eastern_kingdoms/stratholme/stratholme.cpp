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
SDName: Stratholme
SD%Complete: 100
SDComment: Misc mobs for instance. GO-script to apply aura and start event for quest 8945
SDCategory: Stratholme
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"/* ContentData
go_service_gate
go_gauntlet_gate
go_stratholme_postbox
mob_restless_soul
mobs_spectral_ghostly_citizen
npc_aurius
EndContentData */


#include "stratholme.h"

/*######
## go_service_gate
######*/

bool GOUse_go_service_gate(Player* /*pPlayer*/, GameObject* pGo)
{
    ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData();

    if (!pInstance)
        return false;

    if (pInstance->GetData(TYPE_BARTHILAS_RUN) != NOT_STARTED)
        return false;

    // if the service gate is used make Barthilas flee
    pInstance->SetData(TYPE_BARTHILAS_RUN, IN_PROGRESS);
    return false;
}

/*######
## go_gauntlet_gate (this is the _first_ of the gauntlet gates, two exist)
######*/

bool GOUse_go_gauntlet_gate(Player* pPlayer, GameObject* pGo)
{
    ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData();

    if (!pInstance)
        return false;

    if (pInstance->GetData(TYPE_BARON_RUN) != NOT_STARTED)
        return false;

    if (Group* pGroup = pPlayer->GetGroup())
    {
        for (GroupReference* itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* pGroupie = itr->getSource();
            if (!pGroupie)
                continue;

            if (!pGroupie->HasAura(SPELL_BARON_ULTIMATUM))
                pGroupie->CastSpell(pGroupie, SPELL_BARON_ULTIMATUM, TRIGGERED_OLD_TRIGGERED);
        }
    }
    else
    {
        if (!pPlayer->HasAura(SPELL_BARON_ULTIMATUM))
            pPlayer->CastSpell(pPlayer, SPELL_BARON_ULTIMATUM, TRIGGERED_OLD_TRIGGERED);
    }

    pInstance->SetData(TYPE_BARON_RUN, IN_PROGRESS);
    return false;
}

/*######
## go_stratholme_postbox
######*/

bool GOUse_go_stratholme_postbox(Player* pPlayer, GameObject* pGo)
{
    ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData();

    if (!pInstance)
        return false;

    if (pInstance->GetData(TYPE_POSTMASTER) == DONE)
        return false;

    // When the data is Special, spawn the postmaster
    if (pInstance->GetData(TYPE_POSTMASTER) == SPECIAL)
    {
        pPlayer->CastSpell(pPlayer, SPELL_SUMMON_POSTMASTER, TRIGGERED_OLD_TRIGGERED);
        pInstance->SetData(TYPE_POSTMASTER, DONE);
    }
    else
        pInstance->SetData(TYPE_POSTMASTER, IN_PROGRESS);

    // Summon 3 postmen for each postbox
    float fX, fY, fZ;
    for (uint8 i = 0; i < 3; ++i)
    {
        pPlayer->GetRandomPoint(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 3.0f, fX, fY, fZ);
        pPlayer->SummonCreature(NPC_UNDEAD_POSTMAN, fX, fY, fZ, 0.0f, TEMPSPAWN_DEAD_DESPAWN, 0);
    }

    return false;
}

/*######
## mob_restless_soul
######*/

enum
{
    // Possibly more of these quotes around.
    SAY_ZAPPED0     = -1329000,
    SAY_ZAPPED1     = -1329001,
    SAY_ZAPPED2     = -1329002,
    SAY_ZAPPED3     = -1329003,

    QUEST_RESTLESS_SOUL     = 5282,

    SPELL_EGAN_BLASTER      = 17368,
    SPELL_SOUL_FREED        = 17370,

    NPC_RESTLESS_SOUL      = 11122,
    NPC_FREED_SOUL         = 11136,
};

// TODO - likely entirely not needed workaround
struct mob_restless_soulAI : public ScriptedAI
{
    mob_restless_soulAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    ObjectGuid m_taggerGuid;
    uint32 m_uiDieTimer;
    bool m_bIsTagged;

    void Reset() override
    {
        m_taggerGuid.Clear();
        m_uiDieTimer = 5000;
        m_bIsTagged = false;
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell) override
    {
        if (pCaster->GetTypeId() == TYPEID_PLAYER)
        {
            if (!m_bIsTagged && pSpell->Id == SPELL_EGAN_BLASTER && ((Player*)pCaster)->GetQuestStatus(QUEST_RESTLESS_SOUL) == QUEST_STATUS_INCOMPLETE)
            {
                m_bIsTagged = true;
                m_taggerGuid = pCaster->GetObjectGuid();
            }
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_FREED_SOUL)
        {
            pSummoned->CastSpell(pSummoned, SPELL_SOUL_FREED, TRIGGERED_NONE);

            switch (urand(0, 3))
            {
                case 0: DoScriptText(SAY_ZAPPED0, pSummoned); break;
                case 1: DoScriptText(SAY_ZAPPED1, pSummoned); break;
                case 2: DoScriptText(SAY_ZAPPED2, pSummoned); break;
                case 3: DoScriptText(SAY_ZAPPED3, pSummoned); break;
            }
        }
    }

    void JustDied(Unit* /*Killer*/) override
    {
        if (m_bIsTagged)
            m_creature->SummonCreature(NPC_FREED_SOUL, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSPAWN_TIMED_DESPAWN, 300000);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_bIsTagged)
        {
            if (m_uiDieTimer < uiDiff)
            {
                if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_taggerGuid))
                    pPlayer->DealDamage(m_creature, m_creature->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
            }
            else
                m_uiDieTimer -= uiDiff;
        }
    }
};

UnitAI* GetAI_mob_restless_soul(Creature* pCreature)
{
    return new mob_restless_soulAI(pCreature);
}

/*######
## mobs_spectral_ghostly_citizen
######*/

enum
{
    SPELL_HAUNTING_PHANTOM  = 16336,
    SPELL_SLAP              = 6754
};

struct mobs_spectral_ghostly_citizenAI : public ScriptedAI
{
    mobs_spectral_ghostly_citizenAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiDieTimer;
    bool m_bIsTagged;

    void Reset() override
    {
        m_uiDieTimer = 5000;
        m_bIsTagged = false;
    }

    void SpellHit(Unit* /*pCaster*/, const SpellEntry* pSpell) override
    {
        if (!m_bIsTagged && pSpell->Id == SPELL_EGAN_BLASTER)
            m_bIsTagged = true;
    }

    void JustDied(Unit* /*Killer*/) override
    {
        if (m_bIsTagged)
        {
            for (uint32 i = 0; i < 4; ++i)
            {
                float x, y, z;
                m_creature->GetRandomPoint(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 20.0f, x, y, z);

                // 100%, 50%, 33%, 25% chance to spawn
                uint32 j = urand(0, i);
                if (j == 0)
                    m_creature->SummonCreature(NPC_RESTLESS_SOUL, x, y, z, 0, TEMPSPAWN_DEAD_DESPAWN, 0);
            }
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_bIsTagged)
        {
            if (m_uiDieTimer < uiDiff)
            {
                m_creature->DealDamage(m_creature, m_creature->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
            }
            else
                m_uiDieTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }

    void ReceiveEmote(Player* pPlayer, uint32 uiEmote) override
    {
        switch (uiEmote)
        {
            case TEXTEMOTE_DANCE:
                EnterEvadeMode();
                break;
            case TEXTEMOTE_RUDE:
                if (m_creature->IsWithinDistInMap(pPlayer, INTERACTION_DISTANCE))
                    m_creature->CastSpell(pPlayer, SPELL_SLAP, TRIGGERED_NONE);
                else
                    m_creature->HandleEmote(EMOTE_ONESHOT_RUDE);
                break;
            case TEXTEMOTE_WAVE:
                m_creature->HandleEmote(EMOTE_ONESHOT_WAVE);
                break;
            case TEXTEMOTE_BOW:
                m_creature->HandleEmote(EMOTE_ONESHOT_BOW);
                break;
            case TEXTEMOTE_KISS:
                m_creature->HandleEmote(EMOTE_ONESHOT_FLEX);
                break;
        }
    }
};

UnitAI* GetAI_mobs_spectral_ghostly_citizen(Creature* pCreature)
{
    return new mobs_spectral_ghostly_citizenAI(pCreature);
}

/*######
## npc_aurius
######*/

enum
{
    GOSSIP_TEXT_AURIUS_1  = 3755,
    GOSSIP_TEXT_AURIUS_2  = 3756,
    GOSSIP_TEXT_AURIUS_3  = 3757,
};

bool QuestRewarded_npc_aurius(Player* /*pPlayer*/, Creature* pCreature, const Quest* pQuest)
{
    ScriptedInstance* pInstance = (ScriptedInstance*)pCreature->GetInstanceData();

    if (!pInstance)
        return false;

    if (pInstance->GetData(TYPE_BARON) == DONE || pInstance->GetData(TYPE_AURIUS) == DONE)
        return false;

    if ((pQuest->GetQuestId() == QUEST_MEDALLION_FAITH))
        pInstance->SetData(TYPE_AURIUS, DONE);

    return true;
}

bool GossipHello_npc_aurius(Player* pPlayer, Creature* pCreature)
{
    ScriptedInstance* pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
    if (!pInstance)
        return false;

    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    uint32 uiGossipId;

    // Baron encounter is complete and Aurius helped
    if (pInstance->GetData(TYPE_BARON) == DONE && pInstance->GetData(TYPE_AURIUS) == DONE)
        uiGossipId = GOSSIP_TEXT_AURIUS_3;
    // Aurius rewarded the quest
    else if (pInstance->GetData(TYPE_AURIUS) == DONE)
        uiGossipId = GOSSIP_TEXT_AURIUS_2;
    else
        uiGossipId = GOSSIP_TEXT_AURIUS_1;

    pPlayer->SEND_GOSSIP_MENU(uiGossipId, pCreature->GetObjectGuid());
    return true;
}

void AddSC_stratholme()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "go_service_gate";
    pNewScript->pGOUse = &GOUse_go_service_gate;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_gauntlet_gate";
    pNewScript->pGOUse = &GOUse_go_gauntlet_gate;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_stratholme_postbox";
    pNewScript->pGOUse = &GOUse_go_stratholme_postbox;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_restless_soul";
    pNewScript->GetAI = &GetAI_mob_restless_soul;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mobs_spectral_ghostly_citizen";
    pNewScript->GetAI = &GetAI_mobs_spectral_ghostly_citizen;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_aurius";
    pNewScript->pGossipHello =  &GossipHello_npc_aurius;
    pNewScript->pQuestRewardedNPC = &QuestRewarded_npc_aurius;
    pNewScript->RegisterSelf();
}
