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
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"/* ContentData
npc_lazy_peon
EndContentData */



/*######
## npc_lazy_peon
######*/

enum
{
    SAY_PEON_AWOKEN      = -1000795,

    SOUND_PEON_WORK      = 6197,
    SOUND_PEON_AWAKE_1   = 6292,
    SOUND_PEON_AWAKE_2   = 6294,

    SPELL_PEON_SLEEP     = 17743,
    SPELL_AWAKEN_PEON    = 19938,

    NPC_SLEEPING_PEON    = 10556,
    GO_LUMBERPILE        = 175784,
};

struct npc_lazy_peonAI : public ScriptedAI
{
    npc_lazy_peonAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    uint32 m_uiSleepTimer;  //Time, until the npc goes to sleep
    uint32 m_uiWakeTimer;   //Time, until the npc wakes up on its own
    uint32 m_uiChopTimer;   //Time, until the npc stops chopping at the tree
    uint32 m_uiKneelTimer;  //Time, delay kneeling to allow sound to finish playing
    uint32 m_uiGetUpTimer;  //Time, npc stays at the pile so kneel animation can finish
    bool m_bIsAtPile;
    float m_fSpawnO;

    void RestartWakeTimer()
    {
        m_uiWakeTimer = urand(100000, 120000);  //Random wakeup timer 1m40s to 2m, retail times unknown, 2m is duration of sleep aura
        m_uiSleepTimer = 0;                     //Ensure only one of the two timers is running at any given time
    }

    void RestartSleepTimer()
    {
        m_uiSleepTimer = 80000;     //Go to sleep after 1m20s, confirmed blizzlike
        m_uiWakeTimer = 0;          //Ensure only one of the two timers is running at any given time
    }

    void Reset() override
    {
        RestartWakeTimer();
        m_uiGetUpTimer = 0;
        m_uiKneelTimer = 0;
        m_uiChopTimer = 0;
        m_bIsAtPile = false;
    }

    void Awaken(Unit* pInvoker)
    {
        if (pInvoker->GetTypeId() == TYPEID_PLAYER)
        {
            DoScriptText(SAY_PEON_AWOKEN, m_creature, pInvoker);
            m_creature->PlayDistanceSound(urand(0, 1) ? SOUND_PEON_AWAKE_1 : SOUND_PEON_AWAKE_2);
            m_creature->SetWalk(false); //peon will run if awoken by player
            ((Player*)pInvoker)->KilledMonsterCredit(m_creature->GetEntry(), m_creature->GetObjectGuid());
        }
        else
            m_creature->SetWalk(true); //peon will walk if awoken on its own

        m_creature->RemoveAurasDueToSpell(SPELL_PEON_SLEEP);
        RestartSleepTimer();
        GoLumberPile();
    }

    void GoLumberPile()
    {
        if (GameObject* pLumber = GetClosestGameObjectWithEntry(m_creature, GO_LUMBERPILE, 15.0f))
        {
            float fX, fY, fZ;
            pLumber->GetContactPoint(m_creature, fX, fY, fZ, CONTACT_DISTANCE);
            m_creature->HandleEmote(EMOTE_STATE_NONE); //cancel chopping
            m_creature->GetMotionMaster()->MovePoint(1, fX, fY, fZ);
            m_uiChopTimer = 0;
            m_uiKneelTimer = 2500; //timer duration guessed, true retail time unknown
        }
        else
            script_error_log("No GameObject of entry %u was found in range or something really bad happened.", GO_LUMBERPILE);
    }

    void MovementInform(uint32 uiMotionType, uint32 uiPointId) override
    {
        if (uiMotionType == POINT_MOTION_TYPE && uiPointId)
        {
            switch (uiPointId)
            {
                case 1: //lumber pile
                {
                    m_creature->SetWalk(true);
                    m_bIsAtPile = true;
                    break;
                }
                case 2: //spawn
                {
                    m_creature->GetMotionMaster()->MoveIdle();
                    m_creature->HandleEmote(EMOTE_STATE_NONE);
                    m_creature->SetFacingTo(m_fSpawnO);
                    DoCastSpellIfCan(m_creature, SPELL_PEON_SLEEP);
                    RestartWakeTimer();
                    break;
                }
            }
        }
        else if (uiMotionType == WAYPOINT_MOTION_TYPE)
        {
            //tree
            m_creature->GetMotionMaster()->MoveIdle();
            m_creature->PlayDistanceSound(SOUND_PEON_WORK);
            m_creature->HandleEmote(EMOTE_STATE_WORK_CHOPWOOD);
            m_uiChopTimer = urand(25000, 30000); //timer duration guessed, true retail time unknown
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiGetUpTimer)
        {
            if (m_uiGetUpTimer <= uiDiff)
            {
                m_uiGetUpTimer = 0;
                m_creature->HandleEmote(EMOTE_STATE_NONE);
                m_creature->GetMotionMaster()->MoveWaypoint();
                m_bIsAtPile = false;
            }
            else
                m_uiGetUpTimer -= uiDiff;
        }

        if (m_uiKneelTimer)
        {
            if (m_uiKneelTimer <= uiDiff)
            {
                if (m_bIsAtPile)
                {
                    m_uiKneelTimer = 0;
                    m_creature->HandleEmote(EMOTE_ONESHOT_KNEEL);
                    m_uiGetUpTimer = 3000; //timer duration guessed, true retail time unknown
                }
                else
                    m_uiKneelTimer = 1; //guarantee it's checked again on next update
            }
            else
                m_uiKneelTimer -= uiDiff;
        }

        if (m_uiChopTimer)
        {
            if (m_uiChopTimer <= uiDiff)
            {
                m_uiChopTimer = 0;
                GoLumberPile();
            }
            else
                m_uiChopTimer -= uiDiff;
        }

        if (m_uiSleepTimer)
        {
            if (m_uiSleepTimer <= uiDiff)
            {
                float fX, fY, fZ;
                m_creature->GetRespawnCoord(fX, fY, fZ, &m_fSpawnO);
                m_uiSleepTimer = 0;
                m_creature->GetMotionMaster()->MovePoint(2, fX, fY, fZ);
                m_bIsAtPile = false;
                m_uiGetUpTimer = 0;
                m_uiKneelTimer = 0;
                m_uiChopTimer = 0;
            }
            else
                m_uiSleepTimer -= uiDiff;
        }

        if (m_uiWakeTimer)
        {
            if (m_uiWakeTimer <= uiDiff)
                Awaken(m_creature);
            else
            {
                //reapply aura if it fell off while the peon was unloaded
                if (!m_creature->HasAura(SPELL_PEON_SLEEP))
                    DoCastSpellIfCan(m_creature, SPELL_PEON_SLEEP);
                //decrement timer
                m_uiWakeTimer -= uiDiff;
            }
        }
    }
};

UnitAI* GetAI_npc_lazy_peon(Creature* pCreature)
{
    return new npc_lazy_peonAI(pCreature);
}

bool EffectDummyCreature_lazy_peon_spell(Unit* pCaster, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    //make sure it's the right spell
    if (uiSpellId == SPELL_AWAKEN_PEON && uiEffIndex == EFFECT_INDEX_0)
    {
        //awaken peon
        if (npc_lazy_peonAI* pPeonAI = dynamic_cast<npc_lazy_peonAI*>(pCreatureTarget->AI()))
            pPeonAI->Awaken(pCaster);
        //right spell, return true
        return true;
    }
    //wrong spell, return false
    return false;
}

void AddSC_durotar()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_lazy_peon";
    pNewScript->GetAI = &GetAI_npc_lazy_peon;
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_lazy_peon_spell;
    pNewScript->RegisterSelf();
}
