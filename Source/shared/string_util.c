#define IN_STRING_UTIL
#include "options.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#ifdef pp_OSX
#include <unistd.h>
#endif
#ifdef WIN32
#ifdef __MINGW32__
#undef S_IFBLK
#undef S_ISBLK
#undef S_ISFIFO
#undef S_ISDIR
#undef S_ISCHR
#undef S_ISREG
#endif
#include <dirent_win.h>
#else
#include <dirent.h>
#endif
#include "MALLOC.h"
#include "datadefs.h"
#include "file_util.h"
#ifdef pp_HASH
#include "mbedtls/md5.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha1.h"
#endif


unsigned int *random_ints, nrandom_ints;

/* ----------------------- AppendString ----------------------------- */

char *AppendString(char *S1, char *S2) {
  strcpy(append_string, S1);
  strcat(append_string, S2);
  return append_string;
}

/* ----------------------- InitRandAB ----------------------------- */

void InitRandAB(int size){
  unsigned int i;

  nrandom_ints=size;
  NewMemory((void **)&random_ints,nrandom_ints*sizeof(unsigned int));
  for(i=0;i<nrandom_ints;i++){
    random_ints[i]=rand();
  }
}

/* ----------------------- RandAB ----------------------------- */

float RandAB(int seed, float minval, float maxval){
  seed = (1 + ABS(seed)) % nrandom_ints;
  return  minval + (maxval-minval)*(float)random_ints[seed]/(float)RAND_MAX;
}

/* ----------------------- FParseCSV ----------------------------- */

void FParseCSV(char *buffer, float *vals, int *valids, int ncols, int *ntokens){

//  copy comma delimited values from buffer into floating point array vals
//  returning number of values found in ntokens

  int nt=0;
  char *token;

  token=strtok(buffer,",");
  while(token!=NULL&&nt<ncols){
    if(STRCMP(token,"NULL")==0){
      valids[nt]=0;
      vals[nt]=0.0;
    }
    else{
      valids[nt]=1;
      sscanf(token,"%f",&vals[nt]);
    }
    nt++;
    token=strtok(NULL,",");
    if(token!=NULL)TrimBack(token);
  }
  *ntokens=nt;
}

/* ----------------------- ParseCSV ----------------------------- */

void ParseCSV(char *buffer, char **tokens, int *ntokens){

//  copy comma delimited values from buffer into character array tokens
//  returning number of values found in ntokens

  int nt=0;
  int i;
  int lenbuffer;
  int inside_quote=0;

  lenbuffer=strlen(buffer);
  for(i=0;i<lenbuffer;i++){
    if(buffer[i]=='"'){
      buffer[i]=' ';
      inside_quote=1-inside_quote;
      continue;
    }
    if(inside_quote==0&&buffer[i]==',')buffer[i]=0;
  }
  tokens[nt++]=buffer;
  for(i=1;i<lenbuffer;i++){
    if(buffer[i]==0){
      tokens[nt++]=buffer+i+1;
    }
  }
  *ntokens=nt;
}

/* ------------------ GetRowCols ------------------------ */

int GetRowCols(FILE *stream, int *nrows, int *ncols){

//  find number of rows (nrows) and number of columns (ncols) in
//  comma delimited file pointed to by stream and returns the length
//  of the longest line

  int nnrows=0,nncols=1,maxcols=0,linelength=0,maxlinelength=0;
  int inside_quote=0;

  while(!feof(stream)){
    char ch;

    ch = getc(stream);
    if(ch=='"')inside_quote=1-inside_quote;
    linelength++;
    if(inside_quote==0&&ch == ',')nncols++;
    if(ch=='\n'){
      if(linelength>maxlinelength)maxlinelength=linelength;
      if(nncols>maxcols)maxcols=nncols;
      linelength=0;
      inside_quote=0;
      nncols=1;
      nnrows++;
    }
  }
  *nrows=nnrows;
  *ncols=maxcols;
  rewind(stream);
  return maxlinelength;
}

/* ------------------ GetGitInfo ------------------------ */

#ifndef pp_GITHASH
  #define pp_GITHASH "unknown"
#endif
#ifndef pp_GITDATE
#define pp_GITDATE "unknown"
#endif
void GetGitInfo(char *githash, char *gitdate){
  char rev[256], *beg=NULL;

  strcpy(rev,pp_GITHASH);
  TrimBack(rev);
  beg = TrimFront(rev);
  if(strlen(beg)>0){
    strcpy(githash,beg);
  }
  else{
    strcpy(githash,"unknown");
  }

  strcpy(rev, pp_GITDATE);
  TrimBack(rev);
  beg = TrimFront(rev);
  if(strlen(beg)>0){
    strcpy(gitdate, beg);
  }
  else{
    strcpy(gitdate, "unknown");
  }
}


/* ------------------ StripQuotes ------------------------ */

void StripQuotes(char *buffer){

// replaces quotes (") with blanks in the character string buffer

  char *c;

  for(c=buffer;c<buffer+strlen(buffer);c++){
    if(*c=='"')*c=' ';
  }
}
/* ------------------ StripCommas ------------------------ */

void StripCommas(char *buffer){

//  replaces commas (,) with blanks in the character string buffer

  char *c;

  for(c=buffer;c<buffer+strlen(buffer);c++){
    if(*c==',')*c=' ';
  }
}

/* ------------------ RandInt ------------------------ */

int RandInt(int min, int max){

//  returns a random integer inclusively between min and max

  int return_val;

  if(min>max){
    return_val = max+((min-max+1)*(float)rand()/((float)RAND_MAX+1.0));
  }
  else{
    return_val = min+((max-min+1)*(float)rand()/((float)RAND_MAX+1.0));
  }
  return return_val;
}

/* ------------------ RandStr ------------------------ */

#ifdef WIN32
#define GETPID GetCurrentProcessId
#else
#define GETPID getpid
#endif

char *RandStr(char* str, int length){

//  returns a random character string of length length

    int i;
    int seed;

    if(str==NULL||length<=0)return NULL;

    seed = time(NULL)%1000;
    seed += GETPID();
    srand(seed);

    for(i=0;i<length;i++){
      str[i]=(char)RandInt(65,90);
    }
    str[length]=0;
    return str;
}

/* ------------------ TrimCommas ------------------------ */

void TrimCommas(char *line){
  char *c;

  for(c = line + strlen(line) - 1;c>=line;c--){
    if(isspace((unsigned char)(*c)))continue;
    if(strncmp(c,",",1)!=0)break;
    *c=' ';
  }
}

/* ------------------ trim ------------------------ */

void TrimBack(char *line){

  //  removes trailing white space from the character string line

  char *c;
  size_t len;

  if(line==NULL)return;
  len = strlen(line);
  if(len==0)return;
  for(c=line+len-1; c>=line; c--){
    if(isspace((unsigned char)(*c)))continue;
    *(c+1)='\0';
    return;
  }
  *line='\0';
}

/* ------------------ TrimFront ------------------------ */

char *TrimFront(char *line){

//  returns first non-blank character at the begininn of line

  char *c;

  for(c=line;c<=line+strlen(line)-1;c++){
    if(!isspace((unsigned char)(*c)))return c;
  }
  return line;
}

/* ------------------ TrimZeros ------------------------ */

void TrimZeros(char *line){
  char *c;

  //  removes trailing zeros in the floating point number found in line

  for(c = line+strlen(line)-1; c>line; c--){
    if((c[0]=='0'&&c[-1]=='.')||c[0]!='0'){
      c[1] = '\0';
      return;
    }
    // if we got here then c[0]==0 and c[-1]!=. so continue and look for another '0'
  }
  line[0] = '\0';
}

/* ------------------ TrimMZeros ------------------------ */

void TrimMZeros(char *line){

//  removes trailing zeros in each floating point number found in line

  char linecopy[1024];
  char *token;

  TrimBack(line);
  strcpy(linecopy,line);
  token=strtok(linecopy," ");
  strcpy(line,"");
  while(token!=NULL){
    TrimZeros(token);
    strcat(line,token);
    strcat(line," ");
    token=strtok(NULL," ");
  }
}

/* ------------------ STRNCMP ------------------------ */

int STRNCMP(const char *s1, const char *s2, const int n){
  int i=0;

  //  same as the standard function, strncmp, but ignores case

  while(toupper(*s1) == toupper(*s2++)&&i<n){
    i++;
    if(*s1++ == 0)return (0);
  }
  return (toupper(*(const unsigned char *)s1) - toupper(*(const unsigned char *)(s2 - 1)));
}

/* ------------------ STRCMP ------------------------ */

int STRCMP(const char *s1, const char *s2){

//  same as the standard function, strcmp, but ignores case

  while (toupper(*s1) == toupper(*s2++)){
    if(*s1++ == 0)return (0);
  }
  return (toupper(*(const unsigned char *)s1) - toupper(*(const unsigned char *)(s2 - 1)));
}

/* ------------------ STRSTR ------------------------ */

char *STRSTR(char *string, const char *key){
  char *k,*s,*ss;

  if(string==NULL||key==NULL)return NULL;

  for(s=string;*s!=0;s++){
    for(k=(char *)key;*k!=0;k++){
      ss = s + (k-key);
      if(*ss==0)return NULL;
      if(toupper(*ss)!=toupper(*k))break;
    }
    if(*k==0)return s;
  }
  return NULL;
}

/* ------------------ ScaleString ------------------------ */

void ScaleString(const char *stringfrom, char *stringto, const float *scale){
  float val;

  sscanf(stringfrom,"%f",&val);
  val = scale[0]*val+scale[1];
  Num2String(stringto,val);
}

/* ------------------ ScaleFloat2String ------------------------ */

void ScaleFloat2String(float floatfrom, char *stringto, const float *scale){
  float val;

  val = scale[0]*floatfrom+scale[1];
  Num2String(stringto,val);
}

/* ------------------ Num2String ------------------------ */

void Num2String(char *string, float tval){
  float tval2,mant10;
  int exp10;

  tval2=ABS(tval);
  if(0.01-.001<=tval2&&tval2<0.1){
    sprintf(string,"%3.2f",tval);
  }
  else if(0.1<=tval2&&tval2<1.0){
    sprintf(string,"%3.2f",tval);
  }
  else if(1.0<=tval2&&tval2<10.0){
    sprintf(string,"%3.2f",tval);
  }
  else if(10.0<=tval2&&tval2<100.0){
    sprintf(string,"%3.1f",tval);
  }
  else if(100.0<=tval2&&tval2<1000.0){
    sprintf(string,"%3.0f",tval);
  }
  else if(1000.0<=tval2&&tval2<10000.0){
    sprintf(string,"%4.0f",tval);
  }
  else if(10000.0<=tval2&&tval2<100000.0){
    sprintf(string,"%5.0f",tval);
    }
  else if(tval2==0.0){
    STRCPY(string,"0.00");
  }
  else{
    mant10 = FrExp10(tval,&exp10);
    mant10 = (float)((int)(10.0f*mant10+0.5f))/10.0f;
    if(mant10>=10.0f){
      mant10/=10.0f;
      exp10++;
    }
    if(exp10<-99){
      STRCPY(string,"0.00");
    }
    else if(exp10>=-99&&exp10<-9){
      sprintf(string,"%2.1f%i",mant10,exp10);
    }
    else if(exp10>99){
      STRCPY(string,"***");
    }
    else{
      if(exp10==0){
        sprintf(string,"%2.1f",mant10);
      }
      else{
        sprintf(string,"%2.1fE%i",mant10,exp10);
      }
    }

    /*sprintf(string,"%1.1e",tval); */
  }
  if(strlen(string)>9)fprintf(stderr,"***fatal error - overwriting string\n");
}

/* ------------------ TrimFrontBack ------------------------ */

char *TrimFrontBack(char *buffer){

//  removes trailing blanks from buffer and returns a pointer to the first non-blank character

  if(buffer==NULL)return NULL;
  TrimBack(buffer);
  return TrimFront(buffer);
}

/* ------------------ GetChid ------------------------ */

char *GetChid(char *file, char *buffer){
  FILE *stream;
  char *chidptr,*c;
  unsigned int i;
  int found1st, found2nd;

  if(file==NULL)return NULL;
  stream=fopen(file,"r");
  if(stream==NULL)return NULL;

  found1st=0;
  found2nd=0;
  chidptr=NULL;
  while(!feof(stream)){
    found1st=0;
    found2nd=0;

    if(fgets(buffer,255,stream)==NULL)break;
    chidptr=strstr(buffer,"CHID");
    if(chidptr==NULL)continue;

    chidptr+=5;
    for(i=0;i<strlen(chidptr);i++){
      c=chidptr+i;
      if(*c=='\''){
        found1st=1;
        chidptr=c+1;
        break;
      }
    }
    if(found1st==0)break;

    for(i=0;i<strlen(chidptr);i++){
      c=chidptr+i;
      if(*c=='\''){
        found2nd=1;
        *c=0;
        break;
      }
    }
    break;
  }
  fclose(stream);
  if(found1st==0||found2nd==0)chidptr=NULL;
  return chidptr;
}
#ifdef pp_GPU

/* ------------------ LogBase2 ------------------------ */

int LogBase2(float xx){

//  returns the log base 2 of the floating point number xx

  int r = 0;
  unsigned int x;

  x=xx;
  while( (x >> r) != 0){
    r++;
  }
  return r-1; // returns -1 for x==0, floor(log2(x)) otherwise
}
#endif

/* ------------------ Array2String ------------------------ */

void Array2String(float *vals, int nvals, char *string){

//  convert an array of floating point numbers to a character string

  char cval[30];
  int i;

  strcpy(string,"");
  if(nvals==0)return;
  for(i=0;i<nvals-1;i++){
    sprintf(cval,"%f",vals[i]);
    TrimZeros(cval);
    strcat(string,cval);
    strcat(string,", ");
  }
  sprintf(cval,"%f",vals[nvals-1]);
  TrimZeros(cval);
  strcat(string,cval);
}

/* ------------------ FrExp10 ------------------------ */

float FrExp10(float x, int *exp10){
  float xabs, mantissa;

  xabs = ABS((double)x);
  if(x==0.0f){
    *exp10=0;
    return 0.0f;
  }
  mantissa = log10((double)xabs);
  *exp10 = (int)floor((double)mantissa);

  mantissa = pow((double)10.0f,(double)mantissa-(double)*exp10);
  if(x<0)mantissa = -mantissa;
  return mantissa;
}

/* ------------------ GetFloatLabel ------------------------ */

char *GetFloatLabel(float val, char *label){
  if(val>=1000000.0){
    sprintf(label,"%.1fM", val/1000000.0);
  }
  else if(val>=1000.0&&val<1000000.0){
    sprintf(label, "%.1fK", val/1000.0);
  }
  else{
    sprintf(label, "%f", val);
  }
  return label;
}

/* ------------------ GetIntLabel ------------------------ */

char *GetIntLabel(int val, char *label){
  if(val>=1000000){
    sprintf(label, "%.1fM", (float)val/1000000.0);
  }
  else if(val>=1000&&val<1000000){
    sprintf(label, "%.1fK", (float)val/1000.0);
  }
  else{
    sprintf(label, "%i", val);
  }
  return label;
}

/* ------------------ GetString ------------------------ */

char *GetString(char *buffer){

//  return pointer to string contained between a pair of double quotes

  char *begin,*end;

  // if buffer contains msgid "string"
  // return a pointer to s in string

  begin=strchr(buffer,'"');
  if(begin==NULL)return NULL;
  begin++;
  end=strrchr(begin,'"');
  if(end==NULL)return NULL;
  end[0]=0;
  TrimBack(begin);
  begin = TrimFront(begin);
  if(strlen(begin)>0)return begin;
  return NULL;
}

  /* ------------------ Time2TimeLabel ------------------------ */

char *Time2TimeLabel(float sv_time, float dt, char *timelabel){
  char *timelabelptr;

  if(dt<0.001){
    sprintf(timelabel,"%4.4f",sv_time);
  }
  else if(dt>=0.001&&dt<0.01){
    sprintf(timelabel,"%4.3f",sv_time);
  }
  else if(dt>=0.01&&dt<0.1){
    sprintf(timelabel,"%4.2f",sv_time);
  }
  else{
    sprintf(timelabel,"%4.1f",sv_time);
  }
  TrimZeros(timelabel);
  TrimBack(timelabel);
  timelabelptr=TrimFront(timelabel);
  return timelabelptr;
}

/* ------------------ Match ------------------------ */

int Match(char *buffer, const char *key){
  size_t lenbuffer;
  size_t lenkey;

  lenkey=strlen(key);
  lenbuffer=strlen(buffer);
  if(lenbuffer<lenkey)return NOTMATCH; // buffer shorter than key so no match
  if(strncmp(buffer,key,lenkey) != 0)return NOTMATCH; // key doesn't match buffer so no match
  if(lenbuffer>lenkey&&!isspace((unsigned char)buffer[lenkey]))return NOTMATCH;
  return MATCH;
}

/* ------------------ MatchUpper ------------------------ */

int MatchUpper(char *buffer, const char *key){
  size_t lenbuffer;
  size_t lenkey;
  size_t i;

  lenkey=strlen(key);
  TrimBack(buffer);
  lenbuffer=strlen(buffer);

  if(lenbuffer<lenkey)return NOTMATCH;
  for(i=0;i<lenkey;i++){
    if(toupper(buffer[i])!=toupper(key[i]))return NOTMATCH;
  }
  if(lenbuffer>lenkey&&!isspace((unsigned char)buffer[lenkey]))return NOTMATCH;
  return MATCH;
}

/* ----------------------- MatchWild ----------------------------- */

int MatchWild(char *pTameText, char *pWildText){
// This function compares text strings, the second of which can have wildcards ('*').
//
//Matching Wildcards: An Algorithm
//by Kirk J. Krauss
// http://drdobbs.com/windows/210200888
// (modified from original by setting bCaseSensitive and cAltTerminator in the
//  body of the routine and changing routine name to MatchWild, also changed
//  formatting to be consistent with smokeview coding style)

  char cAltTerminator='\0';
#ifdef WIN32
  int bCaseSensitive=0;
#else
  int bCaseSensitive=1;
#endif
  int bMatch = 1;
  char *pAfterLastWild = NULL; // The location after the last '*', if we've encountered one
  char *pAfterLastTame = NULL; // The location in the tame string, from which we started after last wildcard
  char t, w;

  if(*pWildText==cAltTerminator)return 1;

        // Walk the text strings one character at a time.
  for(;;){
    t = *pTameText;
    w = *pWildText;

    if(!t || t == cAltTerminator){
      if(!w || w == cAltTerminator)break;    // "x" matches "x"
      else if(w == '*'){
        pWildText++;
        continue;                             // "x*" matches "x" or "xy"
      }
      else if(pAfterLastTame){
        if(!(*pAfterLastTame) || *pAfterLastTame == cAltTerminator){
          bMatch = 0;
          break;
        }
        pTameText = pAfterLastTame++;
        pWildText = pAfterLastWild;
        continue;
      }
      bMatch = 0;
      break;                                  // "x" doesn't match "xy"
    }
    else{
      if(!bCaseSensitive){
  //   convert characters to lowercase
        if(t >= 'A' && t <= 'Z')t += ('a' - 'A');
        if(w >= 'A' && w <= 'Z')w += ('a' - 'A');
      }
      if(t != w){
        if(w == '*'){
          pAfterLastWild = ++pWildText;
          pAfterLastTame = pTameText;
          continue;                           // "*y" matches "xy"
        }
        else if(pAfterLastWild){
          pWildText = pAfterLastWild;
          w = *pWildText;
          if(!w || w == cAltTerminator)break;// "*" matches "x"
          else{
            if(!bCaseSensitive && w >= 'A' && w <= 'Z')w += ('a' - 'A');
            if(t == w)pWildText++;
          }
          pTameText++;
          continue;                           // "*sip*" matches "mississippi"
        }
        else{
          bMatch = 0;
          break;                              // "x" doesn't match "y"
        }
      }
    }
    pTameText++;
    pWildText++;
  }
  return bMatch;
}

/* ----------------------- RemoveComment ----------------------------- */

char *RemoveComment(char *buffer){
  char *comment;

  comment = strstr(buffer,"//");
  if(comment!=NULL)comment[0]=0;
  TrimBack(buffer);
  return TrimFront(buffer);
}

/* ------------------ GetProgVersion ------------------------ */

void GetProgVersion(char *PROGversion){
  strcpy(PROGversion,PROGVERSION);
}

/* ------------------ SetLabelsIso ------------------------ */

int SetLabelsIso(flowlabels *flowlabel, char *longlabel, char *shortlabel, char *unit, float *levels, int nlevels){
  char buffer[255];
  size_t len;
  char clevels[1024];

  Array2String(levels,nlevels,clevels);
  if(longlabel==NULL){
    strcpy(buffer,"*");
  }
  else{
    strcpy(buffer,longlabel);
    strcat(buffer,": ");
    strcat(buffer,clevels);
  }
  len=strlen(buffer);
  if(NewMemory((void **)&flowlabel->longlabel,(unsigned int)(len+1))==0)return LABEL_ERR;
  STRCPY(flowlabel->longlabel,buffer);

  if(shortlabel==NULL){
    strcpy(buffer,"*");
  }
  else{
    strcpy(buffer,shortlabel);
  }
  len=strlen(buffer);
  if(NewMemory((void **)&flowlabel->shortlabel,(unsigned int)(len+1))==0)return LABEL_ERR;
  STRCPY(flowlabel->shortlabel,buffer);

  if(unit==NULL){
    strcpy(buffer,"*");
  }
  else{
    strcpy(buffer,unit);
  }
  len=strlen(buffer);
  if(NewMemory((void **)&flowlabel->unit,(unsigned int)(len+1))==0)return LABEL_ERR;
  STRCPY(flowlabel->unit,buffer);

  return LABEL_OK;
}

/* ------------------ SetLabels ------------------------ */

int SetLabels(flowlabels *flowlabel, char *longlabel, char *shortlabel, char *unit){
  char buffer[255];
  size_t len;

  if(longlabel==NULL){
    strcpy(buffer,"*");
  }
  else{
    strcpy(buffer,longlabel);
  }
  len=strlen(buffer);
  if(NewMemory((void **)&flowlabel->longlabel,(unsigned int)(len+1))==0)return LABEL_ERR;
  STRCPY(flowlabel->longlabel,buffer);

  if(shortlabel==NULL){
    strcpy(buffer,"*");
  }
  else{
    strcpy(buffer,shortlabel);
  }
  len=strlen(buffer);
  if(NewMemory((void **)&flowlabel->shortlabel,(unsigned int)(len+1))==0)return LABEL_ERR;
  STRCPY(flowlabel->shortlabel,buffer);

  if(unit==NULL){
    strcpy(buffer,"*");
  }
  else{
    strcpy(buffer,unit);
  }
  len=strlen(buffer);
  if(NewMemory((void **)&flowlabel->unit,(unsigned int)(len+1))==0)return LABEL_ERR;
  STRCPY(flowlabel->unit,buffer);

  return LABEL_OK;
}

/* ------------------ AppendLabels ------------------------ */

int AppendLabels(flowlabels *flowlabel, char *suffix_label){
  size_t newlen;
  char *longlabel;

  if(flowlabel==NULL || flowlabel->longlabel==NULL)return LABEL_OK;
  if(suffix_label == NULL || strlen(suffix_label)==0)return LABEL_OK;

  newlen = strlen(flowlabel->longlabel) + strlen(suffix_label);
  if(NewMemory((void **)&longlabel, (unsigned int)(newlen+1)) == 0)return LABEL_ERR;
  STRCPY(longlabel,flowlabel->longlabel);
  STRCAT(longlabel, suffix_label);
  FREEMEMORY(flowlabel->longlabel);
  flowlabel->longlabel=longlabel;

  return LABEL_OK;
}

/* ------------------ ReadLabelsBNDS ------------------------ */

int ReadLabelsBNDS(flowlabels *flowlabel, BFILE *stream, char *bufferD, char *bufferE, char *bufferF, char *suffix_label){
  char buffer2[255], *buffer;
  size_t len;
  int len_suffix_label = 0;

  if(stream != NULL){
    if(FGETS(buffer2, 255, stream) == NULL){
      strcpy(buffer2, "*");
    }
    strcpy(bufferD, buffer2);
  }
  else{
    strcpy(buffer2, bufferD);
  }

  len = strlen(buffer2);
  buffer = TrimFront(buffer2);
  TrimBack(buffer);
  len = strlen(buffer);
  if(suffix_label != NULL)len_suffix_label = strlen(suffix_label);
  if(NewMemory((void **)&flowlabel->longlabel, (unsigned int)(len + len_suffix_label + 1)) == 0)return LABEL_ERR;
  STRCPY(flowlabel->longlabel, buffer);
  if(suffix_label != NULL&&strlen(suffix_label) > 0)STRCAT(flowlabel->longlabel, suffix_label);

  if(stream != NULL){
    if(FGETS(buffer2, 255, stream) == NULL){
      strcpy(buffer2, "**");
    }
    strcpy(bufferE, buffer2);
  }
  else{
    strcpy(buffer2, bufferE);
  }

  len = strlen(buffer2);
  buffer = TrimFront(buffer2);
  TrimBack(buffer);
  len = strlen(buffer);
  if(NewMemory((void **)&flowlabel->shortlabel, (unsigned int)(len + 1)) == 0)return LABEL_ERR;
  STRCPY(flowlabel->shortlabel, buffer);

  if(stream != NULL){
    if(FGETS(buffer2, 255, stream) == NULL){
      strcpy(buffer2, "***");
    }
    strcpy(bufferF, buffer2);
  }
  else{
    strcpy(buffer2, bufferF);
  }

  len = strlen(buffer2);
  buffer = TrimFront(buffer2);
  TrimBack(buffer);
  len = strlen(buffer) + 1;// allow room for deg C symbol in case it is present
  if(NewMemory((void *)&flowlabel->unit, (unsigned int)(len + 1)) == 0)return LABEL_ERR;
#ifdef pp_DEG
  if(strlen(buffer) == 1 && strcmp(buffer, "C") == 0){
    unsigned char *unit;

    unit = (unsigned char *)flowlabel->unit;
    unit[0] = DEG_SYMBOL;
    unit[1] = 'C';
    unit[2] = '\0';
  }
  else{
    STRCPY(flowlabel->unit, buffer);
  }
#else
  STRCPY(flowlabel->unit, buffer);
#endif
  return LABEL_OK;
}

/* ------------------ ReadLabels ------------------------ */

int ReadLabels(flowlabels *flowlabel, BFILE *stream, char *suffix_label){
  char buffer2[255], *buffer;
  size_t len;
  int len_suffix_label = 0;
  int len_skip_label = 10;  // add extra space to label in case there is an isosurface skip parameter

  if(FGETS(buffer2,255,stream)==NULL){
    strcpy(buffer2,"*");
  }

  len=strlen(buffer2);
  buffer=TrimFront(buffer2);
  TrimBack(buffer);
  len=strlen(buffer);
  if(suffix_label!=NULL)len_suffix_label = strlen(suffix_label);
  if(NewMemory((void **)&flowlabel->longlabel,(unsigned int)(len+len_suffix_label+len_skip_label+1))==0)return LABEL_ERR;
  STRCPY(flowlabel->longlabel,buffer);
  if(suffix_label!=NULL&&strlen(suffix_label)>0)STRCAT(flowlabel->longlabel, suffix_label);

  if(FGETS(buffer2,255,stream)==NULL){
    strcpy(buffer2,"**");
  }

  len=strlen(buffer2);
  buffer=TrimFront(buffer2);
  TrimBack(buffer);
  len=strlen(buffer);
  if(NewMemory((void **)&flowlabel->shortlabel,(unsigned int)(len+1))==0)return LABEL_ERR;
  STRCPY(flowlabel->shortlabel,buffer);

  if(FGETS(buffer2,255,stream)==NULL){
    strcpy(buffer2,"***");
  }

  len=strlen(buffer2);
  buffer=TrimFront(buffer2);
  TrimBack(buffer);
  len=strlen(buffer)+1;// allow room for deg C symbol in case it is present
  if(NewMemory((void *)&flowlabel->unit,(unsigned int)(len+1))==0)return LABEL_ERR;
#ifdef pp_DEG
  if(strlen(buffer)==1&&strcmp(buffer,"C")==0){
    unsigned char *unit;

    unit=(unsigned char *)flowlabel->unit;
    unit[0]=DEG_SYMBOL;
    unit[1]='C';
    unit[2]='\0';
  }
  else{
    STRCPY(flowlabel->unit,buffer);
  }
#else
  STRCPY(flowlabel->unit,buffer);
#endif
  return LABEL_OK;
}

/* ------------------ Date2Day ------------------------ */

unsigned int Date2Day(char *tokenorig){
  // mm/dd/yyyy -> days after 1/1/2000
  // (/yyyy optional, if absent assume current year)
  char token[256];
  char *slash1=NULL, *slash2=NULL;
  char *month, *day, *year=NULL;
  int imonth, iday, iyear;
  int month2days[]={0,31,59,90,120,151,181,212,243,273,304,334};
  int days_local;
  int ileap;

  strcpy(token,tokenorig);
  slash1=strchr(token,'/');
  if(slash1==NULL)return 0;

  slash2 = strchr(slash1+1, '/');
  if(slash2==NULL){
    char *dayend;

    year=NULL;
    month=token;
    day=slash1+1;
    dayend=strchr(day,' ');
    if(dayend!=NULL)*dayend=0;
    *slash1=0;
  }
  else{
    char *yearend;

    month=token;
    day=slash1+1;
    year=slash2+1;
    yearend=strchr(year,' ');
    *slash1=0;
    *slash2=0;
    if(yearend!=NULL)*yearend=0;
  }
  days_local=0;
  iyear=0;
  if(year!=NULL)iyear = atoi(year)-2000;
  imonth = atoi(month);
  iday = atoi(day);
  ileap = iyear/4 + 1;
  if(iyear%4==0&&imonth<3)ileap--;
  days_local += iyear*365;
  days_local += month2days[imonth-1];
  days_local += iday - 1 +ileap;
  return days_local;
}

/* ------------------ ToLower ------------------------ */

void ToLower(char *string){
   char *c;

   if(string==NULL)return;
   for(c=string;*c!=0;c++){
     if(*c>='A'&&*c<='Z')*c+='a'-'A';
   }
}

/* ------------------ Time2Sec ------------------------ */

unsigned int Time2Sec(char *tokenorig){
// hh:mm:ss --> seconds after midnight
//  (:ss optional)
  char token[256];
  char *colon1, *colon2;
  char *hour=NULL,*min=NULL,*sec=NULL;
  char *minend, *secend;
  int time_local;

  strcpy(token,tokenorig);
  colon1=strchr(token,':');
  if(colon1==NULL){
    return 0;
  }
  else{
    colon2=strchr(colon1+1,':');
  }
  hour=token;
  *colon1=0;
  min=colon1+1;
  if(colon2==NULL){
    minend=strchr(min,' ');
    if(minend!=NULL)*minend=0;
  }
  else{
    *colon2=0;
    sec=colon2+1;
    secend=strchr(sec,' ');
    if(secend!=NULL)*secend=0;
  }
  time_local=0;
  if(strlen(hour)>0)time_local+=3600*atoi(hour);
  if(min!=NULL&&strlen(min)>0)time_local+=60*atoi(min);
  if(sec!=NULL&&strlen(sec)>0)time_local+=atoi(sec);
  return time_local;
}

/* ------------------ STRCHRR ------------------------ */

char *STRCHRR(char *strbeg, char *searchbeg, int c){
  char *cc;

  if(searchbeg>strbeg){
    for(cc=searchbeg;cc>=strbeg;cc--){
      if(*cc==c||*cc==0)return cc+1;
    }
    return strbeg;
  }
  else{
    for(cc=searchbeg;cc<=strbeg;cc++){
      if(*cc==c||*cc==0)return cc-1;
    }
    return strbeg;
  }
}

/* ------------------ Date2Sec2 ------------------------ */

unsigned int Date2Sec2(char *tokenorig){
  char token[256];
  char *colon;
  char *tim=NULL,*timend=NULL;
  int secs=0;
  int local_time;

  strcpy(token,tokenorig);
  colon=strchr(token,':');
  if(colon!=NULL)tim=STRCHRR(token,colon,' ');

  if(tim!=NULL){
    timend=strchr(tim,' ');
    if(timend!=NULL)*timend=0;
    secs=Time2Sec(tim);
  }
  local_time=secs;
  return local_time;
}

/* ------------------ Date2Sec ------------------------ */

unsigned int Date2Sec(char *tokenorig){
  char token[256];
  char *slash, *colon;
  char *date=NULL,*dateend=NULL;
  char *tim=NULL,*timend=NULL;
  int days=0, secs=0;
  int local_time;

  strcpy(token,tokenorig);
  slash=strchr(token,'/');
  colon=strchr(token,':');
  if(slash!=NULL)date=STRCHRR(token,slash,' ');
  if(colon!=NULL)tim=STRCHRR(token,colon,' ');

  if(date!=NULL){
    dateend=strchr(slash,' ');
    if(dateend!=NULL)*dateend=0;
    days=Date2Day(date);
  }
  if(tim!=NULL){
    timend=strchr(tim,' ');
    if(timend!=NULL)*timend=0;
    secs=Time2Sec(tim);
  }
  local_time=86400*days+secs;
  return local_time;
}

/* ------------------ DiffDate ------------------------ */

unsigned int DiffDate(char *token, char *tokenbase){
  int difft;

  difft = Date2Sec(token) - Date2Sec(tokenbase);
  return difft;
}

#ifdef pp_BETA
  #define SET_VERSIONTITLE
#else
#ifndef pp_OFFICIAL_RELEASE
  #define SET_VERSIONTITLE
#endif
#endif

/* ------------------ GetBaseTitle ------------------------ */

void GetBaseTitle(char *progname, char *title_base){
  char version[100];
  char git_version[100];
  char git_date[100];

  GetGitInfo(git_version, git_date);    // get githash

  // construct string of the form:
  //   5.x.y_#

  GetProgVersion(version);

  strcpy(title_base, progname);

  strcat(title_base, " ");
  strcat(title_base, version);

#ifdef SET_VERSIONTITLE
  if(strcmp(version,"")!=0)strcat(title_base, "(");
  strcat(title_base, git_version);
  if(strcmp(version,"")!=0)strcat(title_base, ")");
#endif
  strcat(title_base, " - ");
}

/* ------------------ GetTitle ------------------------ */

void GetTitle(char *progname, char *fulltitle){
  char title_base[1024];

  GetBaseTitle(progname, title_base);

  STRCPY(fulltitle, title_base);
  STRCAT(fulltitle, __DATE__);
#ifdef pp_BETA
  STRCAT(fulltitle, " - ");
  STRCAT(fulltitle, __TIME__);
#endif
}

#ifdef pp_HASH
#define HASH_BUFFER_LEN 1
#define HASH_MD5_LEN   16
#define HASH_SHA1_LEN   20
#define HASH_SHA256_LEN   32

/* ------------------ GeHashSHA1 ------------------------ */

unsigned char *GetHashSHA1(char *file){
  FILE *stream = NULL;

  if(file==NULL)return NULL;
  stream = fopen(file, "rb");
  if(stream==NULL){
    char *pathentry, fullpath[1024];

    pathentry = Which(file);
    strcpy(fullpath, pathentry);
    strcat(fullpath, file);
#ifdef WIN32
    {
      const char *ext;

      ext = fullpath+strlen(fullpath)-4;
      if(strlen(fullpath)<=4||STRCMP(ext, ".exe")!=0)strcat(fullpath, ".exe");
    }
#endif

    stream = fopen(fullpath, "rb");
    if(stream==NULL)return NULL;
  }

  {
    int i;
    unsigned char hash[HASH_SHA1_LEN], *return_hash;
    mbedtls_sha1_context ctx;
    unsigned char data[HASH_BUFFER_LEN];
    size_t len_data;

    mbedtls_sha1_init(&ctx);
    mbedtls_sha1_starts(&ctx);
    while((len_data = fread(data, 1, HASH_BUFFER_LEN, stream))!=0){
      mbedtls_sha1_update(&ctx, data, len_data);
    }

    mbedtls_sha1_finish(&ctx, hash);
    fclose(stream);

    NewMemory((void **)&return_hash, 2 * HASH_SHA1_LEN + 1);

    for(i = 0; i < HASH_SHA1_LEN; i++){
      sprintf((char *)return_hash + 2 * i, "%02x", hash[i]);
    }
    return_hash[2 * HASH_SHA1_LEN] = 0;
    return return_hash;
  }
}

/* ------------------ GeHashMD5 ------------------------ */

unsigned char *GetHashMD5(char *file){
  mbedtls_md5_context ctx;
  FILE *stream=NULL;
  unsigned char data[HASH_BUFFER_LEN];
  unsigned char hash[HASH_MD5_LEN], *return_hash;
  int i;
  size_t len_data;

  if(file==NULL)return NULL;
  stream = fopen(file, "rb");
  if(stream == NULL){
    char *pathentry, fullpath[1024];

    pathentry = Which(file);
    strcpy(fullpath, pathentry);
    strcat(fullpath, file);
#ifdef WIN32
    {
      const char *ext;

      ext = fullpath + strlen(fullpath) - 4;
      if(strlen(fullpath) <= 4 || STRCMP(ext, ".exe") != 0)strcat(fullpath, ".exe");
    }
#endif

    stream = fopen(fullpath, "rb");
    if(stream == NULL)return NULL;
  }

  mbedtls_md5_init(&ctx);
  mbedtls_md5_starts(&ctx);
  while((len_data = fread(data, 1, HASH_BUFFER_LEN, stream)) != 0){
    mbedtls_md5_update(&ctx, data, len_data);
  }
  mbedtls_md5_finish(&ctx, hash);
  fclose(stream);

  NewMemory((void **)&return_hash, 2*HASH_MD5_LEN + 1);

  for(i = 0; i<HASH_MD5_LEN; i++){
    sprintf((char *)return_hash+2*i, "%02x", hash[i]);
  }
  return_hash[2*HASH_MD5_LEN] = 0;
  return return_hash;
}

/* ------------------ GetHashSHA256 ------------------------ */

unsigned char *GetHashSHA256(char *file){
  mbedtls_sha256_context ctx;
  FILE *stream = NULL;
  unsigned char data[HASH_BUFFER_LEN];
  unsigned char hash[HASH_SHA256_LEN], *return_hash;
  int i;
  size_t len_data;

  if(file==NULL)return NULL;
  stream = fopen(file, "rb");
  if(stream==NULL){
    char *pathentry, fullpath[1024];

    pathentry = Which(file);
    strcpy(fullpath, pathentry);
    strcat(fullpath, file);
#ifdef WIN32
    {
      const char *ext;

      ext = fullpath+strlen(fullpath)-4;
      if(strlen(fullpath)<=4||STRCMP(ext, ".exe")!=0)strcat(fullpath, ".exe");
    }
#endif

    stream = fopen(fullpath, "rb");
    if(stream==NULL)return NULL;
  }

  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts(&ctx,0);
  while((len_data = fread(data, 1, HASH_BUFFER_LEN, stream))!=0){
    mbedtls_sha256_update(&ctx, data, len_data);
  }
  mbedtls_sha256_finish(&ctx, hash);
  fclose(stream);

  NewMemory((void **)&return_hash, 2*HASH_SHA256_LEN+1);

  for(i = 0; i<HASH_SHA256_LEN; i++){
    sprintf((char *)return_hash+2*i, "%02x", hash[i]);
  }
  return_hash[2*HASH_SHA256_LEN] = 0;
  return return_hash;
}
#endif

/* ------------------ UsageCommon ------------------------ */

void UsageCommon(int option){
  if(option == HELP_SUMMARY){
    PRINTF("  -help      - display help summary\n");
    PRINTF("  -help_all  - display all help info\n");
    PRINTF("  -version   - display version information\n");
  }
#ifdef pp_HASH
  if(option == HELP_ALL){
    PRINTF("  -md5       - display an md5 hash when -version is invoked\n");
    PRINTF("  -sha1      - display a sha1 hash when -version is invoked\n");
    PRINTF("  -sha256    - display a sha256 hash when -version is invoked\n");
    PRINTF("  -hash_all  - display all hashes when -version option is invoked\n");
    PRINTF("  -hash_none - do not display any hashes  when -version is invoked\n");
  }
#endif
}

/* ------------------ ParseCommonOptions ------------------------ */

int ParseCommonOptions(int argc, char **argv){
  int i, no_minus,first_arg=0;

  no_minus = 0;
  for(i = 1; i<argc; i++){
    char *argi;

    argi = argv[i];
    if(argi==NULL||argi[0]!='-'){
      if(first_arg==0){
        first_arg = i;
        return first_arg;
      }
      no_minus = 1;
      continue;
    }
    if(STRCMP("-help", argi)==0||(STRCMP("-h", argi)==0&&STRCMP("-help_all",argi)!=0)){
      show_help = 1;
      continue;
    }
    if(STRCMP("-help_all", argi) == 0){
      show_help = 2;
      continue;
    }
    if(STRCMP("-version", argi)==0||STRCMP("-v", argi)==0){
      if(no_minus==0)show_version = 1;
      continue;
    }
#ifdef pp_HASH
    if(STRCMP("-sha256", argi)==0){
      hash_option = HASH_SHA256;
      continue;
    }
    if(STRCMP("-sha1", argi)==0){
      hash_option = HASH_SHA1;
      continue;
    }
    if(STRCMP("-md5", argi)==0){
      hash_option = HASH_MD5;
      continue;
    }
    if(STRCMP("-hash_all", argi)==0){
      hash_option = HASH_ALL;
      continue;
    }
    if(STRCMP("-hash_none", argi)==0){
      hash_option = HASH_NONE;
      continue;
    }
#endif
  }
  return first_arg;
}

/* ------------------ version ------------------------ */

#ifdef pp_HASH
void PRINTversion(char *progname, char *progfullpath, int option){
#else
void PRINTversion(char *progname){
#endif
  char version[256];
  char githash[256];
  char gitdate[256];
  char releasetitle[1024];

  GetProgVersion(version);
  GetGitInfo(githash, gitdate);    // get githash
  GetTitle(progname, releasetitle);

  PRINTF("\n");
  PRINTF("%s\n\n", releasetitle);
  if(strcmp(version, "") != 0){
    PRINTF("Version          : %s\n", version);
  }
  PRINTF("Revision         : %s\n", githash);
  PRINTF("Revision Date    : %s\n", gitdate);
  PRINTF("Compilation Date : %s %s\n", __DATE__, __TIME__);
#ifndef pp_COMPVER
#define pp_COMPVER "unknown"
#endif
  PRINTF("Compiler         : %s\n", pp_COMPVER);

#ifdef pp_HASH
  if(option==HASH_MD5||option==HASH_ALL){
    unsigned char *hash = NULL;

    hash = GetHashMD5(progfullpath);
    if(hash!=NULL)PRINTF("Checksum(MD5)    : %s\n", hash);
    FREEMEMORY(hash);
  }
  if(option==HASH_SHA1||option==HASH_ALL){
    unsigned char *hash = NULL;

    hash = GetHashSHA1(progfullpath);
    if(hash!=NULL)PRINTF("Checksum(SHA1)   : %s\n", hash);
    FREEMEMORY(hash);
  }
  if(option==HASH_SHA256||option==HASH_ALL){
    unsigned char *hash = NULL;

    hash = GetHashSHA256(progfullpath);
    if(hash!=NULL)PRINTF("Checksum(SHA256) : %s\n", hash);
    FREEMEMORY(hash);
  }
#endif
#ifdef WIN32
  PRINTF("Platform         : WIN64 ");
#ifdef pp_INTEL
  PRINTF(" (Intel C/C++)");
#else
  PRINTF(" (MSVS C/C++)");
#endif
  PRINTF("\n");
#endif
#ifdef pp_OSX
  PRINTF("Platform         : OSX64\n");
#endif
#ifdef pp_LINUX
  PRINTF("Platform         : LINUX64\n");
#endif
}
