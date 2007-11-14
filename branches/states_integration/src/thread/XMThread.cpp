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
#include "SDL_thread.h"
#include "SDL_mutex.h"
#include "Game.h"
#include "helpers/Log.h"

XMThread::XMThread()
{
  m_isRunning        = false;
  m_pThread          = NULL;
  m_progress         = 0;
  m_currentOperation = "";
  m_pGame            = NULL;
  m_pDb              = NULL;
  m_progressMutex    = SDL_CreateMutex();
  m_curOpMutex       = SDL_CreateMutex();
}

XMThread::~XMThread()
{
  SDL_DestroyMutex(m_progressMutex);
  SDL_DestroyMutex(m_curOpMutex);
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
    m_pThread          = SDL_CreateThread(&XMThread::run, this);
    m_isRunning        = true;
    m_progress         = 0;
    m_currentOperation = "";
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

void XMThread::killThread()
{
  SDL_KillThread(m_pThread);
  m_pThread   = NULL;
  m_isRunning = false;
}

int XMThread::getThreadProgress()
{
  int progress;

  SDL_LockMutex(m_progressMutex);
  progress = m_progress;
  SDL_UnlockMutex(m_progressMutex);

  return progress;
}

std::string XMThread::getThreadCurrentOperation()
{
  std::string curOp;

  SDL_LockMutex(m_curOpMutex);
  curOp = m_currentOperation;
  SDL_UnlockMutex(m_curOpMutex);

  return curOp;
}

void XMThread::setThreadProgress(int progress)
{
  SDL_LockMutex(m_progressMutex);
  m_progress = progress;
  SDL_UnlockMutex(m_progressMutex);
}

void XMThread::setThreadCurrentOperation(std::string curOp)
{
  SDL_LockMutex(m_curOpMutex);
  m_currentOperation = curOp;
  SDL_UnlockMutex(m_curOpMutex);
}

#define DATABASE_FILE FS::getUserDirUTF8() + "/" + "xm.db"

int XMThread::threadFunctionEncapsulate()
{
  m_pDb = new xmDatabase(DATABASE_FILE);

  int returnValue = realThreadFunction();
  m_isRunning     = false;

  return returnValue;
}
