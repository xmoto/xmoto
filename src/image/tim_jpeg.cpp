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

/* tim_jpeg.cpp
 *
 * libjpeg wrapper.
 */
#include "tim.h"
#include <ctype.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "jerror.h"
#include <jpeglib.h>
}

/*==============================================================================
Hmm, stricmp() is microsoftish
==============================================================================*/
#if !defined(WIN32) && !defined(__amigaos4__)
static int stricmp(const char *pc1, const char *pc2) {
  int s1 = strlen(pc1);
  int s2 = strlen(pc2);
  if (s1 != s2)
    return 1;
  for (int i = 0; i < s1; i++)
    if (tolower(pc1[i]) != tolower(pc2[i]))
      return 1;
  return 0;
}
#endif

/*==============================================================================
Local types
==============================================================================*/

/* Internal struct that desripes our error manager */
#define TIM_MAX_JPG_ERROR_MESSAGES 16

typedef struct _error_mgr_t {
  struct jpeg_error_mgr Public;
  jmp_buf SetJmp; /* spaghetti-code nightmare! :-O */
  int nNumMessages;
  char *pcMessages[TIM_MAX_JPG_ERROR_MESSAGES];
} tim_error_mgr_t;

/* Internal struct that descripes our source manager */
#define TIM_JPG_INPUT_BUF_SIZE 4096

typedef struct _tim_source_mgr_t {
  struct jpeg_source_mgr Public;
  JOCTET Buffer[TIM_JPG_INPUT_BUF_SIZE];
  void *pvHandle; /* File handle */
  int nStartOfFile;
} tim_source_mgr_t;

/* Internal struct that descripes our destination manager */
#define TIM_JPG_OUTPUT_BUF_SIZE 4096

typedef struct _tim_destination_mgr_t {
  struct jpeg_destination_mgr Public;
  JOCTET Buffer[TIM_JPG_OUTPUT_BUF_SIZE];
  void *pvHandle; /* File handle */
} tim_destination_mgr_t;

/* Jpeg RGB pixel format */
typedef struct _tim_jpeg_pixel_t {
  unsigned char r, g, b;
} tim_jpeg_pixel_t;

/*==============================================================================
Error stuff
==============================================================================*/

void tim_jpeg_error_exit(j_common_ptr pci) {
  tim_error_mgr_t *pErrorMgr;

  /* Get pointer to actual struct */
  pErrorMgr = (tim_error_mgr_t *)pci->err;

  /* Output the message */
  (*pci->err->output_message)(pci);

  /* Do the spaghetti jump */
  longjmp(pErrorMgr->SetJmp, 1);
}

void tim_jpeg_output_message(j_common_ptr pci) {
  char cTemp[256];
  tim_error_mgr_t *pErrorMgr;

  /* Get pointer to actual struct */
  pErrorMgr = (tim_error_mgr_t *)pci->err;

  /* Format message */
  (*pci->err->format_message)(pci, cTemp);

  /*printf("[%s]\n",cTemp);*/

  /* Add it to the list */
  if (pErrorMgr->nNumMessages < TIM_MAX_JPG_ERROR_MESSAGES) {
    pErrorMgr->pcMessages[pErrorMgr->nNumMessages] =
      tim_strdup((tim_session_t *)pci->client_data, cTemp);
    pErrorMgr->nNumMessages++;
  }
}

/*==============================================================================
Source stuff
==============================================================================*/

METHODDEF(void) tim_jpeg_init_source(j_decompress_ptr cinfo) {
  tim_source_mgr_t *pSrc = (tim_source_mgr_t *)cinfo->src;
  pSrc->nStartOfFile = TRUE;
}

METHODDEF(boolean) tim_jpeg_fill_input_buffer(j_decompress_ptr cinfo) {
  int nBytes;
  tim_session_t *pSession;
  tim_source_mgr_t *pSrc = (tim_source_mgr_t *)cinfo->src;

  /* Get session */
  pSession = (tim_session_t *)cinfo->client_data;

  /* Read data */
  nBytes =
    tim_read(pSession, pSrc->pvHandle, pSrc->Buffer, TIM_JPG_INPUT_BUF_SIZE);
  if (nBytes <= 0) {
    /* Empty input file equals fatal error */
    if (pSrc->nStartOfFile)
      ERREXIT(cinfo, JERR_INPUT_EMPTY);

    /* Insert a fake EOI marker */
    pSrc->Buffer[0] = (JOCTET)0xFF;
    pSrc->Buffer[1] = (JOCTET)JPEG_EOI;
    nBytes = 2;
  }

  pSrc->Public.next_input_byte = pSrc->Buffer;
  pSrc->Public.bytes_in_buffer = nBytes;
  pSrc->nStartOfFile = FALSE;

  return TRUE;
}

METHODDEF(void)
tim_jpeg_skip_input_data(j_decompress_ptr cinfo, long nNumBytes) {
  tim_source_mgr_t *pSrc = (tim_source_mgr_t *)cinfo->src;

  if (nNumBytes > 0) {
    while (nNumBytes > (long)pSrc->Public.bytes_in_buffer) {
      nNumBytes -= (long)pSrc->Public.bytes_in_buffer;
      (void)tim_jpeg_fill_input_buffer(cinfo);
    }
    pSrc->Public.next_input_byte += (size_t)nNumBytes;
    pSrc->Public.bytes_in_buffer -= (size_t)nNumBytes;
  }
}

METHODDEF(void) tim_jpeg_term_source(j_decompress_ptr cinfo) {
  /* Just empty */
}

void tim_jpeg_setup_src(tim_session_t *pSession,
                        j_decompress_ptr cinfo,
                        void *pvHandle) {
  tim_source_mgr_t *pSrc;

  /* Allocate memory for manager */
  cinfo->src =
    (struct jpeg_source_mgr *)tim_alloc(pSession, sizeof(tim_source_mgr_t));
  pSrc = (tim_source_mgr_t *)cinfo->src;

  /* Set values */
  pSrc->Public.init_source = tim_jpeg_init_source;
  pSrc->Public.fill_input_buffer = tim_jpeg_fill_input_buffer;
  pSrc->Public.skip_input_data = tim_jpeg_skip_input_data;
  pSrc->Public.resync_to_restart = jpeg_resync_to_restart; /* Default */
  pSrc->Public.term_source = tim_jpeg_term_source;
  pSrc->pvHandle = pvHandle;
  pSrc->Public.bytes_in_buffer = 0;
  pSrc->Public.next_input_byte = NULL;
}

/*==============================================================================
Destination stuff
==============================================================================*/

METHODDEF(void) tim_jpeg_init_destination(j_compress_ptr cinfo) {
  tim_destination_mgr_t *pDest = (tim_destination_mgr_t *)cinfo->dest;
  pDest->Public.next_output_byte = pDest->Buffer;
  pDest->Public.free_in_buffer = TIM_JPG_OUTPUT_BUF_SIZE;
}

METHODDEF(boolean) tim_jpeg_empty_output_buffer(j_compress_ptr cinfo) {
  tim_destination_mgr_t *pDest = (tim_destination_mgr_t *)cinfo->dest;

  if (tim_write((tim_session_t *)cinfo->client_data,
                pDest->pvHandle,
                pDest->Buffer,
                TIM_JPG_OUTPUT_BUF_SIZE) != TIM_JPG_OUTPUT_BUF_SIZE)
    ERREXIT(cinfo, JERR_FILE_WRITE);

  pDest->Public.next_output_byte = pDest->Buffer;
  pDest->Public.free_in_buffer = TIM_JPG_OUTPUT_BUF_SIZE;

  return TRUE;
}

METHODDEF(void) tim_jpeg_term_destination(j_compress_ptr cinfo) {
  tim_destination_mgr_t *pDest = (tim_destination_mgr_t *)cinfo->dest;
  size_t nDataCount = TIM_JPG_OUTPUT_BUF_SIZE - pDest->Public.free_in_buffer;

  /* Write any data remaining in the buffer */
  if (nDataCount > 0) {
    if (tim_write((tim_session_t *)cinfo->client_data,
                  pDest->pvHandle,
                  pDest->Buffer,
                  (int)nDataCount) != (int)nDataCount)
      ERREXIT(cinfo, JERR_FILE_WRITE);
  }
}

void tim_jpeg_setup_dest(tim_session_t *pSession,
                         j_compress_ptr cinfo,
                         void *pvHandle) {
  tim_destination_mgr_t *pDest;

  /* Allocate memory for manager */
  cinfo->dest = (struct jpeg_destination_mgr *)tim_alloc(
    pSession, sizeof(tim_destination_mgr_t));
  pDest = (tim_destination_mgr_t *)cinfo->dest;

  /* Set values */
  pDest->Public.init_destination = tim_jpeg_init_destination;
  pDest->Public.empty_output_buffer = tim_jpeg_empty_output_buffer;
  pDest->Public.term_destination = tim_jpeg_term_destination;
  pDest->pvHandle = pvHandle;
}

/*==============================================================================
EXIF stuff (read-only, I'm a lazy guy)
==============================================================================*/

typedef enum _bytealign_t {
  BA_MOTOROLA = 0x4d4d,
  BA_INTEL = 0x4949
} bytealign_t;
typedef enum _dataformat_t {
  DF_UNSIGNED_BYTE = 1,
  DF_STRING = 2,
  DF_UNSIGNED_SHORT = 3,
  DF_UNSIGNED_LONG = 4,
  DF_UNSIGNED_RATIONAL = 5,
  DF_SIGNED_BYTE = 6,
  DF_UNDEFINED = 7,
  DF_SIGNED_SHORT = 8,
  DF_SIGNED_LONG = 9,
  DF_SIGNED_RATIONAL = 10,
  DF_SINGLE_FLOAT = 11,
  DF_DOUBLE_FLOAT = 12
} dataformat_t;

char tim_jpeg_get_byte(tim_session_t *pSession, void *pvHandle) {
  unsigned char n;
  tim_read(pSession, pvHandle, &n, 1);
  return n;
}

unsigned short tim_jpeg_get_motorola_short(tim_session_t *pSession,
                                           void *pvHandle) {
  unsigned char n1, n2;
  tim_read(pSession, pvHandle, &n1, 1);
  tim_read(pSession, pvHandle, &n2, 1);
  return n2 | (n1 << 8);
}

unsigned long tim_jpeg_get_motorola_long(tim_session_t *pSession,
                                         void *pvHandle) {
  unsigned short n1, n2;
  n1 = tim_jpeg_get_motorola_short(pSession, pvHandle);
  n2 = tim_jpeg_get_motorola_short(pSession, pvHandle);
  return n2 | (n1 << 16);
}

unsigned short tim_jpeg_get_intel_short(tim_session_t *pSession,
                                        void *pvHandle) {
  unsigned char n1, n2;
  tim_read(pSession, pvHandle, &n1, 1);
  tim_read(pSession, pvHandle, &n2, 1);
  return n1 | (n2 << 8);
}

unsigned long tim_jpeg_get_intel_long(tim_session_t *pSession, void *pvHandle) {
  unsigned short n1, n2;
  n1 = tim_jpeg_get_intel_short(pSession, pvHandle);
  n2 = tim_jpeg_get_intel_short(pSession, pvHandle);
  return n1 | (n2 << 16);
}

unsigned short tim_jpeg_get_short(tim_session_t *pSession,
                                  void *pvHandle,
                                  bytealign_t ba) {
  if (ba == BA_MOTOROLA)
    return tim_jpeg_get_motorola_short(pSession, pvHandle);
  else if (ba == BA_INTEL)
    return tim_jpeg_get_intel_short(pSession, pvHandle);
  return 0;
}

unsigned long tim_jpeg_get_long(tim_session_t *pSession,
                                void *pvHandle,
                                bytealign_t ba) {
  if (ba == BA_MOTOROLA)
    return tim_jpeg_get_motorola_long(pSession, pvHandle);
  else if (ba == BA_INTEL)
    return tim_jpeg_get_intel_long(pSession, pvHandle);
  return 0;
}

int tim_jpeg_get_bytes_per_component(int nDataFormat) {
  switch (nDataFormat) {
    case 1:
      return 1;
    case 2:
      return 1;
    case 3:
      return 2;
    case 4:
      return 4;
    case 5:
      return 8;
    case 6:
      return 1;
    case 7:
      return 1;
    case 8:
      return 2;
    case 9:
      return 4;
    case 10:
      return 8;
    case 11:
      return 4;
    case 12:
      return 8;
  }

  return 0;
}

void tim_jpeg_get_string(tim_session_t *pSession,
                         void *pvHandle,
                         char *pcBuf,
                         int nBufSize,
                         int nOffset) {
  int nOldOffset, i;
  nOldOffset = tim_seek(pSession, pvHandle, 0, TIM_SM_REL);
  tim_seek(pSession, pvHandle, nOffset, TIM_SM_ABS);
  for (i = 0; i < nBufSize - 1; i++) {
    pcBuf[i] = tim_jpeg_get_byte(pSession, pvHandle);
    if (!pcBuf[i])
      break;
  }
  pcBuf[nBufSize - 1] = '\0';
  tim_seek(pSession, pvHandle, nOldOffset, TIM_SM_ABS);
}

void tim_jpeg_get_rational(tim_session_t *pSession,
                           void *pvHandle,
                           char *pcBuf,
                           int nOffset) {
  int nOldOffset, nNum, nDenom;
  nOldOffset = tim_seek(pSession, pvHandle, 0, TIM_SM_REL);
  tim_seek(pSession, pvHandle, nOffset, TIM_SM_ABS);
  tim_read(pSession, pvHandle, &nNum, 4);
  tim_read(pSession, pvHandle, &nDenom, 4);
  sprintf(pcBuf, "%d/%d", nNum, nDenom);
  tim_seek(pSession, pvHandle, nOldOffset, TIM_SM_ABS);
}

int tim_jpeg_load_exif(tim_session_t *pSession,
                       tim_image_info_t *pInfo,
                       void *pvHandle) {
  int nAPP1Size;
  bytealign_t ba;
  int nTIFFStart, nFirstIFD, nNumEntries, nTagNum, nDataFormat;
  int i, nNumComponents, nValue /*,nNextOffset*/;
  int nContinueAt = 0;
  char cTemp[256];

  /* Reset I/O handle */
  tim_seek(pSession, pvHandle, 0, TIM_SM_ABS);

  /* Reset info */
  memset(pInfo, 0, sizeof(tim_image_info_t));

  /* Get first marker - it must be the SOI marker */
  if (tim_jpeg_get_motorola_short(pSession, pvHandle) != 0xFFD8)
    return TIM_RV_ERR_INVALID_FORMAT;

  /* Now we get more spicy - to understand this, the next marker MUST be APP1 */
  if (tim_jpeg_get_motorola_short(pSession, pvHandle) == 0xFFE1) {
    nAPP1Size = tim_jpeg_get_motorola_short(pSession, pvHandle) - 4;

    /* Is it EXIF? */
    if (tim_jpeg_get_byte(pSession, pvHandle) == 'E' &&
        tim_jpeg_get_byte(pSession, pvHandle) == 'x' &&
        tim_jpeg_get_byte(pSession, pvHandle) == 'i' &&
        tim_jpeg_get_byte(pSession, pvHandle) == 'f' &&
        tim_jpeg_get_byte(pSession, pvHandle) == 0 &&
        tim_jpeg_get_byte(pSession, pvHandle) == 0) {
      /* Remember this position - it's start of TIFF area */
      nTIFFStart = tim_seek(pSession, pvHandle, 0, TIM_SM_REL);

      /* Determine byte alignment */
      ba = (bytealign_t)tim_jpeg_get_motorola_short(pSession, pvHandle);

      /* Jump */
      tim_seek(pSession, pvHandle, 2, TIM_SM_REL);

      /* Get position of first IFD */
      nFirstIFD = tim_jpeg_get_long(pSession, pvHandle, ba);

      /* Jump to first IFD */
      tim_seek(pSession, pvHandle, nTIFFStart + nFirstIFD, TIM_SM_ABS);

      while (1) {
        /* Get number of entries */
        nNumEntries = tim_jpeg_get_short(pSession, pvHandle, ba);
        nContinueAt = 0; /* Default: No continue */

        for (i = 0; i < nNumEntries; i++) {
          /* Extract tag */
          nTagNum = tim_jpeg_get_short(pSession, pvHandle, ba);
          nDataFormat = tim_jpeg_get_short(pSession, pvHandle, ba);
          nNumComponents = tim_jpeg_get_long(pSession, pvHandle, ba);
          nValue = tim_jpeg_get_long(pSession, pvHandle, ba);

          /* Analyze! */
          if (nTagNum == 0x010E && nDataFormat == DF_STRING) {
            /* ImageDescription */
            tim_jpeg_get_string(pSession,
                                pvHandle,
                                pInfo->cImageDescription,
                                sizeof(pInfo->cImageDescription),
                                nValue + nTIFFStart);
          } else if (nTagNum == 0x010F && nDataFormat == DF_STRING) {
            /* Make */
            tim_jpeg_get_string(pSession,
                                pvHandle,
                                pInfo->cCameraManufacturer,
                                sizeof(pInfo->cCameraManufacturer),
                                nValue + nTIFFStart);
          } else if (nTagNum == 0x0110 && nDataFormat == DF_STRING) {
            /* Model */
            tim_jpeg_get_string(pSession,
                                pvHandle,
                                pInfo->cCameraModel,
                                sizeof(pInfo->cCameraModel),
                                nValue + nTIFFStart);
          } else if (nTagNum == 0x0112 && nDataFormat == DF_UNSIGNED_SHORT) {
            /* Orientation */
            pInfo->nOrientation = nValue;
          } else if (nTagNum == 0x8769 && nDataFormat == DF_UNSIGNED_LONG) {
            /* Uhh. nice... Go for the Sub IFD!! */
            nContinueAt = nValue;
          } else if (nTagNum == 0x829A && nDataFormat == DF_UNSIGNED_RATIONAL) {
            /* ExposureTime */
            tim_jpeg_get_rational(
              pSession, pvHandle, pInfo->cExposureTime, nValue + nTIFFStart);
          } else if (nTagNum == 0x8822 && nDataFormat == DF_UNSIGNED_SHORT) {
            /* ExposureProgram */
            switch (nValue) {
              case 1:
                strcpy(pInfo->cExposureProgram, "Manual");
                break;
              case 2:
                strcpy(pInfo->cExposureProgram, "Normal");
                break;
              case 3:
                strcpy(pInfo->cExposureProgram, "Aperture Priority");
                break;
              case 4:
                strcpy(pInfo->cExposureProgram, "Shutter Priority");
                break;
              case 5:
                strcpy(pInfo->cExposureProgram, "Creative");
                break;
              case 6:
                strcpy(pInfo->cExposureProgram, "Action");
                break;
              case 7:
                strcpy(pInfo->cExposureProgram, "Portrait");
                break;
              case 8:
                strcpy(pInfo->cExposureProgram, "Landscape");
                break;
            }
          } else if (nTagNum == 0x9003 && nDataFormat == DF_STRING) {
            /* Date */
            tim_jpeg_get_string(
              pSession, pvHandle, cTemp, sizeof(cTemp), nValue + nTIFFStart);
            sscanf(cTemp,
                   "%d:%d:%d %d:%d:%d",
                   &pInfo->OriginalDate.nYear,
                   &pInfo->OriginalDate.nMonth,
                   &pInfo->OriginalDate.nDay,
                   &pInfo->OriginalDate.nHour,
                   &pInfo->OriginalDate.nMinute,
                   &pInfo->OriginalDate.nSecond);
          } else if (nTagNum == 0x9209 && nDataFormat == DF_UNSIGNED_SHORT) {
            /* Flash */
            switch (nValue) {
              case 0:
                pInfo->nFlash = FALSE;
              default:
                pInfo->nFlash = TRUE;
            }
          }
        }

        /* Have we been told where to continue? */
        if (!nContinueAt)
          break;
        tim_seek(pSession, pvHandle, nTIFFStart + nContinueAt, TIM_SM_ABS);
      }
    }
  }

  /* Reset I/O handle */
  tim_seek(pSession, pvHandle, 0, TIM_SM_ABS);

  /* OK */
  return TIM_RV_OK;
}

/*==============================================================================
Load function
==============================================================================*/

int tim_jpeg_load(tim_session_t *pSession,
                  tim_image_t **ppImage,
                  tim_image_info_t *pInfo,
                  const char *pcSource) {
  void *pvHandle;
  struct jpeg_decompress_struct ci;
  tim_error_mgr_t Err;
  tim_image_t *pImage;
  unsigned int nRowStride, nOffset;
  const char *pc;
  JSAMPARRAY Buffer;
  tim_jpeg_pixel_t *pScan;
  int nRet;
  tim_image_info_t EXIF;

  /* Open source file */
  pvHandle = tim_open(pSession, pcSource, TIM_IM_READ);
  if (!pvHandle)
    return TIM_RV_ERR_NOT_FOUND;

  /* Should we try to retrieve any EXIF stuff? */
  if (!tim_is_hint(pSession, ".jpg", "jpeg_ignore_exif")) {
    nRet = tim_jpeg_load_exif(pSession, &EXIF, pvHandle);
    if (nRet != TIM_RV_OK) {
      tim_close(pSession, pvHandle);
      return nRet;
    }
  } else {
    memset(&EXIF, 0, sizeof(EXIF));
  }

  /* Set client data */
  ci.client_data = (void *)pSession;

  /* Setup error manager */
  ci.err = jpeg_std_error(&Err.Public);
  Err.nNumMessages = 0;
  Err.Public.error_exit = tim_jpeg_error_exit;
  Err.Public.output_message = tim_jpeg_output_message;

  /* Setup setjmp (nooo) */
  if (setjmp(Err.SetJmp)) {
    /* This is where we go on errors... */
    jpeg_destroy_decompress(&ci);
    tim_free(pSession, ci.src);
    tim_close(pSession, pvHandle);
    return TIM_RV_ERR_LIBRARY;
  }

  /* Create decompression object */
  jpeg_create_decompress(&ci);

  /* Tell IJG where to find the image */
  tim_jpeg_setup_src(pSession, &ci, pvHandle);

  /* Read header */
  jpeg_read_header(&ci, TRUE);

  /* Make sure we get RGB */
  ci.out_color_space = JCS_RGB;
  ci.out_color_components = 3;

  /* Hints? */
  switch (tim_get_hint_int(pSession, ".jpg", "jpeg_downscale", 1)) {
    case 2: /* Scale by 1/2 */
      ci.scale_num = 1;
      ci.scale_denom = 2;
      break;
    case 4: /* Scale by 1/4 */
      ci.scale_num = 1;
      ci.scale_denom = 4;
      break;
    case 8: /* Scale by 1/8 */
      ci.scale_num = 1;
      ci.scale_denom = 8;
      break;
    default: /* Scale by 1/1 */
      ci.scale_num = 1;
      ci.scale_denom = 1;
      break;
  }

  if (tim_is_hint(pSession, ".jpg", "jpeg_no_fancy_upsampling")) {
    ci.do_fancy_upsampling = FALSE;
  }

  pc = tim_get_hint(pSession, ".jpg", "jpeg_dct_method", "ifast");
  if (!stricmp(pc, "islow"))
    ci.dct_method = JDCT_ISLOW;
  else if (!stricmp(pc, "float"))
    ci.dct_method = JDCT_FLOAT;
  else
    ci.dct_method = JDCT_IFAST;

  /* Only retrieve info? */
  if (pInfo) {
    /* Infomode! */
    memcpy(pInfo, &EXIF, sizeof(EXIF)); /* Did we get some EXIF? */
    pInfo->nWidth = ci.image_width / ci.scale_denom;
    pInfo->nHeight = ci.image_height / ci.scale_denom;
    pInfo->PixelType = TIM_PT_RGB24;
  } else {
    /* Start decompressing... */
    jpeg_start_decompress(&ci);

    /* Allocate image */
    pImage = tim_create_image(
      pSession, ci.output_width, ci.output_height, TIM_PT_RGB24);
    if (!pImage)
      ERREXIT(&ci, 0);
    *ppImage = pImage;

    memcpy(&pImage->Info, &EXIF, sizeof(EXIF)); /* Did we get some EXIF? */
    pImage->Info.nWidth = ci.output_width;
    pImage->Info.nHeight = ci.output_height;
    pImage->Info.PixelType = TIM_PT_RGB24;

    nRowStride = ci.output_width * ci.out_color_components;
    Buffer =
      (*ci.mem->alloc_sarray)((j_common_ptr)&ci, JPOOL_IMAGE, nRowStride, 1);

    /* Decode */
    while (ci.output_scanline < ci.output_height) {
      /* Read scanline */
      jpeg_read_scanlines(&ci, Buffer, 1);
      pScan = (tim_jpeg_pixel_t *)Buffer[0];

      /* Set channel data */
      for (unsigned int i = 0; i < ci.output_width; i++) {
        /* Calculate position in channels */
        nOffset = i + (ci.output_scanline - 1) * ci.output_width;

        /* Copy pixel */
        pImage->pRGB[nOffset].r = pScan[i].r;
        pImage->pRGB[nOffset].g = pScan[i].g;
        pImage->pRGB[nOffset].b = pScan[i].b;
      }
    }

    /* End */
    jpeg_finish_decompress(&ci);
  }

  /* Clean up */
  jpeg_destroy_decompress(&ci);
  tim_free(pSession, ci.src);
  tim_close(pSession, pvHandle);

  /* OK */
  return TIM_RV_OK;
}

/*==============================================================================
Save function
==============================================================================*/

int tim_jpeg_save(tim_image_t *pImage, const char *pcTarget) {
  void *pvHandle;
  struct jpeg_compress_struct ci;
  tim_error_mgr_t Err;
  unsigned int i, nOffset;
  tim_jpeg_pixel_t *pScan;
  JSAMPROW RowPointer[1];

  /* Open source file */
  pvHandle = tim_open(pImage->pSession, pcTarget, TIM_IM_WRITE);
  if (!pvHandle)
    return TIM_RV_ERR_COULD_NOT_PERFORM;

  /* Set client data */
  ci.client_data = (void *)pImage->pSession;

  /* Setup error manager */
  ci.err = jpeg_std_error(&Err.Public);
  Err.nNumMessages = 0;
  Err.Public.error_exit = tim_jpeg_error_exit;
  Err.Public.output_message = tim_jpeg_output_message;

  /* Setup setjmp (nooo) */
  if (setjmp(Err.SetJmp)) {
    /* This is where we go on errors... */
    jpeg_destroy_compress(&ci);
    tim_free(pImage->pSession, ci.dest);
    tim_close(pImage->pSession, pvHandle);
    return TIM_RV_ERR_LIBRARY;
  }

  /* Create compression object */
  jpeg_create_compress(&ci);

  /* Set destination */
  tim_jpeg_setup_dest(pImage->pSession, &ci, pvHandle);

  /* Set options and parametres for compression */
  ci.image_width = pImage->Info.nWidth;
  ci.image_height = pImage->Info.nHeight;
  ci.input_components = 3;
  ci.in_color_space = JCS_RGB;

  /* Set defaults */
  jpeg_set_defaults(&ci);

  /* Set quality */
  jpeg_set_quality(
    &ci, tim_get_hint_int(pImage->pSession, ".jpg", "jpeg_quality", 95), TRUE);

  /* Begin */
  jpeg_start_compress(&ci, TRUE);
  pScan = (tim_jpeg_pixel_t *)tim_alloc(pImage->pSession,
                                        ci.image_width * ci.input_components);
  if (!pScan)
    ERREXIT(&ci, 0);

  while (ci.next_scanline < ci.image_height) {
    nOffset = ci.next_scanline * ci.image_width;

    for (i = 0; i < ci.image_width; i++) {
      switch (pImage->Info.PixelType) {
        case TIM_PT_RGB24:
          pScan[i].r = pImage->pRGB[nOffset + i].r;
          pScan[i].g = pImage->pRGB[nOffset + i].g;
          pScan[i].b = pImage->pRGB[nOffset + i].b;
          break;
        case TIM_PT_RGBA32:
          pScan[i].r = pImage->pRGBA[nOffset + i].r;
          pScan[i].g = pImage->pRGBA[nOffset + i].g;
          pScan[i].b = pImage->pRGBA[nOffset + i].b;
          break;
        default:
          pScan[i].r = pScan[i].g = pScan[i].b = 0;
      }
    }

    RowPointer[0] = &((JSAMPLE *)pScan)[0];
    jpeg_write_scanlines(&ci, RowPointer, 1);
  }

  tim_free(pImage->pSession, pScan);

  /* Clean up */
  jpeg_finish_compress(&ci);
  tim_free(pImage->pSession, ci.dest);
  tim_close(pImage->pSession, pvHandle);
  jpeg_destroy_compress(&ci);

  /* OK */
  return TIM_RV_OK;
}

/*==============================================================================
Init
==============================================================================*/

int tim_add_jpeg_support(tim_session_t *pSession) {
  tim_file_format_t *pFormat;

  /* Define JPEG file format */
  pFormat =
    tim_create_file_format(pSession, ".jpg", tim_jpeg_save, tim_jpeg_load);
  if (!pFormat)
    return TIM_RV_ERR_COULD_NOT_PERFORM;

  /* Add additional extension and return */
  return tim_add_extension_to_file_format(pFormat, ".jpeg");
}
