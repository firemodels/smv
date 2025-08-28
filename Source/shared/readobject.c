#include "options.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dmalloc.h"
#include "datadefs.h"
#include "histogram.h"
#include "isobox.h"
#include "string_util.h"

#include "readobject.h"
#include "stdio_m.h"

#define BUFFER_SIZE 1000

#define TOKEN_FLOAT 0
#define TOKEN_COMMAND 1
#define TOKEN_GETVAL 2
#define TOKEN_STRING 3
#define TOKEN_TEXTURE 4

#define BOX_CLIPPLANES 0

int GetTokenId(char *token, int *opptr, int *num_opptr, int *num_outopptr);
void ParseSmvObjectString(object_collection *objectscoll, char *string,
                          char **tokens, int *ntokens);
sv_object *InitSmvObject1(object_collection *objectscoll, const char *label,
                          const char *commands, int visible);
sv_object *InitSmvObject2(object_collection *objectscoll, const char *label,
                          const char *commandsoff, const char *commandson,
                          int visible);
sv_object *GetSmvObjectType2(object_collection *objectscoll, char *olabel,
                             sv_object *default_object);
void FreeObject(sv_object *object);

char *ParseObjectFrame(object_collection *objectscoll, const char *buffer,
                       FILE *stream, int *eof, sv_object_frame *frame);

/* ----------------------- FreeObject ----------------------------- */

void FreeObject(sv_object *object){
  sv_object *prev, *next;
  sv_object_frame *framei, *frame_start;

  prev = object->prev;
  next = object->next;

  prev->next = next;
  next->prev = prev;

  frame_start = &object->first_frame;
  framei = frame_start->next;
  for(; framei->next != NULL;){
    sv_object_frame *next_frame;

    next_frame = framei->next;
    if(framei->nsymbols > 0){
      FREEMEMORY(framei->symbols);
    }
    if(framei->ntokens > 0){
      FREEMEMORY(framei->tokens);
    }
    FREEMEMORY(framei);
    framei = next_frame;
  }
  FREEMEMORY(object);
}

/* ----------------------- InitBoxClipInfo ----------------------------- */

void InitBoxClipInfo(clipdata *ci, float xmin, float xmax, float ymin,
                     float ymax, float zmin, float zmax){
  ci->option = BOX_CLIPPLANES;
  ci->clip_xmin = 1;
  ci->clip_xmax = 1;
  ci->clip_ymin = 1;
  ci->clip_ymax = 1;
  ci->clip_zmin = 1;
  ci->clip_zmax = 1;
  ci->xmin = xmin;
  ci->xmax = xmax;
  ci->ymin = ymin;
  ci->ymax = ymax;
  ci->zmin = zmin;
  ci->zmax = zmax;
}

/* ----------------------- get_SMVOBJECT_type ----------------------------- */

sv_object *GetSmvObjectType(object_collection *objectscoll, char *olabel,
                            sv_object *default_object){
  int i;
  sv_object *objecti;
  char label[256], *labelptr;

  if(olabel == NULL) return default_object;
  strcpy(label, olabel);
  TrimBack(label);
  labelptr = TrimFront(label);
  if(strlen(labelptr) == 0) return default_object;
  for(i = 0; i < objectscoll->nobject_defs; i++){
    objecti = objectscoll->object_defs[i];
    if(STRCMP(labelptr, objecti->label) == 0){
      objecti->used = 1;
      return objecti;
    }
  }
  return default_object;
}

/* ----------------------- get_SMVOBJECT_type2 ----------------------------- */

sv_object *GetSmvObjectType2(object_collection *objectscoll, char *olabel,
                             sv_object *default_object){
  sv_object *object_start, *objecti;
  char label[256], *labelptr;

  if(olabel == NULL) return default_object;
  strcpy(label, olabel);
  TrimBack(label);
  labelptr = TrimFront(label);
  if(strlen(labelptr) == 0) return default_object;
  object_start = objectscoll->object_def_first.next;
  objecti = object_start;
  for(; objecti->next != NULL;){
    if(STRCMP(labelptr, objecti->label) == 0){
      objecti->used = 1;
      return objecti;
    }
    objecti = objecti->next;
  }
  return default_object;
}

/* ------------------ ParseSmvObjectString ------------------------ */

void ParseSmvObjectString(object_collection *objectscoll, char *string,
                          char **tokens, int *ntokens){
  int i, len, in_quote, in_token, last_in_token, ntok2 = 0;
  char *c;
  char *tokens_head[BUFFER_SIZE], *tokens_tail[BUFFER_SIZE];
  int in_head = 1, nhead = 0, ntail = 0;

  for(i=0;i<BUFFER_SIZE;i++){
    tokens_tail[i] = NULL;
  }
  c = string;
  in_quote = 0;
  in_token = 0;
  last_in_token = 0;
  len = strlen(string);
  for(i = 0; i <= len; i++){
    switch(*c){
    case '"':
      in_quote = 1 - in_quote;
      in_token = 1;
      break;
    case ' ':
      if(in_quote == 0){
        in_token = 0;
      }
      break;
    case 0:
      in_token = 0;
      break;
    default:
      in_token = 1;
      break;
    }
    if(in_token > last_in_token){
      if(in_head == 1 && c[0] == ':'){
        tokens_head[nhead++] = c;
      }
      else{
        tokens_tail[ntail++] = c;
        in_head = 0;
      }
    }
    if(in_token < last_in_token){
      char *tok;
      int in_head2;

      *c = 0;
      if(ntail > 0) tok = tokens_tail[ntail - 1];
      if(ntail > 0 &&
          (strcmp(tok, "include") == 0 || strcmp(tok, "includef") == 0)){
        int j;
        sv_object *included_object;
        int iframe_local;
        char *object_name;
        int nparms;
        sv_object_frame *frame;
        int len2;

        object_name = tokens_tail[ntail - 2];
        if(object_name[0] == '"') object_name++;
        len2 = strlen(object_name);
        if(object_name[len2 - 1] == '"') object_name[len2 - 1] = 0;

        if(objectscoll->std_object_defs.missing_device == NULL){
          char com_buffer[1024];

          strcpy(
              com_buffer,
              "0 0 255 setrgb push 45.0 rotatey -0.1 offsetz 0.05 0.2 drawdisk "
              "pop push -45.0 rotatey -0.1 offsetz 0.05 0.2 drawdisk pop");
          objectscoll->std_object_defs.missing_device = InitSmvObject1(
              objectscoll, "missing_device", com_buffer, 1);
        }

        included_object =
            GetSmvObjectType2(objectscoll, object_name,
                              objectscoll->std_object_defs.missing_device);

        if(strcmp(tok, "includef") == 0 &&
            included_object != objectscoll->std_object_defs.missing_device &&
            ntail > 2){
          char *iframe_label;

          iframe_label = tokens_tail[ntail - 3];
          sscanf(iframe_label, "%i", &iframe_local);
          if(iframe_local < 0) iframe_local = 0;
          if(iframe_local > included_object->nframes - 1)
            iframe_local = included_object->nframes - 1;
          nparms = 3;
        }
        else{
          iframe_local = 0;
          nparms = 2;
        }
        ntail -= nparms;
        for(j = 0, frame = included_object->first_frame.next;
             frame->next != NULL; j++, frame = frame->next){
          if(j == iframe_local) break;
        }
        in_head2 = 1;
        for(j = 0; j < frame->ntokens; j++){
          char *cc;

          cc = frame->tokens[j].tokenlabel;
          if(in_head2 == 1 && cc[0] == ':'){
            tokens_head[nhead++] = frame->tokens[j].tokenfulllabel;
          }
          else{
            in_head2 = 0;
            tokens_tail[ntail++] = cc;
          }
        }
      }
    }
    last_in_token = in_token;
    c++;
  }
  ntok2 = 0;
  for(i = 0; i < nhead; i++){
    tokens[ntok2++] = tokens_head[i];
  }
  for(i = 0; i < ntail; i++){
    tokens[ntok2++] = tokens_tail[i];
  }
  *ntokens = ntok2;
}

/* ----------------------- GetIndepVarIndices ----------------------------- */

void GetIndepVarIndices(sv_object *smv_object, char **var_indep_strings,
                        int nvars_indep, int *index){

  int i;
  sv_object_frame *obj_frame;

  obj_frame = smv_object->obj_frames[0];

  for(i = 0; i < nvars_indep; i++){
    char *var;

    var = var_indep_strings[i];
    index[i] = GetObjectFrameTokenLoc(var, obj_frame);
  }
}

/* ----------------------- ParseObjectFrame ----------------------------- */

// Previously ParseDeviceFrame
char *ParseObjectFrame(object_collection *objectscoll, const char *buffer_in,
                       FILE *stream, int *eof, sv_object_frame *frame){
  char buffer[1024];

  char object_buffer[10 * BUFFER_SIZE];
  int ntokens;
  char *token, *tokens[BUFFER_SIZE];
  char *buffer_ptr = NULL, *buffer2;
  int i;
  int nsymbols, ncommands;
  int ntextures_local = 0;
  int last_command_index = 0;

  strcpy(buffer, buffer_in);
  *eof = 0;

  frame->error = 0;
  TrimBack(buffer);
  strcpy(object_buffer, buffer);
  while(stream != NULL && !feof(stream)){
    if(fgets(buffer, 255, stream) == NULL){
      *eof = 1;
      break;
    }
    RemoveComment(buffer);
    buffer2 = TrimFront(buffer);
    if(Match(buffer2, "OBJECTDEF") == 1 || Match(buffer2, "AVATARDEF") == 1 ||
        Match(buffer2, "NEWFRAME") == 1){
      buffer_ptr = buffer2;
      break;
    }
    strcat(object_buffer, " ");
    strcat(object_buffer, buffer2);
  }
  ParseSmvObjectString(objectscoll, object_buffer, tokens, &ntokens);
  frame->ntokens = ntokens;
  if(ntokens > 0){
    NewMemory((void **)&frame->tokens, ntokens * sizeof(tokendata));
    NewMemory((void **)&frame->symbols, ntokens * sizeof(int));
    NewMemory((void **)&frame->command_list, ntokens * sizeof(tokendata *));
  }

  // count symbols and commands, zero out access counter

  nsymbols = 0;
  ncommands = 0;
  for(i = 0; i < ntokens; i++){
    tokendata *toki;
    char c;
    token = tokens[i];
    toki = frame->tokens + i;
    strcpy(toki->token, token);
    strcpy(toki->tokenlabel, token);
    strcpy(toki->tokenfulllabel, token);
    toki->reads = 0;

    c = token[0];

    if(c == ':'){
      frame->symbols[nsymbols++] = i;
    }
    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) ncommands++;
  }
  frame->nsymbols = nsymbols;
  frame->ncommands = ncommands;

  // fill in token data structure

  nsymbols = 0;
  ncommands = 0;
  for(i = 0; i < ntokens; i++){
    tokendata *toki;
    char c;

    toki = frame->tokens + i;

    c = toki->token[0];
    toki->is_label = 0;
    toki->is_string = 0;
    toki->is_texturefile = 0;
    toki->next = NULL;
    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')){
      int nargs_actual, noutargs_actual;
      tokendata *this_token, *last_token;
      int error_code;

      toki->type = TOKEN_COMMAND;
      error_code = GetTokenId(toki->token, &toki->command, &toki->nvars,
                              &toki->noutvars);
      toki->included_frame = 0;
      toki->included_object = NULL;
      if(error_code == 1){
        frame->error = 1;
        fprintf(
            stderr,
            "*** Error: unable to identify the command, %s, while parsing:\n\n",
            toki->token);
        fprintf(stderr, "      %s\n\n", object_buffer);
      }
      frame->command_list[ncommands] = toki;
      if(ncommands > 0){
        this_token = toki;
        last_token = frame->command_list[ncommands - 1];
        last_token->next = this_token;
        this_token->next = NULL;
        nargs_actual = i - last_command_index - 1;
      }
      else{
        nargs_actual = toki->nvars;
      }
      if(nargs_actual != toki->nvars){
        frame->error = 1;
        if(toki->nvars == 1){
          fprintf(stderr,
                  "*** Error: The command %s in device %s has %i arguments, %i "
                  "was expected\n",
                  toki->token, frame->device->label, nargs_actual, toki->nvars);
        }
        else{
          fprintf(stderr,
                  "*** Error: The command %s in device %s has %i arguments, %i "
                  "were expected\n",
                  toki->token, frame->device->label, nargs_actual, toki->nvars);
        }
      }
      if(nargs_actual == toki->nvars){
        int ii;

        noutargs_actual = 0;
        for(ii = 0; ii < nargs_actual; ii++){
          tokendata *tokii;

          tokii = toki - 1 - ii;
          if(tokii < frame->tokens) break;
          c = tokii->token[0];
          if(c != ':') continue;
          noutargs_actual++;
        }
        if(noutargs_actual != toki->noutvars){
          if(toki->noutvars == 1){
            fprintf(stderr,
                    "*** Error: The command %s in device %s has %i output "
                    "arguments, %i was expected\n",
                    toki->token, frame->device->label, noutargs_actual,
                    toki->noutvars);
          }
          else{
            fprintf(stderr,
                    "*** Error: The command %s in device %s has %i output "
                    "arguments, %i were expected\n",
                    toki->token, frame->device->label, noutargs_actual,
                    toki->noutvars);
          }
        }
      }
      ncommands++;
      last_command_index = i;
    }
    else if(c == '$'){
      tokendata *tokdest;

      toki->loc = GetObjectFrameTokenLoc(toki->token + 1, frame);
      if(toki->loc >= 0){
        tokdest = frame->tokens + toki->loc;
        toki->varptr = &tokdest->var;
        toki->stringptr = tokdest->string;
        tokdest->reads++;
      }
      else{
        frame->error = 1;
        toki->varptr = NULL;
        toki->stringptr = NULL;
        fprintf(stderr, "*** Error: The label %s in device %s is not defined\n",
                toki->token, frame->device->label);
      }

      toki->type = TOKEN_GETVAL;
    }
    else if(c == ':'){
      char *var, *val, *equal;
      char bufcopy[1024];

      strcpy(bufcopy, toki->token);
      var = strtok(bufcopy, "=");
      toki->default_val = 0.0;
      strcpy(toki->default_string, "");
      if(var != NULL){
        val = strtok(NULL, "=");
        if(val != NULL){
          char *quoted_string;

          quoted_string = strstr(val, "\"");
          if(quoted_string != NULL){
            int len;

            toki->is_string = 1;
            quoted_string++;
            len = strlen(quoted_string);
            if(quoted_string[len - 1] == '"') quoted_string[len - 1] = ' ';
            TrimBack(quoted_string);
            quoted_string = TrimFront(quoted_string);
            strcpy(toki->default_string, quoted_string);
            quoted_string = strstr(quoted_string, "t%");
            if(quoted_string != NULL){
              quoted_string += 2;
              quoted_string = TrimFront(quoted_string);
              strcpy(toki->default_string, quoted_string);
              toki->is_texturefile = 1;
            }
            toki->default_val = 0.0;
          }
          else{
            strcpy(toki->tokenlabel, var);
            sscanf(val, "%f", &toki->default_val);
          }
        }
      }
      equal = strchr(toki->token, '=');
      if(equal != NULL) *equal = 0;
      toki->type = TOKEN_FLOAT;
      toki->varptr = &toki->var;
      toki->stringptr = toki->string;
      toki->is_label = 1;
      nsymbols++;
    }
    else if(c == '"'){
      char string_copy[256], *sptr;
      int lenstr;
      char *texturefile;

      toki->type = TOKEN_STRING;
      toki->var = 0.0;
      toki->varptr = &toki->var;
      toki->stringptr = toki->string;
      sptr = string_copy;
      strcpy(sptr, toki->token);
      sptr++;
      texturefile = strstr(sptr, "t%");
      if(texturefile != NULL){
        sptr = texturefile + 2;
        toki->type = TOKEN_TEXTURE;
        ntextures_local++;
      }
      lenstr = strlen(sptr);
      if(sptr[lenstr - 1] == '"') sptr[lenstr - 1] = ' ';
      TrimBack(sptr);
      sptr = TrimFront(sptr);
      strcpy(toki->string, sptr);
    }
    else{
      toki->type = TOKEN_FLOAT;
      sscanf(toki->token, "%f", &toki->var);
      toki->varptr = &toki->var;
    }
  }
  frame->ntextures = ntextures_local;
  for(i = 0; i < ntokens; i++){
    tokendata *toki;
    char c;

    toki = frame->tokens + i;
    c = toki->token[0];
    if(c != ':') continue;
  }

  // define data structures for conditional tokens

  for(i = 0; i < ncommands; i++){
    tokendata *toki;

    toki = frame->command_list[i];
    switch(toki->command){
      int j, if_level;

    case SV_IF:
      if_level = 0;
      for(j = i + 1; j < ncommands; j++){
        tokendata *tokj;

        tokj = frame->command_list[j];
        if(tokj->command == SV_IF){
          if_level++;
          continue;
        }
        if(if_level > 0 && tokj->command == SV_ENDIF){
          if_level--;
          continue;
        }
        if(if_level == 0 &&
            (tokj->command == SV_ELSE || tokj->command == SV_ENDIF)){
          toki->elsenext = frame->command_list[j + 1];
          break;
        }
      }
      break;
    case SV_ELSE:
      if_level = 0;
      for(j = i + 1; j < ncommands; j++){
        tokendata *tokj;

        tokj = frame->command_list[j];
        if(tokj->command == SV_IF){
          if_level++;
          continue;
        }
        if(if_level > 0 && tokj->command == SV_ENDIF){
          if_level--;
          continue;
        }
        if(if_level == 0 && tokj->command == SV_ENDIF){
          toki->next = frame->command_list[j + 1];
          break;
        }
      }
      break;
    default:
      break;
    }
  }
  return buffer_ptr;
}

/* ----------------------- GetTokenId ----------------------------- */

int GetTokenId(char *token, int *opptr, int *num_opptr, int *num_outopptr){

  int op, num_op, num_outop;
  int return_val;


  return_val = 0;
  if(STRCMP(token, "translate") == 0){
    op = SV_TRANSLATE;
    num_op = SV_TRANSLATE_NUMARGS;
    num_outop = SV_TRANSLATE_NUMOUTARGS;
  }
  else if(STRCMP(token, "gtranslate") == 0){
    op = SV_GTRANSLATE;
    num_op = SV_GTRANSLATE_NUMARGS;
    num_outop = SV_GTRANSLATE_NUMOUTARGS;
  }
  else if(STRCMP(token, "no_op") == 0){
    op = SV_NO_OP;
    num_op = SV_NO_OP_NUMARGS;
    num_outop = SV_NO_OP_NUMOUTARGS;
  }
  else if(STRCMP(token, "offsetx") == 0){
    op = SV_OFFSETX;
    num_op = SV_OFFSETX_NUMARGS;
    num_outop = SV_OFFSETX_NUMOUTARGS;
  }
  else if(STRCMP(token, "offsety") == 0){
    op = SV_OFFSETY;
    num_op = SV_OFFSETY_NUMARGS;
    num_outop = SV_OFFSETY_NUMOUTARGS;
  }
  else if(STRCMP(token, "offsetz") == 0){
    op = SV_OFFSETZ;
    num_op = SV_OFFSETZ_NUMARGS;
    num_outop = SV_OFFSETZ_NUMOUTARGS;
  }
  else if(STRCMP(token, "rotatexyz") == 0){
    op = SV_ROTATEXYZ;
    num_op = SV_ROTATEXYZ_NUMARGS;
    num_outop = SV_ROTATEXYZ_NUMOUTARGS;
  }
  else if(STRCMP(token, "rotateaxis") == 0){
    op = SV_ROTATEAXIS;
    num_op = SV_ROTATEAXIS_NUMARGS;
    num_outop = SV_ROTATEAXIS_NUMOUTARGS;
  }
  else if(STRCMP(token, "rotateeye") == 0){
    op = SV_ROTATEEYE;
    num_op = SV_ROTATEEYE_NUMARGS;
    num_outop = SV_ROTATEEYE_NUMOUTARGS;
  }
  else if(STRCMP(token, "rotatex") == 0){
    op = SV_ROTATEX;
    num_op = SV_ROTATEX_NUMARGS;
    num_outop = SV_ROTATEX_NUMOUTARGS;
  }
  else if(STRCMP(token, "rotatey") == 0){
    op = SV_ROTATEY;
    num_op = SV_ROTATEY_NUMARGS;
    num_outop = SV_ROTATEY_NUMOUTARGS;
  }
  else if(STRCMP(token, "rotatez") == 0){
    op = SV_ROTATEZ;
    num_op = SV_ROTATEZ_NUMARGS;
    num_outop = SV_ROTATEZ_NUMOUTARGS;
  }
  else if(STRCMP(token, "if") == 0){
    op = SV_IF;
    num_op = SV_IF_NUMARGS;
    num_outop = SV_IF_NUMOUTARGS;
  }
  else if(STRCMP(token, "else") == 0){
    op = SV_ELSE;
    num_op = SV_ELSE_NUMARGS;
    num_outop = SV_ELSE_NUMOUTARGS;
  }
  else if(STRCMP(token, "endif") == 0){
    op = SV_ENDIF;
    num_op = SV_ENDIF_NUMARGS;
    num_outop = SV_ENDIF_NUMOUTARGS;
  }
  else if(STRCMP(token, "LT") == 0){
    op = SV_LT;
    num_op = SV_LT_NUMARGS;
    num_outop = SV_LT_NUMOUTARGS;
  }
  else if(STRCMP(token, "LE") == 0){
    op = SV_LE;
    num_op = SV_LE_NUMARGS;
    num_outop = SV_LE_NUMOUTARGS;
  }
  else if(STRCMP(token, "GT") == 0){
    op = SV_GT;
    num_op = SV_GT_NUMARGS;
    num_outop = SV_GT_NUMOUTARGS;
  }
  else if(STRCMP(token, "EQ") == 0){
    op = SV_EQ;
    num_op = SV_EQ_NUMARGS;
    num_outop = SV_EQ_NUMOUTARGS;
  }
  else if(STRCMP(token, "GE") == 0){
    op = SV_GE;
    num_op = SV_GE_NUMARGS;
    num_outop = SV_GE_NUMOUTARGS;
  }
  else if(STRCMP(token, "AND") == 0){
    op = SV_AND;
    num_op = SV_AND_NUMARGS;
    num_outop = SV_AND_NUMOUTARGS;
  }
  else if(STRCMP(token, "OR") == 0){
    op = SV_OR;
    num_op = SV_OR_NUMARGS;
    num_outop = SV_OR_NUMOUTARGS;
  }
  else if(STRCMP(token, "scalexyz") == 0){
    op = SV_SCALEXYZ;
    num_op = SV_SCALEXYZ_NUMARGS;
    num_outop = SV_SCALEXYZ_NUMOUTARGS;
  }
  else if(STRCMP(token, "scaleauto") == 0){
    op = SV_SCALEAUTO;
    num_op = SV_SCALEAUTO_NUMARGS;
    num_outop = SV_SCALEAUTO_NUMOUTARGS;
  }
  else if(STRCMP(token, "scalegrid") == 0){
    op = SV_SCALEGRID;
    num_op = SV_SCALEGRID_NUMARGS;
    num_outop = SV_SCALEGRID_NUMOUTARGS;
  }
  else if(STRCMP(token, "scale") == 0 && STRCMP(token, "scalexyz") != 0 &&
           STRCMP(token, "scaleauto") != 0 && STRCMP(token, "scalegrid") != 0){
    op = SV_SCALE;
    num_op = SV_SCALE_NUMARGS;
    num_outop = SV_SCALE_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawcube") == 0){
    op = SV_DRAWCUBE;
    num_op = SV_DRAWCUBE_NUMARGS;
    num_outop = SV_DRAWCUBE_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawcubec") == 0){
    op = SV_DRAWCUBEC;
    num_op = SV_DRAWCUBEC_NUMARGS;
    num_outop = SV_DRAWCUBEC_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawboxxyz") == 0){
    op = SV_DRAWBOXXYZ;
    num_op = SV_DRAWBOXXYZ_NUMARGS;
    num_outop = SV_DRAWBOXXYZ_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawprismxyz") == 0){
    op = SV_DRAWPRISMXYZ;
    num_op = SV_DRAWPRISMXYZ_NUMARGS;
    num_outop = SV_DRAWPRISMXYZ_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawvent") == 0){
    op = SV_DRAWVENT;
    num_op = SV_DRAWVENT_NUMARGS;
    num_outop = SV_DRAWVENT_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawsquare") == 0){
    op = SV_DRAWSQUARE;
    num_op = SV_DRAWSQUARE_NUMARGS;
    num_outop = SV_DRAWSQUARE_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawdisk") == 0){
    op = SV_DRAWDISK;
    num_op = SV_DRAWDISK_NUMARGS;
    num_outop = SV_DRAWDISK_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawwheel") == 0){
    op = SV_DRAWWHEEL;
    num_op = SV_DRAWWHEEL_NUMARGS;
    num_outop = SV_DRAWWHEEL_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawcdisk") == 0){
    op = SV_DRAWCDISK;
    num_op = SV_DRAWCDISK_NUMARGS;
    num_outop = SV_DRAWCDISK_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawhexdisk") == 0){
    op = SV_DRAWHEXDISK;
    num_op = SV_DRAWHEXDISK_NUMARGS;
    num_outop = SV_DRAWHEXDISK_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawpolydisk") == 0){
    op = SV_DRAWPOLYDISK;
    num_op = SV_DRAWPOLYDISK_NUMARGS;
    num_outop = SV_DRAWPOLYDISK_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawring") == 0){
    op = SV_DRAWRING;
    num_op = SV_DRAWRING_NUMARGS;
    num_outop = SV_DRAWRING_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawnotchplate") == 0){
    op = SV_DRAWNOTCHPLATE;
    num_op = SV_DRAWNOTCHPLATE_NUMARGS;
    num_outop = SV_DRAWNOTCHPLATE_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawtrunccone") == 0){
    op = SV_DRAWTRUNCCONE;
    num_op = SV_DRAWTRUNCCONE_NUMARGS;
    num_outop = SV_DRAWTRUNCCONE_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawcone") == 0){
    op = SV_DRAWCONE;
    num_op = SV_DRAWCONE_NUMARGS;
    num_outop = SV_DRAWCONE_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawtsphere") == 0){
    op = SV_DRAWTSPHERE;
    num_op = SV_DRAWTSPHERE_NUMARGS;
    num_outop = SV_DRAWTSPHERE_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawsphere") == 0){
    op = SV_DRAWSPHERE;
    num_op = SV_DRAWSPHERE_NUMARGS;
    num_outop = SV_DRAWSPHERE_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawhsphere") == 0){
    op = SV_DRAWHSPHERE;
    num_op = SV_DRAWHSPHERE_NUMARGS;
    num_outop = SV_DRAWHSPHERE_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawtriblock") == 0){
    op = SV_DRAWTRIBLOCK;
    num_op = SV_DRAWTRIBLOCK_NUMARGS;
    num_outop = SV_DRAWTRIBLOCK_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawline") == 0){
    op = SV_DRAWLINE;
    num_op = SV_DRAWLINE_NUMARGS;
    num_outop = SV_DRAWLINE_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawpoint") == 0){
    op = SV_DRAWPOINT;
    num_op = SV_DRAWPOINT_NUMARGS;
    num_outop = SV_DRAWPOINT_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawcircle") == 0){
    op = SV_DRAWCIRCLE;
    num_op = SV_DRAWCIRCLE_NUMARGS;
    num_outop = SV_DRAWCIRCLE_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawfilledcircle") == 0){
    op = SV_DRAWFILLEDCIRCLE;
    num_op = SV_DRAWFILLEDCIRCLE_NUMARGS;
    num_outop = SV_DRAWFILLEDCIRCLE_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawarc") == 0){
    op = SV_DRAWARC;
    num_op = SV_DRAWARC_NUMARGS;
    num_outop = SV_DRAWARC_NUMOUTARGS;
  }
  else if(STRCMP(token, "drawarcdisk") == 0){
    op = SV_DRAWARCDISK;
    num_op = SV_DRAWARCDISK_NUMARGS;
    num_outop = SV_DRAWARCDISK_NUMOUTARGS;
  }
  else if(STRCMP(token, "setcolor") == 0){
    op = SV_SETCOLOR;
    num_op = SV_SETCOLOR_NUMARGS;
    num_outop = SV_SETCOLOR_NUMOUTARGS;
  }
  else if(STRCMP(token, "gettextureindex") == 0){
    op = SV_GETTEXTUREINDEX;
    num_op = SV_GETTEXTUREINDEX_NUMARGS;
    num_outop = SV_GETTEXTUREINDEX_NUMOUTARGS;
  }
  else if(STRCMP(token, "setrgb") == 0){
    op = SV_SETRGB;
    num_op = SV_SETRGB_NUMARGS;
    num_outop = SV_SETRGB_NUMOUTARGS;
  }
  else if(STRCMP(token, "setrgbval") == 0){
    op = SV_SETRGBVAL;
    num_op = SV_SETRGBVAL_NUMARGS;
    num_outop = SV_SETRGBVAL_NUMOUTARGS;
  }
  else if(STRCMP(token, "setlinewidth") == 0){
    op = SV_SETLINEWIDTH;
    num_op = SV_SETLINEWIDTH_NUMARGS;
    num_outop = SV_SETLINEWIDTH_NUMOUTARGS;
  }
  else if(STRCMP(token, "setpointsize") == 0){
    op = SV_SETPOINTSIZE;
    num_op = SV_SETPOINTSIZE_NUMARGS;
    num_outop = SV_SETPOINTSIZE_NUMOUTARGS;
  }
  else if(STRCMP(token, "setbw") == 0){
    op = SV_SETBW;
    num_op = SV_SETBW_NUMARGS;
    num_outop = SV_SETBW_NUMOUTARGS;
  }
  else if(STRCMP(token, "push") == 0){
    op = SV_PUSH;
    num_op = SV_PUSH_NUMARGS;
    num_outop = SV_PUSH_NUMOUTARGS;
  }
  else if(STRCMP(token, "pop") == 0){
    op = SV_POP;
    num_op = SV_POP_NUMARGS;
    num_outop = SV_POP_NUMOUTARGS;
  }
  else if(STRCMP(token, "include") == 0){
    op = SV_INCLUDE;
    num_op = SV_INCLUDE_NUMARGS;
    num_outop = SV_INCLUDE_NUMOUTARGS;
  }
  else if(STRCMP(token, "orienx") == 0){
    op = SV_ORIENX;
    num_op = SV_ORIENX_NUMARGS;
    num_outop = SV_ORIENX_NUMOUTARGS;
  }
  else if(STRCMP(token, "orieny") == 0){
    op = SV_ORIENY;
    num_op = SV_ORIENY_NUMARGS;
    num_outop = SV_ORIENY_NUMOUTARGS;
  }
  else if(STRCMP(token, "orienz") == 0){
    op = SV_ORIENZ;
    num_op = SV_ORIENZ_NUMARGS;
    num_outop = SV_ORIENZ_NUMOUTARGS;
  }
  else if(STRCMP(token, "randxy") == 0){
    op = SV_RANDXY;
    num_op = SV_RANDXY_NUMARGS;
    num_outop = SV_RANDXY_NUMOUTARGS;
  }
  else if(STRCMP(token, "randxz") == 0){
    op = SV_RANDXZ;
    num_op = SV_RANDXZ_NUMARGS;
    num_outop = SV_RANDXZ_NUMOUTARGS;
  }
  else if(STRCMP(token, "randyz") == 0){
    op = SV_RANDYZ;
    num_op = SV_RANDYZ_NUMARGS;
    num_outop = SV_RANDYZ_NUMOUTARGS;
  }
  else if(STRCMP(token, "randxyz") == 0){
    op = SV_RANDXYZ;
    num_op = SV_RANDXYZ_NUMARGS;
    num_outop = SV_RANDXYZ_NUMOUTARGS;
  }
  else if(STRCMP(token, "includef") == 0){
    op = SV_INCLUDEF;
    num_op = SV_INCLUDEF_NUMARGS;
    num_outop = SV_INCLUDEF_NUMOUTARGS;
  }
  else if(STRCMP(token, "abs") == 0){
    op = SV_ABS;
    num_op = SV_ABS_NUMARGS;
    num_outop = SV_ABS_NUMOUTARGS;
  }
  else if(STRCMP(token, "add") == 0){
    op = SV_ADD;
    num_op = SV_ADD_NUMARGS;
    num_outop = SV_ADD_NUMOUTARGS;
  }
  else if(STRCMP(token, "sub") == 0){
    op = SV_SUB;
    num_op = SV_SUB_NUMARGS;
    num_outop = SV_SUB_NUMOUTARGS;
  }
  else if(STRCMP(token, "mult") == 0){
    op = SV_MULT;
    num_op = SV_MULT_NUMARGS;
    num_outop = SV_MULT_NUMOUTARGS;
  }
  else if(STRCMP(token, "div") == 0){
    op = SV_DIV;
    num_op = SV_DIV_NUMARGS;
    num_outop = SV_DIV_NUMOUTARGS;
  }
  else if(STRCMP(token, "gett") == 0){
    op = SV_GETT;
    num_op = SV_GETT_NUMARGS;
    num_outop = SV_GETT_NUMOUTARGS;
  }
  else if(STRCMP(token, "multiaddt") == 0){
    op = SV_MULTIADDT;
    num_op = SV_MULTIADDT_NUMARGS;
    num_outop = SV_MULTIADDT_NUMOUTARGS;
  }
  else if(STRCMP(token, "clip") == 0){
    op = SV_CLIP;
    num_op = SV_CLIP_NUMARGS;
    num_outop = SV_CLIP_NUMOUTARGS;
  }
  else if(STRCMP(token, "clipx") == 0){
    op = SV_CLIP;
    num_op = SV_CLIP_NUMARGS;
    num_outop = SV_CLIP_NUMOUTARGS;
  }
  else if(STRCMP(token, "clipy") == 0){
    op = SV_CLIP;
    num_op = SV_CLIP_NUMARGS;
    num_outop = SV_CLIP_NUMOUTARGS;
  }
  else if(STRCMP(token, "clipz") == 0){
    op = SV_CLIP;
    num_op = SV_CLIP_NUMARGS;
    num_outop = SV_CLIP_NUMOUTARGS;
  }
  else if(STRCMP(token, "clipoff") == 0){
    op = SV_CLIP;
    num_op = SV_CLIP_NUMARGS;
    num_outop = SV_CLIP_NUMOUTARGS;
  }
  else if(STRCMP(token, "mirrorclip") == 0){
    op = SV_MIRRORCLIP;
    num_op = SV_MIRRORCLIP_NUMARGS;
    num_outop = SV_MIRRORCLIP_NUMOUTARGS;
  }
  else if(STRCMP(token, "periodicclip") == 0){
    op = SV_PERIODICCLIP;
    num_op = SV_PERIODICCLIP_NUMARGS;
    num_outop = SV_PERIODICCLIP_NUMOUTARGS;
  }
  else{
    op = SV_ERR;
    num_op = 0;
    num_outop = 0;
    return_val = 1;
  }
  *opptr = op;
  *num_opptr = num_op;
  *num_outopptr = num_outop;
  return return_val;
}

/* ------------------ GetObjectFrameTokenLoc ------------------------ */

int GetObjectFrameTokenLoc(char *var, sv_object_frame *frame){
  int i;

  for(i = 0; i < frame->nsymbols; i++){
    int ii;
    tokendata *toki;
    char *token_var;

    ii = frame->symbols[i];
    toki = frame->tokens + ii;
    token_var = toki->tokenlabel + 1;
    if(STRCMP(var, token_var) == 0) return ii;
  }
  return -1;
}

/* ----------------------- init_SMVOBJECT2 ----------------------------- */

sv_object *InitSmvObject2(object_collection *objectscoll, const char *label,
                          const char *commandsoff, const char *commandson,
                          int visible){
  sv_object *object;
  int i;

  NewMemory((void **)&object, sizeof(sv_object));
  object->select_mode = 0;
  object->used = 0;
  object->visible = visible;
  strcpy(object->label, label);
  object->nframes = 2;
  object->obj_frames = NULL;
  NewMemory((void **)&object->obj_frames,
            object->nframes * sizeof(sv_object_frame *));

  for(i = 0; i < object->nframes; i++){
    sv_object_frame *framei;
    int eof;

    if(i == 0){
      NewMemory((void **)&framei, sizeof(sv_object_frame));
      object->obj_frames[0] = framei;
      framei->error = 0;
      framei->device = object;
      ParseObjectFrame(objectscoll, commandsoff, NULL, &eof, framei);
      framei->ntextures = 0;
    }
    else{
      NewMemory((void **)&framei, sizeof(sv_object_frame));
      object->obj_frames[1] = framei;
      framei->device = object;
      ParseObjectFrame(objectscoll, commandson, NULL, &eof, framei);
      framei->error = 0;
      framei->ntextures = 0;
    }
  }
  return object;
}

/* ----------------------- InitSmvObject1 ----------------------------- */

sv_object *InitSmvObject1(object_collection *objectscoll, const char *label,
                          const char *commands, int visible){
  sv_object *object;
  sv_object_frame *framei;
  int eof;

  NewMemory((void **)&object, sizeof(sv_object));
  object->select_mode = 0;
  object->used = 0;
  object->visible = visible;
  strcpy(object->label, label);
  object->nframes = 1;
  object->obj_frames = NULL;
  NewMemory((void **)&framei, sizeof(sv_object_frame));
  NewMemory((void **)&object->obj_frames,
            object->nframes * sizeof(sv_object_frame *));

  object->obj_frames[0] = framei;
  framei->device = object;
  ParseObjectFrame(objectscoll, commands, NULL, &eof, framei);
  framei->error = 0;
  framei->ntextures = 0;
  return object;
}

/* ------------------ ClearObjectCollection ------------------------ */

void ClearObjectCollection(object_collection *objectscoll){
  sv_object *object;

  for(;;){
    object = objectscoll->object_def_last.prev;
    if(object->prev == NULL) break;
    FreeObject(object);
  }
}

/* ------------------ FreeObjectCollection ------------------------ */

void FreeObjectCollection(object_collection *objectscoll){
  ClearObjectCollection(objectscoll);
  FreeMemory(objectscoll);
}

/* ----------------------- GetSmvObject ----------------------------- */

sv_object *GetSmvObject(object_collection *objectscoll, char *label){
  sv_object *objecti, *object_start;

  object_start = objectscoll->object_def_first.next;
  objecti = object_start;
  for(; objecti->next != NULL; objecti = objecti->next){
    if(STRCMP(objecti->label, label) == 0) return objecti;
  }
  return NULL;
}

/* ----------------------- ReadObjectDefs ----------------------------- */

int ReadObjectDefs(object_collection *objectscoll, const char *file){
  FILE *stream;
  char buffer[256], *trim_buffer;
  char *buffer_ptr;
  sv_object *temp_object, *prev_object, *next_object, *current_object;
  sv_object_frame *current_frame = NULL;
  int firstdef;
  sv_object *object_start, *objecti;
  size_t lenbuffer;
  int ndevices = 0;
  int eof = 0;

  stream = fopen(file, "r");
  if(stream == NULL) return 0;

  firstdef = -1;
  buffer_ptr = NULL;
  while(!feof(stream)){
    CheckMemory;
    if(buffer_ptr == NULL){
      if(eof == 1 || fgets(buffer, 255, stream) == NULL) break;
      buffer_ptr = buffer;
    }
    RemoveComment(buffer_ptr);
    trim_buffer = TrimFront(buffer_ptr);
    lenbuffer = strlen(buffer_ptr);
    if(lenbuffer < 1){
      buffer_ptr = NULL;
      continue;
    }

    if(Match(buffer_ptr, "OBJECTDEF") == 1 ||
        Match(buffer_ptr, "AVATARDEF") == 1){
      int object_type = IS_NOT_AVATAR;
      char *label;

      sv_object_frame *first_frame, *last_frame;

      if(Match(buffer_ptr, "AVATARDEF") == 1){
        object_type = IS_AVATAR;
      }
      ndevices++;
      if(fgets(buffer, 255, stream) == NULL) break;
      RemoveComment(buffer);
      label = TrimFront(buffer);
      temp_object = GetSmvObject(objectscoll, label);
      if(temp_object != NULL){
        FreeObject(temp_object);
      }

      NewMemory((void **)&current_object, sizeof(sv_object));
      current_object->used = 0;
      current_object->select_mode = 0;
      strcpy(current_object->label, label);
      prev_object = objectscoll->object_def_last.prev;
      next_object = &objectscoll->object_def_last;

      prev_object->next = current_object;
      next_object->prev = current_object;

      current_object->next = next_object;
      current_object->prev = prev_object;
      current_object->visible = 1;

      current_object->nframes = 0;

      first_frame = &current_object->first_frame;
      last_frame = &current_object->last_frame;
      current_object->type = object_type;

      first_frame->next = last_frame;
      first_frame->prev = NULL;
      last_frame->prev = first_frame;
      last_frame->next = NULL;

      firstdef = 1;
      buffer_ptr = NULL;
      continue;
    }
    if(Match(trim_buffer, "NEWFRAME") == 1 || firstdef == 1){
      sv_object_frame *prev_frame, *next_frame;

      if(firstdef == -1) continue;
      NewMemory((void **)&current_frame, sizeof(sv_object_frame));

      next_frame = &current_object->last_frame;
      prev_frame = next_frame->prev;

      next_frame->prev = current_frame;
      prev_frame->next = current_frame;

      current_frame->next = next_frame;
      current_frame->prev = prev_frame;

      current_frame->device = current_object;
      current_object->nframes++;

      firstdef = 0;
      if(Match(trim_buffer, "NEWFRAME") == 1){
        buffer_ptr = NULL;
        continue;
      }
    }
    // If a frame has not been started, don't try and parse it.
    if(current_frame != NULL){
      buffer_ptr = ParseObjectFrame(objectscoll, buffer, stream, &eof,
                                    current_frame);
    }
  }
  fclose(stream);

  object_start = objectscoll->object_def_first.next;
  objecti = object_start;
  objectscoll->nobject_defs = 0;
  for(; objecti->next != NULL;){
    CheckMemory;
    (objectscoll->nobject_defs)++;
    objecti->obj_frames = NULL;
    if(objecti->nframes > 0){
      NewMemory((void **)&objecti->obj_frames,
                objecti->nframes * sizeof(sv_object_frame *));
    }
    objecti = objecti->next;
  }
  FREEMEMORY(objectscoll->object_defs);
  if(objectscoll->nobject_defs > 0){
    int i, j;

    NewMemory((void **)&objectscoll->object_defs,
              objectscoll->nobject_defs * sizeof(sv_object *));

    object_start = objectscoll->object_def_first.next;
    objecti = object_start;
    i = 0;
    for(; objecti->next != NULL;){
      sv_object_frame *frame_start, *framei;

      CheckMemory;
      objectscoll->object_defs[i] = objecti;
      i++;
      frame_start = objecti->first_frame.next;
      framei = frame_start;
      j = 0;
      for(; framei->next != NULL;){
        int npushpop = 0, ii;

        CheckMemory;
        objecti->obj_frames[j] = framei;
        for(ii = 0; ii < framei->ncommands; ii++){
          tokendata *command;
          int op;

          command = framei->command_list[ii];

          op = command->command;
          if(op == SV_PUSH){
            npushpop++;
          }
          else if(op == SV_POP){
            npushpop--;
            if(npushpop < 0){
              npushpop = 0;
              command->command = SV_NO_OP;
            }
          }
        }
        if(npushpop > 0){
          fprintf(
              stderr,
              "*** Error: The number of push and pop commands are not equal\n");
          framei->error = 1;
        }
        framei = framei->next;
        j++;
      }
      objecti = objecti->next;
    }
  }
  PRINTF("%d object definitions read from %s\n", ndevices, file);
  return ndevices;
}

/* ----------------------- InitAvatar ----------------------------- */

void InitAvatar(object_collection *objectscoll){
  int iavatar_types_local;
  sv_object *objecti, *object_start;
  char com_buffer[1024];
  char labels[1024];

  strcpy(labels,
         ":DUM1 :DUM2 :DUM3 :W :D :H1 :SX :SY :SZ :R :G :B :HX :HY :HZ ");

  object_start = objectscoll->object_def_first.next;
  objectscoll->navatar_types = 2;
  for(objecti = object_start; objecti->next != NULL; objecti = objecti->next){
    if(objecti->type == IS_AVATAR) objectscoll->navatar_types++;
  }
  NewMemory((void **)&objectscoll->avatar_types,
            objectscoll->navatar_types * sizeof(sv_object *));

  strcpy(com_buffer, labels);
  strcat(com_buffer, "0.0 0.0 1.0 translate 255 0 0 setrgb 0.03 0.1 drawdisk 0 "
                     "0 255 setrgb 90.0 rotatey 0.03 0.2 drawdisk");
  objectscoll->avatar_defs_backup[0] =
      InitSmvObject1(objectscoll, "Avatar_1", com_buffer, 1);
  objectscoll->avatar_defs_backup[0]->type = IS_AVATAR;

  strcpy(com_buffer, labels);
  strcat(com_buffer, "255 255 0 setrgb 0.02 0.05 drawdisk");
  objectscoll->avatar_defs_backup[1] =
      InitSmvObject1(objectscoll, "Avatar_2", com_buffer, 1);
  objectscoll->avatar_defs_backup[1]->type = IS_AVATAR;

  objectscoll->avatar_types[0] = objectscoll->avatar_defs_backup[0];
  objectscoll->avatar_types[1] = objectscoll->avatar_defs_backup[1];

  iavatar_types_local = 2;
  for(objecti = object_start; objecti->next != NULL; objecti = objecti->next){
    if(objecti->type == IS_NOT_AVATAR) continue;
    objectscoll->avatar_types[iavatar_types_local++] = objecti;
  }
}

/* ------------------ InitStdObjectDefs ------------------------ */

void InitStdObjectDefs(object_collection *objectscoll, int isZoneFireModel){
  if(isZoneFireModel == 1){
    objectscoll->std_object_defs.target_object_backup = InitSmvObject1(
        objectscoll, "target", "255 255 0 setrgb 0.02 0.05 drawdisk", 1);
  }
  else{
    objectscoll->std_object_defs.target_object_backup = InitSmvObject1(
        objectscoll, "sensor", "255 255 0 setrgb 0.038 drawcube", 1);
  }

  objectscoll->std_object_defs.thcp_object_backup = InitSmvObject1(
      objectscoll, "thcp", "255 255 0 setrgb 0.038 drawcube", 1);

  objectscoll->std_object_defs.heat_detector_object_backup = InitSmvObject2(
      objectscoll, "heat_detector", "0 255 0 setrgb 0.038 drawcube",
      "255 0 0 setrgb 0.038 drawcube", 1);

  objectscoll->std_object_defs.sprinkler_upright_object_backup = InitSmvObject2(
      objectscoll, "sprinkler_upright", "0 255 0 setrgb 0.038 drawcube",
      "255 0 0 setrgb 0.038 drawcube", 1);

  objectscoll->std_object_defs.smoke_detector_object_backup = InitSmvObject2(
      objectscoll, "smoke_detector", "127 127 127 setrgb 0.2 0.05 drawdisk",
      "255 0 0 setrgb 0.2 0.05 drawdisk", 1);

  objectscoll->std_object_defs.error_device = InitSmvObject1(
      objectscoll, "error_device",
      "255 0 0 setrgb push 45.0 rotatey -0.1 offsetz 0.05 0.2 drawdisk pop "
      "push -45.0 rotatey -0.1 offsetz 0.05 0.2 drawdisk pop",
      1);

  if(objectscoll->std_object_defs.missing_device == NULL){
    objectscoll->std_object_defs.missing_device = InitSmvObject1(
        objectscoll, "missing_device",
        "0 0 255 setrgb push 45.0 rotatey -0.1 offsetz 0.05 0.2 drawdisk "
        "pop push -45.0 rotatey -0.1 offsetz 0.05 0.2 drawdisk pop",
        1);
  }
}

/* ------------------ InitObjectCollection ------------------------ */

int InitObjectCollection(object_collection *coll) {
  // Set everything to NULL
  memset(coll, 0, sizeof(object_collection));
  strcpy(coll->object_def_first.label, "first");
  coll->object_def_first.next = &coll->object_def_last;
  coll->object_def_first.prev = NULL;

  strcpy(coll->object_def_last.label, "last");
  coll->object_def_last.next = NULL;
  coll->object_def_last.prev = &coll->object_def_first;
  coll->object_defs = NULL;
  return 0;
}

/* ------------------ CreateObjectCollection ------------------------ */

object_collection *CreateObjectCollection(void) {
  object_collection *coll;
  if(NEWMEMORY(coll, sizeof(object_collection)) == 0) return NULL;
  int ret = InitObjectCollection(coll);
  if(ret != 0) {
    FREEMEMORY(coll);
    return NULL;
  }
  else {
    return coll;
  }
}

/* ------------------ LoadDefaultObjectDefs ------------------------ */

void LoadDefaultObjectDefs(object_collection *objectscoll){
  objectscoll->nobject_defs = 4;
  FREEMEMORY(objectscoll->object_defs);
  NewMemory((void **)&objectscoll->object_defs, 4 * sizeof(sv_object *));
  objectscoll->object_defs[0] =
      objectscoll->std_object_defs.target_object_backup;
  objectscoll->object_defs[1] =
      objectscoll->std_object_defs.heat_detector_object_backup;
  objectscoll->object_defs[2] =
      objectscoll->std_object_defs.sprinkler_upright_object_backup;
  objectscoll->object_defs[3] =
      objectscoll->std_object_defs.smoke_detector_object_backup;
}

/* ----------------------- ReadDefaultObjectCollection ----------------------------- */

void ReadDefaultObjectCollection(object_collection *objectscoll,
                                 const char *fdsprefix,
                                 int isZoneFireModel){

  // There are 5 places to retrieve object definitions from:
  //
  //   1. A file within SMV root directory named "objects.svo".
  //   2. A file in the current directory named "objects.svo".
  //   3. A file in the current directory named "${fdsprefix}.svo".
  //   4. A file pointed to by SMOKEVIEW_OBJECT_DEFS_PATH.
  //   5. A file pointed to be envar SMOKEVIEW_OBJECT_DEFS.
  //
  // Last definition wins.

  // Read "objects.svo" from bin dir
  char *path = GetSmvRootFile("objects.svo");
  if(path != NULL){
    ReadObjectDefs(objectscoll, path);
    FREEMEMORY(path);
  }

  // Read "objects.svo" from the current directory.
  ReadObjectDefs(objectscoll, "objects.svo");

  // Read "${fdsprefix}.svo" from the current directory
  if(fdsprefix != NULL){
    char *objectfile;
    char *ext = ".svo";
    NEWMEMORY(objectfile, sizeof(char) * (strlen(fdsprefix) + strlen(ext) + 1));
    strcpy(objectfile, fdsprefix);
    strcat(objectfile, ext);
    ReadObjectDefs(objectscoll, objectfile);
    FREEMEMORY(objectfile);
  }

#ifdef SMOKEVIEW_OBJECT_DEFS_PATH
  // Read objects file pointed to be macro SMOKEVIEW_OBJECT_DEFS_PATH.
  // Useful when install paths differ per platform.
  ReadObjectDefs(objectscoll, SMOKEVIEW_OBJECT_DEFS_PATH);
#endif

  // Read objects file from the envar SMOKEVIEW_OBJECT_DEFS
  char *envar_object_path = getenv("SMOKEVIEW_OBJECT_DEFS");
  if(envar_object_path != NULL){
    ReadObjectDefs(objectscoll, envar_object_path);
  }

  InitAvatar(objectscoll);

  InitStdObjectDefs(objectscoll, isZoneFireModel);

  // If no objects were read, insert the 4 standard objects
  if(objectscoll->nobject_defs == 0){
    LoadDefaultObjectDefs(objectscoll);
  }
}

/* ----------------------- UpdateDeviceTextures ----------------------------- */

void UpdateDeviceTextures(object_collection *objectscoll, int ndeviceinfo,
                          devicedata *deviceinfo, int npropinfo,
                          propdata *propinfo, int *ndevice_texture_listptr,
                          int **device_texture_list_indexptr,
                          char ***device_texture_listptr){

  // create a list of device textures

  int i;
  int ndevice_texture_list=0;
  char **device_texture_list=NULL;
  int *device_texture_list_index=NULL;

  for(i = 0; i < ndeviceinfo; i++){
    devicedata *devicei;

    devicei = deviceinfo + i;

    if(devicei->object == NULL){
      devicei->object =
          GetSmvObjectType(objectscoll, devicei->deviceID,
                           objectscoll->std_object_defs.missing_device);
    }
  }

  // count device textures

  for(i = 0; i < ndeviceinfo; i++){
    devicedata *devicei;
    sv_object *object;
    int j;

    devicei = deviceinfo + i;
    object = devicei->object;
    for(j = 0; j < object->nframes; j++){
      sv_object_frame *frame;

      frame = object->obj_frames[j];
      ndevice_texture_list += frame->ntextures;
    }
  }
  for(i = 0; i < npropinfo; i++){
    propdata *propi;

    propi = propinfo + i;

    ndevice_texture_list += propi->ntextures;
  }

  // allocate data structures and fill in list

  if(ndevice_texture_list > 0){
    NewMemory((void **)&device_texture_list,
              ndevice_texture_list * sizeof(char *));
    NewMemory((void **)&device_texture_list_index,
              ndevice_texture_list * sizeof(int));
    ndevice_texture_list = 0;
    for(i = 0; i < ndeviceinfo; i++){
      devicedata *devicei;
      sv_object *object;
      int j;

      devicei = deviceinfo + i;
      object = devicei->object;
      for(j = 0; j < object->nframes; j++){
        sv_object_frame *frame;
        int k;

        frame = object->obj_frames[j];
        if(frame->ntextures == 0) continue;
        for(k = 0; k < frame->ntokens; k++){
          tokendata *toki;
          int kk;
          int dup;

          toki = frame->tokens + k;
          if(toki->type != TOKEN_TEXTURE) continue;
          dup = 0;
          for(kk = 0; kk < ndevice_texture_list; kk++){
            if(strcmp(device_texture_list[kk], toki->string) == 0){
              dup = 1;
              break;
            }
          }
          if(dup == 0)
            device_texture_list[ndevice_texture_list++] = toki->string;
        }
      }
    }
    for(i = 0; i < npropinfo; i++){
      propdata *propi;
      int j;

      propi = propinfo + i;
      if(propi->ntextures == 0) continue;
      for(j = 0; j < propi->ntextures; j++){
        int dup;
        char *texturefile;
        int kk;

        texturefile = propi->texturefiles[j];
        dup = 0;
        for(kk = 0; kk < ndevice_texture_list; kk++){
          if(strcmp(device_texture_list[kk], texturefile) == 0){
            dup = 1;
            break;
          }
        }
        if(dup == 0) device_texture_list[ndevice_texture_list++] = texturefile;
      }
    }
  }
  *ndevice_texture_listptr      = ndevice_texture_list;
  *device_texture_listptr       = device_texture_list;
  *device_texture_list_indexptr = device_texture_list_index;
}

/* ----------------------- UpdatePartClassDepend ----------------------------- */

void UpdatePartClassDepend(partclassdata *partclassi){
  int i;

  if(partclassi->prop!=NULL){
    sv_object_frame *obj_frame;
    int nvar;

    obj_frame=partclassi->prop->smv_object->obj_frames[0];
    for(i=0;i<partclassi->nvars_dep-3;i++){
      char *var;

      var=partclassi->vars_dep[i];
      partclassi->vars_dep_index[i]= GetObjectFrameTokenLoc(var,obj_frame);
    }
    nvar = partclassi->nvars_dep;
    partclassi->vars_dep_index[nvar-3]= GetObjectFrameTokenLoc("R",obj_frame);
    partclassi->vars_dep_index[nvar-2]= GetObjectFrameTokenLoc("G",obj_frame);
    partclassi->vars_dep_index[nvar-1]= GetObjectFrameTokenLoc("B",obj_frame);
  }
}

/* ----------------------- GetNDevices ----------------------------- */
#define BUFFER_LEN 255
int GetNDevices(char *file){
  FILE *stream;
  char buffer[BUFFER_LEN], *comma;
  int buffer_len = BUFFER_LEN, nd = 0;

  if(file == NULL) return 0;
  stream = fopen(file, "r");
  if(stream == NULL) return 0;
  fgets(buffer, buffer_len, stream);
  comma = strchr(buffer, ',');
  if(comma != NULL) *comma = 0;
  TrimBack(buffer);
  if(strcmp(buffer, "//HEADER") != 0){
    fclose(stream);
    return 0;
  }

  while(!feof(stream)){
    fgets(buffer, buffer_len, stream);
    comma = strchr(buffer, ',');
    if(comma != NULL) *comma = 0;
    TrimBack(buffer);
    if(strcmp(buffer, "//DATA") == 0){
      break;
    }
    if(strcmp(buffer, "DEVICE") == 0){
      nd++;
    }
  }
  fclose(stream);
  return nd;
}
