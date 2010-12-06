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

#define INPUT_NB_PLAYERS 4

#include "XMKey.h"
#include "helpers/Singleton.h"
#include <string>
#include <vector>

class xmDatabase;
class UserConfig;
class Universe;
class Scene;

struct IGlobalKey {
  IGlobalKey();
  IGlobalKey(const std::string& i_name, const XMKey& i_key, const std::string i_help);

  std::string name;
  XMKey key;
  std::string help;
};

// write them in the order you want to see displayed them in the options list
enum INPUT_GLOBALKEYS {
  INPUT_SWITCHFAVORITE = 0,
  INPUT_SWITCHBLACKLIST,
  INPUT_SWITCHUGLYMODE,
  INPUT_RESTARTLEVEL,
  INPUT_RESTARTCHECKPOINT,
  INPUT_SHOWCONSOLE,
  INPUT_CHAT,
  INPUT_CONSOLEHISTORYPLUS,
  INPUT_CONSOLEHISTORYMINUS,

  INPUT_NB_GLOBALKEYS
};

  /*===========================================================================
  Script hooks
  ===========================================================================*/
  #define MAX_SCRIPT_KEY_HOOKS 16
  
  struct InputScriptKeyHook {
    XMKey nKey;                 /* Hooked key */
    std::string FuncName;       /* Script function to invoke */    
    Scene *pGame;            /* Pointer to game */
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

  void loadConfig(UserConfig *pConfig, xmDatabase* pDb, const std::string& i_id_profile);
  void init(UserConfig *pConfig, xmDatabase* pDb, const std::string& i_id_profile, bool i_enableJoysticks);
  void uninit();

  void resetScriptKeyHooks(void) {m_nNumScriptKeyHooks = 0;}
  void addScriptKeyHook(Scene *pGame,const std::string &keyName,const std::string &FuncName);

  int getNumScriptKeyHooks() const;
  InputScriptKeyHook getScriptKeyHooks(int i) const;
  XMKey getScriptActionKeys(int i_player, int i_actionScript) const;

  std::string getKeyByAction(const std::string &Action, bool i_tech = false);
  std::string* getJoyId(Uint8 i_joynum);
  Uint8 getJoyNum(const std::string& i_name);
  std::string* getJoyIdByStrId(const std::string& i_name);
  SDL_Joystick* getJoyById(std::string* i_id);
  InputEventType joystickAxisSens(Sint16 m_joyAxisValue);
  void recheckJoysticks();
  std::vector<std::string>& getJoysticksNames();
  bool areJoysticksEnabled () const;
  void enableJoysticks(bool i_value);

  void setDefaultConfig();
  void saveConfig(UserConfig *pConfig, xmDatabase* pDb, const std::string& i_id_profile);

  bool isANotGameSetKey(XMKey* i_xmkey) const;
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
  void setSCRIPTACTION(int i_player, int i_action, XMKey i_value);
  XMKey getSCRIPTACTION(int i_player, int i_action) const;

  void setGlobalKey(unsigned int INPUT_key, XMKey i_value);
  const XMKey* getGlobalKey(unsigned int INPUT_key) const;
  std::string getGlobalKeyHelp(unsigned int INPUT_key) const;

  static float joyRawToFloat(float raw, float neg, float deadzone_neg, float deadzone_pos, float pos);

private:

  /* Data */
  int m_nNumScriptKeyHooks;
  InputScriptKeyHook m_ScriptKeyHooks[MAX_SCRIPT_KEY_HOOKS];
      
  std::vector<SDL_Joystick *> m_Joysticks;
  std::vector<std::string>    m_JoysticksNames;
  std::vector<std::string>    m_JoysticksIds;
      
  XMKey m_nDriveKey[INPUT_NB_PLAYERS];
  XMKey m_nBrakeKey[INPUT_NB_PLAYERS];
  XMKey m_nPullBackKey[INPUT_NB_PLAYERS];
  XMKey m_nPushForwardKey[INPUT_NB_PLAYERS];
  XMKey m_nChangeDirKey[INPUT_NB_PLAYERS];
  XMKey m_nScriptActionKeys[INPUT_NB_PLAYERS][MAX_SCRIPT_KEY_HOOKS];

  IGlobalKey m_globalKeys[INPUT_NB_GLOBALKEYS];
};

#endif
