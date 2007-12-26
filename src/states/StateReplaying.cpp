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
  MotoGame* pWorld = pGame->getMotoGame();

  m_stopToUpdate = false;
  GameRenderer::instance()->setShowEngineCounter(false);
  GameRenderer::instance()->setShowTimePanel(true);
  m_replayBiker = NULL;

  try {  
    pGame->initCameras(1); // init camera for only one player

    try {
      m_replayBiker = pWorld->addReplayFromFile(m_replay,
						Theme::instance(),
						Theme::instance()->getPlayerTheme(),
						XMSession::instance()->enableEngineSound());
      pWorld->getCamera()->setPlayerToFollow(m_replayBiker);
      pWorld->getCamera()->setScroll(false, pWorld->getGravity());
    } catch(Exception &e) {
      StateManager::instance()->replaceState(new StateMessageBox(NULL, "Unable to read the replay: " + e.getMsg(), UI_MSGBOX_OK));
      return;
    }

    /* Fine, open the level */
    try {
      pWorld->loadLevel(xmDatabase::instance("main"), m_replayBiker->levelId());
    } catch(Exception &e) {
      StateManager::instance()->replaceState(new StateMessageBox(this, e.getMsg(), UI_MSGBOX_OK));
      return;
    }
    
    if(pWorld->getLevelSrc()->isXMotoTooOld()) {
      Logger::Log("** Warning ** : level '%s' specified by replay '%s' requires newer X-Moto",
		  m_replayBiker->levelId().c_str(),
		  m_replay.c_str()
		  );

      char cBuf[256];
      sprintf(cBuf,GAMETEXT_NEWERXMOTOREQUIRED,
	      pWorld->getLevelSrc()->getRequiredVersion().c_str());
      abortPlaying();
      StateManager::instance()->replaceState(new StateMessageBox(this, cBuf, UI_MSGBOX_OK));
      return;
    }
    
    /* Init level */    
    InputHandler::instance()->reset();
    pWorld->prePlayLevel(NULL, false);
    
    /* add the ghosts */
    if(XMSession::instance()->enableGhosts()) {
      try {
	pGame->addGhosts(pWorld, Theme::instance());
      } catch(Exception &e) {
	/* anyway */
      }
    }
    
    /* *** */
    
    char c_tmp[1024];
    snprintf(c_tmp, 1024,
	     GAMETEXT_BY_PLAYER,
	     m_replayBiker->playerName().c_str());
    pWorld->setInfos(pWorld->getLevelSrc()->Name() + " " + std::string(c_tmp));
    
    GameRenderer::instance()->prepareForNewLevel(pWorld);
    pGame->playMusic(pWorld->getLevelSrc()->Music());

    GameRenderer::instance()->showReplayHelp(pWorld->getSpeed(),
					     pWorld->getLevelSrc()->isScripted() == false);
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
  GameApp::instance()->getMotoGame()->setInfos("");
}

bool StateReplaying::update()
{
  if(StateScene::update() == false)
    return false;
  
  if(m_replayBiker->isDead() || m_replayBiker->isFinished()) {
    m_stopToUpdate = true;
    
    if(m_replayBiker->isFinished()) {
      GameApp::instance()->getMotoGame()->setTime(m_replayBiker->finishTime());
    }
  }

  return true;
}

void StateReplaying::keyDown(int nKey, SDLMod mod,int nChar)
{
  GameApp*  pGame  = GameApp::instance();
  MotoGame* pWorld = pGame->getMotoGame();

  switch(nKey) {
    
  case SDLK_ESCAPE:
    m_requestForEnd = true;
    closePlaying();
    break;          

  case SDLK_RIGHT:
    /* Right arrow key: fast forward */
    if(m_stopToUpdate == false) {
      pWorld->fastforward(1);
    }
    break;

  case SDLK_LEFT:
    if(pWorld->getLevelSrc()->isScripted() == false) {
      pWorld->fastrewind(1);
      m_stopToUpdate = false;
    } else {
      // rerun the replay
      closePlaying();
      StateManager::instance()->replaceState(new StateReplaying(m_replay));
    }
    break;

  case SDLK_F2:
    pGame->switchFollowCamera();
    break;
    
  case SDLK_F3:
    pGame->switchLevelToFavorite(pWorld->getLevelSrc()->Id(), true);
    break;
    
  case SDLK_SPACE:
    /* pause */
    pWorld->pause();
    GameRenderer::instance()->showReplayHelp(pWorld->getSpeed(),
					     pWorld->getLevelSrc()->isScripted() == false);
    break;

  case SDLK_UP:
    /* faster */
    pWorld->faster();
    GameRenderer::instance()->showReplayHelp(pWorld->getSpeed(),
					     pWorld->getLevelSrc()->isScripted() == false);
    break;

  case SDLK_DOWN:
    /* slower */
    pWorld->slower();
    m_stopToUpdate = false;
    GameRenderer::instance()->showReplayHelp(pWorld->getSpeed(),
					     pWorld->getLevelSrc()->isScripted() == false);
    break;

  default:
    StateScene::keyDown(nKey, mod, nChar);
  }
}

