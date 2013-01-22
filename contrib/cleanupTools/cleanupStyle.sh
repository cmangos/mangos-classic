#!/bin/sh

# SUPPORTED: ASTYLE COMMENT_STYLE OVERRIDE_CORRECTNESS
# ASTYLE                    <- runs AStyle
# COMMENT_STYLE             <- converts //bla to // bla (note, // Bla might be better, but this would conflict with code (at least)
# Use at own Risk:
# OVERRIDE_CORRECTNESS[2]   <- Appends override correctness (add 2 for second pass)

# Do config stuff
sh "${0%/*}/cleanupToolConfig.sh"
if [ "$?" != "0" ]
then
  echo "You need to edit the configuration file before you can use this tool!"
  echo "Configuration file: ${0%/*}/cleanupTools.config"
  exit 0
fi

# And use config settings
. "${0%/*}/cleanupTools.config"


## Internal Stuff

# Mangos Cleanup options for AStyle
OPTIONS="--convert-tabs --align-pointer=type --suffix=none \
          --keep-one-line-blocks --keep-one-line-statements \
          --indent-classes --indent-switches --indent-namespaces \
          --pad-header --unpad-paren --pad-oper --style=allman"

if [ "$DO_ON" = "MANGOS_SRC" ]
then
  FILEPATH=$BASEPATH/src
  OPTIONS="$OPTIONS --exclude=ScriptDev2"
elif [ "$DO_ON" = "MANGOS_CONTRIB" ]
then
  FILEPATH=$BASEPATH/contrib
elif [ "$DO_ON" = "MANGOS_WHOLE" ]
then
  FILEPATH=$BASEPATH
  OPTIONS="$OPTIONS --exclude=ScriptDev2"
elif [ "$DO_ON" = "SD2" ]
then
  FILEPATH=$BASEPATH/src/bindings/ScriptDev2
else
  exit 1
fi

OPTIONS="$OPTIONS --recursive"

####################################################################################################
##                              USING ASTYLE UPDATING                                             ##
####################################################################################################
if [ $DO = ASTYLE ]
then
  echo "Process $FILEPATH with options $MOPTIONS"
  $ASTYLE $OPTIONS "$FILEPATH/*.cpp" "$FILEPATH/*.h"

  #Restore style of auto-generated sql-files
  if [ "$DO_ON" = "MANGOS_SRC" -o "$DO_ON" = "MANGOS_WHOLE" ]
  then
      if [ -f "$BASEPATH/src/shared/revision_nr.h" ]
      then
        sed 's/^#define REVISION_NR/ #define REVISION_NR/g' < ${BASEPATH}/src/shared/revision_nr.h > temp
        mv temp "$BASEPATH/src/shared/revision_nr.h"
      fi
      if [ -f "$BASEPATH/src/shared/revision_sql.h" ]
      then
        sed 's/^#define REVISION_DB/ #define REVISION_DB/g' < ${BASEPATH}/src/shared/revision_sql.h > temp
        mv temp "$BASEPATH/src/shared/revision_sql.h"
      fi
  elif [ "$DO_ON" = "SD2" ]
  then
    if [ -f "$BASEPATH/src/bindings/ScriptDev2/sd2_revision_nr.h" ]
      then
        sed 's/^#define SD2_REVISION_NR/ #define SD2_REVISION_NR/g' < ${BASEPATH}/src/bindings/ScriptDev2/sd2_revision_nr.h > temp
        mv temp "$BASEPATH/src/bindings/ScriptDev2/sd2_revision_nr.h"
      fi
  fi

  echo "Processing $DO on $DO_ON done"
  exit 0
fi


####################################################################################################
##                                     UNIFYING COMMENTS                                          ##
####################################################################################################
## Make comments neater //bla => // bla ## note for uppercase  s:\// \([a-z]\):// \1:g
if [ $DO = COMMENT_STYLE ]
then
  if [ "$DO_ON" = "MANGOS_SRC" ]
  then
    LIST=`find ${BASEPATH}/src/ -path "${BASEPATH}/src/bindings/ScriptDev2" -prune -o -type f -iname "*.cpp" -print -o -iname "*.h" -print `
  elif [ "$DO_ON" = "SD2" ]
  then
    LIST=`find ${BASEPATH}/src/bindings/ScriptDev2 -type f -iname "*.cpp" -o -iname "*.h" `
  else
    echo "Unsupported combination with DO and DO_ON"
    exit 1
  fi

  for l in $LIST
  do
    echo "Converting comments in $l"
    sed '/http:/ !s:\//\([a-zA-Z0-9]\):// \1:g;
         s#^\(............................................................\) [ ]*//#\1//#g;
         /^..............................................\/[\/]*$/ !s#^\(..................................................\)//#\1          //#g; ' $l > temp
    if [ "$?" != "0" ]
    then
      echo "An error HAPPENED"
      exit 1
    fi

    mv temp $l
  done

  echo "Processing $DO on $DO_ON done"
  exit 0
fi

####################################################################################################
##                                    OVERRIDE CORRECTNESS                                        ##
####################################################################################################
# use OVERRIDE_CORRECTNESS  to create helper file listing all virtual methods
# use OVERRIDE_CORRECTNESS2 to add the word "override" after all functions with virtual names

if [ "$DO_ON" != "MANGOS_SRC" -a "$DO_ON" != "SD2" ]
then
  echo "OVERRIDE atm only supported for MANGOS_SRC or SD2 target"
  exit 0
fi

if [ $DO = OVERRIDE_CORRECTNESS ]
then
  #Create a list of all virtual functions in mangos (class scope)
  if [ $DO_ON = MANGOS_SRC ]
  then
    HEADERLIST=`find ${BASEPATH}/src/ -path "${BASEPATH}/src/bindings/ScriptDev2" -prune -o -type f -iname "*.h" -print`
  else
    HEADERLIST=`find ${BASEPATH}/src/bindings/ScriptDev2 -type f -iname "*.h"`
    HEADERLIST="${HEADERLIST} ${BASEPATH}/src/game/CreatureAI.h"
    HEADERLIST="${HEADERLIST} ${BASEPATH}/src/game/InstanceData.h"
  fi

  rm virtuals
  for h in $HEADERLIST
  do
    grep "virtual" $h >> virtuals
  done

  #Filter comment lines
  sed -n '/^[ ]*\/\// !p' virtuals | sed -n '/^[ ]*\/\*/ !p' | \
  ##Get function name
  sed -r -n '/^[ ]*virtual[ ][ ]*const[\*]*[ ][ ]*[a-zA-Z_~0-9\*:&]*[ ][ ]*const[\*]*[ ][]*[a-zA-Z_~0-9]*[ ]*\(.*\)/ { s/^[ ]*virtual[ ]*const[ ]*[a-zA-Z_~0-9\*:]*[ ]*const[ ]*([a-zA-Z_~0-9]*).*/\1/g; tEnde}
       /^[ ]*virtual[ ][ ]*const[\*]*[ ][ ]*[a-zA-Z_~0-9\*:&]*[ ][ ]*[a-zA-Z_~0-9]*[ ]*\(.*\)/                       { s/^[ ]*virtual[ ]*const[ ]*[a-zA-Z_~0-9\*:]*[ ]*([a-zA-Z_~0-9]*).*/\1/g;          tEnde}
       /^[ ]*virtual[ ][ ]*[a-zA-Z_~0-9\*:&]*[ ][ ]*const[\*]*[ ][ ]*[a-zA-Z_~0-9]*[ ]*\(.*\)/                       { s/^[ ]*virtual[ ]*[a-zA-Z_~0-9\*:]*[ ]*const[\*]*[ ]*([a-zA-Z_~0-9]*).*/\1/g;          tEnde}
       /^[ ]*virtual[ ]*[a-zA-Z_~0-9\*:&]*[ ][ ]*[a-zA-Z_~0-9]*[ ]*\(.*\)/                                           { s/^[ ]*virtual[ ]*[a-zA-Z_~0-9\*:]*[ ][ ]*([a-zA-Z_~0-9]*).*/\1/g;}
       :Ende; p' > virtualNames

  ##Make its entries unique
  sort < virtualNames > virtuals
  uniq  < virtuals > virtualNames

  echo "Check file virtualNames for inconsitencies!"
  rm virtuals

  echo "Processing $DO on $DO_ON done"
  exit 0
fi

if [ $DO = OVERRIDE_CORRECTNESS2 ]
then
  ## Assume virtualNames is good
  if [ $DO_ON = MANGOS_SRC ]
  then
    FILELIST=`find ${BASEPATH}/src/ -path "${BASEPATH}/src/bindings/ScriptDev2" -prune -o -type f -iname "*.h" -print`
  else
    FILELIST=`find ${BASEPATH}/src/bindings/ScriptDev2 -type f -iname "*.cpp" -o -iname "*.h"`
  fi

  for h in $FILELIST
  do
    echo "Progress file $h"
    while read f
    do
      # basic pattern: NAME(..)...[;]
      PARSE="s/( "$f"\(.*\)[ ]*const[\*]*);/\1 override;/g; t
             s/( "$f"\(.*\));/\1 override;/g; t
             s/( "$f"\(.*\)[ ]*const[\*]*)$/\1 override/g; t
             s/( "$f"\(.*\))$/\1 override/g; t
             s/( "$f"\(.*\)[ ]*const[\*]*)([ ].*)/\1 override\2/g; t
             s/( "$f"\(.*\))([ ].*)/\1 override\2/g"
      sed -r "$PARSE" $h > temp
      mv temp $h
    done < virtualNames
  done

  echo "Processing $DO on $DO_ON done"
  exit 0

fi

exit 0
