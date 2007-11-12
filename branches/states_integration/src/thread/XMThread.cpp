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
#include "Game.h"

XMThread::XMThread()
{
  m_isRunning        = false;
  m_pThread          = NULL;
  m_progress         = 0;
  m_currentOperation = "";
  m_pGame            = NULL;
}

XMThread::~XMThread()
{
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
  return m_progress;
}

std::string XMThread::getThreadCurrentOperation()
{
  return m_currentOperation;
}

int XMThread::threadFunctionEncapsulate()
{
  int returnValue = realThreadFunction();
  m_pThread       = NULL;
  m_isRunning     = false;

  return returnValue;
}
