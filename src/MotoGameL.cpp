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

/* 
 *  LUA glue.
 */
#include "Game.h"
#include "MotoGame.h"
#include "VFileIO.h"
#include "Input.h"

#include "xmscene/Bike.h"
#include "xmscene/BikeGhost.h"
#include "xmscene/BikePlayer.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

namespace vapp {

  /*===========================================================================
  Active input handler (feed me a handgrenade) 
  ===========================================================================*/  
  InputHandler *m_pActiveInputHandler = NULL;

  /*===========================================================================
  Externs
  ===========================================================================*/  
  extern MotoGame *m_pMotoGame;        

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

  /*===========================================================================
  Game.* 
    Lua game library functions
  ===========================================================================*/
  int L_Game_Log(lua_State *pL) {
    /* no event for this */

    std::string Out;
    for(int i=0;i<lua_gettop(pL);i++) 
      Out.append(luaL_checkstring(pL,i+1));
    Log((char *)Out.c_str());    
    return 0;    
  }
  
  int L_Game_ClearMessages(lua_State *pL) {
    /* event for this */
    m_pMotoGame->createGameEvent(new MGE_ClearMessages(m_pMotoGame->getTime()));
    return 0;
  }

  int L_Game_PlaceInGameArrow(lua_State *pL) {
    /* event for this */
    m_pMotoGame->createGameEvent(new MGE_PlaceInGameArrow(m_pMotoGame->getTime(),
                X_luaL_check_number(pL,1),
                X_luaL_check_number(pL,2),
                X_luaL_check_number(pL,3)));
    return 0;
  }

  int L_Game_PlaceScreenArrow(lua_State *pL) {
    /* event for this */
    m_pMotoGame->createGameEvent(new MGE_PlaceScreenarrow(m_pMotoGame->getTime(),
                X_luaL_check_number(pL,1),
                X_luaL_check_number(pL,2),
                X_luaL_check_number(pL,3)));
    return 0;
  }

  int L_Game_HideArrow(lua_State *pL) {
    /* event for this */
    m_pMotoGame->createGameEvent(new MGE_HideArrow(m_pMotoGame->getTime()));
    return 0;
  }
  
  int L_Game_GetTime(lua_State *pL) {
    /* no event for this */

    /* Get current game time */
    lua_pushnumber(pL,m_pMotoGame->getTime());
    return 1;
  }
  
  int L_Game_Message(lua_State *pL) {  
    /* event for this */

    /* Convert all arguments to strings */
    std::string Out;
    for(int i=0;i<lua_gettop(pL);i++) 
      Out.append(luaL_checkstring(pL, i+1));
  
    m_pMotoGame->createGameEvent(new MGE_Message(m_pMotoGame->getTime(), Out));
    return 0;
  }  
  
  int L_Game_IsPlayerInZone(lua_State *pL) {
    /* no event for this */
    bool res = false;
    Zone* v_zone = &(m_pMotoGame->getLevelSrc()->getZoneById(luaL_checkstring(pL, 1)));

    for(int i=0; i<m_pMotoGame->Players().size(); i++) {
      if(m_pMotoGame->Players()[i]->isTouching(*v_zone)) {
	res = true;
      }
    }
    lua_pushboolean(pL, res?1:0);
    return 1;
  }
  
  int L_Game_MoveBlock(lua_State *pL) {
    /* event for this */

    m_pMotoGame->createGameEvent(new MGE_MoveBlock(m_pMotoGame->getTime(),
               luaL_checkstring(pL,1),
               X_luaL_check_number(pL,2),
               X_luaL_check_number(pL,3)));
    return 0;
  }
  
  int L_Game_GetBlockPos(lua_State *pL) {
    /* no event for this */

    /* Find the specified block and return its position */
    Block &pBlock = m_pMotoGame->getLevelSrc()->getBlockById(luaL_checkstring(pL,1));
    lua_pushnumber(pL,pBlock.DynamicPosition().x);
    lua_pushnumber(pL,pBlock.DynamicPosition().y);
    return 2;
  }
  
  int L_Game_SetBlockPos(lua_State *pL) {
    /* event for this */

    m_pMotoGame->createGameEvent(new MGE_SetBlockPos(m_pMotoGame->getTime(),
                 luaL_checkstring(pL,1),
                 X_luaL_check_number(pL,2),
                 X_luaL_check_number(pL,3)));
    return 0;
  }  

  int L_Game_SetGravity(lua_State *pL) {
    /* event for this */

    m_pMotoGame->createGameEvent(new MGE_SetGravity(m_pMotoGame->getTime(),
                X_luaL_check_number(pL,1),
                X_luaL_check_number(pL,2)));
    return 0;
  }  

  int L_Game_GetGravity(lua_State *pL) {
    /* no event for this */

    /* Get gravity */
    lua_pushnumber(pL, m_pMotoGame->getGravity().x);
    lua_pushnumber(pL, m_pMotoGame->getGravity().y);    
    return 2;
  }

  int L_Game_SetPlayerPosition(lua_State *pL) {
    /* event for this */
    bool bRight = X_luaL_check_number(pL,3) > 0.0f;
    m_pMotoGame->createGameEvent(new MGE_SetPlayerPosition(m_pMotoGame->getTime(),
                 X_luaL_check_number(pL,1),
                 X_luaL_check_number(pL,2),
                 bRight));
    return 0;
  }  
  
  int L_Game_GetPlayerPosition(lua_State *pL) {
    /* no event for this */
    float x = 0.0, y = 0.0;
    DriveDir v_direction = DD_RIGHT;

    if(m_pMotoGame->Players().size() > 0) {
      x = m_pMotoGame->Players()[0]->getState()->CenterP.x;
      y = m_pMotoGame->Players()[0]->getState()->CenterP.y;
      v_direction = m_pMotoGame->Players()[0]->getState()->Dir;
    }

    lua_pushnumber(pL, x);
    lua_pushnumber(pL, y);
    lua_pushnumber(pL, v_direction);

    return 3;
  }

  int L_Game_GetEntityPos(lua_State *pL) {
    /* no event for this */

    /* Find the specified entity and return its position */
    Entity *p = &(m_pMotoGame->getLevelSrc()->getEntityById(luaL_checkstring(pL,1)));
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
  
  int L_Game_GetEntityRadius(lua_State *pL) {
    /* no event for this */
    lua_pushnumber(pL, m_pMotoGame->getLevelSrc()->getEntityById(luaL_checkstring(pL,1)).Size());
    return 1;
  }

  int L_Game_IsEntityTouched(lua_State *pL) {
    /* no event for this */

    bool v_touch = false;
    if(m_pMotoGame->Players().size() > 0) {
      v_touch = m_pMotoGame->Players()[0]->isTouching(m_pMotoGame->getLevelSrc()->getEntityById(luaL_checkstring(pL,1)));
    }

    lua_pushnumber(pL, v_touch? 1:0);
    return 1;
  }


  int L_Game_SetEntityPos(lua_State *pL) {
    /* event for this */
    m_pMotoGame->createGameEvent(new MGE_SetEntityPos(m_pMotoGame->getTime(),
                  luaL_checkstring(pL,1),
                  X_luaL_check_number(pL,2),
                  X_luaL_check_number(pL,3)));
    return 0;
  }  
  
  int L_Game_SetKeyHook(lua_State *pL) {
    /* no event for this */

    if(m_pActiveInputHandler != NULL) {
      m_pActiveInputHandler->addScriptKeyHook(m_pMotoGame,luaL_checkstring(pL,1),luaL_checkstring(pL,2));
    }
    return 0;
  }

  int L_Game_GetKeyByAction(lua_State *pL) {
    /* no event for this */

    if(m_pActiveInputHandler != NULL) {
      lua_pushstring(pL,m_pActiveInputHandler->getKeyByAction(luaL_checkstring(pL,1)).c_str());
      return 1;
    }
    return 0;
  }  

  int L_Game_SetBlockCenter(lua_State *pL) {
    /* event for this */    
    m_pMotoGame->createGameEvent(new MGE_SetBlockCenter(m_pMotoGame->getTime(),
              luaL_checkstring(pL,1),
              X_luaL_check_number(pL,2),
              X_luaL_check_number(pL,3)));
    return 0;
  }  
    
  int L_Game_SetBlockRotation(lua_State *pL) {
    /* event for this */    
    m_pMotoGame->createGameEvent(new MGE_SetBlockRotation(m_pMotoGame->getTime(),
                luaL_checkstring(pL,1),
                X_luaL_check_number(pL,2)));
    return 0;
  }  
    
  int L_Game_SetDynamicEntityRotation(lua_State *pL) {
    /* event for this */    
    m_pMotoGame->createGameEvent(new MGE_SetDynamicEntityRotation(m_pMotoGame->getTime(),
                  luaL_checkstring(pL,1),
                  X_luaL_check_number(pL,2),
                  X_luaL_check_number(pL,3),
                  X_luaL_check_number(pL,4),
                  X_luaL_check_number(pL,5),
                  X_luaL_check_number(pL,6))); 
    return 0;
  }

  int L_Game_SetDynamicEntityTranslation(lua_State *pL) {
    /* event for this */    
    m_pMotoGame->createGameEvent(new MGE_SetDynamicEntityTranslation(m_pMotoGame->getTime(),
                     luaL_checkstring(pL,1),
                     X_luaL_check_number(pL,2),
                     X_luaL_check_number(pL,3),
                     X_luaL_check_number(pL,4),
                     X_luaL_check_number(pL,5),
                     X_luaL_check_number(pL,6)));
    return 0;
  }

  int L_Game_SetDynamicEntityNone(lua_State *pL) {
    /* event for this */    
    m_pMotoGame->createGameEvent(new MGE_SetDynamicEntityNone(m_pMotoGame->getTime(),
                    luaL_checkstring(pL,1)));
    return 0;
  }

  int L_Game_SetDynamicBlockRotation(lua_State *pL) {
    /* event for this */
    m_pMotoGame->createGameEvent(new MGE_SetDynamicBlockRotation(m_pMotoGame->getTime(),
                 luaL_checkstring(pL,1),
                 X_luaL_check_number(pL,2),
                 X_luaL_check_number(pL,3),
                 X_luaL_check_number(pL,4),
                 X_luaL_check_number(pL,5),
                 X_luaL_check_number(pL,6))); 
    return 0;
  }

  int L_Game_SetDynamicBlockTranslation(lua_State *pL) {
    /* event for this */    
    m_pMotoGame->createGameEvent(new MGE_SetDynamicBlockTranslation(m_pMotoGame->getTime(),
                    luaL_checkstring(pL,1),
                    X_luaL_check_number(pL,2),
                    X_luaL_check_number(pL,3),
                    X_luaL_check_number(pL,4),
                    X_luaL_check_number(pL,5),
                    X_luaL_check_number(pL,6)));
    return 0;
  }

  int L_Game_SetDynamicBlockNone(lua_State *pL) {
    /* event for this */    
    m_pMotoGame->createGameEvent(new MGE_SetDynamicBlockNone(m_pMotoGame->getTime(),
                   luaL_checkstring(pL,1)));
    return 0;
  }

  int L_Game_CameraZoom(lua_State *pL) {
    /* event for this */
    m_pMotoGame->createGameEvent(new MGE_CameraZoom(m_pMotoGame->getTime(),
                X_luaL_check_number(pL,1)));
    return 0;
  }

  int L_Game_CameraMove(lua_State *pL) {
    /* event for this */
    m_pMotoGame->createGameEvent(new MGE_CameraMove(m_pMotoGame->getTime(),
                X_luaL_check_number(pL,1),
                X_luaL_check_number(pL,2)));
    return 0;
  }

  int L_Game_KillPlayer(lua_State *pL) {
    m_pMotoGame->createGameEvent(new MGE_PlayerDies(m_pMotoGame->getTime(), false));
    return 0;
  }

  int L_Game_KillEntity(lua_State *pL) {
    m_pMotoGame->createKillEntityEvent(luaL_checkstring(pL,1));
    return 0;
  }

  int L_Game_RemainingStrawberries(lua_State *pL) {
    /* no event for this */
    lua_pushnumber(pL,m_pMotoGame->getNbRemainingStrawberries());
    return 1;
  }

  int L_Game_WinPlayer(lua_State *pL) {
    m_pMotoGame->makePlayerWin();
    return 1;
  }

  int L_Game_PenaltyTime(lua_State *pL) {
    /* event for this */
    m_pMotoGame->createGameEvent(new MGE_PenalityTime(m_pMotoGame->getTime(),
                  X_luaL_check_number(pL,1)));
    return 0;
  }
}

