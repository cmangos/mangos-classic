#!/bin/sh

# Simple helper script to create backport lists (github markup based wiki format)
OUTPUT_FILE="contrib/backporting/update-commit-log.log"
COMMIT_FORMAT=": %h"
FULL_FORMAT="${COMMIT_FORMAT} * %an (committer %cn)%n\`\`\`%n%s%n\`\`\`"
FOOTER_FORMAT="FILE LAST UPDATED BASED ON... "

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

REMOTE=$(git branch -r --contains $HASH | sed 's% *\(.*\)%\1%')
[[ $? != 0 ]] && exit 1

REMOTE_NAME=$(echo $REMOTE | sed 's%\(.*\)/.*%\1%')
[[ $? != 0 ]] && exit 1

REPOSITORY=$(git ls-remote --get-url $REMOTE_NAME | sed 's%.*github.com.\(.*\)\.git%\1%')
[[ $? != 0 ]] && exit 1

LINK_FORMAT="[$REMOTE_NAME](http://github.com/$REPOSITORY/commit/%h)"

git log $HASH..$REMOTE --pretty=format:"* $LINK_FORMAT$FULL_FORMAT" --reverse >> $OUTPUT_FILE
echo "" >> $OUTPUT_FILE
echo "$(git log -1 --pretty="$FOOTER_FORMAT$LINK_FORMAT$COMMIT_FORMAT" $REMOTE)" >> $OUTPUT_FILE
echo "" >> $OUTPUT_FILE
