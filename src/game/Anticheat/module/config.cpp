#include "config.hpp"
#include "Anticheat/Anticheat.hpp"

#include "Policies/Singleton.h"

INSTANTIATE_SINGLETON_1(NamreebAnticheat::AnticheatConfig);

namespace
{
static const char *sCheatTypeNames[] =
{
    "WallClimb",
    "WaterWalk",
    "SlowFall",
    "BadFallReset",
    "FlyHack",
    "Forbidden",
    "MultiJump",
    "TimeBack",
    "FastJump",
    "JumpSpeedChange",
    "NullClientTime",
    "RootMove",
    "Explore",
    "ExploreHigh",
    "OverspeedZ",
    "HeartbeatSkip",
    "ClockDesync",
    "FakeTransport",
    "TeleToTransport",
    "Teleport",
    "TeleportFar",
    "FixedZ",
    "BadOrderAck",
    "Warden"
};

static_assert(sizeof(sCheatTypeNames) / sizeof(sCheatTypeNames[0]) == NamreebAnticheat::CheatType::CHEATS_COUNT,
    "sCheatTypeNames has the wrong number of strings");
}

namespace NamreebAnticheat
{
void AnticheatConfig::setConfig(AnticheatConfigUInt32Values index, char const* fieldname, uint32 defvalue)
{
    setConfig(index, GetIntDefault(fieldname, defvalue));
}

void AnticheatConfig::setConfig(AnticheatConfigBoolValues index, char const* fieldname, bool defvalue)
{
    setConfig(index, GetBoolDefault(fieldname, defvalue));
}

void AnticheatConfig::setConfig(AnticheatConfigFloatValues index, char const* fieldname, float defvalue)
{
    setConfig(index, GetFloatDefault(fieldname, defvalue));
}

AnticheatConfig::AnticheatConfig()
{
    memset(m_configUInt32Values, 0, sizeof(m_configUInt32Values));
    memset(m_configBoolValues, 0, sizeof(m_configBoolValues));
    memset(m_configFloatValues, 0, sizeof(m_configFloatValues));
}

void AnticheatConfig::loadConfigSettings()
{
    setConfig(CONFIG_BOOL_AC_ENABLED, "Enable", false);
    setConfig(CONFIG_BOOL_AC_MOVEMENT_USE_EXTRAPOLATION, "Movement.UseExtrapolation", false);
    setConfig(CONFIG_BOOL_AC_MOVEMENT_SPEED_HACK_ENABLED, "Movement.SpeedHack.Enable", false);
    setConfig(CONFIG_BOOL_AC_ANTISPAM_ENABLED, "Antispam.Enable", false);
    setConfig(CONFIG_BOOL_AC_ANTISPAM_SILENCE, "Antispam.Silence", false);

    setConfig(CONFIG_UINT32_AC_FINGERPRINT_HISTORY, "FingerprintHistory", 30);
    setConfig(CONFIG_UINT32_AC_FINGERPRINT_LEVEL, "FingerprintLevel", 6);

    setConfig(CONFIG_UINT32_AC_KICK_DELAY_MIN, "KickDelay.Min", 5);
    setConfig(CONFIG_UINT32_AC_KICK_DELAY_MAX, "KickDelay.Max", 10);
    setConfig(CONFIG_UINT32_AC_BAN_DELAY_MIN, "BanDelay.Min", 5);
    setConfig(CONFIG_UINT32_AC_BAN_DELAY_MAX, "BanDelay.Max", 10);
    setConfig(CONFIG_UINT32_AC_IP_BAN_DELAY_MIN, "IPBanDelay.Min", 5);
    setConfig(CONFIG_UINT32_AC_IP_BAN_DELAY_MAX, "IPBanDelay.Max", 10);

    setConfig(CONFIG_UINT32_AC_ANTISPAM_MAX_LEVEL, "Antispam.MaxLevel", 25);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_NORMALIZE_MASK, "Antispam.NormalizeMask", 0);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_ANALYSIS_TIMER, "Antispam.AnalysisTimer", 30);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_MAX_RATE, "Antispam.MaxRate", 30);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_RATE_GRACE_PERIOD, "Antispam.RateGracePeriod", 45);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_MAX_UNIQUE_PERCENTAGE, "Antispam.MaxUniquePercentage", 90);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_MIN_UNIQUE_MESSAGES, "Antispam.MinUniqueMessages", 100);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_BLACKLIST_NOTIFY, "Antispam.BlacklistNotify", 5);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_BLACKLIST_NOTIFY_COOLDOWN, "Antispam.BlacklistNotifyCooldown", 10);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_BLACKLIST_SILENCE, "Antispam.BlacklistSilence", 15);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_BLACKLIST_LEVEL, "Antispam.BlacklistLevel", 5);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_CACHE_EXPIRATION, "Antispam.CacheExpiration", 3600);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_UNIQUENESS_THRESHOLD, "Antispam.UniquenessThreshold", 5);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_MIN_REPETITION_MESSAGES, "Antispam.MinRepetitionMessages", 5);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_REPETITION_NOTIFY, "Antispam.RepetitionNotify", 50);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_REPETITION_SILENCE, "Antispam.RepetitionSilence", 0);
    setConfig(CONFIG_UINT32_AC_ANTISPAM_REPETITION_MOVEMENT_TIMEOUT, "Antispam.RepetitionMovementTimeout", 60);

    setConfig(CONFIG_FLOAT_AC_ANTISPAM_REPETITION_DISTANCE_SCALE, "Antispam.RepetitionDistanceScale", 1.f);
    setConfig(CONFIG_FLOAT_AC_ANTISPAM_REPETITION_TIME_SCALE, "Antispam.RepetitionTimeScale", 0.f);

    for (auto i = 0; i < CHEATS_COUNT; ++i)
    {
        std::stringstream tickCount, tickAction, totalCount, totalAction;

        // warden parameters are named differently
        if (i == CHEAT_TYPE_WARDEN)
        {
            tickCount << GetDetectorName(static_cast<CheatType>(i)) << ".TickCount";
            tickAction <<GetDetectorName(static_cast<CheatType>(i)) << ".TickAction";
            totalCount << GetDetectorName(static_cast<CheatType>(i)) << ".TotalCount";
            totalAction << GetDetectorName(static_cast<CheatType>(i)) << ".TotalAction";
        }
        else
        {
            tickCount << "Movement." << GetDetectorName(static_cast<CheatType>(i)) << ".TickCount";
            tickAction << "Movement." << GetDetectorName(static_cast<CheatType>(i)) << ".TickAction";
            totalCount << "Movement." << GetDetectorName(static_cast<CheatType>(i)) << ".TotalCount";
            totalAction << "Movement." << GetDetectorName(static_cast<CheatType>(i)) << ".TotalAction";
        }

        _responseRules[i] =
        {
            static_cast<uint32>(GetIntDefault(tickCount.str().c_str(), 0)),
            static_cast<uint32>(GetIntDefault(tickAction.str().c_str(), 0)),
            static_cast<uint32>(GetIntDefault(totalCount.str().c_str(), 0)),
            static_cast<uint32>(GetIntDefault(totalAction.str().c_str(), 0))
        };
    }

    setConfig(CONFIG_UINT32_AC_WARDEN_TIMEOUT, "Warden.Timeout", 30);
    setConfig(CONFIG_UINT32_AC_WARDEN_SCAN_FREQUENCY, "Warden.ScanFrequency", 15);
    setConfig(CONFIG_UINT32_AC_WARDEN_SCAN_COUNT, "Warden.ScanCount", 10);
    setConfig(CONFIG_UINT32_AC_WARDEN_MINIMUM_LEVEL, "Warden.MinimumLevel", 25);
    setConfig(CONFIG_UINT32_AC_WARDEN_MINIMUM_ADVANCED_LEVEL, "Warden.MinimumAdvancedLevel", 18);
    setConfig(CONFIG_UINT32_AC_WARDEN_SUSPICIOUS_ENDSCENE_HOOK_ACTION, "Warden.SuspiciousEndSceneHookAction", 1);
    setConfig(CONFIG_BOOL_AC_MOVEMENT_CHEAT_BAD_FALL_RESET_ENABLED, "Movement.BadFallReset.Enable", true);
    setConfig(CONFIG_UINT32_AC_MOVEMENT_CHEAT_BAD_FALL_RESET_THRESHOLD, "Movement.BadFallReset.Threshold", 1);
    setConfig(CONFIG_UINT32_AC_MOVEMENT_CHEAT_BAD_FALL_RESET_PENALTY, "Movement.BadFallReset.Penalty", CHEAT_ACTION_INFO_LOG | CHEAT_ACTION_PROMPT_LOG | CHEAT_ACTION_KICK);
    setConfig(CONFIG_BOOL_AC_WARDEN_ENABLED, "Warden.Enable", false);
    m_wardenModuleDir = GetStringDefault("Warden.ModuleDir", "warden_modules");
}

bool AnticheatConfig::CheckResponse(CheatType cheatType, uint32 tickCount, uint32 totalCount, uint32 &actionMask) const
{
    MANGOS_ASSERT(cheatType < CHEATS_COUNT);

    actionMask = CHEAT_ACTION_NONE;

    // too many for this tick?  append these actions
    if (tickCount && _responseRules[cheatType].tickCount &&
        tickCount >= _responseRules[cheatType].tickCount)
        actionMask |= _responseRules[cheatType].tickAction;

    // too many total?  append these actions
    if (totalCount && _responseRules[cheatType].totalCount &&
        totalCount >= _responseRules[cheatType].totalCount)
        actionMask |= _responseRules[cheatType].totalAction;

    return actionMask != CHEAT_ACTION_NONE;
}

uint32 AnticheatConfig::GetKickDelay() const
{
    auto const minDelay = getConfig(CONFIG_UINT32_AC_KICK_DELAY_MIN);
    auto const maxDelay = getConfig(CONFIG_UINT32_AC_KICK_DELAY_MAX);

    return urand(minDelay*1000, maxDelay*1000);
}

uint32 AnticheatConfig::GetBanDelay() const
{
    auto const minDelay = getConfig(CONFIG_UINT32_AC_BAN_DELAY_MIN);
    auto const maxDelay = getConfig(CONFIG_UINT32_AC_BAN_DELAY_MAX);

    return urand(minDelay * 1000, maxDelay * 1000);
}

uint32 AnticheatConfig::GetIPBanDelay() const
{
    auto const minDelay = getConfig(CONFIG_UINT32_AC_IP_BAN_DELAY_MIN);
    auto const maxDelay = getConfig(CONFIG_UINT32_AC_IP_BAN_DELAY_MAX);

    return urand(minDelay * 1000, maxDelay * 1000);
}

void AnticheatConfig::GetBanWaveTime(uint32 &day, uint32 &hour, uint32 &minute) const
{
    day = getConfig(CONFIG_UINT32_AC_BAN_WAVE_DAY);
    hour = getConfig(CONFIG_UINT32_AC_BAN_WAVE_HOUR);
    minute = getConfig(CONFIG_UINT32_AC_BAN_WAVE_DAY);
}

const char *AnticheatConfig::GetDetectorName(CheatType cheatType)
{
    MANGOS_ASSERT(cheatType < CHEATS_COUNT);

    return sCheatTypeNames[cheatType];
}
}