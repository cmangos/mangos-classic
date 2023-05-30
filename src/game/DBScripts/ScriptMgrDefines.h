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

#ifndef _SCRIPTMGRDEF_H
#define _SCRIPTMGRDEF_H

#include <string>
#include <map>

enum ScriptMapType
{
    SCRIPT_TYPE_QUEST_END           = 0,
    SCRIPT_TYPE_QUEST_START         = 1,
    SCRIPT_TYPE_SPELL               = 2,
    SCRIPT_TYPE_GAMEOBJECT          = 3,
    SCRIPT_TYPE_GAMEOBJECT_TEMPLATE = 4,
    SCRIPT_TYPE_EVENT               = 5,
    SCRIPT_TYPE_GOSSIP              = 6,
    SCRIPT_TYPE_CREATURE_DEATH      = 7,
    SCRIPT_TYPE_CREATURE_MOVEMENT   = 8,
    SCRIPT_TYPE_RELAY               = 9,
    SCRIPT_TYPE_INTERNAL            = 10, // must be last
    SCRIPT_TYPE_MAX
};

struct StringId
{
    int32 Id;
    std::string Name;
};

typedef std::map<int32, StringId> StringIdMap;
typedef std::map<std::string, StringId> StringIdMapByString;

#endif