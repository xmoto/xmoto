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

#ifndef __STATEPREPLAYINGREPLAY_H__
#define __STATEPREPLAYINGREPLAY_H__

#include "StatePreplaying.h"

class ReplayBiker;

class StatePreplayingReplay : public StatePreplaying {
public:
  StatePreplayingReplay(const std::string i_replay, bool i_sameLevel);
  virtual ~StatePreplayingReplay();

protected:
  virtual void initUniverse();
  virtual void preloadLevels();
  virtual void initPlayers();
  virtual void runPlaying();

  std::string m_replay;
  ReplayBiker *m_replayBiker; /* replay watched */
};

#endif
