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

#ifndef __GAME_H__
#define __GAME_H__

#include "VCommon.h"
#include "VApp.h"
#include "LevelSrc.h"
#include "MotoGame.h"
#include "VTexture.h"
#include "Renderer.h"
#include "Replay.h"
#include "UserConfig.h"
#include "GameText.h"
#include "PlayerData.h"
#include "Sound.h"
#include "Input.h"

namespace vapp {

	/*===========================================================================
	States
  ===========================================================================*/
  enum GameState {
    GS_UNASSIGNED = 0,
    GS_MENU,                  /* In the game menu */
    GS_PLAYING,               /* Playing the game */
    GS_REPLAYING,             /* Playing back a replay */
    GS_PAUSE,                 /* Paused from GS_PLAYING */
    GS_JUSTDEAD,              /* Head-banging too much */
    GS_EDIT_PROFILES,         /* In profile editor */
    GS_FINISHED               /* Finished a level */
  };

	/*===========================================================================
	Menu background graphical settings
  ===========================================================================*/
  enum MenuBackgroundGraphics {
    MENU_GFX_OFF,
    MENU_GFX_LOW,
    MENU_GFX_HIGH
  };

	/*===========================================================================
	Game application
  ===========================================================================*/
  class GameApp : public App {
    public:
      GameApp() {m_bShowMiniMap=true;
                 m_bDebugMode=false;
                 m_bListReplays=false;
                 m_bListLevels=false;
                 m_bTimeDemo=false;
                 m_bShowFrameRate=false;
                 m_pReplay=NULL;
                 m_pQuitMsgBox=NULL;
                 m_pNotifyMsgBox=NULL;
                 m_pNewProfileMsgBox=NULL;
                 m_pDeleteProfileMsgBox=NULL;}
        
      /* Virtual methods */
      virtual void drawFrame(void);
      virtual void userInit(void);
      virtual void userPreInit(void);
      virtual void userShutdown(void);
      virtual void keyDown(int nKey,int nChar);
      virtual void keyUp(int nKey);
      virtual void mouseDown(int nButton);
      virtual void mouseDoubleClick(int nButton);
      virtual void mouseUp(int nButton);
      virtual void parseUserArgs(std::vector<std::string> &UserArgs);
      virtual void helpUserArgs(void);
      virtual void selectDisplayMode(int *pnWidth,int *pnHeight,int *pnBPP,bool *pbWindowed);

      
      /* Methods */
      void setState(GameState s);
      void notifyMsg(std::string Msg);      
    
    private: 
      /* Data */
      bool m_bShowFrameRate;                    /* true: frame rate */
      bool m_bListLevels;                       /* true: list installed levels */
      bool m_bListReplays;                      /* true: list all replays */
      bool m_bTimeDemo;                         /* true: (valid for replaying) - performance benchmark */
      bool m_bDebugMode;                        /* true: show debug info */
      std::string m_PlaySpecificLevel;          /* If set, we only want to 
                                                   play this level */
      std::string m_PlaySpecificReplay;         /* If set, we only want to 
                                                   playback this replay */          
      std::string m_ForceProfile;               /* Force this player profile */    
      std::string m_GraphDebugInfoFile;
      int m_nNumLevels;
      LevelSrc m_Levels[2048];                  /* Array of levels */
      
      InputHandler m_InputHandler;              /* The glorious input handler */
      GameState m_State;                        /* Current state */      
      MotoGame m_MotoGame;                      /* Game object */
      GameRenderer m_Renderer;                  /* Renderer */
      Replay *m_pReplay;                        /* Active replay object */
      int m_nFrame;                             /* Frame # */
      PlayerProfile *m_pPlayer;                 /* The player's profile */
      
      double m_fLastFrameTime;                  /* When the last frama was initiated */
      double m_fLastPerfStateTime;
      
      /* Sound effects */
      SoundSample *m_pEndOfLevelSFX;
      SoundSample *m_pStrawberryPickupSFX;
      SoundSample *m_pWreckerTouchSFX;
      SoundSample *m_pDieSFX;
      
      /* Various popups */
      UIMsgBox *m_pQuitMsgBox;
      UIMsgBox *m_pNotifyMsgBox;
            
      /* Keep a copy of the previous bike-controller state, so we can track 
         changes for the replayer */
      BikeController m_PrevBikeC;
      
      /* Main menu background / title */
      Texture *m_pTitleBL,*m_pTitleBR,*m_pTitleTL,*m_pTitleTR;
      
      /* Main menu buttons and stuff */
      int m_nNumMainMenuButtons;
      UIButton *m_pMainMenuButtons[10];
      UIFrame *m_pOptionsWindow,*m_pHelpWindow,*m_pReplaysWindow,*m_pBestTimesWindow,*m_pPlayWindow;
      UIWindow *m_pMainMenu;
      
      /* In-game PAUSE menu fun */
      UIFrame *m_pPauseMenu;
      int m_nPauseShade;
      UIButton *m_pPauseMenuButtons[10];
      int m_nNumPauseMenuButtons;
      
      /* In-game JUSTDEAD menu fun */
      UIFrame *m_pJustDeadMenu;
      int m_nJustDeadShade;
      UIButton *m_pJustDeadMenuButtons[10];            
      int m_nNumJustDeadMenuButtons;      
      float m_fCoolDownEnd;

      /* In-game FINISH menu fun */
      UIFrame *m_pFinishMenu;
      int m_nFinishShade;
      UIButton *m_pFinishMenuButtons[10];            
      int m_nNumFinishMenuButtons;      
      UIBestTimes *m_pBestTimes;
      
      /* Profile editor fun */
      UIFrame *m_pProfileEditor;  
      UIMsgBox *m_pNewProfileMsgBox;    
      UIMsgBox *m_pDeleteProfileMsgBox;
      
      /* Config & profiles */
      UserConfig m_Config;
      PlayerData m_Profiles;
      
      /* Misc settings */
      MenuBackgroundGraphics m_MenuBackgroundGraphics;
      bool m_bShowMiniMap;
            
      /* Helpers */
      LevelSrc *_FindLevelByID(std::string ID);
      void _StoreControllerState(void);
      void _RecordReplay(void);
      void _PlaybackReplay(void);          
      void _HandleMainMenu(void);  
      void _HandlePauseMenu(void);
      void _HandleJustDeadMenu(void);
      void _HandleFinishMenu(void);
      void _HandleProfileEditor(void);
      void _CreateReplayList(UIList *pList);
      void _CreateLevelLists(UIList *pExternalLevels,UIList *pInternalLists);
      void _CreateProfileList(void);
      void _CreateDefaultConfig(void);
      void _UpdateActionKeyList(void);
      void _ChangeKeyConfig(void);
      void _ConfigureJoystick(void);
      void _MakeBestTimesWindow(UIBestTimes *pWindow,std::string PlayerName,std::string LevelID,
                                float fFinishTime,std::string TimeStamp);      
      void _DrawMenuBackground(void); 
      void _DispatchMouseHover(void);                               
      void _InitMenus(void);        
      void _ImportOptions(void);
      void _DefaultOptions(void);
      void _SaveOptions(void);
      void _UpdateSettings(void);
      void _UpdateLevelLists(void);
      void _GameScreenshot(void);

      void _UpdateLoadingScreen(float fDone,Texture *pScreen);      
      
      void _SimpleMessage(const std::string &Msg);
      
      int _IsKeyInUse(const std::string &Key);
  };

};

#endif
