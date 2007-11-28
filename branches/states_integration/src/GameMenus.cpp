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

/* 
 *  Game application. (menus)
 */
#include "GameText.h"
#include "Game.h"
#include "helpers/Log.h"
#include "XMBuild.h"
#include "XMSession.h"
#include "drawlib/DrawLib.h"
#include "gui/specific/GUIXMoto.h"
#include "xmscene/Camera.h"
#include "states/StateManager.h"
#include "PhysSettings.h"
  
  void GameApp::_SimpleMessage(const std::string &Msg,UIRect *pRect,bool bNoSwap) {      

  }

  void GameApp::setLevelInfoFrameBestPlayer(std::string pLevelID,
					    UIWindow *i_pLevelInfoFrame,
					    UIButton *i_pLevelInfoViewReplayButton,
					    UIStatic *i_pBestPlayerText
					    ) {

  }

  void GameApp::viewHighscoreOf() {
    char **v_result;
    unsigned int nrow;
    std::string v_levelAuthor;
    std::string v_fileUrl;
    std::string v_replayName;

    m_PlaySpecificReplay = "";

    v_result = m_db->readDB("SELECT id_profile, fileUrl "
			    "FROM webhighscores WHERE id_level=\"" + 
			    xmDatabase::protectString(m_pLevelToShowOnViewHighscore) + "\" "
			    "AND id_room=" + m_xmsession->idRoom() + ";",
			    nrow);
    if(nrow == 0) {
      m_db->read_DB_free(v_result);
      return;
    }

    v_levelAuthor = m_db->getResult(v_result, 2, 0, 0);
    v_fileUrl     = m_db->getResult(v_result, 2, 0, 1);
    v_replayName  = FS::getFileBaseName(v_fileUrl);
    m_db->read_DB_free(v_result);

    if(m_db->replays_exists(v_replayName)) {
      m_PlaySpecificReplay = v_replayName;
      return;
    }
    
    if(m_xmsession->www() == false) {
      return;
    }

    try {
      _SimpleMessage(GAMETEXT_DLHIGHSCORE,&m_InfoMsgBoxRect);
      m_pWebHighscores->downloadReplay(v_fileUrl);
      addReplay(v_replayName);
      getStateManager()->sendAsynchronousMessage("REPLAYS_UPDATED");
      
      /* not very nice : make a new search to be sure the replay is here */
      /* because it could have been downloaded but unplayable : for macosx for example */
      if(m_db->replays_exists(v_replayName) == false) {
	notifyMsg(GAMETEXT_FAILEDTOLOADREPLAY);
	return;
      }
    } catch(Exception &e) {
      notifyMsg(GAMETEXT_FAILEDDLREPLAY + std::string("\n") + GAMETEXT_CHECK_YOUR_WWW);
      return;
    }
    m_PlaySpecificReplay = v_replayName;
  }
