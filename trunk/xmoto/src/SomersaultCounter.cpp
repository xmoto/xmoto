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
  m_initialized = false;
}

SomersaultCounter::~SomersaultCounter() {
}

void SomersaultCounter::init() {
  m_initialized = false;
}

bool SomersaultCounter::update(vapp::Vector2f p_wheel1, vapp::Vector2f p_wheel2) {
  int v_current_state;
  bool v_res;

  v_current_state = getCurrentState(p_wheel1, p_wheel2);

  /* update the last states */
  if(m_initialized == false) {
    initWith(v_current_state);
    m_initialized = true;
  } else {
    if(v_current_state != m_state[4]) { /* update only if the state change */
      /* update last states */
      for(int i=1; i<5; i++) {
	m_state[i-1] = m_state[i];
      }
      m_state[4] = v_current_state;
    }
  }    

  v_res = isDetected();
  if(v_res) {
    //for(int i=0; i<5; i++) {
    //  printf("[%i]", m_state[i]);
    //}
    //printf("\n");
    // reinit to detect a new one
    initWith(v_current_state);
  }
  return v_res;
}

void SomersaultCounter::initWith(int s) {
  for(int i=0; i<5; i++) {
    m_state[i] = s;
  }
}

bool SomersaultCounter::isDetected() {

  /* all the states must be differents */
  return (m_state[0] != m_state[1] &&
	  m_state[0] != m_state[2] &&
	  m_state[0] != m_state[3] &&
	  m_state[0] == m_state[4] &&
	  m_state[1] != m_state[2] &&
	  m_state[1] != m_state[3] &&
	  m_state[1] != m_state[4] &&
	  m_state[2] != m_state[3] &&
	  m_state[2] != m_state[4] &&
	  m_state[3] != m_state[4]
	  );
}

int SomersaultCounter::getCurrentState(vapp::Vector2f p_wheel1, vapp::Vector2f p_wheel2) {
  if(p_wheel1.x <= p_wheel2.x && p_wheel1.y < p_wheel2.y) {
    return 1;
  }

  if(p_wheel1.x > p_wheel2.x && p_wheel1.y <= p_wheel2.y) {
    return 2;
  }

  if(p_wheel1.x >= p_wheel2.x && p_wheel1.y > p_wheel2.y) {
    return 3;
  }

  return 4;
}
