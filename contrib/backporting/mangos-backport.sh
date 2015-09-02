#!/bin/bash

# The script works pretty simple - each time an empty commit is made,
# copying author and message from the original one.
# Then the original commit is cherry-picked -n (no commit)
# and the changes are git commit --amended to the prepared empty commit.

### general definitions

## user-tunable
AUTORESOLVE_FILES[0]="src/shared/revision_sql.h"
AUTORESOLVE_FILES[1]="src/shared/revision_nr.h"
AUTORESOLVE_FILES_REMOVE[0]=0
AUTORESOLVE_FILES_REMOVE[1]=1
GIT_AMEND_OPTS="-s"
GIT_RECOVER="git reset --hard HEAD^"
CONFLICT_RETVAL=2    # for batch usage
AUTOCOMMIT=1


### print error to stderr
function print_error {
  echo -e "${@}" 1>&2
}

### prints help
function print_help {
  echo -e "Usage: ${0##*/} [OPTIONS] <hash>" \
    "\nBackports a specified commit to current branch." \
    "\n\n  -n       Never automatically commit." \
    "\n"
}

### recover from an error, deleting empty commit
function git_recover {
  print_error "----------"
  print_error "Caught an error, checking for last commit ..."

  # check if the last commit is empty,
  # ie. it has the same tree object dependency as it's parent
  local head_tree=$(git log -1 --pretty="%T" HEAD)
  local prev_tree=$(git log -1 --pretty="%T" HEAD^)
  if [[ $head_tree == $prev_tree ]]; then
    print_error "Last commit empty, running ${GIT_RECOVER}" \
          "\nto previous HEAD (should be ${CURRENT_HEAD})."
    print_error "----------"
    ${GIT_RECOVER}
  else
    print_error "Last commit isn't empty (or git log failed) -" \
          "something strange happened,\ncheck git log" \
          "and do the cleanup (if needed) by hand."
  fi
  exit 1
}

### amend the empty commit, assigning new tree
function git_autoamend {

  # if the index is empty, there's nothing to amend
  if [[ -z $(git diff-index --cached HEAD) ]]; then
    git_retval=$?
    [[ $git_retval != 0 ]] && git_recover

    print_error "The index is empty, nothing to amend. This should" \
          "not happen during normal\nworkflow, so you" \
          "probably did something crazy like picking\na" \
          "commit to a branch where it already exists."
    git_recover
  fi

  git commit ${GIT_AMEND_OPTS} --amend -C HEAD
  git_retval=$?
  [[ $git_retval != 0 ]] && git_recover
}


### main()

## arguments

# arg parsing
while getopts "n" OPTION; do
  case $OPTION in
    n)
      AUTOCOMMIT=0
      ;;
    \?)
      print_help
      exit 1
      ;;
  esac
done;
shift $(( $OPTIND - 1 ))
ORIG_REF=${1}

# check for needed arguments
if [[ -z ${ORIG_REF} ]]; then
  print_help
  exit 1
fi

## startup checks

# check for needed commands
for cmd in git grep sed wc; do
  if [[ -z $(which ${cmd}) ]]; then
    print_error "error: ${cmd}: command not found"
    exit 1
  fi
done;

# are we in git root dir?
if [[ ! -d .git/ ]]; then
  print_error "error: not in repository root directory"
  exit 1
fi

# is the index clean?
if [[ ! -z $(git diff-index HEAD) ]]; then
  print_error "error: dirty index, run mixed/hard reset first"
  exit 1
fi

## original commit infos

# branch
PARENT_REPOSITORY=$(git branch -r --contains $ORIG_REF | sed 's% *\(.*\)/.*%\1%')
[[ $? != 0 ]] && exit 1

# remote
PARENT_REPOSITORY=$(git ls-remote --get-url $PARENT_REPOSITORY | sed 's%.*github.com.\(.*\)\.git%\1%')
[[ $? != 0 ]] && exit 1

# current HEAD commit hash
CURRENT_HEAD=$(git show -s --pretty=format:'%h' HEAD)
[[ $? != 0 ]] && exit 1

# author with email
COMMIT_AUTHOR=$(git show -s --pretty=format:'%an <%ae>' ${ORIG_REF})
[[ $? != 0 ]] && exit 1

# commit object hash (abbrev)
COMMIT_HASH=$(git show -s --pretty=format:'%h' ${ORIG_REF})
[[ $? != 0 ]] && exit 1

# subject (with removed revision number)
COMMIT_SUBJECT=$(git show -s --pretty=format:'%s' ${ORIG_REF} | sed -r 's/\[[a-z]?[0-9]*\] //')
[[ $? != 0 ]] && exit 1

# body
COMMIT_BODY=$(git show -s --pretty=format:'%b' ${ORIG_REF})
[[ $? != 0 ]] && exit 1

# whole message (yea, it could be done without echo)
COMMIT_MESSAGE=$(echo -e "${COMMIT_SUBJECT}\n\n${COMMIT_BODY}\n\n(based on ${PARENT_REPOSITORY}@${COMMIT_HASH})")
[[ $? != 0 ]] && exit 1

## new empty commit ready, so create it
git commit --author="${COMMIT_AUTHOR}" -m "${COMMIT_MESSAGE}" --allow-empty
[[ $? != 0 ]] && exit 1


## first, try cherry-picking the commit and catch conflicts.
## - if there are none, simply amend and exit
## - if there are none related to $AUTORESOLVE, only prepare
##   the backported commit
## - when multiple conflicts occur, including $AUTORESOLVE,
##   do the resolution for the one file, prepare the commit
##   and let user resolve the rest (+ amend)
## - when only single ($AUTORESOLVE) conflict occur, resolve it
##   and fire up $EDITOR to autocommit

pick_out=$(git cherry-pick -n ${ORIG_REF} 2>&1)
pick_retval=$?

# exit if there was a fatal app error
if [[ $pick_retval > 1 ]]; then
  print_error "${pick_out}"
  git_recover
fi

# get a list of unmerged files
unmerged_files=$(git diff-files --diff-filter=U | sed 's/^[^\t]*\t//')
git_retval=$?
if [[ $git_retval != 0 ]]; then
  print_error "${pick_out}"
  git_recover
fi

# simply amend if the pick was successful
if [[ $pick_retval == 0 && -z $unmerged_files ]]; then
  if [[ ${AUTOCOMMIT} > 0 ]]; then
    git_autoamend
  fi
  exit 0
fi

# sanity check
if [[ -z $unmerged_files ]]; then
  print_error "${pick_out}"
  print_error "----------"
  print_error "git cherry-pick failed with status 1," \
        "\nbut no unmerged files were found."
  git_recover
fi

for ((i=0; i<${#AUTORESOLVE_FILES[@]}; i++));
do

  AUTORESOLVE=${AUTORESOLVE_FILES[$i]}

  # if $AUTORESOLVE isn't there (but other conflicts are)
  if [[ -z $(echo "${unmerged_files}" | grep ${AUTORESOLVE}) ]]; then
    continue
  fi

  # do the resolution - use old version of the file
  if [[ -f ${AUTORESOLVE} ]]; then
    if [[ ${AUTORESOLVE_FILES_REMOVE[$i]} > 0 ]]; then
      [[ $? != 0 ]] && git_recover
      git rm -q ${AUTORESOLVE}
    else
      git show :2:${AUTORESOLVE} > ${AUTORESOLVE}
      [[ $? != 0 ]] && git_recover
      git add ${AUTORESOLVE}
    fi
    [[ $? != 0 ]] && git_recover
  else
    print_error "${pick_out}"
    print_error "----------"
    print_error "error: ${AUTORESOLVE} not found, cannot resolve"
    git_recover
  fi

done

unmerged_files=$(git diff-files --diff-filter=U | sed 's/^[^\t]*\t//')
git_retval=$?
if [[ $git_retval != 0 ]]; then
  print_error "${pick_out}"
  git_recover
fi

# if $AUTORESOLVE_FILES were the only conflicts, amend the commit
if [[ -z ${unmerged_files} ]]; then
  if [[ ${AUTOCOMMIT} > 0 ]]; then
    git_autoamend
  fi
  exit 0

# else let the user do all other conflict resolutions
else
  print_error "${pick_out}"
  echo "----------"
  echo "Please run git commit ${GIT_AMEND_OPTS} --amend" \
       "after resolving all conflicts."
  echo "To recover from the resolution, use ${GIT_RECOVER}."
  exit ${CONFLICT_RETVAL}
fi
