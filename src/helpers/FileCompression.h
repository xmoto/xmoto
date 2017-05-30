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

#ifndef __FILECOMPRESSION_H__
#define __FILECOMPRESSION_H__

#include <string>

class FileCompression {
public:
  static void bunzip2(std::string p_fileIN, std::string p_fileOUT);
  static void bzip2(std::string p_fileIN, std::string p_fileOUT);

  static char *zcompress(const char *i_data, int i_len, int &o_outputLen);
  static void zuncompress(const char *i_compressedData,
                          int i_compressedLen,
                          char *io_outputData,
                          int i_outputDataLen);
};

#endif /* __FILECOMPRESSION_H__ */
