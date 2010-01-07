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

#ifndef __XMNETCLIENT_H__
#define __XMNETCLIENT_H__

#include "../include/xm_SDL_net.h"
#include <string>
#include <vector>
#include "../helpers/Singleton.h"
#include "../include/xm_SDL.h"
#include "NetActions.h"

#define XM_CLIENT_MAX_UDP_PACKET_SIZE 1024 // bytes

class ClientListenerThread;
class NetAction;
class NetGhost;
class Universe;
class xmDatabase;

class NetOtherClient {
 public:
  NetOtherClient(int i_id, const std::string& i_name);
  ~NetOtherClient();

  int id() const;

  std::string name() const;
  void setName(const std::string& i_name);

  std::string lastPlayingLevelId();
  void setPlayingLevelId(xmDatabase* pDb, const std::string& i_idlevel);
  std::string playingLevelName();

  NetGhost* netGhost(unsigned int i_subsrc);
  void setNetGhost(unsigned int i_subsrc, NetGhost* i_netGhost);

 private:
  int m_id;
  std::string  m_name;
  NetGhost*    m_ghosts[NETACTION_MAX_SUBSRC];
  std::string m_playingLevelId;
  std::string m_playingLevelName;
  std::string m_lastPlayingLevelId;
};

class NetClient : public Singleton<NetClient> {
  public:
  NetClient();
  ~NetClient();

  void connect(const std::string& i_server, int i_port);
  void disconnect();
  bool isConnected();
  void send(NetAction* i_netAction, int i_subsrc);
  TCPsocket* tcpSocket();
  UDPsocket* udpSocket();
  UDPpacket* sendPacket();
  std::string udpBindKey() const;

  void executeNetActions(xmDatabase* pDb);
  void addNetAction(NetAction* i_act);

  void changeMode(NetClientMode i_mode);
  NetClientMode mode() const;

  void startPlay(Universe* i_universe);
  bool isPlayInitialized();
  void endPlay();

  std::vector<NetOtherClient*>& otherClients();

  private:
  bool m_isConnected;
  IPaddress serverIp;
  TCPsocket m_tcpsd;
  UDPsocket m_udpsd;
  ClientListenerThread* m_clientListenerThread;
  UDPpacket* m_udpSendPacket;
  std::string m_udpBindKey;

  unsigned int getOtherClientNumberById(int i_id) const;

  std::vector<NetAction*> m_netActions;
  SDL_mutex* m_netActionsMutex;

  Universe* m_universe;
  NetClientMode m_mode;

  std::vector<NetOtherClient*> m_otherClients;

  void manageAction(xmDatabase* pDb, NetAction* i_netAction);
  void cleanOtherClientsGhosts();
};

#endif