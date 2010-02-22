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

#include "FileCompression.h"
#include <stdlib.h>
#if defined (WIN32)
  #include "../bzip2/bzlib.h"
#else
  #include <bzlib.h>
#endif
#include "VExcept.h"
#if defined(WIN32) 
  #include "zlib.h"
#else
  #include <zlib.h>
#endif

#define BUFSIZE 4096

void FileCompression::bunzip2(std::string p_fileIN, std::string p_fileOUT) {
  FILE   *f_in, *f_out;
  BZFILE *b_in;
  int     nBuf;
  char    buf[BUFSIZE];
  int     bzerror_in;
  int     nbWrote;

  // open in file
  f_in = fopen (p_fileIN.c_str(), "rb");
  if (!f_in) {
    throw Exception("Unable to read file " + p_fileIN);
  }
  b_in = BZ2_bzReadOpen (&bzerror_in, f_in, 0, 0, NULL, 0);
  if(bzerror_in != BZ_OK) {
    BZ2_bzReadClose (&bzerror_in, b_in);
    fclose(f_in);
    throw Exception("Unable to read file " + p_fileIN);
  }
  
  //open out file
  f_out = fopen(p_fileOUT.c_str(), "wb");
  if (!f_out) {
    BZ2_bzReadClose (&bzerror_in, b_in);
    fclose(f_in);
    throw Exception("Unable to write file " + p_fileOUT);
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
	fclose(f_in);
	fclose(f_out);
	throw Exception("Unable to write file " + p_fileOUT);
      }
    }
  }
  
  // close in file
  if(bzerror_in != BZ_STREAM_END) {
    BZ2_bzReadClose (&bzerror_in, b_in);
    fclose(f_in);
    fclose(f_out);
    throw Exception("Unable to read file " + p_fileIN);
  }
   
  BZ2_bzReadClose (&bzerror_in, b_in);
  fclose(f_in);
  
  // close write file
  fclose(f_out);
}

void FileCompression::bzip2(std::string p_fileIN, std::string p_fileOUT) {
  FILE   *f_in, *f_out;
  BZFILE *b_out;
  int     nBuf;
  char    buf[BUFSIZE];
  int     bzerror_out;

  // open in file
  f_in = fopen (p_fileIN.c_str(), "rb");
  if (!f_in) {
    throw Exception("Unable to read file " + p_fileIN);
  }

  //open out file
  f_out = fopen(p_fileOUT.c_str(), "wb");
  if (!f_out) {
    fclose(f_in);
    throw Exception("Unable to write file " + p_fileOUT);
  }

  b_out = BZ2_bzWriteOpen (&bzerror_out, f_out, 9, 0, 0);
  if(bzerror_out != BZ_OK) {
    fclose(f_in);
    fclose(f_out);
    throw Exception("Unable to write file " + p_fileOUT);
  }

  // read file
  do {
    nBuf = fread(buf, 1, BUFSIZE, f_in);
    if(nBuf != 0) {
      BZ2_bzWrite(&bzerror_out, b_out, buf, nBuf);
      if(bzerror_out != BZ_OK) {
	BZ2_bzWriteClose (&bzerror_out, b_out, 0, NULL, NULL);
	fclose(f_in);
	fclose(f_out);
	throw Exception("Unable to write file " + p_fileOUT);
      }
    }
  } while(nBuf != 0);
  if(feof(f_in) == 0) {
    BZ2_bzWriteClose (&bzerror_out, b_out, 0, NULL, NULL);
    fclose(f_in);
    fclose(f_out);
    throw Exception("Unable to write file " + p_fileOUT);
  }

  BZ2_bzWriteClose (&bzerror_out, b_out, 0, NULL, NULL);
  if(bzerror_out != BZ_OK) {
    throw Exception("Unable to write file " + p_fileOUT);
  }

  fclose(f_in);
  fclose(f_out);
}

char* FileCompression::zcompress(const char* i_data, int i_len, int& o_outputLen) {
  char *pcCompressedData;
  uLongf pcCompressedDataSize;
  int nZRet;

  pcCompressedDataSize = compressBound(i_len);
  pcCompressedData = (char*) malloc(pcCompressedDataSize);
  if(pcCompressedData == NULL) {
    throw Exception("Unable to malloc for compression");
  }

  nZRet = compress2((Bytef *)pcCompressedData, &pcCompressedDataSize,
		    (Bytef *)i_data, i_len, 9);

  if(nZRet != Z_OK) {
    free(pcCompressedData);
    throw Exception("Unable to zcompress");
  }
  o_outputLen = pcCompressedDataSize;

  return pcCompressedData;
}

void FileCompression::zuncompress(const char* i_compressedData, int i_compressedLen, char* io_outputData, int i_outputDataLen) {
  int nZRet;
  uLongf v_len;

  v_len = i_outputDataLen;
  nZRet = uncompress((Bytef *)io_outputData, &v_len,
		     (Bytef *)i_compressedData, i_compressedLen);
  if(nZRet != Z_OK || ((int)v_len) != i_outputDataLen) {
    throw Exception("Unable to zuncompress");
  }
}
