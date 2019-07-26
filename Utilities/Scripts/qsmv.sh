#!/bin/bash
QSMV=$0

# ---------------------------- usage ----------------------------------

function usage {
  echo "Usage: qsmv.sh [-e smv_command] [-q queue] casename"
  echo ""
  echo "runs smokeview on the case casename.smv using the script casename.ssf"
  echo ""
  echo "options:"
  echo " -e exe - full path of smokeview used to run case "
  echo "    [default: $REPOROOT/smv/Build/smokeview/intel_linux_64/smokeview_intel_linux_64]"
  echo " -h   - show commonly used options"
  echo " -H   - show all options"
  echo " -p n - run n instances of smokeview each instance rendering 1/n'th of the total images"
  echo "        only use this option if you have a RENDERALL keyword in your .ssf smokeview script"
  echo " -q q - name of queue. [default: batch]"
  echo " -v   - output generated script"
  if [ "$HELP" == "" ]; then
    exit
  fi
  echo "Other options:"
  echo " -c     - smokeview script file [default: casename.ssf]"
  echo " -d dir - specify directory where the case is found [default: .]"
  echo " -i     - use installed smokeview"
  echo " -s     - first frame rendered [default: 1]"
  echo " -S     - interval between frames [default: 1]"
  echo ""
  exit
}

CURDIR=`pwd`

#*** define repo root

if [ "$FIREMODELS" == "" ]; then
  QSMV_PATH=$(dirname `which $0`)
  cd $QSMV_PATH/../../..
  REPOROOT=`pwd`
else
  REPOROOT=$FIREMODELS
fi

cd $CURDIR

#*** define xstart and xstop scripts used to start and stop X11 environment

XSTART=$REPOROOT/smv/Utilities/Scripts/startXserver.sh
XSTOP=$REPOROOT/smv/Utilities/Scripts/stopXserver.sh

#*** define resource manager that is used
#    (not tested with slurm yet)

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
c_arg=
d_arg=
e_arg=
i_arg=
q_arg=
v_arg=

if [ $# -lt 1 ]; then
  usage
fi

commandline=`echo $* | sed 's/-V//' | sed 's/-v//'`

#*** read in parameters from command line

while getopts 'c:d:e:hHip:q:s:S:v' OPTION
do
case $OPTION  in
  c)
   smv_script="$OPTARG"
   c_arg="-c $smv_script"
   ;;
  d)
   dir="$OPTARG"
   d_arg="-d $dir"
   ;;
  e)
   exe="$OPTARG"
   e_arg="-e $exe"
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
  p)
   nprocs="$OPTARG"
   ;;
  q)
   queue="$OPTARG"
   q_arg="-q $queue"
   ;;
  s)
   first="$OPTARG"
   render_opts=1
   ;;
  S)
   skip="$OPTARG"
   render_opts=1
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
if [ $nprocs != 1 ]; then
  for i in $(seq 1 $nprocs); do
    $QSMV $c_arg $d_arg $e_arg $i_arg $q_arg $v_arg -s $i -S $nprocs $in
  done
  exit
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
  smv_script="-script_file $smv_script"
else
  smokeview_script_file=${infile}.ssf
  smv_script=-runscript
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
    echo "smokeview is not installed. Run aborted."
    ABORTRUN=y
    exe=
  else
    smvpath=`which smokeview`
    smvdir=$(dirname "${smvpath}")
    curdir=`pwd`
    cd $smvdir
    exe=`pwd`/smokeview
    cd $curdir
  fi
else
  if [ "$exe" == "" ]; then
    exe=$REPOROOT/smv/Build/smokeview/intel_linux_64/smokeview_linux_64
  fi
fi

let ppn=$ncores
let nodes=1

TITLE="$infile"

cd $dir
fulldir=`pwd`

#*** define files

basefile=${infile}_f${first}_s${skip}
outerr=$fulldir/$basefile.err
outlog=$fulldir/$basefile.log
scriptlog=$fulldir/$basefile.slog
in_full_file=$fulldir/$in
smvfile=${in}.smv
in_full_smvfile=$fulldir/${in}.smv

#*** make sure files needed by qsmv.sh exist

if [ "$showinput" == "0" ]; then
  if ! [ -e $exe ]; then
    echo "The smokeview executable, $exe, does not exist."
    ABORTRUN=y
  fi

  if ! [ -e $smvfile ]; then
    echo "The smokeview file, $smvfile, does not exist."
    ABORTRUN=y
  fi

  if ! [ -e $smokeview_script_file ]; then
    echo "The smokeview script file, $smokeview_script_file, does not exist."
    ABORTRUN=y
  fi

  if [ "$ABORTRUN" == "y" ]; then
    echo "Run aborted."
    exit
  fi
fi

QSUB="qsub -q $queue"

#*** setup for SLURM (alternative to torque)

if [ "$RESOURCE_MANAGER" == "SLURM" ]; then
  QSUB="sbatch -p $queue --ignore-pbs"
  MPIRUN='srun'
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
# $0 $commandline
EOF

if [ "$queue" != "none" ]; then
  if [ "$RESOURCE_MANAGER" == "SLURM" ]; then
    cat << EOF >> $scriptfile
#SBATCH -J $JOBPREFIX$infile
#SBATCH -e $outerr
#SBATCH -o $outlog
#SBATCH -p $queue
#SBATCH -n $n_mpi_processes
####SBATCH --nodes=$nodes
#SBATCH --cpus-per-task=$n_openmp_threads
$SLURM_MEM
EOF
    if [ "$walltimestring_slurm" != "" ]; then
      cat << EOF >> $scriptfile
#SBATCH $walltimestring_slurm
EOF
    fi

  else
    cat << EOF >> $scriptfile
#PBS -N $JOBPREFIX${TITLE}/f${first}s$skip
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
echo "             Host: \`hostname\`"
echo "            Queue: $queue"

source $XSTART
$exe $script_file $smv_script $render_opts $in
source $XSTOP

EOF

#*** output script file to screen if -v option was selected

if [ "$showinput" == "1" ]; then
  cat $scriptfile
  rm $scriptfile
  echo
  exit
fi

#*** output info to screen

echo "     smokeview file: $smvfile"
echo "      smokeview exe: $exe"
echo "             script: $smokeview_script_file"
echo "        start frame: $first"
echo "         frame skip: $skip"
echo "              Queue: $queue"

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
