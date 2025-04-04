#define INMAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datadefs.h"
#include "string_util.h"
#include "file_util.h"
#include "dmalloc.h"

//dummy change to bump version number to  0.9

/* ------------------ Usage ------------------------ */

void Usage(int option){
  char githash[100];
  char gitdate[100];

  GetGitInfo(githash,gitdate);    // get githash

  printf("\n");
  printf("wind2fds [-prefix label] [-offset x y z] input_file [output_file]\n");
  printf("%s - %s\n\n", githash, __DATE__);
  printf("Convert spreadsheets containing wind data to files compatible with Smokeview:\n\n");

  printf("options:\n");
  printf("  -prefix label  - prefix column headers with label\n");
  printf("  -offset x y z  - offset sensor locations by (x,y,z)\n");
  UsageCommon(HELP_SUMMARY);
  if(option == HELP_ALL){
    printf("  -wv            - converting a non-sodar file\n");
    printf("  -date mm/dd/yyyy - only convert data recorded on mm/dd/yyyy\n");
    printf("  -mindate \"mm/dd/yyyy [hh:mm:ss]\" - ignore data recorded before specified date\n");
    printf("  -maxdate \"mm/dd/yyyy [hh:mm:ss]\" - ignore data recorded after specified date\n");
    printf("  -mintime \"hh:mm:ss\" - ignore data recorded before specified time (on any date)\n");
    printf("  -maxtime \"hh:mm:ss\" - ignore data recorded after specified time (on any date)\n");
    UsageCommon(HELP_ALL);
  }
  printf("\n  datafile.csv   - spreadsheet file to be converted. Use '-' to input data\n");
  printf("                   from standard input\n");
}

/* ------------------ gettokens ------------------------ */

int gettokens(char *tokens, char **tokenptrs){
  int ntokenptrs;
  char *token;
  int i;

  ntokenptrs=0;
  token=strtok(tokens,",");
  while(token!=NULL){
    tokenptrs[ntokenptrs++]=token;
    token=strtok(NULL,",");
  }
  for(i=0;i<ntokenptrs;i++){
    TrimBack(tokenptrs[i]);
    tokenptrs[i]=TrimFront(tokenptrs[i]);
  }
  return ntokenptrs;
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  char *arg,*csv,*argin=NULL,*argout=NULL;
  char in_file[256],out_file[256];
  FILE *stream_in=NULL, *stream_out=NULL;
  int buffer_len, nrows, ncols;
  char *buffer,*labels,**labelptrs;
  char *datalabels,**datalabelptrs;
  int nlabelptrs,ndatalabelptrs;
  int *transfer,ntransfer,itransfer;
  float *zdev;
  float xyzoffset[3]={0.0,0.0,0.0};
  int i;
  char prefix[256],percen[2];
  int useprefix=0;
  char coffset[255];
  int is_sodar_file=1;
  char tokenbase[256], *tokenbaseptr=NULL;
  char *c_date_ptr=NULL, c_date[256];

  char *c_mintime_ptr=NULL, c_mintime[256];
  char *c_maxtime_ptr=NULL, c_maxtime[256];
  int have_mintime=0, have_maxtime=0;
  unsigned int i_mintime=0, i_maxtime=0;

  char *c_mindatetime_ptr=NULL, c_mindatetime[256];
  char *c_maxdatetime_ptr=NULL, c_maxdatetime[256];
  int have_mindatetime=0, have_maxdatetime=0;
  unsigned int i_mindatetime=0, i_maxdatetime=0;
  int lendate=0;

  SetStdOut(stdout);
  initMALLOC();

  ParseCommonOptions(argc, argv);
  if(show_help!=0){
    Usage(show_help);
    return 1;
  }
  if(show_version==1){
    PRINTVERSION("wind2fds");
    return 1;
  }

  strcpy(percen,"%");
  strcpy(prefix,"");

  if(argc==1){
    PRINTVERSION("wind2fds ");
   return 1;
  }

  for(i=1;i<argc;i++){
    arg=argv[i];
    if(strcmp(arg,"-wv")==0){
      is_sodar_file=0;
      continue;
    }
    if(strcmp(arg,"-offset")==0){
      i++;
      if(i>=argc)continue;
      arg=argv[i];
      if(strlen(arg)>1){
        sscanf(arg,"%f %f %f",xyzoffset,xyzoffset+1,xyzoffset+2);
      }
      continue;
    }
    else if(strcmp(arg,"-date")==0){
      c_date_ptr=c_date;
      i++;
      if(i>=argc)continue;
      arg=argv[i];
      strcpy(c_date,arg);
      lendate=strlen(c_date);
      continue;
    }
    else if(strcmp(arg,"-mintime")==0){
      i++;
      if(i>argc)continue;
      arg=argv[i];
      c_mintime_ptr=c_mintime;
      strcpy(c_mintime_ptr,arg);
      have_mintime=1;
      i_mintime=Date2Sec2(c_mintime_ptr);
      continue;
    }
    else if(strcmp(arg,"-maxtime")==0){
      i++;
      if(i>argc)continue;
      arg=argv[i];
      c_maxtime_ptr=c_maxtime;
      strcpy(c_maxtime_ptr,arg);
      have_maxtime=1;
      i_maxtime=Date2Sec2(c_maxtime_ptr);
      continue;
    }
    else if(strcmp(arg,"-mindate")==0){
      i++;
      if(i>argc)continue;
      arg=argv[i];
      c_mindatetime_ptr=c_mindatetime;
      strcpy(c_mindatetime_ptr,arg);
      if(strchr(c_mindatetime_ptr,':')!=NULL){
        have_mindatetime=1;
        i_mindatetime=Date2Sec(c_mindatetime_ptr);
      }
      else{
        i_mindatetime=Date2Day(c_mindatetime_ptr);
      }
      continue;
    }
    else if(strcmp(arg,"-maxdate")==0){
      i++;
      if(i>argc)continue;
      arg=argv[i];
      c_maxdatetime_ptr=c_maxdatetime;
      strcpy(c_maxdatetime_ptr,arg);
      if(strchr(c_maxdatetime_ptr,':')!=NULL){
        have_maxdatetime=1;
        i_maxdatetime=Date2Sec(c_maxdatetime_ptr);
      }
      else{
        i_maxdatetime=Date2Day(c_maxdatetime_ptr);
      }
      continue;
    }
    else if(strcmp(arg,"-prefix")==0){
      useprefix=1;
      i++;
      if(i>=argc)continue;
      arg=argv[i];
      strcpy(prefix,arg);
      strcat(prefix,"_");
      continue;
    }
    if(argin==NULL){
      argin=arg;
      continue;
    }
    if(argout==NULL){
      argout=arg;
      continue;
    }
  }

  if(argin==NULL){
    fprintf(stderr,"*** Error: An input file was not specified\n");
    if(stream_in!=NULL&&stream_in!=stdin)fclose(stream_in);
    if(stream_out!=NULL)fclose(stream_out);
    return 1;
  }
  if(strcmp(argin,"-")==0){
    stream_in=stdin;
  }
  else{
    strcpy(in_file,argin);
    stream_in=fopen(in_file,"r");
  }
  if(stream_in==NULL){
    fprintf(stderr,"*** Error: The file %s could not be opened for input\n",in_file);
    if(stream_out!=NULL)fclose(stream_out);
    return 1;
  }

  if(argout==NULL){
    if(strcmp(argin,"-")==0){
      strcpy(out_file,"stdin_exp.csv");
    }
    else{
      strcpy(out_file,argin);
      csv=strstr(out_file,".csv");
      if(csv!=NULL)*csv=0;
      strcat(out_file,"_exp.csv");
    }
  }
  else{
    strcpy(out_file, argout);
  }

  stream_out=fopen(out_file,"w");
  if(stream_out==NULL){
    fprintf(stderr,"*** Error: The file %s could not be opened for output\n",out_file);
    if(stream_in!=NULL&&stream_in!=stdin)fclose(stream_in);
    return 1;
  }

  initMALLOC();

  buffer_len=GetRowCols(stream_in, &nrows, &ncols);
  buffer_len+=10;

  NewMemory((void **)&buffer,buffer_len);
  NewMemory((void **)&labels,buffer_len);
  NewMemory((void **)&labelptrs,buffer_len*sizeof(char *));
  NewMemory((void **)&datalabels,buffer_len);
  NewMemory((void **)&datalabelptrs,buffer_len*sizeof(char *));
  NewMemory((void **)&transfer,buffer_len*sizeof(int));
  NewMemory((void **)&zdev,buffer_len*sizeof(float));

  if(fgets(labels,buffer_len,stream_in)==NULL){
    fprintf(stderr,"*** Error: The file %s is empty\n",in_file);
    if(stream_in!=NULL&&stream_in!=stdin)fclose(stream_in);
    if(stream_out!=NULL)fclose(stream_out);
    return 1;
  }
  if(is_sodar_file==1){
    while(strncmp(labels,"Sodar",5)==0){
      if(fgets(labels,buffer_len,stream_in)==NULL){
        fprintf(stderr,"*** Error: The file %s is empty\n",in_file);
        if(stream_in!=NULL&&stream_in!=stdin)fclose(stream_in);
        if(stream_out!=NULL)fclose(stream_out);
        return 1;
      }
    }
  }
  else{
    if(fgets(labels,buffer_len,stream_in)==NULL){
      fprintf(stderr,"*** Error: The file %s is empty\n",in_file);
      if(stream_in!=NULL&&stream_in!=stdin)fclose(stream_in);
      if(stream_out!=NULL)fclose(stream_out);
      return 1;
    }
  }

  nlabelptrs=gettokens(labels,labelptrs);
  ntransfer=0;
  for(i=0;i<nlabelptrs;i++){
    char *token;

    token=labelptrs[i];
    if(strcmp(token,"time")==0){
      transfer[i]=1;
      ntransfer++;
    }
    else if(strcmp(token,"TIMESTAMP")==0){
      transfer[i]=1;
      ntransfer++;
    }
    else if(strncmp(token,"ws",2)==0){
      transfer[i]=2;
      zdev[i]=(float)atof(token+2);
      ntransfer++;
    }
    else if(strncmp(token,"WS",2)==0){
      transfer[i]=2;
      zdev[i]=0.0;
      ntransfer++;
    }
    else if(strncmp(token,"wd",2)==0){
      zdev[i]=(float)atof(token+2);
      transfer[i]=3;
      ntransfer++;
    }
    else if(strcmp(token,"WindDir")==0){
      zdev[i]=0.0;
      transfer[i]=3;
      ntransfer++;
    }
    else if(strncmp(token,"sds",3)==0){
      zdev[i]=(float)atof(token+3);
      transfer[i]=4;
      ntransfer++;
    }
    else if(strncmp(token,"sdwd",4)==0){
      zdev[i]=(float)atof(token+4);
      transfer[i]=5;
      ntransfer++;
    }
    else{
      transfer[i]=0;
    }
  }
  fprintf(stream_out,"//HEADER\n");
  if(c_date_ptr!=NULL){
    fprintf(stream_out,"  //  date: %s\n",c_date_ptr);
  }
  if(c_mindatetime_ptr!=NULL){
    fprintf(stream_out,"  //  mindate: %s\n",c_mindatetime_ptr);
  }
  if(c_maxdatetime_ptr!=NULL){
    fprintf(stream_out,"  //  maxdate: %s\n",c_maxdatetime_ptr);
  }
  for(i=0;i<nlabelptrs;i++){
    char token2[256];

    strcpy(token2,"");
    if(useprefix==1)strcat(token2,prefix);
    strcat(token2,labelptrs[i]);
    sprintf(coffset,"%f %f %f",xyzoffset[0],xyzoffset[1],xyzoffset[2]+zdev[i]);
    TrimMZeros(coffset);
    if(transfer[i]==2){
      fprintf(stream_out,"DEVICE\n");
      fprintf(stream_out," %s %s VELOCITY %s sensor\n",token2,percen,percen);
      fprintf(stream_out," %s\n",coffset);
    }
    else if(transfer[i]==3){
      fprintf(stream_out,"DEVICE\n");
      fprintf(stream_out," %s %s ANGLE %s sensor\n",token2,percen,percen);
      fprintf(stream_out," %s\n",coffset);
    }
    else if(transfer[i]==4){
      fprintf(stream_out,"DEVICE\n");
      fprintf(stream_out," %s %s SD_VELOCITY %s sensor\n",token2,percen,percen);
      fprintf(stream_out," %s\n",coffset);
    }
    else if(transfer[i]==5){
      fprintf(stream_out,"DEVICE\n");
      fprintf(stream_out," %s %s SD_ANGLE %s sensor\n",token2,percen,percen);
      fprintf(stream_out," %s\n",coffset);
    }
  }
  fprintf(stream_out,"//DATA\n");
  itransfer=0;
  for(i=0;i<nlabelptrs;i++){
    if(transfer[i]==1){
      fprintf(stream_out,"s,s");
      itransfer++;
      if(itransfer!=ntransfer)fprintf(stream_out,",");
    }
    if(transfer[i]==2){
      fprintf(stream_out,"m/s");
      itransfer++;
      if(itransfer!=ntransfer)fprintf(stream_out,",");
    }
    if(transfer[i]==3){
      fprintf(stream_out,"deg");
      itransfer++;
      if(itransfer!=ntransfer)fprintf(stream_out,",");
    }
    if(transfer[i]==4){
      fprintf(stream_out,"m/s");
      itransfer++;
      if(itransfer!=ntransfer)fprintf(stream_out,",");
    }
    if(transfer[i]==5){
      fprintf(stream_out,"deg");
      itransfer++;
      if(itransfer!=ntransfer)fprintf(stream_out,",");
    }
  }
  fprintf(stream_out,"\n");
  itransfer=0;
  for(i=0;i<nlabelptrs;i++){
    char token2[256];

    strcpy(token2,"");
    if(useprefix==1)strcat(token2,prefix);
    strcat(token2,labelptrs[i]);
    if(transfer[i]!=0){
      if(transfer[i]==1){
        fprintf(stream_out,"%s,time_orig",labelptrs[i]);
      }
      else{
        fprintf(stream_out,"%s",token2);
      }
      itransfer++;
      if(itransfer!=ntransfer)fprintf(stream_out,",");
    }
  }
  if(is_sodar_file==0){
    if(fgets(labels,buffer_len,stream_in)==NULL){
      fprintf(stderr,"*** Error: The file %s is empty\n",in_file);
      fclose(stream_out);
      return 1;
    }
    if(fgets(labels,buffer_len,stream_in)==NULL){
      fprintf(stderr,"*** Error: The file %s is empty\n",in_file);
      fclose(stream_out);
      return 1;
    }
  }
  fprintf(stream_out,"\n");
  while(!feof(stream_in)){
    int skip_time;

    CheckMemory;
    if(fgets(datalabels,buffer_len,stream_in)==NULL)break;
    ndatalabelptrs=gettokens(datalabels,datalabelptrs);
    itransfer=0;
    skip_time=0;
    for(i=0;i<ndatalabelptrs;i++){
      char *token;

      if(transfer[i]==0)continue;
      token=datalabelptrs[i];
      if(transfer[i]==1){
        if(strchr(token,':')!=NULL){
          unsigned int time_local=0;

          if(c_date_ptr!=NULL&&strncmp(c_date_ptr,token,lendate)!=0){
            skip_time=1;
            break;
          }
          if(c_mintime_ptr!=NULL){
            if(have_mintime==1&&Date2Sec2(token)<i_mintime){
              skip_time=1;
              break;
            }
          }
          if(c_maxtime_ptr!=NULL){
            if(have_maxtime==1&&Date2Sec2(token)<i_maxtime){
              skip_time=1;
              break;
            }
          }
          if(c_mindatetime_ptr!=NULL){
            if((have_mindatetime==0&&Date2Day(token)<i_mindatetime)||(have_mindatetime==1&&Date2Sec(token)<i_mindatetime)){
              skip_time=1;
              break;
            }
          }
          if(c_maxdatetime_ptr!=NULL){
            if((have_maxdatetime==0&&Date2Day(token)>i_maxdatetime)||(have_maxdatetime==1&&Date2Sec(token)>i_maxdatetime)){
              skip_time=1;
              break;
            }
          }
          if(tokenbaseptr==NULL){
            tokenbaseptr=tokenbase;
            strcpy(tokenbase,token);
            time_local=0;
          }
          else{
            time_local = DiffDate(token,tokenbaseptr);
          }
          fprintf(stream_out,"%i,%s",time_local,token);
        }
        else{
          fprintf(stream_out,"%s",token);
        }
      }
      if(transfer[i]==2||transfer[i]==4){
        if(strcmp(token,"99.99")==0||strcmp(token,"NAN")==0){
          fprintf(stream_out,"NULL");
        }
        else{
          fprintf(stream_out,"%s",token);
        }
      }
      if(transfer[i]==3||transfer[i]==5){
        if(strcmp(token,"9999")==0||strcmp(token,"NAN")==0){
          fprintf(stream_out,"NULL");
        }
        else{
          fprintf(stream_out,"%s",token);
        }
      }
      itransfer++;
      if(itransfer!=ntransfer)fprintf(stream_out,",");
    }
    if(skip_time==1)continue;
    fprintf(stream_out,"\n");
  }

  if(stream_in!=NULL&&stream_in!=stdin)fclose(stream_in);
  if(stream_out!=NULL)fclose(stream_out);
  return 0;
}
