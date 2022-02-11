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
SDName: Shadowfang_Keep
SD%Complete: 90
SDComment: Scourge Invasion boss is missing, Archmage Arugal timers have to be checked
SDCategory: Shadowfang Keep
EndScriptData

*/

/* ContentData
npc_shadowfang_prisoner
mob_arugal_voidwalker
boss_arugal
npc_deathstalker_vincent
EndContentData */

#include "AI/ScriptDevAI/base/CombatAI.h"
#include "AI/ScriptDevAI/base/escort_ai.h"
#include "AI/ScriptDevAI/base/TimerAI.h"
#include "AI/ScriptDevAI/include/sc_common.h"
#include "shadowfang_keep.h"
#include "Spells/Scripts/SpellScript.h"

/*######
## npc_shadowfang_prisoner
######*/

enum
{
    SAY_FREE_AS             = -1033000,
    SAY_OPEN_DOOR_AS        = -1033001,
    SAY_POST_DOOR_AS        = -1033002,
    EMOTE_VANISH_AS         = -1033014,
    SAY_FREE_AD             = -1033003,
    SAY_OPEN_DOOR_AD        = -1033004,
    SAY_POST1_DOOR_AD       = -1033005,
    SAY_POST2_DOOR_AD       = -1033006,
    EMOTE_UNLOCK_DOOR_AD    = -1033015,

    SPELL_UNLOCK            = 6421,

    GOSSIP_ITEM_DOOR        = -3033000
};

struct npc_shadowfang_prisonerAI : public npc_escortAI
{
    npc_shadowfang_prisonerAI(Creature* creature) : npc_escortAI(creature)
    {
        m_instance = (ScriptedInstance*)creature->GetInstanceData();
        m_npcEntry = creature->GetEntry();
        m_speechStep = 1;
        AddCustomAction(1, true, [&]() { HandleSpeech(); });
    }

    ScriptedInstance* m_instance;
    uint32 m_npcEntry;
    uint8 m_speechStep;

    void HandleSpeech()
    {
        uint32 timer = 0;
        switch (m_speechStep)
        {
            case 1:
            {
                if (m_npcEntry == NPC_ADA)
                    DoScriptText(SAY_POST1_DOOR_AD, m_creature);
                else
                    DoScriptText(SAY_POST_DOOR_AS, m_creature);
                timer = 2 * IN_MILLISECONDS;

                if (m_instance)
                    m_instance->SetData(TYPE_FREE_NPC, DONE);
                break;
            }
            case 2:
            {
                if (m_npcEntry == NPC_ASH)
                {
                    DoCastSpellIfCan(m_creature, SPELL_FIRE);
                    timer = 2500;
                }
                else
                {
                    SetRun();
                    SetEscortPaused(false);
                    DoScriptText(SAY_POST2_DOOR_AD, m_creature);
                    DisableTimer(1);
                }
                break;
            }
            case 3:
            {
                if (m_npcEntry == NPC_ASH)
                {
                    DoScriptText(EMOTE_VANISH_AS, m_creature);
                    m_creature->ForcedDespawn();
                }
                break;
            }
        }
        ++m_speechStep;
        ResetTimer(1, timer);
    }

    void WaypointReached(uint32 point) override
    {
        switch (point)
        {
            case 11:
                if (m_npcEntry == NPC_ASH)
                    DoScriptText(SAY_OPEN_DOOR_AS, m_creature);
                break;
            case 12:
            {
                SetEscortPaused(true);
                GameObject* door = m_instance->GetSingleGameObjectFromStorage(GO_COURTYARD_DOOR);
                m_creature->SetFacingToObject(door);
                if (m_npcEntry == NPC_ASH)
                {
                    DoCastSpellIfCan(m_creature, SPELL_UNLOCK);
                    ResetTimer(1, 5 * IN_MILLISECONDS);
                }
                else
                {
                    DoScriptText(SAY_OPEN_DOOR_AD, m_creature);
                    DoScriptText(EMOTE_UNLOCK_DOOR_AD, m_creature);
                    ResetTimer(1, 6 * IN_MILLISECONDS);
                }
                break;
            }
            case 31:
            {
                if (m_npcEntry == NPC_ADA)
                    m_creature->ForcedDespawn();
                break;
            }
            default:
                break;
        }
    }
};

bool GossipHello_npc_shadowfang_prisoner(Player* player, Creature* creature)
{
    ScriptedInstance* instance = (ScriptedInstance*)creature->GetInstanceData();

    if (instance && instance->GetData(TYPE_FREE_NPC) != DONE)
        player->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_DOOR, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    uint32 textId = creature->GetEntry() == NPC_ADA ? GOSSIP_ADA : GOSSIP_ASH;
    player->SEND_GOSSIP_MENU(textId, creature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_shadowfang_prisoner(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();

        if (npc_shadowfang_prisonerAI* escortAI = dynamic_cast<npc_shadowfang_prisonerAI*>(creature->AI()))
        {
            if (creature->GetEntry() == NPC_ASH)
                DoScriptText(SAY_FREE_AS, creature);
            else
                DoScriptText(SAY_FREE_AD, creature);

            escortAI->Start();
        }
    }
    return true;
}

/*######
## mob_arugal_voidwalker
######*/

enum
{
    NPC_VOIDWALKER      = 4627,
    SPELL_DARK_OFFERING = 7154,
    MAX_VOID_WALKERS    = 4,
};

enum ArugalVoidWalker
{
    VOIDWALKER_DARKOFFERING,
    VOIDWALKER_CHECK_GROUP,
    VOIDWALKER_ACTIONS_MAX
};

// Coordinates for Voidwalker home
static const Waypoint voidwalkerHome = { -146.06f,  2172.84f, 127.953f};  // This is the initial location, in the middle of the room

struct mob_arugal_voidwalkerAI : public CombatAI
{
    mob_arugal_voidwalkerAI(Creature* creature) : CombatAI(creature, VOIDWALKER_ACTIONS_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(VOIDWALKER_DARKOFFERING, 4400, 12500);
        AddCustomAction(VOIDWALKER_CHECK_GROUP, 1000u, [&]() { CheckGroupStatus(); });
        m_position = -1;
        m_leaderGuid = ObjectGuid();
    }

    ScriptedInstance* m_instance;

    int8 m_position;           // 0 - leader, 1 - behind-right, 2 - behind, 3 - behind-left
    ObjectGuid m_leaderGuid;

    void GetAIInformation(ChatHandler& reader) override
    {
        if (!m_leaderGuid)
            reader.PSendSysMessage("Arugal's Voidwalker - no group leader found");
        if (IsLeader())
            reader.PSendSysMessage("Arugal's Voidwalker - is group leader");
        else
        {
            if (m_position)
                reader.PSendSysMessage("Arugal's Voidwalker - is follower with position %u", m_position);
            else
                reader.PSendSysMessage("Arugal's Voidwalker - is neither leader nor follower");
        }
    }

    bool IsLeader()
    {
        return m_position == 0;
    }

    void CheckGroupStatus()
    {
        ResetTimer(VOIDWALKER_CHECK_GROUP, 1 * IN_MILLISECONDS);

        // Do not care about group while fighting
        if (m_creature->IsInCombat())
            return;

        // If a group leader already exists and is alive, do nothing
        if (m_leaderGuid && !IsLeader())
        {
            if (Creature* leader = m_creature->GetMap()->GetCreature(m_leaderGuid))
            {
                // Leader is alive and we are following it, do nothing
                if (leader->IsAlive() && m_creature->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
                    return;
            }
        }

        // If we are leader and already following path, do nothing
        if (IsLeader())
        {
            if (m_creature->GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
                return;
        }

        // For whatever other reason, re-initialise the group
        InitVoidWalkersGroup();
    }

    void InitVoidWalkersGroup()
    {
        SetLeader();

        // Inform other Voidwalkers
        CreatureList walkersList;
        GetCreatureListWithEntryInGrid(walkersList, m_creature, m_creature->GetEntry(), 50.0f);

        uint8 position = 0;
        for (auto& walker : walkersList)
        {
            if (walker->IsAlive() && walker->GetObjectGuid() != m_leaderGuid)
            {
                mob_arugal_voidwalkerAI* walkerAI = static_cast<mob_arugal_voidwalkerAI*>(walker->AI());
                walkerAI->SetFollower(m_creature->GetObjectGuid(), ++position);
            }
        }

        if (position + 1 > MAX_VOID_WALKERS)
            script_error_log("mob_arugal_voidwalker for %s: has more buddies than expected (%u expected, found: %u).", m_creature->GetGuidStr().c_str(), MAX_VOID_WALKERS, position);
    }

    void SetLeader()
    {
        // No leader found or already dead: set ourselves and start following the waypoints
        m_leaderGuid = m_creature->GetObjectGuid();
        m_position = 0;
        m_creature->GetMotionMaster()->Clear();
        m_creature->GetMotionMaster()->MoveWaypoint(0);
    }

    void SetFollower(ObjectGuid leaderGuid, uint8 position)
    {
        m_leaderGuid = leaderGuid;
        m_position = position;

        // Start following the leader
        if (Creature* leader = m_creature->GetMap()->GetCreature(m_leaderGuid))
            m_creature->GetMotionMaster()->MoveFollow(leader, 1.0f, M_PI / 2 * m_position);
        else
            script_error_log("mob_arugal_voidwalker for %s: tried to follow master but master not found in map.", m_creature->GetGuidStr().c_str());
    }

    void ExecuteAction(uint32 action) override
    {
        if (action == VOIDWALKER_DARKOFFERING)
        {
            if (Unit* dearestFriend = DoSelectLowestHpFriendly(10.0f, 290))
            {
                if (DoCastSpellIfCan(dearestFriend, SPELL_DARK_OFFERING) == CAST_OK)
                    ResetCombatAction(action, urand(4, 12) * IN_MILLISECONDS);
            }
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_VOIDWALKER, DONE);
    }
};

/*######
## boss_arugal
######*/

enum
{
    SPELL_VOID_BOLT                 = 7588,
    SPELL_SHADOW_PORT_UPPER_LEDGE   = 7587,
    SPELL_SHADOW_PORT_SPAWN_LEDGE   = 7586,
    SPELL_SHADOW_PORT_STAIRS        = 7136,
    SPELL_ARUGALS_CURSE             = 7621,
    SPELL_THUNDERSHOCK              = 7803,
};

enum ArugalPosition
{
    POSITION_SPAWN_LEDGE = 0,
    POSITION_UPPER_LEDGE = 1,
    POSITION_STAIRS      = 2
};

struct SpawnPoint
{
    float fX, fY, fZ, fO;
};

// Cordinates for voidwalker spawns
static const SpawnPoint VWSpawns[] =
{
    // fX        fY         fZ        fO
    { -155.352f, 2172.780f, 128.448f, 4.679f},
    { -147.059f, 2163.193f, 128.696f, 0.128f},
    { -148.869f, 2180.859f, 128.448f, 1.814f},
    { -140.203f, 2175.263f, 128.448f, 0.373f}
};

// Roughly the height of Fenrus' room,
// Used to tell how he should behave
const float HEIGHT_FENRUS_ROOM      = 140.0f;

enum ArugalActions
{
    ARUGAL_TELEPORT,
    ARUGAL_CURSE,
    ARUGAL_VOID_BOLT,
    ARUGAL_THUNDERSHOCK,
    ARUGAL_ACTIONS_MAX,
};

struct boss_arugalAI : public CombatAI
{
    boss_arugalAI(Creature* creature) : CombatAI(creature, ARUGAL_ACTIONS_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(ARUGAL_TELEPORT, 22000, 26000);
        AddCombatAction(ARUGAL_CURSE, 20000, 30000);
        AddCombatAction(ARUGAL_VOID_BOLT, 1000u);
        AddCombatAction(ARUGAL_THUNDERSHOCK, 1000u);
        SetRangedMode(true, 50.f, TYPE_FULL_CASTER);
        AddMainSpell(SPELL_VOID_BOLT);
        AddMainSpell(SPELL_THUNDERSHOCK);

        // This is only for the instance of Arugal spawned in Fenrus room for little event
        if (creature->GetPositionZ() < HEIGHT_FENRUS_ROOM)
        {
            m_creature->SetImmuneToPlayer(true);
            m_creature->SetImmuneToNPC(true);
            m_creature->SetVisibility(VISIBILITY_OFF);
        }
    }

    ScriptedInstance* m_instance;

    ArugalPosition m_position;
    bool m_attacking, m_eventMode;

    void Reset() override
    {
        m_attacking = true;
        m_position = POSITION_SPAWN_LEDGE;
    }

    void Aggro(Unit* who) override
    {
        DoScriptText(YELL_AGGRO, m_creature);
        DoCastSpellIfCan(who, SPELL_VOID_BOLT);
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->GetTypeId() == TYPEID_PLAYER)
            DoScriptText(YELL_KILLED_PLAYER, m_creature);
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*sender*/, Unit* invoker, uint32 /*miscValue*/) override
    {
        if (eventType == AI_EVENT_CUSTOM_A && invoker->GetTypeId() == TYPEID_UNIT)
        {
            if (invoker->GetEntry() != m_creature->GetEntry())
                return;

            ObjectGuid leaderGuid;

            for (uint8 i = 0; i < MAX_VOID_WALKERS; ++i)
            {
               // No suitable spell was found for this in DBC files, so summoning the hard way
               if (Creature* voidwalker = m_creature->SummonCreature(NPC_VOIDWALKER, VWSpawns[i].fX, VWSpawns[i].fY, VWSpawns[i].fZ, VWSpawns[i].fO, TEMPSPAWN_DEAD_DESPAWN, 1))
               {
                   // Set Voidwalker's home to the middle of the room to avoid evade in an unreachable place
                   voidwalker->SetRespawnCoord(voidwalkerHome.fX, voidwalkerHome.fY, voidwalkerHome.fZ, voidwalker->GetOrientation());

                   if (!i)
                   {
                       leaderGuid = voidwalker->GetObjectGuid();
                       if (auto* voidwalkerAI = dynamic_cast<mob_arugal_voidwalkerAI*>(voidwalker->AI()))
                           voidwalkerAI->SetLeader();
                   }
                   else
                   {
                       if (auto* voidwalkerAI = dynamic_cast<mob_arugal_voidwalkerAI*>(voidwalker->AI()))
                           voidwalkerAI->SetFollower(leaderGuid, i);
                   }
               }
            }
        }
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case ARUGAL_VOID_BOLT:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_VOID_BOLT) == CAST_OK)
                    ResetCombatAction(action, urand(2900, 4800));
                break;
            }
            case ARUGAL_THUNDERSHOCK:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0, uint32(0), SELECT_FLAG_IN_MELEE_RANGE))
                {
                        if (DoCastSpellIfCan(nullptr, SPELL_THUNDERSHOCK) == CAST_OK)
                            ResetCombatAction(action, urand(30, 38) * IN_MILLISECONDS);
                }
                break;
            }
            case ARUGAL_CURSE:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
                {
                    DoScriptText(YELL_WORGEN_CURSE, m_creature);
                    if (DoCastSpellIfCan(target, SPELL_ARUGALS_CURSE) == CAST_OK)
                        ResetCombatAction(action, urand(20, 35) * IN_MILLISECONDS);
                }
                break;
            }
            case ARUGAL_TELEPORT:
            {
                uint8 newPosition = urand(0, 2);

                // Do nothing if we would teleport to our actual place, unless we are on the upper ledge (to prevent tank from breaking aggro if away for too long)
                if (newPosition == m_position && m_position != POSITION_UPPER_LEDGE)
                {
                    ResetCombatAction(action, urand(10, 15) * IN_MILLISECONDS);
                    break;
                }

                uint32 teleportSpellId;
                switch (newPosition)
                {
                    case POSITION_SPAWN_LEDGE:
                    {
                        teleportSpellId = SPELL_SHADOW_PORT_SPAWN_LEDGE;
                        break;
                    }
                    case POSITION_UPPER_LEDGE:
                    {
                        teleportSpellId = SPELL_SHADOW_PORT_UPPER_LEDGE;
                        break;
                    }
                    case POSITION_STAIRS:
                    {
                        teleportSpellId = SPELL_SHADOW_PORT_STAIRS;
                        break;
                    }
                    default:
                        return;
                }

                if (m_creature->IsNonMeleeSpellCasted(false))
                    m_creature->InterruptNonMeleeSpells(false);

                CanCastResult castResult = DoCastSpellIfCan(m_creature, teleportSpellId);
                // Force the cast to be triggered if we are OOM and on the upper ledge (else encounter is stuck)
                if (castResult == CAST_FAIL_POWER && m_position == POSITION_UPPER_LEDGE)
                    castResult = DoCastSpellIfCan(m_creature, teleportSpellId, CAST_TRIGGERED);

                if (castResult == CAST_OK)
                {
                    if (newPosition == POSITION_UPPER_LEDGE)
                    {
                        // Prevent melee whatever and don't stay there for too long
                        SetRangedMode(true, 100.0f, TYPE_NO_MELEE_MODE);
                        ResetCombatAction(action, urand(2000, 2200));
                    }
                    else
                    {
                        SetRangedMode(true, 100.0f, TYPE_FULL_CASTER);
                        ResetCombatAction(action, urand(48, 55) * IN_MILLISECONDS);
                    }
                    m_position = (ArugalPosition)newPosition;
                }

                break;
            }
            default:
                break;
        }
    }
};

/*######
## npc_deathstalker_vincent
######*/

enum
{
    SAY_VINCENT_DIE     = -1033016,

    FACTION_FRIENDLY    = 35
};

struct npc_deathstalker_vincentAI : public ScriptedAI
{
    npc_deathstalker_vincentAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (ScriptedInstance*)creature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_instance;

    void Reset() override
    {
        // Set the proper stand state (standing or dead) depending on the intro event having been played or not
        if (m_instance && m_instance->GetData(TYPE_INTRO) == DONE && !m_creature->GetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_STAND_STATE))
            m_creature->SetStandState(UNIT_STAND_STATE_DEAD);

        if (m_instance && (m_instance->GetData(TYPE_INTRO) == NOT_STARTED || m_instance->GetData(TYPE_INTRO) == IN_PROGRESS))
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);
    }

    void DamageTaken(Unit* /*dealer*/, uint32& damage, DamageEffectType /*damagetype*/, SpellEntry const* /*spellInfo*/) override
    {
        if (m_creature->getStandState())
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);

        if (damage >= m_creature->GetHealth())
        {
            damage = std::min(damage, m_creature->GetHealth() - 1);
            m_creature->SetFactionTemporary(FACTION_FRIENDLY, TEMPFACTION_NONE);
            EnterEvadeMode();
            DoScriptText(SAY_VINCENT_DIE, m_creature);
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        if (m_creature->IsInCombat() && m_creature->GetFaction() == FACTION_FRIENDLY)
            EnterEvadeMode();

        ScriptedAI::UpdateAI(diff);
    }
};

struct ForsakenSkill : public AuraScript
{
    void OnPeriodicDummy(Aura* aura) const override
    {
        // Possibly need cast one of them (but
        // 7038 Forsaken Skill: Swords
        // 7039 Forsaken Skill: Axes
        // 7040 Forsaken Skill: Daggers
        // 7041 Forsaken Skill: Maces
        // 7042 Forsaken Skill: Staves
        // 7043 Forsaken Skill: Bows
        // 7044 Forsaken Skill: Guns
        // 7045 Forsaken Skill: 2H Axes
        // 7046 Forsaken Skill: 2H Maces
        // 7047 Forsaken Skill: 2H Swords
        // 7048 Forsaken Skill: Defense
        // 7049 Forsaken Skill: Fire
        // 7050 Forsaken Skill: Frost
        // 7051 Forsaken Skill: Holy
        // 7053 Forsaken Skill: Shadow
        static uint32 forsakenSpells[] = { 7038,7039,7040,7041,7042,7043,7044,7045,7046,7047,7048,7049,7050,7051,7053 };
        if (urand(0, 99) == 0)
            aura->GetTarget()->CastSpell(nullptr, forsakenSpells[urand(0, 14)], TRIGGERED_OLD_TRIGGERED);
    }
};

void AddSC_shadowfang_keep()
{
    Script* newScript = new Script;
    newScript->Name = "npc_shadowfang_prisoner";
    newScript->pGossipHello =  &GossipHello_npc_shadowfang_prisoner;
    newScript->pGossipSelect = &GossipSelect_npc_shadowfang_prisoner;
    newScript->GetAI = &GetNewAIInstance<npc_shadowfang_prisonerAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "mob_arugal_voidwalker";
    newScript->GetAI = &GetNewAIInstance<mob_arugal_voidwalkerAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "boss_arugal";
    newScript->GetAI = &GetNewAIInstance<boss_arugalAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "npc_deathstalker_vincent";
    newScript->GetAI = &GetNewAIInstance<npc_deathstalker_vincentAI>;
    newScript->RegisterSelf();

    RegisterSpellScript<ForsakenSkill>("spell_forsaken_skill");
}
