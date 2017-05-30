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

#ifndef __XMDEMO_H__
#define __XMDEMO_H__

#include <string>

class ProxySettings;

class XMDemo {
public:
  XMDemo(const std::string i_demoFile);
  ~XMDemo();

  void getLevel(ProxySettings *i_proxy);
  void getReplay(ProxySettings *i_proxy);
  void destroyFiles();

  std::string levelFile() const;
  std::string replayFile() const;

private:
  std::string m_levelUrl;
  std::string m_replayUrl;
  std::string m_levelFile;
  std::string m_replayFile;
};

#endif
