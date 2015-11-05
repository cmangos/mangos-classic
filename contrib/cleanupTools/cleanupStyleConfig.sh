#!/bin/sh

# Basic configuration for the cleanup helper scripts with mangos
# Be sure to read this whole file and edit the variables as required
#
############################################################
#               Generic options:
############################################################
# Define the path to <MaNGOS> folder relatively from where you want to call the scripts.
# The cleanupHistory.sh script must be called from within the repository that is to be cleaned!
#
BASEPATH="."
#
# Path to Astyle from the <MaNGOS> directory
ASTYLE="../AStyle/bin/AStyle.exe"


############################################################
#               The cleanupStyle.sh tool:
############################################################
# Job selection options are:
#   DO with possible choices:       ASTYLE (to call the Astyle tool)
#                                   COMMENT_STYLE (to change comments from //foo to // foo and align them to column 61 if possible
#                                   OVERRIDE_CORRECTNESS[2] use at own RISK, this is maybe not too usefull
DO="ASTYLE"
#
#   DO_ON with possible choices:    MANGOS_SRC      to invoke the above cleanup routine on <MaNGOS>/src directory and subdirectories
#                                   MANGOS_CONTRIB  to invoke the above cleanup routine on <MaNGOS>/contrib directory and subdirectories
#                                   MANGOS_WHOLE    to invoke the above cleanup routine on <MaNGOS> directory and subdirectories
#                                   SD2             to invoke the above cleanup routine on <MaNGOS>/src/bindings/ScriptDev2 directory and subdirectories
DO_ON="MANGOS_SRC"


############################################################
#               The cleanupHistory.sh tool:
############################################################
# This tool will USE the cleanupStyle.sh tool to cleanup the history of the current branch with the settings above
# It will clean the history based on the variable BASE_COMMIT that needs to be defined here
BASE_COMMIT=""



###
# Internal variables, do not change except you know what you are doing
###
# cleanup-tool
CLEANUP_TOOL="$BASEPATH/contrib/cleanupTools/cleanupStyle.sh"
# path from current's caller position to ASTYLE
ASTYLE="$BASEPATH/$ASTYLE"

