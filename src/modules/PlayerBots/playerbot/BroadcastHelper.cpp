
#include "playerbot/playerbot.h"
#include "BroadcastHelper.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/AiFactory.h"
#include "Chat/Channel.h"

using namespace ai;

BroadcastHelper::BroadcastHelper() {}

uint8 BroadcastHelper::GetLocale()
{
    uint8 locale = sConfig.GetIntDefault("DBC.Locale", 0 /*LocaleConstant::LOCALE_enUS*/);
    // -- In case we're using auto detect on config file^M
    if (locale >= MAX_LOCALE)
        locale = LocaleConstant::LOCALE_enUS;
    return locale;
}

bool BroadcastHelper::BroadcastTest(
    PlayerbotAI* ai,
    Player* bot
)
{
    //return something to ignore the logic
    return false;

    //don't think bots rejoin channnels when changing areas?
    //bot->GetPlayerbotMgr()->JoinChatChannels(bot);

    std::map<std::string, std::string> placeholders;
    placeholders["%rand1"] = std::to_string(urand(0, 1));
    placeholders["%rand2"] = std::to_string(urand(0, 1));
    placeholders["%rand3"] = std::to_string(urand(0, 1));

    int32 rand = urand(0, 1);

    if (rand == 1 && ai->SayToTrade(BOT_TEXT2("Posted to trade, %rand1, %rand2, %rand3", placeholders)))//+
        return true;
    else if (ai->SayToGuildRecruitment(BOT_TEXT2("Posted to GuildRecruitment, %rand1, %rand2, %rand3", placeholders)))//+
        return true;

    return ai->SayToTrade(BOT_TEXT2("Posted to trade, %rand1, %rand2, %rand3", placeholders));

    //int32 rand = urand(1, 8);
    if (rand == 1 && ai->SayToGuild(BOT_TEXT2("Posted to guild, %rand1, %rand2, %rand3", placeholders)))//+
        return true;
    else if (rand == 2 && ai->SayToWorld(BOT_TEXT2("Posted to world, %rand1, %rand2, %rand3", placeholders)))//+
        return true;
    else if (rand == 3 && ai->SayToGeneral(BOT_TEXT2("Posted to general, %rand1, %rand2, %rand3", placeholders)))//+
        return true;
    else if (rand == 4 && ai->SayToTrade(BOT_TEXT2("Posted to trade, %rand1, %rand2, %rand3", placeholders)))//+
        return true;
    else if (rand == 5 && ai->SayToLFG(BOT_TEXT2("Posted to LFG, %rand1, %rand2, %rand3", placeholders)))//
        return true;
    else if (rand == 6 && ai->SayToLocalDefense(BOT_TEXT2("Posted to LocalDefense, %rand1, %rand2, %rand3", placeholders)))//+
        return true;
    else if (rand == 7 && ai->SayToWorldDefense(BOT_TEXT2("Posted to WorldDefense, %rand1, %rand2, %rand3", placeholders)))//+
        return true;
    else if (rand == 8 && ai->SayToGuildRecruitment(BOT_TEXT2("Posted to GuildRecruitment, %rand1, %rand2, %rand3", placeholders)))//
        return true;

    return false;
}

/**
@param toChannels - map of (ToChannel, chance), where chance is in range 0-100 as uint32 (unless global chance is not 100%)

@return true if said to the channel, false otherwise
*/
bool BroadcastHelper::BroadcastToChannelWithGlobalChance(PlayerbotAI* ai, std::string message, std::list<std::pair<ToChannel, uint32>> toChannels)
{
    if (message.empty())
    {
        return false;
    }

    for (const auto& pair : toChannels) {
        uint32 roll = urand(1, 100);
        uint32 chance = pair.second;
        uint32 broadcastRoll = urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue);

        switch (pair.first)
        {
            case TO_GUILD:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.broadcastToGuildGlobalChance
                    && ai->SayToGuild(message))
                {
                    return true;
                }
                break;
            }
            case TO_WORLD:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.broadcastToWorldGlobalChance
                    && ai->SayToWorld(message))
                {
                    return true;
                }
                break;
            }
            case TO_GENERAL:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.broadcastToGeneralGlobalChance
                    && ai->SayToGeneral(message))
                {
                    return true;
                }
                break;
            }
            case TO_TRADE:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.broadcastToTradeGlobalChance
                    && ai->SayToTrade(message))
                {
                    return true;
                }
                break;
            }
            case TO_LOOKING_FOR_GROUP:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.broadcastToLFGGlobalChance
                    && ai->SayToLFG(message))
                {
                    return true;
                }
                break;
            }
            case TO_LOCAL_DEFENSE:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.broadcastToLocalDefenseGlobalChance
                    && ai->SayToLocalDefense(message))
                {
                    return true;
                }
                break;
            }
            case TO_WORLD_DEFENSE:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.broadcastToWorldDefenseGlobalChance
                    && ai->SayToWorldDefense(message))
                {
                    return true;
                }
                break;
            }
            case TO_GUILD_RECRUITMENT:
            {
                if (roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.broadcastToGuildRecruitmentGlobalChance
                    && ai->SayToGuildRecruitment(message))
                {
                    return true;
                }
                break;
            }
            case TO_SAY:
            {
                if (ai->HasPlayerNearby(sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_SAY))
                    && roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.broadcastToSayGlobalChance
                    && ai->Say(message))
                {
                    return true;
                }
                break;
            }
            case TO_YELL:
            {
                if (ai->HasPlayerNearby(sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_YELL))
                    && roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.broadcastToYellGlobalChance
                    && ai->Yell(message))
                {
                    return true;
                }
                break;
            }
            case TO_SAY_CITY:
            {
                if (ai->HasPlayerNearby(sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_SAY))
                    && ai->GetBot()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING)
                    && roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.broadcastToSayGlobalChance
                    && ai->Say(message))
                {
                    return true;
                }
                break;
            }
            case TO_YELL_CITY:
            {
                if (ai->HasPlayerNearby(sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_YELL))
                    && ai->GetBot()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING)
                    && roll <= chance
                    && broadcastRoll <= sPlayerbotAIConfig.broadcastToYellGlobalChance
                    && ai->Yell(message))
                {
                    return true;
                }
                break;
            }
            default:
                break;
        }
    }

    return false;
}

bool BroadcastHelper::BroadcastLootingItem(
    PlayerbotAI* ai,
    Player* bot,
    const ItemPrototype *proto,
    ItemQualifier& itemQualifier
    )
{
    std::map<std::string, std::string> placeholders;
    placeholders["%item_link"] = ai->GetChatHelper()->formatItem(itemQualifier);
    AreaTableEntry const* current_area = ai->GetCurrentArea();
    AreaTableEntry const* current_zone = ai->GetCurrentZone();
    placeholders["%area_name"] = current_area ? ai->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
    placeholders["%zone_name"] = current_zone ? ai->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
    placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
    placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
    placeholders["%my_level"] = std::to_string(bot->GetLevel());

    switch (proto->Quality)
    {
        case ITEM_QUALITY_POOR:
            if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceLootingItemPoor)
            {
                return BroadcastToChannelWithGlobalChance(
                    ai,
                    BOT_TEXT2("broadcast_looting_item_poor", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case ITEM_QUALITY_NORMAL:
            if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceLootingItemNormal)
            {
                return BroadcastToChannelWithGlobalChance(
                    ai,
                    BOT_TEXT2("broadcast_looting_item_normal", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case ITEM_QUALITY_UNCOMMON:
            if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceLootingItemUncommon)
            {
                return BroadcastToChannelWithGlobalChance(
                    ai,
                    BOT_TEXT2("broadcast_looting_item_uncommon", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case ITEM_QUALITY_RARE:
            if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceLootingItemRare)
            {
                return BroadcastToChannelWithGlobalChance(
                    ai,
                    BOT_TEXT2("broadcast_looting_item_rare", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case ITEM_QUALITY_EPIC:
            if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceLootingItemEpic)
            {
                return BroadcastToChannelWithGlobalChance(
                    ai,
                    BOT_TEXT2("broadcast_looting_item_epic", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case ITEM_QUALITY_LEGENDARY:
            if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceLootingItemLegendary)
            {
                return BroadcastToChannelWithGlobalChance(
                    ai,
                    BOT_TEXT2("broadcast_looting_item_legendary", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case ITEM_QUALITY_ARTIFACT:
            if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceLootingItemArtifact)
            {
                return BroadcastToChannelWithGlobalChance(
                    ai,
                    BOT_TEXT2("broadcast_looting_item_artifact", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        default:
            break;
    }

    return false;
}

bool BroadcastHelper::BroadcastQuestAccepted(
    PlayerbotAI* ai,
    Player* bot,
    const Quest* quest
)
{
    if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceQuestAccepted)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_link"] = ai->GetChatHelper()->formatQuest(quest);
        AreaTableEntry const* current_area = ai->GetCurrentArea();
        AreaTableEntry const* current_zone = ai->GetCurrentZone();
        placeholders["%area_name"] = current_area ? ai->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? ai->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
        placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
        placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("broadcast_quest_accepted_generic", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastQuestUpdateAddKill(
    PlayerbotAI* ai,
    Player* bot,
    Quest const* quest,
    uint32 availableCount,
    uint32 requiredCount,
    std::string obectiveName
)
{
    std::map<std::string, std::string> placeholders;
    AreaTableEntry const* current_area = ai->GetCurrentArea();
    AreaTableEntry const* current_zone = ai->GetCurrentZone();
    placeholders["%area_name"] = current_area ? ai->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
    placeholders["%zone_name"] = current_zone ? ai->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
    placeholders["%quest_link"] = ai->GetChatHelper()->formatQuest(quest);
    placeholders["%quest_obj_name"] = obectiveName;
    placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
    placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
    placeholders["%my_level"] = std::to_string(bot->GetLevel());
    placeholders["%quest_obj_available"] = std::to_string(availableCount);
    placeholders["%quest_obj_required"] = std::to_string(requiredCount);
    placeholders["%quest_obj_missing"] = std::to_string(requiredCount - std::min(availableCount, requiredCount));
    placeholders["%quest_obj_full_formatted"] = ai->GetChatHelper()->formatQuestObjective(obectiveName, availableCount, requiredCount);

    if (availableCount < requiredCount
        && urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceQuestUpdateObjectiveProgress)
    {
        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("broadcast_quest_update_add_kill_objective_progress", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }
    else if (availableCount == requiredCount
        && urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceQuestUpdateObjectiveCompleted)
    {
        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("broadcast_quest_update_add_kill_objective_completed", placeholders),
            { {TO_YELL, 10}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastQuestUpdateAddItem(
    PlayerbotAI* ai,
    Player* bot,
    Quest const* quest,
    uint32 availableCount,
    uint32 requiredCount,
    const ItemPrototype* proto
)
{
    std::map<std::string, std::string> placeholders;
    AreaTableEntry const* current_area = ai->GetCurrentArea();
    AreaTableEntry const* current_zone = ai->GetCurrentZone();
    placeholders["%area_name"] = current_area ? ai->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
    placeholders["%zone_name"] = current_zone ? ai->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
    placeholders["%quest_link"] = ai->GetChatHelper()->formatQuest(quest);
    std::string itemLinkFormatted = ai->GetChatHelper()->formatItem(proto);
    placeholders["%item_link"] = itemLinkFormatted;
    placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
    placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
    placeholders["%my_level"] = std::to_string(bot->GetLevel());
    placeholders["%quest_obj_available"] = std::to_string(availableCount);
    placeholders["%quest_obj_required"] = std::to_string(requiredCount);
    placeholders["%quest_obj_missing"] = std::to_string(requiredCount - std::min(availableCount, requiredCount));
    placeholders["%quest_obj_full_formatted"] = ai->GetChatHelper()->formatQuestObjective(itemLinkFormatted, availableCount, requiredCount);

    if (availableCount < requiredCount
        && urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceQuestUpdateObjectiveProgress)
    {
        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("broadcast_quest_update_add_item_objective_progress", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }
    else if (availableCount == requiredCount
        && urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceQuestUpdateObjectiveCompleted)
    {
        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("broadcast_quest_update_add_item_objective_completed", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastQuestUpdateFailedTimer(
    PlayerbotAI* ai,
    Player* bot,
    Quest const* quest
)
{
    if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceQuestUpdateFailedTimer)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_link"] = ai->GetChatHelper()->formatQuest(quest);
        AreaTableEntry const* current_area = ai->GetCurrentArea();
        AreaTableEntry const* current_zone = ai->GetCurrentZone();
        placeholders["%area_name"] = current_area ? ai->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? ai->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
        placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
        placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("broadcast_quest_update_failed_timer", placeholders),
            { {TO_YELL, 10}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastQuestUpdateComplete(
    PlayerbotAI* ai,
    Player* bot,
    Quest const* quest
)
{
    if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceQuestUpdateComplete)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_link"] = ai->GetChatHelper()->formatQuest(quest);
        AreaTableEntry const* current_area = ai->GetCurrentArea();
        AreaTableEntry const* current_zone = ai->GetCurrentZone();
        placeholders["%area_name"] = current_area ? ai->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? ai->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
        placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
        placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());


        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("broadcast_quest_update_complete", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastQuestTurnedIn(
    PlayerbotAI* ai,
    Player* bot,
    Quest const* quest
)
{
    if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceQuestTurnedIn)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_link"] = ai->GetChatHelper()->formatQuest(quest);
        AreaTableEntry const* current_area = ai->GetCurrentArea();
        AreaTableEntry const* current_zone = ai->GetCurrentZone();
        placeholders["%area_name"] = current_area ? ai->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? ai->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
        placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
        placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("broadcast_quest_turned_in", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastCreatureKill(
    PlayerbotAI* ai,
    Player* bot,
    Creature *creature
)
{
    std::map<std::string, std::string> placeholders;
    placeholders["%victim_name"] = creature->GetName();
    AreaTableEntry const* current_area = ai->GetCurrentArea();
    AreaTableEntry const* current_zone = ai->GetCurrentZone();
    placeholders["%area_name"] = current_area ? ai->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
    placeholders["%zone_name"] = current_zone ? ai->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
    placeholders["%victim_level"] = creature->GetLevel();
    placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
    placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
    placeholders["%my_level"] = std::to_string(bot->GetLevel());

    //if ((creature->IsElite() && !creature->GetMap()->IsDungeon())
    //if creature->IsWorldBoss()
    //if creature->GetLevel() > DEFAULT_MAX_LEVEL + 1
    //if creature->GetLevel() > bot->GetLevel() + 4

    if (creature->IsPet())
    {
        if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceKillPet)
        {
            return BroadcastToChannelWithGlobalChance(
                ai,
                BOT_TEXT2("broadcast_killed_pet", placeholders),
                { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
            );
        }
    }
    else
    {
        switch (creature->GetCreatureInfo()->Rank)
        {
        case CREATURE_ELITE_NORMAL:
            if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceKillNormal)
            {
                return BroadcastToChannelWithGlobalChance(
                    ai,
                    BOT_TEXT2("broadcast_killed_normal", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case CREATURE_ELITE_ELITE:
            if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceKillElite)
            {
                return BroadcastToChannelWithGlobalChance(
                    ai,
                    BOT_TEXT2("broadcast_killed_elite", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case CREATURE_ELITE_RAREELITE:
            if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceKillRareelite)
            {
                return BroadcastToChannelWithGlobalChance(
                    ai,
                    BOT_TEXT2("broadcast_killed_rareelite", placeholders),
                    { {TO_YELL, 10}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case CREATURE_ELITE_WORLDBOSS:
            if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceKillWorldboss)
            {
                return BroadcastToChannelWithGlobalChance(
                    ai,
                    BOT_TEXT2("broadcast_killed_worldboss", placeholders),
                    { {TO_YELL, 10}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case CREATURE_ELITE_RARE:
            if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceKillRare)
            {
                return BroadcastToChannelWithGlobalChance(
                    ai,
                    BOT_TEXT2("broadcast_killed_rare", placeholders),
                    { {TO_YELL, 10}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        case CREATURE_UNKNOWN:
            if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceKillUnknown)
            {
                return BroadcastToChannelWithGlobalChance(
                    ai,
                    BOT_TEXT2("broadcast_killed_unknown", placeholders),
                    { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
                );
            }
            break;
        default:
            break;
        }
    }

    return false;
}

bool BroadcastHelper::BroadcastPlayerKill(
    PlayerbotAI* ai,
    Player* bot,
    Player* victim
)
{
    std::map<std::string, std::string> placeholders;
    placeholders["%victim_name"] = victim->GetName();
    AreaTableEntry const* current_area = ai->GetCurrentArea();
    AreaTableEntry const* current_zone = ai->GetCurrentZone();
    placeholders["%area_name"] = current_area ? ai->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
    placeholders["%zone_name"] = current_zone ? ai->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
    placeholders["%victim_level"] = victim->GetLevel();
    placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
    placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
    placeholders["%my_level"] = std::to_string(bot->GetLevel());

    if (victim)
    {
        if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceKillPlayer)
        {
            placeholders["%victim_class"] = ai->GetChatHelper()->formatClass(victim->getClass());

            return BroadcastToChannelWithGlobalChance(
                ai,
                BOT_TEXT2("broadcast_killed_player", placeholders),
                { {TO_WORLD_DEFENSE, 50}, {TO_LOCAL_DEFENSE, 50}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
            );
        }
    }

    return false;
}

bool BroadcastHelper::BroadcastLevelup(
    PlayerbotAI* ai,
    Player* bot
)
{
    uint32 level = bot->GetLevel();

    std::map<std::string, std::string> placeholders;
    AreaTableEntry const* current_area = ai->GetCurrentArea();
    AreaTableEntry const* current_zone = ai->GetCurrentZone();
    placeholders["%area_name"] = current_area ? ai->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
    placeholders["%zone_name"] = current_zone ? ai->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
    placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
    placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
    placeholders["%my_level"] = std::to_string(level);

    if (level == sPlayerbotAIConfig.randomBotMaxLevel
        && urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceLevelupMaxLevel)
    {
        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("broadcast_levelup_max_level", placeholders),
            { {TO_GUILD, 30}, {TO_WORLD, 90}, {TO_GENERAL, 100} }
        );
    }
    // It's divisible by 10
    else if (level % 10 == 0
        && urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceLevelupTenX)
    {
        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("broadcast_levelup_10x", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 90}, {TO_GENERAL, 100} }
        );
    }
    else if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceLevelupGeneric)
    {
        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("broadcast_levelup_generic", placeholders),
            { {TO_GUILD, 90}, {TO_WORLD, 90}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastGuildMemberPromotion(
    PlayerbotAI* ai,
    Player* bot,
    Player* player
)
{
    if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceGuildManagement)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%other_name"] = player->GetName();
        placeholders["%other_class"] = ai->GetChatHelper()->formatClass(player->getClass());
        placeholders["%other_race"] = ai->GetChatHelper()->formatRace(player->getRace());
        placeholders["%other_level"] = std::to_string(player->GetLevel());

        return ai->SayToGuild(BOT_TEXT2("broadcast_guild_promotion", placeholders));
    }

    return false;
}

bool BroadcastHelper::BroadcastGuildMemberDemotion(
    PlayerbotAI* ai,
    Player* bot,
    Player* player
)
{
    if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceGuildManagement)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%other_name"] = player->GetName();
        placeholders["%other_class"] = ai->GetChatHelper()->formatClass(player->getClass());
        placeholders["%other_race"] = ai->GetChatHelper()->formatRace(player->getRace());
        placeholders["%other_level"] = std::to_string(player->GetLevel());

        return ai->SayToGuild(BOT_TEXT2("broadcast_guild_demotion", placeholders));
    }

    return false;
}

bool BroadcastHelper::BroadcastGuildGroupOrRaidInvite(
    PlayerbotAI* ai,
    Player* bot,
    Player* player,
    Group* group,
    Guild* guild
)
{
    std::map<std::string, std::string> placeholders;
    placeholders["%name"] = player->GetName();
    AreaTableEntry const* current_area = ai->GetCurrentArea();
    AreaTableEntry const* current_zone = ai->GetCurrentZone();
    placeholders["%area_name"] = current_area ? ai->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
    placeholders["%zone_name"] = current_zone ? ai->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");

    //TODO move texts to sql!
    if (group && group->IsRaidGroup())
    {
        if (urand(0, 3))
        {
            return ai->SayToGuild(BOT_TEXT2("Hey anyone want to raid in %zone_name", placeholders));
        }
        else
        {
            return ai->SayToGuild(BOT_TEXT2("Hey %name I'm raiding in %zone_name do you want to join me?", placeholders));
        }
    }
    else
    {
        //(bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH)
        if (urand(0, 3))
        {
            return ai->SayToGuild(BOT_TEXT2("Hey anyone wanna group up in %zone_name?", placeholders));
        }
        else
        {
            return ai->SayToGuild(BOT_TEXT2("Hey %name do you want join my group? I'm heading for %zone_name", placeholders));
        }
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestInstance(
    PlayerbotAI* ai,
    std::vector<std::string>& allowedInstances,
    Player* bot
)
{
    if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceSuggestInstance)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%my_role"] = ai->GetChatHelper()->formatClass(bot, AiFactory::GetPlayerSpecTab(bot));

        std::ostringstream itemout;
        //itemout << "|c00b000b0" << allowedInstances[urand(0, allowedInstances.size() - 1)] << "|r";
        itemout << allowedInstances[urand(0, allowedInstances.size() - 1)];
        placeholders["%instance_name"] = itemout.str();

        placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
        placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("suggest_instance", placeholders),
            { {TO_SAY_CITY, 50}, {TO_YELL_CITY, 10}, {TO_LOOKING_FOR_GROUP, 50}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestQuest(
    PlayerbotAI* ai,
    std::vector<uint32>& quests,
    Player* bot
)
{
    if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceSuggestQuest)
    {

        int index = rand() % quests.size();

        Quest const* quest = sObjectMgr.GetQuestTemplate(quests[index]);

        std::map<std::string, std::string> placeholders;
        placeholders["%my_role"] = ai->GetChatHelper()->formatClass(bot, AiFactory::GetPlayerSpecTab(bot));
        placeholders["%quest_link"] = ai->GetChatHelper()->formatQuest(quest);
        placeholders["%quest_level"] = std::to_string(quest->GetQuestLevel());
        placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
        placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("suggest_quest", placeholders),
            { {TO_SAY_CITY, 50}, {TO_YELL_CITY, 10}, {TO_LOOKING_FOR_GROUP, 50}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestGrindMaterials(
    PlayerbotAI* ai,
    std::string item,
    Player* bot
)
{
    if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceSuggestGrindMaterials)
    {

        std::map<std::string, std::string> placeholders;
        placeholders["%my_role"] = ai->GetChatHelper()->formatClass(bot, AiFactory::GetPlayerSpecTab(bot));
        placeholders["%category"] = item;

        placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
        placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("suggest_trade", placeholders),
            { {TO_SAY_CITY, 50}, {TO_YELL_CITY, 10}, {TO_TRADE, 50}, {TO_LOOKING_FOR_GROUP, 50}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestGrindReputation(
    PlayerbotAI* ai,
    std::vector<std::string> levels,
    std::vector<std::string> allowedFactions,
    Player* bot
)
{
    if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceSuggestGrindReputation)
    {

        std::map<std::string, std::string> placeholders;
        placeholders["%my_role"] = ai->GetChatHelper()->formatClass(bot, AiFactory::GetPlayerSpecTab(bot));
        placeholders["%rep_level"] = levels[urand(0, 2)];
        std::ostringstream rnd; rnd << urand(1, 5) << "K";
        placeholders["%rndK"] = rnd.str();

        std::ostringstream itemout;
        //itemout << "|c004040b0" << allowedFactions[urand(0, allowedFactions.size() - 1)] << "|r";
        itemout << allowedFactions[urand(0, allowedFactions.size() - 1)];
        placeholders["%faction"] = itemout.str();

        placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
        placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("suggest_faction", placeholders),
            { {TO_SAY_CITY, 50}, {TO_YELL_CITY, 10}, {TO_LOOKING_FOR_GROUP, 50}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestSell(
    PlayerbotAI* ai,
    const ItemPrototype* proto,
    uint32 count,
    uint32 price,
    Player* bot
)
{
    if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceSuggestSell)
    {

        std::map<std::string, std::string> placeholders;
        placeholders["%item_link"] = ai->GetChatHelper()->formatItem(proto, 0);
        placeholders["%item_formatted_link"] = ai->GetChatHelper()->formatItem(proto, count);
        placeholders["%item_count"] = std::to_string(count);
        placeholders["%cost_gold"] = ai->GetChatHelper()->formatMoney(price);

        placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
        placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("suggest_sell", placeholders),
            { {TO_SAY_CITY, 50}, {TO_YELL_CITY, 10}, {TO_TRADE, 90}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestSomething(
    PlayerbotAI* ai,
    Player* bot
)
{
    if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceSuggestSomething)
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%my_role"] = ai->GetChatHelper()->formatClass(bot, AiFactory::GetPlayerSpecTab(bot));

        AreaTableEntry const* current_area = ai->GetCurrentArea();
        AreaTableEntry const* current_zone = ai->GetCurrentZone();
        placeholders["%area_name"] = current_area ? ai->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? ai->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
        placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
        placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("suggest_something", placeholders),
            { {TO_SAY, 70}, {TO_YELL, 10},{TO_GUILD, 10}, {TO_WORLD, 70}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestSomethingToxic(
    PlayerbotAI* ai,
    Player* bot
)
{
    if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceSuggestSomethingToxic)
    {
        //items
        std::vector<Item*> botItems = ai->GetInventoryAndEquippedItems();

        std::map<std::string, std::string> placeholders;

        placeholders["%random_inventory_item_link"] = botItems.size() > 0 ? ai->GetChatHelper()->formatItem(botItems[rand() % botItems.size()]) : BOT_TEXT("string_empty_link");

        placeholders["%my_role"] = ai->GetChatHelper()->formatClass(bot, AiFactory::GetPlayerSpecTab(bot));
        AreaTableEntry const* current_area = ai->GetCurrentArea();
        AreaTableEntry const* current_zone = ai->GetCurrentZone();
        placeholders["%area_name"] = current_area ? ai->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? ai->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
        placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
        placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("suggest_something_toxic", placeholders),
            { {TO_SAY, 70}, {TO_YELL, 10},{TO_GUILD, 10}, {TO_WORLD, 70}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestToxicLinks(
    PlayerbotAI* ai,
    Player* bot
)
{
    if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceSuggestToxicLinks)
    {
        //quests
        std::vector<uint32> incompleteQuests;
        for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 questId = bot->GetQuestSlotQuestId(slot);
            if (!questId)
                continue;

            QuestStatus status = bot->GetQuestStatus(questId);
            if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_NONE)
                incompleteQuests.push_back(questId);
        }

        //items
        std::vector<Item*> botItems = ai->GetInventoryAndEquippedItems();

        //spells
        //?

        std::map<std::string, std::string> placeholders;

        placeholders["%random_inventory_item_link"] = botItems.size() > 0 ? ai->GetChatHelper()->formatItem(botItems[rand() % botItems.size()]) : BOT_TEXT("string_empty_link");
        placeholders["%prefix"] = sPlayerbotAIConfig.toxicLinksPrefix;

        if (incompleteQuests.size() > 0)
        {
            Quest const* quest = sObjectMgr.GetQuestTemplate(incompleteQuests[rand() % incompleteQuests.size()]);
            placeholders["%random_taken_quest_or_item_link"] = ai->GetChatHelper()->formatQuest(quest);
        }
        else
        {
            placeholders["%random_taken_quest_or_item_link"] = placeholders["%random_inventory_item_link"];
        }

        placeholders["%my_role"] = ai->GetChatHelper()->formatClass(bot, AiFactory::GetPlayerSpecTab(bot));
        AreaTableEntry const* current_area = ai->GetCurrentArea();
        AreaTableEntry const* current_zone = ai->GetCurrentZone();
        placeholders["%area_name"] = current_area ? ai->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? ai->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
        placeholders["%my_class"] = ai->GetChatHelper()->formatClass(bot->getClass());
        placeholders["%my_race"] = ai->GetChatHelper()->formatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());

        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("suggest_toxic_links", placeholders),
            { {TO_SAY, 70}, {TO_YELL, 10},{TO_GUILD, 10}, {TO_WORLD, 70}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

bool BroadcastHelper::BroadcastSuggestThunderfury(
    PlayerbotAI* ai,
    Player* bot
)
{
    if (urand(1, sPlayerbotAIConfig.broadcastChanceMaxValue) <= sPlayerbotAIConfig.broadcastChanceSuggestThunderfury)
    {
        //items
        std::vector<Item*> botItems = ai->GetInventoryAndEquippedItems();

        std::map<std::string, std::string> placeholders;
        ItemPrototype const* thunderfuryProto = sObjectMgr.GetItemPrototype(19019);
        placeholders["%thunderfury_link"] = bot->GetPlayerbotAI()->GetChatHelper()->formatItem(thunderfuryProto);
        placeholders["%random_inventory_item_link"] = botItems.size() > 0 ? ai->GetChatHelper()->formatItem(botItems[rand() % botItems.size()]) : BOT_TEXT("string_empty_link");

        return BroadcastToChannelWithGlobalChance(
            ai,
            BOT_TEXT2("thunderfury_spam", placeholders),
            { {TO_SAY, 70}, {TO_YELL, 10},{TO_WORLD, 70}, {TO_GENERAL, 100} }
        );
    }

    return false;
}