#pragma once

#include "Config/Config.h"
#include "Talentspec.h"
#include "Globals/SharedDefines.h"

class Player;
class PlayerbotMgr;
class ChatHandler;

enum class BotCheatMask : uint32
{
    none = 0,
    taxi = 1 << 0,
    gold = 1 << 1,
    health = 1 << 2,
    mana = 1 << 3,
    power = 1 << 4,
    item = 1 << 5,
    cooldown = 1 << 6,
    repair = 1 << 7,
    movespeed = 1 << 8,
    attackspeed = 1 << 9,
    breath = 1 << 10,
    glyph = 1 << 11,
    maxMask = 1 << 12
};

enum class BotAutoLogin : uint32
{
    DISABLED = 0,
    LOGIN_ALL_WITH_MASTER = 1,
    LOGIN_ONLY_ALWAYS_ACTIVE = 2
};

enum class BotSelfBotLevel : uint32
{
    DISABLED = 0,
    GM_ONLY = 1,
    ACTIVE_BY_COMMAND = 2,
    ALWAYS_ALLOWED = 3,
    ACTIVE_BY_LOGIN = 4, 
    ALWAYS_ACTIVE = 5
};

enum class BotAlwaysOnline : uint32
{
    DISABLED = 0,
    ACTIVE = 1,
    DISABLED_BY_COMMAND = 2
};

enum class BotLoginCriteriaType : uint8
{
    RACECLASS = 0,
    LEVEL = 1,
    RANGE_TO_PLAYER = 2,
    MAX_LOGIN_CRITERIA = 3
};

#define MAX_GEAR_PROGRESSION_LEVEL 6

class ConfigAccess
{
private:
    std::string m_filename;
    std::string m_envVarPrefix;
    std::unordered_map<std::string, std::string> m_entries; // keys are converted to lower case.  values cannot be.

public:
    std::vector<std::string> GetValues(const std::string& name) const;
    std::mutex m_configLock;
};

struct ParsedUrl {
    std::string hostname;
    std::string path;
    int port;
    bool https;
};

//GlyphPrioritySpecMap[specId][level] = {{glyphItemId, prereqTalentSpell}};
using GlyphPriority = std::pair<uint32, uint32>;
using GlyphPriorityList = std::vector<GlyphPriority>;
using GlyphPriorityLevelMap = std::unordered_map<uint32, GlyphPriorityList>;
using GlyphPrioritySpecMap = std::unordered_map<uint32, GlyphPriorityLevelMap>;

class PlayerbotAIConfig
{
public:
    PlayerbotAIConfig();
    static PlayerbotAIConfig& instance()
    {
        static PlayerbotAIConfig instance;
        return instance;
    }

public:
    bool Initialize();
    bool IsInRandomAccountList(uint32 id);
    bool IsFreeAltBot(uint32 guid);
    bool IsFreeAltBot(Player* player) {return IsFreeAltBot(player->GetGUIDLow());}
    bool IsInRandomQuestItemList(uint32 id);
	bool IsInPvpProhibitedZone(uint32 id);

    bool enabled;
    bool allowGuildBots;
    bool allowMultiAccountAltBots;
    uint32 globalCoolDown, reactDelay, maxWaitForMove, expireActionTime, dispelAuraDuration, passiveDelay, repeatDelay,
        errorDelay, rpgDelay, sitDelay, returnDelay, lootDelay;
    float sightDistance, spellDistance, reactDistance, grindDistance, lootDistance, groupMemberLootDistance, groupMemberLootDistanceWithActiveMaster,
        gatheringDistance, groupMemberGatheringDistance, groupMemberGatheringDistanceWithActiveMaster, shootDistance,
        fleeDistance, tooCloseDistance, meleeDistance, followDistance, raidFollowDistance, whisperDistance, contactDistance,
        aoeRadius, rpgDistance, targetPosRecalcDistance, farDistance, healDistance, aggroDistance, proximityDistance, maxFreeMoveDistance, freeMoveDelay;
    uint32 criticalHealth, lowHealth, mediumHealth, almostFullHealth;
    uint32 lowMana, mediumMana;

    uint32 openGoSpell;
    bool randomBotAutologin;
    BotAutoLogin botAutologin;
    std::string randomBotMapsAsString;
    std::vector<uint32> randomBotMaps;
    std::list<uint32> randomBotQuestItems;
    std::list<uint32> randomBotAccounts;
    std::list<uint32> randomBotSpellIds;
    std::list<uint32> randomBotQuestIds;
    std::list<uint32> immuneSpellIds;
    std::list<std::pair<uint32, uint32>> freeAltBots;
    std::list<std::string> toggleAlwaysOnlineAccounts;
    std::list<std::string> toggleAlwaysOnlineChars;
    bool enableRandomTeleports;
    uint32 randomBotTeleportDistance;
    bool randomBotTeleportNearPlayer;
    uint32 randomBotTeleportNearPlayerMaxAmount;
    float randomBotTeleportNearPlayerMaxAmountRadius;
    uint32 randomBotTeleportMinInterval, randomBotTeleportMaxInterval;
    uint32 randomGearMaxLevel;
    uint32 randomGearMaxDiff;
    bool randomGearUpgradeEnabled;
    bool randomGearTabards;
    bool randomGearTabardsReplaceGuild;
    bool randomGearTabardsUnobtainable;
    float randomGearTabardsChance;
    std::list<uint32> randomGearBlacklist;
    std::list<uint32> randomGearWhitelist;
    bool randomGearProgression;
    float randomGearLoweringChance;
    float randomBotMaxLevelChance;
    float randomBotRpgChance;
    float usePotionChance;
    float attackEmoteChance;
    bool randomBotAutoCreate;
    uint32 minRandomBots, maxRandomBots;
    uint32 randomBotUpdateInterval, randomBotCountChangeMinInterval, randomBotCountChangeMaxInterval;
    uint32 loginBoostPercentage;
    bool randomBotTimedLogout, randomBotTimedOffline;
    uint32 minRandomBotInWorldTime, maxRandomBotInWorldTime;
    uint32 minRandomBotRandomizeTime, maxRandomBotRandomizeTime;
    uint32 minRandomBotChangeStrategyTime, maxRandomBotChangeStrategyTime;
    uint32 minRandomBotReviveTime, maxRandomBotReviveTime;
    uint32 minRandomBotPvpTime, maxRandomBotPvpTime;
    uint32 randomBotsPerInterval, randomBotsMaxLoginsPerInterval;
    uint32 minRandomBotsPriceChangeInterval, maxRandomBotsPriceChangeInterval;
    //Auction house settings
    bool shouldQueryAHListingsOutsideOfAH;
    std::list<uint32> ahOverVendorItemIds;
    std::list<uint32> vendorOverAHItemIds;
    bool botCheckAllAuctionListings;
    //
    bool randomBotJoinLfg;
    bool logRandomBotJoinLfg;
    bool randomBotJoinBG;
    bool randomBotAutoJoinBG;
    uint32 randomBotBracketCount;
    bool randomBotLoginAtStartup;
    uint32 randomBotTeleLevel;
    bool logInGroupOnly, logValuesPerTick;
    bool fleeingEnabled;
    bool summonAtInnkeepersEnabled;
    std::string combatStrategies, nonCombatStrategies, reactStrategies, deadStrategies;
    std::string randomBotCombatStrategies, randomBotNonCombatStrategies, randomBotReactStrategies, randomBotDeadStrategies;
    uint32 randomBotMinLevel, randomBotMaxLevel;
    float randomChangeMultiplier;
    uint32 specProbability[MAX_CLASSES][10];
    std::string premadeLevelSpec[MAX_CLASSES][10][91]; //lvl 10 - 100
    uint32 classRaceProbabilityTotal;
    uint32 classRaceProbability[MAX_CLASSES][MAX_RACES];
    bool useFixedClassRaceCounts;
    using ClassRacePair = std::pair<uint8, uint8>;
    std::map<ClassRacePair, uint32> fixedClassRaceCounts;
    uint32 levelProbability[DEFAULT_MAX_LEVEL + 1];
    ClassSpecs classSpecs[MAX_CLASSES];
    GlyphPrioritySpecMap glyphPriorityMap[MAX_CLASSES];
    bool gearProgressionSystemEnabled;
    uint32 gearProgressionSystemItemLevels[MAX_GEAR_PROGRESSION_LEVEL][2];
    int32 gearProgressionSystemItems[MAX_GEAR_PROGRESSION_LEVEL][MAX_CLASSES][4][SLOT_EMPTY];
    std::string commandPrefix, commandSeparator;
    std::string randomBotAccountPrefix;
    uint32 randomBotAccountCount;
    bool deleteRandomBotAccounts;
    uint32 randomBotGuildCount;
    bool deleteRandomBotGuilds;
    uint32 randomBotArenaTeamCount;
    bool deleteRandomBotArenaTeams;
    std::list<uint32> randomBotArenaTeams;
	bool RandombotsWalkingRPG;
	bool RandombotsWalkingRPGInDoors;
    bool boostFollow;
    bool turnInRpg;
    bool globalSoundEffects;
    std::list<uint32> randomBotGuilds;
	std::list<uint32> pvpProhibitedZoneIds;
    bool enableGreet;
    bool randomBotShowHelmet;
    bool randomBotShowCloak;
    bool disableRandomLevels;
    bool instantRandomize;
    bool gearscorecheck;
    int32 levelCheck;
	bool randomBotPreQuests;
    float playerbotsXPrate;
    bool disableBotOptimizations;
    bool disableActivityPriorities;
    uint32 botActiveAlone;
    uint32 diffWithPlayer;
    uint32 diffEmpty;
    uint32 minEnchantingBotLevel;
    uint32 randombotStartingLevel;
    bool randomBotSayWithoutMaster;
    bool randomBotInvitePlayer;
    bool randomBotGroupNearby;
    bool randomBotRaidNearby;
    bool randomBotGuildNearby;
    bool randomBotFormGuild;
    bool randomBotRandomPassword;
    bool inviteChat;

    uint32 guildMaxBotLimit;

    bool enableBroadcasts;
    uint32 broadcastChanceMaxValue;

    uint32 broadcastToGuildGlobalChance;
    uint32 broadcastToWorldGlobalChance;
    uint32 broadcastToGeneralGlobalChance;
    uint32 broadcastToTradeGlobalChance;
    uint32 broadcastToLFGGlobalChance;
    uint32 broadcastToLocalDefenseGlobalChance;
    uint32 broadcastToWorldDefenseGlobalChance;
    uint32 broadcastToGuildRecruitmentGlobalChance;
    uint32 broadcastToSayGlobalChance;
    uint32 broadcastToYellGlobalChance;

    uint32 broadcastChanceLootingItemPoor;
    uint32 broadcastChanceLootingItemNormal;
    uint32 broadcastChanceLootingItemUncommon;
    uint32 broadcastChanceLootingItemRare;
    uint32 broadcastChanceLootingItemEpic;
    uint32 broadcastChanceLootingItemLegendary;
    uint32 broadcastChanceLootingItemArtifact;

    uint32 broadcastChanceQuestAccepted;
    uint32 broadcastChanceQuestUpdateObjectiveCompleted;
    uint32 broadcastChanceQuestUpdateObjectiveProgress;
    uint32 broadcastChanceQuestUpdateFailedTimer;
    uint32 broadcastChanceQuestUpdateComplete;
    uint32 broadcastChanceQuestTurnedIn;

    uint32 broadcastChanceKillNormal;
    uint32 broadcastChanceKillElite;
    uint32 broadcastChanceKillRareelite;
    uint32 broadcastChanceKillWorldboss;
    uint32 broadcastChanceKillRare;
    uint32 broadcastChanceKillUnknown;
    uint32 broadcastChanceKillPet;
    uint32 broadcastChanceKillPlayer;

    uint32 broadcastChanceLevelupGeneric;
    uint32 broadcastChanceLevelupTenX;
    uint32 broadcastChanceLevelupMaxLevel;

    uint32 broadcastChanceSuggestInstance;
    uint32 broadcastChanceSuggestQuest;
    uint32 broadcastChanceSuggestGrindMaterials;
    uint32 broadcastChanceSuggestGrindReputation;
    uint32 broadcastChanceSuggestSell;
    uint32 broadcastChanceSuggestSomething;

    uint32 broadcastChanceSuggestSomethingToxic;

    uint32 broadcastChanceSuggestToxicLinks;
    std::string toxicLinksPrefix;
    uint32 toxicLinksRepliesChance;

    uint32 broadcastChanceSuggestThunderfury;
    uint32 thunderfuryRepliesChance;

    uint32 broadcastChanceGuildManagement;

    uint32 guildRepliesRate;

    uint32 botAcceptDuelMinimumLevel;

    bool talentsInPublicNote;
    bool nonGmFreeSummon;

    BotSelfBotLevel selfBotLevel;
    uint32 iterationsPerTick;

    std::string autoPickReward;
    bool autoEquipUpgradeLoot;
    bool syncQuestWithPlayer;
    bool syncQuestForPlayer;
    std::string autoTrainSpells;
    std::string autoPickTalents;
    bool autoLearnTrainerSpells;
    bool autoLearnQuestSpells;
    bool autoLearnDroppedSpells;
    bool autoDoQuests;
    bool syncLevelWithPlayers;
    uint32 syncLevelMaxAbove, syncLevelNoPlayer;
    uint32 tweakValue; //Debugging config
    float respawnModNeutral, respawnModHostile;
    uint32 respawnModThreshold, respawnModMax;
    bool respawnModForPlayerBots, respawnModForInstances;

    bool randomBotLoginWithPlayer;
    bool asyncBotLogin, preloadHolders;
    uint32 freeRoomForNonSpareBots;
    uint32 loginBotsNearPlayerRange;
    std::vector<std::string> defaultLoginCriteria;
    std::vector<std::vector<std::string>> loginCriteria;

    bool jumpInBg;
    bool jumpWithPlayer;
    bool jumpFollow;
    bool jumpChase;
    bool useKnockback;
    float jumpNoCombatChance;
    float jumpMeleeInCombatChance;
    float jumpRandomChance;
    float jumpInPlaceChance;
    float jumpBackwardChance;
    float jumpHeightLimit;
    float jumpVSpeed;
    float jumpHSpeed;

    std::mutex m_logMtx;

    std::list<std::string> allowedLogFiles;
    std::list<std::string> debugFilter;

    std::unordered_map <std::string, std::pair<FILE*, bool>> logFiles;

    std::list<std::string> botCheats;
    uint32 botCheatMask = 0;

    std::vector<std::string> BotCheatMaskName = { "taxi", "gold", "health", "mana", "power", "item", "cooldown", "repair", "movespeed", "attackspeed", "breath", "glyph", "maxMask" };

    struct worldBuff{
        uint32 spellId;
        uint32 factionId = 0;
        uint32 classId = 0;
        uint32 specId = 0;
        uint32 minLevel = 0;
        uint32 maxLevel = 0;
        uint32 eventId = 0;
    };

    std::vector<worldBuff> worldBuffs;

    int commandServerPort;
    bool perfMonEnabled;
    bool bExplicitDbStoreSave = false;

    //LM BEGIN
    std::string llmApiEndpoint, llmApiKey, llmApiJson, llmPrePrompt, llmPreRpgPrompt, llmPrompt, llmPostPrompt, llmResponseStartPattern, llmResponseEndPattern, llmResponseDeletePattern, llmResponseSplitPattern;
    uint32 llmEnabled, llmContextLength, llmBotToBotChatChance, llmGenerationTimeout, llmMaxSimultaniousGenerations, llmRpgAIChatChance;
    bool llmGlobalContext;
    ParsedUrl llmEndPointUrl;
    std::set<uint32> llmBlockedReplyChannels;
    //LM END

    uint32 EatDrinkMinDistance = 5;
    uint32 EatDrinkMaxDistance = 1000;

    std::string GetValue(std::string name);
    void SetValue(std::string name, std::string value);

    void loadFreeAltBotAccounts();

    std::string GetTimestampStr();

    bool hasLog(std::string fileName) { return std::find(allowedLogFiles.begin(), allowedLogFiles.end(), fileName) != allowedLogFiles.end(); };
    bool openLog(std::string fileName, char const* mode = "a", bool haslog = false);
    bool isLogOpen(std::string fileName) { auto it = logFiles.find(fileName); return it != logFiles.end() && it->second.second;}
    void log(std::string fileName, const char* str, ...);

    void logEvent(PlayerbotAI* ai, std::string eventName, std::string info1 = "", std::string info2 = "");
    void logEvent(PlayerbotAI* ai, std::string eventName, ObjectGuid guid, std::string info2);

    bool CanLogAction(PlayerbotAI* ai, std::string actionName, bool isExecute, std::string lastActionName);

private:
    void LoadTalentSpecs();

    Config config;
};

#define sPlayerbotAIConfig MaNGOS::Singleton<PlayerbotAIConfig>::Instance()

