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

#ifndef __UTF8_H__
#define __UTF8_H__

#include <vector>
#include <string>

class utf8 {
  public:
  static int byte_size_from_utf8_first(unsigned char ch);
  static std::vector<std::string> split_utf8_string(const std::string &src);
  static std::string txt2vertical(const std::string& i_str);
  static std::string getNextChar(const std::string &src, unsigned int& io_pos);
  static void getNextChar(const std::string &src, unsigned int& io_pos, std::string& o_char);

  static std::string utf8_concat(const std::string& i_a, const std::string& i_b);
  static std::string utf8_insert(const std::string& i_a, const std::string& i_b, unsigned int i_numChar);
  static std::string utf8_delete(const std::string& i_a, unsigned int i_numChar);
  static unsigned int utf8_length(const std::string& i_a);
  static std::string utf8_substring(const std::string& i_a, unsigned int i_numChar, unsigned int i_nbChars);

  static void utf8_split(const std::string& i_line, const std::string& i_char, std::vector<std::string>& o_split);
};

#endif
