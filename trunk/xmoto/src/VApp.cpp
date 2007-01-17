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

#ifdef USE_GETTEXT
#include "Locales.h"
#endif

namespace vapp {

  /*===========================================================================
  Global application log 
  ===========================================================================*/
  bool g_bQuietLog = false;
  bool g_bVerbose = false;
  
  void LogRaw(const char *pcMsg) {
    if(!g_bQuietLog) {
      Verbose(pcMsg);
      FS::writeLog(pcMsg);
    }
  }
  
  void Verbose(const char *pcMsg) {
#if defined(_MSC_VER) && defined(_DEBUG)
    printf("%s\n",pcMsg); /* I like my stdout when debugging :P */
#else
    if(g_bVerbose) 
    printf("%s\n",pcMsg); /* also write to stdout */
#endif
  }

  void Log(const char *pcFmt,...) {
    va_list List;
    char cBuf[1024];
    va_start(List,pcFmt);
    vsprintf(cBuf,pcFmt,List);
    va_end(List);
    
    LogRaw(cBuf);    
  }

  /*===========================================================================
  Sub app entry point and stuff
  ===========================================================================*/
  int SubApp::run(App *pParent) {
    /* Reset everything */
    m_nRetVal = 0;
    m_bShouldClose = false;
    m_pParentApp = pParent;
    
    /* Start looping */
    while(!m_bShouldClose) {
      /* Handle SDL events */      
      SDL_PumpEvents();
      
      SDL_Event Event;
      while(SDL_PollEvent(&Event)) {
        int ch=0;
    
        /* What event? */
        switch(Event.type) {
          case SDL_KEYDOWN: 
            if((Event.key.keysym.unicode&0xff80)==0) {
              ch = Event.key.keysym.unicode & 0x7F;
            }
            keyDown(Event.key.keysym.sym,ch);            
            break;
          case SDL_KEYUP: 
            keyUp(Event.key.keysym.sym);            
            break;
          case SDL_MOUSEBUTTONDOWN:
            mouseDown(Event.button.button);
            break;
          case SDL_MOUSEBUTTONUP:
            mouseUp(Event.button.button);
            break;
          case SDL_QUIT:  
            /* Force quit */
            getParent()->quit();
            return 0;
        }
      }
    
      pParent->getDrawLib()->clearGraphics();
      pParent->getDrawLib()->resetGraphics();
      
      /* Update */
      update();
      
      pParent->getDrawLib()->flushGraphics();
    }
    
    /* Return */
    return m_nRetVal;
  }
  
  void SubApp::subClose(int nRetVal) {
    m_nRetVal = nRetVal;
    m_bShouldClose = true;
  }
  
  /*===========================================================================
  Main application entry point
  ===========================================================================*/
  void App::run(int nNumArgs,char **ppcArgs) {

    /* init endian system */ 
#ifdef USE_GETTEXT
    Locales::init();
#endif
    SwapEndian::Swap_Init();

    srand(time(NULL));

    if(SwapEndian::bigendien) {
      Log("Systeme is bigendien");
    } else {
      Log("Systeme is littleendien");
    }

    try {
      /* Parse command-line arguments */
      _ParseArgs(nNumArgs,ppcArgs);        
    } 
    catch (SyntaxError &e) {
      if(e.getMsg().length() > 0)
        printf("syntax error : %s\n",e.getMsg().c_str());
      return; /* abort */
    }

    /* Init file system stuff */
    FS::init( "xmoto" );

    /* Do user pre-init */
    userPreInit();

    /* init drawLib */
    if(m_CmdDrawLibName == "") {
      drawLib = DrawLib::DrawLibFromName(selectDrawLibMode());
    } else {
      drawLib = DrawLib::DrawLibFromName(m_CmdDrawLibName);
    }

    if(drawLib == NULL) {
      throw Exception("Drawlib not initialized");
    }

    drawLib->setNoGraphics(m_useGraphics == false);
    drawLib->setDontUseGLExtensions(m_useGraphics == false);

    int configured_width,configured_height,configured_BPP;
    bool configured_windowed;
    /* user configuration */
    selectDisplayMode(&configured_width, &configured_height, &configured_BPP, &configured_windowed);
        
    /* overwrite by cmdline configuration */
    if(isCmdDispWidth())    configured_width     = m_CmdDispWidth;
    if(isCmdDispHeight())   configured_height    = m_CmdDispHeight;
    if(isCmdDispBPP())      configured_BPP       = m_CmdDispBpp;
    if(isCmdDispWindowed()) configured_windowed  = m_CmdWindowed;

    /* Init! */
    _Init(configured_width,configured_height,configured_BPP , configured_windowed);
    if(!drawLib->isNoGraphics()) {        
      drawLib->setDrawDims(configured_width,configured_height,configured_width,configured_height);
    }
    
    /* Now perform user init */
    userInit();
    
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
              keyDown(Event.key.keysym.sym,ch);            
              break;
            case SDL_KEYUP: 
              keyUp(Event.key.keysym.sym);            
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
      sprintf(cBuf,"%02d:%02d:%02d %d-%02d-%02d",pTime->tm_hour,pTime->tm_min,pTime->tm_sec,
              pTime->tm_year+1900,pTime->tm_mon+1,pTime->tm_mday+1);                    
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
    int nHSecs = (int) (fSecs * 100.0);
    char cBuf[256];
    sprintf(cBuf,"%02d:%02d:%02d",nHSecs/6000,(nHSecs%6000)/100,(nHSecs%6000)%100);
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
  void App::_Init(int nDispWidth,int nDispHeight,int nDispBPP,bool bWindowed) {
	       /* Init SDL */
    if(drawLib->isNoGraphics()) {
      if(SDL_Init(SDL_INIT_TIMER) < 0)
        throw Exception("(1) SDL_Init : " + std::string(SDL_GetError()));
      
      /* No graphics mojo here, thank you */
      return;
    }
    else {
      if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0)
        throw Exception("(2) SDL_Init : " + std::string(SDL_GetError()));
    }
    drawLib->init(nDispWidth,nDispHeight,nDispBPP,bWindowed,&m_theme);
    /* Set window title */
    SDL_WM_SetCaption(m_AppName.c_str(),m_AppName.c_str());
  }

  /*===========================================================================
  Uninit 
  ===========================================================================*/
  void App::_Uninit(void) {
    /* Tell user app to turn off */
    userShutdown();

    if(!drawLib->isNoGraphics()) {
      /* Uninit drawing library */
      drawLib->unInit();
    }

    
    /* Shutdown SDL */
    SDL_Quit();
  }
  
  /*===========================================================================
  Return available display modes
  ===========================================================================*/
  std::vector<std::string>* App::getDisplayModes(){
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
      Log("** Warning ** : No display modes available.");
      throw Exception("getDisplayModes : No modes available.");
    }

    /* Always include these to modes */
    modes->push_back("800 X 600");
    modes->push_back("1024 X 768");

    /* Check if or resolution is restricted */
    if(sdl_modes == (SDL_Rect **)-1){
      /* Should never happen */
      //Log("All resolutions available.");
      modes->push_back("1280 X 1024");
      modes->push_back("1600 X 1200");
    } else{
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

  


  /*===========================================================================
  Parse command-line arguments
  ===========================================================================*/
  void App::_ParseArgs(int nNumArgs,char **ppcArgs) {
    std::vector<std::string> UserArgs;
  
    /* Walk through the args */
    for(int i=1;i<nNumArgs;i++) {
      if(!strcmp(ppcArgs[i],"-pack")) {
	Packager::go();
	exit(0); /* leaks memory, but who cares? :) */
      } else if(!strcmp(ppcArgs[i],"-unpack")) {
	std::string BinFile = "xmoto.bin";
	if(i+1 < nNumArgs) {BinFile = ppcArgs[i+1]; i++;}
          std::string OutDir = ".";
          if(i+1 < nNumArgs) {OutDir = ppcArgs[i+1]; i++;}
          bool bMakePackageList = true;
          if(i+1 < nNumArgs && ppcArgs[i+1]=="no_lst") {bMakePackageList=false; i++;}
          Packager::goUnpack(BinFile,OutDir,bMakePackageList);
          exit(0); /* leaks memory too, but still nobody cares */
      } else if(!strcmp(ppcArgs[i],"-nogfx")) {
	m_useGraphics = true;
      }
      else if(!strcmp(ppcArgs[i],"-res")) {
        if(i+1 == nNumArgs) 
          throw SyntaxError("missing resolution");
        sscanf(ppcArgs[i+1],"%dx%d",&m_CmdDispWidth,&m_CmdDispHeight);
        m_bCmdDispWidth = m_bCmdDispHeight = true;
        i++;
      }
      else if(!strcmp(ppcArgs[i],"-bpp")) {
        if(i+1 == nNumArgs) 
          throw SyntaxError("missing bit depth");
	m_CmdDispBpp = atoi(ppcArgs[i+1]);
        m_bCmdDispBPP = true;
        i++;
      }
      else if(!strcmp(ppcArgs[i],"-fs")) {
	m_CmdWindowed = false;
        m_bCmdWindowed = true;
      }
      else if(!strcmp(ppcArgs[i],"-win")) {
	m_CmdWindowed = true;
        m_bCmdWindowed = true;
      }
      else if(!strcmp(ppcArgs[i],"-q")) {
        g_bQuietLog = true;
      } 
      else if(!strcmp(ppcArgs[i],"-v")) {
        g_bVerbose = true;
      } 
      else if(!strcmp(ppcArgs[i],"-noexts")) {
	m_useGlExtension = false;
      }
      else if(!strcmp(ppcArgs[i],"-nowww")) {
        m_bNoWWW = true;
#ifdef ENABLE_SDLGFX
      } else if(!strcmp(ppcArgs[i],"-drawlib")) {
        if(i+1 == nNumArgs) {
          throw SyntaxError("missing drawlib");
	}
	m_CmdDrawLibName = ppcArgs[i+1];
        i++;
#endif
      } else if(!strcmp(ppcArgs[i],"-h") || !strcmp(ppcArgs[i],"-?") ||
              !strcmp(ppcArgs[i],"--help") || !strcmp(ppcArgs[i],"-help")) {
        printf("%s (Version %s)\n",m_AppName.c_str(),getVersionString().c_str());
        if(m_CopyrightInfo.length()>0)
          printf("%s\n",m_CopyrightInfo.c_str());
        printf("usage:  %s {options}\n"
               "options:\n",m_AppCommand.c_str());
        
        printf("\t-res WIDTHxHEIGHT\n\t\tSpecifies display resolution to use.\n");
        printf("\t-bpp BITS\n\t\tTry to use this display color bit depth.\n");
        printf("\t-fs\n\t\tForces fullscreen mode.\n");
        printf("\t-win\n\t\tForces windowed mode.\n");
        printf("\t-nogfx\n\t\tDon't show any graphical elements.\n");
        printf("\t-q\n\t\tDon't print messages to screen, and don't save them in the log.\n");
        printf("\t-v\n\t\tBe verbose.\n");
        printf("\t-noexts\n\t\tDon't use any OpenGL extensions.\n");
        printf("\t-nowww\n\t\tDon't allow xmoto to connect on the web.\n");
#ifdef ENABLE_SDLGFX
        printf("\t-sdlgfx\n\t\tSelect SDL_fgx as rendering engine.\n");
#endif
        helpUserArgs();
        printf("\n");
        
        throw SyntaxError();
      }
      else {
        /* Add it to argument vector */
        UserArgs.push_back(ppcArgs[i]);
      }
    }
    
    /* Pass any arguments to the user app */
    if(UserArgs.size() > 0)
      parseUserArgs(UserArgs);
  }

}

