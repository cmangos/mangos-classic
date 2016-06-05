/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
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

#ifndef MANGOS_CREATURE_EAI_H
#define MANGOS_CREATURE_EAI_H

#include "Common.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Unit.h"

class Player;
class WorldObject;

#define EVENT_UPDATE_TIME               500
#define MAX_ACTIONS                     3
#define MAX_PHASE                       32

enum EventAI_Type
{
    EVENT_T_TIMER_IN_COMBAT         = 0,                    // InitialMin, InitialMax, RepeatMin, RepeatMax
    EVENT_T_TIMER_OOC               = 1,                    // InitialMin, InitialMax, RepeatMin, RepeatMax
    EVENT_T_HP                      = 2,                    // HPMax%, HPMin%, RepeatMin, RepeatMax
    EVENT_T_MANA                    = 3,                    // ManaMax%,ManaMin% RepeatMin, RepeatMax
    EVENT_T_AGGRO                   = 4,                    // NONE
    EVENT_T_KILL                    = 5,                    // RepeatMin, RepeatMax
    EVENT_T_DEATH                   = 6,                    // NONE
    EVENT_T_EVADE                   = 7,                    // NONE
    EVENT_T_SPELLHIT                = 8,                    // SpellID, School, RepeatMin, RepeatMax
    EVENT_T_RANGE                   = 9,                    // MinDist, MaxDist, RepeatMin, RepeatMax
    EVENT_T_OOC_LOS                 = 10,                   // NoHostile, MaxRnage, RepeatMin, RepeatMax
    EVENT_T_SPAWNED                 = 11,                   // Condition, CondValue1
    EVENT_T_TARGET_HP               = 12,                   // HPMax%, HPMin%, RepeatMin, RepeatMax
    EVENT_T_TARGET_CASTING          = 13,                   // RepeatMin, RepeatMax
    EVENT_T_FRIENDLY_HP             = 14,                   // HPDeficit, Radius, RepeatMin, RepeatMax
    EVENT_T_FRIENDLY_IS_CC          = 15,                   // DispelType, Radius, RepeatMin, RepeatMax
    EVENT_T_FRIENDLY_MISSING_BUFF   = 16,                   // SpellId, Radius, RepeatMin, RepeatMax
    EVENT_T_SUMMONED_UNIT           = 17,                   // CreatureId, RepeatMin, RepeatMax
    EVENT_T_TARGET_MANA             = 18,                   // ManaMax%, ManaMin%, RepeatMin, RepeatMax
    EVENT_T_QUEST_ACCEPT            = 19,                   // QuestID
    EVENT_T_QUEST_COMPLETE          = 20,                   //
    EVENT_T_REACHED_HOME            = 21,                   // NONE
    EVENT_T_RECEIVE_EMOTE           = 22,                   // EmoteId, Condition, CondValue1, CondValue2
    EVENT_T_AURA                    = 23,                   // Param1 = SpellID, Param2 = Number of time stacked, Param3/4 Repeat Min/Max
    EVENT_T_TARGET_AURA             = 24,                   // Param1 = SpellID, Param2 = Number of time stacked, Param3/4 Repeat Min/Max
    EVENT_T_SUMMONED_JUST_DIED      = 25,                   // CreatureId, RepeatMin, RepeatMax
    EVENT_T_SUMMONED_JUST_DESPAWN   = 26,                   // CreatureId, RepeatMin, RepeatMax
    EVENT_T_MISSING_AURA            = 27,                   // Param1 = SpellID, Param2 = Number of time stacked expected, Param3/4 Repeat Min/Max
    EVENT_T_TARGET_MISSING_AURA     = 28,                   // Param1 = SpellID, Param2 = Number of time stacked expected, Param3/4 Repeat Min/Max
    EVENT_T_TIMER_GENERIC           = 29,                   // InitialMin, InitialMax, RepeatMin, RepeatMax
    EVENT_T_RECEIVE_AI_EVENT        = 30,                   // AIEventType, Sender-Entry, unused, unused
    EVENT_T_ENERGY                  = 31,                   // EnergyMax%, EnergyMin%, RepeatMin, RepeatMax

    EVENT_T_END,
};

enum EventAI_ActionType
{
    ACTION_T_NONE                       = 0,                // No action
    ACTION_T_TEXT                       = 1,                // TextId1, optionally -TextId2, optionally -TextId3(if -TextId2 exist). If more than just -TextId1 is defined, randomize. Negative values.
    ACTION_T_SET_FACTION                = 2,                // FactionId (or 0 for default)
    ACTION_T_MORPH_TO_ENTRY_OR_MODEL    = 3,                // Creature_template entry(param1) OR ModelId (param2) (or 0 for both to demorph)
    ACTION_T_SOUND                      = 4,                // SoundId
    ACTION_T_EMOTE                      = 5,                // EmoteId
    ACTION_T_RANDOM_SAY                 = 6,                // UNUSED
    ACTION_T_RANDOM_YELL                = 7,                // UNUSED
    ACTION_T_RANDOM_TEXTEMOTE           = 8,                // UNUSED
    ACTION_T_RANDOM_SOUND               = 9,                // SoundId1, SoundId2, SoundId3 (-1 in any field means no output if randomed that field)
    ACTION_T_RANDOM_EMOTE               = 10,               // EmoteId1, EmoteId2, EmoteId3 (-1 in any field means no output if randomed that field)
    ACTION_T_CAST                       = 11,               // SpellId, Target, CastFlags
    ACTION_T_SUMMON                     = 12,               // CreatureID, Target, Duration in ms
    ACTION_T_THREAT_SINGLE_PCT          = 13,               // Threat%, Target
    ACTION_T_THREAT_ALL_PCT             = 14,               // Threat%
    ACTION_T_QUEST_EVENT                = 15,               // QuestID, Target
    ACTION_T_CAST_EVENT                 = 16,               // QuestID, SpellId, Target - must be removed as hack?
    ACTION_T_SET_UNIT_FIELD             = 17,               // Field_Number, Value, Target
    ACTION_T_SET_UNIT_FLAG              = 18,               // Flags (may be more than one field OR'd together), Target
    ACTION_T_REMOVE_UNIT_FLAG           = 19,               // Flags (may be more than one field OR'd together), Target
    ACTION_T_AUTO_ATTACK                = 20,               // AllowAttackState (0 = stop attack, anything else means continue attacking)
    ACTION_T_COMBAT_MOVEMENT            = 21,               // AllowCombatMovement (0 = stop combat based movement, anything else continue attacking)
    ACTION_T_SET_PHASE                  = 22,               // Phase
    ACTION_T_INC_PHASE                  = 23,               // Value (may be negative to decrement phase, should not be 0)
    ACTION_T_EVADE                      = 24,               // No Params
    ACTION_T_FLEE_FOR_ASSIST            = 25,               // No Params
    ACTION_T_QUEST_EVENT_ALL            = 26,               // QuestID, UseThreatList (1 = true, 0 = false)
    ACTION_T_CAST_EVENT_ALL             = 27,               // CreatureId, SpellId
    ACTION_T_REMOVEAURASFROMSPELL       = 28,               // Target, Spellid
    ACTION_T_RANGED_MOVEMENT            = 29,               // Distance, Angle
    ACTION_T_RANDOM_PHASE               = 30,               // PhaseId1, PhaseId2, PhaseId3
    ACTION_T_RANDOM_PHASE_RANGE         = 31,               // PhaseMin, PhaseMax
    ACTION_T_SUMMON_ID                  = 32,               // CreatureId, Target, SpawnId
    ACTION_T_KILLED_MONSTER             = 33,               // CreatureId, Target
    ACTION_T_SET_INST_DATA              = 34,               // Field, Data
    ACTION_T_SET_INST_DATA64            = 35,               // Field, Target
    ACTION_T_UPDATE_TEMPLATE            = 36,               // Entry, Team
    ACTION_T_DIE                        = 37,               // No Params
    ACTION_T_ZONE_COMBAT_PULSE          = 38,               // No Params
    ACTION_T_CALL_FOR_HELP              = 39,               // Radius
    ACTION_T_SET_SHEATH                 = 40,               // Sheath (0-passive,1-melee,2-ranged)
    ACTION_T_FORCE_DESPAWN              = 41,               // Delay (0-instant despawn)
    ACTION_T_SET_INVINCIBILITY_HP_LEVEL = 42,               // MinHpValue, format(0-flat,1-percent from max health)
    ACTION_T_MOUNT_TO_ENTRY_OR_MODEL    = 43,               // Creature_template entry(param1) OR ModelId (param2) (or 0 for both to unmount)
    ACTION_T_CHANCED_TEXT               = 44,               // Chance to display the text, TextId1, optionally TextId2. If more than just -TextId1 is defined, randomize. Negative values.
    ACTION_T_THROW_AI_EVENT             = 45,               // EventType, Radius, unused
    ACTION_T_SET_THROW_MASK             = 46,               // EventTypeMask, unused, unused
    ACTION_T_SET_STAND_STATE            = 47,               // StandState, unused, unused
    ACTION_T_CHANGE_MOVEMENT            = 48,               // MovementType, WanderDistance, unused
    ACTION_T_DYNAMIC_MOVEMENT           = 49,               // EnableDynamicMovement (1 = on; 0 = off)

    ACTION_T_END,
};

enum Target
{
    // Self (m_creature)
    TARGET_T_SELF                           = 0,            // Self cast

    // Hostile targets
    TARGET_T_HOSTILE                        = 1,            // Our current target (ie: highest aggro)
    TARGET_T_HOSTILE_SECOND_AGGRO           = 2,            // Second highest aggro (generaly used for cleaves and some special attacks)
    TARGET_T_HOSTILE_LAST_AGGRO             = 3,            // Dead last on aggro (no idea what this could be used for)
    TARGET_T_HOSTILE_RANDOM                 = 4,            // Just any random target on our threat list
    TARGET_T_HOSTILE_RANDOM_NOT_TOP         = 5,            // Any random target except top threat

    // Invoker targets
    TARGET_T_ACTION_INVOKER                 = 6,            // Unit who caused this Event to occur (only works for EVENT_T_AGGRO, EVENT_T_KILL, EVENT_T_DEATH, EVENT_T_SPELLHIT, EVENT_T_OOC_LOS, EVENT_T_FRIENDLY_HP, EVENT_T_FRIENDLY_IS_CC, EVENT_T_FRIENDLY_MISSING_BUFF, EVENT_T_RECEIVE_EMOTE, EVENT_T_RECEIVE_AI_EVENT)
    TARGET_T_ACTION_INVOKER_OWNER           = 7,            // Unit who is responsible for Event to occur (only works for EVENT_T_AGGRO, EVENT_T_KILL, EVENT_T_DEATH, EVENT_T_SPELLHIT, EVENT_T_OOC_LOS, EVENT_T_FRIENDLY_HP, EVENT_T_FRIENDLY_IS_CC, EVENT_T_FRIENDLY_MISSING_BUFF, EVENT_T_RECEIVE_EMOTE, EVENT_T_RECEIVE_AI_EVENT)
    TARGET_T_EVENT_SENDER                   = 10,           // Unit who sent an AIEvent that was received with EVENT_T_RECEIVE_AI_EVENT

    // Hostile players
    TARGET_T_HOSTILE_RANDOM_PLAYER          = 8,            // Just any random player on our threat list
    TARGET_T_HOSTILE_RANDOM_NOT_TOP_PLAYER  = 9,            // Any random player from threat list except top threat
};

enum EventFlags
{
    EFLAG_REPEATABLE            = 0x01,                     // Event repeats
    EFLAG_RESERVED_1            = 0x02,
    EFLAG_RESERVED_2            = 0x04,
    EFLAG_RESERVED_3            = 0x08,
    EFLAG_RESERVED_4            = 0x10,
    EFLAG_RANDOM_ACTION         = 0x20,                     // Event only execute one from existed actions instead each action.
    EFLAG_RESERVED_6            = 0x40,
    EFLAG_DEBUG_ONLY            = 0x80,                     // Event only occurs in debug build
    // no free bits, uint8 field
};

enum SpawnedEventMode
{
    SPAWNED_EVENT_ALWAY = 0,
    SPAWNED_EVENT_MAP   = 1,
    SPAWNED_EVENT_ZONE  = 2
};

struct CreatureEventAI_Action
{
    EventAI_ActionType type: 16;
    union
    {
        // ACTION_T_TEXT                                    = 1
        struct
        {
            int32 TextId[3];
        } text;
        // ACTION_T_SET_FACTION                             = 2
        struct
        {
            uint32 factionId;                               // faction id or 0 to restore default faction
            uint32 factionFlags;                            // flags will restore default faction at evade and/or respawn
        } set_faction;
        // ACTION_T_MORPH_TO_ENTRY_OR_MODEL                 = 3
        struct
        {
            uint32 creatureId;                              // set one from fields (or 0 for both to demorph)
            uint32 modelId;
        } morph;
        // ACTION_T_SOUND                                   = 4
        struct
        {
            uint32 soundId;
        } sound;
        // ACTION_T_EMOTE                                   = 5
        struct
        {
            uint32 emoteId;
        } emote;
        // ACTION_T_RANDOM_SOUND                            = 9
        struct
        {
            int32 soundId1;                                 // (-1 in any field means no output if randomed that field)
            int32 soundId2;
            int32 soundId3;
        } random_sound;
        // ACTION_T_RANDOM_EMOTE                            = 10
        struct
        {
            int32 emoteId1;                                 // (-1 in any field means no output if randomed that field)
            int32 emoteId2;
            int32 emoteId3;
        } random_emote;
        // ACTION_T_CAST                                    = 11
        struct
        {
            uint32 spellId;
            uint32 target;
            uint32 castFlags;
        } cast;
        // ACTION_T_SUMMON                                  = 12
        struct
        {
            uint32 creatureId;
            uint32 target;
            uint32 duration;
        } summon;
        // ACTION_T_THREAT_SINGLE_PCT                       = 13
        struct
        {
            int32 percent;
            uint32 target;
        } threat_single_pct;
        // ACTION_T_THREAT_ALL_PCT                          = 14
        struct
        {
            int32 percent;
        } threat_all_pct;
        // ACTION_T_QUEST_EVENT                             = 15
        struct
        {
            uint32 questId;
            uint32 target;
        } quest_event;
        // ACTION_T_CAST_EVENT                              = 16
        struct
        {
            uint32 creatureId;
            uint32 spellId;
            uint32 target;
        } cast_event;
        // ACTION_T_SET_UNIT_FIELD                          = 17
        struct
        {
            uint32 field;
            uint32 value;
            uint32 target;
        } set_unit_field;
        // ACTION_T_SET_UNIT_FLAG                           = 18,  // value provided mask bits that will be set
        // ACTION_T_REMOVE_UNIT_FLAG                        = 19,  // value provided mask bits that will be clear
        struct
        {
            uint32 value;
            uint32 target;
        } unit_flag;
        // ACTION_T_AUTO_ATTACK                             = 20
        struct
        {
            uint32 state;                                   // 0 = stop attack, anything else means continue attacking
        } auto_attack;
        // ACTION_T_COMBAT_MOVEMENT                         = 21
        struct
        {
            uint32 state;                                   // 0 = stop combat based movement, anything else continue attacking
            uint32 melee;                                   // if set: at stop send melee combat stop if in combat, use for terminate melee fighting state for switch to ranged
        } combat_movement;
        // ACTION_T_SET_PHASE                               = 22
        struct
        {
            uint32 phase;
        } set_phase;
        // ACTION_T_INC_PHASE                               = 23
        struct
        {
            int32 step;
        } set_inc_phase;
        // ACTION_T_QUEST_EVENT_ALL                         = 26
        struct
        {
            uint32 questId;
            uint32 useThreatList;                           // bool: 1 = true; 0 = false
        } quest_event_all;
        // ACTION_T_CAST_EVENT_ALL                          = 27
        struct
        {
            uint32 creatureId;
            uint32 spellId;
        } cast_event_all;
        // ACTION_T_REMOVEAURASFROMSPELL                    = 28
        struct
        {
            uint32 target;
            uint32 spellId;
        } remove_aura;
        // ACTION_T_RANGED_MOVEMENT                         = 29
        struct
        {
            uint32 distance;
            int32  angle;
        } ranged_movement;
        // ACTION_T_RANDOM_PHASE                            = 30
        struct
        {
            uint32 phase1;
            uint32 phase2;
            uint32 phase3;
        } random_phase;
        // ACTION_T_RANDOM_PHASE_RANGE                      = 31
        struct
        {
            uint32 phaseMin;
            uint32 phaseMax;
        } random_phase_range;
        // ACTION_T_SUMMON_ID                               = 32
        struct
        {
            uint32 creatureId;
            uint32 target;
            uint32 spawnId;
        } summon_id;
        // ACTION_T_KILLED_MONSTER                          = 33
        struct
        {
            uint32 creatureId;
            uint32 target;
        } killed_monster;
        // ACTION_T_SET_INST_DATA                           = 34
        struct
        {
            uint32 field;
            uint32 value;
        } set_inst_data;
        // ACTION_T_SET_INST_DATA64                         = 35
        struct
        {
            uint32 field;
            uint32 target;
        } set_inst_data64;
        // ACTION_T_UPDATE_TEMPLATE                         = 36
        struct
        {
            uint32 creatureId;
            uint32 team;
        } update_template;
        // ACTION_T_CALL_FOR_HELP                           = 39
        struct
        {
            uint32 radius;
        } call_for_help;
        // ACTION_T_SET_SHEATH                              = 40
        struct
        {
            uint32 sheath;
        } set_sheath;
        // ACTION_T_FORCE_DESPAWN                           = 41
        struct
        {
            uint32 msDelay;
        } forced_despawn;
        // ACTION_T_SET_INVINCIBILITY_HP_LEVEL              = 42
        struct
        {
            uint32 hp_level;
            uint32 is_percent;
        } invincibility_hp_level;
        // ACTION_T_MOUNT_TO_ENTRY_OR_MODEL                 = 43
        struct
        {
            uint32 creatureId;                              // set one from fields (or 0 for both to dismount)
            uint32 modelId;
        } mount;
        // ACTION_T_CHANCED_TEXT                            = 44
        struct
        {
            uint32 chance;
            int32 TextId[2];
        } chanced_text;
        // ACTION_T_THROW_AI_EVENT                          = 45
        struct
        {
            uint32 eventType;
            uint32 radius;
            uint32 unused;
        } throwEvent;
        // ACTION_T_SET_THROW_MASK                          = 46
        struct
        {
            uint32 eventTypeMask;
            uint32 unused1;
            uint32 unused2;
        } setThrowMask;
        // ACTION_T_SET_STAND_STATE                         = 47
        struct
        {
            uint32 standState;
            uint32 unused1;
            uint32 unused2;
        } setStandState;
        // ACTION_T_CHANGE_MOVEMENT                         = 48
        struct
        {
            uint32 movementType;
            uint32 wanderDistance;
            uint32 unused1;
        } changeMovement;
        // ACTION_T_DYNAMIC_MOVEMENT                        = 49
        struct
        {
            uint32 state;                                   // bool: 1 = on; 0 = off
            uint32 unused1;
            uint32 unused2;
        } dynamicMovement;
        // RAW
        struct
        {
            uint32 param1;
            uint32 param2;
            uint32 param3;
        } raw;
    };
};

struct CreatureEventAI_Event
{
    uint32 event_id;

    uint32 creature_id;

    uint32 event_inverse_phase_mask;

    EventAI_Type event_type : 16;
    uint8 event_chance : 8;
    uint8 event_flags  : 8;

    union
    {
        // EVENT_T_TIMER_IN_COMBAT                          = 0
        // EVENT_T_TIMER_OOC                                = 1
        // EVENT_T_TIMER_GENERIC                            = 29
        struct
        {
            uint32 initialMin;
            uint32 initialMax;
            uint32 repeatMin;
            uint32 repeatMax;
        } timer;
        // EVENT_T_HP                                       = 2
        // EVENT_T_MANA                                     = 3
        // EVENT_T_TARGET_HP                                = 12
        // EVENT_T_TARGET_MANA                              = 18
        // EVENT_T_ENERGY                                   = 31
        struct
        {
            uint32 percentMax;
            uint32 percentMin;
            uint32 repeatMin;
            uint32 repeatMax;
        } percent_range;
        // EVENT_T_KILL                                     = 5
        struct
        {
            uint32 repeatMin;
            uint32 repeatMax;
        } kill;
        // EVENT_T_SPELLHIT                                 = 8
        struct
        {
            uint32 spellId;
            uint32 schoolMask;                              // -1 (==0xffffffff) is ok value for full mask, or must be more limited mask like (0 < 1) = 1 for normal/physical school
            uint32 repeatMin;
            uint32 repeatMax;
        } spell_hit;
        // EVENT_T_RANGE                                    = 9
        struct
        {
            uint32 minDist;
            uint32 maxDist;
            uint32 repeatMin;
            uint32 repeatMax;
        } range;
        // EVENT_T_OOC_LOS                                  = 10
        struct
        {
            uint32 noHostile;
            uint32 maxRange;
            uint32 repeatMin;
            uint32 repeatMax;
        } ooc_los;
        // EVENT_T_SPAWNED                                  = 11
        struct
        {
            uint32 condition;
            uint32 conditionValue1;
        } spawned;
        // EVENT_T_TARGET_CASTING                           = 13
        struct
        {
            uint32 repeatMin;
            uint32 repeatMax;
        } target_casting;
        // EVENT_T_FRIENDLY_HP                              = 14
        struct
        {
            uint32 hpDeficit;
            uint32 radius;
            uint32 repeatMin;
            uint32 repeatMax;
        } friendly_hp;
        // EVENT_T_FRIENDLY_IS_CC                           = 15
        struct
        {
            uint32 dispelType;                              // unused ?
            uint32 radius;
            uint32 repeatMin;
            uint32 repeatMax;
        } friendly_is_cc;
        // EVENT_T_FRIENDLY_MISSING_BUFF                    = 16
        struct
        {
            uint32 spellId;
            uint32 radius;
            uint32 repeatMin;
            uint32 repeatMax;
        } friendly_buff;
        // EVENT_T_SUMMONED_UNIT                            = 17
        // EVENT_T_SUMMONED_JUST_DIED                       = 25
        // EVENT_T_SUMMONED_JUST_DESPAWN                    = 26
        struct
        {
            uint32 creatureId;
            uint32 repeatMin;
            uint32 repeatMax;
        } summoned;
        // EVENT_T_QUEST_ACCEPT                             = 19
        // EVENT_T_QUEST_COMPLETE                           = 20
        struct
        {
            uint32 questId;
        } quest;
        // EVENT_T_RECEIVE_EMOTE                            = 22
        struct
        {
            uint32 emoteId;
            uint32 condition;
            uint32 conditionValue1;
            uint32 conditionValue2;
        } receive_emote;
        // EVENT_T_AURA                                     = 23
        // EVENT_T_TARGET_AURA                              = 24
        // EVENT_T_MISSING_AURA                             = 27
        // EVENT_T_TARGET_MISSING_AURA                      = 28
        struct
        {
            uint32 spellId;
            uint32 amount;
            uint32 repeatMin;
            uint32 repeatMax;
        } buffed;
        // EVENT_T_RECEIVE_AI_EVENT                         = 30
        struct
        {
            uint32 eventType;                               // See CreatureAI.h enum AIEventType - Receive only events of this type
            uint32 senderEntry;                             // Optional npc from only whom this event can be received
            uint32 unused1;
            uint32 unused2;
        } receiveAIEvent;
        // RAW
        struct
        {
            uint32 param1;
            uint32 param2;
            uint32 param3;
            uint32 param4;
        } raw;
    };

    CreatureEventAI_Action action[MAX_ACTIONS];
};

#define AIEVENT_DEFAULT_THROW_RADIUS    30.0f

// Event_Map
typedef std::vector<CreatureEventAI_Event> CreatureEventAI_Event_Vec;
typedef std::unordered_map<uint32, CreatureEventAI_Event_Vec > CreatureEventAI_Event_Map;

struct CreatureEventAI_Summon
{
    uint32 id;

    float position_x;
    float position_y;
    float position_z;
    float orientation;
    uint32 SpawnTimeSecs;
};

// EventSummon_Map
typedef std::unordered_map<uint32, CreatureEventAI_Summon> CreatureEventAI_Summon_Map;

struct CreatureEventAIHolder
{
    CreatureEventAIHolder(CreatureEventAI_Event p) : Event(p), Time(0), Enabled(true) {}

    CreatureEventAI_Event Event;
    uint32 Time;
    bool Enabled;

    // helper
    bool UpdateRepeatTimer(Creature* creature, uint32 repeatMin, uint32 repeatMax);
};

class MANGOS_DLL_SPEC CreatureEventAI : public CreatureAI
{
    public:
        explicit CreatureEventAI(Creature* c);
        ~CreatureEventAI()
        {
            m_CreatureEventAIList.clear();
        }

        void GetAIInformation(ChatHandler& reader) override;

        void JustRespawned() override;
        void Reset();
        void JustReachedHome() override;
        void EnterCombat(Unit* enemy) override;
        void EnterEvadeMode() override;
        void JustDied(Unit* killer) override;
        void KilledUnit(Unit* victim) override;
        void JustSummoned(Creature* pUnit) override;
        void AttackStart(Unit* who) override;
        void MoveInLineOfSight(Unit* who) override;
        void SpellHit(Unit* pUnit, const SpellEntry* pSpell) override;
        void DamageTaken(Unit* done_by, uint32& damage) override;
        void HealedBy(Unit* healer, uint32& healedAmount) override;
        void UpdateAI(const uint32 diff) override;
        bool IsVisible(Unit*) const override;
        void ReceiveEmote(Player* pPlayer, uint32 text_emote) override;
        void SummonedCreatureJustDied(Creature* unit) override;
        void SummonedCreatureDespawn(Creature* unit) override;
        void ReceiveAIEvent(AIEventType eventType, Creature* pSender, Unit* pInvoker, uint32 miscValue) override;

        static int Permissible(const Creature*);

        bool ProcessEvent(CreatureEventAIHolder& pHolder, Unit* pActionInvoker = nullptr, Creature* pAIEventSender = nullptr);
        void ProcessAction(CreatureEventAI_Action const& action, uint32 rnd, uint32 EventId, Unit* pActionInvoker, Creature* pAIEventSender);
        inline uint32 GetRandActionParam(uint32 rnd, uint32 param1, uint32 param2, uint32 param3);
        inline int32 GetRandActionParam(uint32 rnd, int32 param1, int32 param2, int32 param3);
        /// If the bool& param is true, an error should be reported
        inline Unit* GetTargetByType(uint32 Target, Unit* pActionInvoker, Creature* pAIEventSender, bool& isError, uint32 forSpellId = 0, uint32 selectFlags = 0);

        bool SpawnedEventConditionsCheck(CreatureEventAI_Event const& event);

        Unit* DoSelectLowestHpFriendly(float range, uint32 MinHPDiff);
        void DoFindFriendlyMissingBuff(std::list<Creature*>& _list, float range, uint32 spellid);
        void DoFindFriendlyCC(std::list<Creature*>& _list, float range);

    protected:
        uint32 m_EventUpdateTime;                           // Time between event updates
        uint32 m_EventDiff;                                 // Time between the last event call

        // Variables used by Events themselves
        typedef std::vector<CreatureEventAIHolder> CreatureEventAIList;
        CreatureEventAIList m_CreatureEventAIList;          // Holder for events (stores enabled, time, and eventid)

        uint8  m_Phase;                                     // Current phase, max 32 phases
        bool   m_MeleeEnabled;                              // If we allow melee auto attack
        bool   m_DynamicMovement;                           // Core will control creatures movement if this is enabled
        bool   m_HasOOCLoSEvent;                            // Cache if a OOC-LoS Event exists
        uint32 m_InvinceabilityHpLevel;                     // Minimal health level allowed at damage apply

        uint32 m_throwAIEventMask;                          // Automatically throw AIEvents that are encoded into this mask
        // Note that Step 100 means that AI_EVENT_GOT_FULL_HEALTH was sent
        // Steps 0..2 correspond to AI_EVENT_LOST_SOME_HEALTH(90%), AI_EVENT_LOST_HEALTH(50%), AI_EVENT_CRITICAL_HEALTH(10%)
        uint32 m_throwAIEventStep;                          // Used for damage taken/ received heal
        float m_LastSpellMaxRange;                          // Maximum spell range that was cast during dynamic movement
};

#endif
