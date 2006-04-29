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
  Game.* 
    Lua game library functions
  ===========================================================================*/
  int L_Game_Log(lua_State *pL) {
    std::string Out;
    for(int i=0;i<lua_gettop(pL);i++) 
      Out.append(luaL_checkstring(pL,i+1));
    Log((char *)Out.c_str());    
    return 0;    
  }
  
  int L_Game_ClearMessages(lua_State *pL) {
    m_pMotoGame->clearGameMessages();
    return 0;
  }

  int L_Game_PlaceInGameArrow(lua_State *pL) {
    m_pMotoGame->getArrowPointer().nArrowPointerMode = 1;
    m_pMotoGame->getArrowPointer().ArrowPointerPos = Vector2f(luaL_check_number(pL,1),luaL_check_number(pL,2));
    m_pMotoGame->getArrowPointer().fArrowPointerAngle = luaL_check_number(pL,3);
    return 0;
  }

  int L_Game_PlaceScreenArrow(lua_State *pL) {
    m_pMotoGame->getArrowPointer().nArrowPointerMode = 2;
    m_pMotoGame->getArrowPointer().ArrowPointerPos = Vector2f(luaL_check_number(pL,1),luaL_check_number(pL,2));
    m_pMotoGame->getArrowPointer().fArrowPointerAngle = luaL_check_number(pL,3);
    return 0;
  }

  int L_Game_HideArrow(lua_State *pL) {
    m_pMotoGame->getArrowPointer().nArrowPointerMode = 0;
    return 0;
  }
  
  int L_Game_GetTime(lua_State *pL) {
    /* Get current game time */
    lua_pushnumber(pL,m_pMotoGame->getTime());
    return 1;
  }
  
  int L_Game_Message(lua_State *pL) {    
    /* Convert all arguments to strings, and spit them out */
    std::string Out;
    for(int i=0;i<lua_gettop(pL);i++) 
      Out.append(luaL_checkstring(pL,i+1));
    m_pMotoGame->gameMessage(Out);    
    return 0;
  }  
  
  int L_Game_IsPlayerInZone(lua_State *pL) {    
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
    /* Find the specified block and move it along the given vector */
    for(int i=0;i<m_pMotoGame->getBlocks().size();i++) {
      ConvexBlock *pBlock = m_pMotoGame->getBlocks()[i];
      if(pBlock->pSrcBlock->ID == luaL_checkstring(pL,1)) {
        pBlock->pSrcBlock->fPosX += luaL_check_number(pL,2);
        pBlock->pSrcBlock->fPosY += luaL_check_number(pL,3);
        break;
      }
    }
    return 0;
  }
  
  int L_Game_GetBlockPos(lua_State *pL) {
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
    /* Find the specified block and set its position */
    for(int i=0;i<m_pMotoGame->getBlocks().size();i++) {
      ConvexBlock *pBlock = m_pMotoGame->getBlocks()[i];
      if(pBlock->pSrcBlock->ID == luaL_checkstring(pL,1)) {
        pBlock->pSrcBlock->fPosX = luaL_check_number(pL,2);
        pBlock->pSrcBlock->fPosY = luaL_check_number(pL,3);
        break;
      }
    }
    return 0;
  }  

  int L_Game_SetGravity(lua_State *pL) {
    /* Set gravity */
    m_pMotoGame->setGravity(luaL_check_number(pL,1),luaL_check_number(pL,2));    
    return 0;
  }  

  int L_Game_GetGravity(lua_State *pL) {
    /* Get gravity */
    lua_pushnumber(pL,m_pMotoGame->getGravity().x);
    lua_pushnumber(pL,m_pMotoGame->getGravity().y);    
    return 2;
  }

  int L_Game_SetPlayerPosition(lua_State *pL) {
    /* Set player position */  
    bool bRight = false;
    if(luaL_check_number(pL,3) > 0.0f) bRight = true;
    
    m_pMotoGame->setPlayerPosition(luaL_check_number(pL,1),luaL_check_number(pL,2),bRight);    
    return 0;
  }  
  
  int L_Game_GetPlayerPosition(lua_State *pL) {
    /* Get player position */
    lua_pushnumber(pL,m_pMotoGame->getPlayerPosition().x);
    lua_pushnumber(pL,m_pMotoGame->getPlayerPosition().y);
    lua_pushnumber(pL,m_pMotoGame->getPlayerFaceDir());
    return 3;
  }

  int L_Game_GetEntityPos(lua_State *pL) {
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
    /* Find the specified entity and set its position */
    for(int i=0;i<m_pMotoGame->getEntities().size();i++) {
      Entity *p = m_pMotoGame->getEntities()[i];
      if(p->ID == luaL_checkstring(pL,1)) {
        p->Pos.x = luaL_checknumber(pL,2);
        p->Pos.y = luaL_checknumber(pL,3);
        return 0;
      }
    }
    /* Entity not found */
    return 0;
  }  
  
  int L_Game_SetKeyHook(lua_State *pL) {
    if(m_pActiveInputHandler != NULL) {
      m_pActiveInputHandler->addScriptKeyHook(m_pMotoGame,luaL_checkstring(pL,1),luaL_checkstring(pL,2));
    }
    return 0;
  }

  int L_Game_GetKeyByAction(lua_State *pL) {
    if(m_pActiveInputHandler != NULL) {
      lua_pushstring(pL,m_pActiveInputHandler->getKeyByAction(luaL_checkstring(pL,1)).c_str());
      return 1;
    }
    return 0;
  }  
    
};

