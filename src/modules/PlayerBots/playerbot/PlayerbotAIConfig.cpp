
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/playerbot.h"
#include "RandomPlayerbotFactory.h"
#include "Accounts/AccountMgr.h"
#include "SystemConfig.h"
#include "playerbot/PlayerbotFactory.h"
#include "RandomItemMgr.h"
#include "World/WorldState.h"
#include "playerbot/PlayerbotHelpMgr.h"

#include "playerbot/TravelMgr.h"

#include <iostream>
#include <numeric>
#include <iomanip>
#include <boost/algorithm/string.hpp>
#include <regex>
#include "PlayerbotLoginMgr.h"

std::vector<std::string> ConfigAccess::GetValues(const std::string& name) const
{
    std::vector<std::string> values;
    auto const nameLower = boost::algorithm::to_lower_copy(name);
    for (auto entry : m_entries)
        if (entry.first.find(nameLower) != std::string::npos)
            values.push_back(entry.first);

    return values;
};

INSTANTIATE_SINGLETON_1(PlayerbotAIConfig);

PlayerbotAIConfig::PlayerbotAIConfig()
: enabled(false)
{
}

template <class T>
void LoadList(std::string value, T &list)
{
    list.clear();
    std::vector<std::string> ids = split(value, ',');
    for (std::vector<std::string>::iterator i = ids.begin(); i != ids.end(); i++)
    {
        std::string string = *i;
        if (string.empty())
            continue;

        uint32 id = atoi(string.c_str());

        list.push_back(id);
    }
}

template <class T>
void LoadListString(std::string value, T& list)
{
    list.clear();
    std::vector<std::string> strings = split(value, ',');
    for (std::vector<std::string>::iterator i = strings.begin(); i != strings.end(); i++)
    {
        std::string string = *i;
        if (string.empty())
            continue;

        list.push_back(string);
    }
}

inline ParsedUrl parseUrl(const std::string& url) {
    std::regex urlRegex(R"((http|https)://([^:/]+)(:([0-9]+))?(/.*)?)");
    std::smatch match;
    if (!std::regex_match(url, match, urlRegex)) {
        throw std::invalid_argument("Invalid URL format");
    }

    ParsedUrl parsed;
    parsed.hostname = match[2];
    parsed.https = match[1] == "https";
    parsed.port = parsed.https ? 443 : (match[4].length() ? std::stoi(match[4]) : 80);
    parsed.path = match[5].length() ? match[5] : std::string("/");
    return parsed;
}

bool PlayerbotAIConfig::Initialize()
{
    sLog.outString("Initializing AI Playerbot by ike3, based on the original Playerbot by blueboy");

    if (!config.SetSource(SYSCONFDIR"aiplayerbot.conf", "PlayerBots_"))
    {
        sLog.outString("AI Playerbot is Disabled. Unable to open configuration file aiplayerbot.conf");
        return false;
    }

    enabled = config.GetBoolDefault("AiPlayerbot.Enabled", false);
    if (!enabled)
    {
        sLog.outString("AI Playerbot is Disabled in aiplayerbot.conf");
        return false;
    }

    ConfigAccess* configA = reinterpret_cast<ConfigAccess*>(&config);

    BarGoLink::SetOutputState(config.GetBoolDefault("AiPlayerbot.ShowProgressBars", false));
    globalCoolDown = (uint32) config.GetIntDefault("AiPlayerbot.GlobalCooldown", 500);
    maxWaitForMove = config.GetIntDefault("AiPlayerbot.MaxWaitForMove", 3000);
    expireActionTime = config.GetIntDefault("AiPlayerbot.ExpireActionTime", 5000);
    dispelAuraDuration = config.GetIntDefault("AiPlayerbot.DispelAuraDuration", 2000);
    reactDelay = (uint32) config.GetIntDefault("AiPlayerbot.ReactDelay", 100);
    passiveDelay = (uint32) config.GetIntDefault("AiPlayerbot.PassiveDelay", 4000);
    repeatDelay = (uint32) config.GetIntDefault("AiPlayerbot.RepeatDelay", 5000);
    errorDelay = (uint32) config.GetIntDefault("AiPlayerbot.ErrorDelay", 5000);
    rpgDelay = (uint32) config.GetIntDefault("AiPlayerbot.RpgDelay", 3000);
    sitDelay = (uint32) config.GetIntDefault("AiPlayerbot.SitDelay", 30000);
    returnDelay = (uint32) config.GetIntDefault("AiPlayerbot.ReturnDelay", 7000);
    lootDelay = (uint32)config.GetIntDefault("AiPlayerbot.LootDelayDelay", 750);

    farDistance = config.GetFloatDefault("AiPlayerbot.FarDistance", 20.0f);
    sightDistance = config.GetFloatDefault("AiPlayerbot.SightDistance", 75.0f);
    spellDistance = config.GetFloatDefault("AiPlayerbot.SpellDistance", 25.0f);
    shootDistance = config.GetFloatDefault("AiPlayerbot.ShootDistance", 25.0f);
    healDistance = config.GetFloatDefault("AiPlayerbot.HealDistance", 125.0f);
    reactDistance = config.GetFloatDefault("AiPlayerbot.ReactDistance", 150.0f);
    maxFreeMoveDistance = config.GetFloatDefault("AiPlayerbot.MaxFreeMoveDistance", 150.0f);
    freeMoveDelay = config.GetFloatDefault("AiPlayerbot.FreeMoveDelay", 30.0f);
    grindDistance = config.GetFloatDefault("AiPlayerbot.GrindDistance", 75.0f);
    aggroDistance = config.GetFloatDefault("AiPlayerbot.AggroDistance", 22.0f);
    lootDistance = config.GetFloatDefault("AiPlayerbot.LootDistance", 25.0f);
    groupMemberLootDistance = config.GetFloatDefault("AiPlayerbot.GroupMemberLootDistance", 15.0f);
    groupMemberLootDistanceWithActiveMaster = config.GetFloatDefault("AiPlayerbot.GroupMemberLootDistanceWithActiveMaster", 10.0f);
    gatheringDistance = config.GetFloatDefault("AiPlayerbot.GatheringDistance", 15.0f);
    groupMemberGatheringDistance = config.GetFloatDefault("AiPlayerbot.GroupMemberGatheringDistance", 10.0f);
    groupMemberGatheringDistanceWithActiveMaster = config.GetFloatDefault("AiPlayerbot.GroupMemberGatheringDistanceWithActiveMaster", 5.0f);
    fleeDistance = config.GetFloatDefault("AiPlayerbot.FleeDistance", 8.0f);
    tooCloseDistance = config.GetFloatDefault("AiPlayerbot.TooCloseDistance", 5.0f);
    meleeDistance = config.GetFloatDefault("AiPlayerbot.MeleeDistance", 1.5f);
    followDistance = config.GetFloatDefault("AiPlayerbot.FollowDistance", 1.5f);
    raidFollowDistance = config.GetFloatDefault("AiPlayerbot.RaidFollowDistance", 5.0f);
    whisperDistance = config.GetFloatDefault("AiPlayerbot.WhisperDistance", 6000.0f);
    contactDistance = config.GetFloatDefault("AiPlayerbot.ContactDistance", 0.5f);
    aoeRadius = config.GetFloatDefault("AiPlayerbot.AoeRadius", 5.0f);
    rpgDistance = config.GetFloatDefault("AiPlayerbot.RpgDistance", 80.0f);
    proximityDistance = config.GetFloatDefault("AiPlayerbot.ProximityDistance", 20.0f);

    criticalHealth = config.GetIntDefault("AiPlayerbot.CriticalHealth", 20);
    lowHealth = config.GetIntDefault("AiPlayerbot.LowHealth", 50);
    mediumHealth = config.GetIntDefault("AiPlayerbot.MediumHealth", 70);
    almostFullHealth = config.GetIntDefault("AiPlayerbot.AlmostFullHealth", 90);
    lowMana = config.GetIntDefault("AiPlayerbot.LowMana", 15);
    mediumMana = config.GetIntDefault("AiPlayerbot.MediumMana", 40);

    randomGearMaxLevel = config.GetIntDefault("AiPlayerbot.RandomGearMaxLevel", 500);
    randomGearMaxDiff = config.GetIntDefault("AiPlayerbot.RandomGearMaxDiff", 9);
    randomGearUpgradeEnabled = config.GetBoolDefault("AiPlayerbot.RandomGearUpgradeEnabled", false);
    randomGearTabards = config.GetBoolDefault("AiPlayerbot.RandomGearTabards", false);
    randomGearTabardsChance = config.GetFloatDefault("AiPlayerbot.RandomGearTabardsChance", 0.1f);
    randomGearTabardsReplaceGuild = config.GetBoolDefault("AiPlayerbot.RandomGearTabardsReplaceGuild", false);
    randomGearTabardsUnobtainable = config.GetBoolDefault("AiPlayerbot.RandomGearTabardsUnobtainable", false);
    LoadList<std::list<uint32> >(config.GetStringDefault("AiPlayerbot.RandomGearBlacklist", ""), randomGearBlacklist);
    LoadList<std::list<uint32> >(config.GetStringDefault("AiPlayerbot.RandomGearWhitelist", ""), randomGearWhitelist);
    randomGearProgression = config.GetBoolDefault("AiPlayerbot.RandomGearProgression", true);
    randomGearLoweringChance = config.GetFloatDefault("AiPlayerbot.RandomGearLoweringChance", 0.15f);
    randomBotMaxLevelChance = config.GetFloatDefault("AiPlayerbot.RandomBotMaxLevelChance", 0.15f);
    randomBotRpgChance = config.GetFloatDefault("AiPlayerbot.RandomBotRpgChance", 0.35f);
    usePotionChance = config.GetFloatDefault("AiPlayerbot.UsePotionChance", 1.0f);
    attackEmoteChance = config.GetFloatDefault("AiPlayerbot.AttackEmoteChance", 0.0f);

    jumpNoCombatChance = config.GetFloatDefault("AiPlayerbot.JumpNoCombatChance", 0.5f);
    jumpMeleeInCombatChance = config.GetFloatDefault("AiPlayerbot.JumpMeleeInCombatChance", 0.5f);
    jumpRandomChance = config.GetFloatDefault("AiPlayerbot.JumpRandomChance", 0.20f);
    jumpInPlaceChance = config.GetFloatDefault("AiPlayerbot.JumpInPlaceChance", 0.50f);
    jumpBackwardChance = config.GetFloatDefault("AiPlayerbot.JumpBackwardChance", 0.10f);
    jumpHeightLimit = config.GetFloatDefault("AiPlayerbot.JumpHeightLimit", 60.f);
    jumpVSpeed = config.GetFloatDefault("AiPlayerbot.JumpVSpeed", 7.96f);
    jumpHSpeed = config.GetFloatDefault("AiPlayerbot.JumpHSpeed", 7.0f);
    jumpInBg = config.GetBoolDefault("AiPlayerbot.JumpInBg", false);
    jumpWithPlayer = config.GetBoolDefault("AiPlayerbot.JumpWithPlayer", false);
    jumpFollow = config.GetBoolDefault("AiPlayerbot.JumpFollow", true);
    jumpChase = config.GetBoolDefault("AiPlayerbot.JumpChase", true);
    useKnockback = config.GetBoolDefault("AiPlayerbot.UseKnockback", true);

    iterationsPerTick = config.GetIntDefault("AiPlayerbot.IterationsPerTick", 100);

    allowGuildBots = config.GetBoolDefault("AiPlayerbot.AllowGuildBots", true);
    allowMultiAccountAltBots = config.GetBoolDefault("AiPlayerbot.AllowMultiAccountAltBots", true);

    randomBotMapsAsString = config.GetStringDefault("AiPlayerbot.RandomBotMaps", "0,1,530,571");
    LoadList<std::vector<uint32> >(randomBotMapsAsString, randomBotMaps);
    LoadList<std::list<uint32> >(config.GetStringDefault("AiPlayerbot.RandomBotQuestItems", "6948,5175,5176,5177,5178,16309,12382,13704,11000,22754"), randomBotQuestItems);
    LoadList<std::list<uint32> >(config.GetStringDefault("AiPlayerbot.RandomBotSpellIds", "54197"), randomBotSpellIds);
	LoadList<std::list<uint32> >(config.GetStringDefault("AiPlayerbot.PvpProhibitedZoneIds", "2255,656,2361,2362,2363,976,35,2268,3425,392,541,1446,3828,3712,3738,3565,3539,3623,4152,3988,4658,4284,4418,4436,4275,4323"), pvpProhibitedZoneIds);

#ifndef MANGOSBOT_ZERO
    // disable pvp near dark portal if event is active
    if (sWorldState.GetExpansion() == EXPANSION_NONE)
        pvpProhibitedZoneIds.insert(pvpProhibitedZoneIds.begin(), 72);
#endif

    LoadList<std::list<uint32> >(config.GetStringDefault("AiPlayerbot.RandomBotQuestIds", "7848,3802,5505,6502,7761,9378"), randomBotQuestIds);
    LoadList<std::list<uint32> >(config.GetStringDefault("AiPlayerbot.ImmuneSpellIds", ""), immuneSpellIds);

    botAutologin = BotAutoLogin(config.GetIntDefault("AiPlayerbot.BotAutologin", 0));
    randomBotAutologin = config.GetBoolDefault("AiPlayerbot.RandomBotAutologin", true);
    randomBotAutoCreate = config.GetBoolDefault("AiPlayerbot.RandomBotAutoCreate", true);
    minRandomBots = config.GetIntDefault("AiPlayerbot.MinRandomBots", 50);
    maxRandomBots = config.GetIntDefault("AiPlayerbot.MaxRandomBots", 200);
    randomBotUpdateInterval = config.GetIntDefault("AiPlayerbot.RandomBotUpdateInterval", 1);
    randomBotCountChangeMinInterval = config.GetIntDefault("AiPlayerbot.RandomBotCountChangeMinInterval", 1 * 1800);
    randomBotCountChangeMaxInterval = config.GetIntDefault("AiPlayerbot.RandomBotCountChangeMaxInterval", 2 * 3600);
    loginBoostPercentage = config.GetFloatDefault("AiPlayerbot.LoginBoostPercentage", 90);
    randomBotTimedLogout = config.GetBoolDefault("AiPlayerbot.RandomBotTimedLogout", true);
    randomBotTimedOffline = config.GetBoolDefault("AiPlayerbot.RandomBotTimedOffline", false);
    minRandomBotInWorldTime = config.GetIntDefault("AiPlayerbot.MinRandomBotInWorldTime", 1 * 1800);
    maxRandomBotInWorldTime = config.GetIntDefault("AiPlayerbot.MaxRandomBotInWorldTime", 6 * 3600);
    minRandomBotRandomizeTime = config.GetIntDefault("AiPlayerbot.MinRandomBotRandomizeTime", 6 * 3600);
    maxRandomBotRandomizeTime = config.GetIntDefault("AiPlayerbot.MaxRandomRandomizeTime", 24 * 3600);
    minRandomBotChangeStrategyTime = config.GetIntDefault("AiPlayerbot.MinRandomBotChangeStrategyTime", 1800);
    maxRandomBotChangeStrategyTime = config.GetIntDefault("AiPlayerbot.MaxRandomBotChangeStrategyTime", 2 * 3600);
    minRandomBotReviveTime = config.GetIntDefault("AiPlayerbot.MinRandomBotReviveTime", 60);
    maxRandomBotReviveTime = config.GetIntDefault("AiPlayerbot.MaxRandomReviveTime", 300);
    enableRandomTeleports = config.GetBoolDefault("AiPlayerbot.EnableRandomTeleports", true);
    randomBotTeleportDistance = config.GetIntDefault("AiPlayerbot.RandomBotTeleportDistance", 1000);
    randomBotTeleportNearPlayer = config.GetBoolDefault("AiPlayerbot.RandomBotTeleportNearPlayer", false);
    randomBotTeleportNearPlayerMaxAmount = config.GetIntDefault("AiPlayerbot.RandomBotTeleportNearPlayerMaxAmount", 0);
    randomBotTeleportNearPlayerMaxAmountRadius = config.GetFloatDefault("AiPlayerbot.RandomBotTeleportNearPlayerMaxAmountRadius", 0.0f);
    randomBotTeleportMinInterval = config.GetIntDefault("AiPlayerbot.RandomBotTeleportTeleportMinInterval", 2 * 3600);
    randomBotTeleportMaxInterval = config.GetIntDefault("AiPlayerbot.RandomBotTeleportTeleportMaxInterval", 48 * 3600);
    randomBotsPerInterval = config.GetIntDefault("AiPlayerbot.RandomBotsPerInterval", 3);
    randomBotsMaxLoginsPerInterval = config.GetIntDefault("AiPlayerbot.RandomBotsMaxLoginsPerInterval", randomBotsPerInterval);
    minRandomBotsPriceChangeInterval = config.GetIntDefault("AiPlayerbot.MinRandomBotsPriceChangeInterval", 2 * 3600);
    maxRandomBotsPriceChangeInterval = config.GetIntDefault("AiPlayerbot.MaxRandomBotsPriceChangeInterval", 48 * 3600);
    //Auction house settings
    shouldQueryAHListingsOutsideOfAH = config.GetBoolDefault("AiPlayerbot.ShouldQueryAHListingsOutsideOfAH", true);
    LoadList<std::list<uint32> >(config.GetStringDefault("AiPlayerbot.AhOverVendorItemIds", ""), ahOverVendorItemIds);
    LoadList<std::list<uint32> >(config.GetStringDefault("AiPlayerbot.VendorOverAHItemIds", ""), vendorOverAHItemIds);
    botCheckAllAuctionListings = config.GetBoolDefault("AiPlayerbot.BotCheckAllAuctionListings", false);
    //
    randomBotJoinLfg = config.GetBoolDefault("AiPlayerbot.RandomBotJoinLfg", true);
    logRandomBotJoinLfg = config.GetBoolDefault("AiPlayerbot.LogRandomBotJoinLfg", false);
    randomBotJoinBG = config.GetBoolDefault("AiPlayerbot.RandomBotJoinBG", true);
    randomBotAutoJoinBG = config.GetBoolDefault("AiPlayerbot.RandomBotAutoJoinBG", false);
    randomBotBracketCount = config.GetIntDefault("AiPlayerbot.RandomBotBracketCount", 3);
    logInGroupOnly = config.GetBoolDefault("AiPlayerbot.LogInGroupOnly", true);
    logValuesPerTick = config.GetBoolDefault("AiPlayerbot.LogValuesPerTick", false);
    fleeingEnabled = config.GetBoolDefault("AiPlayerbot.FleeingEnabled", true);
    summonAtInnkeepersEnabled = config.GetBoolDefault("AiPlayerbot.SummonAtInnkeepersEnabled", true);
    randomBotMinLevel = config.GetIntDefault("AiPlayerbot.RandomBotMinLevel", 1);
    randomBotMaxLevel = config.GetIntDefault("AiPlayerbot.RandomBotMaxLevel", 255);
    randomBotLoginAtStartup = config.GetBoolDefault("AiPlayerbot.RandomBotLoginAtStartup", true);
    randomBotTeleLevel = config.GetIntDefault("AiPlayerbot.RandomBotTeleLevel", 5);
    openGoSpell = config.GetIntDefault("AiPlayerbot.OpenGoSpell", 6477);

    randomChangeMultiplier = config.GetFloatDefault("AiPlayerbot.RandomChangeMultiplier", 1.0);

    randomBotCombatStrategies = config.GetStringDefault("AiPlayerbot.RandomBotCombatStrategies", "-threat,+custom::say");
    randomBotNonCombatStrategies = config.GetStringDefault("AiPlayerbot.RandomBotNonCombatStrategies", "+custom::say");
    randomBotReactStrategies = config.GetStringDefault("AiPlayerbot.RandomBotReactStrategies", "");
    randomBotDeadStrategies = config.GetStringDefault("AiPlayerbot.RandomBotDeadStrategies", "");
    combatStrategies = config.GetStringDefault("AiPlayerbot.CombatStrategies", "");
    nonCombatStrategies = config.GetStringDefault("AiPlayerbot.NonCombatStrategies", "+return,+delayed roll");
    reactStrategies = config.GetStringDefault("AiPlayerbot.ReactStrategies", "");
    deadStrategies = config.GetStringDefault("AiPlayerbot.DeadStrategies", "");

    commandPrefix = config.GetStringDefault("AiPlayerbot.CommandPrefix", "");
    commandSeparator = config.GetStringDefault("AiPlayerbot.CommandSeparator", "\\\\");

    commandServerPort = config.GetIntDefault("AiPlayerbot.CommandServerPort", 0);
    perfMonEnabled = config.GetBoolDefault("AiPlayerbot.PerfMonEnabled", false);
    bExplicitDbStoreSave = config.GetBoolDefault("AiPlayerbot.ExplicitDbStoreSave", false);

    randomBotLoginWithPlayer = config.GetBoolDefault("AiPlayerbot.RandomBotLoginWithPlayer", false);
    asyncBotLogin = config.GetBoolDefault("AiPlayerbot.AsyncBotLogin", false);
    preloadHolders = config.GetBoolDefault("AiPlayerbot.PreloadHolders", false);
    
    freeRoomForNonSpareBots = config.GetIntDefault("AiPlayerbot.FreeRoomForNonSpareBots", 1);

    loginBotsNearPlayerRange = config.GetIntDefault("AiPlayerbot.LoginBotsNearPlayerRange", 1000);
    
    LoadListString<std::vector<std::string> >(config.GetStringDefault("AiPlayerbot.DefaultLoginCriteria", "maxbots,spareroom,offline"), defaultLoginCriteria);

    std::vector<std::string> criteriaValues = configA->GetValues("AiPlayerbot.LoginCriteria");
    std::sort(criteriaValues.begin(), criteriaValues.end());
    loginCriteria.clear();
    for (auto& value : criteriaValues)
    {
        loginCriteria.push_back({});
        LoadListString<std::vector<std::string> >(config.GetStringDefault(value, ""), loginCriteria.back());
    }

    if (criteriaValues.empty())
    {
        loginCriteria.push_back({ "group" });
        loginCriteria.push_back({ "arena" });
        loginCriteria.push_back({ "bg" });
        loginCriteria.push_back({ "guild" });
        loginCriteria.push_back({ "logoff,classrace,level,online" });
        loginCriteria.push_back({ "logoff,classrace,level" });
        loginCriteria.push_back({ "logoff,classrace" });
    }
    

    for (uint32 level = 1; level <= DEFAULT_MAX_LEVEL; ++level)
    {
        levelProbability[level] = config.GetIntDefault("AiPlayerbot.LevelProbability." + std::to_string(level), 100);
    }

    sLog.outString("Loading Race/Class probabilities");

    classRaceProbabilityTotal = 0;

    useFixedClassRaceCounts = config.GetBoolDefault("AiPlayerbot.ClassRace.UseFixedClassRaceCounts", false);
    RandomPlayerbotFactory factory(0);

    for (uint32 race = 1; race < MAX_RACES; ++race)
    {
        //Set race defaults
        if (race > 0)
        {
            int rProb = config.GetIntDefault("AiPlayerbot.ClassRaceProb.0." + std::to_string(race), 100);

            for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
            {
                classRaceProbability[cls][race] = rProb;
            }
        }
    }

    //Class overrides
    for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
    {
        int cProb = config.GetIntDefault("AiPlayerbot.ClassRaceProb." + std::to_string(cls), -1);

        if (cProb >= 0)
        {
            for (uint32 race = 1; race < MAX_RACES; ++race)
            {
                classRaceProbability[cls][race] = cProb;
            }
        }
    }

    //Race Class overrides
    for (uint32 race = 1; race < MAX_RACES; ++race)
    {
        for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
        {
            int rcProb = config.GetIntDefault("AiPlayerbot.ClassRaceProb." + std::to_string(cls) + "." + std::to_string(race), -1);
            if (rcProb >= 0)
                classRaceProbability[cls][race] = rcProb;

            if (!factory.isAvailableRace(cls, race))
                classRaceProbability[cls][race] = 0;
            else
                classRaceProbabilityTotal += classRaceProbability[cls][race];
        }
    }

    if (useFixedClassRaceCounts)
    {

        // Warn about unsupported config keys
        for (uint32 race = 1; race < MAX_RACES; ++race)
        {
            std::string raceKey = "AiPlayerbot.ClassRaceProb.0." + std::to_string(race);
            int val = config.GetIntDefault(raceKey.c_str(), -1);
            if (val >= 0)
                sLog.outError("Fixed class/race counts does not yet support '%s' (race-only). This config entry will be ignored.", raceKey.c_str());
        }

        for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
        {
            std::string classKey = "AiPlayerbot.ClassRaceProb." + std::to_string(cls);
            int val = config.GetIntDefault(classKey.c_str(), -1);
            if (val >= 0)
                sLog.outError("Fixed class/race counts does not yet support '%s' (class-only). This config entry will be ignored.", classKey.c_str());
        }

        //Parse and build fixedClassRacesCounts
        {
            for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
	    {
	        for (uint32 race = 1; race < MAX_RACES; ++race)
	        {
		    std::string key = "AiPlayerbot.ClassRaceProb." + std::to_string(cls) + "." + std::to_string(race);
		    int count = config.GetIntDefault(key, -1);

		    if (count >= 0 && factory.isAvailableRace(cls, race))
		    {
		        fixedClassRaceCounts[{cls, race}] = count;
		    }
	        }
	    }
        }
    }

    botCheats.clear();
    LoadListString<std::list<std::string>>(config.GetStringDefault("AiPlayerbot.BotCheats", "taxi,item,breath"), botCheats);

    botCheatMask = 0;

    if (std::find(botCheats.begin(), botCheats.end(), "taxi") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::taxi;
    if (std::find(botCheats.begin(), botCheats.end(), "gold") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::gold;
    if (std::find(botCheats.begin(), botCheats.end(), "health") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::health;
    if (std::find(botCheats.begin(), botCheats.end(), "mana") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::mana;
    if (std::find(botCheats.begin(), botCheats.end(), "power") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::power;
    if (std::find(botCheats.begin(), botCheats.end(), "item") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::item;
    if (std::find(botCheats.begin(), botCheats.end(), "cooldown") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::cooldown;
    if (std::find(botCheats.begin(), botCheats.end(), "repair") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::repair;
    if (std::find(botCheats.begin(), botCheats.end(), "movespeed") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::movespeed;
    if (std::find(botCheats.begin(), botCheats.end(), "attackspeed") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::attackspeed;
    if (std::find(botCheats.begin(), botCheats.end(), "breath") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::breath;

    LoadListString<std::list<std::string>>(config.GetStringDefault("AiPlayerbot.AllowedLogFiles", ""), allowedLogFiles);
    LoadListString<std::list<std::string>>(config.GetStringDefault("AiPlayerbot.DebugFilter", "add gathering loot,check values,emote,check mount state,jump"), debugFilter);

    worldBuffs.clear();

    //Get all config values starting with AiPlayerbot.WorldBuff
    std::vector<std::string> values = configA->GetValues("AiPlayerbot.WorldBuff");

    if (values.size())
    {
        sLog.outString("Loading WorldBuffs");
        BarGoLink wbuffBar(values.size());

        for (auto value : values)
        {
            std::vector<std::string> ids = split(value, '.');
            std::vector<uint32> params = { 0,0,0,0,0,0 };

            //Extract faction, class, spec, minlevel, maxlevel
            for (uint8 i = 0; i < 6; i++)
                if (ids.size() > i + 2)
                    params[i] = stoi(ids[i + 2]);

            //Get list of buffs for this combination.
            std::list<uint32> buffs;
            LoadList<std::list<uint32>>(config.GetStringDefault(value, ""), buffs);

            //Store buffs for later application.
            for (auto buff : buffs)
            {
                worldBuff wb = { buff, params[0], params[1], params[2], params[3], params[4], params[5] };
                worldBuffs.push_back(wb);
            }

            wbuffBar.step();
        }
    }

    randomBotAccountPrefix = config.GetStringDefault("AiPlayerbot.RandomBotAccountPrefix", "rndbot");
    randomBotAccountCount = config.GetIntDefault("AiPlayerbot.RandomBotAccountCount", 50);
    deleteRandomBotAccounts = config.GetBoolDefault("AiPlayerbot.DeleteRandomBotAccounts", false);
    randomBotGuildCount = config.GetIntDefault("AiPlayerbot.RandomBotGuildCount", 20);
    deleteRandomBotGuilds = config.GetBoolDefault("AiPlayerbot.DeleteRandomBotGuilds", false);

    //arena
    randomBotArenaTeamCount = config.GetIntDefault("AiPlayerbot.RandomBotArenaTeamCount", 20);
    deleteRandomBotArenaTeams = config.GetBoolDefault("AiPlayerbot.DeleteRandomBotArenaTeams", false);

    //cosmetics (by lidocain)
    randomBotShowCloak = config.GetBoolDefault("AiPlayerbot.RandomBotShowCloak", false);
    randomBotShowHelmet = config.GetBoolDefault("AiPlayerbot.RandomBotShowHelmet", false);

	//SPP switches
    enableGreet = config.GetBoolDefault("AiPlayerbot.EnableGreet", false);
	disableRandomLevels = config.GetBoolDefault("AiPlayerbot.DisableRandomLevels", false);
    instantRandomize = config.GetBoolDefault("AiPlayerbot.InstantRandomize", true);
    randomBotRandomPassword = config.GetBoolDefault("AiPlayerbot.RandomBotRandomPassword", true);
    playerbotsXPrate = config.GetFloatDefault("AiPlayerbot.XPRate", 1.0f);
    disableBotOptimizations = config.GetBoolDefault("AiPlayerbot.DisableBotOptimizations", false);
    disableActivityPriorities = config.GetBoolDefault("AiPlayerbot.DisableActivityPriorities", false);
    botActiveAlone = config.GetIntDefault("AiPlayerbot.botActiveAlone", 10);
    diffWithPlayer = config.GetIntDefault("AiPlayerbot.DiffWithPlayer", 100);
    diffEmpty = config.GetIntDefault("AiPlayerbot.DiffEmpty", 200);
    RandombotsWalkingRPG = config.GetBoolDefault("AiPlayerbot.RandombotsWalkingRPG", false);
    RandombotsWalkingRPGInDoors = config.GetBoolDefault("AiPlayerbot.RandombotsWalkingRPG.InDoors", false);
    minEnchantingBotLevel = config.GetIntDefault("AiPlayerbot.minEnchantingBotLevel", 60);
    randombotStartingLevel = config.GetIntDefault("AiPlayerbot.randombotStartingLevel", 5);
    gearscorecheck = config.GetBoolDefault("AiPlayerbot.GearScoreCheck", false);
    levelCheck = config.GetIntDefault("AiPlayerbot.LevelCheck", 30);
	randomBotPreQuests = config.GetBoolDefault("AiPlayerbot.PreQuests", true);
    randomBotSayWithoutMaster = config.GetBoolDefault("AiPlayerbot.RandomBotSayWithoutMaster", false);
    randomBotInvitePlayer = config.GetBoolDefault("AiPlayerbot.RandomBotInvitePlayer", true);
    randomBotGroupNearby = config.GetBoolDefault("AiPlayerbot.RandomBotGroupNearby", true);
    randomBotRaidNearby = config.GetBoolDefault("AiPlayerbot.RandomBotRaidNearby", true);
    randomBotGuildNearby = config.GetBoolDefault("AiPlayerbot.RandomBotGuildNearby", true);
    inviteChat = config.GetBoolDefault("AiPlayerbot.InviteChat", true);

    guildMaxBotLimit = config.GetIntDefault("AiPlayerbot.GuildMaxBotLimit", 1000);

    ////////////////////////////
    enableBroadcasts = config.GetBoolDefault("AiPlayerbot.EnableBroadcasts", true);

    //broadcastChanceMaxValue is used in urand(1, broadcastChanceMaxValue) for broadcasts,
    //lowering it will increase the chance, setting it to 0 will disable broadcasts
    //for internal use, not intended to be change by the user
    broadcastChanceMaxValue = enableBroadcasts ? 30000 : 0;

    //all broadcast chances should be in range 1-broadcastChanceMaxValue, value of 0 will disable this particular broadcast
    //setting value to max does not guarantee the broadcast, as there are some internal randoms as well
    broadcastToGuildGlobalChance = config.GetIntDefault("AiPlayerbot.BroadcastToGuildGlobalChance", 30000);
    broadcastToWorldGlobalChance = config.GetIntDefault("AiPlayerbot.BroadcastToWorldGlobalChance", 30000);
    broadcastToGeneralGlobalChance = config.GetIntDefault("AiPlayerbot.BroadcastToGeneralGlobalChance", 30000);
    broadcastToTradeGlobalChance = config.GetIntDefault("AiPlayerbot.BroadcastToTradeGlobalChance", 30000);
    broadcastToLFGGlobalChance = config.GetIntDefault("AiPlayerbot.BroadcastToLFGGlobalChance", 30000);
    broadcastToLocalDefenseGlobalChance = config.GetIntDefault("AiPlayerbot.BroadcastToLocalDefenseGlobalChance", 30000);
    broadcastToWorldDefenseGlobalChance = config.GetIntDefault("AiPlayerbot.BroadcastToWorldDefenseGlobalChance", 30000);
    broadcastToGuildRecruitmentGlobalChance = config.GetIntDefault("AiPlayerbot.BroadcastToGuildRecruitmentGlobalChance", 30000);
    broadcastToSayGlobalChance = config.GetIntDefault("AiPlayerbot.BroadcastToSayGlobalChance", 30000);
    broadcastToYellGlobalChance = config.GetIntDefault("AiPlayerbot.BroadcastToYellGlobalChance", 30000);

    broadcastChanceLootingItemPoor = config.GetIntDefault("AiPlayerbot.BroadcastChanceLootingItemPoor", 30);
    broadcastChanceLootingItemNormal = config.GetIntDefault("AiPlayerbot.BroadcastChanceLootingItemNormal", 300);
    broadcastChanceLootingItemUncommon = config.GetIntDefault("AiPlayerbot.BroadcastChanceLootingItemUncommon", 10000);
    broadcastChanceLootingItemRare = config.GetIntDefault("AiPlayerbot.BroadcastChanceLootingItemRare", 20000);
    broadcastChanceLootingItemEpic = config.GetIntDefault("AiPlayerbot.BroadcastChanceLootingItemEpic", 30000);
    broadcastChanceLootingItemLegendary = config.GetIntDefault("AiPlayerbot.BroadcastChanceLootingItemLegendary", 30000);
    broadcastChanceLootingItemArtifact = config.GetIntDefault("AiPlayerbot.BroadcastChanceLootingItemArtifact", 30000);

    broadcastChanceQuestAccepted = config.GetIntDefault("AiPlayerbot.BroadcastChanceQuestAccepted", 6000);
    broadcastChanceQuestUpdateObjectiveCompleted = config.GetIntDefault("AiPlayerbot.BroadcastChanceQuestUpdateObjectiveCompleted", 300);
    broadcastChanceQuestUpdateObjectiveProgress = config.GetIntDefault("AiPlayerbot.BroadcastChanceQuestUpdateObjectiveProgress", 300);
    broadcastChanceQuestUpdateFailedTimer = config.GetIntDefault("AiPlayerbot.BroadcastChanceQuestUpdateFailedTimer", 300);
    broadcastChanceQuestUpdateComplete = config.GetIntDefault("AiPlayerbot.BroadcastChanceQuestUpdateComplete", 1000);
    broadcastChanceQuestTurnedIn = config.GetIntDefault("AiPlayerbot.BroadcastChanceQuestTurnedIn", 10000);

    broadcastChanceKillNormal = config.GetIntDefault("AiPlayerbot.BroadcastChanceKillNormal", 30);
    broadcastChanceKillElite = config.GetIntDefault("AiPlayerbot.BroadcastChanceKillElite", 300);
    broadcastChanceKillRareelite = config.GetIntDefault("AiPlayerbot.BroadcastChanceKillRareelite", 3000);
    broadcastChanceKillWorldboss = config.GetIntDefault("AiPlayerbot.BroadcastChanceKillWorldboss", 20000);
    broadcastChanceKillRare = config.GetIntDefault("AiPlayerbot.BroadcastChanceKillRare", 10000);
    broadcastChanceKillUnknown = config.GetIntDefault("AiPlayerbot.BroadcastChanceKillUnknown", 100);
    broadcastChanceKillPet = config.GetIntDefault("AiPlayerbot.BroadcastChanceKillPet", 10);
    broadcastChanceKillPlayer = config.GetIntDefault("AiPlayerbot.BroadcastChanceKillPlayer", 30);

    broadcastChanceLevelupGeneric = config.GetIntDefault("AiPlayerbot.BroadcastChanceLevelupGeneric", 20000);
    broadcastChanceLevelupTenX = config.GetIntDefault("AiPlayerbot.BroadcastChanceLevelupTenX", 30000);
    broadcastChanceLevelupMaxLevel = config.GetIntDefault("AiPlayerbot.BroadcastChanceLevelupMaxLevel", 30000);

    broadcastChanceSuggestInstance = config.GetIntDefault("AiPlayerbot.BroadcastChanceSuggestInstance", 5000);
    broadcastChanceSuggestQuest = config.GetIntDefault("AiPlayerbot.BroadcastChanceSuggestQuest", 10000);
    broadcastChanceSuggestGrindMaterials = config.GetIntDefault("AiPlayerbot.BroadcastChanceSuggestGrindMaterials", 5000);
    broadcastChanceSuggestGrindReputation = config.GetIntDefault("AiPlayerbot.BroadcastChanceSuggestGrindReputation", 5000);
    broadcastChanceSuggestSell = config.GetIntDefault("AiPlayerbot.BroadcastChanceSuggestSell", 300);
    broadcastChanceSuggestSomething = config.GetIntDefault("AiPlayerbot.BroadcastChanceSuggestSomething", 30000);

    broadcastChanceSuggestSomethingToxic = config.GetIntDefault("AiPlayerbot.BroadcastChanceSuggestSomethingToxic", 0);

    broadcastChanceSuggestToxicLinks = config.GetIntDefault("AiPlayerbot.BroadcastChanceSuggestToxicLinks", 0);
    toxicLinksPrefix = config.GetStringDefault("AiPlayerbot.ToxicLinksPrefix", "gnomes");

    broadcastChanceSuggestThunderfury = config.GetIntDefault("AiPlayerbot.BroadcastChanceSuggestThunderfury", 1);

    //does not depend on global chance
    broadcastChanceGuildManagement = config.GetIntDefault("AiPlayerbot.BroadcastChanceGuildManagement", 30000);
    ////////////////////////////

    toxicLinksRepliesChance = config.GetIntDefault("AiPlayerbot.ToxicLinksRepliesChance", 30); //0-100
    thunderfuryRepliesChance = config.GetIntDefault("AiPlayerbot.ThunderfuryRepliesChance", 40); //0-100
    guildRepliesRate = config.GetIntDefault("AiPlayerbot.GuildRepliesRate", 100); //0-100

    botAcceptDuelMinimumLevel = config.GetIntDefault("AiPlayerbot.BotAcceptDuelMinimumLevel", 10);

    randomBotFormGuild = config.GetBoolDefault("AiPlayerbot.RandomBotFormGuild", true);

    boostFollow = config.GetBoolDefault("AiPlayerbot.BoostFollow", false);
    turnInRpg = config.GetBoolDefault("AiPlayerbot.TurnInRpg", false);
    globalSoundEffects = config.GetBoolDefault("AiPlayerbot.GlobalSoundEffects", false);
    nonGmFreeSummon = config.GetBoolDefault("AiPlayerbot.NonGmFreeSummon", false);

    //SPP automation
    autoPickReward = config.GetStringDefault("AiPlayerbot.AutoPickReward", "no");
    autoEquipUpgradeLoot = config.GetBoolDefault("AiPlayerbot.AutoEquipUpgradeLoot", false);
    syncQuestWithPlayer = config.GetBoolDefault("AiPlayerbot.SyncQuestWithPlayer", false);
    syncQuestForPlayer = config.GetBoolDefault("AiPlayerbot.SyncQuestForPlayer", false);
    autoTrainSpells = config.GetStringDefault("AiPlayerbot.AutoTrainSpells", "no");
    autoPickTalents = config.GetStringDefault("AiPlayerbot.AutoPickTalents", "no");
    autoLearnTrainerSpells = config.GetBoolDefault("AiPlayerbot.AutoLearnTrainerSpells", false);
    autoLearnQuestSpells = config.GetBoolDefault("AiPlayerbot.AutoLearnQuestSpells", false);
    autoLearnDroppedSpells = config.GetBoolDefault("AiPlayerbot.AutoLearnDroppedSpells", false);
    autoDoQuests = config.GetBoolDefault("AiPlayerbot.AutoDoQuests", true);
    syncLevelWithPlayers = config.GetBoolDefault("AiPlayerbot.SyncLevelWithPlayers", false);
    syncLevelMaxAbove = config.GetIntDefault("AiPlayerbot.SyncLevelMaxAbove", 5);
    syncLevelNoPlayer = config.GetIntDefault("AiPlayerbot.SyncLevelNoPlayer", randombotStartingLevel);
    tweakValue = config.GetIntDefault("AiPlayerbot.TweakValue", 0);
    talentsInPublicNote = config.GetBoolDefault("AiPlayerbot.TalentsInPublicNote", false);
    respawnModNeutral = config.GetFloatDefault("AiPlayerbot.RespawnModNeutral", 10.0f);
    respawnModHostile = config.GetFloatDefault("AiPlayerbot.RespawnModHostile", 5.0f);
    respawnModThreshold = config.GetIntDefault("AiPlayerbot.RespawnModThreshold", 10);
    respawnModMax = config.GetIntDefault("AiPlayerbot.RespawnModMax", 18);
    respawnModForPlayerBots = config.GetBoolDefault("AiPlayerbot.RespawnModForPlayerBots", false);
    respawnModForInstances = config.GetBoolDefault("AiPlayerbot.RespawnModForInstances", false);

    //LLM START
    llmEnabled = config.GetIntDefault("AiPlayerbot.LLMEnabled", 1);
    llmApiEndpoint = config.GetStringDefault("AiPlayerbot.LLMApiEndpoint", "http://127.0.0.1:5001/api/v1/generate");
    try {
        llmEndPointUrl = parseUrl(llmApiEndpoint);
    }
    catch (const std::invalid_argument& e) {
        sLog.outError("Unable to parse LLMApiEndpoint url: %s", e.what());
    }
    llmApiKey = config.GetStringDefault("AiPlayerbot.LLMApiKey", "");    
    llmApiJson = config.GetStringDefault("AiPlayerbot.LLMApiJson", "{ \"max_length\": 100, \"prompt\": \"[<pre prompt>]<context> <prompt> <post prompt>\"}");
    llmContextLength = config.GetIntDefault("AiPlayerbot.LLMContextLength", 4096);
    llmGenerationTimeout = config.GetIntDefault("AiPlayerbot.LLMGenerationTimeout", 600);
    llmMaxSimultaniousGenerations = config.GetIntDefault("AiPlayerbot.LLMMaxSimultaniousGenerations", 100);
        
    
    llmPrePrompt = config.GetStringDefault("AiPlayerbot.LLMPrePrompt", "You are a roleplaying character in World of Warcraft: <expansion name>. Your name is <bot name>. The <other type> <other name> is speaking to you <channel name> and is an <other gender> <other race> <other class> of level <other level>. You are level <bot level> and play as a <bot gender> <bot race> <bot class> that is currently in <bot subzone> <bot zone>. Answer as a roleplaying character. Limit responses to 100 characters.");

    llmPreRpgPrompt = config.GetStringDefault("AiPlayerbot.LLMRpgPrompt", "In World of Warcraft: <expansion name> in <bot zone> <bot subzone> stands <bot type> <bot name> a level <bot level> <bot gender> <bot race> <bot class>."
        " Standing nearby is <unit type> <unit name> <unit subname> a level <unit level> <unit gender> <unit race> <unit faction> <unit class>. Answer as a roleplaying character. Limit responses to 100 characters.");



    llmPrompt = config.GetStringDefault("AiPlayerbot.LLMPrompt", "<receiver name>:<initial message>");
    llmPostPrompt = config.GetStringDefault("AiPlayerbot.LLMPostPrompt", "<sender name>:");

    llmResponseStartPattern = config.GetStringDefault("AiPlayerbot.LLMResponseStartPattern", R"(("text":\s*"))");
    llmResponseEndPattern = config.GetStringDefault("AiPlayerbot.LLMResponseEndPattern", R"(("|\b(?!<sender name>\b)(\w+):))");
    llmResponseDeletePattern = config.GetStringDefault("AiPlayerbot.LLMResponseDeletePattern", R"((\\n|<sender name>:|\\[^ ]+))");
    llmResponseSplitPattern = config.GetStringDefault("AiPlayerbot.LLMResponseSplitPattern", R"((\*.*?\*)|(\[.*?\])|(\'.*\')|([^\*\[\] ][^\*\[\]]+?[.?!]))");

    if (false) //Disable for release
    {
        sLog.outError("# AiPlayerbot.LLMResponseStartPattern = %s", llmResponseStartPattern.c_str());
        sLog.outError("# AiPlayerbot.LLMResponseEndPattern = %s", llmResponseEndPattern.c_str());
        sLog.outError("# AiPlayerbot.LLMResponseDeletePattern = %s", llmResponseDeletePattern.c_str());
        sLog.outError("# AiPlayerbot.LLMResponseSplitPattern = %s", llmResponseSplitPattern.c_str());
    }

    try {
        std::regex pattern(llmResponseStartPattern);
    }
    catch (const std::regex_error& e) {        
        sLog.outError("Regex error in %s: %s", llmResponseStartPattern.c_str(), e.what());
    }

    try {
        std::regex pattern(llmResponseEndPattern);
    }
    catch (const std::regex_error& e) {
        sLog.outError("Regex error in %s: %s", llmResponseEndPattern.c_str(), e.what());
    }

    try {
        std::regex pattern(llmResponseDeletePattern);
    }
    catch (const std::regex_error& e) {
        sLog.outError("Regex error in %s: %s", llmResponseDeletePattern.c_str(), e.what());
    }

    try {
        std::regex pattern(llmResponseSplitPattern);
    }
    catch (const std::regex_error& e) {
        sLog.outError("Regex error in %s: %s", llmResponseSplitPattern.c_str(), e.what());
    }

    llmGlobalContext = config.GetBoolDefault("AiPlayerbot.LLMGlobalContext", false);
    llmBotToBotChatChance = config.GetIntDefault("AiPlayerbot.LLMBotToBotChatChance", 0);
    llmRpgAIChatChance = config.GetIntDefault("AiPlayerbot.LLMRpgAIChatChance", 100);

    std::list<std::string> blockedChannels;
    LoadListString<std::list<std::string>>(config.GetStringDefault("AiPlayerbot.LLMBlockedReplyChannels", ""), blockedChannels);
    std::map<std::string, ChatChannelSource> sourceName;
    sourceName["guild"] = ChatChannelSource::SRC_GUILD;
    sourceName["world"] = ChatChannelSource::SRC_WORLD;
    sourceName["general"] = ChatChannelSource::SRC_GENERAL;
    sourceName["trade"] = ChatChannelSource::SRC_TRADE;
    sourceName["lfg"] = ChatChannelSource::SRC_LOOKING_FOR_GROUP;
    sourceName["ldefence"] = ChatChannelSource::SRC_LOCAL_DEFENSE;
    sourceName["wdefence"] = ChatChannelSource::SRC_WORLD_DEFENSE;
    sourceName["grecruitement"] = ChatChannelSource::SRC_GUILD_RECRUITMENT;
    sourceName["say"] = ChatChannelSource::SRC_SAY;
    sourceName["whisper"] = ChatChannelSource::SRC_WHISPER;
    sourceName["emote"] = ChatChannelSource::SRC_EMOTE;
    sourceName["temote"] = ChatChannelSource::SRC_TEXT_EMOTE;
    sourceName["yell"] = ChatChannelSource::SRC_YELL;
    sourceName["party"] = ChatChannelSource::SRC_PARTY;
    sourceName["raid"] = ChatChannelSource::SRC_RAID;

    for (auto& channelName : blockedChannels)
        llmBlockedReplyChannels.insert(sourceName[channelName]);

    //LLM END

    // Gear progression system
    gearProgressionSystemEnabled = config.GetBoolDefault("AiPlayerbot.GearProgressionSystem.Enable", false);

    // Gear progression phase
    for (uint8 phase = 0; phase < MAX_GEAR_PROGRESSION_LEVEL; phase++)
    {
        std::ostringstream os; os << "AiPlayerbot.GearProgressionSystem." << std::to_string(phase) << ".MinItemLevel";
        gearProgressionSystemItemLevels[phase][0] = config.GetIntDefault(os.str().c_str(), 9999999);
        os.str(""); os << "AiPlayerbot.GearProgressionSystem." << std::to_string(phase) << ".MaxItemLevel";
        gearProgressionSystemItemLevels[phase][1] = config.GetIntDefault(os.str().c_str(), 9999999);

        // Gear progression class
        for (uint8 cls = 1; cls < MAX_CLASSES; cls++)
        {
            // Gear progression spec
            for (uint8 spec = 0; spec < 4; spec++)
            {
                // Gear progression slot
                for (uint8 slot = 0; slot < SLOT_EMPTY; slot++)
                {
                    std::ostringstream os; os << "AiPlayerbot.GearProgressionSystem." << std::to_string(phase) << "." << std::to_string(cls) << "." << std::to_string(spec) << "." << std::to_string(slot);
                    gearProgressionSystemItems[phase][cls][spec][slot] = config.GetIntDefault(os.str().c_str(), -1);
                }
            }
        }
    }

    sLog.outString("Loading free bots.");
    selfBotLevel = BotSelfBotLevel(config.GetIntDefault("AiPlayerbot.SelfBotLevel", uint32(BotSelfBotLevel::GM_ONLY)));
    LoadListString<std::list<std::string>>(config.GetStringDefault("AiPlayerbot.ToggleAlwaysOnlineAccounts", ""), toggleAlwaysOnlineAccounts);
    LoadListString<std::list<std::string>>(config.GetStringDefault("AiPlayerbot.ToggleAlwaysOnlineChars", ""), toggleAlwaysOnlineChars);

    for (std::string& nm : toggleAlwaysOnlineAccounts)
        std::transform(nm.begin(), nm.end(), nm.begin(), toupper);

    for (std::string& nm : toggleAlwaysOnlineChars)
    {
        std::transform(nm.begin(), nm.end(), nm.begin(), tolower);
        nm[0] = toupper(nm[0]);
    }

    loadFreeAltBotAccounts();

    targetPosRecalcDistance = config.GetFloatDefault("AiPlayerbot.TargetPosRecalcDistance", 0.1f),

    sLog.outString("Loading area levels.");
    sTravelMgr.LoadAreaLevels();
    sLog.outString("Loading spellIds.");
    ChatHelper::PopulateSpellNameList();
    ItemUsageValue::PopulateProfessionReagentIds();
    ItemUsageValue::PopulateSoldByVendorItemIds();
    ItemUsageValue::PopulateReagentItemIdsForCraftableItemIds();

    RandomPlayerbotFactory::CreateRandomBots();
    PlayerbotFactory::Init();
    sRandomItemMgr.Init();
    sPlayerbotTextMgr.LoadBotTexts();
    sPlayerbotTextMgr.LoadBotTextChance();
    sPlayerbotHelpMgr.LoadBotHelpTexts();

    LoadTalentSpecs();

    if (sPlayerbotAIConfig.autoDoQuests)
    {
        sLog.outString("Loading Quest Detail Data...");
        sTravelMgr.LoadQuestTravelTable();
    }

    sLog.outString("Loading named locations...");
    sRandomPlayerbotMgr.LoadNamedLocations();

    if (sPlayerbotAIConfig.randomBotJoinBG)
        sRandomPlayerbotMgr.LoadBattleMastersCache();

    sLog.outString("---------------------------------------");
    sLog.outString("        AI Playerbot initialized       ");
    sLog.outString("---------------------------------------");
    sLog.outString();

    return true;
}

bool PlayerbotAIConfig::IsInRandomAccountList(uint32 id)
{
    return find(randomBotAccounts.begin(), randomBotAccounts.end(), id) != randomBotAccounts.end();
}

bool PlayerbotAIConfig::IsFreeAltBot(uint32 guid)
{
    for (auto bot : freeAltBots)
        if (bot.second == guid)
            return true;

    return false;
}

bool PlayerbotAIConfig::IsInRandomQuestItemList(uint32 id)
{
    return find(randomBotQuestItems.begin(), randomBotQuestItems.end(), id) != randomBotQuestItems.end();
}

bool PlayerbotAIConfig::IsInPvpProhibitedZone(uint32 id)
{
	return find(pvpProhibitedZoneIds.begin(), pvpProhibitedZoneIds.end(), id) != pvpProhibitedZoneIds.end();
}

std::string PlayerbotAIConfig::GetValue(std::string name)
{
    std::ostringstream out;

    if (name == "GlobalCooldown")
        out << globalCoolDown;
    else if (name == "ReactDelay")
        out << reactDelay;

    else if (name == "SightDistance")
        out << sightDistance;
    else if (name == "SpellDistance")
        out << spellDistance;
    else if (name == "ReactDistance")
        out << reactDistance;
    else if (name == "GrindDistance")
        out << grindDistance;
    else if (name == "LootDistance")
        out << lootDistance;
    else if (name == "FleeDistance")
        out << fleeDistance;

    else if (name == "CriticalHealth")
        out << criticalHealth;
    else if (name == "LowHealth")
        out << lowHealth;
    else if (name == "MediumHealth")
        out << mediumHealth;
    else if (name == "AlmostFullHealth")
        out << almostFullHealth;
    else if (name == "LowMana")
        out << lowMana;

    else if (name == "IterationsPerTick")
        out << iterationsPerTick;

    return out.str();
}

void PlayerbotAIConfig::SetValue(std::string name, std::string value)
{
    std::istringstream out(value, std::istringstream::in);

    if (name == "GlobalCooldown")
        out >> globalCoolDown;
    else if (name == "ReactDelay")
        out >> reactDelay;

    else if (name == "SightDistance")
        out >> sightDistance;
    else if (name == "SpellDistance")
        out >> spellDistance;
    else if (name == "ReactDistance")
        out >> reactDistance;
    else if (name == "GrindDistance")
        out >> grindDistance;
    else if (name == "LootDistance")
        out >> lootDistance;
    else if (name == "FleeDistance")
        out >> fleeDistance;

    else if (name == "CriticalHealth")
        out >> criticalHealth;
    else if (name == "LowHealth")
        out >> lowHealth;
    else if (name == "MediumHealth")
        out >> mediumHealth;
    else if (name == "AlmostFullHealth")
        out >> almostFullHealth;
    else if (name == "LowMana")
        out >> lowMana;

    else if (name == "IterationsPerTick")
        out >> iterationsPerTick;
}

void PlayerbotAIConfig::loadFreeAltBotAccounts()
{
    bool allCharsOnline = (selfBotLevel == BotSelfBotLevel::ALWAYS_ACTIVE);

    freeAltBots.clear();

    auto results = LoginDatabase.PQuery("SELECT username, id FROM account where username not like '%s%%'", randomBotAccountPrefix.c_str());
    if (results)
    {
        do
        {
            bool accountToggle = false;

            Field* fields = results->Fetch();
            std::string accountName = fields[0].GetString();
            uint32 accountId = fields[1].GetUInt32();

            if (std::find(toggleAlwaysOnlineAccounts.begin(), toggleAlwaysOnlineAccounts.end(), accountName) != toggleAlwaysOnlineAccounts.end())
                accountToggle = true;

            auto result = CharacterDatabase.PQuery("SELECT name, guid FROM characters WHERE account = '%u'", accountId);
            if (!result)
                continue;

            do
            {
                bool charToggle = false;

                Field* fields = result->Fetch();
                std::string charName = fields[0].GetString();
                uint32 guid = fields[1].GetUInt32();

                BotAlwaysOnline always = BotAlwaysOnline(sRandomPlayerbotMgr.GetValue(guid, "always"));

                if (always == BotAlwaysOnline::DISABLED_BY_COMMAND)
                    continue;

                if (std::find(toggleAlwaysOnlineChars.begin(), toggleAlwaysOnlineChars.end(), charName) != toggleAlwaysOnlineChars.end())
                    charToggle = true;

                bool thisCharAlwaysOnline = allCharsOnline;

                if (accountToggle || charToggle)
                    thisCharAlwaysOnline = !thisCharAlwaysOnline;

                if ((thisCharAlwaysOnline && always != BotAlwaysOnline::DISABLED_BY_COMMAND) || always == BotAlwaysOnline::ACTIVE)
                {
                    sLog.outString("Enabling always online for %s", charName.c_str());
                    freeAltBots.push_back(std::make_pair(accountId, guid));
                }

            } while (result->NextRow());


        } while (results->NextRow());
    }
}

std::string PlayerbotAIConfig::GetTimestampStr()
{
    time_t t = time(nullptr);
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    char buf[20];
    snprintf(buf, 20, "%04d-%02d-%02d %02d:%02d:%02d", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
    return std::string(buf);
}

bool PlayerbotAIConfig::openLog(std::string fileName, char const* mode, bool haslog)
{
    if (!haslog && !hasLog(fileName))
        return false;

    auto logFileIt = logFiles.find(fileName);
    if (logFileIt == logFiles.end())
    {
        logFiles.insert(make_pair(fileName, std::make_pair(nullptr, false)));
        logFileIt = logFiles.find(fileName);
    }

    FILE* file = logFileIt->second.first;
    bool fileOpen = logFileIt->second.second;

    if (fileOpen) //close log file
        fclose(file);

    std::string m_logsDir = sConfig.GetStringDefault("LogsDir");
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }


    file = fopen((m_logsDir + fileName).c_str(), mode);
    fileOpen = true;

    logFileIt->second.first = file;
    logFileIt->second.second = fileOpen;

    return true;
}

void PlayerbotAIConfig::log(std::string fileName, const char* str, ...)
{
    if (!str)
        return;

    std::lock_guard<std::mutex> guard(m_logMtx);

    if (!isLogOpen(fileName))
        if (!openLog(fileName, "a"))
            return;

    FILE* file = logFiles.find(fileName)->second.first;

    va_list ap;
    va_start(ap, str);
    vfprintf(file, str, ap);
    fprintf(file, "\n");
    va_end(ap);
    fflush(file);

    fflush(stdout);
}

void PlayerbotAIConfig::logEvent(PlayerbotAI* ai, std::string eventName, std::string info1, std::string info2)
{
    if (hasLog("bot_events.csv"))
    {
        Player* bot = ai->GetBot();

        std::ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
        out << bot->GetName() << ",";
        out << eventName << ",";
        out << std::fixed << std::setprecision(2);
        WorldPosition(bot).printWKT(out);

        out << std::to_string(bot->getRace()) << ",";
        out << std::to_string(bot->getClass()) << ",";
        float subLevel = ai->GetLevelFloat();

        out << subLevel << ",";

        out << "\"" << info1 << "\",";
        out << "\"" << info2 << "\"";

        log("bot_events.csv", out.str().c_str());
    }
};

void PlayerbotAIConfig::logEvent(PlayerbotAI* ai, std::string eventName, ObjectGuid guid, std::string info2)
{
    std::string info1 = "";

    Unit* victim;
    if (guid)
    {
        victim = ai->GetUnit(guid);
        if (victim)
            info1 = victim->GetName();
    }

    logEvent(ai, eventName, info1, info2);
};

bool PlayerbotAIConfig::CanLogAction(PlayerbotAI* ai, std::string actionName, bool isExecute, std::string lastActionName)
{
    bool forRpg = (actionName.find("rpg") == 0) && ai->HasStrategy("debug rpg", BotState::BOT_STATE_NON_COMBAT);

    if (!forRpg)
    {
        if (isExecute && !ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
            return false;

        if (!isExecute && !ai->HasStrategy("debug action", BotState::BOT_STATE_NON_COMBAT))
            return false;

        if ((lastActionName == actionName) && (actionName == "melee"))
        {
            return false;
        }
    }

    return std::find(debugFilter.begin(), debugFilter.end(), actionName) == debugFilter.end();
}

void PlayerbotAIConfig::LoadTalentSpecs()
{
    sLog.outString("Loading TalentSpecs");

    uint32 maxSpecLevel = 0;

    for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
    {
        classSpecs[cls] = ClassSpecs(1 << (cls - 1));
        for (uint32 spec = 0; spec < MAX_LEVEL; ++spec)
        {
            std::ostringstream os; os << "AiPlayerbot.PremadeSpecName." << cls << "." << spec;
            std::string specName = config.GetStringDefault(os.str().c_str(), "");
            if (!specName.empty())
            {
                std::ostringstream os; os << "AiPlayerbot.PremadeSpecProb." << cls << "." << spec;
                int probability = config.GetIntDefault(os.str().c_str(), 100);

                TalentPath talentPath(spec, specName, probability);

                for (uint32 level = 10; level <= 100; level++)
                {
                    std::ostringstream os; os << "AiPlayerbot.PremadeSpecLink." << cls << "." << spec << "." << level;
                    std::string specLink = config.GetStringDefault(os.str().c_str(), "");
                    specLink = specLink.substr(0, specLink.find("#", 0));
                    specLink = specLink.substr(0, specLink.find(" ", 0));

                    if (!specLink.empty())
                    {
                        if (maxSpecLevel < level)
                            maxSpecLevel = level;

                        std::ostringstream out;

                        //Ignore bad specs.
                        if (!classSpecs[cls].baseSpec.CheckTalentLink(specLink, &out))
                        {
                            sLog.outErrorDb("Error with premade spec link: %s", specLink.c_str());
                            sLog.outErrorDb("%s", out.str().c_str());
                            continue;
                        }

                        TalentSpec linkSpec(&classSpecs[cls].baseSpec, specLink);

                        if (!linkSpec.CheckTalents(TalentSpec::LeveltoPoints(level), &out))
                        {
                            sLog.outErrorDb("Error with premade spec: %s", specLink.c_str());
                            sLog.outErrorDb("%s", out.str().c_str());
                            continue;
                        }


                        talentPath.talentSpec.push_back(linkSpec);
                    }

                    {
                        //Glyphs

                        using GlyphPriority = std::pair<std::string, uint32>;
                        using GlyphPriorityList = std::vector<GlyphPriority>;
                        using GlyphPriorityLevelMap = std::unordered_map<uint32, GlyphPriorityList>;
                        using GlyphPrioritySpecMap = std::unordered_map<uint32, GlyphPriorityLevelMap>;

                        std::ostringstream os; os << "AiPlayerbot.PremadeSpecGlyp." << cls << "." << spec << "." << level;

                        std::string glyphList = config.GetStringDefault(os.str().c_str(), "");
                        glyphList = glyphList.substr(0, glyphList.find("#", 0));
                        boost::trim_right(glyphList);

                        if (!glyphList.empty())
                        {
                            Tokens premadeSpecGlyphs = Qualified::getMultiQualifiers(glyphList, ",");

                            for (auto& glyph : premadeSpecGlyphs)
                            {
                                Tokens tokens = Qualified::getMultiQualifiers(glyph, "|");
                                std::string glyphName = "Glyph of " + tokens[0];
                                uint32 talentId = tokens.size() > 1 ? stoi(tokens[1]) : 0;

                                bool glyphFound = false;
                                for (auto& itemId : sRandomItemMgr.GetGlyphs(1 << (cls - 1)))
                                {
                                    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);

                                    if (!proto)
                                        continue;

                                    if (proto->Name1 == glyphName)
                                    {
                                        glyphPriorityMap[cls][spec][level].push_back(std::make_pair(itemId, talentId));
                                        glyphFound = true;
                                        break;
                                    }
                                }

                                if (!glyphFound)
                                {
                                    sLog.outError("%s is not found for class %d (spec %d level %d)", glyphName.c_str(), cls, spec, level);
                                }

                            }
                        }
                    }
                }

                //Only add paths that have atleast 1 spec.
                if (talentPath.talentSpec.size() > 0)
                    classSpecs[cls].talentPath.push_back(talentPath);
            }
        }
    }

    if (classSpecs[1].talentPath.empty())
        sLog.outErrorDb("No premade specs found!!");
    else
    {
        if (maxSpecLevel < DEFAULT_MAX_LEVEL && randomBotMaxLevel < DEFAULT_MAX_LEVEL)
            sLog.outErrorDb("!!!!!!!!!!! randomBotMaxLevel and the talentspec levels are below this expansions max level. Please check if you have the correct config file!!!!!!");

    }
}
