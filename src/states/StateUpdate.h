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

#include "../thread/XMThread.h"
#include "StateManager.h"
#include "StateWaiting.h"
#include <string>

class UIRoot;

class StateUpdate : public StateWaiting {
public:
  StateUpdate(bool drawStateBehind, bool updateStatesBehind);
  virtual ~StateUpdate();

  virtual void enter();
  virtual void leave();

  virtual bool update();
  virtual bool updateWhenUnvisible() { return true; }

  /* input */
  virtual void xmKey(InputEventType i_type, const XMKey &i_xmkey);

protected:
  bool m_threadStarted;
  bool m_threadFinished;

  // the thread
  XMThread *m_pThread;

  // the message displayed in the message box if the thread failed
  std::string m_msg;
  // display the message, even if the thread succed
  bool m_messageOnSuccess;
  bool m_messageOnSuccessModal;
  // dont display the message when the thread failed
  bool m_messageOnFailure;
  bool m_messageOnFailureModal;

  // for child customization
  virtual void callAfterThreadFinished(int threadResult);
  virtual bool callBeforeLaunchingThread();

  // for the message box when a thread badly finished
  virtual void sendFromMessageBox(const std::string &i_id,
                                  UIMsgBoxButton i_button,
                                  const std::string &i_input);

  virtual void onThreadFinishes(bool i_res);
};

#endif
