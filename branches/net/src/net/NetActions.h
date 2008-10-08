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

#ifndef __NETACTIONS_H__
#define __NETACTIONS_H__

#include <string>
#include <SDL_net.h>
#include "../xmscene/BasicSceneStructs.h"

#define NETACTION_MAX_PACKET_SIZE 1024 * 10 // bytes

class NetClient;

class NetAction {
  public:
  NetAction();
  virtual ~NetAction();
  virtual std::string actionKey() = 0;

  virtual void execute(NetClient* i_netClient) {};
  virtual void send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP);

  static NetAction* newNetAction(void* data, unsigned int len);
  static void logStats();

  protected:
  void send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP, const void* subPacketData, int subPacketLen);

  private:
  static bool isCommand(void* data, unsigned int len, const std::string& i_cmd);
  static char m_buffer[NETACTION_MAX_PACKET_SIZE];
  static unsigned int m_biggestTCPPacket;
  static unsigned int m_biggestUDPPacket;
  static unsigned int m_nbTCPPacketsSent;
  static unsigned int m_nbUDPPacketsSent;
  static unsigned int m_TCPPacketsSizeSent;
  static unsigned int m_UDPPacketsSizeSent;
};

class NA_udpBind : public NetAction {
  public:
  NA_udpBind(const std::string& i_key);
  NA_udpBind(void* data, unsigned int len);
  virtual ~NA_udpBind();
  std::string actionKey() { return ActionKey; };
  static std::string ActionKey;

  void send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP);

  std::string key() const;

  private:
  std::string m_key;
};

class NA_udpBindQuery : public NetAction {
  public:
  NA_udpBindQuery();
  NA_udpBindQuery(void* data, unsigned int len);
  virtual ~NA_udpBindQuery();
  std::string actionKey() { return ActionKey; };
  static std::string ActionKey;
  
  virtual void execute(NetClient* i_netClient);
  void send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP);

  private:
};

/* the client send its key by tcp (udpBindKey) */
/* then, the server ask via tcp (udpBindQuery) the client to send an udp packet (udpBind) */
/* the server bind */

class NA_udpBindKey : public NetAction {
  public:
  NA_udpBindKey(const std::string& i_key);
  NA_udpBindKey(void* data, unsigned int len);
  virtual ~NA_udpBindKey();
  std::string actionKey() { return ActionKey; };
  static std::string ActionKey;

  void send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP);

  std::string key() const;

  private:
  std::string m_key;
};

class NA_chatMessage : public NetAction {
  public:
  NA_chatMessage(const std::string& i_msg);
  NA_chatMessage(void* data, unsigned int len);
  virtual ~NA_chatMessage();
  std::string actionKey() { return ActionKey; };
  static std::string ActionKey;

  virtual void execute(NetClient* i_netClient);
  void send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP);

  std::string getMessage();

  private:
  std::string m_msg;
};

class NA_frame : public NetAction {
  public:
  NA_frame(SerializedBikeState* i_state);
  NA_frame(void* data, unsigned int len);
  virtual ~NA_frame();
  std::string actionKey() { return ActionKey; };
  static std::string ActionKey;

  virtual void execute(NetClient* i_netClient);
  void send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP);

  SerializedBikeState getState();

  private:
   SerializedBikeState m_state;
};

class NA_presentation : public NetAction {
  public:
  NA_presentation(const std::string& i_name);
  NA_presentation(void* data, unsigned int len);
  virtual ~NA_presentation();
  std::string actionKey() { return ActionKey; };
  static std::string ActionKey;

  void send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP);

  std::string getName();

  private:
  std::string m_name;
};

class NA_startingLevel : public NetAction {
  public:
  NA_startingLevel(const std::string& i_levelId);
  NA_startingLevel(void* data, unsigned int len);
  virtual ~NA_startingLevel();
  std::string actionKey() { return ActionKey; };
  static std::string ActionKey;

  void send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP);
  virtual void execute(NetClient* i_netClient);

  std::string getLevelId();

  private:
  std::string m_levelId;
};

#endif
