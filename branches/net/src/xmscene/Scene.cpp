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

/* 
 *  Game object. Handles all of the gamestate management und so weiter.
 */
#include "../GameText.h"
#include "../Game.h"
#include "Scene.h"
#include "../VFileIO.h"
#include "../BSP.h"
#include "../Sound.h"
#include "../PhysSettings.h"
#include "../GameEvents.h"
#include "Entity.h"
#include "BikeGhost.h"
#include "BikePlayer.h"
#include "BikeParameters.h"
#include "../helpers/Log.h"
#include "Camera.h"
#include "Block.h"
#include "Entity.h"
#include "../Replay.h"
#include "../LuaLibGame.h"
#include "../ScriptDynamicObjects.h"
#include "ChipmunkWorld.h"
#include "../helpers/Random.h"
#include "PhysicsSettings.h"
#include "../net/NetClient.h"
#include "../net/NetActions.h"

  MotoGame::MotoGame() {
    m_bDeathAnimEnabled=true;
    m_lastCallToEveryHundreath = 0;

    m_showGhostTimeDiff = true;

    m_motoGameHooks = NULL;

    m_speed_factor = 1.00f;
    m_is_paused = false;
    m_playEvents = true;

    m_lastStateSerializationTime = -100; /* loong time ago :) */

    m_pLevelSrc = NULL;

    m_luaGame = NULL;

    m_currentCamera = 0;

    m_chipmunkWorld = NULL;

    m_halfUpdate = true;

    m_physicsSettings = NULL;
  }
  
  MotoGame::~MotoGame() {
    cleanPlayers();
    cleanGhosts();
  }

  void MotoGame::loadLevel(xmDatabase *i_db, const std::string& i_id_level) {
    char **v_result;
    unsigned int nrow;

    m_pLevelSrc = new Level();
    try {
      v_result = i_db->readDB("SELECT filepath FROM levels "
			      "WHERE id_level=\"" + xmDatabase::protectString(i_id_level) + "\";",
			      nrow);
      if(nrow != 1) {
	i_db->read_DB_free(v_result);
	throw Exception("Level " + i_id_level + " not found");
      }
      m_pLevelSrc->setFileName(i_db->getResult(v_result, 1, 0, 0));
      i_db->read_DB_free(v_result);

      m_pLevelSrc->loadReducedFromFile();

    } catch(Exception &e) {
      delete m_pLevelSrc;
      m_pLevelSrc = NULL;
      throw e;
    }
  }

void MotoGame::cleanGhosts() {
  for(unsigned int i=0; i<m_ghosts.size(); i++) {
    delete m_ghosts[i];
  }
  m_ghosts.clear();
}

void MotoGame::cleanPlayers() {
  for(unsigned int i=0; i<m_players.size(); i++) {
    if(m_players[i]->getOnBikerHooks() != NULL) {
      delete m_players[i]->getOnBikerHooks();
    }
    delete m_players[i];
  }
  m_players.clear();
}

  void MotoGame::setHooks(MotoGameHooks *i_motoGameHooks) {
    m_motoGameHooks = i_motoGameHooks;
  }

  /*===========================================================================
    Teleporting
    ===========================================================================*/
  void MotoGame::setPlayerPosition(int i_player, float x,float y,bool bFaceRight) {
    /* Going to teleport? Do it now, before we tinker to much with the state */
    if(m_players[i_player]->isDead() == false) {
      m_players[i_player]->initToPosition(Vector2f(x,y), bFaceRight?DD_RIGHT:DD_LEFT, m_PhysGravity);
    }
  }
  
  const Vector2f &MotoGame::getPlayerPosition(int i_player) {
    return m_players[i_player]->getState()->CenterP;
    throw Exception("Invalid player number");
  }
  
  bool MotoGame::getPlayerFaceDir(int i_player) {
    return m_players[i_player]->getState()->Dir == DD_RIGHT;
    throw Exception("Invalid player number");
  }

  /*===========================================================================
    Add game message
    ===========================================================================*/
  void MotoGame::gameMessage(std::string Text, bool bOnce, int duration) {
    /* "unique"? */
    GameMessage *pMsg = NULL;
    
    if(bOnce) {
      /* Yeah, if there's another "unique", replace it */
      for(unsigned int i=0;i<m_GameMessages.size();i++) {
        if(m_GameMessages[i]->bOnce) {
          pMsg = m_GameMessages[i];
          break;
        }
      }
    }
    
    if(pMsg == NULL) {    
      if(bOnce) { /* don't allow \n split on bOnce messages (i'm too lazy) */
	pMsg = new GameMessage;
	pMsg->removeTime = getTime() + duration;
	pMsg->bNew = true;
	pMsg->nAlpha = 255;
	pMsg->bOnce = bOnce;
	pMsg->Text = Text;
	m_GameMessages.push_back(pMsg);	
      } else {
	/* split the message if \n is encoutered */
	std::string v_txtRest = Text;
	int n = v_txtRest.find_first_of("\n");

	while(n >= 0 && (unsigned int)n < v_txtRest.length()) {
	  pMsg = new GameMessage;
	  pMsg->removeTime = getTime() + duration;
	  pMsg->bNew = true;
	  pMsg->nAlpha = 255;
	  pMsg->bOnce = bOnce;
	  pMsg->Text = v_txtRest.substr(0, n);
	  m_GameMessages.push_back(pMsg);
	  v_txtRest = v_txtRest.substr(n+1, v_txtRest.length()-1);
	  n = v_txtRest.find_first_of("\n");
	}

	pMsg = new GameMessage;
	pMsg->removeTime = getTime() + duration;
	pMsg->bNew = true;
	pMsg->nAlpha = 255;
	pMsg->bOnce = bOnce;
	pMsg->Text = v_txtRest;
	m_GameMessages.push_back(pMsg);
      }
    } else {
      /* redefine the text only to replay a bonce message */
      pMsg->Text = Text;
      pMsg->removeTime = getTime() + duration;
      pMsg->nAlpha = 255;
    }

    updateGameMessages();
  }

  void MotoGame::clearGameMessages(void) {
    for(unsigned int i=0 ;i<m_GameMessages.size() ;i++)
      m_GameMessages[i]->removeTime=0;
  }

  /*===========================================================================
    Update game
    ===========================================================================*/
  void MotoGame::updateLevel(int timeStep, Replay *i_recordedReplay) {
    float v_diff;

    if(m_is_paused)
      return;

    if(m_halfUpdate == true) {
      getLevelSrc()->updateToTime(*this, m_physicsSettings);
      m_halfUpdate = false;
    } else
      m_halfUpdate = true;

    updateGameMessages();
    
    /* Increase time */
    if(m_speed_factor != 1.00f) {

      // save the diff when timeStep is not an integer to make nice rewind/forward
      v_diff = ((float)timeStep) * m_speed_factor;
      m_floattantTimeStepDiff += v_diff - ((int) v_diff);
      m_time += (int)(v_diff);
      m_time += ((int)m_floattantTimeStepDiff);
      m_floattantTimeStepDiff -= ((int)m_floattantTimeStepDiff);

      if(m_time < 0)
	m_time = 0;
    } else {
      m_time += timeStep;
    }     

    /* Update misc stuff (only when not playing a replay) */
    if(m_playEvents) {
      getLevelSrc()->updatePhysics(timeStep, &m_Collision, m_chipmunkWorld);
      _UpdateZones();
      _UpdateEntities();
    }

    /* Invoke Tick() script function */
    /* and play script dynamic objects */
    int v_nbCents = 0;
    while(getTime() - m_lastCallToEveryHundreath > 1) {
      if(m_playEvents) {
	if(m_luaGame->scriptCallBool("Tick", true) == false) {
	  throw Exception("level script Tick() returned false");
	}
      }
      v_nbCents++;
      m_lastCallToEveryHundreath += 1;
    }
    nextStateScriptDynamicObjects(v_nbCents);

    for(unsigned int i=0; i<m_ghosts.size(); i++) {
      m_ghosts[i]->updateToTime(getTime(), timeStep, &m_Collision, m_PhysGravity, this);
    }
    
    for(unsigned int i=0; i<m_players.size(); i++) {
      m_players[i]->updateToTime(m_time, timeStep, &m_Collision, m_PhysGravity, this);

      if(m_playEvents) {
	/* New wheel-spin particles? */
	if(m_players[i]->isWheelSpinning()) {
	  if(NotSoRandom::randomNum(0,1) < 0.7f) {
	    ParticlesSource *v_debris;
	    v_debris = (ParticlesSource*) getLevelSrc()->getEntityById("BikeDebris");
	    v_debris->setDynamicPosition(m_players[i]->getWheelSpinPoint());	
	    v_debris->addParticle(m_time);
	  }
	}
      }
    }

    if(m_chipmunkWorld != NULL) {
      /* players moves, update their positions */
      m_chipmunkWorld->updateWheelsPosition(m_players);
    }

    executeEvents(i_recordedReplay);

    /* save the replay */
    if(i_recordedReplay != NULL) {
      /* We'd like to serialize the game state 25 times per second for the replay */
      if(getTime() - m_lastStateSerializationTime >= 100.0f/i_recordedReplay->getFrameRate()) {
        m_lastStateSerializationTime = getTime();
        
        /* Get it */
	for(unsigned int i=0; i<Players().size(); i++) {
	  if(Players().size() == 1 || NetClient::instance()->isConnected()) {
	    if(Players()[i]->isDead() == false && Players()[i]->isFinished() == false) {
	      SerializedBikeState BikeState;
	      getSerializedBikeState(Players()[i]->getState(), getTime(), &BikeState, m_physicsSettings);

	      /* only store the state if 1 player plays */
	      if(Players().size() == 1) {
		i_recordedReplay->storeState(BikeState);
		i_recordedReplay->storeBlocks(m_pLevelSrc->Blocks());
	      }

	      // always send (framerate network = framerate replay for the moment)
	      NA_frame na(&BikeState);
	      NetClient::instance()->send(&na);
	    }
	  }
	}
      }
    }

    /* Entities scheduled for termination? */
    for(unsigned int i=0;i<m_DelSchedule.size();i++) {
      _KillEntity(m_DelSchedule[i]);
    }
    m_DelSchedule.clear();
  }

  void MotoGame::executeEvents(Replay *p_replay) {
    /* Handle events generated this update */
    while(getNumPendingGameEvents() > 0) {
      MotoGameEvent *pEvent = getNextGameEvent();
      if(p_replay != NULL) {
	/* Encode event */
	_SerializeGameEventQueue(p_replay, pEvent);
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
      if(i < 0 || (unsigned int)i >= m_GameMessages.size())
	break;      
      if(getTime() > m_GameMessages[i]->removeTime) {
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
    Prepare the specified level for playing through this game object
    ===========================================================================*/
  void MotoGame::prePlayLevel(Replay *recordingReplay,
			      bool i_playEvents) {
    m_playEvents = i_playEvents;
    /* load the level if not */
    if(m_pLevelSrc->isFullyLoaded() == false) {
      m_pLevelSrc->loadFullyFromFile();
    }
    
    /* Create Lua state */
    m_luaGame = new LuaLibGame(this);

    /* physics */
    m_physicsSettings = new PhysicsSettings("Physics/original.xml");
    
    /* Clear collision system */
    m_Collision.reset();
    m_pLevelSrc->setCollisionSystem(&m_Collision);

    /* Set default gravity */
    m_PhysGravity.x = 0;
    m_PhysGravity.y = -(m_physicsSettings->WorldGravity());

    m_time = 0;
    m_floattantTimeStepDiff = 0.0;
    m_speed_factor = 1.00f;
    m_is_paused = false;

    m_nLastEventSeq = 0;
    
    m_Arrow.nArrowPointerMode = 0;
 
    m_lastCallToEveryHundreath = 0;

    /* Load and parse level script */
    bool bTryParsingEncapsulatedLevelScript = true;
    bool bNeedScript = false;
    bool bGotScript = false;
    
    if(m_pLevelSrc->scriptFileName() != "") {
      FileHandle *pfh = FS::openIFile(std::string("./Levels/") + m_pLevelSrc->scriptFileName());
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

	try {
	  m_luaGame->loadScript(ScriptBuf, m_pLevelSrc->scriptFileName());
	} catch(Exception &e) {
	  delete m_luaGame;
	  m_luaGame = NULL;
	  throw e;
	}

        bGotScript = true;
        bTryParsingEncapsulatedLevelScript = false;
      }       
    }    
    
    if(bTryParsingEncapsulatedLevelScript && m_pLevelSrc->scriptSource() != "") {
      /* Use the Lua aux lib to load the buffer */

      try {
	m_luaGame->loadScript(m_pLevelSrc->scriptSource(), m_pLevelSrc->scriptFileName());
      } catch(Exception &e) {
	std::string error_msg = m_luaGame->getErrorMsg();
	delete m_luaGame;
	m_luaGame = NULL;
	throw Exception("failed to load level encapsulated script :\n" + error_msg);
      }      

      bGotScript = true;      
    }    
    
    if(bNeedScript && !bGotScript) {
      delete m_luaGame;
      m_luaGame = NULL;
      throw Exception("failed to get level script");
    }

    // load chimunk
    if(m_pLevelSrc->isPhysics()) {
      LogInfo("Running a physics level");
      m_chipmunkWorld = new ChipmunkWorld(m_physicsSettings);
    }

    /* Generate extended level data to be used by the game */
    try {
      _GenerateLevel();
    } catch(Exception &e) {
      LogWarning(std::string("Level generation failed !\n" + e.getMsg()).c_str());
      throw Exception(e);
    }        

    m_myLastStrawberries.clear();

    /* add the debris particlesSource */
    ParticlesSource *v_debris = new ParticlesSourceDebris("BikeDebris");
    v_debris->loadToPlay();
    v_debris->setZ(1.0);
    getLevelSrc()->spawnEntity(v_debris);

    /* execute events */
    m_lastStateSerializationTime = -100;
    if(m_playEvents) {
      executeEvents(recordingReplay);
    }
  }

  ReplayBiker* MotoGame::addReplayFromFile(std::string i_ghostFile,
					   Theme *i_theme, BikerTheme* i_bikerTheme,
					   bool i_enableEngineSound) {
    ReplayBiker* v_biker = NULL;
    v_biker = new ReplayBiker(i_ghostFile, m_physicsSettings, i_theme, i_bikerTheme);
    v_biker->setPlaySound(i_enableEngineSound);
    m_players.push_back(v_biker);
    return v_biker;
  }

  FileGhost* MotoGame::addGhostFromFile(std::string i_ghostFile, const std::string& i_info,
					Theme *i_theme, BikerTheme* i_bikerTheme,
					const TColor& i_filterColor,
					const TColor& i_filterUglyColor) {
    FileGhost* v_ghost = NULL;

    /* the level must be set to add a ghost */
    if(m_pLevelSrc == NULL) {
      throw Exception("No level defined");
    }

    v_ghost = new FileGhost(i_ghostFile, m_physicsSettings, false, i_theme, i_bikerTheme,
			    i_filterColor, i_filterUglyColor);
    v_ghost->setPlaySound(false);
    v_ghost->setInfo(i_info);
    v_ghost->initLastToTakeEntities(m_pLevelSrc);
    m_ghosts.push_back(v_ghost);
    return v_ghost;
  }

  NetGhost* MotoGame::addNetGhost(const std::string& i_info,
				  Theme *i_theme,
				  BikerTheme* i_bikerTheme,
				  const TColor& i_filterColor,
				  const TColor& i_filterUglyColor) {
    NetGhost* v_ghost = NULL;
    LogInfo("New NetGhost");

    v_ghost = new NetGhost(m_physicsSettings, i_theme, i_bikerTheme,
			   i_filterColor, i_filterUglyColor);
    v_ghost->setPlaySound(false);
    v_ghost->setInfo(i_info);
    m_ghosts.push_back(v_ghost);
    return v_ghost;
  }

  std::vector<Ghost *>& MotoGame::Ghosts() {
    return m_ghosts;
  }

  std::vector<Biker *>& MotoGame::Players() {
    return m_players;
  }

  void MotoGame::playLevel() {
  /* Invoke the OnLoad() script function */
    if(m_playEvents) {
      bool bOnLoadSuccess;

      try {
	bOnLoadSuccess = m_luaGame->scriptCallBool("OnLoad", true);
      } catch(Exception &e) {
	bOnLoadSuccess = false;
      }
      /* if no OnLoad(), assume success */
      /* Success? */
      if(!bOnLoadSuccess) {
	LogError("OnLoad script function failed !");
	throw Exception("OnLoad script function failed !");
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
      if(m_luaGame != NULL) {
	delete m_luaGame;
	m_luaGame = NULL;
      }
      m_pLevelSrc->unloadToPlay();      
      delete m_pLevelSrc;

      /* Release reference to level source */
      m_pLevelSrc = NULL;      
    }
    
    /* Get rid of game messages */
    for(unsigned int i=0; i<m_GameMessages.size(); i++)
      delete m_GameMessages[i];
    m_GameMessages.clear();

    /* clean Sdynamic objects for scripts */
    cleanScriptDynamicObjects();

    /* clean event queue */
    cleanEventsQueue();

    cleanPlayers();
    cleanGhosts();

    removeCameras();

    if(m_chipmunkWorld != NULL) {
      delete m_chipmunkWorld;
      m_chipmunkWorld = NULL;
    }

    if(m_physicsSettings != NULL) {
      delete m_physicsSettings;
    }
  }

  /*===========================================================================
    Level generation (i.e. parsing of level source)
    ===========================================================================*/
  void MotoGame::_GenerateLevel(void) {
    if(m_pLevelSrc == NULL) {
      LogWarning("Can't generate level when no source is assigned!");
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
    for(unsigned int i=0; i<InBlocks.size(); i++) { 
      for(unsigned int j=0; j<InBlocks[i]->Vertices().size(); j++) {
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

    m_Collision.setDims(LevelBoundsMin.x,LevelBoundsMin.y,
			LevelBoundsMax.x,LevelBoundsMax.y,
			m_pLevelSrc->getNumberLayer(),
			m_pLevelSrc->getLayerOffsets());

    LogInfo("Generating level from %d block%s...",InBlocks.size(),InBlocks.size()==1?"":"s");
    
    /* For each input block */
    int nTotalBSPErrors = 0;
    
    nTotalBSPErrors = m_pLevelSrc->loadToPlay(m_chipmunkWorld, m_physicsSettings);

    if(nTotalBSPErrors > 0) {
      LogWarning(" %d BSP error%s in total",nTotalBSPErrors,nTotalBSPErrors==1?"":"s");
      gameMessage(std::string(GAMETEXT_WARNING) + ":");
      gameMessage(GAMETEXT_ERRORSINLEVEL);
    }

    if(m_playEvents){
      /* Give limits to collision system */
      m_Collision.defineLine( m_pLevelSrc->LeftLimit(), m_pLevelSrc->TopLimit(),
			      m_pLevelSrc->LeftLimit(), m_pLevelSrc->BottomLimit(),
			      m_physicsSettings->BikeWheelBlockGrip());
      m_Collision.defineLine( m_pLevelSrc->LeftLimit(), m_pLevelSrc->BottomLimit(),
			      m_pLevelSrc->RightLimit(), m_pLevelSrc->BottomLimit(),
			      m_physicsSettings->BikeWheelBlockGrip());
      m_Collision.defineLine( m_pLevelSrc->RightLimit(), m_pLevelSrc->BottomLimit(),
			      m_pLevelSrc->RightLimit(), m_pLevelSrc->TopLimit(),
			      m_physicsSettings->BikeWheelBlockGrip());
      m_Collision.defineLine( m_pLevelSrc->RightLimit(), m_pLevelSrc->TopLimit(),
			      m_pLevelSrc->LeftLimit(), m_pLevelSrc->TopLimit(),
			      m_physicsSettings->BikeWheelBlockGrip());

      /* Show stats about the collision system */
      CollisionSystemStats CStats;
      m_Collision.getStats(&CStats);
      LogInfo(" %dx%d grid with %.1fx%.1f cells (%.0f%% empty)",
	  CStats.nGridWidth,CStats.nGridHeight,CStats.fCellWidth,CStats.fCellHeight,
	  CStats.fPercentageOfEmptyCells);
      LogInfo(" %d total blocking lines",CStats.nTotalLines);
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

  
  /*===========================================================================
    Update zone specific stuff -- call scripts where needed
    ===========================================================================*/
  void MotoGame::_UpdateZones(void) {
    /* Check player touching for each zone */
    for(unsigned int i=0;i<m_pLevelSrc->Zones().size();i++) {
      Zone *pZone = m_pLevelSrc->Zones()[i];

      for(unsigned int j=0; j<m_players.size(); j++) {
	Biker* v_player = m_players[j];
     
	if(m_players[j]->isDead() == false) {

	  /* Check it against the wheels and the head */
	  if(pZone->doesCircleTouch(v_player->getState()->FrontWheelP, v_player->getState()->Parameters()->WheelRadius()) ||
	     pZone->doesCircleTouch(v_player->getState()->RearWheelP,  v_player->getState()->Parameters()->WheelRadius()) ||
	     pZone->doesCircleTouch(v_player->getState()->HeadP, v_player->getState()->Parameters()->HeadSize())) {       
	    /* In the zone -- did he just enter it? */
	    if(v_player->setTouching(pZone, true) == PlayerBiker::added){
	      createGameEvent(new MGE_PlayerEntersZone(getTime(), pZone, j));
	    }
	  } else {
	    /* Not in the zone... but was he during last update? - i.e. has 
	       he just left it? */      
	    if(v_player->setTouching(pZone, false) == PlayerBiker::removed){
	      createGameEvent(new MGE_PlayerLeavesZone(getTime(), pZone, j));
	    }
	  }
	}
      }
    }
  }

  void MotoGame::_UpdateEntities(void) {
    for(unsigned int j=0; j<m_players.size(); j++) {
      Biker* v_player = m_players[j];

      if(m_players[j]->isDead() == false) {

	Vector2f HeadPos = v_player->getState()->Dir==DD_RIGHT?v_player->getState()->HeadP:v_player->getState()->Head2P;
	
	/* Get biker bounding box */
	AABB BBox;
	float headSize = v_player->getState()->Parameters()->HeadSize();
	float wheelRadius = v_player->getState()->Parameters()->WheelRadius();
	/* in case the body is outside of the aabb */
	float securityMargin = 0.5;
	
	BBox.addPointToAABB2f(HeadPos[0]-headSize-securityMargin,
			      HeadPos[1]-headSize-securityMargin);
	BBox.addPointToAABB2f(v_player->getState()->FrontWheelP[0]-wheelRadius-securityMargin,
			      v_player->getState()->FrontWheelP[1]-wheelRadius-securityMargin);
	BBox.addPointToAABB2f(v_player->getState()->RearWheelP[0]-wheelRadius-securityMargin,
			      v_player->getState()->RearWheelP[1]-wheelRadius-securityMargin);
	
	BBox.addPointToAABB2f(HeadPos[0]+headSize+securityMargin,
			      HeadPos[1]+headSize+securityMargin);
	BBox.addPointToAABB2f(v_player->getState()->FrontWheelP[0]+wheelRadius+securityMargin,
			      v_player->getState()->FrontWheelP[1]+wheelRadius+securityMargin);
	BBox.addPointToAABB2f(v_player->getState()->RearWheelP[0]+wheelRadius+securityMargin,
			      v_player->getState()->RearWheelP[1]+wheelRadius+securityMargin);
	
	std::vector<Entity*> entities = m_Collision.getEntitiesNearPosition(BBox);
	
	/* Do player touch anything? */
	for(unsigned int i=0; i<entities.size(); i++) {
	  /* Test against the biker aabb first */
	  if(true){
	    /* Head? */
	    if(circleTouchCircle2f(entities[i]->DynamicPosition(),
				   entities[i]->Size(),
				   HeadPos,
				   v_player->getState()->Parameters()->HeadSize())) {
	      if(v_player->setTouching(entities[i], true) == PlayerBiker::added){
		createGameEvent(new MGE_PlayerTouchesEntity(getTime(),
							    entities[i]->Id(),
							    true, j));
	      }
	      
	      /* Wheel then ? */
	    } else if(circleTouchCircle2f(entities[i]->DynamicPosition(),
					  entities[i]->Size(),
					  v_player->getState()->FrontWheelP,
					  v_player->getState()->Parameters()->WheelRadius()) ||
		      circleTouchCircle2f(entities[i]->DynamicPosition(),
					  entities[i]->Size(),
					  v_player->getState()->RearWheelP,
					  v_player->getState()->Parameters()->WheelRadius())) {
	      if(v_player->setTouching(entities[i], true) == PlayerBiker::added){
		createGameEvent(new MGE_PlayerTouchesEntity(getTime(),
							    entities[i]->Id(),
							    false, j));
	      }
	      
	      /* body then ?*/
	    } else if(touchEntityBodyExceptHead(*(v_player->getState()), *(entities[i]))) {
	      if(v_player->setTouching(entities[i], true) == PlayerBiker::added){
		createGameEvent(new MGE_PlayerTouchesEntity(getTime(),
							    entities[i]->Id(),
							    false, j));
	      }
	    } else {
	      /* TODO::generate an event "leaves entity" if needed */
	      v_player->setTouching(entities[i], false);
	    }
	  } else {
	    /* TODO::generate an event "leaves entity" if needed */
	    v_player->setTouching(entities[i], false);
	  }
	}
      }
    }
  }
  
  /*===========================================================================
    Entity stuff (public)
    ===========================================================================*/
  void MotoGame::deleteEntity(Entity *pEntity) {
    /* Already scheduled for deletion? */
    for(unsigned int i=0; i<m_DelSchedule.size(); i++)
      if(m_DelSchedule[i] == pEntity) return;
    m_DelSchedule.push_back(pEntity);
  }
  
  void MotoGame::touchEntity(int i_player, Entity *pEntity,bool bHead) {
    /* Start by invoking scripts if any */
    if(m_playEvents) {
      m_luaGame->scriptCallTblVoid(pEntity->Id(), "Touch");
      m_luaGame->scriptCallTblVoid(pEntity->Id(), "TouchBy", i_player);

      if(pEntity->DoesMakeWin()) {
	if(getNbRemainingStrawberries() == 0) {
	  makePlayerWin(i_player);
	}
      }

      if(pEntity->DoesKill()) {
	createGameEvent(new MGE_PlayerDies(getTime(), true, i_player));
      }

      if(pEntity->IsToTake()) {
	/* OH... nice */
	createGameEvent(new MGE_EntityDestroyed(getTime(), pEntity->Id(), pEntity->Speciality(), pEntity->DynamicPosition(), pEntity->Size()));
      }
    }
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

  void MotoGame::handleEvent(MotoGameEvent *pEvent) {     
    switch(pEvent->getType()) {
      case GAME_EVENT_PLAYER_TOUCHES_ENTITY:
      case GAME_EVENT_PLAYERS_TOUCHE_ENTITY:
      /* touching an entity creates events, so, don't call it */
      break;
      default:
      pEvent->doAction(this);
      break;
    }
  }

  void MotoGame::SetEntityPos(std::string pEntityID, float pX, float pY) {
    SetEntityPos(m_pLevelSrc->getEntityById(pEntityID), pX, pY);
  }

  void MotoGame::SetEntityPos(Entity *pEntity, float pX, float pY) {
    pEntity->setDynamicPosition(Vector2f(pX, pY));
    // move it in the collision system only if it's not dead
    if(pEntity->isAlive() == true){
      m_Collision.moveEntity(pEntity);
    }
  }

void MotoGame::translateEntity(std::string pEntityID, float x, float y)
{
  translateEntity(m_pLevelSrc->getEntityById(pEntityID), x, y);
}

void MotoGame::translateEntity(Entity* pEntity, float x, float y)
{
  pEntity->translate(x, y);
  if(pEntity->isAlive() == true){
    m_Collision.moveEntity(pEntity);
  }
}

  void MotoGame::PlaceInGameArrow(float pX, float pY, float pAngle) {
    getArrowPointer().nArrowPointerMode = 1;
    getArrowPointer().ArrowPointerPos = Vector2f(pX, pY);
    getArrowPointer().fArrowPointerAngle = pAngle;
  }

  void MotoGame::PlaceScreenArrow(float pX, float pY, float pAngle) {
    getArrowPointer().nArrowPointerMode = 2;
    getArrowPointer().ArrowPointerPos = Vector2f(pX, pY);
    getArrowPointer().fArrowPointerAngle = pAngle;
  }

  void MotoGame::HideArrow() {
    getArrowPointer().nArrowPointerMode = 0;
  }

  void MotoGame::MoveBlock(std::string pBlockID, float pX, float pY) {
    Block* v_block = m_pLevelSrc->getBlockById(pBlockID);
    MoveBlock(v_block, pX, pY);
  }
  
  void MotoGame::MoveBlock(Block* pBlock, float pX, float pY) {
    pBlock->translate(pX, pY);
    m_Collision.moveDynBlock(pBlock);
  }
  
  void MotoGame::SetBlockPos(std::string pBlockID, float pX, float pY) {
    Block* v_block = m_pLevelSrc->getBlockById(pBlockID);
    v_block->setDynamicPositionAccordingToCenter(Vector2f(pX, pY));
    m_Collision.moveDynBlock(v_block);
  }
  
  void MotoGame::SetPhysicsBlockPos(std::string pBlockID, float pX, float pY) {
    Block* v_block = m_pLevelSrc->getBlockById(pBlockID);
    v_block->setDynamicPositionAccordingToCenter(Vector2f(pX, pY));
    if (v_block->isPhysics()) {
      v_block->setPhysicsPosition(pX, pY);
    } else {
      // report error?
    }
    m_Collision.moveDynBlock(v_block);
  }
  
  void MotoGame::SetBlockCenter(std::string pBlockID, float pX, float pY) {
    Block* v_block = m_pLevelSrc->getBlockById(pBlockID);
    v_block->setCenter(Vector2f(pX, pY));
    m_Collision.moveDynBlock(v_block);
  }
  
  void MotoGame::SetBlockRotation(std::string pBlockID, float pAngle) {
    Block* v_block = m_pLevelSrc->getBlockById(pBlockID);
    SetBlockRotation(v_block, pAngle);
  }     
  
  void MotoGame::SetBlockRotation(Block* pBlock, float pAngle) {
    if(pBlock->setDynamicRotation(pAngle) == true)
      m_Collision.moveDynBlock(pBlock);
  }     
  
  void MotoGame::SetEntityDrawAngle(std::string pEntityID, float pAngle) {
    Entity *v_entity;
    v_entity = getLevelSrc()->getEntityById(pEntityID);
    v_entity->setDrawAngle(pAngle);
  }

  void MotoGame::DisplayDiffFromGhost() {
    if(m_ghosts.size() > 0) {
      float v_diffToGhost;

      /* take the more */
      v_diffToGhost = m_ghosts[0]->diffToPlayer();
      for(unsigned int i=1; i<m_ghosts.size(); i++) {
	if(m_ghosts[i]->diffToPlayer() > v_diffToGhost) {
	  v_diffToGhost = m_ghosts[i]->diffToPlayer();
	}
      }

      char msg[256];
      snprintf(msg, 256, "%+.2f", v_diffToGhost/100.0);
      this->gameMessage(msg,true);
    }
  }
  
  void MotoGame::cleanScriptDynamicObjects() {
    for(unsigned int i=0; i<m_SDynamicObjects.size(); i++) {
      delete m_SDynamicObjects[i];
    }
    m_SDynamicObjects.clear();
  }

  void MotoGame::nextStateScriptDynamicObjects(int i_nbCents) {
    unsigned int i = 0;

    while(i < m_SDynamicObjects.size()) {
      if(m_SDynamicObjects[i]->nextState(this, i_nbCents) == false) {
	delete m_SDynamicObjects[i];
        m_SDynamicObjects.erase(m_SDynamicObjects.begin() + i);
      } else {
	i++;
      }
    }
  }

  void MotoGame::removeSDynamicOfObject(std::string pObject) {
    unsigned int i = 0;

    while(i < m_SDynamicObjects.size()) {
      if(m_SDynamicObjects[i]->getObjectId() == pObject) {
	delete m_SDynamicObjects[i];
        m_SDynamicObjects.erase(m_SDynamicObjects.begin() + i);
      } else {
	i++;
      }
    }
  }

  void MotoGame::CameraZoom(float pZoom) {
    getCamera()->desactiveActionZoom();
    getCamera()->setRelativeZoom(pZoom);
  }
   
  void MotoGame::CameraMove(float p_x, float p_y) {
    getCamera()->desactiveActionZoom();
    getCamera()->moveCamera(p_x, p_y);
  }

  void MotoGame::CameraRotate(float i_angle) {
    getCamera()->setDesiredRotationAngle(i_angle);
  }
   
  void MotoGame::CameraAdaptToGravity() {
    getCamera()->adaptRotationAngleToGravity(m_PhysGravity);
  }

  void MotoGame::killPlayer(int i_player) {
    if(m_players[i_player]->isDead() == false && m_players[i_player]->isFinished() == false) {
      m_players[i_player]->setDead(true, getTime());

      if(m_bDeathAnimEnabled) {
	m_players[i_player]->setBodyDetach(true);
      }
      m_players[i_player]->getControler()->stopContols();

      // inform camera that the player dies (for the following point)
      for(unsigned int i=0; i<m_cameras.size(); i++){
	if(m_cameras[i]->getPlayerToFollow() == m_players[i_player]) {
	  m_cameras[i]->setPlayerDead();
	}
      }
    }
  }

  void MotoGame::addForceToPlayer(int i_player, const Vector2f& i_force, int i_startTime, int i_endTime) {
    m_players[i_player]->addBodyForce(m_time, i_force, i_startTime, i_endTime);
 }

  void MotoGame::playerEntersZone(int i_player, Zone *pZone) {
    if(m_playEvents) {
      m_luaGame->scriptCallTblVoid(pZone->Id(), "OnEnter");
      m_luaGame->scriptCallTblVoid(pZone->Id(), "OnEnterBy", i_player);
    }
  }
  
  void MotoGame::playerLeavesZone(int i_player, Zone *pZone) {
    if(m_playEvents) {
      m_luaGame->scriptCallTblVoid(pZone->Id(), "OnLeave");
      m_luaGame->scriptCallTblVoid(pZone->Id(), "OnLeaveBy", i_player);
    }
  }

  void MotoGame::playerTouchesEntity(int i_player, std::string p_entityID, bool p_bTouchedWithHead) {
    touchEntity(i_player, getLevelSrc()->getEntityById(p_entityID), p_bTouchedWithHead);
  }

  void MotoGame::entityDestroyed(const std::string& i_entityId) {
    Entity *v_entity;
    v_entity = getLevelSrc()->getEntityById(i_entityId);

    if(v_entity->IsToTake()) {
      ParticlesSource *v_stars = new ParticlesSourceStar("");
      v_stars->setInitialPosition(v_entity->DynamicPosition());
      v_stars->loadToPlay();
      v_stars->setZ(1.0);
      for(int i=0; i<3; i++) {
	v_stars->addParticle(m_time);
      }
      getLevelSrc()->spawnEntity(v_stars);

      if(m_motoGameHooks != NULL) {
	m_motoGameHooks->OnTakeEntity();
      }

      /* update timediff */
      m_myLastStrawberries.push_back(getTime());
      
      for(unsigned int i=0; i<m_ghosts.size(); i++) {
	m_ghosts[i]->updateDiffToPlayer(m_myLastStrawberries);
      }
      
      if(m_showGhostTimeDiff) {
	DisplayDiffFromGhost();
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
    v_entity = m_pLevelSrc->getEntityById(p_entityID);
    createGameEvent(new MGE_EntityDestroyed(getTime(),
                                            v_entity->Id(),
                                            v_entity->Speciality(),
                                            v_entity->DynamicPosition(),
                                            v_entity->Size()));
  }

  unsigned int MotoGame::getNbRemainingStrawberries() {
    return m_pLevelSrc->countToTakeEntities();
  }

  void MotoGame::makePlayerWin(int i_player) {
    if(m_players[i_player]->isDead() == false && m_players[i_player]->isFinished() == false) {
      m_players[i_player]->setFinished(true, getTime());
    }
  }

  void MotoGame::addPenalityTime(int i_time) {
    if(i_time > 0) {
      m_time += i_time;
    }
  }

  void MotoGame::_KillEntity(Entity *pEnt) {
    getLevelSrc()->killEntity(pEnt->Id());
    /* now that rendering use the space partionnement,
       we have to remove entity from the collision system */
    m_Collision.removeEntity(pEnt);

    /* special case for the last strawberry : if we are touching the end, finish the level */
    if(getNbRemainingStrawberries() == 0) {
      for(unsigned int j=0; j<m_players.size(); j++) {
	if(m_players[j]->isDead() == false) {
	  for(unsigned int i=0; i<m_players[j]->EntitiesTouching().size(); i++) {
	    if(m_players[j]->EntitiesTouching()[i]->DoesMakeWin()) {
	      makePlayerWin(j);
	    }
	  }
	}
      }
    }
  }


  PlayerBiker* MotoGame::addPlayerBiker(Vector2f i_position, DriveDir i_direction,
					Theme *i_theme, BikerTheme* i_bikerTheme,
					const TColor& i_filterColor,
					const TColor& i_filterUglyColor,
					bool i_enableEngineSound) {
    PlayerBiker* v_playerBiker = new PlayerBiker(m_physicsSettings, i_position, i_direction, m_PhysGravity,
						 i_theme, i_bikerTheme,
						 i_filterColor, i_filterUglyColor);
    v_playerBiker->setOnBikerHooks(new MotoGameOnBikerHooks(this, m_players.size()));
    v_playerBiker->setPlaySound(i_enableEngineSound);
    m_players.push_back(v_playerBiker);

    if(m_chipmunkWorld != NULL) {
      m_chipmunkWorld->addPlayer(v_playerBiker);
    }

    return v_playerBiker;
  }

  void MotoGame::setGravity(float x,float y) {
    m_PhysGravity.x=x;
    m_PhysGravity.y=y;

    for(unsigned int i=0; i<m_players.size(); i++) {
      if(m_players[i]->isDead() == false) {
	m_players[i]->resetAutoDisabler();
      }
    }

    // change gravity for chipmunk
    if(m_chipmunkWorld != NULL) {
      m_chipmunkWorld->setGravity(x, y);
    }

    //m_renderer->adaptRotationAngleToGravity();
  }

  const Vector2f & MotoGame::getGravity() {
    return m_PhysGravity;
  }

  void MotoGame::pause() {
    m_is_paused = ! m_is_paused;
  }

  float MotoGame::getSpeed() const {
    if(m_is_paused) {
      return 0.0;
    }
    return m_speed_factor;
  }

  void MotoGame::slower(float i_increment) {
    if(m_is_paused == false) {
      m_speed_factor -= i_increment;
    }
    if(getLevelSrc() != NULL) {
      if(getLevelSrc()->isScripted()) {
	if(m_speed_factor < 0.0) m_speed_factor = 0.0;
      }
    }
  }

  void MotoGame::faster(float i_increment) {
    if(m_is_paused == false) {
      m_speed_factor += i_increment;
    }
  }

  void MotoGame::fastforward(int i_time) {
    m_time += i_time;
  }

  void MotoGame::fastrewind(int i_time) {
    if(getLevelSrc() != NULL) {
      if(getLevelSrc()->isScripted() == false) {
	m_time -= i_time;
	if(m_time < 0) m_time = 0;
      }
    }
  }

  bool MotoGame::doesPlayEvents() const {
    return m_playEvents;
  }

  void MotoGame::setInfos(const std::string& i_infos) {
    m_infos = i_infos;
  }

  std::string MotoGame::getInfos() const {
    return m_infos;
  }

  LuaLibGame* MotoGame::getLuaLibGame() {
    return m_luaGame;
  }

  Camera* MotoGame::getCamera(){
    return m_cameras[m_currentCamera];
  }
  unsigned int MotoGame::getNumberCameras(){
    // the last camera is the autozoom one
    return m_cameras.size()==1?1:m_cameras.size()-1;
  }
  void MotoGame::setCurrentCamera(unsigned int currentCamera){
    m_currentCamera = currentCamera;
  }
  unsigned int MotoGame::getCurrentCamera(){
    return m_currentCamera;
  }
  void MotoGame::addCamera(Vector2i upperleft, Vector2i downright, bool i_useActiveZoom){
    Camera* i_cam = new Camera(upperleft, downright);
    i_cam->allowActiveZoom(i_useActiveZoom);
    m_cameras.push_back(i_cam);
    m_cameras.back()->initCamera();
  }
  void MotoGame::resetFollow(){
    for(unsigned int i=0; i<m_cameras.size(); i++){
      m_cameras[i]->setPlayerToFollow(NULL);
    }
  }
  void MotoGame::removeCameras(){
    for(unsigned int i=0; i<m_cameras.size(); i++){
      delete m_cameras[i];
    }
    m_cameras.clear();
  }
  void MotoGame::setAutoZoomCamera(){
    if(m_cameras.size() == 1){
      setCurrentCamera(0);
    }
    else{
      // the last camera is the autozoom one
      setCurrentCamera(m_cameras.size()-1);
    }
  }

bool MotoGame::isAutoZoomCamera(){
  return (getCurrentCamera() == getNumberCameras());
}

  std::vector<Camera*>& MotoGame::Cameras() {
    return m_cameras;
  }

PhysicsSettings* MotoGame::getPhysicsSettings() {
  return m_physicsSettings;
}

MotoGameOnBikerHooks::MotoGameOnBikerHooks(MotoGame* i_motoGame, int i_playerNumber) {
  m_motoGame = i_motoGame;
  m_playerNumber = i_playerNumber;
}

MotoGameOnBikerHooks::~MotoGameOnBikerHooks() {
}

void MotoGameOnBikerHooks::onSomersaultDone(bool i_counterclock) {
  if(m_motoGame->doesPlayEvents() == false) return;
  m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg("OnSomersault", i_counterclock ? 1:0);
  m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg("OnSomersaultBy",
						       i_counterclock ? 1:0, m_playerNumber);
}

void MotoGameOnBikerHooks::onWheelTouches(int i_wheel, bool i_touch) {
  if(m_motoGame->doesPlayEvents() == false) return;

  if(i_wheel == 1) {
    if(i_touch) {
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg("OnWheel1Touchs"  , 1);
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg("OnWheel1TouchsBy", 1, m_playerNumber);
    } else {
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg("OnWheel1Touchs"  , 0);
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg("OnWheel1TouchsBy", 0, m_playerNumber);
    }
  } else {
    if(i_touch) {
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg("OnWheel2Touchs"  , 1);
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg("OnWheel2TouchsBy", 1, m_playerNumber);
    } else {
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg("OnWheel2Touchs"  , 0);
      m_motoGame->getLuaLibGame()->scriptCallVoidNumberArg("OnWheel2TouchsBy", 0, m_playerNumber);
    }
  }
}

void MotoGameOnBikerHooks::onHeadTouches() {
  m_motoGame->createGameEvent(new MGE_PlayerDies(m_motoGame->getTime(), false, m_playerNumber));
}

