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

#include "NetClient.h"
#include "thread/ClientListenerThread.h"
#include "../helpers/VExcept.h"
#include "../helpers/Log.h"
#include "helpers/Net.h"
#include <sstream>

NetClient::NetClient() {
    m_isConnected = false;
    m_clientListenerThread = NULL;
}

NetClient::~NetClient() {
}

void NetClient::connect(const std::string& i_server, int i_port) {
  IPaddress ip;

  if (SDLNet_ResolveHost(&ip, i_server.c_str(), i_port) < 0) {
    throw Exception(SDLNet_GetError());
  }

  if (!(m_sd = SDLNet_TCP_Open(&ip))) {
    throw Exception(SDLNet_GetError());
  }

  m_clientListenerThread = new ClientListenerThread(this);
  m_clientListenerThread->startThread();

  m_isConnected = true;

  LogInfo("client: connected on %s:%d", i_server.c_str(), i_port);
}

void NetClient::disconnect() {
  if(m_clientListenerThread->isThreadRunning()) {
    m_clientListenerThread->killThread();
  }
  delete m_clientListenerThread;

  LogInfo("client: disconnected")
  SDLNet_TCP_Close(m_sd);
  m_isConnected = false;
}

bool NetClient::isConnected() {
  return m_isConnected;
}

void NetClient::sendChatMessage(const std::string& i_msg) {
  unsigned int nread;
  std::string v_data;
  
  std::ostringstream v_nb;
  v_nb << i_msg.size();

  v_data = "message\n" + v_nb.str() + "\n" + i_msg + "\n";

  // don't send the \0
  if( (nread = SDLNet_TCP_Send(m_sd, v_data.c_str(), v_data.size())) != v_data.size()) {
    throw Exception("TCP_Send failed");
  }
}

TCPsocket* NetClient::socket() {
  return &m_sd;
}
