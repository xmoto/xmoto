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

#include "LuaLibGame.h"
#include "helpers/VExcept.h"
#include "xmscene/Scene.h"
#include "GameEvents.h"
#include "Input.h"
#include "Locales.h"
#include "helpers/Log.h"
#include "xmscene/Level.h"
#include "xmscene/Block.h"
#include "xmscene/ScriptTimer.h"
#include "XMSession.h"
#include <sstream>

#define STIMER_DELAY_DEFAULT 100
#define STIMER_LOOPS_DEFAULT 0

luaL_reg LuaLibGame::m_gameFuncs[] = {
  {"GetTime",                      LuaLibGame::L_Game_GetTime},
  {"Message",                      LuaLibGame::L_Game_Message},
  {"IsPlayerInZone",               LuaLibGame::L_Game_IsPlayerInZone},
  {"MoveBlock",                    LuaLibGame::L_Game_MoveBlock},
  {"GetBlockPos",                  LuaLibGame::L_Game_GetBlockPos},
  {"SetBlockPos",                  LuaLibGame::L_Game_SetBlockPos},
  {"SetPhysicsBlockPos",           LuaLibGame::L_Game_SetBlockPos},
  {"PlaceInGameArrow",             LuaLibGame::L_Game_PlaceInGameArrow},
  {"PlaceScreenArrow",             LuaLibGame::L_Game_PlaceScreenArrow},
  {"HideArrow",                    LuaLibGame::L_Game_HideArrow},
  {"ClearMessages",                LuaLibGame::L_Game_ClearMessages},
  {"SetGravity",                   LuaLibGame::L_Game_SetGravity},
  {"GetGravity",                   LuaLibGame::L_Game_GetGravity},
  {"SetPlayerPosition",            LuaLibGame::L_Game_SetPlayerPosition},
  {"AddForceToPlayer",             LuaLibGame::L_Game_AddForceToPlayer},
  {"GetPlayerPosition",            LuaLibGame::L_Game_GetPlayerPosition},
  {"GetEntityPos",                 LuaLibGame::L_Game_GetEntityPos},
  {"SetEntityPos",                 LuaLibGame::L_Game_SetEntityPos},
  {"SetKeyHook",                   LuaLibGame::L_Game_SetKeyHook},
  {"GetKeyByAction",               LuaLibGame::L_Game_GetKeyByAction},
  {"Log",                          LuaLibGame::L_Game_Log},
  {"SetBlockCenter",               LuaLibGame::L_Game_SetBlockCenter},
  {"SetBlockRotation",             LuaLibGame::L_Game_SetBlockRotation},
  {"SetDynamicEntitySelfRotation", LuaLibGame::L_Game_SetDynamicEntitySelfRotation},
  {"SetDynamicEntityRotation",     LuaLibGame::L_Game_SetDynamicEntityRotation},
  {"SetDynamicEntityTranslation",  LuaLibGame::L_Game_SetDynamicEntityTranslation},
  {"SetDynamicEntityNone",         LuaLibGame::L_Game_SetDynamicEntityNone},
  {"SetDynamicBlockRotation",      LuaLibGame::L_Game_SetDynamicBlockRotation},
  {"SetDynamicBlockSelfRotation",  LuaLibGame::L_Game_SetDynamicBlockSelfRotation},
  {"SetPhysicsBlockSelfRotation",  LuaLibGame::L_Game_SetPhysicsBlockSelfRotation},
  {"SetPhysicsBlockTranslation",   LuaLibGame::L_Game_SetPhysicsBlockTranslation},
  {"SetDynamicBlockTranslation",   LuaLibGame::L_Game_SetDynamicBlockTranslation},
  {"SetDynamicBlockNone",          LuaLibGame::L_Game_SetDynamicBlockNone},
  {"CameraZoom",        	   LuaLibGame::L_Game_CameraZoom},
  {"CameraMove",        	   LuaLibGame::L_Game_CameraMove},
  {"GetEntityRadius",       	   LuaLibGame::L_Game_GetEntityRadius},
  {"IsEntityTouched",       	   LuaLibGame::L_Game_IsEntityTouched},
  {"KillPlayer",                   LuaLibGame::L_Game_KillPlayer},
  {"KillEntity",                   LuaLibGame::L_Game_KillEntity},
  {"RemainingStrawberries",        LuaLibGame::L_Game_RemainingStrawberries},
  {"WinPlayer",                    LuaLibGame::L_Game_WinPlayer},
  {"AddPenaltyTime",               LuaLibGame::L_Game_PenaltyTime},
  {"IsAPlayerInZone",              LuaLibGame::L_Game_IsAPlayerInZone},
  {"SetAPlayerPosition", 	   LuaLibGame::L_Game_SetAPlayerPosition},
  {"GetAPlayerPosition", 	   LuaLibGame::L_Game_GetAPlayerPosition},
  {"KillAPlayer", 		   LuaLibGame::L_Game_KillAPlayer},
  {"WinAPlayer",  		   LuaLibGame::L_Game_WinAPlayer},
  {"NumberOfPlayers",              LuaLibGame::L_Game_NumberOfPlayers},
  {"CameraRotate",                 LuaLibGame::L_Game_CameraRotate},
  {"CameraAdaptToGravity",         LuaLibGame::L_Game_CameraAdaptToGravity},
  {"SetCameraRotationSpeed",       LuaLibGame::L_Game_SetCameraRotationSpeed},
  {"PlaySound",                    LuaLibGame::L_Game_PlaySound},
  {"PlayMusic",                    LuaLibGame::L_Game_PlayMusic},
  {"StopMusic",                    LuaLibGame::L_Game_StopMusic},
  {"GetPlayerVelocity",            LuaLibGame::L_Game_GetPlayerVelocity},
  {"GetPlayerSpeed",               LuaLibGame::L_Game_GetPlayerSpeed},
  {"GetPlayerAngle",               LuaLibGame::L_Game_GetPlayerAngle}, 
  {"GetPlayerProfileName",         LuaLibGame::L_Game_GetPlayerProfileName},
  {"StartTimer",                   LuaLibGame::L_Game_StartTimer},   
  {"SetTimerDelay",                LuaLibGame::L_Game_SetTimerDelay}, 
  {"StopTimer",                    LuaLibGame::L_Game_StopTimer}, 
  {"SetCameraPosition",            LuaLibGame::L_Game_SetCameraPosition}, 
  {NULL, NULL}
};
Scene*     LuaLibGame::m_exec_world              = NULL;
InputHandler* LuaLibGame::m_exec_activeInputHandler = NULL;

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

LuaLibGame::LuaLibGame(Scene *i_pScene) {
  m_pL = lua_open();
  luaopen_base(m_pL);   
  luaopen_math(m_pL);
  luaopen_table(m_pL);
  luaL_openlib(m_pL, "Game", m_gameFuncs, 0);

  m_pScene           = i_pScene;
  m_pActiveInputHandler = InputHandler::instance();
}

LuaLibGame::~LuaLibGame() {
  lua_close(m_pL);
}

void LuaLibGame::setWorld() {
  m_exec_world              = m_pScene;
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

void LuaLibGame::loadScriptFile(const std::string& i_scriptFilename) {
  FileHandle *pfh = XMFS::openIFile(FDT_DATA, i_scriptFilename);

  if(pfh == NULL) {
    throw Exception("unable to load script " + i_scriptFilename);
  }

  std::string Line,ScriptBuf="";
  
  try {
    while(XMFS::readNextLine(pfh,Line)) {
      if(Line.length() > 0) {
	ScriptBuf.append(Line.append("\n"));
      }
    }
  } catch(Exception &e) {
    XMFS::closeFile(pfh);
    throw e;
  }
  
  XMFS::closeFile(pfh);
  loadScript(ScriptBuf, i_scriptFilename);
}

void LuaLibGame::loadScript(const std::string& i_scriptCode, const std::string& i_scriptFilename) {
  /* Use the Lua aux lib to load the buffer */
  int nRet;

  setWorld();

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

  for(int i=0; i<args_numberOfArguments(pL); i++)
    Out.append(luaL_checkstring(pL, i+1));

  LogInfo((char *)Out.c_str());

  return 0;    
}
  
int LuaLibGame::L_Game_ClearMessages(lua_State *pL) {
  args_CheckNumberOfArguments(pL, 0);

  /* event for this */
  m_exec_world->createGameEvent(new MGE_ClearMessages(m_exec_world->getTime()));
  return 0;
}

int LuaLibGame::L_Game_PlaceInGameArrow(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new MGE_PlaceInGameArrow(m_exec_world->getTime(),
							X_luaL_check_number(pL,1),
							X_luaL_check_number(pL,2),
							X_luaL_check_number(pL,3)));
  return 0;
}

int LuaLibGame::L_Game_PlaceScreenArrow(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new MGE_PlaceScreenarrow(m_exec_world->getTime(),
							X_luaL_check_number(pL,1),
							X_luaL_check_number(pL,2),
							X_luaL_check_number(pL,3)));
  return 0;
}

int LuaLibGame::L_Game_HideArrow(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new MGE_HideArrow(m_exec_world->getTime()));
  return 0;
}
  
int LuaLibGame::L_Game_GetTime(lua_State *pL) {
  args_CheckNumberOfArguments(pL, 0);

  /* no event for this */

  /* Get current game time */
  lua_pushnumber(pL,m_exec_world->getTime() / 100.0);
  return 1;
}
  
int LuaLibGame::L_Game_Message(lua_State *pL) {  
  /* event for this */

  /* Convert all arguments to strings */
  std::string Out;
  for(int i=0;i<lua_gettop(pL);i++) 
    Out.append(_(luaL_checkstring(pL, i+1)));
  
  m_exec_world->createGameEvent(new MGE_Message(m_exec_world->getTime(), Out));
  return 0;
}  
  
int LuaLibGame::L_Game_IsPlayerInZone(lua_State *pL) {
  /* no event for this */
  bool res = false;
  Zone* v_zone;

  try {
    v_zone = m_exec_world->getLevelSrc()->getZoneById(luaL_checkstring(pL, 1));

    for(unsigned int i=0; i<m_exec_world->Players().size(); i++) {
      if(m_exec_world->Players()[i]->isTouching(v_zone)) {
	res = true;
      }
    }
  } catch(Exception &e) {
    /* res will be false */
  }

  lua_pushboolean(pL, res?1:0);
  return 1;
}
  
int LuaLibGame::L_Game_MoveBlock(lua_State *pL) {
  /* event for this */

  m_exec_world->createGameEvent(new MGE_MoveBlock(m_exec_world->getTime(),
						 luaL_checkstring(pL,1),
						 X_luaL_check_number(pL,2),
						 X_luaL_check_number(pL,3)));
  return 0;
}
  
int LuaLibGame::L_Game_GetBlockPos(lua_State *pL) {
  /* no event for this */

  /* Find the specified block and return its position */
  Block* pBlock;

  try {
    pBlock = m_exec_world->getLevelSrc()->getBlockById(luaL_checkstring(pL,1));
    lua_pushnumber(pL,pBlock->DynamicPosition().x);
    lua_pushnumber(pL,pBlock->DynamicPosition().y);
  } catch(Exception &e) {
    lua_pushnumber(pL, 0);
    lua_pushnumber(pL, 0);
  }

  return 2;
}
  
int LuaLibGame::L_Game_SetBlockPos(lua_State *pL) {
  /* event for this */

  m_exec_world->createGameEvent(new MGE_SetBlockPos(m_exec_world->getTime(),
						   luaL_checkstring(pL,1),
						   X_luaL_check_number(pL,2),
						   X_luaL_check_number(pL,3)));
  return 0;
}  

int LuaLibGame::L_Game_SetGravity(lua_State *pL) {
  /* event for this */

  m_exec_world->createGameEvent(new MGE_SetGravity(m_exec_world->getTime(),
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
  m_exec_world->createGameEvent(new MGE_SetPlayersPosition(m_exec_world->getTime(),
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
  Entity *p;

  try {
    p = m_exec_world->getLevelSrc()->getEntityById(luaL_checkstring(pL,1));
  } catch(Exception &e) {
    p = NULL;
  }

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
  try {
    lua_pushnumber(pL, m_exec_world->getLevelSrc()->getEntityById(luaL_checkstring(pL,1))->Size());
  } catch(Exception &e) {
    lua_pushnumber(pL, 0);
  }
  return 1;
}

int LuaLibGame::L_Game_IsEntityTouched(lua_State *pL) {
  /* no event for this */

  bool v_touch = false;
  if(m_exec_world->Players().size() > 0) {
    try {
      v_touch = m_exec_world->Players()[0]->isTouching(m_exec_world->getLevelSrc()->getEntityById(luaL_checkstring(pL,1)));
    } catch(Exception &e) {
      /* v_touch will be false */
    }
  }

  lua_pushnumber(pL, v_touch? 1:0);
  return 1;
}


int LuaLibGame::L_Game_SetEntityPos(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new MGE_SetEntityPos(m_exec_world->getTime(),
						    luaL_checkstring(pL,1),
						    X_luaL_check_number(pL,2),
						    X_luaL_check_number(pL,3)));
  return 0;
}  
  
int LuaLibGame::L_Game_SetKeyHook(lua_State *pL) {
  /* no event for this */

  if(m_exec_activeInputHandler != NULL) {
    try {
      m_exec_activeInputHandler->addScriptKeyHook(m_exec_world, luaL_checkstring(pL,1), luaL_checkstring(pL,2));
    } catch(Exception &e) {  
			luaL_error (pL, "AddScriptKeyHook failed");    
    }
  }
  return 0;
}

int LuaLibGame::L_Game_GetKeyByAction(lua_State *pL) {
  /* no event for this */

  if(m_exec_activeInputHandler != NULL) {
    lua_pushstring(pL,m_exec_activeInputHandler->getFancyKeyByAction(luaL_checkstring(pL,1)).c_str());
    return 1;
  }
  return 0;
}  

int LuaLibGame::L_Game_SetBlockCenter(lua_State *pL) {
  /* event for this */    
  m_exec_world->createGameEvent(new MGE_SetBlockCenter(m_exec_world->getTime(),
						      luaL_checkstring(pL,1),
						      X_luaL_check_number(pL,2),
						      X_luaL_check_number(pL,3)));
  return 0;
}  
    
int LuaLibGame::L_Game_SetBlockRotation(lua_State *pL) {
  /* event for this */    
  m_exec_world->createGameEvent(new MGE_SetBlockRotation(m_exec_world->getTime(),
							luaL_checkstring(pL,1),
							X_luaL_check_number(pL,2)));
  return 0;
}  
    
int LuaLibGame::L_Game_SetDynamicEntityRotation(lua_State *pL) {
  /* event for this */    
  m_exec_world->createGameEvent(new MGE_SetDynamicEntityRotation(m_exec_world->getTime(),
								 luaL_checkstring(pL,1),
								 X_luaL_check_number(pL,2),
								 X_luaL_check_number(pL,3),
								 (int)(X_luaL_check_number(pL,4)),
								 (int)X_luaL_check_number(pL,5),
								 (int)X_luaL_check_number(pL,6))); 
  return 0;
}

int LuaLibGame::L_Game_SetDynamicEntitySelfRotation(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new MGE_SetDynamicEntitySelfRotation(m_exec_world->getTime(),
								     luaL_checkstring(pL,1),
								     (int)(X_luaL_check_number(pL,2)),
								     (int)X_luaL_check_number(pL,3),
								     (int)X_luaL_check_number(pL,4))); 
  return 0;
}

int LuaLibGame::L_Game_SetDynamicEntityTranslation(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new MGE_SetDynamicEntityTranslation(m_exec_world->getTime(),
								    luaL_checkstring(pL,1),
								    X_luaL_check_number(pL,2),
								    X_luaL_check_number(pL,3),
								    (int)(X_luaL_check_number(pL,4)),
								    (int)X_luaL_check_number(pL,5),
								    (int)X_luaL_check_number(pL,6)));
  return 0;
}

int LuaLibGame::L_Game_SetDynamicEntityNone(lua_State *pL) {
  /* event for this */    
  m_exec_world->createGameEvent(new MGE_SetDynamicEntityNone(m_exec_world->getTime(),
							    luaL_checkstring(pL,1)));
  return 0;
}

int LuaLibGame::L_Game_SetDynamicBlockRotation(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new MGE_SetDynamicBlockRotation(m_exec_world->getTime(),
								luaL_checkstring(pL,1),
								X_luaL_check_number(pL,2),
								X_luaL_check_number(pL,3),
								(int)(X_luaL_check_number(pL,4)),
								(int)X_luaL_check_number(pL,5),
								(int)X_luaL_check_number(pL,6)));
  return 0;
}

int LuaLibGame::L_Game_SetDynamicBlockSelfRotation(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new MGE_SetDynamicBlockSelfRotation(m_exec_world->getTime(),
								    luaL_checkstring(pL,1),
								    (int)(X_luaL_check_number(pL,2)),
								    (int)X_luaL_check_number(pL,3),
								    (int)X_luaL_check_number(pL,4))); 
  return 0;
}

int LuaLibGame::L_Game_SetPhysicsBlockSelfRotation(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new MGE_SetPhysicsBlockSelfRotation(m_exec_world->getTime(),
								    luaL_checkstring(pL,1),
								    (int)(X_luaL_check_number(pL,2)),
								    (int)X_luaL_check_number(pL,3),
								    (int)X_luaL_check_number(pL,4))); 
  return 0;
}

int LuaLibGame::L_Game_SetPhysicsBlockTranslation(lua_State* pL)
{
  m_exec_world->createGameEvent(new MGE_SetPhysicsBlockTranslation(m_exec_world->getTime(),
								   luaL_checkstring(pL, 1),
								   X_luaL_check_number(pL, 2),
								   X_luaL_check_number(pL, 3),
								   (int)X_luaL_check_number(pL, 4),
								   (int)X_luaL_check_number(pL, 5),
								   (int)X_luaL_check_number(pL, 6)));

  return 0;
}

int LuaLibGame::L_Game_SetDynamicBlockTranslation(lua_State *pL) {
  /* event for this */    
  m_exec_world->createGameEvent(new MGE_SetDynamicBlockTranslation(m_exec_world->getTime(),
								   luaL_checkstring(pL,1),
								   X_luaL_check_number(pL,2),
								   X_luaL_check_number(pL,3),
								   (int)(X_luaL_check_number(pL,4)),
								   (int)X_luaL_check_number(pL,5),
								   (int)X_luaL_check_number(pL,6)));
  return 0;
}

int LuaLibGame::L_Game_SetDynamicBlockNone(lua_State *pL) {
  /* event for this */    
  m_exec_world->createGameEvent(new MGE_SetDynamicBlockNone(m_exec_world->getTime(),
							   luaL_checkstring(pL,1)));
  return 0;
}

int LuaLibGame::L_Game_CameraZoom(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new MGE_CameraZoom(m_exec_world->getTime(),
						  X_luaL_check_number(pL,1)));
  return 0;
}

int LuaLibGame::L_Game_CameraMove(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new MGE_CameraMove(m_exec_world->getTime(),
						  X_luaL_check_number(pL,1),
						  X_luaL_check_number(pL,2)));
  return 0;
}

int LuaLibGame::L_Game_SetCameraPosition(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new MGE_CameraSetPos(m_exec_world->getTime(),
						  X_luaL_check_number(pL,1),
						  X_luaL_check_number(pL,2)));
  return 0;
}

int LuaLibGame::L_Game_KillPlayer(lua_State *pL) {
  m_exec_world->createGameEvent(new MGE_PlayersDie(m_exec_world->getTime(), false));
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
  m_exec_world->createGameEvent(new MGE_PenalityTime(m_exec_world->getTime(),
						     (int)(X_luaL_check_number(pL,1)*100)));
  return 0;
}

int LuaLibGame::L_Game_IsAPlayerInZone(lua_State *pL) {
  /* no event for this */
  Zone* v_zone;
  int v_player;

  try {
    v_zone = m_exec_world->getLevelSrc()->getZoneById(luaL_checkstring(pL, 1));
    v_player = (int)X_luaL_check_number(pL, 2);
    
  if(v_player < 0 || (unsigned int)v_player >= m_exec_world->Players().size()) {
    luaL_error (pL, "Invalid player number");
  }

    lua_pushboolean(pL, m_exec_world->Players()[v_player]->isTouching(v_zone)?1:0);
  } catch(Exception &e) {
    lua_pushboolean(pL, 0);
  }

  return 1;
}

int LuaLibGame::L_Game_SetAPlayerPosition(lua_State *pL) {
  /* event for this */
  bool bRight  = X_luaL_check_number(pL,3) > 0.0f;
  int v_player = (int)X_luaL_check_number(pL,4);

  if(v_player < 0 || (unsigned int)v_player >= m_exec_world->Players().size()) {
		luaL_error (pL, "Invalid player number");
  }

  m_exec_world->createGameEvent(new MGE_SetPlayerPosition(m_exec_world->getTime(),
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

  int v_player = (int)X_luaL_check_number(pL,1);

  if(v_player < 0 || (unsigned int)v_player >= m_exec_world->Players().size()) {
		luaL_error (pL, "Invalid player number");
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
  int v_player = (int)X_luaL_check_number(pL,1);

  if(v_player < 0 || (unsigned int)v_player >= m_exec_world->Players().size()) {
		luaL_error (pL, "Invalid player number");
  }

  m_exec_world->createGameEvent(new MGE_PlayerDies(m_exec_world->getTime(), false, v_player));
  return 0;
}

int LuaLibGame::L_Game_WinAPlayer(lua_State *pL) {
  int v_player = (int)X_luaL_check_number(pL,1);

  if(v_player < 0 || (unsigned int)v_player >= m_exec_world->Players().size()) {
		luaL_error (pL, "Invalid player number");
  }

  m_exec_world->makePlayerWin(v_player);
  return 0;
}

int LuaLibGame::L_Game_NumberOfPlayers(lua_State *pL) {
  lua_pushnumber(pL, m_exec_world->Players().size());
  return 1;
}


int LuaLibGame::L_Game_CameraRotate(lua_State *pL) {
  m_exec_world->createGameEvent(new MGE_CameraRotate(m_exec_world->getTime(),
							   X_luaL_check_number(pL,1)));
  return 0;
}

int LuaLibGame::L_Game_CameraAdaptToGravity(lua_State *pL) {
  m_exec_world->createGameEvent(new MGE_CameraAdaptToGravity(m_exec_world->getTime()));
  return 0;
}

int LuaLibGame::L_Game_AddForceToPlayer(lua_State *pL) {
  /* event for this */
  m_exec_world->createGameEvent(new MGE_AddForceToPlayer(m_exec_world->getTime(),
							 Vector2f(X_luaL_check_number(pL,1),
								  X_luaL_check_number(pL,2)),
							 (int)X_luaL_check_number(pL,3),
							 (int)X_luaL_check_number(pL,4),
							 (int)X_luaL_check_number(pL,5)));
  return 0;
}  

int LuaLibGame::L_Game_SetCameraRotationSpeed(lua_State *pL) {
  m_exec_world->createGameEvent(new MGE_SetCameraRotationSpeed(m_exec_world->getTime(),X_luaL_check_number(pL,1)));
  return 0;
}

int LuaLibGame::L_Game_PlaySound(lua_State *pL) {
  if(lua_gettop(pL) == 1) { // if there are 2 arguments, consider the 2nd one
    m_exec_world->createGameEvent(new MGE_PlaySound(m_exec_world->getTime(),luaL_checkstring(pL,1)));
  } else {
    m_exec_world->createGameEvent(new MGE_PlaySound(m_exec_world->getTime(),luaL_checkstring(pL,1), X_luaL_check_number(pL,2)));
  }

  return 0;
}

int LuaLibGame::L_Game_PlayMusic(lua_State *pL) {
  m_exec_world->createGameEvent(new MGE_PlayMusic(m_exec_world->getTime(),luaL_checkstring(pL,1)));
  return 0;
}

int LuaLibGame::L_Game_StopMusic(lua_State *pL) {
  m_exec_world->createGameEvent(new MGE_StopMusic(m_exec_world->getTime()));
  return 0;
}

/* by Tuhoojabotti */
int LuaLibGame::L_Game_GetPlayerVelocity(lua_State *pL) {
  /* no event for this */
  int v_player = (int)X_luaL_check_number(pL,1);

  if(v_player < 0 || (unsigned int)v_player >= m_exec_world->Players().size()) {
		luaL_error (pL, "Invalid player number");
  }
  lua_pushnumber(pL, m_exec_world->Players()[v_player]->getBikeLinearVel());

  return 1;
}
/* by Tuhoojabotti */
int LuaLibGame::L_Game_GetPlayerSpeed(lua_State *pL) {
  /* no event for this */
  int v_player = (int)X_luaL_check_number(pL,1);
  
  if(v_player < 0 || (unsigned int)v_player >= m_exec_world->Players().size()) {
		luaL_error (pL, "Invalid player number");
  }
  lua_pushnumber(pL, m_exec_world->Players()[v_player]->getBikeEngineSpeed());

  return 1;
}
/* by Tuhoojabotti */
int LuaLibGame::L_Game_GetPlayerAngle(lua_State *pL) {
  /* no event for this */
  int v_player = (int)X_luaL_check_number(pL,1);
  
  if(v_player < 0 || (unsigned int)v_player >= m_exec_world->Players().size()) {
		luaL_error (pL, "Invalid player number");
  }
  lua_pushnumber(pL, m_exec_world->Players()[v_player]->getAngle());

  return 1; //return 1 value
}

int LuaLibGame::L_Game_GetPlayerProfileName(lua_State *pl) {
  /* no event for this */
  std::string v_profileName;
  v_profileName = XMSession::instance()->profile();
  lua_pushstring(pl,v_profileName.c_str());
  
  return 1;
}

/*=====================================================
        Script Timer Functions! by tuhoojabotti
=====================================================*/
//START
int LuaLibGame::L_Game_StartTimer(lua_State *pL) {
	args_CheckNumberOfArguments(pL, 1, 3); //we need 1 or 3 args
	//set the defaults
  int v_numargs=args_numberOfArguments(pL); 
  std::string v_name="";
	int v_delay = STIMER_DELAY_DEFAULT;
	int v_loops=STIMER_LOOPS_DEFAULT;
	ScriptTimer* v_timer=NULL;
  //get parameters
	v_name=(std::string)luaL_checkstring(pL,1);
	v_timer=m_exec_world->getScriptTimerByName(v_name);
	//check the optional args
	if(v_numargs>1){
		v_delay=(int)luaL_checknumber(pL,2);
		if(v_delay<1){luaL_error (pL, "StartTimer: delay can't be smaller than 1.");}
	}
	if(v_numargs>2){
		v_loops=(int)luaL_checknumber(pL,3);
		if(v_loops<0){luaL_error (pL, "StartTimer: loops can't be smaller than 0.");}
	}
	//if timer is not found
	if(v_timer==NULL){
		m_exec_world->createScriptTimer(v_name,v_delay,v_loops); //create timer
	}else{
		if(v_numargs == 1){ //only name given
			v_timer->StartTimer(); //start the timer
		}else{ //numarg is not 1 so it can only be 2 or 3 which is okay
			v_timer->ResetTimer(v_delay,v_loops,m_exec_world->getTime());
			/* For example: if only delay is given then the loops=0 (default) */
		}
	}
  return 0; //return no values to the script
}
//SET DELAY
int LuaLibGame::L_Game_SetTimerDelay(lua_State *pL) {
	args_CheckNumberOfArguments(pL, 2, 2); //exactly 2 args needed
  std::string v_name=(std::string)luaL_checkstring(pL,1);
	int v_delay=(int)luaL_checknumber(pL,2);
	ScriptTimer* v_timer=m_exec_world->getScriptTimerByName(v_name);
	if(v_timer!=NULL){
		v_timer->SetTimerDelay(v_delay);
	}else{
		luaL_error (pL, "An error occured, timer doesn't exist!");   
	}
	return 0; //return no values to the script
}
//STOP
int LuaLibGame::L_Game_StopTimer(lua_State *pL) {
	args_CheckNumberOfArguments(pL, 1, 1); //exactly 1 arg needed
  std::string v_name = (std::string)luaL_checkstring(pL,1); //get name
  ScriptTimer* v_timer=m_exec_world->getScriptTimerByName(v_name);
  if(v_timer!=NULL){ //timer is found
		v_timer->PauseTimer(); //pause it
  }else{
		luaL_error (pL, "An error occured, timer doesn't exist!"); 
	}
  return 0; //return no values to the script
}
/*=====================================================
            Script Helpers
=====================================================*/

int LuaLibGame::args_numberOfArguments(lua_State *pL) {
  return lua_gettop(pL);
}

void LuaLibGame::args_CheckNumberOfArguments(lua_State *pL, int i_from, int i_to) {
  int n = args_numberOfArguments(pL);

  if(i_to == -1) {
    if(n != i_from) {
      luaL_error (pL, "Invalid number of arguments");  
    }
  } else {
    if(n < i_from || n > i_to) {
      luaL_error (pL, "Invalid number of arguments");  
    }
  }
}
