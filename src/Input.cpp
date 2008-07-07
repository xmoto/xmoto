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
    m_keyboard_mod = i_event.key.keysym.mod;
    break;

  case SDL_MOUSEBUTTONDOWN:
    m_input = XMK_MOUSEBUTTON;
    m_mouseButton_button = i_event.button.button;
    break;

  default:
    throw Exception("Unknow key");
  }
}

XMKey::XMKey(SDLKey nKey,SDLMod mod) {
  m_input = XMK_KEYBOARD;
  m_keyboard_sym = nKey;
  m_keyboard_mod = mod;
}

XMKey::XMKey(const std::string& i_key, bool i_basicMode) {
  int pos;
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
    m_keyboard_mod = (SDLMod) atoi(v_rest.c_str());

  } else if(v_current == "M") { // mouse button
    m_input = XMK_MOUSEBUTTON;
    m_mouseButton_button = (Uint8) atoi(v_rest.c_str());

  } else {
    throw Exception("Invalid key");
  }
}
  
bool XMKey::operator==(const XMKey& i_other) const {
  if(m_input != i_other.m_input) {
    return false;
  }

  if(m_input == XMK_KEYBOARD) {
    return m_keyboard_sym == i_other.m_keyboard_sym && 
      ((m_keyboard_mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_META)) == 
       (i_other.m_keyboard_mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_META)));
  }

  if(m_input == XMK_MOUSEBUTTON) {
    return m_mouseButton_button == i_other.m_mouseButton_button;
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
  }

  return v_res.str();
}

std::string XMKey::toFancyString() {
  std::ostringstream v_res;

  if(m_input == XMK_KEYBOARD) {
    v_res << "[" << GAMETEXT_KEYBOARD << "] " << SDL_GetKeyName(m_keyboard_sym);
  } else if(m_input == XMK_MOUSEBUTTON) {
    v_res << "[" << GAMETEXT_MOUSE << "] " << ((int)m_mouseButton_button);
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
	
	if(m_ControllerModeID[p] == CONTROLLER_MODE_KEYBOARD) {
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
  Globals
  ===========================================================================*/  
  /* Action ID to string table */
	/*
  InputActionType InputHandler::m_ActionTypeTable[] = {
    "Drive",           ACTION_DRIVE,
    "Brake",           ACTION_BRAKE,             
    "Pull",            ACTION_PULL,          
    "Push",            ACTION_PUSH,       
    "ChangeDirection", ACTION_CHANGEDIR,     
    NULL
  };
*/

  /*===========================================================================
  Init/uninit
  ===========================================================================*/  
void InputHandler::init(UserConfig *pConfig, xmDatabase* pDb, const std::string& i_id_profile) {
    /* Initialize joysticks (if any) */
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    
    /* Enable unicode translation and key repeats */
    SDL_EnableUNICODE(1);         
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);

    /* Open all joysticks */
    for(int i=0; i<SDL_NumJoysticks(); i++) {
      m_Joysticks.push_back(SDL_JoystickOpen(i));
    }

    m_pActiveJoystick1 = NULL;
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

  namespace {
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
    float joyRawToFloat(float raw, float neg, float deadzone_neg, float deadzone_pos, float pos) {

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
  }

  /*===========================================================================
  Input device update
  ===========================================================================*/  
  void InputHandler::updateUniverseInput(Universe* i_universe) {
    Biker* v_biker;

    if(i_universe == NULL) {
      return;
    }

    if(i_universe->getScenes().size() <= 0) {
      return;
    }
    
    if(i_universe->getScenes()[0]->Players().size() <= 0) {
      return;
    }

    v_biker = i_universe->getScenes()[0]->Players()[0]; // only for the first player for the moment

    /* Joystick? */
    /* joystick only for player 1 */
     if(m_ControllerModeID[0] == CONTROLLER_MODE_JOYSTICK1 && m_pActiveJoystick1 != NULL) {
      SDL_JoystickUpdate();     

      v_biker->getControler()->stopContols();
      
      /* Update buttons */
      for(int i=0; i<SDL_JoystickNumButtons(m_pActiveJoystick1); i++) {
        if(SDL_JoystickGetButton(m_pActiveJoystick1,i)) {
          if(!m_JoyButtonsPrev[i]) {
            /* Click! */
            if(m_nJoyButtonChangeDir1 == i) {
              v_biker->getControler()->setChangeDir(true);
            }
          }

          m_JoyButtonsPrev[i] = true;
        }        
        else
          m_JoyButtonsPrev[i] = false;
      }
      
      /** Update axis */           
      int nRawPrim = SDL_JoystickGetAxis(m_pActiveJoystick1,m_nJoyAxisPrim1);
      v_biker->getControler()->setDrive(-joyRawToFloat(nRawPrim, m_nJoyAxisPrimMin1, m_nJoyAxisPrimLL1, m_nJoyAxisPrimUL1, m_nJoyAxisPrimMax1));
      int nRawSec = SDL_JoystickGetAxis(m_pActiveJoystick1,m_nJoyAxisSec1);
      v_biker->getControler()->setPull(-joyRawToFloat(nRawSec, m_nJoyAxisSecMin1, m_nJoyAxisSecLL1, m_nJoyAxisSecUL1, m_nJoyAxisSecMax1));
    }
  }
  
  /*===========================================================================
  Read configuration
  ===========================================================================*/  
  std::string InputHandler::waitForKey(void) {
    SDL_Event Event;

    /* Start waiting for a key */      
    SDL_PumpEvents();
    while(SDL_PollEvent(&Event)) {
      if(Event.type == SDL_QUIT ||
	 (Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_ESCAPE) ) {
	return "<<QUIT>>";
      }

      try {
	  return XMKey(Event).toString();
      } catch(Exception &e) {
	/* invalid key */
      }
    }

    return "";
  }
  
  /*===========================================================================
  Read configuration
  ===========================================================================*/  
void InputHandler::loadConfig(UserConfig *pConfig, xmDatabase* pDb, const std::string& i_id_profile) {
    /* Set defaults */
    setDefaultConfig();
  
    /* Get controller mode? (Keyboard or joystick) */
    std::string ControllerMode1 = pConfig->getString("ControllerMode1");    
    if(ControllerMode1 != "Keyboard" && ControllerMode1 != "Joystick1") {
      ControllerMode1 = "Keyboard"; /* go default then */
      Logger::Log("** Warning ** : 'ControllerMode1' must be either 'Keyboard' or 'Joystick1'!");
    }

    /* Get settings for mode */
    if(ControllerMode1 == "Keyboard") {
      /* We're using the keyboard */
      m_ControllerModeID[0] = CONTROLLER_MODE_KEYBOARD;

      try {
	m_nDriveKey[0]        = XMKey(pDb->config_getString(i_id_profile, "KeyDrive1", ""));
	m_nBrakeKey[0]        = XMKey(pDb->config_getString(i_id_profile, "KeyBrake1", ""));
	m_nPullBackKey[0]     = XMKey(pDb->config_getString(i_id_profile, "KeyFlipLeft1", ""));
	m_nPushForwardKey[0]  = XMKey(pDb->config_getString(i_id_profile, "KeyFlipRight1", ""));
	m_nChangeDirKey[0]    = XMKey(pDb->config_getString(i_id_profile, "KeyChangeDir1", ""));
      } catch(Exception &e) {
	Logger::Log("** Warning ** : Invalid keyboard configuration!");
	setDefaultConfig();
      }

    } else {
      /* We're using joystick 1 */
      m_ControllerModeID[0] = CONTROLLER_MODE_JOYSTICK1;      
      
      int nIdx = pConfig->getInteger("JoyIdx1");

      if(nIdx >= 0) {
	m_pActiveJoystick1 = m_Joysticks[nIdx];
	
	/* Okay, fetch the rest of the config */          
	m_nJoyAxisPrim1 = pConfig->getInteger("JoyAxisPrim1");
	m_nJoyAxisPrimMax1 = pConfig->getInteger("JoyAxisPrimMax1");
	m_nJoyAxisPrimMin1 = pConfig->getInteger("JoyAxisPrimMin1");
	m_nJoyAxisPrimUL1 = pConfig->getInteger("JoyAxisPrimUL1");
	m_nJoyAxisPrimLL1 = pConfig->getInteger("JoyAxisPrimLL1");
	
	m_nJoyAxisSec1 = pConfig->getInteger("JoyAxisSec1");
	m_nJoyAxisSecMax1 = pConfig->getInteger("JoyAxisSecMax1");
	m_nJoyAxisSecMin1 = pConfig->getInteger("JoyAxisSecMin1");
	m_nJoyAxisSecUL1 = pConfig->getInteger("JoyAxisSecUL1");
	m_nJoyAxisSecLL1 = pConfig->getInteger("JoyAxisSecLL1");
	
	m_nJoyButtonChangeDir1 = pConfig->getInteger("JoyButtonChangeDir1");
	
	/* Init all joystick buttons */
	m_JoyButtonsPrev.clear();
	for(int i=0; i<SDL_JoystickNumButtons(m_pActiveJoystick1); i++) {
	  m_JoyButtonsPrev.push_back(false);
	}

      }
    }

    m_ControllerModeID[1] = CONTROLLER_MODE_KEYBOARD;
    m_ControllerModeID[2] = CONTROLLER_MODE_KEYBOARD;
    m_ControllerModeID[3] = CONTROLLER_MODE_KEYBOARD;

    try {
      m_nDriveKey[1]       = XMKey(pDb->config_getString(i_id_profile, "KeyDrive2"    , ""));
      m_nBrakeKey[1]       = XMKey(pDb->config_getString(i_id_profile, "KeyBrake2"    , ""));
      m_nPullBackKey[1]    = XMKey(pDb->config_getString(i_id_profile, "KeyFlipLeft2" , ""));
      m_nPushForwardKey[1] = XMKey(pDb->config_getString(i_id_profile, "KeyFlipRight2", ""));
      m_nChangeDirKey[1]   = XMKey(pDb->config_getString(i_id_profile, "KeyChangeDir2", ""));
      
      m_nDriveKey[2]       = XMKey(pDb->config_getString(i_id_profile, "KeyDrive3"    , ""));
      m_nBrakeKey[2]       = XMKey(pDb->config_getString(i_id_profile, "KeyBrake3"    , ""));
      m_nPullBackKey[2]    = XMKey(pDb->config_getString(i_id_profile, "KeyFlipLeft3" , ""));
      m_nPushForwardKey[2] = XMKey(pDb->config_getString(i_id_profile, "KeyFlipRight3", ""));
      m_nChangeDirKey[2]   = XMKey(pDb->config_getString(i_id_profile, "KeyChangeDir3", ""));
      
      m_nDriveKey[3]       = XMKey(pDb->config_getString(i_id_profile, "KeyDrive4"    , ""));
      m_nBrakeKey[3]       = XMKey(pDb->config_getString(i_id_profile, "KeyBrake4"    , ""));
      m_nPullBackKey[3]    = XMKey(pDb->config_getString(i_id_profile, "KeyFlipLeft4" , ""));
      m_nPushForwardKey[3] = XMKey(pDb->config_getString(i_id_profile, "KeyFlipRight4", ""));
      m_nChangeDirKey[3]   = XMKey(pDb->config_getString(i_id_profile, "KeyChangeDir4", ""));
    } catch(Exception &e) {    
      Logger::Log("** Warning ** : Invalid keyboard configuration!");
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

  /*===========================================================================
  Handle an input event
  ===========================================================================*/  
void InputHandler::handleInput(Universe* i_universe, InputEventType Type, int nKey,SDLMod mod) {
    unsigned int p, pW;
    Biker *v_biker;
    XMKey v_key;

    if(Type == INPUT_KEY_DOWN || Type == INPUT_KEY_UP) {
      v_key = XMKey((SDLKey)nKey, mod);
    } else if(Type == INPUT_MOUSE_DOWN || Type == INPUT_MOUSE_UP) {
      v_key = XMKey((Uint8)nKey);
    } else {
      return;
    }

    /* Update controller 1 */
    /* Keyboard controlled */
    switch(Type) {
    case INPUT_KEY_DOWN:
    case INPUT_MOUSE_DOWN:
      p = 0; // player number p
      pW = 0; // number of players in the previous worlds
      for(unsigned int j=0; j<i_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<i_universe->getScenes()[j]->Players().size(); i++) {
	  v_biker = i_universe->getScenes()[j]->Players()[i];

	  if(m_ControllerModeID[p] == CONTROLLER_MODE_KEYBOARD) {
	    if(m_nDriveKey[p] == v_key) {
	      /* Start driving */
	      v_biker->getControler()->setThrottle(1.0f);
	    } else if(m_nBrakeKey[p] == v_key) {
	      /* Brake */
	      v_biker->getControler()->setBreak(1.0f);
	    } else if((m_nPullBackKey[p]    == v_key && m_mirrored == false) ||
		      (m_nPushForwardKey[p] == v_key && m_mirrored)) {
	      /* Pull back */
	      v_biker->getControler()->setPull(1.0f);
	    } else if((m_nPushForwardKey[p] == v_key && m_mirrored == false) ||
		      (m_nPullBackKey[p]    == v_key && m_mirrored)) {
	      /* Push forward */
	      v_biker->getControler()->setPull(-1.0f);            
	    }
	    else if(m_nChangeDirKey[p] == v_key) {
	      /* Change dir */
	      if(m_changeDirKeyAlreadyPress[p] == false){
		v_biker->getControler()->setChangeDir(true);
		m_changeDirKeyAlreadyPress[p] = true;
	      }
	    }
	  }
	  p++;
	}
	pW+= i_universe->getScenes()[j]->Players().size();
      }

      break;
    case INPUT_KEY_UP:
    case INPUT_MOUSE_UP:
      p = 0; // player number p
      pW = 0; // number of players in the previous worlds
      for(unsigned int j=0; j<i_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<i_universe->getScenes()[j]->Players().size(); i++) {
	  v_biker = i_universe->getScenes()[j]->Players()[i];

	  if(m_ControllerModeID[p] == CONTROLLER_MODE_KEYBOARD) {
	    if(m_nDriveKey[p] == v_key) {
	      /* Stop driving */
	      v_biker->getControler()->setThrottle(0.0f);
	    }
	    else if(m_nBrakeKey[p] == v_key) {
	      /* Don't brake */
	      v_biker->getControler()->setBreak(0.0f);
	    }
	    else if((m_nPullBackKey[p]    == v_key && m_mirrored == false) ||
		    (m_nPushForwardKey[p] == v_key && m_mirrored)) {
	      /* Pull back */
	      v_biker->getControler()->setPull(0.0f);
	    }
	    else if((m_nPushForwardKey[p] == v_key && m_mirrored == false) ||
		    (m_nPullBackKey[p]    == v_key && m_mirrored)) {
	      /* Push forward */
	      v_biker->getControler()->setPull(0.0f);            
	    }
	    else if(m_nChangeDirKey[p] == v_key) {
	      m_changeDirKeyAlreadyPress[p] = false;
	    }
	  }
	  p++;
	}
	pW+= i_universe->getScenes()[j]->Players().size();
      }
      break;
    }

    /* Have the script hooked this key? */
    if(Type == INPUT_KEY_DOWN) {
      for(int i=0; i<m_nNumScriptKeyHooks; i++) {
	if(m_ScriptKeyHooks[i].nKey == v_key) {
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
    m_ControllerModeID[0] = CONTROLLER_MODE_KEYBOARD;
    m_ControllerModeID[1] = CONTROLLER_MODE_KEYBOARD;
    m_ControllerModeID[2] = CONTROLLER_MODE_KEYBOARD;
    m_ControllerModeID[3] = CONTROLLER_MODE_KEYBOARD;
    
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
