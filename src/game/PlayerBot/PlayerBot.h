#ifndef _PLAYERBOT_H
#define _PLAYERBOT_H

#include "Common.h"

class PlayerBot
{
    private:
        WorldSession* m_session;

        void InitializeSession(uint32 accountId);

    public:
        PlayerBot(uint32 accountId);
        ~PlayerBot();

        void UpdateAI(const uint32 diff);

        bool Login(ObjectGuid characterGuid);
        bool Logout();

        bool IsLoggedIn();

        Player* GetPlayer();
};

#endif