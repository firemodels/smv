#include "options.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

#include "smokeviewvars.h"

#ifdef pp_HTML

/* --------------------------  webgeomdata ------------------------------------ */

typedef struct _webgeomdata {
  char type[32];
  unsigned char *textures;
  float *verts, *colors;
  int *indices, *framesizes;
  int nverts, nindices, framesize, nframes;
} webgeomdata;

/* ------------------ GetPartFileNodes ------------------------ */

void GetPartVerts(int option, int option2, int *offset, 
  float *verts, float *colors, int *nverts, 
  int *indices, int *nindices,
  int *frame_sizes, int *nframes){
  int i, first = 1;
  int ibeg, iend;
  partdata *parttime=NULL;
  int itime;

  *nverts = 0;
  *nindices  = 0;
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
  if(first==1)return;
  if(option2==ALL_TIMES){
    ibeg = 0;
    iend = *nframes;
  }
  else{
    *nframes = 1;
    ibeg = parttime->itime;
    iend = parttime->itime+1;
  }
  for(itime = ibeg; itime<iend; itime++){
    for(i = 0; i<npartinfo; i++){
      partdata *parti;

      parti = partinfo+i;
      if(parti->loaded==0||parti->display==0||part5show==0)continue;
      if(streak5show == 0 || (streak5show == 1 && showstreakhead == 1)){
        part5data *datacopy;

        datacopy = parti->data5 + parti->nclasses*itime;
        *nverts   += datacopy->npoints;
        *nindices += datacopy->npoints;
      }
    }
  }
  if(option==0)return;
  for(itime = ibeg; itime<iend; itime++){
    frame_sizes[itime-ibeg] = 0;
    for(i = 0; i<npartinfo; i++){
      partdata *parti;
      int j;

      parti = partinfo+i;
      if(parti->loaded==0||parti->display==0||part5show==0)continue;
      if(streak5show==0||(streak5show==1&&showstreakhead==1)){
        part5data *datacopy;
        short *sx, *sy, *sz;
        partclassdata *partclassi;
        int partclass_index, itype;

        datacopy = parti->data5+parti->nclasses*itime;
        frame_sizes[itime-ibeg] += datacopy->npoints;
        sx = datacopy->sx;
        sy = datacopy->sy;
        sz = datacopy->sz;

        partclassi = parti->partclassptr[i];
        partclass_index = partclassi - partclassinfo;
        itype = current_property->class_types[partclass_index];

        for(j=0;j<datacopy->npoints;j++){
          *verts++   = xplts[sx[j]];
          *verts++   = yplts[sy[j]];
          *verts++   = zplts[sz[j]];
          if(itype==-1){
            *colors++   = 0.0;
            *colors++   = 0.0;
            *colors++   = 0.0;
          }
          else{
            unsigned char *color_index;
            float *color;

            color_index = datacopy->irvals + itype*datacopy->npoints + j;
            color = rgb_full[*color_index];
            *colors++   = color[0];
            *colors++   = color[1];
            *colors++   = color[2];
          }
          *indices++ = *offset + j;
        }
        *offset += datacopy->npoints;
      }
    }
  }
}

  /* ------------------ GetBndfNodeVerts ------------------------ */

void GetBndfNodeVerts(int option, int option2, int *offset,
  float *verts, unsigned char *textures, int *nverts,
  int *tris, int *ntris,
  int *frame_size, int *nframes){
  int i, first = 1, minsteps;
  int itime, ibeg, iend;
  meshdata *meshpatch;
  int nv, nt;

  nv = 0;
  nt = 0;
  *frame_size = 0;
  *nframes = 0;
  *nverts = 0;
  *ntris = 0;
  for(i = 0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo+i;
    if(patchi->loaded==0||patchi->display==0||patchi->structured==NO)continue;
    if(patchi->patch_filetype!=PATCH_STRUCTURED_NODE_CENTER)continue;
    meshpatch = meshinfo+patchi->blocknumber;
    if(first==1){
      first = 0;
      minsteps = patchi->ntimes;
    }
    else{
      minsteps = MIN(minsteps, patchi->ntimes);
    }
    if(option2==CURRENT_TIME)break;
  }
  if(first==1){
    return;
  }
  if(option2==ALL_TIMES){
    ibeg = 0;
    iend = minsteps;
    *nframes = iend;
  }
  else{
    ibeg = meshpatch->patch_itime;
    iend = meshpatch->patch_itime+1;
    *nframes = 1;
  }
  *frame_size = 0;
  for(itime = ibeg; itime<iend; itime++){
    int j;

    for(j = 0;j<npatchinfo;j++){
      patchdata *patchi;
      meshdata *meshpatch;
      int n, *vis_boundaries, *patchdir, *boundarytype, *boundary_row, *boundary_col, *blockstart;
      unsigned char *cpatch_time;

      patchi = patchinfo+j;
      if(patchi->loaded==0||patchi->display==0||patchi->structured==NO)continue;
      if(patchi->patch_filetype!=PATCH_STRUCTURED_NODE_CENTER)continue;

      meshpatch = meshinfo+patchi->blocknumber;
      patchdir = meshpatch->patchdir;
      vis_boundaries = meshpatch->vis_boundaries;
      boundarytype = meshpatch->boundarytype;
      boundary_row = meshpatch->boundary_row;
      boundary_col = meshpatch->boundary_col;
      blockstart = meshpatch->blockstart;

      cpatch_time = meshpatch->cpatchval+itime*meshpatch->npatchsize;
      if(itime==ibeg){
        for(n = 0;n<meshpatch->npatches;n++){
          int drawit;

          drawit = 0;
          if(vis_boundaries[n]==1&&patchdir[n]>0){
            if(boundarytype[n]==INTERIORwall||showpatch_both==0){
              drawit = 1;
            }
          }
          if(drawit==1){
            int nrow, ncol;

            nrow = boundary_row[n];
            ncol = boundary_col[n];
            nv += nrow*ncol;
            nt += 2*(nrow-1)*(ncol-1);
          }
        }
        *nverts     += nv;
        *ntris      += nt;
        *frame_size += nv;
      }
      if(option==1){
        for(n = 0;n<meshpatch->npatches;n++){
          int drawit, irow, nrow, ncol;

          drawit = 0;
          if(vis_boundaries[n]==1&&patchdir[n]>0){
            if(boundarytype[n]==INTERIORwall||showpatch_both==0){
              drawit = 1;
            }
          }
          if(drawit==0)continue;
          nrow = boundary_row[n];
          ncol = boundary_col[n];
          if(itime==ibeg){
            float *xyzpatchcopy;

            xyzpatchcopy = meshpatch->xyzpatch+3*blockstart[n];
            for(irow = 0;irow<nrow;irow++){
              int icol;
              float *xyz;

              xyz = xyzpatchcopy+3*irow*ncol;
              for(icol = 0;icol<ncol;icol++){
                *verts++ = *xyz++;
                *verts++ = *xyz++;
                *verts++ = *xyz++;
              }
            }
            for(irow = 0;irow<nrow-1;irow++){
              int icol;

              for(icol = 0;icol<ncol-1;icol++){
                *tris++ = *offset+(icol+0)+(irow+0)*ncol;
                *tris++ = *offset+(icol+1)+(irow+0)*ncol;
                *tris++ = *offset+(icol+1)+(irow+1)*ncol;

                *tris++ = *offset+(icol+0)+(irow+0)*ncol;
                *tris++ = *offset+(icol+1)+(irow+1)*ncol;
                *tris++ = *offset+(icol+0)+(irow+1)*ncol;
              }
            }
            *offset += ncol*nrow;
          }
          for(irow = 0;irow<nrow;irow++){
            int icol;
            unsigned char *cpatchval1;

            cpatchval1 = cpatch_time + blockstart[n] + irow*ncol;
            for(icol = 0;icol<ncol;icol++){
              *textures++ = *cpatchval1++;
            }
          }
        }
      }
    }
  }
}

/* ------------------ GetSliceCellVerts ------------------------ */

void GetSliceCellVerts(int option, int option2, int *offset, float *verts, unsigned char *textures, int *nverts, int *tris, int *ntris, int *frame_size, int *nframes){
  int islice, nv = 0, nt = 0;
  int ibeg, iend, itime, first=1, minsteps;
  slicedata *slicetime=NULL;

  for(islice = 0; islice<nsliceinfo; islice++){
    slicedata *slicei;

    slicei = sliceinfo+islice;
    if(slicei->loaded==0||slicei->display==0||slicei->slice_filetype!=SLICE_CELL_CENTER||slicei->volslice==1)continue;
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

      if(slicei->loaded==0||slicei->display==0||slicei->slice_filetype!=SLICE_CELL_CENTER||slicei->volslice==1)continue;
      if(slicei->idir!=XDIR&&slicei->idir!=YDIR&&slicei->idir!=ZDIR)continue;

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
          int ntris, nverts;

          ntris = 2*(nrows-1)*(ncols-1);
          nverts = 3*ntris;
          *frame_size += nverts;
          nv += nverts;
          nt += ntris;
        }
        if(option==1){
          meshdata *meshi;
          float *xplt, *yplt, *zplt;
          int plotx, ploty, plotz;
          float  constval;
          int n, i, j, k;

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
              int index = 0;

              constval = xplt[plotx];
              for(j = slicei->js1; j<slicei->js2; j++){
                for(k = slicei->ks1; k<slicei->ks2; k++){
                  *tris++ = *offset+index+0;
                  *tris++ = *offset+index+1;
                  *tris++ = *offset+index+2;

                  *tris++ = *offset+index+3;
                  *tris++ = *offset+index+4;
                  *tris++ = *offset+index+5;
                  index += 6;

                  verts[0] = constval; verts[1] = yplt[j  ]; verts[2] = zplt[k  ]; verts += 3;
                  verts[0] = constval; verts[1] = yplt[j+1]; verts[2] = zplt[k+1]; verts += 3;
                  verts[0] = constval; verts[1] = yplt[j  ]; verts[2] = zplt[k+1]; verts += 3;
                  verts[0] = constval; verts[1] = yplt[j  ]; verts[2] = zplt[k  ]; verts += 3;
                  verts[0] = constval; verts[1] = yplt[j+1]; verts[2] = zplt[k  ]; verts += 3;
                  verts[0] = constval; verts[1] = yplt[j+1]; verts[2] = zplt[k+1]; verts += 3;
                }
              }
              *offset += 6*(nrows-1)*(ncols-1);
            }
            // textures
            for(j = slicei->js1; j<slicei->js2; j++){
              n = (j+1-slicei->js1)*slicei->nslicei*slicei->nslicek-1;
              n += (plotx-slicei->is1)*slicei->nslicek+1;

              for(k = slicei->ks1; k<slicei->ks2; k++){
                n++;
                *textures++ = iq[n];
                *textures++ = iq[n];
                *textures++ = iq[n];
                *textures++ = iq[n];
                *textures++ = iq[n];
                *textures++ = iq[n];
              }
            }
            break;
          case YDIR:
            if(itime==ibeg){
              int index = 0;

              constval = yplt[ploty];
              for(i = slicei->is1; i<slicei->is2; i++){
                for(k = slicei->ks1; k<slicei->ks2; k++){
                  *tris++ = *offset+index+0;
                  *tris++ = *offset+index+1;
                  *tris++ = *offset+index+2;

                  *tris++ = *offset+index+3;
                  *tris++ = *offset+index+4;
                  *tris++ = *offset+index+5;
                  index += 6;

                  verts[0] = xplt[i  ]; verts[1] = constval; verts[2] = zplt[k  ]; verts += 3;
                  verts[0] = xplt[i+1]; verts[1] = constval; verts[2] = zplt[k+1]; verts += 3;
                  verts[0] = xplt[i  ]; verts[1] = constval; verts[2] = zplt[k+1]; verts += 3;
                  verts[0] = xplt[i  ]; verts[1] = constval; verts[2] = zplt[k  ]; verts += 3;
                  verts[0] = xplt[i+1]; verts[1] = constval; verts[2] = zplt[k  ]; verts += 3;
                  verts[0] = xplt[i+1]; verts[1] = constval; verts[2] = zplt[k+1]; verts += 3;
                }
              }
              *offset += 6*(nrows-1)*(ncols-1);
            }
            // textures
            for(i = slicei->is1; i<slicei->is2; i++){
              n = (i+1-slicei->is1)*slicei->nslicej*slicei->nslicek-1;
              n += (ploty-slicei->js1)*slicei->nslicek+1;

              for(k = slicei->ks1; k<slicei->ks2; k++){
                n++;
                *textures++ = iq[n];
                *textures++ = iq[n];
                *textures++ = iq[n];
                *textures++ = iq[n];
                *textures++ = iq[n];
                *textures++ = iq[n];
              }
            }
            break;
          case ZDIR:
            if(itime==ibeg){
              int index = 0;

              constval = zplt[plotz];
              for(i = slicei->is1; i<slicei->is2; i++){
                for(j = slicei->js1; j<slicei->js2; j++){
                  *tris++ = *offset+index+0;
                  *tris++ = *offset+index+1;
                  *tris++ = *offset+index+2;

                  *tris++ = *offset+index+3;
                  *tris++ = *offset+index+4;
                  *tris++ = *offset+index+5;
                  index += 6;

                  verts[0] = xplt[i  ]; verts[1] = yplt[j  ]; verts[2] = constval; verts += 3;
                  verts[0] = xplt[i+1]; verts[1] = yplt[j+1]; verts[2] = constval; verts += 3;
                  verts[0] = xplt[i  ]; verts[1] = yplt[j+1]; verts[2] = constval; verts += 3;
                  verts[0] = xplt[i  ]; verts[1] = yplt[j  ]; verts[2] = constval; verts += 3;
                  verts[0] = xplt[i+1]; verts[1] = yplt[j  ]; verts[2] = constval; verts += 3;
                  verts[0] = xplt[i+1]; verts[1] = yplt[j+1]; verts[2] = constval; verts += 3;
                }
              }
              *offset += 6*(nrows-1)*(ncols-1);
            }
            // textures
            for(i = slicei->is1; i<slicei->is2; i++){
              n = (i+1-slicei->is1)*slicei->nslicej*slicei->nslicek-1;
              n += (plotz-slicei->ks1)*slicei->nslicey+1;

              for(j = slicei->js1; j<slicei->js2; j++){
                n++;
                *textures++ = iq[n];
                *textures++ = iq[n];
                *textures++ = iq[n];
                *textures++ = iq[n];
                *textures++ = iq[n];
                *textures++ = iq[n];
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

/* ------------------ GetSliceGeomVerts ------------------------ */

void GetSliceGeomVerts(int option, int option2, int *offset, float *verts, unsigned char *textures, int *nverts, int *tris, int *ntris, int *frame_size, int *nframes){
  int islice, nv = 0, nt = 0;
  int ibeg, iend, itime, first = 1, minsteps;
  slicedata *slicetime = NULL;

  for(islice = 0; islice<nsliceinfo; islice++){
    slicedata *slicei;

    slicei = sliceinfo+islice;
    if(slicei->loaded==0||slicei->display==0||slicei->slice_filetype!=SLICE_GEOM||slicei->volslice==1)continue;
    if(slicei->idir!=XDIR&&slicei->idir!=YDIR&&slicei->idir!=ZDIR)continue;
    slicetime = slicei;
    if(first==1){
      minsteps = slicei->ntimes;
      first = 0;
    }
    else{
      minsteps = MIN(minsteps, slicei->ntimes);
    }
    if(option2==CURRENT_TIME)break;
  }
  if(first==1){
    *frame_size = 0;
    *nframes = 0;
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
      geomdata *geomi;
      geomlistdata *geomlisti;
      patchdata *patchi;
      unsigned char *ivals;

      slicei = sliceinfo+islice;

      if(slicei->loaded==0||slicei->display==0||slicei->slice_filetype!=SLICE_GEOM||slicei->volslice==1)continue;
      if(slicei->idir!=XDIR&&slicei->idir!=YDIR&&slicei->idir!=ZDIR)continue;

      // preliminary code for obtaining geometry vertices and triangles

      patchi = slicei->patchgeom;
      ivals = patchi->geom_ivals_dynamic[itime];
      geomi = slicei->patchgeom->geominfo;
      geomlisti = geomi->geomlistinfo-1;
      if(itime==ibeg){
        int nt_geom, nv_geom;

        nt_geom = geomlisti->ntriangles;
        nv_geom = 3*nt_geom;
        nt += nt_geom;
        nv += nv_geom;
        *frame_size += nv_geom;
      }
      if(option==1){
        int i;

        if(itime==ibeg){
          int index=0;

          for(i = 0; i<geomlisti->ntriangles; i++){
            tridata *trii;
            vertdata *v1, *v2, *v3;

            *tris++ = *offset+index+0;
            *tris++ = *offset+index+1;
            *tris++ = *offset+index+2;
            index += 3;
            trii = geomlisti->triangles+i;
            v1 = trii->verts[0];
            v2 = trii->verts[1];
            v3 = trii->verts[2];
            *verts++ = NORMALIZE_X(v1->xyz[0]);
            *verts++ = NORMALIZE_Y(v1->xyz[1]);
            *verts++ = NORMALIZE_Z(v1->xyz[2]);
            *verts++ = NORMALIZE_X(v2->xyz[0]);
            *verts++ = NORMALIZE_Y(v2->xyz[1]);
            *verts++ = NORMALIZE_Z(v2->xyz[2]);
            *verts++ = NORMALIZE_X(v3->xyz[0]);
            *verts++ = NORMALIZE_Y(v3->xyz[1]);
            *verts++ = NORMALIZE_Z(v3->xyz[2]);
          }
          *offset += 3*geomlisti->ntriangles;
        }
        for(i = 0; i<geomlisti->ntriangles; i++){
          *textures++ = ivals[i];
          *textures++ = ivals[i];
          *textures++ = ivals[i];
        }
      }
    }
  }
  *nverts = nv;
  *ntris = nt;
}

/* ------------------ GetSliceNodeVerts ------------------------ */

void GetSliceNodeVerts(int option, int option2, int *offset, float *verts, unsigned char *textures, int *nverts, int *tris, int *ntris, int *frame_size, int *nframes){
  int islice, nv = 0, nt = 0;
  int ibeg, iend, itime, first=1, minsteps;
  slicedata *slicetime=NULL;

  for(islice = 0; islice<nsliceinfo; islice++){
    slicedata *slicei;

    slicei = sliceinfo+islice;
    if(slicei->loaded==0||slicei->display==0||slicei->slice_filetype!=SLICE_NODE_CENTER||slicei->volslice==1)continue;
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

      if(slicei->loaded==0||slicei->display==0||slicei->slice_filetype!=SLICE_NODE_CENTER||slicei->volslice==1)continue;
      if(slicei->idir!=XDIR&&slicei->idir!=YDIR&&slicei->idir!=ZDIR)continue;

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
  int nverts = 0, nlines = 0;
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

void BndfNodeTriangles2Geom(webgeomdata *bndf_node_web, int option){
  int nverts = 0, nindices = 0, offset = 0;
  float *verts, *verts_save;
  unsigned char *textures, *textures_save;
  int *indices, *indices_save;

  if(npatchinfo>0){
    int nbndf_node_verts, nbndf_node_tris;

    GetBndfNodeVerts(0, option, NULL, NULL, NULL, &nbndf_node_verts,
      NULL, &nbndf_node_tris,
      &(bndf_node_web->framesize), &(bndf_node_web->nframes));

    nverts   += 3*nbndf_node_verts;     // 3 coordinates per vertex
    nindices += 3*nbndf_node_tris;  // 3 indices per triangles
  }

  if(nverts==0||nindices==0){
    bndf_node_web->nverts   = 0;
    bndf_node_web->nindices = 0;
    bndf_node_web->verts    = NULL;
    bndf_node_web->textures = NULL;
    bndf_node_web->indices  = NULL;
    return;
  }

  NewMemory((void **)&verts_save, nverts*sizeof(float));
  NewMemory((void **)&textures_save, (bndf_node_web->framesize*bndf_node_web->nframes)*sizeof(float));
  NewMemory((void **)&indices_save, nindices*sizeof(int));
  verts = verts_save;
  textures = textures_save;
  indices = indices_save;

  // load slice file data into data structures

  if(npatchinfo>0){
    int nbndf_node_verts, nbndf_node_tris;

    GetBndfNodeVerts(1, option, &offset, verts, textures, &nbndf_node_verts,
      indices, &nbndf_node_tris,
      &(bndf_node_web->framesize), &(bndf_node_web->nframes));
    verts     += 3*nbndf_node_verts;
    indices += 3*nbndf_node_tris;
  }

  bndf_node_web->nverts   = nverts;
  bndf_node_web->nindices = nindices;
  bndf_node_web->verts    = verts_save;
  bndf_node_web->textures = textures_save;
  bndf_node_web->indices  = indices_save;
}


/* ------------------ PartNodeVerts2Geom ------------------------ */

void PartNodeVerts2Geom(webgeomdata *part_node_web, int option){
  int nverts = 0, nindices=0, offset = 0;
  float *verts, *verts_save, *colors, *colors_save;
  int *indices, *indices_save, *framesizes;

  if(npartinfo>0){
    int npart_verts, npart_indices;

    GetPartVerts(0, option, NULL, NULL, NULL, &npart_verts, NULL, &npart_indices, NULL, &(part_node_web->nframes));

    nverts   += 3*npart_verts;     // 3 coordinates per vertex
    nindices += npart_verts;       // 3 indice per vertex
  }

  if(nverts==0){
    part_node_web->nverts     = 0;
    part_node_web->nindices   = 0;
    part_node_web->framesize  = 0;
    part_node_web->verts      = NULL;
    part_node_web->colors     = NULL;
    part_node_web->textures   = NULL;
    part_node_web->framesizes = NULL;
    part_node_web->indices    = NULL;
    return;
  }

  NewMemory((void **)&verts_save, nverts*sizeof(float));
  NewMemory((void **)&colors_save, nverts*sizeof(float));
  NewMemory((void **)&indices_save, nindices*sizeof(int));
  NewMemory((void **)&framesizes,part_node_web->nframes*sizeof(int));
  verts   = verts_save;
  colors  = colors_save;
  indices = indices_save;

  // load particle file data into data structures

  if(npartinfo>0){
    int npart_verts, npart_indices;


    GetPartVerts(1, option, &offset, 
      verts, colors, &npart_verts, 
      indices, &npart_indices,
      framesizes, &(part_node_web->nframes));
    verts   += 3*npart_verts;
    indices += npart_indices;
  }

  part_node_web->nverts     = nverts;
  part_node_web->nindices   = nindices;
  part_node_web->verts      = verts_save;
  part_node_web->colors     = colors_save;
  part_node_web->textures   = NULL;
  part_node_web->framesizes = framesizes;
  part_node_web->indices    = indices_save;
}

/* ------------------ SliceCellTriangles2Geom ------------------------ */

void SliceCellTriangles2Geom(webgeomdata *slice_cell_web, int option){
  int nverts = 0, nindices = 0, offset = 0;
  float *verts, *verts_save;
  unsigned char *textures, *textures_save;
  int *indices, *indices_save;

  if(nsliceinfo>0){
    int nslice_verts, nslice_tris;

    GetSliceCellVerts(0, option, NULL, NULL, NULL, &nslice_verts, NULL, &nslice_tris, &(slice_cell_web->framesize), &(slice_cell_web->nframes));

    nverts   += 3*nslice_verts;     // 3 coordinates per vertex
    nindices += 3*nslice_tris;  // 3 indices per triangles
  }

  if(nverts==0||nindices==0){
    slice_cell_web->nverts   = 0;
    slice_cell_web->nindices = 0;
    slice_cell_web->verts    = NULL;
    slice_cell_web->textures = NULL;
    slice_cell_web->indices  = NULL;
    return;
  }

  NewMemory((void **)&verts_save, nverts*sizeof(float));
  NewMemory((void **)&textures_save, (slice_cell_web->framesize*slice_cell_web->nframes)*sizeof(float));
  NewMemory((void **)&indices_save, nindices*sizeof(int));
  verts    = verts_save;
  textures = textures_save;
  indices  = indices_save;

  // load slice file data into data structures

  if(nsliceinfo>0){
    int nslice_verts, nslice_tris;

    GetSliceCellVerts(1, option, &offset, 
      verts, textures, &nslice_verts, 
      indices, &nslice_tris, 
      &(slice_cell_web->framesize), &(slice_cell_web->nframes));
    verts   += 3*nslice_verts;
    indices += 3*nslice_tris;
  }

  slice_cell_web->nverts   = nverts;
  slice_cell_web->nindices = nindices;
  slice_cell_web->verts    = verts_save;
  slice_cell_web->textures = textures_save;
  slice_cell_web->indices  = indices_save;
}

/* ------------------ SliceNodeTriangles2Geom ------------------------ */

void SliceNodeTriangles2Geom(webgeomdata *slice_node_web, int option){
  int nverts = 0, nindices = 0, offset = 0;
  float *verts, *verts_save;
  unsigned char *textures, *textures_save;
  int *indices, *indices_save;

  if(nsliceinfo>0){
    int nslice_verts, nslice_tris;

    GetSliceNodeVerts(0, option, NULL, NULL, NULL, &nslice_verts, NULL, &nslice_tris, &(slice_node_web->framesize), &(slice_node_web->nframes));

    nverts   += 3*nslice_verts;     // 3 coordinates per vertex
    nindices += 3*nslice_tris;  // 3 indices per triangles
  }

  if(nverts==0||nindices==0){
    slice_node_web->nverts   = 0;
    slice_node_web->nindices = 0;
    slice_node_web->verts    = NULL;
    slice_node_web->textures = NULL;
    slice_node_web->indices  = NULL;
    return;
  }

  NewMemory((void **)&verts_save, nverts*sizeof(float));
  NewMemory((void **)&textures_save, (slice_node_web->framesize*slice_node_web->nframes)*sizeof(float));
  NewMemory((void **)&indices_save, nindices*sizeof(int));
  verts = verts_save;
  textures = textures_save;
  indices = indices_save;

  // load slice file data into data structures

  if(nsliceinfo>0){
    int nslice_verts, nslice_tris;

    GetSliceNodeVerts(1, option, &offset, verts, textures, &nslice_verts, indices, &nslice_tris, &(slice_node_web->framesize), &(slice_node_web->nframes));
    verts   += 3*nslice_verts;
    indices += 3*nslice_tris;
  }

  slice_node_web->nverts   = nverts;
  slice_node_web->nindices = nindices;
  slice_node_web->verts    = verts_save;
  slice_node_web->textures = textures_save;
  slice_node_web->indices  = indices_save;
}

/* ------------------ SliceGeomTriangles2Geom ------------------------ */

void SliceGeomTriangles2Geom(webgeomdata *slice_geom_web, int option){
  int nverts = 0, nindices = 0, offset = 0;
  float *verts, *verts_save;
  unsigned char *textures, *textures_save;
  int *indices, *indices_save;

  if(nsliceinfo>0){
    int nslice_verts, nslice_tris;

    GetSliceGeomVerts(0, option, NULL, NULL, NULL, &nslice_verts, NULL, &nslice_tris, &(slice_geom_web->framesize), &(slice_geom_web->nframes));

    nverts += 3*nslice_verts;     // 3 coordinates per vertex
    nindices += 3*nslice_tris;  // 3 indices per triangles
  }

  if(nverts==0||nindices==0){
    slice_geom_web->nverts   = 0;
    slice_geom_web->nindices = 0;
    slice_geom_web->verts    = NULL;
    slice_geom_web->textures = NULL;
    slice_geom_web->indices  = NULL;
    return;
  }

  NewMemory((void **)&verts_save, nverts*sizeof(float));
  NewMemory((void **)&textures_save, (slice_geom_web->framesize*slice_geom_web->nframes)*sizeof(float));
  NewMemory((void **)&indices_save, nindices*sizeof(int));
  verts = verts_save;
  textures = textures_save;
  indices = indices_save;

  // load slice file data into data structures

  if(nsliceinfo>0){
    int nslice_verts, nslice_tris;

    GetSliceGeomVerts(1, option, &offset, verts, textures, &nslice_verts, indices, &nslice_tris, &(slice_geom_web->framesize), &(slice_geom_web->nframes));
    verts   += 3*nslice_verts;
    indices += 3*nslice_tris;
  }

  slice_geom_web->nverts   = nverts;
  slice_geom_web->nindices = nindices;
  slice_geom_web->verts    = verts_save;
  slice_geom_web->textures = textures_save;
  slice_geom_web->indices  = indices_save;
}

/* ------------------ ObstLitTriangles2Geom ------------------------ */

void ObstLitTriangles2Geom(float **vertsptr, float **normalsptr, float **colorsptr, int *n_verts, int **trianglesptr, int *n_triangles){
  int j;
  int nverts = 0, nindices = 0, offset = 0;
  float *verts, *verts_save, *normals, *normals_save, *colors, *colors_save;
  int *indices, *indices_save;

  // count triangle vertices and indices for blockes

  for(j = 0; j<nmeshes; j++){
    meshdata *meshi;

    meshi     = meshinfo+j;
    nverts   += meshi->nbptrs*24*3;     // 24 vertices per blockages * 3 coordinates per vertex
    nindices += meshi->nbptrs*6*2*3;   // 6 faces per blockage * 2 triangles per face * 3 indicies per triangle
  }

  if(nverts==0||nindices==0){
    *n_verts = 0;
    *n_triangles  = 0;
    *vertsptr     = NULL;
    *normalsptr   = NULL;
    *colorsptr    = NULL;
    *trianglesptr = NULL;
    return;
  }

  NewMemory((void **)&verts_save, nverts*sizeof(float));
  NewMemory((void **)&normals_save, nverts*sizeof(float));
  NewMemory((void **)&colors_save, nverts*sizeof(float));
  NewMemory((void **)&indices_save, nindices*sizeof(int));
  verts = verts_save;
  normals = normals_save;
  colors = colors_save;
  indices = indices_save;

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
        *indices++ = offset+tris[3*k+0];
        *indices++ = offset+tris[3*k+1];
        *indices++ = offset+tris[3*k+2];
      }
      offset += 24;
    }
  }

  *n_verts = nverts;
  *n_triangles = nindices;
  *vertsptr = verts_save;
  *normalsptr = normals_save;
  *colorsptr = colors_save;
  *trianglesptr = indices_save;
}

/* ------------------ GeomLitTriangles2Geom ------------------------ */

void GeomLitTriangles2Geom(float **vertsptr, float **normalsptr, float **colorsptr, int *n_verts, int **trianglesptr, int *n_triangles){
  int nverts = 0, nindices = 0, offset = 0;
  float *verts, *verts_save, *normals, *normals_save, *colors, *colors_save;
  int *indices, *indices_save;

  // count triangle vertices and indices for immersed geometry objects

  if(ngeominfoptrs>0){
    int ngeom_verts, ngeom_tris;

    LOCK_TRIANGLES;
    GetGeomInfoPtrs(0);
    UNLOCK_TRIANGLES;
    ShowHideSortGeometry(0, NULL);
    GetGeometryNodes(0, NULL, NULL, NULL, NULL, &ngeom_verts, NULL, &ngeom_tris);

    nverts += 3*ngeom_verts; // 3 coordinates per vertex
    nindices += 3*ngeom_tris;  // 3 indices per triangles
  }

  if(nverts==0||nindices==0){
    *n_verts      = 0;
    *n_triangles  = 0;
    *vertsptr     = NULL;
    *normalsptr   = NULL;
    *colorsptr    = NULL;
    *trianglesptr = NULL;
    return;
  }

  NewMemory((void **)&verts_save, nverts*sizeof(float));
  NewMemory((void **)&normals_save, nverts*sizeof(float));
  NewMemory((void **)&colors_save, nverts*sizeof(float));
  NewMemory((void **)&indices_save, nindices*sizeof(int));
  verts   = verts_save;
  normals = normals_save;
  colors  = colors_save;
  indices = indices_save;

  // load immersed geometry info into data structures

  if(ngeominfoptrs>0){
    int ngeom_verts, ngeom_tris;

    GetGeometryNodes(1, &offset, verts, normals, colors, &ngeom_verts, indices, &ngeom_tris);
    verts   += 3*ngeom_verts;
    normals += 3*ngeom_verts;
    indices += 3*ngeom_tris;
  }

  *n_verts     = nverts;
  *n_triangles = nindices;
  *vertsptr     = verts_save;
  *normalsptr   = normals_save;
  *colorsptr    = colors_save;
  *trianglesptr = indices_save;
}

/* ------------------ GetHtmlFileName ------------------------ */

int GetHtmlFileName(char *htmlfile_full, int option){
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
  strcat(htmlfile_full, ".html");
  return 0;
}

#define PER_ROW 12
#define PERCOLOR_ROW 8
#define PERBIN_ROW 24

/* --------------------------  InitWebgeom ------------------------------------ */

void InitWebgeom(webgeomdata *wi, char *label){
  strcpy(wi->type, label);
  wi->textures  = NULL;
  wi->verts     = NULL;
  wi->indices   = NULL;
  wi->nverts    = 0;
  wi->nindices  = 0;
  wi->framesize = 0;
  wi->nframes   = 0;
}

/* ------------------ OutputFixedFrame ------------------------ */

void OutputFixedFrame(FILE *stream_out, char *label, webgeomdata *webgi){
  int i;

  fprintf(stream_out, "\n\n// %s\n\n", label);

  if(webgi->nframes>0){
    fprintf(stream_out, "         var nframes = %i;\n", webgi->nframes);
    fprintf(stream_out, "         var show_%s          = 1;\n", webgi->type);
    fprintf(stream_out, "         var %s_file_ready    = 1;\n", webgi->type);
  }
  else{
    fprintf(stream_out, "         var show_%s          = 0;\n", webgi->type);
    fprintf(stream_out, "         var %s_file_ready    = 0;\n", webgi->type);
  }
  fprintf(stream_out, "         var frame_size_%s = %i;\n", webgi->type, webgi->framesize);
  fprintf(stream_out, "         var vertices_%s = [\n", webgi->type);
  for(i = 0; i<webgi->nverts-1; i++){
    char varlabel[100];

    sprintf(varlabel, "%.3f", webgi->verts[i]);
    TrimZeros(varlabel);
    fprintf(stream_out, "%s,", varlabel);
    if(i%PER_ROW==(PER_ROW-1))fprintf(stream_out, "\n");
  }
  if(webgi->nverts>0){
    char varlabel[100];

    sprintf(varlabel, "%.3f", webgi->verts[webgi->nverts-1]);
    TrimZeros(varlabel);
    fprintf(stream_out, "%s\n", varlabel);
  }
  fprintf(stream_out, "         ];\n");

  fprintf(stream_out, "\n");

  fprintf(stream_out, "         var textures_%s_data = [\n", webgi->type);
  for(i = 0; i<webgi->framesize*webgi->nframes-1; i++){
    char varlabel[100];

    sprintf(varlabel, "%i", CLAMP((int)webgi->textures[i], 0, 255));
    fprintf(stream_out, "%s,", varlabel);
    if(i%PERBIN_ROW==(PERBIN_ROW-1))fprintf(stream_out, "\n");
  }
  if(webgi->framesize*webgi->nframes>0){
    char varlabel[100];

    sprintf(varlabel, "%i", CLAMP((int)webgi->textures[webgi->framesize*webgi->nframes-1], 0, 255));
    fprintf(stream_out, "%s\n", varlabel);
  }
  fprintf(stream_out, "         ];\n");

  fprintf(stream_out, "         var textures_%s = new Float32Array([\n", webgi->type);
  for(i = 0; i<webgi->framesize-1; i++){
    char varlabel[100];

    sprintf(varlabel, "%.3f", CLAMP((float)webgi->textures[i]/255.0, 0.0, 1.0));
    fprintf(stream_out, "%s,", varlabel);
    if(i%PERBIN_ROW==(PERBIN_ROW-1))fprintf(stream_out, "\n");
  }
  if(webgi->framesize>0){
    char varlabel[100];

    sprintf(varlabel, "%.3f", CLAMP((float)webgi->textures[webgi->framesize-1]/255.0, 0.0, 1.0));
    fprintf(stream_out, "%s\n", varlabel);
  }
  fprintf(stream_out, "         ]);\n");

  fprintf(stream_out, "         var indices_%s = [\n", webgi->type);
  for(i = 0; i<webgi->nindices-1; i++){
    fprintf(stream_out, "%i,", webgi->indices[i]);
    if(i%PERBIN_ROW==(PERBIN_ROW-1))fprintf(stream_out, "\n");
  }
  if(webgi->nindices>0){
    fprintf(stream_out, "%i\n", webgi->indices[webgi->nindices-1]);
  }
  fprintf(stream_out, "         ];\n");
}

/* ------------------ OutputVariableFrame ------------------------ */

void OutputVariableFrame(FILE *stream_out, char *label, webgeomdata *webgi){
  int i, offset = 0, nverts_max=0;

  fprintf(stream_out, "\n\n// %s\n\n", label);

  if(webgi->nframes>0){
    nverts_max = webgi->framesizes[0];
    for(i=1;i<webgi->nframes;i++){
      nverts_max = MAX(nverts_max,webgi->framesizes[i]);
    }
    fprintf(stream_out, "         var nframes = %i;\n",         webgi->nframes);
    fprintf(stream_out, "         var n%s_frames = %i;\n",      webgi->type, webgi->nframes);
    fprintf(stream_out, "         var show_%s          = 1;\n", webgi->type);
    fprintf(stream_out, "         var %s_file_ready    = 1;\n", webgi->type);
  }
  else{
    fprintf(stream_out, "         var show_%s          = 0;\n", webgi->type);
    fprintf(stream_out, "         var %s_file_ready    = 0;\n", webgi->type);
  }

  fprintf(stream_out, "         var %s_sizes = [\n", webgi->type);
  for(i = 0;i<webgi->nframes-1;i++){
    fprintf(stream_out, "%i,", webgi->framesizes[i]);
    if(i%PER_ROW==(PER_ROW-1))fprintf(stream_out, "\n");
  }
  if(webgi->nframes>0)fprintf(stream_out, "%i\n", webgi->framesizes[webgi->nframes-1]);
  fprintf(stream_out, "         ];\n");

  fprintf(stream_out, "         var %s_offsets = [\n", webgi->type);
  for(i = 0;i<webgi->nframes-1;i++){
    fprintf(stream_out, "%i,", offset);
    if(i%PER_ROW==(PER_ROW-1))fprintf(stream_out, "\n");
    offset += webgi->framesizes[i];
  }
  if(webgi->nframes>0)fprintf(stream_out, "%i\n", offset);
  fprintf(stream_out, "         ];\n");

  fprintf(stream_out, "         var vertices_%s = [\n", webgi->type);
  for(i = 0; i<nverts_max-1; i++){
    char varlabel[100];

    sprintf(varlabel, "%.3f", webgi->verts[i]);
    TrimZeros(varlabel);
    fprintf(stream_out, "%s,", varlabel);
    if(i%PER_ROW==(PER_ROW-1))fprintf(stream_out, "\n");
  }
  if(nverts_max>0){
    char varlabel[100];

    sprintf(varlabel, "%.3f", webgi->verts[nverts_max-1]);
    TrimZeros(varlabel);
    fprintf(stream_out, "%s\n", varlabel);
  }
  fprintf(stream_out, "         ];\n");

  fprintf(stream_out, "         var colors_%s = [\n", webgi->type);
  fprintf(stream_out, "         ];\n");

  fprintf(stream_out, "         var vertices_%s_data = [\n", webgi->type);
  for(i = 0; i<webgi->nverts-1; i++){
    char varlabel[100];

    sprintf(varlabel, "%.3f", webgi->verts[i]);
    TrimZeros(varlabel);
    fprintf(stream_out, "%s,", varlabel);
    if(i%PER_ROW==(PER_ROW-1))fprintf(stream_out, "\n");
  }
  if(webgi->nverts>0){
    char varlabel[100];

    sprintf(varlabel, "%.3f", webgi->verts[webgi->nverts-1]);
    TrimZeros(varlabel);
    fprintf(stream_out, "%s\n", varlabel);
  }
  fprintf(stream_out, "         ];\n");

  fprintf(stream_out, "         var colors_%s_data = [\n", webgi->type);
  for(i = 0; i<webgi->nverts-1; i++){
    fprintf(stream_out, "%.3f,",webgi->colors[i]);
    if(i%PER_ROW==(PER_ROW-1))fprintf(stream_out, "\n");
  }
  if(webgi->nverts>0)fprintf(stream_out, "%.3f\n",webgi->colors[webgi->nverts-1]);
  fprintf(stream_out, "         ];\n");

  fprintf(stream_out, "         var indices_%s = [\n", webgi->type);
  for(i = 0; i<nverts_max-1; i++){
    fprintf(stream_out, "%i,", i);
    if(i%PERBIN_ROW==(PERBIN_ROW-1))fprintf(stream_out, "\n");
  }
  if(nverts_max>0)fprintf(stream_out, "%i\n", nverts_max-1);
  fprintf(stream_out, "         ];\n");

  fprintf(stream_out, "\n");
}

/* ------------------ Smv2Html ------------------------ */

int Smv2Html(char *html_file, int option, int from_where){
  FILE *stream_in = NULL, *stream_out;
  float *vertsObstLit, *normalsObstLit, *colorsObstLit;
  int nvertsObstLit, *facesObstLit, nfacesObstLit;
  float *vertsGeomLit, *normalsGeomLit, *colorsGeomLit;
  int nvertsGeomLit, *facesGeomLit, nfacesGeomLit;
  int have_blockages = 0, have_geometry = 0;

  float *vertsLine, *colorsLine;
  int nvertsLine, *facesLine, nfacesLine;

  char html_fullfile[1024];
  int copy_html, i;
  webgeomdata slice_node_web, slice_cell_web, slice_geom_web, bndf_node_web, part_node_web;

  stream_in = fopen(smokeview_html, "r");
  if(stream_in==NULL){
    printf("***error: smokeview html template file %s failed to open\n", smokeview_html);
    return 1;
  }

  if(from_where==FROM_SCRIPT){
    strcpy(html_fullfile, html_file);
  }
  else{
    int return_val;

    return_val = GetHtmlFileName(html_fullfile, option);
    if(return_val==1){
      fclose(stream_in);
      return 1;
    }
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

  InitWebgeom(&slice_node_web, "slice_node");
  InitWebgeom(&slice_cell_web, "slice_cell");
  InitWebgeom(&slice_geom_web, "slice_geom");
  InitWebgeom(&bndf_node_web,  "bndf_node");
  InitWebgeom(&part_node_web,  "part");

  SliceNodeTriangles2Geom(&slice_node_web, option);
  SliceCellTriangles2Geom(&slice_cell_web, option);
  SliceGeomTriangles2Geom(&slice_geom_web, option);
  BndfNodeTriangles2Geom( &bndf_node_web,  option);
  PartNodeVerts2Geom(     &part_node_web,  option);

  ObstLitTriangles2Geom(&vertsObstLit, &normalsObstLit, &colorsObstLit, &nvertsObstLit, &facesObstLit, &nfacesObstLit);
  GeomLitTriangles2Geom(&vertsGeomLit, &normalsGeomLit, &colorsGeomLit, &nvertsGeomLit, &facesGeomLit, &nfacesGeomLit);
  Lines2Geom(&vertsLine, &colorsLine, &nvertsLine, &facesLine, &nfacesLine);

  for(i = 0;i<nmeshes;i++){
    meshdata *meshi;

    meshi = meshinfo+i;
    if(meshi->nbptrs>0){
      have_blockages = 1;
      break;
    }
  }
  if(nopaque_triangles>0||ntransparent_triangles>0){
    have_geometry = 1;
  }

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
      fprintf(stream_out, "<button onclick = \"Reset()\">Reset View </button>\n");
	  fprintf(stream_out, "<button onclick = \"EnterVR()\">Enter VR </button>\n");
	  fprintf(stream_out, "<br>\n");

      //show/hide scene elements

      if(have_blockages==1)fprintf(stream_out, "<button onclick = \"show_blockages=ShowHide(show_blockages)\">blockages</button>\n");
      if(have_geometry==1)fprintf(stream_out, "<button onclick = \"show_geom=ShowHide(show_geom)\">geometry</button>\n");
      fprintf(stream_out, "<button onclick = \"show_outlines=ShowHide(show_outlines)\">outlines</button><br>\n");
      if(slice_node_web.nverts>0){
        have_data = 1;
        fprintf(stream_out, "<button onclick = \"show_slice_node=ShowHide(show_slice_node)\">slice(node centered)</button>\n");
      }
      if(slice_cell_web.nverts>0){
        have_data = 1;
        fprintf(stream_out, "<button onclick=\"show_slice_cell=ShowHide(show_slice_cell)\">slice(cell centered)</button>\n");
      }
      if(slice_geom_web.nverts>0){
        have_data = 1;
        fprintf(stream_out, "<button onclick = \"show_slice_geom=ShowHide(show_slice_geom)\">slice(geom)</button>\n");
      }
      if(bndf_node_web.nverts>0){
        have_data = 1;
        fprintf(stream_out, "<button onclick = \"show_bndf_node=ShowHide(show_bndf_node)\">boundary(node centered)</button>\n");
      }
      if(part_node_web.nverts>0){
        have_data = 1;
        fprintf(stream_out, "<button onclick = \"show_part=ShowHide(show_part)\">particle</button>\n");
      }
      if(have_data==1)fprintf(stream_out, "<br>\n");

      //time stepping controls

// buttons need to be defined for all cases for now
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

      fprintf(stream_out, "         var part_file_ready     = 0;\n");
      fprintf(stream_out, "         var show_part           = 0;\n");
      fprintf(stream_out, "\n");
      fprintf(stream_out, "         var show_outlines       = 1;\n");
      fprintf(stream_out, "         var show_blockages      = 1;\n");
      fprintf(stream_out, "         var show_geom           = 1;\n\n");

      fprintf(stream_out, "         const texture_colorbar_data = new Uint8Array([\n");
      for(i = 0; i<255; i++){
        int ii[3];

        ii[0] = CLAMP(255*rgb_slice[4*i+0], 0, 255);
        ii[1] = CLAMP(255*rgb_slice[4*i+1], 0, 255);
        ii[2] = CLAMP(255*rgb_slice[4*i+2], 0, 255);
        fprintf(stream_out, "%i,%i,%i,255,", ii[0], ii[1], ii[2]);
        if(i%PERCOLOR_ROW==(PERCOLOR_ROW-1))fprintf(stream_out, "\n");
      }
      {
        int ii[3];

        i=255;
        ii[0] = CLAMP(255*rgb_slice[4*i+0], 0, 255);
        ii[1] = CLAMP(255*rgb_slice[4*i+1], 0, 255);
        ii[2] = CLAMP(255*rgb_slice[4*i+2], 0, 255);
        fprintf(stream_out, "%i,%i,%i,255\n", ii[0], ii[1], ii[2]);
      }
      fprintf(stream_out, "         ]);\n");
      fprintf(stream_out, "         const texture_colorbar_numcolors = 256;\n");

      OutputFixedFrame(stream_out, "node centered slice files", &slice_node_web);
      OutputFixedFrame(stream_out, "cell centered slice files", &slice_cell_web);
      OutputFixedFrame(stream_out, "geometry slice files",      &slice_geom_web);
      OutputFixedFrame(stream_out, "boundary files",            &bndf_node_web);
      OutputVariableFrame(stream_out, "particle files",         &part_node_web);

      // add obst triangles
      fprintf(stream_out, "\n\n//  blockages\n\n");
      fprintf(stream_out, "         var vertices_obst_lit = [\n");
      for(i = 0; i<nvertsObstLit-1; i++){
        char label[100];

        sprintf(label, "%f", vertsObstLit[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1))fprintf(stream_out, "\n");
      }
      if(nvertsObstLit>0){
        char label[100];

        sprintf(label, "%f", vertsObstLit[nvertsObstLit-1]);
        TrimZeros(label);
        fprintf(stream_out, "%s\n", label);
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var normals_obst_lit = [\n");
      for(i = 0; i<nvertsObstLit-1; i++){
        char label[100];

        sprintf(label, "%f", normalsObstLit[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1))fprintf(stream_out, "\n");
      }
      if(nvertsObstLit>0){
        char label[100];

        sprintf(label, "%f", normalsObstLit[nvertsObstLit-1]);
        TrimZeros(label);
        fprintf(stream_out, "%s\n", label);
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var colors_obst_lit = [\n");
      for(i = 0; i<nvertsObstLit-1; i++){
        char label[100];

        sprintf(label, "%f", colorsObstLit[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1))fprintf(stream_out, "\n");
      }
      if(nvertsObstLit>0){
        char label[100];

        sprintf(label, "%f", colorsObstLit[nvertsObstLit-1]);
        TrimZeros(label);
        fprintf(stream_out, "%s\n", label);
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var indices_obst_lit = [\n");
      for(i = 0; i<nfacesObstLit-1; i++){
        fprintf(stream_out, "%i,", facesObstLit[i]);
        if(i%PERBIN_ROW==(PERBIN_ROW-1))fprintf(stream_out, "\n");
      }
      if(nfacesObstLit>0){
        fprintf(stream_out, "%i\n", facesObstLit[nfacesObstLit-1]);
      }
      fprintf(stream_out, "         ];\n");

      // add geom triangles
      fprintf(stream_out, "\n\n//  geometry\n\n");
      fprintf(stream_out, "         var vertices_geom_lit = [\n");
      for(i = 0; i<nvertsGeomLit-1; i++){
        char label[100];

        sprintf(label, "%f", vertsGeomLit[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1))fprintf(stream_out, "\n");
      }
      if(nvertsGeomLit>0){
        char label[100];

        sprintf(label, "%f", vertsGeomLit[nvertsGeomLit-1]);
        TrimZeros(label);
        fprintf(stream_out, "%s\n", label);
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var normals_geom_lit = [\n");
      for(i = 0; i<nvertsGeomLit-1; i++){
        char label[100];

        sprintf(label, "%f", normalsGeomLit[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1))fprintf(stream_out, "\n");
      }
      if(nvertsGeomLit>0){
        char label[100];

        sprintf(label, "%f", normalsGeomLit[nvertsGeomLit-1]);
        TrimZeros(label);
        fprintf(stream_out, "%s\n", label);
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var colors_geom_lit = [\n");
      for(i = 0; i<nvertsGeomLit-1; i++){
        char label[100];

        sprintf(label, "%f", colorsGeomLit[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1))fprintf(stream_out, "\n");
      }
      if(nvertsGeomLit>0){
        char label[100];

        sprintf(label, "%f", colorsGeomLit[nvertsGeomLit-1]);
        TrimZeros(label);
        fprintf(stream_out, "%s\n", label);
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var indices_geom_lit = [\n");
      for(i = 0; i<nfacesGeomLit-1; i++){
        fprintf(stream_out, "%i,", facesGeomLit[i]);
        if(i%PERBIN_ROW==(PERBIN_ROW-1))fprintf(stream_out, "\n");
      }
      if(nfacesGeomLit>0){
        fprintf(stream_out, "%i\n", facesGeomLit[nfacesGeomLit-1]);
      }
      fprintf(stream_out, "         ];\n");

      // add lines
      fprintf(stream_out, "\n\n//  lines \n\n");
      fprintf(stream_out, "         var vertices_line = [\n");
      for(i = 0; i<nvertsLine-1; i++){
        char label[100];

        sprintf(label, "%f", vertsLine[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1))fprintf(stream_out, "\n");
      }
      if(nvertsLine>0){
        char label[100];

        sprintf(label, "%f", vertsLine[nvertsLine-1]);
        TrimZeros(label);
        fprintf(stream_out, "%s\n", label);
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var colors_line = [\n");
      for(i = 0; i<nvertsLine-1; i++){
        char label[100];

        sprintf(label, "%f", colorsLine[i]);
        TrimZeros(label);
        fprintf(stream_out, "%s,", label);
        if(i%PER_ROW==(PER_ROW-1))fprintf(stream_out, "\n");
      }
      if(nvertsLine>0){
        char label[100];

        sprintf(label, "%f", colorsLine[nvertsLine-1]);
        TrimZeros(label);
        fprintf(stream_out, "%s\n", label);
      }
      fprintf(stream_out, "         ];\n");

      fprintf(stream_out, "         var indices_line = [\n");
      for(i = 0; i<nfacesLine-1; i++){
        fprintf(stream_out, "%i,", facesLine[i]);
        if(i%PERBIN_ROW==(PERBIN_ROW-1))fprintf(stream_out, "\n");
      }
      if(nfacesLine>0){
        fprintf(stream_out, "%i\n", facesLine[nfacesLine-1]);
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
