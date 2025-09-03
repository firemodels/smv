#include "options.h"
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
