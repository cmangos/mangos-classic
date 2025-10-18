
#include "playerbot/playerbot.h"
#include "ResetInstancesAction.h"

using namespace ai;

bool ResetInstancesAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    WorldPacket packet(CMSG_RESET_INSTANCES, 0);
    bot->GetSession()->HandleResetInstancesOpcode(packet);

    ai->TellPlayer(requester, "Resetting all instances", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    return true;
}

bool ResetRaidsAction::Execute(Event& event)
{
#ifndef MANGOSBOT_ZERO
    for (uint8 i = 0; i < MAX_DIFFICULTY; ++i)
#endif
    {
#ifndef MANGOSBOT_ZERO
        Player::BoundInstancesMap& binds = bot->GetBoundInstances(Difficulty(i));
#else
        Player::BoundInstancesMap& binds = bot->GetBoundInstances();
#endif
        for (Player::BoundInstancesMap::iterator itr = binds.begin(); itr != binds.end();)
        {
            if (itr->first != bot->GetMapId())
            {
#ifndef MANGOSBOT_ZERO
                bot->UnbindInstance(itr, Difficulty(i));
#else
                bot->UnbindInstance(itr);
#endif
            }
            else
            {
                ++itr;
            }
        }
    }

    return true;
}