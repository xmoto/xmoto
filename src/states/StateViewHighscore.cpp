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

#include "StateViewHighscore.h"
#include "StateManager.h"
#include "StatePreplayingReplay.h"
#include "common/VFileIO.h"
#include "common/XMSession.h"
#include "drawlib/DrawLib.h"
#include "helpers/Log.h"
#include "thread/DownloadReplaysThread.h"
#include "xmoto/Game.h"
#include "xmoto/GameText.h"
#include "xmoto/SysMessage.h"

StateViewHighscore::StateViewHighscore(const std::string &i_id_level,
                                       bool drawStateBehind,
                                       bool updateStatesBehind)
  : StateWaiting(drawStateBehind, updateStatesBehind) {
  m_name = "StateViewHighscore";
  m_id_level = i_id_level;

  StateManager::instance()->registerAsObserver(std::string("REPLAY_DOWNLOADED"),
                                               this);
  StateManager::instance()->registerAsObserver(
    std::string("REPLAY_FAILEDTODOWNLOAD"), this);
}

StateViewHighscore::~StateViewHighscore() {
  StateManager::instance()->unregisterAsObserver(
    std::string("REPLAY_DOWNLOADED"), this);
  StateManager::instance()->unregisterAsObserver(
    std::string("REPLAY_FAILEDTODOWNLOAD"), this);
}

void StateViewHighscore::enter() {
  char **v_result;
  unsigned int nrow;
  std::string v_replayName;

  StateWaiting::enter();

  // waiting information
  m_progress = 0;
  m_currentOperation = GAMETEXT_DLGHOST;
  m_currentMicroOperation = "";
  updateGUI();
  //

  v_result = xmDatabase::instance("main")->readDB(
    "SELECT fileUrl "
    "FROM webhighscores WHERE id_level=\"" +
      xmDatabase::protectString(m_id_level) +
      "\" "
      "AND id_room=" +
      XMSession::instance()->idRoom(0) + ";",
    nrow);

  if (nrow != 1) {
    xmDatabase::instance("main")->read_DB_free(v_result);

    // no ghost to download
    m_requestForEnd = true;
    return;
  }

  m_url = xmDatabase::instance("main")->getResult(v_result, 1, 0, 0);
  xmDatabase::instance("main")->read_DB_free(v_result);

  v_replayName = XMFS::getFileBaseName(m_url);

  // replay already downloaded
  if (xmDatabase::instance("main")->replays_exists(v_replayName)) {
    StateManager::instance()->replaceState(
      new StatePreplayingReplay(v_replayName, false), getStateId());
  } else {
    StateManager::instance()->getReplayDownloaderThread()->add(m_url);
    StateManager::instance()->getReplayDownloaderThread()->doJob();
  }
}

void StateViewHighscore::executeOneCommand(std::string cmd, std::string args) {
  if (cmd == "REPLAY_DOWNLOADED") {
    std::string v_replayName;
    v_replayName = XMFS::getFileBaseName(m_url);
    if (args == v_replayName) {
      StateManager::instance()->replaceState(
        new StatePreplayingReplay(v_replayName, false), getStateId());
    }
  } else if (cmd == "REPLAY_FAILEDTODOWNLOAD") {
    GameApp::instance()->enableWWW(false);
    SysMessage::instance()->displayError(
      GAMETEXT_FAILEDDLREPLAY + std::string("\n") + SYS_MSG_WWW_DISABLED);
    StateManager::instance()->sendAsynchronousMessage(
      std::string("CHANGE_WWW_ACCESS"));
  } else {
    GameState::executeOneCommand(cmd, args);
  }
}

void StateViewHighscore::xmKey(InputEventType i_type, const XMKey &i_xmkey) {
  if (i_type == INPUT_DOWN &&
      (i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE) ||
       i_xmkey.getJoyButton() == SDL_CONTROLLER_BUTTON_B)) {
    m_requestForEnd = true;
  }

  else {
    StateWaiting::xmKey(i_type, i_xmkey);
  }
}
