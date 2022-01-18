#include "options.h"
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

/* ------------------ GetNewScriptFileName ------------------------ */

void GetNewScriptFileName(char *newscriptfilename){
  char buffer[1024];
  int i;
  int nexti;
  scriptfiledata *scriptfile;

  for(i=0;i<1000;i++){
    if(i==0){
      strcpy(buffer,fdsprefix);
      strcat(buffer,".ssf");
    }
    else{
      sprintf(buffer,"%s_%03i.ssf",fdsprefix,i);
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

/* ------------------ insert_luascriptfile ------------------------ */

#ifdef pp_LUA
luascriptfiledata *insert_luascriptfile(char *file){
  luascriptfiledata *thisptr,*prevptr,*nextptr;
  int len;
  luascriptfiledata *luascriptfile;
  int idmax=-1;

  for(luascriptfile=first_luascriptfile.next;luascriptfile->next!=NULL;luascriptfile=luascriptfile->next){
    if(luascriptfile->id>idmax)idmax=luascriptfile->id;
    if(luascriptfile->file==NULL)continue;
    if(strcmp(file,luascriptfile->file)==0)return luascriptfile;
  }

  NewMemory((void **)&thisptr,sizeof(luascriptfiledata));
  nextptr = &last_luascriptfile;
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
#endif

/* ------------------ StartScript ------------------------ */

void StartScript(void){
  if(scriptinfo==NULL){
    fprintf(stderr,"*** Error: Smokeview script does not exist\n");
    if(stderr2!=NULL)fprintf(stderr2,"*** Error: Smokeview script does not exist\n");
    return;
  }
  GluiScriptDisable();
  current_script_command=scriptinfo-1;
  iso_multithread_save = iso_multithread;
  iso_multithread = 0;
  viewpoint_script_ptr = NULL;
}

/* ------------------ GetCharPointer ------------------------ */

char *GetCharPointer(char *buffer2){
  char *cval=NULL, *buffptr;
  int len;

  buffptr = RemoveComment(buffer2);
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

/* ------------------ GetScriptKeywordIndex ------------------------ */

int GetScriptKeywordIndex(char *keyword){
  if(keyword==NULL||strlen(keyword)==0)return SCRIPT_UNKNOWN;

  if(MatchUpper(keyword,"CBARFLIP") == MATCH)return SCRIPT_CBARFLIP;                     // documented
  if(MatchUpper(keyword,"CBARNORMAL") == MATCH)return SCRIPT_CBARNORMAL;                 // documented
  if(MatchUpper(keyword,"EXIT") == MATCH)return SCRIPT_EXIT;                             // documented
  if(MatchUpper(keyword,"GSLICEORIEN")==MATCH)return SCRIPT_GSLICEORIEN;
  if(MatchUpper(keyword,"GSLICEPOS")==MATCH)return SCRIPT_GSLICEPOS;
  if(MatchUpper(keyword,"GSLICEVIEW")==MATCH)return SCRIPT_GSLICEVIEW;
  if(MatchUpper(keyword,"PROJECTION")==MATCH)return SCRIPT_PROJECTION;
  if(MatchUpper(keyword,"ISORENDERALL")==MATCH)return SCRIPT_ISORENDERALL;
  if(MatchUpper(keyword,"KEYBOARD") == MATCH)return SCRIPT_KEYBOARD;                     // documented
  if(MatchUpper(keyword,"LABEL")==MATCH)return SCRIPT_LABEL;
  if(MatchUpper(keyword,"LOAD3DSMOKE") == MATCH)return SCRIPT_LOAD3DSMOKE;               // documented
  if(MatchUpper(keyword,"LOADBOUNDARY") == MATCH)return SCRIPT_LOADBOUNDARY;             // documented
  if(MatchUpper(keyword,"LOADBOUNDARYM") == MATCH)return SCRIPT_LOADBOUNDARYM;
  if(MatchUpper(keyword,"LOADFILE") == MATCH)return SCRIPT_LOADFILE;                     // documented
  if(MatchUpper(keyword,"LOADINIFILE") == MATCH)return SCRIPT_LOADINIFILE;               // documented
  if(MatchUpper(keyword,"LOADISO") == MATCH)return SCRIPT_LOADISO;                       // documented
  if(MatchUpper(keyword,"LOADISOM") == MATCH)return SCRIPT_LOADISOM;                     // documented
  if(MatchUpper(keyword,"LOADPARTICLES") == MATCH)return SCRIPT_LOADPARTICLES;           // documented
  if(MatchUpper(keyword,"LOADPLOT3D") == MATCH)return SCRIPT_LOADPLOT3D;                 // documented
  if(MatchUpper(keyword,"LOADSLICE") == MATCH)return SCRIPT_LOADSLICE;                   // documented
  if(MatchUpper(keyword,"LOADSLCF")==MATCH)return SCRIPT_LOADSLCF;
  if(MatchUpper(keyword,"LOADSLICERENDER")==MATCH)return SCRIPT_LOADSLICERENDER;
  if(MatchUpper(keyword,"LOADSLICEM") == MATCH)return SCRIPT_LOADSLICEM;
  if(MatchUpper(keyword,"LOADTOUR") == MATCH)return SCRIPT_LOADTOUR;                     // documented
  if(MatchUpper(keyword,"LOADVOLSMOKE") == MATCH)return SCRIPT_LOADVOLSMOKE;             // documented
  if(MatchUpper(keyword,"LOADVOLSMOKEFRAME") == MATCH)return SCRIPT_LOADVOLSMOKEFRAME;   // documented
  if(MatchUpper(keyword,"LOADVFILE") == MATCH)return SCRIPT_LOADVFILE;                   // documented
  if(MatchUpper(keyword,"LOADVSLICE") == MATCH)return SCRIPT_LOADVSLICE;                 // documented
  if(MatchUpper(keyword,"LOADVSLICEM") == MATCH)return SCRIPT_LOADVSLICEM;
  if(MatchUpper(keyword,"MAKEMOVIE") == MATCH)return SCRIPT_MAKEMOVIE;
  if(MatchUpper(keyword,"MOVIETYPE")==MATCH)return SCRIPT_MOVIETYPE;
  if(MatchUpper(keyword,"PARTCLASSCOLOR") == MATCH)return SCRIPT_PARTCLASSCOLOR;         // documented
  if(MatchUpper(keyword,"PARTCLASSTYPE") == MATCH)return SCRIPT_PARTCLASSTYPE;           // documented
  if(MatchUpper(keyword,"PLOT3DPROPS") == MATCH)return SCRIPT_PLOT3DPROPS;               // documented
  if(MatchUpper(keyword,"XYZVIEW")==MATCH)return SCRIPT_XYZVIEW;                         // documented
  if(MatchUpper(keyword,"VIEWXMIN")==MATCH)return SCRIPT_VIEWXMIN;                       // documented
  if(MatchUpper(keyword,"VIEWXMAX")==MATCH)return SCRIPT_VIEWXMAX;                       // documented
  if(MatchUpper(keyword,"VIEWYMIN")==MATCH)return SCRIPT_VIEWYMIN;                       // documented
  if(MatchUpper(keyword,"VIEWYMAX")==MATCH)return SCRIPT_VIEWYMAX;                       // documented
  if(MatchUpper(keyword,"VIEWZMIN")==MATCH)return SCRIPT_VIEWZMIN;                       // documented
  if(MatchUpper(keyword,"VIEWZMAX")==MATCH)return SCRIPT_VIEWZMAX;                       // documented
  if(MatchUpper(keyword,"RENDER360ALL") == MATCH)return SCRIPT_RENDER360ALL;
  if(MatchUpper(keyword,"RENDERALL") == MATCH)return SCRIPT_RENDERALL;                   // documented
  if(MatchUpper(keyword,"RENDERCLIP") == MATCH)return SCRIPT_RENDERCLIP;                 // documented
  if(MatchUpper(keyword,"RENDERDIR") == MATCH)return SCRIPT_RENDERDIR;                   // documented
  if(MatchUpper(keyword,"RENDERDOUBLEONCE") == MATCH)return SCRIPT_RENDERDOUBLEONCE;     // documented
  if(MatchUpper(keyword,"RENDERHTMLALL")==MATCH)return SCRIPT_RENDERHTMLALL;
  if(MatchUpper(keyword,"RENDERHTMLDIR") == MATCH)return SCRIPT_RENDERHTMLDIR;
  if(MatchUpper(keyword,"RENDERHTMLGEOM") == MATCH)return SCRIPT_RENDERHTMLGEOM;
  if(MatchUpper(keyword,"RENDERHTMLOBST") == MATCH)return SCRIPT_RENDERHTMLOBST;
  if(MatchUpper(keyword,"RENDERHTMLONCE") ==MATCH)return SCRIPT_RENDERHTMLONCE;
  if(MatchUpper(keyword,"RENDERHTMLSLICENODE")==MATCH)return SCRIPT_RENDERHTMLSLICENODE;
  if(MatchUpper(keyword,"RENDERHTMLSLICECELL")==MATCH)return SCRIPT_RENDERHTMLSLICECELL;
  if(MatchUpper(keyword,"RENDERONCE") == MATCH)return SCRIPT_RENDERONCE;                 // documented
  if(MatchUpper(keyword,"RENDERSIZE") == MATCH)return SCRIPT_RENDERSIZE;
  if(MatchUpper(keyword,"RENDERSTART") == MATCH)return SCRIPT_RENDERSTART;
  if(MatchUpper(keyword,"RENDERTYPE") == MATCH)return SCRIPT_RENDERTYPE;
  if(MatchUpper(keyword,"RGBTEST")==MATCH)return SCRIPT_RGBTEST;
  if(MatchUpper(keyword,"SCENECLIP") == MATCH)return SCRIPT_SCENECLIP;
  if(MatchUpper(keyword,"SETTOURKEYFRAME") == MATCH)return SCRIPT_SETTOURKEYFRAME;
  if(MatchUpper(keyword,"SETTIMEVAL") == MATCH)return SCRIPT_SETTIMEVAL;                 // documented
  if(MatchUpper(keyword,"SETSLICEBOUNDS")==MATCH)return SCRIPT_SETSLICEBOUNDS;
  if(MatchUpper(keyword,"SETBOUNDBOUNDS")==MATCH)return SCRIPT_SETBOUNDBOUNDS;
  if(MatchUpper(keyword,"SETTOURVIEW") == MATCH)return SCRIPT_SETTOURVIEW;
  if(MatchUpper(keyword,"SETVIEWPOINT") == MATCH)return SCRIPT_SETVIEWPOINT;             // documented
  if(MatchUpper(keyword,"SHOWPLOT3DDATA") == MATCH)return SCRIPT_SHOWPLOT3DDATA;         // documented
  if(MatchUpper(keyword,"SHOWSMOKESENSORS")==MATCH)return SCRIPT_SHOWSMOKESENSORS;
  if(MatchUpper(keyword,"SMOKEFRAMES")==MATCH)return SCRIPT_SMOKEFRAMES;
  if(MatchUpper(keyword,"UNLOADALL") == MATCH)return SCRIPT_UNLOADALL;                   // documented
  if(MatchUpper(keyword,"UNLOADTOUR") == MATCH)return SCRIPT_UNLOADTOUR;                 // documented
  if(MatchUpper(keyword,"VOLSMOKERENDERALL") == MATCH)return SCRIPT_VOLSMOKERENDERALL;   // documented
  if(MatchUpper(keyword,"XSCENECLIP")==MATCH)return SCRIPT_XSCENECLIP;                   // documented
  if(MatchUpper(keyword,"YSCENECLIP") == MATCH)return SCRIPT_YSCENECLIP;                 // documented
  if(MatchUpper(keyword,"ZSCENECLIP") == MATCH)return SCRIPT_ZSCENECLIP;                 // documented

  return SCRIPT_UNKNOWN;
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

/* ------------------ ScriptErrorCheck ------------------------ */

void ScriptErrorCheck(char *keyword, char *data){
  if(GetScriptKeywordIndex(data)!=SCRIPT_UNKNOWN){
    fprintf(stderr,"*** Error: While parsing the Smokeview script entry: %s ,\n",keyword);
    fprintf(stderr,"           a keyword was found in \"%s\", data was expected.\n",data);
    if(stderr2!=NULL)fprintf(stderr2,"*** Error: While parsing the Smokeview script entry: %s ,\n",keyword);
    if(stderr2!=NULL)fprintf(stderr2,"           a keyword was found in \"%s\", data was expected.\n",data);
  }
}

#define SETbuffer \
if(fgets(buffer, 255, stream) == NULL){\
scriptEOF = 1; \
break; \
}\
buffptr = RemoveComment(buffer); \
buffptr = TrimFront(buffptr); \
ScriptErrorCheck(keyword, buffptr)

#define SETcval \
SETbuffer;\
scripti->cval=GetCharPointer(buffptr)

#define SETcval2 \
SETbuffer;\
scripti->cval2 = GetCharPointer(buffptr)

#define SETfval \
SETbuffer;\
sscanf(buffptr, "%f", &scripti->fval)

#define SETival \
SETbuffer;\
sscanf(buffptr, "%i", &scripti->ival)

#define SETival2 \
SETbuffer;\
sscanf(buffptr, "%i", &scripti->ival2)

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

/* ------------------ GetScriptKeyWord ------------------------ */

int GetScriptKeyWord(char *token, char **keywords, int nkeywords){
  int i;

  for(i = 0; i<nkeywords; i++){
    char *kw;

    kw = keywords[i];
    if(strcmp(token, kw)==0)return i;
  }
  return TOKEN_UNKNOWN;
}

/* ------------------ ParseTokens ------------------------ */

int ParseTokens(char *buffer, char **keywords, int *type, int nkeywords, int *tokens, int *itokens, float *ftokens, char **ctokens, int max_tokens){
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
    keyword_index = GetScriptKeyWord(kw, keywords, nkeywords);
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
    switch (type[keyword_index]){
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
	ASSERT(FFALSE);
	break;
    }
  }
  return i;

}

/* ------------------ CompileScript ------------------------ */

int CompileScript(char *scriptfile){
  FILE *stream;
  int return_val;

  return_val=1;
  if(scriptfile==NULL){
    fprintf(stderr,"*** Error: scriptfile name is NULL\n");
    return return_val;
  }
  stream=fopen(scriptfile,"r");
  if(stream==NULL){
    fprintf(stderr,"*** Error: scriptfile, %s, could not be opened for input\n",scriptfile);
    return return_val;
  }

  return_val=0;

  /*
   ************************************************************************
   ************************ start of pass 1 *********************************
   ************************************************************************
 */

  FreeScript();

  while(!feof(stream)){
    char buffer[1024], buffer2[1024], *buffptr;

    if(fgets(buffer2,255,stream)==NULL)break;
    buffptr = RemoveComment(buffer2);
    strcpy(buffer, buffptr);


    if(GetScriptKeywordIndex(buffer)!=SCRIPT_UNKNOWN)nscriptinfo++;
  }

  if(nscriptinfo==0){
    fclose(stream);
    fprintf(stderr,"*** Error: scriptfile has no usable commands\n");
    return 1;
  }

NewMemory((void **)&scriptinfo, nscriptinfo*sizeof(scriptdata));

  nscriptinfo=0;

  /*
   ************************************************************************
   ************************ start of pass 2 *********************************
   ************************************************************************
 */
  rewind(stream);
  while(!feof(stream)){
    int keyword_index;
    int scriptEOF;
    char keyword[255];
    char buffer[1024], buffer2[1024], *buffptr;
    scriptdata *scripti;
    int fatal_error;

    fatal_error = 0;

    if(fgets(buffer2,255,stream)==NULL)break;
    buffptr = RemoveComment(buffer2);
    strcpy(buffer, buffptr);

    if(strlen(buffer)==0)continue;

    keyword_index = GetScriptKeywordIndex(buffer);
    if(keyword_index==SCRIPT_UNKNOWN)continue;
    strcpy(keyword,buffer);

    scripti = scriptinfo + nscriptinfo;
    InitScriptI(scripti,keyword_index,buffer);

    scriptEOF=0;
    switch(keyword_index){

// UNLOADALL
      case SCRIPT_UNLOADALL:

// LOADPARTICLES
      case SCRIPT_LOADPARTICLES:
        scripti->need_graphics = 0;
        break;

// CBARFLIP:
      case SCRIPT_CBARFLIP:

// CBARNORMAL:
      case SCRIPT_CBARNORMAL:

// SHOWSMOKESENSORS
      case SCRIPT_SHOWSMOKESENSORS:
        break;

// RENDERSIZE
// width height (int)
      case SCRIPT_RENDERSIZE:
        SETbuffer;
        sscanf(buffer, "%i %i", &scripti->ival, &scripti->ival2);
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
        if(keyword_index==SCRIPT_RENDERHTMLDIR)scripti->need_graphics = 0;
        SETbuffer;
        if(script_renderdir_cmd!=NULL&&strlen(script_renderdir_cmd)>0){
          strcpy(buffer, script_renderdir_cmd);
        }
        len = strlen(buffer);
        if(len>0){
#ifdef WIN32
          for(i=0;i<len;i++){
            if(buffer[i]=='/')buffer[i]='\\';
          }
          if(buffer[len-1]!='\\')strcat(buffer,dirseparator);
#else
          for(i=0;i<len;i++){
            if(buffer[i]=='\\')buffer[i]='/';
          }
          if(buffer[len-1]!='/')strcat(buffer,dirseparator);
#endif
          scripti->cval= GetCharPointer(buffer);
        }
        }
        break;

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
        sscanf(buffer,"%i %f %i %f",&scripti->ival,&scripti->fval,&scripti->ival2,&scripti->fval2);
        break;

// RENDERCLIP
// flag left right bottom top indentations in pixels, clip if flag==1
      case SCRIPT_RENDERCLIP:
        SETbuffer;
        sscanf(buffer,"%i %i %i %i %i",&scripti->ival,&scripti->ival2,&scripti->ival3,&scripti->ival4, &scripti->ival5);
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
        sscanf(buffer, "%i", &scripti->ival);

        SETcval2;
        scripti->need_graphics = 0;
        break;

// RENDERSTART
//  start_frame (int) skip_frame (int)
      case SCRIPT_RENDERSTART:
        SETbuffer;
        sscanf(buffer,"%i %i",&scripti->ival,&scripti->ival2);
        break;

// RENDERALL
//  skip (int)
// file name base (char) (or blank to use smokeview default)
      case SCRIPT_RENDERALL:
        SETbuffer;
        scripti->ival=1;   // skip
        scripti->ival3=0;  // first frame
        sscanf(buffer,"%i %i",&scripti->ival,&scripti->ival3);
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
        sscanf(buffer, "%i %i", &scripti->ival, &scripti->ival3);
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
        sscanf(buffer,"%i %i",&scripti->ival,&scripti->ival3);
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
        sscanf(buffer, "%i %i %i", &scripti->ival, &scripti->ival3, &scripti->ival4);
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

// EXIT
      case SCRIPT_EXIT:

// UNLOADTOUR
      case SCRIPT_UNLOADTOUR:
        scripti->cval=NULL;
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

          sscanf(buffer,"%i %i %i %i %f",&scripti->ival,&scripti->ival2,&scripti->ival3,&scripti->ival4,&fv);
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

        sscanf(buffer, "%f %f %f %f %f", &scripti->fval, &scripti->fval2, &scripti->fval3, &scripti->fval4, &scripti->fval5);
        if(ABS(scripti->fval5-90)<0.1)scripti->fval5=89.9;
        if(ABS(scripti->fval5+90)<0.1)scripti->fval5=-89.9;
        break;

// SHOWPLOT3DDATA
//  mesh number (int) orientation (int)  value (0/1) (int) position (float)
      case SCRIPT_SHOWPLOT3DDATA:
        SETbuffer;
        sscanf(buffer,"%i %i %i %i %f",&scripti->ival,&scripti->ival2,&scripti->ival3,&scripti->ival4,&scripti->fval);
        if(scripti->ival2==4){
          sscanf(buffer,"%i %i %i %i %i",&scripti->ival,&scripti->ival2,&scripti->ival3,&scripti->ival4,&scripti->ival5);
        }
        break;

// LOADVOLSMOKEFRAME
//  mesh index, frame (int)
      case SCRIPT_LOADVOLSMOKEFRAME:
        SETbuffer;
        sscanf(buffer,"%i %i",&scripti->ival,&scripti->ival2);
        break;

// LOADSLICERENDER
//  (char)quantity
//  1/2/3 (int)dir  (float)position
//  (char)renderfile_base
// (int)start (int)skip (float) tmin (float)tmax
      case SCRIPT_LOADSLICERENDER:
        SETcval;

        SETbuffer;
        sscanf(buffer, "%i %f", &scripti->ival, &scripti->fval);
        scripti->ival = CLAMP(scripti->ival, 0, 3);
        scripti->need_graphics = 0;
        SETcval2;
        SETbuffer;
        scripti->fval2 = 1.0;
        scripti->fval3 = 0.0;
        sscanf(buffer, "%i %i %f %f", &scripti->ival2, &scripti->ival3, &scripti->fval2, &scripti->fval3);
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
          char *ctokens[MAX_SLCF_TOKENS];
          char *keywords[]={"QUANTITY",   "ID",         "PBX",       "PBY",       "PBZ",      "PB3D",        "AGL_SLICE",  "VECTOR",       "CELL_CENTERED"};
          int types[]={     TOKEN_STRING, TOKEN_STRING, TOKEN_FLOAT, TOKEN_FLOAT, TOKEN_FLOAT, TOKEN_LOGICAL, TOKEN_FLOAT, TOKEN_LOGICAL,   TOKEN_LOGICAL};
          int nkeywords=9, tokens[MAX_SLCF_TOKENS], itokens[MAX_SLCF_TOKENS], ntokens;
          float ftokens[MAX_SLCF_TOKENS];
          int i;

          SETbuffer;
          strcpy(buffer2, buffer);

          ntokens = ParseTokens(buffer, keywords, types, nkeywords, tokens, itokens, ftokens, &(ctokens[0]), MAX_SLCF_TOKENS);
          if(ntokens==0){
            printf("***error: problems were found parsing LOADSLCF\n");
            printf(" %s\n",buffer2);
            fatal_error = 1;
            break;
          }

          for(i=0;i<ntokens;i++){
            switch (tokens[i]){
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
		ASSERT(FFALSE);
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
        sscanf(buffer, "%i %f", &scripti->ival, &scripti->fval);
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
        sscanf(buffer, "%i %f", &scripti->ival, &scripti->fval);
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
        sscanf(buffer, "%i %f", &scripti->ival, &scripti->fval);
        scripti->ival = CLAMP(scripti->ival, 0, 3);
        SETival2;
        scripti->need_graphics = 0;
        break;

// LOADPLOT3D
//  mesh number (int) time (float)
      case SCRIPT_LOADPLOT3D:
        SETbuffer;
        sscanf(buffer," %i %f",&scripti->ival,&scripti->fval);
        scripti->need_graphics = 0;
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
        sscanf(buffer," %i %f %i %f, %s",&scripti->ival,&scripti->fval, &scripti->ival2,&scripti->fval2, scripti->quantity2);
        break;

// SETSLICEBOUNDS
//  type (char) ivalmin (int) valmin (float) ivalmax (int) valmax quantity (char)
      case SCRIPT_SETSLICEBOUNDS:
        SETbuffer;
        sscanf(buffer," %i %f %i %f, %s",&scripti->ival,&scripti->fval, &scripti->ival2,&scripti->fval2, scripti->quantity2);
        break;

// SETTOURVIEW
//   viewtype  showpath showtour_locus tension
      case SCRIPT_SETTOURVIEW:
        SETbuffer;
        sscanf(buffer,"%i %i %i %f",&scripti->ival,&scripti->ival2,&scripti->ival3,&scripti->fval);
        break;

// SETTOURKEYFRAME
//  time (float)
      case SCRIPT_SETTOURKEYFRAME:
        SETfval;
        break;

// SMOKEFRAMES
//  num (int) usesubset (int)
      case SCRIPT_SMOKEFRAMES:
        SETbuffer;
        sscanf(buffer,"%i %i %i",&scripti->ival,&scripti->ival2,&scripti->ival3);
        break;

// RGBTEST
//  x y z r g b delta
      case SCRIPT_RGBTEST:
        SETbuffer;
        sscanf(buffer, "%f %f %f %i %i %i %i", &scripti->fval, &scripti->fval2, &scripti->fval3, &scripti->ival, &scripti->ival2, &scripti->ival3, &scripti->ival4);
        break;

        // GSLICEVIEW
// show_gslice (int) show_triangles (int)  show_triangulation (int) show_normals (int)
      case SCRIPT_GSLICEVIEW:
        SETbuffer;
        sscanf(buffer,"%i %i %i %i",&scripti->ival,&scripti->ival2,&scripti->ival3,&scripti->ival4);
        break;

    // PROJECTION
        // 1/2 perspective/size preserving
     case SCRIPT_PROJECTION:
       SETbuffer;
       scripti->ival = 1;
       sscanf(buffer, "%i", &scripti->ival);
       if(scripti->ival!=2)scripti->ival = 1;
       break;

// GSLICEPOS
// x (float) y (float) z (float)
      case SCRIPT_GSLICEPOS:
        SETbuffer;
        sscanf(buffer,"%f %f %f",&scripti->fval,&scripti->fval2,&scripti->fval3);
        break;

// GSLICEORIEN
// azimuth (float) elevation (float)
      case SCRIPT_GSLICEORIEN:
        SETbuffer;
        sscanf(buffer,"%f %f",&scripti->fval,&scripti->fval2);
        break;
      default:
	ASSERT(FFALSE);
	break;
    }
    if(scriptEOF==1)break;
    if(keyword_index!=SCRIPT_UNKNOWN&&fatal_error==0)nscriptinfo++;
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
  strcat(web_filename, scripti->cval2);
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
  Smv2Html(web_filename, option, FROM_SCRIPT, VR_NO);

  GetWebFileName(webvr_filename, scripti);
  strcat(webvr_filename,"_vr.html");
#ifdef pp_HTML_VR
  Smv2Html(webvr_filename, option, FROM_SCRIPT, VR_YES);
#endif
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
  for(i = 0; i<nmeshes; i++){
    meshdata *meshi;
    volrenderdata *vr;

    if(meshnum!=i && meshnum>=0)continue;
    meshi = meshinfo+i;
    vr = &meshi->volrenderinfo;
    volframemax = MAX(volframemax,vr->ntimes);
  }
  return volframemax;
}

/* ------------------ LoadSmokeFrame ------------------------ */

void LoadSmokeFrame(int meshnum, int framenum){
  int first = 1;
  int i;
  int max_frames = -1, frame_old;
  float valtime;

  if(meshnum > nmeshes - 1||meshnum<-1)meshnum = -1;

  max_frames = GetVolFrameMax(meshnum);
  if(max_frames > 0)UpdateLoadFrameMax(max_frames);
  frame_old = framenum;
  framenum = CLAMP(framenum, 0, max_frames-1);
  if(framenum!=frame_old)UpdateLoadFrameVal(framenum);

  update_fileload = 1;
  for(i = 0; i<nmeshes; i++){
    meshdata *meshi;
    volrenderdata *vr;

    if(meshnum != i && meshnum >= 0)continue;
    meshi = meshinfo + i;
    vr = &meshi->volrenderinfo;
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
  UpdateLoadTimeVal(valtime);
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
  if(meshnum<0||meshnum>nmeshes-1)meshnum = 0;

  meshi = meshinfo+meshnum;
  vr = &meshi->volrenderinfo;

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
  if(update_timebounds==1)UpdateTimeFrameBounds(time_framemin, time_framemax);

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
  UpdateLoadFrameVal(smokeframe);
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
  int framenum, index;
  int i, count = 0;
  int fileindex;

  index = scripti->ival;
  framenum = scripti->ival2;
  fileindex = scripti->ival4;
  if(index > nmeshes - 1)index = -1;

  update_readiso_geom_wrapup = UPDATE_ISO_START_ALL;
  CancelUpdateTriangles();
  for(i = 0; i < nisoinfo; i++){
    int errorcode;
    isodata *isoi;

    isoi = isoinfo + i;
    if(isoi->isof_index + 1 == fileindex){
      ReadIso(isoi->file, i, LOAD, &framenum, &errorcode);
      count++;
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

  if(nisoinfo == 0){
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
  strcpy(movie_name, scripti->cval);
  strcpy(render_file_base,scripti->cval2);
  movie_framerate=scripti->fval;
  RenderCB(MAKE_MOVIE);
}

/* ------------------ ScriptLoadParticles ------------------------ */

void ScriptLoadParticles(scriptdata *scripti){
  int i;
  int errorcode;
  int count=0;
  int part_multithread_save;
  FREEMEMORY(loaded_file);

  PRINTF("script: loading particles files\n\n");


  part_multithread_save = part_multithread;
  part_multithread = 0;
  npartframes_max=GetMinPartFrames(PARTFILE_LOADALL);
  for(i=0;i<npartinfo;i++){
    partdata *parti;

    parti = partinfo + i;
    if(parti->evac==1)continue;
    ReadPart(parti->file,i,UNLOAD,&errorcode);
    count++;
  }
  for(i = 0;i<npartinfo;i++){
    partdata *parti;

    parti = partinfo+i;
    parti->finalize = 0;
  }
  for(i = npartinfo-1;i>=0;i--){
    partdata *parti;

    parti = partinfo+i;
    if(parti->evac==1)continue;
    parti->finalize = 1;
    break;
  }
  for(i=0;i<npartinfo;i++){
    partdata *parti;

    parti = partinfo + i;
    if(parti->evac==1)continue;
    ReadPart(parti->file,i,LOAD,&errorcode);
    if(scripti->cval!=NULL&&strlen(scripti->cval)>0){
      FREEMEMORY(loaded_file);
      NewMemory((void **)&loaded_file,strlen(scripti->cval)+1);
      strcpy(loaded_file,scripti->cval);
    }
    count++;
  }
  if(count == 0){
    fprintf(stderr, "*** Error: Particles files failed to load\n");
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: Particles files failed to load\n");
  }
  force_redisplay=1;
  UpdateFrameNumber(0);
  updatemenu=1;
  part_multithread = part_multithread_save;
}

/* ------------------ ScriptLoadIso ------------------------ */

void ScriptLoadIso(scriptdata *scripti, int meshnum){
  int i;
  int count=0;

  FREEMEMORY(loaded_file);
  PRINTF("script: loading isosurface files of type: %s\n\n",scripti->cval);

  update_readiso_geom_wrapup = UPDATE_ISO_START_ALL;
  CancelUpdateTriangles();
  for(i = 0; i<nisoinfo; i++){
    int errorcode;
    isodata *isoi;
    char label2[100];
    int lencval, lenlabel;

    isoi = isoinfo + i;
    if(meshnum != -1 && isoi->blocknumber+1 != meshnum)continue;
    lencval = strlen(scripti->cval);
    lenlabel = strlen(isoi->surface_label.longlabel);
    if(lencval<=lenlabel){
      strncpy(label2, isoi->surface_label.longlabel, lencval);
      label2[lencval] = 0;
      if(STRCMP(label2, scripti->cval)==0){
        ReadIso(isoi->file, i, LOAD, NULL, &errorcode);
        if(scripti->cval!=NULL&&strlen(scripti->cval)>0){
          FREEMEMORY(loaded_file);
          NewMemory((void **)&loaded_file, strlen(scripti->cval)+1);
          strcpy(loaded_file, scripti->cval);
        }
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
  else if(imesh>=0&&imesh<nmeshes){
    meshdata *meshi;
    volrenderdata *vr;

    meshi = meshinfo + imesh;
    vr = &meshi->volrenderinfo;
    ReadVolsmokeAllFrames(vr);
  }
}

/* ------------------ ScriptLoad3dSmoke ------------------------ */

void ScriptLoad3dSmoke(scriptdata *scripti){
  int i;
  int errorcode;
  int count=0;
  int lastsmoke;

  FREEMEMORY(loaded_file);
  PRINTF("script: loading smoke3d files of type: %s\n\n",scripti->cval);

  for(i = nsmoke3dinfo-1;i >=0;i--){
    smoke3ddata *smoke3di;

    smoke3di = smoke3dinfo + i;
    if(MatchUpper(smoke3di->label.longlabel, scripti->cval) == MATCH){
      lastsmoke = i;
      break;
    }
  }

  for(i=0;i<nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;

    smoke3di = smoke3dinfo + i;
    if(MatchUpper(smoke3di->label.longlabel,scripti->cval) == MATCH){
      smoke3di->finalize = 0;
      if(lastsmoke == i)smoke3di->finalize = 1;
      ReadSmoke3D(ALL_SMOKE_FRAMES, i, LOAD, FIRST_TIME, &errorcode);
      if(scripti->cval!=NULL&&strlen(scripti->cval)>0){
        FREEMEMORY(loaded_file);
        NewMemory((void **)&loaded_file,strlen(scripti->cval)+1);
        strcpy(loaded_file,scripti->cval);
      }
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
  ASSERT(scripti->quantity!=NULL);
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
    meshi = meshinfo+slicei->blocknumber;
    if(max[0]!=meshi->ibar||max[1]!=meshi->jbar||max[2]!=meshi->kbar)return 0;
  }
  else{
    if(slicei->slice_filetype==SLICE_TERRAIN){
      if(strcmp(scripti->c_pbxyz, "AGL_SLICE")!=0)return 0;
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

  for(i=0;i<nmultivsliceinfo;i++){
    multivslicedata *mvslicei;
    vslicedata *vslicei;
    int j;
    slicedata *slicei;

    mvslicei = multivsliceinfo + i;
    if(mvslicei->nvslices<=0)continue;
    vslicei = vsliceinfo + mvslicei->ivslices[0];
    slicei = sliceinfo + vslicei->ival;

    if(SliceMatch(scripti, slicei)==0)continue;

    for(j=0;j<mvslicei->nvslices;j++){
      vslicedata *vslicej;
      int finalize_save;

      vslicej = vsliceinfo+mvslicei->ivslices[j];
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
      PRINTF("PB3D=T\n");
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

  for(i = 0; i<nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo+i;
    slicei->finalize = 0;
  }
  for(i = nsliceinfo-1; i>=0; i--){
    slicedata *slicei;

    slicei = sliceinfo+i;
    if(SliceMatch(scripti, slicei)==0)continue;
    slicei->finalize = 1;
    break;
  }

  for(i = 0; i<nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo+i;
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

  for(i=0;i<nmultisliceinfo;i++){
    multislicedata *mslicei;
    slicedata *slicei;
    int j;

    mslicei = multisliceinfo + i;
    if(mslicei->nslices<=0)continue;
    slicei = sliceinfo + mslicei->islices[0];
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

      slicej = sliceinfo+mslicei->islices[j];
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
      FREEMEMORY(loaded_file);
      slicej = sliceinfo + mslicei->islices[j];
      if(slicej->file != NULL&&strlen(slicej->file) > 0){
        NewMemory((void **)&loaded_file, strlen(slicej->file) + 1);
        strcpy(loaded_file, slicej->file);
      }
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

    for(i = 0; i<nsliceinfo; i++){
      slicedata *slicei;
      char *slice_type;
      FILE *stream;

      slicei = sliceinfo+i;
      slice_type = slicei->label.shortlabel;
      if(strcmp(type, slice_type)!=0)continue;
      stream = fopen(slicei->bound_file, "r");
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
  for(i = 0; i<nmultisliceinfo; i++){
    multislicedata *mslicei;
    slicedata *slicei;
    int j;

    mslicei = multisliceinfo+i;
    if(mslicei->nslices<=0)continue;
    slicei = sliceinfo+mslicei->islices[0];
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
      slicei = sliceinfo+mslicei->islices[j];
      if(slicei->nframes==0){
        if(slicei->slice_filetype==SLICE_GEOM){
          int nvals, error;

          slicei->nframes = GetGeomDataSize(slicei->file, &nvals, &scripti->fval2, &scripti->fval3, ALL_FRAMES, NULL, NULL,  &error);
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
    for(i = 0; i<nsliceinfo; i++){
      slicedata *slicei;

      slicei = sliceinfo+i;
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

  for(i = 0; i<nmultisliceinfo; i++){
    multislicedata *mslicei;
    slicedata *slicei;
    int j;

    mslicei = multisliceinfo+i;
    if(mslicei->nslices<=0)continue;
    slicei = sliceinfo+mslicei->islices[0];
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

      slicej = sliceinfo+mslicei->islices[j];
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
      FREEMEMORY(loaded_file);
      if(slicej->file!=NULL&&strlen(slicej->file)>0){
        NewMemory((void **)&loaded_file, strlen(slicej->file)+1);
        strcpy(loaded_file, slicej->file);
      }
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
    valid_frame = 0;
    RenderState(RENDER_OFF);
  }
}

/* ------------------ ScriptLoadSliceM ------------------------ */

void ScriptLoadSliceM(scriptdata *scripti, int meshnum){
  int i;

  PRINTF("script: loading slice files of type: %s in mesh %i\n\n", scripti->cval,meshnum);

  for(i = 0; i < nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo + i;
    if(slicei->blocknumber + 1 != meshnum)continue;
    if(MatchUpper(slicei->label.longlabel, scripti->cval) == NOTMATCH)continue;
    if(scripti->ival == 0){
      int *min, *max;
      meshdata *meshi;

      if(slicei->volslice == 0)continue;
      min = slicei->ijk_min;
      max = slicei->ijk_max;
      if(min[0] != 0 || min[1] != 0 || min[2] != 0)continue;
      meshi = meshinfo + slicei->blocknumber;
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

  for(i=0;i<nmultivsliceinfo;i++){
    multivslicedata *mvslicei;
    vslicedata *vslicei;
    int j;
    slicedata *slicei;

    mvslicei = multivsliceinfo + i;
    if(mvslicei->nvslices<=0)continue;
    vslicei = vsliceinfo + mvslicei->ivslices[0];
    slicei = sliceinfo + vslicei->ival;
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

      vslicej = vsliceinfo+mvslicei->ivslices[j];
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

  for(i=0;i<nmultivsliceinfo;i++){
    multivslicedata *mvslicei;
    vslicedata *vslicei;
    int j;
    slicedata *slicei;

    mvslicei = multivsliceinfo + i;
    if(mvslicei->nvslices<=0)continue;
    vslicei = vsliceinfo + mvslicei->ivslices[0];
    slicei = sliceinfo + vslicei->ival;
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

  for(i=0;i<ntourinfo;i++){
    tourdata *touri;

    touri = tourinfo + i;
    if(strcmp(touri->label,scripti->cval)==0){
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

  FREEMEMORY(loaded_file);
  PRINTF("Script: loading boundary files of type: %s\n\n",scripti->cval);

  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(meshnum == -1 || patchi->blocknumber + 1 == meshnum){
      if(strcmp(patchi->label.longlabel, scripti->cval) == 0){
        LOCK_COMPRESS
          ReadBoundary(i, LOAD, &errorcode);
        if(scripti->cval != NULL&&strlen(scripti->cval) > 0){
          FREEMEMORY(loaded_file);
          NewMemory((void **)&loaded_file, strlen(scripti->cval) + 1);
          strcpy(loaded_file, scripti->cval);
        }
        count++;
        UNLOCK_COMPRESS
          if(meshnum == -1)break;
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
  int count=0;

  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(propi->particle_property==0)continue;
    if(strcmp(propi->label->longlabel,scripti->cval)==0){
      ParticlePropShowMenu(i);
      count++;
    }
  }
  if(count == 0){
    fprintf(stderr, "*** Error: particle class color: %s failed to be set\n", scripti->cval);
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: particle class color: %s failed to be set\n", scripti->cval);
  }

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
  UpdatePlot3dListIndex();

  vecfactor=1.0;
  if(scripti->fval>=0.0)vecfactor=scripti->fval;
  UpdateVectorWidgets();

  PRINTF("script: vecfactor=%f\n",vecfactor);

  contour_type=CLAMP(scripti->ival4,0,2);
  UpdatePlot3dDisplay();

  if(visVector==1&&nplot3dloaded>0){
    meshdata *gbsave,*gbi;

    gbsave=current_mesh;
    for(i=0;i<nmeshes;i++){
      gbi = meshinfo + i;
      if(gbi->plot3dfilenum==-1)continue;
      UpdateCurrentMesh(gbi);
      UpdatePlotSlice(XDIR);
      UpdatePlotSlice(YDIR);
      UpdatePlotSlice(ZDIR);
    }
    UpdateCurrentMesh(gbsave);
  }
}

/* ------------------ ScriptShowSmokeSensors ------------------------ */

void ScriptShowSmokeSensors(void){
  int i,j;
  FILE *stream_smokesensors;
  int nsmokesensors;
  float sensor_time=0.0;

  // count smokesensors

  nsmokesensors=0;
  for(i=0;i<ndeviceinfo;i++){
    devicedata *devicei;

    devicei = deviceinfo + i;
    if(STRCMP(devicei->object->label,"smokesensor")==0)nsmokesensors++;
  }
  if(nsmokesensors == 0)return;

  // first time, create a file to put smokesensor values in

  if(file_smokesensors==NULL){
    NewMemory((void **)&file_smokesensors,strlen(fdsprefix)+17+1);
    strcpy(file_smokesensors,fdsprefix);
    strcat(file_smokesensors,"_ss.csv");
    stream_smokesensors = fopen(file_smokesensors, "w");

    fprintf(stream_smokesensors, "s,");
    for(i = 1;i < nsmokesensors-1;i++){
      fprintf(stream_smokesensors, ",");
    }
    fprintf(stream_smokesensors, "\n");

    j = 0;
    fprintf(stream_smokesensors, "Time,");
    for(i = 0;i < ndeviceinfo;i++){
      devicedata *devicei;

      devicei = deviceinfo + i;
      if(STRCMP(devicei->object->label, "smokesensor") == 0){
        j++;
        if(j == nsmokesensors){
          fprintf(stream_smokesensors, "%s\n",devicei->label);
        }
        else{
          fprintf(stream_smokesensors, "%s,", devicei->label);
        }
      }
    }
  }
  else{
    stream_smokesensors = fopen(file_smokesensors, "a");
  }

  if(global_times!=NULL&&itimes>=0&&itimes<nglobal_times){
    sensor_time = global_times[itimes];
  }
  fprintf(stream_smokesensors,"%f,",sensor_time);
  j = 0;
  for(i=0;i<ndeviceinfo;i++){
    devicedata *devicei;

    devicei = deviceinfo + i;
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
  SceneMotionCB(CUSTOM_VIEW);
  ViewpointCB(RESTORE_VIEW);
  set_view_xyz[0]      = x;
  set_view_xyz[1]      = y;
  set_view_xyz[2]      = z;
  customview_azimuth   = az;
  customview_elevation = elev;
  use_customview       = 1;
  SceneMotionCB(CUSTOM_VIEW);
  SceneMotionCB(SET_VIEW_XYZ);
  UpdatePosView();
}

/* ------------------ ScriptShowPlot3dData ------------------------ */

void ScriptShowPlot3dData(scriptdata *scripti){
  meshdata *meshi;
  int imesh, dir, showhide;
  float val;
  int isolevel;

  imesh = scripti->ival-1;
  if(imesh<0||imesh>nmeshes-1)return;

  meshi = meshinfo + imesh;
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
      ASSERT(FFALSE);
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
    for(j=0;j<npartclassinfo;j++){
      partclassdata *partclassj;

      if(propi->class_present[j]==0)continue;
      partclassj = partclassinfo + j;
      if(partclassj->kind==HUMANS)continue;
      if(strcmp(partclassj->name,scripti->cval)==0){
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

/* ------------------ ScriptLoadIniFile ------------------------ */

void ScriptLoadIniFile(scriptdata *scripti){
  PRINTF("script: loading ini file %s\n\n",scripti->cval);
  windowresized=0;
  ReadIni(scripti->cval);
}

/* ------------------ ScriptLoadFile ------------------------ */

void ScriptLoadFile(scriptdata *scripti){
  int i;
  int errorcode;

  FREEMEMORY(loaded_file);
  PRINTF("script: loading file %s\n\n",scripti->cval);
  if(scripti->cval!=NULL&&strlen(scripti->cval)>0){
    NewMemory((void **)&loaded_file,strlen(scripti->cval)+1);
    strcpy(loaded_file,scripti->cval);
  }
  for(i=0;i<nsliceinfo;i++){
    slicedata *sd;

    sd = sliceinfo + i;
    if(strcmp(sd->file,scripti->cval)==0){
      if(i<nsliceinfo-nfedinfo){
        ReadSlice(sd->file,i, ALL_FRAMES, NULL, LOAD, SET_SLICECOLOR,&errorcode);
      }
      else{
        ReadFed(i, ALL_FRAMES, NULL, LOAD,FED_SLICE,&errorcode);
      }
      return;
    }
  }
  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(strcmp(patchi->file,scripti->cval)==0){
      ReadBoundary(i,LOAD,&errorcode);
      return;
    }
  }
  npartframes_max=GetMinPartFrames(PARTFILE_LOADALL);
  for(i=0;i<npartinfo;i++){
    partdata *parti;

    parti = partinfo + i;
    if(strcmp(parti->file,scripti->cval)==0){
      LoadParticleMenu(i);
      return;
    }
  }
  CancelUpdateTriangles();
  for(i=0;i<nisoinfo;i++){
    isodata *isoi;

    isoi = isoinfo + i;
    if(strcmp(isoi->file,scripti->cval)==0){
      ReadIso(isoi->file,i,LOAD,NULL,&errorcode);
      if(update_readiso_geom_wrapup == UPDATE_ISO_ONE_NOW)ReadIsoGeomWrapup(FOREGROUND);
      return;
    }
  }
  for(i=0;i<nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;

    smoke3di = smoke3dinfo + i;
    if(strcmp(smoke3di->file,scripti->cval)==0){
      smoke3di->finalize = 1;
      ReadSmoke3D(ALL_SMOKE_FRAMES, i, LOAD, FIRST_TIME, &errorcode);
      return;
    }
  }
  for(i=0;i<nzoneinfo;i++){
    zonedata *zonei;

    zonei = zoneinfo + i;
    if(strcmp(zonei->file,scripti->cval)==0){
      ReadZone(i,LOAD,&errorcode);
      return;
    }
  }
  for(i=0;i<nplot3dinfo;i++){
    plot3ddata *plot3di;

    plot3di = plot3dinfo + i;
    if(strcmp(plot3di->file,scripti->cval)==0){
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

  FREEMEMORY(script_labelstring);
  if(scripti->cval!=NULL&&strlen(scripti->cval)>0){
    NewMemory((void **)&script_labelstring,strlen(scripti->cval)+1);
    strcpy(script_labelstring,scripti->cval);
    PRINTF("*******************************\n");
    PRINTF("*** %s ***\n",script_labelstring);
    PRINTF("*******************************\n");
  }
}

/* ------------------ ScriptLoadPlot3d ------------------------ */

void ScriptLoadPlot3D(scriptdata *scripti){
  int i;
  float time_local;
  int blocknum;
  int count=0;

  time_local = scripti->fval;
  blocknum = scripti->ival-1;

  for(i=0;i<nplot3dinfo;i++){
    plot3ddata *plot3di;

    plot3di = plot3dinfo + i;
    if(plot3di->blocknumber==blocknum&&ABS(plot3di->time-time_local)<0.5){
      count++;
      LoadPlot3dMenu(i);
    }
  }
  UpdateRGBColors(COLORBAR_INDEX_NONE);
  SetLabelControls();
  if(count == 0){
    fprintf(stderr, "*** Error: Plot3d file failed to load\n");
    if(stderr2!=NULL)fprintf(stderr2, "*** Error: Plot3d file failed to load\n");
  }

  //UpdateMenu();
}

/* ------------------ ScriptLoadVecFile ------------------------ */

void ScriptLoadVecFile(scriptdata *scripti){
  int i;

  FREEMEMORY(loaded_file);
  PRINTF("script: loading vector slice file %s\n\n",scripti->cval);
  for(i=0;i<nvsliceinfo;i++){
    slicedata *val;
    vslicedata *vslicei;

    vslicei = vsliceinfo + i;
    val = sliceinfo + vslicei->ival;
    if(val==NULL)continue;
    if(strcmp(val->reg_file,scripti->cval)==0){
      LoadVSliceMenu(i);
      if(scripti->cval!=NULL&&strlen(scripti->cval)>0){
        NewMemory((void **)&loaded_file,strlen(scripti->cval)+1);
        strcpy(loaded_file,scripti->cval);
      }
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
    SetGluiTourKeyframe();
    UpdateTourControls();
  }
}

/* ------------------ ScriptSetTourView ------------------------ */

void ScriptSetTourView(scriptdata *scripti){
  edittour=scripti->ival;
  show_avatar =scripti->ival3;
  tour_global_tension_flag=1;
  tour_global_tension=scripti->fval;
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
  UpdateTourState();
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
  char message[255];

  timeval = scripti->fval;
  updatetimes_debug = message;
  UpdateTimes();
  updatetimes_debug = NULL;
  PRINTF("script: setting time to %f\n\n",timeval);
  if(global_times!=NULL&&nglobal_times>0){
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
        if(stderr2!=NULL)fprintf(stderr2, "all times: ");

        for(i=0;i<nglobal_times;i++){
          fprintf(stderr,"%f ",global_times[i]);
          if(stderr2!=NULL)fprintf(stderr2, "%f ", global_times[i]);
        }
        fprintf(stderr," ***\n");
        if(stderr2!=NULL)fprintf(stderr2, " ***\n");
        if(loaded_file != NULL){
          fprintf(stderr, "           loaded file: %s\n", loaded_file);
          if(stderr2!=NULL)fprintf(stderr2, "           loaded file: %s\n", loaded_file);
        }
        if(script_labelstring != NULL){
          fprintf(stderr, "                 label: %s\n", script_labelstring);
          if(stderr2!=NULL)fprintf(stderr2, "                 label: %s\n", script_labelstring);
        }
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
  }
}

/* ------------------ ScriptProjection ------------------------ */

void ScriptProjection(scriptdata *scripti){
  if(scripti->ival==1){
    projection_type = PROJECTION_PERSPECTIVE;
  }
  else{
    projection_type = PROJECTION_ORTHOGRAPHIC;
  }
  SceneMotionCB(PROJECTION);
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

/* ------------------ ScriptSmokeframes ------------------------ */

void ScriptSmokeframes(scriptdata *scripti){
  smoke_num = scripti->ival;
  smoke_subset = scripti->ival2;
  use_newsmoke = scripti->ival3;
  Smoke3dCB(SMOKE_NEW);
}

/* ------------------ ScriptRGBtest ------------------------ */

void ScriptRGBtest(scriptdata *scripti){
  update_rgb_test = 1;
  rgb_test_xyz[0] = scripti->fval;
  rgb_test_xyz[1] = scripti->fval2;
  rgb_test_xyz[2] = scripti->fval3;
  NORMALIZE_XYZ(rgb_test_xyz, rgb_test_xyz);
  rgb_test_rgb[0] = scripti->ival;
  rgb_test_rgb[1] = scripti->ival2;
  rgb_test_rgb[2] = scripti->ival3;
  rgb_test_delta  = scripti->ival4;
  use_lighting = 0;
}

/* ------------------ ScriptSetViewpoint ------------------------ */

void ScriptSetViewpoint(scriptdata *scripti){
  char *viewpoint;

  viewpoint = scripti->cval;
  update_viewpoint_script = 3;
  strcpy(viewpoint_script, viewpoint);
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
    ASSERT(FFALSE);
    break;
  }
  ResetGluiView(EXTERNAL_VIEW);
  use_customview=0;
  SceneMotionCB(CUSTOM_VIEW);
  SceneMotionCB(ZAXIS_CUSTOM);
}

/* ------------------ ScriptViewXYZMINMAXPersp ------------------------ */
void ResetDefaultMenu(int var);
void ScriptViewXYZMINMAXPersp(int command){
  switch (command){
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
    ASSERT(FFALSE);
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
  ResetGluiView(EXTERNAL_VIEW);

  use_customview = 0;
  SceneMotionCB(CUSTOM_VIEW);
  ViewpointCB(RESTORE_VIEW);
  set_view_xyz[0]      = xcen;
  set_view_xyz[1]      = ycen;
  set_view_xyz[2]      = zcen;
  customview_azimuth   = azimuth;
  customview_elevation = elevation;
  use_customview       = 1;
  SceneMotionCB(CUSTOM_VIEW);
  SceneMotionCB(SET_VIEW_XYZ);
  UpdatePosView();
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
        }
        PRINTF("script: setting html render path to %s\n", script_htmldir_path);
      }
      else{
        script_htmldir_path = NULL;
      }
      break;
    case SCRIPT_KEYBOARD:
      {
        char *key;

        script_keystate=0;
        key = scripti->cval + strlen(scripti->cval) - 1;
        if(strncmp(scripti->cval,"ALT",3)==0)script_keystate=GLUT_ACTIVE_ALT;

        Keyboard(*key,FROM_SCRIPT);
        returnval=1;
      }
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
    case SCRIPT_SHOWSMOKESENSORS:
      ScriptShowSmokeSensors();
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
    case SCRIPT_LOADSLICE:
      ScriptLoadSlice(scripti);
      break;
    case SCRIPT_LOADSLICERENDER:
 //   Since ScriptLoadSliceRender is called multiple times, call this routine from DoScripts in callback.c
 //     ScriptLoadSliceRender(scripti);
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
    case SCRIPT_SETBOUNDBOUNDS:
      ScriptSetBoundBounds(scripti);
      break;
    case SCRIPT_SETTOURVIEW:
      ScriptSetTourView(scripti);
      break;
    case SCRIPT_SETTOURKEYFRAME:
      ScriptSetTourKeyFrame(scripti);
      break;
    case SCRIPT_SETVIEWPOINT:
      ScriptSetViewpoint(scripti);
      break;
    case SCRIPT_SMOKEFRAMES:
      ScriptSmokeframes(scripti);
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
      ASSERT(FFALSE);
      break;
  }
  GLUTPOSTREDISPLAY;
  return returnval;
}
