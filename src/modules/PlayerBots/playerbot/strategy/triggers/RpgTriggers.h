#pragma once
#include "playerbot/strategy/Trigger.h"

namespace ai
{
    class NoRpgTargetTrigger : public Trigger
    {
    public:
        NoRpgTargetTrigger(PlayerbotAI* ai, std::string name = "no rpg target", int checkInterval = 10) : Trigger(ai, name, checkInterval) {}

        virtual bool IsActive() { return !AI_VALUE(GuidPosition, "rpg target") || AI_VALUE2(float, "distance", "rpg target") > sPlayerbotAIConfig.reactDistance * 2; };
    };

    class HasRpgTargetTrigger : public NoRpgTargetTrigger
    {
    public:
        HasRpgTargetTrigger(PlayerbotAI* ai, std::string name = "has rpg target", int checkInterval = 2) : NoRpgTargetTrigger(ai, name, checkInterval) {}

        virtual bool IsActive() { return !NoRpgTargetTrigger::IsActive() && AI_VALUE(std::string, "next rpg action") != "choose rpg target"; }; //Ingore rpg targets that only have the cancel action available.
    };

    class FarFromRpgTargetTrigger : public NoRpgTargetTrigger
    {
    public:
        FarFromRpgTargetTrigger(PlayerbotAI* ai, std::string name = "far from rpg target", int checkInterval = 1) : NoRpgTargetTrigger(ai, name, checkInterval) {}

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "far from rpg target"; } //Must equal iternal name
        virtual std::string GetHelpDescription() { return "This trigger activates [h:strategy|no rpg target] is not active and is 5y away from it's current [h:value|rpg target].";}
        virtual std::vector<std::string> GetUsedTriggers() { return { "no rpg target" }; }
        virtual std::vector<std::string> GetUsedValues() { return {"distance", "rpg target"}; }
#endif

        virtual bool IsActive() { return !NoRpgTargetTrigger::IsActive() && AI_VALUE2(float, "distance", "rpg target") > maxDistance; };
    protected:
        float maxDistance = INTERACTION_DISTANCE;
    };

    class NearRpgTargetTrigger : public FarFromRpgTargetTrigger
    {
    public:
        NearRpgTargetTrigger(PlayerbotAI* ai, std::string name = "near rpg target", int checkInterval = 1) : FarFromRpgTargetTrigger(ai, name, checkInterval) {}

        virtual bool IsActive() { return !NoRpgTargetTrigger::IsActive() && !FarFromRpgTargetTrigger::IsActive(); };
    };

    //Sub actions:
    class RpgTrigger : public FarFromRpgTargetTrigger
    {
    public:
        RpgTrigger(PlayerbotAI* ai, std::string name = "rpg", int checkInterval = 2) : FarFromRpgTargetTrigger(ai, name, checkInterval) {}

        GuidPosition getGuidP() { return AI_VALUE(GuidPosition, "rpg target"); }

        virtual bool IsActive();
        virtual Event Check() { if (!NoRpgTargetTrigger::IsActive() && (AI_VALUE(std::string, "next rpg action") == "choose rpg target" || !FarFromRpgTargetTrigger::IsActive())) return Trigger::Check(); return Event(); };
    };


    class RpgWanderTrigger : public RpgTrigger
    {
    public:
        RpgWanderTrigger(PlayerbotAI* ai, std::string name = "rpg wander") : RpgTrigger(ai, name) {}
        virtual bool IsActive() { return ai->HasRealPlayerMaster() && (!bot->GetGroup() || !getGuidP().GetPlayer() || !bot->GetGroup()->IsMember(getGuidP())); };
    };

    class RpgTaxiTrigger : public RpgTrigger
    {
    public:
        RpgTaxiTrigger(PlayerbotAI* ai, std::string name = "rpg taxi") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgDiscoverTrigger : public RpgTrigger
    {
    public:
        RpgDiscoverTrigger(PlayerbotAI* ai, std::string name = "rpg discover") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgStartQuestTrigger : public RpgTrigger
    {
    public:
        RpgStartQuestTrigger(PlayerbotAI* ai, std::string name = "rpg start quest") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgEndQuestTrigger : public RpgTrigger
    {
    public:
        RpgEndQuestTrigger(PlayerbotAI* ai, std::string name = "rpg end quest") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgRepeatQuestTrigger : public RpgTrigger
    {
    public:
        RpgRepeatQuestTrigger(PlayerbotAI* ai, std::string name = "rpg repeat quest") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgBuyTrigger : public RpgTrigger
    {
    public:
        RpgBuyTrigger(PlayerbotAI* ai, std::string name = "rpg buy") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgSellTrigger : public RpgTrigger
    {
    public:
        RpgSellTrigger(PlayerbotAI* ai, std::string name = "rpg sell") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgAHSellTrigger : public RpgTrigger
    {
    public:
        RpgAHSellTrigger(PlayerbotAI* ai, std::string name = "rpg ah sell") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgAHBuyTrigger : public RpgTrigger
    {
    public:
        RpgAHBuyTrigger(PlayerbotAI* ai, std::string name = "rpg ah buy") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgGetMailTrigger : public RpgTrigger
    {
    public:
        RpgGetMailTrigger(PlayerbotAI* ai, std::string name = "rpg get mail") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgRepairTrigger : public RpgTrigger
    {
    public:
        RpgRepairTrigger(PlayerbotAI* ai, std::string name = "rpg repair") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgTrainTrigger : public RpgTrigger
    {
    public:
        RpgTrainTrigger(PlayerbotAI* ai, std::string name = "rpg train") : RpgTrigger(ai, name) {}

        static bool IsTrainerOf(CreatureInfo const* cInfo, Player* pPlayer);

        virtual bool IsActive();
    };

    class RpgHealTrigger : public RpgTrigger
    {
    public:
        RpgHealTrigger(PlayerbotAI* ai, std::string name = "rpg heal") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgHomeBindTrigger : public RpgTrigger
    {
    public:
        RpgHomeBindTrigger(PlayerbotAI* ai, std::string name = "rpg home bind") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgQueueBGTrigger : public RpgTrigger
    {
    public:
        RpgQueueBGTrigger(PlayerbotAI* ai, std::string name = "rpg queue bg") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgBuyPetitionTrigger : public RpgTrigger
    {
    public:
        RpgBuyPetitionTrigger(PlayerbotAI* ai, std::string name = "rpg buy petition") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgUseTrigger : public RpgTrigger
    {
    public:
        RpgUseTrigger(PlayerbotAI* ai, std::string name = "rpg use") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgQuestUseTrigger : public RpgUseTrigger
    {
    public:
        RpgQuestUseTrigger(PlayerbotAI* ai, std::string name = "rpg quest use") : RpgUseTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgAIChatTrigger : public RpgTrigger
    {
    public:
        RpgAIChatTrigger(PlayerbotAI* ai, std::string name = "rpg ai chat") : RpgTrigger(ai, name, checkInterval = 1) {}
        virtual bool IsActive();
    };

    class RpgSpellTrigger : public RpgTrigger
    {
    public:
        RpgSpellTrigger(PlayerbotAI* ai, std::string name = "rpg spell") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgCraftTrigger : public RpgTrigger
    {
    public:
        RpgCraftTrigger(PlayerbotAI* ai, std::string name = "rpg craft") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgTradeUsefulTrigger : public RpgTrigger
    {
    public:
        RpgTradeUsefulTrigger(PlayerbotAI* ai, std::string name = "rpg trade useful") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    protected:
        virtual bool isFriend(Player* player); //Move to value later.
    };

    class RpgEnchantTrigger : public RpgTradeUsefulTrigger
    {
    public:
        RpgEnchantTrigger(PlayerbotAI* ai, std::string name = "rpg enchant") : RpgTradeUsefulTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgDuelTrigger : public RpgTrigger
    {
    public:
        RpgDuelTrigger(PlayerbotAI* ai, std::string name = "rpg duel") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgItemTrigger : public RpgTrigger
    {
    public:
        RpgItemTrigger(PlayerbotAI* ai, std::string name = "rpg item") : RpgTrigger(ai, name) { maxDistance = INTERACTION_DISTANCE * 2; }
        virtual bool IsActive();
    };

    class RpgSpellClickTrigger : public RpgTrigger
    {
    public:
        RpgSpellClickTrigger(PlayerbotAI* ai, std::string name = "rpg spell click") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RpgGossipTalkTrigger : public RpgTrigger
    {
    public:
        RpgGossipTalkTrigger(PlayerbotAI* ai, std::string name = "rpg gossip talk") : RpgTrigger(ai, name) {}
        virtual bool IsActive();
    };

    class RandomJumpTrigger : public Trigger
    {
    public:
        RandomJumpTrigger(PlayerbotAI* ai, std::string name = "random jump") : Trigger(ai, name, 2) {}
        bool IsActive() override;
    };
}
