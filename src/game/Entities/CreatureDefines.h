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

#ifndef MANGOSSERVER_CREATURE_DEFINES_H
#define MANGOSSERVER_CREATURE_DEFINES_H

#include "Common.h"
#include "Globals/EnumFlag.h"

// keeping all naming since its known and does not hurt

enum class CreatureStaticFlags : uint32
{
    MOUNTABLE                   = 0x00000001,
    NO_XP                       = 0x00000002,
    NO_LOOT                     = 0x00000004,
    UNKILLABLE                  = 0x00000008,
    TAMEABLE                    = 0x00000010, // CREATURE_TYPE_FLAG_TAMEABLE
    IMMUNE_TO_PC                = 0x00000020, // UNIT_FLAG_IMMUNE_TO_PC
    IMMUNE_TO_NPC               = 0x00000040, // UNIT_FLAG_IMMUNE_TO_NPC
    CAN_WIELD_LOOT              = 0x00000080,
    SESSILE                     = 0x00000100, // creature_template_movement.Rooted = 1
    UNINTERACTIBLE              = 0x00000200, // UNIT_FLAG_UNINTERACTIBLE
    NO_AUTOMATIC_REGEN          = 0x00000400, // Creatures with that flag uses no UNIT_FLAG2_REGENERATE_POWER
    DESPAWN_INSTANTLY           = 0x00000800, // Creature instantly disappear when killed
    CORPSE_RAID                 = 0x00001000,
    CREATOR_LOOT                = 0x00002000, // Lootable only by creator(engineering dummies)
    NO_DEFENSE                  = 0x00004000,
    NO_SPELL_DEFENSE            = 0x00008000,
    BOSS_MOB                    = 0x00010000, // CREATURE_TYPE_FLAG_BOSS_MOB, original description: Raid Boss Mob
    COMBAT_PING                 = 0x00020000,
    AQUATIC                     = 0x00040000, // aka Water Only, creature_template_movement.Ground = 0
    AMPHIBIOUS                  = 0x00080000, // creature_template_movement.Swim = 1
    NO_MELEE_FLEE               = 0x00100000, // Prevents melee and makes npc flee if it enters combat - also misused to just prevent melee on npcs that cant move
    VISIBLE_TO_GHOSTS           = 0x00200000, // CREATURE_TYPE_FLAG_VISIBLE_TO_GHOSTS
    PVP_ENABLING                = 0x00400000, // Old UNIT_FLAG_PVP_ENABLING, now UNIT_BYTES_2_OFFSET_PVP_FLAG from UNIT_FIELD_BYTES_2
    DO_NOT_PLAY_WOUND_ANIM      = 0x00800000, // CREATURE_TYPE_FLAG_DO_NOT_PLAY_WOUND_ANIM
    NO_FACTION_TOOLTIP          = 0x01000000, // CREATURE_TYPE_FLAG_NO_FACTION_TOOLTIP
    IGNORE_COMBAT               = 0x02000000, // Actually only changes react state to passive
    ONLY_ATTACK_PVP_ENABLING    = 0x04000000, // "Only attack targets that are PvP enabling"
    CALLS_GUARDS                = 0x08000000, // Creature will summon a guard if player is within its aggro range (even if creature doesn't attack per se)
    CAN_SWIM                    = 0x10000000, // UnitFlags 0x8000 UNIT_FLAG_CAN_SWIM
    FLOATING                    = 0x20000000, // creature_template_movement.Flight = 1
    MORE_AUDIBLE                = 0x40000000, // CREATURE_TYPE_FLAG_MORE_AUDIBLE
    LARGE_AOI                   = 0x80000000  // UnitFlags2 0x200000
};

DEFINE_ENUM_FLAG(CreatureStaticFlags);

enum class CreatureStaticFlags2 : uint32
{
    NO_PET_SCALING                      = 0x00000001,
    FORCE_PARTY_MEMBERS_INTO_COMBAT     = 0x00000002, // Original description: Force Raid Combat
    LOCK_TAPPERS_TO_RAID_ON_DEATH       = 0x00000004, // "Lock Tappers To Raid On Death", toggleable by 'Set "RAID_LOCK_ON_DEATH" flag for unit(s)' action, CREATURE_FLAG_EXTRA_INSTANCE_BIND
    SPELL_ATTACKABLE                    = 0x00000008, // CREATURE_TYPE_FLAG_SPELL_ATTACKABLE, original description(not valid anymore?): No Harmful Vertex Coloring
    NO_CRUSHING_BLOWS                   = 0x00000010,
    NO_OWNER_THREAT                     = 0x00000020,
    NO_WOUNDED_SLOWDOWN                 = 0x00000040,
    USE_CREATOR_BONUSES                 = 0x00000080,
    IGNORE_FEIGN_DEATH                  = 0x00000100,
    IGNORE_SANCTUARY                    = 0x00000200,
    ACTION_TRIGGERS_WHILE_CHARMED       = 0x00000400,
    INTERACT_WHILE_DEAD                 = 0x00000800, // CREATURE_TYPE_FLAG_INTERACT_WHILE_DEAD
    NO_INTERRUPT_SCHOOL_COOLDOWN        = 0x00001000,
    RETURN_SOUL_SHARD_TO_MASTER_OF_PET  = 0x00002000,
    SKIN_WITH_HERBALISM                 = 0x00004000, // CREATURE_TYPE_FLAG_SKIN_WITH_HERBALISM
    SKIN_WITH_MINING                    = 0x00008000, // CREATURE_TYPE_FLAG_SKIN_WITH_MINING
    ALERT_CONTENT_TEAM_ON_DEATH         = 0x00010000,
    ALERT_CONTENT_TEAM_AT_90_PCT_HP     = 0x00020000,
    ALLOW_MOUNTED_COMBAT                = 0x00040000, // CREATURE_TYPE_FLAG_ALLOW_MOUNTED_COMBAT
    PVP_ENABLING_OOC                    = 0x00080000,
    NO_DEATH_MESSAGE                    = 0x00100000, // CREATURE_TYPE_FLAG_NO_DEATH_MESSAGE
    IGNORE_PATHING_FAILURE              = 0x00200000,
    FULL_SPELL_LIST                     = 0x00400000,
    DOES_NOT_REDUCE_REPUTATION_FOR_RAIDS = 0x00800000,
    IGNORE_MISDIRECTION                 = 0x01000000,
    HIDE_BODY                           = 0x02000000, // UNIT_FLAG2_HIDE_BODY
    SPAWN_DEFENSIVE                     = 0x04000000,
    SERVER_ONLY                         = 0x08000000,
    CAN_SAFE_FALL                       = 0x10000000, // Original description: No Collision
    CAN_ASSIST                          = 0x20000000, // CREATURE_TYPE_FLAG_CAN_ASSIST, original description: Player Can Heal/Buff
    NO_SKILL_GAINS                      = 0x40000000,
    NO_PET_BAR                          = 0x80000000  // CREATURE_TYPE_FLAG_NO_PET_BAR
};

DEFINE_ENUM_FLAG(CreatureStaticFlags2);

enum class CreatureStaticFlags3 : uint32
{
    NO_DAMAGE_HISTORY              = 0x00000001,
    DONT_PVP_ENABLE_OWNER          = 0x00000002,
    DO_NOT_FADE_IN                 = 0x00000004, // UNIT_FLAG2_DO_NOT_FADE_IN
    MASK_UID                       = 0x00000008, // CREATURE_TYPE_FLAG_MASK_UID, original description: Non-Unique In Combat Log
    SKIN_WITH_ENGINEERING          = 0x00000010, // CREATURE_TYPE_FLAG_SKIN_WITH_ENGINEERING
    NO_AGGRO_ON_LEASH              = 0x00000020,
    NO_FRIENDLY_AREA_AURAS         = 0x00000040,
    EXTENDED_CORPSE_DURATION       = 0x00000080,
    CANNOT_SWIM                    = 0x00000100, // UNIT_FLAG_CANNOT_SWIM
    TAMEABLE_EXOTIC                = 0x00000200, // CREATURE_TYPE_FLAG_TAMEABLE_EXOTIC
    GIGANTIC_AOI                   = 0x00000400, // Since MoP, creatures with that flag have UnitFlags2 0x400000
    INFINITE_AOI                   = 0x00000800, // Since MoP, creatures with that flag have UnitFlags2 0x40000000
    CANNOT_PENETRATE_WATER         = 0x00001000, // Waterwalking
    NO_NAME_PLATE                  = 0x00002000, // CREATURE_TYPE_FLAG_NO_NAME_PLATE
    CHECKS_LIQUIDS                 = 0x00004000,
    NO_THREAT_FEEDBACK             = 0x00008000,
    USE_MODEL_COLLISION_SIZE       = 0x00010000, // CREATURE_TYPE_FLAG_USE_MODEL_COLLISION_SIZE
    ATTACKER_IGNORES_FACING        = 0x00020000, // In 3.3.5 used only by Rocket Propelled Warhead
    ALLOW_INTERACTION_WHILE_IN_COMBAT = 0x00040000, // CREATURE_TYPE_FLAG_ALLOW_INTERACTION_WHILE_IN_COMBAT
    SPELL_CLICK_FOR_PARTY_ONLY     = 0x00080000,
    FACTION_LEADER                 = 0x00100000,
    IMMUNE_TO_PLAYER_BUFFS         = 0x00200000,
    COLLIDE_WITH_MISSILES          = 0x00400000, // CREATURE_TYPE_FLAG_COLLIDE_WITH_MISSILES
    CAN_BE_MULTITAPPED             = 0x00800000, // Original description: Do Not Tap (Credit to threat list)
    DO_NOT_PLAY_MOUNTED_ANIMATIONS = 0x01000000, // CREATURE_TYPE_FLAG_DO_NOT_PLAY_MOUNTED_ANIMATIONS, original description: Disable Dodge, Parry and Block Animations
    CANNOT_TURN                    = 0x02000000, // UNIT_FLAG2_CANNOT_TURN
    ENEMY_CHECK_IGNORES_LOS        = 0x04000000,
    FOREVER_CORPSE_DURATION        = 0x08000000, // 7 days
    PETS_ATTACK_WITH_3D_PATHING    = 0x10000000, // "Pets attack with 3d pathing (Kologarn)"
    LINK_ALL                       = 0x20000000, // CREATURE_TYPE_FLAG_LINK_ALL
    AI_CAN_AUTO_TAKEOFF_IN_COMBAT  = 0x40000000,
    AI_CAN_AUTO_LAND_IN_COMBAT     = 0x80000000
};

DEFINE_ENUM_FLAG(CreatureStaticFlags3);

enum class CreatureStaticFlags4 : uint32
{
    NO_BIRTH_ANIM                            = 0x00000001, // SMSG_UPDATE_OBJECT's "NoBirthAnim"
    TREAT_AS_PLAYER_FOR_DIMINISHING_RETURNS  = 0x00000002, // Primarily used by ToC champions
    TREAT_AS_PLAYER_FOR_PVP_DEBUFF_DURATION  = 0x00000004, // Primarily used by ToC champions
    INTERACT_ONLY_WITH_CREATOR               = 0x00000008, // CREATURE_TYPE_FLAG_INTERACT_ONLY_WITH_CREATOR, original description: Only Display Gossip for Summoner
    DO_NOT_PLAY_UNIT_EVENT_SOUNDS            = 0x00000010, // CREATURE_TYPE_FLAG_DO_NOT_PLAY_UNIT_EVENT_SOUNDS, original description: No Death Scream
    HAS_NO_SHADOW_BLOB                       = 0x00000020, // CREATURE_TYPE_FLAG_HAS_NO_SHADOW_BLOB, original description(wrongly linked type flag or behavior was changed?): Can be Healed by Enemies
    DEALS_TRIPLE_DAMAGE_TO_PC_CONTROLLED_PETS = 0x00000040,
    NO_NPC_DAMAGE_BELOW_85PTC                = 0x00000080,
    OBEYS_TAUNT_DIMINISHING_RETURNS          = 0x00000100, // CREATURE_FLAG_EXTRA_OBEYS_TAUNT_DIMINISHING_RETURNS
    NO_MELEE_APPROACH                        = 0x00000200,
    UPDATE_CREATURE_RECORD_WHEN_INSTANCE_CHANGES_DIFFICULTY = 0x00000400, // Used only by Snobold Vassal
    CANNOT_DAZE                              = 0x00000800, // "Cannot Daze (Combat Stun)"
    FLAT_HONOR_AWARD                         = 0x00001000,
    IGNORE_LOS_WHEN_CASTING_ON_ME            = 0x00002000, // "Other objects can ignore line of sight requirements when casting spells on me", used only by Ice Tomb in 3.3.5
    GIVE_QUEST_KILL_CREDIT_WHILE_OFFLINE     = 0x00004000,
    TREAT_AS_RAID_UNIT_FOR_HELPFUL_SPELLS    = 0x00008000, // CREATURE_TYPE_FLAG_TREAT_AS_RAID_UNIT, "Treat as Raid Unit For Helpful Spells (Instances ONLY)", used by Valithria Dreamwalker
    DONT_REPOSITION_IF_MELEE_TARGET_IS_TOO_CLOSE = 0x00010000, // "Don't reposition because melee target is too close"
    PET_OR_GUARDIAN_AI_DONT_GO_BEHIND_TARGET = 0x00020000,
    MINUTE_5_LOOT_ROLL_TIMER                 = 0x00040000, // Used by Lich King
    FORCE_GOSSIP                             = 0x00080000, // CREATURE_TYPE_FLAG_FORCE_GOSSIP
    DONT_REPOSITION_WITH_FRIENDS_IN_COMBAT   = 0x00100000,
    DO_NOT_SHEATHE                           = 0x00200000, // CREATURE_TYPE_FLAG_DO_NOT_SHEATHE, original description: Manual Sheathing control
    IGNORE_SPELL_MIN_RANGE_RESTRICTIONS      = 0x00400000, // UnitFlags2 0x8000000, original description: Attacker Ignores Minimum Ranges
    SUPPRESS_INSTANCE_WIDE_RELEASE_IN_COMBAT = 0x00800000,
    PREVENT_SWIM                             = 0x01000000, // UnitFlags2 0x1000000, original description: AI will only swim if target swims
    HIDE_IN_COMBAT_LOG                       = 0x02000000, // UnitFlags2 0x2000000, original description: Don't generate combat log when engaged with NPC's
    ALLOW_NPC_COMBAT_WHILE_UNINTERACTIBLE    = 0x04000000,
    PREFER_NPCS_WHEN_SEARCHING_FOR_ENEMIES   = 0x08000000,
    ONLY_GENERATE_INITIAL_THREAT             = 0x10000000,
    DO_NOT_TARGET_ON_INTERACTION             = 0x20000000, // CREATURE_TYPE_FLAG_DO_NOT_TARGET_ON_INTERACTION, original description: Doesn't change target on right click
    DO_NOT_RENDER_OBJECT_NAME                = 0x40000000, // CREATURE_TYPE_FLAG_DO_NOT_RENDER_OBJECT_NAME, original description: Hide name in world frame
    QUEST_BOSS                               = 0x80000000  // CREATURE_TYPE_FLAG_QUEST_BOSS
};

DEFINE_ENUM_FLAG(CreatureStaticFlags4);

enum class CreatureStaticFlags5 : uint32
{
    UNTARGETABLE_BY_CLIENT              = 0x00000001, // UnitFlags2 0x4000000 UNIT_FLAG2_UNTARGETABLE_BY_CLIENT
    FORCE_SELF_MOUNTING                 = 0x00000002,
    UNINTERACTIBLE_IF_HOSTILE           = 0x00000004, // UnitFlags2 0x10000000
    DISABLES_XP_AWARD                   = 0x00000008,
    DISABLE_AI_PREDICTION               = 0x00000010,
    NO_LEAVECOMBAT_STATE_RESTORE        = 0x00000020,
    BYPASS_INTERACT_INTERRUPTS          = 0x00000040,
    DEGREE_BACK_ARC_240                 = 0x00000080,
    INTERACT_WHILE_HOSTILE              = 0x00000100, // UnitFlags2 0x4000 UNIT_FLAG2_INTERACT_WHILE_HOSTILE
    DONT_DISMISS_ON_FLYING_MOUNT        = 0x00000200,
    PREDICTIVE_POWER_REGEN              = 0x00000400, // CREATURE_TYPEFLAGS_2_UNK1
    HIDE_LEVEL_INFO_IN_TOOLTIP          = 0x00000800, // CREATURE_TYPEFLAGS_2_UNK2
    HIDE_HEALTH_BAR_UNDER_TOOLTIP       = 0x00001000, // CREATURE_TYPEFLAGS_2_UNK3
    SUPPRESS_HIGHLIGHT_WHEN_TARGETED_OR_MOUSED_OVER = 0x00002000, // UnitFlags2 0x80000
    AI_PREFER_PATHABLE_TARGETS          = 0x00004000,
    FREQUENT_AREA_TRIGGER_CHECKS        = 0x00008000,
    ASSIGN_KILL_CREDIT_TO_ENCOUNTER_LIST= 0x00010000,
    NEVER_EVADE                         = 0x00020000,
    AI_CANT_PATH_ON_STEEP_SLOPES        = 0x00040000,
    AI_IGNORE_LOS_TO_MELEE_TARGET       = 0x00080000,
    NO_TEXT_IN_CHAT_BUBBLE              = 0x00100000, // "Never display emote or chat text in a chat bubble", CREATURE_TYPEFLAGS_2_UNK4
    CLOSE_IN_ON_UNPATHABLE_TARGET       = 0x00200000, // "AI Pets close in on unpathable target"
    DONT_GO_BEHIND_ME                   = 0x00400000, // "Pet/Guardian AI Don't Go Behind Me (use on target)"
    NO_DEATH_THUD                       = 0x00800000, // CREATURE_TYPEFLAGS_2_UNK5
    CLIENT_LOCAL_CREATURE               = 0x01000000,
    CAN_DROP_LOOT_WHILE_IN_A_CHALLENGE_MODE_INSTANCE = 0x02000000,
    HAS_SAFE_LOCATION                   = 0x04000000,
    NO_HEALTH_REGEN                     = 0x08000000,
    NO_POWER_REGEN                      = 0x10000000,
    NO_PET_UNIT_FRAME                   = 0x20000000,
    NO_INTERACT_ON_LEFT_CLICK           = 0x40000000, // CREATURE_TYPEFLAGS_2_UNK6
    GIVE_CRITERIA_KILL_CREDIT_WHEN_CHARMED = 0x80000000
};

DEFINE_ENUM_FLAG(CreatureStaticFlags5);

enum class CreatureStaticFlags6 : uint32
{
    DO_NOT_AUTO_RESUMMON                = 0x00000001, // "Do not auto-resummon this companion creature"
    REPLACE_VISIBLE_UNIT_IF_AVAILABLE   = 0x00000002, // "Smooth Phasing: Replace visible unit if available"
    IGNORE_REALM_COALESCING_HIDING_CODE = 0x00000004, // "Ignore the realm coalescing hiding code (always show)"
    TAPS_TO_FACTION                     = 0x00000008,
    ONLY_QUESTGIVER_FOR_SUMMONER        = 0x00000010,
    AI_COMBAT_RETURN_PRECISE            = 0x00000020,
    HOME_REALM_ONLY_LOOT                = 0x00000040,
    NO_INTERACT_RESPONSE                = 0x00000080, // TFLAG2_UNK7
    NO_INITIAL_POWER                    = 0x00000100,
    DONT_CANCEL_CHANNEL_ON_MASTER_MOUNTING = 0x00000200,
    CAN_TOGGLE_BETWEEN_DEATH_AND_PERSONAL_LOOT = 0x00000400,
    ALWAYS_STAND_ON_TOP_OF_TARGET       = 0x00000800, // "Always, ALWAYS tries to stand right on top of his move to target. ALWAYS!!", toggleable by 'Set "Always Stand on Target" flag for unit(s)' or not same?
    UNCONSCIOUS_ON_DEATH                = 0x00001000,
    DONT_REPORT_TO_LOCAL_DEFENSE_CHANNEL_ON_DEATH = 0x00002000,
    PREFER_UNENGAGED_MONSTERS           = 0x00004000, // "Prefer unengaged monsters when picking a target"
    USE_PVP_POWER_AND_RESILIENCE        = 0x00008000, // "Use PVP power and resilience when players attack this creature"
    DONT_CLEAR_DEBUFFS_ON_LEAVE_COMBAT  = 0x00010000,
    PERSONAL_LOOT_HAS_FULL_SECURITY     = 0x00020000, // "Personal loot has full security (guaranteed push/mail delivery)"
    TRIPLE_SPELL_VISUALS                = 0x00040000,
    USE_GARRISON_OWNER_LEVEL            = 0x00080000,
    IMMEDIATE_AOI_UPDATE_ON_SPAWN       = 0x00100000,
    UI_CAN_GET_POSITION                 = 0x00200000,
    SEAMLESS_TRANSFER_PROHIBITED        = 0x00400000,
    ALWAYS_USE_GROUP_LOOT_METHOD        = 0x00800000,
    NO_BOSS_KILL_BANNER                 = 0x01000000,
    FORCE_TRIGGERING_PLAYER_LOOT_ONLY   = 0x02000000,
    SHOW_BOSS_FRAME_WHILE_UNINTERACTABLE= 0x04000000,
    SCALES_TO_PLAYER_LEVEL              = 0x08000000,
    AI_DONT_LEAVE_MELEE_FOR_RANGED_WHEN_TARGET_GETS_ROOTED = 0x10000000,
    DONT_USE_COMBAT_REACH_FOR_CHAINING  = 0x20000000,
    DO_NOT_PLAY_PROCEDURAL_WOUND_ANIM   = 0x40000000,
    APPLY_PROCEDURAL_WOUND_ANIM_TO_BASE = 0x80000000  // TFLAG2_UNK14
};

DEFINE_ENUM_FLAG(CreatureStaticFlags6);

enum class CreatureStaticFlags7 : uint32
{
    IMPORTANT_NPC                            = 0x00000001,
    IMPORTANT_QUEST_NPC                      = 0x00000002,
    LARGE_NAMEPLATE                          = 0x00000004,
    TRIVIAL_PET                              = 0x00000008,
    AI_ENEMIES_DONT_BACKUP_WHEN_I_GET_ROOTED = 0x00000010,
    NO_AUTOMATIC_COMBAT_ANCHOR               = 0x00000020,
    ONLY_TARGETABLE_BY_CREATOR               = 0x00000040,
    TREAT_AS_PLAYER_FOR_ISPLAYERCONTROLLED   = 0x00000080,
    GENERATE_NO_THREAT_OR_DAMAGE             = 0x00000100,
    INTERACT_ONLY_ON_QUEST                   = 0x00000200,
    DISABLE_KILL_CREDIT_FOR_OFFLINE_PLAYERS  = 0x00000400,
    AI_ADDITIONAL_PATHING                    = 0x00080000,
};

DEFINE_ENUM_FLAG(CreatureStaticFlags7);

enum class CreatureStaticFlags8 : uint32
{
    FORCE_CLOSE_IN_ON_PATH_FAIL_BEHAVIOR     = 0x00000002,
    USE_2D_CHASING_CALCULATION               = 0x00000020,
    USE_FAST_CLASSIC_HEARTBEAT               = 0x00000040,
};

DEFINE_ENUM_FLAG(CreatureStaticFlags8);

#endif