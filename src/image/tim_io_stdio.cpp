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

/* tim_io_stdio.cpp
 *
 * stdio driver.
 */
#include "tim.h"
#include <stdio.h>

/*==============================================================================
File I/O system - stdio
==============================================================================*/

void *tim_stdio_open(const char *pcWhere, tim_io_mode_t IOMode) {
  const char *pcIOMode = NULL;

  /* Determine mode of I/O */
  switch (IOMode) {
    case TIM_IM_READ:
      pcIOMode = "rb";
      break;
    case TIM_IM_WRITE:
      pcIOMode = "wb";
      break;
  }

  /* No prober I/O mode? */
  if (pcIOMode == NULL)
    return NULL;

  /* Use fopen() to open the file */
  return (void *)fopen(pcWhere, pcIOMode);
}

void tim_stdio_close(void *pvHandle) {
  /* Use fclose() to close the file */
  fclose((FILE *)pvHandle);
}

int tim_stdio_seek(void *pvHandle, int nOffset, tim_seek_mode_t SeekMode) {
  int nSeekMode = -1;
  int nCurPos;

  /* Use fseek() to seek in the file, depending on the SeekMode */
  switch (SeekMode) {
    case TIM_SM_ABS:
      nSeekMode = SEEK_SET;
      break; /* Absolute seeking */
    case TIM_SM_REL:
      nSeekMode = SEEK_CUR;
      break; /* Seek relatively to current pos */
    case TIM_SM_END:
      nSeekMode = SEEK_END;
      break; /* Seek relatively to end */
  }

  /* No prober SeekMode? */
  if (nSeekMode < 0)
    return TIM_RV_ERR_INVALID_PARAM;

  /* Perform the actual seek using fseek() */
  if (fseek((FILE *)pvHandle, nOffset, nSeekMode))
    return TIM_RV_ERR_COULD_NOT_PERFORM;

  /* Get current position using ftell() */
  nCurPos = ftell((FILE *)pvHandle);
  if (nCurPos < 0)
    return TIM_RV_ERR_COULD_NOT_PERFORM;

  /* Return current position */
  return nCurPos;
}

int tim_stdio_read(void *pvHandle, void *pvBuf, int nSize) {
  /* Fill buffer if posible (using fread()) */
  return (int)fread(pvBuf, 1, nSize, (FILE *)pvHandle);
}

int tim_stdio_write(void *pvHandle, void *pvBuf, int nSize) {
  /* Write buffer contents if posible (using fwrite()) */
  return (int)fwrite(pvBuf, 1, nSize, (FILE *)pvHandle);
}

int tim_stdio_eof(void *pvHandle) {
  /* feof() */
  return feof((FILE *)pvHandle);
}

/*==============================================================================
Globals
==============================================================================*/

tim_io_t g_IO_Stdio = { tim_stdio_open, tim_stdio_close, tim_stdio_seek,
                        tim_stdio_read, tim_stdio_write, tim_stdio_eof };
