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

#include "SomersaultCounter.h"

SomersaultCounter::SomersaultCounter() {
  init();
}

SomersaultCounter::~SomersaultCounter() {}

void SomersaultCounter::init() {
  m_last_initialized = false;
  m_last_state = 0;
  m_min_move = 0;
  m_max_move = 0;
  m_current_move = 0;
  m_nbClockwise = 0;
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

bool SomersaultCounter::update(double p_angle, bool &bCounterclock) {
  bool v_res;
  double v_diff;
  v_res = false;

  /* update the last states */
  if (m_last_initialized == false) {
    m_last_state = p_angle;
    m_last_initialized = true;
    return false;
  }

  v_diff = m_last_state - p_angle;
  /* break the circle */
  if (v_diff >= 3.14159f / 2.0) {
    v_diff -= 2.0 * 3.14159f;
  } else {
    if (v_diff <= -3.14159f / 2.0) {
      v_diff += 2.0 * 3.14159f;
    }
  }

  /* update only if the state change (at some degrees) */
  if (v_diff < 0.05 && v_diff > -0.05) {
    return false;
  }

  m_current_move += v_diff;
  m_min_move = m_current_move < m_min_move ? m_current_move : m_min_move;
  m_max_move = m_current_move > m_max_move ? m_current_move : m_max_move;

  v_res = m_max_move - m_min_move >= 2 * 3.14159f;

  // printf("min=%.2f, max=%.2f, diff=%.2f\n", m_min_move, m_max_move, v_diff);

  if (v_res) {
    /* increase number of somersaults */
    if (v_diff > 0.0) {
      m_nbClockwise++;
      bCounterclock = true;
    } else {
      m_nbCounterClockwise++;
      bCounterclock = false;
    }

    // reinit to detect a new one
    m_min_move = 0.0;
    m_max_move = 0.0;
    m_current_move = 0.0;
  }

  /* update last move */
  m_last_state = p_angle;

  return v_res;
}
