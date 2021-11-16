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
 *  Math library.
 */
#include "VMath.h"
#include "../helpers/Log.h"

#define BIG_VALUE 99999999.0f

/*===========================================================================
  Simple Interpolation
  ===========================================================================*/
float SimpleInterpolate(float i_old, float i_new, float i_smooth) {
  return i_old + (i_new - i_old) * (1.0 / i_smooth);
}

/*===========================================================================
  Bounding Circle
  ===========================================================================*/

BoundingCircle::BoundingCircle() {
  reset();
  m_aabb.reset();
}

void BoundingCircle::reset() {
  m_center.x = 0.0;
  m_center.y = 0.0;
  m_radius = 0.0;

  m_xmin = m_ymin = BIG_VALUE;
  m_xmax = m_ymax = -BIG_VALUE;
}

void BoundingCircle::init(Vector2f &center, float radius) {
  m_center = center;
  m_radius = radius;
  calculateBoundingBox();
}

bool BoundingCircle::lineTouch(const Vector2f &P0, const Vector2f &P1) {
  Vector2f res1, res2;
  return (intersectLineCircle2f(m_center, m_radius, P0, P1, res1, res2) != 0);
}

bool BoundingCircle::circleTouch(const Vector2f &Cp, const float Cr) {
  return circleTouchCircle2f(m_center, m_radius, Cp, Cr);
}

bool BoundingCircle::pointTouch(const Vector2f &point) {
  float diffX = point.x - m_center.x;
  float diffY = point.y - m_center.y;
  return (std::fabs(diffX) < m_radius) && (std::fabs(diffY) < m_radius);
}

void BoundingCircle::translate(const float x, const float y) {
  m_center.x += x;
  m_center.y += y;
  m_aabb.translateAABB(x, y);
}

void BoundingCircle::addPointToCircle(const float x, const float y) {
  if (x < m_xmin)
    m_xmin = x;
  if (x > m_xmax)
    m_xmax = x;
  if (y < m_ymin)
    m_ymin = y;
  if (y > m_ymax)
    m_ymax = y;
}

void BoundingCircle::calculateBoundingCircle() {
  Vector2f center;
  center.x = (m_xmin + m_xmax) / 2.0;
  center.y = (m_ymin + m_ymax) / 2.0;

  float tx = (m_xmax - m_xmin) / 2.0;
  float ty = (m_ymax - m_ymin) / 2.0;

  float radius = tx + ty - std::min(tx, ty) / 2.0;

  init(center, radius);
}

void BoundingCircle::calculateBoundingBox() {
  m_aabb.reset();
  m_aabb.addPointToAABB2f(m_center.x + m_radius, m_center.y + m_radius);
  m_aabb.addPointToAABB2f(m_center.x - m_radius, m_center.y - m_radius);
}

/*===========================================================================
  AABB
  ===========================================================================*/
AABB::AABB() {
  reset();
}

void AABB::reset() {
  BMin.x = BMin.y = BIG_VALUE;
  BMax.x = BMax.y = -BIG_VALUE;
}

void AABB::addPointToAABB2f(const Vector2f &Point) {
  if (Point.x < BMin.x)
    BMin.x = Point.x;
  if (Point.y < BMin.y)
    BMin.y = Point.y;
  if (Point.x > BMax.x)
    BMax.x = Point.x;
  if (Point.y > BMax.y)
    BMax.y = Point.y;
}

bool AABB::lineTouchBorder(const Vector2f &P0,
                           const Vector2f &P1,
                           Vector2f &o_touchOnePoint,
                           AABBSide &o_side) {
  if (intersectLineLine2f(
        P0, P1, BMin, Vector2f(BMin.x, BMax.y), o_touchOnePoint) != 0) {
    o_side = AABB_LEFT;
    return true;
  }

  if (intersectLineLine2f(
        P0, P1, Vector2f(BMin.x, BMax.y), BMax, o_touchOnePoint) != 0) {
    o_side = AABB_TOP;
    return true;
  }

  if (intersectLineLine2f(
        P0, P1, BMax, Vector2f(BMax.x, BMin.y), o_touchOnePoint) != 0) {
    o_side = AABB_RIGHT;
    return true;
  }

  if (intersectLineLine2f(
        P0, P1, Vector2f(BMax.x, BMin.y), BMin, o_touchOnePoint) != 0) {
    o_side = AABB_BOTTOM;
    return true;
  }

  return false;
}

bool AABB::lineTouchAABB2f(const Vector2f &P0, const Vector2f &P1) {
  if (P0.x < BMin.x && P1.x < BMin.x)
    return false;
  if (P0.y < BMin.y && P1.y < BMin.y)
    return false;
  if (P0.x > BMax.x && P1.x > BMax.x)
    return false;
  if (P0.y > BMax.y && P1.y > BMax.y)
    return false;

  Vector2f v = P1 - P0;
  float x, y;

  if (v.y != 0.0f) {
    /* Top */
    y = BMin.y;
    x = P0.x + v.x * (y - P0.y) / v.y;
    if (x >= BMin.x && x < BMax.x) {
      return true;
    }

    /* Bottom */
    y = BMax.y;
    x = P0.x + v.x * (y - P0.y) / v.y;
    if (x >= BMin.x && x < BMax.x) {
      return true;
    }
  }

  if (v.x != 0.0f) {
    /* Left */
    x = BMin.x;
    y = P0.y + v.y * (x - P0.x) / v.x;
    if (y >= BMin.y && y < BMax.y) {
      return true;
    }

    /* Right */
    x = BMax.x;
    y = P0.y + v.y * (x - P0.x) / v.x;
    if (y >= BMin.y && y < BMax.y) {
      return true;
    }
  }

  /* No touch */
  return false;
}

bool AABB::circleTouchAABB2f(const Vector2f &Cp, float Cr) {
  /* TODO: do this in a more precise way */
  return AABBTouchAABB2f(Cp - Vector2f(Cr, Cr), Cp + Vector2f(Cr, Cr));
}

bool AABB::pointTouchAABB2f(const Vector2f &P) {
  return (P.x >= BMin.x && P.y >= BMin.y && P.x <= BMax.x && P.y <= BMax.y);
}

bool AABB::AABBTouchAABB2f(const Vector2f &BMin1, const Vector2f &BMax1) {
  Vector2f FMin(BMin1.x < BMin.x ? BMin1.x : BMin.x,
                BMin1.y < BMin.y ? BMin1.y : BMin.y);
  Vector2f FMax(BMax1.x > BMax.x ? BMax1.x : BMax.x,
                BMax1.y > BMax.y ? BMax1.y : BMax.y);

  return (FMax.x - FMin.x < 0.0001f + (BMax1.x - BMin1.x) + (BMax.x - BMin.x) &&
          FMax.y - FMin.y < 0.0001f + (BMax1.y - BMin1.y) + (BMax.y - BMin.y));
}

/*===========================================================================
  Vector2f intersections and stuff
  ===========================================================================*/
int intersectLineCircle2f(const Vector2f &Cp,
                          float Cr,
                          const Vector2f &A0,
                          const Vector2f &A1,
                          Vector2f &Res1,
                          Vector2f &Res2) {
  /* first, aabb check */
  Vector2f line_0 = A1;
  Vector2f line_1 = A0;
  Vector2f circle_0 = Vector2f(Cp.x - Cr, Cp.y - Cr);
  Vector2f circle_1 = Vector2f(Cp.x + Cr, Cp.y + Cr);
  bool lineInversed = true;

  if (A0.x < A1.x) {
    line_0 = A0;
    line_1 = A1;
    lineInversed = false;
  }

  if (line_1.x < circle_0.x || line_0.x > circle_1.x)
    return 0;

  if (A0.y < A1.y) {
    if (lineInversed) {
      line_0 = A0;
      line_1 = A1;
    }
  } else {
    if (!lineInversed) {
      line_0 = A1;
      line_1 = A0;
    }
  }

  if (line_1.y < circle_0.y || line_0.y > circle_1.y)
    return 0;

  /* then the slow naive one */
  /* Slooow... naiiiive.... optimize! */
  Vector2f rd = A1 - A0;
  if (A1.almostEqual(A0))
    return 0;

  /*
          IMPORTANT: keep double for precision !!! otherwise it sucks on some
     levels
  */
  double rl = rd.normalize();
  Vector2f v = A0 - Cp;
  double b = -v.dot(rd);
  double det = (b * b) - v.dot(v) + Cr * Cr;
  if (det < 0 && det > -0.0001f)
    det = 0;

  if (det >= 0) {
    det = sqrt(det);
    double i1 = b - det;
    double i2 = b + det;

    if (i1 > -0.0001f && i1 < rl + 0.0001f && i2 > -0.0001f &&
        i2 < rl + 0.0001f) {
      Res1 = A0 + rd * i1;
      Res2 = A0 + rd * i2;
      return 2;
    } else if (i1 > -0.0001f && i1 < rl + 0.0001f) {
      Res1 = A0 + rd * i1;
      return 1;
    } else if (i2 > -0.0001f && i2 < rl + 0.0001f) {
      Res1 = A0 + rd * i2;
      return 1;
    }
  }
  return 0;
}

int intersectLineLine2f(const Vector2f &A0,
                        const Vector2f &A1,
                        const Vector2f &B0,
                        const Vector2f &B1,
                        Vector2f &Res) {
  if (A0.almostEqual(A1) || B0.almostEqual(B1))
    return 0;

  /* first, aabb check */
  Vector2f line1_0 = A1;
  Vector2f line1_1 = A0;
  Vector2f line2_0 = B1;
  Vector2f line2_1 = B0;
  bool Ainversed = true, Binversed = true;

  if (A0.x < A1.x) {
    line1_0 = A0;
    line1_1 = A1;
    Ainversed = false;
  }
  if (B0.x < B1.x) {
    line2_0 = B0;
    line2_1 = B1;
    Binversed = false;
  }

  if (line1_1.x < line2_0.x || line1_0.x > line2_1.x)
    return 0;

  if (A0.y < A1.y) {
    if (Ainversed) {
      line1_0 = A0;
      line1_1 = A1;
    }
  } else {
    if (!Ainversed) {
      line1_0 = A1;
      line1_1 = A0;
    }
  }
  if (B0.y < B1.y) {
    if (Binversed) {
      line2_0 = B0;
      line2_1 = B1;
    }
  } else {
    if (!Binversed) {
      line2_0 = B1;
      line2_1 = B0;
    }
  }

  if (line1_1.y < line2_0.y || line1_0.y > line2_1.y)
    return 0;

  /* then, precise check */
  Vector2f P = B0;
  Vector2f N = Vector2f(-(B1.y - B0.y), (B1.x - B0.x));

  float den = N.dot(A1 - A0);
  if (fabs(den) < 0.0001f)
    return 0;

  float t = (P.dot(N) - A0.dot(N)) / den;
  if (t < 0.0f || t > 1.0f)
    return 0;

  P = A0;
  N = Vector2f(-(A1.y - A0.y), (A1.x - A0.x));

  den = N.dot(B1 - B0);
  if (fabs(den) < 0.0001f)
    return 0;

  t = (P.dot(N) - B0.dot(N)) / den;
  if (t < 0.0f || t > 1.0f)
    return 0;

  Res = B0 + (B1 - B0) * t;

  return 1;
}

void intersectLineLine2fCramer(Vector2f A1,
                               Vector2f A2,
                               Vector2f B1,
                               Vector2f B2,
                               Vector2f &contactPoint) {
  // equations of lines (A1, A2) and (B1, B2)
  // a*x + b*y = e
  // c*x + d*y = f
  float a, b, c, d, e, f;
  a = A1.y - A2.y;
  b = A2.x - A1.x;
  e = a * A1.x + b * A1.y;

  c = B1.y - B2.y;
  d = B2.x - B1.x;
  f = c * B2.x + d * B2.y;

  // calculate intersection point with Cramer's formula
  float divider = a * d - b * c;
  contactPoint = Vector2f((e * d - b * f) / divider, (a * f - e * c) / divider);
}

void calculatePointOnNormal(Vector2f &A1,
                            Vector2f &B1,
                            float length,
                            bool inside,
                            Vector2f &A2,
                            Vector2f &B2) {
  Vector2f N(-B1.y + A1.y, B1.x - A1.x);
  float t = length / sqrt(N.x * N.x + N.y * N.y);
  if (inside == true) {
    t = -t;
  }
  A2 = Vector2f(A1.x + t * N.x, A1.y + t * N.y);
  B2 = Vector2f(B1.x + t * N.x, B1.y + t * N.y);
}

void calculatePointOnVector(Vector2f &A1,
                            Vector2f &A2,
                            float length,
                            Vector2f &newA2) {
  Vector2f A(A2.x - A1.x, A2.y - A1.y);
  float t = length / sqrt(A.x * A.x + A.y * A.y);
  newA2 = Vector2f(A1.x + t * A.x, A1.y + t * A.y);
}

float interpolation_cubic(float i_a, float i_b, float i_c, float i_d, float t) {
  double t2 = t * t;
  double x0, x1, x2, x3;
  float v_res;

  x0 = i_d - i_c - i_a + i_b;
  x1 = i_a - i_b - x0;
  x2 = i_c - i_a;
  x3 = i_b;
  v_res = x0 * t * t2 + x1 * t2 + x2 * t + x3;

  return v_res;
}

Vector2f interpolation_cubic(Vector2f i_a,
                             Vector2f i_b,
                             Vector2f i_c,
                             Vector2f i_d,
                             float t) {
  return Vector2f(interpolation_cubic(i_a.x, i_b.x, i_c.x, i_d.x, t),
                  interpolation_cubic(i_a.y, i_b.y, i_c.y, i_d.y, t));
}

float interpolateAngle(float r1, float r2, float t) {
  // main case - no new revolution done
  if ((r1 - r2 >= 0 && r1 - r2 < M_PI) || (r2 - r1 >= 0 && r2 - r1 < M_PI)) {
    return r1 + (r2 - r1) * t;
  }

  // normalize angles between0  and 2 PI
  while (r1 < 0.0) {
    r1 += 2 * M_PI;
  }
  while (r2 < 0.0) {
    r2 += 2 * M_PI;
  }
  while (r1 > 2 * M_PI) {
    r1 -= 2 * M_PI;
  }
  while (r2 > 2 * M_PI) {
    r2 -= 2 * M_PI;
  }

  // check for revolution
  if ((r1 - r2 >= 0 && r1 - r2 < M_PI) || (r2 - r1 >= 0 && r2 - r1 < M_PI)) {
    return r1 + (r2 - r1) * t;
  }

  if (r1 - r2 > 0) {
    r2 += 2 * M_PI;
    return r1 + (r2 - r1) * t;
  }

  r1 += 2 * M_PI;
  return r1 + (r2 - r1) * t;
}
