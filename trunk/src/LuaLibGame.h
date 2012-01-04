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

#ifndef __LUALIBGAME_H__
#define __LUALIBGAME_H__

#include <string>
#include "LuaLibBase.h"

class Scene;
class InputHandler;

class LuaLibGame : public LuaLibBase {
public:
  LuaLibGame(Scene *i_pScene);
  ~LuaLibGame();

protected:
  /*
    static lua lib calls are share between the lulibgame instances
    then, setInstance must be set before lua call
  */
  void setInstance();

private:
  Scene* m_pScene;
  InputHandler *m_pActiveInputHandler;

  static InputHandler* m_exec_activeInputHandler;
  static Scene*        m_exec_world;
  static luaL_Reg      m_gameFuncs[];

  /* Lua library prototypes */
  static int L_Game_GetTime(lua_State *pL);
  static int L_Game_Message(lua_State *pL);
  static int L_Game_IsPlayerInZone(lua_State *pL);
  static int L_Game_MoveBlock(lua_State *pL);
  static int L_Game_GetBlockPos(lua_State *pL);
  static int L_Game_SetBlockPos(lua_State *pL);
  static int L_Game_PlaceInGameArrow(lua_State *pL);
  static int L_Game_PlaceScreenArrow(lua_State *pL);
  static int L_Game_HideArrow(lua_State *pL);
  static int L_Game_ClearMessages(lua_State *pL);
  static int L_Game_SetGravity(lua_State *pL);  
  static int L_Game_GetGravity(lua_State *pL);  
  static int L_Game_SetPlayerPosition(lua_State *pL);
  static int L_Game_GetPlayerPosition(lua_State *pL);
  static int L_Game_AddForceToPlayer(lua_State *pL);
  static int L_Game_GetEntityPos(lua_State *pL);
  static int L_Game_SetEntityPos(lua_State *pL);
  static int L_Game_SetKeyHook(lua_State *pL);
  static int L_Game_GetKeyByAction(lua_State *pL);
  static int L_Game_GetKeyByActionTech(lua_State *pL);
  static int L_Game_Log(lua_State *pL);
  static int L_Game_SetBlockCenter(lua_State *pL);
  static int L_Game_SetBlockRotation(lua_State *pL);
  static int L_Game_SetDynamicEntityRotation(lua_State *pL);
  static int L_Game_SetDynamicEntitySelfRotation(lua_State *pL);
  static int L_Game_SetDynamicEntityTranslation(lua_State *pL);
  static int L_Game_SetDynamicEntityNone(lua_State *pL);
  static int L_Game_SetDynamicBlockRotation(lua_State *pL);
  static int L_Game_SetDynamicBlockSelfRotation(lua_State *pL);
  static int L_Game_SetPhysicsBlockSelfRotation(lua_State *pL);
  static int L_Game_SetPhysicsBlockTranslation(lua_State* pL);
  static int L_Game_SetDynamicBlockTranslation(lua_State *pL);
  static int L_Game_SetDynamicBlockNone(lua_State *pL);
  static int L_Game_CameraZoom(lua_State *pL);
  static int L_Game_CameraMove(lua_State *pL);
  static int L_Game_GetEntityRadius(lua_State *pL);
  static int L_Game_IsEntityTouched(lua_State *pL);
  static int L_Game_KillPlayer(lua_State *pL);
  static int L_Game_KillEntity(lua_State *pL);
  static int L_Game_RemainingStrawberries(lua_State *pL);
  static int L_Game_WinPlayer(lua_State *pL);
  static int L_Game_PenaltyTime(lua_State *pL);
  static int L_Game_IsAPlayerInZone(lua_State *pL);
  static int L_Game_SetAPlayerPosition(lua_State *pL);
  static int L_Game_GetAPlayerPosition(lua_State *pL);
  static int L_Game_KillAPlayer(lua_State *pL);
  static int L_Game_WinAPlayer(lua_State *pL);
  static int L_Game_NumberOfPlayers(lua_State *pL);
  static int L_Game_CameraRotate(lua_State *pL);
  static int L_Game_CameraAdaptToGravity(lua_State *pL);
  static int L_Game_SetCameraRotationSpeed(lua_State *pL);
  static int L_Game_PlaySound(lua_State *pL);
  static int L_Game_PlayMusic(lua_State *pL);
  static int L_Game_StopMusic(lua_State *pL);
  static int L_Game_GetPlayerVelocity(lua_State *pL);
  static int L_Game_GetPlayerSpeed(lua_State *pL);
  static int L_Game_GetPlayerAngle(lua_State *pL);
  static int L_Game_SetCameraPosition(lua_State *pL);
  static int L_Game_GetPlayerProfileName(lua_State *pL);
  /* Script Timer Functions*/
  static int L_Game_StartTimer(lua_State *pL);  
  static int L_Game_SetTimerDelay(lua_State *pL);  
  static int L_Game_StopTimer(lua_State *pL);
};


#endif
