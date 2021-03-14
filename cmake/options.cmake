option(DEBUG                "Include additional debug-code in core" OFF)
option(WARNINGS             "Show all warnings during compile"      OFF)
option(POSTGRESQL           "Use PostgreSQL"                        OFF)
option(PCH                  "Use precompiled headers"               ON)
option(BUILD_GAME_SERVER    "Build game server"                     ON)
option(BUILD_LOGIN_SERVER   "Build login server"                    ON)
option(BUILD_EXTRACTORS     "Build map/dbc/vmap/mmap extractors"    OFF)
option(BUILD_SCRIPTDEV      "Build ScriptDev. (OFF Speedup build)"  ON)
option(BUILD_PLAYERBOT      "Build Playerbot mod"                   OFF)
option(BUILD_AHBOT          "Build Auction House Bot mod"           OFF)
option(BUILD_METRICS        "Build Metrics, generate data for Grafana" OFF)
option(BUILD_RECASTDEMOMOD  "Build map/vmap/mmap viewer"            OFF)
option(BUILD_GIT_ID         "Build git_id"                          OFF)
option(BUILD_DOCS           "Build documentation with doxygen"      OFF)

# TODO: options that should be checked/created:
#option(CLI                  "With CLI"                              ON)
#option(RA                   "With Remote Access"                    OFF)
#option(SQL                  "Copy SQL files"                        OFF)
#option(TOOLS                "Build tools"                           OFF)

message("")
message(STATUS
  "This script builds the MaNGOS server.
  Options that can be used in order to configure the process:
    CMAKE_INSTALL_PREFIX    Path where the server should be installed to
    PCH                     Use precompiled headers
    DEBUG                   Include additional debug-code in core
    WARNINGS                Show all warnings during compile
    POSTGRESQL              Use PostgreSQL instead of mysql
    BUILD_GAME_SERVER       Build game server (core server)
    BUILD_LOGIN_SERVER      Build login server (auth server)
    BUILD_EXTRACTORS        Build map/dbc/vmap/mmap extractor
    BUILD_SCRIPTDEV         Build scriptdev. (Disable it to speedup build in dev mode by not including scripts)
    BUILD_PLAYERBOT         Build Playerbot mod
    BUILD_AHBOT             Build Auction House Bot mod
    BUILD_METRICS           Build Metrics, generate data for Grafana
    BUILD_RECASTDEMOMOD     Build map/vmap/mmap viewer
    BUILD_GIT_ID            Build git_id
    BUILD_DOCS              Build documentation with doxygen

  To set an option simply type -D<OPTION>=<VALUE> after 'cmake <srcs>'.
  Also, you can specify the generator with -G. see 'cmake --help' for more details
  For example:
    Build server
    cmake -DCMAKE_INSTALL_PREFIX=../opt/cmangos ..

    Build full server with extractor
    cmake -DCMAKE_INSTALL_PREFIX=../opt/cmangos -DBUILD_EXTRACTORS=ON ..

    Build only server without scripts and extractors and in debug mode
    cmake -DCMAKE_INSTALL_PREFIX=../opt/cmangos -DDEBUG=ON -DBUILD_ALL_SCRIPTS=OFF ..

    Build only login server
    cmake -DCMAKE_INSTALL_PREFIX=../opt/cmangos -DBUILD_GAME_SERVER=OFF .."
)
message("")
