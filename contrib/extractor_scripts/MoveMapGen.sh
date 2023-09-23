#!/bin/bash

# This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

## Syntax of this helper
## 1st param must be "maps" to generate the maps and offmesh or "offmesh" to recreate the special tiles from the OFFMESH_FILE
## 2nd param may be an additional filename for storing log
## 3rd param may be an addition filename for storing detailed log
## 4th param may be a number of threads to use for maps processing

PREFIX="$(dirname "$0")"

## Additional Parameters to be forwarded to MoveMapGen, see mmaps/readme for instructions
PARAMS="--silent --configInputPath config.json"

## Offmesh file
OFFMESH_FILE="offmesh.txt"

## Normal log file (if not overwritten by second param
LOG_FILE="MoveMapGen.log"
## Detailed log file
DETAIL_LOG_FILE="MoveMapGen_detailed.log"

badParam()
{
 echo "ERROR! Bad arguments!"
 echo "You can (re)extract mmaps with this helper script,"
 echo "or recreate only the tiles from the offmash file"
 echo
 echo "Call with 'maps' to create mmaps"
 echo "Call with 'offmesh' to reextract the tiles from offmash file"
 echo
}

if [ "$#" = "5" ]
then
  OUTPUT_PATH="$2"
  LOG_FILE="$3"
  DETAIL_LOG_FILE="$4"
  if [ "$5" != "all" ]
  then
    PARAMS="${PARAMS} --threads $5"
  fi
elif [ "$#" = "4" ]
then
  OUTPUT_PATH="$2"
  LOG_FILE="$3"
  DETAIL_LOG_FILE="$4"
elif [ "$#" = "3" ]
then
  OUTPUT_PATH="$2"
  LOG_FILE="$3"
fi

# Offmesh file provided?
OFFMESH=""
MMG_RES="--workdir ${OUTPUT_PATH:-.}/"
if [ "$OFFMESH_FILE" != "" ]
then
 if [ ! -f "$OFFMESH_FILE" ]
 then
   echo "ERROR! Offmesh file $OFFMESH_FILE could not be found."
   echo "Provide valid file or none. You need to edit the script"
   exit 1
 else
   OFFMESH="--offMeshInput $OFFMESH_FILE"
 fi
fi

createHeader()
{
 echo "$(date): Start creating MoveMaps" | tee -a "$LOG_FILE"
 echo "Used params: $PARAMS $OFFMESH" | tee -a "$LOG_FILE"
 echo "Detailed log can be found in $DETAIL_LOG_FILE" | tee -a "$LOG_FILE"
 echo "Start creating MoveMaps" | tee -a "$DETAIL_LOG_FILE"
 echo
 echo "Be PATIENT - This may take a long time with small number of threads and might also have gaps between visible changes on the console."
 echo "WAIT until you are informed that 'creating MoveMaps' is 'finished'!"
}

# Create mmaps directory if not exist
if [ ! -d mmaps ]
then
 mkdir "${OUTPUT_PATH:-.}/mmaps"
fi

# Param control
case "$1" in

 "maps" )
    createHeader
    "$PREFIX"/MoveMapGen $PARAMS $OFFMESH $MMG_RES --buildGameObjects | tee -a "$DETAIL_LOG_FILE"
   ;;
 "offmesh" )
   echo "$(date): Recreate offmeshes from file $OFFMESH_FILE" | tee -a "$LOG_FILE"
   echo "Recreate offmeshes from file $OFFMESH_FILE" | tee -a "$DETAIL_LOG_FILE"
   while read map tile line
   do
     "$PREFIX"/MoveMapGen $PARAMS $OFFMESH $MMG_RES "$map" --tile "$tile" | tee -a "$DETAIL_LOG_FILE"
     echo "$(date): Recreated $map $tile from $OFFMESH_FILE" | tee -a "$LOG_FILE"
   done < $OFFMESH_FILE &
   ;;
 * )
   badParam
   exit 1
   ;;
esac

wait

echo  | tee -a "$LOG_FILE"
echo  | tee -a "$DETAIL_LOG_FILE"
echo "$(date): Finished creating MoveMaps" | tee -a "$LOG_FILE"
echo "$(date): Finished creating MoveMaps" >> "$DETAIL_LOG_FILE"
