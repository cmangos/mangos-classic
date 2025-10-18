#
# Copyright (C) 2005-2018 MaNGOS project <http://getmangos.com/>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

set(LIBRARY_NAME playerbots)

#Add PCH Files
set(bots_PCH "${CMAKE_CURRENT_SOURCE_DIR}/botpch.h")

#Base files
file(GLOB Playerbot_Source ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/*.h)
    source_group("Player Bot" FILES ${Playerbot_Source})

file(GLOB AHbot_Source ${CMAKE_CURRENT_SOURCE_DIR}/ahbot/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/ahbot/*.h)
    source_group("AH Bot" FILES ${AHbot_Source})

#Define base source library
set(LIBRARY_SRCS
    ${Playerbot_Source}
    ${AHbot_Source}
    ${bots_PCH}
    ${CMAKE_CURRENT_SOURCE_DIR}/botpch.cpp
)

#Strategy files
file(GLOB Playerbot_Strategy ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/*.h)
    source_group("Player Bot\\Strategies" FILES ${Playerbot_Strategy})
    LIST(APPEND LIBRARY_SRCS ${Playerbot_Strategy})

#Action files
file(GLOB Playerbot_Actions ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/actions/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/actions/*.h)
    source_group("Player Bot\\Strategies\\Actions" FILES ${Playerbot_Actions})
    LIST(APPEND LIBRARY_SRCS ${Playerbot_Actions})

#Generic files
file(GLOB Playerbot_Generic ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/generic/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/generic/*.h)
    source_group("Player Bot\\Strategies\\Generic" FILES ${Playerbot_Generic})
    LIST(APPEND LIBRARY_SRCS ${Playerbot_Generic})

#Trigger files
file(GLOB Playerbot_Triggers ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/triggers/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/triggers/*.h)
    source_group("Player Bot\\Strategies\\Triggers" FILES ${Playerbot_Triggers})
    LIST(APPEND LIBRARY_SRCS ${Playerbot_Triggers})

#Value files
file(GLOB Playerbot_Values ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/values/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/values/*.h)
    source_group("Player Bot\\Strategies\\Values" FILES ${Playerbot_Values})
    LIST(APPEND LIBRARY_SRCS ${Playerbot_Values})

## Class files

#Druid AI
file(GLOB Playerbot_Druid ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/druid/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/druid/*.h)
    source_group("Player Bot\\Strategies\\Druid" FILES ${Playerbot_Druid})
    LIST(APPEND LIBRARY_SRCS ${Playerbot_Druid})

#Hunter AI
file(GLOB Playerbot_Hunter ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/hunter/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/hunter/*.h)
    source_group("Player Bot\\Strategies\\Hunter" FILES ${Playerbot_Hunter})
    LIST(APPEND LIBRARY_SRCS ${Playerbot_Hunter})

#Mage AI
file(GLOB Playerbot_Mage ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/mage/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/mage/*.h)
    source_group("Player Bot\\Strategies\\Mage" FILES ${Playerbot_Mage})
    LIST(APPEND LIBRARY_SRCS ${Playerbot_Mage})

#Paladin AI
file(GLOB Playerbot_Paladin ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/paladin/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/paladin/*.h)
    source_group("Player Bot\\Strategies\\Paladin" FILES ${Playerbot_Paladin})
    LIST(APPEND LIBRARY_SRCS ${Playerbot_Paladin})

#Priest AI
file(GLOB Playerbot_Priest ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/priest/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/priest/*.h)
    source_group("Player Bot\\Strategies\\Priest" FILES ${Playerbot_Priest})
    LIST(APPEND LIBRARY_SRCS ${Playerbot_Priest})

#Rogue AI
file(GLOB Playerbot_Rogue ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/rogue/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/rogue/*.h)
    source_group("Player Bot\\Strategies\\Rogue" FILES ${Playerbot_Rogue})
    LIST(APPEND LIBRARY_SRCS ${Playerbot_Rogue})

#Shaman AI
file(GLOB Playerbot_Shaman ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/shaman/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/shaman/*.h)
    source_group("Player Bot\\Strategies\\Shaman" FILES ${Playerbot_Shaman})
    LIST(APPEND LIBRARY_SRCS ${Playerbot_Shaman})

#Warlock AI
file(GLOB Playerbot_Warlock ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/warlock/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/warlock/*.h)
    source_group("Player Bot\\Strategies\\Warlock" FILES ${Playerbot_Warlock})
    LIST(APPEND LIBRARY_SRCS ${Playerbot_Warlock})

#Warrior AI
file(GLOB Playerbot_Warrior ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/warrior/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/warrior/*.h)
    source_group("Player Bot\\Strategies\\Warrior" FILES ${Playerbot_Warrior})
    LIST(APPEND LIBRARY_SRCS ${Playerbot_Warrior})

#Deathknight AI
file(GLOB Playerbot_DeathKnight ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/deathknight/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/playerbot/strategy/deathknight/*.h)
    source_group("Player Bot\\Strategies\\DeathKnight" FILES ${Playerbot_DeathKnight})
    LIST(APPEND LIBRARY_SRCS ${Playerbot_DeathKnight})

# Define ENABLE_PLAYERBOTS if need
if (BUILD_PLAYERBOTS)
  add_definitions(-DENABLE_PLAYERBOTS)
endif()

# Define Core
if ( ${CMAKE_PROJECT_NAME} MATCHES "CMaNGOS")
  add_definitions(-DCMANGOS)
else()
  if ( ${CMAKE_PROJECT_NAME} MATCHES "MaNGOS")
    add_definitions(-DMANGOS)
  endif()
endif()

# Define Expansion
if ( ${CMAKE_PROJECT_NAME} MATCHES "Classic")
  add_definitions(-DMANGOSBOT_ZERO)
endif()
if ( ${CMAKE_PROJECT_NAME} MATCHES "TBC")
  add_definitions(-DMANGOSBOT_ONE)
endif()
if ( ${CMAKE_PROJECT_NAME} MATCHES "WoTLK")
  add_definitions(-DMANGOSBOT_TWO)
endif()

# Define ENABLE_MODULES if need
if (BUILD_MODULES)
  add_definitions(-DENABLE_MODULES)
endif()

# Define BUILD_AHBOT if need
if (BUILD_AHBOT)
  add_definitions(-DBUILD_AHBOT)
endif()

if(PCH)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
endif()

add_library(${LIBRARY_NAME} STATIC ${LIBRARY_SRCS})

target_link_libraries(${LIBRARY_NAME}
  PRIVATE shared
  PRIVATE Detour
  PRIVATE g3dlite
  PRIVATE zlib
)
if(WIN32)
  target_link_libraries(${LIBRARY_NAME}
    PRIVATE ws2_32
  )
endif()

target_include_directories(${LIBRARY_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

# Install config files
if ( ${CMAKE_PROJECT_NAME} MATCHES "Classic")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/playerbot/aiplayerbot.conf.dist.in ${CMAKE_CURRENT_BINARY_DIR}/aiplayerbot.conf.dist)
endif()	
if ( ${CMAKE_PROJECT_NAME} MATCHES "TBC")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/playerbot/aiplayerbot.conf.dist.in.tbc ${CMAKE_CURRENT_BINARY_DIR}/aiplayerbot.conf.dist)
endif()	
if ( ${CMAKE_PROJECT_NAME} MATCHES "WoTLK")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/playerbot/aiplayerbot.conf.dist.in.wotlk ${CMAKE_CURRENT_BINARY_DIR}/aiplayerbot.conf.dist)
endif()
if (NOT CONF_INSTALL_DIR)
  set(CONF_INSTALL_DIR ${CONF_DIR})
endif()
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/aiplayerbot.conf.dist DESTINATION ${CONF_INSTALL_DIR})

set_target_properties(${LIBRARY_NAME} PROPERTIES PROJECT_LABEL "PlayerBots")

# Generate PCH
if(PCH)
  target_precompile_headers(${LIBRARY_NAME} PRIVATE ${bots_PCH})
endif()
