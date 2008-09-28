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
#define XM_CLIENT_MAX_COMMAND_SIZE 256 // limit the size of a command
			  // WARNING : command must contain only ascii chars

ClientListenerThread::ClientListenerThread(NetClient* i_netClient) {
    m_netClient = i_netClient;
}

ClientListenerThread::~ClientListenerThread() {
}

int ClientListenerThread::realThreadFunction() {
  int nread;
  char buffer[XM_CLIENT_BUFFER_SIZE];
  std::string v_cmd;

  while( (nread = SDLNet_TCP_Recv(*(m_netClient->socket()),
			       buffer, XM_CLIENT_BUFFER_SIZE)) > 0) {
    // todo : concat buffers if buffer doesn't make a full command
    try {
      v_cmd = getCommand(buffer, nread);
    } catch(Exception &e) {
      // todo
    }

    if(v_cmd == "message") {
      LogInfo("client[message]:%s\n", getChatMessage(buffer, nread));
    }
  }
 
  return 1;
}

std::string ClientListenerThread::getCommand(void* data, int len) {
  int i = 0;
  std::string v_cmd;

  while(i<len && i<XM_CLIENT_MAX_COMMAND_SIZE) {
    if(((char*)data)[i] == '\n') { // assume ASCII chars command
      ((char*)data)[i] = '\0';
      v_cmd = std::string((char*)data);
      ((char*)data)[i] = '\n';
      return v_cmd;
    }
    i++;
  }
  throw Exception("Invalid net command");
}

std::string ClientListenerThread::getChatMessage(void* data, int len) {
  // get number of lines
  // get message
  return "";
}
