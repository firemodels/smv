#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "part2iso.h"
#include "dmalloc.h"
#include "stdio_buffer.h"
#include "getdata.h"

/* ------------------ ReadSMV ------------------------ */

int ReadSMV(char *smvfile){
  FILE *stream = NULL;
  bufferstreamdata *streamsmv;
  int igrid,ipdim;
#define BUFFERSIZE 255
  char buffer[BUFFERSIZE];

  ipdim=0;
  stream=FOPEN(smvfile,"r");
  if(stream==NULL){
    PRINTF("The file: %s could not be opened\n",smvfile);
    return 1;
  }
  fclose(stream);

  streamsmv = GetSMVBuffer(smvfile);
  if(streamsmv==NULL)return 1;

  while(!FEOF(streamsmv)){
    if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
    CheckMemory;
    if(strncmp(buffer," ",1)==0)continue;

    if(Match(buffer,"CLASS_OF_PARTICLES") == 1){
      int i,nclasses;

      npartclassinfo++;
      FGETS(buffer,BUFFERSIZE,streamsmv);
      FGETS(buffer,BUFFERSIZE,streamsmv);
      FGETS(buffer,BUFFERSIZE,streamsmv);
      sscanf(buffer,"%i",&nclasses);
      if(nclasses>0)maxpart5propinfo+=nclasses;
      for(i=0; i<nclasses; i++){
        FGETS(buffer,BUFFERSIZE,streamsmv);
      }
      continue;
    }

    if(Match(buffer,"PRT5") == 1){
      npartinfo++;
      continue;
    }

    if(Match(buffer,"GRID") == 1){
      nmeshes++;
      continue;
    }

    if(Match(buffer,"PDIM") == 1){
      ipdim++;
      continue;
    }
  }

  if(nmeshes>0&&nmeshes==ipdim){
    NewMemory((void **)&meshinfo,nmeshes*sizeof(meshdata));
  }
  else{ return 1; }

  // allocate memory for particle file info

  if(npartinfo>0){
    part *parti;
    int i;

    NewMemory((void **)&partinfo,npartinfo*sizeof(part));
    for(i=0; i<npartinfo; i++){
      parti = partinfo + i;
      parti->file=NULL;
    }
  }
  if(npartclassinfo>0){
    NewMemory((void **)&partclassinfo,npartclassinfo*sizeof(partclassdata));
  }
  if(maxpart5propinfo>0){
    NewMemory((void **)&part5propinfo,maxpart5propinfo*sizeof(partpropdata));
  }

  // read the mesh and particle metadata on the second pass

  npartclassinfo=0;
  npart5propinfo=0;
  npartinfo=0;
  ipdim=0;
  igrid=0;
  REWIND(streamsmv);

  while(!FEOF(streamsmv)){
    if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
    CheckMemory;
    if(strncmp(buffer," ",1)==0)continue;

    if(Match(buffer,"GRID") == 1){
      meshdata *meshi;

      meshi=meshinfo+igrid;
      igrid++;
      FGETS(buffer,BUFFERSIZE,streamsmv);
      sscanf(buffer,"%i %i %i",&meshi->ibar,&meshi->jbar,&meshi->kbar);
      continue;
    }

    if(Match(buffer,"PDIM") == 1){
      meshdata *meshi;

      meshi=meshinfo+ipdim;
      ipdim++;
      FGETS(buffer,BUFFERSIZE,streamsmv);
      sscanf(buffer,"%f %f %f %f %f %f",&meshi->xbar0,&meshi->xbar,&meshi->ybar0,&meshi->ybar,&meshi->zbar0,&meshi->zbar);
      continue;
    }

    if(Match(buffer,"CLASS_OF_PARTICLES") == 1){
      partclassdata *partclassi;
      int j;
      char *percen;

      partclassi = partclassinfo + npartclassinfo;

      FGETS(buffer,BUFFERSIZE,streamsmv);
      percen=strchr(buffer,'%');
      if(percen!=NULL)*percen=0;
      TrimBack(buffer);
      NewMemory((void **)&partclassi->name,strlen(buffer)+1);
      strcpy(partclassi->name,buffer);

      FGETS(buffer,BUFFERSIZE,streamsmv);

      FGETS(buffer,BUFFERSIZE,streamsmv);
      sscanf(buffer,"%i",&partclassi->ntypes);
      if(partclassi->ntypes>0){
        NewMemory((void **)&partclassi->labels,partclassi->ntypes*sizeof(flowlabels));
        for(j=0; j<partclassi->ntypes; j++){
          flowlabels *labelj;
          partpropdata *part5propi;

          labelj = partclassi->labels+j;
          labelj->longlabel=NULL;
          labelj->shortlabel=NULL;
          labelj->unit=NULL;
          ReadLabels(labelj,streamsmv,NULL);
          part5propi=GetPartProp(labelj->shortlabel);
          if(part5propi==NULL){
            part5propi = part5propinfo + npart5propinfo;
            part5propi->label.longlabel=labelj->longlabel;
            part5propi->label.shortlabel=labelj->shortlabel;
            part5propi->label.unit=labelj->unit;
            npart5propinfo++;
          }
        }
      }
      npartclassinfo++;
      continue;
    }

    if(Match(buffer,"PRT5") == 1){
      int meshindex=0;
      int i;
      char *buffer2;
      int len;
      part *parti;
      FILE_SIZE filesize;

      len=strlen(buffer);
      if(len>4){
        buffer2=buffer+4;
        sscanf(buffer2,"%i",&meshindex);
        meshindex--;

      }

      if(meshindex<0||meshindex>=nmeshes)return 1;
      parti = partinfo + npartinfo;
      parti->partmesh = meshinfo + meshindex;

      if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
      TrimBack(buffer);
      buffer2=TrimFront(buffer);
      if(strlen(buffer2)==0)break;
      if(GetFileInfo(buffer2,GLOBsourcedir,&filesize)==0){
        int lendir=0;

        if(GLOBsourcedir!=NULL)lendir=strlen(GLOBsourcedir);
        NewMemory((void **)&parti->file,(unsigned int)(strlen(buffer2)+lendir+1));
        if(GLOBsourcedir!=NULL){
          STRCPY(parti->file,GLOBsourcedir);
          STRCAT(parti->file,buffer2);
        }
        else{
          STRCPY(parti->file,buffer2);
        }
        parti->filesize=filesize;
        npartinfo++;
      }
      else{
        fprintf(stderr,"*** Warning: the file, %s, does not exist.\n",buffer2);
      }
      if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
      sscanf(buffer,"%i",&parti->nclasses);
      if(parti->nclasses>0){
        NewMemory((void **)&parti->classptr,parti->nclasses*sizeof(partclassdata *));
      }
      else{
        parti->nclasses=0;
      }
      for(i=0; i<parti->nclasses; i++){
        int classindex;

        if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
        sscanf(buffer,"%i",&classindex);
        parti->classptr[i]=partclassinfo + classindex - 1;
      }
      continue;
    }

  }
  {
    int i;

    for(i=0; i<nmeshes; i++){
      meshdata *meshi;
      int ii;
      float *xpltcell, *ypltcell, *zpltcell;

      meshi = meshinfo + i;
      meshi->dx = (meshi->xbar-meshi->xbar0)/meshi->ibar;
      meshi->dy = (meshi->ybar-meshi->ybar0)/meshi->jbar;
      meshi->dz = (meshi->zbar-meshi->zbar0)/meshi->kbar;
      meshi->xpltcell=NULL;
      NewMemory((void **)&meshi->xpltcell,(meshi->ibar+2)*sizeof(float));
      xpltcell = meshi->xpltcell+1;
      for(ii=-1; ii<meshi->ibar+1; ii++){
        xpltcell[ii] = meshi->xbar0 + (ii+0.5)*meshi->dx;
      }
      CheckMemory;

      meshi->ypltcell=NULL;
      NewMemory((void **)&meshi->ypltcell,(meshi->jbar+2)*sizeof(float));
      ypltcell = meshi->ypltcell+1;
      for(ii=-1; ii<meshi->jbar+1; ii++){
        ypltcell[ii] = meshi->ybar0 + (ii+0.5)*meshi->dy;
      }
      CheckMemory;

      meshi->zpltcell=NULL;
      NewMemory((void **)&meshi->zpltcell,(meshi->kbar+2)*sizeof(float));
      zpltcell = meshi->zpltcell+1;
      for(ii=-1; ii<meshi->kbar+1; ii++){
        zpltcell[ii] = meshi->zbar0 + (ii+0.5)*meshi->dz;
      }
      CheckMemory;

    }
  }
  fclose_buffer(streamsmv->fileinfo);
  FREEMEMORY(streamsmv);
  return 0;
}

