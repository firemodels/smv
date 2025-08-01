#include "options.h"
#include "zlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <zlib.h>
#include "svzip.h"
#include "dmalloc.h"
#include "compress.h"
#include "getdata.h"

pdfdata pdfmerge,pdfframe;

#define FORTREAD(var,size) FSEEK(BOUNDARYFILE,4,SEEK_CUR);\
                           returncode=fread(var,4,size,BOUNDARYFILE);\
                           FSEEK(BOUNDARYFILE,4,SEEK_CUR)

/* ------------------ CleanBoundary ------------------------ */

int CleanBoundary(patchdata *patchi){
  FILE *BOUNDARYFILE=NULL;
  char boundaryfile_svz[1024], boundarysizefile_svz[1024];
  FILE *boundarystream=NULL,*boundarysizestream=NULL;
  char *boundary_file;
  char filetype[256];
  char *shortlabel;

  boundary_file=patchi->file;

  // check if boundary file is accessible

  strcpy(filetype,"");
  shortlabel=patchi->label.shortlabel;
  if(strlen(shortlabel)>0)strcat(filetype,shortlabel);
  TrimBack(filetype);

  BOUNDARYFILE=fopen(boundary_file,"rb");
  if(BOUNDARYFILE==NULL){
    return 0;
  }
  fclose(BOUNDARYFILE);

  // set up boundary compressed file

  if(GLOBdestdir!=NULL){
    strcpy(boundaryfile_svz,GLOBdestdir);
    strcat(boundaryfile_svz,patchi->filebase);
  }
  else{
    strcpy(boundaryfile_svz,patchi->file);
  }
  strcat(boundaryfile_svz,".svz");

  if(GLOBdestdir!=NULL){
    strcpy(boundarysizefile_svz,GLOBdestdir);
    strcat(boundarysizefile_svz,patchi->filebase);
  }
  else{
    strcpy(boundarysizefile_svz,patchi->file);
  }
  strcat(boundarysizefile_svz,".szz");

  boundarystream=fopen(boundaryfile_svz,"rb");
  if(boundarystream!=NULL){
    fclose(boundarystream);
    PRINTF("  Removing %s\n",boundaryfile_svz);
    UNLINK(boundaryfile_svz);
    LOCK_COMPRESS;
    GLOBfilesremoved++;
    UNLOCK_COMPRESS;
  }
  boundarysizestream=fopen(boundarysizefile_svz,"rb");
  if(boundarysizestream!=NULL){
    fclose(boundarysizestream);
    PRINTF("  Removing %s\n",boundarysizefile_svz);
    UNLINK(boundarysizefile_svz);
    LOCK_COMPRESS;
    GLOBfilesremoved++;
    UNLOCK_COMPRESS;
  }
  return 0;
}

/* ------------------ ConvertBoundaryGEOM ------------------------ */

int ConvertBoundaryGEOM(patchdata *patchi, int *thread_index){
  FILE *BOUNDARYFILE = NULL, *boundarystream = NULL, *boundarysizestream = NULL;
  char boundaryfile_svz[1024], boundarysizefile_svz[1024], *boundary_file;
  unsigned char *cvals=NULL, *compressed_vals=NULL;
  float *vals=NULL, time_local;
  uLongf ncompressed_vals;
  uLong ntotal;
  int one = 1, zero = 0;
  unsigned int sizebefore = 0, sizeafter = 0;
  int percent_done, percent_next = PERCENT_SKIP;
  int returncode;
  LINT data_loc;
#ifndef pp_THREAD
  int count;
#endif

  boundary_file = patchi->file;
  patchi->compressed = 0;

#ifdef pp_THREAD
  {
    int fileindex;

    fileindex = patchi + 1 - patchinfo;
    sprintf(threadinfo[*thread_index].label, "bf %i", fileindex);
  }
#endif

  // check if boundary file is accessible

  if(GetFileInfo(boundary_file, NULL, NULL) != 0){
    fprintf(stderr, "*** Warning: The file %s does not exist\n", boundary_file);
    return 0;
  }

  BOUNDARYFILE = fopen(boundary_file, "rb");
  if(BOUNDARYFILE == NULL){
    fprintf(stderr, "*** Warning: The file %s could not be opened\n", boundary_file);
    return 0;
  }

  // set up boundary compressed file

  if(GLOBdestdir != NULL){
    strcpy(boundaryfile_svz, GLOBdestdir);
    strcat(boundaryfile_svz, patchi->filebase);
  }
  else{
    strcpy(boundaryfile_svz, patchi->file);
  }
  strcat(boundaryfile_svz, ".svz");

  if(GLOBdestdir != NULL){
    strcpy(boundarysizefile_svz, GLOBdestdir);
    strcat(boundarysizefile_svz, patchi->filebase);
  }
  else{
    strcpy(boundarysizefile_svz, patchi->file);
  }
  strcat(boundarysizefile_svz, ".szz");

  if(GLOBoverwrite_b == 0){
    boundarystream = fopen(boundaryfile_svz, "rb");
    if(boundarystream != NULL){
      if(boundarystream != NULL){
        fclose(boundarystream);
        fprintf(stderr, "*** Warning: The file %s exists.\n", boundaryfile_svz);
        fprintf(stderr, "     Use the -f option to overwrite smokezip compressed files\n");
      }
      fclose(BOUNDARYFILE);
      return 0;
    }
  }

  boundarystream = fopen(boundaryfile_svz, "wb");
  boundarysizestream = fopen(boundarysizefile_svz, "w");
  if(boundarystream == NULL || boundarysizestream == NULL){
    if(boundarystream == NULL){
      fprintf(stderr, "*** Warning: The file %s could not be opened for writing\n", boundaryfile_svz);
    }
    if(boundarysizestream == NULL){
      fprintf(stderr, "*** Warning: The file %s could not be opened for writing\n", boundarysizefile_svz);
    }
    if(boundarystream != NULL)fclose(boundarystream);
    if(boundarysizestream != NULL)fclose(boundarysizestream);
    fclose(BOUNDARYFILE);
    return 0;
  }

#ifndef pp_THREAD
  char *shortlabel;
  char *unit, units[256], cval[256];
  char filetype[256];

  strcpy(filetype, "");
  shortlabel = patchi->label.shortlabel;
  if(strlen(shortlabel) > 0)strcat(filetype, shortlabel);
  TrimBack(filetype);

  PRINTF("Compressing %s (%s)\n", boundary_file, filetype);

  strcpy(units, "");
  unit = patchi->label.unit;
  if(strlen(unit) > 0)strcat(units, unit);
  TrimBack(units);
  sprintf(cval, "%f", patchi->valmin);
  TrimZeros(cval);
  PRINTF("  using min=%s %s", cval, units);
  sprintf(cval, "%f", patchi->valmax);
  TrimZeros(cval);
  PRINTF(" max=%s %s\n", cval, units);
#endif
  int endian, version;

  FORTREAD(&endian,  1);
  FORTREAD(&version, 1);
  sizebefore = 24;

  int fileversion = 0;
  fwrite(&one,           4, 1, boundarystream); // write out a 1 to determine "endianness" when file is read in later
  fwrite(&zero,          4, 1, boundarystream); // write out a zero now, then a one just before file is closed
  fwrite(&fileversion,   4, 1, boundarystream); // write out compressed fileversion in case file format changes later
  sizeafter = 12;
  float denom, valmin, valmax, valminmax[2];


  valmin = patchi->valmin;
  valmax = patchi->valmax;
  denom = valmax - valmin;
  if(denom <= 0.0){
    valmin = 0.0;
    valmax = 1.0;
    denom = 1.0;
  }
  valminmax[0] = valmin;
  valminmax[1] = valmax;
  fwrite(valminmax, 4, 2, boundarystream);
  sizeafter += 8;

  //***format
  // INPUT
  // 1
  // version
  // for each time step:
  // time
  // nvert_static, ntri_static, nvert_dynamic, ntri_dynamic
  // if(nvert_static>0) vals_1, ...vals_nvert_static
  // if(ntri_static>0)  vals_1, ...vals_ntri_static
  // if(nvert_dynamic>0)vals_1, ...vals_nvert_dynamic
  // if(ntri_dynamic>0) vals_1, ...vals_ntri_dynamic

  // OUTPUT
  // 1
  // completion (0/1)
  // fileversion (compressed format)
  // min max (used to perform conversion)
  // for each time step
  // time
  // nval1,nval2,nval3,nval4
  // ncompressed
  // compressed_1,...,compressed_ncompressed

#ifndef pp_THREAD
    PRINTF(" ");
#endif
    int MAXVALS = 0;
    int MAXCOMPRESSEDVALS;    
    while(feof(BOUNDARYFILE) == 0){
      int nvals[4], offset[4];

      FORTREAD(&time_local, 1);
      sizebefore += 12;
      if(returncode == 0)break;

      FORTREAD(nvals, 4);
      sizebefore += 8 + 4 * 4;
      if(returncode == 0)break;

      ntotal = nvals[0] + nvals[1] + nvals[2] + nvals[3];
      offset[0] = 0;
      offset[1] = offset[0] + nvals[0];
      offset[2] = offset[1] + nvals[1];
      offset[3] = offset[2] + nvals[2];
      if(ntotal > MAXVALS){
        MAXVALS = ntotal+1000;
        MAXCOMPRESSEDVALS = 1.01*MAXVALS+600;
        FREEMEMORY(vals);
        FREEMEMORY(cvals);
        FREEMEMORY(compressed_vals);
        NewMemory((void **)&vals, MAXVALS * sizeof(float));
        NewMemory((void **)&cvals, MAXVALS);
        NewMemory((void **)&compressed_vals, MAXCOMPRESSEDVALS);
      }
      ncompressed_vals = 0;
      if(ntotal > 0){
        int i;
        int exit_loop;

        exit_loop = 0;
        for(i = 0; i < 4; i++){
          if(nvals[i] > 0){
            FORTREAD(vals + offset[i], nvals[i]);
            sizebefore += 8 + 4*nvals[i];
            if(returncode==0){
              exit_loop = 1;
              break;
            }
          }
        }
        if(exit_loop==1)break;
        for(i = 0; i < ntotal; i++){
          float val;

          val      = CLAMP((vals[i] - valmin) / denom, 0.0, 1.0);
          cvals[i] = CLAMP((unsigned char)(255.0 * val), 0, 255);
        }
        ncompressed_vals = MAXCOMPRESSEDVALS;
        CompressZLIB(compressed_vals, &ncompressed_vals, cvals, ntotal);
      }
      fprintf(boundarysizestream, "%f %i %i\n", time_local, ( int )ntotal, ( int )ncompressed_vals);
      fwrite(&time_local,       4, 1,                boundarystream); // write out time_local
      fwrite(&nvals,            4, 4,                boundarystream); // write out nvals
      fwrite(&ncompressed_vals, 4, 1,                boundarystream); // write out compressed size of frame
      if(ncompressed_vals>0){
        fwrite(compressed_vals, 1, ncompressed_vals, boundarystream); // write out compressed buffer
      }
      sizeafter += 8 + 16 + ncompressed_vals;
      data_loc = FTELL(BOUNDARYFILE);
      percent_done = 100.0 * ( float )data_loc / ( float )patchi->filesize;
#ifdef pp_THREAD
      threadinfo[*thread_index].stat = percent_done;
      if(percent_done > percent_next){
        LOCK_PRINT;
        PrintThreadStats();
        UNLOCK_PRINT;
        percent_next += PERCENT_SKIP;
      }
#else
      if(percent_done > percent_next){
        PRINTF(" %i%s", percent_next, GLOBpp);
        FFLUSH();
        percent_next += PERCENT_SKIP;
      }
#endif
    }
#ifndef pp_THREAD
    PRINTF(" 100%s completed\n", GLOBpp);
#endif
  fclose(BOUNDARYFILE);
  FSEEK(boundarystream, 8, SEEK_SET);
  fwrite(&one, 4, 1, boundarystream);  // write completion code
  fclose(boundarystream);
  fclose(boundarysizestream);
  {
    char before_label[256], after_label[256];
    GetFileSizeLabel(sizebefore, before_label);
    GetFileSizeLabel(sizeafter, after_label);
#ifdef pp_THREAD
    patchi->compressed = 1;
    sprintf(patchi->summary, "%s -> %s (%4.1f%s)", before_label, after_label, ( float )sizebefore / ( float )sizeafter, GLOBx);
    threadinfo[*thread_index].stat = -1;
#else
    PRINTF("  records=%i, ", count);
    PRINTF("Sizes: original=%s, ", before_label);
    PRINTF("compressed=%s (%4.1f%s)\n\n", after_label, ( float )sizebefore / ( float )sizeafter, GLOBx);
#endif
  }
  return 1;
}

/* ------------------ ConvertBoundaryBNDF ------------------------ */

int ConvertBoundaryBNDF(patchdata *patchi, int *thread_index){
  FILE *BOUNDARYFILE=NULL;
  char boundaryfile_svz[1024], boundarysizefile_svz[1024];
  FILE *boundarystream=NULL,*boundarysizestream=NULL;
  int npatch;
  int ijkbounds[9];
  int i;
  int fileversion,one;
  float time_local;
  int i1, i2, j1, j2, k1, k2;
  float *patchvals=NULL,*patchvalscopy;
  unsigned char *full_boundarybuffer=NULL,*compressed_boundarybuffer=NULL;
  int returncode=0;
  int ncompressed_zlibSAVE;
  uLongf ncompressed_zlib;
  uLong npatchfull;
  unsigned int sizebefore=0, sizeafter=0;
  int count=-1;
  int version_local;
  char *boundary_file;
  char filetype[256];
  char *shortlabel, *unit;
  char units[256];
  float minmax[2];
  char cval[256];
  int percent_done;
  int percent_next = PERCENT_SKIP;
  LINT data_loc;
  int zero=0;
  float time_max;

  boundary_file=patchi->file;
  version_local=patchi->version;
  patchi->compressed=0;

#ifdef pp_THREAD
  {
    int fileindex;

    fileindex = patchi + 1 - patchinfo;
    sprintf(threadinfo[*thread_index].label,"bf %i",fileindex);
  }
#endif
  fileversion = 1;
  one = 1;
  zero=0;

  // check if boundary file is accessible

  strcpy(filetype,"");
  shortlabel=patchi->label.shortlabel;
  if(strlen(shortlabel)>0)strcat(filetype,shortlabel);
  TrimBack(filetype);

  if(GetFileInfo(boundary_file,NULL,NULL)!=0){
    fprintf(stderr,"*** Warning: The file %s does not exist\n",boundary_file);
    return 0;
  }

  BOUNDARYFILE=fopen(boundary_file,"rb");
  if(BOUNDARYFILE==NULL){
    fprintf(stderr,"*** Warning: The file %s could not be opened\n",boundary_file);
    return 0;
  }

  // set up boundary compressed file

  if(GLOBdestdir!=NULL){
    strcpy(boundaryfile_svz,GLOBdestdir);
    strcat(boundaryfile_svz,patchi->filebase);
  }
  else{
    strcpy(boundaryfile_svz,patchi->file);
  }
  strcat(boundaryfile_svz,".svz");

  if(GLOBdestdir!=NULL){
    strcpy(boundarysizefile_svz,GLOBdestdir);
    strcat(boundarysizefile_svz,patchi->filebase);
  }
  else{
    strcpy(boundarysizefile_svz,patchi->file);
  }
  strcat(boundarysizefile_svz,".szz");

  if(GLOBoverwrite_b==0){
    boundarystream=fopen(boundaryfile_svz,"rb");
    boundarysizestream=fopen(boundarysizefile_svz,"r");
    if(boundarystream!=NULL||boundarysizestream!=NULL){
      if(boundarystream!=NULL){
        fclose(boundarystream);
        fprintf(stderr,"*** Warning: The file %s exists.\n",boundaryfile_svz);
        fprintf(stderr,"     Use the -f option to overwrite smokezip compressed files\n");
      }
      fclose(BOUNDARYFILE);
      return 0;
    }
  }

  boundarystream=fopen(boundaryfile_svz,"wb");
  boundarysizestream=fopen(boundarysizefile_svz,"w");
  if(boundarystream==NULL||boundarysizestream==NULL){
    if(boundarystream==NULL){
      fprintf(stderr,"*** Warning: The file %s could not be opened for writing\n",boundaryfile_svz);
    }
    if(boundarysizestream==NULL){
      fprintf(stderr,"*** Warning: The file %s could not be opened for writing\n",boundarysizefile_svz);
    }
    if(boundarystream!=NULL)fclose(boundarystream);
    if(boundarysizestream!=NULL)fclose(boundarysizestream);
    fclose(BOUNDARYFILE);
    return 0;
  }


  // read and write boundary header
#ifndef pp_THREAD
  PRINTF("Compressing %s (%s)\n",boundary_file,filetype);
#endif

  strcpy(units,"");
  unit=patchi->label.unit;
  if(strlen(unit)>0)strcat(units,unit);
  TrimBack(units);
  sprintf(cval,"%f",patchi->valmin);
  TrimZeros(cval);
#ifndef pp_THREAD
  PRINTF("  using min=%s %s",cval,units);
#endif
  sprintf(cval,"%f",patchi->valmax);
  TrimZeros(cval);
#ifndef pp_THREAD
  PRINTF(" max=%s %s\n",cval,units);
#endif


  fwrite(&one,4,1,boundarystream);           // write out a 1 to determine "endianness" when file is read in later
  fwrite(&zero,4,1,boundarystream);          // write out a zero now, then a one just before file is closed
  fwrite(&fileversion,4,1,boundarystream);   // write out compressed fileversion in case file format changes later
  fwrite(&version_local,4,1,boundarystream);       // fds boundary file version
  sizeafter=16;

  // endian
  // completion (0/1)
  // fileversion (compressed format)
  // version  (bndf version)
  // global min max (used to perform conversion)
  // local min max  (min max found for this file)
  // npatch
  // i1,i2,j1,j2,k1,k2,idir,dummy,dummy (npatch times)
  // time_local
  // compressed size of frame
  // compressed buffer


  {
    int skip;

    skip = 3*(4+30+4);  // skip over 3 records each containing a 30 byte FORTRAN character string
    returncode=FSEEK(BOUNDARYFILE,skip,SEEK_CUR);
    sizebefore=skip;
  }

  npatch=0;
  if(returncode==0){
    FORTREAD(&npatch,1);
    if(returncode==0)npatch=0;
    sizebefore+=12;

    minmax[0]=patchi->valmin;
    minmax[1]=patchi->valmax;
    fwrite(minmax,4,2,boundarystream);    // conversion min max vals
    FSEEK(boundarystream,8,SEEK_CUR);       // skip over local min max vals (we're set in pass 1);
    fwrite(&npatch,4,1,boundarystream);   // write out npatch
    sizeafter+=20;
  }

  if(npatch>0){

    int nbounds=6;
    int *ijks=NULL,*ijkscopy;

    if(NewMemory((void **)&ijks,6*npatch*sizeof(int))==0)goto wrapup;
    CheckMemory;
    ijkscopy=ijks;
    if(version_local==1)nbounds=9;

    npatchfull=0;
    for(i=0;i<npatch;i++){
      int j;

      FORTREAD(ijkbounds,nbounds);
      sizebefore+=(nbounds+2)*4;
      if(returncode==0)goto wrapup;
      fwrite(ijkbounds,4,nbounds,boundarystream);         // write out i1,i2,j1,j2,k1,k2,idir,dummy,dummy
      sizeafter+=4*nbounds;                               // note:  data can be read into one block of size 9*nblocks

      i1 = ijkbounds[0];
      i2 = ijkbounds[1];
      j1 = ijkbounds[2];
      j2 = ijkbounds[3];
      k1 = ijkbounds[4];
      k2 = ijkbounds[5];
      for(j=0;j<6;j++){
        *ijkscopy++=ijkbounds[j];
      }
      npatchfull+=(i2+1-i1)*(j2+1-j1)*(k2+1-k1);
      CheckMemory;
    }

    ncompressed_zlibSAVE=1.01*npatchfull+600;
    if(NewMemory((void **)&patchvals,npatchfull*sizeof(float))==0)goto wrapup;
    if(NewMemory((void **)&full_boundarybuffer,npatchfull)==0)goto wrapup;
    if(NewMemory((void **)&compressed_boundarybuffer,ncompressed_zlibSAVE)==0)goto wrapup;
#ifndef pp_THREAD
    PRINTF(" ");
#endif
    time_max=-1000000.0;
    while(feof(BOUNDARYFILE)==0){
      int j;

      FORTREAD(&time_local,1);
      sizebefore+=12;
      if(returncode==0)break;

      patchvalscopy=patchvals;
      for(j=0;j<npatch;j++){
        int size;

        i1 = ijks[6*j];
        i2 = ijks[6*j+1];
        j1 = ijks[6*j+2];
        j2 = ijks[6*j+3];
        k1 = ijks[6*j+4];
        k2 = ijks[6*j+5];
        size = (i2+1-i1)*(j2+1-j1)*(k2+1-k1);

        FORTREAD(patchvalscopy,size);
        sizebefore+=(size+2)*4;
        if(returncode==0)goto wrapup;
        patchvalscopy+=size;
      }

//      patchi = patchinfo + i;

      if(time_local<time_max)continue;
      count++;

      if(count%GLOBboundzipstep!=0)continue;
      time_max=time_local;

      for(i=0;i<npatchfull;i++){
        unsigned char ival;
        float val;

        val = patchvals[i];

        if(val<patchi->valmin){
          ival=0;
        }
        else if(val>patchi->valmax){
          ival=255;
        }
        else{
          ival=1+253*(val-patchi->valmin)/(patchi->valmax-patchi->valmin);
        }
        full_boundarybuffer[i]=ival;
      }

      //int compress (Bytef *dest,   uLongf *destLen, const Bytef *source, uLong sourceLen);
      ncompressed_zlib=ncompressed_zlibSAVE;
      returncode=CompressZLIB(compressed_boundarybuffer, &ncompressed_zlib, full_boundarybuffer, npatchfull);
      if(returncode!=0){
        fprintf(stderr,"*** Error: compress returncode=%i\n",returncode);
      }
//      PRINTF("time=%f before %i after=%i\n",time_local,npatchfull,ncompressed_zlib);

      fprintf(boundarysizestream,"%f %i %i\n",time_local,(int)npatchfull,(int)ncompressed_zlib);
      fwrite(&time_local,4,1,boundarystream);                                       // write out time_local
      fwrite(&ncompressed_zlib,4,1,boundarystream);                           // write out compressed size of frame
      fwrite(compressed_boundarybuffer,1,ncompressed_zlib,boundarystream);    // write out compressed buffer
      sizeafter+=ncompressed_zlib+8;

      data_loc=FTELL(BOUNDARYFILE);
      percent_done=100.0*(float)data_loc/(float)patchi->filesize;
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
wrapup:
#ifndef pp_THREAD
    PRINTF(" 100%s completed\n",GLOBpp);
#endif
    FREEMEMORY(ijks);
    FREEMEMORY(patchvals);
    FREEMEMORY(full_boundarybuffer);
    FREEMEMORY(compressed_boundarybuffer);
  }

  fclose(BOUNDARYFILE);
  FSEEK(boundarystream,8,SEEK_SET);
  fwrite(&one,4,1,boundarystream);  // write completion code
  fclose(boundarystream);
  fclose(boundarysizestream);
  {
    char before_label[256],after_label[256];
    GetFileSizeLabel(sizebefore,before_label);
    GetFileSizeLabel(sizeafter,after_label);
#ifdef pp_THREAD
    patchi->compressed=1;
    sprintf(patchi->summary,"%s -> %s (%4.1f%s)",before_label,after_label,(float)sizebefore/(float)sizeafter,GLOBx);
    threadinfo[*thread_index].stat=-1;
#else
    PRINTF("  records=%i, ",count);
    PRINTF("Sizes: original=%s, ",before_label);
    PRINTF("compressed=%s (%4.1f%s)\n\n",after_label,(float)sizebefore/(float)sizeafter,GLOBx);
#endif
  }

  return 1;

}

/* ------------------ GetPatchBoundInfo ------------------------ */

bounddata *GetPatchBoundInfo(char *label){
  int i;

  for(i = 0; i < npatchbounds; i++){
    bounddata *boundi;
    char *labeli;

    boundi = patchbounds + i;
    labeli = boundi->label;
    if(strcmp(label, labeli) == 0)return boundi;
  }
  return NULL;
}

/* ------------------ InitBoundaryBounds ------------------------ */

void InitBoundaryBounds(void){
  int i;

  npatchbounds = 0;
  for(i = 0; i < npatchinfo; i++){
    patchdata *patchi;
    bounddata *pb;

    patchi = patchinfo + i;
    if(GetPatchBoundInfo(patchi->label.shortlabel) == NULL){
      pb = patchbounds + npatchbounds;
      strcpy(pb->label, patchi->label.shortlabel);
      pb->valmin = 1000000000.0;
      pb->valmax = -pb->valmin;
      pb->setvalmin = 0;
      pb->setvalmax = 0;
      npatchbounds++;
    }
  }
}

/* ------------------ GetBoundaryBounds ------------------------ */

void GetBoundaryBounds(void){
  int i;

  for(i=0;i<npatchbounds;i++){
    bounddata *pbi;
    int j;
    int have_bound=0;

    pbi = patchbounds + i;
    if(pbi->setvalmin==1&&pbi->setvalmax==1)continue; // bound obtained from ini file
    for(j=0;j<npatchinfo;j++){  // bound computed from .bnd files
      patchdata *patchj;

      patchj = patchinfo + j;
      if(strcmp(pbi->label, patchj->label.shortlabel)!=0)continue;
      if(GetFileBounds(patchj->boundfile, &(patchj->valmin), &(patchj->valmax))==1){
        pbi->valmin = MIN(pbi->valmin, patchj->valmin);
        pbi->valmax = MAX(pbi->valmax, patchj->valmax);
        have_bound = 1;
      }
    }
    if(have_bound==1){
      pbi->setvalmin = 1;
      pbi->setvalmax = 1;
      for(j = 0;j < npatchinfo;j++){  // bound computed from .bnd files
        patchdata *patchj;

        patchj = patchinfo + j;
        if(strcmp(pbi->label, patchj->label.shortlabel) != 0)continue;
        patchj->setvalmin = 1;
        patchj->setvalmax = 1;
        patchj->valmin    = pbi->valmin;
        patchj->valmax    = pbi->valmax;
      }
    }
  }
}

/* ------------------ CompressPatches ------------------------ */

void *CompressPatches(void *arg){
  int i;
  patchdata *patchi;
  int *thread_index;

  thread_index = (int *)arg;

  if(npatchinfo<=0)return NULL;
  LOCK_PATCH;
  if(GLOBfirst_patch==1){
    GLOBfirst_patch=0;

    if(GLOBcleanfiles==1){
      for(i=0;i<npatchinfo;i++){
        patchi = patchinfo + i;
        CleanBoundary(patchi);
      }
      UNLOCK_PATCH;
      return NULL;
    }
    for(i=0;i<npatchbounds;i++){
      bounddata *pbi;
      int j;

      pbi = patchbounds + i;
      for(j = 0; j < npatchinfo; j++){
        patchdata *patchj;

        patchj = patchinfo + j;
        if(strcmp(patchj->label.shortlabel, pbi->label) == 0){
          patchj->setvalmax = pbi->setvalmax;
          patchj->setvalmin = pbi->setvalmin;
          patchj->valmax    = pbi->valmax;
          patchj->valmin    = pbi->valmin;
        }
      }
    }

  // find bounds

    GetBoundaryBounds();
    for(i=0;i<npatchinfo;i++){
      patchi = patchinfo + i;
      if(patchi->setvalmin==1&&patchi->setvalmax==1){
        patchi->doit=1;
      }
      else{
        patchi->doit=0;
      }
    }
  }
  UNLOCK_PATCH;

  if(GLOBcleanfiles==1)return NULL;

  // convert and compress files

  for(i=0;i<npatchinfo;i++){
    patchi = patchinfo + i;

    if(patchi->doit==1){
      LOCK_PATCH;
      if(patchi->inuse==1){
        UNLOCK_PATCH;
        continue;
      }
      patchi->inuse=1;
      UNLOCK_PATCH;

      if(patchi->is_geom == 0){
        ConvertBoundaryBNDF(patchi, thread_index);
      }
      else{
        ConvertBoundaryGEOM(patchi, thread_index);
      }
    }
    else{
      PRINTF("%s not compressed\n",patchi->file);
      PRINTF("  Min and Max for %s not set in .ini file\n",patchi->label.shortlabel);
    }
  }
  return NULL;
}
