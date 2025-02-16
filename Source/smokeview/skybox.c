#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "glew.h"
#include GLUT_H

#include "smokeviewvars.h"
#include "readimage.h"

/* ------------------ LoadSkyTexture ------------------------ */

void LoadSkyTexture(char *filebase, texturedata *texti){
  char *filebuffer=NULL;
  int texwid, texht;
  int errorcode;
  unsigned char *floortex;
  int is_transparent;

  TrimBack(filebase);
  texti->name=0;
  texti->loaded=0;
  if(strcmp(filebase,"NULL")==0)return;
  NewMemory((void **)&filebuffer,strlen(filebase)+1);
  STRCPY(filebuffer,filebase);

  glGenTextures(1,&texti->name);
  glBindTexture(GL_TEXTURE_2D,texti->name);
  floortex=ReadPicture(global_scase.texturedir, filebuffer,&texwid,&texht,&is_transparent,0);
  texti->is_transparent = is_transparent;
  if(floortex==NULL){
    FREEMEMORY(filebuffer);
    return;
  }
  errorcode=gluBuild2DMipmaps(GL_TEXTURE_2D,4, texwid, texht, GL_RGBA, GL_UNSIGNED_BYTE, floortex);
  if(errorcode!=0){
    FREEMEMORY(floortex);
    FREEMEMORY(filebuffer);
    return;
  }
  FREEMEMORY(floortex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  #ifdef pp_GPU
  if(gpuactive==1){
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  else{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  }
  #else
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  #endif
  texti->file=filebuffer;
  texti->loaded=1;
  return;
}
/* ------------------ FreeSkybox ------------------------ */

void FreeSkybox(void){
  int i;
  skyboxdata *skyi;

  for(i=0;i<nskyboxinfo;i++){
    int j;

    skyi = skyboxinfo + i;
    for(j=0;j<6;j++){
      FREEMEMORY(skyi->face[j].file);
    }
  }
  FREEMEMORY(skyboxinfo);
  nskyboxinfo=0;
}

#ifdef pp_SKYBOX_FLOOR
/* ------------------ DrawFloor ------------------------ */

void DrawFloor(void){
  int i;

/* stuff min and max grid data into a more convenient form
  assuming the following grid numbering scheme

       5-------6
     / |      /|
   /   |     / |
  4 -------7   |
  |    |   |   |
  Z    1---|---2
  |  Y     |  /
  |/       |/
  0--X-----3

  */
  float points[]={
    0.0,0.0,0.0,
    0.0,1.0,0.0,
    1.0,1.0,0.0,
    1.0,0.0,0.0,
    0.0,0.0,1.0,
    0.0,1.0,1.0,
    1.0,1.0,1.0,
    1.0,0.0,1.0
  };
  float normals[]={
     0.0,-1.0, 0.0,
    -1.0, 0.0, 0.0,
     0.0, 1.0, 0.0,
     1.0, 0.0, 0.0,
     0.0, 0.0, 1.0,
     0.0, 0.0,-1.0
  };
  int faces[]={
    1,2,6,5,
    2,3,7,6,
    3,0,4,7,
    0,1,5,4,
    0,3,2,1,
    5,6,7,4
  };
  float *xyz;
  float *normal;
  int *faceptr;

  for(i=0;i<8;i++){
    xyz = points + 3*i;
    xyz[0] = 5.0*(xyz[0]-0.5);
    xyz[1] = 5.0*(xyz[1]-0.5);
    xyz[2] = 0.0;
  }

  glDisable(GL_BLEND);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
  glEnable(GL_TEXTURE_2D);

  for(i=4;i<5;i++){

    if(skyboxinfo->face[i].file==NULL)continue;

    glBindTexture(GL_TEXTURE_2D,skyboxinfo->face[i].name);
    glBegin(GL_QUADS);

    normal = normals + 3*i;
    faceptr = faces + 4*i;

    glNormal3fv(normal);
    glTexCoord2f(0.0,0.0);
    xyz = points + 3*faceptr[0];
    glVertex3fv(xyz);

    glTexCoord2f(1.0,0.0);
    xyz = points + 3*faceptr[1];
    glVertex3fv(xyz);

    glTexCoord2f(1.0,1.0);
    xyz = points + 3*faceptr[2];
    glVertex3fv(xyz);

    glTexCoord2f(0.0,1.0);
    xyz = points + 3*faceptr[3];
    glVertex3fv(xyz);
    glEnd();

  }
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
}
#endif

/* ------------------ DrawSkybox ------------------------ */

void DrawSkybox(void){
  int i;

  /* stuff min and max grid data into a more convenient form
  assuming the following grid numbering scheme

      5--------6
    / |      / |
   /  |     /  |
  4--------7   |
  |   |    |   |
  Z   1----|---2
  |  Y     |  /
  |/       |/
  0--X-----3

  */
  float points[] = {
    0.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, 0.0, 0.0,
    0.0, 0.0, 1.0,
    0.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 0.0, 1.0
  };
  float normals[] = {
    0.0, -1.0, 0.0,  // ymax
    -1.0, 0.0, 0.0,  // xmax
    0.0, 1.0, 0.0,   // ymin
    1.0, 0.0, 0.0,   // xmin
    0.0, 0.0, 1.0,   // zmin
    0.0, 0.0, -1.0   // zmax
  };
  int faces[] = {
    1, 2, 6, 5,
    2, 3, 7, 6,
    3, 0, 4, 7,
    0, 1, 5, 4,
    0, 3, 2, 1,
    5, 6, 7, 4
  };
  int *faceptr;

  for(i = 0; i<8; i++){
    float *xyz;
    float skybox_diam=3.0;

    xyz = points+3*i;
    xyz[0] = skybox_diam*(xyz[0]-0.5)+camera_current->eye[0];
    xyz[1] = skybox_diam*(xyz[1]-0.5)+camera_current->eye[1];
    xyz[2] = skybox_diam*(xyz[2]-0.5)+camera_current->eye[2];
  }

  glDisable(GL_BLEND);
  glDepthMask(GL_FALSE);
  glDisable(GL_DEPTH_TEST);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glEnable(GL_TEXTURE_2D);

  for(i = 0; i<6; i++){
    float *normal, *xyz;

    if(skyboxinfo->face[i].file==NULL)continue;

    glBindTexture(GL_TEXTURE_2D, skyboxinfo->face[i].name);
    glBegin(GL_QUADS);

    normal = normals+3*i;
    faceptr = faces+4*i;

    glNormal3fv(normal);
    glTexCoord2f(0.0, 0.0);
    xyz = points+3*faceptr[0];
    glVertex3fv(xyz);

    glTexCoord2f(1.0, 0.0);
    xyz = points+3*faceptr[1];
    glVertex3fv(xyz);

    glTexCoord2f(1.0, 1.0);
    xyz = points+3*faceptr[2];
    glVertex3fv(xyz);

    glTexCoord2f(0.0, 1.0);
    xyz = points+3*faceptr[3];
    glVertex3fv(xyz);
    glEnd();

  }
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glEnable(GL_BLEND);
#ifdef pp_SKYBOX_FLOOR
  DrawFloor();
#endif
}
