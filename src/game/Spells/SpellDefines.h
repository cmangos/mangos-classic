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

#ifndef __SPELL_DEFINES_H
#define __SPELL_DEFINES_H

#include "Common.h"

// Spell triggering settings for CastSpell that enable us to skip some checks so that we can investigate spell specific settings
enum TriggerCastFlags : uint32
{
    TRIGGERED_NONE                              = 0x00000000,   // Not Triggered
    TRIGGERED_OLD_TRIGGERED                     = 0x00000001,   // Legacy bool support TODO: Restrict usage as much as possible.
    TRIGGERED_IGNORE_HIT_CALCULATION            = 0x00000002,   // Will ignore calculating hit in SpellHitResult
    TRIGGERED_IGNORE_UNSELECTABLE_FLAG          = 0x00000004,   // Ignores UNIT_FLAG_NOT_SELECTABLE in CheckTarget
    TRIGGERED_INSTANT_CAST                      = 0x00000008,   // Will ignore any cast time set in spell entry
    TRIGGERED_AUTOREPEAT                        = 0x00000010,   // Will signal spell system that this is internal autorepeat call
    TRIGGERED_IGNORE_UNATTACKABLE_FLAG          = 0x00000020,   // Ignores UNIT_FLAG_NOT_ATTACKABLE in CheckTarget
    TRIGGERED_DO_NOT_PROC                       = 0x00000040,   // Spells from scripts should not proc - DBScripts for example
    TRIGGERED_PET_CAST                          = 0x00000080,   // Spell that should report error through pet opcode
    TRIGGERED_NORMAL_COMBAT_CAST                = 0x00000100,   // AI needs to be notified about change of target TODO: change into TRIGGERED_NONE
    TRIGGERED_IGNORE_GCD                        = 0x00000200,   // Ignores GCD - to be used in spell scripts
    TRIGGERED_IGNORE_COSTS                      = 0x00000400,   // Ignores spell costs
    TRIGGERED_IGNORE_COOLDOWNS                  = 0x00000800,   // Ignores cooldowns
    TRIGGERED_IGNORE_CURRENT_CASTED_SPELL       = 0x00001000,   // Ignores concurrent casts and is not set as currently executed
    TRIGGERED_HIDE_CAST_IN_COMBAT_LOG           = 0x00002000,   // Sends cast flag for ignoring combat log display - used for many procs - default behaviour for triggered by aura
    TRIGGERED_FULL_MASK                         = 0xFFFFFFFF
};

#endif
