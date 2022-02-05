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

#ifndef MANGOSSERVER_FILESYSTEM_H
#define MANGOSSERVER_FILESYSTEM_H

// Credits:
// Original solution for std::filesystem resolution by BrainStone
// https://stackoverflow.com/a/53365539

#if defined(__cpp_lib_filesystem)
    #define MANGOS_STD_FILESYSTEM_EXPERIMENTAL 0
#elif defined(__cpp_lib_experimental_filesystem)
    #define MANGOS_STD_FILESYSTEM_EXPERIMENTAL 1
#elif !defined(__has_include)
    #define MANGOS_STD_FILESYSTEM_EXPERIMENTAL 1
#elif __has_include(<filesystem>)
    #ifdef _MSC_VER
        #if __has_include(<yvals_core.h>)
            #include <yvals_core.h>
            #if defined(_HAS_CXX17) && _HAS_CXX17
                #define MANGOS_STD_FILESYSTEM_EXPERIMENTAL 0
            #endif
        #endif
        #ifndef MANGOS_STD_FILESYSTEM_EXPERIMENTAL
            #define MANGOS_STD_FILESYSTEM_EXPERIMENTAL 1
        #endif
    #else // #ifdef _MSC_VER
        #define MANGOS_STD_FILESYSTEM_EXPERIMENTAL 0
    #endif
#elif __has_include(<experimental/filesystem>)
    #define MANGOS_STD_FILESYSTEM_EXPERIMENTAL 1
#else
    #error Could not find system header "<filesystem>" or "<experimental/filesystem>"
#endif

#if MANGOS_STD_FILESYSTEM_EXPERIMENTAL
    #include <experimental/filesystem>
    namespace MaNGOS
    {
        namespace Filesystem = std::experimental::filesystem;
    }
#else
    #include <filesystem>
    namespace MaNGOS
    {
        namespace Filesystem = std::filesystem;
    }
#endif

#endif
