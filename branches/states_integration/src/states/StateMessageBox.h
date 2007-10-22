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

#ifndef __STATEMESSAGEBOX_H__
#define __STATEMESSAGEBOX_H__

#include "StateManager.h"
#include "StateMenu.h"
#include "gui/basic/GUI.h"

class UIRoot;
class UIMsgBox;
class StateMessageBoxReceiver;

class StateMessageBox : public StateMenu {
  public:
  StateMessageBox(StateMessageBoxReceiver* i_receiver,
		  GameApp* pGame,
		  const std::string& i_text,
		  int i_buttons,
		  bool i_input = false,
		  bool i_query = false,
		  bool drawStateBehind    = true,
		  bool updateStatesBehind = false
		  );
  virtual ~StateMessageBox();
  
  virtual void enter();
  virtual void leave();
  /* called when a new state is pushed or poped on top of the
     current one*/
  virtual void enterAfterPop();
  virtual void leaveAfterPush();
  
  virtual bool update();
  virtual bool render();
  /* input */
  virtual void keyDown(int nKey, SDLMod mod,int nChar);
  virtual void keyUp(int nKey,   SDLMod mod);
  virtual void mouseDown(int nButton);
  virtual void mouseDoubleClick(int nButton);
  virtual void mouseUp(int nButton);
  
  static void clean();

 protected:
  virtual void checkEvents();

  private:
  UIMsgBox* m_msgbox;
  StateMessageBoxReceiver* m_receiver;
  UIMsgBoxButton m_clickedButton;

  void createGUI(GameApp* pGame, const std::string& i_text, int i_buttons, bool i_input, bool i_query);
};

#endif
