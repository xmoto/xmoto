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

#include "XMThread.h"
#include "common/VCommon.h"
#include "common/VFileIO.h"
#include "helpers/Log.h"
#include "xmoto/Game.h"

XMThread::XMThread(const std::string &i_dbKey, bool i_dbReadOnly) {
  m_isRunning = false;
  m_isSleeping = false;
  m_askThreadToEnd = false;
  m_askThreadToSleep = false;
  m_pThread = NULL;
  m_progress = 0;
  m_currentOperation = "";
  m_currentMicroOperation = "";
  m_pDb = NULL;
  m_curOpMutex = SDL_CreateMutex();
  m_curMicOpMutex = SDL_CreateMutex();
  m_sleepMutex = SDL_CreateMutex();
  m_sleepCond = SDL_CreateCond();
  m_wakeUpInfos = "";
  m_safeKill = false;
  m_askSafeKill = false;
  m_dbKey = i_dbKey;
  m_dbReadOnly = i_dbReadOnly;
}

XMThread::~XMThread() {
  if (isThreadRunning() == true) {
    killThread();
  }
  SDL_DestroyMutex(m_curOpMutex);
  SDL_DestroyMutex(m_curMicOpMutex);
  SDL_DestroyMutex(m_sleepMutex);
  SDL_DestroyCond(m_sleepCond);
  xmDatabase::destroy(m_dbKey);
}

int XMThread::run(void *pThreadInstance) {
  XMThread *thisThread = reinterpret_cast<XMThread *>(pThreadInstance);

  return thisThread->threadFunctionEncapsulate();
}

void XMThread::startThread() {
  m_progress = -1;
  m_currentOperation = "";
  m_askThreadToEnd = false;
  m_isRunning = true; // set before running the thread
  m_pThread = SDL_CreateThread(&XMThread::run, NULL, this);
}

int XMThread::runInMain() {
  m_progress = -1;
  m_currentOperation = "";
  m_askThreadToEnd = false;
  m_isRunning = true; // set before running the thread

  return run(this);
}

int XMThread::waitForThreadEnd() {
  int returnValue;
  SDL_WaitThread(m_pThread, &returnValue);
  m_pThread = NULL;
  m_isRunning = false;
  return returnValue;
}

bool XMThread::isThreadRunning() {
  return m_isRunning;
}

void XMThread::askThreadToEnd() {
  m_askThreadToEnd = true;
}

void XMThread::askThreadToSleep() {
  if (m_isSleeping == false) {
    m_askThreadToSleep = true;
  }
}

void XMThread::killThread() {
  LogWarning("Kill violently the thread");
  // TODO:
  // SDL_KillThread(m_pThread);
  m_pThread = NULL;
  m_isRunning = false;
}

void XMThread::sleepThread() {
  SDL_LockMutex(m_sleepMutex);

  m_isSleeping = true;
  m_askThreadToSleep = false;

  SDL_CondWait(m_sleepCond, m_sleepMutex);
  SDL_UnlockMutex(m_sleepMutex);
}

void XMThread::unsleepThread(std::string infos) {
  if (m_isSleeping == true) {
    SDL_LockMutex(m_sleepMutex);
    SDL_CondSignal(m_sleepCond);

    m_isSleeping = false;
    m_wakeUpInfos = infos;

    SDL_UnlockMutex(m_sleepMutex);
  }
}

void XMThread::setSafeKill(bool i_value) {
  m_safeKill = i_value;

  // kill if it was asked
  if (m_safeKill && m_isRunning && m_askSafeKill) {
    killThread();
  }
}

void XMThread::safeKill() {
  m_askSafeKill = true;

  // kill if thread is in a safe state
  if (m_safeKill && m_isRunning) {
    killThread();
  }
}

int XMThread::getThreadProgress() {
  return m_progress;
}

std::string XMThread::getThreadCurrentOperation() {
  std::string curOp;

  SDL_LockMutex(m_curOpMutex);
  curOp = m_currentOperation;
  SDL_UnlockMutex(m_curOpMutex);

  return curOp;
}

std::string XMThread::getThreadCurrentMicroOperation() {
  std::string curMicOp;

  SDL_LockMutex(m_curMicOpMutex);
  curMicOp = m_currentMicroOperation;
  SDL_UnlockMutex(m_curMicOpMutex);

  return curMicOp;
}

void XMThread::setThreadProgress(int progress) {
  m_progress = progress;
}

void XMThread::setThreadCurrentOperation(std::string curOp) {
  SDL_LockMutex(m_curOpMutex);
  m_currentOperation = curOp;
  SDL_UnlockMutex(m_curOpMutex);
}

void XMThread::setThreadCurrentMicroOperation(std::string curMicOp) {
  SDL_LockMutex(m_curMicOpMutex);
  m_currentMicroOperation = curMicOp;
  SDL_UnlockMutex(m_curMicOpMutex);
}

int XMThread::threadFunctionEncapsulate() {
  // we can only have one thread at once.
  LogDebug("Open db for thread with key '%s'", m_dbKey.c_str());
  m_pDb = xmDatabase::instance(m_dbKey);
  m_pDb->init(DATABASE_FILE, m_dbReadOnly);

  int returnValue = realThreadFunction();
  m_isRunning = false;

  return returnValue;
}
