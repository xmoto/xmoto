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
  //keesj:todo perhaps move these functions to VCommon
  //because they are used in the DrawLib, I don't think
  //drawlib should depend on VApp
  void Log(const char *pcFmt, ...);
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
    } virtual ~ SubApp() {
    };

    /* Methods */
    int run(App * pParent);

    /* Data interface */
    App *getParent(void) {
      return m_pParentApp;
    }

    /* Protected */
    void subClose(int nRetVal);

  protected:

    /* Virtual protected methods */
    virtual void update(void) {
    }
    virtual void keyDown(int nKey, SDLMod mod, int nChar) {
    }
    virtual void keyUp(int nKey, SDLMod mod) {
    }
    virtual void mouseDown(int nButton) {
    }
    virtual void mouseUp(int nButton) {
    }

  private:
    /* Data */
    App * m_pParentApp;
    bool m_bShouldClose;
    int m_nRetVal;
  };

  /*===========================================================================
  Vector graphics application base class
  ===========================================================================*/
  class App {
  public:
    App() {
      m_bQuit = false;
      m_fAppTime = 0.0f;
      m_AppName = "";
      m_CopyrightInfo = "";
      m_AppCommand = "";
      m_UserNotify = "";
      m_fFramesPerSecond = 25.0f;
      m_fNextFrame = 0.0f;
      m_bNoWWW = false;
      m_nFrameDelay = 0;
      drawLib = NULL;

      m_useGraphics = true;
      m_useGlExtension = true;

      m_CmdDispWidth = -1;
      m_CmdDispHeight = -1;
      m_CmdDispBpp = -1;
      m_CmdWindowed = false;
      m_bCmdDispWidth = false;
      m_bCmdDispHeight = false;
      m_bCmdDispBPP = false;
      m_bCmdWindowed = false;

    } virtual ~ App() {
    }

    /* Methods */
    void run(int nNumArgs, char **ppcArgs);

    /* Application definition */
    void setAppName(const std::string & i) {
      m_AppName = i;
    }
    void setCopyrightInfo(const std::string & i) {
      m_CopyrightInfo = i;
    }
    void setAppCommand(const std::string & i) {
      m_AppCommand = i;
    }
    void setFPS(float i) {
      m_fFramesPerSecond = i;
    }
    static double getTime(void);
    static double getRealTime(void);
    static std::string getTimeStamp(void);
    void quit(void);
    static std::string formatTime(float fSecs);
    void getMousePos(int *pnX, int *pnY);
    bool haveMouseMoved(void);




    std::vector < std::string > *getDisplayModes(int windowed);

    void setFrameDelay(int nDelay) {
      m_nFrameDelay = nDelay;
    }

    void scissorGraphics(int x, int y, int nWidth, int nHeight);
    void getScissorGraphics(int *px, int *py, int *pnWidth, int *pnHeight);
    Img *grabScreen(void);


    const std::string & getUserNotify(void) {
      return m_UserNotify;
    }


    float getFPS(void) {
      return m_fFramesPerSecond;
    }
//      bool isNoGraphics(void) {return m_bNoGraphics;}
//      void setNoGraphics(bool b) {m_bNoGraphics = b;}
    bool isNoWWW(void) {
      return m_bNoWWW;
    }

    void setNoWWW(bool bValue) {
      m_bNoWWW = bValue;
    }
    static std::string getVersionString();

    virtual bool isUglyMode() {
      return false;
    };

      /**
       * keesj:TOTO
       * the getDrawLib and getTheme method of the app
       * are there in order to make the current code compile
       * I don't know yet what part of xmoto should be responsable
       * of the theme. Idem for the GUI classes. they currentely
       * contain a pointer to an VApp. this interdepenency of GUI and vapp
       * creates a tight coupling between both.
       * while the GUI only uses the theme and the drawing functions
       **/
    DrawLib *getDrawLib() {
      return drawLib;
    };
    Theme *getTheme() {
      return &m_theme;
    };


  protected:
    bool isCmdDispWidth(void) {
      return m_bCmdDispWidth;
    }
    bool isCmdDispHeight(void) {
      return m_bCmdDispHeight;
    }
    bool isCmdDispBPP(void) {
      return m_bCmdDispBPP;
    }
    bool isCmdDispWindowed(void) {
      return m_bCmdWindowed;
    }
    /* Virtual protected methods */
    virtual void drawFrame(void) {
    }
    virtual void keyDown(int nKey, SDLMod mod, int nChar) {
    }
    virtual void keyUp(int nKey, SDLMod mod) {
    }
    virtual void mouseDown(int nButton) {
    }
    virtual void mouseDoubleClick(int nButton) {
    }
    virtual void mouseUp(int nButton) {
    }
    virtual void parseUserArgs(std::vector < std::string > &UserArgs) {
    }
    virtual void helpUserArgs(void) {
    }
    virtual void userInit(void) {
    }
    virtual void userPreInit(void) {
    }
    virtual void userShutdown(void) {
    }
    virtual void selectDisplayMode(int *pnWidth, int *pnHeight, int *pnBPP,
				   bool * pbWindowed) {
    }

    virtual std::string selectDrawLibMode() {
      return "";
    }
      /**
       * The DrawLib instance to use for this app
       **/
    Theme m_theme;
    DrawLib *drawLib;
    bool m_useGraphics;
    bool m_useGlExtension;

    int  m_CmdDispWidth;
    int  m_CmdDispHeight;
    int  m_CmdDispBpp;
    bool m_CmdWindowed;
    std::string m_CmdDrawLibName;

  private:
    /* Private helper functions */
    void _Init(int nDispWidth, int nDispHeight, int nDispBPP,
	       bool bWindowed);
    void _Uninit(void);
    void _ParseArgs(int nNumArgs, char **ppcArgs);





    /* Data */
    int m_nFrameDelay;		/* # of millisecs to wait after screen buffer swap */

    bool m_bNoWWW;
    float m_fFramesPerSecond;	/* Force this FPS */


    bool m_bCmdDispWidth, m_bCmdDispHeight, m_bCmdDispBPP, m_bCmdWindowed;

    /* User nofification */
    std::string m_UserNotify;

    /* App def */
    std::string m_AppName;	/* Name of app */
    std::string m_CopyrightInfo;	/* Application copyright string */
    std::string m_AppCommand;	/* Command to start app */

    /* Run-time fun */
    bool m_bQuit;		/* Quit flag */
    double m_fAppTime;		/* Current application time */
    double m_fNextFrame;	/* Time next frame rendering should begin */
  };

}

#endif
