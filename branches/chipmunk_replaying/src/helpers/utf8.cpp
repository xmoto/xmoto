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

#include "utf8.h"
#include "VExcept.h"

int utf8::byte_size_from_utf8_first(unsigned char ch) {
  int count;
  if ((ch & 0x80) == 0)
  count = 1;
  else if ((ch & 0xE0) == 0xC0)
  count = 2;
  else if ((ch & 0xF0) == 0xE0)
  count = 3;
  else if ((ch & 0xF8) == 0xF0)
  count = 4;
  else if ((ch & 0xFC) == 0xF8)
  count = 5;
  else if ((ch & 0xFE) == 0xFC)
  count = 6;
  else {
    throw Exception("Invalid utf-8 char"); /* stop on invalid characters */
  }
  return count;
}

std::vector<std::string> utf8::split_utf8_string(const std::string &src) {
  std::vector<std::string> ret;
  std::string v_line;

  for(size_t i = 0; i < src.size(); /* nop */) {
    const int size = byte_size_from_utf8_first(src[i]);
    if(i + size > src.size())
      throw Exception("Invalid utf-8 char");
    
    if(src.substr(i, size) == "\n") {
      ret.push_back(v_line);
      v_line = "";
    } else {
      v_line += src.substr(i, size);
    }
    i += size;
  }
  ret.push_back(v_line);
  return ret;
}

std::string utf8::getNextChar(const std::string& src, unsigned int& io_pos) {
  std::string v_res;
  const int size = byte_size_from_utf8_first(src[io_pos]);
  if(io_pos + size > src.size())
    throw Exception("Invalid utf-8 char");
  v_res = src.substr(io_pos, size);
  io_pos += size;

  return v_res;
}

void utf8::getNextChar(const std::string &src, unsigned int& io_pos, std::string& o_char)
{
 const int size = byte_size_from_utf8_first(src[io_pos]);
  if(io_pos + size > src.size())
    throw Exception("Invalid utf-8 char");
  o_char = src.substr(io_pos, size);
  io_pos += size;
}

std::string utf8::txt2vertical(const std::string& i_str) {
  std::string v_res;

  for(size_t i=0; i<i_str.size(); /* nop */) {
    const int size = byte_size_from_utf8_first(i_str[i]);
    if(i + size > i_str.size()) {
      throw Exception("Invalid utf-8 char");
    }    
    v_res += i_str.substr(i, size) + "\n";
    i += size;
  }
  return v_res;
}
