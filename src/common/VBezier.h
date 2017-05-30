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

#ifndef __VBEZIER_H__
#define __VBEZIER_H__

#include "VCommon.h"
#include "helpers/VMath.h"

/*===========================================================================
Cubic bezier curve
===========================================================================*/
class BezierCurve {
public:
  BezierCurve() {}

  /* Methods */
  Vector3f step(float t);
  float length(void);

  /* Data interface */
  Vector3f &getP(int n) {
    if (n < 0 || n > 3)
      throw Exception("(1) cubic bezier point index out of bounds");
    return m_P[n];
  }
  void setP(int n, const Vector3f &p) {
    if (n < 0 || n > 3)
      throw Exception("(2) cubic bezier point index out of bounds");
    m_P[n] = p;
  }

private:
  /* Helper functions */
  float _SectionLength(float t1,
                       float t2,
                       unsigned int nSteps,
                       float fGuestimate);

  /* Data */
  Vector3f m_P[4]; /* Coefficients */
};

/*===========================================================================
Bezier shape point
===========================================================================*/
struct BezierShapePoint {
  Vector3f Pos; /* Position of point */
  Vector3f c1, c2; /* Control points */
};

/*===========================================================================
Bezier shape
===========================================================================*/
class BezierShape {
public:
  BezierShape() {
    m_pPoints = NULL;
    m_pCurves = NULL;
    m_pfCurveLengths = NULL;
    m_nMaxPoints = m_nNumPoints = 0;
    m_fLength = 0;
  }

  /* Methods */
  void startCreation(int nMaxPoints);
  void finishCreation();
  void addPoint(BezierShapePoint &p);
  float length();
  Vector3f step(float t);

  /* Data interface */
  int getNumPoints() { return m_nNumPoints; }
  BezierShapePoint &getPoint(unsigned int n) {
    if (n >= m_nNumPoints)
      throw Exception("bezier shape point index out of bounds");
    return m_pPoints[n];
  }
  BezierCurve &getCurve(unsigned int n) {
    if (n >= m_nNumPoints)
      throw Exception("bezier shape curve index out of bounds");
    return m_pCurves[n];
  }

private:
  /* Helper functions */
  float _UpdateLength();

  /* Data */
  BezierShapePoint *m_pPoints;
  BezierCurve *m_pCurves;
  float *m_pfCurveLengths;
  unsigned int m_nMaxPoints, m_nNumPoints;
  float m_fLength;
};

#endif
