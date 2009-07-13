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
#include "../NetActions.h"
#include "../ActionReader.h"
#include "../../helpers/Log.h"
#include "../../helpers/VExcept.h"
#include "../../states/StateManager.h"

#define XM_CLIENT_WAIT_TIMEOUT 1000

ClientListenerThread::ClientListenerThread(NetClient* i_netClient) {
    m_netClient = i_netClient;
}

ClientListenerThread::~ClientListenerThread() {
}

int ClientListenerThread::realThreadFunction() {
  SDLNet_SocketSet v_set;
  int scn;
  int n_activ;
  bool v_onError = false;
  UDPpacket* v_udpReceiptPacket;
  ActionReader v_tcpReader;
  NetAction* v_netAction;

  // use a set to get the timeout
  v_set = SDLNet_AllocSocketSet(2); // tcp + udp
  if(!v_set) {
    LogError("client: SDLNet_AllocSocketSet: %s", SDLNet_GetError());
    StateManager::instance()->sendAsynchronousMessage("CLIENT_DISCONNECTED_BY_ERROR");
    return 1;
  }

  scn = SDLNet_TCP_AddSocket(v_set, *(m_netClient->tcpSocket()));
  if(scn == -1) {
    SDLNet_FreeSocketSet(v_set);
    LogError("client: SDLNet_TCP_AddSocket: %s", SDLNet_GetError());
    StateManager::instance()->sendAsynchronousMessage("CLIENT_DISCONNECTED_BY_ERROR");
    return 1;
  }

  scn = SDLNet_UDP_AddSocket(v_set, *(m_netClient->udpSocket()));
  if(scn == -1) {
    SDLNet_TCP_DelSocket(v_set, *(m_netClient->tcpSocket()));
    LogError("client: SDLNet_UDP_AddSocket: %s", SDLNet_GetError());
    StateManager::instance()->sendAsynchronousMessage("CLIENT_DISCONNECTED_BY_ERROR");
    return 1;
  }

  v_udpReceiptPacket = SDLNet_AllocPacket(XM_CLIENT_MAX_UDP_PACKET_SIZE);
  if(!v_udpReceiptPacket) {
    SDLNet_TCP_DelSocket(v_set, *(m_netClient->tcpSocket()));
    SDLNet_UDP_DelSocket(v_set, *(m_netClient->udpSocket()));
    LogError("client: SDLNet_AllocPacket: %s", SDLNet_GetError());
    StateManager::instance()->sendAsynchronousMessage("CLIENT_DISCONNECTED_BY_ERROR");
    return 1;
  }

  StateManager::instance()->sendAsynchronousMessage("CLIENT_STATUS_CHANGED");

  /* Wait for a connection */
  while(m_askThreadToEnd == false) {
    n_activ = SDLNet_CheckSockets(v_set, XM_CLIENT_WAIT_TIMEOUT);
    if(n_activ == -1) {
      LogError("SDLNet_CheckSockets: %s", SDLNet_GetError());
      m_askThreadToEnd = true;
      v_onError = true;
    } else {
      if(n_activ != 0) {

	if(SDLNet_SocketReady(*(m_netClient->udpSocket()))) {
	  if(SDLNet_UDP_Recv(*(m_netClient->udpSocket()), v_udpReceiptPacket) == 1) {

	    NetAction* v_netAction;
	    try {
	      v_netAction = ActionReader::UDPReadAction(v_udpReceiptPacket->data, v_udpReceiptPacket->len);
	      m_netClient->addNetAction(v_netAction);
	    } catch(Exception &e) {
	      // ok, forget it, probably a bad packet received
	      LogError("client: bad UDP packet received (%s)", e.getMsg().c_str());
	    }

	  }
	}

	else if(SDLNet_SocketReady(*(m_netClient->tcpSocket()))) {
	  try {
	    while(v_tcpReader.TCPReadAction(m_netClient->tcpSocket(), &v_netAction)) {
	      m_netClient->addNetAction(v_netAction);
	    }
	  } catch(Exception &e) {
	    LogError("client: bad TCP packet received (%s)", e.getMsg().c_str());
	    m_askThreadToEnd = true; // invalid command, stop the listener
	    v_onError = true;
	  }
	}
      }
    }
  }

  SDLNet_TCP_DelSocket(v_set, *(m_netClient->tcpSocket()));
  SDLNet_TCP_DelSocket(v_set, *(m_netClient->udpSocket()));
  SDLNet_FreeSocketSet(v_set);

  SDLNet_FreePacket(v_udpReceiptPacket);

  if(v_onError) {
    LogInfo("client: ending on error");
    StateManager::instance()->sendAsynchronousMessage("CLIENT_DISCONNECTED_BY_ERROR");
  } else {
    LogInfo("client: ending normally");
  }

  StateManager::instance()->sendAsynchronousMessage("CLIENT_STATUS_CHANGED");

  return 0;
}
