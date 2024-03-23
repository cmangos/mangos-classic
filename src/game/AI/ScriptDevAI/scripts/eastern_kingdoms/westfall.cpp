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
SDName: Westfall
SD%Complete: 90
SDComment: Quest support: 155, 1651
SDCategory: Westfall
EndScriptData
*/

#include "AI/ScriptDevAI/base/escort_ai.h"
#include "AI/ScriptDevAI/base/TimerAI.h"
#include "AI/ScriptDevAI/include/sc_common.h"

/* ContentData
npc_daphne_stilwell
npc_defias_traitor
npc_foreman_klaven_mortwake
EndContentData */

/*######
## npc_daphne_stilwell
######*/

enum
{
    YELL_DEFIAS_START   = -1000412,
    YELL_DAPHNE_START   = -1000413,
    SAY_DS_START        = -1000293,
    SAY_WAVE_DOWN_1     = -1000294,
    SAY_WAVE_DOWN_2     = -1000295,
    SAY_WAVE_DOWN_3     = -1000414,
    SAY_DS_DOWN_3       = -1000296,
    SAY_DS_PROLOGUE     = -1000297,

    SPELL_SHOOT         = 6660,
    QUEST_TOME_VALOR    = 1651,
    NPC_DEFIAS_RAIDER   = 6180,
    EQUIP_ID_RIFLE      = 2511,

    DAPHNE_SHOOT_CD     = 2000
};

enum Wave
{
    FIRST,
    SECOND,
    THIRD
};

float RaiderCoords[15][3] =
{
    {-11428.520f, 1612.757f, 72.241f}, // Spawn1
    {-11422.998f, 1616.106f, 74.153f}, // Spawn2
    {-11430.354f, 1618.334f, 72.632f}, // Spawn3
    {-11423.307f, 1621.033f, 74.224f}, // Spawn4
    {-11427.141f, 1623.220f, 73.168f}, // Spawn5

    {-11453.118f, 1554.380f, 53.100f}, // WP1a
    {-11449.692f, 1554.672f, 53.598f}, // WP2a
    {-11454.533f, 1558.679f, 52.497f}, // WP3a
    {-11449.488f, 1557.817f, 53.443f}, // WP4a
    {-11452.123f, 1559.800f, 52.890f}, // WP5a

    {-11475.067f, 1534.259f, 50.199f}, // WP1b
    {-11470.306f, 1533.835f, 50.267f}, // WP2b
    {-11471.954f, 1539.599f, 50.273f}, // WP3b
    {-11465.560f, 1534.399f, 50.649f}, // WP4b
    {-11467.391f, 1537.989f, 50.726f}  // WP5b
};

struct npc_daphne_stilwellAI : public npc_escortAI
{
    npc_daphne_stilwellAI(Creature* creature) : npc_escortAI(creature)
    {
        AddCustomAction(1, true, [&]() { DoSendWave(); });
        Reset();
    }

    uint32 m_shootTimer;
    uint8 m_wave;

    bool m_introIsDone;

    GuidList m_summonedRaidersGUIDs;

    void Reset() override
    {
        // Don't reset while escorting
        if (HasEscortState(STATE_ESCORT_ESCORTING))
            return;

        m_introIsDone = false;
        m_shootTimer = 0;
    }

    void WaypointReached(uint32 pointId) override
    {
        switch (pointId)
        {
            case 4:
            {
                // Start summoning the three waves (everything else is handled in DoSendWave())
                m_wave = 0;
                ResetTimer(1, 1 * IN_MILLISECONDS);
                m_creature->HandleEmote(EMOTE_STATE_USESTANDING_NOSHEATHE);
                break;
            }
            case 5:
            {
                // Take her gun
                SetEquipmentSlots(false, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE, EQUIP_ID_RIFLE);
                m_creature->SetSheath(SHEATH_STATE_RANGED);
                m_creature->HandleEmote(EMOTE_STATE_STAND);
                break;
            }
            case 7:
            {
                // Stay put and wait for the Defias to come down
                SetRun(false);
                DoScriptText(YELL_DAPHNE_START, m_creature);
                SetCombatMovement(false);
                SetEscortPaused(true);
                break;
            }
            case 8:
            {
                // Cheer after defeating the Defias
                DoScriptText(SAY_DS_DOWN_3, m_creature);
                m_creature->HandleEmote(EMOTE_ONESHOT_CHEER);
                break;
            }
            case 9:
            {
                DoScriptText(SAY_DS_PROLOGUE, m_creature);
                SetCombatMovement(true);
                break;
            }
            case 12:
            {
                SetEquipmentSlots(true);
                m_creature->SetSheath(SHEATH_STATE_UNARMED);
                m_creature->HandleEmote(EMOTE_STATE_USESTANDING);
                break;
            }
            case 13:
            {
                m_creature->HandleEmote(EMOTE_STATE_STAND);
                break;
            }
            case 16:
            {
                if (Player* player = GetPlayerForEscort())
                    player->RewardPlayerAndGroupAtEventExplored(QUEST_TOME_VALOR, m_creature);
                break;
            }
            case 17:
            {
                DoEndEscort();
                break;
            }
            default:
                break;
        }
    }

    void DoSendWave()
    {
        if (m_wave > 2)
            return;

        ++m_wave;

        uint8 firstWpOffset = 5;
        uint8 numberOfAdds = m_wave + 2;

        for (int counter = 0; counter < numberOfAdds; counter++)
        {
            if (Creature* add = m_creature->SummonCreature(NPC_DEFIAS_RAIDER, RaiderCoords[counter][0], RaiderCoords[counter][1], RaiderCoords[counter][2], 0, TEMPSPAWN_TIMED_OOC_DESPAWN, 30000, false, true))
            {
                add->GetMotionMaster()->MovePoint(counter, RaiderCoords[firstWpOffset + counter][0], RaiderCoords[firstWpOffset + counter][1], RaiderCoords[firstWpOffset + counter][2]);
                if (!m_introIsDone)
                {
                    DoScriptText(YELL_DEFIAS_START, add);
                    m_introIsDone = true;
                }
                // Leader of each wave does roar emote on spawn
                if (counter == 0)
                    add->HandleEmote(EMOTE_ONESHOT_ROAR);
            }
        }

        // Keep summoning waves on a timer
        if (m_wave < 3)
            ResetTimer(1, 50 * IN_MILLISECONDS);
        else
            DisableTimer(1);
    }

    void SummonedMovementInform(Creature* summoned, uint32 motionType, uint32 data) override
    {
        if (summoned->GetEntry() != NPC_DEFIAS_RAIDER || motionType != POINT_MOTION_TYPE) // sanity check
            return;

        if (data <= 4)
        {
            uint8 secondWpOffset = 10;
            summoned->GetMotionMaster()->MovePoint(5, RaiderCoords[secondWpOffset + data][0], RaiderCoords[secondWpOffset + data][1], RaiderCoords[secondWpOffset + data][2]);
            return;
        }

        summoned->GetMotionMaster()->MoveIdle();
    }

    void Aggro(Unit* /*who*/) override
    {
        SetCombatMovement(false);
    }

    void JustReachedHome() override
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
            m_creature->SetSheath(SHEATH_STATE_RANGED);
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_DEFIAS_RAIDER)
            m_summonedRaidersGUIDs.push_back(summoned->GetObjectGuid());
    }

    void SummonedCreatureJustDied(Creature* summoned) override
    {
        m_summonedRaidersGUIDs.remove(summoned->GetObjectGuid());

        if (m_summonedRaidersGUIDs.empty())
        {
            if (m_wave == 3)
            {
                SetEscortPaused(false);
                m_wave = 0;
            }
            else
            {
                int32 textId;
                switch (urand(0, 2))
                {
                    case 0:
                        textId = SAY_WAVE_DOWN_1;
                        break;
                    case 1:
                        textId = SAY_WAVE_DOWN_2;
                        break;
                    case 2:
                        textId = SAY_WAVE_DOWN_3;
                        break;
                }
                DoScriptText(textId, m_creature);
            }
        }
    }

    void SummonedCreatureDespawn(Creature* summoned) override // just in case this happens somehow
    {
        if (summoned->IsAlive())
            m_summonedRaidersGUIDs.remove(summoned->GetObjectGuid());
    }

    void JustDied(Unit* killer) override
    {
        m_summonedRaidersGUIDs.clear();

        npc_escortAI::JustDied(killer);
    }

    void DoEndEscort()
    {
        m_creature->ForcedDespawn();
        m_creature->Respawn();
    }

    void UpdateEscortAI(const uint32 diff) override
    {
        UpdateTimers(diff, m_unit->IsInCombat());

        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        if (m_shootTimer < diff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_SHOOT) == CAST_OK)
                m_shootTimer = DAPHNE_SHOOT_CD;
        }
        else
            m_shootTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_daphne_stilwell(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_TOME_VALOR)
    {
        DoScriptText(SAY_DS_START, creature);

        if (npc_daphne_stilwellAI* daphne = dynamic_cast<npc_daphne_stilwellAI*>(creature->AI()))
            daphne->Start(true, player, quest);
    }

    return true;
}

UnitAI* GetAI_npc_daphne_stilwell(Creature* creature)
{
    return new npc_daphne_stilwellAI(creature);
}

/*######
## npc_defias_traitor
######*/

enum
{
    SAY_START                = 9,
    SAY_PROGRESS             = 10,
    SAY_END                  = 11,

    SAY_AGGRO_1              = 487,
    SAY_AGGRO_2              = 485,
    SAY_AGGRO_3              = 489,

    DEFIAS_TRAITOR_PATH      = 467,
    QUEST_DEFIAS_BROTHERHOOD = 155
};

struct npc_defias_traitorAI : public npc_escortAI
{
    npc_defias_traitorAI(Creature* creature) : npc_escortAI(creature)
    {
        SetReactState(REACT_DEFENSIVE);
        Reset();
    }

    void WaypointReached(uint32 pointId) override
    {
        switch (pointId)
        {
            case 65:
                SetRun(false);
                if (Player* player = GetPlayerForEscort())
                    DoBroadcastText(SAY_PROGRESS, m_creature, player);
                break;
            case 94:
                if (Player* player = GetPlayerForEscort())
                {
                    DoBroadcastText(SAY_END, m_creature, player);
                    player->RewardPlayerAndGroupAtEventExplored(QUEST_DEFIAS_BROTHERHOOD, m_creature);
                }
                m_creature->ForcedDespawn(3000, true);
                break;
        }
    }

    void Aggro(Unit* who) override
    {
        switch (urand(0, 3))
        {
            case 0: DoBroadcastText(SAY_AGGRO_1, m_creature, who); break;
            case 1: DoBroadcastText(SAY_AGGRO_2, m_creature, who); break;
            case 2: DoBroadcastText(SAY_AGGRO_3, m_creature, who); break;
        }
    }

    void Reset() override { }
};

bool QuestAccept_npc_defias_traitor(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_DEFIAS_BROTHERHOOD)
    {
        DoBroadcastText(SAY_START, creature, player);

        if (npc_defias_traitorAI* escortAI = dynamic_cast<npc_defias_traitorAI*>(creature->AI()))
            escortAI->Start(true, player, quest, true, false, DEFIAS_TRAITOR_PATH); // instant respawn
    }

    return true;
}

/*######
## npc_foreman_klaven_mortwake
######*/

enum KlavenMortwake
{
    SAY_STEALTH_ALERT_MORTWAKE = 3092
};

struct npc_foreman_klaven_mortwakeAI : public ScriptedAI
{
    npc_foreman_klaven_mortwakeAI(Creature* creature) : ScriptedAI(creature)
    {
        Reset();
    }

    void Reset() override {}

    void OnStealthAlert(Unit* who) override
    {
        DoBroadcastText(SAY_STEALTH_ALERT_MORTWAKE, m_creature, who);
    }
};

void AddSC_westfall()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_daphne_stilwell";
    pNewScript->GetAI = &GetAI_npc_daphne_stilwell;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_daphne_stilwell;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_defias_traitor";
    pNewScript->GetAI = &GetNewAIInstance<npc_defias_traitorAI>;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_defias_traitor;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_foreman_klaven_mortwake";
    pNewScript->GetAI = &GetNewAIInstance<npc_foreman_klaven_mortwakeAI>;
    pNewScript->RegisterSelf();
}
