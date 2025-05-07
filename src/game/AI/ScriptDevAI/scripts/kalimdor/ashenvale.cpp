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
SDName: Ashenvale
SD%Complete: 70
SDComment: Quest support: 976, 6482, 6544, 6641
SDCategory: Ashenvale Forest
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"/* ContentData
npc_muglash
npc_ruul_snowhoof
npc_torek
npc_feero_ironhand
EndContentData */


#include "AI/ScriptDevAI/base/escort_ai.h"
#include "World/WorldStateDefines.h"

/*####
# npc_muglash
####*/

enum
{
    SAY_MUGLASH_AGGRO       = 8412,
    SAY_MUGLASH_BRAZIER     = 8556,
    SAY_MUGLASH_FAIL        = 8409,
    SAY_MUGLASH_EVENT_01    = 8410,
    SAY_MUGLASH_EVENT_02    = 8413,
    SAY_MUGLASH_EVENT_03    = 8567,
    SAY_MUGLASH_EVENT_04    = 8568,
    SAY_MUGLASH_SUCCESS     = 8569,
    SAY_MUGLASH_SUCCESS_02  = 8558,
    SAY_MUGLASH_SUCCESS_03  = 8564,
    SAY_MUGLASH_SUCCESS_04  = 8565,

    QUEST_VORSHA            = 6641,
    MUGLASH_ESCORT_PATH     = 12717,
    SPELL_MUGLASH_WAITING   = 20861, // This spell triggers Muglash Despawn after 5 minutes

    GO_NAGA_BRAZIER         = 178247,

    NPC_MUGLASH             = 12717,

    // First Wave
    NPC_WRATH_RIDER         = 3713,
    NPC_WRATH_SORCERESS     = 3717,
    NPC_WRATH_RAZORTAIL     = 3712,

    // 2nd Wave
    NPC_WRATH_PRIESTESS     = 3944,
    NPC_WRATH_MYRMIDON      = 3711,
    NPC_WRATH_SEAWITCH      = 3715,

    // 3rd Wave
    NPC_VORSHA              = 12940,
};

enum MuglashActions
{
    MUGLASH_ACTION_MAX,
    MUGLASH_FAIL,
    MUGLASH_EVENT
};

struct firstWaveLocations
{
    uint32 uiEntry;
    float fX, fY, fZ;
};
// First Wave Data
static const firstWaveLocations nagaLocations[3] =
{
    { NPC_WRATH_RAZORTAIL, 3629.9194f, 1169.9987f, -3.4472558f},
    { NPC_WRATH_RIDER, 3617.8516f, 1097.7166f, -4.0877485f},
    { NPC_WRATH_SORCERESS, 3583.1497f, 1165.5658f, -5.3660164f}
};

// 2nd Wave Data
struct secondWaveLocations
{
    uint32 uiEntry;
    float fX, fY, fZ;
};
static const secondWaveLocations secondnagaLocations[3] =
{
    { NPC_WRATH_MYRMIDON, 3575.009f, 1119.8704f, -4.2547455f},
    { NPC_WRATH_SEAWITCH, 3606.9448f, 1176.3822f, -2.9632108f},
    { NPC_WRATH_PRIESTESS, 3651.5383f, 1155.5522f, -3.9628646f}
};

struct npc_muglashAI : public npc_escortAI
{
    npc_muglashAI(Creature* creature) : npc_escortAI(creature)
    {
        Reset();
        AddCustomAction(MUGLASH_FAIL, true, [&]() { DoFailEscort(); }, TIMER_COMBAT_OOC);
        AddCustomAction(MUGLASH_EVENT, true, [&]() { DoEvent(); }, TIMER_ALWAYS);
        m_uiEventId = 0;
        m_uiWaveOneAlive = 0;
        m_uiWaveTwoAlive = 0;
    }

    uint32 m_uiEventId;
    uint8 m_uiWaveOneAlive;
    uint8 m_uiWaveTwoAlive;

    void Reset() override
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            m_uiEventId = 0;
        }
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (HasEscortState(STATE_ESCORT_PAUSED))
        {
            if (urand(0, 1))
                return;

            if (Player* player = GetPlayerForEscort())
                DoBroadcastText(SAY_MUGLASH_AGGRO, m_creature, player);
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 10:
                // Before entering Water ignore mmaps so npc swims
                m_creature->SetIgnoreMMAP(true);
                break;
            case 12:
                // Intermediary Point as first point after Water
                m_creature->SetIgnoreMMAP(false);
                break;
            case 15:
                // Stop Waypoints here and start Brazier Event
                SetEscortPaused(true);
                ResetTimer(MUGLASH_EVENT, 2000);
                m_uiEventId = 0; // For Safty set EventID to 0
                break;
            case 16:
                SetEscortPaused(true);
                break;
            case 17:
                SetEscortPaused(true);
                ResetTimer(MUGLASH_EVENT, 2000);
                m_uiEventId = 5;
                break;
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*sender*/, Unit* invoker, uint32 /*miscValue*/) override
    {
        if (eventType == AI_EVENT_CUSTOM_A)
        {
            // Player used object, remove despawn aura and disable fail timer
            DisableTimer(MUGLASH_FAIL);
            m_creature->RemoveAurasDueToSpell(SPELL_MUGLASH_WAITING);
            // Start Waves
            m_uiEventId = 2;
            ResetTimer(MUGLASH_EVENT, 2000);
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        // Ignore mmap so they can get out of water without strange movement
        summoned->SetIgnoreMMAP(true);
        summoned->GetMotionMaster()->MovePath(1);

        switch (summoned->GetEntry())
        {
            case NPC_WRATH_RAZORTAIL:
            case NPC_WRATH_RIDER:
            case NPC_WRATH_SORCERESS:
                ++m_uiWaveOneAlive;
                break;
            case NPC_WRATH_MYRMIDON:
            case NPC_WRATH_SEAWITCH:
            case NPC_WRATH_PRIESTESS:
                ++m_uiWaveTwoAlive;
                break;
        }
    }

    void SummonedCreatureJustDied(Creature* summoned) override
    {
        switch (summoned->GetEntry())
        {
            // First Wave
            case NPC_WRATH_RAZORTAIL:
            case NPC_WRATH_RIDER:
            case NPC_WRATH_SORCERESS:
                --m_uiWaveOneAlive;
                // Continue Event if all are dead and spawn 2nd wave already
                if (m_uiWaveOneAlive == 0)
                {
                    for (auto& i : secondnagaLocations)
                        m_creature->SummonCreature(i.uiEntry, i.fX, i.fY, i.fZ, 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 60000, true, true);
                    m_uiEventId = 3;
                    ResetTimer(MUGLASH_EVENT, 5000);
                }
                break;
            case NPC_WRATH_MYRMIDON:
            case NPC_WRATH_SEAWITCH:
            case NPC_WRATH_PRIESTESS:
                --m_uiWaveTwoAlive;
                // If 2nd Wave is Dead go to next waypoint and spawn Vorsha
                if (m_uiWaveTwoAlive == 0)
                {
                    m_creature->SummonCreature(NPC_VORSHA, 3630.2092f, 1190.3536f, -16.624332f, 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 60000, true, false);
                    SetEscortPaused(false); // go to next waypoint
                }
                break;
            case NPC_VORSHA:
                m_uiEventId = 7;
                ResetTimer(MUGLASH_EVENT, 2000);
                break;
        }
    }

    // Failed cause players didnt use Naga Brazier object
    void DoFailEscort()
    {
        if (Player* player = GetPlayerForEscort())
        {
            DoBroadcastText(SAY_MUGLASH_FAIL, m_creature, player);
            m_creature->HandleEmote(EMOTE_ONESHOT_TALK);
        }
        // Make Object not interactable when it quest failed
        if (GameObject* go = GetClosestGameObjectWithEntry(m_creature, GO_NAGA_BRAZIER, INTERACTION_DISTANCE * 2))
        {
            go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
        }
    }

    // When Player activates object start Event
    void DoEvent()
    {
        switch (m_uiEventId)
        {
            case 0:
                // Muglash reached Brazier object, start fail timer in case no player activates the object
                if (Player* player = GetPlayerForEscort())
                {
                    DoBroadcastText(SAY_MUGLASH_BRAZIER, m_creature, player);
                    m_creature->HandleEmote(EMOTE_ONESHOT_TALK);
                }
                // Let Escort fail after 5 minutes if players dont use Naga Brazier object
                ResetTimer(MUGLASH_FAIL, 300000);
                ++m_uiEventId;
                ResetTimer(MUGLASH_EVENT, 2000);
                break;
            case 1:
                // Make Naga Brazier object usable
                if (GameObject* go = GetClosestGameObjectWithEntry(m_creature, GO_NAGA_BRAZIER, INTERACTION_DISTANCE * 2))
                {
                    go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
                }
                DoCastSpellIfCan(m_creature, SPELL_MUGLASH_WAITING);
                break;
            case 2:
                // First Wave
                if (Player* player = GetPlayerForEscort())
                    DoBroadcastText(SAY_MUGLASH_EVENT_01, m_creature, player);
                // Summon first wave of adds
                for (auto& i : nagaLocations)
                    m_creature->SummonCreature(i.uiEntry, i.fX, i.fY, i.fZ, 0.0f, TEMPSPAWN_TIMED_OOC_DESPAWN, 60000, true, true);
                SetEscortPaused(false); // go to next waypoint
                break;
            case 3:
                // First Wave is Dead rest now
                DoBroadcastText(SAY_MUGLASH_EVENT_02, m_creature);
                m_creature->SetStandState(UNIT_STAND_STATE_SIT);
                ++m_uiEventId;
                ResetTimer(MUGLASH_EVENT, 9000);
                break;
            case 4:
                // 2nd wave comes to muglash
                m_creature->SetStandState(UNIT_STAND_STATE_SIT);
                ++m_uiEventId;
                break;
            case 5:
                // 2nd wave is dead
                DoBroadcastText(SAY_MUGLASH_EVENT_03, m_creature);
                ++m_uiEventId;
                ResetTimer(MUGLASH_EVENT, 3000);
                break;
            case 6:
                m_creature->HandleEmote(EMOTE_ONESHOT_POINT);
                DoBroadcastText(SAY_MUGLASH_EVENT_04, m_creature);
                break;
            case 7:
                m_creature->HandleEmote(EMOTE_ONESHOT_CHEER);
                if (Player* player = GetPlayerForEscort())
                    DoBroadcastText(SAY_MUGLASH_SUCCESS, m_creature, player);
                ++m_uiEventId;
                ResetTimer(MUGLASH_EVENT, 8000);
                break;
            case 8:
                if (Player* player = GetPlayerForEscort())
                {
                    // Award quest credit
                    player->RewardPlayerAndGroupAtEventExplored(QUEST_VORSHA, m_creature);
                }
                m_creature->HandleEmote(EMOTE_ONESHOT_BOW);
                if (Player* player = GetPlayerForEscort())
                    DoBroadcastText(SAY_MUGLASH_SUCCESS_02, m_creature, player);
                ++m_uiEventId;
                ResetTimer(MUGLASH_EVENT, 3000);
                break;
            case 9:
                m_creature->HandleEmote(EMOTE_ONESHOT_TALK);
                DoBroadcastText(SAY_MUGLASH_SUCCESS_03, m_creature);
                ++m_uiEventId;
                ResetTimer(MUGLASH_EVENT, 5000);
                break;
            case 10:
                DoBroadcastText(SAY_MUGLASH_SUCCESS_04, m_creature);
                ++m_uiEventId;
                ResetTimer(MUGLASH_EVENT, 5000);
                break;
            case 11:
                // Escort Finished, move random around point before despawning
                m_creature->GetMotionMaster()->MoveRandomAroundPoint(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 5.0f);
                End();
                m_creature->ForcedDespawn(14000); // Despawn after 14
                break;
        }
    }
};

bool QuestAccept_npc_muglash(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_VORSHA)
    {
        if (npc_muglashAI* pEscortAI = dynamic_cast<npc_muglashAI*>(creature->AI()))
        {
            creature->SetFactionTemporary(FACTION_ESCORT_H_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);
            pEscortAI->Start(false, player, quest, false, false, MUGLASH_ESCORT_PATH);
        }
    }
    return true;
}

bool GOUse_go_naga_brazier(Player* player, GameObject* go)
{
    // When player finishs cast inform npc muglash
    if (Creature* creature = GetClosestCreatureWithEntry(go, NPC_MUGLASH, INTERACTION_DISTANCE * 2))
    {
        creature->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, player, creature);
    }
    // Make Object not interactable again and remove flames visual
    go->SetGoState(GO_STATE_READY);
    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
    return true;
}

/*####
# npc_ruul_snowhoof
####*/

enum
{
    QUEST_FREEDOM_TO_RUUL   = 6482,
    NPC_T_AVENGER           = 3925,
    NPC_T_SHAMAN            = 3924,
    NPC_T_PATHFINDER        = 3926,
    SPELL_RUUL_SHAPECHANGE  = 20514,
    SAY_RUUL_COMPLETE       = -1010022
};

static uint32 m_ruulAmbushers[3] = { NPC_T_AVENGER, NPC_T_SHAMAN, NPC_T_PATHFINDER};

static float m_ruulAmbushCoords[2][3] =
        {
                {3425.33f, -595.93f, 178.31f},    // First ambush
                {3245.34f, -506.66f, 150.05f},    // Second ambush
        };

struct npc_ruul_snowhoofAI : public npc_escortAI
{
    npc_ruul_snowhoofAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

    void Reset() override
    {
        DoCastSpellIfCan(m_creature, SPELL_RUUL_SHAPECHANGE, TRIGGERED_OLD_TRIGGERED);
    }

    void DoSpawnAmbush(uint8 index)
    {
        for (auto ambusherEntry : m_ruulAmbushers)
        {
            float fx, fy, fz;
            m_creature->GetRandomPoint(m_ruulAmbushCoords[index][0], m_ruulAmbushCoords[index][1], m_ruulAmbushCoords[index][2], 7.0f, fx, fy, fz);
            if (Creature* ambusher = m_creature->SummonCreature(ambusherEntry, fx, fy, fz, 0, TEMPSPAWN_DEAD_DESPAWN, 60 * IN_MILLISECONDS))
            {
                ambusher->SetWalk(false);
                ambusher->GetMotionMaster()->MovePoint(0, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ());
            }
        }
    }

    void WaypointReached(uint32 pointId) override
    {
        switch (pointId)
        {
            case 14:
                DoSpawnAmbush(0);
                break;
            case 31:
                DoSpawnAmbush(1);
                break;
            case 32:
                m_creature->SetImmuneToNPC(true);
                m_creature->RemoveAurasDueToSpell(SPELL_RUUL_SHAPECHANGE);
                if (Player* player = GetPlayerForEscort())
                {
                    m_creature->SetFacingToObject(player);
                    player->RewardPlayerAndGroupAtEventExplored(QUEST_FREEDOM_TO_RUUL, m_creature);
                }
                break;
            case 33:
                if (Player* player = GetPlayerForEscort())
                {
                    DoScriptText(SAY_RUUL_COMPLETE, m_creature, player);
                    m_creature->SetFacingToObject(player);
                }
                m_creature->HandleEmote(EMOTE_ONESHOT_BOW);
                m_creature->SetWalk(false);
                break;
            case 34:
                DoCastSpellIfCan(m_creature, SPELL_RUUL_SHAPECHANGE);
                break;
            case 36:
                m_creature->SetImmuneToNPC(false);
                m_creature->SetWalk(true);
                m_creature->ForcedDespawn();
        }
    }
};

bool QuestAccept_npc_ruul_snowhoof(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_FREEDOM_TO_RUUL)
    {
        creature->SetFactionTemporary(FACTION_ESCORT_H_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_PLAYER);
        creature->SetStandState(UNIT_STAND_STATE_STAND);

        if (npc_ruul_snowhoofAI* escortAI = dynamic_cast<npc_ruul_snowhoofAI*>(creature->AI()))
            escortAI->Start(false, player, quest);
    }
    return true;
}

UnitAI* GetAI_npc_ruul_snowhoofAI(Creature* pCreature)
{
    return new npc_ruul_snowhoofAI(pCreature);
}

/*####
# npc_torek
####*/

enum
{
    SAY_READY                   = -1000106,
    SAY_MOVE                    = -1000107,
    SAY_PREPARE                 = -1000108,
    SAY_WIN                     = -1000109,
    SAY_END                     = -1000110,

    SPELL_REND                  = 11977,
    SPELL_THUNDERCLAP           = 8078,

    QUEST_TOREK_ASSULT          = 6544,

    NPC_SPLINTERTREE_RAIDER     = 12859,
    NPC_DURIEL                  = 12860,
    NPC_SILVERWING_SENTINEL     = 12896,
    NPC_SILVERWING_WARRIOR      = 12897
};

struct npc_torekAI : public npc_escortAI
{
    npc_torekAI(Creature* pCreature) : npc_escortAI(pCreature) {Reset();}

    uint32 m_uiRend_Timer;
    uint32 m_uiThunderclap_Timer;

    void Reset() override
    {
        m_uiRend_Timer = 5000;
        m_uiThunderclap_Timer = 8000;
    }

    void WaypointReached(uint32 uiPointId) override
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch (uiPointId)
        {
            case 2:
                DoScriptText(SAY_MOVE, m_creature, pPlayer);
                break;
            case 9:
                DoScriptText(SAY_PREPARE, m_creature, pPlayer);
                break;
            case 20:
                // TODO: verify location and creatures amount.
                m_creature->SummonCreature(NPC_DURIEL, 1776.73f, -2049.06f, 109.83f, 1.54f, TEMPSPAWN_TIMED_OOC_DESPAWN, 25000);
                m_creature->SummonCreature(NPC_SILVERWING_SENTINEL, 1774.64f, -2049.41f, 109.83f, 1.40f, TEMPSPAWN_TIMED_OOC_DESPAWN, 25000);
                m_creature->SummonCreature(NPC_SILVERWING_WARRIOR, 1778.73f, -2049.50f, 109.83f, 1.67f, TEMPSPAWN_TIMED_OOC_DESPAWN, 25000);
                break;
            case 21:
                DoScriptText(SAY_WIN, m_creature, pPlayer);
                pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_TOREK_ASSULT, m_creature);
                break;
            case 22:
                DoScriptText(SAY_END, m_creature, pPlayer);
                break;
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        pSummoned->AI()->AttackStart(m_creature);
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        if (m_uiRend_Timer < uiDiff)
        {
            DoCastSpellIfCan(m_creature->GetVictim(), SPELL_REND);
            m_uiRend_Timer = 20000;
        }
        else
            m_uiRend_Timer -= uiDiff;

        if (m_uiThunderclap_Timer < uiDiff)
        {
            DoCastSpellIfCan(m_creature, SPELL_THUNDERCLAP);
            m_uiThunderclap_Timer = 30000;
        }
        else
            m_uiThunderclap_Timer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_torek(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_TOREK_ASSULT)
    {
        // TODO: find companions, make them follow Torek, at any time (possibly done by mangos/database in future?)
        DoScriptText(SAY_READY, pCreature, pPlayer);

        if (npc_torekAI* pEscortAI = dynamic_cast<npc_torekAI*>(pCreature->AI()))
            pEscortAI->Start(true, pPlayer, pQuest);
    }

    return true;
}

UnitAI* GetAI_npc_torek(Creature* pCreature)
{
    return new npc_torekAI(pCreature);
}

/*####
# npc_feero_ironhand
####*/

enum
{
    SAY_QUEST_START             = -1000771,
    SAY_FIRST_AMBUSH_START      = -1000772,
    SAY_FIRST_AMBUSH_END        = -1000773,
    SAY_SECOND_AMBUSH_START     = -1000774,
    SAY_SCOUT_SECOND_AMBUSH     = -1000775,
    SAY_SECOND_AMBUSH_END       = -1000776,
    SAY_FINAL_AMBUSH_START      = -1000777,
    SAY_BALIZAR_FINAL_AMBUSH    = -1000778,
    SAY_FINAL_AMBUSH_ATTACK     = -1000779,
    SAY_QUEST_END               = -1000780,

    QUEST_SUPPLIES_TO_AUBERDINE = 976,

    NPC_DARK_STRAND_ASSASSIN    = 3879,
    NPC_FORSAKEN_SCOUT          = 3893,

    NPC_ALIGAR_THE_TORMENTOR    = 3898,
    NPC_BALIZAR_THE_UMBRAGE     = 3899,
    NPC_CAEDAKAR_THE_VICIOUS    = 3900,
};

/*
 * Notes about the event:
 * The summon coords and event sequence are guesswork based on the comments from wowhead and wowwiki
 */

// Distance, Angle or Offset
static const float aSummonPositions[2][2] =
{
    {30.0f, 1.25f},
    {30.0f, 0.95f}
};

// Hardcoded positions for the last 3 mobs
static const float aEliteSummonPositions[3][4] =
{
    {4243.12f, 108.22f, 38.12f, 3.62f},
    {4240.95f, 114.04f, 38.35f, 3.56f},
    {4235.78f, 118.09f, 38.08f, 4.12f}
};

struct npc_feero_ironhandAI : public npc_escortAI
{
    npc_feero_ironhandAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        Reset();
    }

    uint8 m_uiCreaturesCount;
    bool m_bIsAttacked;

    void Reset() override
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            m_uiCreaturesCount  = 0;
            m_bIsAttacked       = false;
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 15:
                // Prepare the first ambush
                DoScriptText(SAY_FIRST_AMBUSH_START, m_creature);
                for (uint8 i = 0; i < 4; ++i)
                    DoSpawnMob(NPC_DARK_STRAND_ASSASSIN, aSummonPositions[0][0], aSummonPositions[0][1] - M_PI_F / 4 * i);
                break;
            case 21:
                // Prepare the second ambush
                DoScriptText(SAY_SECOND_AMBUSH_START, m_creature);
                for (uint8 i = 0; i < 3; ++i)
                    DoSpawnMob(NPC_FORSAKEN_SCOUT, aSummonPositions[1][0], aSummonPositions[1][1] - M_PI_F / 3 * i);
                break;
            case 30:
                // Final ambush
                DoScriptText(SAY_FINAL_AMBUSH_START, m_creature);
                m_creature->SummonCreature(NPC_BALIZAR_THE_UMBRAGE, aEliteSummonPositions[0][0], aEliteSummonPositions[0][1], aEliteSummonPositions[0][2], aEliteSummonPositions[0][3], TEMPSPAWN_TIMED_OOC_DESPAWN, 20000);
                m_creature->SummonCreature(NPC_ALIGAR_THE_TORMENTOR, aEliteSummonPositions[1][0], aEliteSummonPositions[1][1], aEliteSummonPositions[1][2], aEliteSummonPositions[1][3], TEMPSPAWN_TIMED_OOC_DESPAWN, 20000);
                m_creature->SummonCreature(NPC_CAEDAKAR_THE_VICIOUS, aEliteSummonPositions[2][0], aEliteSummonPositions[2][1], aEliteSummonPositions[2][2], aEliteSummonPositions[2][3], TEMPSPAWN_TIMED_OOC_DESPAWN, 20000);
                break;
            case 31:
                // Complete the quest
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->RewardPlayerAndGroupAtEventExplored(QUEST_SUPPLIES_TO_AUBERDINE, m_creature);
                break;
        }
    }

    void AttackedBy(Unit* pWho) override
    {
        // Yell only at the first attack
        if (!m_bIsAttacked)
        {
            if (((Creature*)pWho)->GetEntry() == NPC_BALIZAR_THE_UMBRAGE)
            {
                DoScriptText(SAY_FINAL_AMBUSH_ATTACK, m_creature);
                m_bIsAttacked = true;
            }
        }
    }

    // Summon mobs at calculated points
    void DoSpawnMob(uint32 uiEntry, float fDistance, float fAngle)
    {
        float fX, fY, fZ;
        m_creature->GetNearPoint(m_creature, fX, fY, fZ, 0, fDistance, fAngle);

        m_creature->SummonCreature(uiEntry, fX, fY, fZ, 0, TEMPSPAWN_TIMED_OOC_DESPAWN, 20000);
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        --m_uiCreaturesCount;

        if (!m_uiCreaturesCount)
        {
            switch (pSummoned->GetEntry())
            {
                case NPC_DARK_STRAND_ASSASSIN:
                    DoScriptText(SAY_FIRST_AMBUSH_END, m_creature);
                    break;
                case NPC_FORSAKEN_SCOUT:
                    DoScriptText(SAY_SECOND_AMBUSH_END, m_creature);
                    break;
                case NPC_ALIGAR_THE_TORMENTOR:
                case NPC_BALIZAR_THE_UMBRAGE:
                case NPC_CAEDAKAR_THE_VICIOUS:
                    DoScriptText(SAY_QUEST_END, m_creature);
                    break;
            }
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_FORSAKEN_SCOUT)
        {
            // Only one of the scouts yells
            if (m_uiCreaturesCount == 1)
                DoScriptText(SAY_SCOUT_SECOND_AMBUSH, pSummoned, m_creature);
        }
        else if (pSummoned->GetEntry() == NPC_BALIZAR_THE_UMBRAGE)
            DoScriptText(SAY_BALIZAR_FINAL_AMBUSH, pSummoned);

        ++m_uiCreaturesCount;
        pSummoned->AI()->AttackStart(m_creature);
    }
};

UnitAI* GetAI_npc_feero_ironhand(Creature* pCreature)
{
    return new npc_feero_ironhandAI(pCreature);
}

bool QuestAccept_npc_feero_ironhand(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_SUPPLIES_TO_AUBERDINE)
    {
        DoScriptText(SAY_QUEST_START, pCreature, pPlayer);
        pCreature->SetFactionTemporary(FACTION_ESCORT_A_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_NPC);

        if (npc_feero_ironhandAI* pEscortAI = dynamic_cast<npc_feero_ironhandAI*>(pCreature->AI()))
            pEscortAI->Start(true, pPlayer, pQuest, true);
    }

    return true;
}

// Destroy Karang's Banner used by Enraged Foulwealds during quest King of the Foulweald (6621)
enum 
{
    GO_KARANGS_BANNER = 178205,
};

struct DestroyKarangsBanner : public SpellScript
{
    void OnSuccessfulFinish(Spell* spell) const override
    {
        if (!spell->GetCaster()->IsCreature())
            return;

        Creature* caster = static_cast<Creature*>(spell->GetCaster());
        if (!caster || !caster->IsAlive())
            return;

        // Tested on Classic, spell should despawn ALL Gameobjects
        GameObjectList bannerList;
        GetGameObjectListWithEntryInGrid(bannerList, caster, GO_KARANGS_BANNER, 50.f);
        for (const auto gobanner : bannerList)
        {
            gobanner->ForcedDespawn();
        }

        // Change Worldstate so NPCs stop respawning
        caster->GetMap()->GetVariableManager().SetVariable(WORLD_STATE_CUSTOM_FOULWEALD, 0);

        // SendAIEvent to handle Despawning after leaving Combat
        caster->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_A, caster, 0, 50);
    }
};

void AddSC_ashenvale()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_muglash";
    pNewScript->GetAI = &GetNewAIInstance<npc_muglashAI>;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_muglash;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_naga_brazier";
    pNewScript->pGOUse = &GOUse_go_naga_brazier;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_ruul_snowhoof";
    pNewScript->GetAI = &GetAI_npc_ruul_snowhoofAI;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_ruul_snowhoof;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_torek";
    pNewScript->GetAI = &GetAI_npc_torek;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_torek;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_feero_ironhand";
    pNewScript->GetAI = &GetAI_npc_feero_ironhand;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_feero_ironhand;
    pNewScript->RegisterSelf();

    RegisterSpellScript<DestroyKarangsBanner>("spell_destroy_karangs_banner");
}
