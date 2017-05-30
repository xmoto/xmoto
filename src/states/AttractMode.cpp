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

#include "AttractMode.h"

AttractMode::AttractMode() {
  m_inAttractMode = false;
  m_quitKey = EVERY_KEY;
}

AttractMode::~AttractMode() {}

void AttractMode::enterAttractMode(int quitAttractKey) {
  m_inAttractMode = true;
  m_quitKey = quitAttractKey;
}

void AttractMode::exitAttractMode() {
  m_inAttractMode = false;
}

bool AttractMode::inAttractMode() {
  return m_inAttractMode;
}

void AttractMode::attractModeKeyDown(SDLKey nKey) {
  if (m_quitKey == NO_KEY) {
    return;
  } else if (m_quitKey == EVERY_KEY) {
    m_inAttractMode = false;
  } else if (m_quitKey == nKey) {
    m_inAttractMode = false;
  }
}
