# Utilities/Scripts

This directory contains scripts for generating images and movies of FDS cases and utility routines used by other scripts in this repo. 

## fds2html.sh

This script is used to generate an HTML page from smokeview (.smv) file and associated slice and boundary files.
To use type `fds2html.sh casename` .

## fds2mov.sh

This script is used to generate a MP4 movie file from an FDS slice file.  Add the following line to your startup file, typically .bashrc 
(change the first part of the following alias command to match where your repo is located):

```alias fds2mov="/home/gforney/FireModels_fork/smv/Utilities/Scripts/fds2mov.sh"```

Type `fds2mov casename` to get a list of available slice files. After selecting a slice file the script give you the option to generate PNG images for each slice file time step, to generate a MP4 movie file (along with PNG files) or a BASH script for generating images which can be customized later.

## qsmv.sh

This script is used to run smokeview on a Linux cluster with a PBS or Slurm queuing system in order to generate images for creating an animation. qsmv.sh speeds up the rendering prrocess by running multiple instances of smokeview. Each instance renders a subset of the simulation time frames. By running multiple instances, the total time required to render simulation image frames is reduced.

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
