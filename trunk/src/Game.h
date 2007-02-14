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
#include "xmscene/Level.h"
#include "MotoGame.h"
#include "VTexture.h"
#include "Renderer.h"
#include "Replay.h"
#include "UserConfig.h"
#include "GameText.h"
#include "PlayerData.h"
#include "Sound.h"
#include "Input.h"
#include "WWW.h"
#include "WWWAppInterface.h"
#include "GUIXMoto.h"
#include "Stats.h"
#include "Credits.h"
#include "LevelsManager.h"

#define PRESTART_ANIMATION_TIME 2.0
#define PRESTART_ANIMATION_LEVEL_MSG_DURATION 1.0
#define PRESTART_ANIMATION_MARGIN_SIZE 5
#define PRESTART_ANIMATION_CURVE 3.0

namespace vapp {

  /*===========================================================================
  Overall game states
  ===========================================================================*/
  enum GameState {
    GS_UNASSIGNED = 0,
    GS_MENU,                  /* In the game menu */
    GS_PREPLAYING,            /* Just before the game start */
    GS_PLAYING,               /* Playing the game */
    GS_PAUSE,                 /* Paused from GS_PLAYING */
    GS_DEADJUST,              /* just dead */
    GS_DEADMENU,              /* Head-banging too much */
    GS_EDIT_PROFILES,         /* In profile editor */
    GS_FINISHED,              /* Finished a level */
    GS_REPLAYING,             /* Replaying */
    GS_LEVEL_INFO_VIEWER,     /* In level info viewer */
    GS_LEVELPACK_VIEWER,      /* In level pack viewer */
    GS_CREDITSMODE,           /* Credits/replay */
#if defined(SUPPORT_WEBACCESS)
    GS_EDIT_WEBCONFIG         /* Editing internet configuration */
#endif
  };

  /*===========================================================================
  Menu background graphical settings
  ===========================================================================*/
  enum MenuBackgroundGraphics {
    MENU_GFX_OFF,
    MENU_GFX_LOW,
    MENU_GFX_HIGH
  };
  

#if defined(ALLOW_GHOST) 
  enum GhostSearchStrategy {
    GHOST_STRATEGY_MYBEST,
    GHOST_STRATEGY_THEBEST,
    GHOST_STRATEGY_BESTOFROOM
  };
#endif

  /*===========================================================================
  Game application
  ===========================================================================*/
  #if defined(SUPPORT_WEBACCESS)
  class GameApp : public App, public WWWAppInterface {
  #else
  class GameApp : public App {
  #endif
    public:
      virtual ~GameApp() {}
      GameApp() {m_pCredits = NULL;
                 m_bShowMiniMap=true;
                 m_bDebugMode=false;
                 m_bListLevels=false;
                 m_bListReplays=false;
                 m_bTimeDemo=false;
                 m_bShowFrameRate=false;
                 m_bEnableLevelCache=true;
                 m_bEnableMenuMusic=false;
                 m_bEnableInitZoom=false;
								 m_autoZoom = false;
								 m_bAutoZoomInitialized = false;
								 m_bLockMotoGame = false;
                 m_bCleanCache=false;
                 m_bEnableDeathAnim=true;
                 m_pQuitMsgBox=NULL;
                 m_pNotifyMsgBox=NULL;
                 m_pInfoMsgBox=NULL;
#if defined(SUPPORT_WEBACCESS)
                 m_pNewLevelsAvailIcon=NULL;
                 m_pWebConfEditor=NULL;
                 m_pWebConfMsgBox=NULL;
#endif
                 m_pNewProfileMsgBox=NULL;
                 m_pDeleteProfileMsgBox=NULL;
                 m_pDeleteReplayMsgBox=NULL;
                 m_pSaveReplayMsgBox=NULL;
                 m_pReplaysWindow=NULL;
                 m_pLevelPacksWindow=NULL;
                 m_pStatsReport=NULL;
                 m_pLevelPackViewer=NULL;  
                 m_pActiveLevelPack=NULL;
                 m_pGameInfoWindow=NULL;
                 m_fFrameTime = 0;
                 m_fFPS_Rate = 0;
                 m_b50FpsMode = false;
                 m_bUglyMode = false;
                 m_bTestThemeMode = false;
                 m_pReplay = NULL;
		 m_updateAutomaticallyLevels = false;
#if defined(ALLOW_GHOST)
     m_pGhostReplay = NULL;
     m_lastGhostReplay = "";
     GhostSearchStrategies[0] = GHOST_STRATEGY_MYBEST;
     GhostSearchStrategies[1] = GHOST_STRATEGY_THEBEST;
     GhostSearchStrategies[2] = GHOST_STRATEGY_BESTOFROOM;
     m_bEnableGhost = true;
     m_bShowGhostTimeDiff = true;
     m_GhostSearchStrategy = GHOST_STRATEGY_MYBEST;
                 m_bEnableGhostInfo = false;
                 m_bGhostMotionBlur = true;
#endif
     m_bAutosaveHighscoreReplays = true;
                 m_bRecordReplays = true;
                 m_bShowCursor = true;
                 m_bEnableEngineSound = true;
                 m_bCompressReplays = true;
                 m_bBenchmark = false;
                 m_bEnableContextHelp = true;     
		 m_bDisplayInfosReplay = false;

#if defined(SUPPORT_WEBACCESS)
                 m_bShowWebHighscoreInGame = false;
                 m_bEnableWebHighscores = true;
     m_pWebHighscores = NULL;
     m_pWebLevels = NULL;
     m_pWebRooms = NULL;
     m_fDownloadTaskProgressLast = 0;
     m_bWebHighscoresUpdatedThisSession = false;
     m_bWebLevelsToDownload = false;

     m_bEnableCheckNewLevelsAtStartup  = true;
     m_bEnableCheckHighscoresAtStartup = true;
#endif
     m_fLastSqueekTime = 0.0f;

     m_Renderer.setTheme(getTheme());
     m_MotoGame.setRenderer(&m_Renderer);

     m_bPrePlayAnim = true;
     m_bShowEngineCounter = true;

     m_currentPlayingList = NULL;
      }
                 
#if defined(SUPPORT_WEBACCESS)                 
        /* WWWAppInterface implementation */ 
        virtual void setTaskProgress(float fPercent);

        virtual void setBeingDownloadedInformation(const std::string &p_information,bool p_bNew = true);
        virtual void readEvents(void);
        
        virtual bool shouldLevelBeUpdated(const std::string &LevelID);

        virtual std::string levelPathForUpdate(const std::string &p_LevelId);
        virtual std::string levelMD5Sum(const std::string &LevelID);
	virtual bool doesLevelExist(const std::string &p_LevelId);
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
      virtual std::string selectDrawLibMode();
      virtual std::string getConfigThemeName(ThemeChoicer *p_themeChoicer);

      /* Methods */
      void setState(GameState s);
      void notifyMsg(std::string Msg);      
      void setPrePlayAnim(bool pEnabled);
      void reloadTheme();

      void PlaySpecificLevel(std::string i_level);
      void PlaySpecificReplay(std::string i_replay);

      /* Data interface */
      bool isUglyMode() {return m_bUglyMode;}
      bool isTestThemeMode(void) {return m_bTestThemeMode;}
    
			void setAutoZoom(bool bValue);

    private: 
      EngineSoundSimulator m_EngineSound;
    
      /* Data */
      ReplayList m_ReplayList;                  /* Replay list */
      bool m_bEnableInitZoom;                   /* true: Perform initial level scroll/zoom */
			bool m_autoZoom;                          /* true : the key is pressed so that it zooms out to see the level */
			bool m_bAutoZoomInitialized;
      bool m_bEnableDeathAnim;                  /* true: Bike falls apart at when dead */
      bool m_bEnableMenuMusic;                  /* true: Play menu music */      
      bool m_bEnableContextHelp;                /* true: Show context help */
      bool m_bBenchmark;                        /* true: Test game performance */
      bool m_bShowFrameRate;                    /* true: frame rate */
      bool m_bListLevels;                       /* true: list installed levels */
      bool m_bListReplays;                      /* true: list replays */
      bool m_bTimeDemo;                         /* true: (valid for replaying) - performance benchmark */
      bool m_bDebugMode;                        /* true: show debug info */
      bool m_bUglyMode;                         /* true: fast 'n ugly graphics */
      bool m_bCleanCache;                       /* true: clean the level cache at startup */
      bool m_bShowEngineCounter;
      bool m_bDisplayInfosReplay;               /* true: just display infos of a replay */
      std::string m_InfosReplay;                /* name of the replay to display information */

      bool m_bTestThemeMode;
      bool m_bEnableEngineSound;                /* true: engine sound is enabled */
      bool m_bCompressReplays;                  /* true: compress replays with zlib */
      bool m_bEnableLevelCache;                 /* true: cache levels for faster loading */
      bool m_bAutosaveHighscoreReplays;
      std::string m_PlaySpecificLevel;          /* If set, we only want to 
                                                   play this level */
      std::string m_PlaySpecificReplay;         /* If set, we only want to
                                                   play this replay */                                                   
      std::string m_ForceProfile;               /* Force this player profile */    
      std::string m_GraphDebugInfoFile;
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
      double m_fLastSqueekTime;
      
      std::string m_playingMusic; /* name of the music played to not restart it if the same must be played on an action */

      bool m_b50FpsMode;
      
      Replay *m_pReplay;
#if defined(ALLOW_GHOST) 
      Replay *m_pGhostReplay;
      String m_lastGhostReplay;
      int m_nGhostFrame;
      enum GhostSearchStrategy GhostSearchStrategies[3];
      bool m_bEnableGhost;
      GhostSearchStrategy m_GhostSearchStrategy;
      bool m_bShowGhostTimeDiff;
      bool m_bGhostMotionBlur;                  /* true: apply fancy motion blur to ghosts */
      bool m_bEnableGhostInfo;
#endif
      std::string m_ReplayPlayerName;
  
      /* WWW */
#if defined(SUPPORT_WEBACCESS)
      bool m_bShowWebHighscoreInGame;           /* true: Show world highscore inside the game */
      WebRoom *m_pWebHighscores;
      WebRooms *m_pWebRooms;
      WebLevels *m_pWebLevels;
      ProxySettings m_ProxySettings;
      std::string m_DownloadingInformation;
      std::string m_DownloadingMessage;
      float m_fDownloadTaskProgressLast;

      bool m_bEnableWebHighscores;              /* true: Read world highscores from website */
      bool m_bWebHighscoresUpdatedThisSession;  /* true: Updated this session */
      bool m_bWebLevelsToDownload;              /* true: there are new levels to download */
      bool m_bEnableCheckNewLevelsAtStartup;
      bool m_bEnableCheckHighscoresAtStartup;

#endif
      
      bool m_bCreditsModeActive;
      
      ThemeChoicer *m_themeChoicer;

      float m_fPrePlayStartTime;
      bool m_bPrePlayAnim;
      float m_fPrePlayCameraLastX;
      float m_fPrePlayCameraLastY;
      float m_fPrePlayStartInitZoom;
      float m_fPrePlayStartCameraX;
      float m_fPrePlayStartCameraY;
      float m_fPreCameraStartX;
      float m_fPreCameraStartY;
      float m_fPreCameraFinalX;
      float m_fPreCameraFinalY;
      float m_zoomX;
      float m_zoomY;
      float m_zoomU;
      float static_time;

      Stats m_GameStats;
      UIWindow *m_pStatsReport;
      
      /* Sound effects */
      SoundSample *m_pEndOfLevelSFX;
      SoundSample *m_pStrawberryPickupSFX;
      SoundSample *m_pWreckerTouchSFX;
      SoundSample *m_pDieSFX;
      
      /* Various popups */
      UIMsgBox *m_pQuitMsgBox;
      UIMsgBox *m_pNotifyMsgBox;
      UIMsgBox *m_pInfoMsgBox;
      UIRect m_InfoMsgBoxRect;

      /* Main menu background / title */
      Texture *m_pTitleBL,*m_pTitleBR,*m_pTitleTL,*m_pTitleTR;
      Texture *m_pCursor;
#if defined(SUPPORT_WEBACCESS)
      Texture *m_pNewLevelsAvailIcon;
#endif
      bool m_bShowCursor;
      
      /* Main menu buttons and stuff */
      int m_nNumMainMenuButtons;
      UITabView *m_pLevelPackTabs;
      UIButton *m_pMainMenuButtons[10];
      UIFrame *m_pOptionsWindow,*m_pHelpWindow,*m_pPlayWindow,*m_pReplaysWindow,*m_pLevelPacksWindow;
      UIWindow *m_pMainMenu;
      UIMsgBox *m_pDeleteReplayMsgBox;
      UIFrame *m_pGameInfoWindow;
      UIFrame *m_pStatsWindow;
      
      /* LEVEL lists */
      UILevelList *m_currentPlayingList;
      UILevelList *m_pAllLevelsList;
#if defined(SUPPORT_WEBACCESS)
      UILevelList *m_pPlayNewLevelsList;
#endif

#if defined(SUPPORT_WEBACCESS)
      UIWindow *m_pLevelInfoFrame;
      UIButton *m_pLevelInfoViewReplayButton;      
      UIStatic *m_pBestPlayerText;

      UIWindow *m_pPackLevelInfoFrame;
      UIButton *m_pPackLevelInfoViewReplayButton;      
      UIStatic *m_pPackBestPlayerText;

      String    m_pLevelToShowOnViewHighscore;
#endif

      /* if true, don't ask for updating levels */
      bool m_updateAutomaticallyLevels;

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
      UIStatic *m_pNewWorldRecord;

      int m_nNumFinishMenuButtons;      
      UIBestTimes *m_pBestTimes;
      
      /* Profile editor fun */
      UIFrame *m_pProfileEditor;  
      UIMsgBox *m_pNewProfileMsgBox;    
      UIMsgBox *m_pDeleteProfileMsgBox;
      
#if defined(SUPPORT_WEBACCESS)
      /* Internet connection configurator */
      UIFrame *m_pWebConfEditor;      
      UIMsgBox *m_pWebConfMsgBox;    
#endif

      /* Level pack viewer fun */
      UIFrame *m_pLevelPackViewer;  
      LevelsPack *m_pActiveLevelPack;
      
      /* Level info viewer fun */
      UIFrame *m_pLevelInfoViewer;      
      std::string m_LevelInfoViewerLevel;
      
      /* Replay saving UI fun */
      UIMsgBox *m_pSaveReplayMsgBox;    
      
      /* Config & profiles */
      UserConfig m_Config;
      PlayerData m_Profiles;

      LevelsManager m_levelsManager;
      
      /* Misc settings */
      MenuBackgroundGraphics m_MenuBackgroundGraphics;
      bool m_bShowMiniMap;
      bool m_bRecordReplays;
      float m_fReplayFrameRate;
      float m_fCurrentReplayFrameRate;
      
      /* Credits */
      Credits *m_pCredits;         
      
      /* Main loop statics */
      double m_fFrameTime;
      float m_fFPS_Rate;

			bool m_bLockMotoGame;

      /* Helpers */
#if defined(SUPPORT_WEBACCESS) 
      void _UpdateWorldRecord(const std::string &LevelID);
#endif
      void _HandleMainMenu(void);  
      void _HandlePauseMenu(void);
      void _HandleJustDeadMenu(void);
      void _HandleFinishMenu(void);
#if defined(SUPPORT_WEBACCESS) 
      void _HandleWebConfEditor(void);
#endif
      void _HandleProfileEditor(void);
      void _HandleLevelInfoViewer(void);
      void _HandleLevelPackViewer(void);
      void _CreateLevelLists(UILevelList *pAllLevels, std::string i_packageName);
      void _CreateReplaysList(UIList *pList);
      void _CreateThemesList(UIList *pList);
#if defined(SUPPORT_WEBACCESS) 
      void _CreateRoomsList(UIList *pList);
#endif
      void _CreateProfileList(void);
      void _CreateDefaultConfig(void);
      void _CreateLevelPackLevelList();
      void _UpdateLevelPackLevelList();
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
      void _InitMenus_PlayingMenus(void);
      void _InitMenus_MainMenu(void);
      void _InitMenus_Others(void);
      void _ImportOptions(void);
      void _DefaultOptions(void);
      void _SaveOptions(void);
      void _UpdateSettings(void);
      void _UpdateLevelLists(void);
      void _UpdateReplaysList(void);
      void _UpdateThemesLists(void);

      void _UpdateLevelsLists();

#if defined(SUPPORT_WEBACCESS) 
      void _UpdateRoomsLists(void);
#endif
      void _GameScreenshot(void);
      void _SaveReplay(const std::string &Name);
    
      void _UpdateLoadingScreen(float fDone,Texture *pScreen,const std::string &NextTask);      
      
      void _SimpleMessage(const std::string &Msg,UIRect *pRect=NULL,bool bNoSwap=false);
      
      int _IsKeyInUse(const std::string &Key);
      
      std::string _DetermineNextLevel(Level *pLevelSrc);
      bool _IsThereANextLevel(Level *pLevelSrc);
      
      bool _IsReplayScripted(Replay *p_pReplay);
      void _RestartLevel(bool i_reloadLevel = false);
  
#if defined(SUPPORT_WEBACCESS)
      void _InitWebConf(void);
      void _CheckForExtraLevels(void);
      void _UpdateWebHighscores(bool bSilent);
      void _UpdateWebLevels(bool bSilent, bool bEnableWeb = true);
      void _UpdateWebThemes(bool bSilent);
      void _UpdateWebTheme(ThemeChoice* pThemeChoice, bool bNotify);
      void _UpgradeWebHighscores();
      void _UpdateWebRooms(bool bSilent);
      void _UpgradeWebRooms(bool bUpdateMenus);
      void _DownloadExtraLevels(void);
      void _UploadHighscore(std::string p_replayname);
      void _ConfigureProxy(void);
#endif

#if defined(SUPPORT_WEBACCESS)
      void setLevelInfoFrameBestPlayer(String pLevelID,
				       UIWindow *i_pLevelInfoFrame,
				       UIButton *i_pLevelInfoViewReplayButton,
				       UIStatic *i_pBestPlayerText
				       );
      void viewHighscoreOf();
#endif

#if defined(ALLOW_GHOST) 
      std::string _getGhostReplayPath_bestOfThePlayer(std::string p_levelId, float &p_time);
      std::string _getGhostReplayPath_bestOfLocal(std::string p_levelId, float &p_time);
#if defined(SUPPORT_WEBACCESS)
      std::string _getGhostReplayPath_bestOfTheRoom(std::string p_levelId, float &p_time);
#endif
      std::string _getGhostReplayPath(std::string p_levelId,
              GhostSearchStrategy p_strategy);
#endif

      void switchUglyMode(bool bUgly);
      void switchTestThemeMode(bool mode);

      void statePrestart_init();
      void statePrestart_step();
      void prestartAnimation_init();
      void prestartAnimation_step();
      
			void zoomAnimation1_init();
			bool zoomAnimation1_step();
			void zoomAnimation1_abort();

			void zoomAnimation2_init();
			bool zoomAnimation2_step();
			bool zoomAnimation2_unstep();
			void zoomAnimation2_abort();

			void lockMotoGame(bool bLock);
			bool isLockedMotoGame() const;

      std::string splitText(const std::string &str, int p_breakLineLength);
      
      /* Main loop utility functions */
      void _UpdateFPSCounter(void);
      void _PrepareFrame(void);
      void _PreUpdateGUI(void);
      void _PreUpdateMenu(void);
      void _DrawMainGUI(void);
      void _DrawMouseCursor(void);
      int _UpdateGamePlaying(void); /* returns number of physics steps performed */
      int _UpdateGameReplaying(void); /* return whether the game state is valid */
      void _PostUpdatePlaying(void);
      void _PostUpdatePause(void);
      void _PostUpdateMenuDead(void);
      void _PostUpdateJustDead(void);
      void _PostUpdateFinished(void);

      int getNumberOfFinishedLevelsOfPack(LevelsPack *i_pack);

			void autoZoom();
  };

}

#endif
