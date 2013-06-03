#!/bin/sh

# Simple helper script to create backport lists

# By user defined (remote/branch to the to-be-backported history)
COMPARE_PATH="cmangos-tbc/master"
OUTPUT_FILE="contrib/backporting/todo_tbc_commits.log"

# By user defined (text format)
#SMALL_FORMAT="tbc(cleaned): %h * %an (committer %cn)"
#FULL_FORMAT="${SMALL_FORMAT}: %s"
#FOOTER_FORMAT="FILE LAST UPDATED BASED ON... ${SMALL_FORMAT}"

# By user defined (Textile markup based wiki format)
SMALL_FORMAT="\"tbc\":http://github.com/cmangos/mangos-tbc/commit/%h: %h * %an (committer %cn)"
FULL_FORMAT="</code></pre>%n%n%n* ${SMALL_FORMAT}<pre><code>%s"
FOOTER_FORMAT="</code></pre>%n%n%nFILE LAST UPDATED BASED ON... ${SMALL_FORMAT}"

# param1 must be the commit hash of last backported commit (of original commit)
if [ "$#" != "1" ]
then
  echo "You must provide the last commit's hash of the \"$OUTPUT_FILE\" file"
  exit 1
fi

# are we in git root dir?
if [[ ! -d .git/ ]]; then
  echo "ERROR: This script is expected to be called from repository root directory"
  echo "Try: contrib/backporting/update-commit-log.sh"
  exit 1
fi

HASH=$1

git log $HASH..$COMPARE_PATH --pretty=format:"${FULL_FORMAT}" --reverse --dirstat >> $OUTPUT_FILE
echo "" >> $OUTPUT_FILE
echo "$(git log -1 --pretty="${FOOTER_FORMAT}" $COMPARE_PATH)" >> $OUTPUT_FILE
echo "" >> $OUTPUT_FILE
