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

#ifndef __USERCONFIG_H__
#define __USERCONFIG_H__

#define XM_CONFIGFILE "config.dat"

#include "common/VCommon.h"
#include <string>
#include <vector>

/*===========================================================================
User variable
===========================================================================*/
struct UserConfigVar {
  std::string Name;
  std::string Value;
  std::string DefaultValue;
};

/*===========================================================================
User config class
===========================================================================*/
class UserConfig {
public:
  ~UserConfig() { _FreeUserConfig(); }

  /* Methods */
  void loadFile(void);
  void saveFile(void);
  UserConfigVar *createVar(std::string Name, std::string DefaultValue);
  std::string getDefaultValue(std::string Name);
  std::string getValue(std::string Name);
  void setValue(std::string Name, std::string Value);

  /* Public helpers */
  float getFloat(std::string Name);
  std::string getString(std::string Name);
  bool getBool(std::string Name);
  int getInteger(std::string Name);
  void setFloat(std::string Name, float v);
  void setString(std::string Name, std::string v);
  void setBool(std::string Name, bool v);
  void setInteger(std::string Name, int v);

  /* Data interface */
  std::vector<UserConfigVar *> &getVars(void) { return m_Vars; }
  bool isChanged(void) { return m_bChangeFlag; }
  void setChanged(bool b) { m_bChangeFlag = b; }

private:
  /* Data */
  std::vector<UserConfigVar *> m_Vars;
  bool m_bChangeFlag; /* Set when anything is changed */

  /* Helpers */
  void _FreeUserConfig(void);

  UserConfigVar *_FindVarByName(std::string Name);
};

#endif
