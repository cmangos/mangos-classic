#ifndef _RandomPlayerbotMgr_H
#define _RandomPlayerbotMgr_H

#include "Common.h"
#include "PlayerbotAIBase.h"
#include "PlayerbotMgr.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "WorldPosition.h"
#include <map>
#include <list>

class WorldPacket;
class Player;
class Unit;
class Object;
class Item;

class CachedEvent
{
public:
    CachedEvent() : value(0), lastChangeTime(0), validIn(0), data("") {}
    CachedEvent(const CachedEvent& other) : value(other.value), lastChangeTime(other.lastChangeTime), validIn(other.validIn), data(other.data) {}
    CachedEvent(uint32 value, uint32 lastChangeTime, uint32 validIn, std::string data = "") : value(value), lastChangeTime(lastChangeTime), validIn(validIn), data(data) {}

public:
    bool IsEmpty() { return !lastChangeTime; }

public:
    uint32 value, lastChangeTime, validIn;
    std::string data;
};

class PerformanceMonitorOperation;

//https://gist.github.com/bradley219/5373998

class botPIDImpl;
class botPID
{
public:
    // Kp -  proportional gain
    // Ki -  Integral gain
    // Kd -  derivative gain
    // dt -  loop interval time
    // max - maximum value of manipulated variable
    // min - minimum value of manipulated variable
    botPID(double dt, double max, double min, double Kp, double Ki, double Kd);
    void adjust(double Kp, double Ki, double Kd);
    void reset();
   
    double calculate(double setpoint, double pv);
    ~botPID();

private:
    botPIDImpl* pimpl;
};

class RandomPlayerbotMgr : public PlayerbotHolder
{
    public:
        RandomPlayerbotMgr();
        virtual ~RandomPlayerbotMgr();
        static RandomPlayerbotMgr& instance()
        {
            static RandomPlayerbotMgr instance;
            return instance;
        }

        virtual void UpdateAIInternal(uint32 elapsed, bool minimal = false);
private:
        void ScaleBotActivity();
        void LogPlayerLocation();
        void DelayedFacingFix();
        void LoginFreeBots();
public:
        static void DatabasePing(QueryResult* result, uint32 pingStart, std::string db);
        void SetDatabaseDelay(std::string db, uint32 delay) {databaseDelay[db] = delay;}
        uint32 GetDatabaseDelay(std::string db) {if(databaseDelay.find(db) == databaseDelay.end()) return 0; return databaseDelay[db];}

        void LoadNamedLocations();
        bool AddNamedLocation(std::string const& name, WorldLocation const& location);
        bool GetNamedLocation(std::string const& name, WorldLocation& location);

        static bool HandlePlayerbotConsoleCommand(ChatHandler* handler, char const* args);
        bool IsRandomBot(Player* bot);
        bool IsRandomBot(uint32 bot);
        bool IsFreeBot(Player* bot) { return IsRandomBot(bot) || sPlayerbotAIConfig.IsFreeAltBot(bot); }
        bool IsFreeBot(uint32 bot) { return IsRandomBot(bot) || sPlayerbotAIConfig.IsFreeAltBot(bot); }
        void InstaRandomize(Player* bot);
        void Randomize(Player* bot);
        void RandomizeFirst(Player* bot);
        void UpdateGearSpells(Player* bot);
        void ScheduleTeleport(uint32 bot, uint32 time = 0);
        void ScheduleChangeStrategy(uint32 bot, uint32 time = 0);
        void HandleCommand(uint32 type, const std::string& text, Player& fromPlayer, std::string channelName = "", Team team = TEAM_BOTH_ALLOWED, uint32 lang = LANG_UNIVERSAL);
        std::string HandleRemoteCommand(std::string request);
        void OnPlayerLogout(Player* player);
        void OnPlayerLogin(Player* player);
        void OnPlayerLoginError(uint32 bot);
        Player* GetRandomPlayer();
        PlayerBotMap& GetPlayers() { return players; };
        Player* GetPlayer(uint32 playerGuid);
        void PrintStats(uint32 requesterGuid);
        double GetBuyMultiplier(Player* bot);
        double GetSellMultiplier(Player* bot);
        void AddTradeDiscount(Player* bot, Player* master, int32 value);
        void SetTradeDiscount(Player* bot, Player* master, uint32 value);
        uint32 GetTradeDiscount(Player* bot, Player* master);
        void Refresh(Player* bot);
        void RandomTeleportForLevel(Player* bot, bool activeOnly);
        void RandomTeleportForLevel(Player* bot) { return RandomTeleportForLevel(bot, true); }
        void RandomTeleportForRpg(Player* bot, bool activeOnly);
        void RandomTeleportForRpg(Player* bot) { return RandomTeleportForRpg(bot, true); }
        int GetMaxAllowedBotCount();
        bool ProcessBot(Player* player);
        void Revive(Player* player);
        void ChangeStrategy(Player* player);
        uint32 GetValue(Player* bot, std::string type);
        uint32 GetValue(uint32 bot, std::string type);
        int32 GetValueValidTime(uint32 bot, std::string event);
        std::string GetData(uint32 bot, std::string type);
        void SetValue(uint32 bot, std::string type, uint32 value, std::string data = "", int32 validIn = -1);
        void SetValue(Player* bot, std::string type, uint32 value, std::string data = "", int32 validIn = -1);
        void Remove(Player* bot);
        void Hotfix(Player* player, uint32 version);
        uint32 GetBattleMasterEntry(Player* bot, BattleGroundTypeId bgTypeId, bool fake = false);
        const CreatureDataPair* GetCreatureDataByEntry(uint32 entry);
        uint32 GetCreatureGuidByEntry(uint32 entry);
        void LoadBattleMastersCache();
        std::map<uint32, std::map<uint32, std::map<uint32, bool> > > NeedBots;
        std::map<uint32, std::map<uint32, std::map<uint32, uint32> > > BgBots;
        std::map<uint32, std::map<uint32, std::map<uint32, uint32> > > VisualBots;
        std::map<uint32, std::map<uint32, std::map<uint32, uint32> > > BgPlayers;
        std::map<uint32, std::map<uint32, std::map<uint32, std::map<uint32, uint32> > > > ArenaBots;
        std::map<uint32, std::map<uint32, std::map<uint32, uint32> > > Rating;
        std::map<uint32, std::map<uint32, std::map<uint32, uint32> > > Supporters;
        std::map<Team, std::vector<uint32>> LfgDungeons;
        void CheckBgQueue();
        void CheckLfgQueue();
        void CheckPlayers();
        void SaveCurTime();
        void SyncEventTimers();
        void AddOfflineGroupBots();
        static Item* CreateTempItem(uint32 item, uint32 count, Player const* player, uint32 randomPropertyId = 0);
        static InventoryResult CanEquipUnseenItem(Player* player, uint8 slot, uint16& dest, uint32 item);

        bool AddRandomBot(uint32 bot);
        virtual void MovePlayerBot(uint32 guid, PlayerbotHolder* newHolder) override;

        std::map<Team, std::map<BattleGroundTypeId, std::list<uint32> > > getBattleMastersCache() { return BattleMastersCache; }

        float getActivityMod() { return activityMod; }
        float getActivityPercentage() { return activityMod * 100.0f; }
        void setActivityPercentage(float percentage) { activityMod = percentage / 100.0f; }

        void PrintTeleportCache();

        void AddFacingFix(uint32 mapId, uint32 instanceId, ObjectGuid guid) { facingFix[mapId][instanceId].push_back(std::make_pair(guid,time(0))); }

        bool arenaTeamsDeleted, guildsDeleted = false;

        std::mutex m_ahActionMutex;

        const std::vector<AuctionEntry>& GetAhPrices(uint32 itemId) {
            static const std::vector<AuctionEntry> emptyVector; // Avoid returning dangling refs
            auto it = ahMirror.find(itemId);
            return (it != ahMirror.end()) ? it->second : emptyVector;}
        uint32 GetPlayersLevel() { return playersLevel; }
	protected:
	    virtual void OnBotLoginInternal(Player * const bot);
    private:
        //pid values are set in constructor
        botPID pid = botPID(1, 50, -50, 0, 0, 0);
        float activityMod = 0.25;
        std::map<std::string, uint32> databaseDelay;
        uint32 GetEventValue(uint32 bot, std::string event);
        std::string GetEventData(uint32 bot, std::string event);
        uint32 SetEventValue(uint32 bot, std::string event, uint32 value, uint32 validIn, std::string data = "");
        std::list<uint32> GetBots();
        std::list<uint32> GetBgBots(uint32 bracket);
        time_t BgCheckTimer;
        time_t LfgCheckTimer;
        time_t PlayersCheckTimer;
        time_t EventTimeSyncTimer;
        time_t OfflineGroupBotsTimer;
        uint32 AddRandomBots();
        bool ProcessBot(uint32 bot);
        void ScheduleRandomize(uint32 bot, uint32 time);
        void RandomTeleport(Player* bot);
        void RandomTeleport(Player* bot, std::vector<WorldLocation> &locs, bool hearth = false, bool activeOnly = false);
        uint32 GetZoneLevel(uint16 mapId, float teleX, float teleY, float teleZ);
        void PrepareTeleportCache();
        typedef void (RandomPlayerbotMgr::*ConsoleCommandHandler) (Player*);

        void MirrorAh();
    private:
        PlayerBotMap players;
        int processTicks;
        std::unordered_map<std::string, WorldLocation> namedLocations;
        std::map<uint8, std::vector<WorldLocation> > locsPerLevelCache;
        std::map<uint32, std::vector<WorldLocation> > rpgLocsCache;
		std::map<uint32, std::map<uint32, std::vector<WorldLocation> > > rpgLocsCacheLevel;
        std::map<uint32, std::map<uint32, std::vector<std::pair<ObjectGuid, WorldLocation>> > > innCacheLevel;
        std::map<Team, std::map<BattleGroundTypeId, std::list<uint32> > > BattleMastersCache;
        std::map<uint32, std::map<std::string, CachedEvent> > eventCache;
        BarGoLink* loginProgressBar;
        std::list<uint32> currentBots;
        std::list<uint32> arenaTeamMembers;
        uint32 bgBotsCount;
        uint32 playersLevel = 0;
        uint32 activeBots = 0;        

        std::unordered_map<uint32, std::vector<std::pair<int32,int32>>> playerBotMoveLog;
        typedef std::unordered_map <uint32, std::list<float>> botPerformanceMetric;
        std::unordered_map<std::string, botPerformanceMetric> botPerformanceMetrics;
        
        std::vector<std::pair<uint32, uint32>> RpgLocationsNear(const WorldLocation pos, const std::map<uint32, std::map<uint32, std::vector<std::string>>>& areaNames, uint32 radius = 2000);
        void PushMetric(botPerformanceMetric& metric, const uint32 bot, const float value, const uint32 maxNum = 60) const;
        float GetMetricDelta(botPerformanceMetric& metric) const;

        bool showLoginWarning;
        std::unordered_map<uint32, std::unordered_map<uint32, std::vector<std::pair<ObjectGuid, time_t>>>> facingFix;

        //                   itemId,             buyout, count
        std::unordered_map < uint32, std::vector<AuctionEntry>> ahMirror;
};

#define sRandomPlayerbotMgr RandomPlayerbotMgr::instance()

#endif
