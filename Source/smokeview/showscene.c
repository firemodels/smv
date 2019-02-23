#include "options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include GLUT_H

#include "update.h"
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

void ShowScene2(int mode, int view_mode, int quad, GLint s_left, GLint s_down){
  if(rotation_type == EYE_CENTERED&&nskyboxinfo>0)DrawSkybox();
  UpdateLights(light_position0, light_position1);
  if(drawlights==1)DrawLights(light_position0, light_position1);


 // if(render_status==RENDER_ON&&render_mode==RENDER_360){
 //   UpdateLights(light_position0, light_position1);
 // }

  if(mode == DRAWSCENE){
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
    DrawDevices();
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

    if(isZoneFireModel == 0 && visFrame == 1 && highlight_flag == 2){
      CLIP_GEOMETRY;
      DrawOutlines();
      SNIFF_ERRORS("after DrawOutlines");
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
      if(visGrid != noGridnoProbe){
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
      DrawDevices();
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
  DrawBlockages(mode, DRAW_OPAQUE);
  SNIFF_ERRORS("DrawBlockages");

  /* ++++++++++++++++++++++++ draw triangles +++++++++++++++++++++++++ */

  if(ngeominfoptrs>0){
    CLIP_GEOMETRY;
    DrawGeom(DRAW_OPAQUE, GEOM_STATIC);
    DrawGeom(DRAW_OPAQUE, GEOM_DYNAMIC);
    SNIFF_ERRORS("DrawGeom");
  }

  /* ++++++++++++++++++++++++ draw diagnostic geometry +++++++++++++++++++++++++ */

  if(show_geometry_diagnostics == 1){
    CLIP_GEOMETRY;
    DrawGeomDiag();
    SNIFF_ERRORS("DrawGeomDiag");
  }

  /* ++++++++++++++++++++++++ draw shooter points +++++++++++++++++++++++++ */

  if(showshooter != 0 && shooter_active == 1){
    CLIP_VALS;
    DrawShooter();
    SNIFF_ERRORS("DrawShooter");
  }

  /* ++++++++++++++++++++++++ draw terrain +++++++++++++++++++++++++ */

  if(visTerrainType != TERRAIN_HIDDEN&&nterraininfo>0){
    int i;

    //shaded 17 0
    //stepped 18 1
    //line    19 2
    //texture 20 3
    //hidden 20 4

    CLIP_GEOMETRY;
    for(i = 0;i<nterraininfo;i++){
      terraindata *terri;
      int only_geom;

      terri = terraininfo + i;
      if(terri->loaded == 1){
        only_geom = 0;
      }
      else{
        only_geom = 1;
      }
      switch(visTerrainType){
      case TERRAIN_3D:
        DrawTerrain(terri, only_geom);
        break;
      case TERRAIN_2D_STEPPED:
        if(cullfaces == 1)glDisable(GL_CULL_FACE);
        glPushMatrix();
        glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
        glTranslatef(-xbar0, -ybar0, -zbar0);
        DrawContours(&meshinfo[i].terrain_contour);
        glPopMatrix();
        if(cullfaces == 1)glEnable(GL_CULL_FACE);
        break;
      case TERRAIN_2D_LINE:
        glPushMatrix();
        glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
        glTranslatef(-xbar0, -ybar0, -zbar0);
        DrawLineContours(&meshinfo[i].terrain_contour, 1.0);
        glPopMatrix();
        break;
      case TERRAIN_3D_MAP:
        if(terrain_texture != NULL&&terrain_texture->loaded == 1){
          DrawTerrainTexture(terri, only_geom);
        }
        else{
          DrawTerrain(terri, only_geom);
        }
        break;
      default:
        ASSERT(FFALSE);
        break;
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
    DrawSliceFrame();
  }

  /* ++++++++++++++++++++++++ draw boundary files +++++++++++++++++++++++++ */

  if(showpatch == 1){
    CLIP_VALS;
    DrawBoundaryFrame(DRAW_OPAQUE);
  }

  /* ++++++++++++++++++++++++ draw labels +++++++++++++++++++++++++ */

  if(visLabels == 1){
    CLIP_GEOMETRY;
    DrawLabels();
  }

  /* ++++++++++++++++++++++++ draw animated isosurfaces +++++++++++++++++++++++++ */

  if(showiso == 1){
    CLIP_VALS;
    DrawIso(DRAW_OPAQUE);
  }

  /* ++++++++++++++++++++++++ draw zone fire modeling info +++++++++++++++++++++++++ */

  if(nrooms>0){
    CLIP_GEOMETRY;
    DrawRoomGeom();
    SNIFF_ERRORS("after DrawRoomGeom");

    if(showzone == 1){
      CLIP_VALS;
      DrawFireData();
      SNIFF_ERRORS("after DrawRoomData");
      if(ReadZoneFile == 1 && nzvents>0){
        DrawVentData();
        SNIFF_ERRORS("after DrawVentData");
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

  /* ++++++++++++++++++++++++ draw triangles +++++++++++++++++++++++++ */

  if(ngeominfoptrs>0){
    CLIP_GEOMETRY;
    DrawGeom(DRAW_TRANSPARENT, GEOM_STATIC);
    DrawGeom(DRAW_TRANSPARENT, GEOM_DYNAMIC);
  }

  if(showiso == 1){
    CLIP_VALS;
    DrawIso(DRAW_TRANSPARENT);
  }

  /* ++++++++++++++++++++++++ draw transparent faces +++++++++++++++++++++++++ */

  CLIP_GEOMETRY;
  DrawTransparentFaces();

  /* ++++++++++++++++++++++++ draw 3D smoke +++++++++++++++++++++++++ */

  if(show3dsmoke == 1){
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

  if(showvolrender == 1&&show3dsmoke==0){
    CLIP_VALS;
    DrawVolSmokeFrame();
  }

  if(show_light_position_direction == 1)DrawLightDirections();
#ifdef pp_SMOKETEST
  if(smoke_test == 1)DrawSmokeTest();
#endif

  if(active_smokesensors == 1 && show_smokesensors != SMOKESENSORS_HIDDEN){
    CLIP_VALS;
    GetSmokeSensors();
    DrawDevicesVal();
  }

  /* ++++++++++++++++++++++++ draw zone fire modeling info +++++++++++++++++++++++++ */

  if(nrooms>0 && showzone == 1){
    CLIP_VALS;
    DrawRoomData();
    SNIFF_ERRORS("after DrawRoomData");
  }

  /* ++++++++++++++++++++++++ draw boundary files +++++++++++++++++++++++++ */

  if(showpatch == 1){
    CLIP_VALS;
    DrawBoundaryFrame(DRAW_TRANSPARENT);
  }

  /* ++++++++++++++++++++++++ draw slice files +++++++++++++++++++++++++ */

  if((show_node_slices_and_vectors == 1 || show_cell_slices_and_vectors==1) || (showslice == 1 && use_transparency_data == 1)){
    if(nslice_loaded>0||ngeomslice_loaded>0){
      CLIP_VALS;
      DrawSliceFrame();
      SNIFF_ERRORS("after DrawSliceFrame");
    }
  }

  /* ++++++++++++++++++++++++ draw transparent blockages +++++++++++++++++++++++++ */

  //  DrawDemo(20,20);
  //  DrawDemo2(1);
  CLIP_GEOMETRY;
  DrawBlockages(mode, DRAW_TRANSPARENT);
  SNIFF_ERRORS("after drawBlokcages");

  /* ++++++++++++++++++++++++ draw vector slice files +++++++++++++++++++++++++ */

  if(showvslice == 1){
    CLIP_VALS;
    DrawVSliceFrame();
  }
  SNIFF_ERRORS("after drawvslice");

  /* ++++++++++++++++++++++++ draw plot3d files +++++++++++++++++++++++++ */

  if(showplot3d == 1){
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

    ViewportScene(quad, view_mode, s_left, s_down, screen);
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
  if(viscolorbarpath==0||colorbar_hidescene==0)ShowScene2(mode, view_mode, quad, s_left, s_down);

/* ++++++++++++++++++++++++ render scene +++++++++++++++++++++++++ */
// if rendering is not working remove following comment
// then determine where Render should have been called
//  Render(view_mode);

  SNIFF_ERRORS("end of ShowScene");
  UNLOCK_IBLANK
}
