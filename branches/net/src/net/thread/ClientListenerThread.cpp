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
#include "../../helpers/Log.h"
#include "../../helpers/VExcept.h"
#include "../../XMSession.h"
#include "../../states/StateManager.h"

#define XM_CLIENT_WAIT_TIMEOUT 1000
#define XM_CLIENT_MAX_PACKET_SIZE 1024 * 10 // bytes
#define XM_CLIENT_MAX_PACKET_SIZE_DIGITS 6 // limit the size of a command : n digits

ClientListenerThread::ClientListenerThread(NetClient* i_netClient) {
    m_netClient = i_netClient;
}

ClientListenerThread::~ClientListenerThread() {
}

int ClientListenerThread::realThreadFunction() {
  int nread;
  char buffer[XM_CLIENT_MAX_PACKET_SIZE];
  std::string v_cmd;
  unsigned int v_packetSize;
  unsigned int v_cmdStart;
  SDLNet_SocketSet v_set;
  int scn;
  int n_activ;
  bool v_onError = false;
  unsigned int v_packetOffset = 0;
  bool v_notEnoughData;

  // use a set to get the timeout
  v_set = SDLNet_AllocSocketSet(1);
  if(!v_set) {
    LogError("client: SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
    return 1;
  }

  scn = SDLNet_TCP_AddSocket(v_set, *(m_netClient->socket()));
  if(scn == -1) {
    LogError("client: SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError());
    return 1;
  }

  StateManager::instance()->sendAsynchronousMessage("CLIENT_STATUS_CHANGED");

  /* Wait for a connection */
  while(m_askThreadToEnd == false) {
    n_activ = SDLNet_CheckSockets(v_set, XM_CLIENT_WAIT_TIMEOUT);
    if(n_activ == -1) {
      LogError("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
      m_askThreadToEnd = true;
      v_onError = true;
    } else {
      if(n_activ != 0) {
	if(SDLNet_SocketReady(*(m_netClient->socket()))) {
	  if( (nread = SDLNet_TCP_Recv(*(m_netClient->socket()),
				       buffer+v_packetOffset,
				       XM_CLIENT_MAX_PACKET_SIZE-v_packetOffset)) > 0) {
	    v_packetOffset += nread;
	    LogDebug("Data received (%u bytes available)", v_packetOffset);
	    v_notEnoughData = false; // you don't know if the buffer is full enough

	    while( (v_packetSize = getSubPacketSize(buffer, v_packetOffset, v_cmdStart)) > 0 &&
		   v_notEnoughData  == false &&
		   m_askThreadToEnd == false) {
	      LogDebug("Packet size is %u", v_packetSize);
	      try {
		if(v_packetOffset-v_cmdStart >= v_packetSize) {
		  LogDebug("One packet to manage");
		  m_netClient->addNetAction(NetAction::newNetAction(((char*)buffer)+v_cmdStart, v_packetSize));
		  
		  // remove the managed packet
		  // main case : the buffer contains exactly one command
		  if(v_packetOffset-v_cmdStart == v_packetSize) {
		    v_packetOffset = 0;
		  } else {
		    // the buffer contains two or more commands
		    // remove the packet
		    v_packetOffset = v_packetOffset - v_cmdStart - v_packetSize;
		    memcpy(buffer, buffer+v_cmdStart+v_packetSize, v_packetOffset);
		  }
		  LogDebug("Packet offset set to %u", v_packetOffset);
		} else {
		  // wait for more data to have a full command
		  LogDebug("Not enough data to make a packet");
		  v_notEnoughData = true;
		}
	      } catch(Exception &e) {
		LogError("client: bad command received (%s)", e.getMsg().c_str());
		m_askThreadToEnd = true; // invalid command, stop the listener
		v_onError = true;
	      }
	    }
	  }
	}
      }
    }
  }

  SDLNet_TCP_DelSocket(v_set, *(m_netClient->socket()));
  SDLNet_FreeSocketSet(v_set);

  if(v_onError) {
    LogInfo("client: ending on error");
  } else {
    LogInfo("client: ending normally");
  }

  StateManager::instance()->sendAsynchronousMessage("CLIENT_STATUS_CHANGED");

  return 0;
}

// return the size of the packet or 0 if no packet is available
unsigned int ClientListenerThread::getSubPacketSize(void* data, unsigned int len, unsigned int& o_cmdStart) {
  unsigned int i=0;
  unsigned int res;

  while(i<len && i<XM_CLIENT_MAX_PACKET_SIZE_DIGITS+1) {
    if(((char*)data)[i] == '\n') {
      o_cmdStart = i+1;
      ((char*)data)[i] = '\0';
      res = atoi((char*)data);
      ((char*)data)[i] = '\n'; // must be reset in case packet is not full
      return res;
    }
    i++;
  }
  return 0;
}
