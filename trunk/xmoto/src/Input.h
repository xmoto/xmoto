/*=============================================================================
XMOTO
Copyright (C) 2005 Rasmus Neckelmann (neckelmann@gmail.com)

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

#ifndef __INPUT_H__
#define __INPUT_H__

#include "VCommon.h"
#include "VApp.h"
#include "MotoGame.h"
#include "UserConfig.h"

namespace vapp {

  /*===========================================================================
  Controller modes
  ===========================================================================*/
  enum ControllerModeID {
    CONTROLLER_MODE_KEYBOARD,
    CONTROLLER_MODE_JOYSTICK1
  };

  /*===========================================================================
  Inputs
  ===========================================================================*/
  enum InputEventType {
    INPUT_KEY_DOWN,
    INPUT_KEY_UP,    
  };
  
  struct InputKeyMap {
    const char *pcKey;        /* Name */
    int nKey;                 /* SDL key sym */
  };
  
  /*===========================================================================
  Input actions
  ===========================================================================*/
  //enum InputActionBehaviour {
  //  ACTION_BEHAVIOUR_INSTANT, /* A kind of action that takes effect instantly
  //                               when the user control is activated */
  //  ACTION_BEHAVIOUR_LINEAR_RAMP,
  //};
  
  enum InputActionTypeID {
    ACTION_NONE,
    ACTION_DRIVE,             /* Param: Throttle (0-1) */
    ACTION_BRAKE,             /* Param: Brake amount (0-1) */
    ACTION_PULLBACK,          /* Param: None */
    ACTION_PUSHFORWARD,       /* Param: None */
    ACTION_CHANGEDIR,         /* Param: None */
  };
    
  struct InputAction {
    InputActionTypeID TypeID; /* Action type */
    float fParam;             /* Action parameter */
  };
  
  struct InputActionType {
    const char *pcDesc;       /* Descriptive string */
    InputActionTypeID TypeID; /* The ID */
    //InputActionBehaviour
  };

  /*===========================================================================
  Input handler class
  ===========================================================================*/
  class InputHandler {
    public:
      
      /* Methods */
      void configure(UserConfig *pConfig);
      void handleInput(InputEventType Type,int nKey,BikeController *pController);
      std::string waitForKey(void);
      void updateInput(BikeController *pController);
      void init(UserConfig *pConfig);
      void uninit(void);
    
    private:

      /* Data */
      ControllerModeID m_ControllerModeID1;      
      InputAction m_ActiveAction;

      std::vector<SDL_Joystick *> m_Joysticks;
      
      /* For ControllerMode1 = CONTROLLER_MODE_KEYBOARD */
      int m_nDriveKey1;
      int m_nBrakeKey1;
      int m_nPullBackKey1;
      int m_nPushForwardKey1;
      int m_nChangeDirKey1;

      /* For ControllerMode1 = CONTROLLER_MODE_JOYSTICK1 */
      SDL_Joystick *m_pActiveJoystick1;
      int m_nJoyAxisPrim1;
      int m_nJoyAxisPrimMax1;
      int m_nJoyAxisPrimMin1;
      int m_nJoyAxisPrimUL1;
      int m_nJoyAxisPrimLL1;
      int m_nJoyButtonFlipLeft1;
      int m_nJoyButtonFlipRight1;
      int m_nJoyButtonChangeDir1;
      
      std::vector<bool> m_JoyButtonsPrev;
      
      /* Static data */
      static InputActionType m_ActionTypeTable[];
      static InputKeyMap m_KeyMap[];
      
      /* Helpers */
      std::string _KeyToString(int nKey);
      int _StringToKey(const std::string &s);
      
      /* Set default config */
      void _SetDefaultConfig(void);
  };
  
};


#endif