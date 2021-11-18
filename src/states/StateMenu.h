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

#ifndef __STATEMENU_H__
#define __STATEMENU_H__

#include <array>
#include "include/xm_SDL.h"

#include "GameState.h"

class UIRoot;
class GameApp;

class StateMenu : public GameState {
public:
  StateMenu(bool drawStateBehind, bool updateStatesBehind);
  virtual ~StateMenu();

  virtual void enter();
  virtual void leave();
  /* called when a new state is pushed or poped on top of the
     current one*/
  virtual void enterAfterPop();
  virtual void leaveAfterPush();

  virtual bool update();
  virtual bool render();
  /* input */
  virtual void xmKey(InputEventType i_type, const XMKey &i_xmkey);

  void handleJoyAxis(JoyAxisEvent event);
  void clearRepeatTimer(SDL_TimerID &timer);

  UIRoot *getGUI() const { return m_GUI; }
  JoyAxisEvent getJoystickRepeat() const { return m_joystickRepeat; }

  static Uint32 repeatTimerCallback(Uint32 interval, void *param);

  void resetJoyAxis(JoyAxis &axis);
  void resetJoyAxes();

private:
  using JoyAxes = std::array<JoyAxis, SDL_CONTROLLER_AXIS_MAX>;
  std::vector<JoyAxes> m_joyAxes;
  JoyAxisEvent m_joystickRepeat;

  JoyAxes &getAxesByJoyIndex(uint8_t index) { return m_joyAxes[index]; }

protected:
  virtual void checkEvents() = 0;
  UIRoot *m_GUI;
};

#endif
