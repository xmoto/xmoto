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

#include "Text.h"

std::string txtToLower(const std::string& i_str) {
    std::string v_res;

    v_res = i_str;
  
    for(unsigned int j=0;j<v_res.length();j++) {
      v_res[j] = tolower(v_res[j]);
    }

    return v_res;
}

std::string splitText(const std::string &str, int p_breakLineLength) {
  std::string v_res = "";
  char c[2] = {' ', '\0'};    
  int lineLength = 0;
  
  for(unsigned int i=0; i<str.length(); i++) {
    if((lineLength > p_breakLineLength && str[i] == ' ') ||
       str[i] == '\n') {
      c[0] = '\n';
      v_res.append(c);
      lineLength = 0;
    } else {
      c[0] = str[i];
      v_res.append(c);
      lineLength++;
    }
  }
  return v_res;
}

std::string formatTime(int i_time) {
  char cBuf[256];
  int nM, nS, nH;

  nM = i_time/6000;
  nS = (i_time - nM*6000)/100;
  nH = i_time - nM*6000 - nS*100;
    
  snprintf(cBuf, 256, "%02d:%02d:%02d", nM, nS, nH);
  return cBuf;
}
