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

#ifndef __ISERIALIZABLE_H__
#define __ISERIALIZABLE_H__

#include <vector>

// T must have a method 'int getTime()'
template <typename T> class ISerializable {
public:
  ISerializable() {
    initReplaying();
  }
  virtual ~ISerializable() {
    std::vector<T*>::iterator it = m_states.begin();
    while(it != m_states.end()) {
      delete (*it);
      ++it;
    }
    m_states.erase();
  }

  void initReplaying() {
    m_curPos  = 0;
    m_nbFrame = m_states.size();
    m_curTime = 0;
  }

  //---------------
  // Serialize

  // add T members in the DBuffer
  virtual void serializeCurrentState(DBuffer& buffer) = 0;

  // to know if we need to store the object state
  bool hasMoved() {
    return m_hasMoved;
  }
  void hasMoved(bool moved) {
    m_hasMoved = moved;
  }

  //---------------
  // Unserialize

  // get T from the DBuffer
  virtual void unserializeOneState(DBuffer& buffer, T& state) = 0;

  // after unserializing one state, add it to the frames of the object
  void addState(T* pState) {
    m_states.push_back(pState);
  }

  //---------------
  // Playing

  // forward and backward
  void jumpToTime(int timeStamp) {
    if(timeStamp > m_curTime) {
      while(m_curPos+1 < m_nbFrame
	    && m_states[m_curPos+1]->getTime() < timeStamp) {
	m_curPos++;

	applyState(m_states[m_curPos]);
      }
    } else if(timeStamp < m_curTime) {
      while(m_curPos-1 > 0
	    && m_states[m_curPos-1]->getTime() > timeStamp) {
	m_curPos--;

	applyState(m_states[m_curPos]);
      }
    }
    m_curTime = timeStamp;
  }


protected:
  // update ISerializable child with the informations from state
  virtual void applyState(T* state) = 0;

  std::vector<T*> m_states;

private:
  bool m_hasMoved;

  // the timestamp we jumped to (may differs from the one in the
  // current frame)
  int m_curTime;
  unsigned int m_curPos;
  unsigned int m_nbFrame;
};

#endif
