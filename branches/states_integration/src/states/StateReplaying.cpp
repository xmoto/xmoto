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

StateReplaying::StateReplaying(GameApp* pGame,
			       const std::string& i_replay
			       ) :
  StateScene(pGame)
{
  m_replay         = i_replay;
  m_stopToUpdate   = false;
  m_updateFps = 100;
  m_renderFps = 50;
}

StateReplaying::~StateReplaying()
{
}


void StateReplaying::enter()
{
  StateScene::enter();

  char **v_result;
  unsigned int nrow;
  char *v_res;

  m_pGame->m_State = GS_REPLAYING; // to be removed, just the time states are finished

  m_stopToUpdate = false;
  m_pGame->getGameRenderer()->setShowEngineCounter(false);
  m_pGame->getGameRenderer()->setShowTimePanel(true);
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
    
    m_pGame->getGameRenderer()->prepareForNewLevel();
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

bool StateReplaying::update()
{
  if(StateScene::update() == false){
    return false;
  }
  
  if(m_replayBiker->isDead() || m_replayBiker->isFinished()) {
    m_stopToUpdate = true;
    
    if(m_replayBiker->isFinished()) {
      m_pGame->getMotoGame()->setTime(m_replayBiker->finishTime());
    }
  }

  return true;
}

bool StateReplaying::render()
{
  for(unsigned int i=0; i<m_pGame->getMotoGame()->Cameras().size(); i++) {
    m_pGame->getMotoGame()->Cameras()[i]->setSpeedMultiplier(10.0); // tell to the camera the nPhysSteps
  }  
  return StateScene::render();
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

  default:
    StateScene::keyDown(nKey, mod, nChar);
  }
}

void StateReplaying::keyUp(int nKey,   SDLMod mod)
{
  StateScene::keyUp(nKey, mod);
}

void StateReplaying::mouseDown(int nButton)
{
  StateScene::mouseDown(nButton);
}

void StateReplaying::mouseDoubleClick(int nButton)
{
  StateScene::mouseDoubleClick(nButton);
}

void StateReplaying::mouseUp(int nButton)
{
  StateScene::mouseUp(nButton);
}
