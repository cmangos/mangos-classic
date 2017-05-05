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

#ifndef MANGOS_GRIDSTATES_H
#define MANGOS_GRIDSTATES_H

#include "Maps/Map.h"

class GridState
{
    public:

        virtual void Update(Map&, NGridType&, GridInfo&, const uint32& x, const uint32& y, const uint32& t_diff) const = 0;
        virtual ~GridState() {}
};

class InvalidState : public GridState
{
    public:

        void Update(Map&, NGridType&, GridInfo&, const uint32& x, const uint32& y, const uint32& t_diff) const override;
        virtual ~InvalidState() {}
};

class ActiveState : public GridState
{
    public:

        void Update(Map&, NGridType&, GridInfo&, const uint32& x, const uint32& y, const uint32& t_diff) const override;
        virtual ~ActiveState() {}
};

class IdleState : public GridState
{
    public:

        void Update(Map&, NGridType&, GridInfo&, const uint32& x, const uint32& y, const uint32& t_diff) const override;
        virtual ~IdleState() {}
};

class RemovalState : public GridState
{
    public:

        void Update(Map&, NGridType&, GridInfo&, const uint32& x, const uint32& y, const uint32& t_diff) const override;
        virtual ~RemovalState() {}
};

#endif
