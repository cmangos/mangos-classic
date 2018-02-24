#
# This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
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

set(LIBRARY_NAME game)

add_definitions(-DDT_POLYREF64)
add_definitions(-D__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES=0)

# include additionals headers
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/vmap
  ${CMAKE_CURRENT_SOURCE_DIR}/AuctionHouseBot
  ${CMAKE_CURRENT_SOURCE_DIR}/BattleGround
  ${CMAKE_CURRENT_SOURCE_DIR}/LuaEngine
  ${CMAKE_CURRENT_SOURCE_DIR}/OutdoorPvP
  ${CMAKE_SOURCE_DIR}/dep/lualib
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_BINARY_DIR}
)

# Find all the input files
FILE(GLOB_RECURSE LIBRARY_SRCS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h" "*.cpp")

if(NOT BUILD_SCRIPTDEV)
  # exclude script folder for faster build time (used for dev mode)
  set (EXCLUDE_DIR "AI/ScriptDevAI/scripts/")
  foreach (TMP_PATH ${LIBRARY_SRCS})
      string (FIND ${TMP_PATH} ${EXCLUDE_DIR} EXCLUDE_DIR_FOUND)
      if (NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
          list(REMOVE_ITEM LIBRARY_SRCS ${TMP_PATH})
      endif ()
  endforeach()
endif()

if(NOT BUILD_PLAYERBOT)
  # exclude Playerbot folder
  set (EXCLUDE_DIR "PlayerBot/")
  foreach (TMP_PATH ${LIBRARY_SRCS})
      string (FIND ${TMP_PATH} ${EXCLUDE_DIR} EXCLUDE_DIR_FOUND)
      if (NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
          list(REMOVE_ITEM LIBRARY_SRCS ${TMP_PATH})
      endif ()
  endforeach()
endif()

file(GLOB SRC_FILES_LUAENGINE LuaEngine/*.cpp LuaEngine/*.h)
set(SRC_GRP_LUAENGINE
  ${SRC_FILES_LUAENGINE}
)

file(GLOB SRC_FILES_LUAENGINE LuaEngine/*.cpp LuaEngine/*.h)
set(SRC_GRP_LUAENGINE
  ${SRC_FILES_LUAENGINE}
)

set(SRC_GRP_CHAT_COMMANDS
    debugcmds.cpp
    Level0.cpp
    Level1.cpp
    Level2.cpp
    Level3.cpp
)

add_library(${LIBRARY_NAME} STATIC
  ${LIBRARY_SRCS}
)

target_link_libraries(${LIBRARY_NAME}
  PUBLIC shared
  PUBLIC g3dlite
  PRIVATE detour
)

set(SRC_GRP_OBJECT
    AuctionHouseMgr.cpp
    AuctionHouseMgr.h
    Bag.cpp
    Bag.h
    Camera.cpp
    Camera.h
    Corpse.cpp
    Corpse.h
    Creature.cpp
    Creature.h
    DynamicObject.cpp
    DynamicObject.h
    Formulas.h
    GameObject.cpp
    GameObject.h
    GMTicketMgr.cpp
    GMTicketMgr.h
    Guild.cpp
    Guild.h
    Item.cpp
    Item.h
    ItemEnchantmentMgr.cpp
    ItemEnchantmentMgr.h
    ItemPrototype.h
    LootMgr.cpp
    LootMgr.h
    Object.cpp
    Object.h
    ObjectAccessor.cpp
    ObjectAccessor.h
    ObjectGuid.cpp
    ObjectGuid.h
    ObjectMgr.cpp
    ObjectMgr.h
    ObjectPosSelector.cpp
    ObjectPosSelector.h
    Pet.cpp
    Pet.h
    Player.cpp
    Player.h
    ReputationMgr.cpp
    ReputationMgr.h
    SocialMgr.cpp
    SocialMgr.h
    SpellMgr.cpp
    SpellMgr.h
    StatSystem.cpp
    TemporarySummon.cpp
    TemporarySummon.h
    Totem.cpp
    Totem.h
    Unit.cpp
    Unit.h
    UnitEvents.h
    UpdateFields.h
    UpdateMask.h
)

set(SRC_GRP_OUTDOOR_PVP
    OutdoorPvP/OutdoorPvP.cpp
    OutdoorPvP/OutdoorPvP.h
    OutdoorPvP/OutdoorPvPEP.cpp
    OutdoorPvP/OutdoorPvPEP.h
    OutdoorPvP/OutdoorPvPMgr.cpp
    OutdoorPvP/OutdoorPvPMgr.h
    OutdoorPvP/OutdoorPvPSI.cpp
    OutdoorPvP/OutdoorPvPSI.h
)

set(SRC_GRP_REFERENCES
    FollowerReference.cpp
    FollowerReference.h
    FollowerRefManager.h
    GroupReference.cpp
    GroupReference.h
    GroupRefManager.h
    HostileRefManager.cpp
    HostileRefManager.h
    MapReference.h
    MapRefManager.h
    ThreatManager.cpp
    ThreatManager.h
)

set(SRC_GRP_SERVER
    DBCEnums.h
    DBCfmt.h
    DBCStores.cpp
    DBCStores.h
    DBCStructure.h
    Opcodes.cpp
    Opcodes.h
    SharedDefines.h
    SQLStorages.cpp
    SQLStorages.h
    WorldSession.cpp
    WorldSession.h
    WorldSocket.cpp
    WorldSocket.h
)

set(SRC_GRP_TOOL
    CharacterDatabaseCleaner.cpp
    CharacterDatabaseCleaner.h
    Language.h
    PlayerDump.cpp
    PlayerDump.h
)

set(SRC_GRP_VMAPS
    vmap/BIH.cpp
    vmap/BIH.h
    vmap/BIHWrap.h
    vmap/DynamicTree.cpp
    vmap/DynamicTree.h
    vmap/GameObjectModel.cpp
    vmap/GameObjectModel.h
    vmap/IVMapManager.h
    vmap/MapTree.cpp
    vmap/MapTree.h
    vmap/ModelInstance.cpp
    vmap/ModelInstance.h
    vmap/RegularGrid.h
    vmap/TileAssembler.cpp
    vmap/TileAssembler.h
    vmap/VMapDefinitions.h
    vmap/VMapFactory.cpp
    vmap/VMapFactory.h
    vmap/VMapManager2.cpp
    vmap/VMapManager2.h
    vmap/VMapTools.h
    vmap/WorldModel.cpp
    vmap/WorldModel.h
)

set(SRC_GRP_WORLD_HANDLERS
    AccountMgr.cpp
    AccountMgr.h
    AddonHandler.cpp
    AddonHandler.h
    AuctionHouseHandler.cpp
    Cell.h
    CellImpl.h
    Channel.cpp
    Channel.h
    ChannelHandler.cpp
    ChannelMgr.cpp
    ChannelMgr.h
    CharacterHandler.cpp
    Chat.cpp
    Chat.h
    ChatHandler.cpp
    CombatHandler.cpp
    CreatureLinkingMgr.cpp
    CreatureLinkingMgr.h
    DuelHandler.cpp
    GameEventMgr.cpp
    GameEventMgr.h
    GMTicketHandler.cpp
    GossipDef.cpp
    GossipDef.h
    GridDefines.h
    GridMap.cpp
    GridMap.h
    GridNotifiers.cpp
    GridNotifiers.h
    GridNotifiersImpl.h
    GridStates.cpp
    GridStates.h
    Group.cpp
    Group.h
    GroupHandler.cpp
    GuildHandler.cpp
    GuildMgr.cpp
    GuildMgr.h
    InstanceData.cpp
    InstanceData.h
    ItemHandler.cpp
    LootHandler.cpp
    Mail.cpp
    Mail.h
    MailHandler.cpp
    Map.cpp
    Map.h
    MapManager.cpp
    MapManager.h
    MapPersistentStateMgr.cpp
    MapPersistentStateMgr.h
    MassMailMgr.cpp
    MassMailMgr.h
    MiscHandler.cpp
    MoveMap.cpp
    MoveMap.h
    MoveMapSharedDefines.h
    MovementHandler.cpp
    NPCHandler.cpp
    NPCHandler.h
    ObjectGridLoader.cpp
    ObjectGridLoader.h
    Path.h
    PetHandler.cpp
    PetitionsHandler.cpp
    PoolManager.cpp
    PoolManager.h
    QueryHandler.cpp
    QuestDef.cpp
    QuestDef.h
    QuestHandler.cpp
    ScriptMgr.cpp
    ScriptMgr.h
    SkillHandler.cpp
    Spell.cpp
    Spell.h
    SpellAuraDefines.h
    SpellAuras.cpp
    SpellAuras.h
    SpellEffects.cpp
    SpellHandler.cpp
    TaxiHandler.cpp
    TradeHandler.cpp
    Transports.cpp
    Transports.h
    UnitAuraProcHandler.cpp
    UpdateData.cpp
    UpdateData.h
    WaypointManager.cpp
    WaypointManager.h
    Weather.cpp
    Weather.h
    World.cpp
    World.h
)

set(LIBRARY_SRCS
    ${SRC_GRP_AI}
    ${SRC_GRP_AHBOT}
    ${SRC_GRP_BATTLEGROUND}
    ${SRC_GRP_LUAENGINE}
    ${SRC_GRP_CHAT_COMMANDS}
    ${SRC_GRP_MOTION_GEN}
    ${SRC_GRP_MOVEMENT}
    ${SRC_GRP_OBJECT}
    ${SRC_GRP_OUTDOOR_PVP}
    ${SRC_GRP_REFERENCES}
    ${SRC_GRP_SERVER}
    ${SRC_GRP_TOOL}
    ${SRC_GRP_VMAPS}
    ${SRC_GRP_WORLD_HANDLERS}
    pchdef.cpp
    pchdef.h
)

source_group("AI"
  FILES
    ${SRC_GRP_AI}
)

source_group("AhBot"
  FILES
    ${SRC_GRP_AHBOT}
)

source_group("BattleGround"
  FILES
    ${SRC_GRP_BATTLEGROUND}
)

source_group("LuaEngine"
  FILES
    ${SRC_GRP_LUAENGINE}
)

source_group("Chat Commands"
  FILES
    ${SRC_GRP_CHAT_COMMANDS}
)

source_group("Motion generators"
  FILES
    ${SRC_GRP_MOTION_GEN}
)

source_group("Movement"
  FILES
    ${SRC_GRP_MOVEMENT}
)

source_group("Object"
  FILES
    ${SRC_GRP_OBJECT}
)

source_group("Outdoor PvP"
  FILES
    ${SRC_GRP_OUTDOOR_PVP}
)

source_group("References"
  FILES
    ${SRC_GRP_REFERENCES}
)

source_group("Server"
  FILES
    ${SRC_GRP_SERVER}
)

source_group("Tool"
  FILES
    ${SRC_GRP_TOOL}
)

source_group("vmaps"
  FILES
    ${SRC_GRP_VMAPS}
)

source_group("World/Handlers"
  FILES
    ${SRC_GRP_WORLD_HANDLERS}
)

if(PCH)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
endif()

target_include_directories(${LIBRARY_NAME}
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${LIBRARY_NAME}
  shared
  detour
  lualib
)

if(UNIX)
  # Both systems don't have libdl and don't need them
  if (NOT (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "NetBSD"))
    target_link_libraries(${LIBRARY_NAME} PUBLIC dl)
  endif()
endif()

add_dependencies(${LIBRARY_NAME} revision.h)
add_dependencies(${LIBRARY_NAME} lualib)

# Define BUILD_PLAYERBOT if need
if (BUILD_PLAYERBOT)
  add_definitions(-DBUILD_PLAYERBOT)
endif()

# Generate precompiled header
if(PCH)
  if (MSVC)
    set(PCHBaseName "pchdef")
    set(PrecompiledBinary "$(IntDir)/${PCHBaseName}.pch")
    set(PrecompiledHeader "${PCHBaseName}.h")
    set(PrecompiledSource "${PCHBaseName}.cpp")
    #set_property(TARGET ${LIBRARY_NAME} PROPERTY COMPILE_FLAGS " /Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\"")
    #set_property(SOURCE ${PrecompiledSource} PROPERTY COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\"")
    set_target_properties(${LIBRARY_NAME} PROPERTIES COMPILE_FLAGS " /Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\" /bigobj")
    set_source_files_properties(${PrecompiledSource} PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\"")
    set_target_properties(${LIBRARY_NAME} PROPERTIES PROJECT_LABEL "Game")
  elseif(NOT MINGW)
    # TODO: Resolve issues with order of includes before enabling Cotire for MinGW builds
    cotire(${LIBRARY_NAME})
  endif()
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/LuaEngine/extensions DESTINATION ${BIN_DIR}/lua_scripts/)
