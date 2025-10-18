
#include "playerbot/playerbot.h"
#include "TellLosAction.h"

#include <boost/algorithm/string.hpp>

using namespace ai;

constexpr std::string_view GOS_PARAM = "gos";
constexpr std::string_view GAMEOBJECTS_PARAM = "game objects";
constexpr std::string_view HIGHLIGHT_PARAM = "highlight";

constexpr std::string_view FILTER_NAME_PARAM = "filter:name:";


bool TellLosAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string param = event.getParam();

    if (param.empty() || param == "targets")
    {
        ListUnits(requester, "--- Targets ---", AI_VALUE(std::list<ObjectGuid>, "possible targets"));
        ListUnits(requester, "--- Targets (All) ---", AI_VALUE(std::list<ObjectGuid>, "all targets"));
    }

    if (param.empty() || param == "npcs")
    {
        ListUnits(requester, "--- NPCs ---", AI_VALUE(std::list<ObjectGuid>, "nearest npcs"));
    }

    if (param.empty() || param == "corpses")
    {
        ListUnits(requester, "--- Corpses ---", AI_VALUE(std::list<ObjectGuid>, "nearest corpses"));
    }

    if (param.empty() || param.find(GOS_PARAM) == 0 || param.find(GAMEOBJECTS_PARAM) == 0)
    {
        std::vector<LosModifierStruct> mods;

        if (param.find(GOS_PARAM) == 0)
        {
            mods = ParseLosModifiers(param.substr(GOS_PARAM.size()));
        }
        else if (param.find(GAMEOBJECTS_PARAM) == 0)
        {
           mods = ParseLosModifiers(param.substr(GAMEOBJECTS_PARAM.size()));
        }

        TellGameObjects(requester, "--- Game objects ---", FilterGameObjects(requester, GoGuidListToObjList(ai, AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los")), mods), mods);
    }

    if (param.empty() || param == "players")
    {
        ListUnits(requester, "--- Friendly players ---", AI_VALUE(std::list<ObjectGuid>, "nearest friendly players"));
    }

    if (param.find(HIGHLIGHT_PARAM) == 0)
    {
       std::list<ObjectGuid> goguids = ChatHelper::parseGameobjects(param.substr(HIGHLIGHT_PARAM.size()));

       if (goguids.size())
       {
          std::list<GameObject*> objects = GoGuidListToObjList(ai, goguids);

          for (GameObject* go : objects)
          {
             WorldPosition spellPosition(go);
             Creature* wpCreature = ai->GetBot()->SummonCreature(15631, spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(), spellPosition.getO(), TEMPSPAWN_TIMED_DESPAWN, 2000.0f);
             wpCreature->SetObjectScale(0.5f);
          }
       }

       return true;
    }

    return true;
}

void TellLosAction::ListUnits(Player* requester, std::string title, std::list<ObjectGuid> units)
{
    ai->TellPlayer(requester, title);

    for (std::list<ObjectGuid>::iterator i = units.begin(); i != units.end(); i++)
    {
        Unit* unit = ai->GetUnit(*i);
        if (unit)
            ai->TellPlayer(requester, unit->GetName());
    }

}

std::list<GameObject*> TellLosAction::GoGuidListToObjList(PlayerbotAI* ai, const std::list<ObjectGuid>& gos)
{
   std::list<GameObject*> gameobjects;

   for (const ObjectGuid& guid : gos)
   {
      if (GameObject* go = ai->GetGameObject(guid))
      {
         gameobjects.push_back(go);
      }
   }

   return gameobjects;
}

std::list<GameObject*> TellLosAction::FilterGameObjects(Player* requester, const std::list<GameObject*>& gos, const std::vector<LosModifierStruct>& mods)
{
   std::list<GameObject*> gameobjects = gos;

   for (const LosModifierStruct& mod : mods)
   {
      switch (mod.typ)
      {
      case LosModifierType::FilterName:
         {
            auto it = gameobjects.begin();
            while (it != gameobjects.end())
            {
               if ((*it)->GetGOInfo()->name != mod.param)
               {
                  it = gameobjects.erase(it);
                  continue;
               }
               ++it;
            }
         }
         break;
      case LosModifierType::FilterRange:
         {
            auto it = gameobjects.begin();
            while (it != gameobjects.end())
            {
               if ((*it)->GetGOInfo()->name != mod.param)
               {
                  it = gameobjects.erase(it);
                  continue;
               }
               ++it;
            }
         }
      break;
      case LosModifierType::SortRange:
         {
            std::vector<std::pair<float, GameObject*>> distanceObjectPairs;

            for (GameObject* obj : gameobjects)
            {
               float distance = requester->GetDistance(obj);
               distanceObjectPairs.emplace_back(distance, obj);
            }

            std::sort(distanceObjectPairs.begin(), distanceObjectPairs.end(),
               [](const std::pair<float, GameObject*>& a, const std::pair<float, GameObject*>& b)
               {
                  return a.first < b.first;
               });

            gameobjects.clear();
            for (const auto& pair : distanceObjectPairs)
            {
               gameobjects.push_back(pair.second);
            }
         }
         break;
      case LosModifierType::FilterFirst:
         {
            if (gameobjects.size())
            {
               gameobjects.erase(++gameobjects.begin(), gameobjects.end());
            }
         }
         break;
      default:
         break;
      }
   }

   return gameobjects;
}

void TellLosAction::TellGameObjects(Player* requester, std::string title, const std::list<GameObject*>& gos, const std::vector<LosModifierStruct>& mods)
{
   ai->TellPlayer(requester, title);

   bool bShowRange = std::find_if(mods.begin(), mods.end(), [](const LosModifierStruct& el){ return el.typ == LosModifierType::ShowRange; }) != mods.end();
   bool bShowGuid = std::find_if(mods.begin(), mods.end(), [](const LosModifierStruct& el) { return el.typ == LosModifierType::ShowGuid; }) != mods.end();

   for (GameObject* go : gos)
   {  
      std::ostringstream ss;

      ss << chat->formatGameobject(go);

      if (bShowRange)
      {
         float distance = requester->GetDistance(go);

         ss << " " << distance << "m";
      }

      if (bShowGuid)
      {
         ss << " " << std::to_string(go->GetGUIDLow());
      }

      WorldPosition spellPosition(go);
      Creature* wpCreature = ai->GetBot()->SummonCreature(15631, spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(), spellPosition.getO(), TEMPSPAWN_TIMED_DESPAWN, 2000.0f);
      wpCreature->SetObjectScale(0.5f);

      ai->TellPlayer(requester, ss.str());
   }
}


std::vector<LosModifierStruct> TellLosAction::ParseLosModifiers(const std::string& str)
{
   std::vector<LosModifierStruct> mods;

   if (str.empty() || str[0] != ' ')
   {
      return mods;
   }

   std::vector<std::string> params = ChatHelper::splitString(str, "|");

   for (std::string param : params)
   {
      boost::trim(param);

      if (param.find(FILTER_NAME_PARAM) == 0)
      {
         mods.emplace_back(LosModifierStruct{ LosModifierType::FilterName, param.substr(FILTER_NAME_PARAM.size()) });
      }
      else if (param.find("sort:range") == 0)
      {
         mods.emplace_back(LosModifierStruct{ LosModifierType::SortRange, "" });
      }
      else if (param.find("filter:range") == 0)
      {
         mods.emplace_back(LosModifierStruct{ LosModifierType::FilterRange, "" });
      }
      else if (param.find("filter:first") == 0)
      {
         mods.emplace_back(LosModifierStruct{ LosModifierType::FilterFirst, "" });
      }
      else if (param.find("show:range") == 0)
      {
         mods.emplace_back(LosModifierStruct{ LosModifierType::ShowRange, "" });
      }
      else if (param.find("show:guid") == 0)
      {
         mods.emplace_back(LosModifierStruct{ LosModifierType::ShowGuid, "" });
      }
   }

   return mods;
}