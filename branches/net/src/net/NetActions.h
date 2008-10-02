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

class NetAction {
  public:
  NetAction();
  virtual ~NetAction();
  virtual std::string actionKey() = 0;

  virtual void execute() = 0;
  virtual void send(TCPsocket* i_sd) = 0;

  static NetAction* newNetAction(void* data, unsigned int len);

  protected:
  void send(TCPsocket* i_sd, const void* subPacketData, int subPacketLen);

  private:
  static bool isCommand(void* data, unsigned int len, const std::string& i_cmd);
};

class NA_chatMessage : public NetAction {
  public:
  NA_chatMessage(const std::string& i_msg);
  NA_chatMessage(void* data, unsigned int len);
  virtual ~NA_chatMessage();
  std::string actionKey() { return ActionKey; };
  static std::string ActionKey;

  virtual void execute();
  void send(TCPsocket* i_sd);

  std::string getMessage();

  private:
  std::string m_msg;
};

#endif
