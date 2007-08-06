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

class XMSession {
  public:
  XMSession();
  void load(const XMArguments* i_xmargs);

  bool isVerbose() const;

  void setUseGraphics(bool i_value);
  bool useGraphics() const;

  private:
  bool m_verbose;
  bool m_useGraphics;
};

#endif
