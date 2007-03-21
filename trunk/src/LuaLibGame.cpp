/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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

#include "LuaLibGame.h"
#include "helpers/VExcept.h"
#include "MotoGame.h"
#include "GameEvents.h"
#include "Input.h"

vapp::MotoGame*     LuaLibGame::m_exec_world              = NULL;
vapp::InputHandler* LuaLibGame::m_exec_activeInputHandler = NULL;
luaL_reg            LuaLibGame::m_gameFuncs[] = {
  {"GetTime",                     LuaLibGame::L_Game_GetTime},
  {"Message",                     LuaLibGame::L_Game_Message},
  {"IsPlayerInZone",              LuaLibGame::L_Game_IsPlayerInZone},
  {"MoveBlock",                   LuaLibGame::L_Game_MoveBlock},
  {"GetBlockPos",                 LuaLibGame::L_Game_GetBlockPos},
  {"SetBlockPos",                 LuaLibGame::L_Game_SetBlockPos},
  {"PlaceInGameArrow",            LuaLibGame::L_Game_PlaceInGameArrow},
  {"PlaceScreenArrow",            LuaLibGame::L_Game_PlaceScreenArrow},
  {"HideArrow",                   LuaLibGame::L_Game_HideArrow},
  {"ClearMessages",               LuaLibGame::L_Game_ClearMessages},
  {"SetGravity",                  LuaLibGame::L_Game_SetGravity},
  {"GetGravity",                  LuaLibGame::L_Game_GetGravity},
  {"SetPlayerPosition",           LuaLibGame::L_Game_SetPlayerPosition},
  {"GetPlayerPosition",           LuaLibGame::L_Game_GetPlayerPosition},
  {"GetEntityPos",                LuaLibGame::L_Game_GetEntityPos},
  {"SetEntityPos",                LuaLibGame::L_Game_SetEntityPos},
  {"SetKeyHook",                  LuaLibGame::L_Game_SetKeyHook},
  {"GetKeyByAction",              LuaLibGame::L_Game_GetKeyByAction},
  {"Log",                         LuaLibGame::L_Game_Log},
  {"SetBlockCenter",              LuaLibGame::L_Game_SetBlockCenter},
  {"SetBlockRotation",            LuaLibGame::L_Game_SetBlockRotation},
  {"SetDynamicEntityRotation",    LuaLibGame::L_Game_SetDynamicEntityRotation},
  {"SetDynamicEntityTranslation", LuaLibGame::L_Game_SetDynamicEntityTranslation},
  {"SetDynamicEntityNone",        LuaLibGame::L_Game_SetDynamicEntityNone},
  {"SetDynamicBlockRotation",     LuaLibGame::L_Game_SetDynamicBlockRotation},
  {"SetDynamicBlockTranslation",  LuaLibGame::L_Game_SetDynamicBlockTranslation},
  {"SetDynamicBlockNone",         LuaLibGame::L_Game_SetDynamicBlockNone},
  {"CameraZoom",        	  LuaLibGame::L_Game_CameraZoom},
  {"CameraMove",        	  LuaLibGame::L_Game_CameraMove},
  {"GetEntityRadius",       	  LuaLibGame::L_Game_GetEntityRadius},
  {"IsEntityTouched",       	  LuaLibGame::L_Game_IsEntityTouched},
  {"KillPlayer",                  LuaLibGame::L_Game_KillPlayer},
  {"KillEntity",                  LuaLibGame::L_Game_KillEntity},
  {"RemainingStrawberries",       LuaLibGame::L_Game_RemainingStrawberries},
  {"WinPlayer",                   LuaLibGame::L_Game_WinPlayer},
  {"AddPenaltyTime",              LuaLibGame::L_Game_PenaltyTime},
  {"IsAPlayerInZone",             LuaLibGame::L_Game_IsAPlayerInZone},
  {"SetAPlayerPosition", 	  LuaLibGame::L_Game_SetAPlayerPosition},
  {"GetAPlayerPosition", 	  LuaLibGame::L_Game_GetAPlayerPosition},
  {"KillAPlayer", 		  LuaLibGame::L_Game_KillAPlayer},
  {"WinAPlayer",  		  LuaLibGame::L_Game_WinAPlayer},
  {"NumberOfPlayers",             LuaLibGame::L_Game_NumberOfPlayers},
  {NULL, NULL}
};

/*===========================================================================
  Lua 5.1 compatibility code (Following is from lua 5.0.2)
  ===========================================================================*/
static void X_tag_error (lua_State *L, int narg, int tag) {
  luaL_typerror(L, narg, lua_typename(L, tag)); 
}

lua_Number X_luaL_check_number(lua_State *L,int narg) {
  lua_Number d = lua_tonumber(L, narg);
  if (d == 0 && !lua_isnumber(L, narg))  /* avoid extra test when d is not 0 */
    X_tag_error(L, narg, LUA_TNUMBER);
  return d;    
}

LuaLibGame::LuaLibGame(vapp::MotoGame *i_pMotoGame, vapp::InputHandler *i_pActiveInputHandler) {
  m_pL = lua_open();
  luaopen_base(m_pL);   
  luaopen_math(m_pL);
  luaopen_table(m_pL);
  luaL_openlib(m_pL,"Game", m_gameFuncs, 0);

  m_pMotoGame           = i_pMotoGame;
  m_pActiveInputHandler = i_pActiveInputHandler;
}

LuaLibGame::~LuaLibGame() {
  lua_close(m_pL);
}

void LuaLibGame::setWorld() {
  m_exec_world              = m_pMotoGame;
  m_exec_activeInputHandler = m_pActiveInputHandler;
}

/*===========================================================================
  Simple lua interaction
  ===========================================================================*/
bool LuaLibGame::scriptCallBool(const std::string& FuncName,bool bDefault) {
  setWorld();
  
  bool bRet = bDefault;
  
    /* Fetch global function */
  lua_getglobal(m_pL,FuncName.c_str());
  
  /* Is it really a function and not just a pile of ****? */
  if(lua_isfunction(m_pL,-1)) {
    /* Call! */
    if(lua_pcall(m_pL,0,1,0) != 0) {
        throw Exception("failed to invoke (bool) " + FuncName + std::string("(): ") + std::string(lua_tostring(m_pL,-1)));
    }
    
    /* Retrieve return value */
    if(!lua_toboolean(m_pL,-1)) {      
      bRet = false;
    }
    else {
      bRet = true;
    }
  }
  
  /* Reset Lua VM */
  lua_settop(m_pL,0);        
  
  return bRet;
}

void LuaLibGame::scriptCallVoid(const std::string& FuncName) {
  setWorld();
  
  /* Fetch global function */
  lua_getglobal(m_pL,FuncName.c_str());
  
  /* Is it really a function and not just a pile of ****? */
  if(lua_isfunction(m_pL,-1)) {
    /* Call! */
    if(lua_pcall(m_pL,0,0,0) != 0) {
      throw Exception("failed to invoke (void) " + FuncName + std::string("(): ") + std::string(lua_tostring(m_pL,-1)));
    }      
  }
  
  /* Reset Lua VM */
  lua_settop(m_pL,0);        
}

void LuaLibGame::scriptCallVoidNumberArg(const std::string& FuncName, int n) {
  setWorld();
  
  /* Fetch global function */
  lua_getglobal(m_pL,FuncName.c_str());
  
  /* Is it really a function and not just a pile of ****? */
  if(lua_isfunction(m_pL,-1)) {
    /* Call! */
    lua_pushnumber(m_pL, n);
    if(lua_pcall(m_pL,1,0,0) != 0) {
      throw Exception("failed to invoke (void) " + FuncName + std::string("(): ") + std::string(lua_tostring(m_pL,-1)));
    }      
  }
  
  /* Reset Lua VM */
  lua_settop(m_pL,0);        
}

void LuaLibGame::scriptCallVoidNumberArg(const std::string& FuncName, int n1, int n2) {
  setWorld();
  
  /* Fetch global function */
  lua_getglobal(m_pL,FuncName.c_str());
  
  /* Is it really a function and not just a pile of ****? */
  if(lua_isfunction(m_pL,-1)) {
    /* Call! */
    lua_pushnumber(m_pL, n1);
    lua_pushnumber(m_pL, n2);
    if(lua_pcall(m_pL,2,0,0) != 0) {
      throw Exception("failed to invoke (void) " + FuncName + std::string("(): ") + std::string(lua_tostring(m_pL,-1)));
    }      
  }
  
  /* Reset Lua VM */
  lua_settop(m_pL,0);        
}

void LuaLibGame::scriptCallTblVoid(const std::string& Table, const std::string& FuncName) {
  setWorld();
  
  /* Fetch global table */        
  lua_getglobal(m_pL,Table.c_str());
  
  //    printf("[%s.%s]\n",Table.c_str(),FuncName.c_str());
  
  if(lua_istable(m_pL,-1)) {
    lua_pushstring(m_pL,FuncName.c_str());
    lua_gettable(m_pL,-2);
    
    if(lua_isfunction(m_pL,-1)) {
      /* Call! */
      if(lua_pcall(m_pL,0,0,0) != 0) {
	throw Exception("failed to invoke (tbl,void) " + Table + std::string(".") + 
			FuncName + std::string("(): ") + std::string(lua_tostring(m_pL,-1)));
      }              
    }
  }
  
  /* Reset Lua VM */
  lua_settop(m_pL,0);        
}

void LuaLibGame::scriptCallTblVoid(const std::string& Table, const std::string& FuncName, int n) {
  setWorld();
  
  /* Fetch global table */        
  lua_getglobal(m_pL,Table.c_str());
  
  //    printf("[%s.%s]\n",Table.c_str(),FuncName.c_str());
  
  if(lua_istable(m_pL,-1)) {
    lua_pushstring(m_pL,FuncName.c_str());
    lua_gettable(m_pL,-2);
    
    if(lua_isfunction(m_pL,-1)) {
      /* Call! */
      lua_pushnumber(m_pL, n);
      if(lua_pcall(m_pL,1,0,0) != 0) {
	throw Exception("failed to invoke (tbl,void) " + Table + std::string(".") + 
			FuncName + std::string("(): ") + std::string(lua_tostring(m_pL,-1)));
      }              
    }
  }
  
  /* Reset Lua VM */
  lua_settop(m_pL,0);        
}

void LuaLibGame::loadScript(const std::string& i_scriptCode, const std::string& i_scriptFilename) {
  /* Use the Lua aux lib to load the buffer */
  int nRet;

  nRet = luaL_loadbuffer(m_pL, i_scriptCode.c_str(), i_scriptCode.length(),
			 i_scriptFilename.c_str()) || lua_pcall(m_pL, 0, 0, 0);    

  /* Returned WHAT? */
  if(nRet != 0) {
    throw Exception("failed to load level script");
  }
}

std::string LuaLibGame::getErrorMsg() {
  return lua_tostring(m_pL,-1);
}

/*===========================================================================
  Game.* 
  Lua game library functions
  ===========================================================================*/
int LuaLibGame::L_Game_Log(lua_State *pL) {
  /* no event for this */
  
  std::string Out;
  for(int i=0;i<lua_gettop(pL);i++) 
    Out.append(luaL_checkstring(pL,i+1));
  vapp::Log((char *)Out.c_str());    
  return 0;    
}
  
int LuaLibGame::L_Game_ClearMessages(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new vapp::MGE_ClearMessages(m_exec_world->getTime()));
  return 0;
}

int LuaLibGame::L_Game_PlaceInGameArrow(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new vapp::MGE_PlaceInGameArrow(m_exec_world->getTime(),
							X_luaL_check_number(pL,1),
							X_luaL_check_number(pL,2),
							X_luaL_check_number(pL,3)));
  return 0;
}

int LuaLibGame::L_Game_PlaceScreenArrow(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new vapp::MGE_PlaceScreenarrow(m_exec_world->getTime(),
							X_luaL_check_number(pL,1),
							X_luaL_check_number(pL,2),
							X_luaL_check_number(pL,3)));
  return 0;
}

int LuaLibGame::L_Game_HideArrow(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new vapp::MGE_HideArrow(m_exec_world->getTime()));
  return 0;
}
  
int LuaLibGame::L_Game_GetTime(lua_State *pL) {
  /* no event for this */

  /* Get current game time */
  lua_pushnumber(pL,m_exec_world->getTime());
  return 1;
}
  
int LuaLibGame::L_Game_Message(lua_State *pL) {  
  /* event for this */

  /* Convert all arguments to strings */
  std::string Out;
  for(int i=0;i<lua_gettop(pL);i++) 
    Out.append(luaL_checkstring(pL, i+1));
  
  m_exec_world->createGameEvent(new vapp::MGE_Message(m_exec_world->getTime(), Out));
  return 0;
}  
  
int LuaLibGame::L_Game_IsPlayerInZone(lua_State *pL) {
  /* no event for this */
  bool res = false;
  Zone* v_zone = &(m_exec_world->getLevelSrc()->getZoneById(luaL_checkstring(pL, 1)));

  for(int i=0; i<m_exec_world->Players().size(); i++) {
    if(m_exec_world->Players()[i]->isTouching(*v_zone)) {
      res = true;
    }
  }
  lua_pushboolean(pL, res?1:0);
  return 1;
}
  
int LuaLibGame::L_Game_MoveBlock(lua_State *pL) {
  /* event for this */

  m_exec_world->createGameEvent(new vapp::MGE_MoveBlock(m_exec_world->getTime(),
						 luaL_checkstring(pL,1),
						 X_luaL_check_number(pL,2),
						 X_luaL_check_number(pL,3)));
  return 0;
}
  
int LuaLibGame::L_Game_GetBlockPos(lua_State *pL) {
  /* no event for this */

  /* Find the specified block and return its position */
  Block &pBlock = m_exec_world->getLevelSrc()->getBlockById(luaL_checkstring(pL,1));
  lua_pushnumber(pL,pBlock.DynamicPosition().x);
  lua_pushnumber(pL,pBlock.DynamicPosition().y);
  return 2;
}
  
int LuaLibGame::L_Game_SetBlockPos(lua_State *pL) {
  /* event for this */

  m_exec_world->createGameEvent(new vapp::MGE_SetBlockPos(m_exec_world->getTime(),
						   luaL_checkstring(pL,1),
						   X_luaL_check_number(pL,2),
						   X_luaL_check_number(pL,3)));
  return 0;
}  

int LuaLibGame::L_Game_SetGravity(lua_State *pL) {
  /* event for this */

  m_exec_world->createGameEvent(new vapp::MGE_SetGravity(m_exec_world->getTime(),
						  X_luaL_check_number(pL,1),
						  X_luaL_check_number(pL,2)));
  return 0;
}  

int LuaLibGame::L_Game_GetGravity(lua_State *pL) {
  /* no event for this */

  /* Get gravity */
  lua_pushnumber(pL, m_exec_world->getGravity().x);
  lua_pushnumber(pL, m_exec_world->getGravity().y);    
  return 2;
}

int LuaLibGame::L_Game_SetPlayerPosition(lua_State *pL) {
  /* event for this */
  bool bRight = X_luaL_check_number(pL,3) > 0.0f;
  m_exec_world->createGameEvent(new vapp::MGE_SetPlayersPosition(m_exec_world->getTime(),
								 X_luaL_check_number(pL,1),
								 X_luaL_check_number(pL,2),
								 bRight));
  return 0;
}  
  
int LuaLibGame::L_Game_GetPlayerPosition(lua_State *pL) {
  /* no event for this */
  float x = 0.0, y = 0.0;
  DriveDir v_direction = DD_RIGHT;

  if(m_exec_world->Players().size() > 0) {
    x = m_exec_world->Players()[0]->getState()->CenterP.x;
    y = m_exec_world->Players()[0]->getState()->CenterP.y;
    v_direction = m_exec_world->Players()[0]->getState()->Dir;
  }

  lua_pushnumber(pL, x);
  lua_pushnumber(pL, y);
  lua_pushnumber(pL, v_direction);

  return 3;
}

int LuaLibGame::L_Game_GetEntityPos(lua_State *pL) {
  /* no event for this */

  /* Find the specified entity and return its position */
  Entity *p = &(m_exec_world->getLevelSrc()->getEntityById(luaL_checkstring(pL,1)));
  if(p != NULL) {
    lua_pushnumber(pL,p->DynamicPosition().x);
    lua_pushnumber(pL,p->DynamicPosition().y);
    return 2;
  }

  /* Entity not found, return <0,0> */
  lua_pushnumber(pL,0);
  lua_pushnumber(pL,0);
  return 2;
}  
  
int LuaLibGame::L_Game_GetEntityRadius(lua_State *pL) {
  /* no event for this */
  lua_pushnumber(pL, m_exec_world->getLevelSrc()->getEntityById(luaL_checkstring(pL,1)).Size());
  return 1;
}

int LuaLibGame::L_Game_IsEntityTouched(lua_State *pL) {
  /* no event for this */

  bool v_touch = false;
  if(m_exec_world->Players().size() > 0) {
    v_touch = m_exec_world->Players()[0]->isTouching(m_exec_world->getLevelSrc()->getEntityById(luaL_checkstring(pL,1)));
  }

  lua_pushnumber(pL, v_touch? 1:0);
  return 1;
}


int LuaLibGame::L_Game_SetEntityPos(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new vapp::MGE_SetEntityPos(m_exec_world->getTime(),
						    luaL_checkstring(pL,1),
						    X_luaL_check_number(pL,2),
						    X_luaL_check_number(pL,3)));
  return 0;
}  
  
int LuaLibGame::L_Game_SetKeyHook(lua_State *pL) {
  /* no event for this */

  if(m_exec_activeInputHandler != NULL) {
    m_exec_activeInputHandler->addScriptKeyHook(m_exec_world,luaL_checkstring(pL,1),luaL_checkstring(pL,2));
  }
  return 0;
}

int LuaLibGame::L_Game_GetKeyByAction(lua_State *pL) {
  /* no event for this */

  if(m_exec_activeInputHandler != NULL) {
    lua_pushstring(pL,m_exec_activeInputHandler->getKeyByAction(luaL_checkstring(pL,1)).c_str());
    return 1;
  }
  return 0;
}  

int LuaLibGame::L_Game_SetBlockCenter(lua_State *pL) {
  /* event for this */    
  m_exec_world->createGameEvent(new vapp::MGE_SetBlockCenter(m_exec_world->getTime(),
						      luaL_checkstring(pL,1),
						      X_luaL_check_number(pL,2),
						      X_luaL_check_number(pL,3)));
  return 0;
}  
    
int LuaLibGame::L_Game_SetBlockRotation(lua_State *pL) {
  /* event for this */    
  m_exec_world->createGameEvent(new vapp::MGE_SetBlockRotation(m_exec_world->getTime(),
							luaL_checkstring(pL,1),
							X_luaL_check_number(pL,2)));
  return 0;
}  
    
int LuaLibGame::L_Game_SetDynamicEntityRotation(lua_State *pL) {
  /* event for this */    
  m_exec_world->createGameEvent(new vapp::MGE_SetDynamicEntityRotation(m_exec_world->getTime(),
								luaL_checkstring(pL,1),
								X_luaL_check_number(pL,2),
								X_luaL_check_number(pL,3),
								X_luaL_check_number(pL,4),
								X_luaL_check_number(pL,5),
								X_luaL_check_number(pL,6))); 
  return 0;
}

int LuaLibGame::L_Game_SetDynamicEntityTranslation(lua_State *pL) {
  /* event for this */    
  m_exec_world->createGameEvent(new vapp::MGE_SetDynamicEntityTranslation(m_exec_world->getTime(),
								   luaL_checkstring(pL,1),
								   X_luaL_check_number(pL,2),
								   X_luaL_check_number(pL,3),
								   X_luaL_check_number(pL,4),
								   X_luaL_check_number(pL,5),
								   X_luaL_check_number(pL,6)));
  return 0;
}

int LuaLibGame::L_Game_SetDynamicEntityNone(lua_State *pL) {
  /* event for this */    
  m_exec_world->createGameEvent(new vapp::MGE_SetDynamicEntityNone(m_exec_world->getTime(),
							    luaL_checkstring(pL,1)));
  return 0;
}

int LuaLibGame::L_Game_SetDynamicBlockRotation(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new vapp::MGE_SetDynamicBlockRotation(m_exec_world->getTime(),
							       luaL_checkstring(pL,1),
							       X_luaL_check_number(pL,2),
							       X_luaL_check_number(pL,3),
							       X_luaL_check_number(pL,4),
							       X_luaL_check_number(pL,5),
							       X_luaL_check_number(pL,6))); 
  return 0;
}

int LuaLibGame::L_Game_SetDynamicBlockTranslation(lua_State *pL) {
  /* event for this */    
  m_exec_world->createGameEvent(new vapp::MGE_SetDynamicBlockTranslation(m_exec_world->getTime(),
								  luaL_checkstring(pL,1),
								  X_luaL_check_number(pL,2),
								  X_luaL_check_number(pL,3),
								  X_luaL_check_number(pL,4),
								  X_luaL_check_number(pL,5),
								  X_luaL_check_number(pL,6)));
  return 0;
}

int LuaLibGame::L_Game_SetDynamicBlockNone(lua_State *pL) {
  /* event for this */    
  m_exec_world->createGameEvent(new vapp::MGE_SetDynamicBlockNone(m_exec_world->getTime(),
							   luaL_checkstring(pL,1)));
  return 0;
}

int LuaLibGame::L_Game_CameraZoom(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new vapp::MGE_CameraZoom(m_exec_world->getTime(),
						  X_luaL_check_number(pL,1)));
  return 0;
}

int LuaLibGame::L_Game_CameraMove(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new vapp::MGE_CameraMove(m_exec_world->getTime(),
						  X_luaL_check_number(pL,1),
						  X_luaL_check_number(pL,2)));
  return 0;
}

int LuaLibGame::L_Game_KillPlayer(lua_State *pL) {
  m_exec_world->createGameEvent(new vapp::MGE_PlayersDie(m_exec_world->getTime(), false));
  return 0;
}

int LuaLibGame::L_Game_KillEntity(lua_State *pL) {
  m_exec_world->createKillEntityEvent(luaL_checkstring(pL,1));
  return 0;
}

int LuaLibGame::L_Game_RemainingStrawberries(lua_State *pL) {
  /* no event for this */
  lua_pushnumber(pL,m_exec_world->getNbRemainingStrawberries());
  return 1;
}

int LuaLibGame::L_Game_WinPlayer(lua_State *pL) {
  for(unsigned int i=0; i<m_exec_world->Players().size(); i++) {
    m_exec_world->makePlayerWin(i);
  }
  return 0;
}

int LuaLibGame::L_Game_PenaltyTime(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new vapp::MGE_PenalityTime(m_exec_world->getTime(),
						    X_luaL_check_number(pL,1)));
  return 0;
}

int LuaLibGame::L_Game_IsAPlayerInZone(lua_State *pL) {
  /* no event for this */
  Zone* v_zone = &(m_exec_world->getLevelSrc()->getZoneById(luaL_checkstring(pL, 1)));
  int v_player = X_luaL_check_number(pL, 2);

  if(v_player < 0 || v_player >= m_exec_world->Players().size()) {
    std::ostringstream v_txt_player;
    v_txt_player << v_player;
    throw Exception("Invalid player " + v_txt_player.str());
  }

  lua_pushboolean(pL, m_exec_world->Players()[v_player]->isTouching(*v_zone)?1:0);
  return 1;
}

int LuaLibGame::L_Game_SetAPlayerPosition(lua_State *pL) {
  /* event for this */
  bool bRight  = X_luaL_check_number(pL,3) > 0.0f;
  int v_player = X_luaL_check_number(pL,4);

  if(v_player < 0 || v_player >= m_exec_world->Players().size()) {
    std::ostringstream v_txt_player;
    v_txt_player << v_player;
    printf("player is %i\n", v_player);
    //throw Exception("Invalid player " + v_txt_player.str());
  }

  m_exec_world->createGameEvent(new vapp::MGE_SetPlayerPosition(m_exec_world->getTime(),
								X_luaL_check_number(pL,1),
								X_luaL_check_number(pL,2),
								bRight,
								v_player));
  return 0;
}

int LuaLibGame::L_Game_GetAPlayerPosition(lua_State *pL) {
  /* no event for this */
  float x = 0.0, y = 0.0;
  DriveDir v_direction = DD_RIGHT;

  int v_player = X_luaL_check_number(pL,1);

  if(v_player < 0 || v_player >= m_exec_world->Players().size()) {
    std::ostringstream v_txt_player;
    v_txt_player << v_player;
    throw Exception("Invalid player " + v_txt_player.str());
  }

  x = m_exec_world->Players()[v_player]->getState()->CenterP.x;
  y = m_exec_world->Players()[v_player]->getState()->CenterP.y;
  v_direction = m_exec_world->Players()[v_player]->getState()->Dir;

  lua_pushnumber(pL, x);
  lua_pushnumber(pL, y);
  lua_pushnumber(pL, v_direction);

  return 3;
}

int LuaLibGame::L_Game_KillAPlayer(lua_State *pL) {
  int v_player = X_luaL_check_number(pL,1);

  if(v_player < 0 || v_player >= m_exec_world->Players().size()) {
    std::ostringstream v_txt_player;
    v_txt_player << v_player;
    throw Exception("Invalid player " + v_txt_player.str());
  }

  m_exec_world->createGameEvent(new vapp::MGE_PlayerDies(m_exec_world->getTime(), false, v_player));
  return 0;
}

int LuaLibGame::L_Game_WinAPlayer(lua_State *pL) {
  int v_player = X_luaL_check_number(pL,1);

  if(v_player < 0 || v_player >= m_exec_world->Players().size()) {
    std::ostringstream v_txt_player;
    v_txt_player << v_player;
    throw Exception("Invalid player " + v_txt_player.str());
  }

  m_exec_world->makePlayerWin(v_player);
  return 0;
}

int LuaLibGame::L_Game_NumberOfPlayers(lua_State *pL) {
  lua_pushnumber(pL, m_exec_world->Players().size());
  return 1;
}
