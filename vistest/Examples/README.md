# Fire and Smoke Demonstration GUI

This directory contains a python GUI script and associated files for demonstrating fire and smoke behavior using FDS and Smokeview.  This GUI works on Windows, Macintosh and Linux computers.  The programs fds and smokeview need to be in the path.  The GUI uses the tkinter module for the interface.  To use the GUI, type:

```DEMOfds.py```

on a Windows computer and

```python DEMOfds.py```

on a Linux or Macintosh computer. 

![DEMOfds](https://github.com/firemodels/smv/assets/12403014/9eb5173a-6be1-4b20-ae18-5c0e9083ea14)

The GUI gives options for changing fire size, door size and the gravity vector.  Two scenarios can be setup and compared. The `Make input` button creates the input files.  The `Run` button runs the case using FDS and the `View` button visualizes the case using Smokeview.

### TO DO

* input error checking - make sure inputs are numbers

