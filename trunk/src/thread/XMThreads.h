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

#ifndef __XMTHREADS_H__
#define __XMTHREADS_H__

#include <string>
#include <map>

class XMThread;

class XMThreads {
 public:
  XMThreads();
  virtual ~XMThreads();

  void addThread(std::string threadId, XMThread* pThread);
  // delete the XMThread
  void removeThread(std::string threadId);
  XMThread* getThread(std::string threadId);

  int getNumberThreads();
  int getNumberRunningThreads();

 protected:
  std::map<std::string, XMThread*> m_threads;
};

#endif
