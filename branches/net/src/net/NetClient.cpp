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
#include "../XMSession.h"
#include "../states/StateManager.h"
#include <sstream>
#include "../helpers/VMath.h"

NetClient::NetClient() {
    m_isConnected = false;
    m_clientListenerThread = NULL;
    m_netActionsMutex = SDL_CreateMutex();
    m_universe = NULL;
    m_udpSendPacket = SDLNet_AllocPacket(XM_CLIENT_MAX_UDP_PACKET_SIZE);

    if(!m_udpSendPacket) {
      throw Exception("SDLNet_AllocPacket: " + std::string(SDLNet_GetError()));
    }

    std::ostringstream v_rd;
    v_rd << randomIntNum(1, RAND_MAX);
    m_udpBindKey = v_rd.str();
}

NetClient::~NetClient() {
  SDL_DestroyMutex(m_netActionsMutex);

  for(unsigned int i=0; i<m_netActions.size(); i++) {
    delete m_netActions[i];
  }

  SDLNet_FreePacket(m_udpSendPacket);
}

std::string NetClient::udpBindKey() const {
  return m_udpBindKey;
}

UDPpacket* NetClient::sendPacket() {
  return m_udpSendPacket;
}

void NetClient::executeNetActions() {
  if(m_netActions.size() == 0) {
    return; // try to not lock via mutex if not needed
  }

  SDL_LockMutex(m_netActionsMutex);
  for(unsigned int i=0; i<m_netActions.size(); i++) {
    //LogInfo("Execute NetAction");
    m_netActions[i]->execute(this);
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

  if(m_isConnected) {
    throw Exception("Already connected");
  }

  if (SDLNet_ResolveHost(&serverIp, i_server.c_str(), i_port) < 0) {
    throw Exception(SDLNet_GetError());
  }

  if (!(m_tcpsd = SDLNet_TCP_Open(&serverIp))) {
    throw Exception(SDLNet_GetError());
  }

  if((m_udpsd = SDLNet_UDP_Open(0)) == 0) {
    LogError("server: SDLNet_UDP_Open: %s", SDLNet_GetError());
    SDLNet_TCP_Close(m_tcpsd);
    throw Exception(SDLNet_GetError());
  }
  m_udpSendPacket->address.host = serverIp.host;
  m_udpSendPacket->address.port = serverIp.port;

  m_clientListenerThread = new ClientListenerThread(this);
  m_clientListenerThread->startThread();

  m_isConnected = true;

  LogInfo("client: connected on %s:%d", i_server.c_str(), i_port);

  // bind udp port on server
  NA_udpBindKey na(m_udpBindKey);
  try {
    NetClient::instance()->send(&na);
  } catch(Exception &e) {
  }
}

void NetClient::disconnect() {
  if(m_clientListenerThread->isThreadRunning()) {
    m_clientListenerThread->askThreadToEnd();
    m_clientListenerThread->waitForThreadEnd();
  }
  delete m_clientListenerThread;

  LogInfo("client: disconnected")
  SDLNet_TCP_Close(m_tcpsd);
  SDLNet_UDP_Close(m_udpsd);
  m_isConnected = false;
}

bool NetClient::isConnected() {
  // check that the client has not finished
  if(m_isConnected) {
    if(m_clientListenerThread->isThreadRunning() == false) {
      disconnect();
    }
  }
  return m_isConnected;
}

TCPsocket* NetClient::tcpSocket() {
  return &m_tcpsd;
}

UDPsocket* NetClient::udpSocket() {
  return &m_udpsd;
}

void NetClient::send(NetAction* i_netAction) {
  try {
    i_netAction->send(&m_tcpsd, &m_udpsd, m_udpSendPacket, &m_udpSendPacket->address);
  } catch(Exception &e) {
    disconnect();
    StateManager::instance()->sendAsynchronousMessage("CLIENT_DISCONNECTED_BY_ERROR");
    throw e;
  }
}

void NetClient::resetPlay(Universe* i_universe) {
  m_netGhosts.clear();
  m_universe = i_universe;
}

std::vector<NetGhost*>& NetClient::NetGhosts() {
  return m_netGhosts;
}

Universe* NetClient::getUniverse() {
  return m_universe;
}

void NetClient::addNetGhost(NetGhost* i_ghost) {
  m_netGhosts.push_back(i_ghost);
}
