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
#include "VCommon.h"
#include "Game.h"
#include "helpers/Log.h"

XMThread::XMThread()
{
  m_isRunning        = false;
  m_isSleeping       = false;
  m_askThreadToEnd   = false;
  m_askThreadToSleep = false;
  m_pThread          = NULL;
  m_progress         = 0;
  m_currentOperation = "";
  m_currentMicroOperation = "";
  m_pGame            = NULL;
  m_pDb              = NULL;
  m_curOpMutex       = SDL_CreateMutex();
  m_curMicOpMutex    = SDL_CreateMutex();
  m_sleepMutex       = SDL_CreateMutex();
  m_sleepCond        = SDL_CreateCond();
  m_wakeUpInfos      = "";
}

XMThread::~XMThread()
{
  SDL_DestroyMutex(m_curOpMutex);
  SDL_DestroyMutex(m_curMicOpMutex);
  SDL_DestroyMutex(m_sleepMutex);
  SDL_DestroyCond(m_sleepCond);
  if(m_pDb != NULL){
    delete m_pDb;
  }
}

int XMThread::run(void* pThreadInstance)
{
  XMThread* thisThread = reinterpret_cast<XMThread*>(pThreadInstance);

  return thisThread->threadFunctionEncapsulate();
}

void XMThread::startThread(GameApp* pGame)
{
    m_pGame            = pGame;
    m_progress         = -1;
    m_currentOperation = "";
    m_askThreadToEnd   = false;
    m_isRunning        = true; // set before running the thread
    m_pThread          = SDL_CreateThread(&XMThread::run, this);
}

int XMThread::waitForThreadEnd()
{
  int returnValue;
  SDL_WaitThread(m_pThread, &returnValue);
  m_pThread   = NULL;
  m_isRunning = false;
  return returnValue;
}

bool XMThread::isThreadRunning()
{
  return m_isRunning;
}

void XMThread::askThreadToEnd()
{
  m_askThreadToEnd = true;
}

void XMThread::askThreadToSleep()
{
  if(m_isSleeping == false){
    m_askThreadToSleep = true;
  }
}

void XMThread::killThread()
{
  SDL_KillThread(m_pThread);
  m_pThread   = NULL;
  m_isRunning = false;
}

void XMThread::sleepThread()
{
  SDL_LockMutex(m_sleepMutex);

  m_isSleeping = true;
  m_askThreadToSleep = false;

  SDL_CondWait(m_sleepCond, m_sleepMutex);
  SDL_UnlockMutex(m_sleepMutex);
}

void XMThread::unsleepThread(std::string infos)
{
  if(m_isSleeping == true){
    SDL_LockMutex(m_sleepMutex);
    SDL_CondSignal(m_sleepCond);

    m_isSleeping = false;
    m_wakeUpInfos = infos;

    SDL_UnlockMutex(m_sleepMutex);
  }
}

int XMThread::getThreadProgress()
{
  return m_progress;
}

std::string XMThread::getThreadCurrentOperation()
{
  std::string curOp;

  SDL_LockMutex(m_curOpMutex);
  curOp = m_currentOperation;
  SDL_UnlockMutex(m_curOpMutex);

  return curOp;
}

std::string XMThread::getThreadCurrentMicroOperation()
{
  std::string curMicOp;

  SDL_LockMutex(m_curMicOpMutex);
  curMicOp = m_currentMicroOperation;
  SDL_UnlockMutex(m_curMicOpMutex);

  return curMicOp;
}

void XMThread::setThreadProgress(int progress)
{
  m_progress = progress;
}

void XMThread::setThreadCurrentOperation(std::string curOp)
{
  SDL_LockMutex(m_curOpMutex);
  m_currentOperation = curOp;
  SDL_UnlockMutex(m_curOpMutex);
}

void XMThread::setThreadCurrentMicroOperation(std::string curMicOp)
{
  SDL_LockMutex(m_curMicOpMutex);
  m_currentMicroOperation = curMicOp;
  SDL_UnlockMutex(m_curMicOpMutex);
}

int XMThread::threadFunctionEncapsulate()
{
  m_pDb = new xmDatabase(DATABASE_FILE);

  int returnValue = realThreadFunction();
  m_isRunning     = false;

  return returnValue;
}
