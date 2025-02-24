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

#ifndef _SCRIPTMGR_H
#define _SCRIPTMGR_H

#include "Common.h"
#include "Entities/ObjectGuid.h"
#include "Server/DBCEnums.h"
#include "DBScripts/ScriptMgrDefines.h"

#include <memory>

class Map;
class Object;
class WorldObject;
class Unit;
class Player;
struct SpellEntry;

enum ScriptCommand                                          // resSource, resTarget are the resulting Source/ Target after buddy search is done
{
    SCRIPT_COMMAND_TALK                     = 0,            // resSource = WorldObject, resTarget = Unit/none
    // dataint = text id from broadcast_text table. dataint2-4 optional for random selected texts.
    SCRIPT_COMMAND_EMOTE                    = 1,            // resSource = Unit, resTarget = Unit/none
    // datalong1 = emote_id, dataint1-4 optional for random selected emotes
    SCRIPT_COMMAND_FIELD_SET                = 2,            // source = any, datalong = field_id, datalong2 = value
    SCRIPT_COMMAND_MOVE_TO                  = 3,            // resSource = Creature, datalong1 = relayId, datalong2 = travel_speed*100, datalong3 = forcedMovement, x/y/z/o
    // data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL: teleport unit to position
    SCRIPT_COMMAND_FLAG_SET                 = 4,            // source = any, datalong = field_id, datalong2 = bitmask
    SCRIPT_COMMAND_FLAG_REMOVE              = 5,            // source = any, datalong = field_id, datalong2 = bitmask
    SCRIPT_COMMAND_TELEPORT_TO              = 6,            // source or target with Player, datalong2 = map_id, x/y/z
    SCRIPT_COMMAND_QUEST_EXPLORED           = 7,            // one from source or target must be Player, another GO/Creature, datalong=quest_id, datalong2=distance or 0
    SCRIPT_COMMAND_KILL_CREDIT              = 8,            // source or target with Player, datalong = creature entry (or 0 for target-entry), datalong2 = bool (0=personal credit, 1=group credit)
    SCRIPT_COMMAND_RESPAWN_GAMEOBJECT       = 9,            // source = any, datalong=db_guid, datalong2=despawn_delay
    SCRIPT_COMMAND_TEMP_SPAWN_CREATURE      = 10,           // source = any, datalong=creature entry, datalong2=despawn_delay, datalong3=pathId, text1 - setRun - 1, text2 - faction ID, text3 - modelId
    // data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL = summon active
    // dataint = (bool) setRun; 0 = off (default), 1 = on
    SCRIPT_COMMAND_OPEN_DOOR                = 11,           // datalong=db_guid (or not provided), datalong2=reset_delay
    SCRIPT_COMMAND_CLOSE_DOOR               = 12,           // datalong=db_guid (or not provided), datalong2=reset_delay
    SCRIPT_COMMAND_ACTIVATE_OBJECT          = 13,           // source = unit, target=GO; data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL send gameobject custom anim, datalong = animId
    SCRIPT_COMMAND_REMOVE_AURA              = 14,           // resSource = Unit, datalong = spell_id
    SCRIPT_COMMAND_CAST_SPELL               = 15,           // resSource = Unit, cast spell at resTarget = Unit
    // datalong=spellid
    // datalong2=castFlags, enum TriggerCastFlags
    // dataint1-4 optional for random selected spell
    SCRIPT_COMMAND_PLAY_SOUND               = 16,           // resSource = WorldObject, target=any/player, datalong (sound_id), datalong2 (bitmask: 0/1=target-player, 0/2=with distance dependent, 0/4=map wide, 0/8=zone wide, 0/16=area wide; so 1|2 = 3 is target with distance dependent), 8+16 does not work together
    SCRIPT_COMMAND_CREATE_ITEM              = 17,           // source or target must be player, datalong = item entry, datalong2 = amount
    SCRIPT_COMMAND_DESPAWN_SELF             = 18,           // resSource = Creature, datalong = despawn delay
    SCRIPT_COMMAND_PLAY_MOVIE               = 19,           // target can only be a player, datalog = movie id
    SCRIPT_COMMAND_MOVEMENT                 = 20,           // resSource = Creature. datalong = MovementType (0:idle, 1:random, 2:waypoint, 3:path, 18:fall), datalong2 = wander-distance/pathId/relayId, datalong3 = timer/passTarget, dataint1 = forcedMovement
    // data_flags &  SCRIPT_FLAG_COMMAND_ADDITIONAL = Random-movement around current position
    SCRIPT_COMMAND_SET_ACTIVEOBJECT         = 21,           // resSource = WorldObject
    // datalong=bool 0=off, 1=on
    SCRIPT_COMMAND_SET_FACTION              = 22,           // resSource = Creature
    // datalong=factionId, datalong2=faction_flags
    SCRIPT_COMMAND_MORPH_TO_ENTRY_OR_MODEL  = 23,           // resSource = Creature, datalong=creature entry/modelid
    // data_flags &  SCRIPT_FLAG_COMMAND_ADDITIONAL = use datalong value as modelid explicit
    SCRIPT_COMMAND_MOUNT_TO_ENTRY_OR_MODEL  = 24,           // resSource = Creature, datalong=creature entry/modelid
    // data_flags &  SCRIPT_FLAG_COMMAND_ADDITIONAL = use datalong value as modelid explicit
    SCRIPT_COMMAND_SET_RUN                  = 25,           // resSource = Creature
    // datalong= bool 0=off, 1=on
    SCRIPT_COMMAND_ATTACK_START             = 26,           // resSource = Creature, resTarget = Unit
    SCRIPT_COMMAND_GO_LOCK_STATE            = 27,           // resSource = GameObject
    // datalong= 1=lock, 2=unlock, 4=set not-interactable, 8=set interactable
    SCRIPT_COMMAND_STAND_STATE              = 28,           // resSource = Creature
    // datalong = stand state (enum UnitStandStateType)
    SCRIPT_COMMAND_MODIFY_NPC_FLAGS         = 29,           // resSource = Creature
    // datalong=NPCFlags
    // datalong2:0x00=add, 0x01=remove, 0x02=toggle
    SCRIPT_COMMAND_SEND_TAXI_PATH           = 30,           // datalong = taxi path id (source or target must be player)
    SCRIPT_COMMAND_TERMINATE_SCRIPT         = 31,           // datalong = search for npc entry if provided
    // datalong2= search distance
    // data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL: terminate steps of this script if npc found
    //                                        ELSE: terminate steps of this script if npc not found
    // dataint=diff to change a waittime of current Waypoint Movement
    SCRIPT_COMMAND_PAUSE_WAYPOINTS          = 32,           // resSource = Creature
    // datalong = 0: unpause waypoint 1: pause waypoint
    SCRIPT_COMMAND_RESERVED_1               = 33,           // reserved for 3.x and later
    SCRIPT_COMMAND_TERMINATE_COND           = 34,           // datalong = condition_id, datalong2 = if != 0 then quest_id of quest that will be failed for player's group if the script is terminated
    // data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL terminate when condition is false ELSE terminate when condition is true
    SCRIPT_COMMAND_SEND_AI_EVENT            = 35,           // resSource = Creature, resTarget = Unit
    // datalong = AIEventType
    // datalong2 = radius. If radius isn't provided and the target is a creature, then send AIEvent to target
    SCRIPT_COMMAND_SET_FACING               = 36,           // resSource = Creature, resTarget WorldObject. Turn resSource towards Taget
    // data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL also set TargetGuid of resSource to resTarget. In this case resTarget MUST be Creature/ Player
    // datalong != 0 Reset TargetGuid, Reset orientation
    SCRIPT_COMMAND_MOVE_DYNAMIC             = 37,           // resSource = Creature, resTarget Worldobject.
    // datalong = 0: Move resSource towards resTarget
    // datalong != 0: Move resSource to a random point between datalong2..datalong around resTarget.
    //      orientation != 0: Obtain a random point around resTarget in direction of orientation
    // data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL Obtain a random point around resTarget in direction of resTarget->GetOrientation + orientation
    // for resTarget == resSource and orientation == 0 this will mean resSource moving forward
    SCRIPT_COMMAND_SEND_MAIL                = 38,           // resSource WorldObject, can be nullptr, resTarget Player
    // datalong: Send mailTemplateId from resSource (if provided) to player resTarget
    // datalong2: AlternativeSenderEntry. Use as sender-Entry
    // datalong3: Money
    // dataint1: Delay (>= 0) in Seconds
    SCRIPT_COMMAND_SET_HOVER                  = 39,           // resSource = Creature
    // datalong = bool 0=off, 1=on
    // data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL set/unset byte flag UNIT_BYTE1_FLAG_FLY_ANIM
    SCRIPT_COMMAND_DESPAWN_GO               = 40,           // resTarget = GameObject, datalong = timer in ms
    SCRIPT_COMMAND_RESPAWN                  = 41,           // resSource = Creature. Requires SCRIPT_FLAG_BUDDY_IS_DESPAWNED to find dead or despawned targets
    SCRIPT_COMMAND_SET_EQUIPMENT_SLOTS      = 42,           // resSource = Creature
    // datalong = resetDefault: bool 0=false, 1=true
    // dataint = main hand slot; dataint2 = off hand slot; dataint3 = ranged slot
    SCRIPT_COMMAND_RESET_GO                 = 43,           // resTarget = GameObject
    SCRIPT_COMMAND_UPDATE_TEMPLATE          = 44,           // resSource = Creature
    // datalong = new Creature entry
    // datalong2 = Alliance(0) Horde(1), other values throw error
    SCRIPT_COMMAND_START_RELAY_SCRIPT       = 45,           // datalong = relayId, datalong2 = random template Id
    SCRIPT_COMMAND_CAST_CUSTOM_SPELL        = 46,           // resSource = Unit, cast spell at resTarget = Unit
    // datalong=spellid
    // datalong2=castFlags, enum TriggerCastFlags
    // dataint1-3 define the &bp value for the spell. At least one field is required.
    SCRIPT_COMMAND_INTERRUPT_SPELL          = 47,           // datalong = SpellType enum CurrentSpellTypes
    SCRIPT_COMMAND_MODIFY_UNIT_FLAGS        = 48,           // resSource = Creature
    // datalong=UnitFlags
    // datalong2:0x00=add, 0x01=remove, 0x02=toggle
    SCRIPT_COMMAND_SET_DATA_64              = 49,           // datalong = set data param 1, datalong2 = set data param 2
    SCRIPT_COMMAND_ZONE_PULSE               = 50,           //
    SCRIPT_COMMAND_SPAWN_GROUP              = 51,           // dalalong = command
    SCRIPT_COMMAND_SET_GOSSIP_MENU          = 52,           // datalong = gossip_menu_id
    SCRIPT_COMMAND_SET_WORLDSTATE           = 53,           // dataint = worldstate id, dataint2 = new value, 
    SCRIPT_COMMAND_SET_SHEATHE              = 54,           // dataint = worldstate id, dataint2 = new value,
    SCRIPT_COMMAND_SET_STRING_ID            = 55,           // datalong = string_id id, datalong2 = 0 unapply, 1 apply
    SCRIPT_COMMAND_MEETINGSTONE             = 200,          // datalong = area id - vanilla only
};

#define MAX_TEXT_ID 4                                       // used for SCRIPT_COMMAND_TALK, SCRIPT_COMMAND_EMOTE, SCRIPT_COMMAND_CAST_SPELL, SCRIPT_COMMAND_TERMINATE_SCRIPT

enum ScriptInfoDataFlags
{
    // default: s/b -> t
    SCRIPT_FLAG_BUDDY_AS_TARGET             = 0x001,        // s -> b
    SCRIPT_FLAG_REVERSE_DIRECTION           = 0x002,        // t* -> s* (* result after previous flag is evaluated)
    SCRIPT_FLAG_SOURCE_TARGETS_SELF         = 0x004,        // s* -> s* (* result after previous flag is evaluated)
    SCRIPT_FLAG_COMMAND_ADDITIONAL          = 0x008,        // command dependend
    SCRIPT_FLAG_BUDDY_BY_GUID               = 0x010,        // take the buddy by guid
    SCRIPT_FLAG_BUDDY_IS_PET                = 0x020,        // buddy is a pet
    SCRIPT_FLAG_BUDDY_IS_DESPAWNED          = 0x040,        // buddy is dead or despawned
    SCRIPT_FLAG_BUDDY_BY_POOL               = 0x080,        // buddy should be part of a pool
    SCRIPT_FLAG_BUDDY_BY_SPAWN_GROUP        = 0x100,        // buddy is from spawn group
    SCRIPT_FLAG_ALL_ELIGIBLE_BUDDIES        = 0x200,        // multisource/multitarget - will execute for each eligible
    SCRIPT_FLAG_BUDDY_BY_GO                 = 0x400,        // take the buddy by GO (for commands which can target both creature and GO)
    SCRIPT_FLAG_BUDDY_BY_STRING_ID          = 0x800,        // takes buddy from string id - creature or go
};
#define MAX_SCRIPT_FLAG_VALID               (2 * SCRIPT_FLAG_BUDDY_BY_STRING_ID - 1)

struct ScriptInfo
{
    uint32 id;
    uint32 delay;
    uint32 command;

    union
    {
        struct                                              // SCRIPT_COMMAND_TALK (0)
        {
            uint32 stringTemplateId;                        // datalong
            uint32 empty1;                                  // datalong2
        } talk;

        struct                                              // SCRIPT_COMMAND_EMOTE (1)
        {
            uint32 emoteId;                                 // datalong
            uint32 unused1;                                 // datalong2
        } emote;

        struct                                              // SCRIPT_COMMAND_FIELD_SET (2)
        {
            uint32 fieldId;                                 // datalong
            uint32 fieldValue;                              // datalong2
        } setField;

        struct                                              // SCRIPT_COMMAND_MOVE_TO (3)
        {
            uint32 relayId;                                 // datalong
            uint32 travelSpeed;                             // datalong2
            uint32 forcedMovement;                          // datalong3
        } moveTo;

        struct                                              // SCRIPT_COMMAND_FLAG_SET (4)
        {
            uint32 fieldId;                                 // datalong
            uint32 fieldValue;                              // datalong2
        } setFlag;

        struct                                              // SCRIPT_COMMAND_FLAG_REMOVE (5)
        {
            uint32 fieldId;                                 // datalong
            uint32 fieldValue;                              // datalong2
        } removeFlag;

        struct                                              // SCRIPT_COMMAND_TELEPORT_TO (6)
        {
            uint32 mapId;                                   // datalong
            uint32 empty;                                   // datalong2
        } teleportTo;

        struct                                              // SCRIPT_COMMAND_QUEST_EXPLORED (7)
        {
            uint32 questId;                                 // datalong
            uint32 distance;                                // datalong2
        } questExplored;

        struct                                              // SCRIPT_COMMAND_KILL_CREDIT (8)
        {
            uint32 creatureEntry;                           // datalong
            uint32 isGroupCredit;                           // datalong2
        } killCredit;

        struct                                              // SCRIPT_COMMAND_RESPAWN_GAMEOBJECT (9)
        {
            uint32 goGuid;                                  // datalong
            uint32 despawnDelay;                            // datalong2
        } respawnGo;

        struct                                              // SCRIPT_COMMAND_TEMP_SPAWN_CREATURE (10)
        {
            uint32 creatureEntry;                           // datalong
            uint32 despawnDelay;                            // datalong2
            uint32 pathId;                                  // datalong3
        } summonCreature;

        // datalong unused                                  // SCRIPT_COMMAND_OPEN_DOOR (11)

        struct                                              // SCRIPT_COMMAND_CLOSE_DOOR (12)
        {
            uint32 goGuid;                                  // datalong
            uint32 resetDelay;                              // datalong2
            uint32 alternate;                               // datalong3
        } changeDoor;

        struct                                              // SCRIPT_COMMAND_ACTIVATE_OBJECT (13)
        {
            uint32 animId;                                  // datalong
            uint32 empty2;                                  // datalong;
        } activateObject;

        struct                                              // SCRIPT_COMMAND_REMOVE_AURA (14)
        {
            uint32 spellId;                                 // datalong
            uint32 defaultOrChargeOrStack;                  // datalong2
        } removeAura;

        struct                                              // SCRIPT_COMMAND_CAST_SPELL (15)
        {
            uint32 spellId;                                 // datalong
            uint32 castFlags;                               // datalong2
        } castSpell;

        struct                                              // SCRIPT_COMMAND_PLAY_SOUND (16)
        {
            uint32 soundId;                                 // datalong
            uint32 flags;                                   // datalong2
            uint32 playParameter;                           // datalong3
        } playSound;

        struct                                              // SCRIPT_COMMAND_CREATE_ITEM (17)
        {
            uint32 itemEntry;                               // datalong
            uint32 amount;                                  // datalong2
        } createItem;

        struct                                              // SCRIPT_COMMAND_DESPAWN_SELF (18)
        {
            uint32 despawnDelay;                            // datalong
            uint32 empty;                                   // datalong2
        } despawn;

        struct                                              // SCRIPT_COMMAND_PLAY_MOVIE (19)
        {
            uint32 movieId;                                 // datalong
            uint32 empty;                                   // datalong2
        } playMovie;

        struct                                              // SCRIPT_COMMAND_MOVEMENT (20)
        {
            uint32 movementType;                            // datalong
            uint32 wanderORpathId;                          // datalong2
            uint32 timerOrPassTarget;                       // datalong3
        } movement;

        struct                                              // SCRIPT_COMMAND_SET_ACTIVEOBJECT (21)
        {
            uint32 activate;                                // datalong
            uint32 empty;                                   // datalong2
        } activeObject;

        struct                                              // SCRIPT_COMMAND_SET_FACTION (22)
        {
            uint32 factionId;                               // datalong
            uint32 flags;                                   // datalong2
        } faction;

        struct                                              // SCRIPT_COMMAND_MORPH_TO_ENTRY_OR_MODEL (23)
        {
            uint32 creatureOrModelEntry;                    // datalong
            uint32 empty1;                                  // datalong2
        } morph;

        struct                                              // SCRIPT_COMMAND_MOUNT_TO_ENTRY_OR_MODEL (24)
        {
            uint32 creatureOrModelEntry;                    // datalong
            uint32 speedChange;                             // datalong2
        } mount;

        struct                                              // SCRIPT_COMMAND_SET_RUN (25)
        {
            uint32 run;                                     // datalong
            uint32 empty;                                   // datalong2
        } run;

        // datalong unused                                  // SCRIPT_COMMAND_ATTACK_START (26)

        struct                                              // SCRIPT_COMMAND_GO_LOCK_STATE (27)
        {
            uint32 lockState;                               // datalong
            uint32 empty;                                   // datalong
        } goLockState;

        struct                                              // SCRIPT_COMMAND_STAND_STATE (28)
        {
            uint32 stand_state;                             // datalong
            uint32 unused1;                                 // datalong2
        } standState;

        struct                                              // SCRIPT_COMMAND_MODIFY_NPC_FLAGS (29)
        {
            uint32 flag;                                    // datalong
            uint32 change_flag;                             // datalong2
        } npcFlag;

        struct                                              // SCRIPT_COMMAND_SEND_TAXI_PATH (30)
        {
            uint32 taxiPathId;                              // datalong
            uint32 empty;
        } sendTaxiPath;

        struct                                              // SCRIPT_COMMAND_TERMINATE_SCRIPT (31)
        {
            uint32 npcOrGOEntry;                                // datalong
            uint32 searchDist;                              // datalong2
            uint32 poolId;                                  // datalong3
            // changeWaypointWaitTime                       // dataint
        } terminateScript;

        struct                                              // SCRIPT_COMMAND_PAUSE_WAYPOINTS (32)
        {
            uint32 doPause;                                 // datalong
            uint32 empty;
        } pauseWaypoint;

        struct                                              // SCRIPT_COMMAND_TERMINATE_COND (34)
        {
            uint32 conditionId;                             // datalong
            uint32 failQuest;                               // datalong2
        } terminateCond;

        struct                                              // SCRIPT_COMMAND_SEND_AI_EVENT_AROUND (35)
        {
            uint32 eventType;                               // datalong
            uint32 radius;                                  // datalong2
            uint32 value;                                   // datalong3
        } sendAIEvent;

        struct                                              // SCRIPT_COMMAND_SET_FACING (36)
        {
            uint32 resetFacing;                             // datalong
            uint32 empty;                                   // datalong2
        } setFacing;

        struct                                              // SCRIPT_COMMAND_MOVE_DYNAMIC (37)
        {
            uint32 maxDist;                                 // datalong
            uint32 minDist;                                 // datalong2
            uint32 fixedDist;                               // datalong3
        } moveDynamic;

        struct                                              // SCRIPT_COMMAND_SEND_MAIL (38)
        {
            uint32 mailTemplateId;                          // datalong
            uint32 altSender;                               // datalong2;
            uint32 money;                                   // datalong3;
        } sendMail;

        struct                                              // SCRIPT_COMMAND_SET_HOVER (39)
        {
            uint32 fly;                                     // datalong
            uint32 empty;                                   // datalong2
        } fly;

        struct                                              // SCRIPT_COMMAND_DESPAWN_GO (40)
        {
            uint32 despawnDelay;                            // datalong
        } despawnGo;

        struct                                              // SCRIPT_COMMAND_SET_EQUIPMENT_SLOTS (42)
        {
            uint32 resetDefault;                            // datalong
            uint32 empty;                                   // datalong2
        } setEquipment;

        // datalong unsed                                   // SCRIPT_COMMAND_RESET_GO (43)

        struct                                              // SCRIPT_COMMAND_UPDATE_TEMPLATE (44)
        {
            uint32 newTemplate;                             // datalong
        } updateTemplate;

        struct                                              // SCRIPT_COMMAND_START_RELAY_SCRIPT
        {
            uint32 relayId;                                 // datalong
            uint32 templateId;                              // datalong2
        } relayScript;

        struct                                              // SCRIPT_COMMAND_CAST_CUSTOM_SPELL (46)
        {
            uint32 spellId;                                 // datalong
            uint32 castFlags;                               // datalong2
        } castCustomSpell;

        struct                                              // SCRIPT_COMMAND_INTERRUPT_SPELL (47)
        {
            uint32 currentSpellType;                        // datalong
        } interruptSpell;

        struct                                              // SCRIPT_COMMAND_MODIFY_UNIT_FLAGS (48)
        {
            uint32 flag;                                    // datalong
            uint32 change_flag;                             // datalong2
        } unitFlag;

        struct                                              // SCRIPT_COMMAND_SET_DATA_64 (49)
        {
            uint32 param1;                                  // datalong
            uint32 param2;                                  // datalong2
        } setData64;

        struct                                              // SCRIPT_COMMAND_SPAWN_GROUP (51)
        {
            uint32 command;                                 // datalong
            uint32 data1;                                   // datalong2
            uint32 data2;                                   // datalong3
        } formationData;

        struct                                              // SCRIPT_COMMAND_SET_GOSSIP_MENU (52)
        {
            uint32 gossipMenuId;                            // datalong
        } setGossipMenu;

        // unused                                           // SCRIPT_COMMAND_SET_WORLDSTATE (53)

        struct                                              // SCRIPT_COMMAND_SET_SHEATHE (54)
        {
            uint32 sheatheState;                            // datalong
        } setSheathe;

        struct                                              // SCRIPT_COMMAND_SET_STRING_ID (55)
        {
            uint32 stringId;                                // datalong
            uint32 apply;                                   // datalong2
        } stringId;

        struct
        {
            uint32 data[3];
        } raw;

        struct                                              // SCRIPT_COMMAND_MEETINGSTONE (200)
        {
            uint32 areaId;                                  // datalong
        } meetingstone;
    };

    // Buddy system (entry can be npc or go entry, depending on command)
    uint32 buddyEntry;                                      // buddy_entry
    uint32 searchRadiusOrGuid;                              // search_radius (can also be guid in case of SCRIPT_FLAG_BUDDY_BY_GUID)
    uint32 data_flags;                                      // data_flags

    int32 textId[MAX_TEXT_ID];                              // dataint to dataint4

    union
    {
        struct
        {
            float data[1];
        } rawFloat;
    };

    float x;
    float y;
    float z;
    float o;
    float speed;
    uint32 condition_id;

    ScriptInfo() : id(0), delay(0), command(0), buddyEntry(0), searchRadiusOrGuid(0), data_flags(0), x(0), y(0), z(0), o(0), speed(0), condition_id(0)
    {
        memset(raw.data, 0, sizeof(raw.data));
        memset(textId, 0, sizeof(textId));
    }

    // helpers
    uint32 GetGOGuid() const
    {
        switch (command)
        {
            case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT:
                return respawnGo.goGuid;
            case SCRIPT_COMMAND_OPEN_DOOR:
            case SCRIPT_COMMAND_CLOSE_DOOR:
                return changeDoor.goGuid;
            default:
                return 0;
        }
    }

    bool IsCreatureAndGOBuddy() const
    {
        switch (command)
        {
            case SCRIPT_COMMAND_MOVE_DYNAMIC:
            case SCRIPT_COMMAND_TERMINATE_SCRIPT:
            case SCRIPT_COMMAND_SET_FACING:
                return true;
            default:
                return false;
        }
    }

    bool IsCreatureBuddy() const
    {
        if (IsCreatureAndGOBuddy())
        {
            if (data_flags & SCRIPT_FLAG_BUDDY_BY_GO)
                return false;

            return true;
        }

        switch (command)
        {
            case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT:
            case SCRIPT_COMMAND_OPEN_DOOR:
            case SCRIPT_COMMAND_CLOSE_DOOR:
            case SCRIPT_COMMAND_ACTIVATE_OBJECT:
            case SCRIPT_COMMAND_GO_LOCK_STATE:
            case SCRIPT_COMMAND_DESPAWN_GO:
            case SCRIPT_COMMAND_RESET_GO:
                return false;
            default:
                return true;
        }
    }

    bool IsDeadOrDespawnedBuddy() const
    {
        return (data_flags & SCRIPT_FLAG_BUDDY_IS_DESPAWNED) != 0;
    }

    bool HasAdditionalScriptFlag() const
    {
        switch (command)
        {
            case SCRIPT_COMMAND_MOVE_TO:
            case SCRIPT_COMMAND_TEMP_SPAWN_CREATURE:
            case SCRIPT_COMMAND_REMOVE_AURA:
            case SCRIPT_COMMAND_ACTIVATE_OBJECT:
            case SCRIPT_COMMAND_CAST_SPELL:
            case SCRIPT_COMMAND_PLAY_SOUND:
            case SCRIPT_COMMAND_CREATE_ITEM:
            case SCRIPT_COMMAND_MOVEMENT:
            case SCRIPT_COMMAND_MORPH_TO_ENTRY_OR_MODEL:
            case SCRIPT_COMMAND_MOUNT_TO_ENTRY_OR_MODEL:
            case SCRIPT_COMMAND_TERMINATE_SCRIPT:
            case SCRIPT_COMMAND_TERMINATE_COND:
            case SCRIPT_COMMAND_SET_FACING:
            case SCRIPT_COMMAND_MOVE_DYNAMIC:
            case SCRIPT_COMMAND_SET_HOVER:
                return true;
            default:
                return false;
        }
    }
};

class ScriptAction
{
    public:
        ScriptAction(ScriptMapType scriptType, Map* _map, ObjectGuid _sourceGuid, ObjectGuid _targetGuid, ObjectGuid _ownerGuid, std::shared_ptr<ScriptInfo> _script);

        bool HandleScriptStep();                            // return true IF AND ONLY IF the script should be terminated
        bool ExecuteDbscriptCommand(WorldObject* pSource, WorldObject* pTarget, Object* pSourceOrItem, bool buddyFound);

        const char* GetTableName() const { return m_table; }
        uint32 GetId() const { return m_script->id; }
        ObjectGuid GetSourceGuid() const { return m_sourceGuid; }
        ObjectGuid GetTargetGuid() const { return m_targetGuid; }
        ObjectGuid GetOwnerGuid() const { return m_ownerGuid; }

        bool IsSameScript(const char* table, uint32 id, ObjectGuid sourceGuid, ObjectGuid targetGuid, ObjectGuid ownerGuid) const
        {
            return table == m_table && id == GetId() &&
                   (sourceGuid == m_sourceGuid || !sourceGuid) &&
                   (targetGuid == m_targetGuid || !targetGuid) &&
                   (ownerGuid == m_ownerGuid || !ownerGuid);
        }

    private:
        ScriptMapType m_scriptType;
        const char* m_table;                                // of which table the script was started
        Map* m_map;                                         // Map on which the action will be executed
        ObjectGuid m_sourceGuid;
        ObjectGuid m_targetGuid;
        ObjectGuid m_ownerGuid;                             // owner of source if source is item
        std::shared_ptr<ScriptInfo> m_script;               // pointer to script data

        // Helper functions
        bool GetScriptCommandObject(const ObjectGuid guid, bool includeItem, Object*& resultObject) const;
        std::pair<bool, bool> GetScriptProcessTargets(WorldObject* originalSource, WorldObject* originalTarget, std::vector<WorldObject*>& finalSources, std::vector<WorldObject*>& finalTargets) const;
        bool LogIfNotCreature(WorldObject* pWorldObject) const;
        bool LogIfNotUnit(WorldObject* pWorldObject) const;
        bool LogIfNotGameObject(WorldObject* pWorldObject) const;
        bool LogIfNotPlayer(WorldObject* pWorldObject) const;
        Player* GetPlayerTargetOrSourceAndLog(WorldObject* pSource, WorldObject* pTarget) const;
};

typedef std::multimap < uint32 /*delay*/, std::shared_ptr<ScriptInfo>> ScriptMap;
typedef std::map < uint32 /*id*/, ScriptMap > ScriptMapMap;
typedef std::pair<const char*, ScriptMapMap> ScriptMapMapName;

class ScriptMgr
{
        enum RandomTemplates
        {
            STRING_TEMPLATE = 0,
            RELAY_TEMPLATE = 1,
            MAX_TYPE // must always be last
        };

    public:
        ScriptMgr();
        ~ScriptMgr() {};

        void LoadScriptMap(ScriptMapType scriptType, bool reload = false);

        void LoadStringIds(bool reload = false);

        void LoadDbScriptStrings();
        void LoadDbScriptRandomTemplates();
        void CheckRandomStringTemplates(std::set<int32>& ids);
        void CheckRandomRelayTemplates();

        bool CheckScriptStringTemplateId(uint32 id) const { return m_scriptTemplates[STRING_TEMPLATE].find(id) != m_scriptTemplates[STRING_TEMPLATE].end(); }
        bool CheckScriptRelayTemplateId(uint32 id) const { return m_scriptTemplates[RELAY_TEMPLATE].find(id) != m_scriptTemplates[RELAY_TEMPLATE].end(); }
        typedef std::pair<int32, uint32> ScriptTemplatePair;
        typedef std::vector<ScriptTemplatePair> ScriptTemplateVector;
        void GetScriptStringTemplate(uint32 id, ScriptTemplateVector& stringTemplate) { stringTemplate = m_scriptTemplates[STRING_TEMPLATE][id]; }
        void GetScriptRelayTemplate(uint32 id, ScriptTemplateVector& stringTemplate) { stringTemplate = m_scriptTemplates[RELAY_TEMPLATE][id]; }
        int32 GetRandomScriptTemplateId(uint32 id, uint8 templateType);
        int32 GetRandomScriptStringFromTemplate(uint32 id);
        int32 GetRandomRelayDbscriptFromTemplate(uint32 id);

        static bool CanSpellEffectStartDBScript(SpellEntry const* spellinfo, SpellEffectIndex effIdx);

        static void CollectPossibleEventIds(std::set<uint32>& eventIds);

        std::shared_ptr<ScriptMapMapName> GetScriptMap(ScriptMapType scriptMapType);

        bool ExistsStringId(uint32 stringId);
        std::shared_ptr<StringIdMap> GetStringIdMap() { return m_stringIds; }
        std::shared_ptr<StringIdMapByString> GetStringIdByStringMap() { return m_stringIdsByString; }
    private:
        void LoadScripts(ScriptMapType scriptType);
        void CheckScriptTexts(ScriptMapType scriptType);

        typedef std::vector<std::string> ScriptNameMap;
        typedef std::unordered_map<uint32, uint32> AreaTriggerScriptMap;
        typedef std::unordered_map<uint32, uint32> EventIdScriptMap;
        typedef std::unordered_map<uint32, ScriptTemplateVector> ScriptTemplateMap;

        AreaTriggerScriptMap    m_AreaTriggerScripts;
        EventIdScriptMap        m_EventIdScripts;
        
        ScriptTemplateMap       m_scriptTemplates[MAX_TYPE];
        ScriptTemplateMap       m_scriptTemplatesEquallyChanced[MAX_TYPE];
        ScriptTemplateMap       m_scriptTemplatesExplicitlyChanced[MAX_TYPE];
        ScriptNameMap           m_scriptNames;

        std::shared_ptr<ScriptMapMapName> m_scriptMaps[SCRIPT_TYPE_MAX];

        // SCRIPT_ID
        std::shared_ptr<StringIdMap> m_stringIds;
        std::shared_ptr<StringIdMapByString> m_stringIdsByString;
};

// Starters for events
bool StartEvents_Event(Map* map, uint32 id, Object* source, Object* target, bool isStart = true);

#define sScriptMgr MaNGOS::Singleton<ScriptMgr>::Instance()

#endif
