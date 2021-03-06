/*=============================================================================
XMOTO

This file is part of XMOTO.

XMOTO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

XMOTO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XMOTO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
=============================================================================*/

#include "LuaLibBase.h"
#include "common/VFileIO.h"
#include "helpers/VExcept.h"

lua_Number LuaLibBase::X_luaL_check_number(lua_State *L, int narg) {
  lua_Number d = lua_tonumber(L, narg);
  if (d == 0) /* avoid extra test when d is not 0 */
    luaL_checktype(L, narg, LUA_TNUMBER);
  return d;
}

LuaLibBase::LuaLibBase(const std::string &i_libname, luaL_Reg i_reg[]) {
  m_pL = luaL_newstate();

#if LUA_VERSION_NUM < 502
  luaopen_base(m_pL);
  luaopen_math(m_pL);
  luaopen_table(m_pL);
#else
  luaL_requiref(m_pL, "_G", luaopen_base, 1);
  luaL_requiref(m_pL, LUA_MATHLIBNAME, luaopen_math, 1);
  luaL_requiref(m_pL, LUA_TABLIBNAME, luaopen_table, 1);
#endif

#if HAVE_LUAL_OPENLIB
  luaL_openlib(m_pL, i_libname.c_str(), i_reg, 0);
#else // HAVE_LUAL_OPENLIB
  lua_newtable(m_pL);

#if LUA_VERSION_NUM >= 502
  luaL_setfuncs(m_pL, i_reg, 0);
#else // LUA_VERSION_NUM >= 502
  luaL_register(m_pL, i_libname.c_str(), i_reg);
#endif // LUA_VERSION_NUM >= 502

  lua_setglobal(m_pL, i_libname.c_str());
#endif // HAVE_LUAL_OPENLIB
}

LuaLibBase::~LuaLibBase() {
  lua_close(m_pL);
}

/*===========================================================================
  Simple lua interaction
  ===========================================================================*/
bool LuaLibBase::scriptCallBool(const std::string &FuncName, bool bDefault) {
  setInstance();

  bool bRet = bDefault;

  /* Fetch global function */
  lua_getglobal(m_pL, FuncName.c_str());

  /* Is it really a function and not just a pile of ****? */
  if (lua_isfunction(m_pL, -1)) {
    /* Call! */
    if (lua_pcall(m_pL, 0, 1, 0) != 0) {
      throw Exception("failed to invoke (bool) " + FuncName +
                      std::string("(): ") +
                      std::string(lua_tostring(m_pL, -1)));
    }

    /* Retrieve return value */
    if (!lua_toboolean(m_pL, -1)) {
      bRet = false;
    } else {
      bRet = true;
    }
  }

  /* Reset Lua VM */
  lua_settop(m_pL, 0);

  return bRet;
}

void LuaLibBase::scriptCallVoid(const std::string &FuncName) {
  setInstance();

  /* Fetch global function */
  lua_getglobal(m_pL, FuncName.c_str());

  /* Is it really a function and not just a pile of ****? */
  if (lua_isfunction(m_pL, -1)) {
    /* Call! */
    if (lua_pcall(m_pL, 0, 0, 0) != 0) {
      throw Exception("failed to invoke (void) " + FuncName +
                      std::string("(): ") +
                      std::string(lua_tostring(m_pL, -1)));
    }
  }

  /* Reset Lua VM */
  lua_settop(m_pL, 0);
}

void LuaLibBase::scriptCallVoidNumberArg(const std::string &FuncName, int n) {
  setInstance();

  /* Fetch global function */
  lua_getglobal(m_pL, FuncName.c_str());

  /* Is it really a function and not just a pile of ****? */
  if (lua_isfunction(m_pL, -1)) {
    /* Call! */
    lua_pushnumber(m_pL, n);
    if (lua_pcall(m_pL, 1, 0, 0) != 0) {
      throw Exception("failed to invoke (void) " + FuncName +
                      std::string("(): ") +
                      std::string(lua_tostring(m_pL, -1)));
    }
  }

  /* Reset Lua VM */
  lua_settop(m_pL, 0);
}

void LuaLibBase::scriptCallVoidNumberArg(const std::string &FuncName,
                                         int n1,
                                         int n2) {
  setInstance();

  /* Fetch global function */
  lua_getglobal(m_pL, FuncName.c_str());

  /* Is it really a function and not just a pile of ****? */
  if (lua_isfunction(m_pL, -1)) {
    /* Call! */
    lua_pushnumber(m_pL, n1);
    lua_pushnumber(m_pL, n2);
    if (lua_pcall(m_pL, 2, 0, 0) != 0) {
      throw Exception("failed to invoke (void) " + FuncName +
                      std::string("(): ") +
                      std::string(lua_tostring(m_pL, -1)));
    }
  }

  /* Reset Lua VM */
  lua_settop(m_pL, 0);
}

void LuaLibBase::scriptCallTblVoid(const std::string &Table,
                                   const std::string &FuncName) {
  setInstance();

  /* Fetch global table */
  lua_getglobal(m_pL, Table.c_str());

  //    printf("[%s.%s]\n",Table.c_str(),FuncName.c_str());

  if (lua_istable(m_pL, -1)) {
    lua_pushstring(m_pL, FuncName.c_str());
    lua_gettable(m_pL, -2);

    if (lua_isfunction(m_pL, -1)) {
      /* Call! */
      if (lua_pcall(m_pL, 0, 0, 0) != 0) {
        throw Exception("failed to invoke (tbl,void) " + Table +
                        std::string(".") + FuncName + std::string("(): ") +
                        std::string(lua_tostring(m_pL, -1)));
      }
    }
  }

  /* Reset Lua VM */
  lua_settop(m_pL, 0);
}

void LuaLibBase::scriptCallTblVoid(const std::string &Table,
                                   const std::string &FuncName,
                                   int n) {
  setInstance();

  /* Fetch global table */
  lua_getglobal(m_pL, Table.c_str());

  //    printf("[%s.%s]\n",Table.c_str(),FuncName.c_str());

  if (lua_istable(m_pL, -1)) {
    lua_pushstring(m_pL, FuncName.c_str());
    lua_gettable(m_pL, -2);

    if (lua_isfunction(m_pL, -1)) {
      /* Call! */
      lua_pushnumber(m_pL, n);
      if (lua_pcall(m_pL, 1, 0, 0) != 0) {
        throw Exception("failed to invoke (tbl,void) " + Table +
                        std::string(".") + FuncName + std::string("(): ") +
                        std::string(lua_tostring(m_pL, -1)));
      }
    }
  }

  /* Reset Lua VM */
  lua_settop(m_pL, 0);
}

void LuaLibBase::loadScriptFile(const std::string &i_scriptFilename) {
  FileHandle *pfh = XMFS::openIFile(FDT_DATA, i_scriptFilename);

  if (pfh == NULL) {
    throw Exception("unable to load script " + i_scriptFilename);
  }

  std::string Line, ScriptBuf = "";

  try {
    while (XMFS::readNextLine(pfh, Line)) {
      if (Line.length() > 0) {
        ScriptBuf.append(Line.append("\n"));
      }
    }
  } catch (Exception &e) {
    XMFS::closeFile(pfh);
    throw e;
  }

  XMFS::closeFile(pfh);
  loadScript(ScriptBuf, i_scriptFilename);
}

void LuaLibBase::loadScript(const std::string &i_scriptCode,
                            const std::string &i_scriptFilename) {
  /* Use the Lua aux lib to load the buffer */
  int nRet;

  setInstance();

  nRet = luaL_loadbuffer(m_pL,
                         i_scriptCode.c_str(),
                         i_scriptCode.length(),
                         i_scriptFilename.c_str()) ||
         lua_pcall(m_pL, 0, 0, 0);

  /* Returned WHAT? */
  if (nRet != 0) {
    throw Exception("failed to load level script");
  }
}

std::string LuaLibBase::getErrorMsg() {
  const char *msg = lua_tostring(m_pL, -1);
  if (msg == NULL) {
    return "No error available";
  }
  return std::string(msg);
}

/*=====================================================
            Script Helpers
=====================================================*/

int LuaLibBase::args_numberOfArguments(lua_State *pL) {
  return lua_gettop(pL);
}

void LuaLibBase::args_CheckNumberOfArguments(lua_State *pL,
                                             int i_from,
                                             int i_to) {
  int n = args_numberOfArguments(pL);

  if (i_to == -1) {
    if (n != i_from) {
      luaL_error(pL, "Invalid number of arguments");
    }
  } else {
    if (n < i_from || n > i_to) {
      luaL_error(pL, "Invalid number of arguments");
    }
  }
}
