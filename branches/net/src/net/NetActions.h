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

#define NETACTION_MAX_PACKET_SIZE 1024 * 2 // bytes

class NetClient;
class ServerThread;

enum NetActionType {
  TNA_udpBind,
  TNA_udpBindQuery,
  TNA_udpBindKey,
  TNA_chatMessage,
  TNA_frame,
  TNA_presentation,
  TNA_playingLevel
};

class NetAction {
  public:
  NetAction();
  virtual ~NetAction();
  virtual std::string actionKey()    = 0;
  virtual NetActionType actionType() = 0;

  virtual void send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP);
  void setSource(int i_src, int i_subsrc);

  int getSource() const;
  int getSubSource() const;

  static NetAction* newNetAction(void* data, unsigned int len);
  static void logStats();

  protected:
  void send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP, const void* subPacketData, int subPacketLen);

  int m_source;    // source client
  int m_subsource; // a client can have several players : 0, 1, 2 or 3
  // the client sending a packet :
  // => (-1, [0,1,2,3])
  // the client receiving a packet from an other client
  // => (x, [0,1,2,3])
  // the client receiving a packet from the server
  // => (-1, 0)
  // the serveur sending a packet to a client
  // => (-1, 0)
  // the server transfering a packet from a client x to others clients
  // => (x, [0,1,2,3])

  private:
  static std::string getLine(void* data, unsigned int len, unsigned int* v_local_offset);
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
  std::string actionKey()    { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

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
  std::string actionKey()    { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

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
  std::string actionKey()    { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

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
  std::string actionKey()    { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

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
  std::string actionKey()    { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP);

  SerializedBikeState* getState();

  private:
   SerializedBikeState m_state;
};

class NA_presentation : public NetAction {
  public:
  NA_presentation(const std::string& i_name);
  NA_presentation(void* data, unsigned int len);
  virtual ~NA_presentation();
  std::string actionKey()    { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP);

  std::string getName();

  private:
  std::string m_name;
};

class NA_playingLevel : public NetAction {
  public:
  NA_playingLevel(const std::string& i_levelId);
  NA_playingLevel(void* data, unsigned int len);
  virtual ~NA_playingLevel();
  std::string actionKey()    { return ActionKey; }
  NetActionType actionType() { return NAType; }
  static std::string ActionKey;
  static NetActionType NAType;

  void send(TCPsocket* i_tcpsd, UDPsocket* i_udpsd, UDPpacket* i_sendPacket, IPaddress* i_udpRemoteIP);

  std::string getLevelId();

  private:
  std::string m_levelId;
};

#endif
