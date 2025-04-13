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

#if !defined(WIN32)
#error file is win32 only
#endif

#include "HashTbl.h"
#include "HighPrecisionTimer.h"
#include <string.h>
#include <windows.h>

#if defined(PROFILE_MAIN_LOOP)

/* WARNING: This file is very non-portable; don't expect it to run on exotic
   windows platforms,
            only x86 */

#define MAX_TIME_CHECKS 32
#define MOVING_AVG_LEN \
  250 /* times are averaged out over this number of samples */

/* Extra types */
struct TimeEntry {
  double fBuf[MOVING_AVG_LEN];
  int nIdx;
};

/* Globals */
static __int64 g_nPerfFreq = 0;
static double g_fResetTime = 0;

static int g_nNumTimeChecks = 0;
static HighPrecisionTimer::TimeCheck g_TimeChecks[MAX_TIME_CHECKS];

static HashTbl<TimeEntry *> g_TimeTbl;

double _GetRawQPC(void) {
  /* Query counter */
  __int64 nPC;
  QueryPerformanceCounter((LARGE_INTEGER *)&nPC);
  if (g_nPerfFreq != 0)
    return (((double)nPC) * 1000000.0f) / (double)g_nPerfFreq;

  return 0;
}

void _InitQPC(void) {
  /* Initialized? */
  if (g_nPerfFreq == 0) {
    /* No, get performance counter frequency */
    QueryPerformanceFrequency((LARGE_INTEGER *)&g_nPerfFreq);
  }
}

void HighPrecisionTimer::reset(void) {
  /* Reset timer */
  _InitQPC();
  g_fResetTime = _GetRawQPC();
  g_nNumTimeChecks = 0;
}

void HighPrecisionTimer::checkTime(const std::string &Where) {
  _InitQPC();
  double fTime = _GetRawQPC();

  /* Room for one more time check? */
  if (g_nNumTimeChecks < MAX_TIME_CHECKS) {
    /* Yup, set it up */
    TimeCheck *pTc = &g_TimeChecks[g_nNumTimeChecks];
    strncpy(pTc->cWhere, Where.c_str(), sizeof(pTc->cWhere) - 1);
    pTc->fAbsTime = fTime;
    if (g_nNumTimeChecks > 0) {
      pTc->fTime = pTc->fAbsTime - g_TimeChecks[g_nNumTimeChecks - 1].fAbsTime;
    } else {
      pTc->fTime = pTc->fAbsTime - g_fResetTime;
    }
    g_nNumTimeChecks++;

    /* Look it up in table */
    try {
      TimeEntry *p = g_TimeTbl.getEntry(Where);
      p->fBuf[p->nIdx] = pTc->fTime;
      p->nIdx++;
      p->nIdx %= MOVING_AVG_LEN; /* make sure we stay in our ring-buffer */

      /* Calculate average value in buffer */
      pTc->fTime = 0;
      for (int i = 0; i < MOVING_AVG_LEN; i++)
        pTc->fTime += p->fBuf[i] / (double)MOVING_AVG_LEN;
    } catch (Exception &) {
      /* Not in table, add it */
      TimeEntry *p = new TimeEntry;
      for (int i = 0; i < MOVING_AVG_LEN; i++)
        p->fBuf[i] = pTc->fTime;
      p->nIdx = 0;
      g_TimeTbl.addEntry(Where, p);
    }
  }
}

int HighPrecisionTimer::numTimeChecks(void) {
  return g_nNumTimeChecks;
}

HighPrecisionTimer::TimeCheck *HighPrecisionTimer::getTimeCheck(int nIdx) {
  if (nIdx < 0 || nIdx >= g_nNumTimeChecks)
    return NULL;
  return &g_TimeChecks[nIdx];
}

#endif
