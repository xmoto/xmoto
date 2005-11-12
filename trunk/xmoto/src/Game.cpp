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
 *  Game application.
 */
#include "Game.h"
#include "VFileIO.h"
#include "Sound.h"
#include "PhysSettings.h"

namespace vapp {

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
    
    switch(s) {
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
            throw Exception("unknown level specified by replay");
          }
          else {    
            /* Init level */                
            m_MotoGame.playLevel( pLevelSrc );
            m_nFrame = 0;
            m_Renderer.prepareForNewLevel();
            
            /* Reconstruct game events that are going to happen during the replay */
            m_MotoGame.unserializeGameEvents( *m_pReplay );            
            
            /* Show help string */
            if(!isNoGraphics()) {
              PlayerTimeEntry *pBestTime = m_Profiles.getBestTime(LevelID);
              PlayerTimeEntry *pBestPTime = m_Profiles.getBestPlayerTime(m_pPlayer->PlayerName,LevelID);
              
              std::string T1 = "  --  ",T2 = "  --  ";
              if(pBestTime != NULL)
                T1 = formatTime(pBestTime->fFinishTime);
              if(pBestPTime != NULL)
                T2 = formatTime(pBestPTime->fFinishTime);
              
              m_Renderer.setBestTime(T1 + std::string(" / ") + T2 + std::string(GAMETEXT_REPLAYHELPTEXT));
            }
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
          throw Exception("no level");
        }
        else {    
          /* Start playing right away */                
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
          
          std::string T1 = "  --  ",T2 = "  --  ";
          if(pBestTime != NULL)
            T1 = formatTime(pBestTime->fFinishTime);
          if(pBestPTime != NULL)
            T2 = formatTime(pBestPTime->fFinishTime);
          
          m_Renderer.setBestTime(T1 + std::string(" / ") + T2);
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
  }
  
  /*===========================================================================
  Update loading screen
  ===========================================================================*/
  void GameApp::_UpdateLoadingScreen(float fDone,Texture *pLoadingScreen) {
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
    
    /* Data time! */
    Log("Loading data...");

    if(m_GraphDebugInfoFile != "") m_Renderer.loadDebugInfo(m_GraphDebugInfoFile);

    Texture *pLoadingScreen = NULL;
    if(!isNoGraphics()) {    
      /* Show loading screen */
      pLoadingScreen = TexMan.loadTexture("Textures/UI/Loading.png",false,true);
      _UpdateLoadingScreen((1.0f/8.0f) * 0,pLoadingScreen);
      
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
      _UpdateLoadingScreen((1.0f/8.0f) * 1,pLoadingScreen);
          
      /* Find all files in the textures dir and load them */
      TexMan.setDefaultTextureName("Dirt");
      
      UITextDraw::initTextDrawing(this);
      UITexture::setApp(this);

      _UpdateLoadingScreen((1.0f/8.0f) * 2,pLoadingScreen);
      
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

      _UpdateLoadingScreen((1.0f/8.0f) * 3,pLoadingScreen);
        
      /* Check to see if we can find the default texture */
      if(TexMan.getTexture("default") == NULL)
        throw Exception("no valid default texture");
        
      /* Load title screen textures + cursor */
      m_pTitleBL = TexMan.loadTexture("Textures/UI/TitleBL.jpg",false,true);
      m_pTitleBR = TexMan.loadTexture("Textures/UI/TitleBR.jpg",false,true);
      m_pTitleTL = TexMan.loadTexture("Textures/UI/TitleTL.jpg",false,true);
      m_pTitleTR = TexMan.loadTexture("Textures/UI/TitleTR.jpg",false,true);
      
      m_pCursor = TexMan.loadTexture("Textures/UI/Cursor.png",false,true,true);

      _UpdateLoadingScreen((1.0f/8.0f) * 4,pLoadingScreen);
    }
     
    /* Find all .lvl files in the level dir and load them */
    std::vector<std::string> LvlFiles = FS::findPhysFiles("Levels/*.lvl");
    for(int i=0;i<LvlFiles.size();i++) {
      /* Load it */
      m_Levels[i].setFileName( LvlFiles[i] );
      m_Levels[i].loadXML();            
      
      /* Output? */
      if(m_bListLevels) {
        printf("%-25s %-25s %-25s\n",FS::getFileBaseName(m_Levels[i].getFileName()).c_str(),m_Levels[i].getID().c_str(),m_Levels[i].getLevelInfo()->Name.c_str());
      }
    }
    m_nNumLevels = LvlFiles.size();
    _UpdateLoadingScreen((1.0f/8.0f) * 5,pLoadingScreen);
    
    if(m_bListLevels) {
      quit();
      return;
    }
    
    Log(" %d level%s loaded",m_nNumLevels,m_nNumLevels==1?"":"s");
    
    if(!isNoGraphics()) {
      /* Initialize renderer */
      m_Renderer.init();
      _UpdateLoadingScreen((1.0f/8.0f) * 6,pLoadingScreen);
      
      /* Bonus info */
      Log("Total: %d kB of textures loaded",TexMan.getTextureUsage()/1024);

      /* Initialize menu system */
      _InitMenus();    
      _UpdateLoadingScreen((1.0f/8.0f) * 7,pLoadingScreen);

      _UpdateLevelLists();
      _UpdateLoadingScreen((1.0f/8.0f) * 8,pLoadingScreen);      
      
      /* Init input system */
      m_InputHandler.init(&m_Config);
    }
        
    /* What to do? */
    if(m_PlaySpecificLevel != "" && !isNoGraphics()) {
      /* ======= PLAY SPECIFIC LEVEL ======= */
      setState(GS_PLAYING);
      Log("Playing as '%s'...",m_pPlayer->PlayerName.c_str());
    }
    else if(m_PlaySpecificReplay != "") {
      /* ======= PLAY SPECIFIC REPLAY ======= */
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

    /* Update sound system and input */
    if(!isNoGraphics()) {
      m_EngineSound.update(getRealTime());
      m_EngineSound.setRPM(0); /* per default don't have engien sound */
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
    
    /* Perform a rather precise calculation of the frame rate */
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
    static int nADelay = 0;
    static int nFCount = 0;
    double fFrameTime = getRealTime();
    double fFrameRenderingTime = fFrameTime - m_fLastFrameTime;
    double fFramesPerSecond = 1.0f / fFrameRenderingTime;
    m_fLastFrameTime = fFrameTime;
    
    if(fFramesPerSecond < 90) {
      if(nFCount < 0) nFCount = 0;
      nFCount++;
    }
    else {
      if(nFCount > 0) nFCount = 0;
      nFCount--;
    }
    
    if(!isNoGraphics()) {
      if(!m_b50FpsMode && nFCount > 100) {
        m_b50FpsMode = true;
        m_Renderer.setSpeedMultiplier(2);   
        //printf("entering 50 fps!\n");
      }
      else if(m_b50FpsMode && nFCount < -100) {
        m_b50FpsMode = false;
        m_Renderer.setSpeedMultiplier(1);   
        //printf("entering 100 fps!\n");
      }
    }
    /* What state? */
    switch(m_State) {
      case GS_MENU:
      case GS_LEVEL_INFO_VIEWER:
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
                  
        /* Draw GUI */
        m_Renderer.getGUI()->paint();                
        
        /* Show frame rate */
        if(m_bShowFrameRate) {
          sprintf(cTemp,"%f",fFPS_Rate);
          drawText(Vector2f(100,0),cTemp);
        }

        /* Delay a bit so we don't eat all CPU */
        SDL_Delay(5);
        break;
      }
      case GS_PAUSE:
      case GS_JUSTDEAD:
      case GS_FINISHED:
      case GS_REPLAYING:
      case GS_PLAYING: {
        /* When did the frame start? */
        float fStartFrameTime = getTime();

        /* Only do this when not paused */
        if(m_State == GS_PLAYING) {
          /* Increase frame counter */
          m_nFrame++;
                    
          /* Update game */
          m_MotoGame.updateLevel( PHYS_STEP_SIZE,NULL,m_pReplay );                
          
          if(m_b50FpsMode) /* if we're aiming for 50 fps instead of 100, do an extra step now */
            m_MotoGame.updateLevel( PHYS_STEP_SIZE,NULL,m_pReplay );      

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
        float fEndFrameTime = getTime();
        
        /* Delay */
        if(m_State == GS_REPLAYING) {
          /* When replaying... */
          nADelay = ((1.0f/m_fCurrentReplayFrameRate - (fEndFrameTime-fStartFrameTime)) * 1000.0f) * 0.5f;
                      
          if(nADelay > 0) {
            if(!m_bTimeDemo)
              SDL_Delay(nADelay);
          }
        }
        else {
          if(m_b50FpsMode)
            nADelay = (2.0f * PHYS_STEP_SIZE - (fEndFrameTime-fStartFrameTime)) * 1000.0f;
          else
            nADelay = (PHYS_STEP_SIZE - (fEndFrameTime-fStartFrameTime)) * 1000.0f;
            
          if(nADelay > 0) {
            if(!m_bTimeDemo)
              SDL_Delay(nADelay);
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
        
        /* Draw GUI */
        m_Renderer.getGUI()->paint();        

        /* Show frame rate */
        if(m_bShowFrameRate) {
          sprintf(cTemp,"%f",fFPS_Rate);
          drawText(Vector2f(100,0),cTemp);
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
              m_Renderer.unprepareForNewLevel();
              setState(GS_MENU);
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
            m_InputHandler.handleInput(INPUT_KEY_DOWN,nKey,m_MotoGame.getBikeController());
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
      case GS_MENU:
        m_Renderer.getGUI()->keyUp(nKey);
        break;
      case GS_PLAYING:
        /* Notify the controller */
        m_InputHandler.handleInput(INPUT_KEY_UP,nKey,m_MotoGame.getBikeController());
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
  
};

