#include "options.h"
#include "zlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "svzip.h"
#include "dmalloc.h"
#include "stdio_buffer.h"
#include "getdata.h"

/* ------------------ InitVolRender ------------------------ */

void InitVolRender(void){
  int i;

  nvolrenderinfo = 0;
  for(i = 0;i < nmeshes;i++){
    meshdata *meshi;
    volrenderdata *vr;

    meshi = meshinfo + i;
    vr = &meshi->volrenderinfo;
    vr->rendermesh = meshi;
    vr->fire = NULL;
    vr->smoke = NULL;
  }
  for(i = 0;i < nsliceinfo;i++){
    slicedata *slicei;
    int blocknumber;
    meshdata *meshi;
    volrenderdata *vr;
    int ni, nj, nk;

    slicei = sliceinfo + i;
    slicei->isvolslice = 0;
    slicei->voltype = 0;
    blocknumber = slicei->blocknumber;
    if(blocknumber < 0 || blocknumber >= nmeshes)continue;
    meshi = meshinfo + blocknumber;
    GetSliceParmsC(slicei->file, &ni, &nj, &nk);

    if(ni != meshi->ibar + 1 || nj != meshi->jbar + 1 || nk != meshi->kbar + 1)continue;
    vr = &meshi->volrenderinfo;

    if(STRCMP(slicei->label.shortlabel, "temp") == 0){
      vr->fire = slicei;
      continue;
    }
    if(IsSootFile(slicei->label.shortlabel, slicei->label.longlabel) == 1){
      vr->smoke = slicei;
      continue;
    }
  }
  nvolrenderinfo = 0;
  for(i = 0;i < nmeshes;i++){
    meshdata *meshi;
    volrenderdata *vr;

    meshi = meshinfo + i;
    vr = &meshi->volrenderinfo;
    if(vr->smoke != NULL){
      nvolrenderinfo++;
      vr->smoke->isvolslice = 1;
      vr->smoke->voltype = 1;
      if(vr->fire != NULL){
        vr->fire->isvolslice = 1;
        vr->fire->voltype = 2;
      }
    }
  }
}

/* ------------------ ReadSMV ------------------------ */

int ReadSMV(char *smvfile){
  FILE *stream = NULL;
  bufferstreamdata *streamsmv;
  int ioffset;
  int unit_start=15;
  int igrid,ipdim;
  int ipatch, ismoke3d, islice;
#define BUFFERSIZE 255
  char buffer[BUFFERSIZE];

  ipdim=0;
  stream=FOPEN(smvfile,"r");
  if(stream==NULL){
    PRINTF("The file: %s could not be opened\n",smvfile);
    return 1;
  }
  fclose(stream);

  streamsmv = GetSMVBuffer(smvfile);

  while(!FEOF(streamsmv)){
    if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
    CheckMemory;
    if(strncmp(buffer," ",1)==0)continue;

  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ SMOKE3D ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(Match(buffer,"SMOKE3D") == 1||Match(buffer, "SMOKF3D") == 1||Match(buffer, "SMOKG3D") == 1){
      nsmoke3dinfo++;
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ BNDF ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(
      Match(buffer, "BNDF") == 1 || Match(buffer, "BNDC") == 1 || Match(buffer, "BNDE") == 1){
      npatchinfo++;
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ CLASS_OF_PARTICLES++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(Match(buffer,"CLASS_OF_PARTICLES") == 1){
      int i,nclasses;

      npartclassinfo++;
      FGETS(buffer,BUFFERSIZE,streamsmv);
      FGETS(buffer,BUFFERSIZE,streamsmv);
      FGETS(buffer,BUFFERSIZE,streamsmv);
      sscanf(buffer,"%i",&nclasses);
      if(nclasses>0)maxpart5propinfo+=nclasses;
      for(i=0;i<nclasses;i++){
        FGETS(buffer,BUFFERSIZE,streamsmv);
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ PART ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(Match(buffer,"PRT5") == 1){
      npartinfo++;
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ SLCF ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(
      Match(buffer,"SLCF") == 1||
      Match(buffer,"SLCC") == 1||
      Match(buffer, "SLCD") == 1 ||
      Match(buffer,"SLCT") == 1
      ){
      nsliceinfo++;
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ GRID ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(Match(buffer,"GRID") == 1){
      nmeshes++;
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ PDIM ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(Match(buffer,"PDIM") == 1){
      ipdim++;
      continue;
    }
  }

  // allocate memory for smoke3d file info

  if(nsmoke3dinfo>0){
    smoke3d *smoke3di;
    int i;

    NewMemory((void **)&smoke3dinfo,nsmoke3dinfo*sizeof(smoke3d));
    for(i=0;i<nsmoke3dinfo;i++){
      smoke3di = smoke3dinfo + i;
      smoke3di->file=NULL;
      smoke3di->filebase=NULL;
    }
  }

  // allocate memory for boundary file info

  if(npatchinfo>0){
    patchdata *patchi;
    int i;

    NewMemory((void **)&patchinfo,  npatchinfo*sizeof(patchdata));
    NewMemory((void **)&patchbounds,npatchinfo*sizeof(bounddata));
    for(i=0;i<npatchinfo;i++){
      patchi = patchinfo + i;
      patchi->file=NULL;
      patchi->filebase=NULL;
      patchi->setvalmin=0;
      patchi->setvalmax=0;
      patchi->valmin=0.0;
      patchi->valmax=1.0;
    }
  }

  if(nmeshes>0&&nmeshes==ipdim){
    NewMemory((void **)&meshinfo,nmeshes*sizeof(meshdata));
  }
  else{
  }
  // allocate memory for slice file info

  if(nsliceinfo>0){
    slicedata *slicei;
    int i;

    NewMemory((void **)&slicebounds, nsliceinfo*sizeof(bounddata));
    NewMemory((void **)&sliceinfo,   nsliceinfo*sizeof(slicedata));
    for(i=0;i<nsliceinfo;i++){
      slicei = sliceinfo + i;
      slicei->file=NULL;
      slicei->filebase=NULL;

      slicei->setvalmin=0;
      slicei->setvalmax=0;
      slicei->valmax=1.0;
      slicei->valmin=0.0;

      slicei->setchopvalmin=0;
      slicei->setchopvalmax=0;
      slicei->chopvalmax=1.0;
      slicei->chopvalmin=0.0;

      slicei->doit=1;
    }
  }

  // allocate memory for particle file info

  if(npartinfo>0){
    part *parti;
    int i;

    NewMemory((void **)&partinfo,npartinfo*sizeof(part));
    for(i=0;i<npartinfo;i++){
      parti = partinfo + i;
      parti->file=NULL;
      parti->filebase=NULL;
      parti->setvalmin=0;
      parti->setvalmax=0;
      parti->valmax=1.0;
      parti->valmin=0.0;
    }
  }
  if(npartclassinfo>0){
    NewMemory((void **)&partclassinfo,npartclassinfo*sizeof(partclassdata));
  }
  if(maxpart5propinfo>0){
    NewMemory((void **)&part5propinfo,maxpart5propinfo*sizeof(partpropdata));
  }

  // read in smv file a second time_local to compress files

  ioffset=0;
  ipatch=0;
  npartclassinfo=0;
  npart5propinfo=0;
  npartinfo=0;
  islice=0;
  ipdim=0;
  igrid=0;
  ismoke3d=0;
  REWIND(streamsmv);
#ifndef pp_THREAD
  if(GLOBcleanfiles==0)PRINTF("Compressing .bf, .iso, .s3d, and .sf data files referenced in %s\n\n",smvfile);
#endif
  if(GLOBcleanfiles==1){
    PRINTF("Removing compressed .bf, .iso, .s3d and .sf data files referenced in %s\n",smvfile);
    PRINTF("   (Each removal occurs only if the corresponding uncompressed file exists)\n\n");
  }
  while(!FEOF(streamsmv)){
    if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
    CheckMemory;
    if(strncmp(buffer," ",1)==0)continue;

    if(Match(buffer,"GRID") == 1){
      meshdata *meshi;

      meshi=meshinfo+igrid;
      igrid++;
      FGETS(buffer,BUFFERSIZE,streamsmv);
      sscanf(buffer,"%i %i %i",&meshi->ibar,&meshi->jbar,&meshi->kbar);
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ PDIM ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(Match(buffer,"PDIM") == 1){
      meshdata *meshi;

      meshi=meshinfo+ipdim;
      ipdim++;
      FGETS(buffer,BUFFERSIZE,streamsmv);
      sscanf(buffer,"%f %f %f %f %f %f",&meshi->xbar0,&meshi->xbar,&meshi->ybar0,&meshi->ybar,&meshi->zbar0,&meshi->zbar);
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ SYST ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(Match(buffer,"SYST") == 1){
      if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
      GLOBsyst=1;
      TrimBack(buffer);
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ OFFSET ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(Match(buffer,"OFFSET") == 1){
      float dummy;

      ioffset++;
      FGETS(buffer,255,streamsmv);
      sscanf(buffer,"%f %f %f",&dummy,&dummy,&dummy);
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ SMOKE3D ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(Match(buffer,"SMOKE3D") == 1||Match(buffer, "SMOKF3D") == 1||Match(buffer, "SMOKG3D") == 1){
      smoke3d *smoke3di;
      FILE_SIZE filesize;
      int filelen;
      char *buffer2;

      smoke3di = smoke3dinfo + ismoke3d;
      smoke3di->unit_start=unit_start++;
      smoke3di->inuse=0;
      smoke3di->compressed=0;
      smoke3di->smokemesh=meshinfo + ioffset - 1;
      smoke3di->file_type = C_FILE;
      if(Match(buffer, "SMOKF3D") == 1||Match(buffer, "SMOKG3D") == 1){
        smoke3di->file_type = FORTRAN_FILE;
      }

      if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
      TrimBack(buffer);
      buffer2=TrimFront(buffer);
      filelen=strlen(buffer2);
      if(GLOBsourcedir!=NULL){
        filelen+=strlen(GLOBsourcedir)+1;
      }
      if(filelen<=0)break;
      if(GetFileInfo(buffer2,GLOBsourcedir,&filesize)==0){
        NewMemory((void **)&smoke3di->file,(unsigned int)(filelen+1));
        NewMemory((void **)&smoke3di->filebase,(unsigned int)(filelen+1));
        STRCPY(smoke3di->filebase,buffer2);
        if(GLOBsourcedir!=NULL){
          STRCPY(smoke3di->file,GLOBsourcedir);
          STRCAT(smoke3di->file,buffer2);
        }
        else{
          STRCPY(smoke3di->file,buffer2);
        }
        smoke3di->filesize=filesize;
        if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
        buffer2 = TrimFront(buffer);
        TrimBack(buffer2);
        if(strcmp(buffer2,"HRRPUV")==0){
          smoke3di->is_soot=0;
        }
        else{
          smoke3di->is_soot=1;
        }
        ismoke3d++;
      }
      else{
        fprintf(stderr,"*** Warning: the file, %s, does not exist.\n",buffer);
        nsmoke3dinfo--;
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++ CLASS_OF_PARTICLES +++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */

    if(Match(buffer,"CLASS_OF_PARTICLES") == 1){
      partclassdata *partclassi;
      int j;
      char *percen;

      partclassi = partclassinfo + npartclassinfo;

      FGETS(buffer,BUFFERSIZE,streamsmv);
      percen=strchr(buffer,'%');
      if(percen!=NULL)percen=0;
      TrimBack(buffer);
      NewMemory((void **)&partclassi->name,strlen(buffer)+1);
      strcpy(partclassi->name,buffer);

      FGETS(buffer,BUFFERSIZE,streamsmv);

      FGETS(buffer,BUFFERSIZE,streamsmv);
      sscanf(buffer,"%i",&partclassi->ntypes);
      if(partclassi->ntypes>0){
        NewMemory((void **)&partclassi->labels,partclassi->ntypes*sizeof(flowlabels));
        for(j=0;j<partclassi->ntypes;j++){
          flowlabels *labelj;
          partpropdata *part5propi;

          labelj = partclassi->labels+j;
          labelj->longlabel=NULL;
          labelj->shortlabel=NULL;
          labelj->unit=NULL;
          ReadLabels(labelj,streamsmv,NULL);
          part5propi=GetPartProp(labelj->shortlabel);
          if(part5propi==NULL){
            part5propi = part5propinfo + npart5propinfo;
            part5propi->label.longlabel=labelj->longlabel;
            part5propi->label.shortlabel=labelj->shortlabel;
            part5propi->label.unit=labelj->unit;
            part5propi->setvalmin=0;
            part5propi->valmin=1.0;
            part5propi->setvalmax=0;
            part5propi->valmax=0.0;
            npart5propinfo++;
          }
        }
      }
      npartclassinfo++;
      continue;
    }

  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ PART ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(Match(buffer,"PRT5") == 1){
      int version_local=0,meshindex=1;
      int i;
      char *buffer2;
      int len;
      part *parti;
      FILE_SIZE filesize;

      len=strlen(buffer);
      if(len>4){
        buffer2=buffer+4;
        sscanf(buffer2,"%i %i",&meshindex,&version_local);
        meshindex--;

      }

      parti = partinfo + npartinfo;
      parti->unit_start=unit_start++;
      parti->partmesh = meshinfo + meshindex;
      parti->inuse=0;
      parti->compressed=0;
      parti->compressed2=0;
      parti->inuse_part2iso=0;

      if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
      TrimBack(buffer);
      buffer2=TrimFront(buffer);
      if(strlen(buffer2)==0)break;
      if(GetFileInfo(buffer2,GLOBsourcedir,&filesize)==0){
        int lendir=0;

        if(GLOBsourcedir!=NULL)lendir=strlen(GLOBsourcedir);
        NewMemory((void **)&parti->file,(unsigned int)(strlen(buffer2)+lendir+1));
        NewMemory((void **)&parti->filebase,(unsigned int)(strlen(buffer2)+1));
        STRCPY(parti->filebase,buffer2);
        if(GLOBsourcedir!=NULL){
          STRCPY(parti->file,GLOBsourcedir);
          STRCAT(parti->file,buffer2);
        }
        else{
          STRCPY(parti->file,buffer2);
        }
        parti->filesize=filesize;
        npartinfo++;
      }
      else{
        fprintf(stderr,"*** Warning: the file, %s, does not exist.\n",buffer2);
      }
      if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
      sscanf(buffer,"%i",&parti->nclasses);
      if(parti->nclasses>0){
        NewMemory((void **)&parti->classptr,parti->nclasses*sizeof(partclassdata *));
      }
      else{
        parti->nclasses=0;
      }
      for(i=0;i<parti->nclasses;i++){
        int classindex;

        if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
        sscanf(buffer,"%i",&classindex);
        parti->classptr[i]=partclassinfo + classindex - 1;
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ BNDF ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(
      Match(buffer, "BNDF") == 1 || Match(buffer, "BNDC") == 1 || Match(buffer, "BNDE") == 1){
      int version_local=0,dummy;
      char *buffer2;
      int len;
      FILE_SIZE filesize;
      patchdata *patchi;

      len=strlen(buffer);
      if(len>4){
        buffer2=buffer+4;
        sscanf(buffer2,"%i %i",&dummy,&version_local);
      }

      patchi = patchinfo + ipatch;
      patchi->unit_start = unit_start++;
      patchi->inuse=0;
      patchi->compressed=0;
      patchi->version=version_local;
      patchi->is_geom = 0;
      if(Match(buffer, "BNDE") == 1)patchi->is_geom = 1;

      if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
      TrimBack(buffer);
      buffer2=TrimFront(buffer);
      if(strlen(buffer2)==0)break;
      if(GetFileInfo(buffer2,GLOBsourcedir,&filesize)==0){
        int lendir=0;

        if(GLOBsourcedir!=NULL)lendir=strlen(GLOBsourcedir);
        NewMemory((void **)&patchi->boundfile, ( unsigned int )(strlen(buffer2) + lendir +strlen(".bnd") + 1));
        NewMemory((void **)&patchi->file,(unsigned int)(strlen(buffer2)+lendir+1));
        NewMemory((void **)&patchi->filebase,(unsigned int)(strlen(buffer2)+1));
        STRCPY(patchi->filebase,buffer2);
        if(GLOBsourcedir!=NULL){
          STRCPY(patchi->file,GLOBsourcedir);
          STRCAT(patchi->file,buffer2);
        }
        else{
          STRCPY(patchi->file,buffer2);
        }
        STRCPY(patchi->boundfile, patchi->file);
        STRCAT(patchi->boundfile, ".bnd");
        if(patchi->is_geom == 1){
          if(FGETS(buffer, BUFFERSIZE, streamsmv) == NULL)break;
        }
        if(ReadLabels(&patchi->label,streamsmv,NULL)==LABEL_ERR){
          fprintf(stderr,"*** Warning: problem reading BNDF entry\n");
          break;
        }
        patchi->filesize=filesize;
        if(patchi->is_geom==0){
          int npatches, error;
          FILE *boundaryunitnumber;

          // TODO: why was this unit set to 15?
          // boundaryunitnumber=15;
          getboundaryheader1(patchi->file,&boundaryunitnumber, &npatches, &error);
          if(npatches>0){
            int *pi1, *pi2, *pj1, *pj2, *pk1, *pk2, *patchdir, *patchsize;
            int i;

            NewMemory((void **)&pi1,npatches*sizeof(int));
            NewMemory((void **)&pi2,npatches*sizeof(int));
            NewMemory((void **)&pj1,npatches*sizeof(int));
            NewMemory((void **)&pj2,npatches*sizeof(int));
            NewMemory((void **)&pk1,npatches*sizeof(int));
            NewMemory((void **)&pk2,npatches*sizeof(int));
            NewMemory((void **)&patchdir,npatches*sizeof(int));
            NewMemory((void **)&patchsize,npatches*sizeof(int));
            patchi->pi1=pi1;
            patchi->pi2=pi2;
            patchi->pj1=pj1;
            patchi->pj2=pj2;
            patchi->pk1=pk1;
            patchi->pk2=pk2;
            patchi->patchdir=patchdir;
            patchi->npatches=npatches;
            patchi->patchsize=patchsize;
            getboundaryheader2(boundaryunitnumber, version_local, npatches, pi1, pi2, pj1, pj2, pk1, pk2, patchdir);
            for(i=0;i<npatches;i++){
              patchi->patchsize[i] = (pi2[i]+1-pi1[i])*(pj2[i]+1-pj1[i])*(pk2[i]+1-pk1[i]);
            }
            CheckMemory;
          }
        }
        ipatch++;
      }
      else{
        fprintf(stderr,"*** Warning: the file, %s, does not exist.\n",buffer);
        if(ReadLabels(&patchi->label,streamsmv,NULL)==LABEL_ERR)break;
        npatchinfo--;
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ SLCF ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(
      Match(buffer,"SLCF") == 1||
      Match(buffer,"SLCC") == 1||
      Match(buffer, "SLCD") == 1 ||
      Match(buffer,"SLCT") == 1)
    {
      int version_local=0,dummy;
      char *buffer2;
      int len;
      FILE_SIZE filesize;
      slicedata *slicei;
      int blocknumber;

      len=strlen(buffer);
      if(len>4){
        buffer2=buffer+4;
        sscanf(buffer2,"%i %i",&dummy,&version_local);
      }

      if(nmeshes>1){
        blocknumber=ioffset-1;
      }
      else{
        blocknumber=0;
      }
      if(len>5){
        buffer2=buffer+4;
        sscanf(buffer2,"%i",&blocknumber);
        blocknumber--;
      }

      slicei = sliceinfo + islice;
      slicei->blocknumber=blocknumber;
      slicei->unit_start=unit_start++;
      slicei->version=version_local;
      slicei->inuse=0;
      slicei->involuse=0;
      slicei->compressed=0;
      slicei->vol_compressed=0;

      if(FGETS(buffer,BUFFERSIZE,streamsmv)==NULL)break;
      TrimBack(buffer);
      buffer2=TrimFront(buffer);
      if(strlen(buffer2)==0)break;
      if(GetFileInfo(buffer2,GLOBsourcedir,&filesize)==0){
        int lendir=0;

        if(GLOBsourcedir!=NULL)lendir=strlen(GLOBsourcedir);
        NewMemory((void **)&slicei->file,(unsigned int)(strlen(buffer2)+lendir+1));
        NewMemory((void **)&slicei->filebase,(unsigned int)(strlen(buffer2)+1));
        NewMemory((void **)&slicei->boundfile,(unsigned int)(strlen(buffer2)+lendir+4+1));

        STRCPY(slicei->filebase,buffer2);
        STRCPY(slicei->file,"");
        if(GLOBsourcedir!=NULL){
          STRCAT(slicei->file,GLOBsourcedir);
        }
        STRCAT(slicei->file,buffer2);

        STRCPY(slicei->boundfile,slicei->file);
        STRCAT(slicei->boundfile,".bnd");

        if(ReadLabels(&slicei->label,streamsmv,NULL)==LABEL_ERR){
          fprintf(stderr,"*** Warning: problem reading SLCF entry\n");
          break;
        }
        slicei->filesize=filesize;
        islice++;
      }
      else{
        fprintf(stderr,"*** Warning: the file, %s, does not exist.\n",buffer2);
        if(ReadLabels(&sliceinfo[islice].label,streamsmv,NULL)==LABEL_ERR)break;
        nsliceinfo--;
      }
      continue;
    }
  }
  {
    int i;

    for(i=0;i<nmeshes;i++){
      meshdata *meshi;
      int ii, jj, kk;
      float *xplt, *yplt, *zplt;
      float *xpltcell, *ypltcell, *zpltcell;

      meshi = meshinfo + i;
      meshi->dx = (meshi->xbar-meshi->xbar0)/meshi->ibar;
      meshi->dy = (meshi->ybar-meshi->ybar0)/meshi->jbar;
      meshi->dz = (meshi->zbar-meshi->zbar0)/meshi->kbar;
      meshi->dxx = (meshi->xbar-meshi->xbar0)/65535;
      meshi->dyy = (meshi->ybar-meshi->ybar0)/65535;
      meshi->dzz = (meshi->zbar-meshi->zbar0)/65535;

      meshi->xplt=NULL;
      NewMemory((void **)&meshi->xplt,(meshi->ibar+1)*sizeof(float));
      xplt = meshi->xplt;
      for(ii=0;ii<meshi->ibar;ii++){
        xplt[ii] = meshi->xbar0 + ii*meshi->dx;
      }
      xplt[meshi->ibar]=meshi->xbar;
      CheckMemory;

      meshi->xpltcell=NULL;
      NewMemory((void **)&meshi->xpltcell,(meshi->ibar+2)*sizeof(float));
      xpltcell = meshi->xpltcell+1;
      for(ii=-1;ii<meshi->ibar+1;ii++){
        xpltcell[ii] = meshi->xbar0 + (ii+0.5)*meshi->dx;
      }
      CheckMemory;

      meshi->yplt=NULL;
      NewMemory((void **)&meshi->yplt,(meshi->jbar+1)*sizeof(float));
      yplt = meshi->yplt;
      for(jj=0;jj<meshi->jbar;jj++){
        yplt[jj] = meshi->ybar0 + jj*meshi->dy;
      }
      yplt[meshi->jbar]=meshi->ybar;
      CheckMemory;

      meshi->ypltcell=NULL;
      NewMemory((void **)&meshi->ypltcell,(meshi->jbar+2)*sizeof(float));
      ypltcell = meshi->ypltcell+1;
      for(ii=-1;ii<meshi->jbar+1;ii++){
        ypltcell[ii] = meshi->ybar0 + (ii+0.5)*meshi->dy;
      }
      CheckMemory;

      meshi->zplt=NULL;
      NewMemory((void **)&meshi->zplt,(meshi->kbar+1)*sizeof(float));
      zplt = meshi->zplt;
      for(kk=0;kk<meshi->kbar;kk++){
        zplt[kk] = meshi->zbar0 + kk*meshi->dz;
      }
      zplt[meshi->kbar]=meshi->zbar;
      CheckMemory;

      meshi->zpltcell=NULL;
      NewMemory((void **)&meshi->zpltcell,(meshi->kbar+2)*sizeof(float));
      zpltcell = meshi->zpltcell+1;
      for(ii=-1;ii<meshi->kbar+1;ii++){
        zpltcell[ii] = meshi->zbar0 + (ii+0.5)*meshi->dz;
      }
      CheckMemory;

    }
  }
  InitVolRender();
  InitBoundaryBounds();
  InitSliceBounds();
  return 0;
}

/* ------------------ ReadINI ------------------------ */

void ReadINI(char *casenameini){
  char *smoketemp;
  char globalini[256],smokeviewini[256];
  char *globaliniptr, *smokeviewiniptr;

  smoketemp = getenv("SMOKEVIEWINI");
  if(smoketemp==NULL)smoketemp=getenv("smokeviewini");
  if(smoketemp==NULL)smoketemp=getenv("svini");
  if(smoketemp==NULL)smoketemp=getenv("SVINI");
  if(smoketemp!=NULL){
    strcpy(globalini,smoketemp);
    strcat(globalini,dirseparator);
    strcat(globalini,"smokeview.ini");
  }
  globaliniptr=globalini;
  smokeviewiniptr=smokeviewini;
  strcpy(smokeviewini,"smokeview.ini");
  if(smoketemp!=NULL)ReadINI2(globaliniptr);
  ReadINI2(smokeviewiniptr);
  ReadINI2(casenameini);
}

/* ------------------ ReadINI2 ------------------------ */

void ReadINI2(char *inifile){
  char buffer[255],buffer2[255];
  char *type_buffer;
  FILE *stream;

  stream=FOPEN(inifile,"r");
  if(stream==NULL)return;

  while(!feof(stream)){
    if(fgets(buffer,BUFFERSIZE,stream)==NULL)break;
    TrimBack(buffer);

    if(Match(buffer,"V_SLICE")==1|| Match(buffer, "V2_SLICE") == 1){
      int setslicemin, setslicemax;
      float slicemin, slicemax;
      int type=0;

      if(Match(buffer, "V2_SLICE") == 1)type = 1;
      fgets(buffer,BUFFERSIZE,stream);
      strcpy(buffer2,"");
      sscanf(buffer,"%i %f %i %f %s",&setslicemin,&slicemin,&setslicemax,&slicemax,buffer2);
      if(type == 1){
        if(setslicemin == 0)setslicemin = 1;
        if(setslicemax == 0)setslicemax = 1;
      }
      type_buffer=TrimFront(buffer2);

      bounddata *pb;

      pb = GetSliceBoundInfo(type_buffer);
      if(pb != NULL){
        if(setslicemin == 1){
          pb->setvalmin = 1;
          pb->valmin = slicemin;
        }
        if(setslicemax == 1){
          pb->setvalmax = 1;
          pb->valmax = slicemax;
        }
      }
      continue;
    }
    if(Match(buffer,"V_BOUNDARY")==1|| Match(buffer, "V2_BOUNDARY") == 1){
      int setpatchmin, setpatchmax;
      float patchmin, patchmax;
      int type=0;

      if(Match(buffer, "V2_BOUNDARY") == 1)type = 1;
      fgets(buffer,BUFFERSIZE,stream);
      strcpy(buffer2,"");
      sscanf(buffer,"%i %f %i %f %s",&setpatchmin,&patchmin,&setpatchmax,&patchmax,buffer2);
      if(type == 1){
        if(setpatchmin == 0)setpatchmin = 1;
        if(setpatchmax == 0)setpatchmax = 1;
      }
      type_buffer=TrimFront(buffer2);
      TrimBack(type_buffer);
      bounddata *pb;

      pb = GetPatchBoundInfo(type_buffer);
      if(pb != NULL){
        if(setpatchmin == 1){
          pb->setvalmin = 1;
          pb->valmin = patchmin;
        }
        if(setpatchmax == 1){
          pb->setvalmax = 1;
          pb->valmax = patchmax;
        }
      }
      continue;
    }
    if(GLOBframeskip<1&&Match(buffer,"SLICEZIPSTEP")==1){
	    fgets(buffer,BUFFERSIZE,stream);
	    sscanf(buffer,"%i",&GLOBslicezipstep);
	    if(GLOBslicezipstep<1)GLOBslicezipstep=1;
      continue;
    }
    if(GLOBframeskip<1&&Match(buffer,"SMOKE3DZIPSTEP")==1){
	    fgets(buffer,BUFFERSIZE,stream);
	    sscanf(buffer,"%i",&GLOBsmoke3dzipstep);
	    if(GLOBsmoke3dzipstep<1)GLOBsmoke3dzipstep=1;
      continue;
    }
    if(GLOBframeskip<1&&Match(buffer,"BOUNDZIPSTEP")==1){
	    fgets(buffer,BUFFERSIZE,stream);
	    sscanf(buffer,"%i",&GLOBboundzipstep);
	    if(GLOBboundzipstep<1)GLOBboundzipstep=1;
      continue;
    }

    if(Match(buffer,"V_PARTICLES")==1||Match(buffer,"V2_PARTICLES")==1){
      int setpartmin, setpartmax;
      float partmin, partmax;
      partpropdata *partpropi;
      int type=1;


      if(Match(buffer,"V2_PARTICLES")==1)type = 1;
      fgets(buffer,BUFFERSIZE,stream);
      strcpy(buffer2,"");
      sscanf(buffer,"%i %f %i %f %s",&setpartmin,&partmin,&setpartmax,&partmax,buffer2);
      if(type == 1){
        if(setpartmin == 0)setpartmin = 1;
        if(setpartmax == 0)setpartmax = 1;
      }
      type_buffer=TrimFront(buffer2);
      TrimBack(type_buffer);
      partpropi=GetPartProp(type_buffer);
      if(partpropi!=NULL){
        partpropi->setvalmax=setpartmax;
        partpropi->setvalmin=setpartmin;
        partpropi->valmax=partmax;
        partpropi->valmin=partmin;
      }
      continue;
    }
  }
  fclose(stream);
  return;
}
