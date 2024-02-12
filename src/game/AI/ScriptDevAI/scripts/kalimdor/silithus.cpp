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
SDName: Silithus
SD%Complete: 100
SDComment: 
SDCategory: Silithus
EndScriptData
ContentData
EndContentData
*/

#include "AI/ScriptDevAI/include/sc_common.h"


/*#####
## npc_nelson_the_nice
######*/

enum
{
    SPELL_FOOLS_PLIGHT          = 23504,

    SPELL_SOUL_FLAME            = 23272,
    SPELL_DREADFUL_FRIGHT       = 23275,
    SPELL_CREEPING_DOOM         = 23589,
    SPELL_CRIPPLING_CLIP        = 23279,

    EMOTE_IMMOBILIZED           = -1001252,

    SPELL_FROST_TRAP            = 13810,

    NPC_NELSON_THE_NICE         = 14536,
    NPC_SOLENOR_THE_SLAYER      = 14530,
    NPC_CREEPING_DOOM           = 14761,
    NPC_THE_CLEANER             = 14503,

    QUEST_STAVE_OF_THE_ANCIENTS = 7636
};

#define GOSSIP_ITEM                 "Show me your real face, demon."

/*#####
## npc_solenor_the_slayer
######*/

struct npc_solenorAI : public ScriptedAI
{
    npc_solenorAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_bTransform = false;
        m_uiDespawn_Timer = 0;
        Reset();
    }

    uint32 m_uiTransform_Timer;
    uint32 m_uiTransformEmote_Timer;
    bool m_bTransform;

    ObjectGuid m_hunterGuid;
    uint32 m_uiDreadfulFright_Timer;
    uint32 m_uiCreepingDoom_Timer;
    uint32 m_uiCastSoulFlame_Timer;
    uint32 m_uiDespawn_Timer;

    void Reset() override
    {
        switch (m_creature->GetEntry())
        {
            case NPC_NELSON_THE_NICE:
                m_creature->SetRespawnDelay(35 * MINUTE);
                m_creature->SetRespawnTime(35 * MINUTE);
                m_creature->NearTeleportTo(-7724.21f, 1676.43f, 7.0571f, 4.80044f);
                if (m_creature->GetMotionMaster()->GetCurrentMovementGeneratorType() != WAYPOINT_MOTION_TYPE)
                {
                    m_creature->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                    m_creature->GetMotionMaster()->Initialize();
                }

                m_creature->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

                m_uiTransform_Timer = 10000;
                m_uiTransformEmote_Timer = 5000;
                m_bTransform = false;
                m_uiDespawn_Timer = 0;
                m_uiCastSoulFlame_Timer = 0;
                break;
            case NPC_SOLENOR_THE_SLAYER:
                if (!m_uiDespawn_Timer)
                {
                    m_uiDespawn_Timer = 20 * MINUTE*IN_MILLISECONDS;
                    m_uiCastSoulFlame_Timer = 150;
                    DoCastSpellIfCan(m_creature, SPELL_SOUL_FLAME);
                }

                m_hunterGuid.Clear();
                m_uiDreadfulFright_Timer = urand(10000, 15000);
                m_uiCreepingDoom_Timer = urand(3000, 6000);
                break;
        }
    }

    /** Nelson the Nice */
    void Transform()
    {
        m_creature->UpdateEntry(NPC_SOLENOR_THE_SLAYER);
        Reset();
    }

    void BeginEvent(ObjectGuid playerGuid)
    {
        m_hunterGuid = playerGuid;
        m_creature->GetMotionMaster()->Clear(false);
        m_creature->GetMotionMaster()->MoveIdle();
        m_creature->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        m_bTransform = true;
    }

    /** Solenor the Slayer */
    void Aggro(Unit* pWho) override
    {
        if (pWho->getClass() == CLASS_HUNTER && (m_hunterGuid.IsEmpty() || m_hunterGuid == pWho->GetObjectGuid())/*&& pWho->GetQuestStatus(QUEST_STAVE_OF_THE_ANCIENTS) == QUEST_STATUS_INCOMPLETE*/)
        {
            m_hunterGuid = pWho->GetObjectGuid();
        }
        else
            DemonDespawn();
    }

    void EnterEvadeMode() override
    {
        m_creature->RemoveGuardians();

        ScriptedAI::EnterEvadeMode();
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (m_creature->GetVictim())
            pSummoned->AI()->AttackStart(m_creature->GetVictim());
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        uint32 m_respawn_delay_Timer = 2 * HOUR;
        m_creature->SetRespawnDelay(m_respawn_delay_Timer);
        m_creature->SetRespawnTime(m_respawn_delay_Timer);
        m_creature->SaveRespawnTime();
    }

    void DemonDespawn(bool triggered = true)
    {
        m_creature->RemoveGuardians();
        m_creature->SetRespawnDelay(15 * MINUTE);
        m_creature->SetRespawnTime(15 * MINUTE);
        m_creature->SaveRespawnTime();

        if (triggered)
        {
            Creature* pCleaner = m_creature->SummonCreature(NPC_THE_CLEANER, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), m_creature->GetAngle(m_creature), TEMPSPAWN_DEAD_DESPAWN, 20 * MINUTE*IN_MILLISECONDS);
            if (pCleaner)
            {
                ThreatList const& tList = m_creature->getThreatManager().getThreatList();

                for (auto itr : tList)
                {
                    if (Unit* pUnit = m_creature->GetMap()->GetUnit(itr->getUnitGuid()))
                    {
                        if (pUnit->IsAlive())
                            pCleaner->AI()->AttackStart(pUnit);
                    }
                }
            }
        }

        m_creature->ForcedDespawn();
    }

    void SpellHit(Unit* /*pCaster*/, const SpellEntry* pSpell) override
    {

        if (pSpell && pSpell->Id == 14268)   // Wing Clip (Rank 3)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_CRIPPLING_CLIP, CAST_TRIGGERED) == CAST_OK)
                DoScriptText(EMOTE_IMMOBILIZED, m_creature);
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        /** Nelson the Nice */
        if (m_bTransform)
        {
            if (m_uiTransformEmote_Timer)
            {
                if (m_uiTransformEmote_Timer <= uiDiff)
                {
                    m_creature->HandleEmote(EMOTE_ONESHOT_LAUGH);
                    m_uiTransformEmote_Timer = 0;
                }
                else
                    m_uiTransformEmote_Timer -= uiDiff;
            }

            if (m_uiTransform_Timer < uiDiff)
            {
                m_bTransform = false;
                Transform();
            }
            else
                m_uiTransform_Timer -= uiDiff;
        }

        /** Solenor the Slayer */
        if (m_uiDespawn_Timer)
        {
            if (m_uiDespawn_Timer <= uiDiff)
            {
                if (m_creature->IsAlive() && !m_creature->IsInCombat())
                    DemonDespawn(false);
            }
            else
                m_uiDespawn_Timer -= uiDiff;
        }

        if (m_uiCastSoulFlame_Timer)
        {
            if (m_uiCastSoulFlame_Timer <= uiDiff)
            {
                // delay this cast so spell animation is visible to the player
                DoCastSpellIfCan(m_creature, SPELL_SOUL_FLAME);
                m_uiCastSoulFlame_Timer = 0;
            }
            else
                m_uiCastSoulFlame_Timer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        if (m_creature->HasAura(SPELL_SOUL_FLAME) && m_creature->HasAura(SPELL_FROST_TRAP))
            m_creature->RemoveAurasDueToSpell(SPELL_SOUL_FLAME);

        if (m_creature->getThreatManager().getThreatList().size() > 1 /*|| pHunter->IsDead()*/)
            DemonDespawn();

        if (m_uiCreepingDoom_Timer < uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_CREEPING_DOOM);
            m_uiCreepingDoom_Timer = 15000;
        }
        else
            m_uiCreepingDoom_Timer -= uiDiff;

        if (m_uiDreadfulFright_Timer < uiDiff)
        {
            if (Unit* pUnit = m_creature->GetVictim())
            {
                if (m_creature->GetDistance(pUnit, false) > 5.0f)
                {
                    if (DoCastSpellIfCan(pUnit, SPELL_DREADFUL_FRIGHT) == CAST_OK)
                        m_uiDreadfulFright_Timer = urand(15000, 20000);
                }
            }
        }
        else
            m_uiDreadfulFright_Timer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

bool GossipHello_npc_solenor(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_STAVE_OF_THE_ANCIENTS) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_ITEM, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_solenor(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 /*uiAction*/)
{
    pPlayer->CLOSE_GOSSIP_MENU();
    ((npc_solenorAI*)pCreature->AI())->BeginEvent(pPlayer->GetObjectGuid());
    return true;
}

UnitAI* GetAI_npc_solenor(Creature* pCreature)
{
    return new npc_solenorAI(pCreature);
}

void AddSC_silithus()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_solenor"; // npc_solenor_the_slayer
    pNewScript->GetAI = &GetAI_npc_solenor;
    pNewScript->pGossipHello = &GossipHello_npc_solenor;
    pNewScript->pGossipSelect = &GossipSelect_npc_solenor;
    pNewScript->RegisterSelf();
}
