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

#ifndef __RANDOM_H__
#define __RANDOM_H__

#include <cstdlib>

// not so random
#define NB_RANDOM 1024
class NotSoRandom {
public:
  static void init() {
    for (unsigned int i = 0; i < NB_RANDOM; i++) {
      // from quake 3
      m_random[i] = ((rand() & 0x7fff) / ((float)0x7fff));
    }
  }

  static inline float randomNum(float min, float max) {
    m_current = (m_current + 1) & (NB_RANDOM - 1);
    return min + (max - min) * m_random[m_current];
  }

private:
  // random numbers in [0, 1]
  static float m_random[NB_RANDOM];
  static unsigned int m_current;
};

#endif
