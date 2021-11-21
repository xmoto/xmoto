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

#include "StateSendVote.h"
#include "common/XMSession.h"
#include "thread/SendVoteThread.h"
#include "xmoto/input/Input.h"

StateSendVote::StateSendVote(const std::string &i_idlevel,
                             const std::string &i_difficulty_value,
                             const std::string &i_quality_value,
                             bool drawStateBehind,
                             bool updateStatesBehind)
  : StateUpdate(drawStateBehind, updateStatesBehind) {
  m_pThread = new SendVoteThread(
    i_idlevel,
    i_difficulty_value,
    i_quality_value,
    XMSession::instance()->adminMode(),
    XMSession::instance()->adminMode() ? XMSession::instance()->profile() : "",
    XMSession::instance()->adminMode() ? XMSession::instance()->wwwPassword()
                                       : "");
  m_name = "StateSendVote";

  m_messageOnSuccess = true;
  m_messageOnSuccessModal = false;
  m_messageOnFailure = true;
  m_messageOnFailureModal = false;
}

StateSendVote::~StateSendVote() {
  delete m_pThread;
}

void StateSendVote::callAfterThreadFinished(int threadResult) {
  m_msg = ((SendVoteThread *)m_pThread)->getMsg();
}

void StateSendVote::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (i_type == INPUT_DOWN &&
      i_xmkey == (*Input::instance()->getGlobalKey(INPUT_KILLPROCESS))) {
    if (m_threadStarted == true) {
      m_pThread->safeKill();
    }
  }
}
