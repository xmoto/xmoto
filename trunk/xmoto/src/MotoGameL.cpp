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
    GameEvent *pEvent = m_pMotoGame->createGameEvent(GAME_EVENT_LUA_CALL_CLEARMESSAGES);
    return 0;
  }

  int L_Game_PlaceInGameArrow(lua_State *pL) {
    /* event for this */
    GameEvent *pEvent = m_pMotoGame->createGameEvent(GAME_EVENT_LUA_CALL_PLACEINGAMEARROW);
    if(pEvent != NULL) {
      pEvent->u.LuaCallPlaceingamearrow.x     = X_luaL_check_number(pL,1);
      pEvent->u.LuaCallPlaceingamearrow.y     = X_luaL_check_number(pL,2);
      pEvent->u.LuaCallPlaceingamearrow.angle = X_luaL_check_number(pL,3);    
    }
    return 0;
  }

  int L_Game_PlaceScreenArrow(lua_State *pL) {
    /* event for this */
    GameEvent *pEvent = m_pMotoGame->createGameEvent(GAME_EVENT_LUA_CALL_PLACESCREENARROW);
    if(pEvent != NULL) {
      pEvent->u.LuaCallPlacescreenarrow.x     = X_luaL_check_number(pL,1);
      pEvent->u.LuaCallPlacescreenarrow.y     = X_luaL_check_number(pL,2);
      pEvent->u.LuaCallPlacescreenarrow.angle = X_luaL_check_number(pL,3);    
    }

    return 0;
  }

  int L_Game_HideArrow(lua_State *pL) {
    /* event for this */
    GameEvent *pEvent = m_pMotoGame->createGameEvent(GAME_EVENT_LUA_CALL_HIDEARROW);
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
      Out.append(luaL_checkstring(pL,i+1));
  
    GameEvent *pEvent = m_pMotoGame->createGameEvent(GAME_EVENT_LUA_CALL_MESSAGE);
    if(pEvent != NULL) {
      strncpy(pEvent->u.LuaCallMessage.cMessage,
	      Out.c_str(),
	      sizeof(pEvent->u.LuaCallMessage.cMessage)-1);
    }
   
    return 0;
  }  
  
  int L_Game_IsPlayerInZone(lua_State *pL) {
    /* no event for this */
   
    int nRet = FALSE;
  
    /* Check whether the player is in the specified zone */
    for(int i=0;i<m_pMotoGame->getLevelSrc()->getZoneList().size();i++) {
      LevelZone *pZone = m_pMotoGame->getLevelSrc()->getZoneList()[i];
      if(pZone->ID == luaL_checkstring(pL,1) && pZone->m_bInZone) {
        nRet = TRUE;
      }
    }    
    lua_pushboolean(pL,nRet);
    return 1;
  }
  
  int L_Game_MoveBlock(lua_State *pL) {
    /* event for this */

    GameEvent *pEvent = m_pMotoGame->createGameEvent(GAME_EVENT_LUA_CALL_MOVEBLOCK);
    if(pEvent != NULL) {
      strncpy(pEvent->u.LuaCallMoveblock.cBlockID,
	      luaL_checkstring(pL,1),
	      sizeof(pEvent->u.LuaCallMoveblock.cBlockID)-1);
      pEvent->u.LuaCallMoveblock.x = X_luaL_check_number(pL,2);
      pEvent->u.LuaCallMoveblock.y = X_luaL_check_number(pL,3);
    }

    return 0;
  }
  
  int L_Game_GetBlockPos(lua_State *pL) {
    /* no event for this */

    /* Find the specified block and return its position */
    for(int i=0;i<m_pMotoGame->getBlocks().size();i++) {
      ConvexBlock *pBlock = m_pMotoGame->getBlocks()[i];
      if(pBlock->pSrcBlock->ID == luaL_checkstring(pL,1)) {
        lua_pushnumber(pL,pBlock->pSrcBlock->fPosX);
        lua_pushnumber(pL,pBlock->pSrcBlock->fPosY);
        return 2;
      }
    }
    /* Block not found, return <0,0> */
    lua_pushnumber(pL,0);
    lua_pushnumber(pL,0);
    return 2;
  }
  
  int L_Game_SetBlockPos(lua_State *pL) {
    /* event for this */

    GameEvent *pEvent = m_pMotoGame->createGameEvent(GAME_EVENT_LUA_CALL_SETBLOCKPOS);
    if(pEvent != NULL) {
      strncpy(pEvent->u.LuaCallSetblockpos.cBlockID,
	      luaL_checkstring(pL,1),
	      sizeof(pEvent->u.LuaCallSetblockpos.cBlockID)-1);
      pEvent->u.LuaCallSetblockpos.x = X_luaL_check_number(pL,2);
      pEvent->u.LuaCallSetblockpos.y = X_luaL_check_number(pL,3);
    }

    return 0;
  }  

  int L_Game_SetGravity(lua_State *pL) {
    /* event for this */

    GameEvent *pEvent = m_pMotoGame->createGameEvent(GAME_EVENT_LUA_CALL_SETGRAVITY);
    if(pEvent != NULL) {
      pEvent->u.LuaCallSetgravity.x = X_luaL_check_number(pL,1);
      pEvent->u.LuaCallSetgravity.y = X_luaL_check_number(pL,2);
    }
      
    return 0;
  }  

  int L_Game_GetGravity(lua_State *pL) {
    /* no event for this */

    /* Get gravity */
    lua_pushnumber(pL,m_pMotoGame->getGravity().x);
    lua_pushnumber(pL,m_pMotoGame->getGravity().y);    
    return 2;
  }

  int L_Game_SetPlayerPosition(lua_State *pL) {
    /* event for this */

    GameEvent *pEvent = m_pMotoGame->createGameEvent(GAME_EVENT_LUA_CALL_SETPLAYERPOSITION);
    if(pEvent != NULL) {
      bool bRight = false;
      if(X_luaL_check_number(pL,3) > 0.0f) bRight = true;

      pEvent->u.LuaCallSetplayerposition.x      = X_luaL_check_number(pL,1);
      pEvent->u.LuaCallSetplayerposition.y      = X_luaL_check_number(pL,2);
      pEvent->u.LuaCallSetplayerposition.bRight = bRight;
    }
    return 0;
  }  
  
  int L_Game_GetPlayerPosition(lua_State *pL) {
    /* no event for this */

    /* Get player position */
    lua_pushnumber(pL,m_pMotoGame->getPlayerPosition().x);
    lua_pushnumber(pL,m_pMotoGame->getPlayerPosition().y);
    lua_pushnumber(pL,m_pMotoGame->getPlayerFaceDir());
    return 3;
  }

  int L_Game_GetEntityPos(lua_State *pL) {
    /* no event for this */

    /* Find the specified entity and return its position */
    for(int i=0;i<m_pMotoGame->getEntities().size();i++) {
      Entity *p = m_pMotoGame->getEntities()[i];
      if(p->ID == luaL_checkstring(pL,1)) {
        lua_pushnumber(pL,p->Pos.x);
        lua_pushnumber(pL,p->Pos.y);
        return 2;
      }
    }
    /* Entity not found, return <0,0> */
    lua_pushnumber(pL,0);
    lua_pushnumber(pL,0);
    return 2;
  }  

  int L_Game_SetEntityPos(lua_State *pL) {
    /* event for this */
    GameEvent *pEvent = m_pMotoGame->createGameEvent(GAME_EVENT_LUA_CALL_SETENTITYPOS);
    if(pEvent != NULL) {
      strncpy(pEvent->u.LuaCallSetentitypos.cEntityID,
	      luaL_checkstring(pL,1),
	      sizeof(pEvent->u.LuaCallSetentitypos.cEntityID)-1);
      pEvent->u.LuaCallSetentitypos.x = X_luaL_check_number(pL,2);
      pEvent->u.LuaCallSetentitypos.y = X_luaL_check_number(pL,3);
      
    }
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
    GameEvent *pEvent = m_pMotoGame->createGameEvent(GAME_EVENT_LUA_CALL_SETBLOCKCENTER);
    if(pEvent != NULL) {
      strncpy(pEvent->u.LuaCallSetBlockCenter.cBlockID,
	      luaL_checkstring(pL,1),
	      sizeof(pEvent->u.LuaCallSetBlockCenter.cBlockID)-1);
      pEvent->u.LuaCallSetBlockCenter.x = X_luaL_check_number(pL,2);
      pEvent->u.LuaCallSetBlockCenter.y = X_luaL_check_number(pL,3);
    }

    return 0;
  }  
    
  int L_Game_SetBlockRotation(lua_State *pL) {
    /* event for this */    
    GameEvent *pEvent = m_pMotoGame->createGameEvent(GAME_EVENT_LUA_CALL_SETBLOCKROTATION);
    if(pEvent != NULL) {
      strncpy(pEvent->u.LuaCallSetBlockRotation.cBlockID,
	      luaL_checkstring(pL,1),
	      sizeof(pEvent->u.LuaCallSetBlockRotation.cBlockID)-1);
      pEvent->u.LuaCallSetBlockRotation.fAngle = X_luaL_check_number(pL,2);
    }

    return 0;
  }  
    
  int L_Game_SetDynamicEntityRotation(lua_State *pL) {
    /* event for this */    
    GameEvent *pEvent = m_pMotoGame->createGameEvent(GAME_EVENT_LUA_CALL_SETDYNAMICENTITYROTATION);
    if(pEvent != NULL) {
      strncpy(pEvent->u.LuaCallSetDynamicEntityRotation.cEntityID,
	      luaL_checkstring(pL,1),
	      sizeof(pEvent->u.LuaCallSetDynamicEntityRotation.cEntityID)-1);
      pEvent->u.LuaCallSetDynamicEntityRotation.fCenterX = X_luaL_check_number(pL,2);
      pEvent->u.LuaCallSetDynamicEntityRotation.fCenterY = X_luaL_check_number(pL,3);
      pEvent->u.LuaCallSetDynamicEntityRotation.fInitAngle = X_luaL_check_number(pL,4);
      pEvent->u.LuaCallSetDynamicEntityRotation.fRadius    = X_luaL_check_number(pL,5);
      pEvent->u.LuaCallSetDynamicEntityRotation.fSpeed     = X_luaL_check_number(pL,6);
      pEvent->u.LuaCallSetDynamicEntityRotation.startTime  = X_luaL_check_number(pL,7);
      pEvent->u.LuaCallSetDynamicEntityRotation.endTime    = X_luaL_check_number(pL,8);
    }
    
    return 0;
  }

};

