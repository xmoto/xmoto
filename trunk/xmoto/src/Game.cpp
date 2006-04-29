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
 *  Game application.
 */
#include "Game.h"
#include "VFileIO.h"
#include "Sound.h"
#include "PhysSettings.h"
#include "Input.h"

#if defined(SUPPORT_WEBACCESS)
  #include <curl/curl.h>
#endif

namespace vapp {

  /* CRY! */
  extern InputHandler *m_pActiveInputHandler;
  
  /*===========================================================================
  Update levels lists - must be done after each completed level
  ===========================================================================*/
  void GameApp::_UpdateLevelLists(void) {
    _CreateLevelLists((UIList *)m_pPlayWindow->getChild("PLAY_LEVEL_TABS:PLAY_EXTERNAL_LEVELS_TAB:PLAY_EXTERNAL_LEVELS_LIST"),
                      (UIList *)m_pPlayWindow->getChild("PLAY_LEVEL_TABS:PLAY_INTERNAL_LEVELS_TAB:PLAY_INTERNAL_LEVELS_LIST"));    
  }

  /*===========================================================================
  Update replays list
  ===========================================================================*/
  void GameApp::_UpdateReplaysList(void) {
    _CreateReplaysList((UIList *)m_pReplaysWindow->getChild("REPLAY_LIST"));                       
  }

  /*===========================================================================
  Change game state
  ===========================================================================*/
  void GameApp::setState(GameState s) {
    /* This function is called to perform a controlled game state change.
       The various states are described below in the switch-statement */  
    m_State = s;
    
    /* Always clear context when changing state */
    m_Renderer.getGUI()->clearContext();
    
    switch(s) {
      case GS_LEVELPACK_VIEWER: {
          m_pLevelPackViewer->showWindow(true);
          m_pMainMenu->showWindow(true);
          
          UIList *pList = (UIList *)m_pLevelPackViewer->getChild("LEVELPACK_LEVEL_LIST");
          if(pList != NULL) {
            pList->makeActive();
          }
        }
        break;
      case GS_REPLAYING: {
        //SDL_ShowCursor(SDL_DISABLE);
        m_bShowCursor = false;
        
        /* Open a replay for input */
        if(m_pReplay != NULL) delete m_pReplay;
        m_pReplay = new Replay;
        std::string LevelID = m_pReplay->openReplay(m_PlaySpecificReplay,&m_fCurrentReplayFrameRate,m_ReplayPlayerName);
        if(LevelID == "") {
          Log("** Warning ** : No valid level identifier could be extracted from the replay: %s",m_PlaySpecificReplay.c_str());
          throw Exception("invalid replay");
        }
        else {
          /* Fine, open the level */
          LevelSrc *pLevelSrc = _FindLevelByID(LevelID);
          if(pLevelSrc == NULL) {
            Log("** Warning ** : level '%s' specified by replay '%s' not found",LevelID.c_str(),m_PlaySpecificReplay.c_str());
            
            char cBuf[256];
            sprintf(cBuf,GAMETEXT_LEVELREQUIREDBYREPLAY,LevelID.c_str());
            setState(GS_MENU);
            notifyMsg(cBuf);                        
          }
          else {    
            /* Init level */    
            m_InputHandler.resetScriptKeyHooks();                                   
            m_MotoGame.playLevel( pLevelSrc );
            m_nFrame = 0;
            m_Renderer.prepareForNewLevel();
            
            /* Reconstruct game events that are going to happen during the replay */
            m_MotoGame.unserializeGameEvents( *m_pReplay );            
            
            /* Show help string */
            if(!isNoGraphics()) {
              PlayerTimeEntry *pBestTime = m_Profiles.getBestTime(LevelID);
              PlayerTimeEntry *pBestPTime = m_Profiles.getBestPlayerTime(m_pPlayer->PlayerName,LevelID);
              
              std::string T1 = "--:--:--",T2 = "--:--:--";
              if(pBestTime != NULL)
                T1 = formatTime(pBestTime->fFinishTime);
              if(pBestPTime != NULL)
                T2 = formatTime(pBestPTime->fFinishTime);
              
              m_Renderer.setBestTime(T1 + std::string(" / ") + T2 + std::string(GAMETEXT_REPLAYHELPTEXT));              
              
              if(m_bBenchmark) m_Renderer.setBestTime("");
              
              /* World-record stuff */
              _UpdateWorldRecord(LevelID);
            }

            m_fStartTime = getRealTime();
          }          
        }
        break;
      }  
      case GS_MENU: {
        //SDL_ShowCursor(SDL_ENABLE);
        m_bShowCursor = true;
        
        /* Any replays to get rid off? */
        if(m_pReplay != NULL) delete m_pReplay;
        m_pReplay = NULL;

        /* The main menu, the one which is entered initially when the game 
           begins. */
        m_pMainMenu->showWindow(true);
        
        /* Did the initializer come up with messages for the user? */
        if(getUserNotify() != "") {
          notifyMsg(getUserNotify());
        }                
        /* Should we show a notification box? (with important one-time info) */
        else if(m_Config.getBool("NotifyAtInit")) {
          notifyMsg(GAMETEXT_NOTIFYATINIT);                    
          
          /* Don't do this again, please */
          m_Config.setBool("NotifyAtInit",false); 
        }        
        break;
      }
      case GS_PLAYING: {
//        SDL_ShowCursor(SDL_DISABLE);
        m_bShowCursor = false;
        
        /* Initialize controls */
        m_InputHandler.configure(&m_Config);
        m_pActiveInputHandler = &m_InputHandler;
      
        /* Default playing state */
        m_fLastFrameTime = 0.0f;
        m_fLastPerfStateTime = 0.0f;
        m_fLastStateSerializationTime = -100.0f; /* loong time ago :) */

        /* We need a profile */
        if(m_pPlayer == NULL) {
          Log("** Warning ** : no player profile selected, use -profile option");
          throw Exception("no player");
        }
        
        /* Find the level */
        LevelSrc *pLevelSrc = _FindLevelByID(m_PlaySpecificLevel);
        if(pLevelSrc == NULL) {
          Log("** Warning ** : level '%s' not found",m_PlaySpecificLevel.c_str());

          char cBuf[256];
          sprintf(cBuf,GAMETEXT_LEVELNOTFOUND,m_PlaySpecificLevel.c_str());
          setState(GS_MENU);
          notifyMsg(cBuf);
//          throw Exception("no level");
        }
        else {    
          /* Start playing right away */     
          m_InputHandler.resetScriptKeyHooks();           
          m_MotoGame.playLevel( pLevelSrc );
          m_State = GS_PLAYING;        
          m_nFrame = 0;
          
          if(m_pReplay != NULL) delete m_pReplay;
          m_pReplay = NULL;
          
          if(m_bRecordReplays && !pLevelSrc->isScripted()) {
            m_pReplay = new Replay;
            m_pReplay->createReplay("Latest.rpl",pLevelSrc->getID(),m_pPlayer->PlayerName,m_fReplayFrameRate,sizeof(SerializedBikeState));
          }
          
          PlayerTimeEntry *pBestTime = m_Profiles.getBestTime(m_PlaySpecificLevel);
          PlayerTimeEntry *pBestPTime = m_Profiles.getBestPlayerTime(m_pPlayer->PlayerName,m_PlaySpecificLevel);
          
          std::string T1 = "--:--:--",T2 = "--:--:--";
          if(pBestTime != NULL)
            T1 = formatTime(pBestTime->fFinishTime);
          if(pBestPTime != NULL)
            T2 = formatTime(pBestPTime->fFinishTime);
          
          m_Renderer.setBestTime(T1 + std::string(" / ") + T2);

          /* World-record stuff */
          _UpdateWorldRecord(m_PlaySpecificLevel);

          /* Prepare level */
          m_Renderer.prepareForNewLevel();
        }
        break;
      }
      case GS_PAUSE: {
//        SDL_ShowCursor(SDL_ENABLE);
        m_bShowCursor = true;

        /* Paused from GS_PLAYING */
        break;
      }
      case GS_JUSTDEAD: {
//        SDL_ShowCursor(SDL_ENABLE);
        m_bShowCursor = true;
        
        /* Finish replay */
        if(m_pReplay != NULL) m_pReplay->finishReplay(false,0.0f);
                
        /* Play the DIE!!! sound */
        //Sound::playSample(m_pDieSFX);

        /* Possible exit of GS_PLAYING, when the player is dead */
        m_pJustDeadMenu->showWindow(true);
        m_nJustDeadShade = 0;
        m_fCoolDownEnd = getRealTime() + 0.3f;
        break;
      }
      case GS_EDIT_PROFILES: {
//        SDL_ShowCursor(SDL_ENABLE);
        m_bShowCursor = true;

        /* The profile editor can work on top of the main menu, or as init
           state when there is no player profiles available */
        m_pProfileEditor->showWindow(true);
        break;
      }
      case GS_FINISHED: {
//        SDL_ShowCursor(SDL_ENABLE);
        m_bShowCursor = true;

        /* Finish replay */
        if(m_pReplay != NULL) m_pReplay->finishReplay(true,m_MotoGame.getFinishTime());
        
        /* Play the good sound. */
        //Sound::playSample(m_pEndOfLevelSFX);

        /* A more lucky outcome of GS_PLAYING than GS_JUSTDEAD :) */
        m_pFinishMenu->showWindow(true);
        m_pBestTimes->showWindow(true);
        m_nFinishShade = 0;            
        break;
      }
    }
    
    m_fLastPhysTime = getTime() - PHYS_STEP_SIZE;
  }

  /*===========================================================================
  Pre-initialize game
  ===========================================================================*/
  void GameApp::userPreInit(void) {
    /* Config */
    _CreateDefaultConfig();
    m_Config.loadFile();
    //for(int i=0;i<m_Config.getVars().size();i++)
    //  printf(" CONFIG: %s=%s\n",m_Config.getVars()[i]->Name.c_str(),m_Config.getVars()[i]->Value.c_str());
  }
  
  /*===========================================================================
  Select display mode
  ===========================================================================*/
  void GameApp::selectDisplayMode(int *pnWidth,int *pnHeight,int *pnBPP,bool *pbWindowed) {
    if(!isCmdDispWidth() && !isCmdDispHeight()) {
      *pnWidth = m_Config.getInteger("DisplayWidth");
      *pnHeight = m_Config.getInteger("DisplayHeight");
    }
    
    if(!isCmdDispBPP()) {
      *pnBPP = m_Config.getInteger("DisplayBPP");
    }
    
    if(!isCmdDispWindowed()) {
      *pbWindowed = m_Config.getBool("DisplayWindowed");
    }
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
    if(s == "Low") m_Renderer.setQuality(GQ_LOW);
    else if(s == "Medium") m_Renderer.setQuality(GQ_MEDIUM);
    else if(s == "High") m_Renderer.setQuality(GQ_HIGH);
    
    /* Show mini map? */
    m_bShowMiniMap = m_Config.getBool("ShowMiniMap");
    
    /* Replay stuff */
    m_fReplayFrameRate = m_Config.getFloat("ReplayFrameRate");
    m_bRecordReplays = m_Config.getBool("StoreReplays");
    m_bCompressReplays = m_Config.getBool("CompressReplays");
    Replay::enableCompression(m_bCompressReplays);
    
    /* Other settings */
    m_bEnableEngineSound = m_Config.getBool("EngineSoundEnable");
    m_bEnableWebHighscores = m_Config.getBool("WebHighscores");
    m_bShowWebHighscoreInGame = m_Config.getBool("ShowInGameWorldRecord");
    m_bEnableContextHelp = m_Config.getBool("ContextHelp");
        
    /* Cache? */
    m_bEnableLevelCache = m_Config.getBool("LevelCache");
    
    /* Configure proxy */
    _ConfigureProxy();
  }
  
  /*===========================================================================
  Update loading screen
  ===========================================================================*/
  void GameApp::_UpdateLoadingScreen(float fDone,Texture *pLoadingScreen,const std::string &NextTask) {
    if(pLoadingScreen != NULL) {
      glClear(GL_COLOR_BUFFER_BIT);
      drawImage(Vector2f(getDispWidth()/2 - 256,getDispHeight()/2 - 40),
                Vector2f(getDispWidth()/2 + 256,getDispHeight()/2 + 40),
                pLoadingScreen,MAKE_COLOR(255,255,255,255));
      drawBox(Vector2f(getDispWidth()/2 + 256 - (512.0f*(1-fDone)),getDispHeight()/2 - 40),              
              Vector2f(getDispWidth()/2 + 256,getDispHeight()/2 - 25),              
              0,MAKE_COLOR(0,0,0,128));
      drawBox(Vector2f(getDispWidth()/2 + 256 - (512.0f*(1-fDone)),getDispHeight()/2 + 25),              
              Vector2f(getDispWidth()/2 + 256,getDispHeight()/2 + 40),              
              0,MAKE_COLOR(0,0,0,128));
             
      drawText(Vector2f(getDispWidth()/2 - 256,getDispHeight()/2 + 40 + 3),NextTask);
              
      SDL_GL_SwapBuffers();
    }
  }
  
  /*===========================================================================
  Initialize game
  ===========================================================================*/
  void GameApp::userInit(void) {
    SDL_ShowCursor(SDL_DISABLE);        
  
    /* Reset timers */
    m_fLastFrameTime = 0.0f;
    m_fLastPerfStateTime = 0.0f;
    m_fLastPhysTime = getTime() - PHYS_STEP_SIZE;
    
    /* And stuff */
    m_nPauseShade = 0;
    m_nJustDeadShade = 0;
    m_nFinishShade = 0;
    
    /* Init some config */
    _UpdateSettings();
    
    /* Profiles */
    Log("Loading profiles...");
    m_Profiles.loadFile();
    Log(" %d profile%s loaded",m_Profiles.getProfiles().size(),m_Profiles.getProfiles().size()==1?"":"s");

    /* Select profile */
    m_pPlayer = NULL;
    if(m_ForceProfile != "") {
      m_pPlayer = m_Profiles.getProfile(m_ForceProfile);
      if(m_pPlayer == NULL)
        Log("** Warning ** : unknown profile '%s'",m_ForceProfile.c_str());       
    }
    if(m_pPlayer == NULL)
      m_pPlayer = m_Profiles.getProfile(m_Config.getString("DefaultProfile"));
    if(m_pPlayer == NULL && !m_Profiles.getProfiles().empty()) {
      /* OK, use the first then */
      m_pPlayer = m_Profiles.getProfiles()[0];
    }

    /* List replays? */  
    if(m_bListReplays) {
      std::vector<ReplayInfo *> Replays = Replay::createReplayList("");
      printf("\nReplay                    Level                     Player\n");
      printf("-----------------------------------------------------------------------\n");
      for(int i=0;i<Replays.size();i++) {
        std::string LevelDesc;
        
        if(Replays[i]->Level.length() == 6 &&
           Replays[i]->Level[0] == '_' && Replays[i]->Level[1] == 'i' &&
           Replays[i]->Level[2] == 'L' && Replays[i]->Level[5] == '_') {
          int nNum;
          sscanf(Replays[i]->Level.c_str(),"_iL%d_",&nNum);
          char cBuf[256];
          sprintf(cBuf,"#%d",nNum+1);
          LevelDesc = cBuf;
        }
        else LevelDesc = Replays[i]->Level;
      
        printf("%-25s %-25s %-25s\n",
               Replays[i]->Name.c_str(),
               LevelDesc.c_str(),
               Replays[i]->Player.c_str());
      }
      if(Replays.empty()) printf("(none)\n");
      Replay::freeReplayList(Replays);
      quit();
      return;
    }

    
    /* Init sound system */
    if(!isNoGraphics()) {
      Log("Initializing sound system...");
      Sound::init(&m_Config);
      if(!Sound::isEnabled()) {
        Log(" (sound is disabled)\n");
      }    
    }
              
    /* Init renderer */
    m_Renderer.setParent( (App *)this );
    m_Renderer.setGameObject( &m_MotoGame );        
    m_Renderer.setDebug( m_bDebugMode );
    m_Renderer.setUglyMode( m_bUglyMode );
    
    /* Tell collision system whether we want debug-info or not */
    m_MotoGame.getCollisionHandler()->setDebug( m_bDebugMode );
    
    /* Data time! */
    Log("Loading data...");

    if(m_GraphDebugInfoFile != "") m_Renderer.loadDebugInfo(m_GraphDebugInfoFile);

    Texture *pLoadingScreen = NULL;
    if(!isNoGraphics()) {    
      /* Show loading screen */
      pLoadingScreen = TexMan.loadTexture("Textures/UI/Loading.png",false,true);
      _UpdateLoadingScreen((1.0f/9.0f) * 0,pLoadingScreen,GAMETEXT_LOADINGSOUNDS);
      
      if(Sound::isEnabled()) {
        /* Load sounds */
        //m_pEndOfLevelSFX = Sound::loadSample("Sounds/EndOfLevel.ogg");
        //m_pDieSFX = Sound::loadSample("Sounds/Die.ogg");
        
        Sound::loadSample("Sounds/Button1.ogg");
//        Sound::loadSample("Sounds/Button2.ogg");
        Sound::loadSample("Sounds/Button3.ogg");
        
        Sound::loadSample("Sounds/PickUpStrawberry.ogg");
        
        m_EngineSound.addBangSample(Sound::loadSample("Sounds/Engine/00.wav"));
        m_EngineSound.addBangSample(Sound::loadSample("Sounds/Engine/01.wav"));
        m_EngineSound.addBangSample(Sound::loadSample("Sounds/Engine/02.wav"));
        m_EngineSound.addBangSample(Sound::loadSample("Sounds/Engine/03.wav"));
        m_EngineSound.addBangSample(Sound::loadSample("Sounds/Engine/04.wav"));
        m_EngineSound.addBangSample(Sound::loadSample("Sounds/Engine/05.wav"));
        m_EngineSound.addBangSample(Sound::loadSample("Sounds/Engine/06.wav"));
        m_EngineSound.addBangSample(Sound::loadSample("Sounds/Engine/07.wav"));
        m_EngineSound.addBangSample(Sound::loadSample("Sounds/Engine/08.wav"));
        m_EngineSound.addBangSample(Sound::loadSample("Sounds/Engine/09.wav"));
        m_EngineSound.addBangSample(Sound::loadSample("Sounds/Engine/10.wav"));
        m_EngineSound.addBangSample(Sound::loadSample("Sounds/Engine/11.wav"));
        m_EngineSound.addBangSample(Sound::loadSample("Sounds/Engine/12.wav"));
        
        Log(" %d sound%s loaded",Sound::getNumSamples(),Sound::getNumSamples()==1?"":"s");
      }
      _UpdateLoadingScreen((1.0f/9.0f) * 1,pLoadingScreen,GAMETEXT_INITTEXT);
          
      /* Find all files in the textures dir and load them */
      TexMan.setDefaultTextureName("Dirt");
      
      UITextDraw::initTextDrawing(this);
      UITexture::setApp(this);

      _UpdateLoadingScreen((1.0f/9.0f) * 2,pLoadingScreen,GAMETEXT_LOADINGTEXTURES);
      
      std::vector<std::string> TextureFiles = FS::findPhysFiles("Textures/*.jpg");
      int nLoaded=0;
      for(int i=0;i<TextureFiles.size();i++) {
        /* Ignore .. and . (and makefiles) */
        if(TextureFiles[i].at(TextureFiles[i].length()-1) != '.' &&
          !FS::isDir(TextureFiles[i])) {
          if(TexMan.loadTexture(TextureFiles[i] ) != NULL)
            nLoaded++;
        }
      }
      Log(" %d texture%s loaded",nLoaded,nLoaded==1?"":"s");

      _UpdateLoadingScreen((1.0f/9.0f) * 3,pLoadingScreen,GAMETEXT_LOADINGMENUGRAPHICS);
        
      /* Check to see if we can find the default texture */
      if(TexMan.getTexture("default") == NULL)
        throw Exception("no valid default texture");
        
      /* Load title screen textures + cursor */
      m_pTitleBL = TexMan.loadTexture("Textures/UI/TitleBL.jpg",false,true);
      m_pTitleBR = TexMan.loadTexture("Textures/UI/TitleBR.jpg",false,true);
      m_pTitleTL = TexMan.loadTexture("Textures/UI/TitleTL.jpg",false,true);
      m_pTitleTR = TexMan.loadTexture("Textures/UI/TitleTR.jpg",false,true);
      
      m_pCursor = TexMan.loadTexture("Textures/UI/Cursor.png",false,true,true);

      _UpdateLoadingScreen((1.0f/9.0f) * 4,pLoadingScreen,GAMETEXT_LOADINGLEVELS);
    }
    
    /* Test level cache directory */
    if(m_bEnableLevelCache && !FS::isDir("LCache")) {
      m_bEnableLevelCache = false;
      Log("** Warning ** : Level cache directory not found, forcing caching off!");
    }
     
    /* Find all .lvl files in the level dir and load them */    
    m_nNumLevels = 0;
    std::vector<std::string> LvlFiles = FS::findPhysFiles("Levels/*.lvl",true);
    int nNumCached = _LoadLevels(LvlFiles);
    
    /* -listlevels? */
    if(m_bListLevels) {
      for(int i=0;i<m_nNumLevels;i++) {          
        printf("%-25s %-25s %-25s\n",FS::getFileBaseName(m_Levels[i].getFileName()).c_str(),m_Levels[i].getID().c_str(),m_Levels[i].getLevelInfo()->Name.c_str());
      }
    }
    _UpdateLoadingScreen((1.0f/9.0f) * 5,pLoadingScreen,GAMETEXT_INITRENDERER);
    
    if(m_bListLevels) {
      quit();
      return;
    }
    
    Log(" %d level%s loaded (%d from cache)",m_nNumLevels,m_nNumLevels==1?"":"s",nNumCached);
    Log(" %d level pack%s",m_LevelPacks.size(),m_LevelPacks.size()==1?"":"s");

    #if defined(SUPPORT_WEBACCESS)    
      /* Fetch highscores from web? */
      if(m_bEnableWebHighscores) {            
        _UpdateLoadingScreen((1.0f/9.0f) * 6,pLoadingScreen,GAMETEXT_DLHIGHSCORES);      
        
        m_pWebHighscores = new WebHighscores(&m_ProxySettings);
        
        /* Try downloading the highscores */
        try {
          m_pWebHighscores->update();
        }
        catch(Exception &e) {
          /* No internet connection, probably... (just use the latest ones, if any) */
          Log("** Warning ** : Failed to update web-highscores [%s]",e.getMsg().c_str());        
        }
        
        /* Upgrade high scores */
        m_pWebHighscores->upgrade();      
      }
    #endif
        
    if(!isNoGraphics()) {
      /* Initialize renderer */
      m_Renderer.init();
      _UpdateLoadingScreen((1.0f/9.0f) * 7,pLoadingScreen,GAMETEXT_INITMENUS);
      
      /* Bonus info */
      Log("Total: %d kB of textures loaded",TexMan.getTextureUsage()/1024);

      /* Initialize menu system */
      _InitMenus();    
      _UpdateLoadingScreen((1.0f/9.0f) * 8,pLoadingScreen,GAMETEXT_UPDATINGLEVELS);

      _UpdateLevelLists();
      _UpdateLoadingScreen((1.0f/9.0f) * 9,pLoadingScreen,GAMETEXT_INITINPUT);      
      
      /* Init input system */
      m_InputHandler.init(&m_Config);
    }
        
    /* What to do? */
    if(m_PlaySpecificLevel != "" && !isNoGraphics()) {
      /* ======= PLAY SPECIFIC LEVEL ======= */
      m_StateAfterPlaying = GS_MENU;
      setState(GS_PLAYING);
      Log("Playing as '%s'...",m_pPlayer->PlayerName.c_str());
    }
    else if(m_PlaySpecificReplay != "") {
      /* ======= PLAY SPECIFIC REPLAY ======= */
      m_StateAfterPlaying = GS_MENU;
      setState(GS_REPLAYING);
    }
    else {
      /* Graphics? */
      if(isNoGraphics())
        throw Exception("menu requires graphics");
        
      /* Do we have a player profile? */
      if(m_pPlayer == NULL) {
        setState(GS_EDIT_PROFILES);
      }
      else {
        /* Enter the menu */
        setState(GS_MENU);
      }
    }            
  }

  /*===========================================================================
  Load some levels...
  ===========================================================================*/
  int GameApp::_LoadLevels(const std::vector<std::string> &LvlFiles) {
    int nNumCached = 0;
  
    for(int i=0;i<LvlFiles.size();i++) {
      int j = m_nNumLevels;
      if(j >= 2048) {
        Log("** Warning ** : Too many levels.");
        break;
      }
      m_nNumLevels++;
    
      m_Levels[j].setFileName( LvlFiles[i] );

      /* Cache or not to cache? */
      if(m_bEnableLevelCache) {
        /* Start by determining file CRC */
        LevelCheckSum Sum;
        m_Levels[j].probeCheckSum(&Sum);
        
        /* Determine name in cache */
        std::string LevelFileBaseName = FS::getFileBaseName(LvlFiles[i]);
        char cCacheFileName[1024];      
        sprintf(cCacheFileName,"LCache/%08x%s.blv",Sum.nCRC32,LevelFileBaseName.c_str());
                  
        /* Got level in cache? */
        if(!m_Levels[j].importBinary(cCacheFileName,&Sum)) {
          /* Not in cache, buggers. Load it from (slow) XML then. */
          m_Levels[j].loadXML();
          
          /* Cache it now */
          m_Levels[j].exportBinary(cCacheFileName,&Sum);
        }
        else nNumCached++;
      }
      else {
        /* Just load it */
        m_Levels[j].loadXML();       
      }
          
      /* Update level pack manager */
      _UpdateLevelPackManager(&m_Levels[j]);
    }
    
    return nNumCached;
  }
  
  /*===========================================================================
  Draw menu/title screen background
  ===========================================================================*/
  void GameApp::_DrawMenuBackground(void) {
    if(m_MenuBackgroundGraphics != MENU_GFX_OFF) {
      if(m_pTitleTL != NULL)
        drawImage(Vector2f(0,0),Vector2f(getDispWidth()/2,getDispHeight()/2),m_pTitleTL);
      if(m_pTitleTR != NULL)
        drawImage(Vector2f(getDispWidth()/2,0),Vector2f(getDispWidth(),getDispHeight()/2),m_pTitleTR);
      if(m_pTitleBR != NULL)
        drawImage(Vector2f(getDispWidth()/2,getDispHeight()/2),Vector2f(getDispWidth(),getDispHeight()),m_pTitleBR);
      if(m_pTitleBL != NULL)
        drawImage(Vector2f(0,getDispHeight()/2),Vector2f(getDispWidth()/2,getDispHeight()),m_pTitleBL);
    }
  }

  /*===========================================================================
  GUI mouse hover
  ===========================================================================*/
  void GameApp::_DispatchMouseHover(void) {
    int nX,nY;
    getMousePos(&nX,&nY);
    m_Renderer.getGUI()->mouseHover(nX,nY);
  }
  
  /*===========================================================================
  Draw frame
  ===========================================================================*/
  void GameApp::drawFrame(void) {
    char cTemp[256];
    bool bValidGameState = true;
        
    /* Per default, don't wait between frames */
    setFrameDelay(0);

    /* Update sound system and input */
    if(!isNoGraphics()) {
      m_EngineSound.update(getRealTime());
      m_EngineSound.setRPM(0); /* per default don't have engine sound */
      Sound::update();
      
      m_InputHandler.updateInput(m_MotoGame.getBikeController());
    }    
    
    /* Whether or not we should have a mouse cursor? */
    switch(m_State) {
      case GS_MENU:
      case GS_EDIT_PROFILES:
      case GS_LEVEL_INFO_VIEWER:
      case GS_PAUSE:
      case GS_JUSTDEAD:
      case GS_FINISHED:
        m_bShowCursor = true;
        //SDL_ShowCursor(SDL_ENABLE);
        break;

      case GS_PLAYING:
      case GS_REPLAYING:
        m_bShowCursor = false;
        //SDL_ShowCursor(SDL_DISABLE);      
        break;
    }
  
    /* Quit msg box open? */
    if(m_pQuitMsgBox != NULL) {
      UIMsgBoxButton Button = m_pQuitMsgBox->getClicked();
      if(Button == UI_MSGBOX_YES) {
        quit();      
        delete m_pQuitMsgBox;
        m_pQuitMsgBox = NULL;
        return;
      }
      else if(Button == UI_MSGBOX_NO) {
        delete m_pQuitMsgBox;
        m_pQuitMsgBox = NULL;
      }  
    }
    /* What about the notify box then? */
    else if(m_pNotifyMsgBox != NULL) {
      if(m_pNotifyMsgBox->getClicked() == UI_MSGBOX_OK) {
        delete m_pNotifyMsgBox;
        m_pNotifyMsgBox = NULL;
      }
    }
    /* And the download-levels box? */
    else if(m_pDownloadLevelsMsgBox != NULL) {
      UIMsgBoxButton Button = m_pDownloadLevelsMsgBox->getClicked();
      if(Button == UI_MSGBOX_YES) {
        /* Download levels! */
        _DownloadExtraLevels();

        delete m_pDownloadLevelsMsgBox;
        m_pDownloadLevelsMsgBox = NULL;
      }
      else if(Button == UI_MSGBOX_NO) {
        delete m_pDownloadLevelsMsgBox;
        m_pDownloadLevelsMsgBox = NULL;
      }
    }
    
    /* Perform a rather precise calculation of the frame rate */    
    double fFrameTime = getRealTime();
    static int nFPS_Frames = 0;
    static double fFPS_LastTime = 0.0f;
    static double fFPS_CurrentTime = 0.0f;
    static float fFPS_Rate = 0.0f;
    fFPS_CurrentTime = getRealTime();
    if(fFPS_CurrentTime - fFPS_LastTime > 1.0f && nFPS_Frames>0) {
      fFPS_Rate = ((float)nFPS_Frames) / (fFPS_CurrentTime - fFPS_LastTime);
      nFPS_Frames = 0;
      fFPS_LastTime = fFPS_CurrentTime;
    }
    nFPS_Frames++;
    
    /* Current time? */
    int nADelay = 0;    
    
    m_fLastFrameTime = fFrameTime; /* FIXME unsused*/
    
    /* What state? */
    switch(m_State) {
      case GS_MENU:
      case GS_LEVEL_INFO_VIEWER:
      case GS_LEVELPACK_VIEWER:
      case GS_EDIT_PROFILES: {
        /* Draw menu background */
        _DrawMenuBackground();
                
        /* Update mouse stuff */
        _DispatchMouseHover();
        
        /* Blah... */
        if(m_State == GS_MENU)
          _HandleMainMenu();
        else if(m_State == GS_EDIT_PROFILES)
          _HandleProfileEditor();
        else if(m_State == GS_LEVEL_INFO_VIEWER)
          _HandleLevelInfoViewer();
        else if(m_State == GS_LEVELPACK_VIEWER)
          _HandleLevelPackViewer();
                  
        /* Draw GUI */
        if(m_bEnableContextHelp)
          m_Renderer.getGUI()->enableContextMenuDrawing(true);
        else
          m_Renderer.getGUI()->enableContextMenuDrawing(false);
          
        m_Renderer.getGUI()->paint();                
        
        /* Show frame rate */
        if(m_bShowFrameRate) {
          sprintf(cTemp,"%f",fFPS_Rate);
          drawText(Vector2f(130,0),cTemp);
        }                

        /* Delay a bit so we don't eat all CPU */
        setFrameDelay(10);
        break;
      }
      case GS_PAUSE:
      case GS_JUSTDEAD:
      case GS_FINISHED:
      case GS_REPLAYING:
      case GS_PLAYING: {
        /* When did the frame start? */
        double fStartFrameTime = getTime();
        int nPhysSteps = 0;

        /* Only do this when not paused */
        if(m_State == GS_PLAYING) {
          /* Increase frame counter */
          m_nFrame++;

          /* Following time code is made by Eric Piel, but I took the liberty to change the minimum
             frame-miss number from 50 to 10, because it wasn't working well. */
             
      	  /* reinitialise if we can't catch up */
      	  if (m_fLastPhysTime - getTime() < -0.1f)
      	    m_fLastPhysTime = getTime() - PHYS_STEP_SIZE;
      
          /* Update game until we've catched up with the real time */
      	  do {
                  m_MotoGame.updateLevel( PHYS_STEP_SIZE,NULL,m_pReplay );
      	    m_fLastPhysTime += PHYS_STEP_SIZE;
      	    nPhysSteps++;
      	    /* don't do this infinitely, maximum miss 10 frames, then give up */
      	  } while ((m_fLastPhysTime + PHYS_STEP_SIZE <= getTime()) && (nPhysSteps < 10));
        	
//      	  printf("%d ",nPhysSteps);
      	  m_Renderer.setSpeedMultiplier(nPhysSteps);
          
      	  if(!m_bTimeDemo) {
      	    /* Never pass this point while being ahead of time, busy wait until it's time */
      	    if (nPhysSteps <= 1) {      	    
      	      while (m_fLastPhysTime > getTime());
      	    }
      	  }
                    
          if(m_bEnableEngineSound) {
            /* Update engine RPM */
            m_EngineSound.setRPM( m_MotoGame.getBikeEngineRPM() ); 
          }
          
          /* We'd like to serialize the game state 25 times per second for the replay */
          if(getRealTime() - m_fLastStateSerializationTime >= 1.0f/m_fReplayFrameRate) {
            m_fLastStateSerializationTime = getRealTime();
            
            /* Get it */
            SerializedBikeState BikeState;
            m_MotoGame.getSerializedBikeState(&BikeState);
            if(m_pReplay != NULL)
              m_pReplay->storeState((const char *)&BikeState);              
          }
        }
        else if(m_State == GS_REPLAYING) {
          m_nFrame++;

          /* Read replay state */
          static SerializedBikeState BikeState;          
          if(m_pReplay != NULL) {       
            /* Even frame number: Read the next state */
            if(m_nFrame%2 || m_nFrame==1) {       
              /* REAL NON-INTERPOLATED FRAME */    
              if(m_pReplay->loadState((char *)&BikeState)) {            
                /* Update game */
                m_MotoGame.updateLevel( PHYS_STEP_SIZE,&BikeState,m_pReplay ); 

                if(m_bEnableEngineSound) {
                  /* Update engine RPM */
                  m_EngineSound.setRPM( m_MotoGame.getBikeEngineRPM() ); 
                }
              }
              else {
                if(m_pReplay->didFinish()) {
                  /* Make sure that it's the same finish time */
                  m_MotoGame.setTime(m_pReplay->getFinishTime());
                }
              }
            }
            else {                          
              /* INTERPOLATED FRAME */
              SerializedBikeState NextBikeState,ibs;
              if(m_pReplay->peekState((char *)&NextBikeState)) {
                /* Nice. Interpolate the states! */
                m_MotoGame.interpolateGameState(&BikeState,&NextBikeState,&ibs,0.5f);

                /* Update game */
                m_MotoGame.updateLevel( PHYS_STEP_SIZE,&ibs,m_pReplay );                 
              }
            }
            
            /* Benchmarking and finished? If so, print the report and quit */
            if(m_pReplay->endOfFile() && m_bBenchmark) {
              double fBenchmarkTime = getRealTime() - m_fStartTime;
              printf("\n");
              printf(" * %d frames rendered in %.0f seconds\n",m_nFrame,fBenchmarkTime);
              printf(" * Average framerate: %.1f fps\n",((double)m_nFrame) / fBenchmarkTime);
              quit();
            }
          }
          else bValidGameState = false;
        }
                
        /* Render */
        if(!isNoGraphics() && bValidGameState) {
          m_Renderer.render();
        
          if(m_bShowMiniMap) {
            if(m_MotoGame.getBikeState()->Dir == DD_LEFT)
              m_Renderer.renderMiniMap(getDispWidth()-150,getDispHeight()-100,150,100);
            else if(m_MotoGame.getBikeState()->Dir == DD_RIGHT)
              m_Renderer.renderMiniMap(0,getDispHeight()-100,150,100);
          }              
        }
                
        /* Also only when not paused */
        if(m_State == GS_PLAYING) {        
          /* News? */
          if(m_MotoGame.isDead()) {
            /* You're dead maan! */
            setState(GS_JUSTDEAD);
          }
          else if(m_MotoGame.isFinished()) {
            /* You're done maaaan! :D */
            std::string TimeStamp = getTimeStamp();
            m_Profiles.addFinishTime(m_pPlayer->PlayerName,"",
                                     m_MotoGame.getLevelSrc()->getID(),m_MotoGame.getFinishTime(),TimeStamp); 
            _MakeBestTimesWindow(m_pBestTimes,m_pPlayer->PlayerName,m_MotoGame.getLevelSrc()->getID(),
                                 m_MotoGame.getFinishTime(),TimeStamp);
            _UpdateLevelLists();
            setState(GS_FINISHED);
          }
        }
        
        /* When did it end? */
        double fEndFrameTime = getTime();
        
        /* Delay */
        if(m_State == GS_REPLAYING) {
          /* When replaying... */
          nADelay = ((1.0f/m_fCurrentReplayFrameRate - (fEndFrameTime-fStartFrameTime)) * 1000.0f) * 0.5f;
        }
      	else if ((m_State == GS_FINISHED) || (m_State == GS_JUSTDEAD) || (m_State == GS_PAUSE)) {
      	  //SDL_Delay(10);
      	  setFrameDelay(10);
      	}
        else {
      	  /* become idle only if we hadn't to skip any frame, recently, and more globaly (80% of fps) */
          if ((nPhysSteps <= 1) && (fFPS_Rate > (0.8f / PHYS_STEP_SIZE)))
          nADelay = ((m_fLastPhysTime + PHYS_STEP_SIZE) - fEndFrameTime) * 1000.0f;
        }
                
        if(nADelay > 0) {
          if(!m_bTimeDemo) {
            //SDL_Delay(nADelay);
            setFrameDelay(nADelay);
      	  }
        }        

        /* Show fps (debug modish) */
        if(m_bDebugMode) {
          static char cBuf[256] = ""; 
          static int nFrameCnt = 0;
          if(fFrameTime - m_fLastPerfStateTime > 0.5f) {
            float f = fFrameTime - m_fLastPerfStateTime;
            sprintf(cBuf,"%.1f",((float)nFrameCnt)/f);
            nFrameCnt = 0;
            m_fLastPerfStateTime = fFrameTime;
          }
          drawText(Vector2f(0,100),cBuf,MAKE_COLOR(0,0,0,255),-1);        
          nFrameCnt++;
        }
        
        if(m_State == GS_PAUSE) {
          /* Okay, nifty thing. Paused! */
          if(m_nPauseShade < 150) m_nPauseShade+=8;
          drawBox(Vector2f(0,0),Vector2f(getDispWidth(),getDispHeight()),0,MAKE_COLOR(0,0,0,m_nPauseShade));                                        

          /* Update mouse stuff */
          _DispatchMouseHover();
          
          /* Blah... */
          _HandlePauseMenu();
        }        
        else if(m_State == GS_JUSTDEAD) {
          /* Hmm, you're dead and you know it. */
          if(m_nJustDeadShade < 150) m_nJustDeadShade+=8;
          drawBox(Vector2f(0,0),Vector2f(getDispWidth(),getDispHeight()),0,MAKE_COLOR(0,0,0,m_nJustDeadShade));     

          /* Update mouse stuff */
          _DispatchMouseHover();
          
          if(getRealTime() > m_fCoolDownEnd) {
            /* Blah... */
            _HandleJustDeadMenu();
          }
        }        
        else if(m_State == GS_FINISHED) {
          /* Hmm, you've won and you know it. */
          if(m_nFinishShade < 150) m_nFinishShade+=8;
          drawBox(Vector2f(0,0),Vector2f(getDispWidth(),getDispHeight()),0,MAKE_COLOR(0,0,0,m_nFinishShade));     

          /* Update mouse stuff */
          _DispatchMouseHover();
          
          /* Blah... */
          _HandleFinishMenu();
        }        
        
        /* Level name to draw? */
        if(m_State == GS_JUSTDEAD || m_State == GS_PAUSE || m_State == GS_FINISHED &&
           m_MotoGame.getLevelSrc() != NULL) {
          UITextDraw::printRaw(m_Renderer.getMediumFont(),0,getDispHeight()-4,m_MotoGame.getLevelSrc()->getLevelInfo()->Name,
                               MAKE_COLOR(255,255,255,255));
        }
        
        /* Context menu? */
        if(m_State == GS_PLAYING || m_State == GS_REPLAYING || !m_bEnableContextHelp)
          m_Renderer.getGUI()->enableContextMenuDrawing(false);
        else
          m_Renderer.getGUI()->enableContextMenuDrawing(true);
        
        /* Draw GUI */
        m_Renderer.getGUI()->paint();        

        /* Show frame rate */
        if(m_bShowFrameRate) {
          sprintf(cTemp,"%f",fFPS_Rate);
          drawText(Vector2f(130,0),cTemp);
        }
        break;
      }
    }    
    
    /* Draw mouse cursor */
    if(!isNoGraphics() && m_bShowCursor) {
      int nMX,nMY;
      getMousePos(&nMX,&nMY);      
      drawImage(Vector2f(nMX-2,nMY-2),Vector2f(nMX+30,nMY+30),m_pCursor);
    }
  }

  /*===========================================================================
  Shutdown game
  ===========================================================================*/
  void GameApp::userShutdown(void) {  
    #if defined(SUPPORT_WEBACCESS)
      if(m_pWebHighscores != NULL)
        delete m_pWebHighscores;
        
      if(m_pWebLevels != NULL)
        delete m_pWebLevels;
    #endif
  
    for(int i=0;i<m_LevelPacks.size();i++) {
      delete m_LevelPacks[i];
    }
    m_LevelPacks.clear();
  
    if(!isNoGraphics()) {
      m_Renderer.unprepareForNewLevel(); /* just to be sure, shutdown can happen quite hard */
      
      m_InputHandler.uninit();
    }
    
    if(m_pReplay != NULL)
      delete m_pReplay;
  
    if(m_pPlayer != NULL) 
      m_Config.setString("DefaultProfile",m_pPlayer->PlayerName);

    Sound::uninit();

    m_Config.saveFile();
    m_Profiles.saveFile();

    if(!isNoGraphics()) {
      UITextDraw::uninitTextDrawing();  
    
      /* Kill textures */
      TexMan.unloadTextures();
    }
  }

  /*===========================================================================
  Screenshooting
  ===========================================================================*/
  void GameApp::_GameScreenshot(void) {
    Img *pShot = grabScreen();      
    FileHandle *pfh;
    char cBuf[256];
    int nShot=0;
    
    /* User preference for format? must be either jpeg or png */
    std::string ShotExtension = m_Config.getString("ScreenshotFormat");
    if(ShotExtension != "jpeg" && ShotExtension != "jpg" && ShotExtension != "png") {
      Log("** Warning ** : unsupported screenshot format '%s', using png instead!",ShotExtension.c_str());
      ShotExtension = "png";
    }    
    
    while(1) {
      sprintf(cBuf,"screenshot%04d.%s",nShot,ShotExtension.c_str());
      nShot++;
      if(nShot > 9999) {
        Log("Too many screenshots!");
        return;
      }
      pfh = FS::openIFile(cBuf);
      if(pfh == NULL) break;
      else FS::closeFile(pfh);
    }
    pShot->saveFile(cBuf);
    delete pShot;
  }

  /*===========================================================================
  Key down event
  ===========================================================================*/
  void GameApp::keyDown(int nKey,int nChar) {
    /* No matter what, F12 always equals a screenshot */
    if(nKey == SDLK_F12) {
      _GameScreenshot();
      return;        
    }
    
    /* If message box... */
    if(m_pQuitMsgBox) {
      if(nKey == SDLK_ESCAPE) {
        delete m_pQuitMsgBox;
        m_pQuitMsgBox = NULL;
      }    
      else
        m_Renderer.getGUI()->keyDown(nKey,nChar);      
      return;
    }
    else if(m_pNotifyMsgBox) {
      if(nKey == SDLK_ESCAPE) {
        delete m_pNotifyMsgBox;
        m_pNotifyMsgBox = NULL;
      }    
      else
        m_Renderer.getGUI()->keyDown(nKey,nChar);      
      return;
    }
  
    /* What state? */
    switch(m_State) {
      case GS_EDIT_PROFILES:
      case GS_LEVEL_INFO_VIEWER:
      case GS_LEVELPACK_VIEWER:
      case GS_MENU: {
        /* The GUI wants to know about keypresses... */
        m_Renderer.getGUI()->keyDown(nKey,nChar);
        break;
      }
      case GS_PAUSE:
        switch(nKey) {
          case SDLK_ESCAPE:
            /* Back to the game, please */
            m_pPauseMenu->showWindow(false);
            m_State = GS_PLAYING;
            break;
          default:
            m_Renderer.getGUI()->keyDown(nKey,nChar);
            break;      
        }
        break;
      case GS_FINISHED:
      case GS_JUSTDEAD:
        switch(nKey) {
          case SDLK_ESCAPE:
            if(m_pSaveReplayMsgBox == NULL) {          
              /* Out of this game, please */
              m_pFinishMenu->showWindow(false);
              m_pBestTimes->showWindow(false);
              m_pJustDeadMenu->showWindow(false);
              m_MotoGame.endLevel();
              m_InputHandler.resetScriptKeyHooks();                         
              m_Renderer.unprepareForNewLevel();
              //setState(GS_MENU);
              setState(m_StateAfterPlaying);
            }
            else {
              if(m_State == GS_JUSTDEAD)
                if(getRealTime() < m_fCoolDownEnd)
                  break;
               
              m_Renderer.getGUI()->keyDown(nKey,nChar);
            }
            break;
          default:
            if(m_State == GS_JUSTDEAD)
              if(getRealTime() < m_fCoolDownEnd)
                break;
             
            m_Renderer.getGUI()->keyDown(nKey,nChar);
            break;      
        }
        break;
      case GS_REPLAYING:
        switch(nKey) {
          case SDLK_ESCAPE:
            /* Escape quits the replay */
            m_MotoGame.endLevel();
            m_InputHandler.resetScriptKeyHooks();                      
            m_Renderer.unprepareForNewLevel();
            setState(GS_MENU);            
            break;          
          case SDLK_RIGHT:
            /* Right arrow key: fast forward */
            if(m_pReplay != NULL)
              m_pReplay->fastforward(1);
            break;
          case SDLK_LEFT:
            /* Left arrow key: rewind */
            if(m_pReplay != NULL)
              m_pReplay->fastrewind(1);
            break;
        }
        break;
      case GS_PLAYING:
        switch(nKey) {
	case SDLK_ESCAPE:
	  /* Escape pauses */
	  setState(GS_PAUSE);
	  m_pPauseMenu->showWindow(true);
	  m_nPauseShade = 0;
	  break;
          default:
            /* Notify the controller */
            m_InputHandler.handleInput(INPUT_KEY_DOWN,nKey,m_MotoGame.getBikeController(), &m_Renderer);
        }
        break; 
    }
  }

  /*===========================================================================
  Key up event
  ===========================================================================*/
  void GameApp::keyUp(int nKey) {
    /* What state? */
    switch(m_State) {
      case GS_EDIT_PROFILES:
      case GS_LEVEL_INFO_VIEWER:
      case GS_FINISHED:
      case GS_JUSTDEAD:
      case GS_LEVELPACK_VIEWER:
      case GS_MENU:
        m_Renderer.getGUI()->keyUp(nKey);
        break;
      case GS_PLAYING:
        /* Notify the controller */
        m_InputHandler.handleInput(INPUT_KEY_UP,nKey,m_MotoGame.getBikeController(), &m_Renderer);
        break; 
    }
  }

  /*===========================================================================
  Mouse events
  ===========================================================================*/
  void GameApp::mouseDoubleClick(int nButton) {
    switch(m_State) {
      case GS_MENU:
      case GS_PAUSE:
      case GS_JUSTDEAD:
      case GS_FINISHED:
      case GS_EDIT_PROFILES:
      case GS_LEVEL_INFO_VIEWER:
      case GS_LEVELPACK_VIEWER:
        int nX,nY;        
        getMousePos(&nX,&nY);
        
        if(nButton == SDL_BUTTON_LEFT)
          m_Renderer.getGUI()->mouseLDoubleClick(nX,nY);
        
        break;
    }
  }

  void GameApp::mouseDown(int nButton) {
    switch(m_State) {
      case GS_MENU:
      case GS_PAUSE:
      case GS_JUSTDEAD:
      case GS_FINISHED:
      case GS_EDIT_PROFILES:
      case GS_LEVEL_INFO_VIEWER:
      case GS_LEVELPACK_VIEWER:
        int nX,nY;        
        getMousePos(&nX,&nY);
        
        if(nButton == SDL_BUTTON_LEFT)
          m_Renderer.getGUI()->mouseLDown(nX,nY);
        else if(nButton == SDL_BUTTON_RIGHT)
          m_Renderer.getGUI()->mouseRDown(nX,nY);
        else if(nButton == SDL_BUTTON_WHEELUP)
          m_Renderer.getGUI()->mouseWheelUp(nX,nY);
        else if(nButton == SDL_BUTTON_WHEELDOWN)        
          m_Renderer.getGUI()->mouseWheelDown(nX,nY);
        
        break;
    }
  }

  void GameApp::mouseUp(int nButton) {
    switch(m_State) {
      case GS_MENU:
      case GS_PAUSE:
      case GS_JUSTDEAD:
      case GS_FINISHED:
      case GS_EDIT_PROFILES:
      case GS_LEVEL_INFO_VIEWER:
      case GS_LEVELPACK_VIEWER:
        int nX,nY;
        getMousePos(&nX,&nY);
        
        if(nButton = SDL_BUTTON_LEFT)
          m_Renderer.getGUI()->mouseLUp(nX,nY);
        else if(nButton = SDL_BUTTON_RIGHT)
          m_Renderer.getGUI()->mouseRUp(nX,nY);
        break;
    }
  }

  /*===========================================================================
  Handle a command-line passed argument
  ===========================================================================*/
  void GameApp::parseUserArgs(std::vector<std::string> &UserArgs) {
    /* Look through them... */
    for(int i=0;i<UserArgs.size();i++) {
      if(UserArgs[i] == "-replay") {
        if(i+1<UserArgs.size()) {
          m_PlaySpecificReplay = UserArgs[i+1];
        }
        else
          throw SyntaxError("no replay specified");        
        i++;
      }
      else if(UserArgs[i] == "-level") {
        if(i+1<UserArgs.size()) {
          m_PlaySpecificLevel = UserArgs[i+1];
          
          /* If it is a plain number, it's for a internal level */
          int nNum = atoi(m_PlaySpecificLevel.c_str());
          if(nNum > 0) {
            char cBuf[256];
            sprintf(cBuf,"_iL%02d_",nNum-1);
            m_PlaySpecificLevel = cBuf;
          }
        }
        else
          throw SyntaxError("no level specified");        
        i++;
      }
      else if(UserArgs[i] == "-debug") {
        m_bDebugMode = true;
      }
      else if(UserArgs[i] == "-profile") {
        if(i+1<UserArgs.size())
          m_ForceProfile = UserArgs[++i];
        else
          throw SyntaxError("no profile specified");        
        i++;
      }
      else if(UserArgs[i] == "-gdebug") {
        if(i+1<UserArgs.size())
          m_GraphDebugInfoFile = UserArgs[++i];
        else
          throw SyntaxError("no debug file specified");        
        i++;
      }      
      else if(UserArgs[i] == "-listlevels") {
        m_bListLevels = true;
        setNoGraphics(true);
      }
      else if(UserArgs[i] == "-listreplays") {
        m_bListReplays = true;
        setNoGraphics(true);
      }
      else if(UserArgs[i] == "-timedemo") {
        m_bTimeDemo = true;
      }
      else if(UserArgs[i] == "-fps") {
        m_bShowFrameRate = true;
      }
      else if(UserArgs[i] == "-ugly") {
				m_bUglyMode = true;
      }
      else if(UserArgs[i] == "-benchmark") {
				m_bBenchmark = true;
      }
    }
  }

  /*===========================================================================
  Show some extra user arg help
  ===========================================================================*/
  void GameApp::helpUserArgs(void) {
    printf("\t-level ID\n\t\tStart playing the given level right away.\n");
    printf("\t-replay NAME\n\t\tPlayback replay with the given name.\n");    
    printf("\t-debug\n\t\tEnable debug mode.\n");
    printf("\t-profile NAME\n\t\tUse this player profile.\n");
    printf("\t-listlevels\n\t\tOutputs a list of all installed levels.\n");
    printf("\t-listreplays\n\t\tOutputs a list of all replays.\n");
    printf("\t-timedemo\n\t\tNo delaying, maximum framerate.\n");
    printf("\t-fps\n\t\tDisplay framerate.\n");
    printf("\t-ugly\n\t\tEnable 'ugly' mode, suitable for computers without\n");
                   printf("\t\ta good OpenGL-enabled video card.\n");
    printf("\t-benchmark\n\t\tOnly meaningful when combined with -replay and\n");
                   printf("\t\t-timedemo. Useful to determine the graphics\n");
                   printf("\t\tperformance.\n");
  }  
    
  /*===========================================================================
  Find a level by ID
  ===========================================================================*/
  LevelSrc *GameApp::_FindLevelByID(std::string ID) {
    /* Look through all level sources... */
    for(int i=0;i<m_nNumLevels;i++) {
      if(m_Levels[i].getID() == ID) return &m_Levels[i];
    }
    return NULL; /* nothing */
  }
    
  /*===========================================================================
  Create the default config
  ===========================================================================*/
  void GameApp::_CreateDefaultConfig(void) {
    /* Display */
    m_Config.createVar( "DisplayWidth",           "800" );
    m_Config.createVar( "DisplayHeight",          "600" );
    m_Config.createVar( "DisplayBPP",             "32" );
    m_Config.createVar( "DisplayWindowed",        "false" );
    m_Config.createVar( "MenuBackgroundGraphics", "High" );
    m_Config.createVar( "GameGraphics",           "High" );
        
    /* Audio */
    m_Config.createVar( "AudioEnable",            "true" );
    m_Config.createVar( "AudioSampleRate",        "22050" );
    m_Config.createVar( "AudioSampleBits",        "16" );
    m_Config.createVar( "AudioChannels",          "Mono" );
    m_Config.createVar( "EngineSoundEnable",      "true" );

    /* Controls */
    m_Config.createVar( "ControllerMode1",        "Keyboard" );
    m_Config.createVar( "KeyDrive1",              "Up" );
    m_Config.createVar( "KeyBrake1",              "Down" );
    m_Config.createVar( "KeyFlipLeft1",           "Left" );
    m_Config.createVar( "KeyFlipRight1",          "Right" );
    m_Config.createVar( "KeyChangeDir1",          "Space" );
    
    #if defined(ENABLE_ZOOMING)
      m_Config.createVar( "KeyZoomIn",              "PageUp" );
      m_Config.createVar( "KeyZoomOut",             "PageDown" );
    #endif
        
    m_Config.createVar( "JoyIdx1",                "-1" );
    m_Config.createVar( "JoyAxisPrim1",           "" );
    m_Config.createVar( "JoyAxisPrimMax1",        "" );
    m_Config.createVar( "JoyAxisPrimMin1",        "" );
    m_Config.createVar( "JoyAxisPrimUL1",         "" );
    m_Config.createVar( "JoyAxisPrimLL1",         "" );
    m_Config.createVar( "JoyButtonFlipLeft1",     "" );
    m_Config.createVar( "JoyButtonFlipRight1",    "" );
    m_Config.createVar( "JoyButtonChangeDir1",    "" );
    
    m_Config.createVar( "JoystickLimboArea",      "0.07" );

    /* Misc */
    m_Config.createVar( "DefaultProfile",         "" );
    m_Config.createVar( "ScreenshotFormat",       "png" );
    m_Config.createVar( "NotifyAtInit",           "true" );
    m_Config.createVar( "ShowMiniMap",            "true" );
    m_Config.createVar( "StoreReplays",           "true" );
    m_Config.createVar( "ReplayFrameRate",        "25" );
    m_Config.createVar( "CompressReplays",        "true" );
    m_Config.createVar( "LevelCache",             "true" );
    m_Config.createVar( "WebHighscores",          "true" );
    m_Config.createVar( "ShowInGameWorldRecord",  "false" );
    m_Config.createVar( "ContextHelp",            "true" );
    
    /* Proxy */
    m_Config.createVar( "ProxyType",              "" ); /* (blank), HTTP, SOCKS4, or SOCKS5 */
    m_Config.createVar( "ProxyServer",            "" ); /* (may include user/pass and port) */
    m_Config.createVar( "ProxyPort",              "-1" );
    //m_Config.createVar( "ProxyAuthUser",          "" ); 
    //m_Config.createVar( "ProxyAuthPwd",          "" );
    
  }
  
  /*===========================================================================
  Notification popup
  ===========================================================================*/
  void GameApp::notifyMsg(std::string Msg) {
    if(m_pNotifyMsgBox != NULL) delete m_pNotifyMsgBox;
    m_pNotifyMsgBox = m_Renderer.getGUI()->msgBox(Msg,(UIMsgBoxButton)(UI_MSGBOX_OK));
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
    if(!FS::copyFile("Replays/Latest.rpl",std::string("Replays/") + RealName + std::string(".rpl"))) {
      Log("** Warning ** : Failed to save replay: %s",Name.c_str());
      notifyMsg(GAMETEXT_FAILEDTOSAVEREPLAY);
    }
  }

  /*===========================================================================
  Level packs
  ===========================================================================*/
  void GameApp::_UpdateLevelPackManager(LevelSrc *pLevelSrc) {
    if(pLevelSrc->getLevelPack() != "") {
      /* Already a known level pack? */
      LevelPack *pPack = _FindLevelPackByName(pLevelSrc->getLevelPack());
      if(pPack == NULL) {
        /* No, register it */
        pPack = new LevelPack;
        pPack->Name = pLevelSrc->getLevelPack();
        m_LevelPacks.push_back(pPack);
        
        /* Set default hints */
        pPack->bShowTimes = true;
        pPack->bShowWebTimes = true;
        
        /* Try to find a hints file for this level pack */
        std::vector<std::string> LpkFiles = FS::findPhysFiles("Levels/*.lpk",true);
        for(int i=0;i<LpkFiles.size();i++) {
          XMLDocument XML; 
          XML.readFromFile(LpkFiles[i]);
          TiXmlDocument *pDoc = XML.getLowLevelAccess();
          
          if(pDoc != NULL) {
            TiXmlElement *pLpkHintsElem = pDoc->FirstChildElement("lpkhints");
            if(pLpkHintsElem != NULL) {
              /* For this level pack? */
              const char *pcFor = pLpkHintsElem->Attribute("for");
              if(pcFor != NULL && pPack->Name == pcFor) {
                /* Yup. Extract hints */
                for(TiXmlElement *pHintElem = pLpkHintsElem->FirstChildElement("hint");
                    pHintElem != NULL; pHintElem=pHintElem->NextSiblingElement("hint")) {
                  /* Check for known hints... */
                  const char *pc;

                  pc = pHintElem->Attribute("show_times");
                  if(pc != NULL) {
                    pPack->bShowTimes = (bool)atoi(pc);
                  }

                  pc = pHintElem->Attribute("show_wtimes");
                  if(pc != NULL) {
                    pPack->bShowWebTimes = (bool)atoi(pc);
                  }
                }
              }              
            }
          }
        }
      }
      
      /* Add level to pack */
      pPack->Levels.push_back(pLevelSrc);
    }
  }

  LevelPack *GameApp::_FindLevelPackByName(const std::string &Name) {
    /* Look for level pack in list */
    for(int i=0;i<m_LevelPacks.size();i++) {
      if(m_LevelPacks[i]->Name == Name) return m_LevelPacks[i];
    }
    return NULL;
  }
 
  std::string GameApp::_DetermineNextLevel(LevelSrc *pLevelSrc) {
    int i;
    bool v_found;
    int v_current_level;

    /* Look through all level sources... */
    i = 0;
    v_found = false;
    while(v_found == false && i<m_nNumLevels) {
      if(m_Levels[i].getID() == pLevelSrc->getID()) {
	v_found = true;
	v_current_level = i;
      } else {
	i++;
      }
    }

    /* if not found */
    if(v_found == false) {
      return "";
    }

    /* determine current level type */
    bool isCurrentPack = pLevelSrc->getLevelPack() != "";

    /* case of pack */
    if(isCurrentPack) {
      LevelPack *pPack = _FindLevelPackByName(pLevelSrc->getLevelPack());
      if(pPack != NULL) {
	/* Determine next then */
	for(int j=0; j<pPack->Levels.size()-1; j++) {
	  if(pPack->Levels[j] == pLevelSrc) {
	    return pPack->Levels[j+1]->getID();
	  }
	}
      }
      return "";
    }

    /* determine current level type */
    bool isCurrentInternal = m_Profiles.isInternal(pLevelSrc->getID());
    
    /* find the next one */
    i++; // from the current one
    while(i<m_nNumLevels) {
      bool isInternal = m_Profiles.isInternal(m_Levels[i].getID());

      /* case of internal level */
      if(isCurrentInternal && isInternal) {
	return m_Levels[i].getID();
      }
     
      /* case of external */
      if(isCurrentInternal == false && isInternal == false) {
	return m_Levels[i].getID();
      }
      
      i++;
    }
    return "";
  }
  
  bool GameApp::_IsThereANextLevel(LevelSrc *pLevelSrc) {
    return _DetermineNextLevel(pLevelSrc) != "";
  }  
 
  /*===========================================================================
  World records
  ===========================================================================*/
  std::string GameApp::_FixHighscoreTime(const std::string &s) {
    char cTime[256];
    int n1=0,n2=0,n3=0;
      
    sscanf(s.c_str(),"%d:%d:%d",&n1,&n2,&n3);
    sprintf(cTime,"%02d:%02d:%02d",n1,n2,n3);
    
    return cTime;
  }
  
  void GameApp::_UpdateWorldRecord(const std::string &LevelID) {  
    m_Renderer.setWorldRecordTime("");
    
    #if defined(SUPPORT_WEBACCESS)
      if(m_bShowWebHighscoreInGame && m_pWebHighscores!=NULL) {
        WebHighscore *pWebHS = m_pWebHighscores->getHighscoreFromLevel(LevelID);
        if(pWebHS != NULL) {        
          m_Renderer.setWorldRecordTime(pWebHS->getRoom()->getRoomName() + ": " + 
                                        _FixHighscoreTime(pWebHS->getTime()) + 
                                        std::string(" (") + pWebHS->getPlayerName() + std::string(")"));
        } 
        else {
          m_Renderer.setWorldRecordTime(pWebHS->getRoom()->getRoomName() + ": "  GAMETEXT_NONE);      
        }                
      }
    #endif
  }
  
  /*===========================================================================
  Extra WWW levels
  ===========================================================================*/
  void GameApp::_DownloadExtraLevels(void) {
    #if defined(SUPPORT_WEBACCESS)
      /* Download extra levels */
      if(m_pWebLevels != NULL) {
        _SimpleMessage("Downloading extra levels...\nPress ESC to abort.\n\n ",&m_DownloadLevelsMsgBoxRect);

        try {                  
          Log("WWW: Downloading levels...");
          clearCancelAsSoonAsPossible();
          m_pWebLevels->upgrade();          
        } 
        catch(Exception &e) {
          Log("** Warning ** : Unable to check for extra levels [%s]",e.getMsg().c_str());
        }      

        /* Got some new levels... load them! */
        const std::vector<std::string> LvlFiles = m_pWebLevels->getDownloadedLevels();
        
        Log("Loading new levels...");
        int nOldNum = m_nNumLevels;
        _LoadLevels(LvlFiles);
        Log(" %d new level%s loaded",m_nNumLevels-nOldNum,(m_nNumLevels-nOldNum)==1?"":"s");
        
        /* Update level lists */
        _UpdateLevelLists();
      }            
    #endif
  }

  void GameApp::_CheckForExtraLevels(void) {
    #if defined(SUPPORT_WEBACCESS)
      /* Check for extra levels */
      try {
        _SimpleMessage("Checking for new levels...");
      
        if(m_pWebLevels != NULL) delete m_pWebLevels;
        m_pWebLevels = new WebLevels(this,&m_ProxySettings);
        
        Log("WWW: Checking for new levels...");
        clearCancelAsSoonAsPossible();
        m_pWebLevels->update();     
        int nULevels=0,nUBytes=0;
        m_pWebLevels->getUpdateInfo(&nUBytes,&nULevels);
        Log("WWW: %d new level%s found",nULevels,nULevels==1?"":"s");

        if(nULevels == 0) {
          notifyMsg("No new levels found.\n\nTry again another time.");
        }        
        else {
          /* Ask user whether he want to download levels or snot */
          if(m_pDownloadLevelsMsgBox == NULL) {
            char cBuf[256];
            sprintf(cBuf,"%d new level%s found. Download %s?",nULevels,nULevels==1?"":"s",nULevels==1?"it":"them");
            m_pDownloadLevelsMsgBox = m_Renderer.getGUI()->msgBox(cBuf,(UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO));
          }
        }
      } 
      catch(Exception &e) {
        Log("** Warning ** : Unable to check for extra levels [%s]",e.getMsg().c_str());
      }      
    #endif
  }
  
  /*===========================================================================
  WWWAppInterface implementation
  ===========================================================================*/
  #if defined(SUPPORT_WEBACCESS)
    
  void GameApp::setTaskProgress(float fPercent) {
    int nBarHeight = 15;
    
    drawBox(Vector2f(m_DownloadLevelsMsgBoxRect.nX+10,m_DownloadLevelsMsgBoxRect.nY+
                                                   m_DownloadLevelsMsgBoxRect.nHeight-
                                                   nBarHeight*2),
            Vector2f(m_DownloadLevelsMsgBoxRect.nX+m_DownloadLevelsMsgBoxRect.nWidth-10,
                     m_DownloadLevelsMsgBoxRect.nY+m_DownloadLevelsMsgBoxRect.nHeight-nBarHeight),
            0,MAKE_COLOR(0,0,0,255),0);
            
                
    drawBox(Vector2f(m_DownloadLevelsMsgBoxRect.nX+10,m_DownloadLevelsMsgBoxRect.nY+
                                                   m_DownloadLevelsMsgBoxRect.nHeight-
                                                   nBarHeight*2),
            Vector2f(m_DownloadLevelsMsgBoxRect.nX+10+((m_DownloadLevelsMsgBoxRect.nWidth-20)*(int)fPercent)/100,
                     m_DownloadLevelsMsgBoxRect.nY+m_DownloadLevelsMsgBoxRect.nHeight-nBarHeight),
            0,MAKE_COLOR(255,0,0,255),0);

    SDL_GL_SwapBuffers();            
  }
  
  void GameApp::setBeingDownloadedLevel(const std::string &LevelName) {
    drawText(Vector2f(0,0),LevelName + std::string("(Temp. message, make it nicer... kthxbye.)                     "),MAKE_COLOR(0,0,0,255));
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
  
  bool GameApp::doesLevelExist(const std::string &LevelID) {
    return _FindLevelByID(LevelID) != NULL;
  }
  
  #endif

  /*===========================================================================
  Configure proxy
  ===========================================================================*/
  void GameApp::_ConfigureProxy(void) {
  #if defined(SUPPORT_WEBACCESS)  
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
      m_ProxySettings.setPort(m_Config.getInteger("ProxyPort"));
      m_ProxySettings.setServer(m_Config.getString("ProxyServer"));      
    }
  #endif
  }
    
  
};


