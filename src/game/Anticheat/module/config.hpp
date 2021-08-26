#ifndef __CONFIG_HPP_
#define __CONFIG_HPP_

#include "Anticheat/Anticheat.hpp"
#include "libanticheat.hpp"
#include "Config/Config.h"
#include "Platform/Define.h"
#include "Policies/Singleton.h"

#include <string>

namespace NamreebAnticheat
{
enum AnticheatConfigUInt32Values
{
    CONFIG_UINT32_AC_ANTISPAM_MAX_LEVEL = 0,
    CONFIG_UINT32_AC_ANTISPAM_NORMALIZE_MASK,
    CONFIG_UINT32_AC_ANTISPAM_ANALYSIS_TIMER,
    CONFIG_UINT32_AC_ANTISPAM_MAX_RATE,
    CONFIG_UINT32_AC_ANTISPAM_RATE_GRACE_PERIOD,
    CONFIG_UINT32_AC_ANTISPAM_MAX_UNIQUE_PERCENTAGE,
    CONFIG_UINT32_AC_ANTISPAM_MIN_UNIQUE_MESSAGES,
    CONFIG_UINT32_AC_ANTISPAM_BLACKLIST_NOTIFY,
    CONFIG_UINT32_AC_ANTISPAM_BLACKLIST_NOTIFY_COOLDOWN,
    CONFIG_UINT32_AC_ANTISPAM_BLACKLIST_SILENCE,
    CONFIG_UINT32_AC_ANTISPAM_BLACKLIST_LEVEL,
    CONFIG_UINT32_AC_ANTISPAM_CACHE_EXPIRATION,
    CONFIG_UINT32_AC_ANTISPAM_UNIQUENESS_THRESHOLD,
    CONFIG_UINT32_AC_ANTISPAM_MIN_REPETITION_MESSAGES,
    CONFIG_UINT32_AC_ANTISPAM_REPETITION_NOTIFY,
    CONFIG_UINT32_AC_ANTISPAM_REPETITION_SILENCE,
    CONFIG_UINT32_AC_ANTISPAM_REPETITION_MOVEMENT_TIMEOUT,
    CONFIG_UINT32_AC_FINGERPRINT_HISTORY,
    CONFIG_UINT32_AC_FINGERPRINT_LEVEL,
    CONFIG_UINT32_AC_KICK_DELAY_MIN,
    CONFIG_UINT32_AC_KICK_DELAY_MAX,
    CONFIG_UINT32_AC_BAN_DELAY_MIN,
    CONFIG_UINT32_AC_BAN_DELAY_MAX,
    CONFIG_UINT32_AC_IP_BAN_DELAY_MIN,
    CONFIG_UINT32_AC_IP_BAN_DELAY_MAX,
    CONFIG_UINT32_AC_BAN_WAVE_DAY,
    CONFIG_UINT32_AC_BAN_WAVE_HOUR,
    CONFIG_UINT32_AC_BAN_WAVE_MINUTE,
    CONFIG_UINT32_AC_WARDEN_TIMEOUT,
    CONFIG_UINT32_AC_WARDEN_SCAN_FREQUENCY,
    CONFIG_UINT32_AC_WARDEN_SCAN_COUNT,
    CONFIG_UINT32_AC_WARDEN_MINIMUM_LEVEL,
    CONFIG_UINT32_AC_WARDEN_MINIMUM_ADVANCED_LEVEL,
    CONFIG_UINT32_AC_WARDEN_SUSPICIOUS_ENDSCENE_HOOK_ACTION,
    CONFIG_UINT32_AC_COUNT
};

enum AnticheatConfigBoolValues
{
    CONFIG_BOOL_AC_ENABLED = 0,
    CONFIG_BOOL_AC_MOVEMENT_USE_EXTRAPOLATION,
    CONFIG_BOOL_AC_MOVEMENT_SPEED_HACK_ENABLED,
    CONFIG_BOOL_AC_ANTISPAM_ENABLED,
    CONFIG_BOOL_AC_ANTISPAM_SILENCE,
    CONFIG_BOOL_AC_WARDEN_ENABLED,
    CONFIG_BOOL_AC_COUNT
};

enum AnticheatConfigFloatValues
{
    CONFIG_FLOAT_AC_ANTISPAM_REPETITION_DISTANCE_SCALE = 0,
    CONFIG_FLOAT_AC_ANTISPAM_REPETITION_TIME_SCALE,
    CONFIG_FLOAT_AC_COUNT
};

struct CheatResponseRule
{
    uint32 tickCount;       // how many occurances must happen in one tick
    uint32 tickAction;      // bitmask of CheatAction to take when the tick occurance count is high enough

    uint32 totalCount;      // how many total occurances must happen to trigger an action
    uint32 totalAction;     // bitmask of CheatAction to take when the total occurance count is high enough
};

class AnticheatConfig : public Config
{
    private:
        uint32  m_configUInt32Values[CONFIG_UINT32_AC_COUNT];
        bool    m_configBoolValues[CONFIG_BOOL_AC_COUNT];
        float   m_configFloatValues[CONFIG_FLOAT_AC_COUNT];

        std::string m_wardenModuleDir;

        CheatResponseRule _responseRules[CHEATS_COUNT];

        void setConfig(AnticheatConfigUInt32Values index, uint32 value) { m_configUInt32Values[index] = value; }
        void setConfig(AnticheatConfigBoolValues index, bool value)     { m_configBoolValues[index] = value;   }
        void setConfig(AnticheatConfigFloatValues index, float value)   { m_configFloatValues[index] = value; }

        void setConfig(AnticheatConfigUInt32Values index, char const* fieldname, uint32 defvalue);
        void setConfig(AnticheatConfigBoolValues index, char const* fieldname, bool defvalue);
        void setConfig(AnticheatConfigFloatValues index, char const* fieldname, float defvalue);

    public:
        AnticheatConfig();

        void loadConfigSettings();

        uint32 getConfig(AnticheatConfigUInt32Values index) const { return m_configUInt32Values[index]; }
        bool getConfig(AnticheatConfigBoolValues index)     const { return m_configBoolValues[index];   }
        float getConfig(AnticheatConfigFloatValues index)   const { return m_configFloatValues[index];  }

        bool EnableAnticheat()                      const { return getConfig(CONFIG_BOOL_AC_ENABLED);                               }
        bool EnableExtrapolation()                  const { return getConfig(CONFIG_BOOL_AC_MOVEMENT_USE_EXTRAPOLATION);            }

        bool IsEnabled(CheatType cheatType) const
        {
            // only enabled when one (or both) of the tick and total rules have a non-zero threshold AND an action
            return (!!_responseRules[cheatType].tickCount && _responseRules[cheatType].tickAction != CHEAT_ACTION_NONE) ||
                (!!_responseRules[cheatType].totalCount && _responseRules[cheatType].totalAction != CHEAT_ACTION_NONE);
        }

        bool EnableAntiSpeedHack()                      const { return getConfig(CONFIG_BOOL_AC_MOVEMENT_SPEED_HACK_ENABLED);               }
        uint32 GetFingerprintHistory()                  const { return getConfig(CONFIG_UINT32_AC_FINGERPRINT_HISTORY);                     }
        uint32 GetFingerprintLevel()                    const { return getConfig(CONFIG_UINT32_AC_FINGERPRINT_LEVEL);                       }

        uint32 GetWardenTimeout()                       const { return getConfig(CONFIG_UINT32_AC_WARDEN_TIMEOUT);                          }
        uint32 GetWardenScanFrequency()                 const { return getConfig(CONFIG_UINT32_AC_WARDEN_SCAN_FREQUENCY);                   }
        uint32 GetWardenScanCount()                     const { return getConfig(CONFIG_UINT32_AC_WARDEN_SCAN_COUNT);                       }
        uint32 GetWardenMinimumLevel()                  const { return getConfig(CONFIG_UINT32_AC_WARDEN_MINIMUM_LEVEL);                    }
        uint32 GetWardenMinimumAdvancedLevel()          const { return getConfig(CONFIG_UINT32_AC_WARDEN_MINIMUM_ADVANCED_LEVEL);           }
        uint32 GetWardenSuspiciousEndSceneHookAction()  const { return getConfig(CONFIG_UINT32_AC_WARDEN_SUSPICIOUS_ENDSCENE_HOOK_ACTION);  }
        std::string GetWardenModuleDirectory()          const { return m_wardenModuleDir;                                                   }

        // returns true when there is an action to perform
        bool CheckResponse(CheatType cheatType, uint32 tickCount, uint32 totalCount, uint32 &actionMask) const;

        uint32 GetKickDelay() const;
        uint32 GetBanDelay() const;
        uint32 GetIPBanDelay() const;

        void GetBanWaveTime(uint32 &day, uint32 &hour, uint32 &minute) const;

        // antispam
        bool EnableAntispam()                           const { return getConfig(CONFIG_BOOL_AC_ANTISPAM_ENABLED);                          }
        bool EnableAntispamSilence()                    const { return getConfig(CONFIG_BOOL_AC_ANTISPAM_SILENCE);                          }
        uint32 GetSpamNormalizationMask()               const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_NORMALIZE_MASK);                 }
        uint32 GetAntispamAnalysisTimer()               const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_ANALYSIS_TIMER);                 }
        uint32 GetAntispamMaxLevel()                    const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_MAX_LEVEL);                      }
        uint32 GetAntispamMaxRate()                     const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_MAX_RATE);                       }
        uint32 GetAntispamMaxUniquePercentage()         const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_MAX_UNIQUE_PERCENTAGE);          }
        uint32 GetAntispamMinUniqueMessages()           const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_MIN_UNIQUE_MESSAGES);            }
        uint32 GetAntispamRateGracePeriod()             const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_RATE_GRACE_PERIOD);              }
        uint32 GetAntispamBlacklistNotify()             const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_BLACKLIST_NOTIFY);               }
        uint32 GetAntispamBlacklistNotifyCooldown()     const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_BLACKLIST_NOTIFY_COOLDOWN);      }
        uint32 GetAntispamBlacklistSilence()            const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_BLACKLIST_SILENCE);              }
        uint32 GetAntispamBlacklistLevel()              const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_BLACKLIST_LEVEL);                }
        uint32 GetAntispamCacheExpiration()             const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_CACHE_EXPIRATION);               }
        uint32 GetAntispamUniquenessThreshold()         const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_UNIQUENESS_THRESHOLD);           }
        uint32 GetAntispamMinRepetitionMessages()       const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_MIN_REPETITION_MESSAGES);        }
        uint32 GetAntispamRepetitionNotify()            const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_REPETITION_NOTIFY);              }
        uint32 GetAntispamRepetitionSilence()           const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_REPETITION_SILENCE);             }
        uint32 GetAntispamRepetitionMovementTimeout()   const { return getConfig(CONFIG_UINT32_AC_ANTISPAM_REPETITION_MOVEMENT_TIMEOUT);    }
        float GetAntispamRepetitionDistanceScale()      const { return getConfig(CONFIG_FLOAT_AC_ANTISPAM_REPETITION_DISTANCE_SCALE);       }
        float GetAntispamRepetitionTimeScale()          const { return getConfig(CONFIG_FLOAT_AC_ANTISPAM_REPETITION_TIME_SCALE);           }

        static const char *GetDetectorName(CheatType cheatType);
};

#define sAnticheatConfig MaNGOS::Singleton<NamreebAnticheat::AnticheatConfig>::Instance()
}

#endif /*!__CONFIG_HPP_*/