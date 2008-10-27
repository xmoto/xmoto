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
#include "../../include/xm_SDL_net.h"

class ActionReader;
class NetAction;

class NetSClient {
  public:
  NetSClient(unsigned int i_id, TCPsocket i_tcpsocket, IPaddress *i_tcpRemoteIP);
  ~NetSClient();

  TCPsocket* tcpSocket();
  IPaddress* tcpRemoteIP();
  IPaddress* udpRemoteIP();
  bool isUdpBinded() const;
  void bindUdp(IPaddress i_udpIPAdress);
  void unbindUdp();

  ActionReader* tcpReader;

  void setUdpBindKey(const std::string& i_key);
  std::string udpBindKey() const;

  void setName(const std::string& i_name);
  std::string name() const;

  void setPlayingLevelId(const std::string& i_levelId);
  std::string playingLevelId() const;

  unsigned int id() const;

  private:
  unsigned int m_id; // uniq id of the client
  TCPsocket m_tcpSocket;
  IPaddress m_tcpRemoteIP;
  IPaddress m_udpRemoteIP;
  bool m_isUdpBinded;
  std::string m_udpBindKey;
  std::string m_name;
  std::string m_playingLevelId;
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
  unsigned int m_nextClientId;

  SDLNet_SocketSet m_set;
  std::vector<NetSClient*> m_clients;

  void acceptClient();
  void manageClientTCP(unsigned int i);
  void manageClientUDP();

  void manageAction(NetAction* i_netAction, unsigned int i_client);

  // if i_execpt >= 0, send to all exept him
  void sendToAllClients(NetAction* i_netAction, int i_src, int i_subsrc, unsigned int i_except = -1);
  void sendToClient(NetAction* i_netAction, unsigned int i, int i_src, int i_subsrc);
  void removeClient(unsigned int i);
};

#endif
