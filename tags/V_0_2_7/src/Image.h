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

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "VCommon.h"

namespace vapp {

  /*===========================================================================
  Lazy stuff
  ===========================================================================*/
  #define color_t Color /* TODO: search and replace instead ;) */
  #define _Assert(x) 
  #define String std::string
  #define CError Exception
  
  /*===========================================================================
  Image class definition
  ===========================================================================*/
  typedef struct _image_info_t {
    int nWidth,nHeight;
  } image_info_t;

  class Img {
    public:
      Img() {m_nWidth=m_nHeight=0; m_pPixels=NULL; m_bAlpha=false;}
      virtual ~Img() {freeMemory();}
    
      /* Methods: main */
      void createEmpty(int nWidth,int nHeight);
      void freeMemory(void);
      void loadFile(String FileName,bool bThumbnail=false);
      void saveFile(String FileName);
      bool checkFile(String FileName,image_info_t *pInfo);
      void resample(int nWidth,int nHeight);
      
      /* Methods: alpha stuff */
      void forceAlpha(int nAlpha);
      void stripAlpha(color_t Background);
      void mergeAlpha(char *pcAlphaMap);
     
      /* Methods: temporary conversions */
      unsigned char *convertToRGB24(void);
      unsigned char *convertToRGBA32(void);
      unsigned char *convertToGray(void);
      unsigned char *convertToAlphaMap(void);
      
      /* Simple drawing/composition */
      void clearRect(int x1,int y1,int x2,int y2,color_t _Color);
      void insertImage(int x,int y,Img *pInsert);
      
      /* Data interface */
      color_t *getPixels(void) {return m_pPixels;}
      int getWidth(void) {return m_nWidth;}
      int getHeight(void) {return m_nHeight;}
      bool isAlpha(void) {return m_bAlpha;}
      void setAlpha(bool b) {m_bAlpha=b;}
      
    private:
      int m_nWidth,m_nHeight;               /* Size of image */
      color_t *m_pPixels;                   /* RGBA pixel data */
      bool m_bAlpha;                        /* true: pixel data contains non-255
                                              alpha values */
                                               
      /* Helper methods */
      void checkAlpha(void);                /* Look through pixels for non-255
                                              alpha values, and set m_bAlpha 
                                              accordingly */                                             
                                               
      /* Helper methods for image resampling, stolen from one of my earlier
        C projects :) */
      color_t _Linterp_scanline(color_t *pScan,int nSrcLen,int nDestLen,int s);
      color_t _Aa_avg_scanline(color_t *pScan,int x1,int x2);
      color_t _Resample_scanline(color_t *pScan,int nScanLen,int nDestLen,int nx);
      void _Resample(color_t *pSrc,int nSrcWidth,int nSrcHeight,
                    color_t *pDest,int nDestWidth,int nDestHeight);                                                    
  };

}

#endif
