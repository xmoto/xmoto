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

#ifndef __VMATH_H__
#define __VMATH_H__

#include "../VCommon.h"
#include "VExcept.h"

/*===========================================================================
  Vectors
  ===========================================================================*/
#define VECTOR_COMPARE_EPSILON    0.001f
#define PI 3.14159265

template <typename _T>
class Vector2 {
  public:
  Vector2<_T>() {x=y=0;}
  Vector2<_T>(_T ix,_T iy) : x(ix),y(iy) {}
  Vector2<_T>(const Vector2<_T> &i) : x(i.x),y(i.y) {}
  
  /* Overloaded operators */
  inline Vector2<_T> operator + (const Vector2<_T> &v) const {
    return Vector2<_T>(x+v.x, y+v.y);
  }
  
  inline Vector2<_T> operator - (const Vector2<_T> &v) const {
    return Vector2<_T>(x-v.x, y-v.y);
  }
  
  inline Vector2<_T> operator - () const {
    return Vector2<_T>(-x,-y);
  }
  
  inline Vector2<_T> operator * (const Vector2<_T> &v) const {
    return Vector2<_T>(x*v.x,y*v.y);
  }
  
  inline Vector2<_T> operator * (_T v) const {
    return Vector2<_T>(x*v,y*v);
  }
  
  inline Vector2<_T> operator / (const Vector2<_T> &v) const {
    return Vector2<_T>(x/v.x,y/v.y);
  }
  
  inline Vector2<_T> operator / (_T v) const {
    return Vector2<_T>(x/v,y/v);
  }
  
  inline Vector2<_T> operator += (const Vector2<_T> &v) {
    x += v.x; y += v.y;
    return Vector2<_T>(x,y);
  }
  
  inline Vector2<_T> operator -= (const Vector2<_T> &v) {
    x -= v.x; y -= v.y;
    return Vector2<_T>(x,y);
  }
  
  inline Vector2<_T> operator *= (const Vector2<_T> &v) {
    x *= v.x; y *= v.y;
    return Vector2<_T>(x,y);
  }
  
  inline Vector2<_T> operator *= (_T v) {
    x *= v; y *= v;
    return Vector2<_T>(x,y);
  }
  
  inline Vector2<_T> operator /= (const Vector2<_T> &v) {
    x /= v.x; y /= v.y;
    return Vector2<_T>(x,y);
  }
  
  inline Vector2<_T> operator /= (_T v) {
    x /= v; y /= v;
    return Vector2<_T>(x,y);
  }
  
  inline Vector2<_T> operator = (const Vector2<_T> &v) {
    return Vector2<_T>(x = v.x,y = v.y);
  }
  
  inline _T &operator [] (int i) {
    switch(i) {
      case 0: return x;
      case 1: return y;
    }
    throw Exception("invalid vector subscript");
  }
  
  inline bool operator == (const Vector2<_T> &v) const {
    if(v.x == x && v.y == y) return true;
    return false;
  }
  
  inline bool operator != (const Vector2<_T> &v) const {
    if(v.x != x || v.y != y) return true;
    return false;
  }
  
  /* Methods */
  inline _T dot(const Vector2<_T> &v) const {
    return x*v.x + y*v.y;
  }
  
  inline bool almostEqual(const Vector2<_T> &v) const {
    if(fabs(v.x - x) >= VECTOR_COMPARE_EPSILON) return false;
    if(fabs(v.y - y) >= VECTOR_COMPARE_EPSILON) return false;
    return true;
  }
  
  inline _T length(void) const {
    return sqrt(x*x + y*y);
  }
  
  inline _T normalize(void) {
    _T v=length();
    if(v == 0) throw Exception("null vector normalize");
    x /= v; y /= v;
    return v;
  }
  
  inline void rotateXY(_T deg) {            
    _T rads=deg*((_T)PI/180.0f);
    _T xx=x*cos(rads)+y*sin(rads);
    y=y*cos(rads)+x*sin(rads);
    x=xx;
  }
  
  /**
   * @return the angle in degrees
   **/
  inline _T angle(){
    _T v = atan2(y,x) * 180 / PI;
    return v;
  }
  
  /* Public data */
  _T x,y;                 
};

template <typename _T>
class Vector3 {
 public:
  Vector3<_T>() {x=y=z=0;}
  Vector3<_T>(_T ix,_T iy,_T iz) : x(ix),y(iy),z(iz) {}
  Vector3<_T>(const Vector3<_T> &i) : x(i.x),y(i.y),z(i.z) {}
  
  /* Overloaded operators */
  inline Vector3<_T> operator + (Vector3<_T> &v) const {
    return Vector3<_T>(x+v.x, y+v.y, z+v.z);
  }
  
  inline Vector3<_T> operator - (const Vector3<_T> &v) const {
    return Vector3<_T>(x-v.x, y-v.y, z-v.z);
  }

  inline Vector3<_T> operator - () const {
    return Vector3<_T>(-x,-y,-z);
  }

  inline Vector3<_T> operator * (const Vector3<_T> &v) const {
    return Vector3<_T>(x*v.x,y*v.y,z*v.z);
  }

  inline Vector3<_T> operator * (_T v) const {
    return Vector3<_T>(x*v,y*v,z*v);
  }

  inline Vector3<_T> operator / (const Vector3<_T> &v) const {
    return Vector3<_T>(x/v.x,y/v.y,z/v.z);
  }

  inline Vector3<_T> operator / (_T v) const {
    return Vector3<_T>(x/v,y/v,z/v);
  }

  inline Vector3<_T> operator += (const Vector3<_T> &v) {
    x += v.x; y += v.y; z += v.z;
    return Vector3<_T>(x,y,z);
  }

  inline Vector3<_T> operator -= (const Vector3<_T> &v) {
    x -= v.x; y -= v.y; z -= v.z;
    return Vector3<_T>(x,y,z);
  }

  inline Vector3<_T> operator *= (const Vector3<_T> &v) {
    x *= v.x; y *= v.y; z *= v.z;
    return Vector3<_T>(x,y,z);
  }

  inline Vector3<_T> operator *= (_T v) {
    x *= v; y *= v; z *= v;
    return Vector3<_T>(x,y,z);
  }

  inline Vector3<_T> operator /= (const Vector3<_T> &v) {
    x /= v.x; y /= v.y; z /= v.z;
    return Vector3<_T>(x,y,z);
  }

  inline Vector3<_T> operator /= (_T v) {
    x /= v; y /= v; z /= v;
    return Vector3<_T>(x,y,z);
  }
      
  inline Vector3<_T> operator = (const Vector3<_T> &v) {
    return Vector3<_T>(x = v.x,y = v.y,z = v.z);
  }
        
  inline _T &operator [] (int i) {
    switch(i) {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    }
    throw Exception("invalid vector subscript");
  }
      
  inline bool operator == (const Vector3<_T> &v) const {
    if(v.x == x && v.y == y && v.z == z) return true;
    return false;
  }
      
  inline bool operator != (const Vector3<_T> &v) const {
    if(v.x != x || v.y != y || v.z != z) return true;
    return false;
  }
      
  /* Methods */
  inline _T dot(const Vector3<_T> &v) const {
    return x*v.x + y*v.y + z*v.z;
  }
      
  inline Vector3<_T> cross(const Vector3<_T> &v) const {
    return Vector3<_T>(y*v.z-z*v.y,z*v.x-x*v.z,x*v.y-y*v.x);
  }
      
  inline bool almostEqual(const Vector3<_T> &v) const {
    if(fabs(v.x - x) >= VECTOR_COMPARE_EPSILON) return false;
    if(fabs(v.y - y) >= VECTOR_COMPARE_EPSILON) return false;
    if(fabs(v.z - z) >= VECTOR_COMPARE_EPSILON) return false;
    return true;
  }
      
  inline _T length(void) const {
    return sqrt(x*x + y*y + z*z);
  }
      
  inline _T normalize(void) {
    _T v=length();
    if(v == 0) throw Exception("null vector normalize");
    x /= v; y /= v; z /= v;
    return v;
  }
      
  inline void rotateXYZ(_T xrotate,_T yrotate,_T zrotate) {
    _T pohf=(_T)PI/180.0f;
    _T tempx=x*cos(yrotate*pohf)-z*sin(yrotate*pohf);
    _T tempz=x*sin(yrotate*pohf)+z*cos(yrotate*pohf);
    z=tempz*cos(xrotate*pohf)-y*sin(xrotate*pohf);
    _T tempy=tempz*sin(xrotate*pohf)+y*cos(xrotate*pohf);
    x=tempx*cos(zrotate*pohf)+tempy*sin(zrotate*pohf);
    y=tempy*cos(zrotate*pohf)+tempx*sin(zrotate*pohf);
  }
    
  /* Public data */
  _T x,y,z;                 
};
  
/* Type shortcuts */
typedef Vector3<float> Vector3f;
typedef Vector3<double> Vector3d;
typedef Vector2<float> Vector2f;
typedef Vector2<double> Vector2d; 
typedef Vector2<int> Vector2i;

/*===========================================================================
  Extra functions -- all related to the Vector2f type... I nice thing to do 
  would be to templating this as well.
  ===========================================================================*/
int intersectLineCircle2f(const Vector2f &Cp,float Cr,const Vector2f &A0,const Vector2f &A1,
                          Vector2f &Res1,Vector2f &Res2);
int intersectLineLine2f(const Vector2f &A0,const Vector2f &A1,const Vector2f &B0,
                        const Vector2f &B1,Vector2f &Res);
bool circleTouchCircle2f(const Vector2f &Cp1,float Cr1,const Vector2f &Cp2,float Cr2);
  
class AABB {
 public:
  AABB();
  void reset();

  Vector2f& getBMin() {return BMin;}
  Vector2f& getBMax() {return BMax;}

  bool lineTouchAABB2f(const Vector2f &P0, const Vector2f &P1);
  bool circleTouchAABB2f(const Vector2f &Cp, const float Cr);
  bool pointTouchAABB2f(const Vector2f &P);
  bool AABBTouchAABB2f(const Vector2f &BMin1, const Vector2f &BMax1);
  void addPointToAABB2f(const Vector2f &Point);
  void addPointToAABB2f(const float x, const float y);

 private:
  Vector2f BMin, BMax;
};

float randomNum(float fMin,float fMax);
int randomIntNum(int nMin, int nMax);

#endif

