#include "options.h"
#define IN_TRANSLATE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dmalloc.h"
#include "string_util.h"
#include "translate.h"

/* ------------------ CompareTrdata ------------------------ */

int CompareTrdata(const void *arg1, const void *arg2){
  trdata *tri, *trj;

  tri = (trdata *)arg1;
  trj = (trdata *)arg2;

  if(tri->key == NULL&&trj->key!=NULL)return -1;
  if(tri->key != NULL&&trj->key == NULL)return 1;
  if(tri->key == NULL&&trj->key == NULL)return 0;
  return STRCMP(tri->key,trj->key);
}

/* ------------------ ParseLang ------------------------ */

int ParseLang(char *file, trdata **trinfoptr, int *ntrinfoptr){

//  read a po file and put english/foreign language string pairs into
//  the trinfo data structure

  FILE *stream;
  trdata *trinfo_local;
  int ntrinfo_local;

  ntrinfo_local=*ntrinfoptr;
  trinfo_local=*trinfoptr;
  if(ntrinfo_local>0){
    int i;

    for(i=0;i<ntrinfo_local;i++){
      trdata *tri;

      tri = trinfo_local + i;
      FREEMEMORY(tri->key);
      FREEMEMORY(tri->value);
    }
    FREEMEMORY(trinfo_local);
  }

  ntrinfo_local=0;
  if(file==NULL)return 0;
  stream=FOPEN(file,"r");
  if(stream==NULL)return 0;

  while(!feof(stream)){
    char buffer[1000];
    char *buf;
    char *key;

    if(fgets(buffer,1000,stream)==NULL)break;
    buf=TrimFront(buffer);
    TrimBack(buf);
    if(strlen(buf)>=2&&strncmp(buf,"//",2)==0)continue;
    key = strstr(buf,"msgid");
    if(key!=NULL&&key==buf)ntrinfo_local++;
  }
  if(ntrinfo_local==0){
    fclose(stream);
    return 0;
  }

  NewMemory((void **)&trinfo_local,sizeof(trdata)*ntrinfo_local);
  *trinfoptr=trinfo_local;
  *ntrinfoptr=ntrinfo_local;

  ntrinfo_local=0;
  rewind(stream);
  while(!feof(stream)){
    char buffer[1000];
    trdata *tri;
    char *keybuf,*valbuf;
    char *buf;
    char *key, *value;
    int doit;

    if(fgets(buffer,1000,stream)==NULL)break;
    buf=TrimFront(buffer);
    TrimBack(buf);
    if(strlen(buf)>=2&&strncmp(buf,"//",2)==0)continue;
    key = strstr(buf,"msgid");
    if(key==NULL||key!=buf)continue;
    ntrinfo_local++;
    tri = trinfo_local + ntrinfo_local - 1;
    key = GetString(key+5);
    if(key==NULL){
      tri->key=NULL;
    }
    else{
      NewMemory((void **)&keybuf,strlen(key)+1);
      strcpy(keybuf,key);
      tri->key=keybuf;
    }

    for(doit=1;doit==1;){
      doit=0;
      if(fgets(buffer,1000,stream)==NULL)break;
      buf=TrimFront(buffer);
      TrimBack(buf);
      if(strlen(buf)>=2&&strncmp(buf,"//",2)==0)doit=1;
    }
    value = GetString(buf+6);
    if(value==NULL){
      tri->value=value;
    }
    else{
      NewMemory((void **)&valbuf,strlen(value)+1);
      strcpy(valbuf,value);
      tri->value=valbuf;
    }
  }
  fclose(stream);

  qsort(trinfo_local,ntrinfo_local,sizeof(trdata),CompareTrdata);
  return 1;
}

/* ------------------ InitTranslate ------------------------ */

void InitTranslate(char *bindir, char *tr_name){

//  initialize po language translation data structures

  char *LANG;

  if(tr_name!=NULL){
    LANG=tr_name;
  }
  else{
    LANG=getenv("LANG");
  }

  tr_otherlang=0;
  if(LANG!=NULL&&strncmp(LANG,"en",2)!=0){
    FILE *stream;
    int lensmokebindir;
    char lang[256];

    lang[0]=tolower(LANG[0]);
    lang[1]=tolower(LANG[1]);
    lang[2]=0;
    lensmokebindir=strlen(bindir);
    FREEMEMORY(smokeview_lang);
    NewMemory((void **)&smokeview_lang,(unsigned int)(lensmokebindir+15+1));
    STRCPY(smokeview_lang,bindir);
    STRCAT(smokeview_lang,"smokeview_");
    STRCAT(smokeview_lang,lang);
    STRCAT(smokeview_lang,".po");

    stream=FOPEN(smokeview_lang,"r");
    if(stream!=NULL){
      fclose(stream);
      tr_otherlang=1;
    }
    tr_otherlang=ParseLang(smokeview_lang,&trinfo,&ntrinfo);
    if(tr_otherlang==1){
      PRINTF("Using translation file: %s",smokeview_lang);
      PRINTF("\n");
    }
    else{
      fprintf(stderr,"*** Warning: Failed to parse translation file: %s",smokeview_lang);
      fprintf(stderr,"\n");
      fprintf(stderr,"%s\n","Menus will be in English");
    }
  }
}

/* ------------------ Translate ------------------------ */

char *Translate(char *string){
  int i, len;
  char *string_before, *string_in, *string_out, *string_after;

  if(tr_otherlang==0)return string;

  len = strlen(string);

  // find leading non-alpha characters

  string_before = NULL;
  string_in = string;
  for(i = 0; i<len; i++){
    char c;

    c=string[i];
    if((c >= 'a'&&c <= 'z') || (c >= 'A'&&c <= 'Z') || (c >= '0'&&c <= '9')){
      string_in = string + i;
      if(string_before != NULL)string_before[i] = 0;
      break;
    }
    if(string_before==NULL)string_before = tr_string_before;
    string_before[i] = c;
  }

  // find trailing : or ...

  string_after = NULL;
  len = strlen(string_in);
  if(len > 0&&string_in[len-1]==':'){
    string_after = tr_string_after;
    strcpy(string_after,":");
    len--;
  }
  if(len > 3){
    char *match;

    match = strstr(string_in + len - 3, "...");
    if(match != NULL){
      string_after = tr_string_after;
      strcpy(string_after, "...");
      len -= 3;
    }
  }

  memcpy(tr_string_in,string_in,len);
  tr_string_in[len]=0;
  string_in=tr_string_in;

  // translate string_in

  {
    trdata *tr_out, tr_in;

    tr_in.key=string_in;

    tr_out = bsearch(&tr_in,trinfo,ntrinfo,sizeof(trdata),CompareTrdata);
    if(tr_out==NULL||tr_out->value==NULL)return string;
    string_out=tr_out->value;
  }

  strcpy(tr_string,"");
  if(string_before!=NULL&&strlen(string_before)>0)strcat(tr_string,string_before);
  strcat(tr_string,string_out);
  if(string_after!=NULL&&strlen(string_after)>0)strcat(tr_string,string_after);
  return tr_string;
}
