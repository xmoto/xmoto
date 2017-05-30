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

#include "TFunctor.h"

  template<typename T> class MultiSingleton {
  public:

    static T* instance(){
      T* returnObject;

      if(m_pMutex == NULL){
	    m_pMutex = SDL_CreateMutex();
      }

      SDL_LockMutex(m_pMutex);

      if(m_defaultKey == ""){
          m_defaultKey = "default";
      }

      typename std::map<std::string, T*>::iterator iter;
      iter = m_instances.find(m_defaultKey);

      if(iter == m_instances.end()){
	    m_instances[m_defaultKey] = new T();
	    m_enabledPropagations[m_defaultKey] = false;
	    //printf("New instance (%s => %X)\n", key.c_str(), m_instances[key]);
      }

      returnObject = m_instances[m_defaultKey];
      SDL_UnlockMutex(m_pMutex);
      return returnObject;
    }


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
	    m_enabledPropagations[key] = false;
        if(m_defaultKey == ""){
            m_defaultKey = key;
        }
	    //printf("New instance (%s => %X)\n", key.c_str(), m_instances[key]);
      }

      returnObject = m_instances[key];

      SDL_UnlockMutex(m_pMutex);

      return returnObject;
    }
    
    static void destroy()
    {
      if(m_pMutex == NULL)
      return;

      SDL_LockMutex(m_pMutex);

      typename std::map<std::string, T*>::iterator iter;
      typename std::map<std::string, bool>::iterator iter2;
      iter = m_instances.find(m_defaultKey);
      iter2 = m_enabledPropagations.find(m_defaultKey);

      if(iter != m_instances.end()){
	    delete iter->second;
	    m_instances.erase(iter);
        m_enabledPropagations.erase(iter2);
      }

      SDL_UnlockMutex(m_pMutex);

      if(m_instances.size() == 0){
	    SDL_DestroyMutex(m_pMutex);
	    m_pMutex = NULL;
        m_defaultKey = "";
      }
      else {
        m_defaultKey = m_instances.begin()->first;
      }

    }
    
    static void destroy(std::string key)
    {
      if(m_pMutex == NULL)
      return;

      SDL_LockMutex(m_pMutex);

      typename std::map<std::string, T*>::iterator iter;
      typename std::map<std::string, bool>::iterator iter2;
      iter = m_instances.find(key);
      iter2 = m_enabledPropagations.find(key);

      if(iter != m_instances.end()){
	    delete iter->second;
	    m_instances.erase(iter);
        m_enabledPropagations.erase(iter2);
      }

      SDL_UnlockMutex(m_pMutex);

      if(m_instances.size() == 0){
	    SDL_DestroyMutex(m_pMutex);
	    m_pMutex = NULL;
      }

    }
    
    static void setDefaultInstance(std::string key){
        m_defaultKey = key;
    }

    static void enablePropagation(std::string key){
	    m_enabledPropagations[key] = true;
    }

    static void disablePropagation(std::string key){
	    m_enabledPropagations[key] = false;
    }

    bool isPropagator(){
        return m_isPropagator;
    }
    
    void setIsPropagator(bool value){
        m_isPropagator = value;
    }

    static void propagate(MultiSingleton<T> * caller,TFunctor<T>* method){
        caller->setIsPropagator(true);
        
        if(m_isPropagating != true){
            m_isPropagating = true;
            
            typename std::map<std::string, T*>::iterator iter;
            typename std::map<std::string, bool>::iterator iter2;
            
            for( iter = m_instances.begin(); iter != m_instances.end(); ++iter ) {
                if( iter->second->isPropagator() == false ){
                    iter2 = m_enabledPropagations.find(iter->first);

                    if( iter2->second == true ){
                        method->call(iter->second);
                    }
                }
            }

            m_isPropagating = false;
        }

        delete method; // freeing resources
        caller->setIsPropagator(false);
    }

  protected:
    MultiSingleton() {}
    virtual ~MultiSingleton() {}

  private:
    static std::map<std::string, T*> m_instances;
    static std::map<std::string, bool> m_enabledPropagations;
    static SDL_mutex* m_pMutex;
    static bool m_isPropagating;
    static std::string m_defaultKey;
    bool m_isPropagator;
  };

template<typename T> std::map<std::string, T*>              MultiSingleton<T>::m_instances;
template<typename T> std::map<std::string, bool>            MultiSingleton<T>::m_enabledPropagations;
template<typename T> SDL_mutex*                             MultiSingleton<T>::m_pMutex = NULL;
template<typename T> std::string                            MultiSingleton<T>::m_defaultKey = "";
template<typename T> bool                                   MultiSingleton<T>::m_isPropagating = false;
//template<typename T> bool                                   MultiSingleton<T>::m_isPropagator = false;

#endif
