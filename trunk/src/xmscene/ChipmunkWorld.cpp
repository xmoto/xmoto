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

#include "ChipmunkWorld.h"
#include "../chipmunk/chipmunk.h"
#include "../PhysSettings.h"
#include "Bike.h"

ChipmunkWorld::ChipmunkWorld() {
  initPhysics();
}

ChipmunkWorld::~ChipmunkWorld() {
  cpSpaceDestroy(m_space);
}

void ChipmunkWorld::setGravity(float i_x, float i_y) {
  m_space->gravity = cpv(i_x * CHIP_GRAVITY_RATIO, i_y * CHIP_GRAVITY_RATIO);
}

void ChipmunkWorld::initPhysics()
{
  cpBody *staticBody;

  cpInitChipmunk();

  cpSpace *space = cpSpaceNew();
  setSpace(space);

  cpResetShapeIdCounter();

  // do need to resolve gravity between ODE and Chipmunk
  space->gravity = cpv(0.0f, CHIP_GRAVITY);

  // static body to 'hang' the ground segments on -- never moves
  staticBody = cpBodyNew(INFINITY,INFINITY);
  setBody(staticBody);
}

void ChipmunkWorld::resizeHashes(unsigned int i_dim, unsigned int i_size)
{
  if (m_space != NULL) {
    cpSpaceResizeActiveHash(m_space, i_dim, i_size);
    cpSpaceResizeStaticHash(m_space, i_dim, i_size);
  }
}

cpSpace *ChipmunkWorld::getSpace()
{
  return m_space;
}


void ChipmunkWorld::setSpace(cpSpace* s)
{
  m_space = s;
}

cpBody *ChipmunkWorld::getBody()
{
  return m_body;
}

void ChipmunkWorld::setBody(cpBody *body)
{
  m_body = body;
}


cpBody *ChipmunkWorld::getFrontWheel(unsigned int i_player)
{
  return m_af[i_player];
}

void ChipmunkWorld::setFrontWheel(cpBody *body, unsigned int i_player)
{
  m_af[i_player] = body;
}

cpBody *ChipmunkWorld::getBackWheel(unsigned int i_player)
{
  return m_ab[i_player];
}

void ChipmunkWorld::setBackWheel(cpBody *body, unsigned int i_player)
{
  m_ab[i_player] = body;
}

void ChipmunkWorld::addPlayer(Biker* i_biker) {
  // Create two anchors for the wheels
  cpBody* v_ab;
  cpBody* v_af;
  cpBody* v_wb;
  cpBody* v_wf;

  // Wheel mass.. as above ODE/Chipmunk
  cpFloat mass=CHIP_WHEEL_MASS;
  cpFloat wheel_moment = cpMomentForCircle(mass, PHYS_WHEEL_RADIUS * CHIP_SCALE_RATIO, 0.0f, cpvzero);
  cpShape *shape;
  
  v_ab = cpBodyNew(INFINITY, INFINITY);
  v_af = cpBodyNew(INFINITY, INFINITY);
  
  // Create the wheels which will be ODE's surrogates in Chipmunk
  v_wb = cpBodyNew(mass, wheel_moment);
  v_wf = cpBodyNew(mass, wheel_moment);
  
  // Add the wheels (but not the anchors) to the space
  cpSpaceAddBody(m_space, v_wb);
  cpSpaceAddBody(m_space, v_wf);
  
  // Pin-joint the wheels to the anchors
  cpSpaceAddJoint(m_space, cpPinJointNew(v_ab, v_wb, cpvzero, cpvzero));
  cpSpaceAddJoint(m_space, cpPinJointNew(v_af, v_wf, cpvzero, cpvzero));
  
  // creating collision shapes for the wheels
  //   change to collision group 1 
  //   -- we don't want (or need) them to collide with the terrain
  shape = cpCircleShapeNew(v_wf, PHYS_WHEEL_RADIUS * CHIP_SCALE_RATIO, cpvzero);
  shape->u = CHIP_WHEEL_FRICTION;
  shape->e = CHIP_WHEEL_ELASTICITY;
  shape->group = 1;
  cpSpaceAddShape(m_space, shape);
  
  shape = cpCircleShapeNew(v_wb, PHYS_WHEEL_RADIUS * CHIP_SCALE_RATIO, cpvzero);
  shape->u = CHIP_WHEEL_FRICTION;
  shape->e = CHIP_WHEEL_ELASTICITY;
  shape->group = 1;
  cpSpaceAddShape(m_space, shape);
  
  m_ab.push_back(v_ab);
  m_af.push_back(v_af);
  m_wb.push_back(v_wb);
  m_wf.push_back(v_wf);

  /* init position */
  v_af->p = cpv(i_biker->getState()->FrontWheelP.x * CHIP_SCALE_RATIO, i_biker->getState()->FrontWheelP.y * CHIP_SCALE_RATIO);
  v_ab->p = cpv(i_biker->getState()->RearWheelP.x * CHIP_SCALE_RATIO, i_biker->getState()->RearWheelP.y * CHIP_SCALE_RATIO);
  v_wf->p = v_af->p;
  v_wb->p = v_ab->p;
}

void ChipmunkWorld::updateWheelsPosition(const std::vector<Biker*>& i_players) {
  cpBody *b;
  cpFloat dx, dy;
    
  for(unsigned int i=0; i<i_players.size(); i++) {
    // inform chipmunk of ODE pos of front wheel
    b = getFrontWheel(i);
    
    b->w = i_players[i]->getFrontWheelVelocity();
    dx = i_players[i]->getState()->FrontWheelP.x * CHIP_SCALE_RATIO - b->p.x;
    dy = i_players[i]->getState()->FrontWheelP.y * CHIP_SCALE_RATIO - b->p.y;
    
    b->p.x += dx/CHIP_WHEEL_DAMPENING;
    b->p.y += dy/CHIP_WHEEL_DAMPENING;
      
    // inform chipmunk of ODE pos of back wheel
    b = getBackWheel(i);
    
    b->w = i_players[i]->getRearWheelVelocity();
    dx = i_players[i]->getState()->RearWheelP.x * CHIP_SCALE_RATIO - b->p.x;
    dy = i_players[i]->getState()->RearWheelP.y * CHIP_SCALE_RATIO - b->p.y;
    
    b->p.x += dx/CHIP_WHEEL_DAMPENING;
    b->p.y += dy/CHIP_WHEEL_DAMPENING;
  }
}
