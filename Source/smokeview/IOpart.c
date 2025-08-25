#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "glui_bounds.h"
#include "histogram.h"
#include "compress.h"
#include "IOobjects.h"
#include "stdio_m.h"

#define NXYZ_COMP_PART 3

#define LOADING 0
#define DRAWING 1

/* ------------------ ClosePartFiles ------------------------ */

void ClosePartFiles(void){
  int i;

  for(i = 0; i<global_scase.npartinfo; i++){
    partdata *parti;

    parti = global_scase.partinfo+i;
    if(parti->loaded==1&&parti->stream!=NULL){
      fclose_m(parti->stream);
      parti->stream = NULL;
    }
  }
}

/* ------------------ GetPartColor ------------------------ */

int GetPartColor(float **color_handle, part5data*datacopy, int show_default, int j, int itype,
                     float valmin, float valmax){
  float *colorptr;
  unsigned char *color;
  int showcolor;

  showcolor = 1;
  if(show_default == 1){
    colorptr = datacopy->partclassbase->rgb;
  }
  else{
    float *rvals;

    color = datacopy->irvals + itype*datacopy->npoints_file;
    rvals = datacopy->rvals + itype*datacopy->npoints_file;
    {
     int colorj;
      float rval;

      rval = CLAMP(255.0*(rvals[j]-valmin)/(valmax-valmin), 0.0, 255.0);
      colorj = rval;
      colorptr = rgb_part + 4*colorj;
    }
    if(current_property != NULL && (color[j] > current_property->imax || color[j] < current_property->imin))showcolor = 0;
  }
  *color_handle = colorptr;
  return showcolor;
}

/* ------------------ GetPartPropS ------------------------ */

partpropdata *GetPartPropS(char *label){
  int i;

  for(i = 0;i < npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(strcmp(propi->label->shortlabel, label) == 0)return propi;
  }
  return NULL;
}

/* ------------------ CopyDepVals ------------------------ */

void CopyDepVals(partclassdata *partclassi, part5data *datacopy, float *colorptr, propdata *prop, int j){
  int ii;
  int ndep_vals;
  float *dep_vals;

  if(prop == NULL)return;
  dep_vals = partclassi->fvars_dep;
  ndep_vals = partclassi->nvars_dep;
  for(ii = 0; ii < partclassi->nvars_dep - 3; ii++){

    unsigned char *var_type;
    unsigned char color_index;
    partpropdata *varprop;
    float valmin, valmax;
    char *shortlabel;
    flowlabels *label;

    shortlabel = NULL;
    varprop = NULL;
    label = datacopy->partclassbase->labels + ii + 2;
    if(label != NULL)shortlabel = label->shortlabel;
    if(shortlabel != NULL)varprop = GetPartPropS(shortlabel);
    if(varprop != NULL){
      var_type = datacopy->irvals + ii*datacopy->npoints_file;
      color_index = var_type[j];
      valmin = varprop->valmin;
      valmax = varprop->valmax;
      dep_vals[ii] = valmin + color_index*(valmax - valmin) / 255.0;
    }
    else{
      dep_vals[ii] = 1.0;
    }
  }

  dep_vals[ndep_vals - 3] = colorptr[0] * 255;
  dep_vals[ndep_vals - 2] = colorptr[1] * 255;
  dep_vals[ndep_vals - 1] = colorptr[2] * 255;
  prop->nvars_dep = partclassi->nvars_dep;
  prop->smv_object->visible = 1;
  for(ii = 0; ii < prop->nvars_dep; ii++){
    prop->fvars_dep[ii] = partclassi->fvars_dep[ii];
  }
  prop->nvars_dep = partclassi->nvars_dep;
  for(ii = 0; ii < partclassi->nvars_dep; ii++){
    prop->vars_dep_index[ii] = partclassi->vars_dep_index[ii];
  }
  prop->tag_number = datacopy->tags[j];
}

/* ------------------ CompareTags ------------------------ */

int CompareTags(const void *arg1, const void *arg2){
  int i, j;

  i = *(int *)arg1;
  j = *(int *)arg2;
  if(i < j)return -1;
  if(i > j)return 1;
  return 0;

}

/* ------------------ GetTagIndex ------------------------ */

int GetTagIndex(const partdata *partin_arg, part5data **datain_arg, int tagval_arg, int flag){
  int *returnval_local;
  part5data *data_local;
  int i;

  assert(sorting_tags == 0);
  if(sorting_tags == 1){
    printf("***error: particle tags accessed while being sorted\n");
    SMV_EXIT(1);
  }
  if(flag==LOADING&&partfast==YES)return -1;

  for(i = -1; i < global_scase.npartinfo; i++){
    const partdata *parti_local;

    if(i == -1){
      parti_local = partin_arg;
      data_local = *datain_arg;
    }
    else{
      parti_local = global_scase.partinfo + i;
      if(parti_local== partin_arg)continue;
      if(parti_local->loaded == 0 || parti_local->display == 0)continue;
      data_local = parti_local->data5 + (*datain_arg- partin_arg->data5);
    }
    if(parti_local->loaded == 0 || parti_local->display == 0)continue;

    if(data_local->npoints_file == 0)continue;
    assert(data_local->npoints_file>0);
    assert(data_local->sort_tags != NULL);
    returnval_local = bsearch(&tagval_arg, data_local->sort_tags, data_local->npoints_file, 2 * sizeof(int), CompareTags);
    if(returnval_local== NULL)continue;
    *datain_arg = data_local;
    return *(returnval_local+ 1);
  }
  return -1;
}

/* ------------------ GetPartBounds ------------------------ */

void GetPartBounds(float *valmin, float *valmax){
  *valmin = part5propinfo[global_prop_index].ppartlevels256[0];
  *valmax = part5propinfo[global_prop_index].ppartlevels256[255];
}

/* ------------------ DrawPart ------------------------ */

void DrawPart(const partdata *parti, int mode){
  int ipframe;
  part5data *datacopy, *datapast;
  int nclasses;
  int i, j;
  int offset_terrain;
  propdata *prop;
  float valmin, valmax;

  if(nglobal_times<1||parti->times[0] > global_times[itimes])return;
  if(global_scase.nterraininfo > 0 && ABS(vertical_factor - 1.0) > 0.01){
    offset_terrain = 1;
  }
  else{
    offset_terrain = 0;
  }

  if(current_property == NULL)return;
  GetPartBounds(&valmin, &valmax);
  if(valmin>=valmax){
    valmin = 0.0;
    valmax = 1.0;
  }
  ipframe = parti->itime;
  assert(ipframe>=0);
  if(ipframe < 0){
    ipframe = 0;
  } //xxx need to check this - why is ipframe < 0 ???
  nclasses = parti->nclasses;
  datacopy = parti->data5 + nclasses*ipframe;
  CheckMemory;
  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
  if(part5show == 1){
    if(streak5show == 0 || (streak5show == 1 && showstreakhead == 1)){
      for(i = 0;i < parti->nclasses;i++){
        float *xpos, *ypos, *zpos;
        unsigned char *vis, *color;
        partclassdata *partclassi;
        int partclass_index, itype, vistype, class_vis;
        int show_default;

        partclassi = parti->partclassptr[i];
        partclass_index = partclassi - global_scase.partclassinfo;

        vistype = current_property->class_present[partclass_index];
        class_vis = current_property->class_vis[partclass_index];

        if(vistype == 0 || datacopy->npoints_file <= 0 || (vistype == 1 && class_vis == 0)){
          if(show_tracers_always == 0 || partclassi->ntypes > 2){
            datacopy++;
            continue;
          }
        }
        itype = current_property->class_types[partclass_index];

        show_default = 0;
        if(itype == -1 || (show_tracers_always == 1 && partclassi->ntypes <= 2)){
          show_default = 1;
        }

        xpos = datacopy->xpos;
        ypos = datacopy->ypos;
        zpos = datacopy->zpos;
        vis = datacopy->vis_part;
        {
          glPointSize(partpointsize);
          if(offset_terrain == 0){

            // *** draw particles as points

            if(datacopy->partclassbase->vis_type == PART_POINTS){
              if(select_part == 1 && selected_part_index > 0 && mode == DRAWSCENE){
                float *rvals=NULL;

                if(itype>=0)rvals = datacopy->rvals + itype*datacopy->npoints_file;
                for(j = 0; j < datacopy->npoints_file; j += partdrawskip){
                  if(vis[j] == 1 && datacopy->tags[j]==selected_part_index){
                    char taglabel[64];


                    if(itype >= 0){
                      sprintf(taglabel, "%i: %f", selected_part_index, rvals[j]);
                    }
                    else{
                      sprintf(taglabel, "%i", selected_part_index);
                    }
                    Output3Text(foregroundcolor, xpos[j], ypos[j], zpos[j], taglabel);
                  }
                }
              }
              glBegin(GL_POINTS);
              if(show_default == 1){
                if(select_part == 0 || mode == DRAWSCENE){
                  glColor4fv(datacopy->partclassbase->rgb);
                  for(j = 0; j < datacopy->npoints_file; j += partdrawskip){
                    if(vis[j] == 1){
                      glVertex3f(xpos[j], ypos[j], zpos[j]);
                    }
                  }
                }
                else{
                  for(j = 0; j < datacopy->npoints_file; j += partdrawskip){
                    if(vis[j] == 1){
                      unsigned char r, g, b;

                      GetRGB((unsigned int)datacopy->tags[j], &r, &g, &b);
                      glColor3ub(r, g, b);
                      glVertex3f(xpos[j], ypos[j], zpos[j]);
                    }
                  }
                }
              }
              else{
                float *rvals;

                rvals = datacopy->rvals + itype * datacopy->npoints_file;
                if(select_part == 0 || mode == DRAWSCENE){
                  for(j = 0; j < datacopy->npoints_file; j += partdrawskip){
                    if(vis[j] == 1){
                      int colorj;
                      float rval;
                      rval = CLAMP(255.0 * (rvals[j] - valmin) / (valmax - valmin), 0.0, 255.0);
                      colorj = rval;
                      if(current_property != NULL && (colorj > current_property->imax || colorj < current_property->imin))continue;
                      if(rgb_part[4 * colorj + 3] == 0.0)continue;
                      glColor4fv(rgb_part + 4 * colorj);
                      glVertex3f(xpos[j], ypos[j], zpos[j]);
                    }
                  }
                }
                else{
                  for(j = 0; j < datacopy->npoints_file; j += partdrawskip){
                    if(vis[j] == 1){
                      int colorj;
                      float rval;
                      rval = CLAMP(255.0 * (rvals[j] - valmin) / (valmax - valmin), 0.0, 255.0);
                      colorj = rval;
                      if(current_property != NULL && (colorj > current_property->imax || colorj < current_property->imin))continue;
                      if(rgb_part[4 * colorj + 3] == 0.0)continue;
                      unsigned char r, g, b;

                      GetRGB((unsigned int)datacopy->tags[j], &r, &g, &b);
                      glColor3ub(r, g, b);
                      glVertex3f(xpos[j], ypos[j], zpos[j]);
                    }
                  }
                }
              }
              glEnd();
            }
            if(mode == SELECTOBJECT){
              datacopy++;
              continue;
            }

            // *** draw particles using smokeview object

            if(datacopy->partclassbase->vis_type == PART_SMV_DEVICE){
              for(j = 0;j < datacopy->npoints_file;j+=partdrawskip){
                float *colorptr;

                if(vis[j] != 1)continue;

                glPushMatrix();
                glTranslatef(xpos[j], ypos[j], zpos[j]);

                glRotatef(-datacopy->partclassbase->elevation, 0.0, 1.0, 0.0);
                glRotatef(datacopy->partclassbase->azimuth, 0.0, 0.0, 1.0);

                //  0->2   color
                //  3      diameter
                //  4      length

                if(show_default == 1){
                  colorptr = datacopy->partclassbase->rgb;
                }
                else{
                  float *rvals, rval;

                  rvals = datacopy->rvals+itype*datacopy->npoints_file;
                  rval = CLAMP(255.0*(rvals[j]-valmin)/(valmax-valmin), 0.0, 255.0);
                  colorptr = rgb_full[(int)rval];
                }
                prop = datacopy->partclassbase->prop;
                CopyDepVals(partclassi, datacopy, colorptr, prop, j);
                glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));

                partfacedir[0] = global_scase.xbar0 + SCALE2SMV(fds_eyepos[0]) - xpos[j];
                partfacedir[1] = global_scase.ybar0 + SCALE2SMV(fds_eyepos[1]) - ypos[j];
                partfacedir[2] = global_scase.zbar0 + SCALE2SMV(fds_eyepos[2]) - zpos[j];

                DrawSmvObject(prop->smv_object, 0, prop, 0, NULL, 0);
                glPopMatrix();
              }
            }

            // *** draw particle as lines

            if(datacopy->partclassbase->vis_type == PART_LINES
              && ((datacopy->dsx != NULL&&datacopy->dsy != NULL&&datacopy->dsz != NULL) || datacopy->partclassbase->device_name != NULL)
              ){
              float *dxv, *dyv, *dzv;
              float dx, dy, dz;
              int flag = 0;

              if(datacopy->dsx != NULL&&datacopy->dsy != NULL&&datacopy->dsz != NULL){
                flag = 1;
                dxv = datacopy->dsx;
                dyv = datacopy->dsy;
                dzv = datacopy->dsz;
              }
              else{
                dx = datacopy->partclassbase->dx;
                dy = datacopy->partclassbase->dy;
                dz = datacopy->partclassbase->dz;
              }
              glBegin(GL_LINES);
              if(show_default == 1){
                glColor4fv(datacopy->partclassbase->rgb);
                for(j = 0;j < datacopy->npoints_file;j+=partdrawskip){
                  if(vis[j] == 1){
                    if(flag == 1){
                      dx = dxv[j];
                      dy = dyv[j];
                      dz = dzv[j];
                    }
                    glVertex3f(xpos[j] - dx, ypos[j] - dy, zpos[j] - dz);
                    glVertex3f(xpos[j] + dx, ypos[j] + dy, zpos[j] + dz);
                  }
                }
              }
              else{
                color = datacopy->irvals + itype*datacopy->npoints_file;
                for(j = 0;j < datacopy->npoints_file;j+=partdrawskip){
                  if(vis[j] == 1){
                    glColor4fv(rgb_full[color[j]]);
                    if(flag == 1){
                      dx = dxv[j];
                      dy = dyv[j];
                      dz = dzv[j];
                    }
                    glVertex3f(xpos[j] - dx, ypos[j] - dy, zpos[j] - dz);
                    glVertex3f(xpos[j] + dx, ypos[j] + dy, zpos[j] + dz);
                  }
                }
              }
              glEnd();
            }
          }
          else{
            glBegin(GL_POINTS);
            if(show_default == 1){
              glColor4fv(datacopy->partclassbase->rgb);
              for(j = 0;j < datacopy->npoints_file;j+=partdrawskip){
                float zoffset;
                int loc;

                zoffset = GetZCellValOffset(global_scase.meshescoll.meshinfo, xpos[j], ypos[j], &loc);
                if(vis[j] == 1)glVertex3f(xpos[j], ypos[j], zpos[j] + zoffset);
              }
            }
            else{
              color = datacopy->irvals + itype*datacopy->npoints_file;
              for(j = 0;j < datacopy->npoints_file;j+=partdrawskip){
                if(vis[j] == 1){
                  glColor4fv(rgb_full[color[j]]);
                  glVertex3f(xpos[j], ypos[j], zpos[j]);
                }
              }
            }
            glEnd();
          }
        }
        datacopy++;
      }
    }
  }

  // draw streak lines

  datacopy = parti->data5 + nclasses*ipframe;

  if(streak5show == 1){
    for(i = 0;i < parti->nclasses;i++){
      float *xpos, *ypos, *zpos;
      unsigned char *vis;
      int k;
      int show_default;
      float *colorptr;

      partclassdata *partclassi;
      int partclass_index, itype, vistype, class_vis;

      partclassi = parti->partclassptr[i];
      partclass_index = partclassi - global_scase.partclassinfo;

      vistype = current_property->class_present[partclass_index];
      class_vis = current_property->class_vis[partclass_index];

      if(vistype == 0 || datacopy->npoints_file <= 0 || (vistype == 1 && class_vis == 0)){
        if(show_tracers_always == 0 || partclassi->ntypes > 2){
          datacopy++;
          continue;
        }
      }
      itype = current_property->class_types[partclass_index];

      show_default = 0;
      if(itype == -1 || (show_tracers_always == 1 && partclassi->ntypes <= 2)){
        show_default = 1;
      }

      xpos = datacopy->xpos;
      ypos = datacopy->ypos;
      zpos = datacopy->zpos;
      vis = datacopy->vis_part;

      if(show_default == 1){

        // draw the streak line

        GetPartColor(&colorptr, datacopy, show_default, 0, itype, valmin, valmax);
        glColor4fv(colorptr);

        glLineWidth(streaklinewidth);
        for(j = 0;j < datacopy->npoints_file;j+=partdrawskip){
          int tagval;

          tagval = datacopy->tags[j];
          if(vis[j] == 0)continue;
          glBegin(GL_LINE_STRIP);
          glVertex3f(xpos[j], ypos[j], zpos[j]);
          for(k = 1;k < streak5step;k++){
            int jj;
            float *xxpos, *yypos, *zzpos;

            if(ipframe - k < 0)break;
            datapast = parti->data5 + nclasses*(ipframe - k) + i;
            jj = GetTagIndex(parti, &datapast, tagval, DRAWING);
            if(jj < 0)break;
            xxpos = datapast->xpos;
            yypos = datapast->ypos;
            zzpos = datapast->zpos;
            glVertex3f(xxpos[jj], yypos[jj], zzpos[jj]);
          }
          glEnd();
        }

        // draw the dot at the end of the streak line
      }
      else{

        // draw the streak line

        for(j = 0;j < datacopy->npoints_file;j+=partdrawskip){
          int tagval;

          tagval = datacopy->tags[j];
          if(vis[j] == 0)continue;
          if(GetPartColor(&colorptr, datacopy, show_default, j, itype, valmin, valmax)==0)continue;
          if(colorptr[3]==0.0)continue;

          glBegin(GL_LINE_STRIP);
          glColor4fv(colorptr);
          glVertex3f(xpos[j], ypos[j], zpos[j]);
          for(k = 1;k < streak5step;k++){
            int jj;
            float *xxpos, *yypos, *zzpos;

            if(ipframe - k < 0)break;
            datapast = parti->data5 + nclasses*(ipframe - k) + i;
            jj = GetTagIndex(parti, &datapast, tagval, DRAWING);
            if(jj < 0 || datapast->irvals == NULL)break;
            xxpos = datapast->xpos;
            yypos = datapast->ypos;
            zzpos = datapast->zpos;

            GetPartColor(&colorptr, datacopy, show_default, jj, itype, valmin, valmax);
            glColor4fv(colorptr);
            glVertex3f(xxpos[jj], yypos[jj], zzpos[jj]);
          }
          glEnd();
        }
      }

      datacopy++;
    }
  }
  glPopMatrix();
}

/* ------------------ DrawPartFrame ------------------------ */

void DrawPartFrame(int mode){
  int i;

  if(use_tload_begin==1&&global_times[itimes]<global_scase.tload_begin)return;
  if(use_tload_end==1&&global_times[itimes]>global_scase.tload_end)return;
  for(i=0;i<global_scase.npartinfo;i++){
    partdata *parti;
    meshdata *meshi;

    parti = global_scase.partinfo + i;
    if(parti->loaded==0||parti->display==0)continue;
    IF_NOT_USEMESH_CONTINUE(USEMESH_DRAW,parti->blocknumber);
    meshi = global_scase.meshescoll.meshinfo + parti->blocknumber;
    if(meshi->datavis == 0)continue;
    DrawPart(parti, mode);
    SNIFF_ERRORS("after DrawPart");
  }
}

/* ------------------ FreePart5Data ------------------------ */

void FreePart5Data(part5data *datacopy_arg){
  FREEMEMORY(datacopy_arg->cvals);
  FREEMEMORY(datacopy_arg->dsx);
  FREEMEMORY(datacopy_arg->dsy);
  FREEMEMORY(datacopy_arg->dsz);
  FREEMEMORY(datacopy_arg->avatar_angle);
  FREEMEMORY(datacopy_arg->avatar_width);
  FREEMEMORY(datacopy_arg->avatar_height);
  FREEMEMORY(datacopy_arg->avatar_depth);
}

/* ------------------ FreeAllPart5Data ------------------------ */

void FreeAllPart5Data(partdata * parti){
  int i;
  part5data *datacopy_local;

  if(parti->data5==NULL)return;
  datacopy_local = parti->data5;
  for(i=0;i<parti->ntimes*parti->nclasses;i++){
    FreePart5Data(datacopy_local);
    datacopy_local++;
  }
  FREEMEMORY(parti->data5);
  FREEMEMORY(parti->vis_part);
  FREEMEMORY(parti->sort_tags);
  FREEMEMORY(parti->irvals);
}

/* ------------------ InitPart5Data ------------------------ */

void InitPart5Data(part5data *datacopy, partclassdata *partclassi){
  datacopy->cvals=NULL;
  datacopy->partclassbase=partclassi;
  datacopy->xpos=NULL;
  datacopy->ypos=NULL;
  datacopy->zpos=NULL;
  datacopy->dsx=NULL;
  datacopy->dsy=NULL;
  datacopy->dsz=NULL;
  datacopy->avatar_angle=NULL;
  datacopy->avatar_width=NULL;
  datacopy->avatar_height=NULL;
  datacopy->avatar_depth=NULL;
  datacopy->tags=NULL;
  datacopy->vis_part=NULL;
  datacopy->sort_tags=NULL;
  datacopy->rvals=NULL;
  datacopy->irvals=NULL;
}

/* ------------------ ComparePart ------------------------ */

int ComparePart(const void *arg1, const void *arg2){
  partdata *parti, *partj;
  int i, j;

  i = *(int *)arg1;
  j = *(int *)arg2;

  parti = global_scase.partinfo + i;
  partj = global_scase.partinfo + j;

  if(parti->blocknumber<partj->blocknumber)return -1;
  if(parti->blocknumber>partj->blocknumber)return 1;
  return 0;
}

/* ------------------ UpdatePartVis ------------------------ */

void UpdatePartVis(int first_frame_arg, partdata *parti_arg, part5data *datacopy_arg, int nclasses_arg){
  int nparts_local;
  unsigned char *vis_part_local;

  nparts_local = datacopy_arg->npoints_file;
  vis_part_local = datacopy_arg->vis_part;

  if(first_frame_arg== 1){
    int ii;

    for(ii = 0; ii < nparts_local; ii++){
      vis_part_local[ii] = 1;
    }
  }
  else{
    int ii;

    part5data *datalast_local;
    int nvis_local = 0, nleft_local;

    for(ii = 0; ii < nparts_local; ii++){
      int tag_index_local;

      datalast_local = datacopy_arg- nclasses_arg;
      tag_index_local = GetTagIndex(parti_arg, &datalast_local, datacopy_arg->tags[ii], LOADING);
      if(tag_index_local!= -1 && datalast_local->vis_part[tag_index_local] == 1){
        datacopy_arg->vis_part[ii] = 1;
        nvis_local++;
      }
      else{
        datacopy_arg->vis_part[ii] = 0;
      }
    }

    nleft_local = nparts_local- nvis_local;
    if(nleft_local > 0){
      for(ii = 0; ii<nparts_local; ii++){
        if(datacopy_arg->vis_part[ii] == 1)continue;
        if(nleft_local>0){
          datacopy_arg->vis_part[ii] = 1;
          nleft_local--;
        }
      }
    }
  }
}

/* ------------------ UpdateAllPartVis ------------------------ */

void UpdateAllPartVis(partdata *parti){
  part5data *datacopy_local;
  int i, j;
  int firstframe_local = 1;

  datacopy_local = parti->data5;
  for(i = 0; i < parti->ntimes; i++){
    for(j = 0; j < parti->nclasses; j++){
      UpdatePartVis(firstframe_local, parti, datacopy_local, parti->nclasses);
      datacopy_local++;
    }
    if(firstframe_local== 1)firstframe_local = 0;
  }
}

/* ------------------ GetSizeFileStatus ------------------------ */

int GetSizeFileStatus(partdata *parti){

  // return -1 if size file cannot be created (corresponding particle file does not exist)
  // return  0 if size file does not need to be created
  // return  1 if size file needs to be created (doesn't exist or is older than corresponding particle file)

  STRUCTSTAT stat_sizefile_buffer, stat_regfile_buffer;
  int stat_sizefile, stat_regfile;

  stat_sizefile = STAT(parti->size_file, &stat_sizefile_buffer);
  stat_regfile = STAT(parti->reg_file, &stat_regfile_buffer);

  if(stat_regfile != 0)return -1;                         // particle file does not exist

  if(stat_sizefile != 0) return 1;                        // size file does not exist
  if(stat_regfile_buffer.st_size > parti->reg_file_size){ // particle file has grown
    parti->reg_file_size = stat_regfile_buffer.st_size;
    return 1;
  }
  if(stat_regfile_buffer.st_mtime > stat_sizefile_buffer.st_mtime)return 1; // size file is older than particle file
  return 0;
}

/* ------------------ CreatePartSizeFileFromBound ------------------------ */

void CreatePartSizeFileFromBound(char *part5boundfile_arg, char *part5sizefile_arg, LINT filepos_arg){
  FILE *streamin_local, *streamout_local;
  float time_local;
  char buffer_local[255];

  streamin_local  = FOPEN_2DIR(part5boundfile_arg, "r");
  streamout_local = FOPEN_2DIR(part5sizefile_arg, "w");

  for(;;){
    int nclasses_local,j,eof_local;
    LINT frame_size_local;
    char format[128];

    eof_local = 0;
    frame_size_local =0;
    if(fgets(buffer_local,255,streamin_local)==NULL)break;
    sscanf(buffer_local, "%f %i", &time_local, &nclasses_local);
#ifdef INTEL_WIN_COMPILER
      strcpy(format, " %f %lli");
#else
      strcpy(format, " %f %li");
#endif
    fprintf(streamout_local, format, time_local, filepos_arg);
    fprintf(streamout_local, "\n");
    frame_size_local += 12;
    for(j = 0; j<nclasses_local; j++){
      int k, npoints_local, ntypes_local;

      frame_size_local += 12;
      if(fgets(buffer_local, 255, streamin_local)==NULL){
        eof_local = 1;
        break;
      }
      sscanf(buffer_local, "%i %i", &ntypes_local, &npoints_local);
      frame_size_local += 4+4*NXYZ_COMP_PART*npoints_local+4;
      frame_size_local += 4 + 4*npoints_local+ 4;
      if(ntypes_local>0){
        frame_size_local += 4 + 4*npoints_local*ntypes_local+ 4;
      }
      fprintf(streamout_local, " %i\n", npoints_local);
      for(k = 0; k<ntypes_local; k++){
        if(fgets(buffer_local, 255, streamin_local)==NULL){
          eof_local = 1;
          break;
        }
      }
      if(eof_local==1)break;
    }
    if(eof_local==1)break;
    filepos_arg += frame_size_local;
  }
  fclose(streamin_local);
  fclose(streamout_local);
}

/* ------------------ CreatePartSizeFileFromPart ------------------------ */

void CreatePartSizeFileFromPart(char *part5file_arg, char *part5sizefile_arg, LINT file_offset_arg){
  FILE *streamout_local;
  FILE_m *stream;
  int count_read;
  int one_local, version_local, nclasses_local=0;
  int i;
  int *numtypes_local, *numpoints_local;
  int skip_local, numvals_local;

  stream       = fopen_b(part5file_arg, NULL, 0, "rb");
  streamout_local = FOPEN_2DIR(part5sizefile_arg, "w");

  FORTREAD_m(&one_local, 4, 1, stream);
  FORTREAD_m(&version_local, 4, 1, stream);
  FORTREAD_m(&nclasses_local, 4, 1, stream);
  NewMemory((void **)&numtypes_local, 2*nclasses_local* sizeof(int));
  NewMemory((void **)&numpoints_local, nclasses_local* sizeof(int));
  for(i = 0; i < nclasses_local; i++){
    FORTREAD_m(numtypes_local+2*i, 4, 2, stream);
    numvals_local = numtypes_local[2 * i] + numtypes_local[2 * i + 1];
    skip_local = 2*numvals_local*(4 + 30 + 4);
    fseek_m(stream, skip_local, SEEK_CUR);
  }
  while(!feof_m(stream)){
    float time_local;
    LINT frame_size_local;
    char format[128];

    frame_size_local =0;

    FORTREAD_m(&time_local, 4, 1, stream);
    frame_size_local += 12;
    if(count_read != 1)break;
    for(i = 0; i < nclasses_local; i++){
      FORTREAD_m(numpoints_local+ i, 4, 1, stream);
      frame_size_local += 12;
      skip_local = 4+4*NXYZ_COMP_PART*numpoints_local[i]+4;
      skip_local += 4 + 4 * numpoints_local[i] + 4;
      if(numtypes_local[2 * i] > 0)    skip_local += 4 + 4 * numpoints_local[i] * numtypes_local[2 * i] + 4;
      if(numtypes_local[2 * i + 1] > 0)skip_local += 4 + 4 * numpoints_local[i] * numtypes_local[2 * i + 1] + 4;
      fseek_m(stream, skip_local, SEEK_CUR);
      frame_size_local +=skip_local;
    }
#ifdef INTEL_WIN_COMPILER
    strcpy(format, "%f %lli");
#else
    strcpy(format, "%f %li");
#endif
    fprintf(streamout_local, format, time_local, file_offset_arg);
    fprintf(streamout_local, "\n");
    file_offset_arg += frame_size_local;
    for(i = 0; i < nclasses_local; i++){
      fprintf(streamout_local, " %i\n", numpoints_local[i]);
    }
  }
  fclose_m(stream);
  fclose(streamout_local);
  FREEMEMORY(numtypes_local);
  FREEMEMORY(numpoints_local);
}

/* ------------------ GetPartHeaderOffset ------------------------ */

LINT GetPartHeaderOffset(partdata *parti_arg){
  LINT file_offset_local=0;
  int nclasses_local, one_local;
  FILE_m *stream=NULL;
  int version_local;
  int count_read;
  int skip_local;
  int i;
  int *numtypes_local = NULL, *numtypescopy_local, *numpoints_local = NULL;
  int numtypes_temp_local[2];

  stream = fopen_b(parti_arg->reg_file, NULL, 0, "rb");
  if(stream==NULL)return 0;

  FORTREAD_m(&one_local, 4, 1, stream);
  file_offset_local += 12;

  FORTREAD_m(&version_local, 4, 1, stream);
  if(count_read != 1)goto wrapup;
  file_offset_local += 12;

  FORTREAD_m(&nclasses_local, 4, 1, stream);
  if(count_read != 1)goto wrapup;
  file_offset_local += 12;

  NewMemory((void **)&numtypes_local,2*nclasses_local*sizeof(int));
  NewMemory((void **)&numpoints_local,nclasses_local*sizeof(int));
  numtypescopy_local =numtypes_local;
  numtypes_temp_local[0]=0;
  numtypes_temp_local[1]=0;
  CheckMemory;
  for(i=0;i<nclasses_local;i++){
    int returncode;

    FORTREAD_m(numtypes_local, 4, 2, stream);
    if(count_read != 2)goto wrapup;
    file_offset_local += 16;
    *numtypescopy_local++=numtypes_temp_local[0];
    *numtypescopy_local++=numtypes_temp_local[1];
    skip_local = 2*(numtypes_temp_local[0]+numtypes_temp_local[1])*(8 + 30);
    file_offset_local +=skip_local;
    returncode=fseek_m(stream,skip_local,SEEK_CUR);
    if(returncode != PASS_m)goto wrapup;
  }
  CheckMemory;

  wrapup:
  FREEMEMORY(numtypes_local);
  FREEMEMORY(numpoints_local);
  fclose_b(stream);
  return file_offset_local;
}

/* ------------------ CreatePartBoundFile ------------------------ */

void CreatePartBoundFile(partdata *parti){
  FILE_m *stream=NULL;
  int one_local, version_local, nclasses_local;
  int i;
  int count_read;
  size_t returncode;
  float time_local;
  int nparts_local, *numtypes_local = NULL, numtypes_temp_local[2];
  FILE *stream_out_local=NULL;

  if(parti->reg_file == NULL)return;
  stream = fopen_b(parti->reg_file, NULL, 0, "rb");
  if(stream==NULL)return;
  if(parti->bound_file!=NULL)stream_out_local = FOPEN_2DIR(parti->bound_file, "w");
  if(stream_out_local==NULL){
    fclose_b(stream);
    return;
  }

  FORTREAD_m(&one_local, 4, 1, stream);
  FORTREAD_m(&version_local, 4, 1, stream);
  if(count_read != 1)goto wrapup;

  FORTREAD_m(&nclasses_local, 4, 1, stream);
  if(count_read != 1)goto wrapup;

  NewMemory((void **)&numtypes_local, 2*nclasses_local*sizeof(int));
  numtypes_temp_local[0] = 0;
  numtypes_temp_local[1] = 0;
  CheckMemory;
  for(i = 0; i<nclasses_local; i++){
    int skip_local;

    FORTREAD_m(numtypes_temp_local, 4, 2, stream);
    if(count_read != 2)goto wrapup;
    numtypes_local[2*i+0] = numtypes_temp_local[0];
    numtypes_local[2*i+1] = numtypes_temp_local[1];
    skip_local = 2*(numtypes_temp_local[0]+numtypes_temp_local[1])*(8+30);
    returncode = fseek_m(stream, skip_local, SEEK_CUR);
    if(returncode != PASS_m)goto wrapup;
  }
  CheckMemory;

  for(;;){
    int jj;

    CheckMemory;
    FORTREAD_m(&time_local, 4, 1, stream);
    if(count_read != 1)goto wrapup;
    fprintf(stream_out_local, "%f %i 1\n", time_local, nclasses_local);

    for(jj = 0; jj<nclasses_local; jj++){
      int skip_local, kk;
      float *rvals_local;

      FORTREAD_m(&nparts_local, 4, 1, stream);
      if(count_read != 1)goto wrapup;

      fprintf(stream_out_local, "%i %i\n", numtypes_local[2*jj], nparts_local);
      CheckMemory;

      skip_local = 4+NXYZ_COMP_PART*nparts_local*sizeof(float)+4; // skip over particle xyz coords
      skip_local += 4+nparts_local*sizeof(int)+4;                   // skip over tags
      returncode = fseek_m(stream, skip_local, SEEK_CUR);
      if(returncode != PASS_m)goto wrapup;
      CheckMemory;
      if(numtypes_local[2*jj]>0){
        FORTREAD_mv((void **)&rvals_local, 4, nparts_local*numtypes_local[2*jj], stream);
        if(count_read != nparts_local * numtypes_local[2 * jj])goto wrapup;
      }
      CheckMemory;
      for(kk = 0; kk<numtypes_local[2*jj]; kk++){
        if(nparts_local>0){
          float valmin_local, valmax_local;
          int ii;

          valmin_local =  1000000000.0;
          valmax_local = -1000000000.0;
          for(ii = 0; ii<nparts_local; ii++){
            valmin_local = MIN(valmin_local, rvals_local[ii]);
            valmax_local = MAX(valmax_local, rvals_local[ii]);
          }
          fprintf(stream_out_local, "%f %f\n", valmin_local, valmax_local);
        }
        else{
          fprintf(stream_out_local, "1.0 0.0\n");
        }
        rvals_local += nparts_local;
      }
    }
    CheckMemory;
  }
wrapup:
  fclose_b(stream);
  fclose(stream_out_local);
  CheckMemory;
  FREEMEMORY(numtypes_local);
}

/* ------------------ CreatePartSizeFile ------------------------ */
void CreatePartSizeFile(partdata *parti){
  FILE *stream_local=NULL;
  LINT header_offset_local;
  char *smokeview_scratchdir = GetUserConfigDir();

  if(parti->reg_file!=NULL)stream_local = fopen(parti->reg_file, "rb");
  if(parti->reg_file==NULL||stream_local==NULL)return;
  fclose(stream_local);
  header_offset_local =GetPartHeaderOffset(parti);
  stream_local = FOPEN_2DIR(parti->bound_file, "r");
  if(stream_local==NULL){
    TestWrite(smokeview_scratchdir, &(parti->bound_file));
    CreatePartBoundFile(parti);
    stream_local = FOPEN_2DIR(parti->bound_file, "r");
  }
  if(stream_local!=NULL){
    fclose(stream_local);
    TestWrite(smokeview_scratchdir, &(parti->size_file));
    CreatePartSizeFileFromBound(parti->bound_file, parti->size_file, header_offset_local);
    FREEMEMORY(smokeview_scratchdir);
    return;
  }
  printf("***warning: particle bound/size file %s could not be opened\n", parti->bound_file);
  printf("            particle sizing proceeding using the full particle file: %s\n", parti->reg_file);
  TestWrite(smokeview_scratchdir, &(parti->size_file));
  CreatePartSizeFileFromPart(parti->reg_file, parti->size_file, header_offset_local);
  FREEMEMORY(smokeview_scratchdir);
}

  /* ------------------ GetPartHistogramFile ------------------------ */
void GetPartHistogramFile(partdata *parti){
  int i;
  part5data *datacopy;

  if(parti->histograms == NULL){
    NewMemory((void **)&parti->histograms, npart5prop*sizeof(histogramdata *));
    for(i = 0; i < npart5prop; i++){
      NewMemory((void **)&parti->histograms[i], sizeof(histogramdata));
      InitHistogram(parti->histograms[i], NHIST_BUCKETS, NULL, NULL);
    }
  }
  for(i = 0; i<npart5prop; i++){
    ResetHistogram(parti->histograms[i], NULL, NULL);
  }
  datacopy = parti->data5;
  if(datacopy != NULL){
    for(i = 0; i < parti->ntimes; i++){
      int j;

      for(j = 0; j < parti->nclasses; j++){
        partclassdata *partclassi;
        float *rvals;
        int k;

        partclassi = parti->partclassptr[j];
        rvals = datacopy->rvals;

        if(rvals!=NULL){
          for(k = 2; k<partclassi->ntypes; k++){
            partpropdata *prop_id;
            int partprop_index;

            prop_id = GetPartProp(partclassi->labels[k].longlabel);
            if(prop_id==NULL)continue;

            partprop_index = prop_id-part5propinfo;
            UpdateHistogram(rvals, NULL,datacopy->npoints_file, parti->histograms[partprop_index]);
            rvals += datacopy->npoints_file;
          }
        }
        datacopy++;
      }
    }
  }
}

/* ------------------ MergePartHistograms ------------------------ */

void MergePartHistograms(void){
  int i;

  if(full_part_histogram==NULL){
    NewMemory((void **)&full_part_histogram, npart5prop*sizeof(histogramdata));
  }
  else{
    for(i=0;i<npart5prop;i++){
      FreeHistogram(full_part_histogram+i);
    }
  }
  for(i=0;i<npart5prop;i++){
    InitHistogram(full_part_histogram+i, NHIST_BUCKETS, NULL, NULL);
  }
  for(i = 0; i<global_scase.npartinfo; i++){
    partdata *parti;
    int j;

    parti = global_scase.partinfo + i;
    if(parti->loaded==0)continue;
    for(j = 0; j < parti->nclasses; j++){
      partclassdata *partclassi;
      int k;

      partclassi = parti->partclassptr[j];
      for(k = 2; k<partclassi->ntypes; k++){
        partpropdata *prop_id;
        int partprop_index;

        prop_id = GetPartProp(partclassi->labels[k].longlabel);
        if(prop_id==NULL)continue;

        partprop_index = prop_id-part5propinfo;
        MergeHistogram(full_part_histogram+partprop_index, parti->histograms[partprop_index], MERGE_BOUNDS);
      }
    }
  }
}

/* ------------------ GeneratePartHistograms ------------------------ */

void GeneratePartHistograms(void){
  int i;

  for(i=0;i<global_scase.npartinfo;i++){
    partdata *parti;

    parti = global_scase.partinfo + i;
    if(parti->loaded==1){
      GetPartHistogramFile(parti);
    }
  }
  MergePartHistograms();
  if(in_part_mt == 1){
    printf("particle setup complete\n");
    in_part_mt = 0;
  }
}

/* ------------------ SortPartTags ------------------------ */

void SortPartTags(partdata *parti){
  int i;
  part5data *datacopy_local;

  datacopy_local = parti->data5;
  for(i = 0; i < parti->ntimes; i++){
    int class_index;

    for(class_index = 0; class_index < parti->nclasses; class_index++){
      qsort(datacopy_local->sort_tags, ( size_t )datacopy_local->npoints_file, 2*sizeof(int), CompareTags);
      datacopy_local++;
    }
  }
}

/* ------------------ SortAllPartTags ------------------------ */

void *SortAllPartTags(void *arg){
  int i;

  INIT_PRINT_TIMER(timer_sortparttags);
  for(i = 0; i < global_scase.npartinfo; i++){
    partdata *parti;

    parti = global_scase.partinfo + i;
    if(parti->loaded == 0)continue;
    SortPartTags(parti);
  }
  PRINT_TIMER(timer_sortparttags, "SortPartTags");
  sorting_tags = 0;
  THREAD_EXIT(sorttags_threads);
}

/* ------------------ GetPartData ------------------------ */

void GetPartData(partdata *parti, int nf_all_arg, FILE_SIZE *file_size_arg){
  FILE_m *stream;
  int class_index;
  int one_local, version_local, nclasses_local;
  int skip_local, nparts_local;
  int *numtypes_local = NULL, *numtypescopy_local, *numpoints_local = NULL;
  int numtypes_temp_local[2];
  int count_local, count2_local, first_frame_local = 1;
  int count_read;
  size_t returncode;
  float time_local;
  part5data *datacopy_local;

  if(parti->stream!=NULL){
    fclose_m(parti->stream);
    parti->stream = NULL;
  }
  stream = fopen_m(parti->reg_file, "rbm");
  *file_size_arg = GetFileSizeSMV(parti->reg_file);
  parti->stream = stream;
  if(stream==NULL)return;

  FORTREAD_m(&one_local,4,1,stream);
  if(count_read!=1)goto wrapup;

  FORTREAD_m(&version_local, 4, 1, stream);
  if(count_read!=1)goto wrapup;

  FORTREAD_m(&nclasses_local,4, 1, stream);
  if(count_read!=1)goto wrapup;

  NewMemory((void **)&numtypes_local,2*nclasses_local*sizeof(int));
  NewMemory((void **)&numpoints_local,nclasses_local*sizeof(int));
  numtypescopy_local =numtypes_local;
  numtypes_temp_local[0]=0;
  numtypes_temp_local[1]=0;
  CheckMemory;
  for(class_index=0;class_index<nclasses_local;class_index++){
    FORTREAD_m(numtypes_temp_local,4,2,stream);
    if(count_read!=2)goto wrapup;

    *numtypescopy_local++=numtypes_temp_local[0];
    *numtypescopy_local++=numtypes_temp_local[1];
    skip_local = 2*(numtypes_temp_local[0]+numtypes_temp_local[1])*(8 + 30);
    returncode = fseek_m(stream,skip_local,SEEK_CUR);
    if(returncode!=PASS_m)goto wrapup;
  }
  CheckMemory;

  datacopy_local = parti->data5;
  count_local =0;
  count2_local =-1;
  parti->ntimes = 0;
  for(;;){
    int doit_local;

    CheckMemory;
    if(count_local>=nf_all_arg)break;
    FORTREAD_m(&time_local,4,1,stream);
    if(count_read!=1)goto wrapup;

    if((tload_step>1       && count_local%tload_step!=0)||
       (use_tload_begin==1 && time_local<global_scase.tload_begin-TEPS)||
       (use_tload_end==1   && time_local>global_scase.tload_end+TEPS)){
      doit_local=0;
    }
    else{
      count2_local++;
      doit_local =1;
    }
    count_local++;

    if(doit_local==1){
      parti->times[count2_local]=time_local;
      parti->ntimes = count2_local+1;
    }
    for(class_index=0;class_index<nclasses_local;class_index++){
      FORTREAD_m(&nparts_local,4,1,stream);
      if(count_read!=1)goto wrapup;
      numpoints_local[class_index] = nparts_local;
      skip_local=0;
      CheckMemory;
      if(doit_local==1){
        float *xyz;

        FORTREAD_mv((void **)&xyz, 4, NXYZ_COMP_PART*nparts_local, stream);
        datacopy_local->xpos = xyz;
        datacopy_local->ypos = xyz +   nparts_local;
        datacopy_local->zpos = xyz + 2*nparts_local;
        if(count_read!=NXYZ_COMP_PART*nparts_local)goto wrapup;
        CheckMemory;
      }
      else{
        skip_local += 4+NXYZ_COMP_PART*nparts_local*sizeof(float)+4;
      }
      CheckMemory;
      if(doit_local==1){
        int *sort_tags_local;
        int j;

        sort_tags_local = datacopy_local->sort_tags;
        FORTREAD_mv((void **)&(datacopy_local->tags),4, nparts_local, stream);
        if(count_read!=nparts_local)goto wrapup;
        CheckMemory;
        if(nparts_local>0){
          for(j=0;j<nparts_local;j++){
            sort_tags_local[2*j]=datacopy_local->tags[j];
            sort_tags_local[2*j+1]=j;
          }
        }
      }
      else{
        skip_local += 4 + nparts_local*sizeof(int) + 4;  // skip over tag
      }
      CheckMemory;
      if(doit_local==1){
        int part_type;
        if(numtypes_local[2*class_index]>0){
          float *valmin_part, *valmax_part;

          FORTREAD_mv((void **)&(datacopy_local->rvals), 4, nparts_local*numtypes_local[2*class_index], stream);
          if(count_read!=nparts_local*numtypes_local[2*class_index])goto wrapup;

          valmin_part = parti->valmin_part;
          valmax_part = parti->valmax_part;
          for(part_type = 0; part_type<numtypes_local[2*class_index]; part_type++){
            int prop_index, k;
            float *vals;

            prop_index = GetPartPropIndex(class_index, part_type+2);
            vals = datacopy_local->rvals+part_type*nparts_local;
            for(k = 0; k<nparts_local; k++){
              float val;

              val = *vals++;
              if(valmin_part[prop_index]>valmax_part[prop_index]){
                valmin_part[prop_index] = val;
                valmax_part[prop_index] = val;
              }
              else{
                valmin_part[prop_index] = MIN(val, valmin_part[prop_index]);
                valmax_part[prop_index] = MAX(val, valmax_part[prop_index]);
              }
            }
          }
        }
      }
      else{
        if(numtypes_local[2*class_index]>0){
          skip_local += 4 + nparts_local*numtypes_local[2*class_index]*sizeof(float) + 4;  // skip over vals for now
        }
      }
      CheckMemory;

      if(skip_local>0){
        returncode = fseek_m(stream,skip_local,SEEK_CUR);
        if(returncode!=PASS_m)goto wrapup;
      }
      else{
        datacopy_local++;
      }
      CheckMemory;
    }
    CheckMemory;
    if(first_frame_local==1)first_frame_local =0;
  }
wrapup:
  UpdateAllPartVis(parti);
  CheckMemory;
  FREEMEMORY(numtypes_local);
  FREEMEMORY(numpoints_local);
}

/* ------------------ PrintPartProp ------------------------ */

#ifdef _DEBUG
void PrintPartProp(void){
  int i;

  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(STRCMP(propi->label->longlabel, "Uniform color")==0){
      PRINTF("label=%s\n", propi->label->longlabel);
    }
    else{
      PRINTF("label=%s min=%f max=%f\n", propi->label->longlabel, propi->valmin, propi->valmax);
      PRINTF("   glbmin=%f glbmax=%f\n", propi->dlg_global_valmin, propi->dlg_global_valmax);
    }
    PRINTF("\n");
  }
}
#endif


/* ------------------ GetPartPropIndexS ------------------------ */

int GetPartPropIndexS(char *shortlabel){
  int i;

  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(strcmp(propi->label->shortlabel,shortlabel)==0)return i;
  }
  return -1;
}

/* ------------------ GetPartPropIndex ------------------------ */

int GetPartPropIndex(int class_i, int class_i_j){
  int i;
  char *label;
  partclassdata *partclassi;
  flowlabels *labels, *labelj;

  assert(class_i>=0&&class_i<global_scase.npartclassinfo);
  class_i = CLAMP(class_i,0, global_scase.npartclassinfo-1);
  partclassi = global_scase.partclassinfo+class_i;

  labels = partclassi->labels;
  assert(class_i_j>=0&&class_i_j<partclassi->ntypes);
  class_i_j = CLAMP(class_i_j,0, partclassi->ntypes-1);
  labelj = labels+class_i_j;

  label = labelj->longlabel;
  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(label!=NULL&&strcmp(propi->label->longlabel,label)==0)return i;
  }
  return 0;
}

/* ------------------ GetPartProp ------------------------ */

partpropdata *GetPartProp(char *label){
  int i;

  if(part5propinfo == NULL)return NULL;
  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(strcmp(propi->label->longlabel,label)==0)return propi;
  }
  return NULL;
}

/* ------------------ SetStreakShow ------------------------ */

void SetStreakShow(int show){
  if(show == 1 && sorting_tags==1){
    THREADcontrol(sorttags_threads, THREAD_JOIN);
  }
  streak5show = show;
}

/* ------------------ InitPartProp ------------------------ */

void InitPartProp(void){
  int i,j,k;

  // 0.  only needed if InitPartProp is called more than once
  // (and if so, need to also free memory of each component)

  FREEMEMORY(part5propinfo);
  npart5prop=0;

  // 1.  count max number of distinct variables

  for(i=0;i<global_scase.npartclassinfo;i++){
    partclassdata *partclassi;

    partclassi = global_scase.partclassinfo + i;
    npart5prop+=(partclassi->ntypes-1);  // don't include first type which is hidden
  }

  // 2. now count the exact amount and put labels into array just allocated

  if(npart5prop>0){
    NewMemory((void **)&part5propinfo,npart5prop*sizeof(partpropdata));
    npart5prop=0;

    for(i=0;i<global_scase.npartclassinfo;i++){
      partclassdata *partclassi;

      partclassi = global_scase.partclassinfo + i;
      for(j=1;j<partclassi->ntypes;j++){ // skip over first type which is hidden
        flowlabels *flowlabel;
        int define_it;

        define_it = 1;
        flowlabel = partclassi->labels + j;
        for(k=0;k<npart5prop;k++){
          partpropdata *propi;
          char *proplabel;

          propi = part5propinfo + k;
          proplabel = propi->label->longlabel;
          if(strcmp(proplabel,flowlabel->longlabel)==0){
            define_it=0;
            break;
          }
        }
        if(define_it==1){
          partpropdata *propi;

          propi = part5propinfo + npart5prop;

          propi->label=flowlabel;
          propi->setvalmin=GLOBAL_MIN;
          propi->setvalmax=GLOBAL_MAX;
          propi->set_global_bounds=1;
          propi->dlg_global_valmin=100000000.0;
          propi->dlg_global_valmax=-propi->dlg_global_valmin;
          propi->valmin=1.0;
          propi->valmax=0.0;
          propi->user_min=1.0;
          propi->user_max=0.0;
          propi->display=0;


          propi->setchopmin=0;
          propi->setchopmax=0;
          propi->chopmin=1.0;
          propi->chopmax=0.0;

          propi->partlabelvals = NULL;
          NewMemory((void **)&propi->partlabelvals, 256*sizeof(float));
          npart5prop++;
        }
      }

    }
  }
  for(i=0;i<npart5prop;i++){
    partpropdata *propi;
    int ii;

    propi = part5propinfo + i;

    propi->class_present=NULL;
    propi->class_vis=NULL;
    propi->class_types=NULL;
    NewMemory((void **)&propi->class_types,global_scase.npartclassinfo*sizeof(unsigned int));
    NewMemory((void **)&propi->class_present,global_scase.npartclassinfo*sizeof(unsigned char));
    NewMemory((void **)&propi->class_vis,global_scase.npartclassinfo*sizeof(unsigned char));
    for(ii=0;ii<global_scase.npartclassinfo;ii++){
      propi->class_vis[ii]=1;
      propi->class_present[ii]=0;
      propi->class_types[ii]=0;
    }
  }
  for(i=0;i<global_scase.npartclassinfo;i++){
    partclassdata *partclassi;

    partclassi = global_scase.partclassinfo + i;
    for(j=1;j<partclassi->ntypes;j++){
      flowlabels *flowlabel;
      partpropdata *classprop;

      flowlabel = partclassi->labels + j;
      classprop = GetPartProp(flowlabel->longlabel);
      if(classprop!=NULL){
        classprop->class_present[i]=1;
        classprop->class_types[i]=j-2;
      }
    }
  }
}

/* ------------------ GetNPartFrames ------------------------ */

int GetNPartFrames(partdata *parti){
  FILE *stream;
  char buffer[256];
  float time_local;
  char *reg_file, *size_file, *bound_file;
  int i;
  int doit = 0;
  int stat_sizefile, stat_regfile;
  STRUCTSTAT stat_sizefile_buffer, stat_regfile_buffer;
  int nframes_all;

  reg_file=parti->reg_file;
  size_file=parti->size_file;
  bound_file = parti->bound_file;

  // if size file doesn't exist then generate it

  stat_sizefile=STAT(size_file,&stat_sizefile_buffer);
  stat_regfile=STAT(reg_file,&stat_regfile_buffer);
  if(stat_regfile!=0)return -1;

  // create a size file if 1) the size does not exist
  //                       2) base file is newer than the size file
  // create_part5sizefile(reg_file,size_file);

  if(stat_regfile_buffer.st_size > parti->reg_file_size){                   // particle file has grown
    parti->reg_file_size = stat_regfile_buffer.st_size;
    doit = 1;
  }
  if(doit==1||stat_sizefile != 0 || stat_regfile_buffer.st_mtime>stat_sizefile_buffer.st_mtime){

    TrimBack(reg_file);
    TrimBack(size_file);
    TrimBack(bound_file);
    if(parti->loaded==1)CreatePartSizeFile(parti);
  }

  stream=FOPEN_2DIR(size_file,"r");
  if(stream==NULL)return -1;

  nframes_all=0;
  for(;;){
    int exitloop;

    if(fgets(buffer,255,stream)==NULL)break;
    sscanf(buffer,"%f",&time_local);
    exitloop=0;
    for(i=0;i<parti->nclasses;i++){
      if(fgets(buffer,255,stream)==NULL){
        exitloop=1;
        break;
      }
    }
    if(exitloop==1)break;
    nframes_all++;
  }
  fclose(stream);
  return nframes_all;
}

/* ------------------ GetMinPartFrames ------------------------ */

int GetMinPartFrames(int flag){
  int i;
  int min_frames=-1;

  INIT_PRINT_TIMER(timer_nparts);
  for(i=0;i<global_scase.npartinfo;i++){
    partdata *parti;
    int nframes;

    parti = global_scase.partinfo + i;
    if(flag == PARTFILE_LOADALL ||
      (flag == PARTFILE_RELOADALL&&parti->loaded == 1) ||
      (flag >= 0 && i == flag)){

      nframes = GetNPartFrames(parti);
      if(nframes > 0){
        if(min_frames == -1){
          min_frames = nframes;
        }
        else{
          if(nframes != -1 && nframes < min_frames)min_frames = nframes;
        }
      }
    }
  }
  PRINT_TIMER(timer_nparts, "GetMinPartFrames");
  return min_frames;
}

#define FORCE 1
#define NOT_FORCE 0

/* ------------------ GetPartHeader ------------------------ */

int GetPartHeader(partdata * parti, int *nf_all, int option_arg, int print_option_arg){
  FILE *stream;
  char buffer_local[256];
  float time_local;
  int i;
  int count_local, nframes_all_local, sizefile_status_local;

  parti->ntimes=0;

  sizefile_status_local = GetSizeFileStatus(parti);
  if(sizefile_status_local== -1)return 0; // particle file does not exist so cannot be sized
  if(option_arg==FORCE||sizefile_status_local== 1){        // size file is missing or older than particle file
    TrimBack(parti->reg_file);
    TrimBack(parti->size_file);
    CreatePartSizeFile(parti);
  }

  stream=FOPEN_2DIR(parti->size_file,"r");
  if(stream==NULL)return 0;

    // pass 1: count frames

  nframes_all_local =0;
  parti->ntimes = 0;
  for(;;){
    int exitloop_local;

    if(fgets(buffer_local,255,stream)==NULL)break;
    sscanf(buffer_local,"%f",&time_local);
    exitloop_local =0;
    for(i=0;i<parti->nclasses;i++){
      if(fgets(buffer_local,255,stream)==NULL||(global_scase.npartinfo>1&&npartframes_max!=-1&&nframes_all_local+1>npartframes_max)){
        exitloop_local =1;
        break;
      }
    }
    if(exitloop_local == 1)break;
    nframes_all_local++;
    if(tload_step>1       && (nframes_all_local-1)%tload_step!=0)continue;
    if(use_tload_begin==1 && time_local<global_scase.tload_begin-TEPS)continue;
    if(use_tload_end==1   && time_local>global_scase.tload_end+TEPS)break;
    (parti->ntimes)++;
  }
  rewind(stream);
  *nf_all       = nframes_all_local;
  if(parti->ntimes==0){
    fclose(stream);
    return 0;
  }

  // allocate memory for number of time steps * number of classes

  CheckMemory;
  NewMemory((void **)&parti->data5,     parti->nclasses*parti->ntimes*sizeof(part5data));
  NewMemory((void **)&parti->times,     parti->ntimes*sizeof(float));
  NewMemory((void **)&parti->times_map, parti->ntimes);
  NewMemory((void **)&parti->filepos,   nframes_all_local*sizeof(LINT));

  // free memory for x, y, z frame data

  for(i=0;i<parti->nclasses;i++){
    partclassdata *partclassi;

    partclassi = parti->partclassptr[i];
    partclassi->maxpoints=0;
  }

  // pass 2 - allocate memory for x, y, z frame data

  int nall_points_local;
  {
    part5data *datacopy_local;
    int fail_local;
    LINT filepos_local;
    int nall_points_types_local;

    fail_local =0;
    count_local =-1;
    datacopy_local =parti->data5;
    for(i=0;i<nframes_all_local;i++){
      int j;
      char format[128];
      int skipit;

      count_local++;
      fail_local =0;
      if(fgets(buffer_local,255,stream)==NULL){
        fail_local =1;
        break;
      }
      filepos_local = -1;
#ifdef INTEL_WIN_COMPILER
      strcpy(format, "%f %lli");
#else
      strcpy(format, "%f %li");
#endif
      sscanf(buffer_local, format, &time_local, &filepos_local);
      parti->filepos[count_local] = filepos_local;               // record file position for every frame
      skipit = 0;
      if(tload_step>1       && count_local%tload_step!=0)skipit = 1;
      if(use_tload_begin==1 && time_local<global_scase.tload_begin-TEPS)skipit = 1;
      if(use_tload_end==1   && time_local>global_scase.tload_end+TEPS)break;
      if(skipit == 1){
        for(j=0;j<parti->nclasses;j++){
          if(fgets(buffer_local,255,stream)==NULL){
            fail_local =1;
          }
        }
        if(fail_local==1)break;
        continue;
      }
      for(j=0;j<parti->nclasses;j++){
        int npoints_local;
        partclassdata *partclassj;

        datacopy_local->time = time_local;
        partclassj = parti->partclassptr[j];
        InitPart5Data(datacopy_local,partclassj);
        if(fgets(buffer_local,255,stream)==NULL){
          fail_local =1;
          break;
        }
        sscanf(buffer_local,"%i",&datacopy_local->npoints_file);
        npoints_local =datacopy_local->npoints_file;
        if(npoints_local>partclassj->maxpoints)partclassj->maxpoints=npoints_local;
        if(npoints_local>0){
          if(partfast==NO){
            NewMemory((void **)&datacopy_local->dsx, npoints_local*sizeof(float));
            NewMemory((void **)&datacopy_local->dsy, npoints_local*sizeof(float));
            NewMemory((void **)&datacopy_local->dsz, npoints_local*sizeof(float));
          }
        }
        datacopy_local++;
      }
      if(fail_local==1)break;
    }
    if(fail_local==1)parti->ntimes=i;
    fclose(stream);

    nall_points_types_local = 0;
    nall_points_local = 0;
    datacopy_local =parti->data5;
    for(i=0;i<parti->ntimes;i++){
      int j;

      for(j=0;j<parti->nclasses;j++){
        int npoints_local, ntypes_local;

        npoints_local            = datacopy_local->npoints_file;
        ntypes_local             = datacopy_local->partclassbase->ntypes;
        nall_points_types_local += npoints_local*ntypes_local;
        nall_points_local       += npoints_local;
        datacopy_local++;
      }
    }
    FREEMEMORY(parti->vis_part);
    FREEMEMORY(parti->sort_tags);

    NewMemory((void **)&parti->vis_part,    MAX(nall_points_local, 1)*sizeof(unsigned char));
    NewMemory((void **)&parti->sort_tags, 2*MAX(nall_points_local, 1)*sizeof(int));

    FREEMEMORY(parti->irvals);
    NewMemory((void **)&parti->irvals, MAX(nall_points_types_local,1)*sizeof(unsigned char));

    datacopy_local =parti->data5;
    nall_points_types_local = 0;
    nall_points_local = 0;
    for(i=0;i<parti->ntimes;i++){
      int j;

      for(j=0;j<parti->nclasses;j++){
        int npoints_local, ntypes_local;

        datacopy_local->irvals    = parti->irvals    +     nall_points_types_local;
        datacopy_local->vis_part  = parti->vis_part  +     nall_points_local;
        datacopy_local->sort_tags = parti->sort_tags +   2*nall_points_local;

        npoints_local            = datacopy_local->npoints_file;
        ntypes_local             = datacopy_local->partclassbase->ntypes;
        nall_points_types_local += npoints_local*ntypes_local;
        nall_points_local       += npoints_local;
        datacopy_local++;
      }
    }
  }
  if(nall_points_local==0){
    return 0;
  }
  return 1;
}

/* ------------------ UpdatePartColors ------------------------ */

void UpdatePartColors(partdata *parti, int flag){
  int j;

  if(colorlabelpart==NULL){
    NewMemory((void **)&colorlabelpart, MAXRGB*sizeof(char *));
    {
      int n;

      for(n = 0; n<MAXRGB; n++){
        colorlabelpart[n] = NULL;
      }
      for(n = 0; n<global_scase.nrgb; n++){
        NewMemory((void **)&colorlabelpart[n], 11);
      }
    }
  }
  if(parti!=NULL){
    if(parti->loaded==1&&parti->display==1){
      if(parti->stream!=NULL){
        GetPartColors(parti, global_scase.nrgb, flag);
      }
      else{
        printf("***warning: particle data in %s was unloaded, colors not updated\n",parti->file);
      }
    }
  }
  else{
    for(j = 0; j<global_scase.npartinfo; j++){
      partdata *partj;

      partj = global_scase.partinfo+j;
      if(partj->loaded==1&&partj->display==1){
        if(partj->stream==NULL){
          printf("***warning: particle data in one or more particle files was unloaded, colors not updated\n");
          return;
        }
      }
    }
    for(j = 0; j<global_scase.npartinfo; j++){
      partdata *partj;

      partj = global_scase.partinfo+j;
      if(partj->loaded==1&&partj->display==1){
        GetPartColors(partj, global_scase.nrgb, flag);
      }
    }
  }
}

/* -----  ------------- FinalizePartLoad ------------------------ */

void FinalizePartLoad(partdata *parti){
  int j;

  for(j = 0; j<global_scase.npartinfo; j++){
    partdata *partj;

    partj = global_scase.partinfo+j;
    if(partj->request_load==1){
      MakeTimesMap(partj->times, &partj->times_map, partj->ntimes);
      partj->request_load = 0;
      partj->loaded = 1;
      partj->display = 1;
    }
  }
  visParticles = 1;
  sorting_tags = 1;
  THREADrun(sorttags_threads);
  if(runscript == 1 || streak5show == 1){
    THREADcontrol(sorttags_threads, THREAD_JOIN);
  }

  // generate histograms now rather than in the background if a script is running

  INIT_PRINT_TIMER(part_time1);
  GetGlobalPartBounds(ALL_FILES);
  SetLoadedPartBounds(NULL, 0);
  PRINT_TIMER(part_time1, "particle get bounds time");
  if(cache_part_data==1){
    INIT_PRINT_TIMER(part_time2);
    for(j = 0; j<global_scase.npartinfo; j++){
      partdata *partj;

      partj = global_scase.partinfo+j;
      if(partj->loaded==1){
        UpdatePartColors(partj, 0);
      }
    }
    PRINT_TIMER(part_time2, "particle update colors time");
  }
#define BOUND_PERCENTILE_DRAW          120
  GLUIPartBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
  parttype = 0;
  ParticlePropShowMenu(part5colorindex);
  plotstate = GetPlotState(DYNAMIC_PLOTS);
  UpdateTimes();
  UpdatePart5Extremes();
  updatemenu = 1;
  ForceIdle();
  glutPostRedisplay();
}

/* -----  ------------- ReadPart ------------------------ */

FILE_SIZE ReadPart(char *file_arg, int ifile_arg, int load_flag, int *errorcode_arg){
  size_t lenfile_local;
  int error_local=0, nf_all_local;
  partdata *parti;
  FILE_SIZE file_size_local;
  float load_time_local;

  SetTimeState();
  START_TIMER(load_time_local);
  assert(ifile_arg>=0&&ifile_arg<global_scase.npartinfo);
  parti=global_scase.partinfo+ifile_arg;

  FreeAllPart5Data(parti);

  if(load_flag==UNLOAD)parti->skipload = 1;
  if(parti->loaded==0&&load_flag==UNLOAD)return 0.0;

  *errorcode_arg =0;
  parti->loaded = 0;
  parti->display=0;

  FREEMEMORY(parti->times);
  FREEMEMORY(parti->times_map);
  FREEMEMORY(parti->filepos);

  if(load_flag == UNLOAD){
    if(parti->finalize == 1){
      UpdatePartColors(parti, 0);
      UpdateTimes();
      updatemenu = 1;
      UpdatePart5Extremes();
      PrintMemoryInfo;
      THREADcontrol(partload_threads, THREAD_LOCK);
      plotstate = GetPlotState(DYNAMIC_PLOTS);
      THREADcontrol(partload_threads, THREAD_UNLOCK);
    }
    return 0.0;
  }

  lenfile_local = strlen(file_arg);
  if(lenfile_local==0){
    ReadPart("",ifile_arg,UNLOAD,&error_local);
    UpdateTimes();
    return 0.0;
  }

  if(use_partload_threads==1){
    THREADcontrol(partload_threads, THREAD_LOCK);
    PrintPartLoadSummary(PART_BEFORE, PART_LOADING);
    THREADcontrol(partload_threads, THREAD_UNLOCK);
  }
  else{
    PRINTF("Loading %s", file_arg);
  }
  int have_particles;

  have_particles = GetPartHeader(parti, &nf_all_local, NOT_FORCE, 1);
  if(have_particles==0){
    ReadPart("", ifile_arg, UNLOAD, &error_local);
    return 0.0;
  }
  CheckMemory;
  INIT_PRINT_TIMER(timer_getpartdata);
  GetPartData(parti, nf_all_local, &file_size_local);
  PRINT_TIMER(timer_getpartdata, "GetPartData");
  CheckMemory;
  THREADcontrol(partload_threads, THREAD_LOCK);
  parti->loaded = 1;
  parti->display = 1;
  parti->hist_update=1;
  if(cache_part_data==0){
    UpdatePartColors(parti, 0);
  }
  THREADcontrol(partload_threads, THREAD_UNLOCK);
  if(cache_part_data==0){
    fclose_m(parti->stream);
    parti->stream = NULL;
  }

  PrintMemoryInfo;

  parti->request_load = 1;
  if(use_partload_threads==1){
    if(global_scase.npartinfo>1){
      THREADcontrol(partload_threads, THREAD_LOCK);
      PrintPartLoadSummary(PART_AFTER, PART_LOADING);
      THREADcontrol(partload_threads, THREAD_UNLOCK);
    }
  }
  else{
    if(parti->finalize == 1){
      INIT_PRINT_TIMER(finalize_part);
      FinalizePartLoad(parti);
      PRINT_TIMER(finalize_part, "FinalizePartLoad");
    }
    STOP_TIMER(load_time_local);
    if(file_size_local>1000000000){
      PRINTF(" - %.1f GB/%.1f s\n", (float)file_size_local/1000000000., load_time_local);
    }
    else if(file_size_local>1000000){
      PRINTF(" - %.1f MB/%.1f s\n", (float)file_size_local/1000000., load_time_local);
    }
    else{
      PRINTF(" - %.0f kB/%.1f s\n", (float)file_size_local/1000., load_time_local);
    }
    update_part_bounds = 1;
  }
  return file_size_local;
}

/* ------------------ UpdatePartMenuLabels ------------------------ */

void UpdatePartMenuLabels(void){
  int i;
  partdata *parti;
  char label[128];
  int lenlabel;

  if(global_scase.npartinfo>0){
    FREEMEMORY(partorderindex);
    NewMemory((void **)&partorderindex,sizeof(int)*global_scase.npartinfo);
    for(i=0;i<global_scase.npartinfo;i++){
      partorderindex[i]=i;
    }
    qsort( (int *)partorderindex, (size_t)global_scase.npartinfo, sizeof(int), ComparePart);

    for(i=0;i<global_scase.npartinfo;i++){
      parti = global_scase.partinfo + i;
      STRCPY(parti->menulabel,"");
      STRCAT(parti->menulabel, "particles");
      lenlabel=strlen(parti->menulabel);
      if(global_scase.meshescoll.nmeshes>1){
        meshdata *partmesh;

        partmesh = global_scase.meshescoll.meshinfo + parti->blocknumber;
        sprintf(label,"%s",partmesh->label);
        if(lenlabel>0)STRCAT(parti->menulabel,", ");
        STRCAT(parti->menulabel,label);
      }
      if(showfiles==1||lenlabel==0){
        if(global_scase.meshescoll.nmeshes>1||lenlabel>0)STRCAT(parti->menulabel,", ");
        STRCAT(parti->menulabel,parti->file);
      }
    }
  }
}
