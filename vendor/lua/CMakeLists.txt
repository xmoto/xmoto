cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(lua C)

add_library(lua STATIC
  lua/lapi.c lua/lapi.h
  lua/lauxlib.c lua/lauxlib.h
  lua/lbaselib.c
  lua/lcode.c lua/lcode.h
  lua/ldblib.c
  lua/ldebug.c lua/ldebug.h
  lua/ldo.c lua/ldo.h
  lua/ldump.c
  lua/lfunc.c lua/lfunc.h
  lua/lgc.c lua/lgc.h
  lua/linit.c
  lua/liolib.c
  lua/llex.c lua/llex.h
  lua/llimits.h
  lua/lmathlib.c
  lua/lmem.c lua/lmem.h
  lua/loadlib.c
  lua/lobject.c lua/lobject.h
  lua/lopcodes.c lua/lopcodes.h
  lua/loslib.c
  lua/lparser.c lua/lparser.h
  lua/lstate.c lua/lstate.h
  lua/lstring.c lua/lstring.h
  lua/lstrlib.c
  lua/ltable.c lua/ltable.h
  lua/ltablib.c
  lua/ltm.c lua/ltm.h
  # lua/lua.c
  # lua/luac.c
  lua/luaconf.h
  lua/lua.h
  lua/lualib.h
  lua/lundump.c lua/lundump.h
  lua/lvm.c lua/lvm.h
  lua/lzio.c lua/lzio.h
  lua/print.c
)
target_include_directories(lua PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)
