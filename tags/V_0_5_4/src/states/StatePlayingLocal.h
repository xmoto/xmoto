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

#ifndef __STATEPLAYINGLOCAL_H__
#define __STATEPLAYINGLOCAL_H__

#include "StatePlaying.h"

class StatePlayingLocal : public StatePlaying {
  public:
  StatePlayingLocal(Universe* i_universe, GameRenderer* i_renderer);
  virtual ~StatePlayingLocal();
  
  virtual void enter();
  virtual void leave();
  /* called when a new state is pushed or poped on top of the
     current one*/
  virtual void enterAfterPop();
  
  virtual bool update();
  virtual void abortPlaying();
  virtual void nextLevel(bool i_positifOrder = true);
  virtual void restartLevel(bool i_reloadLevel = false);

  /* input */
  virtual void xmKey(InputEventType i_type, const XMKey& i_xmkey);

  private:
  void onOneFinish();
  void onAllDead();

  void handleInput(InputEventType Type, const XMKey& i_xmkey);

  bool m_gameIsFinished;
};

#endif
