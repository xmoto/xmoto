extern "C" {
  #if defined(HAVE_LUA5_1_LUA_H)
    #include <lua5.1/lua.h>
    #include <lua5.1/lauxlib.h>
    #include <lua5.1/lualib.h>
  #elif defined(HAVE_LUA51_LUA_H)
    #include <lua51/lua.h>
    #include <lua51/lauxlib.h>
    #include <lua51/lualib.h>
  #else
    #include "lua_vendored/lua.h"
    #include "lua_vendored/lualib.h"
    #include "lua_vendored/lauxlib.h"
  #endif
}
