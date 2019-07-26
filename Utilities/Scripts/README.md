# Utilities/Scripts

This directory contains scripts used by other scripts in the smv repo.

## qsmv.sh

The script qsmv.sh is used to run smokeview on a Linux cluster with a PBS or Slurm queuing system to generate images for creating an animation. qsmv.sh speeds up this rendering by parallelizing in time not space. (it is not practical to parallelize in space by rendering a portion of the scene on multiple nodes since it would be difficult if not impossible to combine these image subsets to form one image).   Each instance of qsmv.sh generates a subset of the time frames in a simulation. By running multiple instances of qsmv.sh, the total time required to render simulation images is reduced.

To get started, add the following line to your startup file, typically .bashrc 
(change the first part to match where your repo is located):

```alias qsmv.sh="/home/gforney/FireModels_fork/smv/Utilities/Scripts/qsmv.sh"```

To use qsmv.sh type:

```qsmv.sh casename```

(the .smv file extension is not required). This runs smokeview on the case `casename.smv` using the smokeview script `casename.ssf` . To run with a different script say `casename2.ssf` type:
```qsmv.sh -c casename2.ssf casename```

Typically, a smokeview script contains keywords such as RENDER or RENDERALL for generating images.  When RENDERALL is used, multiple instances of smokeview may run by using -p n where n is the number of instances.  Each instance creates 1/n'th of the total number time frames, reducing the time required to generate all the images in a case.

qsmv.sh uses either a smokeview that was built in the repo containing qsmv.sh or a smokeview found in your path.  To build smokeview perform the following steps:

1. cd smv/Build/LIBS/intel_linux_64
2. type: 
 ./make_LIBS.sh
3. cd smv/Build/smokeview/intel_linux_64 
4. type:
 ./make_smokeview.sh
 
More details on building smokeview may be found [here.](https://github.com/firemodels/smv/tree/master/Build/README.md)  To use the installed smokeview, use the `-i` option.

More usage information follows.

```
Usage: qsmv.sh [-e smv_command] [-q queue] casename

runs smokeview on the case casename.smv using the script casename.ssf

options:
 -e exe - full path of smokeview used to run case
    [default: /home/gforney/FireModels_fork/smv/Build/smokeview/intel_linux_64/smokeview_intel_linux_64]
 -h   - show commonly used options
 -H   - show all options
 -p n - run n instances of smokeview each instance rendering 1/n'th of the total images
        only use this option if you have a RENDERALL keyword in your .ssf smokeview script
 -q q - name of queue. [default: batch]
 -v   - output generated script
Other options:
 -c     - smokeview script file [default: casename.ssf]
 -d dir - specify directory where the case is found [default: .]
 -i     - use installed smokeview
 -s     - first frame rendered [default: 1]
 -S     - interval between frames [default: 1]
```
