
#include "playerbot/playerbot.h"
#include "GoAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/values/Formations.h"
#include "playerbot/strategy/values/PositionValue.h"
#include "playerbot/TravelMgr.h"
#include "MotionGenerators/PathFinder.h"
#include "ChooseTravelTargetAction.h"
#include "playerbot/TravelMgr.h"
#include "TellLosAction.h"

using namespace ai;

constexpr std::string_view LOS_GOS_PARAM = "los gos";

std::vector<std::string> split(const std::string& s, char delim);
char* strstri(const char* haystack, const char* needle);

bool GoAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    std::string param = event.getParam();

    if (param.empty())
    {
       param = getQualifier();
    }

    if (param == "?")
    {
        float x = bot->GetPositionX();
        float y = bot->GetPositionY();
        Map2ZoneCoordinates(x, y, bot->GetZoneId());
        std::ostringstream out;
        out << "I am at " << x << "," << y;
        ai->TellPlayer(requester, out.str());
        return true;
    }

    if (param.find("where") != std::string::npos)
    {
        return TellWhereToGo(param, requester);
    }
    if (param.find("how") != std::string::npos && param.size() > 4)
    {
        std::string destination = param.substr(4);

        DestinationList dests = ChooseTravelTargetAction::FindDestination(bot, destination);
        if (dests.empty())
        {
            ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + destination);
            return false;
        }

        WorldPosition botPos(bot);

        TravelDestination* dest = *std::min_element(dests.begin(), dests.end(), [botPos](TravelDestination* i, TravelDestination* j) {return i->DistanceTo(botPos) < j->DistanceTo(botPos); });

        if (!dest)
        {
            ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + destination);
            return false;
        }
        return TellHowToGo(dest, requester);
    }
    std::map<std::string, int> goTos;
    goTos.emplace(std::pair("zone", 5));
    goTos.emplace(std::pair("quest", 6));
    goTos.emplace(std::pair("npc", 4));
    goTos.emplace(std::pair("mob", 4));
    goTos.emplace(std::pair("boss", 5));
    goTos.emplace(std::pair("to", 3));
    for (const auto& option : goTos)
    {
        if (param.find(option.first) == 0 && param.size() > option.second)
        {             
            std::string destination = param.substr(option.second);
            DestinationList dests;
            TravelDestination* dest = nullptr;
            if (option.first == "to")
            {
                dests = ChooseTravelTargetAction::FindDestination(bot, destination);
            }
            else
            {
                dests = ChooseTravelTargetAction::FindDestination(bot, destination, option.first == "zone", option.first == "npc", option.first == "quest", option.first == "mob", option.first == "boss");
            }

            if (dests.empty())
            {
                ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + destination);
                return false;
            }

            WorldPosition botPos(bot);

            dest = *std::min_element(dests.begin(), dests.end(), [botPos](TravelDestination* i, TravelDestination* j) {return i->DistanceTo(botPos) < j->DistanceTo(botPos); });


            if (!dest)
            {
                ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + destination);
                return false;
            }

            if (LeaderAlreadyTraveling(dest))
                return false;

            if (ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("guard", BotState::BOT_STATE_NON_COMBAT) || (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) && ai->GetMaster() && !ai->IsSelfMaster()))
                return TellHowToGo(dest, requester);

            return TravelTo(dest, requester);

        }
    }
    if (param.find("travel") != std::string::npos && param.size()> 7)
    {
        std::string destination = param.substr(7);

        DestinationList dests;
        TravelDestination* dest = nullptr;

        dests = ChooseTravelTargetAction::FindDestination(bot, destination);

        if (dests.empty())
        {
            ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + destination);
            return false;
        }

        WorldPosition botPos(bot);

        dest = *std::min_element(dests.begin(), dests.end(), [botPos](TravelDestination* i, TravelDestination* j) {return i->DistanceTo(botPos) < j->DistanceTo(botPos); });

        return TravelTo(dest, requester);
    }

    if (MoveToGo(param, requester))
        return true;

    if (MoveToUnit(param, requester))
        return true;

    if (MoveToGps(param, requester))
        return true;


    if (MoveToMapGps(param, requester))
        return true;

    return MoveToPosition(param, requester);

    ai->TellPlayer(requester, "Whisper 'go x,y', 'go [game object]', 'go unit' or 'go position' and I will go there." + ChatHelper::formatValue("help", "action:go", "go help") + " for more information.");
    return false;
}

bool GoAction::TellWhereToGo(std::string& param, Player* requester) const
{
    std::string text;

    if (param.size() > 6)
        text = param.substr(6);

    ai->TellPlayerNoFacing(requester, "I have no place I want to go to.");
    return false;   
}

bool GoAction::LeaderAlreadyTraveling(TravelDestination* dest) const
{
    return AI_VALUE(bool, "travel target traveling");
}

bool GoAction::TellHowToGo(TravelDestination* dest, Player* requester) const
{
    WorldPosition botPos = WorldPosition(bot);
    WorldPosition* point = dest->GetClosestPoint(botPos);

    if (!point)
    {
        ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + dest->GetTitle());
        return false;
    }

    std::vector<WorldPosition> beginPath, endPath;
    TravelNodeRoute route = sTravelNodeMap.getRoute(botPos, *point, beginPath, bot);

    if (route.isEmpty())
    {
        ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + dest->GetTitle());
        return false;
    }

    WorldPosition poi = *point;
    float pointAngle = botPos.getAngleTo(poi);

    if (botPos.distance(poi) > sPlayerbotAIConfig.reactDistance || route.getNodes().size() == 1)
    {
        poi = botPos;
        TravelNode* nearNode = nullptr;
        TravelNode* nextNode = nullptr;

        nextNode = nearNode = route.getNodes().front();

        for (auto node : route.getNodes())
        {
            if (node == nearNode)
                continue;

            TravelNodePath* travelPath = nextNode->getPathTo(node);

            std::vector<WorldPosition> path = travelPath->getPath();

            for (auto& p : path)
            {
                if (p.distance(botPos) > sPlayerbotAIConfig.reactDistance)
                    continue;

                if (p.distance(*point) > poi.distance(*point))
                    continue;

                poi = p;
                nextNode = node;
            }
        }

        if (nearNode)
            ai->TellPlayerNoFacing(requester, "We are now near " + nearNode->getName() + ".");

        ai->TellPlayerNoFacing(requester, "if we want to travel to " + dest->GetTitle());
        if (nextNode->getPosition()->getAreaName(true, true) != botPos.getAreaName(true, true))
            ai->TellPlayerNoFacing(requester, "we should head to " + nextNode->getName() + " in " + nextNode->getPosition()->getAreaName(true, true));
        else
            ai->TellPlayerNoFacing(requester, "we should head to " + nextNode->getName());

        pointAngle = botPos.getAngleTo(poi);
    }
    else
        ai->TellPlayerNoFacing(requester, "We are near " + dest->GetTitle());

    ai->TellPlayer(requester, "it is " + std::to_string(uint32(round(poi.distance(botPos)))) + " yards to the " + ChatHelper::formatAngle(pointAngle));
    sServerFacade.SetFacingTo(bot, pointAngle, true);
    bot->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
    ai->Poi(poi.getX(), poi.getY());

    return true;
}

bool GoAction::TravelTo(TravelDestination* dest, Player* requester) const
{
    TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");
    WorldPosition botPos = WorldPosition(bot);
    if (dest)
    {
        WorldPosition* point = dest->GetClosestPoint(botPos);

        if (!point)
            return false;

        target->SetTarget(dest, point);
        target->SetForced(true);
        target->SetConditions({ "not::manual bool::is travel refresh"});

        std::ostringstream out; out << "Traveling to " << dest->GetTitle();
        ai->TellPlayerNoFacing(requester, out.str());

        if (!ai->HasStrategy("travel", BotState::BOT_STATE_NON_COMBAT))
            ai->ChangeStrategy("+travel once", BotState::BOT_STATE_NON_COMBAT);

        return true;
    }
    else
    {
        sTravelMgr.SetNullTravelTarget(target);
        target->SetForced(false);
        return false;
    }
}

bool GoAction::MoveToGo(std::string& param, Player* requester)
{
   auto loopthroughobjects = [&](const std::list<GameObject*>& gos) -> bool
      {
         for (GameObject* go : gos)
         {
            if (go && sServerFacade.isSpawned(go))
            {
               if (sServerFacade.IsDistanceGreaterThan(sServerFacade.GetDistance2d(bot, go), sPlayerbotAIConfig.reactDistance))
               {
                  ai->TellError(requester, "It is too far away");
                  return false;
               }

               std::ostringstream out; out << "Moving to " << ChatHelper::formatGameobject(go);
               ai->TellPlayerNoFacing(requester, out.str());

               WorldPosition pos;
               go->GetPosition(pos);
               const float angle = GetFollowAngle();
               const float distance = INTERACTION_DISTANCE;
               pos += WorldPosition(0, cos(angle)* distance, sin(angle)* distance, 0.5f);

               UpdateStrategyPosition(pos);

               return MoveTo(pos);
            }
         }

         return false;
      };

   std::list<ObjectGuid> goguids = ChatHelper::parseGameobjects(param);

   if (goguids.size())
   {
      return loopthroughobjects(TellLosAction::GoGuidListToObjList(ai, goguids));
   }

   if (param.find(LOS_GOS_PARAM) == 0)
   {
      std::vector<LosModifierStruct> mods = TellLosAction::ParseLosModifiers(param.substr(LOS_GOS_PARAM.size()));

      return loopthroughobjects(TellLosAction::FilterGameObjects(requester, TellLosAction::GoGuidListToObjList(ai, AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los")), mods));
   }

    return false;
}

bool GoAction::MoveToUnit(std::string& param, Player* requester)
{
    std::list<ObjectGuid> units;
    std::list<ObjectGuid> npcs = AI_VALUE(std::list<ObjectGuid>, "nearest npcs");
    units.insert(units.end(), npcs.begin(), npcs.end());
    std::list<ObjectGuid> players = AI_VALUE(std::list<ObjectGuid>, "nearest friendly players");
    units.insert(units.end(), players.begin(), players.end());
    for (std::list<ObjectGuid>::iterator i = units.begin(); i != units.end(); i++)
    {
        Unit* unit = ai->GetUnit(*i);
        if (unit && strstri(unit->GetName(), param.c_str()))
        {
            std::ostringstream out; out << "Moving to " << unit->GetName();
            ai->TellPlayerNoFacing(requester, out.str());
            return MoveNear(bot->GetMapId(), unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ() + 0.5f, ai->GetRange("follow"));
        }
    }

    return false;
}

bool GoAction::MoveToGps(std::string& param, Player* requester)
{
    if (param.find(";") != std::string::npos)
    {
        std::vector<std::string> coords = split(param, ';');
        float x = atof(coords[0].c_str());
        float y = atof(coords[1].c_str());
        float z;
        if (coords.size() > 2)
            z = atof(coords[2].c_str());
        else
            z = bot->GetPositionZ();

        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
        {
            PathFinder path(bot);

            path.calculate(x, y, z, false);

            Vector3 end = path.getEndPosition();
            Vector3 aend = path.getActualEndPosition();

            PointsArray& points = path.getPath();
            PathType type = path.getPathType();

            std::ostringstream out;

            out << x << ";" << y << ";" << z << " =";

            out << "path is: ";

            out << type;

            out << " of length ";

            out << points.size();

            out << " with offset ";

            out << (end - aend).length();


            for (auto i : points)
            {
                CreateWp(bot, i.x, i.y, i.z, 0.0, 11144);
            }

            ai->TellPlayer(requester, out);
        }

        if (bot->IsWithinLOS(x, y, z, true))
            return MoveNear(bot->GetMapId(), x, y, z, 0);
        else
            return MoveTo(bot->GetMapId(), x, y, z, false, false);
    }
    return false;
}

bool GoAction::MoveToMapGps(std::string& param, Player* requester)
{
    if (param.find(",") != std::string::npos)
    {
        std::vector<std::string> coords = split(param, ',');
        float x = atof(coords[0].c_str());
        float y = atof(coords[1].c_str());

        Zone2MapCoordinates(x, y, bot->GetZoneId());

        Map* map = bot->GetMap();
        float z = bot->GetPositionZ();

        if (!WorldPosition(bot->GetMapId(), x, y, z).isValid())
            return false;

        bot->UpdateAllowedPositionZ(x, y, z);

        if (sServerFacade.IsDistanceGreaterThan(sServerFacade.GetDistance2d(bot, x, y), sPlayerbotAIConfig.reactDistance))
        {
            ai->TellPlayer(requester, BOT_TEXT("error_far"));
            return false;
        }

        const TerrainInfo* terrain = map->GetTerrain();
        if (terrain->IsUnderWater(x, y, z) || terrain->IsInWater(x, y, z))
        {
            ai->TellError(requester, BOT_TEXT("error_water"));
            return false;
        }

#ifdef MANGOSBOT_TWO
        float ground = map->GetHeight(bot->GetPhaseMask(), x, y, z + 0.5f);
#else
        float ground = map->GetHeight(x, y, z + 0.5f);
#endif
        if (ground <= INVALID_HEIGHT)
        {
            ai->TellError(requester, BOT_TEXT("error_cant_go"));
            return false;
        }

        float x1 = x, y1 = y;
        Map2ZoneCoordinates(x1, y1, bot->GetZoneId());
        std::ostringstream out; out << "Moving to " << x1 << "," << y1;
        ai->TellPlayerNoFacing(requester, out.str());
        return MoveNear(bot->GetMapId(), x, y, z + 0.5f, ai->GetRange("follow"));
    }
    return false;
}

bool GoAction::MoveToPosition(std::string& param, Player* requester)
{
    PositionEntry pos = context->GetValue<PositionMap&>("position")->Get()[param];
    if (pos.isSet())
    {
        if (sServerFacade.IsDistanceGreaterThan(sServerFacade.GetDistance2d(bot, pos.x, pos.y), sPlayerbotAIConfig.reactDistance))
        {
            ai->TellError(requester, BOT_TEXT("error_far"));
            return false;
        }

        std::ostringstream out; out << "Moving to position " << param;
        ai->TellPlayerNoFacing(requester, out.str());
        return MoveNear(bot->GetMapId(), pos.x, pos.y, pos.z + 0.5f, ai->GetRange("follow"));
    }
    return false;
}

void GoAction::UpdateStrategyPosition(const WorldPosition& position)
{
   if (ai->HasStrategy("stay", ai->GetState()))
   {
      PositionMap& posMap = AI_VALUE(PositionMap&, "position");
      PositionEntry& stayPosition = posMap["stay"];

      stayPosition.Set(position.getX(), position.getY(), position.getZ(), position.getMapId());
      posMap["stay"] = stayPosition;
      posMap["return"] = stayPosition;
   }
   else if (ai->HasStrategy("guard", ai->GetState()))
   {
      PositionMap& posMap = AI_VALUE(PositionMap&, "position");
      PositionEntry& guardPosition = posMap["guard"];

      guardPosition.Set(position.getX(), position.getY(), position.getZ(), position.getMapId());
      posMap["guard"] = guardPosition;
      posMap["return"] = guardPosition;
   }
}
