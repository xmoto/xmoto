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

#include "ActionReader.h"
#include "helpers/Log.h"
#include "helpers/VExcept.h"
#include "common/XMSession.h"
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include "NetActions.h"
#include "helpers/Net.h"

#define XM_MAX_PACKET_SIZE_DIGITS 6 // limit the size of a command : n digits

unsigned int ActionReader::m_biggestTCPPacketReceived = 0;
unsigned int ActionReader::m_biggestUDPPacketReceived = 0;
unsigned int ActionReader::m_nbTCPPacketsReceived     = 0;
unsigned int ActionReader::m_nbUDPPacketsReceived     = 0;
unsigned int ActionReader::m_TCPPacketsSizeReceived   = 0;
unsigned int ActionReader::m_UDPPacketsSizeReceived   = 0;

ActionReader::ActionReader() {
    m_tcpPacketOffset     = 0;
    m_tcpNotEnoughData    = false;
    m_tcpPossiblyInBuffer = false;
}

ActionReader::~ActionReader() {
}

/* log version */
void ActionReader::logStats() {
  LogInfo("%-36s : %u", "net: number of TCP packets received", ActionReader::m_nbTCPPacketsReceived);
  LogInfo("%-36s : %s", "net: biggest TCP packet received"   , XMNet::getFancyBytes(ActionReader::m_biggestTCPPacketReceived).c_str());
  LogInfo("%-36s : %s", "net: size of TCP packets received"  , XMNet::getFancyBytes(ActionReader::m_TCPPacketsSizeReceived).c_str());
  LogInfo("%-36s : %u", "net: number of UDP packets received", ActionReader::m_nbUDPPacketsReceived);
  LogInfo("%-36s : %s", "net: biggest UDP packet received"   , XMNet::getFancyBytes(ActionReader::m_biggestUDPPacketReceived).c_str());
  LogInfo("%-36s : %s", "net: size of UDP packets received"  , XMNet::getFancyBytes(ActionReader::m_UDPPacketsSizeReceived).c_str());
}

bool ActionReader::TCPReadAction(TCPsocket* i_tcpsd, NetActionU* o_netAction) {
  int nread;
  unsigned int v_cmdStart;
  unsigned int v_packetSize;

  if(m_tcpPossiblyInBuffer == false) { // don't read from the socket if there is possibly still packets in the buffer
    if( (nread = SDLNet_TCP_Recv(*(i_tcpsd),
				 m_tcpBuffer+m_tcpPacketOffset,
				 XM_MAX_PACKET_SIZE-m_tcpPacketOffset)) <= 0) {
      throw DisconnectedException();
    }
    m_tcpPacketOffset += nread;
    LogDebug("Data received (%u bytes available)", m_tcpPacketOffset);
    m_tcpNotEnoughData = false; // you don't know if the buffer is full enough

    if(nread > ActionReader::m_biggestTCPPacketReceived) {
      ActionReader::m_biggestTCPPacketReceived = nread;
    }
    ActionReader::m_nbTCPPacketsReceived++;
    ActionReader::m_TCPPacketsSizeReceived += nread;
  }

  m_tcpPossiblyInBuffer = false;
  
  while( (v_packetSize = getSubPacketSize(m_tcpBuffer, m_tcpPacketOffset, v_cmdStart)) > 0 && m_tcpNotEnoughData == false) {
    LogDebug("Packet size is %u", v_packetSize);
    try {
      if(m_tcpPacketOffset-v_cmdStart >= v_packetSize) {
	LogDebug("One packet to manage");
	NetAction::getNetAction(o_netAction, ((char*)m_tcpBuffer)+v_cmdStart, v_packetSize);
	
	// remove the managed packet
	// main case : the buffer contains exactly one command
	if(m_tcpPacketOffset-v_cmdStart == v_packetSize) {
	  m_tcpPacketOffset = 0;
	} else {
	  // the buffer contains two or more commands
	  // remove the packet
	  m_tcpPacketOffset = m_tcpPacketOffset - v_cmdStart - v_packetSize;
	  memcpy(m_tcpBuffer, m_tcpBuffer+v_cmdStart+v_packetSize, m_tcpPacketOffset);
	}
	LogDebug("Packet offset set to %u", m_tcpPacketOffset);
	m_tcpPossiblyInBuffer = true;
	return true;
      } else {
	// wait for more data to have a full command
	LogDebug("Not enough data to make a packet");
	m_tcpNotEnoughData = true;
      }
    } catch(Exception &e) {
      LogError("net: bad command received (%s)", e.getMsg().c_str());
      throw Exception("Invalid command");
    }
  }

  return false; // no more action readable
}

// return the size of the packet or 0 if no packet is available
unsigned int ActionReader::getSubPacketSize(void* data, unsigned int len, unsigned int& o_cmdStart) {
  unsigned int i=0;
  unsigned int res;

  while(i<len && i<XM_MAX_PACKET_SIZE_DIGITS+1) {
    if(((char*)data)[i] == '\n') {
      o_cmdStart = i+1;
      ((char*)data)[i] = '\0';
      res = atoi((char*)data);
      ((char*)data)[i] = '\n'; // must be reset in case packet is not full

      if(res == 0) {
	Logger::LogData(data, len);
	LogWarning("net: nasty client detected (1)");
	throw Exception("net: nasty client detected");
      }

      return res;
    }
    i++;
  }

  if(i == XM_MAX_PACKET_SIZE_DIGITS+1) {
    Logger::LogData(data, len);
    LogWarning("net: nasty client detected (2)");
    throw Exception("net: nasty client detected");
  }

  return 0;
}

void ActionReader::UDPReadAction(Uint8* data, int len, NetActionU* o_netAction) {
  unsigned int v_size;
  unsigned int v_cmdStart;

  if(len > ActionReader::m_biggestUDPPacketReceived) {
    ActionReader::m_biggestUDPPacketReceived = len;
  }
  ActionReader::m_nbUDPPacketsReceived++;
  ActionReader::m_UDPPacketsSizeReceived += len;

  if( (v_size = ActionReader::getSubPacketSize(data, len, v_cmdStart)) > 0) {
    NetAction::getNetAction(o_netAction, ((char*)data)+v_cmdStart, v_size);
  } else {
    throw Exception("net: nasty client detected (3)");
  }
}
