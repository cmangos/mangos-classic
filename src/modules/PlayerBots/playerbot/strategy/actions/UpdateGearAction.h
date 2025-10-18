#pragma once
#include "playerbot/strategy/Action.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/PlayerbotFactory.h"

namespace ai
{
    class UpdateGearAction : public Action
    {
    public:
        UpdateGearAction(PlayerbotAI* ai);
        virtual bool Execute(Event& event);
        virtual bool isUseful();

    private:
        uint8 GetProgressionLevel(uint32 itemLevel);
        uint8 GetMasterAverageProgressionLevel();
        uint8 GetMasterItemProgressionLevel(uint8 slot, uint8 avgProgressionLevel);
       
        void EnchantItem(Item* item);

    private:
        std::vector<EnchantTemplate> enchants;
    };
}
