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

set(LIBRARY_NAME game)

set(LIBRARY_SRCS
    AccountMgr.cpp
    AccountMgr.h
    AddonHandler.cpp
    AddonHandler.h
    AggressorAI.cpp
    AggressorAI.h
    AuctionHouseBot/AuctionHouseBot.cpp
    AuctionHouseBot/AuctionHouseBot.h
    AuctionHouseHandler.cpp
    AuctionHouseMgr.cpp
    AuctionHouseMgr.h
    Bag.cpp
    Bag.h
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
    Camera.cpp
    Camera.h
    Cell.h
    CellImpl.h
    Channel.cpp
    Channel.h
    ChannelHandler.cpp
    ChannelMgr.cpp
    ChannelMgr.h
    CharacterDatabaseCleaner.cpp
    CharacterDatabaseCleaner.h
    CharacterHandler.cpp
    Chat.cpp
    Chat.h
    ChatHandler.cpp
    CombatHandler.cpp
    ConfusedMovementGenerator.cpp
    ConfusedMovementGenerator.h
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
    CreatureLinkingMgr.cpp
    CreatureLinkingMgr.h
    DBCEnums.h
    DBCfmt.h
    DBCStores.cpp
    DBCStores.h
    DBCStructure.h
    debugcmds.cpp
    DuelHandler.cpp
    DynamicObject.cpp
    DynamicObject.h
    extras/Mod.cpp
    extras/Mod.h
    extras/SpellMod.cpp
    FleeingMovementGenerator.cpp
    FleeingMovementGenerator.h
    FollowerReference.cpp
    FollowerReference.h
    FollowerRefManager.h
    Formulas.h
    GameEventMgr.cpp
    GameEventMgr.h
    GameObject.cpp
    GameObject.h
    GMTicketHandler.cpp
    GMTicketMgr.cpp
    GMTicketMgr.h
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
    GroupReference.cpp
    GroupReference.h
    GroupRefManager.h
    GuardAI.cpp
    GuardAI.h
    Guild.cpp
    Guild.h
    GuildHandler.cpp
    GuildMgr.cpp
    GuildMgr.h
    HomeMovementGenerator.cpp
    HomeMovementGenerator.h
    HostileRefManager.cpp
    HostileRefManager.h
    IdleMovementGenerator.cpp
    IdleMovementGenerator.h
    InstanceData.cpp
    InstanceData.h
    Item.cpp
    Item.h
    ItemEnchantmentMgr.cpp
    ItemEnchantmentMgr.h
    ItemHandler.cpp
    ItemPrototype.h
    Language.h
    Level0.cpp
    Level1.cpp
    Level2.cpp
    Level3.cpp
    LootHandler.cpp
    LootMgr.cpp
    LootMgr.h
    Mail.cpp
    Mail.h
    MailHandler.cpp
    Map.cpp
    Map.h
    MapManager.cpp
    MapManager.h
    MapPersistentStateMgr.cpp
    MapPersistentStateMgr.h
    MapReference.h
    MapRefManager.h
    MassMailMgr.cpp
    MassMailMgr.h
    MiscHandler.cpp
    MotionMaster.cpp
    MotionMaster.h
    MoveMap.cpp
    MoveMap.h
    MoveMapSharedDefines.h
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
    MovementGenerator.cpp
    MovementGenerator.h
    MovementGeneratorImpl.h
    MovementHandler.cpp
    NPCHandler.cpp
    NPCHandler.h
    NullCreatureAI.cpp
    NullCreatureAI.h
    Object.cpp
    Object.h
    ObjectAccessor.cpp
    ObjectAccessor.h
    ObjectGridLoader.cpp
    ObjectGridLoader.h
    ObjectGuid.cpp
    ObjectGuid.h
    ObjectMgr.cpp
    ObjectMgr.h
    ObjectPosSelector.cpp
    ObjectPosSelector.h
    Opcodes.cpp
    Opcodes.h
    OutdoorPvP/OutdoorPvP.cpp
    OutdoorPvP/OutdoorPvP.h
    OutdoorPvP/OutdoorPvPEP.cpp
    OutdoorPvP/OutdoorPvPEP.h
    OutdoorPvP/OutdoorPvPMgr.cpp
    OutdoorPvP/OutdoorPvPMgr.h
    OutdoorPvP/OutdoorPvPSI.cpp
    OutdoorPvP/OutdoorPvPSI.h
    Path.h
    PathFinder.cpp
    PathFinder.h
    pchdef.cpp
    pchdef.h
    Pet.cpp
    Pet.h
    PetAI.cpp
    PetAI.h
    PetHandler.cpp
    PetitionsHandler.cpp
    Player.cpp
    Player.h
    PlayerDump.cpp
    PlayerDump.h
    PointMovementGenerator.cpp
    PointMovementGenerator.h
    PoolManager.cpp
    PoolManager.h
    QueryHandler.cpp
    QuestDef.cpp
    QuestDef.h
    QuestHandler.cpp
    RandomMovementGenerator.cpp
    RandomMovementGenerator.h
    ReactorAI.cpp
    ReactorAI.h
    ReputationMgr.cpp
    ReputationMgr.h
    ScriptMgr.cpp
    ScriptMgr.h
    SharedDefines.h
    SkillHandler.cpp
    SocialMgr.cpp
    SocialMgr.h
    Spell.cpp
    Spell.h
    SpellAuraDefines.h
    SpellAuras.cpp
    SpellAuras.h
    SpellEffects.cpp
    SpellHandler.cpp
    SpellMgr.cpp
    SpellMgr.h
    SQLStorages.cpp
    SQLStorages.h
    StatSystem.cpp
    TargetedMovementGenerator.cpp
    TargetedMovementGenerator.h
    TaxiHandler.cpp
    TemporarySummon.cpp
    TemporarySummon.h
    ThreatManager.cpp
    ThreatManager.h
    Totem.cpp
    Totem.h
    TotemAI.cpp
    TotemAI.h
    TradeHandler.cpp
    Transports.cpp
    Transports.h
    Unit.cpp
    Unit.h
    UnitAuraProcHandler.cpp
    UnitEvents.h
    UpdateData.cpp
    UpdateData.h
    UpdateFields.h
    UpdateMask.h
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
    WaypointManager.cpp
    WaypointManager.h
    WaypointMovementGenerator.cpp
    WaypointMovementGenerator.h
    Weather.cpp
    Weather.h
    World.cpp
    World.h
    WorldSession.cpp
    WorldSession.h
    WorldSocket.cpp
    WorldSocket.h
    WorldSocketMgr.cpp
    WorldSocketMgr.h
   )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/vmap
  ${CMAKE_CURRENT_SOURCE_DIR}/AuctionHouseBot
  ${CMAKE_CURRENT_SOURCE_DIR}/BattleGround
  ${CMAKE_CURRENT_SOURCE_DIR}/OutdoorPvP
  ${CMAKE_SOURCE_DIR}/dep/include/g3dlite
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation/
  ${CMAKE_SOURCE_DIR}/dep/include
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/src/framework
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src/shared
  ${MYSQL_INCLUDE_DIR}
  ${ACE_INCLUDE_DIR}
)

source_group("Object"
  REGULAR_EXPRESSION .*
)

source_group("World/Handlers"
  # REGULAR_EXPRESSION Mgr|Handler|Manager|BattleGround|Cell|Channel|Chat|Gossip|Grid|Group|Instance|Mail|Map|Path|Pool|Quest|Script|Skill|Spell|Transports|Update|Weather|World
  REGULAR_EXPRESSION Mgr|Handler|Manager|Cell|Channel|Chat|Gossip|Grid|Instance|Map|Path|Pool|Script|Skill|Transports|Update|Weather|World
)

source_group("Motion generators"
  REGULAR_EXPRESSION MoveMap|MoveMapSharedDefines|Generator|MotionMaster
)

source_group("Server"
  REGULAR_EXPRESSION Socket|Session|Opcodes|DBC
  FILES
    SharedDefines.h
)

source_group("Chat Commands"
  REGULAR_EXPRESSION Level[0-9]
  FILES
    debugcmds.cpp
)

source_group("Tool"
  REGULAR_EXPRESSION DatabaseCleaner|Language|PlayerDump
)

source_group("References"
  REGULAR_EXPRESSION Reference|RefManager|ThreatManager
)

source_group("Movement"
  REGULAR_EXPRESSION spline|Spline|packet_builder
  FILES
    movement/typedefs.h
    movement/util.cpp
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
if(NOT ACE_USE_EXTERNAL)
  add_dependencies(${LIBRARY_NAME} ACE_Project)
endif()

# Generate precompiled header
if(PCH)
  if(MSVC OR XCODE)
    if(MSVC)
      set(game_pch "${CMAKE_CURRENT_SOURCE_DIR}/pchdef.cpp")
    endif()
    add_native_precompiled_header(${LIBRARY_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/pchdef.h)
  elseif(CMAKE_COMPILER_IS_GNUCXX)
    add_precompiled_header(${LIBRARY_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/pchdef.h)
    if(NOT ACE_USE_EXTERNAL)
      add_dependencies(game_pch_dephelp ACE_Project)
    endif()
  endif()
endif()
