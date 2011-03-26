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

#include "NetClient.h"
#include "NetActions.h"
#include "thread/ClientListenerThread.h"
#include "../helpers/VExcept.h"
#include "../helpers/utf8.h"
#include "../helpers/Log.h"
#include "../Game.h"
#include "helpers/Net.h"
#include "../XMSession.h"
#include "../states/StateManager.h"
#include "../states/StatePreplayingNet.h"
#include <sstream>
#include "../helpers/VMath.h"
#include "../SysMessage.h"
#include "../Universe.h"
#include "../Theme.h"
#include "../xmscene/BikeGhost.h"
#include "../GameText.h"
#include "../DBuffer.h"
#include "../GameEvents.h"
#include "../db/xmDatabase.h"
#include "../xmscene/Camera.h"
#include "VirtualNetLevelsList.h"
#include "../Replay.h"

#define XMCLIENT_KILL_ALERT_DURATION 100
#define XMCLIENT_PREPARE_TO_PLAY_DURATION 100

NetClient::NetClient() {
    m_isConnected = false;
    m_serverReceivesUdp = false;
    m_serverSendsUdp    =  false;
    m_clientListenerThread = NULL;
    m_netActionsMutex = SDL_CreateMutex();
    m_universe = NULL;
    m_mode = NETCLIENT_GHOST_MODE;
    m_points = 0;
    m_udpSendPacket = SDLNet_AllocPacket(XM_CLIENT_MAX_UDP_PACKET_SIZE);

    if(!m_udpSendPacket) {
      throw Exception("SDLNet_AllocPacket: " + std::string(SDLNet_GetError()));
    }

    std::ostringstream v_rd;
    v_rd << randomIntNum(1, RAND_MAX);
    m_udpBindKey = v_rd.str();

    m_lastOwnFPS = 0;
    m_currentOwnFramesNb = 0;
    m_currentOwnFramesTime = GameApp::getXMTimeInt();

    m_otherClientsLevelsList = new VirtualNetLevelsList(this);

    // ping information
    m_lastPing.id = -1;
    m_lastPing.pingTime = -1;
    m_lastPing.pongTime = -1;
}

NetClient::~NetClient() {
  delete m_otherClientsLevelsList;

  SDL_DestroyMutex(m_netActionsMutex);

  for(unsigned int i=0; i<m_netActions.size(); i++) {
    delete m_netActions[i];
  }

  SDLNet_FreePacket(m_udpSendPacket);
}

std::string NetClient::udpBindKey() const {
  return m_udpBindKey;
}

UDPpacket* NetClient::sendPacket() {
  return m_udpSendPacket;
}

void NetClient::executeNetActions(xmDatabase* pDb) {
  if(m_netActions.size() == 0) {
    return; // try to not lock via mutex if not needed
  }

  SDL_LockMutex(m_netActionsMutex);
  for(unsigned int i=0; i<m_netActions.size(); i++) {
    //LogInfo("Execute NetAction (%s)", m_netActions[i]->actionKey().c_str());
    manageAction(pDb, m_netActions[i]);
    delete m_netActions[i];
  }
  m_netActions.clear();
  SDL_UnlockMutex(m_netActionsMutex);
}

void NetClient::addNetAction(NetAction* i_act) {
  SDL_LockMutex(m_netActionsMutex);
  m_netActions.push_back(i_act);
  SDL_UnlockMutex(m_netActionsMutex);
}

void NetClient::connect(const std::string& i_server, int i_port) {

  if(m_isConnected) {
    throw Exception("Already connected");
  }

  // reset udp server informations
  m_serverReceivesUdp = false;
  m_serverSendsUdp    =  false;

  if (SDLNet_ResolveHost(&serverIp, i_server.c_str(), i_port) < 0) {
    throw Exception(SDLNet_GetError());
  }

  if (!(m_tcpsd = SDLNet_TCP_Open(&serverIp))) {
    throw Exception(SDLNet_GetError());
  }

  if((m_udpsd = SDLNet_UDP_Open(0)) == 0) {
    LogError("server: SDLNet_UDP_Open: %s", SDLNet_GetError());
    SDLNet_TCP_Close(m_tcpsd);
    throw Exception(SDLNet_GetError());
  }
  m_udpSendPacket->address = serverIp;

  m_clientListenerThread = new ClientListenerThread(this);
  m_clientListenerThread->startThread();

  m_isConnected = true;

  LogInfo("client: connected on %s:%d", i_server.c_str(), i_port);

  char buf[512];
  snprintf(buf, 512, GAMETEXT_PRESSCTRLCTOCHAT, InputHandler::instance()->getGlobalKey(INPUT_CHAT)->toFancyString().c_str());
  SysMessage::instance()->addConsoleLine(buf, CLT_INFORMATION);

  // bind udp port on server
  NA_clientInfos na(XM_NET_PROTOCOL_VERSION, m_udpBindKey);
  try {
    NetClient::instance()->send(&na, 0);
  } catch(Exception &e) {
  }

  // changeName
  NA_changeName nap(XMSession::instance()->profile());
  try {
    NetClient::instance()->send(&nap, 0);
  } catch(Exception &e) {
  }
}

void NetClient::disconnect() {
  if(m_isConnected == false) {
    return;
  }

  if(m_clientListenerThread->isThreadRunning()) {
    m_clientListenerThread->askThreadToEnd();
    m_clientListenerThread->waitForThreadEnd();
  }
  delete m_clientListenerThread;

  LogInfo("client: disconnected")
  SDLNet_TCP_Close(m_tcpsd);
  SDLNet_UDP_Close(m_udpsd);

  for(unsigned int i=0; i<m_otherClients.size(); i++) {
    delete m_otherClients[i];
    m_otherClients.clear();
  }

  m_isConnected = false;
}

bool NetClient::isConnected() {
  // check that the client has not finished
  if(m_isConnected) {
    if(m_clientListenerThread->isThreadRunning() == false) {
      disconnect();
    }
  }
  return m_isConnected;
}

TCPsocket* NetClient::tcpSocket() {
  return &m_tcpsd;
}

UDPsocket* NetClient::udpSocket() {
  return &m_udpsd;
}

void NetClient::send(NetAction* i_netAction, int i_subsrc, bool i_forceUdp) {
  i_netAction->setSource(0, i_subsrc);

  try {
    if(i_forceUdp) {
      i_netAction->send(NULL, &m_udpsd, m_udpSendPacket, &m_udpSendPacket->address);
    } else if(m_serverReceivesUdp) {
      i_netAction->send(&m_tcpsd, &m_udpsd, m_udpSendPacket, &m_udpSendPacket->address);
    } else {
      i_netAction->send(&m_tcpsd, NULL, NULL, NULL);
    }
  } catch(Exception &e) {
    disconnect();
    LogWarning("send failed : %s", e.getMsg().c_str());
    throw e;
  }
}

void NetClient::startPlay(Universe* i_universe) {
  m_universe = i_universe;
}

bool NetClient::isPlayInitialized() {
  return m_universe != NULL;
}

void NetClient::endPlay() {
  for(unsigned int i=0; i<m_otherClients.size(); i++) {
    for(unsigned int j=0; j<NETACTION_MAX_SUBSRC; j++) {
      m_otherClients[i]->setNetGhost(j, NULL);
    }
  }
  m_universe = NULL;
}

void NetClient::changeMode(NetClientMode i_mode) {
  m_mode = i_mode;

  NA_clientMode na(i_mode);
  try {
    send(&na, 0);
  } catch(Exception &e) {
  }
}

NetClientMode NetClient::mode() const {
  return m_mode;
}

int NetClient::points() const {
  return m_points;
}

std::vector<NetOtherClient*>& NetClient::otherClients() {
  return m_otherClients;
}

unsigned int NetClient::getOtherClientNumberById(int i_id) const {
  for(unsigned int i=0; i<m_otherClients.size(); i++) {
    if(m_otherClients[i]->id() == i_id) {
      return i;
    }
  }
  throw Exception("Invalid id");
}

int NetClient::getOwnFrameFPS() const {
  return m_lastOwnFPS;
}

void NetClient::updateOtherClientsMode(std::vector<int> i_slavePlayers) {
  bool v_found;

  for(unsigned int i=0; i<m_otherClients.size(); i++) {
    v_found = false;
    for(unsigned j=0; j<i_slavePlayers.size(); j++) {
      if(m_otherClients[i]->id() == i_slavePlayers[j]) {
	m_otherClients[i]->setMode(NETCLIENT_SLAVE_MODE);
	v_found = true;
      }
    }
    if(v_found == false) {
      m_otherClients[i]->setMode(NETCLIENT_GHOST_MODE);
    }
  }
}

void NetClient::manageAction(xmDatabase* pDb, NetAction* i_netAction) {
  switch(i_netAction->actionType()) {

  case TNA_clientInfos:
  case TNA_clientMode:
  case TNA_playerControl:
  case TNA_srvCmd:
  case TNA_clientsNumber:
  case TNA_clientsNumberQuery:
    /* should not happend */
    break;

  case TNA_udpBindQuery:
    {
      NA_udpBind na(m_udpBindKey);
      try {
	// send the packet 3 times to get more change it arrives
	for(unsigned int i=0; i<3; i++) {
	  send(&na, 0, true);
	}
      } catch(Exception &e) {
      }
    }
    break;

  case TNA_udpBind:
    {
      if(m_serverSendsUdp == false) {
	m_serverSendsUdp = true;
	LogInfo("client: i can receive udp from the server");
	NA_udpBindValidation na;
	try {
	  send(&na, 0);
	} catch(Exception &e) {
	}
      }
    }

  case TNA_udpBindValidation:
    {
      if(m_serverReceivesUdp == false) {
	LogInfo("client: the server can receive udp from me");
	m_serverReceivesUdp = true;
      }
    }
    break;

  case TNA_chatMessage:
    {
      try {
	std::string v_str;
	std::string v_author;

	// retrieve message
	v_str = ((NA_chatMessage*)i_netAction)->getMessage();
	if(i_netAction->getSource() == -1) { /* server */
	  v_author = "server";
	  SysMessage::instance()->addConsoleLine(getDisplayMessage(v_str, v_author), CLT_SERVER);
	} else {
	  v_author = m_otherClients[getOtherClientNumberById(i_netAction->getSource())]->name();
	  SysMessage::instance()->addConsoleLine(getDisplayMessage(v_str, v_author));
	}
      } catch(Exception &e) {
      }
    }
    break;

  case TNA_chatMessagePP:
    {
      try {
	std::string v_str;
	std::string v_author;

	// retrieve message
	v_str = ((NA_chatMessagePP*)i_netAction)->getMessage();
	if(utf8::is_utf8_valid(v_str)) { // ignore if the message is not valid
	  if(i_netAction->getSource() == -1) { /* server */
	    v_author = "server";
	    SysMessage::instance()->addConsoleLine(getDisplayMessage(v_str, v_author), CLT_SERVER);
	  } else {
	    v_author = m_otherClients[getOtherClientNumberById(i_netAction->getSource())]->name();
	    
	    if(((NA_chatMessagePP*)i_netAction)->privatePeople().size() == 0) { /* public */
	      SysMessage::instance()->addConsoleLine(getDisplayMessage(v_str, v_author));
	    } else { /* private */
	      SysMessage::instance()->addConsoleLine(getDisplayMessage(v_str, v_author), CLT_PRIVATE);
	    }
	  }
	}

      } catch(Exception &e) {
      }
    }
    break;
    
  case TNA_frame:
    {
      NetGhost* v_ghost = NULL;
      int v_clientId = -1;

      if(m_universe == NULL) {
	return;
      }

      /* the server sending us our own frame */
      if(i_netAction->getSource() == -1) {

	if(GameApp::getXMTimeInt() - m_currentOwnFramesTime > 1000) {
	  m_lastOwnFPS = (m_currentOwnFramesNb*1000) / (GameApp::getXMTimeInt() - m_currentOwnFramesTime);
	  m_currentOwnFramesTime = GameApp::getXMTimeInt();
	  m_currentOwnFramesNb = 0;
	}
	m_currentOwnFramesNb++;

	for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	  for(unsigned int j=0; j<m_universe->getScenes()[i]->Players().size(); j++) {
	    
	      BikeState::convertStateFromReplay(((NA_frame*)i_netAction)->getState(),
						m_universe->getScenes()[i]->Players()[j]->getStateForUpdate(),
						m_universe->getScenes()[i]->getPhysicsSettings());

	      // adjust the time of the server frame to the time of the local scene
	      m_universe->getScenes()[i]->setTargetTime(((NA_frame*)i_netAction)->getState()->fGameTime*100.0);
	  }

	  // if the game is in pause, at least update the player position
	  if(m_universe->getScenes()[i]->isPaused()) {
	    m_universe->getScenes()[i]->updatePlayers(0 /* 0 to not update */, true);
	  }
	}
      } else {

	// search the client
	for(unsigned int i=0; i<m_otherClients.size(); i++) {
	  if(m_otherClients[i]->id() == i_netAction->getSource()) {
	    v_clientId = i;
	    break;
	  }
	}
	if(v_clientId < 0) {
	  return; // client not declared
	}
	
	// check if the ghost already exists
	if(m_otherClients[v_clientId]->netGhost(i_netAction->getSubSource()) != NULL) {
	  v_ghost = m_otherClients[v_clientId]->netGhost(i_netAction->getSubSource());
	}
	
	if(v_ghost == NULL) {
	  /* add the net ghost */

	  // if this is a client of the current party, add it as normal player
	  bool v_isSlaveMode = m_otherClients[v_clientId]->mode() == NETCLIENT_SLAVE_MODE;

	  for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {

	    if(v_isSlaveMode) {
	      v_ghost = m_universe->getScenes()[i]->
		addNetGhost(m_otherClients[v_clientId]->name(), Theme::instance(),
			    Theme::instance()->getNetPlayerTheme(),
			    TColor(0,255,255,0),
			    TColor(GET_RED(Theme::instance()->getNetPlayerTheme()->getUglyRiderColor()),
				   GET_GREEN(Theme::instance()->getNetPlayerTheme()->getUglyRiderColor()),
				   GET_BLUE(Theme::instance()->getNetPlayerTheme()->getUglyRiderColor()),
				   0)
			    );
	    } else {
	      v_ghost = m_universe->getScenes()[i]->
		addNetGhost(m_otherClients[v_clientId]->name(), Theme::instance(),
			    Theme::instance()->getGhostTheme(),
			    TColor(255,255,255,0),
			    TColor(GET_RED(Theme::instance()->getGhostTheme()->getUglyRiderColor()),
				   GET_GREEN(Theme::instance()->getGhostTheme()->getUglyRiderColor()),
				   GET_BLUE(Theme::instance()->getGhostTheme()->getUglyRiderColor()),
				   0)
			    );
	    }
	    m_otherClients[v_clientId]->setNetGhost(i_netAction->getSubSource(), v_ghost);
	  }
	}
	
	// take the physic of the first world
	if(m_universe->getScenes().size() > 0) {
	  BikeState::convertStateFromReplay(((NA_frame*)i_netAction)->getState(), v_ghost->getStateForUpdate(),
					    m_universe->getScenes()[0]->getPhysicsSettings());
	}
      }
    }
    break;
      
  case TNA_changeName:
    {
      // change the client name
      for(unsigned int i=0; i<m_otherClients.size(); i++) {
	if(m_otherClients[i]->id() == i_netAction->getSource()) {
	  m_otherClients[i]->setName(((NA_changeName*)i_netAction)->getName());
	}
      }
    }
    break;

  case TNA_playingLevel:
    {
      try {
	std::string v_levelId = ((NA_playingLevel*)i_netAction)->getLevelId();
	NetOtherClient* v_client = m_otherClients[getOtherClientNumberById(i_netAction->getSource())];
	char buf[512];

	// updating playing level
	if(v_levelId != "" && v_levelId != v_client->lastPlayingLevelId()) {
	  v_client->setPlayingLevelId(pDb, v_levelId);
	  snprintf(buf, 512, GAMETEXT_CLIENTPLAYING, v_client->name().c_str(), v_client->playingLevelName().c_str());
	  SysMessage::instance()->addConsoleLine(buf, CLT_GAMEINFORMATION);
	} else {
	  v_client->setPlayingLevelId(pDb, v_levelId);
	}

      } catch(Exception &e) {
      }
    }
    break;

  case TNA_serverError:
    {
      //KY
      // try to translate this message : server messages are sent untranslated
      SysMessage::instance()->displayError(_(((NA_serverError*)i_netAction)->getMessage().c_str()));
    }
    break;

  case TNA_changeClients:
    {
      NetInfosClient nic;
      char buf[512];

      for(unsigned int i=0; i<((NA_changeClients*)i_netAction)->getAddedInfosClients().size(); i++) {
	m_otherClients.push_back(new NetOtherClient(((NA_changeClients*)i_netAction)->getAddedInfosClients()[i].NetId,
						    ((NA_changeClients*)i_netAction)->getAddedInfosClients()[i].Name));
	snprintf(buf, 512, GAMETEXT_CLIENTCONNECTSERVER, ((NA_changeClients*)i_netAction)->getAddedInfosClients()[i].Name.c_str());
	SysMessage::instance()->addConsoleLine(buf, CLT_GAMEINFORMATION);
      }

      for(unsigned int i=0; i<((NA_changeClients*)i_netAction)->getRemovedInfosClients().size(); i++) {
	unsigned int j=0;
	while(j<m_otherClients.size()) {
	  if(m_otherClients[j]->id() == ((NA_changeClients*)i_netAction)->getRemovedInfosClients()[i].NetId) {
	    snprintf(buf, 512, GAMETEXT_CLIENTDISCONNECTSERVER, m_otherClients[j]->name().c_str());
	    SysMessage::instance()->addConsoleLine(buf, CLT_GAMEINFORMATION);
	    delete m_otherClients[j];
	    m_otherClients.erase(m_otherClients.begin()+j);
	  } else {
	    j++;
	  }
	}
      }
    }
    break;
    
  case TNA_slaveClientsPoints:
    {
      for(unsigned int i=0; i<((NA_slaveClientsPoints*)i_netAction)->getPointsClients().size(); i++) {
	if(((NA_slaveClientsPoints*)i_netAction)->getPointsClients()[i].NetId == -1) { // self
	  m_points = ((NA_slaveClientsPoints*)i_netAction)->getPointsClients()[i].Points;
	} else { // find the associated player
	  for(unsigned int j=0; j<m_otherClients.size(); j++) {
	    // don't check the mode while the mode can be still not received (it's set via marked to play)
	    if(m_otherClients[j]->id() == ((NA_slaveClientsPoints*)i_netAction)->getPointsClients()[i].NetId) {
	      m_otherClients[j]->setPoints(((NA_slaveClientsPoints*)i_netAction)->getPointsClients()[i].Points);
	    }
	  }
	}
      }
    }
    break;

  case TNA_prepareToPlay:
    {
      updateOtherClientsMode(((NA_prepareToPlay*)i_netAction)->players());
      StateManager::instance()->sendAsynchronousMessage("NET_PREPARE_PLAYING", ((NA_prepareToPlay*)i_netAction)->idLevel());
    }
    break;

  case TNA_prepareToGo:
    {
      if(m_universe != NULL) {
	std::ostringstream v_alert;

	if(((NA_prepareToGo*)i_netAction)->time() == 0) {
	  v_alert << GAMETEXT_GO;
	} else {
	  v_alert << ((NA_prepareToGo*)i_netAction)->time();
	}
	for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	  m_universe->getScenes()[i]->gameMessage(v_alert.str(), true, XMCLIENT_PREPARE_TO_PLAY_DURATION);
	  if(((NA_prepareToGo*)i_netAction)->time() == 0) {
	    if(m_universe->getScenes()[i]->isPaused()) {
	      m_universe->getScenes()[i]->pause();
	    }
	  }
	}
      }
    }
    break;

  case TNA_killAlert:
    {
      if(m_universe != NULL) {
	std::ostringstream v_alert;
	v_alert << ((NA_killAlert*)i_netAction)->time();
	for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	  m_universe->getScenes()[i]->gameMessage(v_alert.str(), true, XMCLIENT_KILL_ALERT_DURATION);
	}
      }
    }
    break;

  case TNA_gameEvents:
    {
      if(m_universe != NULL) {
	DBuffer v_buffer;
	v_buffer.initInput(((NA_gameEvents*)i_netAction)->buffer(), ((NA_gameEvents*)i_netAction)->bufferSize());

	while(v_buffer.numRemainingBytes() > 0) {
	  SceneEvent* v_se = SceneEvent::getUnserialized(v_buffer, false);
	  for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	    m_universe->getScenes()[i]->handleEvent(v_se);
	  }
	  delete v_se;
	}
      }
    }
    break;

  case TNA_srvCmdAsw:
    {
      StateManager::instance()->sendAsynchronousMessage("NET_SRVCMDASW", ((NA_srvCmdAsw*)i_netAction)->getAnswer());
    }
    break;

  case TNA_ping:
    {
      if(((NA_ping*)i_netAction)->isPong()) {
        if(m_lastPing.id == ((NA_ping*)i_netAction)->id()) {
          // same id, update the rcv time
	  m_lastPing.pongTime = GameApp::getXMTimeInt();
        } else {
          // not the same last id, so, it means that an other ping has been send. Ignore this pong.
	}
      } else { // receive a ping
	NA_ping na(((NA_ping*)i_netAction)); // send the pong as answer to the ping
	try {
	  send(&na, 0);
	} catch(Exception &e) {
	}
      }
    }
    break;

  }
}

NetOtherClient::NetOtherClient(int i_id, const std::string& i_name) {
  m_id       = i_id;
  m_name     = i_name;
  m_netMode  = NETCLIENT_ANY_MODE;

  for(unsigned int i=0; i<NETACTION_MAX_SUBSRC; i++) {
    m_ghosts[i] = NULL;
  }
}

NetOtherClient::~NetOtherClient() {
}

int NetOtherClient::id() const {
  return m_id;
}

std::string NetOtherClient::name() const {
  return m_name;
}

void NetOtherClient::setName(const std::string& i_name) {
  m_name = i_name;
}

NetClientMode NetOtherClient::mode() const {
  return m_netMode;
}

void NetOtherClient::setMode(NetClientMode i_netMode) {
  m_netMode = i_netMode;
}

std::string NetOtherClient::lastPlayingLevelId() {
  return m_lastPlayingLevelId;
}

void NetOtherClient::setPlayingLevelId(xmDatabase* pDb, const std::string& i_id_level) {

  if(i_id_level != "") {
    m_lastPlayingLevelId = i_id_level;
  }

  // update levelName
  if(i_id_level == "") {
    m_playingLevelName = "";
  } else if(i_id_level != m_playingLevelId) {
    char **v_result;
    unsigned int nrow;
  
    v_result = pDb->readDB("SELECT name FROM levels where id_level=\"" + xmDatabase::protectString(i_id_level) + "\";", nrow);
    if(nrow == 0) {
      m_playingLevelName = GAMETEXT_UNKNOWN;
    } else {
      m_playingLevelName = pDb->getResult(v_result, 1, 0, 0);  
    }
    pDb->read_DB_free(v_result);    
  }

  m_playingLevelId = i_id_level;
}

std::string NetOtherClient::playingLevelName() {
  return m_playingLevelName;
}

NetGhost* NetOtherClient::netGhost(unsigned int i_subsrc) {
  return m_ghosts[i_subsrc];
}

void NetOtherClient::setNetGhost(unsigned int i_subsrc, NetGhost* i_netGhost) {
  m_ghosts[i_subsrc] = i_netGhost;
}

int NetOtherClient::points() const {
  return m_points;
}

void NetOtherClient::setPoints(int i_points) {
  m_points = i_points;
}

void NetClient::getOtherClientsNameList(std::vector<std::string>& io_list, const std::string& i_suffix) {
  for (int i = 0, n = m_otherClients.size(); i < n; i++) {
    io_list.push_back(m_otherClients[i]->name() + i_suffix);
  }
}

void NetClient::addChatTransformations(std::vector<std::string>& io_clientList, const std::string& i_suffix) {
  io_clientList.push_back("/me" + i_suffix);
}

std::string NetClient::getDisplayMessage(const std::string& i_msg, const std::string& i_author) {

  if(i_author + " " == i_msg.substr(0, i_author.size()+1)) {
    return i_msg;
  }

  // add author only the message is not starting by it
  return i_author + ": " + i_msg;
}

VirtualNetLevelsList* NetClient::getOtherClientLevelsList(xmDatabase* pDb) {
  m_otherClientsLevelsList->setDb(pDb);
  return m_otherClientsLevelsList;
}

void NetClient::fillPrivatePeople(const std::string& i_msg, const std::string& i_private_suffix,
				  std::vector<int>& io_private_people,
				  std::vector<std::string>& o_unknown_players)  {
  int n = 0;
  size_t nfound;
  std::string v_toanalyse, v_lastWord;
  int v_lastWordPos;
  bool v_found, v_foundList;
  
  //printf("=>+%s+\n", i_msg.c_str());
  
  // main case : no private message
  if(i_msg.find(i_private_suffix) == std::string::npos) {
    return;
  }

  // get substring to the next space
  while( (nfound = i_msg.find(i_private_suffix, n)) != std::string::npos ) {
    v_toanalyse = i_msg.substr(n, nfound-n);
    n = nfound+i_private_suffix.length();
    v_lastWordPos = v_toanalyse.rfind(" ") +1;
    v_lastWord = v_toanalyse.substr(v_lastWordPos);
    
    //printf("+%s+%s+(keeping+%s+)\n", v_toanalyse.c_str(), v_lastWord.c_str(), i_msg.substr(n).c_str());

    // for each client, if v_tonalyse is (space|nothing before)name +
    v_found = false;
    for(unsigned int i=0; i<m_otherClients.size(); i++) {
      if(m_otherClients[i]->name() == v_lastWord) {
	v_found = true;
	v_foundList = false;
	for(unsigned int j=0; j<io_private_people.size(); j++) {
	  if(io_private_people[j] == m_otherClients[i]->id()) {
	    v_foundList = true;
	  }
	}
	if(v_foundList == false) {
	  io_private_people.push_back(m_otherClients[i]->id());
	  //printf("add %s\n", v_lastWord.c_str());
	}
      }
    }

    // not found => add it into the unknown players list
    if(v_found == false) {
      v_foundList = false;
      for(unsigned int i=0; i<o_unknown_players.size(); i++) {
	if(o_unknown_players[i] == v_lastWord) {
	  v_foundList = true;
	}
      }
      if(v_foundList == false) {
	o_unknown_players.push_back(v_lastWord);
      }
    }
  }
}

void NetClient::memoriesPP(const std::vector<int>& i_private_people) {
  m_previous_private_people = i_private_people;
}

std::string NetClient::getMemoriedPPAsString(const std::string& i_suffix) {
  std::string v_res;
  
  for(unsigned int i=0; i<m_previous_private_people.size(); i++) {
    for(unsigned int j=0; j<m_otherClients.size(); j++) {
      if(m_previous_private_people[i] == m_otherClients[j]->id()) {
	v_res += m_otherClients[j]->name() + i_suffix;
      }
    }
  }
  return v_res;
}
