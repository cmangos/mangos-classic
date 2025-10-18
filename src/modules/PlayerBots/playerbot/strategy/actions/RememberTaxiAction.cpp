
#include "playerbot/playerbot.h"
#include "RememberTaxiAction.h"
#include "playerbot/strategy/values/LastMovementValue.h"

using namespace ai;

bool RememberTaxiAction::Execute(Event& event)
{


    WorldPacket p(event.getPacket());
    p.rpos(0);

    switch (p.GetOpcode())
    {
    case CMSG_ACTIVATETAXI:
        {
            LastMovement& movement = context->GetValue<LastMovement&>("last taxi")->Get();
            movement.taxiNodes.clear();
            movement.taxiNodes.resize(2);

            p >> movement.taxiMaster >> movement.taxiNodes[0] >> movement.taxiNodes[1];
            return true;
        }
    case CMSG_ACTIVATETAXIEXPRESS:
        {
            ObjectGuid guid;
            uint32 node_count;
#ifndef MANGOSBOT_TWO
            uint32 totalcost;
            p >> guid >> totalcost >> node_count;
#else
            p >> guid >> node_count;
#endif

            LastMovement& movement = context->GetValue<LastMovement&>("last taxi")->Get();
            movement.taxiNodes.clear();
            for (uint32 i = 0; i < node_count; ++i)
            {
                uint32 node;
                p >> node;
                movement.taxiNodes.push_back(node);
            }

            return true;
        }
    }

    return false;
}
