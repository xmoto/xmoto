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
#include "../SysMessage.h"
#include "../helpers/VExcept.h"
#include <sstream>

std::string NA_chatMessage::ActionKey = "message";

NetAction::NetAction() {
}

NetAction::~NetAction() {
}

void NetAction::send(TCPsocket* i_sd, const void* subPacketData, int subPacketLen) {
  unsigned int nread;
  std::string v_data;
  
  int v_subPacketSize = actionKey().length() + 1 + subPacketLen + 1;
  std::ostringstream v_nb;
  v_nb << v_subPacketSize;

  v_data = v_nb.str() + "\n" + actionKey() + "\n" + std::string((char*)subPacketData) + "\n";

  // don't send the \0
  if( (nread = SDLNet_TCP_Send(*i_sd, v_data.c_str(), v_data.size())) != v_data.size()) {
    throw Exception("TCP_Send failed");
  }
}

NetAction* NetAction::newNetAction(void* data, unsigned int len) {
  if(isCommand(data, len, NA_chatMessage::ActionKey)) {
    return new NA_chatMessage(((char*)data)+8, len-8);
  } else {
    throw Exception("client: invalid command");
  }
}

bool NetAction::isCommand(void* data, unsigned int len, const std::string& i_cmd) {
  if(i_cmd.length() + 1 > len) {
    return false;
  }

  if(((char*) data)[i_cmd.length()] != '\n') {
    return false;
  }

  return strncmp((char*)data, i_cmd.c_str(), i_cmd.length()) == 0;
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

void NA_chatMessage::execute() {
  SysMessage::instance()->displayInformation(m_msg);
}

void NA_chatMessage::send(TCPsocket* i_sd) {
  NetAction::send(i_sd, m_msg.c_str(), m_msg.size()); // don't send the \0
}

std::string NA_chatMessage::getMessage() {
  return m_msg;
}
