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

#ifndef __STATEDEADMENU_H__
#define __STATEDEADMENU_H__

#include "StateManager.h"
#include "StateMenu.h"

class UIRoot;
class Universe;

class StateDeadMenu : public StateMenu {
  public:
  StateDeadMenu(Universe* i_universe,
		bool i_doShadeAnim,
		bool drawStateBehind    = true,
		bool updateStatesBehind = false
		);
  virtual ~StateDeadMenu();
  
  virtual void enter();
  virtual void leave();
  
  /* input */
  virtual void xmKey(InputEventType i_type, const XMKey& i_xmkey);

  static  void clean();

  virtual void sendFromMessageBox(const std::string& i_id, UIMsgBoxButton i_button, const std::string& i_input);
  virtual void executeOneCommand(std::string cmd, std::string args);

 protected:
  virtual void checkEvents();
  Universe* m_universe;

  private:
  /* GUI */
  static UIRoot* m_sGUI;
  static void createGUIIfNeeded();

  std::string m_replayName; // to save temporarly the replay name
};

#endif
