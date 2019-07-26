# Utilities/Scripts

This directory contains scripts used by other scripts in the smv repo.

## qsmv.sh

The script qsmv.sh is used to run smokeview on a Linux cluster with a PBS or Slurm queuing system to generate images for creating an animation. qsmv.sh speeds up this rendering by parallelizing in time not space. (it is not practical to parallelize in space by rendering a portion of the scene on multiple nodes since it would be difficult if not impossible to combine these image subsets one image).   Each instance of qsmv.sh generates a subset of the time frames in a simulation. By running multiple instances of qsmv.sh, the total time required to render simulation images is reduced.

To get started, add the following line to your startup file, typically .bashrc 
(change the first part to match where your repo is located):

```alias qsmv.sh="/home/gforney/FireModels_fork/smv/Utilities/Scripts/qsmv.sh"```

To use qsmv.sh type

```qsmv.sh casename```

(the .smv file extension is not required).  
This runs smokeview on the case `casename.smv` using the smokeview script `casename.ssf` . 
Type `qstat -a` to see this job in the queue and `qstat -n` to see which node it is running on.

Typically, a smokeview script contains keywords for generating images.  
Multiple instances of qsmv.sh may be run, where each instance uses the `-s` and `-S` keywords
to specify the first frame and frame offset respectivly,
reducing the time required to generate all the images in a case.

More detailed usage information follows.

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
