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

#ifndef __SERVERTHREAD_H__
#define __SERVERTHREAD_H__

#include "../../thread/XMThread.h"
#include <vector>
#include <SDL_net.h>

class NetSClient {
  public:
  NetSClient(TCPsocket i_socket, IPaddress* i_remoteIP);
  ~NetSClient();

  TCPsocket* socket();
  IPaddress* remoteIP();

  private:
  TCPsocket m_socket;
  IPaddress* m_remoteIP;
};

class ServerThread : public XMThread {
  public:
  ServerThread();
  virtual ~ServerThread();

  int realThreadFunction();

  private:
  SDLNet_SocketSet m_set;
  std::vector<NetSClient*> m_clients;

  void acceptClient(TCPsocket* sd);
  void manageClient(unsigned int i, void* data, int len);

  // if i_execpt >= 0, send to all exept him
  void sendToAllClients(void* data, int len, unsigned int i_except = -1);
  void sendToClient(void* data, int len, unsigned int i);
  void removeClient(unsigned int i);
};

#endif
