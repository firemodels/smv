#define IN_MAIN
#include "options.h"
#include <stdio.h>
#include <string.h>
#include "string_util.h"
#include "dmalloc.h"

int add_msgstring=0;
int add_comments=0;

/* ------------------ Usage ------------------------ */

void Usage(char *prog, int option){
  char githash[1024];
  char gitdate[1024];

  GetGitInfo(githash,gitdate);

  fprintf(stdout, "\n%s (%s) %s \n",prog, githash, __DATE__);
  fprintf(stdout,"Create a .po file by parsing a collection of .c/.h/.cpp files\n");
  fprintf(stdout,"looking for strings of the form _(\"....\") , outputting each\n");
  fprintf(stdout,"string found as \n");
  fprintf(stdout,"MSGID \".....\"\n\n");
  fprintf(stdout,"Usage:\n");
  fprintf(stdout,"  makepo [options]\n");
  fprintf(stdout,"  -a  - the string\n");
  fprintf(stdout,"MSGSTR \"\"\n");
  fprintf(stdout,"is also output\n");
  UsageCommon(HELP_SUMMARY);
  if(option == HELP_ALL){
    UsageCommon(HELP_ALL);
  }
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  char buffer[1024];
  int ii;
  char *arg,*prog;
  FILE *stream;

  initMALLOC();
  SetStdOut(stdout);

  stream=stdin;
  prog=argv[0];
  ParseCommonOptions(argc, argv);
  if(show_help!=0){
    Usage("makepo",show_help);
    return 1;
  }
  if(show_version==1){
    PRINTVERSION("makepo");
    return 1;
  }
  for(ii=1;ii<argc;ii++){
    int lenarg;

    arg=argv[ii];
    lenarg=strlen(arg);
    if(arg[0]=='-'&&lenarg>1){
      switch(arg[1]){
      case 'a':
        add_msgstring=1;
        break;
      case 'c':
        add_comments=1;
        break;
      default:
        Usage(prog,HELP_ALL);
        return 1;
      }
    }
    else{
      Usage(prog,HELP_ALL);
      return 1;
    }
  }
  if(add_comments==1){
    printf("// Translate the phrase after each msgid and place\n");
    printf("// it on the following line after msgstr, as in:\n");
    printf("// msgid \"original phrase\"\n");
    printf("// msgstr \"translated phrase\"\n");
    printf("// \n");
    printf("// If you have trouble with some of the terms feel free\n");
    printf("// to ask.  It is also OK to leave troublesome terms\n");
    printf("// untranslated, they will be output in English\n");
    printf("// \n");
    printf("// Suggested translation priorities:\n");
    printf("// 1.  translate terms in menus, e.g. Load/Unload, \n");
    printf("//     Show/Hide etc.\n");
    printf("// 2.  translate terms in dialog boxes.\n");
    printf("// 3.  Now go through the following list and translate terms.\n");
    printf("//     that are left.  \n");
    printf("// \n");
  }
  if(add_msgstring==0){
    while(!feof(stream)){
      char *beg,*end, *beg2;

      fgets(buffer,sizeof(buffer),stream);
      beg=strstr(buffer,"_(\"");
      if(beg==NULL)continue;
      beg+=3;
      for(beg2=beg;beg2<buffer+sizeof(buffer);beg2++){
        char c;

        c = *beg2;
        if((c>='a'&&c<='z')||(c>='A'&&c<='Z'))break;
        if((c=='1'||c=='2'||c=='3')&&(beg2[1]=='D'||beg2[1]=='d')){
          beg2+=2;
        }
      }
      end=strstr(beg+1,"\"");
      if(end!=NULL){
        int len;

        end[0]=0;
        TrimBack(beg2);
        len=strlen(beg2);
        if(len>0&&beg2[len-1]==':')beg2[len-1]=0;
        len=strlen(beg2);
        if(len>3){
          char *match;

          match=strstr(beg2+len-3,"...");
          if(match!=NULL)match[0]=0;
        }
        printf("msgid  \"%s\"\n",beg2);
      }
    }
  }
  else{
    while(!feof(stream)){
      char *beg;

      fgets(buffer,sizeof(buffer),stream);
      TrimBack(buffer);
      beg=strstr(buffer,"msgid");
      if(beg!=NULL&&beg==buffer){
        printf("%s\n",buffer);
        printf("msgstr \"\"\n");
        printf("\n");
      }
    }
  }
}
