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

#ifndef __STATEUPDATE_H__
#define __STATEUPDATE_H__

#include "StateManager.h"
#include "StateMenu.h"
#include "thread/XMThread.h"
#include <string>

class UIRoot;

class StateUpdate : public StateMenu {
public:
  StateUpdate(GameApp* pGame,
	      bool drawStateBehind,
	      bool updateStatesBehind);			 
  virtual ~StateUpdate();

  virtual void enter();
  virtual void leave();

  virtual bool update();

  /* input */
  virtual void keyDown(int nKey, SDLMod mod,int nChar);
  virtual void keyUp(int nKey,   SDLMod mod);

  static void clean();

protected:
  virtual void checkEvents();
  virtual void updateGUI();
  bool m_threadStarted;

  // updated by the child class
  int         m_progress;
  std::string m_currentOperation;
  std::string m_currentMicroOperation;

  // the thread
  XMThread*   m_pThread;

  // the message displayed in the message box if the thread failed
  std::string m_errorMessage;

  // for child customization
  virtual bool callAfterThreadFinishedOk();
  virtual bool callBeforeLaunchingThread();

  // for the message box when a thread badly finished
  void send(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input);

private:
  /* GUI */
  static UIRoot* m_sGUI;
  static void createGUIIfNeeded(GameApp* pGame);

  void init();
};

#endif
