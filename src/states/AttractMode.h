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

#ifndef __ATTRACTMODE_H__
#define __ATTRACTMODE_H__

#define EVERY_KEY -1
#define NO_KEY -2

#include "common/VCommon.h"
#include "include/xm_SDL.h"

class AttractMode {
public:
  AttractMode();
  virtual ~AttractMode();

  void enterAttractMode(int quitAttractKey);
  void exitAttractMode();

  bool inAttractMode();

  void attractModeKeyDown(SDLKey nKey);

protected:
  bool m_inAttractMode;
  int  m_quitKey;
};

#endif
