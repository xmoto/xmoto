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

#include "StateDeadMenu.h"
#include "Game.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"
#include "StateMessageBox.h"
#include "Universe.h"
#include "Replay.h"
#include "helpers/Log.h"

/* static members */
UIRoot*  StateDeadMenu::m_sGUI = NULL;

StateDeadMenu::StateDeadMenu(Universe* i_universe,
			     bool i_doShadeAnim,
			     bool drawStateBehind,
			     bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind,
	    true,
	    i_doShadeAnim)
{
  m_name    = "StateDeadMenu";
  m_universe = i_universe;

  if(XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("RESTART");
    StateManager::instance()->registerAsEmitter("NEXTLEVEL");
    StateManager::instance()->registerAsEmitter("ABORT");
  }
}

StateDeadMenu::~StateDeadMenu()
{
}

void StateDeadMenu::enter()
{
  GameApp* gameApp = GameApp::instance();
  std::string v_id_level;

  if(m_universe != NULL) {
    if(m_universe->getScenes().size() > 0) {
      v_id_level = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  if(m_universe != NULL) {
    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->setInfos(m_universe->getScenes()[i]->getLevelSrc()->Name());
    }
  }
  
  createGUIIfNeeded();
  m_GUI = m_sGUI;

  UIButton *playNextButton = reinterpret_cast<UIButton *>(m_GUI->getChild("DEADMENU_FRAME:PLAYNEXT_BUTTON"));
  playNextButton->enableWindow(gameApp->isThereANextLevel(v_id_level));

  if(m_universe != NULL) {
    UIButton* saveReplayButton = reinterpret_cast<UIButton *>(m_GUI->getChild("DEADMENU_FRAME:SAVEREPLAY_BUTTON"));
    saveReplayButton->enableWindow(m_universe->isAReplayToSave());
  }

  StateMenu::enter();
}

void StateDeadMenu::leave()
{
  if(m_universe != NULL) {
    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->setInfos("");
    }
  }
}

void StateDeadMenu::checkEvents() {
  UIButton *pTryAgainButton = reinterpret_cast<UIButton *>(m_GUI->getChild("DEADMENU_FRAME:TRYAGAIN_BUTTON"));
  if(pTryAgainButton->isClicked()) {
    pTryAgainButton->setClicked(false);

    m_requestForEnd = true;
    StateManager::instance()->sendAsynchronousMessage("RESTART");
  }

  UIButton *pSavereplayButton = reinterpret_cast<UIButton *>(m_GUI->getChild("DEADMENU_FRAME:SAVEREPLAY_BUTTON"));
  if(pSavereplayButton->isClicked()) {
    pSavereplayButton->setClicked(false);

    StateMessageBox* v_msgboxState = new StateMessageBox(this, std::string(GAMETEXT_ENTERREPLAYNAME) + ":",
							 UI_MSGBOX_OK|UI_MSGBOX_CANCEL, true, Replay::giveAutomaticName());
    v_msgboxState->setId("SAVEREPLAY");
    StateManager::instance()->pushState(v_msgboxState);
  }

  UIButton *pPlaynextButton = reinterpret_cast<UIButton *>(m_GUI->getChild("DEADMENU_FRAME:PLAYNEXT_BUTTON"));
  if(pPlaynextButton->isClicked()) {
    pPlaynextButton->setClicked(false);

    m_requestForEnd = true;
    StateManager::instance()->sendAsynchronousMessage("NEXTLEVEL");
  }

  UIButton *pAbortButton = reinterpret_cast<UIButton *>(m_GUI->getChild("DEADMENU_FRAME:ABORT_BUTTON"));
  if(pAbortButton->isClicked()) {
    pAbortButton->setClicked(false);

    m_requestForEnd = true;
    StateManager::instance()->sendAsynchronousMessage("ABORT");
  }

  UIButton *pQuitButton = reinterpret_cast<UIButton *>(m_GUI->getChild("DEADMENU_FRAME:QUIT_BUTTON"));
  if(pQuitButton->isClicked()) {
    pQuitButton->setClicked(false);

    StateMessageBox* v_msgboxState = new StateMessageBox(this, GAMETEXT_QUITMESSAGE, UI_MSGBOX_YES|UI_MSGBOX_NO);
    v_msgboxState->setId("QUIT");
    StateManager::instance()->pushState(v_msgboxState);
  }

}

void StateDeadMenu::sendFromMessageBox(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  if(i_id == "QUIT") {
    switch(i_button) {
    case UI_MSGBOX_YES:
      m_requestForEnd = true;
      GameApp::instance()->requestEnd();
      break;
    case UI_MSGBOX_NO:
      return;
      break;
    default:
      break;
    }
  } else if(i_id == "SAVEREPLAY") {
    if(i_button == UI_MSGBOX_OK) {
      if(m_universe != NULL) {
	m_replayName = i_input;
	addCommand("SAVEREPLAY");
      }
    }
  }
}

void StateDeadMenu::executeOneCommand(std::string cmd, std::string args)
{
  if(XMSession::instance()->debug() == true) {
    Logger::Log("cmd [%s [%s]] executed by state [%s].",
		cmd.c_str(), args.c_str(), getName().c_str());
  }

  if(cmd == "SAVEREPLAY") {
    try {
      m_universe->saveReplay(m_replayName);
    } catch(Exception &e) {
      StateManager::instance()->pushState(new StateMessageBox(NULL, e.getMsg(), UI_MSGBOX_OK));
    }
  } else {
    GameState::executeOneCommand(cmd, args);
  }
}

void StateDeadMenu::keyDown(int nKey, SDLMod mod,int nChar, const std::string& i_utf8Char)
{
  if(nKey == SDLK_ESCAPE){
    /* quit this state */
    StateManager::instance()->sendAsynchronousMessage("ABORT");
    m_requestForEnd = true;
  }
  else {
    StateMenu::keyDown(nKey, mod, nChar, i_utf8Char);
  }
}

void StateDeadMenu::clean() {
  if(StateDeadMenu::m_sGUI != NULL) {
    delete StateDeadMenu::m_sGUI;
    StateDeadMenu::m_sGUI = NULL;
  }
}

void StateDeadMenu::createGUIIfNeeded() {
  UIButton *v_button;
  UIFrame  *v_frame;

  if(m_sGUI != NULL)
    return;

  DrawLib* drawLib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot();
  m_sGUI->setFont(drawLib->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      drawLib->getDispWidth(),
		      drawLib->getDispHeight());

  v_frame = new UIFrame(m_sGUI,
			drawLib->getDispWidth()/2  - 400/2,
			drawLib->getDispHeight()/2 - 540/2,
			"", 400, 540);
  v_frame->setID("DEADMENU_FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_MENU);

  UIStatic *pDeadText = new UIStatic(v_frame, 0, 100, GAMETEXT_JUSTDEAD, v_frame->getPosition().nWidth, 36);
  pDeadText->setFont(drawLib->getFontMedium());

  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 5*57/2 + 0*57, GAMETEXT_TRYAGAIN, 207, 57);
  v_button->setID("TRYAGAIN_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_TRY_LEVEL_AGAIN);
  v_button->setFont(drawLib->getFontSmall());
  v_frame->setPrimaryChild(v_button); /* default button */

  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 5*57/2 + 1*57, GAMETEXT_SAVEREPLAY, 207, 57);
  v_button->setID("SAVEREPLAY_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_SAVE_A_REPLAY);
  v_button->setFont(drawLib->getFontSmall());

  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 5*57/2 + 2*57, GAMETEXT_PLAYNEXT, 207, 57);
  v_button->setID("PLAYNEXT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_PLAY_NEXT_LEVEL);
  v_button->setFont(drawLib->getFontSmall());

  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 5*57/2 + 3*57, GAMETEXT_ABORT, 207, 57);
  v_button->setID("ABORT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_BACK_TO_MAIN_MENU);
  v_button->setFont(drawLib->getFontSmall());

  v_button = new UIButton(v_frame, 400/2 - 207/2, v_frame->getPosition().nHeight/2 - 5*57/2 + 4*57, GAMETEXT_QUIT, 207, 57);
  v_button->setID("QUIT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_QUIT_THE_GAME);
  v_button->setFont(drawLib->getFontSmall());
}
