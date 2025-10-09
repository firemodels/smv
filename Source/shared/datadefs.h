#ifndef DATADEFS_H_DEFINED
#define DATADEFS_H_DEFINED

#define ALL_FRAMES       -1

#define SLICE_UNKNOWN     -1
#define SLICE_NODE_CENTER 1
#define SLICE_CELL_CENTER 2
#define SLICE_TERRAIN     4
#define SLICE_GEOM        6

#define CSV_UNDEFINED 0
#define CSV_DEFINED   1
#define CSV_DEFINING  2

#define XXX 0
#define YYY 1
#define ZZZ 2

#define IMIN 0
#define IMAX 1
#define JMIN 2
#define JMAX 3
#define KMIN 4
#define KMAX 5

#define WALL_1 0
#define WALL_3 1
#define WALL_6 2

// (front wall = 1, right wall = 2, back wall = 3, left wall = 4)

#define FRONT_WALL  1
#define RIGHT_WALL  2
#define BACK_WALL   3
#define LEFT_WALL   4
#define BOTTOM_WALL 5
#define TOP_WALL    6

#define TEMP_IGNITION_MAX 100000.
#define SURFACE_TEMPMIN  -100000.
#define SURFACE_TEMPMAX   100000.

#define BLOCK_regular 0
#define BLOCK_texture 1
#define BLOCK_outline 2
#define BLOCK_hidden -2

#define DIR_UNDEFINED -1
#define DOWN_Y 0
#define UP_X   1
#define UP_Y   2
#define DOWN_X 3
#define DOWN_Z 4
#define UP_Z   5

#define MAX_ISO_COLORS 10

#define NOT_FDSBLOCK 0
#define FDSBLOCK     1

#define UNCOMPRESSED_ALLFRAMES 0
#define COMPRESSED_ALLFRAMES   1

#define COMPRESSED_UNKNOWN -1
#define UNCOMPRESSED        2
#define COMPRESSED_RLE      0 // 3d smoke file format assumes rle parameter is 0
#define COMPRESSED_ZLIB     1 // 3d smoke file format assumes zlib parameter is 1

#define CFACE_NORMALS_NO  0
#define CFACE_NORMALS_YES 1

#define PATCH_STRUCTURED_NODE_CENTER 0
#define PATCH_STRUCTURED_CELL_CENTER 1
#define PATCH_GEOMETRY_BOUNDARY      2
#define PATCH_GEOMETRY_SLICE         3

#define C_GENERATED       0
#define FORTRAN_GENERATED 1

#define ONLY_IN_GAS           0
#define GAS_AND_SOLID         1
#define ONLY_IN_SOLID         2
#define NEITHER_GAS_NOR_SOLID 3

#define XLEFT   -1
#define XRIGHT   1
#define YFRONT  -2
#define YBACK    2
#define ZBOTTOM -3
#define ZTOP     3

#define VENT_SOLID   0
#define VENT_OUTLINE 2
#define VENT_HIDDEN -2

#define HFLOW_VENT 0
#define VFLOW_VENT 1
#define MFLOW_VENT 2

#define TERRAIN_SURFACE    0
#define TERRAIN_IMAGE      1
#define TERRAIN_HIDDEN     2
#define TERRAIN_TOP        3

#define TEXTURE_SPHERICAL   0
#define TEXTURE_RECTANGULAR 1

#define ZONEVENT_CIRCLE 1
#define ZONEVENT_SQUARE 2

#define INTERIORwall 0
#define FRONTwall    1
#define BACKwall     2
#define LEFTwall     3
#define RIGHTwall    4
#define UPwall       5
#define DOWNwall     6

#define PART_POINTS     1
#define PART_SPHERES    2
#define PART_LINES      3
#define PART_SMV_DEVICE 4

#define MAXPLOT3DVARS 6
#define NRGB         12

#define LOAD        0
#define UNLOAD      1
#define RESETBOUNDS 2
#define RELOAD      3
#define UPDATE_HIST 4
#define BOUNDS_ONLY 5

#ifndef TOGGLE_ROLLOUT
#define TOGGLE_ROLLOUT(procinfo,nprocinfo,proc,proc_id,dialog_id) \
  InsertRollout(proc,dialog_id); \
  procinfo[nprocinfo].rollout = proc; \
  procinfo[nprocinfo].rollout_id = proc_id; \
  procinfo[nprocinfo].dialog = dialog_id; \
  (nprocinfo)++
#endif

#define ONEORZERO(val) if(val!=0)val=1

#define K2C(T) ((T)-273.15)
#define C2K(T) ((T)+273.15)

#define USEMESH_DRAW -1
#define IF_NOT_USEMESH_RETURN0(loaded,blocknum)\
  if(loaded==1)return 0;\
  if((blocknum)>=0 && global_scase.meshescoll.meshinfo[(blocknum)].use == 0){\
    return 0;\
   }
#define IF_NOT_USEMESH_CONTINUE(loaded,blocknum)\
  if(loaded==1)continue;\
  if((blocknum)>=0 && global_scase.meshescoll.meshinfo[(blocknum)].use == 0){\
    continue;\
   }

#define SCALE2FDS(x) ((x)*xyzmaxdiff)
#define SCALE2SMV(x) ((x)/xyzmaxdiff)

#define SCALE2FDSL(x) ((x)*xyzmaxdiff_local)

#define NO      0
#define YES     1

#define PLANEDIST(norm,xyz0,xyz) ((xyz[0]-xyz0[0])*norm[0]+(xyz[1]-xyz0[1])*norm[1]+(xyz[2]-xyz0[2])*norm[2])

#define REL_VAL(val, valmin, valmax) ((float)((val)-(valmin))/(float)((valmax)-(valmin)))
#define SHIFT_VAL(val, valmin, valmax, shift_val) ((valmin) + ((valmax)-(valmin))*pow(REL_VAL((val),(valmin),(valmax)),(shift_val)))

#define FDS2SMV_X(x) (((x)-global_scase.xbar0)/xyzmaxdiff)
#define FDS2SMV_Y(y) (((y)-global_scase.ybar0)/xyzmaxdiff)
#define FDS2SMV_Z(z) (((z)-global_scase.zbar0)/xyzmaxdiff)

#define SMV2FDS_X(x) (global_scase.xbar0+(x)*xyzmaxdiff)
#define SMV2FDS_Y(y) (global_scase.ybar0+(y)*xyzmaxdiff)
#define SMV2FDS_Z(z) (global_scase.zbar0+(z)*xyzmaxdiff)

#define DONOT_SET_MINMAX_FLAG 0
#define SET_MINMAX_FLAG       1

#define VERT_AVG2(v1,v2,vavg) \
  vavg[0]=(v1[0]+v2[0])/2.0;\
  vavg[1]=(v1[1]+v2[1])/2.0;\
  vavg[2]=(v1[2]+v2[2])/2.0

#define VERT_AVG3(v1,v2,v3,vavg) \
  vavg[0]=(v1[0]+v2[0]+v3[0])/3.0;\
  vavg[1]=(v1[1]+v2[1]+v3[1])/3.0;\
  vavg[2]=(v1[2]+v2[2]+v3[2])/3.0

#define DIST3(v1,v2,dist2) \
  dx=v1[0]-v2[0];\
  dy=v1[1]-v2[1];\
  dz=v1[2]-v2[2];\
  dist2=dx*dx+dy*dy+dz*dz

#define DDIST3(v1,v2,dist2) \
  dx=v1[0]-v2[0];\
  dy=v1[1]-v2[1];\
  dz=v1[2]-v2[2];\
  dist2=sqrt(dx*dx+dy*dy+dz*dz)

#define SMV2FDS_XYZ(XYZ_OUT,XYZ_IN)\
(XYZ_OUT)[0] = SMV2FDS_X((XYZ_IN)[0]);\
(XYZ_OUT)[1] = SMV2FDS_Y((XYZ_IN)[1]);\
(XYZ_OUT)[2] = SMV2FDS_Z((XYZ_IN)[2])

#define FDS2SMV_XYZ(XYZ_OUT,XYZ_IN)\
(XYZ_OUT)[0] = FDS2SMV_X((XYZ_IN)[0]);\
(XYZ_OUT)[1] = FDS2SMV_Y((XYZ_IN)[1]);\
(XYZ_OUT)[2] = FDS2SMV_Z((XYZ_IN)[2])

#define INCIRCLE(x,y,z,incirc) \
{\
  float ddx, ddy, ddz;\
  ddx = SMV2FDS_X(x)-cvi->origin[0];\
  ddy = SMV2FDS_Y(y)-cvi->origin[1];\
  ddz = SMV2FDS_Z(z)-cvi->origin[2];\
  incirc=( ddx*ddx + ddy*ddy + ddz*ddz <= cvi->radius*cvi->radius ? 1 : 0 );\
}

#define VEC2MA(vec,a)\
      (vec)[0] *= (a);\
      (vec)[1] *= (a)

#define VEC3MA(vec,a)\
  (vec)[0] *= (a);\
  (vec)[1] *= (a);\
  (vec)[2] *= (a)

#ifndef DOT2
#define DOT2(x,y) ((x)[0]*(y)[0]+(x)[1]*(y)[1])
#endif

#ifndef ROTATE
#define ROTATE(xto,xfrom,az)\
  (xto)[0] = (xfrom)[0]*cos((az)) - (xfrom)[1]*sin((az));\
  (xto)[1] = (xfrom)[0]*sin((az)) + (xfrom)[1]*cos((az))
#endif

//   i    j    k
// x[0] x[1] x[2]
// y[0] y[1] y[2]

#ifndef CROSS
#define CROSS(xy,x,y) \
  (xy)[0] = (x)[1]*(y)[2] - (y)[1]*(x)[2];\
  (xy)[1] = (x)[2]*(y)[0] - (y)[2]*(x)[0];\
  (xy)[2] = (x)[0]*(y)[1] - (y)[0]*(x)[1]
#endif

#ifndef VEC3EQ
#define VEC3EQ(y,x)\
  (y)[0]=(x)[0];\
  (y)[1]=(x)[1];\
  (y)[2]=(x)[2]
#endif

#ifndef VEC3EQCONS
#define VEC3EQCONS(y,x)\
  (y)[0]=(x);\
  (y)[1]=(x);\
  (y)[2]=(x)
#endif

#ifndef VEC3DIFF
#define VEC3DIFF(ymx,y,x)\
  (ymx)[0]=(y)[0]-(x)[0];\
  (ymx)[1]=(y)[1]-(x)[1];\
  (ymx)[2]=(y)[2]-(x)[2]
#endif

#ifndef DOT3
#define DOT3(x,y) ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#endif

#ifndef DOT3SKIP
#define DOT3SKIP(x,ix,y,iy) ((x)[0]*(y)[0]+(x)[ix]*(y)[iy]+(x)[2*ix]*(y)[2*iy])
#endif

#ifndef DOT4SKIP
#define DOT4SKIP(x,ix,y,iy) ((x)[0]*(y)[0]+(x)[ix]*(y)[iy]+(x)[2*ix]*(y)[2*iy]+(x)[3*ix]*(y)[3*iy])
#endif

#ifndef MAXDIFF2
#define MAXDIFF2(x,y) MAX(ABS(x[0]-y[0]),ABS(x[1]-y[1]))
#endif

#ifndef MAXDIFF3
#define MAXDIFF3(x,y) MAX(  MAXDIFF2(x,y),ABS(x[2]-y[2])  )
#endif

#ifndef NORM3
#define NORM3(x) sqrt((x)[0]*(x)[0]+(x)[1]*(x)[1]+(x)[2]*(x)[2])
#endif

#ifndef NORMALIZE3
#define NORMALIZE3(x)\
  {\
  float denom;\
  denom=NORM3(x);\
  if(denom!=0.0){\
  (x)[0]/=denom;\
  (x)[1]/=denom;\
  (x)[2]/=denom;\
  }\
  }
#endif

#ifndef NORM2
#define NORM2(x) sqrt((x)[0]*(x)[0]+(x)[1]*(x)[1])
#endif

#ifndef DEG2RAD
#define DEG2RAD (3.14159265359f/180.0)
#endif

#ifndef RAD2DEG
#define RAD2DEG (180.0/3.14159265359f)
#endif

#ifndef MAX
#define MAX(a,b)  ((a)>(b) ? (a) : (b))
#endif

#ifndef MAXABS3
#define MAXABS3(x) (MAX(ABS((x)[0]),MAX(ABS((x)[1]),ABS((x)[2]))))
#endif

#ifndef MIN
#define MIN(a,b)  ((a)<(b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a) ((a)>=0 ? (a) : (-(a)))
#endif

#ifndef SIGN
#define SIGN(x) ((x)>=0 ?  1  :  -1)
#endif

#ifndef MIX
#define MIX(f,a,b) ( (f)*(a) + (1.0-(f))*(b))
#endif

#ifndef MIX2
#define MIX2(i,n,a,b) ( ((float)(i)/(float)(n))*(a) + ( 1.0-((float)(i)/(float)(n)) )*(b))
#endif

#ifndef CLAMP
#define CLAMP(x,lo,hi)  MIN(MAX((x),(lo)),(hi))
#endif

#ifndef GETINDEX
#define GETINDEX(xval,xmin,dx,nx) CLAMP(((xval)-(xmin))/(dx),0,(nx)-1)
#endif

#ifndef IJCIRC
#define IJCIRC(i,j) ((i)+1+((j)+1)*nx)
#endif

#ifndef IJKNODE
#define IJKNODE(i,j,k) ((i)+(j)*nx+(k)*nxy)
#endif

#ifndef F_IJKNODE
#define F_IJKNODE(i,j,k) ((i)*nyz+(j)*nz+(k))
#endif

#ifndef IJKN
#define IJKN(i,j,k,n) (IJKNODE(i,j,k)+(n)*nxyz)
#endif

#ifndef IJKCELL
#define IJKCELL(i,j,k) ((i)+ (j)*ibar+(k)*ibar*jbar)
#endif

#ifndef IJCELL2
#define IJCELL2(i,j) (nxcell*(j) + (i))
#endif

#ifndef IJ
#define IJ(i,j) ((i)+(j)*nx)
#endif

#ifndef IJ2
#define IJ2(i,j) ((nycell+1)*(i) + (j))
#endif

#define FOPEN_2DIR(file, mode) fopen_2dir_scratch(file, mode)

#define PI           3.14159265359f

#define TOBW(col) ( 0.299*(col)[0] + 0.587*(col)[1] + 0.114*(col)[2])

#endif
