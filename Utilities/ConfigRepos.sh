#!/bin/bash
# Check out and tag the revisions defined for the release bundle.
set -e

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
BOTREPO="$( cd "$SCRIPTDIR/.." && pwd )"
FMROOT="$( cd "$BOTREPO/.." && pwd )"
MODE=${1:-}
if [[ "$MODE" != reset ]]; then
  source "$BOTREPO/Bundlebot/release/config.sh"
fi

STATUS=0
repos="cad cfast exp fds fig out smv"
for repo in $repos
do
  if [[ "$MODE" == reset ]]; then
    if (
      cd "$FMROOT/$repo" || exit 1
      echo "----------------------------------------------"
      echo "repo: $repo"
      echo "git checkout master"
      if ! git checkout master; then
        echo "***Error: checkout of master failed for $repo" >&2
        exit 1
      fi
      if git show-ref --verify --quiet refs/heads/release; then
        echo "git branch -D release"
        if ! git branch -D release; then
          echo "***Error: removal of release branch failed for $repo" >&2
          exit 1
        fi
      else
        result=$?
        if [[ "$result" != 1 ]]; then
          echo "***Error: cannot check release branch for $repo" >&2
          exit 1
        fi
      fi
    ); then
      :
    else
      STATUS=1
    fi
    continue
  fi
  TAG=
  HASH=
  case "$repo" in
    cad) TAG=$BUNDLE_CAD_TAG; HASH=$BUNDLE_CAD_HASH ;;
    cfast) TAG=$BUNDLE_CFAST_TAG; HASH=$BUNDLE_CFAST_HASH ;;
    exp) TAG=$BUNDLE_EXP_TAG; HASH=$BUNDLE_EXP_HASH ;;
    fds) TAG=$BUNDLE_FDS_TAG; HASH=$BUNDLE_FDS_HASH ;;
    fig) TAG=$BUNDLE_FIG_TAG; HASH=$BUNDLE_FIG_HASH ;;
    out) TAG=$BUNDLE_OUT_TAG; HASH=$BUNDLE_OUT_HASH ;;
    smv) TAG=$BUNDLE_SMV_TAG; HASH=$BUNDLE_SMV_HASH ;;
  esac
  repo_dir="$FMROOT/$repo"
  ERROR=
  if [[ "$TAG" == "" ]] || [[ "$HASH" == "" ]]; then
    echo "***Error: missing release tag or revision for $repo" >&2
    ERROR=1
  fi
  if [[ ! -d "$repo_dir" ]]; then
    echo "***Error: repository directory $repo_dir does not exist" >&2
    ERROR=1
  fi
  if [ "$ERROR" != "" ]; then
    STATUS=1
    continue
  fi
  if (
    cd "$repo_dir" || exit 1
    echo "----------------------------------------------"
    echo "repo: $repo"
    # Check every configured remote, including origin and firemodels.
    # Stop before changing the branch or tag if publication cannot be checked.
    remotes=$(git remote) || exit 1
    if [[ "$remotes" == "" ]]; then
      echo "***Error: no remotes configured for $repo; cannot check tag $TAG" >&2
      exit 1
    fi
    for remote in $remotes
    do
      if ! remote_tag=$(git ls-remote --tags "$remote" "refs/tags/$TAG"); then
        echo "***Error: cannot check tag $TAG on remote $remote for $repo" >&2
        exit 1
      fi
      if [[ "$remote_tag" != "" ]]; then
        echo "***Error: tag $TAG is already published on remote $remote for $repo; refusing to replace it" >&2
        exit 1
      fi
    done
    echo "git checkout -B release $HASH"
    if ! git checkout -B release "$HASH"; then
      echo "***Error: checkout failed for $repo at $HASH" >&2
      exit 1
    fi
    echo "git tag -f -a $TAG -m \"tag for $TAG\""
    if ! git tag -f -a "$TAG" -m "tag for $TAG"; then
      echo "***Error: creation of tag $TAG failed for $repo" >&2
      exit 1
    fi
  ); then
    :
  else
    STATUS=1
  fi
done
exit "$STATUS"
