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

/* tim_png.cpp
 *
 * libpng wrapper.
 */
#include "tim.h"
#include <png.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>

/*==============================================================================
Types
==============================================================================*/

typedef struct _png_env_t {
  void *pvFile;
  tim_session_t *pSession;
} png_env_t;

/*==============================================================================
I/O callbacks
==============================================================================*/

void user_read_data(png_structp PngPtr, png_bytep Data, png_size_t Length) {
  png_env_t *pEnv;

  /* Get file handle */
  pEnv = (png_env_t *)png_get_io_ptr(PngPtr);

  /* Read */
  tim_read(pEnv->pSession, pEnv->pvFile, (void *)Data, (int)Length);
}

void user_write_data(png_structp PngPtr, png_bytep Data, png_size_t Length) {
  png_env_t *pEnv;

  /* Get file handle */
  pEnv = (png_env_t *)png_get_io_ptr(PngPtr);

  /* Write */
  tim_write(pEnv->pSession, pEnv->pvFile, (void *)Data, (int)Length);
}

void user_flush_data(png_structp PngPtr) {}

/*==============================================================================
Error callbacks
==============================================================================*/

void user_error_fn(png_structp PngPtr, png_const_charp ErrorMsg) {
  /* Just an invisible error-sink */
}

void user_warning_fn(png_structp PngPtr, png_const_charp WarningMsg) {
  /* Just an invisible warning-sink */
}

/*==============================================================================
Load function
==============================================================================*/

int tim_png_load(tim_session_t *pSession,
                 tim_image_t **ppImage,
                 tim_image_info_t *pInfo,
                 const char *pcSource) {
  png_structp PngPtr;
  png_infop InfoPtr;
  png_infop EndInfo;
  int nChannels, nWidth, nHeight, nRowBytes, nBitDepth, nColorType,
    nNumberOfPasses, i;
  png_env_t Env;
  tim_image_t *pImg;
  tim_pixel_type_t pt;
  png_bytep *pRowPointers = NULL;

  /* Store session handle */
  Env.pSession = pSession;

  if (ppImage != NULL)
    *ppImage = NULL;

  /* Open file */
  Env.pvFile = tim_open(pSession, pcSource, TIM_IM_READ);
  if (Env.pvFile == NULL)
    return TIM_RV_ERR_NOT_FOUND;

  /* Init for reading */
  PngPtr = png_create_read_struct(
    PNG_LIBPNG_VER_STRING, (png_voidp)&Env, user_error_fn, user_warning_fn);
  if (!PngPtr) {
    tim_close(pSession, Env.pvFile);
    return TIM_RV_ERR_LIBRARY;
  }

  InfoPtr = png_create_info_struct(PngPtr);
  if (!InfoPtr) {
    png_destroy_read_struct(&PngPtr, (png_infopp)NULL, (png_infopp)NULL);
    tim_close(pSession, Env.pvFile);
    return TIM_RV_ERR_LIBRARY;
  }

  EndInfo = png_create_info_struct(PngPtr);
  if (!EndInfo) {
    png_destroy_read_struct(&PngPtr, &InfoPtr, (png_infopp)NULL);
    tim_close(pSession, Env.pvFile);
    return TIM_RV_ERR_LIBRARY;
  }

  /* Set error handling stuff. God, how I hate setjmp/longjmp :| */
  if (setjmp(png_jmpbuf(PngPtr))) {
    png_destroy_read_struct(&PngPtr, &InfoPtr, &EndInfo);
    tim_close(pSession, Env.pvFile);
    if (ppImage != NULL && *ppImage != NULL) {
      tim_destroy_image(*ppImage);
      *ppImage = NULL;
    }
    if (pRowPointers != NULL)
      tim_free(pSession, pRowPointers);
    return TIM_RV_ERR_LIBRARY;
  }

  /* Init I/O */
  png_set_read_fn(PngPtr, &Env, user_read_data);

  /* Read info */
  png_read_info(PngPtr, InfoPtr);

  /* Get info */
  nChannels = png_get_channels(PngPtr, InfoPtr);
  nWidth = png_get_image_width(PngPtr, InfoPtr);
  nHeight = png_get_image_height(PngPtr, InfoPtr);
  nBitDepth = png_get_bit_depth(PngPtr, InfoPtr);
  nColorType = png_get_color_type(PngPtr, InfoPtr);
  nRowBytes = png_get_rowbytes(PngPtr, InfoPtr);

  if (nColorType & PNG_COLOR_MASK_ALPHA)
    pt = TIM_PT_RGBA32;
  else
    pt = TIM_PT_RGB24;

  /* Get info only? */
  if (pInfo) {
    /* Info only */
    memset(pInfo, 0, sizeof(tim_image_info_t));
    pInfo->nWidth = nWidth;
    pInfo->nHeight = nHeight;
    pInfo->PixelType = pt;
  } else {
    /* Setup input transformation */
    if (nColorType == PNG_COLOR_TYPE_PALETTE)
      png_set_palette_to_rgb(PngPtr);

    if (nColorType == PNG_COLOR_TYPE_GRAY && nBitDepth < 8)
      png_set_expand_gray_1_2_4_to_8(PngPtr);

    if (png_get_valid(PngPtr, InfoPtr, PNG_INFO_tRNS))
      png_set_tRNS_to_alpha(PngPtr);

    if (nBitDepth == 16)
      png_set_strip_16(PngPtr);

    if (nBitDepth < 8)
      png_set_packing(PngPtr);

    if (nColorType == PNG_COLOR_TYPE_GRAY ||
        nColorType == PNG_COLOR_TYPE_GRAY_ALPHA)
      png_set_gray_to_rgb(PngPtr);

    /* Tell PNG to handle interlacing */
    nNumberOfPasses = png_set_interlace_handling(PngPtr);

    /* Update info struct */
    png_read_update_info(PngPtr, InfoPtr);
    nRowBytes = png_get_rowbytes(PngPtr, InfoPtr);

    /* Allocate image structure */
    pImg = tim_create_image(pSession, nWidth, nHeight, pt);
    if (pImg == NULL)
      longjmp(png_jmpbuf(PngPtr), -1); /* Long jump! (yick!) I'm only using this
                                          because I enjoy hating myself :) */
    *ppImage = pImg;

    /* Load image */
    pRowPointers =
      (png_bytep *)tim_alloc(pSession, sizeof(png_bytep) * nHeight);
    if (pRowPointers == NULL)
      longjmp(png_jmpbuf(PngPtr), -1);

    for (i = 0; i < nHeight; i++)
      pRowPointers[i] = (png_bytep) & ((char *)pImg->pvData)[nRowBytes * i];

    png_read_image(PngPtr, pRowPointers);
    tim_free(pSession, (void *)pRowPointers);

    /* Finish */
    png_read_end(PngPtr, EndInfo);
  }

  /* Clean up */
  png_destroy_read_struct(&PngPtr, &InfoPtr, &EndInfo);
  tim_close(pSession, Env.pvFile);

  /* OK */
  return TIM_RV_OK;
}

/*==============================================================================
Save function
==============================================================================*/

int tim_png_save(tim_image_t *pImage, const char *pcTarget) {
  png_env_t Env;
  png_structp PngPtr;
  tim_session_t *pSession;
  png_infop InfoPtr;
  int nColorType;
  png_bytep *pRowPointers = NULL;
  int nRowBytes, i;

  /* Store session handle */
  pSession = pImage->pSession;
  Env.pSession = pSession;

  /* Open file */
  Env.pvFile = tim_open(pSession, pcTarget, TIM_IM_WRITE);
  if (Env.pvFile == NULL)
    return TIM_RV_ERR_NOT_FOUND;

  /* Create write & info structs */
  PngPtr = png_create_write_struct(
    PNG_LIBPNG_VER_STRING, (png_voidp)&Env, user_error_fn, user_warning_fn);
  if (!PngPtr) {
    tim_close(pSession, Env.pvFile);
    return TIM_RV_ERR_LIBRARY;
  }

  InfoPtr = png_create_info_struct(PngPtr);
  if (!InfoPtr) {
    png_destroy_write_struct(&PngPtr, (png_infopp)NULL);
    tim_close(pSession, Env.pvFile);
    return TIM_RV_ERR_LIBRARY;
  }

  /* Set error handling stuff */
  if (setjmp(png_jmpbuf(PngPtr))) {
    png_destroy_write_struct(&PngPtr, &InfoPtr);
    tim_close(pSession, Env.pvFile);
    if (pRowPointers)
      tim_free(pSession, pRowPointers);
    return TIM_RV_ERR_LIBRARY;
  }

  /* Init I/O */
  png_set_write_fn(PngPtr, &Env, user_write_data, user_flush_data);

  /* Setup compression */
  png_set_compression_level(PngPtr, Z_BEST_COMPRESSION);
  png_set_compression_mem_level(PngPtr, 8);
  png_set_compression_strategy(PngPtr, Z_DEFAULT_STRATEGY);
  png_set_compression_window_bits(PngPtr, 15);
  png_set_compression_method(PngPtr, 8);
  png_set_compression_buffer_size(PngPtr, 8192);

  /* Set header */
  if (pImage->Info.PixelType == TIM_PT_RGB24) {
    nColorType = PNG_COLOR_TYPE_RGB;
    nRowBytes = pImage->Info.nWidth * 3;
  } else if (pImage->Info.PixelType == TIM_PT_RGBA32) {
    nColorType = PNG_COLOR_TYPE_RGB_ALPHA;
    nRowBytes = pImage->Info.nWidth * 4;
  } else
    longjmp(png_jmpbuf(PngPtr), -1);

  png_set_IHDR(PngPtr,
               InfoPtr,
               pImage->Info.nWidth,
               pImage->Info.nHeight,
               8,
               nColorType,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  /* Write header */
  png_write_info(PngPtr, InfoPtr);

  /* Write */
  pRowPointers =
    (png_bytep *)tim_alloc(pSession, sizeof(png_bytep) * pImage->Info.nHeight);
  if (pRowPointers == NULL)
    longjmp(png_jmpbuf(PngPtr), -1);

  for (i = 0; i < pImage->Info.nHeight; i++)
    pRowPointers[i] = (png_bytep) & ((char *)pImage->pvData)[nRowBytes * i];

  png_write_image(PngPtr, pRowPointers);

  /* Write end */
  png_write_end(PngPtr, InfoPtr);
  tim_close(pSession, Env.pvFile);
  if (pRowPointers)
    tim_free(pSession, pRowPointers);
  png_destroy_write_struct(&PngPtr, &InfoPtr);

  /* OK */
  return TIM_RV_OK;
}

/*==============================================================================
Init
==============================================================================*/

int tim_add_png_support(tim_session_t *pSession) {
  tim_file_format_t *pFormat;

  /* Define PNG file format */
  pFormat =
    tim_create_file_format(pSession, ".png", tim_png_save, tim_png_load);
  if (!pFormat)
    return TIM_RV_ERR_COULD_NOT_PERFORM;

  /* Return OK */
  return TIM_RV_OK;
}
