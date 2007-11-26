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

#include "UpdateHighscoresThread.h"
#include "GameText.h"
#include "Game.h"
#include "states/StateManager.h"

UpdateHighscoresThread::UpdateHighscoresThread()
  : XMThread()
{
  m_highscorePath = i_highscorePath;
}

UpdateHighscoresThread::~UpdateHighscoresThread()
{
}

int UpdateHighscoresThread::realThreadFunction()
{
#if 0  
  /* Fetch highscores from web? */
  m_pWebRooms = new WebRooms(&m_ProxySettings);
  m_pWebHighscores = new WebRoom(&m_ProxySettings);
  m_pWebHighscores->setWebsiteInfos(m_WebHighscoresIdRoom,
				    m_WebHighscoresURL);
 
  if(m_xmsession->www() && m_PlaySpecificLevelFile == "" && m_PlaySpecificReplay == "") {
    bool bSilent = true;
    try {
      if(m_bEnableCheckHighscoresAtStartup) {
	_UpdateLoadingScreen((1.0f/9.0f) * 3,GAMETEXT_DLHIGHSCORES);
	_UpdateWebHighscores(bSilent);
	_UpgradeWebHighscores();
      }
    } catch(Exception &e) {
      /* No internet connection, probably... (just use the latest times, if any) */
      Logger::Log("** Warning ** : Failed to update web-highscores [%s]",e.getMsg().c_str());
      if(!bSilent)
	notifyMsg(GAMETEXT_FAILEDDLHIGHSCORES + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
    }
 
    if(m_bEnableCheckNewLevelsAtStartup) {
      try {
	_UpdateLoadingScreen((1.0f/9.0f) * 4,GAMETEXT_DLLEVELSCHECK);
	_UpdateWebLevels(bSilent);
      } catch(Exception &e) {
	Logger::Log("** Warning ** : Failed to update web-levels [%s]",e.getMsg().c_str());
	if(!bSilent)
	  notifyMsg(GAMETEXT_FAILEDDLHIGHSCORES + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
      }
    }
  }
#endif
}

std::string UpdateHighscoresThread::getMsg() const
{
  return m_msg;
}

void UpdateHighscoresThread::setTaskProgress(float p_percent)
{
  setThreadProgress(p_percent);
}
