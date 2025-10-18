#pragma once

#include "Category.h"
#include "ItemBag.h"
#include "playerbot/PlayerbotAIBase.h"
#include "AuctionHouse/AuctionHouseMgr.h"
#include "Entities/ObjectGuid.h"
#include "Server/WorldSession.h"


#define MAX_AUCTIONS 3
#define AHBOT_WON_EXPIRE 0
#define AHBOT_WON_PLAYER 1
#define AHBOT_WON_SELF 2
#define AHBOT_WON_BID 3
#define AHBOT_WON_DELAY 4
#define AHBOT_SELL_DELAY 5
#define AHBOT_SENDMAIL 6

namespace ahbot
{
    class AhBot
    {
    public:
        AhBot() : nextAICheckTime(0), updating(false) {}
        virtual ~AhBot();
        static AhBot& instance()
        {
            static AhBot instance;
            return instance;
        }

    public:
        static bool HandleAhBotCommand(ChatHandler* handler, char const* args);
        ObjectGuid GetAHBplayerGUID();
        void Init();
        void Update();
        void ForceUpdate();
        void HandleCommand(std::string command);
        void Won(AuctionEntry* entry) { AddToHistory(entry); }
        void Expired(AuctionEntry* entry) {}

        double GetCategoryMultiplier(std::string category)
        {
            return categoryMultipliers[category] ? categoryMultipliers[category] : 1;
        }

        int32 GetSellPrice(const ItemPrototype* proto);
        int32 GetBuyPrice(const ItemPrototype* proto);
        double GetRarityPriceMultiplier(const ItemPrototype* proto);
        bool IsUsedBySkill(const ItemPrototype* proto, uint32 skillId);

    private:
        int Answer(int auction, Category* category, ItemBag* inAuctionItems);
        int AddAuctions(int auction, Category* category, ItemBag* inAuctionItems);
        int AddAuction(int auction, Category* category, const ItemPrototype* proto);
        void Expire(int auction);
        void PrintStats(int auction);
        void AddToHistory(AuctionEntry* entry, uint32 won = 0);
        void CleanupHistory();
        uint32 GetAvailableMoney(uint32 auctionHouse);
        void CheckCategoryMultipliers();
        void updateMarketPrice(uint32 itemId, double price, uint32 auctionHouse);
        bool IsBotAuction(uint32 bidder);
        uint32 GetRandomBidder(uint32 auctionHouse);
        void LoadRandomBots();
        uint32 GetAnswerCount(uint32 itemId, uint32 auctionHouse, uint32 withinTime);
        std::vector<AuctionEntry*> LoadAuctions(const AuctionHouseObject::AuctionEntryMap& auctionEntryMap, Category*& category,
                int& auction);
        void FindMinPrice(const AuctionHouseObject::AuctionEntryMap& auctionEntryMap, AuctionEntry*& entry, Item*& item, uint32* minBid,
                uint32* minBuyout);
        uint32 GetBuyTime(uint32 entry, uint32 itemId, uint32 auctionHouse, Category*& category, double priceLevel);
        uint32 GetTime(std::string category, uint32 id, uint32 auctionHouse, uint32 type);
        void SetTime(std::string category, uint32 id, uint32 auctionHouse, uint32 type, uint32 value);
        uint32 GetSellTime(uint32 itemId, uint32 auctionHouse, Category*& category);
        void CheckSendMail(uint32 bidder, uint32 price, AuctionEntry *entry);
        void Dump();
        void CleanupPropositions();
        void DeleteMail(std::list<uint32> buffer);

    public:
        static uint32 auctionIds[MAX_AUCTIONS];
        static uint32 auctioneers[MAX_AUCTIONS];
        static std::map<uint32, uint32> factions;

    private:
        AvailableItemsBag availableItems;
        time_t nextAICheckTime;
        std::map<std::string, double> categoryMultipliers;
        std::map<std::string, uint32> categoryMaxAuctionCount;
        std::map<std::string, uint32> categoryMaxItemAuctionCount;
        std::map<std::string, uint64> categoryMultiplierExpireTimes;
        std::map<uint32, std::vector<uint32>> bidders;
        std::set<uint32> allBidders;
        bool updating;
    };
};

#define auctionbot MaNGOS::Singleton<ahbot::AhBot>::Instance()

