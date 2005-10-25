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

namespace vapp {

  /*===========================================================================
  Global application log 
  ===========================================================================*/
  bool g_bQuietLog = false;
  
  void Log(char *pcFmt,...) {
    va_list List;
    char cBuf[1024];
    va_start(List,pcFmt);
    vsprintf(cBuf,pcFmt,List);
    va_end(List);
    
    if(!g_bQuietLog)
      printf("%s\n",cBuf);
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
    
      /* Clear screen */  
      glClear(GL_COLOR_BUFFER_BIT);
      
      /* Update */
      update();
      
      /* Swap buffers */
      SDL_GL_SwapBuffers();
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
    try {
      /* Parse command-line arguments */
      _ParseArgs(nNumArgs,ppcArgs);        
    } 
    catch (SyntaxError &e) {
      if(e.getMsg().length() > 0)
        printf("syntax error : %s\n",e.getMsg().c_str());
      return; /* abort */
    }

    /* Do user pre-init */
    userPreInit();

    selectDisplayMode(&m_nDispWidth,&m_nDispHeight,&m_nDispBPP,&m_bWindowed);
        
    /* Init! */
    _Init(m_nDispWidth,m_nDispHeight,m_nDispBPP,m_bWindowed);
    if(!isNoGraphics()) {        
      /* Tell DrawLib about it */
      //#if defined(EMUL_800x600)      
      //  setDrawDims(m_nDispWidth,m_nDispHeight,800,600);
      //#else
        setDrawDims(m_nDispWidth,m_nDispHeight,m_nDispWidth,m_nDispHeight);
      //#endif
    }
    
    /* Now perform user init */
    userInit();
    
    /* Enter the main loop */
    while(!m_bQuit) {
      if(!isNoGraphics()) {
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
        glClear(GL_COLOR_BUFFER_BIT);
      }
      
      /* Update user app */
      drawFrame();
      
      if(!isNoGraphics()) {
        /* Swap buffers */
        SDL_GL_SwapBuffers();
      }
    }
    
    /* Shutdown */
    _Uninit();
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
    int nHSecs = fSecs * 100;
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
    if(isNoGraphics()) {
      if(SDL_Init(SDL_INIT_TIMER) < 0)
        throw Exception("(1) SDL_Init : " + std::string(SDL_GetError()));
      
      /* No graphics mojo here, thank you */
      return;
    }
    else {
      if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0)
        throw Exception("(2) SDL_Init : " + std::string(SDL_GetError()));
    }
    
    /* Set suggestions */
    m_nDispWidth = nDispWidth;
    m_nDispHeight = nDispHeight;
    m_nDispBPP = nDispBPP;
    m_bWindowed = bWindowed;

	  /* Get some video info */
	  const SDL_VideoInfo *pVidInfo=SDL_GetVideoInfo();
	  if(pVidInfo==NULL)
      throw Exception("(1) SDL_GetVideoInfo : " + std::string(SDL_GetError()));
  
	  /* Determine target bit depth */
	  if(m_bWindowed) 
		  /* In windowed mode we can't tinker with the bit-depth */
		  m_nDispBPP=pVidInfo->vfmt->BitsPerPixel; 			

	  /* Setup GL stuff - note we setup depth too, even though it isn't used
	    at all. This is simply for completeness :) */
	  /* 2005-10-05 ... note that we no longer ask for explicit settings... it's
	                    better to do it per auto */
	  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);	

	  //switch(m_nDispBPP) {
		 // case 16: 
			//  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,5);
			//  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,6);
			//  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,5);
			//  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,16);
			//  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);	
			//  break;
		 // case 32: 
			//  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
			//  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
			//  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
			//  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,16);
			//  break;
	  //}	
	
	  /* Create video flags */
	  int nFlags = SDL_OPENGL;
	  if(!m_bWindowed) nFlags|=SDL_FULLSCREEN;
	
	  /* At last, try to "set the video mode" */
	  if(SDL_SetVideoMode(m_nDispWidth,m_nDispHeight,m_nDispBPP,nFlags)==NULL) {
	    Log("** Warning ** : Tried to set video mode %dx%d @ %d-bit, but SDL responded: %s\n"
	        "                Now SDL will try determining a proper mode itself.",m_nDispWidth,m_nDispHeight,m_nDispBPP);
	  
	    /* Hmm, try letting it decide the BPP automatically */
	    if(SDL_SetVideoMode(m_nDispWidth,m_nDispHeight,0,nFlags)==NULL) {
        throw Exception("SDL_SetVideoMode : " + std::string(SDL_GetError()));
      }
    }
		
	  /* Retrieve actual configuration */
	  pVidInfo=SDL_GetVideoInfo();
	  if(pVidInfo==NULL)
      throw Exception("(2) SDL_GetVideoInfo : " + std::string(SDL_GetError()));
  									
	  m_nDispBPP=pVidInfo->vfmt->BitsPerPixel;
	
	  /* Set window title */
	  SDL_WM_SetCaption(m_AppName.c_str(),m_AppName.c_str());

	  /* Force OpenGL to talk 2D */
	  glViewport(0,0,m_nDispWidth,m_nDispHeight);
	  glMatrixMode(GL_PROJECTION);
	  glLoadIdentity();
	  //gluOrtho2D(0,0,m_nDispWidth,m_nDispHeight);
//	  glOrtho(0,0,m_nDispWidth,m_nDispHeight,-1,1);
	  glOrtho(0,m_nDispWidth,0,m_nDispHeight,-1,1);
	  
	  glMatrixMode(GL_MODELVIEW);
	  glLoadIdentity();		
		
	  /* Enable unicode translation and key repeats */
	  SDL_EnableUNICODE(1);    		  
	  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
	  
	  /* Init drawing library */
	  initLib(&TexMan);
	  
	  /* Output some general info */
	  Log("GL: %s (%s)",glGetString(GL_RENDERER),glGetString(GL_VENDOR));
	  if(glGetString(GL_RENDERER) == NULL || 
	     glGetString(GL_VENDOR) == NULL) {
	    Log("** Warning ** : GL strings NULL!");
	    throw Exception("GL strings are NULL!");
	  }
	  
    /* Windows: check whether we are using the standard GDI OpenGL software driver... If
       so make sure the user is warned */
    #if defined(WIN32) 
      if(!strcmp(reinterpret_cast<const char *>(glGetString(GL_RENDERER)),"GDI Generic") &&
         !strcmp(reinterpret_cast<const char *>(glGetString(GL_VENDOR)),"Microsoft Corporation")) {
        Log("** Warning ** : No GL hardware acceleration!");
        m_UserNotify = "It seems that no OpenGL hardware acceleration is available!\n"
                       "Please make sure OpenGL is configured properly.";
      }
    #endif
	  
//          Log("GL: %s",glGetString(GL_EXTENSIONS));
	  /* Init OpenGL extensions */
	  if(m_bDontUseGLExtensions) 
	    m_bVBOSupported = false;
	  else
	    m_bVBOSupported = isExtensionSupported("GL_ARB_vertex_buffer_object");
	  
	  if(m_bVBOSupported) {
		  glGenBuffersARB=(PFNGLGENBUFFERSARBPROC)SDL_GL_GetProcAddress("glGenBuffersARB");
		  glBindBufferARB=(PFNGLBINDBUFFERARBPROC)SDL_GL_GetProcAddress("glBindBufferARB");
		  glBufferDataARB=(PFNGLBUFFERDATAARBPROC)SDL_GL_GetProcAddress("glBufferDataARB");
		  glDeleteBuffersARB=(PFNGLDELETEBUFFERSARBPROC)SDL_GL_GetProcAddress("glDeleteBuffersARB");	    

	    glEnableClientState( GL_VERTEX_ARRAY );		
	    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		  	  
	    Log("GL: using Vertex Buffer Objects");	  
	  }
	  else
	    Log("GL: not using Vertex Buffer Objects");	  
	  
	  /* Set background color to black */
	  glClearColor(0.0f,0.0f,0.0f,0.0f);
	  glClear(GL_COLOR_BUFFER_BIT);
	  SDL_GL_SwapBuffers();	  	      
  }

  /*===========================================================================
  Uninit 
  ===========================================================================*/
  void App::_Uninit(void) {
    /* Tell user app to turn off */
    userShutdown();
  
    if(!isNoGraphics()) {
      /* Uninit drawing library */
      uninitLib(&TexMan);
    }
        
    /* Shutdown SDL */
    SDL_Quit();
  }
  
  /*===========================================================================
  Set graphics scissoring
  ===========================================================================*/
  void App::scissorGraphics(int x,int y,int nWidth,int nHeight) {
    //float fx = (float)(m_nDispWidth*x) / (float)getDispWidth();
    //float fy = (float)(m_nDispHeight*y) / (float)getDispHeight();
    //float fw = (float)(m_nDispWidth*nWidth) / (float)getDispWidth();
    //float fh = (float)(m_nDispHeight*nHeight) / (float)getDispHeight();
    float fx = (float)(m_nDispWidth*x) / (float)getDispWidth();
    float fy = (float)(m_nDispHeight*y) / (float)getDispHeight();
    float fw = (float)(m_nDispWidth*nWidth) / (float)getDispWidth();
    float fh = (float)(m_nDispHeight*nHeight) / (float)getDispHeight();
  
    //glScissor(fx,m_nDispHeight - (fy+fh)-1,fw+1,fh+1);
    
    glScissor(x,m_nDispHeight - (y+nHeight),nWidth,nHeight);
  }  

  /*===========================================================================
  Check for OpenGL extension
  ===========================================================================*/
  bool App::isExtensionSupported(std::string Ext) {
    const unsigned char *pcExtensions = NULL;
    const unsigned char *pcStart;
    unsigned char *pcWhere,*pcTerminator;
    
    pcExtensions = glGetString(GL_EXTENSIONS);
    if(pcExtensions == NULL) {
      Log("Failed to determine OpenGL extensions. Try stopping all other\n"
          "applications that might use your OpenGL hardware.\n"
          "If it still doesn't work, please create a detailed bug report.\n"
          );
      throw Exception("glGetString() : NULL");
    }
    
    pcStart = pcExtensions;
    while(1) {
      pcWhere = (unsigned char *)strstr((const char*)pcExtensions,Ext.c_str());
      if(pcWhere == NULL) break;
      pcTerminator = pcWhere + Ext.length();
      if(pcWhere == pcStart || *(pcWhere-1) == ' ')
        if(*pcTerminator == ' ' || *pcTerminator == '\0')
          return true;
      pcStart = pcTerminator;
    }
    return false;
  }
  
  /*===========================================================================
  Grab screen contents
  ===========================================================================*/
  Img *App::grabScreen(void) {
    Img *pImg = new Img;
    
    pImg->createEmpty(m_nDispWidth,m_nDispHeight);
    Color *pPixels = pImg->getPixels();
    unsigned char *pcTemp = new unsigned char [m_nDispWidth*3];
  
    /* Select frontbuffer */
    glReadBuffer(GL_FRONT);

    /* Read the pixels (reversed) */
    for(int i=0;i<m_nDispHeight;i++) {          
	    glReadPixels(0,i,m_nDispWidth,1,GL_RGB,GL_UNSIGNED_BYTE,pcTemp);
	    for(int j=0;j<m_nDispWidth;j++) {
	      pPixels[(m_nDispHeight - i - 1)*m_nDispWidth + j] = MAKE_COLOR(
	        pcTemp[j*3],pcTemp[j*3+1],pcTemp[j*3+2],255
	      );
	    }
    }		        
    
    delete [] pcTemp;
    return pImg;            
  }

  /*===========================================================================
  Parse command-line arguments
  ===========================================================================*/
  void App::_ParseArgs(int nNumArgs,char **ppcArgs) {
    std::vector<std::string> UserArgs;
  
    /* Walk through the args */
    for(int i=1;i<nNumArgs;i++) {
      if(!strcmp(ppcArgs[i],"-nogfx")) {
        m_bNoGraphics = true;
      }
      else if(!strcmp(ppcArgs[i],"-res")) {
        if(i+1 == nNumArgs) 
          throw SyntaxError("missing resolution");
          
        sscanf(ppcArgs[i+1],"%dx%d",&m_nDispWidth,&m_nDispHeight);
        m_bCmdDispWidth = m_bCmdDispHeight = true;
        i++;
      }
      else if(!strcmp(ppcArgs[i],"-bpp")) {
        if(i+1 == nNumArgs) 
          throw SyntaxError("missing bit depth");
          
        m_nDispBPP = atoi(ppcArgs[i+1]);
        m_bCmdDispBPP = true;
        i++;
      }
      else if(!strcmp(ppcArgs[i],"-fs")) {
        m_bWindowed = false;
        m_bCmdWindowed = true;
      }
      else if(!strcmp(ppcArgs[i],"-win")) {
        m_bWindowed = true;
        m_bCmdWindowed = true;
      }
      else if(!strcmp(ppcArgs[i],"-q")) {
        g_bQuietLog = true;
      } 
      else if(!strcmp(ppcArgs[i],"-noexts")) {
        m_bDontUseGLExtensions = true;
      }     
      else if(!strcmp(ppcArgs[i],"-h") || !strcmp(ppcArgs[i],"-?") ||
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
        printf("\t-q\n\t\tLeave stdout pretty much alone.\n");
        printf("\t-noexts\n\t\tDon't use any OpenGL extensions.\n");
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

};

