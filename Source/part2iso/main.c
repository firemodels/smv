#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "part2iso.h"
#include "dmalloc.h"

void Usage(int option){
  PRINTF("part2iso [options] casename\n");
  PRINTF("Generate isosurfaces from particle data referenced by casename.smv\n\n");
  PRINTF("  -s sourcedir - directory containing the case and particle files\n");
  UsageCommon(option);
}

int main(int argc, char **argv){
  char *filebase=NULL;
  char smvfile[1024];
  int i;
  common_opts opts;
  SetStdOut(stdout);
  initMALLOC();
  opts=ParseCommonOptions(argc,argv);
  if(opts.show_help!=0){ Usage(opts.show_help); return 0; }
  if(opts.show_version==1){ PRINTVERSION("part2iso",&opts); return 0; }
  for(i=1; i<argc; i++){
    if(strcmp(argv[i],"-s")==0&&i+1<argc){
      size_t len;
      i++;
      len=strlen(argv[i]);
      if(len==0)return 1;
      NewMemory((void **)&GLOBsourcedir,len+2);
      strcpy(GLOBsourcedir,argv[i]);
      if(GLOBsourcedir[len-1]!=dirseparator[0])strcat(GLOBsourcedir,dirseparator);
    }
    else if(strcmp(argv[i],"-part2iso")==0){
      /* Accepted for compatibility; conversion is always enabled. */
    }
    else if(argv[i][0]=='-'||filebase!=NULL){ Usage(HELP_ALL); return 1; }
    else{ filebase=argv[i]; }
  }
  if(filebase==NULL){ Usage(HELP_ALL); return 1; }
  if(snprintf(smvfile,sizeof(smvfile),"%s%s",GLOBsourcedir==NULL?"":GLOBsourcedir,filebase)>=(int)sizeof(smvfile)-8){
    fprintf(stderr,"*** Error: case path is too long\n");
    return 1;
  }
  if(strlen(smvfile)>4&&strcmp(smvfile+strlen(smvfile)-4,".smv")==0)smvfile[strlen(smvfile)-4]=0;
  strcpy(GLOBsmvisofile,smvfile);
  strcat(GLOBsmvisofile,".isosmv");
  strcat(smvfile,".smv");
  strcpy(GLOBpp,"%");
  GLOBfirst_part2iso_smvopen=1;
  if(ReadSMV(smvfile)!=0)return 1;
  for(i=0; i<npartinfo; i++)Part2Iso(partinfo+i);
  return 0;
}