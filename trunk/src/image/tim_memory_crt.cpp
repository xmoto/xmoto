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

/* tim_memory_crt.cpp
 * 
 * CRT memory driver.
 */

#if defined(__FreeBSD__) || (defined(__APPLE__) && defined(__MACH__))
  #include <stdlib.h>
#else
  #include <malloc.h>
#endif

#include "tim.h"

/*==============================================================================
Memory system - Standard CRT
==============================================================================*/

void *tim_crt_alloc(int nSize) {
	/* Allocate memory using malloc() */
	return malloc(nSize);
}

void *tim_crt_realloc(void *pvBlock,int nSize) {
	/* Re-allocate memory using realloc() */
	return realloc(pvBlock,nSize);
}

void tim_crt_free(void *pvBlock) {
	/* Free memory using free() */
	free(pvBlock);
}

/*==============================================================================
Globals
==============================================================================*/

tim_memory_t g_Memory_CRT={
	tim_crt_alloc,tim_crt_realloc,tim_crt_free
};
