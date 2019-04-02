# dem2fds usage

Notes on creating FDS input files from digital elevation map (dem) files using the utility dem2fds.

### Preliminaries

* Download and install Smokeview from <https://pages.nist.gov/fds-smv/downloads.html>.  This download provides the utility dem2fds used for creating the FDS input files.

* Download and install Imagemagick from <https://imagemagick.org/script/download.php>.  This program is used to convert terrain image files from jpeg 2000 to jpeg format.

* Go to the USGS National Map website at <https://viewer.nationalmap.gov/basic> 
to obtain elevation and terrain image data files.
These files are used by dem2fds to create FDS input files.
This website has a map of the United States on the right and a set of data
download options is on the left. Zoom in on the region
of interest then download elevation and terrain image files using the steps described below.

![USGS dem webpage](https://github.com/firemodels/fds/wiki/images/usgs_dem_webpage.png)

   Obtaining elevation data:

   1. Select the _Elevation Products (3DEP)_ checkbox.
   2. Select 1/3 arc-second DEM checkbox
   3. Select the _GridFloat_ File format option
   4. Click on the map and zoom in to a region of interest.
   5. Click on the _Find Products_ button.  You should see one or more
entries listed with headings beginning with
USGS NED 1/3 arc-second ...... 1 x 1 degree .
   6. Click on the _Footprint_ link to verify where various files are located.
   7. Click on _Download_ link to download files of interest.
   8. Unzip these files using a program such as Winzip. Open the directory created with the unzip operation and
copy the .flt and .hdr files to a directory where you'll keep your elevation and image files.

   Obtaining terrain images:

   1. Click on the _Return to Search_ button
and uncheck (if checked) the _Elevation Products (3DEP)_ checkbox
   2. Select the _Imagery - 1 meter (NAIP)_ checkbox
   3. Click on the map and zoom in to
a region of interest within the elevation region selected for for elevation files.
   4. Click on the _Find Products_ button.  You should see one or more
entries with each entry beginning with **m_** and ending with **.jp2** .
   5. Click on the _Footprint- link to verify image file locations.
   6. Click on _Download_ link to download files of interest. The image files should
cover the region you are using to define an FDS input file.
   7. Copy the downloaded files to the same directory you used to copy the .flt and .hdr elevation files.
   8. The .jp2 files you just downloaded are in _jpeg 2000_ format.
They need to be converted to jpeg format .
To do this on a PC, run the `jp2conv` command.
On Linux or a Mac, run the `jp2conv.sh` command.
These commands are installed with the latest version of Smokeview.

Put the downloaded elevation and terrain files in a directory.  This directory may then be referenced by
dem2fds using the -dir command line keyword when creating a FDS input file.

### Creating an FDS input file from dem data

* Create a dem2fds input file named `casename.in` containing the following

```
GRID
 ibar+1 jbar+1 kbar
LONGLATMINMAX
 longmin longmax latmin latmax
```

The integer parameters `ibar`, `jbar` and `kbar` are used 
to define the `IJK` keyword of the FDS input file's `&MESH` namelist.   
In dem2fds, these parameters correspond to the number of longitude and latitude divisions
and the number of divisions along the z direction respectively.  The decimal parameters
`longmin`, `longmax`, `latmin` and `latmax`
are the minimum and maximum
longitude and latitude values enclosing the region being modeled.

Two other methods for specifying a terrain region are to use the
`LONGLATCENTER` or `LONGLATORIG` keywords as in

```
GRID
  ibar+1 jbar+1 kbar dx dy
LONGLATCENTER
  longcen  latcen
```

or

```
GRID
  ibar+1 jbar+1 kbar dx dy
LONGLATORIG
  longorig  latorig
```
where `dx` and `dy` are the scenario width and length in meters
( `ibar`, `jbar` and `kbar` are defined as before),
`longcen` and `latcen` are the longitude and
latitude at the scenario center,
`i.e.` at position (`dx/2`, `dy/2`)
and `longorig` and `latorig` are the longitude and
latitude at the scenario origin,
`i.e.` at position (`0.0, 0.0`).

* Finally, create an FDS input file, by running the command:

```
 dem2fds -dir elevation_directory casename.in
```
where `elevation_directory` is the path of the directory containing the
elevation and image files downloaded previously and
casename.in is the file containing `GRID` and `LATLONG...` keywords.

* To view the resulting case, run the commands:

```
fds casename.fds
smokeview casename
```

### Overview of dem2fds Input Keywords

* GRID - Specifying the simulation grid

```
GRID
 ibar+1 jbar+1 kbar dx dy zmin zmax
```

ibar, jbar, bar - number of divisions along the x, y and z directions
dx, dy - distance along the x and y axis.

Note, dx and dy are required if LATLONGCENTER or
LATLONGORIG are used to specify a terrain region and
optional if LATLONGMINMAX is used to specify a terrain region (dem2fds computes
dx and dy for LATLONGMINMAX keywords).

zmin, zmax - minimum and maximum z elevation.  These parameters are optional.


* LONGLATCENTER - specifying the longitude and latitude at the scenario center

```
LONGLATCENTER
 longcen latcen
```

longcen, latcen - longitude and latitude at the center of the scenario, at
position (dx/2,dy/2) .

* LONGLATORIG - specifying the longitude and latitude at the scenario origin

```
LONGLATORIG
 longorig latorig
```

longorig, latorig - longitude and latitude at the origin of the scenario, at
position (0,0) .

* LONGLATMINMAX - specifying longitudes and latitudes that bound the scenario

```
LONGLATMINMAX
 longmin longmax latmin latmax
```

longmin, longmax - minimum and maximum longitude
latmin, latmax - minimum and maximum latitude

### dem2fds Command Line Options

```

dem2fds (SMV6.5.3-57-g038cc8b) Jul 28 2017
Create an FDS input file using elevation and image
  data obtained from http://viewer.nationalmap.gov 

Usage:
  dem2fds [options] casename.in
  -dir dir  - directory containing map and elevation files
  -elevdir dir - directory containing elevation files (if different than -dir directory)
  -geom     - represent terrain using using &GEOM keywords (experimental)
  -obst     - represent terrain using &OBST keywords 
  -help      - display help summary
  -help_all  - display all help info
  -version   - display version information
  -elevs    - output elevations, do not create an FDS input file
  -matl matl_id - specify a MATL ID for use with the -geom option 
  -overlap - assume that there is a 300 pixel overlap between maps.
  -show     - highlight image and fds scenario boundaries
  -surf surf_id - specify surf ID for use with OBSTs or geometry 
```

* The `-obst` option is used by default.
* The `-geom` option is experimental testing the new geometry file format.
* All terrain images should contain a 300 pixel overlap buffer.  However
this is not always the case.  The `-nobuffer` option is used for those
cases when there is no overlap between two adjacent images.

### Examples

Data files used for the Devils Tower examples may be
[downloaded from here.](https://drive.google.com/drive/folders/0B-W-dkXwdHWNSnk0RE9aMDhhekE?usp=sharing)
These files were generated using the steps outlined previously.  The .jp2 files do not need to be downloaded 
unless you want to experiment converting image files from jpeg 2000 to jpeg format.

1. Devils Tower I

[Devils tower](https://en.wikipedia.org/wiki/Devils_Tower) is located in north eastern Wyoming.  
This region was chosen as a
test case because it is easy to verify (or not) that the geometry and the overlayed image generated by dem2fds
is consistent at the tower location.

* dem2fds input file:
```
GRID
 101 101 30
LONGLATMINMAX
 -104.7256 -104.7096 44.5850 44.5945
```

The `IJK` portion of the `&MESH` line in the FDS input file is

```
&MESH IJK=100,100,30, XB=...
```

* create an fds input file using:

`dem2fds -overlap -dir terrain\tower tower.in`

where terrain\tower is a directory containing elevation data and terrain images for Devils Tower.

* The image below was created by running FDS on the input file generated using dem2fds then viewing with smokeview
using:
```
fds tower.fds
smokeview tower.fds
```
![devils tower](https://github.com/firemodels/fds/wiki/images/tower1.png)

2. Devils Tower II

This example is similar to the first.  It uses the -show command line option of dem2fds 
to add file names to the terrain images and to show a red rectangle for the region being simulated.
This is useful for ensuring that the correct elevation and image files are being used and if not which 
ones should be downloaded.

* dem2fds input file:

The longitude/latitude region is larger than in the first example.  It corresponds to the region
of the data files that were downloaded rather than the region being modeled.

```
GRID
 101 101 30
LONGLATMINMAX
 -104.7256 -104.7096 44.5850 44.5945
```

* create an fds input file using:

Again, the -show option is used to hilight the region being simulated and identify the files that were downloaded.

`dem2fds -overlap -show -dir terrain\tower tower.in`

where terrain\tower is a directory containing elevation data and terrain images for Devils Tower.

* create the following image using the commands:

```
fds tower.fds
smokeview tower.fds
```

![devils tower](https://github.com/firemodels/fds/wiki/images/tower2.png)


