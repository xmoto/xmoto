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
 *  Game application. (init-related stuff)
 */
 
/* rneckelmann 2006-09-30: moved a lot of stuff from Game.cpp into here to 
                           make it a tad smaller */ 
#include "Game.h"
#include "VFileIO.h"
#include "Sound.h"
#include "PhysSettings.h"
#include "Input.h"

#if defined(SUPPORT_WEBACCESS)
  #include <curl/curl.h>
#endif

#define CURRENT_LEVEL_INDEX_FILE_VERSION 1

namespace vapp {

  /*===========================================================================
  Pre-initialize game
  ===========================================================================*/
  void GameApp::userPreInit(void) {
    /* Config */
    _CreateDefaultConfig();
    m_Config.loadFile();
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
    Sprite* pSprite;

    switchUglyMode(m_bUglyMode);
    switchTestThemeMode(m_bTestThemeMode);

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
    
    /* load theme */
    m_themeChoicer = new ThemeChoicer(
#if defined(SUPPORT_WEBACCESS)
              this,
              &m_ProxySettings
#endif
              );
    try {
      m_theme.load(m_themeChoicer->getFileName(getConfigThemeName(m_themeChoicer)));
    } catch(Exception &e) {
      /* unable to load the theme, load the default one */
      m_theme.load(m_themeChoicer->getFileName(THEME_DEFAULT_THEMENAME));
    }

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
    
    /* Update stats */
    m_GameStats.loadXML("stats.xml");
    if(m_pPlayer != NULL)
      m_GameStats.xmotoStarted(m_pPlayer->PlayerName);
    
    /* Update replays */
    m_ReplayList.initFromDir();
    
    /* List replays? */  
    if(m_bListReplays) {
      std::vector<ReplayInfo *> *Replays = m_ReplayList.findReplays();
      printf("\nReplay                    Level                     Player\n");
      printf("-----------------------------------------------------------------------\n");
      for(int i=0;i<Replays->size();i++) {
        std::string LevelDesc;
        
        if((*Replays)[i]->Level.length() == 6 &&
           (*Replays)[i]->Level[0] == '_' && (*Replays)[i]->Level[1] == 'i' &&
           (*Replays)[i]->Level[2] == 'L' && (*Replays)[i]->Level[5] == '_') {
          int nNum;
          sscanf((*Replays)[i]->Level.c_str(),"_iL%d_",&nNum);
          char cBuf[256];
          sprintf(cBuf,"#%d",nNum+1);
          LevelDesc = cBuf;
        }
        else LevelDesc = (*Replays)[i]->Level;
      
        printf("%-25s %-25s %-25s\n",
               (*Replays)[i]->Name.c_str(),
               LevelDesc.c_str(),
               (*Replays)[i]->Player.c_str());
      }
      if(Replays->empty()) printf("(none)\n");
      delete Replays;
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

#if defined(ALLOW_GHOST)
    m_Renderer.setGhostMotionBlur( m_bGhostMotionBlur );
#endif
    
    /* Tell collision system whether we want debug-info or not */
    m_MotoGame.getCollisionHandler()->setDebug( m_bDebugMode );
    
    /* Data time! */
    Log("Loading data...");

    if(m_GraphDebugInfoFile != "") m_Renderer.loadDebugInfo(m_GraphDebugInfoFile);

    Texture *pLoadingScreen = NULL;
    if(!isNoGraphics()) {    
      /* Show loading screen */
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "Loading");

      if(pSprite != NULL) {
  pLoadingScreen = pSprite->getTexture(false, true);
      }

      _UpdateLoadingScreen((1.0f/9.0f) * 0,pLoadingScreen,GAMETEXT_LOADINGSOUNDS);

      if(Sound::isEnabled()) {
        /* Load sounds */
        Sound::loadSample("Sounds/NewHighscore.ogg");
        //m_pDieSFX = Sound::loadSample("Sounds/Die.ogg");
        
        Sound::loadSample("Sounds/Button1.ogg");
//        Sound::loadSample("Sounds/Button2.ogg");
        Sound::loadSample("Sounds/Button3.ogg");
        
        Sound::loadSample("Sounds/PickUpStrawberry.ogg");        
        Sound::loadSample("Sounds/Headcrash.ogg");
        //Sound::loadSample("Sounds/Squeek.ogg");

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
      UITextDraw::initTextDrawing(this);
      UITexture::setApp(this);

      _UpdateLoadingScreen((1.0f/9.0f) * 3,pLoadingScreen,GAMETEXT_LOADINGMENUGRAPHICS);
        
      /* Load title screen textures + cursor + stuff */
      m_pTitleBL = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "TitleBL");
      if(pSprite != NULL) {
        m_pTitleBL = pSprite->getTexture(false, true);
      }

      m_pTitleBR = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "TitleBR");
      if(pSprite != NULL) {
        m_pTitleBR = pSprite->getTexture(false, true);
      }

      m_pTitleTL = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "TitleTL");
      if(pSprite != NULL) {
        m_pTitleTL = pSprite->getTexture(false, true);
      }

      m_pTitleTR = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "TitleTR");
      if(pSprite != NULL) {
        m_pTitleTR = pSprite->getTexture(false, true);
      }

      m_pCursor = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "Cursor");
      if(pSprite != NULL) {
        m_pCursor = pSprite->getTexture(false, true, FM_LINEAR);
      }

#if defined(SUPPORT_WEBACCESS)  
      m_pNewLevelsAvailIcon = NULL;
      pSprite = m_theme.getSprite(SPRITE_TYPE_UI, "NewLevelsAvailable");
      if(pSprite != NULL) {
        m_pNewLevelsAvailIcon = pSprite->getTexture(false, true, FM_LINEAR);
      }
#endif

      _UpdateLoadingScreen((1.0f/9.0f) * 4,pLoadingScreen,GAMETEXT_LOADINGLEVELS);
    }
    
    /* Test level cache directory */
    std::string LCachePath = FS::getUserDir() + std::string("/LCache");
    if(m_bEnableLevelCache && !FS::isDir(LCachePath)) {
      try {
	FS::mkArborescence(LCachePath);
      } catch(Exception &e) {
	m_bEnableLevelCache = false;
	Log("** Warning ** : Level cache directory can't be created, forcing caching off!");
      }
    }
    
    /* Should we clean the level cache? (can also be done when disabled) */
    if(m_bCleanCache) {
      /* Find all .blv-files in the directory */
      std::vector<std::string> BlvFiles = FS::findPhysFiles("LCache/*.blv");
      Log("Trying to clean %d files from level cache...",BlvFiles.size());
      int nNumDeleted = 0;
      for(int i=0;i<BlvFiles.size();i++) {
        if(FS::deleteFile(BlvFiles[i]))
          nNumDeleted++;
      }
      Log(" %d file%s deleted succesfully",nNumDeleted,nNumDeleted==1?"":"s");
    }
     
    /* Find all .lvl files in the level dir and load them */    
    try {
      loadLevelsFromIndex();
    } catch(Exception &e) {
      Log(std::string("Unable to load levels from index:\n" + e.getMsg()).c_str());
      loadLevelsFromFiles(true);
    }
    
    /* -listlevels? */
    if(m_bListLevels) {
      for(int i=0;i<m_nNumLevels;i++) {          
        printf("%-25s %-25s %-25s\n",FS::getFileBaseName(m_Levels[i].FileName()).c_str(),m_Levels[i].Id().c_str(),m_Levels[i].Name().c_str());
      }
    }

    _UpdateLoadingScreen((1.0f/9.0f) * 5,pLoadingScreen,GAMETEXT_INITRENDERER);
    
    if(m_bListLevels) {
      quit();
      return;
    }
    
    #if defined(SUPPORT_WEBACCESS)    
      /* Fetch highscores from web? */
      if(m_pWebHighscores != NULL) delete m_pWebHighscores;
      m_pWebHighscores = new WebRoom(&m_ProxySettings);      
      m_pWebHighscores->setWebsiteURL(m_Config.getString("WebHighscoresURL"));
      
      if(m_bEnableWebHighscores) {  
  bool bSilent = true;
          
  try {
    if(m_bEnableCheckHighscoresAtStartup) {
      _UpdateLoadingScreen((1.0f/9.0f) * 6,pLoadingScreen,GAMETEXT_DLHIGHSCORES);      
      _UpdateWebHighscores(bSilent);
    }
    if(m_bEnableCheckNewLevelsAtStartup) {
      _UpdateLoadingScreen((1.0f/9.0f) * 6,pLoadingScreen,GAMETEXT_DLLEVELSCHECK);      
      _UpdateWebLevels(bSilent);       
    }
  } catch(Exception &e) {
    /* No internet connection, probably... (just use the latest times, if any) */
    Log("** Warning ** : Failed to update web-highscores [%s]",e.getMsg().c_str());              
    if(!bSilent)
      notifyMsg(GAMETEXT_FAILEDDLHIGHSCORES);
  }
      }

      _UpgradeWebHighscores();

    if(m_pWebRooms != NULL) delete m_pWebRooms;
    m_pWebRooms = new WebRooms(&m_ProxySettings);
    _UpgradeWebRooms(false);

    #endif
        
    if(!isNoGraphics()) {
      /* Initialize renderer */
      m_Renderer.init();
      _UpdateLoadingScreen((1.0f/9.0f) * 7,pLoadingScreen,GAMETEXT_INITMENUS);
      
      /* Initialize menu system */
      _InitMenus();    
      _UpdateLoadingScreen((1.0f/9.0f) * 8,pLoadingScreen,GAMETEXT_UPDATINGLEVELS);

      _UpdateLevelsLists();
      _UpdateLoadingScreen((1.0f/9.0f) * 9,pLoadingScreen,GAMETEXT_INITINPUT);      
      
      /* Init input system */
      m_InputHandler.init(&m_Config);
    }
        
    /* What to do? */
    if(m_PlaySpecificLevel != "" && !isNoGraphics()) {
      /* ======= PLAY SPECIFIC LEVEL ======= */
      m_StateAfterPlaying = GS_MENU;
      setState(GS_PREPLAYING);
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
#if defined(SUPPORT_WEBACCESS)
      else if(m_Config.getBool("WebConfAtInit")) {
        /* We need web-config */
        _InitWebConf();
        setState(GS_EDIT_WEBCONFIG);
      }
#endif
      else {
        /* Enter the menu */
        setState(GS_MENU);
      }
    }            
  }
  
  void GameApp::loadLevelsFromFiles(bool bSilent) {
    if(!bSilent) {
      _SimpleMessage(GAMETEXT_RELOADINGLEVELS, &m_DownloadMsgBoxRect);
    }

    m_nNumLevels = 0;
    std::vector<std::string> LvlFiles = FS::findPhysFiles("Levels/*.lvl", true);
    loadLevelsFromLvl(LvlFiles);
    try {
      /* then, recreate the index */
      createLevelsIndex();
    } catch(Exception &e2) {
      Log((std::string("Unable to create the level index:\n") + e2.getMsg()).c_str());
    }
  }

  std::string GameApp::LevelIndexFileName() {
    return vapp::FS::getUserDir() + "/" + "LCache/levels.index";
  }

  void GameApp::loadLevelsFromIndex() {
    int v_nbLevels;
    m_nNumLevels = 0;

    vapp::FileHandle *pfh = vapp::FS::openIFile(LevelIndexFileName());
    if(pfh == NULL) {
      throw Exception((std::string("Unable to open file ") + LevelIndexFileName()).c_str());
    }

    try {
      int v_version = vapp::FS::readInt_LE(pfh); /* version */
      if(v_version != CURRENT_LEVEL_INDEX_FILE_VERSION) {
	throw Exception("Invalid level index file version");
      }
      v_nbLevels = vapp::FS::readInt_LE(pfh);
      for(int i=0; i<v_nbLevels; i++) {
	m_Levels[i].setFileName(vapp::FS::readString(pfh));
	m_Levels[i].importBinaryHeader(pfh);
	m_nNumLevels++;
      }

    } catch(Exception &e) {
      m_nNumLevels = 0;
      vapp::FS::closeFile(pfh);
      throw e;
    }
    vapp::FS::closeFile(pfh);
  }
   
  void GameApp::createLevelsIndex() {
    /* for windows : your must remove the file before create it */
    remove(LevelIndexFileName().c_str());

    vapp::FileHandle *pfh = vapp::FS::openOFile(LevelIndexFileName());
    if(pfh == NULL) {
      throw Exception((std::string("Unable to open file ") + LevelIndexFileName()).c_str());
      return;
    }

    try {
      vapp::FS::writeInt_LE(pfh, CURRENT_LEVEL_INDEX_FILE_VERSION); /* version */
      vapp::FS::writeInt_LE(pfh, m_nNumLevels);
      for(int i=0; i<m_nNumLevels; i++) {
	vapp::FS::writeString(pfh, m_Levels[i].FileName());
	m_Levels[i].exportBinaryHeader(pfh);
      }
    } catch(Exception &e) {
      vapp::FS::closeFile(pfh);
      throw e;
    }

    vapp::FS::closeFile(pfh);
  }

  /*===========================================================================
  Load some levels...
  ===========================================================================*/
  void GameApp::loadLevelsFromLvl(const std::vector<std::string> &LvlFiles) {
    for(int i=0;i<LvlFiles.size();i++) {    
      int j = m_nNumLevels;
      if(j >= 2048) {
        Log("** Warning ** : Too many levels.");
        break;
      }
    
      bool bCached = false;
      try {
        // Load the level
        m_Levels[j].setFileName( LvlFiles[i] );
        bCached = m_Levels[j].loadReducedFromFile(m_bEnableLevelCache);
        
        // Check for ID conflict
        for(int k=0;k<m_nNumLevels;k++) {
          if(m_Levels[k].Id() == m_Levels[j].Id()) {
            /* Conflict! */
            Log("** Warning ** : More than one level with ID '%s'!",m_Levels[k].Id().c_str());
            Log("                (%s)",m_Levels[j].FileName().c_str());
            Log("                (%s)",m_Levels[k].FileName().c_str());
            if(bCached) Log("                (cached)");
            throw Exception("Duplicate level ID");
          }
        }
        
        m_nNumLevels++;
      
      } catch(Exception &e) {
        Log("** Warning ** : Problem loading '%s' (%s)",
          LvlFiles[i].c_str(),e.getMsg().c_str());            
      }
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

      if(m_pWebRooms != NULL)
      delete m_pWebRooms;  
    #endif
  
    if(m_pCredits != NULL)
      delete m_pCredits;
  
      destroyLevelsPacks();
    
    m_GameStats.saveXML("stats.xml");
      
    if(!isNoGraphics()) {
      m_Renderer.unprepareForNewLevel(); /* just to be sure, shutdown can happen quite hard */
      m_Renderer.shutdown();
      m_InputHandler.uninit();
    }

    delete m_themeChoicer;
    
    if(m_pReplay != NULL)
      delete m_pReplay;

#if defined(ALLOW_GHOST)
      if(m_pGhostReplay != NULL)
      delete m_pGhostReplay;
#endif  

    if(m_pPlayer != NULL) 
      m_Config.setString("DefaultProfile",m_pPlayer->PlayerName);

    if(m_pMenuMusic != NULL) {
      Mix_FreeMusic(m_pMenuMusic);
    }

    Sound::uninit();

    m_Config.saveFile();
    m_Profiles.saveFile();

    if(!isNoGraphics()) {
      UITextDraw::uninitTextDrawing();  
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
    bool v_isANumber =  true;
    for(int i=0; i<m_PlaySpecificLevel.length(); i++) {
      if(m_PlaySpecificLevel[i] < '0' || m_PlaySpecificLevel[i] > '9') {
        v_isANumber = false;
      }
    }
    if(v_isANumber) {
      int nNum = atoi(m_PlaySpecificLevel.c_str());
      if(nNum > 0) {
        char cBuf[256];
        sprintf(cBuf,"_iL%02d_",nNum-1);
        m_PlaySpecificLevel = cBuf;
      }
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
          m_ForceProfile = UserArgs[i+1];
        else
          throw SyntaxError("no profile specified");        
        i++;
      }
      else if(UserArgs[i] == "-gdebug") {
        if(i+1<UserArgs.size())
          m_GraphDebugInfoFile = UserArgs[i+1];
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
      else if(UserArgs[i] == "-testTheme") {
        m_bTestThemeMode = true;
      }
      else if(UserArgs[i] == "-benchmark") {
        m_bBenchmark = true;
      }
      else if(UserArgs[i] == "-cleancache") {
        m_bCleanCache = true;
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
    printf("\t-testTheme\n\t\tDisplay forms around the theme to check it.\n");
    printf("\t-benchmark\n\t\tOnly meaningful when combined with -replay and\n");
    printf("\t\t-timedemo. Useful to determine the graphics\n");
             printf("\t\tperformance.\n");
    printf("\t-cleancache\n\t\tDeletes the content of the level cache.\n");
  }  
  
  /*===========================================================================
  Create the default config
  ===========================================================================*/
  void GameApp::_CreateDefaultConfig(void) {
    m_Config.createVar( "Theme",                  THEME_DEFAULT_THEMENAME);    

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
    
    m_Config.createVar( "AutosaveHighscoreReplays", "true");
    m_Config.createVar("LimitFramerate", "false");

    #if defined(ENABLE_ZOOMING)
      m_Config.createVar( "KeyZoomIn",              "PageUp" );
      m_Config.createVar( "KeyZoomOut",             "PageDown" );
      m_Config.createVar( "KeyZoomInit",            "Home" );
      m_Config.createVar( "KeyCameraMoveXUp",       "Pad 6" );
      m_Config.createVar( "KeyCameraMoveXDown",     "Pad 4" );
      m_Config.createVar( "KeyCameraMoveYUp",       "Pad 8" );
      m_Config.createVar( "KeyCameraMoveYDown",     "Pad 2" );
    #endif
        
    m_Config.createVar( "JoyIdx1",                "0" );
    m_Config.createVar( "JoyAxisPrim1",           "1" );
    m_Config.createVar( "JoyAxisPrimMax1",        "32760" );
    m_Config.createVar( "JoyAxisPrimMin1",        "-32760" );
    m_Config.createVar( "JoyAxisPrimUL1",         "1024" );
    m_Config.createVar( "JoyAxisPrimLL1",         "-1024" );
    m_Config.createVar( "JoyAxisSec1",            "0" );
    m_Config.createVar( "JoyAxisSecMax1",         "32760" );
    m_Config.createVar( "JoyAxisSecMin1",         "-32760" );
    m_Config.createVar( "JoyAxisSecUL1",          "1024" );
    m_Config.createVar( "JoyAxisSecLL1",          "-1024" );
    m_Config.createVar( "JoyButtonChangeDir1",    "0" );
    m_Config.createVar( "JoystickLimboArea",      "0.07" );

    /* Misc */
    m_Config.createVar( "DefaultProfile",         "" );
    m_Config.createVar( "ScreenshotFormat",       "png" );
    m_Config.createVar( "NotifyAtInit",           "true" );
    m_Config.createVar( "ShowMiniMap",            "true" );
    m_Config.createVar( "ShowEngineCounter",      "false" );

    m_Config.createVar( "StoreReplays",           "true" );
    m_Config.createVar( "ReplayFrameRate",        "25" );
    m_Config.createVar( "CompressReplays",        "true" );
    m_Config.createVar( "LevelCache",             "true" );
    m_Config.createVar( "ContextHelp",            "true" );
    m_Config.createVar( "MenuMusic",              "true" );    
    m_Config.createVar( "InitZoom",               "false" );
    m_Config.createVar( "DeathAnim",              "true" );

#if defined(SUPPORT_WEBACCESS)
    m_Config.createVar( "WebHighscores",            "false" );
    m_Config.createVar( "CheckHighscoresAtStartup", "true" );
    m_Config.createVar( "CheckNewLevelsAtStartup",  "true" );
    m_Config.createVar( "ShowInGameWorldRecord",    "false" );
    m_Config.createVar( "WebConfAtInit",            "true" );
    
    /* Webstuff */
    m_Config.createVar( "WebHighscoresURL",       DEFAULT_WEBHIGHSCORES_URL );
    m_Config.createVar( "WebLevelsURL",           DEFAULT_WEBLEVELS_URL);
    m_Config.createVar( "WebThemesURL",           DEFAULT_WEBTHEMES_URL);
    m_Config.createVar( "WebThemesURLBase",       DEFAULT_WEBTHEMES_SPRITESURLBASE);
    m_Config.createVar( "WebRoomsURL",            DEFAULT_WEBROOMS_URL);

    /* Proxy */
    m_Config.createVar( "ProxyType",              "" ); /* (blank), HTTP, SOCKS4, or SOCKS5 */
    m_Config.createVar( "ProxyServer",            "" ); /* (may include user/pass and port) */
    m_Config.createVar( "ProxyPort",              "-1" );
    m_Config.createVar( "ProxyAuthUser",          "" ); 
    m_Config.createVar( "ProxyAuthPwd",           "" );

    /* auto upload */
    m_Config.createVar( "WebHighscoreUploadURL"      , DEFAULT_UPLOADREPLAY_URL);
    m_Config.createVar( "WebHighscoreUploadIdRoom"   , DEFAULT_WEBROOM_ID);
    m_Config.createVar( "WebHighscoreUploadLogin"    , "");
    m_Config.createVar( "WebHighscoreUploadPassword" , ""); 

#endif
    
#if defined(ALLOW_GHOST)
    m_Config.createVar( "EnableGhost"        , "true");
    m_Config.createVar( "GhostSearchStrategy", "0");
    m_Config.createVar( "ShowGhostTimeDiff"  , "true");
    m_Config.createVar( "DisplayGhostInfo"   , "false");
    m_Config.createVar( "GhostMotionBlur"    , "true" );
#endif
  }
  
}

