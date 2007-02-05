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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      }
      
      /* Update user app */
      drawFrame();
      
      if(!isNoGraphics()) {
        /* Swap buffers */
        SDL_GL_SwapBuffers();
        
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

    /* Setup GL stuff */
    /* 2005-10-05 ... note that we no longer ask for explicit settings... it's
                      better to do it per auto */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1); 
  
    /* Create video flags */
    int nFlags = SDL_OPENGL;
    if(!m_bWindowed) nFlags|=SDL_FULLSCREEN;
  
    /* At last, try to "set the video mode" */
    if(SDL_SetVideoMode(m_nDispWidth,m_nDispHeight,m_nDispBPP,nFlags)==NULL) {
      Log("** Warning ** : Tried to set video mode %dx%d @ %d-bit, but SDL responded: %s\n"
          "                Now SDL will try determining a proper mode itself.",m_nDispWidth,m_nDispHeight,m_nDispBPP);
    
      /* Hmm, try letting it decide the BPP automatically */
      if(SDL_SetVideoMode(m_nDispWidth,m_nDispHeight,0,nFlags)==NULL) {       
        /* Still no luck */
        Log("** Warning ** : Still no luck, now we'll try 800x600 in a window.");
        m_nDispWidth = 800; m_nDispHeight = 600;        
        m_bWindowed = true;
        if(SDL_SetVideoMode(m_nDispWidth,m_nDispHeight,0,SDL_OPENGL)==NULL) {       
          throw Exception("SDL_SetVideoMode : " + std::string(SDL_GetError()));
        }       
      }
    }
    
    /* Retrieve actual configuration */
    pVidInfo=SDL_GetVideoInfo();
    if(pVidInfo==NULL)
      throw Exception("(2) SDL_GetVideoInfo : " + std::string(SDL_GetError()));
                    
    m_nDispBPP=pVidInfo->vfmt->BitsPerPixel;

    /* Did we get a z-buffer? */        
    int nDepthBits;
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE,&nDepthBits);
    if(nDepthBits == 0)
      throw Exception("no depth buffer");  
  
    /* Set window title */
    SDL_WM_SetCaption(m_AppName.c_str(),m_AppName.c_str());

    /* Force OpenGL to talk 2D */
    glViewport(0,0,m_nDispWidth,m_nDispHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,m_nDispWidth,0,m_nDispHeight,-1,1);
    
    glClearDepth(1);
    glDepthFunc(GL_LEQUAL);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();   
    
    /* Enable unicode translation and key repeats */
    SDL_EnableUNICODE(1);         
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
     
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
        //m_UserNotify = "It seems that no OpenGL hardware acceleration is available!\n"
        //               "Please make sure OpenGL is configured properly.";
      }
    #endif
    
    /* Init OpenGL extensions */
    if(m_bDontUseGLExtensions) {
      m_bVBOSupported = false;
      m_bFBOSupported = false;
      m_bShadersSupported = false;
    }
    else {
      m_bVBOSupported = isExtensionSupported("GL_ARB_vertex_buffer_object");
      m_bFBOSupported = isExtensionSupported("GL_EXT_framebuffer_object");
      
      m_bShadersSupported = isExtensionSupported("GL_ARB_fragment_shader") &&
                            isExtensionSupported("GL_ARB_vertex_shader") &&
                            isExtensionSupported("GL_ARB_shader_objects");
    }
    
    if(m_bVBOSupported) {
      glGenBuffersARB=(PFNGLGENBUFFERSARBPROC)SDL_GL_GetProcAddress("glGenBuffersARB");
      glBindBufferARB=(PFNGLBINDBUFFERARBPROC)SDL_GL_GetProcAddress("glBindBufferARB");
      glBufferDataARB=(PFNGLBUFFERDATAARBPROC)SDL_GL_GetProcAddress("glBufferDataARB");
      glDeleteBuffersARB=(PFNGLDELETEBUFFERSARBPROC)SDL_GL_GetProcAddress("glDeleteBuffersARB");      

      glEnableClientState( GL_VERTEX_ARRAY );   
      glEnableClientState( GL_TEXTURE_COORD_ARRAY );
          
      Log("GL: using ARB_vertex_buffer_object");    
    }
    else
      Log("GL: not using ARB_vertex_buffer_object");    
      
    if(m_bFBOSupported) {
      glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glIsRenderbufferEXT");
      glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindRenderbufferEXT");
      glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffersEXT");
      glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenRenderbuffersEXT");
      glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)SDL_GL_GetProcAddress("glRenderbufferStorageEXT");
      glGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)SDL_GL_GetProcAddress("glGetRenderbufferParameterivEXT");
      glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC)SDL_GL_GetProcAddress("glIsFramebufferEXT");
      glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindFramebufferEXT");
      glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteFramebuffersEXT");
      glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenFramebuffersEXT");
      glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT");
      glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture1DEXT");
      glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture2DEXT");
      glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture3DEXT");
      glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glFramebufferRenderbufferEXT");
      glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)SDL_GL_GetProcAddress("glGetFramebufferAttachmentParameterivEXT");
      glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)SDL_GL_GetProcAddress("glGenerateMipmapEXT");
          
      Log("GL: using EXT_framebuffer_object");
    }
    else
      Log("GL: not using EXT_framebuffer_object");
      
    if(m_bShadersSupported) {
      glBindAttribLocationARB = (PFNGLBINDATTRIBLOCATIONARBPROC)SDL_GL_GetProcAddress("glBindAttribLocationARB");
      glGetActiveAttribARB = (PFNGLGETACTIVEATTRIBARBPROC)SDL_GL_GetProcAddress("glGetActiveAttribARB");
      glGetAttribLocationARB = (PFNGLGETATTRIBLOCATIONARBPROC)SDL_GL_GetProcAddress("glGetAttribLocationARB");
      glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)SDL_GL_GetProcAddress("glDeleteObjectARB");
      glGetHandleARB = (PFNGLGETHANDLEARBPROC)SDL_GL_GetProcAddress("glGetHandleARB");
      glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)SDL_GL_GetProcAddress("glDetachObjectARB");
      glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)SDL_GL_GetProcAddress("glCreateShaderObjectARB");
      glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)SDL_GL_GetProcAddress("glShaderSourceARB");
      glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)SDL_GL_GetProcAddress("glCompileShaderARB");
      glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)SDL_GL_GetProcAddress("glCreateProgramObjectARB");
      glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)SDL_GL_GetProcAddress("glAttachObjectARB");
      glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)SDL_GL_GetProcAddress("glLinkProgramARB");
      glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)SDL_GL_GetProcAddress("glUseProgramObjectARB");
      glValidateProgramARB = (PFNGLVALIDATEPROGRAMARBPROC)SDL_GL_GetProcAddress("glValidateProgramARB");
      glUniform1fARB = (PFNGLUNIFORM1FARBPROC)SDL_GL_GetProcAddress("glUniform1fARB");
      glUniform2fARB = (PFNGLUNIFORM2FARBPROC)SDL_GL_GetProcAddress("glUniform2fARB");
      glUniform3fARB = (PFNGLUNIFORM3FARBPROC)SDL_GL_GetProcAddress("glUniform3fARB");
      glUniform4fARB = (PFNGLUNIFORM4FARBPROC)SDL_GL_GetProcAddress("glUniform4fARB");
      glUniform1iARB = (PFNGLUNIFORM1IARBPROC)SDL_GL_GetProcAddress("glUniform1iARB");
      glUniform2iARB = (PFNGLUNIFORM2IARBPROC)SDL_GL_GetProcAddress("glUniform2iARB");
      glUniform3iARB = (PFNGLUNIFORM3IARBPROC)SDL_GL_GetProcAddress("glUniform3iARB");
      glUniform4iARB = (PFNGLUNIFORM4IARBPROC)SDL_GL_GetProcAddress("glUniform4iARB");
      glUniform1fvARB = (PFNGLUNIFORM1FVARBPROC)SDL_GL_GetProcAddress("glUniform1fvARB");
      glUniform2fvARB = (PFNGLUNIFORM2FVARBPROC)SDL_GL_GetProcAddress("glUniform2fvARB");
      glUniform3fvARB = (PFNGLUNIFORM3FVARBPROC)SDL_GL_GetProcAddress("glUniform3fvARB");
      glUniform4fvARB = (PFNGLUNIFORM4FVARBPROC)SDL_GL_GetProcAddress("glUniform4fvARB");
      glUniform1ivARB = (PFNGLUNIFORM1IVARBPROC)SDL_GL_GetProcAddress("glUniform1ivARB");
      glUniform2ivARB = (PFNGLUNIFORM2IVARBPROC)SDL_GL_GetProcAddress("glUniform2ivARB");
      glUniform3ivARB = (PFNGLUNIFORM3IVARBPROC)SDL_GL_GetProcAddress("glUniform3ivARB");
      glUniform4ivARB = (PFNGLUNIFORM4IVARBPROC)SDL_GL_GetProcAddress("glUniform4ivARB");
      glUniformMatrix2fvARB = (PFNGLUNIFORMMATRIX2FVARBPROC)SDL_GL_GetProcAddress("glUniformMatrix2fvARB");
      glUniformMatrix3fvARB = (PFNGLUNIFORMMATRIX3FVARBPROC)SDL_GL_GetProcAddress("glUniformMatrix3fvARB");
      glUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC)SDL_GL_GetProcAddress("glUniformMatrix4fvARB");
      glGetObjectParameterfvARB = (PFNGLGETOBJECTPARAMETERFVARBPROC)SDL_GL_GetProcAddress("glGetObjectParameterfvARB");
      glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)SDL_GL_GetProcAddress("glGetObjectParameterivARB");
      glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)SDL_GL_GetProcAddress("glGetInfoLogARB");
      glGetAttachedObjectsARB = (PFNGLGETATTACHEDOBJECTSARBPROC)SDL_GL_GetProcAddress("glGetAttachedObjectsARB");
      glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)SDL_GL_GetProcAddress("glGetUniformLocationARB");
      glGetActiveUniformARB = (PFNGLGETACTIVEUNIFORMARBPROC)SDL_GL_GetProcAddress("glGetActiveUniformARB");
      glGetUniformfvARB = (PFNGLGETUNIFORMFVARBPROC)SDL_GL_GetProcAddress("glGetUniformfvARB");
      glGetUniformivARB = (PFNGLGETUNIFORMIVARBPROC)SDL_GL_GetProcAddress("glGetUniformivARB");
      glGetShaderSourceARB = (PFNGLGETSHADERSOURCEARBPROC)SDL_GL_GetProcAddress("glGetShaderSourceARB");    
        
      Log("GL: using ARB_fragment_shader/ARB_vertex_shader/ARB_shader_objects");
    }
    else
      Log("GL: not using ARB_fragment_shader/ARB_vertex_shader/ARB_shader_objects");
    
    /* Set background color to black */
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapBuffers();  

    /* Init drawing library */
    initLib(&m_theme);

        
  }

  /*===========================================================================
  Uninit 
  ===========================================================================*/
  void App::_Uninit(void) {
    /* Tell user app to turn off */
    userShutdown();

    if(!isNoGraphics()) {
      /* Uninit drawing library */
      uninitLib(&m_theme);
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
    //float fx = (float)(m_nDispWidth*x) / (float)getDispWidth();
    //float fy = (float)(m_nDispHeight*y) / (float)getDispHeight();
    //float fw = (float)(m_nDispWidth*nWidth) / (float)getDispWidth();
    //float fh = (float)(m_nDispHeight*nHeight) / (float)getDispHeight();
    //float fx = (float)(m_nDispWidth*x) / (float)getDispWidth();
    //float fy = (float)(m_nDispHeight*y) / (float)getDispHeight();
    //float fw = (float)(m_nDispWidth*nWidth) / (float)getDispWidth();
    //float fh = (float)(m_nDispHeight*nHeight) / (float)getDispHeight();
  
    //glScissor(fx,m_nDispHeight - (fy+fh)-1,fw+1,fh+1);
    
    glScissor(x,m_nDispHeight - (y+nHeight),nWidth,nHeight);
    
    m_nLScissorX = x;
    m_nLScissorY = y;
    m_nLScissorW = nWidth;
    m_nLScissorH = nHeight;
  }  
  
  void App::getScissorGraphics(int *px,int *py,int *pnWidth,int *pnHeight) {
    *px = m_nLScissorX;
    *py = m_nLScissorY;
    *pnWidth = m_nLScissorW;
    *pnHeight = m_nLScissorH;
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
      else if(!strcmp(ppcArgs[i],"-v")) {
        g_bVerbose = true;
      } 
      else if(!strcmp(ppcArgs[i],"-noexts")) {
        m_bDontUseGLExtensions = true;
      }
      else if(!strcmp(ppcArgs[i],"-nowww")) {
        m_bNoWWW = true;
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
        printf("\t-q\n\t\tDon't print messages to screen, and don't save them in the log.\n");
        printf("\t-v\n\t\tBe verbose.\n");
        printf("\t-noexts\n\t\tDon't use any OpenGL extensions.\n");
        printf("\t-nowww\n\t\tDon't allow xmoto to connect on the web.\n");
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

