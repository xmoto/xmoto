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

#include "CmdArgumentParser.h"
#include <sstream>

float CmdArgumentParser::getFloat(std::string &args) {
  std::string token = nextToken(args);
  if (token.size() == 0) {
  }
  return atof(token.c_str());
}

int CmdArgumentParser::getInt(std::string &args) {
  std::string token = nextToken(args);
  if (token.size() == 0) {
  }
  return atoi(token.c_str());
}

std::string CmdArgumentParser::getString(std::string &args) {
  return nextToken(args);
}

void CmdArgumentParser::addFloat(float value, std::string &args) {
  if (args.size() != 0)
    args += " ";

  std::ostringstream oss;
  oss << value;
  args += oss.str();
}

void CmdArgumentParser::addInt(int value, std::string &args) {
  if (args.size() != 0)
    args += " ";

  std::ostringstream oss;
  oss << value;
  args += oss.str();
}

void CmdArgumentParser::addString(std::string value, std::string &args) {
  if (args.size() != 0)
    args += " ";

  args = args + "\"" + value + "\"";
}

std::string CmdArgumentParser::nextToken(std::string &args) {
  unsigned int cur = 0;
  unsigned int size = args.size();
  std::string token = "";

  // skip space
  while (cur < size && args[cur] == ' ') {
    cur++;
  }

  // handle quoted strings
  if (args[cur] == '\"') {
    cur++;
    while (cur < size && args[cur] != '\"') {
      token += args[cur];
      cur++;
    }
  }
  // handle other tokens
  else {
    while (cur < size && args[cur] != ' ') {
      token += args[cur];
      cur++;
    }
  }

  // update args to remove parsed token
  if (cur == size) {
    args = "";
  } else {
    args = args.substr(cur, size - cur);
  }

  return token;
}
