#/bin/bash
reporoot=$HOME/FireModels_fork
smvbin=$reporoot/smv/Build/smokeview/intel_linux
bindir=$reporoot/smv/Build/for_bundle
$smvbin/smokeview_win -bindir $bindir $*
