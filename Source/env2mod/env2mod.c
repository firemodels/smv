#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "env2mod.h"
#include "datadefs.h"
#include "string_util.h"
#include "dmalloc.h"

/* ------------------ NextLine ------------------------ */

char *NextLine(char *buffer, int len, FILE *stream){
  char *read;

  for(;;){
    read = fgets(buffer, len, stream);
    if(read==NULL)return NULL;
    if(buffer[0] == '#')continue;
    if(strchr(buffer, '=') == NULL)continue;
    return buffer;
  }
}

/* ------------------ Split ------------------------ */

void Split(char *buffer, char **keyptr, char **valptr){
  char *eq,*key,*val;

  *keyptr = NULL;
  *valptr = NULL;
  if(buffer==NULL)return;
  eq = strchr(buffer, '=');
  if(eq==NULL)return;

  key = buffer;
  val = eq+1;
  eq[0] = 0;
  key = TrimFrontBack(key);
  val = TrimFrontBack(val);
  *keyptr = key;
  *valptr = val;
}

/* ------------------ OutputPath ------------------------ */

void OutputPath(FILE *stream, int list_type, char *key, char *values){
  char *entry,delim[2];
  char *entrylist[ENV2MOD_MAXLIST];
  int nlist = 0;

  if(values==NULL||strlen(values) == 0)return;
  if(list_type!=ENV2MOD_PREPEND&&list_type!=ENV2MOD_APPEND)return;

  // need to construct a list because prepend-path entries need to be outputted in reverse order

  strcpy(delim, ":");
  entry = strtok(values, delim);
  while(entry != NULL){
    if(strlen(entry)>0){
      entrylist[nlist] = entry;
      nlist++;
    }
    entry = strtok(NULL, delim);
  }
  if(list_type==ENV2MOD_APPEND){
    int i;

    for(i = 0;i<nlist;i++){

      entry = entrylist[i];
      fprintf(stream, "   append-path %s %s\n", key, entry);
    }
  }
  else if(list_type==ENV2MOD_PREPEND){
    int i;

    for(i = 0;i<nlist;i++){
      entry = entrylist[nlist-1-i];
      fprintf(stream, "   prepend-path %s %s\n", key, entry);
    }
  }
}

/* ------------------ CreateModule ------------------------ */

int CreateModule(char *left_file, char* right_file, char *module_file){
  FILE *stream_left = NULL, *stream_right = NULL, *stream_module = NULL;
  char buffer_left[LEN_BUFFER], buffer_right[LEN_BUFFER];
  char *read_left, *read_right;
  char *key_left = NULL, *key_right = NULL;
  char *val_left = NULL, *val_right = NULL;

  if(left_file == NULL || strlen(left_file) == 0)return -1;
  if(right_file == NULL || strlen(right_file) == 0)return -1;
  if(module_file == NULL || strlen(module_file) == 0)return -1;

  stream_left = FOPEN(left_file, "r");
  if(stream_left == NULL)return -1;

  stream_right = FOPEN(right_file, "r");
  if(stream_right == NULL){
    fclose(stream_left);
    return -1;
  }

  if(strcmp(module_file, "-") == 0){
    stream_module = stdout;
  }
  else{
    stream_module = FOPEN(module_file, "w");
  }
  if(stream_module==NULL){
    fclose(stream_left);
    fclose(stream_right);
    return -1;
  }

  fprintf(stream_module,"#%%Module####################################\n");
  fprintf(stream_module,"\n");
  fprintf(stream_module,"# put name of module here\n\n");
  fprintf(stream_module,"proc ModulesHelp { } {\n");
  fprintf(stream_module,"    global helpmsg\n");
  fprintf(stream_module,"    puts stderr \"\\t$helpmsg\\n\"\n");
  fprintf(stream_module,"}\n\n");

  fprintf(stream_module,"#set home path_to_module\n\n");
  fprintf(stream_module,"#if [ file isdirectory $home ] {\n");
  fprintf(stream_module,"#   module-whatis   \"Sets up your enviornment to use $home\"\n");
  fprintf(stream_module,"#   set helpmsg \"Sets up your environment to use $home\"\n\n");

  read_left = NextLine(buffer_left, LEN_BUFFER, stream_left);
  if(read_left != NULL)Split(read_left, &key_left, &val_left);

  read_right = NextLine(buffer_right, LEN_BUFFER, stream_right);
  if(read_right!=NULL)Split(read_right, &key_right, &val_right);

  while(read_right!=NULL){
    int key_compare;

    if(read_left == NULL){
      key_compare = 1;
    }
    else{
      key_compare = strcmp(key_left, key_right);
    }

    if(key_compare<0){
      read_left = NextLine(buffer_left, LEN_BUFFER, stream_left);
      if(read_left != NULL)Split(read_left, &key_left, &val_left);
    }
    else if(key_compare==0){
      if(strcmp(val_left, val_right)!=0){
        char *match;

        match=strstr(val_right, val_left);
        if(match==NULL){
          if(strlen(val_right)>0){
            if(strstr(key_right, "PATH") != NULL){
              OutputPath(stream_module, ENV2MOD_PREPEND, key_right, val_right);
            }
            else{
              fprintf(stream_module, "   setenv %s %s\n", key_right, val_right);
            }
          }
        }
        else{
          char *prepend_string, *append_string_local;

          prepend_string = val_right;
          append_string_local = match+strlen(val_left);
          match[0] = 0;

          prepend_string = TrimFrontBack(prepend_string);
          OutputPath(stream_module, ENV2MOD_PREPEND, key_right, prepend_string);

          append_string_local = TrimFrontBack(append_string_local);
          OutputPath(stream_module, ENV2MOD_APPEND, key_right, append_string_local);
        }
      }
      read_left = NextLine(buffer_left, LEN_BUFFER, stream_left);
      if(read_left != NULL)Split(read_left, &key_left, &val_left);

      read_right = NextLine(buffer_right, LEN_BUFFER, stream_right);
      if(read_right != NULL)Split(read_right, &key_right, &val_right);
    }
    else{
      if(strlen(val_right)>0){
        if(strstr(key_right, "PATH") != NULL){
          OutputPath(stream_module, ENV2MOD_PREPEND, key_right, val_right);
        }
        else{
          fprintf(stream_module, "   setenv %s %s\n", key_right, val_right);
        }
      }
      read_right = NextLine(buffer_right, LEN_BUFFER, stream_right);
      if(read_right != NULL)Split(read_right, &key_right, &val_right);
    }
  }
  fprintf(stream_module,"#} else {\n");
  fprintf(stream_module,"#   module-whatis \"directory $home does NOT exist\"\n");
  fprintf(stream_module,"#   set helpmsg \"directory $home does NOT exist\"\n");
  fprintf(stream_module,"#}\n");

  fclose(stream_left);
  fclose(stream_right);
  fclose(stream_module);
  return 0;
}

/* ------------------ OutputScriptPath ------------------------ */

void OutputScriptPath(FILE *stream, int list_type, char *key, char *values){
  char *entry,delim[2],dollar[2];
  char *entrylist[ENV2MOD_MAXLIST];
  int nlist = 0;

  if(values==NULL||strlen(values) == 0)return;
  if(list_type!=ENV2MOD_PREPEND&&list_type!=ENV2MOD_APPEND)return;

  // need to construct a list because prepend-path entries need to be outputted in reverse order

  strcpy(delim, ":");
  strcpy(dollar,"$");
  entry = strtok(values, delim);
  while(entry != NULL){
    if(strlen(entry)>0){
      entrylist[nlist] = entry;
      nlist++;
    }
    entry = strtok(NULL, delim);
  }
  if(list_type==ENV2MOD_APPEND){
    int i;

    for(i = 0;i<nlist;i++){

      entry = entrylist[i];
      fprintf(stream, "   %s=%s%s:%s\n", key, dollar,key,entry);
    }
  }
  else if(list_type==ENV2MOD_PREPEND){
    int i;

    for(i = 0;i<nlist;i++){
      entry = entrylist[nlist-1-i];
      fprintf(stream, "   %s=%s:%s%s\n", key, entry,dollar,key);
    }
  }
}

/* ------------------ CreateScript ------------------------ */

int CreateScript(char *left_file, char* right_file, char *module_file){
  FILE *stream_left = NULL, *stream_right = NULL, *stream_module = NULL;
  char buffer_left[LEN_BUFFER], buffer_right[LEN_BUFFER];
  char *read_left, *read_right;
  char *key_left = NULL, *key_right = NULL;
  char *val_left = NULL, *val_right = NULL;

  if(left_file == NULL || strlen(left_file) == 0)return -1;
  if(right_file == NULL || strlen(right_file) == 0)return -1;
  if(module_file == NULL || strlen(module_file) == 0)return -1;

  stream_left = FOPEN(left_file, "r");
  if(stream_left == NULL)return -1;

  stream_right = FOPEN(right_file, "r");
  if(stream_right == NULL){
    fclose(stream_left);
    return -1;
  }

  if(strcmp(module_file, "-") == 0){
    stream_module = stdout;
  }
  else{
    stream_module = FOPEN(module_file, "w");
  }
  if(stream_module==NULL){
    fclose(stream_left);
    fclose(stream_right);
    return -1;
  }

  fprintf(stream_module,"#!/bin/bash\n");
  fprintf(stream_module,"\n");
  fprintf(stream_module,"#home=path_to_module\n\n");
  fprintf(stream_module,"#if [ -e $home ]; then\n");

  read_left = NextLine(buffer_left, LEN_BUFFER, stream_left);
  if(read_left != NULL)Split(read_left, &key_left, &val_left);

  read_right = NextLine(buffer_right, LEN_BUFFER, stream_right);
  if(read_right!=NULL)Split(read_right, &key_right, &val_right);

  while(read_right!=NULL){
    int key_compare;

    if(read_left == NULL){
      key_compare = 1;
    }
    else{
      key_compare = strcmp(key_left, key_right);
    }

    if(key_compare<0){
      read_left = NextLine(buffer_left, LEN_BUFFER, stream_left);
      if(read_left != NULL)Split(read_left, &key_left, &val_left);
    }
    else if(key_compare==0){
      if(strcmp(val_left, val_right)!=0){
        char *match;

        match=strstr(val_right, val_left);
        if(match==NULL){
          if(strlen(val_right)>0){
            if(strstr(key_right, "PATH") != NULL){
              OutputScriptPath(stream_module, ENV2MOD_PREPEND, key_right, val_right);
            }
            else{
              fprintf(stream_module, "   %s=%s\n", key_right, val_right);
            }
          }
        }
        else{
          char *prepend_string, *append_string_local;

          prepend_string = val_right;
          append_string_local = match+strlen(val_left);
          match[0] = 0;

          prepend_string = TrimFrontBack(prepend_string);
          OutputScriptPath(stream_module, ENV2MOD_PREPEND, key_right, prepend_string);

          append_string_local = TrimFrontBack(append_string_local);
          OutputScriptPath(stream_module, ENV2MOD_APPEND, key_right, append_string_local);
        }
      }
      read_left = NextLine(buffer_left, LEN_BUFFER, stream_left);
      if(read_left != NULL)Split(read_left, &key_left, &val_left);

      read_right = NextLine(buffer_right, LEN_BUFFER, stream_right);
      if(read_right != NULL)Split(read_right, &key_right, &val_right);
    }
    else{
      if(strlen(val_right)>0){
        if(strstr(key_right, "PATH") != NULL){
          OutputScriptPath(stream_module, ENV2MOD_PREPEND, key_right, val_right);
        }
        else{
          fprintf(stream_module, "   %s=%s\n", key_right, val_right);
        }
      }
      read_right = NextLine(buffer_right, LEN_BUFFER, stream_right);
      if(read_right != NULL)Split(read_right, &key_right, &val_right);
    }
  }
  fprintf(stream_module,"#fi\n");

  fclose(stream_left);
  fclose(stream_right);
  fclose(stream_module);
  return 0;
}
