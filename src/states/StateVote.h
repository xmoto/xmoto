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

#ifndef __STATEVOTE_H__
#define __STATEVOTE_H__

#include "StateManager.h"
#include "StateMenu.h"

class StateVote : public StateMenu {
public:
  StateVote(const std::string &i_idlevel,
            bool drawStateBehind = true,
            bool updateStatesBehind = false);
  virtual ~StateVote();

  static void clean();

  virtual void enter();
  virtual void leave();

  virtual void xmKey(InputEventType i_type, const XMKey &i_xmkey);

protected:
  virtual void checkEvents();

private:
  std::string m_idlevel;

  /* GUI */
  static UIRoot *m_sGUI;
  static void createGUIIfNeeded(RenderSurface *i_screen);

  bool isToSkip();
  void updateRights();
};

#endif
