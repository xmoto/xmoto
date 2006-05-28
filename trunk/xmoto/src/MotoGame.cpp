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
 *  Game object. Handles all of the gamestate management und so weiter.
 */
#include "Game.h"
#include "MotoGame.h"
#include "VFileIO.h"
#include "BSP.h"
#include "Sound.h"
#include "PhysSettings.h"

namespace vapp {

  /*===========================================================================
    Globals
    ===========================================================================*/

  /* Prim. game object */
  MotoGame *m_pMotoGame;        
  
  /* Lua library prototypes */
  int L_Game_GetTime(lua_State *pL);
  int L_Game_Message(lua_State *pL);
  int L_Game_IsPlayerInZone(lua_State *pL);
  int L_Game_MoveBlock(lua_State *pL);
  int L_Game_GetBlockPos(lua_State *pL);
  int L_Game_SetBlockPos(lua_State *pL);
  int L_Game_PlaceInGameArrow(lua_State *pL);
  int L_Game_PlaceScreenArrow(lua_State *pL);
  int L_Game_HideArrow(lua_State *pL);
  int L_Game_ClearMessages(lua_State *pL);
  int L_Game_SetGravity(lua_State *pL);  
  int L_Game_GetGravity(lua_State *pL);  
  int L_Game_SetPlayerPosition(lua_State *pL);
  int L_Game_GetPlayerPosition(lua_State *pL);
  int L_Game_GetEntityPos(lua_State *pL);
  int L_Game_SetEntityPos(lua_State *pL);
  int L_Game_SetKeyHook(lua_State *pL);
  int L_Game_GetKeyByAction(lua_State *pL);
  int L_Game_Log(lua_State *pL);
  
  /* "Game" Lua library */
  static const luaL_reg g_GameFuncs[] = {
    {"GetTime", L_Game_GetTime},
    {"Message", L_Game_Message},
    {"IsPlayerInZone", L_Game_IsPlayerInZone},
    {"MoveBlock", L_Game_MoveBlock},
    {"GetBlockPos", L_Game_GetBlockPos},
    {"SetBlockPos", L_Game_SetBlockPos},
    {"PlaceInGameArrow", L_Game_PlaceInGameArrow},
    {"PlaceScreenArrow", L_Game_PlaceScreenArrow},
    {"HideArrow", L_Game_HideArrow},
    {"ClearMessages", L_Game_ClearMessages},
    {"SetGravity", L_Game_SetGravity},
    {"GetGravity", L_Game_GetGravity},
    {"SetPlayerPosition", L_Game_SetPlayerPosition},
    {"GetPlayerPosition", L_Game_GetPlayerPosition},
    {"GetEntityPos", L_Game_GetEntityPos},
    {"SetEntityPos", L_Game_SetEntityPos},
    {"SetKeyHook", L_Game_SetKeyHook},
    {"GetKeyByAction", L_Game_GetKeyByAction},
    {"Log", L_Game_Log},
    {NULL, NULL}
  };

  /*===========================================================================
    Init the lua game lib
    ===========================================================================*/
  LUALIB_API int luaopen_Game(lua_State *pL) {
    luaL_openlib(pL,"Game",g_GameFuncs,0);
    return 1;
  }

  /*===========================================================================
    Simple lua interaction
    ===========================================================================*/
  bool MotoGame::scriptCallBool(std::string FuncName,bool bDefault) {
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
  
  void MotoGame::scriptCallVoid(std::string FuncName) {
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
  
  void MotoGame::scriptCallTblVoid(std::string Table,std::string FuncName) {
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

  /*===========================================================================
    Teleporting
    ===========================================================================*/
  void MotoGame::setPlayerPosition(float x,float y,bool bFaceRight) {
    /* Request teleport next frame */
    m_TeleportDest.bDriveRight = bFaceRight;
    m_TeleportDest.Pos = Vector2f(x,y);
    m_bTeleport = true;
  }
  
  const Vector2f &MotoGame::getPlayerPosition(void) {
    return m_BikeS.CenterP;
  }
  
  bool MotoGame::getPlayerFaceDir(void) {
    if(m_BikeS.Dir == DD_RIGHT) return true;
    else return false;
  }

  /*===========================================================================
    Dummy adding
    ===========================================================================*/
  void MotoGame::addDummy(Vector2f Pos,float r,float g,float b) {
    m_Dummies[m_nNumDummies].Pos = Pos;
    m_Dummies[m_nNumDummies].r = r;
    m_Dummies[m_nNumDummies].g = g;
    m_Dummies[m_nNumDummies].b = b;
    m_nNumDummies++;
  }

  /*===========================================================================
    Add game message
    ===========================================================================*/
  void MotoGame::gameMessage(std::string Text) {
    GameMessage *pMsg = new GameMessage;
    pMsg->fRemoveTime = getTime() + 5.0f;
    pMsg->bNew = true;
    pMsg->nAlpha = 255;
    pMsg->Text = Text;
    m_GameMessages.push_back(pMsg);
  }

  void MotoGame::clearGameMessages(void) {
    for(int i=0;i<m_GameMessages.size();i++)
      m_GameMessages[i]->fRemoveTime=0.0f;
  }

  /*===========================================================================
    Update game
    ===========================================================================*/
  void MotoGame::updateLevel(float fTimeStep,SerializedBikeState *pReplayState,DBuffer *pDBuffer) {
    bool v_enableScript = pReplayState == NULL; // don't use script on replay (lua action must be read from the replay)

    /* Dummies are small markers that can show different things during debugging */
    resetDummies();
    
    /* Going to teleport? Do it now, before we tinker to much with the state */
    if(m_bTeleport) {
      /* Clear stuff */
      clearStates();    
      
      m_fLastAttitudeCon = -1000.0f;
      m_fAttitudeCon = 0.0f;
      
      m_PlayerFootAnchorBodyID = NULL;
      m_PlayerHandAnchorBodyID = NULL;
      m_PlayerTorsoBodyID = NULL;
      m_PlayerUArmBodyID = NULL;
      m_PlayerLArmBodyID = NULL;
      m_PlayerULegBodyID = NULL;
      m_PlayerLLegBodyID = NULL;
      m_PlayerFootAnchorBodyID2 = NULL;
      m_PlayerHandAnchorBodyID2 = NULL;
      m_PlayerTorsoBodyID2 = NULL;
      m_PlayerUArmBodyID2 = NULL;
      m_PlayerLArmBodyID2 = NULL;
      m_PlayerULegBodyID2 = NULL;
      m_PlayerLLegBodyID2 = NULL;

      /* Restart physics */
      _UninitPhysics();
      _InitPhysics();

      /* Calculate bike stuff */
      _CalculateBikeAnchors();    
      Vector2f C( m_TeleportDest.Pos.x - m_BikeA.Tp.x, m_TeleportDest.Pos.y - m_BikeA.Tp.y);
      _PrepareBikePhysics(C);
          
      m_BikeS.Dir = m_TeleportDest.bDriveRight?DD_RIGHT:DD_LEFT;
      
      m_BikeS.fCurBrake = m_BikeS.fCurEngine = 0.0f;
      
      m_bTeleport = false;
    }
    
    /* Handle game messages (keep them in place) */
    int i=0;
    while(1) {
      if(i >= m_GameMessages.size()) break;      
      if(getTime() > m_GameMessages[i]->fRemoveTime) {
        m_GameMessages[i]->nAlpha -= 2;
        
        if(m_GameMessages[i]->nAlpha <= 0) {
          delete m_GameMessages[i];
          m_GameMessages.erase(m_GameMessages.begin() + i);
          i--;
          continue;
        }
      }
      Vector2f TargetPos = Vector2f(0.2f,0.5f - (m_GameMessages.size()*0.05f)/2.0f + 0.05f*i);
      if(m_GameMessages[i]->bNew) {
        m_GameMessages[i]->Vel = Vector2f(0,0);
        m_GameMessages[i]->Pos = TargetPos;
        m_GameMessages[i]->bNew = false;
      }
      else {
        if(!TargetPos.almostEqual(m_GameMessages[i]->Pos)) {
          Vector2f F = TargetPos - m_GameMessages[i]->Pos;      
          m_GameMessages[i]->Vel += F*0.0005f;
          m_GameMessages[i]->Pos += m_GameMessages[i]->Vel;
        }
      }
      i++;
    }
    
    /* Increase time */
    if(pReplayState == NULL)
      m_fTime += fTimeStep;
    else
      m_fTime = pReplayState->fGameTime;
    
    /* Are we going to change direction during this update? */
    bool bChangeDir = false;
    if(m_BikeC.bChangeDir) {
      m_BikeC.bChangeDir = false;
      bChangeDir = true;
      
      m_BikeS.Dir = m_BikeS.Dir==DD_LEFT?DD_RIGHT:DD_LEFT; /* switch */
    }
  
    /* Update game state */
    _UpdateGameState(pReplayState);
        
    /* Update misc stuff (only when not playing a replay) */
    if(pReplayState == NULL) {
      _UpdateZones();
      _UpdateEntities();
    }
    
    /* Invoke PreDraw() script function */
    if(v_enableScript) {
      if(!scriptCallBool("PreDraw",
			 true)) {
	throw Exception("level script PreDraw() returned false");
      }
    } 

    /* Only make a full physics update when not replaying */
    if(pReplayState == NULL) {
      /* Update physics */
      _UpdatePhysics(fTimeStep);
          
      /* Handle events generated this update */
      while(getNumPendingGameEvents() > 0) {
	GameEvent *pEvent = getNextGameEvent();
	if(pEvent != NULL) {
	  if(pDBuffer != NULL) {
	    /* Encode event */
	    _SerializeGameEventQueue(*pDBuffer,pEvent);
	  }
			  
	  /* What event? */
	  switch(pEvent->Type) {
	  case GAME_EVENT_PLAYER_DIES:
	    {
	      m_bDead = true;
	    }
	    break;
	  case GAME_EVENT_PLAYER_ENTERS_ZONE:						
	    {
	      /* Notify script */
	      if(v_enableScript) {
		scriptCallTblVoid( pEvent->u.PlayerEntersZone.pZone->ID,"OnEnter" );
	      }
	    }
	    break;
	  case GAME_EVENT_PLAYER_LEAVES_ZONE:
	    {
	      if(v_enableScript) {
		/* Notify script */
		scriptCallTblVoid( pEvent->u.PlayerEntersZone.pZone->ID,"OnLeave" );						
	      }
	    }
	    break;
	  case GAME_EVENT_PLAYER_TOUCHES_ENTITY:
	    {
	      Entity *pEntityToTouch = findEntity(pEvent->u.PlayerTouchesEntity.cEntityID);
	      if(pEntityToTouch != NULL) {
		touchEntity(pEntityToTouch,pEvent->u.PlayerTouchesEntity.bHead, v_enableScript);
	      }
	    }
	    break;
	  case GAME_EVENT_ENTITY_DESTROYED: 
	    {
	      /* Destroy entity */
	      Entity *pEntityToDestroy = findEntity(pEvent->u.EntityDestroyed.cEntityID);
	      if(pEntityToDestroy != NULL) {
		deleteEntity(pEntityToDestroy);
	      }
	    }
	    break;

	  case GAME_EVENT_LUA_CALL_SETENTITYPOS:
	    {
	      _SetEntityPos(pEvent->u.LuaCallSetentitypos.cEntityID,
			    pEvent->u.LuaCallSetentitypos.x,
			    pEvent->u.LuaCallSetentitypos.y);
	    }
	    break;

	  case GAME_EVENT_LUA_CALL_CLEARMESSAGES:
	    {
	      clearGameMessages();
	    }
	    break;

	  case GAME_EVENT_LUA_CALL_PLACEINGAMEARROW:
	    {
	      _PlaceInGameArrow(pEvent->u.LuaCallPlaceingamearrow.x,
				pEvent->u.LuaCallPlaceingamearrow.y,
				pEvent->u.LuaCallPlaceingamearrow.angle);
	    }
	    break;

	  case GAME_EVENT_LUA_CALL_PLACESCREENARROW:
	    {
	      _PlaceScreenArrow(pEvent->u.LuaCallPlacescreenarrow.x,
				pEvent->u.LuaCallPlacescreenarrow.y,
				pEvent->u.LuaCallPlacescreenarrow.angle);  
	    }
	    break;

	  case GAME_EVENT_LUA_CALL_HIDEARROW:
	    {
	      getArrowPointer().nArrowPointerMode = 0;
	    }
	    break;

	  case GAME_EVENT_LUA_CALL_MESSAGE:
	    {
	      gameMessage(pEvent->u.LuaCallMessage.cMessage);
	    }
	    break;

	  case GAME_EVENT_LUA_CALL_MOVEBLOCK:
	    {
	      _MoveBlock(pEvent->u.LuaCallMoveblock.cBlockID,
			 pEvent->u.LuaCallMoveblock.x,
			 pEvent->u.LuaCallMoveblock.y);
	    }
	    break;

	  case GAME_EVENT_LUA_CALL_SETBLOCKPOS:
	    {
	      _SetBlockPos(pEvent->u.LuaCallSetblockpos.cBlockID,
			   pEvent->u.LuaCallSetblockpos.x,
			   pEvent->u.LuaCallSetblockpos.y);
	    }
	    break;

	  case GAME_EVENT_LUA_CALL_SETGRAVITY:
	    {
	      setGravity(pEvent->u.LuaCallSetgravity.x,
			 pEvent->u.LuaCallSetgravity.y);
	    }
	    break;

	  case GAME_EVENT_LUA_CALL_SETPLAYERPOSITION:
	    {
	      m_pMotoGame->setPlayerPosition(pEvent->u.LuaCallSetplayerposition.x,
					     pEvent->u.LuaCallSetplayerposition.y,
					     pEvent->u.LuaCallSetplayerposition.bRight);
	    }
	    break;

	  }
	}
	else break;
      }
    }
    else {
      /* Well, handle replay events instead */
      _UpdateReplayEvents();
    }

    /* Entities scheduled for termination? */
    for(int i=0;i<m_DelSchedule.size();i++)
      _KillEntity(m_DelSchedule[i]);
    m_DelSchedule.clear();
    
    /* Remember bike pos for next time */
    //m_PrevFrontWheelP = m_BikeS.FrontWheelP;
    //m_PrevRearWheelP = m_BikeS.RearWheelP;
  }

  /*===========================================================================
    State/stuff clearing
    ===========================================================================*/
  void MotoGame::clearStates(void) {      
    /* BIKE_S */
    m_BikeS.CenterP = Vector2f(0,0);
    m_BikeS.Dir = DD_RIGHT;
    m_BikeS.fBikeEngineRPM = 0.0f;
    m_BikeS.Elbow2P = Vector2f(0,0);
    m_BikeS.ElbowP = Vector2f(0,0);
    m_BikeS.fCurBrake = 0.0f;
    m_BikeS.fCurEngine = 0.0f;
    m_BikeS.Foot2P = Vector2f(0,0);
    m_BikeS.FootP = Vector2f(0,0);
    m_BikeS.FrontAnchor2P = Vector2f(0,0);
    m_BikeS.FrontAnchorP = Vector2f(0,0);
    m_BikeS.FrontWheelP = Vector2f(0,0);
    m_BikeS.Hand2P = Vector2f(0,0);
    m_BikeS.HandP = Vector2f(0,0);
    m_BikeS.Head2P = Vector2f(0,0);
    m_BikeS.HeadP = Vector2f(0,0);
    m_BikeS.Knee2P = Vector2f(0,0);
    m_BikeS.KneeP = Vector2f(0,0);
    m_BikeS.LowerBody2P = Vector2f(0,0);
    m_BikeS.LowerBodyP = Vector2f(0,0);
    m_BikeS.pAnchors = NULL;
    //m_BikeS.pfFramePos = NULL;
    //m_BikeS.pfFrameRot = NULL;
    //m_BikeS.pfFrontWheelPos = NULL;
    //m_BikeS.pfFrontWheelRot = NULL;
    //m_BikeS.pfRearWheelPos = NULL;
    //m_BikeS.pfRearWheelRot = NULL;
    //m_BikeS.pfPlayerLArmPos = NULL;
    //m_BikeS.pfPlayerUArmPos = NULL;
    //m_BikeS.pfPlayerLLegPos = NULL;
    //m_BikeS.pfPlayerULegPos = NULL;
    //m_BikeS.pfPlayerTorsoPos = NULL;
    //m_BikeS.pfPlayerTorsoRot = NULL;
    m_BikeS.PlayerLArmP = Vector2f(0,0);
    m_BikeS.PlayerLLegP = Vector2f(0,0);
    m_BikeS.PlayerTorsoP = Vector2f(0,0);
    m_BikeS.PlayerUArmP = Vector2f(0,0);
    m_BikeS.PlayerULegP = Vector2f(0,0);
    //m_BikeS.pfPlayerLArm2Pos = NULL;
    //m_BikeS.pfPlayerUArm2Pos = NULL;
    //m_BikeS.pfPlayerLLeg2Pos = NULL;
    //m_BikeS.pfPlayerULeg2Pos = NULL;
    //m_BikeS.pfPlayerTorso2Pos = NULL;
    //m_BikeS.pfPlayerTorso2Rot = NULL;
    m_BikeS.PlayerLArm2P = Vector2f(0,0);
    m_BikeS.PlayerLLeg2P = Vector2f(0,0);
    m_BikeS.PlayerTorso2P = Vector2f(0,0);
    m_BikeS.PlayerUArm2P = Vector2f(0,0);
    m_BikeS.PlayerULeg2P = Vector2f(0,0);
    m_BikeS.PrevFq = Vector2f(0,0);
    m_BikeS.PrevRq = Vector2f(0,0);
    m_BikeS.PrevPFq = Vector2f(0,0);
    m_BikeS.PrevPHq = Vector2f(0,0);
    m_BikeS.PrevPFq2 = Vector2f(0,0);
    m_BikeS.PrevPHq2 = Vector2f(0,0);
    m_BikeS.RearWheelP = Vector2f(0,0);
    m_BikeS.RFrontWheelP = Vector2f(0,0);
    m_BikeS.RRearWheelP = Vector2f(0,0);
    m_BikeS.Shoulder2P = Vector2f(0,0);
    m_BikeS.ShoulderP = Vector2f(0,0);
    m_BikeS.SwingAnchor2P = Vector2f(0,0);
    m_BikeS.SwingAnchorP = Vector2f(0,0);
    
    /* BIKE_P */
    memset(&m_BikeP,0,sizeof(m_BikeP));
    
    /* BIKE_C */
    memset(&m_BikeC,0,sizeof(m_BikeC)); 

    m_isGhostActive = false;           
  }

  /*===========================================================================
    Prepare the specified level for playing through this game object
    ===========================================================================*/
  void MotoGame::playLevel(LevelSrc *pLevelSrc, bool bIsAReplay) {
    bool v_enableScript = bIsAReplay == false;

    /* Clean up first, just for safe's sake */
    endLevel();               
    
    /* Set default gravity */
    m_PhysGravity.x = 0;
    m_PhysGravity.y = PHYS_WORLD_GRAV;
    
    /* Create Lua state */
    m_pL = lua_open();
    luaopen_math(m_pL);
    luaopen_Game(m_pL);    
    m_pMotoGame = this;
    
    /* Clear collision system */
    m_Collision.reset();
    
    /* Clear stuff */
    clearStates();
    
    m_bWheelSpin = false;
    
    m_fTime = 0.0f;
    m_fFinishTime = 0.0f;
    m_fLastAttitudeCon = -1000.0f;
    m_fAttitudeCon = 0.0f;
    
    m_nStillFrames = 0;
    m_nNumDummies = 0;
    
    m_nLastEventSeq = 0;
    
    m_Arrow.nArrowPointerMode = 0;

    m_bTeleport=false;
        
    m_PlayerFootAnchorBodyID = NULL;
    m_PlayerHandAnchorBodyID = NULL;
    m_PlayerTorsoBodyID = NULL;
    m_PlayerUArmBodyID = NULL;
    m_PlayerLArmBodyID = NULL;
    m_PlayerULegBodyID = NULL;
    m_PlayerLLegBodyID = NULL;
    m_PlayerFootAnchorBodyID2 = NULL;
    m_PlayerHandAnchorBodyID2 = NULL;
    m_PlayerTorsoBodyID2 = NULL;
    m_PlayerUArmBodyID2 = NULL;
    m_PlayerLArmBodyID2 = NULL;
    m_PlayerULegBodyID2 = NULL;
    m_PlayerLLegBodyID2 = NULL;
    
    m_bDead = m_bFinished = false;
    
    m_nGameEventQueueReadIdx = m_nGameEventQueueWriteIdx = 0;
    
    /* Clear zone flags */
    for(int i=0;i<pLevelSrc->getZoneList().size();i++)
      pLevelSrc->getZoneList()[i]->m_bInZone = false;
    
    /* Load and parse level script */
    bool bTryParsingEncapsulatedLevelScript = true;
    bool bNeedScript = false;
    bool bGotScript = false;
    
    if(pLevelSrc->getScriptFile() != "") {
      FileHandle *pfh = FS::openIFile(std::string("./Levels/") + pLevelSrc->getScriptFile());
      if(pfh == NULL) {
        /* Well, file not found -- try encapsulated script */
        bNeedScript = true;
      }
      else {      
        std::string Line,ScriptBuf="";
        
        while(FS::readNextLine(pfh,Line)) {
          if(Line.length() > 0) {
            ScriptBuf.append(Line.append("\n"));
          }
        }
        
        FS::closeFile(pfh);

        /* Use the Lua aux lib to load the buffer */
        int nRet = luaL_loadbuffer(m_pL,ScriptBuf.c_str(),ScriptBuf.length(),pLevelSrc->getScriptFile().c_str()) ||
	  lua_pcall(m_pL,0,0,0);    

        /* Returned WHAT? */
        if(nRet != 0) {
          lua_close(m_pL);
          throw Exception("failed to load level script");
        }

        bGotScript = true;
        bTryParsingEncapsulatedLevelScript = false;
      }       
    }    
    
    if(bTryParsingEncapsulatedLevelScript && pLevelSrc->getScriptSource() != "") {
      /* Use the Lua aux lib to load the buffer */
      int nRet = luaL_loadbuffer(m_pL,pLevelSrc->getScriptSource().c_str(),pLevelSrc->getScriptSource().length(),
				 pLevelSrc->getFileName().c_str()) || lua_pcall(m_pL,0,0,0);    
         
      /* Returned WHAT? */
      if(nRet != 0) {
	lua_close(m_pL);
	throw Exception("failed to load level encapsulated script");
      }

      bGotScript = true;      
    }    
    
    if(bNeedScript && !bGotScript) {
      lua_close(m_pL);
      throw Exception("failed to get level script");
    }
    
    /* Initialize physics */
    _InitPhysics();
    
    /* Set level source reference -- this tells the world that the level is ready */
    m_pLevelSrc = pLevelSrc;
    
    /* Generate extended level data to be used by the game */
    _GenerateLevel();
        
    /* Calculate bike stuff */
    _CalculateBikeAnchors();    
    Vector2f C( pLevelSrc->getPlayerStartX() - m_BikeA.Tp.x, pLevelSrc->getPlayerStartY() - m_BikeA.Tp.y);
    _PrepareBikePhysics(C);
    
    //const dReal *pf = dBodyGetPosition(m_FrontWheelBodyID);
    //m_PrevFrontWheelP = Vector2f(pf[0],pf[1]);
    //pf = dBodyGetPosition(m_RearWheelBodyID);
    //m_PrevRearWheelP = Vector2f(pf[0],pf[1]);
    
    /* Drive left-to-right for starters */
    m_BikeS.Dir = DD_RIGHT;
    
    m_BikeS.fCurBrake = m_BikeS.fCurEngine = 0.0f;
    
    /* Spawn initial entities */
    for(int i=0;i<m_pLevelSrc->getEntityList().size();i++) {
      LevelEntity *pLEnt = m_pLevelSrc->getEntityList()[i];
      
      EntityType Type = _TransEntityType(pLEnt->TypeID);
      if(Type != ET_UNASSIGNED) {
        _SpawnEntity(pLEnt->ID,Type,Vector2f(pLEnt->fPosX,pLEnt->fPosY),pLEnt);        
      }
      else
        Log("** Warning ** : Unknown entity type '%s' for '%s'",pLEnt->TypeID.c_str(),pLEnt->ID.c_str());
    }
    
    /* Invoke the OnLoad() script function */
    if(v_enableScript) {
      bool bOnLoadSuccess = scriptCallBool("OnLoad",
					   true);
      /* if no OnLoad(), assume success */
      /* Success? */
      if(!bOnLoadSuccess) {
	/* Hmm, the script insists that we shouldn't begin playing... */
	endLevel();      
      }
    }
  }

  /*===========================================================================
    Free this game object
    ===========================================================================*/
  void MotoGame::endLevel(void) {
    /* If not already freed */
    if(m_pLevelSrc != NULL) {
      /* Clean up */
      while(m_Entities.size() > 0) _KillEntity(m_Entities[0]);
      while(m_Blocks.size() > 0) {
        while(m_Blocks[0]->Vertices.size() > 0) {
          delete m_Blocks[0]->Vertices[0];
          m_Blocks[0]->Vertices.erase( m_Blocks[0]->Vertices.begin() );
        }
        delete m_Blocks[0];
        m_Blocks.erase( m_Blocks.begin() );
      }
      while(m_OvEdges.size() > 0) {
        delete m_OvEdges[0];
        m_OvEdges.erase( m_OvEdges.begin() );
      }
      lua_close(m_pL);
      m_pL = NULL;

      m_FSprites.clear();
      m_BSprites.clear();
      
      /* Stop physics */
      _UninitPhysics();
      
      /* Release reference to level source */
      m_pLevelSrc = NULL;      
    }
    
    /* Get rid of game messages */
    for(int i=0;i<m_GameMessages.size();i++)
      delete m_GameMessages[i];
    m_GameMessages.clear();
    
    /* Get rid of replay events */
    for(int i=0;i<m_ReplayEvents.size();i++)
      delete m_ReplayEvents[i];
    m_ReplayEvents.clear();
  }

  /*===========================================================================
    Level generation (i.e. parsing of level source)
    ===========================================================================*/
  void MotoGame::_GenerateLevel(void) {
    if(m_pLevelSrc == NULL) {
      Log("** Warning ** : Can't generate level when no source is assigned!");
      return;
    }
        
    /* Ok, our primary job is to convert the set of input blocks, which CAN
       contain concave polygons, into a final set of (possibly) smaller 
       blocks that are guaranteed to be convex. */
    std::vector<LevelBlock *> &InBlocks = m_pLevelSrc->getBlockList();           

    /* Start by determining the bounding box of the level */
    Vector2f LevelBoundsMin,LevelBoundsMax;
    LevelBoundsMin.x = m_pLevelSrc->getLeftLimit();
    LevelBoundsMax.x = m_pLevelSrc->getRightLimit();
    LevelBoundsMin.y = m_pLevelSrc->getBottomLimit();
    LevelBoundsMax.y = m_pLevelSrc->getTopLimit();
    for(int i=0;i<InBlocks.size();i++) { 
      for(int j=0;j<InBlocks[i]->Vertices.size();j++) {
        LevelBoundsMin.x = InBlocks[i]->fPosX+InBlocks[i]->Vertices[j]->fX < LevelBoundsMin.x ? 
	  InBlocks[i]->fPosX+InBlocks[i]->Vertices[j]->fX : LevelBoundsMin.x;
        LevelBoundsMin.y = InBlocks[i]->fPosY+InBlocks[i]->Vertices[j]->fY < LevelBoundsMin.y ? 
	  InBlocks[i]->fPosY+InBlocks[i]->Vertices[j]->fY : LevelBoundsMin.y;
        LevelBoundsMax.x = InBlocks[i]->fPosX+InBlocks[i]->Vertices[j]->fX > LevelBoundsMax.x ? 
	  InBlocks[i]->fPosX+InBlocks[i]->Vertices[j]->fX : LevelBoundsMax.x;
        LevelBoundsMax.y = InBlocks[i]->fPosY+InBlocks[i]->Vertices[j]->fY > LevelBoundsMax.y ? 
	  InBlocks[i]->fPosY+InBlocks[i]->Vertices[j]->fY : LevelBoundsMax.y;
      }
    }
    
    m_Collision.setDims(LevelBoundsMin.x,LevelBoundsMin.y,LevelBoundsMax.x,LevelBoundsMax.y);
    //m_Collision.setSize(LevelBoundsMax.x - LevelBoundsMin.x,LevelBoundsMax.y - LevelBoundsMin.y);
    //m_Collision.setCenter((LevelBoundsMax.x + LevelBoundsMin.x)/2.0f,(LevelBoundsMax.y + LevelBoundsMin.y)/2.0f);
    
    Log("Generating level from %d block%s...",InBlocks.size(),InBlocks.size()==1?"":"s");
    
    /* For each input block */
    int nTotalBSPErrors = 0;
    
    for(int i=0;i<InBlocks.size();i++) {
      /* Do the "convexifying" the BSP-way. It might be overkill, but we'll
         probably appreciate it when the input data is very complex. It'll also 
         let us handle crossing edges, and other kinds of weird input. */
      BSP BSPTree;
      
      /* Define edges */
      for(int j=0;j<InBlocks[i]->Vertices.size();j++) {
        /* Next vertex? */
        int jnext = j+1;
        if(jnext == InBlocks[i]->Vertices.size()) jnext=0;
        
        if(!InBlocks[i]->bBackground) {
          /* Add line to collision handler */
          m_Collision.defineLine(InBlocks[i]->fPosX + InBlocks[i]->Vertices[j]->fX,
				 InBlocks[i]->fPosY + InBlocks[i]->Vertices[j]->fY,
				 InBlocks[i]->fPosX + InBlocks[i]->Vertices[jnext]->fX,
				 InBlocks[i]->fPosY + InBlocks[i]->Vertices[jnext]->fY);
        }
        
        /* Add line to BSP generator */
        BSPTree.addLineDef( Vector2f(InBlocks[i]->Vertices[j]->fX,
                                     InBlocks[i]->Vertices[j]->fY),
                            Vector2f(InBlocks[i]->Vertices[jnext]->fX,
                                     InBlocks[i]->Vertices[jnext]->fY) );                                             
        /* Is this a special edge? */
        if(InBlocks[i]->Vertices[j]->EdgeEffect != "") {
          OverlayEdge *pEdge = new OverlayEdge;
          pEdge->Effect = _TransEdgeEffect(InBlocks[i]->Vertices[j]->EdgeEffect);
          pEdge->P1 = Vector2f(InBlocks[i]->Vertices[j]->fX,InBlocks[i]->Vertices[j]->fY);
          pEdge->P2 = Vector2f(InBlocks[i]->Vertices[jnext]->fX,InBlocks[i]->Vertices[jnext]->fY);
          pEdge->pSrcBlock = InBlocks[i];
          m_OvEdges.push_back( pEdge );
        }
      }
      
      /* Compute */
      std::vector<BSPPoly *> &BSPPolys = BSPTree.compute();      
                  
      /* Create blocks */
      for(int j=0;j<BSPPolys.size();j++) {
        _CreateBlock(BSPPolys[j],InBlocks[i]);
      }
      Log(" %d poly%s generated from block #%d",BSPPolys.size(),BSPPolys.size()==1?"":"s",i+1);

      /* Errors from BSP? */        
      nTotalBSPErrors += BSPTree.getNumErrors();      
    }
    
    Log(" %d special edge%s",m_OvEdges.size(),m_OvEdges.size()==1?"":"s");
    Log(" %d poly%s in total",m_Blocks.size(),m_Blocks.size()==1?"":"s");        
    
    if(nTotalBSPErrors > 0) {
      Log(" %d BSP error%s in total",nTotalBSPErrors,nTotalBSPErrors==1?"":"s");
      gameMessage(GAMETEXT_WARNING);
      gameMessage(GAMETEXT_ERRORSINLEVEL);
    }
    
    /* Create level surroundings (by limits) */    
    float fVMargin = 20,fHMargin = 20;
    ConvexBlock *pBlock; 
    ConvexBlockVertex *pVertex;
    
    /* TOP */
    m_Blocks.push_back( pBlock = new ConvexBlock );    
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getLeftLimit() - fHMargin, m_pLevelSrc->getTopLimit() + fVMargin);
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit() + fHMargin, m_pLevelSrc->getTopLimit() + fVMargin);
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit(), m_pLevelSrc->getTopLimit());
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getTopLimit());
    pBlock->pSrcBlock = NULL;

    /* BOTTOM */
    m_Blocks.push_back( pBlock = new ConvexBlock );    
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit(), m_pLevelSrc->getBottomLimit());
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit() + fHMargin, m_pLevelSrc->getBottomLimit() - fVMargin);
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getLeftLimit() - fHMargin, m_pLevelSrc->getBottomLimit() - fVMargin);
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getBottomLimit());
    pBlock->pSrcBlock = NULL;

    /* LEFT */
    m_Blocks.push_back( pBlock = new ConvexBlock );    
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getTopLimit());
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getBottomLimit());
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getLeftLimit() - fHMargin, m_pLevelSrc->getBottomLimit() - fVMargin);
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getLeftLimit() - fHMargin, m_pLevelSrc->getTopLimit() + fVMargin);
    pBlock->pSrcBlock = NULL;

    /* RIGHT */
    m_Blocks.push_back( pBlock = new ConvexBlock );    
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit(), m_pLevelSrc->getTopLimit());
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit() + fHMargin, m_pLevelSrc->getTopLimit() + fVMargin);
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit() + fHMargin, m_pLevelSrc->getBottomLimit() - fVMargin);
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit(), m_pLevelSrc->getBottomLimit());
    pBlock->pSrcBlock = NULL;
    
    /* Give limits to collision system */
    m_Collision.defineLine( m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getTopLimit(),
                            m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getBottomLimit() );
    m_Collision.defineLine( m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getBottomLimit(),
                            m_pLevelSrc->getRightLimit(), m_pLevelSrc->getBottomLimit() );
    m_Collision.defineLine( m_pLevelSrc->getRightLimit(), m_pLevelSrc->getBottomLimit(),
                            m_pLevelSrc->getRightLimit(), m_pLevelSrc->getTopLimit() );
    m_Collision.defineLine( m_pLevelSrc->getRightLimit(), m_pLevelSrc->getTopLimit(),
                            m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getTopLimit() );
    
    /* Show stats about the collision system */
    CollisionSystemStats CStats;
    m_Collision.getStats(&CStats);
    Log(" %dx%d grid with %.1fx%.1f cells (%.0f%% empty)",
        CStats.nGridWidth,CStats.nGridHeight,CStats.fCellWidth,CStats.fCellHeight,
        CStats.fPercentageOfEmptyCells);
    Log(" %d total blocking lines",CStats.nTotalLines);
  }
  
  /*===========================================================================
    Create block from BSP polygon
    ===========================================================================*/
  void MotoGame::_CreateBlock(BSPPoly *pPoly,LevelBlock *pSrcBlock) {
    ConvexBlock *pBlock = new ConvexBlock;
    pBlock->pSrcBlock = pSrcBlock;
    
    for(int i=0;i<pPoly->Vertices.size();i++) {
      ConvexBlockVertex *pVertex = new ConvexBlockVertex;
      
      pVertex->P = pPoly->Vertices[i]->P;
      
      pBlock->Vertices.push_back( pVertex );
    }
    
    m_Blocks.push_back( pBlock );
  }
  
  /*===========================================================================
    Calculate important bike anchor points from parameters 
    ===========================================================================*/
  void MotoGame::_CalculateBikeAnchors(void) {
    m_BikeA.Tp = Vector2f( 0, -m_BikeP.Ch );
    m_BikeA.Rp = m_BikeA.Tp + Vector2f( -0.5f*m_BikeP.Wb, m_BikeP.WR );
    m_BikeA.Fp = m_BikeA.Tp + Vector2f( 0.5f*m_BikeP.Wb, m_BikeP.WR );
    m_BikeA.AR = Vector2f(m_BikeP.RVx,m_BikeP.RVy);
    m_BikeA.AF = Vector2f(m_BikeP.FVx,m_BikeP.FVy);
    m_BikeA.AR2 = Vector2f(-m_BikeP.RVx,m_BikeP.RVy);
    m_BikeA.AF2 = Vector2f(-m_BikeP.FVx,m_BikeP.FVy);
    m_BikeA.PLAp = (Vector2f(m_BikeP.PEVx,m_BikeP.PEVy) + Vector2f(m_BikeP.PHVx,m_BikeP.PHVy))*0.5f;
    m_BikeA.PUAp = (Vector2f(m_BikeP.PEVx,m_BikeP.PEVy) + Vector2f(m_BikeP.PSVx,m_BikeP.PSVy))*0.5f;
    m_BikeA.PLLp = (Vector2f(m_BikeP.PFVx,m_BikeP.PFVy) + Vector2f(m_BikeP.PKVx,m_BikeP.PKVy))*0.5f;
    m_BikeA.PULp = (Vector2f(m_BikeP.PLVx,m_BikeP.PLVy) + Vector2f(m_BikeP.PKVx,m_BikeP.PKVy))*0.5f;
    m_BikeA.PTp = (Vector2f(m_BikeP.PLVx,m_BikeP.PLVy) + Vector2f(m_BikeP.PSVx,m_BikeP.PSVy))*0.5f;
    m_BikeA.PHp = Vector2f(m_BikeP.PHVx,m_BikeP.PHVy);
    m_BikeA.PFp = Vector2f(m_BikeP.PFVx,m_BikeP.PFVy);
    m_BikeA.PLAp2 = (Vector2f(-m_BikeP.PEVx,m_BikeP.PEVy) + Vector2f(-m_BikeP.PHVx,m_BikeP.PHVy))*0.5f;
    m_BikeA.PUAp2 = (Vector2f(-m_BikeP.PEVx,m_BikeP.PEVy) + Vector2f(-m_BikeP.PSVx,m_BikeP.PSVy))*0.5f;
    m_BikeA.PLLp2 = (Vector2f(-m_BikeP.PFVx,m_BikeP.PFVy) + Vector2f(-m_BikeP.PKVx,m_BikeP.PKVy))*0.5f;
    m_BikeA.PULp2 = (Vector2f(-m_BikeP.PLVx,m_BikeP.PLVy) + Vector2f(-m_BikeP.PKVx,m_BikeP.PKVy))*0.5f;
    m_BikeA.PTp2 = (Vector2f(-m_BikeP.PLVx,m_BikeP.PLVy) + Vector2f(-m_BikeP.PSVx,m_BikeP.PSVy))*0.5f;
    m_BikeA.PHp2 = Vector2f(-m_BikeP.PHVx,m_BikeP.PHVy);
    m_BikeA.PFp2 = Vector2f(-m_BikeP.PFVx,m_BikeP.PFVy);
  }

  /*===========================================================================
    Check whether the given circle touches the zone
    ===========================================================================*/
  bool MotoGame::_DoCircleTouchZone(const Vector2f &Cp,float Cr,LevelZone *pZone) {
    /* Check each zone primitive */
    for(int i=0;i<pZone->Prims.size();i++) {
      if(pZone->Prims[i]->Type == LZPT_BOX) {
        /* Do simple AABB-check */
        Vector2f CMin(Cp.x-Cr,Cp.y-Cr);
        Vector2f CMax(Cp.x+Cr,Cp.y+Cr);        
        Vector2f FMin(CMin.x < pZone->Prims[i]->fLeft ? CMin.x : pZone->Prims[i]->fLeft,
                      CMin.y < pZone->Prims[i]->fBottom ? CMin.y : pZone->Prims[i]->fBottom);
        Vector2f FMax(CMax.x > pZone->Prims[i]->fRight ? CMax.x : pZone->Prims[i]->fRight,
                      CMax.y > pZone->Prims[i]->fTop ? CMax.y : pZone->Prims[i]->fTop);
        if(FMax.x - FMin.x < (CMax.x - CMin.x + pZone->Prims[i]->fRight - pZone->Prims[i]->fLeft) &&
           FMax.y - FMin.y < (CMax.y - CMin.y + pZone->Prims[i]->fTop - pZone->Prims[i]->fBottom))
          return true; /* Touch! */
      }
    }
    
    /* No touching! */
    return false;
  }
  
  /*===========================================================================
    Update zone specific stuff -- call scripts where needed
    ===========================================================================*/
  void MotoGame::_UpdateZones(void) {
    /* Check player touching for each zone */
    for(int i=0;i<m_pLevelSrc->getZoneList().size();i++) {
      LevelZone *pZone = m_pLevelSrc->getZoneList()[i];
      
      /* Check it against the wheels and the head */
      if(_DoCircleTouchZone( m_BikeS.FrontWheelP,m_BikeP.WR,pZone ) ||
         _DoCircleTouchZone( m_BikeS.RearWheelP,m_BikeP.WR,pZone )) {       
        /* In the zone -- did he just enter it? */
        if(!pZone->m_bInZone) {
	  /* Generate event */
	  GameEvent *pEvent = createGameEvent(GAME_EVENT_PLAYER_ENTERS_ZONE);
	  if(pEvent != NULL) {
	    pZone->m_bInZone = true;
	    pEvent->u.PlayerEntersZone.pZone = pZone;
	  }
        }
      }         
      else {
        /* Not in the zone... but was he during last update? - i.e. has 
           he just left it? */      
        if(pZone->m_bInZone) {
	  /* Generate event */
	  GameEvent *pEvent = createGameEvent(GAME_EVENT_PLAYER_LEAVES_ZONE);
	  if(pEvent != NULL) {
	    pZone->m_bInZone = false;
	    pEvent->u.PlayerLeavesZone.pZone = pZone;
	  }
        }
      }
    }
  }
  
  /*===========================================================================
    Entity management
    ===========================================================================*/
  Entity *MotoGame::_SpawnEntity(std::string ID,EntityType Type,Vector2f Pos,LevelEntity *pSrc) {
    /* Allocate entity */
    Entity *pEnt = new Entity;
    pEnt->Type = Type;
    pEnt->pSrc = pSrc;
    pEnt->Pos = Pos;
    pEnt->ID = ID;
    pEnt->fSize = 0.5f;
    
    if(pSrc != NULL) pEnt->fSize = pSrc->fSize;
    
    /* Init it */
    switch(Type) {
    case ET_SPRITE:
      pEnt->fSpriteZ = 1.0f;              
      if(pSrc != NULL)
	pEnt->fSpriteZ = atof(m_pLevelSrc->getEntityParam(pSrc,"z","1.0").c_str());        
          
      pEnt->SpriteType = "";
      if(pSrc != NULL)
	pEnt->SpriteType = m_pLevelSrc->getEntityParam(pSrc,"name","");
                          
      /* Foreground/background? */
      if(pEnt->fSpriteZ > 0.0f)
	m_FSprites.push_back(pEnt); /* TODO: keep these lists ordered! */
      else
	m_BSprites.push_back(pEnt);
      break;
    case ET_WRECKER:
      break;
    case ET_ENDOFLEVEL:
      break;
    case ET_PLAYERSTART:
      break;
    case ET_DUMMY:
      break;
    case ET_PARTICLESOURCE:
      pEnt->ParticleType = "";
      if(pSrc != NULL)
	pEnt->ParticleType = m_pLevelSrc->getEntityParam(pSrc,"type","");
      pEnt->fNextParticleTime = 0;
      break;
    default:
      /* TODO: Warning */        
      ;
    }
    
    /* Add it */
    m_Entities.push_back(pEnt);
        
    /* Return it */
    return pEnt;
  }
  
  void MotoGame::_KillEntity(Entity *pEnt) { /* brutal */
    for(int i=0;i<m_Entities.size();i++) {
      if(m_Entities[i] == pEnt) {
        delete pEnt;
        m_Entities.erase(m_Entities.begin() + i);
        return;
      }
    }
    
    /* TODO: Warning (not found) */
  }

  void MotoGame::_UpdateEntities(void) {
    /* Do player touch anything? */
    for(int i=0;i<m_Entities.size();i++) {            
      /* Head? */						
      Vector2f HeadPos = m_BikeS.Dir==DD_RIGHT?m_BikeS.HeadP:m_BikeS.Head2P;
			
      if(circleTouchCircle2f(m_Entities[i]->Pos,m_Entities[i]->fSize,HeadPos,m_BikeP.fHeadSize)) {
	if(!m_Entities[i]->bTouched) {
	  /* Generate event */
	  GameEvent *pEvent = createGameEvent(GAME_EVENT_PLAYER_TOUCHES_ENTITY);
	  if(pEvent != NULL) {
	    pEvent->u.PlayerTouchesEntity.bHead = true;						
	    strncpy(pEvent->u.PlayerTouchesEntity.cEntityID,m_Entities[i]->ID.c_str(),sizeof(pEvent->u.PlayerTouchesEntity.cEntityID)-1);						
	    m_Entities[i]->bTouched = true;
	  }					
	}
      }
      /* Wheel then? */
      else if(circleTouchCircle2f(m_Entities[i]->Pos,m_Entities[i]->fSize,m_BikeS.FrontWheelP,m_BikeP.WR) ||
              circleTouchCircle2f(m_Entities[i]->Pos,m_Entities[i]->fSize,m_BikeS.RearWheelP,m_BikeP.WR)) {
	if(!m_Entities[i]->bTouched) {
	  /* Generate event */
	  GameEvent *pEvent = createGameEvent(GAME_EVENT_PLAYER_TOUCHES_ENTITY);
	  if(pEvent != NULL) {
	    pEvent->u.PlayerTouchesEntity.bHead = false;
	    strncpy(pEvent->u.PlayerTouchesEntity.cEntityID,m_Entities[i]->ID.c_str(),sizeof(pEvent->u.PlayerTouchesEntity.cEntityID)-1);
	    m_Entities[i]->bTouched = true;
	  }					
	}
      }      
      else {
	/* Not touching */
	m_Entities[i]->bTouched = false;
      }
    }
  }

  EntityType MotoGame::_TransEntityType(std::string Name) {
    if(Name == "Sprite") return ET_SPRITE;
    if(Name == "PlayerStart") return ET_PLAYERSTART;
    if(Name == "EndOfLevel") return ET_ENDOFLEVEL;
    if(Name == "Wrecker") return ET_WRECKER;
    if(Name == "Strawberry") return ET_STRAWBERRY;
    if(Name == "ParticleSource") return ET_PARTICLESOURCE;
    if(Name == "Dummy") return ET_DUMMY;
    
    return ET_UNASSIGNED;
  }
  
  EdgeEffect MotoGame::_TransEdgeEffect(std::string Name) {
    if(Name == "Grass") return EE_GRASS;
    
    return EE_UNASSIGNED;
  }

  /*===========================================================================
    Entity stuff (public)
    ===========================================================================*/
  void MotoGame::deleteEntity(Entity *pEntity) {
    /* Already scheduled for deletion? */
    for(int i=0;i<m_DelSchedule.size();i++)
      if(m_DelSchedule[i] == pEntity) return;
    m_DelSchedule.push_back(pEntity);
  }
  
  void MotoGame::touchEntity(Entity *pEntity,bool bHead, bool bEnableScript) {
    /* Start by invoking scripts if any */
    if(bEnableScript) {
      scriptCallTblVoid(pEntity->ID,"Touch");
    }    

    /* What kind of entity? */
    switch(pEntity->Type) {
    case ET_SPRITE:        
      break;
    case ET_PLAYERSTART:
      break;
    case ET_ENDOFLEVEL:
      {
	/* How many strawberries left? */
	if(countEntitiesByType(ET_STRAWBERRY) == 0) {
	  /* Level is done! */
	  m_bFinished = true;
	  m_fFinishTime = getTime();
	}
      }
      break;
    case ET_WRECKER: 
      {
	/* Hmm :( */
	GameEvent *pEvent = createGameEvent(GAME_EVENT_PLAYER_DIES);        
	if(pEvent != NULL) {
	  pEvent->u.PlayerDies.bWrecker = true;
	}
      }
      break;
    case ET_STRAWBERRY:
      {
	/* OH... nice */
	GameEvent *pEvent = createGameEvent(GAME_EVENT_ENTITY_DESTROYED);
	if(pEvent != NULL) {
	  strncpy(pEvent->u.EntityDestroyed.cEntityID,pEntity->ID.c_str(),sizeof(pEvent->u.EntityDestroyed.cEntityID)-1);
	  pEvent->u.EntityDestroyed.Type = pEntity->Type;
	  pEvent->u.EntityDestroyed.fSize = pEntity->fSize;
	  pEvent->u.EntityDestroyed.fPosX = pEntity->Pos.x;
	  pEvent->u.EntityDestroyed.fPosY = pEntity->Pos.y;
	}                
	/* Play yummy-yummy sound */
	Sound::playSampleByName("Sounds/PickUpStrawberry.ogg");
      }
      break;
    }
  }
  
  int MotoGame::countEntitiesByType(EntityType Type) {
    int n = 0;
  
    /* Count entities with this type */
    for(int i=0;i<m_Entities.size();i++)
      if(m_Entities[i]->Type == Type) n++;
      
    return n;
  }

  Entity *MotoGame::findEntity(const std::string &ID) {	
    for(int i=0;i<m_Entities.size();i++) {
      if(m_Entities[i]->ID == ID) return m_Entities[i];
    }
    return NULL;
  }

  /*===========================================================================
    Game event queue management
    ===========================================================================*/
  GameEvent *MotoGame::createGameEvent(GameEventType Type) {
    /* Space left in queue? */
    if(getNumPendingGameEvents() < GAME_EVENT_QUEUE_SIZE - 1) {
      /* Yup. */
      GameEvent *pEvent = &m_GameEventQueue[m_nGameEventQueueWriteIdx];			
      pEvent->Type = Type;
      pEvent->nSeq = m_nLastEventSeq++;
      m_nGameEventQueueWriteIdx++;			
      if(m_nGameEventQueueWriteIdx == GAME_EVENT_QUEUE_SIZE) {
	m_nGameEventQueueWriteIdx = 0;
      }
      return pEvent;
    }
		
    /* No */
    return NULL;
  }
  
  GameEvent *MotoGame::getNextGameEvent(void) {
    /* Anything in queue? */
    if(getNumPendingGameEvents() > 0) {
      /* Get next event and advance the read idx */
      GameEvent *pEvent = &m_GameEventQueue[m_nGameEventQueueReadIdx];
      m_nGameEventQueueReadIdx++;
      if(m_nGameEventQueueReadIdx == GAME_EVENT_QUEUE_SIZE) {
	m_nGameEventQueueReadIdx = 0;
      }
			
      return pEvent;
    }
		
    /* Nope, nothing */
    return NULL;
  }

  int MotoGame::getNumPendingGameEvents(void) {
    if(m_nGameEventQueueReadIdx < m_nGameEventQueueWriteIdx) {
      return m_nGameEventQueueWriteIdx - m_nGameEventQueueReadIdx;
    }
    else if(m_nGameEventQueueReadIdx > m_nGameEventQueueWriteIdx) {
      return GAME_EVENT_QUEUE_SIZE - m_nGameEventQueueReadIdx + m_nGameEventQueueWriteIdx;
    }
		
    return 0;
  }

  /*===========================================================================
    Update recorded replay events
    ===========================================================================*/
  void MotoGame::_UpdateReplayEvents(void) {
    /* Start looking for events that should be passed */
    for(int i=0;i<m_ReplayEvents.size();i++) {
      /* Not passed? And with a time stamp that tells it should have happened
         by now? */
      if(!m_ReplayEvents[i]->bPassed && m_ReplayEvents[i]->fTime < getTime()) {
        /* Nice. Handle this event, replay style */
        _HandleReplayEvent(&m_ReplayEvents[i]->Event);
        
        /* Pass it */
        m_ReplayEvents[i]->bPassed = true;
      }
    }

    /* Now see if we have moved back in time and whether we should apply some
       REVERSE events */
    for(int i=m_ReplayEvents.size()-1;i>=0;i--) {
      /* Passed? And with a time stamp larger than current time? */
      if(m_ReplayEvents[i]->bPassed && m_ReplayEvents[i]->fTime > getTime()) {
        /* Nice. Handle this event, replay style BACKWARDS */
        _HandleReverseReplayEvent(&m_ReplayEvents[i]->Event);

        /* Un-pass it */
        m_ReplayEvents[i]->bPassed = false;
      }
    }
  }

  void MotoGame::_HandleReplayEvent(GameEvent *pEvent) {
    /* Note how the number of events handled here are smaller than in the 
       primary event handling loop. That's because when playing replays, we
       only care about the most basic events - not events causing other
       events to be triggered */
    switch(pEvent->Type) {
    case GAME_EVENT_ENTITY_DESTROYED:
      {
	/* Destroy entity */
	Entity *pEntityToDestroy = findEntity(pEvent->u.EntityDestroyed.cEntityID);
	if(pEntityToDestroy != NULL) {
	  deleteEntity(pEntityToDestroy);
	}        
	else
	  Log("** Warning ** : Failed to destroy entity '%s' specified by replay!",
	      pEvent->u.EntityDestroyed.cEntityID);
      }
      break;

    case GAME_EVENT_LUA_CALL_SETENTITYPOS:
      {
	_SetEntityPos(pEvent->u.LuaCallSetentitypos.cEntityID,
		      pEvent->u.LuaCallSetentitypos.x,
		      pEvent->u.LuaCallSetentitypos.y);
      }
      break;

    case GAME_EVENT_LUA_CALL_CLEARMESSAGES:
      {
	clearGameMessages();
      }
      break;
      
    case GAME_EVENT_LUA_CALL_PLACEINGAMEARROW:
      {
	_PlaceInGameArrow(pEvent->u.LuaCallPlaceingamearrow.x,
			  pEvent->u.LuaCallPlaceingamearrow.y,
			  pEvent->u.LuaCallPlaceingamearrow.angle);
      }
      break;
      
    case GAME_EVENT_LUA_CALL_PLACESCREENARROW:
      {
	_PlaceScreenArrow(pEvent->u.LuaCallPlaceingamearrow.x,
			  pEvent->u.LuaCallPlaceingamearrow.y,
			  pEvent->u.LuaCallPlaceingamearrow.angle);
      }
      break;
      
    case GAME_EVENT_LUA_CALL_HIDEARROW:
      {
	getArrowPointer().nArrowPointerMode = 0;
      }
      break;
      
    case GAME_EVENT_LUA_CALL_MESSAGE:
      {
	gameMessage(pEvent->u.LuaCallMessage.cMessage); 
      }
      break;
      
    case GAME_EVENT_LUA_CALL_MOVEBLOCK:
      {
	_MoveBlock(pEvent->u.LuaCallMoveblock.cBlockID,
		   pEvent->u.LuaCallMoveblock.x,
		   pEvent->u.LuaCallMoveblock.y);
      }
      break;
      
    case GAME_EVENT_LUA_CALL_SETBLOCKPOS:
      {
	_SetBlockPos(pEvent->u.LuaCallSetblockpos.cBlockID,
		     pEvent->u.LuaCallSetblockpos.x,
		     pEvent->u.LuaCallSetblockpos.y);
      }
      break;
      
    case GAME_EVENT_LUA_CALL_SETGRAVITY:
      {
	setGravity(pEvent->u.LuaCallSetgravity.x,
		   pEvent->u.LuaCallSetgravity.y);
		   
      }
      break;
      
    case GAME_EVENT_LUA_CALL_SETPLAYERPOSITION:
      {
	m_pMotoGame->setPlayerPosition(pEvent->u.LuaCallSetplayerposition.x,
				       pEvent->u.LuaCallSetplayerposition.y,
				       pEvent->u.LuaCallSetplayerposition.bRight);
      }
      break;
      
    }
  }
  
  void MotoGame::_HandleReverseReplayEvent(GameEvent *pEvent) {
    /* Apply events with reverse results */
    switch(pEvent->Type) {
    case GAME_EVENT_ENTITY_DESTROYED:
      {
	/* Un-destroy entity (create it :P) */
	Entity *pEntityToDestroy = findEntity(pEvent->u.EntityDestroyed.cEntityID);
	if(pEntityToDestroy == NULL) {
	  Entity *pNew = _SpawnEntity(pEvent->u.EntityDestroyed.cEntityID,
				      pEvent->u.EntityDestroyed.Type,
				      Vector2f(pEvent->u.EntityDestroyed.fPosX,
					       pEvent->u.EntityDestroyed.fPosY),
				      NULL);
	  if(pNew != NULL) {
	    pNew->fSize = pEvent->u.EntityDestroyed.fSize;
	  }					                              
	}        
	else
	  Log("** Warning ** : Failed to create entity '%s' specified by replay - it's already there!",
	      pEvent->u.EntityDestroyed.cEntityID);
      }
      break;

    case GAME_EVENT_LUA_CALL_SETENTITYPOS:
    case GAME_EVENT_LUA_CALL_CLEARMESSAGES:
    case GAME_EVENT_LUA_CALL_PLACEINGAMEARROW:
    case GAME_EVENT_LUA_CALL_PLACESCREENARROW:
    case GAME_EVENT_LUA_CALL_HIDEARROW:
    case GAME_EVENT_LUA_CALL_MESSAGE:
    case GAME_EVENT_LUA_CALL_MOVEBLOCK:
    case GAME_EVENT_LUA_CALL_SETBLOCKPOS:
    case GAME_EVENT_LUA_CALL_SETGRAVITY:
    case GAME_EVENT_LUA_CALL_SETPLAYERPOSITION:
      {
	/* hum : do nothing */
	/* what can i do ?  */
      }
      break;

    }
  }  

  void MotoGame::_SetEntityPos(String pEntityID, float pX, float pY) {
    /* Find the specified entity and set its position */
    for(int i=0;i<getEntities().size();i++) {
      Entity *p = getEntities()[i];
      if(pEntityID == p->ID) {
	p->Pos.x = pX;
	p->Pos.y = pY;
      }
    }
    /* Entity not found */
  }

  void MotoGame::_PlaceInGameArrow(float pX, float pY, float pAngle) {
    m_pMotoGame->getArrowPointer().nArrowPointerMode = 1;
    m_pMotoGame->getArrowPointer().ArrowPointerPos = Vector2f(pX, pY);
    m_pMotoGame->getArrowPointer().fArrowPointerAngle = pAngle;
  }

  void MotoGame::_PlaceScreenArrow(float pX, float pY, float pAngle) {
    m_pMotoGame->getArrowPointer().nArrowPointerMode = 2;
    m_pMotoGame->getArrowPointer().ArrowPointerPos = Vector2f(pX, pY);
    m_pMotoGame->getArrowPointer().fArrowPointerAngle = pAngle;
  }

  void MotoGame::_MoveBlock(String pBlockID, float pX, float pY) {
    /* Find the specified block and move it along the given vector */
    for(int i=0;i<getBlocks().size();i++) {
      ConvexBlock *pBlock = getBlocks()[i];
      if(pBlock->pSrcBlock->ID == pBlockID) {
        pBlock->pSrcBlock->fPosX += pX;
        pBlock->pSrcBlock->fPosY += pY;
        break;
      }
    }
  }

  void MotoGame::_SetBlockPos(String pBlockID, float pX, float pY) {
    /* Find the specified block and set its position */
    for(int i=0;i<getBlocks().size();i++) {
      ConvexBlock *pBlock = getBlocks()[i];
      if(pBlock->pSrcBlock->ID == pBlockID) {
        pBlock->pSrcBlock->fPosX = pX;
        pBlock->pSrcBlock->fPosY = pY;
        break;
      }
    }
  }

#if defined(ALLOW_GHOST) 
  void MotoGame::UpdateGhostFromReplay(SerializedBikeState *pReplayState) {
    _UpdateStateFromReplay(pReplayState, &m_GhostBikeS);
  }
#endif

};
