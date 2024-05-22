#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fds2fed.h"

/* ------------------ ReadSMV ------------------------ */

int ReadSMV(char *smvfile){
  FILE *stream = NULL;
  bufferstreamdata *streamsmv;
  int ioffset;
  int unit_start=15;
  int igrid,ipdim;
  int islice;
#define BUFFERSIZE 255
  char buffer[BUFFERSIZE];

  igrid=0;
  ipdim=0;
  stream=fopen(smvfile,"r");
  if(stream==NULL){
    PRINTF("The file: %s could not be opened\n",smvfile);
    return 1;
  }
  fclose(stream);

  streamsmv = GetSMVBuffer(smvfile, NULL);

  while(!FEOF(streamsmv)){
    if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
    CheckMemory;
    if(strncmp(buffer," ",1)==0)continue;

  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ SLCF ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(
      Match(buffer,"SLCF") == 1||
      Match(buffer,"SLCC") == 1||
      Match(buffer, "SLCD") == 1 ||
      Match(buffer,"SLCT") == 1
      ){
      nsliceinfo++;
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ GRID ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(Match(buffer,"GRID") == 1){
      nmeshes++;
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ PDIM ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(Match(buffer,"PDIM") == 1){
      ipdim++;
      continue;
    }
  }


  if(nmeshes>0&&nmeshes==ipdim){
    NewMemory((void **)&meshinfo,nmeshes*sizeof(meshdata));
  }

  // allocate memory for slice file info

  if(nsliceinfo>0){
    slicedata *slicei;
    int i;

    NewMemory((void **)&sliceinfo,   nsliceinfo*sizeof(slicedata));
    for(i=0;i<nsliceinfo;i++){
      slicei = sliceinfo + i;
      slicei->file=NULL;
      slicei->filebase=NULL;
    }
  }

  // read in smv file a second time_local to compress files

  ioffset=0, islice=0, ipdim=0, igrid=0;
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
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ PDIM ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(Match(buffer,"PDIM") == 1){
      meshdata *meshi;

      meshi=meshinfo+ipdim;
      ipdim++;
      FGETS(buffer,BUFFERSIZE,streamsmv);
      sscanf(buffer,"%f %f %f %f %f %f",&meshi->xbar0,&meshi->xbar,&meshi->ybar0,&meshi->ybar,&meshi->zbar0,&meshi->zbar);
      continue;
    }
   /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ SLCF ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(
      Match(buffer,"SLCF") == 1||
      Match(buffer,"SLCC") == 1||
      Match(buffer, "SLCD") == 1 ||
      Match(buffer,"SLCT") == 1)
    {
      int version_local=0,dummy;
      char *buffer2;
      int len;
      FILE_SIZE filesize;
      slicedata *slicei;
      int blocknumber;

      len=strlen(buffer);
      if(len>4){
        buffer2=buffer+4;
        sscanf(buffer2,"%i %i",&dummy,&version_local);
      }

      if(nmeshes>1){
        blocknumber=ioffset-1;
      }
      else{
        blocknumber=0;
      }
      if(len>5){
        buffer2=buffer+4;
        sscanf(buffer2,"%i",&blocknumber);
        blocknumber--;
      }

      slicei = sliceinfo + islice;
      slicei->blocknumber=blocknumber;

      if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
      TrimBack(buffer);
      buffer2=TrimFront(buffer);
      if(strlen(buffer2)==0)break;
      if(GetFileInfo(buffer2,GLOBsourcedir,&filesize)==0){
        int lendir=0;

        if(GLOBsourcedir!=NULL)lendir=strlen(GLOBsourcedir);
        NewMemory((void **)&slicei->file,(unsigned int)(strlen(buffer2)+lendir+1));
        NewMemory((void **)&slicei->filebase,(unsigned int)(strlen(buffer2)+1));

        STRCPY(slicei->filebase,buffer2);
        STRCPY(slicei->file,"");
        if(GLOBsourcedir!=NULL){
          STRCAT(slicei->file,GLOBsourcedir);
        }
        STRCAT(slicei->file,buffer2);

        if(ReadLabels(&slicei->label,streamsmv,NULL)==LABEL_ERR){
          fprintf(stderr,"*** Warning: problem reading SLCF entry\n");
          break;
        }
        islice++;
      }
      else{
        fprintf(stderr,"*** Warning: the file, %s, does not exist.\n",buffer2);
        if(ReadLabels(&sliceinfo[islice].label,streamsmv,NULL)==LABEL_ERR)break;
        nsliceinfo--;
      }
      continue;
    }
  }
  return 0;
}

