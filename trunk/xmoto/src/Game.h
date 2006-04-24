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
#include "WebHighscores.h"
#include "WebHSAppInterface.h"

namespace vapp {

	/*===========================================================================
	Overall game states
  ===========================================================================*/
  enum GameState {
    GS_UNASSIGNED = 0,
    GS_MENU,                  /* In the game menu */
    GS_PLAYING,               /* Playing the game */
    GS_PAUSE,                 /* Paused from GS_PLAYING */
    GS_JUSTDEAD,              /* Head-banging too much */
    GS_EDIT_PROFILES,         /* In profile editor */
    GS_FINISHED,              /* Finished a level */
    GS_REPLAYING,             /* Replaying */
    GS_LEVEL_INFO_VIEWER,     /* In level info viewer */
    GS_LEVELPACK_VIEWER,      /* In level pack viewer */
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
	Level packs
  ===========================================================================*/
  struct LevelPack {
    std::string Name;
    std::vector<LevelSrc *> Levels;
  };

	/*===========================================================================
	Game application
  ===========================================================================*/
  #if defined(SUPPORT_WEBHIGHSCORES)
  class GameApp : public App, public WebHSAppInterface {
  #else
  class GameApp : public App {
  #endif
    public:
      virtual ~GameApp() {}
      GameApp() {m_bShowMiniMap=true;
                 m_bDebugMode=false;
                 m_bListLevels=false;
                 m_bListReplays=false;
                 m_bTimeDemo=false;
                 m_bShowFrameRate=false;
                 m_bEnableLevelCache=true;
                 m_pQuitMsgBox=NULL;
                 m_pNotifyMsgBox=NULL;
                 m_pNewProfileMsgBox=NULL;
                 m_pDeleteProfileMsgBox=NULL;
                 m_pDeleteReplayMsgBox=NULL;
                 m_pSaveReplayMsgBox=NULL;
                 m_pReplaysWindow=NULL;
                 m_pLevelPacksWindow=NULL;
                 m_pLevelPackViewer=NULL;  
                 m_pActiveLevelPack=NULL;
                 m_pGameInfoWindow=NULL;
                 m_b50FpsMode = false;
                 m_bUglyMode = false;
                 m_pReplay = NULL;
                 m_bRecordReplays = true;
                 m_bShowCursor = true;
                 m_bEnableEngineSound = true;
                 m_bCompressReplays = true;
                 m_bBenchmark = false;
                 m_bEnableWebHighscores = true;
                 m_bShowWebHighscoreInGame = false;
                 m_bEnableContextHelp = true;
                 }
                 
      #if defined(SUPPORT_WEBHIGHSCORES)                 
        /* WebHSAppInterface implementation */ 
        virtual void beginTask(WebHSTask Task);
        virtual void setTaskProgress(float fPercent);
        virtual void endTask(void);

        virtual void setBeingDownloadedLevel(const std::string &LevelName);
        virtual void readEvents(void);
        
        virtual bool doesLevelExist(const std::string &LevelID); 
      #endif
      
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
      EngineSoundSimulator m_EngineSound;
    
      /* Data */
      std::vector<LevelPack *> m_LevelPacks;    /* Level packs */
      
      bool m_bEnableContextHelp;                /* true: Show context help */
      bool m_bShowWebHighscoreInGame;           /* true: Show world highscore inside the game */
      bool m_bEnableWebHighscores;              /* true: Read world highscores from website */
      bool m_bBenchmark;                        /* true: Test game performance */
      bool m_bShowFrameRate;                    /* true: frame rate */
      bool m_bListLevels;                       /* true: list installed levels */
      bool m_bListReplays;                      /* true: list replays */
      bool m_bTimeDemo;                         /* true: (valid for replaying) - performance benchmark */
      bool m_bDebugMode;                        /* true: show debug info */
      bool m_bUglyMode;													/* true: fast 'n ugly graphics */
      bool m_bEnableEngineSound;                /* true: engine sound is enabled */
      bool m_bCompressReplays;                  /* true: compress replays with zlib */
      bool m_bEnableLevelCache;                 /* true: cache levels for faster loading */
      std::string m_PlaySpecificLevel;          /* If set, we only want to 
                                                   play this level */
      std::string m_PlaySpecificReplay;         /* If set, we only want to
                                                   play this replay */                                                   
      std::string m_ForceProfile;               /* Force this player profile */    
      std::string m_GraphDebugInfoFile;
      int m_nNumLevels;
      LevelSrc m_Levels[2048];                  /* Array of levels */
      
      InputHandler m_InputHandler;              /* The glorious input handler */
      GameState m_State;                        /* Current state */      
      GameState m_StateAfterPlaying;            /* State that should be used later */
      MotoGame m_MotoGame;                      /* Game object */      
      GameRenderer m_Renderer;                  /* Renderer */
      int m_nFrame;                             /* Frame # */
      PlayerProfile *m_pPlayer;                 /* The player's profile */
      
      double m_fLastFrameTime;                  /* When the last frama was initiated */
      double m_fLastPerfStateTime;
      float m_fLastStateSerializationTime;    
      double m_fLastPhysTime;                  /* When the last physic was computed */
      double m_fStartTime;                      
      
      bool m_b50FpsMode;
      
      Replay *m_pReplay;
      std::string m_ReplayPlayerName;
      
      /* Web-highscores */
      #if defined(SUPPORT_WEBHIGHSCORES)
        WebHighscores m_WebHighscores;
      #endif
      
      /* Sound effects */
      SoundSample *m_pEndOfLevelSFX;
      SoundSample *m_pStrawberryPickupSFX;
      SoundSample *m_pWreckerTouchSFX;
      SoundSample *m_pDieSFX;
      
      /* Various popups */
      UIMsgBox *m_pQuitMsgBox;
      UIMsgBox *m_pNotifyMsgBox;
            
      /* Main menu background / title */
      Texture *m_pTitleBL,*m_pTitleBR,*m_pTitleTL,*m_pTitleTR;
      Texture *m_pCursor;
      bool m_bShowCursor;
      
      /* Main menu buttons and stuff */
      int m_nNumMainMenuButtons;
      UIButton *m_pMainMenuButtons[10];
      UIFrame *m_pOptionsWindow,*m_pHelpWindow,*m_pPlayWindow,*m_pReplaysWindow,*m_pLevelPacksWindow;
      UIWindow *m_pMainMenu;
      UIMsgBox *m_pDeleteReplayMsgBox;
      UIFrame *m_pGameInfoWindow;
            
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
      
      /* Level pack viewer fun */
      UIFrame *m_pLevelPackViewer;  
      LevelPack *m_pActiveLevelPack;
      
      /* Level info viewer fun */
      UIFrame *m_pLevelInfoViewer;      
      std::string m_LevelInfoViewerLevel;
      
      /* Replay saving UI fun */
      UIMsgBox *m_pSaveReplayMsgBox;    
      
      /* Config & profiles */
      UserConfig m_Config;
      PlayerData m_Profiles;
      
      /* Misc settings */
      MenuBackgroundGraphics m_MenuBackgroundGraphics;
      bool m_bShowMiniMap;
      bool m_bRecordReplays;
      float m_fReplayFrameRate;
      float m_fCurrentReplayFrameRate;
            
      /* Helpers */
      void _UpdateWorldRecord(const std::string &LevelID);
      LevelSrc *_FindLevelByID(std::string ID);
      void _HandleMainMenu(void);  
      void _HandlePauseMenu(void);
      void _HandleJustDeadMenu(void);
      void _HandleFinishMenu(void);
      void _HandleProfileEditor(void);
      void _HandleLevelInfoViewer(void);
      void _HandleLevelPackViewer(void);
      void _CreateLevelLists(UIList *pExternalLevels,UIList *pInternalLevels);
      void _CreateReplaysList(UIList *pList);
      void _CreateProfileList(void);
      void _CreateDefaultConfig(void);
      void _CreateLevelPackLevelList(void);
      void _UpdateActionKeyList(void);
      void _UpdateLevelPackList(void);
      void _UpdateLevelInfoViewerBestTimes(const std::string &LevelID);     
      void _UpdateLevelInfoViewerReplays(const std::string &LevelID);     
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
      void _UpdateReplaysList(void);
      void _GameScreenshot(void);
      void _SaveReplay(const std::string &Name);
    
      void _UpdateLoadingScreen(float fDone,Texture *pScreen,const std::string &NextTask);      
      
      void _SimpleMessage(const std::string &Msg);
      
      int _IsKeyInUse(const std::string &Key);
      
      void _UpdateLevelPackManager(LevelSrc *pLevelSrc);
      LevelPack *_FindLevelPackByName(const std::string &Name);
      std::string _DetermineNextLevel(LevelSrc *pLevelSrc);
      bool _IsThereANextLevel(LevelSrc *pLevelSrc);
  };

};

#endif
