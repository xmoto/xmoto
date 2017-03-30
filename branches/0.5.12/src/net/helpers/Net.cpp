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

#include "Net.h"
#include <sstream>

std::string XMNet::getIp(IPaddress* i_ip) {
  Uint32 val = SDLNet_Read32(&i_ip->host);
  char str[3+1+3+1+3+1+3+1];
  
  snprintf(str, 3+1+3+1+3+1+3+1,"%i.%i.%i.%i",
	   val >> 24, (val >> 16) %256, (val >> 8) %256, val%256);

  return std::string(str);
}

std::string XMNet::getFancyBytes(unsigned int i_bytes) {
  std::ostringstream v_s;
  unsigned int v_n;

  v_n = i_bytes;
  if(v_n < 1024) {
    v_s << v_n << " bytes";
    return v_s.str();
  }

  v_n = v_n / 1024;
  if(v_n < 1024) {
    v_s << v_n << " kB";
    return v_s.str();
  }

  v_n = v_n / 1024;
  v_s << v_n << " mB";
  return v_s.str();
}
