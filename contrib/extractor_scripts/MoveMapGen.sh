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
## First param must be number of to be used CPUs (only 1, 2, 3, 4 supported) or "offmesh" to recreate the special tiles from the OFFMESH_FILE
## Second param can be an additional filename for storing log
## Third param can be an addition filename for storing detailed log

## Additional Parameters to be forwarded to MoveMapGen, see mmaps/readme for instructions
PARAMS="--silent --configInputPath config.json"

## Already a few map extracted, and don't care anymore
EXCLUDE_MAPS=""
#EXCLUDE_MAPS="0 1 530 571" # example to exclude the continents

## Offmesh file
OFFMESH_FILE="offmesh.txt"

## Normal log file (if not overwritten by second param
LOG_FILE="MoveMapGen.log"
## Detailed log file
DETAIL_LOG_FILE="MoveMapGen_detailed.log"

## ! Use below only for finetuning or if you know what you are doing !

## All maps
LIST_A="1"
LIST_B="0"
LIST_C="169"
LIST_D="269 533 35 44 389 129 109 450 34 249 48"
LIST_E="531 37 309 369"
LIST_F="229 409 43 70 90 189"
LIST_G="36 33 209 451 47"
LIST_H="509 30 289 230"
LIST_I="42 449 25 529 489 329 429 349 469 13"

badParam()
{
 echo "ERROR! Bad arguments!"
 echo "You can (re)extract mmaps with this helper script,"
 echo "or recreate only the tiles from the offmash file"
 echo
 echo "Call with number of processes (1 - 4) to create mmaps"
 echo "Call with 'offmesh' to reextract the tiles from offmash file"
 echo
 echo "For further fine-tuning edit this helper script"
 echo
}

if [ "$#" = "3" ]
then
 LOG_FILE=$2
 DETAIL_LOG_FILE=$3
elif [ "$#" = "2" ]
then
 LOG_FILE=$2
fi

# Offmesh file provided?
OFFMESH=""
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

# Function to process a list
createMMaps()
{
 for i in $@
 do
   for j in $EXCLUDE_MAPS
   do
     if [ "$i" = "$j" ]
     then
       continue 2
     fi
   done
   ./MoveMapGen $PARAMS $OFFMESH $i | tee -a $DETAIL_LOG_FILE
   echo "`date`: (Re)created map $i" | tee -a $LOG_FILE
 done
}

createHeader()
{
 echo "`date`: Start creating MoveMaps" | tee -a $LOG_FILE
 echo "Used params: $PARAMS $OFFMESH" | tee -a $LOG_FILE
 echo "Detailed log can be found in $DETAIL_LOG_FILE" | tee -a $LOG_FILE
 echo "Start creating MoveMaps" | tee -a $DETAIL_LOG_FILE
 echo
 echo "Be PATIENT - This will take a long time and might also have gaps between visible changes on the console."
 echo "WAIT until you are informed that 'creating MoveMaps' is 'finished'!"
}

# Create mmaps directory if not exist
if [ ! -d mmaps ]
then
 mkdir mmaps
fi

# Param control
case "$1" in
 "1" )
   createHeader $1
   createMMaps $LIST_A $LIST_B $LIST_C $LIST_D $LIST_F $LIST_G $LIST_H $LIST_I &
   ./MoveMapGen $PARAMS $OFFMESH --onlyGO
   ;;
 "2" )
   createHeader $1
   createMMaps $LIST_A $LIST_E $LIST_H $LIST_I &
   createMMaps $LIST_B $LIST_C $LIST_D $LIST_F $LIST_G &
   ./MoveMapGen $PARAMS $OFFMESH --onlyGO
   ;;
 "4" )
   createHeader $1
   createMMaps $LIST_A &
   createMMaps $LIST_B &
   createMMaps $LIST_C $LIST_F $LIST_G &
   createMMaps $LIST_D $LIST_E $LIST_H $LIST_I &
   ./MoveMapGen $PARAMS $OFFMESH --onlyGO
   ;;
 "8" )
   createHeader $1
   createMMaps $LIST_A &
   createMMaps $LIST_B &
   createMMaps $LIST_C &
   createMMaps $LIST_D &
   createMMaps $LIST_E &
   createMMaps $LIST_F $LIST_H &
   createMMaps $LIST_G &
   createMMaps $LIST_I &
   ./MoveMapGen $PARAMS $OFFMESH --onlyGO
   ;;
 "offmesh" )
   echo "`date`: Recreate offmeshs from file $OFFMESH_FILE" | tee -a $LOG_FILE
   echo "Recreate offmeshs from file $OFFMESH_FILE" | tee -a $DETAIL_LOG_FILE
   while read map tile line
   do
     ./MoveMapGen $PARAMS $OFFMESH $map --tile $tile | tee -a $DETAIL_LOG_FILE
     echo "`date`: Recreated $map $tile from $OFFMESH_FILE" | tee -a $LOG_FILE
   done < $OFFMESH_FILE &
   ;;
 * )
   badParam
   exit 1
   ;;
esac

wait

echo  | tee -a $LOG_FILE
echo  | tee -a $DETAIL_LOG_FILE
echo "`date`: Finished creating MoveMaps" | tee -a $LOG_FILE
echo "`date`: Finished creating MoveMaps" >> $DETAIL_LOG_FILE
