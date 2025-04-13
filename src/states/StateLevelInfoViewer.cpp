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

#include "StateLevelInfoViewer.h"
#include "StateMessageBox.h"
#include "StatePreplayingReplay.h"
#include "StateReplaying.h"
#include "common/VFileIO.h"
#include "common/XMSession.h"
#include "drawlib/DrawLib.h"
#include "helpers/Log.h"
#include "helpers/Text.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"
#include "xmoto/Replay.h"
#include "xmoto/SysMessage.h"

/* static members */
UIRoot *StateLevelInfoViewer::m_sGUI = NULL;

StateLevelInfoViewer::StateLevelInfoViewer(const std::string &level,
                                           bool drawStateBehind,
                                           bool updateStatesBehind)
  : StateMenu(drawStateBehind, updateStatesBehind) {
  m_level = level;
  m_name = "StateLevelInfoViewer";

  StateManager::instance()->registerAsObserver("REPLAYS_DELETE", this);
  StateManager::instance()->registerAsObserver("REPLAYS_UPDATED", this);
  if (XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("REPLAYS_UPDATED");
  }
}

StateLevelInfoViewer::~StateLevelInfoViewer() {
  StateManager::instance()->unregisterAsObserver("REPLAYS_DELETE", this);
  StateManager::instance()->unregisterAsObserver("REPLAYS_UPDATED", this);
}

void StateLevelInfoViewer::enter() {
  createGUIIfNeeded(&m_screen);
  m_GUI = m_sGUI;
  updateGUI();

  GameApp::instance()->playMenuMusic("menu1");

  StateMenu::enter();
}

void StateLevelInfoViewer::enterAfterPop() {
  GameApp::instance()->playMenuMusic("menu1");
  StateMenu::enterAfterPop();
}

void StateLevelInfoViewer::checkEvents() {
  UIButton *pOKButton = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_OK_BUTTON"));
  UIButton *pDeleteCacheFileBtn = reinterpret_cast<UIButton *>(m_GUI->getChild(
    "LEVEL_VIEWER_FRAME:LEVEL_VIEWER_DELETE_CACHE_FILE_BUTTON"));
  UIButton *pLV_BestTimes_Personal = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_PERSONAL"));
  UIButton *pLV_BestTimes_All = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_ALL"));
  UIButton *pLV_Info_CopyLevelPath = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "GENERALINFO_TAB:LEVEL_VIEWER_INFO_COPYLEVELPATH"));
  UIButton *pLV_Replays_Personal = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_"
                    "TAB:LEVEL_VIEWER_REPLAYS_PERSONAL"));
  UIButton *pLV_Replays_All = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_"
                    "TAB:LEVEL_VIEWER_REPLAYS_ALL"));
  UIButton *pLV_Replays_Show = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_"
                    "TAB:LEVEL_VIEWER_REPLAYS_SHOW"));
  UIButton *pLV_Replays_Delete = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_"
                    "TAB:LEVEL_VIEWER_REPLAYS_DELETE"));
  UIList *pLV_Replays_List = reinterpret_cast<UIList *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_"
                    "TAB:LEVEL_VIEWER_REPLAYS_LIST"));

  /* Check buttons */
  if (pOKButton->isClicked()) {
    pOKButton->setClicked(false);
    m_requestForEnd = true;
  }

  if (pDeleteCacheFileBtn->isClicked()) {
    pDeleteCacheFileBtn->setClicked(false);
    pDeleteCacheFileBtn->enableWindow(false);
    Level::removeFromCache(xmDatabase::instance("main"), m_level);
    SysMessage::instance()->displayInformation(GAMETEXT_LEVELCACHEFILEDELETED);
  }

  if (pLV_BestTimes_All->isClicked() || pLV_BestTimes_Personal->isClicked()) {
    pLV_BestTimes_All->setClicked(false);
    pLV_BestTimes_Personal->setClicked(false);
    updateLevelInfoViewerBestTimes();
  }

  if (pLV_Info_CopyLevelPath->isClicked()) {
    pLV_Info_CopyLevelPath->setClicked(false);

    if (SDL_SetClipboardText(m_levelFullPath.c_str()) != 0) {
      LogWarning("%s:\n%s", GAMETEXT_COPYTOCLIPBOARDFAILED, SDL_GetError());
    }
  }

  if (pLV_Replays_All->isClicked() || pLV_Replays_Personal->isClicked()) {
    pLV_Replays_All->setClicked(false);
    pLV_Replays_Personal->setClicked(false);
    updateLevelInfoViewerReplays();
  }

  if (pLV_Replays_Show->isClicked()) {
    pLV_Replays_Show->setClicked(false);
    /* Show replay */
    if (pLV_Replays_List->getSelected() >= 0 &&
        pLV_Replays_List->getSelected() <
          pLV_Replays_List->getEntries().size()) {
      UIListEntry *pListEntry =
        pLV_Replays_List->getEntries()[pLV_Replays_List->getSelected()];
      if (pListEntry != NULL && !pListEntry->Text.empty()) {
        /* Do it captain */
        std::string playSpecificReplay = pListEntry->Text[0];
        StateManager::instance()->pushState(
          new StatePreplayingReplay(playSpecificReplay, false));
      }
    }
  }

  if (pLV_Replays_Delete->isClicked()) {
    pLV_Replays_Delete->setClicked(false);
    /* delete replay ? */
    if (pLV_Replays_List->getSelected() >= 0 &&
        pLV_Replays_List->getSelected() <
          pLV_Replays_List->getEntries().size()) {
      UIListEntry *pListEntry =
        pLV_Replays_List->getEntries()[pLV_Replays_List->getSelected()];
      if (pListEntry != NULL && !pListEntry->Text.empty()) {
        /* Do it captain */
        std::string playSpecificReplay = pListEntry->Text[0];

        StateMessageBox *v_msgboxState = new StateMessageBox(
          this, GAMETEXT_DELETEREPLAYMESSAGE, UI_MSGBOX_YES | UI_MSGBOX_NO);
        StateManager::instance()->pushState(v_msgboxState);
        v_msgboxState->setMsgBxId("REPLAYS_DELETE");
        v_msgboxState->makeActiveButton(UI_MSGBOX_YES);
      }
    }
  }
}

void StateLevelInfoViewer::sendFromMessageBox(const std::string &i_id,
                                              UIMsgBoxButton i_button,
                                              const std::string &i_input) {
  if (i_id == "REPLAYS_DELETE") {
    if (i_button == UI_MSGBOX_YES) {
      addCommand("REPLAYS_DELETE");
    }
    return;
  } else {
    StateMenu::sendFromMessageBox(i_id, i_button, i_input);
  }
}

void StateLevelInfoViewer::executeOneCommand(std::string cmd,
                                             std::string args) {
  UIListEntry *pEntry = NULL;

  LogDebug("cmd [%s [%s]] executed by state [%s].",
           cmd.c_str(),
           args.c_str(),
           getName().c_str());

  if (cmd == "REPLAYS_DELETE") {
    UIList *v_list = reinterpret_cast<UIList *>(
      m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                      "REPLAYS_TAB:LEVEL_VIEWER_REPLAYS_LIST"));
    if (v_list->getSelected() >= 0 &&
        v_list->getSelected() < v_list->getEntries().size()) {
      pEntry = v_list->getEntries()[v_list->getSelected()];
      if (pEntry != NULL) {
        try {
          Replay::deleteReplay(pEntry->Text[0]);
        } catch (Exception &e) {
          LogError(e.getMsg().c_str());
        }
        try {
          xmDatabase::instance("main")->replays_delete(pEntry->Text[0]);
        } catch (Exception &e) {
          LogError(e.getMsg().c_str());
        }
        StateManager::instance()->sendAsynchronousMessage("REPLAYS_UPDATED");
        updateLevelInfoViewerReplays();
      }
    }
  } else if (cmd == "REPLAYS_UPDATED") {
    updateLevelInfoViewerReplays();
  } else {
    GameState::executeOneCommand(cmd, args);
  }
}

void StateLevelInfoViewer::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (i_type == INPUT_DOWN &&
      (i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE) ||
       i_xmkey.getJoyButton() == SDL_CONTROLLER_BUTTON_B)) {
    m_requestForEnd = true;
  }

  else {
    StateMenu::xmKey(i_type, i_xmkey);
  }
}

void StateLevelInfoViewer::clean() {
  if (StateLevelInfoViewer::m_sGUI != NULL) {
    delete StateLevelInfoViewer::m_sGUI;
    StateLevelInfoViewer::m_sGUI = NULL;
  }
}

void StateLevelInfoViewer::createGUIIfNeeded(RenderSurface *i_screen) {
  if (m_sGUI != NULL)
    return;

  DrawLib *drawLib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot(i_screen);
  m_sGUI->setFont(drawLib->getFontSmall());
  m_sGUI->setPosition(
    0, 0, i_screen->getDispWidth(), i_screen->getDispHeight());

  /* Initialize level info viewer */
  int x = i_screen->getDispWidth() / 2 - 350;
  int y = i_screen->getDispHeight() / 2 - 250;
  std::string caption = "";
  int nWidth = 700;
  int nHeight = 500;

  UIFrame *v_frame;
  v_frame = new UIFrame(m_sGUI, x, y, caption, nWidth, nHeight);
  v_frame->setID("LEVEL_VIEWER_FRAME");
  v_frame->setStyle(UI_FRAMESTYLE_TRANS);

  UIStatic *pLevelInfoViewerTitle =
    new UIStatic(v_frame, 0, 0, "(level name goes here)", 700, 40);
  pLevelInfoViewerTitle->setID("LEVEL_VIEWER_TITLE");
  pLevelInfoViewerTitle->setFont(drawLib->getFontMedium());

  UITabView *pLevelViewerTabs =
    new UITabView(v_frame,
                  20,
                  40,
                  "",
                  v_frame->getPosition().nWidth - 40,
                  v_frame->getPosition().nHeight - 115);
  pLevelViewerTabs->setFont(drawLib->getFontSmall());
  pLevelViewerTabs->setID("LEVEL_VIEWER_TABS");
  pLevelViewerTabs->setTabContextHelp(0, CONTEXTHELP_GENERAL_INFO);
  pLevelViewerTabs->setTabContextHelp(1, CONTEXTHELP_BEST_TIMES_INFO);
  pLevelViewerTabs->setTabContextHelp(2, CONTEXTHELP_REPLAYS_INFO);

  UIWindow *pLVTab_Info =
    new UIWindow(pLevelViewerTabs,
                 20,
                 40,
                 GAMETEXT_GENERALINFO,
                 pLevelViewerTabs->getPosition().nWidth - 40,
                 pLevelViewerTabs->getPosition().nHeight - 60);
  pLVTab_Info->enableWindow(true);
  pLVTab_Info->setID("LEVEL_VIEWER_GENERALINFO_TAB");

  UIWindow *pLVTab_BestTimes =
    new UIWindow(pLevelViewerTabs,
                 20,
                 40,
                 GAMETEXT_BESTTIMES,
                 pLevelViewerTabs->getPosition().nWidth - 40,
                 pLevelViewerTabs->getPosition().nHeight - 60);
  pLVTab_BestTimes->enableWindow(true);
  pLVTab_BestTimes->showWindow(false);
  pLVTab_BestTimes->setID("LEVEL_VIEWER_BESTTIMES_TAB");

  UIWindow *pLVTab_Replays =
    new UIWindow(pLevelViewerTabs,
                 20,
                 40,
                 GAMETEXT_REPLAYS,
                 pLevelViewerTabs->getPosition().nWidth - 40,
                 pLevelViewerTabs->getPosition().nHeight - 60);
  pLVTab_Replays->enableWindow(true);
  pLVTab_Replays->showWindow(false);
  pLVTab_Replays->setID("LEVEL_VIEWER_REPLAYS_TAB");

  UIButton *pOKButton = new UIButton(
    v_frame, 11, v_frame->getPosition().nHeight - 68, GAMETEXT_OK, 115, 57);
  pOKButton->setFont(drawLib->getFontSmall());
  pOKButton->setType(UI_BUTTON_TYPE_SMALL);
  pOKButton->setID("LEVEL_VIEWER_OK_BUTTON");
  pOKButton->setContextHelp(CONTEXTHELP_BACK_TO_MAIN_MENU);

  UIButton *pDeleteCacheFileBtn =
    new UIButton(v_frame,
                 v_frame->getPosition().nWidth - 115 * 2 - 10,
                 v_frame->getPosition().nHeight - 68,
                 GAMETEXT_DELETELEVELCACHEFILE,
                 115 * 2,
                 57);
  pDeleteCacheFileBtn->enableWindow(false);
  pDeleteCacheFileBtn->setFont(drawLib->getFontSmall());
  pDeleteCacheFileBtn->setType(UI_BUTTON_TYPE_LARGE);
  pDeleteCacheFileBtn->setID("LEVEL_VIEWER_DELETE_CACHE_FILE_BUTTON");
  pDeleteCacheFileBtn->setContextHelp(CONTEXTHELP_DELETE_CACHED_LEVEL_FILE);

  /* Level info viewer - general info */
  UIStatic *pLV_Info_LevelPack = new UIStatic(pLVTab_Info,
                                              0,
                                              0,
                                              "(pack name goes here)",
                                              pLVTab_Info->getPosition().nWidth,
                                              40);
  pLV_Info_LevelPack->setID("LEVEL_VIEWER_INFO_LEVELPACK");
  pLV_Info_LevelPack->showWindow(true);
  pLV_Info_LevelPack->setHAlign(UI_ALIGN_LEFT);
  pLV_Info_LevelPack->setVAlign(UI_ALIGN_TOP);
  pLV_Info_LevelPack->setFont(drawLib->getFontSmall());

  UIStatic *pLV_Info_LevelName = new UIStatic(pLVTab_Info,
                                              0,
                                              40,
                                              "(level name goes here)",
                                              pLVTab_Info->getPosition().nWidth,
                                              40);
  pLV_Info_LevelName->setID("LEVEL_VIEWER_INFO_LEVELNAME");
  pLV_Info_LevelName->showWindow(true);
  pLV_Info_LevelName->setHAlign(UI_ALIGN_LEFT);
  pLV_Info_LevelName->setVAlign(UI_ALIGN_TOP);
  pLV_Info_LevelName->setFont(drawLib->getFontSmall());

  UIStatic *pLV_Info_LevelId =
    new UIStatic(pLVTab_Info, 0, 80, "", pLVTab_Info->getPosition().nWidth, 40);
  pLV_Info_LevelId->setID("LEVEL_VIEWER_INFO_LEVELID");
  pLV_Info_LevelId->showWindow(true);
  pLV_Info_LevelId->setHAlign(UI_ALIGN_LEFT);
  pLV_Info_LevelId->setVAlign(UI_ALIGN_TOP);
  pLV_Info_LevelId->setFont(drawLib->getFontSmall());

  UIStatic *pLV_Info_LevelFile = new UIStatic(
    pLVTab_Info, 0, 120, "", pLVTab_Info->getPosition().nWidth, 40);
  pLV_Info_LevelFile->setID("LEVEL_VIEWER_INFO_LEVELFILE");
  pLV_Info_LevelFile->showWindow(true);
  pLV_Info_LevelFile->setHAlign(UI_ALIGN_LEFT);
  pLV_Info_LevelFile->setVAlign(UI_ALIGN_TOP);
  pLV_Info_LevelFile->setFont(drawLib->getFontSmall());

  UIButton *pLV_Info_CopyLevelPath =
    new UIButton(pLVTab_Info, 0, 160 - 28 + 6, GAMETEXT_LEVELCOPYPATH, 160, 40);
  pLV_Info_CopyLevelPath->setID("LEVEL_VIEWER_INFO_COPYLEVELPATH");
  pLV_Info_CopyLevelPath->enableWindow(false);
  pLV_Info_CopyLevelPath->setType(UI_BUTTON_TYPE_LARGE);
  pLV_Info_CopyLevelPath->setFont(drawLib->getFontSmall());

  UIStatic *pLV_Info_Author = new UIStatic(pLVTab_Info,
                                           0,
                                           200,
                                           "(author goes here)",
                                           pLVTab_Info->getPosition().nWidth,
                                           40);
  pLV_Info_Author->setID("LEVEL_VIEWER_INFO_AUTHOR");
  pLV_Info_Author->showWindow(true);
  pLV_Info_Author->setHAlign(UI_ALIGN_LEFT);
  pLV_Info_Author->setVAlign(UI_ALIGN_TOP);
  pLV_Info_Author->setFont(drawLib->getFontSmall());

  UIStatic *pLV_Info_Date = new UIStatic(pLVTab_Info,
                                         0,
                                         240,
                                         "(date goes here)",
                                         pLVTab_Info->getPosition().nWidth,
                                         40);
  pLV_Info_Date->setID("LEVEL_VIEWER_INFO_DATE");
  pLV_Info_Date->showWindow(true);
  pLV_Info_Date->setHAlign(UI_ALIGN_LEFT);
  pLV_Info_Date->setVAlign(UI_ALIGN_TOP);
  pLV_Info_Date->setFont(drawLib->getFontSmall());

  UIStatic *pLV_Info_Description =
    new UIStatic(pLVTab_Info,
                 0,
                 280,
                 "(description goes here)",
                 pLVTab_Info->getPosition().nWidth,
                 200);
  pLV_Info_Description->setID("LEVEL_VIEWER_INFO_DESCRIPTION");
  pLV_Info_Description->showWindow(true);
  pLV_Info_Description->setHAlign(UI_ALIGN_LEFT);
  pLV_Info_Description->setVAlign(UI_ALIGN_TOP);
  pLV_Info_Description->setFont(drawLib->getFontSmall());

  /* Level info viewer - best times */
  UIButton *pLV_BestTimes_Personal =
    new UIButton(pLVTab_BestTimes,
                 5,
                 5,
                 GAMETEXT_PERSONAL,
                 (pLVTab_BestTimes->getPosition().nWidth - 40) / 2,
                 28);
  pLV_BestTimes_Personal->setType(UI_BUTTON_TYPE_RADIO);
  pLV_BestTimes_Personal->setID("LEVEL_VIEWER_BESTTIMES_PERSONAL");
  pLV_BestTimes_Personal->enableWindow(true);
  pLV_BestTimes_Personal->setChecked(true);
  pLV_BestTimes_Personal->setFont(drawLib->getFontSmall());
  pLV_BestTimes_Personal->setGroup(421023);
  pLV_BestTimes_Personal->setContextHelp(CONTEXTHELP_ONLY_SHOW_PERSONAL_BESTS);

  UIButton *pLV_BestTimes_All =
    new UIButton(pLVTab_BestTimes,
                 5 + ((pLVTab_BestTimes->getPosition().nWidth - 40) / 2) * 1,
                 5,
                 GAMETEXT_ALL,
                 (pLVTab_BestTimes->getPosition().nWidth - 40) / 2,
                 28);
  pLV_BestTimes_All->setType(UI_BUTTON_TYPE_RADIO);
  pLV_BestTimes_All->setID("LEVEL_VIEWER_BESTTIMES_ALL");
  pLV_BestTimes_All->enableWindow(true);
  pLV_BestTimes_All->setChecked(false);
  pLV_BestTimes_All->setFont(drawLib->getFontSmall());
  pLV_BestTimes_All->setGroup(421023);
  pLV_BestTimes_All->setContextHelp(CONTEXTHELP_SHOW_ALL_BESTS);

  UIList *pLV_BestTimes_List =
    new UIList(pLVTab_BestTimes,
               5,
               43,
               "",
               pLVTab_BestTimes->getPosition().nWidth - 10,
               pLVTab_BestTimes->getPosition().nHeight - 100);
  pLV_BestTimes_List->setID("LEVEL_VIEWER_BESTTIMES_LIST");
  pLV_BestTimes_List->setFont(drawLib->getFontSmall());
  pLV_BestTimes_List->addColumn(GAMETEXT_FINISHTIME, 128);
  pLV_BestTimes_List->addColumn(GAMETEXT_PLAYER,
                                pLV_BestTimes_List->getPosition().nWidth - 128);

  UIStatic *pLV_BestTimes_WorldRecord =
    new UIStatic(pLVTab_BestTimes,
                 5,
                 pLVTab_BestTimes->getPosition().nHeight - 50,
                 "",
                 pLVTab_BestTimes->getPosition().nWidth,
                 50);
  pLV_BestTimes_WorldRecord->setID("LEVEL_VIEWER_BESTTIMES_WORLDRECORD");
  pLV_BestTimes_WorldRecord->setFont(drawLib->getFontSmall());
  pLV_BestTimes_WorldRecord->setVAlign(UI_ALIGN_CENTER);
  pLV_BestTimes_WorldRecord->setHAlign(UI_ALIGN_LEFT);

  /* Level info viewer - replays */
  UIButton *pLV_Replays_Personal =
    new UIButton(pLVTab_Replays,
                 5,
                 5,
                 GAMETEXT_PERSONAL,
                 (pLVTab_Replays->getPosition().nWidth - 40) / 2,
                 28);
  pLV_Replays_Personal->setType(UI_BUTTON_TYPE_RADIO);
  pLV_Replays_Personal->setID("LEVEL_VIEWER_REPLAYS_PERSONAL");
  pLV_Replays_Personal->enableWindow(true);
  pLV_Replays_Personal->setChecked(false);
  pLV_Replays_Personal->setFont(drawLib->getFontSmall());
  pLV_Replays_Personal->setGroup(421024);
  pLV_Replays_Personal->setContextHelp(CONTEXTHELP_ONLY_SHOW_PERSONAL_REPLAYS);

  UIButton *pLV_Replays_All =
    new UIButton(pLVTab_Replays,
                 5 + ((pLVTab_Replays->getPosition().nWidth - 40) / 2) * 1,
                 5,
                 GAMETEXT_ALL,
                 (pLVTab_Replays->getPosition().nWidth - 40) / 2,
                 28);
  pLV_Replays_All->setType(UI_BUTTON_TYPE_RADIO);
  pLV_Replays_All->setID("LEVEL_VIEWER_REPLAYS_ALL");
  pLV_Replays_All->enableWindow(true);
  pLV_Replays_All->setChecked(true);
  pLV_Replays_All->setFont(drawLib->getFontSmall());
  pLV_Replays_All->setGroup(421024);
  pLV_Replays_All->setContextHelp(CONTEXTHELP_SHOW_ALL_REPLAYS);

  UIList *pLV_Replays_List =
    new UIList(pLVTab_Replays,
               5,
               43,
               "",
               pLVTab_Replays->getPosition().nWidth - 10,
               pLVTab_Replays->getPosition().nHeight - 100);
  pLV_Replays_List->setID("LEVEL_VIEWER_REPLAYS_LIST");
  pLV_Replays_List->setFont(drawLib->getFontSmall());
  pLV_Replays_List->addColumn(GAMETEXT_REPLAY, 128);
  pLV_Replays_List->addColumn(GAMETEXT_PLAYER, 128);
  pLV_Replays_List->addColumn(GAMETEXT_FINISHTIME, 128);

  UIButton *pLV_Replays_Show =
    new UIButton(pLVTab_Replays,
                 0,
                 pLVTab_Replays->getPosition().nHeight - 50,
                 GAMETEXT_SHOW,
                 115,
                 57);
  pLV_Replays_Show->setFont(drawLib->getFontSmall());
  pLV_Replays_Show->setType(UI_BUTTON_TYPE_SMALL);
  pLV_Replays_Show->setID("LEVEL_VIEWER_REPLAYS_SHOW");
  pLV_Replays_Show->setContextHelp(CONTEXTHELP_RUN_SELECTED_REPLAY);
  pLV_Replays_List->setEnterButton(pLV_Replays_Show);

  UIButton *pLV_Replays_Delete =
    new UIButton(pLVTab_Replays,
                 115,
                 pLVTab_Replays->getPosition().nHeight - 50,
                 GAMETEXT_DELETE,
                 115,
                 57);
  pLV_Replays_Delete->setFont(drawLib->getFontSmall());
  pLV_Replays_Delete->setType(UI_BUTTON_TYPE_SMALL);
  pLV_Replays_Delete->setID("LEVEL_VIEWER_REPLAYS_DELETE");
  pLV_Replays_Delete->setContextHelp(CONTEXTHELP_DELETE_REPLAY);
}

void StateLevelInfoViewer::updateGUI() {
  char **v_result;
  unsigned int nrow;
  std::string v_levelId = m_level;
  std::string v_levelFile;
  std::string v_levelName;
  std::string v_levelDescription;
  std::string v_levelAuthor;
  std::string v_levelPack;
  std::string v_levelDateStr;

  v_result = xmDatabase::instance("main")->readDB(
    "SELECT a.name, a.filepath, a.author, a.description, b.packName, "
    "a.date_str "
    "FROM levels AS a LEFT OUTER JOIN weblevels AS b ON "
    "(a.id_level=b.id_level) WHERE a.id_level=\"" +
      xmDatabase::protectString(v_levelId) + "\";",
    nrow);
  if (nrow == 0) {
    xmDatabase::instance("main")->read_DB_free(v_result);
    return;
  }

  v_levelName = xmDatabase::instance("main")->getResult(v_result, 6, 0, 0);
  v_levelFile = xmDatabase::instance("main")->getResult(v_result, 6, 0, 1);
  v_levelAuthor = xmDatabase::instance("main")->getResult(v_result, 6, 0, 2);
  v_levelDescription =
    xmDatabase::instance("main")->getResult(v_result, 6, 0, 3);
  v_levelPack =
    xmDatabase::instance("main")->getResult(v_result, 6, 0, 4) == NULL
      ? ""
      : xmDatabase::instance("main")->getResult(v_result, 6, 0, 4);
  v_levelDateStr = xmDatabase::instance("main")->getResult(v_result, 6, 0, 5);
  xmDatabase::instance("main")->read_DB_free(v_result);

  bool levelInCache = Level::isInCache(xmDatabase::instance("main"), m_level);

  if (XMFS::isPathAbsolute(v_levelFile) &&
      XMFS::doesRealFileOrDirectoryExists(v_levelFile) &&
      !XMFS::isDir(v_levelFile)) {
    m_levelFullPath = v_levelFile;
  }

  /* Set information */

  UIStatic *pLevelName = reinterpret_cast<UIStatic *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TITLE"));
  if (pLevelName != NULL) {
    pLevelName->setCaption(v_levelName);
  }

  UIButton *pDeleteCacheFileBtn = reinterpret_cast<UIButton *>(m_GUI->getChild(
    "LEVEL_VIEWER_FRAME:LEVEL_VIEWER_DELETE_CACHE_FILE_BUTTON"));
  pDeleteCacheFileBtn->enableWindow(levelInCache);

  UIStatic *pGeneralInfo_LevelPack = reinterpret_cast<UIStatic *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "GENERALINFO_TAB:LEVEL_VIEWER_INFO_LEVELPACK"));
  UIStatic *pGeneralInfo_LevelName = reinterpret_cast<UIStatic *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "GENERALINFO_TAB:LEVEL_VIEWER_INFO_LEVELNAME"));
  UIStatic *pGeneralInfo_LevelId = reinterpret_cast<UIStatic *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "GENERALINFO_TAB:LEVEL_VIEWER_INFO_LEVELID"));
  UIStatic *pGeneralInfo_LevelFile = reinterpret_cast<UIStatic *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "GENERALINFO_TAB:LEVEL_VIEWER_INFO_LEVELFILE"));
  UIButton *pGeneralInfo_CopyLevelPath = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "GENERALINFO_TAB:LEVEL_VIEWER_INFO_COPYLEVELPATH"));
  UIStatic *pGeneralInfo_Author = reinterpret_cast<UIStatic *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "GENERALINFO_TAB:LEVEL_VIEWER_INFO_AUTHOR"));
  UIStatic *pGeneralInfo_Date = reinterpret_cast<UIStatic *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "GENERALINFO_TAB:LEVEL_VIEWER_INFO_DATE"));
  UIStatic *pGeneralInfo_Description = reinterpret_cast<UIStatic *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "GENERALINFO_TAB:LEVEL_VIEWER_INFO_DESCRIPTION"));

  if (pGeneralInfo_LevelPack != NULL)
    pGeneralInfo_LevelPack->setCaption(std::string(GAMETEXT_LEVELPACK) + ": " +
                                       v_levelPack);

  if (pGeneralInfo_LevelName != NULL)
    pGeneralInfo_LevelName->setCaption(std::string(GAMETEXT_LEVELNAME) + ": " +
                                       v_levelName);

  if (pGeneralInfo_LevelId != NULL)
    pGeneralInfo_LevelId->setCaption(std::string(GAMETEXT_LEVELID) + ": " +
                                     v_levelId);

  if (pGeneralInfo_LevelFile != NULL)
    pGeneralInfo_LevelFile->setCaption(
      std::string(GAMETEXT_LEVELFILE) + ": " +
      (m_levelFullPath == "" ? GAMETEXT_LEVELPATHBUILTIN : m_levelFullPath));

  if (pGeneralInfo_CopyLevelPath != NULL)
    pGeneralInfo_CopyLevelPath->enableWindow(m_levelFullPath != "");

  if (pGeneralInfo_Author != NULL)
    pGeneralInfo_Author->setCaption(std::string(GAMETEXT_AUTHOR) + ": " +
                                    v_levelAuthor);

  if (pGeneralInfo_Date != NULL)
    pGeneralInfo_Date->setCaption(std::string(GAMETEXT_DATE) + ": " +
                                  v_levelDateStr);

  if (pGeneralInfo_Description != NULL) {
    if (v_levelDescription.size() <= 50) {
      pGeneralInfo_Description->setCaption(std::string(GAMETEXT_DESCRIPTION) +
                                           ": " + v_levelDescription);
    } else { // split too large description
      pGeneralInfo_Description->setCaption(std::string(GAMETEXT_DESCRIPTION) +
                                           ":\n" +
                                           splitText(v_levelDescription, 50));
    }
  }

  updateLevelInfoViewerBestTimes();
  updateLevelInfoViewerReplays();
}

void StateLevelInfoViewer::updateLevelInfoViewerBestTimes() {
  char **v_result;
  unsigned int nrow;
  int v_finishTime;
  std::string v_profile;

  UIList *pList = reinterpret_cast<UIList *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_LIST"));
  UIButton *pLV_BestTimes_Personal = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_PERSONAL"));
  UIButton *pLV_BestTimes_All = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_ALL"));
  UIStatic *pLV_BestTimes_WorldRecord = reinterpret_cast<UIStatic *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_WORLDRECORD"));

  if (pList != NULL && pLV_BestTimes_All != NULL &&
      pLV_BestTimes_Personal != NULL &&
      XMSession::instance()->profile() != "" &&
      pLV_BestTimes_WorldRecord != NULL) {
    /* Create list */
    pList->clear();
    if (pLV_BestTimes_All->getChecked()) {
      v_result = xmDatabase::instance("main")->readDB(
        "SELECT finishTime, id_profile FROM profile_completedLevels "
        "WHERE id_level=\"" +
          xmDatabase::protectString(m_level) +
          "\" "
          "ORDER BY finishTime+0 LIMIT 10;",
        nrow);
      for (unsigned int i = 0; i < nrow; i++) {
        v_finishTime =
          atoi(xmDatabase::instance("main")->getResult(v_result, 2, i, 0));
        v_profile = xmDatabase::instance("main")->getResult(v_result, 2, i, 1);

        UIListEntry *pEntry = pList->addEntry(formatTime(v_finishTime));
        pEntry->Text.push_back(v_profile);
      }
      xmDatabase::instance("main")->read_DB_free(v_result);
    } else {
      v_result = xmDatabase::instance("main")->readDB(
        "SELECT finishTime FROM profile_completedLevels "
        "WHERE id_profile=\"" +
          xmDatabase::protectString(XMSession::instance()->profile()) +
          "\" "
          "AND   id_level=\"" +
          xmDatabase::protectString(m_level) +
          "\" "
          "ORDER BY finishTime+0 LIMIT 10;",
        nrow);
      for (unsigned int i = 0; i < nrow; i++) {
        v_finishTime =
          atoi(xmDatabase::instance("main")->getResult(v_result, 1, i, 0));
        UIListEntry *pEntry = pList->addEntry(formatTime(v_finishTime));
        pEntry->Text.push_back(XMSession::instance()->profile());
      }
      xmDatabase::instance("main")->read_DB_free(v_result);
    }

    /* Get record */
    if (XMSession::instance()->www()) {
      char **v_result;
      unsigned int nrow;
      std::string v_roomName;
      std::string v_id_profile;
      int v_finishTime = 0;

      v_result = xmDatabase::instance("main")->readDB(
        "SELECT a.name, b.id_profile, b.finishTime "
        "FROM webrooms AS a LEFT OUTER JOIN webhighscores AS b "
        "ON (a.id_room = b.id_room "
        "AND b.id_level=\"" +
          xmDatabase::protectString(m_level) +
          "\") "
          "WHERE a.id_room=" +
          XMSession::instance()->idRoom(0) + ";",
        nrow);
      if (nrow != 1) {
        pLV_BestTimes_WorldRecord->setCaption("");
        return;
      }
      v_roomName = xmDatabase::instance("main")->getResult(v_result, 3, 0, 0);
      if (xmDatabase::instance("main")->getResult(v_result, 3, 0, 1) != NULL) {
        v_id_profile =
          xmDatabase::instance("main")->getResult(v_result, 3, 0, 1);
        v_finishTime =
          atoi(xmDatabase::instance("main")->getResult(v_result, 3, 0, 2));
      }
      xmDatabase::instance("main")->read_DB_free(v_result);

      if (v_id_profile != "") {
        char c_tmp[1024];
        snprintf(c_tmp, 1024, GAMETEXT_BY_PLAYER, v_id_profile.c_str());
        pLV_BestTimes_WorldRecord->setCaption(v_roomName + ": " +
                                              formatTime(v_finishTime) + " " +
                                              std::string(c_tmp));
      } else {
        pLV_BestTimes_WorldRecord->setCaption(v_roomName + ": " +
                                              GAMETEXT_WORLDRECORDNA);
      }
    } else
      pLV_BestTimes_WorldRecord->setCaption("");
  }
}

void StateLevelInfoViewer::updateLevelInfoViewerReplays() {
  UIList *pList = reinterpret_cast<UIList *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_"
                    "TAB:LEVEL_VIEWER_REPLAYS_LIST"));
  UIButton *pLV_BestTimes_Personal = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_PERSONAL"));
  UIButton *pLV_BestTimes_All = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_"
                    "BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_ALL"));
  UIButton *pLV_Replays_Personal = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_"
                    "TAB:LEVEL_VIEWER_REPLAYS_PERSONAL"));
  UIButton *pLV_Replays_All = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_"
                    "TAB:LEVEL_VIEWER_REPLAYS_ALL"));
  UIButton *pLV_Replays_Show = reinterpret_cast<UIButton *>(
    m_GUI->getChild("LEVEL_VIEWER_FRAME:LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_"
                    "TAB:LEVEL_VIEWER_REPLAYS_SHOW"));
  int v_selected;

  if (pList != NULL && pLV_BestTimes_All != NULL &&
      pLV_BestTimes_Personal != NULL &&
      XMSession::instance()->profile() != "" && pLV_Replays_Show != NULL) {
    char **v_result;
    unsigned int nrow;

    /* Personal or all replays? */
    std::string v_sql;

    if (pLV_Replays_All->getChecked()) {
      v_sql = "SELECT name, id_profile, isFinished, finishTime FROM replays "
              "WHERE id_level=\"" +
              xmDatabase::protectString(m_level) + "\";";
    } else if (pLV_Replays_Personal->getChecked()) {
      v_sql = "SELECT name, id_profile, isFinished, finishTime FROM replays "
              "WHERE id_level=\"" +
              xmDatabase::protectString(m_level) +
              "\" "
              "AND   id_profile=\"" +
              xmDatabase::protectString(XMSession::instance()->profile()) +
              "\";";
    }

    /* Create list */
    v_selected = pList->getSelected();
    pList->clear();

    v_result = xmDatabase::instance("main")->readDB(v_sql, nrow);
    for (unsigned int i = 0; i < nrow; i++) {
      UIListEntry *pEntry = pList->addEntry(
        xmDatabase::instance("main")->getResult(v_result, 4, i, 0));
      pEntry->Text.push_back(
        xmDatabase::instance("main")->getResult(v_result, 4, i, 1));

      if (atoi(xmDatabase::instance("main")->getResult(v_result, 4, i, 2)) >
          0) {
        pEntry->Text.push_back(formatTime(
          atoi(xmDatabase::instance("main")->getResult(v_result, 4, i, 3))));
      } else {
        pEntry->Text.push_back("(" + std::string(GAMETEXT_NOTFINISHED) + ")");
      }
    }
    xmDatabase::instance("main")->read_DB_free(v_result);
    pList->setRealSelected(v_selected);

    /* Clean up */
    pLV_Replays_Personal->enableWindow(true);
    pLV_Replays_All->enableWindow(true);
    pLV_Replays_Show->enableWindow(true);
    pList->enableWindow(true);
  }
}
