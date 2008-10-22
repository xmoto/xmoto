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

#ifndef __FACTORY_H__
#define __FACTORY_H__

#include <map>
#include <string>

typedef void* (*objectConstructor)();

// createObjectHelper from the book "Game Programming Gems #5",
// article "Choose Your Path - A Menu System"
template<typename ClassType>
void* createObjectHelper() {
  return new ClassType;
}

class Factory {
public:
  virtual ~Factory() {}

#define REGISTER_OBJECT(name) registerObject(#name, \
                                             &createObjectHelper<name>)

  void registerObject(std::string name, objectConstructor cons) {
    m_registeredObjects[name] = cons;
  }

  void* createObject(std::string name) {
    std::map<std::string, objectConstructor>::iterator itFind;

    itFind = m_registeredObjects.find(name);
    if(itFind != m_registeredObjects.end()) {
      return ((*itFind).second)();
    } else {
      return NULL;
    }
  }

private:
  std::map<std::string, objectConstructor> m_registeredObjects;
};

#endif
