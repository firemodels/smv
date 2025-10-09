#ifndef READOBJECT_H_DEFINED
#define READOBJECT_H_DEFINED
#include "options_common.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_util.h"
#include "string_util.h"

#include "shared_structures.h"

#if defined(_WIN32)
#include <windows.h>
#endif
#include GLU_H
#include GL_H

#define VEL_INVALID 0
#define VEL_CARTESIAN 1
#define VEL_POLAR 2

#define VECTOR_LINE 0
#define VECTOR_ARROW 1
#define VECTOR_OBJECT 2
#define VECTOR_PROFILE 3

#define CIRCLE_SEGS 12

#define SV_TRANSLATE 100
#define SV_ROTATEX 101
#define SV_ROTATEY 102
#define SV_ROTATEZ 103
#define SV_SCALEXYZ 104
#define SV_SCALE 105
#define SV_SCALEAUTO 106
#define SV_SCALEGRID 107
#define SV_OFFSETX 108
#define SV_OFFSETY 109
#define SV_OFFSETZ 110
#define SV_MULTIADDT 112
#define SV_CLIP 113
#define SV_MIRRORCLIP 114
#define SV_PERIODICCLIP 115
#define SV_ADD 116
#define SV_SUB 117
#define SV_MULT 118
#define SV_DIV 119
#define SV_GETT 120
#define SV_IF 121
#define SV_ELSE 122
#define SV_ENDIF 123
#define SV_GT 124
#define SV_GE 125
#define SV_LT 126
#define SV_LE 127
#define SV_AND 128
#define SV_OR 129
#define SV_ABS 130
#define SV_EQ 131
#define SV_ROTATEXYZ 132
#define SV_GTRANSLATE 133
#define SV_ROTATEAXIS 134
#define SV_ROTATEEYE 135
#define SV_INCLUDE 136
#define SV_INCLUDEF 137
#define SV_RANDXY 138
#define SV_RANDXZ 139
#define SV_RANDYZ 140
#define SV_RANDXYZ 141
#define SV_ORIENX 142
#define SV_ORIENY 143
#define SV_ORIENZ 144
#define SV_CLIPX 146
#define SV_CLIPY 147
#define SV_CLIPZ 148
#define SV_CLIPOFF 149

#define SV_TRANSLATE_NUMARGS 3
#define SV_ROTATEX_NUMARGS 1
#define SV_ROTATEY_NUMARGS 1
#define SV_ROTATEZ_NUMARGS 1
#define SV_SCALEXYZ_NUMARGS 3
#define SV_SCALEAUTO_NUMARGS 1
#define SV_SCALEGRID_NUMARGS 1
#define SV_SCALE_NUMARGS 1
#define SV_OFFSETX_NUMARGS 1
#define SV_OFFSETY_NUMARGS 1
#define SV_OFFSETZ_NUMARGS 1
#define SV_MULTIADDT_NUMARGS 3
#define SV_CLIP_NUMARGS 4
#define SV_MIRRORCLIP_NUMARGS 4
#define SV_PERIODICCLIP_NUMARGS 4
#define SV_ADD_NUMARGS 3
#define SV_SUB_NUMARGS 3
#define SV_MULT_NUMARGS 3
#define SV_DIV_NUMARGS 3
#define SV_GETT_NUMARGS 1
#define SV_IF_NUMARGS 1
#define SV_ELSE_NUMARGS 0
#define SV_ENDIF_NUMARGS 0
#define SV_GT_NUMARGS 3
#define SV_GE_NUMARGS 3
#define SV_LT_NUMARGS 3
#define SV_LE_NUMARGS 3
#define SV_AND_NUMARGS 3
#define SV_OR_NUMARGS 3
#define SV_ABS_NUMARGS 2
#define SV_EQ_NUMARGS 2
#define SV_ROTATEXYZ_NUMARGS 3
#define SV_GTRANSLATE_NUMARGS 3
#define SV_ROTATEAXIS_NUMARGS 4
#define SV_ROTATEEYE_NUMARGS 0
#define SV_INCLUDE_NUMARGS 1
#define SV_INCLUDEF_NUMARGS 2
#define SV_RANDXY_NUMARGS 1
#define SV_RANDXZ_NUMARGS 1
#define SV_RANDYZ_NUMARGS 1
#define SV_RANDXYZ_NUMARGS 1
#define SV_ORIENX_NUMARGS 3
#define SV_ORIENY_NUMARGS 3
#define SV_ORIENZ_NUMARGS 3
#define SV_CLIPX_NUMARGS 4
#define SV_CLIPY_NUMARGS 4
#define SV_CLIPZ_NUMARGS 4
#define SV_CLIPOFF_NUMARGS 0

#define SV_TRANSLATE_NUMOUTARGS 0
#define SV_ROTATEX_NUMOUTARGS 0
#define SV_ROTATEY_NUMOUTARGS 0
#define SV_ROTATEZ_NUMOUTARGS 0
#define SV_SCALEXYZ_NUMOUTARGS 0
#define SV_SCALEAUTO_NUMOUTARGS 0
#define SV_SCALEGRID_NUMOUTARGS 0
#define SV_SCALE_NUMOUTARGS 0
#define SV_OFFSETX_NUMOUTARGS 0
#define SV_OFFSETY_NUMOUTARGS 0
#define SV_OFFSETZ_NUMOUTARGS 0
#define SV_MULTIADDT_NUMOUTARGS 1
#define SV_CLIP_NUMOUTARGS 1
#define SV_MIRRORCLIP_NUMOUTARGS 1
#define SV_PERIODICCLIP_NUMOUTARGS 1
#define SV_ADD_NUMOUTARGS 1
#define SV_SUB_NUMOUTARGS 1
#define SV_MULT_NUMOUTARGS 1
#define SV_DIV_NUMOUTARGS 1
#define SV_GETT_NUMOUTARGS 1
#define SV_IF_NUMOUTARGS 0
#define SV_ELSE_NUMOUTARGS 0
#define SV_ENDIF_NUMOUTARGS 0
#define SV_GT_NUMOUTARGS 1
#define SV_GE_NUMOUTARGS 1
#define SV_LT_NUMOUTARGS 1
#define SV_LE_NUMOUTARGS 1
#define SV_AND_NUMOUTARGS 1
#define SV_OR_NUMOUTARGS 1
#define SV_ABS_NUMOUTARGS 1
#define SV_EQ_NUMOUTARGS 0
#define SV_ROTATEXYZ_NUMOUTARGS 0
#define SV_GTRANSLATE_NUMOUTARGS 0
#define SV_ROTATEAXIS_NUMOUTARGS 0
#define SV_ROTATEEYE_NUMOUTARGS 0
#define SV_INCLUDE_NUMOUTARGS 0
#define SV_INCLUDEF_NUMOUTARGS 0
#define SV_RANDXY_NUMOUTARGS 0
#define SV_RANDXZ_NUMOUTARGS 0
#define SV_RANDYZ_NUMOUTARGS 0
#define SV_RANDXYZ_NUMOUTARGS 0
#define SV_ORIENX_NUMOUTARGS 0
#define SV_ORIENY_NUMOUTARGS 0
#define SV_ORIENZ_NUMOUTARGS 0
#define SV_CLIPX_NUMOUTARGS 0
#define SV_CLIPY_NUMOUTARGS 0
#define SV_CLIPZ_NUMOUTARGS 0
#define SV_CLIPOFF_NUMOUTARGS 0

#define SV_DRAWCUBE 200
#define SV_DRAWSPHERE 201
#define SV_DRAWDISK 202
#define SV_DRAWLINE 203
#define SV_DRAWCIRCLE 204
#define SV_DRAWTRUNCCONE 205
#define SV_DRAWNOTCHPLATE 206
#define SV_DRAWRING 207
#define SV_DRAWCONE 208
#define SV_DRAWHEXDISK 209
#define SV_DRAWPOLYDISK 210
#define SV_DRAWPOINT 211
#define SV_DRAWARC 212
#define SV_DRAWCDISK 213
#define SV_DRAWTSPHERE 214
#define SV_DRAWARCDISK 215
#define SV_DRAWSQUARE 216
#define SV_DRAWVENT 217
#define SV_DRAWCUBEC 218
#define SV_DRAWHSPHERE 219
#define SV_DRAWTRIBLOCK 220
#define SV_DRAWFILLEDCIRCLE 221
#define SV_DRAWWHEEL 222
#define SV_DRAWBOXXYZ 223
#define SV_DRAWPRISMXYZ 224

#define SV_DRAWCUBE_NUMARGS 1
#define SV_DRAWSPHERE_NUMARGS 1
#define SV_DRAWDISK_NUMARGS 2
#define SV_DRAWLINE_NUMARGS 6
#define SV_DRAWCIRCLE_NUMARGS 1
#define SV_DRAWFILLEDCIRCLE_NUMARGS 1
#define SV_DRAWTRUNCCONE_NUMARGS 3
#define SV_DRAWNOTCHPLATE_NUMARGS 4
#define SV_DRAWRING_NUMARGS 3
#define SV_DRAWCONE_NUMARGS 2
#define SV_DRAWHEXDISK_NUMARGS 2
#define SV_DRAWPOLYDISK_NUMARGS 3
#define SV_DRAWPOINT_NUMARGS 0
#define SV_DRAWARC_NUMARGS 2
#define SV_DRAWCDISK_NUMARGS 2
#define SV_DRAWTSPHERE_NUMARGS 2
#define SV_DRAWARCDISK_NUMARGS 3
#define SV_DRAWSQUARE_NUMARGS 1
#define SV_DRAWVENT_NUMARGS 2
#define SV_DRAWCUBEC_NUMARGS 1
#define SV_DRAWHSPHERE_NUMARGS 1
#define SV_DRAWTRIBLOCK_NUMARGS 2
#define SV_DRAWWHEEL_NUMARGS 2
#define SV_DRAWBOXXYZ_NUMARGS 6
#define SV_DRAWPRISMXYZ_NUMARGS 7

#define SV_DRAWCUBE_NUMOUTARGS 0
#define SV_DRAWSPHERE_NUMOUTARGS 0
#define SV_DRAWDISK_NUMOUTARGS 0
#define SV_DRAWLINE_NUMOUTARGS 0
#define SV_DRAWCIRCLE_NUMOUTARGS 0
#define SV_DRAWFILLEDCIRCLE_NUMOUTARGS 0
#define SV_DRAWTRUNCCONE_NUMOUTARGS 0
#define SV_DRAWNOTCHPLATE_NUMOUTARGS 0
#define SV_DRAWRING_NUMOUTARGS 0
#define SV_DRAWCONE_NUMOUTARGS 0
#define SV_DRAWHEXDISK_NUMOUTARGS 0
#define SV_DRAWPOLYDISK_NUMOUTARGS 0
#define SV_DRAWPOINT_NUMOUTARGS 0
#define SV_DRAWARC_NUMOUTARGS 0
#define SV_DRAWCDISK_NUMOUTARGS 0
#define SV_DRAWTSPHERE_NUMOUTARGS 0
#define SV_DRAWARCDISK_NUMOUTARGS 0
#define SV_DRAWSQUARE_NUMOUTARGS 0
#define SV_DRAWVENT_NUMOUTARGS 0
#define SV_DRAWCUBEC_NUMOUTARGS 0
#define SV_DRAWHSPHERE_NUMOUTARGS 0
#define SV_DRAWTRIBLOCK_NUMOUTARGS 0
#define SV_DRAWWHEEL_NUMOUTARGS 0
#define SV_DRAWBOXXYZ_NUMOUTARGS 0
#define SV_DRAWPRISMXYZ_NUMOUTARGS 0

#define SV_PUSH 300
#define SV_POP 301
#define SV_SETRGB 302
#define SV_SETRGBVAL 303
#define SV_SETBW 304
#define SV_SETLINEWIDTH 305
#define SV_SETPOINTSIZE 306
#define SV_SETCOLOR 307
#define SV_GETTEXTUREINDEX 308

#define SV_NO_OP 999

#define SV_PUSH_NUMARGS 0
#define SV_POP_NUMARGS 0
#define SV_SETRGB_NUMARGS 3
#define SV_SETRGBVAL_NUMARGS 3
#define SV_SETBW_NUMARGS 1
#define SV_SETLINEWIDTH_NUMARGS 1
#define SV_SETPOINTSIZE_NUMARGS 1
#define SV_SETCOLOR_NUMARGS 1
#define SV_GETTEXTUREINDEX_NUMARGS 2
#define SV_NO_OP_NUMARGS 0

#define SV_PUSH_NUMOUTARGS 0
#define SV_POP_NUMOUTARGS 0
#define SV_SETRGB_NUMOUTARGS 0
#define SV_SETRGBVAL_NUMOUTARGS 0
#define SV_SETBW_NUMOUTARGS 0
#define SV_SETLINEWIDTH_NUMOUTARGS 0
#define SV_SETPOINTSIZE_NUMOUTARGS 0
#define SV_SETCOLOR_NUMOUTARGS 0
#define SV_GETTEXTUREINDEX_NUMOUTARGS 1
#define SV_NO_OP_NUMOUTARGS 0

#define SV_ERR -1

#define IS_AVATAR 1
#define IS_NOT_AVATAR 0

#define NLAT device_sphere_segments
#define NLONG (2 * device_sphere_segments)

// BEGIN MAIN API

/**
 * @brief Initialise an @ref object_collection.
 *
 * @returns A @ref object_collection which has been properly initialized.
 */
object_collection *CreateObjectCollection(void);

/**
 * @brief Initialise an already allocated object_collection. This is useful
 * when the collection is allocated as part of a larger data structure.
 *
 * @param[inout] coll
 * @return int
 */
int InitObjectCollection(object_collection *coll);

/**
 * @brief Read objects from the standard file locations, using fallback objects
 * if object definitions are not found.
 *
 * @param[inout] objectscoll Pointer to the location of the @ref
 * object_collection to read object definitions into. This @ref
 * object_collection
 * @param[in] fdsprefix The fdsprefix. This is used to find case-specific object
 * files (e.g., "${fdsprefix}.svo"). If NULL, such files are never read.
 * @param[in] isZoneFireModel Is this model a zone fire model.
 */
void ReadDefaultObjectCollection(object_collection *objectscoll,
                                 const char *fdsprefix,
                                 int isZoneFireModel);

/**
 * @brief Clear a @ref object_collection. This does not free the data structure
 * itself but simply empties it.
 *
 * @param[inout] objectscoll The @ref object_collection to clear.
 */
void ClearObjectCollection(object_collection *objectscoll);

/**
 * @brief Free an @ref object_collection previously allocated with NewMemory or
 * created by @ref CreateObjectCollection.
 *
 * @param[inout] objectscoll The @ref object_collection to free.
 */
void FreeObjectCollection(object_collection *objectscoll);

/**
 * @brief Given a label, find the @ref sv_object in the given @ref
 * object_collection.
 *
 * @param objectscoll The object_collection to search.
 * @param label The label to search for.
 *
 * @return A pointer to the object if found, NULL if not found.
 */
sv_object *GetSmvObject(object_collection *objectscoll, char *label);

/**
 * @brief Given a label, find the @ref sv_object in the given @ref
 * object_collection.
 *
 * @param objectscoll The object_collection to search.
 * @param label The label to search for.
 * @param default_object The object to return if an object is not found.
 *
 * @return A pointer to the object if found, NULL if not found.
 */
sv_object *GetSmvObjectType(object_collection *objectscoll, char *olabel,
                            sv_object *default_object);

/**
 * @brief Read in object definitions from an object file and add them to an
 * object collection.
 *
 * @param[inout] nobject_defs The number of object definitions in object_defs
 * @param[in] file The file path to read from
 * @param[out] setbw A boolean value to set wether these objects should be set
 * to black and white
 *
 * @returns The number of objects read
 */
int ReadObjectDefs(object_collection *objectscoll, const char *file);
// END MAIN API

// These still need to be documented.

int GetObjectFrameTokenLoc(char *var, sv_object_frame *frame);
void GetIndepVarIndices(sv_object *smv_object, char **var_indep_strings,
                        int nvars_indep, int *index);
void UpdateDeviceTextures(object_collection *objectscoll, int ndeviceinfo,
                          devicedata *deviceinfo, int npropinfo,
                          propdata *propinfo, int *ndevice_texture_list,
                          int **device_texture_list_indexptr,
                          char ***device_texture_listptr);
EXTERNCPP void UpdatePartClassDepend(partclassdata *partclassi);

#endif
