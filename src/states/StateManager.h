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

#include "StateMessageBoxReceiver.h"
#include "StateMenuContextReceiver.h"
#include "helpers/Singleton.h"

class VideoRecorder;

class GameState : public StateMessageBoxReceiver,
		  public StateMenuContextReceiver {
public:
  GameState(bool drawStateBehind,
	    bool updateStatesBehind,
	    bool i_doShade     = false,
	    bool i_doShadeAnim = false);
  virtual ~GameState();

  virtual void enter();
  virtual void leave() {}
  /* called when a new state is pushed or poped on top of the
     current one */
  virtual void enterAfterPop() {}
  virtual void leaveAfterPush() {}

  // return true if update/render was done
  virtual bool update() {return false;}
  virtual bool render();
  virtual bool updateWhenUnvisible() {return false;}
  virtual void onRenderFlush() {}

  /* input */
  virtual void keyDown(int nKey, SDLMod mod,int nChar);
  virtual void keyUp(int nKey,   SDLMod mod) {}
  virtual void mouseDown(int nButton) {}
  virtual void mouseDoubleClick(int nButton) {}
  virtual void mouseUp(int nButton) {}

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

  void setCurrentRenderFps(int curRenFps){
    m_curRenderFps = curRenFps;
  }

  void setMaxFps(int maxFps){
    m_maxFps = maxFps;
    m_updatePeriod = (float)m_maxFps / (float)m_updateFps;
  }

  std::string getId() const;
  void setId(const std::string& i_id);

  virtual void send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input);
  virtual void send(const std::string& i_id, const std::string& i_message);

  std::string getName() const {
    return m_name;
  }

  void simpleMessage(const std::string& msg);

  bool showCursor() {
    return m_showCursor;
  }
  void executeCommands();
  virtual void executeOneCommand(std::string cmd);

protected:
  bool doUpdate();

  bool m_requestForEnd;    

  // the desired fps for updating and rendering the state
  int m_updateFps;
  int m_renderFps;
  // state behind the top state have to render at the same speed at it.
  int m_curRenderFps;
  int m_maxFps;
  // how many max fps beat for one update/render
  float m_updatePeriod;
  // current beat counters
  float m_updateCounter;

  std::string m_name;
  bool m_showCursor;

  std::queue<std::string> m_commands;

private:
  bool        m_isHide;
  bool        m_drawStateBehind;
  bool        m_updateStatesBehind;
  std::string m_id;

  // shade
  bool  m_doShade;
  bool  m_doShadeAnim;
  float m_nShadeTime;
};

class StateManager : public Singleton<StateManager> {
  friend class Singleton<StateManager>;

private:
  StateManager();
  ~StateManager();

public:
    void pushState(GameState* pNewState);
    GameState* popState();
    /* return the previous top state */
    GameState* replaceState(GameState* pNewState);

    /*
      after some events, a state can requestForEnd
      -> return NULL or the state which requested to be ended
    */
  void flush();

  void update();
  void render();
  /* input */
  void keyDown(int nKey, SDLMod mod,int nChar);
  void keyUp(int nKey,   SDLMod mod);
  void mouseDown(int nButton);
  void mouseDoubleClick(int nButton);
  void mouseUp(int nButton);
  void changeFocus(bool i_hasFocus);
  void changeVisibility(bool i_visible);

  bool needUpdateOrRender();

  // to display on the screen
  int getCurrentUpdateFPS();
  int getCurrentRenderFPS();

  // send the message to every states
  void sendSynchronousMessage(std::string cmd);
  void sendAsynchronousMessage(std::string cmd);

  bool isTopOfTheStates(GameState* i_state);

  int getMaxFps(){
    return m_maxFps;
  }

  // video recorder
  VideoRecorder* getVideoRecorder();

  /* ask to states to clean themself */
  static void cleanStates();

private:
  void calculateWhichStateIsRendered();
  void calculateFps();
  bool doRender();
  void drawFps();
  void drawStack();
  void drawCursor();

  bool m_isVisible;
  bool m_hasFocus;

  std::vector<GameState*> m_statesStack;

  // last time the m_current*Fps have been filled
  int m_lastFpsTime;

  // contains the number of frame during the last second
  int m_currentRenderFps;
  int m_currentUpdateFps;

  // counting the number of frame for the current second
  int m_renderFpsNbFrame;
  int m_updateFpsNbFrame;

  // the hz at which the state manager runs
  // (the highest from the states)
  int m_maxUpdateFps;
  int m_maxRenderFps;
  int m_maxFps;

  // to render depending on the max Hz
  float m_renderCounter;
  // the desired rendering fps
  int m_curRenderFps;
  // how many max fps beat for one render
  float m_renderPeriod;

  // cursor
  Texture* m_cursor;

  // video
  VideoRecorder* m_videoRecorder;

};

#endif
