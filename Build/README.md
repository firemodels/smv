
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
      * cd to `smv/Build/LIBS/intel_platform4`
      * type make_LIBS.sh on a Linux or OSX system and make_LIBS.bat on a Windows system

### Building Smokeview

  * There is a sub-directory under `smv/Build/smokeview` for each target in the Smokeview Makefile.  To build Smokeview on a Linux or OSX system, cd into the `intel_linux` or `intel_osx` directory and type

   ```./make_smokeview.sh```

On a Windows system cd into the `intel_win` directory and type

   ```.\make_smokeview.bat ```

### Building Smokeview Utilities

Similarly to build a smokeview utiltity such as smokezip or smokediff using the Intel compilers cd into `smv/Build/prog/intel_platform` and type
`./make_prog.sh` on a linux or osx system and `make_prog` on a windows sytem where `prog` is the name of the utility (`smokediff` or `smokezip` for example) and `platform` is `linux`, `osx` or `win` .

## Building Smokeview and Utilities via CMake

Smokeview and its utilities also support building via
[CMake](https://cmake.org). Using CMake is necessary to support platform and
compiler combinations not listed above (e.g. MSVC, clang, and gcc on Windows and
clang generally). CMake can be run as normal, but some examples are included
here (all run from the root of the repository).

For example, to build Smokeview and all its utilities on Windows (via MSVC)
the following might be used (the directory `cbuild` is used to not conflict with
`Build` and `dist` is used as the directory for the final artifacts):

```sh
  cmake -B cbuild -DCMAKE_BUILD_TYPE=Release
  cmake --build cbuild --config Release
  cmake --install cbuild --config Release --prefix dist
```

Or, to build with Intel OneAPI:

```sh
  call "C:\Program Files (x86)\Intel\oneAPI\setvars.bat"
  cmake -B cbuild -G Ninja -DCMAKE_BUILD_TYPE=Release
  cmake --build cbuild --config Release
  cmake --install cbuild --config Release --prefix dist
```

Note that the scripts to build with Lua in the `Build/smokeview/${platform}`
will automatically use the CMake commands as above. These scripts will also run
all of the available tests.

### Enabling Lua Scripting Engine

As an alternative to the *.ssf style scripts, Smokeview can also be built to
execute Lua scripts. Lua can be enabled by setting `LUA=ON`. The MSVC examples
above can modfied as follows:

```sh
  cmake -B cbuild -DLUA=ON -DCMAKE_BUILD_TYPE=Release
  cmake --build cbuild --config Release
  cmake --install cbuild --config Release --prefix dist
```

#### Running Smokeview with Lua

When built with Lua, Smokeview includes some shared libraries and other code.
This is the reason for the install step. In the example above, the `dist`
directory contains all of the necessary executables, code, and shared libraries.
