/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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

#include "FileCompression.h"
#include <bzlib.h>
#include "../VExcept.h"

#define BUFSIZE 4096

void FileCompression::bunzip2(std::string p_fileIN, std::string p_fileOUT) {
  FILE   *f_in, *f_out;
  BZFILE *b_in;
  int     nBuf;
  char    buf[BUFSIZE];
  int     bzerror_in;
  int     nbWrote;

  // open in file
  f_in = fopen (p_fileIN.c_str(), "r");
  if (!f_in) {
    throw vapp::Exception("Unable to read file " + p_fileIN);
  }
  b_in = BZ2_bzReadOpen (&bzerror_in, f_in, 0, 0, NULL, 0);
  if(bzerror_in != BZ_OK) {
    BZ2_bzReadClose (&bzerror_in, b_in);
    throw vapp::Exception("Unable to read file " + p_fileIN);
  }

  //open out file
  f_out = fopen(p_fileOUT.c_str(), "w");
  if (!f_out) {
    fclose(f_out);
    throw vapp::Exception("Unable to write file " + p_fileOUT);
  }

  // read file
  bzerror_in = BZ_OK;
  while(bzerror_in == BZ_OK) {
    nBuf = BZ2_bzRead(&bzerror_in, b_in, buf, BUFSIZE);
    if (bzerror_in == BZ_OK || bzerror_in == BZ_STREAM_END) {
      // write file
      nbWrote = fwrite(buf, 1, nBuf, f_out);
      if(nbWrote != nBuf) {
	BZ2_bzReadClose (&bzerror_in, b_in);
	fclose(f_out);
	throw vapp::Exception("Unable to write file " + p_fileOUT);
      }
    }
  }

  // close in file
  if(bzerror_in != BZ_STREAM_END) {
    BZ2_bzReadClose (&bzerror_in, b_in);
    fclose(f_out);
    throw vapp::Exception("Unable to read file " + p_fileIN);
  } else {
    BZ2_bzReadClose (&bzerror_in, b_in);
  }

  // close write file
  fclose(f_out);
}
