#define INMAIN
#include "options.h"
#include "zlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "svzip.h"
#include "string_util.h"
#include "dmalloc.h"

//dummy change to bump version number to 1.4.8
//dummy change to force githash update

/* ------------------ Usage ------------------------ */

void Usage(int option){
  char githash[256];
  char gitdate[256];

  GetGitInfo(githash, gitdate);    // get githash

  PRINTF("\n");
  PRINTF("smokezip [options] casename\n");
  PRINTF("%s - %s\n\n", githash, __DATE__);
  PRINTF("Compress FDS data files\n\n");
  PRINTF("casename - Smokeview .smv file for case to be compressed\n\n");
  PRINTF("options:\n");
  PRINTF("  -c  - cleans or removes all compressed files\n");
#ifdef pp_THREAD
  PRINTF("  -t nthread - Compress nthread files at a time (up to %i)\n", NTHREADS_MAX);
#endif

  UsageCommon(HELP_SUMMARY);
  if(option == HELP_ALL){
    PRINTF("overwrite options:\n");
    PRINTF("  -f  - overwrites all compressed files\n");
    PRINTF("  -2  - overwrites 2d slice compressed files\n");
    PRINTF("  -3  - overwrites 3d smoke files\n");
    PRINTF("  -b  - overwrites boundary compressed files\n");
    PRINTF("  -part2iso - generate isosurfaces from particle data\n");
    PRINTF("  -no_chop - do not chop or truncate slice data.  Smokezip compresses\n");
    PRINTF("        slice data ignoring chop values specified in the ini file\n");
    PRINTF("compress options:\n");
    PRINTF("  -n3 - do not compress 3d smoke files\n");
    PRINTF("  -nb - do not compress boundary files\n");
    PRINTF("  -ns - do not compress slice files\n");
    PRINTF("output options:\n");
    PRINTF("  -d destdir - copies compressed files (and files needed by Smokeview\n");
    PRINTF("        to view the case) to the directory destdir\n");
    PRINTF("  -s sourcedir - specifies directory containing source files\n");
    PRINTF("  -demo - Creates the files (compressed and .svd ) needed by the\n");
    PRINTF("        Smokeview demonstrator mode.  Compresses files that are autoloaded, \n");
    PRINTF("        uses (20.0,620.0) and (0.0,0.23) for temperature and oxygen bounds\n");
    PRINTF("        and creates the .svd file which activates the Smokeview demonstrator\n");
    PRINTF("        mode.\n");
    PRINTF("  -skip skipval - skip frames when compressing files\n\n");
    UsageCommon(HELP_ALL);
  }
}

/* ------------------ main ------------------------ */

int main(int argc, char **argv){

//  Bytef *source,*sourcecheck,*dest;
//  int sourceLen, destLen;
//  int returncode=0;

  char *arg;

  char *filebase;
  int filelen;
  char smvfile[1024];
  char smzlogfile[1024];
  char smvfilebase[1024];
  char *ext;
  char inifile[1024];
  char inifilebase[1024];
  int i;
  int redirect=0;

  SetStdOut(stdout);
  initMALLOC();

  ParseCommonOptions(argc, argv);
  if(show_help!=0){
    Usage(show_help);
    return 0;
  }
  if(show_version==1){
    PRINTVERSION("smokezip");
    return 0;
  }

  GLOBdoit_smoke3d=1;
  GLOBdoit_boundary=1;
  GLOBdoit_slice=1;
  GLOBdoit_volslice=1;

  strcpy(GLOBpp,"%");
  strcpy(GLOBx,"X");
  GLOBfirst_initsphere=1;
  GLOBfirst_slice=1;
  GLOBfirst_patch=1;
  GLOBfirst_part2iso=1;
  GLOBfirst_part2iso_smvopen=1;
#ifdef pp_THREAD
  mt_nthreads=2;
#endif
  GLOBframeskip=-1;
  GLOBno_chop=0;
  GLOBmake_demo=0;
  GLOBsyst=0;
  GLOBendianfile=NULL;
  GLOBdestdir=NULL;
  GLOBsourcedir=NULL;
  GLOBoverwrite_b=0;
  GLOBoverwrite_s=0;
  GLOBpartfile2iso=0;
  GLOBoverwrite_slice=0;
  GLOBoverwrite_volslice=0;
  GLOBcleanfiles=0;
  GLOBsmoke3dzipstep=1;
  GLOBboundzipstep=1;
  GLOBslicezipstep=1;
  GLOBfilesremoved=0;

  npatchinfo=0;
  nsmoke3dinfo=0;
  npartinfo=0;
  npartclassinfo=0;
  partinfo=NULL;
  partclassinfo=NULL;
  maxpart5propinfo=0;
  npart5propinfo=0;
  nsliceinfo=0;
  sliceinfo=NULL;
  nmeshes=0;

  patchinfo=NULL;
  smoke3dinfo=NULL;

  filebase=NULL;
  if(argc==1){
    PRINTVERSION("Smokezip ");
    return 0;
  }

  for(i=1;i<argc;i++){
    int lenarg;
    int lenarg2;
    char *arg2;

    arg=argv[i];
    lenarg=strlen(arg);
    if(arg[0]=='-'&&lenarg>1){
      switch(arg[1]){
      case 'n':
        if(strcmp(arg,"-n3")==0){
          GLOBdoit_smoke3d=0;
        }
        else if(strcmp(arg,"-nb")==0){
          GLOBdoit_boundary=0;
        }
        else if(strcmp(arg,"-ns")==0){
          GLOBdoit_slice=0;
        }
        else if(strcmp(arg,"-nvs")==0){
          GLOBdoit_volslice=0;
        }
        else if(strcmp(arg,"-no_chop")==0){
          GLOBno_chop=1;
        }
        break;
      case '2':
        GLOBoverwrite_slice=1;
        break;
      case '3':
        GLOBoverwrite_volslice=1;
        GLOBoverwrite_s=1;
        break;
      case 'p':
        if(strcmp(arg,"-part2iso")==0){
          GLOBpartfile2iso=1;
        }
        break;
      case 'f':
        GLOBoverwrite_b=1;
        GLOBoverwrite_s=1;
        GLOBoverwrite_slice=1;
        GLOBoverwrite_volslice=1;
        break;
      case 'c':
        GLOBcleanfiles=1;
        break;
      case 'r':
        redirect=1;
        break;
      case 's':
        if(i+1>=argc)break;
        if(lenarg==2){
            lenarg2=strlen(argv[i+1]);
            NewMemory((void **)&GLOBsourcedir,lenarg2+2);
            strcpy(GLOBsourcedir,argv[i+1]);
            if(GLOBsourcedir[lenarg2-1]!=dirseparator[0]){
              strcat(GLOBsourcedir,dirseparator);
            }
            if(GetFileInfo(GLOBsourcedir,NULL,NULL)!=0){
              fprintf(stderr,"*** Warning: The source directory specified, %s, does not exist or cannot be accessed\n",GLOBsourcedir);
              return 1;
            }
           i++;
        }
        else if(strcmp(arg,"-skip")==0){
          GLOBframeskip=-1;
          arg2=argv[i+1];
          sscanf(arg2,"%i",&GLOBframeskip);
          if(GLOBframeskip>0){
            GLOBslicezipstep=GLOBframeskip;
            GLOBsmoke3dzipstep=GLOBframeskip;
            GLOBboundzipstep=GLOBframeskip;
          }
          i++;
        }
        break;
      case 'd':
        if(strcmp(arg,"-demo")==0){
          GLOBmake_demo=1;
          break;
        }
        if(i+1<argc){
          lenarg2=strlen(argv[i+1]);
          NewMemory((void **)&GLOBdestdir,lenarg2+2);
          strcpy(GLOBdestdir,argv[i+1]);
          if(GLOBdestdir[lenarg2-1]!=dirseparator[0]){
            strcat(GLOBdestdir,dirseparator);
          }
 //         if(GetFileInfo(GLOBdestdir,NULL,NULL)!=0){
 //           fprintf(stderr,"*** Warning: The destination directory %s does not exist or cannot be accessed\n",GLOBdestdir);
 //           return 1;
 //         }
          i++;
        }
        break;
#ifdef pp_THREAD
      case 't':
        mt_compress=1;
        if(i+1<argc){
          arg2=argv[i+1];
          sscanf(arg2,"%i",&mt_nthreads);
          if(mt_nthreads<1)mt_nthreads=1;
          if(mt_nthreads>NTHREADS_MAX)mt_nthreads=NTHREADS_MAX;
          i++;
        }
        break;
#endif
      default:
        Usage(HELP_ALL);
        return 1;
      }
    }
    else{
      if(filebase==NULL){
        filebase=argv[i];
      }
    }
  }

#ifdef pp_THREAD
  if(GLOBcleanfiles==1)mt_nthreads=1;
#endif

  // construct smv filename

  if(filebase==NULL){
    Usage(HELP_ALL);
    return 1;
  }
#ifdef pp_THREAD
  InitPthreadMutexes();
#endif
  filelen=strlen(filebase);
  if(filelen>4){
    ext=filebase+filelen-4;
    if(strcmp(ext,".smv")==0){
      ext[0]=0;
      filelen=strlen(filebase);
    }
  }
  NewMemory((void **)&smvzip_filename, (unsigned int)(strlen(filebase) + strlen(".smvzip") + 1));
  STRCPY(smvzip_filename, filebase);
  STRCAT(smvzip_filename, ".smvzip");
  if(GLOBsourcedir==NULL){
    strcpy(smvfile,filebase);
    strcpy(smzlogfile,filebase);
  }
  else{
    strcpy(smvfile,GLOBsourcedir);
    strcat(smvfile,filebase);
    strcpy(smzlogfile,GLOBsourcedir);
    strcat(smzlogfile,filebase);
  }
  strcpy(smvfilebase,filebase);
  if(GLOBpartfile2iso==1||GLOBcleanfiles==1){
    strcpy(GLOBsmvisofile,smvfile);
    strcat(GLOBsmvisofile,".isosmv");
  }

  strcat(smvfile,".smv");
  if(redirect==1){
    if(GLOBsourcedir==NULL){
      strcpy(smzlogfile,filebase);
    }
    else{
      strcpy(smzlogfile,GLOBsourcedir);
      strcat(smzlogfile,filebase);
    }
    strcat(smzlogfile,".smzlog");
    SMZLOG_STREAM=fopen(smzlogfile,"w");
    if(SMZLOG_STREAM!=NULL){
      SetStdOut(SMZLOG_STREAM);
    }
  }

  // construct ini file name

  strcpy(inifile,smvfile);
  inifile[strlen(inifile)-4]=0;
  strcat(inifile,".ini");
  strcpy(inifilebase,filebase);
  strcat(inifilebase,".ini");

  // make sure smv file name exists

  if(GetFileInfo(smvfile,NULL,NULL)!=0){
    fprintf(stderr,"*** Error: The file %s does not exist\n",smvfile);
    return 1;
  }

  // make sure smv file can be opened

  if(ReadSMV(smvfile)!=0)return 1;

  ReadINI(inifile);

#ifdef pp_THREAD
  CompressAllMT();
#else
  CompressAll(NULL);
#endif

  if(GLOBcleanfiles==1){
    if(FileExistsOrig(smvzip_filename)==1){
      UNLINK(smvzip_filename);
    }
  }
  else{
    FILE *stream;

    stream = fopen(smvzip_filename, "w");
    if(stream!=NULL){
      fprintf(stream, "1\n");
      fclose(stream);
    }
  }
  if(GLOBcleanfiles==0&&GLOBdestdir!=NULL){
    PRINTF("Copying .smv, .ini and .end files to %s directory\n",GLOBdestdir);
    CopyFILE(GLOBdestdir,smvfile,smvfilebase,REPLACE_FILE);
    CopyFILE(GLOBdestdir,inifile,inifilebase,REPLACE_FILE);
  }
  if(GLOBcleanfiles==1&&GLOBfilesremoved==0){
    PRINTF("No compressed files were removed\n");
  }
  if(GLOBmake_demo==1){
    MakeSVD(GLOBdestdir,smvfile);
  }
  return 0;
}

/* ------------------ CompressAll ------------------------ */

void *CompressAll(void *arg){
  int *thread_index;

  thread_index=(int *)(arg);
  if(GLOBdoit_boundary==1)CompressPatches(thread_index);
  if(GLOBdoit_slice==1)CompressSlices(thread_index);
  if(GLOBdoit_volslice==1)CompressVolSlices(thread_index);
  if(GLOBdoit_smoke3d==1)Compress3DSmokes(thread_index);
  ConvertParts2Iso(thread_index);
  return NULL;
}

/* ------------------ MakeSVD ------------------------ */

void MakeSVD(char *in_dir, char *smvfile){
  char *file_out=NULL,*svd;

  if(smvfile==NULL)return;
  svd=strrchr(smvfile,'.');
  if(svd==NULL)return;

  NewMemory((void **)&file_out,(svd-smvfile)+4+1);
  strcat(file_out,smvfile);
  strcpy(svd,".svd");

  if(in_dir==NULL){
    CopyFILE(".",smvfile,file_out,REPLACE_FILE);
  }
  else{
    CopyFILE(in_dir,smvfile,file_out,REPLACE_FILE);
  }

}

/* ------------------ PrintSummary ------------------------ */

void PrintSummary(void){
  int i;
  int nsum;
  int nsum2;

  PRINTF("\n");
  PRINTF("********* Summary **************\n");
  PRINTF("\n");
  nsum=0;
  nsum2=0;
  for(i=0;i<nsliceinfo;i++){
    slicedata *slicei;

    slicei = sliceinfo + i;
    if(slicei->compressed==1)nsum++;
    if(slicei->vol_compressed==1)nsum2++;
  }
  if(nsum>0||nsum2>0)PRINTF("*** slice files ***\n");
  if(nsum>0){
    for(i=0;i<nsliceinfo;i++){
      slicedata *slicei;
      flowlabels *label;

      slicei = sliceinfo + i;
      if(slicei->compressed==0)continue;
      label=&slicei->label;
      PRINTF("%s  (%s): %s\n",slicei->file,label->longlabel,slicei->summary);
      PRINTF("  min=%f %s, max=%f %s \n",slicei->valmin,label->unit,slicei->valmax,label->unit);
    }
  }

  nsum=0;
  for(i=0;i<nsliceinfo;i++){
    slicedata *slicei;

    slicei = sliceinfo + i;
    if(slicei->vol_compressed==1)nsum++;
  }
  if(nsum>0){
    for(i=0;i<nsliceinfo;i++){
      slicedata *slicei;
      flowlabels *label;

      slicei = sliceinfo + i;
      if(slicei->vol_compressed==0)continue;
      label=&slicei->label;
      PRINTF("%s (%s)\n  %s\n",slicei->file,label->longlabel,slicei->volsummary);
    }
  }
  if(nsum>0||nsum2>0)PRINTF("\n");

  nsum=0;
  for(i=0;i<nsmoke3dinfo;i++){
    smoke3d *smoke3di;

    smoke3di = smoke3dinfo + i;
    if(smoke3di->compressed==1)nsum++;
  }
  if(nsum>0){
    PRINTF("\n*** 3D smoke/fire files ***\n");
    for(i=0;i<nsmoke3dinfo;i++){
      smoke3d *smoke3di;

      smoke3di = smoke3dinfo + i;
      if(smoke3di->compressed==0)continue;
      PRINTF("%s:  %s\n",smoke3di->file,smoke3di->summary);
    }
    PRINTF("\n");
  }

  nsum=0;
  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(patchi->compressed==1)nsum++;
  }
  if(nsum>0){
    PRINTF("*** boundary files ***\n");
    for(i=0;i<npatchinfo;i++){
      patchdata *patchi;
      flowlabels *label;
      char type[32];

      patchi = patchinfo + i;
      if(patchi->compressed==0)continue;
      label=&patchi->label;
      if(patchi->is_geom){
        strcpy(type, "GEOM");
      }
      else{
        strcpy(type, "BNDF");
      }
      PRINTF("%s (%s/%s):  %s\n",patchi->file,label->longlabel,type,patchi->summary);
      PRINTF("  min=%f %s, max=%f %s \n",patchi->valmin,label->unit,patchi->valmax,label->unit);
    }
    PRINTF("\n");
  }

  nsum=0;
  for(i=0;i<npartinfo;i++){
    part *parti;

    parti = partinfo + i;
    if(parti->compressed2==1)nsum++;
  }
  if(nsum>0){
    PRINTF("*** particle files ***\n");
    for(i=0;i<npartinfo;i++){
      int j;
      part *parti;

      parti = partinfo + i;
      if(parti->compressed2==0)continue;

      PRINTF("%s converted to:\n",parti->file);
      for(j=0;j<parti->nsummaries;j++){
        PRINTF("  %s\n",parti->summaries[j]);
      }
      PRINTF("\n");
    }
    PRINTF("\n");
  }

}
