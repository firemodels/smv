#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "svdiff.h"
#include "string_util.h"
#include "dmalloc.h"
#include "stdio_buffer.h"

/* ------------------ usage ------------------------ */

void Usage(int option){
  char githash[100];
  char gitdate[100];

  GetGitInfo(githash, gitdate);    // get githash

  PRINTF("\n");
  PRINTF("smokediff [options] smv_case1 smv_case2\n");
  PRINTF("%s - %s\n\n", githash, __DATE__);

  PRINTF("smokediff compares two FDS cases by subtracting data referenced in smv_case2 from\n");
  PRINTF("corresponding data referenced in smv_case1 (smv_case1 - smv_case2).  Slice, PLOT3d\n");
  PRINTF("and boundary files are supported.  Differenced results may be viewed by opening\n");
  PRINTF("smv_case1_diff.smv in Smokeview or by using the -smv option when running smokediff.\n\n");

  PRINTF("Mesh bounds must be identical for corresponding meshes.  Mesh resolutions must be\n");
  PRINTF("identical when differencing boundary and PLOT3D files.  The x, y, and z mesh\n");
  PRINTF("resolutions in smv_case2 must be integer multiples of the corresponding x, y, z mesh\n");
  PRINTF("resolutions in smv_case1 when differencing slice files.\n\n");

  printf("options:\n");
  UsageCommon(HELP_SUMMARY);

  if(option == HELP_ALL){
    PRINTF("  -s1 dir1 - directory containing case smv_case1.smv\n");
    PRINTF("  -s2 dir2 - directory containing case smv_case2.smv\n");
    PRINTF("  -d  dir  - directory containing created differenced files\n");
    PRINTF("  -nb      - do not difference boundary files\n");
    PRINTF("  -np      - do not difference Plot3d files\n");
    PRINTF("  -ns      - do not difference slice files\n");
    PRINTF("  -smv     - view case in smokeview when differencing is complete\n");
    PRINTF("  -type label - difference only data of type label (in boundary and slice files)\n");
    UsageCommon(HELP_ALL);
  }
  PRINTF("\n  smv_case1,smv_case2 - Two smokeview cases to compare.\n");
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){

  char *smv1=NULL, *smv2=NULL, *arg;
  bufferstreamdata *smv_buffer1, *smv_buffer2;
  char smv1_out[1024];
  char svdlogfile[1024];
  char *smoke1=NULL, *smoke2=NULL, smv_out[1024];
  char smoke1a[1024], smoke2a[1024];
  char smoke1b[1024], smoke2b[1024];
  char fed_smoke1[1024], fed_smoke2[1024];

  FILE *stream_out, *stream_in1, *stream_in2;
  int no_plot3d=0, no_slice=0, no_boundary=0;
  int i;
  int open_smokeview=0;
  int redirect=0;

  display_warnings=1;
  SetStdOut(stdout);
  initMALLOC();
#ifdef _WIN32
  strcpy(dirseparator,"\\");
#else
  strcpy(dirseparator,"/");
#endif
  strcpy(pp,"%");

  common_opts opts = ParseCommonOptions(argc, argv);
  if(opts.show_help!=0){
    Usage(opts.show_help);
    return 0;
  }
  if(opts.show_version==1){
    PRINTVERSION("smokediff", &opts);
    return 0;
  }

  NewMemory((void **)&caseinfo,2*sizeof(casedata));


 // check_histogram();
  test_mode=0;
  sourcedir1=NULL;
  sourcedir2=NULL;
  destdir=NULL;
  strcpy(type_label,"");

  if(argc==1){
    PRINTVERSION("Smokediff ", &opts);
    return 0;
  }

/* -e{850} loop index i is modified within loop */
  for(i=1;i<argc;i++){
    arg=argv[i];
    if(arg[0]=='-'&&strlen(arg)>1){
      char *key;

      key = arg+1;
      switch(arg[1]){
      case 't':
        if(strcmp(key,"type")==0){
          char *label;

          i++;
          strcpy(type_label,"");
          if(i<argc){
            label=argv[i];
            if(label!=NULL&&strlen(label)>0){
              strcpy(type_label,label);
            }
          }
        }
        else{
          test_mode=1;
        }
        break;
      case 'n':
        if(arg[2]=='p'){
          no_plot3d=1;
        }
        else if(arg[2]=='s'){
          no_slice=1;
        }
        else if(arg[2]=='b'){
          no_boundary=1;
        }
        else{
          Usage(HELP_ALL);
          return 1;
        }
        break;
      case 'r':
        redirect=1;
        break;
      case 's':
        if(arg[2]=='m'&&arg[3]=='v'){
          open_smokeview=1;
          break;
        }
        if(i+1>=argc)break;
        if(arg[2]=='1'){
          sourcedir1= SetDir(argv[i+1]);
          if(sourcedir1==NULL)return 1;
          i++;
        }
        if(arg[2]=='2'){
          sourcedir2= SetDir(argv[i+1]);
          if(sourcedir2==NULL)return 1;
          i++;
        }
        break;
      case 'd':
        if(i+1>=argc)break;
        destdir= SetDir(argv[i+1]);
        if(destdir==NULL)return 1;
        i++;
        break;
      case 'w':
        display_warnings=0;
        break;
      default:
        Usage(HELP_ALL);
        return 1;
      }
    }
    else{
      if(smv1==NULL){
        smv1=argv[i];
      }
      else{
        smv2=argv[i];
      }
    }
  }

  smoke1=NULL;
  strcpy(smv1_out,"");
  if(smv1!=NULL){
    strcat(smv1_out,smv1);
    strcat(smv1_out,".smv");
    FullFile(smoke1a,sourcedir1,smv1);

    strcpy(fed_smoke1,smoke1a);
    strcat(fed_smoke1,".fed_smv");

    strcpy(smoke1b,smoke1a);
    strcat(smoke1b,".smvtmp");

    strcat(smoke1a,".smv");
    smoke1 = smoke1a;

    if(FILE_EXISTS(fed_smoke1)==YES){
      CopyFILE(".",smoke1a, smoke1b, REPLACE_FILE);
      CopyFILE(".",fed_smoke1, smoke1b, APPEND_FILE);
      smoke1=smoke1b;
    }
  }
  if(smv2!=NULL){
    FullFile(smoke2a,sourcedir2,smv2);

    strcpy(fed_smoke2,smoke2a);
    strcat(fed_smoke2,".fed_smv");

    strcpy(smoke2b,smoke2a);
    strcat(smoke2b,".smvtmp");
    strcat(smoke2a,".smv");
    smoke2 = smoke2a;

    if(FILE_EXISTS(fed_smoke2)==YES){
      CopyFILE(".",smoke2a, smoke2b, REPLACE_FILE);
      CopyFILE(".",fed_smoke2, smoke2b, APPEND_FILE);
      smoke2=smoke2b;
    }
  }
  // make sure smv file names exists

  if(redirect==1){
    strcpy(svdlogfile,"");
    if(destdir!=NULL)strcat(svdlogfile,destdir);
    if(smv1!=NULL)strcat(svdlogfile,smv1);
    strcat(svdlogfile,"_diff.svdlog");
    LOG_FILENAME=FOPEN(svdlogfile,"w");
    if(LOG_FILENAME!=NULL){
      SetStdOut(LOG_FILENAME);
    }
  }
  if(GetFileInfo(smoke1,NULL,NULL)!=0||GetFileInfo(smoke2,NULL,NULL)!=0){
    if(GetFileInfo(smoke1,NULL,NULL)!=0){
      fprintf(stderr,"*** Error The .smv file, %s, does not exist\n",smoke1);
    }
    if(GetFileInfo(smoke2,NULL,NULL)!=0){
      fprintf(stderr,"*** Error The .smv file, %s, does not exist\n",smoke2);
    }
    return 1;
  }
  MakeOutFile(smv_out,destdir,smv1_out,".smv");

  stream_out=FOPEN(smv_out,"w");
  if(stream_out==NULL){
    fprintf(stderr,"*** Error The .smv file, %s, could not be opened for output.\n",smv_out);
  }
  stream_in1=FOPEN(smoke1,"r");
  if(stream_in1==NULL){
    fprintf(stderr,"*** Error The .smv file, %s, could not be opened for input\n",smoke1);
  }
  stream_in2=FOPEN(smoke2,"r");
  if(stream_in2==NULL){
    fprintf(stderr,"*** Error The .smv file, %s, could not be opened for input.\n",smoke2);
  }
  if(stream_out==NULL||stream_in1==NULL||stream_in2==NULL){
    if(stream_out!=NULL)fclose(stream_out);
    if(stream_in1!=NULL)fclose(stream_in1);
    if(stream_in2!=NULL)fclose(stream_in2);
    return 1;
  }
  fclose(stream_in1);
  fclose(stream_in2);

  caseinfo[0].dir=sourcedir1;
  caseinfo[1].dir=sourcedir2;

  PRINTF("reading %s\n",smoke1);
  FFLUSH();

  smv_buffer1 = GetSMVBuffer(smoke1);

  ReadSMV(smv_buffer1, stream_out, caseinfo);
  FCLOSE(smv_buffer1);

  PRINTF("reading %s\n",smoke2);
  FFLUSH();
  smv_buffer2 = GetSMVBuffer(smoke2);
  ReadSMV(smv_buffer2, NULL, caseinfo+1);
  FCLOSE(smv_buffer2);

  if(no_slice==0){
    SetupSlice(stream_out);
    DiffSlices(stream_out);
  }
  if(no_boundary==0){
    SetupBoundary(stream_out);
    DiffBoundarYes(stream_out);
  }
  if(no_plot3d==0){
    SetupPlot3D(stream_out);
    DiffPlot3Ds(stream_out);
  }

  fclose(stream_out);
  if(open_smokeview==1){
    char command[1024];

    strcpy(command,"smokeview ");
    strcat(command,smv_out);
    system(command);
  }

  return 0;
}
