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

#include "VirtualNetLevelsList.h"
#include "NetClient.h"
#include "../LevelsManager.h"

VirtualNetLevelsList::VirtualNetLevelsList(NetClient* i_client) {
    m_client = i_client;
    m_db = NULL; // must be set before used
}

VirtualNetLevelsList::~VirtualNetLevelsList() {
}

std::string VirtualNetLevelsList::determinePreviousLevel(const std::string& i_id_level) {
  for(int i=m_client->otherClients().size()-1; i>=0; i--) {
    if(m_client->otherClients()[i]->lastPlayingLevelId() == i_id_level) {
      // level found
      // now search the first level which is not this one
      for(int j=i; j>=0; j--) { // start from i to be sure there is at least one value
	if(m_client->otherClients()[j]->lastPlayingLevelId() != i_id_level &&
	   m_client->otherClients()[j]->lastPlayingLevelId() != "") {
	  if(LevelsManager::instance()->doesLevelExist(m_client->otherClients()[j]->lastPlayingLevelId(), m_db)) {
	    return m_client->otherClients()[j]->lastPlayingLevelId();
	  }
	}
      }
    }
  }

  // back to the last different because nothing found ?
  for(int i=m_client->otherClients().size()-1; i>=0; i--) {
    if(m_client->otherClients()[i]->lastPlayingLevelId() != "" &&
       m_client->otherClients()[i]->lastPlayingLevelId() != i_id_level) {
      if(LevelsManager::instance()->doesLevelExist(m_client->otherClients()[i]->lastPlayingLevelId(), m_db)) {
	return m_client->otherClients()[i]->lastPlayingLevelId();
      }
    }
  }

  // nothing found
  return "";
}

std::string VirtualNetLevelsList::determineNextLevel(const std::string& i_id_level) {
  for(unsigned int i=0; i<m_client->otherClients().size(); i++) {
    if(m_client->otherClients()[i]->lastPlayingLevelId() == i_id_level) {
      // level found
      // now search the first level which is not this one
      for(unsigned int j=i; j<m_client->otherClients().size(); j++) { // start from i to be sure there is at least one value
	if(m_client->otherClients()[j]->lastPlayingLevelId() != i_id_level &&
	   m_client->otherClients()[j]->lastPlayingLevelId() != "") {
	  if(LevelsManager::instance()->doesLevelExist(m_client->otherClients()[j]->lastPlayingLevelId(), m_db)) {
	    return m_client->otherClients()[j]->lastPlayingLevelId();
	  }
	}
      }
    }
  }

  // back to the first different because nothing found ?
  for(unsigned int i=0; i<m_client->otherClients().size(); i++) {
    if(m_client->otherClients()[i]->lastPlayingLevelId() != "" &&
       m_client->otherClients()[i]->lastPlayingLevelId() != i_id_level) {
      if(LevelsManager::instance()->doesLevelExist(m_client->otherClients()[i]->lastPlayingLevelId(), m_db)) {
	return m_client->otherClients()[i]->lastPlayingLevelId();
      }
    }
  }

  // nothing found
  return "";
}

void VirtualNetLevelsList::setDb(xmDatabase* pDb) {
  m_db = pDb;
}
