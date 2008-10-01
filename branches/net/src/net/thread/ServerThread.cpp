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

#define XM_SERVER_WAIT_TIMEOUT 1000
#define XM_SERVER_NB_SOCKETS_MAX 128
#define XM_SERVER_CLIENT_BUFFER_SIZE 1024

NetSClient::NetSClient(TCPsocket i_socket, IPaddress *i_remoteIP) {
    m_socket      = i_socket;
    m_remoteIP    = i_remoteIP;
}

NetSClient::~NetSClient() {
}

TCPsocket* NetSClient::socket() {
  return &m_socket;
}

IPaddress* NetSClient::remoteIP() {
  return m_remoteIP;
}

ServerThread::ServerThread() {
    m_set = NULL;
}

ServerThread::~ServerThread() {
}

int ServerThread::realThreadFunction() {
  IPaddress ip;
  TCPsocket sd;
  int n_activ;
  unsigned int i;
  char buffer[XM_SERVER_CLIENT_BUFFER_SIZE];
  int nread;
  int ssn;

  LogInfo("server: starting");

  /* Resolving the host using NULL make network interface to listen */
  if(SDLNet_ResolveHost(&ip, NULL, XMSession::instance()->serverPort()) < 0) {
    LogError("server: SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return 1;
  }

  m_set = SDLNet_AllocSocketSet(XM_SERVER_NB_SOCKETS_MAX);
  if(!m_set) {
    LogError("server: SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
    return 1;
  }
 
  /* Open a connection with the IP provided (listen on the host's port) */
  LogInfo("server: open connexion");
  if((sd = SDLNet_TCP_Open(&ip)) == 0) {
    LogError("server: SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    SDLNet_FreeSocketSet(m_set);
    return 1;
  }

  ssn = SDLNet_TCP_AddSocket(m_set, sd);
  if(ssn == -1) {
    LogError("server: SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError());
    SDLNet_FreeSocketSet(m_set);
    SDLNet_TCP_Close(sd);
    return 1;
  }

  /* Wait for a connection */
  while(m_askThreadToEnd == false) {
    n_activ = SDLNet_CheckSockets(m_set, XM_SERVER_WAIT_TIMEOUT);
    if(n_activ == -1) {
      LogError("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
      m_askThreadToEnd = true;
    } else {
      if(n_activ != 0) {
	// server socket
	if(SDLNet_SocketReady(sd)) {
	  acceptClient(&sd);
	} else {

	  i = 0;
	  while(i<m_clients.size()) {
	    if(SDLNet_SocketReady(*(m_clients[i]->socket()))) {
	      if( (nread = SDLNet_TCP_Recv(*(m_clients[i]->socket()),
	      				   buffer, XM_SERVER_CLIENT_BUFFER_SIZE)) > 0) {
	      	manageClient(i, buffer, nread);
	      	i++;
	      } else {
	      	LogInfo("server: host disconnected: %s:%d",
	      		XMNet::getIp(m_clients[i]->remoteIP()).c_str(),
	      		SDLNet_Read16(&(m_clients[i]->remoteIP())->port));
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

  SDLNet_TCP_DelSocket(m_set, sd);

  LogInfo("server: close connexion");
  SDLNet_TCP_Close(sd);

  SDLNet_FreeSocketSet(m_set);
  m_set = NULL;

  LogInfo("server: ending normally");
  return 0;
}

void ServerThread::removeClient(unsigned int i) {
  SDLNet_TCP_DelSocket(m_set, *(m_clients[i]->socket()));
  SDLNet_TCP_Close(*(m_clients[i]->socket()));
  delete m_clients[i];
  m_clients.erase(m_clients.begin() + i);
}

void ServerThread::sendToClient(void* data, int len, unsigned int i) {
  int nread;
  
  if( (nread = SDLNet_TCP_Send(*(m_clients[i]->socket()), data, len)) != len) {
    throw Exception("TCP_Send failed");
  }
}

void ServerThread::sendToAllClients(void* data, int len, unsigned int i_except) {
  unsigned int i=0;
  
  while(i<m_clients.size()) {
    if(i != i_except) {
      try {
	sendToClient(data, len, i);
	i++;
      } catch(Exception &e) {
	removeClient(i);
      }
    } else {
      i++;
    }
  }
}

void ServerThread::acceptClient(TCPsocket* sd) {
  TCPsocket csd;
  IPaddress *remoteIP;
  int scn;

  if((csd = SDLNet_TCP_Accept(*sd)) == 0) {
    return;
  }

  /* Get the remote address */
  if((remoteIP = SDLNet_TCP_GetPeerAddress(csd)) == NULL) {
    LogWarning("server: SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());
    SDLNet_TCP_Close(csd);
    return;
  }

  /* Print the address, converting in the host format */
  LogInfo("server: host connected: %s:%d",
	  XMNet::getIp(remoteIP).c_str(), SDLNet_Read16(&remoteIP->port));

  scn = SDLNet_TCP_AddSocket(m_set, csd);
  if(scn == -1) {
    LogError("server: SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError());
    SDLNet_TCP_Close(csd);
    return;
  }

  m_clients.push_back(new NetSClient(csd, remoteIP));

  // welcome
  //std::string v_msg = "Xmoto server\n";
  //try {
  //  sendToClient((void*) v_msg.c_str(), v_msg.length(), m_clients.size()-1);
  //} catch(Exception &e) {
  //  removeClient(m_clients.size()-1);
  //}
}

void ServerThread::manageClient(unsigned int i, void* data, int len) {
  sendToAllClients(data, len, i);
}
