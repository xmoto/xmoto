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

/* tim.cpp
 *
 * TinyImage API main.
 */
#include "tim.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*==============================================================================
  BAD BAD BAD
  ==============================================================================*/
#if !defined(WIN32)
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
  Memory system
  ==============================================================================*/

void *tim_alloc(tim_session_t *pSession, int nSize) {
  /* Allocate block of memory */
  return pSession->Memory.tim_callback_alloc(nSize);
}

void *tim_realloc(tim_session_t *pSession, void *pvBlock, int nSize) {
  /* Re-allocate block of memory */
  return pSession->Memory.tim_callback_realloc(pvBlock, nSize);
}

void tim_free(tim_session_t *pSession, void *pvBlock) {
  /* Free block of memory */
  pSession->Memory.tim_callback_free(pvBlock);
}

char *tim_strdup(tim_session_t *pSession, const char *pcString) {
  char *pcNewString;
  int nLen;
  nLen = (int)strlen(pcString) + 1;
  pcNewString = (char *)tim_alloc(pSession, nLen);
  if (pcNewString == NULL)
    return NULL;
  memcpy(pcNewString, pcString, nLen);
  return pcNewString;
}

/*==============================================================================
  File I/O system
  ==============================================================================*/

void *tim_open(tim_session_t *pSession,
               const char *pcWhere,
               tim_io_mode_t IOMode) {
  /* Open file */
  return pSession->IO.tim_callback_open(pcWhere, IOMode);
}

void tim_close(tim_session_t *pSession, void *pvHandle) {
  /* Close file */
  pSession->IO.tim_callback_close(pvHandle);
}

int tim_seek(tim_session_t *pSession,
             void *pvHandle,
             int nOffset,
             tim_seek_mode_t SeekMode) {
  /* Seek in file and return new offset */
  return pSession->IO.tim_callback_seek(pvHandle, nOffset, SeekMode);
}

int tim_read(tim_session_t *pSession, void *pvHandle, void *pvBuf, int nSize) {
  /* Read from file */
  return pSession->IO.tim_callback_read(pvHandle, pvBuf, nSize);
}

int tim_write(tim_session_t *pSession, void *pvHandle, void *pvBuf, int nSize) {
  /* Write to file */
  return pSession->IO.tim_callback_write(pvHandle, pvBuf, nSize);
}

int tim_eof(tim_session_t *pSession, void *pvHandle) {
  /* Return EOF flag */
  return pSession->IO.tim_callback_eof(pvHandle);
}

/*==============================================================================
  File format management
  ==============================================================================*/

int tim_add_extension_to_file_format(tim_file_format_t *p,
                                     const char *pcExtension) {
  /* Room for more extensions? */
  if (p->nNumExtensions >= TIM_MAX_EXTENSIONS_PER_FILE_FORMAT)
    return TIM_RV_ERR_INTERNAL_LIMIT;

  /* Set extension */
  p->pcExtensions[p->nNumExtensions++] = tim_strdup(p->pSession, pcExtension);
  if (p->pcExtensions[p->nNumExtensions - 1] == NULL) {
    p->nNumExtensions--;
    return TIM_RV_ERR_MEMORY;
  }

  /* Return ok */
  return TIM_RV_OK;
}

tim_file_format_t *tim_create_file_format(tim_session_t *pSession,
                                          const char *pcExtension,
                                          tim_callback_save_t SaveCallback,
                                          tim_callback_load_t LoadCallback) {
  tim_file_format_t *p;

  /* Room for more formats? */
  if (pSession->nNumFileFormats >= TIM_MAX_FILE_FORMATS)
    return NULL;

  /* New file format */
  p = &pSession->FileFormats[pSession->nNumFileFormats++];

  /* Set contents */
  p->tim_callback_save = SaveCallback;
  p->tim_callback_load = LoadCallback;
  p->pSession = pSession;

  /* Set standard extension */
  if (tim_add_extension_to_file_format(p, pcExtension) < 0) {
    pSession->nNumFileFormats--;
    return NULL;
  }

  /* Return */
  return p;
}

tim_file_format_t *tim_find_file_format(tim_session_t *pSession,
                                        const char *pcExtension) {
  int i, j;

  /* Do we have a file format with this extension? */
  for (i = 0; i < pSession->nNumFileFormats; i++) {
    for (j = 0; j < pSession->FileFormats[i].nNumExtensions; j++) {
      /* This? */
      if (!stricmp(pSession->FileFormats[i].pcExtensions[j], pcExtension)) {
        /* Yep */
        return &pSession->FileFormats[i];
      }
    }
  }

  /* No! */
  return NULL;
}

tim_file_format_t *tim_determine_file_format(tim_session_t *pSession,
                                             const char *pcFileName) {
  int i;

  /* Extract extension */
  for (i = strlen(pcFileName) - 1; i >= 0; i--) {
    if (pcFileName[i] == '.')
      return tim_find_file_format(pSession, &pcFileName[i]);
  }

  /* Nothing */
  return NULL;
}

tim_file_format_hint_t *tim_find_file_format_hint(tim_file_format_t *pFormat,
                                                  const char *pcTag) {
  tim_file_format_hint_t *p;

  /* Is this hint found? */
  for (p = pFormat->pFirstHint; p != NULL; p = p->pNext)
    if (!stricmp(p->cTag, pcTag))
      return p;

  /* Nope */
  return NULL;
}

tim_file_format_hint_t *tim_add_file_format_hint(tim_session_t *pSession,
                                                 const char *pcExt,
                                                 const char *pcTag,
                                                 const char *pcValue) {
  tim_file_format_t *pFormat;
  tim_file_format_hint_t *p;

  /* Find format */
  pFormat = tim_find_file_format(pSession, pcExt);
  if (!pFormat)
    return NULL;

  /* Is this hint already there? - if so, change its value */
  p = tim_find_file_format_hint(pFormat, pcTag);
  if (!p) {
    /* Nope, create it */
    p = (tim_file_format_hint_t *)tim_alloc(pFormat->pSession,
                                            sizeof(tim_file_format_hint_t));
    if (!p)
      return NULL;

    /* Set tag */
    strcpy(p->cTag, pcTag);

    /* Insert in list */
    p->pNext = NULL;
    p->pPrev = pFormat->pLastHint;
    if (!pFormat->pFirstHint)
      pFormat->pFirstHint = p;
    else
      pFormat->pLastHint->pNext = p;
    pFormat->pLastHint = p;
  }

  /* Set value */
  strcpy(p->cValue, pcValue);

  return p;
}

int tim_remove_hint(tim_session_t *pSession,
                    const char *pcExt,
                    const char *pcTag) {
  tim_file_format_t *pFormat;
  tim_file_format_hint_t *p;

  /* Find format */
  pFormat = tim_find_file_format(pSession, pcExt);
  if (!pFormat)
    return TIM_RV_ERR_NOT_FOUND;

  /* Find hint */
  p = tim_find_file_format_hint(pFormat, pcTag);
  if (p) {
    /* It's there... get rid of it!! */
    if (p->pNext)
      p->pNext->pPrev = p->pPrev;
    else
      pFormat->pLastHint = p->pPrev;

    if (p->pPrev)
      p->pPrev->pNext = p->pNext;
    else
      pFormat->pFirstHint = p->pNext;

    tim_free(pSession, p);
  }

  /* OK */
  return TIM_RV_OK;
}

int tim_is_hint(tim_session_t *pSession, const char *pcExt, const char *pcTag) {
  tim_file_format_t *pFormat;

  /* Find format */
  pFormat = tim_find_file_format(pSession, pcExt);
  if (!pFormat)
    return false;

  /* Hint there? */
  if (tim_find_file_format_hint(pFormat, pcTag))
    return true;

  /* No such hint */
  return false;
}

const char *tim_get_hint(tim_session_t *pSession,
                         const char *pcExt,
                         const char *pcTag,
                         const char *pcDefaultValue) {
  tim_file_format_t *pFormat;
  tim_file_format_hint_t *p;

  /* Find format */
  pFormat = tim_find_file_format(pSession, pcExt);
  if (!pFormat)
    return pcDefaultValue;

  /* Find hint */
  p = tim_find_file_format_hint(pFormat, pcTag);
  if (!p)
    return pcDefaultValue;

  /* Return value */
  return p->cValue;
}

int tim_set_hint(tim_session_t *pSession,
                 const char *pcExt,
                 const char *pcTag,
                 const char *pcValue) {
  /* Set hint */
  if (!tim_add_file_format_hint(pSession, pcExt, pcTag, pcValue))
    return TIM_RV_ERR_COULD_NOT_PERFORM;

  /* OK */
  return TIM_RV_OK;
}

float tim_get_hint_float(tim_session_t *pSession,
                         const char *pcExt,
                         const char *pcTag,
                         float fDefaultValue) {
  char cTemp[256];

  /* Make default value string */
  sprintf(cTemp, "%f", fDefaultValue);

  /* Get & return */
  return (float)atof(tim_get_hint(pSession, pcExt, pcTag, cTemp));
}

int tim_get_hint_int(tim_session_t *pSession,
                     const char *pcExt,
                     const char *pcTag,
                     int nDefaultValue) {
  char cTemp[256];

  /* Make default value string */
  sprintf(cTemp, "%d", nDefaultValue);

  /* Get & return */
  return atoi(tim_get_hint(pSession, pcExt, pcTag, cTemp));
}

int tim_set_hint_float(tim_session_t *pSession,
                       const char *pcExt,
                       const char *pcTag,
                       float fValue) {
  char cTemp[256];
  sprintf(cTemp, "%f", fValue);
  return tim_set_hint(pSession, pcExt, pcTag, cTemp);
}

int tim_set_hint_int(tim_session_t *pSession,
                     const char *pcExt,
                     const char *pcTag,
                     int nValue) {
  char cTemp[256];
  sprintf(cTemp, "%d", nValue);
  return tim_set_hint(pSession, pcExt, pcTag, cTemp);
}

/*==============================================================================
  Image management
  ==============================================================================*/

int tim_get_bytes_per_pixel(tim_pixel_type_t PixelType) {
  switch (PixelType) {
    case TIM_PT_RGB24:
      return 3;
    case TIM_PT_RGBA32:
      return 4;
    default:
      /* Invalid pixel type */
      return 0;
  }
}

tim_image_t *tim_create_image(tim_session_t *pSession,
                              int nWidth,
                              int nHeight,
                              tim_pixel_type_t PixelType) {
  tim_image_t *p;
  int nBytesPerPixel;

  /* How many bytes per pixel? */
  nBytesPerPixel = tim_get_bytes_per_pixel(PixelType);
  if (nBytesPerPixel == 0)
    return NULL;

  /* Allocate image struct */
  p = (tim_image_t *)tim_alloc(pSession, sizeof(tim_image_t));
  if (!p) {
    /* Failed to allocate */
    return NULL;
  }

  /* Reset contents */
  memset(p, 0, sizeof(tim_image_t));

  /* Set session */
  p->pSession = pSession;

  /* Insert in list */
  p->pPrev = pSession->pLastImage;
  if (!pSession->pFirstImage)
    pSession->pFirstImage = p;
  else
    pSession->pLastImage->pNext = p;
  pSession->pLastImage = p;

  /* Set basic information */
  p->Info.nWidth = nWidth;
  p->Info.nHeight = nHeight;
  p->Info.PixelType = PixelType;

  /* Allocate pixels */
  p->pvData = (void *)tim_alloc(pSession, nWidth * nHeight * nBytesPerPixel);
  if (!p->pvData) {
    tim_free(pSession, p);
    return NULL;
  }
  memset(p->pvData, 0, nWidth * nHeight * nBytesPerPixel);

  /* Return */
  return p;
}

void tim_destroy_image(tim_image_t *p) {
  /* Remove from list */
  if (p->pNext)
    p->pNext->pPrev = p->pPrev;
  else
    p->pSession->pLastImage = p->pPrev;

  if (p->pPrev)
    p->pPrev->pNext = p->pNext;
  else
    p->pSession->pFirstImage = p->pNext;

  /* Free pixel data */
  tim_free(p->pSession, p->pvData);

  /* Free Struct */
  tim_free(p->pSession, p);
}

int tim_is_image_readable(tim_session_t *pSession, const char *pcFileName) {
  tim_file_format_t *pFileFormat;

  /* Find file format */
  pFileFormat = tim_determine_file_format(pSession, pcFileName);
  if (pFileFormat && pFileFormat->tim_callback_load)
    return true;
  return false;
}

int tim_is_image_writable(tim_session_t *pSession, const char *pcFileName) {
  tim_file_format_t *pFileFormat;

  /* Find file format */
  pFileFormat = tim_determine_file_format(pSession, pcFileName);
  if (pFileFormat && pFileFormat->tim_callback_save)
    return true;
  return false;
}

int tim_get_image_info(tim_session_t *pSession,
                       const char *pcFileName,
                       tim_image_info_t *pInfo) {
  tim_file_format_t *pFileFormat;

  /* Find file format */
  pFileFormat = tim_determine_file_format(pSession, pcFileName);
  if (pFileFormat && pFileFormat->tim_callback_load) {
    return pFileFormat->tim_callback_load(pSession, NULL, pInfo, pcFileName);
  }

  return TIM_RV_ERR_COULD_NOT_PERFORM;
}

int tim_read_image(tim_session_t *pSession,
                   const char *pcFileName,
                   tim_image_t **ppImage) {
  tim_file_format_t *pFileFormat;

  /* Find file format */
  pFileFormat = tim_determine_file_format(pSession, pcFileName);
  if (pFileFormat && pFileFormat->tim_callback_load) {
    return pFileFormat->tim_callback_load(pSession, ppImage, NULL, pcFileName);
  }

  return TIM_RV_ERR_COULD_NOT_PERFORM;
}

int tim_write_image(tim_image_t *pImage, const char *pcFileName) {
  tim_file_format_t *pFileFormat;

  /* Find file format */
  pFileFormat = tim_determine_file_format(pImage->pSession, pcFileName);
  if (pFileFormat && pFileFormat->tim_callback_save) {
    return pFileFormat->tim_callback_save(pImage, pcFileName);
  }

  return TIM_RV_ERR_COULD_NOT_PERFORM;
}

/*==============================================================================
  Session management
  ==============================================================================*/

void tim_free_session(tim_session_t *pSession) {
  int i, j;

  /* Free images */
  while (pSession->pFirstImage)
    tim_destroy_image(pSession->pFirstImage);

  /* Free memory associated with this session */
  /* File format extensions and hints must be freed */
  for (i = 0; i < pSession->nNumFileFormats; i++) {
    /* Hints */
    while (pSession->FileFormats[i].pFirstHint)
      tim_remove_hint(pSession,
                      pSession->FileFormats[i].pcExtensions[0],
                      pSession->FileFormats[i].pFirstHint->cTag);

    /* Exts */
    for (j = 0; j < pSession->FileFormats[i].nNumExtensions; j++)
      tim_free(pSession, pSession->FileFormats[i].pcExtensions[j]);
  }
}

int tim_init_session(tim_session_t *pSession,
                     tim_memory_t *pMemory,
                     tim_io_t *pIO) {
  /* Validate memory struct */
  if (pMemory->tim_callback_alloc == NULL ||
      pMemory->tim_callback_free == NULL ||
      pMemory->tim_callback_realloc == NULL)
    return TIM_RV_ERR_INVALID_PARAM;

  /* Validate I/O struct */
  if (pIO->tim_callback_close == NULL || pIO->tim_callback_open == NULL ||
      pIO->tim_callback_read == NULL || pIO->tim_callback_seek == NULL ||
      pIO->tim_callback_write == NULL || pIO->tim_callback_eof == NULL)
    return TIM_RV_ERR_INVALID_PARAM;

  /* Reset session */
  memset(pSession, 0, sizeof(tim_session_t));

  /* Copy callback structs */
  memcpy(&pSession->Memory, pMemory, sizeof(tim_memory_t));
  memcpy(&pSession->IO, pIO, sizeof(tim_io_t));

  /* OK */
  return TIM_RV_OK;
}
