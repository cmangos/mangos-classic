#!/bin/sh
# This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
# This program is free software licensed under GPL version 2
# Please see the included COPYING for full text */

###############################################################################
#
#   Simple helper script to help merging branches & pull requests
#
###############################################################################

#internal use
script_file_name="MergeHelper.sh"
config_file_name="_MergeHelper.config"

base_dir=${0%[/\\]*}
config_file=$base_dir/$config_file_name

DEBUG=0

no_ff_merge="--no-ff"

# #############################################################################
# Helper functions

# #############################################################################
# Re(create) the config file
function create_config {
cat > $config_file << EOF
###############################################################################
# This is the config file for the '$script_file_name' script
#
# The format to automatically merge (and fetch) remote branches is
#      repo/branch
#      repo2/branch2
#
# The format to automatically merge local branches is
#       branch1
#       branch2
#
# The format to automatically merge (and fetch) pull requests is
#      remote#PullRequestNumber1
#      remote#PullRequestNumber2
#
###############################################################################

# Which remote branches should be fetched and merged?
# Format to automatically fetch from a repo and merge the branch: <repo/branch>
# Example:
#origin/master


# Which local branches should be merged?
# Format to automatically merge a local branch: <branchName>
#some_cool_feature


# Which pull requests shall be merged from a repo?
# Format to automatically fetch and merge a pull request: <Repo#NumberOfPullRequest>
# Example:
#origin#17

# Enjoy using the tool
EOF
}

# #############################################################################
# Display help
function display_help {
  echo
  echo "Welcome to the CMaNGOS merge helper $script_file_name"
  echo
  echo "Run this tool from a bash compatible terminal (on windows like Git Bash)"
  echo
  echo "To configure edit the file \"$config_file\""
  echo
  echo "Supported options are"
  echo "  -h -- displays the help you currently see"
  echo "  -c -- specify config file (Default: $base_dir/$config_file_name )"
  echo "  -r -- change the remote from which you want to merge a pull-request"
  echo "  -l -- list available pull-requests from the remote (by default from origin)"
  echo "  -f -- do fast forward merges if possible (default: do not use fast-forward merges)"
  echo
  echo "Supported parameters are"
  echo "  XX -- Number which pull-reqest to merge"
  echo "        (Default PR pulled from origin, the remote can be changed with -r option"
  echo
}

# #############################################################################
# Function to fetch and merge a pull reqest
# Call with param1 == remote, param2 == Number
function merge_pull_request {
  echo "Now fetching and merging pull-request $2 (from remote $1)"
  git fetch $1 +refs/pull/$2/head:refs/remotes/$1/pr/$2
  [[ $? != 0 ]] && exit 1
  git merge $no_ff_merge $1/pr/$2
  [[ $? != 0 ]] && exit 1
}

# #############################################################################
# Main Method
# #############################################################################

# Must have Git available
git rev-parse --git-dir 1>/dev/null 2>&1
if [[ $? -ne 0 ]]; then
  echo "ERROR: This script must be used within a Git working tree"
  echo "Try to start from your main cmangos directory by using"
  echo "  \"contrib/$script_file_name\""
  read -p"Press [RETURN] to exit"
  exit 1
fi

# Config options
pull_remote="origin"
do_help=0
do_list=0
while getopts ":hc:r:lf" opt; do
  case $opt in
    h)
      do_help=1
      ;;
    c)
      config_file=$OPTARG
      echo "Using configuration file $config_file"
      ;;
    r)
      pull_remote=$OPTARG
      echo "Using remote $pull_remote for parameter pull-request merge"
      ;;
    l)
      do_list=1
      ;;
    f)
      no_ff_merge=""
      ;;
    \?)
      echo "Invalid option: -$OPTARG"
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument."
      exit 1
      ;;
  esac
done
shift $((OPTIND-1))

# Display help and exit
if [ $do_help -eq 1 ]; then
  display_help
  if [ ! -f $config_file ]; then
    create_config
  fi
  exit 0
fi

# List remotes and exit
if [ $do_list -eq 1 ]; then
  echo "Available Pull-Requests on remote $pull_remote:"
  git ls-remote $pull_remote pull/*/head | sed 's;.*refs/pull/\([0-9]*\)/head;  \1;g'
  exit 0
fi

# Called with Pull Request for direct merge
if [ "$1" != "" ]; then
  # Alternate call with pull-request number
  merge_pull_request $pull_remote $1
  exit 0
fi

# Check if config file present
if [ ! -f $config_file ]
then
  create_config
  display_help
  read -p"Press [RETURN] to exit"
  exit 1
fi

already_fetched_remotes=""
# The config file exists, read it line by line and use its content
while read cline
do
  if [ $DEBUG -eq 1 ]; then echo "DEBUG - Processing line $cline"; fi
  # Non-empty and non-comment line (comments start with #)
  if [[ "$cline" != "" && "$cline" != \#* ]]; then
    if echo $cline | egrep -s "^([[:alnum:]_-]+)#([[:digit:]]+)$" ; then
      # Pull-Request found - Format is <remote#number>
      remote=`echo $cline | sed -r 's@([[:alnum:]_-]+)#([[:digit:]]+)@\1@'`
      PR=`echo $cline | sed -r 's@([[:alnum:]_-]+)#([[:digit:]]+)@\2@'`

      if [ $DEBUG -eq 1 ]; then echo "DEBUG - pull request line found (remote $remote PR $PR)"; fi

      merge_pull_request $remote $PR

    elif echo $cline | egrep -s "^([[:alnum:]_-]+)/([[:alnum:]_-]+)$" ; then
      # remote branch found - Format is <remote/branch>
      remote=`echo $cline | sed -r 's@([[:alnum:]_-]+)/([[:alnum:]_-]+)@\1@'`
      branch=`echo $cline | sed -r 's@([[:alnum:]_-]+)/([[:alnum:]_-]+)@\2@'`

      if [ $DEBUG -eq 1 ]; then echo "DEBUG - remote branch line found (remote $remote branch $branch)"; fi

      found_remote=0
      for f in $already_fetched_remotes; do
        if [ "$f" = "$remote" ]; then
          found_remote=1
        fi
      done

      if [ $found_remote -eq 0 ]; then
        already_fetched_remotes="$already_fetched_remotes $remote"
        echo "Fetching $remote"
        git fetch $remote
        [[ $? != 0 ]] && exit 1
      fi

      echo "Merging $branch from $remote"
      git merge $no_ff_merge $remote/$branch
      [[ $? != 0 ]] && exit 1

    elif echo $cline | egrep -s "^([[:alnum:]_-]+)$" ; then
      # local branch found - Format is <branchName>
      branch=`echo $cline | sed -r 's@([[:alnum:]_-]+)@\1@'`

      if [ $DEBUG -eq 1 ]; then echo "DEBUG - local branch line found (branch $branch)"; fi

      git merge $no_ff_merge $branch
      [[ $? != 0 ]] && exit 1

    else
      echo "Unsupported config line found: $cline"
      echo "Aborting..."
      exit 1
    fi
  fi
done < $config_file

echo
echo "All done."
echo
exit 0
