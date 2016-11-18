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
    SAY_PEON_AWOKEN = -1000795,

    SOUND_PEON_WORK = 6197,
    SOUND_PEON_AWAKE_1 = 6292,
    SOUND_PEON_AWAKE_2 = 6294,

    SPELL_PEON_SLEEP = 17743,
    SPELL_AWAKEN_PEON = 19938,

    NPC_SLEEPING_PEON = 10556,
    GO_LUMBERPILE = 175784,
};

struct npc_lazy_peonAI : public ScriptedAI
{
    npc_lazy_peonAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
        //tree waypoint
        //TODO do this via invisible creatures once they're added in the db (or actual waypoints)
        switch (m_creature->GetGUIDLow())
        {
            case 6527:
            {
                m_fTreeX = -754.196;
                m_fTreeY = -4141.18;
                m_fTreeZ = 39.4156;
                m_fTreeO = 1.54425;
                break;
            }
            case 7373:
            {
                m_fTreeX = -775.806;
                m_fTreeY = -4196.64;
                m_fTreeZ = 44.5941;
                m_fTreeO = 2.26289;
                break;
            }
            case 6525:
            {
                m_fTreeX = -757.896;
                m_fTreeY = -4324.19;
                m_fTreeZ = 45.1184;
                m_fTreeO = 4.3012;
                break;
            }
            case 7374:
            {
                m_fTreeX = -622.255;
                m_fTreeY = -4348.61;
                m_fTreeZ = 41.0623;
                m_fTreeO = 4.83115;
                break;
            }
            case 6524:
            {
                m_fTreeX = -634.882;
                m_fTreeY = -4480.31;
                m_fTreeZ = 46.2619;
                m_fTreeO = 4.71333;
                break;
            }
            case 7376:
            {
                m_fTreeX = -498.072;
                m_fTreeY = -4455.45;
                m_fTreeZ = 51.0777;
                m_fTreeO = 3.27999;
                break;
            }
            case 3348:
            {
                m_fTreeX = -510.965;
                m_fTreeY = -4372.57;
                m_fTreeZ = 45.6692;
                m_fTreeO = 0.480042;
                break;
            }
            case 3346:
            {
                m_fTreeX = -334.424;
                m_fTreeY = -4439.34;
                m_fTreeZ = 54.6275;
                m_fTreeO = 4.45416;
                break;
            }
            case 3347:
            {
                m_fTreeX = -229.428;
                m_fTreeY = -4449.48;
                m_fTreeZ = 63.3728;
                m_fTreeO = 0.016658;
                break;
            }
            case 3345:
            {
                m_fTreeX = -225.614;
                m_fTreeY = -4284.88;
                m_fTreeZ = 65.0956;
                m_fTreeO = 5.00395;
                break;
            }
            case 7372:
            {
                m_fTreeX = -213.599;
                m_fTreeY = -4220.75;
                m_fTreeZ = 62.2392;
                m_fTreeO = 1.93932;
                break;
            }
            case 7375:
            {
                m_fTreeX = -265.354;
                m_fTreeY = -4145.13;
                m_fTreeZ = 55.767;
                m_fTreeO = 5.78542;
                break;
            }
            case 6523:
            {
                m_fTreeX = -320.191;
                m_fTreeY = -4122.77;
                m_fTreeZ = 51.3316;
                m_fTreeO = 1.09659;
                break;
            }
            case 6526:
            {
                m_fTreeX = -375.545;
                m_fTreeY = -4018.32;
                m_fTreeZ = 50.3465;
                m_fTreeO = 2.86373;
                break;
            }
            default:
            {
                //if there's no tree point, set it to current so the peon doesn't wander off
                m_fTreeX = m_creature->GetPositionX();
                m_fTreeY = m_creature->GetPositionY();
                m_fTreeZ = m_creature->GetPositionZ();
                m_fTreeO = m_creature->GetOrientation();
            }
        }
    }

    uint32 m_uiSleepTimer; //Time, until the npc goes to sleep
    uint32 m_uiWakeTimer;  //Time, until the npc wakes up on its own
    uint32 m_uiChopTimer; //Time, until the npc stops chopping at the tree
    uint32 m_uiKneelTimer; //Time, delay kneeling to allow sound to finish playing
    uint32 m_uiGetUpTimer; //Time, npc stays at the pile so kneel animation can finish
    bool m_bIsAtPile;
    float m_fTreeX;
    float m_fTreeY;
    float m_fTreeZ;
    float m_fTreeO;
    float m_fSpawnO;

    void RestartWakeTimer() {
        m_uiWakeTimer = urand(100000, 120000); //Random wakeup timer 1m40s to 2m, retail times unknown, 2m is duration of sleep aura
        m_uiSleepTimer = 0; //Ensure only one of the two timers is running at any given time
    }

    void RestartSleepTimer() {
        m_uiSleepTimer = 80000; //Go to sleep after 1m20s, confirmed blizzlike
        m_uiWakeTimer = 0; //Ensure only one of the two timers is running at any given time
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
            m_uiKneelTimer = 2500; //timer duration guessed, true retail time unknown
        }
        else
            script_error_log("No GameObject of entry %u was found in range or something really bad happened.", GO_LUMBERPILE);
    }

    void MovementInform(uint32 uiMotionType, uint32 uiPointId) override
    {
        if (uiMotionType != POINT_MOTION_TYPE || !uiPointId)
            return;

        switch (uiPointId)
        {
            case 1: //lumber pile
            {
                m_creature->SetWalk(true);
                m_bIsAtPile = true;
                break;
            }
            case 2: //tree
            {
                m_creature->SetFacingTo(m_fTreeO);
                m_creature->PlayDistanceSound(SOUND_PEON_WORK);
                m_creature->HandleEmote(EMOTE_STATE_WORK_CHOPWOOD);
                m_uiChopTimer = urand(25000, 30000); //timer duration guessed, true retail time unknown
                break;
            }
            case 3: //spawn
            {
                m_creature->SetFacingTo(m_fSpawnO);
                m_creature->GetMotionMaster()->MoveTargetedHome(); //hacky way of getting the peon to sleep
                m_creature->HandleEmote(EMOTE_STATE_NONE);
                RestartWakeTimer();
                break;
            }
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiGetUpTimer)
        {
            if (m_uiGetUpTimer <= uiDiff)
            {
                m_uiGetUpTimer = 0;
                m_creature->GetMotionMaster()->MovePoint(2, m_fTreeX, m_fTreeY, m_fTreeZ);
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
                m_creature->GetMotionMaster()->MovePoint(3, fX, fY, fZ);
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
                m_uiWakeTimer -= uiDiff;
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
        //TODO some of these checks are now redundant, should be removed by someone who knows how to do so without breaking things
        if (!pCreatureTarget->HasAura(SPELL_PEON_SLEEP) || pCaster->GetTypeId() != TYPEID_PLAYER || pCreatureTarget->GetEntry() != NPC_SLEEPING_PEON)
            return true;

        if (npc_lazy_peonAI* pPeonAI = dynamic_cast<npc_lazy_peonAI*>(pCreatureTarget->AI()))
            pPeonAI->Awaken(pCaster);

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
