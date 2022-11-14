#!/usr/bin/env bash
# Script to generate a changelog in "Keep a Changelog" format from git log.
# Only commits starting from an exclamation mark are added to changelog. For
# more details about expected commit message format see
# https://portal.softeq.com/display/SEDH/Git+flow#Gitflow-Commitmessage.
#
# The script updates a single section related to current release (if current
# branch is release branch) or "Unreleased" section (if current branch isn't
# release branch). All other sections in CHANGELOG.md remain as is. It's safe to
# edit generated changelog manually before creating a release commit, those
# modifications will present in all future releases.
#
# The script check if current branch name starts with release/$SEMVER, then
# parse git-log to gather feature lists starting from HEAD commit to previous
# release found in CHANGELOG.md. If CHANGELOG.md already contains $SEMVER
# section the section will be regenerated from scratch.
# CHANGELOG.md example in release/0.2.0 branch:
#   # Changelog
#   Some information
#
#   ## [0.2.0] - date
#   ### Added
#   - feature1 (going to be regenerated)
#   - feature2
#
#   ## [0.1.0] - date
#   ### Added
#   - feature3 (with some manual modifications)
#
# In example above all commits until release commit 0.1.0 will be added to
# [0.2.0] section. Previous content at [0.2.0] section removed. Section [0.1.0]
# remains as is.
#
# If current branch is not release branch, the script parses git-log until the
# last release mentioned in CHANGELOG.md. All features go to "Unreleased"
# section. If the section is present in CHANGELOG.md, the content of the section
# is wiped out and generated from scratch.
# CHANGELOG.md example in feature/feature-100500 branch:
#   # Changelog
#   Some information
#
#   ## Unreleased
#   ### Added
#   - feature4
#   - Content of this section wiped out and regenerated from git-log
#
#   ## [0.2.0] - date
#   ### Added
#   - feature1 (this content remains the same as in CHANGELOG.md)
#   - feature2
#
#   ## [0.1.0] - date
#   ### Added
#   - feature3 (with some manual modifications)
#
# NOTE: If CHANGELOG.md doesn't exist, print default CHANGELOG with no version
# info.
#
# See https://keepachangelog.com/en/1.0.0/ for change log format details.

IFS=$'\n'

# An array of section names present in CHANGELOG file. Sections are printed in
# the same order as listed here.
CHANGELOG_SECTIONS=("HEAD")
# Changelog sections' content. {section_name: content}
declare -A CHANGELOG
# Link section. Contains a generated URLs for each release. {section_name: URL}
declare -A LINKS

# Convert URL https://USER@SERVER_ADDR/scm/PROJECTNAME/REPONAME.git to
# https://SERVER_ADDR/projects/PROJECTNAME/repos/REPONAME
REPO_URL=$(git config --get remote.origin.url | sed -En 's/(.*\/\/)(.*@)?(.*)\/scm\/(.*)\/(.*)\.git/\1\3\/projects\/\4\/repos\/\5/p')
if [[ "${REPO_URL}" == "" ]]; then
    # Try to convert cloned by SSH repo:
    # ssh://git@SERVER_ADDR:7999/PROJECTNAME/REPONAME.git to
    # https://SERVER_ADDR/projects/PROJECTNAME/repos/REPONAME
    REPO_URL=$(git config --get remote.origin.url | sed -En 's/(.*\/\/)(.*@)?(.*):[0-9]+\/(.*)\/(.*)\.git/https:\/\/\3\/projects\/\4\/repos\/\5/p')

    if [[ "${REPO_URL}" == "" ]]; then
        # Same as above, but without port number after server
        REPO_URL=$(git config --get remote.origin.url | sed -En 's/(.*\/\/)(.*@)?(.*)\/(.*)\/(.*)\.git/https:\/\/\3\/projects\/\4\/repos\/\5/p')
    fi
fi
# Store master branch. Used to compare current head with last release.
MASTER_BRANCH=$(git config --get branch.master.merge) # refs/heads/master

# Get project name from repo URL
if [[ "${REPO_URL}" != "" ]]; then
    PROJECT_NAME=$(basename ${REPO_URL})
else
    TOP_DIR=$(git rev-parse --show-toplevel)
    PROJECT_NAME=$(basename ${TOP_DIR})

    verbose_print "$(basename $0): Failed to get repo URL. Links section is not going to be updated"
    verbose_print "$(basename $0): remote.origin.url: $(git config --get remote.origin.url)"
fi

# Setup a default changelog header
read -r -d '' CHANGELOG["HEAD"] << EOM
# ${PROJECT_NAME}
All notable changes to this project will be documented in this file.

The format of the file is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
Please use [Release workflow](https://portal.softeq.com/pages/viewpage.action?spaceKey=SEDH&title=Git+flow#Gitflow-Releaseworkflow) and [Commit messages style guide](https://portal.softeq.com/pages/viewpage.action?spaceKey=SEDH&title=Git+flow#Gitflow-Commitmessagesstyleguide)

See [README.md](README.md) for more details.
EOM
CHANGELOG["HEAD"]+=$'\n'

# Extract feature description from commit message
# arg1: line containing commit message
function cleanupCommit() {
    commitMessage=$(echo "$1" | sed -En 's/.*(\![\=|\+|\-])[[:space:]]+\[.*\](.*)/\2/p')
    commitMessage=$(echo "$commitMessage" | sed -E 's/^[[[:space:]]+|\:|]\:[[:space:]]+//g')
    commitMessage=$(echo "$commitMessage" | sed -E 's/^[[:space:]]//g')
    echo "$commitMessage"
    return 0
}

# Generate description for ${RELEASE}
# Used variables:
#   RELEASE:                release name
#   RELEASE_DATE:           release date (may be empty)
#   ADD, CHANGE, REMOVE: arrays of added/changed/removed features
function generate_release_description() {
    echo -n "## [${RELEASE}]"
    if [[ "${RELEASE_DATE}" != "" ]]; then # "Unreleased" has no date
        echo -n " - ${RELEASE_DATE}"
    fi
    echo

    if [[ ${#ADD[@]} -ne 0 ]]; then
        echo "### Added"
        for feature in "${ADD[@]}"; do
            echo "- $feature"
        done
        echo
    fi
    if [[ ${#CHANGE[@]} -ne 0 ]]; then
        echo "### Changed"
        for feature in "${CHANGE[@]}"; do
            echo "- $feature"
        done
        echo
    fi
    if [[ ${#REMOVED[@]} -ne 0 ]]; then
        echo "### Removed"
        for feature in "${REMOVED[@]}"; do
            echo "- $feature"
        done
        echo
    fi

    return 0
}

# Get release name from git-log line
# arg1: A single line produced by `git log --sparse --oneline --decorate=full`
# return: Release name or empty string if commit wasn't merged from release
#         branch.
#
# The line describes a release if it has been merged from
# origin/release/$VERSION_NUMBER branch. For example:
# f08bd69 (tag: refs/tags/0.3.0, refs/remotes/origin/release/0.3.0) = [OCULUS-7] Library release 0.3.0
# f08bd69 (HEAD -> refs/heads/release/0.3.0, tag: refs/tags/0.3.0, refs/remotes/origin/release/0.3.0) = [OCULUS-7] Library release 0.3.0
# 5247e5b (HEAD -> refs/heads/release/3.0.0, refs/heads/master) !+ [JIRA-10]: Add main.c
function get_release_name() {
    REFS=$(echo "$1" | cut -d' ' -f2- | cut -d')' -f1)
    REFS="${REFS:1}"  # remove leading bracket
    for ref in ${REFS//,/$IFS}; do
        RELEASE_NAME=$(echo "${ref}" | sed -En 's/.*(origin|heads)\/release\/(.*)/\2/p')
        if [[ "${RELEASE_NAME}" != "" ]]; then
            echo "${RELEASE_NAME}"
            return 0
        fi
    done

    return 0
}

# Get release tag or revision from git-log line
# arg1: A single line produced by `git log --sparse --oneline --decorate=full`
function get_release_tag() {
    REFS=$(echo "$1" | cut -d' ' -f2- | cut -d')' -f1)
    REFS="${REFS:1}"  # remove leading bracket
    for ref in ${REFS//,/$IFS}; do
        TAG=$(echo "${ref}" | sed -n 's/.*tag: \(.*\)/\1/p')
        if [[ "${TAG}" != "" ]]; then
            echo "${TAG}"
            return 0
        fi
    done

    REV=$(echo "$1" | cut -d' ' -f1)
    echo "${REV}"
    return 0
}

# Print a message to stderr if ${VERBOSE} is not clear
# arg1: a string to print
function verbose_print() {
    if [[ ! -z "${VERBOSE}" ]]; then 
        >&2 echo $1
    fi
}

# Parse command line options
while [[ $# -gt 0 ]]; do
    case $1 in
    -v|--verbose)
        VERBOSE="y"
        shift
        ;;
    -o|--out)
        OUTFILE="$2"
        shift
        shift
        ;;
    -h|--help)
        echo "Usage: $(basename $0) [-v|--verbose] [-o|--out FILE] [-h|--help]"
        echo ""
        echo "Change log generator."
        echo "Script to generate a changelog in "Keep a Changelog" format from git log."
        echo "Only commits starting from an exclamation mark are added to changelog. For"
        echo "more details about expected commit message format see"
        echo "https://portal.softeq.com/display/SEDH/Git+flow#Gitflow-Commitmessage."
        echo ""
        echo "The script updates a single section related to current release (if current"
        echo "branch is release branch) or "Unreleased" section (if current branch isn't"
        echo "release branch). All other sections in CHANGELOG.md remain as is. It's safe to"
        echo "edit generated changelog manually before creating a release commit, those"
        echo "modifications will present in all future releases."
        echo ""
        echo "Options:"
        echo "  -v, --verbose    Print some debug info to stderr"
        echo "  -o, --out FILE   Print changelog to specified file"
        echo "  -h, --help       Print this help and exit"
        echo ""
        echo "Example usage:"
        echo "  $(basename $0) -o CHANGELOG.md"
        echo "Command above updates CHANGELOG.md in current directory"
        exit 0
        ;;
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
    esac
done

################################################################################
# If CHANGELOG.md exists parse it to load existing sections
################################################################################
if [[ -s "CHANGELOG.md" ]]; then
    verbose_print "CHANGELOG.md: Read changelog"
    CUR_SECTION="HEAD"
    CHANGELOG["${CUR_SECTION}"]=""

    while read line; do
        section=$(echo "$line" | sed -En 's/^## \[(([0-9]+\.[0-9]+\.[0-9]+.*)|Unreleased)\].*/\1/p')
        if [[ "${section}" != "" ]]; then
            CUR_SECTION=${section}
            CHANGELOG_SECTIONS+=("${section}")
            verbose_print "CHANGELOG.md: Found section ${section}"
        fi
        LINK_NAME=$(echo "$line" | sed -En 's/^\[(([0-9]+\.[0-9]+\.[0-9]+.*)|Unreleased)\]: (.*)/\1/p')
        if [[ "${LINK_NAME}" != "" ]] && [[ -v "CHANGELOG[${LINK_NAME}]" ]]; then
            LINK_URL=$(echo "$line" | sed -En 's/^\[(([0-9]+\.[0-9]+\.[0-9]+.*)|Unreleased)\]: (.+)/\3/p')
            LINKS["${LINK_NAME}"]="${LINK_URL}"
            verbose_print "CHANGELOG.md: Found link ${LINK_NAME} to ${LINK_URL}"
        else
            if [[ "${CHANGELOG["${CUR_SECTION}"]}" != "" ]]; then
                CHANGELOG["${CUR_SECTION}"]+=$'\n'
            fi
            CHANGELOG["${CUR_SECTION}"]+=$line
        fi
    done < CHANGELOG.md
else
    verbose_print "CHANGELOG.md: Not found or empty. Generate it from scratch"
fi

################################################################################
# Add messages from git-log
################################################################################
# Get current branch name to update corresponding section
BRANCH=$(git rev-parse --abbrev-ref HEAD)
RELEASE=$(echo ${BRANCH} | sed -En 's/^release\/(.*)/\1/p')
if [[ "${RELEASE}" == "" ]]; then
    RELEASE="Unreleased"
    verbose_print "$(basename $0): Current branch is development"
else
    verbose_print "$(basename $0): Current branch is release [${RELEASE}]"
fi

# Remove "Unreleased" section and ${RELEASE} section from CHANGELOG.
# If it's release branch all commits go to ${RELEASE} section, i.e. no commits
# under "Unreleased" section. Otherwise $RELEASE="Unreleased", so only
# unreleased section removed.
NEW_SECTIONS_LIST=()
for i in "${CHANGELOG_SECTIONS[@]}"; do
    if [[ "$i" != "Unreleased" ]] && [[ "$i" != "${RELEASE}" ]]; then
        NEW_SECTIONS_LIST+=( $i )
    fi
done
CHANGELOG_SECTIONS=( ${NEW_SECTIONS_LIST[@]} )
unset CHANGELOG["Unreleased"]
unset CHANGELOG["${RELEASE}"]

# Get current release info
RELEASE_DATE="" # Empty for "Unreleased" release
RELEASE_TAG=""  # Empty for "Unreleased" release
if [[ "${RELEASE}" != "Unreleased" ]]; then
    LAST_COMMIT=$(git log --sparse --oneline --decorate=full -1)
    RELEASE_REV=$(echo "$LAST_COMMIT" | cut -d' ' -f1)
    RELEASE_DATE=$(git show -s --format=%as ${RELEASE_REV})
    RELEASE_TAG="$(get_release_tag "$LAST_COMMIT")"
fi

# Current release added, changed, removed features arrays
ADD=()
CHANGE=()
REMOVED=()

# Section number in changelog to insert current release section before
# CHANGELOG_SECTIONS:
#   [0]: HEAD
#        <- New section gonna be inserted here
#   [1]: Some-release-read-from-file
CHANGELOG_INSERT_POS=1

GIT_COMMITS=$(git log --sparse --oneline --decorate=full)
LAST_LINE=$(echo "${GIT_COMMITS}" | tail -n 1)
for line in $GIT_COMMITS; do
    if [[ $line =~ "release" ]]; then
        PREV_RELEASE="$(get_release_name "$line")"

        if [[ "${PREV_RELEASE}" != "" ]]; then
            # Get previous release info to generate links
            PREV_RELEASE_REV=$(echo "$line" | cut -d' ' -f1)
            PREV_RELEASE_DATE=$(git show -s --format=%as ${PREV_RELEASE_REV})
            PREV_RELEASE_TAG="$(get_release_tag "$line")"
            verbose_print "Git-log: Found release [${PREV_RELEASE}]: ${line}"
        fi
    fi

    LINE_OP=$(echo "$line" | sed -En 's/.*(\![\=|\+|\-])(.*)/\1/p')
    case $LINE_OP in
    "!+")
        feature=( "$(cleanupCommit "$line")" )
        if [[ "$feature" != "" ]]; then
            ADD+=( "$(cleanupCommit "$line")" )
        fi
        ;;
    "!=")
        feature=( "$(cleanupCommit "$line")" )
        if [[ "$feature" != "" ]]; then
            CHANGE+=( "$(cleanupCommit "$line")" )
        fi
        ;;
    "!-")
        feature=( "$(cleanupCommit "$line")" )
        if [[ "$feature" != "" ]]; then
            REMOVED+=( "$(cleanupCommit "$line")" )
        fi
        ;;
    esac

    if [[ "${line}" == "${LAST_LINE}" ]] || \
       [[ "${PREV_RELEASE}" != "" && "${PREV_RELEASE}" != "${RELEASE}" ]]
    then
        if [[ ! "${CHANGELOG_SECTIONS[@]}" =~ "${RELEASE}" ]]; then
            verbose_print "$(basename $0): Update section [${RELEASE}]"

            # Generate release description
            CHANGELOG["${RELEASE}"]="$(generate_release_description)"
            CHANGELOG["${RELEASE}"]+=$'\n'

            # Insert current release section
            CHANGELOG_SECTIONS=(${CHANGELOG_SECTIONS[@]:0:${CHANGELOG_INSERT_POS}} ${RELEASE} ${CHANGELOG_SECTIONS[@]:${CHANGELOG_INSERT_POS}})
        else
            verbose_print "$(basename $0): Section [${RELEASE}] present in changelog. Skip it"
        fi

        # Generate link to current release
        if [[ "${REPO_URL}" != "" ]]; then
            if [[ "${RELEASE}" == "Unreleased" ]]; then
                if [[ "${PREV_RELEASE_TAG}" == "" ]]; then
                    # If no previous release found
                    LINKS["${RELEASE}"]="${REPO_URL}"
                else
                    LINKS["${RELEASE}"]="${REPO_URL}/compare/diff?targetBranch=${PREV_RELEASE_TAG}&sourceBranch=${MASTER_BRANCH}"
                fi
            else
                LINKS["${RELEASE}"]="${REPO_URL}/browse?at=${RELEASE_TAG}"
            fi
            verbose_print "$(basename $0): Update link for [${RELEASE}]: ${LINKS["${RELEASE}"]}"
        fi

        # Next releases will be inserted after this one
        let "CHANGELOG_INSERT_POS=CHANGELOG_INSERT_POS+1"

        # Reset lists of features
        ADD=()
        CHANGE=()
        REMOVED=()

        # Set found release as current one
        RELEASE=${PREV_RELEASE}
        RELEASE_DATE=${PREV_RELEASE_DATE}
        RELEASE_TAG=${PREV_RELEASE_TAG}
        PREV_RELEASE=""
        PREV_RELEASE_DATE=""
        PREV_RELEASE_TAG=""
    fi
done

################################################################################
# Print CHANGELOG
################################################################################
if [[ ! -z "${OUTFILE}" ]]; then
    exec >"${OUTFILE}"
fi

for section in "${CHANGELOG_SECTIONS[@]}"; do
    echo "${CHANGELOG["${section}"]}"
done

for section in "${CHANGELOG_SECTIONS[@]}"; do
    if [[ "${LINKS[${section}]}" != "" ]]; then
        echo "[${section}]: ${LINKS[${section}]}"
    fi
done
