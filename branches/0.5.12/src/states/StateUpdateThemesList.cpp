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

#include "../GameText.h"
#include "StateUpdateThemesList.h"
#include "../thread/UpdateThemesListThread.h"
#include "../Input.h"

StateUpdateThemesList::StateUpdateThemesList(bool drawStateBehind,
					     bool updateStatesBehind)
  : StateUpdate(drawStateBehind, updateStatesBehind)
{
  m_pThread = new UpdateThemesListThread();
  m_name    = "StateUpdateThemesList";
  m_msg     = GAMETEXT_FAILEDUPDATETHEMESLIST + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW;
}

StateUpdateThemesList::~StateUpdateThemesList()
{
  delete m_pThread;
}

void StateUpdateThemesList::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  if(i_type == INPUT_DOWN && i_xmkey == (*InputHandler::instance()->getGlobalKey(INPUT_KILLPROCESS))) {
    m_pThread->safeKill();
  }
}
