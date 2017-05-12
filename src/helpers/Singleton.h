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

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <stdlib.h>

template<typename T> class Singleton {
public:
  static bool exists() {
    return m_pInstance != NULL;
  }

  static T* instance() {
    if(m_pInstance == NULL){
      m_pInstance = new T();
    }
    return m_pInstance;
  }

  static void destroy() {
    if(m_pInstance != NULL){
      delete m_pInstance;
      m_pInstance = NULL;
    }
  }

protected:
  Singleton() {
  }
  virtual ~Singleton() {
  }

private:
  static T* m_pInstance;
};

template<typename T> T* Singleton<T>::m_pInstance = NULL;

#endif
