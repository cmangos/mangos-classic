#pragma once
#include "GenericActions.h"

namespace ai
{
   enum class LosModifierType
   {
      FilterName,   // filter:name:<text>
      SortRange,    // sort:range
      FilterRange,  // filter:range
      FilterFirst,  // filter:first
      ShowRange,    // show:range
      ShowGuid,     // show:guid
   };

   struct LosModifierStruct
   {
      LosModifierType typ;
      std::string param;
   };

    class TellLosAction : public ChatCommandAction
    {
    public:
        TellLosAction(PlayerbotAI* ai) : ChatCommandAction(ai, "los") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

        static std::vector<LosModifierStruct> ParseLosModifiers(const std::string& str);
        static std::list<GameObject*> GoGuidListToObjList(PlayerbotAI* ai, const std::list<ObjectGuid>& gos);
        static std::list<GameObject*> FilterGameObjects(Player* requester, const std::list<GameObject*>& gos, const std::vector<LosModifierStruct>& mods);

    private:
        void ListUnits(Player* requester, std::string title, std::list<ObjectGuid> units);

        void TellGameObjects(Player* requester, std::string title, const std::list<GameObject*>& gos, const std::vector<LosModifierStruct>& mods);
    };
}
