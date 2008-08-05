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

#include "StateCheckWww.h"
#include "../thread/CheckWwwThread.h"

StateCheckWww::StateCheckWww(bool forceUpdate,
			     bool drawStateBehind,
			     bool updateStatesBehind)
  : StateUpdate(drawStateBehind, updateStatesBehind)
{
  m_pThread = new CheckWwwThread(forceUpdate);
  m_name    = "StateCheckWww";
}

StateCheckWww::~StateCheckWww()
{
  delete m_pThread;
}

void StateCheckWww::callAfterThreadFinished(int threadResult)
{
  m_msg = ((CheckWwwThread*)m_pThread)->getMsg();
}

void StateCheckWww::keyDown(SDLKey nKey, SDLMod mod,int nChar, const std::string& i_utf8Char) {
  if(nKey == SDLK_k && (mod & KMOD_CTRL) != 0) {
    if(m_threadStarted == true) {
      m_pThread->safeKill();
    }
  }
}
