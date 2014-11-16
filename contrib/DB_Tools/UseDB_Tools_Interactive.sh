#!/bin/bash
# Only simple wrapper to call in interactive mode
SCRIPT_PATH="${BASH_SOURCE[0]}"
./`dirname ${SCRIPT_PATH}`/UseDB_Tools.sh -i $@
