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

#include "GeomsManager.h"
#include "Game.h"
#ifdef ENABLE_OPENGL
#include "drawlib/DrawLibOpenGL.h"
#endif
#include "helpers/Log.h"
#include "xmscene/Block.h"

Geom::Geom() {
  pTexture = NULL;
  pSprite = NULL;
}

LevelGeoms::LevelGeoms(const std::string &i_levelId) {
  m_levelId = i_levelId;
  m_geomsLoaded = false;
}

LevelGeoms::~LevelGeoms() {
  // printf("~LevelGeoms(%25s) : blockGeoms = %4i, edgeGeoms = %4i\n",
  // m_levelId.c_str(), m_blockGeoms.size(), m_edgeGeoms.size());

  deleteGeoms(m_blockGeoms);
  deleteGeoms(m_edgeGeoms, true);
}

unsigned int LevelGeoms::getNumberOfRegisteredScenes() {
  return m_scenes.size();
}

std::string LevelGeoms::getLevelId() {
  return m_levelId;
}

void LevelGeoms::register_scene(Scene *i_scene) {
  m_scenes.push_back(i_scene);

  if (m_geomsLoaded == false) {
    m_geomsLoaded = true;
    saveGeoms(i_scene);
  }
}

void LevelGeoms::saveGeoms(Scene *i_scene) {
  BlockGeoms v_bg;

  for (unsigned int i = 0; i < i_scene->getLevelSrc()->Blocks().size(); i++) {
    // save according to the scene blocks index
    v_bg.gmain = i_scene->getLevelSrc()->Blocks()[i]->getGeom();
    v_bg.gedges = i_scene->getLevelSrc()->Blocks()[i]->getEdgeGeoms();
    m_savedBlockGeoms.push_back(v_bg);
  }
}

void LevelGeoms::unregister_scene(Scene *i_scene) {
  unsigned int i = 0;

  while (i < m_scenes.size()) {
    if (m_scenes[i] == i_scene) {
      m_scenes.erase(m_scenes.begin() + i);
    } else {
      i++;
    }
  }
}

unsigned int LevelGeoms::getNumberOfBlockGeoms() const {
  return m_blockGeoms.size();
}

unsigned int LevelGeoms::getNumberOfEdgeGeoms() const {
  return m_edgeGeoms.size();
}

BlockGeoms LevelGeoms::getBlockGeom(Block *pBlock,
                                    unsigned int i_blockIndex,
                                    unsigned int &o_geomBytes) {
  BlockGeoms v_bg;

  o_geomBytes = 0;

  // a such level is already loaded
  if (m_geomsLoaded) {
    v_bg = m_savedBlockGeoms[i_blockIndex];
  } else {
    // the block is not loaded, it must be loaded

    Vector2f Center;
    unsigned int v_geomBytes;

    if (pBlock->isDynamic()) {
      Center.x = 0.0;
      Center.y = 0.0;
    } else {
      Center = pBlock->DynamicPosition();
    }

    v_bg.gmain = loadBlockGeom(pBlock, Center, v_geomBytes);
    o_geomBytes += v_geomBytes;
    v_bg.gedges = loadBlockEdgeGeom(pBlock, Center, v_geomBytes);
    o_geomBytes += v_geomBytes;
  }

  return v_bg;
}

Geom *LevelGeoms::loadBlockGeom(Block *pBlock,
                                const Vector2f &Center,
                                unsigned int &o_geomBytes) {
  o_geomBytes = 0;

  Geom *pSuitableGeom = new Geom();
  m_blockGeoms.push_back(pSuitableGeom);

  for (unsigned int j = 0; j < pBlock->ConvexBlocks().size(); j++) {
    Vector2f v_center = Vector2f(0.0, 0.0);

    GeomPoly *pPoly = new GeomPoly;
    pSuitableGeom->Polys.push_back(pPoly);

    pPoly->nNumVertices = pBlock->ConvexBlocks()[j]->Vertices().size();
    pPoly->pVertices = new GeomCoord[pPoly->nNumVertices];
    pPoly->pTexCoords = new GeomCoord[pPoly->nNumVertices];

    for (unsigned int k = 0; k < pPoly->nNumVertices; k++) {
      pPoly->pVertices[k].x =
        Center.x + pBlock->ConvexBlocks()[j]->Vertices()[k]->Position().x;
      pPoly->pVertices[k].y =
        Center.y + pBlock->ConvexBlocks()[j]->Vertices()[k]->Position().y;
      pPoly->pTexCoords[k].x =
        pBlock->ConvexBlocks()[j]->Vertices()[k]->TexturePosition().x;
      pPoly->pTexCoords[k].y =
        pBlock->ConvexBlocks()[j]->Vertices()[k]->TexturePosition().y;

      v_center += Vector2f(pPoly->pVertices[k].x, pPoly->pVertices[k].y);
    }
    v_center /= pPoly->nNumVertices;

    /* fix the gap problem with polygons */
    float a =
      0.003; /* seems to be a good value, put negativ value to make it worst */
    for (unsigned int k = 0; k < pPoly->nNumVertices; k++) {
      Vector2f V = Vector2f(pPoly->pVertices[k].x - v_center.x,
                            pPoly->pVertices[k].y - v_center.y);
      if (V.length() != 0.0) {
        V.normalize();
        V *= a;
        pPoly->pVertices[k].x += V.x;
        pPoly->pVertices[k].y += V.y;
      }
    }

    o_geomBytes += pPoly->nNumVertices * (4 * sizeof(float));
#ifdef ENABLE_OPENGL
    /* Use VBO optimization? */
    if (GameApp::instance()->getDrawLib()->useVBOs()) {
      /* Copy static coordinates unto video memory */
      ((DrawLibOpenGL *)GameApp::instance()->getDrawLib())
        ->glGenBuffersARB(1, (GLuint *)&pPoly->nVertexBufferID);
      ((DrawLibOpenGL *)GameApp::instance()->getDrawLib())
        ->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nVertexBufferID);
      ((DrawLibOpenGL *)GameApp::instance()->getDrawLib())
        ->glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                          pPoly->nNumVertices * 2 * sizeof(float),
                          (void *)pPoly->pVertices,
                          GL_STATIC_DRAW_ARB);

      ((DrawLibOpenGL *)GameApp::instance()->getDrawLib())
        ->glGenBuffersARB(1, (GLuint *)&pPoly->nTexCoordBufferID);
      ((DrawLibOpenGL *)GameApp::instance()->getDrawLib())
        ->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nTexCoordBufferID);
      ((DrawLibOpenGL *)GameApp::instance()->getDrawLib())
        ->glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                          pPoly->nNumVertices * 2 * sizeof(float),
                          (void *)pPoly->pTexCoords,
                          GL_STATIC_DRAW_ARB);
    }
#endif
  }

  return pSuitableGeom;
}

void LevelGeoms::calculateEdgePosition(Block *pBlock,
                                       BlockVertex *vertexA1,
                                       BlockVertex *vertexB1,
                                       BlockVertex *vertexC1,
                                       Vector2f center,
                                       Vector2f &A1,
                                       Vector2f &B1,
                                       Vector2f &B2,
                                       Vector2f &A2,
                                       Vector2f &C1,
                                       Vector2f &C2,
                                       Vector2f oldC2,
                                       Vector2f oldB2,
                                       bool useOld,
                                       bool AisLast,
                                       bool &swapDone,
                                       float i_edgeDepth,
                                       float i_edgeMaterialDepth) {
  Vector2f vAPos = vertexA1->Position();
  Vector2f vBPos = vertexB1->Position();
  Vector2f vCPos = vertexC1->Position();

  /* link A to B */
  /*get scale value from inksmoto material, if value is set there, if not (-1),
   * get value from theme */
  float fDepth = i_edgeMaterialDepth;
  if (fDepth == DEFAULT_EDGE_DEPTH)
    fDepth = i_edgeDepth;

  // add a small border because polygons are a bit larger to avoid gap polygon
  // pb.
  float v_border;
  if (fDepth > 0)
    v_border = 0.01;
  else
    v_border = -0.01;

  switch (pBlock->getEdgeDrawMethod()) {
    case Block::angle:
      pBlock->calculateEdgePosition_angle(vAPos,
                                          vBPos,
                                          A1,
                                          B1,
                                          B2,
                                          A2,
                                          v_border,
                                          fDepth,
                                          center,
                                          pBlock->edgeAngle());
      break;
    case Block::inside:
      pBlock->calculateEdgePosition_inout(vAPos,
                                          vBPos,
                                          vCPos,
                                          A1,
                                          B1,
                                          B2,
                                          A2,
                                          C1,
                                          C2,
                                          v_border,
                                          fDepth,
                                          center,
                                          oldC2,
                                          oldB2,
                                          useOld,
                                          AisLast,
                                          swapDone,
                                          true);
      break;
    case Block::outside:
      pBlock->calculateEdgePosition_inout(vAPos,
                                          vBPos,
                                          vCPos,
                                          A1,
                                          B1,
                                          B2,
                                          A2,
                                          C1,
                                          C2,
                                          v_border,
                                          fDepth,
                                          center,
                                          oldC2,
                                          oldB2,
                                          useOld,
                                          AisLast,
                                          swapDone,
                                          false);
      break;
  }
}

void LevelGeoms::calculateEdgeTexture(Block *pBlock,
                                      Vector2f A1,
                                      Vector2f B1,
                                      Vector2f B2,
                                      Vector2f A2,
                                      Vector2f &ua1,
                                      Vector2f &ub1,
                                      Vector2f &ub2,
                                      Vector2f &ua2,
                                      float i_edgeScale,
                                      float i_edgeMaterialScale) {
  float fXScale =
    i_edgeMaterialScale; // look if theres a inksmoto material scale defined
  if (fXScale == DEFAULT_EDGE_SCALE)
    fXScale = i_edgeScale;

  switch (pBlock->getEdgeDrawMethod()) {
    case Block::inside:
    case Block::outside: {
      Vector2f N1(-B1.y + A1.y, B1.x - A1.x);

      if (N1.x == 0.0 && N1.y == 0.0) {
        LogWarning("normal is null for block %s vertex (%f,%f)",
                   pBlock->Id().c_str(),
                   A1.x,
                   A1.y);
      }

      N1.normalize();

      ua1.x = A1.x * fXScale * N1.y - A1.y * fXScale * N1.x;
      ub1.x = B1.x * fXScale * N1.y - B1.y * fXScale * N1.x;

      Vector2f N2(-B2.y + A2.y, B2.x - A2.x);

      if (N2.x == 0.0 && N2.y == 0.0) {
        LogWarning("normal is null for block %s vertex (%f,%f)",
                   pBlock->Id().c_str(),
                   A2.x,
                   A2.y);
      }

      N2.normalize();

      ua2.x = A2.x * fXScale * N2.y - A2.y * fXScale * N2.x;
      ub2.x = B2.x * fXScale * N2.y - B2.y * fXScale * N2.x;

      bool drawInside = (pBlock->getEdgeDrawMethod() == Block::inside);
      if (drawInside == true) {
        ua1.y = ub1.y = 0.01;
        ua2.y = ub2.y = 0.99;
      } else {
        ua1.y = ub1.y = 0.99;
        ua2.y = ub2.y = 0.01;
      }
    } break;
    case Block::angle: {
      // all the unwanted children of cos and sin
      float radAngle = deg2rad(pBlock->edgeAngle());
      ua1.x = ua2.x =
        A1.x * fXScale * sinf(radAngle) - A1.y * fXScale * cosf(radAngle);
      ua1.y = ub1.y = 0.01;
      ub1.x = ub2.x =
        B1.x * fXScale * sinf(radAngle) - B1.y * fXScale * cosf(radAngle);
      ua2.y = ub2.y = 0.99;
    } break;
  }
}

// i would have prefer to removed the sprite loading from here,
// but it's easier which the geom.pTexture can be set easily here
// however, it's must be reloaded in case of duplicate of the geom for
// registering in the Renderer
std::vector<Geom *> LevelGeoms::loadBlockEdgeGeom(Block *pBlock,
                                                  Vector2f Center,
                                                  unsigned int &o_geomBytes) {
  std::string v_currentEdgeEffect;
  TColor v_currentEdgeBlendColor;
  EdgeEffectSprite *v_currentEdgeSprite;
  float v_currentEdgeMaterialScale;
  float v_currentEdgeMaterialDepth;

  std::vector<Geom *> v_geoms;

  o_geomBytes = 0;
  //  if(XMSession::instance()->gameGraphics() != GFX_LOW){  //lets load always
  //  all gfx, for beeing able to switch modes ingame

  v_currentEdgeEffect = "";
  v_currentEdgeBlendColor = DEFAULT_EDGE_BLENDCOLOR;
  v_currentEdgeSprite = NULL;
  v_currentEdgeMaterialScale = DEFAULT_EDGE_SCALE;
  v_currentEdgeMaterialDepth = DEFAULT_EDGE_DEPTH;

  Vector2f oldC2, oldB2;
  bool useOld = false;
  bool swapDone = false;

  // create edge texture
  std::vector<BlockVertex *> &vertices = pBlock->Vertices();

  // if the last and the first vertex have edge effect, we have to
  // calculate oldC2 and oldB2
  if (vertices.size() > 1) {
    BlockVertex *lastVertex = vertices[vertices.size() - 1];
    BlockVertex *firstVertex = vertices[0];
    BlockVertex *secondVertex = vertices[1];

    if (lastVertex->EdgeEffect() != "" && firstVertex->EdgeEffect() != "") {
      EdgeEffectSprite *pType = NULL;
      std::string v_edgeMaterialTextureName =
        pBlock->getEdgeMaterialTexture(firstVertex->EdgeEffect());
      if (v_edgeMaterialTextureName == "") {
        pType = (EdgeEffectSprite *)Theme::instance()->getSprite(
          SPRITE_TYPE_EDGEEFFECT, firstVertex->EdgeEffect());
      } else { // edge material defined then
        pType = (EdgeEffectSprite *)Theme::instance()->getSprite(
          SPRITE_TYPE_EDGEEFFECT, v_edgeMaterialTextureName);
      }

      if (pType == NULL) {
        LogWarning("Invalid edge effect %s", firstVertex->EdgeEffect().c_str());
        useOld = false;
      } else {
        Vector2f a1, b1, b2, a2, c1, c2;
        v_currentEdgeSprite = pType;
        v_currentEdgeEffect = firstVertex->EdgeEffect();
        calculateEdgePosition(pBlock,
                              lastVertex,
                              firstVertex,
                              secondVertex,
                              Center,
                              a1,
                              b1,
                              b2,
                              a2,
                              c1,
                              c2,
                              oldC2,
                              oldB2,
                              useOld,
                              false,
                              swapDone,
                              v_currentEdgeSprite->getDepth(),
                              v_currentEdgeMaterialDepth);
        oldC2 = c2;
        oldB2 = b2;
        useOld = true;
        v_currentEdgeBlendColor =
          pBlock->getEdgeMaterialColor(firstVertex->EdgeEffect());
        v_currentEdgeMaterialScale =
          pBlock->getEdgeMaterialScale(firstVertex->EdgeEffect());
        v_currentEdgeMaterialDepth =
          pBlock->getEdgeMaterialDepth(firstVertex->EdgeEffect());
      }
    }
  }

  /* determine whether the normal vector of the edge heads up or down

     if the same edge effect goes around a whole block, problems with e.g.
     half-moon-shaped
     blocks occur: the first is drawn under the last edge.
     to deal with this problem, we set the cutEdge bool which indicates to start
     a new geom,
     as soon as the direction of drawing is changed */

  bool v_cutEdge = false;
  bool v_edgeOrientation = true;
  bool v_oldEdgeOrientation = v_edgeOrientation;
  for (unsigned int j = 0; j < vertices.size(); j++) {
    BlockVertex *vertexA = vertices[j];
    std::string edgeEffect = vertexA->EdgeEffect();
    if (edgeEffect == "") {
      useOld = false;
      continue;
    }

    BlockVertex *vertexB = vertices[(j + 1) % vertices.size()];
    BlockVertex *vertexC = vertices[(j + 2) % vertices.size()];

    bool AisLast = (vertexB->EdgeEffect() == "");

    // check if edge orientation has changed
    Vector2f v_checkOrientation =
      Vector2f(vertexB->Position().x - vertexA->Position().x,
               vertexB->Position().y - vertexA->Position().y);
    v_checkOrientation.normal();
    v_checkOrientation.rotateXY(270.0 - pBlock->edgeAngle());
    v_oldEdgeOrientation = v_edgeOrientation;
    if (v_checkOrientation.y > 0) {
      v_edgeOrientation = true; // upper edge
    } else {
      v_edgeOrientation = false; // lower edge
    }

    if (j != 0 && v_edgeOrientation != v_oldEdgeOrientation) {
      v_cutEdge = true;
    }

    // check if edge texture is in material or pure
    std::string v_edgeMaterialTextureName =
      pBlock->getEdgeMaterialTexture(edgeEffect);

    EdgeEffectSprite *pSprite = NULL;
    if (v_edgeMaterialTextureName == "") { // the edge effect is then probably
      // pure oldschool. lets load it then
      pSprite = (EdgeEffectSprite *)Theme::instance()->getSprite(
        SPRITE_TYPE_EDGEEFFECT, edgeEffect);
    } else { // we seem to have a material defined.
      pSprite = (EdgeEffectSprite *)Theme::instance()->getSprite(
        SPRITE_TYPE_EDGEEFFECT, v_edgeMaterialTextureName);
    }
    if (pSprite == NULL) {
      useOld = false;
      continue;
    }

    if (edgeEffect !=
        v_currentEdgeEffect) { // if a new edge effect texture occurs, load it
      EdgeEffectSprite *pType = NULL;
      if (v_edgeMaterialTextureName == "") { // no material defined then
        pType = (EdgeEffectSprite *)Theme::instance()->getSprite(
          SPRITE_TYPE_EDGEEFFECT, edgeEffect);
        if (pType == NULL) {
          LogWarning("Invalid edge effect %s", edgeEffect.c_str());
          useOld = false;
          continue;
        }
        v_currentEdgeBlendColor = DEFAULT_EDGE_BLENDCOLOR;
        v_currentEdgeMaterialScale = DEFAULT_EDGE_SCALE;
        v_currentEdgeMaterialDepth = DEFAULT_EDGE_DEPTH;
      } else { // seems we ve got a material!
        pType = (EdgeEffectSprite *)Theme::instance()->getSprite(
          SPRITE_TYPE_EDGEEFFECT, v_edgeMaterialTextureName);
        if (pType == NULL) {
          LogWarning("Invalid edge material %s", edgeEffect.c_str());
          useOld = false;
          continue;
        }
        v_currentEdgeBlendColor = pBlock->getEdgeMaterialColor(edgeEffect);
        v_currentEdgeMaterialScale = pBlock->getEdgeMaterialScale(edgeEffect);
        v_currentEdgeMaterialDepth = pBlock->getEdgeMaterialDepth(edgeEffect);
      }

      v_currentEdgeSprite = pType;
      v_currentEdgeEffect = edgeEffect;
    }

    // if a geom for current edge effect exists, get its index number
    Geom *pGeom =
      edgeGeomExists(v_geoms, pBlock, edgeEffect, v_edgeOrientation);
    if (pGeom == NULL || v_cutEdge) {
      v_cutEdge = false;

      // create a new one
      pGeom = new Geom;

      m_edgeGeoms.push_back(pGeom);
      v_geoms.push_back(pGeom);
      pGeom->pTexture = NULL;
      pGeom->pSprite = pSprite;
      pGeom->material = edgeEffect;
      GeomPoly *pPoly = new GeomPoly;
      pGeom->Polys.push_back(pPoly);
    }

    GeomPoly *pPoly = pGeom->Polys[0];

    pGeom->edgeBlendColor = v_currentEdgeBlendColor;
    pGeom->isUpper = v_edgeOrientation;

    Vector2f a1, b1, b2, a2, c1, c2;
    calculateEdgePosition(pBlock,
                          vertexA,
                          vertexB,
                          vertexC,
                          Center,
                          a1,
                          b1,
                          b2,
                          a2,
                          c1,
                          c2,
                          oldC2,
                          oldB2,
                          useOld,
                          AisLast,
                          swapDone,
                          v_currentEdgeSprite->getDepth(),
                          v_currentEdgeMaterialDepth);

    Vector2f ua1, ub1, ub2, ua2;

    calculateEdgeTexture(pBlock,
                         a1,
                         b1,
                         b2,
                         a2,
                         ua1,
                         ub1,
                         ub2,
                         ua2,
                         v_currentEdgeSprite->getScale(),
                         v_currentEdgeMaterialScale);

    pPoly->nNumVertices += 4;
    pPoly->pVertices = (GeomCoord *)realloc(
      pPoly->pVertices, pPoly->nNumVertices * sizeof(GeomCoord));
    pPoly->pTexCoords = (GeomCoord *)realloc(
      pPoly->pTexCoords, pPoly->nNumVertices * sizeof(GeomCoord));
    o_geomBytes += (4 * sizeof(float));

    pPoly->pVertices[pPoly->nNumVertices - 4].x = a1.x;
    pPoly->pVertices[pPoly->nNumVertices - 4].y = a1.y;
    pPoly->pTexCoords[pPoly->nNumVertices - 4].x = ua1.x;
    pPoly->pTexCoords[pPoly->nNumVertices - 4].y = ua1.y;
    pPoly->pVertices[pPoly->nNumVertices - 3].x = b1.x;
    pPoly->pVertices[pPoly->nNumVertices - 3].y = b1.y;
    pPoly->pTexCoords[pPoly->nNumVertices - 3].x = ub1.x;
    pPoly->pTexCoords[pPoly->nNumVertices - 3].y = ub1.y;
    pPoly->pVertices[pPoly->nNumVertices - 2].x = b2.x;
    pPoly->pVertices[pPoly->nNumVertices - 2].y = b2.y;
    pPoly->pTexCoords[pPoly->nNumVertices - 2].x = ub2.x;
    pPoly->pTexCoords[pPoly->nNumVertices - 2].y = ub2.y;
    pPoly->pVertices[pPoly->nNumVertices - 1].x = a2.x;
    pPoly->pVertices[pPoly->nNumVertices - 1].y = a2.y;
    pPoly->pTexCoords[pPoly->nNumVertices - 1].x = ua2.x;
    pPoly->pTexCoords[pPoly->nNumVertices - 1].y = ub2.y;

    useOld = true;
    if (swapDone == true) {
      oldB2 = a2;
    } else {
      oldB2 = b2;
    }
    oldC2 = c2;
  }

  /* now lets sort the edgeGeoms for we get the right drawing order:
     upper edges must be drawn above lower edges, which we achieve by sorting
     the vector
     of the block edge geoms: put lower edgegeoms first and upper edgegeoms
     behind  */

  std::vector<Geom *> v_upperBlockGeomsIndex;
  std::vector<Geom *> v_lowerBlockGeomsIndex;

  for (unsigned int i = 0; i < v_geoms.size(); i++) {
    if (v_geoms[i]->isUpper) { // then edge is upper
      v_upperBlockGeomsIndex.push_back(v_geoms[i]);
    } else {
      v_lowerBlockGeomsIndex.push_back(v_geoms[i]);
    }
  }
  // now replace the block edgeGeoms index by our sorted one
  std::vector<Geom *> v_tempVec;
  v_tempVec.reserve(v_upperBlockGeomsIndex.size() +
                    v_lowerBlockGeomsIndex.size());
  v_tempVec.insert(v_tempVec.end(),
                   v_lowerBlockGeomsIndex.begin(),
                   v_lowerBlockGeomsIndex.end());
  v_tempVec.insert(v_tempVec.end(),
                   v_upperBlockGeomsIndex.begin(),
                   v_upperBlockGeomsIndex.end());
  v_geoms = v_tempVec;

#ifdef ENABLE_OPENGL
  /* Use VBO optimization? */
  if (GameApp::instance()->getDrawLib()->useVBOs()) {
    for (unsigned int k = 0; k < v_geoms.size(); k++) {
      for (unsigned int l = 0; l < v_geoms[k]->Polys.size(); l++) {
        GeomPoly *pPoly = v_geoms[k]->Polys[l];

        ((DrawLibOpenGL *)GameApp::instance()->getDrawLib())
          ->glGenBuffersARB(1, (GLuint *)&pPoly->nVertexBufferID);
        ((DrawLibOpenGL *)GameApp::instance()->getDrawLib())
          ->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nVertexBufferID);
        ((DrawLibOpenGL *)GameApp::instance()->getDrawLib())
          ->glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                            pPoly->nNumVertices * 2 * sizeof(float),
                            (void *)pPoly->pVertices,
                            GL_STATIC_DRAW_ARB);

        ((DrawLibOpenGL *)GameApp::instance()->getDrawLib())
          ->glGenBuffersARB(1, (GLuint *)&pPoly->nTexCoordBufferID);
        ((DrawLibOpenGL *)GameApp::instance()->getDrawLib())
          ->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nTexCoordBufferID);
        ((DrawLibOpenGL *)GameApp::instance()->getDrawLib())
          ->glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                            pPoly->nNumVertices * 2 * sizeof(float),
                            (void *)pPoly->pTexCoords,
                            GL_STATIC_DRAW_ARB);
      }
    }
  }
#endif

  return v_geoms;
}

Geom *LevelGeoms::edgeGeomExists(std::vector<Geom *> &i_geoms,
                                 Block *pBlock,
                                 std::string material,
                                 bool i_isUpper) {
  for (unsigned int i = 0; i < i_geoms.size(); i++) {
    if (i_geoms[i]->material == material && i_geoms[i]->isUpper == i_isUpper) {
      return i_geoms[i];
    }
  }

  return NULL;
}

LevelGeoms *GeomsManager::getLevelGeom(Scene *i_scene) {
  for (unsigned int i = 0; i < m_levelGeoms.size(); i++) {
    if (m_levelGeoms[i]->getLevelId() == i_scene->getLevelSrc()->Id()) {
      return m_levelGeoms[i];
    }
  }
  return NULL;
}

/*===========================================================================
  Called when we don't want to play the level anymore
  ===========================================================================*/
void LevelGeoms::deleteGeoms(std::vector<Geom *> &geom, bool useFree) {
  /* Clean up optimized scene */
  for (unsigned int i = 0; i < geom.size(); i++) {
    for (unsigned int j = 0; j < geom[i]->Polys.size(); j++) {
#ifdef ENABLE_OPENGL
      if (geom[i]->Polys[j]->nVertexBufferID) {
        ((DrawLibOpenGL *)GameApp::instance()->getDrawLib())
          ->glDeleteBuffersARB(1,
                               (GLuint *)&geom[i]->Polys[j]->nVertexBufferID);
        ((DrawLibOpenGL *)GameApp::instance()->getDrawLib())
          ->glDeleteBuffersARB(1,
                               (GLuint *)&geom[i]->Polys[j]->nTexCoordBufferID);
      }
#endif

      if (useFree == true) {
        free(geom[i]->Polys[j]->pTexCoords);
        free(geom[i]->Polys[j]->pVertices);
      } else {
        delete[] geom[i]->Polys[j]->pTexCoords;
        delete[] geom[i]->Polys[j]->pVertices;
      }
      delete geom[i]->Polys[j];
    }
    delete geom[i];
  }
  geom.clear();
}

GeomsManager::GeomsManager() {}

GeomsManager::~GeomsManager() {
  for (unsigned int i = 0; i < m_levelGeoms.size(); i++) {
    delete m_levelGeoms[i];
  }
}

LevelGeoms *GeomsManager::register_begin(Scene *i_scene) {
  LevelGeoms *v_levelGeoms = getLevelGeom(i_scene);

  // clean levelGeoms -- except if one has the same id as i_scene to not have to
  // reload geoms
  unsigned int i = 0;
  while (i < m_levelGeoms.size()) {
    if (m_levelGeoms[i]->getNumberOfRegisteredScenes() == 0 &&
        m_levelGeoms[i]->getLevelId() != i_scene->getLevelSrc()->Id()) {
      delete m_levelGeoms[i];
      m_levelGeoms.erase(m_levelGeoms.begin() + i);
    } else {
      i++;
    }
  }

  // create it if not exists
  if (v_levelGeoms == NULL) {
    v_levelGeoms = new LevelGeoms(i_scene->getLevelSrc()->Id());
    m_levelGeoms.push_back(v_levelGeoms);
  }

  return v_levelGeoms;
}

void GeomsManager::register_end(Scene *i_scene) {
  getLevelGeom(i_scene)->register_scene(i_scene);
}

void GeomsManager::unregister(Scene *i_scene) {
  getLevelGeom(i_scene)->unregister_scene(i_scene);
}

unsigned int GeomsManager::getNumberOfBlockGeoms() const {
  unsigned int n = 0;
  for (unsigned int i = 0; i < m_levelGeoms.size(); i++) {
    n += m_levelGeoms[i]->getNumberOfBlockGeoms();
  }
  return n;
}

unsigned int GeomsManager::getNumberOfEdgeGeoms() const {
  unsigned int n = 0;
  for (unsigned int i = 0; i < m_levelGeoms.size(); i++) {
    n += m_levelGeoms[i]->getNumberOfEdgeGeoms();
  }
  return n;
}
