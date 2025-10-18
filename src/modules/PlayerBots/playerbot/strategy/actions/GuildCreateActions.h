#pragma once
#include "ChooseTravelTargetAction.h"
#include "playerbot/strategy/values/BudgetValues.h"
#include "playerbot/ServerFacade.h"

namespace ai
{
    class TravelTarget;

    class BuyPetitionAction : public Action 
    {
    public:
        BuyPetitionAction(PlayerbotAI* ai) : Action(ai, "buy petition") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful();
        static bool canBuyPetition(Player* bot);
    };

    class PetitionOfferAction : public Action 
    {
    public:
        PetitionOfferAction(PlayerbotAI* ai, std::string name = "petition offer") : Action(ai, name) {}
        virtual bool Execute(Event& event);
        virtual bool isUseful() { return sPlayerbotAIConfig.randomBotFormGuild && !bot->GetGuildId(); };
    };

    class PetitionOfferNearbyAction : public PetitionOfferAction 
    {
    public:
        PetitionOfferNearbyAction(PlayerbotAI* ai) : PetitionOfferAction(ai, "petition offer nearby") {}
        virtual bool Execute(Event& event);
        virtual bool isUseful() { return sPlayerbotAIConfig.randomBotFormGuild && !bot->GetGuildId() && AI_VALUE2(uint32, "item count", chat->formatQItem(5863)) && AI_VALUE(uint8, "petition signs") < sWorld.getConfig(CONFIG_UINT32_MIN_PETITION_SIGNS); };
    };

    class PetitionTurnInAction : public ChooseTravelTargetAction 
    {
    public:
        PetitionTurnInAction(PlayerbotAI* ai) : ChooseTravelTargetAction(ai, "turn in petition") {}
        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };

    class BuyTabardAction : public ChooseTravelTargetAction 
    {
    public:
        BuyTabardAction(PlayerbotAI* ai) : ChooseTravelTargetAction(ai, "buy tabard") {}
        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };
}
