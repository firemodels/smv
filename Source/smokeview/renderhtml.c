#include "options.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

#include "smokeviewvars.h"

#ifdef pp_HTML

/* ------------------ GetPartFileNodes ------------------------ */

void GetPartFileNodes(int option, int option2, float *verts, float *colors, int *nverts, int *frame_sizes, int *nframes){
  int i, ntimes=0, first = 1;
  int ibeg, iend;
  partdata *parttime=NULL;
  int itime, ipart;

  if(option2==ALL_TIMES){
    for(i = 0; i<npartinfo; i++){
      partdata *parti;

      parti = partinfo+i;
      if(parti->loaded==0||parti->display==0)continue;
      parttime = parti;
      if(first==1){
        *nframes = parti->ntimes;
        first = 0;
      }
      else{
        *nframes = MIN(parti->ntimes, *nframes);
      }
    }
    ibeg = 0;
    iend = *nframes;
  }
  else{
    *nframes = 1;
    ibeg = parttime->itime;
    iend = parttime->itime+1;
  }
  if(option==0)return;
  for(itime = ibeg; itime<iend; itime++){

    frame_sizes[itime-ibeg] = 0;
    for(i = 0; i<npartinfo; i++){
      partdata *parti;
      int nclasses;
      part5data *datacopy;

      parti = partinfo+i;
      if(parti->loaded==0||parti->display==0)continue;
      nclasses = parti->nclasses;
      datacopy = parti->data5+nclasses*itime;

    }
  }

}

  /* ------------------ GetBndfFileNodes ------------------------ */

void GetBndfFileNodes(int option, int option2, int *offset, float *verts, unsigned char *textures, int *nverts, int *tris, int *ntris, int *frame_size, int *nframes){
  *ntris = 0;
  *nverts = 0;
}

  /* ------------------ GetSliceFileNodes ------------------------ */

void GetSliceFileNodes(int option, int option2, int *offset, float *verts, unsigned char *textures, int *nverts, int *tris, int *ntris, int *frame_size, int *nframes){
  int islice, nv = 0, nt = 0, count = 0;
  int ibeg, iend, itime, first=1, minsteps;
  slicedata *slicetime=NULL;

  for(islice = 0; islice<nsliceinfo; islice++){
    slicedata *slicei;

    slicei = sliceinfo+islice;
    if(slicei->loaded==0||slicei->display==0||slicei->slicefile_type!=SLICE_NODE_CENTER||slicei->volslice==1)continue;
    if(slicei->idir!=XDIR&&slicei->idir!=YDIR&&slicei->idir!=ZDIR)continue;
    slicetime = slicei;
    if(first==1){
      minsteps=slicei->ntimes;
      first = 0;
    }
    else{
      minsteps = MIN(minsteps, slicei->ntimes);
    }
    if(option2==CURRENT_TIME)break;
  }
  if(first==1){
    *frame_size=0;
    *nframes=0;
    *nverts = 0;
    *ntris = 0;
    return;
  }
  if(option2==ALL_TIMES){
    ibeg = 0;
    iend = minsteps;
    *nframes = iend;
  }
  else{
    ibeg = slicetime->itime;
    iend = slicetime->itime+1;
    *nframes = 1;
  }
  *frame_size = 0;
  for(itime = ibeg; itime<iend; itime++){

    for(islice = 0; islice<nsliceinfo; islice++){
      slicedata *slicei;
      int nrows, ncols;
      unsigned char *iq;

      slicei = sliceinfo+islice;

      if(slicei->loaded==0||slicei->display==0||slicei->slicefile_type!=SLICE_NODE_CENTER||slicei->volslice==1)continue;
      if(slicei->idir!=XDIR&&slicei->idir!=YDIR&&slicei->idir!=ZDIR)continue;

      // preliminary code for obtaining geometry vertices and triangles
      if(1==0){
        geomdata *geomi;
        geomlistdata *geomlisti;

        geomi = slicei->patchgeom->geominfo;
        geomlisti = geomi->geomlistinfo - 1;
        if(geomlisti->norms_defined==0){
        //  UpdatePatchGeomTriangles(slicei->patchgeom, geom_type);
        }
      }

      iq = slicei->slicelevel+itime*slicei->nsliceijk;
      switch(slicei->idir){
      case XDIR:
        ncols = slicei->nslicej;
        nrows = slicei->nslicek;
        break;
      case YDIR:
        ncols = slicei->nslicei;
        nrows = slicei->nslicek;
        break;
      case ZDIR:
        ncols = slicei->nslicei;
        nrows = slicei->nslicej;
        break;
      }
      if(nrows>1&&ncols>1){
        if(itime==ibeg){
          *frame_size += nrows*ncols;
          nv += nrows*ncols;
          nt += 2*(nrows-1)*(ncols-1);
        }
        if(option==1){
          meshdata *meshi;
          float *xplt, *yplt, *zplt;
          int plotx, ploty, plotz;
          float  constval;
          int n, i, j, k, nj, nk;
          int ii, jj, kk;

          meshi = meshinfo+slicei->blocknumber;

          xplt = meshi->xplt;
          yplt = meshi->yplt;
          zplt = meshi->zplt;
          plotx = slicei->is1;
          ploty = slicei->js1;
          plotz = slicei->ks1;

          switch(slicei->idir){
          case XDIR:
            if(itime==ibeg){
              // vertices
              constval = xplt[plotx];
              for(j = slicei->js1; j<=slicei->js2; j++){
                for(k = slicei->ks1; k<=slicei->ks2; k++){
                  *verts++ = constval;
                  *verts++ = yplt[j];
                  *verts++ = zplt[k];
                }
              }
              // triangle indices
              nk = slicei->ks2+1-slicei->ks1;
              for(j = slicei->js1; j<slicei->js2; j++){
                jj = j-slicei->js1;
                for(k = slicei->ks1; k<slicei->ks2; k++){
                  int i00, i01, i11, i10;

                  kk = k-slicei->ks1;
                  i00 = nk*(jj+0)+kk+0;
                  i01 = nk*(jj+0)+kk+1;
                  i10 = nk*(jj+1)+kk+0;
                  i11 = nk*(jj+1)+kk+1;

                  *tris++ = *offset+i00;
                  *tris++ = *offset+i10;
                  *tris++ = *offset+i11;

                  *tris++ = *offset+i00;
                  *tris++ = *offset+i11;
                  *tris++ = *offset+i01;
                }
              }
              *offset += nrows*ncols;
            }
            // textures
            for(j = slicei->js1; j<=slicei->js2; j++){
              n = (j-slicei->js1)*slicei->nslicei*slicei->nslicek-1;
              n += (plotx-slicei->is1)*slicei->nslicek;

              for(k = slicei->ks1; k<=slicei->ks2; k++){
                *textures++ = iq[++n];
              }
            }
            break;
          case YDIR:
            // vertices
            if(itime==ibeg){
              constval = yplt[ploty];
              for(i = slicei->is1; i<=slicei->is2; i++){
                for(k = slicei->ks1; k<=slicei->ks2; k++){
                  *verts++ = xplt[i];
                  *verts++ = constval;
                  *verts++ = zplt[k];
                }
              }
              // triangle indices
              nk = slicei->ks2+1-slicei->ks1;
              for(i = slicei->is1; i<slicei->is2; i++){
                ii = i-slicei->is1;
                for(k = slicei->ks1; k<slicei->ks2; k++){
                  int i00, i01, i11, i10;

                  kk = k-slicei->ks1;
                  i00 = nk*(ii+0)+kk+0;
                  i01 = nk*(ii+0)+kk+1;
                  i10 = nk*(ii+1)+kk+0;
                  i11 = nk*(ii+1)+kk+1;

                  *tris++ = *offset+i00;
                  *tris++ = *offset+i10;
                  *tris++ = *offset+i11;

                  *tris++ = *offset+i00;
                  *tris++ = *offset+i11;
                  *tris++ = *offset+i01;
                }
              }
              *offset += nrows*ncols;
            }
            // textures
            for(i = slicei->is1; i<=slicei->is2; i++){
              n = (i-slicei->is1)*slicei->nslicej*slicei->nslicek-1;
              n += (ploty-slicei->js1)*slicei->nslicek;

              for(k = slicei->ks1; k<=slicei->ks2; k++){
                *textures++ = iq[++n];
              }
            }
            break;
          case ZDIR:
            if(itime==ibeg){
              // vertices
              constval = zplt[plotz];
              for(i = slicei->is1; i<=slicei->is2; i++){
                for(j = slicei->js1; j<=slicei->js2; j++){
                  *verts++ = xplt[i];
                  *verts++ = yplt[j];
                  *verts++ = constval;
                }
              }
              // triangle indices
              nj = slicei->js2+1-slicei->js1;
              for(i = slicei->is1; i<slicei->is2; i++){
                ii = i-slicei->is1;
                for(j = slicei->js1; j<slicei->js2; j++){
                  int i00, i01, i11, i10;

                  jj = j-slicei->js1;
                  i00 = nj*(ii+0)+jj+0;
                  i01 = nj*(ii+0)+jj+1;
                  i10 = nj*(ii+1)+jj+0;
                  i11 = nj*(ii+1)+jj+1;

                  *tris++ = *offset+i00;
                  *tris++ = *offset+i10;
                  *tris++ = *offset+i11;

                  *tris++ = *offset+i00;
                  *tris++ = *offset+i11;
                  *tris++ = *offset+i01;
                }
              }
              *offset += nrows*ncols;
            }
            // textures
            for(i = slicei->is1; i<=slicei->is2; i++){
              n = (i-slicei->is1)*slicei->nslicej*slicei->nslicek-1;
              n += (plotz-slicei->ks1)*slicei->nslicey;

              for(j = slicei->js1; j<=slicei->js2; j++){
                *textures++ = iq[++n];
              }
            }
            break;
          }
        }
      }
    }
  }
  *nverts = nv;
  *ntris = nt;
}

/* ------------------ GetGeometryNodes ------------------------ */

void GetGeometryNodes(int option, int *offset, float *verts, float *norms, float *colors, int *nverts, int *tris, int *ntris){
  int i, nv = 0, nt = 0;

  for(i = 0; i<ngeominfoptrs; i++){
    geomdata *geomi;
    geomlistdata *geomlisti;

    geomi = geominfoptrs[i];

    // reject unwanted geometry

    if((geomi->fdsblock==NOT_FDSBLOCK && geomi->geomtype!=GEOM_ISO)||geomi->patchactive==1)continue;
    geomlisti = geomi->geomlistinfo-1;

    nv += geomlisti->nverts;
    nt += geomlisti->ntriangles;

    if(option==1){
      int j;
      float *xyz_in, xyz_out[3];
      float *norm_in;

      for(j = 0; j<geomlisti->nverts; j++){
        float col2[3] = {0.0, 0.0, 1.0};
        float *col;

        if(geomlisti->verts[j].ntriangles>0){
          col = geomlisti->verts[j].triangles[0]->geomsurf->color;
        }
        else{
          col = col2;
        }
        xyz_in = geomlisti->verts[j].xyz;
        norm_in = geomlisti->verts[j].vert_norm;
        NORMALIZE_XYZ(xyz_out, xyz_in);
        *verts++ = xyz_out[0];
        *verts++ = xyz_out[1];
        *verts++ = xyz_out[2];
        *norms++ = norm_in[0];
        *norms++ = norm_in[1];
        *norms++ = norm_in[2];
        *colors++ = col[0];
        *colors++ = col[1];
        *colors++ = col[2];
      }
      for(j = 0; j<geomlisti->ntriangles; j++){
        *tris++ = *offset+geomlisti->triangles[j].verts[0]-geomlisti->verts;
        *tris++ = *offset+geomlisti->triangles[j].verts[1]-geomlisti->verts;
        *tris++ = *offset+geomlisti->triangles[j].verts[2]-geomlisti->verts;
      }
      *offset += geomlisti->nverts;
    }
  }
  *nverts = nv;
  *ntris = nt;
}


/* ------------------ GetBlockNodes ------------------------ */

void GetBlockNodes(const meshdata *meshi, blockagedata *bc, float *xyz, float *norms, int *tris){
  /*

  23---------22
  /         /
  /         /
  20--------21
  z direction
  19 ------  18
  /         /
  /         /
  16 ------ 17


  15--------14
  /         /
  /         /
  12--------13
  x direction
  11------  10
  /         /
  /         /
  8 ------ 9

  7---------6
  /         /
  /         /
  4--------5
  y direction
  3 ------  2
  /         /
  /         /
  0 ------ 1
  */
  int n;
  float xminmax[2], yminmax[2], zminmax[2];
  float *xplt, *yplt, *zplt;
  int ii[8] = {0, 1, 1, 0, 0, 1, 1, 0};
  int jj[8] = {0, 0, 1, 1, 0, 0, 1, 1};
  int kk[8] = {0, 0, 0, 0, 1, 1, 1, 1};

  int inds[36] = {
    0, 1, 5, 0, 5, 4,
    2, 3, 7, 2, 7, 6,
    1, 2, 6, 1, 6, 5,
    3, 0, 4, 3, 4, 7,
    4, 5, 6, 4, 6, 7,
    0, 2, 1, 0, 3, 2
  };

  xplt = meshi->xplt;
  yplt = meshi->yplt;
  zplt = meshi->zplt;

  xminmax[0] = xplt[bc->ijk[IMIN]];
  xminmax[1] = xplt[bc->ijk[IMAX]];
  yminmax[0] = yplt[bc->ijk[JMIN]];
  yminmax[1] = yplt[bc->ijk[JMAX]];
  zminmax[0] = zplt[bc->ijk[KMIN]];
  zminmax[1] = zplt[bc->ijk[KMAX]];

  for(n = 0; n<8; n++){
    *xyz++ = xminmax[ii[n]];
    *xyz++ = yminmax[jj[n]];
    *xyz++ = zminmax[kk[n]];
  }
  for(n = 8; n<16; n++){
    *xyz++ = xminmax[ii[n-8]];
    *xyz++ = yminmax[jj[n-8]];
    *xyz++ = zminmax[kk[n-8]];
  }
  for(n = 16; n<24; n++){
    *xyz++ = xminmax[ii[n-16]];
    *xyz++ = yminmax[jj[n-16]];
    *xyz++ = zminmax[kk[n-16]];
  }
  for(n = 0; n<36; n++){
    int offset;

    offset = 0;
    if(n>=12)offset = 8;
    if(n>=24)offset = 16;
    *tris++ = offset+inds[n];
  }
  for(n = 0; n<72; n++){
    norms[n] = 0.0;
  }

  norms[1] = -1.0;
  norms[4] = -1.0;
  norms[7] = 1.0;
  norms[10] = 1.0;
  norms[13] = -1.0;
  norms[16] = -1.0;
  norms[19] = 1.0;
  norms[22] = 1.0;

  norms[24] = -1.0;
  norms[27] = 1.0;
  norms[30] = 1.0;
  norms[33] = -1.0;
  norms[36] = -1.0;
  norms[39] = 1.0;
  norms[42] = 1.0;
  norms[45] = -1.0;

  norms[50] = -1.0;
  norms[53] = -1.0;
  norms[56] = -1.0;
  norms[59] = -1.0;
  norms[62] = 1.0;
  norms[65] = 1.0;
  norms[68] = 1.0;
  norms[71] = 1.0;
}

/* ------------------ Lines2Geom ------------------------ */

void Lines2Geom(float **vertsptr, float **colorsptr, int *n_verts, int **linesptr, int *n_lines){
  int nverts = 0, nlines = 0, offset = 0;
  float *verts, *verts_save, *colors, *colors_save;
  int *lines, *lines_save;
  int i;

  nverts = 8*3;
  nlines = 12*2;

  if(nverts==0||nlines==0){
    *n_verts = 0;
    *n_lines = 0;
    *vertsptr = NULL;
    *colorsptr = NULL;
    *linesptr = NULL;
    return;
  }

  NewMemory((void **)&verts_save, nverts*sizeof(float));
  NewMemory((void **)&colors_save, nverts*sizeof(float));
  NewMemory((void **)&lines_save, nlines*sizeof(int));
  verts = verts_save;
  colors = colors_save;
  lines = lines_save;

  *verts++ = 0.0;
  *verts++ = 0.0;
  *verts++ = 0.0;

  *verts++ = xbar;
  *verts++ = 0.0;
  *verts++ = 0.0;

  *verts++ = xbar;
  *verts++ = ybar;
  *verts++ = 0.0;

  *verts++ = 0.0;
  *verts++ = ybar;
  *verts++ = 0.0;

  *verts++ = 0.0;
  *verts++ = 0.0;
  *verts++ = zbar;

  *verts++ = xbar;
  *verts++ = 0.0;
  *verts++ = zbar;

  *verts++ = xbar;
  *verts++ = ybar;
  *verts++ = zbar;

  *verts++ = 0.0;
  *verts++ = ybar;
  *verts++ = zbar;

  for(i = 0; i<24; i++){
    *colors++ = 0.0;
    verts_save[i] = verts_save[i];
  }

  *lines++ = 0;
  *lines++ = 4;
  *lines++ = 1;
  *lines++ = 5;
  *lines++ = 2;
  *lines++ = 6;
  *lines++ = 3;
  *lines++ = 7;

  *lines++ = 0;
  *lines++ = 1;
  *lines++ = 3;
  *lines++ = 2;
  *lines++ = 4;
  *lines++ = 5;
  *lines++ = 7;
  *lines++ = 6;

  *lines++ = 0;
  *lines++ = 3;
  *lines++ = 1;
  *lines++ = 2;
  *lines++ = 5;
  *lines++ = 6;
  *lines++ = 4;
  *lines++ = 7;

  *n_verts = nverts;
  *n_lines = nlines;
  *vertsptr = verts_save;
  *colorsptr = colors_save;
  *linesptr = lines_save;
}

/* ------------------ BndfTriangles2Geom ------------------------ */

void BndfTriangles2Geom(float **vertsptr, unsigned char **texturesptr, int *n_verts, int **trianglesptr, int *n_triangles, int option, int *frame_size, int *nframes){
  int j;
  int nverts = 0, ntriangles = 0, offset = 0;
  float *verts, *verts_save;
  unsigned char *textures, *textures_save;
  int *triangles, *triangles_save;

  if(npatchinfo>0){
    int nbndf_verts, nbndf_tris;

    GetBndfFileNodes(0, option, NULL, NULL, NULL, &nbndf_verts, NULL, &nbndf_tris, frame_size, nframes);

    nverts += 3*nbndf_verts;     // 3 coordinates per vertex
    ntriangles += 3*nbndf_tris;  // 3 indices per triangles
  }

  if(nverts==0||ntriangles==0){
    *n_verts = 0;
    *n_triangles = 0;
    *vertsptr = NULL;
    *texturesptr = NULL;
    *trianglesptr = NULL;
    return;
  }

  NewMemory((void **)&verts_save, nverts*sizeof(float));
  NewMemory((void **)&textures_save, (*frame_size*(*nframes))*sizeof(float));
  NewMemory((void **)&triangles_save, ntriangles*sizeof(int));
  verts = verts_save;
  textures = textures_save;
  triangles = triangles_save;

  // load slice file data into data structures

  if(npatchinfo>0){
    int nbndf_verts, nbndf_tris;

    GetBndfFileNodes(1, option, &offset, verts, textures, &nbndf_verts, triangles, &nbndf_tris, frame_size, nframes);
    verts += 3*nbndf_verts;
    triangles += 3*nbndf_tris;
  }

  *n_verts = nverts;
  *n_triangles = ntriangles;
  *vertsptr = verts_save;
  *texturesptr = textures_save;
  *trianglesptr = triangles_save;
}

/* ------------------ SliceTriangles2Geom ------------------------ */

void SliceTriangles2Geom(float **vertsptr, unsigned char **texturesptr, int *n_verts, int **trianglesptr, int *n_triangles, int option, int *frame_size, int *nframes){
  int j;
  int nverts = 0, ntriangles = 0, offset = 0;
  float *verts, *verts_save;
  unsigned char *textures, *textures_save;
  int *triangles, *triangles_save;

  if(nsliceinfo>0){
    int nslice_verts, nslice_tris;

    GetSliceFileNodes(0, option, NULL, NULL, NULL, &nslice_verts, NULL, &nslice_tris, frame_size, nframes);

    nverts += 3*nslice_verts;     // 3 coordinates per vertex
    ntriangles += 3*nslice_tris;  // 3 indices per triangles
  }

  if(nverts==0||ntriangles==0){
    *n_verts = 0;
    *n_triangles = 0;
    *vertsptr = NULL;
    *texturesptr = NULL;
    *trianglesptr = NULL;
    return;
  }

  NewMemory((void **)&verts_save, nverts*sizeof(float));
  NewMemory((void **)&textures_save, (*frame_size*(*nframes))*sizeof(float));
  NewMemory((void **)&triangles_save, ntriangles*sizeof(int));
  verts = verts_save;
  textures = textures_save;
  triangles = triangles_save;

  // load slice file data into data structures

  if(nsliceinfo>0){
    int nslice_verts, nslice_tris;

    GetSliceFileNodes(1, option, &offset, verts, textures, &nslice_verts, triangles, &nslice_tris, frame_size, nframes);
    verts += 3*nslice_verts;
    triangles += 3*nslice_tris;
  }

  *n_verts = nverts;
  *n_triangles = ntriangles;
  *vertsptr = verts_save;
  *texturesptr = textures_save;
  *trianglesptr = triangles_save;
}

/* ------------------ LitTriangles2Geom ------------------------ */

void LitTriangles2Geom(float **vertsptr, float **normalsptr, float **colorsptr, int *n_verts, int **trianglesptr, int *n_triangles){
  int j;
  int nverts = 0, ntriangles = 0, offset = 0;
  float *verts, *verts_save, *normals, *normals_save, *colors, *colors_save;
  int *triangles, *triangles_save;

  // count triangle vertices and indices for blockes

  for(j = 0; j<nmeshes; j++){
    meshdata *meshi;

    meshi = meshinfo+j;
    nverts += meshi->nbptrs*24*3;     // 24 vertices per blockages * 3 coordinates per vertex
    ntriangles += meshi->nbptrs*6*2*3;   // 6 faces per blockage * 2 triangles per face * 3 indicies per triangle
  }

  // count triangle vertices and indices for immersed geometry objects

  if(ngeominfoptrs>0){
    int ngeom_verts, ngeom_tris;

    LOCK_TRIANGLES;
    GetGeomInfoPtrs(0);
    UNLOCK_TRIANGLES;
    ShowHideSortGeometry(0, NULL);
    GetGeometryNodes(0, NULL, NULL, NULL, NULL, &ngeom_verts, NULL, &ngeom_tris);

    nverts += 3*ngeom_verts; // 3 coordinates per vertex
    ntriangles += 3*ngeom_tris;  // 3 indices per triangles
  }

  if(nverts==0||ntriangles==0){
    *n_verts = 0;
    *n_triangles = 0;
    *vertsptr = NULL;
    *normalsptr = NULL;
    *colorsptr = NULL;
    *trianglesptr = NULL;
    return;
  }

  NewMemory((void **)&verts_save, nverts*sizeof(float));
  NewMemory((void **)&normals_save, nverts*sizeof(float));
  NewMemory((void **)&colors_save, nverts*sizeof(float));
  NewMemory((void **)&triangles_save, ntriangles*sizeof(int));
  verts = verts_save;
  normals = normals_save;
  colors = colors_save;
  triangles = triangles_save;

  // load blockage info into data structures

  for(j = 0; j<nmeshes; j++){
    meshdata *meshi;
    int i;

    meshi = meshinfo+j;
    for(i = 0; i<meshi->nbptrs; i++){
      blockagedata *bc;
      float xyz[72];
      int tris[36];
      float norms[72];
      int k;

      bc = meshi->blockageinfoptrs[i];
      GetBlockNodes(meshi, bc, xyz, norms, tris);
      for(k = 0; k<24; k++){
        *verts++ = xyz[3*k+0];
        *verts++ = xyz[3*k+1];
        *verts++ = xyz[3*k+2];
        *normals++ = norms[3*k+0];
        *normals++ = norms[3*k+1];
        *normals++ = norms[3*k+2];
        *colors++ = bc->color[0];
        *colors++ = bc->color[1];
        *colors++ = bc->color[2];
      }
      for(k = 0; k<12; k++){
        *triangles++ = offset+tris[3*k+0];
        *triangles++ = offset+tris[3*k+1];
        *triangles++ = offset+tris[3*k+2];
      }
      offset += 24;
    }
  }

  // load immersed geometry info into data structures

  if(ngeominfoptrs>0){
    int ngeom_verts, ngeom_tris;

    GetGeometryNodes(1, &offset, verts, normals, colors, &ngeom_verts, triangles, &ngeom_tris);
    verts += 3*ngeom_verts;
    normals += 3*ngeom_verts;
    triangles += 3*ngeom_tris;
  }

  *n_verts = nverts;
  *n_triangles = ntriangles;
  *vertsptr = verts_save;
  *normalsptr = normals_save;
  *colorsptr = colors_save;
  *trianglesptr = triangles_save;
}

/* ------------------ GetHtmlFileName ------------------------ */

int GetHtmlFileName(char *htmlfile_full, char *htmlslicefile_full, char *htmlslicefile_base, int option){
  char htmlfile_dir[1024], htmlfile_suffix[1024];
  int image_num;

  // construct html filename

  strcpy(htmlfile_dir, ".");
  strcpy(htmlfile_suffix, "");

  // directory - put files in '.' or smokevewtempdir

  if(Writable(htmlfile_dir)==NO){
    if(Writable(smokeviewtempdir)==YES){
      strcpy(htmlfile_dir, smokeviewtempdir);
    }
    else{
      if(smokeviewtempdir!=NULL&&strlen(smokeviewtempdir)>0){
        fprintf(stderr, "*** Error: unable to output html file to either directories %s or %s\n",
          htmlfile_dir, smokeviewtempdir);
      }
      else{
        fprintf(stderr, "*** Error: unable to output html file to directory %s \n", htmlfile_dir);
      }
      return 1;
    }
  }

  // filename suffix

  if(option==CURRENT_TIME){
    if(RenderTime==0){
      image_num = seqnum;
    }
    else{
      image_num = itimes;
    }
    sprintf(htmlfile_suffix, "_%04i", image_num);
  }
  else{
    strcpy(htmlfile_suffix, "_all");
  }

  // form full filename from parts

  strcpy(htmlfile_full, html_file_base);
  strcat(htmlfile_full, htmlfile_suffix);
  strcpy(htmlslicefile_full, htmlfile_full);
  strcat(htmlfile_full, ".html");
  strcat(htmlslicefile_full, ".htmld");

  strcpy(htmlslicefile_base, html_file_base);
  strcat(htmlslicefile_base, htmlfile_suffix);
  strcat(htmlslicefile_base, ".htmld");
  return 0;
}

/* ------------------ Smv2Html ------------------------ */

int Smv2Html(char *html_file, int option){
  FILE *stream_in = NULL, *stream_out;
  float *vertsLitSolid, *normalsLitSolid, *colorsLitSolid;
  int nvertsLitSolid, *facesLitSolid, nfacesLitSolid;

  float *verts_slice_node;
  unsigned char *textures_slice;
  int nverts_slice_node, *faces_slice_node, nfaces_slice_node;
  int slice_node_framesize, nslice_node_frames;

  float *verts_bndf;
  unsigned char *textures_bndf;
  int nverts_bndf, *faces_bndf, nfaces_bndf;
  int bndf_framesize, nbndf_frames;

  float *vertsLine, *colorsLine;
  int nvertsLine, *facesLine, nfacesLine;

  char html_fullfile[1024], html_slicefile[1024], html_slicefile_base[1024];
  int return_val;
  int copy_html;
  int have_slice_geom = 0;
  int i;

  for(i = 0; i<nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo+i;
    if(slicei->loaded==0||slicei->display==0)continue;
    if(slicei->slicefile_type==SLICE_GEOM){
      have_slice_geom = 1;
      break;
    }
  }

  stream_in = fopen(smokeview_html, "r");
  if(stream_in==NULL){
    printf("***error: smokeview html template file %s failed to open\n", smokeview_html);
    return 1;
  }

  return_val = GetHtmlFileName(html_fullfile, html_slicefile, html_slicefile_base, option);
  if(return_val==1){
    fclose(stream_in);
    return 1;
  }
  stream_out = fopen(html_fullfile, "w");
  if(stream_out==NULL){
    printf("***error: html output file %s failed to open for output\n", html_fullfile);
    fclose(stream_in);
    return 1;
  }

  printf("outputting html to %s", html_fullfile);
  rewind(stream_in);

  // obtain vertices, triangles and lines

  BndfTriangles2Geom(&verts_bndf, &textures_bndf, &nverts_bndf, &faces_bndf, &nfaces_bndf, option,
    &bndf_framesize, &nbndf_frames);
  SliceTriangles2Geom(&verts_slice_node, &textures_slice, &nverts_slice_node, &faces_slice_node, &nfaces_slice_node, option,
    &slice_node_framesize, &nslice_node_frames);
  LitTriangles2Geom(&vertsLitSolid, &normalsLitSolid, &colorsLitSolid, &nvertsLitSolid, &facesLitSolid, &nfacesLitSolid);
  Lines2Geom(&vertsLine, &colorsLine, &nvertsLine, &facesLine, &nfacesLine);

#define PER_ROW 12
#define PERCOLOR_ROW 8
#define PERBIN_ROW 24
  copy_html = 1;
  for(;;){
    char buffer[255];
    int have_data = 0;

    if(feof(stream_in)!=0)break;

    if(fgets(buffer, 255, stream_in)==NULL)break;
    TrimBack(buffer);
    if(Match(buffer, "<!--***CANVAS")==1){
      fprintf(stream_out, "<p>\n");

      // reset buttons
      fprintf(stream_out, "<button onclick = \"Reset()\">Reset View </button><br>\n");

      //show/hide scene elements
      fprintf(stream_out, "<button onclick = \"show_blockages=ShowHide(show_blockages)\">blockages</button>\n");
      fprintf(stream_out, "<button onclick = \"show_outlines=ShowHide(show_outlines)\">outlines</button><br>\n");
      if(nverts_slice_node>0){
        have_data = 1;
        fprintf(stream_out, "<button onclick = \"show_slice_node=ShowHide(show_slice_node)\">slice(node centered)</button>\n");
      }
      //fprintf(stream_out, "<button onclick=\"show_slice_cell=ShowHide(show_slice_cell)\">slice(cell centered)</button>\n");
      if(have_slice_geom==1){
        have_data = 1;
        fprintf(stream_out, "<button onclick = \"show_slice_geom=ShowHide(show_slice_geom)\">slice(geom)</button>\n");
      }
      if(have_data==1)fprintf(stream_out, "<br>\n");

      //pause
      if(option==ALL_TIMES){
        fprintf(stream_out, "<button onclick = \"SetTime(-2)\"><<</button>\n");
        fprintf(stream_out, "<button onclick = \"SetTime(-1)\"><</button>\n");
        fprintf(stream_out, "<button type = \"button\" id = \"buttonPauseResume\" onclick = \"SetTime(0)\">Pause</button>\n");
        fprintf(stream_out, "<button onclick = \"SetTime(1)\">></button>\n");
        fprintf(stream_out, "<button onclick = \"SetTime(2)\">>></button><br>\n");
      }


      fprintf(stream_out, "<canvas width = \"%i\" height = \"%i\" id = \"webSmokeview\"></canvas>", screenWidth, screenHeight);
      continue;
    }
    else if(Match(buffer, "//***VERTS")==1){
      int i;

      // center of scene
      fprintf(stream_out, "         var xcen=%f;\n", xbar/2.0);
      fprintf(stream_out, "         var ycen=%f;\n", ybar/2.0);
      fprintf(stream_out, "         var zcen=%f;\n", zbar/2.0);
      if(option==ALL_TIMES){
        fprintf(stream_out, "         document.getElementById(\"buttonPauseResume\").style.width = \"75px\";\n");
      }

      // add unlit triangles
      fprintf(stream_out, "         var vertices_slice_node = [\n");

      for(i = 0; i<nverts_slice_node; i++){
        char label[100];

        sprintf(label, "%f", verts_slice_node[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1)||i==(nverts_slice_node-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var nframes = %i;\n", nslice_node_frames);
      fprintf(stream_out, "\n");
      fprintf(stream_out, "         var frame_size_slice_node = %i;\n", slice_node_framesize);
      fprintf(stream_out, "         var slice_node_file = \"%s\";\n", html_slicefile_base);

      if(slice_node_framesize*nslice_node_frames>0){
        FILE *slicestream_out =NULL;

        slicestream_out = fopen(html_slicefile,"wb");
        if(slicestream_out!=NULL){
          fwrite(textures_slice, sizeof(unsigned char), slice_node_framesize*nslice_node_frames, slicestream_out);
          fclose(slicestream_out);
        }
      }

      fprintf(stream_out, "         var show_slice_node          = 1;\n");
#ifdef pp_HTML_FILE
      fprintf(stream_out, "         var slice_node_file_ready    = 0;\n");
      fprintf(stream_out, "         var textures_slice_node_data = new Uint8Array(nframes*frame_size);\n");
#else
      fprintf(stream_out, "         var slice_node_file_ready    = 1;\n");
      fprintf(stream_out, "         var textures_slice_node_data = [\n");
      for(i = 0; i<slice_node_framesize*nslice_node_frames; i++){
        char label[100];

        sprintf(label, "%i", CLAMP((int)textures_slice[i],0,255) );
        fprintf(stream_out, "%s,", label);
        if(i%PERBIN_ROW==(PERBIN_ROW-1)||i==(slice_node_framesize*nslice_node_frames-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");
#endif
      fprintf(stream_out, "         var textures_slice_node = new Float32Array([\n");
      for(i = 0; i<slice_node_framesize; i++){
        char label[100];

        sprintf(label, "%f", CLAMP((float)textures_slice[i]/255.0, 0.0, 1.0));
        fprintf(stream_out, "%s,", label);
        if(i%PERBIN_ROW==(PERBIN_ROW-1)||i==(slice_node_framesize-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ]);\n");

      fprintf(stream_out, "         var part_file_ready     = 0;\n");
      fprintf(stream_out, "         var show_part           = 0;\n");
      fprintf(stream_out, "\n");
      fprintf(stream_out, "         var bndf_file_ready     = 0;\n");
      fprintf(stream_out, "         var show_bndf           = 0;\n");
      fprintf(stream_out, "\n");
      fprintf(stream_out, "         var show_outlines       = 1;\n");
      fprintf(stream_out, "         var show_blockages      = 1;\n");
      fprintf(stream_out, "         var slice_cell_file_ready = 0;\n");
      fprintf(stream_out, "         var show_slice_cell     = 0;\n");

      fprintf(stream_out, "         const texture_colorbar_data = new Uint8Array([\n");
      for(i = 0; i<256; i++){
        int ii[3];

        ii[0] = CLAMP(255*rgb_slice[4*i+0], 0, 255);
        ii[1] = CLAMP(255*rgb_slice[4*i+1], 0, 255);
        ii[2] = CLAMP(255*rgb_slice[4*i+2], 0, 255);
        fprintf(stream_out, "%i,%i,%i,255,", ii[0], ii[1], ii[2]);
        if(i%PERCOLOR_ROW==(PERCOLOR_ROW-1)||i==255)fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ]);\n");
      fprintf(stream_out, "         const texture_colorbar_numcolors = 256;\n");

      fprintf(stream_out, "         var indices_slice_node = [\n");
        for(i = 0; i<nfaces_slice_node; i++){
        fprintf(stream_out, "%i,", faces_slice_node[i]);
        if(i%PERBIN_ROW==(PERBIN_ROW-1)||i==(nfaces_slice_node-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      // cell centered slicefiles
      fprintf(stream_out, "         var vertices_slice_cell = [\n");
      fprintf(stream_out, "         ];\n");
      fprintf(stream_out, "         var textures_slice_cell = new Float32Array([\n");
      fprintf(stream_out, "         ]);\n");
      fprintf(stream_out, "         var indices_slice_cell = [\n");
      fprintf(stream_out, "         ];\n");


      // add lit triangles
      fprintf(stream_out, "         var vertices_lit = [\n");
      for(i = 0; i<nvertsLitSolid; i++){
        char label[100];

        sprintf(label, "%f", vertsLitSolid[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1)||i==(nvertsLitSolid-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var normals_lit = [\n");
      for(i = 0; i<nvertsLitSolid; i++){
        char label[100];

        sprintf(label, "%f", normalsLitSolid[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1)||i==(nvertsLitSolid-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var colors_lit = [\n");
      for(i = 0; i<nvertsLitSolid; i++){
        char label[100];

        sprintf(label, "%f", colorsLitSolid[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1)||i==(nvertsLitSolid-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var indices_lit = [\n");
      for(i = 0; i<nfacesLitSolid; i++){
        fprintf(stream_out, "%i,", facesLitSolid[i]);
        if(i%PERBIN_ROW==(PERBIN_ROW-1)||i==(nfacesLitSolid-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      // add lines
      fprintf(stream_out, "         var vertices_line = [\n");
      for(i = 0; i<nvertsLine; i++){
        char label[100];

        sprintf(label, "%f", vertsLine[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1)||i==(nvertsLine-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var colors_line = [\n");
      for(i = 0; i<nvertsLine; i++){
        char label[100];

        sprintf(label, "%f", colorsLine[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1)||i==(nvertsLine-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var indices_line = [\n");
      for(i = 0; i<nfacesLine; i++){
        fprintf(stream_out, "%i,", facesLine[i]);
        if(i%PERBIN_ROW==(PERBIN_ROW-1)||i==(nfacesLine-1))fprintf(stream_out, "\n");
      }
      fprintf(stream_out, "         ];\n");
      continue;
    }
    else if(Match(buffer, "//HIDE_ON")==1){
      copy_html = 0;
      continue;
    }
    else if(Match(buffer, "//HIDE_OFF")==1){
      copy_html = 1;
      continue;
    }
    else if(copy_html==1)fprintf(stream_out, "%s\n", buffer);
  }

  fclose(stream_in);
  fclose(stream_out);
  printf(" - complete\n");
  return 0;
}
#endif
