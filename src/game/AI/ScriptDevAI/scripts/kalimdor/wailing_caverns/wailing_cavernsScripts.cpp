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
SDName: wailing_caverns
SD%Complete: 90
SDComment: Missing vipers emerge effect, Naralex doesn't fly at exit(Core issue)
SDCategory: Wailing Caverns
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "wailing_caverns.h"
#include "AI/ScriptDevAI/base/escort_ai.h"

enum
{
    SAY_FIRST_CORNER        = 1256,
    SAY_CONTINUE            = 1257,
    SAY_CIRCLE_BANISH       = 1258,
    SAY_PURIFIED            = 1259,
    SAY_NARALEX_CHAMBER     = 1263,
    SAY_BEGIN_RITUAL        = 1264,
    SAY_MUTANUS             = 1276,
    SAY_NARALEX_AWAKE       = 1271,
    SAY_AWAKE               = 2101,
    SAY_NARALEX_THANKYOU    = 1272,
    SAY_FAREWELL            = 2103,
    SAY_AGGRO_1             = 1273,                     // Random between the first 2
    SAY_AGGRO_2             = 1277,
    SAY_AGGRO_3             = 1274,                     // During the awakening ritual

    EMOTE_RITUAL_BEGIN      = 1265,
    EMOTE_NARALEX_AWAKE     = 1268,
    EMOTE_BREAK_THROUGH     = 1269,
    EMOTE_VISION            = 1270,

    GOSSIP_ITEM_BEGIN       = -3043000,
    TEXT_ID_BEGIN           = 699,
    TEXT_ID_DISCIPLE        = 698,

    SPELL_MARK              = 5232,                         // Buff before the fight. To be removed after 4.0.3
    SPELL_SLEEP             = 1090,
    SPELL_POTION            = 8141,
    SPELL_CLEANSING         = 6270,
    SPELL_AWAKENING         = 6271,
    SPELL_SHAPESHIFT        = 8153,

    NPC_DEVIATE_RAPTOR      = 3636,                         // 2 of them at the first stop
    NPC_DEVIATE_VIPER       = 5755,                         // 3 RND at the circle
    NPC_DEVIATE_ADDER       = 5048,                         // 3 RND at the circle
    NPC_DEVIATE_MOCCASIN    = 5762,                         // 6 of them at Naralex chamber
    NPC_NIGHTMARE_ECTOPLASM = 5763,                         // 10 of them at Naralex chamber
    NPC_MUTANUS             = 3654,

    PATH_ID_NARALEX         = 3678,                      // Sniffed Waypoints for Escort Event
};

struct firstWaveLocations
{
    uint32 uiEntry;
    float fX, fY, fZ;
    uint32 uiPathId;
};

// Raptor spawns
static const firstWaveLocations raptorLocations[2] =
{
    { NPC_DEVIATE_RAPTOR , -67.44779f, 214.5348f, -93.42037f, 1 },
    { NPC_DEVIATE_RAPTOR , -67.85276f, 203.7873f, -93.57328f, 2 }
};

static const float circleLocations[3][4] =
{
    { -50.1237f, 274.7166f, -92.7608f, 3.0368f},
    { -60.2538f, 273.0981f, -92.7608f, 0.4014f},
    { -57.5452f, 280.2068f, -92.7608f, 5.0789f}
};

struct secondWaveLocations
{
    uint32 uiEntry;
    float fX, fY, fZ;
    uint32 uiPathId;
};

// Todo: have more then 3 spawn points, but always spawn 3
static const secondWaveLocations moccasinLocations[3] =
{
    { NPC_DEVIATE_MOCCASIN, 171.39545f, 213.76605f, -105.50746f, 1},
    { NPC_DEVIATE_MOCCASIN, 156.72229f, 189.91829f, -107.48995f, 2},
    { NPC_DEVIATE_MOCCASIN, 121.39977f, 166.31746f, -105.54061f, 3}
};

struct thirdWaveLocations
{
    uint32 uiEntry;
    float fX, fY, fZ;
    uint32 uiPathId;
};
// Todo: have more then 7 spawn points, but always spawn 7
static const thirdWaveLocations ectoplasmLocations[7] =
{
    { NPC_NIGHTMARE_ECTOPLASM, 162.06705f, 218.71494f, -105.36240f, 1},
    { NPC_NIGHTMARE_ECTOPLASM, 115.55489f, 168.22847f, -105.68655f, 2},
    { NPC_NIGHTMARE_ECTOPLASM, 82.065025f, 280.37723f, -103.29671f, 3},
    { NPC_NIGHTMARE_ECTOPLASM, 144.84305f, 278.07928f, -104.57445f, 4},
    { NPC_NIGHTMARE_ECTOPLASM, 155.84459f, 186.68817f, -107.08412f, 5},
    { NPC_NIGHTMARE_ECTOPLASM, 145.35356f, 219.34600f, -102.98572f, 6},
    { NPC_NIGHTMARE_ECTOPLASM, 164.62735f, 274.12335f, -107.29780f, 7}
};

float  mutanosLocations[4] = { 150.94276f, 262.79715f, -103.90348f, 1 };

struct npc_disciple_of_naralexAI : public npc_escortAI
{
    npc_disciple_of_naralexAI(Creature* creature) : npc_escortAI(creature), m_instance(static_cast<instance_wailing_caverns*>(creature->GetInstanceData()))
    {
        Reset();
    }

    instance_wailing_caverns* m_instance;

    uint32 m_uiEventTimer;
    uint32 m_uiSleepTimer;
    uint32 m_uiPotionTimer;
    uint32 m_uiCleansingTimer;
    uint8 m_uiSummonedAlive;
    bool m_bIsFirstHit;

    uint32 m_uiPoint;
    uint8 m_uiSubeventPhase;

    void Reset() override
    {
        m_uiSleepTimer      = 5000;
        m_uiPotionTimer     = 5000;
        m_uiCleansingTimer  = 0;
        m_bIsFirstHit       = false;                        // Used to trigger the combat yell; reset at every evade

        // Don't reset mob counter during the event
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            m_uiEventTimer = 0;

            m_uiPoint = 0;
            m_uiSubeventPhase = 0;

            m_uiSummonedAlive = 0;
        }
    }

    void JustRespawned() override
    {
        npc_escortAI::JustRespawned();

        // Reset event if can
        if (m_instance && m_instance->GetData(TYPE_DISCIPLE) != DONE)
            m_instance->SetData(TYPE_DISCIPLE, FAIL);
    }

    void AttackedBy(Unit* pAttacker) override
    {
        if (!m_bIsFirstHit)
        {
            if (pAttacker->GetEntry() == NPC_MUTANUS)
                DoBroadcastText(SAY_MUTANUS, m_creature, pAttacker);
            // Check if already in ritual
            else if (m_uiPoint >= 30)
                DoBroadcastText(SAY_AGGRO_3, m_creature, pAttacker);
            else
                // Aggro 1 should be in 90% of the cases
                DoBroadcastText(roll_chance_i(90) ? SAY_AGGRO_1 : SAY_AGGRO_2, m_creature, pAttacker);

            m_bIsFirstHit = true;
        }
    }

    // Overwrite the evade function, to change the combat stop function (keep casting some spells)
    void EnterEvadeMode() override
    {
        // Do not stop casting at these points
        if (m_uiPoint == 30)
        {
            m_creature->SetLootRecipient(nullptr);
            m_creature->CombatStop(false);

            if (m_creature->IsAlive())
                m_creature->GetMotionMaster()->MovementExpired(true);

            Reset();

            // Remove running
            m_creature->SetWalk(true);
        }
        else
            npc_escortAI::EnterEvadeMode();
    }

    void JustStartedEscort() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_DISCIPLE, IN_PROGRESS);
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 12:
                DoBroadcastText(SAY_FIRST_CORNER, m_creature);
                m_uiSubeventPhase = 0;
                m_uiEventTimer = 2000;
                m_uiPoint = uiPointId;
                SetEscortPaused(true);
                break;
            case 30:
                m_uiSubeventPhase = 0;
                m_uiEventTimer = 1000;
                m_uiPoint = uiPointId;
                SetEscortPaused(true);
                break;
            case 70:                
                m_uiEventTimer = 1000;
                m_uiSubeventPhase = 0;
                m_uiPoint = uiPointId;
                SetEscortPaused(true);
                break;
        }
    }

    void JustSummoned(Creature* /*pSummoned*/) override
    {
        ++m_uiSummonedAlive;
    }

    void SummonedCreatureJustDied(Creature* /*pSummoned*/) override
    {
        if (m_uiSummonedAlive == 0)
            return;                                         // Actually if this happens, something went wrong before

        --m_uiSummonedAlive;

        // Continue Event if all are dead and we are in a stopped subevent
        if (m_uiSummonedAlive == 0 && m_uiEventTimer == 0)
            m_uiEventTimer = 2000;
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (m_uiEventTimer)
        {
            if (m_uiEventTimer <= uiDiff)
            {
                switch (m_uiPoint)
                {
                    // Corner stop -> raptors
                    case 12:
                        switch (m_uiSubeventPhase)
                        {
                            case 0:
                                // Summon raptors at first stop
                                for (auto& i : raptorLocations)
                                    m_creature->SummonCreature(i.uiEntry, i.fX, i.fY, i.fZ, 0.0f, TEMPSPAWN_DEAD_DESPAWN, 20000, true, true, i.uiPathId);
                                m_uiEventTimer = 0;
                                ++m_uiSubeventPhase;
                                break;
                            case 1:
                                // After the summoned mobs are killed continue                                
                                SetEscortPaused(false);
                                m_uiEventTimer = 0;
                                break;
                        }
                        break;
                    // Circle stop -> vipers
                    case 30:
                        switch (m_uiSubeventPhase)
                        {
                            case 0:
                                DoBroadcastText(SAY_CIRCLE_BANISH, m_creature);
                                DoCastSpellIfCan(m_creature, SPELL_CLEANSING);
                                m_uiEventTimer = 20000;
                                ++m_uiSubeventPhase;
                                break;
                            case 1:
                                // Summon vipers at the first circle
                                for (uint8 i = 0; i < 3; ++i)
                                    m_creature->SummonCreature(urand(0, 1) ? NPC_DEVIATE_ADDER : NPC_DEVIATE_VIPER, circleLocations[i][0], circleLocations[i][1], circleLocations[i][2], circleLocations[i][3], TEMPSPAWN_TIMED_OOC_DESPAWN, 20000, true);
                                m_uiEventTimer = 0;
                                ++m_uiSubeventPhase;
                                break;
                            case 2:
                                // Wait for casting to be complete - TODO, if spawned creatures are dead and cast finished we continue without any waiting
                                ++m_uiSubeventPhase;
                                m_uiEventTimer = 10000;
                                break;
                            case 3:
                                m_creature->HandleEmote(EMOTE_ONESHOT_TALK);
                                DoBroadcastText(SAY_PURIFIED, m_creature);
                                m_uiEventTimer = 0;
                                ++m_uiPoint;             // Increment this in order to avoid the special case evade
                                SetEscortPaused(false);
                                break;
                        }
                        break;
                    // Chamber stop -> ritual and final boss
                    case 70:
                        switch (m_uiSubeventPhase)
                        {
                            case 0:
                                DoBroadcastText(SAY_BEGIN_RITUAL, m_creature);
                                m_creature->HandleEmote(EMOTE_ONESHOT_BEG);
                                SetCombatScriptStatus(true);
                                SetCombatMovement(false);
                                m_uiEventTimer = 5000;
                                ++m_uiSubeventPhase;
                                break;
                            case 1:
                                DoCastSpellIfCan(m_creature, SPELL_AWAKENING);
                                DoBroadcastText(EMOTE_RITUAL_BEGIN, m_creature);
                                m_uiEventTimer = 3000;
                                ++m_uiSubeventPhase;
                                break;
                            case 2:
                                // First set of mobs
                                for (auto& i : moccasinLocations)
                                    m_creature->SummonCreature(i.uiEntry, i.fX, i.fY, i.fZ, 0.0f, TEMPSPAWN_DEAD_DESPAWN, 20000, false, false, i.uiPathId);
                                m_uiEventTimer = 5000;
                                ++m_uiSubeventPhase;
                                break;
                            case 3:
                                if (Creature* pNaralex = m_instance->GetSingleCreatureFromStorage(NPC_NARALEX))
                                    DoBroadcastText(EMOTE_NARALEX_AWAKE, pNaralex);
                                m_uiEventTimer = 0;
                                ++m_uiSubeventPhase;
                                break;
                            case 4:
                                // Second set of mobs
                                for (auto& i : ectoplasmLocations)
                                    m_creature->SummonCreature(i.uiEntry, i.fX, i.fY, i.fZ, 0.0f, TEMPSPAWN_DEAD_DESPAWN, 20000, true, false, i.uiPathId);
                                m_uiEventTimer = 20000;
                                ++m_uiSubeventPhase;
                                break;
                            case 5:
                                // Advance only when all mobs are dead
                                if (Creature* pNaralex = m_instance->GetSingleCreatureFromStorage(NPC_NARALEX))
                                    DoBroadcastText(EMOTE_BREAK_THROUGH, pNaralex);
                                ++m_uiSubeventPhase;
                                m_uiEventTimer = 0;
                                break;
                            case 6:
                                // Mutanus
                                if (Creature* pNaralex = m_instance->GetSingleCreatureFromStorage(NPC_NARALEX))
                                    DoBroadcastText(EMOTE_VISION, pNaralex);

                                m_creature->SummonCreature(NPC_MUTANUS, mutanosLocations[0], mutanosLocations[1], mutanosLocations[2], 0.0f, TEMPSPAWN_DEAD_DESPAWN, 20000, true, false, mutanosLocations[3]);
                                m_uiEventTimer = 0;
                                ++m_uiSubeventPhase;
                                break;
                            case 7:
                                // Awaken Naralex after mutanus is defeated
                                if (Creature* pNaralex = m_instance->GetSingleCreatureFromStorage(NPC_NARALEX))
                                {
                                    pNaralex->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoBroadcastText(SAY_NARALEX_AWAKE, pNaralex);
                                }
                                m_instance->SetData(TYPE_DISCIPLE, DONE);
                                ++m_uiSubeventPhase;
                                m_uiEventTimer = 5000;
                                break;
                            case 8:
                                m_creature->InterruptNonMeleeSpells(false, SPELL_AWAKENING);
                                m_creature->RemoveAurasDueToSpell(SPELL_AWAKENING);
                                m_creature->HandleEmote(EMOTE_ONESHOT_POINT);
                                DoBroadcastText(SAY_AWAKE, m_creature);
                                m_uiEventTimer = 1000;
                                ++m_uiSubeventPhase;
                                break;
                            case 9:
                                if (Creature* pNaralex = m_instance->GetSingleCreatureFromStorage(NPC_NARALEX))    
                                { 
                                    pNaralex->HandleEmote(EMOTE_ONESHOT_BOW);
                                    DoBroadcastText(SAY_NARALEX_THANKYOU, pNaralex);  
                                }
                                m_uiEventTimer = 7000;
                                ++m_uiSubeventPhase;
                                break;
                            case 10:
                                // Say Farewell
                                if (Creature* pNaralex = m_instance->GetSingleCreatureFromStorage(NPC_NARALEX))
                                {
                                    pNaralex->HandleEmote(EMOTE_ONESHOT_TALK);
                                    DoBroadcastText(SAY_FAREWELL, pNaralex);                                    
                                }
                                m_uiEventTimer = 3000;
                                ++m_uiSubeventPhase;
                                break;
                            case 11:
                                // Shapeshift into a bird
                                if (Creature* pNaralex = m_instance->GetSingleCreatureFromStorage(NPC_NARALEX))                                
                                    pNaralex->CastSpell(pNaralex, SPELL_SHAPESHIFT, TRIGGERED_NONE);                                
                                DoCastSpellIfCan(m_creature, SPELL_SHAPESHIFT);
                                m_uiEventTimer = 8000;
                                ++m_uiSubeventPhase;
                                break;
                            case 12:
                                SetEscortPaused(false);
                                m_creature->SetLevitate(true);
                                m_creature->SetHover(true);
                                SetRun();
                                // Send them flying somewhere outside of the room
                                if (Creature* pNaralex = m_instance->GetSingleCreatureFromStorage(NPC_NARALEX))
                                {
                                    // Set to flying
                                    pNaralex->SetLevitate(true);
                                    pNaralex->SetHover(true);
                                    pNaralex->SetWalk(false);

                                    // Set following
                                    pNaralex->GetMotionMaster()->MoveFollow(m_creature, 5.0f, 0);
                                }
                                m_uiEventTimer = 30000;
                                break;
                            case 13:
                                if (Creature* pNaralex = m_instance->GetSingleCreatureFromStorage(NPC_NARALEX))
                                    pNaralex->ForcedDespawn(1000); // Despawn after some time
                                m_creature->ForcedDespawn(1000);
                                m_instance->DespawnAll(); // whole instance despawns
                                break;
                        }
                        break;
                }
            }
            else
                m_uiEventTimer -= uiDiff;
        }

        if (m_uiPotionTimer < uiDiff)
        {
            // if health lower than 80%, cast heal
            if (m_creature->GetHealthPercent() < 80.0f)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_POTION) == CAST_OK)
                    m_uiPotionTimer = 45000;
            }
            else
                m_uiPotionTimer = 5000;
        }
        else
            m_uiPotionTimer -= uiDiff;

        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        if (m_uiSleepTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_SLEEP) == CAST_OK)
                    m_uiSleepTimer = 30000;
            }
        }
        else
            m_uiSleepTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

bool GossipHello_npc_disciple_of_naralex(Player* player, Creature* creature)
{
    ScriptedInstance* m_pInstance = (ScriptedInstance*)creature->GetInstanceData();

    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetObjectGuid());

    // Buff the players
    creature->CastSpell(player, SPELL_MARK, TRIGGERED_NONE);
    
    if (m_pInstance && m_pInstance->GetData(TYPE_DISCIPLE) == SPECIAL || player->IsGameMaster())
    {
        player->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_BEGIN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(TEXT_ID_BEGIN, creature->GetObjectGuid());
    }
    else
        player->SEND_GOSSIP_MENU(TEXT_ID_DISCIPLE, creature->GetObjectGuid());
 
    return true;
}

bool GossipSelect_npc_disciple_of_naralex(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
{
    ScriptedInstance* m_pInstance = (ScriptedInstance*)creature->GetInstanceData();

    if (!m_pInstance)
        return false;

    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        if (npc_disciple_of_naralexAI* escortAI = dynamic_cast<npc_disciple_of_naralexAI*>(creature->AI()))
        {
            escortAI->Start(false, player, nullptr, false, false, PATH_ID_NARALEX);               // Note: after 4.0.3 set him run = true
            creature->SetFactionTemporary(FACTION_ESCORT_N_NEUTRAL_ACTIVE, TEMPFACTION_RESTORE_RESPAWN);
        }
        player->CLOSE_GOSSIP_MENU();
    }
    return true;
}

void AddSC_wailing_caverns()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_disciple_of_naralex";
    pNewScript->GetAI = &GetNewAIInstance<npc_disciple_of_naralexAI>;
    pNewScript->pGossipHello =  &GossipHello_npc_disciple_of_naralex;
    pNewScript->pGossipSelect = &GossipSelect_npc_disciple_of_naralex;
    pNewScript->RegisterSelf();
}
