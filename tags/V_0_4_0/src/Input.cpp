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
#include "LuaLibGame.h"
#include "helpers/Log.h"
#include "xmscene/Camera.h"
#include "xmscene/BikeController.h"
#include "Universe.h"

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
    Uint8 *v_keystate = SDL_GetKeyState(NULL);
    unsigned int p, pW;
    Biker *v_biker;

    p = 0; // player number p
    pW = 0; // number of players in the previous worlds
    for(unsigned int j=0; j<i_universe->getScenes().size(); j++) {
      for(unsigned int i=0; i<i_universe->getScenes()[j]->Players().size(); i++) {
	v_biker = i_universe->getScenes()[j]->Players()[i];
	
	if(m_ControllerModeID[p] == CONTROLLER_MODE_KEYBOARD) {
	  if(v_keystate[m_nDriveKey[p]] == 1) {
	    /* Start driving */
	    v_biker->getControler()->setThrottle(1.0f);
	  } else if(v_keystate[m_nBrakeKey[p]] == 1) {
	    /* Brake */
	    v_biker->getControler()->setBreak(1.0f);
	  } else if((v_keystate[m_nPullBackKey[p]]    == 1 && m_mirrored == false) ||
		    (v_keystate[m_nPushForwardKey[p]] == 1 && m_mirrored)) {
	      /* Pull back */
	    v_biker->getControler()->setPull(1.0f);
	  } else if((v_keystate[m_nPushForwardKey[p]] == 1 && m_mirrored == false) ||
		    (v_keystate[m_nPullBackKey[p]]    == 1 && m_mirrored)) {
	    /* Push forward */
	    v_biker->getControler()->setPull(-1.0f);            
	  }
	  else if(v_keystate[m_nChangeDirKey[p]] == 1) {
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
  /* Key code to string table */  
  InputKeyMap InputHandler::m_KeyMap[] = {
    {"Up",              SDLK_UP},
    {"Down",            SDLK_DOWN},
    {"Left",            SDLK_LEFT},
    {"Right",           SDLK_RIGHT},
    {"Space",           SDLK_SPACE},
    {"A",               SDLK_a},
    {"B",               SDLK_b},
    {"C",               SDLK_c},
    {"D",               SDLK_d},
    {"E",               SDLK_e},
    {"F",               SDLK_f},
    {"G",               SDLK_g},
    {"H",               SDLK_h},
    {"I",               SDLK_i},
    {"J",               SDLK_j},
    {"K",               SDLK_k},
    {"L",               SDLK_l},
    {"M",               SDLK_m},
    {"N",               SDLK_n},
    {"O",               SDLK_o},
    {"P",               SDLK_p},
    {"Q",               SDLK_q},
    {"R",               SDLK_r},
    {"S",               SDLK_s},
    {"T",               SDLK_t},
    {"U",               SDLK_u},
    {"V",               SDLK_v},
    {"W",               SDLK_w},
    {"X",               SDLK_x},
    {"Y",               SDLK_y},
    {"Z",               SDLK_z},
    {"1",               SDLK_1},
    {"2",               SDLK_2},
    {"3",               SDLK_3},
    {"4",               SDLK_4},
    {"5",               SDLK_5},
    {"6",               SDLK_6},
    {"7",               SDLK_7},
    {"8",               SDLK_8},
    {"9",               SDLK_9},
    {"0",               SDLK_0},
    {"PageUp",          SDLK_PAGEUP},
    {"PageDown",        SDLK_PAGEDOWN},
    {"Home",            SDLK_HOME},
    {"End",             SDLK_END},
    {"Return",          SDLK_RETURN},
    {"Left Click",      SDL_BUTTON_LEFT},
    {"Right Click",     SDL_BUTTON_RIGHT},
    {"Middle Click",    SDL_BUTTON_MIDDLE},
    {"Wheel down",      SDL_BUTTON_WHEELDOWN},
    {"Wheel up",        SDL_BUTTON_WHEELUP},
    {"Pad 0", SDLK_KP0},
    {"Pad 1", SDLK_KP1},
    {"Pad 2", SDLK_KP2},
    {"Pad 3", SDLK_KP3},
    {"Pad 4", SDLK_KP4},
    {"Pad 5", SDLK_KP5},
    {"Pad 6", SDLK_KP6},
    {"Pad 7", SDLK_KP7},
    {"Pad 8", SDLK_KP8},
    {"Pad 9", SDLK_KP9},

    /* TODO: add more */
    {NULL, 0}
  };
  
  /*===========================================================================
  Easy translation between keys and their codes
  ===========================================================================*/  
  std::string InputHandler::keyToString(int nKey) {
    int i=0;
    while(m_KeyMap[i].pcKey != NULL) {
      if(m_KeyMap[i].nKey == nKey) return m_KeyMap[i].pcKey;
      i++;
    }
    
    return ""; /* unknown! */
  }
  
  int InputHandler::stringToKey(const std::string &s) {
    int i=0;
    while(m_KeyMap[i].pcKey != NULL) {
      if(s == m_KeyMap[i].pcKey) return m_KeyMap[i].nKey;
      i++;
    }
    
    return -1;
  }

  /*===========================================================================
  Init/uninit
  ===========================================================================*/  
  void InputHandler::init(UserConfig *pConfig) {
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
    loadConfig(pConfig);
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
    /* Start waiting for a key */
    std::string Ret = "";
    
    bool bWait = true;
    while(bWait) {
      /* Crunch SDL events */
      SDL_Event Event;
      
      SDL_PumpEvents();
      while(SDL_PollEvent(&Event)) {
        switch(Event.type) {
          case SDL_QUIT:
            return "<<QUIT>>";
	case SDL_MOUSEBUTTONDOWN:
	  Ret = keyToString(Event.button.button);
	  if(Ret != "") bWait = false;
	  break;
	case SDL_KEYDOWN:
            if(Event.key.keysym.sym == SDLK_ESCAPE) return "<<CANCEL>>";
          
            Ret = keyToString(Event.key.keysym.sym);
            if(Ret != "") bWait = false;
            break;
        }
      }
    }
    
    return Ret;
  }
  
  /*===========================================================================
  Read configuration
  ===========================================================================*/  
  void InputHandler::loadConfig(UserConfig *pConfig) {
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
      m_nDriveKey[0] = stringToKey(pConfig->getString("KeyDrive1"));
      m_nBrakeKey[0] = stringToKey(pConfig->getString("KeyBrake1"));
      m_nPullBackKey[0] = stringToKey(pConfig->getString("KeyFlipLeft1"));
      m_nPushForwardKey[0] = stringToKey(pConfig->getString("KeyFlipRight1"));
      m_nChangeDirKey[0] = stringToKey(pConfig->getString("KeyChangeDir1"));
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

    m_nDriveKey[1]    	 = stringToKey(pConfig->getString("KeyDrive2"));
    m_nBrakeKey[1]    	 = stringToKey(pConfig->getString("KeyBrake2"));
    m_nPullBackKey[1] 	 = stringToKey(pConfig->getString("KeyFlipLeft2"));
    m_nPushForwardKey[1] = stringToKey(pConfig->getString("KeyFlipRight2"));
    m_nChangeDirKey[1]   = stringToKey(pConfig->getString("KeyChangeDir2"));

    m_nDriveKey[2]    	 = stringToKey(pConfig->getString("KeyDrive3"));
    m_nBrakeKey[2]    	 = stringToKey(pConfig->getString("KeyBrake3"));
    m_nPullBackKey[2] 	 = stringToKey(pConfig->getString("KeyFlipLeft3"));
    m_nPushForwardKey[2] = stringToKey(pConfig->getString("KeyFlipRight3"));
    m_nChangeDirKey[2]   = stringToKey(pConfig->getString("KeyChangeDir3"));

    m_nDriveKey[3]    	 = stringToKey(pConfig->getString("KeyDrive4"));
    m_nBrakeKey[3]    	 = stringToKey(pConfig->getString("KeyBrake4"));
    m_nPullBackKey[3] 	 = stringToKey(pConfig->getString("KeyFlipLeft4"));
    m_nPushForwardKey[3] = stringToKey(pConfig->getString("KeyFlipRight4"));
    m_nChangeDirKey[3]   = stringToKey(pConfig->getString("KeyChangeDir4"));
    
    /* All good? */
    bool isUserKeyOk = true;
    for(unsigned int i=0; i<4; i++) {
      if(m_nDriveKey[i]<0 || m_nBrakeKey[i]<0 || m_nPullBackKey[i]<0 ||
	 m_nPushForwardKey[i]<0 || m_nChangeDirKey[i]<0) {
	isUserKeyOk = false;
      }
    }

    if(isUserKeyOk == false) {
      Logger::Log("** Warning ** : Invalid keyboard configuration!");
      _SetDefaultConfigToUnsetKeys();
    }    
  }
  
  /*===========================================================================
  Add script key hook
  ===========================================================================*/  
  void InputHandler::addScriptKeyHook(MotoGame *pGame,const std::string &KeyName,const std::string &FuncName) {
    if(m_nNumScriptKeyHooks < MAX_SCRIPT_KEY_HOOKS) {
      m_ScriptKeyHooks[m_nNumScriptKeyHooks].FuncName = FuncName;
      m_ScriptKeyHooks[m_nNumScriptKeyHooks].nKey = stringToKey(KeyName);
      m_ScriptKeyHooks[m_nNumScriptKeyHooks].pGame = pGame;
      m_nNumScriptKeyHooks++;
    }
  }  

  /*===========================================================================
  Handle an input event
  ===========================================================================*/  
void InputHandler::handleInput(Universe* i_universe, InputEventType Type,int nKey,SDLMod mod) {
    unsigned int p, pW;
    Biker *v_biker;

    /* Update controller 1 */
    /* Keyboard controlled */
    switch(Type) {
    case INPUT_KEY_DOWN:
      p = 0; // player number p
      pW = 0; // number of players in the previous worlds
      for(unsigned int j=0; j<i_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<i_universe->getScenes()[j]->Players().size(); i++) {
	  v_biker = i_universe->getScenes()[j]->Players()[i];

	  if(m_ControllerModeID[p] == CONTROLLER_MODE_KEYBOARD) {
	    if(m_nDriveKey[p] == nKey) {
	      /* Start driving */
	      v_biker->getControler()->setThrottle(1.0f);
	    } else if(m_nBrakeKey[p] == nKey) {
	      /* Brake */
	      v_biker->getControler()->setBreak(1.0f);
	    } else if((m_nPullBackKey[p]    == nKey && m_mirrored == false) ||
		      (m_nPushForwardKey[p] == nKey && m_mirrored)) {
	      /* Pull back */
	      v_biker->getControler()->setPull(1.0f);
	    } else if((m_nPushForwardKey[p] == nKey && m_mirrored == false) ||
		      (m_nPullBackKey[p]    == nKey && m_mirrored)) {
	      /* Push forward */
	      v_biker->getControler()->setPull(-1.0f);            
	    }
	    else if(m_nChangeDirKey[p] == nKey) {
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
      p = 0; // player number p
      pW = 0; // number of players in the previous worlds
      for(unsigned int j=0; j<i_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<i_universe->getScenes()[j]->Players().size(); i++) {
	  v_biker = i_universe->getScenes()[j]->Players()[i];

	  if(m_ControllerModeID[p] == CONTROLLER_MODE_KEYBOARD) {
	    if(m_nDriveKey[p] == nKey) {
	      /* Stop driving */
	      v_biker->getControler()->setThrottle(0.0f);
	    }
	    else if(m_nBrakeKey[p] == nKey) {
	      /* Don't brake */
	      v_biker->getControler()->setBreak(0.0f);
	    }
	    else if((m_nPullBackKey[p]    == nKey && m_mirrored == false) ||
		    (m_nPushForwardKey[p] == nKey && m_mirrored)) {
	      /* Pull back */
	      v_biker->getControler()->setPull(0.0f);
	    }
	    else if((m_nPushForwardKey[p] == nKey && m_mirrored == false) ||
		    (m_nPullBackKey[p]    == nKey && m_mirrored)) {
	      /* Push forward */
	      v_biker->getControler()->setPull(0.0f);            
	    }
	    else if(m_nChangeDirKey[p] == nKey) {
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
	if(m_ScriptKeyHooks[i].nKey == nKey) {
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
    
    m_nDriveKey[0]       = SDLK_UP;
    m_nBrakeKey[0]       = SDLK_DOWN;
    m_nPullBackKey[0]    = SDLK_LEFT;
    m_nPushForwardKey[0] = SDLK_RIGHT;
    m_nChangeDirKey[0]   = SDLK_SPACE;

    m_nDriveKey[1]       = SDLK_a;
    m_nBrakeKey[1]       = SDLK_q;
    m_nPullBackKey[1]    = SDLK_z;
    m_nPushForwardKey[1] = SDLK_e;
    m_nChangeDirKey[1]   = SDLK_w;

    m_nDriveKey[2]       = SDLK_r;
    m_nBrakeKey[2]       = SDLK_f;
    m_nPullBackKey[2]    = SDLK_t;
    m_nPushForwardKey[2] = SDLK_y;
    m_nChangeDirKey[2]   = SDLK_v;

    m_nDriveKey[3]       = SDLK_y;
    m_nBrakeKey[3]       = SDLK_h;
    m_nPullBackKey[3]    = SDLK_u;
    m_nPushForwardKey[3] = SDLK_i;
    m_nChangeDirKey[3]   = SDLK_n;
    
  }  

  void InputHandler::_SetDefaultConfigToUnsetKeys() {
    m_ControllerModeID[0] = CONTROLLER_MODE_KEYBOARD;
    m_ControllerModeID[1] = CONTROLLER_MODE_KEYBOARD;
    m_ControllerModeID[2] = CONTROLLER_MODE_KEYBOARD;
    m_ControllerModeID[3] = CONTROLLER_MODE_KEYBOARD;
    
    if(m_nDriveKey[0]       < 0) { m_nDriveKey[0] 	    = SDLK_UP;       }
    if(m_nBrakeKey[0]       < 0) { m_nBrakeKey[0] 	    = SDLK_DOWN;     }
    if(m_nPullBackKey[0]    < 0) { m_nPullBackKey[0]    = SDLK_LEFT;     }
    if(m_nPushForwardKey[0] < 0) { m_nPushForwardKey[0] = SDLK_RIGHT;    }
    if(m_nChangeDirKey[0]   < 0) { m_nChangeDirKey[0]   = SDLK_SPACE;    }
 
    if(m_nDriveKey[1]       < 0) { m_nDriveKey[1] 	    = SDLK_a;       }
    if(m_nBrakeKey[1]       < 0) { m_nBrakeKey[1] 	    = SDLK_q;     }
    if(m_nPullBackKey[1]    < 0) { m_nPullBackKey[1]    = SDLK_z;     }
    if(m_nPushForwardKey[1] < 0) { m_nPushForwardKey[1] = SDLK_e;    }
    if(m_nChangeDirKey[1]   < 0) { m_nChangeDirKey[1]   = SDLK_w;    }

    if(m_nDriveKey[2]       < 0) { m_nDriveKey[2] 	    = SDLK_r;       }
    if(m_nBrakeKey[2]       < 0) { m_nBrakeKey[2] 	    = SDLK_f;     }
    if(m_nPullBackKey[2]    < 0) { m_nPullBackKey[2]    = SDLK_t;     }
    if(m_nPushForwardKey[2] < 0) { m_nPushForwardKey[2] = SDLK_y;    }
    if(m_nChangeDirKey[2]   < 0) { m_nChangeDirKey[2]   = SDLK_v;    }

    if(m_nDriveKey[3]       < 0) { m_nDriveKey[3] 	    = SDLK_y;       }
    if(m_nBrakeKey[3]       < 0) { m_nBrakeKey[3] 	    = SDLK_h;     }
    if(m_nPullBackKey[3]    < 0) { m_nPullBackKey[3]    = SDLK_h;     }
    if(m_nPushForwardKey[3] < 0) { m_nPushForwardKey[3] = SDLK_i;    }
    if(m_nChangeDirKey[3]   < 0) { m_nChangeDirKey[3]   = SDLK_n;    }
  }

  /*===========================================================================
  Get key by action...
  ===========================================================================*/  
  std::string InputHandler::getKeyByAction(const std::string &Action) {
    if(Action == "Drive")    	return keyToString(m_nDriveKey[0]);
    if(Action == "Brake")    	return keyToString(m_nBrakeKey[0]);
    if(Action == "PullBack") 	return keyToString(m_nPullBackKey[0]);
    if(Action == "PushForward") return keyToString(m_nPushForwardKey[0]);
    if(Action == "ChangeDir")   return keyToString(m_nChangeDirKey[0]);

    if(Action == "Drive 2")    	  return keyToString(m_nDriveKey[1]);
    if(Action == "Brake 2")    	  return keyToString(m_nBrakeKey[1]);
    if(Action == "PullBack 2") 	  return keyToString(m_nPullBackKey[1]);
    if(Action == "PushForward 2") return keyToString(m_nPushForwardKey[1]);
    if(Action == "ChangeDir 2")   return keyToString(m_nChangeDirKey[1]);

    if(Action == "Drive 3")    	  return keyToString(m_nDriveKey[2]);
    if(Action == "Brake 3")    	  return keyToString(m_nBrakeKey[2]);
    if(Action == "PullBack 3") 	  return keyToString(m_nPullBackKey[2]);
    if(Action == "PushForward 3") return keyToString(m_nPushForwardKey[2]);
    if(Action == "ChangeDir 3")   return keyToString(m_nChangeDirKey[2]);

    if(Action == "Drive 4")    	  return keyToString(m_nDriveKey[3]);
    if(Action == "Brake 4")    	  return keyToString(m_nBrakeKey[3]);
    if(Action == "PullBack 4") 	  return keyToString(m_nPullBackKey[3]);
    if(Action == "PushForward 4") return keyToString(m_nPushForwardKey[3]);
    if(Action == "ChangeDir 4")   return keyToString(m_nChangeDirKey[3]);

    return "?";
  }

void InputHandler::saveConfig(UserConfig *pConfig) {
  std::string n;

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

    pConfig->setString("KeyDrive"     + n, keyToString(m_nDriveKey[i]));
    pConfig->setString("KeyBrake"     + n, keyToString(m_nBrakeKey[i]));
    pConfig->setString("KeyFlipLeft"  + n, keyToString(m_nPullBackKey[i]));
    pConfig->setString("KeyFlipRight" + n, keyToString(m_nPushForwardKey[i]));
    pConfig->setString("KeyChangeDir" + n, keyToString(m_nChangeDirKey[i]));
  }
}

void InputHandler::setDRIVE(int i_player, int i_value) {
  m_nDriveKey[i_player] = i_value;
}

int InputHandler::getDRIVE(int i_player) const {
  return m_nDriveKey[i_player];
}

void InputHandler::setBRAKE(int i_player, int i_value) {
  m_nBrakeKey[i_player] = i_value;
}

int InputHandler::getBRAKE(int i_player) const {
  return m_nBrakeKey[i_player];
}

void InputHandler::setFLIPLEFT(int i_player, int i_value) {
  m_nPullBackKey[i_player] = i_value;
}

int InputHandler::getFLIPLEFT(int i_player) const {
  return m_nPullBackKey[i_player];
}

void InputHandler::setFLIPRIGHT(int i_player, int i_value) {
  m_nPushForwardKey[i_player] = i_value;
}

int InputHandler::getFLIPRIGHT(int i_player) const {
  return m_nPushForwardKey[i_player];
}

void InputHandler::setCHANGEDIR(int i_player, int i_value) {
  m_nChangeDirKey[i_player] = i_value;
}

int InputHandler::getCHANGEDIR(int i_player) const {
  return m_nChangeDirKey[i_player];
}
