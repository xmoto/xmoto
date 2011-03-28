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
class VirtualNetLevelsList;

class NetOtherClient {
 public:
  NetOtherClient(int i_id, const std::string& i_name);
  ~NetOtherClient();

  int id() const;

  std::string name() const;
  NetClientMode mode() const;
  void setName(const std::string& i_name);
  void setMode(NetClientMode i_netMode);

  std::string lastPlayingLevelId();
  void setPlayingLevelId(xmDatabase* pDb, const std::string& i_idlevel);
  std::string playingLevelName();

  NetGhost* netGhost(unsigned int i_subsrc);
  void setNetGhost(unsigned int i_subsrc, NetGhost* i_netGhost);

  int points() const;
  void setPoints(int i_points);

 private:
  int m_id;
  std::string  m_name;
  NetGhost*    m_ghosts[NETACTION_MAX_SUBSRC];
  NetClientMode m_netMode;
  std::string m_playingLevelId;
  std::string m_playingLevelName;
  std::string m_lastPlayingLevelId;
  int m_points;
};

class NetClient : public Singleton<NetClient> {
  public:
  NetClient();
  ~NetClient();

  // wake up the server ; don't abuse of this function
  static void fastConnectDisconnect(const std::string& i_server, int i_port);

  void connect(const std::string& i_server, int i_port);
  void disconnect();
  bool isConnected();
  void send(NetAction* i_netAction, int i_subsrc, bool i_forceUdp = false);
  TCPsocket* tcpSocket();
  UDPsocket* udpSocket();
  UDPpacket* sendPacket();
  std::string udpBindKey() const;

  void executeNetActions(xmDatabase* pDb);
  void addNetAction(NetAction* i_act);

  void changeMode(NetClientMode i_mode);
  NetClientMode mode() const;
  int points() const;

  void startPlay(Universe* i_universe);
  bool isPlayInitialized();
  void endPlay();

  void getOtherClientsNameList(std::vector<std::string>& io_list, const std::string& i_suffix);
  void addChatTransformations(std::vector<std::string>& io_clientList, const std::string& i_suffix);
  std::string getDisplayMessage(const std::string& i_msg, const std::string& i_author);

  std::vector<NetOtherClient*>& otherClients();
  void fillPrivatePeople(const std::string& i_msg, const std::string& i_private_suffix, std::vector<int>& io_private_people,
			 std::vector<std::string>& o_unknown_players);

  int getOwnFrameFPS() const;
  VirtualNetLevelsList* getOtherClientLevelsList(xmDatabase* pDb);

  void memoriesPP(const std::vector<int>& i_private_people);
  std::string getMemoriedPPAsString(const std::string& i_suffix);

  private:
  bool m_isConnected;
  IPaddress serverIp;
  bool m_serverReceivesUdp;
  bool m_serverSendsUdp;
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
  int m_points;

  void updateOtherClientsMode(std::vector<int> i_slavePlayers);

  void manageAction(xmDatabase* pDb, NetAction* i_netAction);
  void cleanOtherClientsGhosts();

  int m_lastOwnFPS;
  int m_currentOwnFramesNb;
  int m_currentOwnFramesTime;

  VirtualNetLevelsList* m_otherClientsLevelsList;
  std::vector<int> m_previous_private_people;

  NetPing m_lastPing;
};

#endif
