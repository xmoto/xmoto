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

#ifndef __BIKEANCHORS_H__
#define __BIKEANCHORS_H__

#include "../helpers/VMath.h"
#include "BikeParameters.h"

class BikeAnchors {
public:
  Vector2f GroundPoint() const;
  void update(BikeParameters *i_bikeParameters);

  //  private:
  Vector2f Tp; /* Point on the ground, exactly between the wheels */
  Vector2f Rp; /* Center of rear wheel */
  Vector2f Fp; /* Center of front wheel */
  Vector2f AR; /* Rear suspension anchor */
  Vector2f AF; /* Front suspension anchor */
  Vector2f AR2; /* Rear suspension anchor (Alt.) */
  Vector2f AF2; /* Front suspension anchor (Alt.) */

  Vector2f PTp; /* Player torso center */
  Vector2f PULp; /* Player upper leg center */
  Vector2f PLLp; /* Player lower leg center */
  Vector2f PUAp; /* Player upper arm center */
  Vector2f PLAp; /* Player lower arm center */
  Vector2f PHp; /* Player hand center */
  Vector2f PFp; /* Player foot center */

  Vector2f PTp2; /* Player torso center (Alt.) */
  Vector2f PULp2; /* Player upper leg center (Alt.) */
  Vector2f PLLp2; /* Player lower leg center (Alt.) */
  Vector2f PUAp2; /* Player upper arm center (Alt.) */
  Vector2f PLAp2; /* Player lower arm center (Alt.) */
  Vector2f PHp2; /* Player hand center (Alt.) */
  Vector2f PFp2; /* Player foot center (Alt.) */
};

#endif /* BIKEANCHORS */
