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
#include "../../helpers/Log.h"

#define XM_CLIENT_BUFFER_SIZE 1024

ClientListenerThread::ClientListenerThread(NetClient* i_netClient) {
    m_netClient = i_netClient;
}

ClientListenerThread::~ClientListenerThread() {
}

int ClientListenerThread::realThreadFunction() {
  int nread;
  char buffer[XM_CLIENT_BUFFER_SIZE+1];

  while( (nread = SDLNet_TCP_Recv(*(m_netClient->socket()),
			       buffer, XM_CLIENT_BUFFER_SIZE)) > 0) {
    buffer[nread] = '\0';
    LogInfo("client:\n%s", buffer);
  }
 
  return 1;
}
