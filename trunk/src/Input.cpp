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
#include <sstream>
#include "helpers/VExcept.h"
#include "helpers/Log.h"
#include "db/xmDatabase.h"

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
void InputHandler::init(UserConfig *pConfig, xmDatabase* pDb, const std::string& i_id_profile, bool i_enableJoysticks) {
    /* Initialize joysticks (if any) */
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);

    enableJoysticks(i_enableJoysticks);
    
    /* Enable unicode translation and key repeats */
    SDL_EnableUNICODE(1);         
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);

    /* Open all joysticks */
    recheckJoysticks();
    loadConfig(pConfig, pDb, i_id_profile);
  }

  void InputHandler::uninit(void) {
    /* Close all joysticks */
    for(unsigned int i=0; i<m_Joysticks.size(); i++) {
      SDL_JoystickClose(m_Joysticks[i]);
    }

    /* No more joysticking */
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
  }

    /**
     * converts a raw joystick axis value to a float, according to specified minimum and maximum values, as well as the deadzone.
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
  float InputHandler::joyRawToFloat(float raw, float neg, float deadzone_neg, float deadzone_pos, float pos) {

      if (neg > pos) {
	std::swap(neg, pos);
	std::swap(deadzone_neg, deadzone_pos);
      }

      if (raw > pos) return +1.0f;
      if (raw > deadzone_pos) return +((raw - deadzone_pos) / (pos - deadzone_pos));
      if (raw < neg) return -1.0f;
      if (raw < deadzone_neg) return -((raw - deadzone_neg) / (neg - deadzone_neg));

      return 0.0f;
    }

  /*===========================================================================
  Read configuration
  ===========================================================================*/  
void InputHandler::loadConfig(UserConfig *pConfig, xmDatabase* pDb, const std::string& i_id_profile) {
  std::string v_key;

  /* Set defaults */
  setDefaultConfig();
  
  /* Get settings for mode */
  for(unsigned int i=0; i<INPUT_NB_PLAYERS; i++) {
    std::ostringstream v_n;
    v_n << (i+1);

    try {
      m_nDriveKey[i]        = XMKey(pDb->config_getString(i_id_profile, "KeyDrive"     + v_n.str(), m_nDriveKey[i].toString()));
    } catch(InvalidSystemKeyException &e) {
      /* keep default key */
    } catch(Exception &e) {
      m_nDriveKey[i] = XMKey(); // load default key (undefined) to not override the key while undefined keys are not saved to avoid config brake in case you forgot to plug your joystick
    }

    try {
      m_nBrakeKey[i]        = XMKey(pDb->config_getString(i_id_profile, "KeyBrake"     + v_n.str(), m_nBrakeKey[i].toString()));
    } catch(InvalidSystemKeyException &e) {
      /* keep default key */
    } catch(Exception &e) {
      m_nBrakeKey[i] = XMKey();
    }

    try {
      m_nPullBackKey[i]     = XMKey(pDb->config_getString(i_id_profile, "KeyFlipLeft"  + v_n.str(), m_nPullBackKey[i].toString()));
    } catch(InvalidSystemKeyException &e) {
      /* keep default key */
    } catch(Exception &e) {
      m_nPullBackKey[i] = XMKey();
    }

    try {
      m_nPushForwardKey[i]  = XMKey(pDb->config_getString(i_id_profile, "KeyFlipRight" + v_n.str(), m_nPushForwardKey[i].toString()));
    } catch(InvalidSystemKeyException &e) {
      /* keep default key */
    } catch(Exception &e) {
      m_nPushForwardKey[i] = XMKey();
    }

    try {
      m_nChangeDirKey[i]    = XMKey(pDb->config_getString(i_id_profile, "KeyChangeDir" + v_n.str(), m_nChangeDirKey[i].toString()));
    } catch(InvalidSystemKeyException &e) {
      /* keep default key */
    } catch(Exception &e) {
      m_nChangeDirKey[i] = XMKey();
    }
    
    for(unsigned int k=0; k<MAX_SCRIPT_KEY_HOOKS; k++) {
      std::ostringstream v_k;
      v_k << (k);
      
      v_key = pDb->config_getString(i_id_profile, "KeyActionScript" + v_n.str() + "_" + v_k.str(), "");
      if(v_key != "") { // don't override the default key if there is nothing in the config
	try {
	  m_nScriptActionKeys[i][k] = XMKey(v_key);
	} catch(InvalidSystemKeyException &e) {
	  /* keep default key */
	} catch(Exception &e) {
	  m_nScriptActionKeys[i][k] = XMKey();
	}
      }
    }
  }

  //
  try {
    m_switchUglyMode = XMKey(pDb->config_getString(i_id_profile, "KeySwitchUglyMode", m_switchUglyMode.toString()));
  } catch(InvalidSystemKeyException &e) {
    /* keep default key */
  } catch(Exception &e) {
    m_switchUglyMode = XMKey();
  }

  try {
    m_switchBlacklist = XMKey(pDb->config_getString(i_id_profile, "KeySwitchBlacklist", m_switchBlacklist.toString()));
  } catch(InvalidSystemKeyException &e) {
    /* keep default key */
  } catch(Exception &e) {
    m_switchBlacklist = XMKey();
  }

  try {
    m_switchFavorite = XMKey(pDb->config_getString(i_id_profile, "KeySwitchFavorite", m_switchFavorite.toString()));
  } catch(InvalidSystemKeyException &e) {
    /* keep default key */
  } catch(Exception &e) {
    m_switchFavorite = XMKey();
  }

  try {
    m_restartLevel = XMKey(pDb->config_getString(i_id_profile, "KeyRestartLevel", m_restartLevel.toString()));
  } catch(InvalidSystemKeyException &e) {
    /* keep default key */
  } catch(Exception &e) {
    m_restartLevel = XMKey();
  }

  try {
    m_showConsole = XMKey(pDb->config_getString(i_id_profile, "KeyShowConsole", m_showConsole.toString()));
  } catch(InvalidSystemKeyException &e) {
    /* keep default key */
  } catch(Exception &e) {
    m_showConsole = XMKey();
  }

}
  
  /*===========================================================================
  Add script key hook
  ===========================================================================*/  
  void InputHandler::addScriptKeyHook(Scene *pGame,const std::string &basicKeyName,const std::string &FuncName) {
    if(m_nNumScriptKeyHooks < MAX_SCRIPT_KEY_HOOKS) {
      m_ScriptKeyHooks[m_nNumScriptKeyHooks].FuncName = FuncName;
      m_ScriptKeyHooks[m_nNumScriptKeyHooks].nKey = XMKey(basicKeyName, true);
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

XMKey InputHandler::getScriptActionKeys(int i_player, int i_actionScript) const {
	return m_nScriptActionKeys[i_player][i_actionScript];
}

std::string* InputHandler::getJoyId(Uint8 i_joynum) {
  return &(m_JoysticksIds[i_joynum]);
}

Uint8 InputHandler::getJoyNum(const std::string& i_name) {
  for(unsigned int i=0; i<m_JoysticksIds.size(); i++) {
    if(m_JoysticksIds[i] == i_name) {
      return i;
    }
  }
  throw Exception("Invalid joystick name");
}

std::string* InputHandler::getJoyIdByStrId(const std::string& i_name) {
  for(unsigned int i=0; i<m_JoysticksIds.size(); i++) {
    if(m_JoysticksIds[i] == i_name) {
      return &(m_JoysticksIds[i]);
    }
  }
  throw Exception("Invalid joystick name");
}

SDL_Joystick* InputHandler::getJoyById(std::string* i_id) {
  for(unsigned int i=0; i<m_JoysticksIds.size(); i++) {
    if( &(m_JoysticksIds[i]) == i_id) {
      return m_Joysticks[i];
    }
  }
  throw Exception("Invalid joystick id");
}

InputEventType InputHandler::joystickAxisSens(Sint16 m_joyAxisValue) {
  return abs(m_joyAxisValue) < INPUT_JOYSTICK_MINIMUM_DETECTION ? INPUT_UP : INPUT_DOWN;
}

  /*===========================================================================
  Set totally default configuration - useful for when something goes wrong
  ===========================================================================*/  
  void InputHandler::setDefaultConfig() {    
    m_nDriveKey[0]       = XMKey(SDLK_UP,    KMOD_NONE);
    m_nBrakeKey[0]       = XMKey(SDLK_DOWN,  KMOD_NONE);
    m_nPullBackKey[0]    = XMKey(SDLK_LEFT,  KMOD_NONE);
    m_nPushForwardKey[0] = XMKey(SDLK_RIGHT, KMOD_NONE);
    m_nChangeDirKey[0]   = XMKey(SDLK_SPACE, KMOD_NONE);

    m_nDriveKey[1]       = XMKey(SDLK_a, KMOD_NONE);
    m_nBrakeKey[1]       = XMKey(SDLK_q, KMOD_NONE);
    m_nPullBackKey[1]    = XMKey(SDLK_z, KMOD_NONE);
    m_nPushForwardKey[1] = XMKey(SDLK_e, KMOD_NONE);
    m_nChangeDirKey[1]   = XMKey(SDLK_w, KMOD_NONE);

    m_nDriveKey[2]       = XMKey(SDLK_r, KMOD_NONE);
    m_nBrakeKey[2]       = XMKey(SDLK_f, KMOD_NONE);
    m_nPullBackKey[2]    = XMKey(SDLK_t, KMOD_NONE);
    m_nPushForwardKey[2] = XMKey(SDLK_y, KMOD_NONE);
    m_nChangeDirKey[2]   = XMKey(SDLK_v, KMOD_NONE);

    m_nDriveKey[3]       = XMKey(SDLK_y, KMOD_NONE);
    m_nBrakeKey[3]       = XMKey(SDLK_h, KMOD_NONE);
    m_nPullBackKey[3]    = XMKey(SDLK_u, KMOD_NONE);
    m_nPushForwardKey[3] = XMKey(SDLK_i, KMOD_NONE);
    m_nChangeDirKey[3]   = XMKey(SDLK_n, KMOD_NONE);
    
    m_switchUglyMode     = XMKey(SDLK_F9, KMOD_NONE);
    m_switchBlacklist    = XMKey(SDLK_b,  KMOD_LCTRL);
    m_switchFavorite     = XMKey(SDLK_F3, KMOD_NONE);
    m_restartLevel       = XMKey(SDLK_RETURN,   KMOD_NONE);
    m_showConsole        = XMKey(SDLK_WORLD_18, KMOD_NONE);
  }  

  /*===========================================================================
  Get key by action...
  ===========================================================================*/  

  std::string InputHandler::getFancyKeyByAction(const std::string &Action) {
    for(unsigned int i=0; i<INPUT_NB_PLAYERS; i++) {
      std::ostringstream v_n;
      
      if(i !=0) { // nothing for player 0
	v_n << " " << (i+1);
      }
      if(Action == "Drive"       + v_n.str()) return m_nDriveKey[i].toFancyString();
      if(Action == "Brake"       + v_n.str()) return m_nBrakeKey[i].toFancyString();
      if(Action == "PullBack"    + v_n.str()) return m_nPullBackKey[i].toFancyString();
      if(Action == "PushForward" + v_n.str()) return m_nPushForwardKey[i].toFancyString();
      if(Action == "ChangeDir"   + v_n.str()) return m_nChangeDirKey[i].toFancyString();
    }
    return "?";
  }

void InputHandler::saveConfig(UserConfig *pConfig, xmDatabase* pDb, const std::string& i_id_profile) {
  pDb->config_setValue_begin();

  for(unsigned int i=0; i<INPUT_NB_PLAYERS; i++) {
    std::ostringstream v_n;
    v_n << (i+1);

    if(m_nDriveKey[i].isDefined()) {
      pDb->config_setString(i_id_profile, "KeyDrive"     + v_n.str(), m_nDriveKey[i].toString()      );
    }
    if(m_nBrakeKey[i].isDefined()) {
      pDb->config_setString(i_id_profile, "KeyBrake"     + v_n.str(), m_nBrakeKey[i].toString()      );
    }
    if(m_nPullBackKey[i].isDefined()) {
      pDb->config_setString(i_id_profile, "KeyFlipLeft"  + v_n.str(), m_nPullBackKey[i].toString()   );
    }
    if(m_nPushForwardKey[i].isDefined()) {
      pDb->config_setString(i_id_profile, "KeyFlipRight" + v_n.str(), m_nPushForwardKey[i].toString());
    }
    if(m_nChangeDirKey[i].isDefined()) {
      pDb->config_setString(i_id_profile, "KeyChangeDir" + v_n.str(), m_nChangeDirKey[i].toString()  );
    }

    for(unsigned int k=0; k<MAX_SCRIPT_KEY_HOOKS; k++) {
      if(m_nScriptActionKeys[i][k].isDefined()) {
	std::ostringstream v_k;
	v_k << (k);

	pDb->config_setString(i_id_profile, "KeyActionScript" + v_n.str() + "_" + v_k.str(), m_nScriptActionKeys[i][k].toString());
      }
    }
  }

  if(m_switchUglyMode.isDefined()) {
    pDb->config_setString(i_id_profile, "KeySwitchUglyMode", m_switchUglyMode.toString());
  }

  if(m_switchBlacklist.isDefined()) {
    pDb->config_setString(i_id_profile, "KeySwitchBlacklist", m_switchBlacklist.toString());
  }

  if(m_switchFavorite.isDefined()) {
    pDb->config_setString(i_id_profile, "KeySwitchFavorite", m_switchFavorite.toString());
  }

  if(m_restartLevel.isDefined()) {
    pDb->config_setString(i_id_profile, "KeyRestartLevel", m_restartLevel.toString());
  }

  if(m_showConsole.isDefined()) {
    pDb->config_setString(i_id_profile, "KeyShowConsole", m_showConsole.toString());
  }

  pDb->config_setValue_end();
}

void InputHandler::setDRIVE(int i_player, XMKey i_value) {
  m_nDriveKey[i_player] = i_value;
}

XMKey InputHandler::getDRIVE(int i_player) const {
  return m_nDriveKey[i_player];
}

void InputHandler::setBRAKE(int i_player, XMKey i_value) {
  m_nBrakeKey[i_player] = i_value;
}

XMKey InputHandler::getBRAKE(int i_player) const {
  return m_nBrakeKey[i_player];
}

void InputHandler::setFLIPLEFT(int i_player, XMKey i_value) {
  m_nPullBackKey[i_player] = i_value;
}

XMKey InputHandler::getFLIPLEFT(int i_player) const {
  return m_nPullBackKey[i_player];
}

void InputHandler::setFLIPRIGHT(int i_player, XMKey i_value) {
  m_nPushForwardKey[i_player] = i_value;
}

XMKey InputHandler::getFLIPRIGHT(int i_player) const {
  return m_nPushForwardKey[i_player];
}

void InputHandler::setCHANGEDIR(int i_player, XMKey i_value) {
  m_nChangeDirKey[i_player] = i_value;
}

XMKey InputHandler::getCHANGEDIR(int i_player) const {
  return m_nChangeDirKey[i_player];
}

void InputHandler::setSCRIPTACTION(int i_player, int i_action, XMKey i_value) {
  m_nScriptActionKeys[i_player][i_action] = i_value;
}

XMKey InputHandler::getSCRIPTACTION(int i_player, int i_action) const {
  return m_nScriptActionKeys[i_player][i_action];
}

void InputHandler::setSwitchUglyMode(XMKey i_value) {
  m_switchUglyMode = i_value;
}

XMKey InputHandler::getSwitchUglyMode() const {
  return m_switchUglyMode;
}

void InputHandler::setSwitchBlacklist(XMKey i_value) {
  m_switchBlacklist = i_value;
}

XMKey InputHandler::getSwitchBlacklist() const {
  return m_switchBlacklist;
}

void InputHandler::setSwitchFavorite(XMKey i_value) {
  m_switchFavorite = i_value;
}

XMKey InputHandler::getSwitchFavorite() const {
  return m_switchFavorite;
}

void InputHandler::setRestartLevel(XMKey i_value) {
  m_restartLevel = i_value;
}

XMKey InputHandler::getRestartLevel() const {
  return m_restartLevel;
}

void InputHandler::setShowConsole(XMKey i_value) {
  m_showConsole = i_value;
}

XMKey InputHandler::getShowConsole() const {
  return m_showConsole;
}

bool InputHandler::isANotGameSetKey(XMKey* i_xmkey) const {
  for(unsigned int i=0; i<INPUT_NB_PLAYERS; i++) {
    if(getDRIVE(i)     == *i_xmkey) return false;
    if(getBRAKE(i)     == *i_xmkey) return false;
    if(getFLIPLEFT(i)  == *i_xmkey) return false;
    if(getFLIPRIGHT(i) == *i_xmkey) return false;
    if(getCHANGEDIR(i) == *i_xmkey) return false;

    for(unsigned int k=0; k<MAX_SCRIPT_KEY_HOOKS; k++) {
      if(m_nScriptActionKeys[i][k] == *i_xmkey) return false;
    }
  }
  return true;
}

void InputHandler::recheckJoysticks() {
  std::string v_joyName, v_joyId;
  int n;
  bool v_continueToOpen = true;
  SDL_Joystick* v_joystick;

  m_Joysticks.clear();
  m_JoysticksNames.clear();
  m_JoysticksIds.clear();

  for(int i=0; i<SDL_NumJoysticks(); i++) {
    if(v_continueToOpen) {
      if( (v_joystick = SDL_JoystickOpen(i)) != NULL) {
	std::ostringstream v_id;
	n = 0;
	v_joyName = SDL_JoystickName(i);
	
	// check if there is an other joystick with the same name
	for(unsigned int j=0; j<m_Joysticks.size(); j++) {
	  if(m_JoysticksNames[j] == v_joyName) {
	    n++;
	  }
	}
	  
	if(n > 0) {
	  v_id << " " << (n+1); // +1 to get an id name starting at 1
	}
	v_joyId = v_joyName + v_id.str();
	m_Joysticks.push_back(v_joystick);
	m_JoysticksNames.push_back(v_joyName);
	m_JoysticksIds.push_back(v_joyId);
	
	LogInfo("Joystick found [%s], id is [%s]", v_joyName.c_str(), v_joyId.c_str());
      } else {
	v_continueToOpen = false; // don't continue to open joystick to keep m_joysticks[joystick.num] working
	LogWarning("fail to open joystick [%s], abort to open other joysticks", v_joyName.c_str());
      }
    }
  }
}

std::vector<std::string>& InputHandler::getJoysticksNames() {
  return m_JoysticksNames;
}
