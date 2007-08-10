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
 *  Vector-graphics application base class.
 */
#if defined(WIN32)
  #include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "VApp.h"
#include "VFileIO.h"
#include "Packager.h"
#include "helpers/SwapEndian.h"
#include "helpers/Log.h"
#include "Game.h"
#include "XMArgs.h"
#include "XMBuild.h"

#ifdef USE_GETTEXT
#include "Locales.h"
#endif

#include "XMSession.h"

/*===========================================================================
SDL main entry point
===========================================================================*/
#if defined(WIN32)
int SDL_main(int nNumArgs,char **ppcArgs) {
#else
int main(int nNumArgs,char **ppcArgs) {
#endif
  /* Start application */
  try {     
    /* Setup basic info */
    vapp::GameApp Game;
    Game.run(nNumArgs,ppcArgs);
  }
  catch (Exception &e) {
    if(Logger::isInitialized()) {
      Logger::Log((std::string("Exception: ") + e.getMsg()).c_str());
    }    

    printf("fatal exception : %s\n",e.getMsg().c_str());        
    SDL_Quit(); /* make sure SDL shuts down gracefully */
    
#if defined(WIN32)
    char cBuf[1024];
    sprintf(cBuf,"Fatal exception occured: %s\n"
	    "Consult the file xmoto.log for more information about what\n"
	    "might has occured.\n",e.getMsg().c_str());                    
    MessageBox(NULL,cBuf,"X-Moto Error",MB_OK|MB_ICONERROR);
#endif
  }
  return 0;
}

namespace vapp {

  App::App() {
    m_bQuit = false;
    m_fAppTime = 0.0f;
    m_UserNotify = "";
    m_fFramesPerSecond = 25.0f;
    m_fNextFrame = 0.0f;
    m_nFrameDelay = 0;
    drawLib = NULL;
    
    m_xmsession = new XMSession();
  }

  App::~App() {
    if(drawLib != NULL) {
/* kejo removed GL specific...*/
      //Log("Nb glyphs created: %i",
	  //drawLib->getFontSmall()->nbGlyphsInMemory()  +
	  //drawLib->getFontMedium()->nbGlyphsInMemory() +
	  //drawLib->getFontBig()->nbGlyphsInMemory()
//	  );
      delete drawLib;
    }

    delete m_xmsession;
  }

  
  /*===========================================================================
  Main application entry point
  ===========================================================================*/
  void App::run(int nNumArgs,char **ppcArgs) {
    XMArguments v_xmArgs;

    /* check args */
    try {
      v_xmArgs.parse(nNumArgs, ppcArgs);
    } catch (Exception &e) {
      printf("syntax error : %s\n", e.getMsg().c_str());
      v_xmArgs.help(nNumArgs >= 1 ? ppcArgs[0] : "xmoto");
      return; /* abort */
    }

    /* help */
    if(v_xmArgs.isOptHelp()) {
      v_xmArgs.help(nNumArgs >= 1 ? ppcArgs[0] : "xmoto");
      return;
    }

    /* init sub-systems */
    SwapEndian::Swap_Init();
    srand(time(NULL));
    FS::init("xmoto");
    Logger::init(FS::getUserDir() + "/xmoto.log");

    /* package / unpackage */
    if(v_xmArgs.isOptPack()) {
      Packager::go(v_xmArgs.getOpt_pack_bin() == "" ? "xmoto.bin" : v_xmArgs.getOpt_pack_bin(),
		   v_xmArgs.getOpt_pack_dir() == "" ? "."         : v_xmArgs.getOpt_pack_dir());
      Logger::uninit();
      FS::uninit();
      return;
    }
    if(v_xmArgs.isOptUnPack()) {
      Packager::goUnpack(v_xmArgs.getOpt_unpack_bin() == "" ? "xmoto.bin" : v_xmArgs.getOpt_unpack_bin(),
			 v_xmArgs.getOpt_unpack_dir() == "" ? "."         : v_xmArgs.getOpt_unpack_dir(),
			 v_xmArgs.getOpt_unpack_noList() == false);
      Logger::uninit();
      FS::uninit();
      return;
    }
    /* ***** */

    /* load config file */
    createDefaultConfig();
    m_Config.loadFile();

    /* load session */
    m_xmsession->load(&m_Config); /* overload default session by userConfig */
    m_xmsession->load(&v_xmArgs); /* overload default session by xmargs     */

    /* apply verbose mode */
    Logger::setVerbose(m_xmsession->isVerbose());

#ifdef USE_GETTEXT
    std::string v_locale = Locales::init(m_Config.getString("Language"));
#endif

    Logger::Log("compiled at "__DATE__" "__TIME__);
    if(SwapEndian::bigendien) {
      Logger::Log("Systeme is bigendien");
    } else {
      Logger::Log("Systeme is littleendien");
    }

    Logger::Log("User directory: %s", FS::getUserDir().c_str());
    Logger::Log("Data directory: %s", FS::getDataDir().c_str());

#ifdef USE_GETTEXT
    Logger::Log("Locales set to '%s' (directory '%s')", v_locale.c_str(), LOCALESDIR);
#endif

    if(v_xmArgs.isOptListLevels() || v_xmArgs.isOptListReplays() || v_xmArgs.isOptReplayInfos()) {
      m_xmsession->setUseGraphics(false);
    }

    _InitWin(m_xmsession->useGraphics());

    if(m_xmsession->useGraphics()) {
      /* init drawLib */
      drawLib = DrawLib::DrawLibFromName(m_xmsession->drawlib());

      if(drawLib == NULL) {
	throw Exception("Drawlib not initialized");
      }

      drawLib->setNoGraphics(m_xmsession->useGraphics() == false);
      drawLib->setDontUseGLExtensions(m_xmsession->glExts() == false);

      /* Init! */
      Logger::Log("Resolution: %ix%i (%i bpp)", m_xmsession->resolutionWidth(), m_xmsession->resolutionHeight(), m_xmsession->bpp());
      drawLib->init(m_xmsession->resolutionWidth(), m_xmsession->resolutionHeight(), m_xmsession->bpp(), m_xmsession->windowed(), &m_theme);
      if(!drawLib->isNoGraphics()) {        
	drawLib->setDrawDims(m_xmsession->resolutionWidth(), m_xmsession->resolutionHeight(),
			     m_xmsession->resolutionWidth(), m_xmsession->resolutionHeight());
      }
    }
    
    /* Now perform user init */
    userInit(&v_xmArgs);
    
    /* Enter the main loop */
    while(!m_bQuit) {
      if(!drawLib->isNoGraphics()) {
        /* Handle SDL events */            
        SDL_PumpEvents();
        
        SDL_Event Event;
        while(SDL_PollEvent(&Event)) {
          int ch=0;
          static int nLastMouseClickX = -100,nLastMouseClickY = -100;
          static int nLastMouseClickButton = -100;
          static float fLastMouseClickTime = 0.0f;
          int nX,nY;

          /* What event? */
          switch(Event.type) {
            case SDL_KEYDOWN: 
              if((Event.key.keysym.unicode&0xff80)==0) {
                ch = Event.key.keysym.unicode & 0x7F;
              }
              keyDown(Event.key.keysym.sym, Event.key.keysym.mod, ch);            
              break;
            case SDL_KEYUP: 
              keyUp(Event.key.keysym.sym, Event.key.keysym.mod);            
              break;
            case SDL_QUIT:  
              /* Force quit */
              quit();
              break;
            case SDL_MOUSEBUTTONDOWN:
              /* Pass ordinary click */
              mouseDown(Event.button.button);
              
              /* Is this a double click? */
              getMousePos(&nX,&nY);
              if(nX == nLastMouseClickX &&
                 nY == nLastMouseClickY &&
                 nLastMouseClickButton == Event.button.button &&
                 (getRealTime() - fLastMouseClickTime) < 0.250f) {                

                /* Pass double click */
                mouseDoubleClick(Event.button.button);                
              }
              fLastMouseClickTime = getRealTime();
              nLastMouseClickX = nX;
              nLastMouseClickY = nY;
              nLastMouseClickButton = Event.button.button;
            
              break;
            case SDL_MOUSEBUTTONUP:
              mouseUp(Event.button.button);
              break;
          }
        }
          
        /* Clear screen */  
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (isUglyMode()){
	  drawLib->clearGraphics();
	}
        drawLib->resetGraphics();

      }
      
      /* Update user app */
      drawFrame();
      
      if(!drawLib->isNoGraphics()) {
        /* Swap buffers */
       drawLib->flushGraphics();
        
        /* Does app want us to delay a bit after the frame? */
        if(m_nFrameDelay > 0)
	  SDL_Delay(m_nFrameDelay);
      }
    }
    
    /* Shutdown */
    _Uninit();
  }

  bool App::haveMouseMoved(void) {
    int nX,nY;
    SDL_GetRelativeMouseState(&nX,&nY);
    if(nX || nY) return true;
    return false;
  }

  void App::getMousePos(int *pnX,int *pnY) {
    int nX,nY;
    SDL_GetMouseState(&nX,&nY);
    
    //int xx = (m_nDispWidth/2 - getDispWidth()/2);
    //int yy = (m_nDispHeight/2 - getDispHeight()/2);
    
    if(pnX) *pnX = nX;// - xx;
    if(pnY) *pnY = nY;// - yy;
    
    //if(pnX) *pnX = (nX*getDispWidth()) / m_nDispWidth;
    //if(pnY) *pnY = (nY*getDispHeight()) / m_nDispHeight;
  }
  
  /*===========================================================================
  Get real-time clock
  ===========================================================================*/
  std::string App::getTimeStamp(void) {
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
  
  double App::getTime(void) {
    return SDL_GetTicks() / 1000.0f;
  }
  double App::getRealTime(void) {
    return SDL_GetTicks() / 1000.0f;
  }
  
  std::string App::formatTime(float fSecs) {
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
  void App::quit(void) {
    /* Set quit flag */
    m_bQuit = true;
  }
  
  /*===========================================================================
  Init 
  ===========================================================================*/
  void App::_InitWin(bool bInitGraphics) {

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
  void App::_Uninit(void) {
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
  std::vector<std::string>* App::getDisplayModes(int windowed){
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

  /*===========================================================================
  Set/get graphics scissoring
  ===========================================================================*/
  void App::scissorGraphics(int x,int y,int nWidth,int nHeight) {
	  drawLib->setClipRect(x,y,nWidth,nHeight);
  }  
  
  void App::getScissorGraphics(int *px,int *py,int *pnWidth,int *pnHeight) {
	  drawLib->getClipRect(px,py,pnWidth,pnHeight);
  }  
  
  bool App::isUglyMode() {
    return m_xmsession->ugly();
  }
}


