#pragma once
#include "playerbot/strategy/Multiplier.h"
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    class RpgActionMultiplier : public Multiplier
    {
    public:
        RpgActionMultiplier(PlayerbotAI* ai) : Multiplier(ai, "rpg action") {}

    public:
        virtual float GetValue(Action* action);
    };

    class RpgStrategy : public Strategy
    {
    public:
        RpgStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        virtual std::string getName() override { return "rpg"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "rpg"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy makes bot move between npcs to automatically do various interaction.\n"
            "This is the main rpg strategy which make bots pickand move to various rpg targets.\n"
            "The interactions included in this strategy are limited to emotesand wait.";            
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return {"rpg quest", "rpg vendor", "rpg explore", "rpg maintenance", "rpg guild", "rpg bg", "rpg player", "rpg craft", "debug rpg"}; }
#endif
    protected:
        virtual void OnStrategyAdded(BotState state) override;
        virtual void OnStrategyRemoved(BotState state) override;
        virtual void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
        virtual void InitNonCombatMultipliers(std::list<Multiplier*>& multipliers) override;
    };

    class RpgQuestStrategy : public Strategy
    {
    public:
        RpgQuestStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        std::string getName() override { return "rpg quest"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "rpg quest"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy makes bot move to npcs that have quests the bot can pick up or hand in.\n"
                "Bots will only pick up quests that give xp unless low on durability.\n"
                "Bots will also try to do repeatable quests.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "rpg" }; }
#endif
    private:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RpgVendorStrategy : public Strategy
    {
    public:
        RpgVendorStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        std::string getName() override { return "rpg vendor"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "rpg vendor"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy makes bot move to vendors, auctionhouses and mail-boxes.\n"
                "Bots will buy and sell items depending on usage see [h:action|query item usage].\n"
                "Bots will pick up their mail if there is any items or gold attached.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "rpg" }; }
#endif
    private:
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RpgExploreStrategy : public Strategy
    {
    public:
        RpgExploreStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        std::string getName() override { return "rpg explore"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "rpg explore"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy makes bot move to innkeepers and flight masters.\n"
                "Bots will make this inn their new home when their current home is far away.\n"
                "Bots pick a random flight (when not with a player) or discover new flightpaths.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "rpg" }; }
#endif
    private:
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RpgMaintenanceStrategy : public Strategy
    {
    public:
        RpgMaintenanceStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        std::string getName() override { return "rpg maintenance"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "rpg maintenance"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy makes bot move to trainers and armorers.\n"
                "Bots will automatically train new spells or skills that are available.\n"
                "Bots will automatically repair their armor when below 80% durability.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "rpg" }; }
#endif
    private:
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RpgGuildStrategy : public Strategy
    {
    public:
        RpgGuildStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        std::string getName() override { return "rpg guild"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "rpg guild"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy makes bot move to guild master npcs.\n"
                "Bots will automatically buy a petition if they are not already in a guild.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "rpg" }; }
#endif
    private:
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RpgBgStrategy : public Strategy
    {
    public:
        RpgBgStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        std::string getName() override { return "rpg bg"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "rpg bg"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy makes bot move to battlemasters.\n"
                "Bots will automatically queue for battlegrounds.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "rpg" }; }
#endif
    private:
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RpgPlayerStrategy : public Strategy
    {
    public:
        RpgPlayerStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        std::string getName() override { return "rpg player"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "rpg player"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy makes bot move to nearby players.\n"
                "Bots will automatically trade other bots items they don't need but the other bot can use.\n"
                "Bots will automatically start duels with other players 3 levels above and 10 levels below them.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "rpg" }; }
#endif
    private:
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RpgCraftStrategy : public Strategy
    {
    public:
        RpgCraftStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        std::string getName() override { return "rpg craft"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "rpg craft"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy makes bot move to nearby objects, players, npcs.\n"
                "Bots will automatically craft items they think are usefull or will give them skill points.\n"
                "Bots will automatically cast random spells on or near players or npcs.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "rpg" }; }
#endif
    private:
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class RpgJumpStrategy : public Strategy
    {
    public:
        RpgJumpStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        std::string getName() override { return "rpg jump"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "rpg jump"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy makes bot jump randomly.\n"
                "Chances of jumps forward, in place, backward depend on config.\n";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "rpg" }; }
#endif
    private:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
    };
}