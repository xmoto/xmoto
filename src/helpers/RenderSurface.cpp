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

#include "RenderSurface.h"

RenderSurface::RenderSurface() {}

RenderSurface::RenderSurface(Vector2i downleft, Vector2i upright) {
  update(downleft, upright);
}

void RenderSurface::update(Vector2i downleft, Vector2i upright) {
  m_downleft = downleft;
  m_upright = upright;
  m_size = upright - downleft;
}

Vector2i &RenderSurface::size() {
  return m_size;
}

int RenderSurface::getDispWidth() const {
  return m_size.x;
}

int RenderSurface::getDispHeight() const {
  return m_size.y;
}

Vector2i &RenderSurface::downleft() {
  return m_downleft;
}

Vector2i &RenderSurface::upright() {
  return m_upright;
}
