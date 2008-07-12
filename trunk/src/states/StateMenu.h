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

#include "GameState.h"

class UIRoot;
class GameApp;

class StateMenu : public GameState {
 public:
  StateMenu(bool drawStateBehind,
	    bool updateStatesBehind,
	    bool i_doShade     = false,
	    bool i_doShadeAnim = true);
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
  virtual void keyDown(int nKey, SDLMod mod,int nChar, const std::string& i_utf8Char);
  virtual void keyUp(int nKey,   SDLMod mod, const std::string& i_utf8Char);
  virtual void mouseDown(int nButton);
  virtual void mouseDoubleClick(int nButton);
  virtual void mouseUp(int nButton);
  
 protected:
  virtual void checkEvents() = 0;
  UIRoot *m_GUI;

 private:
};

#endif
