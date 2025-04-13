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

#include "StateLevelPackViewer.h"
#include "StateLevelInfoViewer.h"
#include "StatePreplayingGame.h"
#include "StateViewHighscore.h"
#include "common/XMSession.h"
#include "drawlib/DrawLib.h"
#include "gui/specific/GUIXMoto.h"
#include "helpers/Log.h"
#include "helpers/Text.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"

/* static members */
UIRoot *StateLevelPackViewer::m_sGUI = NULL;

StateLevelPackViewer::StateLevelPackViewer(LevelsPack *pActiveLevelPack,
                                           bool drawStateBehind,
                                           bool updateStatesBehind)
  : StateMenu(drawStateBehind, updateStatesBehind) {
  m_name = "StateLevelPackViewer";
  m_pActiveLevelPack = pActiveLevelPack;
  m_require_updateLevelsList = false;

  StateManager::instance()->registerAsObserver("LEVELS_UPDATED", this);
  StateManager::instance()->registerAsObserver("HIGHSCORES_UPDATED", this);
  if (XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("FAVORITES_UPDATED");
  }
}

StateLevelPackViewer::~StateLevelPackViewer() {
  StateManager::instance()->unregisterAsObserver("LEVELS_UPDATED", this);
  StateManager::instance()->unregisterAsObserver("HIGHSCORES_UPDATED", this);
}

void StateLevelPackViewer::enter() {
  createGUIIfNeeded(&m_screen);
  m_GUI = m_sGUI;

  updateGUI();
  updateInfoFrame();
  updateRights(); // update rights after updateinfoframe because it can hide the
  // infoframe

  GameApp::instance()->playMenuMusic("menu1");
  StateMenu::enter();
}

void StateLevelPackViewer::enterAfterPop() {
  if (m_require_updateLevelsList) {
    updateGUI();
    m_require_updateLevelsList = false;
  }

  // reset the current playing list
  GameApp::instance()->setCurrentPlayingList(NULL);

  // the three buttons are disabled, so enable them
  updateRights();

  GameApp::instance()->playMenuMusic("menu1");
  StateMenu::enterAfterPop();
}

void StateLevelPackViewer::checkEvents() {
  /* Get buttons and list */
  UIButton *pCancelButton =
    reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:CANCEL_BUTTON"));
  UIButton *pPlayButton =
    reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:PLAY_BUTTON"));
  UIButton *pLevelInfoButton =
    reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:INFO_BUTTON"));
  UIButton *pLevelAddToFavoriteButton =
    reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:ADDTOFAVORITE_BUTTON"));
  UIButton *pLevelRandomizeButton =
    reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:RANDOMIZE_BUTTON"));
  UIButton *pShowHighscore = reinterpret_cast<UIButton *>(
    m_GUI->getChild("FRAME:INFO_FRAME:BESTPLAYER_VIEW"));
  UILevelList *pList =
    reinterpret_cast<UILevelList *>(m_GUI->getChild("FRAME:LEVEL_LIST"));
  UIEdit *pLevelFilterEdit =
    reinterpret_cast<UIEdit *>(m_GUI->getChild("FRAME:LEVEL_FILTER"));

  /* check filter */
  if (pLevelFilterEdit != NULL) {
    if (pLevelFilterEdit->hasChanged()) {
      pLevelFilterEdit->setHasChanged(false);
      pList->setFilter(pLevelFilterEdit->getCaption());
      updateInfoFrame();
      updateRights();
    }
  }

  /* Check buttons */
  if (pCancelButton != NULL && pCancelButton->isClicked()) {
    pCancelButton->setClicked(false);

    m_requestForEnd = true;
  }

  if (pPlayButton != NULL && pPlayButton->isClicked()) {
    pPlayButton->setClicked(false);
    std::string i_level = pList->getSelectedLevel();
    if (i_level != "") {
      GameApp::instance()->setCurrentPlayingList(pList);
      StateManager::instance()->pushState(
        new StatePreplayingGame(i_level, false));
    }
  }

  if (pLevelAddToFavoriteButton != NULL &&
      pLevelAddToFavoriteButton->isClicked()) {
    pLevelAddToFavoriteButton->setClicked(false);
    switchtoFavorites();
  }

  if (pLevelRandomizeButton != NULL && pLevelRandomizeButton->isClicked()) {
    pLevelRandomizeButton->setClicked(false);

    pList->randomize();
    updateInfoFrame();
    updateRights();
  }

  /* any list clicked ? */
  if (pList->isChanged()) {
    pList->setChanged(false);
    updateInfoFrame();
    updateRights();
  }

  if (pShowHighscore != NULL && pShowHighscore->isClicked() == true) {
    pShowHighscore->setClicked(false);
    GameApp::instance()->setCurrentPlayingList(pList);
    StateManager::instance()->pushState(
      new StateViewHighscore(getInfoFrameLevelId(), true));
  }

  if (pLevelInfoButton != NULL && pLevelInfoButton->isClicked()) {
    pLevelInfoButton->setClicked(false);

    std::string v_id_level = pList->getSelectedLevel();
    if (v_id_level != "") {
      StateManager::instance()->pushState(new StateLevelInfoViewer(v_id_level));
    }
  }
}

void StateLevelPackViewer::switchtoFavorites() {
  UILevelList *pList =
    reinterpret_cast<UILevelList *>(m_GUI->getChild("FRAME:LEVEL_LIST"));
  std::string v_id_level = pList->getSelectedLevel();

  if (v_id_level != "") {
    GameApp::instance()->switchLevelToFavorite(v_id_level, true);
    StateManager::instance()->sendAsynchronousMessage("FAVORITES_UPDATED");

    /* in case of the favorite package, the levels list must be updated */
    if (m_pActiveLevelPack->Name() == VPACKAGENAME_FAVORITE_LEVELS) {
      updateLevelsList();
    } else {
      updateRights();
    }
  }
}

void StateLevelPackViewer::executeOneCommand(std::string cmd,
                                             std::string args) {
  LogDebug("cmd [%s [%s]] executed by state [%s].",
           cmd.c_str(),
           args.c_str(),
           getName().c_str());

  if (cmd == "LEVELS_UPDATED" || cmd == "HIGHSCORES_UPDATED") {
    m_require_updateLevelsList = true;
  } else {
    GameState::executeOneCommand(cmd, args);
  }
}

void StateLevelPackViewer::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (i_type == INPUT_DOWN &&
      (i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE) ||
       i_xmkey.getJoyButton() == SDL_CONTROLLER_BUTTON_B)) {
    m_requestForEnd = true;
  }

  else if (i_type == INPUT_DOWN && i_xmkey == (*Input::instance()->getGlobalKey(
                                                INPUT_SWITCHFAVORITE))) {
    switchtoFavorites();
  }

  else {
    StateMenu::xmKey(i_type, i_xmkey);
  }
}

void StateLevelPackViewer::clean() {
  if (StateLevelPackViewer::m_sGUI != NULL) {
    delete StateLevelPackViewer::m_sGUI;
    StateLevelPackViewer::m_sGUI = NULL;
  }
}

void StateLevelPackViewer::createGUIIfNeeded(RenderSurface *i_screen) {
  if (m_sGUI != NULL)
    return;

  DrawLib *drawLib = GameApp::instance()->getDrawLib();

  unsigned int width = i_screen->getDispWidth();
  unsigned int height = i_screen->getDispHeight();

  m_sGUI = new UIRoot(i_screen);
  m_sGUI->setFont(drawLib->getFontSmall());
  m_sGUI->setPosition(0, 0, width, height);

  /* Initialize level pack viewer */
  unsigned int v_offsetX = width / 32;
  unsigned int v_offsetY = height / 32;
  unsigned int frameWidth = width - v_offsetX * 2;
  unsigned int frameHeight = height - v_offsetY * 2;

  std::string caption = "";
  UIFrame *v_frame;
  v_frame =
    new UIFrame(m_sGUI, v_offsetX, v_offsetY, caption, frameWidth, frameHeight);
  v_frame->setID("FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_TRANS);

  unsigned int levelListWidth = v_frame->getPosition().nWidth - 300;
  unsigned int leftMargin = 20;

  UIStatic *pLevelPackViewerTitle = new UIStatic(v_frame,
                                                 0,
                                                 0,
                                                 "(level pack name goes here)",
                                                 v_frame->getPosition().nWidth,
                                                 40);
  pLevelPackViewerTitle->setID("VIEWER_TITLE");
  pLevelPackViewerTitle->setFont(drawLib->getFontMedium());

  UIButton *pLevelPackPlay = new UIButton(v_frame,
                                          v_frame->getPosition().nWidth - 250,
                                          50,
                                          GAMETEXT_STARTLEVEL,
                                          207,
                                          57);
  pLevelPackPlay->setFont(drawLib->getFontSmall());
  pLevelPackPlay->setID("PLAY_BUTTON");
  pLevelPackPlay->setContextHelp(CONTEXTHELP_PLAY_SELECTED_LEVEL);

  UIStatic *pSomeText = new UIStatic(v_frame,
                                     (levelListWidth / 4) - 90,
                                     70,
                                     std::string(GAMETEXT_FILTER) + ":",
                                     90,
                                     25);
  pSomeText->setFont(drawLib->getFontSmall());
  pSomeText->setHAlign(UI_ALIGN_RIGHT);

  UIEdit *pLevelFilterEdit = new UIEdit(
    v_frame, leftMargin + levelListWidth / 4, 70, "", levelListWidth / 2, 25);
  pLevelFilterEdit->setFont(drawLib->getFontSmall());
  pLevelFilterEdit->setID("LEVEL_FILTER");
  pLevelFilterEdit->setContextHelp(CONTEXTHELP_LEVEL_FILTER);

  UILevelList *pLevelPackLevelList =
    new UILevelList(v_frame,
                    leftMargin,
                    100,
                    "",
                    levelListWidth,
                    v_frame->getPosition().nHeight - 130);
  pLevelPackLevelList->setFont(drawLib->getFontSmall());
  pLevelPackLevelList->setContextHelp(CONTEXTHELP_SELECT_LEVEL_IN_LEVEL_PACK);
  pLevelPackLevelList->setID("LEVEL_LIST");
  pLevelPackLevelList->setEnterButton(pLevelPackPlay);

  pSomeText = new UIStatic(v_frame,
                           leftMargin + levelListWidth / 2 - 200,
                           v_frame->getPosition().nHeight - 40,
                           "",
                           400,
                           50);
  pSomeText->setFont(drawLib->getFontSmall());
  pSomeText->setHAlign(UI_ALIGN_CENTER);
  pSomeText->setID("MINISTAT");
  pSomeText->setAllowContextHelp(true);
  pSomeText->setContextHelp(CONTEXTHELP_MINISTATTIMEFORPACKLEVEL);

  UIButton *pLevelPackInfo = new UIButton(v_frame,
                                          v_frame->getPosition().nWidth - 250,
                                          107,
                                          GAMETEXT_LEVELINFO,
                                          207,
                                          57);
  pLevelPackInfo->setFont(drawLib->getFontSmall());
  pLevelPackInfo->setID("INFO_BUTTON");
  pLevelPackInfo->setContextHelp(CONTEXTHELP_LEVEL_INFO);

  UIButton *pLevelPackAddToFavorite =
    new UIButton(v_frame,
                 v_frame->getPosition().nWidth - 250,
                 164,
                 GAMETEXT_ADDTOFAVORITE,
                 207,
                 57);
  pLevelPackAddToFavorite->setFont(drawLib->getFontSmall());
  pLevelPackAddToFavorite->setID("ADDTOFAVORITE_BUTTON");
  pLevelPackAddToFavorite->setContextHelp(CONTEXTHELP_ADDTOFAVORITE);

  UIButton *pLevelPackRandomize =
    new UIButton(v_frame,
                 v_frame->getPosition().nWidth - 250,
                 221,
                 GAMETEXT_RANDOMIZE,
                 207,
                 57);
  pLevelPackRandomize->setFont(drawLib->getFontSmall());
  pLevelPackRandomize->setID("RANDOMIZE_BUTTON");
  pLevelPackRandomize->setContextHelp(CONTEXTHELP_RANDOMIZE);

  UIButton *pLevelPackCancel = new UIButton(
    v_frame, v_frame->getPosition().nWidth - 250, 278, GAMETEXT_CLOSE, 207, 57);
  pLevelPackCancel->setFont(drawLib->getFontSmall());
  pLevelPackCancel->setID("CANCEL_BUTTON");
  pLevelPackCancel->setContextHelp(CONTEXTHELP_CLOSE_LEVEL_PACK);

  /* level info frame */
  UIWindow *v_infoFrame = new UIWindow(v_frame,
                                       v_frame->getPosition().nWidth - 220,
                                       v_frame->getPosition().nHeight - 100,
                                       "",
                                       220,
                                       100);
  v_infoFrame->showWindow(false);
  v_infoFrame->setID("INFO_FRAME");
  pSomeText = new UIStatic(v_infoFrame, 0, 5, "", 220, 50);
  pSomeText->setFont(drawLib->getFontSmall());
  pSomeText->setHAlign(UI_ALIGN_CENTER);
  pSomeText->setID("BESTPLAYER");
  UIButton *v_infoButton =
    new UIButton(v_infoFrame, 22, 40, GAMETEXT_VIEWTHEHIGHSCORE, 176, 40);
  v_infoButton->setFont(drawLib->getFontSmall());
  v_infoButton->setContextHelp(CONTEXTHELP_VIEWTHEHIGHSCORE);
  v_infoButton->setID("BESTPLAYER_VIEW");
}

void StateLevelPackViewer::updateLevelsList() {
  UILevelList *pList =
    reinterpret_cast<UILevelList *>(m_GUI->getChild("FRAME:LEVEL_LIST"));
  int v_selected = pList->getSelected();
  char **v_result;
  unsigned int nrow;
  int v_totalProfileTime = 0;
  int v_totalHighscoreTime = 0;

  int v_playerHighscore;
  int v_roomHighscore;
  xmDatabase *pDb = xmDatabase::instance("main");

  /* get selected item */
  std::string v_selected_levelName = "";
  if (pList->getSelected() >= 0 &&
      pList->getSelected() < pList->getEntries().size()) {
    UIListEntry *pEntry = pList->getEntries()[pList->getSelected()];
    v_selected_levelName = pEntry->Text[0];
  }

  pList->clear();

  // clear the filter
  UIEdit *pLevelFilterEdit =
    reinterpret_cast<UIEdit *>(m_GUI->getChild("FRAME:LEVEL_FILTER"));
  pLevelFilterEdit->setCaption("");
  pList->setFilter("");
  updateRights();

  /* Obey hints */
  pList->unhideAllColumns();
  if (!m_pActiveLevelPack->ShowTimes()) {
    pList->hideBestTime();
  }
  if (!m_pActiveLevelPack->ShowWebTimes()) {
    pList->hideRoomBestTime();
  }

  v_result = pDb->readDB(
    m_pActiveLevelPack->getLevelsWithHighscoresQuery(
      XMSession::instance()->profile(), XMSession::instance()->idRoom(0)),
    nrow);
  for (unsigned int i = 0; i < nrow; i++) {
    if (pDb->getResult(v_result, 4, i, 2) == NULL) {
      v_playerHighscore = -1;
    } else {
      v_playerHighscore = atoi(pDb->getResult(v_result, 4, i, 2));
      v_totalProfileTime += v_playerHighscore;
    }

    if (pDb->getResult(v_result, 4, i, 3) == NULL) {
      v_roomHighscore = -1;
      if (v_playerHighscore > 0) {
        v_totalHighscoreTime +=
          v_playerHighscore; // add player time in case he has a better score
        // than room, to not have to update www
      }
    } else {
      v_roomHighscore = atoi(pDb->getResult(v_result, 4, i, 3));
      if (v_playerHighscore > 0 && v_playerHighscore < v_roomHighscore) {
        v_totalHighscoreTime +=
          v_playerHighscore; // add player time in case he has a better score
        // than room, to not have to update www
      } else {
        if (v_playerHighscore > 0) {
          v_totalHighscoreTime +=
            v_roomHighscore; // only make sum on levels that the player finished
        }
      }
    }

    pList->addLevel(pDb->getResult(v_result, 4, i, 0),
                    pDb->getResult(v_result, 4, i, 1),
                    v_playerHighscore,
                    v_roomHighscore);
  }
  pDb->read_DB_free(v_result);

  /* reselect the previous level */
  if (v_selected_levelName != "") {
    int nLevel = -1;
    for (unsigned int i = 0; i < pList->getEntries().size(); i++) {
      if (pList->getEntries()[i]->Text[0] == v_selected_levelName) {
        nLevel = i;
        break;
      }
    }

    if (nLevel == -1) { // level not found, keep the same number in the list
      pList->setRealSelected(v_selected);
    } else {
      pList->setRealSelected(nLevel);
    }
  }

  /* ministat pack */
  UIStatic *pSomeText =
    reinterpret_cast<UIStatic *>(m_GUI->getChild("FRAME:MINISTAT"));
  pSomeText->setCaption(formatTime(v_totalProfileTime) + " / " +
                        formatTime(v_totalHighscoreTime));
}

void StateLevelPackViewer::updateGUI() {
  UIStatic *pTitle =
    reinterpret_cast<UIStatic *>(m_GUI->getChild("FRAME:VIEWER_TITLE"));
  pTitle->setCaption(m_pActiveLevelPack->Name());

  UILevelList *pList =
    reinterpret_cast<UILevelList *>(m_GUI->getChild("FRAME:LEVEL_LIST"));
  pList->setNumeroted(true);
  pList->makeActive();

  updateLevelsList();
}

void StateLevelPackViewer::updateInfoFrame() {
  UILevelList *v_list =
    reinterpret_cast<UILevelList *>(m_GUI->getChild("FRAME:LEVEL_LIST"));
  UIStatic *v_someText = reinterpret_cast<UIStatic *>(
    m_GUI->getChild("FRAME:INFO_FRAME:BESTPLAYER"));
  UIButton *v_button = reinterpret_cast<UIButton *>(
    m_GUI->getChild("FRAME:INFO_FRAME:BESTPLAYER_VIEW"));
  UIWindow *v_window =
    reinterpret_cast<UIWindow *>(m_GUI->getChild("FRAME:INFO_FRAME"));

  std::string v_id_level = v_list->getSelectedLevel();
  std::string v_id_profile;
  std::string v_url;
  bool v_isAccessible;

  if (v_list->nbVisibleItems() <= 0) {
    v_window->showWindow(false);
    return;
  }

  if (v_id_level != "") {
    if (GameApp::instance()->getHighscoreInfos(
          0, v_id_level, &v_id_profile, &v_url, &v_isAccessible)) {
      v_someText->setCaption(std::string(GAMETEXT_BESTPLAYER) + " : " +
                             v_id_profile);
      v_button->enableWindow(v_isAccessible);
      v_window->showWindow(true);
    } else {
      v_window->showWindow(false);
    }
  }
}

std::string StateLevelPackViewer::getInfoFrameLevelId() {
  UILevelList *v_list =
    reinterpret_cast<UILevelList *>(m_GUI->getChild("FRAME:LEVEL_LIST"));
  return v_list->getSelectedLevel();
}

void StateLevelPackViewer::updateRights() {
  UIButton *v_button;
  UILevelList *v_list =
    reinterpret_cast<UILevelList *>(m_GUI->getChild("FRAME:LEVEL_LIST"));
  std::string v_id_level = v_list->getSelectedLevel();
  xmDatabase *pDb = xmDatabase::instance("main");

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:PLAY_BUTTON"));
  v_button->enableWindow(v_list->nbVisibleItems() > 0);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:INFO_BUTTON"));
  v_button->enableWindow(v_list->nbVisibleItems() > 0);
  v_button =
    reinterpret_cast<UIButton *>(m_GUI->getChild("FRAME:ADDTOFAVORITE_BUTTON"));
  v_button->enableWindow(v_list->nbVisibleItems() > 0);

  if (v_id_level != "") {
    if (LevelsManager::instance()->isInFavorite(
          XMSession::instance()->profile(), v_id_level, pDb)) {
      v_button->setCaption(GAMETEXT_DELETEFROMFAVORITE);
    } else {
      v_button->setCaption(GAMETEXT_ADDTOFAVORITE);
    }
  }
}
