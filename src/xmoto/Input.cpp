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
 *  Input handler
 */
#include "Input.h"
#include "GameText.h"
#include "db/xmDatabase.h"
#include "helpers/Log.h"
#include "helpers/VExcept.h"
#include <sstream>

InputHandler::InputHandler() {
  reset();
}

void InputHandler::reset() {
  resetScriptKeyHooks();
}

bool InputHandler::areJoysticksEnabled() const {
  return SDL_JoystickEventState(SDL_QUERY) == SDL_ENABLE;
}

void InputHandler::enableJoysticks(bool i_value) {
  SDL_JoystickEventState(i_value ? SDL_ENABLE : SDL_IGNORE);
}

/*===========================================================================
Init/uninit
===========================================================================*/
void InputHandler::init(UserConfig *pConfig,
                        xmDatabase *pDb,
                        const std::string &i_id_profile,
                        bool i_enableJoysticks) {
  /* Initialize joysticks (if any) */
  SDL_InitSubSystem(SDL_INIT_JOYSTICK);

  enableJoysticks(i_enableJoysticks);

  // TODO:
  //SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  /* Open all joysticks */
  recheckJoysticks();
  loadConfig(pConfig, pDb, i_id_profile);
}

void InputHandler::uninit(void) {
  /* Close all joysticks */
  for (unsigned int i = 0; i < m_Joysticks.size(); i++) {
    SDL_JoystickClose(m_Joysticks[i]);
  }

  /* No more joysticking */
  SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

/**
 * converts a raw joystick axis value to a float, according to specified minimum
 * and maximum values, as well as the deadzone.
 *
 *                 (+)      ____
 *           result |      /|
 *                  |     / |
 *                  |    /  |
 *  (-)________ ____|___/___|____(+)
 *             /|   |   |   |    input
 *            / |   |   |   |
 *           /  |   |   |   |
 *     _____/   |   |   |   |
 *          |   |  (-)  |   |
 *         neg  dead-zone  pos
 *
 */
float InputHandler::joyRawToFloat(float raw,
                                  float neg,
                                  float deadzone_neg,
                                  float deadzone_pos,
                                  float pos) {
  if (neg > pos) {
    std::swap(neg, pos);
    std::swap(deadzone_neg, deadzone_pos);
  }

  if (raw > pos)
    return +1.0f;
  if (raw > deadzone_pos)
    return +((raw - deadzone_pos) / (pos - deadzone_pos));
  if (raw < neg)
    return -1.0f;
  if (raw < deadzone_neg)
    return -((raw - deadzone_neg) / (neg - deadzone_neg));

  return 0.0f;
}

/*===========================================================================
Read configuration
===========================================================================*/
void InputHandler::loadConfig(UserConfig *pConfig,
                              xmDatabase *pDb,
                              const std::string &i_id_profile) {
  std::string v_key;

  /* Set defaults */
  setDefaultConfig();

  /* Get settings for mode */
  for (unsigned int i = 0; i < INPUT_NB_PLAYERS; i++) {
    std::ostringstream v_n;
    v_n << (i + 1);

    for (unsigned int j = 0; j < INPUT_NB_PLAYERKEYS; j++) {
      try {
        m_playerKeys[i][j].key =
          XMKey(pDb->config_getString(i_id_profile,
                                      m_playerKeys[i][j].name + v_n.str(),
                                      m_playerKeys[i][j].key.toString()));
      } catch (InvalidSystemKeyException &e) {
        /* keep default key */
      } catch (Exception &e) {
        m_playerKeys[i][j].key =
          XMKey(); // load default key (undefined) to not override the key while
        // undefined keys are not saved to avoid config brake in case
        // you forgot to plug your joystick
      }
    }

    // script keys
    for (unsigned int k = 0; k < MAX_SCRIPT_KEY_HOOKS; k++) {
      std::ostringstream v_k;
      v_k << (k);

      v_key = pDb->config_getString(
        i_id_profile, "KeyActionScript" + v_n.str() + "_" + v_k.str(), "");
      if (v_key != "") { // don't override the default key if there is nothing
        // in the config
        try {
          m_nScriptActionKeys[i][k] = XMKey(v_key);
        } catch (InvalidSystemKeyException &e) {
          /* keep default key */
        } catch (Exception &e) {
          m_nScriptActionKeys[i][k] = XMKey();
        }
      }
    }
  }

  // global keys
  for (unsigned int i = 0; i < INPUT_NB_GLOBALKEYS; i++) {
    try {
      m_globalKeys[i].key = XMKey(pDb->config_getString(
        i_id_profile, m_globalKeys[i].name, m_globalKeys[i].key.toString()));
    } catch (InvalidSystemKeyException &e) {
      /* keep default key */
    } catch (Exception &e) {
      m_globalKeys[i].key = XMKey();
    }
  }
}

/*===========================================================================
Add script key hook
===========================================================================*/
void InputHandler::addScriptKeyHook(Scene *pGame,
                                    const std::string &keyName,
                                    const std::string &FuncName) {
  if (m_nNumScriptKeyHooks < MAX_SCRIPT_KEY_HOOKS) {
    m_ScriptKeyHooks[m_nNumScriptKeyHooks].FuncName = FuncName;

    if (keyName.size() == 1) { /* old basic mode */
      m_ScriptKeyHooks[m_nNumScriptKeyHooks].nKey = XMKey(keyName, true);
    } else {
      m_ScriptKeyHooks[m_nNumScriptKeyHooks].nKey = XMKey(keyName);
    }
    m_ScriptKeyHooks[m_nNumScriptKeyHooks].pGame = pGame;
    m_nNumScriptKeyHooks++;
  }
}

int InputHandler::getNumScriptKeyHooks() const {
  return m_nNumScriptKeyHooks;
}

InputScriptKeyHook InputHandler::getScriptKeyHooks(int i) const {
  return m_ScriptKeyHooks[i];
}

XMKey InputHandler::getScriptActionKeys(int i_player,
                                        int i_actionScript) const {
  return m_nScriptActionKeys[i_player][i_actionScript];
}

std::string *InputHandler::getJoyId(Uint8 i_joynum) {
  return &(m_JoysticksIds[i_joynum]);
}

Uint8 InputHandler::getJoyNum(const std::string &i_name) {
  for (unsigned int i = 0; i < m_JoysticksIds.size(); i++) {
    if (m_JoysticksIds[i] == i_name) {
      return i;
    }
  }
  throw Exception("Invalid joystick name");
}

std::string *InputHandler::getJoyIdByStrId(const std::string &i_name) {
  for (unsigned int i = 0; i < m_JoysticksIds.size(); i++) {
    if (m_JoysticksIds[i] == i_name) {
      return &(m_JoysticksIds[i]);
    }
  }
  throw Exception("Invalid joystick name");
}

SDL_Joystick *InputHandler::getJoyById(std::string *i_id) {
  for (unsigned int i = 0; i < m_JoysticksIds.size(); i++) {
    if (&(m_JoysticksIds[i]) == i_id) {
      return m_Joysticks[i];
    }
  }
  throw Exception("Invalid joystick id");
}

InputEventType InputHandler::joystickAxisSens(Sint16 m_joyAxisValue) {
  return abs(m_joyAxisValue) < INPUT_JOYSTICK_MINIMUM_DETECTION ? INPUT_UP
                                                                : INPUT_DOWN;
}

/*===========================================================================
Set totally default configuration - useful for when something goes wrong
===========================================================================*/
void InputHandler::setDefaultConfig() {
  m_playerKeys[0][INPUT_DRIVE] =
    IFullKey("KeyDrive", XMKey(SDLK_UP, KMOD_NONE), GAMETEXT_DRIVE);
  m_playerKeys[0][INPUT_BRAKE] =
    IFullKey("KeyBrake", XMKey(SDLK_DOWN, KMOD_NONE), GAMETEXT_BRAKE);
  m_playerKeys[0][INPUT_FLIPLEFT] =
    IFullKey("KeyFlipLeft", XMKey(SDLK_LEFT, KMOD_NONE), GAMETEXT_FLIPLEFT);
  m_playerKeys[0][INPUT_FLIPRIGHT] =
    IFullKey("KeyFlipRight", XMKey(SDLK_RIGHT, KMOD_NONE), GAMETEXT_FLIPRIGHT);
  m_playerKeys[0][INPUT_CHANGEDIR] =
    IFullKey("KeyChangeDir", XMKey(SDLK_SPACE, KMOD_NONE), GAMETEXT_CHANGEDIR);

  m_playerKeys[1][INPUT_DRIVE] =
    IFullKey("KeyDrive", XMKey(SDLK_a, KMOD_NONE), GAMETEXT_DRIVE);
  m_playerKeys[1][INPUT_BRAKE] =
    IFullKey("KeyBrake", XMKey(SDLK_q, KMOD_NONE), GAMETEXT_BRAKE);
  m_playerKeys[1][INPUT_FLIPLEFT] =
    IFullKey("KeyFlipLeft", XMKey(SDLK_z, KMOD_NONE), GAMETEXT_FLIPLEFT);
  m_playerKeys[1][INPUT_FLIPRIGHT] =
    IFullKey("KeyFlipRight", XMKey(SDLK_e, KMOD_NONE), GAMETEXT_FLIPRIGHT);
  m_playerKeys[1][INPUT_CHANGEDIR] =
    IFullKey("KeyChangeDir", XMKey(SDLK_w, KMOD_NONE), GAMETEXT_CHANGEDIR);

  m_playerKeys[2][INPUT_DRIVE] =
    IFullKey("KeyDrive", XMKey(SDLK_r, KMOD_NONE), GAMETEXT_DRIVE);
  m_playerKeys[2][INPUT_BRAKE] =
    IFullKey("KeyBrake", XMKey(SDLK_f, KMOD_NONE), GAMETEXT_BRAKE);
  m_playerKeys[2][INPUT_FLIPLEFT] =
    IFullKey("KeyFlipLeft", XMKey(SDLK_t, KMOD_NONE), GAMETEXT_FLIPLEFT);
  m_playerKeys[2][INPUT_FLIPRIGHT] =
    IFullKey("KeyFlipRight", XMKey(SDLK_y, KMOD_NONE), GAMETEXT_FLIPRIGHT);
  m_playerKeys[2][INPUT_CHANGEDIR] =
    IFullKey("KeyChangeDir", XMKey(SDLK_v, KMOD_NONE), GAMETEXT_CHANGEDIR);

  m_playerKeys[3][INPUT_DRIVE] =
    IFullKey("KeyDrive", XMKey(SDLK_u, KMOD_NONE), GAMETEXT_DRIVE);
  m_playerKeys[3][INPUT_BRAKE] =
    IFullKey("KeyBrake", XMKey(SDLK_j, KMOD_NONE), GAMETEXT_BRAKE);
  m_playerKeys[3][INPUT_FLIPLEFT] =
    IFullKey("KeyFlipLeft", XMKey(SDLK_i, KMOD_NONE), GAMETEXT_FLIPLEFT);
  m_playerKeys[3][INPUT_FLIPRIGHT] =
    IFullKey("KeyFlipRight", XMKey(SDLK_o, KMOD_NONE), GAMETEXT_FLIPRIGHT);
  m_playerKeys[3][INPUT_CHANGEDIR] =
    IFullKey("KeyChangeDir", XMKey(SDLK_k, KMOD_NONE), GAMETEXT_CHANGEDIR);

  m_globalKeys[INPUT_SWITCHUGLYMODE] = IFullKey(
    "KeySwitchUglyMode", XMKey(SDLK_F9, KMOD_NONE), GAMETEXT_SWITCHUGLYMODE);
  m_globalKeys[INPUT_SWITCHBLACKLIST] = IFullKey(
    "KeySwitchBlacklist", XMKey(SDLK_b, KMOD_LCTRL), GAMETEXT_SWITCHBLACKLIST);
  m_globalKeys[INPUT_SWITCHFAVORITE] = IFullKey(
    "KeySwitchFavorite", XMKey(SDLK_F3, KMOD_NONE), GAMETEXT_SWITCHFAVORITE);
  m_globalKeys[INPUT_RESTARTLEVEL] = IFullKey(
    "KeyRestartLevel", XMKey(SDLK_RETURN, KMOD_NONE), GAMETEXT_RESTARTLEVEL);
  m_globalKeys[INPUT_SHOWCONSOLE] = IFullKey(
    "KeyShowConsole", XMKey(SDL_SCANCODE_GRAVE, KMOD_NONE), GAMETEXT_SHOWCONSOLE);
  m_globalKeys[INPUT_CONSOLEHISTORYPLUS] =
    IFullKey("KeyConsoleHistoryPlus",
             XMKey(SDLK_PLUS, KMOD_LCTRL),
             GAMETEXT_CONSOLEHISTORYPLUS);
  m_globalKeys[INPUT_CONSOLEHISTORYMINUS] =
    IFullKey("KeyConsoleHistoryMinus",
             XMKey(SDLK_MINUS, KMOD_LCTRL),
             GAMETEXT_CONSOLEHISTORYMINUS);
  m_globalKeys[INPUT_RESTARTCHECKPOINT] =
    IFullKey("KeyRestartCheckpoint",
             XMKey(SDLK_BACKSPACE, KMOD_NONE),
             GAMETEXT_RESTARTCHECKPOINT);
  m_globalKeys[INPUT_CHAT] =
    IFullKey("KeyChat", XMKey(SDLK_c, KMOD_LCTRL), GAMETEXT_CHATDIALOG);
  m_globalKeys[INPUT_CHATPRIVATE] = IFullKey(
    "KeyChatPrivate", XMKey(SDLK_p, KMOD_LCTRL), GAMETEXT_CHATPRIVATEDIALOG);
  m_globalKeys[INPUT_LEVELWATCHING] = IFullKey(
    "KeyLevelWatching", XMKey(SDLK_TAB, KMOD_NONE), GAMETEXT_LEVELWATCHING);
  m_globalKeys[INPUT_SWITCHPLAYER] = IFullKey(
    "KeySwitchPlayer", XMKey(SDLK_F2, KMOD_NONE), GAMETEXT_SWITCHPLAYER);
  m_globalKeys[INPUT_SWITCHTRACKINGSHOTMODE] =
    IFullKey("KeySwitchTrackingshotMode",
             XMKey(SDLK_F4, KMOD_NONE),
             GAMETEXT_SWITCHTRACKINGSHOTMODE);
  m_globalKeys[INPUT_NEXTLEVEL] =
    IFullKey("KeyNextLevel", XMKey(SDLK_PAGEUP, KMOD_NONE), GAMETEXT_NEXTLEVEL);
  m_globalKeys[INPUT_PREVIOUSLEVEL] = IFullKey("KeyPreviousLevel",
                                               XMKey(SDLK_PAGEDOWN, KMOD_NONE),
                                               GAMETEXT_PREVIOUSLEVEL);
  m_globalKeys[INPUT_SWITCHRENDERGHOSTTRAIL] =
    IFullKey("KeySwitchRenderGhosttrail",
             XMKey(SDLK_g, KMOD_LCTRL),
             GAMETEXT_SWITCHREDERGHOSTTRAIL);
  m_globalKeys[INPUT_SCREENSHOT] =
    IFullKey("KeyScreenshot", XMKey(SDLK_F12, KMOD_NONE), GAMETEXT_SCREENSHOT);
  m_globalKeys[INPUT_SWITCHWWWACCESS] = IFullKey(
    "KeySwitchWWWAccess", XMKey(SDLK_F8, KMOD_NONE), GAMETEXT_SWITCHWWWACCESS);
  m_globalKeys[INPUT_SWITCHFPS] =
    IFullKey("KeySwitchFPS", XMKey(SDLK_F7, KMOD_NONE), GAMETEXT_SWITCHFPS);
  m_globalKeys[INPUT_SWITCHGFXQUALITYMODE] =
    IFullKey("KeySwitchGFXQualityMode",
             XMKey(SDLK_F10, KMOD_NONE),
             GAMETEXT_SWITCHGFXQUALITYMODE);
  m_globalKeys[INPUT_SWITCHGFXMODE] = IFullKey(
    "KeySwitchGFXMode", XMKey(SDLK_F11, KMOD_NONE), GAMETEXT_SWITCHGFXMODE);
  m_globalKeys[INPUT_SWITCHNETMODE] = IFullKey(
    "KeySwitchNetMode", XMKey(SDLK_n, KMOD_LCTRL), GAMETEXT_SWITCHNETMODE);
  m_globalKeys[INPUT_SWITCHHIGHSCOREINFORMATION] =
    IFullKey("KeySwitchHighscoreInformation",
             XMKey(SDLK_w, KMOD_LCTRL),
             GAMETEXT_SWITCHHIGHSCOREINFORMATION);
  m_globalKeys[INPUT_NETWORKADMINCONSOLE] =
    IFullKey("KeyNetworkAdminConsole",
             XMKey(SDLK_s, (SDL_Keymod)(KMOD_LCTRL | KMOD_LALT)),
             GAMETEXT_NETWORKADMINCONSOLE);
  m_globalKeys[INPUT_SWITCHSAFEMODE] =
    IFullKey("KeySafeMode", XMKey(SDLK_F6, KMOD_NONE), GAMETEXT_SWITCHSAFEMODE);

  // uncustomizable keys
  m_globalKeys[INPUT_HELP] =
    IFullKey("KeyHelp", XMKey(SDLK_F1, KMOD_NONE), GAMETEXT_HELP, false);
  m_globalKeys[INPUT_RELOADFILESTODB] = IFullKey("KeyReloadFilesToDb",
                                                 XMKey(SDLK_F5, KMOD_NONE),
                                                 GAMETEXT_RELOADFILESTODB,
                                                 false);
  m_globalKeys[INPUT_PLAYINGPAUSE] =
    IFullKey("KeyPlayingPause",
             XMKey(SDLK_ESCAPE, KMOD_NONE),
             GAMETEXT_PLAYINGPAUSE,
             false); // don't set it to true while ESCAPE is not setable via the
  // option as a key
  m_globalKeys[INPUT_KILLPROCESS] = IFullKey(
    "KeyKillProcess", XMKey(SDLK_k, KMOD_LCTRL), GAMETEXT_KILLPROCESS, false);
  m_globalKeys[INPUT_REPLAYINGREWIND] = IFullKey("KeyReplayingRewind",
                                                 XMKey(SDLK_LEFT, KMOD_NONE),
                                                 GAMETEXT_REPLAYINGREWIND,
                                                 false);
  m_globalKeys[INPUT_REPLAYINGFORWARD] = IFullKey("KeyReplayingForward",
                                                  XMKey(SDLK_RIGHT, KMOD_NONE),
                                                  GAMETEXT_REPLAYINGFORWARD,
                                                  false);
  m_globalKeys[INPUT_REPLAYINGPAUSE] = IFullKey("KeyReplayingPause",
                                                XMKey(SDLK_SPACE, KMOD_NONE),
                                                GAMETEXT_REPLAYINGPAUSE,
                                                false);
  m_globalKeys[INPUT_REPLAYINGSTOP] = IFullKey("KeyReplayingStop",
                                               XMKey(SDLK_ESCAPE, KMOD_NONE),
                                               GAMETEXT_REPLAYINGSTOP,
                                               false);
  m_globalKeys[INPUT_REPLAYINGFASTER] = IFullKey("KeyReplayingFaster",
                                                 XMKey(SDLK_UP, KMOD_NONE),
                                                 GAMETEXT_REPLAYINGFASTER,
                                                 false);
  m_globalKeys[INPUT_REPLAYINGABITFASTER] =
    IFullKey("KeyReplayingABitFaster",
             XMKey(SDLK_UP, KMOD_LCTRL),
             GAMETEXT_REPLAYINGABITFASTER,
             false);
  m_globalKeys[INPUT_REPLAYINGSLOWER] = IFullKey("KeyReplayingSlower",
                                                 XMKey(SDLK_DOWN, KMOD_NONE),
                                                 GAMETEXT_REPLAYINGSLOWER,
                                                 false);
  m_globalKeys[INPUT_REPLAYINGABITSLOWER] =
    IFullKey("KeyReplayingABitSlower",
             XMKey(SDLK_DOWN, KMOD_LCTRL),
             GAMETEXT_REPLAYINGABITSLOWER,
             false);
}

/*===========================================================================
Get key by action...
===========================================================================*/

std::string InputHandler::getKeyByAction(const std::string &Action,
                                         bool i_tech) {
  for (unsigned int i = 0; i < INPUT_NB_PLAYERS; i++) {
    std::ostringstream v_n;

    if (i != 0) { // nothing for player 0
      v_n << " " << (i + 1);
    }

    if (Action == "Drive" + v_n.str())
      return i_tech ? m_playerKeys[i][INPUT_DRIVE].key.toString()
                    : m_playerKeys[i][INPUT_DRIVE].key.toFancyString();
    if (Action == "Brake" + v_n.str())
      return i_tech ? m_playerKeys[i][INPUT_BRAKE].key.toString()
                    : m_playerKeys[i][INPUT_BRAKE].key.toFancyString();
    if (Action == "PullBack" + v_n.str())
      return i_tech ? m_playerKeys[i][INPUT_FLIPLEFT].key.toString()
                    : m_playerKeys[i][INPUT_FLIPLEFT].key.toFancyString();
    if (Action == "PushForward" + v_n.str())
      return i_tech ? m_playerKeys[i][INPUT_FLIPRIGHT].key.toString()
                    : m_playerKeys[i][INPUT_FLIPRIGHT].key.toFancyString();
    if (Action == "ChangeDir" + v_n.str())
      return i_tech ? m_playerKeys[i][INPUT_CHANGEDIR].key.toString()
                    : m_playerKeys[i][INPUT_CHANGEDIR].key.toFancyString();
  }
  return "?";
}

void InputHandler::saveConfig(UserConfig *pConfig,
                              xmDatabase *pDb,
                              const std::string &i_id_profile) {
  pDb->config_setValue_begin();

  for (unsigned int i = 0; i < INPUT_NB_PLAYERS; i++) {
    std::ostringstream v_n;
    v_n << (i + 1);

    // player keys
    for (unsigned int j = 0; j < INPUT_NB_PLAYERKEYS; j++) {
      if (m_playerKeys[i][j].key.isDefined()) {
        pDb->config_setString(i_id_profile,
                              m_playerKeys[i][j].name + v_n.str(),
                              m_playerKeys[i][j].key.toString());
      }
    }

    // script keys
    for (unsigned int k = 0; k < MAX_SCRIPT_KEY_HOOKS; k++) {
      if (m_nScriptActionKeys[i][k].isDefined()) {
        std::ostringstream v_k;
        v_k << (k);

        pDb->config_setString(i_id_profile,
                              "KeyActionScript" + v_n.str() + "_" + v_k.str(),
                              m_nScriptActionKeys[i][k].toString());
      }
    }
  }

  for (unsigned int i = 0; i < INPUT_NB_GLOBALKEYS; i++) {
    pDb->config_setString(
      i_id_profile, m_globalKeys[i].name, m_globalKeys[i].key.toString());
  }

  pDb->config_setValue_end();
}

void InputHandler::setSCRIPTACTION(int i_player, int i_action, XMKey i_value) {
  m_nScriptActionKeys[i_player][i_action] = i_value;
}

XMKey InputHandler::getSCRIPTACTION(int i_player, int i_action) const {
  return m_nScriptActionKeys[i_player][i_action];
}

void InputHandler::setGlobalKey(unsigned int INPUT_key, XMKey i_value) {
  m_globalKeys[INPUT_key].key = i_value;
}

const XMKey *InputHandler::getGlobalKey(unsigned int INPUT_key) const {
  return &(m_globalKeys[INPUT_key].key);
}

std::string InputHandler::getGlobalKeyHelp(unsigned int INPUT_key) const {
  return m_globalKeys[INPUT_key].help;
}

bool InputHandler::getGlobalKeyCustomizable(unsigned int INPUT_key) const {
  return m_globalKeys[INPUT_key].customizable;
}

void InputHandler::setPlayerKey(unsigned int INPUT_key,
                                int i_player,
                                XMKey i_value) {
  m_playerKeys[i_player][INPUT_key].key = i_value;
}

const XMKey *InputHandler::getPlayerKey(unsigned int INPUT_key,
                                        int i_player) const {
  return &(m_playerKeys[i_player][INPUT_key].key);
}

std::string InputHandler::getPlayerKeyHelp(unsigned int INPUT_key,
                                           int i_player) const {
  return m_playerKeys[i_player][INPUT_key].help;
}

bool InputHandler::isANotGameSetKey(XMKey *i_xmkey) const {
  for (unsigned int i = 0; i < INPUT_NB_PLAYERS; i++) {
    for (unsigned int j = 0; j < INPUT_NB_PLAYERKEYS; j++) {
      if ((*getPlayerKey(j, i)) == *i_xmkey) {
        return false;
      }
    }

    for (unsigned int k = 0; k < MAX_SCRIPT_KEY_HOOKS; k++) {
      if (m_nScriptActionKeys[i][k] == *i_xmkey)
        return false;
    }
  }
  return true;
}

void InputHandler::recheckJoysticks() {
  std::string v_joyName, v_joyId;
  int n;
  bool v_continueToOpen = true;
  SDL_Joystick *v_joystick;

  m_Joysticks.clear();
  m_JoysticksNames.clear();
  m_JoysticksIds.clear();

  for (int i = 0; i < SDL_NumJoysticks(); i++) {
    if (v_continueToOpen) {
      if ((v_joystick = SDL_JoystickOpen(i)) != NULL) {
        std::ostringstream v_id;
        n = 0;
        // TODO:
        //v_joyName = SDL_JoystickName(i);

        // check if there is an other joystick with the same name
        for (unsigned int j = 0; j < m_Joysticks.size(); j++) {
          if (m_JoysticksNames[j] == v_joyName) {
            n++;
          }
        }

        if (n > 0) {
          v_id << " " << (n + 1); // +1 to get an id name starting at 1
        }
        v_joyId = v_joyName + v_id.str();
        m_Joysticks.push_back(v_joystick);
        m_JoysticksNames.push_back(v_joyName);
        m_JoysticksIds.push_back(v_joyId);

        LogInfo("Joystick found [%s], id is [%s]",
                v_joyName.c_str(),
                v_joyId.c_str());
      } else {
        v_continueToOpen = false; // don't continue to open joystick to keep
        // m_joysticks[joystick.num] working
        LogWarning("fail to open joystick [%s], abort to open other joysticks",
                   v_joyName.c_str());
      }
    }
  }
}

std::vector<std::string> &InputHandler::getJoysticksNames() {
  return m_JoysticksNames;
}

IFullKey::IFullKey(const std::string &i_name,
                   const XMKey &i_key,
                   const std::string i_help,
                   bool i_customisable) {
  name = i_name;
  key = i_key;
  help = i_help;
  customizable = i_customisable;
}

IFullKey::IFullKey() {}
