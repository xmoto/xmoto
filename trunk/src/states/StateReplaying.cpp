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
#include "xmscene/BikePlayer.h"
#include "helpers/Log.h"
#include "XMSession.h"
#include "xmscene/Camera.h"
#include "StateMessageBox.h"
#include "Universe.h"

StateReplaying::StateReplaying(const std::string& i_replay) :
  StateScene()
{
  m_replay       = i_replay;
  m_stopToUpdate = false;
  m_updateFps = 100;
  m_renderFps = 50;
  m_name      = "StateReplaying";
}

StateReplaying::~StateReplaying()
{
}

void StateReplaying::enter()
{
  StateScene::enter();

  GameApp*  pGame  = GameApp::instance();

  m_stopToUpdate = false;
  GameRenderer::instance()->setShowEngineCounter(false);
  GameRenderer::instance()->setShowTimePanel(true);
  m_replayBiker = NULL;

  m_universe =  new Universe();

  try {
    m_universe->initPlay(1, false);

    try {
      if(m_universe->getScenes().size() > 0) {
	m_replayBiker = m_universe->getScenes()[0]->addReplayFromFile(m_replay,
								      Theme::instance(),
								      Theme::instance()->getPlayerTheme(),
								      XMSession::instance()->enableEngineSound());
	m_universe->getScenes()[0]->getCamera()->setPlayerToFollow(m_replayBiker);
	m_universe->getScenes()[0]->getCamera()->setScroll(false, m_universe->getScenes()[0]->getGravity());
      }
    } catch(Exception &e) {
      delete m_universe;
      StateManager::instance()->replaceState(new StateMessageBox(NULL, "Unable to read the replay: " + e.getMsg(), UI_MSGBOX_OK));
      return;
    }

    /* Fine, open the level */
    try {
      if(m_replayBiker != NULL) {
	for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	  m_universe->getScenes()[i]->loadLevel(xmDatabase::instance("main"), m_replayBiker->levelId());
	}
      }
    } catch(Exception &e) {
      delete m_universe;
      StateManager::instance()->replaceState(new StateMessageBox(this, e.getMsg(), UI_MSGBOX_OK));
      return;
    }
    
    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      if(m_universe->getScenes()[i]->getLevelSrc()->isXMotoTooOld()) {
	Logger::Log("** Warning ** : level '%s' specified by replay '%s' requires newer X-Moto",
		    m_replayBiker->levelId().c_str(),
		    m_replay.c_str()
		    );
	
	char cBuf[256];
	sprintf(cBuf,GAMETEXT_NEWERXMOTOREQUIRED,
		m_universe->getScenes()[i]->getLevelSrc()->getRequiredVersion().c_str());
	abortPlaying();
	StateManager::instance()->replaceState(new StateMessageBox(this, cBuf, UI_MSGBOX_OK));
	return;
      }
    }
    
    /* Init level */    
    InputHandler::instance()->reset();

    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->prePlayLevel(NULL, false);
    }

    /* add the ghosts */
    if(XMSession::instance()->enableGhosts()) {
      try {
	for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	  pGame->addGhosts(m_universe->getScenes()[i], Theme::instance());
	}
      } catch(Exception &e) {
	/* anyway */
      }
    }
    
    /* *** */
    
    char c_tmp[1024];
    snprintf(c_tmp, 1024,
	     GAMETEXT_BY_PLAYER,
	     m_replayBiker->playerName().c_str());

    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->setInfos(m_universe->getScenes()[i]->getLevelSrc()->Name() + " " + std::string(c_tmp));
    }

    GameRenderer::instance()->prepareForNewLevel(m_universe);

    if(m_universe->getScenes().size() > 0) {
      // play music of the first world
      GameApp::instance()->playMusic(m_universe->getScenes()[0]->getLevelSrc()->Music());
    }

    if(m_universe->getScenes().size() > 0) {
      GameRenderer::instance()->showReplayHelp(m_universe->getScenes()[0]->getSpeed(),
					       m_universe->getScenes()[0]->getLevelSrc()->isScripted() == false);
    }

    // highscores
    setScoresTimes();    
  } catch(Exception &e) {
    abortPlaying();
    StateManager::instance()->replaceState(new StateMessageBox(this, GameApp::splitText(e.getMsg(), 50), UI_MSGBOX_OK));
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
  
  if(m_replayBiker->isDead() || m_replayBiker->isFinished()) {
    m_stopToUpdate = true;
    
    if(m_replayBiker->isFinished()) {
      if(m_universe != NULL) {
	for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	  m_universe->getScenes()[i]->setTime(m_replayBiker->finishTime());
	}
      }
    }
  }

  return true;
}

void StateReplaying::keyDown(int nKey, SDLMod mod,int nChar)
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
	  m_universe->getScenes()[i]->fastforward(1);
	}
      }
    }
    break;

  case SDLK_LEFT:
    if(m_universe != NULL) {
      if(m_universe->getScenes().size() > 0) {
	if(m_universe->getScenes()[0]->getLevelSrc()->isScripted() == false) {
	  for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	    m_universe->getScenes()[i]->fastrewind(1);
	  }
	  m_stopToUpdate = false;
	} else {
	  // rerun the replay
	  closePlaying();
	  StateManager::instance()->replaceState(new StateReplaying(m_replay));
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

      if(m_universe->getScenes().size() > 0) {
	GameRenderer::instance()->showReplayHelp(m_universe->getScenes()[0]->getSpeed(),
						 m_universe->getScenes()[0]->getLevelSrc()->isScripted() == false);
      }
    }
    break;

  case SDLK_UP:
    /* faster */
    if(m_universe != NULL) {
      for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	m_universe->getScenes()[i]->faster();
      }
      if(m_universe->getScenes().size() > 0) {
	GameRenderer::instance()->showReplayHelp(m_universe->getScenes()[0]->getSpeed(),
						 m_universe->getScenes()[0]->getLevelSrc()->isScripted() == false);
      }
    }
    break;

  case SDLK_DOWN:
    /* slower */
    if(m_universe != NULL) {
      for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	m_universe->getScenes()[i]->slower();
      }
      m_stopToUpdate = false;
      if(m_universe->getScenes().size() > 0) {
	GameRenderer::instance()->showReplayHelp(m_universe->getScenes()[0]->getSpeed(),
						 m_universe->getScenes()[0]->getLevelSrc()->isScripted() == false);
      }
    }
    break;

  default:
    StateScene::keyDown(nKey, mod, nChar);
  }
}

