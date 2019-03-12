#ifndef ENV2MOD_H_DEFINED
#define ENV2MOD_H_DEFINED

#define LEN_BUFFER 1024
#define ENV2MOD_MAXLIST 1000
#define ENV2MOD_PREPEND 0
#define ENV2MOD_APPEND 1


EXTERNCPP int CreateModule(char *left_file, char* right_file, char *module_file);
EXTERNCPP int CreateScript(char *left_file, char* right_file, char *module_file);

#endif
