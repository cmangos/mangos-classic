//add here most rarely modified headers to speed up debug build compilation
#include "Server/WorldSocket.h"                                    // must be first to make ACE happy with ACE includes in it
#include "Common.h"

#include "Maps/MapManager.h"
#include "Log/Log.h"
#include "Globals/ObjectAccessor.h"
#include "Entities/ObjectGuid.h"
#include "Server/SQLStorages.h"
#include "Server/Opcodes.h"
#include "Globals/SharedDefines.h"
#include "Guilds/GuildMgr.h"
#include "Globals/ObjectMgr.h"
#include "DBScripts/ScriptMgr.h"

#include "playerbot/playerbot.h"