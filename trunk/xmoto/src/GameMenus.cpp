/*=============================================================================
XMOTO
Copyright (C) 2005 Rasmus Neckelmann (neckelmann@gmail.com)

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

  /*===========================================================================
  Create menus and hide them
  ===========================================================================*/
  void GameApp::_InitMenus(void) {
    /* TODO: it would obviously be a good idea to put this gui stuff into
       a nice XML file instead. This really stinks */
  
    /* Best times windows (at finish) */
    m_pBestTimes = new UIBestTimes(m_Renderer.getGUI(),10,50,"",290,500);
    m_pBestTimes->setFont(m_Renderer.getMediumFont());
    m_pBestTimes->setHFont(m_Renderer.getSmallFont());

    /* Initialize finish menu */
    m_pFinishMenu = new UIFrame(m_Renderer.getGUI(),300,60,"",400,480);
    m_pFinishMenu->setStyle(UI_FRAMESTYLE_MENU);
    
    m_pFinishMenuButtons[0] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_TRYAGAIN,207,57);
//    m_pFinishMenuButtons[1] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_SAVEREPLAY,207,57);
    m_pFinishMenuButtons[1] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_ABORT,207,57);
    m_pFinishMenuButtons[2] = new UIButton(m_pFinishMenu,0,0,GAMETEXT_QUIT,207,57);
    m_nNumFinishMenuButtons = 3;

    UIStatic *pFinishText = new UIStatic(m_pFinishMenu,0,95,GAMETEXT_FINISH,m_pFinishMenu->getPosition().nWidth,36);
    pFinishText->setFont(m_Renderer.getMediumFont());

    for(int i=0;i<m_nNumFinishMenuButtons;i++) {
      m_pFinishMenuButtons[i]->setPosition(200-207/2,10+m_pFinishMenu->getPosition().nHeight/2 - (m_nNumFinishMenuButtons*57)/2 + i*57,207,57);
      m_pFinishMenuButtons[i]->setFont(m_Renderer.getSmallFont());
    }

    m_pFinishMenu->setPrimaryChild(m_pFinishMenuButtons[0]); /* default button: Try Again (TODO: set this to "Play next") */
                      
    /* Initialize pause menu */
    m_pPauseMenu = new UIFrame(m_Renderer.getGUI(),getDispWidth()/2 - 200,100,"",400,480);
    m_pPauseMenu->setStyle(UI_FRAMESTYLE_MENU);
    
    m_pPauseMenuButtons[0] = new UIButton(m_pPauseMenu,0,0,GAMETEXT_RESUME,207,57);
    m_pPauseMenuButtons[1] = new UIButton(m_pPauseMenu,0,0,GAMETEXT_RESTART,207,57);
    m_pPauseMenuButtons[2] = new UIButton(m_pPauseMenu,0,0,GAMETEXT_ABORT,207,57);
    m_pPauseMenuButtons[3] = new UIButton(m_pPauseMenu,0,0,GAMETEXT_QUIT,207,57);
    m_nNumPauseMenuButtons = 4;

    UIStatic *pPauseText = new UIStatic(m_pPauseMenu,0,95,GAMETEXT_PAUSE,m_pPauseMenu->getPosition().nWidth,36);
    pPauseText->setFont(m_Renderer.getMediumFont());
    
    for(int i=0;i<m_nNumPauseMenuButtons;i++) {
      m_pPauseMenuButtons[i]->setPosition(200 -207/2,20+m_pPauseMenu->getPosition().nHeight/2 - (m_nNumPauseMenuButtons*57)/2 + i*57,207,57);
      m_pPauseMenuButtons[i]->setFont(m_Renderer.getSmallFont());
    }
    
    m_pPauseMenu->setPrimaryChild(m_pPauseMenuButtons[0]); /* default button: Resume */
    
    /* Initialize just-dead menu */
    m_pJustDeadMenu = new UIFrame(m_Renderer.getGUI(),getDispWidth()/2 - 200,100,"",400,400);
    m_pJustDeadMenu->setStyle(UI_FRAMESTYLE_MENU);
    
    m_pJustDeadMenuButtons[0] = new UIButton(m_pJustDeadMenu,0,0,GAMETEXT_TRYAGAIN,207,57);
    m_pJustDeadMenuButtons[1] = new UIButton(m_pJustDeadMenu,0,0,GAMETEXT_ABORT,207,57);
    m_pJustDeadMenuButtons[2] = new UIButton(m_pJustDeadMenu,0,0,GAMETEXT_QUIT,207,57);
    m_nNumJustDeadMenuButtons = 3;

    UIStatic *pJustDeadText = new UIStatic(m_pJustDeadMenu,0,85,GAMETEXT_JUSTDEAD,m_pJustDeadMenu->getPosition().nWidth,36);
    pJustDeadText->setFont(m_Renderer.getMediumFont());
    
    for(int i=0;i<m_nNumJustDeadMenuButtons;i++) {
      m_pJustDeadMenuButtons[i]->setPosition( 200 -207/2,20+m_pJustDeadMenu->getPosition().nHeight/2 - (m_nNumJustDeadMenuButtons*57)/2 + i*57,207,57);
      m_pJustDeadMenuButtons[i]->setFont(m_Renderer.getSmallFont());
    }

    m_pJustDeadMenu->setPrimaryChild(m_pJustDeadMenuButtons[0]); /* default button: Try Again */
    
    /* Initialize main menu */      
    m_pMainMenu = new UIWindow(m_Renderer.getGUI(),0,0,"",
                                m_Renderer.getGUI()->getPosition().nWidth,
                                m_Renderer.getGUI()->getPosition().nHeight);
    m_pMainMenu->showWindow(true);
    m_pMainMenu->enableWindow(true);                                 
    
    m_pMainMenuButtons[0] = new UIButton(m_pMainMenu,0,0,GAMETEXT_LEVELS,207,57);
//    m_pMainMenuButtons[1] = new UIButton(m_pMainMenu,0,0,GAMETEXT_VIEWREPLAYS,207,57);
//    m_pMainMenuButtons[1] = new UIButton(m_pMainMenu,0,0,GAMETEXT_BESTTIMES,207,57);
    m_pMainMenuButtons[1] = new UIButton(m_pMainMenu,0,0,GAMETEXT_OPTIONS,207,57);
    m_pMainMenuButtons[2] = new UIButton(m_pMainMenu,0,0,GAMETEXT_HELP,207,57);
    m_pMainMenuButtons[3] = new UIButton(m_pMainMenu,0,0,GAMETEXT_QUIT,207,57);
    m_nNumMainMenuButtons = 4;
        
//    UIStatic *pPlayerText = new UIStatic(m_pMainMenu,300,85,"",getDispWidth()-300-120,50);
    UIStatic *pPlayerText = new UIStatic(m_pMainMenu,300,(getDispHeight()*85)/600,"",getDispWidth()-300-120,50);
    pPlayerText->setFont(m_Renderer.getMediumFont());            
    pPlayerText->setHAlign(UI_ALIGN_RIGHT);
    pPlayerText->setID("PLAYERTAG");
    if(m_pPlayer != NULL) pPlayerText->setCaption(std::string(GAMETEXT_CURPLAYER) + m_pPlayer->PlayerName);
    
//    UIButton *pChangePlayerButton = new UIButton(m_pMainMenu,getDispWidth()-115,80,GAMETEXT_CHANGE,115,57);
    UIButton *pChangePlayerButton = new UIButton(m_pMainMenu,getDispWidth()-115,(getDispHeight()*80)/600,GAMETEXT_CHANGE,115,57);
    pChangePlayerButton->setType(UI_BUTTON_TYPE_SMALL);
    pChangePlayerButton->setFont(m_Renderer.getSmallFont());
    pChangePlayerButton->setID("CHANGEPLAYERBUTTON");
    
    UIStatic *pSomeText = new UIStatic(m_pMainMenu,0,getDispHeight()-20,
                                        std::string("X-Moto/") + getVersionString() + std::string(" ALPHA"),
                                        getDispWidth(),20);
    pSomeText->setFont(m_Renderer.getSmallFont());
    pSomeText->setVAlign(UI_ALIGN_BOTTOM);
    pSomeText->setHAlign(UI_ALIGN_LEFT);
    
    for(int i=0;i<m_nNumMainMenuButtons;i++) {
      m_pMainMenuButtons[i]->setPosition(47,getDispHeight()/2 - (m_nNumMainMenuButtons*57)/2 + i*57,207,57);
      m_pMainMenuButtons[i]->setFont(m_Renderer.getSmallFont());
    }           

    m_pMainMenu->setPrimaryChild(m_pMainMenuButtons[0]); /* default button: Play */
          
    m_pHelpWindow = new UIFrame(m_pMainMenu,300,(getDispHeight()*140)/600,"",getDispWidth()-300-20,getDispHeight()-40-(getDispHeight()*120)/600);
    m_pHelpWindow->showWindow(false);
    pSomeText = new UIStatic(m_pHelpWindow,0,0,GAMETEXT_HELP,m_pHelpWindow->getPosition().nWidth,36);
    pSomeText->setFont(m_Renderer.getMediumFont());
    pSomeText = new UIStatic(m_pHelpWindow,10,46,GAMETEXT_HELPTEXT,m_pHelpWindow->getPosition().nWidth-20,m_pHelpWindow->getPosition().nHeight-56);
    pSomeText->setFont(m_Renderer.getSmallFont());
    pSomeText->setVAlign(UI_ALIGN_TOP);
    pSomeText->setHAlign(UI_ALIGN_LEFT);
    
    m_pReplaysWindow = new UIFrame(m_pMainMenu,300,(getDispHeight()*140)/600,"",getDispWidth()-300-20,getDispHeight()-40-(getDispHeight()*120)/600);
    m_pReplaysWindow->showWindow(false);
    pSomeText = new UIStatic(m_pReplaysWindow,0,0,GAMETEXT_REPLAYS,m_pHelpWindow->getPosition().nWidth,36);
    pSomeText->setFont(m_Renderer.getMediumFont());      
    UIList *pReplaysList = new UIList(m_pReplaysWindow,20,40,"",m_pReplaysWindow->getPosition().nWidth-40,m_pReplaysWindow->getPosition().nHeight-115);
    pReplaysList->setFont(m_Renderer.getSmallFont());
    pReplaysList->addColumn(GAMETEXT_NAME,128);
    pReplaysList->addColumn(GAMETEXT_LEVEL,128);
    pReplaysList->addColumn(GAMETEXT_PLAYER,128);
    _CreateReplayList(pReplaysList);
    UIButton *pViewReplayButton = new UIButton(m_pReplaysWindow,11,m_pReplaysWindow->getPosition().nHeight-68,GAMETEXT_VIEW,115,57);
    pViewReplayButton->setFont(m_Renderer.getSmallFont());
    pViewReplayButton->setType(UI_BUTTON_TYPE_SMALL);
    
    m_pBestTimesWindow = new UIFrame(m_pMainMenu,300,(getDispHeight()*140)/600,"",getDispWidth()-300-20,getDispHeight()-40-(getDispHeight()*120)/600);      
    m_pBestTimesWindow->showWindow(false);
    pSomeText = new UIStatic(m_pBestTimesWindow,0,0,GAMETEXT_BESTTIMES,m_pHelpWindow->getPosition().nWidth,36);
    pSomeText->setFont(m_Renderer.getMediumFont());

    m_pPlayWindow = new UIFrame(m_pMainMenu,300,(getDispHeight()*140)/600,"",getDispWidth()-300-20,getDispHeight()-40-(getDispHeight()*120)/600);      
    m_pPlayWindow->showWindow(false);
    pSomeText = new UIStatic(m_pPlayWindow,0,0,GAMETEXT_CHOOSELEVEL,m_pHelpWindow->getPosition().nWidth,36);
    pSomeText->setFont(m_Renderer.getMediumFont());
    UITabView *pLevelTabs = new UITabView(m_pPlayWindow,20,40,"",m_pPlayWindow->getPosition().nWidth-40,m_pPlayWindow->getPosition().nHeight-115);
    pLevelTabs->setFont(m_Renderer.getSmallFont());
    pLevelTabs->setID("PLAY_LEVEL_TABS");  
    UIButton *pGoButton = new UIButton(m_pPlayWindow,11,m_pPlayWindow->getPosition().nHeight-68,GAMETEXT_STARTLEVEL,115,57);
    pGoButton->setFont(m_Renderer.getSmallFont());
    pGoButton->setType(UI_BUTTON_TYPE_SMALL);
    pGoButton->setID("PLAY_GO_BUTTON");
    UIWindow *pInternalLevelsTab = new UIWindow(pLevelTabs,20,40,GAMETEXT_BUILTINLEVELS,pLevelTabs->getPosition().nWidth-40,pLevelTabs->getPosition().nHeight-60);
    pInternalLevelsTab->enableWindow(true);
    pInternalLevelsTab->setID("PLAY_INTERNAL_LEVELS_TAB");
    UIWindow *pExternalLevelsTab = new UIWindow(pLevelTabs,20,40,GAMETEXT_EXTERNALLEVELS,pLevelTabs->getPosition().nWidth-40,pLevelTabs->getPosition().nHeight-60);
    pExternalLevelsTab->enableWindow(true);
    pExternalLevelsTab->showWindow(false);
    pExternalLevelsTab->setID("PLAY_EXTERNAL_LEVELS_TAB");
    UIList *pInternalLevelsList = new UIList(pInternalLevelsTab,0,0,"",pInternalLevelsTab->getPosition().nWidth,pInternalLevelsTab->getPosition().nHeight);      
    pInternalLevelsList->setID("PLAY_INTERNAL_LEVELS_LIST");
    pInternalLevelsList->showWindow(true);
    pInternalLevelsTab->showWindow(true);
    pInternalLevelsList->setFont(m_Renderer.getSmallFont());
    pInternalLevelsList->addColumn(GAMETEXT_LEVEL,128);
    pInternalLevelsList->setEnterButton( pGoButton );
    UIList *pExternalLevelsList = new UIList(pExternalLevelsTab,0,0,"",pExternalLevelsTab->getPosition().nWidth,pExternalLevelsTab->getPosition().nHeight);      /* -64 to make room for bonus */
    pExternalLevelsList->setID("PLAY_EXTERNAL_LEVELS_LIST");
    pExternalLevelsList->setFont(m_Renderer.getSmallFont());
    pExternalLevelsList->addColumn(GAMETEXT_LEVEL,250);
    pExternalLevelsList->addColumn(GAMETEXT_FILE,250);
    pExternalLevelsList->setEnterButton( pGoButton );        

    //m_pPlayWindow->setPrimaryChild(m_pJustDeadMenuButtons[0]); /* default button: Try Again */

    m_pOptionsWindow = new UIFrame(m_pMainMenu,300,(getDispHeight()*140)/600,"",getDispWidth()-300-20,getDispHeight()-40-(getDispHeight()*120)/600);
    m_pOptionsWindow->showWindow(false);
    pSomeText = new UIStatic(m_pOptionsWindow,0,0,GAMETEXT_OPTIONS,m_pHelpWindow->getPosition().nWidth,36);
    pSomeText->setFont(m_Renderer.getMediumFont());
    UITabView *pOptionsTabs  = new UITabView(m_pOptionsWindow,20,40,"",m_pOptionsWindow->getPosition().nWidth-40,m_pOptionsWindow->getPosition().nHeight-115);
    pOptionsTabs->setID("OPTIONS_TABS");
    pOptionsTabs->setFont(m_Renderer.getSmallFont());
    UIButton *pSaveOptionsButton = new UIButton(m_pOptionsWindow,11,m_pReplaysWindow->getPosition().nHeight-68,GAMETEXT_SAVE,115,57);
    pSaveOptionsButton->setID("SAVE_BUTTON");
    pSaveOptionsButton->setFont(m_Renderer.getSmallFont());
    pSaveOptionsButton->setType(UI_BUTTON_TYPE_SMALL);
    UIButton *pDefaultOptionsButton = new UIButton(m_pOptionsWindow,126,m_pReplaysWindow->getPosition().nHeight-68,GAMETEXT_DEFAULTS,115,57);
    pDefaultOptionsButton->setID("DEFAULTS_BUTTON");
    pDefaultOptionsButton->setFont(m_Renderer.getSmallFont());
    pDefaultOptionsButton->setType(UI_BUTTON_TYPE_SMALL);      
    UIWindow *pGeneralOptionsTab = new UIWindow(pOptionsTabs,20,40,GAMETEXT_GENERAL,pOptionsTabs->getPosition().nWidth-40,pOptionsTabs->getPosition().nHeight);
    pGeneralOptionsTab->enableWindow(true);
    pGeneralOptionsTab->showWindow(true);
    pGeneralOptionsTab->setID("GENERAL_TAB");

    UIButton *pShowMiniMap = new UIButton(pGeneralOptionsTab,5,5,GAMETEXT_SHOWMINIMAP,(pGeneralOptionsTab->getPosition().nWidth-40)/2,28);
    pShowMiniMap->setType(UI_BUTTON_TYPE_CHECK);
    pShowMiniMap->setID("SHOWMINIMAP");
    pShowMiniMap->enableWindow(true);
    pShowMiniMap->setFont(m_Renderer.getSmallFont());
    pShowMiniMap->setGroup(50023);
    
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

    UIButton *p32BitsPerPixel = new UIButton(pVideoOptionsTab,5 + ((pVideoOptionsTab->getPosition().nWidth-40)/2)*1,5,GAMETEXT_32BPP,(pVideoOptionsTab->getPosition().nWidth-40)/2,28);
    p32BitsPerPixel->setType(UI_BUTTON_TYPE_RADIO);
    p32BitsPerPixel->setID("32BPP");
    p32BitsPerPixel->enableWindow(true);
    p32BitsPerPixel->setFont(m_Renderer.getSmallFont());
    p32BitsPerPixel->setGroup(20023);
    
    UIList *pDispResList = new UIList(pVideoOptionsTab,5,43,"",pVideoOptionsTab->getPosition().nWidth-10,128);
    pDispResList->setID("RES_LIST");
    pDispResList->setFont(m_Renderer.getSmallFont());
    pDispResList->addColumn(GAMETEXT_SCREENRES,pDispResList->getPosition().nWidth);
    //pDispResList->addEntry("512 X 384");
    //pDispResList->addEntry("640 X 480");
    pDispResList->addEntry("800 X 600");
    pDispResList->addEntry("1024 X 768");
    pDispResList->addEntry("1280 X 1024");
    pDispResList->addEntry("1600 X 1200");

    UIButton *pRunWindowed = new UIButton(pVideoOptionsTab,5,180,GAMETEXT_RUNWINDOWED,(pVideoOptionsTab->getPosition().nWidth-40)/1,28);
    pRunWindowed->setType(UI_BUTTON_TYPE_CHECK);
    pRunWindowed->setID("RUN_WINDOWED");
    pRunWindowed->enableWindow(true);
    pRunWindowed->setFont(m_Renderer.getSmallFont());
    
    pSomeText = new UIStatic(pVideoOptionsTab,5,208,GAMETEXT_MENUGFX,120,28);
    pSomeText->setFont(m_Renderer.getSmallFont());    
    pSomeText->enableWindow(true);
    pSomeText->showWindow(true);

    UIButton *pMenuLow = new UIButton(pVideoOptionsTab,120,208,GAMETEXT_LOW,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pMenuLow->setType(UI_BUTTON_TYPE_RADIO);
    pMenuLow->setID("MENULOW");
    pMenuLow->enableWindow(true);
    pMenuLow->setFont(m_Renderer.getSmallFont());
    pMenuLow->setGroup(20024);

    UIButton *pMenuMed = new UIButton(pVideoOptionsTab,120+((pVideoOptionsTab->getPosition().nWidth-120)/3)*1,208,GAMETEXT_MEDIUM,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pMenuMed->setType(UI_BUTTON_TYPE_RADIO);
    pMenuMed->setID("MENUMEDIUM");
    pMenuMed->enableWindow(true);
    pMenuMed->setFont(m_Renderer.getSmallFont());
    pMenuMed->setGroup(20024);

    UIButton *pMenuHigh = new UIButton(pVideoOptionsTab,120+((pVideoOptionsTab->getPosition().nWidth-120)/3)*2,208,GAMETEXT_HIGH,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pMenuHigh->setType(UI_BUTTON_TYPE_RADIO);
    pMenuHigh->setID("MENUHIGH");
    pMenuHigh->enableWindow(true);
    pMenuHigh->setFont(m_Renderer.getSmallFont());
    pMenuHigh->setGroup(20024);

    pSomeText = new UIStatic(pVideoOptionsTab,5,236,GAMETEXT_GAMEGFX,120,28);
    pSomeText->setFont(m_Renderer.getSmallFont());    
    pSomeText->enableWindow(true);
    pSomeText->showWindow(true);

    UIButton *pGameLow = new UIButton(pVideoOptionsTab,120,236,GAMETEXT_LOW,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pGameLow->setType(UI_BUTTON_TYPE_RADIO);
    pGameLow->setID("GAMELOW");
    pGameLow->enableWindow(true);
    pGameLow->setFont(m_Renderer.getSmallFont());
    pGameLow->setGroup(20025);

    UIButton *pGameMed = new UIButton(pVideoOptionsTab,120+((pVideoOptionsTab->getPosition().nWidth-120)/3)*1,236,GAMETEXT_MEDIUM,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pGameMed->setType(UI_BUTTON_TYPE_RADIO);
    pGameMed->setID("GAMEMEDIUM");
    pGameMed->enableWindow(true);
    pGameMed->setFont(m_Renderer.getSmallFont());
    pGameMed->setGroup(20025);

    UIButton *pGameHigh = new UIButton(pVideoOptionsTab,120+((pVideoOptionsTab->getPosition().nWidth-120)/3)*2,236,GAMETEXT_HIGH,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pGameHigh->setType(UI_BUTTON_TYPE_RADIO);
    pGameHigh->setID("GAMEHIGH");
    pGameHigh->enableWindow(true);
    pGameHigh->setFont(m_Renderer.getSmallFont());
    pGameHigh->setGroup(20025);

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
    
    UIButton *pSampleRate11Button = new UIButton(pAudioOptionsTab,25,33,GAMETEXT_11KHZ,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pSampleRate11Button->setType(UI_BUTTON_TYPE_RADIO);
    pSampleRate11Button->setID("RATE11KHZ");
    pSampleRate11Button->enableWindow(true);
    pSampleRate11Button->setFont(m_Renderer.getSmallFont());
    pSampleRate11Button->setGroup(10023);
    
    UIButton *pSampleRate22Button = new UIButton(pAudioOptionsTab,25 + ((pAudioOptionsTab->getPosition().nWidth-40)/3)*1,33,GAMETEXT_22KHZ,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pSampleRate22Button->setType(UI_BUTTON_TYPE_RADIO);
    pSampleRate22Button->setID("RATE22KHZ");
    pSampleRate22Button->enableWindow(true);
    pSampleRate22Button->setFont(m_Renderer.getSmallFont());
    pSampleRate22Button->setGroup(10023);
    
    UIButton *pSampleRate44Button = new UIButton(pAudioOptionsTab,25 + ((pAudioOptionsTab->getPosition().nWidth-40)/3)*2,33,GAMETEXT_44KHZ,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pSampleRate44Button->setType(UI_BUTTON_TYPE_RADIO);
    pSampleRate44Button->setID("RATE44KHZ");
    pSampleRate44Button->enableWindow(true);
    pSampleRate44Button->setFont(m_Renderer.getSmallFont());
    pSampleRate44Button->setGroup(10023);

    UIButton *pSample8Button = new UIButton(pAudioOptionsTab,25,61,GAMETEXT_8BIT,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pSample8Button->setType(UI_BUTTON_TYPE_RADIO);
    pSample8Button->setID("8BIT");
    pSample8Button->enableWindow(true);
    pSample8Button->setFont(m_Renderer.getSmallFont());
    pSample8Button->setGroup(10024);

    UIButton *pSample16Button = new UIButton(pAudioOptionsTab,25 + ((pAudioOptionsTab->getPosition().nWidth-40)/3)*1,61,GAMETEXT_16BIT,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);    
    pSample16Button->setType(UI_BUTTON_TYPE_RADIO);
    pSample16Button->setID("16BIT");
    pSample16Button->enableWindow(true);
    pSample16Button->setFont(m_Renderer.getSmallFont());
    pSample16Button->setGroup(10024);

    UIButton *pMonoButton = new UIButton(pAudioOptionsTab,25,89,GAMETEXT_MONO,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pMonoButton->setType(UI_BUTTON_TYPE_RADIO);
    pMonoButton->setID("MONO");
    pMonoButton->enableWindow(true);
    pMonoButton->setFont(m_Renderer.getSmallFont());
    pMonoButton->setGroup(10025);
    
    UIButton *pStereoButton = new UIButton(pAudioOptionsTab,25 + ((pAudioOptionsTab->getPosition().nWidth-40)/3)*1,89,GAMETEXT_STEREO,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pStereoButton->setType(UI_BUTTON_TYPE_RADIO);
    pStereoButton->setID("STEREO");
    pStereoButton->enableWindow(true);
    pStereoButton->setFont(m_Renderer.getSmallFont());
    pStereoButton->setGroup(10025);

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

    UIButton *pConfigureJoystick = new UIButton(pControlsOptionsTab,0,180,GAMETEXT_CONFIGUREJOYSTICK,207,57);
    pConfigureJoystick->setType(UI_BUTTON_TYPE_LARGE);
    pConfigureJoystick->setID("CONFIGURE_JOYSTICK");
    pConfigureJoystick->enableWindow(true);
    pConfigureJoystick->setFont(m_Renderer.getSmallFont());

    /* Initialize profile editor */
    m_pProfileEditor = new UIFrame(m_Renderer.getGUI(),getDispWidth()/2-350,getDispHeight()/2-250,"",700,500); 
    m_pProfileEditor->setStyle(UI_FRAMESTYLE_TRANS);           
    UIStatic *pProfileEditorTitle = new UIStatic(m_pProfileEditor,0,0,GAMETEXT_PLAYERPROFILES,700,50);
    pProfileEditorTitle->setFont(m_Renderer.getMediumFont());
    UIList *pProfileList = new UIList(m_pProfileEditor,20,50,"",400,430);
    pProfileList->setFont(m_Renderer.getSmallFont());
    pProfileList->addColumn(GAMETEXT_PLAYERPROFILE,128);      
    pProfileList->setID("PROFILE_LIST");
    UIButton *pProfUseButton = new UIButton(m_pProfileEditor,450,50,GAMETEXT_USEPROFILE,207,57);
    pProfUseButton->setFont(m_Renderer.getSmallFont());
    pProfUseButton->setID("USEPROFILE_BUTTON");
    UIButton *pProfNewButton = new UIButton(m_pProfileEditor,450,107,GAMETEXT_NEWPROFILE,207,57);
    pProfNewButton->setFont(m_Renderer.getSmallFont());
    pProfNewButton->setID("NEWPROFILE_BUTTON");
    UIButton *pProfCancelButton = new UIButton(m_pProfileEditor,450,164,GAMETEXT_CLOSE,207,57);
    pProfCancelButton->setFont(m_Renderer.getSmallFont());
    pProfCancelButton->setID("CANCEL_BUTTON");
    UIButton *pProfDeleteButton = new UIButton(m_pProfileEditor,450,423,GAMETEXT_DELETEPROFILE,207,57);
    pProfDeleteButton->setFont(m_Renderer.getSmallFont());
    pProfDeleteButton->setID("DELETEPROFILE_BUTTON");
    _CreateProfileList();
    
    /* Hide menus */
    m_pMainMenu->showWindow(false);
    m_pPauseMenu->showWindow(false);
    m_pJustDeadMenu->showWindow(false);
    m_pProfileEditor->showWindow(false);
    m_pFinishMenu->showWindow(false);
    m_pBestTimes->showWindow(false);
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
  }
  
  /*===========================================================================
  Update pause menu
  ===========================================================================*/
  void GameApp::_HandlePauseMenu(void) {
    /* Any of the pause menu buttons clicked? */
    for(int i=0;i<m_nNumPauseMenuButtons;i++) {
      if(m_pPauseMenuButtons[i]->isClicked()) {
        if(m_pPauseMenuButtons[i]->getCaption() == GAMETEXT_QUIT) {
          if(m_pQuitMsgBox == NULL)
            m_pQuitMsgBox = m_Renderer.getGUI()->msgBox(GAMETEXT_QUITMESSAGE,
                                                        (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
        }
        else if(m_pPauseMenuButtons[i]->getCaption() == GAMETEXT_ABORT) {
          if(m_pReplay != NULL) delete m_pReplay;
          m_pReplay = NULL;
          m_pPauseMenu->showWindow(false);
          m_MotoGame.endLevel();
          m_Renderer.unprepareForNewLevel();
          setState(GS_MENU);          
        }
        else if(m_pPauseMenuButtons[i]->getCaption() == GAMETEXT_RESTART) {
          if(m_pReplay != NULL) delete m_pReplay;
          m_pReplay = NULL;
          m_pPauseMenu->showWindow(false);
          m_MotoGame.endLevel();
          m_Renderer.unprepareForNewLevel();
          setState(GS_PLAYING);                               
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
    /* Any of the finish menu buttons clicked? */
    for(int i=0;i<m_nNumFinishMenuButtons;i++) {
      if(m_pFinishMenuButtons[i]->isClicked()) {
        if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_QUIT) {
          if(m_pQuitMsgBox == NULL)
            m_pQuitMsgBox = m_Renderer.getGUI()->msgBox(GAMETEXT_QUITMESSAGE,
                                                        (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
        }
        else if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_TRYAGAIN) {
          if(m_pReplay != NULL) delete m_pReplay;
          m_pReplay = NULL;
          LevelSrc *pCurLevel = m_MotoGame.getLevelSrc();
          m_PlaySpecificLevel = pCurLevel->getID();
          m_pFinishMenu->showWindow(false);
          m_pBestTimes->showWindow(false);
          m_MotoGame.endLevel();
          m_Renderer.unprepareForNewLevel();
          setState(GS_PLAYING);           
        }
        else if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_ABORT) {
          if(m_pReplay != NULL) delete m_pReplay;
          m_pReplay = NULL;
          m_pFinishMenu->showWindow(false);
          m_pBestTimes->showWindow(false);
          m_MotoGame.endLevel();
          m_Renderer.unprepareForNewLevel();
          setState(GS_MENU);
        }

        /* Don't process this clickin' more than once */
        m_pFinishMenuButtons[i]->setClicked(false);
      }
    }
  }

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
              pList->setSelected(0);

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

      _UpdateLevelLists();
                  
      m_pProfileEditor->showWindow(false);
      m_pMainMenu->enableChildren(true);
      m_pMainMenu->enableWindow(true);

      setState(GS_MENU);
      
      UIStatic *pPlayerTag = reinterpret_cast<UIStatic *>(m_pMainMenu->getChild("PLAYERTAG"));
      if(pPlayerTag) {
        pPlayerTag->setCaption(std::string(GAMETEXT_CURPLAYER) + m_pPlayer->PlayerName);
      }                   
    }    
    else if(pDeleteButton->isClicked()) {
      if(m_pDeleteProfileMsgBox == NULL)
        m_pDeleteProfileMsgBox = m_Renderer.getGUI()->msgBox(GAMETEXT_DELETEPLAYERMESSAGE,
                                                          (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
    }
    else if(pNewButton->isClicked()) {
      if(m_pNewProfileMsgBox == NULL) {
        m_pNewProfileMsgBox = m_Renderer.getGUI()->msgBox(GAMETEXT_ENTERPLAYERNAME,
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
            _UpdateLevelLists();
          }
        
          pPlayerTag->setCaption(std::string(GAMETEXT_CURPLAYER) + m_pPlayer->PlayerName);
        }       
      }
    }
  }
  
  /*===========================================================================
  Update just-dead menu
  ===========================================================================*/
  void GameApp::_HandleJustDeadMenu(void) {
    /* Any of the just-dead menu buttons clicked? */
    for(int i=0;i<m_nNumJustDeadMenuButtons;i++) {
      if(m_pJustDeadMenuButtons[i]->isClicked()) {
        if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_QUIT) {
          if(m_pQuitMsgBox == NULL)
            m_pQuitMsgBox = m_Renderer.getGUI()->msgBox(GAMETEXT_QUITMESSAGE,
                                                        (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
        }
        else if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_TRYAGAIN) {
          if(m_pReplay != NULL) delete m_pReplay;
          m_pReplay = NULL;
          LevelSrc *pCurLevel = m_MotoGame.getLevelSrc();
          m_PlaySpecificLevel = pCurLevel->getID();
          m_pJustDeadMenu->showWindow(false);
          m_MotoGame.endLevel();
          m_Renderer.unprepareForNewLevel();          
          setState(GS_PLAYING);          
        }
        else if(m_pJustDeadMenuButtons[i]->getCaption() == GAMETEXT_ABORT) {
          if(m_pReplay != NULL) delete m_pReplay;
          m_pReplay = NULL;
          m_pJustDeadMenu->showWindow(false);
          m_MotoGame.endLevel();
          m_Renderer.unprepareForNewLevel();
          setState(GS_MENU);
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
          m_pOptionsWindow->showWindow(false);
          m_pHelpWindow->showWindow(false);
          m_pReplaysWindow->showWindow(false);
          m_pBestTimesWindow->showWindow(false);
          m_pPlayWindow->showWindow(true);                    
        }
        else if(m_pMainMenuButtons[i]->getCaption() == GAMETEXT_VIEWREPLAYS) {        
          m_pOptionsWindow->showWindow(false);
          m_pHelpWindow->showWindow(false);
          m_pReplaysWindow->showWindow(true);
          m_pBestTimesWindow->showWindow(false);
          m_pPlayWindow->showWindow(false);
        }
        else if(m_pMainMenuButtons[i]->getCaption() == GAMETEXT_OPTIONS) {
          if(m_pOptionsWindow->isHidden()) _ImportOptions();        
          m_pOptionsWindow->showWindow(true);
          m_pHelpWindow->showWindow(false);
          m_pReplaysWindow->showWindow(false);
          m_pBestTimesWindow->showWindow(false);
          m_pPlayWindow->showWindow(false);
        }
        else if(m_pMainMenuButtons[i]->getCaption() == GAMETEXT_HELP) {
          m_pOptionsWindow->showWindow(false);
          m_pHelpWindow->showWindow(true);
          m_pReplaysWindow->showWindow(false);
          m_pBestTimesWindow->showWindow(false);
          m_pPlayWindow->showWindow(false);
        }
        else if(m_pMainMenuButtons[i]->getCaption() == GAMETEXT_BESTTIMES) {
          m_pOptionsWindow->showWindow(false);
          m_pHelpWindow->showWindow(false);
          m_pReplaysWindow->showWindow(false);
          m_pBestTimesWindow->showWindow(true);
          m_pPlayWindow->showWindow(false);
        }
        else if(m_pMainMenuButtons[i]->getCaption() == GAMETEXT_QUIT) {
          if(m_pQuitMsgBox == NULL)
            m_pQuitMsgBox = m_Renderer.getGUI()->msgBox(GAMETEXT_QUITMESSAGE,
                                                        (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
        }
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
    
    /* OPTIONS */
    UIButton *pEnableAudioButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:ENABLE_AUDIO");
    UIButton *p11kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE11KHZ");
    UIButton *p22kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE22KHZ");
    UIButton *p44kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE44KHZ");
    UIButton *pSample8Button = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:8BIT");
    UIButton *pSample16Button = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:16BIT");
    UIButton *pMonoButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:MONO");
    UIButton *pStereoButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:STEREO");
        
    if(pEnableAudioButton) {
      bool t=pEnableAudioButton->getChecked();
      p11kHzButton->enableWindow(t);
      p22kHzButton->enableWindow(t);
      p44kHzButton->enableWindow(t);      
      pSample8Button->enableWindow(t);
      pSample16Button->enableWindow(t);
      pMonoButton->enableWindow(t);
      pStereoButton->enableWindow(t);
    }
    
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
    
    /* PLAY */
    UIButton *pPlayGoButton = (UIButton *)m_pPlayWindow->getChild("PLAY_GO_BUTTON");
    UIList *pPlayExternalLevelsList = (UIList *)m_pPlayWindow->getChild("PLAY_LEVEL_TABS:PLAY_EXTERNAL_LEVELS_TAB:PLAY_EXTERNAL_LEVELS_LIST");
    UIList *pPlayInternalLevelsList = (UIList *)m_pPlayWindow->getChild("PLAY_LEVEL_TABS:PLAY_INTERNAL_LEVELS_TAB:PLAY_INTERNAL_LEVELS_LIST");

    if(pPlayGoButton && pPlayGoButton->isClicked()) {
      pPlayGoButton->setClicked(false);
      
      /* Find out what to play */
      LevelSrc *pLevelSrc = NULL;
      if(pPlayInternalLevelsList && !pPlayInternalLevelsList->isBranchHidden() && pPlayInternalLevelsList->getSelected()>=0) {
        /* Play selected internal level */
        if(!pPlayInternalLevelsList->getEntries().empty()) {
          UIListEntry *pEntry = pPlayInternalLevelsList->getEntries()[pPlayInternalLevelsList->getSelected()];
          pLevelSrc = reinterpret_cast<LevelSrc *>(pEntry->pvUser);        
        }
      }
      else if(pPlayExternalLevelsList && !pPlayExternalLevelsList->isBranchHidden() && pPlayExternalLevelsList->getSelected()>=0) {
        /* Play selected external level */
        if(!pPlayExternalLevelsList->getEntries().empty()) {
          UIListEntry *pEntry = pPlayExternalLevelsList->getEntries()[pPlayExternalLevelsList->getSelected()];        
          pLevelSrc = reinterpret_cast<LevelSrc *>(pEntry->pvUser);
        }
      }
      
      /* Start playing it */
      if(pLevelSrc != NULL) {
        m_pMainMenu->showWindow(false);      
        m_PlaySpecificLevel = pLevelSrc->getID();
        setState(GS_PLAYING);
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
  
  void GameApp::_SimpleMessage(const std::string &Msg) {      
    m_Renderer.getGUI()->paint();
    drawBox(Vector2f(0,0),Vector2f(getDispWidth(),getDispHeight()),0,MAKE_COLOR(0,0,0,170),0);
    int cx,cy;

    m_Renderer.getGUI()->setFont(m_Renderer.getMediumFont());
    m_Renderer.getGUI()->getTextSize(Msg.c_str(),&cx,&cy);
    
    int nW = cx + 150, nH = cy + 150;
    int nx = getDispWidth()/2 - nW/2,ny = getDispHeight()/2 - nH/2;

    m_Renderer.getGUI()->putElem(nx,ny,-1,-1,UI_ELEM_FRAME_TL,false);
    m_Renderer.getGUI()->putElem(nx+nW-8,ny,-1,-1,UI_ELEM_FRAME_TR,false);
    m_Renderer.getGUI()->putElem(nx+nW-8,ny+nH-8,-1,-1,UI_ELEM_FRAME_BR,false);
    m_Renderer.getGUI()->putElem(nx,ny+nH-8,-1,-1,UI_ELEM_FRAME_BL,false);
    m_Renderer.getGUI()->putElem(nx+8,ny,nW-16,-1,UI_ELEM_FRAME_TM,false);
    m_Renderer.getGUI()->putElem(nx+8,ny+nH-8,nW-16,-1,UI_ELEM_FRAME_BM,false);
    m_Renderer.getGUI()->putElem(nx,ny+8,-1,nH-16,UI_ELEM_FRAME_ML,false);
    m_Renderer.getGUI()->putElem(nx+nW-8,ny+8,-1,nH-16,UI_ELEM_FRAME_MR,false);
    m_Renderer.getGUI()->putRect(nx+8,ny+8,nW-16,nH-16,MAKE_COLOR(0,0,0,127));

    m_Renderer.getGUI()->putText(getDispWidth()/2 - cx/2,getDispHeight()/2,Msg.c_str());
    m_Renderer.getGUI()->setFont(m_Renderer.getSmallFont());
    
    SDL_GL_SwapBuffers();
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
            sprintf(cBuf,GAMETEXT_PRESSANYKEYTO
                         "\n"
                         GAMETEXT_ALREADYUSED,pActionList->getEntries()[nSel]->Text[0].c_str());
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
          pList->setSelected(i);
      
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
  Find all replays and add the to the list
  ===========================================================================*/
  void GameApp::_CreateReplayList(UIList *pList) {
    std::vector<ReplayInfo *> Replays = Replay::probeReplays();
    
    for(int i=0;i<Replays.size();i++) {
      UIListEntry *pEntry = pList->addEntry(Replays[i]->Name);
      pEntry->Text.push_back(Replays[i]->LevelID);
      pEntry->Text.push_back(Replays[i]->PlayerName);
    }
    
    Replay::freeReplayList(Replays);
  }

  /*===========================================================================
  Scan through loaded levels
  ===========================================================================*/
  void GameApp::_CreateLevelLists(UIList *pExternalLevels,UIList *pInternalLevels) {
    /* List of internal levels. Yeah, to let the user see a difference between
       the built-ins and the ones he has installed himself, here's a list of
       all those which should be considered internal */    
    pExternalLevels->clear();
    pInternalLevels->clear();
    
    if(m_pPlayer == NULL) return;
  
    for(int i=0;i<m_nNumLevels;i++) {
      LevelSrc *pLevel = &m_Levels[i];
      
      /* Internal or external? */
      bool bInternal = m_Profiles.isInternal(pLevel->getID());
      
      std::string Name,File;
      
      if(pLevel->getLevelInfo()->Name != "") Name = pLevel->getLevelInfo()->Name;
      else Name = "???";

      if(pLevel->getFileName() != "") File = FS::getFileBaseName(pLevel->getFileName());
      else File = "???";
      
      if(bInternal) {
        /* Consider it internal */
        /* Skipped or completed? */
        std::string Tag="";

        if(m_Profiles.isLevelCompleted(m_pPlayer->PlayerName,pLevel->getID()))
          Tag=GAMETEXT_COMPLETED;
        else if(m_Profiles.isLevelSkipped(m_pPlayer->PlayerName,pLevel->getID()))
          Tag=GAMETEXT_SKIPPED;

//        printf("[%s][%s]%s\n",m_pPlayer->PlayerName.c_str(),pLevel->getID().c_str(),Tag.c_str());
        
        pInternalLevels->addEntry(Name+Tag,reinterpret_cast<void *>(pLevel));
      }
      else {
        /* Consider it external */
        UIListEntry *pEntry = pExternalLevels->addEntry(Name,reinterpret_cast<void *>(pLevel));
        pEntry->Text.push_back(File);
      }
    }
  }

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
  
    UIButton *pEnableAudioButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:ENABLE_AUDIO");
    UIButton *p11kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE11KHZ");
    UIButton *p22kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE22KHZ");
    UIButton *p44kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE44KHZ");
    UIButton *pSample8Button = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:8BIT");
    UIButton *pSample16Button = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:16BIT");
    UIButton *pMonoButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:MONO");
    UIButton *pStereoButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:STEREO");

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
    
    pShowMiniMap->setChecked(m_Config.getBool("ShowMiniMap"));
    
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
      pResList->setSelected(0);
    }
    else {
      pResList->setSelected(nMode);
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
    
    m_Config.setValue("ControllerMode1",m_Config.getDefaultValue("ControllerMode1"));
    m_Config.setValue("KeyDrive1",m_Config.getDefaultValue("KeyDrive1"));
    m_Config.setValue("KeyBrake1",m_Config.getDefaultValue("KeyBrake1"));
    m_Config.setValue("KeyFlipLeft1",m_Config.getDefaultValue("KeyFlipLeft1"));
    m_Config.setValue("KeyFlipRight1",m_Config.getDefaultValue("KeyFlipRight1"));
    m_Config.setValue("KeyChangeDir1",m_Config.getDefaultValue("KeyChangeDir1"));

    /* The following require restart */
    m_Config.setChanged(false);      
        
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

    UIButton *pEnableAudioButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:ENABLE_AUDIO");
    UIButton *p11kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE11KHZ");
    UIButton *p22kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE22KHZ");
    UIButton *p44kHzButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:RATE44KHZ");
    UIButton *pSample8Button = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:8BIT");
    UIButton *pSample16Button = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:16BIT");
    UIButton *pMonoButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:MONO");
    UIButton *pStereoButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:AUDIO_TAB:STEREO");
    
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
    
    if(pMenuLow->getChecked()) m_Config.setString("MenuBackgroundGraphics","Low");
    else if(pMenuMed->getChecked()) m_Config.setString("MenuBackgroundGraphics","Medium");
    else if(pMenuHigh->getChecked()) m_Config.setString("MenuBackgroundGraphics","High");
    
    if(pGameLow->getChecked()) m_Config.setString("GameGraphics","Low");
    else if(pGameMed->getChecked()) m_Config.setString("GameGraphics","Medium");
    else if(pGameHigh->getChecked()) m_Config.setString("GameGraphics","High");
    
    if(pKeyboardControl->getChecked()) m_Config.setString("ControllerMode1","Keyboard");
    else if(pJoystickControl->getChecked()) m_Config.setString("ControllerMode1","Joystick1");
    
    for(int i=0;i<pActionList->getEntries().size();i++) {
      if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_DRIVE) m_Config.setString("KeyDrive1",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_BRAKE) m_Config.setString("KeyBrake1",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_FLIPLEFT) m_Config.setString("KeyFlipLeft1",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_FLIPRIGHT) m_Config.setString("KeyFlipRight1",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_CHANGEDIR) m_Config.setString("KeyChangeDir1",pActionList->getEntries()[i]->Text[1]);
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
  
};

