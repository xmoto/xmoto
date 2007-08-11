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

#ifndef __HIGHPRECISIONTIMER_H__
#define __HIGHPRECISIONTIMER_H__

#include <string>

#include "../BuildConfig.h"

class HighPrecisionTimer {
  public:
    /* Types */
    struct TimeCheck {
      char cWhere[64];
      double fTime;
      double fAbsTime;
    };
  
    /* Methods */
    #if defined(PROFILE_MAIN_LOOP)
      static void reset(void);        
      static void checkTime(const std::string &Where);
      static int numTimeChecks(void);
      static HighPrecisionTimer::TimeCheck *getTimeCheck(int nIdx);
    #else
      /* Dummy implementation */
      static void reset(void) {}
      static void checkTime(const std::string &Where) {}
      static int numTimeChecks(void) {return 0;}
      static HighPrecisionTimer::TimeCheck *getTimeCheck(int nIdx) {return NULL;}
    #endif
};

#endif

