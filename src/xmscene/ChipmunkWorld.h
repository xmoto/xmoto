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

#ifndef __CHIPMUNKWORLD_H__
#define __CHIPMUNKWORLD_H__

#include <vector>

class cpSpace;
class cpBody;
class cpConstraint;
class cpShape;
class PlayerLocalBiker;
class Biker;
class PhysicsSettings;
class Level;

class ChipmunkWorld {
public:
  ChipmunkWorld(PhysicsSettings *i_physicsSettings, Level *i_level);
  ~ChipmunkWorld();
  void resizeHashes(float i_dim, unsigned int i_size);

  cpSpace *getSpace();
  void setSpace(cpSpace *s);

  cpBody *getBody();
  cpBody *getFrontWheel(unsigned int i_player);
  cpBody *getBackWheel(unsigned int i_player);
  void setBody(cpBody *body);
  void setFrontWheel(cpBody *body, unsigned int i_player);
  void setBackWheel(cpBody *body, unsigned int i_player);
  void setGravity(float i_x, float i_y);

  void addPlayer(PlayerLocalBiker *i_biker);
  void updateWheelsPosition(const std::vector<Biker *> &i_players);

private:
  void initPhysics(PhysicsSettings *i_physicsSettings, Level *i_level);
  cpSpace *m_space;
  cpBody *m_body;

  std::vector<cpBody *> m_ab; // wheel anchors
  std::vector<cpBody *> m_af;
  std::vector<cpBody *> m_wb; // wheel bodies
  std::vector<cpBody *> m_wf;
  std::vector<cpConstraint *> m_joints;
  std::vector<cpShape *> m_shapes;
};

#endif
