
#include "playerbot/playerbot.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/PlayerbotFactory.h"
#include "PlayerbotDbStore.h"
#include <cstdlib>
#include <iostream>

#include "LootObjectStack.h"
#include "strategy/values/Formations.h"
#include "strategy/values/PositionValue.h"
INSTANTIATE_SINGLETON_1(PlayerbotDbStore);

using namespace ai;

void PlayerbotDbStore::Load(PlayerbotAI *ai, std::string preset)
{
    uint64 guid = ai->GetBot()->GetObjectGuid().GetRawValue();

    auto results = CharacterDatabase.PQuery("SELECT `key`,`value` FROM `ai_playerbot_db_store` WHERE `guid` = '%lu' AND `preset` = '%s'", guid, preset.c_str());
    if (results)
    {
        ai->ClearStrategies(BotState::BOT_STATE_COMBAT);
        ai->ClearStrategies(BotState::BOT_STATE_NON_COMBAT);
        ai->ChangeStrategy("+chat", BotState::BOT_STATE_COMBAT);
        ai->ChangeStrategy("+chat", BotState::BOT_STATE_NON_COMBAT);

        std::list<std::string> values;
        do
        {
            Field* fields = results->Fetch();
            std::string key = fields[0].GetString();
            std::string value = fields[1].GetString();
            if (key == "value") values.push_back(value);
            else if (key == "co") ai->ChangeStrategy(value, BotState::BOT_STATE_COMBAT);
            else if (key == "nc") ai->ChangeStrategy(value, BotState::BOT_STATE_NON_COMBAT);
            else if (key == "dead") ai->ChangeStrategy(value, BotState::BOT_STATE_DEAD);
            else if (key == "react") ai->ChangeStrategy(value, BotState::BOT_STATE_REACTION);
        } while (results->NextRow());

        ai->GetAiObjectContext()->Load(values);
    }
}

void PlayerbotDbStore::Save(PlayerbotAI *ai, std::string preset)
{
    uint64 guid = ai->GetBot()->GetObjectGuid().GetRawValue();

    Reset(ai, preset);

    std::list<std::string> data = ai->GetAiObjectContext()->Save();
    for (std::list<std::string>::iterator i = data.begin(); i != data.end(); ++i)
    {
        SaveValue(guid, preset, "value", *i);
    }

    SaveValue(guid, preset, "co", FormatStrategies("co", ai->GetStrategies(BotState::BOT_STATE_COMBAT)));
    SaveValue(guid, preset, "nc", FormatStrategies("nc", ai->GetStrategies(BotState::BOT_STATE_NON_COMBAT)));
    SaveValue(guid, preset, "dead", FormatStrategies("dead", ai->GetStrategies(BotState::BOT_STATE_DEAD)));
    SaveValue(guid, preset, "react", FormatStrategies("react", ai->GetStrategies(BotState::BOT_STATE_REACTION)));
}

std::string PlayerbotDbStore::FormatStrategies(std::string type, std::list<std::string_view> strategies)
{
    std::ostringstream out;
    for(const auto& strategy : strategies)
        out << "+" << strategy << ",";

    std::string res = out.str();
    return res.substr(0, res.size() - 1);
}

void PlayerbotDbStore::Reset(PlayerbotAI *ai, std::string preset)
{
    uint64 guid = ai->GetBot()->GetObjectGuid().GetRawValue();
    uint32 account = sObjectMgr.GetPlayerAccountIdByGUID(ObjectGuid(guid));

    CharacterDatabase.PExecute("DELETE FROM `ai_playerbot_db_store` WHERE `guid` = '%lu' AND `preset` = '%s'", guid, preset.c_str());
}

void PlayerbotDbStore::SaveValue(uint64 guid, std::string preset, std::string key, std::string value)
{
    CharacterDatabase.PExecute("INSERT INTO `ai_playerbot_db_store` (`guid`, `preset`, `key`, `value`) VALUES ('%lu', '%s', '%s', '%s')", guid, preset.c_str(), key.c_str(), value.c_str());
}
