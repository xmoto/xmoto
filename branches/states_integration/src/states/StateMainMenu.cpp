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
#include "StatePreplaying.h"
#include "StateLevelInfoViewer.h"
#include "StateMessageBox.h"
#include "StateHelp.h"
#include "StateEditProfile.h"
#include "StateReplaying.h"
#include "StateRequestKey.h"
#include "StateLevelPackViewer.h"
#include "LevelsManager.h"
#include "helpers/Log.h"
#include "helpers/System.h"
#include "Sound.h"

/* static members */
UIRoot*  StateMainMenu::m_sGUI = NULL;

StateMainMenu::StateMainMenu(GameApp* pGame,
			     bool drawStateBehind,
			     bool updateStatesBehind
			     ):
  StateMenu(drawStateBehind,
	    updateStatesBehind,
	    pGame)
{
  Sprite* pSprite;

  m_name    = "StateMainMenu";
  m_quickStartList = NULL;

  /* Load title screen textures */
  m_pTitleBL = NULL;
  pSprite = pGame->getTheme()->getSprite(SPRITE_TYPE_UI, "TitleBL");
  if(pSprite != NULL) {
    m_pTitleBL = pSprite->getTexture(false, true, FM_LINEAR);
  }

  m_pTitleBR = NULL;
  pSprite = pGame->getTheme()->getSprite(SPRITE_TYPE_UI, "TitleBR");
  if(pSprite != NULL) {
    m_pTitleBR = pSprite->getTexture(false, true, FM_LINEAR);
  }

  m_pTitleTL = NULL;
  pSprite = pGame->getTheme()->getSprite(SPRITE_TYPE_UI, "TitleTL");
  if(pSprite != NULL) {
    m_pTitleTL = pSprite->getTexture(false, true, FM_LINEAR);
  }

  m_pTitleTR = NULL;
  pSprite = pGame->getTheme()->getSprite(SPRITE_TYPE_UI, "TitleTR");
  if(pSprite != NULL) {
    m_pTitleTR = pSprite->getTexture(false, true, FM_LINEAR);
  }

  m_require_updateFavoriteLevelsList = false;
}

StateMainMenu::~StateMainMenu()
{
  if(m_quickStartList != NULL) {
    delete m_quickStartList;
  }
}


void StateMainMenu::enter()
{
  m_pGame->playMusic("menu1");
  
  createGUIIfNeeded(m_pGame);
  m_GUI = m_sGUI;

  // updates
  updateProfile();
  updateLevelsPacksList();
  updateLevelsLists();
  updateReplaysList();
  updateStats();
  
  // update options
  updateThemesList();
  updateResolutionsList();
  updateControlsList();
  updateRoomsList();
  updateOptions();
  updateAudioOptions();
  updateWWWOptions();
  updateGhostsOptions();

  StateMenu::enter();
}

void StateMainMenu::leave()
{
  StateMenu::leave();
}

void StateMainMenu::enterAfterPop()
{
  StateMenu::enterAfterPop();

  if(m_require_updateFavoriteLevelsList) {
    updateFavoriteLevelsList();
    m_require_updateFavoriteLevelsList = false;
  }
}

void StateMainMenu::leaveAfterPush()
{
  StateMenu::leaveAfterPush();
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
}

void StateMainMenu::checkEventsMainWindow() {
  UIButton*           v_button;
  UILevelList*        v_list;
  std::string         v_id_level;
  UIQuickStartButton* v_quickstart;

  // quickstart
  v_quickstart = reinterpret_cast<UIQuickStartButton *>(m_GUI->getChild("MAIN:QUICKSTART"));

  if(v_quickstart->hasChanged()) {
    v_quickstart->setHasChanged(false);

    m_pGame->getSession()->setQuickStartQualityMIN(v_quickstart->getQualityMIN());
    m_pGame->getSession()->setQuickStartQualityMAX(v_quickstart->getQualityMAX());
    m_pGame->getSession()->setQuickStartDifficultyMIN(v_quickstart->getDifficultyMIN());
    m_pGame->getSession()->setQuickStartDifficultyMAX(v_quickstart->getDifficultyMAX());
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
      m_pGame->setCurrentPlayingList(m_quickStartList);
      v_id_level = m_quickStartList->getLevel(0);
    } catch(Exception &e) {
      v_id_level = "tut1";
    }
    StatePreplaying::setPlayAnimation(true);
    m_pGame->getStateManager()->pushState(new StatePreplaying(m_pGame, v_id_level));    
  }

  // quit
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:QUIT"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    StateMessageBox* v_msgboxState = new StateMessageBox(this, m_pGame, GAMETEXT_QUITMESSAGE, UI_MSGBOX_YES|UI_MSGBOX_NO);
    v_msgboxState->setId("QUIT");
    m_pGame->getStateManager()->pushState(v_msgboxState);
  }

  // help
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:HELP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getStateManager()->pushState(new StateHelp(m_pGame));
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

  // edit profile
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:CHANGEPLAYERBUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getStateManager()->pushState(new StateEditProfile(m_pGame, this));
  }
}

void StateMainMenu::checkEventsLevelsMultiTab() {
  UIButton* v_button;

  // MultiStopWhenOneFinishes
  v_button = reinterpret_cast<UIButton*>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:MULTI_TAB:ENABLEMULTISTOPWHENONEFINISHES"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    m_pGame->getSession()->setMultiStopWhenOneFinishes(v_button->getChecked());
  }

  // multi players
  for(unsigned int i=0; i<4; i++) {
    std::ostringstream s_nbPlayers;
    s_nbPlayers << (int) i+1;
    v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:MULTI_TAB:MULTINB_" + s_nbPlayers.str()));  
    if(v_button->isClicked()) {
      v_button->setClicked(false);
      if(v_button->getChecked()) {
	m_pGame->getSession()->setMultiNbPlayers(i+1);
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
      m_pGame->getStateManager()->pushState(new StateLevelPackViewer(m_pGame, nSelectedPack));
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
      m_pGame->setCurrentPlayingList(v_list);
      StatePreplaying::setPlayAnimation(true);
      m_pGame->getStateManager()->pushState(new StatePreplaying(m_pGame, v_id_level));
    }
  }

  // level info button
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:FAVORITE_TAB:LEVEL_INFO_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    v_list = reinterpret_cast<UILevelList *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:FAVORITE_TAB:FAVORITE_LIST"));
    v_id_level = v_list->getSelectedLevel();

    if(v_id_level != "") {
      m_pGame->getStateManager()->pushState(new StateLevelInfoViewer(m_pGame, v_id_level));
    }
  }

  // delete from favorite button
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:FAVORITE_TAB:DELETE_FROM_FAVORITE_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    v_list = reinterpret_cast<UILevelList *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:FAVORITE_TAB:FAVORITE_LIST"));
    v_id_level = v_list->getSelectedLevel();

    if(v_id_level != "") {
      m_pGame->getLevelsManager()->delFromFavorite(m_pGame->getDb(), m_pGame->getSession()->profile(), v_id_level);
      updateFavoriteLevelsList();
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
      m_pGame->setCurrentPlayingList(v_list);
      StatePreplaying::setPlayAnimation(true);
      m_pGame->getStateManager()->pushState(new StatePreplaying(m_pGame, v_id_level));
    }
  }

  // level info button
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:NEWLEVELS_TAB:LEVEL_INFO_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    v_list = reinterpret_cast<UILevelList *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:NEWLEVELS_TAB:NEWLEVELS_LIST"));
    v_id_level = v_list->getSelectedLevel();

    if(v_id_level != "") {
      m_pGame->getStateManager()->pushState(new StateLevelInfoViewer(m_pGame, v_id_level));
    }
  }

  // download button
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:NEWLEVELS_TAB:DOWNLOAD_LEVELS_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    m_pGame->checkForExtraLevels();
  }

}

bool StateMainMenu::update()
{
  return StateMenu::update();
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
    m_pGame->getStateManager()->pushState(new StateHelp(m_pGame));
    break;

  default:
    StateMenu::keyDown(nKey, mod, nChar);
    checkEvents();
    break;

  }
}

void StateMainMenu::keyUp(int nKey,   SDLMod mod)
{
  StateMenu::keyUp(nKey, mod);
}

void StateMainMenu::mouseDown(int nButton)
{
  StateMenu::mouseDown(nButton);
}

void StateMainMenu::mouseDoubleClick(int nButton)
{
  StateMenu::mouseDoubleClick(nButton);
}

void StateMainMenu::mouseUp(int nButton)
{
  StateMenu::mouseUp(nButton);
}

void StateMainMenu::clean() {
  if(StateMainMenu::m_sGUI != NULL) {
    delete StateMainMenu::m_sGUI;
    StateMainMenu::m_sGUI = NULL;
  }
}

void StateMainMenu::createGUIIfNeeded(GameApp* pGame) {
  if(m_sGUI != NULL) return;
  DrawLib* drawlib = pGame->getDrawLib();


  m_sGUI = new UIRoot();
  m_sGUI->setApp(pGame);
  m_sGUI->setFont(drawlib->getFontSmall()); 
  m_sGUI->setPosition(0, 0,
		      drawlib->getDispWidth(),
		      drawlib->getDispHeight());

  UIWindow* v_menu;
  UIButton* v_button;
  UIStatic* v_someText;

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
					pGame->getSession()->quickStartQualityMIN(),
					pGame->getSession()->quickStartDifficultyMIN(),
					pGame->getSession()->quickStartQualityMAX(),
					pGame->getSession()->quickStartDifficultyMAX()
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

  // frames
  makeWindowLevels(pGame,  v_menu);
  makeWindowReplays(pGame, v_menu);
  makeWindowOptions(pGame, v_menu);
  makeWindowStats(pGame, v_menu);
}

void StateMainMenu::updateProfile() {
  UIStatic* v_playerTag = reinterpret_cast<UIStatic *>(m_GUI->getChild("MAIN:PLAYERTAG"));
  std::string v_caption;

  if(m_pGame->getSession()->profile() != "") {
    v_caption = std::string(GAMETEXT_PLAYER) + ": " + m_pGame->getSession()->profile() + "@" + m_pGame->getHighscoresRoomName();
  }

  v_playerTag->setCaption(v_caption);
}

UIWindow* StateMainMenu::makeWindowStats(GameApp* pGame, UIWindow* i_parent) {
  UIStatic* v_someText;
  UIFrame* v_window;
  UIButton* v_button;
  DrawLib* drawlib = pGame->getDrawLib();

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
  float v_totalPlayedTime = 0.0;
  int   v_nbPlayed        = 0;
  int   v_nbDied          = 0;
  int   v_nbCompleted     = 0;
  int   v_nbRestarted     = 0;
  int   v_nbDiffLevels    = 0;
  std::string v_level_name= "";
  
  if(v_window != NULL){
    delete v_window;
  }

  v_window = new UIWindow(i_parent, x, y, "", nWidth, nHeight);
  v_window->setID("REPORT");
 
  v_result = m_pGame->getDb()->readDB("SELECT a.nbStarts, a.since, SUM(b.playedTime), "
				      "SUM(b.nbPlayed), SUM(b.nbDied), SUM(b.nbCompleted), "
				      "SUM(b.nbRestarted), count(b.id_level) "
				      "FROM stats_profiles AS a INNER JOIN stats_profiles_levels AS b "
				      "ON a.id_profile=b.id_profile "
				      "WHERE a.id_profile=\"" + xmDatabase::protectString(m_pGame->getSession()->profile()) + "\" "
				      "GROUP BY a.id_profile;",
				      nrow);
  
  if(nrow == 0) {
    m_pGame->getDb()->read_DB_free(v_result);
    return;
  }
  
  v_nbStarts        = atoi(m_pGame->getDb()->getResult(v_result, 8, 0, 0));
  v_since           =      m_pGame->getDb()->getResult(v_result, 8, 0, 1);
  v_totalPlayedTime = atof(m_pGame->getDb()->getResult(v_result, 8, 0, 2));
  v_nbPlayed        = atoi(m_pGame->getDb()->getResult(v_result, 8, 0, 3));
  v_nbDied          = atoi(m_pGame->getDb()->getResult(v_result, 8, 0, 4));
  v_nbCompleted     = atoi(m_pGame->getDb()->getResult(v_result, 8, 0, 5));
  v_nbRestarted     = atoi(m_pGame->getDb()->getResult(v_result, 8, 0, 6));
  v_nbDiffLevels    = atoi(m_pGame->getDb()->getResult(v_result, 8, 0, 7));
  
  m_pGame->getDb()->read_DB_free(v_result);
  
  /* Per-player info */
  char cBuf[512];
  char cTime[512];
  int nHours = ((int)v_totalPlayedTime) / (60 * 60);
  int nMinutes = (((int)v_totalPlayedTime) / (60)) - nHours*60;
  int nSeconds = (((int)v_totalPlayedTime)) - nMinutes*60 - nHours*3600;
  if(nHours > 0)
    sprintf(cTime,(std::string(GAMETEXT_XHOURS) + ", " + std::string(GAMETEXT_XMINUTES) + ", " + std::string(GAMETEXT_XSECONDS)).c_str(),nHours,nMinutes,nSeconds);
  else if(nMinutes > 0)
    sprintf(cTime,(std::string(GAMETEXT_XMINUTES) + ", " + std::string(GAMETEXT_XSECONDS)).c_str(),nMinutes,nSeconds);
  else
    sprintf(cTime,GAMETEXT_XSECONDS,nSeconds);
  
  sprintf(cBuf,GAMETEXT_XMOTOGLOBALSTATS,      
	  v_since.c_str(), v_nbStarts, v_nbPlayed, v_nbDiffLevels,
	  v_nbDied, v_nbCompleted, v_nbRestarted, cTime);                           
  
  UIStatic *pText = new UIStatic(v_window, 0, 0, cBuf, nWidth, 80);
  pText->setHAlign(UI_ALIGN_LEFT);
  pText->setTextSolidColor(MAKE_COLOR(255,255,0,255));
  pText->setFont(m_pGame->getDrawLib()->getFontSmall());
  
  /* Per-level stats */      
  pText = new UIStatic(v_window,0,90, std::string(GAMETEXT_MOSTPLAYEDLEVELSFOLLOW) + ":",nWidth,20);
  pText->setHAlign(UI_ALIGN_LEFT);
  pText->setTextSolidColor(MAKE_COLOR(255,255,0,255));
  pText->setFont(m_pGame->getDrawLib()->getFontSmall());      
  
  v_result = m_pGame->getDb()->readDB("SELECT a.name, b.nbPlayed, b.nbDied, "
				      "b.nbCompleted, b.nbRestarted, b.playedTime "
				      "FROM levels AS a INNER JOIN stats_profiles_levels AS b ON a.id_level=b.id_level "
				      "WHERE id_profile=\"" + xmDatabase::protectString(m_pGame->getSession()->profile()) + "\" "
				      "ORDER BY nbPlayed DESC LIMIT 10;",
				      nrow);
  
  int cy = 110;
  for(int i=0; i<nrow; i++) {
    if(cy + 45 > nHeight) break; /* out of window */
    
    v_level_name      =      m_pGame->getDb()->getResult(v_result, 6, i, 0);
    v_totalPlayedTime = atof(m_pGame->getDb()->getResult(v_result, 6, i, 5));
    v_nbDied          = atoi(m_pGame->getDb()->getResult(v_result, 6, i, 2));
    v_nbPlayed        = atoi(m_pGame->getDb()->getResult(v_result, 6, i, 1));
    v_nbCompleted     = atoi(m_pGame->getDb()->getResult(v_result, 6, i, 3));
    v_nbRestarted     = atoi(m_pGame->getDb()->getResult(v_result, 6, i, 4));
    
    sprintf(cBuf,("[%s] %s:\n   " + std::string(GAMETEXT_XMOTOLEVELSTATS)).c_str(),
	    GameApp::formatTime(v_totalPlayedTime).c_str(), v_level_name.c_str(),
	    v_nbPlayed, v_nbDied, v_nbCompleted, v_nbRestarted);
    
    pText = new UIStatic(v_window, 0, cy, cBuf, nWidth, 45);
    pText->setHAlign(UI_ALIGN_LEFT);        
    pText->setTextSolidColor(MAKE_COLOR(255,255,0,255));
    pText->setFont(m_pGame->getDrawLib()->getFontSmall());
    
    cy += 45;
  }  
  m_pGame->getDb()->read_DB_free(v_result);
}

UIWindow* StateMainMenu::makeWindowReplays(GameApp* pGame, UIWindow* i_parent) {
  UIWindow* v_window;
  UIStatic* v_someText;
  UIEdit*   v_edit;
  UIButton *v_button, *v_showButton;
  UIList*   v_list;

  v_window = new UIFrame(i_parent, 220, i_parent->getPosition().nHeight*7/30, "",
			 i_parent->getPosition().nWidth -220 -20,
			 i_parent->getPosition().nHeight -40 -i_parent->getPosition().nHeight/5 -10);
  v_window->setID("FRAME_REPLAYS");
  v_window->showWindow(false);
   
  v_someText = new UIStatic(v_window, 0, 0, GAMETEXT_REPLAYS, v_window->getPosition().nWidth, 36);
  v_someText->setFont(pGame->getDrawLib()->getFontMedium());

  v_someText = new UIStatic(v_window, 10, 35, std::string(GAMETEXT_FILTER) + ":", 90, 25);
  v_someText->setFont(pGame->getDrawLib()->getFontSmall());
  v_someText->setHAlign(UI_ALIGN_RIGHT);
  v_edit = new UIEdit(v_window, 110, 35, "", 200, 25);
  v_edit->setFont(pGame->getDrawLib()->getFontSmall());
  v_edit->setID("REPLAYS_FILTER");
  v_edit->setContextHelp(CONTEXTHELP_REPLAYS_FILTER);

  /* show button */
  v_button = new UIButton(v_window, 5, v_window->getPosition().nHeight-68, GAMETEXT_SHOW, 105, 57);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_SMALL);
  v_button->setID("REPLAYS_SHOW_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_RUN_REPLAY);
  v_showButton = v_button;

  /* delete button */
  v_button = new UIButton(v_window, 105, v_window->getPosition().nHeight-68, GAMETEXT_DELETE, 105, 57);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_SMALL);
  v_button->setID("REPLAYS_DELETE_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_DELETE_REPLAY);

  /* upload button */
  v_button = new UIButton(v_window, 199, v_window->getPosition().nHeight-68, GAMETEXT_UPLOAD_HIGHSCORE, 186, 57);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_SMALL);
  v_button->setID("REPLAYS_UPLOADHIGHSCORE_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_UPLOAD_HIGHSCORE);

  /* filter */
  v_button = new UIButton(v_window, v_window->getPosition().nWidth-105, v_window->getPosition().nHeight-68,
			  GAMETEXT_LISTALL, 115, 57);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setChecked(false);
  v_button->setID("REPLAYS_LIST_ALL");
  v_button->setContextHelp(CONTEXTHELP_ALL_REPLAYS);

  /* list */
  v_list = new UIList(v_window, 20, 65, "", v_window->getPosition().nWidth-40, v_window->getPosition().nHeight-115-25);
  v_list->setID("REPLAYS_LIST");
  v_list->setFont(pGame->getDrawLib()->getFontSmall());
  v_list->addColumn(GAMETEXT_REPLAY, v_list->getPosition().nWidth/2 - 100, CONTEXTHELP_REPLAYCOL);
  v_list->addColumn(GAMETEXT_LEVEL,  v_list->getPosition().nWidth/2 - 28,  CONTEXTHELP_REPLAYLEVELCOL);
  v_list->addColumn(GAMETEXT_PLAYER,128,CONTEXTHELP_REPLAYPLAYERCOL);
  v_list->setEnterButton(v_showButton);

  return v_window;
}

UIWindow* StateMainMenu::makeWindowOptions_general(GameApp* pGame, UIWindow* i_parent) {
  UIWindow*  v_window;
  UIButton*  v_button;
  UIList*    v_list;
  DrawLib* drawlib = pGame->getDrawLib();

  UIWindow* v_generalTab = new UIWindow(i_parent, 20, 40, GAMETEXT_GENERAL,
					i_parent->getPosition().nWidth-40, i_parent->getPosition().nHeight);
  v_generalTab->setID("GENERAL_TAB");
  
  v_button = new UIButton(v_generalTab, 5, 33-28-10, GAMETEXT_SHOWMINIMAP, (v_generalTab->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("SHOWMINIMAP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_MINI_MAP);
  
  v_button = new UIButton(v_generalTab, 5, 63-28-10, GAMETEXT_SHOWENGINECOUNTER, (v_generalTab->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("SHOWENGINECOUNTER");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_ENGINE_COUNTER);

  v_button = new UIButton(v_generalTab, 5+(v_generalTab->getPosition().nWidth-40)/2, 33-28-10, GAMETEXT_INITZOOM,
			  (v_generalTab->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("INITZOOM");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_INITZOOM);

  v_button = new UIButton(v_generalTab, 5+(v_generalTab->getPosition().nWidth-40)/2, 63-28-10, GAMETEXT_DEATHANIM,
			  (v_generalTab->getPosition().nWidth-40)/2, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("DEATHANIM");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_DEATHANIM);

  v_button = new UIButton(v_generalTab, 5, 93-28-10, GAMETEXT_ENABLECONTEXTHELP, v_generalTab->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLECONTEXTHELP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_SHOWCONTEXTHELP);
 
  v_button = new UIButton(v_generalTab, 5, 123-28-10, GAMETEXT_AUTOSAVEREPLAYS, v_generalTab->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("AUTOSAVEREPLAYS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50023);
  v_button->setContextHelp(CONTEXTHELP_AUTOSAVEREPLAYS);
   
  v_list = new UIList(v_generalTab, 5, 120, "", 
		      v_generalTab->getPosition().nWidth-10, v_generalTab->getPosition().nHeight-125-90);
  v_list->setID("THEMES_LIST");
  v_list->setFont(drawlib->getFontSmall());
  v_list->addColumn(GAMETEXT_THEMES, (v_list->getPosition().nWidth*3) / 5);
  v_list->addColumn("", (v_list->getPosition().nWidth*2) / 5);
  v_list->setContextHelp(CONTEXTHELP_THEMES);

  v_button = new UIButton(v_generalTab, v_generalTab->getPosition().nWidth -200 -200, v_generalTab->getPosition().nHeight - 95,
			  GAMETEXT_UPDATETHEMESLIST, 207, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("UPDATE_THEMES_LIST");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_UPDATETHEMESLIST);

  v_button = new UIButton(v_generalTab, v_generalTab->getPosition().nWidth -200, v_generalTab->getPosition().nHeight - 95,
			  GAMETEXT_GETSELECTEDTHEME, 207, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("GET_SELECTED_THEME");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GETSELECTEDTHEME);

  return v_window;
}

UIWindow* StateMainMenu::makeWindowOptions_video(GameApp* pGame, UIWindow* i_parent) {
  UIWindow*  v_window;
  UIButton*  v_button;
  UIList*    v_list;
  UIStatic*  v_someText;
  DrawLib* drawlib = pGame->getDrawLib();

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
}

UIWindow* StateMainMenu::makeWindowOptions_audio(GameApp* pGame, UIWindow* i_parent) {
  UIWindow*  v_window;
  UIButton*  v_button;
  UIList*    v_list;
  DrawLib* drawlib = pGame->getDrawLib();

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
}

UIWindow* StateMainMenu::makeWindowOptions_controls(GameApp* pGame, UIWindow* i_parent) {
  UIWindow*  v_window;
  UIButton*  v_button;
  UIList*    v_list;
  DrawLib* drawlib = pGame->getDrawLib();

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
}

UIWindow* StateMainMenu::makeWindowOptions_rooms(GameApp* pGame, UIWindow* i_parent) {
  UIWindow*  v_window;
  UIButton*  v_button;
  UIList*    v_list;
  UIStatic*  v_someText;
  UIEdit*    v_edit;
  DrawLib* drawlib = pGame->getDrawLib();

  v_window = new UIWindow(i_parent, 0, 26, GAMETEXT_WWWTAB, i_parent->getPosition().nWidth, i_parent->getPosition().nHeight);
  v_window->setID("WWW_TAB");
  v_window->showWindow(false);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth-225, v_window->getPosition().nHeight-80,
			  GAMETEXT_PROXYCONFIG, 207, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("PROXYCONFIG");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_PROXYCONFIG);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth-225-200, v_window->getPosition().nHeight-80,
			  GAMETEXT_UPDATEHIGHSCORES, 207, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("UPDATEHIGHSCORES");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_UPDATEHIGHSCORES);

  UITabView *v_roomsTabs = new UITabView(v_window, 0, 0, "", v_window->getPosition().nWidth, v_window->getPosition().nHeight-76);
  v_roomsTabs->setID("TABS");
  v_roomsTabs->setFont(drawlib->getFontSmall());
  v_roomsTabs->setTabContextHelp(0, CONTEXTHELP_WWW_MAIN_TAB);
  v_roomsTabs->setTabContextHelp(1, CONTEXTHELP_WWW_ROOMS_TAB);

  v_window = new UIWindow(v_roomsTabs, 20, 40, GAMETEXT_WWWMAINTAB, v_roomsTabs->getPosition().nWidth-40,
			  v_roomsTabs->getPosition().nHeight);
  v_window->setID("MAIN_TAB");

  v_button = new UIButton(v_window, 5, 5, GAMETEXT_ENABLEWEBHIGHSCORES, (v_window->getPosition().nWidth-40), 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLEWEB");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50123);
  v_button->setContextHelp(CONTEXTHELP_DOWNLOAD_BEST_TIMES);

  v_button = new UIButton(v_window, 5, 43, GAMETEXT_ENABLECHECKNEWLEVELSATSTARTUP,v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLECHECKNEWLEVELSATSTARTUP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50123);
  v_button->setContextHelp(CONTEXTHELP_ENABLE_CHECK_NEW_LEVELS_AT_STARTUP);
  
  v_button = new UIButton(v_window, 5, 81, GAMETEXT_ENABLECHECKHIGHSCORESATSTARTUP, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLECHECKHIGHSCORESATSTARTUP");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50123);
  v_button->setContextHelp(CONTEXTHELP_ENABLE_CHECK_HIGHSCORES_AT_STARTUP);

  v_button = new UIButton(v_window, 5, 119, GAMETEXT_ENABLEINGAMEWORLDRECORD, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("INGAMEWORLDRECORD");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setGroup(50123);
  v_button->setContextHelp(CONTEXTHELP_INGAME_WORLD_RECORD);

  v_window = new UIWindow(v_roomsTabs, 20, 40, GAMETEXT_WWWROOMSTAB, v_roomsTabs->getPosition().nWidth-40,
			  v_roomsTabs->getPosition().nHeight);
  v_window->setID("ROOMS_TAB");
  v_window->showWindow(false);

  v_list = new UIList(v_window, 5, 10, "", v_window->getPosition().nWidth-200, v_window->getPosition().nHeight-30 - 85);
  v_list->setID("ROOMS_LIST");
  v_list->setFont(drawlib->getFontSmall());
  v_list->addColumn(GAMETEXT_ROOM, v_list->getPosition().nWidth);
  v_list->setContextHelp(CONTEXTHELP_WWW_ROOMS_LIST);

  v_someText = new UIStatic(v_window, v_window->getPosition().nWidth-180, 5, std::string(GAMETEXT_LOGIN) + ":", 130, 30);
  v_someText->setHAlign(UI_ALIGN_LEFT);
  v_someText->setFont(drawlib->getFontSmall()); 

  v_edit = new UIEdit(v_window, v_window->getPosition().nWidth-180, 30, "", 150, 25);
  v_edit->setFont(drawlib->getFontSmall());
  v_edit->setID("ROOM_LOGIN");
  v_edit->setContextHelp(CONTEXTHELP_ROOM_LOGIN);

  v_someText = new UIStatic(v_window, v_window->getPosition().nWidth-180, 65, std::string(GAMETEXT_PASSWORD) + ":", 130, 30);
  v_someText->setHAlign(UI_ALIGN_LEFT);
  v_someText->setFont(drawlib->getFontSmall()); 

  v_edit = new UIEdit(v_window, v_window->getPosition().nWidth-180, 90, "", 150, 25);
  v_edit->hideText(true);
  v_edit->setFont(drawlib->getFontSmall());
  v_edit->setID("ROOM_PASSWORD");
  v_edit->setContextHelp(CONTEXTHELP_ROOM_PASSWORD);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth/2 - 212, v_window->getPosition().nHeight - 100, GAMETEXT_UPDATEROOMSSLIST, 215, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("UPDATE_ROOMS_LIST");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_UPDATEROOMSLIST);

  v_button = new UIButton(v_window, v_window->getPosition().nWidth/2 + 5, v_window->getPosition().nHeight - 100, GAMETEXT_UPLOAD_ALL_HIGHSCORES, 215, 57);
  v_button->setFont(drawlib->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setID("REPLAY_UPLOADHIGHSCOREALL_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_UPLOAD_HIGHSCORE_ALL);	
}

UIWindow* StateMainMenu::makeWindowOptions_ghosts(GameApp* pGame, UIWindow* i_parent) {
  UIWindow*  v_window;
  UIButton*  v_button;
  UIList*    v_list;
  DrawLib* drawlib = pGame->getDrawLib();

  v_window = new UIWindow(i_parent, 20, 40, GAMETEXT_GHOSTTAB, i_parent->getPosition().nWidth-40, i_parent->getPosition().nHeight);
  v_window->setID("GHOSTS_TAB");
  v_window->showWindow(false);

  v_button = new UIButton(v_window, 5, 5, GAMETEXT_ENABLEGHOST, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("ENABLE_GHOSTS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GHOST_MODE);

  v_button = new UIButton(v_window, 5+20, 35, GAMETEXT_GHOST_STRATEGY_MYBEST, v_window->getPosition().nWidth-40,28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("GHOST_STRATEGY_MYBEST");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GHOST_STRATEGY_MYBEST);

  v_button = new UIButton(v_window, 5+20, 65, GAMETEXT_GHOST_STRATEGY_THEBEST, v_window->getPosition().nWidth-40,28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("GHOST_STRATEGY_THEBEST");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GHOST_STRATEGY_THEBEST);

  v_button = new UIButton(v_window, 5+20, 95, GAMETEXT_GHOST_STRATEGY_BESTOFROOM, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("GHOST_STRATEGY_BESTOFROOM");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_GHOST_STRATEGY_BESTOFROOM);

  v_button = new UIButton(v_window, 5, 125, GAMETEXT_DISPLAYGHOSTTIMEDIFF, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("DISPLAY_GHOST_TIMEDIFFERENCE");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_DISPLAY_GHOST_TIMEDIFF);

  v_button = new UIButton(v_window, 5, 185, GAMETEXT_DISPLAYGHOSTINFO, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("DISPLAY_GHOSTS_INFOS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_DISPLAY_GHOST_INFO);

  v_button = new UIButton(v_window, 5, 155, GAMETEXT_HIDEGHOSTS, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("HIDEGHOSTS");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_HIDEGHOSTS);

  v_button = new UIButton(v_window, 5, 215, GAMETEXT_MOTIONBLURGHOST, v_window->getPosition().nWidth-40, 28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setID("MOTION_BLUR_GHOST");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setContextHelp(CONTEXTHELP_MOTIONBLURGHOST);
}

UIWindow* StateMainMenu::makeWindowOptions(GameApp* pGame, UIWindow* i_parent) {
  UIWindow *v_window, *v_frame;
  UIStatic*  v_someText;
  UITabView* v_tabview;
  UIButton*  v_button;
  DrawLib* drawlib = pGame->getDrawLib();

  v_window = new UIFrame(i_parent, 220, i_parent->getPosition().nHeight*7/30, "",
			 i_parent->getPosition().nWidth -220 -20,
			 i_parent->getPosition().nHeight -40 -i_parent->getPosition().nHeight/5 -10);
  v_window->setID("FRAME_OPTIONS");
  v_window->showWindow(false);
   
  v_someText = new UIStatic(v_window, 0, 0, GAMETEXT_OPTIONS, v_window->getPosition().nWidth, 36);
  v_someText->setFont(pGame->getDrawLib()->getFontMedium());

  v_tabview  = new UITabView(v_window, 20, 40, "", v_window->getPosition().nWidth-40, v_window->getPosition().nHeight-115);
  v_tabview->setID("TABS");
  v_tabview->setFont(drawlib->getFontSmall());
  v_tabview->setTabContextHelp(0, CONTEXTHELP_GENERAL_OPTIONS);
  v_tabview->setTabContextHelp(1, CONTEXTHELP_VIDEO_OPTIONS);
  v_tabview->setTabContextHelp(2, CONTEXTHELP_AUDIO_OPTIONS);
  v_tabview->setTabContextHelp(3, CONTEXTHELP_CONTROL_OPTIONS);

  v_frame = makeWindowOptions_general(pGame, v_tabview);
  v_frame = makeWindowOptions_video(pGame, v_tabview);
  v_frame = makeWindowOptions_audio(pGame, v_tabview);
  v_frame = makeWindowOptions_controls(pGame, v_tabview);
  v_frame = makeWindowOptions_rooms(pGame, v_tabview);
  v_frame = makeWindowOptions_ghosts(pGame, v_tabview);

  v_button = new UIButton(v_window, 20, v_window->getPosition().nHeight-68, GAMETEXT_DEFAULTS, 115, 57);
  v_button->setID("DEFAULTS_BUTTON");
  v_button->setFont(drawlib->getFontSmall());
  v_button->setType(UI_BUTTON_TYPE_SMALL);      
  v_button->setContextHelp(CONTEXTHELP_DEFAULTS);

  return v_window;
}

UIWindow* StateMainMenu::makeWindowLevels(GameApp* pGame, UIWindow* i_parent) {
  UIWindow*    v_window;
  UIStatic*    v_someText;
  UIButton*    v_button;
  UIPackTree*  v_packTree;
  UILevelList* v_list;

  v_window = new UIFrame(i_parent, 220, i_parent->getPosition().nHeight*7/30, "",
			 i_parent->getPosition().nWidth -220 -20,
			 i_parent->getPosition().nHeight -40 -i_parent->getPosition().nHeight/5 -10);
  v_window->setID("FRAME_LEVELS");
  v_window->showWindow(false);
   
  v_someText = new UIStatic(v_window, 0, 0, GAMETEXT_LEVELS, v_window->getPosition().nWidth, 36);
  v_someText->setFont(pGame->getDrawLib()->getFontMedium());

  /* tabs */
  UITabView *v_levelTabs = new UITabView(v_window, 20, 40, "",
					 v_window->getPosition().nWidth-40, v_window->getPosition().nHeight-60);
  v_levelTabs->setFont(pGame->getDrawLib()->getFontSmall());
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
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  v_button->setID("PACK_OPEN_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_VIEW_LEVEL_PACK);
  
  /* pack list */
  v_packTree = new UIPackTree(v_packTab, 10, 0, "", v_packTab->getPosition().nWidth-20, v_packTab->getPosition().nHeight-105);      
  v_packTree->setFont(pGame->getDrawLib()->getFontSmall());
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
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  v_button->setID("PLAY_GO_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_PLAY_SELECTED_LEVEL);

  /* favorite list */
  v_list = new UILevelList(v_favoriteTab, 0, 0, "", v_favoriteTab->getPosition().nWidth, v_favoriteTab->getPosition().nHeight-105);     
  v_list->setID("FAVORITE_LIST");
  v_list->setFont(pGame->getDrawLib()->getFontSmall());
  v_list->setSort(true);
  v_list->setEnterButton(v_button);

  /* favorite other buttons */
  v_button= new UIButton(v_favoriteTab, 105, v_favoriteTab->getPosition().nHeight-103, GAMETEXT_SHOWINFO, 105, 57);
  v_button->setType(UI_BUTTON_TYPE_SMALL);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  v_button->setID("LEVEL_INFO_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_LEVEL_INFO);
  
  v_button= new UIButton(v_favoriteTab, v_favoriteTab->getPosition().nWidth-187, v_favoriteTab->getPosition().nHeight-103, GAMETEXT_DELETEFROMFAVORITE, 187, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
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
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  v_button->setID("PLAY_GO_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_PLAY_SELECTED_LEVEL);

  /* new levels list */
  v_list = new UILevelList(v_newLevelsTab, 0, 0, "", v_newLevelsTab->getPosition().nWidth, v_newLevelsTab->getPosition().nHeight-105);     
  v_list->setFont(pGame->getDrawLib()->getFontSmall());
  v_list->setID("NEWLEVELS_LIST");
  v_list->setSort(true);
  v_list->setEnterButton(v_button);

  /* new levels tab other buttons */
  v_button = new UIButton(v_newLevelsTab, 105, v_newLevelsTab->getPosition().nHeight-103, GAMETEXT_SHOWINFO, 105, 57);
  v_button->setType(UI_BUTTON_TYPE_SMALL);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  v_button->setID("LEVEL_INFO_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_LEVEL_INFO);

  v_button = new UIButton(v_newLevelsTab, v_newLevelsTab->getPosition().nWidth-187, v_newLevelsTab->getPosition().nHeight-103,
			  GAMETEXT_DOWNLOADLEVELS, 187, 57);
  v_button->setType(UI_BUTTON_TYPE_LARGE);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  v_button->setID("DOWNLOAD_LEVELS_BUTTON");
  v_button->setContextHelp(CONTEXTHELP_DOWNLOADLEVELS);

  /* multi tab */
  UIWindow *v_multiOptionsTab = new UIWindow(v_levelTabs, 20, 40, GAMETEXT_MULTI,
					     v_levelTabs->getPosition().nWidth-40, v_levelTabs->getPosition().nHeight);
  v_multiOptionsTab->showWindow(false);
  v_multiOptionsTab->setID("MULTI_TAB");

  v_someText = new UIStatic(v_multiOptionsTab, 10, 0, GAMETEXT_NB_PLAYERS, v_multiOptionsTab->getPosition().nWidth, 40);
  v_someText->setFont(pGame->getDrawLib()->getFontMedium());
  v_someText->setHAlign(UI_ALIGN_LEFT);

  // choice of the number of players
  for(unsigned int i=0; i<4; i++) {
    std::ostringstream s_nbPlayers;
    char strPlayer[64];
    snprintf(strPlayer, 64, GAMETEXT_NPLAYER(i+1), i+1);
    s_nbPlayers << (int) i+1;

    v_button = new UIButton(v_multiOptionsTab, 0, 40+(i*20), strPlayer, v_multiOptionsTab->getPosition().nWidth, 28);
    v_button->setType(UI_BUTTON_TYPE_RADIO);
    v_button->setFont(pGame->getDrawLib()->getFontSmall());
    v_button->setID("MULTINB_" + s_nbPlayers.str());
    v_button->setGroup(10200);
    v_button->setContextHelp(CONTEXTHELP_MULTI);
    
    // always check the 1 player mode
    if(i == 0) {
      v_button->setChecked(true);
    }
  }

  v_button = new UIButton(v_multiOptionsTab, 0, v_multiOptionsTab->getPosition().nHeight - 40 - 28 - 10,
			  GAMETEXT_MULTISTOPWHENONEFINISHES, v_multiOptionsTab->getPosition().nWidth,28);
  v_button->setType(UI_BUTTON_TYPE_CHECK);
  v_button->setFont(pGame->getDrawLib()->getFontSmall());
  v_button->setID("ENABLEMULTISTOPWHENONEFINISHES");
  v_button->setGroup(50050);
  v_button->setContextHelp(CONTEXTHELP_MULTISTOPWHENONEFINISHES);    

  return v_window;
}

//
//    /* level info frame */
//    m_pLevelInfoFrame = new UIWindow(m_pMainMenu,0,drawlib->getDispHeight()/2 - (m_nNumMainMenuButtons*57)/2 + m_nNumMainMenuButtons*57,"",220,100);
//    m_pLevelInfoFrame->showWindow(false);
//    m_pBestPlayerText = new UIStatic(m_pLevelInfoFrame, 0, 5,"", 220, 50);
//    m_pBestPlayerText->setFont(drawlib->getFontSmall());
//    m_pBestPlayerText->setHAlign(UI_ALIGN_CENTER);
//    m_pBestPlayerText->showWindow(true);
//    m_pLevelInfoViewReplayButton = new UIButton(m_pLevelInfoFrame,22,40, GAMETEXT_VIEWTHEHIGHSCORE,176,40);
//    m_pLevelInfoViewReplayButton->setFont(drawlib->getFontSmall());
//    m_pLevelInfoViewReplayButton->setContextHelp(CONTEXTHELP_VIEWTHEHIGHSCORE);
//
//    
//    /* new levels ? */
//    m_pNewLevelsAvailable = new UIButtonDrawn(m_pMainMenu,
//					      "NewLevelsAvailablePlain",
//					      "NewLevelsAvailablePlain",
//					      "NewLevelsAvailablePlain",
//					      5, -65,
//					      GAMETEXT_NEWLEVELS_AVAIBLE, 200, 200);
//    m_pNewLevelsAvailable->setFont(drawlib->getFontSmall());      
//    m_pNewLevelsAvailable->setID("NEWLEVELAVAILBLE");
//    
//
//    
//    //m_pGameInfoWindow = new UIFrame(m_pMainMenu,47,20+getDispHeight()/2 + (m_nNumMainMenuButtons*57)/2,
//    //                                "",207,getDispHeight() - (20+getDispHeight()/2 + (m_nNumMainMenuButtons*57)/2));
//    //m_pGameInfoWindow->showWindow(true);
//
//    
//    /* OPTIONS */
//    m_pOptionsWindow = makeOptionsWindow(drawLib, m_pMainMenu, &m_Config);
//    _UpdateThemesLists();
//    _UpdateRoomsLists();
//    /* ***** */


void StateMainMenu::drawBackground() {
  if(m_pGame->getSession()->menuGraphics() != GFX_LOW && m_pGame->getSession()->ugly() == false) {
    DrawLib* drawlib = m_pGame->getDrawLib();
    int w = drawlib->getDispWidth();
    int h = drawlib->getDispHeight();

    if(m_pTitleTL != NULL)
      drawlib->drawImage(Vector2f(0, 0), Vector2f(w/2, h/2), m_pTitleTL);
    if(m_pTitleTR != NULL)
      drawlib->drawImage(Vector2f(w/2, 0), Vector2f(w, h/2), m_pTitleTR);
    if(m_pTitleBR != NULL)
      drawlib->drawImage(Vector2f(w/2, h/2), Vector2f(w, h), m_pTitleBR);
    if(m_pTitleBL != NULL)
      drawlib->drawImage(Vector2f(0, h/2), Vector2f(w/2, h), m_pTitleBL);
  } else {
    m_pGame->getDrawLib()->clearGraphics();
  }
}

UILevelList* StateMainMenu::buildQuickStartList() {
  UILevelList* v_list;
  UIQuickStartButton *v_quickStart = reinterpret_cast<UIQuickStartButton *>(m_GUI->getChild("MAIN:QUICKSTART"));

  v_list = new UILevelList(m_GUI, 0, 0, "", 0, 0);     
  v_list->setID("QUICKSTART_LIST");
  v_list->showWindow(false);
  
  createLevelListsSql(v_list,
		      LevelsManager::getQuickStartPackQuery(m_pGame->getDb(),
							    v_quickStart->getQualityMIN(),
							    v_quickStart->getDifficultyMIN(),
							    v_quickStart->getQualityMAX(),
							    v_quickStart->getDifficultyMAX(),
							    m_pGame->getSession()->profile(), m_pGame->getSession()->idRoom()));
  return v_list;
}

void StateMainMenu::createLevelListsSql(UILevelList *io_levelsList, const std::string& i_sql) {
  char **v_result;
  unsigned int nrow;
  float v_playerHighscore, v_roomHighscore;
  
  /* get selected item */
  std::string v_selected_levelName = "";
  if(io_levelsList->getSelected() >= 0 && io_levelsList->getSelected() < io_levelsList->getEntries().size()) {
    UIListEntry *pEntry = io_levelsList->getEntries()[io_levelsList->getSelected()];
    v_selected_levelName = pEntry->Text[0];
  }
  
  io_levelsList->clear();
  
  v_result = m_pGame->getDb()->readDB(i_sql,
				      nrow);
  for(unsigned int i=0; i<nrow; i++) {
    if(m_pGame->getDb()->getResult(v_result, 4, i, 2) == NULL) {
      v_playerHighscore = -1.0;
    } else {
      v_playerHighscore = atof(m_pGame->getDb()->getResult(v_result, 4, i, 2));
    }
    
    if(m_pGame->getDb()->getResult(v_result, 4, i, 3) == NULL) {
      v_roomHighscore = -1.0;
    } else {
      v_roomHighscore = atof(m_pGame->getDb()->getResult(v_result, 4, i, 3));
    }
    
    io_levelsList->addLevel(m_pGame->getDb()->getResult(v_result, 4, i, 0),
			    m_pGame->getDb()->getResult(v_result, 4, i, 1),
			    v_playerHighscore,
			    v_roomHighscore
			    );
  }
  m_pGame->getDb()->read_DB_free(v_result);    
  
  /* reselect the previous level */
  if(v_selected_levelName != "") {
    int nLevel = 0;
    for(int i=0; i<io_levelsList->getEntries().size(); i++) {
      if(io_levelsList->getEntries()[i]->Text[0] == v_selected_levelName) {
	nLevel = i;
	break;
      }
      }
    io_levelsList->setRealSelected(nLevel);
  }
}

void StateMainMenu::send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input) {
  if(i_id == "QUIT") {
    if(i_button == UI_MSGBOX_YES) {
      m_requestForEnd = true;
      m_pGame->requestEnd();
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

  if(i_id == "STATE_MANAGER" && i_message == "FAVORITES_UPDATED") {
    m_require_updateFavoriteLevelsList = true;
  }

  if(i_id == "REQUESTKEY") {
    v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:CONTROLS_TAB:KEY_ACTION_LIST"));
    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {    
      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      pEntry->Text[1] = i_message;
    }
  }

//	/* Good... is the key already in use? */
//	int nAlreadyUsedBy = _IsKeyInUse(NewKey);
//	if(nAlreadyUsedBy > 0 && nAlreadyUsedBy != nSel) {
//	  /* affect to the key already in use the current key */
//	  pActionList->getEntries()[nAlreadyUsedBy]->Text[1] = pActionList->getEntries()[nSel]->Text[1];
//	}
//	pActionList->getEntries()[nSel]->Text[1] = NewKey;
  //printf("Change key to %s\n", i_message.c_str());
}

void StateMainMenu::executeOneCommand(std::string cmd)
{
  if(cmd == "UPDATEPROFILE") {
    updateProfile();

    // update packs
    m_pGame->getLevelsManager()->makePacks(m_pGame->getDb(),
					   m_pGame->getSession()->profile(),
					   m_pGame->getUserConfig()->getString("WebHighscoresIdRoom"),
					   m_pGame->getSession()->debug());

    // update lists and stats
    updateLevelsPacksList();
    updateLevelsLists();
    updateReplaysList();
    updateStats();
    return;
  }

  if(cmd == "REPLAYS_DELETE") {
    UILevelList* v_list = reinterpret_cast<UILevelList *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_LIST"));
    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      if(pEntry != NULL) {
	try {
	  Replay::deleteReplay(pEntry->Text[0]);
	} catch(Exception &e) {
	  Logger::Log(e.getMsg().c_str());
	}
	try {
	  m_pGame->getDb()->replays_delete(pEntry->Text[0]);
	} catch(Exception &e) {
	  Logger::Log(e.getMsg().c_str());
	}
	updateReplaysList();
      }
    }
  }
}

void StateMainMenu::updateLevelsLists() {
  updateFavoriteLevelsList();
  updateNewLevelsList();
}

void StateMainMenu::createLevelLists(UILevelList *i_list, const std::string& i_packageName) {
  LevelsPack *v_levelsPack = &(m_pGame->getLevelsManager()->LevelsPackByName(i_packageName));
  createLevelListsSql(i_list, v_levelsPack->getLevelsWithHighscoresQuery(m_pGame->getSession()->profile(),
									 m_pGame->getSession()->idRoom()));
}

void StateMainMenu::updateFavoriteLevelsList() {
  createLevelLists((UILevelList *)m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:FAVORITE_TAB:FAVORITE_LIST"),
		   VPACKAGENAME_FAVORITE_LEVELS);
}

void StateMainMenu::updateNewLevelsList() {
  createLevelLists((UILevelList *)m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:NEWLEVELS_TAB:NEWLEVELS_LIST"),
		   VPACKAGENAME_NEW_LEVELS);
}

void StateMainMenu::updateLevelsPacksList() {
  UIPackTree *pTree = (UIPackTree *)m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:PACK_TAB:PACK_TREE");

  /* get selected item */
  std::string v_selected_packName = pTree->getSelectedEntry();
  std::string p_packName;
  LevelsManager* v_lm = m_pGame->getLevelsManager();

  pTree->clear();
    
  for(int i=0; i<v_lm->LevelsPacks().size(); i++) {
    p_packName = v_lm->LevelsPacks()[i]->Name();

    /* the unpackaged pack exists only in debug mode */
    if(p_packName != "" || m_pGame->getSession()->debug()) {
      if(p_packName == "") {
	p_packName = GAMETEXT_UNPACKED_LEVELS_PACK;
      }
	
      pTree->addPack(v_lm->LevelsPacks()[i],
		     v_lm->LevelsPacks()[i]->Group(),
		     v_lm->LevelsPacks()[i]->getNumberOfFinishedLevels(m_pGame->getDb(),
								       m_pGame->getSession()->profile()),
		     v_lm->LevelsPacks()[i]->getNumberOfLevels(m_pGame->getDb())
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

  v_listAll = ((UIButton *) m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_LIST_ALL"))->getChecked();
  v_list = (UIList *) m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_LIST");

  /* Clear list */
  v_list->clear();
    
  /* Enumerate replays */
  std::string PlayerSearch;
  if(v_listAll) {
    v_sql = "SELECT a.name, a.id_profile, b.name FROM replays AS a "
      "INNER JOIN levels AS b ON a.id_level = b.id_level;";
  } else {
    v_sql = "SELECT a.name, a.id_profile, b.name FROM replays AS a "
      "INNER JOIN levels AS b ON a.id_level = b.id_level "
      "WHERE a.id_profile=\"" + xmDatabase::protectString(m_pGame->getSession()->profile()) + "\";";
  }

  v_result = m_pGame->getDb()->readDB(v_sql, nrow);
  for(unsigned int i=0; i<nrow; i++) {
    UIListEntry *pEntry = v_list->addEntry(m_pGame->getDb()->getResult(v_result, 3, i, 0));
    pEntry->Text.push_back(m_pGame->getDb()->getResult(v_result, 3, i, 2));
    pEntry->Text.push_back(m_pGame->getDb()->getResult(v_result, 3, i, 1));
  }
  m_pGame->getDb()->read_DB_free(v_result); 

  // apply the filter
  UIEdit*      v_edit;
  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_FILTER"));
  v_list->setFilter(v_edit->getCaption());
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
  }

  // show
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_SHOW_BUTTON"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pListEntry = v_list->getEntries()[v_list->getSelected()];
      if(pListEntry != NULL) {
	m_pGame->getStateManager()->pushState(new StateReplaying(m_pGame, pListEntry->Text[0]));	  
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
	StateMessageBox* v_msgboxState = new StateMessageBox(this, m_pGame, GAMETEXT_DELETEREPLAYMESSAGE, UI_MSGBOX_YES|UI_MSGBOX_NO);
	v_msgboxState->setId("REPLAYS_DELETE");
	m_pGame->getStateManager()->pushState(v_msgboxState);	
      }
    }
  }

  // list all
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_LIST_ALL"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    updateReplaysList();      
  }


//    if(pReplaysList->getEntries().empty()) {
//      pReplaysShowButton->enableWindow(false);
//      pReplaysDeleteButton->enableWindow(false);
//    }
//    else {
//      pReplaysShowButton->enableWindow(true);
//      pReplaysDeleteButton->enableWindow(true);
//    }
//    
//    if(pReplaysList->isChanged()) {
//      pReplaysList->setChanged(false);
//      pUploadHighscoreButton->enableWindow(false);
//
//      if(m_xmsession->www()) {
//	if(pReplaysList->getSelected() >= 0 && pReplaysList->getSelected() < pReplaysList->getEntries().size()) {
//	  UIListEntry *pListEntry = pReplaysList->getEntries()[pReplaysList->getSelected()];
//	  if(pListEntry != NULL) {
//	    ReplayInfo* rplInfos;
//	    rplInfos = Replay::getReplayInfos(pListEntry->Text[0]);
//	    if(rplInfos != NULL) {
//	      if(rplInfos->fFinishTime > 0.0 && rplInfos->Player == m_xmsession->profile()) {
//		
//		char **v_result;
//		unsigned int nrow;
//		float v_finishTime;
//		
//		v_result = m_db->readDB("SELECT finishTime "
//					"FROM webhighscores WHERE id_level=\"" + 
//					xmDatabase::protectString(rplInfos->Level) + "\""
//					"AND id_room=" + m_WebHighscoresIdRoom + ";",
//					nrow);
//		if(nrow == 0) {
//		  pUploadHighscoreButton->enableWindow(true);
//		  m_db->read_DB_free(v_result);
//		} else {
//		  v_finishTime = atof(m_db->getResult(v_result, 1, 0, 0));
//		  m_db->read_DB_free(v_result);
//		  pUploadHighscoreButton->enableWindow(rplInfos->fFinishTime < v_finishTime);
//		}  	      
//	      }
//	      delete rplInfos; 
//	    }
//	  }
//	}
//      }
//    }
//    
//    if(pUploadHighscoreButton->isClicked()) {
//      pReplaysList->setClicked(false);
//      if(pReplaysList->getSelected() >= 0 && pReplaysList->getSelected() < pReplaysList->getEntries().size()) {
//        UIListEntry *pListEntry = pReplaysList->getEntries()[pReplaysList->getSelected()];
//        if(pListEntry != NULL) {
//	  uploadHighscore(pListEntry->Text[0]);
//	}
//      }
//    }
//
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
  
  v_result = m_pGame->getDb()->readDB("SELECT a.id_theme, a.checkSum, b.checkSum "
				      "FROM themes AS a LEFT OUTER JOIN webthemes AS b "
				      "ON a.id_theme=b.id_theme ORDER BY a.id_theme;",
			  nrow);
  for(unsigned int i=0; i<nrow; i++) {
    v_id_theme = m_pGame->getDb()->getResult(v_result, 3, i, 0);
    v_ck1      = m_pGame->getDb()->getResult(v_result, 3, i, 1);
    if(m_pGame->getDb()->getResult(v_result, 3, i, 2) != NULL) {
      v_ck2      = m_pGame->getDb()->getResult(v_result, 3, i, 2);
    }
    
    pEntry = pList->addEntry(v_id_theme.c_str(),
			     NULL);
    if(v_ck1 == v_ck2 || v_ck2 == "") {
      pEntry->Text.push_back(GAMETEXT_THEMEHOSTED);
    } else {
      pEntry->Text.push_back(GAMETEXT_THEMEREQUIREUPDATE);
    }
  }
  m_pGame->getDb()->read_DB_free(v_result);
  
  v_result = m_pGame->getDb()->readDB("SELECT a.id_theme FROM webthemes AS a LEFT OUTER JOIN themes AS b "
				      "ON a.id_theme=b.id_theme WHERE b.id_theme IS NULL;",
				      nrow);
  for(unsigned int i=0; i<nrow; i++) {
    v_id_theme = m_pGame->getDb()->getResult(v_result, 1, i, 0);
    pEntry = pList->addEntry(v_id_theme.c_str(), NULL);
    pEntry->Text.push_back(GAMETEXT_THEMENOTHOSTED);
  }
  m_pGame->getDb()->read_DB_free(v_result);
  
  /* reselect the previous theme */
  if(v_selected_themeName != "") {
    int nTheme = 0;
    for(int i=0; i<pList->getEntries().size(); i++) {
      if(pList->getEntries()[i]->Text[0] == v_selected_themeName) {
	nTheme = i;
	break;
      }
    }
    pList->setRealSelected(nTheme);
  }  
}

void StateMainMenu::updateThemesList() {
  createThemesList(reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:THEMES_LIST")));
}

void StateMainMenu::updateResolutionsList() {
  UIList* v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:RESOLUTIONS_LIST"));

  v_list->clear();
  std::vector<std::string>* modes = System::getDisplayModes(m_pGame->getSession()->windowed());
  for(int i=0; i < modes->size(); i++) {
    v_list->addEntry((*modes)[i].c_str());
  }
  delete modes;
}

void StateMainMenu::updateControlsList() {
  UIList *pList = (UIList *)m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:CONTROLS_TAB:KEY_ACTION_LIST");
  pList->clear();
    
  UIListEntry *p;
    
  p = pList->addEntry(GAMETEXT_DRIVE); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyDrive1"));
  p = pList->addEntry(GAMETEXT_BRAKE); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyBrake1"));
  p = pList->addEntry(GAMETEXT_FLIPLEFT); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyFlipLeft1"));
  p = pList->addEntry(GAMETEXT_FLIPRIGHT); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyFlipRight1"));
  p = pList->addEntry(GAMETEXT_CHANGEDIR); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyChangeDir1"));

  p = pList->addEntry(GAMETEXT_DRIVE + std::string(" 2")); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyDrive2"));
  p = pList->addEntry(GAMETEXT_BRAKE + std::string(" 2")); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyBrake2"));
  p = pList->addEntry(GAMETEXT_FLIPLEFT + std::string(" 2")); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyFlipLeft2"));
  p = pList->addEntry(GAMETEXT_FLIPRIGHT + std::string(" 2")); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyFlipRight2"));
  p = pList->addEntry(GAMETEXT_CHANGEDIR + std::string(" 2")); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyChangeDir2"));
   
  p = pList->addEntry(GAMETEXT_DRIVE + std::string(" 3")); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyDrive3"));
  p = pList->addEntry(GAMETEXT_BRAKE + std::string(" 3")); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyBrake3"));
  p = pList->addEntry(GAMETEXT_FLIPLEFT + std::string(" 3")); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyFlipLeft3"));
  p = pList->addEntry(GAMETEXT_FLIPRIGHT + std::string(" 3")); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyFlipRight3"));
  p = pList->addEntry(GAMETEXT_CHANGEDIR + std::string(" 3")); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyChangeDir3"));

  p = pList->addEntry(GAMETEXT_DRIVE + std::string(" 4")); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyDrive4"));
  p = pList->addEntry(GAMETEXT_BRAKE + std::string(" 4")); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyBrake4"));
  p = pList->addEntry(GAMETEXT_FLIPLEFT + std::string(" 4")); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyFlipLeft4"));
  p = pList->addEntry(GAMETEXT_FLIPRIGHT + std::string(" 4")); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyFlipRight4"));
  p = pList->addEntry(GAMETEXT_CHANGEDIR + std::string(" 4")); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyChangeDir4"));
 
#if defined(ENABLE_ZOOMING)
  p = pList->addEntry(GAMETEXT_ZOOMIN); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyZoomIn"));
  p = pList->addEntry(GAMETEXT_ZOOMOUT); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyZoomOut"));
  p = pList->addEntry(GAMETEXT_ZOOMINIT); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyZoomInit"));   
  p = pList->addEntry(GAMETEXT_CAMERAMOVEXUP); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyCameraMoveXUp"));
  p = pList->addEntry(GAMETEXT_CAMERAMOVEXDOWN); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyCameraMoveXDown"));
  p = pList->addEntry(GAMETEXT_CAMERAMOVEYUP); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyCameraMoveYUp"));
  p = pList->addEntry(GAMETEXT_CAMERAMOVEYDOWN); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyCameraMoveYDown"));
  p = pList->addEntry(GAMETEXT_AUTOZOOM); p->Text.push_back(m_pGame->getUserConfig()->getString("KeyAutoZoom"));
#endif
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
    delete pList->getEntries()[i]->pvUser;
  }
  pList->clear();

  v_result = m_pGame->getDb()->readDB("SELECT id_room, name FROM webrooms ORDER BY id_room ASC;",
				      nrow);

  for(unsigned int i=0; i<nrow; i++) {
    v_roomId   = m_pGame->getDb()->getResult(v_result, 2, i, 0);
    v_roomName = m_pGame->getDb()->getResult(v_result, 2, i, 1);
    pEntry = pList->addEntry(v_roomName,
			     reinterpret_cast<void *>(new std::string(v_roomId))
			     );    
  }
  m_pGame->getDb()->read_DB_free(v_result);

  /* reselect the previous room */
  if(v_selected_roomName != "") {
    int nRoom = 0;
    for(int i=0; i<pList->getEntries().size(); i++) {
      if(pList->getEntries()[i]->Text[0] == v_selected_roomName) {
	nRoom = i;
	break;
      }
    }
    pList->setRealSelected(nRoom);
  }
}


void StateMainMenu::updateRoomsList() {
  createRoomsList(reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB:ROOMS_LIST")));
}

void StateMainMenu::updateOptions() {
  UIButton* v_button;
  UIList*   v_list;
  UIEdit*   v_edit;

  // level tab (multi)
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:MULTI_TAB:ENABLEMULTISTOPWHENONEFINISHES"));
  v_button->setChecked(m_pGame->getSession()->MultiStopWhenOneFinishes());

  // options/general
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:SHOWMINIMAP"));
  v_button->setChecked(m_pGame->getSession()->showMinimap());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:SHOWENGINECOUNTER"));
  v_button->setChecked(m_pGame->getSession()->showEngineCounter());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:INITZOOM"));
  v_button->setChecked(m_pGame->getSession()->enableInitZoom());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:DEATHANIM"));
  v_button->setChecked(m_pGame->getSession()->enableDeadAnimation());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:ENABLECONTEXTHELP"));
  v_button->setChecked(m_pGame->getSession()->enableContextHelp());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:AUTOSAVEREPLAYS"));
  v_button->setChecked(m_pGame->getSession()->autosaveHighscoreReplays());

  // theme
  v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:THEMES_LIST"));
  std::string v_themeName = m_pGame->getSession()->theme();
  int nTheme = 0;
  for(int i=0; i<v_list->getEntries().size(); i++) {
    if(v_list->getEntries()[i]->Text[0] == v_themeName) {
      nTheme = i;
      break;
    }
  }
  v_list->setRealSelected(nTheme);  


  // video
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:16BPP"));
  v_button->setChecked(m_pGame->getSession()->bpp() == 16);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:32BPP"));
  v_button->setChecked(m_pGame->getSession()->bpp() == 32);

  v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:RESOLUTIONS_LIST"));
  char cBuf[256];
  sprintf(cBuf, "%d X %d", m_pGame->getSession()->resolutionWidth(), m_pGame->getSession()->resolutionHeight());
  int nMode = 0;
  for(int i=0; i<v_list->getEntries().size(); i++) {
    if(v_list->getEntries()[i]->Text[0] == std::string(cBuf)) {
      nMode = i;
      break;
    }
  }
  v_list->setRealSelected(nMode);  

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:WINDOWED"));
  v_button->setChecked(m_pGame->getSession()->windowed());

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:MENULOW"));
  v_button->setChecked(m_pGame->getSession()->menuGraphics() == GFX_LOW);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:MENUMEDIUM"));
  v_button->setChecked(m_pGame->getSession()->menuGraphics() == GFX_MEDIUM);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:MENUHIGH"));
  v_button->setChecked(m_pGame->getSession()->menuGraphics() == GFX_HIGH);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:GAMELOW"));
  v_button->setChecked(m_pGame->getSession()->gameGraphics() == GFX_LOW);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:GAMEMEDIUM"));
  v_button->setChecked(m_pGame->getSession()->gameGraphics() == GFX_MEDIUM);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:GAMEHIGH"));
  v_button->setChecked(m_pGame->getSession()->gameGraphics() == GFX_HIGH);

  // audio
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_AUDIO"));
  v_button->setChecked(m_pGame->getSession()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE11KHZ"));
  v_button->setChecked(m_pGame->getSession()->audioSampleRate() == 11025);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE22KHZ"));
  v_button->setChecked(m_pGame->getSession()->audioSampleRate() == 22050);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE44KHZ"));
  v_button->setChecked(m_pGame->getSession()->audioSampleRate() == 44100);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:8BITS"));
  v_button->setChecked(m_pGame->getSession()->audioSampleBits() == 8);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:16BITS"));
  v_button->setChecked(m_pGame->getSession()->audioSampleBits() == 16);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:MONO"));
  v_button->setChecked(m_pGame->getSession()->audioChannels() == 1);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:STEREO"));
  v_button->setChecked(m_pGame->getSession()->audioChannels() == 2);
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_ENGINE_SOUND"));
  v_button->setChecked(m_pGame->getSession()->enableEngineSound());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_MENU_MUSIC"));
  v_button->setChecked(m_pGame->getSession()->enableMenuMusic());

  // controls

  // www
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLEWEB"));
  v_button->setChecked(m_pGame->getSession()->www());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKNEWLEVELSATSTARTUP"));
  v_button->setChecked(m_pGame->getSession()->checkNewHighscoresAtStartup());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKHIGHSCORESATSTARTUP"));
  v_button->setChecked(m_pGame->getSession()->checkNewLevelsAtStartup());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:INGAMEWORLDRECORD"));
  v_button->setChecked(m_pGame->getSession()->showHighscoreInGame());

  v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB:ROOMS_LIST"));

  std::string v_id_room = m_pGame->getSession()->idRoom();
  if(v_id_room == "") {v_id_room = DEFAULT_WEBROOM_ID;}
  for(int i=0; i<v_list->getEntries().size(); i++) {
    if((*(std::string*)v_list->getEntries()[i]->pvUser) == v_id_room) {
      v_list->setRealSelected(i);
      break;
    }
  }

  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB:ROOM_LOGIN"));
  v_edit->setCaption(m_pGame->getSession()->uploadLogin());
  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB:ROOM_PASSWORD"));
  v_edit->setCaption(m_pGame->getSession()->uploadPassword());

  // ghosts
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:ENABLE_GHOSTS"));
  v_button->setChecked(m_pGame->getSession()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_MYBEST"));
  v_button->setChecked(m_pGame->getSession()->ghostStrategy_MYBEST());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_THEBEST"));
  v_button->setChecked(m_pGame->getSession()->ghostStrategy_THEBEST());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_BESTOFROOM"));
  v_button->setChecked(m_pGame->getSession()->ghostStrategy_BESTOFROOM());

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:DISPLAY_GHOST_TIMEDIFFERENCE"));
  v_button->setChecked(m_pGame->getSession()->showGhostTimeDifference());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:DISPLAY_GHOSTS_INFOS"));
  v_button->setChecked(m_pGame->getSession()->showGhostsInfos());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:HIDEGHOSTS"));
  v_button->setChecked(m_pGame->getSession()->hideGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:MOTION_BLUR_GHOST"));
  v_button->setChecked(m_pGame->getSession()->ghostMotionBlur());
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
    m_pGame->getSession()->setShowMinimap(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:SHOWENGINECOUNTER"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setShowEngineCounter(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:INITZOOM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setEnableInitZoom(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:DEATHANIM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setEnableDeadAnimation(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:ENABLECONTEXTHELP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setEnableContextHelp(v_button->getChecked()); 
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:AUTOSAVEREPLAYS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setAutosaveHighscoreReplays(v_button->getChecked()); 
  }

  v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GENERAL_TAB:THEMES_LIST"));
  if(v_list->isClicked()) {
    v_list->setClicked(false);
    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      m_pGame->getSession()->setTheme(pEntry->Text[0]);
      if(m_pGame->getTheme()->Name() != m_pGame->getSession()->theme()) {
	m_pGame->reloadTheme();
      }
    }
  }

  // video tab
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:16BPP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setBpp(16); 
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:32BPP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setBpp(32); 
  }

  v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:RESOLUTIONS_LIST"));
  if(v_list->isClicked()) {
    v_list->setClicked(false);
    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      int nW,nH;

      sscanf(pEntry->Text[0].c_str(),"%d X %d", &nW, &nH);
      m_pGame->getSession()->setResolutionWidth(nW);
      m_pGame->getSession()->setResolutionHeight(nH);
    }
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:WINDOWED"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setWindowed(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:MENULOW"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setMenuGraphics(GFX_LOW);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:MENUMEDIUM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setMenuGraphics(GFX_MEDIUM);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:MENUHIGH"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setMenuGraphics(GFX_HIGH);
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:GAMELOW"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setGameGraphics(GFX_LOW);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:GAMEMEDIUM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setGameGraphics(GFX_MEDIUM);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:VIDEO_TAB:GAMEHIGH"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setGameGraphics(GFX_HIGH);
  }

  // sound
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_AUDIO"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    bool v_before = m_pGame->getSession()->enableAudio();
    m_pGame->getSession()->setEnableAudio(v_button->getChecked());
    Sound::setActiv(m_pGame->getSession()->enableAudio());
    updateAudioOptions();
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE11KHZ"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setAudioSampleRate(11025);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE22KHZ"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setAudioSampleRate(22050);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE44KHZ"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setAudioSampleRate(44100);
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:8BITS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setAudioSampleBits(8);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:16BITS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setAudioSampleBits(16);
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:MONO"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setAudioChannels(1);
  }
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:STEREO"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setAudioChannels(2);
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_ENGINE_SOUND"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setEnableEngineSound(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_MENU_MUSIC"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);

    bool v_before = m_pGame->getSession()->enableMenuMusic();
    m_pGame->getSession()->setEnableMenuMusic(v_button->getChecked());
    Sound::setActiv(m_pGame->getSession()->enableAudio());
  }

  // controls
  v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:CONTROLS_TAB:KEY_ACTION_LIST"));
  if(v_list->isItemActivated()) {
    v_list->setItemActivated(false);

    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      char cBuf[1024];                

      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      sprintf(cBuf, GAMETEXT_PRESSANYKEYTO, pEntry->Text[0].c_str());
      m_pGame->getStateManager()->pushState(new StateRequestKey(m_pGame, cBuf, this));      
    }
  }

  // www
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLEWEB"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setWWW(v_button->getChecked());
    updateWWWOptions();
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKNEWLEVELSATSTARTUP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setCheckNewLevelsAtStartup(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKHIGHSCORESATSTARTUP"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setCheckNewHighscoresAtStartup(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:INGAMEWORLDRECORD"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setShowHighscoreInGame(v_button->getChecked());
  }

  v_list = reinterpret_cast<UIList *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB:ROOMS_LIST"));
  if(v_list->isClicked()) {
    v_list->setClicked(false);
    if(v_list->getSelected() >= 0 && v_list->getSelected() < v_list->getEntries().size()) {
      UIListEntry *pEntry = v_list->getEntries()[v_list->getSelected()];
      m_pGame->getSession()->setIdRoom(*((std::string*)v_list->getEntries()[v_list->getSelected()]->pvUser));
    }
  }

  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB:ROOM_LOGIN"));
  if(v_edit->hasChanged()) {
    v_edit->setHasChanged(false);
    m_pGame->getSession()->setUploadLogin(v_edit->getCaption());
  }

  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:ROOMS_TAB:ROOM_PASSWORD"));
  if(v_edit->hasChanged()) {
    v_edit->setHasChanged(false);
    m_pGame->getSession()->setUploadPassword(v_edit->getCaption());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:ENABLE_GHOSTS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setEnableGhosts(v_button->getChecked());
    updateGhostsOptions();
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_MYBEST"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setGhostStrategy_MYBEST(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_THEBEST"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setGhostStrategy_THEBEST(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_BESTOFROOM"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setGhostStrategy_BESTOFROOM(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:DISPLAY_GHOST_TIMEDIFFERENCE"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setShowGhostTimeDifference(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:DISPLAY_GHOSTS_INFOS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setShowGhostsInfos(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:HIDEGHOSTS"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setHideGhosts(v_button->getChecked());
  }

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:MOTION_BLUR_GHOST"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
    m_pGame->getSession()->setGhostMotionBlur(v_button->getChecked());
  }
}

void StateMainMenu::updateAudioOptions() {
  UIButton* v_button;

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE11KHZ"));
  v_button->enableWindow(m_pGame->getSession()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE22KHZ"));
  v_button->enableWindow(m_pGame->getSession()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:RATE44KHZ"));
  v_button->enableWindow(m_pGame->getSession()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:8BITS"));
  v_button->enableWindow(m_pGame->getSession()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:16BITS"));
  v_button->enableWindow(m_pGame->getSession()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:MONO"));
  v_button->enableWindow(m_pGame->getSession()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:STEREO"));
  v_button->enableWindow(m_pGame->getSession()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_ENGINE_SOUND"));
  v_button->enableWindow(m_pGame->getSession()->enableAudio());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:AUDIO_TAB:ENABLE_MENU_MUSIC"));
  v_button->enableWindow(m_pGame->getSession()->enableAudio());
}

void StateMainMenu::updateWWWOptions() {
  UIButton* v_button;

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKNEWLEVELSATSTARTUP"));
  v_button->enableWindow(m_pGame->getSession()->www());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:WWW_TAB:TABS:MAIN_TAB:ENABLECHECKHIGHSCORESATSTARTUP"));
  v_button->enableWindow(m_pGame->getSession()->www()); 
}

void StateMainMenu::updateGhostsOptions() {
  UIButton* v_button;

  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_MYBEST"));
  v_button->enableWindow(m_pGame->getSession()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_THEBEST"));
  v_button->enableWindow(m_pGame->getSession()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:GHOST_STRATEGY_BESTOFROOM"));
  v_button->enableWindow(m_pGame->getSession()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:DISPLAY_GHOST_TIMEDIFFERENCE"));
  v_button->enableWindow(m_pGame->getSession()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:DISPLAY_GHOSTS_INFOS"));
  v_button->enableWindow(m_pGame->getSession()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:HIDEGHOSTS"));
  v_button->enableWindow(m_pGame->getSession()->enableGhosts());
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_OPTIONS:TABS:GHOSTS_TAB:MOTION_BLUR_GHOST"));
  v_button->enableWindow(m_pGame->getSession()->enableGhosts());
}
