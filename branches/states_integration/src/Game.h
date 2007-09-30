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

#ifndef __GAME_H__
#define __GAME_H__

#include "VCommon.h"
#include "xmscene/Scene.h"
#include "db/xmDatabaseUpdateInterface.h"
#include "XMotoLoadLevelsInterface.h"
#include "WWWAppInterface.h"
#include "Input.h"
#include "WWW.h"
#include "LevelsManager.h"

#define PRESTART_ANIMATION_TIME 2.0
#define INPLAY_ANIMATION_TIME 1.0
#define INPLAY_ANIMATION_SPEED 10
#define PRESTART_ANIMATION_LEVEL_MSG_DURATION 1.0
#define PRESTART_ANIMATION_MARGIN_SIZE 5
#define PRESTART_ANIMATION_CURVE 3.0

/* logf(PRESTART_ANIMATION_CURVE + 1.0) = 1.386294361*/
#define LOGF_PRE_ANIM_TIME_ADDED_ONE 1.386294361

class XMArguments;
class xmDatabase;
class XMSession;
class Img;
class SysMessage;
class GameApp;
class DrawLib;
class WebRoom;
class WebRooms;
class WebLevels;
class LevelsPack;
class UIWindow;
class UIQuickStartButton;
class UILevelList;
class LevelsManager;
class Credits;
class SoundSample;
class StateManager;

   /*===========================================================================
   Overall game states
   ===========================================================================*/
   enum OldGameState {
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
     GS_EDIT_WEBCONFIG         /* Editing internet configuration */
   };

   /*===========================================================================
   Menu background graphical settings
   ===========================================================================*/
   enum MenuBackgroundGraphics {
     MENU_GFX_OFF,
     MENU_GFX_LOW,
     MENU_GFX_HIGH
   };

   class XMMotoGameHooks : public MotoGameHooks {
   public:
     XMMotoGameHooks();
     virtual ~XMMotoGameHooks();
     void setGameApps(GameApp *i_GameApp, MotoGame *i_MotoGame);
     void OnTakeEntity();

   private:
     GameApp  *m_GameApp;
     MotoGame *m_MotoGame;
   };

   /*===========================================================================
   Game application
   ===========================================================================*/
   class GameApp : public XMotoLoadLevelsInterface, public XmDatabaseUpdateInterface, public WWWAppInterface {
     public:
     GameApp();
     ~GameApp();
           
    void run(int nNumArgs, char **ppcArgs);

        /* WWWAppInterface implementation */ 
        virtual void setTaskProgress(float fPercent);

        virtual void setBeingDownloadedInformation(const std::string &p_information,bool p_bNew = true);
        virtual void readEvents(void);
        
        virtual bool shouldLevelBeUpdated(const std::string &LevelID);

	/* load level */
	void loadLevelHook(std::string i_level, int i_percentage);
	void updatingDatabase(std::string i_message);

      /* ** */
      void drawFrame(void);
      void userInit(XMArguments* v_xmArgs);
      void userShutdown(void);
      void keyDown(int nKey, SDLMod mod,int nChar);
      void keyUp(int nKey, SDLMod mod);
      void mouseDown(int nButton);
      void mouseDoubleClick(int nButton);
      void mouseUp(int nButton);
      std::string getConfigThemeName(ThemeChoicer *p_themeChoicer);

      /* Methods */
      MotoGame* getMotoGame();

      void setState(OldGameState s);
      void notifyMsg(std::string Msg);      
      void setPrePlayAnim(bool pEnabled);
      void reloadTheme();

      void setAutoZoom(bool bValue);
      bool AutoZoom();
      void setAutoZoomStep(int n);
      int AutoZoomStep();

      void TeleportationCheatTo(int i_player, Vector2f i_position);

      bool creditsModeActive();

      void initCameras(int nbPlayer);

      void setSpecificReplay(const std::string& i_replay);
      void setSpecificLevelId(const std::string& i_levelID);
      void setSpecificLevelFile(const std::string& i_leveFile);

      /* */
      static double getXMTime(void);
      static std::string getTimeStamp(void);
      void quit(void);
      static std::string formatTime(float fSecs);
      static void getMousePos(int *pnX, int *pnY);
      bool haveMouseMoved(void);
      
      static std::vector<std::string>* getDisplayModes(int windowed);
      Img *grabScreen(void);
      bool isUglyMode();
      /* */

      DrawLib *getDrawLib() {
	return drawLib;
      };
      Theme *getTheme() {
	return &m_theme;
      };
      void setFPS(float i) {
	m_fFramesPerSecond = i;
      }

      void setFrameDelay(int nDelay) {
	m_nFrameDelay = nDelay;
      }

      const std::string & getUserNotify(void) {
	return m_UserNotify;
      }
    
      float getFPS(void) {
	return m_fFramesPerSecond;
      }

      XMSession* getSession();
      void setShowCursor(bool bValue);
      void switchLevelToFavorite(const std::string& i_levelId, bool v_displayMessage = false);

      bool isThereANextLevel(const std::string& i_id_level);
      bool isThereAPreviousLevel(const std::string& i_id_level); 

      // to call while playing
      void restartLevel(bool i_reloadLevel = false);
      void closePlaying();
      void abortPlaying();
      void playNextLevel();
      void playMusic(const std::string& i_music); // "" => no music
      bool isAReplayToSave() const;
      void isTheCurrentPlayAHighscore(bool& o_personal, bool& o_room);
      void saveReplay(const std::string &Name);
      void uploadHighscore(std::string p_replayname, bool b_notify = true);

      // ask the game to close as soon as possible
      void requestEnd();

      //
      xmDatabase* getDb();

      /* public so that we can add progressively the states ; to be removed */
      OldGameState m_State; /* Current state */      
      OldGameState m_StateAfterPlaying;            /* State that should be used later */

   protected:
      void createDefaultConfig();

    private:   
      /* Data */
      bool m_bMultiStopWhenOneFinishes;         /* in multiplayer, stop the game when one finishes the level */
      bool m_bEnableInitZoom;                   /* true: Perform initial level scroll/zoom */
      bool m_autoZoom;                          /* true : the key is pressed so that it zooms out to see the level */
      int m_autoZoomStep;
      bool m_bAutoZoomInitialized;
      bool m_bEnableDeathAnim;                  /* true: Bike falls apart at when dead */
      bool m_bEnableMenuMusic;                  /* true: Play menu music */      
      bool m_bEnableContextHelp;                /* true: Show context help */

      bool m_bEnableEngineSound;                /* true: engine sound is enabled */
      bool m_bCompressReplays;                  /* true: compress replays with zlib */

      ReplayBiker* m_replayBiker; /* link to the replay biker in REPLAYING state */
      bool m_stopToUpdateReplay;
      bool m_allowReplayInterpolation;

      InputHandler m_InputHandler;              /* The glorious input handler */
      MotoGame m_MotoGame;                      /* Game object */      
      XMMotoGameHooks m_MotoGameHooks;
      GameRenderer* m_Renderer;                  /* Renderer */
      int m_nFrame;                             /* Frame # */
       
      double m_fLastFrameTime;                  /* When the last frama was initiated */
      double m_fLastPerfStateTime;   
      double m_fLastPhysTime;                  /* When the last physic was computed */
      double m_fStartTime;                      
      
      std::string m_playingMusic; /* name of the music played to not restart it if the same must be played on an action */

      bool m_b50FpsMode;
      bool m_reloadingLevelsUser;
      
      Replay *m_pJustPlayReplay;

      bool m_bEnableGhost;
      bool m_bShowGhostTimeDiff;
      bool m_bGhostMotionBlur;                  /* true: apply fancy motion blur to ghosts */
      bool m_bEnableGhostInfo;
      bool m_bHideGhosts;

      /* WWW */
      bool m_bShowWebHighscoreInGame;           /* true: Show world highscore inside the game */
      WebRoom *m_pWebHighscores;
      WebRooms *m_pWebRooms;
      WebLevels *m_pWebLevels;
      ProxySettings m_ProxySettings;
      std::string m_DownloadingInformation;
      std::string m_DownloadingMessage;
      float m_fDownloadTaskProgressLast;

      std::string m_WebHighscoresIdRoom;
      std::string m_WebHighscoresURL;
      std::string m_WebHighscoresRoomName;

      bool m_bWebHighscoresUpdatedThisSession;  /* true: Updated this session */
      bool m_bWebLevelsToDownload;              /* true: there are new levels to download */
      bool m_bEnableCheckNewLevelsAtStartup;
      bool m_bEnableCheckHighscoresAtStartup;
      
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

      float fAnimPlayStartZoom;
      float fAnimPlayStartCameraX; 
      float fAnimPlayStartCameraY;
      float fAnimPlayFinalZoom;
      float fAnimPlayFinalCameraX1;
      float fAnimPlayFinalCameraY1;
      float fAnimPlayFinalCameraX2;
      float fAnimPlayFinalCameraY2;

      UIWindow *m_pStatsReport;
      
      UIQuickStartButton *m_pQuickStart;

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
      bool m_bShowCursor;

      /* */
      UIButtonDrawn* m_pNewLevelsAvailable;
      
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
      UILevelList *m_pPlayNewLevelsList;
      UILevelList *m_quickStartList;

      UIWindow *m_pLevelInfoFrame;
      UIButton *m_pLevelInfoViewReplayButton;      
      UIStatic *m_pBestPlayerText;

      UIWindow *m_pPackLevelInfoFrame;
      UIButton *m_pPackLevelInfoViewReplayButton;      
      UIStatic *m_pPackBestPlayerText;

      std::string    m_pLevelToShowOnViewHighscore;

      /* if true, don't ask for updating levels */
      bool m_updateAutomaticallyLevels;

      /* In-game JUSTDEAD menu fun */
      UIFrame *m_pJustDeadMenu;
      int m_nJustDeadShade;
      UIButton *m_pJustDeadMenuButtons[10];            
      int m_nNumJustDeadMenuButtons;      
      float m_fCoolDownEnd;
     
      /* Profile editor fun */
      UIFrame *m_pProfileEditor;  
      UIMsgBox *m_pNewProfileMsgBox;    
      UIMsgBox *m_pDeleteProfileMsgBox;
      
      /* Internet connection configurator */
      UIFrame *m_pWebConfEditor;      
      UIMsgBox *m_pWebConfMsgBox;    

      /* Level pack viewer fun */
      UIFrame *m_pLevelPackViewer;  
      LevelsPack *m_pActiveLevelPack;
      
      /* Level info viewer fun */
      UIFrame *m_pLevelInfoViewer;      
      std::string m_LevelInfoViewerLevel;
      
      /* Replay saving UI fun */
      UIMsgBox *m_pSaveReplayMsgBox;    

      LevelsManager m_levelsManager;
      
      /* Misc settings */
      MenuBackgroundGraphics m_MenuBackgroundGraphics;
      bool m_bShowMiniMap;
      bool m_bRecordReplays;
      float m_fReplayFrameRate;
      
      /* Credits */
      Credits *m_pCredits;         
      
      SysMessage* m_sysMsg;

      /* Main loop statics */
      double m_fFrameTime;
      float m_fFPS_Rate;
      
      std::string m_PlaySpecificReplay;
      std::string m_PlaySpecificLevelId;
      std::string m_PlaySpecificLevelFile;

      bool m_bLockMotoGame;
      xmDatabase *m_db;

      UserConfig m_Config;
      
      Theme m_theme;
      DrawLib *drawLib;

      XMSession* m_xmsession;
      StateManager* m_stateManager;

      int m_nFrameDelay;		/* # of millisecs to wait after screen buffer swap */
      
      float m_fFramesPerSecond;	/* Force this FPS */
      
      /* User nofification */
      std::string m_UserNotify;
      
      /* Run-time fun */
      bool m_bQuit;		/* Quit flag */
      double m_fAppTime;		/* Current application time */
      double m_fNextFrame;	/* Time next frame rendering should begin */

      /* Helpers */
      void _UpdateWorldRecord(const std::string &LevelID);
      void _HandleMainMenu(void);  
      void _HandleJustDeadMenu(void);
      void _HandleWebConfEditor(void);
      void _HandleProfileEditor(void);
      void _HandleLevelInfoViewer(void);
      void _HandleLevelPackViewer(void);
      void _CreateLevelListsSql(UILevelList *pAllLevels, const std::string& i_sql);
      void _CreateLevelLists(UILevelList *pAllLevels, std::string i_packageName);
      void _CreateReplaysList(UIList *pList);
      void _CreateThemesList(UIList *pList);
      void _CreateRoomsList(UIList *pList);
      void _CreateProfileList(void);
      void _CreateLevelPackLevelList();
      void _UpdateLevelPackLevelList(const std::string& v_levelPack);
      void _UpdateActionKeyList(void);
      void _UpdateLevelPackList(void);
      void _UpdateLevelInfoViewerBestTimes(const std::string &LevelID);     
      void _UpdateLevelInfoViewerReplays(const std::string &LevelID);     
      void _ChangeKeyConfig(void);
      void _ConfigureJoystick(void);
      void _DrawMenuBackground(void); 
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
      void _UpdateCurrentPackList(const std::string& i_id_level, float i_playerHighscore);

      void _UpdateRoomsLists(void);
      void _GameScreenshot(void);
    
      void _UpdateLoadingScreen(float fDone, const std::string &NextTask);
      
      void _SimpleMessage(const std::string &Msg,UIRect *pRect=NULL,bool bNoSwap=false);
      
      int _IsKeyInUse(const std::string &Key);
      
      std::string _DetermineNextLevel(const std::string& i_id_level);
      std::string _DeterminePreviousLevel(const std::string& i_id_level);
  
      void _InitWebConf(void);
      void _CheckForExtraLevels(void);
      void _UpdateWebHighscores(bool bSilent);
      void _UpdateWebLevels(bool bSilent, bool bEnableWeb = true);
      void _UpdateWebThemes(bool bSilent);
      void _UpdateWebTheme(const std::string& i_id_theme, bool bNotify);
      void _UpgradeWebHighscores();
      void _UpdateWebRooms(bool bSilent);
      void _UpgradeWebRooms(bool bUpdateMenus);
      void _DownloadExtraLevels(void);
      void _UploadAllHighscores();
      void _ConfigureProxy(void);

      void setLevelInfoFrameBestPlayer(std::string pLevelID,
				       UIWindow *i_pLevelInfoFrame,
				       UIButton *i_pLevelInfoViewReplayButton,
				       UIStatic *i_pBestPlayerText
				       );
      void viewHighscoreOf();
      void enableWWW(bool bValue);
      void enableFps(bool bValue);

      UILevelList* buildQuickStartList();

      std::string _getGhostReplayPath_bestOfThePlayer(std::string p_levelId, float &p_time);
      std::string _getGhostReplayPath_bestOfLocal(std::string p_levelId, float &p_time);
      std::string _getGhostReplayPath_bestOfTheRoom(std::string p_levelId, float &p_time);
      void addGhosts(MotoGame* i_motogame, Theme* i_theme);

      void switchUglyMode(bool bUgly);
      void switchTestThemeMode(bool mode);
      void switchUglyOverMode(bool mode);

      void statePrestart_init();
      void statePrestart_step();
      void prestartAnimation_init();
      void prestartAnimation_step();
      
      void zoomAnimation1_init();
      bool zoomAnimation1_step();
      void zoomAnimation1_abort();
      
      void zoomAnimation2_init();
      bool zoomAnimation2_step();
      void zoomAnimation2_init_unzoom();
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
      void _PostUpdateMenuDead(void);
      void _PostUpdateJustDead(void);

      void autoZoom();

      int getNumberOfPlayersToPlay();
      TColor getColorFromPlayerNumber(int i_player);
      TColor getUglyColorFromPlayerNumber(int i_player);

      UIWindow* stats_generateReport(const std::string &PlayerName, UIWindow *pParent,
				     int x, int y, int nWidth, int nHeight, FontManager* pFont);

      void initReplaysFromDir();
      void addReplay(const std::string& i_file);

      static UIFrame* makeOptionsWindow(DrawLib* i_drawLib, UIWindow* io_parent, UserConfig* i_Config);
      static UIFrame* makeHelpWindow(DrawLib* i_drawLib, UIWindow* io_parent, UserConfig* i_Config);

      void updatePlayerTag();
      
      void addLevelToFavorite(const std::string& i_levelId);

      void switchFollowCamera();

      /* */
      void _InitWin(bool bInitGraphics);
      void _Uninit(void);
  };

#endif
