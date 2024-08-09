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

#ifndef _OBJECTMGR_H
#define _OBJECTMGR_H

#include "Common.h"
#include "Entities/Object.h"
#include "Entities/Creature.h"
#include "Entities/Player.h"
#include "Entities/GameObject.h"
#include "Quests/QuestDef.h"
#include "Entities/ItemPrototype.h"
#include "Entities/NPCHandler.h"
#include "Database/DatabaseEnv.h"
#include "Maps/Map.h"
#include "Maps/MapPersistentStateMgr.h"
#include "Entities/ObjectGuid.h"
#include "Globals/Conditions.h"
#include "Maps/SpawnGroupDefines.h"
#include "Util/UniqueTrackablePtr.h"

#include <map>
#include <climits>
#include <memory>
#include <tuple>
#include <optional>

class Group;
class Item;
class SQLStorage;
class UnitConditionMgr;
class CombatConditionMgr;
class WorldStateExpressionMgr;

struct UnitConditionEntry;
struct CombatConditionEntry;
struct WorldStateExpressionEntry;

struct GameTele
{
    float  position_x;
    float  position_y;
    float  position_z;
    float  orientation;
    uint32 mapId;
    std::string name;
    std::wstring wnameLow;
};

typedef std::unordered_map<uint32, GameTele > GameTeleMap;

struct AreaTrigger
{
    uint32 entry;
    uint8  requiredLevel;
    uint32 requiredItem;
    uint32 requiredItem2;
    uint32 requiredQuest;
    uint32 target_mapId;
    float  target_X;
    float  target_Y;
    float  target_Z;
    float  target_Orientation;
    uint32 conditionId;
    std::string status_failed_text;

    // Operators
    bool IsMinimal() const
    {
        return requiredLevel == 0 && requiredItem == 0 && requiredItem2 == 0 && requiredQuest == 0;
    }

    bool IsLessOrEqualThan(AreaTrigger const* l) const      // Expected to have same map
    {
        MANGOS_ASSERT(target_mapId == l->target_mapId);
        return requiredLevel <= l->requiredLevel && requiredItem <= l->requiredItem && requiredItem2 <= l->requiredItem2
               && requiredQuest <= l->requiredQuest;
    }
};

struct BroadcastText
{
    uint32 Id;
    std::vector<std::string> maleText;
    std::vector<std::string> femaleText;
    ChatType chatTypeId; // not contained in SMSG_DB_REPLY, but added here for ease of use
    Language languageId;
    // uint32 conditionId;
    // uint32 emotesId;
    // uint32 flags;
    uint32 soundId1;
    // uint32 soundId2;
    uint32 emoteIds[3];
    uint32 emoteDelays[3];
    // uint32 verifiedBuild;

    std::string const& GetText(int32 locIdx, uint8 gender = GENDER_MALE, bool forceGender = false) const
    {
        ++locIdx; // broadcast text has default at position 0
        if ((gender == GENDER_FEMALE || gender == GENDER_NONE) && (forceGender || !femaleText[DEFAULT_LOCALE].empty()))
        {
            if (locIdx >= 0 && femaleText.size() > size_t(locIdx) && !femaleText[locIdx].empty())
                return femaleText[locIdx];
            return femaleText[DEFAULT_LOCALE];
        }
        // else if (gender == GENDER_MALE)
        {
            if (locIdx >= 0 && maleText.size() > size_t(locIdx) && !maleText[locIdx].empty())
                return maleText[locIdx];
            return maleText[DEFAULT_LOCALE];
        }
    }

    BroadcastText() : maleText(DEFAULT_LOCALE + 1), femaleText(DEFAULT_LOCALE + 1) {}
};

typedef std::map<uint32, BroadcastText> BroadcastTextMap;

typedef std::map < uint32/*player guid*/, uint32/*instance*/ > CellCorpseSet;
struct CellObjectGuids
{
    CellGuidSet creatures;
    CellGuidSet gameobjects;
    CellCorpseSet corpses;
};
typedef std::unordered_map < uint32/*cell_id*/, CellObjectGuids > CellObjectGuidsMap;
typedef std::unordered_map < uint32/*mapid*/, CellObjectGuidsMap > MapObjectGuids;

// mangos string ranges
#define MIN_MANGOS_STRING_ID           1                    // 'mangos_string'
#define MAX_MANGOS_STRING_ID           2000000000

struct MangosStringLocale
{
    MangosStringLocale() : SoundId(0), Type(0), LanguageId(LANG_UNIVERSAL), Emote(0), broadcastText(nullptr) { }

    std::vector<std::string> Content;                       // 0 -> default, i -> i-1 locale index
    uint32 SoundId;
    uint8  Type;
    Language LanguageId;
    uint32 Emote;
    BroadcastText const* broadcastText;
};

typedef std::unordered_map<uint32, CreatureSpawnTemplate> CreatureSpawnTemplateMap;
typedef std::unordered_map<uint32 /*guid*/, CreatureData> CreatureDataMap;
typedef CreatureDataMap::value_type CreatureDataPair;

class FindCreatureData
{
    public:
        FindCreatureData(uint32 id, Player* player) : i_id(id), i_player(player),
            i_anyData(nullptr), i_mapData(nullptr), i_mapDist(0.0f), i_spawnedData(nullptr), i_spawnedDist(0.0f) {}

        bool operator()(CreatureDataPair const& dataPair);
        CreatureDataPair const* GetResult() const;

    private:
        uint32 i_id;
        Player* i_player;

        CreatureDataPair const* i_anyData;
        CreatureDataPair const* i_mapData;
        float i_mapDist;
        CreatureDataPair const* i_spawnedData;
        float i_spawnedDist;
};

typedef std::unordered_map<uint32, GameObjectData> GameObjectDataMap;
typedef GameObjectDataMap::value_type GameObjectDataPair;

class FindGOData
{
    public:
        FindGOData(uint32 id, Player* player) : i_id(id), i_player(player),
            i_anyData(nullptr), i_mapData(nullptr), i_mapDist(0.0f), i_spawnedData(nullptr), i_spawnedDist(0.0f) {}

        bool operator()(GameObjectDataPair const& dataPair);
        GameObjectDataPair const* GetResult() const;

    private:
        uint32 i_id;
        Player* i_player;

        GameObjectDataPair const* i_anyData;
        GameObjectDataPair const* i_mapData;
        float i_mapDist;
        GameObjectDataPair const* i_spawnedData;
        float i_spawnedDist;
};

typedef std::unordered_map<uint32, CreatureLocale> CreatureLocaleMap;
typedef std::unordered_map<uint32, GameObjectLocale> GameObjectLocaleMap;
typedef std::unordered_map<uint32, ItemLocale> ItemLocaleMap;
typedef std::unordered_map<uint32, QuestLocale> QuestLocaleMap;
typedef std::unordered_map<uint32, NpcTextLocale> NpcTextLocaleMap;
typedef std::unordered_map<uint32, PageTextLocale> PageTextLocaleMap;
typedef std::unordered_map<int32, MangosStringLocale> MangosStringLocaleMap;
typedef std::unordered_map<uint32, GossipMenuItemsLocale> GossipMenuItemsLocaleMap;
typedef std::unordered_map<uint32, PointOfInterestLocale> PointOfInterestLocaleMap;
typedef std::unordered_map<uint32, AreaTriggerLocale> AreaTriggerLocaleMap;

typedef std::multimap<int32, uint32> ExclusiveQuestGroupsMap;
typedef std::multimap<uint32, ItemRequiredTarget> ItemRequiredTargetMap;
typedef std::multimap<uint32, uint32> QuestRelationsMap;
typedef std::pair<ExclusiveQuestGroupsMap::const_iterator, ExclusiveQuestGroupsMap::const_iterator> ExclusiveQuestGroupsMapBounds;
typedef std::pair<ItemRequiredTargetMap::const_iterator, ItemRequiredTargetMap::const_iterator> ItemRequiredTargetMapBounds;
typedef std::pair<QuestRelationsMap::const_iterator, QuestRelationsMap::const_iterator> QuestRelationsMapBounds;

struct PetLevelInfo
{
    PetLevelInfo() : health(0), mana(0), armor(0) { for (unsigned short& stat : stats) stat = 0; }

    uint16 stats[MAX_STATS];
    uint16 health;
    uint16 mana;
    uint16 armor;
};

// We assume the rate is in general the same for all three types below, but chose to keep three for scalability and customization
struct RepRewardRate
{
    float quest_rate;                                       // We allow rate = 0.0 in database. For this case,
    float creature_rate;                                    // it means that no reputation are given at all
    float spell_rate;                                       // for this faction/rate type.
};

struct ReputationOnKillEntry
{
    uint32 repfaction1;
    uint32 repfaction2;
    bool is_teamaward1;
    uint32 reputation_max_cap1;
    int32 repvalue1;
    bool is_teamaward2;
    uint32 reputation_max_cap2;
    int32 repvalue2;
    bool team_dependent;
};

struct RepSpilloverTemplate
{
    uint32 faction[MAX_SPILLOVER_FACTIONS];
    float faction_rate[MAX_SPILLOVER_FACTIONS];
    uint32 faction_rank[MAX_SPILLOVER_FACTIONS];
};

struct PointOfInterest
{
    uint32 entry;
    float x;
    float y;
    uint32 icon;
    uint32 flags;
    uint32 data;
    std::string icon_name;
};

struct GossipMenuItems
{
    uint32          menu_id;
    uint32          id;
    uint8           option_icon;
    std::string     option_text;
    uint32          option_broadcast_text;
    uint32          option_id;
    uint32          npc_option_npcflag;
    int32           action_menu_id;
    uint32          action_poi_id;
    uint32          action_script_id;
    bool            box_coded;
    std::string     box_text;
    uint32          box_broadcast_text;
    uint16          conditionId;
};

struct GossipMenus
{
    uint32          entry;
    uint32          text_id;
    uint32          script_id;
    uint16          conditionId;
};

typedef std::multimap<uint32, GossipMenus> GossipMenusMap;
typedef std::pair<GossipMenusMap::const_iterator, GossipMenusMap::const_iterator> GossipMenusMapBounds;
typedef std::multimap<uint32, GossipMenuItems> GossipMenuItemsMap;
typedef std::pair<GossipMenuItemsMap::const_iterator, GossipMenuItemsMap::const_iterator> GossipMenuItemsMapBounds;

struct PetCreateSpellEntry
{
    uint32 spellid[4];
};

struct DungeonEncounter
{
    DungeonEncounter(DungeonEncounterEntry const* _dbcEntry, EncounterCreditType _creditType, uint32 _creditEntry, uint32 _lastEncounterDungeon)
        : dbcEntry(_dbcEntry), creditType(_creditType), creditEntry(_creditEntry), lastEncounterDungeon(_lastEncounterDungeon) { }
    DungeonEncounterEntry const* dbcEntry;
    EncounterCreditType creditType;
    uint32 creditEntry;
    uint32 lastEncounterDungeon;
};

typedef std::multimap<uint32, DungeonEncounter> DungeonEncounterMap;
typedef std::pair<DungeonEncounterMap::const_iterator, DungeonEncounterMap::const_iterator> DungeonEncounterMapBounds;

struct TaxiShortcutData
{
    uint32 lengthTakeoff;
    uint32 lengthLanding;
};

typedef std::unordered_multimap <uint32 /*nodeid*/, TaxiShortcutData> TaxiShortcutMap;

struct QuestgiverGreeting
{
    std::string text;
    uint32 emoteId;
    uint32 emoteDelay;
};

struct QuestgiverGreetingLocale
{
    std::vector<std::string> localeText;
};

typedef std::map<uint32, QuestgiverGreeting> QuestgiverGreetingMap;
typedef std::map<uint32, QuestgiverGreetingLocale> QuestgiverGreetingLocaleMap;

enum
{
    QUESTGIVER_CREATURE = 0,
    QUESTGIVER_GAMEOBJECT = 1,
    QUESTGIVER_TYPE_MAX = 2,
};

struct TrainerGreeting
{
    std::string text;
};

struct TrainerGreetingLocale
{
    std::vector<std::string> localeText;
};

typedef std::map<uint32, TrainerGreeting> TrainerGreetingMap;
typedef std::map<uint32, TrainerGreetingLocale> TrainerGreetingLocaleMap;

// NPC gossip text id
typedef std::unordered_map<uint32, uint32> CacheNpcTextIdMap;

typedef std::unordered_map<uint32, VendorItemData> CacheVendorItemMap;
typedef std::unordered_map<uint32, TrainerSpellData> CacheTrainerSpellMap;

enum SkillRangeType
{
    SKILL_RANGE_LANGUAGE,                                   // 300..300
    SKILL_RANGE_LEVEL,                                      // 1..max skill for level
    SKILL_RANGE_MONO,                                       // 1..1, grey monolite bar
    SKILL_RANGE_RANK,                                       // 1..skill for known rank
    SKILL_RANGE_NONE,                                       // 0..0 always
};

SkillRangeType GetSkillRangeType(SkillLineEntry const* pSkill, bool racial);

#define MAX_PLAYER_NAME          12                         // max allowed by client name length
#define MAX_INTERNAL_PLAYER_NAME 15                         // max server internal player name length ( > MAX_PLAYER_NAME for support declined names )
#define MAX_PET_NAME             12                         // max allowed by client name length
#define MAX_CHARTER_NAME         24                         // max allowed by client name length

bool normalizePlayerName(std::string& name, size_t max_len = MAX_INTERNAL_PLAYER_NAME);

struct LanguageDesc
{
    Language lang_id;
    uint32   spell_id;
    uint32   skill_id;
};

extern LanguageDesc lang_description[LANGUAGES_COUNT];
LanguageDesc const* GetLanguageDescByID(uint32 lang);

class PlayerDumpReader;

class HonorStanding
{
    public:
        HonorStanding()
        {
            honorPoints = 0;
            honorKills  = 0;
            guid        = 0;
            rpEarning   = 0;
        }

        float honorPoints;
        uint32 honorKills;
        uint32 guid;
        float rpEarning;

        HonorStanding* GetInfo() { return this; };

        // create the standing order
        bool operator < (const HonorStanding& rhs) const
        {
            return honorPoints > rhs.honorPoints;
        }
};

typedef std::list<HonorStanding> HonorStandingList;

template<typename T>
class IdGenerator
{
    public:                                                 // constructors
        explicit IdGenerator(char const* _name) : m_name(_name), m_nextGuid(1) {}

    public:                                                 // modifiers
        void Set(T val) { m_nextGuid = val; }
        T Generate();

    public:                                                 // accessors
        T GetNextAfterMaxUsed() const { return m_nextGuid; }

    private:                                                // fields
        char const* m_name;
        T m_nextGuid;
};

struct CreatureImmunity
{
    uint32 type;
    uint32 value;
};

typedef std::vector<CreatureImmunity> CreatureImmunityVector;
typedef std::map<uint32, CreatureImmunityVector> CreatureImmunitySetMap;
typedef std::map<uint32, CreatureImmunitySetMap> CreatureImmunityContainer;

struct WorldStateName
{
    int32 Id;
    std::string Name;
};

class ObjectMgr
{
        friend class PlayerDumpReader;

    public:
        ObjectMgr();
        ~ObjectMgr();

        typedef std::unordered_map<uint32, Item*> ItemMap;

        typedef std::unordered_map<uint32, Group*> GroupMap;

        typedef std::unordered_map<uint32, MaNGOS::unique_trackable_ptr<Quest>> QuestMap;

        typedef std::unordered_map<uint32, AreaTrigger> AreaTriggerMap;

        typedef std::unordered_map<uint32, RepRewardRate > RepRewardRateMap;
        typedef std::unordered_map<uint32, ReputationOnKillEntry> RepOnKillMap;
        typedef std::unordered_map<uint32, RepSpilloverTemplate> RepSpilloverTemplateMap;

        typedef std::unordered_map<uint32, PointOfInterest> PointOfInterestMap;


        typedef std::unordered_map<uint32, PetCreateSpellEntry> PetCreateSpellMap;

        std::unordered_map<uint32, std::pair<bool, std::vector<uint32>>> const& GetCreatureSpawnEntry() const { return m_creatureSpawnEntryMap; }

        std::vector<uint32> LoadGameobjectInfo();

        void PackGroupIds();
        Group* GetGroupById(uint32 id) const;
        void AddGroup(Group* group);
        void RemoveGroup(Group* group);
        GroupMap::iterator GetGroupMapBegin() { return mGroupMap.begin(); }
        GroupMap::iterator GetGroupMapEnd() { return mGroupMap.end(); }

        CreatureModelInfo const* GetCreatureModelRandomGender(uint32 display_id) const;
        uint32 GetCreatureModelOtherTeamModel(uint32 modelId) const;

        PetLevelInfo const* GetPetLevelInfo(uint32 creature_id, uint32 level) const;

        PlayerClassInfo const* GetPlayerClassInfo(uint32 class_) const
        {
            if (class_ >= MAX_CLASSES) return nullptr;
            return &playerClassInfo[class_];
        }
        void GetPlayerClassLevelInfo(uint32 class_, uint32 level, PlayerClassLevelInfo* info) const;

        PlayerInfo const* GetPlayerInfo(uint32 race, uint32 class_) const
        {
            if (race   >= MAX_RACES)   return nullptr;
            if (class_ >= MAX_CLASSES) return nullptr;
            PlayerInfo const* info = &playerInfo[race][class_];
            if (info->displayId_m == 0 || info->displayId_f == 0) return nullptr;
            return info;
        }
        void GetPlayerLevelInfo(uint32 race, uint32 class_, uint32 level, PlayerLevelInfo* info) const;

        ObjectGuid GetPlayerGuidByName(std::string name) const;
        int32 GetPlayerMapIdByGUID(ObjectGuid const& guid) const;
        bool GetPlayerNameByGUID(ObjectGuid guid, std::string& name) const;
        Team GetPlayerTeamByGUID(ObjectGuid guid) const;
        uint8 GetPlayerClassByGUID(ObjectGuid guid) const;
        uint32 GetPlayerAccountIdByGUID(ObjectGuid guid) const;
        uint32 GetPlayerAccountIdByPlayerName(const std::string& name) const;

        bool AddTaxiShortcut(const TaxiPathEntry* path, uint32 lengthTakeoff, uint32 lengthLanding);
        bool GetTaxiShortcut(uint32 pathid, TaxiShortcutData& data);
        void LoadTaxiShortcuts();
        uint32 GetNearestTaxiNode(float x, float y, float z, uint32 mapid, Team team) const;
        void GetTaxiPath(uint32 source, uint32 destination, uint32& path, uint32& cost) const;
        uint32 GetTaxiMountDisplayId(uint32 id, Team team) const;

        Quest const* GetQuestTemplate(uint32 quest_id) const
        {
            QuestMap::const_iterator itr = mQuestTemplates.find(quest_id);
            return itr != mQuestTemplates.end() ? itr->second.get() : nullptr;
        }
        QuestMap const& GetQuestTemplates() const { return mQuestTemplates; }

        uint32 GetQuestForAreaTrigger(uint32 Trigger_ID) const
        {
            QuestAreaTriggerMap::const_iterator itr = mQuestAreaTriggerMap.find(Trigger_ID);
            if (itr != mQuestAreaTriggerMap.end())
                return itr->second;
            return 0;
        }
        bool IsTavernAreaTrigger(uint32 Trigger_ID) const
        {
            return mTavernAreaTriggerSet.find(Trigger_ID) != mTavernAreaTriggerSet.end();
        }

        bool IsGameObjectForQuests(uint32 entry) const
        {
            return mGameObjectForQuestSet.find(entry) != mGameObjectForQuestSet.end();
        }

        GossipText const* GetGossipText(uint32 Text_ID) const;

        QuestgiverGreeting const* GetQuestgiverGreetingData(uint32 entry, uint32 type) const;
        TrainerGreeting const* GetTrainerGreetingData(uint32 entry) const;

        AreaTrigger const* GetAreaTrigger(uint32 trigger) const
        {
            AreaTriggerMap::const_iterator itr = mAreaTriggers.find(trigger);
            if (itr != mAreaTriggers.end())
                return &itr->second;
            return nullptr;
        }

        std::vector<uint32> const* GetAllRandomEntries(std::unordered_map<uint32, std::pair<bool, std::vector<uint32>>> const& map, uint32 dbguid) const
        {
            auto itr = map.find(dbguid);
            if (itr != map.end())
                return &((*itr).second).second;
            return nullptr;
        }

        bool IsRandomDbGuidDynguided(std::unordered_map<uint32, std::pair<bool, std::vector<uint32>>> const& map, uint32 dbguid) const
        {
            auto itr = map.find(dbguid);
            if (itr != map.end())
                return (*itr).second.first;
            return false;
        }

        uint32 GetRandomEntry(std::unordered_map<uint32, std::pair<bool, std::vector<uint32>>> const& map, uint32 dbguid) const
        {
            if (auto spawnList = GetAllRandomEntries(map, dbguid))
                return (*spawnList)[irand(0, spawnList->size() - 1)];
            return 0;
        }

        uint32 GetRandomGameObjectEntry(uint32 dbguid) const { return GetRandomEntry(m_gameobjectSpawnEntryMap, dbguid); }
        std::vector<uint32> const* GetAllRandomGameObjectEntries(uint32 dbguid) const { return GetAllRandomEntries(m_gameobjectSpawnEntryMap, dbguid); }
        bool IsGameObjectDbGuidDynGuided(uint32 dbGuid) const { return IsRandomDbGuidDynguided(m_gameobjectSpawnEntryMap, dbGuid); }

        uint32 GetRandomCreatureEntry(uint32 dbguid) const { return GetRandomEntry(m_creatureSpawnEntryMap, dbguid); }
        std::vector<uint32> const* GetAllRandomCreatureEntries(uint32 dbguid) const { return GetAllRandomEntries(m_creatureSpawnEntryMap, dbguid); }
        bool IsCreatureDbGuidDynGuided(uint32 dbGuid) const { return IsRandomDbGuidDynguided(m_creatureSpawnEntryMap, dbGuid); }

        AreaTrigger const* GetGoBackTrigger(uint32 map_id) const;
        AreaTrigger const* GetMapEntranceTrigger(uint32 Map) const;

        RepRewardRate const* GetRepRewardRate(uint32 factionId) const
        {
            RepRewardRateMap::const_iterator itr = m_RepRewardRateMap.find(factionId);
            if (itr != m_RepRewardRateMap.end())
                return &itr->second;

            return nullptr;
        }

        ReputationOnKillEntry const* GetReputationOnKillEntry(uint32 id) const
        {
            RepOnKillMap::const_iterator itr = mRepOnKill.find(id);
            if (itr != mRepOnKill.end())
                return &itr->second;
            return nullptr;
        }

        RepSpilloverTemplate const* GetRepSpilloverTemplate(uint32 factionId) const
        {
            RepSpilloverTemplateMap::const_iterator itr = m_RepSpilloverTemplateMap.find(factionId);
            if (itr != m_RepSpilloverTemplateMap.end())
                return &itr->second;

            return nullptr;
        }

        PointOfInterest const* GetPointOfInterest(uint32 id) const
        {
            PointOfInterestMap::const_iterator itr = mPointsOfInterest.find(id);
            if (itr != mPointsOfInterest.end())
                return &itr->second;
            return nullptr;
        }

        PetCreateSpellEntry const* GetPetCreateSpellEntry(uint32 id) const
        {
            PetCreateSpellMap::const_iterator itr = mPetCreateSpell.find(id);
            if (itr != mPetCreateSpell.end())
                return &itr->second;
            return nullptr;
        }

        // Static wrappers for various accessors
        static GameObjectInfo const* GetGameObjectInfo(uint32 id);                  ///< Wrapper for sGOStorage.LookupEntry
        static Player* GetPlayer(const char* name);         ///< Wrapper for ObjectAccessor::FindPlayerByName
        static Player* GetPlayer(ObjectGuid guid, bool inWorld = true);             ///< Wrapper for ObjectAccessor::FindPlayer
        static CreatureInfo const* GetCreatureTemplate(uint32 id);                  ///< Wrapper for sCreatureStorage.LookupEntry
        static CreatureModelInfo const* GetCreatureModelInfo(uint32 modelid);       ///< Wrapper for sCreatureModelStorage.LookupEntry
        static EquipmentInfo const* GetEquipmentInfo(uint32 entry);                 ///< Wrapper for sEquipmentStorage.LookupEntry
        static CreatureDataAddon const* GetCreatureAddon(uint32 lowguid);           ///< Wrapper for sCreatureDataAddonStorage.LookupEntry
        static CreatureDataAddon const* GetCreatureTemplateAddon(uint32 entry);     ///< Wrapper for sCreatureInfoAddonStorage.LookupEntry
        static ItemPrototype const* GetItemPrototype(uint32 id);                    ///< Wrapper for sItemStorage.LookupEntry
        static InstanceTemplate const* GetInstanceTemplate(uint32 map);             ///< Wrapper for sInstanceTemplate.LookupEntry
        static WorldTemplate const* GetWorldTemplate(uint32 map);                   ///< Wrapper for sWorldTemplate.LookupEntry
        static CreatureConditionalSpawn const* GetCreatureConditionalSpawn(uint32 lowguid); ///< Wrapper for sCreatureConditionalSpawnStore.LookupEntry

        void LoadGroups();
        void LoadQuests();
        void LoadQuestRelations()
        {
            LoadGameobjectQuestRelations();
            LoadGameobjectInvolvedRelations();
            LoadCreatureQuestRelations();
            LoadCreatureInvolvedRelations();
        }
        void LoadGameobjectQuestRelations();
        void LoadGameobjectInvolvedRelations();
        void LoadCreatureQuestRelations();
        void LoadCreatureInvolvedRelations();

        bool LoadMangosStrings(DatabaseType& db, char const* table, int32 min_value, int32 max_value, bool extra_content);
        bool LoadMangosStrings() { return LoadMangosStrings(WorldDatabase, "mangos_string", MIN_MANGOS_STRING_ID, MAX_MANGOS_STRING_ID, false); }
        void LoadPetCreateSpells();
        void LoadCreatureLocales();
        void LoadCreatureTemplates();
        void LoadCreatures();
        void LoadCreatureAddons();
        void LoadCreatureClassLvlStats();
        void LoadCreatureConditionalSpawn();
        void LoadCreatureSpawnDataTemplates();
        void LoadCreatureSpawnEntry();
        void LoadCreatureModelInfo();
        void LoadEquipmentTemplates();
        void LoadGameObjectLocales();
        void LoadGameObjects();
        void LoadGameObjectSpawnEntry();
        void LoadItemPrototypes();
        void LoadItemRequiredTarget();
        void LoadItemLocales();
        void LoadQuestLocales();
        void LoadGossipTextLocales();
        void LoadQuestgiverGreeting();
        void LoadQuestgiverGreetingLocales();
        void LoadTrainerGreetings();
        void LoadTrainerGreetingLocales();
        void LoadPageTextLocales();
        void LoadGossipMenuItemsLocales();
        void LoadPointOfInterestLocales();
        void LoadInstanceEncounters();
        void LoadInstanceTemplate();
        void LoadWorldTemplate();
        void LoadWorldStateNames();
        void LoadConditions();
        void LoadAreatriggerLocales();

        void LoadGossipText();

        void LoadAreaTriggerTeleports();
        void LoadQuestAreaTriggers();
        void LoadTavernAreaTriggers();
        void LoadGameObjectForQuests();

        void LoadItemTexts();
        void LoadPageTexts();

        void LoadPlayerInfo();
        void LoadPetLevelInfo();
        void LoadExplorationBaseXP();
        void LoadPetNames();
        void LoadPetNumber();
        void LoadCorpses();
        void LoadFishingBaseSkillLevel();

        void LoadReputationRewardRate();
        void LoadReputationOnKill();
        void LoadReputationSpilloverTemplate();

        void LoadPointsOfInterest();

        void LoadSpellTemplate();
        void CheckSpellCones();

        void LoadCreatureTemplateSpells(std::shared_ptr<CreatureSpellListContainer> container);
        void LoadCreatureCooldowns();
        void LoadCreatureImmunities();
        std::shared_ptr<CreatureSpellListContainer> LoadCreatureSpellLists();

        void LoadSpawnGroups();

        void LoadGameTele();

        void LoadNpcGossips();

        void LoadGossipMenus();

        void LoadDungeonEncounters();

        void LoadVendorTemplates();
        void LoadVendors() { LoadVendors("npc_vendor", false); }
        void LoadTrainerTemplates();
        void LoadTrainers() { LoadTrainers("npc_trainer", false); }

        void LoadBroadcastText();
        void LoadBroadcastTextLocales();

        std::tuple<std::shared_ptr<std::map<int32, UnitConditionEntry>>, std::shared_ptr<std::map<int32, WorldStateExpressionEntry>>, std::shared_ptr<std::map<int32, CombatConditionEntry>>> LoadConditionsAndExpressions();
        std::shared_ptr<std::map<int32, UnitConditionEntry>> GetUnitConditions();
        std::shared_ptr<std::map<int32, WorldStateExpressionEntry>> GetWorldStateExpressions();
        std::shared_ptr<std::map<int32, CombatConditionEntry>> GetCombatConditions();

        /// @param _map Map* of the map for which to load active entities. If nullptr active entities on continents are loaded
        void LoadActiveEntities(Map* _map);

        std::string GeneratePetName(uint32 entry);
        uint32 GetBaseXP(uint32 level) const;
        uint32 GetXPForLevel(uint32 level) const;
        uint32 GetXPForPetLevel(uint32 level) const { return GetXPForLevel(level) / 4; }

        int32 GetFishingBaseSkillLevel(uint32 entry) const
        {
            FishingBaseSkillMap::const_iterator itr = mFishingBaseForArea.find(entry);
            return itr != mFishingBaseForArea.end() ? itr->second : 0;
        }

        static std::optional<HonorStanding> GetHonorStandingByGUID(uint32 guid, uint32 side);
        static std::optional<HonorStanding> GetHonorStandingByPosition(uint32 position, uint32 side);
        HonorStandingList GetStandingListBySide(uint32 side);
        uint32 GetHonorStandingPositionByGUID(uint32 guid, uint32 side);
        void UpdateHonorStandingByGuid(uint32 guid, HonorStanding standing, uint32 side) ;
        void FlushRankPoints(uint32 dateTop);
        void DistributeRankPoints(uint32 team, uint32 dateBegin, bool flush = false);
        void LoadStandingList(uint32 dateBegin);
        void LoadStandingList();

        void ReturnOrDeleteOldMails(bool serverUp);

        void SetHighestGuids();

        // used for set initial guid counter for map local guids
        uint32 GetFirstTemporaryCreatureLowGuid() const { return m_FirstTemporaryCreatureGuid; }
        uint32 GetFirstTemporaryGameObjectLowGuid() const { return m_FirstTemporaryGameObjectGuid; }

        // used in .npc add/.gobject add commands for adding static spawns
        uint32 GenerateStaticCreatureLowGuid() { if (m_StaticCreatureGuids.GetNextAfterMaxUsed() >= m_FirstTemporaryCreatureGuid) return 0; return m_StaticCreatureGuids.Generate(); }
        uint32 GenerateStaticGameObjectLowGuid() { if (m_StaticGameObjectGuids.GetNextAfterMaxUsed() >= m_FirstTemporaryGameObjectGuid) return 0; return m_StaticGameObjectGuids.Generate(); }

        uint32 GeneratePlayerLowGuid() { return m_CharGuids.Generate(); }
        uint32 GenerateItemLowGuid() { return m_ItemGuids.Generate(); }
        uint32 GenerateCorpseLowGuid() { return m_CorpseGuids.Generate(); }

        uint32 GenerateAuctionID() { return m_AuctionIds.Generate(); }
        uint32 GenerateGuildId() { return m_GuildIds.Generate(); }
        uint32 GenerateGroupId() { return m_GroupIds.Generate(); }
        uint32 GenerateItemTextID() { return m_ItemTextIds.Generate(); }
        uint32 GenerateMailID() { return m_MailIds.Generate(); }
        uint32 GeneratePetNumber() { return m_PetNumbers.Generate(); }

        uint32 CreateItemText(std::string text);
        void AddItemText(uint32 itemTextId, const std::string& text) { mItemTexts[itemTextId] = text; }
        std::string GetItemText(uint32 id)
        {
            ItemTextMap::const_iterator itr = mItemTexts.find(id);
            if (itr != mItemTexts.end())
                return itr->second;
            else
                return "There is no info for this item";
        }

        CreatureDataPair const* GetCreatureDataPair(uint32 guid) const
        {
            CreatureDataMap::const_iterator itr = mCreatureDataMap.find(guid);
            if (itr == mCreatureDataMap.end()) return nullptr;
            return &*itr;
        }

        CreatureData const* GetCreatureData(uint32 guid) const
        {
            CreatureDataPair const* dataPair = GetCreatureDataPair(guid);
            return dataPair ? &dataPair->second : nullptr;
        }

        CreatureSpawnTemplate const* GetCreatureSpawnTemplate(uint32 entry) const
        {
            auto itr = m_creatureSpawnTemplateMap.find(entry);
            return itr != m_creatureSpawnTemplateMap.end() ? &(*itr).second : nullptr;
        }

        CreatureData* GetCreatureData(uint32 guid)
        {
            auto dataItr = mCreatureDataMap.find(guid);
            if (dataItr != mCreatureDataMap.end())
                return &dataItr->second;
            return nullptr;
        }

        CreatureData& NewOrExistCreatureData(uint32 guid) { return mCreatureDataMap[guid]; }
        void DeleteCreatureData(uint32 guid);

        template<typename Worker>
        void DoCreatureData(Worker& worker) const
        {
            for (CreatureDataMap::const_iterator itr = mCreatureDataMap.begin(); itr != mCreatureDataMap.end(); ++itr)
                if (worker(*itr))
                    break;
        }

        CreatureLocale const* GetCreatureLocale(uint32 entry) const
        {
            CreatureLocaleMap::const_iterator itr = mCreatureLocaleMap.find(entry);
            if (itr == mCreatureLocaleMap.end()) return nullptr;
            return &itr->second;
        }

        void GetCreatureLocaleStrings(uint32 entry, int32 loc_idx, char const** namePtr, char const** subnamePtr = nullptr) const;

        GameObjectLocale const* GetGameObjectLocale(uint32 entry) const
        {
            GameObjectLocaleMap::const_iterator itr = mGameObjectLocaleMap.find(entry);
            if (itr == mGameObjectLocaleMap.end()) return nullptr;
            return &itr->second;
        }

        ItemLocale const* GetItemLocale(uint32 entry) const
        {
            ItemLocaleMap::const_iterator itr = mItemLocaleMap.find(entry);
            if (itr == mItemLocaleMap.end()) return nullptr;
            return &itr->second;
        }

        void GetItemLocaleStrings(uint32 entry, int32 loc_idx, std::string* namePtr, std::string* descriptionPtr = nullptr) const;

        QuestLocale const* GetQuestLocale(uint32 entry) const
        {
            QuestLocaleMap::const_iterator itr = mQuestLocaleMap.find(entry);
            if (itr == mQuestLocaleMap.end()) return nullptr;
            return &itr->second;
        }

        void GetQuestLocaleStrings(uint32 entry, int32 loc_idx, std::string* titlePtr) const;

        NpcTextLocale const* GetNpcTextLocale(uint32 entry) const
        {
            NpcTextLocaleMap::const_iterator itr = mNpcTextLocaleMap.find(entry);
            if (itr == mNpcTextLocaleMap.end()) return nullptr;
            return &itr->second;
        }

        BroadcastText const* GetBroadcastText(uint32 entry) const
        {
            auto itr = m_broadcastTextMap.find(entry);
            if (itr == m_broadcastTextMap.end()) return nullptr;
            return &itr->second;
        }

        typedef std::string NpcTextArray[MAX_GOSSIP_TEXT_OPTIONS];
        void GetNpcTextLocaleStringsAll(uint32 entry, int32 loc_idx, NpcTextArray* text0_Ptr, NpcTextArray* text1_Ptr) const;
        void GetNpcTextLocaleStrings0(uint32 entry, int32 loc_idx, std::string* text0_0_Ptr, std::string* text1_0_Ptr) const;

        void GetQuestgiverGreetingLocales(uint32 entry, uint32 type, int32 loc_idx, std::string* titlePtr) const;

        QuestgiverGreetingLocale const* GetQuestgiverGreetingLocale(uint32 entry, uint32 type) const
        {
            auto itr = m_questgiverGreetingLocaleMap[type].find(entry);
            if (itr == m_questgiverGreetingLocaleMap[type].end()) return nullptr;
            return &itr->second;
        }

        void GetTrainerGreetingLocales(uint32 entry, int32 loc_idx, std::string* titlePtr) const;

        TrainerGreetingLocale const* GetTrainerGreetingLocale(uint32 entry) const
        {
            auto itr = m_trainerGreetingLocaleMap.find(entry);
            if (itr == m_trainerGreetingLocaleMap.end()) return nullptr;
            return &itr->second;
        }

        void GetAreaTriggerLocales(uint32 entry, int32 loc_idx, std::string* titlePtr) const;

        AreaTriggerLocale const* GetAreaTriggerLocale(uint32 entry) const
        {
            auto itr = m_areaTriggerLocaleMap.find(entry);
            if (itr == m_areaTriggerLocaleMap.end()) return nullptr;
            return &itr->second;
        }

        PageTextLocale const* GetPageTextLocale(uint32 entry) const
        {
            PageTextLocaleMap::const_iterator itr = mPageTextLocaleMap.find(entry);
            if (itr == mPageTextLocaleMap.end()) return nullptr;
            return &itr->second;
        }

        GossipMenuItemsLocale const* GetGossipMenuItemsLocale(uint32 entry) const
        {
            GossipMenuItemsLocaleMap::const_iterator itr = mGossipMenuItemsLocaleMap.find(entry);
            if (itr == mGossipMenuItemsLocaleMap.end()) return nullptr;
            return &itr->second;
        }

        PointOfInterestLocale const* GetPointOfInterestLocale(uint32 poi_id) const
        {
            PointOfInterestLocaleMap::const_iterator itr = mPointOfInterestLocaleMap.find(poi_id);
            if (itr == mPointOfInterestLocaleMap.end()) return nullptr;
            return &itr->second;
        }

        GameObjectDataPair const* GetGODataPair(uint32 guid) const
        {
            GameObjectDataMap::const_iterator itr = mGameObjectDataMap.find(guid);
            if (itr == mGameObjectDataMap.end()) return nullptr;
            return &*itr;
        }

        GameObjectData const* GetGOData(uint32 guid) const
        {
            GameObjectDataPair const* dataPair = GetGODataPair(guid);
            return dataPair ? &dataPair->second : nullptr;
        }

        GameObjectData& NewGOData(uint32 guid) { return mGameObjectDataMap[guid]; }
        void DeleteGOData(uint32 guid);

        template<typename Worker>
        void DoGOData(Worker& worker) const
        {
            for (GameObjectDataMap::const_iterator itr = mGameObjectDataMap.begin(); itr != mGameObjectDataMap.end(); ++itr)
                if (worker(*itr))                           // arg = GameObjectDataPair
                    break;
        }

        MangosStringLocale const* GetMangosStringLocale(int32 entry) const
        {
            MangosStringLocaleMap::const_iterator itr = mMangosStringLocaleMap.find(entry);
            if (itr == mMangosStringLocaleMap.end()) return nullptr;
            return &itr->second;
        }
        uint32 GetLoadedStringsCount(int32 minEntry) const
        {
            std::map<int32, uint32>::const_iterator itr = m_loadedStringCount.find(minEntry);
            if (itr != m_loadedStringCount.end())
                return itr->second;
            return 0;
        }

        const char* GetMangosString(int32 entry, int locale_idx) const;
        inline const char* GetMangosStringForDbcLocale(int32 entry) const { return GetMangosString(entry, m_Dbc2StorageLocaleIndex); }

        int GetDbc2StorageLocaleIndex() const { return m_Dbc2StorageLocaleIndex; }
        void SetDbc2StorageLocaleIndex(LocaleConstant loc) { m_Dbc2StorageLocaleIndex = GetStorageLocaleIndexFor(loc); }

        int GetStorageLocaleIndexFor(LocaleConstant loc);
        int GetOrNewStorageLocaleIndexFor(LocaleConstant loc);

        // global grid objects state (static DB spawns, global spawn mods from gameevent system)
        CellObjectGuids const& GetCellObjectGuids(uint16 mapid, uint32 cell_id)
        {
            return mMapObjectGuids[mapid][cell_id];
        }

        // modifiers for global grid objects state (static DB spawns, global spawn mods from gameevent system)
        // Don't must be used for modify instance specific spawn state modifications
        void AddCreatureToGrid(uint32 guid, CreatureData const* data);
        void RemoveCreatureFromGrid(uint32 guid, CreatureData const* data);
        void AddGameobjectToGrid(uint32 guid, GameObjectData const* data);
        void RemoveGameobjectFromGrid(uint32 guid, GameObjectData const* data);
        void AddCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid, uint32 instance);
        void DeleteCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid);

        // reserved names
        void LoadReservedPlayersNames();
        bool IsReservedName(const std::string& name) const;

        static bool CheckPublicMessageLanguage(const std::string& message);

        // name with valid structure and symbols
        static uint8 CheckPlayerName(const std::string& name, bool create = false);
        static PetNameInvalidReason CheckPetName(const std::string& name);
        static bool IsValidCharterName(const std::string& name);

        // Check if a player meets condition conditionId
        bool IsConditionSatisfied(uint32 conditionId, WorldObject const* target, Map const* map, WorldObject const* source, ConditionSource conditionSourceType) const;
        bool IsWorldStateExpressionSatisfied(int32 expressionId, Map const* map);
        bool IsUnitConditionSatisfied(int32 conditionId, Unit const* source, Unit const* target);
        bool IsCombatConditionSatisfied(int32 expressionId, Unit const* source, float range);

        GameTele const* GetGameTele(uint32 id) const
        {
            GameTeleMap::const_iterator itr = m_GameTeleMap.find(id);
            if (itr == m_GameTeleMap.end()) return nullptr;
            return &itr->second;
        }

        GameTele const* GetGameTele(const std::string& name) const;
        GameTeleMap const& GetGameTeleMap() const { return m_GameTeleMap; }
        bool AddGameTele(GameTele& tele);
        bool DeleteGameTele(const std::string& name);

        uint32 GetNpcGossip(uint32 entry) const
        {
            CacheNpcTextIdMap::const_iterator iter = m_mCacheNpcTextIdMap.find(entry);
            if (iter == m_mCacheNpcTextIdMap.end())
                return 0;

            return iter->second;
        }

        TrainerSpellData const* GetNpcTrainerSpells(uint32 entry) const
        {
            CacheTrainerSpellMap::const_iterator iter = m_mCacheTrainerSpellMap.find(entry);
            if (iter == m_mCacheTrainerSpellMap.end())
                return nullptr;

            return &iter->second;
        }

        TrainerSpellData const* GetNpcTrainerTemplateSpells(uint32 entry) const
        {
            CacheTrainerSpellMap::const_iterator iter = m_mCacheTrainerTemplateSpellMap.find(entry);
            if (iter == m_mCacheTrainerTemplateSpellMap.end())
                return nullptr;

            return &iter->second;
        }

        VendorItemData const* GetNpcVendorItemList(uint32 entry) const
        {
            CacheVendorItemMap::const_iterator  iter = m_mCacheVendorItemMap.find(entry);
            if (iter == m_mCacheVendorItemMap.end())
                return nullptr;

            return &iter->second;
        }

        VendorItemData const* GetNpcVendorTemplateItemList(uint32 entry) const
        {
            CacheVendorItemMap::const_iterator  iter = m_mCacheVendorTemplateItemMap.find(entry);
            if (iter == m_mCacheVendorTemplateItemMap.end())
                return nullptr;

            return &iter->second;
        }

        void AddVendorItem(uint32 entry, uint32 item, uint32 maxcount, uint32 incrtime);
        bool RemoveVendorItem(uint32 entry, uint32 item);
        bool IsVendorItemValid(bool isTemplate, char const* tableName, uint32 vendor_entry, uint32 item_id, uint32 maxcount, uint32 incrtime, uint16 conditionId, Player* pl = nullptr) const;

        ItemRequiredTargetMapBounds GetItemRequiredTargetMapBounds(uint32 uiItemEntry) const
        {
            return m_ItemRequiredTarget.equal_range(uiItemEntry);
        }

        DungeonEncounterMapBounds GetDungeonEncounterBounds(uint32 creditEntry) const
        {
            return m_DungeonEncounters.equal_range(creditEntry);
        }

        DungeonEncounterMapBounds GetDungeonEncounterBoundsByMap(uint32 mapId) const
        {
            return m_DungeonEncountersByMap.equal_range(mapId);
        }

        // check if an entry on some map have is an encounter
        bool IsEncounter(uint32 creditEntry, uint32 mapId) const;

        bool IsExistingGossipMenuId(uint32 menuId)
        {
            return m_mGossipMenusMap.find(menuId) != m_mGossipMenusMap.end();
        }
        GossipMenusMapBounds GetGossipMenusMapBounds(uint32 uiMenuId) const
        {
            return m_mGossipMenusMap.equal_range(uiMenuId);
        }

        GossipMenuItemsMapBounds GetGossipMenuItemsMapBounds(uint32 uiMenuId) const
        {
            return m_mGossipMenuItemsMap.equal_range(uiMenuId);
        }

        ExclusiveQuestGroupsMapBounds GetExclusiveQuestGroupsMapBounds(int32 groupId) const
        {
            return m_ExclusiveQuestGroups.equal_range(groupId);
        }

        QuestRelationsMapBounds GetCreatureQuestRelationsMapBounds(uint32 entry) const
        {
            return m_CreatureQuestRelations.equal_range(entry);
        }

        QuestRelationsMapBounds GetCreatureQuestInvolvedRelationsMapBounds(uint32 entry) const
        {
            return m_CreatureQuestInvolvedRelations.equal_range(entry);
        }

        QuestRelationsMapBounds GetGOQuestRelationsMapBounds(uint32 entry) const
        {
            return m_GOQuestRelations.equal_range(entry);
        }

        QuestRelationsMapBounds GetGOQuestInvolvedRelationsMapBounds(uint32 entry) const
        {
            return m_GOQuestInvolvedRelations.equal_range(entry);
        }

        QuestRelationsMap& GetCreatureQuestRelationsMap() { return m_CreatureQuestRelations; }

        std::pair<uint32, uint32> GetCreatureCooldownRange(uint32 entry, uint32 spellId) const
        {
            auto itrEntry = m_creatureCooldownMap.find(entry);
            if (itrEntry == m_creatureCooldownMap.end())
                return {0, 0};

            auto& map = itrEntry->second;
            auto itrSpell = map.find(spellId);
            if (itrSpell == map.end())
                return { 0, 0 };

            return { itrSpell->second.first, itrSpell->second.second };
        }

        bool GetCreatureCooldown(uint32 entry, uint32 spellId, uint32& cooldown) const
        {
            auto itrEntry = m_creatureCooldownMap.find(entry);
            if (itrEntry == m_creatureCooldownMap.end())
                return false;
            auto& map = itrEntry->second;
            auto itrSpell = map.find(spellId);
            if (itrSpell == map.end())
                return false;
            cooldown = urand(itrSpell->second.first, itrSpell->second.second);
            return true;
        }

        /**
        * \brief: Data returned is used to compute health, mana, armor, damage of creatures. May be nullptr.
        * \param uint32 level               creature level
        * \param uint32 unitClass           creature class, related to CLASSMASK_ALL_CREATURES
        * \return: CreatureClassLvlStats const* or nullptr
        *
        * Description: GetCreatureClassLvlStats give fast access to creature stats data.
        * FullName: ObjectMgr::GetCreatureClassLvlStats
        * Access: public
        * Qualifier: const
        **/
        CreatureClassLvlStats const* GetCreatureClassLvlStats(uint32 level, uint32 unitClass) const;

        CreatureImmunityVector const* GetCreatureImmunitySet(uint32 entry, uint32 setId) const;

        CreatureSpellList* GetCreatureSpellList(uint32 Id) const; // only for starttime checks - else use Map
        std::shared_ptr<CreatureSpellListContainer> GetCreatureSpellListContainer() { return m_spellListContainer; }
        std::shared_ptr<SpawnGroupEntryContainer> GetSpawnGroupContainer() { return m_spawnGroupContainer; }

        bool HasWorldStateName(int32 Id) const;
        WorldStateName* GetWorldStateName(int32 Id);

        std::vector<uint32>* GetCreatureDynGuidForMap(uint32 mapId);
        std::vector<uint32>* GetGameObjectDynGuidForMap(uint32 mapId);

        // these must be called from world thread
        void AddDynGuidForMap(uint32 mapId, std::pair<std::vector<uint32>, std::vector<uint32>> const& dbGuids);
        void RemoveDynGuidForMap(uint32 mapId, std::pair<std::vector<uint32>, std::vector<uint32>> const& dbGuids);

        uint32 GetMaxGoDbGuid() const { return m_maxGoDbGuid; }
        uint32 GetMaxCreatureDbGuid() const { return m_maxCreatureDbGuid; }

        uint32 GetTypeFlagsFromStaticFlags(CreatureTypeFlags typeFlags, uint32 staticFlags1, uint32 staticFlags2, uint32 staticFlags3, uint32 staticFlags4) const;
    protected:

        // current locale settings
        uint8   m_Dbc2StorageLocaleIndex;

        // first free id for selected id type
        IdGenerator<uint32> m_AuctionIds;
        IdGenerator<uint32> m_GuildIds;
        IdGenerator<uint32> m_ItemTextIds;
        IdGenerator<uint32> m_MailIds;
        IdGenerator<uint32> m_PetNumbers;
        IdGenerator<uint32> m_GroupIds;

        // initial free low guid for selected guid type for map local guids
        uint32 m_FirstTemporaryCreatureGuid;
        uint32 m_FirstTemporaryGameObjectGuid;

        // guids from reserved range for use in .npc add/.gobject add commands for adding new static spawns (saved in DB) from client.
        ObjectGuidGenerator<HIGHGUID_UNIT>        m_StaticCreatureGuids;
        ObjectGuidGenerator<HIGHGUID_GAMEOBJECT>  m_StaticGameObjectGuids;

        // first free low guid for selected guid type
        ObjectGuidGenerator<HIGHGUID_PLAYER>     m_CharGuids;
        ObjectGuidGenerator<HIGHGUID_ITEM>       m_ItemGuids;
        ObjectGuidGenerator<HIGHGUID_CORPSE>     m_CorpseGuids;

        QuestMap            mQuestTemplates;

        typedef std::unordered_map<uint32, GossipText> GossipTextMap;
        typedef std::unordered_map<uint32, uint32> QuestAreaTriggerMap;
        typedef std::unordered_map<uint32, std::string> ItemTextMap;
        typedef std::set<uint32> TavernAreaTriggerSet;
        typedef std::set<uint32> GameObjectForQuestSet;

        GroupMap            mGroupMap;

        ItemTextMap         mItemTexts;

        QuestAreaTriggerMap mQuestAreaTriggerMap;
        TavernAreaTriggerSet mTavernAreaTriggerSet;
        GameObjectForQuestSet mGameObjectForQuestSet;
        GossipTextMap       mGossipText;
        AreaTriggerMap      mAreaTriggers;

        RepRewardRateMap    m_RepRewardRateMap;
        RepOnKillMap        mRepOnKill;
        RepSpilloverTemplateMap m_RepSpilloverTemplateMap;

        GossipMenusMap      m_mGossipMenusMap;
        GossipMenuItemsMap  m_mGossipMenuItemsMap;

        std::unordered_map<uint32, std::pair<bool, std::vector<uint32>>> m_creatureSpawnEntryMap;
        std::unordered_map<uint32, std::pair<bool, std::vector<uint32>>> m_gameobjectSpawnEntryMap;
		
        PointOfInterestMap  mPointsOfInterest;

        PetCreateSpellMap   mPetCreateSpell;

        // character reserved names
        typedef std::set<std::wstring> ReservedNamesMap;
        ReservedNamesMap    m_ReservedNames;

        TaxiShortcutMap     m_TaxiShortcutMap;

        GameTeleMap         m_GameTeleMap;

        ItemRequiredTargetMap m_ItemRequiredTarget;

        typedef             std::vector<LocaleConstant> LocalForIndex;
        LocalForIndex        m_LocalForIndex;

        ExclusiveQuestGroupsMap m_ExclusiveQuestGroups;

        QuestRelationsMap       m_CreatureQuestRelations;
        QuestRelationsMap       m_CreatureQuestInvolvedRelations;
        QuestRelationsMap       m_GOQuestRelations;
        QuestRelationsMap       m_GOQuestInvolvedRelations;

    private:
        void LoadCreatureAddons(SQLStorage& creatureaddons, char const* entryName, char const* comment);
        void ConvertCreatureAddonAuras(CreatureDataAddon* addon, char const* table, char const* guidEntryStr);
        void LoadQuestRelationsHelper(QuestRelationsMap& map, char const* table);
        void LoadVendors(char const* tableName, bool isTemplates);
        void LoadTrainers(char const* tableName, bool isTemplates);

        void LoadGossipMenu(std::set<uint32>& gossipScriptSet);
        void LoadGossipMenuItems(std::set<uint32>& gossipScriptSet);

        typedef std::map<uint32, PetLevelInfo*> PetLevelInfoMap;
        // PetLevelInfoMap[creature_id][level]
        PetLevelInfoMap petInfo;                            // [creature_id][level]

        PlayerClassInfo playerClassInfo[MAX_CLASSES];

        void BuildPlayerLevelInfo(uint8 race, uint8 _class, uint8 level, PlayerLevelInfo* info) const;
        PlayerInfo playerInfo[MAX_RACES][MAX_CLASSES];

        typedef std::vector<uint32> PlayerXPperLevel;       // [level]
        PlayerXPperLevel mPlayerXPperLevel;

        typedef std::map<uint32, uint32> BaseXPMap;         // [area level][base xp]
        BaseXPMap mBaseXPTable;

        typedef std::map<uint32, int32> FishingBaseSkillMap;// [areaId][base skill level]
        FishingBaseSkillMap mFishingBaseForArea;

        // Standing System
        HonorStandingList HordeHonorStandingList;
        HonorStandingList AllyHonorStandingList;

        typedef std::map<uint32, std::vector<std::string> > HalfNameMap;
        HalfNameMap PetHalfName0;
        HalfNameMap PetHalfName1;

        typedef std::multimap<uint32 /*mapId*/, uint32 /*guid*/> ActiveObjectGuidsOnMap;

        // Array to store creature stats, Max creature level + 1 (for data alignement with in game level)
        CreatureClassLvlStats m_creatureClassLvlStats[DEFAULT_MAX_CREATURE_LEVEL + 1][MAX_CREATURE_CLASS];

        MapObjectGuids mMapObjectGuids;
        ActiveObjectGuidsOnMap m_activeCreatures;
        ActiveObjectGuidsOnMap m_activeGameObjects;
        CreatureSpawnTemplateMap m_creatureSpawnTemplateMap;
        CreatureDataMap mCreatureDataMap;
        CreatureLocaleMap mCreatureLocaleMap;
        std::unordered_map<uint32, std::unordered_map<uint32, std::pair<uint32, uint32>>> m_creatureCooldownMap;
        GameObjectDataMap mGameObjectDataMap;
        GameObjectLocaleMap mGameObjectLocaleMap;
        ItemLocaleMap mItemLocaleMap;
        QuestLocaleMap mQuestLocaleMap;
        NpcTextLocaleMap mNpcTextLocaleMap;
        PageTextLocaleMap mPageTextLocaleMap;
        MangosStringLocaleMap mMangosStringLocaleMap;
        std::map<int32 /*minEntryOfBracket*/, uint32 /*count*/> m_loadedStringCount;
        GossipMenuItemsLocaleMap mGossipMenuItemsLocaleMap;
        PointOfInterestLocaleMap mPointOfInterestLocaleMap;
        AreaTriggerLocaleMap m_areaTriggerLocaleMap;

        DungeonEncounterMap m_DungeonEncounters;
        DungeonEncounterMap m_DungeonEncountersByMap;

        QuestgiverGreetingMap m_questgiverGreetingMap[QUESTGIVER_TYPE_MAX];
        QuestgiverGreetingLocaleMap m_questgiverGreetingLocaleMap[QUESTGIVER_TYPE_MAX];
        TrainerGreetingMap m_trainerGreetingMap;
        TrainerGreetingLocaleMap m_trainerGreetingLocaleMap;

        CacheNpcTextIdMap m_mCacheNpcTextIdMap;
        CacheVendorItemMap m_mCacheVendorTemplateItemMap;
        CacheVendorItemMap m_mCacheVendorItemMap;
        CacheTrainerSpellMap m_mCacheTrainerTemplateSpellMap;
        CacheTrainerSpellMap m_mCacheTrainerSpellMap;

        BroadcastTextMap m_broadcastTextMap;

        CreatureImmunityContainer m_creatureImmunities;

        std::shared_ptr<CreatureSpellListContainer> m_spellListContainer;

        std::shared_ptr<SpawnGroupEntryContainer> m_spawnGroupContainer;

        std::map<int32, WorldStateName> m_worldStateNames;

        std::unique_ptr<UnitConditionMgr> m_unitConditionMgr;
        std::unique_ptr<WorldStateExpressionMgr> m_worldStateExpressionMgr;
        std::unique_ptr<CombatConditionMgr> m_combatConditionMgr;

        std::map<uint32, std::vector<uint32>> m_dynguidCreatureDbGuids;
        std::map<uint32, std::vector<uint32>> m_dynguidGameobjectDbGuids;

        uint32 m_maxGoDbGuid;
        uint32 m_maxCreatureDbGuid;
};

#define sObjectMgr MaNGOS::Singleton<ObjectMgr>::Instance()

/// generic text function
bool DoDisplayText(WorldObject* source, int32 entry, Unit const* target = nullptr, uint32 chatTypeOverride = 0);

// scripting access functions
bool LoadMangosStrings(DatabaseType& db, char const* table, int32 start_value = -1000000, int32 end_value = std::numeric_limits<int32>::min(), bool extra_content = false);
CreatureInfo const* GetCreatureTemplateStore(uint32 entry);
Quest const* GetQuestTemplateStore(uint32 entry);
MangosStringLocale const* GetMangosStringData(int32 entry);

#endif
