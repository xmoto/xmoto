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

#include "NetActions.h"
#include "../helpers/VExcept.h"
#include "../helpers/Log.h"
#include "NetClient.h"
#include "../XMSession.h"
#include <sstream>

#define NETACTION_MAX_SUBSRC 4 // maximum 4 players by client

char NetAction::m_buffer[NETACTION_MAX_PACKET_SIZE];
unsigned int NetAction::m_biggestTCPPacket   = 0;
unsigned int NetAction::m_biggestUDPPacket   = 0;
unsigned int NetAction::m_nbTCPPacketsSent   = 0;
unsigned int NetAction::m_nbUDPPacketsSent   = 0;
unsigned int NetAction::m_TCPPacketsSizeSent = 0;
unsigned int NetAction::m_UDPPacketsSizeSent = 0;

std::string NA_chatMessage::ActionKey  = "message";
// frame : while it's sent a lot, reduce it at maximum
std::string NA_frame::ActionKey        = "f";
std::string NA_clientInfos::ActionKey  = "clientInfos";
std::string NA_udpBind::ActionKey      = "udpbind";
std::string NA_udpBindQuery::ActionKey = "udpbindingQuery";
std::string NA_changeName::ActionKey = "changeName";
std::string NA_playingLevel::ActionKey = "playingLevel";
std::string NA_serverError::ActionKey  = "serverError";

NetActionType NA_chatMessage::NAType  = TNA_chatMessage;
NetActionType NA_frame::NAType        = TNA_frame;
NetActionType NA_clientInfos::NAType  = TNA_clientInfos;
NetActionType NA_udpBind::NAType      = TNA_udpBind;
NetActionType NA_udpBindQuery::NAType = TNA_udpBindQuery;
NetActionType NA_changeName::NAType = TNA_changeName;
NetActionType NA_playingLevel::NAType = TNA_playingLevel;
NetActionType NA_serverError::NAType  = TNA_serverError;

NetAction::NetAction() {
  m_source    = -2; // < -1 => undefined
  m_subsource = -2;
}

NetAction::~NetAction() {
}

void NetAction::logStats() {
  LogInfo("net: number of TCP packets sent : %u", NetAction::m_nbTCPPacketsSent);
  LogInfo("net: biggest TCP packet sent : %u bytes", NetAction::m_biggestTCPPacket);
  LogInfo("net: size of TCP packets sent : %u bytes", NetAction::m_TCPPacketsSizeSent);
  LogInfo("net: number of UDP packets sent : %u", NetAction::m_nbUDPPacketsSent);
  LogInfo("net: biggest UDP packet sent : %u bytes", NetAction::m_biggestUDPPacket);
  LogInfo("net: size of UDP packets sent : %i bytes", NetAction::m_UDPPacketsSizeSent);
}

void NetAction::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP,
		     const void* subPacketData, int subPacketLen) {
  unsigned int nread;
  std::string v_data;

  if(m_source < -1 || m_subsource < -1) {
    throw Exception("Invalid source");
  }

  unsigned int v_subPacketSize = subPacketLen + 1;
  std::ostringstream v_nb, v_src, v_subsrc;
  v_src    << m_source;
  v_subsrc << m_subsource;
  unsigned int v_subHeaderSize    =
    v_src.str().length()    + 1 +
    v_subsrc.str().length() + 1 +
    actionKey().length()    + 1;
  v_nb << (v_subHeaderSize + v_subPacketSize);
  unsigned int v_headerSize = v_nb.str().length() + 1 + v_subHeaderSize;

  unsigned int v_totalPacketSize = v_headerSize + v_subPacketSize;

  if(v_totalPacketSize > NETACTION_MAX_PACKET_SIZE) {
    throw Exception("net: too big packet to send");
  }

  snprintf(m_buffer, NETACTION_MAX_PACKET_SIZE, "%s\n%s\n%s\n%s\n",
	   v_nb.str().c_str(), v_src.str().c_str(), v_subsrc.str().c_str(), actionKey().c_str());
  if(subPacketLen != 0) {
    memcpy(m_buffer + v_headerSize, subPacketData, subPacketLen);
  }
  m_buffer[v_totalPacketSize-1] = '\n';

  if(i_udpsd != NULL) {
    if((v_totalPacketSize) > (unsigned int) i_sendPacket->maxlen) {
      LogWarning("UDP packet too big");
    } else {
      i_sendPacket->len = v_totalPacketSize;
      memcpy(i_sendPacket->data, m_buffer, v_totalPacketSize);

      i_sendPacket->address = *i_udpRemoteIP;
      if(SDLNet_UDP_Send(*i_udpsd, -1, i_sendPacket) == 0) {
	LogWarning("SDLNet_UDP_Send failed : %s", SDLNet_GetError());
      }

      if(v_totalPacketSize > NetAction::m_biggestUDPPacket) {
	NetAction::m_biggestUDPPacket = v_totalPacketSize;
      }
      NetAction::m_nbUDPPacketsSent++;
      NetAction::m_UDPPacketsSizeSent += v_totalPacketSize;
    }

  } else if(i_tcpsd != NULL) {
    // don't send the \0
    if( (nread = SDLNet_TCP_Send(*i_tcpsd, m_buffer, v_totalPacketSize)) != v_totalPacketSize) {
      throw Exception("TCP_Send failed");
    }

    if(v_totalPacketSize > NetAction::m_biggestTCPPacket) {
      NetAction::m_biggestTCPPacket = v_totalPacketSize;
    }
    NetAction::m_nbTCPPacketsSent++;
    NetAction::m_TCPPacketsSizeSent += v_totalPacketSize;

  } else {
    LogWarning("Packet not send, no protocol set");
  }
}

void NetAction::setSource(int i_src, int i_subsrc) {
  m_source    = i_src;
  m_subsource = i_subsrc;
}

int NetAction::getSource() const {
  return m_source;
}

int NetAction::getSubSource() const {
  return m_subsource;
}

NetAction* NetAction::newNetAction(void* data, unsigned int len) {
  std::string v_cmd;
  int v_src, v_subsrc;
  unsigned int v_local_offset, v_totalOffset = 0;
  NetAction* v_res;

  v_src = atoi(getLine(((char*)data)+v_totalOffset, len-v_totalOffset, &v_local_offset).c_str());
  v_totalOffset += v_local_offset;

  v_subsrc = atoi(getLine(((char*)data+v_totalOffset), len-v_totalOffset, &v_local_offset).c_str());
  v_totalOffset += v_local_offset;

  if(v_src < -1 || v_subsrc < -1 || v_subsrc >= NETACTION_MAX_SUBSRC) {
    throw Exception("Invalid source");
  }

  v_cmd = getLine(((char*)data+v_totalOffset), len, &v_local_offset);
  v_totalOffset += v_local_offset;

  if(v_cmd == NA_chatMessage::ActionKey) {
    v_res = new NA_chatMessage(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_frame::ActionKey) {
    v_res = new NA_frame(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_clientInfos::ActionKey) {
    v_res = new NA_clientInfos(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_udpBindQuery::ActionKey) {
    v_res = new NA_udpBindQuery(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_udpBind::ActionKey) {
    v_res = new NA_udpBind(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_changeName::ActionKey) {
    v_res = new NA_changeName(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_playingLevel::ActionKey) {
    v_res = new NA_playingLevel(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_serverError::ActionKey) {
    v_res = new NA_serverError(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else {
    //((char*)data)[len-1] = '\0';
    //LogInfo("Invalid command : %s", (char*)data);
    throw Exception("net: invalid command");
  }

  v_res->setSource(v_src, v_subsrc);
  return v_res;
}

std::string NetAction::getLine(void* data, unsigned int len, unsigned int* v_local_offset) {
  std::string v_res;
  *v_local_offset = 0;

  while(*v_local_offset < len && ((char*)data)[*v_local_offset] != '\n') {
    (*v_local_offset)++;
  }
  (*v_local_offset)++;

  if(*v_local_offset > len) {
    throw Exception("NetAction: no line found");
  }

  ((char*)data)[(*v_local_offset)-1] = '\0';
  v_res = std::string(((char*)data));
  ((char*)data)[(*v_local_offset)-1] = '\n';

  return v_res;
}

void NetAction::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  NetAction::send(i_tcpsd, i_udpsd, i_sendPacket, i_udpRemoteIP, NULL, 0);
}

NA_chatMessage::NA_chatMessage(const std::string& i_msg) {
  m_msg = i_msg;
}

NA_chatMessage::NA_chatMessage(void* data, unsigned int len) {
  ((char*)data)[len-1] = '\0';
  m_msg = std::string((char*)data);
  ((char*)data)[len-1] = '\n';
}

NA_chatMessage::~NA_chatMessage() {
}

void NA_chatMessage::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  NetAction::send(i_tcpsd, NULL, NULL, NULL, m_msg.c_str(), m_msg.size()); // don't send the \0
}

std::string NA_chatMessage::getMessage() {
  return m_msg;
}

NA_serverError::NA_serverError(const std::string& i_msg) {
  m_msg = i_msg;
}

NA_serverError::NA_serverError(void* data, unsigned int len) {
  ((char*)data)[len-1] = '\0';
  m_msg = std::string((char*)data);
  ((char*)data)[len-1] = '\n';
}

NA_serverError::~NA_serverError() {
}

void NA_serverError::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  NetAction::send(i_tcpsd, NULL, NULL, NULL, m_msg.c_str(), m_msg.size()); // don't send the \0
}

std::string NA_serverError::getMessage() {
  return m_msg;
}

NA_frame::NA_frame(SerializedBikeState* i_state) {
  m_state = *i_state;
}

NA_frame::NA_frame(void* data, unsigned int len) {
  if(len-1 != sizeof(SerializedBikeState)) {
    throw Exception("Invalid NA_frame");
  }
  memcpy(&m_state, data, len-1); // -1 because in the protocol, you always finish by a \n
}

NA_frame::~NA_frame() {
}

void NA_frame::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  if(i_udpsd != NULL) {
    // if udp is available, prefer udp
    NetAction::send(NULL, i_udpsd, i_sendPacket, i_udpRemoteIP, &m_state, sizeof(SerializedBikeState));
  } else {
    NetAction::send(i_tcpsd, i_udpsd, i_sendPacket, i_udpRemoteIP, &m_state, sizeof(SerializedBikeState));
  }
}

SerializedBikeState* NA_frame::getState() {
  return &m_state;
}

NA_udpBind::NA_udpBind(const std::string& i_key) {
  m_key = i_key;
}

NA_udpBind::NA_udpBind(void* data, unsigned int len) {
  unsigned int v_localOffset;
  m_key = getLine(data, len, &v_localOffset);
}

NA_udpBind::~NA_udpBind() {
}

void NA_udpBind::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  // force UDP
  NetAction::send(NULL, i_udpsd, i_sendPacket, i_udpRemoteIP, m_key.c_str(), m_key.size()); // don't send the \0
}

std::string NA_udpBind::key() const {
  return m_key;
}

NA_udpBindQuery::NA_udpBindQuery() {
}

NA_udpBindQuery::NA_udpBindQuery(void* data, unsigned int len) {
}

NA_udpBindQuery::~NA_udpBindQuery() {
}

void NA_udpBindQuery::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  // force TCP
  NetAction::send(i_tcpsd, NULL, NULL, NULL, NULL, 0);
}

NA_clientInfos::NA_clientInfos(int i_protocolVersion, const std::string& i_udpBindKey) {
  m_protocolVersion = i_protocolVersion;
  m_udpBindKey      = i_udpBindKey;
}

NA_clientInfos::NA_clientInfos(void* data, unsigned int len) {
  unsigned int v_localOffset;
  m_protocolVersion = atoi(getLine(data, len, &v_localOffset).c_str());
  m_udpBindKey      = getLine(((char*)data)+v_localOffset, len-v_localOffset, &v_localOffset);
}

NA_clientInfos::~NA_clientInfos() {
}

void NA_clientInfos::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  std::ostringstream v_protocolVersion;
  v_protocolVersion << m_protocolVersion;

  std::string v_send;
  v_send = v_protocolVersion.str() + "\n" + m_udpBindKey;

  // force TCP
  NetAction::send(i_tcpsd, NULL, NULL, NULL, v_send.c_str(), v_send.size()); // don't send the \0
}

int NA_clientInfos::protocolVersion() const {
  return m_protocolVersion;
}

std::string NA_clientInfos::udpBindKey() const {
  return m_udpBindKey;
}

NA_changeName::NA_changeName(const std::string& i_name) {
  m_name = i_name;
}

NA_changeName::NA_changeName(void* data, unsigned int len) {
  unsigned int v_localOffset;
  m_name = getLine(data, len, &v_localOffset);
}

NA_changeName::~NA_changeName() {
}

void NA_changeName::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  NetAction::send(i_tcpsd, NULL, NULL, NULL, m_name.c_str(), m_name.size()); // don't send the \0
}

std::string NA_changeName::getName() {
  return m_name;
}

NA_playingLevel::NA_playingLevel(const std::string& i_levelId) {
  m_levelId = i_levelId;
}

NA_playingLevel::NA_playingLevel(void* data, unsigned int len) {
  unsigned int v_localOffset;
  m_levelId = getLine(data, len, &v_localOffset);
}

NA_playingLevel::~NA_playingLevel() {
}

void NA_playingLevel::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  NetAction::send(i_tcpsd, NULL, NULL, NULL, m_levelId.c_str(), m_levelId.size()); // don't send the \0
}

std::string NA_playingLevel::getLevelId() {
  return m_levelId;
}
