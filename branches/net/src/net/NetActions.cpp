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

#include "NetActions.h"
#include "../SysMessage.h"
#include "../helpers/VExcept.h"
#include "../helpers/Log.h"
#include "NetClient.h"
#include "../Universe.h"
#include "../Theme.h"
#include "../xmscene/BikeGhost.h"
#include <sstream>

char NetAction::m_buffer[NETACTION_MAX_PACKET_SIZE];
unsigned int NetAction::m_biggestPacket = 0;
unsigned int NetAction::m_nbPacketsSent = 0;

std::string NA_chatMessage::ActionKey = "message";
std::string NA_frame::ActionKey       = "frame";

NetAction::NetAction() {
}

NetAction::~NetAction() {
}

void NetAction::logStats() {
  LogInfo("net: number of packets sent : %u", NetAction::m_nbPacketsSent);
  LogInfo("net: biggest packet sent : %u bytes", NetAction::m_biggestPacket);
}

void NetAction::send(TCPsocket* i_sd, const void* subPacketData, int subPacketLen) {
  unsigned int nread;
  std::string v_data;
  
  int v_subPacketSize = actionKey().length() + 1 + subPacketLen + 1;
  std::ostringstream v_nb;
  v_nb << v_subPacketSize;
  unsigned int v_totalPacketSize = v_nb.str().length() + 1 + v_subPacketSize;

  if(v_totalPacketSize > NetAction::m_biggestPacket) {
    NetAction::m_biggestPacket = v_totalPacketSize;
  }

  if(v_totalPacketSize > NETACTION_MAX_PACKET_SIZE) {
    throw Exception("net: too big packet to send");
  }

  snprintf(m_buffer, NETACTION_MAX_PACKET_SIZE, "%s\n%s\n", v_nb.str().c_str(), actionKey().c_str());
  memcpy(m_buffer + v_nb.str().length() + 1 + actionKey().length() + 1, subPacketData, subPacketLen);
  m_buffer[v_totalPacketSize-1] = '\n';

  // don't send the \0
  if( (nread = SDLNet_TCP_Send(*i_sd, m_buffer, v_totalPacketSize)) != v_totalPacketSize) {
    throw Exception("TCP_Send failed");
  }
  NetAction::m_nbPacketsSent++;
}

NetAction* NetAction::newNetAction(void* data, unsigned int len) {
  if(isCommand(data, len, NA_chatMessage::ActionKey)) {
    return new NA_chatMessage(((char*)data)+(NA_chatMessage::ActionKey.size()+1),
			      len          -(NA_chatMessage::ActionKey.size()+1));
  } 

  else if(isCommand(data, len, NA_frame::ActionKey)) {
    return new NA_frame(((char*)data)+(NA_frame::ActionKey.size()+1),
			      len    -(NA_frame::ActionKey.size()+1));
  } 
  
  else {
    throw Exception("client: invalid command");
  }
}

bool NetAction::isCommand(void* data, unsigned int len, const std::string& i_cmd) {
  if(i_cmd.length() + 1 > len) {
    return false;
  }

  if(((char*) data)[i_cmd.length()] != '\n') {
    return false;
  }

  return strncmp((char*)data, i_cmd.c_str(), i_cmd.length()) == 0;
}

NA_chatMessage::NA_chatMessage(const std::string& i_msg) {
  m_msg = i_msg;
}

NA_chatMessage::NA_chatMessage(void* data, unsigned int len) {
  ((char*)data)[len-1] = '\0';
  m_msg = std::string((char*)data);
  ((char*)data)[len-1] = '\n';
}

NA_chatMessage::~NA_chatMessage() {
}

void NA_chatMessage::execute(NetClient* i_netClient) {
  SysMessage::instance()->displayInformation(m_msg);
}

void NA_chatMessage::send(TCPsocket* i_sd) {
  NetAction::send(i_sd, m_msg.c_str(), m_msg.size()); // don't send the \0
}

std::string NA_chatMessage::getMessage() {
  return m_msg;
}

NA_frame::NA_frame(SerializedBikeState* i_state) {
  m_state = *i_state;
}

NA_frame::NA_frame(void* data, unsigned int len) {
  if(len-1 != sizeof(SerializedBikeState)) {
    throw Exception("Invalid NA_frame");
  }
  memcpy(&m_state, data, len-1); // -1 because in the protocol, you always finish by a \n
}

NA_frame::~NA_frame() {
}

void NA_frame::execute(NetClient* i_netClient) {
  //LogInfo("Frame received");
  NetGhost* v_ghost;
  Universe* v_universe = i_netClient->getUniverse();

  /*

    NEED A MUTEX
    
   */

  if(i_netClient->NetGhosts().size() == 0) {
    /* add the net ghost */
    if(v_universe != NULL) {
      for(unsigned int i=0; i<v_universe->getScenes().size(); i++) {
	v_ghost = v_universe->getScenes()[i]->addNetGhost("Net ghost", Theme::instance(),
							  Theme::instance()->getGhostTheme(),
							  TColor(255,255,255,0),
							  TColor(GET_RED(Theme::instance()->getGhostTheme()->getUglyRiderColor()),
								 GET_GREEN(Theme::instance()->getGhostTheme()->getUglyRiderColor()),
								 GET_BLUE(Theme::instance()->getGhostTheme()->getUglyRiderColor()),
								 0)
							  );
	i_netClient->addNetGhost(v_ghost);
      }
    }
  }
  
  if(i_netClient->getUniverse() != NULL) {
    // take the physic of the first world
    if(i_netClient->getUniverse()->getScenes().size() > 0) {
      BikeState::convertStateFromReplay(&m_state, i_netClient->NetGhosts()[0]->getState(),
					i_netClient->getUniverse()->getScenes()[0]->getPhysicsSettings());
    }
  }
}

void NA_frame::send(TCPsocket* i_sd) {
  NetAction::send(i_sd, &m_state, sizeof(SerializedBikeState));
}

SerializedBikeState NA_frame::getState() {
  return m_state;
}
