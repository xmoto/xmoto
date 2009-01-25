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
#include "../BasicStructures.h"

class ActionReader;
class NetAction;
class Universe;
class DBuffer;

enum ServerP2Phase { SP2_PHASE_NONE, SP2_PHASE_WAIT_CLIENTS, SP2_PHASE_PLAYING };

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
  void setMode(NetClientMode i_mode);
  NetClientMode mode() const;

  // indicates whether the client is in the current SP2 party
  void markToPlay(bool i_value);
  bool isMarkedToPlay();
  void markScenePlayer(unsigned int i_numScene, unsigned int i_numPlayer);
  unsigned int getNumScene() const;
  unsigned int getNumPlayer() const;
  int lastActivTime() const;
  void setLastActivTime(int i_time);
  int lastInactivTimeAlert() const;
  void setLastInactivTimeAlert(int i_time);

  void setPlayingLevelId(const std::string& i_levelId);
  std::string playingLevelId() const;

  unsigned int id() const;

  private:
  unsigned int m_id;    // uniq id of the client
  NetClientMode m_mode; // playing mode (simple ghost or slave)
  bool m_isMarkedToPlay;
  unsigned int m_numScene; // number of the scene in which the client plays (if marked to play)
  unsigned int m_numPlayer; // number of the player in the scene which the client plays (if marked to play)
  TCPsocket m_tcpSocket;
  IPaddress m_tcpRemoteIP;
  IPaddress m_udpRemoteIP;
  bool m_isUdpBinded;
  std::string m_udpBindKey;
  std::string m_name;
  std::string m_playingLevelId;
  int m_lastActivTime;
  int m_lastInactivTimeAlert;
};

class ServerThread : public XMThread {
  public:
  ServerThread(const std::string& i_dbKey);
  virtual ~ServerThread();

  int realThreadFunction();

  private:
  TCPsocket m_tcpsd;
  UDPsocket m_udpsd;
  UDPpacket* m_udpPacket;
  unsigned int m_nextClientId;

  Universe* m_universe;
  DBuffer* m_DBuffer;
  ServerP2Phase m_sp2phase;
  double m_fLastPhysTime;
  int m_lastFrameTimeStamp;
  int m_frameLate;
  int m_wantedSleepingFramerate;
  int m_currentFrame;
  int m_sceneStartTime;
  int m_lastPrepareToGoAlert;

  SDLNet_SocketSet m_set;
  std::vector<NetSClient*> m_clients;

  void acceptClient();
  void manageClientTCP(unsigned int i);
  void manageClientUDP();

  void manageAction(NetAction* i_netAction, unsigned int i_client);

  void run_loop();
  void manageNetwork();

  // if i_execpt >= 0, send to all exept him
  void sendToAllClients(NetAction* i_netAction, int i_src, int i_subsrc, unsigned int i_except = -1);
  void sendToAllClientsHavingMode(NetClientMode i_mode, NetAction* i_netAction, int i_src, int i_subsrc, unsigned int i_except = -1);
  void sendToAllClientsMarkedToPlay(NetAction* i_netAction, int i_src, int i_subsrc, unsigned int i_except = -1);
  void sendToClient(NetAction* i_netAction, unsigned int i, int i_src, int i_subsrc);
  void removeClient(unsigned int i);
  unsigned int nbClientsInMode(NetClientMode i_mode);

  /* SP2 */
  void SP2_initPlaying();
  void SP2_uninitPlaying();
  void SP2_updateScenePlaying();
  void SP2_updateCheckScenePlaying();
  void SP2_setPhase(ServerP2Phase i_sp2phase);
  void SP2_unsetPhase();
  void SP2_manageInactivity();
  bool SP2_managePreplayTime();
  std::string SP2_determineLevel();
  void SP2_sendSceneEvents(DBuffer* i_buffer);
};

#endif