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

#include "UpgradeLevelsThread.h"
#include "Game.h"
#include "helpers/Log.h"
#include "GameText.h"
#include "states/StateManager.h"

UpgradeLevelsThread::UpgradeLevelsThread(GameState* pCallingState)
  : XMThread()
{
  m_pWebLevels    = new WebLevels(this);
  m_pCallingState = pCallingState;
  m_updateAutomaticallyLevels = false;
}

UpgradeLevelsThread::~UpgradeLevelsThread()
{
  delete m_pWebLevels;
}

void UpgradeLevelsThread::setBeingDownloadedInformation(const std::string &p_information, bool p_isNew)
{
#if 0
  void GameApp::setBeingDownloadedInformation(const std::string &p_information,bool bNew) {
    m_DownloadingInformation = p_information;
  }
#endif  
}

void UpgradeLevelsThread::readEvents()
{
#if 0
  void GameApp::readEvents(void) {
    /* Check for events */ 
    SDL_PumpEvents();
    
    SDL_Event Event;
    while(SDL_PollEvent(&Event)) {
      /* What event? */
      switch(Event.type) {
        case SDL_KEYDOWN: 
          if(Event.key.keysym.sym == SDLK_ESCAPE)
            setCancelAsSoonAsPossible();
          break;
        case SDL_QUIT:  
          /* Force quit */
          quit();
          setCancelAsSoonAsPossible();
          return;
      }
    }    
  }
#endif
}

bool UpgradeLevelsThread::shouldLevelBeUpdated(const std::string &LevelID)
{
#if 0  
  /*===========================================================================
  WWWAppInterface implementation
  ===========================================================================*/
  bool GameApp::shouldLevelBeUpdated(const std::string &LevelID) {
    if(m_updateAutomaticallyLevels) {
      return true;
    }

    /* Hmm... ask user whether this level should be updated */
    bool bRet = true;
    bool bDialogBoxOpen = true;
    char cBuf[1024];
    char **v_result;
    unsigned int nrow;
    std::string v_levelName;
    std::string v_levelFileName;

    v_result = m_db->readDB("SELECT name, filepath "
			    "FROM levels "
			    "WHERE id_level=\"" + xmDatabase::protectString(LevelID) + "\";",
			    nrow);
    if(nrow != 1) {
      m_db->read_DB_free(v_result);
      return true;
    }

    v_levelName     = m_db->getResult(v_result, 2, 0, 0);
    v_levelFileName = m_db->getResult(v_result, 2, 0, 1);
    m_db->read_DB_free(v_result);

    sprintf(cBuf,(std::string(GAMETEXT_WANTTOUPDATELEVEL) + "\n(%s)").c_str(), v_levelName.c_str(),
	    v_levelFileName.c_str());
    m_Renderer->getGUI()->setFont(drawLib->getFontSmall());
    UIMsgBox *pMsgBox = m_Renderer->getGUI()->msgBox(cBuf,(UIMsgBoxButton)(UI_MSGBOX_YES|UI_MSGBOX_NO|UI_MSGBOX_YES_FOR_ALL));

    while(bDialogBoxOpen) {
      SDL_PumpEvents();
      
      SDL_Event Event;
      while(SDL_PollEvent(&Event)) {
	/* What event? */
	switch(Event.type) {
	case SDL_QUIT:  
	  /* Force quit */
	  quit();
	  setCancelAsSoonAsPossible();
	  return false;
	case SDL_MOUSEBUTTONDOWN:
	  mouseDown(Event.button.button);
	  break;
	case SDL_MOUSEBUTTONUP:
	  mouseUp(Event.button.button);
	  break;
	}
      }
      
      UIMsgBoxButton Button = pMsgBox->getClicked();
      if(Button != UI_MSGBOX_NOTHING) {
	if(Button == UI_MSGBOX_NO) {
	  bRet = false;
	}
	if(Button == UI_MSGBOX_YES_FOR_ALL) {
	  m_updateAutomaticallyLevels = true;
	}
	bDialogBoxOpen = false;
      }
      
      m_Renderer->getGUI()->dispatchMouseHover();
      
      m_Renderer->getGUI()->paint();
      
      UIRect TempRect;
      
      //if(m_pCursor != NULL) {        
      //	int nMX,nMY;
      //	getMousePos(&nMX,&nMY);      
      //	drawLib->drawImage(Vector2f(nMX-2,nMY-2),Vector2f(nMX+30,nMY+30),m_pCursor);
      //}
      
      drawLib->flushGraphics();
    }
    
    delete pMsgBox;
    setTaskProgress(m_fDownloadTaskProgressLast);
    
    return bRet;        
  }
#endif
}

int UpgradeLevelsThread::realThreadFunction()
{
  /* Check for extra levels */
  try {
    setThreadCurrentOperation(GAMETEXT_CHECKINGFORLEVELS);
    setThreadProgress(0);
      
    ProxySettings* pProxySettings = m_pGame->getSession()->proxySettings();
    std::string    webLevelsUrl   = m_pGame->getSession()->webLevelsUrl();
    m_pWebLevels->setWebsiteInfos(webLevelsUrl, pProxySettings);

    Logger::Log("WWW: Checking for new or updated levels...");

    clearCancelAsSoonAsPossible();

    m_pWebLevels->update(m_pDb);

    int nULevels=0;
    nULevels = m_pWebLevels->nbLevelsToGet(m_pDb);
    bool bWebLevelsToDownload = (nULevels != 0);

    Logger::Log("WWW: %d new or updated level%s found",
		nULevels,
		(nULevels ==1 ) ? "" : "s");

    if(nULevels == 0) {
      m_msg = GAMETEXT_NONEWLEVELS;
      return 1;
    }
    else {
      m_pGame->getStateManager()->sendAsynchronousMessage("NEWLEVELAVAILABLE");
    }
  } catch (Exception& e) {
    Logger::Log("** Warning ** : Unable to check for extra levels [%s]", e.getMsg().c_str());
    m_msg = GAMETEXT_FAILEDCHECKLEVELS + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW;

    return 1;
  }

  setThreadProgress(100);

  Logger::Log("before sleeping");

  // sleep while we don't have the user response
  sleepThread();

  Logger::Log("after sleeping");

  if(m_askThreadToEnd == true){
    return 0;
  }

  Logger::Log("lets downloading");

  setThreadCurrentOperation(GAMETEXT_DLLEVELS);
  setThreadProgress(0);

  try {                  
    Logger::Log("WWW: Downloading levels...");

    clearCancelAsSoonAsPossible();

    m_pWebLevels->upgrade(m_pDb);
    m_pGame->getStateManager()->sendAsynchronousMessage("NO_NEW_LEVELS_TO_DOWNLOAD");
  }
  catch(Exception& e) {
    m_msg = GAMETEXT_FAILEDDLLEVELS + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW;
    Logger::Log("** Warning ** : Unable to download extra levels [%s]",e.getMsg().c_str());

    return 1;
  }

  /* Got some new levels... load them! */
  Logger::Log("Loading new and updated levels...");

  m_pGame->getLevelsManager()->updateLevelsFromLvl(m_pDb,
						  m_pWebLevels->getNewDownloadedLevels(),
						  m_pWebLevels->getUpdatedDownloadedLevels());

  /* Update level lists */
  m_pGame->getStateManager()->sendAsynchronousMessage("LEVELS_UPDATED");	

  return 0;
}

void UpgradeLevelsThread::setTaskProgress(float p_percent)
{
  setThreadProgress(p_percent);
}

std::string UpgradeLevelsThread::getMsg() const
{
  return m_msg;
}
