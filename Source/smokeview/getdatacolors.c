#include "options.h"
#include "glew.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "smokeviewvars.h"
#include "glui_bounds.h"

#define EXPMIN -1
#define EXPMAX 3

/* ------------------ GetBoundaryColors ------------------------ */

void GetBoundaryColors(float *t, int nt, unsigned char *it,
              int settmin, float *ttmin, int settmax, float *ttmax,
              float *tmin_arg, float *tmax_arg,
              int ndatalevel, int nlevel,
              char **labels, char *scale, float *tvals256,
              int *extreme_min, int *extreme_max
              ){
  int n;
  float *tcopy, factor, tval, range;
  int expmin, expmax;
  int itt;
  float local_tmin, local_tmax, tmin2, tmax2;

  tmin2 = *t;
  tmax2 = *t;

  STRCPY(scale,"");
  tcopy = t+1;
  for(n=1;n<nt;n++){
    if(*tcopy<tmin2)tmin2=*tcopy;
    if(*tcopy>tmax2)tmax2=*tcopy;
    tcopy++;
  }
  *tmin_arg = tmin2;
  *tmax_arg = tmax2;
  *extreme_min=0;
  *extreme_max=0;
  if(settmin!=SET_MIN){
    *ttmin=tmin2;
  }
  if(settmax!=SET_MAX){
    *ttmax=tmax2;
  }
  local_tmin = *ttmin;
  local_tmax = *ttmax;

  range = local_tmax - local_tmin;
  factor = 0.0f;
  if(range!=0.0f)factor = (ndatalevel-2*extreme_data_offset)/range;
  for(n=0;n<nt;n++){
    float val;

    val = *t;

    if(val<local_tmin){
      itt=0;
      *extreme_min=1;
    }
    else if(val>local_tmax){
      itt=ndatalevel-1;
      *extreme_max=1;
    }
    else{
      itt=extreme_data_offset+(int)(factor*(val-local_tmin));
    }
    *it++ = CLAMP(itt, colorbar_offset, ndatalevel - 1 - colorbar_offset);
    t++;
  }
  GetMantissaExponent(local_tmax, &expmax);
  GetMantissaExponent(local_tmin, &expmin);
  if(expmin!=0&&expmax!=0&&expmax-expmin<=2&&(expmin<-2||expmin>2)){
    local_tmin *= pow((double)10.0,(double)-expmin);
    local_tmax *= pow((double)10.0,(double)-expmin);
    sprintf(scale,"*10^%i",expmin);
  }
  if(expmin==0&&(expmax<EXPMIN||expmax>EXPMAX)){
    local_tmin *= pow((double)10.0,(double)-expmax);
    local_tmax *= pow((double)10.0,(double)-expmax);
    sprintf(scale,"*10^%i",expmax);
  }
  if(expmax==0&&(expmin<EXPMIN||expmin>EXPMAX)){
    local_tmin *= pow((double)10.0,(double)-expmin);
    local_tmax *= pow((double)10.0,(double)-expmin);
    sprintf(scale,"*10^%i",expmin);
  }
  range = local_tmax - local_tmin;
  factor = range/(nlevel-2);
  for(n=1;n<nlevel-2;n++){
    tval = local_tmin + (n-1)*factor;
    Num2String(&labels[n][0],tval);
  }
  tval = local_tmin + (nlevel-3)*factor;
  for(n=0;n<256;n++){
    tvals256[n] = (local_tmin*(255-n) + n*local_tmax)/255.;
  }
  Num2String(&labels[nlevel-2][0],tval);
  tval = local_tmax;
  Num2String(&labels[nlevel-1][0],tval);
}

/* ------------------ WriteBoundIni ------------------------ */

void WriteBoundIni(void){
  FILE *stream = NULL;
  char *fullfilename = NULL;
  int i;

  if(fullfilename == NULL)return;

  for(i = 0; i < global_scase.npatchinfo; i++){
    bounddata *boundi;
    patchdata *patchi;
    int skipi;
    int j;

    skipi = 0;
    patchi = global_scase.patchinfo + i;
    if(patchi->bounds.defined == 0)continue;
    for(j = 0; j < i - 1; j++){
      patchdata *patchj;

      patchj = global_scase.patchinfo + j;
      if(patchi->shortlabel_index == patchj->shortlabel_index&&patchi->patch_filetype == patchj->patch_filetype){
        skipi = 1;
        break;
      }
    }
    if(skipi == 1)continue;

    boundi = &patchi->bounds;
    if(stream == NULL){
      stream = fopen(fullfilename, "w");
      if(stream == NULL){
        FREEMEMORY(fullfilename);
        return;
      }
    }
    float dummy = 0.0;
    fprintf(stream, "B_BOUNDARY\n");
    fprintf(stream, " %f %f %f %f %i %s\n", boundi->global_min, dummy, dummy, boundi->global_max, patchi->patch_filetype, patchi->label.shortlabel);
  }
  if(stream != NULL)fclose(stream);
  FREEMEMORY(fullfilename);
}

/* ------------------ GetBoundaryColors3 ------------------------ */

void GetBoundaryColors3(patchdata *patchi, float *t, int start, int nt, unsigned char *it,
              float *ttmin, float *ttmax,
              int nlevel,
              char **patchlabels, float *patchvalues, float *tvals256,
              int *extreme_min, int *extreme_max, int flag
              ){
  int n;
  float factor, tval, range;
  int itt;
  float new_tmin, new_tmax;

  int set_valmin, set_valmax;
  char *label;

  label = patchi->label.shortlabel;
  GLUIGetMinMax(BOUND_PATCH, label, &set_valmin, ttmin, &set_valmax, ttmax);
  new_tmin = *ttmin;
  new_tmax = *ttmax;

  if(flag==1){
    CheckMemory;
    range = new_tmax-new_tmin;
    factor = 0.0f;
    if(range!=0.0f)factor = (float)(255-2*extreme_data_offset)/range;

    t += start;
    it += start;
    for(n = start; n<nt; n++){
      float val;

      val = *t++;

      if(val<new_tmin){
        itt = 0;
        *extreme_min = 1;
      }
      else if(val>new_tmax){
        itt = 255;
        *extreme_max = 1;
      }
      else{
        itt = extreme_data_offset+(int)(factor*(val-new_tmin));
      }
      *it++ = CLAMP(itt, colorbar_offset, 255-colorbar_offset);
    }
  }
  CheckMemory;
  range = new_tmax - new_tmin;
  factor = range/(nlevel-2);
  for(n=1;n<nlevel-2;n++){
    tval = new_tmin + (n-1)*factor;
    Num2String(&patchlabels[n][0],tval);
    patchvalues[n] = tval;
  }
  tval = new_tmin + (nlevel-3)*factor;
  for(n=0;n<256;n++){
    tvals256[n] = (new_tmin*(255-n) + n*new_tmax)/255.;
  }
  Num2String(&patchlabels[nlevel-2][0],tval);
  patchvalues[nlevel-2] = tval;
  tval = new_tmax;
  Num2String(&patchlabels[nlevel-1][0],tval);
  patchvalues[nlevel-1] = tval;
}

/* ------------------ UpdateAllBoundaryColors ------------------------ */

void UpdateAllBoundaryColors(int flag){
  int i, *list = NULL, nlist = 0;

  if(global_scase.npatchinfo==0)return;
  NewMemory((void **)&list, global_scase.npatchinfo*sizeof(int));
  nlist = 0;
  for(i = 0; i<global_scase.npatchinfo; i++){
    meshdata *meshi;
    patchdata *patchi;

    patchi = global_scase.patchinfo+i;
    if(patchi->loaded==0)continue;
    switch(patchi->patch_filetype){
      case PATCH_STRUCTURED_NODE_CENTER:
      case PATCH_STRUCTURED_CELL_CENTER:
        meshi = global_scase.meshescoll.meshinfo+patchi->blocknumber;
        if(meshi->patchval==NULL||meshi->cpatchval==NULL)continue;
        list[nlist++] = i;
        break;
      case PATCH_GEOMETRY_BOUNDARY:
        if(patchi->geom_vals==NULL)continue;
        list[nlist++] = i;
        break;
      case PATCH_GEOMETRY_SLICE:
        break;
      default:
        assert(FFALSE);
        break;
    }
  }
  if(nlist>0){
    SetLoadedPatchBounds(list, nlist);
    for(i = 0; i<nlist; i++){
      patchdata *patchi;

      patchi = global_scase.patchinfo+list[i];
      if(patchi->loaded==1){
        int set_valmin, set_valmax;
        float valmin, valmax;
        char *label;

        label = patchi->label.shortlabel;
        GLUIGetMinMax(BOUND_PATCH, label, &set_valmin, &valmin, &set_valmax, &valmax);
        switch(patchi->patch_filetype){
          case PATCH_STRUCTURED_NODE_CENTER:
          case PATCH_STRUCTURED_CELL_CENTER:
            if(patchi->blocknumber>=0){
              meshdata *meshi;
              int npatchvals;

              meshi = global_scase.meshescoll.meshinfo+patchi->blocknumber;
              npatchvals = patchi->ntimes*meshi->npatchsize;
              GetBoundaryColors3(patchi, meshi->patchval, 0, npatchvals, meshi->cpatchval,
                                 &glui_patchmin, &glui_patchmax,
                                 global_scase.nrgb, colorlabelpatch, colorvaluespatch, boundarylevels256,
                                 &patchi->extreme_min, &patchi->extreme_max, flag);
            }
            break;
          case PATCH_GEOMETRY_BOUNDARY:
          case PATCH_GEOMETRY_SLICE:
            GetBoundaryColors3(patchi, patchi->geom_vals, 0, patchi->geom_nvals, patchi->geom_ivals,
                               &valmin, &valmax,
                               global_scase.nrgb, colorlabelpatch, colorvaluespatch, boundarylevels256,
                               &patchi->extreme_min, &patchi->extreme_max, flag);
            break;
          default:
            assert(FFALSE);
            break;
        }
      }
    }
  }
  FREEMEMORY(list);
}

/* ------------------ GetBoundaryLabels ------------------------ */

void GetBoundaryLabels(
              float local_tmin, float local_tmax,
              char **boundarylabels, float *boundaryvalues, float *tvals256, int nlevel){
  int n;
  float factor, tval, range;

  range = local_tmax - local_tmin;
  factor = range/(nlevel-2);
  for(n=1;n<nlevel-2;n++){
    tval = local_tmin + (n-1)*factor;
    Num2String(&boundarylabels[n][0],tval);
    boundaryvalues[n] = tval;
  }
  tval = local_tmin + (nlevel-3)*factor;
  for(n=0;n<256;n++){
    tvals256[n] = (local_tmin*(255-n) + n*local_tmax)/255.;
  }
  Num2String(&boundarylabels[nlevel-2][0],tval);
  boundaryvalues[nlevel-2] = tval;
  tval = local_tmax;
  Num2String(&boundarylabels[nlevel-1][0],tval);
  boundaryvalues[nlevel-1] = tval;
}

/* ------------------ UpdatePart5Extremes ------------------------ */

void UpdatePart5Extremes(void){
  int i;

  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    propi->extreme_max=0;
    propi->extreme_min=0;
  }
}

/* ------------------ GetPartColors ------------------------ */

void GetPartColors(partdata *parti, int nlevel, int flag){
  int i;
  part5data *datacopy;
  // float *diameter_data;
  float *length_data, *azimuth_data, *elevation_data;
  float *u_vel_data, *v_vel_data, *w_vel_data;

  if(parti->stream==NULL){
    printf("***warning: particle data in %s was unloaded, colors not updated\n",parti->file);
    return;
  }

  datacopy = parti->data5;

  int num, num2;
  int *part_set_valmin, *part_set_valmax;
  float *part_valmin, *part_valmax;

  num = GLUIGetNValtypes(BOUND_PART);
  NewMemory((void **)&part_set_valmin, num*sizeof(int));
  NewMemory((void **)&part_valmin,     num*sizeof(float));
  NewMemory((void **)&part_set_valmax, num*sizeof(int));
  NewMemory((void **)&part_valmax,     num*sizeof(float));
  GLUIGetMinMaxAll(BOUND_PART, part_set_valmin, part_valmin, part_set_valmax, part_valmax, &num2);

  int start=0;
  if(flag==0)start = parti->ntimes+1;// skip particle conversion if flag is 0
  for(i=start;i<parti->ntimes;i++){
    int j;

    for(j=0;j<parti->nclasses;j++){
      float valmin, valmax, dval;
      partclassdata *partclassi;
      float *rvals;
      unsigned char *irvals;
      float *dsx, *dsy, *dsz;
      int local_flag, k;

      partclassi = parti->partclassptr[j];
      rvals = datacopy->rvals;
      irvals = datacopy->irvals;
 // caused problems with coloring - might need in some form if crashes stil occur
      if(rvals==NULL || irvals==NULL || datacopy->npoints_file==0){
        datacopy++;
        continue;
      }
      for(k=2;k<partclassi->ntypes;k++){
        partpropdata *prop_id;

        if(datacopy->npoints_file ==0)continue;
        prop_id = GetPartProp(partclassi->labels[k].longlabel);
        if(prop_id==NULL)continue;

          {
          int prop_id_index;
          int m;

          prop_id_index = prop_id-part5propinfo;
          valmin = part_valmin[prop_id_index];
          valmax = part_valmax[prop_id_index];
          dval = valmax - valmin;
          if(dval<=0.0)dval=1.0;

          for(m = 0; m<datacopy->npoints_file; m++){
            float val;
            int irval;

            val = *rvals++;
            irval = extreme_data_offset+(float)(255-2*extreme_data_offset)*(val-valmin)/dval;
            *irvals++ = CLAMP(irval, 0, 255);
          }
        }
      }
      //** do some data conversion if the right data columns are present
      azimuth_data=NULL;
//      diameter_data=NULL;
      elevation_data=NULL;
      length_data=NULL;
      u_vel_data=NULL;
      v_vel_data=NULL;
      w_vel_data=NULL;

      if(partfast==NO){
        if(partclassi->col_azimuth>=0){
          azimuth_data = datacopy->rvals+partclassi->col_azimuth*datacopy->npoints_file;
        }
        if(partclassi->col_diameter>=0){
          // diameter_data=datacopy->rvals+partclassi->col_diameter*datacopy->npoints_file;
        }
        if(partclassi->col_elevation>=0){
          elevation_data = datacopy->rvals+partclassi->col_elevation*datacopy->npoints_file;
        }
        if(partclassi->col_length>=0){
          length_data = datacopy->rvals+partclassi->col_length*datacopy->npoints_file;
        }
        if(partclassi->col_u_vel>=0){
          u_vel_data = datacopy->rvals+partclassi->col_u_vel*datacopy->npoints_file;
        }
        if(partclassi->col_v_vel>=0){
          v_vel_data = datacopy->rvals+partclassi->col_v_vel*datacopy->npoints_file;
        }
        if(partclassi->col_w_vel>=0){
          w_vel_data = datacopy->rvals+partclassi->col_w_vel*datacopy->npoints_file;
        }
        local_flag = 0;
        if(azimuth_data!=NULL&&elevation_data!=NULL&&length_data!=NULL){
          int m;

          local_flag = 1;
          dsx = datacopy->dsx;
          dsy = datacopy->dsy;
          dsz = datacopy->dsz;
          for(m = 0; m<datacopy->npoints_file; m++){
            float az, elev, length;

            az = azimuth_data[m]*DEG2RAD;
            elev = elevation_data[m]*DEG2RAD;
            length = SCALE2SMV(length_data[m]);
            dsx[m] = cos(az)*cos(elev)*length/2.0;
            dsy[m] = sin(az)*cos(elev)*length/2.0;
            dsz[m] = sin(elev)*length/2.0;
          }
        }
        if(u_vel_data!=NULL&&v_vel_data!=NULL&&w_vel_data!=NULL){
          float denom;
          int m;
          partpropdata *prop_U, *prop_V, *prop_W;

          prop_U = GetPartProp(partclassi->labels[partclassi->col_u_vel+2].longlabel);
          prop_V = GetPartProp(partclassi->labels[partclassi->col_v_vel+2].longlabel);
          prop_W = GetPartProp(partclassi->labels[partclassi->col_w_vel+2].longlabel);
          if(prop_U!=NULL&&prop_V!=NULL&&prop_W!=NULL){
            float umax, vmax, wmax;

            umax = MAX(ABS(prop_U->valmin), ABS(prop_U->valmax));
            vmax = MAX(ABS(prop_V->valmin), ABS(prop_V->valmax));
            wmax = MAX(ABS(prop_W->valmin), ABS(prop_W->valmax));

            denom = sqrt(umax*umax+vmax*vmax+wmax*wmax);
            if(denom==0.0)denom = 1.0;
          }
          else{
            denom = 1.0;
          }

          local_flag = 1;
          dsx = datacopy->dsx;
          dsy = datacopy->dsy;
          dsz = datacopy->dsz;
          for(m = 0; m<datacopy->npoints_file; m++){
            dsx[m] = 0.05*u_vel_data[m]/denom;
            dsy[m] = 0.05*v_vel_data[m]/denom;
            dsz[m] = 0.05*w_vel_data[m]/denom;
          }
        }
        if(local_flag==0){
          FREEMEMORY(datacopy->dsx);
          FREEMEMORY(datacopy->dsy);
          FREEMEMORY(datacopy->dsz);
        }
      }
      datacopy++;
    }
  }

  // erase data memory in a separate loop (so all "columns" are available when doing any conversions)

  for(i=0;i<npart5prop;i++){
    int n;
    partpropdata *propi;
    float local_tmin, local_tmax;
    float factor,range,tval;
    float *vals;
    float *ppartlevels256;

    propi = part5propinfo + i;

    local_tmin = part_valmin[i];
    local_tmax = part_valmax[i];
    vals       = propi->partlabelvals;
    ppartlevels256=propi->ppartlevels256;

    range = local_tmax - local_tmin;

    factor = range/(nlevel-2);
    for(n=1;n<nlevel-2;n++){
      tval = local_tmin + (n-1)*factor;
      vals[n] = tval;
    }
    for(n=0;n<256;n++){
      ppartlevels256[n] = (local_tmin*(255-n) + n*local_tmax)/255.;
    }
    tval = local_tmin + (nlevel-3)*factor;
    vals[nlevel-2] = tval;

    tval = local_tmax;
    vals[nlevel-1] = tval;
    CheckMemory;
  }
  FREEMEMORY(part_set_valmin);
  FREEMEMORY(part_valmin);
  FREEMEMORY(part_set_valmax);
  FREEMEMORY(part_valmax);
}

/* ------------------ GetZoneColor ------------------------ */

int GetZoneColor(float t, float local_tmin, float local_tmax, int nlevel){
  int level;

  if(t<=local_tmin)return 0;
  if(t>=local_tmax)return nlevel-1;
  if(local_tmin==local_tmax)return 0;
  level=nlevel*(t-local_tmin)/(local_tmax-local_tmin);
  if(level<0)return 0;
  if(level>nlevel-1)return nlevel-1;
  return level;
}


/* ------------------ GetZoneColors ------------------------ */

void GetZoneColors(const float *t, int nt, unsigned char *it,
               float ttmin, float ttmax, int nlevel, int nlevel_full,
               char **zonelabels, float zonevalues[12], float *tvals256
               ){
  int n;
  float dt, factor;
  int itt;
  float local_tmin, local_tmax;
  float range;
  float tval;

  if(t==NULL||it==NULL)return;
  local_tmin = ttmin;
  local_tmax = ttmax;

  dt = local_tmax - local_tmin;
  factor=0.0f;
  if(dt!=0.0f)factor = (nlevel_full-2*extreme_data_offset)/dt;
  for(n=0;n<nt;n++){
    float val;

    val=*t;
    if(val<local_tmin){
      itt=0;
    }
    else if(val>local_tmax){
      itt=nlevel_full-1;
    }
    else{
      itt=extreme_data_offset+(int)(factor*(val-local_tmin));
    }
    *it++=CLAMP(itt,colorbar_offset,nlevel_full-1-colorbar_offset);
    t++;
  }

  range = local_tmax - local_tmin;
  factor = range/(nlevel-2);
  for(n=1;n<nlevel-2;n++){
    tval = local_tmin + (n-1)*factor;
    zonevalues[n] = tval;
    Num2String(&zonelabels[n][0],tval);
  }
  tval = local_tmin + (nlevel-3)*factor;
  for(n=0;n<256;n++){
    tvals256[n] = (local_tmin*(255-n) + n*local_tmax)/255.;
  }
  zonevalues[nlevel-2] = tval;
  Num2String(&zonelabels[nlevel-2][0],tval);
  tval = local_tmax;
  zonevalues[nlevel-1] = tval;
  Num2String(&zonelabels[nlevel-1][0],tval);
}

/* ------------------ GetPlot3DColors ------------------------ */

void GetPlot3DColors(int plot3dvar, float *ttmin, float *ttmax,
              int ndatalevel, int nlevel,
              char **labels,char **labelsiso,float *tlevels, float *tlevels256,
              int *extreme_min, int *extreme_max, int flag
              ){
  int n;
  float dt, factor, tval;
  float local_tmin, local_tmax;
  float range;
  float tminorig, tmaxorig, dtorig;
  int itt;
  float *q;
  unsigned char *iq;
  plot3ddata *p;
  meshdata *meshi;
  int i;
  int ntotal;

  local_tmin = *ttmin;
  local_tmax = *ttmax;
  tminorig = local_tmin;
  tmaxorig = local_tmax;

  if(flag==1){
    range = local_tmax-local_tmin;
    if(range!=0.0f){
      factor = (float)(ndatalevel-2*extreme_data_offset)/range;
    }
    else{
      factor = 0.0f;
    }

    for(i = 0; i<global_scase.nplot3dinfo; i++){
      p = global_scase.plot3dinfo+i;
      if(p->loaded==0||p->display==0)continue;
      meshi = global_scase.meshescoll.meshinfo+p->blocknumber;
      ntotal = (meshi->ibar+1)*(meshi->jbar+1)*(meshi->kbar+1);

      if(meshi->qdata!=NULL){
        q  = meshi->qdata  + plot3dvar*ntotal;
        iq = meshi->iqdata + plot3dvar*ntotal;
        for(n = 0; n<ntotal; n++){
          float val;

          val = *q;
          if(val<local_tmin){
            itt = 0;
            *extreme_min = 1;
          }
          else if(val>local_tmax){
            itt = ndatalevel-1;
            *extreme_max = 1;
          }
          else{
            itt = extreme_data_offset+(int)(factor*(val-local_tmin));
          }
          *iq++ = CLAMP(itt, colorbar_offset, ndatalevel-1-colorbar_offset);
          q++;
        }
      }
    }
  }

  range = local_tmax-local_tmin;
  dt = range/(float)(nlevel-1);
  dtorig = (tmaxorig-tminorig)/(float)(nlevel-1);
  for(n=0;n<nlevel-1;n++){
    tval = local_tmin + n*dt;
    Num2String(&labels[n][0],tval);
    colorvaluesp3[plot3dvar][n] = tval;
    Num2String(&labelsiso[n][0],tval+dt/2.0);
  }
  for(n=0;n<256;n++){
    tlevels256[n] = (local_tmin*(255-n) + local_tmax*n)/255.;
  }
  tval = local_tmax;
  Num2String(&labels[nlevel-1][0],tval);
  colorvaluesp3[plot3dvar][nlevel-1] = tval;
  Num2String(&labelsiso[nlevel-1][0],tval);

  for(n=0;n<nlevel;n++){
    tlevels[n]=tminorig+(float)n*dtorig;
  }

  if(flag==1){
    for(i = 0; i<global_scase.nplot3dinfo; i++){
      p = global_scase.plot3dinfo+i;
      if(p->loaded==0||p->display==0)continue;
      meshi = global_scase.meshescoll.meshinfo+p->blocknumber;
      ntotal = (meshi->ibar+1)*(meshi->jbar+1)*(meshi->kbar+1);

      if(meshi->qdata==NULL){
        float qval, *qvals;

        qvals = p3levels256[plot3dvar];
        iq = meshi->iqdata+plot3dvar*ntotal;
        for(n = 0; n<ntotal; n++){
          qval = qvals[*iq];
          itt = (int)(factor*(qval-local_tmin));
          if(itt<0)itt = 0;
          if(itt>ndatalevel-1)itt = ndatalevel-1;
          *iq++ = itt;
        }
      }
    }
  }
}

/* ------------------ UpdateAllPlot3DColors ------------------------ */

void UpdateAllPlot3DColors(int flag){
  int i, updated=0;

  for(i = 0; i < global_scase.nplot3dinfo; i++){
    plot3ddata *plot3di;
    int errorcode;

    plot3di = global_scase.plot3dinfo + i;
    if(plot3di->loaded == 1){
      UpdatePlot3DColors(plot3di, flag, &errorcode);
      updated = 1;
    }
  }
  if(updated==1&&flag==1){
    UpdatePlotSlice(XDIR);
    UpdatePlotSlice(YDIR);
    UpdatePlotSlice(ZDIR);
  }
}

/* ------------------ MakeColorLabels ------------------------ */

void MakeColorLabels(char colorlabels[12][11], float colorvalues[12], float tmin_arg, float tmax_arg, int nlevel){
  float range, dt;
  int n;

  range = tmax_arg-tmin_arg;
  dt = range/(float)(nlevel-2);

  for(n=1;n<nlevel;n++){
    float tval;

    tval = tmin_arg + (n-1)*dt;
    colorvalues[n] = tval;
    Float2String(&colorlabels[n][0], tval, ncolorlabel_digits, force_fixedpoint);
  }
}

/* ------------------ UpdateSliceColors ------------------------ */

void UpdateSliceColors(int last_slice){
  int ii, error;

  for(ii = 0; ii<nslice_loaded; ii++){
    int i;
    slicedata *sd;

    i = slice_loaded_list[ii];
    sd = global_scase.slicecoll.sliceinfo+i;
    if(sd->vloaded==0&&sd->display==0)continue;
    if(sd->slicefile_labelindex==slicefile_labelindex){
      int set_slicecolor;

      set_slicecolor = DEFER_SLICECOLOR;
      if(i==last_slice){
        set_slicecolor = SET_SLICECOLOR;
        sd->finalize = 1;
      }
      ReadSlice("", i, ALL_FRAMES, NULL, RESETBOUNDS, set_slicecolor, &error);
    }
  }
}

/* ------------------ UpdateSliceBounds2 ------------------------ */

void UpdateSliceBounds2(void){
  int ii, error;

  for(ii = 0; ii<nslice_loaded; ii++){
    int i;
    slicedata *sd;
    int set_valmin, set_valmax;
    float qmin, qmax;

    i = slice_loaded_list[ii];
    sd = global_scase.slicecoll.sliceinfo+i;
    if(sd->display==0)continue;
    GLUIGetMinMax(BOUND_SLICE, sd->label.shortlabel, &set_valmin, &qmin, &set_valmax, &qmax);
    sd->valmin_slice      = qmin;
    sd->valmax_slice      = qmax;
    sd->globalmin_slice   = qmin;
    sd->globalmax_slice   = qmax;
    SetSliceColors(qmin, qmax, sd, 0, &error);
  }
  for(ii = 0; ii<global_scase.slicecoll.nvsliceinfo; ii++){
    vslicedata *vd;
    slicedata *sd;
    int set_valmin, set_valmax;
    float qmin, qmax;

    vd = global_scase.slicecoll.vsliceinfo+ii;
    if(vd->loaded==0||vd->display==0||vd->ival==-1)continue;
    sd = global_scase.slicecoll.sliceinfo+vd->ival;
    GLUIGetMinMax(BOUND_SLICE, sd->label.shortlabel, &set_valmin, &qmin, &set_valmax, &qmax);
    sd->valmin_slice    = qmin;
    sd->valmax_slice    = qmax;
    sd->globalmin_slice = qmin;
    sd->globalmax_slice = qmax;
    SetSliceColors(qmin, qmax, sd, 0, &error);
  }
}

/* ------------------ GetSliceColors ------------------------ */

void GetSliceColors(const float *t, int nt, unsigned char *it,
              float local_tmin, float local_tmax,
              int ndatalevel, int nlevel,
              char colorlabels[12][11], float colorvalues[12], float *tlevels256,
              int *extreme_min, int *extreme_max, int flag
              ){
  int n;
  float factor, tval;
  float range;
  int itt;

  range = local_tmax-local_tmin;
  *extreme_min=0;
  *extreme_max=0;
  if(range!=0.0f){
    factor = (float)(ndatalevel-2*extreme_data_offset)/range;
  }
  else{
    factor = 0.0f;
  }
  if(flag==1){
    for(n=0;n<nt;n++){
      float val;

      val = *t;

      if(val<local_tmin){
        itt=0;
        *extreme_min=1;
      }
      else if(val>local_tmax){
        itt=ndatalevel-1;
        *extreme_max=1;
      }
      else{
        itt=extreme_data_offset+(int)(factor*(val-local_tmin));
      }
      *it++ = CLAMP(itt, colorbar_offset, ndatalevel - 1 - colorbar_offset);
      t++;
    }
  }

  MakeColorLabels(colorlabels, colorvalues, local_tmin, local_tmax, nlevel);

  for(n=0;n<256;n++){
    tlevels256[n] = (local_tmin*(255-n) + local_tmax*n)/255.;
  }
  tval = local_tmax;
  colorvalues[nlevel-1] = tval;
  Float2String(&colorlabels[nlevel-1][0], tval, ncolorlabel_digits, force_fixedpoint);
}

/* ------------------ GetColorbarLabels ------------------------ */

void GetColorbarLabels(float local_tmin, float local_tmax, int nlevel,
              char labels[12][11], float *tlevels256){
  int n;
  float dt, tval;
  float range;

  range = local_tmax-local_tmin;
  dt = range/(float)(nlevel-2);
  for(n=1;n<nlevel-1;n++){
    tval = local_tmin + (n-1)*dt;
    Num2String(&labels[n][0],tval);
  }
  for(n=0;n<256;n++){
    tlevels256[n] = (local_tmin*(255-n) + local_tmax*n)/255.;
  }
  tval = local_tmax;
  Num2String(&labels[nlevel-1][0],tval);
}

/* ------------------ InitCadColors ------------------------ */

void InitCadColors(void){
  int n, i1, i2, i;
  float xx, f1, f2, sum;
  switch(setbw){
   case 0:
    for(n=0;n<nrgb_cad;n++){
      xx = (float)n/(float)nrgb_cad * (float)(global_scase.nrgb-1);
      i1 = (int)xx;
      i2 = (int)(xx+1);
      f2 = xx - (float)i1;
      f1 = 1.0f - f2;
      sum=0.0;
      for(i=0;i<3;i++){
        rgb_cad[n][i] = f1*global_scase.rgb[i1][i] + f2*global_scase.rgb[i2][i];
        sum += rgb_cad[n][i]*rgb_cad[n][i];
      }
      sum=sqrt((double)sum);
      if(sum>0.0){
        for(i=0;i<3;i++){
          rgb_cad[n][i] /= sum;
        }
      }
      rgb_cad[n][3]=1.0;
    }
    break;
   case 1:
    for(n=0;n<nrgb_cad;n++){
      xx = (float)n/(float)nrgb_cad;
      for(i=0;i<3;i++){
        rgb_cad[n][i] = xx;
      }
      rgb_cad[n][3]=1.0;
    }
    break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ UpdateTexturebar ------------------------ */

void UpdateTexturebar(void){
  SNIFF_ERRORS("UpdateTexturebar - start");
  if(use_graphics==0)return;
  glBindTexture(GL_TEXTURE_1D, terrain_colorbar_id);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, rgb_terrain2);
  SNIFF_ERRORS("UpdateTexturebar - glTexImage1D (rgb_terrain2) ");

  glBindTexture(GL_TEXTURE_1D, texture_colorbar_id);
  glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,256,0,GL_RGBA,GL_FLOAT,rgb_full);
  SNIFF_ERRORS("UpdateTexturebar - glTexImage1D (rgb_full) ");

  glBindTexture(GL_TEXTURE_1D,texture_slice_colorbar_id);
  glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,256,0,GL_RGBA,GL_FLOAT,rgb_slice);
  SNIFF_ERRORS("UpdateTexturebar - glTexImage1D (rgb_slice) ");

  glBindTexture(GL_TEXTURE_1D,texture_patch_colorbar_id);
  glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,256,0,GL_RGBA,GL_FLOAT,rgb_patch);
  SNIFF_ERRORS("UpdateTexturebar - glTexImage1D (rgb_patch) ");

  glBindTexture(GL_TEXTURE_1D,texture_plot3d_colorbar_id);
  glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,256,0,GL_RGBA,GL_FLOAT,rgb_plot3d);
  SNIFF_ERRORS("UpdateTexturebar - glTexImage1D (rgb_plot3d) ");

  glBindTexture(GL_TEXTURE_1D,texture_iso_colorbar_id);
  glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,256,0,GL_RGBA,GL_FLOAT,rgb_iso);
  SNIFF_ERRORS("UpdateTexturebar - glTexImage1D (rgb_iso) ");

  glBindTexture(GL_TEXTURE_1D,slicesmoke_colormap_id);
  glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,MAXSMOKERGB,0,GL_RGBA,GL_FLOAT,rgb_slicesmokecolormap_01);
  SNIFF_ERRORS("UpdateTexturebar - glTexImage1D (rgb_slicesmokecolormap_01) ");

  glBindTexture(GL_TEXTURE_1D,volsmoke_colormap_id);
  glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,MAXSMOKERGB,0,GL_RGBA,GL_FLOAT,rgb_volsmokecolormap);
  SNIFF_ERRORS("UpdateTexturebar - glTexImage1D (rgb_volsmokecolormap) ");

#ifdef pp_GPU
  if(gpuactive==1&&nvolrenderinfo>0&&showvolrender==1){
    glActiveTexture(GL_TEXTURE2);
    glTexSubImage1D(GL_TEXTURE_1D,0,0,MAXSMOKERGB,GL_RGBA,GL_FLOAT, rgb_volsmokecolormap);
    SNIFF_ERRORS("UpdateTexturebar - glTexSubImage1D (rgb_volsmokecolormap) ");
    glActiveTexture(GL_TEXTURE0);
  }
  if(gpuactive==1&&SHOW_gslice_data==1&& slice3d_colormap_id_defined==1){
    glActiveTexture(GL_TEXTURE4);
    glTexSubImage1D(GL_TEXTURE_1D,0,0,256,GL_RGBA,GL_FLOAT, rgb_slice);
    SNIFF_ERRORS("updatecolors after glTexSubImage1D (rgb_slice)");
    glActiveTexture(GL_TEXTURE0);
  }
#endif

}

/* ------------------ InitRGB ------------------------ */

void InitRGB(void){
  int n;
  float transparent_level_local=1.0;

  if(use_transparency_data==1)transparent_level_local=transparent_level;

  if(setbw==0){
    ConvertColor(TO_COLOR);
    if(nrgb_ini > 0){
      global_scase.nrgb = nrgb_ini;
      for(n=0;n<nrgb_ini;n++){
        global_scase.rgb[n][0] = rgb_ini[n*3];
        global_scase.rgb[n][1] = rgb_ini[n*3+1];
        global_scase.rgb[n][2] = rgb_ini[n*3+2];
        global_scase.rgb[n][3] = transparent_level_local;
      }
    }
    else{
      for(n=0;n<global_scase.nrgb;n++){
        global_scase.rgb[n][0] = rgb_base[n][0];
        global_scase.rgb[n][1] = rgb_base[n][1];
        global_scase.rgb[n][2] = rgb_base[n][2];
        global_scase.rgb[n][3] = transparent_level_local;
      }
    }
  }
  else{
    ConvertColor(TO_BW);
    for(n=0;n<global_scase.nrgb;n++){
      global_scase.rgb[n][0] = bw_base[n][0];
      global_scase.rgb[n][1] = bw_base[n][1];
      global_scase.rgb[n][2] = bw_base[n][2];
      global_scase.rgb[n][3] = transparent_level_local;
    }
  }
}

/* ------------------ UpdateCO2Colormap ------------------------ */

void UpdateCO2Colormap(void){
  int n;
  float transparent_level_local=1.0;
  float *co2_cb;
  float *rgb_colormap=NULL;

  if(use_transparency_data==1)transparent_level_local=transparent_level;

  co2_cb = colorbars.colorbarinfo[colorbars.co2_colorbar_index].colorbar_rgb;
  rgb_colormap = rgb_sliceco2colormap_01;

  switch(co2_colormap_type){
    case CO2_RGB:
      for(n=0;n<MAXSMOKERGB;n++){
        rgb_colormap[4*n+0] = (float)co2_color_int255[0] / 255.0;
        rgb_colormap[4*n+1] = (float)co2_color_int255[1] / 255.0;
        rgb_colormap[4*n+2] = (float)co2_color_int255[2] / 255.0;
        rgb_colormap[4*n+3] = transparent_level_local;
      }
      break;
    case CO2_COLORBAR:
      for(n=0;n<MAXSMOKERGB;n++){
        rgb_colormap[4*n+0] = co2_cb[3*n+0];
        rgb_colormap[4*n+1] = co2_cb[3*n+1];
        rgb_colormap[4*n+2] = co2_cb[3*n+2];
        rgb_colormap[4*n+3] = transparent_level_local;
      }
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ UpdateSmokeColormap ------------------------ */

void UpdateSmokeColormap(int option){
  int n;
  float transparent_level_local=1.0;
  unsigned char *alpha_rgb;
  float *fire_cb;
  float valmin=0.0, valmax=1.0;
  float valmin_cb=0.0, valmax_cb=1.0;
  float *rgb_colormap=NULL;

  rgb_colormap = rgb_slicesmokecolormap_01;
  if(have_fire==HRRPUV_index){
    valmin    = global_scase.hrrpuv_min;
    valmin_cb = global_hrrpuv_cb_min;
    valmax    = global_scase.hrrpuv_max;
    valmax_cb = global_hrrpuv_cb_max;
  }
  if(have_fire==TEMP_index){
    valmin    = global_scase.temp_min;
    valmin_cb = global_temp_cb_min;
    valmax    = global_scase.temp_max;
    valmax_cb = global_temp_cb_max;
  }

  if(use_transparency_data==1)transparent_level_local=transparent_level;

  alpha_rgb = colorbars.colorbarinfo[colorbartype].colorbar_alpha;
  fire_cb = colorbars.colorbarinfo[colorbars.fire_colorbar_index].colorbar_rgb;

  assert(have_fire==NO_FIRE||have_fire==HRRPUV_index||have_fire==TEMP_index);
  switch(fire_colormap_type){
    case FIRECOLOR_RGB:

      for(n=0;n<MAXSMOKERGB;n++){
        int use_smoke;

        use_smoke = 1;
        if(have_fire==HRRPUV_index||have_fire==TEMP_index){
          float val;

          val = valmin + (float)n*(valmax-valmin)/(float)(MAXSMOKERGB-1);
          if(val>=valmin_cb){
            use_smoke = 0;
          }
        }
        if(use_smoke==1){
          rgb_colormap[4*n+0] = (float)smoke_color_int255[0]/255.0;
          rgb_colormap[4*n+1] = (float)smoke_color_int255[1]/255.0;
          rgb_colormap[4*n+2] = (float)smoke_color_int255[2]/255.0;
        }
        else{
          rgb_colormap[4*n+0]=(float)fire_color_int255[0]/255.0;
          rgb_colormap[4*n+1]=(float)fire_color_int255[1]/255.0;
          rgb_colormap[4*n+2]=(float)fire_color_int255[2]/255.0;
        }
        if(alpha_rgb[n]==0){
          rgb_colormap[4*n+3]=0.0;
        }
        else{
          rgb_colormap[4*n+3]=transparent_level_local;
        }
      }
      break;
    case FIRECOLOR_COLORBAR:
      if(have_fire == NO_FIRE)break;
      for(n=0;n<MAXSMOKERGB;n++){
        int use_smoke;
        float val;

        use_smoke = 1;
        if(have_fire==HRRPUV_index||have_fire==TEMP_index){
          val = valmin + (float)n*(valmax-valmin)/(float)(MAXSMOKERGB-1);
          if(val>valmin_cb){
            use_smoke = 0;
          }
        }
        if(use_smoke==1){
          rgb_colormap[4*n+0] = (float)smoke_color_int255[0]/255.0;
          rgb_colormap[4*n+1] = (float)smoke_color_int255[1]/255.0;
          rgb_colormap[4*n+2] = (float)smoke_color_int255[2]/255.0;
        }
        else{
          int cb_index;
          float *fire1;

          cb_index = global_cb_min_index + (float)(global_cb_max_index - global_cb_min_index)*(val-valmin_cb)/(valmax_cb-valmin_cb);
          cb_index = CLAMP(cb_index,global_cb_min_index,global_cb_max_index);
          fire1 = fire_cb + 3*cb_index;
          rgb_colormap[4*n]   = fire1[0];
          rgb_colormap[4*n+1] = fire1[1];
          rgb_colormap[4*n+2] = fire1[2];
        }
        if(alpha_rgb[n]==0){
          rgb_colormap[4*n+3]=0.0;
        }
        else{
          rgb_colormap[4*n+3]=transparent_level_local;
        }
      }
      break;
    default:
      assert(FFALSE);
      break;
  }
  UpdateTexturebar();
}

/* ------------------ UpdateRGBColors ------------------------ */

void UpdateRGBColors(int colorbar_index){

  int n,nn;
  int i;
  float *rgb2ptr;
  int cci;
  meshdata *meshi;
  int vent_offset, outline_offset;
  facedata *facej;
  float transparent_level_local=1.0;

  if(use_transparency_data==1)transparent_level_local=transparent_level;

  InitCadColors();
  InitRGB();
  nrgb_full = MAXRGB;
  for(n=0;n<nrgb_full;n++){
    rgb_trans[4*n]=0.0;
    rgb_trans[4*n+1]=0.0;
    rgb_trans[4*n+2]=0.0;
    rgb_trans[4*n+3]=(float)n/(float)(nrgb_full-1);
  }
  if(colorbars.colorbarinfo!=NULL){
    unsigned char *alpha_rgb;
    colorbardata *cbi;

    cbi = colorbars.colorbarinfo + colorbartype;

    alpha_rgb = colorbars.colorbarinfo[colorbartype].colorbar_alpha;
    for(n=0;n<nrgb_full;n++){
      rgb_full[n][0]=cbi->colorbar_rgb[3*n];
      rgb_full[n][1]=cbi->colorbar_rgb[3*n+1];
      rgb_full[n][2]=cbi->colorbar_rgb[3*n+2];
      if(alpha_rgb[n]==0){
        rgb_full[n][3]=0.0;
      }
      else{
        rgb_full[n][3]=transparent_level_local;
      }
    }
    UpdateSmokeColormap(RENDER_SLICE);
    UpdateSmokeColormap(RENDER_VOLUME);
  }
  else{
    for(n=0;n<nrgb_full;n++){
      rgb_full[n][0]=(float)n/(float)(nrgb_full);
      rgb_full[n][1]=(float)n/(float)(nrgb_full);
      rgb_full[n][2]=(float)n/(float)(nrgb_full);
      rgb_full[n][3]=transparent_level_local;
    }
  }
  if(contour_type==LINE_CONTOURS){
    for(n=0;n<nrgb_full;n++){
      rgb_full2[n][3]=rgb_full[n][3];
      rgb_full[n][3]=0;
    }

    int width;
    width = ( int )(256.0 / 10.0 );
    for(n=0;n<11;n++){
      int cbmin, cbmax, j;

      cbmin = CLAMP(n * width, 0, 255);
      cbmax = CLAMP(cbmin + colorbar_linewidth, 0, 255);
      cbmin = CLAMP(cbmax - colorbar_linewidth, 0, 255);
      for(j = cbmin; j <= cbmax; j++){
        rgb_full[j][3] = rgb_full2[j][3];
      }
    }
  }
  if(contour_type==STEPPED_CONTOURS){
    int index[11];

    for(n=0;n<10;n++){
      index[n]=n*25.4;
    }
    index[10]=nrgb_full;
    for(n=0;n<10;n++){
      int mid;

      mid = (index[n]+index[n+1])/2;
      for(i=index[n];i<index[n+1];i++){
        rgb_full[i][0]=rgb_full[mid][0];
        rgb_full[i][1]=rgb_full[mid][1];
        rgb_full[i][2]=rgb_full[mid][2];
        rgb_full[i][3]=rgb_full[mid][3];
      }
    }
  }
  if(colorbar_flip==1){
    {
      int nnn;

      for(n=0;n<nrgb_full;n++){
        rgb_full2[n][0]=rgb_full[n][0];
        rgb_full2[n][1]=rgb_full[n][1];
        rgb_full2[n][2]=rgb_full[n][2];
        rgb_full2[n][3]=rgb_full[n][3];
      }
      for(n=0;n<nrgb_full;n++){
        nnn=nrgb_full-1-n;
        rgb_full[nnn][0]=rgb_full2[n][0];
        rgb_full[nnn][1]=rgb_full2[n][1];
        rgb_full[nnn][2]=rgb_full2[n][2];
        rgb_full[nnn][3]=rgb_full2[n][3];
      }
    }
  }
  global_colorbar_index=colorbar_index;
  if(colorbar_index>=0){
    float highlight_black[3]={0.0,0.0,0.0},highlight_red[3]={1.0,0.0,0.0},*highlight_color;
    int cbmin, cbmax;

    valindex = global_colorbar_index;
    if(valindex<0)valindex=0;
    if(valindex>255)valindex=255;
    cci = colorbar_index;
    if(ABS(colorbar_shift-1.0)>0.0001){
      cci = SHIFT_VAL(cci, 0, 255, colorbar_shift);
    }

    if(setbw==1){
      highlight_color=highlight_red;
    }
    else{
      highlight_color=highlight_black;
    }
    cbmin = cci-colorbar_selection_width;
    cbmax = cci+colorbar_selection_width;
    if(cbmin<0){
      cbmax = cbmax - cbmin;
      cbmin = 0;
    }
    if(cbmax>255){
      cbmin = cbmin - (cbmax-255);
      cbmax = 255;
    }
    for(n=cbmin;n<cbmax+1;n++){
      rgb_full[n][0]=highlight_color[0];
      rgb_full[n][1]=highlight_color[1];
      rgb_full[n][2]=highlight_color[2];
      rgb_full[n][3]=transparent_level_local;
    }
  }
  if(show_extreme_mindata==1){
    rgb_full[0][0]=rgb_below_min[0]/255.0;
    rgb_full[0][1]=rgb_below_min[1]/255.0;
    rgb_full[0][2]=rgb_below_min[2]/255.0;
  }
  if(show_extreme_maxdata==1){
    rgb_full[255][0]=rgb_above_max[0]/255.0;
    rgb_full[255][1]=rgb_above_max[1]/255.0;
    rgb_full[255][2]=rgb_above_max[2]/255.0;
  }
  if(rgb2_ini!=NULL){
    rgb2ptr=rgb2_ini;
  }
  else{
    rgb2ptr=&(rgb2[0][0]);
  }
  if(colorbar_index!=0){
    for(n=0;n<global_scase.nrgb;n++){
      nn=n*(nrgb_full-1)/(global_scase.nrgb-1);
      global_scase.rgb[n][0] = rgb_full[nn][0];
      global_scase.rgb[n][1] = rgb_full[nn][1];
      global_scase.rgb[n][2] = rgb_full[nn][2];
      global_scase.rgb[n][3] = transparent_level_local;
    }
  }
  for(n=global_scase.nrgb;n<global_scase.nrgb+global_scase.nrgb2;n++){
    global_scase.rgb[n][0]=rgb2ptr[3*(n-global_scase.nrgb)];
    global_scase.rgb[n][1]=rgb2ptr[3*(n-global_scase.nrgb)+1];
    global_scase.rgb[n][2]=rgb2ptr[3*(n-global_scase.nrgb)+2];
    global_scase.rgb[n][3]=transparent_level_local;
  }
  rgb_white=global_scase.nrgb;
  rgb_yellow=global_scase.nrgb+1;
  rgb_blue=global_scase.nrgb+2;
  rgb_red=global_scase.nrgb+3;

  float zero3[3]={0.0, 0.0, 0.0}, one3[3]={1.0, 1.0, 1.0};

  if(background_flip==0){
    memcpy(foregroundcolor,             foregroundbasecolor, 3*sizeof(float));
    memcpy(backgroundcolor,             backgroundbasecolor, 3*sizeof(float));
    memcpy(global_scase.rgb[rgb_white], one3,                3*sizeof(float));
    memcpy(global_scase.rgb[rgb_black], zero3,               3*sizeof(float));
  }
  else{
    memcpy(foregroundcolor,             backgroundbasecolor, 3*sizeof(float));
    memcpy(backgroundcolor,             foregroundbasecolor, 3*sizeof(float));
    memcpy(global_scase.rgb[rgb_white], zero3,               3*sizeof(float));
    memcpy(global_scase.rgb[rgb_black], one3,                3*sizeof(float));
  }
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    int j;

    meshi=global_scase.meshescoll.meshinfo + i;
    vent_offset = 6*meshi->nbptrs;
    outline_offset = vent_offset + meshi->nvents;
    if(meshi->faceinfo == NULL)continue;
    for(j=outline_offset;j<outline_offset+6;j++){
      facej = meshi->faceinfo + j;
      facej->color=foregroundcolor;
    }
  }
  UpdateChopColors();
  InitCadColors();
  UpdateTexturebar();
}

/* ------------------ UpdateChopColors ------------------------ */

void UpdateChopColors(void){
  int i;
  int ichopmin=0,ichopmax;
#define NCHOP 8
  float transparent_level_local=1.0;

  int   setpatchchopmin_local=0, setpatchchopmax_local=0;
  float patchchopmin_local=1.0, patchchopmax_local=0.0;

  int   glui_setslicechopmin_local=0, glui_setslicechopmax_local=0;
  float glui_slicechopmin_local=1.0, glui_slicechopmax_local=0.0;

  int setpartchopmin_local=0, setpartchopmax_local=0;
  float glui_partmin_local=1.0, glui_partmax_local=0.0;
  float partchopmin_local=1.0,  partchopmax_local=0.0;

  int setp3chopmin_temp_local=0, setp3chopmax_temp_local=0;
  float p3chopmin_temp_local=1.0, p3chopmax_temp_local=0.0;
  float glui_p3min_local=1.0, glui_p3max_local=0.0;


  cpp_boundsdata *bounds;

  SNIFF_ERRORS("UpdateChopColors: start");
  bounds                = GLUIGetBoundsData(BOUND_PATCH);
  if(bounds!=NULL){
    setpatchchopmin_local = bounds->set_chopmin;
    setpatchchopmax_local = bounds->set_chopmax;
    patchchopmin_local = bounds->chopmin;
    patchchopmax_local = bounds->chopmax;
  }

  bounds                     = GLUIGetBoundsData(BOUND_SLICE);
  if(bounds!=NULL){
    glui_setslicechopmin_local = bounds->set_chopmin;
    glui_setslicechopmax_local = bounds->set_chopmax;
    glui_slicechopmin_local = bounds->chopmin;
    glui_slicechopmax_local = bounds->chopmax;
  }

  bounds               = GLUIGetBoundsData(BOUND_PART);
  if(bounds!=NULL){
    setpartchopmin_local = bounds->set_chopmin;
    setpartchopmax_local = bounds->set_chopmax;
    partchopmin_local = bounds->chopmin;
    partchopmax_local = bounds->chopmax;
    glui_partmin_local = bounds->valmin[bounds->set_valmin];
    glui_partmax_local = bounds->valmax[bounds->set_valmax];
  }

  bounds                  = GLUIGetBoundsData(BOUND_PLOT3D);
  if(bounds!=NULL){
    setp3chopmin_temp_local = bounds->set_chopmin;
    setp3chopmax_temp_local = bounds->set_chopmax;
    p3chopmin_temp_local = bounds->chopmin;
    p3chopmax_temp_local = bounds->chopmax;
    glui_p3min_local = bounds->valmin[bounds->set_valmin];
    glui_p3max_local = bounds->valmax[bounds->set_valmax];
  }

  if(use_transparency_data==1)transparent_level_local=transparent_level;

  for(i=0;i<nrgb_full;i++){
    rgb_iso[4*i]=rgb_full[i][0];
    rgb_iso[4*i+1]=rgb_full[i][1];
    rgb_iso[4*i+2]=rgb_full[i][2];
    if(rgb_full[i][3]>0.001){
      rgb_iso[4*i+3]=transparent_level_local;
    }
    else{
      rgb_iso[4*i+3]=0.0;
    }

    rgb_slice[4*i]=rgb_full[i][0];
    rgb_slice[4*i+1]=rgb_full[i][1];
    rgb_slice[4*i+2]=rgb_full[i][2];
    if(rgb_full[i][3]>0.001){
      rgb_slice[4*i+3]=transparent_level_local;
    }
    else{
      rgb_slice[4*i+3]=0.0;
    }

    rgb_terrain2[4 * i] = rgb_full[i][0];
    rgb_terrain2[4 * i + 1] = rgb_full[i][1];
    rgb_terrain2[4 * i + 2] = rgb_full[i][2];
    if(rgb_full[i][3] > 0.001){
      rgb_terrain2[4 * i + 3] = transparent_level_local;
    }
    else{
      rgb_terrain2[4 * i + 3] = 0.0;
    }
    if(show_zlevel == 1){
      int ilevel;
      float dz;

      dz = (terrain_zmax - terrain_zmin)*geom_vert_exag;
      if(ABS(dz)<0.01)dz=1;

      ilevel = 255 * geom_vert_exag*(terrain_zlevel - terrain_zmin) / dz;
      if(ABS(ilevel - i) < 3){
        rgb_terrain2[4 * i] = 0;
        rgb_terrain2[4 * i + 1] = 0;
        rgb_terrain2[4 * i + 2] = 0;
      }
    }

    rgb_part[4 * i] = rgb_full[i][0];
    rgb_part[4*i+1]=rgb_full[i][1];
    rgb_part[4*i+2]=rgb_full[i][2];
    rgb_part[4*i+3]=rgb_full[i][3];
    if(rgb_full[i][3]>0.001){
      rgb_part[4*i+3]=transparent_level_local;
    }
    else{
      rgb_part[4*i+3]=0.0;
    }

    rgb_plot3d[4*i]=rgb_full[i][0];
    rgb_plot3d[4*i+1]=rgb_full[i][1];
    rgb_plot3d[4*i+2]=rgb_full[i][2];
    rgb_plot3d[4*i+3]=rgb_full[i][3];
    if(rgb_full[i][3]>0.001){
      rgb_plot3d[4*i+3]=transparent_level_local;
    }
    else{
      rgb_plot3d[4*i+3]=0.0;
    }

    rgb_patch[4*i]=rgb_full[i][0];
    rgb_patch[4*i+1]=rgb_full[i][1];
    rgb_patch[4*i+2]=rgb_full[i][2];
    if(rgb_full[i][3]>0.001){
      rgb_patch[4*i+3]=transparent_level_local;
    }
    else{
      rgb_patch[4*i+3]=0.0;
    }
  }
  if(showall_3dslices==1){
    int slice3d_loaded = 0;

    for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
      slicedata *slicei;

      slicei = global_scase.slicecoll.sliceinfo + i;
      if(slicei->volslice==1&&slicei->loaded==1&&slicei->display==1){
        slice3d_loaded = 1;
        break;
      }
    }
    if(slice3d_loaded==1){
      for(i = 0; i < nrgb_full; i++){
        float factor;

        factor = (float)i/255.0;
        rgb_slice[4 * i + 3] = transparent_level_local*factor*factor;
      }
    }
  }
  {
    float smin, smax;
    int chop_patch_local;

    smin = boundarylevels256[0];
    smax = boundarylevels256[255];

// make boundary opacities same as base colorbar opaque except when greater than chopmax or less than chopmin values
    if(setpatchchopmin_local==1){
      ichopmin=nrgb_full*(patchchopmin_local-smin)/(smax-smin);
      if(ichopmin<0)ichopmin=0;
      if(ichopmin>nrgb_full-1)ichopmin=nrgb_full-1;
      for(i=0;i<ichopmin;i++){
        rgb_patch[4*i+3]=0.0;
      }
      for(i=ichopmin-NCHOP;i<ichopmin;i++){
        int ii;

        if(i<=0)continue;
        if(i>nrgb_full-1)continue;
        ii = i - (ichopmin-NCHOP);
        if(ii>NCHOP-1)continue;
        rgb_patch[4*i+3]=transparent_level_local*(float)ii/(float)(NCHOP-1);
      }
    }
    if(setpatchchopmax_local==1){
      ichopmax=nrgb_full*(patchchopmax_local - smin)/(smax-smin);
      if(ichopmax<0)ichopmax=0;
      if(ichopmax>nrgb_full-1)ichopmax=nrgb_full-1;
      for(i=ichopmax;i<nrgb_full;i++){
        rgb_patch[4*i+3]=0.0;
      }
      for(i=ichopmax;i<ichopmax+NCHOP;i++){
        int ii;

        if(i<=0)continue;
        if(i>nrgb_full-1)continue;
        ii = NCHOP-1-(i - ichopmax);
        if(ii>NCHOP-1)continue;
        rgb_patch[4*i+3]=transparent_level_local*(float)ii/(float)(NCHOP-1);
      }
    }
    chop_patch_local = 0;
    for(i = 0; i<nrgb_full; i++){
      if(rgb_patch[4*i+3]==0.0){
        chop_patch_local = 1;
        break;
      }
    }
    if(chop_patch != chop_patch_local){
      chop_patch      = chop_patch_local;
      updatefacelists = 1;
    }
  }
  if(slicebounds!=NULL&&slicefile_labelindex!=-1){
    float smin, smax;

    //smin=slicebounds[slicefile_labelindex].dlg_valmin;
    //smax=slicebounds[slicefile_labelindex].dlg_valmax;
    smin = colorbar_slice_min;
    smax = colorbar_slice_max;

    if(glui_setslicechopmin_local==1){
      ichopmin=nrgb_full*(glui_slicechopmin_local-smin)/(smax-smin);
      if(ichopmin<0)ichopmin=0;
      if(ichopmin>nrgb_full-1)ichopmin=nrgb_full-1;
      for(i=0;i<ichopmin;i++){
        rgb_slice[4*i+3]=0.0;
      }
      for(i=ichopmin-NCHOP;i<ichopmin;i++){
        int ii;

        if(i<=0)continue;
        if(i>nrgb_full-1)continue;
        ii = i - (ichopmin-NCHOP);
        if(ii>NCHOP-1)continue;
        rgb_slice[4*i+3]=transparent_level_local*(float)ii/(float)(NCHOP-1);
      }
    }
    if(glui_setslicechopmax_local==1){
      ichopmax=nrgb_full*(glui_slicechopmax_local - smin)/(smax-smin);
      if(ichopmax<0)ichopmax=0;
      if(ichopmax>nrgb_full-1)ichopmax=nrgb_full-1;
      for(i=ichopmax;i<nrgb_full;i++){
        rgb_slice[4*i+3]=0.0;
      }
      for(i=ichopmax;i<ichopmax+NCHOP;i++){
        int ii;

        if(i<=0)continue;
        if(i>nrgb_full-1)continue;
        ii = NCHOP-1-(i - ichopmax);
        if(ii>NCHOP-1)continue;
        rgb_slice[4*i+3]=transparent_level_local*(float)ii/(float)(NCHOP-1);
      }
    }
  }

  if(glui_partmax_local>glui_partmin_local){
    for(i = 0; i<nrgb_full; i++){
      rgb_part[4*i+3] = 1.0;
    }
    if(setpartchopmin_local==1){
      ichopmin = nrgb_full*(partchopmin_local-glui_partmin_local)/(glui_partmax_local-glui_partmin_local);
      if(ichopmin<0)ichopmin=0;
      if(ichopmin>nrgb_full-1)ichopmin=nrgb_full-1;
      for(i=0;i<ichopmin;i++){
        rgb_part[4*i+3]=0.0;
      }
    }
    if(setpartchopmax_local==1){
      ichopmax = nrgb_full*(partchopmax_local - glui_partmin_local)/(glui_partmax_local - glui_partmin_local);
      if(ichopmax<0)ichopmax=0;
      if(ichopmax>nrgb_full-1)ichopmax=nrgb_full-1;
      for(i=ichopmax;i<nrgb_full;i++){
        rgb_part[4*i+3]=0.0;
      }
    }
  }

  if(glui_p3max_local> glui_p3min_local){
    if(setp3chopmin_temp_local==1){
      ichopmin=nrgb_full*(p3chopmin_temp_local - glui_p3min_local)/(glui_p3max_local - glui_p3min_local);
      if(ichopmin<0)ichopmin=0;
      if(ichopmin>nrgb_full-1)ichopmin=nrgb_full-1;
      for(i=0;i<ichopmin;i++){
        rgb_plot3d[4*i+3]=0.0;
      }
      for(i=ichopmin-NCHOP;i<ichopmin;i++){
        int ii;

        if(i<=0)continue;
        if(i>nrgb_full-1)continue;
        ii = i - (ichopmin-NCHOP);
        if(ii>NCHOP-1)continue;
        rgb_plot3d[4*i+3]=transparent_level_local*(float)ii/(float)(NCHOP-1);
      }
      for(i = 0; i<global_scase.nrgb-2; i++){
        int ii;
        float factor;

        factor = 256.0/(float)(global_scase.nrgb-2);

        ii = factor*((float)i+0.5);
        if(ii>255)ii = 255;
        rgb_plot3d_contour[i] = rgb_plot3d + 4*ii;
      }
      rgb_plot3d_contour[global_scase.nrgb-2] = rgb_plot3d;
      rgb_plot3d_contour[global_scase.nrgb-1] = rgb_plot3d + 4*255;
    }
    if(setp3chopmax_temp_local==1){
      ichopmax=nrgb_full*(p3chopmax_temp_local - glui_p3min_local)/(glui_p3max_local - glui_p3min_local);
      if(ichopmax<0)ichopmax=0;
      if(ichopmax>nrgb_full-1)ichopmax=nrgb_full-1;
      for(i=ichopmax;i<nrgb_full;i++){
        rgb_plot3d[4*i+3]=0.0;
      }
      for(i=ichopmax;i<ichopmax+NCHOP;i++){
        int ii;

        if(i<=0)continue;
        if(i>nrgb_full-1)continue;
        ii = NCHOP-1-(i - ichopmax);
        if(ii>NCHOP-1)continue;
        rgb_plot3d[4*i+3]=transparent_level_local*(float)ii/(float)(NCHOP-1);
      }
    }
  }
  for(i=0;i<global_scase.npartinfo;i++){
    partdata *parti;

    parti = global_scase.partinfo + i;
    if(parti->loaded==0)continue;
    AdjustPart5Chops(); // only needs to be called once
    break;
  }
  UpdateTexturebar();
  ShiftColorbars();
}

/* ------------------ GetRGB ------------------------ */

void GetRGB(unsigned int val, unsigned char *rr, unsigned char *gg, unsigned char *bb){
  unsigned char r, g, b;

  r = val >> (ngreenbits+nbluebits);
  r = r << nredshift;

  g = val >> nbluebits;
  g = g&rgbmask[ngreenbits-1];
  g = g << ngreenshift;

  b = val&rgbmask[nbluebits-1];
  b = b << nblueshift;
  *rr=r; *gg=g; *bb=b;
}

/* ------------------ GetColorTranPtr ------------------------ */

float *GetColorTranPtr(float *color, float transparency){
  float col[4];

  col[0] = color[0];
  col[1] = color[1];
  col[2] = color[2];
  col[3] = transparency;
  return GetColorPtr(&global_scase, col);
}

  /* ------------------ ConvertColor ------------------------ */

void ConvertColor(int flag){
  colordata *colorptr;

  switch(flag){
   case TO_BW:
    for(colorptr=global_scase.firstcolor;colorptr!=NULL;colorptr=colorptr->nextcolor){
      colorptr->color[0]=colorptr->bw_color[0];
      colorptr->color[1]=colorptr->bw_color[1];
      colorptr->color[2]=colorptr->bw_color[2];
    }
    break;
   case TO_COLOR:
    for(colorptr=global_scase.firstcolor;colorptr!=NULL;colorptr=colorptr->nextcolor){
      colorptr->color[0]=colorptr->full_color[0];
      colorptr->color[1]=colorptr->full_color[1];
      colorptr->color[2]=colorptr->full_color[2];
    }
    break;
   default:
     assert(FFALSE);
     break;
  }
}
