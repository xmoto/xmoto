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

namespace vapp {

  /*===========================================================================
  Globals
  ===========================================================================*/  
  /* Action ID to string table */
  InputActionType InputHandler::m_ActionTypeTable[] = {
    "Drive",           ACTION_DRIVE,
    "Brake",           ACTION_BRAKE,             
    "PullBack",        ACTION_PULLBACK,          
    "PushForward",     ACTION_PUSHFORWARD,       
    "ChangeDirection", ACTION_CHANGEDIR,     
    #if defined(ENABLE_ZOOMING)    
      "ZoomIn",          ACTION_ZOOMIN,     
      "ZoomOut",         ACTION_ZOOMOUT,     
    #endif
    NULL
  };

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
    "Return",          SDLK_RETURN,
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

  /*===========================================================================
  Input device update
  ===========================================================================*/  
  void InputHandler::updateInput(BikeController *pController) {
    /* Joystick? */
    if(m_ControllerModeID1 == CONTROLLER_MODE_JOYSTICK1 && m_pActiveJoystick1 != NULL) {
      SDL_JoystickUpdate();     
      
      pController->fDrive = 0.0f;      
      pController->fBrake = 0.0f;    
      
      pController->bChangeDir = false;  
      pController->bPullBack = false;  
      pController->bPushForward = false;  
      
      /* Update buttons */
      for(int i=0;i<SDL_JoystickNumButtons(m_pActiveJoystick1);i++) {
        if(SDL_JoystickGetButton(m_pActiveJoystick1,i)) {
          if(!m_JoyButtonsPrev[i]) {
            /* Click! */
            if(m_nJoyButtonChangeDir1 == i) {
              pController->bChangeDir = true;
            }
          }
          
          /* Just down */
          if(m_nJoyButtonFlipLeft1 == i) {
            pController->bPullBack = true;
          }
          else if(m_nJoyButtonFlipRight1 == i) {
            pController->bPushForward = true;
          }
          
          m_JoyButtonsPrev[i] = true;
        }        
        else
          m_JoyButtonsPrev[i] = false;
      }
      
      /* Get the primary axis and map it. The following nice ASCII figure
         should explain how it is done :)
        
                     (+)
                   drive
                  throttle    _______
                      |      /|
                      |     / |
                      |    /  |
      (-)________ ____|___/___|____raw axis data (+)
                 /|   |   |   |
                / |   |   |   |
               /  |   |   |   |
         _____/   |   |   |   | 
              |   | brake |   |
              |   | power |   |
              |   |  (-)  |   |
             Min  LL     UL  Max
             
             
         Min: The minimum raw axis data to expect.
         Max: Maximum raw axis data to expect.
         LL: Lower limit. If the data is between this and zero, nothing 
             happens.
         UL: Upper limit. If the data is between this and zero, nothing 
             happens.
         Drive throttle: A value between 0 and 1.
         Brake power: A value between 0 and 1.
      */
            
      int nRaw = -SDL_JoystickGetAxis(m_pActiveJoystick1,m_nJoyAxisPrim1);
      
      if(m_nJoyAxisPrimMin1 > m_nJoyAxisPrimMax1) {
        /* Drive? */
        if(nRaw < m_nJoyAxisPrimUL1) {
          float f = ((float)(nRaw - m_nJoyAxisPrimUL1)) / 
                    (float)(m_nJoyAxisPrimMax1 - m_nJoyAxisPrimUL1);
          if(f>1.0f) f=1.0f;
          if(f>0.0f) {
            pController->fDrive = f;
          }
        }
        /* Brake? */
        else if(nRaw > m_nJoyAxisPrimLL1) {
          float f = ((float)(m_nJoyAxisPrimLL1 - nRaw)) / 
                    (float)(m_nJoyAxisPrimLL1 - m_nJoyAxisPrimMin1);
          if(f>1.0f) f=1.0f;
          if(f>0.0f) {
            pController->fBrake = f;
          }
        }
      }
      else if(m_nJoyAxisPrimMin1 < m_nJoyAxisPrimMax1) {
        /* Brake? */
        if(nRaw > m_nJoyAxisPrimUL1) {
          float f = ((float)(nRaw - m_nJoyAxisPrimUL1)) / 
                    (float)(m_nJoyAxisPrimMax1 - m_nJoyAxisPrimUL1);
          if(f>1.0f) f=1.0f;
          if(f>0.0f) {
            pController->fBrake = f;
          }
        }
        /* Drive? */
        else if(nRaw < m_nJoyAxisPrimLL1) {
          float f = ((float)(m_nJoyAxisPrimLL1 - nRaw)) / 
                    (float)(m_nJoyAxisPrimLL1 - m_nJoyAxisPrimMin1);
          if(f>1.0f) f=1.0f;
          if(f>0.0f) {
            pController->fDrive = f;
          }
        }
      }
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
    std::string ControllerMode = pConfig->getString("ControllerMode1");    
    if(ControllerMode != "Keyboard" && ControllerMode != "Joystick1") {
      ControllerMode = "Keyboard"; /* go default then */
      Log("** Warning ** : 'ControllerMode1' must be either 'Keyboard' or 'Joystick1'!");
    }

    while(1) {    
      /* Get settings for mode */
      if(ControllerMode == "Keyboard") {
        /* We're using the keyboard */
        m_ControllerModeID1 = CONTROLLER_MODE_KEYBOARD;
        
        m_nDriveKey1 = _StringToKey(pConfig->getString("KeyDrive1"));
        m_nBrakeKey1 = _StringToKey(pConfig->getString("KeyBrake1"));
        m_nPullBackKey1 = _StringToKey(pConfig->getString("KeyFlipLeft1"));
        m_nPushForwardKey1 = _StringToKey(pConfig->getString("KeyFlipRight1"));
        m_nChangeDirKey1 = _StringToKey(pConfig->getString("KeyChangeDir1"));

        #if defined(ENABLE_ZOOMING)          
	        m_nZoomIn = _StringToKey(pConfig->getString("KeyZoomIn"));
	        m_nZoomOut = _StringToKey(pConfig->getString("KeyZoomOut"));
	      #endif
        
        /* All good? */
        if(m_nDriveKey1<0 || m_nBrakeKey1<0 || m_nPullBackKey1<0 ||
          m_nPushForwardKey1<0 || m_nChangeDirKey1<0 
          #if defined(ENABLE_ZOOMING)
            ||m_nZoomIn<0 || m_nZoomOut <0
          #endif
	        ) {
          Log("** Warning ** : Invalid keyboard configuration!");
	  _SetDefaultConfigToUnsetKeys();
        }
        break;
      }
      else if(ControllerMode == "Joystick1") {
        /* We're using joystick 1 */
        m_ControllerModeID1 = CONTROLLER_MODE_JOYSTICK1;      
        
        int nIdx = pConfig->getInteger("JoyIdx1");
        if(nIdx < 0) {
          Log("** Warning ** : Joystick is not configured, falling back to keyboard!");
          ControllerMode = "Keyboard";
        }
        else {
          m_pActiveJoystick1 = m_Joysticks[nIdx];
        
          /* Okay, fetch the rest of the config */          
          m_nJoyAxisPrim1 = pConfig->getInteger("JoyAxisPrim1");
          m_nJoyAxisPrimMax1 = pConfig->getInteger("JoyAxisPrimMax1");
          m_nJoyAxisPrimMin1 = pConfig->getInteger("JoyAxisPrimMin1");
          m_nJoyAxisPrimUL1 = pConfig->getInteger("JoyAxisPrimUL1");
          m_nJoyAxisPrimLL1 = pConfig->getInteger("JoyAxisPrimLL1");
          m_nJoyButtonFlipLeft1 = pConfig->getInteger("JoyButtonFlipLeft1");
          m_nJoyButtonFlipRight1 = pConfig->getInteger("JoyButtonFlipRight1");
          m_nJoyButtonChangeDir1 = pConfig->getInteger("JoyButtonChangeDir1");
          
          /* Init all joystick buttons */
          m_JoyButtonsPrev.clear();
          for(int i=0;i<SDL_JoystickNumButtons(m_pActiveJoystick1);i++) {
            m_JoyButtonsPrev.push_back(false);
          }
          
          break;
        }
      }
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
				 BikeController *pController,
				 GameRenderer *pGameRender) {
    /* Update controller 1 */
    if(m_ControllerModeID1 == CONTROLLER_MODE_KEYBOARD) {
      /* Keyboard controlled */
      switch(Type) {
        case INPUT_KEY_DOWN: 
          if(m_nDriveKey1 == nKey) {
            /* Start driving */
            pController->fDrive = 1.0f;
          }
          else if(m_nBrakeKey1 == nKey) {
            /* Brake */
            pController->fBrake = 1.0f;
          }
          else if(m_nPullBackKey1 == nKey) {
            /* Pull back */
            pController->bPullBack = true;
          }
          else if(m_nPushForwardKey1 == nKey) {
            /* Push forward */
            pController->bPushForward = true;            
          } 
          #if defined(ENABLE_ZOOMING)          
          else if(m_nZoomIn == nKey) {
	          /* Zoom in */
	          pGameRender->zoom(0.002);
	        } 
	        else if(m_nZoomOut == nKey) {
	          /* Zoom out */
	          pGameRender->zoom(-0.002);
	        }
	        #endif

          break;
        case INPUT_KEY_UP:
          if(m_nDriveKey1 == nKey) {
            /* Stop driving */
            pController->fDrive = 0.0f;
          }
          else if(m_nBrakeKey1 == nKey) {
            /* Don't brake */
            pController->fBrake = 0.0f;
          }
          else if(m_nPullBackKey1 == nKey) {
            /* Pull back */
            pController->bPullBack = false;
          }
          else if(m_nPushForwardKey1 == nKey) {
            /* Push forward */
            pController->bPushForward = false;            
          }
          else if(m_nChangeDirKey1 == nKey) {
            /* Change dir */
            pController->bChangeDir = true;
          }
          break;
      }      
    }
    
    /* Have the script hooked this key? */
    if(Type == INPUT_KEY_DOWN) {
      for(int i=0;i<m_nNumScriptKeyHooks;i++) {
        if(m_ScriptKeyHooks[i].nKey == nKey) {
          /* Invoke script */
          m_ScriptKeyHooks[i].pGame->scriptCallVoid(m_ScriptKeyHooks[i].FuncName);
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
    
    #if defined(ENABLE_ZOOMING)    
      m_nZoomIn          = SDLK_PAGEUP;
      m_nZoomOut         = SDLK_PAGEDOWN;
    #endif
  }  

  void InputHandler::_SetDefaultConfigToUnsetKeys() {
    m_ControllerModeID1 = CONTROLLER_MODE_KEYBOARD;
    
    if(m_nDriveKey1       < 0) { m_nDriveKey1 	    = SDLK_UP;       }
    if(m_nBrakeKey1       < 0) { m_nBrakeKey1 	    = SDLK_DOWN;     }
    if(m_nPullBackKey1    < 0) { m_nPullBackKey1    = SDLK_LEFT;     }
    if(m_nPushForwardKey1 < 0) { m_nPushForwardKey1 = SDLK_RIGHT;    }
    if(m_nChangeDirKey1   < 0) { m_nChangeDirKey1   = SDLK_SPACE;    }
    
    #if defined(ENABLE_ZOOMING)
      if(m_nZoomIn          < 0) { m_nZoomIn          = SDLK_PAGEUP;   }
      if(m_nZoomOut         < 0) { m_nZoomOut         = SDLK_PAGEDOWN; }
    #endif
  }

  /*===========================================================================
  Get key by action...
  ===========================================================================*/  
  std::string InputHandler::getKeyByAction(const std::string &Action) {
    if(m_ControllerModeID1 != CONTROLLER_MODE_KEYBOARD) return "?";
    
    if(Action == "Drive") return _KeyToString(m_nDriveKey1);
    if(Action == "Brake") return _KeyToString(m_nBrakeKey1);
    if(Action == "PullBack") return _KeyToString(m_nPullBackKey1);
    if(Action == "PushForward") return _KeyToString(m_nPushForwardKey1);
    if(Action == "ChangeDir") return _KeyToString(m_nChangeDirKey1);
    
    #if defined(ENABLE_ZOOMING)    
      if(Action == "ZoomIn") return _KeyToString(m_nZoomIn);
      if(Action == "ZoomOut") return _KeyToString(m_nZoomOut);
    #endif

    return "?";
  }

};

