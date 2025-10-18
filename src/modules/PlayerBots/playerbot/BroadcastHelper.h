#pragma once

namespace ai
{

    class BroadcastHelper
    {
    public:
        BroadcastHelper();

    public:
        enum ToChannel
        {
            TO_GUILD = 1,
            TO_WORLD = 2,
            TO_GENERAL = 3,
            TO_TRADE = 4,
            TO_LOOKING_FOR_GROUP = 5,
            TO_LOCAL_DEFENSE = 6,
            TO_WORLD_DEFENSE = 7,
            TO_GUILD_RECRUITMENT = 8,
            TO_SAY = 9,
            TO_YELL = 10,
            TO_SAY_CITY = 11,
            TO_YELL_CITY = 12
        };

        static uint8 GetLocale();
        static bool BroadcastTest(
            PlayerbotAI* ai,
            Player* bot
        );
        static bool BroadcastToChannelWithGlobalChance(
            PlayerbotAI* ai,
            std::string message,
            std::list<std::pair<ToChannel, uint32>> toChannels
        );
        static bool BroadcastLootingItem(
            PlayerbotAI* ai,
            Player* bot,
            const ItemPrototype* proto,
            ItemQualifier& itemQualifier
        );
        static bool BroadcastQuestAccepted(
            PlayerbotAI* ai,
            Player* bot,
            const Quest* quest
        );
        static bool BroadcastQuestUpdateAddKill(
            PlayerbotAI* ai,
            Player* bot,
            Quest const* quest,
            uint32 availableCount,
            uint32 requiredCount,
            std::string obectiveName
        );
        static bool BroadcastQuestUpdateAddItem(
            PlayerbotAI* ai,
            Player* bot,
            Quest const* quest,
            uint32 availableCount,
            uint32 requiredCount,
            const ItemPrototype* proto
        );
        static bool BroadcastQuestUpdateFailedTimer(
            PlayerbotAI* ai,
            Player* bot,
            Quest const* quest
        );
        static bool BroadcastQuestUpdateComplete(
            PlayerbotAI* ai,
            Player* bot,
            Quest const* quest
        );
        static bool BroadcastQuestTurnedIn(
            PlayerbotAI* ai,
            Player* bot,
            Quest const* quest
        );
        static bool BroadcastCreatureKill(
            PlayerbotAI* ai,
            Player* bot,
            Creature* creature
        );
        static bool BroadcastPlayerKill(
            PlayerbotAI* ai,
            Player* bot,
            Player* victim
        );
        static bool BroadcastLevelup(
            PlayerbotAI* ai,
            Player* bot
        );
        static bool BroadcastGuildMemberPromotion(
            PlayerbotAI* ai,
            Player* bot,
            Player* player
        );
        static bool BroadcastGuildMemberDemotion(
            PlayerbotAI* ai,
            Player* bot,
            Player* player
        );
        static bool BroadcastGuildGroupOrRaidInvite(
            PlayerbotAI* ai,
            Player* bot,
            Player* player,
            Group* group,
            Guild* guild
        );
        static bool BroadcastSuggestInstance(
            PlayerbotAI* ai,
            std::vector<std::string>& allowedInstances,
            Player* bot
        );
        static bool BroadcastSuggestQuest(
            PlayerbotAI* ai,
            std::vector<uint32>& quests,
            Player* bot
        );
        static bool BroadcastSuggestGrindMaterials(
            PlayerbotAI* ai,
            std::string item,
            Player* bot
        );
        static bool BroadcastSuggestGrindReputation(
            PlayerbotAI* ai,
            std::vector<std::string> levels,
            std::vector<std::string> allowedFactions,
            Player* bot
        );
        static bool BroadcastSuggestSell(
            PlayerbotAI* ai,
            const ItemPrototype* proto,
            uint32 count,
            uint32 price,
            Player* bot
        );
        static bool BroadcastSuggestSomething(
            PlayerbotAI* ai,
            Player* bot
        );
        static bool BroadcastSuggestSomethingToxic(
            PlayerbotAI* ai,
            Player* bot
        );
        static bool BroadcastSuggestToxicLinks(
            PlayerbotAI* ai,
            Player* bot
        );
        static bool BroadcastSuggestThunderfury(
            PlayerbotAI* ai,
            Player* bot
        );
    };


};