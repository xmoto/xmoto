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

  InputHandler::InputHandler() {
    reset();
  }

  void InputHandler::reset() {
    m_mirrored = false;
    resetScriptKeyHooks();
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
    #if defined(ENABLE_ZOOMING)    
      "ZoomIn",          ACTION_ZOOMIN,     
      "ZoomOut",         ACTION_ZOOMOUT,
      "ZoomInit",        ACTION_ZOOMINIT,
      "CameraMoveXUp",   ACTION_CAMERAMOVEXUP,
      "CameraMoveXDown", ACTION_CAMERAMOVEXDOWN,
      "CameraMoveYUp",   ACTION_CAMERAMOVEYUP,
      "CameraMoveYDown", ACTION_CAMERAMOVEYDOWN,
    #endif
    NULL
  };
*/
  /* Key code to string table */  
  InputKeyMap InputHandler::m_KeyMap[] = {
    "Up",              SDLK_UP,
    "Down",            SDLK_DOWN,
    "Left",            SDLK_LEFT,
    "Right",           SDLK_RIGHT,
    "Space",           SDLK_SPACE,
    "A",               SDLK_a,
    "B",               SDLK_b,
    "C",               SDLK_c,
    "D",               SDLK_d,
    "E",               SDLK_e,
    "F",               SDLK_f,
    "G",               SDLK_g,
    "H",               SDLK_h,
    "I",               SDLK_i,
    "J",               SDLK_j,
    "K",               SDLK_k,
    "L",               SDLK_l,
    "M",               SDLK_m,
    "N",               SDLK_n,
    "O",               SDLK_o,
    "P",               SDLK_p,
    "Q",               SDLK_q,
    "R",               SDLK_r,
    "S",               SDLK_s,
    "T",               SDLK_t,
    "U",               SDLK_u,
    "V",               SDLK_v,
    "W",               SDLK_w,
    "X",               SDLK_x,
    "Y",               SDLK_y,
    "Z",               SDLK_z,
    "1",               SDLK_1,
    "2",               SDLK_2,
    "3",               SDLK_3,
    "4",               SDLK_4,
    "5",               SDLK_5,
    "6",               SDLK_6,
    "7",               SDLK_7,
    "8",               SDLK_8,
    "9",               SDLK_9,
    "0",               SDLK_0,
    "PageUp",          SDLK_PAGEUP,
    "PageDown",        SDLK_PAGEDOWN,
    "Home",            SDLK_HOME,
    "End",             SDLK_END,
    "Return",          SDLK_RETURN,
    "Left Click",      SDL_BUTTON_LEFT,
    "Right Click",     SDL_BUTTON_RIGHT,
    "Middle Click",    SDL_BUTTON_MIDDLE,
    "Wheel down",      SDL_BUTTON_WHEELDOWN,
    "Wheel up",        SDL_BUTTON_WHEELUP,
    "Pad 0", SDLK_KP0,
    "Pad 1", SDLK_KP1,
    "Pad 2", SDLK_KP2,
    "Pad 3", SDLK_KP3,
    "Pad 4", SDLK_KP4,
    "Pad 5", SDLK_KP5,
    "Pad 6", SDLK_KP6,
    "Pad 7", SDLK_KP7,
    "Pad 8", SDLK_KP8,
    "Pad 9", SDLK_KP9,

    /* TODO: add more */
    NULL    
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
  
  int InputHandler::_StringToKey(const std::string &s) {
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

    /* Open all joysticks */
    for(int i=0;i<SDL_NumJoysticks();i++) {
      m_Joysticks.push_back( SDL_JoystickOpen(i) );
    }
        
    m_pActiveJoystick1 = NULL;
  }
  
  void InputHandler::uninit(void) {
    /* Close all joysticks */
    for(int i=0;i<m_Joysticks.size();i++) {
      SDL_JoystickClose( m_Joysticks[i] );
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
  void InputHandler::updateInput(std::vector<Biker*>& i_bikers) {
    /* Joystick? */
    /* joystick only for player 1 */
     if(m_ControllerModeID[0] == CONTROLLER_MODE_JOYSTICK1 && m_pActiveJoystick1 != NULL) {
      SDL_JoystickUpdate();     

      i_bikers[0]->getControler()->stopContols();
      
      /* Update buttons */
      for(int i=0;i<SDL_JoystickNumButtons(m_pActiveJoystick1);i++) {
        if(SDL_JoystickGetButton(m_pActiveJoystick1,i)) {
          if(!m_JoyButtonsPrev[i]) {
            /* Click! */
            if(m_nJoyButtonChangeDir1 == i) {
              i_bikers[0]->getControler()->setChangeDir(true);
            }
          }

          m_JoyButtonsPrev[i] = true;
        }        
        else
          m_JoyButtonsPrev[i] = false;
      }
      
      /** Update axis */           
      int nRawPrim = SDL_JoystickGetAxis(m_pActiveJoystick1,m_nJoyAxisPrim1);
      i_bikers[0]->getControler()->setDrive(-joyRawToFloat(nRawPrim, m_nJoyAxisPrimMin1, m_nJoyAxisPrimLL1, m_nJoyAxisPrimUL1, m_nJoyAxisPrimMax1));
      int nRawSec = SDL_JoystickGetAxis(m_pActiveJoystick1,m_nJoyAxisSec1);
      i_bikers[0]->getControler()->setPull(-joyRawToFloat(nRawSec, m_nJoyAxisSecMin1, m_nJoyAxisSecLL1, m_nJoyAxisSecUL1, m_nJoyAxisSecMax1));
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
  void InputHandler::configure(UserConfig *pConfig) {
    /* Set defaults */
    _SetDefaultConfig();
  
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
      m_nDriveKey[0] = _StringToKey(pConfig->getString("KeyDrive1"));
      m_nBrakeKey[0] = _StringToKey(pConfig->getString("KeyBrake1"));
      m_nPullBackKey[0] = _StringToKey(pConfig->getString("KeyFlipLeft1"));
      m_nPushForwardKey[0] = _StringToKey(pConfig->getString("KeyFlipRight1"));
      m_nChangeDirKey[0] = _StringToKey(pConfig->getString("KeyChangeDir1"));
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
	for(int i=0;i<SDL_JoystickNumButtons(m_pActiveJoystick1);i++) {
	  m_JoyButtonsPrev.push_back(false);
	}
	
      }
    }

    m_ControllerModeID[1] = CONTROLLER_MODE_KEYBOARD;
    m_ControllerModeID[2] = CONTROLLER_MODE_KEYBOARD;
    m_ControllerModeID[3] = CONTROLLER_MODE_KEYBOARD;

    m_nDriveKey[1]    	 = _StringToKey(pConfig->getString("KeyDrive2"));
    m_nBrakeKey[1]    	 = _StringToKey(pConfig->getString("KeyBrake2"));
    m_nPullBackKey[1] 	 = _StringToKey(pConfig->getString("KeyFlipLeft2"));
    m_nPushForwardKey[1] = _StringToKey(pConfig->getString("KeyFlipRight2"));
    m_nChangeDirKey[1]   = _StringToKey(pConfig->getString("KeyChangeDir2"));

    m_nDriveKey[2]    	 = _StringToKey(pConfig->getString("KeyDrive3"));
    m_nBrakeKey[2]    	 = _StringToKey(pConfig->getString("KeyBrake3"));
    m_nPullBackKey[2] 	 = _StringToKey(pConfig->getString("KeyFlipLeft3"));
    m_nPushForwardKey[2] = _StringToKey(pConfig->getString("KeyFlipRight3"));
    m_nChangeDirKey[2]   = _StringToKey(pConfig->getString("KeyChangeDir3"));

    m_nDriveKey[3]    	 = _StringToKey(pConfig->getString("KeyDrive4"));
    m_nBrakeKey[3]    	 = _StringToKey(pConfig->getString("KeyBrake4"));
    m_nPullBackKey[3] 	 = _StringToKey(pConfig->getString("KeyFlipLeft4"));
    m_nPushForwardKey[3] = _StringToKey(pConfig->getString("KeyFlipRight4"));
    m_nChangeDirKey[3]   = _StringToKey(pConfig->getString("KeyChangeDir4"));
    
#if defined(ENABLE_ZOOMING)          
    m_nZoomIn   = _StringToKey(pConfig->getString("KeyZoomIn"));
    m_nZoomOut  = _StringToKey(pConfig->getString("KeyZoomOut"));
    m_nZoomInit = _StringToKey(pConfig->getString("KeyZoomInit"));
    m_nCameraMoveXUp   = _StringToKey(pConfig->getString("KeyCameraMoveXUp"));
    m_nCameraMoveXDown = _StringToKey(pConfig->getString("KeyCameraMoveXDown"));
    m_nCameraMoveYUp   = _StringToKey(pConfig->getString("KeyCameraMoveYUp"));
    m_nCameraMoveYDown = _StringToKey(pConfig->getString("KeyCameraMoveYDown"));
#endif
    
    /* All good? */
    bool isUserKeyOk = true;
    for(unsigned int i=0; i<4; i++) {
      if(m_nDriveKey[i]<0 || m_nBrakeKey[i]<0 || m_nPullBackKey[i]<0 ||
	 m_nPushForwardKey[i]<0 || m_nChangeDirKey[i]<0) {
	isUserKeyOk = false;
      }
    }

    if(isUserKeyOk == false
#if defined(ENABLE_ZOOMING)
       || m_nZoomIn<0 || m_nZoomOut <0 || m_nZoomInit <0
       || m_nCameraMoveXUp<0 || m_nCameraMoveXDown<0 
       || m_nCameraMoveYUp<0 || m_nCameraMoveYDown<0
#endif
       ) {
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
      m_ScriptKeyHooks[m_nNumScriptKeyHooks].nKey = _StringToKey(KeyName);
      m_ScriptKeyHooks[m_nNumScriptKeyHooks].pGame = pGame;
      m_nNumScriptKeyHooks++;
    }
  }  

  /*===========================================================================
  Handle an input event
  ===========================================================================*/  
  void InputHandler::handleInput(InputEventType Type,int nKey,SDLMod mod,
				 std::vector<Biker*>& i_bikers,
				 std::vector<Camera*>& i_cameras,
				 GameApp *pGameApp) {
//#m_MotoGame.Players()[i]->isDead() == false) {
//
    /* Update controller 1 */
    /* Keyboard controlled */
 switch(Type) {
   case INPUT_KEY_DOWN:
   for(unsigned int i=0; i<i_bikers.size(); i++) {
     if(m_ControllerModeID[i] == CONTROLLER_MODE_KEYBOARD) {
       if(m_nDriveKey[i] == nKey) {
	 /* Start driving */
	 i_bikers[i]->getControler()->setDrive(1.0f);
       } else if(m_nBrakeKey[i] == nKey) {
	  /* Brake */
	  i_bikers[i]->getControler()->setDrive(-1.0f);
       } else if((m_nPullBackKey[i]    == nKey && m_mirrored == false) ||
		 (m_nPushForwardKey[i] == nKey && m_mirrored)) {
		   /* Pull back */
		   i_bikers[i]->getControler()->setPull(1.0f);
       } else if((m_nPushForwardKey[i] == nKey && m_mirrored == false) ||
		 (m_nPullBackKey[i]    == nKey && m_mirrored)) {
		   /* Push forward */
		   i_bikers[i]->getControler()->setPull(-1.0f);            
       }
     }
   }

#if defined(ENABLE_ZOOMING)          
      if(m_nZoomIn == nKey) {
	/* Zoom in */
	for(unsigned int i=0; i<i_cameras.size(); i++) {
	  i_cameras[i]->zoom(0.002);
	}
      } 
      else if(m_nZoomOut == nKey) {
	/* Zoom out */
	for(unsigned int i=0; i<i_cameras.size(); i++) {
	  i_cameras[i]->zoom(-0.002);
	}
      }
      else if(m_nZoomInit == nKey) {
	/* Zoom init */
	for(unsigned int i=0; i<i_cameras.size(); i++) {
	  i_cameras[i]->initCamera();
	}
      }
      else if(m_nCameraMoveXUp == nKey) {
	for(unsigned int i=0; i<i_cameras.size(); i++) {
	  i_cameras[i]->moveCamera(1.0, 0.0);
	}
      }
      else if(m_nCameraMoveXDown == nKey) {
	for(unsigned int i=0; i<i_cameras.size(); i++) {
	  i_cameras[i]->moveCamera(-1.0, 0.0);
	}
      }
      else if(m_nCameraMoveYUp == nKey) {
	for(unsigned int i=0; i<i_cameras.size(); i++) {
	  i_cameras[i]->moveCamera(0.0, 1.0);
	}
      }
      else if(m_nCameraMoveYDown == nKey) {
	for(unsigned int i=0; i<i_cameras.size(); i++) {
	  i_cameras[i]->moveCamera(0.0, -1.0);
	}
      } else if(nKey == SDLK_KP0 && ((mod & KMOD_LCTRL) == KMOD_LCTRL)) {
	for(unsigned int i=0; i<i_bikers.size(); i++) {
	  if(i_cameras.size() > 0) {
	    pGameApp->TeleportationCheatTo(i, Vector2f(i_cameras[0]->getCameraPositionX(),
						       i_cameras[0]->getCameraPositionY()));
	  }
	}
      }
#endif
      
      break;
      case INPUT_KEY_UP:
   for(unsigned int i=0; i<i_bikers.size(); i++) {
     if(m_ControllerModeID[i] == CONTROLLER_MODE_KEYBOARD) {
	if(m_nDriveKey[i] == nKey) {
	  /* Stop driving */
	  i_bikers[i]->getControler()->setDrive(0.0f);
	}
	else if(m_nBrakeKey[i] == nKey) {
	  /* Don't brake */
	  i_bikers[i]->getControler()->setDrive(0.0f);
	}
	else if((m_nPullBackKey[i]    == nKey && m_mirrored == false) ||
		(m_nPushForwardKey[i] == nKey && m_mirrored)) {
	  /* Pull back */
	  i_bikers[i]->getControler()->setPull(0.0f);
	}
	else if((m_nPushForwardKey[i] == nKey && m_mirrored == false) ||
		(m_nPullBackKey[i]    == nKey && m_mirrored)) {
	  /* Push forward */
	  i_bikers[i]->getControler()->setPull(0.0f);            
	}
	else if(m_nChangeDirKey[i] == nKey) {
	  /* Change dir */
	  i_bikers[i]->getControler()->setChangeDir(true);
	}
     }
   }
   break;
 }
    
    /* Have the script hooked this key? */
    if(Type == INPUT_KEY_DOWN) {
      for(int i=0;i<m_nNumScriptKeyHooks;i++) {
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
  void InputHandler::_SetDefaultConfig(void) {
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
    
    #if defined(ENABLE_ZOOMING)    
      m_nZoomIn          = SDLK_KP7;
      m_nZoomOut         = SDLK_KP9;
      m_nZoomInit        = SDLK_HOME;
      m_nCameraMoveXUp   = SDLK_KP6;
      m_nCameraMoveXDown = SDLK_KP4;
      m_nCameraMoveYUp   = SDLK_KP8;
      m_nCameraMoveYDown = SDLK_KP2;
    #endif
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
   
    #if defined(ENABLE_ZOOMING)
    if(m_nZoomIn          < 0) { m_nZoomIn          = SDLK_KP7;   }
    if(m_nZoomOut         < 0) { m_nZoomOut         = SDLK_KP9; }
    if(m_nZoomInit        < 0) { m_nZoomInit        = SDLK_HOME; }
    if(m_nCameraMoveXUp   < 0) { m_nCameraMoveXUp   = SDLK_KP6; }
    if(m_nCameraMoveXDown < 0) { m_nCameraMoveXDown = SDLK_KP4; }
    if(m_nCameraMoveYUp   < 0) { m_nCameraMoveYUp   = SDLK_KP8; }
    if(m_nCameraMoveYDown < 0) { m_nCameraMoveYDown = SDLK_KP2; }
    #endif
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
    
    #if defined(ENABLE_ZOOMING)    
    if(Action == "ZoomIn")   	    	return keyToString(m_nZoomIn);
    if(Action == "ZoomOut")  	    	return keyToString(m_nZoomOut);
    if(Action == "ZoomInit") 	    	return keyToString(m_nZoomInit);
    if(Action == "CameraMoveXUp")   return keyToString(m_nCameraMoveXUp);
    if(Action == "CameraMoveXDown") return keyToString(m_nCameraMoveXDown);
    if(Action == "CameraMoveYUp")   return keyToString(m_nCameraMoveYUp);
    if(Action == "CameraMoveYDown") return keyToString(m_nCameraMoveYDown);
    #endif

    return "?";
  }
