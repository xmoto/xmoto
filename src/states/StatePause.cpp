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

#include "StatePause.h"
#include "StateMessageBox.h"
#include "drawlib/DrawLib.h"
#include "helpers/Log.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"
#include "xmoto/Sound.h"
#include "xmoto/Universe.h"

/* static members */
UIRoot *StatePause::m_sGUI = NULL;

StatePause::StatePause(Universe *i_universe,
                       const std::string &i_parentId,
                       bool drawStateBehind,
                       bool updateStatesBehind)
  : StateMenu(drawStateBehind, updateStatesBehind) {
  m_name = "StatePause";
  m_universe = i_universe;
  m_parentId = i_parentId;

  if (XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("RESTART");
    StateManager::instance()->registerAsEmitter("NEXTLEVEL");
    StateManager::instance()->registerAsEmitter("ABORT");
  }
}

StatePause::~StatePause() {}

void StatePause::enter() {
  std::string v_id_level;
  GameApp *pGame = GameApp::instance();

  if (m_universe != NULL) {
    if (m_universe->getScenes().size() > 0) {
      v_id_level = m_universe->getScenes()[0]->getLevelSrc()->Id();
    }
  }

  if (m_universe != NULL) {
    for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->setInfos(
        m_universe->getScenes()[i]->getLevelSrc()->Name());
    }
  }

  Sound::togglePauseMusic();

  createGUIIfNeeded(&m_screen);
  m_GUI = m_sGUI;

  /* reset the playnext button */
  UIButton *playNextButton = reinterpret_cast<UIButton *>(
    m_GUI->getChild("PAUSE_FRAME:PLAYNEXT_BUTTON"));
  playNextButton->enableWindow(pGame->isThereANextLevel(v_id_level));

  StateMenu::enter();
}

void StatePause::leave() {
  StateMenu::leave();

  if (m_universe != NULL) {
    for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->setInfos("");
    }
  }

  Sound::togglePauseMusic();
}

void StatePause::checkEvents() {
  UIButton *pResumeButton =
    reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:RESUME_BUTTON"));
  if (pResumeButton->isClicked()) {
    pResumeButton->setClicked(false);

    m_requestForEnd = true;
  }

  UIButton *pRestartButton =
    reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:RESTART_BUTTON"));
  if (pRestartButton->isClicked()) {
    pRestartButton->setClicked(false);

    m_requestForEnd = true;
    StateManager::instance()->sendAsynchronousMessage("RESTART", m_parentId);
  }

  UIButton *pPlayNextButton = reinterpret_cast<UIButton *>(
    m_GUI->getChild("PAUSE_FRAME:PLAYNEXT_BUTTON"));
  if (pPlayNextButton->isClicked()) {
    pPlayNextButton->setClicked(false);

    m_requestForEnd = true;
    StateManager::instance()->sendAsynchronousMessage("NEXTLEVEL", m_parentId);
  }

  UIButton *pAbortButton =
    reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:ABORT_BUTTON"));
  if (pAbortButton->isClicked()) {
    pAbortButton->setClicked(false);

    m_requestForEnd = true;
    StateManager::instance()->sendAsynchronousMessage("ABORT", m_parentId);
  }

  UIButton *pQuitButton =
    reinterpret_cast<UIButton *>(m_GUI->getChild("PAUSE_FRAME:QUIT_BUTTON"));
  if (pQuitButton->isClicked()) {
    pQuitButton->setClicked(false);

    StateMessageBox *v_msgboxState = new StateMessageBox(
      this, GAMETEXT_QUITMESSAGE, UI_MSGBOX_YES | UI_MSGBOX_NO);
    v_msgboxState->setMsgBxId("QUIT");
    StateManager::instance()->pushState(v_msgboxState);
  }
}

void StatePause::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (i_type == INPUT_DOWN &&
      (i_xmkey == (*Input::instance()->getGlobalKey(INPUT_PLAYINGPAUSE)) ||
       i_xmkey.getJoyButton() == SDL_CONTROLLER_BUTTON_START)) {
    /* quit this state */
    m_requestForEnd = true;
  }

  else if (i_type == INPUT_DOWN && i_xmkey == (*Input::instance()->getGlobalKey(
                                                INPUT_SWITCHFAVORITE))) {
    if (m_universe != NULL) {
      if (m_universe->getScenes().size() > 0) { // just add the first world
        GameApp::instance()->switchLevelToFavorite(
          m_universe->getScenes()[0]->getLevelSrc()->Id(), true);
        StateManager::instance()->sendAsynchronousMessage("FAVORITES_UPDATED");
      }
    }
  }

  else if (i_type == INPUT_DOWN && i_xmkey == (*Input::instance()->getGlobalKey(
                                                INPUT_SWITCHBLACKLIST))) {
    if (m_universe != NULL) {
      if (m_universe->getScenes().size() >
          0) { // just blacklist the first world
        GameApp::instance()->switchLevelToBlacklist(
          m_universe->getScenes()[0]->getLevelSrc()->Id(), true);
        StateManager::instance()->sendAsynchronousMessage(
          "BLACKLISTEDLEVELS_UPDATED");
      }
    }
  }

  else {
    StateMenu::xmKey(i_type, i_xmkey);
  }
}

void StatePause::clean() {
  if (StatePause::m_sGUI != NULL) {
    delete StatePause::m_sGUI;
    StatePause::m_sGUI = NULL;
  }
}

void StatePause::createGUIIfNeeded(RenderSurface *i_screen) {
  UIButton *v_button;
  UIFrame *v_frame;

  if (m_sGUI != NULL)
    return;

  DrawLib *drawLib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot(i_screen);
  m_sGUI->setFont(drawLib->getFontSmall());
  m_sGUI->setPosition(
    0, 0, i_screen->getDispWidth(), i_screen->getDispHeight());

  v_frame = new UIFrame(m_sGUI,
                        i_screen->getDispWidth() / 2 - 400 / 2,
                        i_screen->getDispHeight() / 2 - 540 / 2,
                        "",
                        400,
                        540);
  v_frame->setID("PAUSE_FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_MENU);

  UIStatic *pPauseText = new UIStatic(
    v_frame, 0, 100, GAMETEXT_PAUSE, v_frame->getPosition().nWidth, 36);
  pPauseText->setFont(drawLib->getFontMedium());

  v_button =
    new UIButton(v_frame,
                 400 / 2 - 207 / 2,
                 v_frame->getPosition().nHeight / 2 - 5 * 57 / 2 + 0 * 57,
                 GAMETEXT_RESUME,
                 207,
                 57);
  v_button->setID("RESUME_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_BACK_TO_GAME);
  v_button->setFont(drawLib->getFontSmall());
  v_frame->setPrimaryChild(v_button); /* default button */

  v_button =
    new UIButton(v_frame,
                 400 / 2 - 207 / 2,
                 v_frame->getPosition().nHeight / 2 - 5 * 57 / 2 + 1 * 57,
                 GAMETEXT_RESTART,
                 207,
                 57);
  v_button->setID("RESTART_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_TRY_LEVEL_AGAIN_FROM_BEGINNING);
  v_button->setFont(drawLib->getFontSmall());

  v_button =
    new UIButton(v_frame,
                 400 / 2 - 207 / 2,
                 v_frame->getPosition().nHeight / 2 - 5 * 57 / 2 + 2 * 57,
                 GAMETEXT_PLAYNEXT,
                 207,
                 57);
  v_button->setID("PLAYNEXT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_PLAY_NEXT_INSTEAD);
  v_button->setFont(drawLib->getFontSmall());

  v_button =
    new UIButton(v_frame,
                 400 / 2 - 207 / 2,
                 v_frame->getPosition().nHeight / 2 - 5 * 57 / 2 + 3 * 57,
                 GAMETEXT_ABORT,
                 207,
                 57);
  v_button->setID("ABORT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_BACK_TO_MAIN_MENU);
  v_button->setFont(drawLib->getFontSmall());

  v_button =
    new UIButton(v_frame,
                 400 / 2 - 207 / 2,
                 v_frame->getPosition().nHeight / 2 - 5 * 57 / 2 + 4 * 57,
                 GAMETEXT_QUIT,
                 207,
                 57);
  v_button->setID("QUIT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_QUIT_THE_GAME);
  v_button->setFont(drawLib->getFontSmall());
}

void StatePause::sendFromMessageBox(const std::string &i_id,
                                    UIMsgBoxButton i_button,
                                    const std::string &i_input) {
  if (i_id == "QUIT") {
    switch (i_button) {
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
  }

  else {
    StateMenu::sendFromMessageBox(i_id, i_button, i_input);
  }
}
