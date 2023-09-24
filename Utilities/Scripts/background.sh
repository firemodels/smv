# ---------------------------- usage ----------------------------------

function usage {
  echo "Usage: background.sh [-d dir -e exepath] input"
  echo ""
  echo "runs background prog"
  echo ""
  echo "options:"
  echo " -d dir - specify directory where the case is found [default: .]"
  echo " -e exe - path for executable to run"
  echo " -h     - display this message"
  echo " -s     - sleep until the number of jobs owned by user is less than the number of availab cores"
  echo ""
  exit
}

CURDIR=`pwd`

#*** define repo root

QSMV=$0
QSMV_PATH=$(dirname `which $0`)
cd $QSMV_PATH/../../..
REPOROOT=`pwd`

cd $CURDIR

if [ $# -lt 1 ]; then
  usage
fi

DIR=
EXE=
USE_FULL=
nprocs=1
SLEEP=

#*** read in parameters from command line

while getopts 'Ad:e:hIn:p:q:stv' OPTION
do
case $OPTION  in
  A)
   dummy=
   ;;
  d)
   DIR="$OPTARG"
   ;;
  e)
   EXE="$OPTARG"
   ;;
  h)
   usage
   exit
   ;;
  I)
   USE_FULL=1
   ;;
  n)
   dummy="${OPTARG}"
   ;;
  p)
   nprocs="${OPTARG}"
   ;;
  q)
   dummy="${OPTARG}"
   ;;
  s)
   SLEEP=1
   ;;
  t)
   dummy=
   ;;
  v)
   showinput=1
   v_arg="-v"
   ;;
esac
done
shift $(($OPTIND-1))

if [ "$EXE" == "" ]; then
  echo "***error: -e parameter required"
  exit
fi

#*** define input file

in=$1
infile=${in%.*}
if [ "$USE_FULL" == "1" ]; then
  input=$in
else
  input=$infile
fi

if [ "$DIR" != "" ]; then
  cd $DIR
fi
if [ "$STOPFDSMAXITER" != "" ]; then
  echo $STOPFDSMAXITER > ${infile}.stop
else
  rm -f ${infile}.stop
fi


LOCKBASE=`whoami`_fdslock
LOCKFILE=/tmp/${input}_${LOCKBASE}$$

if [ "$SLEEP" != "" ]; then
  NPROCS=`grep processors /proc/cpuinfo | wc -l`
  NJOBS=`ls -l /tmp/*${LOCKFILES}* | wc -l`

  while [ $NJOBS -GT $NPROCS ]; do
   sleep 10
  done
  sleep 1
fi

touch $LOCKFILE
$EXE $input
rm $LOCKFILE
