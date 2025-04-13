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

bool utf8::is_utf8_valid(const std::string &i_str) {
  size_t i = 0;

  try {
    while (i < i_str.size()) {
      const int size = byte_size_from_utf8_first(i_str[i]);
      if (i + size > i_str.size()) {
        throw Exception("Invalid utf-8 char");
      }
      i += size;
    }
  } catch (Exception &e) {
    return false;
  }

  return true;
}

std::vector<std::string> utf8::split_utf8_string(const std::string &src) {
  std::vector<std::string> ret;
  std::string v_line;

  for (size_t i = 0; i < src.size(); /* nop */) {
    const int size = byte_size_from_utf8_first(src[i]);
    if (i + size > src.size())
      throw Exception("Invalid utf-8 char");

    if (src.substr(i, size) == "\n") {
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

std::string utf8::getNextChar(const std::string &src, unsigned int &io_pos) {
  std::string v_res;
  const int size = byte_size_from_utf8_first(src[io_pos]);
  if (io_pos + size > src.size()) {
    // printf("%i + %i > %i (%s)\n", io_pos, size, src.size(), src.c_str());
    throw Exception("Invalid utf-8 char");
  }
  v_res = src.substr(io_pos, size);
  io_pos += size;

  return v_res;
}

void utf8::getNextChar(const std::string &src,
                       unsigned int &io_pos,
                       std::string &o_char) {
  const int size = byte_size_from_utf8_first(src[io_pos]);
  if (io_pos + size > src.size()) {
    throw Exception("Invalid utf-8 char");
  }
  o_char = src.substr(io_pos, size);
  io_pos += size;
}

std::string utf8::txt2vertical(const std::string &i_str) {
  std::string v_res;

  for (size_t i = 0; i < i_str.size(); /* nop */) {
    const int size = byte_size_from_utf8_first(i_str[i]);
    if (i + size > i_str.size()) {
      throw Exception("Invalid utf-8 char");
    }
    v_res += i_str.substr(i, size) + "\n";
    i += size;
  }
  return v_res;
}

std::string utf8::utf8_concat(const std::string &i_a, const std::string &i_b) {
  return i_a + i_b;
}

std::string utf8::utf8_insert(const std::string &i_a,
                              const std::string &i_b,
                              unsigned int i_numChar) {
  unsigned int n = 0; // utf-8 size for i_numChar
  std::string v_res = i_a;

  for (unsigned int i = 0; i < i_numChar; i++) {
    getNextChar(i_a, n);
  }
  for (unsigned int i = 0; i < i_b.size(); i++) {
    v_res.insert(v_res.begin() + n + i, i_b[i]);
  }

  return v_res;
}

std::string utf8::utf8_delete(const std::string &input, int32_t at) {
  int32_t pos = 0;
  int32_t cursor = 0;
  std::string result;
  std::string current;

  while (input.length() > pos) {
    current = getNextChar(input, (unsigned int &)pos);
    cursor++;

    if (cursor != at)
      result += current;
  }

  return result;
}

unsigned int utf8::utf8_length(const std::string &i_a) {
  unsigned int n = 0;
  unsigned int v_res = 0;

  while (i_a.length() > n) {
    getNextChar(i_a, n);
    v_res++;
  }

  return v_res;
}

std::string utf8::utf8_substring(const std::string &i_a,
                                 size_t i_numChar,
                                 size_t i_nbChars) {
  std::string v_res;
  unsigned int n_begin = 0;
  unsigned int n_end;

  for (unsigned int i = 0; i < i_numChar; i++) {
    if (i_a.length() > n_begin) {
      getNextChar(i_a, n_begin);
    } else {
      throw Exception("Invalid utf-8 char splitting");
    }
  }

  if (i_nbChars == std::string::npos) {
    return i_a.substr(n_begin, i_nbChars);
  }

  n_end = n_begin;
  for (unsigned int i = 0; i < i_nbChars; i++) {
    if (i_a.length() > n_end) {
      getNextChar(i_a, n_end);
    } else {
      throw Exception("Invalid utf-8 char splitting");
    }
  }

  return i_a.substr(n_begin, n_end - n_begin);
}

std::string utf8::utf8_substring_abs(const std::string &str,
                                     size_t start,
                                     size_t end) {
  return utf8::utf8_substring(
    str, start, (end == std::string::npos) ? end : (end - start));
}

void utf8::utf8_split(const std::string &i_line,
                      const std::string &i_char,
                      std::vector<std::string> &o_split) {
  unsigned int n = 0;
  unsigned int n_previous = 0;
  unsigned int v_previous_split = 0;
  std::string v_char;
  std::string v_arg;

  while (i_line.length() > n) {
    n_previous = n;
    v_char = getNextChar(i_line, n);
    if (v_char == i_char) {
      v_arg = i_line.substr(v_previous_split, n_previous - v_previous_split);
      if (v_arg != "") {
        o_split.push_back(v_arg);
      }
      v_previous_split = n;
    }
  }

  v_arg = i_line.substr(v_previous_split, i_line.length() - v_previous_split);
  if (v_arg != "") {
    o_split.push_back(v_arg);
  }
}
