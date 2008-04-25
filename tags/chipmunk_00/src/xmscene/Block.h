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

#ifndef __BLOCK_H__
#define __BLOCK_H__

class CollisionSystem;
class Line;

#include "helpers/VMath.h"
#include "helpers/Color.h"
#include "ChipmunkHelper.h"

class FileHandle;
class XMLDocument;
class TiXmlElement;
class BSPPoly;
class Block;

#define DEFAULT_EDGE_ANGLE 270.0f

/*===========================================================================
  Convex block vertex
  ===========================================================================*/
class ConvexBlockVertex {
  public:
  ConvexBlockVertex(const Vector2f& i_position, const Vector2f& i_texturePosition);
  ~ConvexBlockVertex();

  /* called many many many times, so we inline it, and make it return a ref */
  inline Vector2f& Position() {
    return m_position;
  }

  /* called many many many times, so we inline it, and make it return a ref */
  inline Vector2f& TexturePosition() {
    return m_texturePosition;
  }

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

  /* called many many many times, so we inline it */
  inline std::vector<ConvexBlockVertex *>& Vertices() {
     return m_vertices;
  }

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
  /* edge from this vertex to the following */
  /* called many many many times, so we inline it, and make it return a ref */
  inline std::string& EdgeEffect() {
    return m_edgeEffect;
  }

  void setPosition(const Vector2f& i_position);

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
  /* called many many many times, so we inline it, and make it return a ref */
  inline Vector2f& DynamicPosition() {
    return m_dynamicPosition;
  }
  float DynamicRotation() const;

  /* called many many many times, so we inline it, and make it return a ref */
  inline Vector2f& DynamicRotationCenter() {
    return m_dynamicRotationCenter;
  }

  Vector2f DynamicPositionCenter() const;
  bool isBackground() const;
  bool isDynamic() const;
  bool isChipmunk() const;
  bool isLayer() const;
  float Grip() const;
  float TextureScale() const;
  std::vector<BlockVertex*>& Vertices();
  /* called many many many times, so we inline it */
  inline std::vector<ConvexBlock*>& ConvexBlocks() {
    return m_convexBlocks;
  }
  typedef enum{angle, inside, outside} EdgeDrawMethod;
  EdgeDrawMethod getEdgeDrawMethod();
  float edgeAngle();

  void setTexture(const std::string& i_texture);
  void setTextureScale(float i_textureScale);
  void setInitialPosition(const Vector2f& i_initialPosition);
  void setBackground(bool i_background);
  void setDynamic(bool i_dynamic);
  void setChipmunk(bool i_dynamic);
  void setIsLayer(bool i_isLayer);
  void setGrip(float i_grip);
  void setCenter(const Vector2f& i_center);
  void setEdgeDrawMethod(EdgeDrawMethod method);
  // in degres, not radian
  void setEdgeAngle(float angle);

  /* position where to display ; do not consider block center */
  void setDynamicPosition(const Vector2f& i_dynamicPosition);
  /* postion where to display - the center */
  void setDynamicPositionAccordingToCenter(const Vector2f& i_dynamicPosition);
  /* angle position ; consider of the block center */
  void setDynamicRotation(float i_dynamicRotation);

  int loadToPlay(CollisionSystem& io_collisionSystem);
  void unloadToPlay();

  void saveXml(FileHandle *i_pfh);
  void saveBinary(FileHandle *i_pfh);
  static Block* readFromXml(XMLDocument* i_xmlSource, TiXmlElement *pElem);
  static Block* readFromBinary(FileHandle *i_pfh);
  AABB& getAABB();
  std::vector<Line *>& getCollisionLines() {return m_collisionLines;}

  int getGeom() {
    return m_geom;
  }
  void setGeom(int geom) {
    m_geom = geom;
  }
  int getLayer() {
    return m_layer;
  }
  void setLayer(int layer) {
    m_layer = layer;
  }

  void addEdgeGeom(int geom);
  std::vector<int>& getEdgeGeoms();
  bool edgeGeomExists(std::string texture);


  // calculate edge position
  void calculateEdgePosition_angle(Vector2f  i_vA, Vector2f i_vB,
				   Vector2f& o_a1, Vector2f& o_b1,
				   Vector2f& o_b2, Vector2f& o_a2,
				   float i_border, float i_depth,
				   Vector2f  i_center, float i_angle);
  void calculateEdgePosition_inout(Vector2f  i_vA1, Vector2f i_vB1, Vector2f i_vC1,
				    Vector2f& o_a1,  Vector2f& o_b1,
				    Vector2f& o_b2,  Vector2f& o_a2,
				    Vector2f& o_c1,  Vector2f& o_c2,
				    float i_border,  float i_depth,
				    Vector2f  center,
				    Vector2f  i_oldC2, Vector2f i_oldB2,
				    bool i_useOld, bool i_AisLast,
				    bool& o_swapDone, bool i_inside);

  cpBody *mBody;

private:
  std::string m_id;
  std::string m_texture;
  float       m_textureScale;
  Vector2f    m_initialPosition;
  float       m_initialRotation;       

  std::vector<BlockVertex*> m_vertices;
  std::vector<ConvexBlock*> m_convexBlocks;
  // one geom for each edge texture
  std::vector<int> m_edgeGeoms;

  bool  m_background;
  bool  m_dynamic;
  bool  m_chipmunk;
  bool  m_isLayer;
  // the background layer of the block.
  // in case of a non m_backgroundLevel, if m_layer is != -1, then it's
  // a static block from the second static blocks layer
  int   m_layer;
  float m_grip;                         /* GRIP of the block */
  AABB  m_BBox;
  bool  m_isBBoxDirty;

  // the geom used to render the block
  int   m_geom;

  /* properties for dynamic */
  float m_dynamicRotation;  /* Block rotation */
  Vector2f m_dynamicRotationCenter;
  Vector2f m_dynamicPositionCenter;
  Vector2f m_dynamicPosition; /* Block position */
  std::vector<Line *> m_collisionLines; /* Line to collide against */

  void addPoly(BSPPoly* i_poly, CollisionSystem& io_collisionSystem);
  void updateCollisionLines();

  EdgeDrawMethod m_edgeDrawMethod;
  float m_edgeAngle;

  std::string edgeToString(EdgeDrawMethod method);
  EdgeDrawMethod stringToEdge(std::string method);
};

#endif /* __BLOCK_H__ */
