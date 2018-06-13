# Building Smokeview and Utilities

The above directories contain scripts for building smokeview and utility programs such as smokediff and smokezip.  
Most of these scripts assume you have an Intel C/C++ compiler installed.  Smokeview, smokediff and smokezip also
assume you have an Intel Fortran Compiler.  Linux and OSX build directories also have scripts for building
these applications using gnu compilers (gcc and gfortran). 

### Preliminaries

  * Install the GITrepository for FDS and Smokeview using the [GIT getting started wiki](https://github.com/firemodels/fds/wiki/Git-Notes-Getting-Started).  

The following steps assume that `~/FDS-SMV` is the repository root where `~` is your home directory.

  * On a Windows PC, install the [GNU make utility - http://gnuwin32.sourceforge.net/packages/make.htm](http://gnuwin32.sourceforge.net/packages/make.htm).

  * The gnu make installer does not add an entry to the PATH variable.  When you run the installer, note where the installer places the make utility executable, then add this directory path to your PATH variable. 

  * On a Linux or OSX system, define the shell variable, `IFORT_COMPILER`, in your startup files to point to the location of your Intel compiler distribution.  For example if using the bash shell, add the following line to your .bashrc startup file (substituting the path to your compiler for the path given below)
{{{
export IFORT_COMPILER=/opt/intel/composerxe
}}}

  * Smokeview requires several software libraries (such as Glut, glui etc) to implement features such as dialog boxes, rendering images or interacting with the underlying OS.  To build these libraries, 
      * cd to `SMV/Build/LIBS/lib_platform_compiler_size` where platform is `win`, `osx` or `linux`; compiler is `intel` or `gcc` and size is `32` or `64`.  (for example cd to `SMV\Build\LIBS\lib_win_intel_64 to build 64 bit windows libraries for Smokeview)
      * type the makelibs.sh or makelibs.bat command that is found there.

  * There is a sub-directory under `SMV/Build/smokeview` for each target in the Smokeview Makefile.  To build Smokeview on a Linux system, cd into the `intel_linux_64` directory and type

   `./make_smv.sh`

On a Windows system cd into the `intel_win_64` directory and type

   `.\make_smv.bat `

To build Smokeview for other platforms, cd into one of the following directories and type the .bat or .sh (depending on platform) command that is found there.

  *  intel_linux_64
  *  intel_osx_64
  *  intel_win_64





