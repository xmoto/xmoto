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

#ifndef __STATECREDITSMODE_H__
#define __STATECREDITSMODE_H__

#include "StateReplaying.h"

class Credits;
class ReplayBiker;

class StateCreditsMode : public StateReplaying {
  public:
  StateCreditsMode(Universe* i_universe, const std::string& i_replay, ReplayBiker* i_replayBiker);
  virtual ~StateCreditsMode();
  
  virtual void enter();
  
  virtual bool render();
  /* input */
  virtual void keyDown(SDLKey nKey, SDLMod mod,int nChar, const std::string& i_utf8Char);
  virtual void keyUp(SDLKey nKey,   SDLMod mod, const std::string& i_utf8Char);
  virtual void mouseDown(int nButton);
  virtual void mouseDoubleClick(int nButton);
  virtual void mouseUp(int nButton);

  virtual void joystickAxisMotion(Uint8 i_joyNum, Uint8 i_joyAxis, Sint16 i_joyAxisValue);
  virtual void joystickButtonDown(Uint8 i_joyNum, Uint8 i_joyButton);
  virtual void joystickButtonUp(Uint8 i_joyNum, Uint8 i_joyButton);
  
  private:
  Credits* m_credits;

  void abort();
};

#endif
