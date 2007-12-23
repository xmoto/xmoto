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

/* 
 *  Generic cubic bezier code fun.
 */

#include "VBezier.h"

#define BEZIER_LENGTH_THRESHOLD   0.01f
#define BEZIER_LENGTH_MAX_STEPS   100
    
  /*===========================================================================
  Calculate point on cubic bezier curve
  ===========================================================================*/
  Vector3f BezierCurve::step(float t) {
    float b = 1.0f - t;
    return Vector3f(m_P[0].x*t*t*t + m_P[1].x*3*t*t*b + m_P[2].x*3*t*b*b + m_P[3].x*b*b*b,
                    m_P[0].y*t*t*t + m_P[1].y*3*t*t*b + m_P[2].y*3*t*b*b + m_P[3].y*b*b*b,
                    m_P[0].z*t*t*t + m_P[1].z*3*t*t*b + m_P[2].z*3*t*b*b + m_P[3].z*b*b*b);
  }
  
  /*===========================================================================
  Approximate full length of the curve
  ===========================================================================*/
  float BezierCurve::length() {
    return _SectionLength(0.0f,1.0f,1,0.0f);
  }
    
  /*===========================================================================
  Calculate length recurvively 
  ===========================================================================*/  
  float BezierCurve::_SectionLength(float t1, float t2,
				    unsigned int nSteps,
				    float fGuestimate) {
    Vector3f Prev;
    float fLength=0.0f;
    
    /* Don't recurse too much */
    if(nSteps > BEZIER_LENGTH_MAX_STEPS)
      return fGuestimate;
    
    /* Step through it */
    for(unsigned int i=0; i<=nSteps; i++) {
      Vector3f P = step(t1+((t2-t1)*(float)i)/(float)nSteps);
    
      if(i!=0) {
        /* Don't do this for first step */
        fLength += (P-Prev).length();
      }
      
      Prev = P;
    }
    
    /* How much better than last guestimate? */
    if(fLength == fGuestimate ||
       (fGuestimate>0.0f && ((fLength - fGuestimate) / fGuestimate) < BEZIER_LENGTH_THRESHOLD)) {
      /* It's "good enough" now :) */
      return fLength;
    }       
    
    /* Return length */
    return _SectionLength(0.0f, 1.0f, nSteps+1, fLength);
  }

  /*===========================================================================
  Start creation of bezier shape
  ===========================================================================*/  
  void BezierShape::startCreation(int nMaxPoints) {
    /* Allocate room for shape */
    m_pPoints = new BezierShapePoint[nMaxPoints];
    m_pCurves = new BezierCurve[nMaxPoints];
    m_pfCurveLengths = new float[nMaxPoints];
    m_nMaxPoints = nMaxPoints;
    m_nNumPoints = 0;
  }
  
  /*===========================================================================
  Call this when all points are added
  ===========================================================================*/  
  void BezierShape::finishCreation() {
    /* Okay, create shape curves */
    for(unsigned int i=0;i<m_nNumPoints;i++) {
      /* Next point? */
      int j;
      if(i+1 == m_nNumPoints)
	j=0;
      else
	j=i+1;
    
      /* Do this curve */
      m_pCurves[i].setP(0,m_pPoints[i].Pos);
      m_pCurves[i].setP(1,m_pPoints[i].c2);
      m_pCurves[i].setP(2,m_pPoints[i+1].c1);
      m_pCurves[i].setP(3,m_pPoints[i+1].Pos);
      
      /* Calculate its length */
      m_pfCurveLengths[i] = m_pCurves[i].length();
    }
    
    _UpdateLength();
  }
  
  /*===========================================================================
  Add point to shape NOT finished with finishCreation()
  ===========================================================================*/  
  void BezierShape::addPoint(BezierShapePoint &p) {
    if(m_nNumPoints >= m_nMaxPoints)
      throw Exception("overfull bezier shape");
    m_pPoints[m_nNumPoints].c1 = p.c1;
    m_pPoints[m_nNumPoints].c2 = p.c2;
    m_pPoints[m_nNumPoints].Pos = p.Pos;
    m_nNumPoints++;
  }
  
  /*===========================================================================  
  Sum up the length of the sections
  ===========================================================================*/  
  float BezierShape::_UpdateLength() {
    float fLength = 0.0f;
    for(unsigned int i=0; i<m_nNumPoints; i++) {
      fLength += m_pfCurveLengths[i];
    }
    m_fLength = fLength;
    return fLength;
  }

  /*===========================================================================  
  Return length
  ===========================================================================*/  
  float BezierShape::length() {
    return m_fLength;
  }  
  
  /*===========================================================================  
  Shape stepping
  ===========================================================================*/  
  Vector3f BezierShape::step(float t) {
    /* Calculate distance to travel */
    //float fDist = t * length();    
    
    /* Go through the curves, look for the one in question */
    //float ta = t, da = 0.0f;
    //for(unsigned int i=0;i<m_nNumPoints;i++) {
      //float t0 = da,t1 = da + m_pfCurveLengths[i];
      //if(fDist >= t0 && fDist < t1) {
        /* Grats, you've found your curve section! */
        //float rt = ta * (m_pfCurveLengths[i] / length());
      //}
      //ta -= m_pfCurveLengths[i] / length();
      //}
    
    return m_pCurves[0].step(0.0f);
  }
