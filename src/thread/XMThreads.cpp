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

#include "XMThreads.h"
#include "XMThread.h"

XMThreads::XMThreads() {}

XMThreads::~XMThreads() {
  // emergency stop.
  std::map<std::string, XMThread *>::iterator iter;
  iter = m_threads.begin();

  while (iter != m_threads.end()) {
    iter->second->askThreadToEnd();
    iter->second->waitForThreadEnd();
    delete iter->second;

    iter++;
  }

  m_threads.clear();
}

void XMThreads::addThread(std::string threadId, XMThread *pThread) {
  m_threads[threadId] = pThread;
}

void XMThreads::removeThread(std::string threadId) {
  XMThread *pThread = m_threads[threadId];
  delete pThread;
  m_threads.erase(threadId);
}

XMThread *XMThreads::getThread(std::string threadId) {
  return m_threads[threadId];
}

int XMThreads::getNumberThreads() {
  return m_threads.size();
}

int XMThreads::getNumberRunningThreads() {
  int nb = 0;

  std::map<std::string, XMThread *>::iterator iter;
  iter = m_threads.begin();

  while (iter != m_threads.end()) {
    if (iter->second->isThreadRunning() == true) {
      nb++;
    }

    iter++;
  }

  return nb;
}
