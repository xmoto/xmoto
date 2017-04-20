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

#ifndef __VIRTUALNETLEVELSLIST_H__
#define __VIRTUALNETLEVELSLIST_H__

#include <string>
#include "xmoto/VirtualLevelsList.h"

class NetClient;
class xmDatabase;

class VirtualNetLevelsList : public VirtualLevelsList {
  public:
  VirtualNetLevelsList(NetClient* i_client);
  virtual ~VirtualNetLevelsList();

  void setDb(xmDatabase* pDb); // must be set before used !!
  virtual std::string determinePreviousLevel(const std::string& i_id_level);
  virtual std::string determineNextLevel(const std::string& i_id_level);

  private:
  NetClient* m_client;
  xmDatabase* m_db;
};

#endif
