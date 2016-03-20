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

# include external dependencies libs headers (warnings disabled)
include_directories(SYSTEM
  ${CMAKE_SOURCE_DIR}/dep/include/g3dlite
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation
  ${CMAKE_SOURCE_DIR}/dep/include
  ${MYSQL_INCLUDE_DIR}
)

# include additionals headers
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/vmap
  ${CMAKE_CURRENT_SOURCE_DIR}/AuctionHouseBot
  ${CMAKE_CURRENT_SOURCE_DIR}/BattleGround
  ${CMAKE_CURRENT_SOURCE_DIR}/OutdoorPvP
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src/shared
)

set(SRC_GRP_AHBOT
    AuctionHouseBot/AuctionHouseBot.cpp
    AuctionHouseBot/AuctionHouseBot.h
)

set(SRC_GRP_BATTLEGROUND
    BattleGround/BattleGround.cpp
    BattleGround/BattleGround.h
    BattleGround/BattleGroundAB.cpp
    BattleGround/BattleGroundAB.h
    BattleGround/BattleGroundAV.cpp
    BattleGround/BattleGroundAV.h
    BattleGround/BattleGroundHandler.cpp
    BattleGround/BattleGroundMgr.cpp
    BattleGround/BattleGroundMgr.h
    BattleGround/BattleGroundWS.cpp
    BattleGround/BattleGroundWS.h
)

set(SRC_GRP_CHAT_COMMANDS
    debugcmds.cpp
    Level0.cpp
    Level1.cpp
    Level2.cpp
    Level3.cpp
)

set(SRC_GRP_MOTION_GEN
    ConfusedMovementGenerator.cpp
    ConfusedMovementGenerator.h
    FleeingMovementGenerator.cpp
    FleeingMovementGenerator.h
    HomeMovementGenerator.cpp
    HomeMovementGenerator.h
    IdleMovementGenerator.cpp
    IdleMovementGenerator.h
    MotionMaster.cpp
    MotionMaster.h
    MovementGenerator.cpp
    MovementGenerator.h
    MovementGeneratorImpl.h   # TODO: this is not in the VC files - does it belong in here?
    PathFinder.cpp
    PathFinder.h
    PointMovementGenerator.cpp
    PointMovementGenerator.h
    RandomMovementGenerator.cpp
    RandomMovementGenerator.h
    TargetedMovementGenerator.cpp
    TargetedMovementGenerator.h
    WaypointMovementGenerator.cpp
    WaypointMovementGenerator.h
)

set(SRC_GRP_MOVEMENT
    movement/MoveSpline.cpp
    movement/MoveSpline.h
    movement/MoveSplineFlag.h
    movement/MoveSplineInit.cpp
    movement/MoveSplineInit.h
    movement/MoveSplineInitArgs.h
    movement/packet_builder.cpp
    movement/packet_builder.h
    movement/spline.cpp
    movement/spline.h
    movement/spline.impl.h
    movement/typedefs.h
    movement/util.cpp
)

set(SRC_GRP_OBJECT
    AggressorAI.cpp
    AggressorAI.h
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
    CreatureAI.cpp
    CreatureAI.h
    CreatureAIImpl.h
    CreatureAIRegistry.cpp
    CreatureAIRegistry.h
    CreatureAISelector.cpp
    CreatureAISelector.h
    CreatureEventAI.cpp
    CreatureEventAI.h
    CreatureEventAIMgr.cpp
    CreatureEventAIMgr.h
    DynamicObject.cpp
    DynamicObject.h
    Formulas.h
    GameObject.cpp
    GameObject.h
    GMTicketMgr.cpp
    GMTicketMgr.h
    GuardAI.cpp
    GuardAI.h
    Guild.cpp
    Guild.h
    Item.cpp
    Item.h
    ItemEnchantmentMgr.cpp
    ItemEnchantmentMgr.h
    ItemPrototype.h
    LootMgr.cpp
    LootMgr.h
    NullCreatureAI.cpp
    NullCreatureAI.h
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
    PetAI.cpp
    PetAI.h
    Player.cpp
    Player.h
    ReactorAI.cpp
    ReactorAI.h
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
    TotemAI.cpp
    TotemAI.h
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
    ${SRC_GRP_AHBOT}
    ${SRC_GRP_BATTLEGROUND}
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

source_group("AhBot"
  FILES
    ${SRC_GRP_AHBOT}
)

source_group("BattleGround"
  FILES
    ${SRC_GRP_BATTLEGROUND}
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

add_library(${LIBRARY_NAME} STATIC
  ${LIBRARY_SRCS}
)

target_link_libraries(${LIBRARY_NAME}
  shared
  detour
)

if(UNIX)
  # Both systems don't have libdl and don't need them
  if (NOT (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "NetBSD"))
    target_link_libraries(${LIBRARY_NAME}
      dl
    )
  endif()
endif()

add_dependencies(${LIBRARY_NAME} revision.h)

# Generate precompiled header
if(PCH)
  cotire(${LIBRARY_NAME})
endif()
