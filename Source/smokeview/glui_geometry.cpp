#define CPP
#include "options.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include GLUT_H

#include "smokeviewvars.h"

#define XMIN_SPIN 20
#define YMIN_SPIN 21
#define ZMIN_SPIN 22
#define XMAX_SPIN 23
#define YMAX_SPIN 24
#define ZMAX_SPIN 25
#define UPDATE_LIST 31
#define RADIO_WALL 32
#define SAVE_SETTINGS 33
#define VISAXISLABELS 34
#define GEOM_MAX_ANGLE 36
#define GEOM_OUTLINE_IOFFSET 37
#define GEOM_IVECFACTOR 38
#define SHOW_TEXTURE_2D_IMAGE 39
#define SHOW_TEXTURE_1D_IMAGE 40
#define TERRAIN_ZMIN 41
#define TERRAIN_ZMAX 42
#define RESET_ZBOUNDS 43
#define TERRAIN_ZLEVEL 44
#define SHOW_ZLEVEL 45
#define GEOM_VERT_EXAG 46
#define RESET_GEOM_OFFSET 47
#define UPDATE_GEOM 48

GLUI_Checkbox *CHECKBOX_show_zlevel = NULL;
GLUI_Checkbox *CHECKBOX_surface_solid=NULL, *CHECKBOX_surface_outline=NULL;
GLUI_Checkbox *CHECKBOX_geom_force_transparent = NULL;
GLUI_Checkbox *CHECKBOX_interior_solid=NULL, *CHECKBOX_interior_outline=NULL;
GLUI_Checkbox *CHECKBOX_geomtest=NULL, *CHECKBOX_triangletest=NULL;
GLUI_Checkbox *CHECKBOX_show_geom_normal = NULL;
GLUI_Checkbox *CHECKBOX_smooth_geom_normal = NULL;
GLUI_Checkbox *CHECKBOX_faces_interior=NULL;
GLUI_Checkbox *CHECKBOX_faces_exterior=NULL;
GLUI_Checkbox *CHECKBOX_volumes_interior=NULL;
GLUI_Checkbox *CHECKBOX_volumes_exterior=NULL;
GLUI_Checkbox *CHECKBOX_show_texture_1dimage = NULL;
GLUI_Checkbox *CHECKBOX_show_texture_2dimage = NULL;

GLUI_Checkbox *CHECKBOX_highlight_edge0=NULL;
GLUI_Checkbox *CHECKBOX_highlight_edge1=NULL;
GLUI_Checkbox *CHECKBOX_highlight_edge2=NULL;
GLUI_Checkbox *CHECKBOX_highlight_edgeother=NULL;
GLUI_Checkbox *CHECKBOX_highlight_vertexdup = NULL;

GLUI_Rollout *ROLLOUT_geomtest=NULL;
GLUI_Rollout *ROLLOUT_geomtest2 = NULL;

GLUI_Panel *PANEL_geom_transparency = NULL;
GLUI_Panel *PANEL_normals = NULL;

GLUI_Spinner *SPINNER_geom_transparency=NULL;
GLUI_Spinner *SPINNER_geom_max_angle = NULL;
GLUI_Spinner *SPINNER_geom_outline_ioffset=NULL;
GLUI_Spinner *SPINNER_geom_ivecfactor = NULL;
GLUI_Spinner *SPINNER_geom_vert_exag=NULL;
GLUI_Spinner *SPINNER_geom_zmin = NULL, *SPINNER_geom_zmax = NULL, *SPINNER_geom_zlevel=NULL;
GLUI_Spinner *SPINNER_geom_delx = NULL;
GLUI_Spinner *SPINNER_geom_dely = NULL;
GLUI_Spinner *SPINNER_geom_delz = NULL;


GLUI_Checkbox *CHECKBOX_visaxislabels;

#define VOL_SHOWHIDE 3


GLUI *glui_geometry=NULL;

GLUI_Button *BUTTON_blockage_1=NULL;
GLUI_Button *BUTTON_reset_zbounds = NULL;
GLUI_Button *BUTTON_reset_offset = NULL;

GLUI_Checkbox *CHECKBOX_blockage=NULL;

GLUI_EditText *EDIT_xmin=NULL, *EDIT_ymin=NULL, *EDIT_zmin=NULL;
GLUI_EditText *EDIT_xmax=NULL, *EDIT_ymax=NULL, *EDIT_zmax=NULL;

GLUI_Listbox *LIST_surface[7]={NULL,NULL,NULL,NULL,NULL,NULL,NULL};

GLUI_Panel *PANEL_obj_select=NULL,*PANEL_faces=NULL,*PANEL_triangles=NULL,*PANEL_volumes=NULL,*PANEL_geom_showhide;
GLUI_Panel *PANEL_obj_stretch2=NULL,*PANEL_obj_stretch3=NULL, *PANEL_obj_stretch4=NULL;
GLUI_Panel *PANEL_geomedgecheck=NULL;
GLUI_Panel *PANEL_group1=NULL;
GLUI_Panel *PANEL_geom_offset=NULL;

GLUI_Rollout *ROLLOUT_geomcheck=NULL;
GLUI_Rollout *ROLLOUT_structured=NULL;
GLUI_Rollout *ROLLOUT_unstructured=NULL;

GLUI_Spinner *SPINNER_face_factor=NULL;

GLUI_StaticText *STATIC_blockage_index=NULL;
GLUI_StaticText *STATIC_mesh_index=NULL;
GLUI_StaticText *STATIC_label=NULL;

char a_updatelabel[1000];
char *updatelabel=NULL;

/* ------------------ UpdateWhereFaceVolumes ------------------------ */

extern "C" void UpdateWhereFaceVolumes(void){
  if(CHECKBOX_faces_interior != NULL)CHECKBOX_faces_interior->set_int_val(show_faces_interior);
  if(CHECKBOX_faces_exterior != NULL)CHECKBOX_faces_exterior->set_int_val(show_faces_exterior);
  if(CHECKBOX_volumes_interior != NULL)CHECKBOX_volumes_interior->set_int_val(show_volumes_interior);
  if(CHECKBOX_volumes_exterior != NULL)CHECKBOX_volumes_exterior->set_int_val(show_volumes_exterior);
}

/* ------------------ UpdateVisAxisLabels ------------------------ */

extern "C" void UpdateVisAxisLabels(void){
  if(CHECKBOX_visaxislabels!=NULL)CHECKBOX_visaxislabels->set_int_val(visaxislabels);
}

/* ------------------ UpdateGeometryControls ------------------------ */

extern "C" void UpdateGeometryControls(void){
  if(CHECKBOX_surface_solid!=NULL)CHECKBOX_surface_solid->set_int_val(show_faces_shaded);
  if(CHECKBOX_surface_outline!=NULL)CHECKBOX_surface_outline->set_int_val(show_faces_outline);
  if(CHECKBOX_interior_solid!=NULL)CHECKBOX_interior_solid->set_int_val(show_volumes_solid);
  if(CHECKBOX_interior_outline!=NULL)CHECKBOX_interior_outline->set_int_val(show_volumes_outline);

  if(CHECKBOX_show_geom_normal != NULL)CHECKBOX_show_geom_normal->set_int_val(show_geom_normal);
  if(CHECKBOX_smooth_geom_normal != NULL)CHECKBOX_smooth_geom_normal->set_int_val(smooth_geom_normal);
}

/* ------------------ GetGeomDialogState ------------------------ */

extern "C" void GetGeomDialogState(void){
  if(ROLLOUT_structured!=NULL){
    if(ROLLOUT_structured->is_open){
      structured_isopen=1;
    }
    else{
      structured_isopen=0;
    }
  }
  if(ROLLOUT_unstructured!=NULL){
    if(ROLLOUT_unstructured->is_open){
      unstructured_isopen=1;
    }
    else{
      unstructured_isopen=0;
    }
  }
}

/* ------------------ HaveTexture ------------------------ */

int HaveTexture(void){
  int i;

  for(i = 0; i < ntextures; i++){
    texturedata *texti;

    texti = textureinfo + i;
    if(texti->loaded == 1 && texti->used == 1)return 1;
  }
  return 0;
}

/* ------------------ GetTextureShow ------------------------ */

int GetTextureShow(void){
  int i;

  for(i = 0; i<ntextures; i++){
    texturedata *texti;

    texti = textureinfo+i;
    if(texti->loaded==1&&texti->used==1){
      if(texti->display == 1)return 1;
    }
  }
  return 0;
}

/* ------------------ BlockeditDlgCB ------------------------ */

void BlockeditDlgCB(int var){
  switch(var){
  case SAVE_SETTINGS:
    updatemenu = 1;
    WriteIni(LOCAL_INI, NULL);
    break;
  case CLOSE_WINDOW:
    DialogMenu(DIALOG_GEOMETRY);
    break;
  default:
    ASSERT(FFALSE);
    break;
  }

}

/* ------------------ GluiGeometrySetup ------------------------ */

extern "C" void GluiGeometrySetup(int main_window){
  int ibar,jbar,kbar;
  float *xplt_orig, *yplt_orig, *zplt_orig;
  surfdata *surfi;
  char *surfacelabel;
  int i;

  ibar=current_mesh->ibar;
  jbar=current_mesh->jbar;
  kbar=current_mesh->kbar;
  xplt_orig=current_mesh->xplt_orig;
  yplt_orig=current_mesh->yplt_orig;
  zplt_orig=current_mesh->zplt_orig;

  update_glui_geometry=0;
  if(glui_geometry!=NULL){
    glui_geometry->close();
    glui_geometry=NULL;
  }
  glui_geometry = GLUI_Master.create_glui("Geometry",0,0,0);
  if(showedit_dialog==0)glui_geometry->hide();

  ROLLOUT_structured = glui_geometry->add_rollout("Obstacles",false);
  INSERT_ROLLOUT(ROLLOUT_structured, glui_geometry);
  if(structured_isopen==1)ROLLOUT_structured->open();
  PANEL_obj_select = glui_geometry->add_panel_to_panel(ROLLOUT_structured,"SURFs");

  PANEL_faces=glui_geometry->add_panel_to_panel(PANEL_obj_select,"",GLUI_PANEL_NONE);

  glui_geometry->add_column_to_panel(PANEL_faces,false);

  if(nsurfinfo>0){
    glui_geometry->add_statictext_to_panel(PANEL_faces,"");

    LIST_surface[DOWN_X] = glui_geometry->add_listbox_to_panel(PANEL_faces,_("Left"),surface_indices+DOWN_X,UPDATE_LIST,ObjectCB);
    LIST_surface[DOWN_X]->set_w(260);
    for(i=0;i<nsurfinfo;i++){
      surfi = surfinfo + sorted_surfidlist[i];
      if(surfi->used_by_obst!=1)continue;
      if(surfi->obst_surface==0)continue;
      surfacelabel = surfi->surfacelabel;
      LIST_surface[DOWN_X]->add_item(i,surfacelabel);
    }

    LIST_surface[UP_X] = glui_geometry->add_listbox_to_panel(PANEL_faces,_("Right"),surface_indices+UP_X,UPDATE_LIST,ObjectCB);
    LIST_surface[UP_X]->set_w(260);
    for(i=0;i<nsurfinfo;i++){
      surfi = surfinfo + sorted_surfidlist[i];
      if(surfi->used_by_obst!=1)continue;
      if(surfi->obst_surface==0)continue;
      surfacelabel = surfi->surfacelabel;
      LIST_surface[UP_X]->add_item(i,surfacelabel);
    }

    LIST_surface[DOWN_Y] = glui_geometry->add_listbox_to_panel(PANEL_faces,_("Front"),surface_indices+DOWN_Y,UPDATE_LIST,ObjectCB);
    LIST_surface[DOWN_Y]->set_w(260);
    for(i=0;i<nsurfinfo;i++){
      surfi = surfinfo + sorted_surfidlist[i];
      if(surfi->used_by_obst!=1)continue;
      if(surfi->obst_surface==0)continue;
      surfacelabel = surfi->surfacelabel;
      LIST_surface[DOWN_Y]->add_item(i,surfacelabel);
    }

    LIST_surface[UP_Y] = glui_geometry->add_listbox_to_panel(PANEL_faces,_("Back"),surface_indices+UP_Y,UPDATE_LIST,ObjectCB);
    LIST_surface[UP_Y]->set_w(260);
    for(i=0;i<nsurfinfo;i++){
      surfi = surfinfo + sorted_surfidlist[i];
      if(surfi->used_by_obst!=1)continue;
      if(surfi->obst_surface==0)continue;
      surfacelabel = surfi->surfacelabel;
      LIST_surface[UP_Y]->add_item(i,surfacelabel);
    }

    LIST_surface[DOWN_Z] = glui_geometry->add_listbox_to_panel(PANEL_faces,_("Down"),surface_indices+DOWN_Z,UPDATE_LIST,ObjectCB);
    LIST_surface[DOWN_Z]->set_w(260);
    for(i=0;i<nsurfinfo;i++){
      surfi = surfinfo + sorted_surfidlist[i];
      if(surfi->used_by_obst!=1)continue;
      if(surfi->obst_surface==0)continue;
      surfacelabel = surfi->surfacelabel;
      LIST_surface[DOWN_Z]->add_item(i,surfacelabel);
    }

    LIST_surface[UP_Z] = glui_geometry->add_listbox_to_panel(PANEL_faces,_("Up"),surface_indices+UP_Z,UPDATE_LIST,ObjectCB);
    LIST_surface[UP_Z]->set_w(260);
    for(i=0;i<nsurfinfo;i++){
      surfi = surfinfo + sorted_surfidlist[i];
      if(surfi->used_by_obst!=1)continue;
      if(surfi->obst_surface==0)continue;
      surfacelabel = surfi->surfacelabel;
      LIST_surface[UP_Z]->add_item(i,surfacelabel);
    }

    ObjectCB(RADIO_WALL);
    for(i=0;i<6;i++){
      LIST_surface[i]->disable();
    }
  }

  glui_geometry->add_column_to_panel(ROLLOUT_structured,false);

  PANEL_obj_stretch4=glui_geometry->add_panel_to_panel(ROLLOUT_structured,"",GLUI_PANEL_NONE);

  {
    char meshlabel[255];

    strcpy(meshlabel,_("Mesh:"));
    strcat(meshlabel,meshinfo->label);
    STATIC_mesh_index=glui_geometry->add_statictext_to_panel(PANEL_obj_stretch4,meshlabel);

  }
  STATIC_blockage_index=glui_geometry->add_statictext_to_panel(PANEL_obj_stretch4,"&OBST number: ");
  STATIC_label=glui_geometry->add_statictext_to_panel(PANEL_obj_stretch4,"&OBST label:");

  PANEL_obj_stretch2 = glui_geometry->add_panel_to_panel(ROLLOUT_structured,"Coordinates");

  CHECKBOX_blockage=glui_geometry->add_checkbox_to_panel(PANEL_obj_stretch2,_("Dimensions snapped to grid"),&blockage_snapped,
    BLOCKAGE_AS_INPUT,ObjectCB);
  CHECKBOX_visaxislabels=glui_geometry->add_checkbox_to_panel(PANEL_obj_stretch2,_("Show axis labels"),&visaxislabels,VISAXISLABELS,ObjectCB);
  PANEL_obj_stretch3 = glui_geometry->add_panel_to_panel(PANEL_obj_stretch2,"",GLUI_PANEL_NONE);
  EDIT_xmin=glui_geometry->add_edittext_to_panel(PANEL_obj_stretch3,"x",GLUI_EDITTEXT_FLOAT,&glui_block_xmin,XMIN_SPIN,ObjectCB);
  EDIT_ymin=glui_geometry->add_edittext_to_panel(PANEL_obj_stretch3,"y",GLUI_EDITTEXT_FLOAT,&glui_block_ymin,YMIN_SPIN,ObjectCB);
  EDIT_zmin=glui_geometry->add_edittext_to_panel(PANEL_obj_stretch3,"z",GLUI_EDITTEXT_FLOAT,&glui_block_zmin,ZMIN_SPIN,ObjectCB);

  glui_geometry->add_column_to_panel(PANEL_obj_stretch3,false);
  EDIT_xmax=glui_geometry->add_edittext_to_panel(PANEL_obj_stretch3,"",GLUI_EDITTEXT_FLOAT,&glui_block_xmax,XMAX_SPIN,ObjectCB);
  EDIT_ymax=glui_geometry->add_edittext_to_panel(PANEL_obj_stretch3,"",GLUI_EDITTEXT_FLOAT,&glui_block_ymax,YMAX_SPIN,ObjectCB);
  EDIT_zmax=glui_geometry->add_edittext_to_panel(PANEL_obj_stretch3,"",GLUI_EDITTEXT_FLOAT,&glui_block_zmax,ZMAX_SPIN,ObjectCB);

  EDIT_xmin->disable();
  EDIT_ymin->disable();
  EDIT_zmin->disable();

  EDIT_xmax->disable();
  EDIT_ymax->disable();
  EDIT_zmax->disable();
  ObjectCB(BLOCKAGE_AS_INPUT);

  EDIT_xmin->set_float_limits(xplt_orig[0],xplt_orig[ibar],GLUI_LIMIT_CLAMP);
  EDIT_xmax->set_float_limits(xplt_orig[0],xplt_orig[ibar],GLUI_LIMIT_CLAMP);
  EDIT_ymin->set_float_limits(yplt_orig[0],yplt_orig[jbar],GLUI_LIMIT_CLAMP);
  EDIT_ymax->set_float_limits(yplt_orig[0],yplt_orig[jbar],GLUI_LIMIT_CLAMP);
  EDIT_zmin->set_float_limits(zplt_orig[0],zplt_orig[kbar],GLUI_LIMIT_CLAMP);
  EDIT_zmax->set_float_limits(zplt_orig[0],zplt_orig[kbar],GLUI_LIMIT_CLAMP);

  if(ngeominfo > 0){
    ROLLOUT_unstructured = glui_geometry->add_rollout("Immersed", false);
    INSERT_ROLLOUT(ROLLOUT_unstructured, glui_geometry);
    if(unstructured_isopen == 1)ROLLOUT_unstructured->open();

    for(i = 0;i < nmeshes;i++){
      meshdata *meshi;

      meshi = meshinfo + i;
      if(meshi->ncutcells > 0){
        glui_geometry->add_checkbox_to_panel(ROLLOUT_unstructured, _("Show cutcells"), &show_cutcells);
        break;
      }
    }

    PANEL_geom_showhide = glui_geometry->add_panel_to_panel(ROLLOUT_unstructured, "", GLUI_PANEL_NONE);
    PANEL_group1 = glui_geometry->add_panel_to_panel(PANEL_geom_showhide, "", GLUI_PANEL_NONE);
    PANEL_triangles = glui_geometry->add_panel_to_panel(PANEL_group1, "faces");
    CHECKBOX_faces_interior = glui_geometry->add_checkbox_to_panel(PANEL_triangles, "interior", &show_faces_interior);
    CHECKBOX_faces_exterior = glui_geometry->add_checkbox_to_panel(PANEL_triangles, "exterior", &show_faces_exterior);
    CHECKBOX_surface_solid = glui_geometry->add_checkbox_to_panel(PANEL_triangles, "solid", &show_faces_shaded, VOL_SHOWHIDE, VolumeCB);
    CHECKBOX_surface_outline = glui_geometry->add_checkbox_to_panel(PANEL_triangles, "outline", &show_faces_outline, VOL_SHOWHIDE, VolumeCB);
    glui_geometry->add_spinner_to_panel(PANEL_triangles, "line width", GLUI_SPINNER_FLOAT, &geom_linewidth);
    glui_geometry->add_spinner_to_panel(PANEL_triangles, "point size", GLUI_SPINNER_FLOAT, &geom_pointsize);
    PANEL_geom_transparency = glui_geometry->add_panel_to_panel(PANEL_triangles, "transparency");
    CHECKBOX_geom_force_transparent = glui_geometry->add_checkbox_to_panel(PANEL_geom_transparency, "force", &geom_force_transparent);
    SPINNER_geom_transparency = glui_geometry->add_spinner_to_panel(PANEL_geom_transparency, "level", GLUI_SPINNER_FLOAT, &geom_transparency);
    SPINNER_geom_transparency->set_float_limits(0.0, 1.0);

    glui_geometry->add_column_to_panel(PANEL_group1, false);


    PANEL_volumes = glui_geometry->add_panel_to_panel(PANEL_group1, "volumes");
    CHECKBOX_volumes_interior = glui_geometry->add_checkbox_to_panel(PANEL_volumes, "interior", &show_volumes_interior);
    CHECKBOX_volumes_exterior = glui_geometry->add_checkbox_to_panel(PANEL_volumes, "exterior", &show_volumes_exterior);
    CHECKBOX_interior_solid = glui_geometry->add_checkbox_to_panel(PANEL_volumes, "solid", &show_volumes_solid, VOL_SHOWHIDE, VolumeCB);
    CHECKBOX_interior_outline = glui_geometry->add_checkbox_to_panel(PANEL_volumes, "outline", &show_volumes_outline, VOL_SHOWHIDE, VolumeCB);

    PANEL_normals = glui_geometry->add_panel_to_panel(PANEL_geom_showhide, "normals");
    CHECKBOX_show_geom_normal = glui_geometry->add_checkbox_to_panel(PANEL_normals, "show", &show_geom_normal);
    CHECKBOX_smooth_geom_normal = glui_geometry->add_checkbox_to_panel(PANEL_normals, "smooth", &smooth_geom_normal);
    SPINNER_geom_ivecfactor = glui_geometry->add_spinner_to_panel(PANEL_normals, "length", GLUI_SPINNER_INT, &geom_ivecfactor, GEOM_IVECFACTOR, VolumeCB);
    SPINNER_geom_ivecfactor->set_int_limits(0, 200);

    ROLLOUT_geomtest2 = glui_geometry->add_rollout_to_panel(ROLLOUT_unstructured, "parameters", false);
    INSERT_ROLLOUT(ROLLOUT_geomtest2, glui_geometry);
    SPINNER_geom_vert_exag = glui_geometry->add_spinner_to_panel(ROLLOUT_geomtest2, "vertical exaggeration", GLUI_SPINNER_FLOAT, &geom_vert_exag, GEOM_VERT_EXAG, VolumeCB);
    SPINNER_geom_vert_exag->set_float_limits(0.1, 10.0);
    CHECKBOX_show_texture_1dimage = glui_geometry->add_checkbox_to_panel(ROLLOUT_geomtest2, "show elevation color", &show_texture_1dimage, SHOW_TEXTURE_1D_IMAGE, VolumeCB);

    GetGeomZBounds(&terrain_zmin, &terrain_zmax);
    SPINNER_geom_zmin = glui_geometry->add_spinner_to_panel(ROLLOUT_geomtest2, "zmin", GLUI_SPINNER_FLOAT, &terrain_zmin, TERRAIN_ZMIN, VolumeCB);
    SPINNER_geom_zmin->set_float_limits(zbar0ORIG, zbarORIG);

    SPINNER_geom_zmax = glui_geometry->add_spinner_to_panel(ROLLOUT_geomtest2, "zmax", GLUI_SPINNER_FLOAT, &terrain_zmax, TERRAIN_ZMAX, VolumeCB);
    SPINNER_geom_zmax->set_float_limits(zbar0ORIG, zbarORIG);

    terrain_zlevel = (terrain_zmin + terrain_zmax) / 2.0;
    CHECKBOX_show_zlevel = glui_geometry->add_checkbox_to_panel(ROLLOUT_geomtest2, "show zlevel", &show_zlevel, SHOW_ZLEVEL, VolumeCB);
    SPINNER_geom_zlevel = glui_geometry->add_spinner_to_panel(ROLLOUT_geomtest2, "zlevel", GLUI_SPINNER_FLOAT, &terrain_zlevel, TERRAIN_ZLEVEL, VolumeCB);
    SPINNER_geom_zlevel->set_float_limits(zbar0ORIG, zbarORIG);

    VolumeCB(GEOM_VERT_EXAG);
    BUTTON_reset_zbounds = glui_geometry->add_button_to_panel(ROLLOUT_geomtest2, _("Reset zmin/zmax"), RESET_ZBOUNDS, VolumeCB);

    if(HaveTexture() == 1){
      show_texture_2dimage = GetTextureShow();
      CHECKBOX_show_texture_2dimage = glui_geometry->add_checkbox_to_panel(ROLLOUT_geomtest2, "image", &show_texture_2dimage, SHOW_TEXTURE_2D_IMAGE, VolumeCB);
      VolumeCB(SHOW_TEXTURE_2D_IMAGE);
    }


    glui_geometry->add_checkbox_to_panel(ROLLOUT_geomtest2, "use max angle", &use_max_angle, GEOM_MAX_ANGLE, VolumeCB);
    SPINNER_geom_max_angle = glui_geometry->add_spinner_to_panel(ROLLOUT_geomtest2, "max angle", GLUI_SPINNER_FLOAT, &geom_max_angle, GEOM_MAX_ANGLE, VolumeCB);
    SPINNER_geom_max_angle->set_float_limits(0.0, 180.0);
    SPINNER_geom_outline_ioffset = glui_geometry->add_spinner_to_panel(ROLLOUT_geomtest2, "outline offset", GLUI_SPINNER_INT, &geom_outline_ioffset, GEOM_OUTLINE_IOFFSET, VolumeCB);
    SPINNER_geom_outline_ioffset->set_int_limits(0, 200);
    SPINNER_face_factor = glui_geometry->add_spinner_to_panel(ROLLOUT_geomtest2, "face factor", GLUI_SPINNER_FLOAT, &face_factor);
    SPINNER_face_factor->set_float_limits(0.0, 0.5);

    PANEL_geom_offset = glui_geometry->add_panel_to_panel(PANEL_group1, "offset");
    SPINNER_geom_delx = glui_geometry->add_spinner_to_panel(PANEL_geom_offset, "dx", GLUI_SPINNER_FLOAT, &geom_delx);
    SPINNER_geom_dely = glui_geometry->add_spinner_to_panel(PANEL_geom_offset, "dy", GLUI_SPINNER_FLOAT, &geom_dely);
    SPINNER_geom_delz = glui_geometry->add_spinner_to_panel(PANEL_geom_offset, "dz", GLUI_SPINNER_FLOAT, &geom_delz);
    glui_geometry->add_checkbox_to_panel(PANEL_geom_offset, "show geometry and boundary files", &glui_show_geom_bndf, UPDATE_GEOM, VolumeCB);
    BUTTON_reset_offset = glui_geometry->add_button_to_panel(PANEL_geom_offset, _("Reset"), RESET_GEOM_OFFSET, VolumeCB);

    ROLLOUT_geomcheck = glui_geometry->add_rollout_to_panel(ROLLOUT_unstructured, "checks", false);
    INSERT_ROLLOUT(ROLLOUT_geomcheck, glui_geometry);
    PANEL_geomedgecheck = glui_geometry->add_panel_to_panel(ROLLOUT_geomcheck, "edges - connected triangles");
    CHECKBOX_highlight_edge0 = glui_geometry->add_checkbox_to_panel(PANEL_geomedgecheck, "0", &highlight_edge0);
    CHECKBOX_highlight_edge1 = glui_geometry->add_checkbox_to_panel(PANEL_geomedgecheck, "1", &highlight_edge1);
    CHECKBOX_highlight_edge2 = glui_geometry->add_checkbox_to_panel(PANEL_geomedgecheck, "2", &highlight_edge2);
    CHECKBOX_highlight_edgeother = glui_geometry->add_checkbox_to_panel(PANEL_geomedgecheck, "3 or more", &highlight_edgeother);

    CHECKBOX_highlight_vertexdup = glui_geometry->add_checkbox_to_panel(ROLLOUT_geomcheck, "duplicate vertices", &highlight_vertexdup);
  }

  glui_geometry->add_separator();
  glui_geometry->add_button(_("Save settings"),SAVE_SETTINGS, BlockeditDlgCB);
  BUTTON_blockage_1=glui_geometry->add_button(_("Close"),CLOSE_WINDOW, BlockeditDlgCB);

  glui_geometry->set_main_gfx_window( main_window );
}

/* ------------------ VolumeCB ------------------------ */

extern "C" void VolumeCB(int var){
  int i;
  switch(var){
  case UPDATE_GEOM:
    LOCK_TRIANGLES;
    show_geom_bndf = glui_show_geom_bndf;
    UNLOCK_TRIANGLES;
    update_times = 1;
    break;
  case RESET_GEOM_OFFSET:
    geom_delx = 0.0;
    geom_dely = 0.0;
    geom_delz = 0.0;
    SPINNER_geom_delx->set_float_val(geom_delx);
    SPINNER_geom_dely->set_float_val(geom_dely);
    SPINNER_geom_delz->set_float_val(geom_delz);
    break;
  case GEOM_VERT_EXAG:
    UpdateGeomNormals();
    break;
  case SHOW_ZLEVEL:
  case TERRAIN_ZLEVEL:
    UpdateChopColors();
  break;
  case RESET_ZBOUNDS:
    GetGeomZBounds(&terrain_zmin, &terrain_zmax);
    SPINNER_geom_zmin->set_float_val(terrain_zmin);
    SPINNER_geom_zmax->set_float_val(terrain_zmax);
    SPINNER_geom_zlevel->set_float_limits(terrain_zmin, terrain_zmax);
  case TERRAIN_ZMIN:
  case TERRAIN_ZMAX:
    if(ABS(terrain_zmin - terrain_zmax) < 0.01){
      terrain_zmax = terrain_zmin + .01;
      SPINNER_geom_zmax->set_float_val(terrain_zmax);
    }
    SPINNER_geom_zlevel->set_float_limits(terrain_zmin, terrain_zmax);
    UpdateChopColors();
  case SHOW_TEXTURE_1D_IMAGE:
    if(show_texture_1dimage == 1 && show_texture_2dimage == 1){
      show_texture_2dimage=0;
      VolumeCB(SHOW_TEXTURE_2D_IMAGE);
      if(CHECKBOX_show_texture_2dimage!=NULL&&CHECKBOX_show_texture_2dimage->get_int_val() == 1)CHECKBOX_show_texture_2dimage->set_int_val(0);
    }
    break;
  case SHOW_TEXTURE_2D_IMAGE:
    if(show_texture_1dimage==1&&show_texture_2dimage==1){
      show_texture_1dimage=0;
      if(CHECKBOX_show_texture_1dimage->get_int_val() == 1)CHECKBOX_show_texture_1dimage->set_int_val(0);
    }
    for(i = 0; i<ntextures; i++){
      texturedata *texti;

      texti = textureinfo+i;
      if(texti->loaded==1&&texti->used==1){
        texti->display = 1 - show_texture_2dimage;
        TextureShowMenu(i);
        if(CHECKBOX_show_texture_2dimage != NULL){
          if(texti->display == 1 && CHECKBOX_show_texture_2dimage->get_int_val() == 0)CHECKBOX_show_texture_2dimage->set_int_val(1);
          if(texti->display == 0 && CHECKBOX_show_texture_2dimage->get_int_val() == 1)CHECKBOX_show_texture_2dimage->set_int_val(0);
        }
        break;
      }
    }
    break;
  case GEOM_IVECFACTOR:
    geom_vecfactor = (float)geom_ivecfactor/1000.0;
    break;
  case GEOM_MAX_ANGLE:
    cos_geom_max_angle=cos(DEG2RAD*geom_max_angle);
    UpdateTriangles(GEOM_STATIC,GEOM_UPDATE_NORMALS);
    break;
  case GEOM_OUTLINE_IOFFSET:
    geom_outline_offset = (float)geom_outline_ioffset/1000.0;
    break;
    // update_volbox_controls=1;
    // face_vis
    // face_vis_old
  case VOL_SHOWHIDE:
    updatemenu=1;
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ HideGluiGeometry ------------------------ */

extern "C" void HideGluiGeometry(void){
  blockageSelect=0;
  CloseRollouts(glui_geometry);
  showedit_dialog=0;
  editwindow_status=CLOSE_WINDOW;
}

/* ------------------ ShowGluiGeometry ------------------------ */

extern "C" void ShowGluiGeometry(void){
  showedit_dialog=1;
  blockageSelect=1;
  UpdateBlockVals(NOT_SELECT_BLOCKS);
  if(glui_geometry!=NULL)glui_geometry->show();
}

/* ------------------ UpdateBlockVals ------------------------ */

extern "C" void UpdateBlockVals(int flag){
  float xmin, xmax, ymin, ymax, zmin, zmax;
  int imin, jmin, kmin;
  int i;
  int temp;
  float *xplt_orig, *yplt_orig, *zplt_orig;
  int ibar, jbar, kbar;

  GetBlockVals(&xmin,&xmax,&ymin,&ymax,&zmin,&zmax,&imin,&jmin,&kmin);

  xplt_orig = current_mesh->xplt_orig;
  yplt_orig = current_mesh->yplt_orig;
  zplt_orig = current_mesh->zplt_orig;
  ibar = current_mesh->ibar;
  jbar = current_mesh->jbar;
  kbar = current_mesh->kbar;

  EDIT_xmin->set_float_limits(xplt_orig[0],xplt_orig[ibar],GLUI_LIMIT_CLAMP);
  EDIT_xmax->set_float_limits(xplt_orig[0],xplt_orig[ibar],GLUI_LIMIT_CLAMP);
  EDIT_ymin->set_float_limits(yplt_orig[0],yplt_orig[jbar],GLUI_LIMIT_CLAMP);
  EDIT_ymax->set_float_limits(yplt_orig[0],yplt_orig[jbar],GLUI_LIMIT_CLAMP);
  EDIT_zmin->set_float_limits(zplt_orig[0],zplt_orig[kbar],GLUI_LIMIT_CLAMP);
  EDIT_zmax->set_float_limits(zplt_orig[0],zplt_orig[kbar],GLUI_LIMIT_CLAMP);

  EDIT_xmin->set_float_val(xmin);
  EDIT_xmax->set_float_val(xmax);
  EDIT_ymin->set_float_val(ymin);
  EDIT_ymax->set_float_val(ymax);
  EDIT_zmin->set_float_val(zmin);
  EDIT_zmax->set_float_val(zmax);
  if(bchighlight!=NULL&&nsurfinfo>0){
    wall_case=bchighlight->walltype;
    ObjectCB(RADIO_WALL);
  }

  if(flag==SELECT_BLOCKS){
    if(bchighlight!=NULL){
      char dialog_label[255];
      meshdata *blockmesh;

      if(nmeshes>1){
        blockmesh = meshinfo + bchighlight->meshindex;
        sprintf(dialog_label,"Mesh label: %s",blockmesh->label);
        STATIC_mesh_index->set_text(dialog_label);
      }
      sprintf(dialog_label,"&OBST index: %i",bchighlight->blockage_id);
      STATIC_blockage_index->set_text(dialog_label);
      strcpy(dialog_label,"&OBST label: ");
      strcat(dialog_label,bchighlight->label);
      STATIC_label->set_text(dialog_label);

      switch(wall_case){
      case WALL_1:
        temp=bchighlight->surf_index[UP_Z];
        for(i=0;i<6;i++){
          bchighlight->surf_index[i]=temp;
        }
        break;
      case WALL_3:
        temp=bchighlight->surf_index[UP_Y];
        bchighlight->surf_index[DOWN_X]=temp;
        bchighlight->surf_index[DOWN_Y]=temp;
        bchighlight->surf_index[UP_X]=temp;
        break;
      case WALL_6:
        break;
      default:
        ASSERT(FFALSE);
        break;
      }

      if(nsurfinfo>0){
        for(i=0;i<6;i++){
          surface_indices[i] = inv_sorted_surfidlist[bchighlight->surf_index[i]];
          surface_indices_bak[i] = inv_sorted_surfidlist[bchighlight->surf_index[i]];
          LIST_surface[i]->set_int_val(surface_indices[i]);
        }
      }
    }
    else{
      if(nsurfinfo>0){
        for(i=0;i<6;i++){
          surface_indices[i]=inv_sorted_surfidlist[0];
          surface_indices_bak[i]=inv_sorted_surfidlist[0];
          LIST_surface[i]->set_int_val(surface_indices[i]);
        }
      }
    }
  }
}

/* ------------------ ObjectCB ------------------------ */

extern "C" void ObjectCB(int var){
  int i,temp;
  switch(var){
    case VISAXISLABELS:
      updatemenu=1;
      break;
    case UPDATE_LIST:
      switch(wall_case){
      case WALL_1:
        temp=surface_indices_bak[UP_Z];
        if(nsurfinfo>0){
          for(i=0;i<6;i++){
            surface_indices[i]=temp;
            LIST_surface[i]->set_int_val(temp);
          }
        }
        break;
      case WALL_3:
        if(nsurfinfo>0){
          for(i=0;i<6;i++){
            temp=surface_indices_bak[i];
            surface_indices[i]=temp;
            LIST_surface[i]->set_int_val(temp);
          }
        }
        break;
      case WALL_6:
        if(nsurfinfo>0){
          for(i=0;i<6;i++){
            temp=surface_indices_bak[i];
            surface_indices[i]=temp;
            LIST_surface[i]->set_int_val(temp);
          }
        }
        break;
      default:
        ASSERT(FFALSE);
        break;
      }

      if(bchighlight!=NULL){
        for(i=0;i<6;i++){
          bchighlight->surf[i]=surfinfo+sorted_surfidlist[surface_indices_bak[i]];
          bchighlight->surf_index[i]=sorted_surfidlist[surface_indices_bak[i]];
        }
        bchighlight->changed_surface=1;
        if(bchighlight->blockage_id>0&&bchighlight->blockage_id<=nchanged_idlist){
          changed_idlist[bchighlight->blockage_id]=1;
        }
        blockages_dirty=1;
        UpdateUseTextures();
        UpdateFaces();
      }
      break;
    case RADIO_WALL:
      if(nsurfinfo==0)break;
      if(bchighlight!=NULL){
        bchighlight->walltype=wall_case;
      }
      switch(wall_case){
      case WALL_6:
        for(i=0;i<6;i++){
          LIST_surface[i]->enable();
        }
        LIST_surface[DOWN_Z]->set_name("z lower face");
        LIST_surface[UP_Z]->set_name("z upper face");
        LIST_surface[DOWN_Y]->set_name("y lower face");
        LIST_surface[UP_Y]->set_name("y upper face");
        LIST_surface[DOWN_X]->set_name("x lower face");
        LIST_surface[UP_X]->set_name("x upper face");
        break;
      case WALL_3:
        for(i=0;i<6;i++){
          LIST_surface[i]->disable();
        }
        LIST_surface[DOWN_Z]->enable();
        LIST_surface[UP_Z]->enable();
        LIST_surface[UP_Y]->enable();

        LIST_surface[DOWN_Z]->set_name("z lower face");
        LIST_surface[UP_Z]->set_name("z upper face");
        LIST_surface[UP_Y]->set_name("side faces");
        LIST_surface[DOWN_Y]->set_name("");
        LIST_surface[DOWN_X]->set_name("");
        LIST_surface[UP_X]->set_name("");

        break;
      case WALL_1:
        for(i=0;i<6;i++){
          LIST_surface[i]->disable();
        }
        LIST_surface[UP_Z]->enable();
        LIST_surface[UP_Z]->set_name("All faces");

        LIST_surface[DOWN_Z]->set_name("");
        LIST_surface[DOWN_Y]->set_name("");
        LIST_surface[UP_Y]->set_name("");
        LIST_surface[DOWN_X]->set_name("");
        LIST_surface[UP_X]->set_name("");
        break;
      default:
        ASSERT(FFALSE);
        break;
      }
      ObjectCB(UPDATE_LIST);
      break;
      case BLOCKAGE_AS_INPUT2:
      case BLOCKAGE_AS_INPUT:
        if(var==BLOCKAGE_AS_INPUT2){
          blockage_snapped=1-blockage_as_input;
          CHECKBOX_blockage->set_int_val(blockage_snapped);
        }
        blockage_as_input=1-blockage_snapped;
        if(blocklocation!=BLOCKlocation_cad){
          if(blockage_as_input==1){
            blocklocation=BLOCKlocation_exact;
          }
          else{
            blocklocation=BLOCKlocation_grid;
          }
        }
        UpdateBlockVals(NOT_SELECT_BLOCKS);
        break;
    default:
      ASSERT(FFALSE);
      break;
  }
}
