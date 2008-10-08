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

#include "ServerThread.h"
#include "../../helpers/Log.h"
#include "../../helpers/VExcept.h"
#include <SDL_net.h>
#include <string>
#include "../helpers/Net.h"
#include "../../XMSession.h"
#include "../../states/StateManager.h"
#include "../ActionReader.h"
#include "../NetActions.h"

#define XM_SERVER_WAIT_TIMEOUT 1000
#define XM_SERVER_NB_SOCKETS_MAX 128
#define XM_SERVER_MAX_UDP_PACKET_SIZE 1024 // bytes

NetSClient::NetSClient(TCPsocket i_tcpSocket, IPaddress *i_tcpRemoteIP) {
    m_tcpSocket   = i_tcpSocket;
    m_udpChannel  = -1;
    m_tcpRemoteIP = *i_tcpRemoteIP;
    m_isUdpBinded = false;
    tcpReader = new ActionReader();
}

NetSClient::~NetSClient() {
  delete tcpReader;
}

TCPsocket* NetSClient::tcpSocket() {
  return &m_tcpSocket;
}

IPaddress* NetSClient::tcpRemoteIP() {
  return &m_tcpRemoteIP;
}

IPaddress* NetSClient::udpRemoteIP() {
  return &m_udpRemoteIP;
}

int NetSClient::udpChannel() const {
  return m_udpChannel;
}

void NetSClient::setChannel(int i_value) {
  m_udpChannel = i_value;
}

bool NetSClient::isUdpBinded() const {
  return m_isUdpBinded;
}

void NetSClient::bindUdp(UDPsocket* i_udpsd, IPaddress i_udpIPAdress, int i_nextUdpBoundId) {
  if(i_nextUdpBoundId > SDLNET_MAX_UDPCHANNELS) {
    throw Exception("Too much udp binding");
  }

  m_udpRemoteIP = i_udpIPAdress;

  if( (m_udpChannel = SDLNet_UDP_Bind(*i_udpsd, i_nextUdpBoundId, &m_udpRemoteIP)) == -1) {
    LogError("server: SDLNet_UDP_Bind: %s", SDLNet_GetError());
    return;
  }

  LogInfo("server: host binded: %s:%i (UDP, channel %i)",
	  XMNet::getIp(&m_udpRemoteIP).c_str(), SDLNet_Read16(&(m_udpRemoteIP.port)), m_udpChannel);
  m_isUdpBinded = true;
}

void NetSClient::unbindUdp(UDPsocket* i_udpsd) {
  SDLNet_UDP_Unbind(*i_udpsd, m_udpChannel);
  m_isUdpBinded = false;
}

void NetSClient::setUdpBindKey(const std::string& i_key) {
  m_udpBindKey = i_key;
}

std::string NetSClient::udpBindKey() const {
  return m_udpBindKey;
}

void NetSClient::setName(const std::string& i_name) {
  m_name = i_name;
}

std::string NetSClient::name() const {
  return m_name;
}

void NetSClient::setPlayingLevelId(const std::string& i_levelId) {
  m_playingLevelId = i_levelId;
}

std::string NetSClient::playingLevelId() const {
  return m_playingLevelId;
}

ServerThread::ServerThread() {
    m_set = NULL;
    m_nextUdpBoundId = 0;
    m_udpPacket = SDLNet_AllocPacket(XM_SERVER_MAX_UDP_PACKET_SIZE);

    if(!m_udpPacket) {
      throw Exception("SDLNet_AllocPacket: " + std::string(SDLNet_GetError()));
    }
}

ServerThread::~ServerThread() {
  SDLNet_FreePacket(m_udpPacket);
}

int ServerThread::realThreadFunction() {
  IPaddress ip;
  int n_activ;
  unsigned int i;
  int ssn;

  LogInfo("server: starting");

  /* Resolving the host using NULL make network interface to listen */
  if(SDLNet_ResolveHost(&ip, NULL, XMSession::instance()->serverPort()) < 0) {
    LogError("server: SDLNet_ResolveHost: %s", SDLNet_GetError());
    return 1;
  }

  m_set = SDLNet_AllocSocketSet(XM_SERVER_NB_SOCKETS_MAX);
  if(!m_set) {
    LogError("server: SDLNet_AllocSocketSet: %s", SDLNet_GetError());
    return 1;
  }
 
  /* Open a connection with the IP provided (listen on the host's port) */
  LogInfo("server: open connexion");
  if((m_tcpsd = SDLNet_TCP_Open(&ip)) == 0) {
    LogError("server: SDLNet_TCP_Open: %s", SDLNet_GetError());
    SDLNet_FreeSocketSet(m_set);
    return 1;
  }
  if((m_udpsd = SDLNet_UDP_Open(XMSession::instance()->serverPort())) == 0) {
    LogError("server: SDLNet_UDP_Open: %s", SDLNet_GetError());
    SDLNet_FreeSocketSet(m_set);
    SDLNet_TCP_Close(m_tcpsd);
    return 1;
  }

  ssn = SDLNet_TCP_AddSocket(m_set, m_tcpsd);
  if(ssn == -1) {
    LogError("server: SDLNet_TCP_AddSocket: %s", SDLNet_GetError());
    SDLNet_FreeSocketSet(m_set);
    SDLNet_TCP_Close(m_tcpsd);
    SDLNet_UDP_Close(m_udpsd);
    return 1;
  }
  ssn = SDLNet_UDP_AddSocket(m_set, m_udpsd);
  if(ssn == -1) {
    LogError("server: SDLNet_UDP_AddSocket: %s", SDLNet_GetError());
    SDLNet_FreeSocketSet(m_set);
    SDLNet_TCP_Close(m_tcpsd);
    SDLNet_UDP_Close(m_udpsd);
    return 1;
  }

  StateManager::instance()->sendAsynchronousMessage("SERVER_STATUS_CHANGED");

  /* Wait for a connection */
  while(m_askThreadToEnd == false) {
    n_activ = SDLNet_CheckSockets(m_set, XM_SERVER_WAIT_TIMEOUT);
    if(n_activ == -1) {
      LogError("SDLNet_CheckSockets: %s", SDLNet_GetError());
      m_askThreadToEnd = true;
    } else {

      if(n_activ != 0) {
	// server socket
	if(SDLNet_SocketReady(m_tcpsd)) {
	  acceptClient();
	}

	else if(SDLNet_SocketReady(m_udpsd)) {

	  // udp socket
	  if(SDLNet_UDP_Recv(m_udpsd, m_udpPacket) == 1) {
	    if(m_udpPacket->channel >= 0) {
	      for(unsigned int i=0; i<m_clients.size(); i++) {
		if(m_clients[i]->udpChannel() == m_udpPacket->channel) {

		  NetAction* v_netAction;
		  try {
		    v_netAction = ActionReader::UDPReadAction(m_udpPacket->data, m_udpPacket->len);
		    manageAction(v_netAction, i);
		    delete v_netAction;
		  } catch(Exception &e) {
		    LogError("%s", e.getMsg().c_str());
		    removeClient(i);
		  }

		  break; // stop : a channel is assigned to only one client
		}
	      }
	    } else {
	      NetAction* v_netAction;
	      try {
		v_netAction = ActionReader::UDPReadAction(m_udpPacket->data, m_udpPacket->len);
		if(v_netAction->actionKey() == NA_udpBind::ActionKey) {
		  for(unsigned int i=0; i<m_clients.size(); i++) {
		    if(m_clients[i]->isUdpBinded() == false) {
		      if(m_clients[i]->udpBindKey() == ((NA_udpBind*)v_netAction)->key()) {
			LogInfo("UDP bind key received via UDP: %s", ((NA_udpBind*)v_netAction)->key().c_str());
			m_clients[i]->bindUdp(&m_udpsd, m_udpPacket->address, m_nextUdpBoundId++);
		      }
		    }
		  }
		} else {
		  LogWarning("Packet received without a channel");
		}
	      } catch(Exception &e) {
	      }
	    }
	  }

	} else {
	  i = 0;
	  while(i<m_clients.size()) {
	    if(SDLNet_SocketReady(*(m_clients[i]->tcpSocket()))) {
	      try {
	      	manageClientTCP(i);
		i++;
	      } catch(Exception &e) {
	      	LogInfo("server: host disconnected: %s:%d (TCP)",
	      		XMNet::getIp(m_clients[i]->tcpRemoteIP()).c_str(),
	      		SDLNet_Read16(&(m_clients[i]->tcpRemoteIP())->port));
	      	removeClient(i);
	      }
	    } else {
	      i++;
	    }
	  }
	}
      }

    }
  }

  LogInfo("server: %i client(s) still connnected", m_clients.size());

  // disconnect all clients
  i=0;
  while(i<m_clients.size()) {
    removeClient(i);
    i++;
  }

  SDLNet_TCP_DelSocket(m_set, m_tcpsd);
  SDLNet_UDP_DelSocket(m_set, m_udpsd);

  LogInfo("server: close connexion");
  SDLNet_TCP_Close(m_tcpsd);
  SDLNet_UDP_Close(m_udpsd);

  SDLNet_FreeSocketSet(m_set);
  m_set = NULL;

  StateManager::instance()->sendAsynchronousMessage("SERVER_STATUS_CHANGED");
  LogInfo("server: ending normally");
  return 0;
}

void ServerThread::removeClient(unsigned int i) {
  SDLNet_TCP_DelSocket(m_set, *(m_clients[i]->tcpSocket()));
  SDLNet_TCP_Close(*(m_clients[i]->tcpSocket()));
  if(m_clients[i]->isUdpBinded()) {
    m_clients[i]->unbindUdp(&m_udpsd);
  }
  delete m_clients[i];
  m_clients.erase(m_clients.begin() + i);
}

void ServerThread::sendToClient(NetAction* i_netAction, unsigned int i) {
  if(m_clients[i]->isUdpBinded()) {
    i_netAction->send(m_clients[i]->tcpSocket(), &m_udpsd, m_udpPacket, m_clients[i]->udpRemoteIP());
  } else {
    i_netAction->send(m_clients[i]->tcpSocket(), NULL, NULL, NULL);
  }
}

void ServerThread::sendToAllClients(NetAction* i_netAction, unsigned int i_except) {
  unsigned int i=0;
  
  while(i<m_clients.size()) {
    if(i != i_except) {
      try {
	sendToClient(i_netAction, i);
	i++;
      } catch(Exception &e) {
	removeClient(i);
      }
    } else {
      i++;
    }
  }
}

void ServerThread::acceptClient() {
  TCPsocket csd;
  IPaddress *tcpRemoteIP;
  int scn;

  if((csd = SDLNet_TCP_Accept(m_tcpsd)) == 0) {
    return;
  }

  /* Get the remote address */
  if((tcpRemoteIP = SDLNet_TCP_GetPeerAddress(csd)) == NULL) {
    LogWarning("server: SDLNet_TCP_GetPeerAddress: %s", SDLNet_GetError());
    SDLNet_TCP_Close(csd);
    return;
  }

  /* Print the address, converting in the host format */
  LogInfo("server: host connected: %s:%d (TCP)",
	  XMNet::getIp(tcpRemoteIP).c_str(), SDLNet_Read16(&tcpRemoteIP->port));

  scn = SDLNet_TCP_AddSocket(m_set, csd);
  if(scn == -1) {
    LogError("server: SDLNet_TCP_AddSocket: %s", SDLNet_GetError());
    SDLNet_TCP_Close(csd);
    return;
  }

  m_clients.push_back(new NetSClient(csd, tcpRemoteIP));
}

void ServerThread::manageClientTCP(unsigned int i) {
  NetAction* v_netAction;

  while(m_clients[i]->tcpReader->TCPReadAction(m_clients[i]->tcpSocket(), &v_netAction)) {
    // manage v_netAction
    
    if(v_netAction->actionKey() == NA_udpBindKey::ActionKey) {

      // udpBindKey received
      LogInfo("UDP bind key of client %i is %s", i, ((NA_udpBindKey*)v_netAction)->key().c_str());
      m_clients[i]->setUdpBindKey(((NA_udpBindKey*)v_netAction)->key());

      // query bind udp
      NA_udpBindQuery na;
      try {
	sendToClient(&na, i);
      } catch(Exception &e) {
      }
    } else if(v_netAction->actionKey() == NA_udpBind::ActionKey ||
	      v_netAction->actionKey() == NA_udpBindQuery::ActionKey) {
      // don't manage these actions
    } else {
      manageAction(v_netAction, i);
    }
    delete v_netAction;
  }
}

void ServerThread::manageAction(NetAction* i_netAction, unsigned int i_client) {
  unsigned int i;

  if(i_netAction->actionKey() == NA_presentation::ActionKey) {
    m_clients[i_client]->setName(((NA_presentation*)i_netAction)->getName());
    LogInfo("Client[%i]'s name is \"%s\"", i_client, m_clients[i_client]->name().c_str());
  }

  else if(i_netAction->actionKey() == NA_startingLevel::ActionKey) {
    m_clients[i_client]->setPlayingLevelId(((NA_startingLevel*)i_netAction)->getLevelId());
    LogInfo("Client[%i] is playing \"%s\"", i_client, m_clients[i_client]->playingLevelId().c_str());
  }

  else if(i_netAction->actionKey() == NA_frame::ActionKey) {

    i = 0;
    while(i<m_clients.size()) {
      if(i != i_client &&
	 m_clients[i_client]->playingLevelId() != "" &&
	 m_clients[i_client]->playingLevelId() == m_clients[i]->playingLevelId()
	 ) {
	try {
	  sendToClient(i_netAction, i);
	  i++;
	} catch(Exception &e) {
	  removeClient(i);
	}
      } else {
	i++;
      }
    }
  }

  else {
    // chat
    sendToAllClients(i_netAction, i_client);
  }
}
