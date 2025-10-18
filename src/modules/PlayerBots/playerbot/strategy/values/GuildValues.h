#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class PetitionSignsValue : public SingleCalculatedValue<uint8>
    {
    public:
        PetitionSignsValue(PlayerbotAI* ai) : SingleCalculatedValue<uint8>(ai, "petition signs") {}

        uint8 Calculate();
    }; 

    class CanHandInPetitionValue : public CalculatedValue<bool>
    {
    public:
        CanHandInPetitionValue(PlayerbotAI* ai) : CalculatedValue<bool>(ai, "can hand in guild petition") {}

        bool Calculate() { return !bot->GetGuildId() && AI_VALUE2(uint32, "item count", chat->formatQItem(5863)) && AI_VALUE(uint8, "petition signs") >= sWorld.getConfig(CONFIG_UINT32_MIN_PETITION_SIGNS); };
    };

    class CanBuyTabard : public CalculatedValue<bool>
    {
    public:
        CanBuyTabard(PlayerbotAI* ai) : CalculatedValue<bool>(ai, "can buy tabard") {}

        bool Calculate();
    };
}
