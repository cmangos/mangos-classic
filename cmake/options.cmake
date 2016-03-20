option(DEBUG                "Include additional debug-code in core" OFF)
option(WARNINGS             "Show all warnings during compile"      OFF)
option(TBB_USE_EXTERNAL     "Use external TBB"                      OFF)
if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  option(USE_STD_MALLOC     "Use standard malloc instead of TBB"    ON)
else()
  option(USE_STD_MALLOC     "Use standard malloc instead of TBB"    OFF)
endif()
option(POSTGRESQL           "Use PostgreSQL"                        OFF)

if(PCH)
  option(PCH                "Use precompiled headers"               ON)
else()
  option(PCH                "Use precompiled headers"               OFF)
endif()

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
    INCLUDE_BINDINGS_DIR    Include a script library in src/bindings/ with the
                            defined name. the name must corespond to the name of
                            the folder and the folder must contain a valid
                            CMakeLists.txt
    TBB_USE_EXTERNAL        Use external TBB
    USE_STD_MALLOC          Use standard malloc instead of TBB
  To set an option simply type -D<OPTION>=<VALUE> after 'cmake <srcs>'.
  Also, you can specify the generator with -G. see 'cmake --help' for more details
  For example: cmake .. -DDEBUG=1 -DCMAKE_INSTALL_PREFIX=/opt/mangos"
)
message("")
