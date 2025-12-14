#ifndef IOSCRIPTS_H_DEFINED
#define IOSCRIPTS_H_DEFINED

#define EXTERNAL_VIEW 1

#define SCRIPT_RENDERONCE        101
#ifdef pp_HTML
#define SCRIPT_RENDERHTMLONCE    102
#define SCRIPT_RENDERHTMLALL     105
#define SCRIPT_RENDERHTMLDIR     109
#define SCRIPT_RENDERHTMLGEOM    123
#define SCRIPT_RENDERHTMLOBST    124
#define SCRIPT_RENDERHTMLSLICENODE 125
#define SCRIPT_RENDERHTMLSLICECELL 126
#endif
#define SCRIPT_RENDERDOUBLEONCE  103
#define SCRIPT_RENDERALL         104
#define SCRIPT_RENDER360ALL      106
#define SCRIPT_VOLSMOKERENDERALL 107
#define SCRIPT_RENDERDIR         108
#define SCRIPT_RENDERCLIP        110
#define SCRIPT_SCENECLIP         111
#define SCRIPT_XSCENECLIP        112
#define SCRIPT_YSCENECLIP        113
#define SCRIPT_ZSCENECLIP        114
#define SCRIPT_CBARFLIP          115
#define SCRIPT_CBARNORMAL        116
#define SCRIPT_RENDERSTART       117
#define SCRIPT_MAKEMOVIE         118
#define SCRIPT_RENDERTYPE        119
#define SCRIPT_RENDERSIZE        120
#define SCRIPT_MOVIETYPE         121
#define SCRIPT_ISORENDERALL      122
#define SCRIPT_GPUOFF            127

#define SCRIPT_LOADFILE          201
#define SCRIPT_LOADVFILE         202
#define SCRIPT_LOADBOUNDARY      203
#define SCRIPT_LOAD3DSMOKE       204
#define SCRIPT_LOADISO           205
#define SCRIPT_LOADPARTICLES     206
#define SCRIPT_LOADSLICE         207
#define SCRIPT_LOADVSLICE        208
#define SCRIPT_LOADTOUR          209
#define SCRIPT_UNLOADTOUR        210
#define SCRIPT_PARTCLASSCOLOR    211
#define SCRIPT_PARTCLASSTYPE     212
#define SCRIPT_LOADINIFILE       213
#define SCRIPT_LOADPLOT3D        214
#define SCRIPT_SHOWPLOT3DDATA    215
#define SCRIPT_PLOT3DPROPS       216
#define SCRIPT_LOADVOLSMOKE      217
#define SCRIPT_LOADVOLSMOKEFRAME 218
#define SCRIPT_LOADISOM          219
#define SCRIPT_LOADBOUNDARYM     220
#define SCRIPT_LOADSLICEM        221
#define SCRIPT_LOADVSLICEM       222
#define SCRIPT_OUTPUTSMOKESENSORS  223
#define SCRIPT_RGBTEST           225
#define SCRIPT_XYZVIEW           226
#define SCRIPT_VIEWXMIN          227
#define SCRIPT_VIEWXMAX          228
#define SCRIPT_VIEWYMIN          229
#define SCRIPT_VIEWYMAX          230
#define SCRIPT_VIEWZMIN          231
#define SCRIPT_VIEWZMAX          232
#define SCRIPT_LOADSLICERENDER   233
#define SCRIPT_LOADSLCF          234
#define SCRIPT_SHOWHVACDUCTVAL   235
#define SCRIPT_SHOWHVACNODEVAL   236
#define SCRIPT_HIDEHVACVALS      237
#define SCRIPT_SHOWALLDEVS       238
#define SCRIPT_HIDEALLDEVS       239
#define SCRIPT_SHOWDEV           240
#define SCRIPT_HIDEDEV           241
#define SCRIPT_UNLOADPLOT2D      242
#define SCRIPT_SHOWCBAREDIT      243
#define SCRIPT_HIDECBAREDIT      244
#define SCRIPT_SETCBAR           245
#define SCRIPT_SETCBARLAB        246
#define SCRIPT_SETCBARRGB        247
#define SCRIPT_LOADSMOKERENDER   248
#define SCRIPT_HILIGHTMINVALS    249
#define SCRIPT_HILIGHTMAXVALS    250
#define SCRIPT_LOADHVAC          251
#define SCRIPT_SETDEMOMODE       252
#define SCRIPT_SMOKEPROP         253
#define SCRIPT_LOADSMV           254

#define SCRIPT_SETTIMEVAL        301
#define SCRIPT_SETVIEWPOINT      302
#define SCRIPT_UNLOADALL         303
#define SCRIPT_KEYBOARD          304
#define SCRIPT_GSLICEVIEW        305
#define SCRIPT_GSLICEPOS         306
#define SCRIPT_GSLICEORIEN       307
#define SCRIPT_SETTOURVIEW       308
#define SCRIPT_SETTOURKEYFRAME   309
#define SCRIPT_EXIT              310
#define SCRIPT_LABEL             311
#define SCRIPT_PROJECTION        312
#define SCRIPT_SETSLICEBOUNDS    314
#define SCRIPT_SETBOUNDBOUNDS    315
#define SCRIPT_SETCLIPX          316
#define SCRIPT_SETCLIPY          317
#define SCRIPT_SETCLIPZ          318
#define SCRIPT_SETCLIPMODE       319
#define SCRIPT_SETSLICEAVERAGE   320
#define SCRIPT_OUTPUTSLICEDATA   321
#define SCRIPT_NOEXIT            322

#define SCRIPT_SLICE_FILE          0
#define SCRIPT_BOUNDARY_FILE       1
#define SCRIPT_SMOKE3D_FILE        2
#define SCRIPT_PART_FILE           3
#define SCRIPT_ISO_FILE            4

#define SCRIPT_UNKNOWN            -1
#define SCRIPT_OK               -997
#define SCRIPT_ERR              -998
#define SCRIPT_EOF              -999

#define NOT_LOADRENDER (current_script_command->command!=SCRIPT_LOADSLICERENDER && current_script_command->command != SCRIPT_LOADSMOKERENDER)
#define IS_LOADRENDER  (current_script_command->command==SCRIPT_LOADSLICERENDER || current_script_command->command == SCRIPT_LOADSMOKERENDER)

//*** IOscript.c headers

EXTERNCPP int  CompileScript(char *scriptfile);
EXTERNCPP char *GetIniFileName(int id);
EXTERNCPP void GetNewScriptFileName(char *newscriptfilename);
EXTERNCPP char *GetScriptFileName(int id);
EXTERNCPP int  GetVolFrameMax(int meshnum);
EXTERNCPP inifiledata *InsertIniFile(char *file);
EXTERNCPP scriptfiledata *InsertScriptFile(char *file);
EXTERNCPP void LoadSmokeFrame(int meshnum, int framenum);
EXTERNCPP void LoadTimeFrame(int meshnum, float timeval);
EXTERNCPP int  RunScriptCommand(scriptdata *script_command);
EXTERNCPP void ScriptLoadIsoFrame2(scriptdata *scripti);
EXTERNCPP void ScriptLoadSliceRender(scriptdata *scripti);
EXTERNCPP void ScriptLoadSmokeRender(scriptdata *scripti);
EXTERNCPP void ScriptLoadVolSmokeFrame2(void);
EXTERNCPP void ScriptViewXYZMINMAXOrtho(int option);
EXTERNCPP void SetTimeVal(float timeval);
EXTERNCPP void SetViewZMAXPersp(void);
EXTERNCPP void StartScript(void);


#endif

