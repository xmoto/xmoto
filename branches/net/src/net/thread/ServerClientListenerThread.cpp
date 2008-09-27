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

#include "ServerClientListenerThread.h"
#include "../../helpers/Log.h"
#include <SDL_net.h>

#define XM_SERVER_CLIENT_BUFFER_SIZE 1024

ServerClientListenerThread::ServerClientListenerThread(TCPsocket i_csd) {
    m_csd = i_csd;
}

ServerClientListenerThread::~ServerClientListenerThread() {
}

std::string ServerClientListenerThread::getIp(IPaddress* i_ip) {
  Uint32 val = SDLNet_Read32(&i_ip->host);
  char str[3+1+3+1+3+1+3+1];
  
  snprintf(str, 3+1+3+1+3+1+3+1,"%i.%i.%i.%i",
	   val >> 24, (val >> 16) %256, (val >> 8) %256, val%256);

  return std::string(str);
}

int ServerClientListenerThread::realThreadFunction() {
  char buffer[XM_SERVER_CLIENT_BUFFER_SIZE +1];
  IPaddress *remoteIP;
  int nread;

  /* Get the remote address */
  if((remoteIP = SDLNet_TCP_GetPeerAddress(m_csd)) == NULL) {
    LogWarning("server: SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());
    return 1;
  }

  /* Print the address, converting in the host format */
  LogInfo("Host connected: %s:%d",
	  getIp(remoteIP).c_str(), SDLNet_Read16(&remoteIP->port));


  while( (nread = SDLNet_TCP_Recv(m_csd, buffer, XM_SERVER_CLIENT_BUFFER_SIZE)) > 0) {
    buffer[nread] = '\0';
    LogInfo("Client says: %s", buffer);
  }

  LogInfo("Host disconnected: %s:%d",
	  getIp(remoteIP).c_str(), SDLNet_Read16(&remoteIP->port));
  
  return 0;
}

TCPsocket* ServerClientListenerThread::socket() {
  return &m_csd;
}
