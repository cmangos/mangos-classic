#include "Config/Config.h"

#include "playerbot/playerbot.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/PlayerbotFactory.h"
#include "Accounts/AccountMgr.h"
#include "Globals/ObjectMgr.h"
#include "Database/DatabaseEnv.h"
#include "PlayerbotAI.h"
#include "Entities/Player.h"
#include "RandomPlayerbotFactory.h"
#include "SystemConfig.h"
#include "Social/SocialMgr.h"
#include "Guilds/GuildMgr.h"

#ifndef MANGOSBOT_ZERO
#ifdef CMANGOS
#include "Arena/ArenaTeam.h"
#endif
#ifdef MANGOS
#include "ArenaTeam.h"
#endif
#endif

#include <random>

std::map<uint8, std::vector<uint8> > RandomPlayerbotFactory::availableRaces;

constexpr RandomPlayerbotFactory::NameRaceAndGender RandomPlayerbotFactory::CombineRaceAndGender(uint8 gender, uint8 race)
{
    switch(race)
    {
    case RACE_HUMAN:    return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::GenericMale)  + gender);
    case RACE_ORC:      return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::OrcMale)      + gender);
    case RACE_DWARF:    return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::DwarfMale)    + gender);
    case RACE_NIGHTELF: return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::NightelfMale) + gender);
    case RACE_UNDEAD:   return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::GenericMale)  + gender);
    case RACE_TAUREN:   return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::TaurenMale)   + gender);
    case RACE_GNOME:    return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::GnomeMale)    + gender);
    case RACE_TROLL:    return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::TrollMale)    + gender);
#ifndef MANGOSBOT_ZERO
    case RACE_DRAENEI:  return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::DraeneiMale)  + gender);
    case RACE_BLOODELF: return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::BloodelfMale) + gender);
#endif
    default:
        sLog.outError("The race with ID %d does not have a naming category", race);
        return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::GenericMale) + gender);
    }
}

RandomPlayerbotFactory::RandomPlayerbotFactory(uint32 accountId) : accountId(accountId)
{
    availableRaces[CLASS_WARRIOR].push_back(RACE_HUMAN);
    availableRaces[CLASS_WARRIOR].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_WARRIOR].push_back(RACE_GNOME);
    availableRaces[CLASS_WARRIOR].push_back(RACE_DWARF);
    availableRaces[CLASS_WARRIOR].push_back(RACE_ORC);
    availableRaces[CLASS_WARRIOR].push_back(RACE_UNDEAD);
    availableRaces[CLASS_WARRIOR].push_back(RACE_TAUREN);
    availableRaces[CLASS_WARRIOR].push_back(RACE_TROLL);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_WARRIOR].push_back(RACE_DRAENEI);
#endif

    availableRaces[CLASS_PALADIN].push_back(RACE_HUMAN);
    availableRaces[CLASS_PALADIN].push_back(RACE_DWARF);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_PALADIN].push_back(RACE_DRAENEI);
    availableRaces[CLASS_PALADIN].push_back(RACE_BLOODELF);
#endif

    availableRaces[CLASS_ROGUE].push_back(RACE_HUMAN);
    availableRaces[CLASS_ROGUE].push_back(RACE_DWARF);
    availableRaces[CLASS_ROGUE].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_ROGUE].push_back(RACE_GNOME);
    availableRaces[CLASS_ROGUE].push_back(RACE_ORC);
    availableRaces[CLASS_ROGUE].push_back(RACE_UNDEAD);
    availableRaces[CLASS_ROGUE].push_back(RACE_TROLL);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_ROGUE].push_back(RACE_BLOODELF);
#endif

    availableRaces[CLASS_PRIEST].push_back(RACE_HUMAN);
    availableRaces[CLASS_PRIEST].push_back(RACE_DWARF);
    availableRaces[CLASS_PRIEST].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_PRIEST].push_back(RACE_TROLL);
    availableRaces[CLASS_PRIEST].push_back(RACE_UNDEAD);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_PRIEST].push_back(RACE_DRAENEI);
    availableRaces[CLASS_PRIEST].push_back(RACE_BLOODELF);
#endif

    availableRaces[CLASS_MAGE].push_back(RACE_HUMAN);
    availableRaces[CLASS_MAGE].push_back(RACE_GNOME);
    availableRaces[CLASS_MAGE].push_back(RACE_UNDEAD);
    availableRaces[CLASS_MAGE].push_back(RACE_TROLL);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_MAGE].push_back(RACE_DRAENEI);
    availableRaces[CLASS_MAGE].push_back(RACE_BLOODELF);
#endif

    availableRaces[CLASS_WARLOCK].push_back(RACE_HUMAN);
    availableRaces[CLASS_WARLOCK].push_back(RACE_GNOME);
    availableRaces[CLASS_WARLOCK].push_back(RACE_UNDEAD);
    availableRaces[CLASS_WARLOCK].push_back(RACE_ORC);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_WARLOCK].push_back(RACE_BLOODELF);
#endif

    availableRaces[CLASS_SHAMAN].push_back(RACE_ORC);
    availableRaces[CLASS_SHAMAN].push_back(RACE_TAUREN);
    availableRaces[CLASS_SHAMAN].push_back(RACE_TROLL);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_SHAMAN].push_back(RACE_DRAENEI);
#endif

    availableRaces[CLASS_HUNTER].push_back(RACE_DWARF);
    availableRaces[CLASS_HUNTER].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_HUNTER].push_back(RACE_ORC);
    availableRaces[CLASS_HUNTER].push_back(RACE_TAUREN);
    availableRaces[CLASS_HUNTER].push_back(RACE_TROLL);
#ifndef MANGOSBOT_ZERO
    availableRaces[CLASS_HUNTER].push_back(RACE_DRAENEI);
    availableRaces[CLASS_HUNTER].push_back(RACE_BLOODELF);
#endif

    availableRaces[CLASS_DRUID].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_DRUID].push_back(RACE_TAUREN);

#ifdef MANGOSBOT_TWO
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_NIGHTELF);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_TAUREN);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_HUMAN);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_ORC);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_UNDEAD);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_TROLL);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_BLOODELF);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_DRAENEI);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_GNOME);
    availableRaces[CLASS_DEATH_KNIGHT].push_back(RACE_DWARF);
#endif
}

bool RandomPlayerbotFactory::isAvailableRace(uint8 cls, uint8 race)
{
    if (race == RACE_GOBLIN)
        return false;
#ifdef MANGOSBOT_TWO
    else if (cls == 10)
#else
    else if (cls == 10 || cls == 6)
#endif
        return false;

    return std::find(availableRaces[cls].begin(), availableRaces[cls].end(), race) != availableRaces[cls].end();
}

uint8 RandomPlayerbotFactory::GetRandomClass()
{
    uint32 classProb[MAX_CLASSES] = { 0 };

    for (uint32 race = 1; race < MAX_RACES; ++race)
    {
        for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
        {
            classProb[cls] += sPlayerbotAIConfig.classRaceProbability[cls][race];
        }
    }

    uint32 randomProb = urand(0, sPlayerbotAIConfig.classRaceProbabilityTotal);

    for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
    {
        if (classProb[cls] > 0 && randomProb < classProb[cls])
            return cls;

        randomProb -= classProb[cls];
    }

    for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
    {
        if (classProb[cls] > 0)
            return cls;
    }

    return 1;
}

uint8 RandomPlayerbotFactory::GetRandomRace(uint8 cls)
{
    uint32 totalClassProb = 0;
    for (uint32 race = 1; race < MAX_RACES; ++race)
    {
        totalClassProb += sPlayerbotAIConfig.classRaceProbability[cls][race];
    }

    uint32 randomProb = urand(0, totalClassProb);

    for (uint32 race = 1; race < MAX_RACES; ++race)
    {
        if (sPlayerbotAIConfig.classRaceProbability[cls][race] > 0 && randomProb < sPlayerbotAIConfig.classRaceProbability[cls][race])
            return race;

        randomProb -= sPlayerbotAIConfig.classRaceProbability[cls][race];
    }

    return availableRaces[cls].front();
}

bool RandomPlayerbotFactory::CreateRandomBot(uint8 cls, std::unordered_map<NameRaceAndGender, std::vector<std::string>>& names, uint8 inputRace)
{
    sLog.outDebug( "Creating new random bot for class %d", cls);

    uint8 gender = rand() % 2 ? GENDER_MALE : GENDER_FEMALE;

    uint8 race = inputRace == 0 ? GetRandomRace(cls) : inputRace;

    NameRaceAndGender raceAndGender = CombineRaceAndGender(gender, race);

    std::string name;
    if(names.empty())
        name = CreateRandomBotName(raceAndGender);
    else
    {
        if (names[raceAndGender].empty())
            return false;
        uint32 i = urand(0, names[raceAndGender].size() - 1);
        name = names[raceAndGender][i];
        swap(names[raceAndGender][i], names[raceAndGender].back());
        names[raceAndGender].pop_back();
    }
    if (name.empty())
        return false;

    std::vector<uint8> skinColors, facialHairTypes;
    std::vector<std::pair<uint8,uint8>> faces, hairs;
    for (CharSectionsMap::const_iterator itr = sCharSectionMap.begin(); itr != sCharSectionMap.end(); ++itr)
    {
        CharSectionsEntry const* entry = itr->second;
        if (entry->Race != race || entry->Gender != gender)
            continue;

#ifndef MANGOSBOT_TWO
        switch (entry->BaseSection)
        {
        case SECTION_TYPE_SKIN:
            skinColors.push_back(entry->ColorIndex);
            break;
        case SECTION_TYPE_FACE:
            faces.push_back(std::pair<uint8,uint8>(entry->VariationIndex, entry->ColorIndex));
            break;
        case SECTION_TYPE_FACIAL_HAIR:
            facialHairTypes.push_back(entry->ColorIndex);
            break;
        case SECTION_TYPE_HAIR:
            hairs.push_back(std::pair<uint8,uint8>(entry->VariationIndex, entry->ColorIndex));
            break;
        }
#else
        switch (entry->BaseSection)
        {
        case SECTION_TYPE_SKIN:
            skinColors.push_back(entry->Color);
            break;
        case SECTION_TYPE_FACE:
            faces.push_back(std::pair<uint8, uint8>(entry->VariationIndex, entry->Color));
            break;
        case SECTION_TYPE_FACIAL_HAIR:
            facialHairTypes.push_back(entry->Color);
            break;
        case SECTION_TYPE_HAIR:
            hairs.push_back(std::pair<uint8, uint8>(entry->VariationIndex, entry->Color));
            break;
        }
#endif
    }

    uint8 skinColor = skinColors[urand(0, skinColors.size() - 1)];
    std::pair<uint8,uint8> face = faces[urand(0, faces.size() - 1)];
    std::pair<uint8,uint8> hair = hairs[urand(0, hairs.size() - 1)];

	bool excludeCheck = (race == RACE_TAUREN) || (gender == GENDER_FEMALE && race != RACE_NIGHTELF && race != RACE_UNDEAD);
#ifndef MANGOSBOT_TWO
	uint8 facialHair = excludeCheck ? 0 : facialHairTypes[urand(0, facialHairTypes.size() - 1)];
#else
	uint8 facialHair = 0;
#endif
	//TODO vector crash on cmangos TWO when creating one of the first bot characters, need a fix

	WorldSession* session = new WorldSession(accountId, NULL, SEC_PLAYER,

#ifdef MANGOSBOT_TWO
        2, 0, LOCALE_enUS, "", 0, 0, false);
#endif
#ifdef MANGOSBOT_ONE
		2, 0, LOCALE_enUS, "", 0, 0, false);
#endif
#ifdef MANGOSBOT_ZERO
        0, LOCALE_enUS, "", 0);
#endif

    session->SetNoAnticheat();

    Player* player = new Player(session);
    if (!player || !session)
    {
        sLog.outError("BOTS: Unable to create session or player for random acc %d - name: \"%s\"; race: %u; class: %u", accountId, name.c_str(), race, cls);
        return false;
    }
	if (!player->Create(sObjectMgr.GeneratePlayerLowGuid(), name, race, cls, gender,
	        face.second, // skinColor,
	        face.first,
	        hair.first,
	        hair.second, // hairColor,
	        facialHair, 0))
    {
        player->DeleteFromDB(player->GetObjectGuid(), accountId, true, true);
        delete session;
        delete player;
        sLog.outError("Unable to create random bot for account %d - name: \"%s\"; race: %u; class: %u",
                accountId, name.c_str(), race, cls);
        return false;
    }

    player->setCinematic(2);
    player->SetAtLoginFlag(AT_LOGIN_NONE);
    //player->SetSemaphoreTeleportFar(true); //Fake teleport to delay sql save
    //player->SaveToDB();
    //player->SetSemaphoreTeleportFar(false);

    sObjectAccessor.AddObject(player);

    sLog.outDebug( "Random bot created for account %d - name: \"%s\"; race: %u; class: %u",
            accountId, name.c_str(), race, cls);

    return true;
}

std::string RandomPlayerbotFactory::CreateRandomBotName(NameRaceAndGender raceAndGender)
{
    auto result = CharacterDatabase.Query("SELECT MAX(name_id) FROM ai_playerbot_names");
    if (!result)
    {
        sLog.outError("No more names left for random bots");
        return "";
    }

    Field *fields = result->Fetch();
    uint32 maxId = fields[0].GetUInt32();

    result = CharacterDatabase.PQuery("SELECT n.name FROM ai_playerbot_names n LEFT OUTER JOIN characters e ON e.name = n.name WHERE e.guid IS NULL and n.gender = '%u' order by rand() limit 1", static_cast<uint8>(raceAndGender));
    if (!result)
    {
        sLog.outError("No more names left for random bots");
        return "";
    }

	fields = result->Fetch();
    std::string bname = fields[0].GetString();
    return bname;
}

inline std::string GetNamePostFix(int32 nr)
{
    std::string ret;

    std::string str("abcdefghijklmnopqrstuvwxyz");

    while (nr >= 0)
    {
        int32 let = nr % 26;
        ret = str[let] + ret;
        nr /= 26;
        nr--;
    }

    return ret;
}

void RandomPlayerbotFactory::CreateRandomBots()
{
    // check if scheduled for delete
    bool delAccs = false;
    bool delFriends = false;
    auto values = CharacterDatabase.Query(
        "select value from ai_playerbot_random_bots where event = 'bot_delete'");

    if (values)
    {
        delAccs = true;

        Field* fields = values->Fetch();
        uint32 deleteType = fields[0].GetUInt32();

        if (deleteType > 1)
            delFriends = true;

    }

    if (sPlayerbotAIConfig.deleteRandomBotAccounts || delAccs)
    {
        std::list<uint32> botAccounts;
        std::list<uint32> botFriends;

        for (uint32 accountNumber = 0; accountNumber < sPlayerbotAIConfig.randomBotAccountCount; ++accountNumber)
        {
            std::ostringstream out; out << sPlayerbotAIConfig.randomBotAccountPrefix << accountNumber;
            std::string accountName = out.str();

            auto result = LoginDatabase.PQuery("SELECT id FROM account where username = '%s'", accountName.c_str());
            if (!result)
                continue;

            Field* fields = result->Fetch();
            uint32 accountId = fields[0].GetUInt32();

            botAccounts.push_back(accountId);
        }

        if (!delFriends)
            sLog.outString("Deleting random bot characters without friends/guild...");
        else
            sLog.outString("Deleting all random bot characters...");


        // load list of friends
        if (!delFriends)
        {
            auto result = CharacterDatabase.PQuery("SELECT friend FROM character_social WHERE flags='%u'", SOCIAL_FLAG_FRIEND);
            if (result)
            {
                do
                {
                    Field* fields = result->Fetch();
                    uint32 guidlo = fields[0].GetUInt32();
                    botFriends.push_back(guidlo);

                } while (result->NextRow());
            }
        }

        auto results = LoginDatabase.PQuery("SELECT id FROM account where username like '%s%%'", sPlayerbotAIConfig.randomBotAccountPrefix.c_str());
        if (results)
        {
            BarGoLink bar(results->GetRowCount());

            do
            {
                Field* fields = results->Fetch();
                uint32 accId = fields[0].GetUInt32();

                if (!delFriends)
                {
                    // existing characters list
                    auto result = CharacterDatabase.PQuery("SELECT guid FROM characters WHERE account='%u'", accId);
                    if (result)
                    {
                        do
                        {
                            Field* fields = result->Fetch();
                            uint32 guidlo = fields[0].GetUInt32();
                            ObjectGuid guid = ObjectGuid(HIGHGUID_PLAYER, guidlo);

                            // if bot is someone's friend - don't delete it
                            if ((find(botFriends.begin(), botFriends.end(), guidlo) != botFriends.end()) && !delFriends)
                                continue;

                            // if bot is in someone's guild - don't delete it
                            uint32 guildId = Player::GetGuildIdFromDB(guid);
                            if (guildId && !delFriends)
                            {
                                Guild* guild = sGuildMgr.GetGuildById(guildId);
                                uint32 accountId = sObjectMgr.GetPlayerAccountIdByGUID(guild->GetLeaderGuid());

                                if (find(botAccounts.begin(), botAccounts.end(), accountId) == botAccounts.end())
                                    continue;
                            }

                            sRandomPlayerbotMgr.OnPlayerLoginError(guidlo);
                            Player::DeleteFromDB(guid, accId, false, true);       // no need to update realm characters
                            //dels.push_back(std::async([guid, accId] {Player::DeleteFromDB(guid, accId, false, true); }));

                        } while (result->NextRow());
                    }
                    bar.step();
                }
                else
                {
                    bar.step();
                    sAccountMgr.DeleteAccount(accId);
                }

            } while (results->NextRow());
        }

        CharacterDatabase.Execute("DELETE FROM ai_playerbot_random_bots WHERE bot NOT IN (SELECT guid FROM characters)");
        sLog.outString("Random bot characters deleted");
    }

    if (!sPlayerbotAIConfig.randomBotAutoCreate)
    {
        for (uint32 accountNumber = 0; accountNumber < sPlayerbotAIConfig.randomBotAccountCount; ++accountNumber)
        {
            std::ostringstream out; out << sPlayerbotAIConfig.randomBotAccountPrefix << accountNumber;
            std::string accountName = out.str();

            auto results = LoginDatabase.PQuery("SELECT id FROM account where username = '%s'", accountName.c_str());
            if (!results)
                continue;

            Field* fields = results->Fetch();
            uint32 accountId = fields[0].GetUInt32();

            sPlayerbotAIConfig.randomBotAccounts.push_back(accountId);
        }

        return;
    }

    int totalAccCount = sPlayerbotAIConfig.randomBotAccountCount;
    sLog.outString("Creating random bot accounts...");

    std::vector<std::future<void>> account_creations;

    BarGoLink bar(totalAccCount);
    for (uint32 accountNumber = 0; accountNumber < sPlayerbotAIConfig.randomBotAccountCount; ++accountNumber)
    {
        std::ostringstream out; out << sPlayerbotAIConfig.randomBotAccountPrefix << accountNumber;
        std::string accountName = out.str();
        auto results = LoginDatabase.PQuery("SELECT id FROM account where username = '%s'", accountName.c_str());
        if (results)
        {
            continue;
        }

        std::string password = "";
        if (sPlayerbotAIConfig.randomBotRandomPassword)
        {
            for (int i = 0; i < 10; i++)
            {
                password += (char)urand('!', 'z');
            }
        }
        else
            password = accountName;

#ifndef MANGOSBOT_ZERO
        uint8 max_expansion = MAX_EXPANSION;
        account_creations.push_back(std::async([accountName, password, max_expansion] {sAccountMgr.CreateAccount(accountName, password, max_expansion); }));
#else
        account_creations.push_back(std::async([accountName, password] {sAccountMgr.CreateAccount(accountName, password); }));
#endif

        sLog.outDebug("Account %s created for random bots", accountName.c_str());
        bar.step();
    }

    BarGoLink bar3(account_creations.size());
    for (uint32 i = 0; i < account_creations.size(); i++)
    {
        bar3.step();
        account_creations[i].wait();
    }

    //LoginDatabase.PExecute("UPDATE account SET expansion = '%u' where username like '%s%%'", 2, sPlayerbotAIConfig.randomBotAccountPrefix.c_str());

    int totalRandomBotChars = 0;
    int totalCharCount = sPlayerbotAIConfig.randomBotAccountCount
#ifdef MANGOSBOT_TWO
        * 10;
#else
        * 9;
#endif

    sLog.outString("Loading available names...");

    std::unordered_map<NameRaceAndGender, std::vector<std::string>> freeNames, allNames;
    std::unordered_map<std::string, bool> used;

    auto result = CharacterDatabase.PQuery("SELECT n.gender, n.name, e.guid FROM ai_playerbot_names n LEFT OUTER JOIN characters e ON e.name = n.name");
    if (!result)
    {
        sLog.outError("No more names left for random bots");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        NameRaceAndGender raceAndGender = static_cast<NameRaceAndGender>(fields[0].GetUInt8());
        std::string bname = fields[1].GetString();
        uint32 guidlo = fields[2].GetUInt32();
        if (!guidlo)
            freeNames[raceAndGender].push_back(bname);
        allNames[raceAndGender].push_back(bname);
        used[bname] = false;
    } while (result->NextRow());

    if (allNames.count(NameRaceAndGender::DwarfMale) == 0)
    {
        sLog.outError("The name database has not been updated. Run ai_playerbot_names.sql to update.");

        auto oldResult = CharacterDatabase.PQuery("SELECT e.name FROM characters e");
        if (oldResult)
        {
            do
            {
                Field* fields = oldResult->Fetch();
                std::string bname = fields[0].GetString();
                used[bname] = false;
            } while (oldResult->NextRow());
        }

        for (uint8 type = 2; type <= static_cast<uint8>(NameRaceAndGender::BloodelfFemale); ++type)
        {
            for (auto name : allNames[static_cast<NameRaceAndGender>(type % 2)])
            {
                name[0] -= 'A' - 'a';
                name = GetNamePostFix(type - 2) + name;
                name[0] += 'A' - 'a';
                allNames[static_cast<NameRaceAndGender>(type)].push_back(name);
                if (used.count(name) == 0)
                {
                    freeNames[static_cast<NameRaceAndGender>(type)].push_back(name);
                }
            }
        }
    }

    BarGoLink bar4(static_cast<uint8>(NameRaceAndGender::BloodelfFemale));

    for (uint8 raceAndGenderIndex = 0; raceAndGenderIndex <= static_cast<uint8>(NameRaceAndGender::BloodelfFemale); ++raceAndGenderIndex)
    {
        const auto raceAndGender = static_cast<NameRaceAndGender>(raceAndGenderIndex);

        int32 postItt = 0;

        std::vector<std::string> newNames;

        // Dividing by 2, assuming equal distribution across the two factions (very conservative consideration of distribution across all races).
        // Given that there are 5k names per race and gender, this will trigger for account numbers exceeding 1000.
        if (totalCharCount / 2 < freeNames[raceAndGender].size())
            continue;
        uint32 namesNeeded = totalCharCount / 2 - freeNames[raceAndGender].size();

        while (namesNeeded)
        {
            std::string post = GetNamePostFix(postItt);

            for (auto name : allNames[raceAndGender])
            {
                if (name.size() + post.size() > 12)
                    continue;

                std::string newName = name + post;
                if (used.find(newName) != used.end())
                    continue;

                used[newName] = false;
                newNames.push_back(newName);
                namesNeeded--;
                if (!namesNeeded)
                    break;
            }

            postItt++;
        }

        bar4.step();

        freeNames[raceAndGender].insert(freeNames[raceAndGender].end(), newNames.begin(), newNames.end());
    }

    sLog.outString(">> Loaded names for " SIZEFMTD " race/gender combinations.", freeNames.size());

    sLog.outString("Creating random bot characters...");
    uint32 botsCreated = 0;
    BarGoLink bar1(totalCharCount);


    // Shallow copy of the fixed config so we can modify it
    std::map<std::pair<uint8, uint8>, uint32> remaining = sPlayerbotAIConfig.fixedClassRaceCounts;

    for (uint32 accountNumber = 0; accountNumber < sPlayerbotAIConfig.randomBotAccountCount; ++accountNumber)
    {
        std::ostringstream out; out << sPlayerbotAIConfig.randomBotAccountPrefix << accountNumber;
        std::string accountName = out.str();

        auto results = LoginDatabase.PQuery("SELECT id FROM account where username = '%s'", accountName.c_str());
        if (!results)
            continue;

        Field* fields = results->Fetch();
        uint32 accountId = fields[0].GetUInt32();

        sPlayerbotAIConfig.randomBotAccounts.push_back(accountId);

        int count = sAccountMgr.GetCharactersCount(accountId);
#ifdef MANGOSBOT_TWO
        if (count >= 10)
#else
        if (count >= 9)
#endif
        {
            totalRandomBotChars += count;
            continue;
        }

	RandomPlayerbotFactory factory(accountId);
	if (sPlayerbotAIConfig.useFixedClassRaceCounts)
	{
#ifdef MANGOSBOT_TWO
	    uint32 maxAllowed = 10 - count;
#else
	    uint32 maxAllowed = 9 - count;
#endif
	    uint32 created = 0;

	    while (!remaining.empty() && created < maxAllowed)
	    {
	        std::vector<std::pair<uint8, uint8>> shuffledKeys;
	        for (const auto& entry : remaining)
	            shuffledKeys.push_back(entry.first);

	        // Shuffle the keys of the map
	        std::random_device rnd;
		std::mt19937 rng(rnd()); // Mersenne Twister RNG
		std::shuffle(shuffledKeys.begin(), shuffledKeys.end(), rng);

	        for (const auto& key : shuffledKeys)
	        {
	            if (created >= maxAllowed)
	                break;

	            uint8 cls = key.first;
	            uint8 race = key.second;

	            if (!((1 << (cls - 1)) & CLASSMASK_ALL_PLAYABLE) || !sChrClassesStore.LookupEntry(cls))
	                continue;

#ifdef MANGOSBOT_TWO
	            if (cls == 10)
	                continue;
#else
	            if (cls == 10 || cls == 6)
	                continue;
#endif

	            if (factory.CreateRandomBot(cls, freeNames, race))
	            {
	                created++;
	                botsCreated++;
	                bar1.step();
	                if (--remaining[key] == 0)
	                    remaining.erase(key);
	            }
	        }
	    }
	}
	else
	{
            for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES - count; ++cls)
            {
                // skip nonexistent classes
                if (!((1 << (cls - 1)) & CLASSMASK_ALL_PLAYABLE) || !sChrClassesStore.LookupEntry(cls))
                    continue;

#ifdef MANGOSBOT_TWO
                if (cls != 10)
#else
                if (cls != 10 && cls != 6)
#endif
                {
                    uint8 rclss = factory.GetRandomClass();
                    botsCreated++;
                    factory.CreateRandomBot(rclss, freeNames);
                    bar1.step();
                }
            }
	}

        totalRandomBotChars += sAccountMgr.GetCharactersCount(accountId);
    }
    if (sPlayerbotAIConfig.useFixedClassRaceCounts && !remaining.empty())
    {
	sLog.outError("Unable to create all requested fixed class/race bots due to account character limits.");
	sLog.outError("The following class/race combination(s) were left uncreated:");

	uint32 totalCount = 0;
	for(const auto& entry : remaining)
	{
	    uint8 cls = entry.first.first;
	    uint8 race = entry.first.second;
	    uint32 count = entry.second;
	    totalCount += count;

	    sLog.outError(" - Class %u, Race %u: %u bots remaining", cls, race, count);
	}
#ifdef MANGOSBOT_TWO
	uint32 missingAccounts = (totalCount + 9) / 10;
#else
        uint32 missingAccounts = (totalCount + 8) / 9;
#endif
	sLog.outError("You need at least %u additional account(s) to fill the remaining fixed class/race combinations.", missingAccounts);
    }


    if (!botsCreated)
    {
	    sLog.outString("No new random bots needed. Accounts: %zu, bots: %d.", sPlayerbotAIConfig.randomBotAccounts.size(), totalRandomBotChars);

        return;
    }

    std::vector<std::future<void>> bot_creations;

    BarGoLink bar2(sObjectAccessor.GetPlayers().size());
    for (auto pl : sObjectAccessor.GetPlayers())
    {
        Player* player = pl.second;
        account_creations.push_back(std::async([player] {player->SaveToDB(); }));
    }

    for (uint32 i = 0; i < account_creations.size(); i++)
    {
        bar2.step();
        account_creations[i].wait();
    }

    std::vector<Player*> players;

    for (auto pl : sObjectAccessor.GetPlayers())
        players.push_back(pl.second);

    for (auto player : players)
    {
        WorldSession* session = player->GetSession();
        session->LogoutPlayer();
        sObjectAccessor.RemoveObject(player);
        delete player;
        delete session;
    }
    sLog.outString("%zu random bot accounts with %d characters available", sPlayerbotAIConfig.randomBotAccounts.size(), totalRandomBotChars+botsCreated);
}


void RandomPlayerbotFactory::CreateRandomGuilds()
{
    std::vector<uint32> randomBots;
    std::map<uint32, std::vector<uint32>> charAccGuids;

    auto charAccounts = CharacterDatabase.PQuery(
        "select `account`, `guid` from `characters`");

    if (charAccounts)
    {
        do
        {
            Field* fields = charAccounts->Fetch();
            uint32 accId = fields[0].GetUInt32();
            uint32 guid = fields[1].GetUInt32();
            charAccGuids[accId].push_back(guid);
        } while (charAccounts->NextRow());
    }

    if (charAccGuids.empty())
        return;

    for (auto charAcc : sPlayerbotAIConfig.randomBotAccounts)
    {
        if (!charAccGuids[charAcc].empty())
            for (auto charGuid : charAccGuids[charAcc])
                randomBots.push_back(charGuid);
    }

    if (randomBots.empty())
        return;

    if (sPlayerbotAIConfig.deleteRandomBotGuilds && !sRandomPlayerbotMgr.guildsDeleted)
    {
        sLog.outString("Deleting random bot guilds...");
        uint32 counter = 0;
        for (std::vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
        {
            ObjectGuid leader(HIGHGUID_PLAYER, *i);
            Guild* guild = sGuildMgr.GetGuildByLeader(leader);
            if (guild)
            {
                guild->Disband();
                counter++;
            }
        }
        sLog.outString("%d Random bot guilds deleted", counter);

        sRandomPlayerbotMgr.guildsDeleted = true;
    }

    if (!sPlayerbotAIConfig.randomBotGuildCount)
        return;

    uint32 guildNumber = 0;
    std::vector<ObjectGuid> availableLeaders;
    for (std::vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
    {
        ObjectGuid leader(HIGHGUID_PLAYER, *i);
        Guild* guild = sGuildMgr.GetGuildByLeader(leader);
        if (guild)
        {
            if (find(sPlayerbotAIConfig.randomBotGuilds.begin(), sPlayerbotAIConfig.randomBotGuilds.end(), guild->GetId()) == sPlayerbotAIConfig.randomBotGuilds.end())
            {
                ++guildNumber;
                sPlayerbotAIConfig.randomBotGuilds.push_back(guild->GetId());
            }
        }
        else
        {
            Player* player = sObjectMgr.GetPlayer(leader);
            if (player && !player->GetGuildId() && player->GetLevel() >= 10)
                availableLeaders.push_back(leader);
        }
    }

    if (availableLeaders.empty())
    {
        sLog.outError("No leaders for random guilds available");
        return;
    }

    uint32 attempts = 0;
    uint32 maxNewGuilds = sPlayerbotAIConfig.randomBotGuildCount - sPlayerbotAIConfig.randomBotGuilds.size();
    bool newGuilds = false;
    for (; guildNumber < maxNewGuilds; ++guildNumber)
    {
        attempts++;
        if (attempts > std::min(uint32(5), sPlayerbotAIConfig.randomBotGuildCount))
            break;
        if (sPlayerbotAIConfig.randomBotGuilds.size() >= sPlayerbotAIConfig.randomBotGuildCount)
            break;

        std::string guildName = CreateRandomGuildName();
        if (guildName.empty())
            continue;

        int index = urand(0, availableLeaders.size() - 1);
        ObjectGuid leader = availableLeaders[index];
        Player* player = sObjectMgr.GetPlayer(leader);
        if (!player || player->GetGuildId())
            continue;

        Guild* guild = new Guild();
        if (!guild->Create(player, guildName))
        {
            sLog.outError("Error creating random guild %s", guildName.c_str());
			continue;
        }

        sGuildMgr.AddGuild(guild);

        // create random emblem
        uint32 st, cl, br, bc, bg;
        bg = urand(0, 51);
        bc = urand(0, 17);
        cl = urand(0, 17);
        br = urand(0, 7);
        st = urand(0, 180);
        guild->SetEmblem(st, cl, br, bc, bg);
        guild->SetGINFO(std::to_string(urand(10, 30)));

        sPlayerbotAIConfig.randomBotGuilds.push_back(guild->GetId());
        sLog.outBasic("Random Guild <%s>, GM: %s", guildName.c_str(), player->GetName());
        newGuilds = true;
    }

    if (newGuilds)
        sLog.outString("Total Random Guilds: %d", (uint32)sPlayerbotAIConfig.randomBotGuilds.size());
}

std::string RandomPlayerbotFactory::CreateRandomGuildName()
{
    auto result = CharacterDatabase.Query("SELECT MAX(name_id) FROM ai_playerbot_guild_names");
    if (!result)
    {
        sLog.outError("No more names left for random guilds");
        return "";
    }

    Field *fields = result->Fetch();
    uint32 maxId = fields[0].GetUInt32();

    uint32 id = urand(0, maxId);
    result = CharacterDatabase.PQuery("SELECT n.name FROM ai_playerbot_guild_names n "
            "LEFT OUTER JOIN guild e ON e.name = n.name "
            "WHERE e.guildid IS NULL AND n.name_id >= '%u' LIMIT 1", id);
    if (!result)
    {
        sLog.outError("No more names left for random guilds");
        return "";
    }

    fields = result->Fetch();
    std::string gname = fields[0].GetString();
    return gname;
}

#ifndef MANGOSBOT_ZERO
void RandomPlayerbotFactory::CreateRandomArenaTeams()
{
    std::vector<uint32> randomBots;

    auto results = CharacterDatabase.PQuery(
        "select `bot` from ai_playerbot_random_bots where event = 'add'");

    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 bot = fields[0].GetUInt32();
            randomBots.push_back(bot);
        } while (results->NextRow());
    }

    if (sPlayerbotAIConfig.deleteRandomBotArenaTeams && !sRandomPlayerbotMgr.arenaTeamsDeleted)
    {
        sLog.outString("Deleting random bot arena teams...");
        for (std::vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
        {
            ObjectGuid captain(HIGHGUID_PLAYER, *i);
            ArenaTeam* arenateam = sObjectMgr.GetArenaTeamByCaptain(captain);
            if (arenateam)
                //sObjectMgr.RemoveArenaTeam(arenateam->GetId());
                arenateam->Disband(NULL);
        }
        sLog.outString("Random bot arena teams deleted");

        sRandomPlayerbotMgr.arenaTeamsDeleted = true;
    }

    uint32 arenaTeamNumber = 0;
    std::map<uint32, uint32> teamsNumber;
    std::map<uint32, uint32> maxTeamsNumber;
    maxTeamsNumber[ARENA_TYPE_2v2] = (uint32)(sPlayerbotAIConfig.randomBotArenaTeamCount * 0.4f);
    maxTeamsNumber[ARENA_TYPE_3v3] = (uint32)(sPlayerbotAIConfig.randomBotArenaTeamCount * 0.3f);
    maxTeamsNumber[ARENA_TYPE_5v5] = (uint32)(sPlayerbotAIConfig.randomBotArenaTeamCount * 0.3f);
    std::vector<ObjectGuid> availableCaptains;
    for (std::vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
    {
        ObjectGuid captain(HIGHGUID_PLAYER, *i);
        ArenaTeam* arenateam = sObjectMgr.GetArenaTeamByCaptain(captain);
        if (arenateam)
        {
            teamsNumber[arenateam->GetType()]++;
            sPlayerbotAIConfig.randomBotArenaTeams.push_back(arenateam->GetId());
        }

        Player* player = sObjectMgr.GetPlayer(captain);
        if (player)
        {
            if (player->GetLevel() < DEFAULT_MAX_LEVEL)
                continue;

            uint8 slot = ArenaTeam::GetSlotByType(ArenaType(ARENA_TYPE_2v2));
            if (player->GetArenaTeamId(slot))
                continue;

            slot = ArenaTeam::GetSlotByType(ArenaType(ARENA_TYPE_3v3));
            if (player->GetArenaTeamId(slot))
                continue;

            slot = ArenaTeam::GetSlotByType(ArenaType(ARENA_TYPE_5v5));
            if (player->GetArenaTeamId(slot))
                continue;

            availableCaptains.push_back(captain);
        }
    }

    uint32 attempts = 0;
    for (; arenaTeamNumber < sPlayerbotAIConfig.randomBotArenaTeamCount; ++arenaTeamNumber)
    {
        if (attempts > sPlayerbotAIConfig.randomBotArenaTeamCount)
            break;

        ArenaType randomType = ARENA_TYPE_2v2;
        switch (urand(0, 2))
        {
        case 0:
            randomType = ARENA_TYPE_2v2;
            break;
        case 1:
            randomType = ARENA_TYPE_3v3;
            break;
        case 2:
            randomType = ARENA_TYPE_5v5;
            break;
        }

        std::string arenaTeamName = CreateRandomArenaTeamName();
        if (arenaTeamName.empty())
            continue;

        if (availableCaptains.empty())
        {
            sLog.outError("No captains for random arena teams available");
            continue;
        }

        int index = urand(0, availableCaptains.size() - 1);
        ObjectGuid captain = availableCaptains[index];
        Player* player = sObjectMgr.GetPlayer(captain);
        if (!player)
        {
            sLog.outError("Cannot find player for captain %d", player->GetGUIDLow());
            continue;
        }

        if (player->GetLevel() < DEFAULT_MAX_LEVEL)
        {
            sLog.outError("Bot %d must be level %d to create an arena team", player->GetGUIDLow(), DEFAULT_MAX_LEVEL);
            continue;
        }

        auto results = CharacterDatabase.PQuery("SELECT `type` FROM ai_playerbot_arena_team_names WHERE name = '%s'", arenaTeamName.c_str());
        if (!results)
        {
            sLog.outError("No valid types for arena teams");
            return;
        }

        Field *fields = results->Fetch();
        uint8 slot = fields[0].GetUInt32();

        std::string arenaTypeName;
        ArenaType type = ARENA_TYPE_2v2;
        switch (slot)
        {
        case 2:
            type = ARENA_TYPE_2v2;
            arenaTypeName = "2v2";
            break;
        case 3:
            type = ARENA_TYPE_3v3;
            arenaTypeName = "3v3";
            break;
        case 5:
            type = ARENA_TYPE_5v5;
            arenaTypeName = "5v5";
            break;
        }

        attempts++;

        if (type != randomType)
            continue;

        if (teamsNumber[type] >= maxTeamsNumber[type])
            continue;

        if (player->GetArenaTeamId(ArenaTeam::GetSlotByType(type)))
            continue;

        ArenaTeam* arenateam = new ArenaTeam();
        if (!arenateam->Create(player->GetObjectGuid(), type, arenaTeamName))
        {
            sLog.outError("Error creating arena team %s", arenaTeamName.c_str());
            continue;
        }
        arenateam->SetCaptain(player->GetObjectGuid());
        sLog.outBasic("Bot #%d %s:%d <%s>: captain of random Arena %s team - %s", player->GetGUIDLow(), player->GetTeam() == ALLIANCE ? "A" : "H", player->GetLevel(), player->GetName(), arenaTypeName.c_str(), arenateam->GetName().c_str());
        // set random emblem
        uint32 backgroundColor = urand(0xFF000000, 0xFFFFFFFF), emblemStyle = urand(0, 101), emblemColor = urand(0xFF000000, 0xFFFFFFFF), borderStyle = urand(0, 5), borderColor = urand(0xFF000000, 0xFFFFFFFF);
        arenateam->SetEmblem(backgroundColor, emblemStyle, emblemColor, borderStyle, borderColor);
        // set random kills (wip)
        //arenateam->SetStats(STAT_TYPE_GAMES_WEEK, urand(0, 30));
        //arenateam->SetStats(STAT_TYPE_WINS_WEEK, urand(0, arenateam->GetStats().games_week));
        //arenateam->SetStats(STAT_TYPE_GAMES_SEASON, urand(arenateam->GetStats().games_week, arenateam->GetStats().games_week * 5));
        //arenateam->SetStats(STAT_TYPE_WINS_SEASON, urand(arenateam->GetStats().wins_week, arenateam->GetStats().games_season));
        sObjectMgr.AddArenaTeam(arenateam);
        sPlayerbotAIConfig.randomBotArenaTeams.push_back(arenateam->GetId());

        for (uint32 i = 0; i < 10; i++)
        {
            if (arenateam->GetMembersSize() >= type)
                break;

            int index = urand(0, availableCaptains.size() - 1);
            ObjectGuid possibleMember = availableCaptains[index];
            if (possibleMember == captain)
                continue;

            Player* member = sObjectMgr.GetPlayer(possibleMember);
            if (!member)
                continue;
            if (member->GetArenaTeamId(arenateam->GetSlot()))
                continue;
            if (member->GetTeam() != player->GetTeam())
                continue;

            arenateam->AddMember(member->GetObjectGuid());
            sLog.outBasic("Bot #%d %s:%d <%s>: added to random Arena %s team - %s", member->GetGUIDLow(), member->GetTeam() == ALLIANCE ? "A" : "H", member->GetLevel(), member->GetName(), arenaTypeName.c_str(), arenateam->GetName().c_str());

            /*if (player->GetArenaTeamIdFromDB(possibleMember, type))
                continue;*/

        }

        if (arenateam->GetMembersSize() < type)
        {
            sLog.outBasic("Random Arena team %s %s: failed to get enough members, deleting...", arenaTypeName.c_str(), arenateam->GetName().c_str());
            arenateam->Disband(nullptr);
            return;
        }

        // set random rating
        arenateam->SetRatingForAll(urand(1500, 2700));
        arenateam->SaveToDB();

        sLog.outBasic("Random Arena team %s %s: created", arenaTypeName.c_str(), arenateam->GetName().c_str());
    }

    sLog.outString("%d random bot arena teams available", arenaTeamNumber);
}

std::string RandomPlayerbotFactory::CreateRandomArenaTeamName()
{
    auto result = CharacterDatabase.Query("SELECT MAX(name_id) FROM ai_playerbot_arena_team_names");
    if (!result)
    {
        sLog.outError("No more names left for random arena teams");
        return "";
    }

    Field *fields = result->Fetch();
    uint32 maxId = fields[0].GetUInt32();

    uint32 id = urand(0, maxId);
    result = CharacterDatabase.PQuery("SELECT n.name FROM ai_playerbot_arena_team_names n "
        "LEFT OUTER JOIN arena_team e ON e.name = n.name "
        "WHERE e.arenateamid IS NULL AND n.name_id >= '%u' LIMIT 1", id);
    if (!result)
    {
        sLog.outError("No more names left for random arena teams");
        return "";
    }

    fields = result->Fetch();
    std::string aname = fields[0].GetString();
    return aname;
}
#endif

