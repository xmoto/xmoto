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

#ifndef __STATEWAITING_H__
#define __STATEWAITING_H__

#include "../thread/XMThread.h"
#include "StateManager.h"
#include "StateMenu.h"
#include <string>

class UIRoot;

class StateWaiting : public StateMenu {
public:
  StateWaiting(bool drawStateBehind, bool updateStatesBehind);
  virtual ~StateWaiting();

  virtual void enter();
  virtual void leave();

  virtual bool update();
  virtual bool updateWhenUnvisible() { return true; }

  /* input */
  virtual void xmKey(InputEventType i_type, const XMKey &i_xmkey);

  static void clean();

protected:
  virtual void updateGUI();

  // updated by the child class
  int m_progress;
  std::string m_currentOperation;
  std::string m_currentMicroOperation;

private:
  /* GUI */
  static UIRoot *m_sGUI;
  static void createGUIIfNeeded(RenderSurface *i_screen);

  void init();
};

#endif
