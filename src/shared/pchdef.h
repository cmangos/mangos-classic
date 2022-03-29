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
#include "ByteBuffer.h"
#include "ProgressBar.h"
#include "WorldPacket.h"
#include "Config/Config.h"
#include "Database/DatabaseEnv.h"
#include "Database/SQLStorage.h"

#endif // PCHDEF_H
