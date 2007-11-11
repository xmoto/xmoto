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

#include "PhysSettings.h"
#include "states/StateEditProfile.h"
#include "states/StateLevelInfoViewer.h"
#include "states/StateReplaying.h"
#include "states/StatePreplaying.h"
#include "states/StateCreditsMode.h"
#include "states/StateLevelPackViewer.h"
#include "states/StateEditWebConfig.h"

  UIFrame* GameApp::makeOptionsWindow(DrawLib* i_drawLib, UIWindow* io_parent, UserConfig* i_Config) {
    UIFrame *v_pOptionsWindow;
    UIStatic *pSomeText;

    v_pOptionsWindow = new UIFrame(io_parent, 220,(i_drawLib->getDispHeight()*140)/600,"",i_drawLib->getDispWidth()-220-20,i_drawLib->getDispHeight()-40-(i_drawLib->getDispHeight()*120)/600-10);
    v_pOptionsWindow->showWindow(false);
    pSomeText = new UIStatic(v_pOptionsWindow,0,0,GAMETEXT_OPTIONS,v_pOptionsWindow->getPosition().nWidth,36);
    pSomeText->setFont(i_drawLib->getFontMedium());
    UITabView *pOptionsTabs  = new UITabView(v_pOptionsWindow,20,40,"",v_pOptionsWindow->getPosition().nWidth-40,v_pOptionsWindow->getPosition().nHeight-115);
    pOptionsTabs->setID("OPTIONS_TABS");
    pOptionsTabs->setFont(i_drawLib->getFontSmall());
    pOptionsTabs->setTabContextHelp(0,CONTEXTHELP_GENERAL_OPTIONS);
    pOptionsTabs->setTabContextHelp(1,CONTEXTHELP_VIDEO_OPTIONS);
    pOptionsTabs->setTabContextHelp(2,CONTEXTHELP_AUDIO_OPTIONS);
    pOptionsTabs->setTabContextHelp(3,CONTEXTHELP_CONTROL_OPTIONS);
    UIButton *pSaveOptionsButton = new UIButton(v_pOptionsWindow,11,v_pOptionsWindow->getPosition().nHeight-68,GAMETEXT_SAVE,115,57);
    pSaveOptionsButton->setID("SAVE_BUTTON");
    pSaveOptionsButton->setFont(i_drawLib->getFontSmall());
    pSaveOptionsButton->setType(UI_BUTTON_TYPE_SMALL);
    pSaveOptionsButton->setContextHelp(CONTEXTHELP_SAVE_OPTIONS);
    UIButton *pDefaultOptionsButton = new UIButton(v_pOptionsWindow,126,v_pOptionsWindow->getPosition().nHeight-68,GAMETEXT_DEFAULTS,115,57);
    pDefaultOptionsButton->setID("DEFAULTS_BUTTON");
    pDefaultOptionsButton->setFont(i_drawLib->getFontSmall());
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
    pShowMiniMap->setFont(i_drawLib->getFontSmall());
    pShowMiniMap->setGroup(50023);
    pShowMiniMap->setContextHelp(CONTEXTHELP_MINI_MAP);

    UIButton *pShowEngineCounter = new UIButton(pGeneralOptionsTab,5,63-28-10,GAMETEXT_SHOWENGINECOUNTER,((pGeneralOptionsTab->getPosition().nWidth-40))/2,28);
    pShowEngineCounter->setType(UI_BUTTON_TYPE_CHECK);
    pShowEngineCounter->setID("SHOWENGINECOUNTER");
    pShowEngineCounter->enableWindow(true);
    pShowEngineCounter->setFont(i_drawLib->getFontSmall());
    pShowEngineCounter->setGroup(50023);
    pShowEngineCounter->setContextHelp(CONTEXTHELP_ENGINE_COUNTER);

    UIButton *pInitZoom = new UIButton(pGeneralOptionsTab,5+((pGeneralOptionsTab->getPosition().nWidth-40))/2,33-28-10,GAMETEXT_INITZOOM,((pGeneralOptionsTab->getPosition().nWidth-40))/2,28);
    pInitZoom->setType(UI_BUTTON_TYPE_CHECK);
    pInitZoom->setID("INITZOOM");
    pInitZoom->enableWindow(true);
    pInitZoom->setFont(i_drawLib->getFontSmall());
    pInitZoom->setGroup(50023);
    pInitZoom->setContextHelp(CONTEXTHELP_INITZOOM);

    UIButton *pDeathAnim = new UIButton(pGeneralOptionsTab,5+((pGeneralOptionsTab->getPosition().nWidth-40))/2,63-28-10,GAMETEXT_DEATHANIM,((pGeneralOptionsTab->getPosition().nWidth-40))/2,28);
    pDeathAnim->setType(UI_BUTTON_TYPE_CHECK);
    pDeathAnim->setID("DEATHANIM");
    pDeathAnim->enableWindow(true);
    pDeathAnim->setFont(i_drawLib->getFontSmall());
    pDeathAnim->setGroup(50023);
    pDeathAnim->setContextHelp(CONTEXTHELP_DEATHANIM);

    UIButton *pContextHelp = new UIButton(pGeneralOptionsTab,5,93-28-10,GAMETEXT_ENABLECONTEXTHELP,(pGeneralOptionsTab->getPosition().nWidth-40),28);
    pContextHelp->setType(UI_BUTTON_TYPE_CHECK);
    pContextHelp->setID("ENABLECONTEXTHELP");
    pContextHelp->enableWindow(true);
    pContextHelp->setFont(i_drawLib->getFontSmall());
    pContextHelp->setGroup(50023);
    pContextHelp->setContextHelp(CONTEXTHELP_SHOWCONTEXTHELP);
 
    UIButton *pAutosaveReplays = new UIButton(pGeneralOptionsTab,5,123-28-10,GAMETEXT_AUTOSAVEREPLAYS,(pGeneralOptionsTab->getPosition().nWidth-40),28);
    pAutosaveReplays->setType(UI_BUTTON_TYPE_CHECK);
    pAutosaveReplays->setID("AUTOSAVEREPLAYS");
    pAutosaveReplays->enableWindow(true);
    pAutosaveReplays->setFont(i_drawLib->getFontSmall());
    pAutosaveReplays->setGroup(50023);
    pAutosaveReplays->setContextHelp(CONTEXTHELP_AUTOSAVEREPLAYS);
   
    UIList *pThemeList = new UIList(pGeneralOptionsTab,5,120,"",
				    pGeneralOptionsTab->getPosition().nWidth-10,
				    pGeneralOptionsTab->getPosition().nHeight-125-90);
    pThemeList->setID("THEMES_LIST");
    pThemeList->setFont(i_drawLib->getFontSmall());
    pThemeList->addColumn(GAMETEXT_THEMES, (pThemeList->getPosition().nWidth*3) / 5);
    pThemeList->addColumn("", (pThemeList->getPosition().nWidth*2) / 5);
    pThemeList->setContextHelp(CONTEXTHELP_THEMES);

    UIButton *pUpdateThemesButton = new UIButton(pGeneralOptionsTab,
						 pGeneralOptionsTab->getPosition().nWidth -200 -200,
						 pGeneralOptionsTab->getPosition().nHeight - 95,
						 GAMETEXT_UPDATETHEMESLIST,
						 207,
						 57);
    pUpdateThemesButton->setType(UI_BUTTON_TYPE_LARGE);
    pUpdateThemesButton->setID("UPDATE_THEMES_LIST");
    pUpdateThemesButton->enableWindow(true);
    pUpdateThemesButton->setFont(i_drawLib->getFontSmall());
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
    pGetSelectedTheme->setFont(i_drawLib->getFontSmall());
    pGetSelectedTheme->setContextHelp(CONTEXTHELP_GETSELECTEDTHEME);

    UIWindow *pVideoOptionsTab = new UIWindow(pOptionsTabs,20,40,GAMETEXT_VIDEO,pOptionsTabs->getPosition().nWidth-40,pOptionsTabs->getPosition().nHeight);
    pVideoOptionsTab->enableWindow(true);
    pVideoOptionsTab->showWindow(false);
    pVideoOptionsTab->setID("VIDEO_TAB");
    
    UIButton *p16BitsPerPixel = new UIButton(pVideoOptionsTab,5,5,GAMETEXT_16BPP,(pVideoOptionsTab->getPosition().nWidth-40)/2,28);
    p16BitsPerPixel->setType(UI_BUTTON_TYPE_RADIO);
    p16BitsPerPixel->setID("16BPP");
    p16BitsPerPixel->enableWindow(true);
    p16BitsPerPixel->setFont(i_drawLib->getFontSmall());
    p16BitsPerPixel->setGroup(20023);
    p16BitsPerPixel->setContextHelp(CONTEXTHELP_HIGHCOLOR);

    UIButton *p32BitsPerPixel = new UIButton(pVideoOptionsTab,5 + ((pVideoOptionsTab->getPosition().nWidth-40)/2)*1,5,GAMETEXT_32BPP,(pVideoOptionsTab->getPosition().nWidth-40)/2,28);
    p32BitsPerPixel->setType(UI_BUTTON_TYPE_RADIO);
    p32BitsPerPixel->setID("32BPP");
    p32BitsPerPixel->enableWindow(true);
    p32BitsPerPixel->setFont(i_drawLib->getFontSmall());
    p32BitsPerPixel->setGroup(20023);
    p32BitsPerPixel->setContextHelp(CONTEXTHELP_TRUECOLOR);
    
    UIList *pDispResList = new UIList(pVideoOptionsTab,5,43,"",
				      pVideoOptionsTab->getPosition().nWidth       - 10,
				      pVideoOptionsTab->getPosition().nHeight - 43 - 10 - 140);
    pDispResList->setID("RES_LIST");
    pDispResList->setFont(i_drawLib->getFontSmall());
    pDispResList->addColumn(GAMETEXT_SCREENRES,pDispResList->getPosition().nWidth,CONTEXTHELP_SCREENRES);

//    std::vector<std::string>* modes = getDisplayModes(i_Config->getBool("DisplayWindowed"));
//    
//    for(int i=0; i < modes->size(); i++) {
//      pDispResList->addEntry((*modes)[i].c_str());
//    }
//    
//    delete modes;

    pDispResList->setContextHelp(CONTEXTHELP_RESOLUTION);

    UIButton *pRunWindowed = new UIButton(pVideoOptionsTab,5, pVideoOptionsTab->getPosition().nHeight - 43 - 10 - 90,
					  GAMETEXT_RUNWINDOWED,
					  (pVideoOptionsTab->getPosition().nWidth-40)/1,
					  28);
    pRunWindowed->setType(UI_BUTTON_TYPE_CHECK);
    pRunWindowed->setID("RUN_WINDOWED");
    pRunWindowed->enableWindow(true);
    pRunWindowed->setFont(i_drawLib->getFontSmall());
    pRunWindowed->setContextHelp(CONTEXTHELP_RUN_IN_WINDOW);
    
    pSomeText = new UIStatic(pVideoOptionsTab,5,pVideoOptionsTab->getPosition().nHeight - 43 - 10 - 60,
			     std::string(GAMETEXT_MENUGFX) +":",120,28);
    pSomeText->setFont(i_drawLib->getFontSmall());    
    pSomeText->enableWindow(true);
    pSomeText->showWindow(true);

    UIButton *pMenuLow = new UIButton(pVideoOptionsTab,120,pVideoOptionsTab->getPosition().nHeight - 43 - 10 - 60,
				      GAMETEXT_LOW,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pMenuLow->setType(UI_BUTTON_TYPE_RADIO);
    pMenuLow->setID("MENULOW");
    pMenuLow->enableWindow(true);
    pMenuLow->setFont(i_drawLib->getFontSmall());
    pMenuLow->setGroup(20024);
    pMenuLow->setContextHelp(CONTEXTHELP_LOW_MENU);

    UIButton *pMenuMed = new UIButton(pVideoOptionsTab,120+((pVideoOptionsTab->getPosition().nWidth-120)/3)*1,
				      pVideoOptionsTab->getPosition().nHeight - 43 - 10 - 60,
				      GAMETEXT_MEDIUM,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pMenuMed->setType(UI_BUTTON_TYPE_RADIO);
    pMenuMed->setID("MENUMEDIUM");
    pMenuMed->enableWindow(true);
    pMenuMed->setFont(i_drawLib->getFontSmall());
    pMenuMed->setGroup(20024);
    pMenuMed->setContextHelp(CONTEXTHELP_MEDIUM_MENU);

    UIButton *pMenuHigh = new UIButton(pVideoOptionsTab,120+((pVideoOptionsTab->getPosition().nWidth-120)/3)*2,
				       pVideoOptionsTab->getPosition().nHeight - 43 - 10 - 60,
				       GAMETEXT_HIGH,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pMenuHigh->setType(UI_BUTTON_TYPE_RADIO);
    pMenuHigh->setID("MENUHIGH");
    pMenuHigh->enableWindow(true);
    pMenuHigh->setFont(i_drawLib->getFontSmall());
    pMenuHigh->setGroup(20024);
    pMenuHigh->setContextHelp(CONTEXTHELP_HIGH_MENU);

    pSomeText = new UIStatic(pVideoOptionsTab,5,
			     pVideoOptionsTab->getPosition().nHeight - 43 - 10 - 30,
			     std::string(GAMETEXT_GAMEGFX) + ":",120,28);
    pSomeText->setFont(i_drawLib->getFontSmall());    
    pSomeText->enableWindow(true);
    pSomeText->showWindow(true);

    UIButton *pGameLow = new UIButton(pVideoOptionsTab,120,
				      pVideoOptionsTab->getPosition().nHeight - 43 - 10 - 30,
				      GAMETEXT_LOW,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pGameLow->setType(UI_BUTTON_TYPE_RADIO);
    pGameLow->setID("GAMELOW");
    pGameLow->enableWindow(true);
    pGameLow->setFont(i_drawLib->getFontSmall());
    pGameLow->setGroup(20025);
    pGameLow->setContextHelp(CONTEXTHELP_LOW_GAME);

    UIButton *pGameMed = new UIButton(pVideoOptionsTab,120+((pVideoOptionsTab->getPosition().nWidth-120)/3)*1,
				      pVideoOptionsTab->getPosition().nHeight - 43 - 10 - 30,
				      GAMETEXT_MEDIUM,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pGameMed->setType(UI_BUTTON_TYPE_RADIO);
    pGameMed->setID("GAMEMEDIUM");
    pGameMed->enableWindow(true);
    pGameMed->setFont(i_drawLib->getFontSmall());
    pGameMed->setGroup(20025);
    pGameMed->setContextHelp(CONTEXTHELP_MEDIUM_GAME);

    UIButton *pGameHigh = new UIButton(pVideoOptionsTab,120+((pVideoOptionsTab->getPosition().nWidth-120)/3)*2,
				       pVideoOptionsTab->getPosition().nHeight - 43 - 10 - 30,
				       GAMETEXT_HIGH,(pVideoOptionsTab->getPosition().nWidth-120)/3,28);
    pGameHigh->setType(UI_BUTTON_TYPE_RADIO);
    pGameHigh->setID("GAMEHIGH");
    pGameHigh->enableWindow(true);
    pGameHigh->setFont(i_drawLib->getFontSmall());
    pGameHigh->setGroup(20025);
    pGameHigh->setContextHelp(CONTEXTHELP_HIGH_GAME);

    /* AUDIO TAB */
    
    UIWindow *pAudioOptionsTab = new UIWindow(pOptionsTabs,20,40,GAMETEXT_AUDIO,pOptionsTabs->getPosition().nWidth-40,pOptionsTabs->getPosition().nHeight);                  
    pAudioOptionsTab->enableWindow(true);
    pAudioOptionsTab->showWindow(false);
    pAudioOptionsTab->setID("AUDIO_TAB");

    UIButton *pEnableAudioButton = new UIButton(pAudioOptionsTab,5,5,GAMETEXT_ENABLEAUDIO,pAudioOptionsTab->getPosition().nWidth-10,28);
    pEnableAudioButton->setType(UI_BUTTON_TYPE_CHECK);
    pEnableAudioButton->setID("ENABLE_AUDIO");
    pEnableAudioButton->enableWindow(true);
    pEnableAudioButton->setFont(i_drawLib->getFontSmall());
    pEnableAudioButton->setContextHelp(CONTEXTHELP_SOUND_ON);
    
    UIButton *pSampleRate11Button = new UIButton(pAudioOptionsTab,25,33,GAMETEXT_11KHZ,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pSampleRate11Button->setType(UI_BUTTON_TYPE_RADIO);
    pSampleRate11Button->setID("RATE11KHZ");
    pSampleRate11Button->enableWindow(true);
    pSampleRate11Button->setFont(i_drawLib->getFontSmall());
    pSampleRate11Button->setGroup(10023);
    pSampleRate11Button->setContextHelp(CONTEXTHELP_11HZ);
    
    UIButton *pSampleRate22Button = new UIButton(pAudioOptionsTab,25 + ((pAudioOptionsTab->getPosition().nWidth-40)/3)*1,33,GAMETEXT_22KHZ,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pSampleRate22Button->setType(UI_BUTTON_TYPE_RADIO);
    pSampleRate22Button->setID("RATE22KHZ");
    pSampleRate22Button->enableWindow(true);
    pSampleRate22Button->setFont(i_drawLib->getFontSmall());
    pSampleRate22Button->setGroup(10023);
    pSampleRate22Button->setContextHelp(CONTEXTHELP_22HZ);
    
    UIButton *pSampleRate44Button = new UIButton(pAudioOptionsTab,25 + ((pAudioOptionsTab->getPosition().nWidth-40)/3)*2,33,GAMETEXT_44KHZ,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pSampleRate44Button->setType(UI_BUTTON_TYPE_RADIO);
    pSampleRate44Button->setID("RATE44KHZ");
    pSampleRate44Button->enableWindow(true);
    pSampleRate44Button->setFont(i_drawLib->getFontSmall());
    pSampleRate44Button->setGroup(10023);
    pSampleRate44Button->setContextHelp(CONTEXTHELP_44HZ);

    UIButton *pSample8Button = new UIButton(pAudioOptionsTab,25,61,GAMETEXT_8BIT,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pSample8Button->setType(UI_BUTTON_TYPE_RADIO);
    pSample8Button->setID("8BIT");
    pSample8Button->enableWindow(true);
    pSample8Button->setFont(i_drawLib->getFontSmall());
    pSample8Button->setGroup(10024);
    pSample8Button->setContextHelp(CONTEXTHELP_8BIT);

    UIButton *pSample16Button = new UIButton(pAudioOptionsTab,25 + ((pAudioOptionsTab->getPosition().nWidth-40)/3)*1,61,GAMETEXT_16BIT,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);    
    pSample16Button->setType(UI_BUTTON_TYPE_RADIO);
    pSample16Button->setID("16BIT");
    pSample16Button->enableWindow(true);
    pSample16Button->setFont(i_drawLib->getFontSmall());
    pSample16Button->setGroup(10024);
    pSample16Button->setContextHelp(CONTEXTHELP_16BIT);

    UIButton *pMonoButton = new UIButton(pAudioOptionsTab,25,89,GAMETEXT_MONO,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pMonoButton->setType(UI_BUTTON_TYPE_RADIO);
    pMonoButton->setID("MONO");
    pMonoButton->enableWindow(true);
    pMonoButton->setFont(i_drawLib->getFontSmall());
    pMonoButton->setGroup(10025);
    pMonoButton->setContextHelp(CONTEXTHELP_MONO);
    
    UIButton *pStereoButton = new UIButton(pAudioOptionsTab,25 + ((pAudioOptionsTab->getPosition().nWidth-40)/3)*1,89,GAMETEXT_STEREO,(pAudioOptionsTab->getPosition().nWidth-40)/3,28);
    pStereoButton->setType(UI_BUTTON_TYPE_RADIO);
    pStereoButton->setID("STEREO");
    pStereoButton->enableWindow(true);
    pStereoButton->setFont(i_drawLib->getFontSmall());
    pStereoButton->setGroup(10025);
    pStereoButton->setContextHelp(CONTEXTHELP_STEREO);

    UIButton *pEnableEngineSoundButton = new UIButton(pAudioOptionsTab,5,117,GAMETEXT_ENABLEENGINESOUND,pAudioOptionsTab->getPosition().nWidth-10,28);
    pEnableEngineSoundButton->setType(UI_BUTTON_TYPE_CHECK);
    pEnableEngineSoundButton->setID("ENABLE_ENGINE_SOUND");
    pEnableEngineSoundButton->enableWindow(true);
    pEnableEngineSoundButton->setFont(i_drawLib->getFontSmall());
    pEnableEngineSoundButton->setContextHelp(CONTEXTHELP_ENGINE_SOUND);
    
    UIButton *pEnableMusicButton = new UIButton(pAudioOptionsTab,5,145,GAMETEXT_ENABLEMUSIC,pAudioOptionsTab->getPosition().nWidth-10,28);
    pEnableMusicButton->setType(UI_BUTTON_TYPE_CHECK);
    pEnableMusicButton->setID("ENABLE_MUSIC");
    pEnableMusicButton->enableWindow(true);
    pEnableMusicButton->setFont(i_drawLib->getFontSmall());
    pEnableMusicButton->setContextHelp(CONTEXTHELP_MUSIC);
    
    UIWindow *pControlsOptionsTab = new UIWindow(pOptionsTabs,20,40,GAMETEXT_CONTROLS,pOptionsTabs->getPosition().nWidth-40,pOptionsTabs->getPosition().nHeight);                  
    pControlsOptionsTab->enableWindow(true);
    pControlsOptionsTab->showWindow(false);
    pControlsOptionsTab->setID("CONTROLS_TAB");

    UIButton *pKeyboardControls = new UIButton(pControlsOptionsTab,5,5,GAMETEXT_KEYBOARD,(pControlsOptionsTab->getPosition().nWidth-40)/2,28);
    pKeyboardControls->setType(UI_BUTTON_TYPE_RADIO);
    pKeyboardControls->setID("KEYBOARD");
    pKeyboardControls->enableWindow(true);
    pKeyboardControls->setFont(i_drawLib->getFontSmall());
    pKeyboardControls->setGroup(200243);
    //pKeyboardControls->setContextHelp("

    UIButton *pJoystickControls = new UIButton(pControlsOptionsTab,5 + ((pControlsOptionsTab->getPosition().nWidth-40)/2)*1,5,GAMETEXT_JOYSTICK,(pVideoOptionsTab->getPosition().nWidth-40)/2,28);
    pJoystickControls->setType(UI_BUTTON_TYPE_RADIO);
    pJoystickControls->setID("JOYSTICK");
    pJoystickControls->enableWindow(true);
    pJoystickControls->setFont(i_drawLib->getFontSmall());
    pJoystickControls->setGroup(200243);    

    UIList *pKeyCList = new UIList(pControlsOptionsTab,5,43,"",pControlsOptionsTab->getPosition().nWidth-10, 118);
    pKeyCList->setID("KEY_ACTION_LIST");
    pKeyCList->setFont(i_drawLib->getFontSmall());
    pKeyCList->addColumn(GAMETEXT_ACTION,pKeyCList->getPosition().nWidth/2);
    pKeyCList->addColumn(GAMETEXT_KEY,pKeyCList->getPosition().nWidth/2);
    pKeyCList->setContextHelp(CONTEXTHELP_SELECT_ACTION);

    UIButton *pConfigureJoystick = new UIButton(pControlsOptionsTab,0,180,GAMETEXT_CONFIGUREJOYSTICK,207,57);
    pConfigureJoystick->setType(UI_BUTTON_TYPE_LARGE);
    pConfigureJoystick->setID("CONFIGURE_JOYSTICK");
    pConfigureJoystick->enableWindow(true);
    pConfigureJoystick->setFont(i_drawLib->getFontSmall());

#if defined(HIDE_JOYSTICK_SUPPORT)
  pKeyboardControls->showWindow(false);
  pJoystickControls->showWindow(false);
  pConfigureJoystick->showWindow(false);
  
  pKeyCList->setPosition(5,5,
			 pControlsOptionsTab->getPosition().nWidth  -10,
			 pControlsOptionsTab->getPosition().nHeight -43 -10 -10);
#endif

    UIWindow *pWWWOptionsTab = new UIWindow(pOptionsTabs,0,26,GAMETEXT_WWWTAB,pOptionsTabs->getPosition().nWidth,pOptionsTabs->getPosition().nHeight);
    pWWWOptionsTab->enableWindow(true);
    pWWWOptionsTab->showWindow(false);
    pWWWOptionsTab->setID("WWW_TAB");

    UITabView *pWWWOptionsTabs  = new UITabView(pWWWOptionsTab,0,0,"",pWWWOptionsTab->getPosition().nWidth,pWWWOptionsTab->getPosition().nHeight-76);
    pWWWOptionsTabs->setID("WWWOPTIONS_TABS");
    pWWWOptionsTabs->setFont(i_drawLib->getFontSmall());
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
    pEnableWebHighscores->setFont(i_drawLib->getFontSmall());
    pEnableWebHighscores->setGroup(50123);
    pEnableWebHighscores->setContextHelp(CONTEXTHELP_DOWNLOAD_BEST_TIMES);

    UIButton *pEnableCheckNewLevelsAtStartup = new UIButton(pWWWMainOptionsTab,5,43,GAMETEXT_ENABLECHECKNEWLEVELSATSTARTUP,(pGeneralOptionsTab->getPosition().nWidth-40),28);
    pEnableCheckNewLevelsAtStartup->setType(UI_BUTTON_TYPE_CHECK);
    pEnableCheckNewLevelsAtStartup->setID("ENABLECHECKNEWLEVELSATSTARTUP");
    pEnableCheckNewLevelsAtStartup->enableWindow(true);
    pEnableCheckNewLevelsAtStartup->setFont(i_drawLib->getFontSmall());
    pEnableCheckNewLevelsAtStartup->setGroup(50123);
    pEnableCheckNewLevelsAtStartup->setContextHelp(CONTEXTHELP_ENABLE_CHECK_NEW_LEVELS_AT_STARTUP);

    UIButton *pEnableCheckHighscoresAtStartup = new UIButton(pWWWMainOptionsTab,5,81,GAMETEXT_ENABLECHECKHIGHSCORESATSTARTUP,(pGeneralOptionsTab->getPosition().nWidth-40),28);
    pEnableCheckHighscoresAtStartup->setType(UI_BUTTON_TYPE_CHECK);
    pEnableCheckHighscoresAtStartup->setID("ENABLECHECKHIGHSCORESATSTARTUP");
    pEnableCheckHighscoresAtStartup->enableWindow(true);
    pEnableCheckHighscoresAtStartup->setFont(i_drawLib->getFontSmall());
    pEnableCheckHighscoresAtStartup->setGroup(50123);
    pEnableCheckHighscoresAtStartup->setContextHelp(CONTEXTHELP_ENABLE_CHECK_HIGHSCORES_AT_STARTUP);

    UIButton *pInGameWorldRecord = new UIButton(pWWWMainOptionsTab,5,119,GAMETEXT_ENABLEINGAMEWORLDRECORD,(pGeneralOptionsTab->getPosition().nWidth-40),28);
    pInGameWorldRecord->setType(UI_BUTTON_TYPE_CHECK);
    pInGameWorldRecord->setID("INGAMEWORLDRECORD");
    pInGameWorldRecord->enableWindow(true);
    pInGameWorldRecord->setFont(i_drawLib->getFontSmall());
    pInGameWorldRecord->setGroup(50123);
    pInGameWorldRecord->setContextHelp(CONTEXTHELP_INGAME_WORLD_RECORD);

    UIButton *pINetConf = new UIButton(pWWWOptionsTab,pWWWOptionsTab->getPosition().nWidth-225,pWWWOptionsTab->getPosition().nHeight-80,GAMETEXT_PROXYCONFIG,207,57);
    pINetConf->setType(UI_BUTTON_TYPE_LARGE);
    pINetConf->setID("PROXYCONFIG");
    pINetConf->setFont(i_drawLib->getFontSmall());
    pINetConf->setContextHelp(CONTEXTHELP_PROXYCONFIG);

    UIButton *pUpdHS = new UIButton(pWWWOptionsTab,pWWWOptionsTab->getPosition().nWidth-225-200,pWWWOptionsTab->getPosition().nHeight-80,GAMETEXT_UPDATEHIGHSCORES,207,57);
    pUpdHS->setType(UI_BUTTON_TYPE_LARGE);
    pUpdHS->setID("UPDATEHIGHSCORES");
    pUpdHS->setFont(i_drawLib->getFontSmall());
    pUpdHS->setContextHelp(CONTEXTHELP_UPDATEHIGHSCORES);

    // rooms tab
    UIList *pRoomsList = new UIList(pWWWRoomsOptionsTab,5,10,"",
				    pWWWRoomsOptionsTab->getPosition().nWidth-200,
				    pWWWRoomsOptionsTab->getPosition().nHeight-30 - 85);
    pRoomsList->setID("ROOMS_LIST");
    pRoomsList->setFont(i_drawLib->getFontSmall());
    pRoomsList->addColumn(GAMETEXT_ROOM, pThemeList->getPosition().nWidth);
    pRoomsList->setContextHelp(CONTEXTHELP_WWW_ROOMS_LIST);

    pSomeText = new UIStatic(pWWWRoomsOptionsTab,
			     pWWWRoomsOptionsTab->getPosition().nWidth-180,
			     5,
			     std::string(GAMETEXT_LOGIN) + ":",
			     130,
			     30);
    pSomeText->setHAlign(UI_ALIGN_LEFT);
    pSomeText->setFont(i_drawLib->getFontSmall()); 
    UIEdit *pRoomLoginEdit = new UIEdit(pWWWRoomsOptionsTab,
					pWWWRoomsOptionsTab->getPosition().nWidth-180,
					30,
					i_Config->getString("WebHighscoreUploadLogin"),150,25);
    pRoomLoginEdit->setFont(i_drawLib->getFontSmall());
    pRoomLoginEdit->setID("ROOM_LOGIN");
    pRoomLoginEdit->setContextHelp(CONTEXTHELP_ROOM_LOGIN);

    pSomeText = new UIStatic(pWWWRoomsOptionsTab,
			     pWWWRoomsOptionsTab->getPosition().nWidth-180,
			     65,
			     std::string(GAMETEXT_PASSWORD) + ":",
			     130,
			     30);
    pSomeText->setHAlign(UI_ALIGN_LEFT);
    pSomeText->setFont(i_drawLib->getFontSmall()); 
    UIEdit *pRoomPasswordEdit = new UIEdit(pWWWRoomsOptionsTab,
					pWWWRoomsOptionsTab->getPosition().nWidth-180,
					90,
					i_Config->getString("WebHighscoreUploadPassword"),150,25);
    pRoomPasswordEdit->hideText(true);
    pRoomPasswordEdit->setFont(i_drawLib->getFontSmall());
    pRoomPasswordEdit->setID("ROOM_PASSWORD");
    pRoomPasswordEdit->setContextHelp(CONTEXTHELP_ROOM_PASSWORD);

    UIButton *pUpdateRoomsButton = new UIButton(pWWWRoomsOptionsTab,
						pWWWRoomsOptionsTab->getPosition().nWidth/2 - 212,
						pWWWRoomsOptionsTab->getPosition().nHeight - 100,
						 GAMETEXT_UPDATEROOMSSLIST,
						 215,
						 57);
    pUpdateRoomsButton->setType(UI_BUTTON_TYPE_LARGE);
    pUpdateRoomsButton->setID("UPDATE_ROOMS_LIST");
    pUpdateRoomsButton->enableWindow(true);
    pUpdateRoomsButton->setFont(i_drawLib->getFontSmall());
    pUpdateRoomsButton->setContextHelp(CONTEXTHELP_UPDATEROOMSLIST);

	/* upload all button */
	UIButton *pUploadAllHighscoresButton = new UIButton(pWWWRoomsOptionsTab,
		pWWWRoomsOptionsTab->getPosition().nWidth/2 + 5,
		pWWWRoomsOptionsTab->getPosition().nHeight - 100,
					GAMETEXT_UPLOAD_ALL_HIGHSCORES,215,57);
	pUploadAllHighscoresButton->setFont(i_drawLib->getFontSmall());
	pUploadAllHighscoresButton->setType(UI_BUTTON_TYPE_LARGE);
	pUploadAllHighscoresButton->setID("REPLAY_UPLOADHIGHSCOREALL_BUTTON");
	pUploadAllHighscoresButton->enableWindow(true);
	pUploadAllHighscoresButton->setContextHelp(CONTEXTHELP_UPLOAD_HIGHSCORE_ALL);	
	
    UIWindow *pGhostOptionsTab = new UIWindow(pOptionsTabs,20,40,GAMETEXT_GHOSTTAB,pOptionsTabs->getPosition().nWidth-40,pOptionsTabs->getPosition().nHeight);
    pGhostOptionsTab->enableWindow(true);
    pGhostOptionsTab->showWindow(false);
    pGhostOptionsTab->setID("GHOST_TAB");

    UIButton *pEnableGhost = new UIButton(pGhostOptionsTab,5,5,GAMETEXT_ENABLEGHOST,(pGhostOptionsTab->getPosition().nWidth-40),28);
    pEnableGhost->setType(UI_BUTTON_TYPE_CHECK);
    pEnableGhost->setID("ENABLE_GHOST");
    pEnableGhost->enableWindow(true);
    pEnableGhost->setFont(i_drawLib->getFontSmall());
    pEnableGhost->setContextHelp(CONTEXTHELP_GHOST_MODE);

    UIButton *pGhostStrategy_MYBEST = new UIButton(pGhostOptionsTab,5+20,35,GAMETEXT_GHOST_STRATEGY_MYBEST,
						   (pGhostOptionsTab->getPosition().nWidth-40),28);
    pGhostStrategy_MYBEST->setType(UI_BUTTON_TYPE_CHECK);
    pGhostStrategy_MYBEST->setID("GHOST_STRATEGY_MYBEST");
    pGhostStrategy_MYBEST->enableWindow(true);
    pGhostStrategy_MYBEST->setFont(i_drawLib->getFontSmall());
    pGhostStrategy_MYBEST->setContextHelp(CONTEXTHELP_GHOST_STRATEGY_MYBEST);

    UIButton *pGhostStrategy_THEBEST = new UIButton(pGhostOptionsTab,5+20,65,GAMETEXT_GHOST_STRATEGY_THEBEST,
						   (pGhostOptionsTab->getPosition().nWidth-40),28);
    pGhostStrategy_THEBEST->setType(UI_BUTTON_TYPE_CHECK);
    pGhostStrategy_THEBEST->setID("GHOST_STRATEGY_THEBEST");
    pGhostStrategy_THEBEST->enableWindow(true);
    pGhostStrategy_THEBEST->setFont(i_drawLib->getFontSmall());
    pGhostStrategy_THEBEST->setContextHelp(CONTEXTHELP_GHOST_STRATEGY_THEBEST);

    UIButton *pGhostStrategy_BESTOFROOM = new UIButton(pGhostOptionsTab,5+20,95,GAMETEXT_GHOST_STRATEGY_BESTOFROOM,
						       (pGhostOptionsTab->getPosition().nWidth-40),28);
    pGhostStrategy_BESTOFROOM->setType(UI_BUTTON_TYPE_CHECK);
    pGhostStrategy_BESTOFROOM->setID("GHOST_STRATEGY_BESTOFROOM");
    pGhostStrategy_BESTOFROOM->enableWindow(true);
    pGhostStrategy_BESTOFROOM->setFont(i_drawLib->getFontSmall());
    pGhostStrategy_BESTOFROOM->setContextHelp(CONTEXTHELP_GHOST_STRATEGY_BESTOFROOM);

    UIButton *pDisplayGhostTimeDiff = new UIButton(pGhostOptionsTab,5,125,GAMETEXT_DISPLAYGHOSTTIMEDIFF,(pGhostOptionsTab->getPosition().nWidth-40),28);
    pDisplayGhostTimeDiff->setType(UI_BUTTON_TYPE_CHECK);
    pDisplayGhostTimeDiff->setID("DISPLAY_GHOST_TIMEDIFF");
    pDisplayGhostTimeDiff->enableWindow(true);
    pDisplayGhostTimeDiff->setFont(i_drawLib->getFontSmall());
    pDisplayGhostTimeDiff->setContextHelp(CONTEXTHELP_DISPLAY_GHOST_TIMEDIFF);

    UIButton *pDisplayGhostInfo = new UIButton(pGhostOptionsTab,5,185,GAMETEXT_DISPLAYGHOSTINFO,(pGhostOptionsTab->getPosition().nWidth-40),28);
    pDisplayGhostInfo->setType(UI_BUTTON_TYPE_CHECK);
    pDisplayGhostInfo->setID("DISPLAY_GHOST_INFO");
    pDisplayGhostInfo->enableWindow(true);
    pDisplayGhostInfo->setFont(i_drawLib->getFontSmall());
    pDisplayGhostInfo->setContextHelp(CONTEXTHELP_DISPLAY_GHOST_INFO);

    UIButton *pHideGhosts = new UIButton(pGhostOptionsTab,5,155,GAMETEXT_HIDEGHOSTS,(pGhostOptionsTab->getPosition().nWidth-40),28);
    pHideGhosts->setType(UI_BUTTON_TYPE_CHECK);
    pHideGhosts->setID("HIDEGHOSTS");
    pHideGhosts->enableWindow(true);
    pHideGhosts->setFont(i_drawLib->getFontSmall());
    pHideGhosts->setContextHelp(CONTEXTHELP_HIDEGHOSTS);

    UIButton *pMotionBlurGhost = new UIButton(pGhostOptionsTab,5,215,GAMETEXT_MOTIONBLURGHOST,(pGhostOptionsTab->getPosition().nWidth-40),28);
    pMotionBlurGhost->setType(UI_BUTTON_TYPE_CHECK);
    pMotionBlurGhost->setID("MOTION_BLUR_GHOST");
    pMotionBlurGhost->enableWindow(true);
    pMotionBlurGhost->setFont(i_drawLib->getFontSmall());
    pMotionBlurGhost->setContextHelp(CONTEXTHELP_MOTIONBLURGHOST);

    return v_pOptionsWindow;
  }

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
    if(m_xmsession->profile() != "") {
      updatePlayerTag();
    }
    
    /* new levels ? */
    m_pNewLevelsAvailable = new UIButtonDrawn(m_pMainMenu,
					      "NewLevelsAvailablePlain",
					      "NewLevelsAvailablePlain",
					      "NewLevelsAvailablePlain",
					      5, -65,
					      GAMETEXT_NEWLEVELS_AVAIBLE, 200, 200);
    m_pNewLevelsAvailable->setFont(drawLib->getFontSmall());      
    m_pNewLevelsAvailable->setID("NEWLEVELAVAILBLE");
    
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
    
    /* OPTIONS */
    m_pOptionsWindow = makeOptionsWindow(drawLib, m_pMainMenu, &m_Config);
    _UpdateThemesLists();
    _UpdateRoomsLists();
    /* ***** */

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
    
    /* Update options */
    _ImportOptions();
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

    p = pList->addEntry(GAMETEXT_DRIVE + std::string(" 2")); p->Text.push_back(m_Config.getString("KeyDrive2"));
    p = pList->addEntry(GAMETEXT_BRAKE + std::string(" 2")); p->Text.push_back(m_Config.getString("KeyBrake2"));
    p = pList->addEntry(GAMETEXT_FLIPLEFT + std::string(" 2")); p->Text.push_back(m_Config.getString("KeyFlipLeft2"));
    p = pList->addEntry(GAMETEXT_FLIPRIGHT + std::string(" 2")); p->Text.push_back(m_Config.getString("KeyFlipRight2"));
    p = pList->addEntry(GAMETEXT_CHANGEDIR + std::string(" 2")); p->Text.push_back(m_Config.getString("KeyChangeDir2"));
   
    p = pList->addEntry(GAMETEXT_DRIVE + std::string(" 3")); p->Text.push_back(m_Config.getString("KeyDrive3"));
    p = pList->addEntry(GAMETEXT_BRAKE + std::string(" 3")); p->Text.push_back(m_Config.getString("KeyBrake3"));
    p = pList->addEntry(GAMETEXT_FLIPLEFT + std::string(" 3")); p->Text.push_back(m_Config.getString("KeyFlipLeft3"));
    p = pList->addEntry(GAMETEXT_FLIPRIGHT + std::string(" 3")); p->Text.push_back(m_Config.getString("KeyFlipRight3"));
    p = pList->addEntry(GAMETEXT_CHANGEDIR + std::string(" 3")); p->Text.push_back(m_Config.getString("KeyChangeDir3"));

    p = pList->addEntry(GAMETEXT_DRIVE + std::string(" 4")); p->Text.push_back(m_Config.getString("KeyDrive4"));
    p = pList->addEntry(GAMETEXT_BRAKE + std::string(" 4")); p->Text.push_back(m_Config.getString("KeyBrake4"));
    p = pList->addEntry(GAMETEXT_FLIPLEFT + std::string(" 4")); p->Text.push_back(m_Config.getString("KeyFlipLeft4"));
    p = pList->addEntry(GAMETEXT_FLIPRIGHT + std::string(" 4")); p->Text.push_back(m_Config.getString("KeyFlipRight4"));
    p = pList->addEntry(GAMETEXT_CHANGEDIR + std::string(" 4")); p->Text.push_back(m_Config.getString("KeyChangeDir4"));
 
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
      p_packName = m_levelsManager.LevelsPacks()[i]->Name();

      /* the unpackaged pack exists only in debug mode */
      if(p_packName != "" || m_xmsession->debug()) {
	if(p_packName == "") {
	  p_packName = GAMETEXT_UNPACKED_LEVELS_PACK;
	}
	
	pTree->addPack(m_levelsManager.LevelsPacks()[i],
		       m_levelsManager.LevelsPacks()[i]->Group(),
		       m_levelsManager.LevelsPacks()[i]->getNumberOfFinishedLevels(m_db, m_xmsession->profile()),
		       m_levelsManager.LevelsPacks()[i]->getNumberOfLevels(m_db)
		       );
	
      }
    }

    /* reselect the previous pack */
    if(v_selected_packName != "") {
      pTree->setSelectedPackByName(v_selected_packName);
    }
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
      m_StateAfterPlaying = GS_MENU;
      m_stateManager->pushState(new StateReplaying(this, m_PlaySpecificReplay));
    }

    if(m_pNewLevelsAvailable->isClicked()) {
      m_pNewLevelsAvailable->setClicked(false);
      m_pLevelInfoFrame->showWindow(false);
      m_pOptionsWindow->showWindow(false);
      m_pReplaysWindow->showWindow(false);
      m_pLevelPacksWindow->showWindow(true);

      m_pLevelPackTabs->selectChildrenById("NEWLEVELS_TAB");

      checkForExtraLevels();
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

    
    if(m_pNewLevelsAvailable != NULL) {
      m_pNewLevelsAvailable->showWindow(m_bWebLevelsToDownload);
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
	  
    UIButton *pINetConf = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:PROXYCONFIG");
    UIButton *pUpdHS = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:UPDATEHIGHSCORES");
    UIButton *pWebHighscores = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLEWEBHIGHSCORES");
    UIButton *pInGameWorldRecord = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:INGAMEWORLDRECORD");

    UIButton *pCheckNewLevelsAtStartup = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLECHECKNEWLEVELSATSTARTUP");
    UIButton *pCheckHighscoresAtStartup = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLECHECKHIGHSCORESATSTARTUP");
    UIWindow *pRoomsTab = (UIWindow *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB");
    UIButton *pUpdRoomsList = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB:UPDATE_ROOMS_LIST");
	UIButton *pUploadAllHighscoresButton = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB:REPLAY_UPLOADHIGHSCOREALL_BUTTON");
			
    UIButton *pUpdThemeList = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:UPDATE_THEMES_LIST");
    UIButton *pUpdSelectedTheme = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:GET_SELECTED_THEME");
    UIList *pThemeList = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:THEMES_LIST");

    UIButton *pEnableGhost = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:ENABLE_GHOST");
    UIButton *pGhostStrategy_MYBEST = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:GHOST_STRATEGY_MYBEST");
    UIButton *pGhostStrategy_THEBEST = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:GHOST_STRATEGY_THEBEST");
    UIButton *pGhostStrategy_BESTOFROOM = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:GHOST_STRATEGY_BESTOFROOM");
    UIButton *pMotionBlurGhost = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:MOTION_BLUR_GHOST");
    UIButton *pDisplayGhostInfo = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:DISPLAY_GHOST_INFO");
    UIButton *pHideGhosts = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:HIDEGHOSTS");
    UIButton *pDisplayGhostTimeDiff = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:DISPLAY_GHOST_TIMEDIFF");

    UIButton *pMultiStopWhenOneFinishes = (UIButton *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:MULTI_TAB:ENABLEMULTISTOPWHENONEFINISHES");

	/* unfortunately because of differences betw->setClicked(false);een finishTime in webhighscores and replays table (one is rounded to 0.01s and other to 0.001s) and lack of math functions in sqlite we cannot make it with just one smart query :( */
	if(pUploadAllHighscoresButton->isClicked()) {
		pUploadAllHighscoresButton->setClicked(false);
		_UploadAllHighscores();
	}		
	
    if(pEnableGhost->getChecked()) {
      pGhostStrategy_MYBEST->enableWindow(true);
      pGhostStrategy_THEBEST->enableWindow(true);
      pGhostStrategy_BESTOFROOM->enableWindow(true);
      pMotionBlurGhost->enableWindow(true);
      pDisplayGhostInfo->enableWindow(true);
      pHideGhosts->enableWindow(true);
      pDisplayGhostTimeDiff->enableWindow(true);
    } else {
      pGhostStrategy_MYBEST->enableWindow(false);
      pGhostStrategy_THEBEST->enableWindow(false);
      pGhostStrategy_BESTOFROOM->enableWindow(false);
      pMotionBlurGhost->enableWindow(false);
      pDisplayGhostInfo->enableWindow(false);
      pHideGhosts->enableWindow(false);
      pDisplayGhostTimeDiff->enableWindow(false);
    }


    if(pWebHighscores->isClicked()) {
      pWebHighscores->setClicked(false);

      if(pWebHighscores->getChecked()) {
        pINetConf->enableWindow(true);
        pUpdHS->enableWindow(true);
	pCheckNewLevelsAtStartup->enableWindow(true);
	pCheckHighscoresAtStartup->enableWindow(true);
	pUpdThemeList->enableWindow(true);
	pUpdSelectedTheme->enableWindow(true);
	pRoomsTab->enableWindow(true);
      } else {
        pINetConf->enableWindow(false);
        pUpdHS->enableWindow(false);
	pCheckNewLevelsAtStartup->enableWindow(false);
	pCheckHighscoresAtStartup->enableWindow(false);
	pUpdThemeList->enableWindow(false);
	pUpdSelectedTheme->enableWindow(false);
	pRoomsTab->enableWindow(false);
      }

      enableWWW(pWebHighscores->getChecked());
    }
    
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

    if(pINetConf->isClicked()) {
      pINetConf->setClicked(false);
      
      m_stateManager->pushState(new StateEditWebConfig(this));

      return;
    }
    
    if(pUpdHS->isClicked()) {
      pUpdHS->setClicked(false);
      try {
	_UpdateWebHighscores(false);
	_UpgradeWebHighscores();    
	_UpdateWebLevels(false);  

	m_levelsManager.makePacks(m_db,
				  m_xmsession->profile(),
				  m_Config.getString("WebHighscoresIdRoom"),
				  m_xmsession->debug());
	_UpdateLevelsLists();
      } catch(Exception &e) {
	notifyMsg(GAMETEXT_FAILEDDLHIGHSCORES + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
      }
    }

    if(pUpdRoomsList->isClicked()) {
      pUpdRoomsList->setClicked(false);
      try {
    	_UpdateWebRooms(false);
    	_UpgradeWebRooms(true);    
      } catch(Exception &e) {
    	notifyMsg(GAMETEXT_FAILEDDLROOMSLIST + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
      }
    }
    
    if(pUpdThemeList->isClicked()) {
      pUpdThemeList->setClicked(false);
      try {
	_UpdateWebThemes(false);
      } catch(Exception &e) {
	notifyMsg(GAMETEXT_FAILEDUPDATETHEMESLIST + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
      }
    }  

    if(pUpdSelectedTheme->isClicked()) {
      pUpdSelectedTheme->setClicked(false);
      try {
	if(!pThemeList->isBranchHidden() && pThemeList->getSelected()>=0) {
	  if(!pThemeList->getEntries().empty()) {
	    UIListEntry *pEntry = pThemeList->getEntries()[pThemeList->getSelected()];
	    _UpdateWebTheme(pEntry->Text[0], true);
	  }
	}
      } catch(Exception &e) {
	notifyMsg(GAMETEXT_FAILEDGETSELECTEDTHEME + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
      }
    } 

    if(pMultiStopWhenOneFinishes->isClicked()) {
      pMultiStopWhenOneFinishes->setClicked(false);

      m_xmsession->setMultiStopWhenOneFinishes(pMultiStopWhenOneFinishes->getChecked());
      m_Config.setBool("MultiStopWhenOneFinishes", pMultiStopWhenOneFinishes->getChecked());
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
    m_Renderer->getGUI()->paint();
    drawLib->drawBox(Vector2f(0,0),Vector2f(drawLib->getDispWidth(),drawLib->getDispHeight()),0,MAKE_COLOR(0,0,0,170),0);

    m_Renderer->getGUI()->setFont(drawLib->getFontMedium());
    FontGlyph* fg = drawLib->getFontMedium()->getGlyph(Msg);
    
    int border = 75;
    int nW = fg->realWidth() + border*2, nH = fg->realHeight() + border*2;
    int nx = drawLib->getDispWidth()/2 - nW/2, ny = drawLib->getDispHeight()/2 - nH/2;
    
    if(pRect != NULL) {
      pRect->nX = nx;
      pRect->nY = ny;
      pRect->nWidth = nW;
      pRect->nHeight = nH;
    }

    m_Renderer->getGUI()->putElem(nx,ny,-1,-1,UI_ELEM_FRAME_TL,false);
    m_Renderer->getGUI()->putElem(nx+nW-8,ny,-1,-1,UI_ELEM_FRAME_TR,false);
    m_Renderer->getGUI()->putElem(nx+nW-8,ny+nH-8,-1,-1,UI_ELEM_FRAME_BR,false);
    m_Renderer->getGUI()->putElem(nx,ny+nH-8,-1,-1,UI_ELEM_FRAME_BL,false);
    m_Renderer->getGUI()->putElem(nx+8,ny,nW-16,-1,UI_ELEM_FRAME_TM,false);
    m_Renderer->getGUI()->putElem(nx+8,ny+nH-8,nW-16,-1,UI_ELEM_FRAME_BM,false);
    m_Renderer->getGUI()->putElem(nx,ny+8,-1,nH-16,UI_ELEM_FRAME_ML,false);
    m_Renderer->getGUI()->putElem(nx+nW-8,ny+8,-1,nH-16,UI_ELEM_FRAME_MR,false);
    m_Renderer->getGUI()->putRect(nx+8,ny+8,nW-16,nH-16,MAKE_COLOR(0,0,0,127));

    m_Renderer->getGUI()->setTextSolidColor(MAKE_COLOR(255,255,255,255));
    m_Renderer->getGUI()->putText(drawLib->getDispWidth()/2,drawLib->getDispHeight()/2,Msg, -0.5, -0.5);
    
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
      
      /* Wait for a key */
      std::string NewKey = m_InputHandler.waitForKey();
      if(NewKey == "<<QUIT>>") {
	/* Quit! */
	quit();
      }        
      else if(NewKey != "<<CANCEL>>" && NewKey != "") {
	/* Good... is the key already in use? */
	int nAlreadyUsedBy = _IsKeyInUse(NewKey);
	if(nAlreadyUsedBy > 0 && nAlreadyUsedBy != nSel) {
	  /* affect to the key already in use the current key */
	  pActionList->getEntries()[nAlreadyUsedBy]->Text[1] = pActionList->getEntries()[nSel]->Text[1];
	}
	pActionList->getEntries()[nSel]->Text[1] = NewKey;
      }      
    }
  }

  /*===========================================================================
  Update replays list
  ===========================================================================*/
  void GameApp::_CreateReplaysList(UIList *pList) {
  }
  
  /*===========================================================================
  Scan through loaded levels
  ===========================================================================*/

// to remove, it's now in statemainmenu
  void GameApp::_CreateLevelListsSql(UILevelList *pAllLevels, const std::string& i_sql) {
    char **v_result;
    unsigned int nrow;
    float v_playerHighscore, v_roomHighscore;
    
    if(m_xmsession->profile() == "") return;
    
    /* get selected item */
    std::string v_selected_levelName = "";
    if(pAllLevels->getSelected() >= 0 && pAllLevels->getSelected() < pAllLevels->getEntries().size()) {
      UIListEntry *pEntry = pAllLevels->getEntries()[pAllLevels->getSelected()];
      v_selected_levelName = pEntry->Text[0];
    }
    
    pAllLevels->clear();

    v_result = m_db->readDB(i_sql,
			    nrow);
    for(unsigned int i=0; i<nrow; i++) {
      if(m_db->getResult(v_result, 4, i, 2) == NULL) {
	v_playerHighscore = -1.0;
      } else {
	v_playerHighscore = atof(m_db->getResult(v_result, 4, i, 2));
      }
      
      if(m_db->getResult(v_result, 4, i, 3) == NULL) {
	v_roomHighscore = -1.0;
      } else {
	v_roomHighscore = atof(m_db->getResult(v_result, 4, i, 3));
      }
      
      pAllLevels->addLevel(m_db->getResult(v_result, 4, i, 0),
			   m_db->getResult(v_result, 4, i, 1),
			   v_playerHighscore,
			   v_roomHighscore
			   );
    }
    m_db->read_DB_free(v_result);    
    
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

  void GameApp::_CreateLevelLists(UILevelList *pAllLevels, std::string i_packageName) {
    LevelsPack *v_levelsPack = &(m_levelsManager.LevelsPackByName(i_packageName));
    _CreateLevelListsSql(pAllLevels, v_levelsPack->getLevelsWithHighscoresQuery(m_xmsession->profile(),
										m_xmsession->idRoom()));
  }

  void GameApp::_CreateThemesList(UIList *pList) {
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

    v_result = m_db->readDB("SELECT a.id_theme, a.checkSum, b.checkSum "
			    "FROM themes AS a LEFT OUTER JOIN webthemes AS b "
			    "ON a.id_theme=b.id_theme ORDER BY a.id_theme;",
			    nrow);
    for(unsigned int i=0; i<nrow; i++) {
      v_id_theme = m_db->getResult(v_result, 3, i, 0);
      v_ck1      = m_db->getResult(v_result, 3, i, 1);
      if(m_db->getResult(v_result, 3, i, 2) != NULL) {
	v_ck2      = m_db->getResult(v_result, 3, i, 2);
      }

      pEntry = pList->addEntry(v_id_theme.c_str(),
      			       NULL);
      if(v_ck1 == v_ck2 || v_ck2 == "") {
	pEntry->Text.push_back(GAMETEXT_THEMEHOSTED);
      } else {
      	pEntry->Text.push_back(GAMETEXT_THEMEREQUIREUPDATE);
      }
    }
    m_db->read_DB_free(v_result);

    v_result = m_db->readDB("SELECT a.id_theme FROM webthemes AS a LEFT OUTER JOIN themes AS b "
    			    "ON a.id_theme=b.id_theme WHERE b.id_theme IS NULL;",
    			    nrow);
    for(unsigned int i=0; i<nrow; i++) {
      v_id_theme = m_db->getResult(v_result, 1, i, 0);
      pEntry = pList->addEntry(v_id_theme.c_str(),
    			       NULL);
      pEntry->Text.push_back(GAMETEXT_THEMENOTHOSTED);
    }
    m_db->read_DB_free(v_result);

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

  void GameApp::_CreateRoomsList(UIList *pList) {
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

    v_result = m_db->readDB("SELECT id_room, name FROM webrooms ORDER BY id_room ASC;",
			    nrow);

    for(unsigned int i=0; i<nrow; i++) {
      v_roomId   = m_db->getResult(v_result, 2, i, 0);
      v_roomName = m_db->getResult(v_result, 2, i, 1);
      pEntry = pList->addEntry(v_roomName,
			       reinterpret_cast<void *>(new std::string(v_roomId))
			       );    
    }
    m_db->read_DB_free(v_result);

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
    m_WebHighscoresURL = m_Config.getString("WebHighscoresURL");
    //m_WebHighscoresIdRoom = m_Config.getString("WebHighscoresIdRoom");
    //std::string v_room_id = m_WebHighscoresIdRoom;
    //if(v_room_id == "") {v_room_id = DEFAULT_WEBROOM_ID;}
    //for(int i=0; i<pRoomsList->getEntries().size(); i++) {
    //  if((*(std::string*)pRoomsList->getEntries()[i]->pvUser) == v_room_id) {
    //	pRoomsList->setRealSelected(i);
    //	break;
    //  }
    //}

    UIButton *pEnableGhost = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:ENABLE_GHOST");
    UIButton *pGhostStrategy_MYBEST = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:GHOST_STRATEGY_MYBEST");
    UIButton *pGhostStrategy_THEBEST = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:GHOST_STRATEGY_THEBEST");
    UIButton *pGhostStrategy_BESTOFROOM = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:GHOST_STRATEGY_BESTOFROOM");
    UIButton *pMotionBlurGhost = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:MOTION_BLUR_GHOST");
    UIButton *pDisplayGhostInfo = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:DISPLAY_GHOST_INFO");
    UIButton *pHideGhosts = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:HIDEGHOSTS");
    UIButton *pDisplayGhostTimeDiff = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:DISPLAY_GHOST_TIMEDIFF");

    pEnableGhost->setChecked(m_Config.getBool("EnableGhost"));
    pGhostStrategy_MYBEST->setChecked(m_Config.getBool("GhostStrategy_MYBEST"));
    pGhostStrategy_THEBEST->setChecked(m_Config.getBool("GhostStrategy_THEBEST"));
    pGhostStrategy_BESTOFROOM->setChecked(m_Config.getBool("GhostStrategy_BESTOFROOM"));
    pMotionBlurGhost->setChecked(m_Config.getBool("GhostMotionBlur"));
    pDisplayGhostInfo->setChecked(m_Config.getBool("DisplayGhostInfo"));
    pHideGhosts->setChecked(m_Config.getBool("HideGhosts"));
    pDisplayGhostTimeDiff->setChecked(m_Config.getBool("ShowGhostTimeDiff"));

    pShowMiniMap->setChecked(m_Config.getBool("ShowMiniMap"));
    pShowEngineCounter->setChecked(m_Config.getBool("ShowEngineCounter"));
    pInitZoom->setChecked(m_Config.getBool("InitZoom"));
    pDeathAnim->setChecked(m_Config.getBool("DeathAnim"));
    pContextHelp->setChecked(m_Config.getBool("ContextHelp"));
    pAutosaveReplays->setChecked(m_xmsession->autosaveHighscoreReplays());

    std::string v_themeName = m_xmsession->theme();
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
    
    if(m_Config.getString("AudioChannels") == "Stereo")
      pStereoButton->setChecked(true);
    else
      pMonoButton->setChecked(true);
      
    pEnableEngineSoundButton->setChecked(m_Config.getBool("EngineSoundEnable"));      
    pEnableMusicButton->setChecked(m_Config.getBool("MenuMusic"));
      
    switch(m_Config.getInteger("DisplayBPP")) {
      case 16: p16bpp->setChecked(true); break;
      case 32: p32bpp->setChecked(true); break;
      default: p16bpp->setChecked(true); break; /* TODO: warning */      
    }
        
    pRunWindowed->setChecked(m_Config.getBool("DisplayWindowed"));
    
    if(m_xmsession->menuGraphics() == GFX_LOW) {
      pMenuLow->setChecked(true);
    } else if(m_xmsession->menuGraphics() == GFX_MEDIUM) {
      pMenuMed->setChecked(true);
    } else {
      pMenuHigh->setChecked(true);
    }

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
    m_Config.setValue("MenuGraphics",m_Config.getDefaultValue("MenuGraphics"));
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
    m_Config.setValue("ControllerMode2",m_Config.getDefaultValue("ControllerMode2"));
    m_Config.setValue("KeyDrive2",m_Config.getDefaultValue("KeyDrive2"));
    m_Config.setValue("KeyBrake2",m_Config.getDefaultValue("KeyBrake2"));
    m_Config.setValue("KeyFlipLeft2",m_Config.getDefaultValue("KeyFlipLeft2"));
    m_Config.setValue("KeyFlipRight2",m_Config.getDefaultValue("KeyFlipRight2"));
    m_Config.setValue("KeyChangeDir2",m_Config.getDefaultValue("KeyChangeDir2"));
    m_Config.setValue("ControllerMode3",m_Config.getDefaultValue("ControllerMode3"));
    m_Config.setValue("KeyDrive3",m_Config.getDefaultValue("KeyDrive3"));
    m_Config.setValue("KeyBrake3",m_Config.getDefaultValue("KeyBrake3"));
    m_Config.setValue("KeyFlipLeft3",m_Config.getDefaultValue("KeyFlipLeft3"));
    m_Config.setValue("KeyFlipRight3",m_Config.getDefaultValue("KeyFlipRight3"));
    m_Config.setValue("KeyChangeDir3",m_Config.getDefaultValue("KeyChangeDir3"));
    m_Config.setValue("ControllerMode4",m_Config.getDefaultValue("ControllerMode4"));
    m_Config.setValue("KeyDrive4",m_Config.getDefaultValue("KeyDrive4"));
    m_Config.setValue("KeyBrake4",m_Config.getDefaultValue("KeyBrake4"));
    m_Config.setValue("KeyFlipLeft4",m_Config.getDefaultValue("KeyFlipLeft4"));
    m_Config.setValue("KeyFlipRight4",m_Config.getDefaultValue("KeyFlipRight4"));
    m_Config.setValue("KeyChangeDir4",m_Config.getDefaultValue("KeyChangeDir4"));

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
      m_Config.setValue("HideGhosts",m_Config.getDefaultValue("HideGhosts"));
      m_Config.setValue("ShowGhostTimeDiff",m_Config.getDefaultValue("ShowGhostTimeDiff"));

      m_Config.setValue("ShowInGameWorldRecord",m_Config.getDefaultValue("ShowInGameWorldRecord"));

      m_Config.setValue("AutosaveHighscoreReplays", m_Config.getDefaultValue("AutosaveHighscoreReplays"));
      m_xmsession->setAutosaveHighscoreReplays(m_Config.getBool("AutosaveHighscoreReplays"));

      m_Config.setValue("CheckNewLevelsAtStartup",m_Config.getDefaultValue("CheckNewLevelsAtStartup"));
      m_Config.setValue("CheckHighscoresAtStartup",m_Config.getDefaultValue("CheckHighscoresAtStartup"));

      //m_WebHighscoresIdRoom = m_Config.getDefaultValue("WebHighscoresIdRoom");
      //m_Config.setValue("WebHighscoresIdRoom", m_WebHighscoresIdRoom);
    m_WebHighscoresURL = m_Config.getDefaultValue("WebHighscoresURL");
    m_Config.setValue("WebHighscoresURL", m_WebHighscoresURL);
    m_Config.setValue("WebHighscoreUploadLogin", m_Config.getDefaultValue("WebHighscoreUploadLogin"));
    m_Config.setValue("WebHighscoreUploadPasword", m_Config.getDefaultValue("WebHighscoreUploadPassword"));
    


    /* The following require restart */
    m_Config.setChanged(false);      

    m_xmsession->setTheme(m_Config.getDefaultValue("Theme"));

    m_Config.setValue("WebHighscores",m_Config.getDefaultValue("WebHighscores"));
    
    m_Config.setValue("EnableGhost",m_Config.getDefaultValue("EnableGhost"));
    m_Config.setValue("GhostStrategy_MYBEST", m_Config.getDefaultValue("GhostStrategy_MYBEST"));
    m_Config.setValue("GhostStrategy_THEBEST", m_Config.getDefaultValue("GhostStrategy_THEBEST"));
    m_Config.setValue("GhostStrategy_BESTOFROOM", m_Config.getDefaultValue("GhostStrategy_BESTOFROOM"));
    m_xmsession->setGhostStrategy_MYBEST(m_Config.getBool("GhostStrategy_MYBEST"));
    m_xmsession->setGhostStrategy_THEBEST(m_Config.getBool("GhostStrategy_THEBEST"));
    m_xmsession->setGhostStrategy_BESTOFROOM(m_Config.getBool("GhostStrategy_BESTOFROOM"));

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
    
    if(pMenuLow->getChecked()) m_Config.setString("MenuGraphics","Low");
    else if(pMenuMed->getChecked()) m_Config.setString("MenuGraphics","Medium");
    else if(pMenuHigh->getChecked()) m_Config.setString("MenuGraphics","High");
    
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

      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_DRIVE + std::string(" 2"))
	m_Config.setString("KeyDrive2",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_BRAKE + std::string(" 2"))
	m_Config.setString("KeyBrake2",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_FLIPLEFT + std::string(" 2"))
	m_Config.setString("KeyFlipLeft2",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_FLIPRIGHT + std::string(" 2"))
	m_Config.setString("KeyFlipRight2",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_CHANGEDIR + std::string(" 2"))
	m_Config.setString("KeyChangeDir2",pActionList->getEntries()[i]->Text[1]);

      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_DRIVE + std::string(" 3"))
	m_Config.setString("KeyDrive3",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_BRAKE + std::string(" 3"))
	m_Config.setString("KeyBrake3",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_FLIPLEFT + std::string(" 3"))
	m_Config.setString("KeyFlipLeft3",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_FLIPRIGHT + std::string(" 3"))
	m_Config.setString("KeyFlipRight3",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_CHANGEDIR + std::string(" 3"))
	m_Config.setString("KeyChangeDir3",pActionList->getEntries()[i]->Text[1]);

      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_DRIVE + std::string(" 4"))
	m_Config.setString("KeyDrive4",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_BRAKE + std::string(" 4"))
	m_Config.setString("KeyBrake4",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_FLIPLEFT + std::string(" 4"))
	m_Config.setString("KeyFlipLeft4",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_FLIPRIGHT + std::string(" 4"))
	m_Config.setString("KeyFlipRight4",pActionList->getEntries()[i]->Text[1]);
      else if(pActionList->getEntries()[i]->Text[0] == GAMETEXT_CHANGEDIR + std::string(" 4"))
	m_Config.setString("KeyChangeDir4",pActionList->getEntries()[i]->Text[1]);
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
    m_xmsession->setEnableMenuMusic(pEnableMusicButton->getChecked());

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
      
    UIButton *pWebHighscores = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLEWEBHIGHSCORES");
    UIButton *pInGameWorldRecord = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:INGAMEWORLDRECORD");
    UIButton *pCheckNewLevelsAtStartup = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLECHECKNEWLEVELSATSTARTUP");
    UIButton *pCheckHighscoresAtStartup = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_MAIN_TAB:ENABLECHECKHIGHSCORESATSTARTUP");
    UIList *pRoomsList = (UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB:ROOMS_LIST");
    UIEdit *pRoomsLogin = (UIEdit *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB:ROOM_LOGIN");
    UIEdit *pRoomsPassword = (UIEdit *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB:ROOM_PASSWORD");

    m_Config.setBool("ShowInGameWorldRecord",pInGameWorldRecord->getChecked());

    m_xmsession->setAutosaveHighscoreReplays(pAutosaveReplays->getChecked());
    m_Config.setBool("AutosaveHighscoreReplays", m_xmsession->autosaveHighscoreReplays());

    m_Config.setBool("CheckNewLevelsAtStartup",pCheckNewLevelsAtStartup->getChecked());
    m_Config.setBool("CheckHighscoresAtStartup",pCheckHighscoresAtStartup->getChecked());

    if(pRoomsList->getSelected() >= 0 &&
       pRoomsList->getSelected() < pRoomsList->getEntries().size()) {
      char **v_result;
      unsigned int nrow;

      //m_WebHighscoresIdRoom = *((std::string*)pRoomsList->getEntries()[pRoomsList->getSelected()]->pvUser);
      //m_Config.setString("WebHighscoresIdRoom", m_WebHighscoresIdRoom);

//     v_result = m_db->readDB("SELECT highscoresUrl FROM webrooms WHERE id_room=" + m_WebHighscoresIdRoom + ";",
//			      nrow);
//     if(nrow == 1) {
//	m_WebHighscoresURL = m_db->getResult(v_result, 1, 0, 0);
//     } else {
//	m_WebHighscoresURL = "";
//     }
//     m_db->read_DB_free(v_result);
//
//     m_Config.setString("WebHighscoresURL", m_WebHighscoresURL);
    }

    m_Config.setString("WebHighscoreUploadLogin", pRoomsLogin->getCaption());
    m_Config.setString("WebHighscoreUploadPassword", pRoomsPassword->getCaption());
    m_Config.setBool("WebHighscores",pWebHighscores->getChecked());

    if(pThemeList->getSelected() >= 0 && pThemeList->getSelected() < pThemeList->getEntries().size()) {
      UIListEntry *pEntry = pThemeList->getEntries()[pThemeList->getSelected()];
      m_xmsession->setTheme(pEntry->Text[0]);
      if(m_theme.Name() != pEntry->Text[0]) {
	Logger::Log("Reloading the theme...");
	reloadTheme();
      }
    }

    UIButton *pEnableGhost = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:ENABLE_GHOST");
    UIButton *pGhostStrategy_MYBEST = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:GHOST_STRATEGY_MYBEST");
    UIButton *pGhostStrategy_THEBEST = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:GHOST_STRATEGY_THEBEST");
    UIButton *pGhostStrategy_BESTOFROOM = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:GHOST_STRATEGY_BESTOFROOM");
    UIButton *pMotionBlurGhost = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:MOTION_BLUR_GHOST");
    UIButton *pDisplayGhostInfo = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:DISPLAY_GHOST_INFO");
    UIButton *pHideGhosts = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:HIDEGHOSTS");
    UIButton *pDisplayGhostTimeDiff = (UIButton *)m_pOptionsWindow->getChild("OPTIONS_TABS:GHOST_TAB:DISPLAY_GHOST_TIMEDIFF");

    m_Config.setBool("EnableGhost",pEnableGhost->getChecked());
    m_Config.setBool("GhostStrategy_MYBEST",pGhostStrategy_MYBEST->getChecked());
    m_Config.setBool("GhostStrategy_THEBEST",pGhostStrategy_THEBEST->getChecked());
    m_Config.setBool("GhostStrategy_BESTOFROOM",pGhostStrategy_BESTOFROOM->getChecked());
    m_xmsession->setGhostStrategy_MYBEST(m_Config.getBool("GhostStrategy_MYBEST"));
    m_xmsession->setGhostStrategy_THEBEST(m_Config.getBool("GhostStrategy_THEBEST"));
    m_xmsession->setGhostStrategy_BESTOFROOM(m_Config.getBool("GhostStrategy_BESTOFROOM"));
    m_Config.setBool("GhostMotionBlur",pMotionBlurGhost->getChecked());
    m_Config.setBool("DisplayGhostInfo",pDisplayGhostInfo->getChecked());
    m_Config.setBool("HideGhosts",pHideGhosts->getChecked());
    m_Config.setBool("ShowGhostTimeDiff",pDisplayGhostTimeDiff->getChecked());


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

    /* set the room name ; set to WR if it cannot be determined */
//   m_WebHighscoresRoomName = "WR";
//   char **v_result;
//   unsigned int nrow;
//   v_result = m_db->readDB("SELECT name "
//			    "FROM webrooms "
//			    "WHERE id_room=" + m_WebHighscoresIdRoom + ";",
//			    nrow);
//   if(nrow == 1) {
//     m_WebHighscoresRoomName = m_db->getResult(v_result, 1, 0, 0);
//   }
//   m_db->read_DB_free(v_result);

    updatePlayerTag();
  }
  
  void GameApp::updatePlayerTag() {
    UIStatic *pPlayerTag = reinterpret_cast<UIStatic *>(m_pMainMenu->getChild("PLAYERTAG"));
    if(pPlayerTag) {
      if(m_xmsession->profile() != "") {
	pPlayerTag->setCaption(std::string(GAMETEXT_PLAYER) + ": " + m_xmsession->profile() + "@" + m_WebHighscoresRoomName);
      }
    }
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
      _UpdateReplaysList();
      
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

  void GameApp::_UpdateLevelPackLevelList(const std::string& v_levelPack) {
    UIPackTree *pTree = (UIPackTree *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:PACK_TAB:LEVELPACK_TREE");
    LevelsPack *v_pack = &(m_levelsManager.LevelsPackByName(v_levelPack));

    pTree->updatePack(v_pack,
		      v_pack->getNumberOfFinishedLevels(m_db, m_xmsession->profile()),
		      v_pack->getNumberOfLevels(m_db)
		      );
  }

