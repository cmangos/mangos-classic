/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
* This program is free software licensed under GPL version 2
* Please see the included DOCS/LICENSE.TXT for more information */

#ifdef WIN32
#   define MANGOS_DLL_EXPORT extern "C" __declspec(dllexport)
#elif defined( __GNUC__ )
#   define MANGOS_DLL_EXPORT extern "C"
#else
#   define MANGOS_DLL_EXPORT extern "C" export
#endif
