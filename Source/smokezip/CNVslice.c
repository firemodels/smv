#include "options.h"
#include "zlib.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <zlib.h>
#include "svzip.h"
#include "dmalloc.h"
#include "compress.h"
#include "getdata.h"

#define FORTSLICEREAD(var,size) FSEEK(SLICEFILE,4,SEEK_CUR);\
                           returncode=fread(var,4,size,SLICEFILE);\
                           FSEEK(SLICEFILE,4,SEEK_CUR)

/* ------------------ GetPatchBoundInfo ------------------------ */

bounddata *GetSliceBoundInfo(char *label){
  int i;

  for(i = 0; i < nslicebounds; i++){
    bounddata *boundi;
    char *labeli;

    boundi = slicebounds + i;
    labeli = boundi->label;
    if(strcmp(label, labeli) == 0)return boundi;
  }
  return NULL;
}

/* ------------------ InitSliceBounds ------------------------ */

void InitSliceBounds(void){
  int i;

  nslicebounds = 0;
  for(i = 0; i < nsliceinfo; i++){
    slicedata *slicei;
    bounddata *pb;

    slicei = sliceinfo + i;
    if(GetSliceBoundInfo(slicei->label.shortlabel) == NULL){
      pb = slicebounds + nslicebounds;
      strcpy(pb->label, slicei->label.shortlabel);
      pb->valmin = 1000000000.0;
      pb->valmax = -pb->valmin;
      pb->setvalmin = 0;
      pb->setvalmax = 0;
      nslicebounds++;
    }
  }
}

/* ------------------ GetSliceBounds ------------------------ */

void GetSliceBounds(void){
  int i;

  for(i = 0;i < nslicebounds;i++){
    bounddata *pbi;
    int j;

    pbi = slicebounds + i;
    if(pbi->setvalmin == 0 || pbi->setvalmax == 0){
      for(j = 0;j < nsliceinfo;j++){  // bound computed from .bnd files
        slicedata *slicej;

        slicej = sliceinfo + j;
      if(strcmp(pbi->label, slicej->label.shortlabel) != 0)continue;
        if(GetFileBounds(slicej->boundfile, &(slicej->valmin), &(slicej->valmax)) == 1){
          pbi->valmin = MIN(pbi->valmin, slicej->valmin);
          pbi->valmax = MAX(pbi->valmax, slicej->valmax);
          pbi->setvalmin = 1;
          pbi->setvalmax = 1;
        }
      }
    }
    if(pbi->setvalmin == 1 && pbi->setvalmax == 1){
      for(j = 0;j < nsliceinfo;j++){  // bound computed from .bnd files
        slicedata *slicej;

        slicej = sliceinfo + j;
        if(strcmp(pbi->label, slicej->label.shortlabel) != 0)continue;
        slicej->setvalmin = 1;
        slicej->setvalmax = 1;
        slicej->valmin = pbi->valmin;
        slicej->valmax = pbi->valmax;
      }
    }
  }
}

/* ------------------ ConvertVolSlice ------------------------ */

int ConvertVolSlice(slicedata *slicei, int *thread_index){
  char slicefile_svz[1024];
  char *slice_file;
  char filetype[1024];
  char *shortlabel;
  int ijkbar[6];
  uLong framesize;
  float *sliceframe_data=NULL;
  int sizebefore, sizeafter;
  int returncode=0;
  LINT data_loc;
  int percent_done;
  int percent_next = PERCENT_SKIP;
#ifndef pp_THREAD
  int count=0;
#endif
  FILE *SLICEFILE;
  FILE *slicestream;

#ifdef pp_THREAD
  if(GLOBcleanfiles==0){
    int fileindex;

    fileindex = slicei + 1 - sliceinfo;
    sprintf(threadinfo[*thread_index].label,"vsf %i",fileindex);
  }
#endif

  slice_file=slicei->file;

  // check if slice file is accessible

  strcpy(filetype,"");
  shortlabel=slicei->label.shortlabel;
  if(strlen(shortlabel)>0)strcat(filetype,shortlabel);
  TrimBack(filetype);

  if(GetFileInfo(slice_file,NULL,NULL)!=0){
    fprintf(stderr,"*** Warning: The file %s does not exist\n",slice_file);
    return 0;
  }

  SLICEFILE=fopen(slice_file,"rb");
  if(SLICEFILE==NULL){
    fprintf(stderr,"*** Warning: The file %s could not be opened\n",slice_file);
    return 0;
  }

  // set up slice compressed file

  if(GLOBdestdir!=NULL){
    strcpy(slicefile_svz,GLOBdestdir);
    strcat(slicefile_svz,slicei->filebase);
  }
  else{
    strcpy(slicefile_svz,slicei->file);
  }

  if(strlen(slicefile_svz)>4)strcat(slicefile_svz,".svv");

  if(GLOBcleanfiles==1){
    slicestream=fopen(slicefile_svz,"rb");
    if(slicestream!=NULL){
      fclose(slicestream);
      PRINTF("  Removing %s\n",slicefile_svz);
      UNLINK(slicefile_svz);
      LOCK_COMPRESS;
      GLOBfilesremoved++;
      UNLOCK_COMPRESS;
    }
    fclose(SLICEFILE);
    return 0;
  }

  if(GLOBoverwrite_slice==0){
    slicestream=fopen(slicefile_svz,"rb");
    if(slicestream!=NULL){
      fclose(slicestream);
      fprintf(stderr,"*** Warning: The file %s exists.\n",slicefile_svz);
      fprintf(stderr,"     Use the -f option to overwrite smokezip compressed files\n");
      fclose(SLICEFILE);
      return 0;
    }
  }

  slicestream=fopen(slicefile_svz,"wb");
  if(slicestream==NULL){
    fprintf(stderr,"*** Warning: The file %s could not be opened for writing\n",slicefile_svz);
    fclose(SLICEFILE);
    return 0;
  }

  // read and write slice header

#ifndef pp_THREAD
  if(GLOBcleanfiles==0){
    PRINTF("Compressing %s (%s)\n",slice_file,filetype);
  }
#endif


  {
    int skip;

    skip = 3*(4+30+4);  // skip over 3 records each containing a 30 byte FORTRAN character string
    FSEEK(SLICEFILE,skip,SEEK_CUR);
    sizebefore=skip;
  }

  FORTSLICEREAD(ijkbar,6);
  sizebefore+=4+6*4+4;
  sizeafter=0;

  {
    int one=1, version_local=0, completion=0;

    fwrite(&one,4,1,slicestream);
    fwrite(&version_local,4,1,slicestream);
    fwrite(&completion,4,1,slicestream);
  }


  {
    int ni, nj, nk;

    ni = ijkbar[1]+1-ijkbar[0];
    nj = ijkbar[3]+1-ijkbar[2];
    nk = ijkbar[5]+1-ijkbar[4];
    framesize = ni*nj*nk;
    NewMemory((void **)&sliceframe_data,framesize*sizeof(float));

    for(;;){
      float vmin, vmax;
      float *valmin, *valmax;
      unsigned char *compressed_data_out;
      uLongf ncompressed_data_out;
      float time_local;

      FORTSLICEREAD(&time_local,1);
      if(returncode==0)break;
      CheckMemory;
      sizebefore+=12;

      FORTSLICEREAD(sliceframe_data,framesize);    //---------------
      if(returncode==0)break;
      CheckMemory;
      sizebefore+=(4+framesize*sizeof(float)+4);

      valmin=NULL;
      valmax=NULL;
      if(slicei->voltype==1){
        vmin=0.0;
        valmin=&vmin;
      }
      else if(slicei->voltype==2){
        vmin=20.0;
        valmin=&vmin;
        vmax=1400.0;
        valmax=&vmax;
      }
      else{
        assert(FFALSE);
      }
      CheckMemory;
      CompressVolSliceFrame(sliceframe_data, framesize, time_local, valmin, valmax,
                &compressed_data_out, &ncompressed_data_out);
      CheckMemory;
      sizeafter+=ncompressed_data_out;
      if(ncompressed_data_out>0){
        fwrite(compressed_data_out,1,ncompressed_data_out,slicestream);
      }
      CheckMemory;
      FREEMEMORY(compressed_data_out);

#ifndef pp_THREAD
      count++;
#endif

      data_loc=FTELL(SLICEFILE);
      percent_done=100.0*(float)data_loc/(float)slicei->filesize;
#ifdef pp_THREAD
      threadinfo[*thread_index].stat=percent_done;
      if(percent_done>percent_next){
        LOCK_PRINT;
        PrintThreadStats();
        UNLOCK_PRINT;
        percent_next += PERCENT_SKIP;
      }
#else
      if(percent_done>percent_next){
        PRINTF(" %i%s",percent_next,GLOBpp);
        FFLUSH();
        percent_next += PERCENT_SKIP;
      }
#endif

    }
    if(returncode!=0){
      fprintf(stderr,"*** Error: compress returncode=%i\n",returncode);
    }
    FREEMEMORY(sliceframe_data);
  }

#ifndef pp_THREAD
    PRINTF(" 100%s completed\n",GLOBpp);
#endif

  {
    int completion=1;

    FSEEK(slicestream,4,SEEK_SET);
    fwrite(&completion,4,1,slicestream);
  }
  fclose(SLICEFILE);
  fclose(slicestream);

  {
    char before_label[256],after_label[256];

    GetFileSizeLabel(sizebefore,before_label);
    GetFileSizeLabel(sizeafter,after_label);
#ifdef pp_THREAD
    slicei->vol_compressed=1;
    sprintf(slicei->volsummary,"%s -> %s (%4.1f%s)",before_label,after_label,(float)sizebefore/(float)sizeafter,GLOBx);
    threadinfo[*thread_index].stat=-1;
#else
    PRINTF("  records=%i, ",count);
    PRINTF("Sizes: original=%s, ",before_label);
    PRINTF("compressed=%s (%4.1f%s)\n\n",after_label,(float)sizebefore/(float)sizeafter,GLOBx);
#endif
  }

  return 1;

}

/* ------------------ ConvertSlice ------------------------ */

void MakeSliceFile(char *slicefile, slicedata *slicei, char *ext){
  if(GLOBdestdir!=NULL){
    strcpy(slicefile, GLOBdestdir);
    strcat(slicefile, slicei->filebase);
  }
  else{
    strcpy(slicefile, slicei->file);
  }
  char *ex;

  ex = strrchr(slicefile, '.');
  if(ex!=NULL&&strcmp(ex,".rle")==0)*ex=0;
  strcat(slicefile,ext);
}

/* ------------------ ConvertSlice ------------------------ */

// unsigned int UnCompressRLE(unsigned char *buffer_in, int nchars_in, unsigned char *buffer_out)

int ConvertSlice(slicedata *slicei, int *thread_index){

  char slicefile_svz[1024], slicesizefile_svz[1024], slicefile_svv[1024];
  int fileversion, one, zero;
  char *slice_file;
  int version_local;
  char filetype[1024];
  char *shortlabel, *unit;
  char units[256];
  int ijkbar[6];
  uLong framesize;
  float *sliceframe_data=NULL;
  unsigned char *sliceframe_compressed=NULL, *sliceframe_uncompressed=NULL;
  unsigned char *sliceframe_compressed_rle=NULL, *sliceframe_uncompressed_rle=NULL;
  char cval[256];
  int sizebefore, sizeafter;
  int returncode=0;
  float minmax[2];
  float time_local;
  LINT data_loc;
  int percent_done;
  int percent_next = PERCENT_SKIP;
  float valmin, valmax, denom;
  int chop_min, chop_max;
  uLongf ncompressed_zlib;
  int ncompressed_save;
#ifndef pp_THREAD
  int count=0;
#endif
  int ncol, nrow, idir;
  float time_max;
  int itime;
  LINT file_loc;

  FILE *SLICEFILE;
  FILE *slicestream,*slicesizestream;

#ifdef pp_THREAD
  if(GLOBcleanfiles==0){
    int fileindex;

    fileindex = slicei + 1 - sliceinfo;
    sprintf(threadinfo[*thread_index].label,"sf %i",fileindex);
  }
#endif

  slice_file=slicei->file;
  version_local=slicei->version;

  fileversion = 1;
  one = 1;
  zero=0;

  // check if slice file is accessible

  strcpy(filetype,"");
  shortlabel=slicei->label.shortlabel;
  if(strlen(shortlabel)>0)strcat(filetype,shortlabel);
  TrimBack(filetype);

  if(GetFileInfo(slice_file,NULL,NULL)!=0){
    fprintf(stderr,"*** Warning: The file %s does not exist\n",slice_file);
    return 0;
  }

  SLICEFILE=fopen(slice_file,"rb");
  if(SLICEFILE==NULL){
    fprintf(stderr,"*** Warning: The file %s could not be opened\n",slice_file);
    return 0;
  }

  // set up slice compressed file

  MakeSliceFile(slicefile_svz,     slicei, ".svz");
  MakeSliceFile(slicefile_svv,     slicei, ".svv");
  MakeSliceFile(slicesizefile_svz, slicei, ".sz");

  if(GLOBcleanfiles==1){
    slicestream=fopen(slicefile_svz,"rb");
    if(slicestream!=NULL){
      fclose(slicestream);
      PRINTF("  Removing %s\n",slicefile_svz);
      UNLINK(slicefile_svz);
      LOCK_COMPRESS;
      GLOBfilesremoved++;
      UNLOCK_COMPRESS;
    }
    slicestream=fopen(slicefile_svv,"rb");
    if(slicestream!=NULL){
      fclose(slicestream);
      PRINTF("  Removing %s\n",slicefile_svv);
      UNLINK(slicefile_svv);
      LOCK_COMPRESS;
      GLOBfilesremoved++;
      UNLOCK_COMPRESS;
    }
    slicesizestream=fopen(slicesizefile_svz,"rb");
    if(slicesizestream!=NULL){
      fclose(slicesizestream);
      PRINTF("  Removing %s\n",slicesizefile_svz);
      UNLINK(slicesizefile_svz);
      LOCK_COMPRESS;
      GLOBfilesremoved++;
      UNLOCK_COMPRESS;
    }
    fclose(SLICEFILE);
    return 0;
  }

  if(GLOBoverwrite_slice==0){
    slicestream=fopen(slicefile_svz,"rb");
    if(slicestream!=NULL){
      fclose(slicestream);
      fprintf(stderr,"*** Warning:  %s exists.\n",slicefile_svz);
      fprintf(stderr,"     Use the -f option to overwrite smokezip compressed files\n");
      fclose(SLICEFILE);
      return 0;
    }
  }

  slicestream=fopen(slicefile_svz,"wb");
  slicesizestream=fopen(slicesizefile_svz,"w");
  if(slicestream==NULL||slicesizestream==NULL){
    if(slicestream==NULL){
      fprintf(stderr,"*** Warning: The file %s could not be opened for writing\n",slicefile_svz);
    }
    if(slicesizestream==NULL){
      fprintf(stderr,"  %s could not be opened for writing\n",slicesizefile_svz);
    }
    if(slicestream!=NULL)fclose(slicestream);
    if(slicesizestream!=NULL)fclose(slicesizestream);
    fclose(SLICEFILE);
    return 0;
  }

  // read and write slice header

  strcpy(units,"");
  unit=slicei->label.unit;
  if(strlen(unit)>0)strcat(units,unit);
  TrimBack(units);
  sprintf(cval,"%f",slicei->valmin);
  TrimZeros(cval);
#ifndef pp_THREAD
  if(GLOBcleanfiles==0){
    PRINTF("Compressing %s (%s)\n",slice_file,filetype);
    PRINTF("  using min=%s %s",cval,units);
  }
#endif
  sprintf(cval,"%f",slicei->valmax);
  TrimZeros(cval);
#ifndef pp_THREAD
  if(GLOBcleanfiles==0){
    PRINTF(" max=%s %s\n",cval,units);
    PRINTF(" ");
  }
#endif
  valmin=slicei->valmin;
  valmax=slicei->valmax;
  denom = valmax-valmin;
  if(denom==0.0)denom=1.0;

  chop_min=0;
  chop_max=255;
  if(GLOBno_chop==0){
    if(slicei->setchopvalmax==1){
        chop_max = 255*(slicei->chopvalmax-valmin)/denom;
        if(chop_max<0)chop_max=0;
        if(chop_max>255)chop_max=255;
    }
    if(slicei->setchopvalmin==1){
       chop_min = 255*(slicei->chopvalmin-valmin)/denom;
       if(chop_min<0)chop_min=0;
       if(chop_min>255)chop_min=255;
    }
  }


  fwrite(&one,4,1,slicestream);           // write out a 1 to determine "endianness" when file is read in later
  fwrite(&zero,4,1,slicestream);          // write out a zero now, then a one just before file is closed
  fwrite(&fileversion,4,1,slicestream);   // write out compressed fileversion in case file format changes later
  fwrite(&version_local,4,1,slicestream);       // fds slice file version
  sizeafter=16;

  //*** SLICE FILE FORMATS

  //*** FDS FORMAT (FORTRAN - each FORTRAN record has a 4 byte header and a 4 byte trailer surrounding the data)

  // 30 byte long label
  // 30 byte short label
  // 30 byte unit
  // i1,i2,j1,j2,k1,k2

  // for each time step:

  // time, compressed frame size
  // qq(1,nbuffer)              where nbuffer = (i2+1-i1)*(j2+1-j1)*(k2+1-k1)



  //*** ZLIB format (C - no extra bytes surrounding data)

  //*** header
  // endian
  // completion (0/1)
  // fileversion (compressed format)
  // version_local  (slicef version)
  // global min max (used to perform conversion)
  // i1,i2,j1,j2,k1,k2


  //*** frame
  // time, compressed frame size                        for each frame
  // compressed buffer


  //*** RLE format (FORTRAN)

  //*** header
  // endian
  // fileversion, slice version
  // global min max (used to perform conversion)
  // i1,i2,j1,j2,k1,k2


  //*** frame
  // time
  // compressed frame size                        for each frame
  // compressed buffer

  {
    int skip;

    skip = 3*(4+30+4);  // skip over 3 records each containing a 30 byte FORTRAN character string
    FSEEK(SLICEFILE,skip,SEEK_CUR);
    sizebefore=skip;
  }

  FORTSLICEREAD(ijkbar,6);
  sizebefore+=8+6*4;

  framesize =  (ijkbar[1]+1-ijkbar[0]);
  framesize *= (ijkbar[3]+1-ijkbar[2]);
  framesize *= (ijkbar[5]+1-ijkbar[4]);

  minmax[0]=slicei->valmin;
  minmax[1]=slicei->valmax;
  fwrite(minmax,4,2,slicestream);    // min max vals
  fwrite(ijkbar,4,6,slicestream);
  sizeafter+=(8+24);


  ncompressed_save=1.02*framesize+600;
  if(NewMemory((void **)&sliceframe_data,ncompressed_save*sizeof(float))==0)goto wrapup;
  if(NewMemory((void **)&sliceframe_compressed,ncompressed_save*sizeof(unsigned char))==0)goto wrapup;
  if(NewMemory((void **)&sliceframe_uncompressed,ncompressed_save*sizeof(unsigned char))==0)goto wrapup;

  fprintf(slicesizestream,"%i %i %i %i %i %i\n",ijkbar[0],ijkbar[1],ijkbar[2],ijkbar[3],ijkbar[4],ijkbar[5]);
  fprintf(slicesizestream,"%f %f\n",minmax[0],minmax[1]);

  idir=0;
  if(ijkbar[0]==ijkbar[1]){
    idir=1;
    ncol = ijkbar[3] + 1 - ijkbar[2];
    nrow = ijkbar[5] + 1 - ijkbar[4];
  }
  else if(ijkbar[2]==ijkbar[3]){
    idir=2;
    ncol = ijkbar[1] + 1 - ijkbar[0];
    nrow = ijkbar[5] + 1 - ijkbar[4];
  }
  else if(ijkbar[4]==ijkbar[5]){
    idir=3;
    ncol = ijkbar[1] + 1 - ijkbar[0];
    nrow = ijkbar[3] + 1 - ijkbar[2];
  }
  if(idir==0){
    ncol = ijkbar[3] + 1 - ijkbar[2];
    nrow = ijkbar[5] + 1 - ijkbar[4];
  }


  {
    int ni, nj, nk;

    ni = ijkbar[1]+1-ijkbar[0];
    nj = ijkbar[3]+1-ijkbar[2];
    nk = ijkbar[5]+1-ijkbar[4];

    time_max=-1000000.0;
    itime=-1;
    for(;;){
      int i;

      FORTSLICEREAD(&time_local,1);
      sizebefore+=12;
      if(returncode==0)break;
      FORTSLICEREAD(sliceframe_data,framesize);    //---------------
      if(returncode==0)break;

      sizebefore+=(8+framesize*4);
      if(time_local<time_max)continue;
      time_max=time_local;

#ifndef pp_THREAD
      count++;
#endif

      data_loc=FTELL(SLICEFILE);
      percent_done=100.0*(float)data_loc/(float)slicei->filesize;
#ifdef pp_THREAD
      threadinfo[*thread_index].stat=percent_done;
      if(percent_done>percent_next){
        LOCK_PRINT;
        PrintThreadStats();
        UNLOCK_PRINT;
        percent_next += PERCENT_SKIP;
      }
#else
      if(percent_done>percent_next){
        PRINTF(" %i%s",percent_next,GLOBpp);
        FFLUSH();
        percent_next += PERCENT_SKIP;
      }
#endif
      for(i=0;i<framesize;i++){
        int ival;
        int icol, jrow, index2;
        int ii,jj,kk;

        // val_in(i,j,k) = i + j*ni + k*ni*nj

        if(framesize<=ncol*nrow){  // only one slice plane

          // i = jrow*ncol + icol;

          icol = i%ncol;
          jrow = i/ncol;

          index2 = icol*nrow + jrow;
        }
        else{
          ii = i%ni;
          jj = (i/ni)%nj;
          kk = i/(ni*nj);

          index2 = ii*nj*nk + jj*nk + kk;
        }

        {
          float val;

          val = sliceframe_data[i];
          if(val<valmin){
            ival=0;
          }
          else if(val>valmax){
            ival=255;
          }
          else{
            ival = 1 + 253*(val-valmin)/denom;
          }
          if(ival<chop_min)ival=0;
          if(ival>chop_max)ival=255;
          sliceframe_uncompressed[index2] = ival;
        }
      }
      itime++;
      if(itime%GLOBslicezipstep!=0)continue;

      //int compress (Bytef *dest,   uLongf *destLen, const Bytef *source, uLong sourceLen);
      ncompressed_zlib=ncompressed_save;
      CompressZLIB(sliceframe_compressed,&ncompressed_zlib,sliceframe_uncompressed,framesize);

      file_loc=FTELL(slicestream);
      fwrite(&time_local,4,1,slicestream);
      fwrite(&ncompressed_zlib,4,1,slicestream);
      fwrite(sliceframe_compressed,1,ncompressed_zlib,slicestream);
      sizeafter+=(8+ncompressed_zlib);
      fprintf(slicesizestream,"%f %i, %li\n",time_local,(int)ncompressed_zlib,(long)file_loc);
    }
    if(returncode!=0){
      fprintf(stderr,"*** Error: compress returncode=%i\n",returncode);
    }
  }

wrapup:
#ifndef pp_THREAD
    PRINTF(" 100%s completed\n",GLOBpp);
#endif
  FREEMEMORY(sliceframe_data);
  FREEMEMORY(sliceframe_compressed);
  FREEMEMORY(sliceframe_uncompressed);
  FREEMEMORY(sliceframe_compressed_rle);
  FREEMEMORY(sliceframe_uncompressed_rle);

  fclose(SLICEFILE);
  FSEEK(slicestream,4,SEEK_SET);
  fwrite(&one,4,1,slicestream);  // write completion code
  fclose(slicestream);
  fclose(slicesizestream);

  {
    char before_label[256],after_label[256];

    GetFileSizeLabel(sizebefore,before_label);
    GetFileSizeLabel(sizeafter,after_label);
#ifdef pp_THREAD
    slicei->compressed=1;
    sprintf(slicei->summary,"%s to %s (%4.1f%s)",before_label,after_label,(float)sizebefore/(float)sizeafter,GLOBx);
    threadinfo[*thread_index].stat=-1;
#else
    PRINTF("  records=%i, ",count);
    PRINTF("Sizes: original=%s, ",before_label);
    PRINTF("compressed=%s (%4.1f%s)\n\n",after_label,(float)sizebefore/(float)sizeafter,GLOBx);
#endif
  }

  return 1;
}

/* ------------------ CompressVolSlices ------------------------ */

void *CompressVolSlices(void *arg){
  int *thread_index;
  int i;

  thread_index = (int *)arg;
  if(nvolrenderinfo<=0)return NULL;

  if(GLOBcleanfiles==1)return NULL;

  // convert and compress files

  for(i=0;i<nsliceinfo;i++){
    slicedata *slicei;

    slicei = sliceinfo + i;

    if(slicei->isvolslice==0)continue;

    LOCK_VOLSLICE;
    if(slicei->involuse==1){
      UNLOCK_VOLSLICE;
      continue;
    }
    slicei->involuse=1;
    UNLOCK_VOLSLICE;

    ConvertVolSlice(slicei,thread_index);
  }
  return NULL;
}

/* ------------------ GetGlobalSliceBounds ------------------------ */

void GetGlobalSliceBounds(char *label){
  int j,count=0;
  float valmin, valmax;

  count = 0;
  for(j = 0;j<nsliceinfo;j++){
    slicedata *slicej;
    FILE *stream;

    slicej = sliceinfo+j;
    if(strcmp(label, slicej->label.shortlabel)!=0)continue;
    stream = fopen(slicej->boundfile, "r");
    if(stream==NULL)continue;
    while(!feof(stream)){
      char buffer[255];
      float t, vmin, vmax;

      if(fgets(buffer, 255, stream)==NULL)break;
      sscanf(buffer, " %f %f %f", &t, &vmin, &vmax);
      if(count==0){
        valmin = vmin;
        valmax = vmax;
      }
      else{
        valmin = MIN(valmin, vmin);
        valmax = MAX(valmax, vmax);
      }
      count++;
    }
    fclose(stream);
  }
  if(count>0){
    for(j = 0;j<nsliceinfo;j++){
      slicedata *slicej;

      slicej = sliceinfo+j;
      if(strcmp(label, slicej->label.shortlabel)!=0)continue;
      slicej->valmin = valmin;
      slicej->valmax = valmax;
      slicej->setvalmax = 1;
      slicej->setvalmin = 1;
      slicej->doit = 1;
    }
  }
}

/* ------------------ CompressSlices ------------------------ */

void *CompressSlices(void *arg){
  int i;
  slicedata *slicei;
  int *thread_index;

  thread_index = (int *)arg;


  if(nsliceinfo<=0)return NULL;
  LOCK_SLICE;
  if(GLOBfirst_slice==1){
    GLOBfirst_slice=0;
    if(GLOBcleanfiles==1){
      for(i=0;i<nsliceinfo;i++){
        slicei = sliceinfo + i;
        ConvertSlice(slicei,thread_index);
      }
      UNLOCK_SLICE;
      return NULL;
    }
    GetSliceBounds();
  }
  UNLOCK_SLICE;

  if(GLOBcleanfiles==1)return NULL;

  // convert and compress files

  for(i=0;i<nsliceinfo;i++){
    slicei = sliceinfo + i;
    LOCK_SLICE;
    if(slicei->inuse==1){
      UNLOCK_SLICE;
      continue;
    }
    slicei->inuse=1;
    UNLOCK_SLICE;

    if(slicei->doit==1){
      if(GLOBmake_demo==1&&(strcmp(slicei->label.longlabel,"TEMPERATURE")==0||strcmp(slicei->label.longlabel,"oxygen")==0)){
        slicei->setvalmax=1;
        slicei->setvalmin=1;
        if(strcmp(slicei->label.longlabel,"TEMPERATURE")==0){
          slicei->valmax=620.0;
          slicei->valmin=20.0;
        }
        else{
          slicei->valmax=0.23;
          slicei->valmin=0.0;
        }
      }
      ConvertSlice(slicei,thread_index);
    }
    else{
      PRINTF("%s not compressed\n",slicei->file);
      PRINTF("  Min and Max for %s not set in .ini file\n",slicei->label.shortlabel);
    }
  }
  return NULL;
}

/* ------------------ GetSliceParmsC ------------------------ */

void GetSliceParmsC(char *file, int *ni, int *nj, int *nk){
    int skip,ijkbar[6];
    FILE *stream;

    *ni=0;
    *nj=0;
    *nk=0;

    stream=fopen(file,"rb");
    if(stream==NULL)return;

    skip = 3*(4+30+4);  // skip over 3 records each containing a 30 byte FORTRAN character string
    FSEEK(stream,skip,SEEK_CUR);

    skip=4;
    FSEEK(stream,skip,SEEK_CUR);
    fread(ijkbar,sizeof(int),6,stream);
    *ni=ijkbar[1]+1-ijkbar[0];
    *nj=ijkbar[3]+1-ijkbar[2];
    *nk=ijkbar[5]+1-ijkbar[4];
    fclose(stream);
}
