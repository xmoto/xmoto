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

#include "StateDownloadGhost.h"
#include "../GameText.h"
#include "StatePreplayingReplay.h"
#include "StateReplaying.h"
#include "../thread/DownloadGhostThread.h"
#include "../helpers/Log.h"
#include "../Game.h"
#include "../SysMessage.h"
#include "../helpers/CmdArgumentParser.h"

StateDownloadGhost::StateDownloadGhost(const std::string& i_id,
				       std::string levelId,
				       bool launchReplaying,
				       bool drawStateBehind,
				       bool updateStatesBehind)
  : StateUpdate(drawStateBehind, updateStatesBehind)
{
  m_pThread         = new DownloadGhostThread(levelId, launchReplaying);
  m_name            = "StateDownloadGhost";
  m_replayName      = "";
  m_launchReplaying = launchReplaying;
  // we don't want a message box on failure.
  m_messageOnFailure = launchReplaying;

  StateManager::instance()->registerAsObserver("GHOST_DOWNLOADED_REPLAY_FILE", this);

  if(XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("CHANGE_WWW_ACCESS");
    StateManager::instance()->registerAsEmitter("GHOST_DOWNLOADING_FAILED");
    StateManager::instance()->registerAsEmitter("GHOST_DOWNLOADED");
  }
}

StateDownloadGhost::~StateDownloadGhost()
{
  StateManager::instance()->unregisterAsObserver("GHOST_DOWNLOADED_REPLAY_FILE", this);
  delete m_pThread;
}

void StateDownloadGhost::setReplay(std::string replayName)
{
  m_replayName = replayName;
}

void StateDownloadGhost::callAfterThreadFinished(int threadResult)
{
  m_msg = ((DownloadGhostThread*)m_pThread)->getMsg();

  if(threadResult != 0) {
    GameApp::instance()->enableWWW(false);
    SysMessage::instance()->displayError(GAMETEXT_FAILEDDLREPLAY + std::string("\n") + SYS_MSG_WWW_DISABLED);
    StateManager::instance()->sendAsynchronousMessage("CHANGE_WWW_ACCESS");
    StateManager::instance()->sendSynchronousMessage("GHOST_DOWNLOADING_FAILED");
  } else {
    if(threadResult == 0 && m_launchReplaying == true){
      std::string msg = "Replay to play: " + m_replayName;
      LogInfo(msg.c_str());
      StateManager::instance()->replaceState(new StatePreplayingReplay(getId(), m_replayName, false),
					     this->getId());
    }
    else{
      StateManager::instance()->sendAsynchronousMessage("GHOST_DOWNLOADED");
    }
  }
}

void StateDownloadGhost::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_k, KMOD_LCTRL)) {
    if(m_threadStarted == true) {
      m_pThread->safeKill();
    }
  }
}

void StateDownloadGhost::executeOneCommand(std::string cmd, std::string args)
{
  LogDebug("cmd [%s [%s]] executed by state [%s].",
	   cmd.c_str(), args.c_str(), getName().c_str());

  if(cmd == "GHOST_DOWNLOADED_REPLAY_FILE") {
    setReplay(CmdArgumentParser::instance()->getString(args));
  } else {
    GameState::executeOneCommand(cmd, args);
  }
}
