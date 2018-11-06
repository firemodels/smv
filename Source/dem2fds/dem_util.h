#define FDS_OBST 0
#define FDS_GEOM 1
#define LEN_BUFFER 1024
#define EARTH_RADIUS 6371000.0
#define INTERP1D(f,v1,v2) ((1.0-(f))*(v1)+(f)*(v2))
#define DONT_INTERPOLATE 0
#define INTERPOLATE 1


#ifdef WIN32
#define STDCALLF extern void _stdcall
#else
#define STDCALLF extern void
#endif

//subroutine elev2geom(output_elev_file, xgrid, ibar, ygrid, jbar, vals, nvals)
#define FORTelev2geom _F(elev2geom)
STDCALLF FORTelev2geom(char *output_elev_file, float *xgrid, int *ibar, float *ygrid, int *jbar, float *vals, int *nvals, FILE_SIZE filelen);

/* --------------------------  elevdata ------------------------------------ */

typedef struct {
  int ncols, nrows, nz, use_it;
  float xllcorner, yllcorner, cellsize;
  char *headerfile, *datafile;
  char filelabel[13];
  float lat_min, lat_max, long_min, long_max, dlong, dlat;
  float lat_min_orig, lat_max_orig, long_min_orig, long_max_orig;
  float val_min, val_max;
  float xmax, ymax, zmin, zmax;
  float xref, yref, longref, latref;
  float *valbuffer;
  gdImagePtr image;
} elevdata;

/* --------------------------  excludedata ------------------------------------ */

typedef struct {
  float xmin, xmax, ymin, ymax;
} excludedata;

EXTERNCPP void GenerateFDSInputFile(char *casename, char *casename_fds, elevdata *fds_elevs, int option);
EXTERNCPP int GetElevations(char *elevfile, char *image_file, elevdata *fds_elevs);

SVEXTERN char image_dir[1024], elev_dir[1024];
#ifdef pp_CSVF
SVEXTERN char csv_file[1024];
#endif
SVEXTERN int SVDECL(overlap_size,0), SVDECL(show_maps,0);
SVEXTERN char surf_id1[1024], surf_id2[1024], matl_id[1024];
SVEXTERN int SVDECL(nexcludeinfo, 0);
SVEXTERN excludedata SVDECL(*excludeinfo, NULL);
SVEXTERN float SVDECL(buff_dist, 10.0);
SVEXTERN int   SVDECL(nmeshx,1), SVDECL(nmeshy,1);
SVEXTERN float   SVDECL(*xplt, NULL), SVDECL(*yplt, NULL);
