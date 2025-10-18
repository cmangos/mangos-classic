
#include "playerbot/playerbot.h"
#include "TellReputationAction.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

bool TellReputationAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    ObjectGuid selection = requester->GetSelectionGuid();
    if (selection.IsEmpty())
        return false;

    Unit* unit = requester->GetMap()->GetUnit(selection);
    if (!unit)
        return false;

    const FactionTemplateEntry *factionTemplate = sServerFacade.GetFactionTemplateEntry(unit);
    uint32 faction = factionTemplate->faction;
    const FactionEntry* entry = sFactionStore.LookupEntry
    #ifdef MANGOSBOT_ONE
            <FactionEntry>
    #endif
            (faction);
    int32 reputation = bot->GetReputationMgr().GetReputation(faction);

    std::ostringstream out;
    out << entry->name[0] << ": ";
    out << "|cff";
    ReputationRank rank = bot->GetReputationMgr().GetRank(entry);
    switch (rank) {
        case REP_HATED:
            out << "cc2222hated";
            break;
        case REP_HOSTILE:
            out << "ff0000hostile";
            break;
        case REP_UNFRIENDLY:
            out << "ee6622unfriendly";
            break;
        case REP_NEUTRAL:
            out << "ffff00neutral";
            break;
        case REP_FRIENDLY:
            out << "00ff00friendly";
            break;
        case REP_HONORED:
            out << "00ff88honored";
            break;
        case REP_REVERED:
            out << "00ffccrevered";
            break;
        case REP_EXALTED:
            out << "00ffffexalted";
            break;
        default:
            out << "808080unknown";
            break;
    }

    out << "|cffffffff";

    int32 base = ReputationMgr::Reputation_Cap + 1;
    for (int i = MAX_REPUTATION_RANK - 1; i >= rank; --i)
        base -= ReputationMgr::PointsInRank[i];

    out << " (" << (reputation - base) << "/" << ReputationMgr::PointsInRank[rank] << ")";
    ai->TellPlayer(requester, out);

    return true;
}
