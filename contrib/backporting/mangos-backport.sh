#!/bin/bash
#
# mangos-backport - a bash helper for backporting in git for MaNGOS project
#  Copyright (C) 2009  freghar <compmancz@gmail.com>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
#  02110-1301, USA
#

# The script works pretty simple - each time an empty commit is made,
# copying author and message from the original one.
# Then the original commit is cherry-picked -n (no commit)
# and the changes are git commit --amended to the prepared empty commit.

### general definitions

## internals
BATCH_PROCESS=0
VERBOSE=0

## user-tunable
AUTORESOLVE="src/shared/revision_nr.h"
GIT_AMEND_OPTS="-s"
GIT_RECOVER="git reset --hard HEAD^"
CONFLICT_RETVAL=2    # for batch usage
GIT_PATH="./"


### print error to stderr
function print_error {
	echo -e "${@}" 1>&2
}

### prints help
function print_help {
	echo -e "Usage: ${0##*/} [-vb] <revspec>" \
		"\nBackports a specified commit to current branch." \
		"\n\n  -b       Batch processing (no interaction)." \
		"\n           (runs amend without calling \$EDITOR)" \
		"\n  -v       Be verbose." \
		"\n  -n       Never automatically commit." \
		"\n"
}

### verbose print
function verbose_print {
	[[ ${VERBOSE} > 0 ]] && echo "${@}"
}

### runs a command and handles it's output verbosity
#function verbose_run {
#	if [[ ${VERBOSE} > 0 ]]; then
#		"${@}"
#		return $?
#	else
#		"${@}" 1 > /dev/null
#		return $?
#	fi
#}

### catches output of a command and returns it's retval
#function catch_out {
#		pick_out=$(${@} 2>&1)
#		return $?
#}

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

	verbose_print "----------"
	if [[ ${BATCH_PROCESS} > 0 ]]; then
		if [[ ${VERBOSE} > 0 ]]; then
			git commit ${GIT_AMEND_OPTS} --amend -C HEAD
			git_retval=$?
		else
			git commit ${GIT_AMEND_OPTS} --amend -C HEAD 1> /dev/null
			git_retval=$?
			[[ $git_retval == 0 ]] && echo \
						"Commit ${COMMIT_HASH} picked."
		fi
	else
		git commit ${GIT_AMEND_OPTS} --amend -c HEAD
		git_retval=$?
	fi
	[[ $git_retval != 0 ]] && git_recover
}


### main()

## arguments

# arg parsing
while getopts "vbn" OPTION; do
	case $OPTION in
		v)
			VERBOSE=1
			;;
		b)
			BATCH_PROCESS=1
			;;
		n)
			NO_AUTOCOMMIT=1
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
COMMIT_REVISION=$(git show -s --pretty=format:'%s' ${ORIG_REF} | sed -nr 's/^\[([a-z]?[0-9]*).*/\1/p')
[[ $? != 0 ]] && exit 1
if [ "$COMMIT_REVISION" != "" ]; then COMMIT_REVISION="[$COMMIT_REVISION] - "; fi

# body
COMMIT_BODY=$(git show -s --pretty=format:'%b' ${ORIG_REF})
[[ $? != 0 ]] && exit 1

# whole message (yea, it could be done without echo)
COMMIT_MESSAGE=$(echo -e "${COMMIT_SUBJECT}\n\n${COMMIT_BODY}\n\n(based on commit $COMMIT_REVISION${COMMIT_HASH})")
[[ $? != 0 ]] && exit 1

## new empty commit ready, so create it
verbose_print "Creating new empty commit on current HEAD (${CURRENT_HEAD}}."
verbose_print "----------"
if [[ ${VERBOSE} > 0 ]]; then
	git commit --author="${COMMIT_AUTHOR}" -m "${COMMIT_MESSAGE}" \
		--allow-empty
	[[ $? != 0 ]] && exit 1
else
	git commit --author="${COMMIT_AUTHOR}" -m "${COMMIT_MESSAGE}" \
		--allow-empty 1> /dev/null
	[[ $? != 0 ]] && exit 1
fi


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
	verbose_print "${pick_out}"
	verbose_print "----------"
	if [[ ${NO_AUTOCOMMIT} > 0 ]]; then
		verbose_print "No conflicts to resolve, nothing to do."
		verbose_print "Please run git commit ${GIT_AMEND_OPTS} --amend" \
			      "after making all necessary changes."
		verbose_print "Use ${GIT_RECOVER} to recover."
	else
		verbose_print "No conflicts to resolve, running amend."
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

# if $AUTORESOLVE isn't there (but other conflicts are), simply exit
if [[ -z $(echo "${unmerged_files}" | grep ${AUTORESOLVE}) ]]; then
	print_error "${pick_out}"
	echo "----------"
	verbose_print "${AUTORESOLVE} not found as unmerged."
	echo "Please run git commit ${GIT_AMEND_OPTS} --amend" \
	     "after resolving all conflicts."
	echo "To recover from the resolution, use ${GIT_RECOVER}."
	exit ${CONFLICT_RETVAL}
fi

# do the resolution - use old version of the file
if [[ -f ${AUTORESOLVE} ]]; then
	verbose_print "${pick_out}"
	verbose_print "----------"
	verbose_print "Auto-resolving ${AUTORESOLVE} using old version."
	git show :2:${AUTORESOLVE} > ${AUTORESOLVE}
	[[ $? != 0 ]] && git_recover
	git add ${AUTORESOLVE}
	[[ $? != 0 ]] && git_recover
#	echo "Resolution of ${AUTORESOLVE} finished successfuly."
else
	print_error "${pick_out}"
	print_error "----------"
	print_error "error: ${AUTORESOLVE} not found, cannot resolve"
	git_recover
fi

# if $AUTORESOLVE was the only conflict, amend the commit
if [[ $(echo "${unmerged_files}" | wc -l) == 1 ]]; then
	verbose_print "----------"
	if [[ ${NO_AUTOCOMMIT} > 0 ]]; then
		verbose_print "All done, autocommit disabled, nothing to do."
		verbose_print "Please run git commit ${GIT_AMEND_OPTS} --amend" \
			      "after making all necessary changes."
		verbose_print "Use ${GIT_RECOVER} to recover."
	else
		verbose_print "All done, running git commit ${GIT_AMEND_OPTS} --amend ..."
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
