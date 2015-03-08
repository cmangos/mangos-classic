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
#include "Policies/Singleton.h"
#include "ObjectGuid.h"
#include "DBCEnums.h"
#include "ace/Atomic_Op.h"

struct AreaTriggerEntry;
struct SpellEntry;
class Aura;
class Creature;
class CreatureAI;
class GameObject;
class InstanceData;
class Item;
class Map;
class Object;
class Player;
class Quest;
class SpellCastTargets;
class Unit;
class WorldObject;

enum ScriptCommand                                          // resSource, resTarget are the resulting Source/ Target after buddy search is done
{
    SCRIPT_COMMAND_TALK                     = 0,            // resSource = WorldObject, resTarget = Unit/none
                                                            // dataint = text entry from db_script_string -table. dataint2-4 optional for random selected texts.
    SCRIPT_COMMAND_EMOTE                    = 1,            // resSource = Unit, resTarget = Unit/none
                                                            // datalong1 = emote_id, dataint1-4 optional for random selected emotes
    SCRIPT_COMMAND_FIELD_SET                = 2,            // source = any, datalong = field_id, datalong2 = value
    SCRIPT_COMMAND_MOVE_TO                  = 3,            // resSource = Creature, datalong2 = travel_speed*100, x/y/z
                                                            // data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL: teleport unit to position
    SCRIPT_COMMAND_FLAG_SET                 = 4,            // source = any, datalong = field_id, datalong2 = bitmask
    SCRIPT_COMMAND_FLAG_REMOVE              = 5,            // source = any, datalong = field_id, datalong2 = bitmask
    SCRIPT_COMMAND_TELEPORT_TO              = 6,            // source or target with Player, datalong2 = map_id, x/y/z
    SCRIPT_COMMAND_QUEST_EXPLORED           = 7,            // one from source or target must be Player, another GO/Creature, datalong=quest_id, datalong2=distance or 0
    SCRIPT_COMMAND_KILL_CREDIT              = 8,            // source or target with Player, datalong = creature entry (or 0 for target-entry), datalong2 = bool (0=personal credit, 1=group credit)
    SCRIPT_COMMAND_RESPAWN_GAMEOBJECT       = 9,            // source = any, datalong=db_guid, datalong2=despawn_delay
    SCRIPT_COMMAND_TEMP_SUMMON_CREATURE     = 10,           // source = any, datalong=creature entry, datalong2=despawn_delay
                                                            // data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL = summon active
    SCRIPT_COMMAND_OPEN_DOOR                = 11,           // datalong=db_guid (or not provided), datalong2=reset_delay
    SCRIPT_COMMAND_CLOSE_DOOR               = 12,           // datalong=db_guid (or not provided), datalong2=reset_delay
    SCRIPT_COMMAND_ACTIVATE_OBJECT          = 13,           // source = unit, target=GO
    SCRIPT_COMMAND_REMOVE_AURA              = 14,           // resSource = Unit, datalong = spell_id
    SCRIPT_COMMAND_CAST_SPELL               = 15,           // resSource = Unit, cast spell at resTarget = Unit
                                                            // datalong=spellid
                                                            // dataint1-4 optional for random selected spell
                                                            // data_flags &  SCRIPT_FLAG_COMMAND_ADDITIONAL = cast triggered
    SCRIPT_COMMAND_PLAY_SOUND               = 16,           // resSource = WorldObject, target=any/player, datalong (sound_id), datalong2 (bitmask: 0/1=target-player, 0/2=with distance dependent, 0/4=map wide, 0/8=zone wide; so 1|2 = 3 is target with distance dependent)
    SCRIPT_COMMAND_CREATE_ITEM              = 17,           // source or target must be player, datalong = item entry, datalong2 = amount
    SCRIPT_COMMAND_DESPAWN_SELF             = 18,           // resSource = Creature, datalong = despawn delay
    SCRIPT_COMMAND_PLAY_MOVIE               = 19,           // target can only be a player, datalog = movie id
    SCRIPT_COMMAND_MOVEMENT                 = 20,           // resSource = Creature. datalong = MovementType (0:idle, 1:random or 2:waypoint), datalong2 = wander-distance
                                                            // data_flags &  SCRIPT_FLAG_COMMAND_ADDITIONAL = Random-movement around current position
    SCRIPT_COMMAND_SET_ACTIVEOBJECT         = 21,           // resSource = Creature
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
                                                            // datalong2:0x00=toggle, 0x01=add, 0x02=remove
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
    SCRIPT_COMMAND_SEND_AI_EVENT_AROUND     = 35,           // resSource = Creature, resTarget = Unit
                                                            // datalong = AIEventType
                                                            // datalong2 = radius
    SCRIPT_COMMAND_SET_FACING               = 36,           // resSource = Creature, resTarget WorldObject. Turn resSource towards Taget
                                                            // data_flags & SCRIPT_FLAG_COMMAND_ADDITIONAL also set TargetGuid of resSource to resTarget. In this case resTarget MUST be Creature/ Player
                                                            // datalong != 0 Reset TargetGuid, Reset orientation
    SCRIPT_COMMAND_MOVE_DYNAMIC             = 37,           // resSource = Creature, resTarget Worldobject.
                                                            // datalong = 0: Move resSource towards resTarget
                                                            // datalong != 0: Move resSource to a random point between datalong2..datalong around resTarget.
                                                            //      orientation != 0: Obtain a random point around resTarget in direction of orientation
};

#define MAX_TEXT_ID 4                                       // used for SCRIPT_COMMAND_TALK, SCRIPT_COMMAND_EMOTE, SCRIPT_COMMAND_CAST_SPELL, SCRIPT_COMMAND_TERMINATE_SCRIPT

enum ScriptInfoDataFlags
{
    // default: s/b -> t
    SCRIPT_FLAG_BUDDY_AS_TARGET             = 0x01,         // s -> b
    SCRIPT_FLAG_REVERSE_DIRECTION           = 0x02,         // t* -> s* (* result after previous flag is evaluated)
    SCRIPT_FLAG_SOURCE_TARGETS_SELF         = 0x04,         // s* -> s* (* result after previous flag is evaluated)
    SCRIPT_FLAG_COMMAND_ADDITIONAL          = 0x08,         // command dependend
    SCRIPT_FLAG_BUDDY_BY_GUID               = 0x10,         // take the buddy by guid
    SCRIPT_FLAG_BUDDY_IS_PET                = 0x20,         // buddy is a pet
};
#define MAX_SCRIPT_FLAG_VALID               (2 * SCRIPT_FLAG_BUDDY_IS_PET - 1)

struct ScriptInfo
{
    uint32 id;
    uint32 delay;
    uint32 command;

    union
    {
        // datalong unused                                  // SCRIPT_COMMAND_TALK (0)

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
            uint32 unused1;                                 // datalong
            uint32 travelSpeed;                             // datalong2
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

        struct                                              // SCRIPT_COMMAND_TEMP_SUMMON_CREATURE (10)
        {
            uint32 creatureEntry;                           // datalong
            uint32 despawnDelay;                            // datalong2
        } summonCreature;

        // datalong unused                                  // SCRIPT_COMMAND_OPEN_DOOR (11)

        struct                                              // SCRIPT_COMMAND_CLOSE_DOOR (12)
        {
            uint32 goGuid;                                  // datalong
            uint32 resetDelay;                              // datalong2
        } changeDoor;

        struct                                              // SCRIPT_COMMAND_ACTIVATE_OBJECT (13)
        {
            uint32 empty1;                                  // datalong
            uint32 empty2;                                  // datalong;
        } activateObject;

        struct                                              // SCRIPT_COMMAND_REMOVE_AURA (14)
        {
            uint32 spellId;                                 // datalong
            uint32 empty;                                   // datalong2
        } removeAura;

        struct                                              // SCRIPT_COMMAND_CAST_SPELL (15)
        {
            uint32 spellId;                                 // datalong
            uint32 empty;                                   // datalong2
        } castSpell;

        struct                                              // SCRIPT_COMMAND_PLAY_SOUND (16)
        {
            uint32 soundId;                                 // datalong
            uint32 flags;                                   // datalong2
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
            uint32 wanderDistance;                          // datalong2
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
            uint32 empty1;                                  // datalong2
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
            uint32 npcEntry;                                // datalong
            uint32 searchDist;                              // datalong2
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
        } moveDynamic;

        struct
        {
            uint32 data[2];
        } raw;
    };

    // Buddy system (entry can be npc or go entry, depending on command)
    uint32 buddyEntry;                                      // buddy_entry
    uint32 searchRadiusOrGuid;                              // search_radius (can also be guid in case of SCRIPT_FLAG_BUDDY_BY_GUID)
    uint8 data_flags;                                       // data_flags

    int32 textId[MAX_TEXT_ID];                              // dataint to dataint4

    float x;
    float y;
    float z;
    float o;

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

    bool IsCreatureBuddy() const
    {
        switch (command)
        {
            case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT:
            case SCRIPT_COMMAND_OPEN_DOOR:
            case SCRIPT_COMMAND_CLOSE_DOOR:
            case SCRIPT_COMMAND_ACTIVATE_OBJECT:
            case SCRIPT_COMMAND_GO_LOCK_STATE:
                return false;
            default:
                return true;
        }
    }

    bool HasAdditionalScriptFlag() const
    {
        switch (command)
        {
            case SCRIPT_COMMAND_MOVE_TO:
            case SCRIPT_COMMAND_TEMP_SUMMON_CREATURE:
            case SCRIPT_COMMAND_CAST_SPELL:
            case SCRIPT_COMMAND_MOVEMENT:
            case SCRIPT_COMMAND_MORPH_TO_ENTRY_OR_MODEL:
            case SCRIPT_COMMAND_MOUNT_TO_ENTRY_OR_MODEL:
            case SCRIPT_COMMAND_TERMINATE_SCRIPT:
            case SCRIPT_COMMAND_TERMINATE_COND:
            case SCRIPT_COMMAND_SET_FACING:
                return true;
            default:
                return false;
        }
    }
};

class ScriptAction
{
    public:
        ScriptAction(const char* _table, Map* _map, ObjectGuid _sourceGuid, ObjectGuid _targetGuid, ObjectGuid _ownerGuid, ScriptInfo const* _script) :
            m_table(_table), m_map(_map), m_sourceGuid(_sourceGuid), m_targetGuid(_targetGuid), m_ownerGuid(_ownerGuid), m_script(_script)
        {}

        bool HandleScriptStep();                            // return true IF AND ONLY IF the script should be terminated

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
        const char* m_table;                                // of which table the script was started
        Map* m_map;                                         // Map on which the action will be executed
        ObjectGuid m_sourceGuid;
        ObjectGuid m_targetGuid;
        ObjectGuid m_ownerGuid;                             // owner of source if source is item
        ScriptInfo const* m_script;                         // pointer to static script data

        // Helper functions
        bool GetScriptCommandObject(const ObjectGuid guid, bool includeItem, Object*& resultObject);
        bool GetScriptProcessTargets(WorldObject* pOrigSource, WorldObject* pOrigTarget, WorldObject*& pFinalSource, WorldObject*& pFinalTarget);
        bool LogIfNotCreature(WorldObject* pWorldObject);
        bool LogIfNotUnit(WorldObject* pWorldObject);
        bool LogIfNotGameObject(WorldObject* pWorldObject);
        Player* GetPlayerTargetOrSourceAndLog(WorldObject* pSource, WorldObject* pTarget);
};

typedef std::multimap < uint32 /*delay*/, ScriptInfo > ScriptMap;
typedef std::map < uint32 /*id*/, ScriptMap > ScriptMapMap;
typedef std::pair<const char*, ScriptMapMap> ScriptMapMapName;

extern ScriptMapMapName sQuestEndScripts;
extern ScriptMapMapName sQuestStartScripts;
extern ScriptMapMapName sSpellScripts;
extern ScriptMapMapName sGameObjectScripts;
extern ScriptMapMapName sGameObjectTemplateScripts;
extern ScriptMapMapName sEventScripts;
extern ScriptMapMapName sGossipScripts;
extern ScriptMapMapName sCreatureDeathScripts;
extern ScriptMapMapName sCreatureMovementScripts;

enum ScriptLoadResult
{
    SCRIPT_LOAD_OK,
    SCRIPT_LOAD_ERR_NOT_FOUND,
    SCRIPT_LOAD_ERR_WRONG_API,
    SCRIPT_LOAD_ERR_OUTDATED,
};

class ScriptMgr
{
    public:
        ScriptMgr();
        ~ScriptMgr();

        void LoadGameObjectScripts();
        void LoadGameObjectTemplateScripts();
        void LoadQuestEndScripts();
        void LoadQuestStartScripts();
        void LoadEventScripts();
        void LoadSpellScripts();
        void LoadGossipScripts();
        void LoadCreatureDeathScripts();
        void LoadCreatureMovementScripts();

        void LoadDbScriptStrings();

        void LoadScriptNames();
        void LoadAreaTriggerScripts();
        void LoadEventIdScripts();

        uint32 GetAreaTriggerScriptId(uint32 triggerId) const;
        uint32 GetEventIdScriptId(uint32 eventId) const;

        const char* GetScriptName(uint32 id) const { return id < m_scriptNames.size() ? m_scriptNames[id].c_str() : ""; }
        uint32 GetScriptId(const char* name) const;
        uint32 GetScriptIdsCount() const { return m_scriptNames.size(); }

        ScriptLoadResult LoadScriptLibrary(const char* libName);
        void UnloadScriptLibrary();
        bool IsScriptLibraryLoaded() const { return m_hScriptLib != NULL; }

        uint32 IncreaseScheduledScriptsCount() { return (uint32)++m_scheduledScripts; }
        uint32 DecreaseScheduledScriptCount() { return (uint32)--m_scheduledScripts; }
        uint32 DecreaseScheduledScriptCount(size_t count) { return (uint32)(m_scheduledScripts -= count); }
        bool IsScriptScheduled() const { return m_scheduledScripts > 0; }
        static bool CanSpellEffectStartDBScript(SpellEntry const* spellinfo, SpellEffectIndex effIdx);

        CreatureAI* GetCreatureAI(Creature* pCreature);
        InstanceData* CreateInstanceData(Map* pMap);

        char const* GetScriptLibraryVersion() const;
        bool OnGossipHello(Player* pPlayer, Creature* pCreature);
        bool OnGossipHello(Player* pPlayer, GameObject* pGameObject);
        bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action, const char* code);
        bool OnGossipSelect(Player* pPlayer, GameObject* pGameObject, uint32 sender, uint32 action, const char* code);
        bool OnQuestAccept(Player* pPlayer, Creature* pCreature, Quest const* pQuest);
        bool OnQuestAccept(Player* pPlayer, GameObject* pGameObject, Quest const* pQuest);
        bool OnQuestAccept(Player* pPlayer, Item* pItem, Quest const* pQuest);
        bool OnQuestRewarded(Player* pPlayer, Creature* pCreature, Quest const* pQuest);
        bool OnQuestRewarded(Player* pPlayer, GameObject* pGameObject, Quest const* pQuest);
        uint32 GetDialogStatus(Player* pPlayer, Creature* pCreature);
        uint32 GetDialogStatus(Player* pPlayer, GameObject* pGameObject);
        bool OnGameObjectUse(Player* pPlayer, GameObject* pGameObject);
        bool OnItemUse(Player* pPlayer, Item* pItem, SpellCastTargets const& targets);
        bool OnAreaTrigger(Player* pPlayer, AreaTriggerEntry const* atEntry);
        bool OnProcessEvent(uint32 eventId, Object* pSource, Object* pTarget, bool isStart);
        bool OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Creature* pTarget, ObjectGuid originalCasterGuid);
        bool OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, GameObject* pTarget, ObjectGuid originalCasterGuid);
        bool OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Item* pTarget, ObjectGuid originalCasterGuid);
        bool OnEffectScriptEffect(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Creature* pTarget, ObjectGuid originalCasterGuid);
        bool OnAuraDummy(Aura const* pAura, bool apply);

    private:
        void CollectPossibleEventIds(std::set<uint32>& eventIds);
        void LoadScripts(ScriptMapMapName& scripts, const char* tablename);
        void CheckScriptTexts(ScriptMapMapName const& scripts, std::set<int32>& ids);

        template<class T>
        void GetScriptHookPtr(T& ptr, const char* name)
        {
            ptr = (T)MANGOS_GET_PROC_ADDR(m_hScriptLib, name);
        }

        typedef std::vector<std::string> ScriptNameMap;
        typedef UNORDERED_MAP<uint32, uint32> AreaTriggerScriptMap;
        typedef UNORDERED_MAP<uint32, uint32> EventIdScriptMap;

        AreaTriggerScriptMap    m_AreaTriggerScripts;
        EventIdScriptMap        m_EventIdScripts;

        ScriptNameMap           m_scriptNames;
        MANGOS_LIBRARY_HANDLE   m_hScriptLib;

        // atomic op counter for active scripts amount
        ACE_Atomic_Op<ACE_Thread_Mutex, long> m_scheduledScripts;

        void (MANGOS_IMPORT* m_pOnInitScriptLibrary)();
        void (MANGOS_IMPORT* m_pOnFreeScriptLibrary)();
        const char* (MANGOS_IMPORT* m_pGetScriptLibraryVersion)();

        CreatureAI* (MANGOS_IMPORT* m_pGetCreatureAI)(Creature*);
        InstanceData* (MANGOS_IMPORT* m_pCreateInstanceData)(Map*);

        bool (MANGOS_IMPORT* m_pOnGossipHello)(Player*, Creature*);
        bool (MANGOS_IMPORT* m_pOnGOGossipHello)(Player*, GameObject*);
        bool (MANGOS_IMPORT* m_pOnGossipSelect)(Player*, Creature*, uint32, uint32);
        bool (MANGOS_IMPORT* m_pOnGOGossipSelect)(Player*, GameObject*, uint32, uint32);
        bool (MANGOS_IMPORT* m_pOnGossipSelectWithCode)(Player*, Creature*, uint32, uint32, const char*);
        bool (MANGOS_IMPORT* m_pOnGOGossipSelectWithCode)(Player*, GameObject*, uint32, uint32, const char*);
        bool (MANGOS_IMPORT* m_pOnQuestAccept)(Player*, Creature*, Quest const*);
        bool (MANGOS_IMPORT* m_pOnGOQuestAccept)(Player*, GameObject*, Quest const*);
        bool (MANGOS_IMPORT* m_pOnItemQuestAccept)(Player*, Item*, Quest const*);
        bool (MANGOS_IMPORT* m_pOnQuestRewarded)(Player*, Creature*, Quest const*);
        bool (MANGOS_IMPORT* m_pOnGOQuestRewarded)(Player*, GameObject*, Quest const*);
        uint32(MANGOS_IMPORT* m_pGetNPCDialogStatus)(Player*, Creature*);
        uint32(MANGOS_IMPORT* m_pGetGODialogStatus)(Player*, GameObject*);
        bool (MANGOS_IMPORT* m_pOnGOUse)(Player*, GameObject*);
        bool (MANGOS_IMPORT* m_pOnItemUse)(Player*, Item*, SpellCastTargets const&);
        bool (MANGOS_IMPORT* m_pOnAreaTrigger)(Player*, AreaTriggerEntry const*);
        bool (MANGOS_IMPORT* m_pOnProcessEvent)(uint32, Object*, Object*, bool);
        bool (MANGOS_IMPORT* m_pOnEffectDummyCreature)(Unit*, uint32, SpellEffectIndex, Creature*, ObjectGuid);
        bool (MANGOS_IMPORT* m_pOnEffectDummyGO)(Unit*, uint32, SpellEffectIndex, GameObject*, ObjectGuid);
        bool (MANGOS_IMPORT* m_pOnEffectDummyItem)(Unit*, uint32, SpellEffectIndex, Item*, ObjectGuid);
        bool (MANGOS_IMPORT* m_pOnEffectScriptEffectCreature)(Unit*, uint32, SpellEffectIndex, Creature*, ObjectGuid);
        bool (MANGOS_IMPORT* m_pOnAuraDummy)(Aura const*, bool);
};

// Starters for events
bool StartEvents_Event(Map* map, uint32 id, Object* source, Object* target, bool isStart = true, Unit* forwardToPvp = NULL);

#define sScriptMgr MaNGOS::Singleton<ScriptMgr>::Instance()

MANGOS_DLL_SPEC uint32 GetAreaTriggerScriptId(uint32 triggerId);
MANGOS_DLL_SPEC uint32 GetEventIdScriptId(uint32 eventId);
MANGOS_DLL_SPEC uint32 GetScriptId(const char* name);
MANGOS_DLL_SPEC char const* GetScriptName(uint32 id);
MANGOS_DLL_SPEC uint32 GetScriptIdsCount();

#endif
