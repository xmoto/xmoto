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
 *  Game application.
 */
#include "Game.h"
#include "VFileIO.h"
#include "Sound.h"
#include "PhysSettings.h"
#include "Input.h"
#include "xmscene/Bike.h"
#include "xmscene/BikeGhost.h"
#include "xmscene/BikePlayer.h"
#include "db/xmDatabase.h"
#include "helpers/Log.h"
#include "XMSession.h"
#include "drawlib/DrawLib.h"
#include "Image.h"
#include "SysMessage.h"
#include "GameText.h"
#include "Credits.h"
#include "gui/specific/GUIXMoto.h"
#include "xmscene/Camera.h"
#include "xmscene/Entity.h"

#include <curl/curl.h>
#include <iomanip.h>


  bool GameApp::haveMouseMoved() {
    int nX,nY;
    SDL_GetRelativeMouseState(&nX,&nY);
    return (nX == 0 && nY == 0);
  }

  void GameApp::getMousePos(int *pnX,int *pnY) {
    SDL_GetMouseState(pnX, pnY);
  }
  
  /*===========================================================================
  Get real-time clock
  ===========================================================================*/
  std::string GameApp::getTimeStamp(void) {
    struct tm *pTime;
    time_t T;
    char cBuf[256] = "";
    time(&T);
    pTime = localtime(&T);
    if(pTime != NULL) {
      sprintf(cBuf,"%d-%02d-%02d %02d:%02d:%02d",
              pTime->tm_year+1900, pTime->tm_mon+1, pTime->tm_mday,
	      pTime->tm_hour, pTime->tm_min, pTime->tm_sec);                    
    }    
    return cBuf;
  }
  
  double GameApp::getXMTime(void) {
    return SDL_GetTicks() / 1000.0f;
  }
  
  std::string GameApp::formatTime(float fSecs) {
    char cBuf[256];
    int nM, nS, nH;
    float nHres;

    nM = (int)(fSecs/60.0);
    nS = (int)(fSecs - ((float)nM)*60.0);
    nHres = (fSecs - ((float)nM)*60.0 - ((float)nS));
    nH = (int)(nHres * 100.0);

    /* hum, case, in which 0.9800 * 100.0 => 0.9799999*/
    if(((int)(nHres * 100.0)) < ((int)((nHres * 100.0) + 0.001))) {
      nH = ((int)((nHres * 100.0) + 0.001));
      nH %= 100;
    }

    sprintf(cBuf,"%02d:%02d:%02d", nM, nS, nH);
    return cBuf;
  }
  
  /*===========================================================================
  Quits the application
  ===========================================================================*/
  void GameApp::quit(void) {
    /* Set quit flag */
    m_bQuit = true;
  }
  
  /*===========================================================================
  Init 
  ===========================================================================*/
  void GameApp::_InitWin(bool bInitGraphics) {

    /* Init SDL */
    if(bInitGraphics == false) {
      if(SDL_Init(SDL_INIT_TIMER) < 0)
        throw Exception("(1) SDL_Init : " + std::string(SDL_GetError()));
      
      /* No graphics mojo here, thank you */
      return;
    } else {
      if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0)
        throw Exception("(2) SDL_Init : " + std::string(SDL_GetError()));
    }
    /* Set window title */
    SDL_WM_SetCaption(XMBuild::getVersionString(true).c_str(), XMBuild::getVersionString(true).c_str());

#if !defined(WIN32) && !defined(__APPLE__) 
    SDL_Surface *v_icon = SDL_LoadBMP(GAMEDATADIR "/xmoto_icone_x.ico");
    if(v_icon != NULL) {
      SDL_SetColorKey(v_icon, SDL_SRCCOLORKEY,
		      SDL_MapRGB(v_icon->format, 236, 45, 211));
      SDL_WM_SetIcon(v_icon, NULL);
    }
#endif

    if(TTF_Init() < 0) {
      throw Exception("Initializing TTF failed: " + std::string(TTF_GetError()));
    }
    atexit(TTF_Quit);
  }

  /*===========================================================================
  Uninit 
  ===========================================================================*/
  void GameApp::_Uninit(void) {
    /* Tell user app to turn off */
    userShutdown();

    if(m_xmsession->useGraphics()) {
      /* Uninit drawing library */
      drawLib->unInit();
    }
    
    Logger::uninit();
    
    /* Shutdown SDL */
    SDL_Quit();
  }
  
  /*===========================================================================
    Return available display modes
    ===========================================================================*/
  std::vector<std::string>* GameApp::getDisplayModes(int windowed){
    std::vector<std::string>* modes = new std::vector<std::string>;
    SDL_Rect **sdl_modes;
    int i, nFlags;

    /* Always use the fullscreen flags to be sure to
       always get a result (no any modes available like in windowed) */
    nFlags = SDL_OPENGL | SDL_FULLSCREEN;

    /* Get available fullscreen/hardware modes */
    sdl_modes = SDL_ListModes(NULL, nFlags);

    /* Check is there are any modes available */
    if(sdl_modes == (SDL_Rect **)0){
      Logger::Log("** Warning ** : No display modes available.");
      throw Exception("getDisplayModes : No modes available.");
    }

    /* Always include these to modes */
    modes->push_back("800 X 600");
    modes->push_back("1024 X 768");
    modes->push_back("1280 X 1024");
    modes->push_back("1600 X 1200");

    /* Print valid modes */
    //Log("Available Modes :");

    for(i=0; sdl_modes[i]; i++){
      char tmp[128];

      /* Menus don't fit under 800x600 */
      if(sdl_modes[i]->w < 800 || sdl_modes[i]->h < 600)
	continue;

      snprintf(tmp, 126, "%d X %d",
	       sdl_modes[i]->w,
	       sdl_modes[i]->h);
      tmp[127] = '\0';

      /* Only single */
      bool findDouble = false;
      //Log("size: %d", modes->size());
      for(unsigned int j=0; j<modes->size(); j++)
	if(!strcmp(tmp, (*modes)[j].c_str())){
	  findDouble = true;
	  break;
	}

      if(!findDouble){
	modes->push_back(tmp);
	//Log("  %s", tmp);
      }
    }

    return modes;
  }

  bool GameApp::isUglyMode() {
    return m_xmsession->ugly();
  }

GameApp::~GameApp() {
  if(m_db != NULL) {
    delete m_db;
  }

  if(drawLib != NULL) {
    delete drawLib;
  }

  delete m_xmsession;
}

GameApp::GameApp() {
  m_bQuit = false;
  m_fAppTime = 0.0f;
  m_UserNotify = "";
  m_fFramesPerSecond = 25.0f;
  m_fNextFrame = 0.0f;
  m_nFrameDelay = 0;
  drawLib = NULL;
  m_Renderer = NULL;
  
  m_xmsession = new XMSession();
  m_sysMsg = NULL;

  m_pCredits = NULL;
  m_bEnableMenuMusic=false;
  m_bEnableInitZoom=true;
  m_bMultiStopWhenOneFinishes=true;
  m_autoZoom = false;
  m_autoZoomStep = 0;
  m_bAutoZoomInitialized = false;
  m_bLockMotoGame = false;
  m_bEnableDeathAnim=true;
  m_pQuitMsgBox=NULL;
  m_pNotifyMsgBox=NULL;
  m_pInfoMsgBox=NULL;

  m_pWebConfEditor=NULL;
  m_pWebConfMsgBox=NULL;

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
  m_pJustPlayReplay = NULL;
  m_updateAutomaticallyLevels = false;
  m_reloadingLevelsUser = false;

  m_bEnableGhost = true;
  m_bShowGhostTimeDiff = true;
  m_bEnableGhostInfo = false;
  m_bHideGhosts = false;
  m_bGhostMotionBlur = true;

  m_bAutosaveHighscoreReplays = true;
  m_bRecordReplays = true;
  m_bShowCursor = true;
  m_bEnableEngineSound = true;
  m_bCompressReplays = true;
  m_bEnableContextHelp = true;     
  
  m_bShowWebHighscoreInGame = false;
  m_pWebHighscores = NULL;
  m_pWebLevels = NULL;
  m_pWebRooms = NULL;
  m_fDownloadTaskProgressLast = 0;
  m_bWebHighscoresUpdatedThisSession = false;
  m_bWebLevelsToDownload = false;
  
  m_bEnableCheckNewLevelsAtStartup  = true;
  m_bEnableCheckHighscoresAtStartup = true;
  
  m_MotoGame.setHooks(&m_MotoGameHooks);
  m_MotoGameHooks.setGameApps(this, &m_MotoGame);
  
  m_bPrePlayAnim = true;
  
  m_currentPlayingList = NULL;
  m_fReplayFrameRate = 25.0;
  m_stopToUpdateReplay = false;
  m_allowReplayInterpolation = true;

  m_quickStartList = NULL;

  m_db = NULL;
}
    
  std::string GameApp::splitText(const std::string &str, int p_breakLineLength) {
    std::string v_res = "";
    char c[2] = {' ', '\0'};    
    int lineLength = 0;

    for(int i=0; i<str.length(); i++) {
      if((lineLength > p_breakLineLength && str[i] == ' ') ||
        str[i] == '\n') {
        c[0] = '\n';
        v_res.append(c);
        lineLength = 0;
      } else {
        c[0] = str[i];
        v_res.append(c);
        lineLength++;
      }
    }
    return v_res;
  }
  
  /*===========================================================================
  Update levels lists - must be done after each completed level
  ===========================================================================*/
  void GameApp::_UpdateLevelLists(void) {
    LevelsPack *v_levelsPack;

    _CreateLevelLists((UILevelList *)m_pLevelPacksWindow->getChild("LEVELPACK_TABS:ALLLEVELS_TAB:ALLLEVELS_LIST"), VPACKAGENAME_FAVORITE_LEVELS);

    _CreateLevelLists(m_pPlayNewLevelsList, VPACKAGENAME_NEW_LEVELS);
  }

  /*===========================================================================
  Update replays list
  ===========================================================================*/
  void GameApp::_UpdateReplaysList(void) {
    _CreateReplaysList((UIList *)m_pReplaysWindow->getChild("REPLAY_LIST"));                       
  }

  void GameApp::_UpdateRoomsLists(void) {
    _CreateRoomsList((UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:WWW_TAB:WWWOPTIONS_TABS:WWW_ROOMS_TAB:ROOMS_LIST"));
  }

  void GameApp::_UpdateThemesLists(void) {
    _CreateThemesList((UIList *)m_pOptionsWindow->getChild("OPTIONS_TABS:GENERAL_TAB:THEMES_LIST"));
  }

  /*===========================================================================
  Change game state
  ===========================================================================*/
  void GameApp::setState(GameState s) {
    /* This function is called to perform a controlled game state change.
       The various states are described below in the switch-statement */  
    m_State = s;
    m_bCreditsModeActive = false;
    std::string v_newMusicPlaying;
    char **v_result;
    unsigned int nrow;
    char *v_res;

    /* Always clear context when changing state */
    m_Renderer->getGUI()->clearContext();
    
    /* reallow particle renderering when changing of state */
    ParticlesSource::setAllowParticleGeneration(true);

    switch(s) {
		case GS_LEVELPACK_VIEWER: {
			v_newMusicPlaying = "menu1";
			m_pLevelPackViewer->showWindow(true);
			m_pMainMenu->showWindow(true);
          
			UIList *pList = (UIList *)m_pLevelPackViewer->getChild("LEVELPACK_LEVEL_LIST");
			if(pList != NULL) {
				pList->makeActive();
			}
			setPrePlayAnim(true);
		}
			break;
		case GS_CREDITSMODE:
		case GS_REPLAYING: {
			m_stopToUpdateReplay = false;
			v_newMusicPlaying = "";
			m_Renderer->setShowEngineCounter(false);

			try {  
				/* ghost, replay */
				m_replayBiker = NULL;
	  
				m_bShowCursor = false;
				bool bCreditsMode = (m_State == GS_CREDITSMODE);
				m_bCreditsModeActive = bCreditsMode;
				m_State = GS_REPLAYING;
	  
				int nbPlayer = 1;
				initCameras(nbPlayer);

				try {
					m_replayBiker = m_MotoGame.addReplayFromFile(m_PlaySpecificReplay,
										     &m_theme, m_theme.getPlayerTheme(),
										     m_bEnableEngineSound);
					m_MotoGame.getCamera()->setPlayerToFollow(m_replayBiker);
				} catch(Exception &e) {
					setState(m_StateAfterPlaying);
					notifyMsg("Unable to read the replay: " + e.getMsg());
					return;
				}

				/* Credits mode? */
				if(bCreditsMode) {
					if(m_pCredits == NULL)
						m_pCredits = new Credits;
	    
	    m_pCredits->init(this, m_replayBiker->getFinishTime(),4,4,
			     std::string(GAMETEXT_CREDITS).c_str());
	  }

				/* Fine, open the level */
				try {
					m_MotoGame.loadLevel(m_db, m_replayBiker->levelId());
				} catch(Exception &e) {
					setState(m_StateAfterPlaying);
					notifyMsg(e.getMsg());     
					return;
				}

				if(m_MotoGame.getLevelSrc()->isXMotoTooOld()) {
					Logger::Log("** Warning ** : level '%s' specified by replay '%s' requires newer X-Moto",m_replayBiker->levelId().c_str(),m_PlaySpecificReplay.c_str());	    
					char cBuf[256];
					sprintf(cBuf,GAMETEXT_NEWERXMOTOREQUIRED,
									m_MotoGame.getLevelSrc()->getRequiredVersion().c_str());
					m_MotoGame.endLevel();
					setState(m_StateAfterPlaying);
					notifyMsg(cBuf); 
					return;
				}
  
	  /* Init level */    
	  m_InputHandler.reset();
	  //m_InputHandler.setMirrored(m_MotoGame.getCamera()->isMirrored());
	  m_MotoGame.prePlayLevel(&m_InputHandler, NULL, false);

	  /* add the ghosts */
	  if(m_bEnableGhost) {
	    try {
	      addGhosts(&m_MotoGame, &m_theme);
	    } catch(Exception &e) {
	      /* anyway */
	    }
	  }

	  /* *** */
	  
	  char c_tmp[1024];
	  snprintf(c_tmp, 1024,
		   GAMETEXT_BY_PLAYER,
		   m_replayBiker->playerName().c_str()
		   );
	  m_MotoGame.setInfos(m_MotoGame.getLevelSrc()->Name() + " " + std::string(c_tmp));
	  
	  m_nFrame = 0;
	  m_Renderer->prepareForNewLevel(bCreditsMode);            
	  v_newMusicPlaying = m_MotoGame.getLevelSrc()->Music();
	  
	  /* Show help string */
	  if(!drawLib->isNoGraphics()) {
	    std::string T1 = "--:--:--",T2 = "--:--:--";
	    
	    /* get best result */
	    v_result = m_db->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
				    "id_level=\"" + 
				    xmDatabase::protectString(m_MotoGame.getLevelSrc()->Id()) + "\";",
				    nrow);
	    v_res = m_db->getResult(v_result, 1, 0, 0);
	    if(v_res != NULL) {
	      T1 = formatTime(atof(v_res));
	    }
	    m_db->read_DB_free(v_result);
	    
	    /* get best player result */
	    v_result = m_db->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
				    "id_level=\"" + 
				    xmDatabase::protectString(m_MotoGame.getLevelSrc()->Id()) + "\" " + 
				    "AND id_profile=\"" + xmDatabase::protectString(m_xmsession->profile())  + "\";",
				    nrow);
	    v_res = m_db->getResult(v_result, 1, 0, 0);
	    if(v_res != NULL) {
	      T2 = formatTime(atof(v_res));
	    }
	    m_db->read_DB_free(v_result);
	    
	    m_Renderer->setBestTime(T1 + std::string(" / ") + T2);
	    m_Renderer->showReplayHelp(m_MotoGame.getSpeed(),
				      m_MotoGame.getLevelSrc()->isScripted() == false);
	    
	    if(m_xmsession->benchmark() || bCreditsMode) m_Renderer->setBestTime("");
	    
	    /* World-record stuff */
	    if(!bCreditsMode)
	      _UpdateWorldRecord(m_MotoGame.getLevelSrc()->Id());
	  }
	  m_fStartTime = getXMTime();
	  
			} catch(Exception &e) {
			  m_MotoGame.endLevel();
			  setState(m_StateAfterPlaying);
				notifyMsg(splitText(e.getMsg(), 50));   
			}
			break;
		}  
    case GS_MENU: {
			v_newMusicPlaying = "menu1";

			//SDL_ShowCursor(SDL_ENABLE);
			m_bShowCursor = true;                
        
			/* The main menu, the one which is entered initially when the game 
				 begins. */
			m_pMainMenu->showWindow(true);

			// enable the preplay animation
			setPrePlayAnim(true);
			break;
		}
		case GS_PREPLAYING: {
			/* because statePrestart_init() can call setState */
			if(m_bEnableMenuMusic && Sound::isEnabled()) {
				Sound::stopMusic();
				m_playingMusic = "";
			}
			statePrestart_init();
			return;
			break;
		}
		case GS_PLAYING: {
			m_Renderer->setShowEngineCounter(m_Config.getBool("ShowEngineCounter"));
			v_newMusicPlaying = "";

			m_bAutoZoomInitialized = false;
				
			try {
				m_MotoGame.playLevel();
				m_State = GS_PLAYING;        
				m_nFrame = 0;
				v_newMusicPlaying = m_MotoGame.getLevelSrc()->Music();
			} catch(Exception &e) {
				Logger::Log("** Warning ** : level '%s' cannot be loaded",m_PlaySpecificLevelId.c_str());
				m_MotoGame.endLevel();
				char cBuf[256];
				sprintf(cBuf,GAMETEXT_LEVELCANNOTBELOADED,m_PlaySpecificLevelId.c_str());
				setState(m_StateAfterPlaying);
				notifyMsg(cBuf);
			}
			break;
		}
		case GS_PAUSE: {
			m_MotoGame.setInfos(m_MotoGame.getLevelSrc()->Name());
			v_newMusicPlaying = m_playingMusic;
			//        SDL_ShowCursor(SDL_ENABLE);
			m_bShowCursor = true;

			/* Paused from GS_PLAYING */
			break;
		}
		case GS_DEADJUST: {
			m_MotoGame.setInfos(m_MotoGame.getLevelSrc()->Name());
			v_newMusicPlaying = "";

			/* Finish replay */
			if(m_pJustPlayReplay != NULL) {
				if(m_MotoGame.Players().size() == 1) {
				  
				  /* save the last state because scene don't record each frame */
				  SerializedBikeState BikeState;
				  MotoGame::getSerializedBikeState(m_MotoGame.Players()[0]->getState(), m_MotoGame.getTime(), &BikeState);
				  m_pJustPlayReplay->storeState(BikeState);
				  m_pJustPlayReplay->finishReplay(false,0.0f);
				}
			}

			/* Update stats */        
			if(m_MotoGame.Players().size() == 1) {
			  m_db->stats_died(m_xmsession->profile(),
					   m_MotoGame.getLevelSrc()->Id(),
					   m_MotoGame.getTime());
			}                

			/* Play the DIE!!! sound */
			try {
				Sound::playSampleByName(m_theme.getSound("Headcrash")->FilePath(),0.3);
			} catch(Exception &e) {
			}

			m_nJustDeadShade = 0;

			if(m_bEnableDeathAnim) {
				m_MotoGame.gameMessage(GAMETEXT_JUSTDEAD_RESTART,     false, 15);
				m_MotoGame.gameMessage(GAMETEXT_JUSTDEAD_DISPLAYMENU, false, 15);
			} else {
				setState(GS_DEADMENU);
			}
			break;
		}
    case GS_DEADMENU: {
      v_newMusicPlaying = "";

      m_bShowCursor = true;
      m_pJustDeadMenu->showWindow(true);

      /* Possible exit of GS_PLAYING, when the player is dead */
      m_fCoolDownEnd = getXMTime() + 0.3f;
      break;
    }
		case GS_EDIT_PROFILES: {
			v_newMusicPlaying = "menu1";
			//        SDL_ShowCursor(SDL_ENABLE);
			m_bShowCursor = true;

			/* The profile editor can work on top of the main menu, or as init
				 state when there is no player profiles available */
			m_pProfileEditor->showWindow(true);
			break;
		}

      case GS_EDIT_WEBCONFIG: {
	v_newMusicPlaying = "menu1";
        m_bShowCursor = true;
        if(m_pWebConfMsgBox != NULL) delete m_pWebConfMsgBox;
        m_pWebConfEditor->showWindow(false);
	m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
        m_pWebConfMsgBox = m_Renderer->getGUI()->msgBox(GAMETEXT_ALLOWINTERNETCONN,
                                                       (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
        break;
      }

		case GS_FINISHED: {
			m_MotoGame.setInfos(m_MotoGame.getLevelSrc()->Name());
			v_newMusicPlaying = "";

			//        SDL_ShowCursor(SDL_ENABLE);
			m_bShowCursor = true;

			/* Finish replay */
			if(m_pJustPlayReplay != NULL) {
				if(m_MotoGame.Players().size() == 1) {
				  /* save the last state because scene don't record each frame */
				  SerializedBikeState BikeState;
				  MotoGame::getSerializedBikeState(m_MotoGame.Players()[0]->getState(), m_MotoGame.getTime(), &BikeState);
				  m_pJustPlayReplay->storeState(BikeState);
				  m_pJustPlayReplay->finishReplay(true,m_MotoGame.Players()[0]->finishTime());
				}
			}

			/* A more lucky outcome of GS_PLAYING than GS_DEADMENU :) */
			m_pFinishMenu->showWindow(true);
			m_pBestTimes->showWindow(true);
			m_nFinishShade = 0;            

			if(m_MotoGame.Players().size() == 1) {
				/* display message on finish and eventually save the replay */
        
				/* is it a highscore ? */
				float v_best_personal_time;
				float v_best_room_time;
				float v_current_time;
				bool v_is_a_highscore;
				bool v_is_a_personal_highscore;
  
				/* get best player result */
				v_result = m_db->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
																"id_level=\"" + 
																xmDatabase::protectString(m_MotoGame.getLevelSrc()->Id()) + "\" " + 
																"AND id_profile=\"" + xmDatabase::protectString(m_xmsession->profile())  + "\";",
																nrow);
				v_res = m_db->getResult(v_result, 1, 0, 0);
				if(v_res != NULL) {
					v_best_personal_time = atof(v_res);
				} else {
					/* should never happend because the score is already stored */
					v_best_personal_time = -1.0;
				}
				m_db->read_DB_free(v_result);

				v_current_time = m_MotoGame.Players()[0]->finishTime();
	  
				v_is_a_personal_highscore = (v_current_time <= v_best_personal_time
																		 || v_best_personal_time < 0.0);
	  
				/* search a better webhighscore */
				v_best_room_time = m_db->webrooms_getHighscoreTime(m_WebHighscoresIdRoom,
																													 m_MotoGame.getLevelSrc()->Id());
				v_is_a_highscore = (v_current_time < v_best_room_time
														|| v_best_room_time < 0.0);
	  
				// disable upload button
				for(int i=0;i<m_nNumFinishMenuButtons;i++) {
					if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_UPLOAD_HIGHSCORE) {
						m_pFinishMenuButtons[i]->enableWindow(false);
					}
				}
	  
				if(v_is_a_highscore) { /* best highscore */
					try {
						Sound::playSampleByName(m_theme.getSound("NewHighscore")->FilePath());
					} catch(Exception &e) {
					}
	    
					// enable upload button
					if(m_xmsession->www()) {
						if(m_pJustPlayReplay != NULL) {
							for(int i=0;i<m_nNumFinishMenuButtons;i++) {
								if(m_pFinishMenuButtons[i]->getCaption() == GAMETEXT_UPLOAD_HIGHSCORE) {
									m_pFinishMenuButtons[i]->enableWindow(true);
								}
							}
						}
					}
	    
					if(m_pJustPlayReplay != NULL && m_bAutosaveHighscoreReplays) {
						std::string v_replayName = Replay::giveAutomaticName();
						_SaveReplay(v_replayName);
						m_Renderer->showMsgNewBestHighscore(v_replayName);
					} else {
						m_Renderer->showMsgNewBestHighscore();
					} /* ok i officially give up on indention in x-moto :P */
				} else {
					if(v_is_a_personal_highscore) { /* personal highscore */
						try {
							Sound::playSampleByName(m_theme.getSound("NewHighscore")->FilePath());
						} catch(Exception &e) {
						}
						if(m_pJustPlayReplay != NULL && m_bAutosaveHighscoreReplays) {
							std::string v_replayName = Replay::giveAutomaticName();
							_SaveReplay(v_replayName);
							m_Renderer->showMsgNewPersonalHighscore(v_replayName);
						} else {
							m_Renderer->showMsgNewPersonalHighscore();
						}
	      
					} else { /* no highscore */
						m_Renderer->hideMsgNewHighscore();
					}
				}
			}

			/* update profiles */
			float v_finish_time = 0.0;
			std::string TimeStamp = getTimeStamp();
			for(unsigned int i=0; i<m_MotoGame.Players().size(); i++) {
				if(m_MotoGame.Players()[i]->isFinished()) {
					v_finish_time  = m_MotoGame.Players()[i]->finishTime();
				}
			}
			if(m_MotoGame.Players().size() == 1) {
				m_db->profiles_addFinishTime(m_xmsession->profile(), m_MotoGame.getLevelSrc()->Id(),
																		 TimeStamp, v_finish_time);
			}
			_MakeBestTimesWindow(m_pBestTimes, m_xmsession->profile(), m_MotoGame.getLevelSrc()->Id(),
													 v_finish_time,TimeStamp);

			/* Update stats */
			/* update stats only in one player mode */
			if(m_MotoGame.Players().size() == 1) {       
				m_db->stats_levelCompleted(m_xmsession->profile(),
																	 m_MotoGame.getLevelSrc()->Id(),
																	 m_MotoGame.Players()[0]->finishTime());
				_UpdateLevelsLists();
				_UpdateCurrentPackList(m_MotoGame.getLevelSrc()->Id(),
															 m_MotoGame.Players()[0]->finishTime());
			}
			break;
		}
    }
        
    m_fLastPhysTime = getXMTime() - PHYS_STEP_SIZE;

    /* manage music */
    if(m_bEnableMenuMusic && Sound::isEnabled()) {
      if(v_newMusicPlaying != m_playingMusic) {
				try {
					if(v_newMusicPlaying == "") {
						m_playingMusic = v_newMusicPlaying;
						Sound::stopMusic();
					} else {
						m_playingMusic = v_newMusicPlaying;
						Sound::playMusic(m_theme.getMusic(v_newMusicPlaying)->FilePath());
					}
				} catch(Exception &e) {
					Logger::Log("** Warning ** : PlayMusic(%s) failed", v_newMusicPlaying.c_str());
					Sound::stopMusic();
				}
      }
    }
  }

  std::string GameApp::getConfigThemeName(ThemeChoicer *p_themeChoicer) {
    std::string v_currentThemeName = m_Config.getString("Theme");

    if(m_db->themes_exists(v_currentThemeName)) {
      return v_currentThemeName;
    }
    /* theme of the config file doesn't exist */
    return THEME_DEFAULT_THEMENAME;
  }

  /*===========================================================================
  Update settings
  ===========================================================================*/
  void GameApp::_UpdateSettings(void) {
    /* Menu graphics */
    std::string s = m_Config.getString("MenuBackgroundGraphics");
    if(s == "Medium") m_MenuBackgroundGraphics = MENU_GFX_LOW;
    else if(s == "High") m_MenuBackgroundGraphics = MENU_GFX_HIGH;
    else m_MenuBackgroundGraphics = MENU_GFX_OFF;
    
    /* Game graphics */
    s = m_Config.getString("GameGraphics");

    if(m_xmsession->useGraphics()) {
      if(s == "Low") m_Renderer->setQuality(GQ_LOW);
      else if(s == "Medium") m_Renderer->setQuality(GQ_MEDIUM);
      else if(s == "High") m_Renderer->setQuality(GQ_HIGH);
    }      

    /* Show mini map? && show engine counter */
    if(m_xmsession->useGraphics()) {
      m_Renderer->setShowMinimap(m_Config.getBool("ShowMiniMap"));
      m_Renderer->setShowEngineCounter(m_Config.getBool("ShowEngineCounter"));
    }    

    /* Replay stuff */
    m_fReplayFrameRate = m_Config.getFloat("ReplayFrameRate");
    m_bRecordReplays = m_Config.getBool("StoreReplays");
    m_bCompressReplays = m_Config.getBool("CompressReplays");
    Replay::enableCompression(m_bCompressReplays);
    m_bAutosaveHighscoreReplays = m_Config.getBool("AutosaveHighscoreReplays");

    /* ghost */
    m_bEnableGhost        = m_Config.getBool("EnableGhost");
    m_bShowGhostTimeDiff  = m_Config.getBool("ShowGhostTimeDiff");
    m_MotoGame.setShowGhostTimeDiff(m_bShowGhostTimeDiff);
    m_bGhostMotionBlur = m_Config.getBool("GhostMotionBlur");
    if(m_xmsession->useGraphics()) {
      m_Renderer->setGhostMotionBlur( m_bGhostMotionBlur );
    }

    m_bEnableGhostInfo = m_Config.getBool("DisplayGhostInfo");
    m_bHideGhosts = m_Config.getBool("HideGhosts");
    if(m_xmsession->useGraphics()) {
      m_Renderer->setGhostDisplayInformation(m_bEnableGhostInfo);
      m_Renderer->setHideGhosts(m_bHideGhosts);
    }

    m_bShowWebHighscoreInGame = m_Config.getBool("ShowInGameWorldRecord");
    m_bEnableCheckNewLevelsAtStartup  = m_Config.getBool("CheckNewLevelsAtStartup");
    m_bEnableCheckHighscoresAtStartup = m_Config.getBool("CheckHighscoresAtStartup");

    /* Other settings */
    m_bEnableEngineSound = m_Config.getBool("EngineSoundEnable");
    m_bEnableContextHelp = m_Config.getBool("ContextHelp");
    m_bEnableMenuMusic = m_Config.getBool("MenuMusic");
    m_bEnableInitZoom = m_Config.getBool("InitZoom");
    m_bEnableDeathAnim = m_Config.getBool("DeathAnim");

    /* multi */
    m_bMultiStopWhenOneFinishes = m_Config.getBool("MultiStopWhenOneFinishes");

    /* www */
    m_WebHighscoresURL    = m_Config.getString("WebHighscoresURL");
    m_WebHighscoresIdRoom = m_Config.getString("WebHighscoresIdRoom");

    /* Configure proxy */
    _ConfigureProxy();
  }
  
  /*===========================================================================
  Draw menu/title screen background
  ===========================================================================*/
  void GameApp::_DrawMenuBackground(void) {
    if(m_MenuBackgroundGraphics != MENU_GFX_OFF && m_xmsession->ugly() == false) {
      if(m_pTitleTL != NULL)
        drawLib->drawImage(Vector2f(0,0),Vector2f(drawLib->getDispWidth()/2,drawLib->getDispHeight()/2),m_pTitleTL);
      if(m_pTitleTR != NULL)
        drawLib->drawImage(Vector2f(drawLib->getDispWidth()/2,0),Vector2f(drawLib->getDispWidth(),drawLib->getDispHeight()/2),m_pTitleTR);
      if(m_pTitleBR != NULL)
        drawLib->drawImage(Vector2f(drawLib->getDispWidth()/2,drawLib->getDispHeight()/2),Vector2f(drawLib->getDispWidth(),drawLib->getDispHeight()),m_pTitleBR);
      if(m_pTitleBL != NULL)
        drawLib->drawImage(Vector2f(0,drawLib->getDispHeight()/2),Vector2f(drawLib->getDispWidth()/2,drawLib->getDispHeight()),m_pTitleBL);
    } else if(m_MenuBackgroundGraphics == MENU_GFX_OFF){
        //in Ugly mode the screen is cleared in the VApp main loop
	//this is not the case when ugly mode is off.
	//and when MENU_GFX_OFF we need to clear the screen
        drawLib->clearGraphics();
    }
  }

  /*===========================================================================
  GUI mouse hover
  ===========================================================================*/
  void GameApp::_DispatchMouseHover(void) {
    int nX,nY;
    getMousePos(&nX,&nY);
    m_Renderer->getGUI()->mouseHover(nX,nY);
  }
    
  /*===========================================================================
  Screenshooting
  ===========================================================================*/
  void GameApp::_GameScreenshot(void) {
    Img *pShot = getDrawLib()->grabScreen();      
    FileHandle *pfh;

    std::string v_ShotsDir;
    std::string v_ShotExtension;
    std::string v_destFile;
    int nShot=0;
    char v_val[5];

    v_ShotsDir = FS::getUserDir() + std::string("/Screenshots");
    FS::mkArborescenceDir(v_ShotsDir);
    v_ShotExtension = m_Config.getString("ScreenshotFormat");
    
    /* User preference for format? must be either jpeg or png */
    if(v_ShotExtension != "jpeg" && v_ShotExtension != "jpg" && v_ShotExtension != "png") {
      Logger::Log("** Warning ** : unsupported screenshot format '%s', using png instead!", v_ShotExtension.c_str());
      v_ShotExtension = "png";
    }    

    do {
      nShot++;
      if(nShot > 9999) {
	Logger::Log("Too many screenshots !");
	delete pShot;
	return;
      }

      snprintf(v_val, 5, "%04d", nShot);
      v_destFile = v_ShotsDir + "/screenshot" + std::string(v_val) + "." + v_ShotExtension;
    } while(FS::fileExists(v_destFile));
    
    try {
      pShot->saveFile(v_destFile.c_str());
    } catch(Exception &e) {
      Logger::Log(std::string("Unable to save the screenshot: " + e.getMsg()).c_str());
    }

    delete pShot;
  }

  void GameApp::enableFps(bool bValue) {
    m_xmsession->setFps(m_xmsession->fps() == false);
    if(m_xmsession->fps()) {
      m_sysMsg->displayText(SYS_MSG_FPS_ENABLED);
    } else {
      m_sysMsg->displayText(SYS_MSG_FPS_DISABLED);
    }
  }

  void GameApp::enableWWW(bool bValue) {
    m_xmsession->setWWW(m_xmsession->www() == false);
    if(m_xmsession->www()) {
      m_sysMsg->displayText(SYS_MSG_WWW_ENABLED);
    } else {
      m_sysMsg->displayText(SYS_MSG_WWW_DISABLED);
    }
  }

  /*===========================================================================
  Key down event
  ===========================================================================*/
  void GameApp::keyDown(int nKey, SDLMod mod, int nChar) {
    /* No matter what, F12 always equals a screenshot */
    if(nKey == SDLK_F12) {
      _GameScreenshot();
      return;        
    }

    if(nKey == SDLK_F8) {
      enableWWW(m_xmsession->www() == false);
      return;        
    }

    if(nKey == SDLK_F7) {
      enableFps(m_xmsession->fps() == false);
      return;        
    }

    if(nKey == SDLK_F9) {
      switchUglyMode(m_xmsession->ugly() == false);
      if(m_xmsession->ugly()) {
	m_sysMsg->displayText(SYS_MSG_UGLY_MODE_ENABLED);
      } else {
	m_sysMsg->displayText(SYS_MSG_UGLY_MODE_DISABLED);
      }
      return;        
    }

    if(nKey == SDLK_RETURN && (((mod & KMOD_LALT) == KMOD_LALT) || ((mod & KMOD_RALT) == KMOD_RALT))) {
      drawLib->toogleFullscreen();
      return;
    }

    if(nKey == SDLK_F10) {
      switchTestThemeMode(m_xmsession->testTheme() == false);
      if(m_xmsession->testTheme()) {
	m_sysMsg->displayText(SYS_MSG_THEME_MODE_ENABLED);
      } else {
	m_sysMsg->displayText(SYS_MSG_THEME_MODE_DISABLED);
      }
      return;        
    }

    if(nKey == SDLK_F11) {
      switchUglyOverMode(m_xmsession->uglyOver() == false);
      if(m_xmsession->uglyOver()) {
	m_sysMsg->displayText(SYS_MSG_UGLY_OVER_MODE_ENABLED);
      } else {
	m_sysMsg->displayText(SYS_MSG_UGLY_OVER_MODE_DISABLED);
      }
      return;        
    }

    /* activate/desactivate interpolation */
    if(nKey == SDLK_i && ( (mod & KMOD_LCTRL) || (mod & KMOD_RCTRL) )) {
      m_allowReplayInterpolation = !m_allowReplayInterpolation;
      if(m_allowReplayInterpolation) {
	m_sysMsg->displayText(SYS_MSG_INTERPOLATION_ENABLED);
      } else {
	m_sysMsg->displayText(SYS_MSG_INTERPOLATION_DISABLED);
      }

      for(unsigned int i=0; i<m_MotoGame.Players().size(); i++) {
	m_MotoGame.Players()[i]->setInterpolation(m_allowReplayInterpolation);
      }

      return;
    }

    if(nKey == SDLK_m && ( (mod & KMOD_LCTRL) || (mod & KMOD_RCTRL) )) {
      for(unsigned int i=0; i<m_MotoGame.Cameras().size(); i++) {
	m_MotoGame.Cameras()[i]->setMirrored(m_MotoGame.Cameras()[i]->isMirrored() == false);
      }
      m_InputHandler.setMirrored(m_MotoGame.Cameras()[0]->isMirrored());
    }

    if(m_State == GS_MENU) {

      if(nKey == SDLK_F5) {
	_SimpleMessage(GAMETEXT_RELOADINGLEVELS, &m_InfoMsgBoxRect);
	m_reloadingLevelsUser = true;
	m_levelsManager.reloadLevelsFromLvl(m_db, this);
	m_pActiveLevelPack = NULL;
	_UpdateLevelsLists();
	_SimpleMessage(GAMETEXT_RELOADINGREPLAYS, &m_InfoMsgBoxRect);
	initReplaysFromDir();
	_UpdateReplaysList();
	m_themeChoicer->initThemesFromDir(m_db);
	_UpdateThemesLists();
      }

    }
    
    /* If message box... */
    if(m_pQuitMsgBox) {
      if(nKey == SDLK_ESCAPE) {
        delete m_pQuitMsgBox;
        m_pQuitMsgBox = NULL;
      }    
      else
        m_Renderer->getGUI()->keyDown(nKey, mod, nChar);      
      return;
    }
    else if(m_pNotifyMsgBox) {
      if(nKey == SDLK_ESCAPE) {
        delete m_pNotifyMsgBox;
        m_pNotifyMsgBox = NULL;
      }    
      else
        m_Renderer->getGUI()->keyDown(nKey, mod, nChar);      
      return;
    }
  
    /* What state? */
    switch(m_State) {
      case GS_EDIT_PROFILES:
      case GS_EDIT_WEBCONFIG:
      case GS_LEVEL_INFO_VIEWER:
      case GS_LEVELPACK_VIEWER:
      case GS_MENU: {
        /* The GUI wants to know about keypresses... */
        m_Renderer->getGUI()->keyDown(nKey, mod,nChar);
        break;
      }
      case GS_PAUSE:
        switch(nKey) {
          case SDLK_ESCAPE:
            /* Back to the game, please */
	  m_MotoGame.setInfos("");
	  m_pPauseMenu->showWindow(false);
	  m_State = GS_PLAYING;
	  break;
	case SDLK_F3:
	  switchLevelToFavorite(m_MotoGame.getLevelSrc()->Id(), true);
	  break;
          default:
            m_Renderer->getGUI()->keyDown(nKey, mod,nChar);
            break;      
        }
        break;
      case GS_DEADJUST:
      {
  switch(nKey) {
  case SDLK_RETURN:
    m_MotoGame.clearGameMessages();
    _RestartLevel();
    break;
  case SDLK_ESCAPE:
    m_MotoGame.clearGameMessages();
    setState(GS_DEADMENU);
    break;
  }
  break;
      }
      case GS_FINISHED:
      case GS_DEADMENU:
        switch(nKey) {
          case SDLK_ESCAPE:
            if(m_pSaveReplayMsgBox == NULL) {          
              /* Out of this game, please */
              m_pFinishMenu->showWindow(false);
        m_Renderer->hideMsgNewHighscore();
              m_pBestTimes->showWindow(false);
              m_pJustDeadMenu->showWindow(false);
							m_MotoGame.resetFollow();
              m_MotoGame.endLevel();
              m_Renderer->unprepareForNewLevel();
              //setState(GS_MENU);
              setState(m_StateAfterPlaying);
            }
            else {
              if(m_State == GS_DEADMENU)
                if(getXMTime() < m_fCoolDownEnd)
                  break;
               
              m_Renderer->getGUI()->keyDown(nKey, mod,nChar);
            }
            break;
          default:
            if(m_State == GS_DEADMENU)
              if(getXMTime() < m_fCoolDownEnd)
                break;
             
            m_Renderer->getGUI()->keyDown(nKey, mod,nChar);
            break;      
        }
        break;
      case GS_REPLAYING:
        switch(nKey) {
          case SDLK_ESCAPE:
            /* Escape quits the replay */
						m_MotoGame.resetFollow();
            m_MotoGame.endLevel();
            m_Renderer->unprepareForNewLevel();
	    setState(m_StateAfterPlaying);
            break;          
          case SDLK_RIGHT:
            /* Right arrow key: fast forward */
	    if(m_stopToUpdateReplay == false) {
	      m_MotoGame.fastforward(1);
	    }
            break;
          case SDLK_LEFT:
	    if(m_MotoGame.getLevelSrc()->isScripted() == false) {
	      m_MotoGame.fastrewind(1);
	      m_stopToUpdateReplay = false;
	    }
            break;
	case SDLK_F2:
	  m_Renderer->switchFollow();
	  break;

	case SDLK_F3:
	  switchLevelToFavorite(m_MotoGame.getLevelSrc()->Id(), true);
	  break;

  case SDLK_SPACE:
    /* pause */
    m_MotoGame.pause();

    m_Renderer->showReplayHelp(m_MotoGame.getSpeed(),
			      m_MotoGame.getLevelSrc()->isScripted() == false
			      ); /* update help */
    break;
  case SDLK_UP:
    /* faster */
    m_MotoGame.faster();

    m_Renderer->showReplayHelp(m_MotoGame.getSpeed(),
			      m_MotoGame.getLevelSrc()->isScripted() == false
			      ); /* update help */
    break;
  case SDLK_DOWN:
    /* slower */
    m_MotoGame.slower();
    m_stopToUpdateReplay = false;
    
    m_Renderer->showReplayHelp(m_MotoGame.getSpeed(),
			      m_MotoGame.getLevelSrc()->isScripted() == false
			      ); /* update help */
    break;
        }
      break;
      case GS_PREPLAYING:
      /* any key to remove the animation */
      //switch(nKey) {
      //case SDLK_ESCAPE:
      //case SDLK_RETURN:
  m_bPrePlayAnim = false;
      //break;
      //}
      break;
      case GS_PLAYING:
        switch(nKey) {
  case SDLK_ESCAPE:
		if(isLockedMotoGame() == false) {
			/* Escape pauses */
			setState(GS_PAUSE);
			m_pPauseMenu->showWindow(true);
			m_nPauseShade = 0;
		}
		break;
	case SDLK_F2:
	  m_Renderer->switchFollow();
	  break;
	case SDLK_F3:
	  switchLevelToFavorite(m_MotoGame.getLevelSrc()->Id(), true);
	  break;
	case SDLK_PAGEUP:
	  if(_IsThereANextLevel(m_PlaySpecificLevelId)) {
	    m_db->stats_abortedLevel(m_xmsession->profile(), m_MotoGame.getLevelSrc()->Id(), m_MotoGame.getTime());
	    m_MotoGame.endLevel();
	    m_Renderer->unprepareForNewLevel();
	    m_PlaySpecificLevelId = _DetermineNextLevel(m_PlaySpecificLevelId);
	    m_bPrePlayAnim = true;
	    setState(GS_PREPLAYING);
	  }
	  break;
	case SDLK_PAGEDOWN:
	  if(_IsThereAPreviousLevel(m_PlaySpecificLevelId)) {
	    m_db-> stats_abortedLevel(m_xmsession->profile(), m_MotoGame.getLevelSrc()->Id(), m_MotoGame.getTime());
	    m_MotoGame.endLevel();
	    m_Renderer->unprepareForNewLevel();
	    m_PlaySpecificLevelId = _DeterminePreviousLevel(m_PlaySpecificLevelId);
	    m_bPrePlayAnim = true;
	    setState(GS_PREPLAYING);
	  }
	  break;
  case SDLK_RETURN:
    /* retart immediatly the level */
    _RestartLevel();
    break;
  case SDLK_F5:
    _RestartLevel(true);
    break;
          default:
            /* Notify the controller */
	    m_InputHandler.handleInput(INPUT_KEY_DOWN,nKey,mod,
				       m_MotoGame.Players(),
				       m_MotoGame.Cameras(),
				       this);
        }
      break; 
    }
  }

  /*===========================================================================
  Key up event
  ===========================================================================*/
  void GameApp::keyUp(int nKey, SDLMod mod) {
    /* What state? */
    switch(m_State) {
      case GS_EDIT_WEBCONFIG:
      case GS_EDIT_PROFILES:
      case GS_LEVEL_INFO_VIEWER:
      case GS_FINISHED:
      case GS_DEADMENU:
      case GS_LEVELPACK_VIEWER:
      case GS_MENU:
        m_Renderer->getGUI()->keyUp(nKey, mod);
        break;
      case GS_PLAYING:
        /* Notify the controller */
      m_InputHandler.handleInput(INPUT_KEY_UP,nKey,mod,
				 m_MotoGame.Players(),
				 m_MotoGame.Cameras(),
				 this);
      break; 
      case GS_DEADJUST:
      {
  break;
      }
    }
  }

  /*===========================================================================
  Mouse events
  ===========================================================================*/
  void GameApp::mouseDoubleClick(int nButton) {
    switch(m_State) {
      case GS_MENU:
      case GS_PAUSE:
      case GS_DEADMENU:
      case GS_FINISHED:
      case GS_EDIT_PROFILES:
      case GS_EDIT_WEBCONFIG:
      case GS_LEVEL_INFO_VIEWER:
      case GS_LEVELPACK_VIEWER:
        int nX,nY;        
        getMousePos(&nX,&nY);
        
        if(nButton == SDL_BUTTON_LEFT)
          m_Renderer->getGUI()->mouseLDoubleClick(nX,nY);
        
        break;
      case GS_DEADJUST:
      break;
    }
  }

  void GameApp::mouseDown(int nButton) {
    switch(m_State) {
      case GS_MENU:
      case GS_PAUSE:
      case GS_DEADMENU:
      case GS_FINISHED:
      case GS_EDIT_PROFILES:
      case GS_EDIT_WEBCONFIG:
      case GS_LEVEL_INFO_VIEWER:
      case GS_LEVELPACK_VIEWER:
        int nX,nY;        
        getMousePos(&nX,&nY);
        
        if(nButton == SDL_BUTTON_LEFT)
          m_Renderer->getGUI()->mouseLDown(nX,nY);
        else if(nButton == SDL_BUTTON_RIGHT)
          m_Renderer->getGUI()->mouseRDown(nX,nY);
        else if(nButton == SDL_BUTTON_WHEELUP)
          m_Renderer->getGUI()->mouseWheelUp(nX,nY);
        else if(nButton == SDL_BUTTON_WHEELDOWN)        
          m_Renderer->getGUI()->mouseWheelDown(nX,nY);
        
        break;

      case GS_PLAYING:
      /* Notify the controller */
      m_InputHandler.handleInput(INPUT_KEY_DOWN,nButton,KMOD_NONE,
				 m_MotoGame.Players(),
				 m_MotoGame.Cameras(),
				 this);

      break;
      case GS_DEADJUST:
      break;
    }
  }

  void GameApp::mouseUp(int nButton) {
    switch(m_State) {
      case GS_MENU:
      case GS_PAUSE:
      case GS_DEADMENU:
      case GS_FINISHED:
      case GS_EDIT_PROFILES:
      case GS_EDIT_WEBCONFIG:
      case GS_LEVEL_INFO_VIEWER:
      case GS_LEVELPACK_VIEWER:
        int nX,nY;
        getMousePos(&nX,&nY);
        
        if(nButton == SDL_BUTTON_LEFT)
          m_Renderer->getGUI()->mouseLUp(nX,nY);
        else if(nButton == SDL_BUTTON_RIGHT)
          m_Renderer->getGUI()->mouseRUp(nX,nY);
        break;

      case GS_PLAYING:
        /* Notify the controller */
      m_InputHandler.handleInput(INPUT_KEY_UP,nButton,KMOD_NONE,
				 m_MotoGame.Players(),
				 m_MotoGame.Cameras(),
				 this);
        break;
      case GS_DEADJUST:
      break;
    }
  }
      
  /*===========================================================================
  Notification popup
  ===========================================================================*/
  void GameApp::notifyMsg(std::string Msg) {
    if(m_pNotifyMsgBox != NULL) delete m_pNotifyMsgBox;
    m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
    m_pNotifyMsgBox = m_Renderer->getGUI()->msgBox(Msg,(UIMsgBoxButton)(UI_MSGBOX_OK));
  }
  
  /*===========================================================================
  Save a replay
  ===========================================================================*/
  void GameApp::_SaveReplay(const std::string &Name) {
    /* This is simply a job of copying the Replays/Latest.rpl file into 
       Replays/Name.rpl */
    std::string RealName = Name;
    
    /* Strip illegal characters from name */
    int i=0;
    while(1) {
      if(i >= RealName.length()) break;
      
      if((RealName[i] >= 'a' && RealName[i] <= 'z') ||
         (RealName[i] >= 'A' && RealName[i] <= 'Z') ||
         (RealName[i] >= '0' && RealName[i] <= '9') ||
         RealName[i]=='!' || RealName[i]=='@' || RealName[i]=='#' || RealName[i]=='&' ||
         RealName[i]=='(' || RealName[i]==')' || RealName[i]=='-' || RealName[i]=='_' ||
         RealName[i]==' ' || RealName[i]=='.' || RealName[i]==',' || RealName[i]=='*') {
        /* This is ok */
        i++;
      }
      else {
        /* Not ok */
        RealName.erase(RealName.begin() + i);
      }            
    }

    /* Try saving */
    std::string v_outputfile;
    if(!FS::copyFile("Replays/Latest.rpl",
         std::string("Replays/") + RealName + std::string(".rpl"),
         v_outputfile)) {
      Logger::Log("** Warning ** : Failed to save replay: %s",Name.c_str());
      notifyMsg(GAMETEXT_FAILEDTOSAVEREPLAY);
    } else {
      /* Update replay list to reflect changes */
      addReplay(v_outputfile);
      _UpdateReplaysList();
    }
  }

  std::string GameApp::_DetermineNextLevel(const std::string& i_id_level) {
    if(m_currentPlayingList == NULL) {
      return "";
    }

    for(int i=0;i<m_currentPlayingList->getEntries().size()-1;i++) {
      if((*((std::string*)m_currentPlayingList->getEntries()[i]->pvUser)) == i_id_level) {
	return *((std::string*)m_currentPlayingList->getEntries()[i+1]->pvUser);
      }
    }
    return *((std::string*)m_currentPlayingList->getEntries()[0]->pvUser);
  }
  
  bool GameApp::_IsThereANextLevel(const std::string& i_id_level) {
    return _DetermineNextLevel(i_id_level) != "";
  }

  std::string GameApp::_DeterminePreviousLevel(const std::string& i_id_level) {
    if(m_currentPlayingList == NULL) {
      return "";
    }

    for(int i=1;i<m_currentPlayingList->getEntries().size();i++) {
      if((*((std::string*)m_currentPlayingList->getEntries()[i]->pvUser)) == i_id_level) {
	return *((std::string*)m_currentPlayingList->getEntries()[i-1]->pvUser);
      }
    }
    return *((std::string*)m_currentPlayingList->getEntries()[m_currentPlayingList->getEntries().size()-1]->pvUser);
  }
  
  bool GameApp::_IsThereAPreviousLevel(const std::string& i_id_level) {
    return _DeterminePreviousLevel(i_id_level) != "";
  } 

  void GameApp::_UpdateWorldRecord(const std::string &LevelID) {  
    char **v_result;
    unsigned int nrow;
    std::string v_roomName;
    std::string v_id_profile;
    float       v_finishTime;

    m_Renderer->setWorldRecordTime("");

    /* don't update if the option is not set */
    if(m_bShowWebHighscoreInGame == false) {
      return;
    }

    v_result = m_db->readDB("SELECT a.name, b.id_profile, b.finishTime "
			    "FROM webrooms AS a LEFT OUTER JOIN webhighscores AS b "
			    "ON (a.id_room = b.id_room "
			    "AND b.id_level=\"" + xmDatabase::protectString(LevelID) + "\") "
			    "WHERE a.id_room=" + m_WebHighscoresIdRoom + ";",
			    nrow);
    if(nrow != 1) {
      /* should not happend */
      m_db->read_DB_free(v_result);
      m_Renderer->setWorldRecordTime(std::string("WR: ") + GAMETEXT_WORLDRECORDNA);
      return;
    }
    v_roomName = m_db->getResult(v_result, 3, 0, 0);
    if(m_db->getResult(v_result, 3, 0, 1) != NULL) {
      v_id_profile = m_db->getResult(v_result, 3, 0, 1);
      v_finishTime = atof(m_db->getResult(v_result, 3, 0, 2));
    }
    m_db->read_DB_free(v_result);
    
    if(v_id_profile != "") {
      m_Renderer->setWorldRecordTime(v_roomName + ": " + 
				    GameApp::formatTime(v_finishTime) +
				    std::string(" (") + v_id_profile + std::string(")"));
    } else {
      m_Renderer->setWorldRecordTime(v_roomName + ": " + 
				    GAMETEXT_WORLDRECORDNA
				    );
    }
  }

  void GameApp::_UpdateWebHighscores(bool bSilent) {
    if(!bSilent) {
      _SimpleMessage(GAMETEXT_DLHIGHSCORES,&m_InfoMsgBoxRect);
    }

    m_bWebHighscoresUpdatedThisSession = true;
    
    /* Try downloading the highscores */
    m_pWebHighscores->setWebsiteInfos(m_WebHighscoresIdRoom,
				      m_WebHighscoresURL);
    Logger::Log("WWW: Checking for new highscores...");
    m_pWebHighscores->update();
  }

  void GameApp::_UpdateWebLevels(bool bSilent, bool bEnableWeb) {
    if(!bSilent) {
      _SimpleMessage(GAMETEXT_DLLEVELSCHECK,&m_InfoMsgBoxRect);
    }

    /* Try download levels list */
    if(m_pWebLevels == NULL) {
      m_pWebLevels = new WebLevels(this,&m_ProxySettings);
    }
    m_pWebLevels->setURL(m_Config.getString("WebLevelsURL"));
    Logger::Log("WWW: Checking for new or updated levels...");

    m_pWebLevels->update(m_db);
    m_bWebLevelsToDownload = m_pWebLevels->nbLevelsToGet(m_db);
  }

  void GameApp::_UpdateWebThemes(bool bSilent) {
    if(!bSilent) {
      _SimpleMessage(GAMETEXT_DLTHEMESLISTCHECK,&m_InfoMsgBoxRect);
    }  

    m_themeChoicer->setURL(m_Config.getString("WebThemesURL"));

    Logger::Log("WWW: Checking for new or updated themes...");

    try {
      m_DownloadingInformation = "";
      m_themeChoicer->updateFromWWW(m_db);
      _UpdateThemesLists();
    } catch(Exception &e) {
      /* file probably doesn't exist */
      Logger::Log("** Warning ** : Failed to analyse web-themes file");   
    }
  }    

  void GameApp::_UpdateWebRooms(bool bSilent) {
    if(!bSilent) {
      _SimpleMessage(GAMETEXT_DLROOMSLISTCHECK,&m_InfoMsgBoxRect);
    }  

    m_pWebRooms->setURL(m_Config.getString("WebRoomsURL"));

    Logger::Log("WWW: Checking for rooms list...");

    try {
      m_pWebRooms->update();
    } catch(Exception &e) {
      /* file probably doesn't exist */
      Logger::Log("** Warning ** : Failed to analyse update webrooms list");    
    }
  }

  void GameApp::_UpdateWebTheme(const std::string& i_id_theme, bool bNotify) {
    char **v_result;
    unsigned int nrow;
    std::string v_id_theme;
    std::string v_ck1, v_ck2;
    bool v_onDisk = false;
    bool v_onWeb  = true;

    v_result = m_db->readDB("SELECT a.id_theme, a.checkSum, b.checkSum "
    			    "FROM themes AS a LEFT OUTER JOIN webthemes AS b "
    			    "ON a.id_theme=b.id_theme "
			    "WHERE a.id_theme=\"" + xmDatabase::protectString(i_id_theme) + "\";",
    			    nrow);
    if(nrow == 1) {
      v_onDisk   = true;
      v_id_theme = m_db->getResult(v_result, 3, 0, 0);
      v_ck1      = m_db->getResult(v_result, 3, 0, 1);
      if(m_db->getResult(v_result, 3, 0, 2) == NULL) {
	v_onWeb = false;
      } else {
	v_ck2      = m_db->getResult(v_result, 3, 0, 2);
      }
    }
    m_db->read_DB_free(v_result);

    if(v_onWeb == false) { /* available on the disk, not on the web */
      if(bNotify) {
	notifyMsg(GAMETEXT_UNUPDATABLETHEMEONWEB);
      }
      return;
    }


    m_DownloadingInformation = "";
    m_DownloadingMessage = std::string(GAMETEXT_DLTHEME) + "\n\n ";
    try {
      Logger::Log("WWW: Downloading a theme...");
      clearCancelAsSoonAsPossible();
      m_themeChoicer->updateThemeFromWWW(m_db, i_id_theme);
      _UpdateThemesLists();
      reloadTheme(); /* reload the theme */
      if(bNotify) {
	notifyMsg(GAMETEXT_THEMEUPTODATE);
      }
    } catch(Exception &e) {
      /* file probably doesn't exist */
      Logger::Log("** Warning ** : Failed to update theme ", i_id_theme.c_str());
      if(bNotify) {
	notifyMsg(GAMETEXT_FAILEDGETSELECTEDTHEME + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
      }
      return;
    }
  }

  void GameApp::_UpgradeWebHighscores() {
    /* Upgrade high scores */
    try {
      m_pWebHighscores->upgrade(m_db);
    } catch(Exception &e) {
      /* file probably doesn't exist */
      Logger::Log("** Warning ** : Failed to analyse web-highscores file");   
    }
  }

  void GameApp::_UpgradeWebRooms(bool bUpdateMenus) {
    /* Upgrade high scores */
    try {
      m_pWebRooms->upgrade(m_db);
      if(bUpdateMenus) {
	_UpdateRoomsLists();
      }
    } catch(Exception &e) {
      /* file probably doesn't exist */
      Logger::Log("** Warning ** : Failed to analyse webrooms file");   
    }
  }

  /*===========================================================================
  Extra WWW levels
  ===========================================================================*/
  void GameApp::_DownloadExtraLevels(void) {
      /* Download extra levels */
      m_DownloadingInformation = "";
      m_DownloadingMessage = std::string(GAMETEXT_DLLEVELS) + "\n\n ";

      if(m_pWebLevels != NULL) {
        try {                  
          Logger::Log("WWW: Downloading levels...");
          clearCancelAsSoonAsPossible();
          m_pWebLevels->upgrade(m_db);
	  m_bWebLevelsToDownload = false;
        } 
        catch(Exception &e) {
          Logger::Log("** Warning ** : Unable to download extra levels [%s]",e.getMsg().c_str());
  
          if(m_pInfoMsgBox != NULL) {
            delete m_pInfoMsgBox;
            m_pInfoMsgBox = NULL;
          }
          notifyMsg(GAMETEXT_FAILEDDLLEVELS + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
        }

        /* Got some new levels... load them! */
        Logger::Log("Loading new and updated levels...");
	m_pActiveLevelPack = NULL; /* the active level pack could no more exists after update */
	m_levelsManager.updateLevelsFromLvl(m_db,
					    m_pWebLevels->getNewDownloadedLevels(),
					    m_pWebLevels->getUpdatedDownloadedLevels()
					    );

         /* Update level lists */
	_UpdateLevelsLists();
      }
  }

  void GameApp::_CheckForExtraLevels(void) {
      /* Check for extra levels */
      try {
        _SimpleMessage(GAMETEXT_CHECKINGFORLEVELS);
      
        if(m_pWebLevels == NULL) {
    m_pWebLevels = new WebLevels(this,&m_ProxySettings);
  }
        m_pWebLevels->setURL(m_Config.getString("WebLevelsURL"));
        
        Logger::Log("WWW: Checking for new or updated levels...");
        clearCancelAsSoonAsPossible();

        m_pWebLevels->update(m_db);
        int nULevels=0;
	nULevels = m_pWebLevels->nbLevelsToGet(m_db);
	m_bWebLevelsToDownload = nULevels!=0;

        Logger::Log("WWW: %d new or updated level%s found",nULevels,nULevels==1?"":"s");

        if(nULevels == 0) {
          notifyMsg(GAMETEXT_NONEWLEVELS);
        }        
        else {
          /* Ask user whether he want to download levels or snot */
          if(m_pInfoMsgBox == NULL) {
            char cBuf[256];
	    snprintf(cBuf, 256, GAMETEXT_NEWLEVELAVAIL(nULevels), nULevels);
	    m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
            m_pInfoMsgBox = m_Renderer->getGUI()->msgBox(cBuf, (UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
          }
        }
      } 
      catch(Exception &e) {
        Logger::Log("** Warning ** : Unable to check for extra levels [%s]",e.getMsg().c_str());
        if(m_pInfoMsgBox != NULL) {
          delete m_pInfoMsgBox;
          m_pInfoMsgBox = NULL;
        }
        notifyMsg(GAMETEXT_FAILEDCHECKLEVELS + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
      } 
  }

  void GameApp::_RestartLevel(bool i_reloadLevel) {
		lockMotoGame(false);

    /* Update stats */        
    if(m_MotoGame.Players().size() == 1) {
      if(m_MotoGame.Players()[0]->isDead() == false) {
				m_db->stats_levelRestarted(m_xmsession->profile(),
																	 m_MotoGame.getLevelSrc()->Id(),
																	 m_MotoGame.getTime());
      }
    }  

		m_MotoGame.resetFollow();
		m_MotoGame.endLevel();

    m_Renderer->unprepareForNewLevel();

    if(i_reloadLevel) {
      try {
				Level::removeFromCache(m_db, m_PlaySpecificLevelId);
      } catch(Exception &e) {
				// hum, not nice
      }
    }

    setState(GS_PREPLAYING);   
  }

  /*===========================================================================
  WWWAppInterface implementation
  ===========================================================================*/
  bool GameApp::shouldLevelBeUpdated(const std::string &LevelID) {
    if(m_updateAutomaticallyLevels) {
      return true;
    }

    /* Hmm... ask user whether this level should be updated */
    bool bRet = true;
    bool bDialogBoxOpen = true;
    char cBuf[1024];
    char **v_result;
    unsigned int nrow;
    std::string v_levelName;
    std::string v_levelFileName;

    v_result = m_db->readDB("SELECT name, filepath "
			    "FROM levels "
			    "WHERE id_level=\"" + xmDatabase::protectString(LevelID) + "\";",
			    nrow);
    if(nrow != 1) {
      m_db->read_DB_free(v_result);
      return true;
    }

    v_levelName     = m_db->getResult(v_result, 2, 0, 0);
    v_levelFileName = m_db->getResult(v_result, 2, 0, 1);
    m_db->read_DB_free(v_result);

    sprintf(cBuf,(std::string(GAMETEXT_WANTTOUPDATELEVEL) + "\n(%s)").c_str(), v_levelName.c_str(),
	    v_levelFileName.c_str());
    m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
    UIMsgBox *pMsgBox = m_Renderer->getGUI()->msgBox(cBuf,(UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO|UI_MSGBOX_YES_FOR_ALL));

    while(bDialogBoxOpen) {
      SDL_PumpEvents();
      
      SDL_Event Event;
      while(SDL_PollEvent(&Event)) {
	/* What event? */
	switch(Event.type) {
	case SDL_QUIT:  
	  /* Force quit */
	  quit();
	  setCancelAsSoonAsPossible();
	  return false;
	case SDL_MOUSEBUTTONDOWN:
	  mouseDown(Event.button.button);
	  break;
	case SDL_MOUSEBUTTONUP:
	  mouseUp(Event.button.button);
	  break;
	}
      }
      
      UIMsgBoxButton Button = pMsgBox->getClicked();
      if(Button != UI_MSGBOX_NOTHING) {
	if(Button == UI_MSGBOX_NO) {
	  bRet = false;
	}
	if(Button == UI_MSGBOX_YES_FOR_ALL) {
	  m_updateAutomaticallyLevels = true;
	}
	bDialogBoxOpen = false;
      }
      
      _DrawMenuBackground();
      _DispatchMouseHover();
      
      m_Renderer->getGUI()->paint();
      
      UIRect TempRect;
      
      if(m_pCursor != NULL) {        
	int nMX,nMY;
	getMousePos(&nMX,&nMY);      
	drawLib->drawImage(Vector2f(nMX-2,nMY-2),Vector2f(nMX+30,nMY+30),m_pCursor);
      }
      
      drawLib->flushGraphics();
    }
    
    delete pMsgBox;
    setTaskProgress(m_fDownloadTaskProgressLast);
    
    return bRet;        
  }
        
  void GameApp::setTaskProgress(float fPercent) {
    int nBarHeight = 15;
    m_fDownloadTaskProgressLast = fPercent;
    readEvents();

    _DrawMenuBackground();
    _SimpleMessage(m_DownloadingMessage,&m_InfoMsgBoxRect,true);
    
    drawLib->drawBox(Vector2f(m_InfoMsgBoxRect.nX+10,m_InfoMsgBoxRect.nY+ m_InfoMsgBoxRect.nHeight-
                                                   nBarHeight*2),
            Vector2f(m_InfoMsgBoxRect.nX+m_InfoMsgBoxRect.nWidth-10,
                     m_InfoMsgBoxRect.nY+m_InfoMsgBoxRect.nHeight-nBarHeight),
            0,MAKE_COLOR(0,0,0,255),0);
            
                
    drawLib->drawBox(Vector2f(m_InfoMsgBoxRect.nX+10,m_InfoMsgBoxRect.nY+
                                                   m_InfoMsgBoxRect.nHeight-
                                                   nBarHeight*2),
            Vector2f(m_InfoMsgBoxRect.nX+10+((m_InfoMsgBoxRect.nWidth-20)*(int)fPercent)/100,
                     m_InfoMsgBoxRect.nY+m_InfoMsgBoxRect.nHeight-nBarHeight),
            0,MAKE_COLOR(255,0,0,255),0);

    FontManager* v_fm = drawLib->getFontSmall();
    FontGlyph* v_fg = v_fm->getGlyph(m_DownloadingInformation);
    v_fm->printString(v_fg,
		      m_InfoMsgBoxRect.nX+10,
		      m_InfoMsgBoxRect.nY+m_InfoMsgBoxRect.nHeight-nBarHeight*2,
		      MAKE_COLOR(255,255,255,128));
    drawLib->flushGraphics();
  }
  
  void GameApp::setBeingDownloadedInformation(const std::string &p_information,bool bNew) {
    m_DownloadingInformation = p_information;
  }
  
  void GameApp::readEvents(void) {
    /* Check for events */ 
    SDL_PumpEvents();
    
    SDL_Event Event;
    while(SDL_PollEvent(&Event)) {
      /* What event? */
      switch(Event.type) {
        case SDL_KEYDOWN: 
          if(Event.key.keysym.sym == SDLK_ESCAPE)
            setCancelAsSoonAsPossible();
          break;
        case SDL_QUIT:  
          /* Force quit */
          quit();
          setCancelAsSoonAsPossible();
          return;
      }
    }    
  }
  
  /*===========================================================================
  Configure proxy
  ===========================================================================*/
  void GameApp::_ConfigureProxy(void) {
    bool bFetchPortAndServer = false;
  
    /* Proxy? */        
    std::string s = m_Config.getString("ProxyType");
    if(s == "HTTP") {
      m_ProxySettings.setType(CURLPROXY_HTTP);
      bFetchPortAndServer = true;
    }
    else if(s == "SOCKS4") {
      m_ProxySettings.setType(CURLPROXY_SOCKS4);
      bFetchPortAndServer = true;
    }
    else if(s == "SOCKS5") {
      m_ProxySettings.setType(CURLPROXY_SOCKS5);
      bFetchPortAndServer = true;
    }
    else {
      m_ProxySettings.setDefaultAuthentification();
      m_ProxySettings.setDefaultPort();
      m_ProxySettings.setDefaultServer();
      m_ProxySettings.setDefaultType();
    }
    
    if(bFetchPortAndServer) {
      m_ProxySettings.setServer(m_Config.getString("ProxyServer"));
      m_ProxySettings.setPort(m_Config.getInteger("ProxyPort"));
      m_ProxySettings.setAuthentification(m_Config.getString("ProxyAuthUser"),
            m_Config.getString("ProxyAuthPwd"));      
    }
  }
  
  std::string GameApp::_getGhostReplayPath_bestOfThePlayer(std::string p_levelId, float &p_time) {
    char **v_result;
    unsigned int nrow;
    std::string res;

    p_time = -1.0;

    v_result = m_db->readDB("SELECT name, finishTime FROM replays "
			    "WHERE id_profile=\"" + xmDatabase::protectString(m_xmsession->profile()) + "\" "
			    "AND   id_level=\""   + xmDatabase::protectString(p_levelId) + "\" "
			    "AND   isFinished=1 "
			    "ORDER BY finishTime LIMIT 1;",
			    nrow);    
    if(nrow == 0) {
      m_db->read_DB_free(v_result);
      return "";
    }

    res = std::string("Replays/") + m_db->getResult(v_result, 2, 0, 0) + std::string(".rpl");
    p_time = atof(m_db->getResult(v_result, 2, 0, 1));

    m_db->read_DB_free(v_result);
    return res;
  }

  std::string GameApp::_getGhostReplayPath_bestOfTheRoom(std::string p_levelId, float &p_time) {
    char **v_result;
    unsigned int nrow;
    std::string res;
    std::string v_replayName;
    std::string v_fileUrl;

    v_result = m_db->readDB("SELECT fileUrl, finishTime FROM webhighscores "
			    "WHERE id_room=" + m_WebHighscoresIdRoom + " "
			    "AND id_level=\"" + xmDatabase::protectString(p_levelId) + "\";",
			    nrow);    
    if(nrow == 0) {
      p_time = -1.0;
      m_db->read_DB_free(v_result);
      return "";
    }

    v_fileUrl = m_db->getResult(v_result, 2, 0, 0);
    v_replayName = FS::getFileBaseName(v_fileUrl);
    p_time = atof(m_db->getResult(v_result, 2, 0, 1));
    m_db->read_DB_free(v_result);

    /* search if the replay is already downloaded */
    if(m_db->replays_exists(v_replayName)) {
      res = std::string("Replays/") + v_replayName + std::string(".rpl");
    } else {
      if(m_xmsession->www()) {
	/* download the replay */
	try {
	  _SimpleMessage(GAMETEXT_DLGHOST,&m_InfoMsgBoxRect);
	  m_pWebHighscores->downloadReplay(v_fileUrl);
	  addReplay(v_replayName);
	  _UpdateReplaysList();
	  res = std::string("Replays/") + v_replayName + std::string(".rpl");
	} catch(Exception &e) {
	  /* do nothing */
	  enableWWW(false);
	}
      }
    }
    return res;
  }

  std::string GameApp::_getGhostReplayPath_bestOfLocal(std::string p_levelId, float &p_time) {
    char **v_result;
    unsigned int nrow;
    std::string res;

    v_result = m_db->readDB("SELECT a.name, a.finishTime FROM replays AS a INNER JOIN stats_profiles AS b "
			    "ON a.id_profile = b.id_profile "
			    "WHERE a.id_level=\""   + xmDatabase::protectString(p_levelId) + "\" "
			    "AND   a.isFinished=1 "
			    "ORDER BY a.finishTime LIMIT 1;",
			    nrow);    
    if(nrow == 0) {
      m_db->read_DB_free(v_result);
      return "";
    }

    res = std::string("Replays/") + m_db->getResult(v_result, 2, 0, 0) + std::string(".rpl");
    p_time = atof(m_db->getResult(v_result, 2, 0, 1));
    m_db->read_DB_free(v_result);
    return res;
  }

  void GameApp::_UploadHighscore(std::string p_replayname, bool b_notify) {
    std::string v_msg;

    try {
      bool v_msg_status_ok;
      clearCancelAsSoonAsPossible();
      m_DownloadingInformation = "";
      m_DownloadingMessage = GAMETEXT_UPLOADING_HIGHSCORE;
      FSWeb::uploadReplay(FS::getUserDir() + "/Replays/" + p_replayname + ".rpl",
        m_WebHighscoresIdRoom,
        m_Config.getString("WebHighscoreUploadLogin"),
        m_Config.getString("WebHighscoreUploadPassword"),
        m_Config.getString("WebHighscoreUploadURL"),
        this,
        &m_ProxySettings,
        v_msg_status_ok,
        v_msg);
      if(v_msg_status_ok) {
	if(b_notify) {
	  notifyMsg(v_msg);
	}
      } else {
	if(b_notify) {
	  notifyMsg(std::string(GAMETEXT_UPLOAD_HIGHSCORE_WEB_WARNING_BEFORE) + "\n" + v_msg);
	}
      }
    } catch(Exception &e) {
      if(b_notify) {
	notifyMsg(GAMETEXT_UPLOAD_HIGHSCORE_ERROR + std::string("\n") + v_msg);
      } else {
	throw Exception(GAMETEXT_UPLOAD_HIGHSCORE_ERROR + std::string("\n") + v_msg);
      }
    }
  }

  TColor GameApp::getColorFromPlayerNumber(int i_player) {
    // try to find nice colors for first player, then automatic
    switch(i_player) {

      case 0:
      return TColor(255, 255, 255, 0);
      break;

      case 1:
      return TColor(125, 125, 125, 0);
      break;

      case 2:
      return TColor(200, 100, 50, 0);
      break;

      case 3:
      return TColor(50, 255, 255, 0);
      break;

      default:
      return TColor((i_player*5)%255, (i_player*20)%255, (i_player*50)%255, 0);
    }

    return TColor(255, 255, 255, 0);
  }

  void GameApp::_UploadAllHighscores() {
    /* 1 is the main room ; don't allow full upload on it */
    if(m_WebHighscoresIdRoom == "1") return;

    _UpdateWebHighscores(false);
    char **v_result;
    unsigned int nrow;
    std::string v_previousIdLevel, v_currentIdLevel;

	std::string query = "SELECT r.id_level, r.name FROM replays r "
    "LEFT OUTER JOIN webhighscores h "
    "ON (r.id_level = h.id_level AND h.id_room=" + m_WebHighscoresIdRoom + ") "
    "INNER JOIN weblevels l ON r.id_level = l.id_level "
    "WHERE r.id_profile=\"" + xmDatabase::protectString(m_xmsession->profile()) + "\" "
    "AND r.isFinished "
    "AND ( (h.id_room IS NULL) OR xm_floord(h.finishTime*100.0) > xm_floord(r.finishTime*100.0)) "
    "ORDER BY r.id_level, r.finishTime;";
    v_result = m_db->readDB(query, nrow);

    try {
      for (int i = 0; i<nrow; i++) {
	std::ostringstream v_percentage;
	v_percentage << std::setprecision (1);
	v_percentage << (i*100.0/nrow);

	v_currentIdLevel = m_db->getResult(v_result, 2, i, 0);
	
	/* send only the best of the replay by level */
	if(v_previousIdLevel != v_currentIdLevel) {
	  v_previousIdLevel = v_currentIdLevel;
	  _SimpleMessage(GAMETEXT_UPLOADING_HIGHSCORE + std::string("\n") + v_percentage.str() + "%");
	  _UploadHighscore(m_db->getResult(v_result, 2, i, 1), false);
	}
      }
    } catch(Exception &e) {
      notifyMsg(e.getMsg());
    }
    m_db->read_DB_free(v_result);
  }

  TColor GameApp::getUglyColorFromPlayerNumber(int i_player) {
    // try to find nice colors for first player, then automatic
    Color v_color;

    switch(i_player) {
      
      case 0:
      v_color = m_theme.getPlayerTheme()->getUglyRiderColor();
      return TColor(GET_RED(v_color), GET_GREEN(v_color), GET_BLUE(v_color));      
      break;
      
      case 1:
      return TColor(125, 125, 125, 0);
      break;
      
      case 2:
      return TColor(255, 50, 50);
      break;
      
      case 3:
      return TColor(50, 50, 255);
      break;

      default:
      return TColor((i_player*5)%255, (i_player*20)%255, (i_player*50)%255, 0);
    }
  }

  void GameApp::switchUglyMode(bool bUgly) {
    m_xmsession->setUgly(bUgly);
    if(bUgly == false) {
      SDL_ShowCursor(SDL_DISABLE);        
    } else {
      SDL_ShowCursor(SDL_ENABLE);
    }
    m_Renderer->setUglyMode(bUgly);
  }

  void GameApp::switchTestThemeMode(bool mode) {
    m_xmsession->setTestTheme(mode);
    m_Renderer->setTestThemeMode(mode);
  }

  void GameApp::switchUglyOverMode(bool mode) {
    m_xmsession->setUglyOver(mode);
    m_Renderer->setUglyOverMode(mode);
  }

  void GameApp::setPrePlayAnim(bool pEnabled) {
    m_bPrePlayAnim = pEnabled;
  }

  void GameApp::statePrestart_init() {
    char **v_result;
    unsigned int nrow;
    char *v_res;

    //        SDL_ShowCursor(SDL_DISABLE);
    m_bShowCursor = false;
      
    /* Initialize controls */
    m_InputHandler.configure(&m_Config);
      
    /* Default playing state */
    m_fLastFrameTime = 0.0f;
    m_fLastPerfStateTime = 0.0f;
      
    /* We need a profile */
    if(m_xmsession->profile() == "") {
      Logger::Log("** Warning ** : no player profile selected, use -profile option");
      throw Exception("no player");
    }
      
    /* Find the level */
    try {
     m_MotoGame.loadLevel(m_db, m_PlaySpecificLevelId);
    } catch(Exception &e) {
      Logger::Log("** Warning ** : level '%s' cannot be loaded",m_PlaySpecificLevelId.c_str());
      char cBuf[256];
      sprintf(cBuf,GAMETEXT_LEVELCANNOTBELOADED,m_PlaySpecificLevelId.c_str());
      setState(m_StateAfterPlaying);
      notifyMsg(cBuf);
      return;
    }

    if(m_MotoGame.getLevelSrc()->isXMotoTooOld()) {
      Logger::Log("** Warning ** : level '%s' requires newer X-Moto",
	  m_MotoGame.getLevelSrc()->Name().c_str());
  
      char cBuf[256];
      sprintf(cBuf,GAMETEXT_NEWERXMOTOREQUIRED,
	      m_MotoGame.getLevelSrc()->getRequiredVersion().c_str());
      m_MotoGame.endLevel();

      setState(m_StateAfterPlaying);
      notifyMsg(cBuf);     
      return;
    }

    /* Start playing right away */     

    if(m_pJustPlayReplay != NULL) delete m_pJustPlayReplay;
    m_pJustPlayReplay = NULL;
      
    if(m_bRecordReplays) {
      m_pJustPlayReplay = new Replay;
      m_pJustPlayReplay->createReplay("Latest.rpl",
				      m_MotoGame.getLevelSrc()->Id(),m_xmsession->profile(), m_fReplayFrameRate,sizeof(SerializedBikeState));
    }
      
      try {
	m_InputHandler.reset();
	//m_InputHandler.setMirrored(m_MotoGame.getCamera()->isMirrored());
	m_MotoGame.prePlayLevel(&m_InputHandler, m_pJustPlayReplay, true);
	m_MotoGame.setInfos("");
	
	/* add the players */
	int v_nbPlayer = getNumberOfPlayersToPlay();
	Logger::Log("Preplay level for %i player(s)", v_nbPlayer);

	initCameras(v_nbPlayer);
	m_Renderer->addPlayTimes(m_MotoGame.getNumberCameras());

	for(int i=0; i<v_nbPlayer; i++) {
		m_MotoGame.setCurrentCamera(i);
		m_MotoGame.getCamera()->setPlayerToFollow(m_MotoGame.addPlayerBiker(m_MotoGame.getLevelSrc()->PlayerStart(),
										    DD_RIGHT,
										    &m_theme, m_theme.getPlayerTheme(),
										    getColorFromPlayerNumber(i),
										    getUglyColorFromPlayerNumber(i),
										    m_bEnableEngineSound));
	}
	// if there's more camera than player (ex: 3 players and 4 cameras),
	// then, make the remaining cameras follow the first player
	if(v_nbPlayer < m_MotoGame.getNumberCameras()){
	  for(int i=v_nbPlayer; i<m_MotoGame.getNumberCameras(); i++){
	    m_MotoGame.setCurrentCamera(i);
	    m_MotoGame.getCamera()->setPlayerToFollow(m_MotoGame.Players()[0]);
	  }
	}

	if(m_MotoGame.getNumberCameras() > 1){
	  // make the zoom camera follow the first player
	  m_MotoGame.setCurrentCamera(m_MotoGame.getNumberCameras());
	  m_MotoGame.getCamera()->setPlayerToFollow(m_MotoGame.Players()[0]);
	}

	/* add the ghosts */
	if(m_bEnableGhost) {
	  try {
	    addGhosts(&m_MotoGame, &m_theme);
	  } catch(Exception &e) {
	    /* anyway */
	  }
	}
      } catch(Exception &e) {
	Logger::Log(std::string("** Warning ** : failed to initialize level\n" + e.getMsg()).c_str());
	m_MotoGame.endLevel();
	setState(m_StateAfterPlaying);
	notifyMsg(splitText(e.getMsg(), 50));
	return;
      }

    m_State = GS_PREPLAYING;

    std::string T1 = "--:--:--", T2 = "--:--:--";

    /* get best result */
    v_result = m_db->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
    			    "id_level=\"" + 
    			    xmDatabase::protectString(m_MotoGame.getLevelSrc()->Id()) + "\";",
    			    nrow);
    v_res = m_db->getResult(v_result, 1, 0, 0);
    if(v_res != NULL) {
      T1 = formatTime(atof(v_res));
    }
    m_db->read_DB_free(v_result);
    
    /* get best player result */
    v_result = m_db->readDB("SELECT MIN(finishTime) FROM profile_completedLevels WHERE "
    			    "id_level=\"" + 
    			    xmDatabase::protectString(m_MotoGame.getLevelSrc()->Id()) + "\" " + 
    			    "AND id_profile=\"" + xmDatabase::protectString(m_xmsession->profile())  + "\";",
    			    nrow);
    v_res = m_db->getResult(v_result, 1, 0, 0);
    if(v_res != NULL) {
      T2 = formatTime(atof(v_res));
    }
    m_db->read_DB_free(v_result);
    
    m_Renderer->setBestTime(T1 + std::string(" / ") + T2);
    m_Renderer->hideReplayHelp();
    
    /* World-record stuff */
    _UpdateWorldRecord(m_PlaySpecificLevelId);

    /* Prepare level */
    m_Renderer->prepareForNewLevel();
    prestartAnimation_init();
  }

  void GameApp::initCameras(int nbPlayer) {
    int width  = drawLib->getDispWidth();
    int height = drawLib->getDispHeight();

    switch(nbPlayer){
    default:
    case 1:
      m_MotoGame.addCamera(Vector2i(0,0),
			   Vector2i(width, height));
      break;
    case 2:
      m_MotoGame.addCamera(Vector2i(0,height/2),
			   Vector2i(width, height));
      m_MotoGame.addCamera(Vector2i(0,0),
			   Vector2i(width, height/2));
      break;
    case 3:
    case 4:
      m_MotoGame.addCamera(Vector2i(0,height/2),
			   Vector2i(width/2, height));
      m_MotoGame.addCamera(Vector2i(width/2,height/2),
			   Vector2i(width, height));
      m_MotoGame.addCamera(Vector2i(0,0),
			   Vector2i(width/2, height/2));
      m_MotoGame.addCamera(Vector2i(width/2,0),
			   Vector2i(width, height/2));
      break;
    }
    // the autozoom camera
    if(nbPlayer > 1){
      m_MotoGame.addCamera(Vector2i(0,0),
			   Vector2i(width, height));
      m_MotoGame.setCurrentCamera(m_MotoGame.getNumberCameras());
      m_MotoGame.getCamera()->initCamera();
    }
    for(int i=0; i<m_MotoGame.getNumberCameras(); i++){
      m_MotoGame.setCurrentCamera(i);
      m_MotoGame.getCamera()->initCamera();
    }
    if(nbPlayer > 1){
      // current cam is autozoom one
      m_MotoGame.setCurrentCamera(m_MotoGame.getNumberCameras());
    }else{
      m_MotoGame.setCurrentCamera(0);
    }
  }

  void GameApp::statePrestart_step() {
    prestartAnimation_step();
  }
  
  void GameApp::zoomAnimation1_init() {
    m_fPrePlayStartTime = getXMTime();
    m_fPrePlayStartInitZoom = m_MotoGame.getCamera()->getCurrentZoom();  // because the man can change ugly mode while the animation
    m_fPrePlayStartCameraX  = m_MotoGame.getCamera()->getCameraPositionX();
    m_fPrePlayStartCameraY  = m_MotoGame.getCamera()->getCameraPositionY();

    m_zoomX = (2.0 * ((float)drawLib->getDispWidth() / (float)drawLib->getDispHeight())) / (m_MotoGame.getLevelSrc()->RightLimit() - m_MotoGame.getLevelSrc()->LeftLimit() + 2*PRESTART_ANIMATION_MARGIN_SIZE);
    m_zoomY = 2.0 /(m_MotoGame.getLevelSrc()->TopLimit() - m_MotoGame.getLevelSrc()->BottomLimit()+2*PRESTART_ANIMATION_MARGIN_SIZE);
		
    if (m_zoomX > m_zoomY){
      float visibleHeight,cameraStartHeight;
			
      m_zoomU=m_zoomX;
      static_time = (m_MotoGame.getLevelSrc()->TopLimit() - m_MotoGame.getLevelSrc()->BottomLimit()) / (2.0/m_zoomU);
			
      visibleHeight = 2.0/m_zoomU;
      cameraStartHeight= visibleHeight/2.0;
			
      m_fPreCameraStartX = (m_MotoGame.getLevelSrc()->RightLimit() + m_MotoGame.getLevelSrc()->LeftLimit())/2;
      m_fPreCameraStartY = m_MotoGame.getLevelSrc()->TopLimit() - cameraStartHeight + PRESTART_ANIMATION_MARGIN_SIZE;
      m_fPreCameraFinalX = (m_MotoGame.getLevelSrc()->RightLimit() + m_MotoGame.getLevelSrc()->LeftLimit())/2;
      m_fPreCameraFinalY = m_MotoGame.getLevelSrc()->BottomLimit() + cameraStartHeight - PRESTART_ANIMATION_MARGIN_SIZE;
			
      if ( fabs(m_fPreCameraStartY - m_fPrePlayStartCameraY) > fabs(m_fPreCameraFinalY - m_fPrePlayStartCameraY)) {
	float f;
	f = m_fPreCameraFinalY;
	m_fPreCameraFinalY = m_fPreCameraStartY;
	m_fPreCameraStartY = f;
      }
			
    }else {
      float visibleWidth,cameraStartLeft;
			
      m_zoomU=m_zoomY;
      static_time = (m_MotoGame.getLevelSrc()->RightLimit() - m_MotoGame.getLevelSrc()->LeftLimit()) / ((2.0 * ((float)drawLib->getDispWidth() / (float)drawLib->getDispHeight()))/m_zoomU);
      
      visibleWidth = (2.0 * ((float)drawLib->getDispWidth() / (float)drawLib->getDispHeight()))/m_zoomU;
      cameraStartLeft = visibleWidth/2.0;
			
      m_fPreCameraStartX = m_MotoGame.getLevelSrc()->RightLimit() - cameraStartLeft + PRESTART_ANIMATION_MARGIN_SIZE;
      m_fPreCameraStartY = (m_MotoGame.getLevelSrc()->BottomLimit() + m_MotoGame.getLevelSrc()->TopLimit())/2;
      m_fPreCameraFinalX = m_MotoGame.getLevelSrc()->LeftLimit() + cameraStartLeft - PRESTART_ANIMATION_MARGIN_SIZE;
      m_fPreCameraFinalY = (m_MotoGame.getLevelSrc()->BottomLimit() + m_MotoGame.getLevelSrc()->TopLimit())/2;
   
      if ( fabs(m_fPreCameraStartX - m_fPrePlayStartCameraX) > fabs(m_fPreCameraFinalX - m_fPrePlayStartCameraX)) {
	float f;
	f = m_fPreCameraFinalX;
	m_fPreCameraFinalX = m_fPreCameraStartX;
	m_fPreCameraStartX = f;
      }
    } 
  }
	
  bool GameApp::zoomAnimation1_step() {
    if(getXMTime() > m_fPrePlayStartTime + static_time + PRESTART_ANIMATION_TIME) {
      return false;
    }
    if(getXMTime() > m_fPrePlayStartTime + static_time){
      float zx, zy, zz;

      zz = logf(PRESTART_ANIMATION_CURVE * ((PRESTART_ANIMATION_TIME + static_time - getXMTime() + m_fPrePlayStartTime) / (PRESTART_ANIMATION_TIME)) + 1.0) / LOGF_PRE_ANIM_TIME_ADDED_ONE * (m_fPrePlayStartInitZoom - m_zoomU);
			
      m_MotoGame.getCamera()->setZoom(m_fPrePlayStartInitZoom - zz);
			
      zx = (PRESTART_ANIMATION_TIME + static_time - getXMTime() + m_fPrePlayStartTime)
      / (PRESTART_ANIMATION_TIME) 
      * (m_fPrePlayStartCameraX - m_fPrePlayCameraLastX);
      zy =  (PRESTART_ANIMATION_TIME + static_time - getXMTime() + m_fPrePlayStartTime)
      / (PRESTART_ANIMATION_TIME) 
      * (m_fPrePlayStartCameraY - m_fPrePlayCameraLastY);
		
      m_MotoGame.getCamera()->setCameraPosition(m_fPrePlayStartCameraX-zx, m_fPrePlayStartCameraY-zy);
    } else {
      float zx,zy;
        
      m_MotoGame.getCamera()->setZoom(m_zoomU);
        
      zx  = (static_time - getXMTime() + m_fPrePlayStartTime) / (static_time) 
      * (m_fPreCameraStartX - m_fPreCameraFinalX); 

      zy = (static_time - getXMTime() + m_fPrePlayStartTime) / (static_time) 
      * (m_fPreCameraStartY - m_fPreCameraFinalY);
				
      m_MotoGame.getCamera()->setCameraPosition( m_fPreCameraStartX  - zx, m_fPreCameraStartY - zy);
				
      m_fPrePlayCameraLastX= m_fPreCameraStartX - zx;
      m_fPrePlayCameraLastY= m_fPreCameraStartY - zy;
    }
    return true;
  }

  void GameApp::zoomAnimation1_abort() {
    m_MotoGame.getCamera()->setZoom(m_fPrePlayStartInitZoom); // because the man can change ugly mode while the animation
    m_MotoGame.getCamera()->setCameraPosition(m_fPrePlayStartCameraX, m_fPrePlayStartCameraY);
  }

  void GameApp::zoomAnimation2_init() {
    zoomAnimation1_init();
    fAnimPlayStartZoom = m_MotoGame.getCamera()->getCurrentZoom(); 
    fAnimPlayStartCameraX = m_MotoGame.getCamera()->getCameraPositionX();
    fAnimPlayStartCameraY = m_MotoGame.getCamera()->getCameraPositionY();
    fAnimPlayFinalZoom = m_zoomU;
    fAnimPlayFinalCameraX1 = m_fPreCameraStartX;
    fAnimPlayFinalCameraY1 = m_fPreCameraStartY;
    fAnimPlayFinalCameraX2 = m_fPreCameraFinalX;
    fAnimPlayFinalCameraY2 = m_fPreCameraFinalY;
    m_fPrePlayStartTime = getXMTime();
    
    m_autoZoomStep = 1;
  }

  bool GameApp::zoomAnimation2_step() {
    switch(m_autoZoomStep) {
      
      case 1:
      if(getXMTime() > m_fPrePlayStartTime + INPLAY_ANIMATION_TIME) {
	float zx, zy;
	zx = (fAnimPlayFinalCameraX1 - fAnimPlayFinalCameraX2) * (sin((getXMTime() - m_fPrePlayStartTime - INPLAY_ANIMATION_TIME) * 2 * 3.1415927 / INPLAY_ANIMATION_SPEED - 3.1415927/2) + 1) / 2;
	zy = (fAnimPlayFinalCameraY1 - fAnimPlayFinalCameraY2) * (sin((getXMTime() - m_fPrePlayStartTime - INPLAY_ANIMATION_TIME) * 2 * 3.1415927 / INPLAY_ANIMATION_SPEED - 3.1415927/2) + 1) / 2;
	m_MotoGame.getCamera()->setCameraPosition(fAnimPlayFinalCameraX1 - zx,fAnimPlayFinalCameraY1 - zy);
	return true;
      }
      if(getXMTime() > m_fPrePlayStartTime){
	float zx, zy, zz, coeff;
	coeff = (getXMTime() - m_fPrePlayStartTime) / (INPLAY_ANIMATION_TIME);
	zx = coeff * (fAnimPlayStartCameraX - fAnimPlayFinalCameraX1);
	zy = coeff * (fAnimPlayStartCameraY - fAnimPlayFinalCameraY1);
	zz = coeff * (fAnimPlayStartZoom - fAnimPlayFinalZoom);
	
	m_MotoGame.getCamera()->setZoom(fAnimPlayStartZoom - zz);
	m_MotoGame.getCamera()->setCameraPosition(fAnimPlayStartCameraX - zx,fAnimPlayStartCameraY - zy);
      }
      
      return true;
      break;

      case 2:
      zoomAnimation2_init_unzoom();
      m_autoZoomStep = 3;
      break;

      case 3:
      return zoomAnimation2_unstep();
      break;
    }

    return true;
  }

  void GameApp::zoomAnimation2_init_unzoom(){
    m_fPrePlayStartTime = getXMTime();
    fAnimPlayFinalZoom = fAnimPlayStartZoom;
    fAnimPlayStartZoom = m_MotoGame.getCamera()->getCurrentZoom();
    fAnimPlayFinalCameraX1 = fAnimPlayStartCameraX;
    fAnimPlayFinalCameraY1 = fAnimPlayStartCameraY;
    fAnimPlayStartCameraX = m_MotoGame.getCamera()->getCameraPositionX();
    fAnimPlayStartCameraY = m_MotoGame.getCamera()->getCameraPositionY();
  }

  bool GameApp::zoomAnimation2_unstep() {
    if(getXMTime() > m_fPrePlayStartTime + INPLAY_ANIMATION_TIME) {
      return false;
    }
    if(getXMTime() > m_fPrePlayStartTime){
      float zx, zy, zz, coeff;
      coeff = (getXMTime() - m_fPrePlayStartTime) / (INPLAY_ANIMATION_TIME);
      zx = coeff * (fAnimPlayStartCameraX - fAnimPlayFinalCameraX1);
      zy = coeff * (fAnimPlayStartCameraY - fAnimPlayFinalCameraY1);
      zz = coeff * (fAnimPlayStartZoom - fAnimPlayFinalZoom);
			
      m_MotoGame.getCamera()->setZoom(fAnimPlayStartZoom - zz);
      m_MotoGame.getCamera()->setCameraPosition(fAnimPlayStartCameraX - zx,fAnimPlayStartCameraY - zy);
      return true;
    }
    return false;
  }

  void GameApp::zoomAnimation2_abort() {
    m_MotoGame.getCamera()->setZoom(fAnimPlayFinalZoom);
    m_MotoGame.getCamera()->setCameraPosition(fAnimPlayFinalCameraX1,fAnimPlayFinalCameraY1);
  }

  void GameApp::prestartAnimation_init() {
    if(m_bPrePlayAnim && m_bEnableInitZoom && m_xmsession->ugly() == false) {
      m_MotoGame.gameMessage(m_MotoGame.getLevelSrc()->Name(), false, PRESTART_ANIMATION_LEVEL_MSG_DURATION);
      zoomAnimation1_init();
    } else {
      m_bPrePlayAnim = false;
      setState(GS_PLAYING);
    }
  }
  
  void GameApp::prestartAnimation_step() {
    if(m_bPrePlayAnim && m_bEnableInitZoom && m_xmsession->ugly() == false) {
      if(zoomAnimation1_step() == false) {
        setPrePlayAnim(false); // disable anim
	zoomAnimation1_abort();
        setState(GS_PLAYING);
      }
    } else { /* animation has been rupted */
      setPrePlayAnim(false); // disable anim
      zoomAnimation1_abort();
      setState(GS_PLAYING);
    }
    m_MotoGame.updateGameMessages();
  }

  void GameApp::_UpdateLevelsLists() {
    _UpdateLevelPackList();
    _UpdateLevelLists();
  }

  void GameApp::reloadTheme() {
    try {
      m_theme.load(m_db->themes_getFileName(getConfigThemeName(m_themeChoicer)));
    } catch(Exception &e) {
      /* unable to load the theme, load the default one */
      m_theme.load(m_db->themes_getFileName(THEME_DEFAULT_THEMENAME));
    }
  }

  void GameApp::setAutoZoom(bool bValue) {
    m_autoZoom = bValue;
  }
  
  bool GameApp::AutoZoom() {
    return m_autoZoom;
  }

  void GameApp::setAutoZoomStep(int n) {
    m_autoZoomStep = n;
  }

  int GameApp::AutoZoomStep() {
    return m_autoZoomStep;
  }

  void GameApp::autoZoom() {
    if(m_autoZoom) {
      if(m_bAutoZoomInitialized == false) {
	lockMotoGame(true);
	zoomAnimation2_init();
	m_bAutoZoomInitialized = true;
      } else {
	if(zoomAnimation2_step() == false) {
	  lockMotoGame(false);
	  m_bAutoZoomInitialized = false;
	  m_autoZoom = false;
	}
      }
    }
  }

  void GameApp::lockMotoGame(bool bLock) {
    m_bLockMotoGame = bLock;
  }
  
  bool GameApp::isLockedMotoGame() const {
    return m_bLockMotoGame;
  }
  
  void XMMotoGameHooks::OnTakeEntity() {
    /* Play yummy-yummy sound */
    if(m_GameApp != NULL) {
      try {
	Sound::playSampleByName(m_GameApp->getTheme()->getSound
				(m_MotoGame->getLevelSrc()->SoundForPickUpStrawberry())->FilePath());
      } catch(Exception &e) {
      }
    }
  }
  
  void XMMotoGameHooks::setGameApps(GameApp *i_GameApp, MotoGame *i_MotoGame) {
    m_GameApp = i_GameApp;
    m_MotoGame = i_MotoGame;
  }

  XMMotoGameHooks::XMMotoGameHooks() {
    m_GameApp = NULL;
  }
  
  XMMotoGameHooks::~XMMotoGameHooks() {
  }

  void GameApp::TeleportationCheatTo(int i_player, Vector2f i_position) {
    m_MotoGame.setPlayerPosition(i_player, i_position.x, i_position.y, true);
    m_MotoGame.getCamera()->initCamera();
    m_MotoGame.addPenalityTime(900); /* 15 min of penality for that ! */
  }

  void GameApp::loadLevelHook(std::string i_level, int i_percentage) {
    std::ostringstream v_percentage;
    v_percentage << i_percentage;
    v_percentage << "%";

    if(m_reloadingLevelsUser == false) {
      _UpdateLoadingScreen(0, std::string(GAMETEXT_LOAD_LEVEL_HOOK) + std::string("\n") + v_percentage.str() + std::string(" ") + i_level);
    } else {
      _SimpleMessage(GAMETEXT_RELOADINGLEVELS + std::string("\n") + v_percentage.str(), &m_InfoMsgBoxRect);
    }

    /* pump events to so that windows don't think the appli is crashed */
    SDL_PumpEvents();
  }

  void GameApp::updatingDatabase(std::string i_message) {
    _UpdateLoadingScreen(0, i_message);

    /* pump events to so that windows don't think the appli is crashed */
    SDL_PumpEvents();
  }

  bool GameApp::creditsModeActive() {
    return m_bCreditsModeActive;
  }

  UIWindow* GameApp::stats_generateReport(const std::string &PlayerName, UIWindow *pParent,
					  int x, int y, int nWidth, int nHeight, FontManager* pFont) {
    /* Create stats window */
    UIWindow *p;
    char **v_result;
    unsigned int nrow;

    int   v_nbStarts        = 0;
    std::string v_since;
    float v_totalPlayedTime = 0.0;
    int   v_nbPlayed        = 0;
    int   v_nbDied          = 0;
    int   v_nbCompleted     = 0;
    int   v_nbRestarted     = 0;
    int   v_nbDiffLevels    = 0;
    std::string v_level_name;
  
    p = new UIWindow(pParent, x, y, "", nWidth, nHeight);
    UIButton *pUpdateButton = new UIButton(p,nWidth-115,nHeight-57,GAMETEXT_UPDATE,115,57);
    pUpdateButton->setContextHelp(CONTEXTHELP_UPDATESTATS);
    pUpdateButton->setFont(pFont);
    pUpdateButton->setType(UI_BUTTON_TYPE_SMALL);
    pUpdateButton->setID("UPDATE_BUTTON");  

    v_result = m_db->readDB("SELECT a.nbStarts, a.since, SUM(b.playedTime), "
			    "SUM(b.nbPlayed), SUM(b.nbDied), SUM(b.nbCompleted), "
			    "SUM(b.nbRestarted), count(b.id_level) "
			    "FROM stats_profiles AS a INNER JOIN stats_profiles_levels AS b "
			    "ON a.id_profile=b.id_profile "
			    "WHERE a.id_profile=\"" + xmDatabase::protectString(PlayerName) + "\" "
			    "GROUP BY a.id_profile;",
			    nrow);

    if(nrow == 0) {
      m_db->read_DB_free(v_result);
      return p;
    }
  
    v_nbStarts        = atoi(m_db->getResult(v_result, 8, 0, 0));
    v_since           =      m_db->getResult(v_result, 8, 0, 1);
    v_totalPlayedTime = atof(m_db->getResult(v_result, 8, 0, 2));
    v_nbPlayed        = atoi(m_db->getResult(v_result, 8, 0, 3));
    v_nbDied          = atoi(m_db->getResult(v_result, 8, 0, 4));
    v_nbCompleted     = atoi(m_db->getResult(v_result, 8, 0, 5));
    v_nbRestarted     = atoi(m_db->getResult(v_result, 8, 0, 6));
    v_nbDiffLevels    = atoi(m_db->getResult(v_result, 8, 0, 7));
  
    m_db->read_DB_free(v_result);
  
    /* Per-player info */
    char cBuf[512];
    char cTime[512];
    int nHours = ((int)v_totalPlayedTime) / (60 * 60);
    int nMinutes = (((int)v_totalPlayedTime) / (60)) - nHours*60;
    int nSeconds = (((int)v_totalPlayedTime)) - nMinutes*60 - nHours*3600;
    if(nHours > 0) sprintf(cTime,(std::string(GAMETEXT_XHOURS) + ", " + std::string(GAMETEXT_XMINUTES) + ", " + std::string(GAMETEXT_XSECONDS)).c_str(),nHours,nMinutes,nSeconds);
    else if(nMinutes > 0) sprintf(cTime,(std::string(GAMETEXT_XMINUTES) + ", " + std::string(GAMETEXT_XSECONDS)).c_str(),nMinutes,nSeconds);
    else sprintf(cTime,GAMETEXT_XSECONDS,nSeconds);
  
    sprintf(cBuf,GAMETEXT_XMOTOGLOBALSTATS,      
	    v_since.c_str(), v_nbStarts, v_nbPlayed, v_nbDiffLevels,
	    v_nbDied, v_nbCompleted, v_nbRestarted, cTime);                           
  
    UIStatic *pText = new UIStatic(p, 0, 0, cBuf, nWidth, 80);
    pText->setHAlign(UI_ALIGN_LEFT);
    pText->setTextSolidColor(MAKE_COLOR(255,255,0,255));
    pText->setFont(pFont);

    /* Per-level stats */      
    pText = new UIStatic(p,0,90, std::string(GAMETEXT_MOSTPLAYEDLEVELSFOLLOW) + ":",nWidth,20);
    pText->setHAlign(UI_ALIGN_LEFT);
    pText->setTextSolidColor(MAKE_COLOR(255,255,0,255));
    pText->setFont(pFont);      

    v_result = m_db->readDB("SELECT a.name, b.nbPlayed, b.nbDied, "
			    "b.nbCompleted, b.nbRestarted, b.playedTime "
			    "FROM levels AS a INNER JOIN stats_profiles_levels AS b ON a.id_level=b.id_level "
			    "WHERE id_profile=\"" + xmDatabase::protectString(PlayerName) + "\" "
			    "ORDER BY nbPlayed DESC LIMIT 10;",
			    nrow);

    int cy = 110;
    for(int i=0; i<nrow; i++) {
      if(cy + 45 > nHeight) break; /* out of window */

      v_level_name      =      m_db->getResult(v_result, 6, i, 0);
      v_totalPlayedTime = atof(m_db->getResult(v_result, 6, i, 5));
      v_nbDied          = atoi(m_db->getResult(v_result, 6, i, 2));
      v_nbPlayed        = atoi(m_db->getResult(v_result, 6, i, 1));
      v_nbCompleted     = atoi(m_db->getResult(v_result, 6, i, 3));
      v_nbRestarted     = atoi(m_db->getResult(v_result, 6, i, 4));
    
      sprintf(cBuf,("[%s] %s:\n   " + std::string(GAMETEXT_XMOTOLEVELSTATS)).c_str(),
	      GameApp::formatTime(v_totalPlayedTime).c_str(), v_level_name.c_str(),
	      v_nbPlayed, v_nbDied, v_nbCompleted, v_nbRestarted);
    
      pText = new UIStatic(p,0,cy,cBuf,nWidth,45);
      pText->setHAlign(UI_ALIGN_LEFT);        
      pText->setTextSolidColor(MAKE_COLOR(255,255,0,255));
      pText->setFont(pFont);
    
      cy += 45;
    }  

    m_db->read_DB_free(v_result);
    return p;
  }

  void GameApp::initReplaysFromDir() {
    ReplayInfo* rplInfos;
    std::vector<std::string> ReplayFiles;
    ReplayFiles = FS::findPhysFiles("Replays/*.rpl");

    m_db->replays_add_begin();

    for(unsigned int i=0; i<ReplayFiles.size(); i++) {
      try {
	/* pump events to so that windows don't think the appli is crashed */
	SDL_PumpEvents();

	if(FS::getFileBaseName(ReplayFiles[i]) == "Latest") {
	  continue;
	}
	addReplay(ReplayFiles[i]);

      } catch(Exception &e) {
	// ok, forget this replay
      }
    }
    m_db->replays_add_end();
  }

  void GameApp::addReplay(const std::string& i_file) {
    ReplayInfo* rplInfos;
    
    rplInfos = Replay::getReplayInfos(FS::getFileBaseName(i_file));
    if(rplInfos == NULL) {
      throw Exception("Unable to extract data from replay file");
    }

    try {
      m_db->replays_add(rplInfos->Level,
			rplInfos->Name,
			rplInfos->Player,
			rplInfos->IsFinished,
			rplInfos->fFinishTime);

    } catch(Exception &e2) {
      delete rplInfos;
      throw e2;
    }
  }

  void GameApp::_UpdateCurrentPackList(const std::string& i_id_level, float i_playerHighscore) {
    if(m_pActiveLevelPack == NULL) return;
    UILevelList *pList = (UILevelList *)m_pLevelPackViewer->getChild("LEVELPACK_LEVEL_LIST"); 
    if(pList == NULL) return;

    pList->updateLevel(i_id_level, i_playerHighscore);
  }

  void GameApp::setSpecificReplay(const std::string& i_replay) {
    m_PlaySpecificReplay = i_replay;
  }
  
  void GameApp::setSpecificLevelId(const std::string& i_levelID) {
    m_PlaySpecificLevelId = i_levelID;
  }

  void GameApp::setSpecificLevelFile(const std::string& i_leveFile) {
    m_PlaySpecificLevelFile = i_leveFile;
  }

void GameApp::addGhosts(MotoGame* i_motogame, Theme* i_theme) {
  std::string v_replay_MYBEST;
  std::string v_replay_THEBEST;
  std::string v_replay_BESTOFROOM;
  float v_fFinishTime;
  float v_player_fFinishTime;

  /* first, add the best of the room -- because if mybest or thebest = bestofroom, i prefer to see writen bestofroom */
  if(m_xmsession->ghostStrategy_BESTOFROOM()) {
    std::string v_replay_MYBEST_tmp;
    v_replay_MYBEST_tmp = _getGhostReplayPath_bestOfThePlayer(i_motogame->getLevelSrc()->Id(), v_player_fFinishTime);
    v_replay_BESTOFROOM = _getGhostReplayPath_bestOfTheRoom(i_motogame->getLevelSrc()->Id(), v_fFinishTime);

    /* add MYBEST if MYBEST if better the  BESTOF ROOM */
    if(v_player_fFinishTime > 0.0 && (v_fFinishTime < 0.0 || v_player_fFinishTime < v_fFinishTime)) {
      v_replay_BESTOFROOM = v_replay_MYBEST_tmp;
    }
    
    if(v_replay_BESTOFROOM != "") {
	m_MotoGame.addGhostFromFile(v_replay_BESTOFROOM,
				    m_db->webrooms_getName(m_WebHighscoresIdRoom),
				    &m_theme, m_theme.getGhostTheme(),
				    TColor(255,255,0,0),
				    TColor(GET_RED(i_theme->getGhostTheme()->getUglyRiderColor()),
					   GET_GREEN(i_theme->getGhostTheme()->getUglyRiderColor()),
					   GET_BLUE(i_theme->getGhostTheme()->getUglyRiderColor()),
					   0)
				    );
    }
  }

  /* second, add your best */
  if(m_xmsession->ghostStrategy_MYBEST()) {
    v_replay_MYBEST = _getGhostReplayPath_bestOfThePlayer(i_motogame->getLevelSrc()->Id(), v_fFinishTime);
    if(v_replay_MYBEST != "") {
      if(v_replay_MYBEST != v_replay_BESTOFROOM) {
	i_motogame->addGhostFromFile(v_replay_MYBEST, GAMETEXT_GHOST_BEST,
				     i_theme, i_theme->getGhostTheme(),
				     TColor(255,150,0,0),
				     TColor(GET_RED(i_theme->getGhostTheme()->getUglyRiderColor()),
					    GET_GREEN(i_theme->getGhostTheme()->getUglyRiderColor()),
					    GET_BLUE(i_theme->getGhostTheme()->getUglyRiderColor()),
					    0)
				     );
      }
    }
  }

  /* third, the best locally */
  if(m_xmsession->ghostStrategy_THEBEST()) {
    v_replay_THEBEST = _getGhostReplayPath_bestOfLocal(i_motogame->getLevelSrc()->Id(), v_fFinishTime);
    if(v_replay_THEBEST != "") {
      if(v_replay_THEBEST != v_replay_MYBEST && v_replay_THEBEST != v_replay_BESTOFROOM) { /* don't add two times the same ghost */
	i_motogame->addGhostFromFile(v_replay_THEBEST, GAMETEXT_GHOST_LOCAL,
				     i_theme, i_theme->getGhostTheme(),
				     TColor(0,0,255,0),
				     TColor(GET_RED(i_theme->getGhostTheme()->getUglyRiderColor()),
					    GET_GREEN(i_theme->getGhostTheme()->getUglyRiderColor()),
					    GET_BLUE(i_theme->getGhostTheme()->getUglyRiderColor()),
					    0)
				     );
      }
    }
  }

}

void GameApp::addLevelToFavorite(const std::string& i_levelId) {
  m_levelsManager.addToFavorite(m_db, m_xmsession->profile(), i_levelId);
  _UpdateLevelPackLevelList(VPACKAGENAME_FAVORITE_LEVELS);
  _UpdateLevelLists();
}

void GameApp::switchLevelToFavorite(const std::string& i_levelId, bool v_displayMessage) {
  if(m_levelsManager.isInFavorite(m_db, m_xmsession->profile(), i_levelId)) {
    m_levelsManager.delFromFavorite(m_db, m_xmsession->profile(), i_levelId);
    if(v_displayMessage) {
      m_sysMsg->displayText(GAMETEXT_LEVEL_DELETED_FROM_FAVORITE);
    }
  } else {
    m_levelsManager.addToFavorite(m_db, m_xmsession->profile(), i_levelId);
    if(v_displayMessage) {
      m_sysMsg->displayText(GAMETEXT_LEVEL_ADDED_TO_FAVORITE);
    }
  }

  _UpdateLevelPackLevelList(VPACKAGENAME_FAVORITE_LEVELS);
  _UpdateLevelLists();
}
