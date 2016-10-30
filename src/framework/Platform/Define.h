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

#ifndef MANGOS_DEFINE_H
#define MANGOS_DEFINE_H

#include "Platform/CompilerDefs.h"

#include <cstdint>

#include <sys/types.h>

#define MANGOS_LITTLEENDIAN 0
#define MANGOS_BIGENDIAN    1

#if !defined(MANGOS_ENDIAN)
#  if defined (ACE_BIG_ENDIAN)
#    define MANGOS_ENDIAN MANGOS_BIGENDIAN
#  else // ACE_BYTE_ORDER != ACE_BIG_ENDIAN
#    define MANGOS_ENDIAN MANGOS_LITTLEENDIAN
#  endif // ACE_BYTE_ORDER
#endif // MANGOS_ENDIAN

#define MANGOS_SCRIPT_NAME "mangosscript"
#define MANGOS_PATH_MAX 1024

#if PLATFORM == PLATFORM_WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  include <Windows.h>
#  ifndef _WIN32_WINNT
#    define _WIN32_WINNT 0x0603
#  endif
typedef HMODULE MANGOS_LIBRARY_HANDLE;
#  define MANGOS_SCRIPT_SUFFIX ".dll"
#  define MANGOS_SCRIPT_PREFIX ""
#  define MANGOS_LOAD_LIBRARY(libname)     LoadLibraryA(libname)
#  define MANGOS_CLOSE_LIBRARY(hlib)       FreeLibrary(hlib)
#  define MANGOS_GET_PROC_ADDR(hlib, name) GetProcAddress(hlib, name)
#  define MANGOS_EXPORT __declspec(dllexport)
#  define MANGOS_IMPORT __cdecl
#else // PLATFORM != PLATFORM_WINDOWS
#  include <dlfcn.h>
typedef void* MANGOS_LIBRARY_HANDLE;
#  define MANGOS_LOAD_LIBRARY(libname)     dlopen(libname, RTLD_LAZY)
#  define MANGOS_CLOSE_LIBRARY(hlib)       dlclose(hlib)
#  define MANGOS_GET_PROC_ADDR(hlib, name) dlsym(hlib, name)
#  define MANGOS_EXPORT export
#  if PLATFORM == PLATFORM_APPLE
#    define MANGOS_SCRIPT_SUFFIX ".dylib"
#  else
#    define MANGOS_SCRIPT_SUFFIX ".so"
#  endif
#  define MANGOS_SCRIPT_PREFIX "lib"
#  if defined(__APPLE_CC__) && defined(BIG_ENDIAN) // TODO:: more work to do with byte order. Have to be rechecked after boost integration.
#    if (defined (__ppc__) || defined (__powerpc__))
#      define MANGOS_IMPORT __attribute__ ((longcall))
#    else
#      define MANGOS_IMPORT
#    endif
#  elif defined(__x86_64__)
#    define MANGOS_IMPORT
#  else
#    define MANGOS_IMPORT __attribute__ ((cdecl))
#  endif //__APPLE_CC__ && BIG_ENDIAN
#endif // PLATFORM

#if PLATFORM == PLATFORM_WINDOWS
#  define MANGOS_DLL_SPEC __declspec(dllexport)
#  ifndef DECLSPEC_NORETURN
#    define DECLSPEC_NORETURN __declspec(noreturn)
#  endif // DECLSPEC_NORETURN
#else // PLATFORM != PLATFORM_WINDOWS
#  define MANGOS_DLL_SPEC
#  define DECLSPEC_NORETURN
#endif // PLATFORM

#if !defined(DEBUG)
#  define MANGOS_INLINE inline
#else // DEBUG
#  if !defined(MANGOS_DEBUG)
#    define MANGOS_DEBUG
#  endif // MANGOS_DEBUG
#  define MANGOS_INLINE
#endif //!DEBUG

#if COMPILER == COMPILER_GNU
#  define ATTR_NORETURN __attribute__((noreturn))
#  define ATTR_PRINTF(F,V) __attribute__ ((format (printf, F, V)))
#else // COMPILER != COMPILER_GNU
#  define ATTR_NORETURN
#  define ATTR_PRINTF(F,V)
#endif // COMPILER == COMPILER_GNU

typedef std::int64_t int64;
typedef std::int32_t int32;
typedef std::int16_t int16;
typedef std::int8_t int8;
typedef std::uint64_t uint64;
typedef std::uint32_t uint32;
typedef std::uint16_t uint16;
typedef std::uint8_t uint8;

#if COMPILER == COMPILER_GNU
#  if !defined(__GXX_EXPERIMENTAL_CXX0X__) || (__GNUC__ < 4) || (__GNUC__ == 4) && (__GNUC_MINOR__ < 7)
#    define override
#  endif
#endif

typedef uint64 OBJECT_HANDLE;

#endif // MANGOS_DEFINE_H
