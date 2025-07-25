#define IN_MAIN
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <process.h>
#include <windows.h>
#endif
#ifdef pp_OSX
#include <unistd.h>
#endif
#include "string_util.h"
#include "background.h"
#include "datadefs.h"
#include "dmalloc.h"
#include "file_util.h"

#ifdef WIN32
void GetSystemTimesAddress(void);
int getnprocs(char *command);
#endif
unsigned char cpuusage(void);

#ifdef pp_LINUX
int get_ncores(void);
float get_load(void);
float get_host_load(char *host);
unsigned char cpuusage_host(char *host,int ncores);
#endif
int get_host_ncores(char *host);

#ifdef pp_OSX
unsigned char cpuusage_host(char *host,int ncores);
#endif

#ifndef WIN32

/* ------------------ Sleep ------------------------ */

void Sleep(int ticks){
  unsigned int time;

  time = ticks/1000.0 + 0.5;
  sleep(time);
}
#endif

/* ------------------ Usage ------------------------ */

void Usage(int option){
  char githash[100];
  char gitdate[100];
  char pp[] = "%";

  GetGitInfo(githash, gitdate);    // get githash

  printf("\n");
  printf("background [-d delay time (s) -h -u max_usage -v] prog [arguments]\n");
  printf("%s %s\n\n", githash, __DATE__);
  printf("Runs a program in the background when resources are available\n\n");
  printf("options:\n");
  printf("  -d dtime  - wait dtime seconds before running prog in the background\n");
  printf("  -m max    - wait to run prog until memory usage is less than max (25-100%s)\n", pp);
  printf("  -u max    - wait to run prog until cpu usage is less than max (25-100%s)\n", pp);
#ifdef WIN32
  printf("  -U max    - wait to run prog until number of instances of prog is less than max \n");
#endif
  UsageCommon(HELP_SUMMARY);
  printf("  prog      - program to run in the background\n");
  printf("  arguments - command line arguments of prog\n\n");
  if(option == HELP_ALL){
    printf("  -debug    - display debug messages\n");
#ifdef pp_LINUX
    printf("  -hosts hostfiles - file containing a list of host names to run jobs on\n");
#endif
#ifdef pp_LINUX
    printf("  -p path   - specify directory path to change to after ssh'ing to remote host\n");
#endif
    UsageCommon(HELP_ALL);
    printf("Example:\n");
    printf("  background -d 1.5 -u 50 prog arg1 arg2\n");
    printf("    runs prog (with arguments arg1 and arg2) after 1.5 seconds\n    and when the CPU usage drops below 50%s\n", pp);
  }
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){
  int i;
#ifdef WIN32
  int nprocs;
#else
  int debug=0;
  char command_buffer[1024];
  char user_path[1024];
#endif
  int argstart=-1;
  float delay_time=0.0;
  int cpu_usage, cpu_usage_max=25;
  int mem_usage, mem_usage_max=75;
  int nprocs_max=-1;
#ifdef pp_LINUX
  FILE *stream=NULL;
#endif

  int itime;
  char *arg;
#ifdef WIN32
  char *command, *base;
#endif

  SetStdOut(stdout);
  initMALLOC();
#ifdef pp_LINUX
  hostlistfile=NULL;
  host=NULL;
  strcpy(user_path,"");
  sprintf(pid,"%i",getpid());
#endif
#ifdef pp_OSX
  sprintf(pid,"%i",getpid());
#endif

  if(argc==1){
    PRINTVERSION("background ");
    return 1;
  }

  ParseCommonOptions(argc, argv);
  if(show_help!=0){
    Usage(show_help);
    return 1;
  }
  if(show_version==1){
    PRINTVERSION("background");
    return 1;
  }

  for(i=1;i<argc;i++){
    int lenarg;

    arg=argv[i];
    lenarg=strlen(arg);
    if(arg[0]=='-'){
      if(lenarg>1){
        switch(arg[1]){
          case 'd':
            if(strlen(arg)<=2||strcmp(arg,"-debug")!=0){
              i++;
              if(i<argc){
                arg=argv[i];
                sscanf(arg,"%f",&delay_time);
                if(delay_time<0.0)delay_time=0.0;
              }
            }
#ifndef WIN32
            else{
              debug=1;
            }
#endif
            break;
#ifdef pp_LINUX
          case 'h':
            if(strcmp(arg,"-hosts")==0){
              i++;
              if(i<argc){
                arg=argv[i];
                hostlistfile=malloc(strlen(arg)+1);
                strcpy(hostlistfile,arg);
              }
            }
            break;
          case 'p':
            i++;
            if(i<argc){
              arg=argv[i];
              if(strlen(arg)>0){
                strcpy(user_path,arg);
              }
            }
            break;
#endif
          case 'm':
            i++;
            if(i<argc){
              arg=argv[i];
              sscanf(arg,"%i",&mem_usage_max);
              if(mem_usage_max<25)mem_usage_max=25;
              if(mem_usage_max>90)mem_usage_max=90;
            }
            break;
          case 'u':
            i++;
            if(i<argc){
              arg=argv[i];
              sscanf(arg,"%i",&cpu_usage_max);
              if(cpu_usage_max<25)cpu_usage_max=25;
              if(cpu_usage_max>100)cpu_usage_max=100;
            }
            break;
          case 'U':
            i++;
            if(i<argc){
              arg=argv[i];
              sscanf(arg,"%i",&nprocs_max);
              nprocs_max = CLAMP(nprocs_max, 1, 100);
            }
            break;
          default:
            printf("Unknown option: %s\n",arg);
            Usage(HELP_ALL);
            return 1;
        }
      }
    }
    else{
      argstart=i;
      break;

    }
  }
#ifdef pp_LINUX
  nhostinfo=0;
  if(hostlistfile!=NULL){
    stream=fopen(hostlistfile,"r");
  }
  if(hostlistfile!=NULL&&stream!=NULL){
    char buffer[255];

    while(!feof(stream)){
      if(fgets(buffer,255,stream)==NULL)break;
      nhostinfo++;
    }
    if(nhostinfo>0){
      hostdata *hd;

      hostinfo=malloc(nhostinfo*sizeof(hostdata));
      hd = hostinfo;
      rewind(stream);
      while(!feof(stream)){
        char *hostname;

        if(fgets(buffer,255,stream)==NULL)break;
        TrimBack(buffer);
        hostname=malloc(strlen(buffer)+1);
        strcpy(hostname,buffer);
        hd->hostname=hostname;
        hd->ncores=get_host_ncores(hostname);
        printf("host: %s ncores=%i\n",hostname,hd->ncores);
        hd++;
      }

    }
  }
  if(stream!=NULL)fclose(stream);
#endif

  if(argstart<0)return 0;

  itime = delay_time*1000;
  if(itime>0){
    Sleep(itime);
  }

#ifdef WIN32
  GetSystemTimesAddress();
  command=argv[argstart];
  base = strrchr(command,'\\');
  if(base!=NULL){
    base++;
  }
  else{
    base=command;
  }

  if(nprocs_max>0){
    nprocs = getnprocs(base);
    while(nprocs>=nprocs_max){
      Sleep(1000);
      nprocs = getnprocs(base);
    }
  }
  else{
    cpu_usage=cpuusage();
    mem_usage=memusage();
    Sleep(200);
    cpu_usage=cpuusage();
    mem_usage=memusage();
    while(cpu_usage>cpu_usage_max || mem_usage>mem_usage_max){
      Sleep(1000);
      cpu_usage = cpuusage();
      mem_usage = memusage();
    }
  }
  _spawnvp(_P_NOWAIT,command, (const char **)(argv+argstart));
#else
  strcpy(command_buffer,"");
  if(hostinfo==NULL){
    cpu_usage=cpuusage();
    mem_usage=memusage();
    Sleep(200);
    cpu_usage=cpuusage();
    mem_usage=memusage();
    host=NULL;
    while(cpu_usage>cpu_usage_max||mem_usage>mem_usage_max){
      Sleep(1000);
      cpu_usage=cpuusage();
      mem_usage=memusage();
    }
  }
  else{
    int doit=0;

    while(doit==0){
      for(i=0;i<nhostinfo;i++){
        hostdata *hd;
        float fusage;

        hd = hostinfo + i;
        host = hd->hostname;
        cpu_usage=cpuusage_host(host,hd->ncores);
        fusage=(float)cpu_usage/255.0;
        if(debug==1)printf("host: %s cpu_usage=%f\n",host,fusage);
        if(cpu_usage<cpu_usage_max){
          if(debug==1)printf(" host %s is now free\n",host);
          doit=1;
          if(strlen(user_path)==0){
            getcwd(user_path,1024);
          }
          strcat(command_buffer,"ssh ");
          strcat(command_buffer,host);
          strcat(command_buffer," \"( cd ");
          strcat(command_buffer,user_path);
          strcat(command_buffer,";");
          break;
        }
      }
      if(doit==0)Sleep(1000);
    }
  }
  for(i=argstart;i<argc;i++){
    arg=argv[i];
    strcat(command_buffer,arg);
    if(i<argc-1){
      strcat(command_buffer," ");
    }
  }
  if(nhostinfo>0)strcat(command_buffer,")\"");
  strcat(command_buffer,"&");
  system(command_buffer);
#endif
  return 0;
}

#ifdef WIN32
typedef BOOL ( __stdcall * pfnGetSystemTimes)( LPFILETIME lpIdleTime, LPFILETIME lpKernelTime, LPFILETIME lpUserTime );
static pfnGetSystemTimes s_pfnGetSystemTimes = NULL;

static HMODULE s_hKernel = NULL;

/* ------------------ GetSystemTimesAddress ------------------------ */

void GetSystemTimesAddress(){
  if( s_hKernel == NULL ){
    s_hKernel = LoadLibrary((LPCSTR)"Kernel32.dll" );
    if( s_hKernel != NULL ){
      s_pfnGetSystemTimes = (pfnGetSystemTimes)GetProcAddress( s_hKernel, "GetSystemTimes" );
      if( s_pfnGetSystemTimes == NULL ){
        FreeLibrary( s_hKernel ); s_hKernel = NULL;
      }
    }
  }
}

/* ------------------ getnprocs ------------------------ */

#ifdef WIN32
int getnprocs(char *command){
  FILE *stream;
  int count=0;
  char com_copy[256], *ext;

  system("tasklist > process.out");

  stream = fopen("process.out","r");
  if(stream==NULL)return 0;

  strcpy(com_copy, command);
  ext = strchr(com_copy, ' ');
  if(ext!=NULL)*ext = 0;

  ext = strchr(com_copy, '.');
  if(ext!=NULL)*ext = 0;

  while(!feof(stream)){
    char buffer[255];

    if(fgets(buffer, 255, stream)==NULL)break;
    ext = strchr(buffer, ' ');
    if(ext!=NULL)*ext = 0;

    ext = strchr(buffer,'.');
    if(ext!=NULL)*ext=0;
    if(STRCMP(buffer,com_copy)==0)count++;
  }
  fclose(stream);
  return count;
}
#endif

/* ------------------ cpuusage ------------------------ */

unsigned char cpuusage(){
  FILETIME               ft_sys_idle;
  FILETIME               ft_sys_kernel;
  FILETIME               ft_sys_user;

  ULARGE_INTEGER         ul_sys_idle;
  ULARGE_INTEGER         ul_sys_kernel;
  ULARGE_INTEGER         ul_sys_user;

  static ULARGE_INTEGER	 ul_sys_idle_old;
  static ULARGE_INTEGER  ul_sys_kernel_old;
  static ULARGE_INTEGER  ul_sys_user_old;

  unsigned char usage_local = 0;

// we cannot directly use GetSystemTimes on C language
/* add this line :: pfnGetSystemTimes */
  s_pfnGetSystemTimes(&ft_sys_idle,    /* System idle time */
  &ft_sys_kernel,  /* system kernel time */
  &ft_sys_user);   /* System user time */

  CopyMemory(&ul_sys_idle  , &ft_sys_idle  , sizeof(FILETIME)); // Could been optimized away...
  CopyMemory(&ul_sys_kernel, &ft_sys_kernel, sizeof(FILETIME)); // Could been optimized away...
  CopyMemory(&ul_sys_user  , &ft_sys_user  , sizeof(FILETIME)); // Could been optimized away...

  usage_local  =
    (
    (
    (
    (
    (ul_sys_kernel.QuadPart - ul_sys_kernel_old.QuadPart)+
    (ul_sys_user.QuadPart   - ul_sys_user_old.QuadPart)
    )
    -
    (ul_sys_idle.QuadPart-ul_sys_idle_old.QuadPart)
    )
    *
    (100)
    )
    /
    (
    (ul_sys_kernel.QuadPart - ul_sys_kernel_old.QuadPart)+
    (ul_sys_user.QuadPart   - ul_sys_user_old.QuadPart)
    )
    );

  ul_sys_idle_old.QuadPart   = ul_sys_idle.QuadPart;
  ul_sys_user_old.QuadPart   = ul_sys_user.QuadPart;
  ul_sys_kernel_old.QuadPart = ul_sys_kernel.QuadPart;

  return usage_local;
}
#endif
#ifdef pp_OSX

/* ------------------ get_sysctl ------------------------ */

void get_sysctl(char *host, char *var, int *ivar, float *fvar){
  char command[256];
  FILE *stream;
  char sysctl_file[256];


  if(ivar!=NULL)*ivar=1;
  if(fvar!=NULL)*fvar=0.0;

  strcpy(sysctl_file,"/tmp/");
  strcat(sysctl_file,var);
  strcat(sysctl_file,"_");
  strcat(sysctl_file,pid);

  strcpy(command,"");
  if(host!=NULL){
    strcat(command,"ssh ");
    strcat(command,host);
    strcat(command," ");
  }
  strcat(command,"sysctl -n ");
  strcat(command,var);
  strcat(command," >");
  strcat(command,sysctl_file);
  system(command);

  stream=fopen(sysctl_file,"r");
  if(stream!=NULL){
    char buffer[255];

    fgets(buffer,255,stream);
    if(ivar!=NULL)sscanf(buffer,"%i",ivar);
    if(fvar!=NULL){
      if(buffer[0]=='{'){
        sscanf(buffer+1,"%f",fvar);
      }
      else{
        sscanf(buffer,"%f",fvar);
      }
    }
    fclose(stream);
  }
  UNLINK(sysctl_file);
}

/* ------------------ get_ncores ------------------------ */

int get_ncores(void){
  int ncores=1;

  get_sysctl(NULL,"hw.ncpu",&ncores,NULL);
  return ncores;
}

/* ------------------ get_host_ncores ------------------------ */

int get_host_ncores(char *host){
  int ncores=1;

  get_sysctl(host,"hw.ncpu",&ncores,NULL);
  return ncores;
}

/* ------------------ get_load ------------------------ */

float get_load(void){
  float load;

  get_sysctl(NULL,"vm.loadavg",NULL,&load);
  return load;
}

/* ------------------ get_host_load ------------------------ */

float get_host_load(char *host){
  float load;

  get_sysctl(host,"vm.loadavg",NULL,&load);
  return load;
}
#endif
#ifdef pp_LINUX

/* ------------------ get_ncores ------------------------ */

int get_ncores(void){
  FILE *stream;
  int ncores=0;
  char buffer[255];

  stream=fopen("/proc/cpuinfo","r");
  if(stream==NULL)return 1;
  while(!feof(stream)){
    if(fgets(buffer,255,stream)==NULL)break;
    if(strlen(buffer)<9)continue;
    buffer[9]=0;
    if(strcmp(buffer,"processor")==0)ncores++;
  }
  if(ncores==0)ncores=1;
  fclose(stream);
  return ncores;
}

/* ------------------ get_host_ncores ------------------------ */

int get_host_ncores(char *hosta){
  FILE *stream;
  char buffer[1024];
  char command[1024];
  char localfile[1024];
  int ncores=0;

  strcpy(localfile,"/tmp/cpuinfo.");
  strcat(localfile,hosta);
  strcat(localfile,".");
  strcat(localfile,pid);

  strcpy(command,"ssh ");
  strcat(command,hosta);
  strcat(command," cat /proc/cpuinfo >");
  strcat(command,localfile);

  system(command);

  stream=fopen(localfile,"r");
  if(stream==NULL){
    printf("unable to open %s\n",localfile);
    return 1;
  }
  while(!feof(stream)){
    if(fgets(buffer,255,stream)==NULL)break;
    if(strlen(buffer)<9)continue;
    buffer[9]=0;
    if(strcmp(buffer,"processor")==0)ncores++;
  }
  if(ncores==0){
    printf("0 cores found in %s\n",localfile);
    ncores=1;
  }
  fclose(stream);
  UNLINK(localfile);
  return ncores;
}

/* ------------------ get_host_load ------------------------ */

float get_host_load(char *host_arg){
  FILE *stream;
  char buffer[1024];
  char command[1024];
  char localfile[1024];
  float load1;

  strcpy(localfile,"/tmp/loadavg.");
  strcat(localfile,host_arg);
  strcat(localfile,".");
  strcat(localfile,pid);

  strcpy(command,"ssh ");
  strcat(command,host_arg);
  strcat(command," cat /proc/loadavg >");
  strcat(command,localfile);

  system(command);

  stream=fopen(localfile,"r");
  if(stream==NULL)return 1.0;
  if(fgets(buffer,255,stream)==NULL){
    fclose(stream);
    return 1.0;
  }
  sscanf(buffer,"%f",&load1);
  fclose(stream);
  UNLINK(localfile);
  return load1;
}

/* ------------------ get_load ------------------------ */

float get_load(void){
  FILE *stream;
  char buffer[255];
  float load1;

  stream=fopen("/proc/loadavg","r");
  if(stream==NULL)return 1.0;
  if(fgets(buffer,255,stream)==NULL){
    fclose(stream);
    return 1.0;
  }
  sscanf(buffer,"%f",&load1);
  fclose(stream);
  return load1;
}
#endif

#ifndef WIN32

/* ------------------ cpuusage_host ------------------------ */

unsigned char cpuusage_host(char *hostb, int ncores){
  float load;
  unsigned char usage;

  load = get_host_load(hostb);
  if(load>ncores)load=ncores;
  usage = 100*(load/(float)ncores);
  return usage;
}

/* ------------------ cpuusage ------------------------ */

unsigned char cpuusage(){
  unsigned char usage;
  float load;
  int ncores;

  ncores = get_ncores();
  load = get_load();
  if(load>ncores)load=ncores;
  usage = 100*(load/(float)ncores);
  return usage;
}
#endif
