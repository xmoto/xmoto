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

#ifndef __BLOCK_H__
#define __BLOCK_H__

namespace vapp{
  class CollisionSystem;
  class Line;
}

#include "../helpers/VMath.h"
#include "../helpers/Color.h"
#include "../BSP.h"

#define XM_DEFAULT_BLOCK_TEXTURE "default"
#define XM_DEFAULT_PHYS_BLOCK_GRIP 20

class Block;

/*===========================================================================
  Convex block vertex
  ===========================================================================*/
class ConvexBlockVertex {
  public:
  ConvexBlockVertex(const Vector2f& i_position, const Vector2f& i_texturePosition);
  ~ConvexBlockVertex();

  Vector2f Position() const;
  Vector2f TexturePosition() const;
  void  setPosition(const Vector2f& i_position);

  private:
  Vector2f m_position;         /* Position of vertex */
  Vector2f m_texturePosition;  /* Posistion of the texture */
};

/*===========================================================================
  Convex block
  ===========================================================================*/
class ConvexBlock {
 public:
  ConvexBlock(Block *i_srcBlock = NULL);
  ~ConvexBlock();
  
  std::vector<ConvexBlockVertex *>& Vertices();
  Block* SourceBlock() const;

  void addVertex(const Vector2f& i_position, const Vector2f& i_texturePosition);
  void Move(const Vector2f& i_vector);

 private:
  std::vector<ConvexBlockVertex *> m_vertices; /* Vertices */
  Block *m_srcBlock;                           /* Source block */
};

class BlockVertex {
 public:
  BlockVertex(const Vector2f& i_position, const std::string& i_edgeEffect = "");
  ~BlockVertex();

  Vector2f Position() const;
  std::string EdgeEffect() const; /* edge from this vertex to the following */

  void setTexturePosition(const Vector2f& i_texturePosition);
  void setColor(const TColor& i_color);

 private:
  Vector2f m_position;
  Vector2f m_texturePosition;
  std::string m_edgeEffect;
  TColor m_color;
};

class Block {
 public:
  Block(std::string i_id);
  ~Block();

  std::string Id() const;
  std::string Texture() const;
  Vector2f InitialPosition() const;
  Vector2f DynamicPosition() const;
  float DynamicRotation() const;
  Vector2f DynamicRotationCenter() const;
  Vector2f DynamicPositionCenter() const;
  bool isBackground() const;
  bool isDynamic() const;
  float Grip() const;
  float TextureScale() const;
  std::vector<BlockVertex *>& Vertices();
  std::vector<ConvexBlock *>& ConvexBlocks();

  void setTexture(const std::string& i_texture);
  void setTextureScale(float i_textureScale);
  void setInitialPosition(const Vector2f& i_initialPosition);
  void setBackground(bool i_background);
  void setDynamic(bool i_dynamic);
  void setGrip(float i_grip);
  void setCenter(const Vector2f& i_center);

	/* position where to display ; do not consider block center */
  void setDynamicPosition(const Vector2f& i_dynamicPosition);

	/* postion where to display - the center */
  void setDynamicPositionAccordingToCenter(const Vector2f& i_dynamicPosition);

	/* angle position ; consider of the block center */
  void setDynamicRotation(float i_dynamicRotation);

  int loadToPlay(vapp::CollisionSystem& io_collisionSystem,
		 bool manageCollisions); /* load for playing */
  void unloadToPlay();

  void saveXml(vapp::FileHandle *i_pfh);
  void saveBinary(vapp::FileHandle *i_pfh);
  static Block* readFromXml(vapp::XMLDocument& i_xmlSource, TiXmlElement *pElem);
  static Block* readFromBinary(vapp::FileHandle *i_pfh);

private:
  std::string m_id;           /* Block ID */
  std::string m_texture;      /* Texture to use... */
  float m_textureScale;       /* Texture scaling constant to use */
  Vector2f m_initialPosition; /* Position */
  float m_initialRotation;       

  std::vector<BlockVertex *> m_vertices;     /* Vertices of block */
  std::vector<ConvexBlock *> m_convexBlocks; /* Convex Blocks */

  bool  m_background;                   /* Background block */
  bool  m_dynamic;
  float m_grip;                         /* GRIP of the block */

  /* properties for dynamic */
  float m_dynamicRotation;  /* Block rotation */
  Vector2f m_dynamicRotationCenter;
  Vector2f m_dynamicPositionCenter;
  Vector2f m_dynamicPosition; /* Block position */
  std::vector<vapp::Line *> m_collisionLines; /* Line to collide against */

  void addPoly(const vapp::BSPPoly& i_poly, vapp::CollisionSystem& io_collisionSystem);
  void updateCollisionLines();
};

#endif /* __BLOCK_H__ */
