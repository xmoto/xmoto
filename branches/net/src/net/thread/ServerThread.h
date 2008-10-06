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

class ActionReader;
class NetAction;

class NetSClient {
  public:
  NetSClient(TCPsocket i_tcpsocket, IPaddress *i_tcpRemoteIP);
  ~NetSClient();

  TCPsocket* tcpSocket();
  IPaddress* tcpRemoteIP();
  int udpChannel() const; // <0 => invalid
  void setChannel(int i_value);
  bool isUdpBinded() const;
  void bindUdp(UDPsocket* i_udpsd, int i_port);
  void unbindUdp(UDPsocket* i_udpsd);

  ActionReader* tcpReader;

  private:
  TCPsocket m_tcpSocket;
  IPaddress m_tcpRemoteIP;
  IPaddress m_udpRemoteIP;
  int m_udpChannel;
  bool m_isUdpBinded;
};

class ServerThread : public XMThread {
  public:
  ServerThread();
  virtual ~ServerThread();

  int realThreadFunction();

  private:
  TCPsocket m_tcpsd;
  UDPsocket m_udpsd;
  UDPpacket* m_udpPacket;

  SDLNet_SocketSet m_set;
  std::vector<NetSClient*> m_clients;

  void acceptClient();
  void manageClient(unsigned int i);

  // if i_execpt >= 0, send to all exept him
  void sendToAllClients(NetAction* i_netAction, unsigned int i_except = -1);
  void sendToClient(NetAction* i_netAction, unsigned int i);
  void removeClient(unsigned int i);
};

#endif
