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

#include "Environment.h"
#include "VExcept.h"
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#endif

std::vector<char *> Environment::m_chars;

void Environment::init() {}

void Environment::uninit() {
  for (unsigned int i = 0; i < m_chars.size(); i++) {
    free(m_chars[i]);
  }
}

void Environment::set_variable(const std::string &i_variable,
                               const std::string &i_value) {
  bool v_set = false;

#ifdef WIN32
  /* On Woe32, each process has two copies of the environment variables,
     one managed by the OS and one managed by the C library. We set
     the value in both locations, so that other software that looks in
     one place or the other is guaranteed to see the value. Even if it's
     a bit slow. See also
     <http://article.gmane.org/gmane.comp.gnu.mingw.user/8272>
     <http://article.gmane.org/gmane.comp.gnu.mingw.user/8273>
     <http://www.cygwin.com/ml/cygwin/1999-04/msg00478.html> */
  if (SetEnvironmentVariable(i_variable.c_str(), i_value.c_str()) == 0) {
    throw Exception("Set Env failed");
  }
  v_set = true;
#endif

#if HAVE_SETENV
  if (setenv(i_variable.c_str(), i_value.c_str(), 1) != 0) {
    throw Exception("Set Env failed (setenv)");
  }
  v_set = true;
#elif HAVE_PUTENV
  std::string v_buffer;
  char *v_str;

  v_buffer = i_variable + "=" + i_value.c_str();
  v_str = (char *)malloc(v_buffer.length() + 1);

  if (v_str == NULL) {
    throw Exception("Set Env failed (malloc)");
  }

  strncpy(v_str, v_buffer.c_str(), v_buffer.length() + 1);

  if (putenv(v_str) != 0) {
    free(v_str);
    throw Exception("Set Env failed (putenv)");
  }
  // free(v_str); MUST NOT BE FREED
  m_chars.push_back(v_str);
  v_set = true;

#else
#warning "Missing setenv and putenv"
#endif

  if (v_set == false) {
    throw Exception("Set Env failed (no available set)");
  }
}

std::string Environment::get_variable(const std::string &i_variable) {
#ifdef WIN32
  return std::string("");
#else
  char *v_res = getenv(i_variable.c_str());
  if (v_res == NULL) {
    return std::string("");
  }
  return std::string(v_res);
#endif
}
