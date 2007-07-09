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

#include "Environment.h"
#include "VExcept.h"
#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#endif

void set_environment_variable(const std::string& i_variable, const std::string& i_value) {
#ifdef WIN32
  if(SetEnvironmentVariable(i_variable.c_str(), i_value.c_str()) == 0) {
#else
  if(setenv(i_variable.c_str(), i_value.c_str(), 1) != 0) {
#endif
    throw Exception("Set Env failed");
  }

}
