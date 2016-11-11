/*
 * This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
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

#ifndef SC_CONFIG_H
#define SC_CONFIG_H

#include "SystemConfig.h"
#include "Platform/CompilerDefs.h"
#include "revision.h"
#include "sd2_revision_nr.h"

#ifdef WIN32
   #define MANGOS_DLL_EXPORT extern "C" __declspec(dllexport)
#elif defined( __GNUC__ )
   #define MANGOS_DLL_EXPORT extern "C"
#else
   #define MANGOS_DLL_EXPORT extern "C" export
#endif

#ifndef _SD2VERSION
   #define _SD2VERSION "Revision [" SD2_REVISION_NR "] (" REVISION_ID ") " REVISION_DATE " " REVISION_TIME
#endif

#if PLATFORM == PLATFORM_WINDOWS
   #ifdef _WIN64
       #define _FULLSD2VERSION _SD2VERSION " (Win64)"
   #else
       #define _FULLSD2VERSION _SD2VERSION " (Win32)"
   #endif
#else
       #define _FULLSD2VERSION _SD2VERSION " (Unix)"
#endif




#endif
