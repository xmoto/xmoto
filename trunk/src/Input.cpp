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
#include "Game.h"
#include "GameText.h"
#include "LuaLibGame.h"
#include "helpers/Log.h"
#include "xmscene/Camera.h"
#include "xmscene/BikeController.h"
#include "Universe.h"
#include <sstream>

  InputHandler::InputHandler() {
    reset();
  }

  void InputHandler::reset() {
    m_mirrored = false;
    resetScriptKeyHooks();
    for(unsigned int i=0; i<INPUT_NB_PLAYERS; i++){
      m_changeDirKeyAlreadyPress[i] = false;
    }
  }

bool InputHandler::areJoysticksEnabled() const {
  return SDL_JoystickEventState(SDL_QUERY) == SDL_ENABLE;
}

void InputHandler::enableJoysticks(bool i_value) {
  SDL_JoystickEventState(i_value ? SDL_ENABLE : SDL_IGNORE);
}

  void InputHandler::dealWithActivedKeys(Universe* i_universe) {
    Uint8 *v_keystate  = SDL_GetKeyState(NULL);
    Uint8 v_mousestate = SDL_GetMouseState(NULL, NULL);
    unsigned int p, pW;
    Biker *v_biker;

    p = 0; // player number p
    pW = 0; // number of players in the previous worlds
    for(unsigned int j=0; j<i_universe->getScenes().size(); j++) {
      for(unsigned int i=0; i<i_universe->getScenes()[j]->Players().size(); i++) {
	v_biker = i_universe->getScenes()[j]->Players()[i];
	
	if(m_nDriveKey[p].isPressed(v_keystate, v_mousestate)) {
	  /* Start driving */
	  v_biker->getControler()->setThrottle(1.0f);
	} else {
	  v_biker->getControler()->setThrottle(0.0f);
	}
	
	if(m_nBrakeKey[p].isPressed(v_keystate, v_mousestate)) {
	  /* Brake */
	  v_biker->getControler()->setBreak(1.0f);
	} else {
	  v_biker->getControler()->setBreak(0.0f);
	}
	
	if(m_mirrored) {
	  if(m_nPushForwardKey[p].isPressed(v_keystate, v_mousestate)) {
	    v_biker->getControler()->setPull(1.0f);
	  } else {
	    v_biker->getControler()->setPull(0.0f);
	  }
	} else {
	  
	}
	
	// pull
	if((m_nPullBackKey[p].isPressed(v_keystate, v_mousestate) && m_mirrored == false) ||
	   (m_nPushForwardKey[p].isPressed(v_keystate, v_mousestate) && m_mirrored)) {
	  /* Pull back */
	  v_biker->getControler()->setPull(1.0f);
	} else {
	  
	  // push // must be in pull else block to not set pull to 0
	  if((m_nPushForwardKey[p].isPressed(v_keystate, v_mousestate) && m_mirrored == false) ||
	     (m_nPullBackKey[p].isPressed(v_keystate, v_mousestate)    && m_mirrored)) {
	    /* Push forward */
	    v_biker->getControler()->setPull(-1.0f);
	  } else {
	    v_biker->getControler()->setPull(0.0f);
	  }
	}
	
	if(m_nChangeDirKey[p].isPressed(v_keystate, v_mousestate)) {
	  /* Change dir */
	  if(m_changeDirKeyAlreadyPress[p] == false){
	    v_biker->getControler()->setChangeDir(true);
	    m_changeDirKeyAlreadyPress[p] = true;
	  }
	} else {
	  m_changeDirKeyAlreadyPress[p] = false;
	}
	p++;
      }
      pW+= i_universe->getScenes()[j]->Players().size();
    }
  }

  void InputHandler::setMirrored(bool i_value) {
    m_mirrored = i_value;
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

}
  
  /*===========================================================================
  Add script key hook
  ===========================================================================*/  
  void InputHandler::addScriptKeyHook(MotoGame *pGame,const std::string &basicKeyName,const std::string &FuncName) {
    if(m_nNumScriptKeyHooks < MAX_SCRIPT_KEY_HOOKS) {
      m_ScriptKeyHooks[m_nNumScriptKeyHooks].FuncName = FuncName;
      m_ScriptKeyHooks[m_nNumScriptKeyHooks].nKey = XMKey(basicKeyName, true);
      m_ScriptKeyHooks[m_nNumScriptKeyHooks].pGame = pGame;
      m_nNumScriptKeyHooks++;
    }
  }  

std::string* InputHandler::getJoyId(Uint8 i_joynum) {
  return &(m_JoysticksIds[i_joynum]);
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
  Handle an input event
  ===========================================================================*/  
void InputHandler::handleInput(Universe* i_universe, InputEventType Type, const XMKey& i_xmkey) {
    unsigned int p, pW;
    Biker *v_biker;

    switch(Type) {
    case INPUT_DOWN:
      p = 0; // player number p
      pW = 0; // number of players in the previous worlds
      for(unsigned int j=0; j<i_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<i_universe->getScenes()[j]->Players().size(); i++) {
	  v_biker = i_universe->getScenes()[j]->Players()[i];

	  // if else is not valid while axis up can be a signal for two sides
	  if(m_nDriveKey[p] == i_xmkey) {
	    /* Start driving */
	    if(i_xmkey.isAnalogic()) {
	      v_biker->getControler()->setThrottle(fabs(i_xmkey.getAnalogicValue()));
	    } else {
	      v_biker->getControler()->setThrottle(1.0f);
	    }
	  }

	  if(m_nBrakeKey[p] == i_xmkey) {
	    /* Brake */
	    v_biker->getControler()->setBreak(1.0f);
	  }

	  if((m_nPullBackKey[p]    == i_xmkey && m_mirrored == false) ||
	     (m_nPushForwardKey[p] == i_xmkey && m_mirrored)) {
	    /* Pull back */
	    if(i_xmkey.isAnalogic()) {
	      v_biker->getControler()->setPull(fabs(i_xmkey.getAnalogicValue()));
	    } else {
	      v_biker->getControler()->setPull(1.0f);
	    }
	  }
	  
	  if((m_nPushForwardKey[p] == i_xmkey && m_mirrored == false) ||
	     (m_nPullBackKey[p]    == i_xmkey && m_mirrored)) {
	    /* Push forward */
	    if(i_xmkey.isAnalogic()) {
	      v_biker->getControler()->setPull(-fabs(i_xmkey.getAnalogicValue()));
	    } else {
	      v_biker->getControler()->setPull(-1.0f);
	    }
	  }
	  
	  if(m_nChangeDirKey[p] == i_xmkey) {
	    /* Change dir */
	    if(m_changeDirKeyAlreadyPress[p] == false){
	      v_biker->getControler()->setChangeDir(true);
	      m_changeDirKeyAlreadyPress[p] = true;
	    }
	  }
	  p++;
	}
	pW+= i_universe->getScenes()[j]->Players().size();
      }

      break;
    case INPUT_UP:
      p = 0; // player number p
      pW = 0; // number of players in the previous worlds
      for(unsigned int j=0; j<i_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<i_universe->getScenes()[j]->Players().size(); i++) {
	  v_biker = i_universe->getScenes()[j]->Players()[i];

	  // if else is not valid while axis up can be a signal for two sides
	  if(m_nDriveKey[p] == i_xmkey) {
	    /* Stop driving */
	    v_biker->getControler()->setThrottle(0.0f);
	  }

	  if(m_nBrakeKey[p] == i_xmkey) {
	    /* Don't brake */
	    v_biker->getControler()->setBreak(0.0f);
	  }

	  if((m_nPullBackKey[p]    == i_xmkey && m_mirrored == false) ||
	     (m_nPushForwardKey[p] == i_xmkey && m_mirrored)) {
	    /* Pull back */
	    v_biker->getControler()->setPull(0.0f);
	  }

	  if((m_nPushForwardKey[p] == i_xmkey && m_mirrored == false) ||
	     (m_nPullBackKey[p]    == i_xmkey && m_mirrored)) {
	    /* Push forward */
	    v_biker->getControler()->setPull(0.0f);
	  }

	  if(m_nChangeDirKey[p] == i_xmkey) {
	    m_changeDirKeyAlreadyPress[p] = false;
	  }
	  p++;
	}
	pW+= i_universe->getScenes()[j]->Players().size();
      }
      break;
    }

    /* Have the script hooked this key? */
    if(Type == INPUT_DOWN) {
      for(int i=0; i<m_nNumScriptKeyHooks; i++) {
	if(m_ScriptKeyHooks[i].nKey == i_xmkey) {
	  /* Invoke script */
	  m_ScriptKeyHooks[i].pGame->getLuaLibGame()->scriptCallVoid(m_ScriptKeyHooks[i].FuncName);
	}
	for(int j=0; j<INPUT_NB_PLAYERS; j++) {
	  if(m_nScriptActionKeys[j][i] == i_xmkey) {
	    m_ScriptKeyHooks[i].pGame->getLuaLibGame()->scriptCallVoid(m_ScriptKeyHooks[i].FuncName);
	  }
	}	
      }
    }
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

bool InputHandler::isANotSetKey(XMKey* i_xmkey) const {
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
