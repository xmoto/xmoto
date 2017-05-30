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

#include "LevelsPacksCountUpdateThread.h"
#include "common/XMSession.h"
#include "states/StateManager.h"
#include "xmoto/LevelsManager.h"

LevelsPacksCountUpdateThread::LevelsPacksCountUpdateThread()
  : XMThread("LPCU", true) {
  if (XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter(
      std::string("LEVELSPACKS_COUNT_UPDATED"));
  }
}

LevelsPacksCountUpdateThread::~LevelsPacksCountUpdateThread() {}

int LevelsPacksCountUpdateThread::realThreadFunction() {
  LevelsManager *v_lm = LevelsManager::instance();

  v_lm->lockLevelsPacks();
  try {
    for (unsigned int i = 0; i < v_lm->LevelsPacks().size(); i++) {
      /* the unpackaged pack exists only in debug mode */
      if (v_lm->LevelsPacks()[i]->Name() != "" ||
          XMSession::instance()->debug()) {
        v_lm->LevelsPacks()[i]->updateCount(m_pDb,
                                            XMSession::instance()->profile());
      }
    }
  } catch (Exception &e) {
    /* some packs could have been updated */
    v_lm->unlockLevelsPacks();
    StateManager::instance()->sendAsynchronousMessage(
      std::string("LEVELSPACKS_COUNT_UPDATED"));
    return 1;
  }
  v_lm->unlockLevelsPacks();

  StateManager::instance()->sendAsynchronousMessage(
    std::string("LEVELSPACKS_COUNT_UPDATED"));
  return 0;
}
