/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
* This program is free software licensed under GPL version 2
* Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_PRECOMPILED_H
#define SC_PRECOMPILED_H

#include "../ScriptMgr.h"
#include "Object.h"
#include "Unit.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "GameObject.h"
#include "sc_creature.h"
#include "sc_gossip.h"
#include "sc_grid_searchers.h"
#include "sc_instance.h"
#include "SpellAuras.h"

#ifdef WIN32
#  include <windows.h>
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    return true;
}
#endif

// sc_gossip.h:             ADD_GOSSIP_ITEM_EXTENDED outcommented box-money (Required until professions are fixed)
// sc_creature.cpp:         Used in ScriptedAI::SelectSpell, outcommented SchoolMask

#endif
