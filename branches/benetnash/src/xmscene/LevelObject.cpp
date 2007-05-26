#include "LevelObject.h"
#include "Renderer.h"





LevelObject::LevelObject(bool is_pseudoClass):pseudoClass(is_pseudoClass) 
{
	m_isOnMiniMap = false;
}

void LevelObject::setOnMiniMap(bool val)
{
	m_isOnMiniMap = val;
}

int LevelObject::compareDepths(LevelObject* lo)
{
	return getDepth() > lo->getDepth();
}

AABB& LevelObject::getAABB()
{
	return m_BBox;
}

bool LevelObject::isVisible(AABB& screenBox) 
{
	/* in child classes this method refreshes AABB of dynamic blocks, 
	so we cannot use direct call */
	AABB& ourAABB = getAABB(); 
	/* our box */
	Vector2f& bbmin = ourAABB.getBMin();
	Vector2f& bbmax = ourAABB.getBMax();
	/* screen box */
	Vector2f& sbmin = screenBox.getBMin();
	Vector2f& sbmax = screenBox.getBMax();
	if (bbmax.x >= sbmin.x &&
		sbmax.x >= bbmin.x &&
		bbmax.y >= sbmin.y &&
		sbmax.y >= bbmin.y )
		return true;
	return false;
}
		
void LevelObject::setDepth(float _z) 
{ 
	__z = _z;
}

float LevelObject::getDepth()
{ 
	return __z;
}

bool LevelObject::isPseudoClass() 
{
	return pseudoClass;
}

PseudoObjectBikes::PseudoObjectBikes(): LevelObject(true) 
{
	setDepth(OBJECT_DEPTH_BIKE);
}

void PseudoObjectBikes::render(vapp::GameRenderer* r)
{
	r->RenderLevelBikes();
}

void PseudoObjectBikes::setDepthAuto()
{
	this->setDepth(0);
}

PseudoObjectExternEntities::PseudoObjectExternEntities(): LevelObject(true)  
{
	setDepth(OBJECT_DEPTH_FRONT_PARTICLES);
}

void PseudoObjectExternEntities::render(vapp::GameRenderer* r) 
{
	r->RenderLevelExternEntities();
}	

void PseudoObjectExternEntities::setDepthAuto()
{
	this->setDepth(10);
}
