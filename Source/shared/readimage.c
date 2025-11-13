#include "options_common.h"
#include "dmalloc.h"
#include "string_util.h"
#include "gd.h"

#include <string.h>


#define BUFFER_LEN 255

/* ------------------ ReadJPEG ------------------------ */

unsigned char *ReadJPEG(const char *filename,int *width, int *height, int *is_transparent){

  FILE *file;
  gdImagePtr image;
  unsigned char *dataptr,*dptr;
  int i,j;
  unsigned int intrgb;
  int WIDTH, HEIGHT;

  file = FOPEN(filename, "rb");
  if(file == NULL)return NULL;
  image = gdImageCreateFromJpeg(file);
  fclose(file);
  if(image==NULL)return NULL;
  WIDTH=gdImageSX(image);
  HEIGHT=gdImageSY(image);
  *width=WIDTH;
  *height=HEIGHT;
  if( NewMemory((void **)&dataptr,(unsigned int)(4*WIDTH*HEIGHT) )==0){
    gdImageDestroy(image);
    return NULL;
  }
  dptr=dataptr;
  *is_transparent = 0;
  for(i = 0; i<HEIGHT; i++){
    for(j=0;j<WIDTH;j++){
      unsigned int a;

      intrgb=(unsigned int)gdImageGetPixel(image,j,(unsigned int)(HEIGHT-(1+i)));
      *dptr++ = (intrgb>>16)&255;
      *dptr++ = (intrgb>>8)&255;
      *dptr++ = intrgb&255;
      a = (intrgb>>24)&255;
      a = 255-a;
      if(a<129)a = 0;
      if(a==0)*is_transparent = 1;
      *dptr++ = (unsigned char)a;
    }
  }
  gdImageDestroy(image);
  return dataptr;

}

/* ------------------ ReadPNG ------------------------ */

unsigned char *ReadPNG(const char *filename,int *width, int *height, int *is_transparent){

  FILE *file;
  gdImagePtr image;
  unsigned char *dataptr,*dptr;
  int i,j;
  unsigned int intrgb;

  file = FOPEN(filename, "rb");
  if(file == NULL)return NULL;
  image = gdImageCreateFromPng(file);
  fclose(file);
  *width=gdImageSX(image);
  *height=gdImageSY(image);
  if( NewMemory((void **)&dataptr,(unsigned int)(4*(*width)*(*height)) )==0){
    gdImageDestroy(image);
    return NULL;
  }
  dptr=dataptr;
  *is_transparent = 0;
  for(i = 0; i<*height; i++){
    for(j=0;j<*width;j++){
      unsigned int a;

      intrgb=(unsigned int)gdImageGetPixel(image,j,(unsigned int)(*height-(1+i)));
      *dptr++ = (intrgb>>16)&255;
      *dptr++ = (intrgb>>8)&255;
      *dptr++ = intrgb&255;
      a = (intrgb>>24)&255;
      a = 255-a;
      if(a<129)a=0;
      if(a==0)*is_transparent = 1;
      *dptr++ = (unsigned char)a;
    }
  }
  gdImageDestroy(image);
  return dataptr;

}

/* ------------------ ReadPicture ------------------------ */

unsigned char *ReadPicture(char *texturedir, char *filename, int *width, int *height, int *is_transparent, int printflag){
  char *ext;
  unsigned char *returncode;
  char *filebuffer=NULL;
  int allocated;

  if(filename==NULL)return NULL;
  if(FILE_EXISTS(filename)==YES){
    filebuffer=filename;
    allocated=0;
  }
  else{
    size_t lenbuffer;

    if(texturedir==NULL){
      if(printflag==1){
        fprintf(stderr,"*** Error: texture file: %s unavailable\n",filename);
      }
      return NULL;
    }
    else{
      FILE *stream;

      lenbuffer=strlen(filename)+strlen(texturedir)+1;
      NewMemory((void **)&filebuffer,(unsigned int)(lenbuffer+1));
      allocated=1;
      strcpy(filebuffer,texturedir);
      strcat(filebuffer,dirseparator);
      strcat(filebuffer,filename);
      stream=FOPEN(filebuffer,"rb");
      if(stream==NULL){
        if(printflag==1){
          fprintf(stderr,"*** Error: texture file: %s unavailable\n",filebuffer);
        }
        FREEMEMORY(filebuffer);
        return NULL;
      }
      else{
        fclose(stream);
      }
    }
  }


  if(printflag==1)PRINTF("Loading texture:%s ",filebuffer);
  ext = filebuffer + strlen(filebuffer) - 4;
  if(strncmp(ext,".jpg",4)==0||strncmp(ext,".JPG",4)==0){
    returncode = ReadJPEG(filebuffer,width,height,is_transparent);
  }
  else if(strncmp(ext,".png",4)==0||strncmp(ext,".PNG",4)==0){
    returncode = ReadPNG(filebuffer,width,height,is_transparent);
  }
  else{
    if(allocated==1){
      FREEMEMORY(filebuffer);
    }
    return NULL;
  }
  if(allocated==1){
    FREEMEMORY(filebuffer);
  }
  if(printflag==1){
    if(returncode!=NULL){
      PRINTF(" - completed\n");
    }
    else{
      PRINTF(" - failed\n");
      fprintf(stderr,"*** Error: attempt to input %s failed\n",filename);
    }
  }
  return returncode;
}

#define GETBIT(val, ibit)         (((val) >> (ibit)) &1)
#define SETBIT(val, bitval, ibit) (val |= ((bitval) << (ibit)))

/* ------------------ DecodePNGData ------------------------ */

unsigned char *DecodePNGData(unsigned char *buffer, int nbuffer, int *ndataptr, int skip, int channel){
  int i, signature_base = 314159, signature = 0, ndata = 0;
  unsigned char *dataptr = NULL;

  // decode signature

  for(i = 0; i < 32; i++){
    unsigned char *c, bitval;

    c = buffer + skip * i + channel;
    bitval = GETBIT(*c, 0);
    SETBIT(signature, bitval, i);
  }
  if(signature != signature_base)return NULL;

  // decode ndata

  buffer += 32 * skip;
  for(i = 0; i < 32; i++){
    unsigned char *c, bitval;

    c = buffer + skip * i + channel;
    bitval = GETBIT(*c, 0);
    SETBIT(ndata, bitval, i);
  }
  if(ndata <= 0)return NULL;

  // decode data

  *ndataptr = ndata;
  if(NewMemory((void **)&dataptr, ndata + 1) == 0){
    return NULL;
  }

  buffer += 32 * skip;
  for(i = 0; i < ndata; i++){
    int j;
    unsigned char *data;

    data = dataptr + i;
    *data = 0;
    for(j = 0; j < 8; j++){
      unsigned char *c, bitval;

      c = buffer + skip * (8 * i + j) + channel;
      bitval = GETBIT(*c, 0);
      SETBIT(*data, bitval, j);
    }
  }
  dataptr[ndata] = 0;
  return dataptr;
}

/* ------------------ EncodePNGData ------------------------ */

void EncodePNGData(unsigned char *buffer, int nbuffer, unsigned char *data, int ndata, int skip, int channel){
  int signature = 314159, i;

  // encode signature

  for(i = 0;i < 32;i++){
    unsigned char *c;

    c = buffer + skip * i + channel;
    *c &= 0xFE;
    *c |= GETBIT(signature, i);
  }

  // encode ndata

  buffer += 32 * skip;
  for(i = 0; i < 32; i++){
    unsigned char *c;

    c = buffer + skip * i + channel;
    *c &= 0xFE;
    *c |= GETBIT(ndata, i);
  }

  // encode data

  buffer += 32 * skip;
  for(i = 0; i < ndata; i++){
    int j;
    unsigned char *dataptr;

    dataptr = data + i;
    for(j = 0; j < 8; j++){
      unsigned char *c;

      c = buffer + skip * (8 * i + j) + channel;
      *c &= 0xFE;
      *c |= GETBIT(*dataptr, j);
    }
  }
}

