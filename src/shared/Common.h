/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MANGOSSERVER_COMMON_H
#define MANGOSSERVER_COMMON_H

#include "Platform/Define.h"

#if COMPILER == COMPILER_MICROSOFT
#  pragma warning(disable:4996)                             // 'function': was declared deprecated
#ifndef __SHOW_STUPID_WARNINGS__
#  pragma warning(disable:4244)                             // 'argument' : conversion from 'type1' to 'type2', possible loss of data
#  pragma warning(disable:4355)                             // 'this' : used in base member initializer list
#endif                                                      // __SHOW_STUPID_WARNINGS__
#endif                                                      // __GNUC__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cmath>
#include <errno.h>
#include <signal.h>
#include <assert.h>

#if defined(__sun__)
#include <ieeefp.h> // finite() on Solaris
#endif

#include <set>
#include <list>
#include <string>
#include <map>
#include <queue>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "Errors.h"
#include "Threading.h"

#if COMPILER == COMPILER_MICROSOFT

#  include <float.h>

#  define I32FMT "%08I32X"
#  define I64FMT "%016I64X"
#  define snprintf _snprintf
#  define vsnprintf _vsnprintf
#  define finite(X) _finite(X)

#else

#  define stricmp strcasecmp
#  define strnicmp strncasecmp

#  define I32FMT "%08X"
#  if ACE_SIZEOF_LONG == 8
#    define I64FMT "%016lX"
#  else
#    define I64FMT "%016llX"
#  endif /* ACE_SIZEOF_LONG == 8 */

#endif

#define __STDC_FORMAT_MACROS
#include <cinttypes>

#define UI64FMTD "%" PRIu64

#define SI64FMTD "%" PRId64

#if COMPILER == COMPILER_MICROSOFT
#  define SIZEFMTD "%Iu"
#else
#  define SIZEFMTD "%zu"
#endif

inline float finiteAlways(float f) { return std::isfinite(f) ? f : 0.0f; }

#define STRINGIZE(a) #a

// used for creating values for respawn for example
#define MAKE_PAIR64(l, h)  uint64( uint32(l) | ( uint64(h) << 32 ) )
#define PAIR64_HIPART(x)   (uint32)((uint64(x) >> 32) & uint64(0x00000000FFFFFFFF))
#define PAIR64_LOPART(x)   (uint32)(uint64(x)         & uint64(0x00000000FFFFFFFF))

#define MAKE_PAIR32(l, h)  uint32( uint16(l) | ( uint32(h) << 16 ) )
#define PAIR32_HIPART(x)   (uint16)((uint32(x) >> 16) & 0x0000FFFF)
#define PAIR32_LOPART(x)   (uint16)(uint32(x)         & 0x0000FFFF)

enum TimeConstants
{
    MINUTE = 60,
    HOUR   = MINUTE * 60,
    DAY    = HOUR * 24,
    WEEK   = DAY * 7,
    MONTH  = DAY * 30,
    YEAR   = MONTH * 12,
    IN_MILLISECONDS = 1000
};

enum AccountTypes
{
    SEC_PLAYER         = 0,
    SEC_MODERATOR      = 1,
    SEC_GAMEMASTER     = 2,
    SEC_ADMINISTRATOR  = 3,
    SEC_CONSOLE        = 4                                  // must be always last in list, accounts must have less security level always also
};

// Used in mangosd/realmd
enum RealmFlags
{
    REALM_FLAG_NONE         = 0x00,
    REALM_FLAG_INVALID      = 0x01,
    REALM_FLAG_OFFLINE      = 0x02,
    REALM_FLAG_SPECIFYBUILD = 0x04,                         // client will show realm version in RealmList screen in form "RealmName (major.minor.revision.build)"
    REALM_FLAG_UNK1         = 0x08,
    REALM_FLAG_UNK2         = 0x10,
    REALM_FLAG_NEW_PLAYERS  = 0x20,
    REALM_FLAG_RECOMMENDED  = 0x40,
    REALM_FLAG_FULL         = 0x80
};

enum LocaleConstant
{
    LOCALE_enUS = 0,                                        // also enGB
    LOCALE_koKR = 1,
    LOCALE_frFR = 2,
    LOCALE_deDE = 3,
    LOCALE_zhCN = 4,
    LOCALE_zhTW = 5,
    LOCALE_esES = 6,
    LOCALE_esMX = 7,
};

#define MAX_LOCALE 8

LocaleConstant GetLocaleByName(const std::string& name);

extern char const* localeNames[MAX_LOCALE];

struct LocaleNameStr
{
    char const* name;
    LocaleConstant locale;
};

// used for iterate all names including alternative
extern LocaleNameStr const fullLocaleNameList[];

// operator new[] based version of strdup() function! Release memory by using operator delete[] !
inline char* mangos_strdup(const char* source)
{
    char* dest = new char[strlen(source) + 1];
    strcpy(dest, source);
    return dest;
}

// we always use stdlibc++ std::max/std::min, undefine some not C++ standard defines (Win API and some pother platforms)
#ifdef max
#  undef max
#endif

#ifdef min
#  undef min
#endif

#ifndef M_PI
#  define M_PI          3.14159265358979323846
#endif

#ifndef M_PI_F
#  define M_PI_F        float(M_PI)
#endif

#ifndef countof
#define countof(array) (sizeof(array) / sizeof((array)[0]))
#endif

#endif
