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
}

void StateMainMenu::leave()
{
  StateMenu::leave();
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
  UIButton *v_button;
  std::string v_id_level;

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
    m_pGame->getStateManager()->pushState(new StatePreplaying(m_pGame, v_id_level));    
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
  v_someText = new UIStatic(v_menu, 200, m_sGUI->getPosition().nHeight*2/15, "Nicolas", m_sGUI->getPosition().nWidth-200-120,50);
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
}

void StateMainMenu::updateProfile() {
  UIStatic* v_playerTag = reinterpret_cast<UIStatic *>(m_GUI->getChild("MAIN:PLAYERTAG"));
  v_playerTag->setCaption(std::string(GAMETEXT_PLAYER) + ": " + m_pGame->getSession()->profile()
			  + "@" + m_pGame->getHighscoresRoomName());
}


UIWindow* makeWindowLevels() {
//    m_pLevelPacksWindow = new UIFrame(m_pMainMenu,220,(drawlib->getDispHeight()*140)/600,"",drawlib->getDispWidth()-220-20,drawlib->getDispHeight()-40-(drawlib->getDispHeight()*120)/600-10);      
//    m_pLevelPacksWindow->showWindow(false);
//    pSomeText = new UIStatic(m_pLevelPacksWindow,0,0,GAMETEXT_LEVELS,m_pLevelPacksWindow->getPosition().nWidth,36);
//    pSomeText->setFont(drawlib->getFontMedium());
//
//    /* tabs of the packs */
//    m_pLevelPackTabs = new UITabView(m_pLevelPacksWindow,20,40,"",m_pLevelPacksWindow->getPosition().nWidth-40,m_pLevelPacksWindow->getPosition().nHeight-60);
//    m_pLevelPackTabs->setFont(drawlib->getFontSmall());
//    m_pLevelPackTabs->setID("LEVELPACK_TABS");
//    m_pLevelPackTabs->enableWindow(true);
//    m_pLevelPackTabs->showWindow(true);
//    m_pLevelPackTabs->setTabContextHelp(0,CONTEXTHELP_LEVEL_PACKS);
//    m_pLevelPackTabs->setTabContextHelp(1,CONTEXTHELP_BUILT_IN_AND_EXTERNALS);
//    m_pLevelPackTabs->setTabContextHelp(2,CONTEXTHELP_NEW_LEVELS);
//
//    /* pack tab */
//    UIWindow *pPackTab = new UIWindow(m_pLevelPackTabs,10,40,GAMETEXT_LEVELPACKS,m_pLevelPackTabs->getPosition().nWidth-20,m_pLevelPackTabs->getPosition().nHeight);
//    pPackTab->enableWindow(true);
//    pPackTab->showWindow(true);
//    pPackTab->setID("PACK_TAB");
//
//    /* open button */
//    UIButton *pOpenButton = new UIButton(pPackTab,11,pPackTab->getPosition().nHeight-57-45,GAMETEXT_OPEN,115,57);
//    pOpenButton->setFont(drawlib->getFontSmall());
//    pOpenButton->setType(UI_BUTTON_TYPE_SMALL);
//    pOpenButton->setID("LEVELPACK_OPEN_BUTTON");
//    pOpenButton->setContextHelp(CONTEXTHELP_VIEW_LEVEL_PACK);
//
//    /* pack list */
//    UIPackTree *pLevelPackTree = new UIPackTree(pPackTab,10,0,"",pPackTab->getPosition().nWidth-20,pPackTab->getPosition().nHeight-105);      
//    pLevelPackTree->setID("LEVELPACK_TREE");
//    pLevelPackTree->showWindow(true);
//    pLevelPackTree->enableWindow(true);
//    pLevelPackTree->setFont(drawlib->getFontSmall());
//    pLevelPackTree->setEnterButton( pOpenButton );
//
//    /* favorite levels tab */
//    UIWindow *pAllLevelsPackTab = new UIWindow(m_pLevelPackTabs,20,40,VPACKAGENAME_FAVORITE_LEVELS,m_pLevelPackTabs->getPosition().nWidth-40,m_pLevelPackTabs->getPosition().nHeight);
//    pAllLevelsPackTab->enableWindow(true);
//    pAllLevelsPackTab->showWindow(false);
//    pAllLevelsPackTab->setID("ALLLEVELS_TAB");
//
//    /* all levels button */
//    UIButton *pGoButton = new UIButton(pAllLevelsPackTab,0,pAllLevelsPackTab->getPosition().nHeight-103,GAMETEXT_STARTLEVEL,105,57);
//    pGoButton->setContextHelp(CONTEXTHELP_PLAY_SELECTED_LEVEL);
//    pGoButton->setFont(drawlib->getFontSmall());
//    pGoButton->setType(UI_BUTTON_TYPE_SMALL);
//    pGoButton->setID("PLAY_GO_BUTTON");
//    UIButton *pLevelInfoButton = new UIButton(pAllLevelsPackTab,105,pAllLevelsPackTab->getPosition().nHeight-103,GAMETEXT_SHOWINFO,105,57);
//    pLevelInfoButton->setFont(drawlib->getFontSmall());
//    pLevelInfoButton->setType(UI_BUTTON_TYPE_SMALL);
//    pLevelInfoButton->setID("PLAY_LEVEL_INFO_BUTTON");
//    pLevelInfoButton->setContextHelp(CONTEXTHELP_LEVEL_INFO);
//
//    UIButton *pDeleteFromFavoriteButton = new UIButton(pAllLevelsPackTab,pAllLevelsPackTab->getPosition().nWidth-187,pAllLevelsPackTab->getPosition().nHeight-103,GAMETEXT_DELETEFROMFAVORITE,187,57);
//    pDeleteFromFavoriteButton->setFont(drawlib->getFontSmall());
//    pDeleteFromFavoriteButton->setType(UI_BUTTON_TYPE_LARGE);
//    pDeleteFromFavoriteButton->setID("ALL_LEVELS_DELETE_FROM_FAVORITE_BUTTON");
//    pDeleteFromFavoriteButton->setContextHelp(CONTEXTHELP_DELETEFROMFAVORITE);
//
//    /* all levels list */
//    m_pAllLevelsList = new UILevelList(pAllLevelsPackTab,0,0,"",pAllLevelsPackTab->getPosition().nWidth,pAllLevelsPackTab->getPosition().nHeight-105);     
//    m_pAllLevelsList->setID("ALLLEVELS_LIST");
//    m_pAllLevelsList->setFont(drawlib->getFontSmall());
//    m_pAllLevelsList->setSort(true);
//    m_pAllLevelsList->setEnterButton( pGoButton );
//
//    /* new levels tab */
//    UIWindow *pNewLevelsPackTab = new UIWindow(m_pLevelPackTabs,20,40,GAMETEXT_NEWLEVELS,m_pLevelPackTabs->getPosition().nWidth-40,m_pLevelPackTabs->getPosition().nHeight);
//    pNewLevelsPackTab->enableWindow(true);
//    pNewLevelsPackTab->showWindow(false);
//    pNewLevelsPackTab->setID("NEWLEVELS_TAB");
//
//    /* new levels tab buttons */
//    UIButton *pNewLevelsGoButton = new UIButton(pNewLevelsPackTab,0,pNewLevelsPackTab->getPosition().nHeight-103,GAMETEXT_STARTLEVEL,105,57);
//    pNewLevelsGoButton->setContextHelp(CONTEXTHELP_PLAY_SELECTED_LEVEL);
//    pNewLevelsGoButton->setFont(drawlib->getFontSmall());
//    pNewLevelsGoButton->setType(UI_BUTTON_TYPE_SMALL);
//    pNewLevelsGoButton->setID("NEW_LEVELS_PLAY_GO_BUTTON");
//    UIButton *pNewLevelsLevelInfoButton = new UIButton(pNewLevelsPackTab,105,pNewLevelsPackTab->getPosition().nHeight-103,GAMETEXT_SHOWINFO,105,57);
//    pNewLevelsLevelInfoButton->setFont(drawlib->getFontSmall());
//    pNewLevelsLevelInfoButton->setType(UI_BUTTON_TYPE_SMALL);
//    pNewLevelsLevelInfoButton->setID("NEW_LEVELS_PLAY_LEVEL_INFO_BUTTON");
//    pNewLevelsLevelInfoButton->setContextHelp(CONTEXTHELP_LEVEL_INFO);
//
//    UIButton *pDownloadLevelsButton = new UIButton(pNewLevelsPackTab,pNewLevelsPackTab->getPosition().nWidth-187,pNewLevelsPackTab->getPosition().nHeight-103,GAMETEXT_DOWNLOADLEVELS,187,57);
//    pDownloadLevelsButton->setFont(drawlib->getFontSmall());
//    pDownloadLevelsButton->setType(UI_BUTTON_TYPE_LARGE);
//    pDownloadLevelsButton->setID("NEW_LEVELS_PLAY_DOWNLOAD_LEVELS_BUTTON");
//    pDownloadLevelsButton->setContextHelp(CONTEXTHELP_DOWNLOADLEVELS);
//
//    /* all levels list */
//    m_pPlayNewLevelsList = new UILevelList(pNewLevelsPackTab,0,0,"",pNewLevelsPackTab->getPosition().nWidth,pNewLevelsPackTab->getPosition().nHeight-105);     
//    m_pPlayNewLevelsList->setID("NEWLEVELS_LIST");
//    m_pPlayNewLevelsList->setFont(drawlib->getFontSmall());
//    m_pPlayNewLevelsList->setSort(true);
//    m_pPlayNewLevelsList->setEnterButton( pNewLevelsGoButton );
//
//    // multi tab
//    UIWindow *pMultiOptionsTab = new UIWindow(m_pLevelPackTabs, 20, 40, GAMETEXT_MULTI,
//					      m_pLevelPackTabs->getPosition().nWidth-40,
//					      m_pLevelPackTabs->getPosition().nHeight);
//    pMultiOptionsTab->enableWindow(true);
//    pMultiOptionsTab->showWindow(false);
//    pMultiOptionsTab->setID("MULTI_TAB");
//
//    pSomeText = new UIStatic(pMultiOptionsTab, 10, 0,
//			     GAMETEXT_NB_PLAYERS, pMultiOptionsTab->getPosition().nWidth, 40);
//    pSomeText->setFont(drawlib->getFontMedium());
//    pSomeText->setHAlign(UI_ALIGN_LEFT);
//
//    for(unsigned int i=0; i<4; i++) {
//      UIButton *pNbPlayers;
//      std::ostringstream s_nbPlayers;
//      char strPlayer[64];
//      snprintf(strPlayer, 64, GAMETEXT_NPLAYER(i+1), i+1);
//      s_nbPlayers << (int) i+1;
//
//      pNbPlayers = new UIButton(pMultiOptionsTab, 0, 40+(i*20), strPlayer, pMultiOptionsTab->getPosition().nWidth, 28);
//      pNbPlayers->setType(UI_BUTTON_TYPE_RADIO);
//      pNbPlayers->setID("MULTINB_" + s_nbPlayers.str());
//      pNbPlayers->enableWindow(true);
//      pNbPlayers->setFont(drawlib->getFontSmall());
//      pNbPlayers->setGroup(10200);
//      pNbPlayers->setContextHelp(CONTEXTHELP_MULTI);
//
//      // always check the 1 player mode
//      if(i == 0) {
//	pNbPlayers->setChecked(true);
//      }
//    }
//
//    UIButton *pMultiStopWhenOneFinishes = new UIButton(pMultiOptionsTab, 0, pMultiOptionsTab->getPosition().nHeight - 40 - 28 - 10,
//						       GAMETEXT_MULTISTOPWHENONEFINISHES,
//						       pMultiOptionsTab->getPosition().nWidth,28);
//    pMultiStopWhenOneFinishes->setType(UI_BUTTON_TYPE_CHECK);
//    pMultiStopWhenOneFinishes->setID("ENABLEMULTISTOPWHENONEFINISHES");
//    pMultiStopWhenOneFinishes->enableWindow(true);
//    pMultiStopWhenOneFinishes->setFont(drawlib->getFontSmall());
//    pMultiStopWhenOneFinishes->setGroup(50050);
//    pMultiStopWhenOneFinishes->setContextHelp(CONTEXTHELP_MULTISTOPWHENONEFINISHES);    
//    pMultiStopWhenOneFinishes->setChecked(m_xmsession->MultiStopWhenOneFinishes());
//

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
//    m_pReplaysWindow = new UIFrame(m_pMainMenu,220,(drawlib->getDispHeight()*140)/600,"",drawlib->getDispWidth()-220-20,drawlib->getDispHeight()-40-(drawlib->getDispHeight()*120)/600-10);      
//    m_pReplaysWindow->showWindow(false);
//    pSomeText = new UIStatic(m_pReplaysWindow,0,0,GAMETEXT_REPLAYS,m_pReplaysWindow->getPosition().nWidth,36);
//    pSomeText->setFont(drawlib->getFontMedium());
//
//    pSomeText = new UIStatic(m_pReplaysWindow, 10, 35, std::string(GAMETEXT_FILTER) + ":", 90, 25);
//    pSomeText->setFont(drawlib->getFontSmall());
//    pSomeText->setHAlign(UI_ALIGN_RIGHT);
//    UIEdit *pLevelFilterEdit = new UIEdit(m_pReplaysWindow,
//					  110,
//					  35,
//					  "",200,25);
//    pLevelFilterEdit->setFont(drawlib->getFontSmall());
//    pLevelFilterEdit->setID("REPLAYS_FILTER");
//    pLevelFilterEdit->setContextHelp(CONTEXTHELP_REPLAYS_FILTER);
//
//    /* show button */
//    UIButton *pShowButton = new UIButton(m_pReplaysWindow,5,m_pReplaysWindow->getPosition().nHeight-68,GAMETEXT_SHOW,105,57);
//    pShowButton->setFont(drawlib->getFontSmall());
//    pShowButton->setType(UI_BUTTON_TYPE_SMALL);
//    pShowButton->setID("REPLAY_SHOW_BUTTON");
//    pShowButton->setContextHelp(CONTEXTHELP_RUN_REPLAY);
//    /* delete button */
//    UIButton *pDeleteButton = new UIButton(m_pReplaysWindow,105,m_pReplaysWindow->getPosition().nHeight-68,GAMETEXT_DELETE,105,57);
//    pDeleteButton->setFont(drawlib->getFontSmall());
//    pDeleteButton->setType(UI_BUTTON_TYPE_SMALL);
//    pDeleteButton->setID("REPLAY_DELETE_BUTTON");
//    pDeleteButton->setContextHelp(CONTEXTHELP_DELETE_REPLAY);
//
//    /* upload button */
//    UIButton *pUploadHighscoreButton = new UIButton(m_pReplaysWindow,199,m_pReplaysWindow->getPosition().nHeight-68,GAMETEXT_UPLOAD_HIGHSCORE,186,57);
//    pUploadHighscoreButton->setFont(drawlib->getFontSmall());
//    pUploadHighscoreButton->setType(UI_BUTTON_TYPE_SMALL);
//    pUploadHighscoreButton->setID("REPLAY_UPLOADHIGHSCORE_BUTTON");
//    pUploadHighscoreButton->enableWindow(false);
//    pUploadHighscoreButton->setContextHelp(CONTEXTHELP_UPLOAD_HIGHSCORE);
//
//    /* filter */
//    UIButton *pListAllButton = new UIButton(m_pReplaysWindow,m_pReplaysWindow->getPosition().nWidth-105,m_pReplaysWindow->getPosition().nHeight-68,GAMETEXT_LISTALL,115,57);
//    pListAllButton->setFont(drawlib->getFontSmall());
//    pListAllButton->setType(UI_BUTTON_TYPE_CHECK);
//    pListAllButton->setChecked(false);
//    pListAllButton->setID("REPLAY_LIST_ALL");
//    pListAllButton->setContextHelp(CONTEXTHELP_ALL_REPLAYS);
//    /* */
//    UIList *pReplayList = new UIList(m_pReplaysWindow,20,65,"",m_pReplaysWindow->getPosition().nWidth-40,m_pReplaysWindow->getPosition().nHeight-115-25);
//    pReplayList->setID("REPLAY_LIST");
//    pReplayList->showWindow(true);
//    pReplayList->setFont(drawlib->getFontSmall());
//    pReplayList->addColumn(GAMETEXT_REPLAY, pReplayList->getPosition().nWidth/2 - 100,CONTEXTHELP_REPLAYCOL);
//    pReplayList->addColumn(GAMETEXT_LEVEL,  pReplayList->getPosition().nWidth/2 - 28,CONTEXTHELP_REPLAYLEVELCOL);
//    pReplayList->addColumn(GAMETEXT_PLAYER,128,CONTEXTHELP_REPLAYPLAYERCOL);
//    pReplayList->setEnterButton( pShowButton );
//    
//    /* OPTIONS */
//    m_pOptionsWindow = makeOptionsWindow(drawLib, m_pMainMenu, &m_Config);
//    _UpdateThemesLists();
//    _UpdateRoomsLists();
//    /* ***** */
//
//    /* HELP */
//    m_pHelpWindow = makeHelpWindow(drawLib, m_pMainMenu, &m_Config);
//    /* ***** */
//

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
