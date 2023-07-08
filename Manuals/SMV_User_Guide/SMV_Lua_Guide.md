# Smokeview - Lua Scripting Guide

## Overview

There are two scripting engines within Smokeview that allow it to be
programmatically controlled. This allows for analysis such as rendering of
visualisations and post-processing to be performed automatically and
reproducibly. This guide does not cover the functionality of Smokeview itself,
only how to access that functionalilty programmatically, as well as any
additional features provided by the Lua engine.

The Lua engine is designed to post-process all data produced by FDS (and also
CFAST) and is not limited to what is visually represented by Smokeview.

As all configuration options are exposed via the Lua API, it is also possible to
use Lua as a configuration file format.

## Getting Started

There are three important concepts/objects in Lua scripts:

- **Instance:** The instance of smokeview.
- **View:** The rendering/viewing item. Usually this is a window,
  but could be any framebuffer device. This controls how data is displayed. If
  no data is being displayed this might not exist.
- **Case:** The simulation data itself. This holds all of the
  information about the model and provides access to the data.

The Lua scripting within smokeview is structured in this way to allow for
flexibility and forwards compatibility. Currently, each of the three items above
can only be instantiated all at once (and is controlled mostly from the way
smokeview is started). This is shown below using the `load_default`
function.

First we load the smokeview Lua library, then the three items above.This loads
the single smokeview instance that is executing this script, the current window,
and the case that has already been loaded.

```lua
    -- Load the smokeview library.
    local smv = require("smv")
    -- Load a case into smokeview. load_default loads the case specified on the
    -- commandline. instance is not likely to be needed much in these contexts.
    local instance, view, case = smv.load_default()
```

## Controlling the Window and View

Window specific control is done via the `window` table. To set the size
of the window use the `window.size(width,height)` command.

The rest of the view control is done via the `view` table. This
doesn't control any of the content of the model or data being viewed, but
modifies the annotation content of the window, such as colour bars, labels,
notes, etc.

### Time Frames

The currently viewed frame of whatever data is loaded is governed via the view
table. The frame can be chosen by setting the `view.frame` value to a specified
frame index (starting from 0). If a certain time is desired the `view.time` can
be set. Note that this will find the frame closest to that time and set
`view.frame` to that. Information on frames can be found via the
`case.global_times` array object, which is an array of all the time values for
each frame. For instance, you can loop through each frame using the following
`for..in` loop:

```lua
for i,time in ipairs(case.global_times) do
    view.frame = i
    view.render("Render at - %.0fs", view.time)
end
```

The render line can be ignored for, as it will be introduced later, except that
it also uses `view.time`, which is the time value of the
current frame.

### Colourbar

The colourbar is controlled using the `view.colorbar` table. The colorbar table
includes the following options:

`flip`: A boolean value to determine if the colorbar is flipped. Can be set or
read.

```lua
-- Set the colorbar as flipped.
view.colorbar.flip = true
if view.colobar.flip
    then print("The colorbar is flipped.")
    else print("The colorbar is not flipped.")
end
```

`index`: An integer value which determines the value of the contour in the
colourbar. When set to nil there is no contour. The integer value is from 0-255.
Can be set or read. If a particular contour value (with respect to the units
shown) is required, the `bounds` information can be used to calculate an
appropriate index value.

```lua
-- Set the contour to the middle of the colorbar.
view.colorbar.index = 255
-- Render an image.
view.render()
-- Remove the contour from the colorbar.
view.colorbar.index = nil
```

`show`: A boolean value which determines if the colorbar is shown. Can be set or
read.

`colors`: A table of colours used in the colorbar. Can be set or read. WARNING:
liable to signifcant changes.

### Clipping

The clipping properties are quite commonly required when rendering different
views of data. These are all accesssed directly using the `view.clipping`
table. The options in this table consist entirely of setting and unsetting the
clipping values for the three axes and selecting a clipping mode.

Setting the clipping type is done via the `view.clipping.mode` value, which
is an integer of 4 different values:

- 0: No clipping.
- 1: Clip blockages and data.
- 2: Clip blockages.
- 3: Clip data.

Setting the clipping dimensions can be done a number of ways. The recommended
way is to treat the clipping object as a value, keeping in mind that
`nil` means that clipping for that value is turned off.

```lua
-- Turn off all clipping values.
view.clipping = nil
-- Set the maximum z clipping plane to 2.0 m.
view.clipping.z.max = 2
-- Set the minimum x clipping plane to -5 m
view.clipping.x.min = -5
-- Turn off clipping on the z axis
view.clipping.z = nil
```

### Camera Control

The camera is set and get using `view.camera.set(camera)` and
`view.camera.get` respectively. This is done using camera objects (tables)
which contain all of the camera information.

A camera table is as follows:

```lua
local topDown = {
    rotationType = 0,
    eyePos = { x = 0.502333, y = -2.405097, z = 0.396825},
    zoom =  1.0,
    viewAngle = 0,
    directionAngle = 0,
    elevationAngle = 0,
    projectionType = 1,
    viewDir  = {x =  0.500000, y = 0.192460, z = 0.396825},
    zAngle = {az = 0.000000, elev = 90.000000},
    transformMatrix = nil,
    clipping = nil
}
```

This camera can now be used to set the view in viewport via:

```lua
view.camera.set(topDown)
```

Note that the various cameras are not managed internally by the Smokeview camera
mechanism when using this technique, and are just stored as Lua variables
containing the data necessary to set up the cameras.

### Colour

The `view.color` table includes a number of options governing colour
and lighting in the view space. The options are as follows:

- `ambientlight(r,g,b)`: Set the ambient light value.
- `backgroundcolor(r,g,b)`: Set the background colour.
- `blockcolor(r,g,b)`: Set the block colour.
- `blockshininess(v)`: Set the blockshininess value.
- `blockspecular(r,g,b)`: Set the block specular colour.
- `boundcolor(r,g,b)`: Set the bound colour.
- `diffuselight(r,g,b)`: Set the diffuse light colour.

### Miscellaneous View Options

There are a large number of view options in Smokeview, most of which are
included in the `view` table. All view options that can be controlled via the
menus or `.ini` config files are available. Here a few of the more common
options:

## Data Loading

%We should consider have a data table instead of a load table. This would make
%more sense when listing loaded data and unloading. In order to view anything
useful in Smokeview data (such as slice files) must be loaded. This is typically
done view teh `load` table, which contains various functions for loading and
unloading data. The only exception is the unloading function, which can be
called as `case.unload.all()`. This is simply a synonym for
`case.load.unload.all()`.

Loading can be a surprisingly complex topic in Smokeview, and for this reason
the Lua engine exposes the data loading API down to the lowest practical level,
which is the loading of a single data file. All data files (with the exception
of CSV files) in Smokeview are per-mesh. This means that, for example, when
loading slice files you will likely want to load multiple files across different
meshes in a simulation. As there is nothing strictly linking various slice files
together there are multiple logical ways to load groups of slice files.

To allow for maximum flexibility, the lower level API only exposes two
functions: `case.load.datafile(filename)` and `case.load.datafile(filename)`
(for non-vector and vector files respectively). All other loading functions are
simply convinience functions to help users determine which files should be
loaded. For example: `case.load.namedslice(name)` finds all instances of a slice
that were given a certain name and loads them using `case.load.datafile`. If you
included a slice in your FDS code of:

```fortran
&SLCF ID='tempx' QUANTITY='TEMPERATURE' PBX=1.3 /
```

Then you could load this value across all meshes using the script:

```lua
load.namedslice("tempx")
```

This is particularly useful when loading slices that span meshes of different
resolutions. For example if the x cell width of some of the simulations was
0.2~m, it might be that some of the slice is at x = 1.3~m and some of the slice
is at x = 1.4~m, in which case the id of the slice is the only thing linking the
various data files.

In other cases the `case.load.slice(matchFunc)` function is useful, where matchFunc
is a function which takes the slice information as an argument and returns a
boolean which determines whether to load each data file or not.

3D Smoke files (including HRRPUV) can be loaded using `case.load3dsmoke(type)`, where
`type` is something like `SOOT MASS FRACTION` or `HRRPUV`.

## Rendering and Output

The basic rendering command is the `view.render()` function, which renders the
current frame. Without an argument this simply uses the default Smokeview
rendering style, rendering into the specified render directory. If the argument
is a string, then that string is used as the base for the filename (the
extension is added). If the argument is a function, then that function is called
at render time and is expected to return the string that will be used as the
filename.

```lua
-- Render as if 'r' was pressed in Smokeview.
view.render()
-- Render to '$RENDERDIR/testimage.png'
view.render('testimage')
-- Render to to a file with the time appended.
view.render("test image at %.2f s", view.time)
```

The directory to which Smokeview renders images can also be retrieved or
modified via the `render.dir` variable.

```lua
-- Print the current render directory to stdout.
print(view.render.dir)
-- Change the render directory to "images".
view.render.dir = "images"
```

## Accessing Information

Most of the information available to Smokeview is exposed in read-only
variables.

The CHID of the currently loaded simulation is stored in `chid`.

Information on the meshes is stored in the `meshinfo` table. Indices in this
table match the MESH indices (i.e. from 1). For example, if we want to look at
the what meshes we have in our mesh we could use the following script:

```lua
    print(string.format("CHID: %s", case.chid))
    print(string.format("----------------------------------------------------------------"))
    print(string.format("#\t|\tMeshId.\t|\t# Cells\t|\tI-J-K"))
    print(string.format("----------------------------------------------------------------"))
    local totalCells = 0
    for k,mesh in ipairs(case.meshes) do
        local nCells = mesh.ibar * mesh.jbar * mesh.kbar
        totalCells = totalCells + nCells
        print(string.format("%d\t|\t%s\t|\t%d\t|\t%d-%d-%d", k, mesh.label, nCells,
                            mesh.ibar, mesh.jbar, mesh.kbar))
    end
    print(string.format("----------------------------------------------------------------"))
    print(string.format("Total\t|\t    \t|\t%d\t|", totalCells))
    print(string.format("----------------------------------------------------------------"))
```

An example result of this script might be:

```
    CHID: ExampleModel
    ----------------------------------------------------------------
    #       |       MeshId. |       # Cells |       I-J-K
    ----------------------------------------------------------------
    1       |       Mesh02  |       275808  |       102-52-52
    2       |       Mesh03  |       102752  |       38-52-52
    3       |       Mesh04  |       65520   |       63-40-26
    4       |       Mesh05  |       70434   |       63-43-26
    5       |       Mesh06  |       75166   |       49-59-26
    6       |       Mesh07  |       308672  |       106-56-52
    7       |       Mesh08  |       110656  |       38-56-52
    ----------------------------------------------------------------
    Total   |               |       1009008 |
    ----------------------------------------------------------------
```

Information on slices is stored in the `case.slices` table, and the
operation is similar.

## Accessing and Post-Processing Data

All of the functionality thus far has covered manipulating Smokeview to produce
various visualisations as you would do manually. This has a lot of benefit in
automating repetive tasks and provided reproducibility, but only covers what is
already possible via Smokeview manually. There is a lot of other work that is
done using the results of FDS simulations that is done outside of Smokeview, but
as our Lua engine has access to all of the data produced by FDS and seen by
Smokeview, it makes sense to utilise it for other post-processing tasks as well.

A very simple but common task is to produce a plot showing the HRR of a
particular simulation over time. This kind of plot can be easily obtained from
Excel or similar data processing tools, but as we are scripting Smokeview to
handle the results of our simulations already, let's do the same for the HRR
plot. This could be handled as follows:

```lua
require("plot")
local hrrDV = case.csvs['hrr'].vectors['HRR']
plot.DV(plotDir, hrrDV, "HRR")
```

This code accesses the CHID_hrr.csv file (the `case.csvs['hrr']` part) and takes
the 'HRR' vector from that file (the `.vectors['HRR']` part), and then plots it
using the funciton plotDV. The CSV API assumes that the first vector of every
CSV file is the 'x' vector, which for FDS is usualy Time.

The plotDV funciton is not currently included by default with Smokeview in order
to not push a particular plotting library, but as the Lua engine executes a full
programming language, any plotting software can be called, with the Lua engine
providing the relevant data.

As full programming engine all kinds of other post processing can occur.
