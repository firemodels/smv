# Smokeview Build Direcrory

The above directories contain scripts for building smokeview and utility programs such as smokediff and smokezip.  
Most of these scripts assume you have an Intel C/C++ compiler installed.  Smokeview, smokediff and smokezip also
assume you have an Intel Fortran Compiler.  Linux and OSX build directories also have scripts for building
these applications using gnu compilers (gcc and gfortran). Smokeview, Smokediff and Smokezip require various libaries.  To build these
libraries cd to the `Build/LIBS/<platform>` directory and type:
```
./make_LIBS.sh
```


