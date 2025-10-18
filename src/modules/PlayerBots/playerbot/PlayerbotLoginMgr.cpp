#include "PlayerbotLoginMgr.h"
#include "Database/DatabaseImpl.h"
#include "PlayerbotMgr.h"
#include "PlayerbotAIConfig.h"
#include "RandomPlayerbotMgr.h"

using namespace ai;

class LoginQueryHolder : public SqlQueryHolder
{
private:
    uint32 m_accountId;
    ObjectGuid m_guid;
public:
    LoginQueryHolder(uint32 accountId, ObjectGuid guid)
        : m_accountId(accountId), m_guid(guid) {
    }
    ObjectGuid GetGuid() const { return m_guid; }
    uint32 GetAccountId() const { return m_accountId; }
    bool Initialize();
};

class PlayerbotLoginQueryHolder : public LoginQueryHolder
{
private:
    uint32 masterAccountId;
    PlayerbotHolder* playerbotHolder;

public:
    PlayerbotLoginQueryHolder(PlayerbotHolder* playerbotHolder, uint32 masterAccount, uint32 accountId, uint32 guid)
        : LoginQueryHolder(accountId, ObjectGuid(HIGHGUID_PLAYER, guid)), masterAccountId(masterAccount), playerbotHolder(playerbotHolder) {
    }

public:
    uint32 GetMasterAccountId() const { return masterAccountId; }
    PlayerbotHolder* GetPlayerbotHolder() { return playerbotHolder; }
};

PlayerLoginInfo::PlayerLoginInfo(const uint32 account, const uint32 guid, const uint8 race, const uint8 cls, const uint32 level, const bool isNew, const WorldPosition& position, const uint32 guildId) : account(account), guid(guid), race(race), cls(cls), level(level), isNew(isNew), position(position), guildId(guildId) {}

PlayerLoginInfo::PlayerLoginInfo(Player* player) : PlayerLoginInfo(player->GetSession()->GetAccountId(), player->GetDbGuid(), player->getRace(), player->getClass(), player->GetLevel(), player->GetTotalPlayedTime() == 0, player, player->GetGuildId()) {};

uint32 PlayerLoginInfo::GetLevel() const
{
    if (!isNew || sPlayerbotAIConfig.disableRandomLevels)
        return level;

    uint32 minRandomLevel, maxRandomLevel;

    minRandomLevel = sPlayerbotAIConfig.randomBotMinLevel;

    maxRandomLevel = std::max(sPlayerbotAIConfig.randomBotMinLevel, std::min(sRandomPlayerbotMgr.GetPlayersLevel() + sPlayerbotAIConfig.syncLevelMaxAbove, sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL)));

    return (minRandomLevel + maxRandomLevel) / 2;
}

bool PlayerLoginInfo::IsNearPlayer(const LoginSpace& space) const
{
    if (space.realPlayerInfos.empty())
        return false;

    if (isNew && sPlayerbotAIConfig.instantRandomize) //We do not know where the bot will be teleported to on randomisation. 
        return true;

    for (auto& player : space.realPlayerInfos)
    {
        WorldPosition p(player.position);
        if (p.mapid == position.mapid && p.sqDistance(position) < sPlayerbotAIConfig.loginBotsNearPlayerRange * sPlayerbotAIConfig.loginBotsNearPlayerRange)
        {
            return true;
        }
    }

    return false;
}

bool PlayerLoginInfo::IsOnPlayerMap(const LoginSpace& space) const
{
    if (space.realPlayerInfos.empty())
        return false;

    for (auto& player : space.realPlayerInfos)
    {
        WorldPosition p(player.position);
        if (p.mapid == position.mapid)
        {
            return true;
        }
    }

    return false;
}

bool PlayerLoginInfo::IsInPlayerGroup(const LoginSpace& space) const
{
    if (space.realPlayerInfos.empty())
        return false;

    if (!groupId)
        return false;

    for (auto& player : space.realPlayerInfos)
    {
        if (player.groupId == groupId)
        {
            return true;
        }
    }

    return false;
}

bool PlayerLoginInfo::IsInPlayerGuild(const LoginSpace& space) const
{
    if (space.realPlayerInfos.empty())
        return false;

    for (auto& player : space.realPlayerInfos)
    {
        if (player.guildId == guildId)
        {
            return true;
        }
    }

    return false;
}

bool PlayerLoginInfo::IsInBG() const
{
    return position.isBg();
}

bool PlayerLoginInfo::IsInArena() const
{
    return position.isArena();
}

bool PlayerLoginInfo::IsInInstance() const
{
    return !position.isOverworld() && !position.isBg() && !position.isArena();
}

bool PlayerLoginInfo::SendHolder()
{
    if (holderState == HolderState::HOLDER_SENT)
        return true;

    if (holderState == HolderState::HOLDER_RECEIVED)
        return false;

    if (loginState == LoginState::BOT_ONLINE)
        return false;

    if (loginState == LoginState::BOT_ON_LOGOUTQUEUE)
        return false;

    if (loginState == LoginState::BOT_OFFLINE)
        return false;

    sRandomPlayerbotMgr.GetValue(guid, "logout");

    holderState = HolderState::HOLDER_SENT;

    if (holder)
        delete holder;

    holder = nullptr; 

    holder = new PlayerbotLoginQueryHolder(&sRandomPlayerbotMgr, 0, account, guid);

    PlayerbotLoginQueryHolder* lqh = (PlayerbotLoginQueryHolder*)holder;

    if (!lqh->Initialize())
    {
        delete holder;                                      // delete all unprocessed queries
        return false;
    }

    CharacterDatabase.DelayQueryHolder(this, &PlayerLoginInfo::HandlePlayerBotLoginCallback, holder);

    return true;
}

void PlayerLoginInfo::HandlePlayerBotLoginCallback(QueryResult* /*dummy*/, SqlQueryHolder* holder)
{
    if (!holder)
    {
        holderState = HolderState::HOLDER_EMPTY;
        return;
    }
  
    holderState = HolderState::HOLDER_RECEIVED;
}

void PlayerLoginInfo::ResetLoginState()
{
    if(loginState == LoginState::BOT_ON_LOGINQUEUE) 
        loginState = LoginState::BOT_OFFLINE;
    
    if(loginState == LoginState::BOT_ON_LOGOUTQUEUE) 
        loginState = LoginState::BOT_ONLINE;
}

void PlayerLoginInfo::FillLoginSpace(LoginSpace& space, FillStep step) const
{
    if (step == FillStep::NOW && (loginState == LoginState::BOT_OFFLINE || loginState == LoginState::BOT_ON_LOGINQUEUE))
        return;

    if (step == FillStep::NEXT_STEP && (loginState == LoginState::BOT_OFFLINE || loginState == LoginState::BOT_ON_LOGOUTQUEUE))
        return;

    space.totalSpace--;
    space.classRaceBucket[cls][race]--;
    space.levelBucket[GetLevel()]--;
};

void PlayerLoginInfo::EmptyLoginSpace(LoginSpace& space, FillStep step) const
{
    if (step == FillStep::NOW && (loginState == LoginState::BOT_OFFLINE || loginState == LoginState::BOT_ON_LOGINQUEUE))
        return;

    if (step == FillStep::NEXT_STEP && (loginState == LoginState::BOT_OFFLINE || loginState == LoginState::BOT_ON_LOGOUTQUEUE))
        return;

    space.totalSpace++;
    space.classRaceBucket[cls][race]++;
    space.levelBucket[GetLevel()]++;
};

void PlayerLoginInfo::SetQueue(bool isWanted, LoginSpace& space)
{
    if (isWanted)
    {
        if (loginState == LoginState::BOT_OFFLINE)
        {
            loginState = LoginState::BOT_ON_LOGINQUEUE;
            FillLoginSpace(space, FillStep::NEXT_STEP);
        }
        else if (loginState == LoginState::BOT_ON_LOGOUTQUEUE)
        {
            loginState = LoginState::BOT_ONLINE;
            FillLoginSpace(space, FillStep::NEXT_STEP);
        }
    }
    else
    {
        if (loginState == LoginState::BOT_ONLINE)
        {
            EmptyLoginSpace(space, FillStep::NEXT_STEP);
            loginState = LoginState::BOT_ON_LOGOUTQUEUE;
        }
        else if (loginState == LoginState::BOT_ON_LOGINQUEUE)
        {
            EmptyLoginSpace(space, FillStep::NEXT_STEP);
            loginState = LoginState::BOT_OFFLINE;
        }
    }
}

bool PlayerLoginInfo::IsQueued() const
{
    if (loginState == LoginState::BOT_ON_LOGINQUEUE)
        return true;

    if (loginState == LoginState::BOT_ON_LOGOUTQUEUE)
        return true;

    return false;
}

LoginCriterionFailType PlayerLoginInfo::MatchNoCriteria(const LoginSpace& space, const LoginCriteria& criteria) const
{
    for (auto& [criterionfail, criterion] : criteria)
        if (criterion(*this, space))
            return criterionfail;

    return LoginCriterionFailType::LOGIN_OK;
}

void PlayerLoginInfo::Update(Player* player)
{
    loginState = LoginState::BOT_ONLINE;
    level = player->GetLevel();
    position = WorldPosition(player);
    isNew = ((level > 1) ? false : (player->GetTotalPlayedTime() == 0));
    groupId = player->GetGroup() ? player->GetGroup()->GetId() : 0;
    guildId = player->GetGuildId();
}

bool PlayerLoginInfo::LoginBot()
{
    if (loginState != LoginState::BOT_ON_LOGINQUEUE)
        return false;

    if (holderState != HolderState::HOLDER_RECEIVED)
        return false;

    if (sObjectMgr.GetPlayer(ObjectGuid(HIGHGUID_PLAYER, guid), false))
    {
        loginState = LoginState::BOT_ONLINE;
        return false;
    }

    sRandomPlayerbotMgr.HandlePlayerBotLoginCallback(nullptr, holder);
    holder = nullptr;
    holderState = HolderState::HOLDER_EMPTY;

    if(sPlayerbotAIConfig.randomBotTimedLogout)
        sRandomPlayerbotMgr.SetValue(guid, "add", 1, "", urand(sPlayerbotAIConfig.minRandomBotInWorldTime, sPlayerbotAIConfig.maxRandomBotInWorldTime));

    Player* player = sObjectMgr.GetPlayer(ObjectGuid(HIGHGUID_PLAYER, guid), false);

    if (!player)
    {
        loginState = LoginState::BOT_OFFLINE;
        return false;
    }

    loginState = LoginState::BOT_ONLINE;

    Update(player);

    return true;
}

bool PlayerLoginInfo::LogoutBot()
{
    if (loginState != LoginState::BOT_ON_LOGOUTQUEUE)
        return false;

    Player* player = sObjectMgr.GetPlayer(ObjectGuid(HIGHGUID_PLAYER, guid), false);

    if (!player)
    {
        loginState = LoginState::BOT_OFFLINE;
        return false;
    }

    Update(player);

    sRandomPlayerbotMgr.SetValue(guid, "add", 0, "", 0);

    sRandomPlayerbotMgr.LogoutPlayerBot(guid);

    if (sObjectMgr.GetPlayer(ObjectGuid(HIGHGUID_PLAYER, guid), false))
        return false;

    loginState = LoginState::BOT_OFFLINE;    

    if (sPlayerbotAIConfig.randomBotTimedOffline)
        sRandomPlayerbotMgr.SetValue(guid, "logout", 1, "", urand(sPlayerbotAIConfig.minRandomBotInWorldTime, sPlayerbotAIConfig.maxRandomBotInWorldTime));

    return true;
}

template <typename T, typename Method, typename... Args>
T GetFuture(Method&& method, std::future<T>& fut, bool restart, Args&&... args) {
    bool isValid = fut.valid();
    bool isReady = false;
    T result = T{};

    if (isValid)
        isReady = fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready;

    if (isReady)
        result = fut.get();

    if(!isValid || (isReady && restart))
        fut = std::async(std::launch::async, std::forward<Method>(method), std::forward<Args>(args)...);

    return result;
}

void PlayerBotLoginMgr::Update(RealPlayers& realPlayers)
{
    UpdateOnlineBots();

    if (botPool.empty())
    {
        botPool = GetFuture(LoadBotsFromDb, futurePool, false);  
        return;
    }

    BotInfos queue = GetFuture(FillLoginLogoutQueue, futureQueue, true, &botPool, realPlayers);

    if (!queue.empty())
    {
        this->LoginLogoutBots(queue);
    }
}

BotPool PlayerBotLoginMgr::LoadBotsFromDb()
{
    BotPool botPool;
    std::set<uint32> accounts;
    std::string prefixString = sPlayerbotAIConfig.randomBotAccountPrefix + "%";
    auto result = LoginDatabase.PQuery("SELECT id FROM account where UPPER(username) like UPPER('%s')", prefixString.c_str());
    if (!result)
    {
        return botPool;
    }

    do
    {
    Field* fields = result->Fetch();
    uint32 accountId = fields[0].GetUInt32();
    accounts.insert(accountId);
    } while (result->NextRow());

    sLog.outDebug("PlayerbotLoginMgr: %d accounts found.", uint32(accounts.size()));

    result = CharacterDatabase.PQuery("SELECT account, guid, race, class, level, online, totaltime, map, position_x, position_y, position_z, orientation, (SELECT guildid FROM guild_member m WHERE m.guid = c.guid) guildId FROM characters c");
         
    if (!result)
    {
        return botPool;
    }

    LoginSpace space;

    do
    {
        Field* fields = result->Fetch();
        uint32 account = fields[0].GetUInt32();

        if (accounts.find(account) == accounts.end())
            continue;

        uint32 guid = fields[1].GetUInt32();
        uint32 race = fields[2].GetUInt8();
        uint32 cls = fields[3].GetUInt8();
        uint32 level = fields[4].GetUInt32();
        bool isOnline = fields[5].GetBool();
        bool isNew = sPlayerbotAIConfig.instantRandomize ? (fields[6].GetUInt32() == 0) : level == 1;
        WorldPosition position(fields[7].GetFloat(), fields[8].GetFloat(), fields[9].GetFloat(), fields[10].GetFloat(), fields[11].GetFloat());
        uint32 guildId = fields[12].GetUInt32();
        botPool.insert(std::make_pair(guid, PlayerLoginInfo(account, guid, race, cls, level, isNew, position, guildId)));

        if (isOnline)
        {
            PlayerLoginInfo* info = &botPool.find(guid)->second;
            Player* player = info->GetPlayer();

            if (player)
            {                
                info->Update(player);
            }
        }
    } while (result->NextRow());

    sLog.outDebug("PlayerbotLoginMgr: %d bots found.", uint32(botPool.size()));

    return botPool;
}

void PlayerBotLoginMgr::SendHolders(const BotInfos& queue)
{  
    CharacterDatabase.AsyncPQuery(&RandomPlayerbotMgr::DatabasePing, sWorld.GetCurrentMSTime(), std::string("CharacterDatabase"), "select 1 from dual");

    for (auto& info : queue)
    {
        if (sRandomPlayerbotMgr.GetDatabaseDelay("CharacterDatabase") > 100)
            break;
        info->SendHolder();
    }
}

void PlayerBotLoginMgr::SendHolders(BotPool* pool)
{
    CharacterDatabase.AsyncPQuery(&RandomPlayerbotMgr::DatabasePing, sWorld.GetCurrentMSTime(), std::string("CharacterDatabase"), "select 1 from dual");

    for (auto& [guid, info] : *pool)
    {
        if (sRandomPlayerbotMgr.GetDatabaseDelay("CharacterDatabase") > 100)
            break;
        info.SendHolder();
    }
}

void PlayerBotLoginMgr::UpdateOnlineBots()
{
    for (auto& info : onlineBots)
    {
        Player* player = info->GetPlayer();
        if(player)
            info->Update(player);
    }
}

#define ADD_CRITERIA(type, condition) criteria.push_back(std::make_pair(LoginCriterionFailType::type, []( const PlayerLoginInfo& info, const LoginSpace& space) {return condition;}))
#define ADD_KEEP_CRITERIA(type, condition) criteria.push_back(std::make_pair(LoginCriterionFailType::type, []( const PlayerLoginInfo& info, const LoginSpace& space) {return !(condition);}))

uint32 PlayerBotLoginMgr::GetLoginCriteriaSize()
{
    return sPlayerbotAIConfig.loginCriteria.size();
}

std::vector<std::string> PlayerBotLoginMgr::GetVariableLoginCriteria(const uint8 attempt)
{
    if (sPlayerbotAIConfig.loginCriteria.empty() || attempt >= sPlayerbotAIConfig.loginCriteria.size())
        return {};

    return sPlayerbotAIConfig.loginCriteria[attempt];
}

LoginCriteria PlayerBotLoginMgr::GetLoginCriteria(const uint8 attempt)
{
    LoginCriteria criteria;

    std::vector<std::string> configCriteria = sPlayerbotAIConfig.defaultLoginCriteria;
    std::vector<std::string> attemptCriteria = GetVariableLoginCriteria(attempt);
    configCriteria.insert(configCriteria.end(), attemptCriteria.begin(), attemptCriteria.end());

    for (auto& criterion : configCriteria)
    {
        if (criterion == "maxbots")
            ADD_CRITERIA(MAX_BOTS, space.totalSpace <= int(0));
        if (criterion == "spareroom" && attempt > 0)
            ADD_CRITERIA(SPARE_ROOM, space.totalSpace <= (int32)sPlayerbotAIConfig.freeRoomForNonSpareBots);
        if (criterion == "online")
            ADD_KEEP_CRITERIA(ONLINE, info.IsOnline());
        if (criterion == "logoff" && sPlayerbotAIConfig.randomBotTimedLogout)
            ADD_CRITERIA(RANDOM_TIMED_LOGOUT, info.IsOnline() && !sRandomPlayerbotMgr.GetValue(info.GetId(), "add"));
        if (criterion == "offline" && sPlayerbotAIConfig.randomBotTimedOffline)
            ADD_CRITERIA(RANDOM_TIMED_OFFLINE, !info.IsOnline() && sRandomPlayerbotMgr.GetValue(info.GetId(), "logout"));
        if (criterion == "bg")
            ADD_KEEP_CRITERIA(BG, info.IsOnline() && info.IsInBG());
        if (criterion == "arena")
            ADD_KEEP_CRITERIA(ARENA, info.IsOnline() && info.IsInArena());
        if (criterion == "instance")
            ADD_KEEP_CRITERIA(INSTANCE, info.IsOnline() && info.IsInInstance());
        if (criterion == "classrace")
            ADD_CRITERIA(CLASSRACE, space.classRaceBucket[info.GetClass()][info.GetRace()] <= 0);
        if (criterion == "level")
            ADD_CRITERIA(LEVEL, space.levelBucket[info.GetLevel()] <= 0);
        if (criterion == "range")
            ADD_KEEP_CRITERIA(RANGE, info.IsNearPlayer(space));
        if (criterion == "map")
            ADD_KEEP_CRITERIA(MAP, info.IsOnPlayerMap(space));
        if (criterion == "guild")
            ADD_KEEP_CRITERIA(GUILD, info.IsInPlayerGuild(space));
        if (criterion == "group")
            ADD_KEEP_CRITERIA(GROUP, info.IsInPlayerGroup(space));
    }

    return criteria;
}

RealPlayerInfos PlayerBotLoginMgr::GetPlayerInfos(const RealPlayers& players)
{
    RealPlayerInfos realPlayers;
    for (auto& [guid, player] : players)
        realPlayers.push_back(player);

    return realPlayers;
}

void PlayerBotLoginMgr::FillLoginSpace(BotPool* pool, LoginSpace& space, FillStep step)
{
    space.currentSpace = GetMaxOnlineBotCount();
    space.totalSpace = GetMaxOnlineBotCount();

    for (uint32 level = 1; level < DEFAULT_MAX_LEVEL + 1; ++level)
    {
        space.levelBucket[level] = GetLevelBucketSize(level);
    }

    for (uint32 race = 1; race < MAX_RACES; ++race)
    {
        for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
        {
            space.classRaceBucket[cls][race] = GetClassRaceBucketSize(cls, race);
        }
    }

    for (auto& [guid, botInfo] : *pool)
    {
        botInfo.FillLoginSpace(space, step);

        if (botInfo.GetLoginState() == LoginState::BOT_ONLINE || botInfo.GetLoginState() == LoginState::BOT_ON_LOGOUTQUEUE)
            space.currentSpace--;
    }   
}

bool PlayerBotLoginMgr::CriteriaStillValid(const LoginCriterionFailType oldFailType, const LoginCriteria& criteria)
{
    switch (oldFailType) //These depends on previous bots accepted/rejected so need to be recalculated.
    {
    case LoginCriterionFailType::UNKNOWN:
    case LoginCriterionFailType::MAX_BOTS:
    case LoginCriterionFailType::SPARE_ROOM:
    case LoginCriterionFailType::CLASSRACE:
    case LoginCriterionFailType::LEVEL:
        return false;
    };

    for (auto& [criterionfail, criterion] : criteria)
        if (criterionfail == oldFailType)
            return true;

    return false;
}

BotInfos PlayerBotLoginMgr::FillLoginLogoutQueue(BotPool* pool, const RealPlayers& realPlayers)
{
    LoginSpace loginSpace;
    loginSpace.realPlayerInfos = GetPlayerInfos(realPlayers);
    FillLoginSpace(pool, loginSpace, FillStep::NEXT_STEP);

    std::unordered_map<uint32, LoginCriterionFailType> loginFails;
    std::set<PlayerLoginInfo*> potentialQueue;

    if(sPlayerBotLoginMgr.debug)
        sLog.outError("PlayerbotLoginMgr: Initial space %d", loginSpace.totalSpace);

    for (uint8 attempt = 0; attempt <= GetLoginCriteriaSize(); attempt++)
    {
        LoginCriteria criteria = GetLoginCriteria(attempt);

        for (auto& [guid, botInfo] : *pool)
        {
            if (CriteriaStillValid(loginFails[guid], criteria))
                continue;

            botInfo.EmptyLoginSpace(loginSpace, FillStep::NOW); //Pretend the bot isn't logged in for a moment.
            loginFails[guid] = botInfo.MatchNoCriteria(loginSpace, criteria);
            botInfo.FillLoginSpace(loginSpace, FillStep::NOW);

            bool isWanted = (loginFails[guid] == LoginCriterionFailType::LOGIN_OK);

            if (!isWanted && attempt)
                continue;

            botInfo.SetQueue(isWanted, loginSpace);

            if (botInfo.IsQueued())
                potentialQueue.insert(&botInfo);

            if (attempt && loginSpace.totalSpace <= 0)
            {
                break;
            }
        }

        if (sPlayerBotLoginMgr.debug)
        {
            std::string variableCriteria;
            for (auto& crit : GetVariableLoginCriteria(attempt))
                variableCriteria += crit + ",";
            if (!variableCriteria.empty())
                variableCriteria.pop_back();
            sLog.outError("PlayerbotLoginMgr: Attempt %d (%s), space left %d", attempt, variableCriteria.c_str(), loginSpace.totalSpace);
        }

        if (loginSpace.totalSpace <= 0)
            break;
    }

    BotInfos queue;
    uint32 logins = 0;

    for (auto& info : potentialQueue)
        if (info->GetLoginState() == LoginState::BOT_ON_LOGINQUEUE)
        {
            queue.push_back(info);
            logins++;

            if (logins >= sPlayerbotAIConfig.randomBotsMaxLoginsPerInterval)
                break;
        }

    loginSpace.currentSpace -= logins;

    uint32 maxLogouts = 0;

    if (loginSpace.currentSpace < (int32)sPlayerbotAIConfig.freeRoomForNonSpareBots)
        maxLogouts = (int32)sPlayerbotAIConfig.freeRoomForNonSpareBots - loginSpace.currentSpace;

    uint32 logouts = 0;

    for (auto& info : potentialQueue)
        if (info->GetLoginState() == LoginState::BOT_ON_LOGOUTQUEUE)
        {
            if (logouts >= maxLogouts)
                break;

            queue.push_back(info);
            logouts++;
        }

    if (sPlayerBotLoginMgr.debug)
        sLog.outError("PlayerbotLoginMgr: Queued to log in: %d, out: %d", logins, logouts);

    if(!sPlayerbotAIConfig.preloadHolders)
        SendHolders(queue);
    else
        SendHolders(pool);

    return queue;
}

void PlayerBotLoginMgr::LoginLogoutBots(const BotInfos& queue)
{
    for (auto& info : queue)
    {        
        if (info->LoginBot())
        {
            onlineBots.push_back(info);
        }
        if (info->LogoutBot())
        {
            onlineBots.erase(std::remove(onlineBots.begin(), onlineBots.end(), info), onlineBots.end());
        }
    }
}

uint32 PlayerBotLoginMgr::GetMaxLevel() 
{
    return std::max(sPlayerbotAIConfig.randomBotMinLevel, std::min(sRandomPlayerbotMgr.GetPlayersLevel() + sPlayerbotAIConfig.syncLevelMaxAbove, sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL)));
}

uint32 PlayerBotLoginMgr::GetMaxOnlineBotCount() 
{
    return sRandomPlayerbotMgr.GetValue(uint32(0), "bot_count");
}

uint32 PlayerBotLoginMgr::GetClassRaceBucketSize(uint8 cls, uint8 race) 
{
    uint32 prob = sPlayerbotAIConfig.classRaceProbability[cls][race];

    if (prob == 0)
        return 0;

    if (sPlayerbotAIConfig.useFixedClassRaceCounts)
        return sPlayerbotAIConfig.classRaceProbability[cls][race];

    return GetMaxOnlineBotCount() * sPlayerbotAIConfig.classRaceProbability[cls][race] / sPlayerbotAIConfig.classRaceProbabilityTotal;
}

uint32 PlayerBotLoginMgr::GetLevelBucketSize(uint32 level) 
{
    uint32 prob = sPlayerbotAIConfig.levelProbability[level];

    if (prob == 0 || level > GetMaxLevel())
        return 0;

    uint32 levelProbabilityTotal = 0;
    for (uint32 level = 1; level < GetMaxLevel(); ++level)
    {
        levelProbabilityTotal += sPlayerbotAIConfig.levelProbability[level];
    }

    return GetMaxOnlineBotCount() * sPlayerbotAIConfig.levelProbability[level] / levelProbabilityTotal;
}

