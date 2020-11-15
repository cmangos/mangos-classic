#include "Locales.h"

char const* localeNames[MAX_LOCALE] =
{
    "enUS",                                                 // also enGB
    "koKR",
    "frFR",
    "deDE",
    "zhCN",
    "zhTW",
    "esES",
};

// used for search by name or iterate all names
LocaleNameStr const fullLocaleNameList[] =
{
    { "enUS",   LOCALE_enUS },
    { "enGB",   LOCALE_enUS },
    { "koKR",   LOCALE_koKR },
    { "frFR",   LOCALE_frFR },
    { "deDE",   LOCALE_deDE },
    { "zhCN",   LOCALE_zhCN },
    { "zhTW",   LOCALE_zhTW },
    { "esES",   LOCALE_esES },
    { nullptr,  LOCALE_enUS }
};

LocaleConstant GetLocaleByName(const std::string& name)
{
    for (LocaleNameStr const* itr = &fullLocaleNameList[0]; itr->name; ++itr)
        if (name == itr->name)
            return itr->locale;

    return LOCALE_enUS;                                     // including enGB case
}
