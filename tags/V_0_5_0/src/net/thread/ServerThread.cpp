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
#include <string>
#include "../helpers/Net.h"
#include "../../XMSession.h"
#include "../../states/StateManager.h"
#include "../ActionReader.h"
#include "../NetActions.h"
#include "../../GameText.h"

#define XM_SERVER_WAIT_TIMEOUT 1000
#define XM_SERVER_NB_SOCKETS_MAX 128
#define XM_SERVER_MAX_UDP_PACKET_SIZE 1024 // bytes

NetSClient::NetSClient(unsigned int i_id, TCPsocket i_tcpSocket, IPaddress *i_tcpRemoteIP) {
    m_id = i_id;
    m_tcpSocket   = i_tcpSocket;
    m_tcpRemoteIP = *i_tcpRemoteIP;
    m_isUdpBinded = false;
    tcpReader = new ActionReader();
}

NetSClient::~NetSClient() {
  delete tcpReader;
}

unsigned int NetSClient::id() const {
  return m_id;
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

bool NetSClient::isUdpBinded() const {
  return m_isUdpBinded;
}

void NetSClient::bindUdp(IPaddress i_udpIPAdress) {
  m_udpRemoteIP = i_udpIPAdress;

  LogInfo("server: host binded: %s:%i (UDP)",
	  XMNet::getIp(&m_udpRemoteIP).c_str(), SDLNet_Read16(&(m_udpRemoteIP.port)));
  m_isUdpBinded = true;
}

void NetSClient::unbindUdp() {
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
    m_nextClientId = 0;
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
	  manageClientUDP();
	} else {
	  i = 0;
	  while(i<m_clients.size()) {
	    if(SDLNet_SocketReady(*(m_clients[i]->tcpSocket()))) {
	      try {
	      	manageClientTCP(i);
		i++;
	      } catch(Exception &e) {
		LogInfo("server: bad TCP packet received by client %u (%s:%d)", i,
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
    m_clients[i]->unbindUdp();
  }

  // send new client to other clients
  NA_changeClients nacc;
  NetInfosClient nic;
  try {
    nic.NetId = m_clients[i]->id();
    nic.Name  = m_clients[i]->name();
    nacc.remove(&nic);
    sendToAllClients(&nacc, -1, 0, i);
  } catch(Exception &e) {
  }

  delete m_clients[i];
  m_clients.erase(m_clients.begin() + i);
}

void ServerThread::sendToClient(NetAction* i_netAction, unsigned int i, int i_src, int i_subsrc) {
  i_netAction->setSource(i_src, i_subsrc);
  if(m_clients[i]->isUdpBinded()) {
    i_netAction->send(m_clients[i]->tcpSocket(), &m_udpsd, m_udpPacket, m_clients[i]->udpRemoteIP());
  } else {
    i_netAction->send(m_clients[i]->tcpSocket(), NULL, NULL, NULL);
  }
}

void ServerThread::sendToAllClients(NetAction* i_netAction, int i_src, int i_subsrc, unsigned int i_except) {
  unsigned int i=0;
  
  while(i<m_clients.size()) {
    if(i != i_except) {
      try {
	sendToClient(i_netAction, i, i_src, i_subsrc);
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

  // to much clients ?
  if(m_clients.size() >= XMSession::instance()->serverMaxClients()) {
    NA_serverError na(UNTRANSLATED_GAMETEXT_TOO_MUCH_CLIENTS);
    na.setSource(-1, 0);
    try {
      na.send(&csd, NULL, NULL, NULL);
    } catch(Exception &e) {
    }

    SDLNet_TCP_Close(csd);
    return;
  }

  scn = SDLNet_TCP_AddSocket(m_set, csd);
  if(scn == -1) {
    LogError("server: SDLNet_TCP_AddSocket: %s", SDLNet_GetError());
    SDLNet_TCP_Close(csd);
    return;
  }

  m_clients.push_back(new NetSClient(m_nextClientId++, csd, tcpRemoteIP));
}

void ServerThread::manageClientTCP(unsigned int i) {
  NetAction* v_netAction;

  while(m_clients[i]->tcpReader->TCPReadAction(m_clients[i]->tcpSocket(), &v_netAction)) {
    manageAction(v_netAction, i);
    delete v_netAction;
  }
}

void ServerThread::manageClientUDP() {
  bool v_managedPacket;
  NetAction* v_netAction;

  if(SDLNet_UDP_Recv(m_udpsd, m_udpPacket) == 1) {
    v_managedPacket = false;
    for(unsigned int i=0; i<m_clients.size(); i++) {
      if(m_clients[i]->udpRemoteIP()->host == m_udpPacket->address.host && 
	 m_clients[i]->udpRemoteIP()->port == m_udpPacket->address.port)  {
	v_managedPacket = true;
	try {
	  v_netAction = ActionReader::UDPReadAction(m_udpPacket->data, m_udpPacket->len);
	  manageAction(v_netAction, i);
	  delete v_netAction;
	} catch(Exception &e) {
	  // ok, a bad packet received, forget it
	  LogWarning("server: bad UDP packet received by client %u (%s:%i)", i,
		     XMNet::getIp(&(m_udpPacket->address)).c_str(), SDLNet_Read16(&(m_udpPacket->address.port)));
	}
	break; // stop : only one client
      }
    }

    // anonym packet ? find the associated client
    if(v_managedPacket == false) {
      try {
	v_netAction = ActionReader::UDPReadAction(m_udpPacket->data, m_udpPacket->len);
	if(v_netAction->actionType() == TNA_udpBind) {
	  for(unsigned int i=0; i<m_clients.size(); i++) {
	    if(m_clients[i]->isUdpBinded() == false) {
	      if(m_clients[i]->udpBindKey() == ((NA_udpBind*)v_netAction)->key()) {
		//LogInfo("UDP bind key received via UDP: %s", ((NA_udpBind*)v_netAction)->key().c_str());
		m_clients[i]->bindUdp(m_udpPacket->address);
		break; // stop : only one client
	      }
	    }
	  }
	} else {
	  LogWarning("Packet of unknown client received");
	}
      } catch(Exception &e) {
	/* forget this bad packet */
	LogWarning("server: bad anonym UDP packet received by %s:%i",
		   XMNet::getIp(&(m_udpPacket->address)).c_str(), SDLNet_Read16(&(m_udpPacket->address.port)));
      }
    }
  }
}

void ServerThread::manageAction(NetAction* i_netAction, unsigned int i_client) {

  switch(i_netAction->actionType()) {

  case TNA_udpBind:
    /* managed before */
    break;

  case TNA_udpBindQuery:
  case TNA_serverError:
  case TNA_changeClients:
    {
      /* should not be received */
      throw Exception("");
    }
    break;

  case TNA_clientInfos:
    {
      // check protocol version
      if(((NA_clientInfos*)i_netAction)->protocolVersion() != XM_NET_PROTOCOL_VERSION) {
	NA_serverError na(UNTRANSLATED_GAMETEXT_SERVER_PROTOCOL_VERSION_INCOMPATIBLE);
	na.setSource(-1, 0);
	try {
	  sendToClient(&na, i_client, -1, 0);
	} catch(Exception &e) {
	}
	throw Exception("Protocol version incompatible");
      }

      // udpBindKey received
      //LogInfo("Protocol version of client %i is %i", i_client, ((NA_clientInfos*)i_netAction)->protocolVersion());
      //LogInfo("UDP bind key of client %i is %s", i_client, ((NA_clientInfos*)i_netAction)->udpBindKey().c_str());
      m_clients[i_client]->setUdpBindKey(((NA_clientInfos*)i_netAction)->udpBindKey());
      
      // query bind udp
      NA_udpBindQuery naq;
      try {
      	sendToClient(&naq, i_client, -1, 0);
      } catch(Exception &e) {
      }

      // send the current clients list to the client only when the client has a name
      NA_changeClients nacc;
      NetInfosClient nic;
      try {
	for(unsigned int i=0; i<m_clients.size(); i++) {
	  if(m_clients[i]->name() != "") {
	    nic.NetId = m_clients[i]->id();
	    nic.Name  = m_clients[i]->name();
	    nacc.add(&nic);
	  }
	}
      	sendToClient(&nacc, i_client, -1, 0);
      } catch(Exception &e) {
      }
    }
    break;
      
  case TNA_chatMessage:
    {
      sendToAllClients(i_netAction, m_clients[i_client]->id(), i_netAction->getSubSource(), i_client);
    }
    break;
    
  case TNA_frame:
    {
      unsigned int i = 0;
      while(i<m_clients.size()) {
	if(i != i_client &&
	   m_clients[i_client]->playingLevelId() != "" &&
	   m_clients[i_client]->playingLevelId() == m_clients[i]->playingLevelId()
	   ) {
	  try {
	    sendToClient(i_netAction, i, m_clients[i_client]->id(), i_netAction->getSubSource());
	    i++;
	  } catch(Exception &e) {
	    removeClient(i);
	  }
	} else {
	  i++;
	}
      }
    }
    break;

  case TNA_changeName:
    {
      m_clients[i_client]->setName(((NA_changeName*)i_netAction)->getName());
      if(m_clients[i_client]->name() == "") {
	throw Exception("Invalid name provided");
      }
      LogInfo("Client[%i]'s name is \"%s\"", i_client, m_clients[i_client]->name().c_str());

      // send new client to other clients
      NA_changeClients nacc;
      NetInfosClient nic;
      try {
	nic.NetId = m_clients[i_client]->id();
	nic.Name  = m_clients[i_client]->name();
	nacc.add(&nic);
      	sendToAllClients(&nacc, -1, 0, i_client);
      } catch(Exception &e) {
      }

    }
    break;

  case TNA_playingLevel:
    {
      m_clients[i_client]->setPlayingLevelId(((NA_playingLevel*)i_netAction)->getLevelId());
      sendToAllClients(i_netAction, m_clients[i_client]->id(), i_netAction->getSubSource(), i_client);
    }
    break;

  }
}
