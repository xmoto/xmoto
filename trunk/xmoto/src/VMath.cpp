/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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
 *  Math library. 
 */
#include <stdlib.h>
#include "VMath.h"

namespace vapp {

  /*===========================================================================
  Random numbers
  ===========================================================================*/
  float randomNum(float fMin,float fMax) {
    return fMin + (fMax-fMin) * ((float)rand() / (float)RAND_MAX);
  }

  /*===========================================================================
  AABB updating
  ===========================================================================*/
  void addPointToAABB2f(Vector2f &BMin,Vector2f &BMax,const Vector2f &Point) {
    if(Point.x < BMin.x) BMin.x = Point.x;
    if(Point.y < BMin.y) BMin.y = Point.y;
    if(Point.x > BMax.x) BMax.x = Point.x;
    if(Point.y > BMax.y) BMax.y = Point.y;    
  }
  
  /*===========================================================================
  Vector2f intersections and stuff
  ===========================================================================*/
  int intersectLineCircle2f(const Vector2f &Cp,float Cr,const Vector2f &A0,
                            const Vector2f &A1,Vector2f &Res1,Vector2f &Res2) {
    /* Slooow... naiiiive.... optimize! */
    Vector2f rd = A1-A0;
    if(A1.almostEqual(A0)) return 0;
    float rl = rd.normalize();
		Vector2f v = A0 - Cp;
		float b = -v.dot(rd);
		float det = (b * b) - v.dot(v) + Cr*Cr;
		if(det<0 && det>-0.0001f) det=0;
		if(det >= 0) {
			det = sqrt(det);
			float i1 = b - det;
			float i2 = b + det;
			
			if(i1>-0.0001f && i1<rl+0.0001f && i2>-0.0001f && i2<rl+0.0001f) {
        Res1 = A0 + rd*i1;
        Res2 = A0 + rd*i2;
        return 2;
			}
			else if(i1>-0.0001f && i1<rl+0.0001f) {
        Res1 = A0 + rd*i1;
        return 1;			  
			}
			else if(i2>-0.0001f && i2<rl+0.0001f) {
        Res1 = A0 + rd*i2;
        return 1;			  
			}
		}
    return 0;
  }
  
  int intersectLineLine2f(const Vector2f &A0, const Vector2f &A1,
			  const Vector2f &B0, const Vector2f &B1,
			  Vector2f &Res) {
    if(A0.almostEqual(A1) || B0.almostEqual(B1)) return 0;
                          
    Vector2f P = B0, N = Vector2f( -(B1.y-B0.y), (B1.x-B0.x) );
    float den = N.dot(A1-A0);
    if(fabs(den) < 0.0001f) return 0;
    float t = (P.dot(N) - A0.dot(N)) / den;
    if(t<0.0f || t>1.0f) return 0;
    
    P = A0;
    N = Vector2f( -(A1.y-A0.y), (A1.x-A0.x) );
    den = N.dot(B1-B0);
    if(fabs(den) < 0.0001f) return 0;
    t = (P.dot(N) - B0.dot(N)) / den;
    if(t<0.0f || t>1.0f) return 0;
        
    Res = B0 + (B1-B0)*t;        
        
    return 1;
  }  
  
  /*===========================================================================
  AABB touching
  ===========================================================================*/
  bool lineTouchAABB2f(const Vector2f &P0,const Vector2f &P1,const Vector2f &BMin,const Vector2f &BMax) {
    if(pointTouchAABB2f(P0,BMin,BMax)) return true;
    if(pointTouchAABB2f(P1,BMin,BMax)) return true;

		if(P0.x < BMin.x && P1.x < BMin.x) return false;
		if(P0.y < BMin.y && P1.y < BMin.y) return false;
		if(P0.x > BMax.x && P1.x > BMax.x) return false;
		if(P0.y > BMax.y && P1.y > BMax.y) return false;

		Vector2f v = P1 - P0;
		float x,y;
		
		if(v.y != 0.0f) {
			/* Top */
			y = BMin.y;
			x = P0.x + v.x*(y - P0.y) / v.y;
			if(x >= BMin.x && x < BMax.x) return true;

			/* Bottom */
			y = BMax.y;
			x = P0.x + v.x*(y - P0.y) / v.y;
			if(x >= BMin.x && x < BMax.x) return true;
		}

		if(v.x != 0.0f) {
			/* Left */
			x = BMin.x;
			y = P0.y + v.y*(x - P0.x) / v.x;
			if(y >= BMin.y && y < BMax.y) return true;

			/* Right */
			x = BMax.x;
			y = P0.y + v.y*(x - P0.x) / v.x;
			if(y >= BMin.y && y < BMax.y) return true;
		}
		
		/* No touch */
    return false;
  }
  
  bool circleTouchAABB2f(const Vector2f &Cp,float Cr,const Vector2f &BMin,const Vector2f &BMax) {
		/* TODO: do this in a more precise way */
		return AABBTouchAABB2f(Cp - Vector2f(Cr,Cr),Cp + Vector2f(Cr,Cr),BMin,BMax);
  }
  
  bool pointTouchAABB2f(const Vector2f &P,const Vector2f &BMin,const Vector2f &BMax) {
    if(P.x >= BMin.x && P.y >= BMin.y && P.x <= BMax.x && P.y <= BMax.y)
      return true;
    return false;
  }
  
  bool AABBTouchAABB2f(const Vector2f &BMin1,const Vector2f &BMax1,const Vector2f &BMin2,const Vector2f &BMax2) {
    Vector2f FMin( BMin1.x<BMin2.x ? BMin1.x:BMin2.x, 
                   BMin1.y<BMin2.y ? BMin1.y:BMin2.y );
    Vector2f FMax( BMax1.x>BMax2.x ? BMax1.x:BMax2.x, 
                   BMax1.y>BMax2.y ? BMax1.y:BMax2.y );
    if(FMax.x-FMin.x < 0.0001f + (BMax1.x-BMin1.x)+(BMax2.x-BMin2.x) &&
       FMax.y-FMin.y < 0.0001f + (BMax1.y-BMin1.y)+(BMax2.y-BMin2.y)) return true;
    return false;
  }
  
  bool circleTouchCircle2f(const Vector2f &Cp1,float Cr1,const Vector2f &Cp2,float Cr2) {
    /* Trivial test :) */
    float d = (Cp2 - Cp1).length();
    if(d < Cr1+Cr2) return true;
    return false;
  }

}

