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
  space->gravity = cpv(0.0f, -300.0f);
  space->iterations=10;
  cpSpaceResizeActiveHash(space, 50.0, 999);
  cpSpaceResizeStaticHash(space, 50.0, 999);


  staticBody = cpBodyNew(INFINITY,INFINITY);
  //staticBody = cpBodyNew(1e1000,1e1000);
  setStaticBody(staticBody);

  cpFloat mass=7;
  cpFloat wheel_moment = cpMomentForCircle(mass, 0.35 * CHIP_RATIO, 0.0f, cpvzero);
  m_ab = cpBodyNew(INFINITY, INFINITY);
  m_af = cpBodyNew(INFINITY, INFINITY);


  m_wf = cpBodyNew(mass, wheel_moment);
  m_wb = cpBodyNew(mass, wheel_moment);
  cpSpaceAddBody(space, m_wf);
  cpSpaceAddBody(space, m_wb);

  cpSpaceAddJoint(space, cpPinJointNew(m_ab, m_wb, cpvzero, cpvzero));
  cpSpaceAddJoint(space, cpPinJointNew(m_af, m_wf, cpvzero, cpvzero));


  cpShape *shape;

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

