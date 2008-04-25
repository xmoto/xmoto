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

#ifndef __MULTISINGLETON_H__
#define __MULTISINGLETON_H__

#include <map>
#include <string>
#include <SDL/SDL_mutex.h>

  template<typename T> class MultiSingleton {
  public:
    static T* instance(std::string key)
    {
      T* returnObject;

      if(m_pMutex == NULL){
	m_pMutex = SDL_CreateMutex();
      }

      SDL_LockMutex(m_pMutex);

      typename std::map<std::string, T*>::iterator iter;
      iter = m_instances.find(key);

      if(iter == m_instances.end()){
	m_instances[key] = new T();
      }

      returnObject = m_instances[key];

      SDL_UnlockMutex(m_pMutex);

      return returnObject;
    }
    static void destroy(std::string key)
    {
      if(m_pMutex == NULL)
      return;

      SDL_LockMutex(m_pMutex);

      typename std::map<std::string, T*>::iterator iter;
      iter = m_instances.find(key);

      if(iter != m_instances.end()){
	delete iter->second;
	m_instances.erase(iter);
      }

      SDL_UnlockMutex(m_pMutex);

      if(m_instances.size() == 0){
	SDL_DestroyMutex(m_pMutex);
	m_pMutex = NULL;
      }

    }

  protected:
    MultiSingleton() {}
    virtual ~MultiSingleton() {}

  private:
    static std::map<std::string, T*> m_instances;
    static SDL_mutex* m_pMutex;
  };

template<typename T> std::map<std::string, T*> MultiSingleton<T>::m_instances;
template<typename T> SDL_mutex*                MultiSingleton<T>::m_pMutex = NULL;

#endif
