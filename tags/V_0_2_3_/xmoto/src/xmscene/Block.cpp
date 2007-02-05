/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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

#include "Block.h"

/* Vertex */
ConvexBlockVertex::ConvexBlockVertex(const Vector2f& i_position, const Vector2f& i_texturePosition) {
  m_position         = i_position;
  m_texturePosition  = i_texturePosition;
}

ConvexBlockVertex::~ConvexBlockVertex() {
}

Vector2f ConvexBlockVertex::Position() const {
  return m_position;
}

Vector2f ConvexBlockVertex::TexturePosition()  const {
  return m_texturePosition;
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

std::vector<ConvexBlockVertex *>& ConvexBlock::Vertices() {
  return m_vertices;
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
  m_grip             = XM_DEFAULT_PHYS_BLOCK_GRIP;
  m_dynamicPosition  = m_initialPosition;
  m_dynamicRotation  = m_initialRotation;
  m_texture          = XM_DEFAULT_BLOCK_TEXTURE;
}

Block::~Block() {
  /* delete vertices */
  for(unsigned int i=0; i<m_vertices.size(); i++) {
    delete m_vertices[i];
  }

  /* delete convex blocks */
  for(unsigned int i=0; i<m_convexBlocks.size(); i++) {
    delete m_convexBlocks[i];
  }

  /* delete collision lines */
  for(unsigned int i=0; i<m_collisionLines.size(); i++) {
    delete m_collisionLines[i];
  }
}

void Block::setCenter(const Vector2f& i_center) {
  /* Correct all polygons in block to this new center */
  for(unsigned int i=0; i<ConvexBlocks().size(); i++) {
    ConvexBlocks()[i]->Move(-i_center);
  }
  setDynamicPosition(DynamicPosition() + i_center);
}

float Block::DynamicRotation() const {
  return m_dynamicRotation;
}

Vector2f Block::DynamicPosition() const {
  return m_dynamicPosition;
}

void Block::updateCollisionLines() {
  /* Ignore background blocks */
  if(isBackground() || isDynamic() == false) return;

  /* Build rotation matrix for block */
  float fR[4]; 
  fR[0] =  cos(DynamicRotation());
  fR[1] = -sin(DynamicRotation());
  fR[2] =  sin(DynamicRotation()); 
  fR[3] =  cos(DynamicRotation());
  unsigned int z = 0;
  
  for(unsigned int i=0; i< m_convexBlocks.size(); i++) {       
    for(unsigned int j=0; j<m_convexBlocks[i]->Vertices().size(); j++) {            
      unsigned int jnext = j==m_convexBlocks[i]->Vertices().size()-1? 0 : j+1;
      ConvexBlockVertex *pVertex1 = m_convexBlocks[i]->Vertices()[j];          
      ConvexBlockVertex *pVertex2 = m_convexBlocks[i]->Vertices()[jnext];          
      
      /* Transform vertices */
      Vector2f Tv1 = Vector2f(pVertex1->Position().x * fR[0] + pVertex1->Position().y * fR[1],
            pVertex1->Position().x * fR[2] + pVertex1->Position().y * fR[3]);
      Tv1 += m_dynamicPosition;
      
      Vector2f Tv2 = Vector2f(pVertex2->Position().x * fR[0] + pVertex2->Position().y * fR[1],
            pVertex2->Position().x * fR[2] + pVertex2->Position().y * fR[3]);
      Tv2 += m_dynamicPosition;

      /* Update line */
      m_collisionLines[z]->x1 = Tv1.x;
      m_collisionLines[z]->y1 = Tv1.y;
      m_collisionLines[z]->x2 = Tv2.x;
      m_collisionLines[z]->y2 = Tv2.y;
      
      /* Next collision line */
      z++;
    }
  }
}

void Block::setDynamicPosition(const Vector2f& i_dynamicPosition) {
  m_dynamicPosition = i_dynamicPosition;
  updateCollisionLines();
}

void Block::setDynamicRotation(float i_dynamicRotation) {
  m_dynamicRotation = i_dynamicRotation;
  updateCollisionLines();
}

int Block::loadToPlay(vapp::CollisionSystem& io_collisionSystem) {

  m_dynamicPosition  = m_initialPosition;
  m_dynamicRotation  = m_initialRotation;

  /* Do the "convexifying" the BSP-way. It might be overkill, but we'll
     probably appreciate it when the input data is very complex. It'll also 
     let us handle crossing edges, and other kinds of weird input. */
  vapp::BSP v_BSPTree;
  
  /* Define edges */
  for(unsigned int i=0; i<Vertices().size(); i++) {
    /* Next vertex? */
    unsigned int inext = i+1;
    if(inext == Vertices().size()) inext=0;
    
    /* add static lines */
    if(isBackground() == false && isDynamic() == false) {
      /* Add line to collision handler */
      io_collisionSystem.defineLine(DynamicPosition().x + Vertices()[i]->Position().x,
                                    DynamicPosition().y + Vertices()[i]->Position().y,
                                    DynamicPosition().x + Vertices()[inext]->Position().x,
                                    DynamicPosition().y + Vertices()[inext]->Position().y,
                                    Grip());
    }
    
    /* Add line to BSP generator */
    v_BSPTree.addLineDef(Vertices()[i]->Position(), Vertices()[inext]->Position());
  }
  
  /* Compute */
  std::vector<vapp::BSPPoly *> &v_BSPPolys = v_BSPTree.compute();      
  
  /* Create convex blocks */
  for(unsigned int i=0; i<v_BSPPolys.size(); i++) {
    addPoly(*(v_BSPPolys[i]), io_collisionSystem);
  }
  
  updateCollisionLines();

  if(v_BSPTree.getNumErrors() > 0) {
    vapp::Log("Error due to the block %s", Id().c_str());
  }

  return v_BSPTree.getNumErrors();  
}

void Block::addPoly(const vapp::BSPPoly& i_poly, vapp::CollisionSystem& io_collisionSystem) {
  ConvexBlock *v_block = new ConvexBlock(this);
  
  for(unsigned int i=0; i<i_poly.Vertices.size(); i++) {
    v_block->addVertex(i_poly.Vertices[i]->P,
                       Vector2f((InitialPosition().x + i_poly.Vertices[i]->P.x) * 0.25,
                                (InitialPosition().y + i_poly.Vertices[i]->P.y) * 0.25));
  }
  m_convexBlocks.push_back(v_block);

  /* add dynamic lines */
  if(isBackground() == false && isDynamic()) {
    /* Define collision lines */
    for(unsigned int i=0; i<i_poly.Vertices.size(); i++) {
      vapp::Line *v_line = new vapp::Line;
      v_line->x1 = v_line->y1 = v_line->x2 = v_line->y2 = 0.0f;
      v_line->fGrip = m_grip;
      m_collisionLines.push_back(v_line);
      io_collisionSystem.addExternalDynamicLine(v_line);
    }
  }
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

std::string Block::Id() const {
  return m_id;
}

bool Block::isBackground() const {
  return m_background;
}

bool Block::isDynamic() const {
  return m_dynamic;
}

void Block::setInitialPosition(const Vector2f& i_initialPosition) {
  m_initialPosition  = i_initialPosition;
}

Vector2f Block::InitialPosition() const {
  return m_initialPosition;
}

float Block::Grip() const {
  return m_grip;
}

std::string Block::Texture() const {
  return m_texture;
}

float Block::TextureScale() const {
  return m_textureScale;
}

std::vector<BlockVertex *>& Block::Vertices() {
  return m_vertices;
}

std::vector<ConvexBlock *>& Block::ConvexBlocks() {
  return m_convexBlocks;
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

void Block::setGrip(float i_grip) {
  m_grip = i_grip;
}

BlockVertex::BlockVertex(const Vector2f& i_position, const std::string& i_edgeEffect) {
  m_position   = i_position;
  m_edgeEffect = i_edgeEffect;
}

BlockVertex::~BlockVertex() {
}

std::string BlockVertex::EdgeEffect() const {
  return m_edgeEffect;
}

Vector2f BlockVertex::Position() const {
  return m_position;
}

void BlockVertex::setTexturePosition(const Vector2f& i_texturePosition) {
  m_texturePosition = i_texturePosition;
}

void BlockVertex::setColor(const TColor &i_color) {
  m_color = i_color;
}

void Block::saveXml(vapp::FileHandle *i_pfh) {
  vapp::FS::writeLineF(i_pfh,"\t<block id=\"%s\">", Id().c_str());
    
  std::string v_position;
  std::ostringstream v_x, v_y;
  v_x << InitialPosition().x;
  v_y << InitialPosition().y;
  
  v_position = "\t\t<position x=\"" + v_x.str() + "\" y=\"" + v_y.str() + "\"";
  
  if(isBackground()) {
    v_position = v_position + " background=\"true\"";
  }
  
  if(isDynamic()) {
    v_position = v_position + " dynamic=\"true\"";
  }
  vapp::FS::writeLineF(i_pfh, (char*) v_position.c_str());
  
  if(Grip() != DEFAULT_PHYS_WHEEL_GRIP) {
    vapp::FS::writeLineF(i_pfh,"\t\t<physics grip=\"%f\"/>", Grip());
  }
  vapp::FS::writeLineF(i_pfh,"\t\t<usetexture id=\"%s\"/>", Texture().c_str());
  for(unsigned int j=0;j<Vertices().size();j++) {
    if(Vertices()[j]->EdgeEffect() != "")
      vapp::FS::writeLineF(i_pfh,"\t\t<vertex x=\"%f\" y=\"%f\" edge=\"%s\"/>",
                           Vertices()[j]->Position().x,
                           Vertices()[j]->Position().y,
                           Vertices()[j]->EdgeEffect().c_str());
    else
      vapp::FS::writeLineF(i_pfh,"\t\t<vertex x=\"%f\" y=\"%f\"/>",
                           Vertices()[j]->Position().x,
                           Vertices()[j]->Position().y);
  }
  vapp::FS::writeLineF(i_pfh,"\t</block>");
}

Block* Block::readFromXml(vapp::XMLDocument& i_xmlSource, TiXmlElement *pElem) {
  
  Block *pBlock = new Block(vapp::XML::getOption(pElem, "id"));
  pBlock->setTexture("default");
        
  TiXmlElement *pUseTextureElem = vapp::XML::findElement(i_xmlSource, pElem,std::string("usetexture"));
  TiXmlElement *pPositionElem   = vapp::XML::findElement(i_xmlSource, pElem,std::string("position"));                
  TiXmlElement *pPhysicsElem    = vapp::XML::findElement(i_xmlSource, pElem,std::string("physics"));          
        
  if(pUseTextureElem != NULL) {
    pBlock->setTexture(vapp::XML::getOption(pUseTextureElem,"id", "default"));
    pBlock->setTextureScale(atof(vapp::XML::getOption(pUseTextureElem,"scale","1").c_str()));
  }
   
  if(pPositionElem != NULL) {
    pBlock->setInitialPosition(Vector2f(atof( vapp::XML::getOption(pPositionElem,"x","0").c_str() ),
                                        atof( vapp::XML::getOption(pPositionElem,"y","0").c_str() )
                                        )
                               );
          
    pBlock->setBackground(vapp::XML::getOption(pPositionElem,"background","false") == "true");
    pBlock->setDynamic(vapp::XML::getOption(pPositionElem,"dynamic","false") == "true");
  }
   
  if(pPhysicsElem != NULL) {
    char str[5];
    snprintf(str, 5, "%f", DEFAULT_PHYS_WHEEL_GRIP);
    pBlock->setGrip(atof(vapp::XML::getOption(pPhysicsElem, "grip", str).c_str()));
  } else {
    pBlock->setGrip(DEFAULT_PHYS_WHEEL_GRIP);
  }
  
  /* Get vertices */
  for(TiXmlElement *pj = pElem->FirstChildElement(); pj!=NULL; pj=pj->NextSiblingElement()) {
    if(!strcmp(pj->Value(),"vertex")) {
      /* Alloc */
      BlockVertex *pVertex = new BlockVertex(Vector2f(atof( vapp::XML::getOption(pj,"x","0").c_str() ),
                                                      atof( vapp::XML::getOption(pj,"y","0").c_str() )),
                                             vapp::XML::getOption(pj,"edge",""));
      
      std::string k;
      Vector2f v_TexturePosition;
      k = vapp::XML::getOption(pj,"tx","");
      if(k != "") {
        v_TexturePosition.x = atof( k.c_str() );
      } else {
        v_TexturePosition.x = pVertex->Position().x * pBlock->TextureScale();
      }
      
      k = vapp::XML::getOption(pj,"ty","");
      if(k != "") {
        v_TexturePosition.y = atof( k.c_str() );
      } else {
        v_TexturePosition.y = pVertex->Position().y * pBlock->TextureScale();
      }
      pVertex->setTexturePosition(v_TexturePosition);
      
      TColor v_color = TColor(atoi(vapp::XML::getOption(pj,"r","255").c_str()),
                              atoi(vapp::XML::getOption(pj,"g","255").c_str()),
                              atoi(vapp::XML::getOption(pj,"b","255").c_str()),
                              atoi(vapp::XML::getOption(pj,"a","255").c_str())
                              );
      pVertex->setColor(v_color);
      
      /* Add it */
      pBlock->Vertices().push_back( pVertex );
    }
  }

  return pBlock;
}

void Block::saveBinary(vapp::FileHandle *i_pfh) {
      vapp::FS::writeString(i_pfh,   Id());
      vapp::FS::writeBool(i_pfh,     isBackground());
      vapp::FS::writeBool(i_pfh,     isDynamic());
      vapp::FS::writeString(i_pfh,   Texture());
      vapp::FS::writeFloat_LE(i_pfh, InitialPosition().x);
      vapp::FS::writeFloat_LE(i_pfh, InitialPosition().y);
      vapp::FS::writeFloat_LE(i_pfh, Grip());

      vapp::FS::writeShort_LE(i_pfh, Vertices().size());
        
      for(unsigned int j=0; j<Vertices().size();j++) {
        vapp::FS::writeFloat_LE(i_pfh, Vertices()[j]->Position().x);
        vapp::FS::writeFloat_LE(i_pfh, Vertices()[j]->Position().y);
        vapp::FS::writeString(i_pfh,   Vertices()[j]->EdgeEffect());
      }   
}

Block* Block::readFromBinary(vapp::FileHandle *i_pfh) {
  Block *pBlock = new Block(vapp::FS::readString(i_pfh));

  pBlock->setBackground(vapp::FS::readBool(i_pfh));
  pBlock->setDynamic(vapp::FS::readBool(i_pfh));
  pBlock->setTexture(vapp::FS::readString(i_pfh));

  Vector2f v_Position;
  v_Position.x = vapp::FS::readFloat_LE(i_pfh);
  v_Position.y = vapp::FS::readFloat_LE(i_pfh);
  pBlock->setInitialPosition(v_Position);
  pBlock->setGrip(vapp::FS::readFloat_LE(i_pfh));
  
  int nNumVertices = vapp::FS::readShort_LE(i_pfh);
  pBlock->Vertices().reserve(nNumVertices);
  for(int j=0;j<nNumVertices;j++) {
    Vector2f v_Position;
    v_Position.x = vapp::FS::readFloat_LE(i_pfh);
    v_Position.y = vapp::FS::readFloat_LE(i_pfh);
    std::string v_EdgeEffect = vapp::FS::readString(i_pfh);
    pBlock->Vertices().push_back(new BlockVertex(v_Position, v_EdgeEffect));
  }

  return pBlock;
}
