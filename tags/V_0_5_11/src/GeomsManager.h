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

#include <vector>
#include <string>
#include "helpers/Singleton.h"
#include "helpers/Color.h"
#include "helpers/VMath.h"

class Texture;
class Scene;
class Block;
class ConvexBlock;
class BlockVertex;
class EdgeEffectSprite;

struct GeomCoord {
  float x,y;
};

struct GeomPoly {
  GeomPoly() {
    nNumVertices = 0;
    pVertices = pTexCoords = NULL;
    nVertexBufferID = nTexCoordBufferID = 0;
  }
    
  unsigned int nNumVertices;
  GeomCoord *pVertices;
  GeomCoord *pTexCoords;
  
  unsigned int nVertexBufferID,nTexCoordBufferID;
};

struct Geom {
  Geom();

  std::string material;
  std::vector<GeomPoly *> Polys;    
  Texture *pTexture;           // only used for edge Geoms
  EdgeEffectSprite* pSprite;   // only used for edge Geoms
  TColor edgeBlendColor;    
  Vector2f Min,Max; /* AABB */
  bool isUpper;
};

struct BlockGeoms {
  Geom* gmain;
  std::vector<Geom*> gedges;
};

class LevelGeoms {
 public:
  LevelGeoms(const std::string& i_levelId);
  ~LevelGeoms();

  BlockGeoms getBlockGeom(Block* pBlock, unsigned int i_blockIndex, unsigned int& o_geomBytes);
  std::string getLevelId();
  unsigned int getNumberOfRegisteredScenes();
  unsigned int getNumberOfBlockGeoms() const;
  unsigned int getNumberOfEdgeGeoms() const;

  // for GeomsMangager use only :
  void register_scene(Scene* i_scene);
  void unregister_scene(Scene* i_scene);

 private:
  Geom* loadBlockGeom(Block* pBlock, const Vector2f& Center, unsigned int& o_geomBytes);
  std::vector<Geom*> loadBlockEdgeGeom(Block* pBlock, Vector2f Center, unsigned int& o_geomBytes);
  Geom* edgeGeomExists(std::vector<Geom*>& i_geoms, Block* pBlock, std::string material, bool i_isUpper);
  void deleteGeoms(std::vector<Geom *>& geom, bool useFree=false);
  void saveGeoms(Scene* i_scene);

  static void calculateEdgePosition(Block* pBlock,
				    BlockVertex* vertexA1,
				    BlockVertex* vertexB1,
				    BlockVertex* vertexC1,
				    Vector2f     center,
				    Vector2f& A1, Vector2f& B1,
				    Vector2f& B2, Vector2f& A2,
				    Vector2f& C1, Vector2f& C2,
				    Vector2f oldC2, Vector2f oldB2, bool useOld,
				    bool AisLast, bool& swapDone,
				    float i_edgeDepth,
				    float i_edgeMaterialDepth);
  static void calculateEdgeTexture(Block* pBlock,
				   Vector2f A1,   Vector2f B1,
				   Vector2f B2,   Vector2f A2,
				   Vector2f& ua1, Vector2f& ub1,
				   Vector2f& ub2, Vector2f& ua2,
				   float i_edgeScale,
				   float i_edgeMaterialScale);

  std::string m_levelId;
  std::vector<Scene* > m_scenes; // scenes using these geoms
  std::vector<Geom*> m_blockGeoms;
  std::vector<Geom*> m_edgeGeoms;
  bool m_geomsLoaded; // once a level has load all the geoms, this is true and all geoms can be reused for new levels
  std::vector<BlockGeoms> m_savedBlockGeoms;
};

class GeomsManager : public Singleton<GeomsManager> {
  friend class Singleton<GeomsManager>;
  public:
  GeomsManager();
  ~GeomsManager();

  // registration
  LevelGeoms* register_begin(Scene* i_scene);
  void register_end(Scene* i_scene);
  void unregister(Scene* i_scene);

  unsigned int getNumberOfBlockGeoms() const;
  unsigned int getNumberOfEdgeGeoms() const;

  private:
  LevelGeoms* getLevelGeom(Scene* i_scene);
  std::vector<LevelGeoms*> m_levelGeoms;
};
