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

#ifndef MANGOS_SYSTEMCONFIG_H
#define MANGOS_SYSTEMCONFIG_H

#ifndef _PACKAGENAME
#define _PACKAGENAME "CMaNGOS"
#endif

#include "revision.h"
#include "Platform/Define.h"

#ifndef VERSION
#error "VERSION undeclared"
#endif

#ifndef _VERSION
# define _VERSION(REVD,REVH) VERSION " (" REVD " - " REVH ")"
#endif

// Format is YYYYMMDDRR where RR is the change in the conf file
// for that day.
#ifndef _MANGOSDCONFVERSION
# define _MANGOSDCONFVERSION 2022012301
#endif
#ifndef _REALMDCONFVERSION
# define _REALMDCONFVERSION 2021031501
#endif

#if MANGOS_ENDIAN == MANGOS_BIG_ENDIAN
# define _ENDIAN_STRING "big-endian"
#else
# define _ENDIAN_STRING "little-endian"
#endif

#if defined(i386) || defined(__i386) || defined(__i386__) || defined(_M_IX86)
# define ARCHITECTURE "x32"
#elif defined(__amd64) || defined(__amd64__) || defined(__x86_64) || defined(_M_X64)
# define ARCHITECTURE "x64"
#elif defined(__ia64)  || defined(__IA64__)  || defined(_M_IA64)
# define ARCHITECTURE "IA64"
#elif defined(__aarch64__)
# define ARCHITECTURE "AArch64"
#elif defined(__arm__)
# define ARCHITECTURE "ARM32"
#else
# define ARCHITECTURE "x32"
#endif

// The path to config files
#ifndef SYSCONFDIR
#  define SYSCONFDIR        ""
#endif

#if PLATFORM == PLATFORM_WINDOWS
# ifdef _WIN64
#  define _ENDIAN_PLATFORM "Win64 (" _ENDIAN_STRING ")"
# else
#  define _ENDIAN_PLATFORM "Win32 (" _ENDIAN_STRING ")"
# endif
# define _MANGOSD_CONFIG  SYSCONFDIR"mangosd.conf"
# define _REALMD_CONFIG   SYSCONFDIR"realmd.conf"
# define _AUCTIONHOUSEBOT_CONFIG   SYSCONFDIR"ahbot.conf"
#else
# if defined  (__FreeBSD__)
#  define _ENDIAN_PLATFORM "FreeBSD_" ARCHITECTURE " (" _ENDIAN_STRING ")"
# elif defined(__NetBSD__)
#  define _ENDIAN_PLATFORM "NetBSD_" ARCHITECTURE " (" _ENDIAN_STRING ")"
# elif defined(__OpenBSD__)
#  define _ENDIAN_PLATFORM "OpenBSD_" ARCHITECTURE " (" _ENDIAN_STRING ")"
# elif defined(__DragonFly__)
#  define _ENDIAN_PLATFORM "DragonFlyBSD_" ARCHITECTURE " (" _ENDIAN_STRING ")"
# elif defined(__APPLE__)
#  define _ENDIAN_PLATFORM "MacOSX_" ARCHITECTURE " (" _ENDIAN_STRING ")"
# elif defined(__linux) || defined(__linux__)
#  define _ENDIAN_PLATFORM "Linux_" ARCHITECTURE " (" _ENDIAN_STRING ")"
# else
#  define _ENDIAN_PLATFORM "Unix_" ARCHITECTURE " (" _ENDIAN_STRING ")"
# endif
# define _MANGOSD_CONFIG  SYSCONFDIR"mangosd.conf"
# define _REALMD_CONFIG  SYSCONFDIR"realmd.conf"
# define _AUCTIONHOUSEBOT_CONFIG   SYSCONFDIR"ahbot.conf"
#endif

#define _FULLVERSION(REVD,REVH) _PACKAGENAME "/" _VERSION(REVD,REVH) " for " _ENDIAN_PLATFORM

#define DEFAULT_PLAYER_LIMIT 100
#define DEFAULT_WORLDSERVER_PORT 8085                       //8129
#define DEFAULT_REALMSERVER_PORT 3724
#endif
