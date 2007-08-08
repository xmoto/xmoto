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

#include "VDraw.h"
#include "Image.h"
#include "UserConfig.h"

class XMSession;
class XMArguments;
class UserConfig;

namespace vapp {


  class App;


  /*===========================================================================
  Vector graphics application base class
  ===========================================================================*/
  class App {
  public:
    App();
    virtual ~App();

    /* Methods */
    void run(int nNumArgs, char **ppcArgs);

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

    static std::vector<std::string>* getDisplayModes(int windowed);

    void setFrameDelay(int nDelay) {
      m_nFrameDelay = nDelay;
    }

    void scissorGraphics(int x, int y, int nWidth, int nHeight);
    void getScissorGraphics(int *px, int *py, int *pnWidth, int *pnHeight);
    Img *grabScreen(void);


    const std::string & getUserNotify(void) {
      return m_UserNotify;
    }

    bool isUglyMode();
    
    float getFPS(void) {
      return m_fFramesPerSecond;
    }

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
    virtual void userInit(XMArguments* v_xmArgs) {
    }
    virtual void userShutdown(void) {
    }
    virtual void createDefaultConfig() {
    }

      /**
       * The DrawLib instance to use for this app
       **/
      
    /* Config & profiles */
    UserConfig m_Config;

    Theme m_theme;
    DrawLib *drawLib;

    XMSession* m_xmsession;

  private:
    /* Private helper functions */
    void _InitWin(bool bInitGraphics);
    void _Uninit(void);

    /* Data */
    int m_nFrameDelay;		/* # of millisecs to wait after screen buffer swap */

    float m_fFramesPerSecond;	/* Force this FPS */

    /* User nofification */
    std::string m_UserNotify;

    /* Run-time fun */
    bool m_bQuit;		/* Quit flag */
    double m_fAppTime;		/* Current application time */
    double m_fNextFrame;	/* Time next frame rendering should begin */
  };

}

#endif
