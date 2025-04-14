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
#include "../ActionReader.h"
#include "../NetActions.h"
#include "../ServerRules.h"
#include "../helpers/Net.h"
#include "common/DBuffer.h"
#include "common/XMSession.h"
#include "db/xmDatabase.h"
#include "helpers/Log.h"
#include "helpers/System.h"
#include "helpers/VExcept.h"
#include "helpers/utf8.h"
#include "states/StateManager.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"
#include "xmoto/Universe.h"
#include "xmscene/BikeController.h"
#include "xmscene/Level.h"
#include <sstream>
#include <string>

#define XM_SERVER_SLAVE_MODE_MIN_PROTOCOL_VERSION 1

#define XM_SERVER_UPLOADING_FPS_PLAYER 40
#define XM_SERVER_UPLOADING_FPS_OPLAYERS 15
#define XM_SERVER_PLAYER_INACTIV_TIME_MAX 1000
#define XM_SERVER_PLAYER_INACTIV_TIME_PREV 300
#define XM_SERVER_NB_SOCKETS_MAX 128
#define XM_SERVER_MAX_UDP_PACKET_SIZE 1024 // bytes
#define XM_SERVER_PREPLAYING_TIME 300
#define XM_SERVER_DEFAULT_BAN_NBDAYS 30
#define XM_SERVER_MAX_FOLLOWING_UDP 100
#define XM_SERVER_DEFAULT_BANNER "Welcome on this server"
#define XM_SERVER_UNPLAYING_SLEEP 10
#define XM_SERVER_NICK_LENGTH_MAX 16

// limit multi private message to avoid people spamming everybody and making
// think it's private
#define XM_SERVER_MAXIMUM_MULTI_PRIVATE_MESSAGE 3

#define XM_SERVER_DEFAULT_RULES "Rules/classical.rules"

NetSClient::NetSClient(unsigned int i_id,
                       TCPsocket i_tcpSocket,
                       IPaddress *i_tcpRemoteIP) {
  m_id = i_id;
  m_mode = NETCLIENT_GHOST_MODE;
  m_isMarkedToPlay = false;
  m_numScene = 0;
  m_numPlayer = 0;
  m_tcpSocket = i_tcpSocket;
  m_tcpRemoteIP = *i_tcpRemoteIP;
  m_isUdpBinded = false;
  m_isUdpBindedValidated = false;
  tcpReader = new ActionReader();
  m_lastActivTime = 0;
  m_lastInactivTimeAlert = -1;
  m_points = 0;
  m_isAdminConnected = false;
  m_lastGhostFrameTime = 0;
  m_protocolVersion = -1; // not set
  m_udpRemoteIP.host = INADDR_NONE;
  m_udpRemoteIP.port = 0;

  // last ping information
  m_lastPing.id = -1;
  m_lastPing.pingTime = -1;
  m_lastPing.pongTime = -1;
}

NetSClient::~NetSClient() {
  delete tcpReader;
}

bool NetSClient::isAdminConnected() const {
  return m_isAdminConnected;
}

void NetSClient::setAdminConnected(bool i_value) {
  m_isAdminConnected = i_value;
  m_adminLoginName = i_value ? m_name : "";
}

std::string NetSClient::adminLoginName() const {
  return m_adminLoginName;
}

unsigned int NetSClient::id() const {
  return m_id;
}

int NetSClient::points() {
  return m_points;
}

void NetSClient::setPoints(int i_points) {
  m_points = i_points;
}

void NetSClient::addPoints(int i_points) {
  m_points += i_points;
}

int NetSClient::lastGhostFrameTime() const {
  return m_lastGhostFrameTime;
}

void NetSClient::setLastGhostFrameTime(int v_time) {
  m_lastGhostFrameTime = v_time;
}

TCPsocket *NetSClient::tcpSocket() {
  return &m_tcpSocket;
}

IPaddress *NetSClient::tcpRemoteIP() {
  return &m_tcpRemoteIP;
}

IPaddress *NetSClient::udpRemoteIP() {
  return &m_udpRemoteIP;
}

bool NetSClient::isUdpBinded() const {
  return m_isUdpBinded;
}

bool NetSClient::isUdpBindedValidated() const {
  return m_isUdpBindedValidated;
}
void NetSClient::bindUdp(IPaddress i_udpIPAdress) {
  m_udpRemoteIP = i_udpIPAdress;

  LogInfo("server: host binded: %s:%i (UDP)",
          XMNet::getIp(&m_udpRemoteIP).c_str(),
          SDLNet_Read16(&(m_udpRemoteIP.port)));
  m_isUdpBinded = true;
}

void NetSClient::validUdpBind() {
  m_isUdpBindedValidated = true;
}

void NetSClient::unbindUdp() {
  m_isUdpBinded = false;
}

void NetSClient::setUdpBindKey(const std::string &i_key) {
  m_udpBindKey = i_key;
}

std::string NetSClient::udpBindKey() const {
  return m_udpBindKey;
}

void NetSClient::setProtocolVersion(int i_protocolVersion) {
  m_protocolVersion = i_protocolVersion;
}

int NetSClient::protocolVersion() const {
  return m_protocolVersion;
}

void NetSClient::setXmVersion(const std::string &i_xmversion) {
  m_xmversion = i_xmversion;
}

std::string NetSClient::xmversion() const {
  return m_xmversion;
}

void NetSClient::setName(const std::string &i_name) {
  m_name = i_name;
}

std::string NetSClient::name() const {
  return m_name;
}

void NetSClient::setMode(NetClientMode i_mode) {
  m_mode = i_mode;
  m_playingLevelId = ""; // reset
}

NetClientMode NetSClient::mode() const {
  return m_mode;
}

int NetSClient::lastActivTime() const {
  return m_lastActivTime;
}

void NetSClient::setLastActivTime(int i_time) {
  m_lastActivTime = i_time;
  m_lastInactivTimeAlert = -1;
}

int NetSClient::lastInactivTimeAlert() const {
  return m_lastInactivTimeAlert;
}

void NetSClient::setLastInactivTimeAlert(int i_time) {
  m_lastInactivTimeAlert = i_time;
}

void NetSClient::markToPlay(ServerRules *i_rules, bool i_value) {
  // do a rule action only if the value changes
  if (i_value != m_isMarkedToPlay) {
    try {
      if (i_value) {
        i_rules->scriptCallVoidNumberArg("Global_whenPlayer_added", id());
      } else {
        i_rules->scriptCallVoidNumberArg("Global_whenPlayer_removed", id());
      }
    } catch (Exception &e) {
      // continue the game even if the rules are badly written
      LogError(std::string("Rules: " + e.getMsg()).c_str());
    }
  }

  m_isMarkedToPlay = i_value;
}

bool NetSClient::isMarkedToPlay() {
  return m_isMarkedToPlay;
}

void NetSClient::markScenePlayer(unsigned int i_numScene,
                                 unsigned int i_numPlayer) {
  m_numScene = i_numScene;
  m_numPlayer = i_numPlayer;
  m_lastActivTime = GameApp::getXMTimeInt();
}

unsigned int NetSClient::getNumScene() const {
  return m_numScene;
}

unsigned int NetSClient::getNumPlayer() const {
  return m_numPlayer;
}

void NetSClient::setPlayingLevelId(const std::string &i_levelId) {
  m_playingLevelId = i_levelId;
}

std::string NetSClient::playingLevelId() const {
  return m_playingLevelId;
}

NetPing *NetSClient::lastPing() {
  return &m_lastPing;
}

ServerThread::ServerThread(const std::string &i_dbKey,
                           int i_port,
                           const std::string &i_adminPassword)
  : XMThread(i_dbKey) {
  m_port = i_port;
  m_adminPassword = i_adminPassword;

  m_set = NULL;
  m_nextClientId = 0;
  m_udpPacket = SDLNet_AllocPacket(XM_SERVER_MAX_UDP_PACKET_SIZE);

  m_universe = NULL;
  m_DBuffer = new DBuffer();
  m_DBuffer->initOutput(XM_NET_MAX_EVENTS_SHOT_SIZE);
  m_sp2phase = SP2_PHASE_NONE;
  m_lastFrameTimeStamp = -1;
  m_frameLate = 0;
  m_currentFrame = 0;
  m_nFollowingUdp = 0;
  m_startTimeStr = GameApp::getTimeStamp();
  m_banner = XM_SERVER_DEFAULT_BANNER;
  m_acceptConnections = false;
  m_unmanagedActions = 0;
  m_rules = NULL;
  m_needToReloadRules = false;
  m_sceneHook = new XMServerSceneHooks(this);

  if (!m_udpPacket) {
    throw Exception("SDLNet_AllocPacket: " + std::string(SDLNet_GetError()));
  }
}

ServerThread::~ServerThread() {
  SDLNet_FreePacket(m_udpPacket);
  delete m_DBuffer;
  if (m_rules != NULL) {
    delete m_rules;
  }
  delete m_sceneHook;
}

int ServerThread::realThreadFunction() {
  IPaddress ip;
  int ssn;

  LogInfo("server: starting");

  // this is not really required here, but it will not start the server if an
  // error occurred
  // init rules
  try {
    reloadRules(XM_SERVER_DEFAULT_RULES);
  } catch (Exception &e) {
    LogError((e.getMsg() + "\n" + m_rules->getErrorMsg()).c_str());
    return 1;
  }

  LogInfo("server: ports %i UDP & TCP", m_port);

  /* Resolving the host using NULL make network interface to listen */
  if (SDLNet_ResolveHost(&ip, NULL, m_port) < 0) {
    LogError("server: SDLNet_ResolveHost: %s", SDLNet_GetError());
    return 1;
  }

  m_set = SDLNet_AllocSocketSet(XM_SERVER_NB_SOCKETS_MAX);
  if (!m_set) {
    LogError("server: SDLNet_AllocSocketSet: %s", SDLNet_GetError());
    return 1;
  }

  /* Open a connection with the IP provided (listen on the host's port) */
  LogInfo("server: open connexion");
  if ((m_tcpsd = SDLNet_TCP_Open(&ip)) == 0) {
    LogError("server: SDLNet_TCP_Open: %s", SDLNet_GetError());
    SDLNet_FreeSocketSet(m_set);
    return 1;
  }

  if ((m_udpsd = SDLNet_UDP_Open(m_port)) == 0) {
    LogError("server: SDLNet_UDP_Open: %s", SDLNet_GetError());
    SDLNet_FreeSocketSet(m_set);
    SDLNet_TCP_Close(m_tcpsd);
    return 1;
  }

  ssn = SDLNet_TCP_AddSocket(m_set, m_tcpsd);
  if (ssn == -1) {
    LogError("server: SDLNet_TCP_AddSocket: %s", SDLNet_GetError());
    SDLNet_FreeSocketSet(m_set);
    SDLNet_TCP_Close(m_tcpsd);
    SDLNet_UDP_Close(m_udpsd);
    return 1;
  }
  ssn = SDLNet_UDP_AddSocket(m_set, m_udpsd);
  if (ssn == -1) {
    LogError("server: SDLNet_UDP_AddSocket: %s", SDLNet_GetError());
    SDLNet_FreeSocketSet(m_set);
    SDLNet_TCP_Close(m_tcpsd);
    SDLNet_UDP_Close(m_udpsd);
    return 1;
  }

  m_acceptConnections = true;
  if (StateManager::exists()) {
    StateManager::instance()->sendAsynchronousMessage("SERVER_STATUS_CHANGED");
  }

  // SP2 phase initialisation
  SP2_setPhase(SP2_PHASE_WAIT_CLIENTS);

  // manage server
  while (m_askThreadToEnd == false) {
    try {
      run_loop();
    } catch (Exception &e) {
      LogWarning("Exception: %s", e.getMsg().c_str());
    }
  }

  // close the server
  close();

  return 0;
}

int ServerThread::port() const {
  return m_port;
}

void ServerThread::close() {
  unsigned int i;

  // close the server
  m_acceptConnections = false;

  // end the game
  SP2_setPhase(SP2_PHASE_NONE);

  // disconnection
  LogInfo("server: %i client(s) still connected", m_clients.size());

  // disconnect all clients
  i = 0;
  while (i < m_clients.size()) {
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

  if (StateManager::exists()) {
    StateManager::instance()->sendAsynchronousMessage("SERVER_STATUS_CHANGED");
  }
  LogInfo("server: ending normally");
}

std::string ServerThread::SP2_determineLevel() {
  char **v_result;
  unsigned int nrow;
  std::string v_id_level;

  // don't allow own levels (isToReload=1)
  v_result = m_pDb->readDB(
    "SELECT id_level "
    "FROM levels "
    "WHERE isToReload=0 AND isScripted=0 AND isPhysics=0 " // warning,
    // addforcetoplayer
    // event cannot be
    // over network game
    // cause player id is
    // serialized
    //"WHERE id_level='_iL00_' " // for tests
    "ORDER BY RANDOM() LIMIT 1;",
    nrow);
  if (nrow == 0) {
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
  m_lastPhysTime = GameApp::getXMTimeInt();

  v_id_level = SP2_determineLevel();
  NA_playingLevel v_napl(v_id_level);

  m_universe = new Universe();
  m_universe->initPlayServer();

  // set hooks for the scenes
  for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
    m_universe->getScenes()[i]->setHooks(m_sceneHook);
  }

  try {
    for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
      v_numPlayer = 0;
      m_universe->getScenes()[i]->loadLevel(m_pDb, v_id_level, true);
      if (m_universe->getScenes()[i]->getLevelSrc()->isXMotoTooOld()) {
        throw Exception("Level " + v_id_level + " is too old");
      }

      m_DBuffer->clear();
      m_universe->getScenes()[i]->prePlayLevel(m_DBuffer, true, true, false);
      SP2_sendSceneEvents(m_DBuffer);

      // add the bikers
      for (unsigned int j = 0; j < m_clients.size(); j++) {
        if (m_clients[j]->isMarkedToPlay()) {
          m_universe->getScenes()[i]->addPlayerLocalBiker(
            v_localNetId,
            m_universe->getScenes()[i]->getLevelSrc()->PlayerStart(),
            DD_RIGHT,
            Theme::instance(),
            Theme::instance()->getPlayerTheme(),
            GameApp::getColorFromPlayerNumber(v_numPlayer),
            GameApp::getUglyColorFromPlayerNumber(v_numPlayer),
            false);
          m_clients[j]->markScenePlayer(0, v_numPlayer);

          // mark the player as playing the level
          m_clients[j]->setPlayingLevelId(v_id_level);
          sendToAllClients(&v_napl, m_clients[j]->id(), 0, j);

          v_numPlayer++;
          v_localNetId++;
        }
      }
      m_universe->getScenes()[i]->playInitLevel();
    }
  } catch (Exception &e) {
    LogWarning("server: Unable to load level %s", v_id_level.c_str());
    throw Exception("Unable to load level " + v_id_level);
  }

  try {
    std::vector<int> v_players;
    for (unsigned int i = 0; i < m_clients.size(); i++) {
      if (m_clients[i]->isMarkedToPlay()) {
        v_players.push_back(m_clients[i]->id());
      }
    }

    NA_prepareToPlay na(v_id_level, v_players);
    sendToAllClientsMarkedToPlay(&na, -1, 0);
  } catch (Exception &e) {
    /* bad */
  }

  m_sceneStartTime = (GameApp::getXMTimeInt() / 10) + XM_SERVER_PREPLAYING_TIME;
  m_lastPrepareToGoAlert = -1;
}

void ServerThread::SP2_uninitPlaying() {
  try {
    m_rules->scriptCallVoid("Round_whenRound_ends");
  } catch (Exception &e) {
    // continue the game even if the rules are badly written
    LogError(std::string("Rules: " + e.getMsg()).c_str());
  }
  delete m_universe;
  m_universe = NULL;
}

void ServerThread::SP2_manageInactivity() {
  int v_inactivDiff;
  int v_prevTime;
  Biker *v_player;
  bool v_isOk;

  /* kill players not playing for a too long time */
  for (unsigned int i = 0; i < m_clients.size(); i++) {
    if (m_clients[i]->isMarkedToPlay()) {
      v_player = m_universe->getScenes()[m_clients[i]->getNumScene()]
                   ->Players()[m_clients[i]->getNumPlayer()];

      if (v_player->isDead() == false && v_player->isFinished() == false) {
        v_isOk = false;

        // check driving
        if (v_player->getControler() != NULL) {
          if (v_player->getControler()->isDriving()) {
            m_clients[i]->setLastActivTime(GameApp::getXMTimeInt());
            v_isOk = true;
          }
        }

        // check last move
        if (v_isOk == false) {
          v_inactivDiff =
            GameApp::getXMTimeInt() - m_clients[i]->lastActivTime();
          if (XM_SERVER_PLAYER_INACTIV_TIME_MAX * 10 < v_inactivDiff) {
            m_universe->getScenes()[m_clients[i]->getNumScene()]->killPlayer(
              m_clients[i]->getNumPlayer());
          } else {
            v_prevTime = XM_SERVER_PLAYER_INACTIV_TIME_MAX * 10 - v_inactivDiff;

            if (m_clients[i]->isMarkedToPlay() &&
                XM_SERVER_PLAYER_INACTIV_TIME_PREV * 10 > v_prevTime) {
              if (m_clients[i]->lastInactivTimeAlert() != v_prevTime / 1000) {
                m_clients[i]->setLastInactivTimeAlert(v_prevTime / 1000);

                NA_killAlert na(m_clients[i]->lastInactivTimeAlert() + 1);
                try {
                  sendToClient(&na, i, -1, 0);
                } catch (Exception &e) {
                  /* hehe, ok, no pb */
                }
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

  if (m_sceneStartTime <= GameApp::getXMTimeInt() / 10 &&
      m_lastPrepareToGoAlert < 0) {
    return false;
  }

  v_waitTime = (m_sceneStartTime - (GameApp::getXMTimeInt() / 10)) / 100;

  if (v_waitTime <
      0) { /* only the GO! has not been send and time is just under 0 */
    m_lastPrepareToGoAlert = -1;
    try {
      NA_prepareToGo na(0);
      sendToAllClientsMarkedToPlay(&na, -1, 0);
    } catch (Exception &e) {
      /* ok, not good */
    }
  } else {
    try {
      if (m_lastPrepareToGoAlert != v_waitTime) {
        NA_prepareToGo na(v_waitTime + 1);
        sendToAllClientsMarkedToPlay(&na, -1, 0);
        m_lastPrepareToGoAlert = v_waitTime;
      }
    } catch (Exception &e) {
      /* ok, not good */
    }
  }

  return true;
}

void ServerThread::SP2_sendSceneEvents(DBuffer *i_buffer) {
  try {
    if (i_buffer->isEmpty() == false) {
      NA_gameEvents na(i_buffer);
      sendToAllClientsMarkedToPlay(&na, -1, 0);
    }
  } catch (Exception &e) {
    /* ok, not good */
  }
}

void ServerThread::SP2_updateScenePlaying() {
  int nPhysSteps;
  Scene *v_scene;
  SerializedBikeState BikeState;
  bool v_updateDone = false;
  bool v_firstFrame = false;

  if (SP2_managePreplayTime() == false) {
    // manage the first time the update is done
    if (m_sp2_gameStarted == false) {
      m_sp2_gameStarted = true;

      try {
        m_rules->scriptCallVoid("Round_whenRound_begins");
      } catch (Exception &e) {
        // continue the game even if the rules are badly written
        LogError(std::string("Rules: " + e.getMsg()).c_str());
      }
    }

    SP2_manageInactivity();

    /* update the scene */
    m_DBuffer->clear();
    nPhysSteps = 0;

    while (m_lastPhysTime + (PHYS_STEP_SIZE * 10) <= GameApp::getXMTimeInt() &&
           nPhysSteps < 10) {
      for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
        v_scene = m_universe->getScenes()[i];
        v_scene->updateLevel(PHYS_STEP_SIZE,
                             NULL,
                             m_DBuffer,
                             nPhysSteps != 0,
                             false /* no particles */,
                             false /* don't update died players */);
      }
      v_updateDone = true;
      m_lastPhysTime += PHYS_STEP_SIZE * 10;
      nPhysSteps++;
    }

    // if the delay is too long, reinitialize -- don't skip in server mode
    // if(m_fLastPhysTime + PHYS_STEP_SIZE/100.0 < GameApp::getXMTime()) {
    //  m_fLastPhysTime = GameApp::getXMTime();
    //}
    SP2_sendSceneEvents(m_DBuffer);
  } else {
    /* send the first frame regularly, so that the client received it once ready
     */
    if (GameApp::getXMTimeInt() - m_firstFrameSent >
        100) { /* 100 => 10 times / seconde */
      m_firstFrameSent = GameApp::getXMTimeInt();
      v_firstFrame = true;
    }
    // initialize the physics time
    m_lastPhysTime = GameApp::getXMTimeInt();
  }

  // send to each client his frame and the frame of the others
  if (v_updateDone || v_firstFrame) {
    if (v_firstFrame ||
        (m_currentFrame % (100 / XM_SERVER_UPLOADING_FPS_PLAYER) == 0 ||
         m_currentFrame % (100 / XM_SERVER_UPLOADING_FPS_OPLAYERS) == 0)) {
      for (unsigned int i = 0; i < m_clients.size(); i++) {
        if (m_clients[i]->isMarkedToPlay()) {
          v_scene = m_universe->getScenes()[m_clients[i]->getNumScene()];

          if (v_scene->Players()[m_clients[i]->getNumPlayer()]->isDead() ==
              false) {
            v_scene->getSerializedBikeState(
              v_scene->Players()[m_clients[i]->getNumPlayer()]->getState(),
              v_scene->getTime(),
              &BikeState,
              v_scene->getPhysicsSettings());
            NA_frame na(&BikeState);
            try {
              if (v_firstFrame ||
                  m_currentFrame % (100 / XM_SERVER_UPLOADING_FPS_PLAYER) ==
                    0) {
                sendToClient(&na, i, -1, 0);
              }
              if (v_firstFrame ||
                  m_currentFrame % (100 / XM_SERVER_UPLOADING_FPS_OPLAYERS) ==
                    0) {
                sendToAllClientsMarkedToPlay(&na, m_clients[i]->id(), 0, i);
              }
            } catch (Exception &e) {
            }
          }
        }
      }
    }
    m_currentFrame = (m_currentFrame + 1) % 1000;
  }
}

void ServerThread::SP2_updateCheckScenePlaying() {
  Scene *v_scene;
  bool v_nobodyPlaying = true;

  for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
    v_scene = m_universe->getScenes()[i];
    for (unsigned int j = 0; j < v_scene->Players().size(); j++) {
      if (v_scene->Players()[j]->isDead() == false &&
          v_scene->Players()[j]->isFinished() == false) {
        v_nobodyPlaying = false;
      }
    }
  }

  if (v_nobodyPlaying) {
    SP2_setPhase(SP2_PHASE_WAIT_CLIENTS);
  }
}

void ServerThread::SP2_setPhase(ServerP2Phase i_sp2phase) {
  SP2_unsetPhase();
  m_sp2phase = i_sp2phase;

  switch (m_sp2phase) {
    case SP2_PHASE_NONE:
      break;

    case SP2_PHASE_WAIT_CLIENTS:
      // load rules if they changed
      try {
        if (m_needToReloadRules) {
          m_needToReloadRules = false;
          reloadRules(XM_SERVER_DEFAULT_RULES);
        }
      } catch (Exception &e) {
        LogError((e.getMsg() + "\n" + m_rules->getErrorMsg()).c_str());
      }
      break;

    case SP2_PHASE_PLAYING:
      m_firstFrameSent = GameApp::getXMTimeInt();
      m_sp2_gameStarted = false;
      m_sp2_lastLoopTime = -1;
      try {
        SP2_initPlaying();
      } catch (Exception &e) {
        LogWarning("Unable to init playing (%s)", e.getMsg().c_str());
        SP2_setPhase(SP2_PHASE_WAIT_CLIENTS);
      }
      break;
  }
}

void ServerThread::SP2_unsetPhase() {
  switch (m_sp2phase) {
    case SP2_PHASE_NONE:
      break;

    case SP2_PHASE_WAIT_CLIENTS:
      break;

    case SP2_PHASE_PLAYING:
      SP2_uninitPlaying();

      /* update scores */
      sendPointsToSlavePlayers();
      break;
  }
}

unsigned int ServerThread::nbClientsInMode(NetClientMode i_mode) {
  unsigned int n = 0;

  for (unsigned int i = 0; i < m_clients.size(); i++) {
    if (m_clients[i]->mode() == i_mode) {
      n++;
    }
  }

  return n;
}

void ServerThread::run_loop() {
  switch (m_sp2phase) {
    case SP2_PHASE_NONE:
      manageNetwork(-1); // wait a network event
      break;

    case SP2_PHASE_WAIT_CLIENTS:
      if (nbClientsInMode(NETCLIENT_SLAVE_MODE) > 0) {
        // mark clients as to play
        for (unsigned int i = 0; i < m_clients.size(); i++) {
          if (m_clients[i]->mode() == NETCLIENT_SLAVE_MODE) {
            m_clients[i]->markToPlay(m_rules, true);
          }
        }
        SP2_setPhase(SP2_PHASE_PLAYING);
      } else {
        manageNetwork(-1); // wait a network event
      }
      break;

    case SP2_PHASE_PLAYING:
      // update the delta time to the reality
      if (m_sp2_lastLoopTime == -1) {
        m_sp2_lastLoopDelta = 0; // initialize the delta
      } else {
        m_sp2_lastLoopDelta += (GameApp::getXMTimeInt() - m_sp2_lastLoopTime) -
                               10; // update the delta
      }
      // too much delta, reset the delta
      if (m_sp2_lastLoopDelta > 100 || m_sp2_lastLoopDelta < -100) {
        m_sp2_lastLoopDelta = 0;
      }
      m_sp2_lastLoopTime = GameApp::getXMTimeInt();

      SP2_updateScenePlaying();
      SP2_updateCheckScenePlaying();

      // mange the network according to time spent
      // what is the remaing time on the 0.01s allowed
      int v_remainingTime = 10 -
                            (GameApp::getXMTimeInt() - m_sp2_lastLoopTime) -
                            m_sp2_lastLoopDelta;
      if (v_remainingTime > 0) {
        manageNetwork(v_remainingTime);
      } else {
        manageNetwork(0); // do at least one loop
      }
      break;
  }
}

/* 0 for no timeout, -1 for the maximum */
void ServerThread::manageNetwork(int i_timeout) {
  int v_start;
  int v_timeout_remaining;

  v_start = GameApp::getXMTimeInt();
  v_timeout_remaining = i_timeout;

  do { // do at least one time, even for i_timeout = 0
    manageNetworkOnePacket(v_timeout_remaining);
    v_timeout_remaining =
      i_timeout - (GameApp::getXMTimeInt() - v_start /* = time spent */);
  } while (v_timeout_remaining > 0);
}

bool ServerThread::manageNetworkOnePacket(int i_timeout) {
  int n_activ;
  unsigned int i;
  bool v_needMore = false;

  n_activ = SDLNet_CheckSockets(m_set, i_timeout);
  if (n_activ == -1) {
    LogError("SDLNet_CheckSockets: %s", SDLNet_GetError());
    m_askThreadToEnd = true;
  } else {
    if (n_activ == 0) {
      m_nFollowingUdp = 0;
    } else {
      v_needMore = true;

      // server socket
      if (SDLNet_SocketReady(m_tcpsd)) {
        acceptClient();
      }

      /*
        avoid udp only management ; by the way, avoid (check tcp at least every
        MAX_FOLLOWING_UDP)
        but don't check at each time tcp packets while it's less a  priority and
        less often
       */
      if (m_nFollowingUdp < XM_SERVER_MAX_FOLLOWING_UDP &&
          SDLNet_SocketReady(m_udpsd)) {
        manageClientUDP();
        m_nFollowingUdp++;
      } else {
        m_nFollowingUdp = 0;

        i = 0;
        while (i < m_clients.size()) {
          if (SDLNet_SocketReady(*(m_clients[i]->tcpSocket()))) {
            try {
              if (manageClientTCP(i) == false) {
                removeClient(i);
              } else {
                i++;
              }
            } catch (Exception &e) {
              if (e.getMsg() == "Disconnected") { // catch(DisconnectedException
                // &e) won't work, i don't
                // understand why
                LogInfo("server: client %u disconnected (%s:%d) : %s",
                        i,
                        XMNet::getIp(m_clients[i]->tcpRemoteIP()).c_str(),
                        SDLNet_Read16(&(m_clients[i]->tcpRemoteIP())->port),
                        e.getMsg().c_str());
                removeClient(i);
              } else {
                LogInfo(
                  "server: bad TCP packet received by client %u (%s:%d) : %s",
                  i,
                  XMNet::getIp(m_clients[i]->tcpRemoteIP()).c_str(),
                  SDLNet_Read16(&(m_clients[i]->tcpRemoteIP())->port),
                  e.getMsg().c_str());
                removeClient(i);
              }
            }
          } else {
            i++;
          }
        }
      }
    }
  }

  // remove client marked to be removed
  cleanClientsMarkedToBeRemoved();

  return v_needMore;
}

void ServerThread::removeClient(unsigned int i) {
  // remove the client from the scene if he is playing
  if (m_universe != NULL) {
    if (m_clients[i]->isMarkedToPlay()) {
      m_universe->getScenes()[m_clients[i]->getNumScene()]->killPlayer(
        m_clients[i]->getNumPlayer());
    }
  }

  SDLNet_TCP_DelSocket(m_set, *(m_clients[i]->tcpSocket()));
  SDLNet_TCP_Close(*(m_clients[i]->tcpSocket()));
  if (m_clients[i]->isUdpBinded()) {
    m_clients[i]->unbindUdp();
  }

  // trigger the rules actions by removing the player of the sp2
  m_clients[i]->markToPlay(m_rules, false);

  // send new client to other clients
  NA_changeClients nacc;
  NetInfosClient nic;
  try {
    nic.NetId = m_clients[i]->id();
    nic.Name = m_clients[i]->name();
    nacc.remove(&nic);
    sendToAllClients(&nacc, -1, 0, i);
  } catch (Exception &e) {
  }

  delete m_clients[i];
  m_clients.erase(m_clients.begin() + i);
}

void ServerThread::sendToClient(NetAction *i_netAction,
                                unsigned int i,
                                int i_src,
                                int i_subsrc,
                                bool i_forceUdp) {
  i_netAction->setSource(i_src, i_subsrc);
  if (i_forceUdp) {
    i_netAction->send(NULL, &m_udpsd, m_udpPacket, m_clients[i]->udpRemoteIP());
  } else if (m_clients[i]->isUdpBinded() &&
             m_clients[i]->isUdpBindedValidated()) {
    i_netAction->send(m_clients[i]->tcpSocket(),
                      &m_udpsd,
                      m_udpPacket,
                      m_clients[i]->udpRemoteIP());
  } else {
    i_netAction->send(m_clients[i]->tcpSocket(), NULL, NULL, NULL);
  }
}

void ServerThread::sendToAllClientsHavingMode(NetClientMode i_mode,
                                              NetAction *i_netAction,
                                              int i_src,
                                              int i_subsrc,
                                              int i_except) {
  for (unsigned int i = 0; i < m_clients.size(); i++) {
    if ((int)i != i_except &&
        (i_mode == NETCLIENT_ANY_MODE || i_mode == m_clients[i]->mode())) {
      try {
        sendToClient(i_netAction, i, i_src, i_subsrc);
      } catch (Exception &e) {
        // don't remove the client while removeclient function can call
        // sendToAllClients ...
      }
    }
  }
}

void ServerThread::sendToAllClients(NetAction *i_netAction,
                                    int i_src,
                                    int i_subsrc,
                                    int i_except) {
  sendToAllClientsHavingMode(
    NETCLIENT_ANY_MODE, i_netAction, i_src, i_subsrc, i_except);
}

void ServerThread::sendToAllClientsMarkedToPlay(NetAction *i_netAction,
                                                int i_src,
                                                int i_subsrc,
                                                int i_except) {
  for (unsigned int i = 0; i < m_clients.size(); i++) {
    if ((int)i != i_except && m_clients[i]->isMarkedToPlay()) {
      try {
        sendToClient(i_netAction, i, i_src, i_subsrc);
      } catch (Exception &e) {
        // don't remove the client while removeclient function can call
        // sendToAllClients ...
      }
    }
  }
}

void ServerThread::sendToAllClientsHavingProtocol(int i_protocol,
                                                  NetAction *i_netAction_lt,
                                                  NetAction *i_netAction_ge,
                                                  int i_src,
                                                  int i_subsrc,
                                                  int i_except) {
  for (unsigned int i = 0; i < m_clients.size(); i++) {
    try {
      if ((int)i != i_except) {
        if (m_clients[i]->protocolVersion() < i_protocol) {
          if (i_netAction_lt != NULL) {
            sendToClient(i_netAction_lt, i, i_src, i_subsrc);
          }
        } else {
          if (i_netAction_ge != NULL) {
            sendToClient(i_netAction_ge, i, i_src, i_subsrc);
          }
        }
      }
    } catch (Exception &e) {
      // don't remove the client while removeclient function can call
      // sendToAllClients ...
    }
  }
}

void ServerThread::sendToAllClientsHavingModeAndProtocol(
  NetClientMode i_mode,
  int i_protocol,
  NetAction *i_netAction_lt,
  NetAction *i_netAction_ge,
  int i_src,
  int i_subsrc,
  int i_except) {
  for (unsigned int i = 0; i < m_clients.size(); i++) {
    if ((int)i != i_except &&
        (i_mode == NETCLIENT_ANY_MODE || i_mode == m_clients[i]->mode())) {
      try {
        if ((int)i != i_except) {
          if (m_clients[i]->protocolVersion() < i_protocol) {
            if (i_netAction_lt != NULL) {
              sendToClient(i_netAction_lt, i, i_src, i_subsrc);
            }
          } else {
            if (i_netAction_ge != NULL) {
              sendToClient(i_netAction_ge, i, i_src, i_subsrc);
            }
          }
        }
      } catch (Exception &e) {
        // don't remove the client while removeclient function can call
        // sendToAllClients ...
      }
    }
  }
}

void ServerThread::sendPointsToSlavePlayers() {
  for (unsigned int i = 0; i < m_clients.size(); i++) {
    if (m_clients[i]->mode() == NETCLIENT_SLAVE_MODE) {
      sendPointsToClient(i);
    }
  }
}

void ServerThread::sendPointsToClient(unsigned int i_client) {
  NA_slaveClientsPoints nascp;
  NetPointsClient npc;
  try {
    for (unsigned int i = 0; i < m_clients.size(); i++) {
      if (m_clients[i]->mode() == NETCLIENT_SLAVE_MODE) {
        if (i == i_client) {
          npc.NetId = -1;
        } else {
          npc.NetId = m_clients[i]->id();
        }
        npc.Points = m_clients[i]->points();
        nascp.add(&npc);
      }
    }
    sendToClient(&nascp, i_client, -1, 0);
  } catch (Exception &e) {
  }
}

void ServerThread::acceptClient() {
  TCPsocket csd;
  IPaddress *tcpRemoteIP;
  int scn;

  if ((csd = SDLNet_TCP_Accept(m_tcpsd)) == 0) {
    return;
  }

  /* Get the remote address */
  if ((tcpRemoteIP = SDLNet_TCP_GetPeerAddress(csd)) == NULL) {
    LogWarning("server: SDLNet_TCP_GetPeerAddress: %s", SDLNet_GetError());
    SDLNet_TCP_Close(csd);
    return;
  }

  // check bans - don't go over this code, while this is the only way to become
  // a client (new NetSClient)
  if (m_pDb->srv_isBanned("", XMNet::getIp(tcpRemoteIP))) {
    LogInfo("server: banned client rejected (%s)",
            (XMNet::getIp(tcpRemoteIP)).c_str());
    NA_serverError na("Connexion refused");
    na.setSource(-1, 0);
    try {
      na.send(&csd, NULL, NULL, NULL);
    } catch (Exception &e) {
    }
    SDLNet_TCP_Close(csd);
    return;
  }

  /* Print the address, converting in the host format */
  LogInfo("server: host connected: %s:%d (TCP)",
          XMNet::getIp(tcpRemoteIP).c_str(),
          SDLNet_Read16(&tcpRemoteIP->port));

  // to much clients ?
  if (m_clients.size() >= XMSession::instance()->serverMaxClients()) {
    NA_serverError na(UNTRANSLATED_GAMETEXT_TOO_MUCH_CLIENTS);
    na.setSource(-1, 0);
    try {
      na.send(&csd, NULL, NULL, NULL);
    } catch (Exception &e) {
    }

    SDLNet_TCP_Close(csd);
    return;
  }

  scn = SDLNet_TCP_AddSocket(m_set, csd);
  if (scn == -1) {
    LogError("server: SDLNet_TCP_AddSocket: %s", SDLNet_GetError());
    SDLNet_TCP_Close(csd);
    return;
  }

  m_clients.push_back(new NetSClient(m_nextClientId++, csd, tcpRemoteIP));
}

bool ServerThread::manageClientTCP(unsigned int i) {
  try {
    while (m_clients[i]->tcpReader->TCPReadAction(m_clients[i]->tcpSocket(),
                                                  &m_preAllocatedNA)) {
      if (manageAction(m_preAllocatedNA.master, i) == false) {
        return false;
      }
    }
  } catch (Exception &e) {
    m_unmanagedActions++;
    throw e;
  }

  return true;
}

void ServerThread::manageClientUDP() {
  bool v_managedPacket;

  if (SDLNet_UDP_Recv(m_udpsd, m_udpPacket) == 1) {
    v_managedPacket = false;
    for (unsigned int i = 0; i < m_clients.size(); i++) {
      if (m_clients[i]->udpRemoteIP()->host == m_udpPacket->address.host &&
          m_clients[i]->udpRemoteIP()->port == m_udpPacket->address.port) {
        v_managedPacket = true;
        try {
          ActionReader::UDPReadAction(
            m_udpPacket->data, m_udpPacket->len, &m_preAllocatedNA);
          if (manageAction(m_preAllocatedNA.master, i) == false) {
            removeClient(i);
            return;
          }
        } catch (Exception &e) {
          m_unmanagedActions++;

          // ok, a bad packet received, forget it
          LogWarning(
            "server: bad UDP packet received by client %u (%s:%i) : %s",
            i,
            XMNet::getIp(&(m_udpPacket->address)).c_str(),
            SDLNet_Read16(&(m_udpPacket->address.port)),
            e.getMsg().c_str());
        }
        break; // stop : only one client
      }
    }

    // anonym packet ? find the associated client
    if (v_managedPacket == false) {
      try {
        ActionReader::UDPReadAction(
          m_udpPacket->data, m_udpPacket->len, &m_preAllocatedNA);
        if (m_preAllocatedNA.master->actionType() == TNA_udpBind) {
          for (unsigned int i = 0; i < m_clients.size(); i++) {
            if (m_clients[i]->isUdpBinded() == false) {
              if (m_clients[i]->udpBindKey() ==
                  ((NA_udpBind *)m_preAllocatedNA.master)->key()) {
                // LogInfo("UDP bind key received via UDP: %s",
                // ((NA_udpBind*)v_netAction)->key().c_str());
                m_clients[i]->bindUdp(m_udpPacket->address);
                if (m_clients[i]->protocolVersion() >=
                    3) { // don't send if the version is lower because the
                  // client will not understand -- udp could not work in
                  // that case
                  LogInfo("server: i can receive udp from the client %i", i);

                  NA_udpBindValidation nabv;
                  try {
                    sendToClient(&nabv, i, -1, 0);
                  } catch (Exception &e) {
                  }

                  NA_udpBind nab("XMS");
                  try {
                    // send the packet 3 times to get more change it arrives
                    for (unsigned int j = 0; j < 3; j++) {
                      sendToClient(&nab, i, -1, 0, true);
                    }
                  } catch (Exception &e) {
                  }
                }
                break; // stop : only one client
              }
            }
          }
        } else {
          LogWarning("Packet of unknown client received");
        }
      } catch (Exception &e) {
        m_unmanagedActions++;

        /* forget this bad packet */
        LogWarning("server: bad anonym UDP packet received by %s:%i",
                   XMNet::getIp(&(m_udpPacket->address)).c_str(),
                   SDLNet_Read16(&(m_udpPacket->address.port)));
      }
    }
  }
}

bool ServerThread::manageAction(NetAction *i_netAction, unsigned int i_client) {
  Scene *v_scene;
  unsigned int v_numPlayer;

  switch (i_netAction->actionType()) {
    case TNA_udpBind:
      /* managed before */
      break;

    case TNA_udpBindQuery:
    case TNA_serverError:
    case TNA_changeClients:
    case TNA_slaveClientsPoints:
    case TNA_clientsNumber:
    case TNA_prepareToPlay:
    case TNA_prepareToGo:
    case TNA_killAlert:
    case TNA_gameEvents:
    case TNA_srvCmdAsw: {
      /* should not be received */
      throw Exception("");
    } break;

    case TNA_clientsNumberQuery: {
      // only clients having a name
      int ntcp = 0;
      int nudp = 0;
      int nghosts = 0;
      int nslaves = 0;

      for (unsigned int i = 0; i < m_clients.size(); i++) {
        if (m_clients[i]->name() != "") {
          if (m_clients[i]->isUdpBindedValidated()) {
            nudp++;
          } else {
            ntcp++;
          }

          switch (m_clients[i]->mode()) {
            case NETCLIENT_GHOST_MODE:
              nghosts++;
              break;
            case NETCLIENT_SLAVE_MODE:
              nslaves++;
              break;
            case NETCLIENT_ANY_MODE:
              /* not a mode */
              break;
          }
        }
      }

      NA_clientsNumber nacn(ntcp, nudp, nghosts, nslaves);
      try {
        sendToClient(&nacn, i_client, -1, 0);
      } catch (Exception &e) {
      }
    } break;

    case TNA_clientInfos: {
      // check protocol version
      if (((NA_clientInfos *)i_netAction)->protocolVersion() >
          XM_NET_PROTOCOL_VERSION) {
        NA_serverError na(
          UNTRANSLATED_GAMETEXT_SERVER_PROTOCOL_VERSION_INCOMPATIBLE);
        na.setSource(-1, 0);
        try {
          sendToClient(&na, i_client, -1, 0);
        } catch (Exception &e) {
        }
        throw Exception("Protocol version incompatible");
      }

      // udpBindKey received
      // LogInfo("Protocol version of client %i is %i", i_client,
      // ((NA_clientInfos*)i_netAction)->protocolVersion());
      // LogInfo("UDP bind key of client %i is %s", i_client,
      // ((NA_clientInfos*)i_netAction)->udpBindKey().c_str());
      m_clients[i_client]->setUdpBindKey(
        ((NA_clientInfos *)i_netAction)->udpBindKey());
      m_clients[i_client]->setProtocolVersion(
        ((NA_clientInfos *)i_netAction)->protocolVersion());
      m_clients[i_client]->setXmVersion(
        ((NA_clientInfos *)i_netAction)->xmversion());

      // query bind udp
      NA_udpBindQuery naq;
      try {
        sendToClient(&naq, i_client, -1, 0);
      } catch (Exception &e) {
      }

      // send the current clients list to the client only when the client has a
      // name (or TNA_clientsNumber)
      NA_changeClients nacc;
      NetInfosClient nic;
      try {
        for (unsigned int i = 0; i < m_clients.size(); i++) {
          if (m_clients[i]->name() != "") { // remove still unset clients
            nic.NetId = m_clients[i]->id();
            nic.Name = m_clients[i]->name();
            nacc.add(&nic);
          }
        }
        sendToClient(&nacc, i_client, -1, 0);
      } catch (Exception &e) {
      }
    } break;

    case TNA_udpBindValidation: {
      if (m_clients[i_client]->isUdpBindedValidated() == false) {
        LogInfo("server: the client %i can receive udp from me", i_client);
        m_clients[i_client]->validUdpBind();
      }
    } break;

    case TNA_chatMessage: /* old message format, convert into the new one */
    {
      // retrieve message
      std::string v_msg = ((NA_chatMessage *)i_netAction)->getMessage();
      std::vector<int> v_pp;
      NA_chatMessagePP napp(v_msg, "", v_pp);
      sendToAllClientsHavingProtocol(4,
                                     i_netAction,
                                     &napp,
                                     m_clients[i_client]->id(),
                                     i_netAction->getSubSource(),
                                     i_client);
    } break;

    case TNA_chatMessagePP: {
      NA_chatMessagePP *na = ((NA_chatMessagePP *)i_netAction);
      NA_chatMessage na_old =
        NA_chatMessage(((NA_chatMessagePP *)i_netAction)->getMessage(), "");

      if (na->privatePeople().size() <=
          XM_SERVER_MAXIMUM_MULTI_PRIVATE_MESSAGE) {
        if (na->privatePeople().size() == 0) {
          // public
          sendToAllClientsHavingProtocol(4,
                                         &na_old,
                                         na,
                                         m_clients[i_client]->id(),
                                         i_netAction->getSubSource(),
                                         i_client);
        } else {
          // private
          for (unsigned int i = 0; i < na->privatePeople().size(); i++) {
            for (unsigned int j = 0; j < m_clients.size(); j++) {
              if (((int)m_clients[j]->id()) == na->privatePeople()[i]) {
                if (m_clients[j]->protocolVersion() < 4) {
                  sendToClient(&na_old,
                               j,
                               m_clients[i_client]->id(),
                               i_netAction->getSubSource());
                } else {
                  sendToClient(na,
                               j,
                               m_clients[i_client]->id(),
                               i_netAction->getSubSource());
                }
              }
            }
          }
        }
      } else {
        std::ostringstream v_str;
        v_str << "Private messages to maximum "
              << XM_SERVER_MAXIMUM_MULTI_PRIVATE_MESSAGE
              << " people are allowed";
        try {
          sendMsgToClient(i_client, v_str.str());
        } catch (Exception &e) {
        }
      }
    } break;

    case TNA_frame: {
      /* clients are limited in number of frame send to avoid too much frames */
      if (GameApp::getXMTimeInt() - m_clients[i_client]->lastGhostFrameTime() >
          1000 / XM_SERVER_UPLOADING_FPS_OPLAYERS) {
        for (unsigned int i = 0; i < m_clients.size(); i++) {
          if (m_clients[i_client]->mode() == NETCLIENT_GHOST_MODE) {
            if (i != i_client && m_clients[i_client]->playingLevelId() != "" &&
                m_clients[i_client]->playingLevelId() ==
                  m_clients[i]->playingLevelId()) {
              try {
                sendToClient(i_netAction,
                             i,
                             m_clients[i_client]->id(),
                             i_netAction->getSubSource());
              } catch (Exception &e) {
                // don't remove the client, it will be done in the main loop
              }
            }
          }
        }
        m_clients[i_client]->setLastGhostFrameTime(GameApp::getXMTimeInt());
      }
    } break;

    case TNA_changeName: {
      m_clients[i_client]->setName(((NA_changeName *)i_netAction)->getName());
      if (m_clients[i_client]->name() == "") {
        throw Exception("Invalid name provided");
      }

      // length
      if (utf8::utf8_length(m_clients[i_client]->name()) >
          XM_SERVER_NICK_LENGTH_MAX) {
        NA_serverError na("Connexion refused : player name too long");
        try {
          sendToClient(&na, i_client, -1, 0);
        } catch (Exception &e) {
          /* ok, no pb */
        }
        return false;
      }

      // check bans
      if (m_pDb->srv_isBanned(
            m_clients[i_client]->name(),
            XMNet::getIp(m_clients[i_client]->tcpRemoteIP()))) {
        LogInfo("server: banned client rejected (%s)",
                m_clients[i_client]->name().c_str());

        NA_serverError na("Connexion refused");
        try {
          sendToClient(&na, i_client, -1, 0);
        } catch (Exception &e) {
          /* ok, no pb */
        }

        return false;
      }

      LogInfo("Client[%i]'s name is \"%s\"",
              i_client,
              m_clients[i_client]->name().c_str());

      // send new client to other clients
      NA_changeClients nacc;
      NetInfosClient nic;
      try {
        nic.NetId = m_clients[i_client]->id();
        nic.Name = m_clients[i_client]->name();
        nacc.add(&nic);
        sendToAllClients(&nacc, -1, 0, i_client);
      } catch (Exception &e) {
      }

    } break;

    case TNA_playingLevel: {
      m_clients[i_client]->setPlayingLevelId(
        ((NA_playingLevel *)i_netAction)->getLevelId());
      sendToAllClients(i_netAction,
                       m_clients[i_client]->id(),
                       i_netAction->getSubSource(),
                       i_client);
    } break;

    case TNA_playerControl: {
      if (m_universe != NULL) {
        m_clients[i_client]->setLastActivTime(GameApp::getXMTimeInt());
        v_scene = m_universe->getScenes()[m_clients[i_client]->getNumScene()];
        v_numPlayer = m_clients[i_client]->getNumPlayer();

        // apply the control on the client
        switch (((NA_playerControl *)i_netAction)->getType()) {
          case PC_BRAKE:
            v_scene->Players()[v_numPlayer]->getControler()->setBreak(
              ((NA_playerControl *)i_netAction)->getFloatValue());
            break;
          case PC_THROTTLE:
            v_scene->Players()[v_numPlayer]->getControler()->setThrottle(
              ((NA_playerControl *)i_netAction)->getFloatValue());
            break;
          case PC_PULL:
            v_scene->Players()[v_numPlayer]->getControler()->setPull(
              ((NA_playerControl *)i_netAction)->getFloatValue());
            break;
          case PC_CHANGEDIR:
            v_scene->Players()[v_numPlayer]->getControler()->setChangeDir(
              ((NA_playerControl *)i_netAction)->getBoolValue());
            break;
        }
      }
    } break;

    case TNA_clientMode: {
      //
      if (((NA_clientMode *)i_netAction)->mode() == NETCLIENT_SLAVE_MODE) {
        // in this mode, the client must satisfy the minimum version allowed by
        // the server
        if (m_clients[i_client]->protocolVersion() <
            XM_SERVER_SLAVE_MODE_MIN_PROTOCOL_VERSION) {
          std::ostringstream v_str;
          v_str << "Protocol version "
                << XM_SERVER_SLAVE_MODE_MIN_PROTOCOL_VERSION
                << " is required on this mode.";
          v_str << " Your version is " << ((NA_clientMode *)i_netAction)->mode()
                << ".\n";
          v_str << " Update X-Moto or play simple ghost mode.";

          NA_serverError na(v_str.str());
          try {
            sendToClient(&na, i_client, -1, 0);
          } catch (Exception &e) {
            /* ok, no pb */
          }

          return false;
        }
      }

      m_clients[i_client]->setMode(((NA_clientMode *)i_netAction)->mode());

      // require that the mode of the player is set (the line before)
      if (((NA_clientMode *)i_netAction)->mode() == NETCLIENT_SLAVE_MODE) {
        // send current scores to the player
        if (m_clients[i_client]->protocolVersion() >= 5) {
          sendPointsToClient(i_client);
        }
      }
    } break;

    case TNA_srvCmd: {
      manageSrvCmd(i_client, ((NA_srvCmd *)i_netAction)->getCommand());
    } break;

    case TNA_ping: {
      if (((NA_ping *)i_netAction)->isPong()) {
        if (m_clients[i_client]->lastPing()->id ==
            ((NA_ping *)i_netAction)->id()) {
          // same id, update the rcv time
          m_clients[i_client]->lastPing()->pongTime = GameApp::getXMTimeInt();
        } else {
          // not the same last id, so, it means that an other ping has been
          // send. Ignore this pong.
        }
      } else { // receive a ping
        NA_ping na(
          ((NA_ping *)i_netAction)); // send the pong as answer to the ping
        try {
          sendToClient(&na, i_client, -1, 0);
        } catch (Exception &e) {
        }
      }
    } break;
  }

  return true;
}

void ServerThread::manageSrvCmd(unsigned int i_client,
                                const std::string &i_cmd) {
  std::string v_answer;
  std::vector<std::string> v_args;
  char **v_result;
  unsigned int nrow;

  utf8::utf8_split(i_cmd, " ", v_args);

  if (v_args.size() < 1) {
    v_answer = "";

  } else if (v_args[0] == "help") {
    if (v_args.size() != 1) {
      v_answer += "help: invalid arguments\n";
    } else {
      v_answer += "help: list commands\n";
      v_answer += "login [password]: connect on the server\n";
      v_answer += "logout: disconnect from the server\n";
      v_answer += "changepassword <password>: change your password\n";
      v_answer += "lsplayers: list connected players\n";
      v_answer += "lsscores: give scores of players in slave mode\n";
      v_answer += "lsbans: list banned players\n";

      std::ostringstream v_n;
      v_n << XM_SERVER_DEFAULT_BAN_NBDAYS;
      v_answer += "ban <id player> <ip|profile> [nbdays]: ban player <id "
                  "player> for nbdays (default is " +
                  v_n.str() + "), by ip or profile\n";
      v_answer += "unban <id ban>: remove a ban\n";
      v_answer += "lsadmins: list admins\n";
      v_answer +=
        "addadmin <id player> <password>: add player <id player> as admin\n";
      v_answer += "rmadmin <id admin>: remove admin\n";
      v_answer += "reloadrules: reload server rules\n";
      v_answer += "ping <all|id player>: information about player network "
                  "connection to the server\n";
      v_answer += "stats: server statistics\n";
      v_answer += "msg <msg>: message to players\n";
    }

  } else if (v_args[0] == "banner") {
    if (v_args.size() != 1) {
      v_answer += "banner: invalid arguments\n";
    } else {
      v_answer += m_banner + "\n";
      v_answer += "Type help to get more information";
    }

  } else if (v_args[0] == "login") {
    if (v_args.size() == 1) { // no arguments (local admins)
      if (XMNet::getIp(m_clients[i_client]->tcpRemoteIP()) == "127.0.0.1") {
        // local admins are allowed only when there is no other admin
        v_result = m_pDb->readDB("SELECT id, id_profile "
                                 "FROM srv_admins;",
                                 nrow);
        m_pDb->read_DB_free(v_result);
        if (nrow == 0) {
          m_clients[i_client]->setAdminConnected(true);
          v_answer += "Local admin\n";
          v_answer += "Connected\n";
        } else {
          v_answer +=
            "Local admins are allowed only when no other admin exists\n";
        }
      } else {
        v_answer +=
          "Only local admins are allowed to connect without password\n";
      }
    } else if (v_args.size() != 2) {
      v_answer += "login: invalid arguments\n";
    } else if (m_clients[i_client]->isAdminConnected()) {
      v_answer += "Already connected\n";
    } else {
      if ((m_adminPassword != "" &&
           m_adminPassword ==
             std::string(v_args[1])) || // master admin password
          (m_pDb->srv_isAdmin(m_clients[i_client]->name(),
                              v_args[1])) // normal admin password
      ) {
        m_clients[i_client]->setAdminConnected(true);
        v_answer += "Connected\n";
      } else {
        v_answer += "Invalid password\n";
      }
    }
  } else if (m_clients[i_client]->isAdminConnected() == false) {
    // don't allow to continue if the client is not connected
    v_answer += "You must login first\n";

  } else if (v_args[0] == "logout") {
    if (v_args.size() != 1) {
      v_answer += "logout: invalid arguments\n";
    } else {
      m_clients[i_client]->setAdminConnected(false);
      v_answer += "Disconnected";
    }

  } else if (v_args[0] == "changepassword") {
    if (v_args.size() != 2) {
      v_answer += "changepassword: invalid arguments\n";
    } else {
      m_pDb->srv_changePassword(m_clients[i_client]->name(), v_args[1]);
      v_answer += "Password changed";
    }

  } else if (v_args[0] == "lsadmins") {
    if (v_args.size() != 1) {
      v_answer += "lsadmins: invalid arguments\n";
    } else {
      char v_adminstr[20];

      v_result = m_pDb->readDB("SELECT id, id_profile "
                               "FROM srv_admins "
                               "ORDER BY id_profile;",
                               nrow);
      for (unsigned int i = 0; i < nrow; i++) {
        snprintf(v_adminstr,
                 20,
                 "%5s: %-12s",
                 m_pDb->getResult(v_result, 2, i, 0),
                 m_pDb->getResult(v_result, 2, i, 1));
        v_answer += v_adminstr;
        v_answer += "\n";
      }
      m_pDb->read_DB_free(v_result);
    }

  } else if (v_args[0] == "rmadmin") {
    if (v_args.size() != 2) {
      v_answer += "rmadmin: invalid arguments\n";
    } else {
      m_pDb->srv_removeAdmin(atoi(v_args[1].c_str()));
      v_answer += "admin removed\n";
    }

  } else if (v_args[0] == "addadmin") {
    if (v_args.size() != 3) {
      v_answer += "addadmin: invalid arguments\n";
    } else {
      try {
        unsigned int v_nclient = getClientById(atoi(v_args[1].c_str()));
        m_pDb->srv_addAdmin(m_clients[v_nclient]->name(), v_args[2]);
        v_answer += "admin added\n";
      } catch (Exception &e) {
        v_answer += "unable to add the admin\n";
        v_answer += e.getMsg() + "\n";
      }
    }

  } else if (v_args[0] == "lsplayers") {
    if (v_args.size() != 1) {
      v_answer += "lsplayers: invalid arguments\n";
    } else {
      char v_clientstr[94];
      std::string v_mode;

      v_answer +=
        "+----+-----------------+----+----------------+-----+----------"
        "--------------+-------+--------+\n";
      v_answer +=
        "|  id|login            |mode|              ip|proto|version   "
        "              |xmproto|ping(mS)|\n";
      v_answer +=
        "+----+-----------------+----+----------------+-----+----------"
        "--------------+-------+--------+\n";

      for (unsigned int i = 0; i < m_clients.size(); i++) {
        std::ostringstream v_ping;

        switch (m_clients[i]->mode()) {
          case NETCLIENT_GHOST_MODE:
            v_mode = "GHOST";
            break;
          case NETCLIENT_SLAVE_MODE:
            v_mode = "SLAVE";
            break;
          default:
            v_mode = "UNKWN";
        }
        if (m_clients[i]->lastPing()->pongTime == -1) {
          v_ping << "-";
        } else {
          v_ping << (m_clients[i]->lastPing()->pongTime -
                     m_clients[i]->lastPing()->pingTime);
        }

        snprintf(v_clientstr,
                 94,
                 "%5u %-17s %-5s %15s %5s %-26s %5i %8s",
                 m_clients[i]->id(),
                 m_clients[i]->name().c_str(),
                 v_mode.c_str(),
                 (XMNet::getIp(m_clients[i]->tcpRemoteIP())).c_str(),
                 m_clients[i]->isUdpBindedValidated() ? "UDP" : "TCP",
                 m_clients[i]->xmversion().c_str(),
                 m_clients[i]->protocolVersion(),
                 v_ping.str().c_str());

        v_answer += v_clientstr;
        v_answer += "\n";
      }
    }

  } else if (v_args[0] == "lsscores") {
    if (v_args.size() != 1) {
      v_answer += "lsscores: invalid arguments\n";
    } else {
      char v_clientstr[48];
      std::string v_mode;
      for (unsigned int i = 0; i < m_clients.size(); i++) {
        if (m_clients[i]->mode() == NETCLIENT_SLAVE_MODE) {
          snprintf(v_clientstr,
                   48,
                   "%5u: %-12s %6i",
                   m_clients[i]->id(),
                   m_clients[i]->name().c_str(),
                   m_clients[i]->points());
          v_answer += v_clientstr;
          v_answer += "\n";
        }
      }
    }

  } else if (v_args[0] == "lsbans") {
    if (v_args.size() != 1) {
      v_answer += "lsban: invalid arguments\n";
    } else {
      char v_banstr[100];

      // clean old bans before reading them
      m_pDb->srv_cleanBans();

      v_result =
        m_pDb->readDB("SELECT id, id_profile, ip, from_date, ROUND(nb_days - "
                      "(julianday('now')-julianday(from_date)), 1) remaining, "
                      "id_admin_banner "
                      "FROM srv_bans "
                      "ORDER BY id;",
                      nrow);

      v_answer += "+----+-----------------+----------------+-------------------"
                  "+----------------+-----------------+\n";
      v_answer += "|  id|login            |ip              | ban date          "
                  "| remaining days | banner          |\n";
      v_answer += "+----+-----------------+----------------+-------------------"
                  "+----------------+-----------------+\n";

      for (unsigned int i = 0; i < nrow; i++) {
        snprintf(v_banstr,
                 100,
                 "%5s %-17s %-16s %19s %16s %17s",
                 std::string(m_pDb->getResult(v_result, 6, i, 0)).c_str(),
                 std::string(m_pDb->getResult(v_result, 6, i, 1)).c_str(),
                 std::string(m_pDb->getResult(v_result, 6, i, 2)).c_str(),
                 std::string(m_pDb->getResult(v_result, 6, i, 3)).c_str(),
                 std::string(m_pDb->getResult(v_result, 6, i, 4)).c_str(),
                 (m_pDb->getResult(v_result, 6, i, 5) == NULL)
                   ? "Unknown"
                   : std::string(m_pDb->getResult(v_result, 6, i, 5)).c_str());
        v_answer += v_banstr;
        v_answer += "\n"; // cause 100 could not include the \n
      }
      m_pDb->read_DB_free(v_result);
    }

  } else if (v_args[0] == "ban") {
    if (v_args.size() != 3 && v_args.size() != 4) {
      v_answer += "ban: invalid arguments\n";
    } else if (v_args[2] != "ip" && v_args[2] != "profile") {
      v_answer += "ban: invalid arguments\n";
    } else {
      try {
        unsigned int v_nclient = getClientById(atoi(v_args[1].c_str()));
        m_pDb->srv_addBan(
          v_args[2] == "profile" ? m_clients[v_nclient]->name() : "*",
          v_args[2] == "ip" ? XMNet::getIp(m_clients[v_nclient]->tcpRemoteIP())
                            : "*",
          v_args.size() == 4 ? atoi(v_args[3].c_str())
                             : XM_SERVER_DEFAULT_BAN_NBDAYS,
          m_clients[i_client]->adminLoginName());
        m_clientMarkToBeRemoved.push_back(atoi(v_args[1].c_str()));
        v_answer += "ban added\n";
      } catch (Exception &e) {
        v_answer += "unable to add the ban\n";
        v_answer += e.getMsg() + "\n";
      }
    }

  } else if (v_args[0] == "unban") {
    if (v_args.size() != 2) {
      v_answer += "unban: invalid arguments\n";
    } else {
      m_pDb->srv_removeBan(atoi(v_args[1].c_str()));
      v_answer += "ban removed\n";
    }

  } else if (v_args[0] == "reloadrules") {
    if (v_args.size() != 1) {
      v_answer += "reloadrules: invalid arguments\n";
    } else {
      m_needToReloadRules = true;
      v_answer += "Rules will be reloaded just before the next round\n";
    }

  } else if (v_args[0] == "stats") {
    if (v_args.size() != 1) {
      v_answer += "stats: invalid arguments\n";
    } else {
      char v_line[256];
      v_answer += "start time : " + m_startTimeStr + "\n";

      //         | bytes | packets | biggest |
      // --------+-------+---------+---------|
      //  in TCP |
      //  in UDP |
      // --------+
      // out TCP |
      // out UDP |

      snprintf(v_line,
               256,
               "+%9s+%13s+%13s+%13s+\n",
               "---------",
               "-------------",
               "-------------",
               "-------------");
      v_answer += v_line;
      snprintf(v_line,
               256,
               "| %7s | %11s | %11s | %11s |\n",
               "       ",
               "   bytes   ",
               "  packets  ",
               "  biggest  ");
      v_answer += v_line;
      snprintf(v_line,
               256,
               "+%9s+%13s+%13s+%13s+\n",
               "---------",
               "-------------",
               "-------------",
               "-------------");
      v_answer += v_line;
      snprintf(
        v_line,
        256,
        "| %7s + %11s + %11i + %11s |\n",
        " in TCP",
        XMNet::getFancyBytes(ActionReader::m_TCPPacketsSizeReceived).c_str(),
        ActionReader::m_nbTCPPacketsReceived,
        XMNet::getFancyBytes(ActionReader::m_biggestTCPPacketReceived).c_str());
      v_answer += v_line;
      snprintf(
        v_line,
        256,
        "| %7s + %11s + %11i + %11s |\n",
        " in UDP",
        XMNet::getFancyBytes(ActionReader::m_UDPPacketsSizeReceived).c_str(),
        ActionReader::m_nbUDPPacketsReceived,
        XMNet::getFancyBytes(ActionReader::m_biggestUDPPacketReceived).c_str());
      v_answer += v_line;
      snprintf(v_line,
               256,
               "| %7s + %11s + %11i + %11s |\n",
               "out TCP",
               XMNet::getFancyBytes(NetAction::m_TCPPacketsSizeSent).c_str(),
               NetAction::m_nbTCPPacketsSent,
               XMNet::getFancyBytes(NetAction::m_biggestTCPPacketSent).c_str());
      v_answer += v_line;
      snprintf(v_line,
               256,
               "| %7s + %11s + %11i + %11s |\n",
               "out UDP",
               XMNet::getFancyBytes(NetAction::m_UDPPacketsSizeSent).c_str(),
               NetAction::m_nbUDPPacketsSent,
               XMNet::getFancyBytes(NetAction::m_biggestUDPPacketSent).c_str());
      v_answer += v_line;
      snprintf(v_line,
               256,
               "+%9s+%13s+%13s+%13s+\n",
               "---------",
               "-------------",
               "-------------",
               "-------------");
      v_answer += v_line;

      std::ostringstream v_nup;
      v_answer += "unmanaged packets : ";
      v_nup << m_unmanagedActions;
      v_answer += v_nup.str() + "\n";

      // db memory
      std::ostringstream v_ndb;
      v_answer += "db memory : ";
      v_ndb << (xmDatabase::getMemoryUsed()) / 1024 / 1024;
      v_answer += v_ndb.str() + "mB" + "\n";

      // system memory
      v_answer += System::getMemoryInfo();
    }

  } else if (v_args[0] == "msg") {
    if (v_args.size() < 2) {
      v_answer += "msg: invalid arguments\n";
    } else {
      std::string v_msg;
      for (unsigned int i = 1; i < v_args.size(); i++) {
        v_msg += v_args[i];
        if (i < v_args.size() - 1) {
          v_msg += " ";
        }
      }

      std::vector<int> v_private_people;
      NA_chatMessagePP na(v_msg.c_str(), "server", v_private_people);
      NA_chatMessage na_old(v_msg.c_str(), "server");
      try {
        sendToAllClientsHavingProtocol(4, &na_old, &na, -1, 0);
      } catch (Exception &e) {
      }
    }

  } else if (v_args[0] == "ping") {
    if (v_args.size() != 2) {
      v_answer += "ping: invalid arguments\n";
    } else {
      NA_ping na;
      int v_arg_client;

      if (v_args[1] == "all") {
        v_arg_client = -1;
      } else {
        v_arg_client = (int)atoi(v_args[1].c_str());
      }

      for (unsigned int i = 0; i < m_clients.size(); i++) {
        if (m_clients[i]->protocolVersion() >= 6) {
          if (m_clients[i]->id() == v_arg_client || v_arg_client == -1) {
            m_clients[i]->lastPing()->id = na.id();
            m_clients[i]->lastPing()->pingTime = GameApp::getXMTimeInt();
            m_clients[i]->lastPing()->pongTime = -1; // reset the pong time
            sendToClient(&na, i, -1, 0);
          }
        }
      }
    }

  } else {
    v_answer = "Unknown command : \"" + v_args[0] +
               "\"\nType help to get more information\n";
  }

  // send the answer
  NA_srvCmdAsw na(v_answer);
  try {
    sendToClient(&na, i_client, -1, 0);
  } catch (Exception &e) {
    /* ok, no pb retry with the error */
    try {
      NA_srvCmdAsw na("An error occurred: " + e.getMsg());
      sendToClient(&na, i_client, -1, 0);
    } catch (Exception &e2) {
      /* ok, no pb */
    }
  }
}

NetSClient *ServerThread::getNetSClientById(unsigned int i_id) const {
  return m_clients[getClientById(i_id)];
}

unsigned int ServerThread::getClientById(unsigned int i_id) const {
  for (unsigned int i = 0; i < m_clients.size(); i++) {
    if (m_clients[i]->id() == i_id) {
      return i;
    }
  }
  throw Exception("Client not found");
}

NetSClient *ServerThread::getNetSClientByScenePlayer(
  unsigned int i_numScene,
  unsigned int i_numPlayer) const {
  for (unsigned int i = 0; i < m_clients.size(); i++) {
    if (m_clients[i]->isMarkedToPlay()) {
      if (m_clients[i]->getNumScene() == i_numScene) {
        if (m_clients[i]->getNumPlayer() == i_numPlayer) {
          return m_clients[i];
        }
      }
    }
  }
  return NULL;
}

void ServerThread::cleanClientsMarkedToBeRemoved() {
  // main case : no client to remove
  if (m_clientMarkToBeRemoved.empty()) {
    return;
  }

  for (unsigned int i = 0; i < m_clientMarkToBeRemoved.size(); i++) {
    try {
      NA_serverError na("Connexion refused");
      try {
        sendToClient(&na, i, -1, 0);
      } catch (Exception &e) {
        /* ok, no pb */
      }
      removeClient(getClientById(m_clientMarkToBeRemoved[i]));
    } catch (Exception &e) {
      /* probably already disconnected */
    }
  }
  m_clientMarkToBeRemoved.clear();
}

bool ServerThread::acceptConnections() const {
  return m_acceptConnections;
}

void ServerThread::sendMsgToClient(unsigned int i_client,
                                   const std::string &i_msg) {
  std::vector<int> v_private_people;

  if (m_clients[i_client]->protocolVersion() < 4) {
    NA_chatMessage na(i_msg, "server");
    try {
      sendToClient(&na, i_client, -1, 0);
    } catch (Exception &e) {
    }
  } else {
    NA_chatMessagePP na(i_msg, "server", v_private_people);
    try {
      sendToClient(&na, i_client, -1, 0);
    } catch (Exception &e) {
    }
  }
}

ServerRules *ServerThread::getRules() {
  return m_rules;
}

Universe *ServerThread::getUniverse() {
  return m_universe;
}

void ServerThread::reloadRules(const std::string &i_rulesFile) {
  if (m_rules != NULL) {
    delete m_rules;
  }
  m_rules = new ServerRules(this);

  // init player points
  for (unsigned int i = 0; i < m_clients.size(); i++) {
    m_clients[i]->setPoints(0);
  }

  m_rules->loadScriptFile(i_rulesFile);
  m_rules->scriptCallVoid("Global_init");

  // simulate the Global_whenPlayer_added for players as the rule ignore these
  // players
  for (unsigned int i = 0; i < m_clients.size(); i++) {
    if (m_clients[i]->mode() == NETCLIENT_SLAVE_MODE) {
      m_rules->scriptCallVoidNumberArg("Global_whenPlayer_added",
                                       m_clients[i]->id());
    }
  }
}

XMServerSceneHooks::XMServerSceneHooks(ServerThread *i_st) {
  m_server = i_st;
}

XMServerSceneHooks::~XMServerSceneHooks() {}

void XMServerSceneHooks::OnEntityToTakeTakenByPlayer(unsigned int i_player) {
  // only one scene is managed on the server -- for the moment
  NetSClient *v_client = m_server->getNetSClientByScenePlayer(0, i_player);

  if (v_client != NULL) {
    try {
      m_server->getRules()->scriptCallVoidNumberArg(
        "Round_whenPlayer_onEntityToTakeTaken", v_client->id());
    } catch (Exception &e) {
      // continue the game even if the rules are badly written
      LogError(std::string("Rules: " + e.getMsg()).c_str());
    }
  }
}

void XMServerSceneHooks::OnEntityToTakeTakenExternal() {
  try {
    m_server->getRules()->scriptCallVoid(
      "Round_whenExternal_onEntityToTakeTaken");
  } catch (Exception &e) {
    // continue the game even if the rules are badly written
    LogError(std::string("Rules: " + e.getMsg()).c_str());
  }
}

void XMServerSceneHooks::OnPlayerWins(unsigned int i_player) {
  // only one scene is managed on the server -- for the moment
  NetSClient *v_client = m_server->getNetSClientByScenePlayer(0, i_player);

  if (v_client != NULL) {
    try {
      m_server->getRules()->scriptCallVoidNumberArg("Round_whenPlayer_wins",
                                                    v_client->id());
    } catch (Exception &e) {
      // continue the game even if the rules are badly written
      LogError(std::string("Rules: " + e.getMsg()).c_str());
    }
  }
}

void XMServerSceneHooks::OnPlayerDies(unsigned int i_player) {
  // only one scene is managed on the server -- for the moment
  NetSClient *v_client = m_server->getNetSClientByScenePlayer(0, i_player);

  if (v_client != NULL) {
    try {
      m_server->getRules()->scriptCallVoidNumberArg("Round_whenPlayer_dies",
                                                    v_client->id());
    } catch (Exception &e) {
      // continue the game even if the rules are badly written
      LogError(std::string("Rules: " + e.getMsg()).c_str());
    }
  }
}

void XMServerSceneHooks::OnPlayerSomersault(unsigned int i_player,
                                            bool i_counterclock) {
  // only one scene is managed on the server -- for the moment
  NetSClient *v_client = m_server->getNetSClientByScenePlayer(0, i_player);

  if (v_client != NULL) {
    try {
      m_server->getRules()->scriptCallVoidNumberArg(
        "Round_whenPlayer_DoesASomersault",
        v_client->id(),
        i_counterclock ? 1 : 0);
    } catch (Exception &e) {
      // continue the game even if the rules are badly written
      LogError(std::string("Rules: " + e.getMsg()).c_str());
    }
  }
}
