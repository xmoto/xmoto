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
#include "../../xmscene/Scene.h"
#include "../NetActions.h"

class ActionReader;
class NetAction;
class Universe;
class DBuffer;
class ServerRules;
class XMServerSceneHooks;

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
  void validUdpBind();
  bool isUdpBindedValidated() const;
  void unbindUdp();

  ActionReader* tcpReader;

  void setUdpBindKey(const std::string& i_key);
  std::string udpBindKey() const;

  void setProtocolVersion(int i_protocolVersion);
  int protocolVersion() const;

  void setXmVersion(const std::string& i_xmversion);
  std::string xmversion() const;

  void setName(const std::string& i_name);
  std::string name() const;
  void setMode(NetClientMode i_mode);
  NetClientMode mode() const;

  // indicates whether the client is in the current SP2 party
  void markToPlay(ServerRules* i_rules, bool i_value);
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
  int points();
  void addPoints(int i_points);
  void setPoints(int i_points);

  // admin connexion
  bool isAdminConnected() const;
  void setAdminConnected(bool i_value);
  std::string adminLoginName() const;

  int lastGhostFrameTime() const;
  void setLastGhostFrameTime(int v_time);

  NetPing* lastPing();

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
  bool m_isUdpBindedValidated;
  std::string m_udpBindKey;
  std::string m_name;
  int m_protocolVersion;
  std::string m_xmversion;
  std::string m_playingLevelId;
  int m_lastActivTime;
  int m_lastInactivTimeAlert;
  int m_points;
  bool m_isAdminConnected;
  std::string m_adminLoginName; // because you can change your name later, so this is your name at the moment you login
  int m_lastGhostFrameTime;
  NetPing m_lastPing;
};

class ServerThread : public XMThread {
  public:
  ServerThread(const std::string& i_dbKey, int i_port, const std::string& i_adminPassword = "" /* empty to disable this feature */);
  virtual ~ServerThread();

  bool acceptConnections() const;
  int realThreadFunction();
  void close(); // close the server if an event need it (ctrl+c)
  int port() const;

  NetSClient* getNetSClientByScenePlayer(unsigned int i_numScene, unsigned int i_numPlayer) const;
  NetSClient* getNetSClientById(unsigned int i_id) const;
  Universe*   getUniverse(); // NULL if no party is currently playing
  ServerRules* getRules();
  void sendPointsToSlavePlayers();

  private:
  TCPsocket m_tcpsd;
  UDPsocket m_udpsd;
  UDPpacket* m_udpPacket;
  unsigned int m_nextClientId;
  NetActionU m_preAllocatedNA;

  int m_port;
  std::string m_adminPassword;

  Universe* m_universe;
  DBuffer* m_DBuffer;
  ServerP2Phase m_sp2phase;
  int m_lastPhysTime;
  int m_lastFrameTimeStamp;
  int m_frameLate;
  int m_currentFrame;
  int m_sceneStartTime;
  int m_lastPrepareToGoAlert;
  int m_firstFrameSent; // send the first frame only one time before the game starts

  unsigned int m_nFollowingUdp; // to avoid tcp famine
  std::string m_startTimeStr;
  std::string m_banner;
  bool m_acceptConnections;
  int m_unmanagedActions;

  SDLNet_SocketSet m_set;
  std::vector<NetSClient*> m_clients;
  ServerRules* m_rules;
  bool         m_needToReloadRules; // rules are reloaded only when out of a round, not immediatly when requested
  XMServerSceneHooks* m_sceneHook;

  void acceptClient();
  bool manageClientTCP(unsigned int i);
  void manageClientUDP();

  // return false if the client must be deconnected
  bool manageAction(NetAction* i_netAction, unsigned int i_client);

  void run_loop();
  
  void manageNetwork(int i_timeout); // 0 for no timeout ; -1 for the maximum
  // manageNetwork helper : 
  bool manageNetworkOnePacket(int i_timeout); // return true if there is possibly something else to manage on network

  // if i_execpt >= 0, send to all exept him
  void sendToAllClients(NetAction* i_netAction, int i_src, int i_subsrc, int i_except = -1);
  void sendToAllClientsHavingMode(NetClientMode i_mode, NetAction* i_netAction, int i_src, int i_subsrc, int i_except = -1);
  void sendToAllClientsMarkedToPlay(NetAction* i_netAction, int i_src, int i_subsrc, int i_except = -1);
  void sendToAllClientsHavingProtocol(int i_protocol, NetAction* i_netAction_lt, NetAction* i_netAction_ge, int i_src, int i_subsrc, int i_except = -1);
  void sendToAllClientsHavingModeAndProtocol(NetClientMode i_mode, int i_protocol, NetAction* i_netAction_lt, NetAction* i_netAction_ge, int i_src, int i_subsrc, int i_except = -1);
  void sendToClient(NetAction* i_netAction, unsigned int i, int i_src, int i_subsrc, bool i_forceUdp = false);
  void sendMsgToClient(unsigned int i_client, const std::string& i_msg);
  void removeClient(unsigned int i);
  unsigned int nbClientsInMode(NetClientMode i_mode);

  /* SP2 */
  void sendPointsToClient(unsigned int i_client);
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
  bool m_sp2_gameStarted;
  int m_sp2_lastLoopTime;
  int m_sp2_lastLoopDelta;

  // server cmd
  unsigned int getClientById(unsigned int i_id) const;
  void manageSrvCmd(unsigned int i_client, const std::string& i_cmd);

  std::vector<unsigned int> m_clientMarkToBeRemoved;
  void cleanClientsMarkedToBeRemoved();

  // rules
  void reloadRules(const std::string& i_rulesFile);
};

class XMServerSceneHooks : public SceneHooks {
public:
  XMServerSceneHooks(ServerThread* i_st);
  virtual ~XMServerSceneHooks();

  void OnEntityToTakeTakenByPlayer(unsigned int i_player);
  void OnEntityToTakeTakenExternal();
  void OnPlayerWins(unsigned int i_player);
  void OnPlayerDies(unsigned int i_player);
  void OnPlayerSomersault(unsigned int i_player, bool i_counterclock);

private:
  ServerThread* m_server;
};

#endif
