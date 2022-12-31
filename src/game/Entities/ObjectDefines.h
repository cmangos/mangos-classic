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

#ifndef CMANGOS_OBJECTDEFINES_H
#define CMANGOS_OBJECTDEFINES_H

#include "Maps/GridDefines.h"

static constexpr float const CONTACT_DISTANCE               = 1.f;
static constexpr float const INTERACTION_DISTANCE           = 5.0f;
static constexpr float const ATTACK_DISTANCE                = 5.0f;
static constexpr float const MELEE_LEEWAY                   = 8.0f / 3.0f;                  // Melee attack and melee spell leeway when moving
static constexpr float const AOE_LEEWAY                     = 2.0f;                         // AOE leeway when moving
static constexpr float const INSPECT_DISTANCE               = 28.0f;
static constexpr float const TRADE_DISTANCE                 = 11.11f;

static constexpr float const MAX_VISIBILITY_DISTANCE        = SIZE_OF_GRIDS;                // max distance for visible object show, limited in 533 yards
static constexpr float const VISIBILITY_DISTANCE_GIGANTIC   = 400.0f;
static constexpr float const VISIBILITY_DISTANCE_LARGE      = 200.0f;
static constexpr float const VISIBILITY_DISTANCE_NORMAL     = 100.0f;
static constexpr float const VISIBILITY_DISTANCE_SMALL      = 50.0f;
static constexpr float const VISIBILITY_DISTANCE_TINY       = 25.0f;
static constexpr float const DEFAULT_VISIBILITY_DISTANCE    = VISIBILITY_DISTANCE_NORMAL;   // default visible distance, 100 yards on continents
static constexpr float const DEFAULT_VISIBILITY_INSTANCE    = 170.0f;                       // default visible distance in instances, 170 yards
static constexpr float const DEFAULT_VISIBILITY_BG          = 533.0f;                       // default visible distance in BG/Arenas, 533 yards

static constexpr float const DEFAULT_WORLD_OBJECT_SIZE      = 0.388999998569489f;           // currently used (correctly?) for any non Unit world objects.
                                                                                            // This is actually the bounding_radius, like player/creature from
                                                                                            // creature_model_data

static constexpr float const DEFAULT_OBJECT_SCALE           = 1.0f;                         // player/item scale as default, npc/go from database, pets from dbc
static constexpr float const DEFAULT_COLLISION_HEIGHT       = 2.03128f;                     // Most common value in dbc
static constexpr float const DEFAULT_COLLISION_WIDTH        = 0.5097f;                      // Most common value in dbc

static constexpr float const MAX_STEALTH_DETECT_RANGE       = 45.0f;
static constexpr float const GRID_ACTIVATION_RANGE          = 45.0f;

struct AreaNameInfo
{
    const char* areaName;
    const char* wmoNameOverride;
};

#endif
