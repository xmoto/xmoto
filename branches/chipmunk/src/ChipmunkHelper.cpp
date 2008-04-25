#include "ChipmunkHelper.h"
#include "PhysSettings.h"
#include <stdio.h>

ChipmunkHelper *ChipmunkHelper::mp_instance = NULL;

void ChipmunkHelper::reInstance()
{
	if (mp_instance != NULL)
	{
		// seems overkill, however there appears to be a bug in the object
		// hash code -- workaround.
		cpSpaceDestroy(m_space);
		initPhysics();
	}

}

ChipmunkHelper *ChipmunkHelper::Instance()
{
	if (mp_instance == NULL)
	{
		mp_instance = new ChipmunkHelper;
		mp_instance->initPhysics();
	}
	return mp_instance;
}

void ChipmunkHelper::initPhysics()
{
  cpBody *staticBody;
  cpBody *chassis, *wheel1, *wheel2;

  cpInitChipmunk();

  cpSpace *space = cpSpaceNew();
  setSpace(space);

  cpResetShapeIdCounter();

  // do need to resolve gravity between ODE and Chipmunk
  space->gravity = cpv(0.0f, -150.0f);
  space->iterations=10;

  // Could be optimised per level.. leave for now
  cpSpaceResizeActiveHash(space, 50.0, 999);
  cpSpaceResizeStaticHash(space, 50.0, 999);

  // static body to 'hang' the ground segments on -- never moves
  staticBody = cpBodyNew(INFINITY,INFINITY);
  setStaticBody(staticBody);

  // Wheel mass.. as above ODE/Chipmunk
  cpFloat mass=7;
  cpFloat wheel_moment = cpMomentForCircle(mass, 0.35 * CHIP_RATIO, 0.0f, cpvzero);

  // Create two anchors for the wheels
  m_ab = cpBodyNew(INFINITY, INFINITY);
  m_af = cpBodyNew(INFINITY, INFINITY);

  // Create the wheels which will be ODE's surrogates in Chipmunk
  m_wb = cpBodyNew(mass, wheel_moment);
  m_wf = cpBodyNew(mass, wheel_moment);

  // Add the wheels (but not the anchors) to the space
  cpSpaceAddBody(space, m_wb);
  cpSpaceAddBody(space, m_wf);

  // Pin-joint the wheels to the anchors
  cpSpaceAddJoint(space, cpPinJointNew(m_ab, m_wb, cpvzero, cpvzero));
  cpSpaceAddJoint(space, cpPinJointNew(m_af, m_wf, cpvzero, cpvzero));


  cpShape *shape;
 
  // creating collision shapes for the wheels
  //   change to collision group 1 
  //   -- we don't want (or need) them to collide with the terrain
  shape = cpCircleShapeNew(m_wf, 0.35 * CHIP_RATIO, cpvzero);
  shape->u = 0.1;
  shape->group = 1;
  cpSpaceAddShape(space,shape);

  shape = cpCircleShapeNew(m_wb, 0.35 * CHIP_RATIO, cpvzero);
  shape->u = 0.1;
  shape->group = 1;
  cpSpaceAddShape(space,shape);

}

cpSpace *ChipmunkHelper::getSpace()
{
	return m_space;
}


void ChipmunkHelper::setSpace(cpSpace* s)
{
	m_space = s;
}

cpBody *ChipmunkHelper::getStaticBody()
{
	return m_body;
}

void ChipmunkHelper::setStaticBody(cpBody *body)
{
	m_body = body;
}


cpBody *ChipmunkHelper::getFrontWheel()
{
	return m_af;
}

void ChipmunkHelper::setFrontWheel(cpBody *body)
{
	m_af = body;
}

cpBody *ChipmunkHelper::getBackWheel()
{
	return m_ab;
}

void ChipmunkHelper::setBackWheel(cpBody *body)
{
	m_ab = body;
}

