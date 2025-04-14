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

/*
 *  I uses my old tinyimage C library for handling image files (.png and
 *  .jpg). One should really rewrite it in C++ so it could fit nicely with
 *  everything else. This file is essential stupid glue code.
 */

#include "Image.h"
#include "VFileIO.h"
#include "VFileIO_types.h"
#include "helpers/Log.h"
#include "helpers/VExcept.h"
#include "image/tim.h"

#include <cassert>

/*============================================================================
  I/O driver: Callbacks
  ============================================================================*/
void *_image_io_open(const char *pcWhere, tim_io_mode_t IOMode) {
  /* Determine mode of I/O, and open */
  try {
    switch (IOMode) {
      case TIM_IM_READ:
        return (void *)XMFS::openIFile(FDT_DATA, pcWhere);
      case TIM_IM_WRITE:
        return (void *)XMFS::openOFile(FDT_DATA, pcWhere);
    }
  } catch (Exception &e) {
    LogWarning("_image_io_open() - exception when opening: %s\n", pcWhere);
  }

  return NULL; /* some prob */
}

void _image_io_close(void *pvHandle) {
  try {
    XMFS::closeFile((FileHandle *)pvHandle);
  } catch (Exception &e) {
    LogWarning("_image_io_close() - exception when closing: %s\n",
               ((FileHandle *)pvHandle)->Name.c_str());
  }
}

int _image_io_seek(void *pvHandle, int nOffset, tim_seek_mode_t SeekMode) {
  try {
    switch (SeekMode) {
      case TIM_SM_ABS:
        XMFS::setOffset((FileHandle *)pvHandle, nOffset);
        break; /* Absolute seeking */
      case TIM_SM_REL:
        XMFS::setOffset((FileHandle *)pvHandle,
                        XMFS::getOffset((FileHandle *)pvHandle) + nOffset);
        break; /* Seek relatively to current pos */
      case TIM_SM_END:
        XMFS::setOffset((FileHandle *)pvHandle,
                        XMFS::getLength((FileHandle *)pvHandle) + nOffset);
        break; /* Seek relatively to end */
      default:
        return TIM_RV_ERR_INVALID_PARAM;
    }
  } catch (Exception &e) {
    LogWarning("_image_io_seek() - exception when seeking: %s\n",
               ((FileHandle *)pvHandle)->Name.c_str());
    return TIM_RV_ERR_COULD_NOT_PERFORM;
  }

  /* Return current position */
  return XMFS::getLength((FileHandle *)pvHandle);
}

int _image_io_read(void *pvHandle, void *pvBuf, int nSize) {
  int nRet = 0;
  try {
    int nBytesLeft = XMFS::getLength((FileHandle *)pvHandle) -
                     XMFS::getOffset((FileHandle *)pvHandle);
    int nBytesToRead = nBytesLeft < nSize ? nBytesLeft : nSize;
    if (XMFS::readBuf((FileHandle *)pvHandle, (char *)pvBuf, nBytesToRead)) {
      nRet = nBytesToRead;
    }
  } catch (Exception &e) {
    LogWarning("_image_io_read() - exception when reading from: %s\n",
               ((FileHandle *)pvHandle)->Name.c_str());
  }

  return nRet;
}

int _image_io_write(void *pvHandle, void *pvBuf, int nSize) {
  int nRet = 0;
  try {
    if (XMFS::writeBuf((FileHandle *)pvHandle, (char *)pvBuf, nSize)) {
      nRet = nSize;
    }
  } catch (Exception &e) {
    LogWarning("_image_io_read() - exception when writing to: %s\n",
               ((FileHandle *)pvHandle)->Name.c_str());
  }

  return nRet;
}

int _image_io_eof(void *pvHandle) {
  try {
    if (XMFS::isEnd((FileHandle *)pvHandle))
      return true;
  } catch (Exception &e) {
    LogWarning(
      "_image_io_eof() - exception thrown by XMFS::isEnd() when checking: %s\n",
      ((FileHandle *)pvHandle)->Name.c_str());
  }
  return false;
}

/*==============================================================================
  I/O driver: Globals
  ==============================================================================*/

tim_io_t g_ImageIODrv = { _image_io_open, _image_io_close, _image_io_seek,
                          _image_io_read, _image_io_write, _image_io_eof };

/*=============================================================================
  Methods: main
  =============================================================================*/

void Img::createEmpty(unsigned int nWidth, unsigned int nHeight) {
  /* Check arguments */
  assert(nWidth > 0 && nHeight > 0);

  /* For free old image, if any */
  freeMemory();

  /* Allocate memory for image */
  m_pPixels = new Color[nWidth * nHeight];

  /* Set various members */
  m_nWidth = nWidth;
  m_nHeight = nHeight;
  m_bAlpha = false;

  /* Clear to black, no alpha */
  clearRect(0, 0, nWidth, nHeight, MAKE_COLOR(0, 0, 0, 255));
}

void Img::freeMemory(void) {
  /* If memory is non-null, free it */
  if (m_pPixels != NULL) {
    delete[] m_pPixels;
    m_pPixels = NULL;
  }
}

void Img::loadFile(const std::string &FileName, bool bThumbnail) {
  /* Free old image */
  freeMemory();

#ifdef DO_NOT_LOAD_TEXTURES
  /* Don't load anything, simply create bogus instead */
  createEmpty(16, 16);
  for (unsigned int i = 0; i < 16 * 16; i++)
    m_pPixels[i] = MAKE_COLOR((rand() * 255) / RAND_MAX,
                              (rand() * 255) / RAND_MAX,
                              (rand() * 255) / RAND_MAX,
                              255);
  return;
#endif

  /* Initialize image library */
  tim_session_t ts;
  if (tim_init_session(&ts, &g_Memory_CRT, &g_ImageIODrv) < 0)
    throw Exception("Img::loadFile: tim_init_session() failed!");
  tim_add_jpeg_support(&ts);
  tim_add_png_support(&ts);

  /* Thumbnail? */
  if (bThumbnail)
    tim_set_hint_int(&ts, ".jpg", "jpeg_downscale", 8);

  /* Load image from file */
  tim_image_t *pImage;
  if (tim_read_image(&ts, (char *)FileName.c_str(), &pImage) < 0)
    throw Exception("Img::loadFile: tim_read_image() failed: " + FileName);
  createEmpty(pImage->Info.nWidth, pImage->Info.nHeight);

  /* Either RGB or RGBA */
  for (unsigned int i = 0; i < m_nWidth * m_nHeight; i++) {
    if (pImage->Info.PixelType == TIM_PT_RGB24)
      m_pPixels[i] = MAKE_COLOR(
        pImage->pRGB[i].r, pImage->pRGB[i].g, pImage->pRGB[i].b, 255);
    else if (pImage->Info.PixelType == TIM_PT_RGBA32)
      m_pPixels[i] = MAKE_COLOR(pImage->pRGBA[i].r,
                                pImage->pRGBA[i].g,
                                pImage->pRGBA[i].b,
                                pImage->pRGBA[i].a);
  }

  if (pImage->Info.PixelType == TIM_PT_RGB24)
    m_bAlpha = false;
  else
    checkAlpha();

  /* Free image again */
  tim_destroy_image(pImage);

  // printf("Loaded image '%s'\n",FileName.c_str());

  /* Uninitialize image library */
  tim_free_session(&ts);
}

void Img::saveFile(const std::string &FileName) {
  /* Anything to save? */
  if (m_pPixels == NULL)
    throw Exception("Img::saveFile: Tried to save NULL image to " + FileName);

  /* Initialize image library */
  tim_session_t ts;
  if (tim_init_session(&ts, &g_Memory_CRT, &g_ImageIODrv) < 0)
    throw Exception("Img::saveFile: tim_init_session() failed!");
  tim_add_jpeg_support(&ts);
  tim_add_png_support(&ts);

  /* Create image */
  tim_image_t *pImage;
  if (m_bAlpha)
    pImage = tim_create_image(&ts, m_nWidth, m_nHeight, TIM_PT_RGBA32);
  else
    pImage = tim_create_image(&ts, m_nWidth, m_nHeight, TIM_PT_RGB24);

  if (pImage == NULL)
    throw Exception("Img::saveFile: tim_create_image() failed!");

  /* Copy content to new image */
  for (unsigned int i = 0; i < m_nWidth * m_nHeight; i++) {
    if (m_bAlpha) {
      pImage->pRGBA[i].r = GET_RED(m_pPixels[i]);
      pImage->pRGBA[i].g = GET_GREEN(m_pPixels[i]);
      pImage->pRGBA[i].b = GET_BLUE(m_pPixels[i]);
      pImage->pRGBA[i].a = GET_ALPHA(m_pPixels[i]);
    } else {
      pImage->pRGB[i].r = GET_RED(m_pPixels[i]);
      pImage->pRGB[i].g = GET_GREEN(m_pPixels[i]);
      pImage->pRGB[i].b = GET_BLUE(m_pPixels[i]);
    }
  }

  /* Save it */
  if (tim_write_image(pImage, (char *)FileName.c_str()) < 0)
    throw Exception("Img::saveFile: tim_write_image() failed: " + FileName);

  /* Free image again */
  tim_destroy_image(pImage);

  /* Uninitialize image library */
  tim_free_session(&ts);
}

bool Img::checkFile(const std::string &FileName, image_info_t *pInfo) {
  bool bValid = false;

  /* Initialize image library */
  tim_session_t ts;
  if (tim_init_session(&ts, &g_Memory_CRT, &g_ImageIODrv) < 0)
    throw Exception("Img::saveFile: tim_init_session() failed!");
  tim_add_jpeg_support(&ts);
  tim_add_png_support(&ts);

  /* Retrieve information from file */
  tim_image_info_t tii;

  if (tim_get_image_info(&ts, (char *)FileName.c_str(), &tii) >= 0) {
    bValid = true;

    if (pInfo != NULL) {
      pInfo->nWidth = tii.nWidth;
      pInfo->nHeight = tii.nHeight;
    }
  }

  /* Uninitialize image library */
  tim_free_session(&ts);

  /* OK */
  return bValid;
}

void Img::resample(unsigned int nWidth, unsigned int nHeight) {
  /* Check arguments */
  assert(nWidth > 0 && nHeight > 0);

  /* Anything to modify? */
  if (m_pPixels == NULL)
    throw Exception("Img::resample: This image is NULL!");

  /* Perform the resampling */
  Color *pTempPixels = new Color[nWidth * nHeight];
  _Resample(m_pPixels, m_nWidth, m_nHeight, pTempPixels, nWidth, nHeight);
  delete[] m_pPixels;

  m_pPixels = pTempPixels;
  m_nWidth = nWidth;
  m_nHeight = nHeight;

  checkAlpha();
}

/*=============================================================================
  Methods: alpha stuff
  =============================================================================*/

void Img::forceAlpha(int nAlpha) {
  /* Anything to modify? */
  if (m_pPixels == NULL)
    throw Exception("Img::forceAlpha: This image is NULL!");

  /* Enforce the given alpha level, all over the image */
  for (unsigned int i = 0; i < m_nWidth * m_nHeight; i++) {
    m_pPixels[i] = MAKE_COLOR(GET_RED(m_pPixels[i]),
                              GET_GREEN(m_pPixels[i]),
                              GET_BLUE(m_pPixels[i]),
                              nAlpha);
  }

  /* Non-255 value? */
  if (nAlpha != 255)
    m_bAlpha = true;
  else
    m_bAlpha = false;
}

void Img::stripAlpha(Color Background) {
  /* Anything to modify? */
  if (m_pPixels == NULL)
    throw Exception("Img::stripAlpha: This image is NULL!");

  if (m_bAlpha) {
    /* Set background color and remove alpha channel */
    for (unsigned int i = 0; i < m_nWidth * m_nHeight; i++) {
      int a = GET_ALPHA(m_pPixels[i]);

      m_pPixels[i] = MAKE_COLOR((GET_RED(m_pPixels[i]) * a) / 255 +
                                  (GET_RED(Background) * (255 - a)) / 255,
                                (GET_GREEN(m_pPixels[i]) * a) / 255 +
                                  (GET_GREEN(Background) * (255 - a)) / 255,
                                (GET_BLUE(m_pPixels[i]) * a) / 255 +
                                  (GET_BLUE(Background) * (255 - a)) / 255,
                                255);
    }

    /* No more alpha */
    m_bAlpha = false;
  }
}

void Img::mergeAlpha(char *pcAlphaMap) {
  /* Anything to modify? */
  if (m_pPixels == NULL)
    throw Exception("Img::mergeAlpha: This image is NULL!");

  /* Apply the given alpha map (dimensions MUST match) */
  for (unsigned int i = 0; i < m_nWidth * m_nHeight; i++) {
    m_pPixels[i] = MAKE_COLOR(GET_RED(m_pPixels[i]),
                              GET_GREEN(m_pPixels[i]),
                              GET_BLUE(m_pPixels[i]),
                              pcAlphaMap[i]);
  }

  /* Check out alphas */
  checkAlpha();
}

/*=============================================================================
  Methods: temporary conversions
  =============================================================================*/

unsigned char *Img::convertToRGB24(void) {
  /* Anything to modify */
  if (m_pPixels == NULL)
    throw Exception("Img::convertToRGB24: This image is NULL!");

  /* Convert to 24-bit RGB format */
  unsigned char *pc = new unsigned char[m_nWidth * m_nHeight * 3];
  for (unsigned int i = 0; i < m_nWidth * m_nHeight; i++) {
    pc[i * 3] = GET_RED(m_pPixels[i]);
    pc[i * 3 + 1] = GET_GREEN(m_pPixels[i]);
    pc[i * 3 + 2] = GET_BLUE(m_pPixels[i]);
  }
  return pc;
}

unsigned char *Img::convertToRGBA32(void) {
  /* Anything to modify */
  if (m_pPixels == NULL)
    throw Exception("Img::convertToRGBA32: This image is NULL!");

  /* Convert to 32-bit RGBA format */
  unsigned char *pc = new unsigned char[m_nWidth * m_nHeight * 4];
  for (unsigned int i = 0; i < m_nWidth * m_nHeight; i++) {
    pc[i * 4] = GET_RED(m_pPixels[i]);
    pc[i * 4 + 1] = GET_GREEN(m_pPixels[i]);
    pc[i * 4 + 2] = GET_BLUE(m_pPixels[i]);
    pc[i * 4 + 3] = GET_ALPHA(m_pPixels[i]);
  }
  return pc;
}

unsigned char *Img::convertToGray(void) {
  /* Anything to modify */
  if (m_pPixels == NULL)
    throw Exception("Img::convertToGray: This image is NULL!");

  /* Convert to 8-bit grayscale format */
  unsigned char *pc = new unsigned char[m_nWidth * m_nHeight];
  for (unsigned int i = 0; i < m_nWidth * m_nHeight; i++) {
    pc[i] = (GET_RED(m_pPixels[i]) + GET_GREEN(m_pPixels[i]) +
             GET_BLUE(m_pPixels[i]) + GET_ALPHA(m_pPixels[i])) /
            4;
  }
  return pc;
}

unsigned char *Img::convertToAlphaMap(void) {
  /* Anything to modify */
  if (m_pPixels == NULL)
    throw Exception("Img::convertToAlphaMap: This image is NULL!");

  /* Convert to 8-bit grayscale format */
  unsigned char *pc = new unsigned char[m_nWidth * m_nHeight];
  for (unsigned int i = 0; i < m_nWidth * m_nHeight; i++) {
    pc[i] = GET_ALPHA(m_pPixels[i]);
  }
  return pc;
}

/*=============================================================================
  Methods: simple manipulation
  =============================================================================*/

void Img::clearRect(unsigned int x1,
                    unsigned int y1,
                    unsigned int x2,
                    unsigned int y2,
                    Color Color) {
  /* Anything to draw at? */
  if (m_pPixels == NULL)
    throw Exception("Img::clearRect: Tried to draw on NULL image!");

  /* Check parameters */
  assert(x1 < x2);
  assert(y1 < y2);

  /* Set color in rect */
  for (unsigned int y = y1; y < y2; y++) {
    for (unsigned int nOffset = y * m_nWidth + x1; nOffset < y * m_nWidth + x2;
         nOffset++) {
      m_pPixels[nOffset] = Color;
    }
  }
}

void Img::insertImage(unsigned int x, unsigned int y, Img *pInsert) {
  /* Anything to draw at? */
  if (m_pPixels == NULL)
    throw Exception("Img::insertImage: Tried to draw on NULL image!");

  /* Anything to draw from? */
  if (pInsert->getPixels() == NULL)
    throw Exception("Img::insertImage: Tried to read from NULL image!");

  /* Insert the given image at the given position */
  for (unsigned int i = 0; i < pInsert->getHeight(); i++) {
    unsigned int yy = y + i;
    if (yy < m_nHeight) {
      for (unsigned int j = 0; j < pInsert->getWidth(); j++) {
        unsigned int xx = x + j;
        if (xx < m_nWidth) {
          /* Copy pixel */
          m_pPixels[xx + yy * m_nWidth] =
            pInsert->getPixels()[j + i * pInsert->getWidth()];
        }
      }
    }
  }
}

/*=============================================================================
  Various helpers
  =============================================================================*/
void Img::checkAlpha(void) {
  /* Anything to check? */
  if (m_pPixels == NULL)
    throw Exception(
      "Img::checkAlpha: Tried to check NULL image for alpha component!");

  /* Non-255 alpha values in images? */
  for (unsigned int i = 0; i < m_nWidth * m_nHeight; i++) {
    if (GET_ALPHA(m_pPixels[i]) != 255) {
      m_bAlpha = true;
      return;
    }
  }

  /* Nope */
  m_bAlpha = false;
}

Color Img::_Linterp_scanline(Color *pScan,
                             unsigned int nSrcLen,
                             unsigned int nDestLen,
                             int s) {
  int t = ((nSrcLen - 1) * s) / nDestLen;
  int v = ((nSrcLen - 1) * s) % nDestLen;
  int r1, g1, b1, a1;
  int r2, g2, b2, a2;
  r1 = GET_RED(pScan[t]), g1 = GET_GREEN(pScan[t]), b1 = GET_BLUE(pScan[t]),
  a1 = GET_ALPHA(pScan[t]);
  r2 = GET_RED(pScan[t + 1]), g2 = GET_GREEN(pScan[t + 1]),
  b2 = GET_BLUE(pScan[t + 1]), a2 = GET_ALPHA(pScan[t + 1]);
  return MAKE_COLOR((r1 * (nDestLen - v)) / nDestLen + (r2 * v) / nDestLen,
                    (g1 * (nDestLen - v)) / nDestLen + (g2 * v) / nDestLen,
                    (b1 * (nDestLen - v)) / nDestLen + (b2 * v) / nDestLen,
                    (a1 * (nDestLen - v)) / nDestLen + (a2 * v) / nDestLen);
}

Color Img::_Aa_avg_scanline(Color *pScan, unsigned int x1, unsigned int x2) {
  int r = 0, g = 0, b = 0, a = 0;
  int d = 0;
  for (unsigned int i = x1; i < x2; i++) {
    r += GET_RED(pScan[i]);
    g += GET_GREEN(pScan[i]);
    b += GET_BLUE(pScan[i]);
    a += GET_ALPHA(pScan[i]);
    d++;
  }
  if (d == 0)
    return pScan[x1];
  else
    return MAKE_COLOR(r / d, g / d, b / d, a / d);
}

Color Img::_Resample_scanline(Color *pScan,
                              unsigned int nScanLen,
                              unsigned int nDestLen,
                              int nx) {
  /* Scanline... upsample, downsample, or straight copy? */
  if (nScanLen < nDestLen)
    return _Linterp_scanline(pScan, nScanLen, nDestLen, nx);
  else if (nScanLen > nDestLen) {
    unsigned int w1, w2;
    w1 = (nx * nScanLen) / nDestLen;
    w2 = ((nx + 1) * nScanLen) / nDestLen;
    return _Aa_avg_scanline(pScan, w1, w2);
  }
  return pScan[nx];
}

void Img::_Resample(Color *pSrc,
                    unsigned int nSrcWidth,
                    unsigned int nSrcHeight,
                    Color *pDest,
                    unsigned int nDestWidth,
                    unsigned int nDestHeight) {
  unsigned int nx, ny, y, y1, y2;
  int r1, g1, b1, a1;
  int r2, g2, b2, a2;
  int r, g, b, a, d;
  Color T1, T2, T;

  for (ny = 0; ny < nDestHeight; ny++) {
    if (nSrcHeight < nDestHeight) {
      /* Upsample y-axis */
      int t = ((nSrcHeight - 1) * ny) / nDestHeight;
      int v = ((nSrcHeight - 1) * ny) % nDestHeight;

      for (nx = 0; nx < nDestWidth; nx++) {
        T1 =
          _Resample_scanline(&pSrc[t * nSrcWidth], nSrcWidth, nDestWidth, nx);
        T2 = _Resample_scanline(
          &pSrc[(t + 1) * nSrcWidth], nSrcWidth, nDestWidth, nx);

        r1 = GET_RED(T1), g1 = GET_GREEN(T1), b1 = GET_BLUE(T1),
        a1 = GET_ALPHA(T1);
        r2 = GET_RED(T2), g2 = GET_GREEN(T2), b2 = GET_BLUE(T2),
        a2 = GET_ALPHA(T2);
        pDest[nDestWidth * ny + nx] = MAKE_COLOR(
          (r1 * (nDestHeight - v)) / nDestHeight + (r2 * v) / nDestHeight,
          (g1 * (nDestHeight - v)) / nDestHeight + (g2 * v) / nDestHeight,
          (b1 * (nDestHeight - v)) / nDestHeight + (b2 * v) / nDestHeight,
          (a1 * (nDestHeight - v)) / nDestHeight + (a2 * v) / nDestHeight);
      }
    } else if (nSrcHeight > nDestHeight) {
      /* Downsample y-axis */
      y1 = (ny * nSrcHeight) / nDestHeight;
      y2 = ((ny + 1) * nSrcHeight) / nDestHeight;
      for (nx = 0; nx < nDestWidth; nx++) {
        r = g = b = a = 0;
        d = 0;
        for (y = y1; y < y2; y++) {
          T =
            _Resample_scanline(&pSrc[y * nSrcWidth], nSrcWidth, nDestWidth, nx);
          r += GET_RED(T), g += GET_GREEN(T), b += GET_BLUE(T),
            a += GET_ALPHA(T);
          d++;
        }
        if (d != 0)
          pDest[nDestWidth * ny + nx] = MAKE_COLOR(r / d, g / d, b / d, a / d);
        else
          pDest[nDestWidth * ny + nx] =
            pSrc[y1 * nSrcWidth + (nx * nSrcWidth) / nDestWidth];
      }
    } else {
      /* Copy y-axis */
      y = (ny * nSrcHeight) / nDestHeight;
      for (nx = 0; nx < nDestWidth; nx++)
        pDest[nx + ny * nDestWidth] =
          _Resample_scanline(&pSrc[y * nSrcWidth], nSrcWidth, nDestWidth, nx);
    }
  }
}
