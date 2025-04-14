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
#include "helpers/VExcept.h"
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
  uint32_t p, pW;
  Biker *v_biker;

  const auto checkKey = [i_xmkey](INPUT_PLAYERKEYS key,
                                  uint32_t player) -> bool {
    // This is a dirty workaround for the following bug:
    // 1. Press down key <x> (where x is a key that controls the bike)
    // 2. Press down ctrl
    // 3. Release key <x>
    // 3. Release ctrl
    // 4. Key <x> gets "stuck", e.g. if it's the "drive forward" key, the bike
    // keeps
    //    driving after the key is released
    // This needs to be properly fixed by overhauling the input code.
    return i_xmkey.equalsIgnoreMods(
      *Input::instance()->getPlayerKey(key, player));
  };

  bool mirrorMode = XMSession::instance()->mirrorMode();

  switch (Type) {
    case INPUT_DOWN: {
      p = 0; // player number p
      pW = 0; // number of players in the previous worlds

      for (auto &scene : m_universe->getScenes()) {
        for (auto &v_biker : scene->Players()) {
          // if else is not valid while axis up can be a signal for two sides
          if (checkKey(INPUT_DRIVE, p)) {
            /* Start driving */
            if (i_xmkey.isAnalog()) {
              float throttle = fabs(i_xmkey.getAnalogValue());
              v_biker->getControler()->setThrottle(throttle);
            } else {
              v_biker->getControler()->setThrottle(1.0f);
            }
          }

          if (checkKey(INPUT_BRAKE, p)) {
            /* Brake */
            v_biker->getControler()->setBreak(1.0f);
          }

          if ((checkKey(INPUT_FLIPLEFT, p) && !mirrorMode) ||
              (checkKey(INPUT_FLIPRIGHT, p) && mirrorMode)) {
            /* Pull back */
            if (i_xmkey.isAnalog()) {
              float pull = fabs(i_xmkey.getAnalogValue());
              v_biker->getControler()->setPull(pull);
            } else {
              v_biker->getControler()->setPull(1.0f);
            }
          }

          if ((checkKey(INPUT_FLIPRIGHT, p) && !mirrorMode) ||
              (checkKey(INPUT_FLIPLEFT, p) && mirrorMode)) {
            /* Push forward */
            if (i_xmkey.isAnalog()) {
              float pull = -fabs(i_xmkey.getAnalogValue());
              v_biker->getControler()->setPull(pull);
            } else {
              v_biker->getControler()->setPull(-1.0f);
            }
          }

          if (checkKey(INPUT_CHANGEDIR, p)) {
            /* Change dir */
            if (m_changeDirKeyAlreadyPress[p] == false) {
              v_biker->getControler()->setChangeDir(true);
              m_changeDirKeyAlreadyPress[p] = true;
            }
          }

          p++;
        }

        pW += scene->Players().size();
      }

      break;
    }

    case INPUT_UP: {
      p = 0; // player number p
      pW = 0; // number of players in the previous worlds
      for (auto &scene : m_universe->getScenes()) {
        for (auto &v_biker : scene->Players()) {
          // if else is not valid while axis up can be a signal for two sides
          if (checkKey(INPUT_DRIVE, p)) {
            /* Stop driving */
            v_biker->getControler()->setThrottle(0.0f);
          }

          if (checkKey(INPUT_BRAKE, p)) {
            /* Don't brake */
            v_biker->getControler()->setBreak(0.0f);
          }

          if ((checkKey(INPUT_FLIPLEFT, p) && !mirrorMode) ||
              (checkKey(INPUT_FLIPRIGHT, p) && mirrorMode)) {
            /* Pull back */
            v_biker->getControler()->setPull(0.0f);
          }

          if ((checkKey(INPUT_FLIPRIGHT, p) && !mirrorMode) ||
              (checkKey(INPUT_FLIPLEFT, p) && mirrorMode)) {
            /* Push forward */
            v_biker->getControler()->setPull(0.0f);
          }

          if (checkKey(INPUT_CHANGEDIR, p)) {
            m_changeDirKeyAlreadyPress[p] = false;
          }

          p++;
        }

        pW += scene->Players().size();
      }

      break;
    }

    default:
      break;
  }
}

void StatePlaying::handleScriptKeys(InputEventType Type, const XMKey &i_xmkey) {
  /* Have the script hooked this key? */
  if (Type == INPUT_DOWN) {
    for (int i = 0; i < Input::instance()->getNumScriptKeyHooks(); i++) {
      if (Input::instance()->getScriptKeyHooks(i).nKey == i_xmkey) {
        /* Invoke script */
        Input::instance()
          ->getScriptKeyHooks(i)
          .pGame->getLuaLibGame()
          ->scriptCallVoid(Input::instance()->getScriptKeyHooks(i).FuncName);
      }
      for (int j = 0; j < INPUT_NB_PLAYERS; j++) {
        if (Input::instance()->getScriptActionKeys(j, i) == i_xmkey) {
          Input::instance()
            ->getScriptKeyHooks(i)
            .pGame->getLuaLibGame()
            ->scriptCallVoid(Input::instance()->getScriptKeyHooks(i).FuncName);
        }
      }
    }
  }
}

void StatePlaying::dealWithActivedKeys() {
  int numkeys = 0;
  const Uint8 *v_keystate = SDL_GetKeyboardState(&numkeys);
  if (!v_keystate) {
    throw Exception("dealWithActivedKeys: SDL_GetKeyboardState returned NULL");
  }

  Uint8 v_mousestate = SDL_GetMouseState(NULL, NULL);
  unsigned int p, pW;
  Biker *v_biker;

  p = 0; // player number p
  pW = 0; // number of players in the previous worlds
  for (unsigned int j = 0; j < m_universe->getScenes().size(); j++) {
    for (unsigned int i = 0; i < m_universe->getScenes()[j]->Players().size();
         i++) {
      v_biker = m_universe->getScenes()[j]->Players()[i];

      if (Input::instance()
            ->getPlayerKey(INPUT_DRIVE, p)
            ->isPressed(v_keystate, v_mousestate, numkeys)) {
        /* Start driving */
        v_biker->getControler()->setThrottle(1.0f);
      } else {
        v_biker->getControler()->setThrottle(0.0f);
      }

      if (Input::instance()
            ->getPlayerKey(INPUT_BRAKE, p)
            ->isPressed(v_keystate, v_mousestate, numkeys)) {
        /* Brake */
        v_biker->getControler()->setBreak(1.0f);
      } else {
        v_biker->getControler()->setBreak(0.0f);
      }

      // pull
      if ((Input::instance()
             ->getPlayerKey(INPUT_FLIPLEFT, p)
             ->isPressed(v_keystate, v_mousestate, numkeys) &&
           XMSession::instance()->mirrorMode() == false) ||
          (Input::instance()
             ->getPlayerKey(INPUT_FLIPRIGHT, p)
             ->isPressed(v_keystate, v_mousestate, numkeys) &&
           XMSession::instance()->mirrorMode())) {
        /* Pull back */
        v_biker->getControler()->setPull(1.0f);
      } else {
        // push // must be in pull else block to not set pull to 0
        if ((Input::instance()
               ->getPlayerKey(INPUT_FLIPRIGHT, p)
               ->isPressed(v_keystate, v_mousestate, numkeys) &&
             XMSession::instance()->mirrorMode() == false) ||
            (Input::instance()
               ->getPlayerKey(INPUT_FLIPLEFT, p)
               ->isPressed(v_keystate, v_mousestate, numkeys) &&
             XMSession::instance()->mirrorMode())) {
          /* Push forward */
          v_biker->getControler()->setPull(-1.0f);
        } else {
          v_biker->getControler()->setPull(0.0f);
        }
      }

      if (Input::instance()
            ->getPlayerKey(INPUT_CHANGEDIR, p)
            ->isPressed(v_keystate, v_mousestate, numkeys)
          /*
           * Make sure we aren't holding down the A button, as this would cause
           * the player to change directions instantly after opening the level
           * (or after skipping the initial zoom) or restarting/resuming from
           * the pause menu, as this likely isn't wanted when playing with a
           * controller. Note that this behavior still stands for the spacebar.
           */
          && !(Input::instance()
                 ->getPlayerKey(INPUT_CHANGEDIR, p)
                 ->getJoyButton() == SDL_CONTROLLER_BUTTON_A)) {
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
