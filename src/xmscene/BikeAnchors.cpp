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

#include "BikeAnchors.h"

Vector2f BikeAnchors::GroundPoint() const {
  return Tp;
}

void BikeAnchors::update(BikeParameters *i_bikeParameters) {
  Tp = Vector2f(0, -i_bikeParameters->Ch);
  Rp = Tp + Vector2f(-0.5f * i_bikeParameters->Wb, i_bikeParameters->WR);
  Fp = Tp + Vector2f(0.5f * i_bikeParameters->Wb, i_bikeParameters->WR);

  AR = Vector2f(i_bikeParameters->RVx, i_bikeParameters->RVy);
  AR2 = Vector2f(-i_bikeParameters->RVx, i_bikeParameters->RVy);

  AF = Vector2f(i_bikeParameters->FVx, i_bikeParameters->FVy);
  AF2 = Vector2f(-i_bikeParameters->FVx, i_bikeParameters->FVy);

  PLAp = (Vector2f(i_bikeParameters->PEVx, i_bikeParameters->PEVy) +
          Vector2f(i_bikeParameters->PHVx, i_bikeParameters->PHVy)) *
         0.5f;
  PLAp2 = (Vector2f(-i_bikeParameters->PEVx, i_bikeParameters->PEVy) +
           Vector2f(-i_bikeParameters->PHVx, i_bikeParameters->PHVy)) *
          0.5f;

  PUAp = (Vector2f(i_bikeParameters->PEVx, i_bikeParameters->PEVy) +
          Vector2f(i_bikeParameters->PSVx, i_bikeParameters->PSVy)) *
         0.5f;
  PUAp2 = (Vector2f(-i_bikeParameters->PEVx, i_bikeParameters->PEVy) +
           Vector2f(-i_bikeParameters->PSVx, i_bikeParameters->PSVy)) *
          0.5f;

  PLLp = (Vector2f(i_bikeParameters->PFVx, i_bikeParameters->PFVy) +
          Vector2f(i_bikeParameters->PKVx, i_bikeParameters->PKVy)) *
         0.5f;
  PLLp2 = (Vector2f(-i_bikeParameters->PFVx, i_bikeParameters->PFVy) +
           Vector2f(-i_bikeParameters->PKVx, i_bikeParameters->PKVy)) *
          0.5f;

  PULp = (Vector2f(i_bikeParameters->PLVx, i_bikeParameters->PLVy) +
          Vector2f(i_bikeParameters->PKVx, i_bikeParameters->PKVy)) *
         0.5f;
  PULp2 = (Vector2f(-i_bikeParameters->PLVx, i_bikeParameters->PLVy) +
           Vector2f(-i_bikeParameters->PKVx, i_bikeParameters->PKVy)) *
          0.5f;

  PTp = (Vector2f(i_bikeParameters->PLVx, i_bikeParameters->PLVy) +
         Vector2f(i_bikeParameters->PSVx, i_bikeParameters->PSVy)) *
        0.5f;
  PTp2 = (Vector2f(-i_bikeParameters->PLVx, i_bikeParameters->PLVy) +
          Vector2f(-i_bikeParameters->PSVx, i_bikeParameters->PSVy)) *
         0.5f;

  PHp = Vector2f(i_bikeParameters->PHVx, i_bikeParameters->PHVy);
  PHp2 = Vector2f(-i_bikeParameters->PHVx, i_bikeParameters->PHVy);

  PFp = Vector2f(i_bikeParameters->PFVx, i_bikeParameters->PFVy);
  PFp2 = Vector2f(-i_bikeParameters->PFVx, i_bikeParameters->PFVy);
}
