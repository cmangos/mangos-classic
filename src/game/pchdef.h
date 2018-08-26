// add here most rarely modified headers to speed up debug build compilation
#ifndef PCHDEF_H
#define PCHDEF_H

#include "Server/WorldSocket.h"
#include "Common.h"

#include "Log.h"
#include "Maps/MapManager.h"
#include "Globals/ObjectAccessor.h"
#include "Server/SQLStorages.h"
#include "Server/Opcodes.h"
#include "Globals/SharedDefines.h"
#include "Guilds/GuildMgr.h"
#include "Globals/ObjectMgr.h"
#include "DBScripts/ScriptMgr.h"

#include "Entities/ObjectGuid.h"
#include "Entities/Object.h"
#include "Entities/Unit.h"
#include "Entities/Creature.h"
#include "Entities/GameObject.h"
#include "Spells/SpellAuras.h"
#include "AI/BaseAI/CreatureAI.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"
#endif // PCHDEF_H
