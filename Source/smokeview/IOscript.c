#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "IOvolsmoke.h"
#include "smokeviewdefs.h"
#include "IOscript.h"
#include "glui_motion.h"
#include "glui_smoke.h"
#include "glui_bounds.h"
#include "IOobjects.h"

static char param_buffer[1024];
static int param_status, line_number;

/* ------------------ GetNewScriptFileName ------------------------ */

void GetNewScriptFileName(char *newscriptfilename){
  char buffer[1024];
  int i;
  int nexti;
  scriptfiledata *scriptfile;

  for(i=0;i<1000;i++){
    if(i==0){
      strcpy(buffer,global_scase.fdsprefix);
      strcat(buffer,".ssf");
    }
    else{
      sprintf(buffer,"%s_%03i.ssf",global_scase.fdsprefix,i);
    }
    nexti=0;
    for(scriptfile=first_scriptfile.next;scriptfile->next!=NULL;scriptfile=scriptfile->next){
      if(strcmp(scriptfile->file,buffer)==0){
        nexti=1;
        break;
      }
    }
    if(nexti==0){
      strcpy(newscriptfilename,buffer);
      return;
    }
  }
  strcpy(newscriptfilename,"");
}

/* ------------------ GetScriptFileName ------------------------ */

char *GetScriptFileName(int id){
  scriptfiledata *scriptfile;

  for(scriptfile=first_scriptfile.next;scriptfile->next!=NULL;scriptfile=scriptfile->next){
    if(scriptfile->id==id)return scriptfile->file;
    if(scriptfile->file==NULL)continue;
  }
  return NULL;
}

/* ------------------ GetIniFileName ------------------------ */

char *GetIniFileName(int id){
  inifiledata *inifile;

  for(inifile=first_inifile.next;inifile->next!=NULL;inifile=inifile->next){
    if(inifile->id==id)return inifile->file;
    if(inifile->file==NULL)continue;
  }
  return NULL;
}

/* ------------------ InsertIniFile ------------------------ */

inifiledata *InsertIniFile(char *file){
  inifiledata *thisptr,*prevptr,*nextptr;
  int len;
  inifiledata *inifile;
  int idmax=-1;

  for(inifile=first_inifile.next;inifile->next!=NULL;inifile=inifile->next){
    if(inifile->id>idmax)idmax=inifile->id;
    if(inifile->file==NULL)continue;
    if(strcmp(file,inifile->file)==0)return NULL;
  }

  NewMemory((void **)&thisptr,sizeof(inifiledata));
  nextptr = &last_inifile;
  prevptr = nextptr->prev;
  nextptr->prev=thisptr;
  prevptr->next=thisptr;

  thisptr->next=nextptr;
  thisptr->prev=prevptr;
  thisptr->file=NULL;
  thisptr->id=idmax+1;

  if(file!=NULL){
    len = strlen(file);
    if(len>0){
      NewMemory((void **)&thisptr->file,len+1);
      strcpy(thisptr->file,file);
    }
  }
  return thisptr;
}

/* ------------------ InsertScriptFile ------------------------ */

scriptfiledata *InsertScriptFile(char *file){
  scriptfiledata *thisptr,*prevptr,*nextptr;
  int len;
  scriptfiledata *scriptfile;
  int idmax=-1;

  for(scriptfile=first_scriptfile.next;scriptfile->next!=NULL;scriptfile=scriptfile->next){
    if(scriptfile->id>idmax)idmax=scriptfile->id;
    if(scriptfile->file==NULL)continue;
    if(strcmp(file,scriptfile->file)==0)return scriptfile;
  }

  NewMemory((void **)&thisptr,sizeof(scriptfiledata));
  nextptr = &last_scriptfile;
  prevptr = nextptr->prev;
  nextptr->prev=thisptr;
  prevptr->next=thisptr;

  thisptr->next=nextptr;
  thisptr->prev=prevptr;
  thisptr->file=NULL;
  thisptr->recording=0;
  thisptr->id=idmax+1;

  if(file!=NULL){
    len = strlen(file);
    if(len>0){
      NewMemory((void **)&thisptr->file,len+1);
      strcpy(thisptr->file,file);
    }
  }
  return thisptr;
}

/* ------------------ StartScript ------------------------ */

void StartScript(void){
  if(scriptinfo==NULL){
    fprintf(stderr,"*** Error: Smokeview script does not exist\n");
    if(stderr2!=NULL)fprintf(stderr2,"*** Error: Smokeview script does not exist\n");
    return;
  }
  GLUIScriptDisable();
  current_script_command=scriptinfo-1;
  use_iso_threads_save = use_iso_threads;
  use_iso_threads = 0;
  viewpoint_script_ptr = NULL;
}

/* ------------------ GetCharPointer ------------------------ */

char *GetCharPointer(char *buffer_arg){
  char *cval=NULL, *buffptr;
  int len;

  RemoveComment(buffer_arg);
  buffptr = TrimFront(buffer_arg);
  len = strlen(buffptr);
  if(len>0){
    NewMemory((void **)&cval,len+1);
    strcpy(cval,buffptr);
  }
  return cval;
}

/* ------------------ FreeScript ------------------------ */

void FreeScript(void){
  scriptdata *scripti;
  int i;

  if(nscriptinfo>0){
    for(i=0;i<nscriptinfo;i++){
      scripti = scriptinfo + i;

      FREEMEMORY(scripti->cval);
      FREEMEMORY(scripti->cval2);
      FREEMEMORY(scripti->cval3);
    }
    FREEMEMORY(scriptinfo);
    nscriptinfo=0;
  }

}

/* ------------------ InitScriptI ------------------------ */

void InitScriptI(scriptdata *scripti, int command,char *label){
  char *label2;

  TrimBack(label);
  label2 = TrimFront(label);
  strcpy(scripti->command_label,label2);
  scripti->command       = command;
  scripti->cval          = NULL;
  scripti->cval2         = NULL;
  scripti->cval3         = NULL;
  scripti->fval          = 0.0;
  scripti->ival          = 0;
  scripti->ival2         = 0;
  scripti->ival3         = 0;
  scripti->ival4         = 0;
  scripti->ival5         = 0;
  scripti->need_graphics = 1;

  scripti->id            = NULL;
  scripti->quantity      = NULL;
  scripti->c_pbxyz       = NULL;
  scripti->pbxyz_val     = 0.0;
  scripti->pbxyz_dir     = 0;
  scripti->cell_centered = 0;
  scripti->vector        = 0;
  strcpy(scripti->quantity2, "");
}

/* ------------------ GetParamBuffer ------------------------ */

int GetParamBuffer(FILE *stream){
  for(;;){
    char *comment;

    if(fgets(param_buffer, 1024, stream)==NULL)return SCRIPT_EOF;
    line_number++;
    comment = strstr(param_buffer, "//");
    if(comment==NULL)comment = strstr(param_buffer, "#");
    if(comment!=NULL)comment[0]=0;
    TrimBack(param_buffer);
    if(strlen(param_buffer)==0)continue;
    break;
  }
  return SCRIPT_OK;
}

/* ------------------ InitKeywords ------------------------ */

void InitKeyword(char *keyword, int index, int nparms){
  keyworddata *kwi;

  kwi = keywordinfo + nkeywordinfo;
  strcpy(kwi->keyword, keyword);
  kwi->index       = index;
  kwi->nparams     = nparms;
  kwi->line_number = 0;
  nkeywordinfo++;
}

/* ------------------ InitKeywords ------------------------ */

void InitKeywords(void){
  if(keywordinfo!=NULL)return;  // only define once
  NewMemory((void **)&keywordinfo, 1000*sizeof(keyworddata));
  nkeywordinfo++;

  InitKeyword("dummy", -999, 0);         // dummy entry used to report errors
// 3d smoke
  InitKeyword("LOAD3DSMOKE",         SCRIPT_LOAD3DSMOKE, 1);         // documented
  InitKeyword("LOADVOLSMOKE",        SCRIPT_LOADVOLSMOKE, 1);        // documented
  InitKeyword("LOADVOLSMOKEFRAME",   SCRIPT_LOADVOLSMOKEFRAME, 1);   // documented

// boundary files
  InitKeyword("LOADBOUNDARY",        SCRIPT_LOADBOUNDARY, 1);        // documented
  InitKeyword("LOADBOUNDARYM",       SCRIPT_LOADBOUNDARYM, 2);       // documented
  InitKeyword("SETBOUNDBOUNDS",      SCRIPT_SETBOUNDBOUNDS, 1);      // documented

// general files
  InitKeyword("LOADFILE",            SCRIPT_LOADFILE, 1);            // documented
  InitKeyword("LOADINIFILE",         SCRIPT_LOADINIFILE, 1);         // documented
  InitKeyword("UNLOADALL",           SCRIPT_UNLOADALL, 0);           // documented

// hvac files

  InitKeyword("HIDEHVACVALS",        SCRIPT_HIDEHVACVALS, 0);        // documented
  InitKeyword("SHOWHVACDUCTVAL",     SCRIPT_SHOWHVACDUCTVAL, 1);     // documented
  InitKeyword("SHOWHVACNODEVAL",     SCRIPT_SHOWHVACNODEVAL, 1);     // documented
  InitKeyword("LOADHVAC",            SCRIPT_LOADHVAC, 0);            // documented

// slice and vector slice files
  InitKeyword("LOADSLCF",            SCRIPT_LOADSLCF, 1);            // documented
  InitKeyword("LOADSLICE",           SCRIPT_LOADSLICE, 2);           // documented
  InitKeyword("LOADSLICEM",          SCRIPT_LOADSLICEM, 3);          // documented
  InitKeyword("LOADVFILE",           SCRIPT_LOADVFILE, 1);           // documented
  InitKeyword("LOADVSLICE",          SCRIPT_LOADVSLICE, 2);          // documented
  InitKeyword("LOADVSLICEM",         SCRIPT_LOADVSLICEM, 3);         // documented
  InitKeyword("SETSLICEBOUNDS",      SCRIPT_SETSLICEBOUNDS, 1);      // documented
  InitKeyword("SETSLICEAVERAGE",     SCRIPT_SETSLICEAVERAGE, 2);     // documented
  InitKeyword("OUTPUTSLICEDATA",     SCRIPT_OUTPUTSLICEDATA, 1);     // documented

// particle files
  InitKeyword("LOADPARTICLES",       SCRIPT_LOADPARTICLES, 0);       // documented
  InitKeyword("PARTCLASSCOLOR",      SCRIPT_PARTCLASSCOLOR, 1);      // documented
  InitKeyword("PARTCLASSTYPE",       SCRIPT_PARTCLASSTYPE, 1);       // documented

// plot3d files
  InitKeyword("LOADPLOT3D",          SCRIPT_LOADPLOT3D, 1);          // documented
  InitKeyword("PLOT3DPROPS",         SCRIPT_PLOT3DPROPS, 1);         // documented
  InitKeyword("SHOWPLOT3DDATA",      SCRIPT_SHOWPLOT3DDATA, 1);      // documented

// isosurface files
  InitKeyword("LOADISO",             SCRIPT_LOADISO, 1);             // documented
  InitKeyword("LOADISOM",            SCRIPT_LOADISOM, 2);            // documented

// show/hide devices
  InitKeyword("HIDEALLDEVS",         SCRIPT_HIDEALLDEVS, 0);         // documented
  InitKeyword("HIDEDEV",             SCRIPT_HIDEDEV, 1);             // documented
  InitKeyword("SHOWALLDEVS",         SCRIPT_SHOWALLDEVS, 0);         // documented
  InitKeyword("SHOWDEV",             SCRIPT_SHOWDEV, 1);             // documented
  InitKeyword("SHOWSMOKESENSORS",    SCRIPT_OUTPUTSMOKESENSORS, 0);  // documented
  InitKeyword("OUTPUTSMOKESENSORS",  SCRIPT_OUTPUTSMOKESENSORS, 0);  // documented

// colorbar
  InitKeyword("CBARNORMAL",          SCRIPT_CBARNORMAL, 0);          // documented
  InitKeyword("CBARFLIP",            SCRIPT_CBARFLIP, 0);            // documented
  InitKeyword("HIDECBAREDIT",        SCRIPT_HIDECBAREDIT, 0);        // documented
  InitKeyword("SHOWCBAREDIT",        SCRIPT_SHOWCBAREDIT, 0);        // documented
  InitKeyword("SETCBAR",             SCRIPT_SETCBAR, 1);             // documented
  InitKeyword("SETCBARLAB",          SCRIPT_SETCBARLAB, 0);          // documented
  InitKeyword("SETCBARRGB",          SCRIPT_SETCBARRGB, 0);          // documented
  InitKeyword("HILIGHTMINVALS",      SCRIPT_HILIGHTMINVALS, 4);      // documented
  InitKeyword("HILIGHTMAXVALS",      SCRIPT_HILIGHTMAXVALS, 4);      // documented

// tour
  InitKeyword("LOADTOUR",            SCRIPT_LOADTOUR, 1);            // documented
  InitKeyword("SETTOURKEYFRAME",     SCRIPT_SETTOURKEYFRAME, 1);     // documented
  InitKeyword("SETTOURVIEW",         SCRIPT_SETTOURVIEW, 1);         // documented
  InitKeyword("UNLOADTOUR",          SCRIPT_UNLOADTOUR, 0);          // documented

// controlling the scene
  InitKeyword("EXIT",                SCRIPT_EXIT, 0);                // documented
  InitKeyword("NOEXIT",              SCRIPT_NOEXIT, 0);              // documented
  InitKeyword("GSLICEORIEN",         SCRIPT_GSLICEORIEN, 1);         // documented
  InitKeyword("GSLICEPOS",           SCRIPT_GSLICEPOS, 1);           // documented
  InitKeyword("GSLICEVIEW",          SCRIPT_GSLICEVIEW, 1);          // documented
  InitKeyword("KEYBOARD",            SCRIPT_KEYBOARD, 1);            // documented
  InitKeyword("PROJECTION",          SCRIPT_PROJECTION, 1);          // documented
  InitKeyword("SCENECLIP",           SCRIPT_SCENECLIP, 1);           // documented
  InitKeyword("SETCLIPMODE",         SCRIPT_SETCLIPMODE, 1);         // documented
  InitKeyword("SETCLIPX",            SCRIPT_SETCLIPX, 1);            // documented
  InitKeyword("SETCLIPY",            SCRIPT_SETCLIPY, 1);            // documented
  InitKeyword("SETCLIPZ",            SCRIPT_SETCLIPZ, 1);            // documented
  InitKeyword("SETDEMOMODE",         SCRIPT_SETDEMOMODE, 1);
  InitKeyword("SMOKEPROP",           SCRIPT_SMOKEPROP, 1);
  InitKeyword("SETTIMEVAL",          SCRIPT_SETTIMEVAL, 1);          // documented
  InitKeyword("SETVIEWPOINT",        SCRIPT_SETVIEWPOINT, 1);        // documented
  InitKeyword("VIEWXMIN",            SCRIPT_VIEWXMIN, 0);            // documented
  InitKeyword("VIEWXMAX",            SCRIPT_VIEWXMAX, 0);            // documented
  InitKeyword("VIEWYMIN",            SCRIPT_VIEWYMIN, 0);            // documented
  InitKeyword("VIEWYMAX",            SCRIPT_VIEWYMAX, 0);            // documented
  InitKeyword("VIEWZMIN",            SCRIPT_VIEWZMIN, 0);            // documented
  InitKeyword("VIEWZMAX",            SCRIPT_VIEWZMAX, 0);            // documented
  InitKeyword("XYZVIEW",             SCRIPT_XYZVIEW, 1);             // documented
  InitKeyword("XSCENECLIP",          SCRIPT_XSCENECLIP, 1);          // documented
  InitKeyword("YSCENECLIP",          SCRIPT_YSCENECLIP, 1);          // documented
  InitKeyword("ZSCENECLIP",          SCRIPT_ZSCENECLIP, 1);          // documented

// rendering images

  InitKeyword("ISORENDERALL",        SCRIPT_ISORENDERALL, 2);        // documented
  InitKeyword("LOADSLICERENDER",     SCRIPT_LOADSLICERENDER, 4);     // documented
  InitKeyword("LOADSMOKERENDER",     SCRIPT_LOADSMOKERENDER, 3);     // documented
  InitKeyword("MAKEMOVIE",           SCRIPT_MAKEMOVIE, 3);           // documented
  InitKeyword("MOVIETYPE",           SCRIPT_MOVIETYPE, 1);           // documented
  InitKeyword("RENDER360ALL",        SCRIPT_RENDER360ALL, 2);        // documented
  InitKeyword("RENDERALL",           SCRIPT_RENDERALL, 2);           // documented
  InitKeyword("RENDERCLIP",          SCRIPT_RENDERCLIP, 1);          // documented
  InitKeyword("RENDERDIR",           SCRIPT_RENDERDIR, 1);           // documented
  InitKeyword("RENDERDOUBLEONCE",    SCRIPT_RENDERDOUBLEONCE, 1);    // documented
  InitKeyword("RENDERHTMLALL",       SCRIPT_RENDERHTMLALL, 1);       // documented
  InitKeyword("RENDERHTMLDIR",       SCRIPT_RENDERHTMLDIR, 1);       // documented
  InitKeyword("RENDERHTMLGEOM",      SCRIPT_RENDERHTMLGEOM, 1);
  InitKeyword("RENDERHTMLOBST",      SCRIPT_RENDERHTMLOBST, 1);
  InitKeyword("RENDERHTMLONCE",      SCRIPT_RENDERHTMLONCE, 1);      // documented
  InitKeyword("RENDERHTMLSLICECELL", SCRIPT_RENDERHTMLSLICECELL, 2);
  InitKeyword("RENDERHTMLSLICENODE", SCRIPT_RENDERHTMLSLICENODE, 2);
  InitKeyword("RENDERONCE",          SCRIPT_RENDERONCE, 1);          // documented
  InitKeyword("RENDERSIZE",          SCRIPT_RENDERSIZE, 1);          // documented
  InitKeyword("RENDERSTART",         SCRIPT_RENDERSTART, 1);         // documented
  InitKeyword("RENDERTYPE",          SCRIPT_RENDERTYPE, 1);          // documented
  InitKeyword("VOLSMOKERENDERALL",   SCRIPT_VOLSMOKERENDERALL, 2);   // documented

// miscellaneous

  InitKeyword("GPUOFF",              SCRIPT_GPUOFF, 0);              // documented
  InitKeyword("LABEL",               SCRIPT_LABEL, 1);               // documented
  InitKeyword("RGBTEST",             SCRIPT_RGBTEST, 1);             // documented
  InitKeyword("UNLOADPLOT2D",        SCRIPT_UNLOADPLOT2D, 0);

  ResizeMemory((void **)&keywordinfo, nkeywordinfo * sizeof(keyworddata));
}

/* ------------------ GetScriptKeywordFromLabel ------------------------ */

keyworddata *GetScriptKeywordFromLabel(char *keyword){
  int i;

  for(i = 1;i < nkeywordinfo;i++){
    keyworddata *kwi;

    kwi = keywordinfo + i;
    if(MatchSSF(kwi->keyword, keyword) == MATCH)return kwi;
  }
  return keywordinfo;
}

/* ------------------ GetWere ------------------------ */

char *GetWere(int n, char *were){
  if(n == 1){
    sprintf(were, "%i was", n);
  }
  else{
    sprintf(were, "%i were", n);
  }
  return were;
}

/* ------------------ GetScriptError ------------------------ */

int GetScriptError(keyworddata *kw, keyworddata *kw_last, int nparams){
  if(kw_last == NULL)return 0;
  if(kw == keywordinfo){
    if(nparams > kw_last->nparams)return 1;
  }
  else{
    if(nparams < kw_last->nparams)return 1;
  }
  return 0;
}

/* ------------------ CheckScript ------------------------ */

int CheckScript(char *file){
  FILE *stream = NULL;
  char *keyword, buffer[1024], were1[32], were2[32];
  int nparams = 0, return_val, reset;
  keyworddata *kw, *kw_last;

  stream = FOPEN(file, "r");
  if(stream == NULL){
    fprintf(stderr, "*** Error: scriptfile, %s, could not be opened for input\n", file);
    return 1;
  }

  // define script keywords
  InitKeywords();

  line_number=0;
  return_val = 0;
  kw = NULL;
  kw_last = NULL;
  reset = 0;
  for(;;){
    char *comment;

    if(fgets(param_buffer, 1024, stream)==NULL){
      if(GetScriptError(kw, kw_last, nparams) == 1){
        fprintf(stderr, "***error: script keyword %s in %s(%i) has the wrong number of data lines\n", kw_last->keyword, file, kw_last->line_number);
        fprintf(stderr, "          %s expected, %s found\n", GetWere(kw_last->nparams, were1), GetWere(nparams, were2));
        if(nparams > kw_last->nparams)printf("          invalid keyword: %s\n", keyword);
        return_val = 2;
      }
      fclose(stream);
      return return_val;
    }
    line_number++;
    comment = strstr(param_buffer, "//");
    if(comment==NULL)comment = strstr(param_buffer, "#");
    if(comment != NULL)comment[0] = 0;
    keyword = TrimFrontBack(buffer);
    if(strlen(buffer)==0)continue;

    kw = GetScriptKeywordFromLabel(keyword);
    if(reset == 1){ // an error was found, don't check for more errors until the next keyword
      if(kw != keywordinfo){
        reset = 0;
        kw_last = kw;
        kw_last->line_number = line_number;
        nparams = 0;
      }
      continue;
    }
    if(kw == keywordinfo)nparams++;
    if(GetScriptError(kw, kw_last, nparams)==1){
      fprintf(stderr, "***error: script keyword %s in %s(%i) has the wrong number of data lines\n", kw_last->keyword, file, kw_last->line_number);
      fprintf(stderr, "          %s expected, %s found\n", GetWere(kw_last->nparams,were1), GetWere(nparams,were2));
      if(nparams>kw_last->nparams)printf("          invalid keyword: %s\n", keyword);
      reset = 1;
      return_val = 2;
    }
    if(kw==keywordinfo)continue;
    kw_last = kw;
    kw_last->line_number = line_number;
    nparams = 0;
  }
  fclose(stream);
  return return_val;
}

/* ------------------ GetScriptKeyword ------------------------ */

keyworddata *GetScriptKeyword(FILE *stream){
  char *keyword=NULL, keyword_buffer[1024];

  for(;;){
    char *comment;

    if(fgets(keyword_buffer, 1024, stream)==NULL)return NULL;
    line_number++;
    comment = strstr(keyword_buffer, "//");
    if(comment==NULL)comment = strstr(keyword_buffer, "#");
    if(comment != NULL)comment[0] = 0;
    TrimBack(keyword_buffer);
    if(strlen(keyword_buffer)==0||keyword_buffer[0]==' ')continue;
    keyword = keyword_buffer;
    break;
  }

  if(keyword==NULL||strlen(keyword)==0){
    strcpy(keywordinfo->keyword, keyword_buffer);
    return keywordinfo;
  }

  int i;
  for(i = 1;i < nkeywordinfo;i++){
    keyworddata *kwi;

    kwi = keywordinfo + i;
    if(MatchSSF(kwi->keyword, keyword) == MATCH)return kwi;
  }
  strcpy(keywordinfo->keyword, keyword_buffer);
  return keywordinfo;
}

/* ------------------ GetXYZ ------------------------ */

void GetXYZ(char *buffer,int *ival){
  int i;

  for(i=0;i<(int)strlen(buffer);i++){
    char *c;

    c = buffer+i;
    if(*c!=' '){
      if(*c=='x'||*c=='X'||*c=='1'){
        *ival=1;
        return;
      }
      if(*c=='y'||*c=='Y'||*c=='2'){
        *ival=2;
        return;
      }
      if(*c=='z'||*c=='Z'||*c=='3'){
        *ival=3;
        return;
      }
      if(*c == 'a' || *c == 'A' || *c == '0'){
        *ival = 0;
        return;
      }
      *ival = 1;
      break;
    }
  }
}


#define SETbuffer \
param_status = GetParamBuffer(stream);\
if(param_status == SCRIPT_EOF){\
  scriptEOF = 1;\
  break;\
};\
if(param_status != SCRIPT_OK)return 1;

#define SETcval \
SETbuffer;\
scripti->cval=GetCharPointer(param_buffer);\

#define SETcval2 \
SETbuffer;\
scripti->cval2=GetCharPointer(param_buffer);

#define SETfval \
SETbuffer;\
sscanf(param_buffer, "%f", &scripti->fval);

#define SETival \
SETbuffer;\
sscanf(param_buffer, "%i", &scripti->ival);

#define SETival2 \
SETbuffer;\
sscanf(param_buffer, "%i", &scripti->ival2);

/* ------------------ RemoveDeg ------------------------ */

void RemoveDeg(char *string){
  int i,ii;

  if(string == NULL)return;
  for(i = 0,ii=0; i < strlen(string);i++){
    unsigned char c;

    c = (unsigned char)string[i];
    if(c == DEG_SYMBOL)continue;
    string[ii] = string[i];
    ii++;
  }
  string[ii] = 0;
}

#define TOKEN_UNKNOWN -1
#define TOKEN_INT      0
#define TOKEN_FLOAT    1
#define TOKEN_STRING   2
#define TOKEN_LOGICAL  3

/* ------------------ GetSLCFKeyWord ------------------------ */

int GetSLCFKeyWord(char *token, char **keywords, int nkeywords){
  int i;

  for(i = 0; i<nkeywords; i++){
    char *kw;

    kw = keywords[i];
    if(strcmp(token, kw)==0)return i;
  }
  return TOKEN_UNKNOWN;
}

/* ------------------ ParseSLCFTokens ------------------------ */

int ParseSLCFTokens(char *buffer, char **keywords, int *type, int nkeywords, int *tokens, int *itokens, float *ftokens, char **ctokens, int max_tokens){
  int i;
  char *kw;

  StripCommas(buffer);
  kw = strtok(buffer, "=");
  for(i = 0; i<max_tokens; i++){
    char *val;
    int keyword_index;

    if(i!=0)kw = strtok(NULL, "=");
    if(kw==NULL)return i;
    kw = TrimFrontBack(kw);
    keyword_index = GetSLCFKeyWord(kw, keywords, nkeywords);
    if(keyword_index==TOKEN_UNKNOWN||type[keyword_index]==TOKEN_UNKNOWN){
      printf("***error: script keyword %s unknown\n", kw);
      return 0;
    }
    tokens[i] = keyword_index;
    if(type[keyword_index]==TOKEN_STRING){
      val = strtok(NULL, "'");
    }
    else{
      val = strtok(NULL, " ");
    }
    if(val==NULL)return i;
    val = TrimFrontBack(val);
    switch(type[keyword_index]){
      case TOKEN_INT:
        sscanf(val, "%i", itokens+i);
        break;
      case TOKEN_FLOAT:
        sscanf(val, "%f", ftokens+i);
        break;
      case TOKEN_STRING:
        ctokens[i] = GetCharPointer(val);
        break;
      case TOKEN_LOGICAL:
        if(val[0]=='.')val++;
        if(val[0]=='T'||val[0]=='t'){
          itokens[i] = 1;
        }
        else if(val[0]=='F'||val[0]=='f'){
          itokens[i] = 0;
        }
        else{
          printf("***error: expected true or false for keyword: %s\n", kw);
          return 0;
        }
        break;
      default:
	assert(FFALSE);
	break;
    }
  }
  return i;

}

/* ------------------ CompileScript ------------------------ */

int CompileScript(char *scriptfile){
  FILE *stream;
  int return_val;

  if(scriptfile==NULL){
    fprintf(stderr,"*** Error: scriptfile name is NULL\n");
    return 1;
  }

  /*
   ************************************************************************
   ************************ start of pass 1 *********************************
   ************************************************************************
 */

  return_val = CheckScript(scriptfile);
  if(return_val!=0)return return_val;

 /*
   ************************************************************************
   ************************ start of pass 2 *********************************
   ************************************************************************
 */

  stream = FOPEN(scriptfile, "r");
  if(stream == NULL){
    fprintf(stderr, "*** Error: scriptfile, %s, could not be opened for input\n", scriptfile);
    return 1;
  }
  FreeScript();

  return_val=0;
  line_number = 0;
  while(!feof(stream)){
    keyworddata *kw;

    kw = GetScriptKeyword(stream);
    if(kw == NULL)break;
    if(kw != keywordinfo)nscriptinfo++;
  }

  if(nscriptinfo==0){
    fclose(stream);
    fprintf(stderr,"*** Error: scriptfile %s has no usable commands\n", scriptfile);
    return 1;
  }

  NewMemory((void **)&scriptinfo, nscriptinfo*sizeof(scriptdata));

  nscriptinfo=0;

  /*
   ************************************************************************
   ************************ start of pass 3 *********************************
   ************************************************************************
 */

  line_number=0;
  rewind(stream);
  while(!feof(stream)){
    int scriptEOF;
    scriptdata *scripti;
    int fatal_error;
    keyworddata *kw;

    fatal_error = 0;
    kw = GetScriptKeyword(stream);
    if(kw == NULL)break;
    if(kw == keywordinfo){
      printf("***error: unknown script keyword '%s' in %s(%i)\n", kw->keyword, scriptfile, line_number);
      return 2;
    }

    scripti = scriptinfo + nscriptinfo;
    InitScriptI(scripti, kw->index, kw->keyword);

    scriptEOF=0;
    switch(kw->index){

// UNLOADALL
      case SCRIPT_UNLOADALL:
        break;

// LOADPARTICLES
      case SCRIPT_LOADPARTICLES:
        scripti->need_graphics = 0;
        break;

// CBARFLIP:
      case SCRIPT_CBARFLIP:
        break;

// CBARNORMAL:
      case SCRIPT_CBARNORMAL:
        break;

// OUTPUTSMOKESENSORS
      case SCRIPT_OUTPUTSMOKESENSORS:
        break;

// SHOWHVACDUCTVAL
      case SCRIPT_SHOWHVACDUCTVAL:
        SETcval;
        break;

// SHOWCBAREDIT
       case SCRIPT_SHOWCBAREDIT:
         break;

// HIDECBAREDIT
       case SCRIPT_HIDECBAREDIT:
         break;

// SETCBAR
       case SCRIPT_SETCBAR:
         SETcval;
         break;

// HIDEHVACVALS
       case SCRIPT_HIDEHVACVALS:
         break;

// SETCBARLAB
       case SCRIPT_SETCBARLAB:
         break;

// SETCBARRGB
       case SCRIPT_SETCBARRGB:
         break;

// SHOWHVACNODEVAL
      case SCRIPT_SHOWHVACNODEVAL:
        SETcval;
        break;

// SETDEMOMODE
      case SCRIPT_SETDEMOMODE:
        SETival;
        scripti->ival = CLAMP(scripti->ival, 0, 5);
        break;

// SMOKEPROP
      case SCRIPT_SMOKEPROP:
        SETfval;
        break;

        // SHOWALLDEVS
      case SCRIPT_SHOWALLDEVS:
        break;

// HIDEALLDEVS
      case SCRIPT_HIDEALLDEVS:
        break;

// SHOWDEV
// dev_id
      case SCRIPT_SHOWDEV:
        SETcval;
        break;

// HIDEDEV
//  dev_id
      case SCRIPT_HIDEDEV:
        SETcval;
        break;

// RENDERSIZE
// width height (int)
      case SCRIPT_RENDERSIZE:
        SETbuffer;
        sscanf(param_buffer, "%i %i", &scripti->ival, &scripti->ival2);
        break;

// HILIGHTMINVALS
//  show/hide r g b (int)
// HILIGHTMAXVALS
//  show/hide r g b (int)
      case SCRIPT_HILIGHTMAXVALS:
      case SCRIPT_HILIGHTMINVALS:
        SETbuffer;
        sscanf(param_buffer, "%i %i %i %i", &scripti->ival, &scripti->ival2, &scripti->ival3, &scripti->ival4);
        scripti->ival  = CLAMP(scripti->ival,  0, 1);
        scripti->ival2 = CLAMP(scripti->ival2, 0, 255);
        scripti->ival3 = CLAMP(scripti->ival3, 0, 255);
        scripti->ival4 = CLAMP(scripti->ival4, 0, 255);
        break;

        // RENDERTYPE
//  jpg or png  (char)
      case SCRIPT_RENDERTYPE:
        SETcval;
        if(STRCMP(scripti->cval, "PNG") == 0){
          scripti->ival = PNG;
        }
        else{
          scripti->ival = JPEG;
        }
        break;

// MOVIETYPE
//  jpg, png or wmv  (char)
      case SCRIPT_MOVIETYPE:
        SETcval;
        if(STRCMP(scripti->cval, "WMV") == 0){
          scripti->ival = WMV;
        }
        if(STRCMP(scripti->cval, "MP4") == 0){
          scripti->ival = MP4;
        }
        else{
          scripti->ival = AVI;
        }
        break;

// RENDERDIR
// RENDERHTMLDIR
//  directory name (char) (where rendered files will go)
      case SCRIPT_RENDERDIR:
      case SCRIPT_RENDERHTMLDIR:
      {
        int len;
        int i;

        scripti->need_graphics = 1;
        if(kw->index==SCRIPT_RENDERHTMLDIR)scripti->need_graphics = 0;
        SETbuffer;
        if(script_renderdir_cmd!=NULL&&strlen(script_renderdir_cmd)>0){
          strcpy(param_buffer, script_renderdir_cmd);
        }
        len = strlen(param_buffer);
        if(len>0){
#ifdef _WIN32
          for(i=0;i<len;i++){
            if(param_buffer[i]=='/')param_buffer[i]='\\';
          }
          if(param_buffer[len-1]!='\\')strcat(param_buffer,dirseparator);
#else
          for(i=0;i<len;i++){
            if(param_buffer[i]=='\\')param_buffer[i]='/';
          }
          if(param_buffer[len-1]!='/')strcat(param_buffer,dirseparator);
#endif
          scripti->cval= GetCharPointer(param_buffer);
        }
        break;
      }

// SCENECLIP
//  clip mode (int)
      case SCRIPT_SCENECLIP:

// LOADVOLSMOKE
//  mesh number (-1 for all meshes) (int)
      case SCRIPT_LOADVOLSMOKE:
        scripti->need_graphics = 0;
        SETival;
        break;

// X/y/ZSCENECLIP
// imin (int) min (float) imax (int) max (float)
      case SCRIPT_XSCENECLIP:
      case SCRIPT_YSCENECLIP:
      case SCRIPT_ZSCENECLIP:
        SETbuffer;
        sscanf(param_buffer,"%i %f %i %f",&scripti->ival,&scripti->fval,&scripti->ival2,&scripti->fval2);
        break;

// RENDERCLIP
// flag left right bottom top indentations in pixels, clip if flag==1
      case SCRIPT_RENDERCLIP:
        SETbuffer;
        sscanf(param_buffer,"%i %i %i %i %i",&scripti->ival,&scripti->ival2,&scripti->ival3,&scripti->ival4, &scripti->ival5);
        break;

// RENDERONCE
// file name base (char) (or blank to use smokeview default)
      case SCRIPT_RENDERONCE:

// RENDERDOUBLEONCE
// file name base (char) (or blank to use smokeview default)
      case SCRIPT_RENDERDOUBLEONCE:
        SETcval2;
        break;

// RENDERHTMLONCE
// RENDERHTMLALL
// file name base (char) (or blank to use smokeview default)
      case SCRIPT_RENDERHTMLONCE:
      case SCRIPT_RENDERHTMLALL:
      case SCRIPT_RENDERHTMLGEOM:
      case SCRIPT_RENDERHTMLOBST:
        scripti->need_graphics = 0;
        SETcval2;
        break;
      case SCRIPT_RENDERHTMLSLICENODE:
      case SCRIPT_RENDERHTMLSLICECELL:
        //  0 current frame, 1 all frames
        // file name base (char) (or blank to use smokeview default)
        SETbuffer;
        scripti->ival = 1;   // skip
        sscanf(param_buffer, "%i", &scripti->ival);

        SETcval2;
        scripti->need_graphics = 0;
        break;

// RENDERSTART
//  start_frame (int) skip_frame (int)
      case SCRIPT_RENDERSTART:
        SETbuffer;
        sscanf(param_buffer,"%i %i",&scripti->ival,&scripti->ival2);
        break;

// RENDERALL
//  skip (int)
// file name base (char) (or blank to use smokeview default)
      case SCRIPT_RENDERALL:
        SETbuffer;
        scripti->ival=1;   // skip
        scripti->ival3=0;  // first frame
        sscanf(param_buffer,"%i %i",&scripti->ival,&scripti->ival3);
        scripti->ival = MAX(scripti->ival, 1);
        scripti->ival3 = MAX(scripti->ival3, 0);
        first_frame_index=scripti->ival3;

        SETcval2;
        break;

// RENDER360ALL
//  skip (int)
// file name base (char) (or blank to use smokeview default)
      case SCRIPT_RENDER360ALL:
        SETbuffer;
        scripti->ival = 1;   // skip
        scripti->ival3 = 0;  // first frame
        sscanf(param_buffer, "%i %i", &scripti->ival, &scripti->ival3);
        scripti->ival = MAX(scripti->ival, 1);
        scripti->ival3 = MAX(scripti->ival3, 0);
        first_frame_index = scripti->ival3;

        SETcval2;
        break;

// VOLSMOKERENDERALL
//  skip (int) start_frame (int)
// file name base (char) (or blank to use smokeview default)
      case SCRIPT_VOLSMOKERENDERALL:
        SETbuffer;
        scripti->ival3=0;  // first frame
        scripti->ival=1;
        sscanf(param_buffer,"%i %i",&scripti->ival,&scripti->ival3);
        scripti->ival=CLAMP(scripti->ival,1,20); // skip
        scripti->exit=0;
        scripti->first=1;
        scripti->remove_frame=-1;
        first_frame_index=scripti->ival3;

        SETcval2;
        break;

// LOADISOM
//  type (char)
//  mesh number (int)
      case SCRIPT_LOADISOM:
        SETcval;
        RemoveDeg(scripti->cval);
        scripti->ival = 1;
        SETival;
        scripti->need_graphics = 0;
        break;

// ISORENDERALL
//  skip (int) start_frame (int) iso file index (int) ( index of &ISOF line in .fds input file)
// file name base (char) (or blank to use smokeview default)
      case SCRIPT_ISORENDERALL:
        SETbuffer;
        scripti->ival3 = 0;  // first frame
        scripti->ival = 1;
        sscanf(param_buffer, "%i %i %i", &scripti->ival, &scripti->ival3, &scripti->ival4);
        scripti->ival=CLAMP(scripti->ival,1,20); // skip
        scripti->exit = 0;
        scripti->first = 1;
        scripti->remove_frame = -1;
        first_frame_index = scripti->ival3;

        SETcval2;  //render file base name
        break;

// MAKEMOVIE
//  movie_name (char)
//  frame_prefix (char)
//  framerate (float)
      case SCRIPT_MAKEMOVIE:
        SETcval;
        SETcval2;
        SETfval;
        break;
// NOEXIT
      case SCRIPT_NOEXIT:
// EXIT
      case SCRIPT_EXIT:

// UNLOADTOUR
      case SCRIPT_UNLOADTOUR:
        scripti->cval=NULL;
        break;

// SETCLIPX
// SETCLIPY
// SETCLIPZ
      case SCRIPT_SETCLIPX:
      case SCRIPT_SETCLIPY:
      case SCRIPT_SETCLIPZ:
        SETbuffer;
        sscanf(param_buffer, "%i %f %i %f", &scripti->ival, &scripti->fval, &scripti->ival2, &scripti->fval2);
        break;

// SETCLIPMODE
      case SCRIPT_SETCLIPMODE:
        SETbuffer;
        sscanf(param_buffer, "%i", &scripti->ival);
        break;

// GPUOFF
      case SCRIPT_GPUOFF:
        break;

// SETVIEWPOINT
//  viewpoint (char)
      case SCRIPT_SETVIEWPOINT:

// LABEL
//   text
      case SCRIPT_LABEL:

      case SCRIPT_KEYBOARD:
        SETcval;
        break;

// LOADINIFILE
//  file (char)
      case SCRIPT_LOADINIFILE:
        scripti->need_graphics = 0;
        SETcval;
        break;

 // LOADFILE
//  file (char)
      case SCRIPT_LOADFILE:

// LOADVFILE
//  file (char)
      case SCRIPT_LOADVFILE:

// PARTCLASSCOLOR
//   color (char)
      case SCRIPT_PARTCLASSCOLOR:

// PARTCLASSTYPE
//   type (char)
      case SCRIPT_PARTCLASSTYPE:

// LOADTOUR
//  type (char)
      case SCRIPT_LOADTOUR:

// LOAD3DSMOKE
//  type (char)
      case SCRIPT_LOAD3DSMOKE:

// LOADISO
//  type (char)
      case SCRIPT_LOADISO:

// LOADBOUNDARY
//   type (char)
      case SCRIPT_LOADBOUNDARY:
        SETcval;
        break;

// LOADBOUNDARYM
//  type (char)
//  mesh number (int)
      case SCRIPT_LOADBOUNDARYM:
        SETcval;
        SETival;
        scripti->need_graphics = 0;
        break;

// PLOT3DPROPS
//  plot3d type (int) showvector (0/1) (int) vector length index (int) plot3d display type (int)
      case SCRIPT_PLOT3DPROPS:
        SETbuffer;
        {
          float fv=-1;

          sscanf(param_buffer,"%i %i %i %i %f",&scripti->ival,&scripti->ival2,&scripti->ival3,&scripti->ival4,&fv);
          if(scripti->ival3<0&&fv>=0.0){
            scripti->fval=fv;
          }
          else{
            scripti->fval=-1.0;
          }
        }
        break;
      case SCRIPT_VIEWXMIN: // generate equivalent XYZVIEW command when script is run
      case SCRIPT_VIEWXMAX:
      case SCRIPT_VIEWYMIN:
      case SCRIPT_VIEWYMAX:
      case SCRIPT_VIEWZMIN:
      case SCRIPT_VIEWZMAX:
        break;

// XYZVIEW
//  xpos (float) ypos (float) zpos (float) az (float) elev (float)
      case SCRIPT_XYZVIEW:
        SETbuffer;

        sscanf(param_buffer, "%f %f %f %f %f", &scripti->fval, &scripti->fval2, &scripti->fval3, &scripti->fval4, &scripti->fval5);
        if(ABS(scripti->fval5-90)<0.1)scripti->fval5=89.9;
        if(ABS(scripti->fval5+90)<0.1)scripti->fval5=-89.9;
        break;

// UNLOADPLOT2D
//  (not parameters)
      case SCRIPT_UNLOADPLOT2D:
        break;

// SHOWPLOT3DDATA
//  mesh number (int) orientation (int)  value (0/1) (int) position (float)
      case SCRIPT_SHOWPLOT3DDATA:
        SETbuffer;
        sscanf(param_buffer,"%i %i %i %i %f",&scripti->ival,&scripti->ival2,&scripti->ival3,&scripti->ival4,&scripti->fval);
        if(scripti->ival2==4){
          sscanf(param_buffer,"%i %i %i %i %i",&scripti->ival,&scripti->ival2,&scripti->ival3,&scripti->ival4,&scripti->ival5);
        }
        break;

// LOADVOLSMOKEFRAME
//  mesh index, frame (int)
      case SCRIPT_LOADVOLSMOKEFRAME:
        SETbuffer;
        sscanf(param_buffer,"%i %i",&scripti->ival,&scripti->ival2);
        break;

// LOADSLICERENDER
//  (char)quantity
//  1/2/3 (int)dir  (float)position
//  (char)renderfile_base
// (int)start (int)skip (float) tmin (float)tmax
      case SCRIPT_LOADSLICERENDER:
        SETcval;

        SETbuffer;
        sscanf(param_buffer, "%i %f", &scripti->ival, &scripti->fval);
        scripti->ival = CLAMP(scripti->ival, 0, 3);
        scripti->need_graphics = 0;
        SETcval2;
        SETbuffer;
        scripti->fval2 = 1.0;
        scripti->fval3 = 0.0;
        sscanf(param_buffer, "%i %i %f %f", &scripti->ival2, &scripti->ival3, &scripti->fval2, &scripti->fval3);
        scripti->ival4 = scripti->ival2;
        scripti->first = 1;
        scripti->exit = 0;
        scripti->fval2 = 1.0;
        scripti->fval3 = 0.0;

        if(script_startframe>0)scripti->ival2=script_startframe;
        if(render_startframe0>=0)scripti->ival2=render_startframe0;

        if(script_skipframe>0)scripti->ival3=script_skipframe;
        if(render_skipframe0>0)scripti->ival3=render_skipframe0;
        break;

// LOADSMOKERENDER
//  (char)quantity (soot, hrrpuv, temp or co2)
//  (char)renderfile_base
// (int)start (int)skip
      case SCRIPT_LOADSMOKERENDER:
        SETcval;
        if(scripti->cval != NULL){
          char *semi;

          semi = strchr(scripti->cval, ';');
          if(semi != NULL){
            *semi = 0;
            semi++;
            scripti->cval3 = GetCharPointer(semi);
            scripti->cval = TrimFrontBack(scripti->cval);
          }
        }
        SETcval2;
        SETbuffer;
        sscanf(param_buffer, "%i %i", &scripti->ival2, &scripti->ival3);
        scripti->ival4 = scripti->ival2;
        scripti->first = 1;
        scripti->exit = 0;
        scripti->fval2 = 1.0;
        scripti->fval3 = 0.0;

        if(script_startframe>0)scripti->ival2=script_startframe;
        if(render_startframe0>=0)scripti->ival2=render_startframe0;

        if(script_skipframe>0)scripti->ival3=script_skipframe;
        if(render_skipframe0>0)scripti->ival3=render_skipframe0;
        break;

// LOADSLCF
//  PBX=val QUANTITY='quantity'
#define KW_QUANTITY      0
#define KW_ID            1
#define KW_PBX           2
#define KW_PBY           3
#define KW_PBZ           4
#define KW_PB3D          5
#define KW_AGL_SLICE     6
#define KW_VECTOR        7
#define KW_CELL_CENTERED 8
      case SCRIPT_LOADSLCF:
        {
#define MAX_SLCF_TOKENS 10
          char param_buffer_copy[1024];
          char *ctokens[MAX_SLCF_TOKENS];
          char *keywords[]={"QUANTITY",   "ID",         "PBX",       "PBY",       "PBZ",      "PB3D",        "AGL_SLICE",  "VECTOR",       "CELL_CENTERED"};
          int types[]={     TOKEN_STRING, TOKEN_STRING, TOKEN_FLOAT, TOKEN_FLOAT, TOKEN_FLOAT, TOKEN_LOGICAL, TOKEN_FLOAT, TOKEN_LOGICAL,   TOKEN_LOGICAL};
          int nkeywords=9, tokens[MAX_SLCF_TOKENS], itokens[MAX_SLCF_TOKENS], ntokens;
          float ftokens[MAX_SLCF_TOKENS];
          int i;

          SETbuffer;
          strcpy(param_buffer_copy, param_buffer);

          ntokens = ParseSLCFTokens(param_buffer, keywords, types, nkeywords, tokens, itokens, ftokens, &(ctokens[0]), MAX_SLCF_TOKENS);
          if(ntokens==0){
            printf("***error: problems were found parsing LOADSLCF\n");
            printf("  buffer: %s\n", param_buffer_copy);
            fatal_error = 1;
            break;
          }

          for(i=0;i<ntokens;i++){
            switch(tokens[i]){
              char label[100];

              case KW_QUANTITY:
                scripti->quantity = ctokens[i];
                break;
              case KW_ID:
                scripti->id = ctokens[i];
                break;
              case KW_PBX:
                scripti->pbxyz_val = ftokens[i];
                scripti->pbxyz_dir = 1;
                strcpy(label, "PBX");
                scripti->c_pbxyz   = GetCharPointer(label);
                break;
              case KW_PBY:
                scripti->pbxyz_val = ftokens[i];
                scripti->pbxyz_dir = 2;
                strcpy(label, "PBY");
                scripti->c_pbxyz   = GetCharPointer(label);
                break;
              case KW_PBZ:
                scripti->pbxyz_val = ftokens[i];
                scripti->pbxyz_dir = 3;
                strcpy(label, "PBZ");
                scripti->c_pbxyz   = GetCharPointer(label);
                break;
              case KW_PB3D:
                scripti->pbxyz_val = 0.0;
                scripti->pbxyz_dir = 0;
                strcpy(label, "PB3D");
                scripti->c_pbxyz = GetCharPointer(label);
                break;
              case KW_AGL_SLICE:
                scripti->pbxyz_val = ftokens[i];
                scripti->pbxyz_dir = 3;
                strcpy(label, "AGL_SLICE");
                scripti->c_pbxyz = GetCharPointer(label);
                break;
              case KW_VECTOR:
                scripti->vector = itokens[i];
                break;
              case KW_CELL_CENTERED:
                scripti->cell_centered = itokens[i];
                break;
	      default:
		       assert(FFALSE);
		break;
            }
          }
          if(scripti->id==NULL){
            if(scripti->quantity==NULL){
              printf("***error: The keyword QUANTITY is required if ID is not used\n");
              fatal_error = 1;
            }
            if(scripti->c_pbxyz==NULL){
              printf("***error: One of the keywords PBX, PBY or PBZ is required if ID is not used\n");
              fatal_error = 1;
            }
          }
        }
        break;

        // LOADSLICE
//  (char)quantity
//  1/2/3 (int)dir  (float)position
      case SCRIPT_LOADSLICE:

// LOADVSLICE
//  type (char)
//  1/2/3 (int)  val (float)
      case SCRIPT_LOADVSLICE:
        SETcval;

        SETbuffer;
        sscanf(param_buffer, "%i %f", &scripti->ival, &scripti->fval);
        scripti->ival = CLAMP(scripti->ival, 0, 3);
        scripti->need_graphics = 0;
        break;

// LOADVSLICEM
//  type (char)
//  1/2/3 (int)  val (float)
//  mesh number (int)
      case SCRIPT_LOADVSLICEM:
        SETcval;

        SETbuffer;
        sscanf(param_buffer, "%i %f", &scripti->ival, &scripti->fval);
        scripti->ival = CLAMP(scripti->ival, 0, 3);
        SETival2;
        scripti->need_graphics = 0;
        break;

// LOADSLICEM
//  type (char)
//  1/2/3 (int)  val (float)
//  mesh number (int)
      case SCRIPT_LOADSLICEM:
        SETcval;

        SETbuffer;
        sscanf(param_buffer, "%i %f", &scripti->ival, &scripti->fval);
        scripti->ival = CLAMP(scripti->ival, 0, 3);
        SETival2;
        scripti->need_graphics = 0;
        break;

// LOADPLOT3D
//  mesh number (int) time (float)
      case SCRIPT_LOADPLOT3D:
        SETbuffer;
        sscanf(param_buffer," %i %f",&scripti->ival,&scripti->fval);
        scripti->need_graphics = 0;
        break;

// LOADHVAC
      case SCRIPT_LOADHVAC:
        break;

// SETTIMEVAL
//  time (float)
      case SCRIPT_SETTIMEVAL:
        SETfval;
        if(scripti->fval<0.0)scripti->fval=0.0;
        scripti->need_graphics = 0;
        break;

// SETBOUNDBOUNDS
//  type (char) ivalmin (int) valmin (float) ivalmax (int) valmax quantity (char)
      case SCRIPT_SETBOUNDBOUNDS:
        SETbuffer;
        sscanf(param_buffer," %i %f %i %f, %s",&scripti->ival,&scripti->fval, &scripti->ival2,&scripti->fval2, scripti->quantity2);
        break;

// SETSLICEBOUNDS
//  type (char) ivalmin (int) valmin (float) ivalmax (int) valmax quantity (char)
      case SCRIPT_SETSLICEBOUNDS:
        SETbuffer;
        sscanf(param_buffer," %i %f %i %f, %s",&scripti->ival,&scripti->fval, &scripti->ival2,&scripti->fval2, scripti->quantity2);
        break;

// SETSLICEAVERAGE
//  slice_average_flag (int) slice_average_interval (float)
      case SCRIPT_SETSLICEAVERAGE:
        SETbuffer;
        sscanf(param_buffer, " %i %f", &scripti->ival, &scripti->fval);
        break;

// OUTPUTSLICEDATA
//  output_slicedata (int)
      case SCRIPT_OUTPUTSLICEDATA:
        SETbuffer;
        sscanf(param_buffer, " %i", &scripti->ival);
        break;

        // SETTOURVIEW
//   viewtype  showpath showtour_locus
      case SCRIPT_SETTOURVIEW:
        SETbuffer;
        sscanf(param_buffer,"%i %i %i",&scripti->ival,&scripti->ival2,&scripti->ival3);
        break;

// SETTOURKEYFRAME
//  time (float)
      case SCRIPT_SETTOURKEYFRAME:
        SETfval;
        break;

// RGBTEST
//  x y z r g b delta
      case SCRIPT_RGBTEST:
        SETbuffer;
        sscanf(param_buffer, "%f %f %f %i %i %i %i", &scripti->fval, &scripti->fval2, &scripti->fval3, &scripti->ival, &scripti->ival2, &scripti->ival3, &scripti->ival4);
        break;

        // GSLICEVIEW
// show_gslice (int) show_triangles (int)  show_triangulation (int) show_normals (int)
      case SCRIPT_GSLICEVIEW:
        SETbuffer;
        sscanf(param_buffer,"%i %i %i %i",&scripti->ival,&scripti->ival2,&scripti->ival3,&scripti->ival4);
        break;

    // PROJECTION
        // 1/2 perspective/size preserving
     case SCRIPT_PROJECTION:
       SETbuffer;
       scripti->ival = 1;
       sscanf(param_buffer, "%i", &scripti->ival);
       if(scripti->ival!=2)scripti->ival = 1;
       break;

// GSLICEPOS
// x (float) y (float) z (float)
      case SCRIPT_GSLICEPOS:
        SETbuffer;
        sscanf(param_buffer,"%f %f %f",&scripti->fval,&scripti->fval2,&scripti->fval3);
        break;

// GSLICEORIEN
// azimuth (float) elevation (float)
      case SCRIPT_GSLICEORIEN:
        SETbuffer;
        sscanf(param_buffer,"%f %f",&scripti->fval,&scripti->fval2);
        break;
      default:
	assert(FFALSE);
	break;
    }
    if(scriptEOF==1)break;
    if(kw!=keywordinfo&&fatal_error==0)nscriptinfo++;
  }
  fclose(stream);
  return return_val;
}

/* ------------------ GetWebFileName ------------------------ */

void GetWebFileName(char *web_filename, scriptdata *scripti){
  strcpy(web_filename, "");
  if(script_htmldir_path!=NULL){
    if(strlen(script_htmldir_path)!=2||
      script_htmldir_path[0]!='.'||
      script_htmldir_path[1]!=dirseparator[0]){
      strcat(web_filename, script_htmldir_path);
      strcat(web_filename, dirseparator);
    }
  }
  if(scripti->cval2 != NULL){
    strcat(web_filename, scripti->cval2);
  }
}

/* ------------------ ScriptRenderSliceNode ------------------------ */

void ScriptRenderSliceNode(scriptdata *scripti){
  char web_filename[1024];

  GetWebFileName(web_filename, scripti);
  if(scripti->ival==0){
    SliceNode2Data(web_filename, HTML_CURRENT_TIME);
  }
  else{
    SliceNode2Data(web_filename, HTML_ALL_TIMES);
  }
}

/* ------------------ ScriptRenderSliceCell ------------------------ */

void ScriptRenderSliceCell(scriptdata *scripti){
  char web_filename[1024];

  GetWebFileName(web_filename, scripti);
  if(scripti->ival==0){
    SliceCell2Data(web_filename, HTML_CURRENT_TIME);
  }
  else{
    SliceCell2Data(web_filename, HTML_ALL_TIMES);
  }
}

/* ------------------ ScriptRenderObst ------------------------ */

void ScriptRenderObst(scriptdata *scripti){
  char web_filename[1024];

  GetWebFileName(web_filename, scripti);
  Obst2Data(web_filename);
}

/* ------------------ ScriptRenderGeom ------------------------ */

void ScriptRenderGeom(scriptdata *scripti){
  char web_filename[1024];

  GetWebFileName(web_filename, scripti);
  Smv2Geom(web_filename);
}

/* ------------------ ScriptRenderHtml ------------------------ */

void ScriptRenderHtml(scriptdata *scripti, int option){
  char web_filename[1024];
  char webvr_filename[1024];

  GetWebFileName(web_filename, scripti);
  strcat(web_filename,".html");
  Smv2Html(web_filename, option, FROM_SCRIPT);

  GetWebFileName(webvr_filename, scripti);
  strcat(webvr_filename,"_vr.html");
}

/* ------------------ ScriptRenderStart ------------------------ */

void ScriptRenderStart(scriptdata *scripti){
  script_startframe=scripti->ival;
  script_skipframe=scripti->ival2;
}

/* ------------------ PrintRenderMessage ------------------------ */

void PrintRenderMessage(int skip_frame, int first_frame){
  if(skip_frame==1){
    PRINTF("script: Rendering every frame starting at frame %i\n\n",first_frame);
  }
  else if(skip_frame==2){
    PRINTF("script: Rendering every %i'nd frame starting at frame %i\n\n",skip_frame,first_frame);
  }
  else if(skip_frame==3){
    PRINTF("script: Rendering every %i'rd frame starting at frame %i\n\n",skip_frame,first_frame);
  }
  else{
    PRINTF("script: Rendering every %i'th frame starting at frame %i\n\n",skip_frame,first_frame);
  }
}

/* ------------------ ScriptRenderAll ------------------------ */

void ScriptRenderAll(scriptdata *scripti){
  int skip_local;

  if(script_startframe>0)scripti->ival3=script_startframe;
  if(render_startframe0>=0)scripti->ival3=render_startframe0;
  first_frame_index=scripti->ival3;
  itimes=first_frame_index;

  if(script_skipframe>0)scripti->ival=script_skipframe;
  if(render_skipframe0>0)scripti->ival=render_skipframe0;
  skip_local=MAX(1,scripti->ival);

  PrintRenderMessage(skip_local,first_frame_index);
  SkipMenu(skip_local);
//  render_skip = skip_local;
//  GLUTPOSTREDISPLAY;
//  updatemenu = 1;
  RenderMenu(RenderStartORIGRES);
}

/* ------------------ ScriptRender360All ------------------------ */

void ScriptRender360All(scriptdata *scripti){
  int skip_local;


  if(script_startframe>0)scripti->ival3 = script_startframe;
  if(render_startframe0 >= 0)scripti->ival3 = render_startframe0;
  first_frame_index = scripti->ival3;
  itimes = first_frame_index;

  if(script_skipframe>0)scripti->ival = script_skipframe;
  if(render_skipframe0>0)scripti->ival = render_skipframe0;
  skip_local = MAX(1, scripti->ival);

  PrintRenderMessage(skip_local,first_frame_index);
  render_mode = RENDER_360;
  SkipMenu(skip_local);
  RenderCB(RENDER_START);
}

/* ------------------ GetVolFrameMax ------------------------ */

int GetVolFrameMax(int meshnum){
  int i, volframemax=-1;

  volframemax = -1;
  for(i = 0; i<global_scase.meshescoll.nmeshes; i++){
    meshdata *meshi;
    volrenderdata *vr;

    if(meshnum!=i && meshnum>=0)continue;
    meshi = global_scase.meshescoll.meshinfo+i;
    vr = meshi->volrenderinfo;
    volframemax = MAX(volframemax,vr->ntimes);
  }
  return volframemax;
}

/* ------------------ LoadSmokeFrame ------------------------ */

void LoadSmokeFrame(int meshnum, int framenum){
  int first = 1;
  int i;
  int max_frames = -1, frame_old;
  float valtime=0.0;

  if(meshnum > global_scase.meshescoll.nmeshes - 1||meshnum<-1)meshnum = -1;

  max_frames = GetVolFrameMax(meshnum);
  if(max_frames > 0)GLUIUpdateLoadFrameMax(max_frames);
  frame_old = framenum;
  framenum = CLAMP(framenum, 0, max_frames-1);
  if(framenum!=frame_old)GLUIUpdateLoadFrameVal(framenum);

  for(i = 0; i<global_scase.meshescoll.nmeshes; i++){
    meshdata *meshi;
    volrenderdata *vr;

    if(meshnum != i && meshnum >= 0)continue;
    meshi = global_scase.meshescoll.meshinfo + i;
    vr = meshi->volrenderinfo;
    FreeVolsmokeFrame(vr, framenum);
    ReadVolsmokeFrame(vr, framenum, &first);
    if(vr->times_defined == 0){
      vr->times_defined = 1;
      GetVolsmokeAllTimes(vr);
    }
    vr->loaded = 1;
    vr->display = 1;
    valtime = vr->times[framenum];
  }
  plotstate = GetPlotState(DYNAMIC_PLOTS);
  stept = 1;
  UpdateTimes();
  force_redisplay = 1;
  UpdateFrameNumber(framenum);
  i = framenum;
  itimes = i;
  script_itime = i;
  stept = 1;
  force_redisplay = 1;
  UpdateFrameNumber(0);
  stept=1;
  Keyboard('t', FROM_SMOKEVIEW);
  UpdateTimeLabels();
  GLUIUpdateLoadTimeVal(valtime);
}

/* ------------------ LoadTimeFrame ------------------------ */

void LoadTimeFrame(int meshnum, float timeval){
  int i, smokeframe;
  float vrtime, mindiff;
  meshdata *meshi;
  volrenderdata *vr;
  int meshnum_orig;
  int update_timebounds = 0;

  meshnum_orig = meshnum;
  if(meshnum<0||meshnum>global_scase.meshescoll.nmeshes-1)meshnum = 0;

  meshi = global_scase.meshescoll.meshinfo+meshnum;
  vr = meshi->volrenderinfo;

  if(vr->times_defined==0)LoadSmokeFrame(meshnum_orig, 0);
  if(time_framemin>time_framemax){
    time_framemin = vr->times[0];
    time_framemax = vr->times[vr->ntimes-1];
    update_timebounds = 1;
  }
  else{
    if(vr->times[0]<time_framemin){
      time_framemin = vr->times[0];
      update_timebounds = 1;
    }
    if(vr->times[vr->ntimes-1]>time_framemax){
      time_framemax = vr->times[vr->ntimes-1];
      update_timebounds = 1;
    }
  }
  if(update_timebounds==1)GLUIUpdateTimeFrameBounds(time_framemin, time_framemax);

  vrtime = vr->times[0];
  mindiff = ABS(timeval-vrtime);
  smokeframe = 0;
  for(i = 1;i<vr->ntimes;i++){
    float diff;

    vrtime = vr->times[i];
    diff = ABS(timeval-vrtime);
    if(diff<mindiff){
      mindiff = diff;
      smokeframe = i;
    }
  }
  GLUIUpdateLoadFrameVal(smokeframe);
  LoadSmokeFrame(meshnum, smokeframe);
}

/* ------------------ ScriptLoadVolSmokeFrame ------------------------ */

void ScriptLoadVolSmokeFrame(scriptdata *scripti, int flag){
  int framenum, index;

  index = scripti->ival;
  framenum = scripti->ival2;
  LoadSmokeFrame(index, framenum);
  Keyboard('r', FROM_SMOKEVIEW);
  if(flag == 1)script_render = 1;// called when only rendering a single frame
}

/* ------------------ ScriptLoadVolSmokeFrame2 ------------------------ */

void ScriptLoadVolSmokeFrame2(void){
  scriptdata scripti;

  scripti.ival = -1;
  scripti.ival2 = itimes;
  ScriptLoadVolSmokeFrame(&scripti, 0);
}

/* ------------------ ScriptVolSmokeRenderAll ------------------------ */

void ScriptVolSmokeRenderAll(scriptdata *scripti){
  int skip_local;

  if(nvolrenderinfo==0){
    PRINTF("*** Error: there is no volume rendered smoke data to render\n");
    ScriptMenu(SCRIPT_CANCEL);
    return;
  }
  ScriptLoadVolSmokeFrame2();

  if(script_startframe>0)scripti->ival3=script_startframe;
  if(vol_startframe0>0)scripti->ival3=vol_startframe0;
  // check first_frame_index
  first_frame_index=scripti->ival3;
  itimes=first_frame_index;

  if(script_skipframe>0)scripti->ival=script_skipframe;
  if(vol_skipframe0>0)scripti->ival=vol_skipframe0;
  skip_local=MAX(1,scripti->ival);

  PRINTF("script: Rendering every %i frame(s) starting at frame %i\n\n",skip_local,scripti->ival3);
  scripti->ival=skip_local;
  RenderMenu(skip_local);
}

/* ------------------ ScriptLoadIsoFrame ------------------------ */

void ScriptLoadIsoFrame(scriptdata *scripti, int flag){
  int framenum;
  int i;
  int fileindex;

  THREADcontrol(isosurface_threads, THREAD_JOIN);
  framenum = scripti->ival2;
  fileindex = scripti->ival4;

  update_readiso_geom_wrapup = UPDATE_ISO_START_ALL;
  CancelUpdateTriangles();
  for(i = 0; i < global_scase.nisoinfo; i++){
    int errorcode;
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    if(isoi->isof_index + 1 == fileindex){
      ReadIso(isoi->file, i, LOAD, &framenum, &errorcode);
    }
  }
  if(update_readiso_geom_wrapup == UPDATE_ISO_ALL_NOW)ReadIsoGeomWrapup(FOREGROUND);
  update_readiso_geom_wrapup = UPDATE_ISO_OFF;

  plotstate = GetPlotState(DYNAMIC_PLOTS);
  stept = 1;
  UpdateTimes();
  force_redisplay = 1;
  UpdateFrameNumber(framenum);
  i = framenum;
  itimes = i;
  script_itime = i;
  stept = 1;
  force_redisplay = 1;
  UpdateFrameNumber(0);
  UpdateTimeLabels();
  Keyboard('r', FROM_SMOKEVIEW);
  if(flag == 1)script_render = 1;// called when only rendering a single frame
}

/* ------------------ ScriptLoadIsoFrame2 ------------------------ */

void ScriptLoadIsoFrame2(scriptdata *scripti){
  scripti->ival2 = itimes;
  ScriptLoadIsoFrame(scripti, 0);
}

/* ------------------ ScriptIsoRenderAll ------------------------ */

void ScriptIsoRenderAll(scriptdata *scripti){
  int skip_local;

  //  skip (int) start_frame (int) iso file index (int) ( index of &ISOF line in .fds input file)
  //  skip == scripti->ival
  //  start_frame == scripti->ival3
  //  &ISOF index == scripti->ival4

  if(global_scase.nisoinfo == 0){
    PRINTF("*** Error: there is no isosurface data to render\n");
    ScriptMenu(SCRIPT_CANCEL);
    return;
  }
  ScriptLoadIsoFrame2(scripti);

  if(script_startframe>0)scripti->ival3 = script_startframe;
  if(render_startframe0>0)scripti->ival3 = render_startframe0;
  // check first_frame_index
  first_frame_index = scripti->ival3;
  itimes = first_frame_index;

  if(script_skipframe>0)scripti->ival = script_skipframe;
  if(render_skipframe0>0)scripti->ival = render_skipframe0;
  skip_local = MAX(1, scripti->ival);

  PRINTF("script: Rendering every %i frame(s) starting at frame %i\n\n", skip_local, scripti->ival3);
  scripti->ival = skip_local;
  RenderMenu(skip_local);
}

/* ------------------ ScriptMakeMovie ------------------------ */

void ScriptMakeMovie(scriptdata *scripti){
  // TODO: there will be an allocation issue here.
  if(scripti->cval != NULL && scripti->cval2 != NULL){
    strcpy(movie_name, scripti->cval);
    strcpy(render_file_base, scripti->cval2);
    movie_framerate = scripti->fval;
    RenderCB(MAKE_MOVIE);
  }
}

/* ------------------ ScriptLoadParticles ------------------------ */

void ScriptLoadParticles(scriptdata *scripti){
  PRINTF("script: loading particles files\n\n");
  LoadParticleMenu(PARTFILE_LOADALL);
}

/* ------------------ ScriptLoadIso ------------------------ */

void ScriptLoadIso(scriptdata *scripti, int meshnum){
  int i;
  int count=0;

  THREADcontrol(isosurface_threads, THREAD_JOIN);
  PRINTF("script: loading isosurface files of type: %s\n\n",scripti->cval);

  update_readiso_geom_wrapup = UPDATE_ISO_START_ALL;
  CancelUpdateTriangles();
  for(i = global_scase.nisoinfo - 1; i >= 0; i--){
    isodata* isoi;

    isoi = global_scase.isoinfo + i;
    isoi->finalize = 0;
  }
  for(i = global_scase.nisoinfo-1; i >=0; i--){
    isodata* isoi;
    char label2[100];
    int lencval, lenlabel;

    isoi = global_scase.isoinfo + i;
    if(meshnum != -1 && isoi->blocknumber + 1 != meshnum)continue;
    lencval = 0;
    if(scripti->cval!=NULL)lencval = strlen(scripti->cval);
    lenlabel = strlen(isoi->surface_label.longlabel);
    if(lencval <= lenlabel){
      strncpy(label2, isoi->surface_label.longlabel, lencval);
      label2[lencval] = 0;
      if(STRCMP(label2, scripti->cval) == 0){
        isoi->finalize = 1;
      }
    }
  }
  for(i = 0; i<global_scase.nisoinfo; i++){
    int errorcode;
    isodata *isoi;
    char label2[100];
    int lencval, lenlabel;

    isoi = global_scase.isoinfo + i;
    if(meshnum != -1 && isoi->blocknumber+1 != meshnum)continue;
    lencval = 0;
    if(scripti->cval!=NULL)lencval = strlen(scripti->cval);
    lenlabel = strlen(isoi->surface_label.longlabel);
    if(lencval<=lenlabel){
      strncpy(label2, isoi->surface_label.longlabel, lencval);
      label2[lencval] = 0;
      if(STRCMP(label2, scripti->cval)==0){
        ReadIso(isoi->file, i, LOAD, NULL, &errorcode);
        count++;
      }
    }
  }
  if(update_readiso_geom_wrapup == UPDATE_ISO_ALL_NOW)ReadIsoGeomWrapup(FOREGROUND);
  update_readiso_geom_wrapup = UPDATE_ISO_OFF;
  if(count == 0){
    fprintf(stderr, "*** Error: Isosurface files of type %s failed to load\n", scripti->cval);
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: Isosurface files of type %s failed to load\n", scripti->cval);
  }
  force_redisplay=1;
  updatemenu=1;
}

/* ------------------ ScriptLoadVolSmoke ------------------------ */

void ScriptLoadVolSmoke(scriptdata *scripti){
  int imesh;

  imesh = scripti->ival;
  if(imesh==-1){
    read_vol_mesh=VOL_READALL;
    ReadVolsmokeAllFramesAllMeshes2(NULL);
  }
  else if(imesh>=0&&imesh<global_scase.meshescoll.nmeshes){
    meshdata *meshi;
    volrenderdata *vr;

    meshi = global_scase.meshescoll.meshinfo + imesh;
    vr = meshi->volrenderinfo;
    ReadVolsmokeAllFrames(vr);
  }
}

/* ------------------ ScriptLoad3dSmoke ------------------------ */

void ScriptLoad3dSmoke(scriptdata *scripti){
  int i;
  int errorcode;
  int count=0;

  PRINTF("script: loading smoke3d files of type: %s\n\n",scripti->cval);
  for(i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    smoke3di->finalize = 0;
  }
  for(i = global_scase.smoke3dcoll.nsmoke3dinfo - 1; i >= 0; i--){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(MatchUpper(smoke3di->label.longlabel, scripti->cval) == MATCH){
      smoke3di->finalize = 1;
      break;
    }
  }
  for(i=0;i<global_scase.smoke3dcoll.nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(MatchUpper(smoke3di->label.longlabel,scripti->cval) == MATCH){
      ReadSmoke3D(ALL_SMOKE_FRAMES, i, LOAD, FIRST_TIME, &errorcode);
      count++;
    }
  }
  if(count == 0){
    fprintf(stderr, "*** Error: Smoke3d files of type %s failed to load\n", scripti->cval);
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: Smoke3d files of type %s failed to load\n", scripti->cval);
  }
  force_redisplay=1;
  updatemenu=1;

}

/* ------------------ SliceMatch ------------------------ */

int SliceMatch(scriptdata *scripti, slicedata *slicei){

  if(scripti->id!=NULL){
    // not a valid slice if there is no slicelabel or it does not match ID
    if(slicei->slicelabel==NULL||MatchUpper(slicei->slicelabel, scripti->id)==NOTMATCH)return 0;

    // if QUANTITY is not specified it CANNOT be a velocity slice
    if(scripti->quantity==NULL){
      if(
        strcmp(slicei->label.shortlabel, "U_VEL")==0||
        strcmp(slicei->label.shortlabel, "V_VEL")==0||
        strcmp(slicei->label.shortlabel, "W_VEL")==0){
        return 0;
      }
    }
    else{
    // if QUANTITY is specified then it has to match slice's quantity
      if(strncmp(scripti->quantity, slicei->label.longlabel,strlen(scripti->quantity))!=0)return 0;
    }
    return 1;
  }

  // ID was not specified so slice has to match QUANTITY, direction and position and type (cell or node cenetered)
  assert(scripti->quantity!=NULL);
  if(scripti->quantity==NULL)return 0;  // should never happen
  if(scripti->quantity!=NULL&&strncmp(scripti->quantity, slicei->label.longlabel,strlen(scripti->quantity))!=0)return 0;
  if(scripti->cell_centered==0&&slicei->slice_filetype==SLICE_CELL_CENTER)return 0;
  if(scripti->cell_centered==1&&slicei->slice_filetype!=SLICE_CELL_CENTER)return 0;
  if(scripti->pbxyz_dir==0){
    int *min, *max;
    meshdata *meshi;

    if(slicei->volslice==0)return 0;                                              // need a 3d slice file but didn't find it

    min = slicei->ijk_min;
    max = slicei->ijk_max;

    if(slicei->slice_filetype==SLICE_CELL_CENTER){
      if(min[0]>1||min[1]>1||min[2]>1)return 0;
    }
    else{
      if(min[0]!=0||min[1]!=0||min[2]!=0)return 0;
    }
    meshi = global_scase.meshescoll.meshinfo+slicei->blocknumber;
    if(max[0]!=meshi->ibar||max[1]!=meshi->jbar||max[2]!=meshi->kbar)return 0;
  }
  else{
    if(slicei->slice_filetype==SLICE_TERRAIN){
      if(scripti->c_pbxyz != NULL && strcmp(scripti->c_pbxyz, "AGL_SLICE")!=0)return 0;
      if(ABS(slicei->above_ground_level-scripti->pbxyz_val)>slicei->delta_orig)return 0;
    }
    else{
      if(slicei->idir!=scripti->pbxyz_dir)return 0;                                 // not a 3d slice and directions don't match
      if(ABS(slicei->position_orig-scripti->pbxyz_val)>slicei->delta_orig)return 0; // not a 3d slice and positions don't match
      if(scripti->cell_centered==1&&slicei->slice_filetype!=SLICE_CELL_CENTER)return 0;
      if(scripti->cell_centered==0&&slicei->slice_filetype!=SLICE_NODE_CENTER)return 0;
    }
  }
  // passed all the test so draw this lice
  return 1;
}

/* ------------------ ScriptLoadVSLCF ------------------------ */

void ScriptLoadVSLCF(scriptdata *scripti){
  int i;
  int count=0;

  PRINTF("script: loading vector slice file with:");
  if(scripti->id!=NULL)PRINTF(" ID=%s", scripti->id);
  if(scripti->quantity!=NULL)PRINTF(" QUANTITY=%s", scripti->quantity);
  if(scripti->c_pbxyz!=NULL){
    PRINTF(" %s=%f", scripti->c_pbxyz, scripti->pbxyz_val);
  }
  if(scripti->cell_centered==1){
    PRINTF(" CELL_CENTERED=T");
  }
  if(scripti->vector==1){
    PRINTF(" VECTOR=T");
  }
  printf("\n");

  for(i=0;i<global_scase.slicecoll.nmultivsliceinfo;i++){
    multivslicedata *mvslicei;
    vslicedata *vslicei;
    int j;
    slicedata *slicei;

    mvslicei = global_scase.slicecoll.multivsliceinfo + i;
    if(mvslicei->nvslices<=0)continue;
    vslicei = global_scase.slicecoll.vsliceinfo + mvslicei->ivslices[0];
    slicei = global_scase.slicecoll.sliceinfo + vslicei->ival;

    if(SliceMatch(scripti, slicei)==0)continue;

    for(j=0;j<mvslicei->nvslices;j++){
      vslicedata *vslicej;
      int finalize_save;

      vslicej = global_scase.slicecoll.vsliceinfo+mvslicei->ivslices[j];
//save finalize
      finalize_save = vslicej->finalize;
      if(j==mvslicei->nvslices-1){
        vslicej->finalize = 1;
      }
      else{
        vslicej->finalize = 0;
      }
      LoadVSliceMenu(mvslicei->ivslices[j]);
      vslicej->finalize = finalize_save;
      count++;
    }
    break;
  }
  if(count == 0){
    fprintf(stderr, "*** Error: Vector slice files of type %s failed to load\n", scripti->cval);
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: Vector slice files of type %s failed to load\n", scripti->cval);
  }
}

/* ------------------ ScriptUnLoadPlot2D ------------------------ */

void ScriptUnLoadPlot2D(scriptdata *scripti){
  int i;

  for(i=0;i<nplot2dinfo;i++){
    plot2ddata *plot2di;

    plot2di = plot2dinfo + i;
    plot2di->show = 0;
  }
}

/* ------------------ ScriptLoadSLCF ------------------------ */

void ScriptLoadSLCF(scriptdata *scripti){
  int i;
  int count = 0;
  int count2=0;

  if(scripti->vector==1){
    ScriptLoadVSLCF(scripti);
    return;
  }

  PRINTF("script: loading slice file with: ");
  if(scripti->id!=NULL){
    if(count2++!=0)printf(", ");
    PRINTF("ID=%s", scripti->id);
  }
  if(scripti->quantity!=NULL){
    if(count2++!=0)printf(", ");
    PRINTF("QUANTITY=%s", scripti->quantity);
  }
  if(scripti->c_pbxyz!=NULL){
    if(count2++!=0)printf(", ");
    if(strcmp(scripti->c_pbxyz,"PB3D")==0){
      PRINTF("PB3D=T");
    }
    else{
      PRINTF("%s=%f", scripti->c_pbxyz, scripti->pbxyz_val);
    }
  }
  if(scripti->cell_centered==1){
    if(count2++!=0)printf(", ");
    PRINTF("CELL_CENTERED=T");
  }
  if(scripti->vector==1){
    if(count2++!=0)printf(", ");
    PRINTF("VECTOR=T");
  }
  printf("\n");

  for(i = 0; i<global_scase.slicecoll.nsliceinfo; i++){
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo+i;
    slicei->finalize = 0;
  }
  for(i = global_scase.slicecoll.nsliceinfo-1; i>=0; i--){
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo+i;
    if(SliceMatch(scripti, slicei)==0)continue;
    slicei->finalize = 1;
    break;
  }

  for(i = 0; i<global_scase.slicecoll.nsliceinfo; i++){
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo+i;
    if(SliceMatch(scripti, slicei)==0)continue;

    LoadSliceMenu(i);
    count++;
    if(slicei->finalize==1)break;
  }
  if(count==0){
    fprintf(stderr, "*** Error: Slice files of type %s failed to load\n", scripti->cval);
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: Slice files of type %s failed to load\n", scripti->cval);
  }
}

/* ------------------ ScriptLoadSlice ------------------------ */

void ScriptLoadSlice(scriptdata *scripti){
  int i;
  int count=0;

  PRINTF("script: loading slice files of type: %s\n\n",scripti->cval);

  for(i=0;i<global_scase.slicecoll.nmultisliceinfo;i++){
    multislicedata *mslicei;
    slicedata *slicei;
    int j;

    mslicei = global_scase.slicecoll.multisliceinfo + i;
    if(mslicei->nslices<=0)continue;
    slicei = global_scase.slicecoll.sliceinfo + mslicei->islices[0];
    if(MatchUpper(slicei->label.longlabel,scripti->cval) == NOTMATCH)continue;
    if(scripti->ival==0){
      if(slicei->volslice==0)continue;
    }
    else{
      if(slicei->idir != scripti->ival)continue;
      if(ABS(slicei->position_orig - scripti->fval) > slicei->delta_orig)continue;
    }
    for(j=0;j<mslicei->nslices;j++){
      slicedata *slicej;
      int finalize_save;

      slicej = global_scase.slicecoll.sliceinfo+mslicei->islices[j];
//save finalize
      finalize_save = slicej->finalize;
      if(j==mslicei->nslices-1){
        slicej->finalize = 1;
      }
      else{
        slicej->finalize = 0;
      }
      LoadSliceMenu(mslicei->islices[j]);
      slicej->finalize = finalize_save;
      count++;
    }
    break;
  }
  if(count == 0){
    fprintf(stderr, "*** Error: Slice files of type %s failed to load\n", scripti->cval);
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: Slice files of type %s failed to load\n", scripti->cval);
  }
}

/* ------------------ SetSliceGlobalBounds ------------------------ */

void SetSliceGlobalBounds(char *type){
  int slice_index;

  slice_index = GetSliceBoundsIndexFromLabel(type);
  if(slice_index>=0&&
    (slicebounds[slice_index].dlg_setvalmin!=SET_MIN||slicebounds[slice_index].dlg_setvalmax!=SET_MAX)
    ){
    int i;
    float valmin = 1000000000.0, valmax = -1000000000.0;

    for(i = 0; i<global_scase.slicecoll.nsliceinfo; i++){
      slicedata *slicei;
      char *slice_type;
      FILE *stream;

      slicei = global_scase.slicecoll.sliceinfo+i;
      slice_type = slicei->label.shortlabel;
      if(strcmp(type, slice_type)!=0)continue;
      stream = FOPEN(slicei->bound_file, "r");
      if(stream==NULL)continue;
      for(;;){
        char buffer[255];
        float time, smin, smax;

        if(fgets(buffer, 255, stream)==NULL)break;
        sscanf(buffer, "%f %f %f", &time, &smin, &smax);
        valmin = MIN(smin, valmin);
        valmax = MAX(smax, valmax);
      }
      fclose(stream);
    }
    if(slicebounds[slice_index].dlg_setvalmin!=SET_MIN){
      slicebounds[slice_index].dlg_setvalmin = SET_MIN;
      slicebounds[slice_index].dlg_valmin = valmin;
    }
    if(slicebounds[slice_index].dlg_setvalmax!=SET_MAX){
      slicebounds[slice_index].dlg_setvalmax = SET_MAX;
      slicebounds[slice_index].dlg_valmax = valmax;
    }
  }
}

/* ------------------ GetNSliceFrames ------------------------ */

int GetNSliceGeomFrames(scriptdata *scripti){
  int nframes = -1;
  int i;

  nframes = -1;
  for(i = 0; i<global_scase.slicecoll.nmultisliceinfo; i++){
    multislicedata *mslicei;
    slicedata *slicei;
    int j;

    mslicei = global_scase.slicecoll.multisliceinfo+i;
    if(mslicei->nslices<=0)continue;
    slicei = global_scase.slicecoll.sliceinfo+mslicei->islices[0];
    if(MatchUpper(slicei->label.longlabel, scripti->cval)==NOTMATCH)continue;
    if(scripti->ival==0){
      if(slicei->volslice==0)continue;
    }
    else{
      if(slicei->idir!=scripti->ival)continue;
      if(ABS(slicei->position_orig-scripti->fval)>slicei->delta_orig)continue;
    }

 // determine number of time frames

    for(j = 0; j<mslicei->nslices; j++){
      slicei = global_scase.slicecoll.sliceinfo+mslicei->islices[j];
      if(slicei->nframes==0){
        if(slicei->slice_filetype==SLICE_GEOM){
          int nvals, error;

          slicei->nframes = GetGeomDataSize(slicei->file, &nvals, ALL_FRAMES, NULL, NULL, NULL, NULL, NULL, &error);
        }
        else{
          slicei->nframes = GetNSliceFrames(slicei->file, &scripti->fval2, &scripti->fval3);
        }
      }
      if(nframes==-1){
        nframes = slicei->nframes;
      }
      else{
        nframes = MIN(nframes, slicei->nframes);
      }
    }
  }
  return nframes;
}

/* ------------------ ScriptLoadSliceRender ------------------------ */

void ScriptLoadSliceRender(scriptdata *scripti){
  int i;
  int count = 0;
  int frame_start, frame_skip, frame_current;
  int valid_frame = 1;

  frame_start = scripti->ival2;
  frame_skip = scripti->ival3;

  if(scripti->first==1){
    char *shortlabel = NULL;

    PRINTF("startup time: %f\n", timer_startup);
    PRINTF("script: loading slice files of type: %s\n", scripti->cval);
    PRINTF("  frames: %i,%i,%i,... \n\n", frame_start, frame_start+frame_skip, frame_start+2*frame_skip);
    scripti->first = 0;
    scripti->exit = 0;
    frame_current = frame_start;
    for(i = 0; i<global_scase.slicecoll.nsliceinfo; i++){
      slicedata *slicei;

      slicei = global_scase.slicecoll.sliceinfo+i;
      if(strcmp(slicei->label.longlabel, scripti->cval)==0){
        shortlabel = slicei->label.shortlabel;
        break;
      }
    }
    if(shortlabel!=NULL){
      SetSliceGlobalBounds(shortlabel);
    }
    frames_total = GetNSliceGeomFrames(scripti);
    frame360 = 0;
  }
  else{
    frame_current = scripti->ival4;
    if(frame_current==frame_start&&frame360==0&&render_mode==RENDER_360){ // output first frame twice - work around for a bug causing first frame to be output incorrectly
      frame360 = 1;
    }
    else{
      frame_current += frame_skip;
    }
  }
  script_itime = frame_current;
  script_render_flag = 1;
  scripti->ival4 = frame_current;

  if(frame_current<frames_total){
    PRINTF("\nFrame: %i of %i, ", frame_current, frames_total);
  }

  for(i = 0; i<global_scase.slicecoll.nmultisliceinfo; i++){
    multislicedata *mslicei;
    slicedata *slicei;
    int j;

    mslicei = global_scase.slicecoll.multisliceinfo+i;
    if(mslicei->nslices<=0)continue;
    slicei = global_scase.slicecoll.sliceinfo+mslicei->islices[0];
    if(MatchUpper(slicei->label.longlabel, scripti->cval)==NOTMATCH)continue;
    if(scripti->ival==0){
      if(slicei->volslice==0)continue;
    }
    else{
      if(slicei->idir!=scripti->ival)continue;
      if(ABS(slicei->position_orig-scripti->fval)>slicei->delta_orig)continue;
    }

    float slice_load_time = 0.0;
    FILE_SIZE total_slice_size = 0.0;

    SetLoadedSliceBounds(mslicei->islices, mslicei->nslices);

    START_TIMER(slice_load_time);
    GLUTSETCURSOR(GLUT_CURSOR_WAIT);

    // load slice data

    for(j = 0; j<mslicei->nslices; j++){
      slicedata *slicej;
      int finalize_save;
      float time_value;
      FILE_SIZE slicefile_size;

      slicej = global_scase.slicecoll.sliceinfo+mslicei->islices[j];
//save finalize
      finalize_save = slicej->finalize;
      if(j==mslicei->nslices-1){
        slicej->finalize = 1;
      }
      else{
        slicej->finalize = 0;
      }
      if(frame_current>=frames_total){
        scripti->exit = 1;
        valid_frame = 0;
        RenderState(RENDER_OFF);
        break;
      }

      FILE_SIZE LoadSlicei(int set_slicecolor, int value, int time_frame, float *time_value);
      slicefile_size = LoadSlicei(SET_SLICECOLOR, mslicei->islices[j], frame_current, &time_value);

      if(slicefile_size==0){
        scripti->exit = 1;
        valid_frame = 0;
        RenderState(RENDER_OFF);
        break;
      }
      total_slice_size += slicefile_size;
      scripti->fval4 = time_value;
      CheckMemory;

//save finalize
      slicej->finalize = finalize_save;
      count++;
    }
    GLUTPOSTREDISPLAY;
    GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
    updatemenu = 1;
    STOP_TIMER(slice_load_time);


    if(frame_current<frames_total){
      PRINTF("files: %i, ", count);
      if(total_slice_size>1000000000){
        PRINTF("file size: %.1f GB, load time: %.1f s\n", (float)total_slice_size/1000000000., slice_load_time);
      }
      else if(total_slice_size>1000000){
        PRINTF("file size: %.1f MB, load time: %.1f s\n", (float)total_slice_size/1000000., slice_load_time);
      }
      else{
        PRINTF("file size: %.0f KB, load time: %.1f s\n", (float)total_slice_size/1000., slice_load_time);
      }
    }
    break;
  }
  if(valid_frame==1&&count==0){
    fprintf(stderr,  "*** Error: Slice files of type %s, frame %i failed to load\n", scripti->cval, frame_current);
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: Slice files of type %s, frame %i failed to load\n", scripti->cval, frame_current);
    scripti->exit = 1;
    RenderState(RENDER_OFF);
  }
}

/* ------------------ GetSmokeType ------------------------ */

int GetSmokeType(char *smoke_type, char *smoke_type2){
  int type = 0;

  if(smoke_type != NULL){
    if(strcmp(smoke_type, "soot")   == 0)type |= 1;
    if(strcmp(smoke_type, "hrrpuv") == 0)type |= 2;
    if(strcmp(smoke_type, "temp")   == 0)type |= 4;
    if(strcmp(smoke_type, "co2")    == 0)type |= 8;
  }
  if(smoke_type2 != NULL){
    if(strcmp(smoke_type2, "soot")   == 0)type |= 1;
    if(strcmp(smoke_type2, "hrrpuv") == 0)type |= 2;
    if(strcmp(smoke_type2, "temp")   == 0)type |= 4;
    if(strcmp(smoke_type2, "co2")    == 0)type |= 8;
  }
  return type;
}

/* ------------------ ScriptLoadSliceRender ------------------------ */

void ScriptLoadSmokeRender(scriptdata *scripti){
  int count = 0;
  int frame_start, frame_skip, frame_current;
  int valid_frame = 1;
  char *smoke_type, *smoke_type2;

  frame_start = scripti->ival2;
  frame_skip  = scripti->ival3;
  smoke_type  = scripti->cval;
  smoke_type2 = scripti->cval3;

  if(scripti->first==1){
    PRINTF("startup time: %f\n", timer_startup);
    PRINTF("script: loading 3D smoke files of type: %s\n", scripti->cval);
    PRINTF("  frames: %i,%i,%i,... \n\n", frame_start, frame_start+frame_skip, frame_start+2*frame_skip);
    scripti->first = 0;
    scripti->exit = 0;
    frame_current = frame_start;
    frame360 = 0;
  }
  else{
    frame_current = scripti->ival4;
    if(frame_current==frame_start&&frame360==0&&render_mode==RENDER_360){ // output first frame twice - work around for a bug causing first frame to be output incorrectly
      frame360 = 1;
    }
    else{
      frame_current += frame_skip;
    }
  }
  script_itime       = frame_current;
  script_render_flag = 1;
  scripti->ival4     = frame_current;
  int type;

  type = GetSmokeType(smoke_type, smoke_type2);
  if(scripti->fval2>scripti->fval3){
    frames_total = GetSmokeNFrames(type, &scripti->fval2, &scripti->fval3);
  }

  if(frame_current<frames_total){
    PRINTF("\nFrame: %i of %i, ", frame_current, frames_total);
  }

  float smoke_load_time;
  START_TIMER(smoke_load_time);
  GLUTSETCURSOR(GLUT_CURSOR_WAIT);

    // load smoke data

  if(frame_current>=frames_total){
    scripti->exit = 1;
    valid_frame = 0;
    RenderState(RENDER_OFF);
  }

  FILE_SIZE total_smokefile_size;
  float time_value;
  total_smokefile_size = LoadSmoke3D(type, frame_current, &count, &time_value);
  scripti->fval4 = time_value;

  if(total_smokefile_size ==0){
    scripti->exit = 1;
    valid_frame = 0;
    RenderState(RENDER_OFF);
  }
  CheckMemory;

//save finalize
  GLUTPOSTREDISPLAY;
  GLUTSETCURSOR(GLUT_CURSOR_LEFT_ARROW);
  updatemenu = 1;
  STOP_TIMER(smoke_load_time);


  if(frame_current<frames_total){
    PRINTF("files: %i, ", count);
    if(total_smokefile_size >1000000000){
      PRINTF("file size: %.1f GB, load time: %.1f s\n", (float)total_smokefile_size /1000000000., smoke_load_time);
    }
    else if(total_smokefile_size >1000000){
      PRINTF("file size: %.1f MB, load time: %.1f s\n", (float)total_smokefile_size /1000000., smoke_load_time);
    }
    else{
      PRINTF("file size: %.0f KB, load time: %.1f s\n", (float)total_smokefile_size /1000., smoke_load_time);
    }
  }
  if(valid_frame==1&&count==0){
    fprintf(stderr,  "*** Error: 3D smoke files of type %s, frame %i failed to load\n", scripti->cval, frame_current);
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: Slice files of type %s, frame %i failed to load\n", scripti->cval, frame_current);
    scripti->exit = 1;
    RenderState(RENDER_OFF);
  }
}

/* ------------------ ScriptLoadSliceM ------------------------ */

void ScriptLoadSliceM(scriptdata *scripti, int meshnum){
  int i;

  PRINTF("script: loading slice files of type: %s in mesh %i\n\n", scripti->cval,meshnum);

  for(i = 0; i < global_scase.slicecoll.nsliceinfo; i++){
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo + i;
    if(slicei->blocknumber + 1 != meshnum)continue;
    if(MatchUpper(slicei->label.longlabel, scripti->cval) == NOTMATCH)continue;
    if(scripti->ival == 0){
      int *min, *max;
      meshdata *meshi;

      if(slicei->volslice == 0)continue;
      min = slicei->ijk_min;
      max = slicei->ijk_max;
      if(min[0] != 0 || min[1] != 0 || min[2] != 0)continue;
      meshi = global_scase.meshescoll.meshinfo + slicei->blocknumber;
      if(max[0] != meshi->ibar || max[1] != meshi->jbar || max[2] != meshi->kbar)continue;
    }
    else{
      if(slicei->idir != scripti->ival)continue;
      if(ABS(slicei->position_orig - scripti->fval) > slicei->delta_orig)continue;
    }
    LoadSliceMenu(i);
  }
}

/* ------------------ ScriptLoadVSlice ------------------------ */

void ScriptLoadVSlice(scriptdata *scripti){
  int i;
  int count=0;

  PRINTF("script: loading vector slice files of type: %s\n\n",scripti->cval);

  for(i=0;i<global_scase.slicecoll.nmultivsliceinfo;i++){
    multivslicedata *mvslicei;
    vslicedata *vslicei;
    int j;
    slicedata *slicei;

    mvslicei = global_scase.slicecoll.multivsliceinfo + i;
    if(mvslicei->nvslices<=0)continue;
    vslicei = global_scase.slicecoll.vsliceinfo + mvslicei->ivslices[0];
    slicei = global_scase.slicecoll.sliceinfo + vslicei->ival;
    if(MatchUpper(slicei->label.longlabel,scripti->cval) == NOTMATCH)continue;
    if(scripti->ival == 0){
      if(slicei->volslice == 0)continue;
    }
    else{
      if(slicei->idir != scripti->ival)continue;
      if(ABS(slicei->position_orig - scripti->fval) > slicei->delta_orig)continue;
    }
    for(j=0;j<mvslicei->nvslices;j++){
      vslicedata *vslicej;
      int finalize_save;

      vslicej = global_scase.slicecoll.vsliceinfo+mvslicei->ivslices[j];
//save finalize
      finalize_save = vslicej->finalize;
      if(j==mvslicei->nvslices-1){
        vslicej->finalize = 1;
      }
      else{
        vslicej->finalize = 0;
      }
      LoadVSliceMenu(mvslicei->ivslices[j]);
      vslicej->finalize = finalize_save;
      count++;
    }
    break;
  }
  if(count == 0){
    fprintf(stderr, "*** Error: Vector slice files of type %s failed to load\n", scripti->cval);
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: Vector slice files of type %s failed to load\n", scripti->cval);
  }
}

/* ------------------ ScriptLoadVSliceM ------------------------ */

void ScriptLoadVSliceM(scriptdata *scripti, int meshnum){
  int i;
  int count=0;

  PRINTF("script: loading vector slice files of type: %s in mesh %i\n\n", scripti->cval,meshnum);

  for(i=0;i<global_scase.slicecoll.nmultivsliceinfo;i++){
    multivslicedata *mvslicei;
    vslicedata *vslicei;
    int j;
    slicedata *slicei;

    mvslicei = global_scase.slicecoll.multivsliceinfo + i;
    if(mvslicei->nvslices<=0)continue;
    vslicei = global_scase.slicecoll.vsliceinfo + mvslicei->ivslices[0];
    slicei = global_scase.slicecoll.sliceinfo + vslicei->ival;
    if(slicei->blocknumber + 1 != meshnum)continue;
    if(MatchUpper(slicei->label.longlabel,scripti->cval) == NOTMATCH)continue;
    if(scripti->ival == 0){
      if(slicei->volslice == 0)continue;
    }
    else{
      if(slicei->idir != scripti->ival)continue;
      if(ABS(slicei->position_orig - scripti->fval) > slicei->delta_orig)continue;
    }
    for(j=0;j<mvslicei->nvslices;j++){
      LoadVSliceMenu(mvslicei->ivslices[j]);
      count++;
    }
    break;
  }
  if(count == 0){
    fprintf(stderr, "*** Error: Vector slice files of type %s in mesh %i failed to load\n", scripti->cval, meshnum);
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: Vector slice files of type %s in mesh %i failed to load\n", scripti->cval, meshnum);
  }
}

/* ------------------ ScriptLoadTour ------------------------ */

void ScriptLoadTour(scriptdata *scripti){
  int i;
  int count=0;

  PRINTF("script: loading tour %s\n\n",scripti->cval);

  for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
    tourdata *touri;

    touri = global_scase.tourcoll.tourinfo + i;
    if(scripti->cval!=NULL&&strcmp(touri->label,scripti->cval)==0){
      TourMenu(i);
      viewtourfrompath=0;
      TourMenu(MENU_TOUR_VIEWFROMROUTE);
      count++;
      break;
    }
  }

  if(count == 0){
    fprintf(stderr, "*** Error: The tour %s failed to load\n", scripti->cval);
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: The tour %s failed to load\n", scripti->cval);
  }
  force_redisplay=1;
  updatemenu=1;
}

/* ------------------ ScriptLoadBoundary ------------------------ */

void ScriptLoadBoundary(scriptdata *scripti, int meshnum){
  int i;
  int errorcode;
  int count=0;

  PRINTF("Script: loading boundary files of type: %s\n\n",scripti->cval);
  for(i=0;i<global_scase.npatchinfo;i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo + i;
    if(meshnum == -1 || patchi->blocknumber + 1 == meshnum){
      if(scripti->cval != NULL && strcmp(patchi->label.longlabel, scripti->cval) == 0){
        THREADcontrol(compress_threads, THREAD_LOCK);
        ReadBoundary(i, LOAD, &errorcode);
        count++;
        THREADcontrol(compress_threads, THREAD_UNLOCK);
        if(meshnum != -1)break;
      }
    }
  }
  if(count == 0){
    fprintf(stderr, "*** Error: Boundary files of type %s failed to load\n", scripti->cval);
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: Boundary files of type %s failed to load\n", scripti->cval);
  }
  force_redisplay=1;
  updatemenu=1;
  UpdateFrameNumber(0);

}

/* ------------------ ScriptPartClassColor ------------------------ */

void ScriptPartClassColor(scriptdata *scripti){
  int i;

  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(scripti->cval != NULL && strcmp(propi->label->longlabel,scripti->cval)==0){
      ParticlePropShowMenu(i);
      return;
    }
  }
  fprintf(stderr, "*** Error: particle class quantity: %s failed to be set\n", scripti->cval);
  if(stderr2!=NULL)fprintf(stderr2, "*** Error: particle class color: %s failed to be set\n", scripti->cval);
}


/* ------------------ ScriptPlot3dProps ------------------------ */

void ScriptPlot3dProps(scriptdata *scripti){
  int i, p_index;

  p_index = scripti->ival;
  if(p_index<1)p_index=1;
  if(p_index>5)p_index=5;

  visVector = scripti->ival2;
  if(visVector!=1)visVector=0;

  plotn = p_index;
  if(plotn<1){
    plotn=numplot3dvars;
  }
  if(plotn>numplot3dvars){
    plotn=1;
  }
  UpdateAllPlotSlices();
  if(visiso==1)UpdateSurface();
  GLUIUpdatePlot3dListIndex();

  vecfactor=1.0;
  if(scripti->fval>=0.0)vecfactor=scripti->fval;
  GLUIUpdateVectorWidgets();

  PRINTF("script: vecfactor=%f\n",vecfactor);

  contour_type=CLAMP(scripti->ival4,0,2);
  GLUIUpdatePlot3dDisplay();

  if(visVector==1&&nplot3dloaded>0){
    meshdata *gbsave,*gbi;

    gbsave=current_mesh;
    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      gbi = global_scase.meshescoll.meshinfo + i;
      if(gbi->plot3dfilenum==-1)continue;
      UpdateCurrentMesh(gbi);
      UpdatePlotSlice(XDIR);
      UpdatePlotSlice(YDIR);
      UpdatePlotSlice(ZDIR);
    }
    UpdateCurrentMesh(gbsave);
  }
}

/* ------------------ ScriptShowCbarEdit ------------------------ */

void ScriptShowCbarEdit(scriptdata *scripti){
  DialogMenu(DIALOG_COLORBAR);
}

/* ------------------ ScriptHideCbarEdit ------------------------ */

void ScriptHideCbarEdit(scriptdata *scripti){
  DialogMenu(DIALOG_COLORBAR);
}

/* ------------------ ScriptSetCbarLab ------------------------ */

void ScriptSetCbarLab(scriptdata *scripti){
  colorbar_coord_type=1;
}

/* ------------------ ScriptSetCbarRgb ------------------------ */

void ScriptSetCbarRgb(scriptdata *scripti){
  colorbar_coord_type=0;
}

/* ------------------ ScriptSetCbar ------------------------ */

void ScriptSetCbar(scriptdata *scripti){
  colorbardata *cb;
  int cb_index;

  if(scripti->cval!=NULL){
    cb = GetColorbar(&colorbars, scripti->cval);
    if(cb != NULL){
      cb_index = cb - colorbars.colorbarinfo;
      ColorbarMenu(cb_index);
    }
  }
}

/* ------------------ ScriptSmokeprop ------------------------ */

void ScriptSmokeprop(scriptdata *scripti) {
  glui_mass_extinct = scripti->fval;
  GLUISmoke3dCB(SMOKE_EXTINCT);
}

/* ------------------ ScriptSetDemoMode ------------------------ */

void ScriptSetDemoMode(scriptdata *scripti){
  demo_mode = scripti->ival;
}

/* ------------------ ScriptShowHVACDuctVAL ------------------------ */

void ScriptShowHVACDuctVal(scriptdata *scripti){
  int ductvalindex;

  ductvalindex = GetHVACDuctValIndex(&global_scase.hvaccoll, scripti->cval);
  if(ductvalindex>=0){
    HVACDuctValueMenu(ductvalindex);
  }
  else{
    printf("***warning: %s not a known hvac duct quantity\n", scripti->cval);
  }
}

/* ------------------ ScriptShowHVACNodeVAL ------------------------ */

void ScriptShowHVACNodeVal(scriptdata *scripti){
  int nodevalindex;

  nodevalindex = GetHVACNodeValIndex(&global_scase.hvaccoll, scripti->cval);
  if(nodevalindex>=0){
    HVACNodeValueMenu(nodevalindex);
  }
  else{
    printf("***warning: %s not a known hvac node quantity\n", scripti->cval);
  }
}

/* ------------------ ScriptHideHVACVals ------------------------ */

void ScriptHideHVACVals(void){
  HVACMenu(MENU_HVAC_HIDE_ALL_VALUES);
}

/* ------------------ ScriptOutputSmokeSensors ------------------------ */

void ScriptOutputSmokeSensors(void){
  int i,j;
  FILE *stream_smokesensors;
  int nsmokesensors;
  float sensor_time=0.0;

  // count smokesensors

  nsmokesensors=0;
  for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
    devicedata *devicei;

    devicei = global_scase.devicecoll.deviceinfo + i;
    if(STRCMP(devicei->object->label,"smokesensor")==0)nsmokesensors++;
  }
  if(nsmokesensors == 0)return;

  // first time, create a file to put smokesensor values in

  if(file_smokesensors==NULL){
    NewMemory((void **)&file_smokesensors,strlen(global_scase.fdsprefix)+17+1);
    strcpy(file_smokesensors,global_scase.fdsprefix);
    strcat(file_smokesensors,"_ss.csv");
    stream_smokesensors = FOPEN(file_smokesensors, "w");

    fprintf(stream_smokesensors, "s,");
    for(i = 1;i < nsmokesensors-1;i++){
      fprintf(stream_smokesensors, ",");
    }
    fprintf(stream_smokesensors, "\n");

    j = 0;
    fprintf(stream_smokesensors, "Time,");
    for(i = 0;i < global_scase.devicecoll.ndeviceinfo;i++){
      devicedata *devicei;

      devicei = global_scase.devicecoll.deviceinfo + i;
      if(STRCMP(devicei->object->label, "smokesensor") == 0){
        j++;
        if(j == nsmokesensors){
          fprintf(stream_smokesensors, "%s\n",devicei->deviceID);
        }
        else{
          fprintf(stream_smokesensors, "%s,", devicei->deviceID);
        }
      }
    }
  }
  else{
    stream_smokesensors = FOPEN(file_smokesensors, "a");
  }

  if(global_times!=NULL&&itimes>=0&&itimes<nglobal_times){
    sensor_time = global_times[itimes];
  }
  fprintf(stream_smokesensors,"%f,",sensor_time);
  j = 0;
  for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
    devicedata *devicei;

    devicei = global_scase.devicecoll.deviceinfo + i;
    if(STRCMP(devicei->object->label,"smokesensor")==0){
      j++;
      if(j==nsmokesensors){
        fprintf(stream_smokesensors,"%i\n",devicei->visval);
      }
      else{
        fprintf(stream_smokesensors,"%i,",devicei->visval);
      }
    }
  }
  fclose(stream_smokesensors);
}

/* ------------------ ScriptXYZView ------------------------ */

void ScriptXYZView(float x, float y, float z, float az, float elev){
  use_customview = 0;
  GLUISceneMotionCB(CUSTOM_VIEW);
  GLUIViewpointCB(RESTORE_VIEW);
  glui_xyz_fds[0]      = x;
  glui_xyz_fds[1]      = y;
  glui_xyz_fds[2]      = z;
  customview_azimuth   = az;
  customview_elevation = elev;
  use_customview       = 1;
  GLUISceneMotionCB(CUSTOM_VIEW);
  GLUISceneMotionCB(SET_VIEW_XYZ);
  GLUIUpdatePosView();
}

/* ------------------ ScriptShowPlot3dData ------------------------ */

void ScriptShowPlot3dData(scriptdata *scripti){
  meshdata *meshi;
  int imesh, dir, showhide;
  float val;
  int isolevel;

  imesh = scripti->ival-1;
  if(imesh<0||imesh>global_scase.meshescoll.nmeshes-1)return;

  meshi = global_scase.meshescoll.meshinfo + imesh;
  UpdateCurrentMesh(meshi);

  dir = CLAMP(scripti->ival2,XDIR,ISO);

  plotn=scripti->ival3;

  showhide = scripti->ival4;
  val = scripti->fval;

  switch(dir){
    case XDIR:
      visx_all=showhide;
      iplotx_all=GetGridIndex(val,XDIR,plotx_all,nplotx_all);
      NextXIndex(1,0);
      NextXIndex(-1,0);
      break;
    case YDIR:
      visy_all=showhide;
      iploty_all=GetGridIndex(val,YDIR,ploty_all,nploty_all);
      NextYIndex(1,0);
      NextYIndex(-1,0);
      break;
    case ZDIR:
      visz_all=showhide;
      iplotz_all=GetGridIndex(val,ZDIR,plotz_all,nplotz_all);
      NextZIndex(1,0);
      NextZIndex(-1,0);
      break;
    case ISO:
      isolevel=scripti->ival5;
      plotiso[plotn-1]=isolevel;
      UpdateShowStep(showhide,ISO);
      UpdateSurface();
      updatemenu=1;
      break;
    default:
      assert(FFALSE);
      break;
  }
  UpdatePlotSlice(dir);

}

/* ------------------ ScriptPartClassType ------------------------ */

void ScriptPartClassType(scriptdata *scripti){
  int i;
  int count=0;

  for(i=0;i<npart5prop;i++){
    partpropdata *propi;
    int j;

    propi = part5propinfo + i;
    if(propi->display==0)continue;
    for(j=0;j<global_scase.npartclassinfo;j++){
      partclassdata *partclassj;

      if(propi->class_present[j]==0)continue;
      partclassj = global_scase.partclassinfo + j;
      if(scripti->cval!=NULL&&strcmp(partclassj->name,scripti->cval)==0){
        ParticlePropShowMenu(-10-j);
        count++;
      }
    }
  }
  if(count == 0){
    fprintf(stderr, "*** Error: particle class type %s failed to be set\n", scripti->cval);
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: particle class type %s failed to be set\n", scripti->cval);
  }
}

/* ------------------ ScriptHilightMinMaxVals ------------------------ */

void ScriptHilightMinMaxVals(scriptdata *scripti, int flag){
  switch(flag){
  case 0:
    show_extreme_mindata = scripti->ival;
    if(show_extreme_mindata == 1){
      glui_down_rgb[0] = scripti->ival2;
      glui_down_rgb[1] = scripti->ival3;
      glui_down_rgb[2] = scripti->ival4;
      GLUIUpdateExtremeVals();
    }
    break;
  case 1:
    show_extreme_maxdata = scripti->ival;
    if(show_extreme_maxdata == 1){
      glui_up_rgb[0] = scripti->ival2;
      glui_up_rgb[1] = scripti->ival3;
      glui_up_rgb[2] = scripti->ival4;
      GLUIUpdateExtremeVals();
    }
    break;
  default:
    assert(FFALSE);
    break;
  }
  GLUIUpdateExtreme();
}

/* ------------------ ScriptLoadIniFile ------------------------ */

void ScriptLoadIniFile(scriptdata *scripti){
  PRINTF("script: loading ini file %s\n\n",scripti->cval);
  windowresized=0;
  ReadIni(scripti->cval);
  UpdateDisplay();               // update all variables that need changing
//  if(update_chop_colors == 1){ // only update variables for chopping data
//    update_chop_colors = 0;
//    UpdateChopColors();
//  }
}

/* ------------------ ScriptLoadFile ------------------------ */

void ScriptLoadFile(scriptdata *scripti){
  int i;
  int errorcode;

  PRINTF("script: loading file %s\n\n",scripti->cval);
  for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
    slicedata *sd;

    sd = global_scase.slicecoll.sliceinfo + i;
    if(scripti->cval != NULL && strcmp(sd->file,scripti->cval)==0){
      sd->finalize = 1;
      ReadSlice(sd->file, i, ALL_FRAMES, NULL, LOAD, SET_SLICECOLOR, &errorcode);
      return;
    }
  }
  for(i=0;i<global_scase.npatchinfo;i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo + i;
    if(scripti->cval != NULL && strcmp(patchi->file,scripti->cval)==0){
      patchi->finalize = 1;
      ReadBoundary(i,LOAD,&errorcode);
      return;
    }
  }
  npartframes_max=GetMinPartFrames(PARTFILE_LOADALL);
  for(i=0;i<global_scase.npartinfo;i++){
    partdata *parti;

    parti = global_scase.partinfo + i;
    if(scripti->cval != NULL && strcmp(parti->file,scripti->cval)==0){
      parti->finalize = 1;
      LoadParticleMenu(i);
      return;
    }
  }
  CancelUpdateTriangles();
  for(i=0;i<global_scase.nisoinfo;i++){
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    if(scripti->cval != NULL && strcmp(isoi->file,scripti->cval)==0){
      ReadIso(isoi->file,i,LOAD,NULL,&errorcode);
      if(update_readiso_geom_wrapup == UPDATE_ISO_ONE_NOW)ReadIsoGeomWrapup(FOREGROUND);
      return;
    }
  }
  for(i=0;i<global_scase.smoke3dcoll.nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(scripti->cval!=NULL&&strcmp(smoke3di->file,scripti->cval)==0){
      smoke3di->finalize = 1;
      smoke3di->finalize = 1;
      ReadSmoke3D(ALL_SMOKE_FRAMES, i, LOAD, FIRST_TIME, &errorcode);
      return;
    }
  }
  for(i=0;i<global_scase.nzoneinfo;i++){
    zonedata *zonei;

    zonei = global_scase.zoneinfo + i;
    if(scripti->cval != NULL && strcmp(zonei->file,scripti->cval)==0){
      ReadZone(i,LOAD,&errorcode);
      return;
    }
  }
  for(i=0;i<global_scase.nplot3dinfo;i++){
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo + i;
    if(scripti->cval!=NULL&&strcmp(plot3di->file,scripti->cval)==0){
      plot3di->finalize = 1;
      ReadPlot3D(plot3di->file,i,LOAD,&errorcode);
      UpdateMenu();
      return;
    }
  }

  fprintf(stderr,"*** Error: file %s failed to load\n",scripti->cval);
  if(stderr2!=NULL)fprintf(stderr2, "*** Error: file %s failed to load\n", scripti->cval);
}

/* ------------------ ScriptLabel ------------------------ */

void ScriptLabel(scriptdata *scripti){
  if(scripti->cval!=NULL&&strlen(scripti->cval)>0){
    PRINTF("*******************************\n");
    PRINTF("*** %s ***\n",scripti->cval);
    PRINTF("*******************************\n");
  }
}

/* ------------------ GetPlot3DTimeIndex ------------------------ */

int GetPlot3DTimeIndex(float time){
  int i;
  int index;

  if(plot3dorderindex == NULL){
    NewMemory((void **)&plot3dorderindex, sizeof(int) * global_scase.nplot3dinfo);
    for(i = 0;i < global_scase.nplot3dinfo;i++){
      plot3dorderindex[i] = i;
    }
    int Plot3dCompare(const void *arg1, const void *arg2);
    qsort((int *)plot3dorderindex, (size_t)global_scase.nplot3dinfo, sizeof(int), Plot3dCompare);
  }
  index = 0;
  for(i = 0;i < global_scase.nplot3dinfo;i++){
    int ii;
    plot3ddata *plot3di;

    ii = plot3dorderindex[i];
    plot3di = global_scase.plot3dinfo + ii;
    if(i == 0){
      if(ABS(time - plot3di->time) < 0.1)return 0;
    }
    else{
      plot3ddata *plot3dim1;

      plot3dim1 = global_scase.plot3dinfo + plot3dorderindex[i - 1];
      if(ABS(plot3di->time - plot3dim1->time) > 0.1){
        index++;
        if(ABS(time - plot3di->time) < 0.1)return index;
      }
    }
  }
  return index;
}

/* ------------------ ScriptLoadPlot3d ------------------------ */

void ScriptLoadPlot3D(scriptdata *scripti){
  int i;
  float time_local;
  int blocknum;
  int count=0;

  time_local = scripti->fval;
  blocknum = scripti->ival-1;

  if(blocknum >= 0){
    for(i = 0;i < global_scase.nplot3dinfo;i++){
      plot3ddata *plot3di;

      plot3di = global_scase.plot3dinfo + i;
      if(plot3di->blocknumber == blocknum && ABS(plot3di->time - time_local) < 0.5){
        count++;
        LoadPlot3dMenu(i);
      }
    }
  }
  else{
    int index;

    index = GetPlot3DTimeIndex(time_local);
    LoadPlot3dMenu(-100000+index);
    count = 0;
    for(i = 0;i < global_scase.nplot3dinfo;i++){
      plot3ddata *plot3di;

      plot3di = global_scase.plot3dinfo + i;
      if(plot3di->loaded == 1)count++;
    }
  }
  UpdateRGBColors(colorbar_select_index);
  GLUISetLabelControls();
  if(count == 0){
    fprintf(stderr, "*** Error: Plot3d file failed to load\n");
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: Plot3d file failed to load\n");
  }

  //UpdateMenu();
}

/* ------------------ ScriptLoadVecFile ------------------------ */

void ScriptLoadVecFile(scriptdata *scripti){
  int i;

  PRINTF("script: loading vector slice file %s\n\n",scripti->cval);
  for(i=0;i<global_scase.slicecoll.nvsliceinfo;i++){
    slicedata *val;
    vslicedata *vslicei;

    vslicei = global_scase.slicecoll.vsliceinfo + i;
    val = global_scase.slicecoll.sliceinfo + vslicei->ival;
    if(val==NULL)continue;
    if(scripti->cval!=NULL&&strcmp(val->reg_file,scripti->cval)==0){
      LoadVSliceMenu(i);
      return;
    }
  }
  fprintf(stderr,"*** Error: Vector slice file %s was not loaded\n",scripti->cval);
  if(stderr2!=NULL)fprintf(stderr2, "*** Error: Vector slice file %s was not loaded\n", scripti->cval);

}

/* ------------------ ScriptSetTourKeyFrame ------------------------ */

void ScriptSetTourKeyFrame(scriptdata *scripti){
  float keyframe_time;
  keyframe *keyj,*minkey=NULL;
  tourdata *touri;
  float minkeytime=1000000000.0;

  if(selected_tour==NULL)return;
  touri=selected_tour;
  keyframe_time=scripti->fval;
  for(keyj=(touri->first_frame).next;keyj->next!=NULL;keyj=keyj->next){
    float diff_time;

    if(keyj==(touri->first_frame).next){
      minkey=keyj;
      minkeytime = ABS(keyframe_time-keyj->time);
      continue;
    }
    diff_time = ABS(keyframe_time-keyj->time);
    if(diff_time<minkeytime){
      minkey=keyj;
      minkeytime=diff_time;
    }
  }
  if(minkey!=NULL){
    NewSelect(minkey);
    GLUISetTourKeyframe();
    GLUIUpdateTourControls();
  }
}

/* ------------------ ScriptSetTourView ------------------------ */

void ScriptSetTourView(scriptdata *scripti){
  edittour=scripti->ival;
  show_avatar =scripti->ival3;
  switch(scripti->ival2){
    case 0:
      viewtourfrompath=0;
      break;
    case 1:
      viewtourfrompath=1;
      break;
    case 2:
      viewtourfrompath=0;
      break;
    default:
      viewtourfrompath=0;
      break;
  }
  GLUIUpdateTourState();
}

/* ------------------ ScriptSetSliceBounds ------------------------ */

void ScriptSetSliceBounds(scriptdata *scripti){
  int set_valmin, set_valmax;
  float valmin, valmax;
  char *quantity;

  set_valmin = scripti->ival;
  set_valmax = scripti->ival2;
  valmin = scripti->fval;
  valmax = scripti->fval2;
  quantity = scripti->quantity2;
  SetSliceBounds(set_valmin, valmin, set_valmax, valmax, quantity);
}

/* ------------------ ScriptSetSliceAverage ------------------------ */

void ScriptSetSliceAverage(scriptdata *scripti){
  slice_average_flag     = scripti->ival;
  slice_average_interval = scripti->fval;
}

/* ------------------ ScriptOutputSlicedata ------------------------ */

void ScriptOutputSliceData(scriptdata *scripti){
  output_slicedata     = scripti->ival;
}

/* ------------------ ScriptSetBoundBounds ------------------------ */

void ScriptSetBoundBounds(scriptdata *scripti){
  int set_valmin, set_valmax;
  float valmin, valmax;
  char *quantity;

  set_valmin = scripti->ival;
  set_valmax = scripti->ival2;
  valmin = scripti->fval;
  valmax = scripti->fval2;
  quantity = scripti->quantity2;
  SetBoundBounds(set_valmin, valmin, set_valmax, valmax, quantity);
}

/* ------------------ ScriptSetTimeVal ------------------------ */

void ScriptSetTimeVal(scriptdata *scripti){
  float timeval;
  int i,imin;
  float valmin;

  timeval = scripti->fval;
  PRINTF("script: setting time to %f\n\n", timeval);
  UpdateTimes();
  if(global_times == NULL || nglobal_times <= 0){
    PRINTF("***error: SETTIMES script failed, global_times time array not defined\n");
    return;
  }
  float mintime, maxtime;

  mintime = global_times[0];
  if(timeval < mintime)timeval = mintime;

  maxtime = global_times[nglobal_times - 1] - 0.0001;
  if(timeval>maxtime){
    float dt;

    dt = timeval-maxtime;
    if(nglobal_times>1&&dt>global_times[1]-global_times[0]){
      fprintf(stderr,"*** Error: data not available at time requested\n");
      fprintf(stderr,"           time: %f s, min time: %f, max time: %f s, number of times: %i\n",
        timeval,global_times[0],global_times[nglobal_times-1],nglobal_times);
      fprintf(stderr,"all times: ");

      if(stderr2!=NULL)fprintf(stderr2, "*** Error: data not available at time requested\n");
      if(stderr2!=NULL)fprintf(stderr2, "           time: %f s, min time: %f, max time: %f s, number of times: %i\n",
        timeval, global_times[0], global_times[nglobal_times - 1], nglobal_times);
    }
    timeval=maxtime;
  }
  valmin=ABS(global_times[0]-timeval);
  imin=0;
  for(i=1;i<nglobal_times;i++){
    float val;

    val = ABS(global_times[i]-timeval);
    if(val<valmin){
      valmin=val;
      imin=i;
    }
  }
  itimes=imin;
  script_itime=imin;
  stept=0;
  last_time_paused = 1;
  force_redisplay=1;
  UpdateFrameNumber(0);
  UpdateTimeLabels();
  ForceIdle();
}

/* ------------------ ScriptProjection ------------------------ */

void ScriptProjection(scriptdata *scripti){
  if(scripti->ival==1){
    projection_type = PROJECTION_PERSPECTIVE;
  }
  else{
    projection_type = PROJECTION_ORTHOGRAPHIC;
  }
  GLUISceneMotionCB(PROJECTION);
}

//    sscanf(buffer,"%i %i %i %i",&vis_gslice_data, &show_gslice_triangles, &show_gslice_triangulation, &show_gslice_normal);
//    sscanf(buffer,"%f %f %f",gslice_xyz,gslice_xyz+1,gslice_xyz+2);
//    sscanf(buffer,"%f %f",gslice_normal_azelev,gslice_normal_azelev+1);

/* ------------------ ScriptGSliceView ------------------------ */

void ScriptGSliceView(scriptdata *scripti){
  vis_gslice_data = scripti->ival;
  show_gslice_triangles = scripti->ival2;
  show_gslice_triangulation = scripti->ival3;
  show_gslice_normal = scripti->ival4;
  update_gslice=1;
}

/* ------------------ ScriptGSlicePos ------------------------ */

void ScriptGSlicePos(scriptdata *scripti){
  gslice_xyz[0]=scripti->fval;
  gslice_xyz[1]=scripti->fval2;
  gslice_xyz[2]=scripti->fval3;
  update_gslice=1;
}

/* ------------------ ScriptGSliceOrien ------------------------ */

void ScriptGSliceOrien(scriptdata *scripti){
  gslice_normal_azelev[0]=scripti->fval;
  gslice_normal_azelev[1]=scripti->fval2;
  update_gslice=1;
}

/* ------------------ SetTimeVal ------------------------ */

void SetTimeVal(float timeval){
  int i;

  if(global_times!=NULL&&nglobal_times>0){
    if(timeval<global_times[0])timeval=global_times[0];
    if(timeval>global_times[nglobal_times-1]-0.0001)timeval=global_times[nglobal_times-1]-0.0001;
    for(i=0;i<nglobal_times;i++){
      float tlow, thigh;

      if(i==0){
        tlow = global_times[i];
        thigh = (global_times[i]+global_times[i+1])/2.0;
      }
      else if(i==nglobal_times-1){
        tlow = (global_times[i-1]+global_times[i])/2.0;
        thigh = global_times[i];
      }
      else{
        tlow=(global_times[i-1]+global_times[i])/2.0;
        thigh=(global_times[i]+global_times[i+1])/2.0;
      }
      if(tlow<=timeval&&timeval<thigh){
        itimes=i;
        stept=1;
        force_redisplay=1;
        UpdateFrameNumber(0);
        UpdateTimeLabels();
        Keyboard('t',FROM_SMOKEVIEW);
        break;
      }
    }
  }
}

/* ------------------ ScriptRGBtest ------------------------ */

void ScriptRGBtest(scriptdata *scripti){
  update_rgb_test = 1;
  rgb_test_xyz[0] = scripti->fval;
  rgb_test_xyz[1] = scripti->fval2;
  rgb_test_xyz[2] = scripti->fval3;
  FDS2SMV_XYZ(rgb_test_xyz, rgb_test_xyz);
  rgb_test_rgb[0] = scripti->ival;
  rgb_test_rgb[1] = scripti->ival2;
  rgb_test_rgb[2] = scripti->ival3;
  rgb_test_delta  = scripti->ival4;
  use_lighting = 0;
  update_use_lighting = 1;
}

/* ------------------ ScriptSetClipx ------------------------ */
#define CLIP_xlower 0
#define CLIP_ylower 1
#define CLIP_zlower 2
#define CLIP_xupper 3
#define CLIP_yupper 4
#define CLIP_zupper 5

void ScriptSetClipx(scriptdata *scripti){
  clipinfo.clip_xmin = scripti->ival;
  clipinfo.clip_xmax = scripti->ival2;
  clipinfo.xmin      = scripti->fval;
  clipinfo.xmax      = scripti->fval2;
  ClipCB(CLIP_xlower);
  ClipCB(CLIP_xupper);
}

/* ------------------ ScriptSetClipy ------------------------ */

void ScriptSetClipy(scriptdata *scripti){
  clipinfo.clip_ymin = scripti->ival;
  clipinfo.clip_ymax = scripti->ival2;
  clipinfo.ymin      = scripti->fval;
  clipinfo.ymax      = scripti->fval2;
  ClipCB(CLIP_ylower);
  ClipCB(CLIP_yupper);
}

/* ------------------ ScriptSetClipz ------------------------ */

void ScriptSetClipz(scriptdata *scripti){
  clipinfo.clip_zmin = scripti->ival;
  clipinfo.clip_zmax = scripti->ival2;
  clipinfo.zmin      = scripti->fval;
  clipinfo.zmax      = scripti->fval2;
  ClipCB(CLIP_zlower);
  ClipCB(CLIP_zupper);
}

/* ------------------ ScriptSetClipMode ------------------------ */

void ScriptSetClipMode(scriptdata *scripti){
  clip_mode = scripti->ival;
}
/* ------------------ ScriptSetViewpoint ------------------------ */

void ScriptSetViewpoint(scriptdata *scripti){
  char *viewpoint;

  viewpoint = scripti->cval;
  update_viewpoint_script = 3;
  if(viewpoint == NULL){
    strcpy(viewpoint_script, "viewpoint");
  }
  else{
    strcpy(viewpoint_script, viewpoint);
  }
  viewpoint_script_ptr = NULL;
  PRINTF("script: set viewpoint to %s\n\n",viewpoint);
  if(GetCamera(viewpoint) == NULL){
    fprintf(stderr, "*** Error: The viewpoint %s was not found\n", viewpoint);
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: The viewpoint %s was not found\n", viewpoint);
  }
  else{
    viewpoint_script_ptr = viewpoint_script;
  }
}

/* ------------------ ScriptViewXYZMINMAXOrtho ------------------------ */

void ScriptViewXYZMINMAXOrtho(int command){
  switch(command){
  case SCRIPT_VIEWXMIN:
  case MENU_VIEW_XMIN:
    zaxis_angles[0] = 0.0;
    zaxis_angles[1] = 90.0;
    zaxis_angles[2] = 90.0;
    break;
  case SCRIPT_VIEWXMAX:
  case MENU_VIEW_XMAX:
    zaxis_angles[0] =   0.0;
    zaxis_angles[1] =  90.0;
    zaxis_angles[2] = -90.0;
    break;

  case SCRIPT_VIEWYMIN:
  case MENU_VIEW_YMIN:
    zaxis_angles[0] =  0.0;
    zaxis_angles[1] = 90.0;
    zaxis_angles[2] =  0.0;
    break;
  case SCRIPT_VIEWYMAX:
  case MENU_VIEW_YMAX:
    zaxis_angles[0] =   0.0;
    zaxis_angles[1] =  90.0;
    zaxis_angles[2] = 180.0;
    break;

  case SCRIPT_VIEWZMIN:
  case MENU_VIEW_ZMIN:
    zaxis_angles[0] = -90.0;
    zaxis_angles[1] =   0.0;
    zaxis_angles[2] =   0.0;
    break;
  case SCRIPT_VIEWZMAX:
  case MENU_VIEW_ZMAX:
    zaxis_angles[0] =  90.0;
    zaxis_angles[1] =  0.0;
    zaxis_angles[2] =  0.0;
    break;
  default:
    assert(FFALSE);
    break;
  }
  GLUIResetView(EXTERNAL_VIEW);
  use_customview=0;
  GLUISceneMotionCB(CUSTOM_VIEW);
  GLUISceneMotionCB(ZAXIS_CUSTOM);
}


/* ------------------ ScriptViewXYZMINMAXPersp ------------------------ */

void ResetDefaultMenu(int var);
void ScriptViewXYZMINMAXPersp(int command){
  switch(command){
  case SCRIPT_VIEWXMIN:
    ResetDefaultMenu(VIEW_XMIN);
    break;
  case SCRIPT_VIEWXMAX:
    ResetDefaultMenu(VIEW_XMAX);
    break;
  case SCRIPT_VIEWYMIN:
    ResetDefaultMenu(VIEW_YMIN);
    break;
  case SCRIPT_VIEWYMAX:
    ResetDefaultMenu(VIEW_YMAX);
    break;
  case SCRIPT_VIEWZMIN:
    ResetDefaultMenu(VIEW_ZMIN);
    break;
  case SCRIPT_VIEWZMAX:
    ResetDefaultMenu(VIEW_ZMAX);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ SetViewZMAXPersp ------------------------ */

void SetViewZMAXPersp(void){
  float aperture_temp1, aperture_temp2;
  float azimuth, elevation;
  float DL;
  float DL1, DL2;
  float width, height;
  float xcen, ycen, zcen;

  aperture_temp1 = Zoom2Aperture(zoom);
  aperture_temp2 = 2.0*RAD2DEG*atan(scene_aspect_ratio*tan(DEG2RAD*aperture_temp1/2.0));

  if(have_geom_factors==1){
    xcen   = (geom_xmin+geom_xmax)/2.0;
    ycen   = (geom_ymin+geom_ymax)/2.0;
    width  = geom_xmax - geom_xmin;
    height = geom_ymax - geom_ymin;
  }
  else{
    xcen = (xbar0ORIG+xbarORIG)/2.0;
    ycen = (ybar0ORIG+ybarORIG)/2.0;
    width = xbarORIG-xbar0ORIG;
    height = ybarORIG-ybar0ORIG;
  }

  DL1 = (width/2.0)/tan(DEG2RAD*aperture_temp1/2.0);
  DL2 = (height/2.0)/tan(DEG2RAD*aperture_temp2/2.0);
  DL = 1.05*MAX(DL1, DL2);

  zcen = zbarORIG+DL;
  elevation = -89.9;
  azimuth = 0.0;
  GLUIResetView(EXTERNAL_VIEW);

  use_customview = 0;
  GLUISceneMotionCB(CUSTOM_VIEW);
  GLUIViewpointCB(RESTORE_VIEW);
  glui_xyz_fds[0]      = xcen;
  glui_xyz_fds[1]      = ycen;
  glui_xyz_fds[2]      = zcen;
  customview_azimuth   = azimuth;
  customview_elevation = elevation;
  use_customview       = 1;
  GLUISceneMotionCB(CUSTOM_VIEW);
  GLUISceneMotionCB(SET_VIEW_XYZ);
  GLUIUpdatePosView();
}

/* ------------------ RunScriptCommand ------------------------ */

int RunScriptCommand(scriptdata *script_command){

// This procedure should return 1 if the smokeview frame should not be advanced.
// (to ensure images are rendered at the right time step)

  scriptdata *scripti;
  int returnval=0;

  if(stderr2 == NULL){
    stderr2 = tmpfile();
  }
  if(script_command>scriptinfo+nscriptinfo-1){
    current_script_command=NULL;
    if(stderr2 != NULL){
      unsigned int nchars;

      nchars = StreamCopy(stderr2, stderr, 0);
      if(nchars>0){
        fprintf(stderr, "----------------------------------------------\n");
        fprintf(stderr, "Smokeview script errors :\n");
        StreamCopy(stderr2, stderr, 1);
        fprintf(stderr, "----------------------------------------------\n");
      }
      fclose(stderr2);
      stderr2 = NULL;
    }
    return returnval;
  }
  scripti = script_command;
  PRINTF("\n");
  PRINTF("script: %s\n",scripti->command_label);
  if(scripti->cval!=NULL){
    PRINTF("script:  %s\n",scripti->cval);
  }
  if(scripti->cval2!=NULL){
    PRINTF("script:  %s\n",scripti->cval2);
  }
  PRINTF("\n");
  switch(scripti->command){
    case SCRIPT_UNLOADALL:
      LoadUnloadMenu(UNLOADALL);
      break;
    case SCRIPT_RENDERSIZE:
      script_render_width = scripti->ival;
      script_render_height = scripti->ival2;
      RenderMenu(RenderCustom);
      break;
    case SCRIPT_RENDERTYPE:
      if(STRCMP(scripti->cval, "JPG")==0){
        UpdateRenderType(JPEG);
      }
      else{
        UpdateRenderType(PNG);
      }
      break;
    case SCRIPT_HILIGHTMINVALS:
      ScriptHilightMinMaxVals(scripti, 0);
      break;
    case SCRIPT_HILIGHTMAXVALS:
      ScriptHilightMinMaxVals(scripti, 1);
      break;
    case SCRIPT_MOVIETYPE:
      if(STRCMP(scripti->cval, "WMV") == 0){
        UpdateMovieType(WMV);
      }
      if(STRCMP(scripti->cval, "MP4") == 0){
        UpdateMovieType(MP4);
      }
      else{
        UpdateMovieType(AVI);
      }
      break;
    case SCRIPT_RENDERDIR:
      if(scripti->cval!=NULL&&strlen(scripti->cval)>0){
        script_dir_path=scripti->cval;
        if(Writable(script_dir_path)==NO){
          fprintf(stderr,"*** Error: Cannot write to the RENDERDIR directory: %s\n",script_dir_path);
          if(stderr2!=NULL)fprintf(stderr2, "*** Error: Cannot write to the RENDERDIR directory: %s\n", script_dir_path);
          SMV_EXIT(2);
        }
        PRINTF("script: setting render path to %s\n",script_dir_path);
      }
      else{
        script_dir_path=NULL;
      }
      break;
    case SCRIPT_RENDERHTMLDIR:
      if(scripti->cval!=NULL&&strlen(scripti->cval)>0){
        script_htmldir_path = scripti->cval;
        if(Writable(script_htmldir_path)==NO){
          fprintf(stderr, "*** Error: Cannot write to the RENDERHTMLDIR directory: %s\n", script_htmldir_path);
          if(stderr2!=NULL)fprintf(stderr2, "*** Error: Cannot write to the RENDERHTMLDIR directory: %s\n", script_htmldir_path);
          SMV_EXIT(2);
        }
        PRINTF("script: setting html render path to %s\n", script_htmldir_path);
      }
      else{
        script_htmldir_path = NULL;
      }
      break;
    case SCRIPT_KEYBOARD:
      if(scripti->cval!=NULL){
        char *key;

        script_keystate=0;
        key = scripti->cval + strlen(scripti->cval) - 1;
        if(strncmp(scripti->cval,"ALT",3)==0)script_keystate=GLUT_ACTIVE_ALT;

        Keyboard(*key,FROM_SCRIPT);
      }
      returnval=1;
      break;
    case SCRIPT_SCENECLIP:
      clip_mode=scripti->ival;
      updatefacelists=1;
      break;
    case SCRIPT_XSCENECLIP:
      clipinfo.clip_xmin=scripti->ival;
      clipinfo.xmin = scripti->fval;

      clipinfo.clip_xmax=scripti->ival2;
      clipinfo.xmax = scripti->fval2;
      updatefacelists=1;
      break;
    case SCRIPT_YSCENECLIP:
      clipinfo.clip_ymin=scripti->ival;
      clipinfo.ymin = scripti->fval;

      clipinfo.clip_ymax=scripti->ival2;
      clipinfo.ymax = scripti->fval2;
      updatefacelists=1;
      break;
    case SCRIPT_ZSCENECLIP:
      clipinfo.clip_zmin=scripti->ival;
      clipinfo.zmin = scripti->fval;

      clipinfo.clip_zmax=scripti->ival2;
      clipinfo.zmax = scripti->fval2;
      updatefacelists=1;
      break;
    case SCRIPT_RENDERCLIP:
      clip_rendered_scene=scripti->ival;
      render_clip_left=scripti->ival2;
      render_clip_right=scripti->ival3;
      render_clip_bottom=scripti->ival4;
      render_clip_top=scripti->ival5;
      break;
    case SCRIPT_RENDERONCE:
      Keyboard('r',FROM_SMOKEVIEW);
      returnval=1;
      break;
    case SCRIPT_RENDERHTMLALL:
      ScriptRenderHtml(scripti, HTML_ALL_TIMES);
      returnval = 1;
      break;
    case SCRIPT_RENDERHTMLONCE:
      ScriptRenderHtml(scripti, HTML_CURRENT_TIME);
      returnval = 1;
      break;
    case SCRIPT_RENDERHTMLGEOM:
      ScriptRenderGeom(scripti);
      returnval = 1;
      break;
    case SCRIPT_RENDERHTMLOBST:
      ScriptRenderObst(scripti);
      returnval = 1;
      break;
    case SCRIPT_RENDERHTMLSLICENODE:
      ScriptRenderSliceNode(scripti);
      returnval = 1;
      break;
    case SCRIPT_RENDERHTMLSLICECELL:
      ScriptRenderSliceCell(scripti);
      returnval = 1;
      break;
    case SCRIPT_RENDERDOUBLEONCE:
      Keyboard('R',FROM_SMOKEVIEW);
      returnval=1;
      break;
    case SCRIPT_RENDERSTART:
      ScriptRenderStart(scripti);
      break;
    case SCRIPT_RENDERALL:
      ScriptRenderAll(scripti);
      break;
    case SCRIPT_RENDER360ALL:
      ScriptRender360All(scripti);
      break;
    case SCRIPT_VOLSMOKERENDERALL:
      ScriptVolSmokeRenderAll(scripti);
      break;
    case SCRIPT_ISORENDERALL:
      ScriptIsoRenderAll(scripti);
      break;
    case SCRIPT_MAKEMOVIE:
      ScriptMakeMovie(scripti);
      break;
    case SCRIPT_LOADFILE:
      ScriptLoadFile(scripti);
      break;
    case SCRIPT_LABEL:
      ScriptLabel(scripti);
      break;
    case SCRIPT_LOADINIFILE:
      ScriptLoadIniFile(scripti);
      break;
    case SCRIPT_GPUOFF:
      usegpu = 0;
      gpuactive = 0;
      break;
    case SCRIPT_LOADVFILE:
      ScriptLoadVecFile(scripti);
      break;
    case SCRIPT_LOADBOUNDARY:
      ScriptLoadBoundary(scripti,-1);
      break;
    case SCRIPT_LOADBOUNDARYM:
      ScriptLoadBoundary(scripti,scripti->ival);
      break;
    case SCRIPT_PARTCLASSCOLOR:
      ScriptPartClassColor(scripti);
      break;
    case SCRIPT_SHOWHVACDUCTVAL:
      ScriptShowHVACDuctVal(scripti);
      break;
    case SCRIPT_SHOWCBAREDIT:
      ScriptShowCbarEdit(scripti);
      break;
    case SCRIPT_HIDECBAREDIT:
      ScriptHideCbarEdit(scripti);
      break;
    case SCRIPT_SETCBARLAB:
      ScriptSetCbarLab(scripti);
      break;
    case SCRIPT_SETCBARRGB:
      ScriptSetCbarRgb(scripti);
      break;
    case SCRIPT_SETCBAR:
      ScriptSetCbar(scripti);
      break;
    case SCRIPT_SHOWHVACNODEVAL:
      ScriptShowHVACNodeVal(scripti);
      break;
    case SCRIPT_SETDEMOMODE:
      ScriptSetDemoMode(scripti);
      break;
    case SCRIPT_SMOKEPROP:
      ScriptSmokeprop(scripti);
      break;
    case SCRIPT_HIDEHVACVALS:
      ScriptHideHVACVals();
      break;
    case SCRIPT_OUTPUTSMOKESENSORS:
      ScriptOutputSmokeSensors();
      break;
    case SCRIPT_SHOWALLDEVS:
      ShowDevicesMenu(MENU_DEVICES_SHOWALL);
      break;
    case SCRIPT_HIDEALLDEVS:
      ShowDevicesMenu(MENU_DEVICES_HIDEALL);
      break;
    case SCRIPT_SHOWDEV:
    case SCRIPT_HIDEDEV:
      {
        int dev_index;

        dev_index = GetDeviceIndexFromLabel(scripti->cval);
        if(dev_index<0){
          printf("***error: device %s does not exist\n", scripti->cval);
          SMV_EXIT(2);
          break;
        }
        dev_index += global_scase.devicecoll.ndeviceinfo;                                       // show device
        if(scripti->command==SCRIPT_HIDEDEV)dev_index += global_scase.devicecoll.ndeviceinfo;  // hide device
        ShowDevicesMenu(dev_index);
      }
      break;
    case SCRIPT_SHOWPLOT3DDATA:
      ScriptShowPlot3dData(scripti);
      break;
    case SCRIPT_PLOT3DPROPS:
      ScriptPlot3dProps(scripti);
      break;
    case SCRIPT_VIEWXMIN:
    case SCRIPT_VIEWXMAX:
    case SCRIPT_VIEWYMIN:
    case SCRIPT_VIEWYMAX:
    case SCRIPT_VIEWZMIN:
    case SCRIPT_VIEWZMAX:
      if(projection_type==PROJECTION_PERSPECTIVE){
        ScriptViewXYZMINMAXPersp(scripti->command);
      }
      else{
        ScriptViewXYZMINMAXOrtho(scripti->command);
      }
      break;
    case SCRIPT_XYZVIEW:
      ScriptXYZView(scripti->fval, scripti->fval2, scripti->fval3, scripti->fval4, scripti->fval5);
      break;
    case SCRIPT_PARTCLASSTYPE:
      ScriptPartClassType(scripti);
      break;
    case SCRIPT_LOADTOUR:
      ScriptLoadTour(scripti);
      break;
    case SCRIPT_UNLOADTOUR:
      TourMenu(MENU_TOUR_MANUAL);
      break;
    case SCRIPT_NOEXIT:
      noexit = 1;
      break;
    case SCRIPT_EXIT:
#ifndef _DEBUG
      SMV_EXIT(0);
#endif
      break;
    case SCRIPT_LOADISO:
      ScriptLoadIso(scripti,-1); // load isosurface for all meshes
      break;
    case SCRIPT_LOADISOM:
      ScriptLoadIso(scripti, scripti->ival); // load isosurface for mesh script->ival
      break;
    case SCRIPT_LOAD3DSMOKE:
      ScriptLoad3dSmoke(scripti);
      break;
    case SCRIPT_LOADVOLSMOKE:
      ScriptLoadVolSmoke(scripti);
      break;
    case SCRIPT_LOADVOLSMOKEFRAME:
      ScriptLoadVolSmokeFrame(scripti,1);
      returnval=1;
      break;
    case SCRIPT_LOADPARTICLES:
      ScriptLoadParticles(scripti);
      break;
    case SCRIPT_LOADSLCF:
      ScriptLoadSLCF(scripti);
      break;
    case SCRIPT_LOADHVAC:
      LoadHVACMenu(MENU_HVAC_LOAD);
      break;
    case SCRIPT_UNLOADPLOT2D:
      ScriptUnLoadPlot2D(scripti);
      break;
    case SCRIPT_LOADSLICE:
      ScriptLoadSlice(scripti);
      break;
    case SCRIPT_LOADSLICERENDER:
 //   Since ScriptLoadSliceRender is called multiple times, call this routine from DoScripts in callback.c
 //     ScriptLoadSliceRender(scripti);
      break;
    case SCRIPT_LOADSMOKERENDER:
      //   Since ScriptLoadSmokeRender is called multiple times, call this routine from DoScripts in callback.c
      //     ScriptLoadSmokeRender(scripti);
      break;
    case SCRIPT_LOADSLICEM:
      ScriptLoadSliceM(scripti, scripti->ival2);
      break;
    case SCRIPT_LOADVSLICE:
      ScriptLoadVSlice(scripti);
      break;
    case SCRIPT_LOADVSLICEM:
      ScriptLoadVSliceM(scripti,scripti->ival2);
      break;
    case SCRIPT_LOADPLOT3D:
      ScriptLoadPlot3D(scripti);
      break;
    case SCRIPT_SETTIMEVAL:
      returnval=1;
      ScriptSetTimeVal(scripti);
      break;
    case SCRIPT_SETSLICEBOUNDS:
      ScriptSetSliceBounds(scripti);
      break;
    case SCRIPT_SETSLICEAVERAGE:
      ScriptSetSliceAverage(scripti);
      break;
    case SCRIPT_OUTPUTSLICEDATA:
      ScriptOutputSliceData(scripti);
      break;
    case SCRIPT_SETBOUNDBOUNDS:
      ScriptSetBoundBounds(scripti);
      break;
    case SCRIPT_SETTOURVIEW:
      ScriptSetTourView(scripti);
      break;
    case SCRIPT_SETTOURKEYFRAME:
      ScriptSetTourKeyFrame(scripti);
      break;
    case SCRIPT_SETCLIPX:
      ScriptSetClipx(scripti);
      break;
    case SCRIPT_SETCLIPY:
      ScriptSetClipy(scripti);
      break;
    case SCRIPT_SETCLIPZ:
      ScriptSetClipz(scripti);
      break;
    case SCRIPT_SETCLIPMODE:
      ScriptSetClipMode(scripti);
      break;
    case SCRIPT_SETVIEWPOINT:
      ScriptSetViewpoint(scripti);
      break;
    case SCRIPT_RGBTEST:
      ScriptRGBtest(scripti);
      break;
    case SCRIPT_GSLICEVIEW:
      ScriptGSliceView(scripti);
      break;
    case SCRIPT_PROJECTION:
      ScriptProjection(scripti);
      break;
    case SCRIPT_GSLICEPOS:
      ScriptGSlicePos(scripti);
      break;
    case SCRIPT_GSLICEORIEN:
      ScriptGSliceOrien(scripti);
      break;
    case SCRIPT_CBARFLIP:
      colorbar_flip=0;
      ColorbarMenu(COLORBAR_FLIP);
      break;
    case SCRIPT_CBARNORMAL:
      colorbar_flip=1;
      ColorbarMenu(COLORBAR_FLIP);
      break;
    default:
      assert(FFALSE);
      break;
  }
  GLUTPOSTREDISPLAY;
  return returnval;
}
