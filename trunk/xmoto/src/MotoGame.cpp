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
  int L_Game_SetBlockCenter(lua_State *pL);
  int L_Game_SetBlockRotation(lua_State *pL);
  int L_Game_SetDynamicEntityRotation(lua_State *pL);
  int L_Game_SetDynamicEntityTranslation(lua_State *pL);
  int L_Game_SetDynamicEntityNone(lua_State *pL);
  int L_Game_SetDynamicBlockRotation(lua_State *pL);
  int L_Game_SetDynamicBlockTranslation(lua_State *pL);
  int L_Game_SetDynamicBlockNone(lua_State *pL);
  int L_Game_CameraZoom(lua_State *pL);
  int L_Game_CameraMove(lua_State *pL);
  int L_Game_GetEntityRadius(lua_State *pL);
  int L_Game_IsEntityTouched(lua_State *pL);
  int L_Game_KillPlayer(lua_State *pL);
  int L_Game_KillEntity(lua_State *pL);
  int L_Game_RemainingStrawberries(lua_State *pL);
  int L_Game_WinPlayer(lua_State *pL);

  /* "Game" Lua library */
  static const luaL_reg g_GameFuncs[] = {
    {"GetTime",        	  	    L_Game_GetTime},
    {"Message",        	  	    L_Game_Message},
    {"IsPlayerInZone", 	  	    L_Game_IsPlayerInZone},
    {"MoveBlock",      	  	    L_Game_MoveBlock},
    {"GetBlockPos",    	  	    L_Game_GetBlockPos},
    {"SetBlockPos",    	  	    L_Game_SetBlockPos},
    {"PlaceInGameArrow",  	    L_Game_PlaceInGameArrow},
    {"PlaceScreenArrow",  	    L_Game_PlaceScreenArrow},
    {"HideArrow",         	    L_Game_HideArrow},
    {"ClearMessages",     	    L_Game_ClearMessages},
    {"SetGravity", 	  	    L_Game_SetGravity},
    {"GetGravity", 	  	    L_Game_GetGravity},
    {"SetPlayerPosition", 	    L_Game_SetPlayerPosition},
    {"GetPlayerPosition", 	    L_Game_GetPlayerPosition},
    {"GetEntityPos", 	  	    L_Game_GetEntityPos},
    {"SetEntityPos", 	  	    L_Game_SetEntityPos},
    {"SetKeyHook",        	    L_Game_SetKeyHook},
    {"GetKeyByAction",    	    L_Game_GetKeyByAction},
    {"Log",               	    L_Game_Log},
    {"SetBlockCenter",    	    L_Game_SetBlockCenter},
    {"SetBlockRotation",  	    L_Game_SetBlockRotation},
    {"SetDynamicEntityRotation",    L_Game_SetDynamicEntityRotation},
    {"SetDynamicEntityTranslation", L_Game_SetDynamicEntityTranslation},
    {"SetDynamicEntityNone",        L_Game_SetDynamicEntityNone},
    {"SetDynamicBlockRotation",     L_Game_SetDynamicBlockRotation},
    {"SetDynamicBlockTranslation",  L_Game_SetDynamicBlockTranslation},
    {"SetDynamicBlockNone",         L_Game_SetDynamicBlockNone},
    {"CameraZoom", 		    L_Game_CameraZoom},
    {"CameraMove", 		    L_Game_CameraMove},
    {"GetEntityRadius", 	    L_Game_GetEntityRadius},
    {"IsEntityTouched", 	    L_Game_IsEntityTouched},
    {"KillPlayer",                  L_Game_KillPlayer},
    {"KillEntity",                  L_Game_KillEntity},
    {"RemainingStrawberries",       L_Game_RemainingStrawberries},
    {"WinPlayer",                   L_Game_WinPlayer},
    {NULL, NULL}
  };

  /*===========================================================================
    Init the lua game lib
    ===========================================================================*/
  LUALIB_API int luaopen_Game(lua_State *pL) {
    luaL_openlib(pL,"Game",g_GameFuncs,0);
    return 1;
  }

  MotoGame::MotoGame() {
    m_pLevelSrc=NULL;
    m_bSqueeking=false;
    clearStates();
    m_lastCallToEveryHundreath = 0.0;
#if defined(ALLOW_GHOST)
    m_showGhostTimeDiff = true;
    m_isGhostActive = false;
#endif
    m_renderer = NULL;
    m_isScriptActiv = false;
    
    bFrontWheelTouching = false;
    bRearWheelTouching  = false;

    m_bodyDetach = false;
  }
  
  MotoGame::~MotoGame() {
    endLevel();
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
  
  void MotoGame::scriptCallVoidNumberArg(std::string FuncName, int n) {
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
    
    resetAutoDisabler();
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
  void MotoGame::gameMessage(std::string Text,bool bOnce, float fDuration) {
    /* "unique"? */
    GameMessage *pMsg = NULL;
    
    if(bOnce) {
      /* Yeah, if there's another "unique", replace it */
      for(int i=0;i<m_GameMessages.size();i++) {
        if(m_GameMessages[i]->bOnce) {
          pMsg = m_GameMessages[i];
          break;
        }
      }
    }
    
    if(pMsg == NULL) {    
      pMsg = new GameMessage;
      m_GameMessages.push_back(pMsg);
    }
    
    pMsg->fRemoveTime = getTime() + fDuration;
    pMsg->bNew = true;
    pMsg->nAlpha = 255;
    pMsg->Text = Text;
    pMsg->bOnce = bOnce;
  }

  void MotoGame::clearGameMessages(void) {
    for(int i=0;i<m_GameMessages.size();i++)
      m_GameMessages[i]->fRemoveTime=0.0f;
  }

  /*===========================================================================
  Update dynamic collision lines
  ===========================================================================*/
  void MotoGame::_UpdateDynamicCollisionLines(void) {
	  std::vector<DynamicBlock *> &Blocks = getDynBlocks();
		for(int i=0;i<Blocks.size();i++) {
		  /* Ignore background blocks */
		  if(!Blocks[i]->bBackground) {
			  /* Build rotation matrix for block */
			  float fR[4]; 
			  fR[0] = cos(Blocks[i]->fRotation); fR[1] = -sin(Blocks[i]->fRotation);
			  fR[2] = sin(Blocks[i]->fRotation); fR[3] = cos(Blocks[i]->fRotation);
			  int z = 0;
  						
			  for(int j=0;j<Blocks[i]->ConvexBlocks.size();j++) {				
				  for(int k=0;k<Blocks[i]->ConvexBlocks[j]->Vertices.size();k++) {				    
				    int knext = k==Blocks[i]->ConvexBlocks[j]->Vertices.size()-1?0:k+1;
				    ConvexBlockVertex *pVertex1 = Blocks[i]->ConvexBlocks[j]->Vertices[k];				  
				    ConvexBlockVertex *pVertex2 = Blocks[i]->ConvexBlocks[j]->Vertices[knext];				  

				    /* Transform vertices */
				    Vector2f Tv1 = Vector2f(pVertex1->P.x * fR[0] + pVertex1->P.y * fR[1],
				                            pVertex1->P.x * fR[2] + pVertex1->P.y * fR[3]);
            Tv1 += Blocks[i]->Position;				                          
				    Vector2f Tv2 = Vector2f(pVertex2->P.x * fR[0] + pVertex2->P.y * fR[1],
				                            pVertex2->P.x * fR[2] + pVertex2->P.y * fR[3]);
            Tv2 += Blocks[i]->Position;				                          
              				  
				    /* Update line */
				    Blocks[i]->CollisionLines[z]->x1 = Tv1.x;
				    Blocks[i]->CollisionLines[z]->y1 = Tv1.y;
				    Blocks[i]->CollisionLines[z]->x2 = Tv2.x;
				    Blocks[i]->CollisionLines[z]->y2 = Tv2.y;
  				  
            /* Next collision line */
				    z++;
				  }
				}
			}
		}		  
  }

  /*===========================================================================
    Update game
    ===========================================================================*/
  void MotoGame::updateLevel(float fTimeStep,SerializedBikeState *pReplayState,Replay *p_replay) {
    m_bSqueeking = false; /* no squeeking right now */

    /* Dummies are small markers that can show different things during debugging */
    resetDummies();
    
    /* Going to teleport? Do it now, before we tinker to much with the state */
    if(m_bTeleport) {
      /* Clear stuff */
      clearStates();    
      
      m_fNextAttitudeCon = -1000.0f;
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
  
    updateGameMessages();
    
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
    
    /* Invoke PreDraw() script function - deprecated */
    if(m_isScriptActiv) {
      if(!scriptCallBool("PreDraw",
			 true)) {
	throw Exception("level script PreDraw() returned false");
      }
    }

    /* Invoke Tick() script function */
    /* and play script dynamic objects */
    while(getTime() - m_lastCallToEveryHundreath > 0.01) {
      if(m_isScriptActiv) {
	if(!scriptCallBool("Tick",
			   true)) {
			     throw Exception("level script Tick() returned false");
			   }
      }
      nextStateScriptDynamicObjects();
      m_lastCallToEveryHundreath += 0.01;
    }

    /* Only make a full physics update when not replaying */
    if(pReplayState == NULL) {
      /* Update physics */
      _UpdatePhysics(fTimeStep);
      if(isDead() == false) {
	executeEvents(p_replay);
      }
    }
    else {
      /* Well, handle replay events instead */
      _UpdateReplayEvents(p_replay);
    }

    /* Entities scheduled for termination? */
    for(int i=0;i<m_DelSchedule.size();i++)
      _KillEntity(m_DelSchedule[i]);
    m_DelSchedule.clear();
    
    // we don't change the sens of the wheel depending on the side, because 
    // loops must not make done just by changing the side
    if(pReplayState == NULL) { /* this does not work for replays */
      double fAngle = acos(m_BikeS.fFrameRot[0]);
      bool bCounterclock;
      if(m_BikeS.fFrameRot[2] < 0.0f) fAngle = 2*3.14159f - fAngle;

      if(m_somersaultCounter.update(fAngle, bCounterclock)) {
	scriptCallVoidNumberArg("OnSomersault", bCounterclock ? 1:0);
      }
    }

    /* Remember bike pos for next time */
    //m_PrevFrontWheelP = m_BikeS.FrontWheelP;
    //m_PrevRearWheelP = m_BikeS.RearWheelP;
    
    //float fAngle = (180.0f * acos(m_BikeS.fFrameRot[0])) / 3.14159f;
    //if(m_BikeS.fFrameRot[2] < 0.0f) fAngle = 360 - fAngle;
    //printf("%f\n",fAngle);
  }

  void MotoGame::executeEvents(Replay *p_replay) {
    /* Handle events generated this update */
    while(getNumPendingGameEvents() > 0) {
      MotoGameEvent *pEvent = getNextGameEvent();
      if(p_replay != NULL) {
	/* Encode event */
	_SerializeGameEventQueue(*p_replay, pEvent);
      }
      
      /* What event? */
      pEvent->doAction(this);
      destroyGameEvent(pEvent);
    }
  }

  void MotoGame::updateGameMessages() {
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
  }

  /*===========================================================================
    Prepare the specified level for playing through this game object
    ===========================================================================*/
  void MotoGame::prePlayLevel(
#if defined(ALLOW_GHOST)    
			   Replay *m_pGhostReplay,
#endif
			   LevelSrc *pLevelSrc,
			   Replay *recordingReplay,
			   bool bIsAReplay) {
    m_isScriptActiv = bIsAReplay == false;
    m_bLevelInitSuccess = true;

    /* Clean up first, just for safe's sake */
    endLevel();               
    
    /* Set default gravity */
    m_PhysGravity.x = 0;
    m_PhysGravity.y = PHYS_WORLD_GRAV;
    
    /* Create Lua state */
    m_pL = lua_open();
    luaopen_base(m_pL);   
    luaopen_math(m_pL);
    luaopen_table(m_pL);
    luaopen_Game(m_pL);    
    m_pMotoGame = this;
    
    /* Clear collision system */
    m_Collision.reset();
    bFrontWheelTouching = false;
    bRearWheelTouching  = false;    

    /* Clear stuff */
    clearStates();
    
    m_bWheelSpin = false;
    
    m_fTime = 0.0f;
    m_fFinishTime = 0.0f;
    m_fNextAttitudeCon = -1000.0f;
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
    setBodyDetach(false);

    m_lastCallToEveryHundreath = 0.0;

#if defined(ALLOW_GHOST)
    m_myLastStrawberries.clear();
    m_ghostLastStrawberries.clear();
    m_myDiffOfGhost = 0.0;

    if(m_pGhostReplay != NULL) {
      InitGhostLastStrawberries(m_pGhostReplay);
    }
#endif

    m_renderer->initCamera();
    m_renderer->initZoom();

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
	std::string error_msg = std::string(lua_tostring(m_pL,-1));
	lua_close(m_pL);
	/* should call error(0) */
	throw Exception("failed to load level encapsulated script :\n" + error_msg);
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
    try {
      _GenerateLevel();
    } catch(Exception &e) {
      m_bLevelInitSuccess = false;
    }        

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
    if(m_isScriptActiv) {
      bool bOnLoadSuccess = scriptCallBool("OnLoad",
					   true);
      /* if no OnLoad(), assume success */
      /* Success? */
      if(!bOnLoadSuccess) {
	      /* Hmm, the script insists that we shouldn't begin playing... */
	      endLevel();      
	      
	      m_bLevelInitSuccess = false;
      }
    }

    /* counter of somersaultcounter */
    m_somersaultCounter.init();

    /* execute events */
    executeEvents(recordingReplay);

    if(bIsAReplay == false) {
      /* Update game state */
      _UpdateGameState(NULL);
    }
  }

  void MotoGame::playLevel(
#if defined(ALLOW_GHOST)    
			   Replay *m_pGhostReplay,
#endif
			   LevelSrc *pLevelSrc,
			   bool bIsAReplay) {

  }

  /*===========================================================================
    Free this game object
    ===========================================================================*/
  void MotoGame::endLevel(void) {
    /* If not already freed */
    if(m_pLevelSrc != NULL) {
      /* Clean up */
      CleanEntities();
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
      
      /* Delete dynamic blocks */
      for(int i=0;i<m_DynBlocks.size();i++) {
        for(int j=0;j<m_DynBlocks[i]->CollisionLines.size();j++)
          delete m_DynBlocks[i]->CollisionLines[j];
          
        delete m_DynBlocks[i];
      }
      m_DynBlocks.clear();
      
      /* Stop physics */
      _UninitPhysics();
      
      /* Release reference to level source */
      m_pLevelSrc = NULL;      
    }
    
    /* Get rid of game messages */
    for(int i=0;i<m_GameMessages.size();i++)
      delete m_GameMessages[i];
    m_GameMessages.clear();

    /* clean Sdynamic objects for scripts */
    cleanScriptDynamicObjects();

    /* clean event queue */
    cleanEventsQueue();
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
    int nDynamicBlocks = 0;
    
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
        
        if(!InBlocks[i]->bBackground && !InBlocks[i]->bDynamic) {
          /* Add line to collision handler */
          m_Collision.defineLine(InBlocks[i]->fPosX + InBlocks[i]->Vertices[j]->fX,
				 InBlocks[i]->fPosY + InBlocks[i]->Vertices[j]->fY,
				 InBlocks[i]->fPosX + InBlocks[i]->Vertices[jnext]->fX,
				 InBlocks[i]->fPosY + InBlocks[i]->Vertices[jnext]->fY,
				 InBlocks[i]->fGrip);
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
      
      DynamicBlock *pDyn = NULL;
      if(InBlocks[i]->bDynamic) {
        /* Define dynamic block */
        pDyn = new DynamicBlock;
        pDyn->pSrcBlock = InBlocks[i];
        pDyn->bBackground = pDyn->pSrcBlock->bBackground;
        pDyn->Position = Vector2f(pDyn->pSrcBlock->fPosX,pDyn->pSrcBlock->fPosY);
        m_DynBlocks.push_back(pDyn);
        
        nDynamicBlocks++;
      }
                 
      /* Create blocks */
      for(int j=0;j<BSPPolys.size();j++) {
        ConvexBlock *pConvexBlock = _CreateBlock(BSPPolys[j],InBlocks[i]);
        
        /* Add to dynamic block? */
        if(pDyn != NULL) {
          /* Define collision lines */
          for(int k=0;k<BSPPolys[j]->Vertices.size();k++) {
            Line *pCLine = new Line;
            pCLine->x1 = pCLine->y1 = pCLine->x2 = pCLine->y2 = 0.0f;
	    pCLine->fGrip = pDyn->pSrcBlock->fGrip;
            pDyn->CollisionLines.push_back(pCLine);
            m_Collision.addExternalDynamicLine(pCLine);
          }
          
          /* Add to list */
          pDyn->ConvexBlocks.push_back(pConvexBlock);
        }
      }
      Log(" %d poly%s generated from block #%d (id=%s)",BSPPolys.size(),BSPPolys.size()==1?"":"s",i+1, InBlocks[i]->ID.c_str());
      
      /* Errors from BSP? */        
      nTotalBSPErrors += BSPTree.getNumErrors();      
    }
    
    Log(" %d special edge%s",m_OvEdges.size(),m_OvEdges.size()==1?"":"s");
    Log(" %d poly%s in total",m_Blocks.size(),m_Blocks.size()==1?"":"s");        
    Log(" %d dynamic block%s",nDynamicBlocks,nDynamicBlocks==1?"":"s");
    
    if(nTotalBSPErrors > 0) {
      Log(" %d BSP error%s in total",nTotalBSPErrors,nTotalBSPErrors==1?"":"s");
      gameMessage(std::string(GAMETEXT_WARNING) + ":");
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
    pVertex->T = pVertex->P * 0.25;
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit() + fHMargin, m_pLevelSrc->getTopLimit() + fVMargin);
    pVertex->T = pVertex->P * 0.25;
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit(), m_pLevelSrc->getTopLimit());
    pVertex->T = pVertex->P * 0.25;
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getTopLimit());
    pVertex->T = pVertex->P * 0.25;
    pBlock->pSrcBlock = NULL;

    /* BOTTOM */
    m_Blocks.push_back( pBlock = new ConvexBlock );    
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit(), m_pLevelSrc->getBottomLimit());
    pVertex->T = pVertex->P * 0.25;
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit() + fHMargin, m_pLevelSrc->getBottomLimit() - fVMargin);
    pVertex->T = pVertex->P * 0.25;
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getLeftLimit() - fHMargin, m_pLevelSrc->getBottomLimit() - fVMargin);
    pVertex->T = pVertex->P * 0.25;
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getBottomLimit());
    pVertex->T = pVertex->P * 0.25;
    pBlock->pSrcBlock = NULL;

    /* LEFT */
    m_Blocks.push_back( pBlock = new ConvexBlock );    
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getTopLimit());
    pVertex->T = pVertex->P * 0.25;
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getBottomLimit());
    pVertex->T = pVertex->P * 0.25;
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getLeftLimit() - fHMargin, m_pLevelSrc->getBottomLimit() - fVMargin);
    pVertex->T = pVertex->P * 0.25;
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getLeftLimit() - fHMargin, m_pLevelSrc->getTopLimit() + fVMargin);
    pVertex->T = pVertex->P * 0.25;
    pBlock->pSrcBlock = NULL;

    /* RIGHT */
    m_Blocks.push_back( pBlock = new ConvexBlock );    
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit(), m_pLevelSrc->getTopLimit());
    pVertex->T = pVertex->P * 0.25;
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit() + fHMargin, m_pLevelSrc->getTopLimit() + fVMargin);
    pVertex->T = pVertex->P * 0.25;
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit() + fHMargin, m_pLevelSrc->getBottomLimit() - fVMargin);
    pVertex->T = pVertex->P * 0.25;
    pBlock->Vertices.push_back( pVertex = new ConvexBlockVertex );
    pVertex->P=Vector2f( m_pLevelSrc->getRightLimit(), m_pLevelSrc->getBottomLimit());
    pVertex->T = pVertex->P * 0.25;
    pBlock->pSrcBlock = NULL;
    
    /* Give limits to collision system */
    m_Collision.defineLine( m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getTopLimit(),
                            m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getBottomLimit(),
			    DEFAULT_PHYS_WHEEL_GRIP);
    m_Collision.defineLine( m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getBottomLimit(),
                            m_pLevelSrc->getRightLimit(), m_pLevelSrc->getBottomLimit(),
			    DEFAULT_PHYS_WHEEL_GRIP );
    m_Collision.defineLine( m_pLevelSrc->getRightLimit(), m_pLevelSrc->getBottomLimit(),
                            m_pLevelSrc->getRightLimit(), m_pLevelSrc->getTopLimit(),
			    DEFAULT_PHYS_WHEEL_GRIP );
    m_Collision.defineLine( m_pLevelSrc->getRightLimit(), m_pLevelSrc->getTopLimit(),
                            m_pLevelSrc->getLeftLimit(), m_pLevelSrc->getTopLimit(),
			    DEFAULT_PHYS_WHEEL_GRIP );
    
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
  ConvexBlock *MotoGame::_CreateBlock(BSPPoly *pPoly,LevelBlock *pSrcBlock) {
    ConvexBlock *pBlock = new ConvexBlock;
    pBlock->pSrcBlock = pSrcBlock;
    
    for(int i=0;i<pPoly->Vertices.size();i++) {
      ConvexBlockVertex *pVertex = new ConvexBlockVertex;
      
      pVertex->P = pPoly->Vertices[i]->P;
      pVertex->T.x = (pSrcBlock->fPosX+pVertex->P.x) * 0.25;
      pVertex->T.y = (pSrcBlock->fPosY+pVertex->P.y) * 0.25;
      
      pBlock->Vertices.push_back( pVertex );
    }
    
    m_Blocks.push_back( pBlock );
    return pBlock;
  }
  
  /*===========================================================================
  Find a dynamic block
  ===========================================================================*/
  DynamicBlock *MotoGame::GetDynamicBlockByID(const std::string &ID) {
    for(int i=0;i<m_DynBlocks.size();i++) {
      if(m_DynBlocks[i]->pSrcBlock->ID == ID)
        return m_DynBlocks[i];
    }
    return NULL;
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
        if(pZone->m_bInZone == false) {
	  createGameEvent(new MGE_PlayerEntersZone(getTime(), pZone));
	  pZone->m_bInZone = true;
        }
      }         
      else {
        /* Not in the zone... but was he during last update? - i.e. has 
           he just left it? */      
        if(pZone->m_bInZone) {
	  createGameEvent(new MGE_PlayerLeavesZone(getTime(), pZone));
	  pZone->m_bInZone = false;
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
      if(m_Entities[i]->ID == pEnt->ID) {
	m_DestroyedEntities.push_back(m_Entities[i]);
        m_Entities.erase(m_Entities.begin() + i);
        return;
      }
    }
    
    /* TODO: Warning (not found) */
  }
  
  void MotoGame::CleanEntities() {
    for(unsigned int i=0;i<m_Entities.size();i++) {
      delete m_Entities[i];
    }    
    m_Entities.clear();

    for(unsigned int i=0;i<m_DestroyedEntities.size();i++) {
      delete m_DestroyedEntities[i];
    }    
    m_DestroyedEntities.clear();
  }

  Entity *MotoGame::getEntityByID(const std::string &ID) {
    for(int i=0;i<m_Entities.size();i++) {
      if(m_Entities[i]->ID == ID) {
        return m_Entities[i];
      }
    }
    return NULL;    
  }  

  bool MotoGame::touchEntityBodyExceptHead(const BikeState &pBike, const Entity &p_entity) {
    Vector2f res1, res2;

    if(pBike.Dir == DD_RIGHT) {
      if(intersectLineCircle2f(p_entity.Pos, p_entity.fSize,
			       pBike.FootP, pBike.KneeP,
			       res1, res2) > 0) {
				 return true;
			       }

      if(intersectLineCircle2f(p_entity.Pos, p_entity.fSize,
			       pBike.KneeP, pBike.LowerBodyP,
			       res1, res2) > 0) {
				 return true;
			       }
      
      if(intersectLineCircle2f(p_entity.Pos, p_entity.fSize,
			       pBike.LowerBodyP, pBike.ShoulderP,
			       res1, res2) > 0) {
				 return true;
			       }

      if(intersectLineCircle2f(p_entity.Pos, p_entity.fSize,
			       pBike.ShoulderP, pBike.ElbowP,
			       res1, res2) > 0) {
				 return true;
			       }

      if(intersectLineCircle2f(p_entity.Pos, p_entity.fSize,
			       pBike.ElbowP, pBike.HandP,
			       res1, res2) > 0) {
				 return true;
			       }
      return false;
    }

    if(pBike.Dir == DD_LEFT) {
      if(intersectLineCircle2f(p_entity.Pos, p_entity.fSize,
			       pBike.Foot2P, pBike.Knee2P,
			       res1, res2) > 0) {
				 return true;
			       }

      if(intersectLineCircle2f(p_entity.Pos, p_entity.fSize,
			       pBike.Knee2P, pBike.LowerBody2P,
			       res1, res2) > 0) {
				 return true;
			       }
      
      if(intersectLineCircle2f(p_entity.Pos, p_entity.fSize,
			       pBike.LowerBody2P, pBike.Shoulder2P,
			       res1, res2) > 0) {
				 return true;
			       }

      if(intersectLineCircle2f(p_entity.Pos, p_entity.fSize,
			       pBike.Shoulder2P, pBike.Elbow2P,
			       res1, res2) > 0) {
				 return true;
			       }

      if(intersectLineCircle2f(p_entity.Pos, p_entity.fSize,
			       pBike.Elbow2P, pBike.Hand2P,
			       res1, res2) > 0) {
				 return true;
			       }
      return false;
    }

    return false;
  }

  void MotoGame::_UpdateEntities(void) {
    Vector2f HeadPos = m_BikeS.Dir==DD_RIGHT?m_BikeS.HeadP:m_BikeS.Head2P;

    /* Do player touch anything? */
    for(int i=0;i<m_Entities.size();i++) {            
      /* Head? */
      if(circleTouchCircle2f(m_Entities[i]->Pos,
			     m_Entities[i]->fSize,
			     HeadPos,
			     m_BikeP.fHeadSize)) {
	if(m_Entities[i]->bTouched == false) {
	  createGameEvent(new MGE_PlayerTouchesEntity(getTime(),
						      m_Entities[i]->ID,
						      true));
	  m_Entities[i]->bTouched = true;
	}

	/* Wheel then ? */
      } else if(circleTouchCircle2f(m_Entities[i]->Pos,
				    m_Entities[i]->fSize,
				    m_BikeS.FrontWheelP,
				    m_BikeP.WR) ||
		circleTouchCircle2f(m_Entities[i]->Pos,
				    m_Entities[i]->fSize,
				    m_BikeS.RearWheelP,
				    m_BikeP.WR)) {
	if(m_Entities[i]->bTouched == false) {
	  createGameEvent(new MGE_PlayerTouchesEntity(getTime(),
						      m_Entities[i]->ID,
						      false));
	  m_Entities[i]->bTouched = true;
	}

	/* body then ?*/
      } else if(touchEntityBodyExceptHead(m_BikeS, *(m_Entities[i]))) {
	createGameEvent(new MGE_PlayerTouchesEntity(getTime(),
						    m_Entities[i]->ID,
						    false));
	m_Entities[i]->bTouched = true;
      } else {
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
    if(Name == "GrassAlt") return EE_GRASSALT;
    if(Name == "RedBricks") return EE_REDBRICKS;
    if(Name == "BlueBricks") return EE_BLUEBRICKS;
    if(Name == "GrayBricks") return EE_GRAYBRICKS;
    
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
  
  void MotoGame::touchEntity(Entity *pEntity,bool bHead) {
    /* Start by invoking scripts if any */
    if(m_isScriptActiv) {
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
	if(getNbRemainingStrawberries() == 0) {
	  makePlayerWin();
	}
      }
      break;
    case ET_WRECKER: 
      {
	createGameEvent(new MGE_PlayerDies(getTime(), true));
      }
      break;
    case ET_STRAWBERRY:
      {
	/* OH... nice */
	createGameEvent(new MGE_EntityDestroyed(getTime(),
						pEntity->ID,
						pEntity->Type,
						pEntity->fSize,
						pEntity->Pos.x,
						pEntity->Pos.y));
#if defined(ALLOW_GHOST)
	if(isGhostActive() && m_showGhostTimeDiff) {
	  m_myLastStrawberries.push_back(getTime());
	  UpdateDiffFromGhost();
	  DisplayDiffFromGhost();
	}
#endif
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
  #define ETRAN(_Name) case _Name: return #_Name
  static const char *_EventName(GameEventType Type) {
    switch(Type) {
      ETRAN(GAME_EVENT_PLAYER_DIES);
      ETRAN(GAME_EVENT_PLAYER_ENTERS_ZONE);
      ETRAN(GAME_EVENT_PLAYER_LEAVES_ZONE);
      ETRAN(GAME_EVENT_PLAYER_TOUCHES_ENTITY);
      ETRAN(GAME_EVENT_ENTITY_DESTROYED);
      ETRAN(GAME_EVENT_LUA_CALL_CLEARMESSAGES);
      ETRAN(GAME_EVENT_LUA_CALL_PLACEINGAMEARROW);
      ETRAN(GAME_EVENT_LUA_CALL_PLACESCREENARROW);
      ETRAN(GAME_EVENT_LUA_CALL_HIDEARROW);
      ETRAN(GAME_EVENT_LUA_CALL_MESSAGE);
      ETRAN(GAME_EVENT_LUA_CALL_MOVEBLOCK);
      ETRAN(GAME_EVENT_LUA_CALL_SETBLOCKPOS);
      ETRAN(GAME_EVENT_LUA_CALL_SETGRAVITY);
      ETRAN(GAME_EVENT_LUA_CALL_SETPLAYERPOSITION);
      ETRAN(GAME_EVENT_LUA_CALL_SETENTITYPOS);
      ETRAN(GAME_EVENT_LUA_CALL_SETBLOCKCENTER);
      ETRAN(GAME_EVENT_LUA_CALL_SETBLOCKROTATION);
      ETRAN(GAME_EVENT_LUA_CALL_SETDYNAMICENTITYROTATION);
      ETRAN(GAME_EVENT_LUA_CALL_SETDYNAMICENTITYTRANSLATION);
      ETRAN(GAME_EVENT_LUA_CALL_SETDYNAMICENTITYNONE);
      ETRAN(GAME_EVENT_LUA_CALL_SETDYNAMICBLOCKROTATION);
      ETRAN(GAME_EVENT_LUA_CALL_SETDYNAMICBLOCKTRANSLATION);
      ETRAN(GAME_EVENT_LUA_CALL_SETDYNAMICBLOCKNONE);
      ETRAN(GAME_EVENT_LUA_CALL_CAMERAZOOM);
      ETRAN(GAME_EVENT_LUA_CALL_CAMERAMOVE);      
    }
    return "??";
  }
    
  void MotoGame::createGameEvent(MotoGameEvent *p_event) {
    m_GameEventQueue.push(p_event);
  }
  
  void MotoGame::destroyGameEvent(MotoGameEvent *p_event) {
    delete p_event;
  }

  MotoGameEvent* MotoGame::getNextGameEvent() {
    /* Anything in queue? */
    if(getNumPendingGameEvents() > 0) {
      MotoGameEvent *v_event = m_GameEventQueue.front();
      m_GameEventQueue.pop();
      return v_event;
    }
		
    /* Nope, nothing */
    return NULL;
  }

  int MotoGame::getNumPendingGameEvents(void) {
    return m_GameEventQueue.size();
  }

  void MotoGame::cleanEventsQueue() {
    MotoGameEvent *v_event;

    while(m_GameEventQueue.empty() == false) {
      v_event = m_GameEventQueue.front();
      m_GameEventQueue.pop();
      destroyGameEvent(v_event);
    }
  }

  /*===========================================================================
    Update recorded replay events
    ===========================================================================*/
  void MotoGame::_UpdateReplayEvents(Replay *p_replay) {
    std::vector<RecordedGameEvent *> *v_replayEvents;

    v_replayEvents = p_replay->getEvents();

    /* Start looking for events that should be passed */
    for(int i=0;i<v_replayEvents->size();i++) {
      /* Not passed? And with a time stamp that tells it should have happened
         by now? */
      if(!(*v_replayEvents)[i]->bPassed && (*v_replayEvents)[i]->Event->getEventTime() < getTime()) {
        /* Nice. Handle this event, replay style */
        _HandleReplayEvent((*v_replayEvents)[i]->Event);
        
        /* Pass it */
        (*v_replayEvents)[i]->bPassed = true;
      }
    }

    /* Now see if we have moved back in time and whether we should apply some
       REVERSE events */
    for(int i=v_replayEvents->size()-1;i>=0;i--) {
      /* Passed? And with a time stamp larger than current time? */
      if((*v_replayEvents)[i]->bPassed && (*v_replayEvents)[i]->Event->getEventTime() > getTime()) {
        /* Nice. Handle this event, replay style BACKWARDS */
	(*v_replayEvents)[i]->Event->revert(this);

        /* Un-pass it */
        (*v_replayEvents)[i]->bPassed = false;
      }
    }
  }

  void MotoGame::_HandleReplayEvent(MotoGameEvent *pEvent) {     
    switch(pEvent->getType()) {
      case GAME_EVENT_PLAYER_TOUCHES_ENTITY:
      /* touching an entity creates events, so, don't call it */
      case GAME_EVENT_LUA_CALL_SETPLAYERPOSITION:
      /* don't set player position while it's already made by the game */
      /* however, this state is recorded : you can imagine than an effect
	 or something is done when this append to alert the player he took
	 a teleporter
      */
      break;
      default:
      pEvent->doAction(this);
      break;
    }
  }
  
  void MotoGame::revertEntityDestroyed(std::string p_entityID) {
    for(int i=0;i<m_DestroyedEntities.size();i++) {
      if(m_DestroyedEntities[i]->ID == p_entityID) {
	m_Entities.push_back(m_DestroyedEntities[i]);
        m_DestroyedEntities.erase(m_DestroyedEntities.begin() + i);
        return;
      }
    }
  }

  void MotoGame::SetEntityPos(String pEntityID, float pX, float pY) {
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

  void MotoGame::PlaceInGameArrow(float pX, float pY, float pAngle) {
    m_pMotoGame->getArrowPointer().nArrowPointerMode = 1;
    m_pMotoGame->getArrowPointer().ArrowPointerPos = Vector2f(pX, pY);
    m_pMotoGame->getArrowPointer().fArrowPointerAngle = pAngle;
  }

  void MotoGame::PlaceScreenArrow(float pX, float pY, float pAngle) {
    m_pMotoGame->getArrowPointer().nArrowPointerMode = 2;
    m_pMotoGame->getArrowPointer().ArrowPointerPos = Vector2f(pX, pY);
    m_pMotoGame->getArrowPointer().fArrowPointerAngle = pAngle;
  }

  void MotoGame::HideArrow() {
    getArrowPointer().nArrowPointerMode = 0;
  }

  void MotoGame::MoveBlock(String pBlockID, float pX, float pY) {
    /* Find the specified block and move it along the given vector */
    DynamicBlock *pBlock = GetDynamicBlockByID(pBlockID);
    if(pBlock != NULL) {
      pBlock->Position.x += pX;
      pBlock->Position.y += pY;
    }    
  }

  void MotoGame::SetBlockPos(String pBlockID, float pX, float pY) {
    /* Find the specified (dynamic) block and set its position */    
    DynamicBlock *pBlock = GetDynamicBlockByID(pBlockID);
    if(pBlock != NULL) {
      pBlock->Position.x = pX;
      pBlock->Position.y = pY;
    }    
  }
  
  void MotoGame::SetBlockCenter(String pBlockID, float pX, float pY) {
    /* Find the specified (dynamic) block and set its center */
    DynamicBlock *pBlock = GetDynamicBlockByID(pBlockID);
    if(pBlock != NULL) {
      /* Correct all polygons in block to this new center */
      for(int i=0;i<pBlock->ConvexBlocks.size();i++) {
        for(int j=0;j<pBlock->ConvexBlocks[i]->Vertices.size();j++) {
          ConvexBlockVertex *pVertex = pBlock->ConvexBlocks[i]->Vertices[j];
          pVertex->P.x = pVertex->P.x - pX;
          pVertex->P.y = pVertex->P.y - pY;
        }
      }
            
      pBlock->Position.x += pX;
      pBlock->Position.y += pY;
    }
  }

  void MotoGame::SetBlockRotation(String pBlockID, float pAngle) {
    /* Find the specified (dynamic) block and set its rotation */
    DynamicBlock *pBlock = GetDynamicBlockByID(pBlockID);
    if(pBlock != NULL) {
      pBlock->fRotation = pAngle;
    }    
  }     

#if defined(ALLOW_GHOST) 
  void MotoGame::UpdateGhostFromReplay(SerializedBikeState *pReplayState) {
    _UpdateStateFromReplay(pReplayState, &m_GhostBikeS);
  }

  void MotoGame::UpdateDiffFromGhost() {
    int v_myDiffOfGhostLastStrawberry;
    
    /* no strawberry, no update */
    if(m_myLastStrawberries.size() == 0) {
      return;
    }
    
    v_myDiffOfGhostLastStrawberry = m_myLastStrawberries.size();
    
    /* the ghost did not get this number of strawberries */
    if(m_ghostLastStrawberries.size() < v_myDiffOfGhostLastStrawberry) {
      return;
    }
    
    m_myDiffOfGhost = m_myLastStrawberries   [v_myDiffOfGhostLastStrawberry-1]
                    - m_ghostLastStrawberries[v_myDiffOfGhostLastStrawberry-1];
  }
  
  void MotoGame::DisplayDiffFromGhost() {
    char msg[256];
    sprintf(msg, "%+.2f", m_myDiffOfGhost);
    this->gameMessage(msg,true);
  }
  
  void MotoGame::InitGhostLastStrawberries(Replay *p_ghostReplay) {
    std::vector<RecordedGameEvent *> *v_replayEvents;
    v_replayEvents = p_ghostReplay->getEvents();
    
    /* Start looking for events */
    for(int i=0;i<v_replayEvents->size();i++) {
      MotoGameEvent *v_event = (*v_replayEvents)[i]->Event;

      if(v_event->getType() == GAME_EVENT_ENTITY_DESTROYED) {
	EntityType v_entityType = ((MGE_EntityDestroyed*)v_event)->getEntityType();
	
	if(v_entityType == ET_STRAWBERRY) {
	  /* new Strawberry for ghost */
	  m_ghostLastStrawberries.push_back((*v_replayEvents)[i]->Event->getEventTime());
	}
      }
    }
  }
#endif

  void MotoGame::cleanScriptDynamicObjects() {
    for(int i=0; i<m_SDynamicObjects.size(); i++) {
      delete m_SDynamicObjects[i];
    }
    m_SDynamicObjects.clear();
  }

  void MotoGame::nextStateScriptDynamicObjects() {
    int i = 0;

    while(i<m_SDynamicObjects.size()) {
      if(m_SDynamicObjects[i]->nextState(this) == false) {
	delete m_SDynamicObjects[i];
        m_SDynamicObjects.erase(m_SDynamicObjects.begin() + i);
      } else {
	i++;
      }
    }
  }

  void MotoGame::removeSDynamicOfObject(std::string pObject) {
    int i = 0;

    while(i<m_SDynamicObjects.size()) {
      if(m_SDynamicObjects[i]->getObjectId() == pObject) {
	delete m_SDynamicObjects[i];
        m_SDynamicObjects.erase(m_SDynamicObjects.begin() + i);
      } else {
	i++;
      }
    }
  }

  void MotoGame::setRenderer(GameRenderer *p_renderer) {
    m_renderer = p_renderer;
  }

  void MotoGame::CameraZoom(float pZoom) {
    if(m_renderer != NULL) {
      m_renderer->zoom(pZoom);
    }
  }
   
  void MotoGame::CameraMove(float p_x, float p_y) {
    if(m_renderer != NULL) {
      m_renderer->moveCamera(p_x, p_y);
    }
  }

  void MotoGame::killPlayer() {
    m_bDead = true;
    setBodyDetach(true);
    stopEngine();
  }

  void MotoGame::playerEntersZone(LevelZone *pZone) {
    if(m_isScriptActiv) {
      scriptCallTblVoid(pZone->ID, "OnEnter");
    }
  }
  
  void MotoGame::playerLeavesZone(LevelZone *pZone) {
    if(m_isScriptActiv) {
      scriptCallTblVoid(pZone->ID, "OnLeave");
    }
  }

  void MotoGame::playerTouchesEntity(std::string p_entityID, bool p_bTouchedWithHead) {
    Entity *pEntityToTouch = findEntity(p_entityID);
    if(pEntityToTouch != NULL) {
      touchEntity(pEntityToTouch, p_bTouchedWithHead);
    }
  }

  void MotoGame::entityDestroyed(std::string p_entityID,
				 EntityType p_type,
				 float p_fSize,
				 float p_fPosX, float p_fPosY) {
     if(p_type == ET_STRAWBERRY) {
       /* Spawn some particles */
       for(int i=0;i<6;i++) {
	        m_renderer->spawnParticle(PT_STAR,Vector2f(p_fPosX, p_fPosY), Vector2f(0,0), 5);
       }	

       /* Play yummy-yummy sound */
       Sound::playSampleByName("Sounds/PickUpStrawberry.ogg");
     }

     /* Destroy entity */
     Entity *pEntityToDestroy = findEntity(p_entityID);
     if(pEntityToDestroy != NULL) {
       deleteEntity(pEntityToDestroy);
     } else {
       Log("** Warning ** : Failed to destroy entity '%s' !",
	   p_entityID);
     }
  }

  void MotoGame::addDynamicObject(SDynamicObject *p_obj) {
    m_SDynamicObjects.push_back(p_obj);
  }

  void MotoGame::createKillEntityEvent(std::string p_entityID) {
    Entity *v_entity;

    v_entity = getEntityByID(p_entityID);
    if(v_entity != NULL) {
      createGameEvent(new MGE_EntityDestroyed(m_pMotoGame->getTime(),
					      v_entity->ID,
					      v_entity->Type,
					      v_entity->fSize,
					      v_entity->Pos.x,
					      v_entity->Pos.y));
    } else {
      Log("** Warning ** : Can't destroy an entity !");
    }
  }

  unsigned int MotoGame::getNbRemainingStrawberries() {
    return countEntitiesByType(ET_STRAWBERRY);
  }

  void MotoGame::makePlayerWin() {
    m_bFinished = true;
    m_fFinishTime = getTime();
  }

  void MotoGame::setBodyDetach(bool state) {
    m_bodyDetach = state;
  }

  void MotoGame::stopEngine() {
    m_BikeC.fDrive = 0.0f;
  }
}
