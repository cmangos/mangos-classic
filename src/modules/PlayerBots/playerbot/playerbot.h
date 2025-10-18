#pragma once

#include "Spells/Spell.h"
#include "Server/WorldPacket.h"
#include "Loot/LootMgr.h"
#include "Entities/GossipDef.h"
#include "Chat/Chat.h"
#include "Common.h"
#include "World/World.h"
#include "Spells/SpellMgr.h"
#include "Globals/ObjectMgr.h"
#include "Entities/Unit.h"
#include "Globals/SharedDefines.h"
#include "MotionGenerators/MotionMaster.h"
#include "Spells/SpellAuras.h"
#include "Guilds/Guild.h"

#include "playerbotDefs.h"
#include "playerbot/PlayerbotAIAware.h"
#include "PlayerbotMgr.h"
#include "playerbot/RandomPlayerbotMgr.h"
#include "ChatHelper.h"
#include "BroadcastHelper.h"
#include "PlayerbotAI.h"
#include "PlayerbotDbStore.h"

#define MANGOSBOT_VERSION 2

std::vector<std::string> split(std::string const& s, char delim);
void split(std::vector<std::string>& dest, std::string const& str, char const* delim);
#ifndef WIN32
int strcmpi(std::string s1, std::string s2);
#endif
