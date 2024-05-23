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
      strcpy(slicei->kwlabel, buffer);
      char *exclame;
      exclame = strchr(slicei->kwlabel, '!');
      if(exclame != NULL)exclame[0] = 0;
      TrimBack(slicei->kwlabel);
      slicei->slicetype = slicetype;
      slicei->blocknumber=blocknumber;
      sliceparms = strchr(buffer, '&');
      if(sliceparms==NULL){
        nsliceinfo--;
        continue;
      }
      sliceparms++;
      sliceparms[-1] = 0;
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
      if(slicei->quant==OTHER){
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
      slicei->in_fed = 0;
      continue;
    }
  }
  return 0;
}

/* ------------------ MatchFED ------------------------ */

int MatchFED(slicedata *slicei, slicedata *slicej){
  if(slicei->blocknumber != slicej->blocknumber)return 0;
  if(slicei->is1 != slicej->is1 || slicei->is2 != slicej->is2)return 0;
  if(slicei->js1 != slicej->js1 || slicei->js2 != slicej->js2)return 0;
  if(slicei->ks1 != slicej->ks1 || slicei->ks2 != slicej->ks2)return 0;
  if(slicei->slicetype != slicej->slicetype)return 0;
  return 1;
}

/* ------------------ MakeFEDSliceFileName ------------------------ */

void MakeFEDSliceFileName(char *fedfile, char *slicefile){
  char *ext;

  strcpy(fedfile, slicefile);
  ext = strrchr(fedfile, '.');
  if(ext != NULL)ext[0]=0;
  strcat(fedfile, ".fedsf");
}

/* ------------------ AddSlice ------------------------ */

void AddSlice(slicedata *slicei){
  feddata *fedi;
  int i;

  for(i = 0;i < nfedinfo;i++){
    fedi = fedinfo + i;
    if(fedi->co != NULL && slicei != fedi->co && MatchFED(slicei, fedi->co)==1){
      if(slicei->quant == O2)fedi->o2 = slicei;;
      if(slicei->quant == CO2)fedi->co2 = slicei;
      fedi->kwlabel = slicei->kwlabel;
      MakeFEDSliceFileName(fedi->file, slicei->file);
      slicei->in_fed = 1;
      return;
    }
    if(fedi->co2 != NULL && slicei != fedi->co2 && MatchFED(slicei, fedi->co2)==1){
      if(slicei->quant == O2)fedi->o2 = slicei;;
      if(slicei->quant == CO)fedi->co = slicei;
      fedi->kwlabel = slicei->kwlabel;
      MakeFEDSliceFileName(fedi->file, slicei->file);
      slicei->in_fed = 1;
      return;
    }
    if(fedi->o2 != NULL && slicei != fedi->o2 && MatchFED(slicei, fedi->o2)==1){
      if(slicei->quant == CO2)fedi->co2 = slicei;;
      if(slicei->quant == CO)fedi->co = slicei;
      fedi->kwlabel = slicei->kwlabel;
      MakeFEDSliceFileName(fedi->file, slicei->file);
      slicei->in_fed = 1;
      return;
    }
  }
  fedi = fedinfo + nfedinfo++;
  if(slicei->quant == CO2)fedi->co2 = slicei;;
  if(slicei->quant == CO)fedi->co = slicei;
  if(slicei->quant == O2)fedi->o2 = slicei;
  MakeFEDSliceFileName(fedi->file, slicei->file);
  fedi->kwlabel = slicei->kwlabel;
  slicei->in_fed = 1;
}

/* ------------------ MakeFEDSmv ------------------------ */

void MakeFEDSmv(char *file){
  int i;
  FILE *stream;

  if(nfedinfo == 0)return;
  stream = fopen(file, "w");
  if(stream == NULL)return;

  for(i = 0;i < nfedinfo;i++){
    feddata *fedi;

    fedi = fedinfo + i;
    fprintf(stream, "%s\n", fedi->kwlabel);
    fprintf(stream, " %s\n", fedi->file);
    fprintf(stream, " Fractional Effective Dose\n");
    fprintf(stream, " FED\n");
    fprintf(stream, " \n");
  }
  fclose(stream);
}

/* ------------------ MakeFEDSlice ------------------------ */

void MakeFEDSlice(feddata *fedi){
  FILE *stream;

  stream = fopen(fedi->file, "wb");
  if(stream == NULL)return;

  fclose(stream);
}

/* ------------------ MakeFEDSlices ------------------------ */

void MakeFEDSlices(void){
  int i;

  for(i = 0;i < nfedinfo;i++){
    feddata *fedi;

    fedi = fedinfo + i;
    MakeFEDSlice(fedi);
  }
}

/* ------------------ MakeFED ------------------------ */

void MakeFED(void){
  int i;

  nfedinfo = 0;
  if(nsliceinfo == 0)return;
  NewMemory((void **)&fedinfo, nsliceinfo * sizeof(feddata));
  for(i = 0;i < nsliceinfo; i++){
    feddata *fedi;

    fedi      = fedinfo + i;
    fedi->co  = NULL;
    fedi->co2 = NULL;
    fedi->o2  = NULL;
  }
  for(i = 0;i < nsliceinfo;i++){
    slicedata *slicei;

    slicei = sliceinfo + i;
    AddSlice(slicei);
  }
}

