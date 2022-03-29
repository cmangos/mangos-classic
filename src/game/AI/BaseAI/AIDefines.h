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

#ifndef MANGOS_AI_DEFINES_H
#define MANGOS_AI_DEFINES_H

enum AIEventType
{
    // Usable with Event AI
    AI_EVENT_JUST_DIED          = 0,                        // Sender = Killed Npc, Invoker = Killer
    AI_EVENT_CRITICAL_HEALTH    = 1,                        // Sender = Hurt Npc, Invoker = DamageDealer - Expected to be sent by 10% health
    AI_EVENT_LOST_HEALTH        = 2,                        // Sender = Hurt Npc, Invoker = DamageDealer - Expected to be sent by 50% health
    AI_EVENT_LOST_SOME_HEALTH   = 3,                        // Sender = Hurt Npc, Invoker = DamageDealer - Expected to be sent by 90% health
    AI_EVENT_GOT_FULL_HEALTH    = 4,                        // Sender = Healed Npc, Invoker = Healer
    AI_EVENT_CUSTOM_EVENTAI_A   = 5,                        // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    AI_EVENT_CUSTOM_EVENTAI_B   = 6,                        // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    AI_EVENT_GOT_CCED           = 7,                        // Sender = CCed Npc, Invoker = Caster that CCed
    AI_EVENT_CUSTOM_EVENTAI_C   = 8,                        // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    AI_EVENT_CUSTOM_EVENTAI_D   = 9,                        // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    AI_EVENT_CUSTOM_EVENTAI_E   = 10,                       // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    AI_EVENT_CUSTOM_EVENTAI_F   = 11,                       // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    MAXIMAL_AI_EVENT_EVENTAI    = 12,

    // Internal Use
    AI_EVENT_CALL_ASSISTANCE    = 13,                       // Sender = Attacked Npc, Invoker = Enemy

    // Predefined for SD2
    AI_EVENT_START_ESCORT       = 100,                      // Invoker = Escorting Player
    AI_EVENT_START_ESCORT_B     = 101,                      // Invoker = Escorting Player
    AI_EVENT_START_EVENT        = 102,                      // Invoker = EventStarter
    AI_EVENT_START_EVENT_A      = 103,                      // Invoker = EventStarter
    AI_EVENT_START_EVENT_B      = 104,                      // Invoker = EventStarter

    // Some IDs for special cases in SD2
    AI_EVENT_CUSTOM_A           = 1000,
    AI_EVENT_CUSTOM_B           = 1001,
    AI_EVENT_CUSTOM_C           = 1002,
    AI_EVENT_CUSTOM_D           = 1003,
    AI_EVENT_CUSTOM_E           = 1004,
    AI_EVENT_CUSTOM_F           = 1005,
};

#endif