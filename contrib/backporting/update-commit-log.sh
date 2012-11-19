#!/bin/sh

# Simple helper script to create SD2-Backporting Lists

# By user defined (remote/branch to the to-be-backported history)
COMPARE_PATH="tbc_cleaned"
TODO_FILE="contrib/backporting/todo_tbc_commits.log"
COMPARE_NAME="TBC(cleaned)"

# param1 must be the commit hash of last backported commit (of original commit)
if [ "$#" != "1" ]
then
  echo "You must provide the last commit's hash of the \"$TODO_FILE\" file"
  exit 1
fi

# are we in git root dir?
if [[ ! -d .git/ ]]; then
  echo "ERROR: This script is expected to be called from repository root directory"
  echo "Try: tool/backporting/update-commit-log.sh"
  exit 1
fi

HASH=$1

git log $HASH..$COMPARE_PATH --pretty=format:"${COMPARE_NAME}_COMMIT %h  by %an (commiter %cn) \" %s \"" --reverse --dirstat >> $TODO_FILE
echo "" >> $TODO_FILE
echo "FILE LAST UPDATE BASED ON $(git log -1 --pretty="%h (by %an) \" %s \"" $COMPARE_PATH)" >> $TODO_FILE
echo "" >> $TODO_FILE
