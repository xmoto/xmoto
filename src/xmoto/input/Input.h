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

#include "XMKey.h"
#include "helpers/Singleton.h"
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

class xmDatabase;
class UserConfig;
class Universe;
class Scene;
class InputSDL12Compat;

static const int INPUT_NB_PLAYERS = 4;
static const int MAX_SCRIPT_KEY_HOOKS = 16;

struct IFullKey {
  IFullKey();
  IFullKey(const std::string &i_name,
           const XMKey &i_defaultKey,
           const std::string i_help,
           bool i_customizable = true);

  std::string name;
  XMKey key;
  XMKey defaultKey;
  std::string help;
  bool customizable;
};

// write them in the order you want to see displayed them in the options list
enum INPUT_GLOBALKEYS {
  INPUT_SWITCHFAVORITE = 0,
  INPUT_SWITCHBLACKLIST,
  INPUT_SWITCHUGLYMODE,
  INPUT_RESTARTLEVEL,
  INPUT_RESTARTCHECKPOINT,
  INPUT_NEXTLEVEL,
  INPUT_PREVIOUSLEVEL,
  INPUT_LEVELWATCHING,
  INPUT_SWITCHPLAYER,
  INPUT_SWITCHTRACKINGSHOTMODE,
  INPUT_SWITCHRENDERGHOSTTRAIL,
  INPUT_SCREENSHOT,
  INPUT_LEVELINFO,
  INPUT_SWITCHWWWACCESS,
  INPUT_SWITCHFPS,
  INPUT_SWITCHGFXQUALITYMODE,
  INPUT_SWITCHGFXMODE,
  INPUT_SHOWCONSOLE,
  INPUT_CHAT,
  INPUT_CHATPRIVATE,
  INPUT_CONSOLEHISTORYPLUS,
  INPUT_CONSOLEHISTORYMINUS,
  INPUT_SWITCHNETMODE,
  INPUT_SWITCHHIGHSCOREINFORMATION,
  INPUT_NETWORKADMINCONSOLE,
  INPUT_SWITCHSAFEMODE,
  INPUT_TOGGLESERVERCONN,

  // uncustomizable keys
  INPUT_HELP,
  INPUT_RELOADFILESTODB,
  INPUT_PLAYINGPAUSE,
  INPUT_KILLPROCESS,
  INPUT_REPLAYINGREWIND,
  INPUT_REPLAYINGFORWARD,
  INPUT_REPLAYINGPAUSE,
  INPUT_REPLAYINGSTOP,
  INPUT_REPLAYINGFASTER,
  INPUT_REPLAYINGABITFASTER,
  INPUT_REPLAYINGSLOWER,
  INPUT_REPLAYINGABITSLOWER,

  INPUT_NB_GLOBALKEYS
};

enum INPUT_PLAYERKEYS {
  INPUT_DRIVE = 0,
  INPUT_BRAKE,
  INPUT_FLIPLEFT,
  INPUT_FLIPRIGHT,
  INPUT_CHANGEDIR,

  INPUT_NB_PLAYERKEYS
};

/*===========================================================================
Script hooks
===========================================================================*/
struct InputScriptKeyHook {
  XMKey nKey; /* Hooked key */
  std::string FuncName; /* Script function to invoke */
  Scene *pGame; /* Pointer to game */
};

/*===========================================================================
Controls
===========================================================================*/
struct Controls {
  IFullKey playerKeys[INPUT_NB_PLAYERKEYS];
  IFullKey scriptActionKeys[MAX_SCRIPT_KEY_HOOKS];
};

/*===========================================================================
Input handler class
===========================================================================*/
class Input : public Singleton<Input> {
  friend class Singleton<Input>;

private:
  Input();
  ~Input() {}

public:
  /* Methods */
  void reset();

  void loadConfig(UserConfig *pConfig,
                  xmDatabase *pDb,
                  const std::string &i_id_profile);
  void init(UserConfig *pConfig,
            xmDatabase *pDb,
            const std::string &i_id_profile,
            bool i_enableJoysticks);
  void uninit();

  void resetScriptKeyHooks(void) { m_nNumScriptKeyHooks = 0; }
  void addScriptKeyHook(Scene *pGame,
                        const std::string &keyName,
                        const std::string &FuncName);

  int getNumScriptKeyHooks() const;
  InputScriptKeyHook getScriptKeyHooks(int i) const;
  XMKey getScriptActionKeys(int i_player, int i_actionScript) const;

  std::string getKeyByAction(const std::string &Action, bool i_tech = false);

  Joystick *getJoyById(SDL_JoystickID num);
  SDL_JoystickID getJoyNum(const Joystick &joystick) const;

  std::vector<Joystick> &getJoysticks() { return m_joysticks; }

  InputEventType joystickAxisSens(int16_t m_joyAxisValue);
  void recheckJoysticks();
  void loadJoystickMappings();
  bool areJoysticksEnabled() const;
  void enableJoysticks(bool i_value);

  void setDefaultConfig();
  void saveConfig(UserConfig *pConfig,
                  xmDatabase *pDb,
                  const std::string &i_id_profile);

  bool isANotGameSetKey(XMKey *i_xmkey) const;

  void setPlayerKey(unsigned int INPUT_key, int i_player, XMKey i_value);
  const XMKey *getPlayerKey(unsigned int INPUT_key, int i_player) const;
  std::string getPlayerKeyHelp(unsigned int INPUT_key, int i_player) const;

  void setSCRIPTACTION(int i_player, int i_action, XMKey i_value);
  XMKey getSCRIPTACTION(int i_player, int i_action) const;

  void setGlobalKey(unsigned int INPUT_key, XMKey i_value);
  const XMKey *getGlobalKey(unsigned int INPUT_key) const;
  std::string getGlobalKeyHelp(unsigned int INPUT_key) const;
  bool getGlobalKeyCustomizable(unsigned int INPUT_key) const;

  static InputEventType eventState(uint32_t type);

private:
  /* Data */
  int m_nNumScriptKeyHooks;
  InputScriptKeyHook m_ScriptKeyHooks[MAX_SCRIPT_KEY_HOOKS];

  std::vector<Joystick> m_joysticks;

  Controls m_controls[INPUT_NB_PLAYERS];
  IFullKey m_globalControls[INPUT_NB_GLOBALKEYS];

  friend class InputSDL12Compat;
};

#endif
