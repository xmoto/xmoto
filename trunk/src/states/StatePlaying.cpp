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
#include "StateFinished.h"
#include "StateDeadJust.h"
#include "StateDeadMenu.h"
#include "Sound.h"
#include "xmscene/Camera.h"
#include "xmscene/Bike.h"
#include "CameraAnimation.h"
#include "Universe.h"
#include "Trainer.h"
#include "SysMessage.h"

StatePlaying::StatePlaying(Universe* i_universe):
  StateScene(i_universe)
{
  m_name = "StatePlaying";
  m_gameIsFinished = false;
  m_displayStats   = false;

  /* prepare stats */
  makeStatsStr();

  if(XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("STATS_UPDATED");
    StateManager::instance()->registerAsEmitter("LEVELS_UPDATED");
  }
}

StatePlaying::~StatePlaying()
{
}

void StatePlaying::enter()
{
  StateScene::enter();

  m_gameIsFinished = false;

  if(XMSession::instance()->hidePlayingInformation() == false) {
    GameRenderer::instance()->setShowEngineCounter(XMSession::instance()->showEngineCounter());
    GameRenderer::instance()->setShowMinimap(XMSession::instance()->showMinimap());
    GameRenderer::instance()->setShowTimePanel(true);
  } else {
    GameRenderer::instance()->setShowEngineCounter(false);
    GameRenderer::instance()->setShowMinimap(false);
    GameRenderer::instance()->setShowTimePanel(false);
  }

  std::string v_level_name;
  try {
    if(m_universe != NULL) {
      for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	v_level_name = m_universe->getScenes()[i]->getLevelSrc()->Name();
	m_universe->getScenes()[i]->playLevel();
      }
    }
  }
  catch(Exception &e) {
    Logger::Log("** Warning ** : level '%s' cannot be loaded", v_level_name.c_str());

    char cBuf[256];
    sprintf(cBuf,GAMETEXT_LEVELCANNOTBELOADED, v_level_name.c_str());

    StateMessageBox* v_msgboxState = new StateMessageBox(this, cBuf, UI_MSGBOX_OK);
    v_msgboxState->setId("ERROR");
    StateManager::instance()->pushState(v_msgboxState);
  }

  setScoresTimes();

  if(m_universe != NULL) {
    if(m_universe->getScenes().size() > 0) {
      // play music of the first world
      GameApp::instance()->playMusic(m_universe->getScenes()[0]->getLevelSrc()->Music());
    }
  }

  // read keys for more reactivity
  InputHandler::instance()->dealWithActivedKeys(m_universe);

  // reset trainer mode use
  Trainer::instance()->resetTrainerUse();
}

void StatePlaying::leave()
{
  if(m_universe != NULL) {
    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->setInfos("");
    }
  }

  if(GameApp::instance()->isRequestingEnd()) {
    // when end if forced, update stats as aborted
    if(m_universe != NULL) {
      if(m_universe->getScenes().size() == 1) {
	if(m_universe->getScenes()[0]->Players().size() == 1) {
	  if(m_universe->getScenes()[0]->Players()[0]->isDead()     == false &&
	     m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
	    
	    xmDatabase::instance("main")->stats_abortedLevel(XMSession::instance()->sitekey(),
							     XMSession::instance()->profile(),
							     m_universe->getScenes()[0]->getLevelSrc()->Id(),
							     m_universe->getScenes()[0]->getTime());
	  }
	}
      }
    }
  }
}

void StatePlaying::enterAfterPop()
{
  StateScene::enterAfterPop();

  // recheck keys
  InputHandler::instance()->dealWithActivedKeys(m_universe);
  m_fLastPhysTime = GameApp::getXMTime();
  m_displayStats = false;
}

bool StatePlaying::update()
{
  if(StateScene::update() == false)
    return false;

  if(isLockedScene() == false) {
    bool v_all_dead       = true;
    bool v_one_still_play = false;
    bool v_one_finished   = false;
    
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Players().size(); i++) {
	  if(m_universe->getScenes()[j]->Players()[i]->isDead() == false) {
	    v_all_dead = false;
	  }
	  if(m_universe->getScenes()[j]->Players()[i]->isFinished()) {
	    v_one_finished = true;
	  }
	  
	  if(m_universe->getScenes()[j]->Players()[i]->isFinished() == false &&
	     m_universe->getScenes()[j]->Players()[i]->isDead() == false) {
	    v_one_still_play = true;
	  }
	}
      }
    }
    
    if(m_gameIsFinished == false) {
      if(v_one_still_play == false || XMSession::instance()->MultiStopWhenOneFinishes()) { // let people continuing when one finished or not
	if(v_one_finished) {
	  /* You're done maaaan! :D */
	  onOneFinish();
	  m_gameIsFinished = true;
	} else if(v_all_dead) {
	  /* You're dead maan! */
	  onAllDead();
	  m_gameIsFinished = true;
	}
      }
    }
  }

  return true;
}

void StatePlaying::keyDown(SDLKey nKey, SDLMod mod,int nChar, const std::string& i_utf8Char)
{
  if(nKey == SDLK_ESCAPE){
    if(isLockedScene() == false) {
      /* Escape pauses */
      m_displayStats = true;
      StateManager::instance()->pushState(new StatePause(m_universe, this));
    }
  }

#if defined(ENABLE_DEV)
  else if(nKey == SDLK_KP0 && (mod & KMOD_LCTRL) == KMOD_LCTRL){
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Players().size(); i++) {
	  if(m_universe->getScenes()[j]->Cameras().size() > 0) {
	    m_universe->TeleportationCheatTo(i, Vector2f(m_universe->getScenes()[j]->Cameras()[0]->getCameraPositionX(),
							 m_universe->getScenes()[j]->Cameras()[0]->getCameraPositionY()));
	  }
	}
      }
    }
  }
  else if(nKey == SDLK_KP0 && (mod & (KMOD_CTRL|KMOD_SHIFT|KMOD_ALT|KMOD_META)) == 0){        //TRAINER
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
        Trainer::instance()->storePosition( m_universe->getScenes()[j]->getLevelSrc()->Id(),
                                            m_universe->getScenes()[j]->getPlayerPosition(0) );
          //TODO: bool getPlayerFaceDir (int i_player)
        char sysmsg[256];
        snprintf(sysmsg, 256, SYS_MSG_TRAIN_STORED, Trainer::instance()->getMaxRestoreIndex()+1);
        SysMessage::instance()->displayText(sysmsg);
      }
    }
  }
  else if(nKey == SDLK_BACKSPACE){       //TRAINER
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
        if( Trainer::instance()->isRestorePositionAvailable( m_universe->getScenes()[j]->getLevelSrc()->Id() ) ) {
          Vector2f pos = Trainer::instance()->getCurrentRestorePosition( m_universe->getScenes()[j]->getLevelSrc()->Id() );
          m_universe->TeleportationCheatTo(0, pos );
          char sysmsg[256];
          snprintf(sysmsg, 256, SYS_MSG_TRAIN_RESTORING, Trainer::instance()->getCurrentRestoreIndex()+1,
                                                         Trainer::instance()->getMaxRestoreIndex()+1);
          SysMessage::instance()->displayText(sysmsg);
        } else {
          SysMessage::instance()->displayText(SYS_MSG_TRAIN_NO_RESTORE_AVAIL);
        }
      }
    }
  }
  else if(nKey == SDLK_KP_MINUS){        //TRAINER
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
        if( Trainer::instance()->isRestorePositionAvailable( m_universe->getScenes()[j]->getLevelSrc()->Id() ) ) {
          Vector2f pos = Trainer::instance()->getPreviousRestorePosition( m_universe->getScenes()[j]->getLevelSrc()->Id() );
          m_universe->TeleportationCheatTo(0, pos );
          char sysmsg[256];
          snprintf(sysmsg, 256, SYS_MSG_TRAIN_RESTORING, Trainer::instance()->getCurrentRestoreIndex()+1,
                                                         Trainer::instance()->getMaxRestoreIndex()+1);
          SysMessage::instance()->displayText(sysmsg);
        } else {
          SysMessage::instance()->displayText(SYS_MSG_TRAIN_NO_RESTORE_AVAIL);
        }
      }
    }
  }
  else if(nKey == SDLK_KP_PLUS){        //TRAINER
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
        if( Trainer::instance()->isRestorePositionAvailable( m_universe->getScenes()[j]->getLevelSrc()->Id() ) ) {
          Vector2f pos = Trainer::instance()->getNextRestorePosition( m_universe->getScenes()[j]->getLevelSrc()->Id() );
          m_universe->TeleportationCheatTo(0, pos );
          char sysmsg[256];
          snprintf(sysmsg, 256, SYS_MSG_TRAIN_RESTORING, Trainer::instance()->getCurrentRestoreIndex()+1,
                                                         Trainer::instance()->getMaxRestoreIndex()+1);
          SysMessage::instance()->displayText(sysmsg);
        } else {
          SysMessage::instance()->displayText(SYS_MSG_TRAIN_NO_RESTORE_AVAIL);
        }
      }
    }
  }
#endif

  else {
    // to avoid people changing direction during the autozoom
    if(m_autoZoom == false){
      /* Notify the controller */
      InputHandler::instance()->handleInput(m_universe, INPUT_DOWN, XMKey(nKey, mod));
    }
  }

  StateScene::keyDown(nKey, mod, nChar, i_utf8Char);
}

void StatePlaying::keyUp(SDLKey nKey, SDLMod mod, const std::string& i_utf8Char) {
  InputHandler::instance()->handleInput(m_universe, INPUT_UP, XMKey(nKey, mod));
  StateScene::keyUp(nKey, mod, i_utf8Char);
}

void StatePlaying::mouseDown(int nButton)
{
  // to avoid people changing direction during the autozoom
  if(m_autoZoom == false){
    InputHandler::instance()->handleInput(m_universe, INPUT_DOWN, XMKey(nButton));
  }
  StateScene::mouseDown(nButton);
}

void StatePlaying::mouseUp(int nButton)
{
  InputHandler::instance()->handleInput(m_universe, INPUT_UP, XMKey(nButton));

  StateScene::mouseUp(nButton);
}

void StatePlaying::joystickAxisMotion(Uint8 i_joyNum, Uint8 i_joyAxis, Sint16 i_joyAxisValue) {
  // to avoid people changing direction during the autozoom
  if(m_autoZoom == false){
    InputHandler::instance()->handleInput(m_universe, InputHandler::instance()->joystickAxisSens(i_joyAxisValue),
					  XMKey(InputHandler::instance()->getJoyId(i_joyNum), i_joyAxis, i_joyAxisValue));
  }
}

void StatePlaying::joystickButtonDown(Uint8 i_joyNum, Uint8 i_joyButton) {
  // to avoid people changing direction during the autozoom
  if(m_autoZoom == false){
    InputHandler::instance()->handleInput(m_universe, INPUT_DOWN, XMKey(InputHandler::instance()->getJoyId(i_joyNum), i_joyButton));
  }
}

void StatePlaying::joystickButtonUp(Uint8 i_joyNum, Uint8 i_joyButton) {
  InputHandler::instance()->handleInput(m_universe, INPUT_UP, XMKey(InputHandler::instance()->getJoyId(i_joyNum), i_joyButton));
}

void StatePlaying::onOneFinish() {
  GameApp*  pGame = GameApp::instance();

  /* finalize the replay */
  if(m_universe != NULL) {
    if(m_universe->isAReplayToSave()) {
      m_universe->finalizeReplay(true);
    }
  }
  
  /* update profile and stats */
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(m_universe->getScenes()[0]->Players().size() == 1) {
	int v_finish_time = 0;
	std::string TimeStamp = pGame->getTimeStamp();
	if(m_universe->getScenes()[0]->Players()[0]->isFinished()) {
	  v_finish_time  = m_universe->getScenes()[0]->Players()[0]->finishTime();
	}
        // Updating the stats if the Trainer has not been used
        if(Trainer::instance()->trainerHasBeenUsed() == false){
            xmDatabase::instance("main")->profiles_addFinishTime(XMSession::instance()->sitekey(),
								 XMSession::instance()->profile(),
								 m_universe->getScenes()[0]->getLevelSrc()->Id(),
								 TimeStamp,
								 v_finish_time);
        }
	xmDatabase::instance("main")->stats_levelCompleted(XMSession::instance()->sitekey(),
							   XMSession::instance()->profile(),
							   m_universe->getScenes()[0]->getLevelSrc()->Id(),
							   m_universe->getScenes()[0]->Players()[0]->finishTime());
	StateManager::instance()->sendAsynchronousMessage("LEVELS_UPDATED");
        StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
      }
    }
  }
  
  StateManager::instance()->pushState(new StateFinished(m_universe, this));
}

void StatePlaying::onAllDead() {  
  if(m_universe != NULL) {	
    if(m_universe->isAReplayToSave()) {
      m_universe->finalizeReplay(false);
    }
  }
  
  /* Update stats */
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(m_universe->getScenes()[0]->Players().size() == 1) {
	xmDatabase::instance("main")->stats_died(XMSession::instance()->sitekey(),
						 XMSession::instance()->profile(),
						 m_universe->getScenes()[0]->getLevelSrc()->Id(),
						 m_universe->getScenes()[0]->getTime());
	StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
      }
    }
  }
  
  /* Play the DIE!!! sound */
  try {
    Sound::playSampleByName(Theme::instance()->getSound("Headcrash")->FilePath(), 0.3);
  } catch(Exception &e) {
  }
  
  if(XMSession::instance()->enableDeadAnimation()) {
    StateManager::instance()->replaceState(new StateDeadJust(m_universe));
  } else {
    StateManager::instance()->pushState(new StateDeadMenu(m_universe, true, this));
  }
}

void StatePlaying::abortPlaying() {
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(m_universe->getScenes()[0]->Players().size() == 1) {
	if(m_universe->getScenes()[0]->Players()[0]->isDead()     == false &&
	   m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
	  xmDatabase::instance("main")->stats_abortedLevel(XMSession::instance()->sitekey(),
							   XMSession::instance()->profile(),
							   m_universe->getScenes()[0]->getLevelSrc()->Id(),
							   m_universe->getScenes()[0]->getTime());
	  StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
	}
      }
    }
  }

  StateScene::abortPlaying();
}

void StatePlaying::nextLevel(bool i_positifOrder) {
  GameApp*  pGame  = GameApp::instance();
  std::string v_nextLevel;
  std::string v_currentLevel;
  
  // take the level id of the first world
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() > 0) {
      v_currentLevel = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  if(i_positifOrder) {
    v_nextLevel = pGame->determineNextLevel(v_currentLevel);
  } else {
    v_nextLevel = pGame->determinePreviousLevel(v_currentLevel);
  }

  /* update stats */
  if(v_nextLevel != "") {
    if(m_universe != NULL) {
      if(m_universe->getScenes().size() == 1) {
	if(m_universe->getScenes()[0]->Players().size() == 1) {
	  if(m_universe->getScenes()[0]->Players()[0]->isDead()     == false &&
	     m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
	    xmDatabase::instance("main")->stats_abortedLevel(XMSession::instance()->sitekey(),
							     XMSession::instance()->profile(),
							     v_currentLevel,
							     m_universe->getScenes()[0]->getTime());
	    StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
	  }
	}
      }
    }
  }

  nextLevelToPlay(i_positifOrder);
}

void StatePlaying::restartLevel(bool i_reloadLevel) {
  /* Update stats */
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(m_universe->getScenes()[0]->Players().size() == 1) {
	if(m_universe->getScenes()[0]->Players()[0]->isDead()     == false &&
	   m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
	  xmDatabase::instance("main")->stats_levelRestarted(XMSession::instance()->sitekey(),
							     XMSession::instance()->profile(),
							     m_universe->getScenes()[0]->getLevelSrc()->Id(),
							     m_universe->getScenes()[0]->getTime());
	  StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
	}
      }
    }
  }

  restartLevelToPlay(i_reloadLevel);
}

bool StatePlaying::renderOverShadow() {
  if(m_displayStats) {
    displayStats();
  }

  return true;
}
