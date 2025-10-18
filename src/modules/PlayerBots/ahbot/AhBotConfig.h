#pragma once

#include "Config/Config.h"

class AhBotConfig
{
public:
    AhBotConfig();
    static AhBotConfig& instance()
    {
        static AhBotConfig instance;
        return instance;
    }

public:
    bool Initialize();

    bool enabled;
    uint64 guid;
    uint32 updateInterval;
    uint32 historyDays, maxSellInterval;
    uint32 itemBuyMinInterval, itemBuyMaxInterval;
    uint32 itemSellMinInterval, itemSellMaxInterval;
    uint32 alwaysAvailableMoney;
    float priceMultiplier, priceQualityMultiplier;
    uint32 defaultMinPrice, stackReducePrice;
    uint32 maxItemLevel, maxRequiredLevel;
    float underPriceProbability;
    std::set<uint32> ignoreItemIds;
    std::set<uint32> ignoreVendorItemIds;
    bool sendmail;

    float GetSellPriceMultiplier(std::string category)
    {
        return GetCategoryParameter(sellPriceMultipliers, "PriceMultiplier.Sell", category, 1.0f);
    }

    float GetBuyPriceMultiplier(std::string category)
    {
        return GetCategoryParameter(buyPriceMultipliers, "PriceMultiplier.Buy", category, 1.0f);
    }

    float GetItemPriceMultiplier(std::string name)
    {
        return GetCategoryParameter(itemPriceMultipliers, "PriceMultiplier.Item", name, 1.0f);
    }

    int32 GetMaxAllowedAuctionCount(std::string category, int32 default_value)
    {
        return (int32)GetCategoryParameter(maxAuctionCount, "MaxAuctionCount", category, default_value);
    }

    int32 GetMaxAllowedItemAuctionCount(std::string category, int32 default_value)
    {
        return (int32)GetCategoryParameter(maxItemAuctionCount, "MaxItemTypeCount", category, default_value);
    }    

    std::string GetStringDefault(const char* name, const char* def)
    {
        return config.GetStringDefault(name, def);
    }

    bool GetBoolDefault(const char* name, const bool def = false)
    {
        return config.GetBoolDefault(name, def);
    }

    int32 GetIntDefault(const char* name, const int32 def)
    {
        return config.GetIntDefault(name, def);
    }

    float GetFloatDefault(const char* name, const float def)
    {
        return config.GetFloatDefault(name, def);
    }

private:
    float GetCategoryParameter(std::map<std::string, float>& cache, std::string type, std::string category, float defaultValue)
    {
        if (cache.find(category) == cache.end())
        {
            std::ostringstream out; out << "AhBot."<< type << "." << category;
            cache[category] = config.GetFloatDefault(out.str().c_str(), defaultValue);
        }

        return cache[category];
    }

private:
    Config config;
    std::map<std::string, float> sellPriceMultipliers;
    std::map<std::string, float> buyPriceMultipliers;
    std::map<std::string, float> itemPriceMultipliers;
    std::map<std::string, float> maxAuctionCount;
    std::map<std::string, float> maxItemAuctionCount;
};

#define sAhBotConfig MaNGOS::Singleton<AhBotConfig>::Instance()

