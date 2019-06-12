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

/**
 * \addtogroup game
 * @{
 * \file
 */


#ifndef __UNIT_H
#define __UNIT_H

#include "Common.h"
#include "Entities/Object.h"
#include "Server/Opcodes.h"
#include "Spells/SpellAuraDefines.h"
#include "AI/BaseAI/CreatureAI.h"
#include "Globals/SharedDefines.h"
#include "Combat/ThreatManager.h"
#include "Combat/HostileRefManager.h"
#include "MotionGenerators/FollowerReference.h"
#include "MotionGenerators/FollowerRefManager.h"
#include "Utilities/EventProcessor.h"
#include "MotionGenerators/MotionMaster.h"
#include "Server/DBCStructure.h"
#include "WorldPacket.h"
#include "Timer.h"
#include "AI/BaseAI/UnitAI.h"

#include <list>

enum SpellInterruptFlags
{
    SPELL_INTERRUPT_FLAG_MOVEMENT     = 0x01,
    SPELL_INTERRUPT_FLAG_DAMAGE       = 0x02,
    SPELL_INTERRUPT_FLAG_UNK3         = 0x04,
    SPELL_INTERRUPT_FLAG_INTERRUPT    = 0x08,
    SPELL_INTERRUPT_FLAG_ABORT_ON_DMG = 0x10,               // _complete_ interrupt on direct damage
    // SPELL_INTERRUPT_UNK             = 0x20               // unk, 564 of 727 spells having this spell start with "Glyph"
};

enum SpellChannelInterruptFlags
{
    CHANNEL_FLAG_UNK        = 0x0001,
    CHANNEL_FLAG_DAMAGE     = 0x0002,
    CHANNEL_FLAG_MOVEMENT   = 0x0008,
    CHANNEL_FLAG_TURNING    = 0x0010,
    CHANNEL_FLAG_DAMAGE2    = 0x0080,
    CHANNEL_FLAG_UNK2       = 0x1000,
    CHANNEL_FLAG_DELAY      = 0x4000,
};

enum SpellAuraInterruptFlags
{
    AURA_INTERRUPT_FLAG_HITBYSPELL                  = 0x00000001,   // 0    removed when getting hit by a negative spell
    AURA_INTERRUPT_FLAG_DAMAGE                      = 0x00000002,   // 1    removed by any damage
    AURA_INTERRUPT_FLAG_UNK2                        = 0x00000004,   // 2
    AURA_INTERRUPT_FLAG_MOVE                        = 0x00000008,   // 3    removed by any movement
    AURA_INTERRUPT_FLAG_TURNING                     = 0x00000010,   // 4    removed by any turning
    AURA_INTERRUPT_FLAG_ENTER_COMBAT                = 0x00000020,   // 5    removed by entering combat
    AURA_INTERRUPT_FLAG_NOT_MOUNTED                 = 0x00000040,   // 6    removed by unmounting
    AURA_INTERRUPT_FLAG_NOT_ABOVEWATER              = 0x00000080,   // 7    removed by entering water
    AURA_INTERRUPT_FLAG_NOT_UNDERWATER              = 0x00000100,   // 8    removed by leaving water
    AURA_INTERRUPT_FLAG_NOT_SHEATHED                = 0x00000200,   // 9    removed by unsheathing
    AURA_INTERRUPT_FLAG_TALK                        = 0x00000400,   // 10   talk to npc / loot? action on creature
    AURA_INTERRUPT_FLAG_USE                         = 0x00000800,   // 11   mine/use/open action on gameobject
    AURA_INTERRUPT_FLAG_MELEE_ATTACK                = 0x00001000,   // 12   removed by attack
    AURA_INTERRUPT_FLAG_UNK13                       = 0x00002000,   // 13
    AURA_INTERRUPT_FLAG_UNK14                       = 0x00004000,   // 14
    AURA_INTERRUPT_FLAG_UNK15                       = 0x00008000,   // 15   removed by casting a spell?
    AURA_INTERRUPT_FLAG_UNK16                       = 0x00010000,   // 16
    AURA_INTERRUPT_FLAG_MOUNTING                    = 0x00020000,   // 17   removed by mounting
    AURA_INTERRUPT_FLAG_NOT_SEATED                  = 0x00040000,   // 18   removed by standing up (used by food and drink mostly and sleep/Fake Death like)
    AURA_INTERRUPT_FLAG_CHANGE_MAP                  = 0x00080000,   // 19   leaving map/getting teleported
    AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION    = 0x00100000,   // 20   removed by auras that make you invulnerable, or make other to loose selection on you
    AURA_INTERRUPT_FLAG_UNK21                       = 0x00200000,   // 21
    AURA_INTERRUPT_FLAG_TELEPORTED                  = 0x00400000,   // 22
    AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT            = 0x00800000,   // 23   removed by entering pvp combat
    AURA_INTERRUPT_FLAG_DIRECT_DAMAGE               = 0x01000000    // 24   removed by any direct damage
};

enum SpellPartialResist
{
    SPELL_PARTIAL_RESIST_NONE = 0,  // 0%, full hit
    SPELL_PARTIAL_RESIST_PCT_25,    // 25%
    SPELL_PARTIAL_RESIST_PCT_50,    // 50%
    SPELL_PARTIAL_RESIST_PCT_75,    // 75%
    SPELL_PARTIAL_RESIST_PCT_100,   // 100%, full resist
};

#define NUM_SPELL_PARTIAL_RESISTS 5

enum SpellModOp
{
    SPELLMOD_DAMAGE                     = 0,
    SPELLMOD_DURATION                   = 1,
    SPELLMOD_THREAT                     = 2,
    SPELLMOD_ATTACK_POWER               = 3,
    SPELLMOD_CHARGES                    = 4,
    SPELLMOD_RANGE                      = 5,
    SPELLMOD_RADIUS                     = 6,
    SPELLMOD_CRITICAL_CHANCE            = 7,
    SPELLMOD_ALL_EFFECTS                = 8,
    SPELLMOD_NOT_LOSE_CASTING_TIME      = 9,
    SPELLMOD_CASTING_TIME               = 10,
    SPELLMOD_COOLDOWN                   = 11,
    SPELLMOD_SPEED                      = 12,
    SPELLMOD_UNK1                       = 13, // unused
    SPELLMOD_COST                       = 14,
    SPELLMOD_CRIT_DAMAGE_BONUS          = 15,
    SPELLMOD_RESIST_MISS_CHANCE         = 16,
    SPELLMOD_JUMP_TARGETS               = 17,
    SPELLMOD_CHANCE_OF_SUCCESS          = 18, // Only used with SPELL_AURA_ADD_FLAT_MODIFIER and affects proc spells
    SPELLMOD_ACTIVATION_TIME            = 19,
    SPELLMOD_EFFECT_PAST_FIRST          = 20,
    SPELLMOD_GLOBAL_COOLDOWN            = 21,
    SPELLMOD_DOT                        = 22,
    SPELLMOD_HASTE                      = 23,
    SPELLMOD_SPELL_BONUS_DAMAGE         = 24,
    SPELLMOD_UNK2                       = 25, // unused
    // SPELLMOD_FREQUENCY_OF_SUCCESS    = 26,                // not used in 2.4.3
    SPELLMOD_MULTIPLE_VALUE             = 27,
    SPELLMOD_RESIST_DISPEL_CHANCE       = 28,
    MAX_SPELLMOD                        = 32,
};

enum SpellFacingFlags
{
    SPELL_FACING_FLAG_INFRONT = 0x0001
};

#define BASE_MELEERANGE_OFFSET 1.33f
#define BASE_MINDAMAGE 1.0f
#define BASE_MAXDAMAGE 2.0f
#define BASE_ATTACK_TIME 2000

// byte value (UNIT_FIELD_BYTES_1,0)
enum UnitStandStateType
{
    UNIT_STAND_STATE_STAND             = 0,
    UNIT_STAND_STATE_SIT               = 1,
    UNIT_STAND_STATE_SIT_CHAIR         = 2,
    UNIT_STAND_STATE_SLEEP             = 3,
    UNIT_STAND_STATE_SIT_LOW_CHAIR     = 4,
    UNIT_STAND_STATE_SIT_MEDIUM_CHAIR  = 5,
    UNIT_STAND_STATE_SIT_HIGH_CHAIR    = 6,
    UNIT_STAND_STATE_DEAD              = 7,
    UNIT_STAND_STATE_KNEEL             = 8
};

#define MAX_UNIT_STAND_STATE             9

/* byte flag value not exist in 1.12, moved/merged in (UNIT_FIELD_BYTES_1,3), in post-1.x it's in (UNIT_FIELD_BYTES_1,2)
enum UnitStandFlags
*/

// byte flags value (UNIT_FIELD_BYTES_1,2)
// This corresponds to free talent points (pet case)

// byte flags value (UNIT_FIELD_BYTES_1,3)
enum UnitBytes1_Flags
{
    UNIT_BYTE1_FLAG_ALWAYS_STAND = 0x01,
    UNIT_BYTE1_FLAGS_CREEP       = 0x02,
    UNIT_BYTE1_FLAG_UNTRACKABLE  = 0x04,
    UNIT_BYTE1_FLAG_ALL          = 0xFF
};

// byte value (UNIT_FIELD_BYTES_2,0)
enum SheathState
{
    SHEATH_STATE_UNARMED  = 0,                              // non prepared weapon
    SHEATH_STATE_MELEE    = 1,                              // prepared melee weapon
    SHEATH_STATE_RANGED   = 2                               // prepared ranged weapon
};

#define MAX_SHEATH_STATE    3

// byte flags value (UNIT_FIELD_BYTES_2,1)
enum UnitBytes2_Flags
{
    UNIT_BYTE2_FLAG_UNK0        = 0x01,
    UNIT_BYTE2_FLAG_UNK1        = 0x02,
    UNIT_BYTE2_FLAG_UNK2        = 0x04,
    UNIT_BYTE2_FLAG_SUPPORTABLE = 0x08,                     // allows for being targeted for healing/bandaging by friendlies
    UNIT_BYTE2_FLAG_AURAS       = 0x10,                     // show positive auras as positive, and allow its dispel
    UNIT_BYTE2_FLAG_UNK5        = 0x20,                     // show negative auras as positive, *not* allowing dispel (at least for pets)
    UNIT_BYTE2_FLAG_UNK6        = 0x40,
    UNIT_BYTE2_FLAG_UNK7        = 0x80
};

#define CREATURE_MAX_SPELLS     8

enum Swing
{
    NOSWING                    = 0,
    SINGLEHANDEDSWING          = 1,
    TWOHANDEDSWING             = 2
};

enum VictimState
{
    VICTIMSTATE_UNAFFECTED     = 0,                         // seen in relation with HITINFO_MISS
    VICTIMSTATE_NORMAL         = 1,
    VICTIMSTATE_DODGE          = 2,
    VICTIMSTATE_PARRY          = 3,
    VICTIMSTATE_INTERRUPT      = 4,
    VICTIMSTATE_BLOCKS         = 5,
    VICTIMSTATE_EVADES         = 6,
    VICTIMSTATE_IS_IMMUNE      = 7,
    VICTIMSTATE_DEFLECTS       = 8
};

enum HitInfo
{
    HITINFO_NORMALSWING         = 0x00000000,
    HITINFO_UNK0                = 0x00000001,               // req correct packet structure
    HITINFO_NORMALSWING2        = 0x00000002,
    HITINFO_LEFTSWING           = 0x00000004,
    HITINFO_UNK3                = 0x00000008,
    HITINFO_MISS                = 0x00000010,
    HITINFO_ABSORB              = 0x00000020,               // plays absorb sound
    HITINFO_RESIST              = 0x00000040,               // resisted atleast some damage
    HITINFO_CRITICALHIT         = 0x00000080,
    HITINFO_UNK8                = 0x00000100,               // wotlk?
    HITINFO_BLOCK               = 0x00000800,
    HITINFO_UNK9                = 0x00002000,               // wotlk?
    HITINFO_GLANCING            = 0x00004000,
    HITINFO_CRUSHING            = 0x00008000,
    HITINFO_NOACTION            = 0x00010000,
    HITINFO_SWINGNOHITSOUND     = 0x00080000
};

struct FactionTemplateEntry;
struct Modifier;
struct SpellEntry;
struct SpellEntryExt;

class Aura;
class SpellAuraHolder;
class Creature;
class Spell;
class DynamicObject;
class GameObject;
class Item;
class Pet;
class PetAura;
class Totem;
class SpellCastTargets;

struct SpellImmune
{
    uint32 type;
    Aura const* aura;
};

typedef std::list<SpellImmune> SpellImmuneList;

enum UnitModifierType
{
    BASE_VALUE = 0,
    BASE_PCT = 1,
    TOTAL_VALUE = 2,
    TOTAL_PCT = 3,
    MODIFIER_TYPE_END = 4
};

enum WeaponDamageRange
{
    MINDAMAGE,
    MAXDAMAGE
};

enum DamageTypeToSchool
{
    RESISTANCE,
    DAMAGE_DEALT,
    DAMAGE_TAKEN
};

enum AuraRemoveMode
{
    AURA_REMOVE_BY_DEFAULT,
    AURA_REMOVE_BY_STACK,                                   // at replace by similar aura
    AURA_REMOVE_BY_CANCEL,
    AURA_REMOVE_BY_DISPEL,
    AURA_REMOVE_BY_DEATH,
    AURA_REMOVE_BY_DELETE,                                  // use for speedup and prevent unexpected effects at player logout/pet unsummon (must be used _only_ after save), delete.
    AURA_REMOVE_BY_SHIELD_BREAK,                            // when absorb shield is removed by damage
    AURA_REMOVE_BY_EXPIRE,                                  // at duration end
    AURA_REMOVE_BY_TRACKING,                                // aura is removed because of a conflicting tracked aura
    AURA_REMOVE_BY_GAINED_STACK                             // gained stack
};

// Spell triggering settings for CastSpell that enable us to skip some checks so that we can investigate spell specific settings
enum TriggerCastFlags : uint32
{
    TRIGGERED_NONE                              = 0x00000000,   // Not Triggered
    TRIGGERED_OLD_TRIGGERED                     = 0x00000001,   // Legacy bool support TODO: Restrict usage as much as possible.
    TRIGGERED_IGNORE_HIT_CALCULATION            = 0x00000002,   // Will ignore calculating hit in SpellHitResult
    TRIGGERED_IGNORE_UNSELECTABLE_FLAG          = 0x00000004,   // Ignores UNIT_FLAG_NOT_SELECTABLE in CheckTarget
    TRIGGERED_INSTANT_CAST                      = 0x00000008,   // Will ignore any cast time set in spell entry
    TRIGGERED_AUTOREPEAT                        = 0x00000010,   // Will signal spell system that this is internal autorepeat call
    TRIGGERED_IGNORE_UNATTACKABLE_FLAG          = 0x00000020,   // Ignores UNIT_FLAG_NOT_ATTACKABLE in CheckTarget
    TRIGGERED_DO_NOT_PROC                       = 0x00000040,   // Spells from scripts should not proc - DBScripts for example
    TRIGGERED_PET_CAST                          = 0x00000080,   // Spell that should report error through pet opcode
    TRIGGERED_NORMAL_COMBAT_CAST                = 0x00000100,   // AI needs to be notified about change of target
    TRIGGERED_FULL_MASK                         = 0xFFFFFFFF
};

enum UnitMods
{
    UNIT_MOD_STAT_STRENGTH,                                 // UNIT_MOD_STAT_STRENGTH..UNIT_MOD_STAT_SPIRIT must be in existing order, it's accessed by index values of Stats enum.
    UNIT_MOD_STAT_AGILITY,
    UNIT_MOD_STAT_STAMINA,
    UNIT_MOD_STAT_INTELLECT,
    UNIT_MOD_STAT_SPIRIT,
    UNIT_MOD_HEALTH,
    UNIT_MOD_MANA,                                          // UNIT_MOD_MANA..UNIT_MOD_HAPPINESS must be in existing order, it's accessed by index values of Powers enum.
    UNIT_MOD_RAGE,
    UNIT_MOD_FOCUS,
    UNIT_MOD_ENERGY,
    UNIT_MOD_HAPPINESS,
    UNIT_MOD_ARMOR,                                         // UNIT_MOD_ARMOR..UNIT_MOD_RESISTANCE_ARCANE must be in existing order, it's accessed by index values of SpellSchools enum.
    UNIT_MOD_RESISTANCE_HOLY,
    UNIT_MOD_RESISTANCE_FIRE,
    UNIT_MOD_RESISTANCE_NATURE,
    UNIT_MOD_RESISTANCE_FROST,
    UNIT_MOD_RESISTANCE_SHADOW,
    UNIT_MOD_RESISTANCE_ARCANE,
    UNIT_MOD_ATTACK_POWER,
    UNIT_MOD_ATTACK_POWER_RANGED,
    UNIT_MOD_DAMAGE_MAINHAND,
    UNIT_MOD_DAMAGE_OFFHAND,
    UNIT_MOD_DAMAGE_RANGED,
    UNIT_MOD_END,
    // synonyms
    UNIT_MOD_STAT_START = UNIT_MOD_STAT_STRENGTH,
    UNIT_MOD_STAT_END = UNIT_MOD_STAT_SPIRIT + 1,
    UNIT_MOD_RESISTANCE_START = UNIT_MOD_ARMOR,
    UNIT_MOD_RESISTANCE_END = UNIT_MOD_RESISTANCE_ARCANE + 1,
    UNIT_MOD_POWER_START = UNIT_MOD_MANA,
    UNIT_MOD_POWER_END = UNIT_MOD_HAPPINESS + 1
};

enum BaseModGroup
{
    CRIT_PERCENTAGE,
    RANGED_CRIT_PERCENTAGE,
    OFFHAND_CRIT_PERCENTAGE,
    SHIELD_BLOCK_VALUE,
    BASEMOD_END
};

enum BaseModType
{
    FLAT_MOD,
    PCT_MOD,
    MOD_END
};

enum DeathState
{
    ALIVE          = 0,                                     // show as alive
    JUST_DIED      = 1,                                     // temporary state at die, for creature auto converted to CORPSE, for player at next update call
    CORPSE         = 2,                                     // corpse state, for player this also meaning that player not leave corpse
    DEAD           = 3,                                     // for creature despawned state (corpse despawned), for player CORPSE/DEAD not clear way switches (FIXME), and use m_deathtimer > 0 check for real corpse state
    JUST_ALIVED    = 4,                                     // temporary state at resurrection, for creature auto converted to ALIVE, for player at next update call
};

// internal state flags for some auras and movement generators, other.
enum UnitState
{
    // persistent state (applied by aura/etc until expire)
    UNIT_STAT_MELEE_ATTACKING = 0x00000001,                 // unit is melee attacking someone Unit::Attack
    //UNIT_STAT_ATTACK_PLAYER = 0x00000002,                 // (Deprecated) unit attack player or player's controlled unit and have contested pvpv timer setup, until timer expire, combat end and etc
    UNIT_STAT_FEIGN_DEATH     = 0x00000004,                 // Unit::SetFeignDeath - a successful feign death is currently active
    UNIT_STAT_STUNNED         = 0x00000008,                 // Aura::HandleAuraModStun
    UNIT_STAT_ROOT            = 0x00000010,                 // Aura::HandleAuraModRoot
    UNIT_STAT_ISOLATED        = 0x00000020,                 // area auras do not affect other players, Aura::HandleAuraModSchoolImmunity
    UNIT_STAT_POSSESSED       = 0x00000040,                 // Aura::HandleAuraModPossess (duplicates UNIT_FLAG_POSSESSED)

    // persistent movement generator state (all time while movement generator applied to unit (independent from top state of movegen)
    UNIT_STAT_TAXI_FLIGHT     = 0x00000080,                 // player is in flight mode (in fact interrupted at far teleport until next map telport landing)
    UNIT_STAT_DISTRACTED      = 0x00000100,                 // DistractedMovementGenerator active

    // persistent movement generator state with non-persistent mirror states for stop support
    // (can be removed temporary by stop command or another movement generator apply)
    // not use _MOVE versions for generic movegen state, it can be removed temporary for unit stop and etc
    UNIT_STAT_CONFUSED        = 0x00000200,                 // ConfusedMovementGenerator active/onstack
    UNIT_STAT_CONFUSED_MOVE   = 0x00000400,
    UNIT_STAT_ROAMING         = 0x00000800,                 // RandomMovementGenerator/PointMovementGenerator/WaypointMovementGenerator active (now always set)
    UNIT_STAT_ROAMING_MOVE    = 0x00001000,
    UNIT_STAT_CHASE           = 0x00002000,                 // ChaseMovementGenerator active
    UNIT_STAT_CHASE_MOVE      = 0x00004000,
    UNIT_STAT_FOLLOW          = 0x00008000,                 // FollowMovementGenerator active
    UNIT_STAT_FOLLOW_MOVE     = 0x00010000,
    UNIT_STAT_FLEEING         = 0x00020000,                 // FleeMovementGenerator/TimedFleeingMovementGenerator active/onstack
    UNIT_STAT_FLEEING_MOVE    = 0x00040000,
    UNIT_STAT_SEEKING_ASSISTANCE = 0x00080000,
    UNIT_STAT_DONT_TURN       = 0x00100000,                 // Creature will not turn and acquire new target
    UNIT_STAT_CHANNELING      = 0x00200000,
    // More room for other MMGens

    // High-Level states (usually only with Creatures)
    UNIT_STAT_NO_COMBAT_MOVEMENT    = 0x01000000,           // Combat Movement for MoveChase stopped
    UNIT_STAT_RUNNING               = 0x02000000,           // SetRun for waypoints and such
    UNIT_STAT_WAYPOINT_PAUSED       = 0x04000000,           // Waypoint-Movement paused genericly (ie by script)

    UNIT_STAT_IGNORE_PATHFINDING    = 0x10000000,           // do not use pathfinding in any MovementGenerator

    // masks (only for check)

    // can't move currently
    UNIT_STAT_CAN_NOT_MOVE    = UNIT_STAT_ROOT | UNIT_STAT_STUNNED | UNIT_STAT_FEIGN_DEATH,

    // stay by different reasons
    UNIT_STAT_NOT_MOVE        = UNIT_STAT_ROOT | UNIT_STAT_STUNNED | UNIT_STAT_FEIGN_DEATH |
                                UNIT_STAT_DISTRACTED,

    // stay or scripted movement for effect( = in player case you can't move by client command)
    UNIT_STAT_NO_FREE_MOVE    = UNIT_STAT_ROOT | UNIT_STAT_STUNNED | UNIT_STAT_FEIGN_DEATH |
                                UNIT_STAT_TAXI_FLIGHT |
                                UNIT_STAT_CONFUSED | UNIT_STAT_FLEEING,

    // not react at move in sight or other
    UNIT_STAT_CAN_NOT_REACT   = UNIT_STAT_STUNNED | UNIT_STAT_FEIGN_DEATH |
                                UNIT_STAT_CONFUSED | UNIT_STAT_FLEEING,

    // AI disabled by some reason
    UNIT_STAT_LOST_CONTROL    = UNIT_STAT_CONFUSED | UNIT_STAT_FLEEING | UNIT_STAT_POSSESSED,

    // above 2 state cases
    UNIT_STAT_CAN_NOT_REACT_OR_LOST_CONTROL  = UNIT_STAT_CAN_NOT_REACT | UNIT_STAT_LOST_CONTROL,

    // masks (for check or reset)

    // for real move using movegen check and stop (except unstoppable flight)
    UNIT_STAT_MOVING          = UNIT_STAT_ROAMING_MOVE | UNIT_STAT_CHASE_MOVE | UNIT_STAT_FOLLOW_MOVE | UNIT_STAT_FLEEING_MOVE,

    UNIT_STAT_RUNNING_STATE   = UNIT_STAT_CHASE_MOVE | UNIT_STAT_FLEEING_MOVE | UNIT_STAT_RUNNING,

    UNIT_STAT_ALL_STATE       = 0xFFFFFFFF,
    UNIT_STAT_ALL_DYN_STATES  = UNIT_STAT_ALL_STATE & ~(UNIT_STAT_NO_COMBAT_MOVEMENT | UNIT_STAT_RUNNING | UNIT_STAT_WAYPOINT_PAUSED | UNIT_STAT_IGNORE_PATHFINDING),
};

enum UnitMoveType
{
    MOVE_WALK           = 0,
    MOVE_RUN            = 1,
    MOVE_RUN_BACK       = 2,
    MOVE_SWIM           = 3,
    MOVE_SWIM_BACK      = 4,
    MOVE_TURN_RATE      = 5,
};

#define MAX_MOVE_TYPE     6

#define BASE_CHARGE_SPEED 27.0f

/// internal used flags for marking special auras - for example some dummy-auras
enum UnitAuraFlags
{
    UNIT_AURAFLAG_ALIVE_INVISIBLE   = 0x1,                  // aura which makes unit invisible for alive
};

enum UnitVisibility
{
    VISIBILITY_OFF                = 0,                      // absolute, not detectable, GM-like, can see all other
    VISIBILITY_ON                 = 1,
    VISIBILITY_GROUP_STEALTH      = 2,                      // detect chance, seen and can see group members
    VISIBILITY_GROUP_INVISIBILITY = 3,                      // invisibility, can see and can be seen only another invisible unit or invisible detection unit, set only if not stealthed, and in checks not used (mask used instead)
    VISIBILITY_GROUP_NO_DETECT    = 4,                      // state just at stealth apply for update Grid state. Don't remove, otherwise stealth spells will break
    VISIBILITY_REMOVE_CORPSE      = 5                       // special totally not detectable visibility for force delete object while removing a corpse
};

// [-ZERO] Need recheck values
// Value masks for UNIT_FIELD_FLAGS
enum UnitFlags
{
    UNIT_FLAG_NONE                  = 0x00000000,
    UNIT_FLAG_UNK_0                 = 0x00000001,           // Movement checks disabled, likely paired with loss of client control packet. We use it to add custom cliffwalking to GM mode until actual usecases will be known.
    UNIT_FLAG_NON_ATTACKABLE        = 0x00000002,           // not attackable
    UNIT_FLAG_CLIENT_CONTROL_LOST   = 0x00000004,           // Generic unspecified loss of control initiated by server script, movement checks disabled, paired with loss of client control packet.
    UNIT_FLAG_PLAYER_CONTROLLED     = 0x00000008,           // players, pets, totems, guardians, companions, charms, any units associated with players
    UNIT_FLAG_PET_RENAME            = 0x00000010,           // Old pet rename: moved to UNIT_FIELD_BYTES_2,2 in TBC+
    UNIT_FLAG_PET_ABANDON           = 0x00000020,           // Old pet abandon: moved to UNIT_FIELD_BYTES_2,2 in TBC+
    UNIT_FLAG_UNK_6                 = 0x00000040,
    UNIT_FLAG_IMMUNE_TO_PLAYER      = 0x00000100,           // Target is immune to players
    UNIT_FLAG_IMMUNE_TO_NPC         = 0x00000200,           // Target is immune to Non-Player Characters
    UNIT_FLAG_PVP                   = 0x00001000,
    UNIT_FLAG_SILENCED              = 0x00002000,           // silenced, 2.1.1
    UNIT_FLAG_PERSUADED             = 0x00004000,           // persuaded, 2.0.8
    UNIT_FLAG_SWIMMING              = 0x00008000,           // controls water swimming animation - TODO: confirm whether dynamic or static
    UNIT_FLAG_NON_ATTACKABLE_2      = 0x00010000,           // removes attackable icon, if on yourself, cannot assist self but can cast TARGET_UNIT_CASTER spells - added by SPELL_AURA_MOD_UNATTACKABLE
    UNIT_FLAG_PACIFIED              = 0x00020000,
    UNIT_FLAG_STUNNED               = 0x00040000,           // Unit is a subject to stun, turn and strafe movement disabled
    UNIT_FLAG_IN_COMBAT             = 0x00080000,
    UNIT_FLAG_TAXI_FLIGHT           = 0x00100000,           // Unit is on taxi, paired with a duplicate loss of client control packet (likely a legacy serverside hack). Disables any spellcasts not allowed in taxi flight client-side.
    UNIT_FLAG_CONFUSED              = 0x00400000,           // Unit is a subject to confused movement, movement checks disabled, paired with loss of client control packet.
    UNIT_FLAG_FLEEING               = 0x00800000,           // Unit is a subject to fleeing movement, movement checks disabled, paired with loss of client control packet.
    UNIT_FLAG_POSSESSED             = 0x01000000,           // Unit is under remote control by another unit, movement checks disabled, paired with loss of client control packet. New master is allowed to use melee attack and can't select this unit via mouse in the world (as if it was own character).
    UNIT_FLAG_NOT_SELECTABLE        = 0x02000000,
    UNIT_FLAG_SKINNABLE             = 0x04000000,
    UNIT_FLAG_AURAS_VISIBLE         = 0x08000000,           // magic detect
    UNIT_FLAG_SHEATHE               = 0x40000000,
    // UNIT_FLAG_UNK_31              = 0x80000000           // no affect in 1.12.1

    // [-ZERO] TBC enumerations [?]
    UNIT_FLAG_NOT_ATTACKABLE_1      = 0x00000080,           // ?? (UNIT_FLAG_PVP_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1) is NON_PVP_ATTACKABLE
    UNIT_FLAG_LOOTING               = 0x00000400,           // loot animation
    UNIT_FLAG_PET_IN_COMBAT         = 0x00000800,           // in combat?, 2.0.8
    UNIT_FLAG_DISARMED              = 0x00200000,           // disable melee spells casting..., "Required melee weapon" added to melee spells tooltip.
//[-ZERO]    UNIT_FLAG_MOUNT                 = 0x08000000,
    UNIT_FLAG_UNK_28                = 0x10000000,
    UNIT_FLAG_UNK_29                = 0x20000000,           // used in Feing Death spell
};

/// Non Player Character flags
enum NPCFlags
{
    UNIT_NPC_FLAG_NONE                  = 0x00000000,
    UNIT_NPC_FLAG_GOSSIP                = 0x00000001,       // 100%
    UNIT_NPC_FLAG_QUESTGIVER            = 0x00000002,       // guessed, probably ok
    UNIT_NPC_FLAG_VENDOR                = 0x00000004,       // 100%
    UNIT_NPC_FLAG_FLIGHTMASTER          = 0x00000008,       // 100%
    UNIT_NPC_FLAG_TRAINER               = 0x00000010,       // 100%
    UNIT_NPC_FLAG_SPIRITHEALER          = 0x00000020,       // guessed
    UNIT_NPC_FLAG_SPIRITGUIDE           = 0x00000040,       // guessed
    UNIT_NPC_FLAG_INNKEEPER             = 0x00000080,       // 100%
    UNIT_NPC_FLAG_BANKER                = 0x00000100,       // 100%
    UNIT_NPC_FLAG_PETITIONER            = 0x00000200,       // 100% 0xC0000 = guild petitions
    UNIT_NPC_FLAG_TABARDDESIGNER        = 0x00000400,       // 100%
    UNIT_NPC_FLAG_BATTLEMASTER          = 0x00000800,       // 100%
    UNIT_NPC_FLAG_AUCTIONEER            = 0x00001000,       // 100%
    UNIT_NPC_FLAG_STABLEMASTER          = 0x00002000,       // 100%
    UNIT_NPC_FLAG_REPAIR                = 0x00004000,       // 100%
    UNIT_NPC_FLAG_OUTDOORPVP            = 0x20000000,       // custom flag for outdoor pvp creatures || Custom flag
};

// [-ZERO] Need check and update
// used in most movement packets (send and received)
enum MovementFlags
{
    MOVEFLAG_NONE               = 0x00000000,
    MOVEFLAG_FORWARD            = 0x00000001,
    MOVEFLAG_BACKWARD           = 0x00000002,
    MOVEFLAG_STRAFE_LEFT        = 0x00000004,
    MOVEFLAG_STRAFE_RIGHT       = 0x00000008,
    MOVEFLAG_TURN_LEFT          = 0x00000010,
    MOVEFLAG_TURN_RIGHT         = 0x00000020,
    MOVEFLAG_PITCH_UP           = 0x00000040,
    MOVEFLAG_PITCH_DOWN         = 0x00000080,
    MOVEFLAG_WALK_MODE          = 0x00000100,               // Walking

    MOVEFLAG_LEVITATING         = 0x00000400,
    MOVEFLAG_FLYING             = 0x00000800,               // [-ZERO] is it really need and correct value
    MOVEFLAG_FALLING            = 0x00002000,
    MOVEFLAG_FALLINGFAR         = 0x00004000,
    MOVEFLAG_SWIMMING           = 0x00200000,               // appears with fly flag also
    MOVEFLAG_SPLINE_ENABLED     = 0x00400000,               // [-ZERO] is it really need and correct value
    MOVEFLAG_CAN_FLY            = 0x00800000,               // [-ZERO] is it really need and correct value
    MOVEFLAG_FLYING_OLD         = 0x01000000,               // [-ZERO] is it really need and correct value

    MOVEFLAG_ONTRANSPORT        = 0x02000000,               // Used for flying on some creatures
    MOVEFLAG_SPLINE_ELEVATION   = 0x04000000,               // used for flight paths
    MOVEFLAG_ROOT               = 0x08000000,               // used for flight paths
    MOVEFLAG_WATERWALKING       = 0x10000000,               // prevent unit from falling through water
    MOVEFLAG_SAFE_FALL          = 0x20000000,               // active rogue safe fall spell (passive)
    MOVEFLAG_HOVER              = 0x40000000,

    MOVEFLAG_MASK_MOVING_FORWARD = MOVEFLAG_FORWARD | MOVEFLAG_STRAFE_LEFT | MOVEFLAG_STRAFE_RIGHT | MOVEFLAG_FALLING,
};

// flags that use in movement check for example at spell casting
MovementFlags const movementFlagsMask = MovementFlags(
        MOVEFLAG_FORWARD | MOVEFLAG_BACKWARD  | MOVEFLAG_STRAFE_LEFT | MOVEFLAG_STRAFE_RIGHT |
        MOVEFLAG_PITCH_UP | MOVEFLAG_PITCH_DOWN | MOVEFLAG_FALLING |
        MOVEFLAG_FALLINGFAR | MOVEFLAG_SPLINE_ELEVATION
                                        );

MovementFlags const movementOrTurningFlagsMask = MovementFlags(
            movementFlagsMask | MOVEFLAG_TURN_LEFT | MOVEFLAG_TURN_RIGHT
        );

class MovementInfo
{
    public:
        MovementInfo() : moveFlags(MOVEFLAG_NONE), time(0),
            t_time(0), s_pitch(0.0f), fallTime(0), u_unk1(0.0f) {}

        // Read/Write methods
        void Read(ByteBuffer& data);
        void Write(ByteBuffer& data) const;

        // Movement flags manipulations
        void AddMovementFlag(MovementFlags f) { moveFlags |= f; }
        void RemoveMovementFlag(MovementFlags f) { moveFlags &= ~f; }
        bool HasMovementFlag(MovementFlags f) const { return (moveFlags & f) != 0; }
        MovementFlags GetMovementFlags() const { return MovementFlags(moveFlags); }
        void SetMovementFlags(MovementFlags f) { moveFlags = f; }

        // Position manipulations
        Position const* GetPos() const { return &pos; }
        void SetTransportData(ObjectGuid guid, float x, float y, float z, float o, uint32 time)
        {
            t_guid = guid;
            t_pos.x = x;
            t_pos.y = y;
            t_pos.z = z;
            t_pos.o = o;
            t_time = time;
        }
        void ClearTransportData()
        {
            t_guid = ObjectGuid();
            t_pos.x = 0.0f;
            t_pos.y = 0.0f;
            t_pos.z = 0.0f;
            t_pos.o = 0.0f;
            t_time = 0;
        }
        ObjectGuid const& GetTransportGuid() const { return t_guid; }
        Position const* GetTransportPos() const { return &t_pos; }
        uint32 GetTransportTime() const { return t_time; }
        uint32 GetFallTime() const { return fallTime; }
        void ChangeOrientation(float o) { pos.o = o; }
        void ChangePosition(float x, float y, float z, float o) { pos.x = x; pos.y = y; pos.z = z; pos.o = o; }
        void UpdateTime(uint32 _time) { time = _time; }
        uint32 GetTime() const { return time; }

        struct JumpInfo
        {
            JumpInfo() : velocity(0.f), sinAngle(0.f), cosAngle(0.f), xyspeed(0.f) {}
            float   velocity, sinAngle, cosAngle, xyspeed;
        };

        JumpInfo const& GetJumpInfo() const { return jump; }
    private:
        // common
        uint32   moveFlags;                                 // see enum MovementFlags
        uint32   time;
        Position pos;
        // transport
        ObjectGuid t_guid;
        Position t_pos;
        uint32   t_time;
        // swimming and unknown
        float    s_pitch;
        // last fall time
        uint32   fallTime;
        // jumping
        JumpInfo jump;
        // spline
        float    u_unk1;
};

inline ByteBuffer& operator<< (ByteBuffer& buf, MovementInfo const& mi)
{
    mi.Write(buf);
    return buf;
}

inline ByteBuffer& operator>> (ByteBuffer& buf, MovementInfo& mi)
{
    mi.Read(buf);
    return buf;
}

namespace Movement
{
    class MoveSpline;
}

/**
 * The different available diminishing return levels.
 * \see DiminishingReturn
 */
enum DiminishingLevels
{
    DIMINISHING_LEVEL_1             = 0,         //< Won't make a difference to stun duration
    DIMINISHING_LEVEL_2             = 1,         //< Reduces stun time by 50%
    DIMINISHING_LEVEL_3             = 2,         //< Reduces stun time by 75%
    DIMINISHING_LEVEL_IMMUNE        = 3          //< The target is immune to the DiminishingGrouop
};

/**
 * Structure to keep track of diminishing returns, for more information
 * about the idea behind diminishing returns, see: http://www.wowwiki.com/Diminishing_returns
 * \see Unit::GetDiminishing
 * \see Unit::IncrDiminishing
 * \see Unit::ApplyDiminishingToDuration
 * \see Unit::ApplyDiminishingAura
 */
struct DiminishingReturn
{
    DiminishingReturn(DiminishingGroup group, uint32 t, uint32 count)
        : DRGroup(group), stack(0), hitTime(t), hitCount(count)
    {}

    /**
     * Group that this diminishing return will affect
     */
    DiminishingGroup        DRGroup: 16;
    /**
     * Seems to be how many times this has been stacked, modified in
     * Unit::ApplyDiminishingAura
     */
    uint16                  stack: 16;
    /**
     * Records at what time the last hit with this DiminishingGroup was done, if it's
     * higher than 15 seconds (ie: 15 000 ms) the DiminishingReturn::hitCount will be reset
     * to DiminishingLevels::DIMINISHING_LEVEL_1, which will do no difference to the duration
     * of the stun etc.
     */
    uint32                  hitTime;
    /**
     * Records how many times a spell of this DiminishingGroup has hit, this in turn
     * decides how how long the duration of the stun etc is.
     */
    uint32                  hitCount;
};

// At least some values expected fixed and used in auras field, other custom
enum MeleeHitOutcome
{
    MELEE_HIT_EVADE     = 0,
    MELEE_HIT_MISS      = 1,
    MELEE_HIT_DODGE     = 2,                                // used as misc in SPELL_AURA_IGNORE_COMBAT_RESULT
    MELEE_HIT_BLOCK     = 3,                                // used as misc in SPELL_AURA_IGNORE_COMBAT_RESULT
    MELEE_HIT_PARRY     = 4,                                // used as misc in SPELL_AURA_IGNORE_COMBAT_RESULT
    MELEE_HIT_GLANCING  = 5,
    MELEE_HIT_CRIT      = 6,
    MELEE_HIT_CRUSHING  = 7,
    MELEE_HIT_NORMAL    = 8,
    MELEE_HIT_BLOCK_CRIT = 9,
};

enum UnitCombatDieSide
{
    UNIT_COMBAT_DIE_MISS,
    UNIT_COMBAT_DIE_RESIST,
    UNIT_COMBAT_DIE_DODGE,
    UNIT_COMBAT_DIE_PARRY,
    UNIT_COMBAT_DIE_DEFLECT,
    UNIT_COMBAT_DIE_BLOCK,
    UNIT_COMBAT_DIE_GLANCE,
    UNIT_COMBAT_DIE_CRIT,
    UNIT_COMBAT_DIE_CRUSH,
    UNIT_COMBAT_DIE_HIT,
};

#define NUM_UNIT_COMBAT_DIE_SIDES (UNIT_COMBAT_DIE_HIT + 1)

// A little helper func for a nice attack table debug output
inline const char* UnitCombatDieSideText(UnitCombatDieSide side)
{
    switch (side)
    {
        case UNIT_COMBAT_DIE_MISS:    return "MISS";
        case UNIT_COMBAT_DIE_RESIST:  return "RESIST";
        case UNIT_COMBAT_DIE_DODGE:   return "DODGE";
        case UNIT_COMBAT_DIE_PARRY:   return "PARRY";
        case UNIT_COMBAT_DIE_DEFLECT: return "DEFLECT";
        case UNIT_COMBAT_DIE_BLOCK:   return "BLOCK";
        case UNIT_COMBAT_DIE_GLANCE:  return "GLANCE";
        case UNIT_COMBAT_DIE_CRIT:    return "CRIT";
        case UNIT_COMBAT_DIE_CRUSH:   return "CRUSH";
        case UNIT_COMBAT_DIE_HIT:     return "HIT";
    }
    return "INVALID";
}

struct CleanDamage
{
    CleanDamage(uint32 _damage, WeaponAttackType _attackType, MeleeHitOutcome _hitOutCome) :
        damage(_damage), attackType(_attackType), hitOutCome(_hitOutCome) {}

    uint32 damage; // only used for rage generation
    WeaponAttackType attackType;
    MeleeHitOutcome hitOutCome;
};

struct SubDamageInfo
{
    SpellSchoolMask damageSchoolMask = SPELL_SCHOOL_MASK_NORMAL;
    uint32 damage = 0;
    uint32 absorb = 0;
    int32 resist = 0;
};

// Struct for use in Unit::CalculateMeleeDamage
// Need create structure like in SMSG_ATTACKERSTATEUPDATE opcode
struct CalcDamageInfo
{
    Unit*  attacker;             // Attacker
    Unit*  target;               // Target for damage
    uint32 totalDamage;
    SubDamageInfo subDamage[MAX_ITEM_PROTO_DAMAGES];
    uint32 blocked_amount;
    uint32 HitInfo;
    uint32 TargetState;
    // Helper
    WeaponAttackType attackType; //
    uint32 procAttacker;
    uint32 procVictim;
    uint32 procEx;
    uint32 cleanDamage;          // Used only for rage calculation
    MeleeHitOutcome hitOutCome;  // TODO: remove this field (need use TargetState)
};

// Spell damage info structure based on structure sending in SMSG_SPELLNONMELEEDAMAGELOG opcode
struct SpellNonMeleeDamage
{
    SpellNonMeleeDamage(Unit* _attacker, Unit* _target, uint32 _SpellID, SpellSchools _school)
        : target(_target), attacker(_attacker), SpellID(_SpellID), damage(0), school(_school),
          absorb(0), resist(0), periodicLog(false), unused(false), blocked(0), HitInfo(0)
    {}

    Unit*   target;
    Unit*   attacker;
    uint32 SpellID;
    uint32 damage;
    SpellSchools school;
    uint32 absorb;
    int32  resist;
    bool   periodicLog;
    bool   unused;
    uint32 blocked;
    uint32 HitInfo;
};

struct SpellPeriodicAuraLogInfo
{
    SpellPeriodicAuraLogInfo(Aura* _aura, uint32 _damage, uint32 _absorb, int32 _resist, float _multiplier)
        : aura(_aura), damage(_damage), absorb(_absorb), resist(_resist), multiplier(_multiplier) {}

    Aura*   aura;
    uint32 damage;
    uint32 absorb;
    int32 resist;
    float  multiplier;
};

uint32 createProcExtendMask(SpellNonMeleeDamage* damageInfo, SpellMissInfo missCondition);

struct CombatData
{
    public:
        CombatData(Unit* owner) : threatManager(ThreatManager(owner)), hostileRefManager(HostileRefManager(owner)) {};

        // Manage all Units threatening us
        ThreatManager threatManager;
        // Manage all Units that are threatened by us
        HostileRefManager hostileRefManager;
};

enum SpellAuraProcResult
{
    SPELL_AURA_PROC_OK              = 0,                    // proc was processed, will remove charges
    SPELL_AURA_PROC_FAILED          = 1,                    // proc failed - if at least one aura failed the proc, charges won't be taken
    SPELL_AURA_PROC_CANT_TRIGGER    = 2                     // aura can't trigger - skip charges taking, move to next aura if exists
};

// Unit* victim, uint32 procAttacker, uint32 procVictim, uint32 procExtra, uint32 amount, WeaponAttackType attType, SpellEntry const* procSpell, bool dontTriggerSpecial

// External struct for passing on data
struct ProcSystemArguments
{
    Unit* attacker;
    Unit* victim;

    uint32 procFlagsAttacker;
    uint32 procFlagsVictim;
    uint32 procExtra;

    uint32 damage; // contains full heal or full damage
    SpellEntry const* procSpell;
    WeaponAttackType attType;

    Spell* spell;

    // Healing specific information
    uint32 healthGain;

    explicit ProcSystemArguments(Unit* victim, uint32 procFlagsAttacker, uint32 procFlagsVictim, uint32 procExtra, uint32 amount, WeaponAttackType attType = BASE_ATTACK,
        SpellEntry const* procSpell = nullptr, Spell* spell = nullptr, uint32 healthGain = 0) : attacker(nullptr), victim(victim), procFlagsAttacker(procFlagsAttacker), procFlagsVictim(procFlagsVictim), procExtra(procExtra), damage(amount),
        procSpell(procSpell), attType(attType), spell(spell), healthGain(healthGain)
    {
    }
};

// Internal struct for passing data to execution
struct ProcExecutionData
{
    bool isVictim;

    Unit* attacker;
    Unit* victim;

    uint32 procFlags;
    uint32 procExtra;

    WeaponAttackType attType;
    uint32 damage; // contains full heal or full damage
    SpellEntry const* procSpell;

    Spell* spell;

    // Healing specific information
    uint32 healthGain;

    Aura* triggeredByAura;
    uint32 cooldown;

    ProcExecutionData(ProcSystemArguments& data, bool isVictim);
};
typedef SpellAuraProcResult(Unit::*pAuraProcHandler)(ProcExecutionData& data);
extern pAuraProcHandler AuraProcHandler[TOTAL_AURAS];

enum CurrentSpellTypes
{
    CURRENT_MELEE_SPELL             = 0,
    CURRENT_GENERIC_SPELL           = 1,
    CURRENT_AUTOREPEAT_SPELL        = 2,
    CURRENT_CHANNELED_SPELL         = 3
};

#define CURRENT_FIRST_NON_MELEE_SPELL 1
#define CURRENT_MAX_SPELL             4
#define INVISIBILITY_MAX              32

enum ActiveStates
{
    ACT_PASSIVE  = 0x01,                                    // 0x01 - passive
    ACT_DISABLED = 0x81,                                    // 0x80 - castable
    ACT_ENABLED  = 0xC1,                                    // 0x40 | 0x80 - auto cast + castable
    ACT_COMMAND  = 0x07,                                    // 0x01 | 0x02 | 0x04
    ACT_REACTION = 0x06,                                    // 0x02 | 0x04
    ACT_DECIDE   = 0x00                                     // custom
};

enum CommandStates
{
    COMMAND_STAY    = 0,
    COMMAND_FOLLOW  = 1,
    COMMAND_ATTACK  = 2,
    COMMAND_DISMISS = 3
};

#define UNIT_ACTION_BUTTON_ACTION(X) (uint32(X) & 0x00FFFFFF)
#define UNIT_ACTION_BUTTON_TYPE(X)   ((uint32(X) & 0xFF000000) >> 24)
#define MAX_UNIT_ACTION_BUTTON_ACTION_VALUE (0x00FFFFFF+1)
#define MAKE_UNIT_ACTION_BUTTON(A,T) (uint32(A) | (uint32(T) << 24))

struct UnitActionBarEntry
{
    UnitActionBarEntry() : packedData(uint32(ACT_DISABLED) << 24) {}

    uint32 packedData;

    // helper
    ActiveStates GetType() const { return ActiveStates(UNIT_ACTION_BUTTON_TYPE(packedData)); }
    uint32 GetAction() const { return UNIT_ACTION_BUTTON_ACTION(packedData); }
    bool IsActionBarForSpell() const
    {
        ActiveStates Type = GetType();
        return Type == ACT_DISABLED || Type == ACT_ENABLED || Type == ACT_PASSIVE;
    }

    void SetActionAndType(uint32 action, ActiveStates type)
    {
        packedData = MAKE_UNIT_ACTION_BUTTON(action, type);
    }

    void SetType(ActiveStates type)
    {
        packedData = MAKE_UNIT_ACTION_BUTTON(UNIT_ACTION_BUTTON_ACTION(packedData), type);
    }

    void SetAction(uint32 action)
    {
        packedData = (packedData & 0xFF000000) | UNIT_ACTION_BUTTON_ACTION(action);
    }
};

typedef UnitActionBarEntry CharmSpellEntry;

enum ActionBarIndex
{
    ACTION_BAR_INDEX_START = 0,
    ACTION_BAR_INDEX_PET_SPELL_START = 3,
    ACTION_BAR_INDEX_PET_SPELL_END = 7,
    ACTION_BAR_INDEX_END = 10,
};

#define MAX_UNIT_ACTION_BAR_INDEX (ACTION_BAR_INDEX_END-ACTION_BAR_INDEX_START)

class CharmInfo
{
    public:
        explicit CharmInfo(Unit* unit);
        ~CharmInfo();

        void SetCharmState(std::string const& ainame, bool withNewThreatList = true);
        void ResetCharmState();
        uint32 GetPetNumber() const { return m_petnumber; }
        void SetPetNumber(uint32 petnumber, bool statwindow);

        void SetCommandState(CommandStates st);
        CommandStates GetCommandState() { return m_CommandState; }
        bool HasCommandState(CommandStates state) { return (m_CommandState == state); }

        void InitPossessCreateSpells();
        void InitCharmCreateSpells();
        void InitPetActionBar();
        void InitEmptyActionBar();

        // return true if successful
        bool AddSpellToActionBar(uint32 spellId, ActiveStates newstate = ACT_DECIDE, uint8 forceSlot = 255);
        bool RemoveSpellFromActionBar(uint32 spellId);
        void LoadPetActionBar(const std::string& data);
        void BuildActionBar(WorldPacket& data) const;
        void SetSpellAutocast(uint32 spell_id, bool state);
        void SetActionBar(uint8 index, uint32 spellOrAction, ActiveStates type)
        {
            PetActionBar[index].SetActionAndType(spellOrAction, type);
        }
        UnitActionBarEntry const* GetActionBarEntry(uint8 index) const { return &(PetActionBar[index]); }

        void ToggleCreatureAutocast(uint32 spellid, bool apply);

        CharmSpellEntry* GetCharmSpell(uint8 index) { return &(m_charmspells[index]); }

        void SetIsRetreating(bool retreating = false) { m_retreating = retreating; }
        bool GetIsRetreating() const { return m_retreating; }

        void SetStayPosition(bool stay = false);
        bool IsStayPosSet() const { return m_stayPosSet; }

        float GetStayPosX() const { return m_stayPosX; }
        float GetStayPosY() const { return m_stayPosY; }
        float GetStayPosZ() const { return m_stayPosZ; }
        float GetStayPosO() const { return m_stayPosO; }

        uint32 GetSpellOpener() const { return m_opener; }
        uint32 GetSpellOpenerMinRange() const { return m_openerMinRange; }
        uint32 GetSpellOpenerMaxRange() const { return m_openerMaxRange; }

        void SetSpellOpener(uint32 spellId = 0, uint32 minRange = 0, uint32 maxRange = 0)
        {
            m_opener = spellId;
            m_openerMinRange = minRange;
            m_openerMaxRange = maxRange;
        }

        UnitAI* GetAI() const { return m_ai; }
        CombatData* GetCombatData() const { return m_combatData; };

    private:
        Unit*               m_unit;
        UnitAI*             m_ai;
        CombatData*         m_combatData;
        UnitActionBarEntry  PetActionBar[MAX_UNIT_ACTION_BAR_INDEX];
        CharmSpellEntry     m_charmspells[CREATURE_MAX_SPELLS];
        CommandStates       m_CommandState;
        uint32              m_petnumber;
        uint32              m_opener;
        uint32              m_openerMinRange;
        uint32              m_openerMaxRange;
        uint32              m_unitFieldFlags;
        uint8               m_unitFieldBytes2_1;
        bool                m_retreating;
        bool                m_stayPosSet;
        float               m_stayPosX;
        float               m_stayPosY;
        float               m_stayPosZ;
        float               m_stayPosO;
};

// used in CallForAllControlledUnits/CheckAllControlledUnits
enum ControlledUnitMask
{
    CONTROLLED_PET       = 0x01,
    CONTROLLED_MINIPET   = 0x02,
    CONTROLLED_GUARDIANS = 0x04,                            // including PROTECTOR_PET
    CONTROLLED_CHARM     = 0x08,
    CONTROLLED_TOTEMS    = 0x10,
};

// for clearing special attacks
#define REACTIVE_TIMER_START 4000

enum ReactiveType
{
    REACTIVE_DEFENSE      = 1,
    REACTIVE_HUNTER_PARRY = 2,
    // REACTIVE_CRIT         = 3,
    // REACTIVE_HUNTER_CRIT  = 4,
    REACTIVE_OVERPOWER    = 5
};

#define MAX_REACTIVE 6

// delay time next attack to prevent client attack animation problems
#define ATTACK_DISPLAY_DELAY 200
#define MAX_PLAYER_STEALTH_DETECT_RANGE 45.0f               // max distance for detection targets by player
#define MAX_CREATURE_ATTACK_RADIUS 45.0f                    // max distance for creature aggro (use with CONFIG_FLOAT_RATE_CREATURE_AGGRO)

// Regeneration defines
#define REGEN_TIME_FULL     2000                            // For this time difference is computed regen value

// Power type values defines
enum PowerDefaults
{
    POWER_RAGE_DEFAULT              = 1000,
    POWER_FOCUS_DEFAULT             = 100,
    POWER_ENERGY_DEFAULT            = 100,
    POWER_HAPPINESS_DEFAULT         = 1000000,
};

enum EvadeState
{
    EVADE_NONE,
    EVADE_COMBAT, // In a dungeon in combat evades all hits until target becomes reachable
    EVADE_HOME, // When running home evades all hits and disables some AI actions
};

struct SpellProcEventEntry;                                 // used only privately

class Unit : public WorldObject
{
    public:
        typedef std::set<Unit*> AttackerSet;
        typedef std::multimap<uint32 /*spellId*/, SpellAuraHolder*> SpellAuraHolderMap;
        typedef std::pair<SpellAuraHolderMap::iterator, SpellAuraHolderMap::iterator> SpellAuraHolderBounds;
        typedef std::pair<SpellAuraHolderMap::const_iterator, SpellAuraHolderMap::const_iterator> SpellAuraHolderConstBounds;
        typedef std::list<SpellAuraHolder*> SpellAuraHolderList;
        typedef std::list<Aura*> AuraList;
        typedef std::list<DiminishingReturn> Diminishing;
        typedef std::set<uint32 /*playerGuidLow*/> ComboPointHolderSet;
        typedef std::map<SpellEntry const*, ObjectGuid /*targetGuid*/> TrackedAuraTargetMap;

        virtual ~Unit();

        void AddToWorld() override;
        void RemoveFromWorld() override;

        void CleanupsBeforeDelete() override;               // used in ~Creature/~Player (or before mass creature delete to remove cross-references to already deleted units)

        float GetObjectBoundingRadius() const override { return m_floatValues[UNIT_FIELD_BOUNDINGRADIUS]; } // overwrite WorldObject version
        float GetCombatReach() const override { return m_floatValues[UNIT_FIELD_COMBATREACH]; } // overwrite WorldObject version

        /**
         * Gets the current DiminishingLevels for the given group
         * @param group The group that you would like to know the current diminishing return level for
         * @return The current diminishing level, up to DIMINISHING_LEVEL_IMMUNE
         */
        DiminishingLevels GetDiminishing(DiminishingGroup  group);
        /**
         * Increases the level of the DiminishingGroup by one level up until
         * DIMINISHING_LEVEL_IMMUNE where the target becomes immune to spells of
         * that DiminishingGroup
         * @param group The group to increase the level for by one
         */
        void IncrDiminishing(DiminishingGroup group);
        /**
         * Calculates how long the duration of a spell should be considering
         * diminishing returns, ie, if the Level passed in is DIMINISHING_LEVEL_IMMUNE
         * then the duration will be zeroed out. If it is DIMINISHING_LEVEL_1 then a full
         * duration will be used
         * @param group The group to affect
         * @param duration The duration to be changed, will be updated with the new duration
         * @param caster Who's casting the spell, used to decide whether anything should be calculated
         * @param Level The current level of diminishing returns for the group, decides the new duration
         * @param isReflected Whether the spell was reflected or not, used to determine if we should do any calculations at all.
         */
        void ApplyDiminishingToDuration(DiminishingGroup  group, int32& duration, Unit* caster, DiminishingLevels Level, bool isReflected);
        /**
         * Applies a diminishing return to the given group if apply is true,
         * otherwise lowers the level by one (?)
         * @param group The group to affect
         * @param apply whether this aura is being added/removed
         */
        void ApplyDiminishingAura(DiminishingGroup  group, bool apply);
        /**
         * Clears all the current diminishing returns for this Unit.
         */
        void ClearDiminishings() { m_Diminishing.clear(); }

        void Update(const uint32 diff) override;

        /**
         * Updates the attack time for the given WeaponAttackType
         * @param type The type of weapon that we want to update the time for
         * @param time the remaining time until we can attack with the WeaponAttackType again
         */
        void setAttackTimer(WeaponAttackType type, uint32 time) { m_attackTimer[type] = time; }
        /**
         * Resets the attack timer to the base value decided by Unit::m_modAttackSpeedPct and
         * Unit::GetAttackTime
         * @param type The weapon attack type to reset the attack timer for.
         */
        void resetAttackTimer(WeaponAttackType type = BASE_ATTACK);
        /**
         * Get's the remaining time until we can do an attack
         * @param type The weapon type to check the remaining time for
         * @return The remaining time until we can attack with this weapon type.
         */
        uint32 getAttackTimer(WeaponAttackType type) const { return m_attackTimer[type]; }
        /**
         * Checks whether the unit can do an attack. Does this by checking the attacktimer for the
         * WeaponAttackType, can probably be thought of as a cooldown for each swing/shot
         * @param type What weapon should we check for
         * @return true if the Unit::m_attackTimer is zero for the given WeaponAttackType
         */
        bool isAttackReady(WeaponAttackType type = BASE_ATTACK) const { return m_attackTimer[type] == 0; }
        /**
         * Checks if the current Unit has a weapon equipped at the moment
         * @return True if there is a weapon.
         */
        virtual bool hasWeapon(WeaponAttackType type) const = 0;
        inline bool hasMainhandWeapon() const { return hasWeapon(BASE_ATTACK); }
        inline bool hasOffhandWeapon() const { return hasWeapon(OFF_ATTACK); }
        inline bool hasRangedWeapon() const { return hasWeapon(RANGED_ATTACK); }
        /**
         * Checks if the current Unit has a usable weapon at the moment
         * @return True if there is a weapon.
         */
        virtual bool hasWeaponForAttack(WeaponAttackType type) const { return CanUseEquippedWeapon(type); }
        inline bool hasMainhandWeaponForAttack() const { return hasWeaponForAttack(BASE_ATTACK); }
        inline bool hasOffhandWeaponForAttack() const { return hasWeaponForAttack(OFF_ATTACK); }
        inline bool hasRangedWeaponForAttack() const { return hasWeaponForAttack(RANGED_ATTACK); }
         /**
         * Does an attack if any of the timers allow it and resets them, if the user
         * isn't in range or behind the target an error is sent to the client.
         * Also makes sure to not make and offhand and mainhand attack at the same
         * time. Only handles non-spells ie melee attacks.
         * @return True if an attack was made and no error happened, false otherwise
         */
        bool UpdateMeleeAttackingState();
        /**
         * Check is a given equipped weapon can be used, ie the mainhand, offhand etc.
         * @param attackType The attack type to check, ie: main/offhand/ranged
         * @return True if the weapon can be used, true except for shapeshifts and if disarmed.
         */
        bool CanUseEquippedWeapon(WeaponAttackType attackType) const
        {
            if (IsInFeralForm())
                return false;

            switch (attackType)
            {
                default:
                case BASE_ATTACK:
                    return !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);
                case OFF_ATTACK:
                case RANGED_ATTACK:
                    return true;
            }
        }
        /** Returns if the Unit can reach a victim with Melee Attack
         *
         * @param pVictim Who we want to reach with a melee attack
         * @param flat_mod The same as sent to Unit::GetCombatReach
         * @return true if we can reach pVictim with a melee attack
         */
        bool CanReachWithMeleeAttack(Unit const* pVictim, float flat_mod = 0.0f) const;
        uint32 m_extraAttacks;
        void DoExtraAttacks(Unit* pVictim);

        bool IsAttackedBy(Unit* attacker) const
        {
            if (m_attackers.find(attacker) != m_attackers.end())
                return true;

            return false;
        }

        bool _addAttacker(Unit* pAttacker)                  //< (Internal Use) must be called only from Unit::Attack(Unit*)
        {
            AttackerSet::const_iterator itr = m_attackers.find(pAttacker);
            if (itr == m_attackers.end())
            {
                m_attackers.insert(pAttacker);
                return true;
            }
            return false;
        }
        void _removeAttacker(Unit* pAttacker)               //< (Internal Use) must be called only from Unit::AttackStop()
        {
            m_attackers.erase(pAttacker);
        }
        Unit* getAttackerForHelper()                        //< Return a possible enemy from this unit to help in combat
        {
            if (getVictim() != nullptr)
                return getVictim();

            if (!m_attackers.empty())
                return *(m_attackers.begin());

            return nullptr;
        }
        /**
         * Tries to attack a Unit/Player, also makes sure to stop attacking the current target
         * if we're already attacking someone.
         * @param victim The Unit to attack
         * @param meleeAttack Whether we should attack with melee or ranged/magic
         * @return True if an attack was initiated, false otherwise
         */
        bool Attack(Unit* victim, bool meleeAttack);
        /**
         * Called when we are attacked by someone in someway, might be when a fear runs out and
         * we want to notify AI to attack again or when a spell hits.
         * @param attacker Who's attacking us
         */
        void AttackedBy(Unit* attacker);
        /**
         * Stop all spells from casting except the one give by except_spellid
         * @param except_spellid This spell id will not be stopped from casting, defaults to 0
         * \see Unit::InterruptSpell
         */
        void CastStop(uint32 except_spellid = 0);
        /**
         * Stops attacking whatever we are attacking at the moment and tells the Unit we are attacking
         * that we are not doing that anymore.
         * @param targetSwitch if we are switching targets or not, defaults to false
         * @return false if we weren't attacking already, true otherwise
         * \see Unit::m_attacking
         */
        virtual bool AttackStop(bool targetSwitch = false, bool includingCast = false, bool includingCombo = false);
        /**
         * Removes all attackers from the Unit::m_attackers set and logs it if someone that
         * wasn't attacking it was in the list. Does this check by checking if Unit::AttackStop()
         * returned false.
         * \see Unit::AttackStop
         */
        void RemoveAllAttackers();

        void MeleeAttackStart(Unit* victim);
        void MeleeAttackStop(Unit* victim);

        /// Returns the Unit::m_attackers, that stores the units that are attacking you
        AttackerSet const& getAttackers() const { return m_attackers; }

        Unit* getVictim() const { return m_attacking; }     //< Returns the victim that this unit is currently attacking
        void CombatStop(bool includingCast = false, bool includingCombo = true);        //< Stop this unit from combat, if includingCast==true, also interrupt casting
        void CombatStopWithPets(bool includingCast = false, bool includingCombo = true);
        void StopAttackFaction(uint32 faction_id);
        Unit* SelectRandomUnfriendlyTarget(Unit* except = nullptr, float radius = ATTACK_DISTANCE) const;
        Unit* SelectRandomFriendlyTarget(Unit* except = nullptr, float radius = ATTACK_DISTANCE) const;
        bool HasDamageInterruptibleStunAura() const;
        void SendMeleeAttackStop(Unit* victim) const;
        void SendMeleeAttackStart(Unit* pVictim) const;

        void addUnitState(uint32 f) { m_state |= f; }
        bool hasUnitState(uint32 f) const { return (m_state & f) != 0; }
        void clearUnitState(uint32 f) { m_state &= ~f; }
        bool CanFreeMove() const { return !hasUnitState(UNIT_STAT_NO_FREE_MOVE) && !GetOwnerGuid(); }

        virtual uint32 GetLevelForTarget(Unit const* /*target*/) const { return getLevel(); }
        bool IsTrivialForTarget(Unit const* pov) const;

        void SetLevel(uint32 lvl);

        uint32 getLevel() const { return GetUInt32Value(UNIT_FIELD_LEVEL); }
        uint8 getRace() const { return GetByteValue(UNIT_FIELD_BYTES_0, 0); }
        uint32 getRaceMask() const { return 1 << (getRace() - 1); }
        uint8 getClass() const { return GetByteValue(UNIT_FIELD_BYTES_0, 1); }
        uint32 getClassMask() const { return 1 << (getClass() - 1); }
        uint8 getGender() const { return GetByteValue(UNIT_FIELD_BYTES_0, 2); }

        float GetStat(Stats stat) const { return float(GetUInt32Value(UNIT_FIELD_STAT0 + stat)); }
        void SetStat(Stats stat, int32 val) { SetStatInt32Value(UNIT_FIELD_STAT0 + stat, val); }

        inline int32 GetArmor() const { return GetResistance(SPELL_SCHOOL_NORMAL) ; }
        inline void SetArmor(int32 val) { SetStatInt32Value(UNIT_FIELD_RESISTANCES, val); }

        inline int32 GetResistance(SpellSchools school) const { return GetInt32Value(UNIT_FIELD_RESISTANCES + school); }
        inline void SetResistance(SpellSchools school, int32 val) { SetInt32Value(UNIT_FIELD_RESISTANCES + school, val); }

        uint32 GetHealth()    const { return GetUInt32Value(UNIT_FIELD_HEALTH); }
        uint32 GetMaxHealth() const { return GetUInt32Value(UNIT_FIELD_MAXHEALTH); }
        float GetHealthPercent() const { return (GetHealth() * 100.0f) / GetMaxHealth(); }
        void SetHealth(uint32 val);
        void SetMaxHealth(uint32 val);
        void SetHealthPercent(float percent);
        int32 ModifyHealth(int32 dVal);
        float OCTRegenHPPerSpirit() const;
        float OCTRegenMPPerSpirit() const;

        Powers GetPowerType() const { return Powers(GetByteValue(UNIT_FIELD_BYTES_0, 3)); }
        void SetPowerType(Powers new_powertype);
        uint32 GetPower(Powers power) const { return GetUInt32Value(UNIT_FIELD_POWER1 + power); }
        uint32 GetMaxPower(Powers power) const { return GetUInt32Value(UNIT_FIELD_MAXPOWER1 + power); }
        float GetPowerPercent() const { return (GetMaxPower(GetPowerType()) == 0) ? 0.0f : (GetPower(GetPowerType()) * 100.0f) / GetMaxPower(GetPowerType()); }
        void SetPower(Powers power, uint32 val);
        void SetMaxPower(Powers power, uint32 val);
        int32 ModifyPower(Powers power, int32 dVal);
        void ApplyPowerMod(Powers power, uint32 val, bool apply);
        void ApplyMaxPowerMod(Powers power, uint32 val, bool apply);
        bool HasMana() { return GetPowerType() == POWER_MANA; }

        uint32 GetAttackTime(WeaponAttackType att) const { return (uint32)(GetFloatValue(UNIT_FIELD_BASEATTACKTIME + att) / m_modAttackSpeedPct[att]); }
        void SetAttackTime(WeaponAttackType att, uint32 val) { SetFloatValue(UNIT_FIELD_BASEATTACKTIME + att, val * m_modAttackSpeedPct[att]); }
        void ApplyAttackTimePercentMod(WeaponAttackType att, float val, bool apply);
        void ApplyCastTimePercentMod(float val, bool apply);

        SheathState GetSheath() const { return SheathState(GetByteValue(UNIT_FIELD_BYTES_2, 0)); }
        virtual void SetSheath(SheathState sheathed) { SetByteValue(UNIT_FIELD_BYTES_2, 0, sheathed); }

        // faction template id
        uint32 getFaction() const { return GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE); }
        void setFaction(uint32 faction) { SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, faction); }
        FactionTemplateEntry const* GetFactionTemplateEntry() const;
        void RestoreOriginalFaction();
        bool IsNeutralToAll() const;
        bool IsContestedGuard() const
        {
            if (FactionTemplateEntry const* entry = GetFactionTemplateEntry())
                return entry->IsContestedGuardFaction();

            return false;
        }

        Player const* GetControllingPlayer(bool ignoreCharms = false) const;

        ReputationRank GetReactionTo(Unit const* unit) const override;
        ReputationRank GetReactionTo(Corpse const* corpse) const override;

        bool IsEnemy(Unit const* unit) const override;
        bool IsFriend(Unit const* unit) const override;

        bool CanAssist(Unit const* unit, bool ignoreFlags = false) const;
        bool CanAssist(Corpse const* corpse) const;

        bool CanAttack(Unit const* unit) const;
        bool CanAttackNow(Unit const* unit) const;

        bool CanCooperate(Unit const* unit) const;

        bool CanInteract(GameObject const* object) const;
        bool CanInteract(Unit const* unit) const;
        bool CanInteractNow(Unit const* unit) const;

        bool IsCivilianForTarget(Unit const* pov) const;

        // Serverside fog of war settings
        bool IsFogOfWarVisibleStealth(Unit const* other) const;
        bool IsFogOfWarVisibleHealth(Unit const* other) const;
        bool IsFogOfWarVisibleStats(Unit const* other) const;

        bool IsInGroup(Unit const* other, bool party = false, bool ignoreCharms = false) const;
        inline bool IsInParty(Unit const* other, bool ignoreCharms = false) const { return IsInGroup(other, true, ignoreCharms); }
        bool IsInGuild(Unit const* other, bool ignoreCharms = false) const;
        bool IsInTeam(Unit const* other, bool ignoreCharms = true) const;

        // extensions of CanAttack and CanAssist API needed serverside
        virtual bool CanAttackSpell(Unit const* target, SpellEntry const* spellInfo = nullptr, bool isAOE = false) const override;
        virtual bool CanAssistSpell(Unit const* target, SpellEntry const* spellInfo = nullptr) const override;

        bool CanAttackOnSight(Unit const* target) const; // Used in MoveInLineOfSight checks
        bool CanAssistInCombatAgainst(Unit const* who, Unit const* enemy) const;

        bool IsImmuneToNPC() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC); }
        void SetImmuneToNPC(bool state);
        bool IsImmuneToPlayer() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER); }
        void SetImmuneToPlayer(bool state);

        bool IsPvP() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); }
        void SetPvP(bool state);
        bool IsPvPFreeForAll() const;
        void SetPvPFreeForAll(bool state);
        bool IsPvPContested() const;
        void SetPvPContested(bool state);
        uint32 GetCreatureType() const;
        uint32 GetCreatureTypeMask() const
        {
            uint32 creatureType = GetCreatureType();
            return (creatureType >= 1) ? (1 << (creatureType - 1)) : 0;
        }

        uint8 getStandState() const { return GetByteValue(UNIT_FIELD_BYTES_1, 0); }
        bool IsSitState() const;
        bool IsStandState() const;
        bool IsSeatedState() const;
        void SetStandState(uint8 state, bool acknowledge = false);

        bool IsMounted() const { return !!GetMountID(); }
        uint32 GetMountID() const { return GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID); }
        void Mount(uint32 mount, uint32 spellId = 0);
        void Unmount(bool from_aura = false);

        uint16 GetSkillMaxForLevel(Unit const* target = nullptr) const { return (target ? GetLevelForTarget(target) : getLevel()) * 5; }

        void Suicide();
        void DealDamageMods(Unit* pVictim, uint32& damage, uint32* absorb, DamageEffectType damagetype, SpellEntry const* spellProto = nullptr);
        uint32 DealDamage(Unit* pVictim, uint32 damage, CleanDamage const* cleanDamage, DamageEffectType damagetype, SpellSchoolMask damageSchoolMask, SpellEntry const* spellProto, bool durabilityLoss);
        int32 DealHeal(Unit* pVictim, uint32 addhealth, SpellEntry const* spellProto, bool critical = false);
        void Kill(Unit* victim, DamageEffectType damagetype, SpellEntry const* spellProto, bool durabilityLoss, bool duel_hasEnded);
        void HandleDamageDealt(Unit* victim, uint32& damage, CleanDamage const* cleanDamage, DamageEffectType damagetype, SpellSchoolMask damageSchoolMask, SpellEntry const* spellProto, bool duel_hasEnded);
        void InterruptOrDelaySpell(Unit* pVictim, DamageEffectType damagetype);

        void PetOwnerKilledUnit(Unit* pVictim);

        void ProcDamageAndSpell(ProcSystemArguments&& data);
        void ProcDamageAndSpellFor(ProcSystemArguments& data, bool isVictim);
        void ProcSkillsAndReactives(bool isVictim, Unit* target, uint32 procFlags, uint32 procEx, WeaponAttackType attType);

        void HandleEmote(uint32 emote_id);                  // auto-select command/state
        void HandleEmoteCommand(uint32 emote_id);
        void HandleEmoteState(uint32 emote_id);
        void AttackerStateUpdate(Unit* pVictim, WeaponAttackType attType = BASE_ATTACK, bool extra = false);

        void CalculateMeleeDamage(Unit* pVictim, CalcDamageInfo* calcDamageInfo, WeaponAttackType attackType = BASE_ATTACK);
        void DealMeleeDamage(CalcDamageInfo* calcDamageInfo, bool durabilityLoss);

        void CalculateSpellDamage(SpellNonMeleeDamage* spellDamageInfo, int32 damage, SpellEntry const* spellInfo, WeaponAttackType attackType = BASE_ATTACK);
        void DealSpellDamage(SpellNonMeleeDamage* damageInfo, bool durabilityLoss);

        SpellMissInfo MeleeSpellHitResult(Unit* pVictim, SpellEntry const* spell);
        SpellMissInfo MagicSpellHitResult(Unit* pVictim, SpellEntry const* spell, SpellSchoolMask schoolMask);
        SpellMissInfo SpellHitResult(Unit* pVictim, SpellEntry const* spell, uint8 effectMask, bool reflectable = false);

        bool CanDualWield() const { return m_canDualWield; }
        void SetCanDualWield(bool value) { m_canDualWield = value; }

        // Unit Combat reactions API: Dodge/Parry/Block
        bool CanDodge() const { return m_canDodge; }
        bool CanParry() const { return m_canParry; }
        bool CanBlock() const { return m_canBlock; }
        // Unit Melee events API: Crush/Glance/Daze
        bool CanCrush() const;
        bool CanGlance() const;
        bool CanDaze() const;

        void SetCanDodge(const bool flag);
        void SetCanParry(const bool flag);
        void SetCanBlock(const bool flag);

        bool CanReactInCombat() const { return (isAlive() && !IsIncapacitated() && !IsEvadingHome()); }
        bool CanDodgeInCombat() const;
        bool CanDodgeInCombat(const Unit* attacker) const;
        bool CanParryInCombat() const;
        bool CanParryInCombat(const Unit* attacker) const;
        bool CanBlockInCombat(SpellSchoolMask weaponSchoolMask = SPELL_SCHOOL_MASK_NORMAL) const;
        bool CanBlockInCombat(const Unit* attacker, SpellSchoolMask weaponSchoolMask = SPELL_SCHOOL_MASK_NORMAL) const;
        bool CanCrushInCombat() const;
        bool CanCrushInCombat(const Unit* victim) const;
        bool CanGlanceInCombat() const { return CanGlance(); }
        bool CanGlanceInCombat(const Unit* victim) const;
        bool CanDazeInCombat() const { return CanDaze(); }
        bool CanDazeInCombat(const Unit* victim) const;

        // Unit Combat Reactions API for abilities (melee and ranged spells): Dodge/Parry/Block/Deflect
        bool CanReactOnAbility(const SpellEntry* ability) const;
        bool CanDodgeAbility(const Unit* attacker, const SpellEntry* ability) const;
        bool CanParryAbility(const Unit* attacker, const SpellEntry* ability) const;
        bool CanBlockAbility(const Unit* attacker, const SpellEntry* ability, bool miss = false) const;
        bool CanDeflectAbility(const Unit* attacker, const SpellEntry* ability) const;

        float CalculateEffectiveDodgeChance(const Unit* attacker, WeaponAttackType attType, const SpellEntry* ability = nullptr) const;
        float CalculateEffectiveParryChance(const Unit* attacker, WeaponAttackType attType, const SpellEntry* ability = nullptr) const;
        float CalculateEffectiveBlockChance(const Unit* attacker, WeaponAttackType attType, const SpellEntry* ability = nullptr) const;
        float CalculateEffectiveCrushChance(const Unit* victim, WeaponAttackType attType) const;
        float CalculateEffectiveGlanceChance(const Unit* victim, WeaponAttackType attType) const;
        float CalculateEffectiveDazeChance(const Unit* victim, WeaponAttackType attType) const;
        uint32 CalculateGlanceAmount(CalcDamageInfo* meleeInfo) const;

        float CalculateAbilityDeflectChance(const Unit* attacker, const SpellEntry* ability) const;

        bool RollAbilityPartialBlockOutcome(const Unit* attacker, WeaponAttackType attType, const SpellEntry* ability) const;

        float GetDodgeChance() const;
        float GetParryChance() const;
        float GetBlockChance() const;

        float GetReflectChance(SpellSchoolMask schoolMask) const;

        virtual bool CanCrit(WeaponAttackType attType) const { return (GetCritChance(attType) >= 0.005f); }
        virtual bool CanCrit(SpellSchoolMask schoolMask) const { return (GetCritChance(schoolMask) >= 0.005f); }
        virtual bool CanCrit(const SpellEntry* entry, SpellSchoolMask schoolMask, WeaponAttackType attType) const;

        virtual float GetCritChance(WeaponAttackType attType) const;
        virtual float GetCritChance(SpellSchoolMask schoolMask) const;
        virtual float GetCritChance(const SpellEntry* entry, SpellSchoolMask schoolMask) const;

        virtual float GetCritTakenChance(SpellSchoolMask dmgSchoolMask, SpellDmgClass dmgClass, bool heal = false) const;

        virtual float GetCritMultiplier(SpellSchoolMask dmgSchoolMask, uint32 creatureTypeMask, const SpellEntry* spell = nullptr, bool heal = false) const;
        virtual float GetCritTakenMultiplier(SpellSchoolMask dmgSchoolMask, SpellDmgClass dmgClass, float ignorePct = 0.0f, bool heal = false) const;
        uint32 CalculateCritAmount(const Unit* victim, uint32 amount, const SpellEntry* entry, bool heal = false) const;
        uint32 CalculateCritAmount(CalcDamageInfo* meleeInfo) const;

        virtual float GetHitChance(WeaponAttackType attackType) const;
        virtual float GetHitChance(SpellSchoolMask schoolMask) const;
        virtual float GetHitChance(const SpellEntry* entry, SpellSchoolMask schoolMask) const;

        virtual float GetMissChance(WeaponAttackType attackType) const;
        virtual float GetMissChance(SpellSchoolMask schoolMask) const;
        virtual float GetMissChance(const SpellEntry* entry, SpellSchoolMask schoolMask) const;

        float CalculateEffectiveCritChance(const Unit* victim, WeaponAttackType attType, const SpellEntry* ability = nullptr) const;
        float CalculateEffectiveMissChance(const Unit* victim, WeaponAttackType attType, const SpellEntry* ability = nullptr) const;

        float CalculateSpellCritChance(const Unit* victim, SpellSchoolMask schoolMask, const SpellEntry* spell) const;
        float CalculateSpellMissChance(const Unit* victim, SpellSchoolMask schoolMask, const SpellEntry* spell) const;

        bool RollSpellCritOutcome(const Unit* victim, SpellSchoolMask schoolMask, const SpellEntry* spell) const;

        virtual int32 GetResistancePenetration(SpellSchools school) const;

        float CalculateEffectiveMagicResistancePercent(const Unit* attacker, SpellSchoolMask schoolMask, bool binary = false) const;
        float RollMagicResistanceMultiplierOutcomeAgainst(const Unit* victim, SpellSchoolMask schoolMask, DamageEffectType dmgType, bool binary = false) const;

        float CalculateSpellResistChance(const Unit* victim, SpellSchoolMask schoolMask, const SpellEntry* spell) const;

        virtual uint32 GetShieldBlockValue() const = 0;
        uint32 GetUnitMeleeSkill(Unit const* target = nullptr) const { return (target ? GetLevelForTarget(target) : getLevel()) * 5; }
        uint32 GetDefenseSkillValue(Unit const* target = nullptr) const;
        uint32 GetWeaponSkillValue(WeaponAttackType attType, Unit const* target = nullptr) const;
        float GetWeaponProcChance() const;
        float GetPPMProcChance(uint32 WeaponSpeed, float PPM) const;

        MeleeHitOutcome RollMeleeOutcomeAgainst(const Unit* pVictim, WeaponAttackType attType, SpellSchoolMask schoolMask) const;

        bool isVendor()       const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR); }
        bool isTrainer()      const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER); }
        bool isQuestGiver()   const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER); }
        bool isGossip()       const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP); }
        bool isTaxi()         const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_FLIGHTMASTER); }
        bool isGuildMaster()  const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_PETITIONER); }
        bool isBattleMaster() const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BATTLEMASTER); }
        bool isBanker()       const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BANKER); }
        bool isInnkeeper()    const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER); }
        bool isSpiritHealer() const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER); }
        bool isSpiritGuide()  const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITGUIDE); }
        bool isTabardDesigner()const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TABARDDESIGNER); }
        bool isAuctioner()    const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_AUCTIONEER); }
        bool isArmorer()      const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_REPAIR); }
        bool isServiceProvider() const
        {
            return HasFlag(UNIT_NPC_FLAGS,
                           UNIT_NPC_FLAG_VENDOR | UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_FLIGHTMASTER |
                           UNIT_NPC_FLAG_PETITIONER | UNIT_NPC_FLAG_BATTLEMASTER | UNIT_NPC_FLAG_BANKER |
                           UNIT_NPC_FLAG_INNKEEPER | UNIT_NPC_FLAG_SPIRITHEALER |
                           UNIT_NPC_FLAG_SPIRITGUIDE | UNIT_NPC_FLAG_TABARDDESIGNER | UNIT_NPC_FLAG_AUCTIONEER);
        }
        bool isSpiritService() const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER | UNIT_NPC_FLAG_SPIRITGUIDE); }

        bool IsTaxiFlying()  const { return hasUnitState(UNIT_STAT_TAXI_FLIGHT); }

        bool isInCombat()  const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT); }
        void SetInCombatState(bool PvP, Unit* enemy = nullptr);
        void SetInDummyCombatState(bool state);
        void SetInCombatWith(Unit* enemy);
        void SetInCombatWithAggressor(Unit* aggressor, bool touchOnly = false);
        inline void SetOutOfCombatWithAggressor(Unit* aggressor) { SetInCombatWithAggressor(aggressor, true); }
        void SetInCombatWithAssisted(Unit* assisted, bool touchOnly = false);
        inline void SetOutOfCombatWithAssisted(Unit* assisted) { SetInCombatWithAssisted(assisted, true); }
        void SetInCombatWithVictim(Unit* victim, bool touchOnly = false);
        inline void SetOutOfCombatWithVictim(Unit* victim) { SetInCombatWithVictim(victim, true); }
        void ClearInCombat();
        uint32 GetCombatTimer() const { return m_CombatTimer; }

        SpellAuraHolderBounds GetSpellAuraHolderBounds(uint32 spell_id)
        {
            return m_spellAuraHolders.equal_range(spell_id);
        }
        SpellAuraHolderConstBounds GetSpellAuraHolderBounds(uint32 spell_id) const
        {
            return m_spellAuraHolders.equal_range(spell_id);
        }

        uint32 GetAuraCount(uint32 spellId) const;

        bool HasAuraType(AuraType auraType) const;
        bool HasAffectedAura(AuraType auraType, SpellEntry const* spellProto) const;
        bool HasAura(uint32 spellId, SpellEffectIndex effIndex) const;
        bool HasAura(uint32 spellId) const
        {
            return m_spellAuraHolders.find(spellId) != m_spellAuraHolders.end();
        }
        bool HasAuraTypeWithCaster(AuraType auratype, ObjectGuid caster) const;

        virtual bool HasSpell(uint32 /*spellID*/) const { return false; }

        bool HasStealthAura()      const { return HasAuraType(SPELL_AURA_MOD_STEALTH); }
        bool HasInvisibilityAura() const { return HasAuraType(SPELL_AURA_MOD_INVISIBILITY); }
        bool isFeared()  const { return HasAuraType(SPELL_AURA_MOD_FEAR); }
        bool isInRoots() const { return HasAuraType(SPELL_AURA_MOD_ROOT); }
        bool IsPolymorphed() const;

        bool isFrozen() const;

        virtual bool IsInWater() const;
        bool IsInSwimmableWater() const;
        virtual bool IsUnderwater() const;
        bool isInAccessablePlaceFor(Unit const* unit) const;

        void EnergizeBySpell(Unit* pVictim, uint32 SpellID, uint32 Damage, Powers powertype);
        uint32 SpellNonMeleeDamageLog(Unit* pVictim, uint32 spellID, uint32 damage);

        SpellCastResult CastSpell(Unit* Victim, uint32 spellId, uint32 triggeredFlags, Item* castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr);
        SpellCastResult CastSpell(Unit* Victim, SpellEntry const* spellInfo, uint32 triggeredFlags, Item* castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr);
        SpellCastResult CastCustomSpell(Unit* Victim, uint32 spellId, int32 const* bp0, int32 const* bp1, int32 const* bp2, uint32 triggeredFlags, Item* castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr);
        SpellCastResult CastCustomSpell(Unit* Victim, SpellEntry const* spellInfo, int32 const* bp0, int32 const* bp1, int32 const* bp2, uint32 triggeredFlags, Item* castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr);
        SpellCastResult CastSpell(float x, float y, float z, uint32 spellId, uint32 triggeredFlags, Item* castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr);
        SpellCastResult CastSpell(float x, float y, float z, SpellEntry const* spellInfo, uint32 triggeredFlags, Item* castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr);
        SpellCastResult CastSpell(SpellCastTargets& targets, SpellEntry const* spellInfo, uint32 triggeredFlags, Item* castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr);

        // Single flag overload uint32
        SpellCastResult CastSpell(Unit* Victim, uint32 spellId, TriggerCastFlags triggeredFlags, Item* castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr)
        {
            return CastSpell(Victim, spellId, uint32(triggeredFlags), castItem, triggeredByAura, originalCaster, triggeredBy);
        }
        SpellCastResult CastCustomSpell(Unit* Victim, uint32 spellId, int32 const* bp0, int32 const* bp1, int32 const* bp2, TriggerCastFlags triggeredFlags, Item* castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr)
        {
            return CastCustomSpell(Victim, spellId, bp0, bp1, bp2, uint32(triggeredFlags), castItem, triggeredByAura, originalCaster, triggeredBy);
        }
        SpellCastResult CastSpell(float x, float y, float z, uint32 spellId, TriggerCastFlags triggeredFlags, Item* castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr)
        {
            return CastSpell(x, y, z, spellId, uint32(triggeredFlags), castItem, triggeredByAura, originalCaster, triggeredBy);
        }

        // Single flag overload SpellEntry
        SpellCastResult CastSpell(Unit* Victim, SpellEntry const* spellInfo, TriggerCastFlags triggeredFlags, Item* castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr)
        {
            return CastSpell(Victim, spellInfo, uint32(triggeredFlags), castItem, triggeredByAura, originalCaster, triggeredBy);
        }
        SpellCastResult CastCustomSpell(Unit* Victim, SpellEntry const* spellInfo, int32 const* bp0, int32 const* bp1, int32 const* bp2, TriggerCastFlags triggeredFlags, Item* castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr)
        {
            return CastCustomSpell(Victim, spellInfo, bp0, bp1, bp2, uint32(triggeredFlags), castItem, triggeredByAura, originalCaster, triggeredBy);
        }
        SpellCastResult CastSpell(float x, float y, float z, SpellEntry const* spellInfo, TriggerCastFlags triggeredFlags, Item* castItem = nullptr, Aura* triggeredByAura = nullptr, ObjectGuid originalCaster = ObjectGuid(), SpellEntry const* triggeredBy = nullptr)
        {
            return CastSpell(x, y, z, spellInfo, uint32(triggeredFlags), castItem, triggeredByAura, originalCaster, triggeredBy);
        }

        void DeMorph();

        void SendAIReaction(AiReaction reactionType);

        void SendAttackStateUpdate(CalcDamageInfo* calcDamageInfo) const;
        void SendAttackStateUpdate(uint32 HitInfo, Unit* target, SpellSchoolMask damageSchoolMask, uint32 Damage, uint32 AbsorbDamage, int32 Resist, VictimState TargetState, uint32 BlockedAmount);
        void SendEnergizeSpellLog(Unit* pVictim, uint32 SpellID, uint32 Damage, Powers powertype) const;
        void SendEnvironmentalDamageLog(uint8 type, uint32 damage, uint32 absorb, int32 resist) const;
        void SendHealSpellLog(Unit* pVictim, uint32 SpellID, uint32 Damage, bool critical = false);
        void SendSpellNonMeleeDamageLog(SpellNonMeleeDamage* log) const;
        void SendSpellNonMeleeDamageLog(Unit* target, uint32 spellID, uint32 damage, SpellSchoolMask damageSchoolMask, uint32 absorbedDamage, int32 resist, bool isPeriodic, uint32 blocked, bool criticalHit = false, bool split = false);
        void SendPeriodicAuraLog(SpellPeriodicAuraLogInfo* pInfo) const;
        void SendSpellMiss(Unit* target, uint32 spellID, SpellMissInfo missInfo) const;
        void SendSpellDamageResist(Unit* target, uint32 spellId) const;
        void SendSpellOrDamageImmune(Unit* target, uint32 spellID) const;

        void SendEnchantmentLog(ObjectGuid targetGuid, uint32 itemEntry, uint32 enchantId) const;

        void CasterHitTargetWithSpell(Unit* realCaster, Unit* target, SpellEntry const* spellInfo, bool success = true);
        bool CanInitiateAttack() const;

        void NearTeleportTo(float x, float y, float z, float orientation, bool casting = false);
        void MonsterMoveWithSpeed(float x, float y, float z, float speed, bool generatePath = false, bool forceDestination = false);
        // recommend use MonsterMove/MonsterMoveWithSpeed for most case that correctly work with movegens
        // if used additional args in ... part then floats must explicitly casted to double
        void SendHeartBeat();

        bool IsMoving() const { return m_movementInfo.HasMovementFlag(movementFlagsMask); }
        bool IsMovingForward() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_MASK_MOVING_FORWARD); }
        bool IsLevitating() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_LEVITATING); }
        bool IsWalking() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_WALK_MODE); }
        bool IsRooted() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_ROOT); }

        virtual void SetLevitate(bool /*enabled*/) {}
        virtual void SetSwim(bool /*enabled*/) {}
        virtual void SetCanFly(bool /*enabled*/) {}
        virtual void SetFeatherFall(bool /*enabled*/) {}
        virtual void SetHover(bool /*enabled*/) {}
        virtual void SetRoot(bool /*enabled*/) {}
        virtual void SetWaterWalk(bool /*enabled*/) {}

        void SetInFront(Unit const* target);
        void SetFacingTo(float ori);
        void SetFacingToObject(WorldObject* pObject);

        bool isAlive() const { return (m_deathState == ALIVE); }
        bool isDead() const { return (m_deathState == DEAD || m_deathState == CORPSE); }
        DeathState getDeathState() const { return m_deathState; }
        virtual void SetDeathState(DeathState s);           // overwritten in Creature/Player/Pet

        bool IsTargetUnderControl(Unit const& target) const;

        // Convenience checkers/getters/setters counterparts for some of the protected Unit guid fields
        // See the comments next to protected methods for meanings
        bool HasCharm(ObjectGuid const& exactGuid = ObjectGuid()) const { return (exactGuid.IsEmpty() ? GetCharmGuid().IsUnit() : (GetCharmGuid() == exactGuid)); }
        bool HasCharmer(ObjectGuid const& exactGuid = ObjectGuid()) const { return (exactGuid.IsEmpty() ? GetCharmerGuid().IsUnit() : (GetCharmerGuid() == exactGuid)); }
        bool HasTarget(ObjectGuid const& exactGuid = ObjectGuid()) const { return (exactGuid.IsEmpty() ? GetTargetGuid().IsUnit() : (GetTargetGuid() == exactGuid)); }
        bool HasChannelObject(ObjectGuid const& exactGuid = ObjectGuid()) const { return (exactGuid.IsEmpty() ? !(GetChannelObjectGuid().IsEmpty()) : (GetChannelObjectGuid() == exactGuid)); }

        Unit* GetCharm(WorldObject const* pov = nullptr) const;
        Unit* GetCharmer(WorldObject const* pov = nullptr) const;
        Unit* GetCreator(WorldObject const* pov = nullptr) const;
        Unit* GetTarget(WorldObject const* pov = nullptr) const;
        Unit* GetChannelObject(WorldObject const* pov = nullptr) const;

        void SetCharm(Unit* charmed) { SetCharmGuid(charmed ? charmed->GetObjectGuid() : ObjectGuid()); }
        void SetCharmer(Unit* charmer) { SetCharmerGuid(charmer ? charmer->GetObjectGuid() : ObjectGuid()); }
        void SetTarget(WorldObject* target) { SetTargetGuid(target ? target->GetObjectGuid() : ObjectGuid()); }
        void SetChannelObject(WorldObject* object) { SetChannelObjectGuid(object ? object->GetObjectGuid() : ObjectGuid()); }

        // Purely logical guid constructs getters/setters
        // Owner: automatically resolves to creator guid and additionally to summoner guid
        ObjectGuid const& GetOwnerGuid() const override { return GetCreatorGuid(); }
        void SetOwnerGuid(ObjectGuid owner) override { SetCreatorGuid(owner); }
        // Pet: automatically resolves to summon guid (permanent pet)
        ObjectGuid const& GetPetGuid() const { return GetSummonGuid(); }
        void SetPetGuid(ObjectGuid pet) { SetSummonGuid(pet); }
        // Selection: by default resolves to target, overriden in Player
        virtual ObjectGuid const& GetSelectionGuid() const { return GetTargetGuid(); }
        virtual void SetSelectionGuid(ObjectGuid guid) { SetTargetGuid(guid); }
        // Master: automatically resolves to charmer or owner guid
        ObjectGuid const& GetMasterGuid() const { ObjectGuid const& guid = GetCharmerGuid(); return (guid ? guid : GetOwnerGuid()); }
        // Spawner: guid of a unit, who is reponsible for starting this unit's parent script (only for script-spawned units within Unit hierarchy)
        virtual ObjectGuid const GetSpawnerGuid() const { return ObjectGuid(); }

        // Convenience unit getters for some of the logical guid constructs above
        Unit* GetOwner(WorldObject const* pov = nullptr, bool recursive = false) const;
        Unit* GetMaster(WorldObject const* pov = nullptr) const;
        Unit* GetSpawner(WorldObject const* pov = nullptr) const;

        // Additional related server-side and client-side ownership-related methods
        // Spell mod owner: static player whose spell mods apply to this unit (server-side)
        virtual Player* GetSpellModOwner() const { return nullptr; }
        // Beneficiary: owner of the xp/loot/etc credit, master or self (server-side)
        Unit* GetBeneficiary() const;
        Player* GetBeneficiaryPlayer() const;

        // Client controlled: check if unit currently is under client control (has active "mover"), optionally check for specific client (server-side)
        bool IsClientControlled(Player const* exactClient = nullptr) const;
        // Controlling client: server PoV on which client (player) controls movement of the unit at the moment, obtain "mover" (server-side)
        Player const* GetClientControlling() const;

        Pet* GetPet() const;
        void SetPet(Unit* pet) { SetPetGuid(pet ? pet->GetObjectGuid() : ObjectGuid()); }

        Pet* GetMiniPet() const;
        void SetMiniPet(Unit* pet) { SetCritterGuid(pet ? pet->GetObjectGuid() : ObjectGuid()); }
        void RemoveMiniPet();

        void AddGuardian(Pet* pet);
        void RemoveGuardian(Pet* pet);
        void RemoveGuardians();
        Pet* FindGuardianWithEntry(uint32 entry);

        CharmInfo* GetCharmInfo() const { return m_charmInfo; }
        virtual CharmInfo* InitCharmInfo(Unit* charm);
        virtual void DeleteCharmInfo() { delete m_charmInfo; m_charmInfo = nullptr; }

        ObjectGuid const& GetTotemGuid(TotemSlot slot) const { return m_TotemSlot[slot]; }
        Totem* GetTotem(TotemSlot slot) const;
        bool IsAllTotemSlotsUsed() const;

        void _AddTotem(TotemSlot slot, Totem* totem);       // only for call from Totem summon code
        void _RemoveTotem(Totem* totem);                    // only for call from Totem class

        template<typename Func>
        void CallForAllControlledUnits(Func const& func, uint32 controlledMask);
        template<typename Func>
        bool CheckAllControlledUnits(Func const& func, uint32 controlledMask) const;

        bool AddSpellAuraHolder(SpellAuraHolder* holder);
        void AddAuraToModList(Aura* aura);

        // removing specific aura stack
        void RemoveAura(Aura* Aur, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveSpellAuraHolder(SpellAuraHolder* holder, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveSingleAuraFromSpellAuraHolder(SpellAuraHolder* holder, SpellEffectIndex index, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveSingleAuraFromSpellAuraHolder(uint32 spellId, SpellEffectIndex effindex, ObjectGuid casterGuid, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);

        // removing specific aura stacks by diff reasons and selections
        void RemoveAurasDueToSpell(uint32 spellId, SpellAuraHolder* except = nullptr, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAurasDueToItemSpell(Item* castItem, uint32 spellId);
        void RemoveAurasByCasterSpell(uint32 spellId, ObjectGuid casterGuid, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAurasDueToSpellByCancel(uint32 spellId);
        void RemoveAurasTriggeredBySpell(uint32 spellId, ObjectGuid casterGuid = ObjectGuid());
        void RemoveAuraStack(uint32 spellId);
        void RemoveAuraCharge(uint32 spellId);

        // removing unknown aura stacks by diff reasons and selections
        void RemoveNotOwnTrackedTargetAuras();
        void RemoveAurasAtMechanicImmunity(uint32 mechMask, uint32 exceptSpellId, bool non_positive = false);
        void RemoveSpellsCausingAura(AuraType auraType);
        void RemoveSpellsCausingAura(AuraType auraType, SpellAuraHolder* except);
        void RemoveSpellsCausingAura(AuraType auraType, SpellAuraHolder* except, bool onlyNegative);
        void RemoveSpellsCausingAura(AuraType auraType, ObjectGuid casterGuid);
        void RemoveRankAurasDueToSpell(uint32 spellId);
        bool RemoveNoStackAurasDueToAuraHolder(SpellAuraHolder* holder);
        void RemoveAurasWithInterruptFlags(uint32 flags);
        void RemoveAurasWithAttribute(uint32 flags);
        void RemoveAurasWithDispelType(DispelType type, ObjectGuid casterGuid = ObjectGuid());
        void RemoveAllAuras(AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAllAurasOnDeath();
        void RemoveAllAurasOnEvade();
        void RemoveAllGroupBuffsFromCaster(ObjectGuid casterGuid);

        // remove specific aura on cast
        void RemoveAurasOnCast(SpellEntry const* castedSpellEntry);

        // removing specific aura FROM stack by diff reasons and selections
        void RemoveAuraHolderFromStack(uint32 spellId, uint32 stackAmount = 1, ObjectGuid casterGuid = ObjectGuid(), AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAuraHolderDueToSpellByDispel(uint32 spellId, uint32 stackAmount, ObjectGuid casterGuid, Unit* dispeller);

        void DelaySpellAuraHolder(uint32 spellId, int32 delaytime, ObjectGuid casterGuid);

        void SetCreateStat(Stats stat, float val) { m_createStats[stat] = val; }
        void SetCreateHealth(uint32 val) { SetUInt32Value(UNIT_FIELD_BASE_HEALTH, val); }
        uint32 GetCreateHealth() const { return GetUInt32Value(UNIT_FIELD_BASE_HEALTH); }
        void SetCreateMana(uint32 val) { SetUInt32Value(UNIT_FIELD_BASE_MANA, val); }
        uint32 GetCreateMana() const { return GetUInt32Value(UNIT_FIELD_BASE_MANA); }
        uint32 GetCreatePowers(Powers power) const;
        float GetCreateStat(Stats stat) const { return m_createStats[stat]; }
        void SetCreateResistance(SpellSchools school, int32 val) { m_createResistances[school] = val; }
        int32 GetCreateResistance(SpellSchools school) const { return m_createResistances[school]; }

        void SetCurrentCastedSpell(Spell* pSpell);
        void InterruptSpell(CurrentSpellTypes spellType, bool withDelayed = true);
        void FinishSpell(CurrentSpellTypes spellType, bool ok = true);

        // set withDelayed to true to account delayed spells as casted
        // delayed+channeled spells are always accounted as casted
        // we can skip channeled or delayed checks using flags
        bool IsNonMeleeSpellCasted(bool withDelayed, bool skipChanneled = false, bool skipAutorepeat = false, bool forMovement = false) const;

        // set withDelayed to true to interrupt delayed spells too
        // delayed+channeled spells are always interrupted
        void InterruptNonMeleeSpells(bool withDelayed, uint32 spell_id = 0);

        Spell* GetCurrentSpell(CurrentSpellTypes spellType) const { return m_currentSpells[spellType]; }
        Spell* FindCurrentSpellBySpellId(uint32 spell_id) const;

        bool CheckAndIncreaseCastCounter();
        void DecreaseCastCounter() { if (m_castCounter) --m_castCounter; }

        ObjectGuid m_ObjectSlotGuid[4];

        ShapeshiftForm GetShapeshiftForm() const { return ShapeshiftForm(GetByteValue(UNIT_FIELD_BYTES_1, 2)); }
        void  SetShapeshiftForm(ShapeshiftForm form) { SetByteValue(UNIT_FIELD_BYTES_1, 2, form); }

        bool IsShapeShifted() const;
        bool IsInFeralForm() const
        {
            ShapeshiftForm form = GetShapeshiftForm();
            return form == FORM_CAT || form == FORM_BEAR || form == FORM_DIREBEAR;
        }

        bool IsInDisallowedMountForm() const
        {
            ShapeshiftForm form = GetShapeshiftForm();
            return form != FORM_NONE && form != FORM_BATTLESTANCE && form != FORM_BERSERKERSTANCE && form != FORM_DEFENSIVESTANCE &&
                   form != FORM_SHADOW && form != FORM_STEALTH;
        }

        float m_modMeleeHitChance;
        float m_modRangedHitChance;
        float m_modSpellHitChance;
        float m_modSpellCritChance[MAX_SPELL_SCHOOL];

        float m_modCritChance[MAX_ATTACK];
        float m_modDodgeChance;
        float m_modParryChance;
        float m_modBlockChance;

        float m_threatModifier[MAX_SPELL_SCHOOL];
        float m_modAttackSpeedPct[3];
        float m_modAttackBaseDPSPct[MAX_ATTACK] = { 1.0f, 1.0f, 1.0f };

        // Event handler
        EventProcessor m_events;

        // stat system
        bool HandleStatModifier(UnitMods unitMod, UnitModifierType modifierType, float amount, bool apply);
        void SetModifierValue(UnitMods unitMod, UnitModifierType modifierType, float value) { m_auraModifiersGroup[unitMod][modifierType] = value; }
        float GetModifierValue(UnitMods unitMod, UnitModifierType modifierType) const;
        float GetTotalStatValue(Stats stat) const;
        int32 GetTotalResistanceValue(SpellSchools school) const;
        float GetTotalAuraModValue(UnitMods unitMod) const;
        SpellSchools GetSpellSchoolByAuraGroup(UnitMods unitMod) const;
        Stats GetStatByAuraGroup(UnitMods unitMod) const;
        Powers GetPowerTypeByAuraGroup(UnitMods unitMod) const;
        bool CanModifyStats() const { return m_canModifyStats; }
        void SetCanModifyStats(bool modifyStats) { m_canModifyStats = modifyStats; }
        virtual bool UpdateStats(Stats stat) = 0;
        virtual bool UpdateAllStats() = 0;
        virtual void UpdateResistances(uint32 school) = 0;
        virtual void UpdateArmor() = 0;
        virtual void UpdateMaxHealth() = 0;
        virtual void UpdateMaxPower(Powers power) = 0;
        virtual void UpdateAttackPowerAndDamage(bool ranged = false) = 0;
        virtual void UpdateDamagePhysical(WeaponAttackType attType) = 0;
        float GetTotalAttackPowerValue(WeaponAttackType attType) const;

        float GetBaseWeaponDamage(WeaponAttackType attType, WeaponDamageRange damageRange, uint8 index = 0) const;
        void SetBaseWeaponDamage(WeaponAttackType attType, WeaponDamageRange damageRange, float value, uint8 index = 0) { m_weaponDamageInfo.weapon[attType].damage[index].value[damageRange] = value; }

        SpellSchools GetWeaponDamageSchool(WeaponAttackType attType, uint8 index = 0) const { return m_weaponDamageInfo.weapon[attType].damage[index].school; }
        void SetWeaponDamageSchool(WeaponAttackType attType, SpellSchools school, uint8 index = 0) { m_weaponDamageInfo.weapon[attType].damage[index].school = school; }

        SpellSchoolMask GetAttackDamageSchoolMask(WeaponAttackType attType, bool first = false) const;
        void SetAttackDamageSchool(WeaponAttackType attType, SpellSchools school);

        inline SpellSchoolMask GetRangedDamageSchoolMask(bool first = false) const { return GetAttackDamageSchoolMask(RANGED_ATTACK, first); }
        inline void SetRangedDamageSchool(SpellSchools school) { SetAttackDamageSchool(RANGED_ATTACK, school); }

        inline SpellSchoolMask GetMeleeDamageSchoolMask(bool main, bool first = false) const { return GetAttackDamageSchoolMask((main ? BASE_ATTACK : OFF_ATTACK), first); }
        inline SpellSchoolMask GetMeleeDamageSchoolMask() const { return SpellSchoolMask(GetMeleeDamageSchoolMask(true, true) | GetMeleeDamageSchoolMask(false, true)); }
        inline void SetMeleeDamageSchool(bool main, SpellSchools school) { SetAttackDamageSchool((main ? BASE_ATTACK : OFF_ATTACK), school); }
        inline void SetMeleeDamageSchool(SpellSchools school) { SetMeleeDamageSchool(true, school); SetMeleeDamageSchool(false, school); }

        // Visibility system
        UnitVisibility GetVisibility() const { return m_Visibility; }
        void SetVisibility(UnitVisibility x);
        void SetVisibilityWithoutUpdate(UnitVisibility x) { m_Visibility = x; }
        void UpdateVisibilityAndView() override;            // overwrite WorldObject::UpdateVisibilityAndView()

        // common function for visibility checks for player/creatures with detection code
        bool IsVisibleForOrDetect(Unit const* u, WorldObject const* viewPoint, bool detect, bool inVisibleList = false, bool is3dDistance = true, bool spell = false) const;
        float GetVisibleDistance(Unit const * target, bool alert = false) const;
        bool CanDetectInvisibilityOf(Unit const* u) const;
        uint32 GetInvisibilityDetectMask() const;
        void SetInvisibilityDetectMask(uint32 index, bool apply);
        uint32 GetInvisibilityMask() const;
        void SetInvisibilityMask(uint32 index, bool apply);
        void AddInvisibilityValue(uint32 index, int32 value) { m_invisibilityValues[index] += value; }
        void AddInvisibilityDetectValue(uint32 index, int32 value) { m_invisibilityDetectValues[index] += value; }
        int32 GetInvisibilityValue(uint32 index) const;
        int32 GetInvisibilityDetectValue(uint32 index) const;

        // virtual functions for all world objects types
        bool isVisibleForInState(Player const* u, WorldObject const* viewPoint, bool inVisibleList) const override;
        // function for low level grid visibility checks in player/creature cases
        virtual bool IsVisibleInGridForPlayer(Player* pl) const = 0;
        bool isInvisibleForAlive() const;

        TrackedAuraTargetMap&       GetTrackedAuraTargets(TrackedAuraType type)       { return m_trackedAuraTargets[type]; }
        TrackedAuraTargetMap const& GetTrackedAuraTargets(TrackedAuraType type) const { return m_trackedAuraTargets[type]; }
        SpellImmuneList m_spellImmune[MAX_SPELL_IMMUNITY];

        // Threat related methods
        bool CanHaveThreatList(bool ignoreAliveState = false) const;
        void AddThreat(Unit* pVictim, float threat = 0.0f, bool crit = false, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NONE, SpellEntry const* threatSpell = nullptr);
        float ApplyTotalThreatModifier(float threat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL);
        void DeleteThreatList();
        bool SelectHostileTarget();
        bool IsSuppressedTarget(Unit* target) const;
        bool IsOfflineTarget(Unit* victim) const;
        bool IsLeashingTarget(Unit* victim) const;
        void TauntUpdate();
        void FixateTarget(Unit* taunter);
        ThreatManager& getThreatManager() { return GetCombatData()->threatManager; }
        ThreatManager const& getThreatManager() const { return const_cast<Unit*>(this)->GetCombatData()->threatManager; }
        void addHatedBy(HostileReference* pHostileReference) { GetCombatData()->hostileRefManager.insertFirst(pHostileReference); };
        void removeHatedBy(HostileReference* /*pHostileReference*/) { /* nothing to do yet */ }
        HostileRefManager& getHostileRefManager() { return GetCombatData()->hostileRefManager; }
        void SetNoThreatState(bool state) { m_noThreat = state; }
        bool GetNoThreatState() { return m_noThreat; }

        Aura* GetAura(uint32 spellId, SpellEffectIndex effindex);
        Aura* GetAura(AuraType type, SpellFamily family, uint64 familyFlag, ObjectGuid casterGuid = ObjectGuid());
        SpellAuraHolder* GetSpellAuraHolder(uint32 spellid) const;
        SpellAuraHolder* GetSpellAuraHolder(uint32 spellid, ObjectGuid casterGuid) const;

        SpellAuraHolderMap&       GetSpellAuraHolderMap()       { return m_spellAuraHolders; }
        SpellAuraHolderMap const& GetSpellAuraHolderMap() const { return m_spellAuraHolders; }
        AuraList const& GetAurasByType(AuraType type) const { return m_modAuras[type]; }
        void ApplyAuraProcTriggerDamage(Aura* aura, bool apply);

        int32 GetTotalAuraModifier(AuraType auratype) const;
        float GetTotalAuraMultiplier(AuraType auratype) const;
        int32 GetMaxPositiveAuraModifier(AuraType auratype) const;
        int32 GetMaxNegativeAuraModifier(AuraType auratype) const;

        int32 GetTotalAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        float GetTotalAuraMultiplierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        int32 GetMaxPositiveAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        int32 GetMaxNegativeAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;

        int32 GetTotalAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        float GetTotalAuraMultiplierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetMaxPositiveAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetMaxNegativeAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;

        Aura* GetDummyAura(uint32 spell_id) const;

        uint32 m_AuraFlags;

        uint32 GetDisplayId() const { return GetUInt32Value(UNIT_FIELD_DISPLAYID); }
        void SetDisplayId(uint32 modelId);
        uint32 GetNativeDisplayId() const { return GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID); }
        void SetNativeDisplayId(uint32 modelId) { SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, modelId); }
        void RestoreDisplayId();

        // at any changes to scale and/or displayId
        void UpdateModelData();
        float GetObjectScaleMod() const;

        DynamicObject* GetDynObject(uint32 spellId, SpellEffectIndex effIndex);
        DynamicObject* GetDynObject(uint32 spellId);
        void AddDynObject(DynamicObject* dynObj);
        void RemoveDynObject(uint32 spellid);
        void RemoveDynObjectWithGUID(ObjectGuid guid) { m_dynObjGUIDs.remove(guid); }
        void RemoveAllDynObjects();

        GameObject* GetGameObject(uint32 spellId) const;
        void AddGameObject(GameObject* gameObj);
        void AddWildGameObject(GameObject* gameObj);
        void RemoveGameObject(GameObject* gameObj, bool del);
        void RemoveGameObject(uint32 spellid, bool del);
        void RemoveAllGameObjects();

        uint32 CalculateDamage(WeaponAttackType attType, bool normalized, uint8 index = 0);
        float GetAPMultiplier(WeaponAttackType attType, bool normalized);
        void ModifyAuraState(AuraState flag, bool apply);
        bool HasAuraState(AuraState flag) const { return HasFlag(UNIT_FIELD_AURASTATE, 1 << (flag - 1)); }
        void UnsummonAllTotems() const;
        Unit* SelectMagnetTarget(Unit* victim, Spell* spell = nullptr);

        int32 SpellBonusWithCoeffs(SpellEntry const* spellProto, int32 total, int32 benefit, int32 ap_benefit, DamageEffectType damagetype, bool donePart);
        int32 SpellBaseDamageBonusDone(SpellSchoolMask schoolMask);
        int32 SpellBaseDamageBonusTaken(SpellSchoolMask schoolMask) const;
        uint32 SpellDamageBonusDone(Unit* pVictim, SpellEntry const* spellProto, uint32 pdamage, DamageEffectType damagetype, uint32 stack = 1);
        uint32 SpellDamageBonusTaken(Unit* pCaster, SpellEntry const* spellProto, uint32 pdamage, DamageEffectType damagetype, uint32 stack = 1);
        int32 SpellBaseHealingBonusDone(SpellSchoolMask schoolMask);
        int32 SpellBaseHealingBonusTaken(SpellSchoolMask schoolMask) const;
        uint32 SpellHealingBonusDone(Unit* pVictim, SpellEntry const* spellProto, int32 healamount, DamageEffectType damagetype, uint32 stack = 1);
        uint32 SpellHealingBonusTaken(Unit* pCaster, SpellEntry const* spellProto, int32 healamount, DamageEffectType damagetype, uint32 stack = 1);
        uint32 MeleeDamageBonusDone(Unit* pVictim, uint32 damage, WeaponAttackType attType, SpellSchoolMask schoolMask, SpellEntry const* spellProto = nullptr, DamageEffectType damagetype = DIRECT_DAMAGE, uint32 stack = 1, bool flat = true);
        uint32 MeleeDamageBonusTaken(Unit* pCaster, uint32 pdamage, WeaponAttackType attType, SpellSchoolMask schoolMask, SpellEntry const* spellProto = nullptr, DamageEffectType damagetype = DIRECT_DAMAGE, uint32 stack = 1, bool flat = true);

        bool IsTriggeredAtSpellProcEvent(ProcExecutionData& data, SpellAuraHolder* holder, SpellProcEventEntry const*& spellProcEvent);
        // Aura proc handlers
        SpellAuraProcResult HandleDummyAuraProc(ProcExecutionData& data);
        SpellAuraProcResult HandleHasteAuraProc(ProcExecutionData& data);
        SpellAuraProcResult HandleProcTriggerSpellAuraProc(ProcExecutionData& data);
        SpellAuraProcResult HandleProcTriggerDamageAuraProc(ProcExecutionData& data);
        SpellAuraProcResult HandleOverrideClassScriptAuraProc(ProcExecutionData& data);
        SpellAuraProcResult HandleModCastingSpeedNotStackAuraProc(ProcExecutionData& data);
        SpellAuraProcResult HandleReflectSpellsSchoolAuraProc(ProcExecutionData& data);
        SpellAuraProcResult HandleModPowerCostSchoolAuraProc(ProcExecutionData& data);
        SpellAuraProcResult HandleMagnetAuraProc(ProcExecutionData& data);
        SpellAuraProcResult HandleMechanicImmuneResistanceAuraProc(ProcExecutionData& data);
        SpellAuraProcResult HandleModResistanceAuraProc(ProcExecutionData& data);
        SpellAuraProcResult HandleRemoveByDamageChanceProc(ProcExecutionData& data);
        SpellAuraProcResult HandleInvisibilityAuraProc(ProcExecutionData& data);
        SpellAuraProcResult HandleNULLProc(ProcExecutionData& /*data*/)
        {
            // no proc handler for this aura type
            return SPELL_AURA_PROC_OK;
        }
        SpellAuraProcResult HandleCantTrigger(ProcExecutionData& /*data*/)
        {
            // this aura type can't proc
            return SPELL_AURA_PROC_CANT_TRIGGER;
        }

        void SetLastManaUse()
        {
            m_lastManaUseTimer = 5000;
        }
        bool IsUnderLastManaUseEffect() const { return m_lastManaUseTimer != 0; }

        void SetContestedPvP(Player* attackedPlayer = nullptr);

        void ApplySpellImmune(Aura const* aura, uint32 op, uint32 type, bool apply);
        void ApplySpellDispelImmunity(Aura const* aura, DispelType type, bool apply);
        virtual bool IsImmuneToSpell(SpellEntry const* spellInfo, bool castOnSelf, uint8 effectMask);
        virtual bool IsImmuneToDamage(SpellSchoolMask meleeSchoolMask);
        virtual bool IsImmuneToSpellEffect(SpellEntry const* spellInfo, SpellEffectIndex index, bool castOnSelf) const;
        bool IsImmuneToSchool(SpellEntry const* spellInfo, uint8 effectMask) const;

        uint32 CalcArmorReducedDamage(Unit* pVictim, const uint32 damage);
        void CalculateDamageAbsorbAndResist(Unit* pCaster, SpellSchoolMask schoolMask, DamageEffectType damagetype, const uint32 damage, uint32* absorb, int32* resist, bool canReflect = false, bool canResist = true, bool binary = false);
        void CalculateAbsorbResistBlock(Unit* pCaster, SpellNonMeleeDamage* damageInfo, SpellEntry const* spellProto, WeaponAttackType attType = BASE_ATTACK);

        void  UpdateSpeed(UnitMoveType mtype, bool forced, float ratio = 1.0f);
        float GetSpeed(UnitMoveType mtype) const;
        float GetSpeedRate(UnitMoveType mtype) const { return m_speed_rate[mtype]; }
        void SetSpeedRate(UnitMoveType mtype, float rate, bool forced = false);

        bool isHover() const { return HasAuraType(SPELL_AURA_HOVER); }

        void _RemoveAllAuraMods();
        void _ApplyAllAuraMods();

        int32 CalculateSpellDamage(Unit const* target, SpellEntry const* spellProto, SpellEffectIndex effect_index, int32 const* effBasePoints = nullptr);

        float CalculateLevelPenalty(SpellEntry const* spellProto) const;

        void addFollower(FollowerReference* pRef) { m_FollowingRefManager.insertFirst(pRef); }
        void removeFollower(FollowerReference* /*pRef*/) { /* nothing to do yet */ }

        MotionMaster* GetMotionMaster() { return &i_motionMaster; }

        bool IsStopped() const { return !(hasUnitState(UNIT_STAT_MOVING)); }
        void StopMoving(bool forceSendStop = false);
        void InterruptMoving(bool forceSendStop = false);

        ///----------Various crowd control methods-----------------
        bool IsImmobilized() const { return hasUnitState(UNIT_STAT_ROOT | UNIT_STAT_STUNNED); }
        void SetImmobilizedState(bool apply, bool stun = false);

        // These getters operate on unit flags set by IncapacitatedState and are meant for formal usage in conjunction with spell effects only
        // For actual internal movement states use UnitState flags
        // TODO: The UnitState thing needs to be rewriten at some point, this kind of duality is bad
        bool IsFleeing() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING); }
        bool IsConfused() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED); }
        bool IsStunned() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED); }
        bool IsIncapacitated() const { return (IsFleeing() || IsConfused() || IsStunned()); }

        void SetFeared(bool apply, ObjectGuid casterGuid = ObjectGuid(), uint32 spellID = 0, uint32 time = 0);
        void SetConfused(bool apply, ObjectGuid casterGuid = ObjectGuid(), uint32 spellID = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        void SetStunned(bool apply, ObjectGuid casterGuid = ObjectGuid(), uint32 spellID = 0);
        void SetIncapacitatedState(bool apply, uint32 state = 0, ObjectGuid casterGuid = ObjectGuid(), uint32 spellID = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT, uint32 time = 0);
        ///----------End of crowd control methods----------

        bool IsFeigningDeath() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_29); }
        bool IsFeigningDeathSuccessfully() const { return hasUnitState(UNIT_STAT_FEIGN_DEATH); }
        void SetFeignDeath(bool apply, ObjectGuid casterGuid = ObjectGuid(), uint32 spellID = 0, bool dynamic = true, bool success = true);

        void InterruptSpellsCastedOnMe(bool killDelayed = false);

        void AddComboPointHolder(uint32 lowguid) { m_ComboPointHolders.insert(lowguid); }
        void RemoveComboPointHolder(uint32 lowguid) { m_ComboPointHolders.erase(lowguid); }
        void ClearComboPointHolders();

        ///----------Pet responses methods-----------------
        void SendPetCastFail(uint32 spellid, SpellCastResult msg) const;
        void SendPetActionFeedback(uint8 msg) const;
        void SendPetTalk(uint32 pettalk) const;
        void SendPetAIReaction() const;
        void SendPetDismiss(uint32 soundId) const;
        ///----------End of Pet responses methods----------

        void propagateSpeedChange() { GetMotionMaster()->propagateSpeedChange(); }

        // reactive attacks
        void ClearAllReactives();
        void StartReactiveTimer(ReactiveType reactive) { m_reactiveTimer[reactive] = REACTIVE_TIMER_START;}
        void UpdateReactives(uint32 p_time);

        // group updates
        void UpdateAuraForGroup(uint8 slot);

        // pet auras
        typedef std::set<PetAura const*> PetAuraSet;
        PetAuraSet m_petAuras;
        void AddPetAura(PetAura const* petSpell);
        void RemovePetAura(PetAura const* petSpell);

        // Movement info
        MovementInfo m_movementInfo;
        Movement::MoveSpline* movespline;

        void ScheduleAINotify(uint32 delay, bool forced = false);
        bool IsAINotifyScheduled() const { return m_AINotifyEvent != nullptr;}
        void FinalizeAINotifyEvent() { m_AINotifyEvent = nullptr; }
        void AbortAINotifyEvent();
        void OnRelocated();

        bool IsLinkingEventTrigger() { return m_isCreatureLinkingTrigger; }

        virtual bool CanSwim() const = 0;
        virtual bool CanFly() const = 0;
        virtual bool CanWalk() const = 0;
        virtual bool CanBeDetected() const { return true; }

        void ForceHealthAndPowerUpdate();   // force server to send new value for hp and power (including max)

        void TriggerEvadeEvents();
        void EvadeTimerExpired();
        bool IsInEvadeMode() const { return m_evadeTimer > 0 || m_evadeMode; }
        bool IsEvadingHome() const { return m_evadeMode == EVADE_HOME; }
        bool IsEvadeRegen() const { return (m_evadeTimer > 0 && m_evadeTimer <= 5000) || m_evadeMode; } // Only regen after 5 seconds, or when in permanent evade
        void StartEvadeTimer() { m_evadeTimer = 10000; } // 10 seconds after which action is taken
        void StopEvade(); // Stops either timer or evade state
        void SetEvade(EvadeState state); // Propagated to pets

        // Take possession of an unit (pet, creature, ...)
        bool TakePossessOf(Unit* possessed);

        // Take possession of a new spawned unit
        Unit* TakePossessOf(SpellEntry const* spellEntry, uint32 effIdx, float x, float y, float z, float ang);

        // Take charm of an unit
        bool TakeCharmOf(Unit* charmed, uint32 spellId = 0, bool advertised = true);

        // Break own charm effects on a specific charmed unit
        void BreakCharmOutgoing(Unit* charmed);

        // Break own charm effects on all charmed units or advertised charm field only
        void BreakCharmOutgoing(bool advertisedOnly = false);

        // Break charm effects from current charmer
        void BreakCharmIncoming();

        // Uncharm (physically revert the charm effect) the unit and reset player control if required
        void Uncharm(Unit* charmed, uint32 spellId = 0);

        // Combat prevention
        bool CanEnterCombat() { return m_canEnterCombat && !IsEvadingHome(); }
        void SetCanEnterCombat(bool can) { m_canEnterCombat = can; }

        void SetTurningOff(bool apply);
        virtual bool IsIgnoringRangedTargets() { return false; }

        float GetAttackDistance(Unit const* pl) const;
        virtual uint32 GetDetectionRange() const { return 20.f; }

        virtual UnitAI* AI() { return nullptr; }
        virtual CombatData* GetCombatData() { return m_combatData; }

        virtual void AddCooldown(SpellEntry const& spellEntry, ItemPrototype const* itemProto = nullptr, bool permanent = false, uint32 forcedDuration = 0) override;

        bool IsSpellProccingHappening() const { return m_spellProcsHappening; }
        void AddDelayedHolderDueToProc(SpellAuraHolder* holder) { m_delayedSpellAuraHolders.push_back(holder); }

        void ResetAutoRepeatSpells() { m_AutoRepeatFirstCast = true; }

        const uint64& GetAuraUpdateMask() const { return m_auraUpdateMask; }
        void SetAuraUpdateMask(uint8 slot) { m_auraUpdateMask |= (uint64(1) << slot); }
        void ResetAuraUpdateMask() { m_auraUpdateMask = 0; }

        // WorldObject overrides
        void UpdateAllowedPositionZ(float x, float y, float& z, Map* atMap = nullptr) const override;

        virtual uint32 GetSpellRank(SpellEntry const* spellInfo);

    protected:

        struct WeaponDamageInfo
        {
            struct Weapon
            {
                struct Damage
                {
                    SpellSchools school = SPELL_SCHOOL_NORMAL;
                    float value[2] = { BASE_MINDAMAGE, BASE_MAXDAMAGE };
                };

                uint32 lines = 1;
                Damage damage[MAX_ITEM_PROTO_DAMAGES];
            };

            Weapon weapon[MAX_ATTACK];
        };

        explicit Unit();

        void _UpdateSpells(uint32 time);
        virtual void _UpdateAutoRepeatSpell();
        bool m_AutoRepeatFirstCast;

        EvadeState GetEvade() const { return m_evadeMode; }

        uint32 m_attackTimer[MAX_ATTACK];

        float m_createStats[MAX_STATS];
        int32 m_createResistances[MAX_SPELL_SCHOOL];

        Unit* m_attacking;

        DeathState m_deathState;

        SpellAuraHolderMap m_spellAuraHolders;
        SpellAuraHolderMap::iterator m_spellAuraHoldersUpdateIterator; // != end() in Unit::m_spellAuraHolders update and point to next element
        AuraList m_deletedAuras;                            // auras removed while in ApplyModifier and waiting deleted
        SpellAuraHolderList m_deletedHolders;

        // Store Auras for which the target must be tracked
        TrackedAuraTargetMap m_trackedAuraTargets[MAX_TRACKED_AURA_TYPES];

        GuidList m_dynObjGUIDs;

        GameObjectList m_gameObj;
        typedef std::map<uint32, ObjectGuid> WildGameObjectMap;
        WildGameObjectMap m_wildGameObjs;
        bool m_isSorted;
        uint32 m_transform;

        AuraList m_modAuras[TOTAL_AURAS];
        float m_auraModifiersGroup[UNIT_MOD_END][MODIFIER_TYPE_END];

        WeaponDamageInfo m_weaponDamageInfo;

        bool m_canModifyStats;
        // std::list< spellEffectPair > AuraSpells[TOTAL_AURAS];  // TODO: use this if ok for mem

        float m_speed_rate[MAX_MOVE_TYPE];

        CharmInfo* m_charmInfo;

        MotionMaster i_motionMaster;

        uint32 m_reactiveTimer[MAX_REACTIVE];
        uint32 m_regenTimer;
        uint32 m_lastManaUseTimer;

        bool m_canDodge;
        bool m_canParry;
        bool m_canBlock;

        bool m_canDualWield = false;

        void DisableSpline();
        bool m_isCreatureLinkingTrigger;
        bool m_isSpawningLinked;

        CombatData* m_combatData;
        Spell* m_currentSpells[CURRENT_MAX_SPELL];

        virtual void SetBaseWalkSpeed(float speed) { m_baseSpeedWalk = speed; }
        virtual void SetBaseRunSpeed(float speed) { m_baseSpeedRun = speed; }

        // base speeds set by model/template
        float m_baseSpeedWalk;
        float m_baseSpeedRun;

        // Protected unit guid fields getters/setters
        // Charm: temporary pet unit guid
        ObjectGuid const& GetCharmGuid() const { return GetGuidValue(UNIT_FIELD_CHARM); }
        void SetCharmGuid(ObjectGuid const& charm) { SetGuidValue(UNIT_FIELD_CHARM, charm); }
        // Summon: permanent pet unit guid (do not use: managed by SetPetGuid/GetPetGuid)
        ObjectGuid const& GetSummonGuid() const { return GetGuidValue(UNIT_FIELD_SUMMON); }
        void SetSummonGuid(ObjectGuid const& summon) { SetGuidValue(UNIT_FIELD_SUMMON, summon); }
        // Charmer: temporary owner unit guid [nameplate]
        ObjectGuid const& GetCharmerGuid() const { return GetGuidValue(UNIT_FIELD_CHARMEDBY); }
        void SetCharmerGuid(ObjectGuid const& owner) { SetGuidValue(UNIT_FIELD_CHARMEDBY, owner); }
        // Summoner: permanent owner unit guid for player pets [nameplate] (do not use: managed by SetOwnerGuid/GetOwnerGuid)
        ObjectGuid const& GetSummonerGuid() const { return GetGuidValue(UNIT_FIELD_SUMMONEDBY); }
        void SetSummonerGuid(ObjectGuid const& owner) { SetGuidValue(UNIT_FIELD_SUMMONEDBY, owner); }
        // Creator: permanent owner unit guid for npc pets or non-pet units [nameplate] (do not use: managed by SetOwnerGuid/GetOwnerGuid)
        ObjectGuid const& GetCreatorGuid() const { return GetGuidValue(UNIT_FIELD_CREATEDBY); }
        void SetCreatorGuid(ObjectGuid const& creator) { SetGuidValue(UNIT_FIELD_CREATEDBY, creator); }
        // Target: current target guid as advertised on unit frames (also known as selection)
        ObjectGuid const& GetTargetGuid() const { return GetGuidValue(UNIT_FIELD_TARGET); }
        void SetTargetGuid(ObjectGuid const& targetGuid) { SetGuidValue(UNIT_FIELD_TARGET, targetGuid); }
        // Persuation target: temporarily increased reputation unit guid (pre-WotLK relations)
        ObjectGuid const& GetPersuadedGuid() const { return GetGuidValue(UNIT_FIELD_PERSUADED); }
        void SetPersuadedGuid(ObjectGuid const& persuaded) { SetGuidValue(UNIT_FIELD_PERSUADED, persuaded); }
        // Channel target: current channeling spell's target worldobject guid
        ObjectGuid const& GetChannelObjectGuid() const { return GetGuidValue(UNIT_FIELD_CHANNEL_OBJECT); }
        void SetChannelObjectGuid(ObjectGuid const& targetGuid) { SetGuidValue(UNIT_FIELD_CHANNEL_OBJECT, targetGuid); }

        // Additional server-side guid fields getters/setters
        // Critter: permanent mini-pet unit guid (pre-WotLK compatibility replacement for critter guid field)
        ObjectGuid const& GetCritterGuid() const { return m_critterGuid; }
        void SetCritterGuid(ObjectGuid critterGuid) { m_critterGuid = critterGuid; }

    private:
        void CleanupDeletedAuras();
        void UpdateSplineMovement(uint32 t_diff);

        Unit* _GetTotem(TotemSlot slot) const;              // for templated function without include need
        Pet* _GetPet(ObjectGuid guid) const;                // for templated function without include need
        Unit* _GetUnit(ObjectGuid guid) const;              // for templated function without include need

        // Wrapper called by DealDamage when a creature is killed
        void JustKilledCreature(Creature* victim, Player* responsiblePlayer);

        uint32 m_state;                                     // Even derived shouldn't modify
        uint32 m_CombatTimer;
        bool   m_dummyCombatState;                          // Used to keep combat state during some aura

        AttackerSet m_attackers;                            // Used to help know who is currently attacking this unit
        uint32 m_castCounter;                               // count casts chain of triggered spells for prevent infinity cast crashes

        UnitVisibility m_Visibility;
        Position m_last_notified_position;
        BasicEvent* m_AINotifyEvent;
        ShortTimeTracker m_movesplineTimer;

        Diminishing m_Diminishing;

        FollowerRefManager m_FollowingRefManager;

        ComboPointHolderSet m_ComboPointHolders;

        ObjectGuid m_critterGuid;                           // pre-WotLK critter compatibility field

        GuidSet m_guardianPets;

        GuidSet m_charmedUnitsPrivate;                      // stores non-advertised active charmed unit guids (e.g. aoe charms)

        ObjectGuid m_TotemSlot[MAX_TOTEM_SLOT];

        bool m_canEnterCombat;

        // Need to safeguard aura application in Unit::Update
        bool m_spellUpdateHappening;
        // Need to safeguard aura proccing in Unit::ProcDamageAndSpell
        bool m_spellProcsHappening;
        std::vector<SpellAuraHolder*> m_delayedSpellAuraHolders;

        bool m_noThreat;

        // guard to prevent chaining extra attacks
        bool m_extraAttacksExecuting;

        uint32 m_evadeTimer; // Used for evade during combat when mob is not running home and target isnt reachable
        EvadeState m_evadeMode; // Used for evade during running home

        // invisibility data
        uint32 m_invisibilityMask;
        uint32 m_detectInvisibilityMask; // is inherited from controller in PC case
        int32 m_invisibilityValues[INVISIBILITY_MAX];
        int32 m_invisibilityDetectValues[INVISIBILITY_MAX];

        uint64 m_auraUpdateMask;

    private:                                                // Error traps for some wrong args using
        // this will catch and prevent build for any cases when all optional args skipped and instead triggered used non boolean type
        // no bodies expected for this declarations
        template <typename TR>
        void CastSpell(Unit* Victim, uint32 spell, TR triggered);
        template <typename TR>
        void CastSpell(Unit* Victim, SpellEntry const* spell, TR triggered);
        template <typename TR>
        void CastCustomSpell(Unit* Victim, uint32 spell, int32 const* bp0, int32 const* bp1, int32 const* bp2, TR triggeredFlags);
        template <typename SP, typename TR>
        void CastCustomSpell(Unit* Victim, SpellEntry const* spell, int32 const* bp0, int32 const* bp1, int32 const* bp2, TR triggeredFlags);
        template <typename TR>
        void CastSpell(float x, float y, float z, uint32 spell, TR triggered);
        template <typename TR>
        void CastSpell(float x, float y, float z, SpellEntry const* spell, TR triggered);
};

template<typename Func>
void Unit::CallForAllControlledUnits(Func const& func, uint32 controlledMask)
{
    if (controlledMask & CONTROLLED_PET)
        if (Pet* pet = GetPet())
            func(pet);

    if (controlledMask & CONTROLLED_MINIPET)
        if (Pet* mini = GetMiniPet())
            func(mini);

    if (controlledMask & CONTROLLED_GUARDIANS)
    {
        for (GuidSet::const_iterator itr = m_guardianPets.begin(); itr != m_guardianPets.end();)
            if (Pet* guardian = _GetPet(*(itr++)))
                func(guardian);
    }

    if (controlledMask & CONTROLLED_TOTEMS)
    {
        for (int i = 0; i < MAX_TOTEM_SLOT; ++i)
            if (Unit* totem = _GetTotem(TotemSlot(i)))
                func(totem);
    }

    if (controlledMask & CONTROLLED_CHARM)
    {
        if (Unit* charm = GetCharm())
            func(charm);

        for (GuidSet::const_iterator itr = m_charmedUnitsPrivate.begin(); itr != m_charmedUnitsPrivate.end();)
            if (Unit* charmed = _GetUnit(*(itr++)))
                func(charmed);
    }
}


template<typename Func>
bool Unit::CheckAllControlledUnits(Func const& func, uint32 controlledMask) const
{
    if (controlledMask & CONTROLLED_PET)
        if (Pet const* pet = GetPet())
            if (func(pet))
                return true;

    if (controlledMask & CONTROLLED_MINIPET)
        if (Pet* mini = GetMiniPet())
            if (func(mini))
                return true;

    if (controlledMask & CONTROLLED_GUARDIANS)
    {
        for (GuidSet::const_iterator itr = m_guardianPets.begin(); itr != m_guardianPets.end();)
            if (Pet const* guardian = _GetPet(*(itr++)))
                if (func(guardian))
                    return true;
    }

    if (controlledMask & CONTROLLED_TOTEMS)
    {
        for (int i = 0; i < MAX_TOTEM_SLOT; ++i)
            if (Unit const* totem = _GetTotem(TotemSlot(i)))
                if (func(totem))
                    return true;
    }

    if (controlledMask & CONTROLLED_CHARM)
    {
        if (Unit const* charm = GetCharm())
            if (func(charm))
                return true;

        for (GuidSet::const_iterator itr = m_charmedUnitsPrivate.begin(); itr != m_charmedUnitsPrivate.end();)
            if (Unit const* charmed = _GetUnit(*(itr++)))
                if (func(charmed))
                    return true;
    }

    return false;
}

// Helper for targets nearest to the spell target
// The spell target is always first unless there is a target at _completely_ the same position (unbelievable case)
struct TargetDistanceOrderNear : public std::binary_function<Unit const, Unit const, bool>
{
    Unit const* m_mainTarget;
    DistanceCalculation m_distcalc;

    TargetDistanceOrderNear(Unit const* target, DistanceCalculation distcalc = DIST_CALC_NONE) : m_mainTarget(target), m_distcalc(distcalc) {}
    // functor for operator ">"
    bool operator()(Unit const* _Left, Unit const* _Right) const
    {
        return m_mainTarget->GetDistanceOrder(_Left, _Right, m_distcalc);
    }
};

// Helper for targets furthest away to the spell target
// The spell target is always first unless there is a target at _completely_ the same position (unbelievable case)
struct TargetDistanceOrderFarAway : public std::binary_function<Unit const, Unit const, bool>
{
    Unit const* m_mainTarget;
    DistanceCalculation m_distcalc;
    TargetDistanceOrderFarAway(Unit const* target, DistanceCalculation distcalc = DIST_CALC_NONE) : m_mainTarget(target), m_distcalc(distcalc) {}
    // functor for operator "<"
    bool operator()(Unit const* _Left, Unit const* _Right) const
    {
        return !m_mainTarget->GetDistanceOrder(_Left, _Right, m_distcalc);
    }
};

struct LowestHPNearestOrder : public std::binary_function<Unit const, Unit const, bool>
{
    Unit const* m_mainTarget;
    DistanceCalculation m_distcalc;

    LowestHPNearestOrder(Unit const* target, DistanceCalculation distcalc = DIST_CALC_NONE) : m_mainTarget(target), m_distcalc(distcalc) {}
    // functor for operator ">"
    bool operator()(Unit const* _Left, Unit const* _Right) const
    {
        if (_Left->GetHealthPercent() == _Right->GetHealthPercent())
            return m_mainTarget->GetDistanceOrder(_Left, _Right, m_distcalc);
        return _Left->GetHealthPercent() < _Right->GetHealthPercent();
    }
};

/** @} */

#endif
