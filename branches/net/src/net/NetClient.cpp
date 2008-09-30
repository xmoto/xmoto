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
#include "NetActions.h"
#include "thread/ClientListenerThread.h"
#include "../helpers/VExcept.h"
#include "../helpers/Log.h"
#include "helpers/Net.h"
#include <sstream>

NetClient::NetClient() {
    m_isConnected = false;
    m_clientListenerThread = NULL;
    m_netActionsMutex = SDL_CreateMutex();
}

NetClient::~NetClient() {
  SDL_DestroyMutex(m_netActionsMutex);

  for(unsigned int i=0; i<m_netActions.size(); i++) {
    delete m_netActions[i];
  }
}

void NetClient::executeNetActions() {
  if(m_netActions.size() == 0) {
    return; // try to not lock via mutex if not needed
  }

  SDL_LockMutex(m_netActionsMutex);
  for(unsigned int i=0; i<m_netActions.size(); i++) {
    LogInfo("Execute NetAction");
    m_netActions[i]->execute();
    delete m_netActions[i];
  }
  m_netActions.clear();
  SDL_UnlockMutex(m_netActionsMutex);
}

void NetClient::addNetAction(NetAction* i_act) {
  SDL_LockMutex(m_netActionsMutex);
  m_netActions.push_back(i_act);
  SDL_UnlockMutex(m_netActionsMutex);
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
    m_clientListenerThread->askThreadToEnd();
    m_clientListenerThread->waitForThreadEnd();
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

  int v_subPacketSize = std::string("message").length() + 1 + i_msg.length() + 1;

  v_nb << v_subPacketSize;
  v_data = v_nb.str() + "\n" + "message\n" + i_msg + "\n";

  // don't send the \0
  if( (nread = SDLNet_TCP_Send(m_sd, v_data.c_str(), v_data.size())) != v_data.size()) {
    throw Exception("TCP_Send failed");
  }
}

TCPsocket* NetClient::socket() {
  return &m_sd;
}
