
#include "playerbot/playerbot.h"
#include "PossibleRpgTargetsValue.h"

#include "playerbot/ServerFacade.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "NearestUnitsValue.h"
#include "playerbot/TravelMgr.h"

using namespace ai;
using namespace MaNGOS;

std::vector<uint32> PossibleRpgTargetsValue::allowedNpcFlags;

PossibleRpgTargetsValue::PossibleRpgTargetsValue(PlayerbotAI* ai, float range) :
        NearestUnitsValue(ai, "possible rpg targets", range, true)
{
    if (!allowedNpcFlags.size())
    {
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_INNKEEPER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_GOSSIP);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_QUESTGIVER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_FLIGHTMASTER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_BANKER);
#ifndef MANGOSBOT_ZERO
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_GUILD_BANKER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER_CLASS);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER_PROFESSION);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR_AMMO);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR_FOOD);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR_POISON);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR_REAGENT);
#endif
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_AUCTIONEER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_STABLEMASTER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_PETITIONER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TABARDDESIGNER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_BATTLEMASTER);

        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_REPAIR);
    }
}

void PossibleRpgTargetsValue::FindUnits(std::list<Unit*> &targets)
{
    AnyUnitInObjectRangeCheck u_check(bot, range);
    UnitListSearcher<AnyUnitInObjectRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);
}

bool PossibleRpgTargetsValue::AcceptUnit(Unit* unit)
{
    TravelTarget* travelTarget = context->GetValue<TravelTarget*>("travel target")->Get();

    if (travelTarget->GetDestination() && travelTarget->GetDestination()->GetEntry() == unit->GetEntry())
        return true;

    if (sServerFacade.IsHostileTo(unit, bot) || dynamic_cast<Player*>(unit))
        return false;

	if (unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER))
		return false;

    for (std::vector<uint32>::iterator i = allowedNpcFlags.begin(); i != allowedNpcFlags.end(); ++i)
    {
		if (unit->HasFlag(UNIT_NPC_FLAGS, *i)) return true;
    }

    if (urand(1, 100) < 25 && sServerFacade.IsFriendlyTo(unit, bot))
        return true;

    if (urand(1, 100) < 5)
        return true;

    return false;
}
