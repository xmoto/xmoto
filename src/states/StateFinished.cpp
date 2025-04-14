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

#include "StateFinished.h"
#include "StateMessageBox.h"
#include "StatePreplayingReplay.h"
#include "StateUploadHighscore.h"
#include "StateVote.h"
#include "common/XMSession.h"
#include "drawlib/DrawLib.h"
#include "helpers/Log.h"
#include "helpers/Text.h"
#include "thread/SendVoteThread.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"
#include "xmoto/Replay.h"
#include "xmoto/Sound.h"
#include "xmoto/SysMessage.h"
#include "xmoto/Universe.h"

/* static members */
UIRoot *StateFinished::m_sGUI = NULL;

StateFinished::StateFinished(Universe *i_universe,
                             const std::string &i_parentId,
                             bool drawStateBehind,
                             bool updateStatesBehind)
  : StateMenu(drawStateBehind, updateStatesBehind) {
  m_name = "StateFinished";
  m_universe = i_universe;
  m_parentId = i_parentId;

  if (XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("RESTART");
    StateManager::instance()->registerAsEmitter("NEXTLEVEL");
    StateManager::instance()->registerAsEmitter("FINISH");
  }
}

StateFinished::~StateFinished() {}

void StateFinished::enter() {
  int v_finish_time = -1;
  int v_room_highscore = -1;
  std::string TimeStamp;
  bool v_is_a_room_highscore = false;
  bool v_is_a_personnal_highscore = false;
  GameApp *pGame = GameApp::instance();
  std::string v_id_level;
  std::string v_roomsTimes;

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

  createGUIIfNeeded(&m_screen);
  m_GUI = m_sGUI;

  /* reset the playnext button */
  UIButton *playNextButton = reinterpret_cast<UIButton *>(
    m_GUI->getChild("FINISHED_FRAME:PLAYNEXT_BUTTON"));
  playNextButton->enableWindow(pGame->isThereANextLevel(v_id_level));

  if (m_universe != NULL) {
    UIButton *saveReplayButton = reinterpret_cast<UIButton *>(
      m_GUI->getChild("FINISHED_FRAME:SAVEREPLAY_BUTTON"));
    saveReplayButton->enableWindow(m_universe->isAReplayToSave());

    UIButton *viewReplayButton = reinterpret_cast<UIButton *>(
      m_GUI->getChild("FINISHED_FRAME:VIEWREPLAY_BUTTON"));
    viewReplayButton->enableWindow(m_universe->isAReplayToSave());
  }

  UIButton *v_uploadButton = reinterpret_cast<UIButton *>(
    m_GUI->getChild("FINISHED_FRAME:UPLOAD_BUTTON"));
  v_uploadButton->enableWindow(false);

  UIStatic *v_pMedal_str =
    reinterpret_cast<UIStatic *>(m_GUI->getChild("MEDALSTR_STATIC"));
  v_pMedal_str->setCaption("");

  UIStatic *v_pNewHighscore_str =
    reinterpret_cast<UIStatic *>(m_GUI->getChild("HIGHSCORESTR_STATIC"));
  v_pNewHighscore_str->setCaption("");

  UIStatic *v_pNewHighscoreSaved_str =
    reinterpret_cast<UIStatic *>(m_GUI->getChild("HIGHSCORESAVEDSTR_STATIC"));
  v_pNewHighscoreSaved_str->setCaption("");

  UIStatic *v_pRoomsTimes_str =
    reinterpret_cast<UIStatic *>(m_GUI->getChild("ROOMSTIMES_STATIC"));
  /* rooms times */
  v_roomsTimes = "";
  int v_room_highscore_tmp;
  std::string v_author_tmp;
  for (unsigned int i = 0; i < XMSession::instance()->nbRoomsEnabled(); i++) {
    v_roomsTimes = v_roomsTimes +
                   GameApp::instance()->getWorldRecord(
                     i, v_id_level, v_room_highscore_tmp, v_author_tmp) +
                   "\n";
    if (i == 0) {
      v_room_highscore = v_room_highscore_tmp;
    }
  }
  v_pRoomsTimes_str->setCaption(v_roomsTimes);

  if (m_universe != NULL) {
    m_universe->isTheCurrentPlayAHighscore(xmDatabase::instance("main"),
                                           v_is_a_personnal_highscore,
                                           v_is_a_room_highscore);
  }

  /* activ button */
  if (XMSession::instance()->beatingMode()) {
    UIButton *pTryAgainButton = reinterpret_cast<UIButton *>(
      m_GUI->getChild("FINISHED_FRAME:TRYAGAIN_BUTTON"));
    pTryAgainButton->makeActive();
  }

  /* replay */
  if (m_universe != NULL) {
    if (m_universe->isAReplayToSave()) {
      /* upload button */
      if (v_is_a_room_highscore) {
        /* active upload button */
        if (XMSession::instance()->www()) {
          v_uploadButton->enableWindow(v_is_a_room_highscore);
          v_uploadButton->makeActive(); // always preselect the upload button
          // even in beating mode
        }
      }

      /* autosave */
      if (v_is_a_room_highscore || v_is_a_personnal_highscore) {
        if (XMSession::instance()->autosaveHighscoreReplays()) {
          std::string v_replayName;
          char v_str[256];
          v_replayName = Replay::giveAutomaticName();

          try {
            m_universe->saveReplay(xmDatabase::instance("main"), v_replayName);
          } catch (Exception &e) {
            StateManager::instance()->pushState(
              new StateMessageBox(this, e.getMsg(), UI_MSGBOX_OK));
          }

          snprintf(v_str, 256, GAMETEXT_SAVE_AS, v_replayName.c_str());
          v_pNewHighscoreSaved_str->setCaption(v_str);
        }
      }
    } else {
      // there is no highscore to save, but we want even make it enable, just to
      // display a message
      if (m_universe->isAnErrorOnSaving()) {
        v_uploadButton->enableWindow(true);
      }
    }
  }

  /* sound */
  if (v_is_a_room_highscore) {
    try {
      GameApp::instance()->playGameMusic("");
      Sound::playSampleByName(
        Theme::instance()->getSound("NewHighscore")->FilePath());
    } catch (Exception &e) {
    }
  } else {
    try {
      GameApp::instance()->playGameMusic("");
      Sound::playSampleByName(
        Theme::instance()->getSound("EndOfLevel")->FilePath());
    } catch (Exception &e) {
    }
  }

  // finish time
  if (m_universe != NULL) {
    if (m_universe->getScenes()[0]->Players().size() == 1) {
      v_finish_time = m_universe->getScenes()[0]->Players()[0]->finishTime();
    }
  }

  /* new highscores text */
  if (v_is_a_room_highscore) {
    v_pNewHighscore_str->setFont(pGame->getDrawLib()->getFontMedium());
    v_pNewHighscore_str->setCaption(GAMETEXT_NEWHIGHSCORE);
  } else {
    if (v_is_a_personnal_highscore) {
      v_pNewHighscore_str->setFont(pGame->getDrawLib()->getFontSmall());
      v_pNewHighscore_str->setCaption(GAMETEXT_NEWHIGHSCOREPERSONAL);

      std::string v_currentMedal;
      if (GameApp::instance()->getCurrentMedal(
            v_room_highscore, v_finish_time, v_currentMedal)) {
        char v_str[128];
        snprintf(v_str, 128, GAMETEXT_NEW_MEDAL, v_currentMedal.c_str());
        v_pMedal_str->setCaption(v_str);
      }
    }
  }

  UIBestTimes *v_pBestTimes =
    reinterpret_cast<UIBestTimes *>(m_GUI->getChild("BESTTIMES"));
  makeBestTimesWindow(
    v_pBestTimes, XMSession::instance()->profile(), v_id_level, v_finish_time);

  if (m_universe != NULL) {
    if (m_universe->getScenes().size() == 1) {
      if (SendVoteThread::isToPropose(
            xmDatabase::instance("main"),
            m_universe->getScenes()[0]->getLevelSrc()->Id())) {
        StateManager::instance()->pushState(
          new StateVote(m_universe->getScenes()[0]->getLevelSrc()->Id()));
      }
    }
  }

  StateMenu::enter();
}

void StateFinished::leave() {
  StateMenu::leave();

  if (m_universe != NULL) {
    for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->setInfos("");
    }
  }
}

void StateFinished::checkEvents() {
  UIButton *pTryAgainButton = reinterpret_cast<UIButton *>(
    m_GUI->getChild("FINISHED_FRAME:TRYAGAIN_BUTTON"));
  if (pTryAgainButton->isClicked()) {
    pTryAgainButton->setClicked(false);

    m_requestForEnd = true;
    StateManager::instance()->sendAsynchronousMessage("RESTART", m_parentId);
  }

  UIButton *pPlaynextButton = reinterpret_cast<UIButton *>(
    m_GUI->getChild("FINISHED_FRAME:PLAYNEXT_BUTTON"));
  if (pPlaynextButton->isClicked()) {
    pPlaynextButton->setClicked(false);

    m_requestForEnd = true;
    StateManager::instance()->sendAsynchronousMessage("NEXTLEVEL", m_parentId);
  }

  UIButton *pSavereplayButton = reinterpret_cast<UIButton *>(
    m_GUI->getChild("FINISHED_FRAME:SAVEREPLAY_BUTTON"));
  if (pSavereplayButton->isClicked()) {
    pSavereplayButton->setClicked(false);

    StateMessageBox *v_msgboxState =
      new StateMessageBox(this,
                          std::string(GAMETEXT_ENTERREPLAYNAME) + ":",
                          UI_MSGBOX_OK | UI_MSGBOX_CANCEL,
                          true,
                          Replay::giveAutomaticName());
    v_msgboxState->setMsgBxId("SAVEREPLAY");
    StateManager::instance()->pushState(v_msgboxState);
  }

  UIButton *pViewreplayButton = reinterpret_cast<UIButton *>(
    m_GUI->getChild("FINISHED_FRAME:VIEWREPLAY_BUTTON"));
  if (pViewreplayButton->isClicked()) {
    pViewreplayButton->setClicked(false);

    if (m_universe->isAReplayToSave()) {
      m_universe->saveReplayTemporary(xmDatabase::instance("main"));
      StateManager::instance()->pushState(
        new StatePreplayingReplay(m_universe->getTemporaryReplayName(), true));
    }
  }

  UIButton *pUploadButton = reinterpret_cast<UIButton *>(
    m_GUI->getChild("FINISHED_FRAME:UPLOAD_BUTTON"));
  if (pUploadButton->isClicked()) {
    pUploadButton->setClicked(false);

    std::string v_replayPath = XMFS::getUserReplaysDir() + "/Latest.rpl";

    if (m_universe->isAnErrorOnSaving()) {
      // fake upload
      SysMessage::instance()->displayError(SYS_MSG_UNSAVABLE_LEVEL);
    } else {
      // normal upload
      StateManager::instance()->pushState(
        new StateUploadHighscore(v_replayPath));
    }
  }

  UIButton *pAbortButton = reinterpret_cast<UIButton *>(
    m_GUI->getChild("FINISHED_FRAME:ABORT_BUTTON"));
  if (pAbortButton->isClicked()) {
    pAbortButton->setClicked(false);

    m_requestForEnd = true;
    StateManager::instance()->sendAsynchronousMessage("FINISH", m_parentId);
  }

  UIButton *pQuitButton =
    reinterpret_cast<UIButton *>(m_GUI->getChild("FINISHED_FRAME:QUIT_BUTTON"));
  if (pQuitButton->isClicked()) {
    pQuitButton->setClicked(false);

    StateMessageBox *v_msgboxState = new StateMessageBox(
      this, GAMETEXT_QUITMESSAGE, UI_MSGBOX_YES | UI_MSGBOX_NO);
    v_msgboxState->setMsgBxId("QUIT");
    StateManager::instance()->pushState(v_msgboxState);
  }
}

void StateFinished::sendFromMessageBox(const std::string &i_id,
                                       UIMsgBoxButton i_button,
                                       const std::string &i_input) {
  if (i_id == "QUIT") {
    switch (i_button) {
      case UI_MSGBOX_YES:
        m_requestForEnd = true;
        GameApp::instance()->requestEnd();
        break;
      case UI_MSGBOX_NO:
      default:
        break;
    }
  } else if (i_id == "SAVEREPLAY") {
    if (i_button == UI_MSGBOX_OK) {
      if (m_universe != NULL) {
        m_replayName = i_input;
        addCommand("SAVEREPLAY");
      }
    }
  } else {
    StateMenu::sendFromMessageBox(i_id, i_button, i_input);
  }
}

void StateFinished::executeOneCommand(std::string cmd, std::string args) {
  LogDebug("cmd [%s [%s]] executed by state [%s].",
           cmd.c_str(),
           args.c_str(),
           getName().c_str());

  if (cmd == "SAVEREPLAY") {
    try {
      m_universe->saveReplay(xmDatabase::instance("main"), m_replayName);
    } catch (Exception &e) {
      StateManager::instance()->pushState(
        new StateMessageBox(NULL, e.getMsg(), UI_MSGBOX_OK));
    }
  } else {
    GameState::executeOneCommand(cmd, args);
  }
}

void StateFinished::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (i_type == INPUT_DOWN &&
      (i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE) ||
       i_xmkey.getJoyButton() == SDL_CONTROLLER_BUTTON_B)) {
    /* quit this state */
    m_requestForEnd = true;
    StateManager::instance()->sendAsynchronousMessage("FINISH", m_parentId);
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

void StateFinished::clean() {
  if (StateFinished::m_sGUI != NULL) {
    delete StateFinished::m_sGUI;
    StateFinished::m_sGUI = NULL;
  }
}

void StateFinished::createGUIIfNeeded(RenderSurface *i_screen) {
  if (m_sGUI != NULL)
    return;

  DrawLib *drawLib = GameApp::instance()->getDrawLib();

  UIFrame *v_frame;
  UIBestTimes *v_pBestTimes;
  UIButton *v_button;
  UIStatic *v_pFinishText;
  UIStatic *v_pNewHighscore_str;
  UIStatic *v_pMedal_str;
  UIStatic *v_pNewHighscoreSaved_str;
  UIStatic *v_pRoomsTimes_str;

  m_sGUI = new UIRoot(i_screen);
  m_sGUI->setFont(drawLib->getFontSmall());
  m_sGUI->setPosition(
    0, 0, i_screen->getDispWidth(), i_screen->getDispHeight());

  v_frame = new UIFrame(m_sGUI,
                        i_screen->getDispWidth() / 2 - 400 / 2,
                        i_screen->getDispHeight() / 2 - 680 / 2,
                        "",
                        400,
                        680);

  v_frame->setID("FINISHED_FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_MENU);

  v_pFinishText = new UIStatic(
    v_frame, 0, 140, GAMETEXT_FINISH, v_frame->getPosition().nWidth, 36);
  v_pFinishText->setFont(drawLib->getFontMedium());

  v_pBestTimes = new UIBestTimes(
    m_sGUI, 10, 50, "", 250, m_sGUI->getPosition().nHeight - 50 * 2);
  v_pBestTimes->setID("BESTTIMES");
  v_pBestTimes->setFont(drawLib->getFontMedium());
  v_pBestTimes->setHFont(drawLib->getFontMedium());

  int v_halign = 10;

  v_button = new UIButton(v_frame,
                          400 / 2 - 207 / 2,
                          v_halign + v_frame->getPosition().nHeight / 2 -
                            7 * 57 / 2 + 0 * 49 + 25,
                          GAMETEXT_TRYAGAIN,
                          207,
                          57);
  v_button->setID("TRYAGAIN_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_PLAY_THIS_LEVEL_AGAIN);
  v_button->setFont(drawLib->getFontSmall());
  v_frame->setPrimaryChild(v_button); /* default button */

  v_button = new UIButton(v_frame,
                          400 / 2 - 207 / 2,
                          v_halign + v_frame->getPosition().nHeight / 2 -
                            7 * 57 / 2 + 1 * 49 + 25,
                          GAMETEXT_PLAYNEXT,
                          207,
                          57);
  v_button->setID("PLAYNEXT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_PLAY_NEXT_LEVEL);
  v_button->setFont(drawLib->getFontSmall());

  v_button = new UIButton(v_frame,
                          400 / 2 - 207 / 2,
                          v_halign + v_frame->getPosition().nHeight / 2 -
                            7 * 57 / 2 + 2 * 49 + 25,
                          GAMETEXT_VIEWREPLAY,
                          207,
                          57);
  v_button->setID("VIEWREPLAY_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_VIEW_REPLAY);
  v_button->setFont(drawLib->getFontSmall());

  v_button = new UIButton(v_frame,
                          400 / 2 - 207 / 2,
                          v_halign + v_frame->getPosition().nHeight / 2 -
                            7 * 57 / 2 + 3 * 49 + 25,
                          GAMETEXT_SAVEREPLAY,
                          207,
                          57);
  v_button->setID("SAVEREPLAY_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_SAVE_A_REPLAY);
  v_button->setFont(drawLib->getFontSmall());

  v_button = new UIButton(v_frame,
                          400 / 2 - 207 / 2,
                          v_halign + v_frame->getPosition().nHeight / 2 -
                            7 * 57 / 2 + 4 * 49 + 25,
                          GAMETEXT_UPLOAD_HIGHSCORE,
                          207,
                          57);
  v_button->setID("UPLOAD_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_UPLOAD_HIGHSCORE);
  v_button->setFont(drawLib->getFontSmall());

  v_button = new UIButton(v_frame,
                          400 / 2 - 207 / 2,
                          v_halign + v_frame->getPosition().nHeight / 2 -
                            7 * 57 / 2 + 5 * 49 + 25,
                          GAMETEXT_ABORT,
                          207,
                          57);
  v_button->setID("ABORT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_BACK_TO_MAIN_MENU);
  v_button->setFont(drawLib->getFontSmall());

  v_button = new UIButton(v_frame,
                          400 / 2 - 207 / 2,
                          v_halign + v_frame->getPosition().nHeight / 2 -
                            7 * 57 / 2 + 6 * 49 + 25,
                          GAMETEXT_QUIT,
                          207,
                          57);
  v_button->setID("QUIT_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_QUIT_THE_GAME);
  v_button->setFont(drawLib->getFontSmall());

  v_pMedal_str = new UIStatic(m_sGUI,
                              0,
                              m_sGUI->getPosition().nHeight - 20 - 20 - 20,
                              "",
                              m_sGUI->getPosition().nWidth,
                              20);
  v_pMedal_str->setID("MEDALSTR_STATIC");
  v_pMedal_str->setFont(drawLib->getFontSmall());
  v_pMedal_str->setHAlign(UI_ALIGN_CENTER);

  v_pNewHighscore_str = new UIStatic(m_sGUI,
                                     0,
                                     m_sGUI->getPosition().nHeight - 20 - 20,
                                     "",
                                     m_sGUI->getPosition().nWidth,
                                     20);
  v_pNewHighscore_str->setID("HIGHSCORESTR_STATIC");
  v_pNewHighscore_str->setFont(drawLib->getFontSmall());
  v_pNewHighscore_str->setHAlign(UI_ALIGN_CENTER);

  v_pNewHighscoreSaved_str = new UIStatic(m_sGUI,
                                          0,
                                          m_sGUI->getPosition().nHeight - 20,
                                          "",
                                          m_sGUI->getPosition().nWidth,
                                          20);
  v_pNewHighscoreSaved_str->setID("HIGHSCORESAVEDSTR_STATIC");
  v_pNewHighscoreSaved_str->setFont(drawLib->getFontSmall());
  v_pNewHighscoreSaved_str->setHAlign(UI_ALIGN_CENTER);

  v_pRoomsTimes_str =
    new UIStatic(m_sGUI, m_sGUI->getPosition().nWidth - 300, 10, "", 290, 150);
  v_pRoomsTimes_str->setID("ROOMSTIMES_STATIC");
  v_pRoomsTimes_str->setFont(drawLib->getFontSmall());
  v_pRoomsTimes_str->setHAlign(UI_ALIGN_RIGHT);
  v_pRoomsTimes_str->setVAlign(UI_ALIGN_TOP);
}

void StateFinished::makeBestTimesWindow(UIBestTimes *pWindow,
                                        const std::string &PlayerName,
                                        const std::string &LevelID,
                                        int i_finishTime) {
  char **v_result;
  unsigned int nrow;
  int n1 = -1, n2 = -1;
  int v_finishTime;
  std::string v_profile;
  xmDatabase *i_db = xmDatabase::instance("main");

  pWindow->clear();

  v_result =
    i_db->readDB("SELECT finishTime, id_profile FROM profile_completedLevels "
                 "WHERE id_level=\"" +
                   xmDatabase::protectString(LevelID) +
                   "\" "
                   "ORDER BY 0+finishTime LIMIT 10;",
                 nrow);
  for (unsigned int i = 0; i < nrow; i++) {
    v_finishTime = atoi(i_db->getResult(v_result, 2, i, 0));
    v_profile = i_db->getResult(v_result, 2, i, 1);
    pWindow->addRow1(formatTime(v_finishTime), v_profile);
    if (v_profile == PlayerName && v_finishTime == i_finishTime)
      n1 = i;
  }
  i_db->read_DB_free(v_result);

  v_result = i_db->readDB("SELECT finishTime FROM profile_completedLevels "
                          "WHERE id_profile=\"" +
                            xmDatabase::protectString(PlayerName) +
                            "\" "
                            "AND   id_level=\"" +
                            xmDatabase::protectString(LevelID) +
                            "\" "
                            "ORDER BY 0+finishTime LIMIT 10;",
                          nrow);
  for (unsigned int i = 0; i < nrow; i++) {
    v_finishTime = atoi(i_db->getResult(v_result, 1, i, 0));
    pWindow->addRow2(formatTime(v_finishTime), PlayerName);
    if (v_finishTime == i_finishTime)
      n2 = i;
  }
  i_db->read_DB_free(v_result);

  pWindow->setup(GAMETEXT_BESTTIMES, n1, n2);
}
