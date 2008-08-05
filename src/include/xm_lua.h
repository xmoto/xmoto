extern "C" {
  #if defined(WIN32) 
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
  #else
    #if defined(HAVE_LUA5_1_LUA_H)
      #include <lua5.1/lua.h>
      #include <lua5.1/lauxlib.h>
      #include <lua5.1/lualib.h>
    #elif defined(HAVE_LUA51_LUA_H)
      #include <lua51/lua.h>
      #include <lua51/lauxlib.h>
      #include <lua51/lualib.h>      
    #elif defined(HAVE_LUA50_LUA_H)
      #include <lua50/lua.h>
      #include <lua50/lauxlib.h>
      #include <lua50/lualib.h>
    #elif defined(HAVE_LUA_LUA_H)
      #include <lua/lua.h>
      #include <lua/lauxlib.h>
      #include <lua/lualib.h>
    #elif defined(HAVE_LUA_H)
      #include <lua.h>
      #include <lualib.h>
      #include <lauxlib.h>
    #elif defined(CMAKE_LUA_H)
      #include "lua.h"
      #include "lualib.h"
      #include "lauxlib.h"
    #else
      #error Missing lua
    #endif
  #endif
}
