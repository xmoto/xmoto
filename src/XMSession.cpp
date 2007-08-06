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

#include "XMSession.h"
#include "XMArgs.h"

XMSession::XMSession() {
  /* default config */
  m_verbose     = false;
  m_useGraphics = true;
}

void XMSession::load(const XMArguments* i_xmargs) {
  if(i_xmargs->isOptVerbose()) {
    m_verbose = true;
  }
  if(i_xmargs->isOptNoGfx()) {
    m_useGraphics = false;
  }
}

bool XMSession::isVerbose() const {
  return m_verbose;
}

bool XMSession::useGraphics() const {
  return m_useGraphics;
}

void XMSession::setUseGraphics(bool i_value) {
  m_useGraphics = i_value;
}
