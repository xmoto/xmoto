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
#include "LevelsManager.h"

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
  
}

StateMainMenu::~StateMainMenu()
{
  if(m_quickStartList != NULL) {
    delete m_quickStartList;
  }
}


void StateMainMenu::enter()
{
  StateMenu::enter();

  m_pGame->playMusic("");
  
  createGUIIfNeeded(m_pGame);
  m_GUI = m_sGUI;

  // updates
  updateProfile();
  updateOptions();
  updateLevelsPacksList();
  updateLevelsLists();
  updateReplaysList();
  updateStats();
}

void StateMainMenu::leave()
{
  UIQuickStartButton *v_quickStart;

  StateMenu::leave();

  v_quickStart = reinterpret_cast<UIQuickStartButton *>(m_GUI->getChild("MAIN:QUICKSTART"));
  m_pGame->getUserConfig()->setInteger("QSQualityMIN",    v_quickStart->getQualityMIN());
  m_pGame->getUserConfig()->setInteger("QSDifficultyMIN", v_quickStart->getDifficultyMIN());
  m_pGame->getUserConfig()->setInteger("QSQualityMAX",    v_quickStart->getQualityMAX());
  m_pGame->getUserConfig()->setInteger("QSDifficultyMAX", v_quickStart->getDifficultyMAX());
}

void StateMainMenu::enterAfterPop()
{
  StateMenu::enterAfterPop();
}

void StateMainMenu::leaveAfterPush()
{
  StateMenu::leaveAfterPush();
}

void StateMainMenu::checkEvents() {
  UIButton*    v_button;
  UILevelList* v_list;
  std::string  v_id_level;

  // quickstart
  v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:QUICKSTART"));
  if(v_button->isClicked()) {
    v_button->setClicked(false);
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

  // level tab
  checkEventsLevelsFavoriteTab();
  checkEventsLevelsNewTab();

  // replay tab
  checkEventsReplays();
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
					pGame->getUserConfig()->getInteger("QSQualityMIN"),
					pGame->getUserConfig()->getInteger("QSDifficultyMIN"),
					pGame->getUserConfig()->getInteger("QSQualityMAX"),
					pGame->getUserConfig()->getInteger("QSDifficultyMAX")
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

void StateMainMenu::updateOptions() {
  UIButton* v_button = reinterpret_cast<UIButton *>(m_GUI->getChild("MAIN:FRAME_LEVELS:TABS:MULTI_TAB:ENABLEMULTISTOPWHENONEFINISHES"));
  v_button->setChecked(m_pGame->getSession()->MultiStopWhenOneFinishes());
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

UIWindow* StateMainMenu::makeWindowOptions(GameApp* pGame, UIWindow* i_parent) {
  UIWindow* v_window;
  UIStatic* v_someText;
 
  v_window = new UIFrame(i_parent, 220, i_parent->getPosition().nHeight*7/30, "",
			 i_parent->getPosition().nWidth -220 -20,
			 i_parent->getPosition().nHeight -40 -i_parent->getPosition().nHeight/5 -10);
  v_window->setID("FRAME_OPTIONS");
  v_window->showWindow(false);
   
  v_someText = new UIStatic(v_window, 0, 0, GAMETEXT_OPTIONS, v_window->getPosition().nWidth, 36);
  v_someText->setFont(pGame->getDrawLib()->getFontMedium());

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
  if(m_pGame->getSession()->menuGraphics() != MENU_GFX_LOW && m_pGame->getSession()->ugly() == false) {
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
							    m_pGame->getSession()->profile(), m_pGame->getHighscoresRoomId()));
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
    switch(i_button) {
    case UI_MSGBOX_YES:
      m_requestForEnd = true;
      m_pGame->requestEnd();
      break;
    case UI_MSGBOX_NO:
      return;
      break;
    }
  }
}

void StateMainMenu::executeOneCommand(std::string cmd)
{
  if(cmd == "UPDATEPROFILE"){
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
  }
}

void StateMainMenu::updateLevelsLists() {
  updateFavoriteLevelsList();
  updateNewLevelsList();
}

void StateMainMenu::createLevelLists(UILevelList *i_list, const std::string& i_packageName) {
  LevelsPack *v_levelsPack = &(m_pGame->getLevelsManager()->LevelsPackByName(i_packageName));
  createLevelListsSql(i_list, v_levelsPack->getLevelsWithHighscoresQuery(m_pGame->getSession()->profile(),
									 m_pGame->getHighscoresRoomId()));
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
}

void StateMainMenu::checkEventsReplays() {
  UIEdit*      v_edit;
  UILevelList* v_list;

  v_list = reinterpret_cast<UILevelList *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_LIST"));

  /* check filter */
  v_edit = reinterpret_cast<UIEdit *>(m_GUI->getChild("MAIN:FRAME_REPLAYS:REPLAYS_FILTER"));
  if(v_edit->hasChanged()) {
    v_edit->setHasChanged(false);
    v_list->setFilter(v_edit->getCaption());
  }

//    if(pReplayFilterEdit != NULL) {
//      if(pReplayFilterEdit->hasChanged()) {
//	pReplayFilterEdit->setHasChanged(false);
//	pReplayList->setFilter(pReplayFilterEdit->getCaption());
//      }
//    }
//
//    /* REPLAYS */        
//    UIButton *pReplaysShowButton = (UIButton *)m_pReplaysWindow->getChild("REPLAY_SHOW_BUTTON");
//    UIButton *pReplaysDeleteButton = (UIButton *)m_pReplaysWindow->getChild("REPLAY_DELETE_BUTTON");
//    UIButton *pUploadHighscoreButton = (UIButton *)m_pReplaysWindow->getChild("REPLAY_UPLOADHIGHSCORE_BUTTON");
//    UIButton *pReplaysListAllButton = (UIButton *)m_pReplaysWindow->getChild("REPLAY_LIST_ALL");
//    UIList *pReplaysList = (UIList *)m_pReplaysWindow->getChild("REPLAY_LIST");
//    
//    if(pReplaysList->getEntries().empty()) {
//      pReplaysShowButton->enableWindow(false);
//      pReplaysDeleteButton->enableWindow(false);
//    }
//    else {
//      pReplaysShowButton->enableWindow(true);
//      pReplaysDeleteButton->enableWindow(true);
//    }
//
//    if(pReplaysListAllButton->isClicked()) {
//      _UpdateReplaysList();      
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
//    if(pReplaysShowButton->isClicked()) {
//      /* Show replay */
//      if(pReplaysList->getSelected() >= 0 && pReplaysList->getSelected() < pReplaysList->getEntries().size()) {
//        UIListEntry *pListEntry = pReplaysList->getEntries()[pReplaysList->getSelected()];
//        if(pListEntry != NULL) {
//          /* Do it captain */
//          pReplaysShowButton->setClicked(false);
//
//          m_PlaySpecificReplay = pListEntry->Text[0];
//	  m_stateManager->pushState(new StateReplaying(this, pListEntry->Text[0]));	  
//        }
//      }
//    }
//    
//    if(pReplaysDeleteButton->isClicked()) {
//      /* Delete replay - but ask the user first */
//      if(m_pDeleteReplayMsgBox == NULL) {
//	m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
//        m_pDeleteReplayMsgBox = m_Renderer->getGUI()->msgBox(GAMETEXT_DELETEREPLAYMESSAGE,
//                                                            (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
//      }
//    }
//
}
