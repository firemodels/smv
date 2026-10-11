#!/bin/bash
CUR=`pwd`
allrepos="bot cad cfast cor exp fds fig out radcal smv test_bundles"
updaterepos=$allrepos
single_repo=
otherrepos="webpages wikis"
BRANCH=master
CHECKOUT_MASTER=
branchname=
PAUSE=10

function usage {
echo "Update the repos $allrepos if they exist"
echo ""
echo "Options:"
echo "-b - update only bot repo"
echo "-B b - checkout branch named b"
echo "-h - display this message"
echo "-K xyz - update only repo named xyz"
echo "-m - checkout master branch before updating"
echo "-w - update only webpages and wikis repos"
exit
}

FMROOT=
WEBWIKI_ONLY=
if [ -e ../.gitbot ]; then
   cd ../..
   FMROOT=`pwd`
else
   echo "***error: this script must be run from the bot/Scripts directory"
   exit
fi

while getopts 'bB:hK:mw' OPTION
do
case $OPTION  in
  b)
   updaterepos=bot
   ;;
  B)
   branchname="$OPTARG";
   ;;
  h)
   usage;
   ;;
  K)
   single_repo="$OPTARG";
   ;;
  m)
   CHECKOUT_MASTER=1
   ;;
  w)
   WEBWIKI_ONLY=1
   ;;
esac
done
shift $(($OPTIND-1))

if [ "$single_repo" != "" ]; then
  updaterepos="$single_repo"
  otherrepos="$single_repo"
fi

UPDATE_REPO ()
{
  local repo=$1
  repodir=$FMROOT/$repo
  echo pausing $PAUSE s
  sleep $PAUSE

  echo "------------- $repo -------------------------------------------"
  if [ ! -e $repodir ]; then
     echo "Skipping, $repo does not exist"
     return
  fi
  cd $repodir
  if [ "$CHECKOUT_MASTER" != "" ]; then
    git checkout master
  fi 
  CURRENT_BRANCH=`git rev-parse --abbrev-ref HEAD`
  if [ "$BRANCH" != "$CURRENT_BRANCH" ]; then
    echo "Skipping, found branch $CURRENT_BRANCH, expecting branch $BRANCH"
    return
  fi
  echo ""
  echo "***  updating from origin"
  echo "     branch: $BRANCH"
  echo "     dir: $repodir"

  git remote update
  git merge origin/$BRANCH
  have_central=`git remote -v | awk '{print $1}' | grep firemodels | wc  -l`
  if [ "$have_central" -gt "0" ]; then
     echo ""
     echo "*** updating from firemodels"
     echo "    branch: $BRANCH"
     echo "    dir: $repodir"
     git merge firemodels/$BRANCH
     ahead=`git status -uno | grep ahead | wc -l`
     if [ "$ahead" -gt "0" ]; then
        git push origin $BRANCH
     fi
  fi
  if [[ "$repo" == "exp" ]]; then
     git submodule foreach git remote update
     git submodule foreach git merge origin/master
  fi
  if [ "$branchname" != "" ]; then
    git checkout -f -B "$branchname"
  fi
}

UPDATE_REPO2 ()
{
  local repo=$1
  repodir=$FMROOT/$repo
  echo pausing $PAUSE s
  sleep $PAUSE

  if [ ! -e $repodir ]; then
     return
  fi
  echo "------------- $repo -------------------------------------------"
  cd $repodir
  if [ "$CHECKOUT_MASTER" != "" ]; then
    git checkout master
  fi 
  BRANCH=`git rev-parse --abbrev-ref HEAD`
  echo ""
  echo "***  updating from firemodels"
  echo "     branch: $BRANCH"
  echo "     dir: $repodir"
  echo ""
  git fetch origin
  git merge origin/$BRANCH
  git status -uno
  if [ "$branchname" != "" ]; then
    git checkout -f -B "$branchname"
  fi
}

if [ "$WEBWIKI_ONLY" == "" ]; then
  for repo in $updaterepos
  do 
    echo
    UPDATE_REPO $repo
  done
else
  for repo in $otherrepos
  do 
    echo
    UPDATE_REPO2 $repo
  done
fi
echo update complete

cd $CURDIR
