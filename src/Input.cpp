/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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

namespace vapp {

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
  std::string InputHandler::_KeyToString(int nKey) {
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
  void InputHandler::updateInput(BikeController *pController,
				 int i_player) {
    if(pController == NULL) {return;}

    /* joystick only for player one */
    if(i_player != 0) {return;}

    /* Joystick? */
     if(m_ControllerModeID1 == CONTROLLER_MODE_JOYSTICK1 && m_pActiveJoystick1 != NULL) {
      SDL_JoystickUpdate();     

      pController->stopContols();
      
      /* Update buttons */
      for(int i=0;i<SDL_JoystickNumButtons(m_pActiveJoystick1);i++) {
        if(SDL_JoystickGetButton(m_pActiveJoystick1,i)) {
          if(!m_JoyButtonsPrev[i]) {
            /* Click! */
            if(m_nJoyButtonChangeDir1 == i) {
              pController->setChangeDir(true);
            }
          }

          m_JoyButtonsPrev[i] = true;
        }        
        else
          m_JoyButtonsPrev[i] = false;
      }
      
      /** Update axis */           
      int nRawPrim = SDL_JoystickGetAxis(m_pActiveJoystick1,m_nJoyAxisPrim1);
      pController->setDrive(-joyRawToFloat(nRawPrim, m_nJoyAxisPrimMin1, m_nJoyAxisPrimLL1, m_nJoyAxisPrimUL1, m_nJoyAxisPrimMax1));
      int nRawSec = SDL_JoystickGetAxis(m_pActiveJoystick1,m_nJoyAxisSec1);
      pController->setPull(-joyRawToFloat(nRawSec, m_nJoyAxisSecMin1, m_nJoyAxisSecLL1, m_nJoyAxisSecUL1, m_nJoyAxisSecMax1));
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
	  Ret = _KeyToString(Event.button.button);
	  if(Ret != "") bWait = false;
	  break;
	case SDL_KEYDOWN:
            if(Event.key.keysym.sym == SDLK_ESCAPE) return "<<CANCEL>>";
          
            Ret = _KeyToString(Event.key.keysym.sym);
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
      Log("** Warning ** : 'ControllerMode1' must be either 'Keyboard' or 'Joystick1'!");
    }

    /* Get settings for mode */
    if(ControllerMode1 == "Keyboard") {
      /* We're using the keyboard */
      m_ControllerModeID1 = CONTROLLER_MODE_KEYBOARD;
      m_nDriveKey1 = _StringToKey(pConfig->getString("KeyDrive1"));
      m_nBrakeKey1 = _StringToKey(pConfig->getString("KeyBrake1"));
      m_nPullBackKey1 = _StringToKey(pConfig->getString("KeyFlipLeft1"));
      m_nPushForwardKey1 = _StringToKey(pConfig->getString("KeyFlipRight1"));
      m_nChangeDirKey1 = _StringToKey(pConfig->getString("KeyChangeDir1"));
    } else {
      /* We're using joystick 1 */
      m_ControllerModeID1 = CONTROLLER_MODE_JOYSTICK1;      
      
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

    m_nDriveKey2 = _StringToKey(pConfig->getString("KeyDrive2"));
    m_nBrakeKey2 = _StringToKey(pConfig->getString("KeyBrake2"));
    m_nPullBackKey2 = _StringToKey(pConfig->getString("KeyFlipLeft2"));
    m_nPushForwardKey2 = _StringToKey(pConfig->getString("KeyFlipRight2"));
    m_nChangeDirKey2 = _StringToKey(pConfig->getString("KeyChangeDir2"));
    
#if defined(ENABLE_ZOOMING)          
    m_nZoomIn   = _StringToKey(pConfig->getString("KeyZoomIn"));
    m_nZoomOut  = _StringToKey(pConfig->getString("KeyZoomOut"));
    m_nZoomInit = _StringToKey(pConfig->getString("KeyZoomInit"));
    m_nCameraMoveXUp   = _StringToKey(pConfig->getString("KeyCameraMoveXUp"));
    m_nCameraMoveXDown = _StringToKey(pConfig->getString("KeyCameraMoveXDown"));
    m_nCameraMoveYUp   = _StringToKey(pConfig->getString("KeyCameraMoveYUp"));
    m_nCameraMoveYDown = _StringToKey(pConfig->getString("KeyCameraMoveYDown"));
    m_nAutoZoom        = _StringToKey(pConfig->getString("KeyAutoZoom"));
#endif
    
    /* All good? */
    if(m_nDriveKey1<0 || m_nBrakeKey1<0 || m_nPullBackKey1<0 ||
       m_nPushForwardKey1<0 || m_nChangeDirKey1<0 ||
       m_nDriveKey2<0 || m_nBrakeKey2<0 || m_nPullBackKey2<0 ||
       m_nPushForwardKey2<0 || m_nChangeDirKey2<0 
#if defined(ENABLE_ZOOMING)
       || m_nZoomIn<0 || m_nZoomOut <0 || m_nZoomInit <0
       || m_nCameraMoveXUp<0 || m_nCameraMoveXDown<0 
       || m_nCameraMoveYUp<0 || m_nCameraMoveYDown<0
       || m_nAutoZoom<0
#endif
       ) {
      Log("** Warning ** : Invalid keyboard configuration!");
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
  void InputHandler::handleInput(InputEventType Type,
				 int nKey,
				 SDLMod mod,
				 BikeController *pController,
				 int i_player,
				 GameRenderer *pGameRender,
				 GameApp *pGameApp) {
    if(pController == NULL) {return;}

    /* Update controller 1 */
    /* Keyboard controlled */
    switch(Type) {
      case INPUT_KEY_DOWN: 
      if(i_player == 0 && m_ControllerModeID1 == CONTROLLER_MODE_KEYBOARD) {
	if(m_nDriveKey1 == nKey) {
	  /* Start driving */
	  pController->setDrive(1.0f);
	}
	else if(m_nBrakeKey1 == nKey) {
	  /* Brake */
	  pController->setDrive(-1.0f);
	}
	else if(m_nPullBackKey1 == nKey) {
	  /* Pull back */
            pController->setPull(1.0f);
	}
	else if(m_nPushForwardKey1 == nKey) {
	  /* Push forward */
	  pController->setPull(-1.0f);            
	}
      } else if(i_player == 1) {
	if(m_nDriveKey2 == nKey) {
	  /* Start driving */
	  pController->setDrive(1.0f);
	}
	else if(m_nBrakeKey2 == nKey) {
	  /* Brake */
	  pController->setDrive(-1.0f);
	}
          else if(m_nPullBackKey2 == nKey) {
            /* Pull back */
            pController->setPull(1.0f);
          }
	else if(m_nPushForwardKey2 == nKey) {
	  /* Push forward */
	  pController->setPull(-1.0f);            
	} 
      }

#if defined(ENABLE_ZOOMING)          
      if(m_nZoomIn == nKey) {
	/* Zoom in */
	pGameRender->zoom(0.002);
      } 
      else if(m_nZoomOut == nKey) {
	/* Zoom out */
	pGameRender->zoom(-0.002);
      }
      else if(m_nZoomInit == nKey) {
	/* Zoom init */
	pGameRender->initCamera();
      }
      else if(m_nCameraMoveXUp == nKey) {
	pGameRender->moveCamera(1.0, 0.0);
      }
      else if(m_nCameraMoveXDown == nKey) {
	pGameRender->moveCamera(-1.0, 0.0);
      }
      else if(m_nCameraMoveYUp == nKey) {
	  pGameRender->moveCamera(0.0, 1.0);
      }
      else if(m_nCameraMoveYDown == nKey) {
	pGameRender->moveCamera(0.0, -1.0);
      }
      else if(m_nAutoZoom == nKey) {
	pGameApp->setAutoZoom(true);
	pGameApp->setAutoUnZoom(false);
      } else if(nKey == SDLK_KP0 && ((mod & KMOD_LCTRL) == KMOD_LCTRL)) {
	pGameApp->TeleportationCheatTo(i_player, Vector2f(pGameRender->getCameraPositionX(),
							  pGameRender->getCameraPositionY()));
      }
#endif
      
      break;
      case INPUT_KEY_UP:
      if(i_player == 0 && m_ControllerModeID1 == CONTROLLER_MODE_KEYBOARD) {
	if(m_nDriveKey1 == nKey) {
	  /* Stop driving */
	  pController->setDrive(0.0f);
	}
	else if(m_nBrakeKey1 == nKey) {
	  /* Don't brake */
	  pController->setDrive(0.0f);
	}
	else if(m_nPullBackKey1 == nKey) {
	  /* Pull back */
	  pController->setPull(0.0f);
	}
	else if(m_nPushForwardKey1 == nKey) {
	  /* Push forward */
	  pController->setPull(0.0f);            
	}
	else if(m_nChangeDirKey1 == nKey) {
	  /* Change dir */
	  pController->setChangeDir(true);
	}
      } else if(i_player == 1) {
	if(m_nDriveKey2 == nKey) {
	  /* Stop driving */
	  pController->setDrive(0.0f);
	}
	else if(m_nBrakeKey2 == nKey) {
	  /* Don't brake */
	  pController->setDrive(0.0f);
	}
	else if(m_nPullBackKey2 == nKey) {
	  /* Pull back */
	  pController->setPull(0.0f);
	}
	else if(m_nPushForwardKey2 == nKey) {
	  /* Push forward */
	  pController->setPull(0.0f);            
	}
	else if(m_nChangeDirKey2 == nKey) {
	  /* Change dir */
	  pController->setChangeDir(true);
	}
      }
      
      if(m_nAutoZoom == nKey) {
	pGameApp->setAutoZoom(false);
	pGameApp->setAutoUnZoom(true);
      }
      break;
    }      
    
    /* Have the script hooked this key? */
    if(i_player == 0) { // handle script key only for player 0 to not call them several times
      if(Type == INPUT_KEY_DOWN) {
	for(int i=0;i<m_nNumScriptKeyHooks;i++) {
	  if(m_ScriptKeyHooks[i].nKey == nKey) {
	    /* Invoke script */
	    m_ScriptKeyHooks[i].pGame->getLuaLibGame()->scriptCallVoid(m_ScriptKeyHooks[i].FuncName);
	  }
	}
      }
    }
  }
  
  /*===========================================================================
  Set totally default configuration - useful for when something goes wrong
  ===========================================================================*/  
  void InputHandler::_SetDefaultConfig(void) {
    m_ControllerModeID1 = CONTROLLER_MODE_KEYBOARD;
    
    m_nDriveKey1       = SDLK_UP;
    m_nBrakeKey1       = SDLK_DOWN;
    m_nPullBackKey1    = SDLK_LEFT;
    m_nPushForwardKey1 = SDLK_RIGHT;
    m_nChangeDirKey1   = SDLK_SPACE;

    m_nDriveKey2       = SDLK_a;
    m_nBrakeKey2       = SDLK_q;
    m_nPullBackKey2    = SDLK_z;
    m_nPushForwardKey2 = SDLK_e;
    m_nChangeDirKey2   = SDLK_w;
    
    #if defined(ENABLE_ZOOMING)    
      m_nZoomIn          = SDLK_PAGEUP;
      m_nZoomOut         = SDLK_PAGEDOWN;
      m_nZoomInit        = SDLK_HOME;
      m_nCameraMoveXUp   = SDLK_KP6;
      m_nCameraMoveXDown = SDLK_KP4;
      m_nCameraMoveYUp   = SDLK_KP8;
      m_nCameraMoveYDown = SDLK_KP2;
      m_nAutoZoom        = SDLK_KP5;
    #endif
  }  

  void InputHandler::_SetDefaultConfigToUnsetKeys() {
    m_ControllerModeID1 = CONTROLLER_MODE_KEYBOARD;
    
    if(m_nDriveKey1       < 0) { m_nDriveKey1 	    = SDLK_UP;       }
    if(m_nBrakeKey1       < 0) { m_nBrakeKey1 	    = SDLK_DOWN;     }
    if(m_nPullBackKey1    < 0) { m_nPullBackKey1    = SDLK_LEFT;     }
    if(m_nPushForwardKey1 < 0) { m_nPushForwardKey1 = SDLK_RIGHT;    }
    if(m_nChangeDirKey1   < 0) { m_nChangeDirKey1   = SDLK_SPACE;    }
 
    if(m_nDriveKey2       < 0) { m_nDriveKey2 	    = SDLK_a;       }
    if(m_nBrakeKey2       < 0) { m_nBrakeKey2 	    = SDLK_q;     }
    if(m_nPullBackKey2    < 0) { m_nPullBackKey2    = SDLK_z;     }
    if(m_nPushForwardKey2 < 0) { m_nPushForwardKey2 = SDLK_e;    }
    if(m_nChangeDirKey2   < 0) { m_nChangeDirKey2   = SDLK_w;    }
   
    #if defined(ENABLE_ZOOMING)
    if(m_nZoomIn          < 0) { m_nZoomIn          = SDLK_PAGEUP;   }
    if(m_nZoomOut         < 0) { m_nZoomOut         = SDLK_PAGEDOWN; }
    if(m_nZoomInit        < 0) { m_nZoomInit        = SDLK_HOME; }
    if(m_nCameraMoveXUp   < 0) { m_nCameraMoveXUp   = SDLK_KP6; }
    if(m_nCameraMoveXDown < 0) { m_nCameraMoveXDown = SDLK_KP4; }
    if(m_nCameraMoveYUp   < 0) { m_nCameraMoveYUp   = SDLK_KP8; }
    if(m_nCameraMoveYDown < 0) { m_nCameraMoveYDown = SDLK_KP2; }
    if(m_nAutoZoom        < 0) { m_nAutoZoom        = SDLK_KP5; }
    #endif
  }

  /*===========================================================================
  Get key by action...
  ===========================================================================*/  
  std::string InputHandler::getKeyByAction(const std::string &Action) {
    if(m_ControllerModeID1 != CONTROLLER_MODE_KEYBOARD) return "?";
    
    if(Action == "Drive")    	return _KeyToString(m_nDriveKey1);
    if(Action == "Brake")    	return _KeyToString(m_nBrakeKey1);
    if(Action == "PullBack") 	return _KeyToString(m_nPullBackKey1);
    if(Action == "PushForward") return _KeyToString(m_nPushForwardKey1);
    if(Action == "ChangeDir")   return _KeyToString(m_nChangeDirKey1);

    if(Action == "Drive 2")    	  return _KeyToString(m_nDriveKey2);
    if(Action == "Brake 2")    	  return _KeyToString(m_nBrakeKey2);
    if(Action == "PullBack 2") 	  return _KeyToString(m_nPullBackKey2);
    if(Action == "PushForward 2") return _KeyToString(m_nPushForwardKey2);
    if(Action == "ChangeDir 2")   return _KeyToString(m_nChangeDirKey2);
    
    #if defined(ENABLE_ZOOMING)    
    if(Action == "ZoomIn")   	    	return _KeyToString(m_nZoomIn);
    if(Action == "ZoomOut")  	    	return _KeyToString(m_nZoomOut);
    if(Action == "ZoomInit") 	    	return _KeyToString(m_nZoomInit);
    if(Action == "CameraMoveXUp")   return _KeyToString(m_nCameraMoveXUp);
    if(Action == "CameraMoveXDown") return _KeyToString(m_nCameraMoveXDown);
    if(Action == "CameraMoveYUp")   return _KeyToString(m_nCameraMoveYUp);
    if(Action == "CameraMoveYDown") return _KeyToString(m_nCameraMoveYDown);
    if(Action == "AutoZoom")        return _KeyToString(m_nAutoZoom);
    #endif

    return "?";
  }

}
