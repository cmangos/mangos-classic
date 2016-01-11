#ifndef _PLAYERBOTMGR_H
#define _PLAYERBOTMGR_H

#include "Common.h"
#include "PlayerBot.h"

typedef std::unordered_map<uint8, PlayerBot> PlayerBotMap;

class PlayerBotMgr
{
    public:
        PlayerBotMgr() { };
        ~PlayerBotMgr() { };
        void UpdateAI(const uint32 diff);

        uint32 CreateBot(uint32 accountId);
        bool DestroyBot(uint32 botId);

        bool LoginBot(uint32 botId, ObjectGuid characterId);
        bool LogoutBot(uint32 botId);

        void LogoutAllBots();

        PlayerBotMap* GetBots();

    private:
        PlayerBotMap m_playerBots;

};

#define sPlayerBotMgr MaNGOS::Singleton<PlayerBotMgr>::Instance()

#endif