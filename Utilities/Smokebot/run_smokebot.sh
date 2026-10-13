#!/bin/bash
EMAIL_LIST="$HOME/.smokebot/smokebot_email_list.sh"

# The Smokebot script is part of an automated continuous integration system.
# Consult the FDS Config Management Plan for more information.

#---------------------------------------------
#                   usage
#---------------------------------------------

function usage {
echo "Verification testing script for smokeview"
echo ""
echo "Options:"
echo ""
echo "-f - force smokebot to run"
echo "-F build - use fds apps from directory build"
echo "-G rootdir - specify github root directory"
echo "-h - display this output"
echo "-k - kill smokebot if it is running"
if [ "$EMAIL" != "" ]; then
  echo "-m email_address - email results to email_address [default: $EMAIL]"
else
  echo "-m email_address - email results to email_address"
fi
echo "-M - make movies"
echo "-o - specify GH_OWNER when uploading manuals. [default: $GH_OWNER]"
echo "-q queue [default: $QUEUE]"
echo "-r - specify GH_REPO when uploading manuals. [default: $GH_REPO]"
echo "-R release_type (master, release or test) - clone fds, exp, fig, out"
echo "   and smv repos.  fds and smv repos will be checked out with"
echo "   a branch named master, release or test [default: master]"
echo "-U - upload guides"
echo "-w directory - web directory containing summary pages"
exit
}

#---------------------------------------------
#                   LIST_DESCENDANTS
#---------------------------------------------

LIST_DESCENDANTS ()
{
  if [ "$1" != "" ]; then
    local children=$(pgrep -P $1)

    for pid in $children
    do
      LIST_DESCENDANTS $pid
    done
    echo "$children"
  fi
}

#VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
#                             beginning of run_smokebot.sh
#^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#*** location of smokebot processor id

if [ ! -d ~/.fdssmvgit ] ; then
  mkdir ~/.fdssmvgit
fi
smokebot_pid=~/.fdssmvgit/firesmokebot_pid

#*** make sure smokebot is started in the right location

CURDIR=`pwd`
if [ -x run_smokebot.sh ]; then
  SMOKEBOTDIR=`pwd`
  cd $CURDIR
else
  echo "***error: smokebot not running in the directory containing smokebot scripts"
  exit
fi

#*** define initial values

SIZE=
KILL_SMOKEBOT=
MOVIE=
UPLOAD=
FORCE=
FORCECLONE=
CLONE_REPOS=
WEB_DIR=
USE_BOT_QFDS=
FOPT=
CPUS_PER_TASK=

REPOROOT=
if [[ -d ../../smv ]]; then
  cd ../..
  REPOROOT=`pwd`
fi
if [[ "$REPOROOT" == "" ]] && [[ -d ../../../smv ]]; then
  cd ../../..
  REPOROOT=`pwd`
fi

WEB_ROOT=/opt/www/html
if [ ! -d $WEB_ROOT ]; then
  WEB_ROOT=/var/www/html
fi
if [ ! -d $WEB_ROOT ]; then
  WEB_ROOT=
fi

#*** check to see if a queing system is available

QUEUE=firebot
notfound=`squeue -a 2>&1 | tail -1 | grep "not found" | wc -l`
if [ $notfound -eq 1 ] ; then
  echo ***error: squeue command not found.
  exit
fi

#*** parse command line options

while getopts 'fF:G:hJkm:Mo:q:r:R:s:T:Uw:W:' OPTION
do
case $OPTION  in
  f)
   FORCE=1
   ;;
  F)
   FOPT="-F $OPTARG"
   ;;
  G)
   REPOROOT="$OPTARG"
   ;;
  h)
   usage
   ;;
  J)
   DUMMY=
   ;;
  k)
   KILL_SMOKEBOT=1
   ;;
  m)
   EMAIL="$OPTARG"
   ;;
  M)
   MOVIE="-M"
   ;;
  o)
   export GH_OWNER="$OPTARG"
   ;;
  q)
   QUEUE="$OPTARG"
   ;;
  r)
   export GH_REPO="$OPTARG"
   ;;
  R)
   CLONE_REPOS="$OPTARG"
   ;;
  T)
   CPUS_PER_TASK="-T $OPTARG"
   ;;
  U)
   UPLOAD="-U"
   ;;
  w)
   WEB_DIR="$OPTARG"
   ;;
  W)
   WEB_ROOT="$OPTARG"
   ;;
  \?)
  echo "***error: unknown option entered. aborting smokebot"
  exit 1
esac
done
shift $(($OPTIND-1))

if [ "$REPOROOT" == "" ]; then
  echo "***error: root directory not defined"
  echo "          use -G reporoot_path to specify directory containing repos"
  exit
else
  if [ ! -d $REPOROOT ]; then
    echo "***error repo root directory $REPOROOT does not exist"
    exit
  fi
fi
REPOROOTOPT="-G $REPOROOT"

if [ "$WEB_DIR" == "" ]; then
  WEB_DIR=`basename $REPOROOT`
  EXT="${WEB_DIR##*_}"
  if [ "$EXT" != "" ]; then
    WEB_DIR=$EXT
  fi
  WEB_DIR=`whoami`/$WEB_DIR
fi

if [ "$WEB_ROOT" == "" ]; then
  WEB_DIR=
fi


# sync fds and smv repos with the the repos used in the last successful firebot run

# warn user (if not the smokebot user) if using the clone option

if [ `whoami` != smokebot ]; then
  if [ "$CLONE_REPOS" != "" ]; then
    echo "You are about to erase and clone the "
    echo "fds, exp, fig, out and smv repos."
    echo "Press any key to continue or <CTRL> c to abort."
    echo "Type $0 -h for other options"
#    read val
  fi
fi

if [ "$CLONE_REPOS" != "" ]; then
  if [ "$CLONE_REPOS" != "release" ]; then
    if [ "$CLONE_REPOS" != "test" ]; then
      CLONE_REPO="master"
    fi
  fi
  CLONE_REPOS="-R $CLONE_REPOS"
fi

if [ "$WEB_DIR" != "" ]; then
  WEB_DIR="-w $WEB_DIR"
fi
if [ "$WEB_ROOT" != "" ]; then
  WEB_ROOT="-W $WEB_ROOT"
fi

#*** kill smokebot

if [ "$KILL_SMOKEBOT" == "1" ]; then
  if [ -e $smokebot_pid ]; then
    PID=`head -1 $smokebot_pid`

    echo killing smokebot processes descended from: $PID
    JOBS=$(LIST_DESCENDANTS $PID)
    if [ "$JOBS" != "" ]; then
      echo killing processes invoked by smokebot: $JOBS
      kill -9 $JOBS
    fi

    JOBIDS=`squeue -o "%.18j %.8u %.2t" | grep SB_ | awk -v user="$USER" '{if($2==user){print $1}}' | awk -F'.' '{print $1}'`
    if [ "$JOBIDS" != "" ]; then
      echo killing smokebot jobs with Id: $JOBIDS
      qdel $JOBIDS
    fi
    
    echo "killing smokebot (PID=$PID)"
    kill -9 $PID

    echo smokebot process $PID killed
    rm -f $smokebot_pid
  else
    echo smokebot not running
  fi
  exit
fi

#*** make sure smokebot is not already running

if [ "$FORCE" == "" ]; then
  if [ -e $smokebot_pid ] ; then
    echo Smokebot or firebot are running.
    echo "If this is not the case, -f option."
    if [ -e $EMAIL_LIST ]; then
      source $EMAIL_LIST
      echo "Smokebot was unable to start.  Another instance was already running or it did not complete successfully"  | mail -s "error: smokebot failed to start" $mailToSMV > /dev/null
    fi
    exit 1
  fi
fi

QUEUE="-q $QUEUE"

if [ "$EMAIL" != "" ]; then
  EMAIL="-m $EMAIL"
fi

#*** run smokebot

touch $smokebot_pid
./smokebot.sh $SIZE $CPuS_PER_TASK $CLONE_REPOS $REPOROOTOPT $FOPT $FORCECLONE $WEB_DIR $WEB_ROOT $QUEUE $UPLOAD $EMAIL $MOVIE "$@"
if [ -e $smokebot_pid ]; then
  rm $smokebot_pid
fi

 
 
