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

/* tim.h */
#ifndef __TIM_H__ /* TIM */
#define __TIM_H__

/*==============================================================================
Return values
==============================================================================*/

typedef enum _tim_retval_t {
  TIM_RV_OK = 0, /* Everything's OK */
  TIM_RV_ERR_INVALID_PARAM = -1, /* Parametres parsed to function are not
                              right */
  TIM_RV_ERR_COULD_NOT_PERFORM =
    -2, /* Could not perform the specified action */
  TIM_RV_ERR_INTERNAL_LIMIT = -3, /* Internal limit (array bound) reached */
  TIM_RV_ERR_MEMORY = -4, /* Some memoryrelated problem */
  TIM_RV_ERR_NOT_FOUND = -5, /* Something wasn't found */
  TIM_RV_ERR_LIBRARY = -6, /* Error occurred in some external library */
  TIM_RV_ERR_INVALID_FORMAT = -7 /* Invalid parameter format */
} tim_retval_t;

/*==============================================================================
Types: Memory system
==============================================================================*/

/* Function callback types */
typedef void *(*tim_callback_alloc_t)(int nSize);
typedef void *(*tim_callback_realloc_t)(void *pvBlock, int nSize);
typedef void (*tim_callback_free_t)(void *pvBlock);

/* Memory system struct */
typedef struct _tim_memory_t {
  /* Memory callbacks */
  tim_callback_alloc_t tim_callback_alloc;
  tim_callback_realloc_t tim_callback_realloc;
  tim_callback_free_t tim_callback_free;
} tim_memory_t;

/*==============================================================================
Types: File I/O system
==============================================================================*/

/* I/O modes */
typedef enum _tim_io_mode_t { TIM_IM_READ, TIM_IM_WRITE } tim_io_mode_t;

/* Seek modes */
typedef enum _tim_seek_mode_t {
  TIM_SM_ABS,
  TIM_SM_REL,
  TIM_SM_END
} tim_seek_mode_t;

/* Function callback types */
typedef void *(*tim_callback_open_t)(const char *pcWhere, tim_io_mode_t IOMode);
typedef void (*tim_callback_close_t)(void *pvHandle);
typedef int (*tim_callback_seek_t)(void *pvHandle,
                                   int nOffset,
                                   tim_seek_mode_t SeekMode);
typedef int (*tim_callback_read_t)(void *pvHandle, void *pvBuf, int nSize);
typedef int (*tim_callback_write_t)(void *pvHandle, void *pvBuf, int nSize);
typedef int (*tim_callback_eof_t)(void *pvHandle);

/* I/O system struct */
typedef struct _tim_io_t {
  /* I/O callbacks */
  tim_callback_open_t tim_callback_open;
  tim_callback_close_t tim_callback_close;
  tim_callback_seek_t tim_callback_seek;
  tim_callback_read_t tim_callback_read;
  tim_callback_write_t tim_callback_write;
  tim_callback_eof_t tim_callback_eof;
} tim_io_t;

/*==============================================================================
Types: Pixels
==============================================================================*/

/* Pixel types */
typedef enum _tim_pixel_type_t {
  TIM_PT_RGB24,
  TIM_PT_RGBA32,
  TIM_PT_UNKNOWN
} tim_pixel_type_t;

/* 24-bit RGB */
typedef struct _tim_rgb_t {
  unsigned char r, g, b;
} tim_rgb24_t;

/* 32-bit RGBA */
typedef struct _tim_rgba_t {
  unsigned char r, g, b, a;
} tim_rgba32_t;

/*==============================================================================
Types: Images
==============================================================================*/

/* Time&date info */
typedef struct _tim_date_t {
  int nYear, nMonth, nDay, nHour, nMinute, nSecond;
} tim_date_t;

/* Image info struct */
typedef struct _tim_image_info_t {
  int nWidth, nHeight; /* Width and height of image */
  tim_pixel_type_t PixelType; /* Pixel type */

  /* Extended info */
  char cImageDescription[64];
  char cCameraManufacturer[64];
  char cCameraModel[64];
  int nOrientation;
  char cExposureTime[16];
  char cExposureProgram[32];
  tim_date_t OriginalDate;
  int nFlash;
} tim_image_info_t;

/* Image struct */
typedef struct _tim_image_t {
  /* Image information */
  tim_image_info_t Info;

  /* Session */
  struct _tim_session_t *pSession; /* Image of this session */

  /* Data */
  union {
    tim_rgb24_t *pRGB;
    tim_rgba32_t *pRGBA;
    void *pvData;
  };

  /* Placement in linked list of images in session */
  struct _tim_image_t *pNext, *pPrev;
} tim_image_t;

/*==============================================================================
Types: File formats
==============================================================================*/

#define TIM_MAX_FILE_FORMATS 32
#define TIM_MAX_EXTENSIONS_PER_FILE_FORMAT 4

/* File format hints */
typedef struct _tim_file_format_hint_t {
  char cTag[32]; /* Name tag of this hint */
  char cValue[256]; /* Value of hint */

  struct _tim_file_format_hint_t *pNext, *pPrev;
} tim_file_format_hint_t;

/* Function callback types */
typedef int (*tim_callback_load_t)(struct _tim_session_t *pSession,
                                   tim_image_t **ppImage,
                                   tim_image_info_t *pInfo,
                                   const char *pcSource);
typedef int (*tim_callback_save_t)(tim_image_t *pImage, const char *pcTarget);

/* File format struct */
typedef struct _tim_file_format_t {
  /* Extensions */
  int nNumExtensions;
  char *pcExtensions[TIM_MAX_EXTENSIONS_PER_FILE_FORMAT];

  /* Hints */
  tim_file_format_hint_t *pFirstHint, *pLastHint;

  /* Session */
  struct _tim_session_t *pSession;

  /* Callbacks */
  tim_callback_load_t tim_callback_load;
  tim_callback_save_t tim_callback_save;
} tim_file_format_t;

/*==============================================================================
General types
==============================================================================*/

/* Main session struct */
typedef struct _tim_session_t {
  /* User callbacks */
  tim_memory_t Memory; /* Memory handling */
  tim_io_t IO; /* File I/O */

  /* Images in session */
  tim_image_t *pFirstImage, *pLastImage;

  /* File formats */
  int nNumFileFormats;
  tim_file_format_t FileFormats[TIM_MAX_FILE_FORMATS];
} tim_session_t;

/*==============================================================================
Globals
==============================================================================*/

/* CRT Memory system */
extern tim_memory_t g_Memory_CRT;

/* stdio I/O system */
extern tim_io_t g_IO_Stdio;

/*==============================================================================
Function prototypes
==============================================================================*/

/* Memory */
void *tim_alloc(tim_session_t *pSession, int nSize);
void *tim_realloc(tim_session_t *pSession, void *pvBlock, int nSize);
void tim_free(tim_session_t *pSession, void *pvBlock);
char *tim_strdup(tim_session_t *pSession, const char *pcString);

/* File I/O */
void *tim_open(tim_session_t *pSession,
               const char *pcWhere,
               tim_io_mode_t IOMode);
void tim_close(tim_session_t *pSession, void *pvHandle);
int tim_seek(tim_session_t *pSession,
             void *pvHandle,
             int nOffset,
             tim_seek_mode_t SeekMode);
int tim_read(tim_session_t *pSession, void *pvHandle, void *pvBuf, int nSize);
int tim_write(tim_session_t *pSession, void *pvHandle, void *pvBuf, int nSize);
int tim_eof(tim_session_t *pSession, void *pvHandle);

/* File format management */
int tim_remove_hint(tim_session_t *pSession,
                    const char *pcExt,
                    const char *pcTag);
int tim_is_hint(tim_session_t *pSession, const char *pcExt, const char *pcTag);
const char *tim_get_hint(tim_session_t *pSession,
                         const char *pcExt,
                         const char *pcTag,
                         const char *pcDefaultValue);
int tim_set_hint(tim_session_t *pSession,
                 const char *pcExt,
                 const char *pcTag,
                 const char *pcValue);
float tim_get_hint_float(tim_session_t *pSession,
                         const char *pcExt,
                         const char *pcTag,
                         float fDefaultValue);
int tim_get_hint_int(tim_session_t *pSession,
                     const char *pcExt,
                     const char *pcTag,
                     int nDefaultValue);
int tim_set_hint_float(tim_session_t *pSession,
                       const char *pcExt,
                       const char *pcTag,
                       float fValue);
int tim_set_hint_int(tim_session_t *pSession,
                     const char *pcExt,
                     const char *pcTag,
                     int nValue);

int tim_add_extension_to_file_format(tim_file_format_t *p,
                                     const char *pcExtension);
tim_file_format_t *tim_create_file_format(tim_session_t *pSession,
                                          const char *pcExtension,
                                          tim_callback_save_t SaveCallback,
                                          tim_callback_load_t LoadCallback);
tim_file_format_t *tim_find_file_format(tim_session_t *pSession,
                                        const char *pcExtension);

/* Image management */
tim_image_t *tim_create_image(tim_session_t *pSession,
                              int nWidth,
                              int nHeight,
                              tim_pixel_type_t PixelType);
void tim_destroy_image(tim_image_t *p);
int tim_is_image_readable(tim_session_t *pSession, const char *pcFileName);
int tim_is_image_writable(tim_session_t *pSession, const char *pcFileName);
int tim_get_image_info(tim_session_t *pSession,
                       const char *pcFileName,
                       tim_image_info_t *pInfo);
int tim_read_image(tim_session_t *pSession,
                   const char *pcFileName,
                   tim_image_t **ppImage);
int tim_write_image(tim_image_t *pImage, const char *pcFileName);

/* Session */
int tim_init_session(tim_session_t *pSession,
                     tim_memory_t *pMemory,
                     tim_io_t *pIO);
void tim_free_session(tim_session_t *pSession);

/* stdio driver */
void *tim_stdio_open(const char *pcWhere, tim_io_mode_t IOMode);
void tim_stdio_close(void *pvHandle);
int tim_stdio_seek(void *pvHandle, int nOffset, tim_seek_mode_t SeekMode);
int tim_stdio_read(void *pvHandle, void *pvBuf, int nSize);
int tim_stdio_write(void *pvHandle, void *pvBuf, int nSize);
int tim_stdio_eof(void *pvHandle);

/* JPEG support */
int tim_add_jpeg_support(tim_session_t *pSession);

/* PNG support */
int tim_add_png_support(tim_session_t *pSession);

#endif
