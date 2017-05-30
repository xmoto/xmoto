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

#include "StatePlaying.h"
#include "common/XMSession.h"
#include "xmoto/LuaLibGame.h"
#include "xmoto/Renderer.h"
#include "xmoto/Universe.h"
#include "xmscene/Bike.h"
#include "xmscene/BikeController.h"
#include "xmscene/BikePlayer.h"

StatePlaying::StatePlaying(Universe *i_universe, GameRenderer *i_renderer)
  : StateScene(i_universe, i_renderer) {
  m_displayStats = false;

  for (unsigned int i = 0; i < INPUT_NB_PLAYERS; i++) {
    m_changeDirKeyAlreadyPress[i] = false;
  }
}

StatePlaying::~StatePlaying() {}

void StatePlaying::enter() {
  StateScene::enter();

  updateWithOptions();
  setScoresTimes();
  playLevelMusic();

  // update description to nothing
  if (m_universe != NULL) {
    for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->setInfos("");
    }
  }
}

void StatePlaying::executeOneCommand(std::string cmd, std::string args) {
  if (cmd == "OPTIONS_UPDATED") {
    updateWithOptions();
  }

  else {
    StateScene::executeOneCommand(cmd, args);
  }
}

bool StatePlaying::renderOverShadow() {
  if (m_displayStats) {
    displayStats();
  }

  return true;
}

void StatePlaying::enterAfterPop() {
  StateScene::enterAfterPop();
  m_displayStats = false;
}

void StatePlaying::handleControllers(InputEventType Type,
                                     const XMKey &i_xmkey) {
  unsigned int p, pW;
  Biker *v_biker;

  switch (Type) {
    case INPUT_DOWN:
      p = 0; // player number p
      pW = 0; // number of players in the previous worlds
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        for (unsigned int i = 0;
             i < m_universe->getScenes()[j]->Players().size();
             i++) {
          v_biker = m_universe->getScenes()[j]->Players()[i];

          // if else is not valid while axis up can be a signal for two sides
          if ((*InputHandler::instance()->getPlayerKey(INPUT_DRIVE, p)) ==
              i_xmkey) {
            /* Start driving */
            if (i_xmkey.isAnalogic()) {
              v_biker->getControler()->setThrottle(
                fabs(i_xmkey.getAnalogicValue()));
            } else {
              v_biker->getControler()->setThrottle(1.0f);
            }
          }

          if ((*InputHandler::instance()->getPlayerKey(INPUT_BRAKE, p)) ==
              i_xmkey) {
            /* Brake */
            v_biker->getControler()->setBreak(1.0f);
          }

          if (((*InputHandler::instance()->getPlayerKey(INPUT_FLIPLEFT, p)) ==
                 i_xmkey &&
               XMSession::instance()->mirrorMode() == false) ||
              ((*InputHandler::instance()->getPlayerKey(INPUT_FLIPRIGHT, p)) ==
                 i_xmkey &&
               XMSession::instance()->mirrorMode())) {
            /* Pull back */
            if (i_xmkey.isAnalogic()) {
              v_biker->getControler()->setPull(
                fabs(i_xmkey.getAnalogicValue()));
            } else {
              v_biker->getControler()->setPull(1.0f);
            }
          }

          if (((*InputHandler::instance()->getPlayerKey(INPUT_FLIPRIGHT, p)) ==
                 i_xmkey &&
               XMSession::instance()->mirrorMode() == false) ||
              ((*InputHandler::instance()->getPlayerKey(INPUT_FLIPLEFT, p)) ==
                 i_xmkey &&
               XMSession::instance()->mirrorMode())) {
            /* Push forward */
            if (i_xmkey.isAnalogic()) {
              v_biker->getControler()->setPull(
                -fabs(i_xmkey.getAnalogicValue()));
            } else {
              v_biker->getControler()->setPull(-1.0f);
            }
          }

          if ((*InputHandler::instance()->getPlayerKey(INPUT_CHANGEDIR, p)) ==
              i_xmkey) {
            /* Change dir */
            if (m_changeDirKeyAlreadyPress[p] == false) {
              v_biker->getControler()->setChangeDir(true);
              m_changeDirKeyAlreadyPress[p] = true;
            }
          }
          p++;
        }
        pW += m_universe->getScenes()[j]->Players().size();
      }

      break;
    case INPUT_UP:
      p = 0; // player number p
      pW = 0; // number of players in the previous worlds
      for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
        for (unsigned int i = 0;
             i < m_universe->getScenes()[j]->Players().size();
             i++) {
          v_biker = m_universe->getScenes()[j]->Players()[i];

          // if else is not valid while axis up can be a signal for two sides
          if ((*InputHandler::instance()->getPlayerKey(INPUT_DRIVE, p)) ==
              i_xmkey) {
            /* Stop driving */
            v_biker->getControler()->setThrottle(0.0f);
          }

          if ((*InputHandler::instance()->getPlayerKey(INPUT_BRAKE, p)) ==
              i_xmkey) {
            /* Don't brake */
            v_biker->getControler()->setBreak(0.0f);
          }

          if (((*InputHandler::instance()->getPlayerKey(INPUT_FLIPLEFT, p)) ==
                 i_xmkey &&
               XMSession::instance()->mirrorMode() == false) ||
              ((*InputHandler::instance()->getPlayerKey(INPUT_FLIPRIGHT, p)) ==
                 i_xmkey &&
               XMSession::instance()->mirrorMode())) {
            /* Pull back */
            v_biker->getControler()->setPull(0.0f);
          }

          if (((*InputHandler::instance()->getPlayerKey(INPUT_FLIPRIGHT, p)) ==
                 i_xmkey &&
               XMSession::instance()->mirrorMode() == false) ||
              ((*InputHandler::instance()->getPlayerKey(INPUT_FLIPLEFT, p)) ==
                 i_xmkey &&
               XMSession::instance()->mirrorMode())) {
            /* Push forward */
            v_biker->getControler()->setPull(0.0f);
          }

          if ((*InputHandler::instance()->getPlayerKey(INPUT_CHANGEDIR, p)) ==
              i_xmkey) {
            m_changeDirKeyAlreadyPress[p] = false;
          }
          p++;
        }
        pW += m_universe->getScenes()[j]->Players().size();
      }
      break;
  }
}

void StatePlaying::handleScriptKeys(InputEventType Type, const XMKey &i_xmkey) {
  /* Have the script hooked this key? */
  if (Type == INPUT_DOWN) {
    for (int i = 0; i < InputHandler::instance()->getNumScriptKeyHooks(); i++) {
      if (InputHandler::instance()->getScriptKeyHooks(i).nKey == i_xmkey) {
        /* Invoke script */
        InputHandler::instance()
          ->getScriptKeyHooks(i)
          .pGame->getLuaLibGame()
          ->scriptCallVoid(
            InputHandler::instance()->getScriptKeyHooks(i).FuncName);
      }
      for (int j = 0; j < INPUT_NB_PLAYERS; j++) {
        if (InputHandler::instance()->getScriptActionKeys(j, i) == i_xmkey) {
          InputHandler::instance()
            ->getScriptKeyHooks(i)
            .pGame->getLuaLibGame()
            ->scriptCallVoid(
              InputHandler::instance()->getScriptKeyHooks(i).FuncName);
        }
      }
    }
  }
}

void StatePlaying::dealWithActivedKeys() {
  Uint8 *v_keystate = SDL_GetKeyState(NULL);
  Uint8 v_mousestate = SDL_GetMouseState(NULL, NULL);
  unsigned int p, pW;
  Biker *v_biker;

  p = 0; // player number p
  pW = 0; // number of players in the previous worlds
  for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
    for (unsigned int i = 0; i < m_universe->getScenes()[j]->Players().size();
         i++) {
      v_biker = m_universe->getScenes()[j]->Players()[i];

      if (InputHandler::instance()
            ->getPlayerKey(INPUT_DRIVE, p)
            ->isPressed(v_keystate, v_mousestate)) {
        /* Start driving */
        v_biker->getControler()->setThrottle(1.0f);
      } else {
        v_biker->getControler()->setThrottle(0.0f);
      }

      if (InputHandler::instance()
            ->getPlayerKey(INPUT_BRAKE, p)
            ->isPressed(v_keystate, v_mousestate)) {
        /* Brake */
        v_biker->getControler()->setBreak(1.0f);
      } else {
        v_biker->getControler()->setBreak(0.0f);
      }

      // pull
      if ((InputHandler::instance()
             ->getPlayerKey(INPUT_FLIPLEFT, p)
             ->isPressed(v_keystate, v_mousestate) &&
           XMSession::instance()->mirrorMode() == false) ||
          (InputHandler::instance()
             ->getPlayerKey(INPUT_FLIPRIGHT, p)
             ->isPressed(v_keystate, v_mousestate) &&
           XMSession::instance()->mirrorMode())) {
        /* Pull back */
        v_biker->getControler()->setPull(1.0f);
      } else {
        // push // must be in pull else block to not set pull to 0
        if ((InputHandler::instance()
               ->getPlayerKey(INPUT_FLIPRIGHT, p)
               ->isPressed(v_keystate, v_mousestate) &&
             XMSession::instance()->mirrorMode() == false) ||
            (InputHandler::instance()
               ->getPlayerKey(INPUT_FLIPLEFT, p)
               ->isPressed(v_keystate, v_mousestate) &&
             XMSession::instance()->mirrorMode())) {
          /* Push forward */
          v_biker->getControler()->setPull(-1.0f);
        } else {
          v_biker->getControler()->setPull(0.0f);
        }
      }

      if (InputHandler::instance()
            ->getPlayerKey(INPUT_CHANGEDIR, p)
            ->isPressed(v_keystate, v_mousestate)) {
        /* Change dir */
        if (m_changeDirKeyAlreadyPress[p] == false) {
          v_biker->getControler()->setChangeDir(true);
          m_changeDirKeyAlreadyPress[p] = true;
        }
      } else {
        m_changeDirKeyAlreadyPress[p] = false;
      }
      p++;
    }
    pW += m_universe->getScenes()[j]->Players().size();
  }
}

void StatePlaying::updateWithOptions() {
  if (m_renderer != NULL) {
    if (XMSession::instance()->hidePlayingInformation() == false) {
      m_renderer->setShowEngineCounter(
        XMSession::instance()->showEngineCounter());
      m_renderer->setShowMinimap(XMSession::instance()->showMinimap());
      m_renderer->setShowTimePanel(true);
    } else {
      m_renderer->setShowEngineCounter(false);
      m_renderer->setShowMinimap(false);
      m_renderer->setShowTimePanel(false);
    }

    m_renderer->setShowGhostsText(true);
  }

  for (unsigned int i = 0; i < m_universe->getScenes().size(); i++) {
    m_universe->getScenes()[i]->setShowGhostTimeDiff(
      XMSession::instance()->showGhostTimeDifference());
  }
}
