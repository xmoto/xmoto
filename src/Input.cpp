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

#define INPUT_JOYSTICK_MINIMUM_DETECTION 100
#define INPUT_JOYSTICK_MAXIMUM_VALUE     32760


XMKey::XMKey() {
    m_input = XMK_KEYBOARD;
    m_keyboard_sym = SDLK_a;
    m_keyboard_mod = KMOD_NONE;
}

XMKey::XMKey(Uint8 nButton) {
  m_input              = XMK_MOUSEBUTTON;
  m_mouseButton_button = nButton;
}

XMKey::XMKey(SDL_Event &i_event) {
  switch(i_event.type) {

  case SDL_KEYDOWN:
    m_input = XMK_KEYBOARD;
    m_keyboard_sym = i_event.key.keysym.sym;
    m_keyboard_mod = (SDLMod) (i_event.key.keysym.mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_META)); // only allow these modifiers
    break;

  case SDL_MOUSEBUTTONDOWN:
    m_input = XMK_MOUSEBUTTON;
    m_mouseButton_button = i_event.button.button;
    break;

  case SDL_JOYAXISMOTION:
    m_input        = XMK_JOYSTICKAXIS;
    m_joyId        = InputHandler::instance()->getJoyId(i_event.jaxis.which);
    m_joyAxis      = i_event.jaxis.axis;
    m_joyAxisValue = i_event.jaxis.value;
    break;

  case SDL_JOYBUTTONDOWN:
    m_input     = XMK_JOYSTICKBUTTON;
    m_joyId     = InputHandler::instance()->getJoyId(i_event.jbutton.which);
    m_joyButton = i_event.jbutton.button;
    break;

  default:
    throw Exception("Unknow key");
  }
}

XMKey::XMKey(SDLKey nKey,SDLMod mod) {
  m_input = XMK_KEYBOARD;
  m_keyboard_sym = nKey;
  m_keyboard_mod = (SDLMod) (mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_META)); // only allow these modifiers
}

XMKey::XMKey(std::string* i_joyId, Uint8 i_joyButton) {
  m_input     = XMK_JOYSTICKBUTTON;
  m_joyId     = i_joyId;
  m_joyButton = i_joyButton;
}

XMKey::XMKey(std::string* i_joyId, Uint8 i_joyAxis, Sint16 i_joyAxisValue) {
  m_input        = XMK_JOYSTICKAXIS;
  m_joyId        = i_joyId;
  m_joyAxis      = i_joyAxis;
  m_joyAxisValue = i_joyAxisValue;
}

XMKey::XMKey(const std::string& i_key, bool i_basicMode) {
  unsigned int pos;
  std::string v_current, v_rest;

  if(i_basicMode) {
    m_input = XMK_KEYBOARD;

    if(i_key.length() != 1) {
      throw Exception("Invalid key");
    }
    m_keyboard_sym = (SDLKey)((int)(tolower(i_key[0]))); // give ascii key while sdl does the same
    m_keyboard_mod = KMOD_NONE;
    return;
  }

  v_rest = i_key;
  pos = v_rest.find(":", 0);

  if(pos == std::string::npos) {
    throw Exception("Invalid key");
  }
  v_current = v_rest.substr(0, pos);
  v_rest = v_rest.substr(pos+1, v_rest.length() -pos -1);

  if(v_current == "K") { // keyboard
    m_input = XMK_KEYBOARD;

    pos = v_rest.find(":", 0);
    if(pos == std::string::npos) {
      throw Exception("Invalid key");
    }
    v_current = v_rest.substr(0, pos);
    v_rest = v_rest.substr(pos+1, v_rest.length() -pos -1);

    m_keyboard_sym = (SDLKey) atoi(v_current.c_str());
    m_keyboard_mod = (SDLMod) ( ((SDLMod) atoi(v_rest.c_str())) & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_META) ); // only allow these modifiers

  } else if(v_current == "M") { // mouse button
    m_input = XMK_MOUSEBUTTON;
    m_mouseButton_button = (Uint8) atoi(v_rest.c_str());

  } else if(v_current == "A") { // joystick axis
    m_input     = XMK_JOYSTICKAXIS;

    // get the axis
    pos = v_rest.find(":", 0);
    if(pos == std::string::npos) {
      throw Exception("Invalid key");
    }
    v_current = v_rest.substr(0, pos);
    v_rest = v_rest.substr(pos+1, v_rest.length() -pos -1);
    m_joyAxis = (Uint8) atoi(v_current.c_str());

    // get the axis value
    pos = v_rest.find(":", 0);
    if(pos == std::string::npos) {
      throw Exception("Invalid key");
    }
    v_current = v_rest.substr(0, pos);
    v_rest = v_rest.substr(pos+1, v_rest.length() -pos -1);
    m_joyAxisValue = (Sint16) atoi(v_current.c_str());

    // get the joyid
    m_joyId        = InputHandler::instance()->getJoyIdByStrId(v_rest);
  } else if(v_current == "J") { // joystick button
    m_input     = XMK_JOYSTICKBUTTON;

    pos = v_rest.find(":", 0);
    if(pos == std::string::npos) {
      throw Exception("Invalid key");
    }
    v_current = v_rest.substr(0, pos);
    v_rest = v_rest.substr(pos+1, v_rest.length() -pos -1);

    m_joyId     = InputHandler::instance()->getJoyIdByStrId(v_rest);
    m_joyButton = (Uint8) atoi(v_current.c_str());
  } else {
    throw Exception("Invalid key");
  }
}

bool XMKey::isAnalogic() const {
  return m_input == XMK_JOYSTICKAXIS;
}

float XMKey::getAnalogicValue() const {
  return InputHandler::joyRawToFloat(m_joyAxisValue,
				     -(INPUT_JOYSTICK_MAXIMUM_VALUE), -(INPUT_JOYSTICK_MINIMUM_DETECTION),
				     INPUT_JOYSTICK_MINIMUM_DETECTION, INPUT_JOYSTICK_MAXIMUM_VALUE);
}
  
bool XMKey::operator==(const XMKey& i_other) const {
  if(m_input != i_other.m_input) {
    return false;
  }

  if(m_input == XMK_KEYBOARD) {
    return m_keyboard_sym == i_other.m_keyboard_sym && 
      ((m_keyboard_mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_META)) == 
       (i_other.m_keyboard_mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_META))); // only allow these modifiers
  }

  if(m_input == XMK_MOUSEBUTTON) {
    return m_mouseButton_button == i_other.m_mouseButton_button;
  }

  if(m_input == XMK_JOYSTICKAXIS) {
    // for m_joyId, pointer instead of strings can be compared safely while strings are required only when reading config
    return m_joyId == i_other.m_joyId && m_joyAxis == i_other.m_joyAxis &&
      ! (
	 // axes are not on the same side if the value are on the opposite sides
	 (m_joyAxisValue         > INPUT_JOYSTICK_MINIMUM_DETECTION &&
	  i_other.m_joyAxisValue < INPUT_JOYSTICK_MINIMUM_DETECTION)
	 ||
	 (m_joyAxisValue         < INPUT_JOYSTICK_MINIMUM_DETECTION &&
	  i_other.m_joyAxisValue > INPUT_JOYSTICK_MINIMUM_DETECTION)
	 );
  }

  if(m_input == XMK_JOYSTICKBUTTON) {
    // for m_joyId, pointer instead of strings can be compared safely while strings are required only when reading config
    return m_joyId == i_other.m_joyId && m_joyButton == i_other.m_joyButton;
  }

  return false;
}

std::string XMKey::toString() {
  std::ostringstream v_res;

  switch(m_input) {
  case XMK_KEYBOARD:
    v_res << "K" << ":" << m_keyboard_sym << ":" << m_keyboard_mod;
    break;
  case XMK_MOUSEBUTTON:
    v_res << "M" << ":" << ((int)m_mouseButton_button);
    break;
 case XMK_JOYSTICKAXIS:
   // put the joyid at the end while it can contain any char
   v_res << "A" << ":" << ((int)m_joyAxis) << ":" << ((int)m_joyAxisValue) << ":" << *m_joyId;
   break;

 case XMK_JOYSTICKBUTTON:
   // put the joyid at the end while it can contain any char
   v_res << "J" << ":" << ((int)m_joyButton) << ":" << *m_joyId;
   break;
  }

  return v_res.str();
}

std::string XMKey::toFancyString() {
  std::ostringstream v_res;

  if(m_input == XMK_KEYBOARD) {
    v_res << "[" << GAMETEXT_KEYBOARD << "] " << SDL_GetKeyName(m_keyboard_sym);
  } else if(m_input == XMK_MOUSEBUTTON) {
    v_res << "[" << GAMETEXT_MOUSE << "] " << ((int)m_mouseButton_button);
  } else if(m_input == XMK_JOYSTICKAXIS) {
    v_res << "[" << GAMETEXT_JOYSTICK << "]";
  } else if(m_input == XMK_JOYSTICKBUTTON) {
    v_res << "[" << GAMETEXT_JOYSTICK << "]" << ((int)m_joyButton)+1; // +1 because it starts at 0
  }

  return v_res.str();
}

bool XMKey::isPressed(Uint8 *i_keystate, Uint8 i_mousestate) {
  if(m_input == XMK_KEYBOARD) {
    return i_keystate[m_keyboard_sym] == 1;
  }

  if(m_input == XMK_MOUSEBUTTON) {
    return (i_mousestate & SDL_BUTTON(m_mouseButton_button)) == SDL_BUTTON(m_mouseButton_button);
  }

  if(m_input == XMK_JOYSTICKAXIS) {
    Sint16 v_axisValue = SDL_JoystickGetAxis(InputHandler::instance()->getJoyById(m_joyId), m_joyAxis);
    return (
	    (v_axisValue > INPUT_JOYSTICK_MINIMUM_DETECTION && m_joyAxisValue > INPUT_JOYSTICK_MINIMUM_DETECTION) ||
	    (v_axisValue < -(INPUT_JOYSTICK_MINIMUM_DETECTION) && m_joyAxisValue < -(INPUT_JOYSTICK_MINIMUM_DETECTION))
	    );
  }

  if(m_input == XMK_JOYSTICKBUTTON) {
    return SDL_JoystickGetButton(InputHandler::instance()->getJoyById(m_joyId), m_joyButton) == 1;
  }

  return false;
}

  InputHandler::InputHandler() {
    reset();
  }

  void InputHandler::reset() {
    m_mirrored = false;
    resetScriptKeyHooks();
    for(unsigned int i=0; i<4; i++){
      m_changeDirKeyAlreadyPress[i] = false;
    }
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
void InputHandler::init(UserConfig *pConfig, xmDatabase* pDb, const std::string& i_id_profile) {
  SDL_Joystick* v_joystick;

    /* Initialize joysticks (if any) */
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_JoystickEventState(SDL_ENABLE);
    
    /* Enable unicode translation and key repeats */
    SDL_EnableUNICODE(1);         
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);

    /* Open all joysticks */
    std::string v_joyName, v_joyId;
    int n;
    bool v_continueToOpen = true;

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
	  
	  v_id << n;
	  v_joyId = v_joyName + v_id.str();
	  m_Joysticks.push_back(v_joystick);
	  m_JoysticksNames.push_back(v_joyName);
	  m_JoysticksIds.push_back(v_joyId);

	  Logger::Log("Joystick found [%s], id is %s", v_joyName.c_str(), v_joyId.c_str());
	} else {
	  v_continueToOpen = false; // don't continue to open joystick to keep m_joysticks[joystick.num] working
	  Logger::Log("** Warning ** : fail to open joystick [%s], abort to open other joysticks", v_joyName.c_str());
	}
      }
    }
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
    /* Set defaults */
    setDefaultConfig();
  
    /* Get settings for mode */
    try {
      m_nDriveKey[0]        = XMKey(pDb->config_getString(i_id_profile, "KeyDrive1", ""));
      m_nBrakeKey[0]        = XMKey(pDb->config_getString(i_id_profile, "KeyBrake1", ""));
      m_nPullBackKey[0]     = XMKey(pDb->config_getString(i_id_profile, "KeyFlipLeft1", ""));
      m_nPushForwardKey[0]  = XMKey(pDb->config_getString(i_id_profile, "KeyFlipRight1", ""));
      m_nChangeDirKey[0]    = XMKey(pDb->config_getString(i_id_profile, "KeyChangeDir1", ""));
      m_nDriveKey[1]        = XMKey(pDb->config_getString(i_id_profile, "KeyDrive2"    , ""));
      m_nBrakeKey[1]        = XMKey(pDb->config_getString(i_id_profile, "KeyBrake2"    , ""));
      m_nPullBackKey[1]     = XMKey(pDb->config_getString(i_id_profile, "KeyFlipLeft2" , ""));
      m_nPushForwardKey[1]  = XMKey(pDb->config_getString(i_id_profile, "KeyFlipRight2", ""));
      m_nChangeDirKey[1]    = XMKey(pDb->config_getString(i_id_profile, "KeyChangeDir2", ""));
      m_nDriveKey[2]        = XMKey(pDb->config_getString(i_id_profile, "KeyDrive3"    , ""));
      m_nBrakeKey[2]        = XMKey(pDb->config_getString(i_id_profile, "KeyBrake3"    , ""));
      m_nPullBackKey[2]     = XMKey(pDb->config_getString(i_id_profile, "KeyFlipLeft3" , ""));
      m_nPushForwardKey[2]  = XMKey(pDb->config_getString(i_id_profile, "KeyFlipRight3", ""));
      m_nChangeDirKey[2]    = XMKey(pDb->config_getString(i_id_profile, "KeyChangeDir3", ""));
      m_nDriveKey[3]        = XMKey(pDb->config_getString(i_id_profile, "KeyDrive4"    , ""));
      m_nBrakeKey[3]        = XMKey(pDb->config_getString(i_id_profile, "KeyBrake4"    , ""));
      m_nPullBackKey[3]     = XMKey(pDb->config_getString(i_id_profile, "KeyFlipLeft4" , ""));
      m_nPushForwardKey[3]  = XMKey(pDb->config_getString(i_id_profile, "KeyFlipRight4", ""));
      m_nChangeDirKey[3]    = XMKey(pDb->config_getString(i_id_profile, "KeyChangeDir4", ""));
    } catch(Exception &e) {
      Logger::Log("** Warning ** : Invalid keys configuration!");
      setDefaultConfig();
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

	  if(m_nDriveKey[p] == i_xmkey) {
	    /* Start driving */
	    if(i_xmkey.isAnalogic()) {
	      v_biker->getControler()->setThrottle(fabs(i_xmkey.getAnalogicValue()));
	    } else {
	      v_biker->getControler()->setThrottle(1.0f);
	    }
	  } else if(m_nBrakeKey[p] == i_xmkey) {
	    /* Brake */
	    v_biker->getControler()->setBreak(1.0f);
	  } else if((m_nPullBackKey[p]    == i_xmkey && m_mirrored == false) ||
		    (m_nPushForwardKey[p] == i_xmkey && m_mirrored)) {
	    /* Pull back */
	    if(i_xmkey.isAnalogic()) {
	      v_biker->getControler()->setPull(fabs(i_xmkey.getAnalogicValue()));
	    } else {
	      v_biker->getControler()->setPull(1.0f);
	    }
	  } else if((m_nPushForwardKey[p] == i_xmkey && m_mirrored == false) ||
		    (m_nPullBackKey[p]    == i_xmkey && m_mirrored)) {
	    /* Push forward */
	    if(i_xmkey.isAnalogic()) {
	      v_biker->getControler()->setPull(-fabs(i_xmkey.getAnalogicValue()));
	    } else {
	      v_biker->getControler()->setPull(-1.0f);
	    }
	  }
	  else if(m_nChangeDirKey[p] == i_xmkey) {
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

	  if(m_nDriveKey[p] == i_xmkey) {
	    /* Stop driving */
	    v_biker->getControler()->setThrottle(0.0f);
	  }
	  else if(m_nBrakeKey[p] == i_xmkey) {
	    /* Don't brake */
	    v_biker->getControler()->setBreak(0.0f);
	  }
	  else if((m_nPullBackKey[p]    == i_xmkey && m_mirrored == false) ||
		  (m_nPushForwardKey[p] == i_xmkey && m_mirrored)) {
	    /* Pull back */
	    v_biker->getControler()->setPull(0.0f);
	  }
	  else if((m_nPushForwardKey[p] == i_xmkey && m_mirrored == false) ||
		  (m_nPullBackKey[p]    == i_xmkey && m_mirrored)) {
	    /* Push forward */
	    v_biker->getControler()->setPull(0.0f);
	  }
	  else if(m_nChangeDirKey[p] == i_xmkey) {
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
    
  }  

  /*===========================================================================
  Get key by action...
  ===========================================================================*/  

  std::string InputHandler::getFancyKeyByAction(const std::string &Action) {
    if(Action == "Drive")    	return m_nDriveKey[0].toFancyString();
    if(Action == "Brake")    	return m_nBrakeKey[0].toFancyString();
    if(Action == "PullBack") 	return m_nPullBackKey[0].toFancyString();
    if(Action == "PushForward") return m_nPushForwardKey[0].toFancyString();
    if(Action == "ChangeDir")   return m_nChangeDirKey[0].toFancyString();

    if(Action == "Drive 2")    	  return m_nDriveKey[1].toFancyString();
    if(Action == "Brake 2")    	  return m_nBrakeKey[1].toFancyString();
    if(Action == "PullBack 2") 	  return m_nPullBackKey[1].toFancyString();
    if(Action == "PushForward 2") return m_nPushForwardKey[1].toFancyString();
    if(Action == "ChangeDir 2")   return m_nChangeDirKey[1].toFancyString();

    if(Action == "Drive 3")    	  return m_nDriveKey[2].toFancyString();
    if(Action == "Brake 3")    	  return m_nBrakeKey[2].toFancyString();
    if(Action == "PullBack 3") 	  return m_nPullBackKey[2].toFancyString();
    if(Action == "PushForward 3") return m_nPushForwardKey[2].toFancyString();
    if(Action == "ChangeDir 3")   return m_nChangeDirKey[2].toFancyString();

    if(Action == "Drive 4")    	  return m_nDriveKey[3].toFancyString();
    if(Action == "Brake 4")    	  return m_nBrakeKey[3].toFancyString();
    if(Action == "PullBack 4") 	  return m_nPullBackKey[3].toFancyString();
    if(Action == "PushForward 4") return m_nPushForwardKey[3].toFancyString();
    if(Action == "ChangeDir 4")   return m_nChangeDirKey[3].toFancyString();

    return "?";
  }

void InputHandler::saveConfig(UserConfig *pConfig, xmDatabase* pDb, const std::string& i_id_profile) {
  std::string n;

	pDb->config_setValue_begin();

  for(unsigned int i=0; i<4; i++) {
    switch(i) {
    case 0:
      n = "1";
      break;
    case 1:
      n = "2";
      break;
    case 2:
      n = "3";
      break;
    case 3:
      n = "4";
      break;
    }


    pDb->config_setString(i_id_profile, "KeyDrive"     + n, m_nDriveKey[i].toString()      );
    pDb->config_setString(i_id_profile, "KeyBrake"     + n, m_nBrakeKey[i].toString()      );
    pDb->config_setString(i_id_profile, "KeyFlipLeft"  + n, m_nPullBackKey[i].toString()   );
    pDb->config_setString(i_id_profile, "KeyFlipRight" + n, m_nPushForwardKey[i].toString());
    pDb->config_setString(i_id_profile, "KeyChangeDir" + n, m_nChangeDirKey[i].toString()  );
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
