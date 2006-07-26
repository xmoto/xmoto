/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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

#ifndef __SOMERSAULTCOUNTER_H__
#define __SOMERSAULTCOUNTER_H__

#include "VMath.h"

class SomersaultCounter {
  public:
  SomersaultCounter();
  ~SomersaultCounter();

  void init();
  bool update(vapp::Vector2f p_wheel1, vapp::Vector2f p_wheel2);

  private:
  int getCurrentState(vapp::Vector2f p_wheel1, vapp::Vector2f p_wheel2);
  bool isDetected();
  void initWith(int s);

  bool m_initialized;
  /* five states : the five last one */
  /* five because we must come back to the original one */
  /*
       |
    4  |  3
  -----O------
    1  |  2
       |

  */
  int m_state[5];
};

#endif /* __SOMERSAULTCOUNTER_H__ */
