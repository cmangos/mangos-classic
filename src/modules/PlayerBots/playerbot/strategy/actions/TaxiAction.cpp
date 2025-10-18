
#include "playerbot/playerbot.h"
#include "TaxiAction.h"
#include "Server/DBCStructure.h"
#include "playerbot/strategy/values/LastMovementValue.h"

using namespace ai;

bool TaxiAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    ai->RemoveShapeshift();

    LastMovement& movement = context->GetValue<LastMovement&>("last taxi")->Get();

    WorldPacket& p = event.getPacket();
    std::string param = event.getParam();
    if ((!p.empty() && (p.GetOpcode() == CMSG_TAXICLEARALLNODES || p.GetOpcode() == CMSG_TAXICLEARNODE)) || param == "clear")
    {
        movement.taxiNodes.clear();
        movement.Set(NULL);
        ai->TellPlayer(requester, "I am ready for the next flight");
        return true;
    }

    std::list<ObjectGuid> units = *context->GetValue<std::list<ObjectGuid> >("nearest npcs");
    for (std::list<ObjectGuid>::iterator i = units.begin(); i != units.end(); i++)
    {
        Creature *npc = bot->GetNPCIfCanInteractWith(*i, UNIT_NPC_FLAG_FLIGHTMASTER);
        if (!npc)
            continue;

        uint32 curloc = sObjectMgr.GetNearestTaxiNode(npc->GetPositionX(), npc->GetPositionY(), npc->GetPositionZ(), npc->GetMapId(), bot->GetTeam());

        std::vector<uint32> nodes;
        for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
        {
            TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i);
            if (entry && entry->from == curloc)
            {
                uint8  field = uint8((i - 1) / 32);
                if (field < TaxiMaskSize) nodes.push_back(i);
            }
        }

        if (param == "?")
        {
            ai->TellPlayerNoFacing(requester, "=== Taxi ===");
            int index = 1;
            for (std::vector<uint32>::iterator i = nodes.begin(); i != nodes.end(); ++i)
            {
                TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(*i);
                if (!entry) continue;

                TaxiNodesEntry const* dest = sTaxiNodesStore.LookupEntry(entry->to);
                if (!dest) continue;

                std::ostringstream out;
                out << index++ << ": " << dest->name[0];
                ai->TellPlayerNoFacing(requester, out.str());
            }
            return true;
        }

        int selected = atoi(param.c_str());
        if (selected)
        {
            uint32 path = nodes[selected - 1];
            TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(path);
            if (!entry) return false;
#ifdef MANGOSBOT_TWO                
            bot->OnTaxiFlightEject(true);
#endif
            bool didTaxi =  bot->ActivateTaxiPathTo({ entry->from, entry->to }, npc, 0);
#ifdef MANGOSBOT_TWO
            bot->ResolvePendingMount();
#endif
            return didTaxi;
        }
#ifdef MANGOSBOT_TWO                
        bot->OnTaxiFlightEject(true);
#endif
        if (!movement.taxiNodes.empty() && !bot->ActivateTaxiPathTo(movement.taxiNodes, npc))
        {
            movement.taxiNodes.clear();
            movement.Set(NULL);
            if (!ai->GetMaster() || sServerFacade.GetDistance2d(bot, ai->GetMaster()) < sPlayerbotAIConfig.reactDistance || ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayerNoFacing(requester, "I can't fly with you");
            return false;
        }

#ifdef MANGOSBOT_TWO
        bot->ResolvePendingMount();
#endif

        return true;
    }

    if(!ai->GetMaster() || sServerFacade.GetDistance2d(bot, ai->GetMaster()) < sPlayerbotAIConfig.reactDistance || ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
        ai->TellPlayerNoFacing(requester, "Cannot find any flightmaster to talk");

    return false;
}
