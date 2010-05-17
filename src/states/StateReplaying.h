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

#ifndef __STATEREPLAYING_H__
#define __STATEREPLAYING_H__

#include "StateScene.h"

class ReplayBiker;

class StateReplaying : public StateScene {
  public:
  StateReplaying(Universe* i_universe, GameRenderer* i_renderer, const std::string& i_replay, ReplayBiker* i_replayBiker);
  virtual ~StateReplaying();
  
  virtual void enter();
  virtual void leave();
  
  virtual bool update();
  virtual void nextLevel(bool i_positifOrder = true);

  /* input */
  virtual void xmKey(InputEventType i_type, const XMKey& i_xmkey);

  virtual void restartLevel(bool i_reloadLevel = false);

  std::string m_replay;
  ReplayBiker* m_replayBiker; /* replay watched */

  private:
  bool m_stopToUpdate;
};

#endif
