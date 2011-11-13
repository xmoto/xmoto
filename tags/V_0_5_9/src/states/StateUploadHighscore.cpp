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

#include "StateUploadHighscore.h"
#include "../thread/UploadHighscoreThread.h"

StateUploadHighscore::StateUploadHighscore(const std::string& i_replayPath,
					   bool drawStateBehind,
					   bool updateStatesBehind)
  : StateUpdate(drawStateBehind, updateStatesBehind)
{
  m_pThread          = new UploadHighscoreThread(i_replayPath);
  m_name             = "StateUploadHighscore";
  m_messageOnSuccess = true;
}

StateUploadHighscore::~StateUploadHighscore()
{
  delete m_pThread;
}

void StateUploadHighscore::callAfterThreadFinished(int threadResult)
{
  m_msg = ((UploadHighscoreThread*) m_pThread)->getMsg();
}
