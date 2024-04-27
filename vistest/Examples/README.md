# Fire and Smoke Demonstration GUI

This directory contains a python GUI script and associated files for demonstrating fire and smoke behavior using FDS and Smokeview.  One can demonstrate the effect of fire size, ventilation and the gravity vector on fire and smoke flow.  This GUI works on Windows, Macintosh and Linux computers.  The programs FDS and Smokeview need to be in the path.  The GUI uses the python tkinter module for the interface.  To use the GUI, type:

```DEMOfds.py```

in a command shell on a Windows computer.  You can also double click on this file.  Type

```python DEMOfds.py```

on a Linux or Macintosh computer. 

![DEMOfds](https://github.com/firemodels/smv/assets/12403014/9eb5173a-6be1-4b20-ae18-5c0e9083ea14)

The GUI gives options for changing fire size, door size and the gravity vector.  Two scenarios can be setup and compared. The `Make input` buttons create the input files.  The `Run` buttons run the cases using FDS and the `View` buttons visualize the cases using Smokeview.

### TO DO

* input error checking - make sure inputs are numbers
* add a note on how to install tkinter if it is not loaded

