#include "GuidPositionValues.h"
#include "playerbot/TravelMgr.h"
#include "NearestGameObjects.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

using namespace ai;
using namespace MaNGOS;

std::list<GuidPosition> GameObjectsValue::Calculate()
{
    std::list<GameObject*> targets;

    AnyGameObjectInObjectRangeCheck u_check(bot, sPlayerbotAIConfig.reactDistance);
    GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects((const WorldObject*)bot, searcher, sPlayerbotAIConfig.reactDistance);

    std::list<GuidPosition> result;
    for (auto& target : targets)
        result.push_back(target);

    return result;
}

std::list<GuidPosition> EntryFilterValue::Calculate()
{
   std::list<GuidPosition> result;

   std::vector<std::string> pair = getMultiQualifiers(getQualifier(), ",");

    if (pair.size() == 2)
    {
       std::list<GuidPosition> guidList = AI_VALUE_SAFE(std::list<GuidPosition>, pair[0]);
       std::vector<std::string> entryList = getMultiQualifiers(AI_VALUE_SAFE(std::string, pair[1]), ",");

       for (auto guid : guidList)
       {
          for (auto entry : entryList)
             if (guid.GetEntry() == stoi(entry))
                result.push_back(guid);
       }
    }

    return result;
}

bool is_numeric(const std::string& number)
{
   char* end = 0;
   std::strtod(number.c_str(), &end);

   return end != 0 && *end == 0;
}


bool is_integer(const std::string& number)
{
   return is_numeric(number.c_str()) && std::strchr(number.c_str(), '.') == 0;
}

std::vector<std::string> GuidFilterValue::QualifierToEntryList(const std::string& qualifier)
{
   std::vector<std::string> entryList;
   std::vector<std::string> qualifierList = getMultiQualifiers(qualifier, ",");

   for (std::string& s : qualifierList)
   {
      if (s.empty())
         continue;

      if (s[0] == '{')
      {
         std::vector<std::string> entryList2 = QualifierToEntryList(s);
         entryList.insert(entryList.end(), entryList2.begin(), entryList2.end());
         continue;
      }

      if (is_integer(s))
      {
         entryList.push_back(s);
         continue;
      }

      std::string ai_value = AI_VALUE_SAFE(std::string, s);

      if (!ai_value.empty())
      {
         std::vector<std::string> entryList2 = QualifierToEntryList(s);
         entryList.insert(entryList.end(), entryList2.begin(), entryList2.end());
         continue;
      }

      std::list<ObjectGuid> goguids = ChatHelper::parseGameobjects(s);

      if (goguids.size())
      {
         for (ObjectGuid guid : goguids)
         {
            entryList.push_back(std::to_string(guid.GetCounter()));
         }
         continue;
      }
   }

   return entryList;
}

std::list<GuidPosition> GuidFilterValue::Calculate()
{
   std::list<GuidPosition> result;

   std::vector<std::string> pair = getMultiQualifiers(getQualifier(), ",");

   if (pair.size() == 2)
   {
      std::list<GuidPosition> guidList = AI_VALUE_SAFE(std::list<GuidPosition>, pair[0]);
      std::vector<std::string> entryList = QualifierToEntryList(pair[1]);

      for (auto guid : guidList)
      {
         for (auto entry : entryList)
         {
            if (entry.empty())
               continue;

            if (guid.GetCounter() == stoi(entry))
               result.push_back(guid);
         }
      }
   }

   return result;
}

std::list<GuidPosition> RangeFilterValue::Calculate()
{
    std::list<GuidPosition> result;
    std::vector<std::string> params = getMultiQualifiers(getQualifier(), ",");

    if (params.size() >= 2)
    {
       std::list<GuidPosition> guidList = AI_VALUE(std::list<GuidPosition>, params[0]);
       float range = stof(params[1]);
       Player* from_player = bot; // default

       if (params.size() >= 3)
       {
          if (params[2] == "self") // technically not needed
          {
             from_player = bot;
          }
          else if (params[2] == "master")
          {
             from_player = GetMaster();
          }
          else
          {
             sLog.outError("RangeFilterValue::Calculate: wrong qualifier params: %s", getQualifier().c_str());
          }
       }

       for (auto guid : guidList)
       {
          if (guid.sqDistance(from_player) <= range * range)
             result.push_back(guid);
       }
    }
    else
    {
       sLog.outError("RangeFilterValue::Calculate: wrong qualifier params: %s", getQualifier().c_str());
    }

    return result;
}

std::list<GuidPosition> GosInSightValue::Calculate()
{ 
   std::string querried_object = "gos";
   std::string querried_distance = std::to_string(sPlayerbotAIConfig.sightDistance);
   std::string querried_relativity = "self";

   std::vector<std::string> params = getMultiQualifiers(getQualifier(), ",");

   for (std::string param : params)
   {
      if (param.find("from::") == 0)
      {
         querried_relativity = param.substr(6);
      }
      else if (param.find("range::") == 0)
      {
         querried_distance = param.substr(7);
      }
   }

   std::ostringstream ss;

   ss << querried_object << ","
      << querried_distance << ","
      << querried_relativity;

   return AI_VALUE2(std::list<GuidPosition>, "range filter", ss.str());
}

std::list<GuidPosition> GoUsableFilterValue::Calculate()
{
    std::list<GuidPosition> guidList = AI_VALUE(std::list<GuidPosition>, getQualifier());

    std::list<GuidPosition> result;

    for (auto guid : guidList)
    {
        if (guid.IsGameObject())
        {
            GameObject* go = guid.GetGameObject(bot->GetInstanceId());
            if(go && !go->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE))
                result.push_back(guid);
        }
    }

    return result;
}

std::list<GuidPosition> GoTrappedFilterValue::Calculate()
{
    std::list<GuidPosition> guidList = AI_VALUE(std::list<GuidPosition>, getQualifier());

    std::list<GuidPosition> result;

    for (auto guid : guidList)
    {
        if (guid.IsGameObject())
        {
            if (!guid.GetGameObjectInfo()->GetLinkedGameObjectEntry())
                result.push_back(guid);
            else
            {
                GameObject* go = guid.GetGameObject(bot->GetInstanceId());
                if (go && !go->GetLinkedTrap())
                    result.push_back(guid);
            }
        }
    }

    return result;
}





