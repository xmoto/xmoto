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
class UILevelList;
class LevelsManager;
class SoundSample;
class StateManager;
class XMotoLoadReplaysInterface;

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

      /* Methods */
      MotoGame* getMotoGame();

      void notifyMsg(std::string Msg);      
      void reloadTheme();

      void TeleportationCheatTo(int i_player, Vector2f i_position);

      void initCameras(int nbPlayer);

      void setSpecificReplay(const std::string& i_replay);
      void setSpecificLevelId(const std::string& i_levelID);
      void setSpecificLevelFile(const std::string& i_leveFile);

      /* */
      static double getXMTime(void);
      static int    getXMTimeInt(void);
      static std::string getTimeStamp(void);
      void quit(void);
      static std::string formatTime(float fSecs);
      static void getMousePos(int *pnX, int *pnY);
      bool haveMouseMoved(void);
      
      Img *grabScreen(void);

      DrawLib *getDrawLib() {
	return drawLib;
      };
      Theme *getTheme() {
	return &m_theme;
      };

      UserConfig* getUserConfig() { /* to remove */
	return &m_Config;
      }

      XMSession* getSession();
      void switchLevelToFavorite(const std::string& i_levelId, bool v_displayMessage = false);

      bool isThereANextLevel(const std::string& i_id_level);
      bool isThereAPreviousLevel(const std::string& i_id_level); 

      std::string getWorldRecord(const std::string &LevelID);

      void addGhosts(MotoGame* i_motogame, Theme* i_theme);

      // to call while playing
      void playMusic(const std::string& i_music); // "" => no music
      bool isAReplayToSave() const;
      Replay* getCurrentReplay();
      void initReplay();
      void isTheCurrentPlayAHighscore(bool& o_personal, bool& o_room);
      void saveReplay(const std::string &Name);
      void uploadHighscore(std::string p_replayname, bool b_notify = true);
      void switchFollowCamera();

      // ask the game to close as soon as possible
      void requestEnd();

      TColor getColorFromPlayerNumber(int i_player);
      TColor getUglyColorFromPlayerNumber(int i_player);

      //
      xmDatabase*    getDb();
      LevelsManager* getLevelsManager();
      StateManager*  getStateManager();
      GameRenderer*  getGameRenderer();
      InputHandler*  getInputHandler();
      ThemeChoicer*  getThemeChoicer();

      SysMessage* getSysMessage() { return m_sysMsg;}

      /* call to close the replay */
      void finalizeReplay(bool i_finished);
      void updateLevelsListsOnEnd();

      static std::string splitText(const std::string &str, int p_breakLineLength);
      void addLevelToFavorite(const std::string& i_levelId);

      void setLevelInfoFrameBestPlayer(std::string pLevelID,
				       UIWindow *i_pLevelInfoFrame,
				       UIButton *i_pLevelInfoViewReplayButton,
				       UIStatic *i_pBestPlayerText
				       );
      void viewHighscoreOf();

      // list played
      void setCurrentPlayingList(UILevelList *i_levelsList) {m_currentPlayingList = i_levelsList;}

      void updateWebHighscores();
      void checkForExtraLevels();

      std::string getWebRoomURL();
      std::string getWebRoomName();
    
      std::string determineNextLevel(const std::string& i_id_level);
      std::string determinePreviousLevel(const std::string& i_id_level);

      void initReplaysFromDir(xmDatabase* threadDb = NULL,
			      XMotoLoadReplaysInterface* pLoadReplaysInterface = NULL);

      void gameScreenshot();
      void enableWWW(bool bValue);
      void enableFps(bool bValue);
      void switchUglyMode(bool bUgly);
      void switchTestThemeMode(bool mode);
      void switchUglyOverMode(bool mode);


   protected:
      void createDefaultConfig();

    private:   
      ReplayBiker* m_replayBiker; /* link to the replay biker in REPLAYING state */
      InputHandler m_InputHandler;              /* The glorious input handler */
      MotoGame m_MotoGame;                      /* Game object */      
      XMMotoGameHooks m_MotoGameHooks;
      GameRenderer* m_Renderer;                  /* Renderer */
       
      std::string m_playingMusic; /* name of the music played to not restart it if the same must be played on an action */

      bool m_reloadingLevelsUser;
      
      Replay *m_pJustPlayReplay;

      /* WWW */
      WebRoom *m_pWebHighscores;
      WebLevels *m_pWebLevels;
      ProxySettings m_ProxySettings;
      std::string m_DownloadingInformation;
      std::string m_DownloadingMessage;
      float m_fDownloadTaskProgressLast;

      bool m_bWebHighscoresUpdatedThisSession;  /* true: Updated this session */
      bool m_bWebLevelsToDownload;              /* true: there are new levels to download */
      
      ThemeChoicer *m_themeChoicer;

      /* Sound effects */
      SoundSample *m_pEndOfLevelSFX;
      SoundSample *m_pStrawberryPickupSFX;
      SoundSample *m_pWreckerTouchSFX;
      SoundSample *m_pDieSFX;
      
      /* Various popups */
      UIMsgBox *m_pNotifyMsgBox;
      UIMsgBox *m_pInfoMsgBox;
      UIRect m_InfoMsgBoxRect;

      /* Main menu buttons and stuff */
      int m_nNumMainMenuButtons;
      UITabView *m_pLevelPackTabs;
      UIButton *m_pMainMenuButtons[10];
      UIFrame *m_pOptionsWindow,*m_pPlayWindow,*m_pReplaysWindow,*m_pLevelPacksWindow;
      UIWindow *m_pMainMenu;
      UIFrame *m_pGameInfoWindow;
      
      /* LEVEL lists */
      UILevelList *m_currentPlayingList;
      UILevelList *m_pAllLevelsList;
      UILevelList *m_pPlayNewLevelsList;

      UIWindow *m_pLevelInfoFrame;
      UIButton *m_pLevelInfoViewReplayButton;      
      UIStatic *m_pBestPlayerText;

      UIWindow *m_pPackLevelInfoFrame;
      UIButton *m_pPackLevelInfoViewReplayButton;      
      UIStatic *m_pPackBestPlayerText;

      std::string    m_pLevelToShowOnViewHighscore;

      /* if true, don't ask for updating levels */
      bool m_updateAutomaticallyLevels;
     
      /* Internet connection configurator */
      UIFrame *m_pWebConfEditor;      
      UIMsgBox *m_pWebConfMsgBox;    

      /* Level pack viewer fun */
      UIFrame *m_pLevelPackViewer;  
      
      /* Replay saving UI fun */
      UIMsgBox *m_pSaveReplayMsgBox;    

      LevelsManager m_levelsManager;

      SysMessage* m_sysMsg;

      /* Main loop statics */
      double m_fFrameTime;
      float m_fFPS_Rate;
      
      std::string m_PlaySpecificReplay;
      std::string m_PlaySpecificLevelId;
      std::string m_PlaySpecificLevelFile;

      xmDatabase *m_db;

      UserConfig m_Config;
      
      Theme m_theme;
      DrawLib *drawLib;

      XMSession* m_xmsession;
      StateManager* m_stateManager;
      
      /* Run-time fun */
      bool m_bQuit;		/* Quit flag */

      // calculate sleeping time
      int m_lastFrameTimeStamp;
      int m_frameLate;

      /* Helpers */
      void _Wait();
      void _HandleMainMenu(void);  
      void _InitMenus(void);        
      void _InitMenus_MainMenu(void);
      void _UpdateCurrentPackList(const std::string& i_id_level, float i_playerHighscore);
    
      void _UpdateLoadingScreen(float fDone, const std::string &NextTask);
      
      void _SimpleMessage(const std::string &Msg,UIRect *pRect=NULL,bool bNoSwap=false);
      
      void _UpdateWebHighscores(bool bSilent);
      void _UpdateWebLevels(bool bSilent, bool bEnableWeb = true);
      void _UpgradeWebHighscores();
      void _DownloadExtraLevels(void);
      void _UploadAllHighscores();


      std::string _getGhostReplayPath_bestOfThePlayer(std::string p_levelId, float &p_time);
      std::string _getGhostReplayPath_bestOfLocal(std::string p_levelId, float &p_time);
      std::string _getGhostReplayPath_bestOfTheRoom(std::string p_levelId, float &p_time);

      /* Main loop utility functions */
      void _PreUpdateGUI(void);

      void addReplay(const std::string& i_file, xmDatabase* threadDb = NULL);

      static UIFrame* makeOptionsWindow(DrawLib* i_drawLib, UIWindow* io_parent, UserConfig* i_Config);

      /* */
      void _InitWin(bool bInitGraphics);
      void _Uninit(void);
  };

#endif
