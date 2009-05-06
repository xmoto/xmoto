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

#include <algorithm>
#include "Block.h"
#include "../Collision.h"
#include "../PhysSettings.h"
#include "../helpers/Log.h"
#include "../VFileIO.h"
#include "../VXml.h"
#include "../BSP.h"
#include "../chipmunk/chipmunk.h"
#include "ChipmunkWorld.h"
#include "PhysicsSettings.h"

#define XM_DEFAULT_BLOCK_TEXTURE "default"
#define XM_DEFAULT_PHYS_BLOCK_MASS 30.0
#define XM_DEFAULT_PHYS_BLOCK_FRICTION 0.5
#define XM_DEFAULT_PHYS_BLOCK_ELASTICITY 0.0

/* Vertex */
ConvexBlockVertex::ConvexBlockVertex(const Vector2f& i_position, const Vector2f& i_texturePosition) {
  m_position         = i_position;
  m_texturePosition  = i_texturePosition;
}

ConvexBlockVertex::~ConvexBlockVertex() {
}

void ConvexBlockVertex::setPosition(const Vector2f& i_position) {
  m_position = i_position;
}

/* Block convex */
ConvexBlock::ConvexBlock(Block *i_srcBlock) {
  m_srcBlock = i_srcBlock;
}

ConvexBlock::~ConvexBlock() {
  for(unsigned int i=0; i<m_vertices.size(); i++) {
    delete m_vertices[i];
  }
}

Block* ConvexBlock::SourceBlock() const {
  return m_srcBlock;
}

void ConvexBlock::addVertex(const Vector2f& i_position, const Vector2f& i_texturePosition) {
  m_vertices.push_back(new ConvexBlockVertex(i_position, i_texturePosition));
}

void ConvexBlock::Move(const Vector2f& i_vector) {
  for(unsigned int i=0; i<m_vertices.size(); i++) {
    m_vertices[i]->setPosition(m_vertices[i]->Position() + i_vector);
  }
}

Block::Block(std::string i_id) {
  m_id               = i_id;
  m_initialPosition  = Vector2f(0.0, 0.0);
  m_initialRotation  = 0;
  m_textureScale     = 1.0;
  m_background       = false;
  m_dynamic          = false;
  m_physics          = false;
  m_isLayer          = false;
  m_gripPer20        = 20.0;
  m_mass             = XM_DEFAULT_PHYS_BLOCK_MASS;
  m_friction         = XM_DEFAULT_PHYS_BLOCK_FRICTION;
  m_elasticity       = XM_DEFAULT_PHYS_BLOCK_ELASTICITY;
  m_dynamicPosition  = m_initialPosition;
  m_dynamicRotation  = m_initialRotation;
  m_dynamicRotationCenter = Vector2f(0.0, 0.0);
  m_dynamicPositionCenter = Vector2f(0.0, 0.0);
  m_texture          = XM_DEFAULT_BLOCK_TEXTURE;
  m_isBBoxDirty      = true;
  m_geom             = -1;
  m_layer            = -1;
  m_edgeDrawMethod   = angle;
  m_edgeAngle        = DEFAULT_EDGE_ANGLE;
  mBody              = NULL;
  m_shape            = NULL;
  m_collisionElement = NULL;
  m_collisionMethod  = None;
  m_collisionRadius  = 0.0f;
  m_blendColor.setRed(255);
  m_blendColor.setGreen(255);
  m_blendColor.setBlue(255);
  m_blendColor.setAlpha(255);

}

Block::~Block() {
  /* delete vertices */
  for(unsigned int i=0; i<m_vertices.size(); i++) {
    delete m_vertices[i];
  }
  m_vertices.clear();

  /* delete convex blocks */
  for(unsigned int i=0; i<m_convexBlocks.size(); i++) {
    delete m_convexBlocks[i];
  }
  m_convexBlocks.clear();

  /* delete collision lines */
  for(unsigned int i=0; i<m_collisionLines.size(); i++) {
    delete m_collisionLines[i];
  }
  m_collisionLines.clear();

  m_edgeGeoms.clear();
  
}

void Block::setCenter(const Vector2f& i_center) {
  m_dynamicRotationCenter = i_center;
  m_dynamicPositionCenter = i_center;
  m_isBBoxDirty           = true;
}

Vector2f Block::DynamicPositionCenter() const {
  return m_dynamicPositionCenter;
}

void Block::updateCollisionLines(bool setDirty, bool forceUpdate) {
  bool manageCollisions = (m_collisionLines.size() != 0);
  /* Ignore background blocks */
  if(isDynamic() == false
     || manageCollisions == false)
    return;

  if(forceUpdate == false && isBackground() == true)
    return;

  if(setDirty == true){
    m_isBBoxDirty = true;
  }

  /* Build rotation matrix for block */
  float fR[4]; 
  fR[0] =  cosf(DynamicRotation());
  fR[1] = -sinf(DynamicRotation());
  fR[2] =  sinf(DynamicRotation()); 
  fR[3] =  cosf(DynamicRotation());
  unsigned int z = 0;

  for(unsigned int j=0; j<Vertices().size(); j++) {
    // FIXME::aren't each vertice transform twice ??
    unsigned int jnext = j==Vertices().size()-1? 0 : j+1;
    BlockVertex *pVertex1 = Vertices()[j];
    BlockVertex *pVertex2 = Vertices()[jnext];


    /* Transform vertices */
    Vector2f Tv1 = Vector2f((pVertex1->Position().x-DynamicRotationCenter().x) * fR[0] + (pVertex1->Position().y-DynamicRotationCenter().y) * fR[1],
			    (pVertex1->Position().x-DynamicRotationCenter().x) * fR[2] + (pVertex1->Position().y-DynamicRotationCenter().y) * fR[3]);
    Tv1 += m_dynamicPosition + DynamicRotationCenter();
    
    Vector2f Tv2 = Vector2f((pVertex2->Position().x-DynamicRotationCenter().x) * fR[0] + (pVertex2->Position().y-DynamicRotationCenter().y) * fR[1],
			    (pVertex2->Position().x-DynamicRotationCenter().x) * fR[2] + (pVertex2->Position().y-DynamicRotationCenter().y) * fR[3]);
    Tv2 += m_dynamicPosition + DynamicRotationCenter();
    
    /* Update line */
    m_collisionLines[z]->x1 = Tv1.x;
    m_collisionLines[z]->y1 = Tv1.y;
    m_collisionLines[z]->x2 = Tv2.x;
    m_collisionLines[z]->y2 = Tv2.y;
    
    /* Next collision line */
    z++;
  }
}

void Block::translateCollisionLines(float x, float y)
{
  bool manageCollisions = (m_collisionLines.size() != 0);

  if(isDynamic() == false
     || manageCollisions == false
     || isBackground() == true)
    return;

  std::vector<Line*>::iterator it = m_collisionLines.begin();
  while(it != m_collisionLines.end()) {
    (*it)->x1 += x;
    (*it)->x2 += x;
    (*it)->y1 += y;
    (*it)->y2 += y;

    it++;
  }
}

void Block::setDynamicPosition(const Vector2f& i_dynamicPosition) {
  Vector2f diff = i_dynamicPosition - m_dynamicPosition;
  m_dynamicPosition = i_dynamicPosition;

  m_BCircle.translate(diff.x, diff.y);
  translateCollisionLines(diff.x, diff.y);
}

void Block::setPhysicsPosition(float ix, float iy) {
  mBody->p.x = ix * CHIP_SCALE_RATIO;
  mBody->p.y = iy * CHIP_SCALE_RATIO;
}

void Block::setDynamicPositionAccordingToCenter(const Vector2f& i_dynamicPosition) {
  Vector2f newPos = i_dynamicPosition - m_dynamicPositionCenter;
  Vector2f diff = newPos - m_dynamicPosition;
  m_dynamicPosition = newPos;

  m_BCircle.translate(diff.x, diff.y);
  translateCollisionLines(diff.x, diff.y);
}

bool Block::setDynamicRotation(float i_dynamicRotation) {
  m_dynamicRotation = i_dynamicRotation;

  // if the center of rotation is the center of the bounding circle,
  // we don't have to set the bounding box to dirty
  if(DynamicRotationCenter() + DynamicPosition() == m_BCircle.getCenter()) {
    updateCollisionLines(false, false);
    return false;
  } else {
    updateCollisionLines(true, true);
    return true;
  }
}

void Block::setCollisionMethod(CollisionMethod method)
{
  m_collisionMethod = method;
}

void Block::setCollisionRadius(float radius)
{
  m_collisionRadius = radius;
}

void Block::translate(float x, float y)
{
  m_dynamicPosition.x += x;
  m_dynamicPosition.y += y;
  // dont recalculate the BoundingCircle
  m_BCircle.translate(x, y);
  translateCollisionLines(x, y);
}

void Block::updatePhysics(int timeStep, CollisionSystem* io_collisionSystem) {
  if(isPhysics()) {
    // move block according to chipmunk, only if they have moved
    bool moved = false;

    Vector2f newPos = Vector2f(((mBody->p.x ) / CHIP_SCALE_RATIO) , (mBody->p.y ) / CHIP_SCALE_RATIO);
    if(DynamicPosition() != newPos) {
      setDynamicPosition(newPos);
      moved = true;
    }

    if(DynamicRotation() != mBody->a) {
      setDynamicRotation(mBody->a);
      moved = true;
    }

    if(moved == true){
      // inform collision system that there has been a change
      io_collisionSystem->moveDynBlock(this);
    }
    
    cpBodyResetForces(mBody);
  }
}

int Block::loadToPlay(CollisionSystem* io_collisionSystem, ChipmunkWorld* i_chipmunkWorld, PhysicsSettings* i_physicsSettings) {

  m_dynamicPosition       = m_initialPosition;
  m_dynamicRotation       = m_initialRotation;
  m_dynamicRotationCenter = Vector2f(0.0, 0.0);
  m_dynamicPositionCenter = Vector2f(0.0, 0.0);
  float tx = 0;
  float ty = 0;


  /* Do the "convexifying" the BSP-way. It might be overkill, but we'll
     probably appreciate it when the input data is very complex. It'll also 
     let us handle crossing edges, and other kinds of weird input. */
  BSP v_BSPTree;
  cpBody*  myBody  = NULL;
  cpVect*  myVerts = NULL;
  
  if(i_chipmunkWorld != NULL) {
    if (isPhysics()) {
      unsigned int size = 2*sizeof(cpVect)*Vertices().size();
      myVerts = (cpVect*)malloc(size);
    }
  }

  /* Define edges */
  for(unsigned int i=0; i<Vertices().size(); i++) {
    /* Next vertex? */
    unsigned int inext = i+1;
    if(inext == Vertices().size()) inext=0;

    /* add static lines */
    if(isBackground() == false && isDynamic() == false && m_layer == -1) {
      /* Add line to collision handler */
      io_collisionSystem->defineLine(DynamicPosition().x + Vertices()[i]->Position().x,
				     DynamicPosition().y + Vertices()[i]->Position().y,
				     DynamicPosition().x + Vertices()[inext]->Position().x,
				     DynamicPosition().y + Vertices()[inext]->Position().y,
				     GripPer20() * i_physicsSettings->BikeWheelBlockGrip() / 20.0);
      

      if(i_chipmunkWorld != NULL) {
	// Create/duplicate terrain for chipmunks objects to use
	cpVect a = cpv( (DynamicPosition().x + Vertices()[i]->Position().x) * CHIP_SCALE_RATIO,
			(DynamicPosition().y + Vertices()[i]->Position().y) * CHIP_SCALE_RATIO);
	cpVect b = cpv( (DynamicPosition().x + Vertices()[inext]->Position().x) * CHIP_SCALE_RATIO,
			(DynamicPosition().y + Vertices()[inext]->Position().y) * CHIP_SCALE_RATIO);

	cpShape *seg = cpSegmentShapeNew(i_chipmunkWorld->getBody(), a, b, 0.0f);
	seg->group = 0;
	seg->u = m_friction;
	seg->e = m_elasticity;
	cpSpaceAddStaticShape(i_chipmunkWorld->getSpace(), seg);
      }
    }

    /* add dynamic lines */
    if(isLayer() == false && isDynamic()) {
      /* Define collision lines */
      Line *v_line = new Line;
      v_line->x1 = v_line->y1 = v_line->x2 = v_line->y2 = 0.0f;
      v_line->fGrip = GripPer20() * i_physicsSettings->BikeWheelBlockGrip() / 20.0;
      m_collisionLines.push_back(v_line);
    }

    if(isPhysics()) {
      // collect vertice count to find middle
      tx += Vertices()[i]->Position().x;      
      ty += Vertices()[i]->Position().y;      
    } else {
      v_BSPTree.addLineDefinition(Vertices()[i]->Position(), Vertices()[inext]->Position());
    }
  }

  if(isPhysics()) {
    // calculate midpoint
    float mdx = tx / Vertices().size();
    float mdy = ty / Vertices().size();

    // for physics objects we reorient around a center of gravity
    // determined by the vertices
    for(unsigned int i=0; i<Vertices().size(); i++) {
      Vertices()[i]->setPosition(Vector2f(Vertices()[i]->Position().x - mdx, Vertices()[i]->Position().y - mdy));
    }

    // then BSP-ify them
    for(unsigned int i=0; i<Vertices().size(); i++) {
      unsigned int inext = i+1;
      if(inext == Vertices().size()) inext=0;

      /* Add line to BSP generator */ 
      v_BSPTree.addLineDefinition(Vertices()[i]->Position(), Vertices()[inext]->Position());
    }

    // modify the object coords with the midpoint
    m_dynamicPosition.x += mdx;
    m_dynamicPosition.y += mdy;
  }

  /* define dynamic block in the collision system */
  if(isLayer() == false && isDynamic()) {
    updateCollisionLines(true, true);
    m_collisionElement = io_collisionSystem->addDynBlock(this);
  }

  if(isDynamic() == false && m_layer == -1){
    io_collisionSystem->addStaticBlock(this, isLayer());
  }

  if(isLayer() == true && m_layer != -1) {
    io_collisionSystem->addBlockInLayer(this, m_layer);
  }

  /* Compute */
  std::vector<BSPPoly *> &v_BSPPolys = v_BSPTree.compute();      
  
  /* Create convex blocks */
  for(unsigned int i=0; i<v_BSPPolys.size(); i++) {
    addPoly(v_BSPPolys[i], io_collisionSystem);
  }

  if(i_chipmunkWorld != NULL) {
    if(isPhysics()) {
      if(getCollisionMethod() == Circle && getCollisionRadius() != 0.0f) {
	float radius = getCollisionRadius();

	cpFloat circle_moment = cpMomentForCircle(m_mass, radius, 0.0f, cpvzero);
	myBody = cpBodyNew(m_mass, circle_moment);
	myBody->p = cpv((DynamicPosition().x * CHIP_SCALE_RATIO),
			(DynamicPosition().y * CHIP_SCALE_RATIO));
	cpSpaceAddBody(i_chipmunkWorld->getSpace(), myBody);
	mBody = myBody;

	m_shape = cpCircleShapeNew(mBody, radius * CHIP_SCALE_RATIO, cpvzero);
	m_shape->u = m_friction;
	m_shape->e = m_elasticity;
	if(isBackground() == true) {
	  m_shape->group = 1;
	}

	cpSpaceAddShape(i_chipmunkWorld->getSpace(), m_shape);
      } else {
	// create body vertices for chipmunk constructors
	for(unsigned int i=0; i<Vertices().size(); i++) {
	  cpVect ma = cpv(Vertices()[i]->Position().x * CHIP_SCALE_RATIO,
			  Vertices()[i]->Position().y * CHIP_SCALE_RATIO);
	  myVerts[i] =  ma;
	}

	// Create body to attach shapes to
	cpFloat bMoment = cpMomentForPoly(m_mass,Vertices().size(), myVerts, cpvzero); 
	free(myVerts);

	// create body 
	myBody = cpBodyNew(m_mass, bMoment);
	myBody->p = cpv((DynamicPosition().x * CHIP_SCALE_RATIO),
			(DynamicPosition().y * CHIP_SCALE_RATIO));
	cpSpaceAddBody(i_chipmunkWorld->getSpace(), myBody);
	mBody = myBody;

	// go through calculated BSP polys, adding one or more shape to the
	for(unsigned int i=0; i<v_BSPPolys.size(); i++) {
	  unsigned int size = 2*sizeof(cpVect)*v_BSPPolys[i]->Vertices().size();
	  myVerts = (cpVect*)malloc(size);

	  // translate for chipmunk
	  for(unsigned int j=0; j<v_BSPPolys[i]->Vertices().size(); j++) {
	    cpVect ma = cpv(v_BSPPolys[i]->Vertices()[j].x * CHIP_SCALE_RATIO,
			    v_BSPPolys[i]->Vertices()[j].y * CHIP_SCALE_RATIO);
	    myVerts[j] =  ma;
	  }

	  // collision shape
	  m_shape = cpPolyShapeNew(myBody, v_BSPPolys[i]->Vertices().size(), myVerts, cpvzero);
	  m_shape->u = m_friction;
	  m_shape->e = m_elasticity;
	  if (isBackground()) {
	    m_shape->group = 1;
	  }

	  cpSpaceAddShape(i_chipmunkWorld->getSpace(), m_shape);

	  // free the temporary vertices array
	  free(myVerts);
	}
      }
    }
  }

  updateCollisionLines(true);

  if(v_BSPTree.getNumErrors() > 0) {
    LogError("Error due to the block %s", Id().c_str());
  }

  return v_BSPTree.getNumErrors();  
}

void Block::addPoly(BSPPoly* i_poly, CollisionSystem* io_collisionSystem) {
  ConvexBlock *v_block = new ConvexBlock(this);
  float scale = TextureScale();

  for(unsigned int i=0; i<i_poly->Vertices().size(); i++) {
    v_block->addVertex(i_poly->Vertices()[i],
		       Vector2f((InitialPosition().x + i_poly->Vertices()[i].x) * 0.25 * scale,
                                (InitialPosition().y + i_poly->Vertices()[i].y) * 0.25 * scale));
  }
  m_convexBlocks.push_back(v_block);
}

void Block::unloadToPlay() {
  for(unsigned int i=0; i<m_convexBlocks.size(); i++) {
    delete m_convexBlocks[i];
  }
  m_convexBlocks.clear();

  for(unsigned int i=0; i<m_collisionLines.size(); i++) {
    delete m_collisionLines[i];
  }
  m_collisionLines.clear();
  
}

bool Block::isPhysics() const {
  return m_physics;
}

bool Block::isLayer() const {
  return m_isLayer;
}

void Block::setInitialPosition(const Vector2f& i_initialPosition) {
  m_initialPosition  = i_initialPosition;
  m_isBBoxDirty = true;
}

Vector2f Block::InitialPosition() const {
  return m_initialPosition;
}

float Block::GripPer20() const {
  return m_gripPer20;
}

float Block::Mass() const {
  return m_mass;
}

float Block::Friction() const {
  return m_friction;
}

float Block::Elasticity() const {
  return m_elasticity;
}

float Block::TextureScale() const {
  return m_textureScale;
}

std::vector<BlockVertex *>& Block::Vertices() {
  return m_vertices;
}

void Block::setTexture(const std::string& i_texture) {
  m_texture = i_texture;
}

void Block::setTextureScale(float i_textureScale) {
  m_textureScale = i_textureScale;
}

void Block::setBackground(bool i_background) {
  m_background = i_background;
}

void Block::setDynamic(bool i_dynamic) {
  m_dynamic = i_dynamic;
}

void Block::setPhysics(bool i_physics) {
  m_physics = i_physics;

  if(m_physics) {
    // we're.. kinda.. dynamic?
    //
    setDynamic(true);
  }
}

void Block::setIsLayer(bool i_isLayer) {
  m_isLayer = i_isLayer;
}

void Block::setGripPer20(float i_gripPer20) {
  m_gripPer20 = i_gripPer20;
}

void Block::setMass(float i_mass) {
  m_mass = i_mass;
}

void Block::setFriction(float i_friction) {
  m_friction = i_friction;
}

void Block::setElasticity(float i_elasticity) {
  m_elasticity = i_elasticity;
}

AABB& Block::getAABB()
{
  if(isDynamic() == true) {
    // will be dirty only once too.
    // after, the bounding circle is translated and rotated.
    if(m_isBBoxDirty == true){
      m_BCircle.reset();
      //updateCollisionLines(true);
      for(unsigned int i=0; i<m_collisionLines.size(); i++){
	Line* pLine = m_collisionLines[i];
	// add only the first point because the second
	// point of the line n is the same as the first
	// point of the line n+1
	m_BCircle.addPointToCircle(pLine->x1, pLine->y1);
      }
      m_BCircle.calculateBoundingCircle();
      m_isBBoxDirty = false;
    }
    return m_BCircle.getAABB();
  } else {
    // will be dirty only once for a static block
    if(m_isBBoxDirty == true){
      m_BBox.reset();

      for(unsigned int i=0; i<Vertices().size(); i++) {
	m_BBox.addPointToAABB2f(DynamicPosition() + Vertices()[i]->Position());
      }

      m_isBBoxDirty = false;
    }
    return m_BBox;
  }
}

BlockVertex::BlockVertex(const Vector2f& i_position, const std::string& i_edgeEffect) {
  m_position   = i_position;
  m_edgeEffect = i_edgeEffect;
}

BlockVertex::~BlockVertex() {
}

void BlockVertex::setPosition(const Vector2f& i_position) {
  m_position = i_position;
}


void BlockVertex::setTexturePosition(const Vector2f& i_texturePosition) {
  m_texturePosition = i_texturePosition;
}

void BlockVertex::setColor(const TColor &i_color) {
  m_color = i_color;
}


bool Block::isPhysics_readFromXml(XMLDocument* i_xmlSource, TiXmlElement *pElem) {
  TiXmlElement* pPositionElem   = XML::findElement(*i_xmlSource, pElem, std::string("position"));

  if(pPositionElem == NULL) {
    return false;
  }

  return XML::getOption(pPositionElem,"physics","false") == "true";
}


Block* Block::readFromXml(XMLDocument* i_xmlSource, TiXmlElement *pElem) {
  
  Block *pBlock = new Block(XML::getOption(pElem, "id"));
  pBlock->setTexture("default");

  TiXmlElement* pUseTextureElem = XML::findElement(*i_xmlSource, pElem, std::string("usetexture"));
  TiXmlElement* pPositionElem   = XML::findElement(*i_xmlSource, pElem, std::string("position"));
  TiXmlElement* pPhysicsElem    = XML::findElement(*i_xmlSource, pElem, std::string("physics"));
  TiXmlElement* pEdgeElem       = XML::findElement(*i_xmlSource, pElem, std::string("edges"));
  TiXmlElement* pColElem        = XML::findElement(*i_xmlSource, pElem, std::string("collision"));


  if(pUseTextureElem != NULL) {
    pBlock->setTexture(XML::getOption(pUseTextureElem,"id", "default"));
    pBlock->setTextureScale(atof(XML::getOption(pUseTextureElem,"scale","1").c_str()));
    
    
    /* Color blending for blocks */
    int blendColor_r = 255, blendColor_g = 255, blendColor_b = 255, blendColor_a = 255;
    std::string v_blendColor = XML::getOption(pUseTextureElem,"color_r");
    if(v_blendColor != "") blendColor_r = atoi(v_blendColor.c_str());
    v_blendColor = XML::getOption(pUseTextureElem,"color_g");
    if(v_blendColor != "") blendColor_g = atoi(v_blendColor.c_str());
    v_blendColor = XML::getOption(pUseTextureElem,"color_b");
    if(v_blendColor != "") blendColor_b = atoi(v_blendColor.c_str());
    v_blendColor = XML::getOption(pUseTextureElem,"color_a");
    if(v_blendColor != "") blendColor_a = atoi(v_blendColor.c_str());
    pBlock->setBlendColor(TColor(blendColor_r,blendColor_g,blendColor_b,blendColor_a));
    
  }

  if(pPositionElem != NULL) {
    pBlock->setInitialPosition(Vector2f(atof( XML::getOption(pPositionElem,"x","0").c_str() ),
                                        atof( XML::getOption(pPositionElem,"y","0").c_str() )
                                        )
                               );

    pBlock->setBackground(XML::getOption(pPositionElem,"background","false") == "true");
    pBlock->setDynamic(XML::getOption(pPositionElem,"dynamic","false") == "true");
    /* setDynamic must be done before setPhysics - physics implies dynamic */
    pBlock->setPhysics(XML::getOption(pPositionElem,"physics","false") == "true");
    pBlock->setIsLayer(XML::getOption(pPositionElem,"islayer","false") == "true");
    pBlock->setLayer(atoi(XML::getOption(pPositionElem,"layerid","-1").c_str()));
 }

  if(pPhysicsElem != NULL) {
    char str[16];

    pBlock->setGripPer20(atof(XML::getOption(pPhysicsElem, "grip", "20.0").c_str()));

    snprintf(str, 16, "%f", XM_DEFAULT_PHYS_BLOCK_MASS);
    std::string mass = XML::getOption(pPhysicsElem, "mass", str);
    if(mass == "INFINITY"){
      // Chipmunk's INFINITY is too big (make blocks disapear), so put a big value instead
      float bigValue = 5000000000000000.0;
      pBlock->setMass(bigValue);
    }
    else
      pBlock->setMass(atof(mass.c_str()));

    snprintf(str, 16, "%f", XM_DEFAULT_PHYS_BLOCK_FRICTION);
    pBlock->setFriction(atof(XML::getOption(pPhysicsElem, "friction", str).c_str()));

    snprintf(str, 16, "%f", XM_DEFAULT_PHYS_BLOCK_ELASTICITY);
    pBlock->setElasticity(atof(XML::getOption(pPhysicsElem, "elasticity", str).c_str()));
  } else {
    pBlock->setGripPer20(20.0);
    pBlock->setMass(XM_DEFAULT_PHYS_BLOCK_MASS);
    pBlock->setFriction(XM_DEFAULT_PHYS_BLOCK_FRICTION);
    pBlock->setElasticity(XM_DEFAULT_PHYS_BLOCK_ELASTICITY);
  }

  if(pEdgeElem != NULL) {
    // angle is the default one
    std::string methodStr = XML::getOption(pEdgeElem, "drawmethod", "angle");
    pBlock->setEdgeDrawMethod(pBlock->stringToEdge(methodStr));
    // 270 is the default one for the 'angle' edge draw method, not used for the others
    pBlock->setEdgeAngle(atof(XML::getOption(pEdgeElem, "angle", "270.0").c_str()));
  } else {
    pBlock->setEdgeDrawMethod(angle);
    pBlock->setEdgeAngle(DEFAULT_EDGE_ANGLE);
  }

  if(pColElem != NULL) {
    std::string methodStr = XML::getOption(pColElem, "type", "None");
    pBlock->setCollisionMethod(pBlock->stringToColMethod(methodStr));
    pBlock->setCollisionRadius(atof(XML::getOption(pColElem, "radius", "0.0").c_str()));    
  }

  float lastX = 0.0;
  float lastY = 0.0;
  bool  firstVertex = true;
  /* Get vertices */
  for(TiXmlElement *pj = pElem->FirstChildElement(); pj!=NULL; pj=pj->NextSiblingElement()) {
    if(!strcmp(pj->Value(),"vertex")) {
      /* Alloc */
      BlockVertex *pVertex = new BlockVertex(Vector2f(atof( XML::getOption(pj, "x", "0").c_str()),
                                                      atof( XML::getOption(pj, "y", "0").c_str())),
                                             XML::getOption(pj, "edge", ""));

      if(firstVertex == true){
	firstVertex = false;	
      }
      else {
	if(lastX == pVertex->Position().x
	   && lastY == pVertex->Position().y){
	  delete pVertex;
	  continue;
	}
      }

      /* Add it */
      pBlock->Vertices().push_back( pVertex );
      lastX = pVertex->Position().x;
      lastY = pVertex->Position().y;
    }
  }

  return pBlock;
}

void Block::saveBinary(FileHandle *i_pfh) {
      FS::writeString(i_pfh,   Id());
      FS::writeBool(i_pfh,     isBackground());
      FS::writeBool(i_pfh,     isDynamic());
      FS::writeBool(i_pfh,     isPhysics());
      FS::writeBool(i_pfh,     isLayer());
      FS::writeInt_LE(i_pfh,   getLayer());
      FS::writeString(i_pfh,   Texture());
      FS::writeFloat_LE(i_pfh, TextureScale());
      
      FS::writeInt_LE(i_pfh, m_blendColor.Red());
      FS::writeInt_LE(i_pfh, m_blendColor.Green());
      FS::writeInt_LE(i_pfh, m_blendColor.Blue());
      FS::writeInt_LE(i_pfh, m_blendColor.Alpha());
      
      FS::writeFloat_LE(i_pfh, InitialPosition().x);
      FS::writeFloat_LE(i_pfh, InitialPosition().y);
      FS::writeFloat_LE(i_pfh, GripPer20());
      FS::writeFloat_LE(i_pfh, Mass());
      FS::writeFloat_LE(i_pfh, Friction());
      FS::writeFloat_LE(i_pfh, Elasticity());
      FS::writeInt_LE(i_pfh,   getEdgeDrawMethod());
      FS::writeFloat_LE(i_pfh, edgeAngle());
      FS::writeInt_LE(i_pfh,   getCollisionMethod());
      FS::writeFloat_LE(i_pfh, getCollisionRadius());

      FS::writeShort_LE(i_pfh, Vertices().size());
        
      for(unsigned int j=0; j<Vertices().size();j++) {
        FS::writeFloat_LE(i_pfh, Vertices()[j]->Position().x);
        FS::writeFloat_LE(i_pfh, Vertices()[j]->Position().y);
        FS::writeString(i_pfh,   Vertices()[j]->EdgeEffect());
      }   
}

Block* Block::readFromBinary(FileHandle *i_pfh) {
  Block *pBlock = new Block(FS::readString(i_pfh));

  pBlock->setBackground(FS::readBool(i_pfh));
  pBlock->setDynamic(FS::readBool(i_pfh));
  pBlock->setPhysics(FS::readBool(i_pfh));
  pBlock->setIsLayer(FS::readBool(i_pfh));
  pBlock->setLayer(FS::readInt_LE(i_pfh));
  pBlock->setTexture(FS::readString(i_pfh));
  pBlock->setTextureScale(FS::readFloat_LE(i_pfh));

  TColor v_blendColor = TColor(255,255,255,255);
  v_blendColor.setRed(FS::readInt_LE(i_pfh));
  v_blendColor.setGreen(FS::readInt_LE(i_pfh));
  v_blendColor.setBlue(FS::readInt_LE(i_pfh));
  v_blendColor.setAlpha(FS::readInt_LE(i_pfh));
  pBlock->setBlendColor(v_blendColor);

  Vector2f v_Position;
  v_Position.x = FS::readFloat_LE(i_pfh);
  v_Position.y = FS::readFloat_LE(i_pfh);
  pBlock->setInitialPosition(v_Position);
  pBlock->setGripPer20(FS::readFloat_LE(i_pfh));
  pBlock->setMass(FS::readFloat_LE(i_pfh));
  pBlock->setFriction(FS::readFloat_LE(i_pfh));
  pBlock->setElasticity(FS::readFloat_LE(i_pfh));
  pBlock->setEdgeDrawMethod((EdgeDrawMethod)FS::readInt_LE(i_pfh));
  pBlock->setEdgeAngle(FS::readFloat_LE(i_pfh));
  pBlock->setCollisionMethod((CollisionMethod)FS::readInt_LE(i_pfh));
  pBlock->setCollisionRadius(FS::readFloat_LE(i_pfh));
  
  int nNumVertices = FS::readShort_LE(i_pfh);
  pBlock->Vertices().reserve(nNumVertices);
  for(int j=0;j<nNumVertices;j++) {
    Vector2f v_Position;
    v_Position.x = FS::readFloat_LE(i_pfh);
    v_Position.y = FS::readFloat_LE(i_pfh);
    std::string v_EdgeEffect = FS::readString(i_pfh);
    pBlock->Vertices().push_back(new BlockVertex(v_Position, v_EdgeEffect));
  }

  return pBlock;
}

void Block::addEdgeGeom(int geom)
{
  m_edgeGeoms.push_back(geom);
}

std::vector<int>& Block::getEdgeGeoms()
{
  return m_edgeGeoms;
}

void Block::calculateEdgePosition_angle(Vector2f  i_vA,  Vector2f i_vB,
					Vector2f& o_a1,  Vector2f& o_b1,
					Vector2f& o_b2,  Vector2f& o_a2,
					float i_border,  float i_depth,
					Vector2f center, float i_angle)
{
  // welcome the allmighty sin and cos
  float radAngle  = deg2rad(i_angle);
  float cosfAngle = cosf(radAngle);
  float sinfAngle = sinf(radAngle);
  Vector2f borderV(cosfAngle*i_border, sinfAngle*i_border);
  Vector2f depthV( cosfAngle*i_depth,  sinfAngle*i_depth);
  o_a1 = i_vA + center - borderV;
  o_b1 = i_vB + center - borderV;
  o_b2 = i_vB + center + depthV;
  o_a2 = i_vA + center + depthV;
}

void Block::calculateEdgePosition_inout(Vector2f i_vA1,  Vector2f i_vB1, Vector2f i_vC1,
					Vector2f& o_a1,  Vector2f& o_b1,
					Vector2f& o_b2,  Vector2f& o_a2,
					Vector2f& o_c1,  Vector2f& o_c2,
					float i_border,  float i_depth,
					Vector2f center,
					Vector2f i_oldC2, Vector2f i_oldB2,
					bool i_useOld, bool i_AisLast,
					bool& o_swapDone, bool i_inside)
{
  /* we want to calculate the point on the normal of vector(vA1,vB1)
     which is at the distance depth of vA1. let's call that point vA2

     as vector(vA1,vB1) and vector(vA1,vA2) are perpendicular, their
     dot product equals zero
     (vB1.x-vA1.x)*(vA2.x-vA1.x)+(vB1.y-vA1.y)*(vA2.y-vA1.y) = 0

     vA2 is on the circle of center vA1 and radius depth
     (vA2.x-vA1.x)^2 + (vA2.y-vA1.y)^2 = depth^2

     from this two equations and a ti89, we find the values of vA2.x
     and vA2.y, which are tooooo big expressions, so, let's ask Avova
     the maths teacher from the irc channel:

     vA2 = vA1 +/- t*N
     t   = depth / length(Nab)
     Nab = normal(vB1-vA1) = vector(-vB1.y+vA1.y, vB1.x-vA1.x)
     and the equation is ((vA1+t*Nab)-vA1)^2 = depth^2

     which gives:

     Nab.x  = -vB1.y+vA1.y
     Nab.y  = vB1.x-vA1.x
     t    = depth / sqrt((-vB1.y+vA1.y)^2 + (vB1.x-vA1.x)^2)
     vA2.x = vA1.x +/- t * (Nab.x)
     vA2.y = vA1.y +/- t * (Nab.y)

     far easier !

     depending on the (A1, B1, C1) angle, there's gaps and overlaps
     between two consecutive edges, so we fix it by calculating B2 as
     the intersection of (A2, AB2) and (C2, CB2) let's use Cramer
     formula to calculate it. (TODO::a nice graphic to explain
     it). point AB2 is on the normal of vector(A1,B1) and point CB2 is
     on the normal of vector(B1,C1), both are at distance depth on the
     normal.

     and there can be a problem if lines (A1,A2) and (B1,B2)
     intersects in a way that the resulting quad is concave

     the code actually doesn't work well... it's some wip
  */
  if(i_inside == true){
    // inside
    o_swapDone = false;

    o_a1 = i_vA1 + Vector2f(center.x, center.y + i_border);
    o_b1 = i_vB1 + Vector2f(center.x, center.y + i_border);
    o_c1 = i_vC1 + Vector2f(center.x, center.y + i_border);

    Vector2f ab2;
    if(i_useOld == true){
      // oldB2 is new A2 and oldC2 is new AB2
      o_a2 = i_oldB2;
      ab2  = i_oldC2;
    } else {
      Vector2f vA2;
      Vector2f vAB2;
      calculatePointOnNormal(i_vA1, i_vB1, i_depth, true, vA2, vAB2);
      o_a2 = vA2  + center;
      ab2  = vAB2 + center;
    }

    if(i_AisLast == true){
      o_b2 = ab2;
      // we don't calculate c2, but we don't care as A is last
    }
    else {
      Vector2f bc2;
      Vector2f vC2;
      Vector2f vBC2;
      calculatePointOnNormal(i_vB1, i_vC1, i_depth, true, vBC2, vC2);
      bc2  = vBC2 + center;
      o_c2 = vC2 + center;

      intersectLineLine2fCramer(o_a2, ab2, bc2, o_c2, o_b2);
      // line (b1,b2) lenght has to be i_depth
      //      calculatePointOnVector(o_b1, o_b2, i_depth, o_b2);
    }

#if 0    
    // it's possible that the resulting quad is concave, so we have to
    // calculate the intersection point of lines (A1, A2) and (B1, B2)
    Vector2f inter;
    intersectLineLine2fCramer(o_a1, o_a2, o_b1, o_b2, inter);
    AABB aabbA;
    aabbA.addPointToAABB2f(o_a1);
    aabbA.addPointToAABB2f(o_a2);
    AABB aabbB;
    aabbB.addPointToAABB2f(o_b1);
    aabbB.addPointToAABB2f(o_b2);
    LogDebug("(%f,%f) (%f,%f) (%f,%f) (%f,%f) inter: (%f,%f)",
		o_a1.x, o_a1.y, o_a2.x, o_a2.y,
		o_b1.x, o_b1.y, o_b2.x, o_b2.y,
		inter.x, inter.y);
    LogDebug("aabbA: (%f,%f)(%f,%f)",
		aabbA.getBMin().x, aabbA.getBMin().y,
		aabbA.getBMax().x, aabbA.getBMax().y);
    LogDebug("aabbB: (%f,%f)(%f,%f)",
		aabbB.getBMin().x, aabbB.getBMin().y,
		aabbB.getBMax().x, aabbB.getBMax().y);
    if(aabbA.pointTouchAABB2f(inter) == true && aabbB.pointTouchAABB2f(inter) == true){
      Vector2f tmp = o_a2;
      o_a2 = o_b2;
      o_b2 = tmp;
      o_swapDone = true;
      LogDebug("swap done");
    }
#endif
  } else {
    // outside
  }
}

Block::EdgeDrawMethod Block::getEdgeDrawMethod()
{
  return m_edgeDrawMethod;
}

float Block::edgeAngle()
{
  return m_edgeAngle;
}

Block::CollisionMethod Block::getCollisionMethod()
{
  return m_collisionMethod;
}

float Block::getCollisionRadius()
{
  return m_collisionRadius;
}

void Block::setEdgeDrawMethod(EdgeDrawMethod method)
{
  m_edgeDrawMethod = method;
}

void Block::setEdgeAngle(float angle)
{
  m_edgeAngle = angle;
}

Block::EdgeDrawMethod Block::stringToEdge(std::string method)
{
  if(method == "inside")
    return inside;
  else if(method == "outside")
    return outside;
  else
    return angle;
}

Block::CollisionMethod Block::stringToColMethod(std::string method)
{
  if(method == "Circle")
    return Circle;
  else
    return None;
}

void Block::setBlendColor(const TColor& i_blendColor)
{
  m_blendColor = i_blendColor;
}

const TColor& Block::getBlendColor() const
{
  return m_blendColor;
}
