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

#include "ServerThread.h"
#include "../../helpers/Log.h"
#include "../../helpers/VExcept.h"
#include <string>
#include "../helpers/Net.h"
#include "../../XMSession.h"
#include "../../states/StateManager.h"
#include "../ActionReader.h"
#include "../NetActions.h"
#include "../../GameText.h"
#include "../../Universe.h"
#include "../../Game.h"
#include "../../db/xmDatabase.h"
#include "../../xmscene/Level.h"
#include "../../xmscene/BikeController.h"

#define XM_SERVER_UPLOADING_FPS_PLAYER   25
#define XM_SERVER_UPLOADING_FPS_OPLAYERS 10
#define XM_SERVER_PLAYER_INACTIV_TIME_MAX  1000
#define XM_SERVER_PLAYER_INACTIV_TIME_PREV  300
#define XM_SERVER_NB_SOCKETS_MAX 128
#define XM_SERVER_MAX_UDP_PACKET_SIZE 1024 // bytes
#define XM_SERVER_PREPLAYING_TIME 300


NetSClient::NetSClient(unsigned int i_id, TCPsocket i_tcpSocket, IPaddress *i_tcpRemoteIP) {
    m_id   = i_id;
    m_mode = NETCLIENT_GHOST_MODE;
    m_isMarkedToPlay = false;
    m_numScene  = 0;
    m_numPlayer = 0;
    m_tcpSocket   = i_tcpSocket;
    m_tcpRemoteIP = *i_tcpRemoteIP;
    m_isUdpBinded = false;
    tcpReader = new ActionReader();
    m_lastActivTime        =  0;
    m_lastInactivTimeAlert = -1;
}

NetSClient::~NetSClient() {
  delete tcpReader;
}

unsigned int NetSClient::id() const {
  return m_id;
}

TCPsocket* NetSClient::tcpSocket() {
  return &m_tcpSocket;
}

IPaddress* NetSClient::tcpRemoteIP() {
  return &m_tcpRemoteIP;
}

IPaddress* NetSClient::udpRemoteIP() {
  return &m_udpRemoteIP;
}

bool NetSClient::isUdpBinded() const {
  return m_isUdpBinded;
}

void NetSClient::bindUdp(IPaddress i_udpIPAdress) {
  m_udpRemoteIP = i_udpIPAdress;

  LogInfo("server: host binded: %s:%i (UDP)",
	  XMNet::getIp(&m_udpRemoteIP).c_str(), SDLNet_Read16(&(m_udpRemoteIP.port)));
  m_isUdpBinded = true;
}

void NetSClient::unbindUdp() {
  m_isUdpBinded = false;
}

void NetSClient::setUdpBindKey(const std::string& i_key) {
  m_udpBindKey = i_key;
}

std::string NetSClient::udpBindKey() const {
  return m_udpBindKey;
}

void NetSClient::setName(const std::string& i_name) {
  m_name = i_name;
}

std::string NetSClient::name() const {
  return m_name;
}

void NetSClient::setMode(NetClientMode i_mode) {
  m_mode = i_mode;
}

NetClientMode NetSClient::mode() const {
  return m_mode;
}

int NetSClient::lastActivTime() const {
  return m_lastActivTime;
}

void NetSClient::setLastActivTime(int i_time) {
  m_lastActivTime        = i_time;
  m_lastInactivTimeAlert = -1;
}

int NetSClient::lastInactivTimeAlert() const {
  return m_lastInactivTimeAlert;
}

void NetSClient::setLastInactivTimeAlert(int i_time) {
  m_lastInactivTimeAlert = i_time;
}

void NetSClient::markToPlay(bool i_value) {
  m_isMarkedToPlay = i_value;
}

bool NetSClient::isMarkedToPlay() {
  return m_isMarkedToPlay;
}

void NetSClient::markScenePlayer(unsigned int i_numScene, unsigned int i_numPlayer) {
  m_numScene  = i_numScene;
  m_numPlayer = i_numPlayer;
  m_lastActivTime = GameApp::getXMTimeInt();
}

unsigned int NetSClient::getNumScene() const {
  return m_numScene;
}

unsigned int NetSClient::getNumPlayer() const {
  return m_numPlayer;
}

void NetSClient::setPlayingLevelId(const std::string& i_levelId) {
  m_playingLevelId = i_levelId;
}

std::string NetSClient::playingLevelId() const {
  return m_playingLevelId;
}

ServerThread::ServerThread(const std::string& i_dbKey) 
  : XMThread(i_dbKey) {
    m_set = NULL;
    m_nextClientId = 0;
    m_udpPacket = SDLNet_AllocPacket(XM_SERVER_MAX_UDP_PACKET_SIZE);

    m_universe = NULL;
    SP2_setPhase(SP2_PHASE_WAIT_CLIENTS);
    m_lastFrameTimeStamp = -1;
    m_frameLate          = 0;
    m_currentFrame       = 0;

    if(!m_udpPacket) {
      throw Exception("SDLNet_AllocPacket: " + std::string(SDLNet_GetError()));
    }
}

ServerThread::~ServerThread() {
  SDLNet_FreePacket(m_udpPacket);
}

int ServerThread::realThreadFunction() {
  IPaddress ip;
  int ssn;
  unsigned int i;

  LogInfo("server: starting");

  /* Resolving the host using NULL make network interface to listen */
  if(SDLNet_ResolveHost(&ip, NULL, XMSession::instance()->serverPort()) < 0) {
    LogError("server: SDLNet_ResolveHost: %s", SDLNet_GetError());
    return 1;
  }

  m_set = SDLNet_AllocSocketSet(XM_SERVER_NB_SOCKETS_MAX);
  if(!m_set) {
    LogError("server: SDLNet_AllocSocketSet: %s", SDLNet_GetError());
    return 1;
  }

  /* Open a connection with the IP provided (listen on the host's port) */
  LogInfo("server: open connexion");
  if((m_tcpsd = SDLNet_TCP_Open(&ip)) == 0) {
    LogError("server: SDLNet_TCP_Open: %s", SDLNet_GetError());
    SDLNet_FreeSocketSet(m_set);
    return 1;
  }

  if((m_udpsd = SDLNet_UDP_Open(XMSession::instance()->serverPort())) == 0) {
    LogError("server: SDLNet_UDP_Open: %s", SDLNet_GetError());
    SDLNet_FreeSocketSet(m_set);
    SDLNet_TCP_Close(m_tcpsd);
    return 1;
  }

  ssn = SDLNet_TCP_AddSocket(m_set, m_tcpsd);
  if(ssn == -1) {
    LogError("server: SDLNet_TCP_AddSocket: %s", SDLNet_GetError());
    SDLNet_FreeSocketSet(m_set);
    SDLNet_TCP_Close(m_tcpsd);
    SDLNet_UDP_Close(m_udpsd);
    return 1;
  }
  ssn = SDLNet_UDP_AddSocket(m_set, m_udpsd);
  if(ssn == -1) {
    LogError("server: SDLNet_UDP_AddSocket: %s", SDLNet_GetError());
    SDLNet_FreeSocketSet(m_set);
    SDLNet_TCP_Close(m_tcpsd);
    SDLNet_UDP_Close(m_udpsd);
    return 1;
  }

  StateManager::instance()->sendAsynchronousMessage("SERVER_STATUS_CHANGED");

  // manage server
  while(m_askThreadToEnd == false) {
    try {
      run_loop();
    } catch(Exception &e) {
      LogWarning("Exception: %s", e.getMsg().c_str());
    }
  }

  // end the game
  SP2_setPhase(SP2_PHASE_NONE);

  // disconnection
  LogInfo("server: %i client(s) still connnected", m_clients.size());

  // disconnect all clients
  i=0;
  while(i<m_clients.size()) {
    removeClient(i);
    i++;
  }

  SDLNet_TCP_DelSocket(m_set, m_tcpsd);
  SDLNet_UDP_DelSocket(m_set, m_udpsd);

  LogInfo("server: close connexion");
  SDLNet_TCP_Close(m_tcpsd);
  SDLNet_UDP_Close(m_udpsd);

  SDLNet_FreeSocketSet(m_set);
  m_set = NULL;

  StateManager::instance()->sendAsynchronousMessage("SERVER_STATUS_CHANGED");
  LogInfo("server: ending normally");
  return 0;
}

std::string ServerThread::SP2_determineLevel() {
  char **v_result;
  unsigned int nrow;
  std::string v_id_level;
 
  // don't allow own levels (isToReload=1)
  v_result = m_pDb->readDB("SELECT id_level FROM levels WHERE isToReload=0 ORDER BY RANDOM() LIMIT 1;",
			 nrow);
  if(nrow == 0) {
    m_pDb->read_DB_free(v_result);
    throw Exception("Unable to get a level");
  }
  v_id_level = m_pDb->getResult(v_result, 1, 0, 0);  
  m_pDb->read_DB_free(v_result);
  /* */

  return v_id_level;
}

void ServerThread::SP2_initPlaying() {
  unsigned int v_numPlayer;
  unsigned int v_localNetId = 0;
  std::string v_id_level;
  m_fLastPhysTime = GameApp::getXMTime();

  v_id_level = SP2_determineLevel();

  m_universe = new Universe();
  m_universe->initPlayServer();

  try {
    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      v_numPlayer = 0;
      m_universe->getScenes()[i]->loadLevel(m_pDb, v_id_level);
      if(m_universe->getScenes()[i]->getLevelSrc()->isXMotoTooOld()) {
	throw Exception("Level " + v_id_level + " is too old");
      }
      m_universe->getScenes()[i]->prePlayLevel(NULL, true);
      
      // add the bikers
      for(unsigned int j=0; j<m_clients.size(); j++) {
	if(m_clients[j]->isMarkedToPlay()) {
	  m_universe->getScenes()[i]->addPlayerLocalBiker(v_localNetId, m_universe->getScenes()[i]->getLevelSrc()->PlayerStart(),
							  DD_RIGHT,
							  Theme::instance(), Theme::instance()->getPlayerTheme(),
							  GameApp::getColorFromPlayerNumber(v_numPlayer),
							  GameApp::getUglyColorFromPlayerNumber(v_numPlayer), false);
	  m_clients[j]->markScenePlayer(0, v_numPlayer);
	  v_numPlayer++;
	  v_localNetId++;
	}
      }
      m_universe->getScenes()[i]->playLevel();
    }
  } catch(Exception &e) {
    LogWarning("Server: Unable to load level %s",  v_id_level.c_str());
    throw Exception("Unable to load level " + v_id_level);
  }

  try {
    NA_prepareToPlay na(v_id_level);
    sendToAllClientsMarkedToPlay(&na, -1, 0);
  } catch(Exception &e) {
    /* bad */
  }

  m_sceneStartTime = (GameApp::getXMTimeInt()/10) + XM_SERVER_PREPLAYING_TIME;
  m_lastPrepareToGoAlert = -1;
}

void ServerThread::SP2_uninitPlaying() {
  delete m_universe;
  m_universe = NULL;
}

void ServerThread::SP2_manageInactivity() {
  int v_inactivDiff;
  int v_prevTime;
  Biker* v_player;

  /* kill players not playing for a too long time */
  for(unsigned int i=0; i<m_clients.size(); i++) {
    if(m_clients[i]->isMarkedToPlay()) {
      v_player = m_universe->getScenes()[m_clients[i]->getNumScene()]->Players()[m_clients[i]->getNumPlayer()];
      
      if(v_player->isDead() == false && v_player->isFinished() == false) {
	v_inactivDiff = GameApp::getXMTimeInt() - m_clients[i]->lastActivTime();
	if(XM_SERVER_PLAYER_INACTIV_TIME_MAX * 10 < v_inactivDiff) {
	  m_universe->getScenes()[m_clients[i]->getNumScene()]->killPlayer(m_clients[i]->getNumPlayer());
	} else {
	  v_prevTime = XM_SERVER_PLAYER_INACTIV_TIME_MAX * 10 - v_inactivDiff;
	  
	  if(m_clients[i]->isMarkedToPlay() && XM_SERVER_PLAYER_INACTIV_TIME_PREV * 10 > v_prevTime) {
	    if(m_clients[i]->lastInactivTimeAlert() != v_prevTime/1000) {
	      m_clients[i]->setLastInactivTimeAlert(v_prevTime/1000);
	      
	      NA_killAlert na(m_clients[i]->lastInactivTimeAlert()+1);
	      try {
		sendToClient(&na, i, -1, 0);
	      } catch(Exception &e) {
		/* hehe, ok, no pb */
	      }
	    }
	  }
	}
      }
    }
  }

}

bool ServerThread::SP2_managePreplayTime() {
  int v_waitTime;

  if(m_sceneStartTime <= GameApp::getXMTimeInt()/10 && m_lastPrepareToGoAlert < 0) {
    return false;
  }

  v_waitTime = (m_sceneStartTime - (GameApp::getXMTimeInt()/10))/100;

  if(v_waitTime < 0) { /* only the GO! has not been send and time is just under 0 */
    m_lastPrepareToGoAlert = -1;
    try {
      NA_prepareToGo na(0);
      sendToAllClientsMarkedToPlay(&na, -1, 0);
    } catch(Exception &e) {
      /* ok, not good */
    }
  } else {
    try {
      if(m_lastPrepareToGoAlert != v_waitTime) {
	NA_prepareToGo na(v_waitTime+1);
	sendToAllClientsMarkedToPlay(&na, -1, 0);
	m_lastPrepareToGoAlert = v_waitTime;
      }
    } catch(Exception &e) {
      /* ok, not good */
      }
  }

  return true;
}

void ServerThread::SP2_updateScenePlaying() {
  int nPhysSteps;
  Scene* v_scene;
  SerializedBikeState BikeState;

  if(SP2_managePreplayTime() == false) {
    SP2_manageInactivity();
  
    /* update the scene */
    nPhysSteps =0;
    while (m_fLastPhysTime + (PHYS_STEP_SIZE)/100.0 <= GameApp::getXMTime() && nPhysSteps < 10) {
      for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	v_scene = m_universe->getScenes()[i];
	v_scene->updateLevel(PHYS_STEP_SIZE, NULL);      
      }
      m_fLastPhysTime += PHYS_STEP_SIZE/100.0;
      nPhysSteps++;
    }
    
    // if the delay is too long, reinitialize
    if(m_fLastPhysTime + PHYS_STEP_SIZE/100.0 < GameApp::getXMTime()) {
      m_fLastPhysTime = GameApp::getXMTime();
    }
  }

  // send to each client his frame and the frame of the others
  if(m_currentFrame%(100/XM_SERVER_UPLOADING_FPS_PLAYER) == 0 || m_currentFrame%(100/XM_SERVER_UPLOADING_FPS_OPLAYERS) == 0) {
    for(unsigned int i=0; i<m_clients.size(); i++) {
      if(m_clients[i]->isMarkedToPlay()) {
	v_scene = m_universe->getScenes()[m_clients[i]->getNumScene()];

	v_scene->getSerializedBikeState(v_scene->Players()[m_clients[i]->getNumPlayer()]->getState(),
					v_scene->getTime(), &BikeState, v_scene->getPhysicsSettings());
	NA_frame na(&BikeState);
	try {
	  if(m_currentFrame%(100/XM_SERVER_UPLOADING_FPS_PLAYER) == 0) {
	    sendToClient(&na, i, -1, 0);
	  }
	  if(m_currentFrame%(100/XM_SERVER_UPLOADING_FPS_OPLAYERS) == 0) {
	    sendToAllClientsMarkedToPlay(&na, m_clients[i]->id(), 0, i);
	  }
	} catch(Exception &e) {
	}
      }
    }
  }

  m_currentFrame = (m_currentFrame +1) % 1000;
}

void ServerThread::SP2_updateCheckScenePlaying() {
  Scene* v_scene;
  bool v_nobodyPlaying = true;

  for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
    v_scene = m_universe->getScenes()[i];
    for(unsigned int j=0; j<v_scene->Players().size(); j++) {
      if(v_scene->Players()[j]->isDead() == false && v_scene->Players()[j]->isFinished() == false) {
	v_nobodyPlaying = false;
      }
    }
  }

  if(v_nobodyPlaying) {
    SP2_setPhase(SP2_PHASE_WAIT_CLIENTS);
  }
}

void ServerThread::SP2_setPhase(ServerP2Phase i_sp2phase) {
  SP2_unsetPhase();
  m_sp2phase = i_sp2phase;

  switch(m_sp2phase) {
  case SP2_PHASE_NONE:
    m_wantedSleepingFramerate = 1;
    break;

  case SP2_PHASE_WAIT_CLIENTS:
    m_wantedSleepingFramerate = 10;
    break;
    
  case SP2_PHASE_PLAYING:
    m_wantedSleepingFramerate = 200;
    try {
      SP2_initPlaying();
    } catch(Exception &e) {
      LogWarning("Unable to init playing (%s)", e.getMsg().c_str());
      SP2_setPhase(SP2_PHASE_WAIT_CLIENTS);
    }
    break;
  }
}

void ServerThread::SP2_unsetPhase() {
  switch(m_sp2phase) {
  case SP2_PHASE_NONE:
    break;

  case SP2_PHASE_WAIT_CLIENTS:
    break;
    
  case SP2_PHASE_PLAYING:
    SP2_uninitPlaying();
    break;
  }
}

unsigned int ServerThread::nbClientsInMode(NetClientMode i_mode) {
  unsigned int n=0;

  for(unsigned int i=0; i<m_clients.size(); i++) {
    if(m_clients[i]->mode() == i_mode) {
      n++;
    }
  }

  return n;
}

void ServerThread::run_loop() {
  manageNetwork();

  switch(m_sp2phase) {

  case SP2_PHASE_NONE:
    break;

  case SP2_PHASE_WAIT_CLIENTS:
    if(nbClientsInMode(NETCLIENT_SLAVE_MODE) > 0) {
      // mark clients as to play
      for(unsigned int i=0; i<m_clients.size(); i++) {
	if(m_clients[i]->mode() == NETCLIENT_SLAVE_MODE) {
	  m_clients[i]->markToPlay(true);
	}
      }
      SP2_setPhase(SP2_PHASE_PLAYING);
    }
    break;
    
  case SP2_PHASE_PLAYING:
    SP2_updateScenePlaying();
    SP2_updateCheckScenePlaying();
    break;
    
  }

  GameApp::wait(m_lastFrameTimeStamp, m_frameLate, m_wantedSleepingFramerate);
}

void ServerThread::manageNetwork() {
  int n_activ;
  unsigned int i;

  n_activ = SDLNet_CheckSockets(m_set, 0);
  if(n_activ == -1) {
    LogError("SDLNet_CheckSockets: %s", SDLNet_GetError());
    m_askThreadToEnd = true;
  } else {
    
    if(n_activ != 0) {
      // server socket
      if(SDLNet_SocketReady(m_tcpsd)) {
	acceptClient();
      }
      
      else if(SDLNet_SocketReady(m_udpsd)) {
	manageClientUDP();
      } else {
	i = 0;
	while(i<m_clients.size()) {
	  if(SDLNet_SocketReady(*(m_clients[i]->tcpSocket()))) {
	    try {
	      manageClientTCP(i);
	      i++;
	    } catch(DisconnectedException &e) {
	      LogInfo("server: client %u disconnected (%s:%d) : %s", i,
		      XMNet::getIp(m_clients[i]->tcpRemoteIP()).c_str(),
		      SDLNet_Read16(&(m_clients[i]->tcpRemoteIP())->port), e.getMsg().c_str());
	      removeClient(i);	
	    } catch(Exception &e) {
	      LogInfo("server: bad TCP packet received by client %u (%s:%d) : %s", i,
		      XMNet::getIp(m_clients[i]->tcpRemoteIP()).c_str(),
		      SDLNet_Read16(&(m_clients[i]->tcpRemoteIP())->port), e.getMsg().c_str());
	      removeClient(i);
	    }
	  } else {
	    i++;
	  }
	}
      }
    }    
  }
}

void ServerThread::removeClient(unsigned int i) {
  // remove the client from the scene if he is playing
  if(m_universe != NULL) {
    if(m_clients[i]->isMarkedToPlay()) {
      m_universe->getScenes()[m_clients[i]->getNumScene()]->killPlayer(m_clients[i]->getNumPlayer());
    }
  }

  SDLNet_TCP_DelSocket(m_set, *(m_clients[i]->tcpSocket()));
  SDLNet_TCP_Close(*(m_clients[i]->tcpSocket()));
  if(m_clients[i]->isUdpBinded()) {
    m_clients[i]->unbindUdp();
  }

  // send new client to other clients
  NA_changeClients nacc;
  NetInfosClient nic;
  try {
    nic.NetId = m_clients[i]->id();
    nic.Name  = m_clients[i]->name();
    nacc.remove(&nic);
    sendToAllClients(&nacc, -1, 0, i);
  } catch(Exception &e) {
  }

  delete m_clients[i];
  m_clients.erase(m_clients.begin() + i);
}

void ServerThread::sendToClient(NetAction* i_netAction, unsigned int i, int i_src, int i_subsrc) {
  i_netAction->setSource(i_src, i_subsrc);
  if(m_clients[i]->isUdpBinded()) {
    i_netAction->send(m_clients[i]->tcpSocket(), &m_udpsd, m_udpPacket, m_clients[i]->udpRemoteIP());
  } else {
    i_netAction->send(m_clients[i]->tcpSocket(), NULL, NULL, NULL);
  }
}

void ServerThread::sendToAllClientsHavingMode(NetClientMode i_mode,
					      NetAction* i_netAction, int i_src, int i_subsrc, unsigned int i_except) {
  for(unsigned int i=0; i<m_clients.size(); i++) {
    if(i != i_except && (i_mode == NETCLIENT_ANY_MODE || i_mode == m_clients[i]->mode())) {
      try {
	sendToClient(i_netAction, i, i_src, i_subsrc);
      } catch(Exception &e) {
	// don't remove the client while removeclient function can call sendToAllClients ...
      }
    }
  }
}

void ServerThread::sendToAllClients(NetAction* i_netAction, int i_src, int i_subsrc, unsigned int i_except) {
  sendToAllClientsHavingMode(NETCLIENT_ANY_MODE, i_netAction, i_src, i_subsrc, i_except);
}

void ServerThread::sendToAllClientsMarkedToPlay(NetAction* i_netAction, int i_src, int i_subsrc, unsigned int i_except) {
  for(unsigned int i=0; i<m_clients.size(); i++) {
    if(i != i_except && m_clients[i]->isMarkedToPlay()) {
      try {
	sendToClient(i_netAction, i, i_src, i_subsrc);
      } catch(Exception &e) {
	// don't remove the client while removeclient function can call sendToAllClients ...
      }
    }
  }
}

void ServerThread::acceptClient() {
  TCPsocket csd;
  IPaddress *tcpRemoteIP;
  int scn;

  if((csd = SDLNet_TCP_Accept(m_tcpsd)) == 0) {
    return;
  }

  /* Get the remote address */
  if((tcpRemoteIP = SDLNet_TCP_GetPeerAddress(csd)) == NULL) {
    LogWarning("server: SDLNet_TCP_GetPeerAddress: %s", SDLNet_GetError());
    SDLNet_TCP_Close(csd);
    return;
  }

  /* Print the address, converting in the host format */
  LogInfo("server: host connected: %s:%d (TCP)",
	  XMNet::getIp(tcpRemoteIP).c_str(), SDLNet_Read16(&tcpRemoteIP->port));

  // to much clients ?
  if(m_clients.size() >= XMSession::instance()->serverMaxClients()) {
    NA_serverError na(UNTRANSLATED_GAMETEXT_TOO_MUCH_CLIENTS);
    na.setSource(-1, 0);
    try {
      na.send(&csd, NULL, NULL, NULL);
    } catch(Exception &e) {
    }

    SDLNet_TCP_Close(csd);
    return;
  }

  scn = SDLNet_TCP_AddSocket(m_set, csd);
  if(scn == -1) {
    LogError("server: SDLNet_TCP_AddSocket: %s", SDLNet_GetError());
    SDLNet_TCP_Close(csd);
    return;
  }

  m_clients.push_back(new NetSClient(m_nextClientId++, csd, tcpRemoteIP));
}

void ServerThread::manageClientTCP(unsigned int i) {
  NetAction* v_netAction;

  while(m_clients[i]->tcpReader->TCPReadAction(m_clients[i]->tcpSocket(), &v_netAction)) {
    manageAction(v_netAction, i);
    delete v_netAction;
  }
}

void ServerThread::manageClientUDP() {
  bool v_managedPacket;
  NetAction* v_netAction;

  if(SDLNet_UDP_Recv(m_udpsd, m_udpPacket) == 1) {
    v_managedPacket = false;
    for(unsigned int i=0; i<m_clients.size(); i++) {
      if(m_clients[i]->udpRemoteIP()->host == m_udpPacket->address.host && 
	 m_clients[i]->udpRemoteIP()->port == m_udpPacket->address.port)  {
	v_managedPacket = true;
	try {
	  v_netAction = ActionReader::UDPReadAction(m_udpPacket->data, m_udpPacket->len);
	  manageAction(v_netAction, i);
	  delete v_netAction;
	} catch(Exception &e) {
	  // ok, a bad packet received, forget it
	  LogWarning("server: bad UDP packet received by client %u (%s:%i) : %s", i,
		     XMNet::getIp(&(m_udpPacket->address)).c_str(), SDLNet_Read16(&(m_udpPacket->address.port)), e.getMsg().c_str());
	}
	break; // stop : only one client
      }
    }

    // anonym packet ? find the associated client
    if(v_managedPacket == false) {
      try {
	v_netAction = ActionReader::UDPReadAction(m_udpPacket->data, m_udpPacket->len);
	if(v_netAction->actionType() == TNA_udpBind) {
	  for(unsigned int i=0; i<m_clients.size(); i++) {
	    if(m_clients[i]->isUdpBinded() == false) {
	      if(m_clients[i]->udpBindKey() == ((NA_udpBind*)v_netAction)->key()) {
		//LogInfo("UDP bind key received via UDP: %s", ((NA_udpBind*)v_netAction)->key().c_str());
		m_clients[i]->bindUdp(m_udpPacket->address);
		break; // stop : only one client
	      }
	    }
	  }
	} else {
	  LogWarning("Packet of unknown client received");
	}
      } catch(Exception &e) {
	/* forget this bad packet */
	LogWarning("server: bad anonym UDP packet received by %s:%i",
		   XMNet::getIp(&(m_udpPacket->address)).c_str(), SDLNet_Read16(&(m_udpPacket->address.port)));
      }
    }
  }
}

void ServerThread::manageAction(NetAction* i_netAction, unsigned int i_client) {
  Scene* v_scene;
  unsigned int v_numPlayer;

  switch(i_netAction->actionType()) {

  case TNA_udpBind:
    /* managed before */
    break;

  case TNA_udpBindQuery:
  case TNA_serverError:
  case TNA_changeClients:
  case TNA_prepareToPlay:
  case TNA_prepareToGo:
  case TNA_killAlert:
    {
      /* should not be received */
      throw Exception("");
    }
    break;

  case TNA_clientInfos:
    {
      // check protocol version
      if(((NA_clientInfos*)i_netAction)->protocolVersion() != XM_NET_PROTOCOL_VERSION) {
	NA_serverError na(UNTRANSLATED_GAMETEXT_SERVER_PROTOCOL_VERSION_INCOMPATIBLE);
	na.setSource(-1, 0);
	try {
	  sendToClient(&na, i_client, -1, 0);
	} catch(Exception &e) {
	}
	throw Exception("Protocol version incompatible");
      }

      // udpBindKey received
      //LogInfo("Protocol version of client %i is %i", i_client, ((NA_clientInfos*)i_netAction)->protocolVersion());
      //LogInfo("UDP bind key of client %i is %s", i_client, ((NA_clientInfos*)i_netAction)->udpBindKey().c_str());
      m_clients[i_client]->setUdpBindKey(((NA_clientInfos*)i_netAction)->udpBindKey());
      
      // query bind udp
      NA_udpBindQuery naq;
      try {
      	sendToClient(&naq, i_client, -1, 0);
      } catch(Exception &e) {
      }

      // send the current clients list to the client only when the client has a name
      NA_changeClients nacc;
      NetInfosClient nic;
      try {
	for(unsigned int i=0; i<m_clients.size(); i++) {
	  if(m_clients[i]->name() != "") {
	    nic.NetId = m_clients[i]->id();
	    nic.Name  = m_clients[i]->name();
	    nacc.add(&nic);
	  }
	}
      	sendToClient(&nacc, i_client, -1, 0);
      } catch(Exception &e) {
      }
    }
    break;
      
  case TNA_chatMessage:
    {
      sendToAllClients(i_netAction, m_clients[i_client]->id(), i_netAction->getSubSource(), i_client);
    }
    break;
    
  case TNA_frame:
    {
      for(unsigned int i=0; i<m_clients.size(); i++) {
	if(m_clients[i_client]->mode() == NETCLIENT_GHOST_MODE) {
	  if(i != i_client &&
	     m_clients[i_client]->playingLevelId() != "" &&
	     m_clients[i_client]->playingLevelId() == m_clients[i]->playingLevelId()
	     ) {
	    try {
	      sendToClient(i_netAction, i, m_clients[i_client]->id(), i_netAction->getSubSource());
	    } catch(Exception &e) {
	      // don't remove the client, it will be done in the main loop
	    }
	  }
	}
      }
    }
    break;

  case TNA_changeName:
    {
      m_clients[i_client]->setName(((NA_changeName*)i_netAction)->getName());
      if(m_clients[i_client]->name() == "") {
	throw Exception("Invalid name provided");
      }
      LogInfo("Client[%i]'s name is \"%s\"", i_client, m_clients[i_client]->name().c_str());

      // send new client to other clients
      NA_changeClients nacc;
      NetInfosClient nic;
      try {
	nic.NetId = m_clients[i_client]->id();
	nic.Name  = m_clients[i_client]->name();
	nacc.add(&nic);
      	sendToAllClients(&nacc, -1, 0, i_client);
      } catch(Exception &e) {
      }

    }
    break;

  case TNA_playingLevel:
    {
      m_clients[i_client]->setPlayingLevelId(((NA_playingLevel*)i_netAction)->getLevelId());
      sendToAllClients(i_netAction, m_clients[i_client]->id(), i_netAction->getSubSource(), i_client);
    }
    break;

  case TNA_playerControl:
    {
      if(m_universe != NULL) {
	m_clients[i_client]->setLastActivTime(GameApp::getXMTimeInt());
	v_scene     = m_universe->getScenes()[m_clients[i_client]->getNumScene()];
	v_numPlayer = m_clients[i_client]->getNumPlayer();

	// apply the control on the client
	switch(((NA_playerControl*)i_netAction)->getType()) {
	case PC_BRAKE:
	  v_scene->Players()[v_numPlayer]->getControler()->setBreak(((NA_playerControl*)i_netAction)->getFloatValue());
	  break;
	case PC_THROTTLE:
	  v_scene->Players()[v_numPlayer]->getControler()->setThrottle(((NA_playerControl*)i_netAction)->getFloatValue());
	    break;
	case PC_PULL:
	  v_scene->Players()[v_numPlayer]->getControler()->setPull(((NA_playerControl*)i_netAction)->getFloatValue());
	  break;
	case PC_CHANGEDIR:
	  v_scene->Players()[v_numPlayer]->getControler()->setChangeDir(((NA_playerControl*)i_netAction)->getBoolValue());
	  break;
	}
      }
    }
    break;

  case TNA_clientMode:
    {
      m_clients[i_client]->setMode(((NA_clientMode*)i_netAction)->mode());
    }
    break;

  }
}
