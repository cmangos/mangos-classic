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
SDName: Durotar
SD%Complete: 100
SDComment: Quest support: 5441
SDCategory: Durotar
EndScriptData */

/* ContentData
npc_lazy_peon
EndContentData */

#include "precompiled.h"

/*######
## npc_lazy_peon
######*/

enum
{
    SAY_PEON_AWAKE_1        = -1000795,
    SAY_PEON_AWAKE_2        = -1000796,

    SPELL_PEON_SLEEP        = 17743,
    SPELL_AWAKEN_PEON       = 19938,

    NPC_SLEEPING_PEON       = 10556,
    GO_LUMBERPILE           = 175784,
};

struct npc_lazy_peonAI : public ScriptedAI
{
    npc_lazy_peonAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
        m_uiStopSleepingTimer = urand(30000, 120000);       // Set on spawn to a potential small timer, to get nice results for initial case
    }

    uint32 m_uiResetSleepTimer;                             // Time, until the npc stops harvesting lumber
    uint32 m_uiStopSleepingTimer;                           // Time, until the npcs (re)starts working on its own

    void Reset() override
    {
        m_uiResetSleepTimer = 0;
        m_uiStopSleepingTimer = urand(90000, 120000);       // Sleeping aura has only 2min duration
    }

    // Can also be self invoked for random working
    void StartLumbering(Unit* pInvoker)
    {
        m_uiStopSleepingTimer = 0;
        if (GameObject* pLumber = GetClosestGameObjectWithEntry(m_creature, GO_LUMBERPILE, 15.0f))
        {
            m_creature->RemoveAurasDueToSpell(SPELL_PEON_SLEEP);

            float fX, fY, fZ;
            m_creature->SetWalk(false);
            pLumber->GetContactPoint(m_creature, fX, fY, fZ, CONTACT_DISTANCE);

            if (pInvoker->GetTypeId() == TYPEID_PLAYER)
            {
                DoScriptText(SAY_PEON_AWAKE_1, m_creature);
                ((Player*)pInvoker)->KilledMonsterCredit(m_creature->GetEntry(), m_creature->GetObjectGuid());
                m_creature->GetMotionMaster()->MovePoint(1, fX, fY, fZ);
            }
            else
                m_creature->GetMotionMaster()->MovePoint(2, fX, fY, fZ);
        }
        else
            script_error_log("No GameObject of entry %u was found in range or something really bad happened.", GO_LUMBERPILE);
    }

    void MovementInform(uint32 uiMotionType, uint32 uiPointId) override
    {
        if (uiMotionType != POINT_MOTION_TYPE || !uiPointId)
            return;

        m_creature->HandleEmote(EMOTE_STATE_WORK_CHOPWOOD);
        // TODO - random bevahior for self-invoked awakening guesswork
        m_uiResetSleepTimer = uiPointId == 1 ? 80000 : urand(30000, 60000);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiResetSleepTimer)
        {
            if (m_uiResetSleepTimer <= uiDiff)
            {
                DoScriptText(SAY_PEON_AWAKE_2, m_creature);
                m_creature->HandleEmote(EMOTE_STATE_NONE);
                EnterEvadeMode();
                m_uiResetSleepTimer = 0;
            }
            else
                m_uiResetSleepTimer -= uiDiff;
        }

        if (m_uiStopSleepingTimer)
        {
            if (m_uiStopSleepingTimer <= uiDiff)
                StartLumbering(m_creature);
            else
                m_uiStopSleepingTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_npc_lazy_peon(Creature* pCreature)
{
    return new npc_lazy_peonAI(pCreature);
}

bool EffectDummyCreature_lazy_peon_awake(Unit* pCaster, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    // always check spellid and effectindex
    if (uiSpellId == SPELL_AWAKEN_PEON && uiEffIndex == EFFECT_INDEX_0)
    {
        if (!pCreatureTarget->HasAura(SPELL_PEON_SLEEP) || pCaster->GetTypeId() != TYPEID_PLAYER || pCreatureTarget->GetEntry() != NPC_SLEEPING_PEON)
            return true;

        if (npc_lazy_peonAI* pPeonAI = dynamic_cast<npc_lazy_peonAI*>(pCreatureTarget->AI()))
            pPeonAI->StartLumbering(pCaster);

        // always return true when we are handling this spell and effect
        return true;
    }

    return false;
}

void AddSC_durotar()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_lazy_peon";
    pNewScript->GetAI = &GetAI_npc_lazy_peon;
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_lazy_peon_awake;
    pNewScript->RegisterSelf();
}
