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

#ifndef __STATEPLAYING_H__
#define __STATEPLAYING_H__

#include "StateScene.h"
#include "xmoto/Input.h"

class StatePlaying : public StateScene {
public:
  StatePlaying(Universe *i_universe, GameRenderer *i_renderer);
  virtual ~StatePlaying() = 0;

  virtual void enter();
  virtual void enterAfterPop();
  virtual void executeOneCommand(std::string cmd, std::string args);
  virtual bool renderOverShadow();

protected:
  void handleControllers(InputEventType Type, const XMKey &i_xmkey);
  void handleScriptKeys(InputEventType Type, const XMKey &i_xmkey);
  void dealWithActivedKeys(); // apply already pressed keys
  void updateWithOptions();

  bool m_displayStats;

private:
  bool m_changeDirKeyAlreadyPress[INPUT_NB_PLAYERS]; // to avoid key repetition
};

#endif
