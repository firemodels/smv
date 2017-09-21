#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "env2mod.h"
#include "datadefs.h"
#include "string_util.h"
#include "MALLOC.h"

/* ------------------ NextLine ------------------------ */

char *NextLine(char *buffer, int len, FILE *stream){
  char *read;

  for(;;){
    read = fgets(buffer, len, stream);
    if(read==NULL)return NULL;
    if(buffer[0] == '#')continue;
    return buffer;
  }
}

/* ------------------ CreateModule ------------------------ */

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

/* ------------------ CreateModule ------------------------ */

int CreateModule(char *left_file, char* right_file, char *module_file){
  FILE *stream_left = NULL, *stream_right = NULL, *stream_module = NULL;
  char buffer_left[LEN_BUFFER], buffer_right[LEN_BUFFER];
  char *read_left, *read_right;

  if(left_file==NULL||strlen(left_file)==0)return -1;
  if(right_file==NULL||strlen(right_file)==0)return -1;
  if(module_file==NULL||strlen(module_file)==0)return -1;

  stream_left = fopen(left_file, "r");
  if(stream_left==NULL)return -1;
  stream_right = fopen(left_file, "r");
  if(stream_right==NULL){
    fclose(stream_left);
    return -1;
  }
  stream_module = fopen(module_file, "w");
  if(stream_module==NULL){
    fclose(stream_left);
    fclose(stream_right);
    return -1;
  }

  fprintf(stream_module, "#%Module\n");

  read_left = NextLine(buffer_left, LEN_BUFFER, stream_left);
  read_right = NextLine(buffer_right, LEN_BUFFER, stream_right);
  while(read_left!=NULL&&read_right!=NULL){
    int key_compare;
    char *key_left, *key_right;
    char *val_left, *val_right;

    Split(read_left, &key_left, &val_left);
    Split(read_right, &key_right, &val_right);

    key_compare = strcmp(key_left, key_right);
    if(key_compare<0){
      read_left = NextLine(buffer_left, LEN_BUFFER, stream_left);
    }
    else if(key_compare==0){
      if(strcmp(val_left, val_right)!=0){
        char *match;

        match=strstr(val_right, val_left);
        if(match==NULL){
          fprintf(stream_module, "setenv %s %s\n", key_right, val_right);
        }
        else{
          char *prepend_string, *append_string;

          prepend_string = val_left;
          append_string = match+strlen(val_left);
          match[0] = 0;

          prepend_string = TrimFrontBack(prepend_string);
          if(strlen(prepend_string)>0)fprintf(stream_module, "prepend-path %s %s\n", key_right, prepend_string);

          append_string = TrimFrontBack(append_string);
          if(strlen(append_string)>0)fprintf(stream_module, "append-path %s %s\n", key_right, append_string);
        }
      }
      read_left = NextLine(buffer_left, LEN_BUFFER, stream_left);
      read_right = NextLine(buffer_right, LEN_BUFFER, stream_right);
    }
    else{
      read_right = NextLine(buffer_right, LEN_BUFFER, stream_right);
    }
  }
  fclose(stream_left);
  fclose(stream_right);
  fclose(stream_module);
  return 0;
}

