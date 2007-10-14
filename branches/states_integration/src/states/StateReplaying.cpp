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

#include "StateReplaying.h"
#include "Game.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"
#include "xmscene/Camera.h"
#include "xmscene/BikePlayer.h"
#include "xmscene/Entity.h"
#include "StateMessageBox.h"
#include "helpers/Log.h"
#include "XMSession.h"
#include "PhysSettings.h"

StateReplaying::StateReplaying(GameApp* pGame,
			       const std::string& i_replay,
			       bool drawStateBehind,
			       bool updateStatesBehind
			       ) :
  GameState(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{
  m_replay         = i_replay;
  m_stopToUpdate   = false;
  m_fLastPhysTime = GameApp::getXMTime() - PHYS_STEP_SIZE;
}

StateReplaying::~StateReplaying()
{

}


void StateReplaying::enter()
{
  char **v_result;
  unsigned int nrow;
  char *v_res;

  m_pGame->m_State = GS_REPLAYING; // to be removed, just the time states are finished
  m_pGame->setShowCursor(false);

  m_stopToUpdate = false;
  m_pGame->getGameRenderer()->setShowEngineCounter(false);
  m_replayBiker = NULL;
  
  try {  
    m_pGame->initCameras(1); // init camera for only one player
    
    try {
      m_replayBiker = m_pGame->getMotoGame()->addReplayFromFile(m_replay,
								m_pGame->getTheme(), m_pGame->getTheme()->getPlayerTheme(),
								m_pGame->getSession()->enableEngineSound());
      m_pGame->getMotoGame()->getCamera()->setPlayerToFollow(m_replayBiker);
    } catch(Exception &e) {
      m_requestForEnd = true;
      m_pGame->getStateManager()->pushState(new StateMessageBox(m_pGame, "Unable to read the replay: " + e.getMsg(), UI_MSGBOX_OK));
      return;
    }
    
    /* Fine, open the level */
    try {
      m_pGame->getMotoGame()->loadLevel(m_pGame->getDb(), m_replayBiker->levelId());
    } catch(Exception &e) {
      m_requestForEnd = true;
      m_pGame->getStateManager()->pushState(new StateMessageBox(m_pGame, e.getMsg(), UI_MSGBOX_OK));
      return;
    }
    
    if(m_pGame->getMotoGame()->getLevelSrc()->isXMotoTooOld()) {
      Logger::Log("** Warning ** : level '%s' specified by replay '%s' requires newer X-Moto",
		  m_replayBiker->levelId().c_str(),
		  m_replay.c_str()
		  );

      char cBuf[256];
      sprintf(cBuf,GAMETEXT_NEWERXMOTOREQUIRED,
	      m_pGame->getMotoGame()->getLevelSrc()->getRequiredVersion().c_str());
      m_pGame->getMotoGame()->endLevel();

      m_requestForEnd = true;
      m_pGame->getStateManager()->pushState(new StateMessageBox(m_pGame, cBuf, UI_MSGBOX_OK));  
      return;
    }
    
    /* Init level */    
    m_pGame->getInputHandler()->reset();
    m_pGame->getMotoGame()->prePlayLevel(m_pGame->getInputHandler(), NULL, false);
    
    /* add the ghosts */
    if(m_pGame->getSession()->enableGhosts()) {
      try {
	m_pGame->addGhosts(m_pGame->getMotoGame(), m_pGame->getTheme());
      } catch(Exception &e) {
	/* anyway */
      }
    }
    
    /* *** */
    
    char c_tmp[1024];
    snprintf(c_tmp, 1024,
	     GAMETEXT_BY_PLAYER,
	     m_replayBiker->playerName().c_str()
	     );
    m_pGame->getMotoGame()->setInfos(m_pGame->getMotoGame()->getLevelSrc()->Name() + " " + std::string(c_tmp));
    
    m_pGame->getGameRenderer()->prepareForNewLevel(false);
    m_pGame->playMusic(m_pGame->getMotoGame()->getLevelSrc()->Music());
    
    /* Show help string */
    std::string T1 = "--:--:--",T2 = "--:--:--";
      
    /* get best result */
    v_result = m_pGame->getDb()->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
			    "id_level=\"" + 
			    xmDatabase::protectString(m_pGame->getMotoGame()->getLevelSrc()->Id()) + "\";",
			    nrow);
    v_res = m_pGame->getDb()->getResult(v_result, 1, 0, 0);
    if(v_res != NULL) {
      T1 = GameApp::formatTime(atof(v_res));
    }
    m_pGame->getDb()->read_DB_free(v_result);
    
    /* get best player result */
    v_result = m_pGame->getDb()->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
			    "id_level=\"" + 
			    xmDatabase::protectString(m_pGame->getMotoGame()->getLevelSrc()->Id()) + "\" " + 
			    "AND id_profile=\"" + xmDatabase::protectString(m_pGame->getSession()->profile())  + "\";",
			    nrow);
    v_res = m_pGame->getDb()->getResult(v_result, 1, 0, 0);
    if(v_res != NULL) {
      T2 = GameApp::formatTime(atof(v_res));
    }
    m_pGame->getDb()->read_DB_free(v_result);
    
    m_pGame->getGameRenderer()->setBestTime(T1 + std::string(" / ") + T2);
    m_pGame->getGameRenderer()->showReplayHelp(m_pGame->getMotoGame()->getSpeed(),
					       m_pGame->getMotoGame()->getLevelSrc()->isScripted() == false);
    
    /* World-record stuff */
    m_pGame->getGameRenderer()->setWorldRecordTime(m_pGame->getWorldRecord(m_pGame->getMotoGame()->getLevelSrc()->Id()));
    
  } catch(Exception &e) {
    m_pGame->getMotoGame()->endLevel();
    m_requestForEnd = true;
    m_pGame->getStateManager()->pushState(new StateMessageBox(m_pGame, GameApp::splitText(e.getMsg(), 50), UI_MSGBOX_OK));  
  }

  /* Context menu? */
  m_pGame->getGameRenderer()->getGUI()->enableContextMenuDrawing(false); // to remove after states

}

void StateReplaying::leave()
{
  m_pGame->getMotoGame()->setInfos("");
}

void StateReplaying::enterAfterPop()
{

}

void StateReplaying::leaveAfterPush()
{

}

void StateReplaying::update()
{
  int nPhysSteps = 0;

  if(m_stopToUpdate) {
    return;
  }

  /* Following time code is made by Eric Piel, but I took the liberty to change the minimum
     frame-miss number from 50 to 10, because it wasn't working well. */
  
  /* reinitialise if we can't catch up */
  if (m_fLastPhysTime - GameApp::getXMTime() < -0.1f) {
    m_fLastPhysTime = GameApp::getXMTime() - PHYS_STEP_SIZE;
  }
  
  /* Update game until we've catched up with the real time */
  
  do {
    m_pGame->getMotoGame()->updateLevel(PHYS_STEP_SIZE);
    m_fLastPhysTime += PHYS_STEP_SIZE;
    nPhysSteps++;
    
    /* don't do this infinitely, maximum miss 10 frames, then give up */
  } while ((m_fLastPhysTime + PHYS_STEP_SIZE <= GameApp::getXMTime()) && (nPhysSteps < 10));
  
  m_pGame->getMotoGame()->setCurrentCamera(0);
  m_pGame->getMotoGame()->getCamera()->setSpeedMultiplier(nPhysSteps);
  
  if(m_pGame->getSession()->timedemo() == false) {
    /* Never pass this point while being ahead of time, busy wait until it's time */
    if(nPhysSteps <= 1) {  
      while (m_fLastPhysTime > GameApp::getXMTime());
    }
  }
  
  if(m_replayBiker->isDead() || m_replayBiker->isFinished()) {
    m_stopToUpdate = true;
    
    if(m_replayBiker->isFinished()) {
      m_pGame->getMotoGame()->setTime(m_replayBiker->finishTime());
    }
  }
}

void StateReplaying::render()
{
//    int nPhysSteps = 0;
//        
//    /* When did the frame start? */
//    double fStartFrameTime = GameApp::getXMTime();                    

  //    getDrawLib()->getMenuCamera()->setCamera2d(); ??

  try {
    m_pGame->getMotoGame()->setCurrentCamera(0);
    m_pGame->getGameRenderer()->render();
    ParticlesSource::setAllowParticleGeneration(m_pGame->getGameRenderer()->nbParticlesRendered() < NB_PARTICLES_TO_RENDER_LIMITATION);
  } catch(Exception &e) {
    m_pGame->getStateManager()->pushState(new StateMessageBox(m_pGame, GameApp::splitText(e.getMsg(), 50), UI_MSGBOX_OK));
    //m_pGame->closePlaying();
    //m_pGame->setState(GS_MENU); // to be removed, just the time states are finished
    //m_requestForEnd = true;
  }

//    /* When did frame rendering end? */
//    double fEndFrameTime = GameApp::getXMTime();
//          
//    /* Calculate how large a delay should be inserted after the frame, to keep the 
//       desired frame rate */
//    int nADelay = 0;    
//          
//	  /* become idle only if we hadn't to skip any frame, recently, and more globaly (80% of fps) */
//    if((nPhysSteps <= 1) && (m_fFPS_Rate > (0.8f / PHYS_STEP_SIZE)))
//      nADelay = ((m_fLastPhysTime + PHYS_STEP_SIZE) - fEndFrameTime) * 1000.0f;
//    
//    if(nADelay > 0) {
//      if(m_xmsession->timedemo() == false) {
//	setFrameDelay(nADelay);
//      }
//    }  
}

void StateReplaying::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {
    
  case SDLK_ESCAPE:
    m_pGame->closePlaying();
    m_pGame->setState(GS_MENU); // to be removed, just the time states are finished
    m_requestForEnd = true;
    break;          

  case SDLK_RIGHT:
    /* Right arrow key: fast forward */
    if(m_stopToUpdate == false) {
      m_pGame->getMotoGame()->fastforward(1);
    }
    break;

  case SDLK_LEFT:
    if(m_pGame->getMotoGame()->getLevelSrc()->isScripted() == false) {
      m_pGame->getMotoGame()->fastrewind(1);
      m_stopToUpdate = false;
    }
    break;

  case SDLK_F2:
    m_pGame->switchFollowCamera();
    break;
    
  case SDLK_F3:
    m_pGame->switchLevelToFavorite(m_pGame->getMotoGame()->getLevelSrc()->Id(), true);
    break;
    
  case SDLK_SPACE:
    /* pause */
    m_pGame->getMotoGame()->pause();
    m_pGame->getGameRenderer()->showReplayHelp(m_pGame->getMotoGame()->getSpeed(), m_pGame->getMotoGame()->getLevelSrc()->isScripted() == false);
    break;

  case SDLK_UP:
    /* faster */
    m_pGame->getMotoGame()->faster();
    m_pGame->getGameRenderer()->showReplayHelp(m_pGame->getMotoGame()->getSpeed(), m_pGame->getMotoGame()->getLevelSrc()->isScripted() == false);
    break;

  case SDLK_DOWN:
    /* slower */
    m_pGame->getMotoGame()->slower();
    m_stopToUpdate = false;
    m_pGame->getGameRenderer()->showReplayHelp(m_pGame->getMotoGame()->getSpeed(), m_pGame->getMotoGame()->getLevelSrc()->isScripted() == false);
    break;

  }
}

void StateReplaying::keyUp(int nKey,   SDLMod mod)
{

}

void StateReplaying::mouseDown(int nButton)
{

}

void StateReplaying::mouseDoubleClick(int nButton)
{

}

void StateReplaying::mouseUp(int nButton)
{

}
