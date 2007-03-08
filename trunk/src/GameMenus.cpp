/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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
#include "Game.h"
#include "VFileIO.h"

namespace vapp {

  void GameApp::_InitMenus_PlayingMenus(void) {
    int i;
    int default_button;

    /* Best times windows (at finish) */
    m_pBestTimes = new UIBestTimes(m_Renderer.getGUI(),10,50,"",290,500);
    m_pBestTimes->setFont(m_Renderer.getMediumFont());
    m_pBestTimes->setHFont(m_Renderer.getSmallFont());

    /* Initialize finish menu */
    m_pFinishMenu = new UIFrame(m_Renderer.getGUI(),300,30,"",400,540);
    m_pFinishMenu->setStyle(UI_FRAMESTYLE_MENU);

    i=0;

    m_pFinishMenuButtons[i] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_TRYAGAIN,207,57);
    m_pFinishMenuButtons[i]->setContextHelp(CONTEXTHELP_PLAY_THIS_LEVEL_AGAIN);
    i++;

    m_pFinishMenuButtons[i] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_PLAYNEXT,207,57);
    m_pFinishMenuButtons[i]->setContextHelp(CONTEXTHELP_PLAY_NEXT_LEVEL);
    default_button = i;
    i++;

    m_pFinishMenuButtons[i] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_SAVEREPLAY,207,57);
    m_pFinishMenuButtons[i]->setContextHelp(CONTEXTHELP_SAVE_A_REPLAY);
    if(!m_bRecordReplays) m_pFinishMenuButtons[1]->enableWindow(false);
    i++;

#if defined(SUPPORT_WEBACCESS)
    m_pFinishMenuButtons[i] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_UPLOAD_HIGHSCORE,207,57);
    m_pFinishMenuButtons[i]->setContextHelp(CONTEXTHELP_UPLOAD_HIGHSCORE);
    m_pFinishMenuButtons[i]->enableWindow(false);
    i++;
#endif
   
    m_pFinishMenuButtons[i] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_ABORT,207,57);
    m_pFinishMenuButtons[i]->setContextHelp(CONTEXTHELP_BACK_TO_MAIN_MENU);
    i++;
    
    m_pFinishMenuButtons[i] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_QUIT,207,57);
    m_pFinishMenuButtons[i]->setContextHelp(CONTEXTHELP_QUIT_THE_GAME);
    i++;

    m_nNumFinishMenuButtons = i;

    UIStatic *pFinishText = new UIStatic(m_pFinishMenu,0,100,GAMETEXT_FINISH,m_pFinishMenu->getPosition().nWidth,36);
    pFinishText->setFont(m_Renderer.getMediumFont());
   
    for(int i=0;i<m_nNumFinishMenuButtons;i++) {
#if defined(SUPPORT_WEBACCESS)
      m_pFinishMenuButtons[i]->setPosition(200-207/2,m_pFinishMenu->getPosition().nHeight/2 - (m_nNumFinishMenuButtons*57)/2 + i*49 + 25,207,57);
#else
      m_pFinishMenuButtons[i]->setPosition(200-207/2,m_pFinishMenu->getPosition().nHeight/2 - (m_nNumFinishMenuButtons*57)/2 + i*57,207,57);
#endif
      m_pFinishMenuButtons[i]->setFont(m_Renderer.getSmallFont());
    }

    m_pFinishMenu->setPrimaryChild(m_pFinishMenuButtons[default_button]); /* default button: Play next */
                      
    /* Initialize pause menu */
    m_pPauseMenu = new UIFrame(m_Renderer.getGUI(),drawLib->getDispWidth()/2 - 200,70,"",400,540);
    m_pPauseMenu->setStyle(UI_FRAMESTYLE_MENU);
    
    m_pPauseMenuButtons[0] = new UIButton(m_pPauseMenu,0,0,GAMETEXT_RESUME,207,57);
    m_pPauseMenuButtons[0]->setContextHelp(CONTEXTHELP_BACK_TO_GAME);
    
    m_pPauseMenuButtons[1] = new UIButton(m_pPauseMenu,0,0,GAMETEXT_RESTART,207,57);
    m_pPauseMenuButtons[1]->setContextHelp(CONTEXTHELP_TRY_LEVEL_AGAIN_FROM_BEGINNING);
    
    m_pPauseMenuButtons[2] = new UIButton(m_pPauseMenu,0,0,GAMETEXT_PLAYNEXT,207,57);
    m_pPauseMenuButtons[2]->setContextHelp(CONTEXTHELP_PLAY_NEXT_INSTEAD);
    
    m_pPauseMenuButtons[3] = new UIButton(m_pPauseMenu,0,0,GAMETEXT_ABORT,207,57);
    m_pPauseMenuButtons[3]->setContextHelp(CONTEXTHELP_BACK_TO_MAIN_MENU);
    
    m_pPauseMenuButtons[4] = new UIButton(m_pPauseMenu,0,0,GAMETEXT_QUIT,207,57);
    m_pPauseMenuButtons[4]->setContextHelp(CONTEXTHELP_QUIT_THE_GAME);    
    m_nNumPauseMenuButtons = 5;

    UIStatic *pPauseText = new UIStatic(m_pPauseMenu,0,100,GAMETEXT_PAUSE,m_pPauseMenu->getPosition().nWidth,36);
    pPauseText->setFont(m_Renderer.getMediumFont());
    
    for(int i=0;i<m_nNumPauseMenuButtons;i++) {
      m_pPauseMenuButtons[i]->setPosition(200 -207/2,10+m_pPauseMenu->getPosition().nHeight/2 - (m_nNumPauseMenuButtons*57)/2 + i*57,207,57);
      m_pPauseMenuButtons[i]->setFont(m_Renderer.getSmallFont());
    }
    
    m_pPauseMenu->setPrimaryChild(m_pPauseMenuButtons[0]); /* default button: Resume */
    
    /* Initialize just-dead menu */
    m_pJustDeadMenu = new UIFrame(m_Renderer.getGUI(),drawLib->getDispWidth()/2 - 200,70,"",400,540);
    m_pJustDeadMenu->setStyle(UI_FRAMESTYLE_MENU);
    
    m_pJustDeadMenuButtons[0] = new UIButton(m_pJustDeadMenu,0,0,GAMETEXT_TRYAGAIN,207,57);
    m_pJustDeadMenuButtons[0]->setContextHelp(CONTEXTHELP_TRY_LEVEL_AGAIN);
    
    m_pJustDeadMenuButtons[1] = new UIButton(m_pJustDeadMenu,0,0,GAMETEXT_SAVEREPLAY,207,57);
    m_pJustDeadMenuButtons[1]->setContextHelp(CONTEXTHELP_SAVE_A_REPLAY);    
    if(!m_bRecordReplays) m_pJustDeadMenuButtons[1]->enableWindow(false);
    
    m_pJustDeadMenuButtons[2] = new UIButton(m_pJustDeadMenu,0,0,GAMETEXT_PLAYNEXT,207,57);
    m_pJustDeadMenuButtons[2]->setContextHelp(CONTEXTHELP_PLAY_NEXT_LEVEL);
    
    m_pJustDeadMenuButtons[3] = new UIButton(m_pJustDeadMenu,0,0,GAMETEXT_ABORT,207,57);
    m_pJustDeadMenuButtons[3]->setContextHelp(CONTEXTHELP_BACK_TO_MAIN_MENU);
    
    m_pJustDeadMenuButtons[4] = new UIButton(m_pJustDeadMenu,0,0,GAMETEXT_QUIT,207,57);
    m_pJustDeadMenuButtons[4]->setContextHelp(CONTEXTHELP_QUIT_THE_GAME);
    m_nNumJustDeadMenuButtons = 5;

    UIStatic *pJustDeadText = new UIStatic(m_pJustDeadMenu,0,100,GAMETEXT_JUSTDEAD,m_pJustDeadMenu->getPosition().nWidth,36);
    pJustDeadText->setFont(m_Renderer.getMediumFont());
    
    for(int i=0;i<m_nNumJustDeadMenuButtons;i++) {
      m_pJustDeadMenuButtons[i]->setPosition( 200 -207/2,10+m_pJustDeadMenu->getPosition().nHeight/2 - (m_nNumJustDeadMenuButtons*57)/2 + i*57,207,57);
      m_pJustDeadMenuButtons[i]->setFont(m_Renderer.getSmallFont());
    }

    m_pJustDeadMenu->setPrimaryChild(m_pJustDeadMenuButtons[0]); /* default button: Try Again */

  }

  void GameApp::_InitMenus_MainMenu(void) {
    /* Initialize main menu */      
    m_pMainMenu = new UIWindow(m_Renderer.getGUI(),0,0,"",
                                m_Renderer.getGUI()->getPosition().nWidth,
                                m_Renderer.getGUI()->getPosition().nHeight);
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
    
#if defined(SUPPORT_WEBACCESS)        
    /* level info frame */
    m_pLevelInfoFrame = new UIWindow(m_pMainMenu,0,drawLib->getDispHeight()/2 - (m_nNumMainMenuButtons*57)/2 + m_nNumMainMenuButtons*57,"",220,100);
    m_pLevelInfoFrame->showWindow(false);
    m_pBestPlayerText = new UIStatic(m_pLevelInfoFrame, 0, 5,"", 220, 50);
    m_pBestPlayerText->setFont(m_Renderer.getSmallFont());
    m_pBestPlayerText->setHAlign(UI_ALIGN_CENTER);
    m_pBestPlayerText->showWindow(true);
    m_pLevelInfoViewReplayButton = new UIButton(m_pLevelInfoFrame,22,40, GAMETEXT_VIEWTHEHIGHSCORE,176,40);
    m_pLevelInfoViewReplayButton->setFont(m_Renderer.getSmallFont());
    m_pLevelInfoViewReplayButton->setContextHelp(CONTEXTHELP_VIEWTHEHIGHSCORE);
#endif

//    UIStatic *pPlayerText = new UIStatic(m_pMainMenu,300,85,"",getDispWidth()-300-120,50);
    UIStatic *pPlayerText = new UIStatic(m_pMainMenu,300,(drawLib->getDispHeight()*85)/600,"",drawLib->getDispWidth()-300-120,50);
    pPlayerText->setFont(m_Renderer.getMediumFont());            
    pPlayerText->setHAlign(UI_ALIGN_RIGHT);
    pPlayerText->setID("PLAYERTAG");
    if(m_pPlayer != NULL) pPlayerText->setCaption(std::string(GAMETEXT_CURPLAYER) + ": " + m_pPlayer->PlayerName);
    
    /* new levels ? */
    UIStatic *pNewLevelText = new UIStatic(m_pMainMenu,5,-90,"",200,200);
    pNewLevelText->setFont(m_Renderer.getSmallFont());            
    pNewLevelText->setHAlign(UI_ALIGN_LEFT);
    pNewLevelText->setID("NEWLEVELAVAILBLE");
    
//    UIButton *pChangePlayerButton = new UIButton(m_pMainMenu,getDispWidth()-115,80,GAMETEXT_CHANGE,115,57);
    UIButton *pChangePlayerButton = new UIButton(m_pMainMenu,drawLib->getDispWidth()-115,(drawLib->getDispHeight()*80)/600,GAMETEXT_CHANGE,115,57);
    pChangePlayerButton->setType(UI_BUTTON_TYPE_SMALL);
    pChangePlayerButton->setFont(m_Renderer.getSmallFont());
    pChangePlayerButton->setID("CHANGEPLAYERBUTTON");
    pChangePlayerButton->setContextHelp(CONTEXTHELP_CHANGE_PLAYER);
    
    UIStatic *pSomeText = new UIStatic(m_pMainMenu,0,drawLib->getDispHeight()-20,
                                        std::string("X-Moto/") + getVersionString(),
                                        drawLib->getDispWidth(),20);
    pSomeText->setFont(m_Renderer.getSmallFont());
    pSomeText->setVAlign(UI_ALIGN_BOTTOM);
    pSomeText->setHAlign(UI_ALIGN_LEFT);
    
    for(int i=0;i<m_nNumMainMenuButtons;i++) {
      m_pMainMenuButtons[i]->setPosition(20,drawLib->	getDispHeight()/2 - (m_nNumMainMenuButtons*57)/2 + i*57,177,57);
      m_pMainMenuButtons[i]->setFont(m_Renderer.getSmallFont());
    }           

    m_pMainMenu->setPrimaryChild(m_pMainMenuButtons[0]); /* default button: Play */
    
    //m_pGameInfoWindow = new UIFrame(m_pMainMenu,47,20+getDispHeight()/2 + (m_nNumMainMenuButtons*57)/2,
    //                                "",207,getDispHeight() - (20+getDispHeight()/2 + (m_nNumMainMenuButtons*57)/2));
    //m_pGameInfoWindow->showWindow(true);
          
    m_pHelpWindow = new UIFrame(m_pMainMenu,220,(drawLib->getDispHeight()*140)/600,"",drawLib->getDispWidth()-220-20,drawLib->getDispHeight()-40-(drawLib->getDispHeight()*120)/600-10);
    m_pHelpWindow->showWindow(false);
    pSomeText = new UIStatic(m_pHelpWindow,0,0,GAMETEXT_HELP,m_pHelpWindow->getPosition().nWidth,36);
    pSomeText->setFont(m_Renderer.getMediumFont());
    pSomeText = new UIStatic(m_pHelpWindow,
			     10,
			     46,
			     GAMETEXT_HELPTEXT(m_Config.getString("KeyDrive1"),
					       m_Config.getString("KeyBrake1"),
					       m_Config.getString("KeyFlipLeft1"),
					       m_Config.getString("KeyFlipRight1"),
					       m_Config.getString("KeyChangeDir1")					       
					       )
			     ,m_pHelpWindow->getPosition().nWidth-20,
			     m_pHelpWindow->getPosition().nHeight-56);
    pSomeText->setFont(m_Renderer.getSmallFont());
    pSomeText->setVAlign(UI_ALIGN_TOP);
    pSomeText->setHAlign(UI_ALIGN_LEFT);
    UIButton *pTutorialButton = new UIButton(m_pHelpWindow,m_pHelpWindow->getPosition().nWidth-120,m_pHelpWindow->getPosition().nHeight-62,
                                             GAMETEXT_TUTORIAL,115,57);
    pTutorialButton->setContextHelp(CONTEXTHELP_TUTORIAL);
    pTutorialButton->setFont(m_Renderer.getSmallFont());
    pTutorialButton->setType(UI_BUTTON_TYPE_SMALL);
    pTutorialButton->setID("HELP_TUTORIAL_BUTTON");    
    UIButton *pCreditsButton = new UIButton(m_pHelpWindow,m_pHelpWindow->getPosition().nWidth-240,m_pHelpWindow->getPosition().nHeight-62,
                                             GAMETEXT_CREDITSBUTTON,115,57);
    pCreditsButton->setContextHelp(CONTEXTHELP_CREDITS);
    pCreditsButton->setFont(m_Renderer.getSmallFont());
    pCreditsButton->setType(UI_BUTTON_TYPE_SMALL);
    pCreditsButton->setID("HELP_CREDITS_BUTTON");

    m_pReplaysWindow = new UIFrame(m_pMainMenu,220,(drawLib->getDispHeight()*140)/600,"",drawLib->getDispWidth()-220-20,drawLib->getDispHeight()-40-(drawLib->getDispHeight()*120)/600-10);      
    m_pReplaysWindow->showWindow(false);
    pSomeText = new UIStatic(m_pReplaysWindow,0,0,GAMETEXT_REPLAYS,m_pHelpWindow->getPosition().nWidth,36);
    pSomeText->setFont(m_Renderer.getMediumFont());
    /* show button */
    UIButton *pShowButton = new UIButton(m_pReplaysWindow,5,m_pReplaysWindow->getPosition().nHeight-68,GAMETEXT_SHOW,105,57);
    pShowButton->setFont(m_Renderer.getSmallFont());
    pShowButton->setType(UI_BUTTON_TYPE_SMALL);
    pShowButton->setID("REPLAY_SHOW_BUTTON");
    pShowButton->setContextHelp(CONTEXTHELP_RUN_REPLAY);
    /* delete button */
    UIButton *pDeleteButton = new UIButton(m_pReplaysWindow,105,m_pReplaysWindow->getPosition().nHeight-68,GAMETEXT_DELETE,105,57);
    pDeleteButton->setFont(m_Renderer.getSmallFont());
    pDeleteButton->setType(UI_BUTTON_TYPE_SMALL);
    pDeleteButton->setID("REPLAY_DELETE_BUTTON");
    pDeleteButton->setContextHelp(CONTEXTHELP_DELETE_REPLAY);
#if defined(SUPPORT_WEBACCESS)
    /* upload button */
    UIButton *pUploadHighscoreButton = new UIButton(m_pReplaysWindow,199,m_pReplaysWindow->getPosition().nHeight-68,GAMETEXT_UPLOAD_HIGHSCORE,186,57);
    pUploadHighscoreButton->setFont(m_Renderer.getSmallFont());
    pUploadHighscoreButton->setType(UI_BUTTON_TYPE_SMALL);
    pUploadHighscoreButton->setID("REPLAY_UPLOADHIGHSCORE_BUTTON");
    pUploadHighscoreButton->enableWindow(false);
    pUploadHighscoreButton->setContextHelp(CONTEXTHELP_UPLOAD_HIGHSCORE);
#endif
    /* filter */
    UIButton *pListAllButton = new UIButton(m_pReplaysWindow,m_pReplaysWindow->getPosition().nWidth-105,m_pReplaysWindow->getPosition().nHeight-68,GAMETEXT_LISTALL,115,57);
    pListAllButton->setFont(m_Renderer.getSmallFont());
    pListAllButton->setType(UI_BUTTON_TYPE_CHECK);
    pListAllButton->setChecked(false);
    pListAllButton->setID("REPLAY_LIST_ALL");
    pListAllButton->setContextHelp(CONTEXTHELP_ALL_REPLAYS);
    /* */
    UIList *pReplayList = new UIList(m_pReplaysWindow,20,40,"",m_pReplaysWindow->getPosition().nWidth-40,m_pReplaysWindow->getPosition().nHeight-115);      
    pReplayList->setID("REPLAY_LIST");
    pReplayList->showWindow(true);
    pReplayList->setFont(m_Renderer.getSmallFont());
    pReplayList->addColumn(GAMETEXT_REPLAY, pReplayList->getPosition().nWidth/2 - 100,CONTEXTHELP_REPLAYCOL);
    pReplayList->addColumn(GAMETEXT_LEVEL,  pReplayList->getPosition().nWidth/2 - 28,CONTEXTHELP_REPLAYLEVELCOL);
    pReplayList->addColumn(GAMETEXT_PLAYER,128,CONTEXTHELP_REPLAYPLAYERCOL);
    pReplayList->setEnterButton( pShowButton );
    
    //m_pPlayWindow->setPrimaryChild(m_pJustDeadMenuButtons[0]); /* default button: Try Again */

    m_pOptionsWindow = new UIFrame(m_pMainMenu,220,(drawLib->getDispHeight()*140)/600,"",drawLib->getDispWidth()-220-20,drawLib->getDispHeight()-40-(drawLib->getDispHeight()*120)/600-10);
    m_pOptionsWindow->showWindow(false);
    pSomeText = new UIStatic(m_pOptionsWindow,0,0,GAMETEXT_OPTIONS,m_pHelpWindow->getPosition().nWidth,36);
    pSomeText->setFont(m_Renderer.getMediumFont());
    UITabView *pOptionsTabs  = new UITabView(m_pOptionsWindow,20,40,"",m_pOptionsWindow->getPosition().nWidth-40,m_pOptionsWindow->getPosition().nHeight-115);
    pOptionsTabs->setID("OPTIONS_TABS");
    pOptionsTabs->setFont(m_Renderer.getSmallFont());
    pOptionsTabs->setTabContextHelp(0,CONTEXTHELP_GENERAL_OPTIONS);
    pOptionsTabs->setTabContextHelp(1,CONTEXTHELP_VIDEO_OPTIONS);
    pOptionsTabs->setTabContextHelp(2,CONTEXTHELP_AUDIO_OPTIONS);
    pOptionsTabs->setTabContextHelp(3,CONTEXTHELP_CONTROL_OPTIONS);
    UIButton *pSaveOptionsButton = new UIButton(m_pOptionsWindow,11,m_pOptionsWindow->getPosition().nHeight-68,GAMETEXT_SAVE,115,57);
    pSaveOptionsButton->setID("SAVE_BUTTON");
    pSaveOptionsButton->setFont(m_Renderer.getSmallFont());
    pSaveOptionsButton->setType(UI_BUTTON_TYPE_SMALL);
    pSaveOptionsButton->setContextHelp(CONTEXTHELP_SAVE_OPTIONS);
    UIButton *pDefaultOptionsButton = new UIButton(m_pOptionsWindow,126,m_pOptionsWindow->getPosition().nHeight-68,GAMETEXT_DEFAULTS,115,57);
    pDefaultOptionsButton->setID("DEFAULTS_BUTTON");
    pDefaultOptionsButton->setFont(m_Renderer.getSmallFont());
    pDefaultOptionsButton->setType(UI_BUTTON_TYPE_SMALL);      
    pDefaultOptionsButton->setContextHelp(CONTEXTHELP_DEFAULTS);
    UIWindow *pGeneralOptionsTab = new UIWindow(pOptionsTabs,20,40,GAMETEXT_GENERAL,pOptionsTabs->getPosition().nWidth-40,pOptionsTabs->getPosition().nHeight);
    pGeneralOptionsTab->enableWindow(true);
    pGeneralOptionsTab->showWindow(true);
    pGeneralOptionsTab->setID("GENERAL_TAB");
    
    UIButton *pShowMiniMap = new UIButton(pGeneralOptionsTab,5,33-28-10,GAMETEXT_SHOWMINIMAP,((pGeneralOptionsTab->getPosition().nWidth-40))/2,28);
    pShowMiniMap->setType(UI_BUTTON_TYPE_CHECK);
    pShowMiniMap->setID("SHOWMINIMAP");
    pShowMiniMap->enableWindow(true);
    pShowMiniMap->setFont(m_Renderer.getSmallFont());
    pShowMiniMap->setGroup(50023);
    pShowMiniMap->setContextHelp(CONTEXTHELP_MINI_MAP);

    UIButton *pShowEngineCounter = new UIButton(pGeneralOptionsTab,5,63-28-10,GAMETEXT_SHOWENGINECOUNTER,((pGeneralOptionsTab->getPosition().nWidth-40))/2,28);
    pShowEngineCounter->setType(UI_BUTTON_TYPE_CHECK);
    pShowEngineCounter->setID("SHOWENGINECOUNTER");
    pShowEngineCounter->enableWindow(true);
    pShowEngineCounter->setFont(m_Renderer.getSmallFont());
    pShowEngineCounter->setGroup(50023);
    pShowEngineCounter->setContextHelp(CONTEXTHELP_ENGINE_COUNTER);

    UIButton *pInitZoom = new UIButton(pGeneralOptionsTab,5+((pGeneralOptionsTab->getPosition().nWidth-40))/2,33-28-10,GAMETEXT_INITZOOM,((pGeneralOptionsTab->getPosition().nWidth-40))/2,28);
    pInitZoom->setType(UI_BUTTON_TYPE_CHECK);
    pInitZoom->setID("INITZOOM");
    pInitZoom->enableWindow(true);
    pInitZoom->setFont(m_Renderer.getSmallFont());
    pInitZoom->setGroup(50023);
    pInitZoom->setContextHelp(CONTEXTHELP_INITZOOM);

    UIButton *pDeathAnim = new UIButton(pGeneralOptionsTab,5+((pGeneralOptionsTab->getPosition().nWidth-40))/2,63-28-10,GAMETEXT_DEATHANIM,((pGeneralOptionsTab->getPosition().nWidth-40))/2,28);
    pDeathAnim->setType(UI_BUTTON_TYPE_CHECK);
    pDeathAnim->setID("DEATHANIM");
    pDeathAnim->enableWindow(true);
    pDeathAnim->setFont(m_Renderer.getSmallFont());
    pDeathAnim->setGroup(50023);
    pDeathAnim->setContextHelp(CONTEXTHELP_DEATHANIM);

    UIButton *pContextHelp = new UIButton(pGeneralOptionsTab,5,93-28-10,GAMETEXT_ENABLECONTEXTHELP,(pGeneralOptionsTab->getPosition().nWidth-40),28);
    pContextHelp->setType(UI_BUTTON_TYPE_CHECK);
    pContextHelp->setID("ENABLECONTEXTHELP");
    pContextHelp->enableWindow(true);
    pContextHelp->setFont(m_Renderer.getSmallFont());
    pContextHelp->setGroup(50023);
    pContextHelp->setContextHelp(CONTEXTHELP_SHOWCONTEXTHELP);
 
    UIButton *pAutosaveReplays = new UIButton(pGeneralOptionsTab,5,123-28-10,GAMETEXT_AUTOSAVEREPLAYS,(pGeneralOptionsTab->getPosition().nWidth-40),28);
    pAutosaveReplays->setType(UI_BUTTON_TYPE_CHECK);
    pAutosaveReplays->setID("AUTOSAVEREPLAYS");
    pAutosaveReplays->enableWindow(true);
    pAutosaveReplays->setFont(m_Renderer.getSmallFont());
    pAutosaveReplays->setGroup(50023);
    pAutosaveReplays->setContextHelp(CONTEXTHELP_AUTOSAVEREPLAYS);
   
    UIList *pThemeList = new UIList(pGeneralOptionsTab,5,120,"",
				    pGeneralOptionsTab->getPosition().nWidth-10,
				    pGeneralOptionsTab->getPosition().nHeight-125-90);
    pThemeList->setID("THEMES_LIST");
    pThemeList->setFont(m_Renderer.getSmallFont());
    pThemeList->addColumn(GAMETEXT_THEMES, (pThemeList->getPosition().nWidth*3) / 5);
    pThemeList->addColumn("", (pThemeList->getPosition().nWidth*2) / 5);
    _UpdateThemesLists();
    pThemeList->setContextHelp(CONTEXTHELP_THEMES);

#if defined(SUPPORT_WEBACCESS)
    UIButton *pUpdateThemesButton = new UIButton(pGeneralOptionsTab,
						 pGeneralOptionsTab->getPosition().nWidth -200 -200,
						 pGeneralOptionsTab->getPosition().nHeight - 95,
						 GAMETEXT_UPDATETHEMESLIST,
						 207,
						 57);
    pUpdateThemesButton->setType(UI_BUTTON_TYPE_LARGE);
    pUpdateThemesButton->setID("UPDATE_THEMES_LIST");
    pUpdateThemesButton->enableWindow(true);
    pUpdateThemesButton->setFont(m_Renderer.getSmallFont());
    pUpdateThemesButton->setContextHelp(CONTEXTHELP_UPDATETHEMESLIST);

    UIButton *pGetSelectedTheme = new UIButton(pGeneralOptionsTab,
					       pGeneralOptionsTab->getPosition().nWidth -200,
					       pGeneralOptionsTab->getPosition().nHeight - 95,
					       GAMETEXT_GETSELECTEDTHEME,
					       207,
					       57);
    pGetSelectedTheme->setType(UI_BUTTON_TYPE_LARGE);
    pGetSelectedTheme->setID("GET_SELECTED_THEME");
    pGetSelectedTheme->enableWindow(true);
    pGetSelectedTheme->setFont(m_Renderer.getSmallFont());
    pGetSelectedTheme->setContextHelp(CONTEXTHELP_GETSELECTEDTHEME);
#endif

    UIWindow *pVideoOptionsTab = new UIWindow(pOptionsTabs,20,40,GAMETEXT_VIDEO,pOptionsTabs->getPosition().nWidth-40,pOptionsTabs->getPosition().nHeight);
    pVideoOptionsTab->enableWindow(true);
    pVideoOptionsTab->showWindow(false);
    pVideoOptionsTab->setID("VIDEO_TAB");
    
    UIButton *p16BitsPerPixel = new UIButton(pVideoOptionsTab,5,5,GAMETEXT_16BPP,(pVideoOptionsTab->getPosition().nWidth-40)/2,28);
    p16BitsPerPixel->setType(UI_BUTTON_TYPE_RADIO);
    p16BitsPerPixel->setID("16BPP");
    p16BitsPerPixel->enableWindow(true);
    p16BitsPerPixel->setFont(m_Renderer.getSmallFont());
    p16BitsPerPixel->setGroup(20023);
    p16BitsPerPixel->setContextHelp(CONTEXTHELP_HIGHCOLOR);

    UIButton *p32BitsPerPixel = new UIButton(pVideoOptionsTab,5 + ((pVideoOptionsTab->getPosition().nWidth-40)/2)*1,5,GAMETEXT_32BPP,(pVideoOptionsTab->getPosition().nWidth-40)/2,28);
    p32BitsPerPixel->setType(UI_BUTTON_TYPE_RADIO);
    p32BitsPerPixel->setID("32BPP");
    p32BitsPerPixel->enableWindow(true);
    p32BitsPerPixel->setFont(m_Renderer.getSmallFont());
    p32BitsPerPixel->setGroup(20023);
    p32BitsPerPixel->setContextHelp(CONTEXTHELP_TRUECOLOR);
    
    UIList *pDispResList = new UIList(pVideoOptionsTab,5,43,"",pVideoOptionsTab->getPosition().nWidth-10,128);
    pDispResList->setID("RES_LIST");
    pDispResList->setFont(m_Renderer.getSmallFont());
    pDispResList->addColumn(GAMETEXT_SCREENRES,pDispResList->getPosition().nWidth,CONTEXTHELP_SCREENRES);

    std::vector<std::string>* modes = getDisplayModes(m_Config.getBool("DisplayWindowed"));
    
    for(int i=0; i < modes->size(); i++) {
      pDispResList->addEntry((*modes)[i].c_str());
    }
    
    delete modes;

    pDispResList->setContextHelp(CONTEXTHELP_RESOLUTION);

    UIButton *pRunWindowed = new UIButton(pVideoOptionsTab,5,180,GAMETEXT_RUNWINDOWED,(pVideoOptionsTab->getPosition().nWidth-40)/1,28);
    pRunWindowed->setType(UI_BUTTON_TYPE_CHECK);
    pRunWindowed->setID("RUN_WINDOWED");
    pRunWindowed->enableWindow(true);
    pRunWindowed->setFont(m_Renderer.getSmallFont());
    pRunWindowed->setContextHelp(CONTEXTHELP_RUN_IN_WINDOW);
    
    pSomeText = new UIStatic(pVideoOptionsTab,5,208,std::string(GAMETEXT_MENUGFX) +":",120,28);
    pSomeText->setFont(m_Renderer.getSmallFont());    
    pSomeText->enableWindow(true);
    pSomeText->showWindow(true);

    UIButton *pMenuLow = new UIButton(pVideoOptionsTab,120,208,GAMETEXT_LOW,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pMenuLow->setType(UI_BUTTON_TYPE_RADIO);
    pMenuLow->setID("MENULOW");
    pMenuLow->enableWindow(true);
    pMenuLow->setFont(m_Renderer.getSmallFont());
    pMenuLow->setGroup(20024);
    pMenuLow->setContextHelp(CONTEXTHELP_LOW_MENU);

    UIButton *pMenuMed = new UIButton(pVideoOptionsTab,120+((pVideoOptionsTab->getPosition().nWidth-120)/3)*1,208,GAMETEXT_MEDIUM,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pMenuMed->setType(UI_BUTTON_TYPE_RADIO);
    pMenuMed->setID("MENUMEDIUM");
    pMenuMed->enableWindow(true);
    pMenuMed->setFont(m_Renderer.getSmallFont());
    pMenuMed->setGroup(20024);
    pMenuMed->setContextHelp(CONTEXTHELP_MEDIUM_MENU);

    UIButton *pMenuHigh = new UIButton(pVideoOptionsTab,120+((pVideoOptionsTab->getPosition().nWidth-120)/3)*2,208,GAMETEXT_HIGH,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pMenuHigh->setType(UI_BUTTON_TYPE_RADIO);
    pMenuHigh->setID("MENUHIGH");
    pMenuHigh->enableWindow(true);
    pMenuHigh->setFont(m_Renderer.getSmallFont());
    pMenuHigh->setGroup(20024);
    pMenuHigh->setContextHelp(CONTEXTHELP_HIGH_MENU);

    pSomeText = new UIStatic(pVideoOptionsTab,5,236,std::string(GAMETEXT_GAMEGFX) + ":",120,28);
    pSomeText->setFont(m_Renderer.getSmallFont());    
    pSomeText->enableWindow(true);
    pSomeText->showWindow(true);

    UIButton *pGameLow = new UIButton(pVideoOptionsTab,120,236,GAMETEXT_LOW,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pGameLow->setType(UI_BUTTON_TYPE_RADIO);
    pGameLow->setID("GAMELOW");
    pGameLow->enableWindow(true);
    pGameLow->setFont(m_Renderer.getSmallFont());
    pGameLow->setGroup(20025);
    pGameLow->setContextHelp(CONTEXTHELP_LOW_GAME);

    UIButton *pGameMed = new UIButton(pVideoOptionsTab,120+((pVideoOptionsTab->getPosition().nWidth-120)/3)*1,236,GAMETEXT_MEDIUM,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pGameMed->setType(UI_BUTTON_TYPE_RADIO);
    pGameMed->setID("GAMEMEDIUM");
    pGameMed->enableWindow(true);
    pGameMed->setFont(m_Renderer.getSmallFont());
    pGameMed->setGroup(20025);
    pGameMed->setContextHelp(CONTEXTHELP_MEDIUM_GAME);

    UIButton *pGameHigh = new UIButton(pVideoOptionsTab,120+((pVideoOptionsTab->getPosition().nWidth-120)/3)*2,236,GAMETEXT_HIGH,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pGameHigh->setType(UI_BUTTON_TYPE_RADIO);
    pGameHigh->setID("GAMEHIGH");
    pGameHigh->enableWindow(true);
    pGameHigh->setFont(m_Renderer.getSmallFont());
    pGameHigh->setGroup(20025);
    pGameHigh->setContextHelp(CONTEXTHELP_HIGH_GAME);

    //UIButton *pMenuGraphics = new UIButton(pVideoOptionsTab,5,180,"Run Windowed",(pVideoOptionsTab->getPosition().nWidth-40)/1,28);
    //pRunWindowed->setType(UI_BUTTON_TYPE_CHECK);
    //pRunWindowed->setID("RUN_WINDOWED");
    //pRunWindowed->enableWindow(true);
    //pRunWindowed->setFont(m_Renderer.getSmallFont());

    
    UIWindow *pAudioOptionsTab = new UIWindow(pOptionsTabs,20,40,GAMETEXT_AUDIO,pOptionsTabs->getPosition().nWidth-40,pOptionsTabs->getPosition().nHeight);                  
    pAudioOptionsTab->enableWindow(true);
    pAudioOptionsTab->showWindow(false);
    pAudioOptionsTab->setID("AUDIO_TAB");

    UIButton *pEnableAudioButton = new UIButton(pAudioOptionsTab,5,5,GAMETEXT_ENABLEAUDIO,pAudioOptionsTab->getPosition().nWidth-10,28);
    pEnableAudioButton->setType(UI_BUTTON_TYPE_CHECK);
    pEnableAudioButton->setID("ENABLE_AUDIO");
    pEnableAudioButton->enableWindow(true);
    pEnableAudioButton->setFont(m_Renderer.getSmallFont());
    pEnableAudioButton->setContextHelp(CONTEXTHELP_SOUND_ON);
    
    UIButton *pSampleRate11Button = new UIButton(pAudioOptionsTab,25,33,GAMETEXT_11KHZ,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pSampleRate11Button->setType(UI_BUTTON_TYPE_RADIO);
    pSampleRate11Button->setID("RATE11KHZ");
    pSampleRate11Button->enableWindow(true);
    pSampleRate11Button->setFont(m_Renderer.getSmallFont());
    pSampleRate11Button->setGroup(10023);
    pSampleRate11Button->setContextHelp(CONTEXTHELP_11HZ);
    
    UIButton *pSampleRate22Button = new UIButton(pAudioOptionsTab,25 + ((pAudioOptionsTab->getPosition().nWidth-40)/3)*1,33,GAMETEXT_22KHZ,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pSampleRate22Button->setType(UI_BUTTON_TYPE_RADIO);
    pSampleRate22Button->setID("RATE22KHZ");
    pSampleRate22Button->enableWindow(true);
    pSampleRate22Button->setFont(m_Renderer.getSmallFont());
    pSampleRate22Button->setGroup(10023);
    pSampleRate22Button->setContextHelp(CONTEXTHELP_22HZ);
    
    UIButton *pSampleRate44Button = new UIButton(pAudioOptionsTab,25 + ((pAudioOptionsTab->getPosition().nWidth-40)/3)*2,33,GAMETEXT_44KHZ,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pSampleRate44Button->setType(UI_BUTTON_TYPE_RADIO);
    pSampleRate44Button->setID("RATE44KHZ");
    pSampleRate44Button->enableWindow(true);
    pSampleRate44Button->setFont(m_Renderer.getSmallFont());
    pSampleRate44Button->setGroup(10023);
    pSampleRate44Button->setContextHelp(CONTEXTHELP_44HZ);

    UIButton *pSample8Button = new UIButton(pAudioOptionsTab,25,61,GAMETEXT_8BIT,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pSample8Button->setType(UI_BUTTON_TYPE_RADIO);
    pSample8Button->setID("8BIT");
    pSample8Button->enableWindow(true);
    pSample8Button->setFont(m_Renderer.getSmallFont());
    pSample8Button->setGroup(10024);
    pSample8Button->setContextHelp(CONTEXTHELP_8BIT);

    UIButton *pSample16Button = new UIButton(pAudioOptionsTab,25 + ((pAudioOptionsTab->getPosition().nWidth-40)/3)*1,61,GAMETEXT_16BIT,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);    
    pSample16Button->setType(UI_BUTTON_TYPE_RADIO);
    pSample16Button->setID("16BIT");
    pSample16Button->enableWindow(true);
    pSample16Button->setFont(m_Renderer.getSmallFont());
    pSample16Button->setGroup(10024);
    pSample16Button->setContextHelp(CONTEXTHELP_16BIT);

    UIButton *pMonoButton = new UIButton(pAudioOptionsTab,25,89,GAMETEXT_MONO,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pMonoButton->setType(UI_BUTTON_TYPE_RADIO);
    pMonoButton->setID("MONO");
    pMonoButton->enableWindow(true);
    pMonoButton->setFont(m_Renderer.getSmallFont());
    pMonoButton->setGroup(10025);
    pMonoButton->setContextHelp(CONTEXTHELP_MONO);
    
    UIButton *pStereoButton = new UIButton(pAudioOptionsTab,25 + ((pAudioOptionsTab->getPosition().nWidth-40)/3)*1,89,GAMETEXT_STEREO,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pStereoButton->setType(UI_BUTTON_TYPE_RADIO);
    pStereoButton->setID("STEREO");
    pStereoButton->enableWindow(true);
    pStereoButton->setFont(m_Renderer.getSmallFont());
    pStereoButton->setGroup(10025);
    pStereoButton->setContextHelp(CONTEXTHELP_STEREO);

    UIButton *pEnableEngineSoundButton = new UIButton(pAudioOptionsTab,5,117,GAMETEXT_ENABLEENGINESOUND,pAudioOptionsTab->getPosition().nWidth-10,28);
    pEnableEngineSoundButton->setType(UI_BUTTON_TYPE_CHECK);
    pEnableEngineSoundButton->setID("ENABLE_ENGINE_SOUND");
    pEnableEngineSoundButton->enableWindow(true);
    pEnableEngineSoundButton->setFont(m_Renderer.getSmallFont());
    pEnableEngineSoundButton->setContextHelp(CONTEXTHELP_ENGINE_SOUND);
    
    UIButton *pEnableMusicButton = new UIButton(pAudioOptionsTab,5,145,GAMETEXT_ENABLEMUSIC,pAudioOptionsTab->getPosition().nWidth-10,28);
    pEnableMusicButton->setType(UI_BUTTON_TYPE_CHECK);
    pEnableMusicButton->setID("ENABLE_MUSIC");
    pEnableMusicButton->enableWindow(true);
    pEnableMusicButton->setFont(m_Renderer.getSmallFont());
    pEnableMusicButton->setContextHelp(CONTEXTHELP_MUSIC);
    
    UIWindow *pControlsOptionsTab = new UIWindow(pOptionsTabs,20,40,GAMETEXT_CONTROLS,pOptionsTabs->getPosition().nWidth-40,pOptionsTabs->getPosition().nHeight);                  
    pControlsOptionsTab->enableWindow(true);
    pControlsOptionsTab->showWindow(false);
    pControlsOptionsTab->setID("CONTROLS_TAB");

    UIButton *pKeyboardControls = new UIButton(pControlsOptionsTab,5,5,GAMETEXT_KEYBOARD,(pControlsOptionsTab->getPosition().nWidth-40)/2,28);
    pKeyboardControls->setType(UI_BUTTON_TYPE_RADIO);
    pKeyboardControls->setID("KEYBOARD");
    pKeyboardControls->enableWindow(true);
    pKeyboardControls->setFont(m_Renderer.getSmallFont());
    pKeyboardControls->setGroup(200243);
    //pKeyboardControls->setContextHelp("

    UIButton *pJoystickControls = new UIButton(pControlsOptionsTab,5 + ((pControlsOptionsTab->getPosition().nWidth-40)/2)*1,5,GAMETEXT_JOYSTICK,(pVideoOptionsTab->getPosition().nWidth-40)/2,28);
    pJoystickControls->setType(UI_BUTTON_TYPE_RADIO);
    pJoystickControls->setID("JOYSTICK");
    pJoystickControls->enableWindow(true);
    pJoystickControls->setFont(m_Renderer.getSmallFont());
    pJoystickControls->setGroup(200243);    

    UIList *pKeyCList = new UIList(pControlsOptionsTab,5,43,"",pControlsOptionsTab->getPosition().nWidth-10,118);
    pKeyCList->setID("KEY_ACTION_LIST");
    pKeyCList->setFont(m_Renderer.getSmallFont());
    pKeyCList->addColumn(GAMETEXT_ACTION,pKeyCList->getPosition().nWidth/2);
    pKeyCList->addColumn(GAMETEXT_KEY,pKeyCList->getPosition().nWidth/2);
    pKeyCList->setContextHelp(CONTEXTHELP_SELECT_ACTION);

    UIButton *pConfigureJoystick = new UIButton(pControlsOptionsTab,0,180,GAMETEXT_CONFIGUREJOYSTICK,207,57);
    pConfigureJoystick->setType(UI_BUTTON_TYPE_LARGE);
    pConfigureJoystick->setID("CONFIGURE_JOYSTICK");
    pConfigureJoystick->enableWindow(true);
    pConfigureJoystick->setFont(m_Renderer.getSmallFont());

#if defined(HIDE_JOYSTICK_SUPPORT)
  pKeyboardControls->showWindow(false);
  pJoystickControls->showWindow(false);
  pConfigureJoystick->showWindow(false);
  
  pKeyCList->setPosition(5,5,pControlsOptionsTab->getPosition().nWidth-10,238);
#endif

#if defined(SUPPORT_WEBACCESS)
    UIWindow *pWWWOptionsTab = new UIWindow(pOptionsTabs,0,26,GAMETEXT_WWWTAB,pOptionsTabs->getPosition().nWidth,pOptionsTabs->getPosition().nHeight);
    pWWWOptionsTab->enableWindow(true);
    pWWWOptionsTab->showWindow(false);
    pWWWOptionsTab->setID("WWW_TAB");

    UITabView *pWWWOptionsTabs  = new UITabView(pWWWOptionsTab,0,0,"",pWWWOptionsTab->getPosition().nWidth,pWWWOptionsTab->getPosition().nHeight-76);
    pWWWOptionsTabs->setID("WWWOPTIONS_TABS");
    pWWWOptionsTabs->setFont(m_Renderer.getSmallFont());
    pWWWOptionsTabs->setTabContextHelp(0, CONTEXTHELP_WWW_MAIN_TAB);
    pWWWOptionsTabs->setTabContextHelp(1, CONTEXTHELP_WWW_ROOMS_TAB);

    UIWindow *pWWWMainOptionsTab = new UIWindow(pWWWOptionsTabs,20,40,GAMETEXT_WWWMAINTAB,pWWWOptionsTabs->getPosition().nWidth-40,pWWWOptionsTabs->getPosition().nHeight);
    pWWWMainOptionsTab->enableWindow(true);
    pWWWMainOptionsTab->showWindow(true);
    pWWWMainOptionsTab->setID("WWW_MAIN_TAB");

    UIWindow *pWWWRoomsOptionsTab = new UIWindow(pWWWOptionsTabs,20,40,GAMETEXT_WWWROOMSTAB,pWWWOptionsTabs->getPosition().nWidth-40,pWWWOptionsTabs->getPosition().nHeight);
    pWWWRoomsOptionsTab->enableWindow(true);
    pWWWRoomsOptionsTab->showWindow(false);
    pWWWRoomsOptionsTab->setID("WWW_ROOMS_TAB");

    UIButton *pEnableWebHighscores = new UIButton(pWWWMainOptionsTab,5,5,GAMETEXT_ENABLEWEBHIGHSCORES,(pGeneralOptionsTab->getPosition().nWidth-40),28);
    pEnableWebHighscores->setType(UI_BUTTON_TYPE_CHECK);
    pEnableWebHighscores->setID("ENABLEWEBHIGHSCORES");
    pEnableWebHighscores->enableWindow(true);
    pEnableWebHighscores->setFont(m_Renderer.getSmallFont());
    pEnableWebHighscores->setGroup(50123);
    pEnableWebHighscores->setContextHelp(CONTEXTHELP_DOWNLOAD_BEST_TIMES);

    UIButton *pEnableCheckNewLevelsAtStartup = new UIButton(pWWWMainOptionsTab,5,43,GAMETEXT_ENABLECHECKNEWLEVELSATSTARTUP,(pGeneralOptionsTab->getPosition().nWidth-40),28);
    pEnableCheckNewLevelsAtStartup->setType(UI_BUTTON_TYPE_CHECK);
    pEnableCheckNewLevelsAtStartup->setID("ENABLECHECKNEWLEVELSATSTARTUP");
    pEnableCheckNewLevelsAtStartup->enableWindow(true);
    pEnableCheckNewLevelsAtStartup->setFont(m_Renderer.getSmallFont());
    pEnableCheckNewLevelsAtStartup->setGroup(50123);
    pEnableCheckNewLevelsAtStartup->setContextHelp(CONTEXTHELP_ENABLE_CHECK_NEW_LEVELS_AT_STARTUP);

    UIButton *pEnableCheckHighscoresAtStartup = new UIButton(pWWWMainOptionsTab,5,81,GAMETEXT_ENABLECHECKHIGHSCORESATSTARTUP,(pGeneralOptionsTab->getPosition().nWidth-40),28);
    pEnableCheckHighscoresAtStartup->setType(UI_BUTTON_TYPE_CHECK);
    pEnableCheckHighscoresAtStartup->setID("ENABLECHECKHIGHSCORESATSTARTUP");
    pEnableCheckHighscoresAtStartup->enableWindow(true);
    pEnableCheckHighscoresAtStartup->setFont(m_Renderer.getSmallFont());
    pEnableCheckHighscoresAtStartup->setGroup(50123);
    pEnableCheckHighscoresAtStartup->setContextHelp(CONTEXTHELP_ENABLE_CHECK_HIGHSCORES_AT_STARTUP);

    UIButton *pInGameWorldRecord = new UIButton(pWWWMainOptionsTab,5,119,GAMETEXT_ENABLEINGAMEWORLDRECORD,(pGeneralOptionsTab->getPosition().nWidth-40),28);
    pInGameWorldRecord->setType(UI_BUTTON_TYPE_CHECK);
    pInGameWorldRecord->setID("INGAMEWORLDRECORD");
    pInGameWorldRecord->enableWindow(true);
    pInGameWorldRecord->setFont(m_Renderer.getSmallFont());
    pInGameWorldRecord->setGroup(50123);
    pInGameWorldRecord->setContextHelp(CONTEXTHELP_INGAME_WORLD_RECORD);

    UIButton *pINetConf = new UIButton(pWWWOptionsTab,pWWWOptionsTab->getPosition().nWidth-225,pWWWOptionsTab->getPosition().nHeight-80,GAMETEXT_PROXYCONFIG,207,57);
    pINetConf->setType(UI_BUTTON_TYPE_LARGE);
    pINetConf->setID("PROXYCONFIG");
    pINetConf->setFont(m_Renderer.getSmallFont());
    pINetConf->setContextHelp(CONTEXTHELP_PROXYCONFIG);

    UIButton *pUpdHS = new UIButton(pWWWOptionsTab,pWWWOptionsTab->getPosition().nWidth-225-200,pWWWOptionsTab->getPosition().nHeight-80,GAMETEXT_UPDATEHIGHSCORES,207,57);
    pUpdHS->setType(UI_BUTTON_TYPE_LARGE);
    pUpdHS->setID("UPDATEHIGHSCORES");
    pUpdHS->setFont(m_Renderer.getSmallFont());
    pUpdHS->setContextHelp(CONTEXTHELP_UPDATEHIGHSCORES);

    // rooms tab
    UIList *pRoomsList = new UIList(pWWWRoomsOptionsTab,5,10,"",
				    pWWWRoomsOptionsTab->getPosition().nWidth-200,
				    pWWWRoomsOptionsTab->getPosition().nHeight-30 - 85);
    pRoomsList->setID("ROOMS_LIST");
    pRoomsList->setFont(m_Renderer.getSmallFont());
    pRoomsList->addColumn(GAMETEXT_ROOM, pThemeList->getPosition().nWidth);
    _UpdateRoomsLists();
    pRoomsList->setContextHelp(CONTEXTHELP_WWW_ROOMS_LIST);

    pSomeText = new UIStatic(pWWWRoomsOptionsTab,
			     pWWWRoomsOptionsTab->getPosition().nWidth-180,
			     5,
			     std::string(GAMETEXT_LOGIN) + ":",
			     130,
			     30);
    pSomeText->setHAlign(UI_ALIGN_LEFT);
    pSomeText->setFont(m_Renderer.getSmallFont()); 
    UIEdit *pRoomLoginEdit = new UIEdit(pWWWRoomsOptionsTab,
					pWWWRoomsOptionsTab->getPosition().nWidth-180,
					30,
					m_Config.getString("WebHighscoreUploadLogin"),150,25);
    pRoomLoginEdit->setFont(m_Renderer.getSmallFont());
    pRoomLoginEdit->setID("ROOM_LOGIN");
    pRoomLoginEdit->setContextHelp(CONTEXTHELP_ROOM_LOGIN);

    pSomeText = new UIStatic(pWWWRoomsOptionsTab,
			     pWWWRoomsOptionsTab->getPosition().nWidth-180,
			     65,
			     std::string(GAMETEXT_PASSWORD) + ":",
			     130,
			     30);
    pSomeText->setHAlign(UI_ALIGN_LEFT);
    pSomeText->setFont(m_Renderer.getSmallFont()); 
    UIEdit *pRoomPasswordEdit = new UIEdit(pWWWRoomsOptionsTab,
					pWWWRoomsOptionsTab->getPosition().nWidth-180,
					90,
					m_Config.getString("WebHighscoreUploadPassword"),150,25);
    pRoomPasswordEdit->hideText(true);
    pRoomPasswordEdit->setFont(m_Renderer.getSmallFont());
    pRoomPasswordEdit->setID("ROOM_PASSWORD");
    pRoomPasswordEdit->setContextHelp(CONTEXTHELP_ROOM_PASSWORD);

    UIButton *pUpdateRoomsButton = new UIButton(pWWWRoomsOptionsTab,
						pWWWRoomsOptionsTab->getPosition().nWidth/2 - 104,
						pWWWRoomsOptionsTab->getPosition().nHeight - 100,
						 GAMETEXT_UPDATEROOMSSLIST,
						 207,
						 57);
    pUpdateRoomsButton->setType(UI_BUTTON_TYPE_LARGE);
    pUpdateRoomsButton->setID("UPDATE_ROOMS_LIST");
    pUpdateRoomsButton->enableWindow(true);
    pUpdateRoomsButton->setFont(m_Renderer.getSmallFont());
    pUpdateRoomsButton->setContextHelp(CONTEXTHELP_UPDATEROOMSLIST);

#endif

    UIWindow *pGhostOptionsTab = new UIWindow(pOptionsTabs,20,40,GAMETEXT_GHOSTTAB,pOptionsTabs->getPosition().nWidth-40,pOptionsTabs->getPosition().nHeight);
    pGhostOptionsTab->enableWindow(true);
    pGhostOptionsTab->showWindow(false);
    pGhostOptionsTab->setID("GHOST_TAB");

    UIButton *pEnableGhost = new UIButton(pGhostOptionsTab,5,5,GAMETEXT_ENABLEGHOST,(pGhostOptionsTab->getPosition().nWidth-40)/2,28);
    pEnableGhost->setType(UI_BUTTON_TYPE_CHECK);
    pEnableGhost->setID("ENABLE_GHOST");
    pEnableGhost->enableWindow(true);
    pEnableGhost->setFont(m_Renderer.getSmallFont());
    pEnableGhost->setContextHelp(CONTEXTHELP_GHOST_MODE);

    UIList *pGhostStrategiesList = new UIList(pGhostOptionsTab,5,43,"",pGhostOptionsTab->getPosition().nWidth-10,125);
    pGhostStrategiesList->setID("GHOST_STRATEGIES_LIST");
    pGhostStrategiesList->setFont(m_Renderer.getSmallFont());
    pGhostStrategiesList->addColumn(GAMETEXT_GHOST_STRATEGIES_TYPE,pGhostStrategiesList->getPosition().nWidth);
    pGhostStrategiesList->addEntry(GAMETEXT_GHOST_STRATEGY_MYBEST, GhostSearchStrategies + 0);
    pGhostStrategiesList->addEntry(GAMETEXT_GHOST_STRATEGY_THEBEST, GhostSearchStrategies + 1);

#if defined(SUPPORT_WEBACCESS)
    pGhostStrategiesList->addEntry(GAMETEXT_GHOST_STRATEGY_BESTOFROOM, GhostSearchStrategies + 2);
#endif

    pGhostStrategiesList->setContextHelp(CONTEXTHELP_GHOST_STRATEGIES);

    UIButton *pDisplayGhostTimeDiff = new UIButton(pGhostOptionsTab,5,175,GAMETEXT_DISPLAYGHOSTTIMEDIFF,(pGhostOptionsTab->getPosition().nWidth-40),28);
    pDisplayGhostTimeDiff->setType(UI_BUTTON_TYPE_CHECK);
    pDisplayGhostTimeDiff->setID("DISPLAY_GHOST_TIMEDIFF");
    pDisplayGhostTimeDiff->enableWindow(true);
    pDisplayGhostTimeDiff->setFont(m_Renderer.getSmallFont());
    pDisplayGhostTimeDiff->setContextHelp(CONTEXTHELP_DISPLAY_GHOST_TIMEDIFF);

    UIButton *pDisplayGhostInfo = new UIButton(pGhostOptionsTab,5,203,GAMETEXT_DISPLAYGHOSTINFO,(pGhostOptionsTab->getPosition().nWidth-40),28);
    pDisplayGhostInfo->setType(UI_BUTTON_TYPE_CHECK);
    pDisplayGhostInfo->setID("DISPLAY_GHOST_INFO");
    pDisplayGhostInfo->enableWindow(true);
    pDisplayGhostInfo->setFont(m_Renderer.getSmallFont());
    pDisplayGhostInfo->setContextHelp(CONTEXTHELP_DISPLAY_GHOST_INFO);

    UIButton *pMotionBlurGhost = new UIButton(pGhostOptionsTab,5,231,GAMETEXT_MOTIONBLURGHOST,(pGhostOptionsTab->getPosition().nWidth-40),28);
    pMotionBlurGhost->setType(UI_BUTTON_TYPE_CHECK);
    pMotionBlurGhost->setID("MOTION_BLUR_GHOST");
    pMotionBlurGhost->enableWindow(true);
    pMotionBlurGhost->setFont(m_Renderer.getSmallFont());
    pMotionBlurGhost->setContextHelp(CONTEXTHELP_MOTIONBLURGHOST);

    /* ***** */

    m_pLevelPacksWindow = new UIFrame(m_pMainMenu,220,(drawLib->getDispHeight()*140)/600,"",drawLib->getDispWidth()-220-20,drawLib->getDispHeight()-40-(drawLib->getDispHeight()*120)/600-10);      
    m_pLevelPacksWindow->showWindow(false);
    pSomeText = new UIStatic(m_pLevelPacksWindow,0,0,GAMETEXT_LEVELS,m_pLevelPacksWindow->getPosition().nWidth,36);
    pSomeText->setFont(m_Renderer.getMediumFont());

    /* tabs of the packs */
    m_pLevelPackTabs = new UITabView(m_pLevelPacksWindow,20,40,"",m_pLevelPacksWindow->getPosition().nWidth-40,m_pLevelPacksWindow->getPosition().nHeight-60);
    m_pLevelPackTabs->setFont(m_Renderer.getSmallFont());
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
    pOpenButton->setFont(m_Renderer.getSmallFont());
    pOpenButton->setType(UI_BUTTON_TYPE_SMALL);
    pOpenButton->setID("LEVELPACK_OPEN_BUTTON");
    pOpenButton->setContextHelp(CONTEXTHELP_VIEW_LEVEL_PACK);

    /* pack list */
    UIPackTree *pLevelPackTree = new UIPackTree(pPackTab,10,0,"",pPackTab->getPosition().nWidth-20,pPackTab->getPosition().nHeight-105);      
    pLevelPackTree->setID("LEVELPACK_TREE");
    pLevelPackTree->showWindow(true);
    pLevelPackTree->enableWindow(true);
    pLevelPackTree->setFont(m_Renderer.getSmallFont());
    pLevelPackTree->setEnterButton( pOpenButton );

    /* favorite levels tab */
    UIWindow *pAllLevelsPackTab = new UIWindow(m_pLevelPackTabs,20,40,VPACKAGENAME_FAVORITE_LEVELS,m_pLevelPackTabs->getPosition().nWidth-40,m_pLevelPackTabs->getPosition().nHeight);
    pAllLevelsPackTab->enableWindow(true);
    pAllLevelsPackTab->showWindow(false);
    pAllLevelsPackTab->setID("ALLLEVELS_TAB");

    /* all levels button */
    UIButton *pGoButton = new UIButton(pAllLevelsPackTab,0,pAllLevelsPackTab->getPosition().nHeight-103,GAMETEXT_STARTLEVEL,105,57);
    pGoButton->setContextHelp(CONTEXTHELP_PLAY_SELECTED_LEVEL);
    pGoButton->setFont(m_Renderer.getSmallFont());
    pGoButton->setType(UI_BUTTON_TYPE_SMALL);
    pGoButton->setID("PLAY_GO_BUTTON");
    UIButton *pLevelInfoButton = new UIButton(pAllLevelsPackTab,105,pAllLevelsPackTab->getPosition().nHeight-103,GAMETEXT_SHOWINFO,105,57);
    pLevelInfoButton->setFont(m_Renderer.getSmallFont());
    pLevelInfoButton->setType(UI_BUTTON_TYPE_SMALL);
    pLevelInfoButton->setID("PLAY_LEVEL_INFO_BUTTON");
    pLevelInfoButton->setContextHelp(CONTEXTHELP_LEVEL_INFO);

    UIButton *pDeleteFromFavoriteButton = new UIButton(pAllLevelsPackTab,pAllLevelsPackTab->getPosition().nWidth-187,pAllLevelsPackTab->getPosition().nHeight-103,GAMETEXT_DELETEFROMFAVORITE,187,57);
    pDeleteFromFavoriteButton->setFont(m_Renderer.getSmallFont());
    pDeleteFromFavoriteButton->setType(UI_BUTTON_TYPE_LARGE);
    pDeleteFromFavoriteButton->setID("ALL_LEVELS_DELETE_FROM_FAVORITE_BUTTON");
    pDeleteFromFavoriteButton->setContextHelp(CONTEXTHELP_DELETEFROMFAVORITE);

    /* all levels list */
    m_pAllLevelsList = new UILevelList(pAllLevelsPackTab,0,0,"",pAllLevelsPackTab->getPosition().nWidth,pAllLevelsPackTab->getPosition().nHeight-105);     
    m_pAllLevelsList->setID("ALLLEVELS_LIST");
    m_pAllLevelsList->setFont(m_Renderer.getSmallFont());
    m_pAllLevelsList->setSort(true);
    m_pAllLevelsList->setEnterButton( pGoButton );

    /* new levels tab */
#if defined(SUPPORT_WEBACCESS)
    UIWindow *pNewLevelsPackTab = new UIWindow(m_pLevelPackTabs,20,40,GAMETEXT_NEWLEVELS,m_pLevelPackTabs->getPosition().nWidth-40,m_pLevelPackTabs->getPosition().nHeight);
    pNewLevelsPackTab->enableWindow(true);
    pNewLevelsPackTab->showWindow(false);
    pNewLevelsPackTab->setID("NEWLEVELS_TAB");

    /* new levels tab buttons */
    UIButton *pNewLevelsGoButton = new UIButton(pNewLevelsPackTab,0,pNewLevelsPackTab->getPosition().nHeight-103,GAMETEXT_STARTLEVEL,105,57);
    pNewLevelsGoButton->setContextHelp(CONTEXTHELP_PLAY_SELECTED_LEVEL);
    pNewLevelsGoButton->setFont(m_Renderer.getSmallFont());
    pNewLevelsGoButton->setType(UI_BUTTON_TYPE_SMALL);
    pNewLevelsGoButton->setID("NEW_LEVELS_PLAY_GO_BUTTON");
    UIButton *pNewLevelsLevelInfoButton = new UIButton(pNewLevelsPackTab,105,pNewLevelsPackTab->getPosition().nHeight-103,GAMETEXT_SHOWINFO,105,57);
    pNewLevelsLevelInfoButton->setFont(m_Renderer.getSmallFont());
    pNewLevelsLevelInfoButton->setType(UI_BUTTON_TYPE_SMALL);
    pNewLevelsLevelInfoButton->setID("NEW_LEVELS_PLAY_LEVEL_INFO_BUTTON");
    pNewLevelsLevelInfoButton->setContextHelp(CONTEXTHELP_LEVEL_INFO);

    UIButton *pDownloadLevelsButton = new UIButton(pNewLevelsPackTab,pNewLevelsPackTab->getPosition().nWidth-187,pNewLevelsPackTab->getPosition().nHeight-103,GAMETEXT_DOWNLOADLEVELS,187,57);
    pDownloadLevelsButton->setFont(m_Renderer.getSmallFont());
    pDownloadLevelsButton->setType(UI_BUTTON_TYPE_LARGE);
    pDownloadLevelsButton->setID("NEW_LEVELS_PLAY_DOWNLOAD_LEVELS_BUTTON");
    pDownloadLevelsButton->setContextHelp(CONTEXTHELP_DOWNLOADLEVELS);

    /* all levels list */
    m_pPlayNewLevelsList = new UILevelList(pNewLevelsPackTab,0,0,"",pNewLevelsPackTab->getPosition().nWidth,pNewLevelsPackTab->getPosition().nHeight-105);     
    m_pPlayNewLevelsList->setID("NEWLEVELS_LIST");
    m_pPlayNewLevelsList->setFont(m_Renderer.getSmallFont());
    m_pPlayNewLevelsList->setSort(true);
    m_pPlayNewLevelsList->setEnterButton( pNewLevelsGoButton );
#endif

  }

  void GameApp::_InitMenus_Others(void) {
    UIStatic *pSomeText;

#if defined(SUPPORT_WEBACCESS)
    /* Initialize internet connection configurator */
    m_pWebConfEditor = new UIFrame(m_Renderer.getGUI(),drawLib->getDispWidth()/2-206,drawLib->getDispHeight()/2-385/2,"",412,425); 
    m_pWebConfEditor->setStyle(UI_FRAMESTYLE_TRANS);           
    m_pWebConfEditor->showWindow(false);
    UIStatic *pWebConfEditorTitle = new UIStatic(m_pWebConfEditor,0,0,GAMETEXT_INETCONF,400,50);
    pWebConfEditorTitle->setFont(m_Renderer.getMediumFont());
   
    #if defined(WIN32)
      /* I don't expect a windows user to know what an environment variable is */
      #define DIRCONNTEXT std::string(GAMETEXT_DIRECTCONN).c_str()
    #else
      #define DIRCONNTEXT (std::string(GAMETEXT_DIRECTCONN) + " / " + std::string(GAMETEXT_USEENVVARS)).c_str()
    #endif
     
    UIButton *pConn1 = new UIButton(m_pWebConfEditor,25,60,DIRCONNTEXT,(m_pWebConfEditor->getPosition().nWidth-50),28);
    pConn1->setType(UI_BUTTON_TYPE_RADIO);
    pConn1->setID("DIRECTCONN");
    pConn1->enableWindow(true);
    pConn1->setFont(m_Renderer.getSmallFont());
    pConn1->setGroup(16023);
    pConn1->setChecked(true);
    pConn1->setContextHelp(CONTEXTHELP_DIRECTCONN);

    UIButton *pConn2 = new UIButton(m_pWebConfEditor,25,88,GAMETEXT_USINGHTTPPROXY,(m_pWebConfEditor->getPosition().nWidth-160),28);
    pConn2->setType(UI_BUTTON_TYPE_RADIO);
    pConn2->setID("HTTPPROXY");
    pConn2->enableWindow(true);
    pConn2->setFont(m_Renderer.getSmallFont());
    pConn2->setGroup(16023);
    pConn2->setContextHelp(CONTEXTHELP_HTTPPROXY);

    UIButton *pConn3 = new UIButton(m_pWebConfEditor,25,116,GAMETEXT_USINGSOCKS4PROXY,(m_pWebConfEditor->getPosition().nWidth-160),28);
    pConn3->setType(UI_BUTTON_TYPE_RADIO);
    pConn3->setID("SOCKS4PROXY");
    pConn3->enableWindow(true);
    pConn3->setFont(m_Renderer.getSmallFont());
    pConn3->setGroup(16023);
    pConn3->setContextHelp(CONTEXTHELP_SOCKS4PROXY);

    UIButton *pConn4 = new UIButton(m_pWebConfEditor,25,144,GAMETEXT_USINGSOCKS5PROXY,(m_pWebConfEditor->getPosition().nWidth-160),28);
    pConn4->setType(UI_BUTTON_TYPE_RADIO);
    pConn4->setID("SOCKS5PROXY");
    pConn4->enableWindow(true);
    pConn4->setFont(m_Renderer.getSmallFont());
    pConn4->setGroup(16023);
    pConn4->setContextHelp(CONTEXTHELP_SOCKS5PROXY);
    
    UIButton *pConnOKButton = new UIButton(m_pWebConfEditor,(m_pWebConfEditor->getPosition().nWidth-160)+28,(m_pWebConfEditor->getPosition().nHeight-68),GAMETEXT_OK,115,57);
    pConnOKButton->setFont(m_Renderer.getSmallFont());
    pConnOKButton->setType(UI_BUTTON_TYPE_SMALL);
    pConnOKButton->setID("PROXYOK");
    pConnOKButton->setContextHelp(CONTEXTHELP_OKPROXY);
    
    UIFrame *pSubFrame = new UIFrame(m_pWebConfEditor,25,185,"",(m_pWebConfEditor->getPosition().nWidth-50),(m_pWebConfEditor->getPosition().nHeight-185-75));
    pSubFrame->setStyle(UI_FRAMESTYLE_TRANS);
    pSubFrame->setID("SUBFRAME");    
    
    pSomeText = new UIStatic(pSubFrame,10,25,std::string(GAMETEXT_PROXYSERVER) + ":",120,25);
    pSomeText->setFont(m_Renderer.getSmallFont());    
    pSomeText->setHAlign(UI_ALIGN_RIGHT);
    UIEdit *pProxyServerEdit = new UIEdit(pSubFrame,135,25,"",190,25);
    pProxyServerEdit->setFont(m_Renderer.getSmallFont());
    pProxyServerEdit->setID("SERVEREDIT");
    pProxyServerEdit->setContextHelp(CONTEXTHELP_PROXYSERVER);

#define CONTEXTHELP_PROXYLOGIN "abc"
#define CONTEXTHELP_PROXYPASSWORD "abc"

    pSomeText = new UIStatic(pSubFrame,10,55,std::string(GAMETEXT_PORT) + ":",120,25);
    pSomeText->setFont(m_Renderer.getSmallFont());    
    pSomeText->setHAlign(UI_ALIGN_RIGHT);
    UIEdit *pProxyPortEdit = new UIEdit(pSubFrame,135,55,"",50,25);
    pProxyPortEdit->setFont(m_Renderer.getSmallFont());
    pProxyPortEdit->setID("PORTEDIT");
    pProxyPortEdit->setContextHelp(CONTEXTHELP_PROXYPORT);

    pSomeText = new UIStatic(pSubFrame,10,85,std::string(GAMETEXT_LOGIN) + ":",120,25);
    pSomeText->setFont(m_Renderer.getSmallFont());    
    pSomeText->setHAlign(UI_ALIGN_RIGHT);
    UIEdit *pProxyLoginEdit = new UIEdit(pSubFrame,135,85,"",190,25);
    pProxyLoginEdit->setFont(m_Renderer.getSmallFont());
    pProxyLoginEdit->setID("LOGINEDIT");
    pProxyLoginEdit->setContextHelp(CONTEXTHELP_PROXYLOGIN);

    pSomeText = new UIStatic(pSubFrame,10,115,std::string(GAMETEXT_PASSWORD) + ":",120,25);
    pSomeText->setFont(m_Renderer.getSmallFont());    
    pSomeText->setHAlign(UI_ALIGN_RIGHT);
    UIEdit *pProxyPasswordEdit = new UIEdit(pSubFrame,135,115,"",190,25);
    pProxyPasswordEdit->setFont(m_Renderer.getSmallFont());
    pProxyPasswordEdit->setID("PASSWORDEDIT");
    pProxyPasswordEdit->setContextHelp(CONTEXTHELP_PROXYPASSWORD);
#endif

    /* Initialize profile editor */
    m_pProfileEditor = new UIFrame(m_Renderer.getGUI(),drawLib->getDispWidth()/2-350,drawLib->getDispHeight()/2-250,"",700,500); 
    m_pProfileEditor->setStyle(UI_FRAMESTYLE_TRANS);           
    UIStatic *pProfileEditorTitle = new UIStatic(m_pProfileEditor,0,0,GAMETEXT_PLAYERPROFILES,700,50);
    pProfileEditorTitle->setFont(m_Renderer.getMediumFont());
    UIList *pProfileList = new UIList(m_pProfileEditor,20,50,"",400,430);
    pProfileList->setFont(m_Renderer.getSmallFont());
    pProfileList->addColumn(GAMETEXT_PLAYERPROFILE,128);      
    pProfileList->setID("PROFILE_LIST");
    pProfileList->setContextHelp(CONTEXTHELP_SELECT_PLAYER_PROFILE);
    UIButton *pProfUseButton = new UIButton(m_pProfileEditor,450,50,GAMETEXT_USEPROFILE,207,57);
    pProfUseButton->setFont(m_Renderer.getSmallFont());
    pProfUseButton->setID("USEPROFILE_BUTTON");
    pProfUseButton->setContextHelp(CONTEXTHELP_USE_PLAYER_PROFILE);
    UIButton *pProfNewButton = new UIButton(m_pProfileEditor,450,107,GAMETEXT_NEWPROFILE,207,57);
    pProfNewButton->setFont(m_Renderer.getSmallFont());
    pProfNewButton->setID("NEWPROFILE_BUTTON");
    pProfNewButton->setContextHelp(CONTEXTHELP_CREATE_PLAYER_PROFILE);
    UIButton *pProfCancelButton = new UIButton(m_pProfileEditor,450,164,GAMETEXT_CLOSE,207,57);
    pProfCancelButton->setFont(m_Renderer.getSmallFont());
    pProfCancelButton->setID("CANCEL_BUTTON");
    pProfCancelButton->setContextHelp(CONTEXTHELP_CLOSE_PROFILE_EDITOR);
    UIButton *pProfDeleteButton = new UIButton(m_pProfileEditor,450,423,GAMETEXT_DELETEPROFILE,207,57);
    pProfDeleteButton->setFont(m_Renderer.getSmallFont());
    pProfDeleteButton->setID("DELETEPROFILE_BUTTON");
    pProfDeleteButton->setContextHelp(CONTEXTHELP_DELETE_PROFILE);
    pProfileList->setEnterButton( pProfUseButton );
    _CreateProfileList();

    /* Initialize level pack viewer */
    m_pLevelPackViewer = new UIFrame(m_Renderer.getGUI(),drawLib->getDispWidth()/2-350,drawLib->getDispHeight()/2-250,"",700,500); 
    m_pLevelPackViewer->setStyle(UI_FRAMESTYLE_TRANS);           
    UIStatic *pLevelPackViewerTitle = new UIStatic(m_pLevelPackViewer,0,0,"(level pack name goes here)",700,40);
    pLevelPackViewerTitle->setID("LEVELPACK_VIEWER_TITLE");
    pLevelPackViewerTitle->setFont(m_Renderer.getMediumFont());

    UIButton *pLevelPackPlay = new UIButton(m_pLevelPackViewer,450,50,GAMETEXT_STARTLEVEL,207,57);
    pLevelPackPlay->setFont(m_Renderer.getSmallFont());
    pLevelPackPlay->setID("LEVELPACK_PLAY_BUTTON");
    pLevelPackPlay->setContextHelp(CONTEXTHELP_PLAY_SELECTED_LEVEL);

    pSomeText = new UIStatic(m_pLevelPackViewer, 20, 70, std::string(GAMETEXT_FILTER) + ":", 90, 25);
    pSomeText->setFont(m_Renderer.getSmallFont());
    pSomeText->setHAlign(UI_ALIGN_RIGHT);
    UIEdit *pLevelFilterEdit = new UIEdit(m_pLevelPackViewer,
																					120,
																					70,
																					"",200,25);
    pLevelFilterEdit->setFont(m_Renderer.getSmallFont());
    pLevelFilterEdit->setID("LEVELPACK_LEVEL_FILTER");
    pLevelFilterEdit->setContextHelp(CONTEXTHELP_LEVEL_FILTER);

    UILevelList *pLevelPackLevelList = new UILevelList(m_pLevelPackViewer,20,100,"",400, 380);
    pLevelPackLevelList->setFont(m_Renderer.getSmallFont());
    pLevelPackLevelList->setContextHelp(CONTEXTHELP_SELECT_LEVEL_IN_LEVEL_PACK);
    pLevelPackLevelList->setID("LEVELPACK_LEVEL_LIST");
    pLevelPackLevelList->setEnterButton( pLevelPackPlay );

    UIButton *pLevelPackInfo = new UIButton(m_pLevelPackViewer,450,107,GAMETEXT_LEVELINFO,207,57);
    pLevelPackInfo->setFont(m_Renderer.getSmallFont());
    pLevelPackInfo->setID("LEVELPACK_INFO_BUTTON");
    pLevelPackInfo->setContextHelp(CONTEXTHELP_LEVEL_INFO);

    UIButton *pLevelPackAddToFavorite = new UIButton(m_pLevelPackViewer,450,164,GAMETEXT_ADDTOFAVORITE,207,57);
    pLevelPackAddToFavorite->setFont(m_Renderer.getSmallFont());
    pLevelPackAddToFavorite->setID("LEVELPACK_ADDTOFAVORITE_BUTTON");
    pLevelPackAddToFavorite->setContextHelp(CONTEXTHELP_ADDTOFAVORITE);

    UIButton *pLevelPackRandomize = new UIButton(m_pLevelPackViewer,450,221,GAMETEXT_RANDOMIZE,207,57);
    pLevelPackRandomize->setFont(m_Renderer.getSmallFont());
    pLevelPackRandomize->setID("LEVELPACK_RANDOMIZE_BUTTON");
    pLevelPackRandomize->setContextHelp(CONTEXTHELP_RANDOMIZE);

    UIButton *pLevelPackCancel = new UIButton(m_pLevelPackViewer,450,278,GAMETEXT_CLOSE,207,57);
    pLevelPackCancel->setFont(m_Renderer.getSmallFont());
    pLevelPackCancel->setID("LEVELPACK_CANCEL_BUTTON");
    pLevelPackCancel->setContextHelp(CONTEXTHELP_CLOSE_LEVEL_PACK);

#if defined(SUPPORT_WEBACCESS)        
    /* level info frame */
    m_pPackLevelInfoFrame = new UIWindow(m_pLevelPackViewer,419,400,"",275,100);
    m_pPackLevelInfoFrame->showWindow(false);
    m_pPackBestPlayerText = new UIStatic(m_pPackLevelInfoFrame, 0, 5,"", 275, 50);
    m_pPackBestPlayerText->setHAlign(UI_ALIGN_RIGHT);
    m_pPackBestPlayerText->setFont(m_Renderer.getSmallFont());
    m_pPackBestPlayerText->setHAlign(UI_ALIGN_CENTER);
    m_pPackBestPlayerText->showWindow(true);
    m_pPackLevelInfoViewReplayButton = new UIButton(m_pPackLevelInfoFrame,50,40, GAMETEXT_VIEWTHEHIGHSCORE,175,40);
    m_pPackLevelInfoViewReplayButton->setFont(m_Renderer.getSmallFont());
    m_pPackLevelInfoViewReplayButton->setContextHelp(CONTEXTHELP_VIEWTHEHIGHSCORE);
#endif

    /* Initialize level info viewer */
    m_pLevelInfoViewer = new UIFrame(m_Renderer.getGUI(),drawLib->getDispWidth()/2-350,drawLib->getDispHeight()/2-250,"",700,500); 
    m_pLevelInfoViewer->setStyle(UI_FRAMESTYLE_TRANS);
    UIStatic *pLevelInfoViewerTitle = new UIStatic(m_pLevelInfoViewer,0,0,"(level name goes here)",700,40);
    pLevelInfoViewerTitle->setID("LEVEL_VIEWER_TITLE");  
    pLevelInfoViewerTitle->setFont(m_Renderer.getMediumFont());
    UITabView *pLevelViewerTabs = new UITabView(m_pLevelInfoViewer,20,40,"",m_pLevelInfoViewer->getPosition().nWidth-40,m_pLevelInfoViewer->getPosition().nHeight-115);
    pLevelViewerTabs->setFont(m_Renderer.getSmallFont());
    pLevelViewerTabs->setID("LEVEL_VIEWER_TABS");  
    pLevelViewerTabs->setTabContextHelp(0,CONTEXTHELP_GENERAL_INFO);
    pLevelViewerTabs->setTabContextHelp(1,CONTEXTHELP_BEST_TIMES_INFO);
    pLevelViewerTabs->setTabContextHelp(2,CONTEXTHELP_REPLAYS_INFO);
    UIWindow *pLVTab_Info = new UIWindow(pLevelViewerTabs,20,40,GAMETEXT_GENERALINFO,pLevelViewerTabs->getPosition().nWidth-40,pLevelViewerTabs->getPosition().nHeight-60);
    pLVTab_Info->enableWindow(true);
    pLVTab_Info->setID("LEVEL_VIEWER_GENERALINFO_TAB");
    UIWindow *pLVTab_BestTimes = new UIWindow(pLevelViewerTabs,20,40,GAMETEXT_BESTTIMES,pLevelViewerTabs->getPosition().nWidth-40,pLevelViewerTabs->getPosition().nHeight-60);
    pLVTab_BestTimes->enableWindow(true);
    pLVTab_BestTimes->showWindow(false);
    pLVTab_BestTimes->setID("LEVEL_VIEWER_BESTTIMES_TAB");
    UIWindow *pLVTab_Replays = new UIWindow(pLevelViewerTabs,20,40,GAMETEXT_REPLAYS,pLevelViewerTabs->getPosition().nWidth-40,pLevelViewerTabs->getPosition().nHeight-60);
    pLVTab_Replays->enableWindow(true);
    pLVTab_Replays->showWindow(false);
    pLVTab_Replays->setID("LEVEL_VIEWER_REPLAYS_TAB");
    UIButton *pOKButton = new UIButton(m_pLevelInfoViewer,11,m_pLevelInfoViewer->getPosition().nHeight-68,GAMETEXT_OK,115,57);
    pOKButton->setFont(m_Renderer.getSmallFont());
    pOKButton->setType(UI_BUTTON_TYPE_SMALL);
    pOKButton->setID("LEVEL_VIEWER_OK_BUTTON");
    pOKButton->setContextHelp(CONTEXTHELP_BACK_TO_MAIN_MENU);
    
    /* Level info viewer - general info */
    UIStatic *pLV_Info_LevelPack = new UIStatic(pLVTab_Info,0,0,"(pack name goes here)",pLVTab_Info->getPosition().nWidth,40);
    pLV_Info_LevelPack->setID("LEVEL_VIEWER_INFO_LEVELPACK");
    pLV_Info_LevelPack->showWindow(true);
    pLV_Info_LevelPack->setHAlign(UI_ALIGN_LEFT);
    pLV_Info_LevelPack->setVAlign(UI_ALIGN_TOP);
    pLV_Info_LevelPack->setFont(m_Renderer.getSmallFont());
    UIStatic *pLV_Info_LevelName = new UIStatic(pLVTab_Info,0,40,"(level name goes here)",pLVTab_Info->getPosition().nWidth,40);
    pLV_Info_LevelName->setID("LEVEL_VIEWER_INFO_LEVELNAME");
    pLV_Info_LevelName->showWindow(true);
    pLV_Info_LevelName->setHAlign(UI_ALIGN_LEFT);
    pLV_Info_LevelName->setVAlign(UI_ALIGN_TOP);
    pLV_Info_LevelName->setFont(m_Renderer.getSmallFont());
    UIStatic *pLV_Info_Author = new UIStatic(pLVTab_Info,0,80,"(author goes here)",pLVTab_Info->getPosition().nWidth,40);
    pLV_Info_Author->setID("LEVEL_VIEWER_INFO_AUTHOR");
    pLV_Info_Author->showWindow(true);
    pLV_Info_Author->setHAlign(UI_ALIGN_LEFT);                
    pLV_Info_Author->setVAlign(UI_ALIGN_TOP);
    pLV_Info_Author->setFont(m_Renderer.getSmallFont());
    UIStatic *pLV_Info_Date = new UIStatic(pLVTab_Info,0,120,"(date goes here)",pLVTab_Info->getPosition().nWidth,40);
    pLV_Info_Date->setID("LEVEL_VIEWER_INFO_DATE");
    pLV_Info_Date->showWindow(true);
    pLV_Info_Date->setHAlign(UI_ALIGN_LEFT);                
    pLV_Info_Date->setVAlign(UI_ALIGN_TOP);
    pLV_Info_Date->setFont(m_Renderer.getSmallFont());
    UIStatic *pLV_Info_Description = new UIStatic(pLVTab_Info,0,160,"(description goes here)",pLVTab_Info->getPosition().nWidth,200);
    pLV_Info_Description->setID("LEVEL_VIEWER_INFO_DESCRIPTION");
    pLV_Info_Description->showWindow(true);
    pLV_Info_Description->setHAlign(UI_ALIGN_LEFT);                
    pLV_Info_Description->setVAlign(UI_ALIGN_TOP);
    pLV_Info_Description->setFont(m_Renderer.getSmallFont());
    
    /* Level info viewer - best times */
    UIButton *pLV_BestTimes_Personal = new UIButton(pLVTab_BestTimes,5,5,GAMETEXT_PERSONAL,(pLVTab_BestTimes->getPosition().nWidth-40)/2,28);
    pLV_BestTimes_Personal->setType(UI_BUTTON_TYPE_RADIO);
    pLV_BestTimes_Personal->setID("LEVEL_VIEWER_BESTTIMES_PERSONAL");
    pLV_BestTimes_Personal->enableWindow(true);
    pLV_BestTimes_Personal->setChecked(true);
    pLV_BestTimes_Personal->setFont(m_Renderer.getSmallFont());
    pLV_BestTimes_Personal->setGroup(421023);
    pLV_BestTimes_Personal->setContextHelp(CONTEXTHELP_ONLY_SHOW_PERSONAL_BESTS);
    UIButton *pLV_BestTimes_All = new UIButton(pLVTab_BestTimes,5 + ((pLVTab_BestTimes->getPosition().nWidth-40)/2)*1,5,GAMETEXT_ALL,(pLVTab_BestTimes->getPosition().nWidth-40)/2,28);
    pLV_BestTimes_All->setType(UI_BUTTON_TYPE_RADIO);
    pLV_BestTimes_All->setID("LEVEL_VIEWER_BESTTIMES_ALL");
    pLV_BestTimes_All->enableWindow(true);
    pLV_BestTimes_All->setChecked(false);
    pLV_BestTimes_All->setFont(m_Renderer.getSmallFont());
    pLV_BestTimes_All->setGroup(421023);
    pLV_BestTimes_All->setContextHelp(CONTEXTHELP_SHOW_ALL_BESTS);
    
    UIList *pLV_BestTimes_List= new UIList(pLVTab_BestTimes,5,43,"",pLVTab_BestTimes->getPosition().nWidth-10,pLVTab_BestTimes->getPosition().nHeight-100);
    pLV_BestTimes_List->setID("LEVEL_VIEWER_BESTTIMES_LIST");
    pLV_BestTimes_List->setFont(m_Renderer.getSmallFont());
    pLV_BestTimes_List->addColumn(GAMETEXT_FINISHTIME,128);
    pLV_BestTimes_List->addColumn(GAMETEXT_PLAYER,pLV_BestTimes_List->getPosition().nWidth-128);    
    UIStatic *pLV_BestTimes_WorldRecord = new UIStatic(pLVTab_BestTimes,5,pLVTab_BestTimes->getPosition().nHeight-50,"",pLVTab_BestTimes->getPosition().nWidth,50);
    pLV_BestTimes_WorldRecord->setID("LEVEL_VIEWER_BESTTIMES_WORLDRECORD");
    pLV_BestTimes_WorldRecord->setFont(m_Renderer.getSmallFont());
    pLV_BestTimes_WorldRecord->setVAlign(UI_ALIGN_CENTER);
    pLV_BestTimes_WorldRecord->setHAlign(UI_ALIGN_LEFT);

    /* Level info viewer - replays */
    UIButton *pLV_Replays_Personal = new UIButton(pLVTab_Replays,5,5,GAMETEXT_PERSONAL,(pLVTab_Replays->getPosition().nWidth-40)/2,28);
    pLV_Replays_Personal->setType(UI_BUTTON_TYPE_RADIO);
    pLV_Replays_Personal->setID("LEVEL_VIEWER_REPLAYS_PERSONAL");
    pLV_Replays_Personal->enableWindow(true);
    pLV_Replays_Personal->setChecked(false);
    pLV_Replays_Personal->setFont(m_Renderer.getSmallFont());
    pLV_Replays_Personal->setGroup(421024);
    pLV_Replays_Personal->setContextHelp(CONTEXTHELP_ONLY_SHOW_PERSONAL_REPLAYS);
    UIButton *pLV_Replays_All = new UIButton(pLVTab_Replays,5 + ((pLVTab_Replays->getPosition().nWidth-40)/2)*1,5,GAMETEXT_ALL,(pLVTab_Replays->getPosition().nWidth-40)/2,28);
    pLV_Replays_All->setType(UI_BUTTON_TYPE_RADIO);
    pLV_Replays_All->setID("LEVEL_VIEWER_REPLAYS_ALL");
    pLV_Replays_All->enableWindow(true);
    pLV_Replays_All->setChecked(true);
    pLV_Replays_All->setFont(m_Renderer.getSmallFont());
    pLV_Replays_All->setGroup(421024);
    pLV_Replays_All->setContextHelp(CONTEXTHELP_SHOW_ALL_REPLAYS);
    
    UIList *pLV_Replays_List= new UIList(pLVTab_Replays,5,43,"",pLVTab_Replays->getPosition().nWidth-10,pLVTab_Replays->getPosition().nHeight-100);
    pLV_Replays_List->setID("LEVEL_VIEWER_REPLAYS_LIST");
    pLV_Replays_List->setFont(m_Renderer.getSmallFont());
    pLV_Replays_List->addColumn(GAMETEXT_REPLAY,128);
    pLV_Replays_List->addColumn(GAMETEXT_PLAYER,128);
    pLV_Replays_List->addColumn(GAMETEXT_FINISHTIME,128);    
    UIButton *pLV_Replays_Show = new UIButton(pLVTab_Replays,0,pLVTab_Replays->getPosition().nHeight-50,GAMETEXT_SHOW,115,57);
    pLV_Replays_Show->setFont(m_Renderer.getSmallFont());
    pLV_Replays_Show->setType(UI_BUTTON_TYPE_SMALL);
    pLV_Replays_Show->setID("LEVEL_VIEWER_REPLAYS_SHOW");
    pLV_Replays_Show->setContextHelp(CONTEXTHELP_RUN_SELECTED_REPLAY);
    pLV_Replays_List->setEnterButton( pLV_Replays_Show );
    
    /* Build stats window */
    m_pStatsWindow = new UIFrame(m_pMainMenu,220,(drawLib->getDispHeight()*140)/600,GAMETEXT_STATS,drawLib->getDispWidth()-200,drawLib->getDispHeight()-40-(drawLib->getDispHeight()*120)/600-10);      
    m_pStatsWindow->setStyle(UI_FRAMESTYLE_LEFTTAG);
    m_pStatsWindow->setFont(m_Renderer.getSmallFont());
    m_pStatsWindow->makeMinimizable(drawLib->getDispWidth()-17,(drawLib->getDispHeight()*140)/600);
    m_pStatsWindow->setMinimized(true);
    m_pStatsWindow->setContextHelp(CONTEXTHELP_STATS);
    m_pStatsWindow->setPosition(drawLib->getDispWidth()-17,(drawLib->getDispHeight()*140)/600,m_pStatsWindow->getPosition().nWidth,m_pStatsWindow->getPosition().nHeight);
    pSomeText = new UIStatic(m_pStatsWindow,40,0,GAMETEXT_STATISTICS,m_pStatsWindow->getPosition().nWidth-80,36);
    pSomeText->setFont(m_Renderer.getMediumFont());
    
    if(m_pPlayer != NULL)
      m_pStatsReport = m_GameStats.generateReport(m_pPlayer->PlayerName,m_pStatsWindow,30,36,m_pStatsWindow->getPosition().nWidth-45,m_pStatsWindow->getPosition().nHeight-36,m_Renderer.getSmallFont());
    else
      m_pStatsReport = m_GameStats.generateReport("",m_pStatsWindow,30,36,m_pStatsWindow->getPosition().nWidth-45,m_pStatsWindow->getPosition().nHeight-36,m_Renderer.getSmallFont());
    
  }

  /*===========================================================================
  Create menus and hide them
  ===========================================================================*/
  void GameApp::_InitMenus(void) {
    /* TODO: it would obviously be a good idea to put this gui stuff into
       a nice XML file instead. This really stinks */
    
    _InitMenus_PlayingMenus();
    _InitMenus_MainMenu();
    _InitMenus_Others();

    /* Hide menus */
    m_pMainMenu->showWindow(false);
    m_pPauseMenu->showWindow(false);
    m_pJustDeadMenu->showWindow(false);
    m_pProfileEditor->showWindow(false);
    m_pFinishMenu->showWindow(false);
    m_pBestTimes->showWindow(false);
    m_pLevelInfoViewer->showWindow(false);
    m_pLevelPackViewer->showWindow(false);
    
    /* Update options */
    _ImportOptions();
  }


  /*===========================================================================
  Add levels to list (level pack)
  ===========================================================================*/  
  void GameApp::_CreateLevelPackLevelList(void) {
    if(m_pActiveLevelPack == NULL) {
      return;
    } 
    
    UILevelList *pList = (UILevelList *)m_pLevelPackViewer->getChild("LEVELPACK_LEVEL_LIST");    
    pList->setNumeroted(true);

    /* get selected item */
    std::string v_selected_levelName = "";
    if(pList->getSelected() >= 0 && pList->getSelected() < pList->getEntries().size()) {
      UIListEntry *pEntry = pList->getEntries()[pList->getSelected()];
      v_selected_levelName = pEntry->Text[0];
    }

    pList->clear();

    // clear the filter
    UIEdit *pLevelFilterEdit = reinterpret_cast<UIEdit *>(m_pLevelPackViewer->getChild("LEVELPACK_LEVEL_FILTER"));  
    pLevelFilterEdit->setCaption("");
    pList->setFilter("");
  
    /* Obey hints */
    pList->unhideAllColumns();
    if(!m_pActiveLevelPack->ShowTimes()) {
      pList->hideBestTime();
    }
    if(!m_pActiveLevelPack->ShowWebTimes()) {
      pList->hideRoomBestTime();
    }

    if(m_pActiveLevelPack->Sorted()) {
      pList->setSort(true, (int(*)(void*, void*))Level::compareLevel);
    } else {
      pList->setSort(true, (int(*)(void*, void*))Level::compareLevelRandom);
    }

    /* Add levels */
    for(int i=0;i<m_pActiveLevelPack->Levels().size();i++) {
       pList->addLevel(m_pActiveLevelPack->Levels()[i],
		       m_pPlayer,
		       &m_Profiles
#if defined(SUPPORT_WEBACCESS)
		       ,
		       m_pWebHighscores
#endif
		       );
    }

    /* reselect the previous level */
    if(v_selected_levelName != "") {
      int nLevel = 0;
      for(int i=0; i<pList->getEntries().size(); i++) {
        if(pList->getEntries()[i]->Text[0] == v_selected_levelName) {
          nLevel = i;
          break;
        }
      }
      pList->setRealSelected(nLevel);
    }
  }
  
  /*===========================================================================
  Update level info viewer best times list
  ===========================================================================*/  
  void GameApp::_UpdateLevelInfoViewerBestTimes(const std::string &LevelID) {
    UIList *pList = (UIList *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_LIST");
    UIButton *pLV_BestTimes_Personal = (UIButton *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_PERSONAL");
    UIButton *pLV_BestTimes_All = (UIButton *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_ALL");
    UIStatic *pLV_BestTimes_WorldRecord = (UIStatic *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_WORLDRECORD");

    if(pList != NULL && pLV_BestTimes_All != NULL && pLV_BestTimes_Personal != NULL && m_pPlayer != NULL &&
       pLV_BestTimes_WorldRecord != NULL) {
      std::vector<PlayerTimeEntry *> Top10;
      
      /* Good. Personal times or all times? */
      if(pLV_BestTimes_All->getChecked()) {
        Top10 = m_Profiles.createLevelTop10(LevelID);
      }
      else if(pLV_BestTimes_Personal->getChecked()) {
        Top10 = m_Profiles.createPlayerOnlyLevelTop10(m_pPlayer->PlayerName,LevelID);
      }
      
      /* Create list */
      pList->clear();
      for(int i=0;i<Top10.size();i++) {
        UIListEntry *pEntry = pList->addEntry(formatTime(Top10[i]->fFinishTime));
        pEntry->Text.push_back(Top10[i]->PlayerName);
      }            
      
      /* Get record */
      #if defined(SUPPORT_WEBACCESS)
      if(m_bEnableWebHighscores && m_pWebHighscores!=NULL) {
        WebHighscore *pWebHS = m_pWebHighscores->getHighscoreFromLevel(LevelID);
        if(pWebHS != NULL) {
          char cTime[512];
          int n1=0,n2=0,n3=0;
          
          sscanf(pWebHS->getTime().c_str(),"%d:%d:%d",&n1,&n2,&n3);
          sprintf(cTime, std::string("%s: %02d:%02d:%02d (" + std::string(GAMETEXT_BY) + " %s)").c_str(), m_pWebHighscores->getRoomName().c_str(), n1,n2,n3,pWebHS->getPlayerName().c_str());
          pLV_BestTimes_WorldRecord->setCaption(cTime);
        }        
        else {
          pLV_BestTimes_WorldRecord->setCaption(m_pWebHighscores->getRoomName() + ": " + GAMETEXT_WORLDRECORDNA);
	}
      }
      else
      #endif
        pLV_BestTimes_WorldRecord->setCaption("");
    }
  }

  /*===========================================================================
  Update level info viewer replays list
  ===========================================================================*/  
  void GameApp::_UpdateLevelInfoViewerReplays(const std::string &LevelID) {
    UIList *pList = (UIList *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_TAB:LEVEL_VIEWER_REPLAYS_LIST");
    UIButton *pLV_BestTimes_Personal = (UIButton *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_PERSONAL");
    UIButton *pLV_BestTimes_All = (UIButton *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_ALL");
    UIButton *pLV_Replays_Personal = (UIButton *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_TAB:LEVEL_VIEWER_REPLAYS_PERSONAL");
    UIButton *pLV_Replays_All = (UIButton *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_TAB:LEVEL_VIEWER_REPLAYS_ALL");
    UIButton *pLV_Replays_Show = (UIButton *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_TAB:LEVEL_VIEWER_REPLAYS_SHOW");

    if(pList != NULL && pLV_BestTimes_All != NULL && pLV_BestTimes_Personal != NULL && m_pPlayer != NULL &&
       pLV_Replays_Show != NULL) {
      std::vector<ReplayInfo *> *Replays;

      /* Personal or all replays? */
      if(pLV_Replays_All->getChecked()) {
	Replays = m_ReplayList.findReplays("",LevelID);
      }
      else if(pLV_Replays_Personal->getChecked()) {
	Replays = m_ReplayList.findReplays(m_pPlayer->PlayerName,LevelID);
      }
      
      /* Create list */
      pList->clear();
      for(int i=0;i<Replays->size();i++) {
	UIListEntry *pEntry = pList->addEntry((*Replays)[i]->Name);
	pEntry->Text.push_back((*Replays)[i]->Player);
	
	if((*Replays)[i]->fFinishTime < 0)
	  pEntry->Text.push_back("("+ std::string(GAMETEXT_NOTFINISHED) + ")");
	else
	  pEntry->Text.push_back(formatTime((*Replays)[i]->fFinishTime));
      }
      
      /* Clean up */
      delete Replays;
      pLV_Replays_Personal->enableWindow(true);
      pLV_Replays_All->enableWindow(true);
      pLV_Replays_Show->enableWindow(true);
      pList->enableWindow(true);
    }
  }
  
  /*===========================================================================
  Update action key list (options)
  ===========================================================================*/  
  void GameApp::_UpdateActionKeyList(void) {
    UIList *pList = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:CONTROLS_TAB:KEY_ACTION_LIST");
    pList->clear();
    
    UIListEntry *p;
    
    p = pList->addEntry(GAMETEXT_DRIVE); p->Text.push_back(m_Config.getString("KeyDrive1"));
    p = pList->addEntry(GAMETEXT_BRAKE); p->Text.push_back(m_Config.getString("KeyBrake1"));
    p = pList->addEntry(GAMETEXT_FLIPLEFT); p->Text.push_back(m_Config.getString("KeyFlipLeft1"));
    p = pList->addEntry(GAMETEXT_FLIPRIGHT); p->Text.push_back(m_Config.getString("KeyFlipRight1"));
    p = pList->addEntry(GAMETEXT_CHANGEDIR); p->Text.push_back(m_Config.getString("KeyChangeDir1"));
    
    #if defined(ENABLE_ZOOMING)    
    p = pList->addEntry(GAMETEXT_ZOOMIN); p->Text.push_back(m_Config.getString("KeyZoomIn"));
    p = pList->addEntry(GAMETEXT_ZOOMOUT); p->Text.push_back(m_Config.getString("KeyZoomOut"));
    p = pList->addEntry(GAMETEXT_ZOOMINIT); p->Text.push_back(m_Config.getString("KeyZoomInit"));   
    p = pList->addEntry(GAMETEXT_CAMERAMOVEXUP); p->Text.push_back(m_Config.getString("KeyCameraMoveXUp"));
    p = pList->addEntry(GAMETEXT_CAMERAMOVEXDOWN); p->Text.push_back(m_Config.getString("KeyCameraMoveXDown"));
    p = pList->addEntry(GAMETEXT_CAMERAMOVEYUP); p->Text.push_back(m_Config.getString("KeyCameraMoveYUp"));
    p = pList->addEntry(GAMETEXT_CAMERAMOVEYDOWN); p->Text.push_back(m_Config.getString("KeyCameraMoveYDown"));
    p = pList->addEntry(GAMETEXT_AUTOZOOM); p->Text.push_back(m_Config.getString("KeyAutoZoom"));
    #endif
  }
  
  /*===========================================================================
  Update level pack list
  ===========================================================================*/  
  void GameApp::_UpdateLevelPackList(void) {
    UIPackTree *pTree = (UIPackTree *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:PACK_TAB:LEVELPACK_TREE");
    /* get selected item */
    std::string v_selected_packName = pTree->getSelectedEntry();

    pTree->clear();
    
    std::string p_packName;
    
    for(int i=0;i<m_levelsManager.LevelsPacks().size();i++) {
      /* don't show empty packs */
      if(m_levelsManager.LevelsPacks()[i]->Levels().size() != 0) {
				p_packName = m_levelsManager.LevelsPacks()[i]->Name();

				/* the unpackaged pack exists only in debug mode */
				if(p_packName != "" || m_bDebugMode) {
					if(p_packName == "") {
						p_packName = GAMETEXT_UNPACKED_LEVELS_PACK;
					}
					
					pTree->addPack(m_levelsManager.LevelsPacks()[i],
						       m_levelsManager.LevelsPacks()[i]->Group(),
						       getNumberOfFinishedLevelsOfPack(m_levelsManager.LevelsPacks()[i]),
						       m_levelsManager.LevelsPacks()[i]->Levels().size());

				}
			}
    }

    /* reselect the previous pack */
    if(v_selected_packName != "") {
      pTree->setSelectedPackByName(v_selected_packName);
    }
  }
  
  /*===========================================================================
  Update pause menu
  ===========================================================================*/
  void GameApp::_HandlePauseMenu(void) {
    /* Any of the pause menu buttons clicked? */
    for(int i=0;i<m_nNumPauseMenuButtons;i++) {
      if(m_pPauseMenuButtons[i]->getCaption() == GAMETEXT_PLAYNEXT) {
        /* Uhm... is it likely that there's a next level? */
        Level *pLS = &(m_levelsManager.LevelById(m_PlaySpecificLevel));
	m_pPauseMenuButtons[i]->enableWindow(_IsThereANextLevel(pLS));
      }

      if(m_pPauseMenuButtons[i]->isClicked()) {
        if(m_pPauseMenuButtons[i]->getCaption() == GAMETEXT_QUIT) {
          if(m_pQuitMsgBox == NULL)
            m_pQuitMsgBox = m_Renderer.getGUI()->msgBox(GAMETEXT_QUITMESSAGE,
                                                        (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
        }
        else if(m_pPauseMenuButtons[i]->getCaption() == GAMETEXT_ABORT) {
          m_pPauseMenu->showWindow(false);
          m_GameStats.abortedLevel(m_pPlayer->PlayerName,m_MotoGame.getLevelSrc()->Id(),m_MotoGame.getLevelSrc()->Name(),m_MotoGame.getTime());

          m_MotoGame.endLevel();
          m_InputHandler.resetScriptKeyHooks();                     
          m_Renderer.unprepareForNewLevel();

          setState(m_StateAfterPlaying);
          //setState(GS_MENU);          
        }
        else if(m_pPauseMenuButtons[i]->getCaption() == GAMETEXT_RESTART) {
          m_pPauseMenu->showWindow(false);
	  _RestartLevel();
        }
        else if(m_pPauseMenuButtons[i]->getCaption() == GAMETEXT_PLAYNEXT) {
	  Level *pLS = &(m_levelsManager.LevelById(m_PlaySpecificLevel));
	  std::string NextLevel = _DetermineNextLevel(pLS);
	  if(NextLevel != "") {        
	    m_pPauseMenu->showWindow(false);              
	    m_GameStats.abortedLevel(m_pPlayer->PlayerName,m_MotoGame.getLevelSrc()->Id(),m_MotoGame.getLevelSrc()->Name(),m_MotoGame.getTime());
	    m_MotoGame.endLevel();
	    m_InputHandler.resetScriptKeyHooks();                     
	    m_Renderer.unprepareForNewLevel();                    
	    
	    m_PlaySpecificLevel = NextLevel;              
	    
	    setPrePlayAnim(true);
              setState(GS_PREPLAYING);
          }
        }
        else if(m_pPauseMenuButtons[i]->getCaption() == GAMETEXT_RESUME) {
          m_pPauseMenu->showWindow(false);
          m_State = GS_PLAYING; /* no don't use setState() for this. Old code, depends on madness */
        }

        /* Don't process this clickin' more than once */
        m_pPauseMenuButtons[i]->setClicked(false);
      }
    }
  }

  /*===========================================================================
  Update finish menu
  ===========================================================================*/
  void GameApp::_HandleFinishMenu(void) {
    /* Is savereplay box open? */
    if(m_pSaveReplayMsgBox != NULL) {
      UIMsgBoxButton Clicked = m_pSaveReplayMsgBox->getClicked();
      if(Clicked != UI_MSGBOX_NOTHING) {
        std::string Name = m_pSaveReplayMsgBox->getTextInput();
      
        delete m_pSaveReplayMsgBox;
        m_pSaveReplayMsgBox = NULL;

        if(Clicked == UI_MSGBOX_OK) {
          _SaveReplay(Name);
        }    
      }
    }
    
    /* Any of the finish menu buttons clicked? */
    for(int i=0;i<m_nNumFinishMenuButtons;i++) {
      if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_SAVEREPLAY) {
        /* Have we recorded a replay? If not then disable the "Save Replay" button */
        if(m_pReplay == NULL) {
          m_pFinishMenuButtons[i]->enableWindow(false);
        }
        else {
          m_pFinishMenuButtons[i]->enableWindow(true);
        }
      }

      if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_PLAYNEXT) {
        /* Uhm... is it likely that there's a next level? */
	Level *pLS = &(m_levelsManager.LevelById(m_PlaySpecificLevel));
	m_pFinishMenuButtons[i]->enableWindow(_IsThereANextLevel(pLS));
      }
      
      if(m_pFinishMenuButtons[i]->isClicked()) {
        if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_QUIT) {
          if(m_pQuitMsgBox == NULL)
            m_pQuitMsgBox = m_Renderer.getGUI()->msgBox(GAMETEXT_QUITMESSAGE,
                                                        (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
        }
        else if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_PLAYNEXT) {
	  Level *pLS = &(m_levelsManager.LevelById(m_PlaySpecificLevel));
	  std::string NextLevel = _DetermineNextLevel(pLS);
	  if(NextLevel != "") {        
	    m_pFinishMenu->showWindow(false);
	    m_Renderer.hideMsgNewHighscore();
	    m_pBestTimes->showWindow(false);
	    m_MotoGame.endLevel();
	    m_InputHandler.resetScriptKeyHooks();                     
	    m_Renderer.unprepareForNewLevel();                    
	    
	    m_PlaySpecificLevel = NextLevel;
	    
	    setPrePlayAnim(true);
	    setState(GS_PREPLAYING);                               
          }
        }
        else if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_SAVEREPLAY) {
          if(m_pReplay != NULL) {
            if(m_pSaveReplayMsgBox == NULL) {
              m_pSaveReplayMsgBox = m_Renderer.getGUI()->msgBox(std::string(GAMETEXT_ENTERREPLAYNAME) + ":",
                                                                (UIMsgBoxButton)(UI_MSGBOX_OK|UI_MSGBOX_CANCEL),
                                                                true);
              m_pSaveReplayMsgBox->setTextInputFont(m_Renderer.getMediumFont());
	      m_pSaveReplayMsgBox->setTextInput(Replay::giveAutomaticName());
            }          
          }
        }
#if defined(SUPPORT_WEBACCESS) 
        else if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_UPLOAD_HIGHSCORE) {
	  _UploadHighscore("Latest");
        }	
#endif
        else if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_TRYAGAIN) {
          Level *pCurLevel = m_MotoGame.getLevelSrc();
          m_PlaySpecificLevel = pCurLevel->Id();
          m_pFinishMenu->showWindow(false);
	  m_Renderer.hideMsgNewHighscore();
          m_pBestTimes->showWindow(false);

	  _RestartLevel();
        }
        else if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_ABORT) {
          m_pFinishMenu->showWindow(false);
	  m_Renderer.hideMsgNewHighscore();
          m_pBestTimes->showWindow(false);
          m_MotoGame.endLevel();
          m_InputHandler.resetScriptKeyHooks();                     
          m_Renderer.unprepareForNewLevel();
//          setState(GS_MENU);          
          setState(m_StateAfterPlaying);                   
        }

        /* Don't process this clickin' more than once */
        m_pFinishMenuButtons[i]->setClicked(false);
      }
    }
  }

  /*===========================================================================
  Update level info viewer
  ===========================================================================*/
  void GameApp::_HandleLevelInfoViewer(void) {
    /* Get buttons */
    UIButton *pOKButton = reinterpret_cast<UIButton *>(m_pLevelInfoViewer->getChild("LEVEL_VIEWER_OK_BUTTON"));    
    UIButton *pLV_BestTimes_Personal = (UIButton *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_PERSONAL");
    UIButton *pLV_BestTimes_All = (UIButton *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_BESTTIMES_TAB:LEVEL_VIEWER_BESTTIMES_ALL");
    UIButton *pLV_Replays_Personal = (UIButton *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_TAB:LEVEL_VIEWER_REPLAYS_PERSONAL");
    UIButton *pLV_Replays_All = (UIButton *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_TAB:LEVEL_VIEWER_REPLAYS_ALL");
    UIButton *pLV_Replays_Show = (UIButton *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_TAB:LEVEL_VIEWER_REPLAYS_SHOW");
    UIList *pLV_Replays_List = (UIList *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_REPLAYS_TAB:LEVEL_VIEWER_REPLAYS_LIST");

    /* Check buttons */
    if(pOKButton->isClicked()) {
      m_State = GS_MENU;
      m_pLevelInfoViewer->showWindow(false);
      m_pMainMenu->enableChildren(true);
      m_pMainMenu->enableWindow(true);
    }   
    
    if(pLV_BestTimes_All->isClicked() || pLV_BestTimes_Personal->isClicked()) {
      _UpdateLevelInfoViewerBestTimes(m_LevelInfoViewerLevel);
    }         

    if(pLV_Replays_All->isClicked() || pLV_Replays_Personal->isClicked()) {
      _UpdateLevelInfoViewerReplays(m_LevelInfoViewerLevel);
    }         

    if(pLV_Replays_Show->isClicked()) {
      /* Show replay */
      if(pLV_Replays_List->getSelected() >= 0 && pLV_Replays_List->getSelected() < pLV_Replays_List->getEntries().size()) {
        UIListEntry *pListEntry = pLV_Replays_List->getEntries()[pLV_Replays_List->getSelected()];
        if(pListEntry != NULL && !pListEntry->Text.empty()) {
          /* Do it captain */
          pLV_Replays_Show->setClicked(false);
          m_pLevelInfoViewer->showWindow(false);
          m_pMainMenu->enableChildren(true);
          m_pMainMenu->enableWindow(true);
          m_pMainMenu->showWindow(false);
          m_PlaySpecificReplay = pListEntry->Text[0];
          setState(GS_REPLAYING);
        }
      }
    }
  }

  /*===========================================================================
  Update level pack viewer
  ===========================================================================*/
  void GameApp::_HandleLevelPackViewer(void) {    
    /* Get buttons and list */
    UIButton *pCancelButton = reinterpret_cast<UIButton *>(m_pLevelPackViewer->getChild("LEVELPACK_CANCEL_BUTTON"));
    UIButton *pPlayButton = reinterpret_cast<UIButton *>(m_pLevelPackViewer->getChild("LEVELPACK_PLAY_BUTTON"));
    UIButton *pLevelInfoButton = reinterpret_cast<UIButton *>(m_pLevelPackViewer->getChild("LEVELPACK_INFO_BUTTON"));
    UIButton *pLevelAddToFavoriteButton = reinterpret_cast<UIButton *>(m_pLevelPackViewer->getChild("LEVELPACK_ADDTOFAVORITE_BUTTON"));
    UIButton *pLevelRandomizeButton = reinterpret_cast<UIButton *>(m_pLevelPackViewer->getChild("LEVELPACK_RANDOMIZE_BUTTON"));
    UILevelList *pList = (UILevelList *)m_pLevelPackViewer->getChild("LEVELPACK_LEVEL_LIST");
		UIEdit *pLevelFilterEdit = reinterpret_cast<UIEdit *>(m_pLevelPackViewer->getChild("LEVELPACK_LEVEL_FILTER"));   

		/* check filter */
		if(pLevelFilterEdit != NULL) {
			if(pLevelFilterEdit->hasChanged()) {
				pLevelFilterEdit->setHasChanged(false);
				pList->setFilter(pLevelFilterEdit->getCaption());
			}
		}

    /* Check buttons */
    if(pCancelButton!=NULL && pCancelButton->isClicked()) {
      pCancelButton->setClicked(false);
      
      m_State = GS_MENU;
      m_pLevelPackViewer->showWindow(false);
      m_pMainMenu->enableChildren(true);
      m_pMainMenu->enableWindow(true);
    }
    
    if(pPlayButton!=NULL && pPlayButton->isClicked()) {
      pPlayButton->setClicked(false);

	Level *pLevelSrc = pList->getSelectedLevel();
	if(pLevelSrc != NULL) {
	  m_pLevelPackViewer->showWindow(false);
	  m_pMainMenu->showWindow(false);      
	  m_PlaySpecificLevel = pLevelSrc->Id();        
	  m_StateAfterPlaying = GS_LEVELPACK_VIEWER;
	  m_currentPlayingList = pList;
	  setState(GS_PREPLAYING);   
	}
    }

    if(pLevelAddToFavoriteButton!=NULL && pLevelAddToFavoriteButton->isClicked()) {
      pLevelAddToFavoriteButton->setClicked(false);
      
      Level *pLevelSrc = pList->getSelectedLevel();
      if(pLevelSrc != NULL) {
	m_levelsManager.addToFavorite(pLevelSrc);
	_UpdateLevelPackList();
	_UpdateLevelLists();
      }
    }

    if(pLevelRandomizeButton!=NULL && pLevelRandomizeButton->isClicked()) {
      pLevelRandomizeButton->setClicked(false);
      
      pList->randomize();
    }

    /* level menu : */
    /* any list clicked ? */
    if(pList->isChanged()) {
      pList->setChanged(false);
      Level *pLevelSrc = pList->getSelectedLevel();
      if(pLevelSrc != NULL) {
#if defined(SUPPORT_WEBACCESS)
	setLevelInfoFrameBestPlayer(pLevelSrc->Id(),
				    m_pPackLevelInfoFrame,
				    m_pPackLevelInfoViewReplayButton,
				    m_pPackBestPlayerText
				    );
#endif
      }
    }

#if defined(SUPPORT_WEBACCESS)
    /* view highscore button clicked */
    if(m_pPackLevelInfoViewReplayButton->isClicked()) {
      m_pPackLevelInfoViewReplayButton->setClicked(false);
      viewHighscoreOf();
      m_pLevelPackViewer->showWindow(false);
      m_pMainMenu->showWindow(false);      
      m_StateAfterPlaying = GS_LEVELPACK_VIEWER;
      setState(GS_REPLAYING); 
    }
#endif

    if(pLevelInfoButton!=NULL && pLevelInfoButton->isClicked()) {
      pLevelInfoButton->setClicked(false);

      Level *pLevelSrc = pList->getSelectedLevel();
      if(pLevelSrc != NULL) {
	
        /* === OPEN LEVEL INFO VIEWER === */      
        /* Set information */
        UIStatic *pLevelName = (UIStatic *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TITLE");
        
        if(pLevelName != NULL) pLevelName->setCaption(pLevelSrc->Name());

        UIStatic *pGeneralInfo_LevelPack = (UIStatic *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_GENERALINFO_TAB:LEVEL_VIEWER_INFO_LEVELPACK");
        UIStatic *pGeneralInfo_LevelName = (UIStatic *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_GENERALINFO_TAB:LEVEL_VIEWER_INFO_LEVELNAME");
        UIStatic *pGeneralInfo_Author = (UIStatic *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_GENERALINFO_TAB:LEVEL_VIEWER_INFO_AUTHOR");
        UIStatic *pGeneralInfo_Date = (UIStatic *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_GENERALINFO_TAB:LEVEL_VIEWER_INFO_DATE");
        UIStatic *pGeneralInfo_Description = (UIStatic *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_GENERALINFO_TAB:LEVEL_VIEWER_INFO_DESCRIPTION");

        if(pGeneralInfo_LevelPack != NULL) pGeneralInfo_LevelPack->setCaption(std::string(GAMETEXT_LEVELPACK) + ": " + pLevelSrc->Pack());
        if(pGeneralInfo_LevelName != NULL) pGeneralInfo_LevelName->setCaption(std::string(GAMETEXT_LEVELNAME) + ": " + pLevelSrc->Name());
        if(pGeneralInfo_Author != NULL) pGeneralInfo_Author->setCaption(std::string(GAMETEXT_AUTHOR) + ": " + pLevelSrc->Author());
        if(pGeneralInfo_Date != NULL) pGeneralInfo_Date->setCaption(std::string(GAMETEXT_DATE) + ": " + pLevelSrc->Date());
        if(pGeneralInfo_Description != NULL) pGeneralInfo_Description->setCaption(std::string(GAMETEXT_DESCRIPTION) + ": " + pLevelSrc->Description());
            
        _UpdateLevelInfoViewerBestTimes(m_LevelInfoViewerLevel = pLevelSrc->Id());
        _UpdateLevelInfoViewerReplays(m_LevelInfoViewerLevel);
        
        /* Nice. Open the level info viewer */
        pLevelInfoButton->setActive(false);
        m_pLevelPackViewer->showWindow(false);
        m_pLevelInfoViewer->showWindow(true);
        m_pMainMenu->enableChildren(false);
        m_pMainMenu->enableWindow(false);
        m_State = GS_LEVEL_INFO_VIEWER;      
      }
    }
  }

#if defined(SUPPORT_WEBACCESS)
  /*===========================================================================
  Update internet connection editor
  ===========================================================================*/
  void GameApp::_InitWebConf(void) {
    /* Get some pointers */
    UIButton *pDirectConn = (UIButton *)m_pWebConfEditor->getChild("DIRECTCONN");
    UIButton *pHTTPConn = (UIButton *)m_pWebConfEditor->getChild("HTTPPROXY");
    UIButton *pSOCKS4Conn = (UIButton *)m_pWebConfEditor->getChild("SOCKS4PROXY");
    UIButton *pSOCKS5Conn = (UIButton *)m_pWebConfEditor->getChild("SOCKS5PROXY");
    UIButton *pConnOK = (UIButton *)m_pWebConfEditor->getChild("PROXYOK");
    UIEdit *pServer = (UIEdit *)m_pWebConfEditor->getChild("SUBFRAME:SERVEREDIT");
    UIEdit *pPort = (UIEdit *)m_pWebConfEditor->getChild("SUBFRAME:PORTEDIT");    
    UIEdit *pLogin    = (UIEdit *)m_pWebConfEditor->getChild("SUBFRAME:LOGINEDIT"); 
    UIEdit *pPassword = (UIEdit *)m_pWebConfEditor->getChild("SUBFRAME:PASSWORDEDIT");     

    pDirectConn->setChecked(false);
    pHTTPConn->setChecked(false);
    pSOCKS4Conn->setChecked(false);
    pSOCKS5Conn->setChecked(false);

    /* Read config */
    pServer->setCaption(m_Config.getString("ProxyServer"));
    char cBuf[256] = "";
    int n = m_Config.getInteger("ProxyPort");
    if(n > 0) sprintf(cBuf,"%d",n);
    pPort->setCaption(cBuf);
    pLogin->setCaption(m_Config.getString("ProxyAuthUser"));
    pPassword->setCaption(m_Config.getString("ProxyAuthPwd"));
    
    std::string s = m_Config.getString("ProxyType");
    if(s == "HTTP") pHTTPConn->setChecked(true);
    else if(s == "SOCKS4") pSOCKS4Conn->setChecked(true);
    else if(s == "SOCKS5") pSOCKS5Conn->setChecked(true);
    else pDirectConn->setChecked(true);
    
    /* Make sure OK button is activated */
    pConnOK->makeActive();
  }
#endif  

#if defined(SUPPORT_WEBACCESS) 
  void GameApp::_HandleWebConfEditor(void) {
    /* Get some pointers */
    UIButton *pDirectConn = (UIButton *)m_pWebConfEditor->getChild("DIRECTCONN");
    UIButton *pHTTPConn = (UIButton *)m_pWebConfEditor->getChild("HTTPPROXY");
    UIButton *pSOCKS4Conn = (UIButton *)m_pWebConfEditor->getChild("SOCKS4PROXY");
    UIButton *pSOCKS5Conn = (UIButton *)m_pWebConfEditor->getChild("SOCKS5PROXY");
    UIButton *pConnOK = (UIButton *)m_pWebConfEditor->getChild("PROXYOK");
    UIEdit *pServer = (UIEdit *)m_pWebConfEditor->getChild("SUBFRAME:SERVEREDIT");
    UIEdit *pPort = (UIEdit *)m_pWebConfEditor->getChild("SUBFRAME:PORTEDIT");    
    UIEdit *pLogin    = (UIEdit *)m_pWebConfEditor->getChild("SUBFRAME:LOGINEDIT"); 
    UIEdit *pPassword = (UIEdit *)m_pWebConfEditor->getChild("SUBFRAME:PASSWORDEDIT");  

    /* The yes/no box open? */
    if(m_pWebConfMsgBox != NULL) {
      UIMsgBoxButton Clicked = m_pWebConfMsgBox->getClicked();
      if(Clicked != UI_MSGBOX_NOTHING) {
        if(Clicked == UI_MSGBOX_YES) {
          /* Show the actual web config editor */
          m_bEnableWebHighscores = true;
          m_pWebConfEditor->showWindow(true);          
        }
        else {
          /* No internet connection thank you */
          m_bEnableWebHighscores = false;
          setState(GS_MENU);
        }
	m_Config.setBool("WebHighscores", m_bEnableWebHighscores);
        
        m_Config.setBool("WebConfAtInit",false);
        delete m_pWebConfMsgBox;
        m_pWebConfMsgBox=NULL;
      }
    }
    else {
      /* OK button pressed? */
      if(pConnOK->isClicked()) {
        pConnOK->setClicked(false);
        
        /* Save settings */
        std::string ProxyType = "";
        if(pHTTPConn->getChecked()) ProxyType = "HTTP";
        else if(pSOCKS4Conn->getChecked()) ProxyType = "SOCKS4";
        else if(pSOCKS5Conn->getChecked()) ProxyType = "SOCKS5";
        
        m_Config.setString("ProxyType",ProxyType);
        
        if(ProxyType != "") {
          int nPort = atoi(pPort->getCaption().c_str());
          if(nPort > 0)
            m_Config.setInteger("ProxyPort",nPort);
          else
            m_Config.setInteger("ProxyPort",-1);          
            
          m_Config.setString("ProxyServer",pServer->getCaption());
	  m_Config.setString("ProxyAuthUser",pLogin->getCaption());
	  m_Config.setString("ProxyAuthPwd" ,pPassword->getCaption());
        }

        m_pWebConfEditor->showWindow(false);
        m_pMainMenu->enableChildren(true);
        m_pMainMenu->enableWindow(true);
        setState(GS_MENU);

        _ConfigureProxy();

        if(!m_bWebHighscoresUpdatedThisSession) {        
	  try {
	    _UpdateWebHighscores(false);
	    _UpgradeWebHighscores();  
	    _UpdateWebLevels(false);

	    _UpdateLevelsLists();
	  } catch(Exception &e) {
	    notifyMsg(GAMETEXT_FAILEDDLHIGHSCORES);
	  }
	}
	
	      m_Config.setBool("WebHighscores",true);
	
        /* Update options */
        _ImportOptions();
        
      }      

      /* Direct connection selected? If so, no need to enabled proxy editing */
      if(pDirectConn->getChecked()) {
        pServer->enableWindow(false);
        pPort->enableWindow(false);
	pLogin->enableWindow(false);
	pPassword->enableWindow(false);
      }            
      else {
        pServer->enableWindow(true);
        pPort->enableWindow(true);
	pLogin->enableWindow(true);
	pPassword->enableWindow(true);
      }
    }
  }
#endif

  /*===========================================================================
  Update profile editor
  ===========================================================================*/
  void GameApp::_HandleProfileEditor(void) {    
    /* Is newplayer box open? */
    if(m_pNewProfileMsgBox != NULL) {
      UIMsgBoxButton Clicked = m_pNewProfileMsgBox->getClicked();
      if(Clicked != UI_MSGBOX_NOTHING) {
        if(Clicked == UI_MSGBOX_OK) {
          /* Create new profile */
          std::string PlayerName = m_pNewProfileMsgBox->getTextInput();
          if(!m_Profiles.createProfile(PlayerName)) {
            /* TODO: error message */
          }
          else
            _CreateProfileList();
        }
        
        delete m_pNewProfileMsgBox;
        m_pNewProfileMsgBox = NULL;
      }
    }
    /* What about the delete player box? */
    if(m_pDeleteProfileMsgBox != NULL) {
      UIMsgBoxButton Clicked = m_pDeleteProfileMsgBox->getClicked();
      if(Clicked != UI_MSGBOX_NOTHING) {
        if(Clicked == UI_MSGBOX_YES) {
          /* Delete selected profile */
          UIList *pList = reinterpret_cast<UIList *>(m_pProfileEditor->getChild("PROFILE_LIST"));
          if(pList != NULL) {
            int nIdx = pList->getSelected();
            if(nIdx >= 0 && nIdx < pList->getEntries().size()) {
              UIListEntry *pEntry = pList->getEntries()[nIdx];
      
              bool bSelNew = false;        
              if(m_pPlayer != NULL && pEntry->Text[0] == m_pPlayer->PlayerName) {
                bSelNew = true;
              }
              
              m_Profiles.destroyProfile(pEntry->Text[0]);
              pList->setRealSelected(0);

              if(bSelNew) {
                m_pPlayer = m_Profiles.getProfile(pEntry->Text[0]);
              }

              _CreateProfileList();              
            }
          }
        }
        delete m_pDeleteProfileMsgBox;
        m_pDeleteProfileMsgBox = NULL;
      }      
    }
  
    /* Get buttons */
    UIButton *pUseButton = reinterpret_cast<UIButton *>(m_pProfileEditor->getChild("USEPROFILE_BUTTON"));
    UIButton *pDeleteButton = reinterpret_cast<UIButton *>(m_pProfileEditor->getChild("DELETEPROFILE_BUTTON"));
    UIButton *pNewButton = reinterpret_cast<UIButton *>(m_pProfileEditor->getChild("NEWPROFILE_BUTTON"));
    UIButton *pCancelButton = reinterpret_cast<UIButton *>(m_pProfileEditor->getChild("CANCEL_BUTTON"));
    
    /* Check them */
    if(pUseButton->isClicked()) {      
      UIList *pList = reinterpret_cast<UIList *>(m_pProfileEditor->getChild("PROFILE_LIST"));
      if(pList != NULL) {
        int nIdx = pList->getSelected();
        if(nIdx >= 0 && nIdx < pList->getEntries().size()) {
          UIListEntry *pEntry = pList->getEntries()[nIdx];
          
          m_pPlayer = m_Profiles.getProfile(pEntry->Text[0]);
        }
      }      
      
      if(m_pPlayer == NULL) throw Exception("failed to set profile");

      _UpdateLevelsLists();
                        
      UIStatic *pPlayerTag = reinterpret_cast<UIStatic *>(m_pMainMenu->getChild("PLAYERTAG"));
      if(pPlayerTag) {
        pPlayerTag->setCaption(std::string(GAMETEXT_CURPLAYER) + ": " + m_pPlayer->PlayerName);
      }                   
      
      _UpdateReplaysList();
      
      m_pProfileEditor->showWindow(false);

#if defined(SUPPORT_WEBACCESS)       
      /* Should we jump to the web config now? */
      if(m_Config.getBool("WebConfAtInit")) {
        _InitWebConf();
        setState(GS_EDIT_WEBCONFIG);
      }
      else
#endif
      {
        m_pMainMenu->enableChildren(true);
        m_pMainMenu->enableWindow(true);

        setState(GS_MENU);
      }

    }    
    else if(pDeleteButton->isClicked()) {
      if(m_pDeleteProfileMsgBox == NULL)
        m_pDeleteProfileMsgBox = m_Renderer.getGUI()->msgBox(GAMETEXT_DELETEPLAYERMESSAGE,
                                                          (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
    }
    else if(pNewButton->isClicked()) {
      if(m_pNewProfileMsgBox == NULL) {
        m_pNewProfileMsgBox = m_Renderer.getGUI()->msgBox(std::string(GAMETEXT_ENTERPLAYERNAME) + ":",
                                                          (UIMsgBoxButton)(UI_MSGBOX_OK|UI_MSGBOX_CANCEL),
                                                          true);
        m_pNewProfileMsgBox->setTextInputFont(m_Renderer.getMediumFont());                                                          
      }
    }
    else if(pCancelButton->isClicked()) {
      if(m_pMainMenu->isHidden()) {
        quit();
      }
      else {
        m_State = GS_MENU;
        m_pProfileEditor->showWindow(false);
        m_pMainMenu->enableChildren(true);
        m_pMainMenu->enableWindow(true);
                        
        UIStatic *pPlayerTag = reinterpret_cast<UIStatic *>(m_pMainMenu->getChild("PLAYERTAG"));
        if(pPlayerTag) {
          if(m_pPlayer == NULL) {
            if(m_Profiles.getProfiles().empty()) throw Exception("no valid profile");
            m_pPlayer = m_Profiles.getProfiles()[0];

	    _UpdateLevelsLists();
          }
        
          pPlayerTag->setCaption(std::string(GAMETEXT_CURPLAYER) + ": " + m_pPlayer->PlayerName);
        }       
      }
    }
  }
  
  /*===========================================================================
  Update just-dead menu
  ===========================================================================*/
  void GameApp::_HandleJustDeadMenu(void) {
    /* Is savereplay box open? */
    if(m_pSaveReplayMsgBox != NULL) {
      UIMsgBoxButton Clicked = m_pSaveReplayMsgBox->getClicked();
      if(Clicked != UI_MSGBOX_NOTHING) {
        std::string Name = m_pSaveReplayMsgBox->getTextInput();
      
        delete m_pSaveReplayMsgBox;
        m_pSaveReplayMsgBox = NULL;

        if(Clicked == UI_MSGBOX_OK) {
          _SaveReplay(Name);
        }        
      }
    }
    
    /* Any of the just-dead menu buttons clicked? */
    for(int i=0;i<m_nNumJustDeadMenuButtons;i++) {
      if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_SAVEREPLAY) {
        /* Have we recorded a replay? If not then disable the "Save Replay" button */
        if(m_pReplay == NULL) {
          m_pJustDeadMenuButtons[i]->enableWindow(false);
        }
        else {
          m_pJustDeadMenuButtons[i]->enableWindow(true);
        }
      }    

      if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_PLAYNEXT) {
        /* Uhm... is it likely that there's a next level? */
	Level *pLS = &(m_levelsManager.LevelById(m_PlaySpecificLevel));
	m_pJustDeadMenuButtons[i]->enableWindow(_IsThereANextLevel(pLS));
      }
      
      if(m_pJustDeadMenuButtons[i]->isClicked()) {
        if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_QUIT) {
          if(m_pQuitMsgBox == NULL)
            m_pQuitMsgBox = m_Renderer.getGUI()->msgBox(GAMETEXT_QUITMESSAGE,
                                                        (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
        }
        else if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_TRYAGAIN) {

          m_pJustDeadMenu->showWindow(false);
	  _RestartLevel();
        }
        else if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_PLAYNEXT) {
	  Level *pLS = &(m_levelsManager.LevelById(m_PlaySpecificLevel));
	  std::string NextLevel = _DetermineNextLevel(pLS);
	  if(NextLevel != "") {        
	    m_pJustDeadMenu->showWindow(false);
	    m_MotoGame.endLevel();
	    m_InputHandler.resetScriptKeyHooks();                     
	    m_Renderer.unprepareForNewLevel();                    
	    
	    m_PlaySpecificLevel = NextLevel;
	    
	    setPrePlayAnim(true);
	    setState(GS_PREPLAYING);                               
          }
        }
        else if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_SAVEREPLAY) {
          if(m_pReplay != NULL) {
            if(m_pSaveReplayMsgBox == NULL) {
              m_pSaveReplayMsgBox = m_Renderer.getGUI()->msgBox(std::string(GAMETEXT_ENTERREPLAYNAME) + ":",
                                                                (UIMsgBoxButton)(UI_MSGBOX_OK|UI_MSGBOX_CANCEL),
                                                                true);
              m_pSaveReplayMsgBox->setTextInputFont(m_Renderer.getMediumFont());                               
	      m_pSaveReplayMsgBox->setTextInput(Replay::giveAutomaticName());
            }          
          }
        }
        else if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_ABORT) {
          m_pJustDeadMenu->showWindow(false);
          m_MotoGame.endLevel();
          m_InputHandler.resetScriptKeyHooks();                     
          m_Renderer.unprepareForNewLevel();
          //setState(GS_MENU);
          setState(m_StateAfterPlaying);
        }

        /* Don't process this clickin' more than once */
        m_pJustDeadMenuButtons[i]->setClicked(false);
      }
    }
  }

  /*===========================================================================
  Update main menu
  ===========================================================================*/
  void GameApp::_HandleMainMenu(void) {
    /* Any of the main menu buttons clicked? */
    for(int i=0;i<m_nNumMainMenuButtons;i++) {
      if(m_pMainMenuButtons[i]->isClicked()) {
        if(m_pMainMenuButtons[i]->getCaption() == GAMETEXT_LEVELS) {
#if defined(SUPPORT_WEBACCESS)
	  m_pLevelInfoFrame->showWindow(false);
#endif
          m_pOptionsWindow->showWindow(false);
          m_pHelpWindow->showWindow(false);
          m_pReplaysWindow->showWindow(false);
          m_pLevelPacksWindow->showWindow(true);                    
          
          /* Make sure all level packs are listed */
          _UpdateLevelPackList();
        }
        else if(m_pMainMenuButtons[i]->getCaption() == GAMETEXT_OPTIONS) {
          if(m_pOptionsWindow->isHidden()) _ImportOptions();        
#if defined(SUPPORT_WEBACCESS)
	  m_pLevelInfoFrame->showWindow(false);
#endif
          m_pOptionsWindow->showWindow(true);
          m_pHelpWindow->showWindow(false);
          m_pReplaysWindow->showWindow(false);
          m_pLevelPacksWindow->showWindow(false);                    
        }
        else if(m_pMainMenuButtons[i]->getCaption() == GAMETEXT_HELP) {
#if defined(SUPPORT_WEBACCESS)
	  m_pLevelInfoFrame->showWindow(false);
#endif
          m_pOptionsWindow->showWindow(false);
          m_pHelpWindow->showWindow(true);
          m_pReplaysWindow->showWindow(false);
          m_pLevelPacksWindow->showWindow(false);                    

	  try {
	    m_levelsManager.LevelById("tut1");
	  } catch(Exception &e) {
	    /* Tutorial not found, disable button */
	    UIButton *pTutButton = (UIButton *)m_pHelpWindow->getChild("HELP_TUTORIAL_BUTTON");
	    pTutButton->enableWindow(false);
	  }
        }
        else if(m_pMainMenuButtons[i]->getCaption() == GAMETEXT_REPLAYS) {
          if(m_pReplaysWindow->isHidden()) _UpdateReplaysList();
#if defined(SUPPORT_WEBACCESS)
	  m_pLevelInfoFrame->showWindow(false);
#endif
          m_pOptionsWindow->showWindow(false);
          m_pHelpWindow->showWindow(false);
          m_pReplaysWindow->showWindow(true);
          m_pLevelPacksWindow->showWindow(false);                    
        }
        else if(m_pMainMenuButtons[i]->getCaption() == GAMETEXT_QUIT) {
          if(m_pQuitMsgBox == NULL)
            m_pQuitMsgBox = m_Renderer.getGUI()->msgBox(GAMETEXT_QUITMESSAGE,
                                                        (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
        }
      }
    }

#if defined(SUPPORT_WEBACCESS)
    /* view highscore button clicked */
    if(m_pLevelInfoViewReplayButton->isClicked()) {
      viewHighscoreOf();
      m_pMainMenu->showWindow(false);
      m_StateAfterPlaying = GS_MENU;
      setState(GS_REPLAYING);
      m_pLevelInfoViewReplayButton->setClicked(false);
    }
#endif

    /* level menu : */
    /* any list clicked ? */
    if(m_pAllLevelsList->isChanged()) {
      Level *pLevelSrc = m_pAllLevelsList->getSelectedLevel();
      if(pLevelSrc != NULL) {
#if defined(SUPPORT_WEBACCESS)
	setLevelInfoFrameBestPlayer(pLevelSrc->Id(),
				    m_pLevelInfoFrame,
				    m_pLevelInfoViewReplayButton,
				    m_pBestPlayerText
				    );
#endif
      }
      m_pAllLevelsList->setChanged(false);
    }

#if defined(SUPPORT_WEBACCESS)
    if(m_pPlayNewLevelsList->isChanged()) {
      Level *pLevelSrc = m_pPlayNewLevelsList->getSelectedLevel();
      if(pLevelSrc != NULL) {
	setLevelInfoFrameBestPlayer(pLevelSrc->Id(),
				    m_pLevelInfoFrame,
				    m_pLevelInfoViewReplayButton,
				    m_pBestPlayerText
				    );
      }
      m_pPlayNewLevelsList->setChanged(false);
    }
#endif

    /* tab of level clicked ? */
    if(m_pLevelPackTabs->isChanged()) {
#if defined(SUPPORT_WEBACCESS)
      m_pLevelInfoFrame->showWindow(false);      
#endif
      m_pLevelPackTabs->setChanged(false);
    }

    /* Delete replay msgbox? */
    if(m_pDeleteReplayMsgBox != NULL) {
      UIMsgBoxButton Clicked = m_pDeleteReplayMsgBox->getClicked();
      if(Clicked != UI_MSGBOX_NOTHING) {
        if(Clicked == UI_MSGBOX_YES) {
          /* Delete selected replay */
          UIList *pList = reinterpret_cast<UIList *>(m_pReplaysWindow->getChild("REPLAY_LIST"));
          if(pList != NULL) {
            int nIdx = pList->getSelected();
            if(nIdx >= 0 && nIdx < pList->getEntries().size()) {
              UIListEntry *pEntry = pList->getEntries()[nIdx];
              if(pEntry != NULL) {
		Replay::deleteReplay(pEntry->Text[0]);
		m_ReplayList.delReplay(pEntry->Text[0]);
                _UpdateReplaysList();
              }
            }
          }
        }
        delete m_pDeleteReplayMsgBox;
        m_pDeleteReplayMsgBox = NULL;
      }      
    }
    
    /* Change player */
    UIButton *pChangePlayerButton = (UIButton *)m_pMainMenu->getChild("CHANGEPLAYERBUTTON");
    UIStatic *pPlayerText = (UIStatic *)m_pMainMenu->getChild("PLAYERTAG");
    if(pChangePlayerButton->isClicked()) {
      /* Open profile editor */
      m_pProfileEditor->showWindow(true);
      m_pMainMenu->enableChildren(false);
      m_pMainMenu->enableWindow(false);
      m_State = GS_EDIT_PROFILES;
      return;
    }

#if defined(SUPPORT_WEBACCESS)
    UIStatic *pNewLevelText = (UIStatic *)m_pMainMenu->getChild("NEWLEVELAVAILBLE");
    if(m_bWebLevelsToDownload) {
      pNewLevelText->showWindow(true);
      if(m_pNewLevelsAvailIcon == NULL)
        pNewLevelText->setCaption(GAMETEXT_NEWLEVELS_AVAIBLE);
      else {
        /* Nice we've got a fancy image to display instead... do that! */
        pNewLevelText->setBackground(m_pNewLevelsAvailIcon);
      }      
    } else {
      pNewLevelText->showWindow(false);
    }
#endif

    /* LEVEL PACKS */
    UIButton *pOpenButton = (UIButton *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:PACK_TAB:LEVELPACK_OPEN_BUTTON");
    UIPackTree *pLevelPackTree = (UIPackTree *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:PACK_TAB:LEVELPACK_TREE");
    if(pOpenButton!=NULL && pLevelPackTree!=NULL && pOpenButton->isClicked()) {
      /* Open level pack viewer */
      LevelsPack* nSelectedPack;
      nSelectedPack = pLevelPackTree->getSelectedPack();
      if(nSelectedPack != NULL) {
        pOpenButton->setClicked(false);
        pOpenButton->setActive(false);      

        m_pActiveLevelPack = nSelectedPack;

        UIStatic *pTitle = (UIStatic *)m_pLevelPackViewer->getChild("LEVELPACK_VIEWER_TITLE");
        if(pTitle != NULL) pTitle->setCaption(m_pActiveLevelPack->Name());
        
        _CreateLevelPackLevelList();
#if defined(SUPPORT_WEBACCESS)
	m_pPackLevelInfoFrame->showWindow(false);
#endif
        m_pLevelPackViewer->showWindow(true);
        m_pMainMenu->enableChildren(false);
        m_pMainMenu->enableWindow(false);
        m_State = GS_LEVELPACK_VIEWER;
        return;
      }
    }
    
    /* OPTIONS */
    UIButton *pEnableAudioButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:ENABLE_AUDIO");
    UIButton *p11kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE11KHZ");
    UIButton *p22kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE22KHZ");
    UIButton *p44kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE44KHZ");
    UIButton *pSample8Button = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:8BIT");
    UIButton *pSample16Button = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:16BIT");
    UIButton *pMonoButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:MONO");
    UIButton *pStereoButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:STEREO");
    UIButton *pEnableEngineSoundButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:ENABLE_ENGINE_SOUND");
    UIButton *pEnableMusicButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:ENABLE_MUSIC");
  
#if defined(SUPPORT_WEBACCESS)  
    UIButton *pINetConf = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:PROXYCONFIG");
    UIButton *pUpdHS = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:UPDATEHIGHSCORES");
    UIButton *pWebHighscores = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLEWEBHIGHSCORES");
    UIButton *pInGameWorldRecord = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:INGAMEWORLDRECORD");

    UIButton *pCheckNewLevelsAtStartup = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLECHECKNEWLEVELSATSTARTUP");
    UIButton *pCheckHighscoresAtStartup = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLECHECKHIGHSCORESATSTARTUP");
    UIWindow *pRoomsTab = (UIWindow *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB");
    UIButton *pUpdRoomsList = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB:UPDATE_ROOMS_LIST");

    UIButton *pUpdThemeList = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:UPDATE_THEMES_LIST");
    UIButton *pUpdSelectedTheme = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:GET_SELECTED_THEME");
    UIList *pThemeList = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:THEMES_LIST");
#endif

    UIButton *pEnableGhost = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:ENABLE_GHOST");
    UIList *pGhostStrategy = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:GHOST_STRATEGIES_LIST");
    UIButton *pMotionBlurGhost = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:MOTION_BLUR_GHOST");
    UIButton *pDisplayGhostInfo = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:DISPLAY_GHOST_INFO");
    UIButton *pDisplayGhostTimeDiff = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:DISPLAY_GHOST_TIMEDIFF");

    if(pEnableGhost->getChecked()) {
      pGhostStrategy->enableWindow(true);
      pMotionBlurGhost->enableWindow(true);
      pDisplayGhostInfo->enableWindow(true);
      pDisplayGhostTimeDiff->enableWindow(true);
    } else {
      pGhostStrategy->enableWindow(false);
      pMotionBlurGhost->enableWindow(false);
      pDisplayGhostInfo->enableWindow(false);
      pDisplayGhostTimeDiff->enableWindow(false);
    }

#if defined(SUPPORT_WEBACCESS)
      if(pWebHighscores->getChecked()) {
        pINetConf->enableWindow(true);
        pUpdHS->enableWindow(true);
	pCheckNewLevelsAtStartup->enableWindow(true);
	pCheckHighscoresAtStartup->enableWindow(true);
	pUpdThemeList->enableWindow(true);
	pUpdSelectedTheme->enableWindow(true);
	pRoomsTab->enableWindow(true);
      }
      else {
        pINetConf->enableWindow(false);
        pUpdHS->enableWindow(false);
	pCheckNewLevelsAtStartup->enableWindow(false);
	pCheckHighscoresAtStartup->enableWindow(false);
	pUpdThemeList->enableWindow(false);
	pUpdSelectedTheme->enableWindow(false);
	pRoomsTab->enableWindow(false);
      }
#endif
    
    if(pEnableAudioButton) {
      bool t=pEnableAudioButton->getChecked();
      p11kHzButton->enableWindow(t);
      p22kHzButton->enableWindow(t);
      p44kHzButton->enableWindow(t);      
      pSample8Button->enableWindow(t);
      pSample16Button->enableWindow(t);
      pMonoButton->enableWindow(t);
      pStereoButton->enableWindow(t);
      pEnableEngineSoundButton->enableWindow(t);
      pEnableMusicButton->enableWindow(t);
    }

#if defined(SUPPORT_WEBACCESS)
    if(pINetConf->isClicked()) {
      pINetConf->setClicked(false);
      
      _InitWebConf();
      m_State = GS_EDIT_WEBCONFIG;
      m_pWebConfEditor->showWindow(true);
      m_pMainMenu->enableChildren(false);
      m_pMainMenu->enableWindow(false);
      return;
    }
    
    if(pUpdHS->isClicked()) {
      pUpdHS->setClicked(false);
      try {
	_UpdateWebHighscores(false);
	_UpgradeWebHighscores();    
	_UpdateWebLevels(false);  

	_UpdateLevelsLists();
      } catch(Exception &e) {
	notifyMsg(GAMETEXT_FAILEDDLHIGHSCORES);
      }
    }

    if(pUpdRoomsList->isClicked()) {
      pUpdRoomsList->setClicked(false);
      try {
    	_UpdateWebRooms(false);
    	_UpgradeWebRooms(true);    
      } catch(Exception &e) {
    	notifyMsg(GAMETEXT_FAILEDDLROOMSLIST);
      }
    }
    
    if(pUpdThemeList->isClicked()) {
      pUpdThemeList->setClicked(false);
      try {
	_UpdateWebThemes(false);
      } catch(Exception &e) {
	notifyMsg(GAMETEXT_FAILEDUPDATETHEMESLIST);
      }
    }  

    if(pUpdSelectedTheme->isClicked()) {
      pUpdSelectedTheme->setClicked(false);
      try {
	ThemeChoice* v_themeChoice = NULL;

	if(!pThemeList->isBranchHidden() && pThemeList->getSelected()>=0) {
	  if(!pThemeList->getEntries().empty()) {
	    UIListEntry *pEntry = pThemeList->getEntries()[pThemeList->getSelected()];
	    v_themeChoice = reinterpret_cast<ThemeChoice*>(pEntry->pvUser);
	    if(v_themeChoice != NULL) {
	      _UpdateWebTheme(v_themeChoice, true);
	    }
	  }
	}
      } catch(Exception &e) {
	notifyMsg(GAMETEXT_FAILEDGETSELECTEDTHEME);
      }
    } 
 
#endif    

    UIButton *pSaveOptions = (UIButton *)m_pOptionsWindow->getChild("SAVE_BUTTON");
    UIButton *pDefaultOptions = (UIButton *)m_pOptionsWindow->getChild("DEFAULTS_BUTTON");
    UIList *pActionKeyList = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:CONTROLS_TAB:KEY_ACTION_LIST");
    UIButton *pJoystickRB = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:CONTROLS_TAB:JOYSTICK");
    UIButton *pKeyboardRB = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:CONTROLS_TAB:KEYBOARD");
    UIButton *pConfigJoystick = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:CONTROLS_TAB:CONFIGURE_JOYSTICK");
    
    if(SDL_NumJoysticks() > 0) {    
      if(pJoystickRB->getChecked()) {
        pActionKeyList->enableWindow(false);
        pConfigJoystick->enableWindow(true);
      }
      else if(pKeyboardRB->getChecked()) {
        pActionKeyList->enableWindow(true);
        pConfigJoystick->enableWindow(false);
      }
    }
    else {
      pJoystickRB->setChecked(false);
      pKeyboardRB->setChecked(true);
      pActionKeyList->enableWindow(true);
      pConfigJoystick->enableWindow(false);
    }
    
    if(pSaveOptions && pSaveOptions->isClicked()) {
      _SaveOptions();
    }
    else if(pDefaultOptions && pDefaultOptions->isClicked()) {
      _DefaultOptions();
    }
    
    if(pActionKeyList && pActionKeyList->isItemActivated()) {
      _ChangeKeyConfig();
    }
    
    if(pConfigJoystick && pConfigJoystick->isClicked()) {
      _ConfigureJoystick();
    }
    
    /* HELP */
    /* Tutorial button clicked? */
    UIButton *pTutorialButton = (UIButton *)m_pHelpWindow->getChild("HELP_TUTORIAL_BUTTON");
    if(pTutorialButton && pTutorialButton->isClicked()) {
      pTutorialButton->setClicked(false);
      
      /* Find first tutorial level */
      try {
	Level *pLevelSrc = &(m_levelsManager.LevelById("tut1"));
        m_pMainMenu->showWindow(false);      
        m_PlaySpecificLevel = pLevelSrc->Id();
        m_StateAfterPlaying = GS_MENU;
	m_currentPlayingList = NULL;
        setState(GS_PREPLAYING);
      } catch(Exception &e) {
      }
    }
    /* View credits? */
    UIButton *pCreditsButton = (UIButton *)m_pHelpWindow->getChild("HELP_CREDITS_BUTTON");
    if(pCreditsButton && pCreditsButton->isClicked()) {
      pCreditsButton->setClicked(false);
      
      m_pMainMenu->showWindow(false);      
      m_PlaySpecificReplay = "credits.rpl";
      m_StateAfterPlaying = GS_MENU;
      setState(GS_CREDITSMODE);      
    }
    
    /* PLAY */
    UIButton *pPlayGoButton = (UIButton *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:ALLLEVELS_TAB:PLAY_GO_BUTTON");
    UIButton *pLevelInfoButton = (UIButton *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:ALLLEVELS_TAB:PLAY_LEVEL_INFO_BUTTON");
    UIButton *pLevelDeleteFromFavoriteButton = (UIButton *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:ALLLEVELS_TAB:ALL_LEVELS_DELETE_FROM_FAVORITE_BUTTON");

#if defined(SUPPORT_WEBACCESS)
    UIButton *pNewLevelsPlayGoButton =    (UIButton *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:NEWLEVELS_TAB:NEW_LEVELS_PLAY_GO_BUTTON");
    UIButton *pNewLevelsLevelInfoButton = (UIButton *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:NEWLEVELS_TAB:NEW_LEVELS_PLAY_LEVEL_INFO_BUTTON");
    UIButton *pNewLevelsPlayDLButton =    (UIButton *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:NEWLEVELS_TAB:NEW_LEVELS_PLAY_DOWNLOAD_LEVELS_BUTTON");

    if(pNewLevelsPlayDLButton != NULL) {
      if(pWebHighscores->getChecked())
	pNewLevelsPlayDLButton->enableWindow(true);
      else
	pNewLevelsPlayDLButton->enableWindow(false);
      
      if(pNewLevelsPlayDLButton->isClicked()) {
	pNewLevelsPlayDLButton->setClicked(false);
	
	_CheckForExtraLevels();
      }
    }
#endif

    if(pLevelDeleteFromFavoriteButton->isClicked()) {
      Level *pLevelSrc = NULL;

      pLevelDeleteFromFavoriteButton->setClicked(false);
      pLevelSrc = m_pAllLevelsList->getSelectedLevel();
      if(pLevelSrc != NULL) {
	m_levelsManager.delFromFavorite(pLevelSrc);
	_UpdateLevelPackList();
	_UpdateLevelLists();
      }
    }

    if(pPlayGoButton->isClicked()
#if defined(SUPPORT_WEBACCESS)
       || pNewLevelsPlayGoButton->isClicked()
#endif
       ) {
      pPlayGoButton->setClicked(false);
#if defined(SUPPORT_WEBACCESS)
      pNewLevelsPlayGoButton->setClicked(false);
#endif
      
      /* Find out what to play */
      Level *pLevelSrc = NULL;

      if(m_pAllLevelsList && !m_pAllLevelsList->isBranchHidden()) {
	pLevelSrc = m_pAllLevelsList->getSelectedLevel();
	m_currentPlayingList = m_pAllLevelsList;
#if defined(SUPPORT_WEBACCESS)
      } else if(m_pPlayNewLevelsList && !m_pPlayNewLevelsList->isBranchHidden()) {
	pLevelSrc = m_pPlayNewLevelsList->getSelectedLevel();
	m_currentPlayingList = m_pPlayNewLevelsList;
#endif
      }

      /* Start playing it */
      if(pLevelSrc != NULL) {
        m_pMainMenu->showWindow(false);      
        m_PlaySpecificLevel = pLevelSrc->Id();
        m_StateAfterPlaying = GS_MENU;
        setState(GS_PREPLAYING);
      }
    }
    else if(pLevelInfoButton->isClicked()
#if defined(SUPPORT_WEBACCESS)
	    || pNewLevelsLevelInfoButton->isClicked()
#endif
	    ) {
      pLevelInfoButton->setClicked(false);
#if defined(SUPPORT_WEBACCESS)
      pNewLevelsLevelInfoButton->setClicked(false);
#endif
      
      /* Find out what level is selected */
      Level *pLevelSrc = NULL;

      if(m_pAllLevelsList && !m_pAllLevelsList->isBranchHidden()) {
	pLevelSrc = m_pAllLevelsList->getSelectedLevel();
#if defined(SUPPORT_WEBACCESS)
      } else if(m_pPlayNewLevelsList && !m_pPlayNewLevelsList->isBranchHidden()) {
	pLevelSrc = m_pPlayNewLevelsList->getSelectedLevel();
#endif
      }
      
      if(pLevelSrc != NULL) {
        /* Set information */
        UIStatic *pLevelName = (UIStatic *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TITLE");
        
        if(pLevelName != NULL) pLevelName->setCaption(pLevelSrc->Name());

        UIStatic *pGeneralInfo_LevelPack = (UIStatic *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_GENERALINFO_TAB:LEVEL_VIEWER_INFO_LEVELPACK");
        UIStatic *pGeneralInfo_LevelName = (UIStatic *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_GENERALINFO_TAB:LEVEL_VIEWER_INFO_LEVELNAME");
        UIStatic *pGeneralInfo_Author = (UIStatic *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_GENERALINFO_TAB:LEVEL_VIEWER_INFO_AUTHOR");
        UIStatic *pGeneralInfo_Date = (UIStatic *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_GENERALINFO_TAB:LEVEL_VIEWER_INFO_DATE");
        UIStatic *pGeneralInfo_Description = (UIStatic *)m_pLevelInfoViewer->getChild("LEVEL_VIEWER_TABS:LEVEL_VIEWER_GENERALINFO_TAB:LEVEL_VIEWER_INFO_DESCRIPTION");

        if(pGeneralInfo_LevelPack != NULL) pGeneralInfo_LevelPack->setCaption(std::string(GAMETEXT_LEVELPACK) + ": " + pLevelSrc->Pack());
        if(pGeneralInfo_LevelName != NULL) pGeneralInfo_LevelName->setCaption(std::string(GAMETEXT_LEVELNAME) + ": " + pLevelSrc->Name());
        if(pGeneralInfo_Author != NULL) pGeneralInfo_Author->setCaption(std::string(GAMETEXT_AUTHOR) + ": " + pLevelSrc->Author());
        if(pGeneralInfo_Date != NULL) pGeneralInfo_Date->setCaption(std::string(GAMETEXT_DATE) + ": " + pLevelSrc->Date());
        if(pGeneralInfo_Description != NULL) pGeneralInfo_Description->setCaption(std::string(GAMETEXT_DESCRIPTION) + ": "  + pLevelSrc->Description());
            
        _UpdateLevelInfoViewerBestTimes(m_LevelInfoViewerLevel = pLevelSrc->Id());
        _UpdateLevelInfoViewerReplays(m_LevelInfoViewerLevel);
        
        /* Nice. Open the level info viewer */
        pLevelInfoButton->setActive(false);
#if defined(SUPPORT_WEBACCESS)
        pNewLevelsLevelInfoButton->setActive(false);
#endif
        m_pLevelInfoViewer->showWindow(true);
        m_pMainMenu->enableChildren(false);
        m_pMainMenu->enableWindow(false);
        m_State = GS_LEVEL_INFO_VIEWER;
      }
    }
       
    /* REPLAYS */        
    UIButton *pReplaysShowButton = (UIButton *)m_pReplaysWindow->getChild("REPLAY_SHOW_BUTTON");
    UIButton *pReplaysDeleteButton = (UIButton *)m_pReplaysWindow->getChild("REPLAY_DELETE_BUTTON");
#if defined(SUPPORT_WEBACCESS)
    UIButton *pUploadHighscoreButton = (UIButton *)m_pReplaysWindow->getChild("REPLAY_UPLOADHIGHSCORE_BUTTON");
#endif
    UIButton *pReplaysListAllButton = (UIButton *)m_pReplaysWindow->getChild("REPLAY_LIST_ALL");
    UIList *pReplaysList = (UIList *)m_pReplaysWindow->getChild("REPLAY_LIST");
    
    if(pReplaysList->getEntries().empty()) {
      pReplaysShowButton->enableWindow(false);
      pReplaysDeleteButton->enableWindow(false);
    }
    else {
      pReplaysShowButton->enableWindow(true);
      pReplaysDeleteButton->enableWindow(true);
    }

    if(pReplaysListAllButton->isClicked()) {
      _UpdateReplaysList();      
    }
    
#if defined(SUPPORT_WEBACCESS)
    if(pReplaysList->isChanged()) {
      pReplaysList->setChanged(false);
      pUploadHighscoreButton->enableWindow(false);

      if(m_bEnableWebHighscores) {
	      if(pReplaysList->getSelected() >= 0 && pReplaysList->getSelected() < pReplaysList->getEntries().size()) {
	        UIListEntry *pListEntry = pReplaysList->getEntries()[pReplaysList->getSelected()];
	        if(pListEntry != NULL) {
	          ReplayInfo* rplInfos;
	          rplInfos = Replay::getReplayInfos(pListEntry->Text[0]);
	          if(rplInfos != NULL) {
	            if(rplInfos->fFinishTime > 0.0 && rplInfos->Player == m_pPlayer->PlayerName) {
		            WebHighscore* wh = m_pWebHighscores->getHighscoreFromLevel(rplInfos->Level);
		            if(wh != NULL) {
		              pUploadHighscoreButton->enableWindow(rplInfos->fFinishTime < wh->getFTime());
		            } else {
		              pUploadHighscoreButton->enableWindow(true);
		            }
	            }  	      
	            delete rplInfos; 
	          }
	        }
	      }
      }
    }
    
    if(pUploadHighscoreButton->isClicked()) {
      pReplaysList->setClicked(false);
      if(pReplaysList->getSelected() >= 0 && pReplaysList->getSelected() < pReplaysList->getEntries().size()) {
        UIListEntry *pListEntry = pReplaysList->getEntries()[pReplaysList->getSelected()];
        if(pListEntry != NULL) {
	  _UploadHighscore(pListEntry->Text[0]);
	}
      }
    }
#endif

    if(pReplaysShowButton->isClicked()) {
      /* Show replay */
      if(pReplaysList->getSelected() >= 0 && pReplaysList->getSelected() < pReplaysList->getEntries().size()) {
        UIListEntry *pListEntry = pReplaysList->getEntries()[pReplaysList->getSelected()];
        if(pListEntry != NULL) {
          /* Do it captain */
          pReplaysShowButton->setClicked(false);
          m_pMainMenu->showWindow(false);
          m_PlaySpecificReplay = pListEntry->Text[0];
          m_StateAfterPlaying = GS_MENU;
          setState(GS_REPLAYING);
        }
      }
    }
    
    if(pReplaysDeleteButton->isClicked()) {
      /* Delete replay - but ask the user first */
      if(m_pDeleteReplayMsgBox == NULL)
        m_pDeleteReplayMsgBox = m_Renderer.getGUI()->msgBox(GAMETEXT_DELETEREPLAYMESSAGE,
                                                            (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));      
    }
    
    /* Statistics window */
    if(m_pStatsReport != NULL) {
      UIButton *pUpdateReport = (UIButton *)m_pStatsReport->getChild("UPDATE_BUTTON");
      if(pUpdateReport != NULL) {
        if(pUpdateReport->isClicked()) {
          m_pStatsWindow->makeActive();
          m_pStatsWindow->setMinimized(false);
          pUpdateReport->setClicked(false);          
          
          /* Update */
          delete m_pStatsReport;
          m_pStatsReport = m_GameStats.generateReport(m_pPlayer->PlayerName,m_pStatsWindow,30,36,m_pStatsWindow->getPosition().nWidth-45,m_pStatsWindow->getPosition().nHeight-36,m_Renderer.getSmallFont());
        }        
      }
    }
  }

  /*===========================================================================
  Change a key config/joystick stuff
  ===========================================================================*/
  int GameApp::_IsKeyInUse(const std::string &Key) {
    UIList *pActionList = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:CONTROLS_TAB:KEY_ACTION_LIST");

    for(int i=0;i<pActionList->getEntries().size();i++) {
      if(pActionList->getEntries()[i]->Text[1] == Key) return i;
    }
    return -1;
  }
  
  void GameApp::_SimpleMessage(const std::string &Msg,UIRect *pRect,bool bNoSwap) {      
    m_Renderer.getGUI()->paint();
    drawLib->drawBox(Vector2f(0,0),Vector2f(drawLib->getDispWidth(),drawLib->getDispHeight()),0,MAKE_COLOR(0,0,0,170),0);
    int cx,cy;

    m_Renderer.getGUI()->setFont(m_Renderer.getMediumFont());
    m_Renderer.getGUI()->getTextSize(Msg.c_str(),&cx,&cy);
    
    int nW = cx + 150, nH = cy + 150;
    int nx = drawLib->getDispWidth()/2 - nW/2,ny = drawLib->getDispHeight()/2 - nH/2;
    
    if(pRect != NULL) {
      pRect->nX = nx;
      pRect->nY = ny;
      pRect->nWidth = nW;
      pRect->nHeight = nH;
    }

    m_Renderer.getGUI()->putElem(nx,ny,-1,-1,UI_ELEM_FRAME_TL,false);
    m_Renderer.getGUI()->putElem(nx+nW-8,ny,-1,-1,UI_ELEM_FRAME_TR,false);
    m_Renderer.getGUI()->putElem(nx+nW-8,ny+nH-8,-1,-1,UI_ELEM_FRAME_BR,false);
    m_Renderer.getGUI()->putElem(nx,ny+nH-8,-1,-1,UI_ELEM_FRAME_BL,false);
    m_Renderer.getGUI()->putElem(nx+8,ny,nW-16,-1,UI_ELEM_FRAME_TM,false);
    m_Renderer.getGUI()->putElem(nx+8,ny+nH-8,nW-16,-1,UI_ELEM_FRAME_BM,false);
    m_Renderer.getGUI()->putElem(nx,ny+8,-1,nH-16,UI_ELEM_FRAME_ML,false);
    m_Renderer.getGUI()->putElem(nx+nW-8,ny+8,-1,nH-16,UI_ELEM_FRAME_MR,false);
    m_Renderer.getGUI()->putRect(nx+8,ny+8,nW-16,nH-16,MAKE_COLOR(0,0,0,127));

    m_Renderer.getGUI()->putText(drawLib->getDispWidth()/2 - cx/2,drawLib->getDispHeight()/2,Msg.c_str());
    m_Renderer.getGUI()->setFont(m_Renderer.getSmallFont());
    
    if(!bNoSwap)
      drawLib->flushGraphics();
  }
  
  void GameApp::_ConfigureJoystick(void) {
    _SimpleMessage("Nothing here yet! :)");
    SDL_Delay(1500);
  }
  
  void GameApp::_ChangeKeyConfig(void) {
    /* Find out what is selected... */
    UIList *pActionList = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:CONTROLS_TAB:KEY_ACTION_LIST");
    int nSel = pActionList->getSelected();
    if(nSel >= 0 && nSel < pActionList->getEntries().size()) {
      char cBuf[1024];                
      sprintf(cBuf,GAMETEXT_PRESSANYKEYTO,pActionList->getEntries()[nSel]->Text[0].c_str());
      _SimpleMessage(cBuf);
      
      while(1) {
        /* Wait for a key */
        std::string NewKey = m_InputHandler.waitForKey();
        if(NewKey == "<<QUIT>>") {
          /* Quit! */
          quit();
        }        
        else if(NewKey == "<<CANCEL>>" || NewKey == "") {
          /* Do nothing */
          break;
        }
        else {
          /* Good... is the key already in use? */
          int nAlreadyUsedBy = _IsKeyInUse(NewKey);
          
          if(nAlreadyUsedBy<0 || nAlreadyUsedBy == nSel) {
            pActionList->getEntries()[nSel]->Text[1] = NewKey;        
            break;
          }
          else {
            sprintf(cBuf, (std::string(GAMETEXT_PRESSANYKEYTO) + "\n" + std::string(GAMETEXT_ALREADYUSED)).c_str(),
		    pActionList->getEntries()[nSel]->Text[0].c_str());
            _SimpleMessage(cBuf);
          }
        }
      }      
    }
  }
  
  /*===========================================================================
  Add all profiles to the list
  ===========================================================================*/
  void GameApp::_CreateProfileList(void) {
    UIList *pList = reinterpret_cast<UIList *>(m_pProfileEditor->getChild("PROFILE_LIST"));
    if(pList != NULL) {
      /* Clear it */
      pList->clear();
      
      /* Add all player profiles to it */
      for(int i=0;i<m_Profiles.getProfiles().size();i++) {
        if(m_pPlayer != NULL && m_pPlayer->PlayerName == m_Profiles.getProfiles()[i]->PlayerName)
          pList->setRealSelected(i);
      
        pList->addEntry(m_Profiles.getProfiles()[i]->PlayerName);
      }
      
      /* Update buttons */
      UIButton *pUseButton = reinterpret_cast<UIButton *>(m_pProfileEditor->getChild("USEPROFILE_BUTTON"));
      UIButton *pDeleteButton = reinterpret_cast<UIButton *>(m_pProfileEditor->getChild("DELETEPROFILE_BUTTON"));
      
      if(m_Profiles.getProfiles().empty()) {
        pUseButton->enableWindow(false);
        pDeleteButton->enableWindow(false);
      }
      else {
        pUseButton->enableWindow(true);
        
        if(m_Profiles.getProfiles().size() > 1) 
          pDeleteButton->enableWindow(true);
        else
          pDeleteButton->enableWindow(false);
      }
    }
  }

  /*===========================================================================
  Update replays list
  ===========================================================================*/
  void GameApp::_CreateReplaysList(UIList *pList) {
    /* Should we list all players' replays? */
    UIButton *pButton = (UIButton *)m_pReplaysWindow->getChild("REPLAY_LIST_ALL");
    bool bListAll = false;
    if(pButton != NULL && pButton->getChecked()) {
      bListAll = true;
    }
    
    /* Clear list */
    pList->clear();
    
    /* Enumerate replays */
    std::string PlayerSearch;
    if(!bListAll) PlayerSearch = m_pPlayer->PlayerName;
    
    std::vector<ReplayInfo *> *Replays = m_ReplayList.findReplays(PlayerSearch);
    
    for(int i=0;i<Replays->size();i++) {
      UIListEntry *pEntry = pList->addEntry((*Replays)[i]->Name);

      Level *pLevel;

      try {
	pLevel = &(m_levelsManager.LevelById((*Replays)[i]->Level));      
        pEntry->Text.push_back(pLevel->Name());
      } catch(Exception &e) {
        pEntry->Text.push_back("(" + std::string(GAMETEXT_UNKNOWNLEVEL) + ")");
      }
      
      pEntry->Text.push_back((*Replays)[i]->Player);
    }

    delete Replays;
  }
  
  /*===========================================================================
  Scan through loaded levels
  ===========================================================================*/
  void GameApp::_CreateLevelLists(UILevelList *pAllLevels, std::string i_packageName) {
    /* get selected item */
    std::string v_selected_levelName = "";
    if(pAllLevels->getSelected() >= 0 && pAllLevels->getSelected() < pAllLevels->getEntries().size()) {
      UIListEntry *pEntry = pAllLevels->getEntries()[pAllLevels->getSelected()];
      v_selected_levelName = pEntry->Text[0];
    }

    pAllLevels->clear();

    if(m_pPlayer == NULL) return;
    LevelsPack *v_levelsPack = &(m_levelsManager.LevelsPackByName(i_packageName));

    for(int i=0;i<v_levelsPack->Levels().size();i++) {
      Level *pLevel = v_levelsPack->Levels()[i];     
      pAllLevels->addLevel(pLevel,
			   m_pPlayer,
			   &m_Profiles
#if defined(SUPPORT_WEBACCESS) 
			   ,
			   m_pWebHighscores
#endif
			   );
    }

    /* reselect the previous level */
    if(v_selected_levelName != "") {
      int nLevel = 0;
      for(int i=0; i<pAllLevels->getEntries().size(); i++) {
        if(pAllLevels->getEntries()[i]->Text[0] == v_selected_levelName) {
          nLevel = i;
          break;
        }
      }
      pAllLevels->setRealSelected(nLevel);
    }
  }

  void GameApp::_CreateThemesList(UIList *pList) {
    /* get selected item */
    std::string v_selected_themeName = "";
    if(pList->getSelected() >= 0 && pList->getSelected() < pList->getEntries().size()) {
      UIListEntry *pEntry = pList->getEntries()[pList->getSelected()];
      v_selected_themeName = pEntry->Text[0];
    }

    /* recreate the list */
    pList->clear();
    std::vector<ThemeChoice*> v_themeChoices;
    UIListEntry *pEntry;
    v_themeChoices = m_themeChoicer->getChoices();
    for(int i=0; i<v_themeChoices.size(); i++) {
      if(v_themeChoices[i]->getHosted()) {

        if(v_themeChoices[i]->getRequireUpdate()) {
          pEntry = pList->addEntry(v_themeChoices[i]->ThemeName().c_str(),
                                   reinterpret_cast<void *>(v_themeChoices[i]));
          pEntry->Text.push_back(GAMETEXT_THEMEREQUIREUPDATE);
        } else {
          pEntry = pList->addEntry(v_themeChoices[i]->ThemeName().c_str(),
                                   reinterpret_cast<void *>(v_themeChoices[i]));
          pEntry->Text.push_back(GAMETEXT_THEMEHOSTED);
        }
      } else {
        pEntry = pList->addEntry(v_themeChoices[i]->ThemeName().c_str(),
                                 reinterpret_cast<void *>(v_themeChoices[i])); 
        pEntry->Text.push_back(GAMETEXT_THEMENOTHOSTED);
      }
    }

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

#if defined(SUPPORT_WEBACCESS) 

  void GameApp::_CreateRoomsList(UIList *pList) {
    UIListEntry *pEntry;
    std::string v_selected_roomName = "";
    std::vector<WebRoomInfos*> v_webroomsInfos;

    /* get selected item */
    if(pList->getSelected() >= 0 && pList->getSelected() < pList->getEntries().size()) {
      UIListEntry *pEntry = pList->getEntries()[pList->getSelected()];
      v_selected_roomName = pEntry->Text[0];
    }

    /* recreate the list */
    pList->clear();

    v_webroomsInfos = m_pWebRooms->getAvailableRooms();
    for(int i=0; i<v_webroomsInfos.size(); i++) {
      pEntry = pList->addEntry(v_webroomsInfos[i]->getName(),
			       reinterpret_cast<void *>(v_webroomsInfos[i])
			       );
    }

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

#endif

  /*===========================================================================
  Fill a window with best times
  ===========================================================================*/
  void GameApp::_MakeBestTimesWindow(UIBestTimes *pWindow,std::string PlayerName,std::string LevelID,
                                     float fFinishTime,std::string TimeStamp) {
    int n1=-1,n2=-1;
    
    pWindow->clear();
    
    std::vector<PlayerTimeEntry *> Global = m_Profiles.createLevelTop10(LevelID);
    std::vector<PlayerTimeEntry *> Personal = m_Profiles.createPlayerOnlyLevelTop10(PlayerName,LevelID);
    
    for(int i=0;i<(Global.size()<10?Global.size():10);i++) {      
      pWindow->addRow1(formatTime(Global[i]->fFinishTime),Global[i]->PlayerName);
      if(Global[i]->fFinishTime == fFinishTime && Global[i]->PlayerName == PlayerName &&
         Global[i]->TimeStamp == TimeStamp) n1 = i;
    }

    for(int i=0;i<(Personal.size()<10?Personal.size():10);i++) {      
      pWindow->addRow2(formatTime(Personal[i]->fFinishTime),Personal[i]->PlayerName);
      if(Personal[i]->fFinishTime == fFinishTime && Personal[i]->PlayerName == PlayerName &&
         Personal[i]->TimeStamp == TimeStamp) n2 = i;
    }

    pWindow->setup(GAMETEXT_BESTTIMES,n1,n2);
  }

  /*===========================================================================
  Options fun
  ===========================================================================*/
  void GameApp::_ImportOptions(void) {
    UIButton *pShowMiniMap = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:SHOWMINIMAP");
    UIButton *pDeathAnim = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:DEATHANIM");
    UIButton *pInitZoom = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:INITZOOM");
    UIButton *pShowEngineCounter = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:SHOWENGINECOUNTER");

    UIButton *pContextHelp = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:ENABLECONTEXTHELP");
    UIButton *pAutosaveReplays = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:AUTOSAVEREPLAYS");

    UIList *pThemeList = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:THEMES_LIST");

    UIButton *pEnableAudioButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:ENABLE_AUDIO");
    UIButton *p11kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE11KHZ");
    UIButton *p22kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE22KHZ");
    UIButton *p44kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE44KHZ");
    UIButton *pSample8Button = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:8BIT");
    UIButton *pSample16Button = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:16BIT");
    UIButton *pMonoButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:MONO");
    UIButton *pStereoButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:STEREO");
    UIButton *pEnableEngineSoundButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:ENABLE_ENGINE_SOUND");
    UIButton *pEnableMusicButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:ENABLE_MUSIC");

    UIButton *p16bpp = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:16BPP");
    UIButton *p32bpp = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:32BPP");
    UIList *pResList = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:RES_LIST");
    UIButton *pRunWindowed = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:RUN_WINDOWED");
    UIButton *pMenuLow = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:MENULOW");
    UIButton *pMenuMed = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:MENUMEDIUM");
    UIButton *pMenuHigh = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:MENUHIGH");
    UIButton *pGameLow = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:GAMELOW");
    UIButton *pGameMed = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:GAMEMEDIUM");
    UIButton *pGameHigh = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:GAMEHIGH");
    
    UIButton *pKeyboardControl = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:CONTROLS_TAB:KEYBOARD");
    UIButton *pJoystickControl = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:CONTROLS_TAB:JOYSTICK");
        
    p11kHzButton->setChecked(false);
    p22kHzButton->setChecked(false);
    p44kHzButton->setChecked(false);
    pSample8Button->setChecked(false);
    pSample16Button->setChecked(false);
    pMonoButton->setChecked(false);
    pStereoButton->setChecked(false);
    pEnableEngineSoundButton->setChecked(false);
    pEnableMusicButton->setChecked(false);
    
    p16bpp->setChecked(false);
    p32bpp->setChecked(false);
    pRunWindowed->setChecked(false);
    pMenuLow->setChecked(false);
    pMenuMed->setChecked(false);
    pMenuHigh->setChecked(false);
    pGameLow->setChecked(false);
    pGameMed->setChecked(false);
    pGameHigh->setChecked(false);
    
    pKeyboardControl->setChecked(false);
    pJoystickControl->setChecked(false);
    
#if defined(SUPPORT_WEBACCESS)
    UIButton *pWebHighscores = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLEWEBHIGHSCORES");
    UIButton *pInGameWorldRecord = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:INGAMEWORLDRECORD");
    UIButton *pCheckNewLevelsAtStartup = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLECHECKNEWLEVELSATSTARTUP");
    UIButton *pCheckHighscoresAtStartup = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLECHECKHIGHSCORESATSTARTUP");
    UIList *pRoomsList = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB:ROOMS_LIST");

    pWebHighscores->setChecked(m_Config.getBool("WebHighscores"));
    pInGameWorldRecord->setChecked(m_Config.getBool("ShowInGameWorldRecord"));
    pCheckNewLevelsAtStartup->setChecked(m_Config.getBool("CheckNewLevelsAtStartup"));
    pCheckHighscoresAtStartup->setChecked(m_Config.getBool("CheckHighscoresAtStartup"));

    /* set room in the list */
    std::string v_room_id = m_Config.getString("WebHighscoreUploadIdRoom");
    if(v_room_id == "") {v_room_id = DEFAULT_WEBROOM_ID;}
    for(int i=0; i<pRoomsList->getEntries().size(); i++) {
      WebRoomInfos* v_wri = (WebRoomInfos*)(pRoomsList->getEntries()[i]->pvUser);
      if(v_wri->getId() == v_room_id) {
	pRoomsList->setRealSelected(i);
	break;
      }
    }
#endif

    UIButton *pEnableGhost = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:ENABLE_GHOST");
    UIList *pGhostStrategy = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:GHOST_STRATEGIES_LIST");
    UIButton *pMotionBlurGhost = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:MOTION_BLUR_GHOST");
    UIButton *pDisplayGhostInfo = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:DISPLAY_GHOST_INFO");
    UIButton *pDisplayGhostTimeDiff = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:DISPLAY_GHOST_TIMEDIFF");

    pEnableGhost->setChecked(m_Config.getBool("EnableGhost"));
    int v_ghost_strategy = m_Config.getInteger("GhostSearchStrategy");
    pMotionBlurGhost->setChecked(m_Config.getBool("GhostMotionBlur"));
    pDisplayGhostInfo->setChecked(m_Config.getBool("DisplayGhostInfo"));
    pDisplayGhostTimeDiff->setChecked(m_Config.getBool("ShowGhostTimeDiff"));

    int nGSMode = -1;
    for(int i=0;i<pGhostStrategy->getEntries().size();i++) {
      if(*((int*)(pGhostStrategy->getEntries()[i]->pvUser)) == v_ghost_strategy) {
        nGSMode = i;
        break;
      }
    }
    if(nGSMode < 0) {
      /* TODO: warning */
      pGhostStrategy->setRealSelected(0);
    }
    else {
      pGhostStrategy->setRealSelected(nGSMode);
    }  

    pShowMiniMap->setChecked(m_Config.getBool("ShowMiniMap"));
    pShowEngineCounter->setChecked(m_Config.getBool("ShowEngineCounter"));
    pInitZoom->setChecked(m_Config.getBool("InitZoom"));
    pDeathAnim->setChecked(m_Config.getBool("DeathAnim"));
    pContextHelp->setChecked(m_Config.getBool("ContextHelp"));
    pAutosaveReplays->setChecked(m_Config.getBool("AutosaveHighscoreReplays"));

    std::string v_themeName = m_Config.getString("Theme");
    int nTheme = 0;
    for(int i=0; i<pThemeList->getEntries().size(); i++) {
      if(pThemeList->getEntries()[i]->Text[0] == v_themeName) {
        nTheme = i;
        break;
      }
    }
    pThemeList->setRealSelected(nTheme);

    pEnableAudioButton->setChecked(m_Config.getBool("AudioEnable"));

    switch(m_Config.getInteger("AudioSampleRate")) {
      case 11025: p11kHzButton->setChecked(true); break;
      case 22050: p22kHzButton->setChecked(true); break;
      case 44100: p44kHzButton->setChecked(true); break;
      default: p22kHzButton->setChecked(true); break; /* TODO: warning */
    }    
    
    switch(m_Config.getInteger("AudioSampleBits")) {
      case 8: pSample8Button->setChecked(true); break;
      case 16: pSample16Button->setChecked(true); break;
      default: pSample16Button->setChecked(true); break; /* TODO: warning */
    }
    
    if(m_Config.getString("AudioChannels") == "stereo")
      pStereoButton->setChecked(true);
    else if(m_Config.getString("AudioChannels") == "mono")
      pMonoButton->setChecked(true);
    else
      pMonoButton->setChecked(true); /* TODO: warning */
      
    pEnableEngineSoundButton->setChecked(m_Config.getBool("EngineSoundEnable"));      
    pEnableMusicButton->setChecked(m_Config.getBool("MenuMusic"));      
      
    switch(m_Config.getInteger("DisplayBPP")) {
      case 16: p16bpp->setChecked(true); break;
      case 32: p32bpp->setChecked(true); break;
      default: p16bpp->setChecked(true); break; /* TODO: warning */      
    }
        
    pRunWindowed->setChecked(m_Config.getBool("DisplayWindowed"));
    
    if(m_Config.getString("MenuBackgroundGraphics") == "Low") 
      pMenuLow->setChecked(true);
    else if(m_Config.getString("MenuBackgroundGraphics") == "Medium") 
      pMenuMed->setChecked(true);
    else if(m_Config.getString("MenuBackgroundGraphics") == "High") 
      pMenuHigh->setChecked(true);

    if(m_Config.getString("GameGraphics") == "Low") 
      pGameLow->setChecked(true);
    else if(m_Config.getString("GameGraphics") == "Medium") 
      pGameMed->setChecked(true);
    else if(m_Config.getString("GameGraphics") == "High") 
      pGameHigh->setChecked(true);
    
    char cBuf[256];
    sprintf(cBuf,"%d X %d",m_Config.getInteger("DisplayWidth"),m_Config.getInteger("DisplayHeight"));
    int nMode = -1;
    for(int i=0;i<pResList->getEntries().size();i++) {
      if(pResList->getEntries()[i]->Text[0] == cBuf) {
        nMode = i;
        break;
      }
    }
    if(nMode < 0) {
      /* TODO: warning */
      pResList->setRealSelected(0);
    }
    else {
      pResList->setRealSelected(nMode);
    }      
    
    /* Controls */
    if(m_Config.getString("ControllerMode1") == "Keyboard" || SDL_NumJoysticks()==0)
      pKeyboardControl->setChecked(true);
    else if(m_Config.getString("ControllerMode1") == "Joystick1")
      pJoystickControl->setChecked(true);
   
    _UpdateActionKeyList();
  }
  
  void GameApp::_DefaultOptions(void) {
    bool bNotify = false;
    
    /* These don't require restart */
    m_Config.setValue("GameGraphics",m_Config.getDefaultValue("GameGraphics"));
    m_Config.setValue("MenuBackgroundGraphics",m_Config.getDefaultValue("MenuBackgroundGraphics"));
    m_Config.setValue("ShowMiniMap",m_Config.getDefaultValue("ShowMiniMap"));
    m_Config.setValue("ShowEngineCounter",m_Config.getDefaultValue("ShowEngineCounter"));
    m_Config.setValue("InitZoom",m_Config.getDefaultValue("InitZoom"));
    m_Config.setValue("DeathAnim",m_Config.getDefaultValue("DeathAnim"));
    m_Config.setValue("ContextHelp",m_Config.getDefaultValue("ContextHelp"));
    m_Config.setValue("EngineSoundEnable",m_Config.getDefaultValue("EngineSoundEnable"));
    m_Config.setValue("MenuMusic",m_Config.getDefaultValue("MenuMusic"));
    
    m_Config.setValue("ControllerMode1",m_Config.getDefaultValue("ControllerMode1"));
    m_Config.setValue("KeyDrive1",m_Config.getDefaultValue("KeyDrive1"));
    m_Config.setValue("KeyBrake1",m_Config.getDefaultValue("KeyBrake1"));
    m_Config.setValue("KeyFlipLeft1",m_Config.getDefaultValue("KeyFlipLeft1"));
    m_Config.setValue("KeyFlipRight1",m_Config.getDefaultValue("KeyFlipRight1"));
    m_Config.setValue("KeyChangeDir1",m_Config.getDefaultValue("KeyChangeDir1"));
    
    #if defined(ENABLE_ZOOMING)
    m_Config.setValue("KeyZoomIn",m_Config.getDefaultValue("KeyZoomIn"));
    m_Config.setValue("KeyZoomOut",m_Config.getDefaultValue("KeyZoomOut"));
    m_Config.setValue("KeyZoomInit",m_Config.getDefaultValue("KeyZoomInit"));
    m_Config.setValue("KeyCameraMoveXUp",m_Config.getDefaultValue("KeyCameraMoveXUp"));
    m_Config.setValue("KeyCameraMoveXDown",m_Config.getDefaultValue("KeyCameraMoveXDown"));
    m_Config.setValue("KeyCameraMoveYUp",m_Config.getDefaultValue("KeyCameraMoveYUp"));
    m_Config.setValue("KeyCameraMoveYDown",m_Config.getDefaultValue("KeyCameraMoveYDown"));
    m_Config.setValue("KeyAutoZoom",m_Config.getDefaultValue("KeyAutoZoom"));
    #endif

      m_Config.setValue("GhostMotionBlur",m_Config.getDefaultValue("GhostMotionBlur"));
      m_Config.setValue("DisplayGhostInfo",m_Config.getDefaultValue("DisplayGhostInfo"));
      m_Config.setValue("ShowGhostTimeDiff",m_Config.getDefaultValue("ShowGhostTimeDiff"));

    #if defined(SUPPORT_WEBACCESS)
      m_Config.setValue("ShowInGameWorldRecord",m_Config.getDefaultValue("ShowInGameWorldRecord"));
      m_Config.setValue("AutosaveHighscoreReplays",m_Config.getDefaultValue("AutosaveHighscoreReplays"));
      m_Config.setValue("CheckNewLevelsAtStartup",m_Config.getDefaultValue("CheckNewLevelsAtStartup"));
      m_Config.setValue("CheckHighscoresAtStartup",m_Config.getDefaultValue("CheckHighscoresAtStartup"));
    #endif

    m_Config.setValue("AutosaveHighscoreReplays",m_Config.getDefaultValue("AutosaveHighscoreReplays"));

    m_Config.setValue("WebHighscoreUploadIdRoom", m_Config.getDefaultValue("WebHighscoreUploadIdRoom"));
    m_Config.setValue("WebHighscoreUploadLogin", m_Config.getDefaultValue("WebHighscoreUploadLogin"));
    m_Config.setValue("WebHighscoreUploadPasword", m_Config.getDefaultValue("WebHighscoreUploadPassword"));
    


    /* The following require restart */
    m_Config.setChanged(false);      

    m_Config.setValue("Theme",m_Config.getDefaultValue("Theme"));

    m_Config.setValue("WebHighscores",m_Config.getDefaultValue("WebHighscores"));
    
      m_Config.setValue("EnableGhost",m_Config.getDefaultValue("EnableGhost"));
      m_Config.setValue("GhostSearchStrategy",m_Config.getDefaultValue("GhostSearchStrategy"));
        
    m_Config.setValue("AudioEnable",m_Config.getDefaultValue("AudioEnable"));
    m_Config.setValue("AudioSampleRate",m_Config.getDefaultValue("AudioSampleRate"));
    m_Config.setValue("AudioSampleBits",m_Config.getDefaultValue("AudioSampleBits"));
    m_Config.setValue("AudioChannels",m_Config.getDefaultValue("AudioChannels"));    
    
    m_Config.setValue("DisplayWidth",m_Config.getDefaultValue("DisplayWidth"));
    m_Config.setValue("DisplayHeight",m_Config.getDefaultValue("DisplayHeight"));
    m_Config.setValue("DisplayBPP",m_Config.getDefaultValue("DisplayBPP"));
    m_Config.setValue("DisplayWindowed",m_Config.getDefaultValue("DisplayWindowed"));
    
    if(m_Config.isChanged()) bNotify = true;
    
    /* Notify? */
    if(bNotify) { 
      notifyMsg(GAMETEXT_OPTIONSREQURERESTART);
    }    

    _ImportOptions();
  }
  
  void GameApp::_SaveOptions(void) {
    bool bNotify = false;

    UIButton *pShowMiniMap = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:SHOWMINIMAP");
    UIButton *pShowEngineCounter = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:SHOWENGINECOUNTER");
    UIButton *pDeathAnim = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:DEATHANIM");
    UIButton *pInitZoom = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:INITZOOM");
    UIButton *pContextHelp = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:ENABLECONTEXTHELP");
    UIButton *pAutosaveReplays = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:AUTOSAVEREPLAYS");
    
    UIList *pThemeList = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:THEMES_LIST");

    UIButton *pEnableAudioButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:ENABLE_AUDIO");
    UIButton *p11kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE11KHZ");
    UIButton *p22kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE22KHZ");
    UIButton *p44kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE44KHZ");
    UIButton *pSample8Button = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:8BIT");
    UIButton *pSample16Button = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:16BIT");
    UIButton *pMonoButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:MONO");
    UIButton *pStereoButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:STEREO");
    UIButton *pEnableEngineSoundButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:ENABLE_ENGINE_SOUND");
    UIButton *pEnableMusicButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:ENABLE_MUSIC");
    
    UIButton *p16bpp = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:16BPP");
    UIButton *p32bpp = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:32BPP");
    UIList *pResList = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:RES_LIST");
    UIButton *pRunWindowed = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:RUN_WINDOWED");
    UIButton *pMenuLow = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:MENULOW");
    UIButton *pMenuMed = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:MENUMEDIUM");
    UIButton *pMenuHigh = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:MENUHIGH");
    UIButton *pGameLow = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:GAMELOW");
    UIButton *pGameMed = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:GAMEMEDIUM");
    UIButton *pGameHigh = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:VIDEO_TAB:GAMEHIGH");

    UIButton *pKeyboardControl = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:CONTROLS_TAB:KEYBOARD");
    UIButton *pJoystickControl = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:CONTROLS_TAB:JOYSTICK");
    UIList *pActionList = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:CONTROLS_TAB:KEY_ACTION_LIST");

    /* First all those which don't need a restart */
    m_Config.setBool("ShowMiniMap",pShowMiniMap->getChecked());
    m_Config.setBool("ShowEngineCounter",pShowEngineCounter->getChecked());
    m_Config.setBool("InitZoom",pInitZoom->getChecked());
    m_Config.setBool("DeathAnim",pDeathAnim->getChecked());
    m_Config.setBool("ContextHelp",pContextHelp->getChecked());
    
    if(pMenuLow->getChecked()) m_Config.setString("MenuBackgroundGraphics","Low");
    else if(pMenuMed->getChecked()) m_Config.setString("MenuBackgroundGraphics","Medium");
    else if(pMenuHigh->getChecked()) m_Config.setString("MenuBackgroundGraphics","High");
    
    if(pGameLow->getChecked()) m_Config.setString("GameGraphics","Low");
    else if(pGameMed->getChecked()) m_Config.setString("GameGraphics","Medium");
    else if(pGameHigh->getChecked()) m_Config.setString("GameGraphics","High");
    
    if(pKeyboardControl->getChecked()) m_Config.setString("ControllerMode1","Keyboard");
    else if(pJoystickControl->getChecked()) m_Config.setString("ControllerMode1","Joystick1");

    for(int i=0;i<pActionList->getEntries().size();i++) {
      if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_DRIVE)
	m_Config.setString("KeyDrive1",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_BRAKE)
	m_Config.setString("KeyBrake1",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_FLIPLEFT)
	m_Config.setString("KeyFlipLeft1",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_FLIPRIGHT)
	m_Config.setString("KeyFlipRight1",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_CHANGEDIR)
	m_Config.setString("KeyChangeDir1",pActionList->getEntries()[i]->Text[1]);
#if defined(ENABLE_ZOOMING)
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_ZOOMIN)
	m_Config.setString("KeyZoomIn",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_ZOOMOUT)
	m_Config.setString("KeyZoomOut",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_ZOOMINIT)
	m_Config.setString("KeyZoomInit",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_CAMERAMOVEXUP)
	m_Config.setString("KeyCameraMoveXUp",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_CAMERAMOVEXDOWN)
	m_Config.setString("KeyCameraMoveXDown",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_CAMERAMOVEYUP)
	m_Config.setString("KeyCameraMoveYUp",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_CAMERAMOVEYDOWN)
	m_Config.setString("KeyCameraMoveYDown",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_AUTOZOOM)
	m_Config.setString("KeyAutoZoom",pActionList->getEntries()[i]->Text[1]);
#endif
    }
    
    m_Config.setBool("EngineSoundEnable",pEnableEngineSoundButton->getChecked());

    m_Config.setBool("MenuMusic",pEnableMusicButton->getChecked());

    if(Sound::isEnabled()) {
      if(pEnableAudioButton->getChecked() && pEnableMusicButton->getChecked()
	 && Sound::isPlayingMusic() == false) {
	try {
	  Sound::playMusic(m_theme.getMusic("menu1")->FilePath());
	} catch(Exception &e) {
	  /* hum, no music */
	}
      } else {
	if((pEnableAudioButton->getChecked() == false || pEnableMusicButton->getChecked() == false)
	   && Sound::isPlayingMusic()) {
	  Sound::stopMusic();
	}
      }
    }
      
#if defined(SUPPORT_WEBACCESS)
    UIButton *pWebHighscores = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLEWEBHIGHSCORES");
    UIButton *pInGameWorldRecord = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:INGAMEWORLDRECORD");
    UIButton *pCheckNewLevelsAtStartup = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLECHECKNEWLEVELSATSTARTUP");
    UIButton *pCheckHighscoresAtStartup = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLECHECKHIGHSCORESATSTARTUP");
    UIList *pRoomsList = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB:ROOMS_LIST");
    UIEdit *pRoomsLogin = (UIEdit *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB:ROOM_LOGIN");
    UIEdit *pRoomsPassword = (UIEdit *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB:ROOM_PASSWORD");

    m_Config.setBool("ShowInGameWorldRecord",pInGameWorldRecord->getChecked());
    m_Config.setBool("AutosaveHighscoreReplays",pAutosaveReplays->getChecked());

    m_Config.setBool("CheckNewLevelsAtStartup",pCheckNewLevelsAtStartup->getChecked());
    m_Config.setBool("CheckHighscoresAtStartup",pCheckHighscoresAtStartup->getChecked());

    if(pRoomsList->getSelected() >= 0 &&
       pRoomsList->getSelected() < pRoomsList->getEntries().size()) {
      WebRoomInfos *wri = (WebRoomInfos*)(pRoomsList->getEntries()[pRoomsList->getSelected()]->pvUser);
      m_Config.setString("WebHighscoreUploadIdRoom", wri->getId());
      m_Config.setString("WebHighscoresURL", wri->getUrlHighscores());
    }

    m_Config.setString("WebHighscoreUploadLogin", pRoomsLogin->getCaption());
    m_Config.setString("WebHighscoreUploadPassword", pRoomsPassword->getCaption());

    /* set room in the list */
    std::string v_room_id = m_Config.getString("WebHighscoreUploadIdRoom");
    if(v_room_id == "") {v_room_id = DEFAULT_WEBROOM_ID;}
    for(int i=0; i<pRoomsList->getEntries().size(); i++) {
      WebRoomInfos* v_wri = (WebRoomInfos*)(pRoomsList->getEntries()[i]->pvUser);
      if(v_wri->getId() == v_room_id) {
	pRoomsList->setRealSelected(i);
	break;
      }
    }
#endif

    UIButton *pMotionBlurGhost = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:MOTION_BLUR_GHOST");
    UIButton *pDisplayGhostInfo = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:DISPLAY_GHOST_INFO");
    UIButton *pDisplayGhostTimeDiff = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:DISPLAY_GHOST_TIMEDIFF");

    m_Config.setBool("GhostMotionBlur",pMotionBlurGhost->getChecked());
    m_Config.setBool("DisplayGhostInfo",pDisplayGhostInfo->getChecked());
    m_Config.setBool("ShowGhostTimeDiff",pDisplayGhostTimeDiff->getChecked());

#if defined(SUPPORT_WEBACCESS)
    m_Config.setBool("WebHighscores",pWebHighscores->getChecked());
#endif

    UIButton *pEnableGhost = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:ENABLE_GHOST");
    UIList *pGhostStrategy = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:GHOST_STRATEGIES_LIST");

    m_Config.setBool("EnableGhost",pEnableGhost->getChecked());

    if(pGhostStrategy->getSelected() >= 0 && pGhostStrategy->getSelected() < pGhostStrategy->getEntries().size()) {
      UIListEntry *pEntry = pGhostStrategy->getEntries()[pGhostStrategy->getSelected()];
      m_Config.setInteger("GhostSearchStrategy", *((int*)(pEntry->pvUser)));
    }

    if(pThemeList->getSelected() >= 0 && pThemeList->getSelected() < pThemeList->getEntries().size()) {
      UIListEntry *pEntry = pThemeList->getEntries()[pThemeList->getSelected()];
      m_Config.setString("Theme", pEntry->Text[0]);
      if(m_theme.Name() != pEntry->Text[0]) {
	Log("Reloading the theme...");
	reloadTheme();
      }
    }
       
    /* The following require restart */
    m_Config.setChanged(false);      

    m_Config.setBool("AudioEnable",pEnableAudioButton->getChecked());
    
    if(p11kHzButton->getChecked()) m_Config.setInteger("AudioSampleRate",11025);
    else if(p22kHzButton->getChecked()) m_Config.setInteger("AudioSampleRate",22050);
    else if(p44kHzButton->getChecked()) m_Config.setInteger("AudioSampleRate",44100);
    
    if(pSample8Button->getChecked()) m_Config.setInteger("AudioSampleBits",8);
    else if(pSample16Button->getChecked()) m_Config.setInteger("AudioSampleBits",16);
    
    if(pMonoButton->getChecked()) m_Config.setString("AudioChannels","Mono");
    else if(pStereoButton->getChecked()) m_Config.setString("AudioChannels","Stereo");
    
    if(p16bpp->getChecked()) m_Config.setInteger("DisplayBPP",16);
    else if(p32bpp->getChecked()) m_Config.setInteger("DisplayBPP",32);

    if(pResList->getSelected() >= 0 && pResList->getSelected() < pResList->getEntries().size()) {
      UIListEntry *pEntry = pResList->getEntries()[pResList->getSelected()];
      int nW,nH;
      sscanf(pEntry->Text[0].c_str(),"%d X %d",&nW,&nH);
      
      m_Config.setInteger("DisplayWidth",nW);
      m_Config.setInteger("DisplayHeight",nH);
    }
    
    m_Config.setBool("DisplayWindowed",pRunWindowed->getChecked());
    
    if(m_Config.isChanged()) bNotify = true;
    
    /* Notify? */
    if(bNotify) { 
      notifyMsg(GAMETEXT_OPTIONSREQURERESTART);
    }    
    
    /* Update things that can be updated */
    _UpdateSettings();
  }
  
#if defined(SUPPORT_WEBACCESS)
  void GameApp::setLevelInfoFrameBestPlayer(String pLevelID,
					    UIWindow *i_pLevelInfoFrame,
					    UIButton *i_pLevelInfoViewReplayButton,
					    UIStatic *i_pBestPlayerText
					    ) {
    if(m_pWebHighscores != NULL) {
      WebHighscore *pWH = m_pWebHighscores->getHighscoreFromLevel(pLevelID);
      if(pWH != NULL) {
	i_pLevelInfoFrame->showWindow(true);
	i_pBestPlayerText->setCaption((std::string(GAMETEXT_BESTPLAYER) + " : " + pWH->getPlayerName()).c_str());
	m_pLevelToShowOnViewHighscore = pLevelID;

	/* search if the replay is already downloaded */
	std::vector<ReplayInfo *> *Replays = m_ReplayList.findReplays("", pLevelID);
	String v_replay_name = pWH->getReplayName();
	bool found = false;
	int i = 0;
	while(i < Replays->size() && found == false) {
	  if((*Replays)[i]->Name == v_replay_name) {
	    found = true;
	  }
	  i++;
	}
	delete Replays;

	if(found) {
	  i_pLevelInfoViewReplayButton->enableWindow(true);
	} else {
	  i_pLevelInfoViewReplayButton->enableWindow(m_bEnableWebHighscores);
	}
      } else {
	i_pLevelInfoFrame->showWindow(false);
	m_pLevelToShowOnViewHighscore = "";
      }
    } else {
      i_pLevelInfoFrame->showWindow(false);
      m_pLevelToShowOnViewHighscore = "";
    }
  }

  void GameApp::viewHighscoreOf() {
    if(m_pWebHighscores == NULL) return;

    WebHighscore *pWH = m_pWebHighscores->getHighscoreFromLevel(m_pLevelToShowOnViewHighscore);
    if(pWH == NULL) return;

    std::vector<ReplayInfo *> *Replays = m_ReplayList.findReplays("", m_pLevelToShowOnViewHighscore);
    int i=0;

    /* search if the replay is already downloaded */
    bool found = false;
    while(i < Replays->size() && found == false) {
      if((*Replays)[i]->Name == pWH->getReplayName()) {
	found = true;
      }
      i++;
    }
    delete Replays;

    if(found == false) {
      if(m_bEnableWebHighscores) {
	try {
	  _SimpleMessage(GAMETEXT_DLHIGHSCORE,&m_InfoMsgBoxRect);
	  pWH->download();
	  
	  m_ReplayList.addReplay(FS::getFileBaseName(pWH->getReplayName()));
	  _UpdateReplaysList();

	  /* not very nice : make a new search to be sure the replay is here */
	  /* because it could have been downloaded but unplayable : for macosx for example */
	  std::vector<ReplayInfo *> *Replays = m_ReplayList.findReplays("", m_pLevelToShowOnViewHighscore);
	  int i=0;
	  /* search if the replay is already downloaded */
	  bool found = false;
	  while(i < Replays->size() && found == false) {
	    if((*Replays)[i]->Name == pWH->getReplayName()) {
	      found = true;
	    }
	    i++;
	  }
	  delete Replays;

	  if(found == false) {
	    notifyMsg(GAMETEXT_FAILEDTOLOADREPLAY);
	    return;
	  }

	} catch(Exception &e) {
	  notifyMsg(GAMETEXT_FAILEDDLREPLAY);
	  return;
	}
      } else {
	return;
      }
    }

    m_PlaySpecificReplay = pWH->getReplayName();      
  }
#endif

  void GameApp::_UpdateLevelPackLevelList() {
    UILevelList *pList = (UILevelList *)m_pLevelPackViewer->getChild("LEVELPACK_LEVEL_LIST");    

    /* Update levels informations */
    pList->updateLevelsInformations(m_pPlayer,
				    &m_Profiles
#if defined(SUPPORT_WEBACCESS)
				    , m_pWebHighscores
#endif
				    );
  }

}

