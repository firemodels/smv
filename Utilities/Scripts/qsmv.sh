#!/bin/bash

# ---------------------------- usage ----------------------------------

function usage {
  echo "Usage: qsmv.sh [-e smv_command] [-q queue] casename"
  echo ""
  echo "runs smokeview on the case casename.smv using the script casename.ssf"
  echo ""
  echo "options:"
  echo " -e exe - full path of smokeview used to run case "
  echo "    [default: $REPOROOT/smv/Build/smokeview/intel_linux/smokeview_intel_linux]"
  echo " -h   - show commonly used options"
  echo " -H   - show all options"
  echo " -P n - run n instances of smokeview each instance rendering 1/n'th of the total images"
  echo "        only use this option if you have a RENDERALL keyword in your .ssf smokeview script"
  echo " -q q - name of queue. [default: batch]"
  echo " -v   - output generated script (do not run)"
  if [ "$HELP" == "" ]; then
    exit
  fi
  echo "Other options:"
  echo " -b     - bin directory"
  echo " -c     - smokeview script file [default: casename.ssf]"
  echo " -C com - execute the command com"
  echo " -d dir - specify directory where the case is found [default: .]"
  echo " -e exe - execute the program exe"
  echo " -i     - use installed smokeview"
  echo " -j p   - job prefix"
  echo " -N n   - reserve n cores [default: $ncores]"
  echo " -r     - redirect output"
  echo " -s     - first frame rendered [default: 1]"
  echo " -S     - interval between frames [default: 1]"
  echo " -T     - share nodes"
  echo ""
  exit
}

CURDIR=`pwd`

#*** define repo root

QSMV=$0
QSMV_PATH=$(dirname `which $0`)
cd $QSMV_PATH/../../..
REPOROOT=`pwd`

if [ ! -e $HOME/.smokebot ]; then
  mkdir $HOME/.smokebot
fi

cd $CURDIR

missing_slurm=`srun -V |& tail -1 | grep "not found" | wc -l`
RESOURCE_MANAGER="NONE"
if [ $missing_slurm -eq 0 ]; then
  RESOURCE_MANAGER="SLURM"
else
  missing_torque=`echo | qmgr -n |& tail -1 | grep "not found" | wc -l`
  if [ $missing_torque -eq 0 ]; then
    RESOURCE_MANAGER="TORQUE"
  fi
fi

if [ "$RESOURCE_MANAGER" == "SLURM" ]; then
  if [ "$SLURM_MEM" != "" ]; then
    SLURM_MEM="#SBATCH --mem=$SLURM_MEM"
  fi
  if [ "$SLURM_MEMPERCPU" != "" ]; then
    SLURM_MEM="#SBATCH --mem-per-cpu=$SLURM_MEMPERCPU"
  fi
else
  RESOURCE_MANAGER="TORQUE"
fi

#*** determine number of cores

ncores=`grep processor /proc/cpuinfo | wc -l`
NRESERVE=$ncores

#*** determine default queue

queue=batch

#*** set default parameter values

HELP=
use_installed=
render_opts=
dir=.
showinput=0
exe=
smv_script=
nprocs=1
redirect=
FED=
dummy=
COMMAND=
SMVBINDIR=
SMVJOBPREFIX=
b_arg=
B_arg=
c_arg=
d_arg=
e_arg=
f_arg=
i_arg=
j_arg=
N_ARG=
q_arg=
r_arg=
T_arg=
v_arg=
one_frame=
NOBOUNDS=

if [ $# -lt 1 ]; then
  usage
fi

commandline=`echo $* | sed 's/-V//' | sed 's/-v//'`

#*** read in parameters from command line

while getopts 'Ab:Bc:C:d:D:e:fFhHij:n:N:Op:P:q:rs:S:tv' OPTION
do
case $OPTION  in
  A)
   dummy=1
   ;;
  b)
   SMVBINDIR="-bindir $OPTARG"
   b_arg="-b $OPTARG"
   ;;
  B)
   NOBOUNDS=-nobounds
   B_ARG="-B"
   ;;
  c)
   smv_script="$OPTARG"
   c_arg="-c $smv_script"
   ;;
  C)
   COMMAND="$OPTARG"
   ;;
  d)
   dir="$OPTARG"
   d_arg="-d $dir"
   ;;
  e)
   exe="$OPTARG"
   e_arg="-e $exe"
   ;;
  f)
   f_arg="-f"
   FED="-fed"
   ;;
  h)
   usage
   exit
   ;;
  H)
   HELP=ALL
   usage
   exit
   ;;
  i)
   use_installed=1
   i_arg="-i"
   ;;
  j)
   SMVJOBPREFIX="${OPTARG}"
   j_arg="-j ${OPTARG}"
   ;;
  n)
   dummy="${OPTARG}"
   ;;
  N)
   NRESERVE="${OPTARG}"
   ;;
  O)
   one_frame=1
   ;;
  p)
   dummy="${OPTARG}"
   ;;
  P)
   nprocs="$OPTARG"
   ;;
  q)
   queue="$OPTARG"
   q_arg="-q $queue"
   ;;
  r)
   redirect="-redirect"
   r_arg="-r"
   ;;
  s)
   first="$OPTARG"
   if [ "$FRAME_OFFSET" != "" ]; then
     first=$(expr $first + $FRAME_OFFSET)
   fi
   render_opts=1
   ;;
  S)
   skip="$OPTARG"
   render_opts=1
   ;;
  t)
   dummy=1
   ;;
  v)
   showinput=1
   v_arg="-v"
   ;;
esac
done
shift $(($OPTIND-1))

#*** define input file

in=$1
infile=${in%.*}

re='^[0-9]+$'
if ! [[ $nprocs =~ $re ]] ; then
   nprocs=1;
fi

if ! [[ $NRESERVE =~ $re ]] ; then
   NRESERVE=$ncores;
fi
N_ARG="-N $NRESERVE"

if [ $nprocs != 1 ]; then
  if [ "$one_frame" == "" ]; then
    for i in $(seq 1 $nprocs); do
      $QSMV $b_arg $B_ARG $c_arg $d_arg $e_arg $f_arg $i_arg $j_arg $N_ARG $q_arg $r_arg $v_arg $T_arg -s $i -S $nprocs $in
    done
  else
    $QSMV $b_arg $B_ARG $c_arg $d_arg $e_arg $f_arg $i_arg $j_arg $N_ARG $q_arg $r_arg $v_arg $T_arg -s $nprocs -S $nprocs $in
  fi
  exit
fi

if [ "$SMVJOBPREFIX" == "" ]; then
  SMVJOBPREFIX=SMV_
fi

# determine frame start and frame skip

if [ "$first" == "" ]; then
  first=1
fi
if [ "$skip" == "" ]; then
  skip=1
fi
if [ "$render_opts" != "" ]; then
  render_opts="-startframe $first -skipframe $skip"
fi

# determine smokeview script file parameters

if [ "$smv_script" != "" ]; then
  smokeview_script_file=$smv_script
  smv_script="-script $smv_script"
else
  smokeview_script_file=${infile}.ssf
  smv_script=-runscript
fi
if [ "$FED" != "" ]; then
  smv_script=
  script_file=
fi

#*** parse walltime parameter

if [ "$walltime" == "" ]; then
    if [ "$RESOURCE_MANAGER" == "SLURM" ]; then
	walltime=99-99:99:99
    else
	walltime=999:0:0
    fi
fi

#*** define executable

if [ "$use_installed" == "1" ]; then
  notfound=`echo | smokeview 2>&1 >/dev/null | tail -1 | grep "not found" | wc -l`
  if [ $notfound -eq 1 ]; then
    echo "smokeview is not installed."
    ABORTRUN=y
    exe=
  else
    smvpath=`which smokeview`
    smvdir=$(dirname "${smvpath}")
    curdir=`pwd`
    if [ "$SMVBINDIR" == "" ]; then
      SMVBINDIR="-bindir $smvdir"
    fi
    cd $smvdir
    exe=`pwd`/smokeview
    cd $curdir
  fi
else
  if [ "$exe" == "" ]; then
    exe=$REPOROOT/smv/Build/smokeview/intel_linux/smokeview_linux
    smvdir=$(dirname "${smvpath}")
    if [ "$SMVBINDIR" == "" ]; then
      SMVBINDIR="-bindir $REPOROOT/smv/Build/for_bundle"
    fi
  fi
fi

let ppn=$NRESERVE
let nodes=1

TITLE="$infile"

cd $dir
fulldir=`pwd`

touch test.$$ >& /dev/null
if [ -e test.$$ ]; then
  rm test.$$
  logdir=`pwd`
else
  logdir=$HOME/.smokeview
  if [ ! -e $fulldir ]; then
    mkdir $fulldir
  fi
fi

#*** define files

basefile=${infile}_f${first}_s${skip}
outerr=$logdir/$basefile.err
outlog=$logdir/$basefile.log
scriptlog=$logdir/$basefile.slog
in_full_file=$fulldir/$infile
smvfile=${infile}.smv
in_full_smvfile=$fulldir/$smvfile

#*** make sure files needed by qsmv.sh exist

if [[ "$showinput" == "0" ]] && [[ "$COMMAND" == "" ]] ; then
  if ! [ -e $exe ]; then
    echo "The smokeview executable, $exe, does not exist."
    ABORTRUN=y
  fi

  if ! [ -e $smvfile ]; then
    echo "The smokeview file, $smvfile, does not exist."
    ABORTRUN=y
  fi

  if [ "$FED" == "" ]; then
    if ! [ -e $smokeview_script_file ]; then
      echo "The smokeview script file, $smokeview_script_file, does not exist."
      ABORTRUN=y
    fi
  fi

  if [ "$ABORTRUN" == "y" ]; then
    echo "Run aborted."
    exit
  fi
fi

QSUB="qsub -q $queue"

#*** setup for SLURM (alternative to torque)

if [ "$RESOURCE_MANAGER" == "SLURM" ]; then
#  QSUB="sbatch --cpus-per-task=16 -p $queue --ignore-pbs "
  QSUB="sbatch -p $queue --ignore-pbs "
fi

if [ "$queue" == "terminal" ]; then
  QSUB=bash
fi

#*** Set walltime parameter only if walltime is specified as input argument

walltimestring_pbs=
walltimestring_slurm=
if [ "$walltime" != "" ]; then
  walltimestring_pbs="-l walltime=$walltime"
  walltimestring_slurm="-t $walltime"
fi

#*** create a random script file for submitting jobs

scriptfile=`mktemp /tmp/script.$$.XXXXXX`

cat << EOF > $scriptfile
#!/bin/bash
# $QSUB  $scriptfile
# $0 $commandline
EOF

if [ "$queue" != "none" ]; then
  if [ "$RESOURCE_MANAGER" == "SLURM" ]; then
    cat << EOF >> $scriptfile
#SBATCH -J ${SMVJOBPREFIX}$infile
#SBATCH -e $outerr
#SBATCH -o $outlog
#SBATCH -p $queue
#SBATCH --nodes=1
#SBATCH --exclusive


$SLURM_MEM
EOF
    if [ "$walltimestring_slurm" != "" ]; then
      cat << EOF >> $scriptfile
#SBATCH $walltimestring_slurm
EOF
    fi

  else
    cat << EOF >> $scriptfile
#PBS -N ${SMVJOBPREFIX}${TITLE}/f${first}s$skip
#PBS -W umask=0022
#PBS -e $outerr
#PBS -o $outlog
#PBS -l nodes=$nodes:ppn=$ppn
EOF
    if [ "$walltimestring_pbs" != "" ]; then
      cat << EOF >> $scriptfile
#PBS $walltimestring_pbs
EOF
    fi
  fi
fi

if [ "$COMMAND" == "" ]; then
cat << EOF >> $scriptfile
cd $fulldir
echo
echo \`date\`
echo "       Executable:$exe"
echo "        Directory: \`pwd\`"
echo "   smokeview file: $smvfile"
echo " smokeview script: $smokeview_script_file"
echo "      start frame: $first"
echo "       frame skip: $skip"
echo "         nobounds: $NOBOUNDS"
echo "             Host: \`hostname\`"
echo "      Run command: $exe $script_file $smv_script $NOBOUNDS $FED $redirect $render_opts $SMVBINDIR $infile"
echo "            Queue: $queue"
echo ""

$QSMV_PATH/XVFB-RUN.sh $exe $script_file $smv_script $NOBOUNDS $FED $redirect $render_opts $SMVBINDIR $infile
EOF
else
cat << EOF >> $scriptfile
cd $fulldir
echo
echo \`date\`
echo "       command:$COMMAND"
echo "     directory: \`pwd\`"
echo "          Host: \`hostname\`"
echo "         Queue: $queue"
echo ""

$COMMAND

EOF
fi

#*** output script file to screen if -v option was selected

if [ "$showinput" == "1" ]; then
  cat $scriptfile
  rm $scriptfile
  echo
  exit
fi

#*** output info to screen

if [ "$COMMAND" == "" ]; then
  echo "     smokeview file: $smvfile"
  echo "      smokeview exe: $exe"
  echo "             script: $smokeview_script_file"
  echo "        start frame: $first"
  echo "         frame skip: $skip"
  echo "              Queue: $queue"
  echo ""
else
  echo "     command: $COMMAND"
  echo "       Queue: $queue"
  echo ""
fi

#*** run script

chmod +x $scriptfile
if [ "$SCRIPTFILES" != "" ]; then
  echo $(basename "$scriptfile") >> $SCRIPTFILES
fi
$QSUB $scriptfile
if [ "$queue" != "none" ]; then
  cat $scriptfile > $scriptlog
  echo "#$QSUB $scriptfile" >> $scriptlog
  rm $scriptfile
fi
