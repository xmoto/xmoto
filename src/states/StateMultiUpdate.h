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

#include "StateMenu.h"
#include "thread/XMThreads.h"
#include <map>

class UIRoot;

class StateMultiUpdate : public StateMenu {
public:
  StateMultiUpdate(bool drawStateBehind, bool updateStatesBehind);
  virtual ~StateMultiUpdate();

  virtual void enter();
  virtual void leave();

  virtual bool update();

  /* input */
  virtual void xmKey(InputEventType i_type, const XMKey &i_xmkey);

  static void clean();

protected:
  virtual void checkEvents();
  virtual void updateGUI();

  // for the message box when a thread badly finished
  void sendFromMessageBox(const std::string &i_id,
                          UIMsgBoxButton i_button,
                          const std::string &i_input);

  class ThreadInfos {
  public:
    bool m_threadStarted;
    bool m_threadFinished;

    // updated by the child class
    int m_progress;
    std::string m_currentOperation;
    std::string m_currentMicroOperation;

    // the message displayed in the message box if the thread failed
    std::string m_errorMessage;
  };

  void initThreadInfos(ThreadInfos *pInfos);

  XMThreads m_threads;
  std::map<std::string, ThreadInfos *> m_threadsInfos;
  int m_numberThreadDisplayed;
  int m_numberThreadRunning;

private:
  /* GUI */
  static UIRoot *m_sGUI;
  static void createGUIIfNeeded(RenderSurface *i_screen);

  void init();
};

#endif
