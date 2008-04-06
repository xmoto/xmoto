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
#include "Game.h"
#include "drawlib/DrawLib.h"
#include "GameText.h"
#include "gui/specific/GUIXMoto.h"
#include "XMSession.h"
#include "StatePreplayingGame.h"
#include "StatePreplayingReplay.h"
#include "StateLevelInfoViewer.h"
#include "StateMessageBox.h"
#include "StateHelp.h"
#include "StateEditProfile.h"
#include "StateReplaying.h"
#include "StateRequestKey.h"
#include "StateLevelPackViewer.h"
#include "StateUpdateThemesList.h"
#include "StateUpdateRoomsList.h"
#include "StateUploadHighscore.h"
#include "StateUploadAllHighscores.h"
#include "StateUpdateTheme.h"
#include "StateCheckWww.h"
#include "StateUpgradeLevels.h"
#include "StateDownloadGhost.h"
#include "LevelsManager.h"
#include "helpers/Log.h"
#include "helpers/System.h"
#include "helpers/Text.h"
#include "StateEditWebConfig.h"
#include "Sound.h"
#include "thread/CheckWwwThread.h"
#include "Replay.h"
#include "Languages.h"

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
}

StateMainMenu::~StateMainMenu()
{
  if(m_quickStartList != NULL) {
    delete m_quickStartList;
  }
}


void StateMainMenu::enter()
{ 
  createGUIIfNeeded();
  m_GUI = m_sGUI;

  GameApp::instance()->playMusic("menu1");

  StateMenu::enter();

  updateProfileStrings();
  updateOptions();
  updateNewLevels(); // check new levels
  updateInfoFrame();

  // show it before updating lists (which can take some time)
  StateManager::instance()->render(); 
  updateLevelsPacksList();
  updateLevelsLists();
  updateReplaysList();
  updateStats();

  if(CheckWwwThread::isNeeded()) {
    StateManager::instance()->pushState(new StateCheckWww());
  }
}

void StateMainMenu::enterAfterPop()
{
  bool v_levelsListsUpdated = false;

  StateMenu::enterAfterPop();

  if(m_require_updateFavoriteLevelsList) {
    updateFavoriteLevelsList();
    m_require_updateFavoriteLevelsList = false;
  }

  if(m_require_updateReplaysList) {
    updateReplaysList();
    m_require_updateReplaysList = false;
  }

  if(m_require_updateLevelsList) {
    LevelsManager::instance()->makePacks(XMSession::instance()->profile(),
					 XMSession::instance()->idRoom(0),
					 XMSession::instance()->debug(),
					 xmDatabase::instance("main"));
    if(v_levelsListsUpdated == false) {
      updateLevelsPacksList();
      updateLevelsLists();
      v_levelsListsUpdated = true;
    }
    m_require_updateLevelsList = false;
  }

  if(m_require_updateStats) {
    // update lists and stats
    if(v_levelsListsUpdated == false) {
      updateLevelsPacksList();
      updateLevelsLists();
      v_levelsListsUpdated = true;
    }
    updateStats();
  }

  GameApp::instance()->playMusic("menu1");
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

  // options
  checkEventsOptions();

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
    UIWindow* v_windowOptions = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_OPTIONS"));
    v_windowLevels->showWindow(v_windowLevels->isHidden());
    v_windowReplays->showWindow(false);
    v_windowOptions->showWindow(false);
  }

  // replays
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:REPLAYS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    UIWindow* v_windowLevels = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_LEVELS"));
    UIWindow* v_windowReplays = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_REPLAYS"));
    UIWindow* v_windowOptions = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_OPTIONS"));
    v_windowLevels->showWindow(false);
    v_windowReplays->showWindow(v_windowReplays->isHidden());
    v_windowOptions->showWindow(false);
  }

  // options
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:OPTIONS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    UIWindow* v_windowLevels = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_LEVELS"));
    UIWindow* v_windowReplays = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_REPLAYS"));
    UIWindow* v_windowOptions = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_OPTIONS"));
    v_windowLevels->showWindow(false);
    v_windowReplays->showWindow(false);
    v_windowOptions->showWindow(v_windowOptions->isHidden());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:DEFAULTS_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    InputHandler::instance()->setDefaultConfig();
    XMSession::instance()->setToDefault();
    updateOptions();
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
    UIWindow* v_windowOptions = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_OPTIONS"));
    UITabView* v_tabView = reinterpret_cast<UITabView *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS"));

    v_windowLevels->showWindow(true);
    v_windowReplays->showWindow(false);
    v_windowOptions->showWindow(false);
    v_tabView->selectChildrenById("NEWLEVELS_TAB");

    StateManager::instance()->pushState(new StateUpgradeLevels());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:INFO_FRAME:BESTPLAYER_VIEW"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

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

void StateMainMenu::keyDown(int nKey, SDLMod mod,int nChar)
{
  switch(nKey) {

  case SDLK_F1:
    StateManager::instance()->pushState(new StateHelp());
    break;

  case SDLK_ESCAPE:{
    UIWindow* v_windowLevels = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_LEVELS"));
    UIWindow* v_windowReplays = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_REPLAYS"));
    UIWindow* v_windowOptions = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_OPTIONS"));

    if(v_windowLevels->isHidden() == false){
      v_windowLevels->showWindow(false);
    }
    else if(v_windowReplays->isHidden() == false){
      v_windowReplays->showWindow(false);
    }
    else if(v_windowOptions->isHidden() == false){
      v_windowOptions->showWindow(false);
    }
  }
    break;

  default:
    StateMenu::keyDown(nKey, mod, nChar);
    break;

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
  makeWindowOptions(v_menu);
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

  // www password static string
  v_strTxt = reinterpret_cast<UIStatic *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:WWW_PASSWORD_STATIC"));
  char buf[256];
  snprintf(buf, 256, GAMETEXT_ACCOUNT_PASSWORD, XMSession::instance()->profile().c_str());
  v_strTxt->setCaption(buf);
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
 
  v_result = pDb->readDB("SELECT a.nbStarts, a.since, SUM(b.playedTime), "
				      "SUM(b.nbPlayed), SUM(b.nbDied), SUM(b.nbCompleted), "
				      "SUM(b.nbRestarted), count(b.id_level) "
				      "FROM stats_profiles AS a INNER JOIN stats_profiles_levels AS b "
				      "ON a.id_profile=b.id_profile "
				      "WHERE a.id_profile=\"" + xmDatabase::protectString(XMSession::instance()->profile()) + "\" "
				      "GROUP BY a.id_profile;",
				      nrow);
  
  if(nrow == 0) {
    pDb->read_DB_free(v_result);
    return;
  }
  
  v_nbStarts        = atoi(pDb->getResult(v_result, 8, 0, 0));
  v_since           =      pDb->getResult(v_result, 8, 0, 1);
  v_totalPlayedTime = atoi(pDb->getResult(v_result, 8, 0, 2));
  v_nbPlayed        = atoi(pDb->getResult(v_result, 8, 0, 3));
  v_nbDied          = atoi(pDb->getResult(v_result, 8, 0, 4));
  v_nbCompleted     = atoi(pDb->getResult(v_result, 8, 0, 5));
  v_nbRestarted     = atoi(pDb->getResult(v_result, 8, 0, 6));
  v_nbDiffLevels    = atoi(pDb->getResult(v_result, 8, 0, 7));
  
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
  
  v_result = pDb->readDB("SELECT a.name, b.nbPlayed, b.nbDied, "
				      "b.nbCompleted, b.nbRestarted, b.playedTime "
				      "FROM levels AS a INNER JOIN stats_profiles_levels AS b ON a.id_level=b.id_level "
				      "WHERE id_profile=\"" + xmDatabase::protectString(XMSession::instance()->profile()) + "\" "
				      "ORDER BY nbPlayed DESC LIMIT 10;",
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

UIWindow* StateMainMenu::makeWindowOptions_general(UIWindow* i_parent) {
  UIWindow*  v_window;
  UIButton*  v_button;
  UIList*    v_list;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  v_window = new UIWindow(i_parent, 20, 40, GAMETEXT_GENERAL,
			  i_parent->getPosition().nWidth-40, i_parent->getPosition().nHeight);
  v_window->setID("GENERAL_TAB");
  
  v_button = new UIButton(v_window, 5, 33-28-10, GAMETEXT_SHOWMINIMAP, (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("SHOWMINIMAP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_MINI_MAP);
  
  v_button = new UIButton(v_window, 5, 63-28-10, GAMETEXT_SHOWENGINECOUNTER, (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("SHOWENGINECOUNTER");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_ENGINE_COUNTER);

  v_button = new UIButton(v_window, 5, 93-28-10, GAMETEXT_ENABLECONTEXTHELP, (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLECONTEXTHELP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_SHOWCONTEXTHELP);
 
  v_button = new UIButton(v_window, 5, 123-28-10, GAMETEXT_AUTOSAVEREPLAYS, (v_window->getPosition().nWidth-40)/*/2*/, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("AUTOSAVEREPLAYS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_AUTOSAVEREPLAYS);

  v_button = new UIButton(v_window, 5+(v_window->getPosition().nWidth+40)/2, 33-28-10, GAMETEXT_INITZOOM,
			  (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("INITZOOM");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_INITZOOM);

  v_button = new UIButton(v_window, 5+(v_window->getPosition().nWidth+40)/2, 63-28-10, GAMETEXT_DEATHANIM,
			  (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("DEATHANIM");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_DEATHANIM);

  /* Button to enable/disable active zoom */
  v_button = new UIButton(v_window, 5+(v_window->getPosition().nWidth+40)/2, 93-28-10, GAMETEXT_CAMERAACTIVEZOOM,
			  (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("CAMERAACTIVEZOOM");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_CAMERAACTIVEZOOM);

   
  v_list = new UIList(v_window, 5, 120, "", 
		      v_window->getPosition().nWidth-10, v_window->getPosition().nHeight-125-90);
  v_list->setID("THEMES_LIST");
  v_list->setFont(drawlib->getFontSmall());
  v_list->addColumn(GAMETEXT_THEMES, (v_list->getPosition().nWidth*3) / 5);
  v_list->addColumn("", (v_list->getPosition().nWidth*2) / 5);
  v_list->setContextHelp(CONTEXTHELP_THEMES);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth -200 -200, v_window->getPosition().nHeight - 95,
			  GAMETEXT_UPDATETHEMESLIST, 207, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("UPDATE_THEMES_LIST");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_UPDATETHEMESLIST);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth -200, v_window->getPosition().nHeight - 95,
			  GAMETEXT_GETSELECTEDTHEME, 207, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("GET_SELECTED_THEME");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GETSELECTEDTHEME);

  return v_window;
}

UIWindow* StateMainMenu::makeWindowOptions_video(UIWindow* i_parent) {
  UIWindow*  v_window;
  UIButton*  v_button;
  UIList*    v_list;
  UIStatic*  v_someText;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  v_window = new UIWindow(i_parent, 20, 40, GAMETEXT_VIDEO, i_parent->getPosition().nWidth-40, i_parent->getPosition().nHeight);
  v_window->setID("VIDEO_TAB");
  v_window->showWindow(false);

  v_button = new UIButton(v_window, 5, 5, GAMETEXT_16BPP, (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("16BPP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20023);
  v_button->setContextHelp(CONTEXTHELP_HIGHCOLOR);

  v_button = new UIButton(v_window, 5 + (v_window->getPosition().nWidth-40)/2, 5, GAMETEXT_32BPP,
			  (v_window->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("32BPP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20023);
  v_button->setContextHelp(CONTEXTHELP_TRUECOLOR);
    
  v_list = new UIList(v_window, 5, 43, "", v_window->getPosition().nWidth - 10, v_window->getPosition().nHeight - 43 - 10 - 140);
  v_list->setID("RESOLUTIONS_LIST");
  v_list->setFont(drawlib->getFontSmall());
  v_list->addColumn(GAMETEXT_SCREENRES, v_list->getPosition().nWidth, CONTEXTHELP_SCREENRES);
  v_list->setContextHelp(CONTEXTHELP_RESOLUTION);

  v_button = new UIButton(v_window,5, v_window->getPosition().nHeight - 43 - 10 - 90, GAMETEXT_RUNWINDOWED,
			  v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("WINDOWED");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_RUN_IN_WINDOW);

  v_someText = new UIStatic(v_window, 5, v_window->getPosition().nHeight - 43 - 10 - 60, std::string(GAMETEXT_MENUGFX) +":", 120, 28);
  v_someText->setFont(drawlib->getFontSmall());    

  v_button = new UIButton(v_window, 120, v_window->getPosition().nHeight - 43 - 10 - 60, GAMETEXT_LOW,
			  (v_window->getPosition().nWidth-120)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("MENULOW");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20024);
  v_button->setContextHelp(CONTEXTHELP_LOW_MENU);
  
  v_button = new UIButton(v_window, 120+(v_window->getPosition().nWidth-120)/3,
			  v_window->getPosition().nHeight - 43 - 10 - 60, GAMETEXT_MEDIUM,(v_window->getPosition().nWidth-120)/3,28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("MENUMEDIUM");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20024);
  v_button->setContextHelp(CONTEXTHELP_MEDIUM_MENU);
  
  v_button = new UIButton(v_window, 120+(v_window->getPosition().nWidth-120)/3*2,
			  v_window->getPosition().nHeight - 43 - 10 - 60, GAMETEXT_HIGH,(v_window->getPosition().nWidth-120)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("MENUHIGH");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20024);
  v_button->setContextHelp(CONTEXTHELP_HIGH_MENU);
  
  v_someText = new UIStatic(v_window, 5, v_window->getPosition().nHeight - 43 - 10 - 30, std::string(GAMETEXT_GAMEGFX) + ":", 120, 28);
  v_someText->setFont(drawlib->getFontSmall());    

  v_button = new UIButton(v_window, 120, v_window->getPosition().nHeight - 43 - 10 - 30,
			  GAMETEXT_LOW,(v_window->getPosition().nWidth-120)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("GAMELOW");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20025);
  v_button->setContextHelp(CONTEXTHELP_LOW_GAME);

  v_button = new UIButton(v_window, 120+(v_window->getPosition().nWidth-120)/3, v_window->getPosition().nHeight - 43 - 10 - 30,
			  GAMETEXT_MEDIUM,(v_window->getPosition().nWidth-120)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("GAMEMEDIUM");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20025);
  v_button->setContextHelp(CONTEXTHELP_MEDIUM_GAME);

  v_button = new UIButton(v_window, 120+(v_window->getPosition().nWidth-120)/3*2, v_window->getPosition().nHeight - 43 - 10 - 30,
			  GAMETEXT_HIGH, (v_window->getPosition().nWidth-120)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("GAMEHIGH");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(20025);
  v_button->setContextHelp(CONTEXTHELP_HIGH_GAME);

  return v_window;
}

UIWindow* StateMainMenu::makeWindowOptions_audio(UIWindow* i_parent) {
  UIWindow*  v_window;
  UIButton*  v_button;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  v_window = new UIWindow(i_parent, 20, 40, GAMETEXT_AUDIO, i_parent->getPosition().nWidth-40, i_parent->getPosition().nHeight);
  v_window->setID("AUDIO_TAB");
  v_window->showWindow(false);

  v_button = new UIButton(v_window, 5, 5, GAMETEXT_ENABLEAUDIO, v_window->getPosition().nWidth-10, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLE_AUDIO");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_SOUND_ON);
    
  v_button = new UIButton(v_window, 25, 33, GAMETEXT_11KHZ, (v_window->getPosition().nWidth-40)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("RATE11KHZ");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(10023);
  v_button->setContextHelp(CONTEXTHELP_11HZ);
    
  v_button = new UIButton(v_window, 25+(v_window->getPosition().nWidth-40)/3, 33, GAMETEXT_22KHZ,
			  (v_window->getPosition().nWidth-40)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("RATE22KHZ");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(10023);
  v_button->setContextHelp(CONTEXTHELP_22HZ);
    
  v_button = new UIButton(v_window, 25+(v_window->getPosition().nWidth-40)/3*2, 33, GAMETEXT_44KHZ,
			  (v_window->getPosition().nWidth-40)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("RATE44KHZ");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(10023);
  v_button->setContextHelp(CONTEXTHELP_44HZ);

  v_button = new UIButton(v_window, 25, 61, GAMETEXT_8BIT, (v_window->getPosition().nWidth-40)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("8BITS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(10024);
  v_button->setContextHelp(CONTEXTHELP_8BIT);

  v_button = new UIButton(v_window, 25+(v_window->getPosition().nWidth-40)/3, 61, GAMETEXT_16BIT,
			  (v_window->getPosition().nWidth-40)/3, 28);    
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("16BITS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(10024);
  v_button->setContextHelp(CONTEXTHELP_16BIT);

  v_button = new UIButton(v_window, 25, 89, GAMETEXT_MONO, (v_window->getPosition().nWidth-40)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("MONO");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(10025);
  v_button->setContextHelp(CONTEXTHELP_MONO);
    
  v_button = new UIButton(v_window, 25+(v_window->getPosition().nWidth-40)/3, 89, GAMETEXT_STEREO,
			  (v_window->getPosition().nWidth-40)/3, 28);
  v_button->setType(UI_BUTTON_TYPE_RADIO);
  v_button->setID("STEREO");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(10025);
  v_button->setContextHelp(CONTEXTHELP_STEREO);

  v_button = new UIButton(v_window, 5, 117, GAMETEXT_ENABLEENGINESOUND, v_window->getPosition().nWidth-10, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLE_ENGINE_SOUND");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_ENGINE_SOUND);
    
  v_button = new UIButton(v_window, 5, 145, GAMETEXT_ENABLEMUSIC, v_window->getPosition().nWidth-10, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLE_MENU_MUSIC");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_MUSIC);

  return v_window;
}

UIWindow* StateMainMenu::makeWindowOptions_controls(UIWindow* i_parent) {
  UIWindow*  v_window;
  UIList*    v_list;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  v_window = new UIWindow(i_parent, 20, 40, GAMETEXT_CONTROLS, i_parent->getPosition().nWidth-40, i_parent->getPosition().nHeight);
  v_window->setID("CONTROLS_TAB");
  v_window->showWindow(false);

//  v_button = new UIButton(v_window, 5, 5, GAMETEXT_KEYBOARD, (v_window->getPosition().nWidth-40)/2, 28);
//  v_button->setType(UI_BUTTON_TYPE_RADIO);
//  v_button->setID("KEYBOARD");
//  v_button->setFont(drawlib->getFontSmall());
//  v_button->setGroup(200243);
//
//  v_button = new UIButton(v_window, 5+(v_window->getPosition().nWidth-40)/2, 5, GAMETEXT_JOYSTICK, (v_window->getPosition().nWidth-40)/2, 28);
//  v_button->setType(UI_BUTTON_TYPE_RADIO);
//  v_button->setID("JOYSTICK");
//  v_button->setFont(drawlib->getFontSmall());
//  v_button->setGroup(200243);    
//
  v_list = new UIList(v_window, 5, 5, "", v_window->getPosition().nWidth-10, v_window->getPosition().nHeight -43 -10 -10);
  v_list->setID("KEY_ACTION_LIST");
  v_list->setFont(drawlib->getFontSmall());
  v_list->addColumn(GAMETEXT_ACTION, v_list->getPosition().nWidth/2);
  v_list->addColumn(GAMETEXT_KEY, v_list->getPosition().nWidth/2);
  v_list->setContextHelp(CONTEXTHELP_SELECT_ACTION);

//  v_button = new UIButton(v_window, 0, 180, GAMETEXT_CONFIGUREJOYSTICK, 207, 57);
//  v_button->setType(UI_BUTTON_TYPE_LARGE);
//  v_button->setID("CONFIGURE_JOYSTICK");
//  v_button->setFont(drawlib->getFontSmall());

  return v_window;
}

UIWindow* StateMainMenu::makeWindowOptions_rooms(UIWindow* i_parent) {
  UIWindow *v_window, *v_wwwwindow;
  UIStatic* v_someText;
  UIEdit*   v_edit;
  UIButton* v_button;
  DrawLib*  drawlib = GameApp::instance()->getDrawLib();

  v_wwwwindow = new UIWindow(i_parent, 0, 26, GAMETEXT_WWWTAB, i_parent->getPosition().nWidth, i_parent->getPosition().nHeight-20);
  v_wwwwindow->setID("WWW_TAB");
  v_wwwwindow->showWindow(false);

  UITabView *v_roomsTabs = new UITabView(v_wwwwindow, 0, 0, "", v_wwwwindow->getPosition().nWidth, v_wwwwindow->getPosition().nHeight);
  v_roomsTabs->setID("TABS");
  v_roomsTabs->setFont(drawlib->getFontSmall());
  v_roomsTabs->setTabContextHelp(0, CONTEXTHELP_WWW_MAIN_TAB);
  v_roomsTabs->setTabContextHelp(1, CONTEXTHELP_WWW_ROOMS_TAB);

  v_window = new UIWindow(v_roomsTabs, 20, 30, GAMETEXT_WWWMAINTAB, v_roomsTabs->getPosition().nWidth-30,
			  v_roomsTabs->getPosition().nHeight);
  v_window->setID("MAIN_TAB");

  v_button = new UIButton(v_window, 5, 0, GAMETEXT_ENABLEWEBHIGHSCORES, (v_window->getPosition().nWidth-40), 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLEWEB");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50123);
  v_button->setContextHelp(CONTEXTHELP_DOWNLOAD_BEST_TIMES);

  // password
  char buf[256];
  snprintf(buf, 256, GAMETEXT_ACCOUNT_PASSWORD, XMSession::instance()->profile().c_str());
  v_someText = new UIStatic(v_window, 35, 25, buf, 300, 30);
  v_someText->setID("WWW_PASSWORD_STATIC");
  v_someText->setHAlign(UI_ALIGN_LEFT);
  v_someText->setFont(drawlib->getFontSmall()); 

  v_edit = new UIEdit(v_window, 35, 50, "", 150, 25);
  v_edit->setFont(drawlib->getFontSmall());
  v_edit->setID("WWW_PASSWORD");
  v_edit->hideText(true);
  v_edit->setContextHelp(CONTEXTHELP_WWW_PASSWORD);

  v_button = new UIButton(v_window, 5, 80, GAMETEXT_ENABLECHECKNEWLEVELSATSTARTUP,v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLECHECKNEWLEVELSATSTARTUP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50123);
  v_button->setContextHelp(CONTEXTHELP_ENABLE_CHECK_NEW_LEVELS_AT_STARTUP);
  
  v_button = new UIButton(v_window, 5, 110, GAMETEXT_ENABLECHECKHIGHSCORESATSTARTUP, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLECHECKHIGHSCORESATSTARTUP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50123);
  v_button->setContextHelp(CONTEXTHELP_ENABLE_CHECK_HIGHSCORES_AT_STARTUP);

  v_button = new UIButton(v_window, 5, 140, GAMETEXT_ENABLEINGAMEWORLDRECORD, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("INGAMEWORLDRECORD");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50123);
  v_button->setContextHelp(CONTEXTHELP_INGAME_WORLD_RECORD);

  v_button = new UIButton(v_window, 5, 170, GAMETEXT_USECRAPPYINFORMATION, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("USECRAPPYINFORMATION");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50123);
  v_button->setContextHelp(CONTEXTHELP_USECRAPPYINFORMATION);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth-225, v_window->getPosition().nHeight -57 - 20 -20,
			  GAMETEXT_PROXYCONFIG, 207, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("PROXYCONFIG");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_PROXYCONFIG);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth-225-200, v_window->getPosition().nHeight -57 - 20 -20,
			  GAMETEXT_UPDATEHIGHSCORES, 207, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("UPDATEHIGHSCORES");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_UPDATEHIGHSCORES);


  for(unsigned int i=0; i<ROOMS_NB_MAX; i++) {
    v_window = makeRoomTab(v_roomsTabs, i);
  }

  return v_wwwwindow;
}

UIWindow* StateMainMenu::makeRoomTab(UIWindow* i_parent, unsigned int i_number) {
  UIWindow* v_window;
  UIList*   v_list;
  UIButton*  v_button;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  std::string v_roomTitle;
  std::ostringstream v_strRoom;
  v_strRoom << i_number;

  if(i_number == 0) {
    v_roomTitle = GAMETEXT_WWWROOMSTAB_REFERENCE;
  } else {
    char v_ctmp[64];
    snprintf(v_ctmp, 64, GAMETEXT_WWWROOMSTAB_OTHER, i_number + 1);
    v_roomTitle = v_ctmp;
  }

  v_window = new UIWindow(i_parent, 4, 25, v_roomTitle,
			  i_parent->getPosition().nWidth-20, i_parent->getPosition().nHeight - 15);
  v_window->setID("ROOMS_TAB_" + v_strRoom.str());
  v_window->showWindow(false);

  if(i_number != 0) {
    v_button = new UIButton(v_window, 0, 0, "", 40, 28);
    v_button->setType(UI_BUTTON_TYPE_CHECK);
    v_button->setID("ROOM_ENABLED");
    v_button->setFont(drawlib->getFontSmall());
    v_button->setContextHelp(CONTEXTHELP_ROOM_ENABLE);
  }

  v_list = new UIList(v_window, 20, 25, "", v_window->getPosition().nWidth-215 - 20 - 20, v_window->getPosition().nHeight - 20 - 50);
  v_list->setID("ROOMS_LIST");
  v_list->setFont(drawlib->getFontSmall());
  v_list->addColumn(GAMETEXT_ROOM, v_list->getPosition().nWidth);
  v_list->setContextHelp(CONTEXTHELP_WWW_ROOMS_LIST);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth - 215, 25 + 57*0, GAMETEXT_UPDATEROOMSSLIST, 215, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("UPDATE_ROOMS_LIST");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_UPDATEROOMSLIST);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth - 215, 25 + 57*1, GAMETEXT_UPLOAD_ALL_HIGHSCORES, 215, 57);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("UPLOADHIGHSCOREALL_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_UPLOAD_HIGHSCORE_ALL);	

  return v_window;
}

UIWindow* StateMainMenu::makeWindowOptions_ghosts(UIWindow* i_parent) {
  UIWindow*  v_window;
  UIButton*  v_button;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  v_window = new UIWindow(i_parent, 20, 40, GAMETEXT_GHOSTTAB, i_parent->getPosition().nWidth-40, i_parent->getPosition().nHeight);
  v_window->setID("GHOSTS_TAB");
  v_window->showWindow(false);

  v_button = new UIButton(v_window, 5, 5, GAMETEXT_ENABLEGHOST, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLE_GHOSTS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GHOST_MODE);

  v_button = new UIButton(v_window, 5+20, 34, GAMETEXT_GHOST_STRATEGY_MYBEST, v_window->getPosition().nWidth-40,28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("GHOST_STRATEGY_MYBEST");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GHOST_STRATEGY_MYBEST);

  v_button = new UIButton(v_window, 5+20, 63, GAMETEXT_GHOST_STRATEGY_THEBEST, v_window->getPosition().nWidth-40,28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("GHOST_STRATEGY_THEBEST");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GHOST_STRATEGY_THEBEST);

  v_button = new UIButton(v_window, 5+20, 92, GAMETEXT_GHOST_STRATEGY_BESTOFREFROOM, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("GHOST_STRATEGY_BESTOFREFROOM");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GHOST_STRATEGY_BESTOFREFROOM);

  v_button = new UIButton(v_window, 5+20, 121, GAMETEXT_GHOST_STRATEGY_BESTOFOTHERROOMS, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("GHOST_STRATEGY_BESTOFOTHERROOMS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GHOST_STRATEGY_BESTOFOTHERROOMS);

  v_button = new UIButton(v_window, 5, 150, GAMETEXT_DISPLAYGHOSTTIMEDIFF, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("DISPLAY_GHOST_TIMEDIFFERENCE");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_DISPLAY_GHOST_TIMEDIFF);

  v_button = new UIButton(v_window, 5, 179, GAMETEXT_HIDEGHOSTS, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("HIDEGHOSTS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_HIDEGHOSTS);

  v_button = new UIButton(v_window, 5, 208, GAMETEXT_DISPLAYGHOSTINFO, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("DISPLAY_GHOSTS_INFOS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_DISPLAY_GHOST_INFO);

  v_button = new UIButton(v_window, 5, 237, GAMETEXT_MOTIONBLURGHOST, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("MOTION_BLUR_GHOST");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_MOTIONBLURGHOST);

  return v_window;
}

UIWindow* StateMainMenu::makeWindowOptions_language(UIWindow* i_parent) {
  UIWindow*  v_window;
  UIList* v_list;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();
  UIListEntry *pEntry;
  unsigned int n = 0;

  v_window = new UIWindow(i_parent, 20, 40, GAMETEXT_LANGUAGETAB, i_parent->getPosition().nWidth-40, i_parent->getPosition().nHeight);
  v_window->setID("LANGUAGE_TAB");
  v_window->showWindow(false);

  /* list */
  v_list = new UIList(v_window, 0, 0, "", v_window->getPosition().nWidth, v_window->getPosition().nHeight-40-20);
  v_list->setID("LANGUAGE_LIST");
  v_list->setFont(drawlib->getFontSmall());
  v_list->addColumn(GAMETEXT_LANGUAGE_NAME, v_list->getPosition().nWidth - 160, CONTEXTHELP_LANGUAGE_NAME);
  v_list->addColumn(GAMETEXT_LANGUAGE_CODE,160,CONTEXTHELP_LANGUAGE_CODE);

  pEntry = v_list->addEntry(GAMETEXT_AUTOMATIC, NULL);
  pEntry->Text.push_back("");
  if(XMSession::instance()->language() == "") v_list->setRealSelected(n);
  n++;

  for(unsigned int i=0; i<NB_LANGUAGES; i++) {
    pEntry = v_list->addEntry(LANGUAGES[i][LANGUAGE_NAME], NULL);
    pEntry->Text.push_back(LANGUAGES[i][LANGUAGE_CODE]);
    if(XMSession::instance()->language() == LANGUAGES[i][LANGUAGE_CODE]) v_list->setRealSelected(i+1);
  }
  return v_window;
}



UIWindow* StateMainMenu::makeWindowOptions(UIWindow* i_parent) {
  UIWindow *v_window, *v_frame;
  UIStatic*  v_someText;
  UITabView* v_tabview;
  UIButton*  v_button;
  DrawLib* drawlib = GameApp::instance()->getDrawLib();

  v_window = new UIFrame(i_parent, 220, i_parent->getPosition().nHeight*7/30, "",
			 i_parent->getPosition().nWidth -220 -20,
			 i_parent->getPosition().nHeight -40 -i_parent->getPosition().nHeight/5 -10);
  v_window->setID("FRAME_OPTIONS");
  v_window->showWindow(false);
   
  v_someText = new UIStatic(v_window, 0, 0, GAMETEXT_OPTIONS, v_window->getPosition().nWidth, 36);
  v_someText->setFont(drawlib->getFontMedium());

  v_tabview  = new UITabView(v_window, 20, 40, "", v_window->getPosition().nWidth-40, v_window->getPosition().nHeight-115);
  v_tabview->setID("TABS");
  v_tabview->setFont(drawlib->getFontSmall());
  v_tabview->setTabContextHelp(0, CONTEXTHELP_GENERAL_OPTIONS);
  v_tabview->setTabContextHelp(1, CONTEXTHELP_VIDEO_OPTIONS);
  v_tabview->setTabContextHelp(2, CONTEXTHELP_AUDIO_OPTIONS);
  v_tabview->setTabContextHelp(3, CONTEXTHELP_CONTROL_OPTIONS);
  v_tabview->setTabContextHelp(4, CONTEXTHELP_LANGUAGE_OPTIONS);

  v_frame = makeWindowOptions_general(v_tabview);
  v_frame = makeWindowOptions_video(v_tabview);
  v_frame = makeWindowOptions_audio(v_tabview);
  v_frame = makeWindowOptions_controls(v_tabview);
  v_frame = makeWindowOptions_rooms(v_tabview);
  v_frame = makeWindowOptions_ghosts(v_tabview);
  v_frame = makeWindowOptions_language(v_tabview);

  v_button = new UIButton(v_window, 20, v_window->getPosition().nHeight-68, GAMETEXT_DEFAULTS, 115, 57);
  v_button->setID("DEFAULTS_BUTTON");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_SMALL);      
  v_button->setContextHelp(CONTEXTHELP_DEFAULTS);

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

void StateMainMenu::send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  if(i_id == "QUIT") {
    if(i_button == UI_MSGBOX_YES) {
      m_requestForEnd = true;
      GameApp::instance()->requestEnd();
    }
  }

  if(i_id == "REPLAYS_DELETE") {
    if(i_button == UI_MSGBOX_YES) {
      m_commands.push("REPLAYS_DELETE");
    }
  }
}

void StateMainMenu::send(const std::string& i_id, const std::string& i_message) {
  UIList* v_list;

  if(i_id == "STATE_MANAGER") {
    if(i_message == "FAVORITES_UPDATED") {
      if(StateManager::instance()->isTopOfTheStates(this)) {
	updateFavoriteLevelsList();
      } else {
	m_require_updateFavoriteLevelsList = true;
      }
      return;
    }
    
   if(i_message == "REPLAYS_UPDATED") {
     if(StateManager::instance()->isTopOfTheStates(this)) {
       updateReplaysList();
     } else {
       m_require_updateReplaysList = true;
     }
     return;
    }

   if(i_message == "STATS_UPDATED") {
     if(StateManager::instance()->isTopOfTheStates(this)) {
       updateLevelsPacksList();
       updateLevelsLists();
       updateReplaysList();
       updateStats();
     } else {
       m_require_updateStats = true;
     }
     return;
    }

   if(i_message == "LEVELS_UPDATED" || i_message == "HIGHSCORES_UPDATED" || i_message == "BLACKLISTEDLEVELS_UPDATED") {
     if(StateManager::instance()->isTopOfTheStates(this)) {
       LevelsManager::instance()->makePacks(XMSession::instance()->profile(),
					    XMSession::instance()->idRoom(0),
					    XMSession::instance()->debug(),
					    xmDatabase::instance("main"));
       updateLevelsPacksList();
       updateLevelsLists();
     } else {
       m_require_updateLevelsList = true;
     }
     return;
   }

  }

  if(i_id == "REQUESTKEY") {
    v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:CONTROLS_TAB:KEY_ACTION_LIST"));
    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {    
      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];

      // is key used
      for(unsigned int i=0;i<v_list->getEntries().size();i++) {
	if(v_list->getEntries()[i]->Text[1] == i_message) {
	  // switch keys
	  v_list->getEntries()[i]->Text[1]  = pEntry->Text[1];
	  setInputKey(v_list->getEntries()[i]->Text[0], v_list->getEntries()[i]->Text[1]);
	}
      }
      pEntry->Text[1] = i_message;
      setInputKey(pEntry->Text[0], i_message);
    }
    return;
  }

  StateMenu::send(i_id, i_message);
}

void StateMainMenu::setInputKey(const std::string& i_strKey, const std::string& i_key) {
  std::string n;

  for(int i=0; i<4; i++) {
    n = "";

    switch(i) {
    case 0:
      n = "";
      break;
    case 1:
      n = " 2";
      break;
    case 2:
      n = " 3";
      break;
    case 3:
      n = " 4";
      break;
    }

    if(i_strKey == GAMETEXT_DRIVE + n) {
      InputHandler::instance()->setDRIVE(i, InputHandler::stringToKey(i_key));
    }
    
    if(i_strKey == GAMETEXT_BRAKE + n) {
      InputHandler::instance()->setBRAKE(i, InputHandler::stringToKey(i_key));
    }
    
    if(i_strKey == GAMETEXT_FLIPLEFT + n) {
      InputHandler::instance()->setFLIPLEFT(i, InputHandler::stringToKey(i_key));
    }
    
    if(i_strKey == GAMETEXT_FLIPRIGHT + n) {
      InputHandler::instance()->setFLIPRIGHT(i, InputHandler::stringToKey(i_key));
    }
    
    if(i_strKey == GAMETEXT_CHANGEDIR + n) {
      InputHandler::instance()->setCHANGEDIR(i, InputHandler::stringToKey(i_key));
    }
  }
}

void StateMainMenu::executeOneCommand(std::string cmd)
{
  UIListEntry *pEntry = NULL;

  if(cmd == "UPDATEPROFILE") {
    updateProfileStrings();
    updateOptions();

    // update packs
    LevelsManager::instance()->makePacks(XMSession::instance()->profile(),
					 XMSession::instance()->idRoom(0),
					 XMSession::instance()->debug(),
					 xmDatabase::instance("main"));

    // update lists and stats
    updateLevelsPacksList();
    updateLevelsLists();
    updateReplaysList();
    updateStats();
    return;
  }

   else if(cmd == "REPLAYS_DELETE") {
    UIList* v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_LIST"));
    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      pEntry = v_list->getEntries()[v_list->getSelected()];
      if(pEntry != NULL) {
	try {
	  Replay::deleteReplay(pEntry->Text[0]);
	} catch(Exception &e) {
	  Logger::Log(e.getMsg().c_str());
	}
	try {
	  xmDatabase::instance("main")->replays_delete(pEntry->Text[0]);
	} catch(Exception &e) {
	  Logger::Log(e.getMsg().c_str());
	}
	StateManager::instance()->sendAsynchronousMessage("REPLAYS_UPDATED");
      }
    }
  }

  else if(cmd == "UPDATE_THEMES_LISTS") {
    updateThemesList();
  }

  else if(cmd == "UPDATE_ROOMS_LISTS") {
    updateRoomsList();      
  }

  else if(cmd == "CHANGE_WWW_ACCESS") {
    updateWWWOptions();
  }

  else if(cmd == "ENABLEAUDIO_CHANGED") {
    updateAudioOptions();
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
}

void StateMainMenu::updateLevelsLists() {
  updateFavoriteLevelsList();
  updateNewLevelsList();
}

void StateMainMenu::createLevelLists(UILevelList *i_list, const std::string& i_packageName) {
  LevelsPack *v_levelsPack = &(LevelsManager::instance()->LevelsPackByName(i_packageName));
  createLevelListsSql(i_list, v_levelsPack->getLevelsWithHighscoresQuery(XMSession::instance()->profile(),
									 XMSession::instance()->idRoom(0)));
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
    
  for(unsigned int i=0; i<v_lm->LevelsPacks().size(); i++) {
    /* the unpackaged pack exists only in debug mode */
    if(v_lm->LevelsPacks()[i]->Name() != "" || XMSession::instance()->debug()) {
      if(v_lm->LevelsPacks()[i]->Name() == "") {
	v_lm->LevelsPacks()[i]->setName(GAMETEXT_UNPACKED_LEVELS_PACK);
      }
	
      pTree->addPack(v_lm->LevelsPacks()[i],
		     v_lm->LevelsPacks()[i]->Group(),
		     v_lm->LevelsPacks()[i]->getNumberOfFinishedLevels(xmDatabase::instance("main"),
								       XMSession::instance()->profile()),
		     v_lm->LevelsPacks()[i]->getNumberOfLevels(xmDatabase::instance("main"))
		     );
      
    }
  }
  
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

void StateMainMenu::createThemesList(UIList *pList) {
  char **v_result;
  unsigned int nrow;
  UIListEntry *pEntry;
  std::string v_id_theme;
  std::string v_ck1, v_ck2;
  
  /* get selected item */
  std::string v_selected_themeName = "";
  if(pList->getSelected() >= 0 && pList->getSelected() < pList->getEntries().size()) {
    UIListEntry *pEntry = pList->getEntries()[pList->getSelected()];
    v_selected_themeName = pEntry->Text[0];
  }
  
  /* recreate the list */
  pList->clear();
  
  v_result = xmDatabase::instance("main")->readDB("SELECT a.id_theme, a.checkSum, b.checkSum "
				      "FROM themes AS a LEFT OUTER JOIN webthemes AS b "
				      "ON a.id_theme=b.id_theme ORDER BY a.id_theme;",
			  nrow);
  for(unsigned int i=0; i<nrow; i++) {
    v_id_theme = xmDatabase::instance("main")->getResult(v_result, 3, i, 0);
    v_ck1      = xmDatabase::instance("main")->getResult(v_result, 3, i, 1);
    if(xmDatabase::instance("main")->getResult(v_result, 3, i, 2) != NULL) {
      v_ck2      = xmDatabase::instance("main")->getResult(v_result, 3, i, 2);
    }
    
    pEntry = pList->addEntry(v_id_theme.c_str(),
			     NULL);
    if(v_ck1 == v_ck2 || v_ck2 == "") {
      pEntry->Text.push_back(GAMETEXT_THEMEHOSTED);
    } else {
      pEntry->Text.push_back(GAMETEXT_THEMEREQUIREUPDATE);
    }
  }
  xmDatabase::instance("main")->read_DB_free(v_result);
  
  v_result = xmDatabase::instance("main")->readDB("SELECT a.id_theme FROM webthemes AS a LEFT OUTER JOIN themes AS b "
				      "ON a.id_theme=b.id_theme WHERE b.id_theme IS NULL;",
				      nrow);
  for(unsigned int i=0; i<nrow; i++) {
    v_id_theme = xmDatabase::instance("main")->getResult(v_result, 1, i, 0);
    pEntry = pList->addEntry(v_id_theme.c_str(), NULL);
    pEntry->Text.push_back(GAMETEXT_THEMENOTHOSTED);
  }
  xmDatabase::instance("main")->read_DB_free(v_result);
  
  /* reselect the previous theme */
  if(v_selected_themeName != "") {
    int nTheme = 0;
    for(unsigned int i=0; i<pList->getEntries().size(); i++) {
      if(pList->getEntries()[i]->Text[0] == v_selected_themeName) {
	nTheme = i;
	break;
      }
    }
    pList->setRealSelected(nTheme);
  }  
}

void StateMainMenu::updateThemesList() {
  UIList* v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:THEMES_LIST"));
  createThemesList(v_list);

  std::string v_themeName = XMSession::instance()->theme();
  int nTheme = 0;
  for(unsigned int i=0; i<v_list->getEntries().size(); i++) {
    if(v_list->getEntries()[i]->Text[0] == v_themeName) {
      nTheme = i;
      break;
    }
  }
  v_list->setRealSelected(nTheme); 
}

void StateMainMenu::updateResolutionsList() {
  UIList* v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:RESOLUTIONS_LIST"));
  char cBuf[256];
  snprintf(cBuf, 256, "%d X %d", XMSession::instance()->resolutionWidth(), XMSession::instance()->resolutionHeight());
  int nMode = 0;

  v_list->clear();
  std::vector<std::string>* modes = System::getDisplayModes(XMSession::instance()->windowed());
  for(unsigned int i=0; i < modes->size(); i++) {
    v_list->addEntry((*modes)[i].c_str());
    if(std::string((*modes)[i]) == std::string(cBuf)) {
      nMode = i;
    }
  }
  delete modes;
  v_list->setRealSelected(nMode);
}

void StateMainMenu::updateControlsList() {
  UIList *pList = (UIList *)m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:CONTROLS_TAB:KEY_ACTION_LIST");
  pList->clear();
    
  UIListEntry *p;

  p = pList->addEntry(GAMETEXT_DRIVE); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getDRIVE(0)));
  p = pList->addEntry(GAMETEXT_BRAKE); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getBRAKE(0)));
  p = pList->addEntry(GAMETEXT_FLIPLEFT); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getFLIPLEFT(0)));
  p = pList->addEntry(GAMETEXT_FLIPRIGHT); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getFLIPRIGHT(0)));
  p = pList->addEntry(GAMETEXT_CHANGEDIR); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getCHANGEDIR(0)));

  p = pList->addEntry(GAMETEXT_DRIVE     + std::string(" 2")); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getDRIVE(1)));
  p = pList->addEntry(GAMETEXT_BRAKE     + std::string(" 2")); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getBRAKE(1)));
  p = pList->addEntry(GAMETEXT_FLIPLEFT  + std::string(" 2")); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getFLIPLEFT(1)));
  p = pList->addEntry(GAMETEXT_FLIPRIGHT + std::string(" 2")); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getFLIPRIGHT(1)));
  p = pList->addEntry(GAMETEXT_CHANGEDIR + std::string(" 2")); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getCHANGEDIR(1)));

  p = pList->addEntry(GAMETEXT_DRIVE     + std::string(" 3")); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getDRIVE(2)));
  p = pList->addEntry(GAMETEXT_BRAKE     + std::string(" 3")); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getBRAKE(2)));
  p = pList->addEntry(GAMETEXT_FLIPLEFT  + std::string(" 3")); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getFLIPLEFT(2)));
  p = pList->addEntry(GAMETEXT_FLIPRIGHT + std::string(" 3")); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getFLIPRIGHT(2)));
  p = pList->addEntry(GAMETEXT_CHANGEDIR + std::string(" 3")); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getCHANGEDIR(2)));

  p = pList->addEntry(GAMETEXT_DRIVE     + std::string(" 4")); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getDRIVE(3)));
  p = pList->addEntry(GAMETEXT_BRAKE     + std::string(" 4")); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getBRAKE(3)));
  p = pList->addEntry(GAMETEXT_FLIPLEFT  + std::string(" 4")); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getFLIPLEFT(3)));
  p = pList->addEntry(GAMETEXT_FLIPRIGHT + std::string(" 4")); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getFLIPRIGHT(3)));
  p = pList->addEntry(GAMETEXT_CHANGEDIR + std::string(" 4")); p->Text.push_back(InputHandler::keyToString(InputHandler::instance()->getCHANGEDIR(3)));
}

void StateMainMenu::createRoomsList(UIList *pList) {
  UIListEntry *pEntry;
  std::string v_selected_roomName = "";
  char **v_result;
  unsigned int nrow;
  std::string v_roomName, v_roomId;

  /* get selected item */
  if(pList->getSelected() >= 0 && pList->getSelected() < pList->getEntries().size()) {
    UIListEntry *pEntry = pList->getEntries()[pList->getSelected()];
    v_selected_roomName = pEntry->Text[0];
  }

  /* recreate the list */
  for(unsigned int i=0; i<pList->getEntries().size(); i++) {
    delete reinterpret_cast<std::string *>(pList->getEntries()[i]->pvUser);
  }
  pList->clear();

  // WR room
  v_result = xmDatabase::instance("main")->readDB("SELECT id_room, name FROM webrooms WHERE id_room = 1;", nrow);
  for(unsigned int i=0; i<nrow; i++) {
    v_roomId   = xmDatabase::instance("main")->getResult(v_result, 2, i, 0);
    v_roomName = xmDatabase::instance("main")->getResult(v_result, 2, i, 1);
    pEntry = pList->addEntry(v_roomName, reinterpret_cast<void *>(new std::string(v_roomId)));    
  }
  xmDatabase::instance("main")->read_DB_free(v_result);

  v_result = xmDatabase::instance("main")->readDB("SELECT id_room, name FROM webrooms ORDER BY UPPER(name);", nrow);
  for(unsigned int i=0; i<nrow; i++) {
    v_roomId   = xmDatabase::instance("main")->getResult(v_result, 2, i, 0);
    v_roomName = xmDatabase::instance("main")->getResult(v_result, 2, i, 1);
    pEntry = pList->addEntry(v_roomName, reinterpret_cast<void *>(new std::string(v_roomId)));    
  }
  xmDatabase::instance("main")->read_DB_free(v_result);

  /* reselect the previous room */
  if(v_selected_roomName != "") {
    int nRoom = 0;
    for(unsigned int i=0; i<pList->getEntries().size(); i++) {
      if(pList->getEntries()[i]->Text[0] == v_selected_roomName) {
	nRoom = i;
	break;
      }
    }
    pList->setRealSelected(nRoom);
  }
}

void StateMainMenu::updateRoomsList() {
  UIList* v_list;
  unsigned int j;
  bool v_found;

  for(unsigned int i=0; i<ROOMS_NB_MAX; i++) {
      std::ostringstream v_strRoom;
      v_strRoom << i;

      v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							  + ":ROOMS_LIST"));
      createRoomsList(v_list);

      std::string v_id_room = XMSession::instance()->idRoom(i);
      if(v_id_room == "") { v_id_room = DEFAULT_WEBROOM_ID; }
      j = 0;
      v_found = false;
      while(j<v_list->getEntries().size() && v_found == false) {
	if((*(std::string*)v_list->getEntries()[j]->pvUser) == v_id_room) {
	  v_list->setRealSelected(j);
	  v_found = true;
	}
	j++;
      }
  }
}


void StateMainMenu::updateOptions() {
  UIButton* v_button;
  UIEdit*   v_edit;

  // level tab (multi)
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:MULTI_TAB:ENABLEMULTISTOPWHENONEFINISHES"));
  v_button->setChecked(XMSession::instance()->MultiStopWhenOneFinishes());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:MULTI_TAB:MULTISCENES"));
  v_button->setChecked(XMSession::instance()->multiScenes() == false);

  // options/general
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:SHOWMINIMAP"));
  v_button->setChecked(XMSession::instance()->showMinimap());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:SHOWENGINECOUNTER"));
  v_button->setChecked(XMSession::instance()->showEngineCounter());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:INITZOOM"));
  v_button->setChecked(XMSession::instance()->enableInitZoom());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:DEATHANIM"));
  v_button->setChecked(XMSession::instance()->enableDeadAnimation());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:CAMERAACTIVEZOOM"));
  v_button->setChecked(XMSession::instance()->enableActiveZoom());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:ENABLECONTEXTHELP"));
  v_button->setChecked(XMSession::instance()->enableContextHelp());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:AUTOSAVEREPLAYS"));
  v_button->setChecked(XMSession::instance()->autosaveHighscoreReplays());

  // video
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:16BPP"));
  v_button->setChecked(XMSession::instance()->bpp() == 16);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:32BPP"));
  v_button->setChecked(XMSession::instance()->bpp() == 32); 

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:WINDOWED"));
  v_button->setChecked(XMSession::instance()->windowed());

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:MENULOW"));
  v_button->setChecked(XMSession::instance()->menuGraphics() == GFX_LOW);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:MENUMEDIUM"));
  v_button->setChecked(XMSession::instance()->menuGraphics() == GFX_MEDIUM);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:MENUHIGH"));
  v_button->setChecked(XMSession::instance()->menuGraphics() == GFX_HIGH);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:GAMELOW"));
  v_button->setChecked(XMSession::instance()->gameGraphics() == GFX_LOW);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:GAMEMEDIUM"));
  v_button->setChecked(XMSession::instance()->gameGraphics() == GFX_MEDIUM);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:GAMEHIGH"));
  v_button->setChecked(XMSession::instance()->gameGraphics() == GFX_HIGH);

  // audio
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_AUDIO"));
  v_button->setChecked(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE11KHZ"));
  v_button->setChecked(XMSession::instance()->audioSampleRate() == 11025);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE22KHZ"));
  v_button->setChecked(XMSession::instance()->audioSampleRate() == 22050);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE44KHZ"));
  v_button->setChecked(XMSession::instance()->audioSampleRate() == 44100);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:8BITS"));
  v_button->setChecked(XMSession::instance()->audioSampleBits() == 8);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:16BITS"));
  v_button->setChecked(XMSession::instance()->audioSampleBits() == 16);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:MONO"));
  v_button->setChecked(XMSession::instance()->audioChannels() == 1);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:STEREO"));
  v_button->setChecked(XMSession::instance()->audioChannels() == 2);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_ENGINE_SOUND"));
  v_button->setChecked(XMSession::instance()->enableEngineSound());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_MENU_MUSIC"));
  v_button->setChecked(XMSession::instance()->enableMenuMusic());

  // controls

  // www
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLEWEB"));
  v_button->setChecked(XMSession::instance()->www());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKNEWLEVELSATSTARTUP"));
  v_button->setChecked(XMSession::instance()->checkNewLevelsAtStartup());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKHIGHSCORESATSTARTUP"));
  v_button->setChecked(XMSession::instance()->checkNewHighscoresAtStartup());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:INGAMEWORLDRECORD"));
  v_button->setChecked(XMSession::instance()->showHighscoreInGame());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:USECRAPPYINFORMATION"));
  v_button->setChecked(XMSession::instance()->useCrappyPack());

  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:WWW_PASSWORD"));
  v_edit->setCaption(XMSession::instance()->wwwPassword());

  for(unsigned int i=0; i<ROOMS_NB_MAX; i++) {
      bool v_enabled = true;
      std::ostringstream v_strRoom;
      v_strRoom << i;

      if(i != 0) {
	v_enabled = XMSession::instance()->nbRoomsEnabled() > i;
	v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
								+ ":ROOM_ENABLED"));
	v_button->setChecked(v_enabled);
      }
   }

  // ghosts
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:ENABLE_GHOSTS"));
  v_button->setChecked(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_MYBEST"));
  v_button->setChecked(XMSession::instance()->ghostStrategy_MYBEST());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_THEBEST"));
  v_button->setChecked(XMSession::instance()->ghostStrategy_THEBEST());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_BESTOFREFROOM"));
  v_button->setChecked(XMSession::instance()->ghostStrategy_BESTOFREFROOM());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_BESTOFOTHERROOMS"));
  v_button->setChecked(XMSession::instance()->ghostStrategy_BESTOFOTHERROOMS());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:DISPLAY_GHOST_TIMEDIFFERENCE"));
  v_button->setChecked(XMSession::instance()->showGhostTimeDifference());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:DISPLAY_GHOSTS_INFOS"));
  v_button->setChecked(XMSession::instance()->showGhostsInfos());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:HIDEGHOSTS"));
  v_button->setChecked(XMSession::instance()->hideGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:MOTION_BLUR_GHOST"));
  v_button->setChecked(XMSession::instance()->ghostMotionBlur());

  // update rights on the options
  updateAudioOptions();
  updateWWWOptions();
  updateGhostsOptions();

  // update lists in options
  updateThemesList();
  updateResolutionsList();
  updateControlsList();
  updateRoomsList();
}

void StateMainMenu::checkEventsOptions() {
  UIButton*    v_button;
  UIEdit*      v_edit;
  UIList*      v_list;
  std::string  v_id_level;

  // general tab
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:SHOWMINIMAP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setShowMinimap(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:SHOWENGINECOUNTER"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setShowEngineCounter(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:INITZOOM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setEnableInitZoom(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:DEATHANIM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setEnableDeadAnimation(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:CAMERAACTIVEZOOM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setEnableActiveZoom(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:ENABLECONTEXTHELP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setEnableContextHelp(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:AUTOSAVEREPLAYS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAutosaveHighscoreReplays(v_button->getChecked()); 
  }

  v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:THEMES_LIST"));
  if(v_list->isClicked()) {
    v_list->setClicked(false);
    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      XMSession::instance()->setTheme(pEntry->Text[0]);
      if(Theme::instance()->Name() != XMSession::instance()->theme()) {
	GameApp::instance()->reloadTheme();
      }
    }
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:UPDATE_THEMES_LIST"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    StateManager::instance()->pushState(new StateUpdateThemesList());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:GET_SELECTED_THEME"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      StateManager::instance()->pushState(new StateUpdateTheme(pEntry->Text[0]));
    }
  }

  // video tab
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:16BPP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setBpp(16); 
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:32BPP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setBpp(32); 
  }

  v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:RESOLUTIONS_LIST"));
  if(v_list->isClicked()) {
    v_list->setClicked(false);
    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      int nW,nH;

      sscanf(pEntry->Text[0].c_str(),"%d X %d", &nW, &nH);
      XMSession::instance()->setResolutionWidth(nW);
      XMSession::instance()->setResolutionHeight(nH);
    }
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:WINDOWED"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setWindowed(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:MENULOW"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setMenuGraphics(GFX_LOW);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:MENUMEDIUM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setMenuGraphics(GFX_MEDIUM);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:MENUHIGH"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setMenuGraphics(GFX_HIGH);
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:GAMELOW"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGameGraphics(GFX_LOW);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:GAMEMEDIUM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGameGraphics(GFX_MEDIUM);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:GAMEHIGH"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGameGraphics(GFX_HIGH);
  }

  // sound
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_AUDIO"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    XMSession::instance()->enableAudio();
    XMSession::instance()->setEnableAudio(v_button->getChecked());
    Sound::setActiv(XMSession::instance()->enableAudio());
    updateAudioOptions();
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE11KHZ"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAudioSampleRate(11025);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE22KHZ"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAudioSampleRate(22050);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE44KHZ"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAudioSampleRate(44100);
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:8BITS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAudioSampleBits(8);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:16BITS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAudioSampleBits(16);
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:MONO"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAudioChannels(1);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:STEREO"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setAudioChannels(2);
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_ENGINE_SOUND"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setEnableEngineSound(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_MENU_MUSIC"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    XMSession::instance()->setEnableMenuMusic(v_button->getChecked());
    Sound::setActiv(XMSession::instance()->enableAudio());
  }

  // controls
  v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:CONTROLS_TAB:KEY_ACTION_LIST"));
  if(v_list->isItemActivated()) {
    v_list->setItemActivated(false);

    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      char cBuf[1024];                

      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      snprintf(cBuf, 1024, GAMETEXT_PRESSANYKEYTO, pEntry->Text[0].c_str());
      StateManager::instance()->pushState(new StateRequestKey(cBuf, this));      
    }
  }

  // www
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:PROXYCONFIG"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    StateManager::instance()->pushState(new StateEditWebConfig());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:UPDATEHIGHSCORES"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    StateManager::instance()->pushState(new StateCheckWww(true));
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLEWEB"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setWWW(v_button->getChecked());
    updateWWWOptions();
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKNEWLEVELSATSTARTUP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setCheckNewLevelsAtStartup(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKHIGHSCORESATSTARTUP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setCheckNewHighscoresAtStartup(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:INGAMEWORLDRECORD"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setShowHighscoreInGame(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:USECRAPPYINFORMATION"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setUseCrappyPack(v_button->getChecked());
  }

  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:WWW_PASSWORD"));
  if(v_edit->hasChanged()) {
    v_edit->setHasChanged(false);
    XMSession::instance()->setWwwPassword(v_edit->getCaption());
  }

  for(unsigned int i=0; i<XMSession::instance()->nbRoomsEnabled(); i++) {
      std::ostringstream v_strRoom;
      v_strRoom << i;

      v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							  + ":ROOMS_LIST"));
      if(v_list->isChanged()) {
	v_list->setChanged(false);
	if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
	  v_list->getEntries()[v_list->getSelected()];
	  XMSession::instance()->setIdRoom(i, *((std::string*)v_list->getEntries()[v_list->getSelected()]->pvUser));
	  updateProfileStrings();
	}
      }

      v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							      + ":UPDATE_ROOMS_LIST"));
      if(v_button->isClicked()) {
	v_button->setClicked(false);
	StateManager::instance()->pushState(new StateUpdateRoomsList());
      }

      v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							      + ":UPLOADHIGHSCOREALL_BUTTON"));
      if(v_button->isClicked()) {
	v_button->setClicked(false);
	StateManager::instance()->pushState(new StateUploadAllHighscores(i));
      }
  }

  for(unsigned int i=1; i<ROOMS_NB_MAX; i++) { // not for 0
      std::ostringstream v_strRoom;
      v_strRoom << i;

      v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							      + ":ROOM_ENABLED"));
      if(v_button->isClicked()) {
	v_button->setClicked(false);
	
	if(v_button->getChecked()) {
	  if(XMSession::instance()->nbRoomsEnabled() < i+1) {
	    XMSession::instance()->setNbRoomsEnabled(i+1);
	    updateWWWOptions();
	  }
	} else {
	  if(XMSession::instance()->nbRoomsEnabled() >= i+1) {
	    XMSession::instance()->setNbRoomsEnabled(i);
	    updateWWWOptions();
	  }
	}
      }
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:ENABLE_GHOSTS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setEnableGhosts(v_button->getChecked());
    updateGhostsOptions();
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_MYBEST"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGhostStrategy_MYBEST(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_THEBEST"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGhostStrategy_THEBEST(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_BESTOFREFROOM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGhostStrategy_BESTOFREFROOM(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_BESTOFOTHERROOMS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGhostStrategy_BESTOFOTHERROOMS(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:DISPLAY_GHOST_TIMEDIFFERENCE"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setShowGhostTimeDifference(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:DISPLAY_GHOSTS_INFOS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setShowGhostsInfos(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:HIDEGHOSTS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setHideGhosts(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:MOTION_BLUR_GHOST"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    XMSession::instance()->setGhostMotionBlur(v_button->getChecked());
  }

  // language
  v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:LANGUAGE_TAB:LANGUAGE_LIST"));
  if(v_list->isClicked()) {
    v_list->setClicked(false);
    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      XMSession::instance()->setLanguage(pEntry->Text[1]);      
      //std::string v_locale = Locales::changeLocale(XMSession::instance()->language());
      //Logger::Log("Locales changed to '%s'", v_locale.c_str());
      //StateManager::instance()->refreshStaticCaptions();
    }
  }

}

void StateMainMenu::updateAudioOptions() {
  UIButton* v_button;

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_AUDIO"));
  v_button->setChecked(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE11KHZ"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE22KHZ"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE44KHZ"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:8BITS"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:16BITS"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:MONO"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:STEREO"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_ENGINE_SOUND"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_MENU_MUSIC"));
  v_button->enableWindow(XMSession::instance()->enableAudio());
}

void StateMainMenu::updateWWWOptions() {
  UIButton* v_button;
  UIList*   v_list;
  UIStatic* v_someText;
  UIEdit*   v_edit;
  UIWindow *v_window;
  bool v_enabled;

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLEWEB"));
  v_button->setChecked(XMSession::instance()->www());

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKNEWLEVELSATSTARTUP"));
  v_button->enableWindow(XMSession::instance()->www());

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKHIGHSCORESATSTARTUP"));
  v_button->enableWindow(XMSession::instance()->www());

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:PROXYCONFIG"));
  v_button->enableWindow(XMSession::instance()->www());

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:UPDATEHIGHSCORES"));
  v_button->enableWindow(XMSession::instance()->www());

  v_someText = reinterpret_cast<UIStatic *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:WWW_PASSWORD_STATIC"));
  v_someText->enableWindow(XMSession::instance()->www());
  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:WWW_PASSWORD"));
  v_edit->enableWindow(XMSession::instance()->www());

  for(unsigned int i=0; i<ROOMS_NB_MAX; i++) {
      std::ostringstream v_strRoom;
      v_strRoom << i;
      v_enabled = XMSession::instance()->nbRoomsEnabled() > i;

      if(i != 0) {
	v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
								+ ":ROOM_ENABLED"));
	v_window = reinterpret_cast<UIWindow *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()));

	if(XMSession::instance()->nbRoomsEnabled() >= i) {
	  v_window->enableWindow(true);
	  v_button->enableWindow(true);
	  v_button->setChecked(XMSession::instance()->nbRoomsEnabled() > i);
	} else {
	  v_window->enableWindow(false);
	  v_button->enableWindow(false);
	  v_button->setChecked(false);
	}
      }

      v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							      + ":UPDATE_ROOMS_LIST"));
      v_button->enableWindow(XMSession::instance()->www() && v_enabled);

      v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							      + ":UPLOADHIGHSCOREALL_BUTTON"));
      v_button->enableWindow(XMSession::instance()->www() && v_enabled);

      v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB_" + v_strRoom.str()
							  + ":ROOMS_LIST"));
      v_list->enableWindow(v_enabled);

  }
}

void StateMainMenu::updateGhostsOptions() {
  UIButton* v_button;

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_MYBEST"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_THEBEST"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_BESTOFREFROOM"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_BESTOFOTHERROOMS"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:DISPLAY_GHOST_TIMEDIFFERENCE"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:DISPLAY_GHOSTS_INFOS"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:HIDEGHOSTS"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:MOTION_BLUR_GHOST"));
  v_button->enableWindow(XMSession::instance()->enableGhosts());
  
  if(GameApp::instance()->getDrawLib()->useShaders() == false) {
    v_button->enableWindow(false);
    v_button->setChecked(false);
  }
}

void StateMainMenu::updateNewLevels() {
  UIButtonDrawn* v_buttonDrawn;

  v_buttonDrawn = reinterpret_cast<UIButtonDrawn *>(m_GUI->getChild("MAIN:NEWLEVELAVAILBLE"));
  v_buttonDrawn->showWindow(false);
}

void StateMainMenu::updateLevelsPackInPackList(const std::string& v_levelPack) {
  UIPackTree *pTree = (UIPackTree *)m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:PACK_TAB:PACK_TREE");
  LevelsPack *v_pack = &(LevelsManager::instance()->LevelsPackByName(v_levelPack));
  
  pTree->updatePack(v_pack,
		    v_pack->getNumberOfFinishedLevels(xmDatabase::instance("main"), XMSession::instance()->profile()),
		    v_pack->getNumberOfLevels(xmDatabase::instance("main")));
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

std::string StateMainMenu::getInfoFrameLevelId()
{
  UILevelList* v_newLevelsList      = (UILevelList *)m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:NEWLEVELS_TAB:NEWLEVELS_LIST");
  UILevelList* v_favoriteLevelsList = (UILevelList *)m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:FAVORITE_TAB:FAVORITE_LIST");
  UILevelList* v_list               = NULL;

  if(v_newLevelsList->isVisible() == true) {
    v_list = v_newLevelsList;
  } else if(v_favoriteLevelsList->isVisible() == true) {
    v_list = v_favoriteLevelsList;
  }

  if(v_list == NULL) {
    return "";
  }
  else{
    return v_list->getSelectedLevel();
  }
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
