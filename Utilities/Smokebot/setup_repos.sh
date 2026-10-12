#!/bin/bash
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
CURDIR=`pwd`
cd $SCRIPTDIR/../../../bot
BOTREPO=`pwd`
cd $CURDIR

function usage {
echo "Create repos used by cfast, fds and/or smokview"
echo ""
echo "Options:"
echo "-3 - clone 3rd party repos: $thirdpartyrepos"
echo "-a - clone all repos: $allrepos"
echo "-b - clone repos in bundle's config.sh script"
echo "-B b - checkout branch named b"
echo "-c - clone repos used by cfastbot: $cfastbotrepos"
echo "-C - force clone"
echo "-D - enable access to firemodels (ie allow git push)"
echo "-e - erase repo before cloning"
echo "-f - clone repos used by firebot: $fdsrepos"
echo "-h - display this message"
echo "-K repo - clone repo named repo, erase first"
echo "-s - clone repos used by smokebot: $smvrepos"
echo "-w - clone wiki and webpage repos"
exit
}

SETUP_REMOTE ()
{
  local repo_dir=$1

  basedir=`basename $repo_dir`
  cd $repo_dir
  CENTRAL=firemodels
  if [ "$repo" == "hypre" ]; then
    CENTRAL=hypre-space
  fi
  if [ "$repo" == "sundials" ]; then
    CENTRAL=LLNL
  fi
  if [ "$repo" == "freeglut" ]; then
    CENTRAL=freeglut
  fi
  if [ "$repo" == "ompi" ]; then
    CENTRAL=open-mpi
  fi
  if [ "$GITUSER" == "firemodels" ]; then
     if [ "$DISABLEPUSH" != "" ]; then
       ndisable=`git remote -v | grep DISABLE | wc -l`
       if [ $ndisable -eq 0 ]; then
          echo disabling push access to $CENTRAL
          git remote set-url --push origin DISABLE
       fi
     fi
  else
     have_central=`git remote -v | awk '{print $1}' | grep $CENTRAL | wc -l`
     if [ $have_central -eq 0 ]; then
        echo setting up remote tracking with $CENTRAL
        git remote add $CENTRAL ${GITHEADER}$CENTRAL/$repo.git
        git remote update
     fi
     if [ "$DISABLEPUSH" != "" ]; then
       ndisable=`git remote -v | grep DISABLE | wc -l`
       if [ $ndisable -eq 0 ]; then
         echo "   disabling push access to $CENTRAL"
         git remote set-url --push $CENTRAL DISABLE
       else
         echo "   push access to $CENTRAL already disabled"
       fi
     fi
  fi
}

#------------------- start of script ---------------------------

CURDIR=`pwd`

allrepos="cad cfast cor exp fds fig out radcal smv test_bundles"
cfastbotrepos="cfast exp fig smv test_bundles"
cfastrepos="cfast exp fds smv"
configrepos="cad cfast exp fds fig out smv"
fdsrepos="cad exp fds fig out smv test_bundles"
firebotrepos="cad exp fds fig fds-smv out smv test_bundles"
smvrepos="cfast fds fig smv test_bundles"
thirdpartyrepos="hypre sundials freeglut"
wikiwebrepos="fds.wiki fds-smv"

repos=
eraserepos=
FORCECLONE=
DISABLEPUSH=1
FMROOT=
WIKIWEB=
branchname=
if [ -e ../.gitbot ]; then
   cd ../..
   FMROOT=`pwd`
else
   echo "***Error: this script must be run from the bot/Scripts directory"
   exit
fi

while getopts '3abB:cCDefhK:sw' OPTION
do
case $OPTION  in
  3)
   repos=$thirdpartyrepos;
   ;;
  a)
   repos=$allrepos;
   ;;
  b)
   repos=$configrepos
   eraserepos=1
   CONFIG_REPOS=1
   ;;
  B)
   branchname="$OPTARG";
   ;;
  c)
   repos=$cfastbotrepos;
   ;;
  C)
   FORCECLONE=1;
   ;;
  D)
   DISABLEPUSH=
   ;;
  e)
   eraserepos=1
   ;;
  f)
   repos=$fdsrepos;
   ;;
  h)
   usage;
   ;;
  K)
   repos="$OPTARG";
   eraserepos=1;
   ;;
  s)
   repos=$smvrepos;
   ;;
  w)
   repos=$wikiwebrepos;
   ;;
esac
done
shift $(($OPTIND-1))

if [ "$CONFIG_REPOS" != "" ]; then
  source $BOTREPO/Bundlebot/release/config.sh
fi

cd $FMROOT/bot
GITHEADER=`git remote -v | grep origin | head -1 | awk  '{print $2}' | awk -F ':' '{print $1}'`
if [ "$GITHEADER" == "git@github.com" ]; then
   GITHEADER="git@github.com:" 
   GITUSER=`git remote -v | grep origin | head -1 | awk -F ':' '{print $2}' | awk -F\/ '{print $1}'`
else
   GITHEADER="https://github.com/"
   GITUSER=`git remote -v | grep origin | head -1 | awk -F '.' '{print $2}' | awk -F\/ '{print $2}'`
fi

if [ "$eraserepos" == "" ]; then
  if [ "$FORCECLONE" == "" ]; then
    echo "You are about to clone the repos: $repos"
  else
    echo "You are cloning the repos: $repos"
  fi
  if [ "$WIKIWEB" == "1" ]; then
     echo "from git@github.com:firemodels into the directory: $FMROOT"
  else
     echo "from $GITHEADER$GITUSER into the directory: $FMROOT"
  fi
  if [ "$FORCECLONE" == "" ]; then
    echo ""
    echo "Press any key to continue or <CTRL> c to abort."
    echo "Type $0 -h for other options"
    read val
  fi
fi

for repo in $repos
do 
  echo
  repo_out=$repo
  WIKIWEB=

  cd $FMROOT

  echo "----------------------------------------------"
  if [ "$repo" == "fds.wiki" ]; then
     repo_out=wikis
     WIKIWEB=1
  fi
  if [ "$repo" == "fds-smv" ]; then
     repo_out=webpages
     WIKIWEB=1
  fi
  repo_dir=$FMROOT/$repo_out
  if [ "$eraserepos" == "" ]; then
    if [ -e $repo_dir ]; then
       echo "   For repo $repo, the directory $repo_dir already exists"
       continue;
    fi
  fi

  echo repo: $repo_out
  if [ "$eraserepos" == "1" ]; then
    if [ -e $repo_out ]; then
      echo removing $repo_out
      rm -rf $repo_out
      if [ -e $repo_out ]; then
         echo "***error: the directory $repo_out failed to be removed"
      fi
    fi
  fi
  if [ "$WIKIWEB" == "1" ]; then
     cd $FMROOT
     git clone ${GITHEADER}firemodels/$repo.git $repo_out
     if [ ! -d $repo_out ]; then
        echo "***error: clone of $repo.git to $repo_out failed"
     fi
     continue
  fi

  GITOWNER=$GITUSER
  if [ "$GITUSER" == "firemodels" ]; then
    if [ "$repo" == "hypre" ]; then
      GITOWNER=hypre-space
    fi
    if [ "$repo" == "sundials" ]; then
      GITOWNER=LLNL
    fi
    if [ "$repo" == "ompi" ]; then
      GITOWNER=open-mpi
    fi
    if [ "$repo" == "freeglut" ]; then
      GITOWNER=freeglut
    fi
  fi

  AT_GITHUB=`git ls-remote $GITHEADER$GITOWNER/$repo.git 2>&1 > /dev/null | grep ERROR | wc -l`
  if [ $AT_GITHUB -gt 0 ]; then
     echo "***Error: The repo $GITHEADER$GITOWNER/$repo.git was not found."
     continue;
  fi 
  
  RECURSIVE=
  if [ "$repo" == "exp" ]; then
     RECURSIVE=--recursive
  fi
  if [ "$repo" != "bot" ]; then
    git clone $RECURSIVE $GITHEADER$GITOWNER/$repo.git $repo_out
    if [ ! -d $repo_out ]; then
      echo "***error: clone of $repo.git to $repo_out failed"
    fi
  fi
  if [ "$CONFIG_REPOS" != "" ]; then
    TAG=
    HASH=
    if [ "$repo" == "cad" ]; then
      TAG=$BUNDLE_CAD_TAG
      HASH=$BUNDLE_CAD_HASH
    fi
    if [ "$repo" == "exp" ]; then
      TAG=$BUNDLE_EXP_TAG
      HASH=$BUNDLE_EXP_HASH
    fi
    if [ "$repo" == "fds" ]; then
      TAG=$BUNDLE_FDS_TAG
      HASH=$BUNDLE_FDS_HASH
    fi
    if [ "$repo" == "fig" ]; then
      TAG=$BUNDLE_FIG_TAG
      HASH=$BUNDLE_FIG_HASH
    fi
    if [ "$repo" == "out" ]; then
      TAG=$BUNDLE_OUT_TAG
      HASH=$BUNDLE_OUT_HASH
    fi
    if [ "$repo" == "smv" ]; then
      TAG=$BUNDLE_SMV_TAG
      HASH=$BUNDLE_SMV_HASH
    fi
#    REPO="${repo^^}"
    if [[ "$TAG" != "" ]] && [[ "$HASH" != "" ]] && [[ -d $repo_out ]]; then
      cd $repo_out
      echo git checkout -b release $HASH
      git checkout -b release $HASH
      echo git tag -a $TAG -m "tag for $TAG"
      git tag -a $TAG -m "tag for $TAG"
    fi
  fi
  if [[ "$branchname" != "" ]] && [[ "$CONFIG_REPOS" == "" ]]; then
    if [ -d $repo ]; then
      cd $repo
      git checkout -f -B $branchname
    else
      echo "***error: $repo not cloned"
    fi
  fi
  SETUP_REMOTE $repo_dir
done
cd $CURDIR
