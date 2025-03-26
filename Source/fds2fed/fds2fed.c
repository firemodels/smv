#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fds2fed.h"
#include "getdata.h"
#include "isobox.h"

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

  streamsmv = GetSMVBuffer(smvfile);

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
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ GRID ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if(
      Match(buffer, "GRID") == 1){
      nmeshinfo++;
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
  if(nmeshinfo > 0){
    int i;

    NewMemory((void **)&meshinfo, nmeshinfo * sizeof(meshdata));
    for(i = 0; i < nmeshinfo; i++){
      meshdata *meshi;

      meshi = meshinfo + i;
      meshi->xplt = NULL;
      meshi->yplt = NULL;
      meshi->zplt = NULL;
      meshi->ibar = 0;
      meshi->jbar = 0;
      meshi->kbar = 0;
    }
  }


  // read in smv file a second time

  islice=0;
  nmeshinfo = 0;
  REWIND(streamsmv);
  while(!FEOF(streamsmv)){
    if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
    CheckMemory;
    if(strncmp(buffer," ",1)==0)continue;
    /*
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      ++++++++++++++++++++++ GRID ++++++++++++++++++++++++++++++
      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    */
    if(Match(buffer, "GRID") == 1){
      meshdata *meshi;
      int ibar, jbar, kbar;

      if(FGETS(buffer, BUFFERSIZE, streamsmv) == NULL)break;
      meshi = meshinfo + nmeshinfo++;
      sscanf(buffer, "%i %i %i", &ibar, &jbar, &kbar);
      meshi->ibar = ibar;
      meshi->jbar = jbar;
      meshi->kbar = kbar;
      if(meshi->ibar>0)NewMemory((void **)&meshi->xplt, (ibar+1)*sizeof(float));
      if(meshi->jbar > 0)NewMemory((void **)&meshi->yplt, (jbar + 1) * sizeof(float));
      if(meshi->kbar > 0)NewMemory((void **)&meshi->zplt, (kbar + 1) * sizeof(float));
      continue;
    }
    if(Match(buffer, "TRNX") == 1|| Match(buffer, "TRNY") == 1|| Match(buffer, "TRNZ") == 1){
      meshdata *meshi;
      int i, dummy;
      int nvals;
      float *vals;

      meshi = meshinfo + nmeshinfo-1;
      nvals = meshi->ibar;
      vals = meshi->xplt;
      if(Match(buffer, "TRNY") == 1){
        nvals = meshi->jbar;
        vals = meshi->yplt;
      }
      if(Match(buffer, "TRNZ") == 1){
        nvals = meshi->kbar;
        vals = meshi->zplt;
      }
      if(FGETS(buffer, BUFFERSIZE, streamsmv) == NULL)break;
      for(i = 0; i <= nvals; i++){
        if(FGETS(buffer, BUFFERSIZE, streamsmv) == NULL)break;
        sscanf(buffer, "%i %f", &dummy, vals + i);
      }
      continue;
    }
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

      strcpy(slicei->keyword_label, buffer);
      TrimBack(slicei->keyword_label);

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

      slicei->is1           = ii1;
      slicei->is2           = ii2;
      slicei->js1           = jj1;
      slicei->js2           = jj2;
      slicei->ks1           = kk1;
      slicei->ks2           = kk2;
      slicei->vals          = NULL;
      slicei->times         = NULL;
      slicei->nframes       = 0;
      slicei->headersize    = 0;
      slicei->diskframesize = 0;
      slicei->memframesize  = 0;
      slicei->vol           = 0;
      if(ii2 != ii1 && jj1 != jj2 && kk1 != kk2)slicei->vol = 1;
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

void MakeFEDFileNames(char *fedslicefile, char *fedisofile, char *bndfedfile, char *slicefile){
  char *ext;

  strcpy(fedslicefile, slicefile);
  ext = strrchr(fedslicefile, '.');
  if(ext != NULL)ext[0]=0;
  strcat(fedslicefile, ".fedsf");

  strcpy(fedisofile, slicefile);
  ext = strrchr(fedisofile, '.');
  if(ext != NULL)ext[0] = 0;
  strcat(fedisofile, ".fediso");

  strcpy(bndfedfile, slicefile);
  ext = strrchr(bndfedfile, '.');
  if(ext != NULL)ext[0] = 0;
  strcat(bndfedfile, ".fedsf.bnd");
}

/* ------------------ AddSlice ------------------------ */

void AddSlice(slicedata *slicei){
  feddata *fedi;
  int i;

  for(i = 0;i < nfedinfo;i++){
    fedi = fedinfo + i;
    if(fedi->co != NULL && slicei != fedi->co && MatchFED(slicei, fedi->co)==1){
      if(slicei->quant == O2)fedi->o2 = slicei;
      if(slicei->quant == CO2)fedi->co2 = slicei;
      fedi->keyword_label = slicei->keyword_label;
      MakeFEDFileNames(fedi->sf_file, fedi->iso_file, fedi->bndfile, slicei->file);
      slicei->in_fed = 1;
      return;
    }
    if(fedi->co2 != NULL && slicei != fedi->co2 && MatchFED(slicei, fedi->co2)==1){
      if(slicei->quant == O2)fedi->o2 = slicei;
      if(slicei->quant == CO)fedi->co = slicei;
      fedi->keyword_label = slicei->keyword_label;
      MakeFEDFileNames(fedi->sf_file, fedi->iso_file, fedi->bndfile, slicei->file);
      slicei->in_fed = 1;
      return;
    }
    if(fedi->o2 != NULL && slicei != fedi->o2 && MatchFED(slicei, fedi->o2)==1){
      if(slicei->quant == CO2)fedi->co2 = slicei;
      if(slicei->quant == CO)fedi->co = slicei;
      fedi->keyword_label = slicei->keyword_label;
      MakeFEDFileNames(fedi->sf_file, fedi->iso_file, fedi->bndfile, slicei->file);
      slicei->in_fed = 1;
      return;
    }
  }
  fedi = fedinfo + nfedinfo++;
  if(slicei->quant == CO2)fedi->co2 = slicei;
  if(slicei->quant == CO)fedi->co = slicei;
  if(slicei->quant == O2)fedi->o2 = slicei;
  MakeFEDFileNames(fedi->sf_file, fedi->iso_file, fedi->bndfile, slicei->file);
  fedi->keyword_label = slicei->keyword_label;
  slicei->in_fed = 1;
}

/* ------------------ MakeFEDSmv ------------------------ */

void MakeFEDSmv(char *file){
  int i;
  FILE *stream;

  if(nfedinfo == 0)return;
  stream = fopen(file, "w");
  if(stream == NULL)return;

  nfedisos = 0;
  for(i = 0;i < nfedinfo;i++){
    feddata *fedi;
    slicedata *fed;

    fedi = fedinfo + i;
    fprintf(stream, "%s\n", fedi->keyword_label);
    fprintf(stream, " %s\n", fedi->sf_file);
    fprintf(stream, " Fractional effective dose\n");
    fprintf(stream, " FED\n");
    fprintf(stream, " \n");

    if(fedi->co  != NULL)fed = fedi->co;
    if(fedi->co2 != NULL)fed = fedi->co2;
    if(fedi->o2  != NULL)fed = fedi->o2;
    if(fed->vol == 1){
      fprintf(stream, "%s %i\n", "ISOF", fed->blocknumber+1);
      fprintf(stream, " %s\n", fedi->iso_file);
      fprintf(stream, " Fractional Effective Dose\n");
      fprintf(stream, " FED\n");
      fprintf(stream, " \n");
      nfedisos++;
    }
  }
  fclose(stream);
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

/* ------------------ GetSliceInfo ------------------------ */

void GetSliceInfo(slicedata *slicei){
  FILE *stream;

  int headersize, framesize, nframes, valframesize;
  float *times, *vals;
  int ijk[6];
  int ip1, ip2, jp1, jp2, kp1, kp2;
  int nxsp, nysp, nzsp;
  
  stream = fopen(slicei->file, "rb");
  if(stream == NULL)return;

  headersize = 3*(4+30+4);

  fseek(stream, 4+headersize, SEEK_CUR);

  fread(ijk, 4, 6, stream);
  rewind(stream);
  fclose(stream);

  ip1 = ijk[0];
  ip2 = ijk[1];
  jp1 = ijk[2];
  jp2 = ijk[3];
  kp1 = ijk[4];
  kp2 = ijk[5];
  headersize += 4+6*4+4;

  nxsp = ip2 + 1 - ip1;
  nysp = jp2 + 1 - jp1;
  nzsp = kp2 + 1 - kp1;

  framesize  = 4 + 4 + 4;                // time
  framesize += 4 + nxsp*nysp*nzsp*4 + 4; // data on disk
  valframesize = nxsp*nysp*nzsp;         // data in memory

  nframes = (int)(GetFileSizeSMV(slicei->file) - headersize) / framesize; // time frames
  NewMemory((void **)&times, nframes*sizeof(float));
  NewMemory((void **)&vals,  nframes*valframesize*sizeof(float));
  slicei->headersize    = headersize;
  slicei->times         = times;
  slicei->vals          = vals;
  slicei->nframes       = nframes;
  slicei->diskframesize = framesize;
  slicei->memframesize  = valframesize;
}

/* ------------------ ReadSlice ------------------------ */

void ReadSlice(slicedata *slicei){
  FILE *STREAM;
  int i;

  if(slicei == NULL)return;
  FREEMEMORY(slicei->vals);
  FREEMEMORY(slicei->times);
  GetSliceInfo(slicei);
  STREAM = fopen(slicei->file, "rb");
  if(STREAM == NULL)return;
  FSEEK(STREAM, slicei->headersize, SEEK_CUR);
  for(i = 0; i < slicei->nframes; i++){
    FORTREAD(slicei->times + i, 1, STREAM);
    FORTREAD(slicei->vals + i*slicei->memframesize, slicei->memframesize, STREAM);
  }
}

/* ------------------ FreeSliceData ------------------------ */

void FreeSliceData(slicedata *slicei){
  if(slicei == NULL)return;
  FREEMEMORY(slicei->vals);
  FREEMEMORY(slicei->times);
}

/* ------------------ FreeFEDData ------------------------ */

void FreeFEDData(feddata *fedi){
  FreeSliceData(fedi->co);
  FreeSliceData(fedi->co2);
  FreeSliceData(fedi->o2);
  FREEMEMORY(fedi->vals);
  FREEMEMORY(fedi->times);
}

/* ------------------ OutputFEDSlice ------------------------ */

void OutputFEDSlice(feddata *fedi){
  writeslicedata(fedi->sf_file, 
    fedi->fed->is1, fedi->fed->is2,
    fedi->fed->js1, fedi->fed->js2,
    fedi->fed->ks1, fedi->fed->ks2,
    fedi->vals, fedi->times, fedi->nframes, 1);
}

/* ------------------ MakeFEDIso ------------------------ */

void MakeFEDIso(feddata *fedi){
  float levels[3] = {0.3, 1.0, 3.0};
  int nlevels = 3;
  int error_local, reduce_triangles=1;
  char *iblank=NULL;
  int nx=0, ny=0, nz=0;
  int i;
  meshdata *isomesh;

  isomesh = meshinfo + fedi->fed->blocknumber;
  nx = isomesh->ibar + 1;
  ny = isomesh->jbar + 1;
  nz = isomesh->kbar + 1;

  CCIsoHeader(fedi->iso_file, "Fractional effective dose", "FED", "", levels, &nlevels, &error_local);
  for(i = 0; i < fedi->nframes; i++){
    float *vals;

    vals = fedi->vals + i * fedi->memframesize;
    CCIsoSurface2File(fedi->iso_file, fedi->times + i, vals, iblank, levels, &nlevels,
      isomesh->xplt, &nx, isomesh->yplt, &ny, isomesh->zplt, &nz, &reduce_triangles, &error_local);
  }
  int output = 0;
  if(isocount == 0 || isocount == nfedisos - 1 || nfedisos < 100 || (nfedisos >= 100 && nfedisos < 1000 && isocount % 10 == 1))output = 1;
  if(nfedisos >= 1000 && isocount % 100 == 1)output = 1;
  if(output == 1)printf("fed isosurface %i of %i generated\n", isocount + 1, nfedisos);
  isocount++;
}

/* ------------------ MakeFEDSlice ------------------------ */

void MakeFEDSlice(feddata *fedi){
  float *vals, *times, *timesfrom = NULL;
  float valmin, valmax;
  int nframes = 1000000000;

  ReadSlice(fedi->co);
  ReadSlice(fedi->co2);
  ReadSlice(fedi->o2);
  fedi->vals         = NULL;
  fedi->times        = NULL;
  fedi->memframesize = 0;
  fedi->fed = NULL;
  if(fedi->co != NULL && fedi->co->nframes > 0){
    fedi->memframesize = fedi->co->memframesize;
    timesfrom = fedi->co->times;
    fedi->fed = fedi->co;
  }
  if(fedi->nframes == 0 && fedi->co2 != NULL && fedi->co2->nframes > 0){
    fedi->memframesize = fedi->co2->memframesize;
    timesfrom = fedi->co2->times;
    fedi->fed = fedi->co2;
  }
  if(fedi->nframes == 0 && fedi->o2 != NULL && fedi->o2->nframes > 0){
    fedi->memframesize = fedi->o2->memframesize;
    timesfrom = fedi->o2->times;
    fedi->fed = fedi->o2;
  }
  if(fedi->co != NULL)nframes = MIN(nframes, fedi->co->nframes);
  if(fedi->co2 != NULL)nframes = MIN(nframes, fedi->co2->nframes);
  if(fedi->o2 != NULL)nframes = MIN(nframes, fedi->o2->nframes);
  fedi->nframes = nframes;
  if(fedi->co != NULL)fedi->co->nframes = nframes;
  if(fedi->co2 != NULL)fedi->co2->nframes = nframes;
  if(fedi->o2 != NULL)fedi->o2->nframes = nframes;
  if(fedi->nframes > 0){
    int i;
    float fedo20, hvco20, fedco0;

    NewMemory((void **)&times, fedi->nframes * sizeof(float));
    NewMemory((void **)&vals, fedi->nframes * fedi->memframesize * sizeof(float));
    fedi->times = times;
    fedi->vals = vals;
    for(i = 0; i < fedi->memframesize; i++){
      vals[i] = 0.0;
    }
    valmin  = 0.0;
    valmax  = 0.0;
    fedo20  = FEDO2(0.209);
    hvco20  = HVCO2(0.0);
    fedco0  = FEDCO(0.0);
    memcpy(fedi->times, timesfrom, fedi->nframes * sizeof(float));
    for(i = 1; i < fedi->nframes; i++){
      int j;
      float dt, *vali, *valim1;
      float *coi=NULL, *co2i=NULL, *o2i=NULL;

      dt = fedi->times[i] - fedi->times[i-1];
      vali   = vals + i*fedi->memframesize;
      valim1 = vali - fedi->memframesize;
      if(fedi->co!=NULL)coi   = fedi->co->vals  + i*fedi->memframesize;
      if(fedi->co2!=NULL)co2i = fedi->co2->vals + i*fedi->memframesize;
      if(fedi->o2!=NULL)o2i   = fedi->o2->vals  + i*fedi->memframesize;
      for(j=0;j<fedi->memframesize;j++){
        float fedval, fedo2, hvco2, fedco;

        fedco  = fedco0;
        hvco2 = hvco20;
        fedo2  = fedo20;
        if(fedi->co!=NULL)fedco   = FEDCO(coi[j]);
        if(fedi->co2!=NULL)hvco2  = HVCO2(co2i[j]);
        if(fedi->o2!=NULL)fedo2   = FEDO2(o2i[j]);
        fedval = fedco*hvco2 + fedo2;
        vali[j] = valim1[j] + dt*fedval;
        valmin = MIN(valmin, vali[j]);
        valmax = MAX(valmax, vali[j]);
      }
    }
    if(fedi->fed->vol == 1){
      MakeFEDIso(fedi);
    }
    FILE *stream;

    stream = fopen(fedi->bndfile, "w");
    fprintf(stream, "%f %f %f\n", 0.0, valmin, valmax);
    OutputFEDSlice(fedi);
  }
  FreeFEDData(fedi);
}

/* ------------------ MakeFEDSlices ------------------------ */

void MakeFEDSlices(void){
  int i;

  if(nfedinfo == 0)printf("no fed slices were found\n");
  if(nfedisos == 0)printf("no fed isosurfaces were found\n");
  for(i = 0; i < nfedinfo; i++){
    feddata *fedi;
    int output;

    fedi = fedinfo + i;
    MakeFEDSlice(fedi);
    output = 0;
    if(i == 0 || i == nfedinfo - 1||nfedinfo<100||(nfedinfo >= 100 && nfedinfo<1000 && i % 10 == 1))output = 1;
    if(nfedinfo >= 1000 && i % 100 == 1)output = 1;
    if(output==1)printf("fed slice %i of %i generated\n", i + 1, nfedinfo);
  }
}
