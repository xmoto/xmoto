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

#ifndef __STATEUPLOADHIGHSCORE_H__
#define __STATEUPLOADHIGHSCORE_H__

#include "StateUpdate.h"

class StateUploadHighscore : public StateUpdate {
  public:
  StateUploadHighscore(GameApp* pGame,
		       const std::string& i_replayPath,
		       bool drawStateBehind    = true,
		       bool updateStatesBehind = false);
  virtual ~StateUploadHighscore();
  
  protected:
  virtual void callAfterThreadFinished(int threadResult);
};

#endif
