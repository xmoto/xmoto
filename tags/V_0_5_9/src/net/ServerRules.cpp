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

#include "ServerRules.h"
#include "../helpers/Log.h"
#include "../Universe.h"
#include "thread/ServerThread.h"

luaL_reg ServerRules::m_rulesFuncs[] = {
    {"Log",              	     ServerRules::L_Rules_Log},
    {"Player_setPoints", 	     ServerRules::L_Rules_player_setPoints},
    {"Player_addPoints", 	     ServerRules::L_Rules_player_addPoints},
    {"SendPointsToPlayers",          ServerRules::L_Rules_sendPointsToPlayers},
    {"GetTime",          	     ServerRules::L_Rules_Round_getTime},
    {"GetNbRemainingEntitiesToTake", ServerRules::L_Rules_Round_getNbRemainingEntitiesToTake},
    {NULL, NULL}
};

ServerThread* ServerRules::m_exec_server = NULL;

ServerRules::ServerRules(ServerThread* i_st) : LuaLibBase("Rules", m_rulesFuncs) {
  m_server = i_st;
}

ServerRules::~ServerRules() {
}

void ServerRules::setInstance() {
  m_exec_server = m_server;
}

/* rules functions */

int ServerRules::L_Rules_Log(lua_State *pL) {
  std::string Out = "Server rules: ";

  for(int i=0; i<args_numberOfArguments(pL); i++)
    Out.append(luaL_checkstring(pL, i+1));
  LogInfo((char *)Out.c_str());

  return 0;    
}

int ServerRules::L_Rules_player_setPoints(lua_State *pL) {
  int v_playerId;
  int v_points;

  args_CheckNumberOfArguments(pL, 2);
  v_playerId = (int) luaL_checknumber(pL,1);
  v_points   = (int) luaL_checknumber(pL,2);
  m_exec_server->getNetSClientById(v_playerId)->setPoints(v_points);
  return 0;
}

int ServerRules::L_Rules_player_addPoints(lua_State *pL) {
  int v_playerId;
  int v_points;

  args_CheckNumberOfArguments(pL, 2);
  v_playerId = (int) luaL_checknumber(pL,1);
  v_points   = (int) luaL_checknumber(pL,2);
  m_exec_server->getNetSClientById(v_playerId)->addPoints(v_points);
  return 0;
}

int ServerRules::L_Rules_sendPointsToPlayers(lua_State *pL) {
  m_exec_server->sendPointsToSlavePlayers();
  return 0;
}

int ServerRules::L_Rules_Round_getTime(lua_State *pL) {
  args_CheckNumberOfArguments(pL, 0);

  Universe* v_universe;
  v_universe = m_exec_server->getUniverse();
  if(v_universe == NULL) {
    lua_pushnumber(pL, 0);
    return 1;
  }

  if(v_universe->getScenes().size() != 1) {
    throw Exception("Server universe have only 1 scene");
  }

  lua_pushnumber(pL, v_universe->getScenes()[0]->getTime() / 100.0);
  return 1;
}

int ServerRules::L_Rules_Round_getNbRemainingEntitiesToTake(lua_State *pL) {
  args_CheckNumberOfArguments(pL, 0);

  Universe* v_universe;
  v_universe = m_exec_server->getUniverse();
  if(v_universe == NULL) {
    lua_pushnumber(pL, 0);
    return 1;
  }

  if(v_universe->getScenes().size() != 1) {
    throw Exception("Server universe have only 1 scene");
  }

  lua_pushnumber(pL, v_universe->getScenes()[0]->getNbRemainingStrawberries());
  return 1;
}
