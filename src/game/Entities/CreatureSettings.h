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

#ifndef MANGOSSERVER_CREATURE_SETTINGS_H
#define MANGOSSERVER_CREATURE_SETTINGS_H
#include "Entities/CreatureDefines.h"

class Creature;

class CreatureSettings
{
    public:
        CreatureSettings(Creature* owner);
        void ResetStaticFlags(CreatureStaticFlags staticFlags, CreatureStaticFlags2 staticFlags2, CreatureStaticFlags3 staticFlags3, CreatureStaticFlags4 staticFlags4);
        // always set/remove one at a time
        void SetFlag(CreatureStaticFlags flag);
        void SetFlag(CreatureStaticFlags2 flag);
        void SetFlag(CreatureStaticFlags3 flag);
        void SetFlag(CreatureStaticFlags4 flag);
        void RemoveFlag(CreatureStaticFlags flag);
        void RemoveFlag(CreatureStaticFlags2 flag);
        void RemoveFlag(CreatureStaticFlags3 flag);
        void RemoveFlag(CreatureStaticFlags4 flag);

        bool HasFlag(CreatureStaticFlags flag) const;
        bool HasFlag(CreatureStaticFlags2 flag) const;
        bool HasFlag(CreatureStaticFlags3 flag) const;
        bool HasFlag(CreatureStaticFlags4 flag) const;
    private:
        void updateFlag(CreatureStaticFlags flag, bool apply);
        void updateFlag(CreatureStaticFlags2 flag, bool apply);
        void updateFlag(CreatureStaticFlags3 flag, bool apply);
        void updateFlag(CreatureStaticFlags4 flag, bool apply);

        CreatureStaticFlags m_flags;
        CreatureStaticFlags2 m_flags2;
        CreatureStaticFlags3 m_flags3;
        CreatureStaticFlags4 m_flags4;
        Creature* m_owner;
};

#endif