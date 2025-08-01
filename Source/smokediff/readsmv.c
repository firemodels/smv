#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "svdiff.h"
#include "dmalloc.h"
#include "getdata.h"

/* ------------------ ReadSMV ------------------------ */

int ReadSMV(bufferstreamdata *streamsmv, FILE *stream_out, casedata *smvcase){

  int igrid,ipdim;
  int islice,iplot3d,iboundary;
  char buffer[255];
  meshdata *meshinfo=NULL;
  slice *sliceinfo=NULL;
  boundary *boundaryinfo=NULL;
  plot3d *plot3dinfo=NULL;
  int nmeshes, nsliceinfo, nplot3dinfo, nboundary_files;
  int itrnx, itrny, itrnz;

  ipdim=0;
  nmeshes=0;
  nsliceinfo=0;
  nplot3dinfo=0;
  nboundary_files=0;
  itrnx=0;
  itrny=0;
  itrnz=0;

  while(!FEOF(streamsmv)){
    if(FGETS(buffer,255,streamsmv)==NULL)break;
    CheckMemory;
    if(strncmp(buffer," ",1)==0)continue;

    if(
      Match(buffer,"SLCF") == 1||
      Match(buffer,"SLCC") == 1||
      Match(buffer,"SLCT") == 1
      ){
      nsliceinfo++;
      continue;
    }
    if(Match(buffer,"BNDF") == 1||
       Match(buffer,"BNDC") == 1
       ){
      nboundary_files++;
      continue;
    }
    if(
      Match(buffer,"PL3D") == 1){
      nplot3dinfo++;
      continue;
    }
    if(Match(buffer,"GRID") == 1){
      nmeshes++;
      continue;
    }
    if(Match(buffer,"PDIM") == 1){
      ipdim++;
      continue;
    }
  }

  if(nmeshes!=ipdim){
    fprintf(stderr,"*** Error (fatal): number of GRID statements (%i) not equal to\n",nmeshes);
    fprintf(stderr,"                 number of PDIM statements (%i)\n",ipdim);
    exit(0);
  }

  // allocate memory for mesh info

  if(nmeshes>0&&nmeshes==ipdim){
    NewMemory((void **)&meshinfo,nmeshes*sizeof(meshdata));
  }
  smvcase->meshinfo = meshinfo;
  smvcase->nmeshes = nmeshes;

  // allocate memory for slice file info

  if(nsliceinfo>0){
    slice *slicei;
    int i;

    NewMemory((void **)&sliceinfo,nsliceinfo*sizeof(slice));
    for(i=0;i<nsliceinfo;i++){
      slicei = sliceinfo + i;
      slicei->file=NULL;
    }
  }
  smvcase->sliceinfo=sliceinfo;
  smvcase->nsliceinfo = nsliceinfo;

  // allocate memory for boundary file info

  if(nboundary_files>0){
    boundary *boundaryi;
    int i;

    NewMemory((void **)&boundaryinfo,nboundary_files*sizeof(boundary));
    for(i=0;i<nboundary_files;i++){
      boundaryi = boundaryinfo + i;
      boundaryi->file=NULL;
    }
  }
  smvcase->boundaryinfo = boundaryinfo;
  smvcase->nboundary_files = nboundary_files;

  // allocate memory for plot3d file info

  if(nplot3dinfo>0){
    NewMemory((void **)&plot3dinfo,nplot3dinfo*sizeof(plot3d));
  }
  smvcase->nplot3dinfo = nplot3dinfo;
  smvcase->plot3dinfo = plot3dinfo;

  islice=0;
  iplot3d=0;
  iboundary=0;
  ipdim=0;
  igrid=0;
  REWIND(streamsmv);
  while(!FEOF(streamsmv)){
    if(FGETS(buffer,255,streamsmv)==NULL)break;
    CheckMemory;
    if(stream_out==NULL&&strncmp(buffer," ",1)==0)continue;

  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ GRID ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(Match(buffer,"GRID") == 1 && meshinfo != NULL){
      meshdata *meshi;
      float *xp, *yp, *zp;
      int ibar, jbar, kbar;

      meshi=meshinfo+igrid;
      igrid++;
      FGETS(buffer,255,streamsmv);
      sscanf(buffer,"%i %i %i",&ibar,&jbar,&kbar);
      NewMemory((void **)&xp,sizeof(float)*(ibar+1));
      NewMemory((void **)&yp,sizeof(float)*(jbar+1));
      NewMemory((void **)&zp,sizeof(float)*(kbar+1));
      meshi->ibar=ibar;
      meshi->jbar=jbar;
      meshi->kbar=kbar;
      meshi->xplt=xp;
      meshi->yplt=yp;
      meshi->zplt=zp;

      if(stream_out!=NULL){
        TrimBack(buffer);
        fprintf(stream_out,"GRID\n%s\n",buffer);
      }

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
      FGETS(buffer,255,streamsmv);
      sscanf(buffer,"%f %f %f %f %f %f",&meshi->xbar0,&meshi->xbar,&meshi->ybar0,&meshi->ybar,&meshi->zbar0,&meshi->zbar);
      if(stream_out!=NULL){
        TrimBack(buffer);
        fprintf(stream_out,"PDIM\n%s\n",buffer);
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ TRNX ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(meshinfo != NULL && Match(buffer,"TRNX")==1){
      float *xpltcopy, *xplt;
      int ibar, idummy, nn;
      meshdata *meshi;

      if(stream_out!=NULL){
        TrimBack(buffer);
        fprintf(stream_out,"%s\n",buffer);
      }
      itrnx++;
      meshi = meshinfo + itrnx - 1;
      xpltcopy=meshi->xplt;
      xplt = meshi->xplt;

      ibar=meshi->ibar;
      FGETS(buffer,255,streamsmv);
      if(stream_out!=NULL){
        TrimBack(buffer);
        fprintf(stream_out,"%s\n",buffer);
      }
      sscanf(buffer,"%i ",&idummy);
      for(nn=0;nn<idummy;nn++){
        FGETS(buffer,255,streamsmv);
        if(stream_out!=NULL){
          TrimBack(buffer);
          fprintf(stream_out,"%s\n",buffer);
        }
      }
      for(nn=0;nn<=ibar;nn++){
        FGETS(buffer,255,streamsmv);
        if(stream_out!=NULL){
          TrimBack(buffer);
          fprintf(stream_out,"%s\n",buffer);
        }
        sscanf(buffer,"%i %f",&idummy,xpltcopy);
        xpltcopy++;
      }
      meshi->dx=xplt[1]-xplt[0];

      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ TRNY ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(meshinfo != NULL && Match(buffer,"TRNY")==1){
      float *ypltcopy, *yplt;
      int jbar, idummy, nn;
      meshdata *meshi;

      if(stream_out!=NULL){
        TrimBack(buffer);
        fprintf(stream_out,"%s\n",buffer);
      }
      itrny++;
      meshi = meshinfo + itrny - 1;
      yplt = meshi->yplt;
      ypltcopy=meshi->yplt;
      jbar=meshi->jbar;
      FGETS(buffer,255,streamsmv);
      if(stream_out!=NULL){
        TrimBack(buffer);
        fprintf(stream_out,"%s\n",buffer);
      }
      sscanf(buffer,"%i ",&idummy);
      for(nn=0;nn<idummy;nn++){
        FGETS(buffer,255,streamsmv);
        if(stream_out!=NULL){
          TrimBack(buffer);
          fprintf(stream_out,"%s\n",buffer);
        }
      }
      for(nn=0;nn<=jbar;nn++){
        FGETS(buffer,255,streamsmv);
        if(stream_out!=NULL){
          TrimBack(buffer);
          fprintf(stream_out,"%s\n",buffer);
        }
        sscanf(buffer,"%i %f",&idummy,ypltcopy);
        ypltcopy++;
      }
      meshi->dy=yplt[1]-yplt[0];
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ TRNZ ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(meshinfo != NULL && Match(buffer,"TRNZ")==1){
      float *zpltcopy,*zplt;
      int kbar, idummy, nn;
      meshdata *meshi;

      if(stream_out!=NULL){
        TrimBack(buffer);
        fprintf(stream_out,"%s\n",buffer);
      }
      itrnz++;
      meshi = meshinfo + itrnz - 1;
      zplt = meshi->zplt;
      zpltcopy=meshi->zplt;
      kbar=meshi->kbar;
      FGETS(buffer,255,streamsmv);
      if(stream_out!=NULL){
        TrimBack(buffer);
        fprintf(stream_out,"%s\n",buffer);
      }
      sscanf(buffer,"%i ",&idummy);
      for(nn=0;nn<idummy;nn++){
        FGETS(buffer,255,streamsmv);
        if(stream_out!=NULL){
          TrimBack(buffer);
          fprintf(stream_out,"%s\n",buffer);
        }
      }
      for(nn=0;nn<=kbar;nn++){
        FGETS(buffer,255,streamsmv);
        if(stream_out!=NULL){
          TrimBack(buffer);
          fprintf(stream_out,"%s\n",buffer);
        }
        sscanf(buffer,"%i %f",&idummy,zpltcopy);
        zpltcopy++;
      }
      meshi->dz = zplt[1]-zplt[0];
      continue;
    }

   /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ PL3D ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(plot3dinfo != NULL && Match(buffer,"PL3D") == 1){
      meshdata *plot3dmesh;
      plot3d *plot3di;
      float time_local;
      int meshnumber=1;
      char full_file[1024];
      FILE_SIZE filesize;

      time_local = 0.0;
      if(strlen(buffer)>4){
        sscanf(buffer+4,"%f %i",&time_local,&meshnumber);
      }

      plot3dmesh = meshinfo + meshnumber - 1;

      plot3di = plot3dinfo + iplot3d;
      plot3di->plot3dmesh=plot3dmesh;
      plot3di->time=time_local;
      TrimBack(buffer);
      strcpy(plot3di->keyword,buffer);

      FGETS(buffer,255,streamsmv);
      FullFile(full_file,smvcase->dir,buffer);
      if(GetFileInfo(full_file,NULL,&filesize)==0){
        int i;

        NewMemory((void **)&plot3di->file,(unsigned int)(strlen(full_file)+1));
        for(i = 0; i < 5; i++){
          NewMemory((void **)&plot3di->histogram[i], sizeof(histogramdata));
          InitHistogram(plot3di->histogram[i],NHIST_BUCKETS, NULL, NULL);
        }

        CheckMemory;
        strcpy(plot3di->file,TrimFront(buffer));
        CheckMemory;
        if(ReadLabels(plot3di->labels+0,streamsmv,NULL)==LABEL_ERR)break;
        if(ReadLabels(plot3di->labels+1,streamsmv,NULL)==LABEL_ERR)break;
        if(ReadLabels(plot3di->labels+2,streamsmv,NULL)==LABEL_ERR)break;
        if(ReadLabels(plot3di->labels+3,streamsmv,NULL)==LABEL_ERR)break;
        if(ReadLabels(plot3di->labels+4,streamsmv,NULL)==LABEL_ERR)break;

        CheckMemory;

        iplot3d++;
      }
      else{
        if(display_warnings==1)fprintf(stderr,"*** Warning: the file, %s, does not exist.\n",full_file);
        CheckMemory;
        if(ReadLabels(plot3di->labels+0,streamsmv,NULL)==LABEL_ERR)break;
        if(ReadLabels(plot3di->labels+1,streamsmv,NULL)==LABEL_ERR)break;
        if(ReadLabels(plot3di->labels+2,streamsmv,NULL)==LABEL_ERR)break;
        if(ReadLabels(plot3di->labels+3,streamsmv,NULL)==LABEL_ERR)break;
        if(ReadLabels(plot3di->labels+4,streamsmv,NULL)==LABEL_ERR)break;
        nplot3dinfo--;
        smvcase->nplot3dinfo=nplot3dinfo;
      }
      continue;
    }

  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ SLCF ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(sliceinfo != NULL && smvcase->meshinfo != NULL &&
     (
      Match(buffer,"SLCF") == 1||
      Match(buffer,"SLCC") == 1||
      Match(buffer,"SLCT") == 1)
      )
    {
      int version_local=0;
      int len;
      FILE_SIZE filesize;
      slice *slicei;
      int meshnumber=0;
      meshdata *slicemesh;
      char full_file[1024];

      len=strlen(buffer);
      if(len>4){
        sscanf(buffer+4,"%i %i",&meshnumber,&version_local);
      }

      slicei = sliceinfo + islice;

      slicemesh = smvcase->meshinfo+meshnumber-1;
      slicei->slicemesh = slicemesh;
      TrimBack(buffer);

      strcpy(slicei->keyword,buffer);

      if(Match(buffer,"SLCF") == 1){
        slicei->slicetype= SLICE_NODE_CENTER;
      }
      if(Match(buffer,"SLCC") == 1){
          slicei->slicetype = SLICE_CELL_CENTER;
      }
      if(Match(buffer,"SLCT") == 1){
        slicei->slicetype= SLICE_TERRAIN;
      }

      slicei->version=version_local;

      if(FGETS(buffer,255,streamsmv)==NULL)break;
      TrimBack(buffer);
      if(strlen(buffer)==0)break;
      FullFile(full_file,smvcase->dir,buffer);
      if(GetFileInfo(full_file,NULL,&filesize)==0){
        int is1=-1, is2=-1, js1=-1, js2=-1, ks1=-1, ks2=-1;
        int ni, nj, nk;
        int error;
        float *xplt, *yplt, *zplt;

        NewMemory((void **)&slicei->file,(unsigned int)(strlen(full_file)+1));
        NewMemory((void **)&slicei->histogram,sizeof(histogramdata));
        InitHistogram(slicei->histogram,NHIST_BUCKETS, NULL, NULL);
        STRCPY(slicei->file, TrimFront(buffer));
        if(ReadLabels(&slicei->label,streamsmv,NULL)==LABEL_ERR){
          fprintf(stderr,"*** Warning: problem reading SLCF entry\n");
          break;
        }
        slicei->filesize=filesize;
        GetSliceParms(full_file,&is1,&is2,&js1,&js2,&ks1,&ks2,&ni,&nj,&nk,&slicei->volslice,&error);
        slicei->is1=is1;
        slicei->is2=is2;
        slicei->js1=js1;
        slicei->js2=js2;
        slicei->ks1=ks1;
        slicei->ks2=ks2;
        xplt = slicemesh->xplt;
        yplt = slicemesh->yplt;
        zplt = slicemesh->zplt;
        slicei->xmin = xplt[is1];
        slicei->xmax = xplt[is2];
        slicei->ymin = yplt[js1];
        slicei->ymax = yplt[js2];
        slicei->zmin = zplt[ks1];
        slicei->zmax = zplt[ks2];
        slicei->slice2=NULL;

        islice++;
      }
      else{
        if(display_warnings==1)fprintf(stderr,"*** Warning: the file, %s, does not exist.\n",buffer);
        if(ReadLabels(&sliceinfo[islice].label,streamsmv,NULL)==LABEL_ERR)break;
        nsliceinfo--;
        smvcase->nsliceinfo=nsliceinfo;
      }
      continue;
    }
  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ BNDF ++++++++++++++++++++++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */
    if(boundaryinfo != NULL && (Match(buffer,"BNDF") == 1||Match(buffer,"BNDC") == 1)){
      int version_local=0;
      int len;
      FILE_SIZE filesize;
      boundary *boundaryi;
      int meshnumber=0;
      meshdata *boundarymesh;
      char full_file[1024];

      len=strlen(buffer);
      if(len>4){
        sscanf(buffer+4,"%i %i",&meshnumber,&version_local);
      }

      boundaryi = boundaryinfo + iboundary;

      boundarymesh = smvcase->meshinfo+meshnumber-1;
      boundaryi->boundarymesh = boundarymesh;
      TrimBack(buffer);

      strcpy(boundaryi->keyword,buffer);

      boundaryi->version=version_local;

      if(Match(buffer,"BNDF") == 1){
        boundaryi->boundarytype=1;
      }
      if(Match(buffer,"BNDC") == 1){
        boundaryi->boundarytype=2;
      }

      if(FGETS(buffer,255,streamsmv)==NULL)break;
      TrimBack(buffer);
      if(strlen(buffer)==0)break;
      FullFile(full_file,smvcase->dir,buffer);
      if(GetFileInfo(full_file,NULL,&filesize)==0){
        int npatches, error;
        FILE *boundaryunitnumber;

        NewMemory((void **)&boundaryi->file,(unsigned int)(strlen(full_file)+1));
        NewMemory((void **)&boundaryi->histogram,sizeof(histogramdata));
        InitHistogram(boundaryi->histogram,NHIST_BUCKETS, NULL, NULL);
        STRCPY(boundaryi->file, TrimFront(buffer));
        if(ReadLabels(&boundaryi->label,streamsmv,NULL)==LABEL_ERR){
          fprintf(stderr,"*** Warning: problem reading BNDF entry\n");
          break;
        }
        boundaryi->filesize=filesize;
        // TODO: why was this set to 15.
        // boundaryunitnumber=15;
        getboundaryheader1(full_file,&boundaryunitnumber, &npatches, &error);
        if(npatches>0){
          int *pi1, *pi2, *pj1, *pj2, *pk1, *pk2, *patchdir, *patch2index, *patchsize, *qoffset;
          int i;

          NewMemory((void **)&pi1,npatches*sizeof(int));
          NewMemory((void **)&pi2,npatches*sizeof(int));
          NewMemory((void **)&pj1,npatches*sizeof(int));
          NewMemory((void **)&pj2,npatches*sizeof(int));
          NewMemory((void **)&pk1,npatches*sizeof(int));
          NewMemory((void **)&pk2,npatches*sizeof(int));
          NewMemory((void **)&patchdir,npatches*sizeof(int));
          NewMemory((void **)&patch2index,npatches*sizeof(int));
          NewMemory((void **)&patchsize,npatches*sizeof(int));
          NewMemory((void **)&qoffset,npatches*sizeof(int));
          boundaryi->pi1=pi1;
          boundaryi->pi2=pi2;
          boundaryi->pj1=pj1;
          boundaryi->pj2=pj2;
          boundaryi->pk1=pk1;
          boundaryi->pk2=pk2;
          boundaryi->patchdir=patchdir;
          boundaryi->patch2index=patch2index;
          boundaryi->npatches=npatches;
          boundaryi->patchsize=patchsize;
          boundaryi->qoffset=qoffset;
          getboundaryheader2(boundaryunitnumber, version_local, npatches, pi1, pi2, pj1, pj2, pk1, pk2, patchdir);
          for(i=0;i<npatches;i++){
            boundaryi->patchsize[i] = (pi2[i]+1-pi1[i])*(pj2[i]+1-pj1[i])*(pk2[i]+1-pk1[i]);
            if(i==0){
              boundaryi->qoffset[i]=0;
            }
            else{
              boundaryi->qoffset[i]=boundaryi->qoffset[i-1]+boundaryi->patchsize[i-1];
            }
          }
          CheckMemory;
       }

        boundaryi->boundary2=NULL;

        iboundary++;
      }
      else{
        fprintf(stderr,"*** Warning: the file, %s, does not exist.\n",buffer);
        if(ReadLabels(&boundaryinfo[iboundary].label,streamsmv,NULL)==LABEL_ERR)break;
        nboundary_files--;
        smvcase->nboundary_files=nboundary_files;
      }
      continue;
    }

  /*
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ++++++++++++++++++++++ vis keywords not differenced++++++++++
    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  */

    // skip over the following keywords

    if(
       Match(buffer,"ISOF") == 1||
       Match(buffer,"ISOG") == 1||
       Match(buffer,"TISOF")==1||
       Match(buffer,"SMOKE3D")==1||
       Match(buffer,"SMOKF3D")==1||
#ifdef pp_CO2SMOKE
       Match(buffer, "SMOKG3D") == 1 ||
#endif
      Match(buffer, "PART") == 1 ||
       Match(buffer,"PRT5")==1   ||
       Match(buffer,"EVA5")==1
       ){
      char comm[1024];

      strcpy(comm,buffer);
      FGETS(buffer,255,streamsmv);
      if(Match(comm,"PRT5")==1||Match(comm,"EVA5")==1){
        int i, nlines;

        FGETS(buffer,255,streamsmv);
        sscanf(buffer,"%i",&nlines);
        for(i=0;i<nlines;i++){
          FGETS(buffer,255,streamsmv);
        }
      }
      else{
        FGETS(buffer,255,streamsmv);
        FGETS(buffer,255,streamsmv);
        FGETS(buffer,255,streamsmv);
      }
      if(Match(comm,"TISOF")==1){
        int i;

        for(i=0;i<3;i++){
          FGETS(buffer,255,streamsmv);
        }
      }
      continue;
    }
    if(stream_out!=NULL){
      TrimBack(buffer);
      fprintf(stream_out,"%s\n",buffer);
    }
    continue;
  }
  if(stream_out!=NULL){
    fprintf(stream_out,"SMOKEDIFF\n");
  }
  return 0;
}

