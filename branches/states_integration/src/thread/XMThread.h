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

#ifndef __XMTHREAD_H__
#define __XMTHREAD_H__

#include <string>

struct SDL_Thread;
struct SDL_mutex;
class GameApp;
class xmDatabase;

class XMThread {
public:
  XMThread();
  virtual ~XMThread();

  void startThread(GameApp* pGame);
  int  waitForThreadEnd();
  bool isThreadRunning();
  // use with care
  void killThread();

  // return a value between 0-100
  int getThreadProgress();
  std::string getThreadCurrentOperation();
  std::string getThreadCurrentMicroOperation();

  // don't use it
  static int run(void* pThreadInstance);

protected:
  virtual int realThreadFunction() = 0;

  void setThreadProgress(int progress);
  void setThreadCurrentOperation(std::string curOp);
  void setThreadCurrentMicroOperation(std::string curMicOp);

  SDL_Thread* m_pThread;
  bool        m_isRunning;

  int         m_progress;
  std::string m_currentOperation;
  // for example, the name of the level beeing downloaded
  std::string m_currentMicroOperation;

  GameApp*    m_pGame;
  // different thread, different database connection
  xmDatabase* m_pDb;

private:
  int threadFunctionEncapsulate();
  SDL_mutex* m_progressMutex;
  SDL_mutex* m_curOpMutex;
  SDL_mutex* m_curMicOpMutex;
};
  
#endif
