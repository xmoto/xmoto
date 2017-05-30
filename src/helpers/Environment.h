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

#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include <string>
#include <vector>

class Environment {
public:
  static void init();
  static void uninit();

  static void set_variable(const std::string &i_variable,
                           const std::string &i_value);
  static std::string get_variable(const std::string &i_variable);

private:
  static std::vector<char *> m_chars;
};

#endif
