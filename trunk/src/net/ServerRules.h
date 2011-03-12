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

#ifndef __SERVERRULES_H__
#define __SERVERRULES_H__

#include "../LuaLibBase.h"

class ServerThread;

class ServerRules : public LuaLibBase {
public:
  ServerRules(ServerThread* i_st);
  ~ServerRules();

protected:
  void setInstance();

private:
  ServerThread* m_server;

  static luaL_reg m_rulesFuncs[];
  static ServerThread* m_exec_server;

  /* Lua library prototypes */
  // system
  static int L_Rules_Log(lua_State *pL);

  // global
  static int L_Rules_player_setPoints(lua_State *pL);
  static int L_Rules_player_addPoints(lua_State *pL);

  // on a round
  static int L_Rules_Round_whenRound_new(lua_State *pL);
  static int L_Rules_Round_whenPlayer_takesEntityToTake(lua_State *pL);
  static int L_Rules_Round_whenPlayer_wins(lua_State *pL);
  static int L_Rules_Round_whenPlayer_dies(lua_State *pL);
  static int L_Rules_Round_getTime(lua_State *pL);
  static int L_Rules_Round_getNbRemainingEntitiesToTake(lua_State *pL);
};

#endif
