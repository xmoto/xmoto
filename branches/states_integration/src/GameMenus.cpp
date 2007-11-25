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

/* 
 *  Game application. (menus)
 */
#include "GameText.h"
#include "Game.h"
#include "VFileIO.h"
#include "Sound.h"
#include "helpers/Log.h"
#include "XMBuild.h"
#include "XMSession.h"
#include "drawlib/DrawLib.h"
#include "gui/specific/GUIXMoto.h"
#include "xmscene/Camera.h"
#include "states/StateManager.h"

#include "PhysSettings.h"

  void GameApp::_InitMenus_MainMenu(void) {
    /* Initialize main menu */      

    m_pMainMenu = new UIWindow(m_Renderer->getGUI(),0,0,"",
                                m_Renderer->getGUI()->getPosition().nWidth,
                                m_Renderer->getGUI()->getPosition().nHeight);
    m_pMainMenu->showWindow(true);
    m_pMainMenu->enableWindow(true);                                 
    
    m_pMainMenuButtons[0] = new UIButton(m_pMainMenu,0,0,GAMETEXT_LEVELS,177,57);
    m_pMainMenuButtons[0]->setContextHelp(CONTEXTHELP_LEVELS);
    
    m_pMainMenuButtons[1] = new UIButton(m_pMainMenu,0,0,GAMETEXT_REPLAYS,177,57);
    m_pMainMenuButtons[1]->setContextHelp(CONTEXTHELP_REPLAY_LIST);
    
    m_pMainMenuButtons[2] = new UIButton(m_pMainMenu,0,0,GAMETEXT_OPTIONS,177,57);
    m_pMainMenuButtons[2]->setContextHelp(CONTEXTHELP_OPTIONS);

    m_pMainMenuButtons[3] = new UIButton(m_pMainMenu,0,0,GAMETEXT_HELP,177,57);
    m_pMainMenuButtons[3]->setContextHelp(CONTEXTHELP_HELP);

    m_pMainMenuButtons[4] = new UIButton(m_pMainMenu,0,0,GAMETEXT_QUIT,177,57);
    m_pMainMenuButtons[4]->setContextHelp(CONTEXTHELP_QUIT_THE_GAME);

    m_nNumMainMenuButtons = 5;
    
    /* level info frame */
    m_pLevelInfoFrame = new UIWindow(m_pMainMenu,0,drawLib->getDispHeight()/2 - (m_nNumMainMenuButtons*57)/2 + m_nNumMainMenuButtons*57,"",220,100);
    m_pLevelInfoFrame->showWindow(false);
    m_pBestPlayerText = new UIStatic(m_pLevelInfoFrame, 0, 5,"", 220, 50);
    m_pBestPlayerText->setFont(drawLib->getFontSmall());
    m_pBestPlayerText->setHAlign(UI_ALIGN_CENTER);
    m_pBestPlayerText->showWindow(true);
    m_pLevelInfoViewReplayButton = new UIButton(m_pLevelInfoFrame,22,40, GAMETEXT_VIEWTHEHIGHSCORE,176,40);
    m_pLevelInfoViewReplayButton->setFont(drawLib->getFontSmall());
    m_pLevelInfoViewReplayButton->setContextHelp(CONTEXTHELP_VIEWTHEHIGHSCORE);

    UIStatic *pPlayerText = new UIStatic(m_pMainMenu,200,(drawLib->getDispHeight()*85)/600,"",drawLib->getDispWidth()-200-120,50);
    pPlayerText->setFont(drawLib->getFontMedium());            
    pPlayerText->setHAlign(UI_ALIGN_RIGHT);
    pPlayerText->setID("PLAYERTAG");
    
    UIButton *pChangePlayerButton = new UIButton(m_pMainMenu,drawLib->getDispWidth()-115,(drawLib->getDispHeight()*80)/600,GAMETEXT_CHANGE,115,57);
    pChangePlayerButton->setType(UI_BUTTON_TYPE_SMALL);
    pChangePlayerButton->setFont(drawLib->getFontSmall());
    pChangePlayerButton->setID("CHANGEPLAYERBUTTON");
    pChangePlayerButton->setContextHelp(CONTEXTHELP_CHANGE_PLAYER);
    
    UIStatic *pSomeText = new UIStatic(m_pMainMenu,0,drawLib->getDispHeight()-20,
                                        std::string("X-Moto/") + XMBuild::getVersionString(true),
                                        drawLib->getDispWidth(),20);
    pSomeText->setFont(drawLib->getFontSmall());
    pSomeText->setVAlign(UI_ALIGN_BOTTOM);
    pSomeText->setHAlign(UI_ALIGN_LEFT);
    
    for(int i=0;i<m_nNumMainMenuButtons;i++) {
      m_pMainMenuButtons[i]->setPosition(20,drawLib->	getDispHeight()/2 - (m_nNumMainMenuButtons*57)/2 + i*57,177,57);
      m_pMainMenuButtons[i]->setFont(drawLib->getFontSmall());
    }           

    m_pMainMenu->setPrimaryChild(m_pMainMenuButtons[0]); /* default button: Play */
    
    //m_pGameInfoWindow = new UIFrame(m_pMainMenu,47,20+getDispHeight()/2 + (m_nNumMainMenuButtons*57)/2,
    //                                "",207,getDispHeight() - (20+getDispHeight()/2 + (m_nNumMainMenuButtons*57)/2));
    //m_pGameInfoWindow->showWindow(true);

    m_pReplaysWindow = new UIFrame(m_pMainMenu,220,(drawLib->getDispHeight()*140)/600,"",drawLib->getDispWidth()-220-20,drawLib->getDispHeight()-40-(drawLib->getDispHeight()*120)/600-10);      
    m_pReplaysWindow->showWindow(false);
    pSomeText = new UIStatic(m_pReplaysWindow,0,0,GAMETEXT_REPLAYS,m_pReplaysWindow->getPosition().nWidth,36);
    pSomeText->setFont(drawLib->getFontMedium());

    pSomeText = new UIStatic(m_pReplaysWindow, 10, 35, std::string(GAMETEXT_FILTER) + ":", 90, 25);
    pSomeText->setFont(drawLib->getFontSmall());
    pSomeText->setHAlign(UI_ALIGN_RIGHT);
    UIEdit *pLevelFilterEdit = new UIEdit(m_pReplaysWindow,
					  110,
					  35,
					  "",200,25);
    pLevelFilterEdit->setFont(drawLib->getFontSmall());
    pLevelFilterEdit->setID("REPLAYS_FILTER");
    pLevelFilterEdit->setContextHelp(CONTEXTHELP_REPLAYS_FILTER);

    /* show button */
    UIButton *pShowButton = new UIButton(m_pReplaysWindow,5,m_pReplaysWindow->getPosition().nHeight-68,GAMETEXT_SHOW,105,57);
    pShowButton->setFont(drawLib->getFontSmall());
    pShowButton->setType(UI_BUTTON_TYPE_SMALL);
    pShowButton->setID("REPLAY_SHOW_BUTTON");
    pShowButton->setContextHelp(CONTEXTHELP_RUN_REPLAY);
    /* delete button */
    UIButton *pDeleteButton = new UIButton(m_pReplaysWindow,105,m_pReplaysWindow->getPosition().nHeight-68,GAMETEXT_DELETE,105,57);
    pDeleteButton->setFont(drawLib->getFontSmall());
    pDeleteButton->setType(UI_BUTTON_TYPE_SMALL);
    pDeleteButton->setID("REPLAY_DELETE_BUTTON");
    pDeleteButton->setContextHelp(CONTEXTHELP_DELETE_REPLAY);

    /* upload button */
    UIButton *pUploadHighscoreButton = new UIButton(m_pReplaysWindow,199,m_pReplaysWindow->getPosition().nHeight-68,GAMETEXT_UPLOAD_HIGHSCORE,186,57);
    pUploadHighscoreButton->setFont(drawLib->getFontSmall());
    pUploadHighscoreButton->setType(UI_BUTTON_TYPE_SMALL);
    pUploadHighscoreButton->setID("REPLAY_UPLOADHIGHSCORE_BUTTON");
    pUploadHighscoreButton->enableWindow(false);
    pUploadHighscoreButton->setContextHelp(CONTEXTHELP_UPLOAD_HIGHSCORE);

    /* filter */
    UIButton *pListAllButton = new UIButton(m_pReplaysWindow,m_pReplaysWindow->getPosition().nWidth-105,m_pReplaysWindow->getPosition().nHeight-68,GAMETEXT_LISTALL,115,57);
    pListAllButton->setFont(drawLib->getFontSmall());
    pListAllButton->setType(UI_BUTTON_TYPE_CHECK);
    pListAllButton->setChecked(false);
    pListAllButton->setID("REPLAY_LIST_ALL");
    pListAllButton->setContextHelp(CONTEXTHELP_ALL_REPLAYS);
    /* */
    UIList *pReplayList = new UIList(m_pReplaysWindow,20,65,"",m_pReplaysWindow->getPosition().nWidth-40,m_pReplaysWindow->getPosition().nHeight-115-25);
    pReplayList->setID("REPLAY_LIST");
    pReplayList->showWindow(true);
    pReplayList->setFont(drawLib->getFontSmall());
    pReplayList->addColumn(GAMETEXT_REPLAY, pReplayList->getPosition().nWidth/2 - 100,CONTEXTHELP_REPLAYCOL);
    pReplayList->addColumn(GAMETEXT_LEVEL,  pReplayList->getPosition().nWidth/2 - 28,CONTEXTHELP_REPLAYLEVELCOL);
    pReplayList->addColumn(GAMETEXT_PLAYER,128,CONTEXTHELP_REPLAYPLAYERCOL);
    pReplayList->setEnterButton( pShowButton );
    
    m_pLevelPacksWindow = new UIFrame(m_pMainMenu,220,(drawLib->getDispHeight()*140)/600,"",drawLib->getDispWidth()-220-20,drawLib->getDispHeight()-40-(drawLib->getDispHeight()*120)/600-10);      
    m_pLevelPacksWindow->showWindow(false);
    pSomeText = new UIStatic(m_pLevelPacksWindow,0,0,GAMETEXT_LEVELS,m_pLevelPacksWindow->getPosition().nWidth,36);
    pSomeText->setFont(drawLib->getFontMedium());

    /* tabs of the packs */
    m_pLevelPackTabs = new UITabView(m_pLevelPacksWindow,20,40,"",m_pLevelPacksWindow->getPosition().nWidth-40,m_pLevelPacksWindow->getPosition().nHeight-60);
    m_pLevelPackTabs->setFont(drawLib->getFontSmall());
    m_pLevelPackTabs->setID("LEVELPACK_TABS");
    m_pLevelPackTabs->enableWindow(true);
    m_pLevelPackTabs->showWindow(true);
    m_pLevelPackTabs->setTabContextHelp(0,CONTEXTHELP_LEVEL_PACKS);
    m_pLevelPackTabs->setTabContextHelp(1,CONTEXTHELP_BUILT_IN_AND_EXTERNALS);
    m_pLevelPackTabs->setTabContextHelp(2,CONTEXTHELP_NEW_LEVELS);

    /* pack tab */
    UIWindow *pPackTab = new UIWindow(m_pLevelPackTabs,10,40,GAMETEXT_LEVELPACKS,m_pLevelPackTabs->getPosition().nWidth-20,m_pLevelPackTabs->getPosition().nHeight);
    pPackTab->enableWindow(true);
    pPackTab->showWindow(true);
    pPackTab->setID("PACK_TAB");

    /* open button */
    UIButton *pOpenButton = new UIButton(pPackTab,11,pPackTab->getPosition().nHeight-57-45,GAMETEXT_OPEN,115,57);
    pOpenButton->setFont(drawLib->getFontSmall());
    pOpenButton->setType(UI_BUTTON_TYPE_SMALL);
    pOpenButton->setID("LEVELPACK_OPEN_BUTTON");
    pOpenButton->setContextHelp(CONTEXTHELP_VIEW_LEVEL_PACK);

    /* pack list */
    UIPackTree *pLevelPackTree = new UIPackTree(pPackTab,10,0,"",pPackTab->getPosition().nWidth-20,pPackTab->getPosition().nHeight-105);      
    pLevelPackTree->setID("LEVELPACK_TREE");
    pLevelPackTree->showWindow(true);
    pLevelPackTree->enableWindow(true);
    pLevelPackTree->setFont(drawLib->getFontSmall());
    pLevelPackTree->setEnterButton( pOpenButton );

    /* favorite levels tab */
    UIWindow *pAllLevelsPackTab = new UIWindow(m_pLevelPackTabs,20,40,VPACKAGENAME_FAVORITE_LEVELS,m_pLevelPackTabs->getPosition().nWidth-40,m_pLevelPackTabs->getPosition().nHeight);
    pAllLevelsPackTab->enableWindow(true);
    pAllLevelsPackTab->showWindow(false);
    pAllLevelsPackTab->setID("ALLLEVELS_TAB");

    /* all levels button */
    UIButton *pGoButton = new UIButton(pAllLevelsPackTab,0,pAllLevelsPackTab->getPosition().nHeight-103,GAMETEXT_STARTLEVEL,105,57);
    pGoButton->setContextHelp(CONTEXTHELP_PLAY_SELECTED_LEVEL);
    pGoButton->setFont(drawLib->getFontSmall());
    pGoButton->setType(UI_BUTTON_TYPE_SMALL);
    pGoButton->setID("PLAY_GO_BUTTON");
    UIButton *pLevelInfoButton = new UIButton(pAllLevelsPackTab,105,pAllLevelsPackTab->getPosition().nHeight-103,GAMETEXT_SHOWINFO,105,57);
    pLevelInfoButton->setFont(drawLib->getFontSmall());
    pLevelInfoButton->setType(UI_BUTTON_TYPE_SMALL);
    pLevelInfoButton->setID("PLAY_LEVEL_INFO_BUTTON");
    pLevelInfoButton->setContextHelp(CONTEXTHELP_LEVEL_INFO);

    UIButton *pDeleteFromFavoriteButton = new UIButton(pAllLevelsPackTab,pAllLevelsPackTab->getPosition().nWidth-187,pAllLevelsPackTab->getPosition().nHeight-103,GAMETEXT_DELETEFROMFAVORITE,187,57);
    pDeleteFromFavoriteButton->setFont(drawLib->getFontSmall());
    pDeleteFromFavoriteButton->setType(UI_BUTTON_TYPE_LARGE);
    pDeleteFromFavoriteButton->setID("ALL_LEVELS_DELETE_FROM_FAVORITE_BUTTON");
    pDeleteFromFavoriteButton->setContextHelp(CONTEXTHELP_DELETEFROMFAVORITE);

    /* all levels list */
    m_pAllLevelsList = new UILevelList(pAllLevelsPackTab,0,0,"",pAllLevelsPackTab->getPosition().nWidth,pAllLevelsPackTab->getPosition().nHeight-105);     
    m_pAllLevelsList->setID("ALLLEVELS_LIST");
    m_pAllLevelsList->setFont(drawLib->getFontSmall());
    m_pAllLevelsList->setSort(true);
    m_pAllLevelsList->setEnterButton( pGoButton );

    /* new levels tab */
    UIWindow *pNewLevelsPackTab = new UIWindow(m_pLevelPackTabs,20,40,GAMETEXT_NEWLEVELS,m_pLevelPackTabs->getPosition().nWidth-40,m_pLevelPackTabs->getPosition().nHeight);
    pNewLevelsPackTab->enableWindow(true);
    pNewLevelsPackTab->showWindow(false);
    pNewLevelsPackTab->setID("NEWLEVELS_TAB");

    /* new levels tab buttons */
    UIButton *pNewLevelsGoButton = new UIButton(pNewLevelsPackTab,0,pNewLevelsPackTab->getPosition().nHeight-103,GAMETEXT_STARTLEVEL,105,57);
    pNewLevelsGoButton->setContextHelp(CONTEXTHELP_PLAY_SELECTED_LEVEL);
    pNewLevelsGoButton->setFont(drawLib->getFontSmall());
    pNewLevelsGoButton->setType(UI_BUTTON_TYPE_SMALL);
    pNewLevelsGoButton->setID("NEW_LEVELS_PLAY_GO_BUTTON");
    UIButton *pNewLevelsLevelInfoButton = new UIButton(pNewLevelsPackTab,105,pNewLevelsPackTab->getPosition().nHeight-103,GAMETEXT_SHOWINFO,105,57);
    pNewLevelsLevelInfoButton->setFont(drawLib->getFontSmall());
    pNewLevelsLevelInfoButton->setType(UI_BUTTON_TYPE_SMALL);
    pNewLevelsLevelInfoButton->setID("NEW_LEVELS_PLAY_LEVEL_INFO_BUTTON");
    pNewLevelsLevelInfoButton->setContextHelp(CONTEXTHELP_LEVEL_INFO);

    UIButton *pDownloadLevelsButton = new UIButton(pNewLevelsPackTab,pNewLevelsPackTab->getPosition().nWidth-187,pNewLevelsPackTab->getPosition().nHeight-103,GAMETEXT_DOWNLOADLEVELS,187,57);
    pDownloadLevelsButton->setFont(drawLib->getFontSmall());
    pDownloadLevelsButton->setType(UI_BUTTON_TYPE_LARGE);
    pDownloadLevelsButton->setID("NEW_LEVELS_PLAY_DOWNLOAD_LEVELS_BUTTON");
    pDownloadLevelsButton->setContextHelp(CONTEXTHELP_DOWNLOADLEVELS);

    /* all levels list */
    m_pPlayNewLevelsList = new UILevelList(pNewLevelsPackTab,0,0,"",pNewLevelsPackTab->getPosition().nWidth,pNewLevelsPackTab->getPosition().nHeight-105);     
    m_pPlayNewLevelsList->setID("NEWLEVELS_LIST");
    m_pPlayNewLevelsList->setFont(drawLib->getFontSmall());
    m_pPlayNewLevelsList->setSort(true);
    m_pPlayNewLevelsList->setEnterButton( pNewLevelsGoButton );

    // multi tab
    UIWindow *pMultiOptionsTab = new UIWindow(m_pLevelPackTabs, 20, 40, GAMETEXT_MULTI,
					      m_pLevelPackTabs->getPosition().nWidth-40,
					      m_pLevelPackTabs->getPosition().nHeight);
    pMultiOptionsTab->enableWindow(true);
    pMultiOptionsTab->showWindow(false);
    pMultiOptionsTab->setID("MULTI_TAB");

    pSomeText = new UIStatic(pMultiOptionsTab, 10, 0,
			     GAMETEXT_NB_PLAYERS, pMultiOptionsTab->getPosition().nWidth, 40);
    pSomeText->setFont(drawLib->getFontMedium());
    pSomeText->setHAlign(UI_ALIGN_LEFT);

    for(unsigned int i=0; i<4; i++) {
      UIButton *pNbPlayers;
      std::ostringstream s_nbPlayers;
      char strPlayer[64];
      snprintf(strPlayer, 64, GAMETEXT_NPLAYER(i+1), i+1);
      s_nbPlayers << (int) i+1;

      pNbPlayers = new UIButton(pMultiOptionsTab, 0, 40+(i*20), strPlayer, pMultiOptionsTab->getPosition().nWidth, 28);
      pNbPlayers->setType(UI_BUTTON_TYPE_RADIO);
      pNbPlayers->setID("MULTINB_" + s_nbPlayers.str());
      pNbPlayers->enableWindow(true);
      pNbPlayers->setFont(drawLib->getFontSmall());
      pNbPlayers->setGroup(10200);
      pNbPlayers->setContextHelp(CONTEXTHELP_MULTI);

      // always check the 1 player mode
      if(i == 0) {
	pNbPlayers->setChecked(true);
      }
    }

    UIButton *pMultiStopWhenOneFinishes = new UIButton(pMultiOptionsTab, 0, pMultiOptionsTab->getPosition().nHeight - 40 - 28 - 10,
						       GAMETEXT_MULTISTOPWHENONEFINISHES,
						       pMultiOptionsTab->getPosition().nWidth,28);
    pMultiStopWhenOneFinishes->setType(UI_BUTTON_TYPE_CHECK);
    pMultiStopWhenOneFinishes->setID("ENABLEMULTISTOPWHENONEFINISHES");
    pMultiStopWhenOneFinishes->enableWindow(true);
    pMultiStopWhenOneFinishes->setFont(drawLib->getFontSmall());
    pMultiStopWhenOneFinishes->setGroup(50050);
    pMultiStopWhenOneFinishes->setContextHelp(CONTEXTHELP_MULTISTOPWHENONEFINISHES);    
    pMultiStopWhenOneFinishes->setChecked(m_xmsession->MultiStopWhenOneFinishes());
  }

  /*===========================================================================
  Create menus and hide them
  ===========================================================================*/
  void GameApp::_InitMenus(void) {
    /* TODO: it would obviously be a good idea to put this gui stuff into
       a nice XML file instead. This really stinks */
    
    _InitMenus_MainMenu();

    /* Hide menus */
    m_pMainMenu->showWindow(false);
  }

  /*===========================================================================
  Update main menu
  ===========================================================================*/
  void GameApp::_HandleMainMenu(void) {
    UIEdit *pReplayFilterEdit = reinterpret_cast<UIEdit *>(m_pReplaysWindow->getChild("REPLAYS_FILTER"));
    UIList *pReplayList = reinterpret_cast<UIList *>(m_pReplaysWindow->getChild("REPLAY_LIST"));

    /* view highscore button clicked */
    if(m_pLevelInfoViewReplayButton->isClicked()) {
      m_pLevelInfoViewReplayButton->setClicked(false);
      viewHighscoreOf();
      m_pMainMenu->showWindow(false);
      //m_stateManager->pushState(new StateReplaying(this, m_PlaySpecificReplay));
    }

    /* level menu : */
    /* any list clicked ? */
    if(m_pAllLevelsList->isChanged()) {
      std::string v_id_level = m_pAllLevelsList->getSelectedLevel();
      if(v_id_level != "") {
	setLevelInfoFrameBestPlayer(v_id_level,
				    m_pLevelInfoFrame,
				    m_pLevelInfoViewReplayButton,
				    m_pBestPlayerText
				    );
      }
      m_pAllLevelsList->setChanged(false);
    }

    if(m_pPlayNewLevelsList->isChanged()) {
      std::string v_id_level = m_pPlayNewLevelsList->getSelectedLevel();
      if(v_id_level != "") {
      	setLevelInfoFrameBestPlayer(v_id_level,
      				    m_pLevelInfoFrame,
      				    m_pLevelInfoViewReplayButton,
      				    m_pBestPlayerText
      				    );
      }
      m_pPlayNewLevelsList->setChanged(false);
    }

    /* tab of level clicked ? */
    if(m_pLevelPackTabs->isChanged()) {
      m_pLevelInfoFrame->showWindow(false);      
      m_pLevelPackTabs->setChanged(false);
    }
    
//    /* OPTIONS */
//    UIButton *pINetConf = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:PROXYCONFIG");
//    UIButton *pUpdHS = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:UPDATEHIGHSCORES");
//    UIButton *pWebHighscores = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLEWEBHIGHSCORES");
//    UIButton *pInGameWorldRecord = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:INGAMEWORLDRECORD");
//
//    UIWindow *pRoomsTab = (UIWindow *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB");
//    UIButton *pUpdRoomsList = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB:UPDATE_ROOMS_LIST");
//	UIButton *pUploadAllHighscoresButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB:REPLAY_UPLOADHIGHSCOREALL_BUTTON");
//			
//    UIButton *pUpdThemeList = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:UPDATE_THEMES_LIST");
//    UIButton *pUpdSelectedTheme = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:GET_SELECTED_THEME");
//    UIList *pThemeList = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:THEMES_LIST");
//
//	/* unfortunately because of differences betw->setClicked(false);een finishTime in webhighscores and replays table (one is rounded to 0.01s and other to 0.001s) and lack of math functions in sqlite we cannot make it with just one smart query :( */
//	if(pUploadAllHighscoresButton->isClicked()) {
//		pUploadAllHighscoresButton->setClicked(false);
//		_UploadAllHighscores();
//	}		
//
//    
//    if(pUpdHS->isClicked()) {
//      pUpdHS->setClicked(false);
//      try {
//	_UpdateWebHighscores(false);
//	_UpgradeWebHighscores();    
//	_UpdateWebLevels(false);  
//
//	m_levelsManager.makePacks(m_db,
//				  m_xmsession->profile(),
//				  m_Config.getString("WebHighscoresIdRoom"),
//				  m_xmsession->debug());
//	_UpdateLevelsLists();
//      } catch(Exception &e) {
//	notifyMsg(GAMETEXT_FAILEDDLHIGHSCORES + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
//      }
//    }
    
  }
  
  void GameApp::_SimpleMessage(const std::string &Msg,UIRect *pRect,bool bNoSwap) {      

  }

  void GameApp::setLevelInfoFrameBestPlayer(std::string pLevelID,
					    UIWindow *i_pLevelInfoFrame,
					    UIButton *i_pLevelInfoViewReplayButton,
					    UIStatic *i_pBestPlayerText
					    ) {
    char **v_result;
    unsigned int nrow;
    std::string v_levelAuthor;
    std::string v_fileUrl;
    
    v_result = m_db->readDB("SELECT id_profile, fileUrl "
			    "FROM webhighscores WHERE id_level=\"" + 
			    xmDatabase::protectString(pLevelID) + "\" "
			    "AND id_room=" + m_xmsession->idRoom() + ";",
			    nrow);
    if(nrow == 0) {
      i_pLevelInfoFrame->showWindow(false);
      m_pLevelToShowOnViewHighscore = "";
      m_db->read_DB_free(v_result);
      return;
    }
    v_levelAuthor = m_db->getResult(v_result, 2, 0, 0);
    v_fileUrl     = m_db->getResult(v_result, 2, 0, 1);
    m_db->read_DB_free(v_result);

    i_pLevelInfoFrame->showWindow(true);
    i_pBestPlayerText->setCaption((std::string(GAMETEXT_BESTPLAYER) + " : " +
				   v_levelAuthor).c_str());
    m_pLevelToShowOnViewHighscore = pLevelID;
    
    /* search if the replay is already downloaded */
    if(m_db->replays_exists(FS::getFileBaseName(v_fileUrl))) {
      i_pLevelInfoViewReplayButton->enableWindow(true);
    } else {
      i_pLevelInfoViewReplayButton->enableWindow(m_xmsession->www());
    }
  }

  void GameApp::viewHighscoreOf() {
    char **v_result;
    unsigned int nrow;
    std::string v_levelAuthor;
    std::string v_fileUrl;
    std::string v_replayName;

    m_PlaySpecificReplay = "";

    v_result = m_db->readDB("SELECT id_profile, fileUrl "
			    "FROM webhighscores WHERE id_level=\"" + 
			    xmDatabase::protectString(m_pLevelToShowOnViewHighscore) + "\" "
			    "AND id_room=" + m_xmsession->idRoom() + ";",
			    nrow);
    if(nrow == 0) {
      m_db->read_DB_free(v_result);
      return;
    }

    v_levelAuthor = m_db->getResult(v_result, 2, 0, 0);
    v_fileUrl     = m_db->getResult(v_result, 2, 0, 1);
    v_replayName  = FS::getFileBaseName(v_fileUrl);
    m_db->read_DB_free(v_result);

    if(m_db->replays_exists(v_replayName)) {
      m_PlaySpecificReplay = v_replayName;
      return;
    }
    
    if(m_xmsession->www() == false) {
      return;
    }

    try {
      _SimpleMessage(GAMETEXT_DLHIGHSCORE,&m_InfoMsgBoxRect);
      m_pWebHighscores->downloadReplay(v_fileUrl);
      addReplay(v_replayName);
      getStateManager()->sendAsynchronousMessage("REPLAYS_UPDATED");
      
      /* not very nice : make a new search to be sure the replay is here */
      /* because it could have been downloaded but unplayable : for macosx for example */
      if(m_db->replays_exists(v_replayName) == false) {
	notifyMsg(GAMETEXT_FAILEDTOLOADREPLAY);
	return;
      }
    } catch(Exception &e) {
      notifyMsg(GAMETEXT_FAILEDDLREPLAY + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
      return;
    }
    m_PlaySpecificReplay = v_replayName;
  }
