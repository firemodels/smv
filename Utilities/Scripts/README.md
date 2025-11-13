# Utilities/Scripts

This directory contains scripts for generating images and animations of FDS cases.  It also contains utility scripts used by other scripts in this repo, scripts for setting up the graphics environment for the image generating scripts and for identifying Git and compiler versions when building smoke iew. 

## slice2html.sh

This script is used to generate an HTML page from an smv file. To use it, add an alias to your startup file, typically .bashrc (change the ... in the first part of the path to match your repo location):

```alias slice2html.sh=".../smv/Utilities/Scripts/slice2html.sh"```

Then type `slice2html casename` in a command shell.

## slice2mp4.sh

This script is used to generate an MP4 animation file from a slice file.  It is run on a Linux system with a queing system. To use:

1.  add the alias

```alias slice2mp4=".../smv/Utilities/Scripts/slice2mp4.sh"```

to your startup file, typically .bashrc .  Change the "..." in the first part to match your repo location.

2.  Either use smokeview you built at smv/Build/smokeview/intel_linux or use slice2mp4 with the -i option to use the smokeview installed on  your system.

3.  cd to a directory containing your case and type `slice2mp4 casename` .  You will see a list of slice files for this case such as
```
index   quantity                      dir       position
    1   U-VELOCITY                      3     255.249954
    2   V-VELOCITY                      3     255.249954
    3   VELOCITY                        3     255.249954
    4   W-VELOCITY                      3     255.249954
    5   BURNING RATE(terrain)           3           0.01
    6   LEVEL SET VALUE(terrain)        3            1.0
    7   U-VELOCITY(terrain)             3           25.0
    8   V-VELOCITY(terrain)             3           25.0
    9   VELOCITY(terrain)               3           25.0
   ```

After selecting a slice, index 9 in this example, you will see a menu for selecting various options such as scene viewpoints,  
how images are generated (number of processes, what queue to use) and an option to generate the animation.
`slice2mp4` creates a bash script and a smokeview script used by to generate the mp4 animation.

```
          slice: TEMPERATURE/Y=1.6
         bounds: default
      color bar: show
       time bar: show
      font size: small
      viewpoint: VIEWYMIN

        PNG dir: .
        mp4 dir: /var/www/html/gforney
      smokeview: /home/gforney/FireModels_fork/smv/Build/smokeview/intel_linux/smokeview_linux
      processes: 32, node sharing on
          queue: batch
          email: gforney@gmail.com

s - select slice
b - set bounds
C - hide color bar
T - hide time bar
F - toggle font size
v - set viewpoint

r - set PNG dir
a - set mp4 dir
m - set email address

p - set number of processes
S - toggle node sharing
q - set queue

1 - create MP4 animation
x - exit
```

Select the queue, number of processes and an email address to send the animation.  Then to generate an animation, select option 2.

## qsmv.sh

This script is used to run smokeview on a Linux cluster with a PBS or Slurm queuing system in order to generate images for creating an animation. qsmv.sh speeds up the rendering prrocess by running multiple instances of smokeview. Each instance renders a subset of the simulation time frames. By running multiple instances, the total time required to render simulation image frames is reduced.

To get started, add the following alias to your startup file, typically .bashrc 
(change the ... in the first part of the path to match where your repo is located):

```alias qsmv.sh=".../smv/Utilities/Scripts/qsmv.sh"```

To use qsmv.sh type:

```qsmv.sh casename```

(the .smv file extension is not required). This runs smokeview on the case `casename.smv` using the smokeview script `casename.ssf` . To run with a different script say `casename2.ssf` type:
```qsmv.sh -c casename2.ssf casename```

Typically, a smokeview script contains keywords such as RENDER or RENDERALL for generating images.  The keyword RENDER generates one image at one point in time.  The keyword RENDERALL generates images for all time steps in the simulation.  When RENDERALL is used, multiple instances of smokeview may be run by using -p n where n is the number of instances of smokeview where each instance generates a subset of the total images. For example if casename.smv has 1000 time steps, the command

```qsmv.sh -p 5 casename```

will run 5 instances of smokeview with each instance generating 200 images.

qsmv.sh uses either a smokeview that was built in the repo containing qsmv.sh or a smokeview found in your path.  To build smokeview perform the following steps:

1. cd smv/Build/LIBS/intel_linux
2. type: 
 ./make_LIBS.sh
3. cd smv/Build/smokeview/intel_linux 
4. type:
 ./make_smokeview.sh
 
More details on building smokeview may be found [here.](https://github.com/firemodels/smv/tree/master/Build/README.md)  To use the installed smokeview, use the `-i` option.

Details on using qsmv.sh are found below.

```
Usage: qsmv.sh [-e smv_command] [-q queue] casename

runs smokeview on the case casename.smv using the script casename.ssf

options:
 -e exe - full path of smokeview used to run case
    [default: /home/gforney/FireModels_fork/smv/Build/smokeview/intel_linux/smokeview_intel_linux]
 -h   - show commonly used options
 -H   - show all options
 -P n - run n instances of smokeview each instance rendering 1/n'th of the total images
        only use this option if you have a RENDERALL keyword in your .ssf smokeview script
 -q q - name of queue. [default: batch]
 -v   - output generated script (do not run)
Other options:
 -b     - bin directory
 -c     - smokeview script file [default: casename.ssf]
 -C com - execute the command com
 -d dir - specify directory where the case is found [default: .]
 -e exe - execute the program exe
 -i     - use installed smokeview
 -j p   - job prefix
 -N n   - reserve n cores [default: 8]
 -r     - redirect output
 -s     - first frame rendered [default: 1]
 -S     - interval between frames [default: 1]
 -T     - share nodes
```
