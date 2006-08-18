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

#ifndef __VAPP_H__
#define __VAPP_H__

#include "VCommon.h"

//#include "VExcept.h"
//#include "VMath.h"
#include "VDraw.h"
//#include "VTexture.h"
#include "Image.h"

namespace vapp {

  class App;

  /*===========================================================================
  Globals
  ===========================================================================*/
  void Log(const char *pcFmt,...);
  void LogRaw(const char *pcMsg);
  void Verbose(const char *pcMsg);  

  /*===========================================================================
  Sub-application - something that runs in the context of a parent app
  useful for ugly pop-menus, etc.
  ===========================================================================*/
  class SubApp {
    public:
      SubApp() {
        m_pParentApp = NULL;
        m_bShouldClose = false;
      }
      virtual ~SubApp() {};
      
      /* Methods */
      int run(App *pParent);
            
      /* Data interface */
      App *getParent(void) {return m_pParentApp;}

      /* Protected */
      void subClose(int nRetVal);
      
    protected:
      
      /* Virtual protected methods */
      virtual void update(void) {}
      virtual void keyDown(int nKey,int nChar) {}
      virtual void keyUp(int nKey) {}
      virtual void mouseDown(int nButton) {}
      virtual void mouseUp(int nButton) {}
                      
    private: 
      /* Data */
      App *m_pParentApp;
      bool m_bShouldClose;
      int m_nRetVal;
  };
  
  /*===========================================================================
  Vector graphics application base class
  ===========================================================================*/
  class App : public DrawLib {
    public:
      App() {m_bQuit=false;
             m_fAppTime=0.0f;
             m_nDispWidth=800;
             m_nDispHeight=600;
             m_nDispBPP=32;
             m_bWindowed=true;
             m_fFramesPerSecond=25.0f;
             m_fNextFrame=0.0f;
             m_bNoGraphics=false;
	     m_bNoWWW = false;                         
             m_bDontUseGLExtensions=false;
             m_nFrameDelay=0;
             m_bShadersSupported = false;
             m_bCmdDispWidth=false;
             m_bCmdDispHeight=false;
             m_bCmdDispBPP=false;
             m_bCmdWindowed=false;
             
             m_bFBOSupported = false;
             
             m_nLScissorX = m_nLScissorY = m_nLScissorW = m_nLScissorH = 0;
                          
             m_AppName="";
             m_CopyrightInfo="";
             m_AppCommand="";
             m_UserNotify="";
             }
      virtual ~App() {}
    
      /* Methods */
      void run(int nNumArgs,char **ppcArgs);
      
      /* Application definition */
      void setAppName(const std::string &i) {m_AppName=i;}
      void setCopyrightInfo(const std::string &i) {m_CopyrightInfo=i;}
      void setAppCommand(const std::string &i) {m_AppCommand=i;}
      void setFPS(float i) {m_fFramesPerSecond=i;}      
            
      static double getTime(void); 
      static double getRealTime(void);
      std::string getTimeStamp(void);
      void quit(void);      
      static std::string formatTime(float fSecs);
      void getMousePos(int *pnX,int *pnY);        
      bool haveMouseMoved(void);    
    
//#if defined(EMUL_800x600)      
//      int getDispWidth(void) {return 800;}
//      int getDispHeight(void) {return 600;}
//#else
      int getDispWidth(void) {return m_nDispWidth;}
      int getDispHeight(void) {return m_nDispHeight;}
//#endif

      std::vector<std::string>* getDisplayModes();

      void setFrameDelay(int nDelay) {m_nFrameDelay=nDelay;}

      void scissorGraphics(int x,int y,int nWidth,int nHeight);
      void getScissorGraphics(int *px,int *py,int *pnWidth,int *pnHeight);
      Img *grabScreen(void);
      bool isExtensionSupported(std::string Ext);

      const std::string &getUserNotify(void) {return m_UserNotify;}

      int getDispBPP(void) {return m_nDispBPP;}
      bool getWindowed(void) {return m_bWindowed;}
      float getFPS(void) {return m_fFramesPerSecond;}      
      bool isNoGraphics(void) {return m_bNoGraphics;}
      void setNoGraphics(bool b) {m_bNoGraphics = b;}
      bool isNoWWW(void) {return m_bNoWWW;}
      static std::string getVersionString(void) {
        char cBuf[256]; sprintf(cBuf,"%d.%d.%d" BUILD_EXTRAINFO,BUILD_MAJORVERSION,BUILD_VERSION,BUILD_MINORVERSION);
        return cBuf;        
      }
      
      bool isCmdDispWidth(void) {return m_bCmdDispWidth;}
      bool isCmdDispHeight(void) {return m_bCmdDispHeight;}
      bool isCmdDispBPP(void) {return m_bCmdDispBPP;}
      bool isCmdDispWindowed(void) {return m_bCmdWindowed;}      
      bool useVBOs(void) {return m_bVBOSupported;}
      bool useFBOs(void) {return m_bFBOSupported;}
      bool useShaders(void) {return m_bShadersSupported;}
      
      Theme m_theme;
      
      /* Extensions */
      PFNGLGENBUFFERSARBPROC glGenBuffersARB;
      PFNGLBINDBUFFERARBPROC glBindBufferARB;
      PFNGLBUFFERDATAARBPROC glBufferDataARB;
      PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
              
      /* Extensions (for render-to-texture) */
      PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXT;
      PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
      PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT;
      PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
      PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
      PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT;
      PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXT;
      PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
      PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
      PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
      PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
      PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT;
      PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
      PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT;
      PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
      PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT;
      PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT;         
      
      /* Extensions (for shaders) */
      PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocationARB;
      PFNGLGETACTIVEATTRIBARBPROC glGetActiveAttribARB;
      PFNGLGETATTRIBLOCATIONARBPROC glGetAttribLocationARB;
      PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
      PFNGLGETHANDLEARBPROC glGetHandleARB;
      PFNGLDETACHOBJECTARBPROC glDetachObjectARB;
      PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
      PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
      PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
      PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
      PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
      PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
      PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;
      PFNGLVALIDATEPROGRAMARBPROC glValidateProgramARB;
      PFNGLUNIFORM1FARBPROC glUniform1fARB;
      PFNGLUNIFORM2FARBPROC glUniform2fARB;
      PFNGLUNIFORM3FARBPROC glUniform3fARB;
      PFNGLUNIFORM4FARBPROC glUniform4fARB;
      PFNGLUNIFORM1IARBPROC glUniform1iARB;
      PFNGLUNIFORM2IARBPROC glUniform2iARB;
      PFNGLUNIFORM3IARBPROC glUniform3iARB;
      PFNGLUNIFORM4IARBPROC glUniform4iARB;
      PFNGLUNIFORM1FVARBPROC glUniform1fvARB;
      PFNGLUNIFORM2FVARBPROC glUniform2fvARB;
      PFNGLUNIFORM3FVARBPROC glUniform3fvARB;
      PFNGLUNIFORM4FVARBPROC glUniform4fvARB;
      PFNGLUNIFORM1IVARBPROC glUniform1ivARB;
      PFNGLUNIFORM2IVARBPROC glUniform2ivARB;
      PFNGLUNIFORM3IVARBPROC glUniform3ivARB;
      PFNGLUNIFORM4IVARBPROC glUniform4ivARB;
      PFNGLUNIFORMMATRIX2FVARBPROC glUniformMatrix2fvARB;
      PFNGLUNIFORMMATRIX3FVARBPROC glUniformMatrix3fvARB;
      PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fvARB;
      PFNGLGETOBJECTPARAMETERFVARBPROC glGetObjectParameterfvARB;
      PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
      PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
      PFNGLGETATTACHEDOBJECTSARBPROC glGetAttachedObjectsARB;
      PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;
      PFNGLGETACTIVEUNIFORMARBPROC glGetActiveUniformARB;
      PFNGLGETUNIFORMFVARBPROC glGetUniformfvARB;
      PFNGLGETUNIFORMIVARBPROC glGetUniformivARB;
      PFNGLGETSHADERSOURCEARBPROC glGetShaderSourceARB;    
      
    protected:

      /* Virtual protected methods */
      virtual void drawFrame(void) {}      
      virtual void keyDown(int nKey,int nChar) {}
      virtual void keyUp(int nKey) {}
      virtual void mouseDown(int nButton) {}
      virtual void mouseDoubleClick(int nButton) {}      
      virtual void mouseUp(int nButton) {}
      virtual void parseUserArgs(std::vector<std::string> &UserArgs) {}
      virtual void helpUserArgs(void) {}
      virtual void userInit(void) {}
      virtual void userPreInit(void) {}
      virtual void userShutdown(void) {}
      virtual void selectDisplayMode(int *pnWidth,int *pnHeight,int *pnBPP,bool *pbWindowed) {}
            
    private:
      /* Private helper functions */
      void _Init(int nDispWidth,int nDispHeight,int nDispBPP,bool bWindowed);
      void _Uninit(void);
      void _ParseArgs(int nNumArgs,char **ppcArgs);
      
      /* Data */
      int m_nFrameDelay; /* # of millisecs to wait after screen buffer swap */
      
      int m_nDispWidth,m_nDispHeight,m_nDispBPP; /* Screen stuff */
      bool m_bWindowed;         /* Windowed or not */
      float m_fFramesPerSecond; /* Force this FPS */
      bool m_bNoGraphics;       /* No-graphics mode */      
      bool m_bNoWWW;

      bool m_bCmdDispWidth,m_bCmdDispHeight,m_bCmdDispBPP,m_bCmdWindowed;
      
      bool m_bVBOSupported;
      bool m_bFBOSupported;
      bool m_bShadersSupported;
      bool m_bDontUseGLExtensions;
      
      int m_nLScissorX,m_nLScissorY,m_nLScissorW,m_nLScissorH;
      
      /* User nofification */
      std::string m_UserNotify;
            
      /* App def */
      std::string m_AppName;        /* Name of app */
      std::string m_CopyrightInfo;  /* Application copyright string */
      std::string m_AppCommand;     /* Command to start app */           
           
      /* Run-time fun */
      bool m_bQuit;             /* Quit flag */
      double m_fAppTime;        /* Current application time */
      double m_fNextFrame;      /* Time next frame rendering should begin */
  };

}

#endif

