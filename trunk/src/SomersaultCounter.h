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

#ifndef __SOMERSAULTCOUNTER_H__
#define __SOMERSAULTCOUNTER_H__

#include "helpers/VMath.h"

class SomersaultCounter {
  public:
  SomersaultCounter();
  ~SomersaultCounter();

  void init();
  bool update(double p_angle, bool &bCounterclock);

  int getTotalClockwise();
  int getTotalCounterClockwise();
  int getTotal();

  private:
  bool m_last_initialized;
  double m_last_state;
  double m_min_move;
  double m_max_move;
  double m_current_move;

  int m_nbClockwise;
  int m_nbCounterClockwise;
};

#endif /* __SOMERSAULTCOUNTER_H__ */
