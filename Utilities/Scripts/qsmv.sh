#!/bin/bash

# ---------------------------- usage ----------------------------------

function usage {
  echo "Usage: qsmv.sh [-e smv_command] [-q queue]  casename"
  echo ""
  echo "qsmv.sh runs smokeview and a smokeview script in batch mode"
  echo ""
  echo " -e exe - full path of FDS used to run case "
  echo "    [default: $REPOROOT/smv/Build/smokeview/intel_${platform}_64/smokeview_intel_${platform}_64]"
  echo " -h   - show commonly used options"
  echo " -H   - show all options"
  echo " -q q - name of queue. [default: batch]"
  echo " -v   - output generated script to standard output"
  echo "input_file - input file"
  if [ "$HELP" == "" ]; then
    exit
  fi
  echo "Other options:"
  echo " -c   - smokeview script command file [default: casename.ssf]"
  echo " -d dir - specify directory where the case is found [default: .]"
  echo " -f repository root - name and location of repository where FDS is located"
  echo "    [default: $REPOROOT]"
  echo " -i use installed fds"
  echo " -m m - reserve m processes per node [default: 1]"
  echo " -s     - first frame rendered [default: 1]"
  echo " -S     - interval between frames [default: 1]"
  echo ""
  exit
}

#*** get directory containing qsmv.sh

QSMV_PATH=$(dirname `which $0`)
CURDIR=`pwd`
cd $QSMV_PATH
QSMV_DIR=`pwd`
cd $CURDIR

#*** define toplevel of the repos

REPOROOT=~/FDS-SMV
if [ "$FIREMODELS" != "" ]; then
  REPOROOT=$FIREMODELS
fi

#*** define xstart and xstop scripts

XSTART=$REPOROOT/smv/Utilities/Scripts/startXserver.sh
XSTOP=$REPOROOT/smv/Utilities/Scripts/stopXserver.sh

#*** define resource manager that is used

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

#*** determine platform

platform="linux"
if [ "`uname`" == "Darwin" ] ; then
  platform="osx"
fi

#*** determine default queue

if [ "$platform" == "osx" ]; then
  queue=none
else
  queue=batch
fi

#*** set default parameter values

HELP=
use_installed=
render_opts=
dir=.
showinput=0
exe=
smv_script=

if [ $# -lt 1 ]; then
  usage
fi

commandline=`echo $* | sed 's/-V//' | sed 's/-v//'`

#*** read in parameters from command line

while getopts 'c:d:e:f:hHim:q:s:S:v' OPTION
do
case $OPTION  in
  c)
   smv_script="$OPTARG"
   ;;
  d)
   dir="$OPTARG"
   ;;
  e)
   exe="$OPTARG"
   ;;
  f)
   REPOROOT="$OPTARG"
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
   ;;
  m)
   max_processes_per_node="$OPTARG"
   ;;
  q)
   queue="$OPTARG"
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
   ;;
esac
done
shift $(($OPTIND-1))

#*** define input file

in=$1
infile=${in%.*}

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
    exe=$REPOROOT/smv/Build/smokeview/intel_${platform}_64/smokeview_${platform}_64
  fi
fi

let ppn=1
let nodes=1

TITLE="$infile"

cd $dir
fulldir=`pwd`

#*** define files

outerr=$fulldir/$infile.err
outlog=$fulldir/$infile.log
stopfile=$fulldir/$infile.stop
scriptlog=$fulldir/$infile.slog
in_full_file=$fulldir/$in
in_full_smvfile=$fulldir/${in}.smv

#*** make sure smokeview file exists

if ! [ -e $in_full_smvfile ]; then
  if [ "$showinput" == "0" ]; then
    echo "The smokeview file, $in_full_smvfile, does not exist. Run aborted."
    ABORTRUN=y
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
#PBS -N $JOBPREFIX$TITLE
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
echo "   smokeview file: $in"
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

echo "         Input file:$in"
echo "         Executable:$exe"
echo "   smokeview script:$smokeview_script_file"
echo "        start frame:$first"
echo "         frame skip:$skip"
echo "              Queue:$queue"

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
