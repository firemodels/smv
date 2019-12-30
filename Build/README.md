## Building Smokeview and Utilities

The directories under smv/Build contain scripts for building smokeview and utilities such as smokediff and smokezip.  
Most of these scripts assume you have an Intel C/C++ compiler.  Smokeview, smokediff and smokezip also
assume you have an Intel Fortran Compiler.  Linux and OSX build directories also have scripts for building
using gcc and gfortran gnu compilers. 

### Preliminaries

  * Install the GIT repository for FDS and Smokeview using the [GIT getting started wiki](https://github.com/firemodels/fds/wiki/Git-Notes-Getting-Started).  

  * On a Windows PC, install the [GNU make utility - http://gnuwin32.sourceforge.net/packages/make.htm](http://gnuwin32.sourceforge.net/packages/make.htm).  The gnu make installer does not add an entry to the PATH variable.  When you run the installer, note where the installer places the make utility executable, then add this directory path to your PATH variable. 

### Building Libraries Needed by Smokeview

  * Smokeview requires several libraries such as glut, png and glui used for implementing dialog boxes, rendering images or interacting with the OS.  To build these libraries using Intel compilers where platform is linux, osx or win:
      * cd to `smv/Build/LIBS/intel_platform_64` 
      * type make_LIBS.sh on a Linux or OSX system and make_LIBS.bat on a Windows system

### Building Smokeview 

  * There is a sub-directory under `smv/Build/smokeview` for each target in the Smokeview Makefile.  To build Smokeview on a Linux or OSX system, cd into the `intel_linux_64` or `intel_osx_64` directory and type

   ```./make_smokeview.sh```

On a Windows system cd into the `intel_win_64` directory and type

   ```.\make_smokeview.bat ```

### Building Smokeview Utilities

Similarly to build a smokeview utiltity such as smokezip or smokediff using the Intel compilers cd into `smv/Build/prog/intel_platform_64` and type
`./make_prog.sh` on a linux or osx system and `make_prog` on a windows sytem where `prog` is the name of the utility (`smokediff` or `smokezip` for example) and `platform` is `linux`, `osx` or `win` .
