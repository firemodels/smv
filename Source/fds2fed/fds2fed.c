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
  int islice;
#define BUFFERSIZE 255
  char buffer[BUFFERSIZE];

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
      Match(buffer,"SLCF") == 1 || Match(buffer,"SLCC") == 1 || Match(buffer, "SLCD") == 1 || Match(buffer,"SLCT") == 1){
      nsliceinfo++;
      continue;
    }
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

  // read in smv file a second time

  islice=0;
  REWIND(streamsmv);
  while(!FEOF(streamsmv)){
    if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
    CheckMemory;
    if(strncmp(buffer," ",1)==0)continue;
   /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ SLCF ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(Match(buffer, "SLCF") == 1 || Match(buffer, "SLCC") == 1 || Match(buffer, "SLCD") == 1 || Match(buffer,"SLCT") == 1){
      int version_local=0,dummy;
      char *buffer2, *sliceparms, *shortlabel;
      int len, blocknumber, slicetype;
      int ii1, ii2, jj1, jj2, kk1, kk2;
      slicedata *slicei;

      if(Match(buffer, "SLCF") == 1)slicetype = SLCF;
      if(Match(buffer, "SLCC") == 1)slicetype = SLCC;
      if(Match(buffer, "SLCD") == 1)slicetype = SLCD;
      if(Match(buffer, "SLCT") == 1)slicetype = SLCT;

      len=strlen(buffer);
      if(len>4){
        buffer2=buffer+4;
        sscanf(buffer2,"%i %i",&dummy,&version_local);
      }
      if(len <= 5){
        nsliceinfo--;
        continue;
      }
      buffer2=buffer+4;
      sscanf(buffer2,"%i",&blocknumber);
      blocknumber--;

      slicei = sliceinfo + islice;
      slicei->slicetype = slicetype;
      slicei->blocknumber=blocknumber;
      sliceparms = strchr(buffer, '&');
      if(sliceparms==NULL){
        nsliceinfo--;
        continue;
      }
      sliceparms[0] = 0;
      sscanf(sliceparms, "%i %i %i %i %i %i", &ii1, &ii2, &jj1, &jj2, &kk1, &kk2);

      if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
      TrimBack(buffer);
      buffer2=TrimFront(buffer);
      if(strlen(buffer2)==0)break;
      NewMemory((void **)&slicei->file,(unsigned int)(strlen(buffer2)+1));
      NewMemory((void **)&slicei->filebase,(unsigned int)(strlen(buffer2)+1));

      STRCPY(slicei->filebase,buffer2);
      STRCPY(slicei->file,"");
      STRCAT(slicei->file,buffer2);
      if(ReadLabels(&slicei->label,streamsmv,NULL)==LABEL_ERR){
        fprintf(stderr,"*** Warning: problem reading SLCF entry\n");
        break;
      }
      shortlabel = slicei->label.shortlabel;
      slicei->quant = OTHER;
      if(strcmp(shortlabel, "X_O2")  == 0)slicei->quant = O2;
      if(strcmp(shortlabel, "X_CO2") == 0)slicei->quant = CO2;
      if(strcmp(shortlabel, "X_CO")  == 0)slicei->quant = CO;
      if(slicei->quant=OTHER){
        nsliceinfo--;
        continue;
      }
      islice++;

      if(slicetype == SLCC){
        ii1 = MAX(ii1, 1);
        ii2 = MAX(ii1, ii2);
      }
      slicei->is1 = ii1;
      slicei->is2 = ii2;
      slicei->js1 = jj1;
      slicei->js2 = jj2;
      slicei->ks1 = kk1;
      slicei->ks2 = kk2;
      continue;
    }
  }
  return 0;
}

