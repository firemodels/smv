# Utilities/Scripts

This directory contains scripts for generating images and animations of FDS cases.  It also contains scripts for
peforming tasks such setting up the graphics environment for the image generating scripts and scripts for identifying Git and compiler 
versions for the compilation sccripts. These notes are preliminary, a work in progress.

## fds2html.sh

This script is used to generate an HTML page from a casename.smv, slice and boundary files.
To use, add an alias to your startup file, typically .bashrc (change the first part to match your repo location):

```alias fds2html.sh="/home/gforney/FireModels_fork/smv/Utilities/Scripts/fds2html.sh"```

Then type `fds2html casename` at a command line. (add more description)

## fds2mov.sh

This script is used to generate an MP4 animation file from an FDS slice file.  To use, add an alias to your startup file, 
typically .bashrc (change the first part to match your repo location):

```alias fds2mov.sh="/home/gforney/FireModels_fork/smv/Utilities/Scripts/fds2mov.sh"```

Then type `fds2mov casename` .  You will see a list of slice files for this case such as
```
  1   VELOCITY                     3     95.0
  2   U-VELOCITY                   3     95.0
  3   V-VELOCITY                   3     95.0
  4   W-VELOCITY                   3     95.0
  5   VELOCITY(terrain)            3     40.0
  6   U-VELOCITY(terrain)          3     40.0
  7   V-VELOCITY(terrain)          3     40.0
  8   W-VELOCITY(terrain)          3     40.0
  9   LEVEL SET VALUE(terrain)     3      1.0
Select slice file: 
   ```

After selecting an option such as `9`, LEVEL SET in this caes, you will see options for 
setting the number of procceses, defining the queue, generating images or generating images and an animation.  
Also, the script for generating images may be customized and run later.

```
     quantity:  LEVEL SET VALUE(terrain)
    processes: 1
        queue: batch
    smokeview: /...../smv/Build/smokeview/intel_linux_64/smokeview_linux_64
      qsmv.sh: /...../smv/Utilities/Scripts/qsmv.sh
 image script: casename_slice_9.sh

p - define number of processes
q - define queue
1 - generate PNG images
2 - generate PNG images and an MP4 animation
x - exit
option:
```

To generate an animation then, select option 2.

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
