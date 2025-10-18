
#include "playerbot/playerbot.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "PlayerbotTextMgr.h"

#include "Database/DatabaseEnv.h"
#include "PlayerbotAI.h"

PlayerbotTextMgr::PlayerbotTextMgr()
{
    for (uint8 i = 1; i < MAX_LOCALE; ++i)
    {
        botTextLocalePriority[sObjectMgr.GetStorageLocaleIndexFor(LocaleConstant(i))] = 0;
    }
}

PlayerbotTextMgr::~PlayerbotTextMgr()
{
}

void PlayerbotTextMgr::ReplaceAll(std::string & str, const std::string & from, const std::string & to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

void PlayerbotTextMgr::ReplacePlaceholders(std::string& text, const std::map<std::string, std::string>& placeholders)
{
    for (auto& placeholder : placeholders)
        ReplaceAll(text, placeholder.first, placeholder.second);
}

void PlayerbotTextMgr::LoadBotTexts()
{
    sLog.outBasic("Loading playerbot texts...");
    auto results = WorldDatabase.PQuery("SELECT `name`, `text`, `say_type`, `reply_type`, `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`, `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8` FROM `ai_playerbot_texts`");
    int count = 0;
    if (results)
    {
        do
        {
            std::string text;
            std::map<int32, std::string> text_locale;
            Field* fields = results->Fetch();
            std::string name = fields[0].GetString();
            text = fields[1].GetString();
            uint32 sayType = fields[2].GetUInt32();
            uint32 replyType = fields[3].GetUInt32();
            for (uint8 i = 1; i < MAX_LOCALE; ++i)
            {
                text_locale[sObjectMgr.GetStorageLocaleIndexFor(LocaleConstant(i))] = fields[i + 3].GetString();
            }
            botTexts[name].push_back(BotTextEntry(name, text, text_locale, sayType, replyType));
            count++;
        } while (results->NextRow());
    }
    sLog.outBasic("%d playerbot texts loaded", count);
}

void PlayerbotTextMgr::LoadBotTextChance()
{
    if (botTextChance.empty())
    {
        auto results = WorldDatabase.PQuery("SELECT name, probability FROM ai_playerbot_texts_chance");
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                std::string name = fields[0].GetString();
                uint32 probability = fields[1].GetUInt32();

                botTextChance[name] = probability;
            } while (results->NextRow());
        }
    }
}

// general texts

std::string PlayerbotTextMgr::GetBotText(std::string name)
{
    if (botTexts.empty())
    {
        sLog.outError("Can't get bot text %s! No bots texts loaded!", name.c_str());
        return "";
    }
    if (botTexts[name].empty())
    {
        sLog.outDetail("Can't get bot text %s! No bots texts for this name!", name.c_str());
        return "";
    }

    std::vector<BotTextEntry>& list = botTexts[name];
    BotTextEntry textEntry = list[urand(0, list.size() - 1)];
    int32 localePrio = GetLocalePriority();
    if (localePrio == -1)
        return textEntry.m_text;
    else
    {
        if (!textEntry.m_text_locales[localePrio].empty())
            return textEntry.m_text_locales[localePrio];
        else
            return textEntry.m_text;
    }
}

std::string PlayerbotTextMgr::GetBotText(std::string name, std::map<std::string, std::string> placeholders)
{
    std::string botText = GetBotText(name);
    if (botText.empty())
        botText = name;

    ReplacePlaceholders(botText, placeholders);

    return botText;
}

// chat replies

std::string PlayerbotTextMgr::GetBotText(ChatReplyType replyType, std::map<std::string, std::string> placeholders)
{
    if (botTexts.empty())
    {
        sLog.outError("Can't get bot text reply %u! No bots texts loaded!", replyType);
        return "";
    }
    if (botTexts["reply"].empty())
    {
        sLog.outDetail("Can't get bot text reply %u! No bots texts replies!", replyType);
        return "";
    }

    std::vector<BotTextEntry>& list = botTexts["reply"];
    std::vector<BotTextEntry> proper_list;
    for (auto text : list)
    {
        if (text.m_replyType == replyType)
            proper_list.push_back(text);
    }

    BotTextEntry textEntry = proper_list[urand(0, proper_list.size() - 1)];
    std::string botText;
    int32 localePrio = GetLocalePriority();
    if (localePrio == -1)
        botText = textEntry.m_text;
    else
    {
        if (!textEntry.m_text_locales[localePrio].empty())
            botText = textEntry.m_text_locales[localePrio];
        else
            botText = textEntry.m_text;
    }

    ReplacePlaceholders(botText, placeholders);

    return botText;
}

std::string PlayerbotTextMgr::GetBotText(ChatReplyType replyType, std::string name)
{
    std::map<std::string, std::string> placeholders;
    placeholders["%s"] = name;

    return GetBotText(replyType, placeholders);
}

// probabilities

bool PlayerbotTextMgr::rollTextChance(std::string name)
{
    if (!botTextChance[name])
        return true;

    return urand(0, 100) < botTextChance[name];
}

bool PlayerbotTextMgr::GetBotText(std::string name, std::string &text)
{
    if (!rollTextChance(name))
        return false;

    text = GetBotText(name);
    return !text.empty();
}

bool PlayerbotTextMgr::GetBotText(std::string name, std::string& text, std::map<std::string, std::string> placeholders)
{
    if (!rollTextChance(name))
        return false;

    text = GetBotText(name, placeholders);
    return !text.empty();
}

void PlayerbotTextMgr::AddLocalePriority(int32 locale)
{
    if (locale == -1)
        return;

    botTextLocalePriority[locale]++;
}

int32 PlayerbotTextMgr::GetLocalePriority()
{
    int32 topLocale = -1;
    uint32 tempCheck = 0;

    // if no real players online, reset top locale
    if (!sWorld.GetActiveSessionCount())
    {
        ResetLocalePriority();
        return 0;
    }

    for (uint8 i = 1; i < MAX_LOCALE; ++i)
    {
        int32 locale_idx = sObjectMgr.GetStorageLocaleIndexFor(LocaleConstant(i));
        if (locale_idx == -1)
            continue;

        if (botTextLocalePriority[locale_idx] > tempCheck)
        {
            tempCheck = botTextLocalePriority[locale_idx];
            topLocale = locale_idx;
        }
    }

    return topLocale;
}

void PlayerbotTextMgr::ResetLocalePriority()
{
    for (uint8 i = 1; i < MAX_LOCALE; ++i)
    {
        botTextLocalePriority[sObjectMgr.GetStorageLocaleIndexFor(LocaleConstant(i))] = 0;
    }
}