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

#ifndef __CMDARGUMENTPARSER_H__
#define __CMDARGUMENTPARSER_H__

#include "../helpers/Singleton.h"
#include <string>

class CmdArgumentParser : public Singleton<CmdArgumentParser> {
  friend class Singleton<CmdArgumentParser>;
private:
  CmdArgumentParser() {}
  ~CmdArgumentParser() {}

public:
  // update args to remove the argument requested
  // raise an exception if it can't parse the args to get the requested type
  float       getFloat(std::string& args);
  int         getInt(std::string& args);
  std::string getString(std::string& args);

  // add the arg to args
  void addFloat(float value, std::string& args);
  void addInt(int value, std::string& args);
  void addString(std::string value, std::string& args);

private:
  // update args
  std::string nextToken(std::string& args);
};

#endif
