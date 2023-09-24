#include "lua.h"

int lua_render(lua_State *L);
lua_State *initLua();
int RunLuaBranch(lua_State *L, int argc, char **argv);

int lua_initsmvproginfo(lua_State *L);
int lua_initsmvdata(lua_State *L);
void addLuaPaths(lua_State *L);

int load_script(const char *filename);
int loadLuaScript(const char *filename);
int runLuaScript();
int loadSSFScript(const char *filename);
int runSSFScript();

int runScriptString(const char *string);
int lua_get_sliceinfo(lua_State *L);
int lua_get_csvinfo(lua_State *L);
int lua_initsmvdata(lua_State *L);
#if LUA_VERSION_NUM < 502
/* macro to avoid warnings about unused variables */
#if !defined(UNUSED)
#define UNUSED(x) ((void)(x))
#endif
#include <lauxlib.h>
#define LUAL_NUMSIZES (sizeof(lua_Integer) * 16 + sizeof(lua_Number))
LUALIB_API void(luaL_checkversion_)(lua_State *L, lua_Number ver, size_t sz);
#define luaL_checkversion(L)                                                   \
  luaL_checkversion_(L, LUA_VERSION_NUM, LUAL_NUMSIZES)
LUALIB_API void(luaL_setfuncs)(lua_State *L, const luaL_Reg *l, int nup);
#define LUA_OK 0
#define luaL_newlibtable(L, l)                                                 \
  lua_createtable(L, 0, sizeof(l) / sizeof((l)[0]) - 1)
#define luaL_newlib(L, l)                                                      \
  (luaL_checkversion(L), luaL_newlibtable(L, l), luaL_setfuncs(L, l, 0))
#endif
#if LUA_VERSION_NUM < 504
#define LUA_OK 0
#endif
