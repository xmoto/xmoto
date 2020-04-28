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

/* The CRC generator is derived from the one by Michael Barr, released into
   the public domain. */

#include "CRCHash.h"

#define POLYNOMIAL 0x04C11DB7
#define INITIAL_REMAINDER 0xFFFFFFFF
#define FINAL_XOR_VALUE 0xFFFFFFFF
#define CHECK_VALUE 0xCBF43926

#define WIDTH (8 * sizeof(unsigned int))
#define TOPBIT (1 << (WIDTH - 1))

#define REFLECT_DATA(X) ((unsigned char)_ReflectCRC32((X), 8))

#define REFLECT_REMAINDER(X) ((unsigned int)_ReflectCRC32((X), WIDTH))

/*===========================================================================
Global data
===========================================================================*/
unsigned int CRC32::m_nCRCTable[256];
bool CRC32::m_bCRCInit = false;

/*===========================================================================
Compute 32-bit CRC of data block
===========================================================================*/
unsigned long CRC32::computeCRC32(const unsigned char *pcData, int nDataLen) {
  /* Make sure the CRC computer is initialized */
  _InitCRC32();

  unsigned int nRemainder = INITIAL_REMAINDER;
  unsigned char cData;
  int nByte;

  for (nByte = 0; nByte < nDataLen; nByte++) {
    cData = REFLECT_DATA(pcData[nByte]) ^ (nRemainder >> (WIDTH - 8));
    nRemainder = m_nCRCTable[cData] ^ (nRemainder << 8);
  }

  return (unsigned long)(REFLECT_REMAINDER(nRemainder) ^ FINAL_XOR_VALUE);
}

/*===========================================================================
32-bit CRC helpers
===========================================================================*/
void CRC32::_InitCRC32(void) {
  unsigned int nRemainder;
  int nDividend;
  unsigned char cBit;

  if (m_bCRCInit)
    return;
  m_bCRCInit = true;

  for (nDividend = 0; nDividend < 256; nDividend++) {
    nRemainder = nDividend << (WIDTH - 8);

    for (cBit = 8; cBit > 0; cBit--) {
      if (nRemainder & TOPBIT) {
        nRemainder = (nRemainder << 1) ^ POLYNOMIAL;
      } else {
        nRemainder = (nRemainder << 1);
      }
    }

    m_nCRCTable[nDividend] = nRemainder;
  }
}

unsigned int CRC32::_ReflectCRC32(unsigned int nData, unsigned char cBits) {
  unsigned int nReflection = 0x00000000;

  for (int cBit = 0; cBit < cBits; cBit++) {
    if (nData & 0x01) {
      nReflection |= (1 << ((cBits - 1) - cBit));
    }
    nData >>= 1;
  }

  return nReflection;
}
