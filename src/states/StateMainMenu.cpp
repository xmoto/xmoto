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

#include "StateMainMenu.h"
#include "../Game.h"
#include "../drawlib/DrawLib.h"
#include "../GameText.h"
#include "../gui/specific/GUIXMoto.h"
#include "../XMSession.h"
#include "StatePreplayingGame.h"
#include "StatePreplayingReplay.h"
#include "StateLevelInfoViewer.h"
#include "StateMessageBox.h"
#include "StateHelp.h"
#include "StateOptions.h"
#include "StateEditProfile.h"
#include "StateReplaying.h"
#include "StateLevelPackViewer.h"
#include "StateUploadHighscore.h"
#include "StateCheckWww.h"
#include "StateUpgradeLevels.h"
#include "StateDownloadGhost.h"
#include "../LevelsManager.h"
#include "../helpers/Log.h"
#include "../helpers/Text.h"
#include "../Sound.h"
#include "../thread/CheckWwwThread.h"
#include "../Replay.h"
#include "../helpers/CmdArgumentParser.h"
#include <sstream>
#include "../thread/LevelsPacksCountUpdateThread.h"
#include "../SysMessage.h"

/* static members */
UIRoot*  StateMainMenu::m_sGUI = NULL;

StateMainMenu::StateMainMenu(bool drawStateBehind,
			     bool updateStatesBehind):
  StateMenu(drawStateBehind,
	    updateStatesBehind)
{
  Sprite* pSprite;

  m_name    = "StateMainMenu";
  m_quickStartList = NULL;
  m_levelsPacksCountThread = NULL;
  m_checkWwwThread         = NULL;

  /* Load title screen textures */
  m_pTitleBL = NULL;
  pSprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "TitleBL");
  if(pSprite != NULL) {
    m_pTitleBL = pSprite->getTexture(false, true, FM_LINEAR);
  }

  m_pTitleBR = NULL;
  pSprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "TitleBR");
  if(pSprite != NULL) {
    m_pTitleBR = pSprite->getTexture(false, true, FM_LINEAR);
  }

  m_pTitleTL = NULL;
  pSprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "TitleTL");
  if(pSprite != NULL) {
    m_pTitleTL = pSprite->getTexture(false, true, FM_LINEAR);
  }

  m_pTitleTR = NULL;
  pSprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "TitleTR");
  if(pSprite != NULL) {
    m_pTitleTR = pSprite->getTexture(false, true, FM_LINEAR);
  }

  m_require_updateFavoriteLevelsList = false;
  m_require_updateReplaysList        = false;
  m_require_updateLevelsList         = false;
  m_require_updateStats              = false;

  StateManager::instance()->registerAsObserver("REPLAYS_DELETE", this);
  StateManager::instance()->registerAsObserver("UPDATEPROFILE", this);
  StateManager::instance()->registerAsObserver("NEW_LEVELS_TO_DOWNLOAD", this);
  StateManager::instance()->registerAsObserver("NO_NEW_LEVELS_TO_DOWNLOAD", this);
  StateManager::instance()->registerAsObserver("FAVORITES_UPDATED", this);
  StateManager::instance()->registerAsObserver("REPLAYS_UPDATED", this);
  StateManager::instance()->registerAsObserver("STATS_UPDATED", this);
  StateManager::instance()->registerAsObserver("LEVELS_UPDATED", this);
  StateManager::instance()->registerAsObserver("HIGHSCORES_UPDATED", this);
  StateManager::instance()->registerAsObserver("BLACKLISTEDLEVELS_UPDATED", this);
  StateManager::instance()->registerAsObserver("LEVELSPACKS_COUNT_UPDATED", this);
  StateManager::instance()->registerAsObserver("CHECKWWWW_DONE", this);
  StateManager::instance()->registerAsObserver("CONFIGURE_WWW_ACCESS", this);

  if(XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("REPLAYS_UPDATED");
    StateManager::instance()->registerAsEmitter("FAVORITES_UPDATED");
  }
}

StateMainMenu::~StateMainMenu()
{
  if(m_levelsPacksCountThread != NULL) {
    /* cleaning thread */
    if(m_levelsPacksCountThread->waitForThreadEnd() != 0) {
      LogWarning("LevelpacksCount thread failed");     
    }
    delete m_levelsPacksCountThread;
  }

  if(m_checkWwwThread != NULL) {
    /* cleaning thread */
    if(m_checkWwwThread->waitForThreadEnd() != 0) {
      LogWarning("CheckWWW thread failed");     
    }
    delete m_checkWwwThread;
  }

  if(m_quickStartList != NULL) {
    delete m_quickStartList;
  }

  StateManager::instance()->unregisterAsObserver("REPLAYS_DELETE", this);
  StateManager::instance()->unregisterAsObserver("UPDATEPROFILE", this);
  StateManager::instance()->unregisterAsObserver("NEW_LEVELS_TO_DOWNLOAD", this);
  StateManager::instance()->unregisterAsObserver("NO_NEW_LEVELS_TO_DOWNLOAD", this);
  StateManager::instance()->unregisterAsObserver("FAVORITES_UPDATED", this);
  StateManager::instance()->unregisterAsObserver("REPLAYS_UPDATED", this);
  StateManager::instance()->unregisterAsObserver("STATS_UPDATED", this);
  StateManager::instance()->unregisterAsObserver("LEVELS_UPDATED", this);
  StateManager::instance()->unregisterAsObserver("HIGHSCORES_UPDATED", this);
  StateManager::instance()->unregisterAsObserver("BLACKLISTEDLEVELS_UPDATED", this);
  StateManager::instance()->unregisterAsObserver("LEVELSPACKS_COUNT_UPDATED", this);
  StateManager::instance()->unregisterAsObserver("CHECKWWWW_DONE", this);
  StateManager::instance()->unregisterAsObserver("CONFIGURE_WWW_ACCESS", this);
}


void StateMainMenu::enter()
{ 
  createGUIIfNeeded();
  m_GUI = m_sGUI;

  StateMenu::enter();

  updateProfileStrings();
  updateOptions();
  updateNewLevels(); // check new levels
  updateInfoFrame();

  // show it before updating lists (which can take some time)
  StateManager::instance()->render(); 

  updateLevelsPacksList(); // update list even if computation is not done the first time to display packs
  updateLevelsLists();
  updateReplaysList();
  updateStats();

  /* don't update packs count now if checks www must be done */
  /* if profile is not initialized, don't update */
  m_initialLevelsPacksDone = false;

  /* no profile */
  if(XMSession::instance()->profile() == "" ||
     xmDatabase::instance("main")->stats_checkKeyExists_stats_profiles(XMSession::instance()->sitekey(),
								       XMSession::instance()->profile()) == false) {
    StateManager::instance()->pushState(new StateEditProfile(this));
    
    /* in case there is no profile, we show a message box */
    /* Should we show a notification box? (with important one-time info) */
    /* thus, is it really required to save this value as profile value ?!? */
    if(XMSession::instance()->notifyAtInit()) {
      StateManager::instance()->pushState(new StateMessageBox(NULL, GAMETEXT_NOTIFYATINIT, UI_MSGBOX_OK));
      XMSession::instance()->setNotifyAtInit(false);
    }
    
  } else {
    if(CheckWwwThread::isNeeded()) {
      if(m_checkWwwThread == NULL) {
	m_checkWwwThread = new CheckWwwThread();
	m_checkWwwThread->startThread();
      }
    } else {
      updateLevelsPacksCountDetached();
    }
  }

  GameApp::instance()->playMenuMusic("menu1");
}

void StateMainMenu::remakePacks() {
  /* after make packs is done, updateLevelsPacksList must 
     systematically done !!!
  */
  LevelsManager::instance()->makePacks(XMSession::instance()->profile(),
				       XMSession::instance()->idRoom(0),
				       XMSession::instance()->debug(),
				       xmDatabase::instance("main"));
  updateLevelsPacksList();
}

void StateMainMenu::enterAfterPop()
{
  bool v_levelsListsUpdated = false;
  bool v_needPacksCount = false;

  StateMenu::enterAfterPop();

  // reset the current playing list
  GameApp::instance()->setCurrentPlayingList(NULL);

  if(m_requestForEnd) {
    return;
  }

  /* updates to do except if request for end is required */

  if(m_require_updateFavoriteLevelsList) {
    updateFavoriteLevelsList();
    m_require_updateFavoriteLevelsList = false;
  }

  if(m_require_updateReplaysList) {
    updateReplaysList();
    m_require_updateReplaysList = false;
  }

  if(m_require_updateLevelsList) {
    remakePacks();
    if(v_levelsListsUpdated == false) {
      updateLevelsLists();
      v_needPacksCount = true;
      v_levelsListsUpdated = true;
    }
    m_require_updateLevelsList = false;
  }

  if(m_require_updateStats) {
    // update lists and stats
    if(v_levelsListsUpdated == false) {
      updateLevelsLists();
      v_needPacksCount = true;
      v_levelsListsUpdated = true;
    }
    updateStats();
    m_require_updateStats = false;
  }

  // differs the pack count at the end to limit the freeze
  if(v_needPacksCount) {
    updateLevelsPacksCountDetached();
  }

  GameApp::instance()->playMenuMusic("menu1");
}

void StateMainMenu::leave() {
}

void StateMainMenu::checkEvents() {  
  // main window
  checkEventsMainWindow();

  // level tab
  checkEventsLevelsPackTab();
  checkEventsLevelsFavoriteTab();
  checkEventsLevelsNewTab();
  checkEventsLevelsMultiTab();

  // replay tab
  checkEventsReplays();

  // update info frame for any event while it's quite long to determine which one are required
  updateInfoFrame();
}

void StateMainMenu::checkEventsMainWindow() {
  UIButton*           v_button;
  std::string         v_id_level;
  UIQuickStartButton* v_quickstart;
  UIButtonDrawn*      v_buttonDrawn;

  // quickstart
  v_quickstart = reinterpret_cast<UIQuickStartButton *>(m_GUI->getChild("MAIN:QUICKSTART"));

  if(v_quickstart->hasChanged()) {
    v_quickstart->setHasChanged(false);

    XMSession::instance()->setQuickStartQualityMIN(v_quickstart->getQualityMIN());
    XMSession::instance()->setQuickStartQualityMAX(v_quickstart->getQualityMAX());
    XMSession::instance()->setQuickStartDifficultyMIN(v_quickstart->getDifficultyMIN());
    XMSession::instance()->setQuickStartDifficultyMAX(v_quickstart->getDifficultyMAX());
  }

  if(v_quickstart->isClicked()) {
    v_quickstart->setClicked(false);

    try {
      if(m_quickStartList != NULL) {
	delete m_quickStartList;
      }
      m_quickStartList = buildQuickStartList();

      if(m_quickStartList->getEntries().size() == 0) {
	throw Exception("Empty quick start list");
      }
      GameApp::instance()->setCurrentPlayingList(m_quickStartList);
      v_id_level = m_quickStartList->getLevel(0);
    } catch(Exception &e) {
      v_id_level = "tut1";
    }
    StateManager::instance()->pushState(new StatePreplayingGame(v_id_level, false));
  }

  // quit
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:QUIT"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    StateMessageBox* v_msgboxState = new StateMessageBox(this, GAMETEXT_QUITMESSAGE, UI_MSGBOX_YES|UI_MSGBOX_NO);
    v_msgboxState->setId("QUIT");
    v_msgboxState->makeActiveButton(UI_MSGBOX_YES);
    StateManager::instance()->pushState(v_msgboxState);
  }

  // help
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:HELP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    StateManager::instance()->pushState(new StateHelp());
  }

  // levels
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:LEVELS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    UIWindow* v_windowLevels = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_LEVELS"));
    UIWindow* v_windowReplays = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_REPLAYS"));
    v_windowLevels->showWindow(v_windowLevels->isHidden());
    v_windowReplays->showWindow(false);
  }

  // replays
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:REPLAYS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    UIWindow* v_windowLevels = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_LEVELS"));
    UIWindow* v_windowReplays = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_REPLAYS"));
    v_windowLevels->showWindow(false);
    v_windowReplays->showWindow(v_windowReplays->isHidden());
  }

  // options
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:OPTIONS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    StateManager::instance()->pushState(new StateOptions());
  }

  // edit profile
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:CHANGEPLAYERBUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    StateManager::instance()->pushState(new StateEditProfile(this));
  }

  // new levels ?
  v_buttonDrawn = reinterpret_cast<UIButtonDrawn *>(m_GUI->getChild("MAIN:NEWLEVELAVAILBLE"));
  if(v_buttonDrawn->isClicked()) {
    v_button->setClicked(false);
    
    UIWindow* v_windowLevels = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_LEVELS"));
    UIWindow* v_windowReplays = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_REPLAYS"));
    UITabView* v_tabView = reinterpret_cast<UITabView *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS"));

    v_windowLevels->showWindow(true);
    v_windowReplays->showWindow(false);
    v_tabView->selectChildrenById("NEWLEVELS_TAB");

    StateManager::instance()->pushState(new StateUpgradeLevels());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:INFO_FRAME:BESTPLAYER_VIEW"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    GameApp::instance()->setCurrentPlayingList(getInfoFrameLevelsList());
    StateManager::instance()->pushState(new StateDownloadGhost(getInfoFrameLevelId(), true));
  }
}

void StateMainMenu::checkEventsLevelsMultiTab() {
  UIButton* v_button;

  // MultiStopWhenOneFinishes
  v_button = reinterpret_cast<UIButton*>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:MULTI_TAB:ENABLEMULTISTOPWHENONEFINISHES"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    XMSession::instance()->setMultiStopWhenOneFinishes(v_button->getChecked());
  }

  // multi scenes
  v_button = reinterpret_cast<UIButton*>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:MULTI_TAB:MULTISCENES"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    XMSession::instance()->setMultiScenes(v_button->getChecked() == false);
  }

  // multi players
  for(unsigned int i=0; i<4; i++) {
    std::ostringstream s_nbPlayers;
    s_nbPlayers << (int) i+1;
    v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:MULTI_TAB:MULTINB_" + s_nbPlayers.str()));  
    if(v_button->isClicked()) {
      v_button->setClicked(false);
      if(v_button->getChecked()) {
	XMSession::instance()->setMultiNbPlayers(i+1);
      }
    }
  }
}

void StateMainMenu::checkEventsLevelsPackTab()
{
  UIButton*   v_button;
  UIPackTree* v_packTree;

  // open button
  v_button = reinterpret_cast<UIButton*>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:PACK_TAB:PACK_OPEN_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    v_packTree = reinterpret_cast<UIPackTree*>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:PACK_TAB:PACK_TREE"));

    LevelsPack* nSelectedPack = v_packTree->getSelectedPack();
    if(nSelectedPack != NULL){
      StateManager::instance()->pushState(new StateLevelPackViewer(nSelectedPack));
    }
  }
}

void StateMainMenu::checkEventsLevelsFavoriteTab() {
  UIButton*    v_button;
  UILevelList* v_list;
  std::string  v_id_level;

  // play buton
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:FAVORITE_TAB:PLAY_GO_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    v_list = reinterpret_cast<UILevelList *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:FAVORITE_TAB:FAVORITE_LIST"));
    v_id_level = v_list->getSelectedLevel();

    if(v_id_level != "") {
      GameApp::instance()->setCurrentPlayingList(v_list);
      StateManager::instance()->pushState(new StatePreplayingGame(v_id_level, false));
    }
  }

  // level info button
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:FAVORITE_TAB:LEVEL_INFO_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    v_list = reinterpret_cast<UILevelList *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:FAVORITE_TAB:FAVORITE_LIST"));
    v_id_level = v_list->getSelectedLevel();

    if(v_id_level != "") {
      StateManager::instance()->pushState(new StateLevelInfoViewer(v_id_level));
    }
  }

  // delete from favorite button
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:FAVORITE_TAB:DELETE_FROM_FAVORITE_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    v_list = reinterpret_cast<UILevelList *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:FAVORITE_TAB:FAVORITE_LIST"));
    v_id_level = v_list->getSelectedLevel();

    if(v_id_level != "") {
      LevelsManager::instance()->delFromFavorite(XMSession::instance()->profile(), v_id_level, xmDatabase::instance("main"));
      StateManager::instance()->sendAsynchronousMessage("FAVORITES_UPDATED");
    }
  }
}

void StateMainMenu::checkEventsLevelsNewTab() {
  UIButton*    v_button;
  UILevelList* v_list;
  std::string  v_id_level;

  // play buton
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:NEWLEVELS_TAB:PLAY_GO_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    v_list = reinterpret_cast<UILevelList *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:NEWLEVELS_TAB:NEWLEVELS_LIST"));
    v_id_level = v_list->getSelectedLevel();

    if(v_id_level != "") {
      GameApp::instance()->setCurrentPlayingList(v_list);
      StateManager::instance()->pushState(new StatePreplayingGame(v_id_level, false));
    }
  }

  // level info button
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:NEWLEVELS_TAB:LEVEL_INFO_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    v_list = reinterpret_cast<UILevelList *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:NEWLEVELS_TAB:NEWLEVELS_LIST"));
    v_id_level = v_list->getSelectedLevel();

    if(v_id_level != "") {
      StateManager::instance()->pushState(new StateLevelInfoViewer(v_id_level));
    }
  }

  // download button
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:NEWLEVELS_TAB:DOWNLOAD_LEVELS_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    StateManager::instance()->pushState(new StateUpgradeLevels());
  }

}

bool StateMainMenu::render()
{
  drawBackground();
  return StateMenu::render();
}

void StateMainMenu::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  UILevelList* v_newLevelsList;
  UILevelList* v_favoriteLevelsList;
  UILevelList* v_list;

  if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_F1, KMOD_NONE)) {
    StateManager::instance()->pushState(new StateHelp());
  }

  else if(i_type == INPUT_DOWN && i_xmkey == InputHandler::instance()->getSwitchFavorite()) {
    /* switch favorites */
    v_newLevelsList      = (UILevelList *)m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:NEWLEVELS_TAB:NEWLEVELS_LIST");
    v_favoriteLevelsList = (UILevelList *)m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:FAVORITE_TAB:FAVORITE_LIST");
    v_list               = NULL;

    if(v_newLevelsList->isVisible() == true) {
      v_list = v_newLevelsList;
    } else if(v_favoriteLevelsList->isVisible() == true) {
      v_list = v_favoriteLevelsList;
    }

    if(v_list != NULL) {
      std::string v_id_level = v_list->getSelectedLevel();
  
      if(v_id_level != "") {
	GameApp::instance()->switchLevelToFavorite(v_id_level, true);
	StateManager::instance()->sendAsynchronousMessage("FAVORITES_UPDATED");
      }
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE)) {
    UIWindow* v_windowLevels = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_LEVELS"));
    UIWindow* v_windowReplays = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_REPLAYS"));
    
    if(v_windowLevels->isHidden() == false){
      v_windowLevels->showWindow(false);
    }
    else if(v_windowReplays->isHidden() == false){
      v_windowReplays->showWindow(false);
    }
  }

  else {
    StateMenu::xmKey(i_type, i_xmkey);
  }
}

void StateMainMenu::clean() {
  if(StateMainMenu::m_sGUI != NULL) {
    delete StateMainMenu::m_sGUI;
    StateMainMenu::m_sGUI = NULL;
  }
}

void StateMainMenu::createGUIIfNeeded() {
  if(m_sGUI != NULL)
    return;

  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  m_sGUI = new UIRoot();
  m_sGUI->setFont(drawlib->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      drawlib->getDispWidth(),
		      drawlib->getDispHeight());

  UIWindow* v_menu;
  UIButton* v_button;
  UIStatic* v_someText;
  UIButtonDrawn* v_buttonDrawn;

  v_menu = new UIWindow(m_sGUI, 0, 0, "", m_sGUI->getPosition().nWidth, m_sGUI->getPosition().nHeight);
  v_menu->setID("MAIN");

  /* buttons on left */
  v_button = new UIButton(v_menu, 20, m_sGUI->getPosition().nHeight/2 - (5*57)/2 + 0*57, GAMETEXT_LEVELS, 177, 57);
  v_button->setID("LEVELS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_LEVELS);
  v_menu->setPrimaryChild(v_button);

  v_button = new UIButton(v_menu, 20, m_sGUI->getPosition().nHeight/2 - (5*57)/2 + 1*57, GAMETEXT_REPLAYS, 177, 57);
  v_button->setID("REPLAYS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_REPLAY_LIST);

  v_button = new UIButton(v_menu, 20, m_sGUI->getPosition().nHeight/2 - (5*57)/2 + 2*57, GAMETEXT_OPTIONS, 177, 57);
  v_button->setID("OPTIONS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_OPTIONS);
  
  v_button = new UIButton(v_menu, 20, m_sGUI->getPosition().nHeight/2 - (5*57)/2 + 3*57, GAMETEXT_HELP, 177, 57);
  v_button->setID("HELP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_HELP);

  v_button = new UIButton(v_menu, 20, m_sGUI->getPosition().nHeight/2 - (5*57)/2 + 4*57, GAMETEXT_QUIT, 177, 57);
  v_button->setID("QUIT");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_QUIT_THE_GAME);

  /* quickstart button */
  UIQuickStartButton *v_quickStart;
  v_quickStart = new UIQuickStartButton(v_menu,
					m_sGUI->getPosition().nWidth  -180 -60,
					m_sGUI->getPosition().nHeight -180 -30,
					GAMETEXT_QUICKSTART, 180, 180,
					XMSession::instance()->quickStartQualityMIN(),
					XMSession::instance()->quickStartDifficultyMIN(),
					XMSession::instance()->quickStartQualityMAX(),
					XMSession::instance()->quickStartDifficultyMAX()
					);
  v_quickStart->setFont(drawlib->getFontSmall());
  v_quickStart->setID("QUICKSTART");
  v_quickStart->setCanGetFocus(false);

  /* profile button */
  v_someText = new UIStatic(v_menu, 200, m_sGUI->getPosition().nHeight*2/15, "", m_sGUI->getPosition().nWidth-200-120,50);
  v_someText->setFont(drawlib->getFontMedium());            
  v_someText->setHAlign(UI_ALIGN_RIGHT);
  v_someText->setID("PLAYERTAG");

  v_button = new UIButton(v_menu, m_sGUI->getPosition().nWidth-115, m_sGUI->getPosition().nHeight*2/15, GAMETEXT_CHANGE, 115, 57);
  v_button->setType(UI_BUTTON_TYPE_SMALL);
  v_button->setFont(drawlib->getFontSmall());
  v_someText->setVAlign(UI_ALIGN_CENTER);
  v_button->setID("CHANGEPLAYERBUTTON");
  v_button->setContextHelp(CONTEXTHELP_CHANGE_PLAYER);

  /* xmoto version */
  v_someText = new UIStatic(v_menu, 0, m_sGUI->getPosition().nHeight-20,
			    std::string("X-Moto/") + XMBuild::getVersionString(true), m_sGUI->getPosition().nWidth, 20);
  v_someText->setFont(drawlib->getFontSmall());
  v_someText->setVAlign(UI_ALIGN_BOTTOM);
  v_someText->setHAlign(UI_ALIGN_LEFT);

  /* new levels ? */
  v_buttonDrawn = new UIButtonDrawn(v_menu, "NewLevelsAvailablePlain", "NewLevelsAvailablePlain", "NewLevelsAvailablePlain", 5, -65,
				    GAMETEXT_NEWLEVELS_AVAIBLE, 200, 200);
  v_buttonDrawn->setFont(drawlib->getFontSmall());      
  v_buttonDrawn->setID("NEWLEVELAVAILBLE");
  v_buttonDrawn->setCanGetFocus(false);

  // info frame
  UIWindow* v_infoFrame = new UIWindow(v_menu, 0, m_sGUI->getPosition().nHeight/2 + (5*57)/2, "", 220, 100);
  v_infoFrame->showWindow(false);
  v_infoFrame->setID("INFO_FRAME");
  v_someText = new UIStatic(v_infoFrame, 0, 5, "", 220, 50);
  v_someText->setFont(drawlib->getFontSmall());
  v_someText->setHAlign(UI_ALIGN_CENTER);
  v_someText->setID("BESTPLAYER");
  UIButton* v_infoButton = new UIButton(v_infoFrame, 22, 40, GAMETEXT_VIEWTHEHIGHSCORE, 176, 40);
  v_infoButton->setFont(drawlib->getFontSmall());
  v_infoButton->setContextHelp(CONTEXTHELP_VIEWTHEHIGHSCORE);
  v_infoButton->setID("BESTPLAYER_VIEW");

  // frames
  makeWindowLevels( v_menu);
  makeWindowReplays(v_menu);
  makeWindowStats(v_menu);
}

void StateMainMenu::updateProfileStrings() {
  UIStatic* v_strTxt;

  // profile title
  v_strTxt = reinterpret_cast<UIStatic *>(m_GUI->getChild("MAIN:PLAYERTAG"));
  std::string v_caption;

  if(XMSession::instance()->profile() != "") {
    v_caption = std::string(GAMETEXT_PLAYER) + ": " + XMSession::instance()->profile() + "@" + GameApp::instance()->getWebRoomName(0);
  }

  v_strTxt->setCaption(v_caption);
}

UIWindow* StateMainMenu::makeWindowStats(UIWindow* i_parent) {
  UIStatic* v_someText;
  UIFrame* v_window;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  v_window = new UIFrame(i_parent, 220, drawlib->getDispHeight()*7/30, GAMETEXT_STATS, drawlib->getDispWidth()-200,
			 drawlib->getDispHeight() -40 -drawlib->getDispHeight()/5 -10);      
  v_window->setStyle(UI_FRAMESTYLE_LEFTTAG);
  v_window->setFont(drawlib->getFontSmall());
  v_window->setID("STATS");
  v_window->makeMinimizable(drawlib->getDispWidth()-17, drawlib->getDispHeight()*7/30);
  v_window->setMinimized(true);
  v_window->setContextHelp(CONTEXTHELP_STATS);

  v_someText = new UIStatic(v_window, 0, 0, GAMETEXT_STATISTICS, v_window->getPosition().nWidth, 36);
  v_someText->setFont(drawlib->getFontMedium());

  return v_window;
}

void StateMainMenu::updateStats() {
  UIWindow* i_parent = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:STATS"));
  UIWindow* v_window = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:STATS:REPORT"));
  int x              = 30;
  int y              = 36;
  int nWidth         = i_parent->getPosition().nWidth-45;
  int nHeight        = i_parent->getPosition().nHeight-36;

  /* Create stats window */
  char **v_result;
  unsigned int nrow;
  
  int   v_nbStarts        = 0;
  std::string v_since;
  int   v_totalPlayedTime = 0;
  int   v_nbPlayed        = 0;
  int   v_nbDied          = 0;
  int   v_nbCompleted     = 0;
  int   v_nbRestarted     = 0;
  int   v_nbDiffLevels    = 0;
  std::string v_level_name= "";
  xmDatabase* pDb = xmDatabase::instance("main");
  
  if(v_window != NULL){
    delete v_window;
  }

  v_window = new UIWindow(i_parent, x, y, "", nWidth, nHeight);
  v_window->setID("REPORT");
 
  v_result = pDb->readDB("SELECT IFNULL(SUM(nbStarts),0), IFNULL(MIN(since),0) "
			 "FROM stats_profiles "
			 "WHERE id_profile=\"" + xmDatabase::protectString(XMSession::instance()->profile()) + "\" "
			 "GROUP BY id_profile;",
			 nrow);
  if(nrow == 0) {
    pDb->read_DB_free(v_result);
    return;
  }
  v_nbStarts        = atoi(pDb->getResult(v_result, 2, 0, 0));
  v_since           =      pDb->getResult(v_result, 2, 0, 1);  
  pDb->read_DB_free(v_result);

  v_result = pDb->readDB("SELECT IFNULL(SUM(b.playedTime),0), IFNULL(SUM(b.nbPlayed),0), "
			 "IFNULL(SUM(b.nbDied),0), IFNULL(SUM(b.nbCompleted),0), "
			 "IFNULL(SUM(b.nbRestarted),0) "
			 "FROM stats_profiles AS a INNER JOIN stats_profiles_levels AS b "
			 "ON (a.id_profile=b.id_profile AND a.sitekey=b.sitekey) "
			 "WHERE a.id_profile=\"" + xmDatabase::protectString(XMSession::instance()->profile()) + "\" "
			 "GROUP BY a.id_profile;",
			 nrow);  
  if(nrow == 0) {
    pDb->read_DB_free(v_result);
    return;
  }
  v_totalPlayedTime = atoi(pDb->getResult(v_result, 5, 0, 0));
  v_nbPlayed        = atoi(pDb->getResult(v_result, 5, 0, 1));
  v_nbDied          = atoi(pDb->getResult(v_result, 5, 0, 2));
  v_nbCompleted     = atoi(pDb->getResult(v_result, 5, 0, 3));
  v_nbRestarted     = atoi(pDb->getResult(v_result, 5, 0, 4));
  pDb->read_DB_free(v_result);

  v_result = pDb->readDB("SELECT COUNT(DISTINCT(id_level)) "
			 "FROM stats_profiles_levels "
			 "WHERE id_profile=\"" + xmDatabase::protectString(XMSession::instance()->profile()) + "\";",
			 nrow);
  if(nrow == 0) {
    pDb->read_DB_free(v_result);
    return;
  }  
  v_nbDiffLevels = atoi(pDb->getResult(v_result, 1, 0, 0));
  pDb->read_DB_free(v_result);

  /* Per-player info */
  char cBuf[512];
  char cTime[512];
  int nHours = v_totalPlayedTime / 100 / (60 * 60);
  int nMinutes = v_totalPlayedTime / 100 / 60 - nHours*60;
  int nSeconds = v_totalPlayedTime /100 - nMinutes*60 - nHours*3600;
  if(nHours > 0)
    snprintf(cTime, 512, (std::string(GAMETEXT_XHOURS) + ", " + std::string(GAMETEXT_XMINUTES) + ", " + std::string(GAMETEXT_XSECONDS)).c_str(),nHours,nMinutes,nSeconds);
  else if(nMinutes > 0)
    snprintf(cTime, 512, (std::string(GAMETEXT_XMINUTES) + ", " + std::string(GAMETEXT_XSECONDS)).c_str(),nMinutes,nSeconds);
  else
    snprintf(cTime, 512, GAMETEXT_XSECONDS,nSeconds);
  
  snprintf(cBuf, 512, std::string(std::string("(") +
				  GAMETEXT_XMOTOGLOBALSTATS_SINCE                     + std::string(")\n")  +
				  GAMETEXT_XMOTOGLOBALSTATS_START(v_nbStarts)         + std::string("; ")   +
				  GAMETEXT_XMOTOLEVELSTATS_PLAYS(v_nbPlayed)          + std::string(" (")   +
				  GAMETEXT_XMOTOGLOBALSTATS_DIFFERENT(v_nbDiffLevels) + std::string("),\n") +
				  GAMETEXT_XMOTOLEVELSTATS_DEATHS(v_nbDied)           + std::string(", ")   +
				  GAMETEXT_XMOTOLEVELSTATS_FINISHED(v_nbCompleted)    + std::string(", ")   +
				  GAMETEXT_XMOTOLEVELSTATS_RESTART(v_nbRestarted)     + std::string(".\n")  +
				  GAMETEXT_XMOTOGLOBALSTATS_TIMEPLAYED).c_str(),
	   v_since.c_str(), v_nbStarts, v_nbPlayed, v_nbDiffLevels,
	   v_nbDied, v_nbCompleted, v_nbRestarted, cTime);                           
  
  UIStatic *pText = new UIStatic(v_window, 0, 0, cBuf, nWidth, 80);
  pText->setHAlign(UI_ALIGN_LEFT);
  pText->setTextSolidColor(MAKE_COLOR(255,255,0,255));
  pText->setFont(GameApp::instance()->getDrawLib()->getFontSmall());
  
  /* Per-level stats */      
  pText = new UIStatic(v_window,0,90, std::string(GAMETEXT_MOSTPLAYEDLEVELSFOLLOW) + ":",nWidth,20);
  pText->setHAlign(UI_ALIGN_LEFT);
  pText->setTextSolidColor(MAKE_COLOR(255,255,0,255));
  pText->setFont(GameApp::instance()->getDrawLib()->getFontSmall());      
  
  v_result = pDb->readDB("SELECT MIN(a.name), IFNULL(SUM(b.nbPlayed),0), IFNULL(SUM(b.nbDied),0), "
			 "IFNULL(SUM(b.nbCompleted),0), IFNULL(SUM(b.nbRestarted),0), IFNULL(SUM(b.playedTime),0) "
			 "FROM levels AS a INNER JOIN stats_profiles_levels AS b ON a.id_level=b.id_level "
			 "WHERE b.id_profile=\"" + xmDatabase::protectString(XMSession::instance()->profile()) + "\" "
			 "GROUP BY b.id_level "
			 "ORDER BY SUM(b.playedTime) DESC LIMIT 10;",
			 nrow);
  
  int cy = 110;
  for(unsigned int i=0; i<nrow; i++) {
    if(cy + 45 > nHeight) break; /* out of window */
    
    v_level_name      =      pDb->getResult(v_result, 6, i, 0);
    v_totalPlayedTime = atoi(pDb->getResult(v_result, 6, i, 5));
    v_nbDied          = atoi(pDb->getResult(v_result, 6, i, 2));
    v_nbPlayed        = atoi(pDb->getResult(v_result, 6, i, 1));
    v_nbCompleted     = atoi(pDb->getResult(v_result, 6, i, 3));
    v_nbRestarted     = atoi(pDb->getResult(v_result, 6, i, 4));
    
    snprintf(cBuf, 512, ("[%s] %s:\n   " + std::string(GAMETEXT_XMOTOLEVELSTATS_PLAYS(v_nbPlayed)       + std::string(", ") +
						       GAMETEXT_XMOTOLEVELSTATS_DEATHS(v_nbDied)        + std::string(", ") +
						       GAMETEXT_XMOTOLEVELSTATS_FINISHED(v_nbCompleted) + std::string(", ") +
						       GAMETEXT_XMOTOLEVELSTATS_RESTART(v_nbRestarted))).c_str(),
	     formatTime(v_totalPlayedTime).c_str(), v_level_name.c_str(),
	     v_nbPlayed, v_nbDied, v_nbCompleted, v_nbRestarted);
    
    pText = new UIStatic(v_window, 0, cy, cBuf, nWidth, 45);
    pText->setHAlign(UI_ALIGN_LEFT);        
    pText->setTextSolidColor(MAKE_COLOR(255,255,0,255));
    pText->setFont(GameApp::instance()->getDrawLib()->getFontSmall());
    
    cy += 45;
  }  
  pDb->read_DB_free(v_result);
}

UIWindow* StateMainMenu::makeWindowReplays(UIWindow* i_parent) {
  UIWindow* v_window;
  UIStatic* v_someText;
  UIEdit*   v_edit;
  UIButton *v_button, *v_showButton;
  UIList*   v_list;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  v_window = new UIFrame(i_parent, 220, i_parent->getPosition().nHeight*7/30, "",
			 i_parent->getPosition().nWidth -220 -20,
			 i_parent->getPosition().nHeight -40 -i_parent->getPosition().nHeight/5 -10);
  v_window->setID("FRAME_REPLAYS");
  v_window->showWindow(false);
   
  v_someText = new UIStatic(v_window, 0, 0, GAMETEXT_REPLAYS, v_window->getPosition().nWidth, 36);
  v_someText->setFont(drawlib->getFontMedium());

  v_someText = new UIStatic(v_window, 10, 35, std::string(GAMETEXT_FILTER) + ":", 90, 25);
  v_someText->setFont(drawlib->getFontSmall());
  v_someText->setHAlign(UI_ALIGN_RIGHT);
  v_edit = new UIEdit(v_window, 110, 35, "", 200, 25);
  v_edit->setFont(drawlib->getFontSmall());
  v_edit->setID("REPLAYS_FILTER");
  v_edit->setContextHelp(CONTEXTHELP_REPLAYS_FILTER);

  /* show button */
  v_button = new UIButton(v_window, 5, v_window->getPosition().nHeight-68, GAMETEXT_SHOW, 110, 57);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_SMALL);
  v_button->setID("REPLAYS_SHOW_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_RUN_REPLAY);
  v_showButton = v_button;

  /* delete button */
  v_button = new UIButton(v_window, 110, v_window->getPosition().nHeight-68, GAMETEXT_DELETE, 115, 57);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_SMALL);
  v_button->setID("REPLAYS_DELETE_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_DELETE_REPLAY);

  /* upload button */
  v_button = new UIButton(v_window, 220, v_window->getPosition().nHeight-68, GAMETEXT_UPLOAD_HIGHSCORE, 130, 57);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_SMALL);
  v_button->setID("REPLAYS_UPLOADHIGHSCORE_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_UPLOAD_HIGHSCORE);

  /* clean */
  v_button = new UIButton(v_window, 345, v_window->getPosition().nHeight-68, GAMETEXT_CLEAN, 116, 57);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_SMALL);
  v_button->setID("REPLAYS_CLEAN_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_REPLAYS_CLEAN);
  v_button->showWindow(false);

  /* filter */
  v_button = new UIButton(v_window, v_window->getPosition().nWidth-105, v_window->getPosition().nHeight-68,
			  GAMETEXT_LISTALL, 115, 57);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setChecked(false);
  v_button->setID("REPLAYS_LIST_ALL");
  v_button->setContextHelp(CONTEXTHELP_ALL_REPLAYS);

  /* list */
  v_list = new UIList(v_window, 20, 65, "", v_window->getPosition().nWidth-40, v_window->getPosition().nHeight-115-25);
  v_list->setID("REPLAYS_LIST");
  v_list->setFont(drawlib->getFontSmall());
  v_list->addColumn(GAMETEXT_REPLAY, v_list->getPosition().nWidth/2 - 100, CONTEXTHELP_REPLAYCOL);
  v_list->addColumn(GAMETEXT_LEVEL,  v_list->getPosition().nWidth/2 - 28,  CONTEXTHELP_REPLAYLEVELCOL);
  v_list->addColumn(GAMETEXT_PLAYER,128,CONTEXTHELP_REPLAYPLAYERCOL);
  v_list->setEnterButton(v_showButton);

  return v_window;
}

UIWindow* StateMainMenu::makeWindowLevels(UIWindow* i_parent) {
  UIWindow*    v_window;
  UIStatic*    v_someText;
  UIButton*    v_button;
  UIPackTree*  v_packTree;
  UILevelList* v_list;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  v_window = new UIFrame(i_parent, 220, i_parent->getPosition().nHeight*7/30, "",
			 i_parent->getPosition().nWidth -220 -20,
			 i_parent->getPosition().nHeight -40 -i_parent->getPosition().nHeight/5 -10);
  v_window->setID("FRAME_LEVELS");
  v_window->showWindow(false);
   
  v_someText = new UIStatic(v_window, 0, 0, GAMETEXT_LEVELS, v_window->getPosition().nWidth, 36);
  v_someText->setFont(drawlib->getFontMedium());

  /* tabs */
  UITabView *v_levelTabs = new UITabView(v_window, 20, 40, "",
					 v_window->getPosition().nWidth-40, v_window->getPosition().nHeight-60);
  v_levelTabs->setFont(drawlib->getFontSmall());
  v_levelTabs->setID("TABS");
  v_levelTabs->setTabContextHelp(0, CONTEXTHELP_LEVEL_PACKS);
  v_levelTabs->setTabContextHelp(1, CONTEXTHELP_BUILT_IN_AND_EXTERNALS);
  v_levelTabs->setTabContextHelp(2, CONTEXTHELP_NEW_LEVELS);

  /* pack tab */
  UIWindow *v_packTab = new UIWindow(v_levelTabs, 10, 40, GAMETEXT_LEVELPACKS,
				     v_levelTabs->getPosition().nWidth-20, v_levelTabs->getPosition().nHeight);
  v_packTab->setID("PACK_TAB");

  /* open button */
  v_button = new UIButton(v_packTab, 11, v_packTab->getPosition().nHeight-57-45, GAMETEXT_OPEN, 115, 57);
  v_button->setType(UI_BUTTON_TYPE_SMALL);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setID("PACK_OPEN_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_VIEW_LEVEL_PACK);
  
  /* pack list */
  v_packTree = new UIPackTree(v_packTab, 10, 0, "", v_packTab->getPosition().nWidth-20, v_packTab->getPosition().nHeight-105);      
  v_packTree->setFont(drawlib->getFontSmall());
  v_packTree->setID("PACK_TREE");
  v_packTree->setEnterButton(v_button);

  /* favorite tab */
  UIWindow *v_favoriteTab = new UIWindow(v_levelTabs, 20, 40, VPACKAGENAME_FAVORITE_LEVELS,
					 v_levelTabs->getPosition().nWidth-40, v_levelTabs->getPosition().nHeight);
  v_favoriteTab->setID("FAVORITE_TAB");
  v_favoriteTab->showWindow(false);

  /* favorite go button */
  v_button = new UIButton(v_favoriteTab, 0, v_favoriteTab->getPosition().nHeight-103, GAMETEXT_STARTLEVEL, 105, 57);
  v_button->setType(UI_BUTTON_TYPE_SMALL);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setID("PLAY_GO_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_PLAY_SELECTED_LEVEL);

  /* favorite list */
  v_list = new UILevelList(v_favoriteTab, 0, 0, "", v_favoriteTab->getPosition().nWidth, v_favoriteTab->getPosition().nHeight-105);     
  v_list->setID("FAVORITE_LIST");
  v_list->setFont(drawlib->getFontSmall());
  v_list->setSort(true);
  v_list->setEnterButton(v_button);

  /* favorite other buttons */
  v_button= new UIButton(v_favoriteTab, 105, v_favoriteTab->getPosition().nHeight-103, GAMETEXT_SHOWINFO, 105, 57);
  v_button->setType(UI_BUTTON_TYPE_SMALL);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setID("LEVEL_INFO_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_LEVEL_INFO);
  
  v_button= new UIButton(v_favoriteTab, v_favoriteTab->getPosition().nWidth-187, v_favoriteTab->getPosition().nHeight-103, GAMETEXT_DELETEFROMFAVORITE, 187, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setID("DELETE_FROM_FAVORITE_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_DELETEFROMFAVORITE);
  
  /* new levels tab */
  UIWindow *v_newLevelsTab = new UIWindow(v_levelTabs, 20, 40, GAMETEXT_NEWLEVELS,
					  v_levelTabs->getPosition().nWidth-40, v_levelTabs->getPosition().nHeight);
  v_newLevelsTab->showWindow(false);
  v_newLevelsTab->setID("NEWLEVELS_TAB");

  /* new levels tab go button */
  v_button = new UIButton(v_newLevelsTab, 0, v_newLevelsTab->getPosition().nHeight-103, GAMETEXT_STARTLEVEL, 105, 57);
  v_button->setType(UI_BUTTON_TYPE_SMALL);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setID("PLAY_GO_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_PLAY_SELECTED_LEVEL);

  /* new levels list */
  v_list = new UILevelList(v_newLevelsTab, 0, 0, "", v_newLevelsTab->getPosition().nWidth, v_newLevelsTab->getPosition().nHeight-105);     
  v_list->setFont(drawlib->getFontSmall());
  v_list->setID("NEWLEVELS_LIST");
  v_list->setSort(true);
  v_list->setEnterButton(v_button);

  /* new levels tab other buttons */
  v_button = new UIButton(v_newLevelsTab, 105, v_newLevelsTab->getPosition().nHeight-103, GAMETEXT_SHOWINFO, 105, 57);
  v_button->setType(UI_BUTTON_TYPE_SMALL);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setID("LEVEL_INFO_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_LEVEL_INFO);

  v_button = new UIButton(v_newLevelsTab, v_newLevelsTab->getPosition().nWidth-187, v_newLevelsTab->getPosition().nHeight-103,
			  GAMETEXT_DOWNLOADLEVELS, 187, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setID("DOWNLOAD_LEVELS_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_DOWNLOADLEVELS);

  /* multi tab */
  UIWindow *v_multiOptionsTab = new UIWindow(v_levelTabs, 20, 40, GAMETEXT_MULTI,
					     v_levelTabs->getPosition().nWidth-40, v_levelTabs->getPosition().nHeight);
  v_multiOptionsTab->showWindow(false);
  v_multiOptionsTab->setID("MULTI_TAB");

  v_someText = new UIStatic(v_multiOptionsTab, 10, 0, GAMETEXT_NB_PLAYERS, v_multiOptionsTab->getPosition().nWidth, 40);
  v_someText->setFont(drawlib->getFontMedium());
  v_someText->setHAlign(UI_ALIGN_LEFT);

  // choice of the number of players
  for(unsigned int i=0; i<4; i++) {
    std::ostringstream s_nbPlayers;
    char strPlayer[64];
    snprintf(strPlayer, 64, GAMETEXT_NPLAYER(i+1), i+1);
    s_nbPlayers << (int) i+1;

    v_button = new UIButton(v_multiOptionsTab, 0, 40+(i*20), strPlayer, v_multiOptionsTab->getPosition().nWidth, 28);
    v_button->setType(UI_BUTTON_TYPE_RADIO);
    v_button->setFont(drawlib->getFontSmall());
    v_button->setID("MULTINB_" + s_nbPlayers.str());
    v_button->setGroup(10200);
    v_button->setContextHelp(CONTEXTHELP_MULTI);
    
    // always check the 1 player mode
    if(i == 0) {
      v_button->setChecked(true);
    }
  }

  v_button = new UIButton(v_multiOptionsTab, 0, v_multiOptionsTab->getPosition().nHeight - 40 - 28 - 28 - 10,
			  GAMETEXT_NOMULTISCENES, v_multiOptionsTab->getPosition().nWidth,28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setID("MULTISCENES");
  v_button->setGroup(50051);
  v_button->setContextHelp(CONTEXTHELP_NOMULTISCENES); 

  v_button = new UIButton(v_multiOptionsTab, 0, v_multiOptionsTab->getPosition().nHeight - 40 - 28 - 10,
			  GAMETEXT_MULTISTOPWHENONEFINISHES, v_multiOptionsTab->getPosition().nWidth,28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setID("ENABLEMULTISTOPWHENONEFINISHES");
  v_button->setGroup(50050);
  v_button->setContextHelp(CONTEXTHELP_MULTISTOPWHENONEFINISHES);    

  return v_window;
}

void StateMainMenu::drawBackground() {
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  if(XMSession::instance()->menuGraphics() != GFX_LOW && XMSession::instance()->ugly() == false) {
    int w = drawlib->getDispWidth();
    int h = drawlib->getDispHeight();

    if(m_pTitleTL != NULL)
      drawlib->drawImage(Vector2f(0, 0), Vector2f(w/2, h/2), m_pTitleTL, 0xFFFFFFFF, true);
    if(m_pTitleTR != NULL)
      drawlib->drawImage(Vector2f(w/2, 0), Vector2f(w, h/2), m_pTitleTR, 0xFFFFFFFF, true);
    if(m_pTitleBR != NULL)
      drawlib->drawImage(Vector2f(w/2, h/2), Vector2f(w, h), m_pTitleBR, 0xFFFFFFFF, true);
    if(m_pTitleBL != NULL)
      drawlib->drawImage(Vector2f(0, h/2), Vector2f(w/2, h), m_pTitleBL, 0xFFFFFFFF, true);
  } else {
    drawlib->clearGraphics();
  }
}

UILevelList* StateMainMenu::buildQuickStartList() {
  UILevelList* v_list;
  UIQuickStartButton *v_quickStart = reinterpret_cast<UIQuickStartButton *>(m_GUI->getChild("MAIN:QUICKSTART"));

  v_list = new UILevelList(m_GUI, 0, 0, "", 0, 0);     
  v_list->setID("QUICKSTART_LIST");
  v_list->showWindow(false);
  v_list->setCanGetFocus(false);
  
  createLevelListsSql(v_list,
		      LevelsManager::getQuickStartPackQuery(v_quickStart->getQualityMIN(),
							    v_quickStart->getDifficultyMIN(),
							    v_quickStart->getQualityMAX(),
							    v_quickStart->getDifficultyMAX(),
							    XMSession::instance()->profile(), XMSession::instance()->idRoom(0),
							    xmDatabase::instance("main")));
  return v_list;
}

void StateMainMenu::createLevelListsSql(UILevelList *io_levelsList, const std::string& i_sql) {
  char **v_result;
  unsigned int nrow;
  int v_playerHighscore, v_roomHighscore;

  /* get selected item */
  std::string v_selected_levelName = "";
  int v_selected;
  if((io_levelsList->getSelected() >= 0) && (io_levelsList->getSelected() < io_levelsList->getEntries().size())) {
    UIListEntry *pEntry = io_levelsList->getEntries()[io_levelsList->getSelected()];
    v_selected_levelName = pEntry->Text[0];
  }
  v_selected = io_levelsList->getSelected();

  io_levelsList->clear();
  
  v_result = xmDatabase::instance("main")->readDB(i_sql,
				      nrow);
  for(unsigned int i=0; i<nrow; i++) {
    if(xmDatabase::instance("main")->getResult(v_result, 4, i, 2) == NULL) {
      v_playerHighscore = -1;
    } else {
      v_playerHighscore = atoi(xmDatabase::instance("main")->getResult(v_result, 4, i, 2));
    }
    
    if(xmDatabase::instance("main")->getResult(v_result, 4, i, 3) == NULL) {
      v_roomHighscore = -1;
    } else {
      v_roomHighscore = atoi(xmDatabase::instance("main")->getResult(v_result, 4, i, 3));
    }
    
    io_levelsList->addLevel(xmDatabase::instance("main")->getResult(v_result, 4, i, 0),
			    xmDatabase::instance("main")->getResult(v_result, 4, i, 1),
			    v_playerHighscore,
			    v_roomHighscore
			    );
  }
  xmDatabase::instance("main")->read_DB_free(v_result);    
  
  /* reselect the previous level */
  if(v_selected_levelName != "") {
    int nLevel = 0;
    bool v_found = false;
    for(unsigned int i=0; i<io_levelsList->getEntries().size(); i++) {
      if(io_levelsList->getEntries()[i]->Text[0] == v_selected_levelName) {
	nLevel = i;
	v_found = true;
	break;
      }
    }
    if(v_found == false) {
      nLevel = v_selected;
    }
    io_levelsList->setRealSelected(nLevel);
  }
}

void StateMainMenu::sendFromMessageBox(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  if(i_id == "QUIT") {
    if(i_button == UI_MSGBOX_YES) {
      m_requestForEnd = true;
      GameApp::instance()->requestEnd();
    }
  }

  if(i_id == "REPLAYS_DELETE") {
    if(i_button == UI_MSGBOX_YES) {
      addCommand("REPLAYS_DELETE");
    }
  }
}

void StateMainMenu::setInputKey(const std::string& i_strKey, const std::string& i_key) {

  for(int i=0; i<INPUT_NB_PLAYERS; i++) {
    std::ostringstream v_n;

    if(i != 0) {
      v_n << " " << (i+1);
    }  

    if(i_strKey == GAMETEXT_DRIVE + v_n.str()) {
      InputHandler::instance()->setDRIVE(i, XMKey(i_key));
    }
    
    if(i_strKey == GAMETEXT_BRAKE + v_n.str()) {
      InputHandler::instance()->setBRAKE(i, XMKey(i_key));
    }
    
    if(i_strKey == GAMETEXT_FLIPLEFT + v_n.str()) {
      InputHandler::instance()->setFLIPLEFT(i, XMKey(i_key));
    }
    
    if(i_strKey == GAMETEXT_FLIPRIGHT + v_n.str()) {
      InputHandler::instance()->setFLIPRIGHT(i, XMKey(i_key));
    }
    
    if(i_strKey == GAMETEXT_CHANGEDIR + v_n.str()) {
      InputHandler::instance()->setCHANGEDIR(i, XMKey(i_key));
    }

    for(unsigned int k=0; k<MAX_SCRIPT_KEY_HOOKS; k++) {
      std::ostringstream v_k;
      v_k << (k+1);
      if(i_strKey == GAMETEXT_SCRIPTACTION + v_n.str() + " " + v_k.str()) {
	InputHandler::instance()->setSCRIPTACTION(i, k, XMKey(i_key));
      }
    }

  }
}

void StateMainMenu::executeOneCommand(std::string cmd, std::string args)
{
  UIListEntry *pEntry = NULL;

  LogDebug("cmd [%s [%s]] executed by state [%s].",
	   cmd.c_str(), args.c_str(), getName().c_str());

  if(cmd == "UPDATEPROFILE") {
    updateProfileStrings();
    updateOptions();

    // update packs
    remakePacks();
    // update lists and stats
    updateLevelsLists();
    updateReplaysList();
    updateLevelsPacksCountDetached();
    updateStats();
  }

  else if(cmd == "REPLAYS_DELETE") {
    UIList* v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_LIST"));
    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      pEntry = v_list->getEntries()[v_list->getSelected()];
      if(pEntry != NULL) {
	try {
	  Replay::deleteReplay(pEntry->Text[0]);
	} catch(Exception &e) {
	  LogError(e.getMsg().c_str());
	}
	try {
	  xmDatabase::instance("main")->replays_delete(pEntry->Text[0]);
	} catch(Exception &e) {
	  LogError(e.getMsg().c_str());
	}
	StateManager::instance()->sendAsynchronousMessage("REPLAYS_UPDATED");
      }
    }
  }

  else if(cmd == "NEW_LEVELS_TO_DOWNLOAD"){
    UIButtonDrawn* v_buttonDrawn;

    v_buttonDrawn = reinterpret_cast<UIButtonDrawn *>(m_GUI->getChild("MAIN:NEWLEVELAVAILBLE"));
    v_buttonDrawn->showWindow(true);
  }

  else if(cmd == "NO_NEW_LEVELS_TO_DOWNLOAD"){
    UIButtonDrawn* v_buttonDrawn;

    v_buttonDrawn = reinterpret_cast<UIButtonDrawn *>(m_GUI->getChild("MAIN:NEWLEVELAVAILBLE"));
    v_buttonDrawn->showWindow(false);
  }

  else if(cmd == "FAVORITES_UPDATED") {
    if(StateManager::instance()->isTopOfTheStates(this))
      updateFavoriteLevelsList();
    else
      m_require_updateFavoriteLevelsList = true;
  }
    
  else if(cmd == "REPLAYS_UPDATED") {
    if(StateManager::instance()->isTopOfTheStates(this))
      updateReplaysList();
    else
      m_require_updateReplaysList = true;
  }

  else if(cmd == "STATS_UPDATED") {
    if(StateManager::instance()->isTopOfTheStates(this)) {
      updateLevelsPacksCountDetached();
      updateLevelsLists();
      updateReplaysList();
      updateStats();
    } else {
      m_require_updateStats = true;
    }
  }

  else if(cmd == "LEVELSPACKS_COUNT_UPDATED") {
    if(m_levelsPacksCountThread != NULL) {
      /* cleaning thread */
      if(m_levelsPacksCountThread->waitForThreadEnd() != 0) {
	LogWarning("LevelpacksCount thread failed");     
      }
      delete m_levelsPacksCountThread;
      m_levelsPacksCountThread = NULL;
      updateLevelsPacksList();
    }
  }

  else if(cmd == "CONFIGURE_WWW_ACCESS") {
    if(CheckWwwThread::isNeeded()) {
      if(m_checkWwwThread == NULL) {
	m_checkWwwThread = new CheckWwwThread();
	m_checkWwwThread->startThread();
      }
    }  
  }

  else if(cmd == "CHECKWWWW_DONE") {
    std::string v_error;
    
    if(m_checkWwwThread != NULL) {
      /* cleaning thread */
      if(m_checkWwwThread->waitForThreadEnd() != 0) {
	v_error = m_checkWwwThread->getMsg();
      }
      delete m_checkWwwThread;
      m_checkWwwThread = NULL;
    }

    /* if initial packs count has not been done (the only cause that check www has been run), do it now */
    if(m_initialLevelsPacksDone == false) {
      updateLevelsLists();      
      updateLevelsPacksCountDetached();
    }

    /* the error display is differed because updates of the lines before produces a mini freeze which make the animation less smooth */
    if(v_error != "") {
      SysMessage::instance()->displayError(v_error);
    }
  }

  else if(cmd == "LEVELS_UPDATED" || cmd == "HIGHSCORES_UPDATED" || cmd == "BLACKLISTEDLEVELS_UPDATED") {
    if(StateManager::instance()->isTopOfTheStates(this)) {
      remakePacks();
      updateLevelsLists();
      updateLevelsPacksCountDetached();
    } else
      m_require_updateLevelsList = true;
  } else {
    GameState::executeOneCommand(cmd, args);
  }
}

void StateMainMenu::updateLevelsPacksCountDetached() {
  m_initialLevelsPacksDone = true;

  if(m_levelsPacksCountThread == NULL) {
    m_levelsPacksCountThread = new LevelsPacksCountUpdateThread();
    m_levelsPacksCountThread->startThread();
  } else {
    /* stop the current one to run a new one */
    if(m_levelsPacksCountThread->waitForThreadEnd() != 0) {
      LogWarning("LevelpacksCount thread failed");     
    }
    delete m_levelsPacksCountThread;
    m_levelsPacksCountThread = new LevelsPacksCountUpdateThread();
    m_levelsPacksCountThread->startThread();
  }
}

void StateMainMenu::updateLevelsLists() {
  updateFavoriteLevelsList();
  updateNewLevelsList();
}

void StateMainMenu::createLevelLists(UILevelList *i_list, const std::string& i_packageName) {
  LevelsManager::instance()->lockLevelsPacks();
  LevelsPack *v_levelsPack = &(LevelsManager::instance()->LevelsPackByName(i_packageName));
  try {
    createLevelListsSql(i_list, v_levelsPack->getLevelsWithHighscoresQuery(XMSession::instance()->profile(),
									   XMSession::instance()->idRoom(0)));
    LevelsManager::instance()->unlockLevelsPacks();
  } catch(Exception &e) {
    LevelsManager::instance()->unlockLevelsPacks();
  }
}

void StateMainMenu::updateFavoriteLevelsList() {
  createLevelLists((UILevelList *)m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:FAVORITE_TAB:FAVORITE_LIST"),
		   VPACKAGENAME_FAVORITE_LEVELS);
  updateLevelsPackInPackList(VPACKAGENAME_FAVORITE_LEVELS);  
}

void StateMainMenu::updateNewLevelsList() {
  createLevelLists((UILevelList *)m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:NEWLEVELS_TAB:NEWLEVELS_LIST"),
		   VPACKAGENAME_NEW_LEVELS);
}

void StateMainMenu::updateLevelsPacksList() {
  UIPackTree *pTree = (UIPackTree *)m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:PACK_TAB:PACK_TREE");

  /* get selected item */
  std::string v_selected_packName = pTree->getSelectedEntry();
  LevelsManager* v_lm = LevelsManager::instance();

  pTree->clear();

  LevelsManager::instance()->lockLevelsPacks();
  for(unsigned int i=0; i<v_lm->LevelsPacks().size(); i++) {
    /* the unpackaged pack exists only in debug mode */
    if(v_lm->LevelsPacks()[i]->Name() != "" || XMSession::instance()->debug()) {
      if(v_lm->LevelsPacks()[i]->Name() == "") {
	v_lm->LevelsPacks()[i]->setName(GAMETEXT_UNPACKED_LEVELS_PACK);
      }
      pTree->addPack(v_lm->LevelsPacks()[i],
		     v_lm->LevelsPacks()[i]->Group(),
		     v_lm->LevelsPacks()[i]->getNumberOfFinishedLevels(),
		     v_lm->LevelsPacks()[i]->getNumberOfLevels()
		     );
    }
  }
  LevelsManager::instance()->unlockLevelsPacks();
  
  /* reselect the previous pack */
  if(v_selected_packName != "") {
    pTree->setSelectedPackByName(v_selected_packName);
  }

}

void StateMainMenu::updateReplaysList() {
  /* Should we list all players' replays? */
  std::string v_sql;
  char **v_result;
  unsigned int nrow;
  bool v_listAll;
  UIList* v_list;
  int v_selected;

  v_listAll = ((UIButton *) m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_LIST_ALL"))->getChecked();
  v_list = (UIList *) m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_LIST");

  /* Clear list */
  v_selected = v_list->getSelected();
  v_list->clear();
    
  /* Enumerate replays */
  std::string PlayerSearch;
  if(v_listAll) {
    v_sql = "SELECT a.name, a.id_profile, b.name FROM replays AS a "
      "LEFT OUTER JOIN levels AS b ON a.id_level = b.id_level;";
  } else {
    v_sql = "SELECT a.name, a.id_profile, b.name FROM replays AS a "
      "LEFT OUTER JOIN levels AS b ON a.id_level = b.id_level "
      "WHERE a.id_profile=\"" + xmDatabase::protectString(XMSession::instance()->profile()) + "\";";
  }

  v_result = xmDatabase::instance("main")->readDB(v_sql, nrow);
  for(unsigned int i=0; i<nrow; i++) {
    UIListEntry *pEntry = v_list->addEntry(xmDatabase::instance("main")->getResult(v_result, 3, i, 0));
    if(xmDatabase::instance("main")->getResult(v_result, 3, i, 2) == NULL) {
      pEntry->Text.push_back(GAMETEXT_UNKNOWN);
    } else {
      pEntry->Text.push_back(xmDatabase::instance("main")->getResult(v_result, 3, i, 2));
    }
    pEntry->Text.push_back(xmDatabase::instance("main")->getResult(v_result, 3, i, 1));
  }
  xmDatabase::instance("main")->read_DB_free(v_result); 

  v_list->setRealSelected(v_selected);

  // apply the filter
  UIEdit*      v_edit;
  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_FILTER"));
  v_list->setFilter(v_edit->getCaption());

  updateReplaysRights();
}

void StateMainMenu::checkEventsReplays() {
  UIEdit*      v_edit;
  UILevelList* v_list;
  UIButton*    v_button;

  v_list = reinterpret_cast<UILevelList *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_LIST"));

  /* check filter */
  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_FILTER"));
  if(v_edit->hasChanged()) {
    v_edit->setHasChanged(false);
    v_list->setFilter(v_edit->getCaption());
    updateReplaysRights();
  }

  /* list changed */
  if(v_list->isChanged()) {
    v_list->setChanged(false);
    updateReplaysRights();
  }

  // show
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_SHOW_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pListEntry = v_list->getEntries()[v_list->getSelected()];
      if(pListEntry != NULL) {
	StateManager::instance()->pushState(new StatePreplayingReplay(pListEntry->Text[0], false));
      }
    }
  }

  // delete
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_DELETE_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pListEntry = v_list->getEntries()[v_list->getSelected()];
      if(pListEntry != NULL) {
	StateMessageBox* v_msgboxState = new StateMessageBox(this, GAMETEXT_DELETEREPLAYMESSAGE, UI_MSGBOX_YES|UI_MSGBOX_NO);
	v_msgboxState->setId("REPLAYS_DELETE");
	v_msgboxState->makeActiveButton(UI_MSGBOX_YES);
	StateManager::instance()->pushState(v_msgboxState);	
	updateReplaysRights();
      }
    }
  }

  // list all
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_LIST_ALL"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    updateReplaysList();
  }

  // upload
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_UPLOADHIGHSCORE_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pListEntry = v_list->getEntries()[v_list->getSelected()];
      if(pListEntry != NULL) {
	std::string v_replayPath = FS::getUserDir() + "/Replays/" + pListEntry->Text[0] + ".rpl";
	StateManager::instance()->pushState(new StateUploadHighscore(v_replayPath));	  
      }
    }
  }

  // clean
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_CLEAN_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
   
    int n;
    char v_buf[256];
    StateMessageBox* v_msgboxState;

    n = Replay::cleanReplaysNb(xmDatabase::instance("main"));
    if(n > 0) {
      snprintf(v_buf, 256, GAMETEXT_CLEAN_CONFIRM(n), n);
      v_msgboxState = new StateMessageBox(this, v_buf, UI_MSGBOX_YES|UI_MSGBOX_NO);
      v_msgboxState->setId("CLEAN_REPLAYS");
      v_msgboxState->makeActiveButton(UI_MSGBOX_NO);
    } else {
      v_msgboxState = new StateMessageBox(this, GAMETEXT_CLEAN_NOTHING_TO_DO, UI_MSGBOX_OK);
    }
    StateManager::instance()->pushState(v_msgboxState); 
  }
}

void StateMainMenu::updateOptions() {
  UIButton* v_button;

  // level tab (multi)
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:MULTI_TAB:ENABLEMULTISTOPWHENONEFINISHES"));
  v_button->setChecked(XMSession::instance()->MultiStopWhenOneFinishes());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:MULTI_TAB:MULTISCENES"));
  v_button->setChecked(XMSession::instance()->multiScenes() == false);
}

void StateMainMenu::updateNewLevels() {
  UIButtonDrawn* v_buttonDrawn;

  v_buttonDrawn = reinterpret_cast<UIButtonDrawn *>(m_GUI->getChild("MAIN:NEWLEVELAVAILBLE"));
  v_buttonDrawn->showWindow(false);
}

void StateMainMenu::updateLevelsPackInPackList(const std::string& v_levelPack) {
  LevelsManager::instance()->lockLevelsPacks();
  try {
    UIPackTree *pTree = (UIPackTree *)m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:PACK_TAB:PACK_TREE");
    LevelsPack *v_pack = &(LevelsManager::instance()->LevelsPackByName(v_levelPack));

    v_pack->updateCount(xmDatabase::instance("main"), XMSession::instance()->profile());
    pTree->updatePack(v_pack,
		      v_pack->getNumberOfFinishedLevels(),
		      v_pack->getNumberOfLevels());
    LevelsManager::instance()->unlockLevelsPacks();
  } catch(Exception &e) {
    LevelsManager::instance()->unlockLevelsPacks();
  }
}

void StateMainMenu::updateInfoFrame() {
  UIStatic* v_someText = reinterpret_cast<UIStatic *>(m_GUI->getChild("MAIN:INFO_FRAME:BESTPLAYER")); 
  UIButton* v_button   = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:INFO_FRAME:BESTPLAYER_VIEW"));
  UIWindow* v_window   = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:INFO_FRAME"));

  std::string v_id_level = getInfoFrameLevelId();
  if(v_id_level == ""){
    v_window->showWindow(false);
    return;
  }

  std::string v_id_profile;
  std::string v_url;
  bool        v_isAccessible;

  if(v_id_level != "") {
    if(GameApp::instance()->getHighscoreInfos(0, v_id_level, &v_id_profile, &v_url, &v_isAccessible)) {
      v_someText->setCaption(std::string(GAMETEXT_BESTPLAYER) + " : " + v_id_profile);
      v_button->enableWindow(v_isAccessible);
      v_window->showWindow(true);
    } else {
      v_window->showWindow(false);
    }
  }
}

void StateMainMenu::updateReplaysRights() {
  UIList*   v_list   = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_LIST"));
  UIButton* v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_UPLOADHIGHSCORE_BUTTON"));

  std::string v_replay;
  ReplayInfo* rplInfos;

  if(v_list->nbVisibleItems() > 0) {
    UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
    if(pEntry != NULL) {
      v_replay = pEntry->Text[0];

      rplInfos = Replay::getReplayInfos(v_replay);
      if(rplInfos != NULL) {
	if(rplInfos->finishTime > 0 && rplInfos->Player == XMSession::instance()->profile()) {
	  char **v_result;
	  unsigned int nrow;
	  float v_finishTime;
	  bool v_enabled = false;

	  for(unsigned int i=0; i<XMSession::instance()->nbRoomsEnabled(); i++) {
	    v_result = xmDatabase::instance("main")->readDB("SELECT finishTime FROM webhighscores WHERE id_level=\"" + 
							    xmDatabase::protectString(rplInfos->Level) + "\" AND id_room=" +
							    XMSession::instance()->idRoom(i) + ";",
							    nrow);
	    if(nrow == 0) {
	      v_enabled = true;
	    } else {
	      v_finishTime = atof(xmDatabase::instance("main")->getResult(v_result, 1, 0, 0));

	      if(rplInfos->finishTime < v_finishTime) {
		v_enabled = true;
	      }
	    }  	      
	    xmDatabase::instance("main")->read_DB_free(v_result);
	  }

	  v_button->enableWindow(v_enabled);
	}
	delete rplInfos;
      }
    }
  } else {
    v_button->enableWindow(false);
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_SHOW_BUTTON"));
  v_button->enableWindow(v_list->nbVisibleItems() > 0);

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_DELETE_BUTTON"));
  v_button->enableWindow(v_list->nbVisibleItems() > 0);

}

UILevelList* StateMainMenu::getInfoFrameLevelsList()
{
  UILevelList* v_newLevelsList      = (UILevelList *)m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:NEWLEVELS_TAB:NEWLEVELS_LIST");
  UILevelList* v_favoriteLevelsList = (UILevelList *)m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:FAVORITE_TAB:FAVORITE_LIST");
  UILevelList* v_list               = NULL;

  if(v_newLevelsList->isVisible() == true) {
    v_list = v_newLevelsList;
  } else if(v_favoriteLevelsList->isVisible() == true) {
    v_list = v_favoriteLevelsList;
  }

  return v_list;
}

std::string StateMainMenu::getInfoFrameLevelId()
{
  UILevelList* v_list = getInfoFrameLevelsList();

  if(v_list == NULL)
    return "";
  else
    return v_list->getSelectedLevel();
}

void StateMainMenu::refreshStaticCaptions() {
  UIButton* v_button;

  if(m_sGUI == NULL) {
    return;
  }

  v_button = reinterpret_cast<UIButton *>(m_sGUI->getChild("MAIN:LEVELS"));
  v_button->setCaption(GAMETEXT_LEVELS);

  v_button = reinterpret_cast<UIButton *>(m_sGUI->getChild("MAIN:REPLAYS"));
  v_button->setCaption(GAMETEXT_REPLAYS);

  v_button = reinterpret_cast<UIButton *>(m_sGUI->getChild("MAIN:OPTIONS"));
  v_button->setCaption(GAMETEXT_OPTIONS);

  v_button = reinterpret_cast<UIButton *>(m_sGUI->getChild("MAIN:HELP"));
  v_button->setCaption(GAMETEXT_HELP);

  v_button = reinterpret_cast<UIButton *>(m_sGUI->getChild("MAIN:QUIT"));
  v_button->setCaption(GAMETEXT_QUIT);
}
