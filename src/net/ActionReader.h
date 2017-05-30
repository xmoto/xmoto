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

#ifndef __ACTIONREADER_H__
#define __ACTIONREADER_H__

#include "../include/xm_SDL_net.h"
#include <string>

#define XM_MAX_PACKET_SIZE 1024 * 10 // bytes

class NetAction;
struct NetActionU;

class ActionReader {
public:
  ActionReader();
  ~ActionReader();

  // return true if a packet is returned ; you should reread to get more action
  // (socket in not read while true is returned)
  bool TCPReadAction(TCPsocket *i_tcpsd, NetActionU *o_netAction);
  static void UDPReadAction(Uint8 *data, int len, NetActionU *o_netAction);

  static void logStats();

  /* stats */
  static unsigned int m_biggestTCPPacketReceived;
  static unsigned int m_biggestUDPPacketReceived;
  static unsigned int m_nbTCPPacketsReceived;
  static unsigned int m_nbUDPPacketsReceived;
  static unsigned int m_TCPPacketsSizeReceived;
  static unsigned int m_UDPPacketsSizeReceived;
  /* ***** */

private:
  unsigned int m_tcpPacketOffset;
  bool m_tcpNotEnoughData;
  char m_tcpBuffer[XM_MAX_PACKET_SIZE];
  bool m_tcpPossiblyInBuffer; // an action is possibly in the buffer

  static unsigned int getSubPacketSize(void *data,
                                       unsigned int len,
                                       unsigned int &o_cmdStart);
};

#endif
