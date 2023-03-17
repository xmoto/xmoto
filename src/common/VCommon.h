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

#ifndef __VCOMMON_H__
#define __VCOMMON_H__

/* Load in configuration */
#include "BuildConfig.h"

#ifdef WIN32
#include <windows.h>
#endif

/* Some places #define _T, which we want for a template parameter */
#undef _T

// used a comparator in sdt::map
#include <string.h> // for strcmp
struct ltstr {
  bool operator()(const char *s1, const char *s2) const {
    return strcmp(s1, s2) < 0;
  }
};

#endif
