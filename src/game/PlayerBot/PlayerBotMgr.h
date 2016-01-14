#ifndef _PLAYERBOTMGR_H
#define _PLAYERBOTMGR_H

#include "Common.h"
#include "PlayerBot.h"

typedef std::unordered_map<uint32, PlayerBot> PlayerBotMap;

class PlayerBotMgr
{
    public:
        uint32 CreateBot(uint32 accountId);
        bool DestroyBot(uint32 botId);

        bool LoginBot(uint32 botId, ObjectGuid characterId);
        bool LogoutBot(uint32 botId);

        void LogoutAllBots();

        PlayerBotMap& GetBots();

    private:
        PlayerBotMap m_playerBots;

};

#define sPlayerBotMgr MaNGOS::Singleton<PlayerBotMgr>::Instance()

#endif