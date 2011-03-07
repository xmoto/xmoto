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

#ifndef __LUALIBBASE_H__
#define __LUALIBBASE_H__

#include <string>
#include "include/xm_lua.h"

class LuaLibBase {
public:
  LuaLibBase(const std::string& i_libname, luaL_reg i_reg[]);
  ~LuaLibBase();

  void loadScriptFile(const std::string& i_scriptFilename);
  void loadScript(const std::string& i_scriptCode, const std::string& i_scriptFilename);
  std::string getErrorMsg();

  bool scriptCallBool(const std::string& FuncName, bool bDefault=false);
  void scriptCallVoid(const std::string& FuncName);
  void scriptCallTblVoid(const std::string& Table, const std::string& FuncName);
  void scriptCallTblVoid(const std::string& Table, const std::string& FuncName, int n);
  void scriptCallVoidNumberArg(const std::string& FuncName, int n);
  void scriptCallVoidNumberArg(const std::string& FuncName, int n1, int n2);

protected:
  // lua requires static values due to the static functions. So, set the instance used if needed.
  // setInstance must be set before lua call
  virtual void setInstance() = 0;

  /* get the number of arguments */
  static int args_numberOfArguments(lua_State *pL);

  /* arguments checks ; throw exception on failure */
  static void args_CheckNumberOfArguments(lua_State *pL, int i_from, int i_to = -1);

  static lua_Number X_luaL_check_number(lua_State *L,int narg);

private:
  lua_State *m_pL;
};


#endif
