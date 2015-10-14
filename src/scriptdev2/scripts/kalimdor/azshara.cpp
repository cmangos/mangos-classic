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
SDName: Azshara
SD%Complete: 90
SDComment: Quest support: 2744, 3141, 9364, 10994
SDCategory: Azshara
EndScriptData */

/* ContentData
mobs_spitelashes
npc_loramus_thalipedes
EndContentData */

#include "precompiled.h"

/*######
## mobs_spitelashes
######*/
enum
{
    // quest related
    SPELL_POLYMORPH_BACKFIRE    = 28406,        // summons npc 16479
    QUEST_FRAGMENTED_MAGIC      = 9364,

    // npc spells
    SPELL_DISARM                = 6713,         // warrior
    SPELL_SCREECH               = 3589,         // screamer
    SPELL_FROST_SHOCK           = 12548,        // serpent guard
    SPELL_RENEW                 = 11640,        // siren
    SPELL_SHOOT                 = 6660,
    SPELL_FROST_SHOT            = 12551,
    SPELL_FROST_NOVA            = 11831,
    SPELL_STRIKE                = 11976,        // myrmidon

    NPC_SPITELASH_WARRIOR       = 6190,
    NPC_SPITELASH_SCREAMER      = 6193,
    NPC_SPITELASH_GUARD         = 6194,
    NPC_SPITELASH_SIREN         = 6195,
    NPC_SPITELASH_MYRMIDON      = 6196,

    TARGET_TYPE_RANDOM          = 0,
    TARGET_TYPE_VICTIM          = 1,
    TARGET_TYPE_SELF            = 2,
    TARGET_TYPE_FRIENDLY        = 3,
};

struct SpitelashAbilityStruct
{
    uint32 m_uiCreatureEntry, m_uiSpellId;
    uint8 m_uiTargetType;
    uint32 m_uiInitialTimer, m_uiCooldown;
};

static SpitelashAbilityStruct m_aSpitelashAbility[8] =
{
    {NPC_SPITELASH_WARRIOR,     SPELL_DISARM,       TARGET_TYPE_VICTIM,     4000,  10000},
    {NPC_SPITELASH_SCREAMER,    SPELL_SCREECH,      TARGET_TYPE_SELF,       7000,  15000},
    {NPC_SPITELASH_GUARD,       SPELL_FROST_SHOCK,  TARGET_TYPE_VICTIM,     7000,  13000},
    {NPC_SPITELASH_SIREN,       SPELL_RENEW,        TARGET_TYPE_FRIENDLY,   4000,  7000},
    {NPC_SPITELASH_SIREN,       SPELL_SHOOT,        TARGET_TYPE_RANDOM,     3000,  9000},
    {NPC_SPITELASH_SIREN,       SPELL_FROST_SHOT,   TARGET_TYPE_RANDOM,     7000,  10000},
    {NPC_SPITELASH_SIREN,       SPELL_FROST_NOVA,   TARGET_TYPE_SELF,       10000, 15000},
    {NPC_SPITELASH_MYRMIDON,    SPELL_STRIKE,       TARGET_TYPE_VICTIM,     3000,  7000}
};

struct mobs_spitelashesAI : public ScriptedAI
{
    mobs_spitelashesAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        for (uint8 i = 0; i < countof(m_aSpitelashAbility); ++i)
        {
            if (m_aSpitelashAbility[i].m_uiCreatureEntry == m_creature->GetEntry())
                m_mSpellTimers[i] = m_aSpitelashAbility[i].m_uiInitialTimer;
        }

        Reset();
    }

    uint32 m_uiMorphTimer;

    std::unordered_map<uint8, uint32> m_mSpellTimers;

    void Reset() override
    {
        m_uiMorphTimer = 0;

        for (std::unordered_map<uint8, uint32>::iterator itr = m_mSpellTimers.begin(); itr != m_mSpellTimers.end(); ++itr)
            itr->second = m_aSpitelashAbility[itr->first].m_uiInitialTimer;
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell) override
    {
        // If already hit by the polymorph return
        if (m_uiMorphTimer)
            return;

        // Creature get polymorphed into a sheep and after 5 secs despawns
        if (pCaster->GetTypeId() == TYPEID_PLAYER && ((Player*)pCaster)->GetQuestStatus(QUEST_FRAGMENTED_MAGIC) == QUEST_STATUS_INCOMPLETE &&
                (pSpell->Id == 118 || pSpell->Id == 12824 || pSpell->Id == 12825 || pSpell->Id == 12826))
            m_uiMorphTimer = 5000;
    }

    bool CanUseSpecialAbility(uint32 uiIndex)
    {
        Unit* pTarget = nullptr;

        switch (m_aSpitelashAbility[uiIndex].m_uiTargetType)
        {
            case TARGET_TYPE_SELF:
                pTarget = m_creature;
                break;
            case TARGET_TYPE_VICTIM:
                pTarget = m_creature->getVictim();
                break;
            case TARGET_TYPE_RANDOM:
                pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, m_aSpitelashAbility[uiIndex].m_uiSpellId, SELECT_FLAG_IN_LOS);
                break;
            case TARGET_TYPE_FRIENDLY:
                pTarget = DoSelectLowestHpFriendly(10.0f);
                break;
        }

        if (pTarget)
        {
            if (DoCastSpellIfCan(pTarget, m_aSpitelashAbility[uiIndex].m_uiSpellId) == CAST_OK)
                return true;
        }

        return false;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiMorphTimer)
        {
            if (m_uiMorphTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_POLYMORPH_BACKFIRE, CAST_TRIGGERED) == CAST_OK)
                {
                    m_uiMorphTimer = 0;
                    m_creature->ForcedDespawn();
                }
            }
            else
                m_uiMorphTimer -= uiDiff;
        }

        for (std::unordered_map<uint8, uint32>::iterator itr = m_mSpellTimers.begin(); itr != m_mSpellTimers.end(); ++itr)
        {
            if (itr->second < uiDiff)
            {
                if (CanUseSpecialAbility(itr->first))
                {
                    itr->second = m_aSpitelashAbility[itr->first].m_uiCooldown;
                    break;
                }
            }
            else
                itr->second -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mobs_spitelashes(Creature* pCreature)
{
    return new mobs_spitelashesAI(pCreature);
}

/*######
## npc_loramus_thalipedes
######*/

bool GossipHello_npc_loramus_thalipedes(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->GetQuestStatus(2744) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Can you help me?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    if (pPlayer->GetQuestStatus(3141) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Tell me your story", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_loramus_thalipedes(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->AreaExploredOrEventHappens(2744);
            break;

        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Please continue", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 21);
            pPlayer->SEND_GOSSIP_MENU(1813, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+21:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "I do not understand", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 22);
            pPlayer->SEND_GOSSIP_MENU(1814, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+22:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Indeed", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 23);
            pPlayer->SEND_GOSSIP_MENU(1815, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+23:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "I will do this with or your help, Loramus", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 24);
            pPlayer->SEND_GOSSIP_MENU(1816, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+24:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Yes", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 25);
            pPlayer->SEND_GOSSIP_MENU(1817, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+25:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->AreaExploredOrEventHappens(3141);
            break;
    }
    return true;
}

void AddSC_azshara()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "mobs_spitelashes";
    pNewScript->GetAI = &GetAI_mobs_spitelashes;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_loramus_thalipedes";
    pNewScript->pGossipHello =  &GossipHello_npc_loramus_thalipedes;
    pNewScript->pGossipSelect = &GossipSelect_npc_loramus_thalipedes;
    pNewScript->RegisterSelf();
}
