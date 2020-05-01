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

#ifndef __GAMESTATE_H__
#define __GAMESTATE_H__

#include "StateMessageBoxReceiver.h"
#include "helpers/RenderSurface.h"
#include "xmoto/XMKey.h"
#include <queue>
#include <string>
#include <utility>

class GameState : public StateMessageBoxReceiver {
public:
  GameState(bool drawStateBehind, bool updateStatesBehind);
  virtual ~GameState();

  void setScreen(const RenderSurface &i_screen);
  RenderSurface *getScreen();

  virtual void enter();
  virtual void leave() {}
  virtual void leaveType() {
  } // you can give a type to states ; if a state is leaved but not replaced by
  // a state of the same type, this method is called
  /* called when a new state is pushed or poped on top of the
     current one */
  virtual void enterAfterPop();
  virtual void leaveAfterPush() {}

  // return true if update/render was done
  virtual bool update() { return false; }
  virtual bool render();
  virtual bool renderOverShadow(); // function to render over the shadow
  virtual bool updateWhenUnvisible() { return false; }
  virtual void onRenderFlush() {}

  /* input */
  virtual void xmKey(InputEventType i_type, const XMKey &i_xmkey);

  bool isHide() { return m_isHide; }
  void setHide(bool isHide) { m_isHide = isHide; }

  bool drawStatesBehind() { return m_drawStateBehind; }
  bool updateStatesBehind() { return m_updateStatesBehind; }

  bool requestForEnd() { return m_requestForEnd; }

  int getUpdateFps() { return m_updateFps; }

  int getRenderFps() { return m_renderFps; }

  void setCurrentRenderFps(int curRenFps) { m_curRenderFps = curRenFps; }

  void setMaxFps(int maxFps) {
    m_maxFps = maxFps;
    m_updatePeriod = (float)m_maxFps / (float)m_updateFps;
  }

  // useful to be sure to replace one state by another (and not always replace
  // the upper state)
  void setStateId(const std::string &i_id);
  std::string getStateId() const;

  //
  void setStateType(const std::string &i_type);
  std::string getStateType() const;

  virtual void sendFromMessageBox(const std::string &i_id,
                                  UIMsgBoxButton i_button,
                                  const std::string &i_input);
  virtual void send(const std::string &i_message, const std::string &i_args);

  std::string getName() const { return m_name; }

  std::string getType() const { return m_type; }

  void simpleMessage(const std::string &msg);

  bool showCursor() { return m_showCursor; }
  void executeCommands();
  virtual void executeOneCommand(std::string cmd, std::string args);

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

  RenderSurface m_screen;

  std::string m_name;
  std::string m_type; // some state can have the same type (exemple, all the
  // state scene are of type scene)
  bool m_showCursor;

  void addCommand(std::string cmd, std::string args = "");

private:
  bool m_isHide;
  bool m_isSafeMode;
  bool m_drawStateBehind;
  bool m_updateStatesBehind;
  std::string m_stateId;
  std::string m_stateType; // type - to be able to have only one instance of one
  // state type (server console, chat box, ...)

  std::queue<std::pair<std::string, std::string>> m_commands;
  SDL_mutex *m_commandsMutex;
};

#endif
