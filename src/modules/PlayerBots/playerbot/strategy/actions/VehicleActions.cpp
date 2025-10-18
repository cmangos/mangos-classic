
#include "playerbot/playerbot.h"
#include "VehicleActions.h"
#include "playerbot/strategy/ItemVisitors.h"
#ifdef MANGOSBOT_TWO
#include "Entities/Vehicle.h"
#endif

using namespace ai;

bool EnterVehicleAction::Execute(Event& event)
{
#ifdef MANGOSBOT_TWO
    // do not switch vehicles yet
    TransportInfo* transportInfo = bot->GetTransportInfo();
    if (transportInfo && transportInfo->IsOnVehicle())
        return false;

    std::list<ObjectGuid> npcs = AI_VALUE(std::list<ObjectGuid>, "nearest vehicles");
    for (std::list<ObjectGuid>::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* vehicle = ai->GetUnit(*i);
        if (!vehicle)
            continue;

        if (!vehicle->IsFriend(bot))
        {
            std::ostringstream out; out << "Vehicle is not friendy!";
            bot->Say(out.str(), LANG_UNIVERSAL);
            continue;
        }

        if (!vehicle->GetVehicleInfo()->CanBoard(bot))
        {
            std::ostringstream out; out << "Can't enter Vehicle!";
            bot->Say(out.str(), LANG_UNIVERSAL);
            continue;
        }

        //if (fabs(bot->GetPositionZ() - vehicle->GetPositionZ()) < 20.0f)
        //    continue;

        //if (sServerFacade.GetDistance2d(bot, vehicle) > 100.0f)
        //    continue;

        if (sServerFacade.GetDistance2d(bot, vehicle) > 10.0f)
            return MoveTo(vehicle, INTERACTION_DISTANCE - 1.0f);

        uint8 seat = 0;
        vehicle->GetVehicleInfo()->Board(bot, seat);

        std::ostringstream out; out << "Entering Vehicle!";
        bot->Say(out.str(), LANG_UNIVERSAL);
        continue;

        //bot->CastSpell(vehicle, SPELL_RIDE_VEHICLE_HARDCODED, TRIGGERED_OLD_TRIGGERED);

        TransportInfo* transportCheck = bot->GetTransportInfo();
        if (!transportCheck || !transportCheck->IsOnVehicle())
            return false;
        else
        {
            seat = transportCheck->GetTransportSeat();
            // dismount because bots can enter vehicle on mount
            ai->Unmount();
            return true;
        }

        // check if bot is on vehicle
        if (Unit* passenger = vehicle->GetVehicleInfo()->GetPassenger(seat))
        {
            if (passenger->GetObjectGuid() == bot->GetObjectGuid())
                return true;
        }
    }
#endif

	return false;
}

bool LeaveVehicleAction::Execute(Event& event)
{
#ifdef MANGOSBOT_TWO
    TransportInfo* transportInfo = bot->GetTransportInfo();
    if (!transportInfo || !transportInfo->IsOnVehicle())
        return false;

    Unit* vehicle = (Unit*)transportInfo->GetTransport();
    VehicleInfo* veh = vehicle->GetVehicleInfo();
    VehicleSeatEntry const* seat = veh->GetSeatEntry(transportInfo->GetTransportSeat());
    if (!seat || !seat->HasFlag(SEAT_FLAG_CAN_EXIT))
        return false;

    //WorldPacket p;
    //bot->GetSession()->HandleRequestVehicleExit(p);

    if (veh)
        veh->UnBoard(bot, false);

    bot->GetCamera().ResetView();
    bot->SetCharm(nullptr);

    return true;
#endif

    return false;
}
