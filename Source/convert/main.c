#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef pp_OSX
#include <unistd.h>
#endif
#include "string_util.h"
#include "dmalloc.h"


#define LENBUFFER 255
char buffer[LENBUFFER], buffcopy[LENBUFFER], *buff;
char label_prefix[100], label_suffix[100];

/* ------------------ GetTokens ------------------------ */

int GetTokensComma(unsigned char *tokens){
  unsigned char *token;
  int ntokens=0;

  if(strstr(buffer, "0x")==NULL)return ntokens;
  token = (unsigned char *)strtok(buffer, ",");
  for(;;){
    int val;

    if(token==NULL)return ntokens;
    token = (unsigned char *)TrimFrontBack((char *)token);
    if(strlen((char *)token)==0)return ntokens;
    sscanf((const char *)token, "%i", &val);
    tokens[ntokens] = val;
    ntokens++;
    token = (unsigned char *)strtok(NULL, ",");
  }
}

/* ------------------ GetPixelData ------------------------ */

//static const GLubyte ch255data[] = {
//0x70,0x70,0x18,0x18,0x18,0x18,0x3c,0x24,0x66,0x66,0x66,0xc3,0xc3,0xc3,0x0,0x66,
//0x66,
//};

void GetPixelData(FILE *stream, unsigned char *bytes, int *nbytes){
  unsigned char tokens[100];
  int ntokens;

  ntokens = GetTokensComma(tokens);
  if(ntokens>0)memcpy(bytes, tokens, ntokens);
  *nbytes = ntokens;

  for(;;){
    if(fgets(buffer, LENBUFFER, stdin)==NULL)break;
    ntokens = GetTokensComma(tokens);
    if(ntokens==0)return;
    memcpy(bytes+(*nbytes), tokens, ntokens);
    *nbytes += ntokens;
  }
  return;
}

/* ------------------ GetPixelInfo ------------------------ */

int GetPixelInfo(FILE *stream, int *byte_types){
  for(;;){
    char *dataptr, *token, *brace, buffer_copy[100];
    int i;

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

void ConvertPixelData(unsigned char *bytes_in, int nbytes_in, unsigned char *bytes_out, int *nbytes_out, int *types_in, int *ncols_data_in, int *nrows_data_out, int *ncols_data_out){
  int row, nrows_font, ncols_font;
  int nrows_data, ncols_data;
  unsigned char high[] = {0x0, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f, 0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff};

  nrows_font = types_in[1];
  ncols_font = types_in[0];
  nrows_data = nrows_font;
  ncols_data = ((ncols_font-1)/8+1);
  *ncols_data_in = ncols_data;
  *ncols_data_out = ((2*ncols_font-1)/8+1);
  *nrows_data_out = 2*nrows_font;

  for(row = 0; row<nrows_data; row++){
    int col;

    for(col = 0; col<ncols_data; col++){
      unsigned char val;
      unsigned char valL, valR;
      unsigned char valLout, valRout;

      val = bytes_in[ IJ_IN(row, col) ];
      valL =  (val&0xf0)>>4;
      valR =  val&0x0f;
      valLout = high[valL];
      valRout = high[valR];
      bytes_out[IJ_OUT(2*row,   2*col)]   = valLout;
      bytes_out[IJ_OUT(2*row,   2*col+1)] = valRout;
      bytes_out[IJ_OUT(2*row+1, 2*col)]   = valLout;
      bytes_out[IJ_OUT(2*row+1, 2*col+1)] = valRout;
    }
  }
  *nbytes_out = 4*nrows_data*ncols_data;
}

/* ------------------ testbit ------------------------ */

int testbit(unsigned char x, unsigned char y){
  if(x!=0&&y==1)return 1;
  if(x==0&&y==0)return 1;
  return 0;
}

/* ------------------ OutputPixelDataInfo ------------------------ */

void AdjustBytes(unsigned char *bytes, int i11, int i21, int i31, int i41){
  int i;
  unsigned char val1, val2, val3, val4;
  unsigned char mask[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
  unsigned char check1[] = {0,0,1,1}, check2[] = {1,1,0,0};

  val1 = bytes[i11];
  val2 = bytes[i21];
  val3 = bytes[i31];
  val4 = bytes[i41];

  // 0011xxxx
  // 0011xxxx
  // 1100xxxx
  // 1100xxxx

  for(i = 0; i<5; i += 2){
    int j, doit;

    doit = 1;
    for(j = 0; j<4; j++){
      if(testbit(val1&mask[i+j], check1[j])==1&&
         testbit(val2&mask[i+j], check1[j])==1&&
         testbit(val3&mask[i+j], check2[j])==1&&
         testbit(val4&mask[i+j], check2[j])==1){
      }
      else{
        doit = 0;
        break;
      }
    }
    if(doit==1){
    // set val2 i+1 and val3 i+2
      bytes[i21] |= mask[i+1];
      bytes[i31] |= mask[i+2];
      return;
    }
  }
  // 1100xxxx
  // 1100xxxx
  // 0011xxxx
  // 0011xxxx
  for(i = 0; i<5; i += 2){
    int j, doit;

    doit = 1;
    for(j = 0; j<4; j++){
      if(testbit(val1&mask[i+j], check2[j])==1&&
         testbit(val2&mask[i+j], check2[j])==1&&
         testbit(val3&mask[i+j], check1[j])==1&&
         testbit(val4&mask[i+j], check1[j])==1){
      }
      else{
        doit = 0;
        break;
      }
    }
    if(doit==1){
    // set val2 i+2 and val3 i+1
      bytes[i21] |= mask[i+2];
      bytes[i31] |= mask[i+1];
      return;
    }
  }
}

/* ------------------ OutputPixelDataInfo ------------------------ */

void OutputPixelDataInfo(unsigned char *bytes, int nbytes, int *byte_types, int ncols_data, int nrows_out, int ncols_out){
  int i, count=0;

  if(1==0){
    for(i = 0; i<nrows_out-4; i+=2){
      int j;

      for(j = 0; j<ncols_out; j++){
        AdjustBytes(bytes, IJ_OUT(i, j), IJ_OUT(i+1, j), IJ_OUT(i+2, j), IJ_OUT(i+3, j));
      }
    }
  }
  for(i = 0; i<nrows_out; i++){
    int j;

    for(j = 0; j<ncols_out; j++){
      int ii;

      ii = IJ_OUT(i, j);
      if(bytes[ii]==0){
        printf("0x00,");
      }
      else{
        printf("%#x,", bytes[ii]);
      }
      if(count%16==15)printf("\n");
      count++;
    }
  }
  printf("\n};\n");

  printf("\n");
  printf("%s {%i,%i,%i,%i,%i,%s};\n", label_prefix, 2*byte_types[0], 2*byte_types[1], 2*byte_types[2], 2*byte_types[3], 2*byte_types[4], label_suffix);

}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){

  initMALLOC();
  SetStdOut(stdout);

  for(;;){
    unsigned char bytes[1000];
    unsigned char bytes2[1000];
    int byte_types[5];

    if(fgets(buffer, LENBUFFER, stdin)==NULL)break;
    TrimBack(buffer);
    if(strstr(buffer, "static const")==NULL){
      printf("%s\n", buffer);
      continue;
    }
    if(strstr(buffer, "GLubyte")!=NULL){
      int nbytes, nbytes2, nrows_out, ncols_out, ncols_data_in;
      char *brace;

      strcpy(buffcopy, buffer);
      brace = strchr(buffcopy, '{');
      if(brace!=NULL)brace[1] = 0;

      GetPixelData(stdin, bytes, &nbytes);
      if(GetPixelInfo(stdin, byte_types)==0)continue;
      printf("%s\n", buffcopy);
      ConvertPixelData(bytes, nbytes, bytes2, &nbytes2, byte_types, &ncols_data_in, &nrows_out, &ncols_out);
      OutputPixelDataInfo(bytes2, nbytes2, byte_types, ncols_data_in, nrows_out, ncols_out);
    }
    else{
      printf("%s\n", buffer);
    }
  }
  fprintf(stderr, "complete\n");


  return 0;
}
