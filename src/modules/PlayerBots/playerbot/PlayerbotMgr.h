#ifndef _PLAYERBOTMGR_H
#define _PLAYERBOTMGR_H

#include "Common.h"
#include "PlayerbotAIBase.h"
#include "Entities/ObjectGuid.h"
#include "Database/DatabaseEnv.h"
#include "Globals/SharedDefines.h"


class WorldPacket;
class Player;
class Unit;
class Object;
class Item;

typedef std::map<uint32, Player*> PlayerBotMap;
typedef std::map<std::string, std::set<std::string> > PlayerBotErrorMap;

class PlayerbotHolder : public PlayerbotAIBase
{
public:
    PlayerbotHolder();
    virtual ~PlayerbotHolder();

    void AddPlayerBot(uint32 guid, uint32 masterAccountId);
	void HandlePlayerBotLoginCallback(QueryResult * dummy, SqlQueryHolder * holder);

    void LogoutPlayerBot(uint32 guid);
    void DisablePlayerBot(uint32 guid, bool logOutPlayer = true);
    Player* GetPlayerBot (uint32 guid) const;

    virtual void UpdateAIInternal(uint32 elapsed, bool minimal = false);
    void UpdateSessions(uint32 elapsed);

    void ForEachPlayerbot(std::function<void(Player*)> fct) const;

    void LogoutAllBots();
    void JoinChatChannels(Player* bot);
    void OnBotLogin(Player* bot);
    virtual void MovePlayerBot(uint32 guid, PlayerbotHolder* newHolder);

    std::list<std::string> HandlePlayerbotCommand(char const* args, Player* master = NULL);
    std::string ProcessBotCommand(std::string cmd, ObjectGuid guid, ObjectGuid masterguid, bool admin, uint32 masterAccountId, uint32 masterGuildId);
    uint32 GetAccountId(std::string name);
    std::string ListBots(Player* master);
    uint32 GetPlayerbotsAmount() const;

protected:
    virtual void OnBotLoginInternal(Player * const bot) = 0;
    void Cleanup();

private:
    PlayerBotMap playerBots;
};

class PlayerbotMgr : public PlayerbotHolder
{
public:
    PlayerbotMgr(Player* const master);
    virtual ~PlayerbotMgr();

    static bool HandlePlayerbotMgrCommand(ChatHandler* handler, char const* args);
    void HandleMasterIncomingPacket(const WorldPacket& packet);
    void HandleMasterOutgoingPacket(const WorldPacket& packet);
    void HandleCommand(uint32 type, const std::string& text, uint32 lang = LANG_UNIVERSAL);
    void OnPlayerLogin(Player* player);
    void CancelLogout();

    virtual void UpdateAIInternal(uint32 elapsed, bool minimal = false);
    void TellError(std::string botName, std::string text);

    Player* GetMaster() const { return master; };

    void SaveToDB();

protected:
    virtual void OnBotLoginInternal(Player * const bot);
    void CheckTellErrors(uint32 elapsed);

private:
    Player* const master;
    PlayerBotErrorMap errors;
    time_t lastErrorTell;
};

#endif
