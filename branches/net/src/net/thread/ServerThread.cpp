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
#include "ServerClientListenerThread.h"

#define XM_SERVER_PORT 4130
#define XM_SERVER_CHECKCLIENT_WAIT 2000

ServerThread::ServerThread() {
}

ServerThread::~ServerThread() {
}

// only this thread write messages to clients
// only clientListener read messages

int ServerThread::realThreadFunction() {
  IPaddress ip;
  TCPsocket sd, csd;
  unsigned int i;
  ServerClientListenerThread* v_serverClientListenerThread;

  LogInfo("server: starting");

  /* Resolving the host using NULL make network interface to listen */
  if(SDLNet_ResolveHost(&ip, NULL, XM_SERVER_PORT) < 0) {
    LogError("server: SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return 1;
  }
 
  /* Open a connection with the IP provided (listen on the host's port) */
  LogInfo("server: open connexion");
  if((sd = SDLNet_TCP_Open(&ip)) == 0) {
    LogError("server: SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    return 1;
  }

  /* Wait for a connection, send data and term */
  while(m_askThreadToEnd == false) {

    /* This check the sd if there is a pending connection.
     * If there is one, accept that, and open a new socket for communicating */

    if((csd = SDLNet_TCP_Accept(sd)) == 0) {
      SDL_Delay(XM_SERVER_CHECKCLIENT_WAIT);
      cleanDisconnectedClients();
    } else {
      v_serverClientListenerThread = new ServerClientListenerThread(csd);
      m_serverClientListenerThread.push_back(v_serverClientListenerThread);
      v_serverClientListenerThread->startThread();

      std::string v_msg = "Xmoto server\n";
      sendToClient((void*) v_msg.c_str(), v_msg.length(), m_serverClientListenerThread.size()-1);
    }
  }

  LogInfo("server: %i client(s) still connnected", m_serverClientListenerThread.size());

  // disconnect all clients
  i=0;
  while(i<m_serverClientListenerThread.size()) {
    removeClient(i);
    i++;
  }

  LogInfo("server: close connexion");
  SDLNet_TCP_Close(sd);

  LogInfo("server: ending normally");
  return 0;
}

void ServerThread::removeClient(unsigned int i) {
  if(m_serverClientListenerThread[i]->isThreadRunning()) {
    m_serverClientListenerThread[i]->killThread();
  }

  SDLNet_TCP_Close(*(m_serverClientListenerThread[i]->socket()));
  delete m_serverClientListenerThread[i];
  m_serverClientListenerThread.erase(m_serverClientListenerThread.begin() + i);
}

void ServerThread::cleanDisconnectedClients() {
  unsigned int i=0;
  while(i<m_serverClientListenerThread.size()) {
    if(m_serverClientListenerThread[i]->isThreadRunning() == false) {
      removeClient(i);
    } else {
      i++;
    }
  }
}

void ServerThread::sendToClient(void* data, int len, unsigned int i) {
  int nread;

  if( (nread = SDLNet_TCP_Send(*(m_serverClientListenerThread[i]->socket()), data, len)) != len) {
    throw Exception("TCP_Send failed");
  }
}

void ServerThread::sendToAllClients(void* data, int len, unsigned int i_except) {
  unsigned int i=0;
  
  while(i<m_serverClientListenerThread.size()) {
    try {
      sendToClient(data, len, i);
      i++;
    } catch(Exception &e) {
      removeClient(i);
    }
  }
}
