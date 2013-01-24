#!/bin/sh
#
# Tool to rebase a branch with cleaned commits
#
# Required:
# - some tool for the cleanup
# - base commit from which on the current branch shall be cleaned

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

if [ "$BASE_COMMIT" = "" ]
then
  echo "You did not specify a base-commit onto which to rebuild history"
  echo "or another unexpected error happened.."
  exit 1
fi

# First: commit all changes onto current branch
git commit -a -m"Cleanup History Commit: Pending Changes of WorkDir"

# Create list of commits that needs to be processed
COMMIT_LIST=`git log --reverse --format=format:"%H" $BASE_COMMIT..HEAD`

# prepare history_cleanup branch
git checkout -b history_cleanup $BASE_COMMIT
# cleanup history
echo "Cleanup History Tool: Cleanup initial state before rewriting history(might result in an empty commit)"
$CLEANUP_TOOL > /dev/null
git commit -a -m"Cleanup History Commit: Cleanup state before starting cleaning history"

for commit in $COMMIT_LIST
do
  git checkout -b history_cleanup_tmp $commit
  [[ $? != 0 ]] && exit 1
  echo "Cleanup History Tool: Cleanup after commit $commit)"
  $CLEANUP_TOOL > /dev/null
  git commit -a -m"Cleanup History Commit: Temp cleaned" --allow-empty
  [[ $? != 0 ]] && exit 1
  git checkout history_cleanup
  [[ $? != 0 ]] && exit 1

  ## Catch differences
  git diff --binary history_cleanup..history_cleanup_tmp > history_cleanup_tmp.patch

  # Get committer information to store it
  COMMITTER_N=$(git show -s --pretty=format:'%cn' ${commit})
  [[ $? != 0 ]] && exit 1
  COMMITTER_M=$(git show -s --pretty=format:'%ce' ${commit})
  [[ $? != 0 ]] && exit 1
  COMMITTER_D=$(git show -s --pretty=format:'%cd' ${commit})
  [[ $? != 0 ]] && exit 1
  GIT_COMMITTER_NAME=$COMMITTER_N; GIT_COMMITTER_EMAIL=$COMMITTER_M; GIT_COMMITTER_DATE=$COMMITTER_D;
  export GIT_COMMITTER_NAME; export GIT_COMMITTER_EMAIL; export GIT_COMMITTER_DATE;

  # Two cases: either diff is identical to original commit, or not
  git diff --binary $commit^..$commit > history_cleanup_compare.patch
  DIFF_COUNT=`diff history_cleanup_tmp.patch history_cleanup_compare.patch | grep ">" | grep -v "> index " | wc -l`
  echo "Current State: Diffcount=$DIFF_COUNT"
  if [ "$DIFF_COUNT" = "      0" ]
  then  # identical# TODO this is too strict!
    git cherry-pick $commit
  else  # different, use new commit
      # commit object hash
      COMMIT_HASH=$(git show -s --pretty=format:'%h' ${commit})
      [[ $? != 0 ]] && exit 1
      # subject
      COMMIT_SUBJECT=$(git show -s --pretty=format:'%s' ${commit})
      [[ $? != 0 ]] && exit 1
      # body
      COMMIT_BODY=$(git show -s --pretty=format:'%b' ${commit})
      [[ $? != 0 ]] && exit 1
      # whole message
      COMMIT_MESSAGE=$(echo -e "${COMMIT_SUBJECT}\n\n${COMMIT_BODY}\n\n(cleaned version of ${COMMIT_HASH})")
      [[ $? != 0 ]] && exit 1

      # Apply Cleaned changeset
      git apply history_cleanup_tmp.patch
      if [ $? != 0 ]
      then
        echo "Could not apply patch with git methods, try with patch tool directly? (y/n)"
        read line
        if [ "$line" = "y" ]
        then
          patch -p1 -d. < history_cleanup_tmp.patch
          echo "Please check manually if this worked.."
          echo "Did it work? (y/n)"
          read line
          if [ "$line" != "y" ]
          then
            exit 1
          fi
        fi
      fi
      if [ $? != 0 ]
      then
        echo "Could not apply patch, store in history_cleanup_tmp.patch"
        exit 1
      fi

      # Git add new files
      NEW_FILES_LIST=`grep -B1 "new file mode" history_cleanup_tmp.patch | grep "diff --git a/" | sed 's!diff --git a/\(.*\) .*!\1!'`
      for fline in $NEW_FILES_LIST
      do
        git add $fline
        [[ $? != 0 ]] && exit 1
      done

      # Commit just created commit with original information
      git commit -a --allow-empty -C"$commit"
      [[ $? != 0 ]] && exit 1
      git commit --amend --allow-empty -m"${COMMIT_MESSAGE}"
      [[ $? != 0 ]] && exit 1
  fi

  git branch -D history_cleanup_tmp
  [[ $? != 0 ]] && exit 1
done

rm history_cleanup_tmp.patch
rm history_cleanup_compare.patch

GIT_COMMITTER_NAME=""; GIT_COMMITTER_EMAIL=""; GIT_COMMITTER_DATE="";
export GIT_COMMITTER_NAME; export GIT_COMMITTER_EMAIL; export GIT_COMMITTER_DATE;
