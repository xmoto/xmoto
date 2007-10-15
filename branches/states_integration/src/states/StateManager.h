/*=============================================================================
XMOTO

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

#ifndef __STATEMANAGER_H__
#define __STATEMANAGER_H__

#include "VCommon.h"

class GameApp;

  class GameState {
  public:
    GameState(bool drawStateBehind,
	      bool updateStatesBehind,
	      GameApp* pGame);
    virtual ~GameState();

    virtual void enter()=0;
    virtual void leave()=0;
    /* called when a new state is pushed or poped on top of the
       current one */
    virtual void enterAfterPop()=0;
    virtual void leaveAfterPush()=0;

    // return true if update/render was done
    virtual bool update()=0;
    virtual bool render()=0;
    /* input */
    virtual void keyDown(int nKey, SDLMod mod,int nChar)=0;
    virtual void keyUp(int nKey,   SDLMod mod)=0;
    virtual void mouseDown(int nButton)=0;
    virtual void mouseDoubleClick(int nButton)=0;
    virtual void mouseUp(int nButton)=0;

    bool isHide(){
      return m_isHide;
    }
    void setHide(bool isHide){
      m_isHide = isHide;
    }
    bool drawStatesBehind(){
      return m_drawStateBehind;
    }
    bool updateStatesBehind(){
      return m_updateStatesBehind;
    }

    bool requestForEnd() {
      return m_requestForEnd;
    }

    int getUpdateFps(){
      return m_updateFps;
    }

    int getRenderFps(){
      return m_renderFps;
    }

  protected:
    bool doUpdate();
    bool doRender();

    bool     m_requestForEnd;    
    GameApp* m_pGame;

    // the desired fps for updating and rendering the state
    int      m_updateFps;
    int      m_renderFps;
    // the last time when an update/render was done
    float    m_fLastUpdateTime;
    float    m_fLastRenderingTime;

  private:
    bool     m_isHide;
    bool     m_drawStateBehind;
    bool     m_updateStatesBehind;
  };

  class StateManager {
  public:
    StateManager(GameApp* pGame);
    ~StateManager();

    void pushState(GameState* pNewState);
    GameState* popState();
    /* return the previous top state */
    GameState* replaceState(GameState* pNewState);

    /*
      after some events, a state can requestForEnd
      -> return NULL or the state which requested to be ended
    */
    GameState* flush();

    void update();
    void render();
    /* input */
    void keyDown(int nKey, SDLMod mod,int nChar);
    void keyUp(int nKey,   SDLMod mod);
    void mouseDown(int nButton);
    void mouseDoubleClick(int nButton);
    void mouseUp(int nButton);

    int getCurrentUpdateFPS();
    int getCurrentRenderFPS();
    int getMaxFps(){
      return m_maxFps;
    }

    /* ask to states to clean themself */
    static void cleanStates();

  private:
    void calculateWhichStateIsRendered();
    void calculateFps();

    std::vector<GameState*> m_statesStack;

    // time at the begining of the last frame rendered/updated
    float m_fLastRenderingTime;
    float m_fLastUpdateTime;

    // last time the m_current*Fps have been filled
    float m_fLastFpsTime;

    // contains the number of frame during the last second
    int   m_currentRenderFps;
    int   m_currentUpdateFps;

    // counting the number of frame for the current second
    int   m_renderFpsNbFrame;
    int   m_updateFpsNbFrame;

    // the hz at which the state manager runs
    // (the highest from the states)
    int   m_maxUpdateFps;
    int   m_maxRenderFps;
    int   m_maxFps;

    GameApp* m_pGame;
  };

#endif
