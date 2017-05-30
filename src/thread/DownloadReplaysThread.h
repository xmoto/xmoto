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

#ifndef __DOWNLOADREPLAYSTHREAD_H__
#define __DOWNLOADREPLAYSTHREAD_H__

#include "XMThread.h"
#include <vector>

class StateManager;
class WebRoom;

class DownloadReplaysThread : public XMThread {
public:
  DownloadReplaysThread(StateManager *i_manager);
  virtual ~DownloadReplaysThread();

  void add(const std::string i_url);
  void doJob();

  int realThreadFunction();

private:
  void play();

  WebRoom *m_pWebRoom;
  SDL_mutex *m_urlsMutex;
  StateManager *m_manager; // for the communication
  std::vector<std::string> m_replaysUrls;
};

#endif
