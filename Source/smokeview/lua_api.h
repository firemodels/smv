#include "lua.h"

int lua_render(lua_State  *L);
lua_State *initLua();

int load_script(char *filename);
int loadLuaScript(char *filename);
int runLuaScript();
int loadSSFScript(char *filename);
int runSSFScript();

void runScriptString(char *string);
int lua_get_sliceinfo(lua_State *L);
int lua_get_csvinfo(lua_State *L);
int lua_initsmvdata(lua_State *L);
