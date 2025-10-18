#include "playerbot.h"
#include "ChatFilter.h"
#include "strategy/values/RtiTargetValue.h"
#include "strategy/values/ItemUsageValue.h"
#include "ChatHelper.h"
#include "Guilds/GuildMgr.h"

using namespace ai;

std::string ChatFilter::Filter(std::string message, std::string filter)
{
    if (message.find("@") == std::string::npos)
        return message;

    if(filter.empty())
        return message.substr(message.find(" ") + 1);

    return message.substr(message.find(filter) + filter.size() + 1);
}


class StrategyChatFilter : public ChatFilter
{
public:
    StrategyChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() {
        return "strategy";
    }
    virtual std::unordered_map<std::string, std::string> GetFilterExamples()
    {
        std::unordered_map<std::string, std::string> retMap;
        retMap["@nc=rpg"] = "All bots that have rpg strategy enabled in noncombat state.";
        retMap["@nonc=travel"] = "All bots that do not have travel strategy enabled in noncombat state.";
        retMap["@co=melee"] = "All bots that have melee strategy enabled in combat state.";
        retMap["@react=<>"] = "All bots that have <> strategy enabled in reaction state.";
        retMap["@dead=<>"] = "All bots that have <> strategy enabled in dead state.";
        return retMap;
    }
    virtual std::string GetHelpDescription() {
        return "This filter selects bots based on their strategies.";
    }
#endif

    virtual std::string Filter(std::string message) override
    {
        Player* bot = ai->GetBot();

        if (message.find("@nc=") == 0)
        {
            std::string strat = message.substr(message.find("=")+1, message.find(" ") - (message.find("=")+1));

            if (!strat.empty())
            {             
                if (ai->HasStrategy(strat,BotState::BOT_STATE_NON_COMBAT))
                    return ChatFilter::Filter(message);
            }
            return message;            
        }
        if (message.find("@nonc=") == 0)
        {
            std::string strat = message.substr(message.find("=") + 1, message.find(" ") - (message.find("=") + 1));

            if (!strat.empty())
            {
                if (!ai->HasStrategy(strat, BotState::BOT_STATE_NON_COMBAT))
                    return ChatFilter::Filter(message);
            }

            return message;
        }
        if (message.find("@co=") == 0)
        {
            std::string strat = message.substr(message.find("=") + 1, message.find(" ") - (message.find("=") + 1));

            if (!strat.empty())
            {
                if (ai->HasStrategy(strat, BotState::BOT_STATE_COMBAT))
                    return ChatFilter::Filter(message);
            }

            return message;
        }
        if (message.find("@noco=") == 0)
        {
            std::string strat = message.substr(message.find("=") + 1, message.find(" ") - (message.find("=") + 1));

            if (!strat.empty())
            {
                if (!ai->HasStrategy(strat, BotState::BOT_STATE_COMBAT))
                    return ChatFilter::Filter(message);
            }

            return message;
        }
        if (message.find("@react=") == 0)
        {
            std::string strat = message.substr(message.find("=") + 1, message.find(" ") - (message.find("=") + 1));

            if (!strat.empty())
            {
                if (ai->HasStrategy(strat, BotState::BOT_STATE_REACTION))
                    return ChatFilter::Filter(message);
            }

            return message;
        }
        if (message.find("@noreact=") == 0)
        {
            std::string strat = message.substr(message.find("=") + 1, message.find(" ") - (message.find("=") + 1));

            if (!strat.empty())
            {
                if (!ai->HasStrategy(strat, BotState::BOT_STATE_REACTION))
                    return ChatFilter::Filter(message);
            }

            return message;
        }
        if (message.find("@dead=") == 0)
        {
            std::string strat = message.substr(message.find("=") + 1, message.find(" ") - (message.find("=") + 1));

            if (!strat.empty())
            {
                if (ai->HasStrategy(strat, BotState::BOT_STATE_DEAD))
                    return ChatFilter::Filter(message);
            }

            return message;
        }
        if (message.find("@nodead=") == 0)
        {
            std::string strat = message.substr(message.find("=") + 1, message.find(" ") - (message.find("=") + 1));

            if (!strat.empty())
            {
                if (!ai->HasStrategy(strat, BotState::BOT_STATE_DEAD))
                    return ChatFilter::Filter(message);
            }

            return message;
        }

        return message;
    }
};

class RoleChatFilter : public ChatFilter
{
public:
    RoleChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() {
        return "role";
    }
    virtual std::unordered_map<std::string,std::string> GetFilterExamples() 
    {
        std::unordered_map<std::string, std::string> retMap;
        retMap["@tank"] = "All bots that have a tank spec.";
        retMap["@dps"] = "All bots that do not have a tank or healing spec.";
        retMap["@heal"] = "All bots that have a healing spec.";
        retMap["@notank"] = "All bots that do not have a tank spec.";
        retMap["@nodps"] = "All bots that have a tank or healing spec.";
        retMap["@noheal"] = "All bots that do not have a healing spec.";
        retMap["@ranged"] = "All bots that use ranged attacks.";
        retMap["@melee"] = "All bots that use melee attacks.";
        return retMap;
    }
    virtual std::string GetHelpDescription() {
        return "This filter selects bots with a specific role or weapon range.";
    }
#endif

    virtual std::string Filter(std::string message) override
    {
        Player* bot = ai->GetBot();
        const bool inGroup = bot->GetGroup() != nullptr;

        bool tank = message.find("@tank") == 0;
        if (tank && !ai->IsTank(bot, inGroup))
            return "";

        bool dps = message.find("@dps") == 0;
        if (dps && (ai->IsTank(bot, inGroup) || ai->IsHeal(bot, inGroup)))
            return "";

        bool heal = message.find("@heal") == 0;
        if (heal && !ai->IsHeal(bot, inGroup))
            return "";

        bool notank = message.find("@notank") == 0;
        if (notank && ai->IsTank(bot, inGroup))
            return "";

        bool nodps = message.find("@nodps") == 0;
        if (nodps && !ai->IsTank(bot, inGroup) && !ai->IsHeal(bot, inGroup))
            return "";

        bool noheal = message.find("@noheal") == 0;
        if (noheal && ai->IsHeal(bot, inGroup))
            return "";

        bool ranged = message.find("@ranged") == 0;
        if (ranged && !ai->IsRanged(bot, inGroup))
            return "";

        bool melee = message.find("@melee") == 0;
        if (melee && ai->IsRanged(bot, inGroup))
            return "";

        if (tank || dps || heal || ranged || melee || noheal || nodps || notank)
            return ChatFilter::Filter(message);

        return message;
    }
};

class LevelChatFilter : public ChatFilter
{
public:
    LevelChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() {
        return "level";
    }
    virtual std::unordered_map<std::string, std::string> GetFilterExamples()
    {
        std::unordered_map<std::string, std::string> retMap;
        retMap["@60"] = "All bots that are level 60.";
        retMap["@10-20"] = "All bots ranging from level 10 to 20.";
        return retMap;
    }
    virtual std::string GetHelpDescription() {
        return "This filter selects bots based on level.";
    }
#endif

    virtual std::string Filter(std::string message) override
    {
        Player* bot = ai->GetBot();

        if (message[0] != '@')
            return message;

        if (message.find("-") != std::string::npos)
        {
            uint32 fromLevel = atoi(message.substr(message.find("@") + 1, message.find("-")).c_str());
            uint32 toLevel = atoi(message.substr(message.find("-") + 1, message.find(" ")).c_str());

            if (bot->GetLevel() >= fromLevel && bot->GetLevel() <= toLevel)
                return ChatFilter::Filter(message);

            return message;
        }

        uint32 level = atoi(message.substr(message.find("@") + 1, message.find(" ")).c_str());
        if (bot->GetLevel() == level)
            return ChatFilter::Filter(message);

        return message;
    }
};

class GearChatFilter : public ChatFilter
{
public:
    GearChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() {
        return "gear";
    }
    virtual std::unordered_map<std::string, std::string> GetFilterExamples()
    {
        std::unordered_map<std::string, std::string> retMap;
        retMap["@tier1"] = "All bots that have an avarage item level comparable to tier1";
        retMap["@tier2-3"] = "All bots an avarage item level comparable to tier2 or tier3.";
        return retMap;
    }
    virtual std::string GetHelpDescription() {
        return "This filter selects bots based on gear level.";
    }
#endif

    virtual std::string Filter(std::string message) override
    {
        Player* bot = ai->GetBot();

        if (message.find("@tier") != 0)
            return message;

        uint32 fromLevel;
        uint32 toLevel;
        uint32 botTier = 0;
        uint32 gs = ai->GetEquipGearScore(bot, false, false);

        if (message.find("-") != std::string::npos)
        {
            fromLevel = atoi(message.substr(message.find("@tier") + 5, message.find("-")).c_str());
            toLevel = atoi(message.substr(message.find("-") + 1, message.find(" ")).c_str());
        }
        else
        {
            fromLevel = atoi(message.substr(message.find("@tier") + 5, message.find(" ")).c_str());
            toLevel = atoi(message.substr(message.find("@tier") + 5, message.find(" ")).c_str());
        }

        if (gs > 60 && gs < 70)
            botTier = 1;
        else if (gs >= 71 && gs <= 76)
            botTier = 2;
        else if (gs >= 77 && gs <= 99)
            botTier = 3;
        else if (gs >= 120 && gs <= 132)
            botTier = 4;
        else if (gs >= 133 && gs <= 145)
            botTier = 5;
        else if (gs >= 146 && gs <= 154)
            botTier = 6;
        else if (gs >= 200 && gs <= 213)
            botTier = 7;
        else if (gs >= 225 && gs <= 232)
            botTier = 8;
        else if (gs >= 232 && gs <= 245)
            botTier = 9;
        else if (gs >= 251 && gs <= 277)
            botTier = 10;

        if (botTier >= fromLevel && botTier <= toLevel)
            return ChatFilter::Filter(message);

        return message;

    }
};

class CombatTypeChatFilter : public ChatFilter
{
public:
    CombatTypeChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() {
        return "combat";
    }
    virtual std::unordered_map<std::string, std::string> GetFilterExamples()
    {
        std::unordered_map<std::string, std::string> retMap;
        retMap["@ranged"] = "All bots that use ranged attacks.";
        retMap["@melee"] = "All bots that use melee attacks.";
        return retMap;
    }
    virtual std::string GetHelpDescription() {
        return "This filter selects bots with a specific weapon range.";
    }
#endif

    virtual std::string Filter(std::string message) override
    {
        Player* bot = ai->GetBot();

        bool melee = message.find("@melee") == 0;
        bool ranged = message.find("@ranged") == 0;

        if (!melee && !ranged)
            return message;

        switch (bot->getClass())
        {
            case CLASS_WARRIOR:
            case CLASS_PALADIN:
            case CLASS_ROGUE:
            /*case CLASS_DEATH_KNIGHT:
                if (ranged)
                    return "";
                break;*/

            case CLASS_HUNTER:
            case CLASS_PRIEST:
            case CLASS_MAGE:
            case CLASS_WARLOCK:
                if (melee)
                    return "";
                break;

            case CLASS_DRUID:
                if (ranged && ai->IsTank(bot,false))
                    return "";
                if (melee && !ai->IsTank(bot, false))
                    return "";
                break;

            case CLASS_SHAMAN:
                if (melee && ai->IsHeal(bot, false))
                    return "";
                if (ranged && !ai->IsHeal(bot, false))
                    return "";
                break;
        }

        return ChatFilter::Filter(message);
    }
};

class RtiChatFilter : public ChatFilter
{
public:
#ifdef GenerateBotHelp
    virtual std::string GetHelpName() {
        return "rti";
    }
    virtual std::unordered_map<std::string, std::string> GetFilterExamples()
    {
        std::unordered_map<std::string, std::string> retMap;
        retMap["@star"] = "All bots that are marked with or are targeing something marked with star.";
        retMap["@circle"] = "All bots that are marked with or are targeing something marked with star.";
        return retMap;
    }
    virtual std::string GetHelpDescription() {
        return "This filter selects bots that are marked with or are targeting sonething marked with a raid target icon.";
    }
#endif

    RtiChatFilter(PlayerbotAI* ai) : ChatFilter(ai)
    {
        rtis.push_back("@star");
        rtis.push_back("@circle");
        rtis.push_back("@diamond");
        rtis.push_back("@triangle");
        rtis.push_back("@moon");
        rtis.push_back("@square");
        rtis.push_back("@cross");
        rtis.push_back("@skull");
    }

    virtual std::string Filter(std::string message) override
    {
        Player* bot = ai->GetBot();
        Group *group = bot->GetGroup();
        if(!group)
            return message;

        bool found = false;
        bool isRti = false;
        for (std::list<std::string>::iterator i = rtis.begin(); i != rtis.end(); i++)
        {
            std::string rti = *i;

            bool isRti = message.find(rti) == 0;
            if (!isRti)
                continue;

            ObjectGuid rtiTarget = group->GetTargetIcon(RtiTargetValue::GetRtiIndex(rti.substr(1)));
            if (bot->GetObjectGuid() == rtiTarget)
                return ChatFilter::Filter(message);

            Unit* target = *ai->GetAiObjectContext()->GetValue<Unit*>("current target");
            if (!target)
                return "";

            if (target->GetObjectGuid() != rtiTarget)
                return "";

            found |= isRti;
            if (found)
                break;
        }

        if (found)
            return ChatFilter::Filter(message);

        return message;
    }

private:
    std::list<std::string> rtis;
};

class ClassChatFilter : public ChatFilter
{
public:
    ClassChatFilter(PlayerbotAI* ai) : ChatFilter(ai)
    {
#ifdef MANGOSBOT_TWO
        classNames["@deathknight"] = CLASS_DEATH_KNIGHT;
#endif
        classNames["@druid"] = CLASS_DRUID;
        classNames["@hunter"] = CLASS_HUNTER;
        classNames["@mage"] = CLASS_MAGE;
        classNames["@paladin"] = CLASS_PALADIN;
        classNames["@priest"] = CLASS_PRIEST;
        classNames["@rogue"] = CLASS_ROGUE;
        classNames["@shaman"] = CLASS_SHAMAN;
        classNames["@warlock"] = CLASS_WARLOCK;
        classNames["@warrior"] = CLASS_WARRIOR;
    }

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() {
        return "class";
    }
    virtual std::unordered_map<std::string, std::string> GetFilterExamples()
    {
        std::unordered_map<std::string, std::string> retMap;
        retMap["@rogue"] = "All rogue bots.";
        retMap["@warlock"] = "All warlock bots.";
        return retMap;
    }
    virtual std::string GetHelpDescription() {
        return "This filter selects bots have a certain class.";
    }
#endif

    virtual std::string Filter(std::string message) override
    {
        Player* bot = ai->GetBot();

        bool found = false;
        bool isClass = false;
        for (std::map<std::string, uint8>::iterator i = classNames.begin(); i != classNames.end(); i++)
        {
            bool isClass = message.find(i->first) == 0;
            if (isClass && bot->getClass() != i->second)
                return "";

            found |= isClass;
            if (found)
                break;
        }

        if (found)
            return ChatFilter::Filter(message);

        return message;
    }

private:
    std::map<std::string, uint8> classNames;
};

class GroupChatFilter : public ChatFilter
{
public:
    GroupChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() {
        return "group";
    }
    virtual std::unordered_map<std::string, std::string> GetFilterExamples()
    {
        std::unordered_map<std::string, std::string> retMap;
        retMap["@group"] = "All bots in a group.";
        retMap["@group2"] = "All bots in group 2.";
        retMap["@group4-6"] = "All bots in group 4 to 6.";
        retMap["@nogroup"] = "All bots that are not grouped.";
        retMap["@leader"] = "All bots that are leader of their group.";
        retMap["@raid"] = "All bots that are in a raid group.";
        retMap["@noraid"] = "All bots that are not in a raid group.";
        retMap["@rleader"] = "All bots that are leader of a raid group.";
        return retMap;
    }
    virtual std::string GetHelpDescription() {
        return "This filter selects bots based on their group.";
    }
#endif

    virtual std::string Filter(std::string message) override
    {
        Player* bot = ai->GetBot();

        if (message.find("@group") == 0)
        {
            std::string pnum = message.substr(6, message.find(" ")-6);
            int from = atoi(pnum.c_str());
            int to = from;
            if (pnum.find("-") != std::string::npos)
            {
                from = atoi(pnum.substr(pnum.find("@") + 1, pnum.find("-")).c_str());
                to = atoi(pnum.substr(pnum.find("-") + 1, pnum.find(" ")).c_str());
            }

            if (!bot->GetGroup())
                return message;

            int sg = (int)bot->GetSubGroup() + 1;
            if (sg >= from && sg <= to)
                return ChatFilter::Filter(message);
        }
        if (message.find("@nogroup") == 0)
        {
            if (bot->GetGroup())
                return message;

            return ChatFilter::Filter(message);
        }
        if (message.find("@leader") == 0)
        {
            if (!bot->GetGroup())
                return message;

            if(bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
                return ChatFilter::Filter(message);
        }
        if (message.find("@raid") == 0)
        {
            if (!bot->GetGroup() || !bot->GetGroup()->IsRaidGroup())
                return message;

            return ChatFilter::Filter(message);
        }
        if (message.find("@noraid") == 0)
        {
            if (bot->GetGroup() && bot->GetGroup()->IsRaidGroup())
                return message;

            return ChatFilter::Filter(message);
        }
        if (message.find("@rleader") == 0)
        {
            if (!bot->GetGroup())
                return message;

            if (!bot->GetGroup()->IsRaidGroup())
                return message;

            if (bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
                return ChatFilter::Filter(message);
        }

        return message;
    }
};

class GuildChatFilter : public ChatFilter
{
public:
    GuildChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() {
        return "guild";
    }
    virtual std::unordered_map<std::string, std::string> GetFilterExamples()
    {
        std::unordered_map<std::string, std::string> retMap;
        retMap["@guild"] = "All bots in a guild.";
        retMap["@guild=raiders"] = "All bots in guild raiders.";
        retMap["@noguild"] = "All bots not in a guild.";
        retMap["@gleader"] = "All bots that are a guild leader.";
        retMap["@rank=Initiate"] = "All bots that have rank Initiate in their guild.";
        return retMap;
    }
    virtual std::string GetHelpDescription() {
        return "This filter selects bots based on their guild.";
    }
#endif

    virtual std::string Filter(std::string message) override
    {
        Player* bot = ai->GetBot();

        if (message.find("@guild=") == 0)
        {
            if (!bot->GetGuildId())
                return message;

            std::string pguild = message.substr(7, message.find(" ")-7);

            if (!pguild.empty())
            {
                Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
                std::string guildName = guild->GetName();

                if (pguild.find(guildName) != 0)
                    return message;
            }

            return ChatFilter::Filter(message);
        }
        if (message.find("@guild") == 0)
        {
            if (!bot->GetGuildId())
                return message;

            std::string pguild = message.substr(6, message.find(" "));
            
            if (!pguild.empty())
            {
                Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
                std::string guildName = guild->GetName();

                if (pguild.find(guildName) == 0)
                    return message;
            }

            return ChatFilter::Filter(message);
        }
        if (message.find("@noguild") == 0)
        {
            if (bot->GetGuildId())
                return message;

            return ChatFilter::Filter(message);
        }
        if (message.find("@gleader") == 0)
        {
            if (!bot->GetGuildId())
                return message;

            Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
            if (guild->GetLeaderGuid() != bot->GetObjectGuid())
                return message;

            return ChatFilter::Filter(message);
        }
        if (message.find("@rank=") == 0)
        {
            if (!bot->GetGuildId())
                return message;

            std::string rank = message.substr(6, message.find(" ")-6);

            if (!rank.empty())
            {
                Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
                std::string rankName = guild->GetRankName(guild->GetRank(bot->GetObjectGuid()));

                if (rank.find(rankName) != 0)
                    return message;
            }

            return ChatFilter::Filter(message);
        }

        return message;
    }
};

class StateChatFilter : public ChatFilter
{
public:
    StateChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() {
        return "state";
    }
    virtual std::unordered_map<std::string, std::string> GetFilterExamples()
    {
        std::unordered_map<std::string, std::string> retMap;
        retMap["@needrepair"] = "All bots that have durability below 20%.";
        retMap["@bagfull"] = "All bots that have no bagspace.";
        retMap["@bagalmostfull"] = "All bots that have below 20% bagspace.";
        retMap["@outside"] = "All bots that are outside of an instance.";
        retMap["@inside"] = "All bots that are inside an instance.";
        return retMap;
    }
    virtual std::string GetHelpDescription() {
        return "This filter selects bots based on their state.";
    }
#endif

    virtual std::string Filter(std::string message) override
    {
        Player* bot = ai->GetBot();
        AiObjectContext* context = bot->GetPlayerbotAI()->GetAiObjectContext();

        if (message.find("@needrepair") == 0)
        {
            if (AI_VALUE(uint8, "durability") > 20)
                return message;
          
            return ChatFilter::Filter(message);
        }
        if (message.find("@outside") == 0)
        {
            if (!WorldPosition(bot).isOverworld())
                return message;

            return ChatFilter::Filter(message);
        }
        if (message.find("@inside") == 0)
        {
            if (WorldPosition(bot).isOverworld())
                return message;

            return ChatFilter::Filter(message);
        }      
        if (message.find("@bagfull") == 0)
        {
            if (AI_VALUE(uint8,"bag space") <= 99)
                return message;

            return ChatFilter::Filter(message);
        }
        if (message.find("@bagalmostfull") == 0)
        {
            if (AI_VALUE(uint8, "bag space") <= 80)
                return message;

            return ChatFilter::Filter(message);
        }

        return message;
    }
};

class UsageChatFilter : public ChatFilter
{
public:
    UsageChatFilter(PlayerbotAI * ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() {
        return "usage";
    }
    virtual std::unordered_map<std::string, std::string> GetFilterExamples()
    {
        std::unordered_map<std::string, std::string> retMap;
        retMap["@use=[itemlink]"] = "All bots that have some use for this item.";
        retMap["@sell=[itemlink]"] = "All bots that will vendor or AH this item.";
        retMap["@need=[itemlink]"] = "All bots that will roll need on this item.";
        retMap["@greed=[itemlink]"] = "All bots that will roll greed on this item.";        
        return retMap;
    }
    virtual std::string GetHelpDescription() {
        return "This filter selects bots based on the use they have for a specific item.";
    }
#endif

    std::string FilterLink(std::string message)
    {
        if (message.find("@") == std::string::npos)
            return message;

        return message.substr(message.find("|r ") + 3);
    }

    virtual std::string Filter(std::string message) override
    {
        Player* bot = ai->GetBot();

        AiObjectContext* context = ai->GetAiObjectContext();

        if (message.find("@use=") == 0)
        {
            std::string item = message.substr(message.find("=") + 1, message.find("|r ") - (message.find("=") + 3));

            if (item.empty())
                return message;

            std::set<std::string> qualifiers = ChatHelper::parseItemQualifiers(item);

            if(qualifiers.empty())
                return message;

            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(*qualifiers.begin()).GetQualifier());

            if (usage != ItemUsage::ITEM_USAGE_NONE && usage != ItemUsage::ITEM_USAGE_AH && usage != ItemUsage::ITEM_USAGE_BROKEN_AH && usage != ItemUsage::ITEM_USAGE_VENDOR)
            {
                return FilterLink(message);
            }

            return message;
        }
        if (message.find("@sell=") == 0)
        {
            std::string item = message.substr(message.find("=") + 1, message.find("|r ") - (message.find("=") + 3));

            if (item.empty())
                return message;

            std::set<std::string> qualifiers = ChatHelper::parseItemQualifiers(item);

            if (qualifiers.empty())
                return message;

            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(*qualifiers.begin()).GetQualifier());

            if (usage == ItemUsage::ITEM_USAGE_AH || usage == ItemUsage::ITEM_USAGE_BROKEN_AH || usage == ItemUsage::ITEM_USAGE_VENDOR)
            {
                return FilterLink(message);
            }

            return message;
        }
        if (message.find("@need=") == 0)
        {
            std::string item = message.substr(message.find("=") + 1, message.find("|r ") - (message.find("=") + 3));

            if (item.empty())
                return message;

            std::set<std::string> qualifiers = ChatHelper::parseItemQualifiers(item);

            if (qualifiers.empty())
                return message;

            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(*qualifiers.begin()).GetQualifier());

            if (usage == ItemUsage::ITEM_USAGE_EQUIP || usage == ItemUsage::ITEM_USAGE_GUILD_TASK || usage == ItemUsage::ITEM_USAGE_BAD_EQUIP || usage == ItemUsage::ITEM_USAGE_FORCE_NEED)
            {
                return FilterLink(message);
            }

            return message;
        }
        if (message.find("@greed=") == 0)
        {
            std::string item = message.substr(message.find("=") + 1, message.find("|r ") - (message.find("=") + 3));

            if (item.empty())
                return message;

            std::set<std::string> qualifiers = ChatHelper::parseItemQualifiers(item);

            if (qualifiers.empty())
                return message;

            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(*qualifiers.begin()).GetQualifier());

            if (usage == ItemUsage::ITEM_USAGE_SKILL || usage == ItemUsage::ITEM_USAGE_USE || usage == ItemUsage::ITEM_USAGE_DISENCHANT || usage == ItemUsage::ITEM_USAGE_BROKEN_AH || usage == ItemUsage::ITEM_USAGE_AH || usage == ItemUsage::ITEM_USAGE_VENDOR || usage == ItemUsage::ITEM_USAGE_FORCE_GREED)
            {
                return FilterLink(message);
            }

            return message;
        }
       
        return message;
    }
};

class TalentSpecChatFilter : public ChatFilter
{
public:
    TalentSpecChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() {
        return "talent spec";
    }
    virtual std::unordered_map<std::string, std::string> GetFilterExamples()
    {
        std::unordered_map<std::string, std::string> retMap;
        retMap["@frost"] = "All bots that have frost spec.";
        retMap["@holy"] = "All bots that have holy spec.";
        return retMap;
    }
    virtual std::string GetHelpDescription() {
        return "This filter selects bots based on their primary talent specialisation.";
    }
#endif

    virtual std::string Filter(std::string message) override
    {
        Player* bot = ai->GetBot();

        AiObjectContext* context = ai->GetAiObjectContext();
        std::string filter = "@" + ChatHelper::specName(bot);

        if (message.find(filter) == 0)
        {
            return ChatFilter::Filter(message);
        }
        
        return message;
    }
};

class LocationChatFilter : public ChatFilter
{
public:
    LocationChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() {
        return "location";
    }
    virtual std::unordered_map<std::string, std::string> GetFilterExamples()
    {
        std::unordered_map<std::string, std::string> retMap;
        retMap["@azeroth"] = "All bots in azeroth overworld.";
        retMap["@eastern kingdoms"] = "All bots in eastern kingdoms overworld.";
        retMap["@dun morogh"] = "All bots in the dun morogh zone.";
        return retMap;
    }
    virtual std::string GetHelpDescription() {
        return "This filter selects bots based on map or zone name.";
    }
#endif

    virtual std::string Filter(std::string message) override
    {
        if (message.find("@") == 0)
        {

            Player* bot = ai->GetBot();

            AiObjectContext* context = ai->GetAiObjectContext();

            Map* map = bot->GetMap();

            if (map)
            {
                std::string name = map->GetMapName();
                std::string filter = name;

               std::transform(filter.begin(), filter.end(), filter.begin(),[](unsigned char c) { return std::tolower(c); });

                filter = "@" + filter;

                if (message.find(filter) == 0)
                {
                    return ChatFilter::Filter(message, filter);
                }
            }

            if (bot->GetTerrain())
            {
                std::string name = WorldPosition(bot).getAreaName(true, true);
                std::string filter = name;

                std::transform(filter.begin(), filter.end(), filter.begin(),[](unsigned char c) { return std::tolower(c); });

                filter = "@" + filter;

                if (message.find(filter) == 0)
                {
                    return ChatFilter::Filter(message, filter);
                }
            }
        }
        return message;
    }
};

class RandomChatFilter : public ChatFilter
{
public:
    RandomChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() {
        return "random";
    }
    virtual std::unordered_map<std::string, std::string> GetFilterExamples()
    {
        std::unordered_map<std::string, std::string> retMap;
        retMap["@random"] = "50% chance the bot responds.";
        retMap["@random=25"] = "25% chance the bot responds.";
        retMap["@fixedrandom"] = "50% chance the bot responds. But always the same bots.";
        retMap["@fixedrandom=25"] = "25% chance the bot responds. But always the same bots.";

        return retMap;
    }
    virtual std::string GetHelpDescription() {
        return "This filter selects random bots.";
    }
#endif

    virtual std::string Filter(std::string message) override
    {
        if (message.find("@random=") == 0)
        {
            std::string num = message.substr(message.find("=") + 1, message.find(" ") - message.find("=")-1);            
            if (urand(0, 100) < stoul(num))
                return ChatFilter::Filter(message);

            return message;
        }
        if (message.find("@random") == 0)
        {
            if (urand(0, 100) < 50)
                return ChatFilter::Filter(message);
        }
        if (message.find("@fixedrandom=") == 0)
        {
            std::string num = message.substr(message.find("=") + 1, message.find(" ") - message.find("=") - 1);
            if (ai->GetFixedBotNumber(BotTypeNumber::CHATFILTER_NUMBER) < stoul(num))
                return ChatFilter::Filter(message);

            return message;
        }
        if (message.find("@fixedrandom") == 0)
        {
            if (ai->GetFixedBotNumber(BotTypeNumber::CHATFILTER_NUMBER) < 50)
                return ChatFilter::Filter(message);
        }

        return message;
    }
};

class QuestChatFilter : public ChatFilter
{
public:
    QuestChatFilter(PlayerbotAI* ai) : ChatFilter(ai) {}

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() {
        return "quest";
    }
    virtual std::unordered_map<std::string, std::string> GetFilterExamples()
    {
        std::unordered_map<std::string, std::string> retMap;
        retMap["@quest=[quest link]"] = "All bots that have the quest and have yet finished it will respond.";
        retMap["@quest=523"] = "All bots that have quest with id 523 will respond.";

        return retMap;
    }
    virtual std::string GetHelpDescription() {
        return "This filter selects bots with a specific quest.";
    }
#endif

    inline std::string toLower(const std::string& str) {
        std::string lowerStr = str;
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
        return lowerStr;
    }

    inline void replaceCaseInsensitive(std::string& str, const std::string& from, const std::string& to) {
        std::string lowerStr = toLower(str);
        std::string lowerFrom = toLower(from);

        size_t pos = lowerStr.find(lowerFrom);
        while (pos != std::string::npos) {
            str.replace(pos, from.length(), to);
            lowerStr.replace(pos, from.length(), toLower(to)); // Keep track of lowercase version for further searching
            pos = lowerStr.find(lowerFrom, pos + to.length()); // Find next occurrence
        }
    }

    virtual std::string Filter(std::string message) override
    {
        if (message.find("@quest=") == 0)
        {
            std::string questString = message.substr(message.find("=") + 1);

            std::set<uint32> questIds = ChatHelper::ExtractAllQuestIds(questString);

            if (questIds.empty() && Qualified::isValidNumberString(questString))
                questIds.insert(stoi(questString));
            else
            {
                for (auto& questId : questIds)
                {
                    Quest const* questTemplate = sObjectMgr.GetQuestTemplate(questId);
                    if (questTemplate)
                        replaceCaseInsensitive(message, " " + ChatHelper::formatQuest(questTemplate), "");
                    replaceCaseInsensitive(message, ChatHelper::formatQuest(questTemplate), "");
                }
            }

            Player* bot = ai->GetBot();
            auto botQuestIds = bot->GetPlayerbotAI()->GetAllCurrentQuestIds();

            std::set<uint32> matchingQuestIds;
            for (auto botQuestId : botQuestIds)
            {
                if (questIds.count(botQuestId) != 0)
                {
                    matchingQuestIds.insert(botQuestId);
                }
            }

            if (!matchingQuestIds.empty())
                return ChatFilter::Filter(message);

            return message;
        }

        return message;
    }
};

CompositeChatFilter::CompositeChatFilter(PlayerbotAI* ai) : ChatFilter(ai)
{
    filters.push_back(new StrategyChatFilter(ai));
    filters.push_back(new RoleChatFilter(ai));
    filters.push_back(new ClassChatFilter(ai));
    filters.push_back(new RtiChatFilter(ai));
    filters.push_back(new CombatTypeChatFilter(ai));
    filters.push_back(new LevelChatFilter(ai));
    filters.push_back(new GroupChatFilter(ai));
    filters.push_back(new GuildChatFilter(ai));
    filters.push_back(new StateChatFilter(ai));
    filters.push_back(new UsageChatFilter(ai));
    filters.push_back(new TalentSpecChatFilter(ai));
    filters.push_back(new LocationChatFilter(ai));
    filters.push_back(new RandomChatFilter(ai));
    filters.push_back(new GearChatFilter(ai));
    filters.push_back(new QuestChatFilter(ai));
    
}

CompositeChatFilter::~CompositeChatFilter()
{
    for (std::list<ChatFilter*>::iterator i = filters.begin(); i != filters.end(); i++)
        delete (*i);
}

std::string CompositeChatFilter::Filter(std::string message)
{
    for (int j = 0; j < filters.size(); ++j)
    {
        for (std::list<ChatFilter*>::iterator i = filters.begin(); i != filters.end(); i++)
        {
            message = (*i)->Filter(message);
            if (message.empty())
                break;
        }
    }

    return message;
}

