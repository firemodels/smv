# Utilities/Scripts

This directory contains scripts used by other scripts in the smv repo. The script qsmv.sh is described below.

## qsmv.sh

The script qsmv.sh is used to run smokeview on a Linux cluster with a PBS or Slurm queuing system in order to generate images for creating an animation. qsmv.sh can also be used to run arbitrary commands using the -C option.  qsmv.sh speeds up the rendering of smokeview images by running multiple instances of qsmv.sh. Each instance renders a subset of the simulation time frames. By running multiple instances of qsmv.sh, the total time required to render simulation images is reduced.

To get started, add the following line to your startup file, typically .bashrc 
(change the first part of the following alias command to match where your repo is located):

```alias qsmv.sh="/home/gforney/FireModels_fork/smv/Utilities/Scripts/qsmv.sh"```

To use qsmv.sh type:

```qsmv.sh casename```

(the .smv file extension is not required). This runs smokeview on the case `casename.smv` using the smokeview script `casename.ssf` . To run with a different script say `casename2.ssf` type:
```qsmv.sh -c casename2.ssf casename```

Typically, a smokeview script contains keywords such as RENDER or RENDERALL for generating images.  The keyword RENDER generates one image at one point in time.  The keyword RENDERALL generates images for all time steps in the simulation.  When RENDERALL is used, multiple instances of smokeview may be run by using -p n where n is the number of instances of smokeview where each instance generates a subset of the total images. For example if casename.smv has 1000 time steps, the command

```qsmv.sh -p 5 casename```

will run 5 instances of smokeview with each instance generating 200 images.

qsmv.sh uses either a smokeview that was built in the repo containing qsmv.sh or a smokeview found in your path.  To build smokeview perform the following steps:

1. cd smv/Build/LIBS/intel_linux_64
2. type: 
 ./make_LIBS.sh
3. cd smv/Build/smokeview/intel_linux_64 
4. type:
 ./make_smokeview.sh
 
More details on building smokeview may be found [here.](https://github.com/firemodels/smv/tree/master/Build/README.md)  To use the installed smokeview, use the `-i` option.

Details on using qsmv.sh are found below.

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
 -b     - bin directory
 -c     - smokeview script file [default: casename.ssf]
 -C com - execute the command com
 -d dir - specify directory where the case is found [default: .]
 -i     - use installed smokeview
 -j p   - job prefix
 -r     - redirect output
 -s     - first frame rendered [default: 1]
 -S     - interval between frames [default: 1]
```
