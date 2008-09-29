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

#include "ClientListenerThread.h"
#include "../NetClient.h"
#include "../../helpers/Log.h"
#include "../../helpers/VExcept.h"

#define XM_CLIENT_BUFFER_SIZE 1024
#define XM_CLIENT_MAX_PACKET_SIZE_DIGITS 6 // limit the size of a command : n digits

ClientListenerThread::ClientListenerThread(NetClient* i_netClient) {
    m_netClient = i_netClient;
}

ClientListenerThread::~ClientListenerThread() {
}

int ClientListenerThread::realThreadFunction() {
  int nread;
  char buffer[XM_CLIENT_BUFFER_SIZE];
  std::string v_cmd;
  unsigned int v_packetSize;
  unsigned int v_cmdStart;

  while( (nread = SDLNet_TCP_Recv(*(m_netClient->socket()),
			       buffer, XM_CLIENT_BUFFER_SIZE)) > 0) {
    v_packetSize = getSubPacketSize(buffer, nread, v_cmdStart);
    LogInfo("Packet received, subpacket size is : %i", v_packetSize);
    if(v_packetSize <= 0) {
      return 1; // invalid command
    }
    
    try {
      if(nread-v_cmdStart == v_packetSize) {
	manageClientSubPacket(((char*)buffer)+v_cmdStart, nread-v_cmdStart);
      } else {
	LogError("client: packet concatenation still not supported");
	// todo : concat buffers if buffer doesn't make a full command
      }
    } catch(Exception &e) {
      LogError("client: bad command received");
      return 1; // invalid command, stop the listener
    }
  }

  return 1;
}

void ClientListenerThread::manageClientSubPacket(void* data, unsigned int len) {
  if(isCommand(data, len, "message")) {
    LogInfo("client[message]:%s", getChatMessage(((char*)data)+8, len-8).c_str());
  } else {
    throw Exception("client: invalid command");
  }
}

unsigned int ClientListenerThread::getSubPacketSize(void* data, unsigned int len, unsigned int& o_cmdStart) {
  unsigned int i=0;
  while(i<len && i<XM_CLIENT_MAX_PACKET_SIZE_DIGITS+1) {
    if(((char*)data)[i] == '\n') {
      o_cmdStart = i+1;
      ((char*)data)[i] = '\0';
      return atoi((char*)data);
    }
    i++;
  }
  return 0;
}

bool ClientListenerThread::isCommand(void* data, unsigned int len, const std::string& i_cmd) {
  if(i_cmd.length() + 1 > len) {
    return false;
  }

  if(((char*) data)[i_cmd.length()] != '\n') {
    return false;
  }

  return strncmp((char*)data, i_cmd.c_str(), i_cmd.length()) == 0;
}

std::string ClientListenerThread::getChatMessage(void* data, unsigned int len) {
  ((char*)data)[len-1] = '\0';
  return std::string((char*)data);
}
