
#include "playerbot/playerbot.h"
#include "Entities/Player.h"
#include "Event.h"


using namespace ai;

ObjectGuid Event::getObject()
{
    if (packet.empty())
        return ObjectGuid();

    WorldPacket p(packet);
    p.rpos(0);
    
    ObjectGuid guid;
    p >> guid;

    return guid;
}