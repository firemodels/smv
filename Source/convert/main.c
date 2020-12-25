#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef pp_OSX
#include <unistd.h>
#endif
#include "string_util.h"
#include "MALLOCC.h"


#define LENBUFFER 255
char buffer[LENBUFFER], buffcopy[LENBUFFER], *buff;

/* ------------------ GetTokens ------------------------ */

int GetTokens(unsigned char *tokens){
  unsigned char *token;
  int ntokens=0;

  if(strstr(buffer, "0x")==NULL)return ntokens;
  token = strtok(buffer, ",");
  for(;;){
    int val;

    if(token==NULL)return ntokens;
    token = TrimFrontBack(token);
    if(strlen(token)==0)return ntokens;
    sscanf(token, "%i", &val);
    tokens[ntokens] = val;
    ntokens++;
    token = strtok(NULL, ",");
  }
}

/* ------------------ GetPixelData ------------------------ */

//static const GLubyte ch255data[] = {
//0x70,0x70,0x18,0x18,0x18,0x18,0x3c,0x24,0x66,0x66,0x66,0xc3,0xc3,0xc3,0x0,0x66,
//0x66,
//};

void GetPixelData(FILE *stream, unsigned char *bytes, int *nbytes){
  int ntokens, i;
  unsigned char tokens[100];

  ntokens = GetTokens(tokens);
  if(ntokens>0)memcpy(bytes, tokens, ntokens);
  *nbytes = ntokens;

  for(;;){
    if(fgets(buffer, LENBUFFER, stdin)==NULL)break;
    ntokens = GetTokens(tokens);
    if(ntokens==0)return;
    memcpy(bytes+(*nbytes), tokens, ntokens);
    *nbytes += ntokens;
  }
  return;
}

/* ------------------ GetPixelInfo ------------------------ */

int GetPixelInfo(FILE *stream, int *byte_types, char *label_prefix, char *label_suffix){
  for(;;){
    char *dataptr, *token, *brace, buffer_copy[100];
    int vals[5], i;

    if(fgets(buffer, LENBUFFER, stdin)==NULL)return 0;

    if(strstr(buffer, "BitmapCharRec")==NULL)continue;
    dataptr = strchr(buffer, '{');
    if(dataptr==NULL)return 0;

    strcpy(buffer_copy, buffer);
    brace = strchr(buffer_copy, '{');
    brace[0] = 0;
    strcpy(label_prefix, buffer_copy);

    dataptr++;
    token = strtok(dataptr, ",");
    sscanf(token, "%i", byte_types);
    for(i = 1; i<5; i++){
      token = strtok(NULL, ",");
      sscanf(token, "%i", byte_types+i);
    }
    token = strtok(NULL, ",");
    if(token == NULL)return 0;
    brace = strchr(token, '}');
    if(brace==NULL)return 0;
    brace[0] = 0;

    strcpy(label_suffix, TrimFrontBack(token));
    break;
  }
  return 1;
}

/* ------------------ ConvertPixelData ------------------------ */

#define IJ_IN(row,col)  ((row)*ncols_data + (col))
#define IJ_OUT(row,col) ((row)*(2*ncols_data) + (col))

void ConvertPixelData(unsigned char *bytes_in, int nbytes_in, unsigned char *bytes_out, int *nbytes_out, int *types_in, int *types_out){
  int row, nrows_font, ncols_font, i;
  int nrows_data, ncols_data;
  unsigned char high[] = {0x0, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f, 0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff};

  nrows_font = types_in[1];
  ncols_font = types_in[0];
  nrows_data = nrows_font;
  ncols_data = ((ncols_font-1)/8+1);

  for(row = 0; row<nrows_data; row++){
    int col;

    for(col = 0; col<ncols_data; col++){
      unsigned char val;
      unsigned char maskL = 0xF0, maskR = 0x0F;
      unsigned char valL, valR;
      unsigned char valLout, valRout;

      val = bytes_in[ IJ_IN(row, col) ];
      valL = val&maskL;
      valR = (val&maskR)>>4;
      valLout = high[valL];
      valRout = high[valR];
      bytes_out[IJ_OUT(2*row, 2*col)] = valLout;
      bytes_out[IJ_OUT(2*row, 2*col)+1] = valRout;
      bytes_out[IJ_OUT(2*row+1, 2*col)] = valLout;
      bytes_out[IJ_OUT(2*row+1, 2*col)+1] = valRout;
    }
  }
  *nbytes_out = 4*nbytes_in;
  for(i = 0; i<5; i++){
    types_out[i] = 2*types_in[i];
  }
}

/* ------------------ OutputPixelDataInfo ------------------------ */

void OutputPixelDataInfo(unsigned char *bytes, int nbytes, int *byte_types, char *label_prefix, char *label_suffix){
  int i;

  for(i = 0; i<nbytes; i++){
    if(bytes[i]==0){
      printf("0x00,");
    }
    else{
      printf("%#x,", bytes[i]);
    }
    if(i%16==15)printf("\n");
  }
  printf("\n};\n");

  printf("\n");
  printf("%s {%i,%i,%i,%i,%i,%s};\n", label_prefix, byte_types[0], byte_types[1], byte_types[2], byte_types[3], byte_types[4], label_suffix);

}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){

  initMALLOC();
  SetStdOut(stdout);

  for(;;){
    unsigned char bytes[1000];
    unsigned char bytes2[1000];
    int byte_types[5], byte_types2[5];

    if(fgets(buffer, LENBUFFER, stdin)==NULL)break;
    TrimBack(buffer);
    if(strstr(buffer, "static const")==NULL){
      printf("%s\n", buffer);
      continue;
    }
    if(strstr(buffer, "GLubyte")!=NULL){
      int nbytes, nbytes2;
      char *brace, label_prefix[100], label_suffix[100];

      strcpy(buffcopy, buffer);
      brace = strchr(buffcopy, '{');
      if(brace!=NULL)brace[1] = 0;

      GetPixelData(stdin, bytes, &nbytes);
      if(GetPixelInfo(stdin, byte_types, label_prefix, label_suffix)==0)continue;
      printf("%s\n", buffcopy);
      ConvertPixelData(bytes, nbytes, bytes2, &nbytes2, byte_types, byte_types2);
      OutputPixelDataInfo(bytes2, nbytes2, byte_types2, label_prefix, label_suffix);
    }
    else{
      printf("%s\n", buffer);
    }
  }
  fprintf(stderr, "complete\n");


  return 0;
}
