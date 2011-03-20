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
#include "../helpers/Text.h"
#include "NetClient.h"
#include "../XMSession.h"
#include "../XMBuild.h"
#include "../DBuffer.h"
#include "extSDL_net.h"
#include <sstream>
#include "../helpers/SwapEndian.h"
#include "helpers/Net.h"
#include "helpers/utf8.h"

char NetAction::m_buffer[NETACTION_MAX_PACKET_SIZE];
unsigned int NetAction::m_biggestTCPPacketSent = 0;
unsigned int NetAction::m_biggestUDPPacketSent = 0;
unsigned int NetAction::m_nbTCPPacketsSent     = 0;
unsigned int NetAction::m_nbUDPPacketsSent     = 0;
unsigned int NetAction::m_TCPPacketsSizeSent   = 0;
unsigned int NetAction::m_UDPPacketsSizeSent   = 0;

std::string NA_chatMessage::ActionKey   = "message";
std::string NA_chatMessagePP::ActionKey = "messagePP";
// frame : while it's sent a lot, reduce it at maximum
std::string NA_frame::ActionKey        	= "f";
std::string NA_clientInfos::ActionKey  	= "clientInfos";
std::string NA_udpBind::ActionKey      	= "udpbind";
std::string NA_udpBindQuery::ActionKey 	= "udpbindingQuery";
std::string NA_udpBindValidation::ActionKey = "udpbindingValidation";
std::string NA_changeName::ActionKey    = "changeName";
std::string NA_playingLevel::ActionKey  = "playingLevel";
std::string NA_serverError::ActionKey   = "serverError";
std::string NA_changeClients::ActionKey = "changeClients";
std::string NA_slaveClientsPoints::ActionKey = "scpoints";
std::string NA_clientsNumber::ActionKey = "clientsNumber";
std::string NA_clientsNumberQuery::ActionKey = "clientsNumberQ";
// control : while it's sent a lot, reduce it at maximum
std::string NA_playerControl::ActionKey = "c";
std::string NA_clientMode::ActionKey  	= "clientMode";
std::string NA_prepareToPlay::ActionKey = "prepareToPlay";
std::string NA_killAlert::ActionKey     = "killAlert";
std::string NA_prepareToGo::ActionKey   = "prepareToGo";
// events are send regularly
std::string NA_gameEvents::ActionKey    = "e";
std::string NA_srvCmd::ActionKey        = "srvCmd";
std::string NA_srvCmdAsw::ActionKey     = "srvCmdAsw";

NetActionType NA_chatMessage::NAType   = TNA_chatMessage;
NetActionType NA_chatMessagePP::NAType = TNA_chatMessagePP;
NetActionType NA_frame::NAType         = TNA_frame;
NetActionType NA_clientInfos::NAType   = TNA_clientInfos;
NetActionType NA_udpBind::NAType       = TNA_udpBind;
NetActionType NA_udpBindQuery::NAType  = TNA_udpBindQuery;
NetActionType NA_udpBindValidation::NAType  = TNA_udpBindValidation;
NetActionType NA_changeName::NAType    = TNA_changeName;
NetActionType NA_clientsNumber::NAType = TNA_clientsNumber;
NetActionType NA_clientsNumberQuery::NAType = TNA_clientsNumberQuery;
NetActionType NA_playingLevel::NAType  = TNA_playingLevel;
NetActionType NA_serverError::NAType   = TNA_serverError;
NetActionType NA_changeClients::NAType = TNA_changeClients;
NetActionType NA_slaveClientsPoints::NAType = TNA_slaveClientsPoints;
NetActionType NA_playerControl::NAType = TNA_playerControl;
NetActionType NA_clientMode::NAType    = TNA_clientMode;
NetActionType NA_prepareToPlay::NAType = TNA_prepareToPlay;
NetActionType NA_killAlert::NAType     = TNA_killAlert;
NetActionType NA_prepareToGo::NAType   = TNA_prepareToGo;
NetActionType NA_gameEvents::NAType    = TNA_gameEvents;
NetActionType NA_srvCmd::NAType        = TNA_srvCmd;
NetActionType NA_srvCmdAsw::NAType     = TNA_srvCmdAsw;

NetAction::NetAction(bool i_forceTcp) {
  m_source    = -2; // < -1 => undefined
  m_subsource = -2;
  m_forceTCP  = i_forceTcp;
}

NetAction::~NetAction() {
}

/* log version */
void NetAction::logStats() {
  LogInfo("%-36s : %u", "net: number of TCP packets sent", NetAction::m_nbTCPPacketsSent);
  LogInfo("%-36s : %s", "net: biggest TCP packet sent"   , XMNet::getFancyBytes(NetAction::m_biggestTCPPacketSent).c_str());
  LogInfo("%-36s : %s", "net: size of TCP packets sent"  , XMNet::getFancyBytes(NetAction::m_TCPPacketsSizeSent).c_str());
  LogInfo("%-36s : %u", "net: number of UDP packets sent", NetAction::m_nbUDPPacketsSent);
  LogInfo("%-36s : %s", "net: biggest UDP packet sent"   , XMNet::getFancyBytes(NetAction::m_biggestUDPPacketSent).c_str());
  LogInfo("%-36s : %s", "net: size of UDP packets sent"  , XMNet::getFancyBytes(NetAction::m_UDPPacketsSizeSent).c_str());
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

  if(i_udpsd != NULL && m_forceTCP == false) {
    if((v_totalPacketSize) > (unsigned int) i_sendPacket->maxlen) {
      LogWarning("UDP packet too big");
    } else {
      i_sendPacket->len = v_totalPacketSize;
      memcpy(i_sendPacket->data, m_buffer, v_totalPacketSize);

      i_sendPacket->address = *i_udpRemoteIP;
      if(SDLNet_UDP_Send(*i_udpsd, -1, i_sendPacket) == 0) {
	LogWarning("SDLNet_UDP_Send failed : %s", SDLNet_GetError());
      }

      if(v_totalPacketSize > NetAction::m_biggestUDPPacketSent) {
	NetAction::m_biggestUDPPacketSent = v_totalPacketSize;
      }
      NetAction::m_nbUDPPacketsSent++;
      NetAction::m_UDPPacketsSizeSent += v_totalPacketSize;
    }

  } else if(i_tcpsd != NULL) {
    // don't send the \0
    if( (nread = SDLNet_TCP_Send_noBlocking(*i_tcpsd, m_buffer, v_totalPacketSize)) != v_totalPacketSize) {
      throw Exception("TCP_Send failed");
    }

    if(v_totalPacketSize > NetAction::m_biggestTCPPacketSent) {
      NetAction::m_biggestTCPPacketSent = v_totalPacketSize;
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
  unsigned int v_totalOffset = 0;
  NetAction* v_res;

  v_src = atoi(getLine(((char*)data)+v_totalOffset, len-v_totalOffset, &v_totalOffset).c_str());
  v_subsrc = atoi(getLine(((char*)data+v_totalOffset), len-v_totalOffset, &v_totalOffset).c_str());

  if(v_src < -1 || v_subsrc < 0 || v_subsrc >= NETACTION_MAX_SUBSRC) { // subsrc must be 0, 1 or 2 or 3
    throw Exception("Invalid source");
  }

  v_cmd = getLine(((char*)data+v_totalOffset), len, &v_totalOffset);

  //printf("cmd = %s (%i/%i)\n", v_cmd.c_str(), v_totalOffset, len-v_totalOffset);

  if(v_cmd == NA_chatMessage::ActionKey) {
    v_res = new NA_chatMessage(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_chatMessagePP::ActionKey) {
    v_res = new NA_chatMessagePP(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_frame::ActionKey) {
    v_res = new NA_frame(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_playerControl::ActionKey) {
    v_res = new NA_playerControl(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_clientInfos::ActionKey) {
    v_res = new NA_clientInfos(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_udpBindQuery::ActionKey) {
    v_res = new NA_udpBindQuery(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_udpBindValidation::ActionKey) {
    v_res = new NA_udpBindValidation(((char*)data)+v_totalOffset, len-v_totalOffset);
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

  else if(v_cmd == NA_changeClients::ActionKey) {
    v_res = new NA_changeClients(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_slaveClientsPoints::ActionKey) {
    v_res = new NA_slaveClientsPoints(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_clientsNumber::ActionKey) {
    v_res = new NA_clientsNumber(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_clientsNumberQuery::ActionKey) {
    v_res = new NA_clientsNumberQuery(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_clientMode::ActionKey) {
    v_res = new NA_clientMode(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_prepareToPlay::ActionKey) {
    v_res = new NA_prepareToPlay(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_killAlert::ActionKey) {
    v_res = new NA_killAlert(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_prepareToGo::ActionKey) {
    v_res = new NA_prepareToGo(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_gameEvents::ActionKey) {
    v_res = new NA_gameEvents(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_srvCmd::ActionKey) {
    v_res = new NA_srvCmd(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else if(v_cmd == NA_srvCmdAsw::ActionKey) {
    v_res = new NA_srvCmdAsw(((char*)data)+v_totalOffset, len-v_totalOffset);
  }

  else {
    //((char*)data)[len-1] = '\0';
    //LogInfo("Invalid command : %s", (char*)data);
    throw Exception("net: invalid command");
  }

  v_res->setSource(v_src, v_subsrc);
  return v_res;
}

std::string NetAction::getLine(void* data, unsigned int len, unsigned int* o_local_offset) {
  std::string v_res;
  unsigned int v_offset = 0;

  if(len <= 0) {
    throw Exception("NetAction: no line found");
  }

  while(v_offset < len && ((char*)data)[v_offset] != '\n') {
    v_offset++;
  }
  v_offset++;

  if(v_offset > len) {
    throw Exception("NetAction: no line found");
  }

  ((char*)data)[v_offset-1] = '\0';
  v_res = std::string(((char*)data));
  ((char*)data)[v_offset-1] = '\n';

  *o_local_offset += v_offset;

  return v_res;
}

std::string NetAction::getLineCheckUTF8(void* data, unsigned int len, unsigned int* o_local_offset) {
  std::string v_res;

  v_res = getLine(data, len, o_local_offset);
  if(utf8::is_utf8_valid(v_res) == false) {
    throw Exception("Invalid decoded strings (utf-8)");
  }

  return v_res;
}

void NetAction::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  NetAction::send(i_tcpsd, i_udpsd, i_sendPacket, i_udpRemoteIP, NULL, 0);
}

NA_chatMessage::NA_chatMessage(const std::string& i_msg, const std::string &i_me) : NetAction(true) {
  m_msg = i_msg;
  if(i_me != "") {
    ttransform(i_me);
  }
}

NA_chatMessage::NA_chatMessage(void* data, unsigned int len) : NetAction(true) {
  if(len > 0) {
    ((char*)data)[len-1] = '\0';
    m_msg = std::string((char*)data);
    ((char*)data)[len-1] = '\n';

    if(utf8::is_utf8_valid(m_msg) == false) {
      throw Exception("Invalid decoded strings (utf-8)");
    }
  }
}

NA_chatMessage::~NA_chatMessage() {
}

void NA_chatMessage::ttransform(const std::string& i_me) {
  m_msg = replaceAll(m_msg, "/me", i_me);
}

void NA_chatMessage::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  NetAction::send(i_tcpsd, NULL, NULL, NULL, m_msg.c_str(), m_msg.size()); // don't send the \0
}

std::string NA_chatMessage::getMessage() {
  return m_msg;
}

NA_chatMessagePP::NA_chatMessagePP(const std::string& i_msg, const std::string &i_me, const std::vector<int>& i_private_people) : NetAction(true) {
  m_msg            = i_msg;
  m_private_people = i_private_people;
  if(i_me != "") {
    ttransform(i_me);
  };
}

NA_chatMessagePP::NA_chatMessagePP(void* data, unsigned int len) : NetAction(true) {
  unsigned int v_localOffset = 0;
  int v_nb, v_n;
  v_nb = atoi(getLine(((char*)data)+v_localOffset, len-v_localOffset, &v_localOffset).c_str());

  // read people
  for(unsigned int i=0; i<v_nb; i++) {
    v_n = atoi(getLine(((char*)data)+v_localOffset, len-v_localOffset, &v_localOffset).c_str());
    m_private_people.push_back(v_n);
  }

  // read msg
  if(v_localOffset < len-1) {
    ((char*)data)[len-1] = '\0';
    m_msg = std::string(((char*)data)+v_localOffset);
    ((char*)data)[len-1] = '\n';

    if(utf8::is_utf8_valid(m_msg) == false) {
      throw Exception("Invalid decoded strings (utf-8)");
    }
  }
}

NA_chatMessagePP::~NA_chatMessagePP() {
}

void NA_chatMessagePP::ttransform(const std::string& i_me) {
  m_msg = replaceAll(m_msg, "/me", i_me);
}

void NA_chatMessagePP::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  std::ostringstream v_send;
  v_send << m_private_people.size() << "\n";
  for(unsigned int i=0; i<m_private_people.size(); i++) {
    v_send << m_private_people[i] << "\n";
  }
  v_send << m_msg;

  NetAction::send(i_tcpsd, NULL, NULL, NULL, v_send.str().c_str(), v_send.str().size()); // don't send the \0
}

std::string NA_chatMessagePP::getMessage() {
  return m_msg;
}

const std::vector<int>& NA_chatMessagePP::privatePeople() const {
  return m_private_people;
}

NA_serverError::NA_serverError(const std::string& i_msg) : NetAction(true) {
  m_msg = i_msg;
}

NA_serverError::NA_serverError(void* data, unsigned int len) : NetAction(true) {
  if(len > 0) {
    ((char*)data)[len-1] = '\0';
    m_msg = std::string((char*)data);
    ((char*)data)[len-1] = '\n';

    if(utf8::is_utf8_valid(m_msg) == false) {
      throw Exception("Invalid decoded strings (utf-8)");
    }
  }
}

NA_serverError::~NA_serverError() {
}

void NA_serverError::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  NetAction::send(i_tcpsd, NULL, NULL, NULL, m_msg.c_str(), m_msg.size()); // don't send the \0
}

std::string NA_serverError::getMessage() {
  return m_msg;
}

NA_frame::NA_frame(SerializedBikeState* i_state) : NetAction(false) {
  m_state = *i_state;
}

NA_frame::NA_frame(void* data, unsigned int len) : NetAction(false) {
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

NA_udpBind::NA_udpBind(const std::string& i_key) : NetAction(false) {
  m_key = i_key;
}

NA_udpBind::NA_udpBind(void* data, unsigned int len) : NetAction(false) {
  unsigned int v_localOffset = 0;
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

NA_udpBindQuery::NA_udpBindQuery() : NetAction(true) { // force the client send the udpbind ; send in tcp to receive in udp
}

NA_udpBindQuery::NA_udpBindQuery(void* data, unsigned int len) : NetAction(true) { // force the client send the udpbind ; send in tcp to receive in udp
}

NA_udpBindQuery::~NA_udpBindQuery() {
}

void NA_udpBindQuery::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  // force TCP
  NetAction::send(i_tcpsd, NULL, NULL, NULL, NULL, 0);
}

NA_udpBindValidation::NA_udpBindValidation() : NetAction(true) { // tcp to confirm
}

NA_udpBindValidation::NA_udpBindValidation(void* data, unsigned int len) : NetAction(true) { // tcp to confirm
}

NA_udpBindValidation::~NA_udpBindValidation() {
}

void NA_udpBindValidation::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  NetAction::send(i_tcpsd, i_udpsd, i_sendPacket, i_udpRemoteIP, NULL, 0);
}

NA_clientInfos::NA_clientInfos(int i_protocolVersion, const std::string& i_udpBindKey) : NetAction(true) {
  m_protocolVersion = i_protocolVersion;
  m_udpBindKey      = i_udpBindKey;
  m_xmversion       =  XMBuild::getVersionString(true);
}

NA_clientInfos::NA_clientInfos(void* data, unsigned int len) : NetAction(true) {
  unsigned int v_localOffset = 0;
  m_protocolVersion = atoi(getLine(data, len, &v_localOffset).c_str());
  m_udpBindKey      = getLine(((char*)data)+v_localOffset, len-v_localOffset, &v_localOffset);

  /* since 2, client version string */
  if(m_protocolVersion >= 2) {
    m_xmversion = getLineCheckUTF8(((char*)data)+v_localOffset, len-v_localOffset, &v_localOffset);
  }
}

NA_clientInfos::~NA_clientInfos() {
}

void NA_clientInfos::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  std::ostringstream v_send;
  v_send << m_protocolVersion << "\n";
  v_send << m_udpBindKey      << "\n";
  v_send << m_xmversion;

  // force TCP
  NetAction::send(i_tcpsd, NULL, NULL, NULL, v_send.str().c_str(), v_send.str().size()); // don't send the \0
}

int NA_clientInfos::protocolVersion() const {
  return m_protocolVersion;
}

std::string NA_clientInfos::udpBindKey() const {
  return m_udpBindKey;
}

std::string NA_clientInfos::xmversion() const {
  return m_xmversion;
}

NA_changeName::NA_changeName(const std::string& i_name) : NetAction(true) {
  m_name = i_name;
}

NA_changeName::NA_changeName(void* data, unsigned int len) : NetAction(true) {
  unsigned int v_localOffset = 0;
  m_name = getLineCheckUTF8(data, len, &v_localOffset);
}

NA_changeName::~NA_changeName() {
}

void NA_changeName::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  NetAction::send(i_tcpsd, NULL, NULL, NULL, m_name.c_str(), m_name.size()); // don't send the \0
}

std::string NA_changeName::getName() {
  return m_name;
}

NA_clientsNumber::NA_clientsNumber(int i_number) : NetAction(true) {
  m_number = i_number;
}

NA_clientsNumber::NA_clientsNumber(void* data, unsigned int len) : NetAction(true) {
  unsigned int v_localOffset = 0;
  m_number = atoi(getLine(data, len, &v_localOffset).c_str());
}

NA_clientsNumber::~NA_clientsNumber() {
}

void NA_clientsNumber::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  std::ostringstream v_send;
  v_send << m_number << "\n";
  NetAction::send(i_tcpsd, NULL, NULL, NULL, v_send.str().c_str(), v_send.str().size()); // don't send the \0
}

int NA_clientsNumber::getNumber() {
  return m_number;
}

NA_clientsNumberQuery::NA_clientsNumberQuery() : NetAction(true) {
}

NA_clientsNumberQuery::NA_clientsNumberQuery(void* data, unsigned int len) : NetAction(true) {
}

NA_clientsNumberQuery::~NA_clientsNumberQuery() {
}

void NA_clientsNumberQuery::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  NetAction::send(i_tcpsd, NULL, NULL, NULL, NULL, 0);
}

NA_playingLevel::NA_playingLevel(const std::string& i_levelId) : NetAction(true) {
  m_levelId = i_levelId;
}

NA_playingLevel::NA_playingLevel(void* data, unsigned int len) : NetAction(true) {
  unsigned int v_localOffset = 0;
  m_levelId = getLineCheckUTF8(data, len, &v_localOffset);
}

NA_playingLevel::~NA_playingLevel() {
}

void NA_playingLevel::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  NetAction::send(i_tcpsd, NULL, NULL, NULL, m_levelId.c_str(), m_levelId.size()); // don't send the \0
}

std::string NA_playingLevel::getLevelId() {
  return m_levelId;
}

NA_changeClients::NA_changeClients() : NetAction(true) {
}

NA_changeClients::NA_changeClients(void* data, unsigned int len) : NetAction(true) {
  unsigned int v_localOffset = 0;
  std::string v_symbol;
  NetInfosClient v_infosClient;

  if(len == 1) { // no client
    return;
  }

  while(v_localOffset < len) {
    v_symbol = getLine(((char*)data)+v_localOffset, len-v_localOffset, &v_localOffset);
    v_infosClient.NetId = atoi(getLine(((char*)data)+v_localOffset, len-v_localOffset, &v_localOffset).c_str());

    if(v_symbol == "+") {
      v_infosClient.Name  = getLineCheckUTF8(((char*)data)+v_localOffset, len-v_localOffset, &v_localOffset);
      m_netAddedInfosClients.push_back(v_infosClient);
    } else if(v_symbol == "-") {
      m_netRemovedInfosClients.push_back(v_infosClient);
    }
  }
}

NA_changeClients::~NA_changeClients() {
}

void NA_changeClients::add(NetInfosClient* i_infoClient) {
  m_netAddedInfosClients.push_back(*i_infoClient);
}

void NA_changeClients::remove(NetInfosClient* i_infoClient) {
  m_netRemovedInfosClients.push_back(*i_infoClient);
}

void NA_changeClients::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  std::ostringstream v_send;

  for(unsigned int i=0; i<m_netAddedInfosClients.size(); i++) {
    v_send << "+" << "\n";
    v_send << m_netAddedInfosClients[i].NetId << "\n";
    v_send << m_netAddedInfosClients[i].Name  << "\n";
  }
  
  for(unsigned int i=0; i<m_netRemovedInfosClients.size(); i++) {
    v_send << "-" << "\n";
    v_send << m_netRemovedInfosClients[i].NetId << "\n";
  }

  if(m_netAddedInfosClients.size() > 0 || m_netRemovedInfosClients.size() > 0) {
    NetAction::send(i_tcpsd, NULL, NULL, NULL, v_send.str().c_str(), v_send.str().size()-1); // don't send the \0 and the last \n
  } else {
    NetAction::send(i_tcpsd, NULL, NULL, NULL, v_send.str().c_str(), v_send.str().size()); // don't send the \0
  }
}

const std::vector<NetInfosClient>& NA_changeClients::getAddedInfosClients() const {
  return m_netAddedInfosClients;
}

const std::vector<NetInfosClient>& NA_changeClients::getRemovedInfosClients() const {
  return m_netRemovedInfosClients;
}

NA_playerControl::NA_playerControl(PlayerControl i_control, float i_value) : NetAction(false) {
  m_control = i_control;
  m_value = i_value;
}

NA_playerControl::NA_playerControl(PlayerControl i_control, bool i_value) : NetAction(false) {
  m_control = i_control;
  m_value = i_value ? 0.5 : -0.5; // negativ or positiv
}

NA_playerControl::NA_playerControl(void* data, unsigned int len) : NetAction(false) {
  unsigned int v_localOffset = 0;

  m_control = (PlayerControl) atoi(getLine(data, len, &v_localOffset).c_str());
  if(PlayerControl_isValid(m_control) == false) {
    throw Exception("Invalid player control");
  }

  if(len-v_localOffset-1 != 4) {
    throw Exception("Invalid player control");
  }
  
  m_value = SwapEndian::read4LFloat(((char*)data) + v_localOffset);

  // don't allow value not between -1 and 1, but don't reject the client if not because float can be sometimes surprising
  if(m_value < -1.0) m_value = -1.0;
  if(m_value >  1.0) m_value =  1.0;
}

NA_playerControl::~NA_playerControl() {
}

void NA_playerControl::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  char buf[7];
  snprintf(buf, 3, "%i\n", (int)(m_control));
  SwapEndian::write4LFloat(buf+2, m_value);
  NetAction::send(i_tcpsd, i_udpsd, i_sendPacket, i_udpRemoteIP, buf, 6); // don't send the \0
}

PlayerControl NA_playerControl::getType() {
  return m_control;
}

float NA_playerControl::getFloatValue() {
  return m_value;
}

bool NA_playerControl::getBoolValue() {
  return m_value > 0.0;
}

NA_clientMode::NA_clientMode(NetClientMode i_mode) : NetAction(true) {
  m_mode = i_mode;
}

NA_clientMode::NA_clientMode(void* data, unsigned int len) : NetAction(true) {
  unsigned int v_localOffset = 0;
  m_mode = (NetClientMode) (atoi(getLine(data, len, &v_localOffset).c_str()));
}

NA_clientMode::~NA_clientMode() {
}

void NA_clientMode::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  std::ostringstream v_send;
  v_send << m_mode;

  // force TCP
  NetAction::send(i_tcpsd, NULL, NULL, NULL, v_send.str().c_str(), v_send.str().size()); // don't send the \0
}

NetClientMode NA_clientMode::mode() const {
  return m_mode;
}

NA_prepareToPlay::NA_prepareToPlay(const std::string& i_id_level, std::vector<int>& i_players) : NetAction(true) {
  m_id_level = i_id_level;

  // copy the vector
  m_players = i_players;
}

NA_prepareToPlay::NA_prepareToPlay(void* data, unsigned int len) : NetAction(true) {
  unsigned int v_localOffset = 0;
  unsigned int v_nplayers;

  m_id_level = getLineCheckUTF8(((char*)data)+v_localOffset, len-v_localOffset, &v_localOffset);
  v_nplayers = atoi(getLine(((char*)data)+v_localOffset, len-v_localOffset, &v_localOffset).c_str());
  for(unsigned int i=0; i<v_nplayers; i++) {
    m_players.push_back(atoi(getLine(((char*)data)+v_localOffset, len-v_localOffset, &v_localOffset).c_str()));
  }
}

NA_prepareToPlay::~NA_prepareToPlay() {
}

void NA_prepareToPlay::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  std::ostringstream v_send;
  v_send << m_id_level       << "\n";
  v_send << m_players.size() << "\n";
  for(unsigned int i=0; i<m_players.size(); i++) {
    v_send << m_players[i]   << "\n";
  }
  NetAction::send(i_tcpsd, NULL, NULL, NULL, v_send.str().c_str(), v_send.str().size()); // don't send the \0
}

std::string NA_prepareToPlay::idLevel() const {
  return m_id_level;
}

const std::vector<int>& NA_prepareToPlay::players() {
  return m_players;
}

NA_killAlert::NA_killAlert(int i_time) : NetAction(false) {
  m_time = i_time;
}

NA_killAlert::NA_killAlert(void* data, unsigned int len) : NetAction(false) {
  unsigned int v_localOffset = 0;
  m_time = atoi(getLine(data, len, &v_localOffset).c_str());
}

NA_killAlert::~NA_killAlert() {
}

void NA_killAlert::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  std::ostringstream v_send;
  v_send << m_time;

  // force TCP
  NetAction::send(i_tcpsd, NULL, NULL, NULL, v_send.str().c_str(), v_send.str().size()); // don't send the \0
}

int NA_killAlert::time() const {
  return m_time;
}

NA_prepareToGo::NA_prepareToGo(int i_time) : NetAction(true) {
  m_time = i_time;
}

NA_prepareToGo::NA_prepareToGo(void* data, unsigned int len) : NetAction(true) {
  unsigned int v_localOffset = 0;
  m_time = atoi(getLine(data, len, &v_localOffset).c_str());
}

NA_prepareToGo::~NA_prepareToGo() {
}

void NA_prepareToGo::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  std::ostringstream v_send;
  v_send << m_time;
  
  // force TCP
  NetAction::send(i_tcpsd, NULL, NULL, NULL, v_send.str().c_str(), v_send.str().size()); // don't send the \0
}

int NA_prepareToGo::time() const {
  return m_time;
}

NA_gameEvents::NA_gameEvents(DBuffer* i_buffer) : NetAction(true) {
  m_bufferLength = i_buffer->copyTo(m_buffer, XM_NET_MAX_EVENTS_SHOT_SIZE);
}

NA_gameEvents::NA_gameEvents(void* data, unsigned int len) : NetAction(true) {
  m_bufferLength = len-1;
  memcpy(m_buffer, data, len-1);
}

NA_gameEvents::~NA_gameEvents() {
}

void NA_gameEvents::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  // force TCP
  NetAction::send(i_tcpsd, NULL, NULL, NULL, m_buffer, m_bufferLength); // don't send the \0
}

char* NA_gameEvents::buffer() {
  return m_buffer;
}

int NA_gameEvents::bufferSize() {
  return m_bufferLength;
}

NA_srvCmd::NA_srvCmd(const std::string& i_cmd) : NetAction(true) {
  m_cmd = i_cmd;
}

NA_srvCmd::NA_srvCmd(void* data, unsigned int len) : NetAction(true) {
  if(len > 0) {
    ((char*)data)[len-1] = '\0';
    m_cmd = std::string((char*)data);
    ((char*)data)[len-1] = '\n';

    if(utf8::is_utf8_valid(m_cmd) == false) {
      throw Exception("Invalid decoded strings (utf-8)");
    }
  }
}

NA_srvCmd::~NA_srvCmd() {
}

void NA_srvCmd::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  NetAction::send(i_tcpsd, NULL, NULL, NULL, m_cmd.c_str(), m_cmd.size()); // don't send the \0
}

std::string NA_srvCmd::getCommand() {
  return m_cmd;
}

NA_srvCmdAsw::NA_srvCmdAsw(const std::string& i_answer) : NetAction(true) {
  m_answer = i_answer;
}

NA_srvCmdAsw::NA_srvCmdAsw(void* data, unsigned int len) : NetAction(true) {
  if(len > 0) {
    ((char*)data)[len-1] = '\0';
    m_answer = std::string((char*)data);
    ((char*)data)[len-1] = '\n';

    if(utf8::is_utf8_valid(m_answer) == false) {
      throw Exception("Invalid decoded strings (utf-8)");
    }
  }
}

NA_srvCmdAsw::~NA_srvCmdAsw() {
}

void NA_srvCmdAsw::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  NetAction::send(i_tcpsd, NULL, NULL, NULL, m_answer.c_str(), m_answer.size()); // don't send the \0
}

std::string NA_srvCmdAsw::getAnswer() {
  return m_answer;
}

NA_slaveClientsPoints::NA_slaveClientsPoints() : NetAction(true) {
}

NA_slaveClientsPoints::NA_slaveClientsPoints(void* data, unsigned int len) : NetAction(true) {
  unsigned int v_localOffset = 0;
  NetPointsClient v_pointsClient;

  if(len == 1) { // no client (only \n)
    return;
  }

  while(v_localOffset < len) {
    v_pointsClient.NetId  = atoi(getLine(((char*)data)+v_localOffset, len-v_localOffset, &v_localOffset).c_str());
    v_pointsClient.Points = atoi(getLine(((char*)data)+v_localOffset, len-v_localOffset, &v_localOffset).c_str());
    m_netPointsClients.push_back(v_pointsClient);
  }
}

NA_slaveClientsPoints::~NA_slaveClientsPoints() {
}

void NA_slaveClientsPoints::send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP) {
  std::ostringstream v_send;

  for(unsigned int i=0; i<m_netPointsClients.size(); i++) {
    v_send << m_netPointsClients[i].NetId  << "\n";
    v_send << m_netPointsClients[i].Points << "\n";
  }

  if(m_netPointsClients.size() > 0) {
    NetAction::send(i_tcpsd, NULL, NULL, NULL, v_send.str().c_str(), v_send.str().size()-1); // don't send the \0 and the last \n
  } else {
    NetAction::send(i_tcpsd, NULL, NULL, NULL, v_send.str().c_str(), v_send.str().size()); // don't send the \0
  }
}

const std::vector<NetPointsClient>& NA_slaveClientsPoints::getPointsClients() const {
  return m_netPointsClients;
}

void NA_slaveClientsPoints::add(NetPointsClient* i_npc) {
  m_netPointsClients.push_back(*i_npc);
}
