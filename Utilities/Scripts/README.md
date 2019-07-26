# Utilities/Scripts

This directory contains scripts used by other scripts in the smv repo.

## qsmv.sh

The script qsmv.sh is used to run smokeview in the background on a Linux cluster with a PBS or Slurm 
queuing system.  To use qsmv.sh type

```qsmv.sh casename```

on a command line.  qsmv.sh will run smokeview on the case `casename.smv` and smokeview script `casename.ssf` . 
If the smokeview script `casename.ssf` contains the `RENDERALL` keyword (to render all frames ) you may use the -s and -S
parameters to cause qsmv.sh to render a portion of the total frames.

To get started, add the following line to your startup file, typically .bashrc (change the first part to match where your repo is located):

```alias qsmv.sh="/home/gforney/FireModels_fork/smv/Utilities/Scripts/qsmv.sh"```

Usage info:

```Usage: qsmv.sh [-e smv_command] [-q queue] casename

runs smokeview on the case casename.smv using the script casename.ssf

options:
 -e exe - full path of smokeview used to run case
    [default: /home/gforney/FireModels_fork/smv/Build/smokeview/intel_linux_64/s                                                    mokeview_intel_linux_64]
 -h   - show commonly used options
 -H   - show all options
 -q q - name of queue. [default: batch]
 -v   - output generated script
Other options:
 -c     - smokeview script file [default: casename.ssf]
 -d dir - specify directory where the case is found [default: .]
 -i     - use installed smokeview
 -s     - first frame rendered [default: 1]
 -S     - interval between frames [default: 1]


```
