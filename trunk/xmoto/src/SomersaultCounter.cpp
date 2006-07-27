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

#include "SomersaultCounter.h"

SomersaultCounter::SomersaultCounter() {
  init();
}

SomersaultCounter::~SomersaultCounter() {
}

void SomersaultCounter::init() {
  m_last_initialized = false;
  m_last_state   = -1;
  m_min_move     = 0;
  m_max_move     = 0;
  m_current_move = 0;
  m_nbClockwise  = 0;
  m_nbCounterClockwise = 0;
}

int SomersaultCounter::getTotalClockwise() {
  return m_nbClockwise;
}

int SomersaultCounter::getTotalCounterClockwise() {
  return m_nbCounterClockwise;
}

int SomersaultCounter::getTotal() {
  return getTotalCounterClockwise() + getTotalClockwise();
}

bool SomersaultCounter::update(vapp::Vector2f p_wheel1, vapp::Vector2f p_wheel2) {
  int v_current_state;
  bool v_res;

  v_res = false;
  v_current_state = getCurrentState(p_wheel1, p_wheel2);

  /* update the last states */
  if(m_last_initialized == false) {
    m_last_state  = v_current_state;
    m_last_initialized = true;
    return false;
  }

  if(v_current_state == m_last_state) { /* update only if the state change */
    return false;
  }

  /* update last state */
  int v_diff = ((v_current_state > m_last_state &&
                 (m_last_state == 1 && v_current_state == 4) == false)
                || (m_last_state == 4 && v_current_state == 1)) ? 1 : -1;
  m_current_move += v_diff;
  m_min_move = m_current_move < m_min_move ? m_current_move : m_min_move;
  m_max_move = m_current_move > m_max_move ? m_current_move : m_max_move;
    
  v_res = m_max_move - m_min_move >= 4;
  if(v_res) {
    /* increase number of somersaults */
    if(v_diff == -1) {
      m_nbClockwise++;
    } else {
      m_nbCounterClockwise++;
    }

    // reinit to detect a new one
    m_min_move     = 0;
    m_max_move     = 0;
    m_current_move = 0;
  }

  /* update last move */
  m_last_state =  v_current_state;

  return v_res;
}


int SomersaultCounter::getCurrentState(vapp::Vector2f p_wheel1, vapp::Vector2f p_wheel2) {
  if(p_wheel1.x <= p_wheel2.x && p_wheel1.y < p_wheel2.y) {
    return 1;
  }

  if(p_wheel1.y <= p_wheel2.y) {
    return 2;
  }

  if(p_wheel1.x >= p_wheel2.x) {
    return 3;
  }

  return 4;
}
