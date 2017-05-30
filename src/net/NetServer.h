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

#ifndef __XMNETSERVER_H__
#define __XMNETSERVER_H__

#include "../helpers/Singleton.h"
#include <string>

class ServerThread;

class NetServer : public Singleton<NetServer> {
public:
  NetServer();
  ~NetServer();

  void start(bool i_deamon,
             int i_port,
             const std::string &i_adminPassword = "");
  void stop();
  bool isStarted();
  void wait();
  bool acceptConnections();
  void setStandAloneOptions();

private:
  ServerThread *m_serverThread;
  bool m_isStarted;
};

#endif
