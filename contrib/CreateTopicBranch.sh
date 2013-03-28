#!/bin/sh
# This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
# This program is free software licensed under GPL version 2
# Please see the included COPYING for full text */

## Dialogue helper to create a "topic-branch" for cmangos

# Steps in interactive dialog:
# 1) Get name of contribution (ie LFG)
# 2) Get type of contribution (feature, fix, rewrite, cleanup)
# 3) Get short description of contribution
# 4) Get place where the topic should be published (own repo --> address)
# 5) Check current state of worktree: dirty/clean;
#     if dirty ask if changes should be taken into new topic
#     ELSE ask if topic should be forked from origin/master or current branch
# Switch to topic-branch (ie "feature_LFG")
# Create initial commit (including some suggested commit message
# 6) Edit the suggested commit message
# 7) Push to repo (check for ok)
# Create output to link to this initial commit from a sticky thread on the forum (address of commit, short description)
# And ask the user to publish this content to some sticky thread

# Helper function   ###########################################################
# Optional param: line of prompt
function read_yes_no {
  while true; do
    if [ "$1" = "" ]; then
      read -p"(y/n) "line
    else
      read -p"$1 " line
    fi

    case "$line" in
      y|Y|yes|Yes|YES)
        return 1
        ;;
      n|N|no|No|NO)
        return 2
        ;;
      "")
        return 0
        ;;
      *)
        echo "Unknown answer, please retry"
        ;;
    esac
  done
}
# #############################################################################

# Main Script

echo "Welcome to CMaNGOS' helper tool to create a branch for a development project"
echo "This script will guide you through seven steps to set up a branch in which you can develop your project"
echo

# Must have Git available
git rev-parse --git-dir 1>/dev/null 2>&1
if [[ $? -ne 0 ]]; then
  echo "ERROR: Cannot find Git directory"
  echo "Try to start from your main cmangos directory by using"
  echo "  \"contrib/CreateTopicBranch.sh\""
  read -p"Press [RETURN] to exit"
  exit 1
fi

checkout_from_current=0
# is the index clean?
if [[ ! -z $(git diff-index HEAD) ]]
then
  echo "Your current working dir is not clean."
  echo "Do you want to take your changes over to your development project? (y/n)"
  read_yes_no "Move current changes? (y/n)"
  if [[ $? -eq 1 ]]
  then
    checkout_from_current=1
  else
    echo "ERROR: dirty index, run mixed/hard reset first"
    exit 1
  fi
fi

echo
echo "Please enter the name of your development project"
read project_name
# name must not include blanks
pr_name_clean=`echo $project_name | sed 's/ /_/g' `
if [ "$pr_name_clean" != "$project_name" ]
then
  echo "You entered a name with blanks, these were substituted to $pr_name_clean"
  read_yes_no "Is this ok for you? (y/n)"
  if [ $? -eq 2 ]; then exit 1; fi
fi
project_name="$pr_name_clean"
if [ "$project_name" = "" ]; then echo "ERROR: You must enter a name for your project"; exit 1; fi
echo
echo "What type of contribution will your project be? (examples: feature, rewrite, cleanup, fix)"
read project_type
# Type must not include blanks
pr_type_clean=`echo $project_type | sed 's/ /_/g' `
if [ "$pr_type_clean" != "$project_type" ]
then
  echo "You entered a type with blanks, these were substituted to $pr_type_clean"
  echo "Is this ok for you? (y/n)"
  read line
  if [ "$line" = "n" ]; then exit 1; fi
fi
project_type="$pr_type_clean"
if [ "$project_type" = "" ]; then echo "ERROR: You must enter a type for your project"; exit 1; fi
echo
echo "Please enter a short -oneline- description of your development project"
read project_desc
echo
echo "On which place do you intend to publish your contribution?"
echo "Currently you have these repositories created:"
echo
git remote
echo
echo "Please enter one of these git remotes"
read remote
remote_address=$(git remote -v | grep "$remote" | head -n1 | awk '{ print $2; }' | sed 's%.*github.com.\(.*\)\.git%\1%')
remote_address="https://github.com/${remote_address}"
echo
if [[ $checkout_from_current -eq 1 ]]
then
  git checkout -b ${project_type}_${project_name}
else
  echo "From where do you want to checkout to your development branch?"
  echo "Default: origin/master - Use \"HEAD\" for current branch"
  read checkout_point
  if [[ "$checkout_point" = "" ]]
  then
    checkout_point="origin/master"
  fi
  git checkout -b ${project_type}_${project_name} $checkout_point
fi
echo
echo "You will now asked to edit some initial commit message"
echo
echo "For this we will start your editor which might start vi, here are some information how to work with it:"
echo "Press \"i\" to start editing, Press ESC to stopp editing."
echo
echo "When you are finished editing, you can exit with [ESC] and \":wq\" "
echo
echo "You can exit the editor with [ESC] and \":wq\" !"
echo
echo "Press [ENTER] when you are ready to start the editor"
read line
echo
git commit -ae --allow-empty -m"Initial commit for development of $RP_TYPE $project_name

Development branch for $project_type

        $project_name

to do:
    $project_desc

This development project is published at:
    ${remote_address}/commits/${project_type}_${project_name}

# Please fill in some description about your project, these points might
# help you to get some structure to your detailed description:
# (Remember lines starting with # will be ignored)

# Overview:
# Expected steps:
# Goals:


# Happy coding!
"
echo

echo "Do you want to directly publish this development branch (suggested)? (y/n)"
read_yes_no "Directly push? (y/n)"
if [[ $? -le 1 ]]
then
  HASH=`git log HEAD ^HEAD^ --pretty=format:"%hn"`
  # Push now
  git push $remote ${project_type}_${project_name}
  echo
  echo "Please link to your started project on our forum!"
  echo
  echo "Here is some suggested content for publishing on the http://cmangos.net forums at:"
  echo "    <LINK>"
  echo "or for a custom project:    <LINK2>"
  echo
  echo "Development of $project_type $project_name - see [URL=${remote_address}/commit/$HASH]Initial commit[/URL]"
  echo "to develop: $project_desc"
  echo
  echo "In case of some contribution for official development, please open a pull request"
  echo
  echo "Note: On Windows you can use Copy&Paste from git-bash by clicking on the top-left corner, select \"Edit\" and then \"mark\""
  echo
fi
echo
echo "Have fun developing your feature!"
echo

exit 0
