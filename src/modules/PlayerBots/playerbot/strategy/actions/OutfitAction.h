#pragma once
#include "playerbot/LootObjectStack.h"
#include "EquipAction.h"

namespace ai
{
    class OutfitAction : public EquipAction 
    {
    public:
        OutfitAction(PlayerbotAI* ai) : EquipAction(ai, "outfit") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        void List(Player* requester);
        void Save(std::string name, ItemIds outfit);
        void Update(std::string name);
    };
}
