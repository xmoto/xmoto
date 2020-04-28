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
struct SDL_cond;

class GameApp;
class xmDatabase;

/**
 * thread mother class for X-Moto. This encapsulates the SDL threads functions
 * and add more stuff for XM.
 */
class XMThread {
public:
  XMThread(const std::string &i_dbKey = "thread", bool i_dbReadOnly = false);
  virtual ~XMThread();

  void startThread();
  int runInMain();

  int waitForThreadEnd();
  bool isThreadRunning();
  virtual void askThreadToEnd();
  // use with care
  void killThread();

  void askThreadToSleep();
  // you can unsleep a thread and give him an info about
  // what happens during its sleep
  void unsleepThread(std::string infos = "");

  // return a value between 0-100
  int getThreadProgress();
  std::string getThreadCurrentOperation();
  std::string getThreadCurrentMicroOperation();

  // don't use it
  static int run(void *pThreadInstance);
  virtual int realThreadFunction() = 0;

  /**
   * @brief ask to kill the thread as soon as it is in a safe state
   */
  void safeKill();

protected:
  void sleepThread();

  void setThreadProgress(int progress);
  void setThreadCurrentOperation(std::string curOp);
  void setThreadCurrentMicroOperation(std::string curMicOp);

  void setSafeKill(bool i_value);

  SDL_Thread *m_pThread;
  bool m_isRunning;
  bool m_isSleeping;
  bool m_askThreadToEnd;
  bool m_askThreadToSleep;

  int m_progress;
  std::string m_currentOperation;
  // for example, the name of the level beeing downloaded
  std::string m_currentMicroOperation;

  std::string m_wakeUpInfos;

  // different thread, different database connection
  xmDatabase *m_pDb;

private:
  int threadFunctionEncapsulate();
  SDL_mutex *m_curOpMutex;
  SDL_mutex *m_curMicOpMutex;
  SDL_mutex *m_sleepMutex;
  SDL_cond *m_sleepCond;
  bool m_safeKill;
  bool m_askSafeKill;
  std::string m_dbKey;
  bool m_dbReadOnly;
};

#endif
