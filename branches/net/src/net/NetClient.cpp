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
#include "../helpers/Log.h"
#include "helpers/Net.h"
#include "../XMSession.h"
#include "../states/StateManager.h"
#include <sstream>
#include "../helpers/VMath.h"
#include "../SysMessage.h"
#include "../Universe.h"
#include "../Theme.h"
#include "../xmscene/BikeGhost.h"
#include "../GameText.h"
#include "../db/xmDatabase.h"

NetClient::NetClient() {
    m_isConnected = false;
    m_clientListenerThread = NULL;
    m_netActionsMutex = SDL_CreateMutex();
    m_universe = NULL;
    m_udpSendPacket = SDLNet_AllocPacket(XM_CLIENT_MAX_UDP_PACKET_SIZE);

    if(!m_udpSendPacket) {
      throw Exception("SDLNet_AllocPacket: " + std::string(SDLNet_GetError()));
    }

    std::ostringstream v_rd;
    v_rd << randomIntNum(1, RAND_MAX);
    m_udpBindKey = v_rd.str();
}

NetClient::~NetClient() {
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

void NetClient::executeNetActions() {
  if(m_netActions.size() == 0) {
    return; // try to not lock via mutex if not needed
  }

  SDL_LockMutex(m_netActionsMutex);
  for(unsigned int i=0; i<m_netActions.size(); i++) {
    //LogInfo("Execute NetAction (%s)", m_netActions[i]->actionKey().c_str());
    manageAction(m_netActions[i]);
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

void NetClient::send(NetAction* i_netAction, int i_subsrc) {
  i_netAction->setSource(0, i_subsrc);

  try {
    i_netAction->send(&m_tcpsd, &m_udpsd, m_udpSendPacket, &m_udpSendPacket->address);
  } catch(Exception &e) {
    disconnect();
    LogWarning("send failed : %s", e.getMsg().c_str());
    throw e;
  }
}

void NetClient::startPlay(Universe* i_universe) {
  m_universe = i_universe;
}

void NetClient::endPlay() {
  for(unsigned int i=0; i<m_otherClients.size(); i++) {
    for(unsigned int j=0; j<NETACTION_MAX_SUBSRC; j++) {
      m_otherClients[i]->setNetGhost(j, NULL);
    }
  }
  m_universe = NULL;
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

void NetClient::manageAction(NetAction* i_netAction) {
  switch(i_netAction->actionType()) {

  case TNA_udpBind:
    /* should not happend */
    break;

  case TNA_udpBindQuery:
    {
      NA_udpBind na(m_udpBindKey);
      try {
	// send the packet 3 times to get more change it arrives
	for(unsigned int i=0; i<3; i++) {
	  send(&na, 0);
	}
      } catch(Exception &e) {
      }
    }
    break;

  case TNA_clientInfos:
    /* should not happend */
    break;
      
  case TNA_chatMessage:
    {
      try {
	std::string v_str = m_otherClients[getOtherClientNumberById(i_netAction->getSource())]->name() +
	  ": " + ((NA_chatMessage*)i_netAction)->getMessage();
	SysMessage::instance()->addConsoleLine(v_str);
      } catch(Exception &e) {
      }
    }
    break;
    
  case TNA_frame:
    {
      //LogInfo("Frame received");
      NetGhost* v_ghost = NULL;
      int v_clientId = -1;

      if(m_universe == NULL) {
	return;
      }

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
	for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	  v_ghost = m_universe->getScenes()[i]->addNetGhost("Net ghost", Theme::instance(),
							    Theme::instance()->getGhostTheme(),
							    TColor(255,255,255,0),
							    TColor(GET_RED(Theme::instance()->getGhostTheme()->getUglyRiderColor()),
								   GET_GREEN(Theme::instance()->getGhostTheme()->getUglyRiderColor()),
								   GET_BLUE(Theme::instance()->getGhostTheme()->getUglyRiderColor()),
								   0)
							    );
	  m_otherClients[v_clientId]->setNetGhost(i_netAction->getSubSource(), v_ghost);
	}
      }
      
      // take the physic of the first world
      if(m_universe->getScenes().size() > 0) {
	BikeState::convertStateFromReplay(((NA_frame*)i_netAction)->getState(), v_ghost->getState(),
					  m_universe->getScenes()[0]->getPhysicsSettings());
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
	char **v_result;
	unsigned int nrow;
	std::string v_levelName;

	if(v_levelId != "" && v_levelId != v_client->lastPlayingLevelId()) {
	  xmDatabase* pDb = xmDatabase::instance("main");
  
	  v_result = pDb->readDB("SELECT name FROM levels where id_level=\"" + xmDatabase::protectString(v_levelId) + "\";", nrow);
	  if(nrow == 0) {
	    v_levelName = GAMETEXT_UNKNOWN;
	  } else {
	    v_levelName = pDb->getResult(v_result, 1, 0, 0);  
	  }
	  pDb->read_DB_free(v_result);

	  snprintf(buf, 512, GAMETEXT_CLIENTPLAYING, v_client->name().c_str(), v_levelName.c_str());
	  SysMessage::instance()->addConsoleLine(buf);
	}

	// updating playing level
	v_client->setPlayingLevelId(v_levelId);

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
	SysMessage::instance()->addConsoleLine(buf);
      }

      for(unsigned int i=0; i<((NA_changeClients*)i_netAction)->getRemovedInfosClients().size(); i++) {
	unsigned int j=0;
	while(j<m_otherClients.size()) {
	  if(m_otherClients[j]->id() == ((NA_changeClients*)i_netAction)->getRemovedInfosClients()[i].NetId) {
	    snprintf(buf, 512, GAMETEXT_CLIENTDISCONNECTSERVER, m_otherClients[j]->name().c_str());
	    SysMessage::instance()->addConsoleLine(buf);
	    delete m_otherClients[j];
	    m_otherClients.erase(m_otherClients.begin()+j);
	  } else {
	    j++;
	  }
	}
      }
    }
    break;

  }
}

NetOtherClient::NetOtherClient(int i_id, const std::string& i_name) {
  m_id       = i_id;
  m_name     = i_name;

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

std::string NetOtherClient::lastPlayingLevelId() {
  return m_lastPlayingLevelId;
}

void NetOtherClient::setPlayingLevelId(const std::string& i_id_level) {
  m_playingLevelId = i_id_level;
  if(m_playingLevelId != "") {
    m_lastPlayingLevelId = m_playingLevelId;
  }
}

NetGhost* NetOtherClient::netGhost(unsigned int i_subsrc) {
  return m_ghosts[i_subsrc];
}

void NetOtherClient::setNetGhost(unsigned int i_subsrc, NetGhost* i_netGhost) {
  m_ghosts[i_subsrc] = i_netGhost;
}
