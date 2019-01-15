// add here most rarely modified headers to speed up debug build compilation
#ifndef PCHDEF_H
#define PCHDEF_H


#include "Platform/Define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cmath>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <set>
#include <list>
#include <string>
#include <map>
#include <queue>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include "Errors.h"
#include "Threading.h"
#include <thread>
#include <boost/asio.hpp>
#include "Common.h"
#include "Log.h"
#include "Server/SQLStorages.h"
#include "Server/DBCStructure.h"
#include "Server/DBCStores.h"
#include "Globals/ObjectAccessor.h"
#include "AI/BaseAI/CreatureAI.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"
#include "Server/Opcodes.h"
#include "ByteBuffer.h"
#include "Entities/UpdateFields.h"
#include "Entities/UpdateData.h"
#include "Globals/SharedDefines.h"
#include "Globals/ObjectMgr.h"
#include "DBScripts/ScriptMgr.h"
#include "Entities/ObjectGuid.h"
#include "Entities/Object.h"
#include "Spells/SpellAuras.h"
#endif // PCHDEF_H
