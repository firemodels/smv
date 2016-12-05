#!/bin/bash
curdir=`pwd`
rundir=$curdir/..
for host in blaze001 blaze002 blaze003 blaze004 blaze005 blaze006 blaze007 blaze008 blaze009 blaze010 blaze011 blaze012 blaze013 blaze014 blaze015 blaze016 blaze017 blaze019 blaze020 blaze021 blaze022 blaze023 blaze024 blaze025 blaze026 blaze027 blaze028 blaze029 blaze030 blaze031 blaze032 
do
echo $host
mkdir $rundir/$host
done
