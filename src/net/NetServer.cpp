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

#include "NetServer.h"
#include "thread/ServerThread.h"
#include "helpers/VExcept.h"
#include "helpers/Log.h"
#include "common/XMSession.h"
#include "NetClient.h"

NetServer::NetServer() {
    m_isStarted = false;
    m_serverThread = NULL;
}
 
NetServer::~NetServer() {
}

void NetServer::start(bool i_deamon, int i_port, const std::string& i_adminPassword) {
  m_serverThread = new ServerThread("NETSERVER", i_port, i_adminPassword);

  if(i_deamon) {
    m_serverThread->startThread();
    m_isStarted = true;
  } else {
    if(m_serverThread->runInMain() != 0) {
      throw Exception("Server thread failed");
    }
  }
}

void NetServer::stop() {
  if(m_serverThread->isThreadRunning()) {
    m_serverThread->askThreadToEnd();
    // the server can be sleeping. send him a packet to wake it up, and stop it
    try {
      NetClient::fastConnectDisconnect("localhost", m_serverThread->port());
    } catch(Exception &e) {
      LogWarning(e.getMsg().c_str());
    }
    m_serverThread->waitForThreadEnd();
  }

  // if the server is badly closed (ie bad event like ctrl+c broke the thread), clean connexions
  if(m_serverThread->acceptConnections()) {
    m_serverThread->close();
  }

  delete m_serverThread;
  m_serverThread = NULL;

  m_isStarted = false;
}

bool NetServer::isStarted() {
  // check that the server has not finished
  if(m_isStarted) {
    if(m_serverThread->isThreadRunning() == false) {
      stop();
    }
  }
  return m_isStarted;
}

void NetServer::wait() {
  if(m_isStarted) {
    if(m_serverThread->waitForThreadEnd() != 0) {
      throw Exception("Server ended with an error");
    }
  }
}

bool NetServer::acceptConnections() {
  if(isStarted() == false) {
    return false;
  }
  return m_serverThread->acceptConnections();
}

void NetServer::setStandAloneOptions() {
  XMSession::instance()->setEnableAudio(false);
}
