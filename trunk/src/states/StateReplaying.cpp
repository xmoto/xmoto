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
#include "StatePreplayingReplay.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"
#include "xmscene/BikePlayer.h"
#include "helpers/Log.h"
#include "helpers/Text.h"
#include "XMSession.h"
#include "xmscene/Camera.h"
#include "StateMessageBox.h"
#include "Universe.h"
#include "Trainer.h"
#include "Game.h"
#include "SysMessage.h"
#include "VideoRecorder.h"

StateReplaying::StateReplaying(Universe* i_universe, const std::string& i_replay, ReplayBiker* i_replayBiker) :
  StateScene()
{
  m_name         = "StateReplaying";
  m_universe     = i_universe;
  m_replay       = i_replay;
  m_replayBiker  = i_replayBiker;
  m_stopToUpdate = false;
  m_updateFps = 100;
  m_renderFps = 50;
}

StateReplaying::~StateReplaying()
{
  // don't clean the replay biker while the scene clean its players
  // delete m_replayBiker;
}

void StateReplaying::enter()
{
  StateScene::enter();

  m_stopToUpdate = false;
  GameRenderer::instance()->setShowEngineCounter(false);

  if(XMSession::instance()->hidePlayingInformation() == false) {
    GameRenderer::instance()->setShowMinimap(XMSession::instance()->showMinimap());
    GameRenderer::instance()->setShowTimePanel(true);
  } else {
    GameRenderer::instance()->setShowMinimap(false);
    GameRenderer::instance()->setShowTimePanel(false);
  }

  try {
    if(XMSession::instance()->hidePlayingInformation() == false) {
      // display replay informations
      char c_tmp[1024];
      snprintf(c_tmp, 1024,
	       GAMETEXT_BY_PLAYER,
	       m_replayBiker->playerName().c_str());
      
      for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	m_universe->getScenes()[i]->setInfos(m_universe->getScenes()[i]->getLevelSrc()->Name() + " " + std::string(c_tmp));
      }
    }

    if(m_universe->getScenes().size() > 0 && XMSession::instance()->hidePlayingInformation() == false) {
      GameRenderer::instance()->showReplayHelp(m_universe->getScenes()[0]->getSpeed(),
					       m_universe->getScenes()[0]->getLevelSrc()->isScripted() == false);
    }

    // music
    if(m_universe->getScenes().size() > 0) { 	 
      // play music of the first world 	 
      GameApp::instance()->playMusic(m_universe->getScenes()[0]->getLevelSrc()->Music()); 	 
    }

    // highscores
    setScoresTimes();    
  } catch(Exception &e) {
    abortPlaying();
    StateManager::instance()->replaceState(new StateMessageBox(this, splitText(e.getMsg(), 50), UI_MSGBOX_OK));
    return;
  }
}

void StateReplaying::leave()
{
  if(m_universe != NULL) {
    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->setInfos("");
    }
  }
}

bool StateReplaying::update()
{
  if(StateScene::update() == false)
    return false;
  
  if(m_replayBiker->isDead() || m_replayBiker->isFinished() && m_stopToUpdate == false) {
    m_stopToUpdate = true;

    if(XMSession::instance()->benchmark()) {
      m_requestForEnd = true;
      closePlaying();
      printf(" * %i frames rendered in %.2f seconds\n", m_benchmarkNbFrame, GameApp::getXMTime() - m_benchmarkStartTime);
      printf(" * Average framerate: %.2f fps\n", ((double)m_benchmarkNbFrame) / (GameApp::getXMTime() - m_benchmarkStartTime));
    }
  }

  return true;
}

void StateReplaying::keyDown(SDLKey nKey, SDLMod mod,int nChar, const std::string& i_utf8Char)
{
  switch(nKey) {
    
  case SDLK_ESCAPE:
    m_requestForEnd = true;
    closePlaying();
    break;          

  case SDLK_RIGHT:
    /* Right arrow key: fast forward */
    if(m_stopToUpdate == false) {
      if(m_universe != NULL) {
	for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	  m_universe->getScenes()[i]->fastforward(100);
	}
      }
    }
    break;

  case SDLK_LEFT:
    if(m_universe != NULL) {
      if(m_universe->getScenes().size() > 0) {
	if(m_universe->getScenes()[0]->getLevelSrc()->isScripted() == false) {
	  for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	    m_universe->getScenes()[i]->fastrewind(100);
	  }
	  m_stopToUpdate = false;
	} else {
	  // rerun the replay
	  restartLevel();
	}
      }
    }
    break;

  case SDLK_SPACE:
    /* pause */
    if(m_universe != NULL) {
      for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	m_universe->getScenes()[i]->pause();
      }

      if(m_universe->getScenes().size() > 0 && XMSession::instance()->hidePlayingInformation() == false) {
	GameRenderer::instance()->showReplayHelp(m_universe->getScenes()[0]->getSpeed(),
						 m_universe->getScenes()[0]->getLevelSrc()->isScripted() == false);
      }
    }
    break;

  case SDLK_UP:
    /* faster */
    if(m_universe != NULL) {
      for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	if((mod & KMOD_CTRL) == 0) {
	  m_universe->getScenes()[i]->faster();
	} else {
	  m_universe->getScenes()[i]->faster(0.01);
	}
      }
      if(m_universe->getScenes().size() > 0 && XMSession::instance()->hidePlayingInformation() == false) {
	GameRenderer::instance()->showReplayHelp(m_universe->getScenes()[0]->getSpeed(),
						 m_universe->getScenes()[0]->getLevelSrc()->isScripted() == false);
      }
    }
    break;

  case SDLK_DOWN:
    /* slower */
    if(m_universe != NULL) {
      for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	if((mod & KMOD_CTRL) == 0) {
	  m_universe->getScenes()[i]->slower();
	} else {
	  m_universe->getScenes()[i]->slower(0.01);
	}
      }
      m_stopToUpdate = false;
      if(m_universe->getScenes().size() > 0 && XMSession::instance()->hidePlayingInformation() == false) {
	GameRenderer::instance()->showReplayHelp(m_universe->getScenes()[0]->getSpeed(),
						 m_universe->getScenes()[0]->getLevelSrc()->isScripted() == false);
      }
    }
    break;

#if defined(ENABLE_DEV)
  case SDLK_KP0:			        	//TRAINER
    /* store current bike position (for trainer) */
    if(m_universe != NULL) {
      for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
        Trainer::instance()->storePosition( m_universe->getScenes()[i]->getLevelSrc()->Id(),
                                            m_universe->getScenes()[i]->getPlayerPosition(0) );
        //TODO: bool getPlayerFaceDir (int i_player)
        char sysmsg[256];
        snprintf(sysmsg, 256, SYS_MSG_TRAIN_STORED, Trainer::instance()->getMaxRestoreIndex()+1);
        SysMessage::instance()->displayText(sysmsg);
      }
    }
    break;
#endif


  default:
    StateScene::keyDown(nKey, mod, nChar, i_utf8Char);
  }
}

void StateReplaying::restartLevel(bool i_reloadLevel) {
  closePlaying();
  GameRenderer::instance()->unprepareForNewLevel();
  StateManager::instance()->replaceState(new StatePreplayingReplay(m_replay, true));
}
