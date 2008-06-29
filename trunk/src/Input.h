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

#ifndef __INPUT_H__
#define __INPUT_H__

#include "VCommon.h"
#include "xmscene/Scene.h"
#include "UserConfig.h"
#include "Renderer.h"

class xmDatabase;

enum XMKey_input {XMK_KEYBOARD, XMK_MOUSEBUTTON};

/* define a key to do something (keyboard:a, mouse:left, ...) */
class XMKey {
 public:
  XMKey();
  XMKey(SDL_Event &i_event);
  XMKey(const std::string& i_key, bool i_basicMode = false); /* basic mode is to give a simple letter, for scripts key */
  XMKey(SDLKey nKey, SDLMod mod); // keyboard
  XMKey(Uint8 nButton);           // mouse

  bool operator==(const XMKey& i_other) const;
  std::string toString();
  std::string toFancyString();
  bool isPressed(Uint8 *i_keystate, Uint8 i_mousestate);

 private:
  XMKey_input m_input;
  SDLKey m_keyboard_sym;
  SDLMod m_keyboard_mod;
  Uint8  m_mouseButton_button;
};

  /*===========================================================================
  Controller modes
  ===========================================================================*/
  enum ControllerModeID {
    CONTROLLER_MODE_KEYBOARD,
    CONTROLLER_MODE_JOYSTICK1
  };

  /*===========================================================================
  Script hooks
  ===========================================================================*/
  #define MAX_SCRIPT_KEY_HOOKS 16
  
  struct InputScriptKeyHook {
    XMKey nKey;                 /* Hooked key */
    std::string FuncName;       /* Script function to invoke */    
    MotoGame *pGame;            /* Pointer to game */
  };

  /*===========================================================================
  Inputs
  ===========================================================================*/
  enum InputEventType {
    INPUT_KEY_DOWN,
    INPUT_KEY_UP,
    INPUT_MOUSE_DOWN,
    INPUT_MOUSE_UP
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
    ACTION_DRIVE,             /* Param: Throttle [0; 1] or Brake [-1; 0] */
    ACTION_PULL,              /* Param: Pull back on the handle bar [0; 1] or push forward on the handle bar [-1; 0] */
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
class InputHandler : public Singleton<InputHandler> {
  friend class Singleton<InputHandler>;

private:
  InputHandler();
  ~InputHandler() {}

public:
      
  /* Methods */
  void reset();
  void dealWithActivedKeys(Universe* i_universe); // apply already pressed keys
  void setMirrored(bool i_value);

  void loadConfig(UserConfig *pConfig, xmDatabase* pDb, const std::string& i_id_profile);
  void handleInput(Universe* i_universe, InputEventType Type, int nKey,SDLMod mod);      
  std::string waitForKey(void);
  void updateUniverseInput(Universe* i_universe);
  void init(UserConfig *pConfig, xmDatabase* pDb, const std::string& i_id_profile);
  void uninit();
      
  void resetScriptKeyHooks(void) {m_nNumScriptKeyHooks = 0;}
  void addScriptKeyHook(MotoGame *pGame,const std::string &basicKeyName,const std::string &FuncName);

  std::string getFancyKeyByAction(const std::string &Action);

  void setDefaultConfig();
  void saveConfig(UserConfig *pConfig, xmDatabase* pDb, const std::string& i_id_profile);
  void setDRIVE(int i_player, XMKey i_value);
  XMKey getDRIVE(int i_player) const;
  void setBRAKE(int i_player, XMKey i_value);
  XMKey getBRAKE(int i_player) const;
  void setFLIPLEFT(int i_player, XMKey i_value);
  XMKey getFLIPLEFT(int i_player) const;
  void setFLIPRIGHT(int i_player, XMKey i_value);
  XMKey getFLIPRIGHT(int i_player) const;
  void setCHANGEDIR(int i_player, XMKey i_value);
  XMKey getCHANGEDIR(int i_player) const;

private:

  /* Data */
  int m_nNumScriptKeyHooks;
  InputScriptKeyHook m_ScriptKeyHooks[MAX_SCRIPT_KEY_HOOKS];
      
  ControllerModeID m_ControllerModeID[4];
  InputAction m_ActiveAction;

  std::vector<SDL_Joystick *> m_Joysticks;
      
  /* For ControllerMode1 = CONTROLLER_MODE_KEYBOARD */
  XMKey m_nDriveKey[4];
  XMKey m_nBrakeKey[4];
  XMKey m_nPullBackKey[4];
  XMKey m_nPushForwardKey[4];
  XMKey m_nChangeDirKey[4];
  // to avoid key repetition
  bool m_changeDirKeyAlreadyPress[4];

  /* For ControllerMode1 = CONTROLLER_MODE_JOYSTICK1 */
  SDL_Joystick *m_pActiveJoystick1;
  int m_nJoyAxisPrim1; /**< Primary axis: Driving and braking */
  int m_nJoyAxisPrimMax1;
  int m_nJoyAxisPrimMin1;
  int m_nJoyAxisPrimUL1;
  int m_nJoyAxisPrimLL1;
  int m_nJoyAxisSec1; /**< Secondary axis: Pulling and pushing the handle bar */
  int m_nJoyAxisSecMax1;
  int m_nJoyAxisSecMin1;
  int m_nJoyAxisSecUL1;
  int m_nJoyAxisSecLL1;
  int m_nJoyButtonChangeDir1;
     
  bool m_mirrored;

  std::vector<bool> m_JoyButtonsPrev;
      
  /* Static data */
  static InputActionType m_ActionTypeTable[];
};


#endif
