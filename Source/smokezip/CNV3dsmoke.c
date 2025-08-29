#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "zlib.h"
#include "svzip.h"
#include "file_util.h"
#include "dmalloc.h"
#include "compress.h"

/* ------------------ Convert3DSmoke ------------------------ */

void Convert3DSmoke(smoke3d *smoke3di, int *thread_index){
  unsigned char *full_alphabuffer=NULL, *compressed_alphabuffer=NULL;
  FILE *smoke3dstream=NULL,*smoke3dsizestream=NULL;
  FILE *SMOKE3DFILE=NULL;
  char smoke3dfile_svz[1024], smoke3dsizefile_svz[1024];
  int nxyz[9];
  int nx, ny, nz;
  int version_local;
  int buffersize;
  unsigned int sizebefore, sizeafter;
  int count;
  float time_local;
  int nchars[2];
  int nfull_file,nfull_data;
  int ncompressed_rle;
  uLongf ncompressed_zlib;
  int returncode=0;
  int percent_done;
  int percent_next = PERCENT_SKIP;
  LINT data_loc;
  char *smoke3dfile;
  float time_max;

  smoke3dfile=smoke3di->file;
  smoke3di->compressed=0;

//int compress (Bytef *dest,   uLongf *destLen, const Bytef *source, uLong sourceLen);
//int uncompress (Bytef *dest,   uLongf *destLen, const Bytef *source, uLong sourceLen);

  if(GLOBcleanfiles==0){
#ifdef pp_THREAD
    int fileindex;

    fileindex = smoke3di + 1 - smoke3dinfo;
    sprintf(threadinfo[*thread_index].label,"s3d %i",fileindex);
#else
    PRINTF("Compressing 3D smokefile %s\n",smoke3dfile);
#endif
  }

  if(GetFileInfo(smoke3dfile,NULL,NULL)!=0){
    fprintf(stderr,"*** Warning:  %s does not exist\n",smoke3dfile);
    return;
  }
  SMOKE3DFILE=FOPEN(smoke3dfile,"rb");
  if(SMOKE3DFILE==NULL){
    fprintf(stderr,"*** Warning:  %s could not be opened\n",smoke3dfile);
    return;
  }

  // name 3d smoke flie

  if(GLOBdestdir!=NULL){
    strcpy(smoke3dfile_svz,GLOBdestdir);
    strcat(smoke3dfile_svz,smoke3di->filebase);
  }
  else{
    strcpy(smoke3dfile_svz,smoke3di->file);
  }
  strcat(smoke3dfile_svz,".svz");

  // name size file

  if(GLOBdestdir!=NULL){
    strcpy(smoke3dsizefile_svz,GLOBdestdir);
    strcat(smoke3dsizefile_svz,smoke3di->filebase);
  }
  else{
    strcpy(smoke3dsizefile_svz,smoke3di->file);
  }
  strcat(smoke3dsizefile_svz,".szz");

  // remove files if clean option is set

  if(GLOBcleanfiles==1){
    smoke3dstream=FOPEN(smoke3dfile_svz,"rb");
    if(smoke3dstream!=NULL){
      fclose(smoke3dstream);
      PRINTF("  Removing %s\n",smoke3dfile_svz);
      UNLINK(smoke3dfile_svz);
      LOCK_COMPRESS;
      GLOBfilesremoved++;
      UNLOCK_COMPRESS;
    }
    smoke3dsizestream=FOPEN(smoke3dsizefile_svz,"r");
    if(smoke3dsizestream!=NULL){
      fclose(smoke3dsizestream);
      PRINTF("  Removing %s\n",smoke3dsizefile_svz);
      UNLINK(smoke3dsizefile_svz);
      LOCK_COMPRESS;
      GLOBfilesremoved++;
      UNLOCK_COMPRESS;
    }
    fclose(SMOKE3DFILE);
    return;
  }

  if(GLOBoverwrite_s==0){
    smoke3dstream=FOPEN(smoke3dfile_svz,"rb");
    if(smoke3dstream!=NULL){
      fclose(smoke3dstream);
      fprintf(stderr,"*** Warning:  %s exists.\n",smoke3dfile_svz);
      fprintf(stderr,"*** Warning:     Use the -f option to overwrite smokezip compressed files\n");
      fclose(SMOKE3DFILE);
      return;
    }
  }

  smoke3dsizestream=FOPEN(smoke3dsizefile_svz,"w");
  smoke3dstream=FOPEN(smoke3dfile_svz,"wb");
  if(smoke3dstream==NULL||smoke3dsizestream==NULL
    ){
    if(smoke3dstream==NULL){
      fprintf(stderr,"*** Warning:  3dsmoke file, %s, could not be opened for output\n",smoke3dfile_svz);
    }
    if(smoke3dsizestream==NULL){
      fprintf(stderr,"*** Warning:  3dsmoke size file, %s, could not be opened for output\n",smoke3dsizefile_svz);
    }
    if(smoke3dsizestream!=NULL)fclose(smoke3dsizestream);
    if(smoke3dstream!=NULL)fclose(smoke3dstream);
    fclose(SMOKE3DFILE);
    return;
  }

  FORTSMOKEREAD(nxyz, 4, 8, SMOKE3DFILE,smoke3di->file_type);

  nxyz[0] = 1;
  version_local = nxyz[1];
  if(version_local==1){
    PRINTF("  already compressed\n");
    fclose(SMOKE3DFILE);
    fclose(smoke3dstream);
    fclose(smoke3dsizestream);
    return;
  }

  version_local=1;

  nxyz[1]=version_local;
  fwrite(nxyz,4,8,smoke3dstream);
  fprintf(smoke3dsizestream,"%i\n",version_local);

  nx = nxyz[3]-nxyz[2]+1;
  ny = nxyz[5]-nxyz[4]+1;
  nz = nxyz[7]-nxyz[6]+1;
  buffersize=2*(1.01*nx*ny*nz+600);
  smoke3di->nx=nx;
  smoke3di->ny=ny;
  smoke3di->nz=nz;
  NewMemory((void **)&full_alphabuffer,buffersize);
  NewMemory((void **)&compressed_alphabuffer,buffersize);

  count=-1;
  sizebefore=8;
  sizeafter=8;
  time_max=-1000000.0;
  for(;;){
    FORTSMOKEREADBR(&time_local, 4, 1, SMOKE3DFILE, smoke3di->file_type);
    FORTSMOKEREADBR(nchars, 4,2, SMOKE3DFILE,smoke3di->file_type);

    nfull_file=nchars[0];
    ncompressed_rle=nchars[1];

    // read compressed frame

    if(ncompressed_rle > 0){
      FORTSMOKEREADBR(compressed_alphabuffer, ncompressed_rle, 1, SMOKE3DFILE, smoke3di->file_type);
    }

    if(time_local<time_max)continue;
    count++;

    sizebefore+=12+ncompressed_rle;

    if(count%GLOBsmoke3dzipstep!=0)continue;
    time_max=time_local;

    // uncompress frame data (from RLE format)

    nfull_data=UnCompressRLE(compressed_alphabuffer, ncompressed_rle, full_alphabuffer);
    CheckMemory;
    if(nfull_file!=nfull_data){
      fprintf(stderr,"*** Warning frame size expected=%i frame size found=%i\n",nfull_file,nfull_data);
    }

    // compress frame data (into ZLIB format)

    ncompressed_zlib=buffersize;
    returncode=CompressZLIB(compressed_alphabuffer, &ncompressed_zlib, full_alphabuffer, nfull_data);
    CheckMemory;
    if(returncode!=0){
      fprintf(stderr,"*** Warning zlib compressor failed - frame %f\n",time_local);
    }

    data_loc=FTELL(SMOKE3DFILE);
    percent_done=100.0*(float)data_loc/(float)smoke3di->filesize;
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

    // write out new entries in the size (sz) file

    nchars[0]=nfull_data;
    nchars[1]=ncompressed_zlib;
    fwrite(&time_local,4,1,smoke3dstream);
    fwrite(nchars,4,2,smoke3dstream);
    if(ncompressed_zlib>0)fwrite(compressed_alphabuffer,1,ncompressed_zlib,smoke3dstream);
    sizeafter+=12+ncompressed_zlib;

// time, nframeboth, ncompressed_rle, ncompressed_zlib, 0
    int dummy=0;
    fprintf(smoke3dsizestream,"%f %i %i %i %i\n",time_local,nfull_data,ncompressed_rle,(int)ncompressed_zlib,dummy);
  }
#ifdef pp_THREAD
  {
    char before_label[256],after_label[256];

    GetFileSizeLabel(sizebefore,before_label);
    GetFileSizeLabel(sizeafter,after_label);
    smoke3di->compressed=1;
    sprintf(smoke3di->summary,"%s -> %s (%4.1f%s)",before_label,after_label,(float)sizebefore/(float)sizeafter,GLOBx);
    FFLUSH();
    threadinfo[*thread_index].stat=-1;
  }
#else
  PRINTF(" 100%s completed\n",GLOBpp);
  PRINTF("  records=%i, ",count);
  {
    char before_label[256],after_label[256];

    GetFileSizeLabel(sizebefore,before_label);
    GetFileSizeLabel(sizeafter,after_label);

    PRINTF("Sizes: original=%s, ",before_label);

    PRINTF("compressed=%s (%4.1f%s)\n\n",after_label,(float)sizebefore/(float)sizeafter,GLOBx);
    FFLUSH();
  }
#endif

  // close files and free buffers

  fclose(SMOKE3DFILE);

  fclose(smoke3dstream);
  fclose(smoke3dsizestream);
  FREEMEMORY(full_alphabuffer);
  FREEMEMORY(compressed_alphabuffer);
}

/* ------------------ convert_smoke3ds ------------------------ */

void *Compress3DSmokes(void *arg){
  int i;
  smoke3d *smoke3di;
  int *thread_index;

  thread_index = (int *)arg;

  for(i=0;i<nsmoke3dinfo;i++){
    smoke3di = smoke3dinfo + i;
    LOCK_SMOKE;
    if(smoke3di->inuse==1){
      UNLOCK_SMOKE;
      continue;
    }
    smoke3di->inuse=1;
    UNLOCK_SMOKE;
    Convert3DSmoke(smoke3di,thread_index);
    CheckMemory;
  }
  return NULL;
}
