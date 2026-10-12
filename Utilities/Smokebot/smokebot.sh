#!/bin/bash

# The Smokebot script is part of an automated continuous integration system.
# Consult the FDS Config Management Plan for more information.

#---------------------------------------------
#                   CD_REPO
#---------------------------------------------

CD_REPO ()
{
  local repodir=$1
  
  if [ ! -e $repodir ]; then
     echo "***error: the repo directory $repodir does not exist."
     echo "          Aborting smokebot."
     return 1
  fi
  cd $repodir
  return 0
}

#---------------------------------------------
#                   GET_TIME
#---------------------------------------------

GET_TIME(){
  echo $(date +"%s")
}

#---------------------------------------------
#                   GET_DURATION
#---------------------------------------------

GET_DURATION(){
  local time_before=$1
  local time_after=$2
  
  DIFF_TIME=`echo $(($time_after-$time_before))`
  TIME_H=`echo $(($DIFF_TIME / 3600 ))`
  TIME_M=`echo $((($DIFF_TIME % 3600 ) / 60))`
  TIME_S=`echo $(($DIFF_TIME % 60 ))`
  if (( "$DIFF_TIME" >= 3600 )) ; then
    echo "${TIME_H}h ${TIME_M}m ${TIME_S}s"
  else
    if (( "$DIFF_TIME" >= 60 )) ; then
      echo "${TIME_M}m ${TIME_S}s"
    else
      echo "${TIME_S}s"
    fi
  fi
}

#---------------------------------------------
#                   check_time_limit
#---------------------------------------------

check_time_limit()
{
   if [ "$TIME_LIMIT_EMAIL_NOTIFICATION" == "sent" ]
   then
      # Continue along
      :
   else
      CURRENT_TIME=$(date +%s)
      ELAPSED_TIME=$(echo "$CURRENT_TIME-$START_TIME"|bc)

      if [[ "$HAVEMAIL" != "" ]] && [[ $ELAPSED_TIME -gt $TIME_LIMIT ]]; then
         echo -e "smokebot has been running for more than 12 hours in Stage ${TIME_LIMIT_STAGE}. \n\nPlease ensure that there are no problems. \n\nThis is a notification only and does not terminate smokebot." | mail $REPLYTO -s "smokebot Notice: smokebot has been running for more than 12 hours." $mailTo > /dev/null
         TIME_LIMIT_EMAIL_NOTIFICATION="sent"
      fi
   fi
}

#---------------------------------------------
#                   set_files_world_readable
#---------------------------------------------

set_files_world_readable()
{
   CD_REPO $smvrepo $SMVBRANCH || return 1
   chmod -R go+r *

   CD_REPO $fdsrepo $FDSBRANCH || return 1
   chmod -R go+r *

   return 0
}

#---------------------------------------------
#                   clean_repo
#---------------------------------------------

clean_repo()
{
  curdir=`pwd`
  local dir=$1
  local branch=$2
  
  CD_REPO $dir $branch || return 1
  git clean -dxf &> /dev/null
  git add . &> /dev/null
  git reset --hard HEAD &> /dev/null
  cd $curdir
  return 0
}

#---------------------------------------------
#                   clean_smokebot_history
#---------------------------------------------

clean_smokebot_history()
{
   
   # Clean Smokebot metafiles
   cd $smokebotdir
   mkdir -p guides               > /dev/null
   mkdir -p $HISTORY_DIR_ARCHIVE > /dev/null
   mkdir -p $OUTPUT_DIR          > /dev/null
   rm -rf $OUTPUT_DIR/*          > /dev/null
   mkdir -p $NEWGUIDE_DIR        > /dev/null
   chmod 775 $NEWGUIDE_DIR
}

#---------------------------------------------
#                   BUILDFDSLIBS
#---------------------------------------------

BUILDFDSLIBS()
{
# setup compilers
  export FDS_BUILD_TARGET=intel
  echo setting up compilers
  source $REPOROOT/fds/Build/Scripts/set_compilers.sh >& /dev/null

  echo building hypre
  source $REPOROOTo/fds/Build/Scripts/HYPRE/build_hypre.sh confmake.sh true >& /dev/null &
  pid_hypre=$!

  echo building sundials
  source $REPOROOT/fds/Build/Scripts/SUNDIALS/build_sundials.sh confmake.sh true >& /dev/null &
  pid_sundials=$1
  wait $pid_hypre
  echo hypre built
  wait $pid_sundials
  echo sundials built
}

#---------------------------------------------
#                   compile_cfast
#---------------------------------------------

compile_cfast()
{
   cd $SMOKEBOT_HOME_DIR

    # Build CFAST
    echo "building cfast"
    cd $cfastrepo/Build/CFAST/intel_linux
    rm -f cfast8_linux cfast7_linux
    make --makefile ../makefile clean &> /dev/null
    ./make_cfast.sh >> $OUTPUT_DIR/compile_cfast.log 2>&1
}

#---------------------------------------------
#                   check_compile_smvapps
#---------------------------------------------

check_compile_smvapps()
{
   if [[ `grep -i -E 'error' $OUTPUT_DIR/smverror.log | wc -l` -eq 0 ]]; then
      # Continue along
      :
   else
      echo "Stage 2 errors:"                        >> $ERROR_LOG
      grep -i -E 'error' $OUTPUT_DIR/smverror.log   >> $ERROR_LOG
      echo ""                                       >> $ERROR_LOG
   fi

}

#---------------------------------------------
#                   check_compile_cfast
#---------------------------------------------

check_compile_cfast()
{
   # Check for errors in CFAST compilation
   cd $cfastrepo/Build/CFAST/intel_linux
   if [[ ! -e "cfast8_linux" ]] && [[ ! -e cfast7_linux ]]; then
      echo "Errors from Stage 2 - build CFAST:" >> $ERROR_LOG
      echo "CFAST failed to compile"            >> $ERROR_LOG
      cat $OUTPUT_DIR/compile_cfast.log         >> $ERROR_LOG
      echo ""                                   >> $ERROR_LOG
      THIS_CFAST_FAILED=1
   else
      stage2_build_cfast=true
   fi
}

#---------------------------------------------
#                   check_update_repo
#---------------------------------------------

check_update_repo()
{
   # Check for GIT errors
   if [ -e $OUTPUT_DIR/stage1_clean_update_repos ]; then
     if [[ `grep -i -E 'warning|modified' $OUTPUT_DIR/stage1_clean_update_repos` == "" ]]
     then
        # Continue along
        :
     else
        echo "warnings from Stage 0 - Update repos"                                   >> $WARNING_LOG
        echo ""                                                                       >> $WARNING_LOG
        grep -A 5 -B 5 -i -E 'warning|modified' $OUTPUT_DIR/stage1_clean_update_repos >> $WARNING_LOG
        echo ""                                                                       >> $WARNING_LOG
     fi
   fi
}

#---------------------------------------------
#                   check_compile_fds_mpi_db
#---------------------------------------------

check_compile_fds_mpi_db()
{
  local FDSDIR=$1
  local FDSEXE=$2
# Check for errors in FDS debug compilation
   cd $FDSDIR
   if [ -e $FDSEXE ]; then
      stage_fdsdb_success=true
      cp $FDSEXE $LATESTAPPS_DIR/fds_db
   else
      echo "Errors from Stage 1b - Compile FDS MPI debug:" >> $ERROR_LOG
      cat $OUTPUT_DIR/stage2_build_fds_debug               >> $ERROR_LOG
      echo ""                                              >> $ERROR_LOG
      THIS_FDS_FAILED=1
      compile_errors=1
   fi

# Check for compiler warnings/remarks
  if [ -e $OUTPUT_DIR/compile_fds_db.log ]; then
    if [[ `grep -i -E 'warning|remark' $OUTPUT_DIR/compile_fds_db.log| grep -v mpiifort | grep -v mpiifx | grep -v 'pointer not aligned at address' | grep -v Referenced | grep -v ipo | grep -v 'find atom' | grep -v 'feupdateenv is not implemented'` == "" ]]
   then
      # Continue along
      :
    else
      echo "Stage 1b warnings:" >> $WARNING_LOG
      grep -A 5 -i -E 'warning|remark' $OUTPUT_DIR/compile_fds_db.log | grep -v mpiifort | grep -v mpiifx | grep -v 'pointer not aligned at address' | grep -v Referenced | grep -v ipo | grep -v 'find atom' | grep -v 'feupdateenv is not implemented'>> $WARNING_LOG
      echo "" >> $WARNING_LOG
   # if the executable does not exist then an email has already been sent
      if [ ! -e $FDSEXE ] ; then
        THIS_FDS_FAILED=1
      fi
      compile_errors=1
    fi
  fi
}


#---------------------------------------------
#                   run_verification_cases_debug
#---------------------------------------------

run_verification_cases_debug()
{
   #  ======================
   #  = Remove .stop files =
   #  ======================

   # Remove all .stop and .err files from Verification directories (recursively)
   rm -rf $smvrepo/Verification_dbg
   cp -r $smvrepo/Verification $smvrepo/Verification_dbg

   #  =====================
   #  = Run all SMV cases =
   #  =====================

   echo "running cases using debug fds"
   cd $smvrepo/Verification_dbg/scripts

   # Submit SMV verification cases and wait for them to start
   echo 'Running SMV verification cases:' >> $OUTPUT_DIR/stage3_run_debug 2>&1
   ./Run_SMV_Cases.sh $INTEL2 -c $cfastrepo $USEINSTALL2 -j $JOBPREFIXD -m 2 -d -q $QUEUE >> $OUTPUT_DIR/stage3_run_debug 2>&1 
}

#---------------------------------------------
#                   check_verification_cases_debug
#---------------------------------------------

check_verification_cases_debug()
{
   # Scan and report any errors in FDS verification cases
   cd $smvrepo/Verification_dbg

   if [[ `grep -rIi 'Run aborted' $OUTPUT_DIR/stage3_run_debug` == "" ]] && \
      [[ `grep -rIi 'Segmentation' Visualization/* WUI/* ` == "" ]] && \
      [[ `grep -rI  'ERROR.*:' Visualization/* WUI/* ` == "" ]] && \
      [[ `grep -rIi 'STOP: Numerical' Visualization/* WUI/* ` == "" ]] && \
      [[ `grep -rIi 'forrtl' Visualization/* WUI/* ` == "" ]]
   then
      stage3_run_debug_success=true
   else
      grep -rIi 'Run aborted' $OUTPUT_DIR/stage3_run_debug > $OUTPUT_DIR/stage3_run_debug_errors
      grep -rIi 'Segmentation' Visualization/* WUI/*  >> $OUTPUT_DIR/stage3_run_debug_errors
      grep -rI  'ERROR.*:' Visualization/* WUI/*  >> $OUTPUT_DIR/stage3_run_debug_errors
      grep -rIi 'STOP: Numerical' -rIi Visualization/* WUI/* >> $OUTPUT_DIR/stage3_run_debug_errors
      grep -rIi -A 20 'forrtl' Visualization/* WUI/*  >> $OUTPUT_DIR/stage3_run_debug_errors
      
      echo "Errors from Stage 3a - Run verification cases (debug mode):" >> $ERROR_LOG
      cat $OUTPUT_DIR/stage3_run_debug_errors >> $ERROR_LOG
      echo "" >> $ERROR_LOG
      THIS_FDS_FAILED=1
   fi
   if [[ `grep 'Warning' -irI $OUTPUT_DIR/stage3_run_debug | grep -v 'SPEC' | grep -v 'Sum of'` == "" ]] && \
      [[ `grep 'Warning' -irI Visualization/* WUI/*      | grep -v 'SPEC' | grep -v 'Sum of'` == "" ]]
   then
      no_warnings=true
   else
      echo "Stage 3b warnings:" >> $WARNING_LOG
      grep 'Warning' -irI $OUTPUT_DIR/stage3_run_debug | grep -v 'SPEC' | grep -v 'Sum of' >> $WARNING_LOG
      grep 'Warning' -irI Visualization/* WUI/*      | grep -v 'SPEC' | grep -v 'Sum of' >> $WARNING_LOG
      echo "" >> $WARNING_LOG
   fi
}

#---------------------------------------------
#                   check_compile_fds_mpi
#---------------------------------------------

check_compile_fds_mpi()
{
  local FDSDIR=$1
  local FDSEXE=$2

   # Check for errors in FDS compilation
   cd $FDSDIR
   if [ -e $FDSEXE ]
   then
      stage_fds_success=true
      cp $FDSEXE $LATESTAPPS_DIR/fds
   else
      echo "Errors from Stage 1c$MPTYPE - Compile FDS MPI$MPYPE release:" >> $ERROR_LOG
      cat $OUTPUT_DIR/compile_fds.log                                     >> $ERROR_LOG
      echo ""                                                             >> $ERROR_LOG
      compile_errors=1
   fi

   # Check for compiler warnings/remarks
   # 'performing multi-file optimizations' and 'generating object file' are part of a normal compile
   if [ -e $OUTPUT_DIR/compile_fds.log ]; then
   if [[ `grep -i -E 'warning|remark' $OUTPUT_DIR/compile_fds.log | grep -v 'pointer not aligned at address' | grep -v Referenced | grep -v ipo | grep -v 'find atom' | grep -v 'performing multi-file optimizations' | grep -v 'generating object file'| grep -v 'feupdateenv is not implemented'` == "" ]]
   then
      # Continue along
      :
   else
      echo "Stage 1c warnings:" >> $WARNING_LOG
      grep -A 5 -i -E 'warning|remark' $OUTPUT_DIR/compile_fds.log | grep -v 'pointer not aligned at address' | grep -v Referenced | grep -v ipo | grep -v 'find atom' | grep -v 'performing multi-file optimizations' | grep -v 'generating object file'| grep -v 'feupdateenv is not implemented' >> $WARNING_LOG
      echo "" >> $WARNING_LOG
      compile_errors=1
   fi
   fi
}

#---------------------------------------------
#                   compare_fds_smv_common_files
#---------------------------------------------

compare_fds_smv_common_files()
{
   fdsdir=$1
   smvdir=$2
   file=$3
   fds_file=$fdsrepo/$fdsdir/$file
   smv_file=$smvrepo/$smvdir/$file
   notexist=
   if ! [ -e $fds_file ]; then
     echo "Warnings Stage 2d" >> $WARNING_LOG
     echo "***warning: The fds repo file, $fds_file, does not exist" >> $WARNING_LOG
     notexist=1
   fi
   if ! [ -e $smv_file ]; then
     echo "Warnings Stage 2d" >> $WARNING_LOG
     echo "***warning: The smv repo file, $smv_file, does not exist" >> $WARNING_LOG
     notexist=1
   fi
   if [ "$notexist" == "" ]; then
     ndiffs=`diff $smv_file $fds_file | wc -l`
     if [ $ndiffs -gt 0 ]; then
       echo "" >> $WARNING_LOG
       echo "Warnings Stage 2d" >> $WARNING_LOG
       echo "***warning: The fds and smv versions of $file are out of synch" >> $WARNING_LOG
     fi
   fi
}

#---------------------------------------------
#                   check_common_files
#---------------------------------------------

check_common_files()
{
  # only compare files if latest repo revisions are checkout out
  if [ "$CHECKOUT" == "" ]; then
    compare_fds_smv_common_files Manuals/Bibliography Manuals/Bibliography BIBLIO_FDS_general.tex
    compare_fds_smv_common_files Manuals/Bibliography Manuals/Bibliography BIBLIO_FDS_mathcomp.tex
    compare_fds_smv_common_files Manuals/Bibliography Manuals/Bibliography BIBLIO_FDS_refs.tex
    compare_fds_smv_common_files Manuals/Bibliography Manuals/Bibliography authors.tex
    compare_fds_smv_common_files Manuals/Bibliography Manuals/Bibliography disclaimer.tex
  fi
}


#---------------------------------------------
#                   wait_verification_cases_end
#---------------------------------------------

wait_verification_cases_end()
{
   stage=$1
   stagelimit=$2
   prefix=$3
   # Scans and wait for verification cases to end
   while           [[ `squeue -o "%.18j %.8u %.2t" | awk '{print $1 $2 $3}' | grep $(whoami) | grep $prefix | grep -v 'C$'` != '' ]]; do
      JOBS_REMAINING=`squeue -o "%.18j %.8u %.2t" | awk '{print $1 $2 $3}' | grep $(whoami)  | grep $prefix | grep -v 'C$' | wc -l`
      echo "Waiting for ${JOBS_REMAINING} verification cases to complete." >> $OUTPUT_DIR/$stage
      TIME_LIMIT_STAGE=$stagelimit
      check_time_limit
      sleep 30
   done
}

#---------------------------------------------
#                   run_verification_cases_release
#---------------------------------------------

run_verification_cases_release()
{
   #  ======================
   #  = Remove .stop files =
   #  ======================

   echo "running cases using release fds"
   # Start running all SMV verification cases
   cd $smvrepo/Verification/scripts
   echo 'Running SMV verification cases:' >> $OUTPUT_DIR/stage3_run_release 2>&1
   ./Run_SMV_Cases.sh $INTEL2 -c $cfastrepo -j $JOBPREFIXR $USEINSTALL2 -q $QUEUE >> $OUTPUT_DIR/stage3_run_release 2>&1
   ./Run_RESTART_Cases.sh -q $QUEUE                                                                >> $OUTPUT_DIR/stage3_run_release 2>&1
}

#---------------------------------------------
#                   check_verification_cases_release
#---------------------------------------------

check_verification_cases_release()
{
   # Scan and report any errors in FDS verification cases
   cd $smvrepo/Verification

   if [[ `grep -rIi 'Run aborted' $OUTPUT_DIR/stage3_run_release` == "" ]] && \
      [[ `grep -rIi 'Segmentation' Visualization/* WUI/* ` == "" ]] && \
      [[ `grep -rI  'ERROR.*:' Visualization/* WUI/*  ` == "" ]] && \
      [[ `grep -rIi 'STOP: Numerical' Visualization/* WUI/*  ` == "" ]] && \
      [[ `grep -rIi  'forrtl' Visualization/* WUI/*  ` == "" ]]
   then
      stage3_run_release_success=true
   else
      grep -rIi 'Run aborted' $OUTPUT_DIR/stage3_run_release  > $OUTPUT_DIR/stage3_run_release_errors
      grep -rIi 'Segmentation' Visualization/* WUI/*     >> $OUTPUT_DIR/stage3_run_release_errors
      grep -rI  'ERROR.*:' Visualization/* WUI/*           >> $OUTPUT_DIR/stage3_run_release_errors
      grep -rIi 'STOP: Numerical' Visualization/* WUI/*  >> $OUTPUT_DIR/stage3_run_release_errors
      grep -rIi -A 20 'forrtl' Visualization/* WUI/*     >> $OUTPUT_DIR/stage3_run_release_errors

      echo "Errors from Stage 3b - Run verification cases (release mode):" >> $ERROR_LOG
      cat $OUTPUT_DIR/stage3_run_release_errors >> $ERROR_LOG
      echo "" >> $ERROR_LOG
      THIS_FDS_FAILED=1
   fi

      
   if [[ `grep 'Warning' -irI $OUTPUT_DIR/stage3_run_release | grep -v 'SPEC' | grep -v 'Sum of'` == "" ]] && \
      [[ `grep 'Warning' -irI Visualization/* WUI/*      | grep -v 'SPEC' | grep -v 'Sum of'` == "" ]]
   then
      no_warnings=true
   else
      echo "Stage 3b warnings:" >> $WARNING_LOG
      grep 'Warning' -irI $OUTPUT_DIR/stage3_run_release | grep -v 'SPEC' | grep -v 'Sum of' >> $WARNING_LOG
      grep 'Warning' -irI Visualization/* WUI/*      | grep -v 'SPEC' | grep -v 'Sum of' >> $WARNING_LOG
      echo "" >> $WARNING_LOG
   fi
}

#---------------------------------------------
#                   make_smv_pictures
#---------------------------------------------

make_smv_pictures()
{
   # Run Make SMV Pictures script (release mode)
   echo "generating images"
   cd $smvrepo/Verification/scripts
   ./Make_SMV_Pictures.sh $CPUS_PER_TASK -q $QUEUE -j SMV_ $USEINSTALL 2>&1 &> $OUTPUT_DIR/stage4_make_picts
   grep -v FreeFontPath $OUTPUT_DIR/stage4_make_picts | grep -v libpng &> $OUTPUT_DIR/stage4_check_picts
}

#---------------------------------------------
#                   check_smv_pictures
#---------------------------------------------

check_smv_pictures()
{
   # Scan and report any errors in make SMV pictures process
   grep -I -E -i Segmentation $smvrepo/Verification/Visualization/*.err >> $OUTPUT_DIR/stage4_check_picts
   grep -I -E -i Segmentation $smvrepo/Verification/WUI/*.err           >> $OUTPUT_DIR/stage4_check_picts
   cd $smokebotdir
   echo "checking image generation"
   if [[ `grep -I -E -i "Segmentation|Error" $OUTPUT_DIR/stage4_check_picts` == "" ]]
   then
      stage4_check_picts_smvpics_success=true
   else
      cp $OUTPUT_DIR/stage4_check_picts  $OUTPUT_DIR/stage4_check_picts_errors

      echo "Errors from Stage 4a - Make SMV pictures (release mode):" >> $ERROR_LOG
      grep -B 5 -A 5 -I -E -i "Segmentation|Error"  $OUTPUT_DIR/stage4_check_picts  >> $ERROR_LOG
      echo "" >> $ERROR_LOG
   fi
   if [[ `grep -I -E -i "Warning" $OUTPUT_DIR/stage4_check_picts` == "" ]]
   then
      # Continue along
      :
   else
      echo "Warnings from Stage 4a - Make SMV pictures (release mode):" >> $WARNING_LOG
      grep -A 2 -I -E -i "Warning" $OUTPUT_DIR/stage4_check_picts                     >> $WARNING_LOG
      echo "" >> $WARNING_LOG
   fi
}

#---------------------------------------------
#                   make_smv_movies
#---------------------------------------------

make_smv_movies()
{
   echo "generating movies"
   cd $smvrepo/Verification
   scripts/Make_SMV_Movies.sh -q $QUEUE 2>&1  &> $OUTPUT_DIR/stage4_make_movies
}

#---------------------------------------------
#                   check_smv_movies
#---------------------------------------------

check_smv_movies()
{
   cd $smokebotdir
   echo "checking movie generation"
   if [[ `grep -I -E -i "Segmentation|Error" $OUTPUT_DIR/stage4_make_movies` == "" ]]
   then
      stage4_make_movies_success=true
   else
      echo "Errors from Stage 4c - Make SMV movies "                    >> $ERROR_LOG
      grep -B 1 -A 1 -I -E -i "Segmentation|Error"  $OUTPUT_DIR/stage4_make_movies >  $OUTPUT_DIR/stage4_make_movies_errors
      grep -B 1 -A 1 -I -E -i "Segmentation|Error"  $OUTPUT_DIR/stage4_make_movies >> $ERROR_LOG
      echo ""                                                           >> $ERROR_LOG
   fi

   # Scan for and report any warnings in make SMV pictures process
   cd $smokebotdir
   if [[ `grep -I -E -i "Warning" $OUTPUT_DIR/stage4_make_movies` == "" ]]
   then
      # Continue along
      :
   else
      echo "Warnings from Stage 4b - Make SMV movies (release mode):" >> $WARNING_LOG
      grep -I -E -i "Warning" $OUTPUT_DIR/stage4_make_movies                 >> $WARNING_LOG
      echo ""                                                         >> $WARNING_LOG
   fi
}

#---------------------------------------------
#                   generate_timing_stats
#---------------------------------------------

generate_timing_stats()
{
   echo "generating timing stats"
   cd $smvrepo/Verification/scripts/
   export QFDS="$smvrepo/Verification/scripts/copyout.sh"
   export RUNCFAST="$smvrepo/Verification/scripts/copyout.sh"

   cd $smvrepo/Verification
   scripts/SMV_Cases.sh

   cd $smvrepo/Utilities/Scripts
   ./fds_timing_stats.sh smokebot > smv_timing_stats.csv
   TOTAL_SMV_TIMES=`tail -1 smv_timing_stats.csv`
   cd $smvrepo/Utilities/Scripts
   ./fds_timing_stats.sh smokebot 1 > smv_benchmarktiming_stats.csv
}

#---------------------------------------------
#                   archive_timing_stats
#---------------------------------------------

archive_timing_stats()
{
  echo "archiving timing stats"
  cd $smvrepo/Utilities/Scripts
  cp smv_timing_stats.csv          "$HISTORY_DIR_ARCHIVE/${SMV_REVISION}_timing.csv"
  cp smv_benchmarktiming_stats.csv "$HISTORY_DIR_ARCHIVE/${SMV_REVISION}_benchmarktiming.csv"
  sort -r -k 2 -t  ',' -n smv_timing_stats.csv | head -10 | awk -F',' '{print $1":", $2}' > $OUTPUT_DIR/slow_cases
  TOTAL_SMV_TIMES=`tail -1 smv_timing_stats.csv`
  if [[ "$UPLOADRESULTS" == "1" ]] && [[ "$USER" == "smokebot" ]]; then
    cd $smokebotdir
    ./smvstatus_updatepub.sh $REPOROOT/webpages $WEBBRANCH
  fi
}

#---------------------------------------------
#                   check_guide
#---------------------------------------------

check_guide()
{
   local stage=$1
   local directory=$2
   local document=$3
   local label=$4

   # Scan and report any errors in build process for guides

   SMOKEBOT_MAN_DIR=
   if [ "$WEB_DIR" != "" ]; then
     if [ -d $WEB_ROOT/$WEB_DIR/manuals ]; then
       SMOKEBOT_MAN_DIR=$WEB_ROOT/$WEB_DIR/manuals
     fi
   fi

   cd $smokebotdir
   if [[ `grep -I "successfully" $stage` == "" ]]; then
      echo "Errors from Stage 5 - Build Smokeview Guides:" >> $ERROR_LOG
      echo $label >> $ERROR_LOG
      cat $stage >> $ERROR_LOG
      echo "" >> $ERROR_LOG
   else
     if [ "$SMOKEBOT_MAN_DIR" != "" ]; then
       cp $directory/$document $SMOKEBOT_MAN_DIR/.
     fi
     chmod 664 $directory/$document
     cp $directory/$document $SMV_SUMMARY_DIR/manuals/.
     cp $directory/$document $NEWGUIDE_DIR/.
     cp $directory/$document $LATESTPUBS_DIR/$document
   fi

   # Check for LaTeX warnings (undefined references or duplicate labels)
   if [[ `grep -E "undefined|multiply defined|multiply-defined" -I ${stage}` == "" ]]
   then
      # Continue along
      :
   else
      echo "Stage 5 warnings:" >> $WARNING_LOG
      echo $label >> $WARNING_LOG
      cat $stage >> $WARNING_LOG
      echo "" >> $WARNING_LOG
   fi
}

#---------------------------------------------
#                   make_guide
#---------------------------------------------

make_guide()
{
   local document=$1
   local directory=$2
   local label=$3

   cd $directory
  
   ./make_guide.sh &> $OUTPUT_DIR/stage5_$document

   # Check guide for completion and copy to website if successful
   check_guide $OUTPUT_DIR/stage5_$document $directory $document.pdf $label
}

#---------------------------------------------
#                   save_build_status
#---------------------------------------------

save_build_status()
{
   HOST=`hostname -s`
   STOP_TIME=$(date)
   STOP_TIME_INT=$(date +%s)
   cd $smokebotdir
   # Save status outcome of build to a text file
   if [[ -e $WARNING_LOG && -e $ERROR_LOG ]]
   then
     echo "***Warnings:" >> $ERROR_LOG
     cat $WARNING_LOG >> $ERROR_LOG
     echo "   build failure and warnings for version: ${SMV_REVISION}, branch: $SMVBRANCH."
     echo "Build failure and warnings;$SMV_DATE;$SMV_SHORTHASH;$SMV_LONGHASH;${SMV_REVISION};$SMVBRANCH;$STOP_TIME_INT;3;$tTOTAL_SMV_TIMES;$HOST" > "$HISTORY_DIR_ARCHIVE/${SMV_REVISION}.txt"
     cat $ERROR_LOG > "$HISTORY_DIR_ARCHIVE/${SMV_REVISION}_errors.txt"

   # Check for errors only
   elif [ -e $ERROR_LOG ]
   then
      echo "   build failure for version: ${SMV_REVISION}, branch: $SMVBRANCH."
      echo "Build failure;$SMV_DATE;$SMV_SHORTHASH;$SMV_LONGHASH;${SMV_REVISION};$SMVBRANCH;$STOP_TIME_INT;3;$TOTAL_SMV_TIMES;$HOST" > "$HISTORY_DIR_ARCHIVE/${SMV_REVISION}.txt"
      cat $ERROR_LOG > "$HISTORY_DIR_ARCHIVE/${SMV_REVISION}_errors.txt"

   # Check for warnings only
   elif [ -e $WARNING_LOG ]
   then
      echo "   build success with warnings for version: ${SMV_REVISION}, branch: $SMVBRANCH."
      echo "Build success with warnings;$SMV_DATE;$SMV_SHORTHASH;$SMV_LONGHASH;${SMV_REVISION};$SMVBRANCH;$STOP_TIME_INT;2;$TOTAL_SMV_TIMES;$HOST" > "$HISTORY_DIR_ARCHIVE/${SMV_REVISION}.txt"
      cat $WARNING_LOG > "$HISTORY_DIR_ARCHIVE/${SMV_REVISION}_warnings.txt"

   # No errors or warnings
   else
      echo "   build success for version: ${SMV_REVISION}, branch: $SMVBRANCH."
      echo "Build success!;$SMV_DATE;$SMV_SHORTHASH;$SMV_LONGHASH;${SMV_REVISION};$SMVBRANCH;$STOP_TIME_INT;1;$TOTAL_SMV_TIMES;$HOST" > "$HISTORY_DIR_ARCHIVE/${SMV_REVISION}.txt"
   fi
}

#---------------------------------------------
#                   save_manuals_dir
#---------------------------------------------

save_manuals_dir()
{
  if [[ ! -e $WARNING_LOG && ! -e $ERROR_LOG ]]
  then
    echo "   archiving Manuals directory"
    rm -rf $MANUAL_DIR_ARCHIVE
    cp -r $smvrepo/Manuals $MANUAL_DIR_ARCHIVE
    if [ "$MAKEMOVIES" == "1" ]; then
      echo "   archiving Manuals directory (movies)"
      rm -rf $MOVIEMANUAL_DIR_ARCHIVE
      cp -r $smvrepo/Manuals $MOVIEMANUAL_DIR_ARCHIVE
    fi
    rm -rf $PUBS_DIR
    cp  -r $LATESTPUBS_DIR $PUBS_DIR
  fi
}

#---------------------------------------------
#                   email_compile_errors
#---------------------------------------------

email_compile_errors()
{
  SMOKEBOT_LOG=/tmp/smokebot_log.$$
  if [[ -e $ERROR_LOG ]]; then
    echo "" > $SMOKEBOT_LOG
  fi
  if [[ -e $WARNING_LOG ]]; then
    echo "" > $SMOKEBOT_LOG
  fi

  if [[ -e $ERROR_LOG ]]; then
    echo "----------------------------------------------" >> $SMOKEBOT_LOG
    echo "---------------- errors ----------------------" >> $SMOKEBOT_LOG
    echo "----------------------------------------------" >> $SMOKEBOT_LOG
    cat $ERROR_LOG >> $SMOKEBOT_LOG
  fi 
  if [[ -e $WARNING_LOG ]]; then
    echo "----------------------------------------------" >> $SMOKEBOT_LOG
    echo "---------------- warnings --------------------" >> $SMOKEBOT_LOG
    echo "----------------------------------------------" >> $SMOKEBOT_LOG
    cat $WARNING_LOG >> $SMOKEBOT_LOG
  fi 

  if [[ "$HAVEMAIL" != "" ]] && [[ -e $SMOKEBOT_LOG ]]; then
    cat $SMOKEBOT_LOG | mail $REPLYTO -s "smokebot compile errors and/or warnings on ${hostname}. ${SMV_REVISION}, $SMVBRANCH" $mailToSMV > /dev/null
    rm -f $SMOKEBOT_LOG
  fi
}

#---------------------------------------------
#                   email_build_status
#---------------------------------------------

email_build_status()
{
  if [[ "$THIS_FDS_FAILED" == "1" ]] ; then
    mailTo="$mailToFDS"
  fi
  if [[ "$THIS_CFAST_FAILED" == "1" ]] ; then
    mailTo="$mailToCFAST"
  fi
  echo $THIS_FDS_FAILED>$FDS_STATUS_FILE
  stop_time=`date`
  icx -v >& compiler_version.out
  ICC_VERSION=`cat compiler_version.out | head -1`
  rm compiler_version.out
  echo "----------------------------------------------"      > $TIME_LOG
  echo "host: $hostname"                                    >> $TIME_LOG
  echo "OS: Linux"                                          >> $TIME_LOG
  echo "repo: $REPOROOT"                                    >> $TIME_LOG
  echo "queue: $QUEUE"                                      >> $TIME_LOG
  echo "cpus per task: $CPUS_PER_TASK_ARG"                  >> $TIME_LOG
  if [ "$ICC_VERSION" != "" ]; then
    echo "C/C++: $ICC_VERSION "                             >> $TIME_LOG
  fi
  echo ""                                                   >> $TIME_LOG
  echo "$CFAST_REVISION/$CFASTBRANCH"                       >> $TIME_LOG
  echo "$FDS_REVISION/$FDSBRANCH"                           >> $TIME_LOG
  echo "$FIG_REVISION/$FIGBRANCH"                           >> $TIME_LOG
  echo "$SMV_REVISION/$SMVBRANCH"                           >> $TIME_LOG
  echo ""                                                   >> $TIME_LOG
  echo "start time: $start_time "                           >> $TIME_LOG
  echo "stop time: $stop_time "                             >> $TIME_LOG
  if [ "$CLONE_REPO_BRANCH" == "" ]; then
    echo "setup repos: $DIFF_CLONE"                         >> $TIME_LOG
  else
    echo "clone repos: $DIFF_CLONE"                         >> $TIME_LOG
  fi
  echo "setup smokebot: $DIFF_SETUP"                        >> $TIME_LOG
  echo "build software: $DIFF_BUILDSOFTWARE"                >> $TIME_LOG
  echo "run cases: $DIFF_RUN_CASES"                         >> $TIME_LOG
  echo "make pictures: $DIFF_MAKEPICTURES"                  >> $TIME_LOG
  if [ "$MAKEMOVIES" == "1" ]; then
    echo "make movies: $DIFF_MAKEMOVIES"                    >> $TIME_LOG
  fi
  if [ "$DIFF_MAKEGUIDES" != "" ]; then
    echo "build guides/compare images: $DIFF_MAKEGUIDES"    >> $TIME_LOG
  fi
  echo "total: $DIFF_SCRIPT_TIME"                           >> $TIME_LOG
  echo ""                                                   >> $TIME_LOG
  DISPLAY_FDS_REVISION=
  DISPLAY_SMV_REVISION=
  if [ "$DISPLAY_FDS_REVISION" == "1" ]; then
    echo "FDS revisions: old: $LAST_FDS_REVISION new: $THIS_FDS_REVISION" >> $TIME_LOG
  fi
  if [ "$DISPLAY_SMV_REVISION" == "1" ]; then
    echo "SMV revisions: old: $LAST_SMV_REVISION new: $THIS_SMV_REVISION" >> $TIME_LOG
  fi
  SOURCE_CHANGED=
  if [[ $THIS_SMV_REVISION != $LAST_SMV_REVISION ]] ; then
    SOURCE_CHANGED=1
    cat $GIT_SMV_LOG_FILE >> $TIME_LOG
  fi
  if [[ $THIS_FDS_REVISION != $LAST_FDS_REVISION ]] ; then
    SOURCE_CHANGED=1
    cat $GIT_FDS_LOG_FILE >> $TIME_LOG
  fi
  if [ "$SOURCE_CHANGED" != "" ]; then
    if [[ $THIS_VER_REVISION != $LAST_VER_REVISION ]] ; then
      cat $GIT_VER_LOG_FILE >> $TIME_LOG
    fi
  fi
  if [ "$NAMELIST_NODOC_STATUS" != "" ]; then
    if [ "$NAMELIST_NODOC_STATUS" == "0" ]; then
     echo "undocumented namelist keywords: $NAMELIST_NODOC_STATUS" >> $TIME_LOG
    fi
  else
    NAMELIST_NODOC_LOG=
  fi
  if [ "$NAMELIST_NOSOURCE_STATUS" == "" ]; then
    NAMELIST_NOSOURCE_LOG=
  fi
  cd $smokebotdir
  echo ""  >> $TIME_LOG
  # Check for warnings and errors
  if [[ "$WEB_URL" != "" ]] && [[ "$UPDATED_WEB_IMAGES" == "1" ]]; then
    if [ -e $IMAGE_DIFFS ]; then
      NUM_CHANGES=`cat $IMAGE_DIFFS | awk '{print $1}'`
      NUM_ERRORS=`cat $IMAGE_DIFFS | awk '{print $2}'`
      echo "images: $WEB_URL, errors/changes: $NUM_ERRORS/$NUM_CHANGES"  >> $TIME_LOG
    else
      echo "images: $WEB_URL" >> $TIME_LOG
    fi
  fi

  if [ -e $KEYWORDS_NODOC_LOG ]; then
     echo ""                 >> $TIME_LOG
     cat $KEYWORDS_NODOC_LOG >> $TIME_LOG
  fi
  if [  -e $KEYWORDS_NOSOURCE_LOG ]; then
     echo ""                    >> $TIME_LOG
     cat $KEYWORDS_NOSOURCE_LOG >> $TIME_LOG
  fi

  if [[ "$WEB_URL" == "" ]]; then
    if [ -e $IMAGE_DIFFS ]; then
      NUM_CHANGES=`cat $IMAGE_DIFFS | awk '{print $1}'`
      NUM_ERRORS=`cat $IMAGE_DIFFS | awk '{print $2}'`
      echo "images errors/changes: $NUM_ERRORS/$NUM_CHANGES"  >> $TIME_LOG
    fi
  fi
  is_bot=
  if [ `whoami` == "firebot" ]; then
    is_bot=1
  fi
  if [ `whoami` == "cfast" ]; then
    is_bot=1
  fi
  if [ `whoami` == "smokebot" ]; then
    is_bot=1
  fi
  if [[ ! -e $WARNING_LOG ]] && [[ ! -e $ERROR_LOG ]]; then
# save apps that were built for bundling
    rm -f $APPS_DIR/*
    cp $LATESTAPPS_DIR/* $APPS_DIR/.

    rm -f $BRANCHAPPS_DIR/*
    cp $LATESTAPPS_DIR/* $BRANCHAPPS_DIR/.

    rm -f $BRANCHPUBS_DIR/*
    cp $LATESTPUBS_DIR/* $BRANCHPUBS_DIR/.
  fi
  if [ "$UPLOADRESULTS" == "1" ]; then
    echo "status: https://pages.nist.gov/fds-smv/smokebot_status.html" >> $TIME_LOG
    if [[ "$is_bot" == "1" ]]; then
      GITURL=https://github.com/$GH_OWNER/$GH_REPO/releases/tag/$GH_SMOKEVIEW_TAG
      echo "Bundles/Guides/Figures: $GITURL"  >> $TIME_LOG
      echo  "***output guides, figures and image summary to Github"             > output/stage_GHupload
      echo  ""                                                  >> output/stage_GHupload
      $UploadSummaryGH                                          &>> output/stage_GHupload
      if [[ ! -e $WARNING_LOG ]] && [[ ! -e $ERROR_LOG ]]; then
        $UploadGuidesGH                                          &>> output/stage_GHupload
      fi
    fi
  fi
  echo ""                                  >> $TIME_LOG
  if [ -e $OUTPUT_DIR/slow_cases ]; then
    echo "cases with longest runtime:"     >> $TIME_LOG
    cat $OUTPUT_DIR/slow_cases             >> $TIME_LOG
    echo ""                                >> $TIME_LOG
  fi
  NAMELIST_LOGS="$NAMELIST_NODOC_LOG $NAMELIST_NOSOURCE_LOG"
  if [[ -e $WARNING_LOG && -e $ERROR_LOG ]]; then
    # Send email with failure message and warnings, body of email contains appropriate log file
    SUBJECT="smokebot failure and warnings on ${hostname}. ${SMV_REVISION}, $SMVBRANCH"
    if [ "$HAVEMAIL" != "" ]; then
      cat $ERROR_LOG $TIME_LOG $NAMELIST_LOGS | mail $REPLYTO -s "$SUBJECT" $mailTo > /dev/null
    fi
    cat $ERROR_LOG $TIME_LOG $NAMELIST_LOGS > $FULL_LOG

  # Check for errors only
  elif [ -e $ERROR_LOG ]; then
    # Send email with failure message, body of email contains error log file
    SUBJECT="smokebot failure on ${hostname}. ${SMV_REVISION}, $SMVBRANCH"
    if [ "$HAVEMAIL" != "" ]; then
      cat $ERROR_LOG $TIME_LOG $NAMELIST_LOGS | mail $REPLYTO -s "$SUBJECT" $mailTo > /dev/null
    fi
    cat $ERROR_LOG $TIME_LOG $NAMELIST_LOGS > $FULL_LOG

  # Check for warnings only
  elif [ -e $WARNING_LOG ]; then
     # Send email with success message, include warnings
    SUBJECT="smokebot success with warnings on ${hostname}. ${SMV_REVISION}, $SMVBRANCH"
    if [ "$HAVEMAIL" != "" ]; then
      cat $WARNING_LOG $TIME_LOG $NAMELIST_LOGS | mail $REPLYTO -s "$SUBJECT" $mailTo > /dev/null
    fi
    cat $WARNING_LOG $TIME_LOG $NAMELIST_LOGS > $FULL_LOG

  # No errors or warnings
  else
# upload guides to a google drive directory
    if [ "$UPLOADRESULTS" == "1" ]; then
      cd $smokebotdir
      echo  "***output guides to Github"  &> output/stage_upload
      echo  ""                            &>> output/stage_upload
      $UploadWEB                  $smvrepo/Manuals $MAKEMOVIES &>> output/stage_upload
    fi

      # Send success message with links to nightly manuals

    SUBJECT="smokebot success on ${hostname}. ${SMV_REVISION}, $SMVBRANCH"
    if [ "$HAVEMAIL" != "" ]; then
      cat $TIME_LOG $NAMELIST_LOGS | mail $REPLYTO -s "$SUBJECT" $mailTo > /dev/null
    fi
    cat $TIME_LOG $NAMELIST_LOGS > $FULL_LOG
  fi
  if [ "$HAVEMAIL" == "" ]; then
    cat $FULL_LOG
    echo ""
    echo "smokebot status: $SUBJECT"
  fi
}

#VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
#                             beginning of smokebot.sh
#^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#*** define initial values

SCRIPT_TIME_beg=`GET_TIME`
CLONE_beg=`GET_TIME`
smokebotdir=`pwd`
OUTPUT_DIR="$smokebotdir/output"
HISTORY_DIR_ARCHIVE="$HOME/.smokebot/history"
MANUAL_DIR_ARCHIVE=$HOME/.smokebot/Manuals
MOVIEMANUAL_DIR_ARCHIVE=$HOME/.smokebot/MovieManuals
KEYWORDS_NODOC_LOG=$OUTPUT_DIR/keywords_nodoc.txt
KEYWORDS_NOSOURCE_LOG=$OUTPUT_DIR/keywords_nosource.txt

LATESTPUBS_DIR=$HOME/.smokebot/pubs_latest
PUBS_DIR=$HOME/.smokebot/pubs

EMAIL_LIST="$HOME/.smokebot/smokebot_email_list.sh"
TIME_LOG=$OUTPUT_DIR/timings
ERROR_LOG=$OUTPUT_DIR/errors
FULL_LOG=$OUTPUT_DIR/full_log
WARNING_LOG=$OUTPUT_DIR/warnings
FYI_LOG=$OUTPUT_DIR/fyis
STAGE_STATUS=$OUTPUT_DIR/stage_status
NEWGUIDE_DIR=$OUTPUT_DIR/Newest_Guides
WEB_DIR=
WEB_ROOT=
UPDATED_WEB_IMAGES=
export SCRIPTFILES=$smokebotdir/scriptfiles

QUEUE=smokebot
MAKEMOVIES=0
mailTo=
mailToArg=
UPLOADRESULTS=
PID_FILE=~/.fdssmvgit/firesmokebot_pid
HTML2PDF=wkhtmltopdf
CLONE_REPO_BRANCH=
CHECKOUT=
compile_errors=
GITURL=
HAVEMAIL=`which mail |& grep -v 'no mail'`
INTEL2="-J"
CPUS_PER_TASK_ARG=16
REPOROOT=
USE_FDS_CACHE=

#*** save pid so -k option (kill smokebot) may be used lateer

echo $$ > $PID_FILE

#*** parse command line options

while getopts 'F:G:m:Mq:R:ST:Uw:W:' OPTION
do
case $OPTION in
  F)
   FDSCACHEDIR="$OPTARG"
   ;;
  G)
   export REPOROOT=$HOME/"$OPTARG"
   ;;
  m)
   mailTo="$OPTARG"
   mailToArg="$OPTARG"
   ;;
  M)
   MAKEMOVIES="1"
   ;;
  q)
   QUEUE="$OPTARG"
   ;;
  R)
   CLONE_REPO_BRANCH="$OPTARG"
   ;;
  T)
   CPUS_PER_TASK_ARG="$OPTARG"
   ;;
  U)
   UPLOADRESULTS=1
   ;;
  w)
   WEB_DIR="$OPTARG"
   ;;
  W)
   WEB_ROOT="$OPTARG"
   ;;
esac
done
shift $(($OPTIND-1))

if [ "$REPOROOT" == "" ]; then
  echo "***error: root directory not defined"
  exit
fi

CPUS_PER_TASK="-T $CPUS_PER_TASK_ARG"

if [ "$FDSCACHEDIR" != "" ]; then
  USE_FDS_CACHE=1
  FDSDEBUG=$FDSCACHEDIR/Build/impi_intel_linux_db/fds_impi_intel_linux_db
  FDSRELEASE=$FDSCACHEDIR/Build/impi_intel_linux/fds_impi_intel_linux
  if [[ ! -x $FDSDEBUG || ! -x $FDSRELEASE ]]; then
    FDSDEBUG=
    FDSRELEASE=
    USE_FDS_CACHE=
  fi
fi

#*** make sure smokebot is running in the right directory

if [ ! -e run_smokebot.sh ]; then
  echo "***error: smokebot not running in the smv/Smokebot directory"
  echo "          Aborting smokebot"
  exit 1
fi

#*** create pub directory

mkdir -p $HOME/.smokebot
mkdir -p $PUBS_DIR
rm -rf $LATESTPUBS_DIR
mkdir -p $LATESTPUBS_DIR

APPS_DIR=$HOME/.smokebot/apps
LATESTAPPS_DIR=$HOME/.smokebot/apps_latest

mkdir -p $APPS_DIR
rm -rf $LATESTAPPS_DIR
mkdir -p $LATESTAPPS_DIR

cfastrepo=$REPOROOT/cfast
fdsrepo=$REPOROOT/fds
smvrepo=$REPOROOT/smv
figrepo=$REPOROOT/fig

if [ "`uname`" == "Darwin" ]; then
 echo "***error: smokebot only runs on Linux computers"
 exit
fi

FDS_DB_DIR=$fdsrepo/Build/impi_intel_linux_db
FDS_DB_EXE=fds_impi_intel_linux_db

FDS_DIR=$fdsrepo/Build/impi_intel_linux
FDS_EXE=fds_impi_intel_linux

# clean smokebot output files

clean_smokebot_history

#*** write out file when firebot first starts
date > $OUTPUT_DIR/stage0_start 2>&1

if [[ "$CLONE_REPO_BRANCH" != "" ]]; then
  echo Cloning repos
  cd smokebotdir

  ./setup_repos.sh -K cfast -B $CLONE_REPO_BRANCH  > $OUTPUT_DIR/stage1_clone_repos  2>&1 &
  pid_cfast=$!
  ./setup_repos.sh -K fds   -B $CLONE_REPO_BRANCH >> $OUTPUT_DIR/stage1_clone_repos  2>&1 &
  pid_fds=$!
  ./setup_repos.sh -K fig   -B $CLONE_REPO_BRANCH >> $OUTPUT_DIR/stage1_clone_repos  2>&1 &
  pid_fig=$!
  ./update_repos.sh -K smv  -B $CLONE_REPO_BRANCH >  $OUTPUT_DIR/stage1_update_repos 2>&1 &
  pid_$smv=$!

  wait $pid_cfast
  wait $pid_fds
  wait $pid_fig
  wait $pid_smv
fi

#*** make sure repos exist

CD_REPO $cfastrepo || exit 1
CFASTBRANCH=`git rev-parse --abbrev-ref HEAD`

CD_REPO $fdsrepo || exit 1
FDSBRANCH=`git rev-parse --abbrev-ref HEAD`

CD_REPO $figrepo ||  exit 1
FIGBRANCH=`git rev-parse --abbrev-ref HEAD`

CD_REPO $smvrepo ||  exit 1
SMVBRANCH=`git rev-parse --abbrev-ref HEAD`

#save apps and pubs in directories under .smokebot/$SMVBRANCH
BRANCH_DIR=$HOME/.smokebot/$SMVBRANCH
BRANCHPUBS_DIR=$BRANCH_DIR/pubs
BRANCHAPPS_DIR=$BRANCH_DIR/apps
mkdir -p $BRANCHPUBS_DIR
mkdir -p $BRANCHAPPS_DIR

# if -a option is invoked, only proceed running smokebot if the
# smokeview or FDS source has changed

if [ "$WEB_ROOT" == "" ]; then
  WEB_DIR=""
fi
if [ "$WEB_DIR" != "" ]; then
  mkdir -p $WEB_ROOT/$WEB_DIR
  if [ -d $WEB_ROOT/$WEB_DIR ]; then
    testfile=$WEB_ROOT/$WEB_DIR/test.$$
    touch $testfile >& /dev/null
    if [ -e $testfile ]; then
      rm $testfile
    else
      WEB_DIR=
    fi
  else
    WEB_DIR=
  fi
fi
if [ "$WEB_DIR" != "" ]; then
  if [ "$WEB_URL_BASE" == "" ]; then
    WEB_HOST=`hostname -A | awk '{print $2}'`
    WEB_URL_BASE=http://$WEB_HOST
  fi
  WEB_URL=$WEB_URL_BASE/$WEB_DIR
else
  WEB_URL=
fi

notfound=`icx -help 2>&1 | tail -1 | grep "not found" | wc -l`
if [ "$notfound" == "1" ] ; then
  export haveCC="0"
  USEINSTALL="-i"
  USEINSTALL2="-u"
else
  export haveCC="1"
  USEINSTALL=
  USEINSTALL2=
fi

echo ""
echo "Smokebot Settings"
echo "-----------------"
echo "  CFAST repo;branch: $cfastrepo;$CFASTBRANCH"
echo "    FDS repo;branch: $fdsrepo;$FDSBRANCH"
echo "    FIG repo;branch: $figrepo;$FIGBRANCH"
echo "    SMV repo;branch: $smvrepo;$SMVBRANCH"
echo "      run directory: $smokebotdir"
if [ "$WEB_DIR" != "" ]; then
  echo "     web dir: $WEB_ROOT/$WEB_DIR"
fi
if [ "$WEB_URL" != "" ]; then
  echo "         URL: $WEB_URL"
fi
echo ""

cd

SMV_SUMMARY_DIR=$smvrepo/Manuals/SMV_Summary
IMAGE_DIFFS=$SMV_SUMMARY_DIR/image_differences

UploadGuidesGH=$smokebotdir/smv_guides2GH.sh
UploadSummaryGH=$smokebotdir/smv_summary2GH.sh
UploadWEB=$smokebotdir/smv_web2GD.sh

THIS_FDS_AUTHOR=
THIS_FDS_FAILED=0
THIS_CFAST_FAILED=0
FDS_STATUS_FILE=$smvrepo/FDS_status
LAST_FDS_FAILED=0
if [ -e $FDS_STATUS_FILE ] ; then
  LAST_FDS_FAILED=`cat $FDS_STATUS_FILE`
fi


# Load mailing list for status report

if [ -e $EMAIL_LIST ]; then
  source $EMAIL_LIST
fi

# define reply to address to prevent bounced emails when doing a reply all to smokebot's status emails

REPLYTO=
if [ "$replyToSMV" != "" ]; then
  REPLYTO="-S replyto=\"$replyToSMV\""
fi

if [ "$mailToArg" != "" ]; then
  mailToFDS=$mailToArg
  mailToSMV=$mailToArg
  mailToCFAST=$mailToArg
fi
if [ "$mailTo" == "" ]; then
  if [ -e $EMAIL_LIST ]; then
    mailTo=$mailToSMV
    if [[ "$LAST_FDS_FAILED" == "1" ]] ; then
      mailTo=$mailToFDS
    fi
  fi
fi
if [ "$mailTo" == "" ]; then
  mailTo=`git config user.email`
fi
if [ "$mailTo" == "" ]; then
  mailTo=`whoami`@`hostname`
fi
if [ "$mailToSMV" == "" ]; then
  mailToSMV=$mailTo
fi
if [ "$mailToFDS" == "" ]; then
  mailToFDS=$mailTo
fi
if [ "$mailToCFAST" == "" ]; then
  mailToCFAST=$mailTo
fi

JOBPREFIXR=SBR_
JOBPREFIXD=SBD_

#  =============================================
#  = Smokebot timing and notification mechanism =
#  =============================================

# This routine checks the elapsed time of Smokebot.
# If Smokebot runs more than 12 hours, an email notification is sent.
# This is a notification only and does not terminate Smokebot.
# This check runs during Stages 3 and 5.

# Start timer
START_TIME=$(date +%s)

# Set time limit (43,200 seconds = 12 hours)
TIME_LIMIT=43200
TIME_LIMIT_EMAIL_NOTIFICATION="unsent"


echo "" > $STAGE_STATUS
hostname=`hostname`
start_time=`date`

### Stage 0 repo operatoins ###
echo "Run Status"
echo "----------"

check_update_repo

CLONE_end=`GET_TIME`
DIFF_CLONE=`GET_DURATION $CLONE_beg $CLONE_end`
if [ "$CLONE_REPO_BRANCH" == "" ]; then
  echo "Setup repos: $DIFF_CLONE" >> $STAGE_STATUS
else
  echo "Cone repos: $DIFF_CLONE" >> $STAGE_STATUS
fi

#define repo revisions
SETUP_beg=`GET_TIME`

rm -f $FYI_LOG
touch $FYI_LOG
cd $cfastrepo
CFAST_REVISION=`git describe --abbrev=7 --long --dirty`

cd $fdsrepo
FDS_REVISION=`git describe --abbrev=7 --long --dirty`

cd $figrepo
FIG_REVISION=`git describe --abbrev=7 --long --dirty`

# copy smv revision and hash to the latest pubs and apps directory
cd $smvrepo

SMV_REVISION=`git describe --abbrev=7 --long --dirty`
SMV_SHORTHASH=`git rev-parse --short HEAD`
SMV_LONGHASH=`git rev-parse HEAD`
SMV_DATE=`git log -1 --format=%cd --date=local $SMV_SHORTHASH`

echo $FDS_REVISION > $smvrepo/Manuals/FDS_REVISION
echo $SMV_REVISION > $smvrepo/Manuals/SMV_REVISION

subrev=`git describe --abbrev | awk -F '-' '{print $2}'`
if [ "$subrev" == "" ]; then
  git describe --abbrev | awk -F '-' '{print $1"-0"}' > $LATESTAPPS_DIR/SMV_REVISION
else
  git describe --abbrev | awk -F '-' '{print $1"-"$2"-"$3}' > $LATESTAPPS_DIR/SMV_REVISION
fi
git rev-parse --short HEAD > $LATESTAPPS_DIR/SMV_HASH

cp $LATESTAPPS_DIR/SMV_REVISION $LATESTPUBS_DIR/SMV_REVISION
cp $LATESTAPPS_DIR/SMV_HASH     $LATESTPUBS_DIR/SMV_HASH

SETUP_end=`GET_TIME`
DIFF_SETUP=`GET_DURATION $SETUP_beg $SETUP_end`
echo "Setup smokebot: $DIFF_SETUP" >> $STAGE_STATUS

#----------------------------- Stage 1 build cfast and FDS     --------------------------------------

BUILDSOFTWARE_beg=`GET_TIME`

#*** stage 2 - build cfast
echo "Building"

cd $smokebotdir
pid_fds_mpi_db=
pid_fds_mpi=
if [ "$USE_FDS_CACHE" != "" ]; then
  cp $FDSDEBUG $fdsrepo/Build/impi_intel_linux_db/fds_impi_intel_linux_db
  cp $FDSRELEASE $fdsrepo/Build/impi_intel_linux/fds_impi_intel_linux
else
  if [ -z "${FIREMODELS}" ]; then
    export FIREMODELS=$REPOROOT
  fi 

# build fds apps

  BUILDFDSLIBS

  echo building debug fds
  cd $REPOROOT/fds/Build/impi_intel_linux_db
  git clean -dxf >& /dev/null
  ./make_fds.sh bot  > $OUTPUT_DIR/compile_fdsdb.log 2>&1 &
  pid_fds_mpi_db=$!

  echo building release fds
  cd $REPOROOT/fds/Build/impi_intel_linux
  git clean -dxf >& /dev/null
  ./make_fds.sh bot  > $OUTPUT_DIR/compile_fds.log 2>&1 &
  pid_fds_mpi=$!
fi

#*** stage 2 build cfast
compile_cfast        &
pid_cfast=$!

#----------------------------- Stage 2 build smokeview     --------------------------------------

#*** stage 2 - build smokeview ustilities

cd $smokebotdir
./make_smvapps.sh $REPOROOT &
pid_smvapps=$!

RUN_CASES=

wait $pid_cfast
check_compile_cfast

#*** stage 3 - run debug cases
if [ "$pid_fds_mpi_db" != "" ]; then
  wait $pid_fds_mpi_db
  echo "debug fds built"
fi
if [ "$FDSDEBUG" == "" ]; then
  check_compile_fds_mpi_db  $FDS_DB_DIR        $FDS_DB_EXE
fi
if [[ $stage_fdsdb_success || "$FDSDEBUG" != "" ]]; then
  run_verification_cases_debug
  RUN_CASES=1
fi

BUILDSOFTWARE_end=`GET_TIME`
DIFF_BUILDSOFTWARE=`GET_DURATION $BUILDSOFTWARE_beg $BUILDSOFTWARE_end`
echo "Build Software: $DIFF_BUILDSOFTWARE" >> $STAGE_STATUS

### report errors right away if they are found

if [ "$compile_errors" == "1" ]; then
  email_compile_errors
fi

#----------------------------- Stage 3 run verification case     --------------------------------------

RUN_CASES_beg=`GET_TIME`

#*** stage 3 - run release cases
if [ "$pid_fds_mpi" != "" ]; then
  wait $pid_fds_mpi
  echo "release fds built"
fi
if [ "$FDSRELEASE" == "" ]; then
  check_compile_fds_mpi     $FDS_DIR           $FDS_EXE
fi
if [[ $stage_fds_success || "$FDSRELEASE" != "" ]]; then
  run_verification_cases_release
  RUN_CASES=1
fi

if [ "$RUN_CASES" != "" ]; then
  wait_verification_cases_end stage3_run_debug 3a $JOBPREFIXD
  wait_verification_cases_end stage3_run_release 3b $JOBPREFIXR
  if [ -e $smvrepo/Verification/scripts/RESTART2_Cases.sh ]; then
    cd $smvrepo/Verification/scripts
    ./RESTART2_Cases.sh $JOBPREFIXR
    wait_verification_cases_end stage3_ver_restart 3c $JOBPREFIXR
  fi
fi

if [[ $stage_fdsdb_success || "$FDSDEBUG" != "" ]]; then
   check_verification_cases_debug
fi
if [[ $stage_fds_success || "$FDSRELEASE" != "" ]]; then
  check_verification_cases_release
fi

RUN_CASES_end=`GET_TIME`
DIFF_RUN_CASES=`GET_DURATION $RUN_CASES_beg $RUN_CASES_end`
echo "Run cases: $DIFF_RUN_CASES" >> $STAGE_STATUS

#----------------------------- Stage 4 generate images and movies     --------------------------------------

### Stage 4 generate images

wait $pid_smvapps
check_compile_smvapps

build_man_pics=1
MAKEPICTURES_beg=`GET_TIME`
if [[ "$build_man_pics" == "1" ]] ; then
  make_smv_pictures
  check_smv_pictures
fi
MAKEPICTURES_end=`GET_TIME`
DIFF_MAKEPICTURES=`GET_DURATION $MAKEPICTURES_beg $MAKEPICTURES_end`
echo "Make pictures: $DIFF_MAKEPICTURES" >> $STAGE_STATUS

if [ "$MAKEMOVIES" == "1" ]; then
  MAKEMOVIES_beg=`GET_TIME`

### Stage 4 generate movies

  make_smv_movies
  check_smv_movies

  MAKEMOVIES_end=`GET_TIME`
  DIFF_MAKEMOVIES=`GET_DURATION $MAKEMOVIES_beg $MAKEMOVIES_end`
  echo "Make movies: $DIFF_MAKEMOVIES" >> $STAGE_STATUS
fi

if [[ "$build_man_pics" == "1" ]] ; then
  generate_timing_stats
fi

#*** stage 5 - build manuals

if [[ "$build_man_pics" == "1" ]] ; then
   MAKEGUIDES_beg=`GET_TIME`
   echo Making guides

   echo "   user"
   make_guide SMV_User_Guide                $smvrepo/Manuals/SMV_User_Guide                SMV_User_Guide &
   pid_smv_ug=$!

   echo "   technical"
   make_guide SMV_Technical_Reference_Guide $smvrepo/Manuals/SMV_Technical_Reference_Guide SMV_Technical_Reference_Guide &
   pid_smv_tg=$!

   echo "   verification"
   make_guide SMV_Verification_Guide        $smvrepo/Manuals/SMV_Verification_Guide        SMV_Verification_Guide
   pid_smv_vg=$!

   DATE=`date +"%b %d, %Y - %r"`

   sed "s/&&DATE&&/$DATE/g"                $SMV_SUMMARY_DIR/templates/movies_template.html  | \
   sed "s/&&FDS_BUILD&&/$FDS_REVISION/g"                                          | \
   sed "s/&&SMV_BUILD&&/$SMV_REVISION/g" > $SMV_SUMMARY_DIR/movies.html


# copy images to be compared to summary directory
   cp $smvrepo/Manuals/SMV_User_Guide/SCRIPT_FIGURES/*.png                $SMV_SUMMARY_DIR/images/user/.
   cp $smvrepo/Manuals/SMV_Technical_Reference_Guide/SCRIPT_FIGURES/*.png $SMV_SUMMARY_DIR/images/user/.
   cp $smvrepo/Manuals/SMV_Verification_Guide/SCRIPT_FIGURES/*.png        $SMV_SUMMARY_DIR/images/verification/.
   cd $smokebotdir
   ./remove_images.sh $SMV_SUMMARY_DIR/images

   cd $smokebotdir
   ./compare_keywords.sh >& $OUTPUT_DIR/keyword_compare.log
   if [ ! -e $KEYWORDS_NODOC_LOG ]; then
     echo undocumented script keywords: 0  > $KEYWORDS_NODOC_LOG
   fi
   if [ ! -e $KEYWORDS_NOSOURCE_LOG ]; then
     echo unimplemented script keywords: 0 > $KEYWORDS_NOSOURCE_LOG
   fi

# compare images generated by this smokebot run with a base set in the fig repo
   cd $smokebotdir
   echo Comparing images
   $smokebotdir/compare_images.sh -t 0.2 >& $OUTPUT_DIR/stage5_compare_images
   rm -f $SMV_SUMMARY_DIR/images/*.png

   wait $pid_ug
   wait $pid_tg
   wait $pid_vg
   MAKEGUIDES_end=`GET_TIME`
   DIFF_MAKEGUIDES=`GET_DURATION $MAKEGUIDES_beg $MAKEGUIDES_end`
   echo "Make guides/compare images: $DIFF_MAKEGUIDES" >> $STAGE_STATUS

   UPDATED_WEB_IMAGES=1

# look for fyis
   if [[ `grep '***fyi:' $OUTPUT_DIR/stage5_compare_images` == "" ]]; then
     # Continue along
       :
   else
     echo "FYIs from Stage 5 - Image comparisons:"     >> $FYI_LOG
     grep '***fyi:' $OUTPUT_DIR/stage5_compare_images   >> $FYI_LOG
   fi

# look for warnings
   if [[ `grep '***warning:' $OUTPUT_DIR/stage5_compare_images` == "" ]]; then
     # Continue along
       :
   else
     echo "Warnings from Stage 5 - Image comparisons:"     >> $WARNING_LOG
     grep '***warning:' $OUTPUT_DIR/stage5_compare_images   >> $WARNING_LOG
   fi

   if [ "$WEB_DIR" != "" ]; then
     WEB_DIR_OLD=${WEB_DIR}_old
     rm -rf $WEB_ROOT/$WEB_DIR_OLD
     if [ -d $WEB_ROOT/$WEB_DIR ]; then
       mv $WEB_ROOT/$WEB_DIR $WEB_ROOT/$WEB_DIR_OLD
     fi
     mkdir -p $WEB_ROOT/$WEB_DIR
     cp -r $SMV_SUMMARY_DIR/* $WEB_ROOT/$WEB_DIR/.
     rm -f $WEB_ROOT/$WEB_DIR/*template.html
   fi

   notfound=`$HTML2PDF -V 2>&1 | tail -1 | grep "not found" | wc -l`
   if [ $notfound -eq 0 ]; then
     if [ -e  $smvrepo/Manuals/SMV_Summary/diffs.html ]; then
       $HTML2PDF --enable-local-file-access $smvrepo/Manuals/SMV_Summary/diffs.html $smvrepo/Manuals/SMV_Summary/SMV_Diffs.pdf
       cp $smvrepo/Manuals/SMV_Summary/SMV_Diffs.pdf $NEWGUIDE_DIR/.
     fi
   fi
else
   echo Errors found, not building guides
fi

SCRIPT_TIME_end=`GET_TIME`
DIFF_SCRIPT_TIME=`GET_DURATION $SCRIPT_TIME_beg $SCRIPT_TIME_end`
echo "Total time: $DIFF_SCRIPT_TIME" >> $STAGE_STATUS

### Report results ###
echo Reporting results
set_files_world_readable || exit 1
save_build_status

save_manuals_dir
if [[ "$build_man_pics" == "1" ]] ; then
  archive_timing_stats
fi
if [ "$HAVEMAIL" != "" ]; then
  echo "   emailing results"
fi
email_build_status
