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

#include "StatePlaying.h"
#include "StatePause.h"
#include "Game.h"
#include "XMSession.h"
#include "helpers/Log.h"
#include "GameText.h"
#include "StateMessageBox.h"

StatePlaying::StatePlaying(GameApp* pGame):
  StateScene(pGame)
{

}

StatePlaying::~StatePlaying()
{

}


void StatePlaying::enter()
{
  StateScene::enter();

  m_pGame->m_State = GS_PLAYING; // to be removed, just the time states are finished

  m_pGame->getGameRenderer()->setShowEngineCounter(m_pGame->getSession()->showEngineCounter());
  m_pGame->getGameRenderer()->setShowMinimap(m_pGame->getSession()->showMinimap());
  m_pGame->getGameRenderer()->setShowTimePanel(true);

  m_bAutoZoomInitialized = false;

  try {
    m_pGame->getMotoGame()->playLevel();
    m_pGame->playMusic(m_pGame->getMotoGame()->getLevelSrc()->Music());

  } catch(Exception &e) {
    Logger::Log("** Warning ** : level '%s' cannot be loaded", m_pGame->getMotoGame()->getLevelSrc()->Name().c_str());
    m_pGame->getMotoGame()->endLevel();

    char cBuf[256];
    sprintf(cBuf,GAMETEXT_LEVELCANNOTBELOADED, m_pGame->getMotoGame()->getLevelSrc()->Name().c_str());
    m_pGame->getStateManager()->pushState(new StateMessageBox(this, m_pGame, cBuf, UI_MSGBOX_OK));
  }
}

void StatePlaying::leave()
{
  m_pGame->getMotoGame()->setInfos("");
}

void StatePlaying::enterAfterPop()
{

}

void StatePlaying::leaveAfterPush()
{

}

bool StatePlaying::update()
{
  if(doUpdate() == false){
    return false;
  }

  StateScene::update();

  int numberCam = m_pGame->getMotoGame()->getNumberCameras();
  m_pGame->getMotoGame()->setCurrentCamera(numberCam);

  /* When actually playing or when dead and the bike is falling apart, a physics update is required */
  //if(isLockedMotoGame()) {
  //  nPhysSteps = 0;
  //} else {
  //nPhysSteps = _UpdateGamePlaying();            
  
  if(numberCam > 1){
    // m_MotoGame.setCurrentCamera(numberCam);
  }
  // autoZoom();


//    bool v_all_dead       = true;
//    bool v_one_still_play = false;
//    bool v_one_finished   = false;
// 
//    for(unsigned int i=0; i<m_MotoGame.Players().size(); i++) {
//      if(m_MotoGame.Players()[i]->isDead() == false) {
//	v_all_dead = false;
//      }
//      if(m_MotoGame.Players()[i]->isFinished()) {
//	v_one_finished = true;
//      }
//
//      if(m_MotoGame.Players()[i]->isFinished() == false && m_MotoGame.Players()[i]->isDead() == false) {
//	v_one_still_play = true;
//      }
//    }
//    
//    if(v_one_still_play == false || m_bMultiStopWhenOneFinishes) { // let people continuing when one finished or not
//      if(v_one_finished) {
//	/* You're done maaaan! :D */
//	
//	/* finalize the replay */
//	if(m_pJustPlayReplay != NULL) {
//	  if(m_MotoGame.Players().size() == 1) {
//	    /* save the last state because scene don't record each frame */
//	    SerializedBikeState BikeState;
//	    MotoGame::getSerializedBikeState(m_MotoGame.Players()[0]->getState(), m_MotoGame.getTime(), &BikeState);
//	    m_pJustPlayReplay->storeState(BikeState);
//	    m_pJustPlayReplay->finishReplay(true,m_MotoGame.Players()[0]->finishTime());
//	  }
//	}
//
//	/* update profiles */
//	float v_finish_time = 0.0;
//	std::string TimeStamp = getTimeStamp();
//	for(unsigned int i=0; i<m_MotoGame.Players().size(); i++) {
//	  if(m_MotoGame.Players()[i]->isFinished()) {
//	    v_finish_time  = m_MotoGame.Players()[i]->finishTime();
//	  }
//	}
//	if(m_MotoGame.Players().size() == 1) {
//	  m_db->profiles_addFinishTime(m_xmsession->profile(), m_MotoGame.getLevelSrc()->Id(),
//				       TimeStamp, v_finish_time);
//	}
//  
//	/* Update stats */
//	/* update stats only in one player mode */
//	if(m_MotoGame.Players().size() == 1) {       
//	  m_db->stats_levelCompleted(m_xmsession->profile(),
//				     m_MotoGame.getLevelSrc()->Id(),
//				     m_MotoGame.Players()[0]->finishTime());
//	  _UpdateLevelsLists();
//	  _UpdateCurrentPackList(m_MotoGame.getLevelSrc()->Id(),
//				 m_MotoGame.Players()[0]->finishTime());
//	}
//
//	m_stateManager->pushState(new StateFinished(this));
//      } else if(v_all_dead) {
//	/* You're dead maan! */
//	setState(GS_DEADJUST);
//      }
//    }

  return true;
}

bool StatePlaying::render()
{
  if(doRender() == false){
    return false;
  }

  StateScene::render();
  return true;
}

void StatePlaying::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {
  case SDLK_ESCAPE:
    //if(LockedMotoGame() == false) {
      /* Escape pauses */
      m_pGame->getStateManager()->pushState(new StatePause(m_pGame));
      //}
    break;

//  case SDLK_PAGEUP:
//    if(isThereANextLevel(m_PlaySpecificLevelId)) {
//      m_db->stats_abortedLevel(m_xmsession->profile(), m_MotoGame.getLevelSrc()->Id(), m_MotoGame.getTime());
//      m_MotoGame.endLevel();
//      m_Renderer->unprepareForNewLevel();
//      m_PlaySpecificLevelId = _DetermineNextLevel(m_PlaySpecificLevelId);
//      m_bPrePlayAnim = true;
//      setState(GS_PREPLAYING);
//    }
//    break;
//  case SDLK_PAGEDOWN:
//    if(isThereAPreviousLevel(m_PlaySpecificLevelId)) {
//      m_db-> stats_abortedLevel(m_xmsession->profile(), m_MotoGame.getLevelSrc()->Id(), m_MotoGame.getTime());
//	m_MotoGame.endLevel();
//	m_Renderer->unprepareForNewLevel();
//	m_PlaySpecificLevelId = _DeterminePreviousLevel(m_PlaySpecificLevelId);
//	m_bPrePlayAnim = true;
//	setState(GS_PREPLAYING);
//    }
//    break;
  case SDLK_RETURN:
    /* retart immediatly the level */
    m_pGame->restartLevel();
    break;

  default:
    /* Notify the controller */
    m_pGame->getInputHandler()->handleInput(INPUT_KEY_DOWN, nKey, mod,
					    m_pGame->getMotoGame()->Players(),
					    m_pGame->getMotoGame()->Cameras(),
					    m_pGame);
  }
  
  StateScene::keyDown(nKey, mod, nChar);
}

void StatePlaying::keyUp(int nKey, SDLMod mod)
{
  m_pGame->getInputHandler()->handleInput(INPUT_KEY_UP,nKey,mod,
			     m_pGame->getMotoGame()->Players(),
			     m_pGame->getMotoGame()->Cameras(),
			     m_pGame);

  StateScene::keyUp(nKey, mod);
}

void StatePlaying::mouseDown(int nButton)
{
  m_pGame->getInputHandler()->handleInput(INPUT_KEY_DOWN,nButton,KMOD_NONE,
			     m_pGame->getMotoGame()->Players(),
			     m_pGame->getMotoGame()->Cameras(),
			     m_pGame);

  StateScene::mouseDown(nButton);
}

void StatePlaying::mouseDoubleClick(int nButton)
{
  StateScene::mouseDoubleClick(nButton);
}

void StatePlaying::mouseUp(int nButton)
{
  m_pGame->getInputHandler()->handleInput(INPUT_KEY_UP,nButton,KMOD_NONE,
			     m_pGame->getMotoGame()->Players(),
			     m_pGame->getMotoGame()->Cameras(),
			     m_pGame);

  StateScene::mouseUp(nButton);
}
