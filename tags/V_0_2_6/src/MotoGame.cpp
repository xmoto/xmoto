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
#include "xmscene/Entity.h"

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
  int L_Game_PenaltyTime(lua_State *pL);

  /* "Game" Lua library */
  static const luaL_reg g_GameFuncs[] = {
    {"GetTime",                 L_Game_GetTime},
    {"Message",                 L_Game_Message},
    {"IsPlayerInZone",          L_Game_IsPlayerInZone},
    {"MoveBlock",               L_Game_MoveBlock},
    {"GetBlockPos",             L_Game_GetBlockPos},
    {"SetBlockPos",             L_Game_SetBlockPos},
    {"PlaceInGameArrow",        L_Game_PlaceInGameArrow},
    {"PlaceScreenArrow",        L_Game_PlaceScreenArrow},
    {"HideArrow",               L_Game_HideArrow},
    {"ClearMessages",           L_Game_ClearMessages},
    {"SetGravity",          L_Game_SetGravity},
    {"GetGravity",          L_Game_GetGravity},
    {"SetPlayerPosition",       L_Game_SetPlayerPosition},
    {"GetPlayerPosition",       L_Game_GetPlayerPosition},
    {"GetEntityPos",          L_Game_GetEntityPos},
    {"SetEntityPos",          L_Game_SetEntityPos},
    {"SetKeyHook",              L_Game_SetKeyHook},
    {"GetKeyByAction",          L_Game_GetKeyByAction},
    {"Log",                     L_Game_Log},
    {"SetBlockCenter",          L_Game_SetBlockCenter},
    {"SetBlockRotation",        L_Game_SetBlockRotation},
    {"SetDynamicEntityRotation",    L_Game_SetDynamicEntityRotation},
    {"SetDynamicEntityTranslation", L_Game_SetDynamicEntityTranslation},
    {"SetDynamicEntityNone",        L_Game_SetDynamicEntityNone},
    {"SetDynamicBlockRotation",     L_Game_SetDynamicBlockRotation},
    {"SetDynamicBlockTranslation",  L_Game_SetDynamicBlockTranslation},
    {"SetDynamicBlockNone",         L_Game_SetDynamicBlockNone},
    {"CameraZoom",        L_Game_CameraZoom},
    {"CameraMove",        L_Game_CameraMove},
    {"GetEntityRadius",       L_Game_GetEntityRadius},
    {"IsEntityTouched",       L_Game_IsEntityTouched},
    {"KillPlayer",                  L_Game_KillPlayer},
    {"KillEntity",                  L_Game_KillEntity},
    {"RemainingStrawberries",       L_Game_RemainingStrawberries},
    {"WinPlayer",                   L_Game_WinPlayer},
    {"AddPenaltyTime",              L_Game_PenaltyTime},
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
    m_bDeathAnimEnabled=true;
    clearStates();
    m_lastCallToEveryHundreath = 0.0;
#if defined(ALLOW_GHOST)
    m_showGhostTimeDiff = true;
    m_isGhostActive = false;
#endif
    m_renderer      = NULL;
    m_isScriptActiv = false;
    m_bIsAReplay    = false;

    bFrontWheelTouching = false;
    bRearWheelTouching  = false;

    m_bodyDetach = false;

    m_motoGameHooks = NULL;
  }
  
  MotoGame::~MotoGame() {
  }  

  void MotoGame::setHooks(MotoGameHooks *i_motoGameHooks) {
    m_motoGameHooks = i_motoGameHooks;
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
    Update game
    ===========================================================================*/
  void MotoGame::updateLevel(float fTimeStep,SerializedBikeState *pReplayState,Replay *p_replay) {
    m_bSqueeking = false; /* no squeeking right now */

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
      m_BikeS.reInitializeAnchors();
      Vector2f C( m_TeleportDest.Pos - m_BikeS.Anchors().GroundPoint());
      _PrepareBikePhysics(C);
          
      m_BikeS.Dir = m_TeleportDest.bDriveRight?DD_RIGHT:DD_LEFT;

      m_BikeS.reInitializeSpeed();
      
      m_bTeleport = false;
    }

    getLevelSrc()->updateToTime(*this);
    updateGameMessages();
    
    /* Increase time */
    if(m_bIsAReplay == false)
      m_fTime += fTimeStep;
    else
      m_fTime = pReplayState->fGameTime;
    
    /* Are we going to change direction during this update? */
    bool bChangeDir = false;
    if(m_BikeC.ChangeDir()) {
      m_BikeC.setChangeDir(false);
      bChangeDir = true;
      
      m_BikeS.Dir = m_BikeS.Dir==DD_LEFT?DD_RIGHT:DD_LEFT; /* switch */
    }
  
    /* Update game state */
    _UpdateGameState(pReplayState);
        
    /* Update misc stuff (only when not playing a replay) */
    if(m_bIsAReplay == false) {
      _UpdateZones();
      _UpdateEntities();
    }
    
    /* Invoke PreDraw() script function - deprecated */
    if(m_isScriptActiv && isDead() == false) {
      if(!scriptCallBool("PreDraw",
       true)) {
  throw Exception("level script PreDraw() returned false");
      }
    }

    /* Invoke Tick() script function */
    /* and play script dynamic objects */
    int v_nbCents = 0;
    while(getTime() - m_lastCallToEveryHundreath > 0.01) {
      if(m_isScriptActiv && isDead() == false) {
	if(!scriptCallBool("Tick",
			   true)) {
			     throw Exception("level script Tick() returned false");
			   }
      }
      v_nbCents++;
      m_lastCallToEveryHundreath += 0.01;
    }
    nextStateScriptDynamicObjects(v_nbCents);

    /* Only make a full physics update when not replaying */
    if(m_bIsAReplay == false) {
      /* Update physics */
      _UpdatePhysics(fTimeStep);

      /* New wheel-spin particles? */
      if(isWheelSpinning()) {
	if(randomNum(0,1) < 0.7f) {
	  ParticlesSource *v_debris;
	  v_debris = (ParticlesSource*) &(getLevelSrc()->getEntityById("BikeDebris"));
	  v_debris->setDynamicPosition(getWheelSpinPoint());	
	  v_debris->addParticle(getWheelSpinDir(), m_pMotoGame->getTime() + 3.0);
	}
      }

      if(isDead() == false) {
	executeEvents(p_replay);
      }
    }
    else {
      /* Well, handle replay events instead */
      _UpdateReplayEvents(p_replay);
    }

    /* Entities scheduled for termination? */
    for(int i=0;i<m_DelSchedule.size();i++) {
      _KillEntity(m_DelSchedule[i]);
    }
    m_DelSchedule.clear();
    
    // we don't change the sens of the wheel depending on the side, because 
    // loops must not make done just by changing the side
    if(m_bIsAReplay == false) { /* this does not work for replays */
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
    m_BikeS.reInitializeSpeed();
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
         Level *pLevelSrc,
         Replay *recordingReplay,
         bool bIsAReplay) {

    m_bIsAReplay = bIsAReplay;
    m_isScriptActiv = (m_bIsAReplay == false);
    m_bLevelInitSuccess = true;

    /* Clean up first, just for safe's sake */
    endLevel();               

    /* load the level if not */
    if(pLevelSrc->isFullyLoaded() == false) {
      pLevelSrc->loadFullyFromFile();
    }

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
    pLevelSrc->setCollisionSystem(&m_Collision);

    /* Clear stuff */
    clearStates();
    
    m_bWheelSpin = false;
    
    m_fTime = 0.0f;
    m_fFinishTime = 0.0f;
    m_fNextAttitudeCon = -1000.0f;
    m_fAttitudeCon = 0.0f;
    
    m_nStillFrames = 0;
    
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

    m_lastCallToEveryHundreath = 0.0;

    m_renderer->initCamera();
    m_renderer->initZoom();

    /* Load and parse level script */
    bool bTryParsingEncapsulatedLevelScript = true;
    bool bNeedScript = false;
    bool bGotScript = false;
    
    if(pLevelSrc->scriptFileName() != "") {
      FileHandle *pfh = FS::openIFile(std::string("./Levels/") + pLevelSrc->scriptFileName());
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
        int nRet = luaL_loadbuffer(m_pL,ScriptBuf.c_str(),ScriptBuf.length(),pLevelSrc->scriptFileName().c_str()) ||
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
    
    if(bTryParsingEncapsulatedLevelScript && pLevelSrc->scriptSource() != "") {
      /* Use the Lua aux lib to load the buffer */
      int nRet = luaL_loadbuffer(m_pL,pLevelSrc->scriptSource().c_str(),pLevelSrc->scriptSource().length(),
         pLevelSrc->FileName().c_str()) || lua_pcall(m_pL,0,0,0);    
         
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
      Log(std::string("** Warning ** : Level generation failed !\n" + e.getMsg()).c_str());
      m_bLevelInitSuccess = false;
      return;
    }        

    /* Calculate bike stuff */
    m_BikeS.reInitializeAnchors();
    Vector2f C( pLevelSrc->PlayerStart() - m_BikeS.Anchors().GroundPoint());
    _PrepareBikePhysics(C);
    setBodyDetach(false);    

    /* Drive left-to-right for starters */
    m_BikeS.Dir = DD_RIGHT;
    
    m_BikeS.reInitializeSpeed();

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
	return;
      }
    }

#if defined(ALLOW_GHOST)
	   m_myLastStrawberries.clear();
	   m_ghostLastStrawberries.clear();
	   m_myDiffOfGhost = 0.0;
	   
	   if(m_pGhostReplay != NULL) {
	     InitGhostLastStrawberries(m_pGhostReplay);
	   }
#endif

    /* add the debris particlesSource */
    ParticlesSource *v_debris = new ParticlesSourceDebris("BikeDebris");
    v_debris->loadToPlay();
    getLevelSrc()->spawnEntity(v_debris);

    /* counter of somersaultcounter */
    m_somersaultCounter.init();

    /* execute events */
    executeEvents(recordingReplay);

    if(m_bIsAReplay == false) {
      /* Update game state */
      _UpdateGameState(NULL);
    }

  }

  void MotoGame::playLevel(
#if defined(ALLOW_GHOST)    
         Replay *m_pGhostReplay,
#endif
         Level *pLevelSrc,
         bool bIsAReplay) {
  }

  /*===========================================================================
    Free this game object
    ===========================================================================*/
  void MotoGame::endLevel(void) {
    /* If not already freed */
    if(m_pLevelSrc != NULL) {
      /* Clean up */
      lua_close(m_pL);
      m_pL = NULL;
      
      /* Stop physics */
      _UninitPhysics();

      m_entitiesTouching.clear();
      m_zonesTouching.clear();
      m_pLevelSrc->unloadToPlay();      

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
    std::vector<Block *> &InBlocks = m_pLevelSrc->Blocks();           

    /* Start by determining the bounding box of the level */
    Vector2f LevelBoundsMin,LevelBoundsMax;
    LevelBoundsMin.x = m_pLevelSrc->LeftLimit();
    LevelBoundsMax.x = m_pLevelSrc->RightLimit();
    LevelBoundsMin.y = m_pLevelSrc->BottomLimit();
    LevelBoundsMax.y = m_pLevelSrc->TopLimit();
    for(int i=0;i<InBlocks.size();i++) { 
      for(int j=0;j<InBlocks[i]->Vertices().size();j++) {
        LevelBoundsMin.x = InBlocks[i]->InitialPosition().x+InBlocks[i]->Vertices()[j]->Position().x < LevelBoundsMin.x ? 
          InBlocks[i]->InitialPosition().x+InBlocks[i]->Vertices()[j]->Position().x : LevelBoundsMin.x;
        LevelBoundsMin.y = InBlocks[i]->InitialPosition().y+InBlocks[i]->Vertices()[j]->Position().y < LevelBoundsMin.y ? 
          InBlocks[i]->InitialPosition().y+InBlocks[i]->Vertices()[j]->Position().y : LevelBoundsMin.y;
        LevelBoundsMax.x = InBlocks[i]->InitialPosition().x+InBlocks[i]->Vertices()[j]->Position().x > LevelBoundsMax.x ? 
          InBlocks[i]->InitialPosition().x+InBlocks[i]->Vertices()[j]->Position().x : LevelBoundsMax.x;
        LevelBoundsMax.y = InBlocks[i]->InitialPosition().y+InBlocks[i]->Vertices()[j]->Position().y > LevelBoundsMax.y ? 
          InBlocks[i]->InitialPosition().y+InBlocks[i]->Vertices()[j]->Position().y : LevelBoundsMax.y;
      }
    }

    Log("MotoGame::_GenerateLevel. number of layers: %d", m_pLevelSrc->getNumberLayer());
    for(int i=0; i<m_pLevelSrc->getNumberLayer(); i++){
      Log("MotoGame::_GenerateLevel. offset layer %d: %f, %f",
	  i, m_pLevelSrc->getLayerOffset(i).x, m_pLevelSrc->getLayerOffset(i).y);
    }
    
    m_Collision.setDims(LevelBoundsMin.x,LevelBoundsMin.y,
			LevelBoundsMax.x,LevelBoundsMax.y,
			m_pLevelSrc->getNumberLayer(),
			m_pLevelSrc->getLayerOffsets());

    Log("Generating level from %d block%s...",InBlocks.size(),InBlocks.size()==1?"":"s");
    
    /* For each input block */
    int nTotalBSPErrors = 0;
    
    m_zonesTouching.clear();
    m_entitiesTouching.clear();

    nTotalBSPErrors = m_pLevelSrc->loadToPlay();

    Log(" %d poly%s in total",m_pLevelSrc->Blocks().size(),m_pLevelSrc->Blocks().size()==1?"":"s");        
    
    if(nTotalBSPErrors > 0) {
      Log(" %d BSP error%s in total",nTotalBSPErrors,nTotalBSPErrors==1?"":"s");
      gameMessage(std::string(GAMETEXT_WARNING) + ":");
      gameMessage(GAMETEXT_ERRORSINLEVEL);
    }

    if(m_bIsAReplay == false){
      /* Give limits to collision system */
      m_Collision.defineLine( m_pLevelSrc->LeftLimit(), m_pLevelSrc->TopLimit(),
			      m_pLevelSrc->LeftLimit(), m_pLevelSrc->BottomLimit(),
			      DEFAULT_PHYS_WHEEL_GRIP);
      m_Collision.defineLine( m_pLevelSrc->LeftLimit(), m_pLevelSrc->BottomLimit(),
			      m_pLevelSrc->RightLimit(), m_pLevelSrc->BottomLimit(),
			      DEFAULT_PHYS_WHEEL_GRIP );
      m_Collision.defineLine( m_pLevelSrc->RightLimit(), m_pLevelSrc->BottomLimit(),
			      m_pLevelSrc->RightLimit(), m_pLevelSrc->TopLimit(),
			      DEFAULT_PHYS_WHEEL_GRIP );
      m_Collision.defineLine( m_pLevelSrc->RightLimit(), m_pLevelSrc->TopLimit(),
			      m_pLevelSrc->LeftLimit(), m_pLevelSrc->TopLimit(),
			      DEFAULT_PHYS_WHEEL_GRIP );

      /* Show stats about the collision system */
      CollisionSystemStats CStats;
      m_Collision.getStats(&CStats);
      Log(" %dx%d grid with %.1fx%.1f cells (%.0f%% empty)",
	  CStats.nGridWidth,CStats.nGridHeight,CStats.fCellWidth,CStats.fCellHeight,
	  CStats.fPercentageOfEmptyCells);
      Log(" %d total blocking lines",CStats.nTotalLines);
    }
  }


  bool MotoGame::touchEntityBodyExceptHead(const BikeState &pBike, const Entity &p_entity) {
    Vector2f res1, res2;

    if(pBike.Dir == DD_RIGHT) {
      if(intersectLineCircle2f(p_entity.DynamicPosition(), p_entity.Size(),
             pBike.FootP, pBike.KneeP,
             res1, res2) > 0) {
         return true;
             }

      if(intersectLineCircle2f(p_entity.DynamicPosition(), p_entity.Size(),
             pBike.KneeP, pBike.LowerBodyP,
             res1, res2) > 0) {
         return true;
             }
      
      if(intersectLineCircle2f(p_entity.DynamicPosition(), p_entity.Size(),
             pBike.LowerBodyP, pBike.ShoulderP,
             res1, res2) > 0) {
         return true;
             }

      if(intersectLineCircle2f(p_entity.DynamicPosition(), p_entity.Size(),
             pBike.ShoulderP, pBike.ElbowP,
             res1, res2) > 0) {
         return true;
             }

      if(intersectLineCircle2f(p_entity.DynamicPosition(), p_entity.Size(),
             pBike.ElbowP, pBike.HandP,
             res1, res2) > 0) {
         return true;
             }
      return false;
    }

    if(pBike.Dir == DD_LEFT) {
      if(intersectLineCircle2f(p_entity.DynamicPosition(), p_entity.Size(),
             pBike.Foot2P, pBike.Knee2P,
             res1, res2) > 0) {
         return true;
             }

      if(intersectLineCircle2f(p_entity.DynamicPosition(), p_entity.Size(),
             pBike.Knee2P, pBike.LowerBody2P,
             res1, res2) > 0) {
         return true;
             }
      
      if(intersectLineCircle2f(p_entity.DynamicPosition(), p_entity.Size(),
             pBike.LowerBody2P, pBike.Shoulder2P,
             res1, res2) > 0) {
         return true;
             }

      if(intersectLineCircle2f(p_entity.DynamicPosition(), p_entity.Size(),
             pBike.Shoulder2P, pBike.Elbow2P,
             res1, res2) > 0) {
         return true;
             }

      if(intersectLineCircle2f(p_entity.DynamicPosition(), p_entity.Size(),
             pBike.Elbow2P, pBike.Hand2P,
             res1, res2) > 0) {
         return true;
             }
      return false;
    }

    return false;
  }

  bool MotoGame::isTouching(const Entity& i_entity) const {
    for(int i=0; i<m_entitiesTouching.size(); i++) {
      if(m_entitiesTouching[i] == &i_entity) {
        return true;
      }
    }
    return false;
  }

  MotoGame::touch MotoGame::setTouching(Entity& i_entity, bool i_touching) {
    bool v_wasTouching = isTouching(i_entity);
    if(v_wasTouching == i_touching) {
      return none;
    }

    if(i_touching) {
      m_entitiesTouching.push_back(&i_entity);
      return added;
    } else {
      for(int i=0; i<m_entitiesTouching.size(); i++) {
        if(m_entitiesTouching[i] == &i_entity) {
          m_entitiesTouching.erase(m_entitiesTouching.begin() + i);
          return removed;
        }
      }
    }
    return none;
  }

  
  /*===========================================================================
    Update zone specific stuff -- call scripts where needed
    ===========================================================================*/
  void MotoGame::_UpdateZones(void) {
    /* Check player touching for each zone */
    for(int i=0;i<m_pLevelSrc->Zones().size();i++) {
      Zone *pZone = m_pLevelSrc->Zones()[i];
      
      /* Check it against the wheels and the head */
      if(pZone->doesCircleTouch(m_BikeS.FrontWheelP, m_BikeS.Parameters().WheelRadius()) ||
         pZone->doesCircleTouch(m_BikeS.RearWheelP,  m_BikeS.Parameters().WheelRadius())) {       
        /* In the zone -- did he just enter it? */
        if(setTouching(*pZone, true) == added){
          createGameEvent(new MGE_PlayerEntersZone(getTime(), pZone));
        }
      }         
      else {
        /* Not in the zone... but was he during last update? - i.e. has 
           he just left it? */      
        if(setTouching(*pZone, false) == removed){
          createGameEvent(new MGE_PlayerLeavesZone(getTime(), pZone));
        }
      }
    }
  }


  void MotoGame::_UpdateEntities(void) {
    Vector2f HeadPos = m_BikeS.Dir==DD_RIGHT?m_BikeS.HeadP:m_BikeS.Head2P;

    /* Get biker bounding box */
    AABB BBox;
    float headSize = m_BikeS.Parameters().HeadSize();
    float wheelRadius = m_BikeS.Parameters().WheelRadius();
    /* in case the body is outside of the aabb */
    float securityMargin = 0.5;

    BBox.addPointToAABB2f(HeadPos[0]-headSize-securityMargin,
			  HeadPos[1]-headSize-securityMargin);
    BBox.addPointToAABB2f(m_BikeS.FrontWheelP[0]-wheelRadius-securityMargin,
			  m_BikeS.FrontWheelP[1]-wheelRadius-securityMargin);
    BBox.addPointToAABB2f(m_BikeS.RearWheelP[0]-wheelRadius-securityMargin,
			  m_BikeS.RearWheelP[1]-wheelRadius-securityMargin);

    BBox.addPointToAABB2f(HeadPos[0]+headSize+securityMargin,
			  HeadPos[1]+headSize+securityMargin);
    BBox.addPointToAABB2f(m_BikeS.FrontWheelP[0]+wheelRadius+securityMargin,
			  m_BikeS.FrontWheelP[1]+wheelRadius+securityMargin);
    BBox.addPointToAABB2f(m_BikeS.RearWheelP[0]+wheelRadius+securityMargin,
			  m_BikeS.RearWheelP[1]+wheelRadius+securityMargin);

    std::vector<Entity*> entities = m_Collision.getEntitiesNearPosition(BBox);

    /* Do player touch anything? */
    for(int i=0;i<entities.size();i++) {
      /* Test against the biker aabb first */
      if(true){
	/* Head? */
	if(circleTouchCircle2f(entities[i]->DynamicPosition(),
			       entities[i]->Size(),
			       HeadPos,
			       m_BikeS.Parameters().HeadSize())) {
	  if(setTouching(*(entities[i]), true) == added){
	    createGameEvent(new MGE_PlayerTouchesEntity(getTime(),
							entities[i]->Id(),
							true));
	  }
	  
	  /* Wheel then ? */
	} else if(circleTouchCircle2f(entities[i]->DynamicPosition(),
				      entities[i]->Size(),
				      m_BikeS.FrontWheelP,
				      m_BikeS.Parameters().WheelRadius()) ||
		  circleTouchCircle2f(entities[i]->DynamicPosition(),
				      entities[i]->Size(),
				      m_BikeS.RearWheelP,
				      m_BikeS.Parameters().WheelRadius())) {
	  if(setTouching(*(entities[i]), true) == added){
	    createGameEvent(new MGE_PlayerTouchesEntity(getTime(),
							entities[i]->Id(),
							false));
	  }
	  
	  /* body then ?*/
	} else if(touchEntityBodyExceptHead(m_BikeS, *(entities[i]))) {
	  if(setTouching(*(entities[i]), true) == added){
	    createGameEvent(new MGE_PlayerTouchesEntity(getTime(),
							entities[i]->Id(),
							false));
	  }
	} else {
	  /* TODO::generate an event "leaves entity" if needed */
	  setTouching(*(entities[i]), false);
	}
      } else {
	/* TODO::generate an event "leaves entity" if needed */
	setTouching(*(entities[i]), false);
      }
    }
  }


  bool MotoGame::isTouching(const Zone& i_zone) const {
    for(unsigned int i=0; i<m_zonesTouching.size(); i++) {
      if(m_zonesTouching[i]->Id() == i_zone.Id()) {
        return true;
      }
    }
    return false;
  }

  MotoGame::touch MotoGame::setTouching(Zone& i_zone, bool i_isTouching) {
    bool v_wasTouching = isTouching(i_zone);
    if(v_wasTouching == i_isTouching) {
      return none;
    }
    
    if(i_isTouching) {
      m_zonesTouching.push_back(&i_zone);
      return added;
    } else {
      for(int i=0; i<m_zonesTouching.size(); i++) {
        if(m_zonesTouching[i] == &i_zone) {
          m_zonesTouching.erase(m_zonesTouching.begin() + i);
          return removed;
        }
      }
    }
    return none;
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
      scriptCallTblVoid(pEntity->Id(),"Touch");
    }

    if(pEntity->DoesMakeWin()) {
      if(getNbRemainingStrawberries() == 0) {
	makePlayerWin();
      }
    }

    if(pEntity->DoesKill()) {
      createGameEvent(new MGE_PlayerDies(getTime(), true));
    }

    if(pEntity->IsToTake()) {
      /* OH... nice */
      createGameEvent(new MGE_EntityDestroyed(getTime(), pEntity->Id(), pEntity->Speciality(), pEntity->DynamicPosition(), pEntity->Size()));
#if defined(ALLOW_GHOST)
      if(isGhostActive() && m_showGhostTimeDiff) {
	m_myLastStrawberries.push_back(getTime());
	UpdateDiffFromGhost();
	DisplayDiffFromGhost();
      }
#endif
    }
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

  void MotoGame::SetEntityPos(String pEntityID, float pX, float pY) {
    SetEntityPos(&(m_pLevelSrc->getEntityById(pEntityID)),
		 pX, pY);
  }

  void MotoGame::SetEntityPos(Entity *pEntity, float pX, float pY) {
    pEntity->setDynamicPosition(Vector2f(pX, pY));
    // move it in the collision system only if it's not dead
    if(pEntity->isAlive() == true){
      m_Collision.moveEntity(pEntity);
    }
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
    Block& v_block = m_pLevelSrc->getBlockById(pBlockID);
    v_block.setDynamicPosition(v_block.DynamicPosition() + Vector2f(pX, pY));
    m_Collision.moveDynBlock(&v_block);
  }
  
  void MotoGame::SetBlockPos(String pBlockID, float pX, float pY) {
    Block& v_block = m_pLevelSrc->getBlockById(pBlockID);
    v_block.setDynamicPosition(Vector2f(pX, pY));
    m_Collision.moveDynBlock(&v_block);
  }
  
  void MotoGame::SetBlockCenter(String pBlockID, float pX, float pY) {
    Block& v_block = m_pLevelSrc->getBlockById(pBlockID);
    v_block.setCenter(Vector2f(pX, pY));
    m_Collision.moveDynBlock(&v_block);
  }
  
  void MotoGame::SetBlockRotation(String pBlockID, float pAngle) {
    Block& v_block = m_pLevelSrc->getBlockById(pBlockID);
    v_block.setDynamicRotation(pAngle);
    m_Collision.moveDynBlock(&v_block);
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
	if(getLevelSrc()->getEntityById(((MGE_EntityDestroyed*)v_event)->EntityId()).IsToTake()) {
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

  void MotoGame::nextStateScriptDynamicObjects(int i_nbCents) {
    int i = 0;

    while(i<m_SDynamicObjects.size()) {
      if(m_SDynamicObjects[i]->nextState(this, i_nbCents) == false) {
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
    setBodyDetach(m_bDeathAnimEnabled);
    m_BikeC.stopContols();
  }

  void MotoGame::playerEntersZone(Zone *pZone) {
    if(m_isScriptActiv) {
      scriptCallTblVoid(pZone->Id(), "OnEnter");
    }
  }
  
  void MotoGame::playerLeavesZone(Zone *pZone) {
    if(m_isScriptActiv) {
      scriptCallTblVoid(pZone->Id(), "OnLeave");
    }
  }

  void MotoGame::playerTouchesEntity(std::string p_entityID, bool p_bTouchedWithHead) {
    touchEntity(&(getLevelSrc()->getEntityById(p_entityID)), p_bTouchedWithHead);
  }

  void MotoGame::entityDestroyed(const std::string& i_entityId) {
    Entity *v_entity;
    v_entity = &(getLevelSrc()->getEntityById(i_entityId));

    if(v_entity->IsToTake()) {
      ParticlesSource *v_stars = new ParticlesSourceStar("");
      v_stars->setInitialPosition(v_entity->DynamicPosition());
      v_stars->loadToPlay();
      for(int i=0; i<3; i++) {
	v_stars->addParticle(Vector2f(0,0), m_pMotoGame->getTime() + 5.0);
      }
      getLevelSrc()->spawnEntity(v_stars);
      
      if(m_motoGameHooks != NULL) {
	m_motoGameHooks->OnTakeEntity();
      }
    }
    
    /* Destroy entity */
    deleteEntity(v_entity);
  }

  void MotoGame::addDynamicObject(SDynamicObject *p_obj) {
    m_SDynamicObjects.push_back(p_obj);
  }

  void MotoGame::createKillEntityEvent(std::string p_entityID) {
    Entity *v_entity;
    v_entity = &(m_pLevelSrc->getEntityById(p_entityID));
    createGameEvent(new MGE_EntityDestroyed(m_pMotoGame->getTime(),
                                            v_entity->Id(),
                                            v_entity->Speciality(),
                                            v_entity->DynamicPosition(),
                                            v_entity->Size()));
  }

  unsigned int MotoGame::getNbRemainingStrawberries() {
    return m_pLevelSrc->countToTakeEntities();
  }

  void MotoGame::makePlayerWin() {
    m_bFinished = true;
    m_fFinishTime = getTime();
  }

  void MotoGame::setBodyDetach(bool state) {
    m_bodyDetach = state;
    m_renderer->setRenderBikeFront(! m_bodyDetach);

    if(m_bodyDetach) {
      dJointSetHingeParam(m_KneeHingeID,  dParamLoStop, 0.0);
      dJointSetHingeParam(m_KneeHingeID,  dParamHiStop, 3.14159/8.0);
      dJointSetHingeParam(m_KneeHingeID2, dParamLoStop, 3.14159/8.0 * -1.0);
      dJointSetHingeParam(m_KneeHingeID2, dParamHiStop, 0.0         * -1.0);

      dJointSetHingeParam(m_LowerBodyHingeID,  dParamLoStop,  -1.2);
      dJointSetHingeParam(m_LowerBodyHingeID,  dParamHiStop,  0.0);
      dJointSetHingeParam(m_LowerBodyHingeID2, dParamLoStop, 0.0  * -1.0);
      dJointSetHingeParam(m_LowerBodyHingeID2, dParamHiStop, -1.2 * -1.0);

      dJointSetHingeParam(m_ShoulderHingeID,  dParamLoStop, -2.0);
      dJointSetHingeParam(m_ShoulderHingeID,  dParamHiStop,  0.0);
      dJointSetHingeParam(m_ShoulderHingeID2, dParamLoStop,  0.0  * -1.0);
      dJointSetHingeParam(m_ShoulderHingeID2, dParamHiStop, -2.0  * -1.0);

      dJointSetHingeParam(m_ElbowHingeID,  dParamLoStop, -1.5);
      dJointSetHingeParam(m_ElbowHingeID,  dParamHiStop, 1.0);
      dJointSetHingeParam(m_ElbowHingeID2, dParamLoStop, 1.0   * -1.0);
      dJointSetHingeParam(m_ElbowHingeID2, dParamHiStop, -1.5  * -1.0);
    }
  }

  void MotoGame::addPenalityTime(float fTime) {
    m_fTime += fTime;
  }

  void MotoGame::_KillEntity(Entity *pEnt) {
    getLevelSrc()->killEntity(pEnt->Id());
    /* now that rendering use the space partionnement,
       we have to remove entity from the collision system */
    m_Collision.removeEntity(pEnt);

    /* special case for the last strawberry : if we are touching the end, finish the level */
    if(getNbRemainingStrawberries() == 0) {
      bool v_touchingMakeWin = false;
      for(int i=0; i<m_entitiesTouching.size(); i++) {
	if(m_entitiesTouching[i]->DoesMakeWin()) {
	  v_touchingMakeWin = true;
	}
      }
      if(v_touchingMakeWin) {
	makePlayerWin();
      }
    }
  }

}
