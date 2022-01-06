#include "options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "viewports.h"
#include "IOobjects.h"

/* ----------------------- DrawLights ----------------------------- */

void DrawLights(float *position0, float *position1){
  float xyz0[3];

  xyz0[0] = (    xbar0ORIG + xbarORIG)/2.0;
  xyz0[1] = (    ybar0ORIG + ybarORIG)/2.0;
  xyz0[2] = (3.0*zbar0ORIG + zbarORIG)/4.0;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-xbar0, -ybar0, -zbar0);
  glLineWidth(10.0);
  glBegin(GL_LINES);
  glColor3f(0.0, 0.0, 0.0);
  if(use_light0==1){
    float xyz1[3];

    glVertex3fv(xyz0);
    xyz1[0] = xyz0[0]+position0[0];
    xyz1[1] = xyz0[1]+position0[1];
    xyz1[2] = xyz0[2]+position0[2];
    glVertex3fv(xyz1);
  }

  if(use_light1==1){
    float xyz1[3];

    glVertex3fv(xyz0);
    xyz1[0] = xyz0[0]+position1[0];
    xyz1[1] = xyz0[1]+position1[1];
    xyz1[2] = xyz0[2]+position1[2];
    glVertex3fv(xyz1);
  }
  glEnd();
  glPopMatrix();
}

/* ------------------ ShowScene2 ------------------------ */

void ShowScene2(int mode){
  if(mode==DRAWSCENE){
    if(rotation_type==EYE_CENTERED&&nskyboxinfo>0)DrawSkybox();
    UpdateLights(light_position0, light_position1);
    if(drawlights==1)DrawLights(light_position0, light_position1);


 // if(render_status==RENDER_ON&&render_mode==RENDER_360){
 //   UpdateLights(light_position0, light_position1);
 // }

    glPointSize((float)1.0);


    /* ++++++++++++++++++++++++ DrawNorth  +++++++++++++++++++++++++ */

    if(vis_northangle == 1){
      CLIP_GEOMETRY;
      DrawNorth();
      SNIFF_ERRORS("after DrawNorth");
    }

    /* ++++++++++++++++++++++++ DrawTrees +++++++++++++++++++++++++ */

    if(ntreeinfo>0){
      CLIP_GEOMETRY;
      DrawTrees();
      SNIFF_ERRORS("after DrawTrees");
    }

    /* ++++++++++++++++++++++++ draw particles +++++++++++++++++++++++++ */

    if(showsmoke == 1){
      CLIP_VALS;
      DrawPartFrame();
    }

    /* ++++++++++++++++++++++++ draw evacuation +++++++++++++++++++++++++ */

    if(showevac == 1){
      CLIP_VALS;
      DrawEvacFrame();
    }

    /* ++++++++++++++++++++++++ draw screeninfo +++++++++++++++++++++++++ */

#ifdef pp_RENDER360_DEBUG
    if(screenview == 1)DrawScreenInfo();
#endif

    /* ++++++++++++++++++++++++ draw circular vents +++++++++++++++++++++++++ */

    if(ncvents>0 && visCircularVents != VENT_HIDE && showpatch==0){
      CLIP_GEOMETRY;
      DrawCircVents(visCircularVents);
    }

    /* ++++++++++++++++++++++++ draw sensors/sprinklers/heat detectors +++++++++++++++++++++++++ */

    CLIP_GEOMETRY;

    DrawDevices(mode);
    if(viswindrose)DrawWindRosesDevices();
    SNIFF_ERRORS("after DrawDevices");

    if(visaxislabels == 1){
      UNCLIP;
      OutputAxisLabels();
      SNIFF_ERRORS("after outputAxisLables");
    }


    /* ++++++++++++++++++++++++ draw user ticks +++++++++++++++++++++++++ */

    if(visUSERticks == 1){
      AntiAliasLine(ON);
      UNCLIP;
      DrawUserTicks();
      AntiAliasLine(OFF);
      SNIFF_ERRORS("after DrawTicks");
    }

    /* ++++++++++++++++++++++++ draw ticks +++++++++++++++++++++++++ */

    if(visFDSticks == 1 && ntickinfo>0){
      UNCLIP;
      DrawTicks();
      SNIFF_ERRORS("after DrawTicks");
    }

    /* ++++++++++++++++++++++++ draw ticks +++++++++++++++++++++++++ */

    if(showgravity_vector == 1){
      UNCLIP;
      DrawGravityAxis();
      SNIFF_ERRORS("after drawaxis");
    }

    /* draw the box framing the simulation (corners at (0,0,0) (xbar,ybar,zbar) */


    /* ++++++++++++++++++++++++ draw simulation frame (corners at (0,0,0) and (xbar,ybar,zbar) +++++++++++++++++++++++++ */


    if(geom_bounding_box_mousedown==1||(isZoneFireModel == 0 && visFrame == 1 && highlight_flag == 2)){
      CLIP_GEOMETRY;
      DrawOutlines();
      SNIFF_ERRORS("after DrawOutlines");
    }

    if(geom_bounding_box_mousedown==1){
      DrawObstBoundingBox();
    }

    if(show_rotation_center == 1){
      unsigned char pcolor[4];

      CLIP_GEOMETRY;
      glPushMatrix();
      glTranslatef(camera_current->xcen, camera_current->ycen, camera_current->zcen);
      pcolor[0] = 255 * foregroundcolor[0];
      pcolor[1] = 255 * foregroundcolor[1];
      pcolor[2] = 255 * foregroundcolor[2];
      DrawSphere(0.03, pcolor);
      glPopMatrix();
    }


    /* ++++++++++++++++++++++++ draw mesh +++++++++++++++++++++++++ */

    if(setPDIM == 1){
      if(visGrid != NOGRID_NOPROBE){
        int igrid;
        meshdata *meshi;

        UNCLIP;
        for(igrid = 0;igrid<nmeshes;igrid++){
          meshi = meshinfo + igrid;
          DrawGrid(meshi);
          SNIFF_ERRORS("DrawGrid");
        }
      }
    }
  } /* end of if(mode==DRAWSCENE) code segment */


    /* ++++++++++++++++++++++++ draw selected devices +++++++++++++++++++++++++ */

  if(mode == SELECTOBJECT){
    if(select_device == 1){
      CLIP_GEOMETRY;

      DrawDevices(mode);
      SNIFF_ERRORS("after drawselect_devices");
      return;
    }
  }

  /* ++++++++++++++++++++++++ draw selected avatars +++++++++++++++++++++++++ */

  if(mode == SELECTOBJECT){
    if(select_avatar == 1){
      CLIP_GEOMETRY;
      DrawSelectAvatars();
      SNIFF_ERRORS("after DrawSelectAvatars");
      return;
    }
  }

  /* ++++++++++++++++++++++++ draw selected avatars +++++++++++++++++++++++++ */
  if(mode==SELECTOBJECT){
    if(select_geom!=GEOM_PROP_NONE){
      CLIP_GEOMETRY;
      DrawSelectGeom();
      SNIFF_ERRORS("after DrawSelectGeom");
      return;
    }
  }

  /* ++++++++++++++++++++++++ DrawSelectTours +++++++++++++++++++++++++ */

  if(mode == SELECTOBJECT){
    if(edittour == 1 && ntourinfo>0){
      CLIP_GEOMETRY;
      DrawSelectTours();
      SNIFF_ERRORS("after DrawSelectTours");
      return;
    }
  }


  /* ++++++++++++++++++++++++ draw tours +++++++++++++++++++++++++ */

  if(showtours == 1){
    CLIP_GEOMETRY;
    DrawTours();
    SNIFF_ERRORS("after DrawTours");
  }

  /* ++++++++++++++++++++++++ draw stereo parallax indicator +++++++++++++++++++++++++ */

  if(show_parallax == 1){
    UNCLIP;
    AntiAliasLine(ON);
    glLineWidth(linewidth);
    glBegin(GL_LINES);
    glColor3fv(foregroundcolor);
    glVertex3f(0.75, 0.0, 0.25);
    glVertex3f(0.75, 1.0, 0.25);
    glEnd();
    AntiAliasLine(OFF);
  }

  /* ++++++++++++++++++++++++ draw blockages +++++++++++++++++++++++++ */

  CLIP_GEOMETRY;
  if(geom_bounding_box_mousedown==0){
    DrawBlockages(mode, DRAW_OPAQUE);
    SNIFF_ERRORS("DrawBlockages");
  }

  /* ++++++++++++++++++++++++ draw triangles +++++++++++++++++++++++++ */

  if(ngeominfoptrs>0){
    CLIP_GEOMETRY;
#ifdef pp_WUI_VAO
    if(have_terrain_vao==0){
      DrawGeom(DRAW_OPAQUE, GEOM_STATIC);
      DrawGeom(DRAW_OPAQUE, GEOM_DYNAMIC);
    }
#else
    if(use_cfaces==1&&ncgeominfo>0){
      int i;

      for(i = 0; i<ncgeominfo; i++){
        geomdata *geomi;

        geomi = cgeominfo+i;
        DrawCGeom(DRAW_OPAQUE, geomi);
      }
    }
    else if(ngeominfoptrs>0){
      DrawGeom(DRAW_OPAQUE, GEOM_STATIC);
      DrawGeom(DRAW_OPAQUE, GEOM_DYNAMIC);
    }
#endif
    SNIFF_ERRORS("DrawGeom");
  }

  /* ++++++++++++++++++++++++ draw shooter points +++++++++++++++++++++++++ */

  if(showshooter != 0 && shooter_active == 1){
    CLIP_VALS;
    DrawShooter();
    SNIFF_ERRORS("DrawShooter");
  }

  /* ++++++++++++++++++++++++ draw terrain +++++++++++++++++++++++++ */

  if(use_cfaces==0||ncgeominfo==0){
#ifdef pp_WUI_VAO
    if(have_terrain_vao==1&&usegpu==1){
      CLIP_GEOMETRY;
      DrawTerrainGeomGPU();
    }
    else{
      CLIP_GEOMETRY;
      DrawTerrainGeom(DRAW_OPAQUE);
    }
#else
    CLIP_GEOMETRY;
    DrawTerrainGeom(DRAW_OPAQUE);
#endif
  }

  if(visTerrainType != TERRAIN_HIDDEN&&nterraininfo>0&&ngeominfo==0 && geom_bounding_box_mousedown==0){
    int i;

    //shaded 17 0
    //stepped 18 1
    //line    19 2
    //texture 20 3
    //hidden 20 4

    CLIP_GEOMETRY;
    for(i = 0;i<nterraininfo;i++){
      terraindata *terri;
      int flag;

      terri = terraininfo + i;
      if(terrain_showonly_top==1){
        flag = TERRAIN_BOTH_SIDES;
      }
      else{
        flag = TERRAIN_TOP_SIDE;
      }
      switch(visTerrainType){
      case TERRAIN_3D:
        DrawTerrainOBST(terri, flag);
        break;
      case TERRAIN_2D_STEPPED:
      case TERRAIN_2D_LINE:
        break;
      case TERRAIN_3D_MAP:
        if(terrain_textures != NULL&&terrain_textures[iterrain_textures].loaded == 1){
          DrawTerrainOBSTTexture(terri);
        }
        else{
          DrawTerrainOBST(terri, flag);
        }
        break;
      default:
        ASSERT(FFALSE);
        break;
      }
    }
    if(visTerrainType==TERRAIN_3D_MAP||visTerrainType==TERRAIN_3D){
      if(terrain_showonly_top==0){
        for(i = 0; i<nmeshes; i++){
          meshdata *meshi;

          meshi = meshinfo+i;
          DrawTerrainOBSTSides(meshi);
        }
      }
    }
  }

  /* ++++++++++++++++++++++++ draw slice files +++++++++++++++++++++++++ */

  if(show_gslice_triangles == 1 || show_gslice_normal == 1 || show_gslice_normal_keyboard == 1 || show_gslice_triangulation == 1){
    CLIP_VALS;
    DrawGSliceOutline();
  }
  if(((show_node_slices_and_vectors == 1 || show_cell_slices_and_vectors==1)&& showvslice == 1) || (showslice == 1 && use_transparency_data == 0)){
    CLIP_VALS;
    if(geom_bounding_box_mousedown==0){
      DrawSliceFrame();
    }
  }

  /* ++++++++++++++++++++++++ draw boundary files +++++++++++++++++++++++++ */

  if(showpatch == 1 && geom_bounding_box_mousedown==0){
    CLIP_VALS;
    DrawBoundaryFrame(DRAW_OPAQUE);
  }

  /* ++++++++++++++++++++++++ draw labels +++++++++++++++++++++++++ */

  if(visLabels == 1){
    CLIP_GEOMETRY;
    DrawLabels();
  }

  /* ++++++++++++++++++++++++ draw animated isosurfaces +++++++++++++++++++++++++ */

  if(showiso == 1 && geom_bounding_box_mousedown==0){
    CLIP_VALS;
    DrawIso(DRAW_OPAQUE);
  }

  /* ++++++++++++++++++++++++ draw zone fire modeling info +++++++++++++++++++++++++ */

  if(nrooms>0){
    CLIP_GEOMETRY;
    DrawZoneRoomGeom();
    SNIFF_ERRORS("after DrawZoneRoomGeom");

    if(showzone == 1){
      CLIP_VALS;
      DrawZoneFireData();
      SNIFF_ERRORS("after DrawZoneFireData");
      if(ReadZoneFile == 1){
        if(nzvents>0){
          DrawZoneVentData();
          SNIFF_ERRORS("after DrawZoneVentData");
        }
        if(have_wall_data==1&&vis_wall_data==1){
          DrawZoneWallData();
          SNIFF_ERRORS("after DrawZoneWallData");
        }
      }
    }
  }

#ifdef pp_SMOKETEST
  if(show3dsmoke==1&&usegpu==0&&hrrpuv_max_blending==1){
    CLIP_VALS;
    DrawSmokeFrame(SMOKE3D_FIRE_ONLY);
  }
#endif

  //**********************************************************************************
  //**********************************************************************************
  //**********************************************************************************
  //    nothing transparent should be drawn before this portion of the code
  //    (ie draw all opaque objects first then draw transparent objects
  //**********************************************************************************
  //**********************************************************************************
  //**********************************************************************************

  /* ++++++++++++++++++++++++ draw terrain +++++++++++++++++++++++++ */

  if(use_cfaces==0||ncgeominfo==0){
#ifdef pp_WUI_VAO
    if(have_terrain_vao==1&&usegpu==1){
    }
    else{
      CLIP_GEOMETRY;
      DrawTerrainGeom(DRAW_TRANSPARENT);
    }
#else
    CLIP_GEOMETRY;
    DrawTerrainGeom(DRAW_TRANSPARENT);
#endif
  }

  /* ++++++++++++++++++++++++ draw triangles +++++++++++++++++++++++++ */

  if(use_cfaces==1&&ncgeominfo>0){
    int i;

    for(i = 0; i<ncgeominfo; i++){
      geomdata *geomi;

      geomi = cgeominfo+i;
      DrawCGeom(DRAW_TRANSPARENT, geomi);
    }
  }
  else if(ngeominfoptrs>0){
    DrawGeom(DRAW_TRANSPARENT, GEOM_STATIC);
    DrawGeom(DRAW_TRANSPARENT, GEOM_DYNAMIC);
  }

  if(showiso == 1 && geom_bounding_box_mousedown==0){
    CLIP_VALS;
    DrawIso(DRAW_TRANSPARENT);
  }

  /* ++++++++++++++++++++++++ draw transparent faces +++++++++++++++++++++++++ */

  CLIP_GEOMETRY;
  DrawTransparentFaces();

  /* ++++++++++++++++++++++++ draw 3D smoke +++++++++++++++++++++++++ */

  if(show3dsmoke == 1 && geom_bounding_box_mousedown==0){
    CLIP_VALS;
#ifdef pp_SMOKETEST
    if(usegpu==0&&hrrpuv_max_blending==1){
      DrawSmokeFrame(SMOKE3D_SMOKE_ONLY);
    }
    else{
      DrawSmokeFrame(SMOKE3D_SMOKE_AND_FIRE);
    }
#else
    DrawSmokeFrame();
#endif
  }

  /* ++++++++++++++++++++++++ draw vol smoke +++++++++++++++++++++++++ */

  if(showvolrender == 1 && show3dsmoke==0 && geom_bounding_box_mousedown==0){
    CLIP_VALS;
    DrawVolSmokeFrame();
  }

  if(show_light_position_direction == 1)DrawLightDirections();
#ifdef pp_SMOKETEST
  if(smoke_test == 1)DrawSmokeTest();
#endif

  if(active_smokesensors == 1 && show_smokesensors != SMOKESENSORS_HIDDEN && geom_bounding_box_mousedown==0){
    CLIP_VALS;
    GetSmokeSensors();

    DrawDevicesVal();
  }

  /* ++++++++++++++++++++++++ draw device plots +++++++++++++++++++++++++ */

  if(mode==DRAWSCENE && geom_bounding_box_mousedown==0){
    if((show_hrrpuv_plot==1&&hrrinfo!=NULL)||showdevice_plot==DEVICE_PLOT_SHOW_ALL||showdevice_plot==DEVICE_PLOT_SHOW_SELECTED){
      DrawDevicePlots();
    }
    if(showdevice_plot==DEVICE_PLOT_SHOW_TREE_ALL){
      DrawTreeDevicePlots();
    }
  }

  /* ++++++++++++++++++++++++ draw zone fire modeling info +++++++++++++++++++++++++ */

  if(nrooms>0 && showzone == 1){
    CLIP_VALS;
    DrawZoneRoomData();
    SNIFF_ERRORS("after DrawZoneRoomData");
  }

  /* ++++++++++++++++++++++++ draw boundary files +++++++++++++++++++++++++ */

  if(showpatch == 1 && geom_bounding_box_mousedown==0){
    CLIP_VALS;
    DrawBoundaryFrame(DRAW_TRANSPARENT);
  }

  /* ++++++++++++++++++++++++ draw slice files +++++++++++++++++++++++++ */

  if((show_node_slices_and_vectors == 1 || show_cell_slices_and_vectors==1) || (showslice == 1 && use_transparency_data == 1)){
    if(nslice_loaded>0||ngeomslice_loaded>0){
      CLIP_VALS;
      if(geom_bounding_box_mousedown==0){
        DrawSliceFrame();
      }
      SNIFF_ERRORS("after DrawSliceFrame");
    }
  }

  /* ++++++++++++++++++++++++ draw transparent blockages +++++++++++++++++++++++++ */

  //  DrawDemo(20,20);
  //  DrawDemo2();
  CLIP_GEOMETRY;
  if(geom_bounding_box_mousedown==0){
    DrawBlockages(mode, DRAW_TRANSPARENT);
  }
  SNIFF_ERRORS("after drawBlokcages");

  /* ++++++++++++++++++++++++ draw vector slice files +++++++++++++++++++++++++ */

  if(showvslice == 1 && geom_bounding_box_mousedown==0){
    CLIP_VALS;
    DrawVSliceFrame();
  }
  SNIFF_ERRORS("after drawvslice");

  /* ++++++++++++++++++++++++ draw plot3d files +++++++++++++++++++++++++ */

  if(showplot3d == 1 && geom_bounding_box_mousedown==0){
    CLIP_VALS;
    DrawPlot3dFrame();
  }
  SNIFF_ERRORS("after DrawPlot3dFrame");
}

/* ------------------ ShowScene ------------------------ */

void ShowScene(int mode, int view_mode, int quad, GLint s_left, GLint s_down, screendata *screen){
  CheckMemory;

  LOCK_IBLANK
  show_mode = mode;

  UNCLIP;

  /* ++++++++++++++++++++++++ update variables as needed +++++++++++++++++++++++++ */

  UpdateShowScene();
  if(stereotype == STEREO_NONE || stereotype == STEREO_TIME)ClearBuffers(mode);

  /* ++++++++++++++++++++++++ setup viewports +++++++++++++++++++++++++ */

  if(mode == DRAWSCENE){
    GetViewportInfo();

    if(clip_rendered_scene == 1){
      ViewportClip(quad, s_left, s_down);
      SNIFF_ERRORS("after ViewportClip");
    }

    if(VP_info.doit == 1){
      ViewportInfo(quad, s_left, s_down);
      SNIFF_ERRORS("after ViewportInfo");
    }

    if(VP_timebar.doit == 1){
      ViewportTimebar(quad, s_left, s_down);
      SNIFF_ERRORS("after ViewportTimebar");
    }

    if(VP_vcolorbar.doit == 1){
      ViewportVerticalColorbar(quad, s_left, s_down);
      SNIFF_ERRORS("after ViewportVerticalColorbar");
    }

    if(VP_title.doit == 1){
      ViewportTitle(quad, s_left, s_down);
      SNIFF_ERRORS("after ViewportTitle");
    }

    if(histogram_draw!=NULL){
      ViewportHistogram(quad, s_left, s_down);
      SNIFF_ERRORS("after ViewportHistogram");
    }

    ViewportScene(quad, view_mode, s_left, s_down, screen);
    if(update_reshape>0){
      update_reshape--;
      GLUTPOSTREDISPLAY;
    }
    SNIFF_ERRORS("after ViewportScene");
  }

  /* ++++++++++++++++++++++++ draw colorbar path using rgb as physical coordinates +++++++++++++++++++++++++ */

  if(viscolorbarpath == 1){
    if(colorbar_hidescene == 1)UNCLIP;
    if(mode==SELECTOBJECT){
      DrawSelectColorbar();
      SNIFF_ERRORS("after DrawSelectColorbars");
    }
    else{
      DrawColorbarPath();
      SNIFF_ERRORS("after DrawColorbarPath");
    }
  }
  if(viscolorbarpath==0||colorbar_hidescene==0)ShowScene2(mode);

/* ++++++++++++++++++++++++ render scene +++++++++++++++++++++++++ */
// if rendering is not working remove following comment
// then determine where Render should have been called
//  Render(view_mode);

  SNIFF_ERRORS("end of ShowScene");
  UNLOCK_IBLANK
}
