#!/bin/bash
curdir=`pwd`
rundir=$curdir/..
fds=../../../fds/Build/mpi_intel_linux_64ib/fds_mpi_intel_linux_64ib
#for host in blaze001 blaze002 blaze003 blaze004 blaze005 blaze006 blaze007 blaze008 blaze009 blaze010 blaze011 blaze012 blaze013 blaze014 blaze015 blaze016 blaze017 blaze019 blaze020 blaze021 blaze022 blaze023 blaze024 blaze025 blaze026 blaze027 blaze028 blaze029 blaze030 blaze031 blaze032 
for host in blaze028 blaze029 blaze030 blaze031 blaze032 
do
sleep 10
echo $host
cd $rundir/$host
qfds.sh -e $fds -l $host thouse5.fds
done
