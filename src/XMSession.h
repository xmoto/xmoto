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

#ifndef __XMCONF_H__
#define __XMCONF_H__

#include <string>

/*
  XMSession   : current session options
  UserConfig  : config file options
  XMArguments : command line options

  first, XMSession constructor init session values,
  then, UserConfig are loaded, and finally, XMArguments overwrite session's values if they are defined
*/

class XMArguments;
class UserConfig;

class XMSession {
  public:
  XMSession();
  void load(const XMArguments* i_xmargs);
  void load(UserConfig* m_Config);
  bool isVerbose() const;

  void setUseGraphics(bool i_value);
  bool useGraphics() const;
  int resolutionWidth() const;
  int resolutionHeight() const;
  int bpp() const;
  int windowed() const;
  bool glExts() const;
  std::string drawlib() const;
  bool www() const;
  void setWWW(bool i_value);

  private:
  bool m_verbose;
  bool m_useGraphics;
  int  m_resolutionWidth;
  int  m_resolutionHeight;
  int  m_bpp;
  bool m_windowed;
  bool m_glExts;
  std::string m_drawlib;
  bool m_www;
};

#endif
