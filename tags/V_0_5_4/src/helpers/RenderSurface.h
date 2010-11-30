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

#ifndef __RENDERSURFACE_H__
#define __RENDERSURFACE_H__

#include "VMath.h"

/*
  the y coord increase this way :

  y

  ^
  |-------\
  | 1 | 2 |
  |-------|
  | 3 | 4 |
  |--------> x

  so a (0,0) coordinate is the lower left point
  on the screen (as used by glViewport)

  there 4 render surfaces on that screen example :
     downleft point      upright point
  1: (0, height/2)         (width/2, height)
  2: (width/2, height/2)   (width, height)
  3: (0, 0)                (width/2, height/2)
  4: (width/2, 0)          (width, height/2)

 */

class RenderSurface {
public:

  RenderSurface();
  RenderSurface(Vector2i downleft, Vector2i upright);

  void update(Vector2i downleft, Vector2i upright);
  Vector2i& size();
  int getDispWidth() const;
  int getDispHeight() const;
  Vector2i& downleft();
  Vector2i& upright();

private:
  Vector2i m_downleft;
  Vector2i m_upright;
  Vector2i m_size;
};

#endif
