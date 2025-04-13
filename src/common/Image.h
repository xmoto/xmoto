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

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "VCommon.h"
#include "helpers/Color.h"
#include <string>

/*===========================================================================
  Image class definition
  ===========================================================================*/
typedef struct _image_info_t {
  int nWidth, nHeight;
} image_info_t;

class Img {
public:
  Img() {
    m_nWidth = m_nHeight = 0;
    m_pPixels = NULL;
    m_bAlpha = false;
  }
  virtual ~Img() { freeMemory(); }

  /* Methods: main */
  void createEmpty(unsigned int nWidth, unsigned int nHeight);
  void freeMemory(void);
  void loadFile(const std::string &FileName, bool bThumbnail = false);
  void saveFile(const std::string &FileName);
  bool checkFile(const std::string &FileName, image_info_t *pInfo);
  void resample(unsigned int nWidth, unsigned int nHeight);

  /* Methods: alpha stuff */
  void forceAlpha(int nAlpha);
  void stripAlpha(Color Background);
  void mergeAlpha(char *pcAlphaMap);

  /* Methods: temporary conversions */
  unsigned char *convertToRGB24(void);
  unsigned char *convertToRGBA32(void);
  unsigned char *convertToGray(void);
  unsigned char *convertToAlphaMap(void);

  /* Simple drawing/composition */
  void clearRect(unsigned int x1,
                 unsigned int y1,
                 unsigned int x2,
                 unsigned int y2,
                 Color _Color);
  void insertImage(unsigned int x, unsigned int y, Img *pInsert);

  /* Data interface */
  Color *getPixels(void) { return m_pPixels; }
  unsigned int getWidth(void) { return m_nWidth; }
  unsigned int getHeight(void) { return m_nHeight; }
  bool isAlpha(void) { return m_bAlpha; }
  void setAlpha(bool b) { m_bAlpha = b; }

private:
  unsigned int m_nWidth, m_nHeight; /* Size of image */
  Color *m_pPixels; /* RGBA pixel data */
  bool m_bAlpha; /* true: pixel data contains non-255
                    alpha values */

  /* Helper methods */
  void checkAlpha(void); /* Look through pixels for non-255
                            alpha values, and set m_bAlpha
                            accordingly */

  /* Helper methods for image resampling, stolen from one of my earlier
     C projects :) */
  Color _Linterp_scanline(Color *pScan,
                          unsigned int nSrcLen,
                          unsigned int nDestLen,
                          int s);
  Color _Aa_avg_scanline(Color *pScan, unsigned int x1, unsigned int x2);
  Color _Resample_scanline(Color *pScan,
                           unsigned int nScanLen,
                           unsigned int nDestLen,
                           int nx);
  void _Resample(Color *pSrc,
                 unsigned int nSrcWidth,
                 unsigned int nSrcHeight,
                 Color *pDest,
                 unsigned int nDestWidth,
                 unsigned int nDestHeight);
};

#endif
