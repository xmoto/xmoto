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

/* 
 *  In-game rendering
 */
#include "VXml.h"
#include "VFileIO.h"
#include "xmscene/Scene.h"
#include "Renderer.h"
#include "GameText.h"
#include "xmscene/Bike.h"
#include "xmscene/BikeGhost.h"
#include "xmscene/BikePlayer.h"
#include "helpers/Log.h"
#include "helpers/Text.h"
#include "drawlib/DrawLib.h"
#include "Game.h"
#include <algorithm>
#include "xmscene/Camera.h"
#include "xmscene/Block.h"
#include "xmscene/Entity.h"
#include "xmscene/Zone.h"
#include "xmscene/SkyApparence.h"
#include "Universe.h"
#include "xmscene/BikeParameters.h"
#include "xmscene/Entity.h"
#include "PhysSettings.h"
#include "xmscene/BasicSceneStructs.h"


#ifdef ENABLE_OPENGL
#include "drawlib/DrawLibOpenGL.h"
#endif

  /* to sort blocks on their texture */
  struct AscendingTextureSort {
    bool operator() (Block* b1, Block* b2) {
      return b1->Texture() < b2->Texture();
    }
  };

GameRenderer::GameRenderer() {
  m_previousEngineSpeed = -1.0;
  m_previousEngineLinVel = -1.0;
  m_sizeMultOfEntitiesToTake = 1.0;
  m_sizeMultOfEntitiesWhichMakeWin = 1.0;
  m_showMinimap = true;
  m_showEngineCounter = true;
  m_showTimePanel = true;
  m_allowGhostEffect = true;
  m_currentEdgeEffect = "";
  m_currentEdgeSprite = NULL;
}
GameRenderer::~GameRenderer() {
}

  /*===========================================================================
  Init at game start-up
  ===========================================================================*/
  void GameRenderer::init(DrawLib* i_drawLib) { 
    /* Overlays? */
    m_drawLib = i_drawLib;
    m_Overlay.init(GameApp::instance()->getDrawLib(),512,512);

    m_nParticlesRendered = 0;
  }

  /*===========================================================================
  Called to prepare renderer for new level
  ===========================================================================*/
void GameRenderer::prepareForNewLevel(Universe* i_universe) {
  // level of the first world
  Level* v_level;
  int n_sameSceneAs;

  if(i_universe == NULL) {
    return;
  }
  if(i_universe->getScenes().size() <= 0) {
    return;
  }

  // can't use the same overlay for the multi cameras,
  // because the fade is made using all the cameras,
  // there should be one overlay per camera.
  if(i_universe->getScenes().size() > 1) {
    m_allowGhostEffect = false;
  } else {
    m_allowGhostEffect = (i_universe->getScenes()[0]->getNumberCameras() == 1);
  }

  initCameras(i_universe);

  m_screenBBox.reset();
  m_layersBBox.reset();

  m_fNextGhostInfoUpdate = 0.0f;
  m_nGhostInfoTrans      = 255;
  m_sizeMultOfEntitiesToTake       = 1.0;
  m_sizeMultOfEntitiesWhichMakeWin = 1.0;

  /* Optimize scene */
  for(unsigned int u=0; u<i_universe->getScenes().size(); u++) {
    v_level = i_universe->getScenes()[u]->getLevelSrc();

    Logger::Log("Loading level %s", v_level->Name().c_str());

    n_sameSceneAs = -1;
    // set to the universe which has the same level to init geoms
    // only 1 time if the level is loaded several times
    for(unsigned int v=0; v<u; v++) {
      if(i_universe->getScenes()[u]->getLevelSrc()->Id() == i_universe->getScenes()[v]->getLevelSrc()->Id()) {
	n_sameSceneAs = v;
	break;
      }
    }

    std::vector<Block *> Blocks = v_level->Blocks();
    int nVertexBytes = 0;

    for(unsigned int i=0; i<Blocks.size(); i++) {
      /* do not load into the graphic card blocks which won't be
	 displayed. On ati card with free driver, levels like green
	 hill zone act 2 doesn't work if there's too much vertex loaded */
      if(XMSession::instance()->gameGraphics() != GFX_HIGH
	 && Blocks[i]->getLayer() != -1)
	continue;
      if(XMSession::instance()->gameGraphics() == GFX_LOW
	 && Blocks[i]->isBackground() == true)
	continue;

      nVertexBytes += loadBlock(Blocks[i], i_universe, u, n_sameSceneAs, i);
    }

    Logger::Log("GL: %d kB vertex buffers", nVertexBytes/1024);
  }
}

void GameRenderer::initCameras(Universe* i_universe)
{
  for(unsigned int j=0; j<i_universe->getScenes().size(); j++) {
    unsigned int numberCamera = i_universe->getScenes()[j]->getNumberCameras();
    if(numberCamera > 1){
      numberCamera++;
    }
    for(unsigned int i=0; i<numberCamera; i++){
      i_universe->getScenes()[j]->setCurrentCamera(i);
      i_universe->getScenes()[j]->getCamera()->prepareForNewLevel();
    }
  }
}

int GameRenderer::loadBlock(Block* pBlock,
			    Universe* i_universe,
			    unsigned int currentScene,
			    int sameSceneAs,
			    int blockIndex)
{
  MotoGame* pScene  = i_universe->getScenes()[currentScene];
  int nVertexBytes  = 0;
  bool dynamicBlock = false;
  std::vector<Geom *>* pGeoms;
  if(pBlock->isDynamic() == true){
    dynamicBlock = true;
    pGeoms = &m_DynamicGeoms;
  }
  else{
    pGeoms = &m_StaticGeoms;
  }

  Vector2f Center;
  if(dynamicBlock == true){
    Center.x = 0.0;
    Center.y = 0.0;
  }
  else{
    Center = pBlock->DynamicPosition();
  }

  Texture* pTexture = loadTexture(pBlock->Texture());
  if(pTexture == NULL){
    pScene->gameMessage(GAMETEXT_MISSINGTEXTURES, true);
  }

  // add the geom
  if(sameSceneAs == -1) {
    nVertexBytes += loadBlockGeom(pBlock, pGeoms, pTexture, Center, pScene);
    nVertexBytes += loadBlockEdge(pBlock, Center, pScene);
    return nVertexBytes;

  } else {
    // the geoms already exist in level sameSceneAs
    // assum that blocks loading of levels have the same number (i)
    Block* pBlockFromOtherLevel = i_universe->getScenes()[sameSceneAs]->getLevelSrc()->Blocks()[blockIndex];
    pBlock->setGeom(pBlockFromOtherLevel->getGeom());

    // the edge geoms already exist too
    for(unsigned int i=0;
	i<pBlockFromOtherLevel->getEdgeGeoms().size();
	i++){
      pBlock->addEdgeGeom(pBlockFromOtherLevel->getEdgeGeoms()[i]);
    }

    return 0;
  }
}

int GameRenderer::loadBlockGeom(Block* pBlock,
				std::vector<Geom *>* pGeoms,
				Texture* pTexture,
				Vector2f Center,
				MotoGame* pScene)
{
  std::vector<ConvexBlock *> ConvexBlocks = pBlock->ConvexBlocks();
  int nVertexBytes  = 0;
  Geom* pSuitableGeom = new Geom;
  pSuitableGeom->pTexture = pTexture;
  int geomIndex = pGeoms->size();
  pGeoms->push_back(pSuitableGeom);
  pBlock->setGeom(geomIndex);

  for(unsigned int j=0; j<ConvexBlocks.size(); j++) {
    Vector2f v_center = Vector2f(0.0, 0.0);

    GeomPoly *pPoly = new GeomPoly;
    pSuitableGeom->Polys.push_back(pPoly);

    pPoly->nNumVertices = ConvexBlocks[j]->Vertices().size();
    pPoly->pVertices    = new GeomCoord[ pPoly->nNumVertices ];
    pPoly->pTexCoords   = new GeomCoord[ pPoly->nNumVertices ];

    for(unsigned int k=0; k<pPoly->nNumVertices; k++) {
      pPoly->pVertices[k].x  = Center.x + ConvexBlocks[j]->Vertices()[k]->Position().x;
      pPoly->pVertices[k].y  = Center.y + ConvexBlocks[j]->Vertices()[k]->Position().y;
      pPoly->pTexCoords[k].x = ConvexBlocks[j]->Vertices()[k]->TexturePosition().x;
      pPoly->pTexCoords[k].y = ConvexBlocks[j]->Vertices()[k]->TexturePosition().y;

      v_center += Vector2f(pPoly->pVertices[k].x, pPoly->pVertices[k].y);
    }
    v_center /= pPoly->nNumVertices;

    /* fix the gap problem with polygons */
    float a = 0.003; /* seems to be a good value, put negativ value to make it worst */
    for(unsigned int k=0; k<pPoly->nNumVertices; k++) {
      Vector2f V = Vector2f(pPoly->pVertices[k].x - v_center.x, pPoly->pVertices[k].y - v_center.y);
      if(V.length() != 0.0) {
	V.normalize();
	V *= a;
	pPoly->pVertices[k].x += V.x;
	pPoly->pVertices[k].y += V.y;
      }
    }

    nVertexBytes += pPoly->nNumVertices * (4 * sizeof(float));
#ifdef ENABLE_OPENGL        
    /* Use VBO optimization? */
    if(GameApp::instance()->getDrawLib()->useVBOs()) {
      /* Copy static coordinates unto video memory */
      ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glGenBuffersARB(1, (GLuint *) &pPoly->nVertexBufferID);
      ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB,pPoly->nVertexBufferID);
      ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glBufferDataARB(GL_ARRAY_BUFFER_ARB,pPoly->nNumVertices*2*sizeof(float),(void *)pPoly->pVertices,GL_STATIC_DRAW_ARB);

      ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glGenBuffersARB(1, (GLuint *) &pPoly->nTexCoordBufferID);
      ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB,pPoly->nTexCoordBufferID);
      ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glBufferDataARB(GL_ARRAY_BUFFER_ARB,pPoly->nNumVertices*2*sizeof(float),(void *)pPoly->pTexCoords,GL_STATIC_DRAW_ARB);
    }
#endif
  }

  return nVertexBytes;
}

int GameRenderer::loadBlockEdge(Block* pBlock, Vector2f Center, MotoGame* pScene)
{
  int nVertexBytes  = 0;
  if(XMSession::instance()->gameGraphics() != GFX_LOW){
    m_currentEdgeEffect = "";
    m_currentEdgeSprite = NULL;

    Vector2f oldC2, oldB2;
    bool useOld   = false;
    bool swapDone = false;

    // create edge texture
    std::vector<BlockVertex *>& vertices = pBlock->Vertices();

    // if the last and the first vertex have edge effect, we have to
    // calculate oldC2 and oldB2
    if(vertices.size() > 1){
      BlockVertex* lastVertex   = vertices[vertices.size()-1];
      BlockVertex* firstVertex  = vertices[0];
      BlockVertex* secondVertex = vertices[1];

      if(lastVertex->EdgeEffect() != "" && firstVertex->EdgeEffect() != ""){
	EdgeEffectSprite* pType = NULL;
	pType = (EdgeEffectSprite*)Theme::instance()->getSprite(SPRITE_TYPE_EDGEEFFECT, firstVertex->EdgeEffect());
	if(pType == NULL) {
	  Logger::Log("** Invalid edge effect %s", firstVertex->EdgeEffect().c_str());
	  useOld = false;
	}
	else{
	  m_currentEdgeSprite = pType;
	  m_currentEdgeEffect = firstVertex->EdgeEffect();

	  Vector2f a1, b1, b2, a2, c1, c2;
	  calculateEdgePosition(pBlock,
				lastVertex, firstVertex, secondVertex,
				Center,
				a1, b1, b2, a2, c1, c2,
				oldC2, oldB2, useOld, false, swapDone);
	  oldC2 = c2;
	  oldB2 = b2;
	  useOld = true;
	  m_currentEdgeSprite = NULL;
	  m_currentEdgeEffect = "";
	}
      }
    }

    for(unsigned int j=0; j<vertices.size(); j++){
      BlockVertex* vertexA = vertices[j];
      std::string edgeEffect = vertexA->EdgeEffect();
      if(edgeEffect == "") {
	useOld = false;
	continue;
      }

      BlockVertex* vertexB  = vertices[(j+1) % vertices.size()];
      BlockVertex* vertexC  = vertices[(j+2) % vertices.size()];

      bool AisLast = (vertexB->EdgeEffect() == "");

      Texture*     pTexture = loadTextureEdge(edgeEffect);
      if(pTexture == NULL){
	pScene->gameMessage(GAMETEXT_MISSINGTEXTURES, true);
	useOld = false;
	continue;
      }

      EdgeEffectSprite* pType = NULL;
      if(edgeEffect != m_currentEdgeEffect) {
	pType = (EdgeEffectSprite*)Theme::instance()->getSprite(SPRITE_TYPE_EDGEEFFECT, edgeEffect);
	if(pType == NULL) {
	  Logger::Log("** Invalid edge effect %s", edgeEffect.c_str());
	  useOld = false;
	  continue;
	}

	m_currentEdgeSprite = pType;
	m_currentEdgeEffect = edgeEffect;
      }

      int geomIndex = edgeGeomExists(pBlock, pTexture->Name);
      if(geomIndex < 0){
	// create a new one
	Geom* pGeom = new Geom;
	geomIndex = m_edgeGeoms.size();
	m_edgeGeoms.push_back(pGeom);
	pBlock->addEdgeGeom(geomIndex);
	pGeom->pTexture = pTexture;
	GeomPoly *pPoly = new GeomPoly;
	pGeom->Polys.push_back(pPoly);
      }
      Geom*     pGeom = m_edgeGeoms[geomIndex];
      GeomPoly* pPoly = pGeom->Polys[0];

      Vector2f a1, b1, b2, a2, c1, c2;
      calculateEdgePosition(pBlock,
			    vertexA, vertexB, vertexC,
			    Center,
			    a1, b1, b2, a2, c1, c2,
			    oldC2, oldB2, useOld, AisLast, swapDone);

      Vector2f ua1, ub1, ub2, ua2;
      calculateEdgeTexture(pBlock,
			   a1, b1, b2, a2,
			   ua1, ub1, ub2, ua2);

      pPoly->nNumVertices += 4;
      pPoly->pVertices    = (GeomCoord*)realloc(pPoly->pVertices,
						pPoly->nNumVertices * sizeof(GeomCoord));
      pPoly->pTexCoords   = (GeomCoord*)realloc(pPoly->pTexCoords ,
						pPoly->nNumVertices * sizeof(GeomCoord));
      nVertexBytes += (4 * sizeof(float));

      pPoly->pVertices[pPoly->nNumVertices-4].x  = a1.x;
      pPoly->pVertices[pPoly->nNumVertices-4].y  = a1.y;
      pPoly->pTexCoords[pPoly->nNumVertices-4].x = ua1.x;
      pPoly->pTexCoords[pPoly->nNumVertices-4].y = ua1.y;
      pPoly->pVertices[pPoly->nNumVertices-3].x  = b1.x;
      pPoly->pVertices[pPoly->nNumVertices-3].y  = b1.y;
      pPoly->pTexCoords[pPoly->nNumVertices-3].x = ub1.x;
      pPoly->pTexCoords[pPoly->nNumVertices-3].y = ub1.y;
      pPoly->pVertices[pPoly->nNumVertices-2].x  = b2.x;
      pPoly->pVertices[pPoly->nNumVertices-2].y  = b2.y;
      pPoly->pTexCoords[pPoly->nNumVertices-2].x = ub2.x;
      pPoly->pTexCoords[pPoly->nNumVertices-2].y = ub2.y;
      pPoly->pVertices[pPoly->nNumVertices-1].x  = a2.x;
      pPoly->pVertices[pPoly->nNumVertices-1].y  = a2.y;
      pPoly->pTexCoords[pPoly->nNumVertices-1].x = ua2.x;
      pPoly->pTexCoords[pPoly->nNumVertices-1].y = ub2.y;

      useOld = true;
      if(swapDone == true){
	oldB2 = a2;
      }else{
	oldB2 = b2;
      }
      oldC2   = c2;
    }

#ifdef ENABLE_OPENGL        
    /* Use VBO optimization? */
    if(GameApp::instance()->getDrawLib()->useVBOs()) {
      for(unsigned int k=0; k<pBlock->getEdgeGeoms().size(); k++){
	int geom = pBlock->getEdgeGeoms()[k];
	for(unsigned int l=0;l<m_edgeGeoms[geom]->Polys.size();l++) {          
	  GeomPoly *pPoly = m_edgeGeoms[geom]->Polys[l];

	  ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glGenBuffersARB(1, (GLuint *) &pPoly->nVertexBufferID);
	  ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB,pPoly->nVertexBufferID);
	  ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glBufferDataARB(GL_ARRAY_BUFFER_ARB,pPoly->nNumVertices*2*sizeof(float),(void *)pPoly->pVertices,GL_STATIC_DRAW_ARB);

	  ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glGenBuffersARB(1, (GLuint *) &pPoly->nTexCoordBufferID);
	  ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB,pPoly->nTexCoordBufferID);
	  ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glBufferDataARB(GL_ARRAY_BUFFER_ARB,pPoly->nNumVertices*2*sizeof(float),(void *)pPoly->pTexCoords,GL_STATIC_DRAW_ARB);
	}
      }
    }
#endif
  }

  return nVertexBytes;
}

void GameRenderer::calculateEdgePosition(Block* pBlock,
					 BlockVertex* vertexA1,
					 BlockVertex* vertexB1,
					 BlockVertex* vertexC1,
					 Vector2f     center,
					 Vector2f& A1, Vector2f& B1,
					 Vector2f& B2, Vector2f& A2,
					 Vector2f& C1, Vector2f& C2,
					 Vector2f oldC2, Vector2f oldB2, bool useOld,
					 bool AisLast, bool& swapDone)
{
  Vector2f vAPos = vertexA1->Position();
  Vector2f vBPos = vertexB1->Position();
  Vector2f vCPos = vertexC1->Position();

  /* link A to B */
  float fDepth  = m_currentEdgeSprite->getDepth();
  // add a small border because polygons are a bit larger to avoid gap polygon pb.
  float v_border; 
  if(fDepth > 0)
    v_border = 0.01;
  else
    v_border = -0.01;

  switch(pBlock->getEdgeDrawMethod()){
  case Block::angle:
    pBlock->calculateEdgePosition_angle(vAPos, vBPos, A1, B1, B2, A2, v_border, fDepth, center, pBlock->edgeAngle());
    break;
  case Block::inside:
    pBlock->calculateEdgePosition_inout(vAPos, vBPos, vCPos, A1, B1, B2, A2, C1, C2, v_border, fDepth, center, oldC2, oldB2, useOld, AisLast, swapDone, true);
    break;
  case Block::outside:
    pBlock->calculateEdgePosition_inout(vAPos, vBPos, vCPos, A1, B1, B2, A2, C1, C2, v_border, fDepth, center, oldC2, oldB2, useOld, AisLast, swapDone, false);
    break;
  }
}

void GameRenderer::calculateEdgeTexture(Block* pBlock,
					Vector2f A1, Vector2f B1,
					Vector2f B2, Vector2f A2,
					Vector2f& ua1, Vector2f& ub1,
					Vector2f& ub2, Vector2f& ua2)
{
  float fXScale = m_currentEdgeSprite->getScale();

  switch(pBlock->getEdgeDrawMethod()){
  case Block::inside:
  case Block::outside:{
    Vector2f N1(-B1.y+A1.y, B1.x-A1.x);

    if(N1.x == 0.0 && N1.y == 0.0){
      Logger::Log("normal is null for block %s vertex (%f,%f)", pBlock->Id().c_str(), A1.x, A1.y);
    }

    N1.normalize();

    ua1.x = A1.x*fXScale*N1.y - A1.y*fXScale*N1.x;
    ub1.x = B1.x*fXScale*N1.y - B1.y*fXScale*N1.x;

    Vector2f N2(-B2.y+A2.y, B2.x-A2.x);

    if(N2.x == 0.0 && N2.y == 0.0){
      Logger::Log("normal is null for block %s vertex (%f,%f)", pBlock->Id().c_str(), A2.x, A2.y);
    }

    N2.normalize();

    ua2.x = A2.x*fXScale*N2.y - A2.y*fXScale*N2.x;
    ub2.x = B2.x*fXScale*N2.y - B2.y*fXScale*N2.x;

    bool drawInside = (pBlock->getEdgeDrawMethod() == Block::inside);
    if(drawInside == true){
      ua1.y = ub1.y = 0.01;
      ua2.y = ub2.y = 0.99;
    } else {
      ua1.y = ub1.y = 0.99;
      ua2.y = ub2.y = 0.01;
    }
  }
    break;
  case Block::angle:{
    // all the unwanted children of cos and sin
    float radAngle = deg2rad(pBlock->edgeAngle());
    ua1.x = ua2.x = A1.x*fXScale*sinf(radAngle) - A1.y*fXScale*cosf(radAngle);
    ua1.y = ub1.y = 0.01;
    ub1.x = ub2.x = B1.x*fXScale*sinf(radAngle) - B1.y*fXScale*cosf(radAngle);
    ua2.y = ub2.y = 0.99;
  }
    break;
  }
}

Texture* GameRenderer::loadTexture(std::string textureName)
{
  Sprite*  pSprite  = Theme::instance()->getSprite(SPRITE_TYPE_TEXTURE, textureName);
  Texture* pTexture = NULL;

  if(pSprite != NULL) {
    try {
      pTexture = pSprite->getTexture();
    } catch(Exception &e) {
      Logger::Log("** Warning ** : Texture '%s' not found!", textureName.c_str());
    }
  } else {
    Logger::Log("** Warning ** : Texture '%s' not found!", textureName.c_str());
  }

  return pTexture;
}

Texture* GameRenderer::loadTextureEdge(std::string textureName)
{
  EdgeEffectSprite* pSprite  = (EdgeEffectSprite*)Theme::instance()->getSprite(SPRITE_TYPE_EDGEEFFECT, textureName);
  Texture* pTexture = NULL;

  if(pSprite != NULL) {
    try {
      pTexture = pSprite->getTexture();
    } catch(Exception &e) {
      Logger::Log("** Warning ** : Texture '%s' not found!", textureName.c_str());
    }
  } else {
    Logger::Log("** Warning ** : Texture '%s' not found!", textureName.c_str());
  }

  return pTexture;
}

int GameRenderer::edgeGeomExists(Block* pBlock, std::string texture)
{
  std::vector<int>& edgeGeoms = pBlock->getEdgeGeoms();

  for(unsigned int i=0; i<edgeGeoms.size(); i++){
    if(m_edgeGeoms[edgeGeoms[i]]->pTexture->Name == texture)
      return edgeGeoms[i];
  }

  return -1;
}

  /*===========================================================================
  Called when we don't want to play the level anymore
  ===========================================================================*/
  void GameRenderer::_deleteGeoms(std::vector<Geom *>& geom, bool useFree)
  {
    /* Clean up optimized scene */
    for(unsigned int i=0;i<geom.size();i++) { 
      for(unsigned int j=0;j<geom[i]->Polys.size();j++) { 
#ifdef ENABLE_OPENGL
        if(geom[i]->Polys[j]->nVertexBufferID) {
          ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glDeleteBuffersARB(1, (GLuint *) &geom[i]->Polys[j]->nVertexBufferID);
          ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glDeleteBuffersARB(1, (GLuint *) &geom[i]->Polys[j]->nTexCoordBufferID);
        }
#endif

	if(useFree == true){
	  free(geom[i]->Polys[j]->pTexCoords);
	  free(geom[i]->Polys[j]->pVertices);
	}else {
	  delete [] geom[i]->Polys[j]->pTexCoords;
	  delete [] geom[i]->Polys[j]->pVertices;
	}
	delete geom[i]->Polys[j];
      }
      delete geom[i];
    }
    geom.clear();
  }
  
  void GameRenderer::unprepareForNewLevel(void) {
    _deleteGeoms(m_StaticGeoms);
    _deleteGeoms(m_DynamicGeoms);
    _deleteGeoms(m_edgeGeoms, true);
  }
  
  void GameRenderer::renderEngineCounter(int x, int y, int nWidth,int nHeight, float pSpeed, float pLinVel) {

// coords of then center ; make it dynamic would be nice
#define ENGINECOUNTER_CENTERX      192.0
#define ENGINECOUNTER_CENTERY      206.0
#define ENGINECOUNTER_RADIUS       150.0
#define ENGINECOUNTER_PICTURE_SIZE 256.0
#define ENGINECOUNTER_STARTANGLE   (-3.14159/17)
#define ENGINECOUNTER_MAX_DIFF     1
#define ENGINECOUNTER_MAX_SPEED	   120
#define ENGINECOUNTER_NEEDLE_WIDTH_FACTOR (1.0/24)
#define ENGINECOUNTER_NEEDLE_BOTTOM_FACTOR (1.0/30)

    float pSpeed_eff;
    float pLinVel_eff;

    if (pSpeed > ENGINECOUNTER_MAX_SPEED)
      pSpeed = ENGINECOUNTER_MAX_SPEED;
	
    if (pLinVel > ENGINECOUNTER_MAX_SPEED)
      pLinVel = ENGINECOUNTER_MAX_SPEED;	
	
    /* don't make line too nasty */
    if(m_previousEngineSpeed < 0.0) {
      pSpeed_eff = pSpeed;
      m_previousEngineSpeed = pSpeed_eff;
    } else {
      if( labs((int)(pSpeed - m_previousEngineSpeed)) > ENGINECOUNTER_MAX_DIFF) {
	if(pSpeed - m_previousEngineSpeed > 0) {
	  pSpeed_eff = m_previousEngineSpeed + ENGINECOUNTER_MAX_DIFF;
	} else {
	  pSpeed_eff = m_previousEngineSpeed - ENGINECOUNTER_MAX_DIFF;
	}
	m_previousEngineSpeed = pSpeed_eff;
      } else {
	/* speed change is to small - ignore it to smooth counter moves*/
	pSpeed_eff = m_previousEngineSpeed;
      }
    }
	
    if (m_previousEngineLinVel < 0.0) {
      pLinVel_eff = pLinVel;
      m_previousEngineLinVel = pLinVel_eff;
    } else {
      if( labs((int)(pLinVel - m_previousEngineLinVel)) > ENGINECOUNTER_MAX_DIFF) {
	if(pLinVel - m_previousEngineLinVel > 0) {
	  pLinVel_eff = m_previousEngineLinVel + ENGINECOUNTER_MAX_DIFF;
	} else {
	  pLinVel_eff = m_previousEngineLinVel - ENGINECOUNTER_MAX_DIFF;
	}
	m_previousEngineLinVel = pLinVel_eff;
      } else {
	/* speed change is to small - ignore it to smooth counter moves*/
	pLinVel_eff = m_previousEngineLinVel;
      }
    }

    Sprite *pSprite;
    Texture *pTexture;
    Vector2f p0, p1, p2, p3;
    Vector2f pcenter, pdest, pcenterl, pcenterr, pbottom;

    p0 = Vector2f(x,        GameApp::instance()->getDrawLib()->getDispHeight()-y-nHeight);
    p1 = Vector2f(x+nWidth, GameApp::instance()->getDrawLib()->getDispHeight()-y-nHeight);
    p2 = Vector2f(x+nWidth, GameApp::instance()->getDrawLib()->getDispHeight()-y);
    p3 = Vector2f(x,        GameApp::instance()->getDrawLib()->getDispHeight()-y);

    pSprite = (MiscSprite*) Theme::instance()->getSprite(SPRITE_TYPE_MISC, "EngineCounter");
    if(pSprite != NULL) {
      pTexture = pSprite->getTexture();
      if(pTexture != NULL) {
	_RenderAlphaBlendedSection(pTexture, p0, p1, p2, p3);

	GameApp::instance()->getDrawLib()->setColorRGB(255,50,50);
	renderEngineCounterNeedle(nWidth, nHeight, p3, pSpeed_eff);
	GameApp::instance()->getDrawLib()->setColorRGB(50,50,255);
	if (pLinVel_eff > -1) {
	  renderEngineCounterNeedle(nWidth, nHeight, p3, pLinVel_eff);
	}
      }
    }
  }

  void GameRenderer::renderEngineCounterNeedle(int nWidth, int nHeight, Vector2f center, float value) {
    float coefw = 1.0 / ENGINECOUNTER_PICTURE_SIZE * nWidth;
    float coefh = 1.0 / ENGINECOUNTER_PICTURE_SIZE * nHeight;  
    Vector2f pcenter = center + Vector2f(ENGINECOUNTER_CENTERX   * coefw,
					 - ENGINECOUNTER_CENTERY * coefh);

    Vector2f pcenterl = pcenter + Vector2f(-cosf(value / 360.0 * (2.0 * 3.14159) + (3.14159/2) + ENGINECOUNTER_STARTANGLE)
					   * (ENGINECOUNTER_RADIUS) * coefw * ENGINECOUNTER_NEEDLE_WIDTH_FACTOR,
					   sinf(value / 360.0  * (2.0 * 3.14159) + (3.14159/2))
					   * (ENGINECOUNTER_RADIUS) * coefh * ENGINECOUNTER_NEEDLE_WIDTH_FACTOR);

    Vector2f pcenterr = pcenter - Vector2f(-cosf(value / 360.0 * (2.0 * 3.14159) + (3.14159/2) + ENGINECOUNTER_STARTANGLE)
					   * (ENGINECOUNTER_RADIUS) * coefw * ENGINECOUNTER_NEEDLE_WIDTH_FACTOR,
					   sinf(value / 360.0  * (2.0 * 3.14159) + (3.14159/2) + ENGINECOUNTER_STARTANGLE)
					   * (ENGINECOUNTER_RADIUS) * coefh * ENGINECOUNTER_NEEDLE_WIDTH_FACTOR);

    Vector2f pdest    = pcenter + Vector2f(-cosf(value / 360.0 * (2.0 * 3.14159) + ENGINECOUNTER_STARTANGLE )
					   * (ENGINECOUNTER_RADIUS) * coefw, sinf(value / 360.0  * (2.0 * 3.14159) + ENGINECOUNTER_STARTANGLE) * (ENGINECOUNTER_RADIUS) * coefh);

    Vector2f pbottom   = pcenter - Vector2f(-cosf(value / 360.0 * (2.0 * 3.14159) + ENGINECOUNTER_STARTANGLE )
					    * (ENGINECOUNTER_RADIUS) * coefw * ENGINECOUNTER_NEEDLE_BOTTOM_FACTOR,
					    sinf(value / 360.0  * (2.0 * 3.14159)
						 + ENGINECOUNTER_STARTANGLE) * (ENGINECOUNTER_RADIUS) * coefh * ENGINECOUNTER_NEEDLE_BOTTOM_FACTOR);
	  
    GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
    GameApp::instance()->getDrawLib()->glVertex(pdest);
    GameApp::instance()->getDrawLib()->glVertex(pcenterl);
    GameApp::instance()->getDrawLib()->glVertex(pbottom);
    GameApp::instance()->getDrawLib()->glVertex(pcenterr);
    GameApp::instance()->getDrawLib()->endDraw();
  }
  
  /*===========================================================================
  Minimap rendering
  ===========================================================================*/
  #define MINIMAPZOOM 5.0f
  #define MINIMAPALPHA 128
  #define MINIVERTEX(Px,Py) \
    GameApp::instance()->getDrawLib()->glVertexSP(x + nWidth/2 + (float)(Px - i_scene->getCamera()->getCameraPositionX())*MINIMAPZOOM, \
                          y + nHeight/2 - (float)(Py - i_scene->getCamera()->getCameraPositionY())*MINIMAPZOOM);    

void GameRenderer::renderMiniMap(MotoGame* i_scene, int x,int y,int nWidth,int nHeight) {
    Biker* pBiker = i_scene->getCamera()->getPlayerToFollow();
    // do not render it if it's the autozoom camera (in multi), or the player is dead (in multi), or no player is followed
    if(i_scene->isAutoZoomCamera() == true
       || ((pBiker == NULL || (pBiker != NULL && pBiker->isDead() == true))
	   && i_scene->getNumberCameras() > 1)){
      return;
    }

    GameApp::instance()->getDrawLib()->drawBox(Vector2f(x,y),Vector2f(x+nWidth,y+nHeight),1,
				       MAKE_COLOR(0,0,0,MINIMAPALPHA),
				       MAKE_COLOR(255,255,255,MINIMAPALPHA));
    // the scissor zone is in the screen coordinates
    Vector2i bottomLeft = i_scene->getCamera()->getDispBottomLeft();

    unsigned int y_translate = bottomLeft.y/2;
    if((unsigned int)bottomLeft.y != GameApp::instance()->getDrawLib()->getDispHeight()
       || i_scene->getNumberCameras() == 1){
      y_translate = 0;
    }
    GameApp::instance()->getDrawLib()->setClipRect(bottomLeft.x + x+1,
			 y+1 - y_translate,
			 nWidth-2,nHeight-2);

#ifdef ENABLE_OPENGL
    glEnable(GL_SCISSOR_TEST);
    glLoadIdentity();
#endif

    /* get minimap AABB in level space
      input:  position on the screen (in the minimap area)
      output: position in the level
    */
#define MAP_TO_LEVEL_X(mapX) ((mapX) - x - nWidth/2)/MINIMAPZOOM  + i_scene->getCamera()->getCameraPositionX()
#define MAP_TO_LEVEL_Y(mapY) ((mapY) - y - nHeight/2)/MINIMAPZOOM + i_scene->getCamera()->getCameraPositionY()
    AABB mapBBox;
    mapBBox.addPointToAABB2f(MAP_TO_LEVEL_X(x), MAP_TO_LEVEL_Y(y));
    mapBBox.addPointToAABB2f(MAP_TO_LEVEL_X(x+nWidth), MAP_TO_LEVEL_Y(y+nHeight));

    /* TOFIX::Draw the static blocks only once in a texture, and reuse it after */
    /* Render blocks */
    std::vector<Block*> Blocks;

    for(int layer=-1; layer<=0; layer++){
      Blocks = i_scene->getCollisionHandler()->getStaticBlocksNearPosition(mapBBox, layer);
      for(unsigned int i=0; i<Blocks.size(); i++) {

	/* Don't draw background blocks neither dynamic ones */
	if(Blocks[i]->isBackground() == false && Blocks[i]->getLayer() == -1) {
	  std::vector<ConvexBlock *> ConvexBlocks = Blocks[i]->ConvexBlocks();
	  for(unsigned int j=0; j<ConvexBlocks.size(); j++) {
	    Vector2f Center = ConvexBlocks[j]->SourceBlock()->DynamicPosition(); 	 

	    GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_POLYGON); 	 
	    GameApp::instance()->getDrawLib()->setColorRGB(128,128,128);
	    /* TOFIX::what's THAT ??!? -->> put all the vertices in a vector and draw them in one opengl call ! */
	    for(unsigned int k=0; k<ConvexBlocks[j]->Vertices().size(); k++) { 	 
	      Vector2f P = Center + ConvexBlocks[j]->Vertices()[k]->Position(); 	 
	      MINIVERTEX(P.x,P.y);
	    } 	 
	    GameApp::instance()->getDrawLib()->endDraw();
	  }
	}
      }
    }

    /* Render dynamic blocks */
    /* display only visible dyn blocks */
    Blocks = i_scene->getCollisionHandler()->getDynBlocksNearPosition(mapBBox);

    /* TOFIX::do not calculate this again. (already done in Block.cpp) */
    for(unsigned int i=0; i<Blocks.size(); i++) {
      if(Blocks[i]->isBackground() == false && Blocks[i]->getLayer() == -1) {
	std::vector<ConvexBlock *> ConvexBlocks = Blocks[i]->ConvexBlocks();
	for(unsigned int j=0; j<ConvexBlocks.size(); j++) {
	  GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
	  GameApp::instance()->getDrawLib()->setColorRGB(128,128,128);
	  /* Build rotation matrix for block */
	  float fR[4];
	  fR[0] = cosf(Blocks[i]->DynamicRotation()); fR[1] = -sinf(Blocks[i]->DynamicRotation());
	  fR[2] = sinf(Blocks[i]->DynamicRotation()); fR[3] = cosf(Blocks[i]->DynamicRotation());
	  for(unsigned int k=0; k<ConvexBlocks[j]->Vertices().size(); k++) {
	    ConvexBlockVertex *pVertex = ConvexBlocks[j]->Vertices()[k];
	    
	    /* Transform vertex */
	    Vector2f Tv = Vector2f((pVertex->Position().x-Blocks[i]->DynamicRotationCenter().x) * fR[0] + (pVertex->Position().y-Blocks[i]->DynamicRotationCenter().y) * fR[1],
				   (pVertex->Position().x-Blocks[i]->DynamicRotationCenter().x) * fR[2] + (pVertex->Position().y-Blocks[i]->DynamicRotationCenter().y) * fR[3]);
	    Tv += Blocks[i]->DynamicPosition() + Blocks[i]->DynamicRotationCenter();
	    
	    MINIVERTEX(Tv.x,Tv.y);
	  }
	  
	  GameApp::instance()->getDrawLib()->endDraw();
	}
      }
    }


    /*
      input: position in the level
      output: position on the screen (draw in the minimap area)
    */
#define LEVEL_TO_SCREEN_X(elemPosX) (x + nWidth/2  + (float)((elemPosX) - i_scene->getCamera()->getCameraPositionX()) * MINIMAPZOOM)
#define LEVEL_TO_SCREEN_Y(elemPosY) (y + nHeight/2 - (float)((elemPosY) - i_scene->getCamera()->getCameraPositionY()) * MINIMAPZOOM)

    for(unsigned int i=0; i<i_scene->Players().size(); i++) {
      Vector2f bikePos(LEVEL_TO_SCREEN_X(i_scene->Players()[i]->getState()->CenterP.x),
		       LEVEL_TO_SCREEN_Y(i_scene->Players()[i]->getState()->CenterP.y));
      GameApp::instance()->getDrawLib()->drawCircle(bikePos, 3, 0, MAKE_COLOR(255,255,255,255), 0);
    }
    
    /* Render ghost position too? */
    for(unsigned int i=0; i<i_scene->Ghosts().size(); i++) {
      Ghost* v_ghost = i_scene->Ghosts()[i];

      Vector2f ghostPos(LEVEL_TO_SCREEN_X(v_ghost->getState()->CenterP.x),
			LEVEL_TO_SCREEN_Y(v_ghost->getState()->CenterP.y));
      GameApp::instance()->getDrawLib()->drawCircle(ghostPos, 3, 0, MAKE_COLOR(96,96,150,255), 0);
    }

    /* FIX::display only visible entities */
    std::vector<Entity*> Entities = i_scene->getCollisionHandler()->getEntitiesNearPosition(mapBBox);

    for(unsigned int i=0;i<Entities.size();i++) {
      Vector2f entityPos(LEVEL_TO_SCREEN_X(Entities[i]->DynamicPosition().x),
			 LEVEL_TO_SCREEN_Y(Entities[i]->DynamicPosition().y));
      if(Entities[i]->DoesMakeWin()) {
        GameApp::instance()->getDrawLib()->drawCircle(entityPos, 3, 0, MAKE_COLOR(255,0,255,255), 0);
      }
      else if(Entities[i]->IsToTake()) {
        GameApp::instance()->getDrawLib()->drawCircle(entityPos, 3, 0, MAKE_COLOR(255,0,0,255), 0);
      }
      else if(Entities[i]->DoesKill()) {
        GameApp::instance()->getDrawLib()->drawCircle(entityPos, 3, 0, MAKE_COLOR(0,0,70,255), 0);
      }
    }
    
#ifdef ENABLE_OPENGL
    glDisable(GL_SCISSOR_TEST);
#endif
    //keesj:todo replace with setClipRect(NULL) in drawlib
    GameApp::instance()->getDrawLib()->setClipRect(0,0,GameApp::instance()->getDrawLib()->getDispWidth(),GameApp::instance()->getDrawLib()->getDispHeight());
  }

void GameRenderer::_RenderGhost(MotoGame* i_scene, Biker* i_ghost, int i) {
  /* Render ghost - ugly mode? */
  if(XMSession::instance()->ugly() == false) {
    if(XMSession::instance()->hideGhosts() == false) { /* ghosts can be hidden, but don't hide text */
      /* No not ugly, fancy! Render into overlay? */      
      if(XMSession::instance()->ghostMotionBlur()
	 && m_allowGhostEffect) {
	m_Overlay.beginRendering();
	m_Overlay.fade(0.15);
      }

      try {
	_RenderBike(i_ghost->getState(), i_ghost->getState()->Parameters(), i_ghost->getBikeTheme(), true,
		    i_ghost->getColorFilter(), i_ghost->getUglyColorFilter());
      } catch(Exception &e) {
	i_scene->gameMessage("Unable to render the ghost", true);
      }

      if(XMSession::instance()->ghostMotionBlur()
	 && m_allowGhostEffect) {
	m_Overlay.endRendering();
	m_Overlay.present();
      }
    }
 
    if(i_ghost->getDescription() != "") {
      if(m_nGhostInfoTrans > 0 && XMSession::instance()->showGhostsInfos()) {
	_RenderInGameText(i_ghost->getState()->CenterP + Vector2f(i*3.0,-1.5f),
			  i_ghost->getDescription(),
			  MAKE_COLOR(255,255,255,m_nGhostInfoTrans));
      }
    }
  }
    
  if(XMSession::instance()->ugly()) {
    if(XMSession::instance()->hideGhosts() == false) { /* ghosts can be hidden, but don't hide text */
      _RenderBike(i_ghost->getState(), i_ghost->getState()->Parameters(),
		  i_ghost->getBikeTheme(),
		  true,
		  i_ghost->getColorFilter(), i_ghost->getUglyColorFilter());
    }
  }
}

int GameRenderer::nbParticlesRendered() const {
  return m_nParticlesRendered;
}

  /*===========================================================================
  Main rendering function
  ===========================================================================*/
  void GameRenderer::render(MotoGame* i_scene) {
    bool v_found;
    Camera*   pCamera = i_scene->getCamera();

    m_nParticlesRendered = 0;
    
    m_rotationAngleForTheFrame = pCamera->guessDesiredAngleRotation();

    pCamera->setCamera2d();

    /* calculate screen AABB to show only visible entities and dyn blocks */

    // calculate scale values with default zoom first
    float currentZoom = pCamera->getCurrentZoom();
    pCamera->initZoom();
    calculateCameraScaleAndScreenAABB(pCamera, m_layersBBox);
    m_xScaleDefault = m_xScale;
    m_yScaleDefault = m_yScale;

    pCamera->setAbsoluteZoom(currentZoom);
    calculateCameraScaleAndScreenAABB(pCamera, m_screenBBox);

    /* SKY! */
    if(XMSession::instance()->ugly() == false) {
      _RenderSky(i_scene,
		 i_scene->getLevelSrc()->Sky()->Zoom(),
		 i_scene->getLevelSrc()->Sky()->Offset(),
		 i_scene->getLevelSrc()->Sky()->TextureColor(),
		 i_scene->getLevelSrc()->Sky()->DriftZoom(),
		 i_scene->getLevelSrc()->Sky()->DriftTextureColor(),
		 i_scene->getLevelSrc()->Sky()->Drifted());
    }    

    if(XMSession::instance()->gameGraphics() == GFX_HIGH && XMSession::instance()->ugly() == false) {
      /* background level blocks */
      _RenderLayer(i_scene, false);
    }

    // the layers may have change the scale transformation
    setCameraTransformations(pCamera, m_xScale, m_yScale);

    if(XMSession::instance()->gameGraphics() != GFX_LOW && XMSession::instance()->ugly() == false) {
      /* Background blocks */
      _RenderDynamicBlocks(i_scene, true);
      _RenderBackground(i_scene);
      
      /* ... then render background sprites ... */      
    }
    _RenderSprites(i_scene, false,true);

    if(XMSession::instance()->gameGraphics() == GFX_HIGH && XMSession::instance()->ugly() == false) {
      /* Render particles (back!) */    
      _RenderParticles(i_scene, false);
    }

    /* ... covered by blocks ... */
    _RenderDynamicBlocks(i_scene, false);
    _RenderBlocks(i_scene);

    /* ... then render "middleground" sprites ... */
    _RenderSprites(i_scene, false,false);

    /* zones */
    if(XMSession::instance()->uglyOver()) {
      for(unsigned int i=0; i<i_scene->getLevelSrc()->Zones().size(); i++) {
	_RenderZone(i_scene->getLevelSrc()->Zones()[i]);
      }
    }

    /* ghosts */
    v_found = false;
    int v_found_i = 0;
    for(unsigned int i=0; i<i_scene->Ghosts().size(); i++) {
      Ghost* v_ghost = i_scene->Ghosts()[i];
      if(v_ghost != pCamera->getPlayerToFollow()) {
	_RenderGhost(i_scene, v_ghost, i);
      } else {
	v_found = true;
	v_found_i = i;
      }
    }
    /* draw the player to follow over the others */
    if(v_found) {
      _RenderGhost(i_scene, i_scene->Ghosts()[v_found_i], v_found_i);
    }

    /* ... followed by the bike ... */
    v_found = false;
    for(unsigned int i=0; i<i_scene->Players().size(); i++) {
      Biker* v_player = i_scene->Players()[i];
      if(v_player != pCamera->getPlayerToFollow()) {
	try {
	  _RenderBike(v_player->getState(),
		      v_player->getState()->Parameters(),
		      v_player->getBikeTheme(),
		      v_player->getRenderBikeFront(),
		      v_player->getColorFilter(),
		      v_player->getUglyColorFilter());
	} catch(Exception &e) {
	  i_scene->gameMessage("Unable to render the biker", true);
	}
      } else {
	v_found = true;
      }
    }
    if(v_found) {
      try {
	_RenderBike(pCamera->getPlayerToFollow()->getState(),
		    pCamera->getPlayerToFollow()->getState()->Parameters(),
		    pCamera->getPlayerToFollow()->getBikeTheme(),
		    pCamera->getPlayerToFollow()->getRenderBikeFront(),
		    pCamera->getPlayerToFollow()->getColorFilter(),
		    pCamera->getPlayerToFollow()->getUglyColorFilter());

	if(XMSession::instance()->debug()) {
	  // render collision points
	  for(unsigned int j=0; j<pCamera->getPlayerToFollow()->CollisionPoints().size(); j++) {
	    _RenderCircle(16, MAKE_COLOR(255,255,0,255),pCamera->getPlayerToFollow()->CollisionPoints()[j], 0.02);
	  }
	}
      } catch(Exception &e) {
	i_scene->gameMessage("Unable to render the biker", true);
      }
    }

    /* ghost information */
    if(i_scene->getTime() > m_fNextGhostInfoUpdate) {
      if(m_nGhostInfoTrans > 0) {
	if(m_fNextGhostInfoUpdate > 1.5f) {
	  m_nGhostInfoTrans-=16;
	}
	m_fNextGhostInfoUpdate += 0.025f;
      }
    }
    
    if(XMSession::instance()->gameGraphics() == GFX_HIGH && XMSession::instance()->ugly() == false) {
      /* Render particles (front!) */    
      _RenderParticles(i_scene, true);
    }
    
    /* ... and finally the foreground sprites! */
    _RenderSprites(i_scene, true,false);

    /* and finally finally, front layers */
    if(XMSession::instance()->gameGraphics() == GFX_HIGH && XMSession::instance()->ugly() == false) {
      _RenderLayers(i_scene, true);
    }

    // put it back
    setCameraTransformations(pCamera, m_xScale, m_yScale);

    if(XMSession::instance()->debug()) {
      /* Draw some collision handling debug info */
      CollisionSystem *pc = i_scene->getCollisionHandler();
      for(unsigned int i=0;i<pc->m_CheckedLines.size();i++) {
        GameApp::instance()->getDrawLib()->setLineWidth(3);
	GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
	GameApp::instance()->getDrawLib()->setColorRGB(255,0,0);
        GameApp::instance()->getDrawLib()->glVertex(pc->m_CheckedLines[i]->x1,pc->m_CheckedLines[i]->y1);
        GameApp::instance()->getDrawLib()->glVertex(pc->m_CheckedLines[i]->x2,pc->m_CheckedLines[i]->y2);
	GameApp::instance()->getDrawLib()->endDraw();
        GameApp::instance()->getDrawLib()->setLineWidth(2);
      }
      for(unsigned int i=0;i<pc->m_CheckedCells.size();i++) {
	GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_LOOP);
	GameApp::instance()->getDrawLib()->setColorRGB(255,0,0);

        GameApp::instance()->getDrawLib()->glVertex(pc->m_CheckedCells[i].x1,pc->m_CheckedCells[i].y1);
        GameApp::instance()->getDrawLib()->glVertex(pc->m_CheckedCells[i].x2,pc->m_CheckedCells[i].y1);
        GameApp::instance()->getDrawLib()->glVertex(pc->m_CheckedCells[i].x2,pc->m_CheckedCells[i].y2);
        GameApp::instance()->getDrawLib()->glVertex(pc->m_CheckedCells[i].x1,pc->m_CheckedCells[i].y2);
	GameApp::instance()->getDrawLib()->endDraw();
      }
      for(unsigned int i=0;i<pc->m_CheckedLinesW.size();i++) {
        GameApp::instance()->getDrawLib()->setLineWidth(1);
	GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
	GameApp::instance()->getDrawLib()->setColorRGB(0,255,0);
        GameApp::instance()->getDrawLib()->glVertex(pc->m_CheckedLinesW[i]->x1,pc->m_CheckedLinesW[i]->y1);
        GameApp::instance()->getDrawLib()->glVertex(pc->m_CheckedLinesW[i]->x2,pc->m_CheckedLinesW[i]->y2);
	GameApp::instance()->getDrawLib()->endDraw();
        GameApp::instance()->getDrawLib()->setLineWidth(1);
      }
      for(unsigned int i=0;i<pc->m_CheckedCellsW.size();i++) {
	GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_LOOP);
	GameApp::instance()->getDrawLib()->setColorRGB(0,255,0);
        GameApp::instance()->getDrawLib()->glVertex(pc->m_CheckedCellsW[i].x1,pc->m_CheckedCellsW[i].y1);
        GameApp::instance()->getDrawLib()->glVertex(pc->m_CheckedCellsW[i].x2,pc->m_CheckedCellsW[i].y1);
        GameApp::instance()->getDrawLib()->glVertex(pc->m_CheckedCellsW[i].x2,pc->m_CheckedCellsW[i].y2);
        GameApp::instance()->getDrawLib()->glVertex(pc->m_CheckedCellsW[i].x1,pc->m_CheckedCellsW[i].y2);
	GameApp::instance()->getDrawLib()->endDraw();
      }

      std::vector<Entity*>& v = pc->getCheckedEntities();
      for(unsigned int i=0; i<v.size(); i++) {
	// draw entities in the cells
	Entity* pSprite = v[i];
	Vector2f C = pSprite->DynamicPosition();
	Color v_color;
      
	switch(pSprite->Speciality()) {
	case ET_KILL:
	  v_color = MAKE_COLOR(80,255,255,255); /* Fix: color changed a bit so it's easier to spot */
	  break;
	case ET_MAKEWIN:
	  v_color = MAKE_COLOR(255,255,0,255); /* Fix: color not same as blocks */
	  break;
	case ET_ISTOTAKE:
	  v_color = MAKE_COLOR(255,0,0,255);
	  break;
	default:
	  v_color = MAKE_COLOR(50,50,50,255); /* Fix: hard-to-see color because of entity's insignificance */
	  break;
	}

	_RenderCircle(20, v_color, C, pSprite->Size()+0.2f);
      }

      /* Render debug info */
      _RenderDebugInfo();
    }

    pCamera->setCamera2d();

    /* minimap + counter */
    if(pCamera->getPlayerToFollow() != NULL) {
      if(showMinimap()) {
	renderMiniMap(i_scene, 0, GameApp::instance()->getDrawLib()->getDispHeight()-100,
		      150,100);
      }
      if(showEngineCounter()
	 && XMSession::instance()->ugly() == false
	 && i_scene->getNumberCameras() == 1) {
	renderEngineCounter(GameApp::instance()->getDrawLib()->getDispWidth()-128,
			    GameApp::instance()->getDrawLib()->getDispHeight()-128,128,128,
			    pCamera->getPlayerToFollow()->getBikeEngineSpeed());
      }
    }

    GameApp::instance()->getDrawLib()->getMenuCamera()->setCamera2d();

    if(m_showTimePanel) {
      renderTimePanel(i_scene);
      /* If there's strawberries in the level, tell the user how many there's left */
      _RenderGameStatus(i_scene);
    }

    renderReplayHelpMessage(i_scene);

    /* And then the game messages */
    _RenderGameMessages(i_scene);            

    FontManager* v_fm = GameApp::instance()->getDrawLib()->getFontMedium();
    FontGlyph* v_fg = v_fm->getGlyph(i_scene->getInfos());
    v_fm->printString(v_fg,
		      5,
		      GameApp::instance()->getDrawLib()->getDispHeight() - v_fg->realHeight() - 2,
		      MAKE_COLOR(255,255,255,255), true);
  }

  /*===========================================================================
  Game status rendering
  ===========================================================================*/
  void GameRenderer::_RenderGameStatus(MotoGame* i_scene) {
    Sprite* pType = NULL;

    // do not render it if it's the autozoom camera or ...
    if(i_scene->isAutoZoomCamera() == true) {
      return;
    }
    
    float x1 = 125;
    float y1 = 2;
    float x2 = 100;
    float y2 = 27;

    int nStrawberriesLeft = i_scene->getLevelSrc()->countToTakeEntities();
    int nQuantity = 0;
    Vector2i bottomLeft(0,0);
    if(i_scene->getNumberCameras() > 1){
      bottomLeft = i_scene->getCamera()->getDispBottomLeft();
    }

    // adapt to the current camera
    float x1_cam = x1 + bottomLeft.x;
    float x2_cam = x2 + bottomLeft.x;
    float y1_cam = y1;
    float y2_cam = y2;
    if((unsigned int)bottomLeft.y != GameApp::instance()->getDrawLib()->getDispHeight()){
      y1_cam += bottomLeft.y;
      y2_cam += bottomLeft.y;
    }

    if(XMSession::instance()->ugly() == false) {
      pType = Theme::instance()->getSprite(SPRITE_TYPE_ANIMATION, i_scene->getLevelSrc()->SpriteForFlower());
      if(pType == NULL) {
	pType = Theme::instance()->getSprite(SPRITE_TYPE_DECORATION, i_scene->getLevelSrc()->SpriteForFlower());
      }
    }
    
    if(nStrawberriesLeft > 0) {
      if(XMSession::instance()->ugly() == false) {
	pType = Theme::instance()->getSprite(SPRITE_TYPE_ANIMATION, i_scene->getLevelSrc()->SpriteForStrawberry());
      if(pType == NULL) {
	pType = Theme::instance()->getSprite(SPRITE_TYPE_DECORATION, i_scene->getLevelSrc()->SpriteForStrawberry());
      }
      }
      nQuantity = nStrawberriesLeft;
    }
            
    if(pType != NULL) {
      if(pType->getBlendMode() == SPRITE_BLENDMODE_ADDITIVE) {
	_RenderAdditiveBlendedSection(pType->getTexture(), Vector2f(x2_cam,GameApp::instance()->getDrawLib()->getDispHeight()-y2_cam),Vector2f(x1_cam,GameApp::instance()->getDrawLib()->getDispHeight()-y2_cam),Vector2f(x1_cam,GameApp::instance()->getDrawLib()->getDispHeight()-y1_cam),Vector2f(x2_cam,GameApp::instance()->getDrawLib()->getDispHeight()-y1_cam));      
      } else {
#ifdef ENABLE_OPENGL
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL,0.5f);      
#endif
	_RenderAlphaBlendedSection(pType->getTexture(), Vector2f(x2_cam,GameApp::instance()->getDrawLib()->getDispHeight()-y2_cam),Vector2f(x1_cam,GameApp::instance()->getDrawLib()->getDispHeight()-y2_cam),Vector2f(x1_cam,GameApp::instance()->getDrawLib()->getDispHeight()-y1_cam),Vector2f(x2_cam,GameApp::instance()->getDrawLib()->getDispHeight()-y1_cam));      
#ifdef ENABLE_OPENGL
	glDisable(GL_ALPHA_TEST);
#endif
      }
    }

    if(nQuantity > 0) {
      char cBuf[256];    
      sprintf(cBuf,"%d",nQuantity);

      /* Draw text */
      FontManager* v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
      FontGlyph* v_fg = v_fm->getGlyph(cBuf);

      v_fm->printString(v_fg,
			(int)((x1_cam+x2_cam)/2 - v_fg->realWidth()/2),
			(int)((y1_cam+y2_cam)/2 - v_fg->realHeight()/2),
			MAKE_COLOR(255,255,0,255));
    }
  }
  
  /*===========================================================================
  Game message rendering
  ===========================================================================*/
  void GameRenderer::_RenderGameMessages(MotoGame* i_scene) {
    float v_fZoom = 60.0f;

    /* Arrow messages */
    ArrowPointer *pArrow = &(i_scene->getArrowPointer());
    if(pArrow->nArrowPointerMode != 0) {
      Vector2f C;
      if(pArrow->nArrowPointerMode == 1) {          
        C=Vector2f(GameApp::instance()->getDrawLib()->getDispWidth()/2 + (float)(pArrow->ArrowPointerPos.x - i_scene->getCamera()->getCameraPositionX())*v_fZoom,
                  GameApp::instance()->getDrawLib()->getDispHeight()/2 - (float)(pArrow->ArrowPointerPos.y - i_scene->getCamera()->getCameraPositionY())*v_fZoom);      
      }
      else if(pArrow->nArrowPointerMode == 2) {          
        C.x=(GameApp::instance()->getDrawLib()->getDispWidth() * pArrow->ArrowPointerPos.x) / 800.0f;
        C.y=(GameApp::instance()->getDrawLib()->getDispHeight() * pArrow->ArrowPointerPos.y) / 600.0f;
      }
      Vector2f p1,p2,p3,p4;
      p1 = Vector2f(1,0); p1.rotateXY(pArrow->fArrowPointerAngle);
      p2 = Vector2f(1,0); p2.rotateXY(90+pArrow->fArrowPointerAngle);
      p3 = Vector2f(1,0); p3.rotateXY(180+pArrow->fArrowPointerAngle);
      p4 = Vector2f(1,0); p4.rotateXY(270+pArrow->fArrowPointerAngle);

      p1 = p1 * 50.0f;
      p2 = p2 * 50.0f;
      p3 = p3 * 50.0f;
      p4 = p4 * 50.0f;

      MiscSprite* pType;
      pType = (MiscSprite*) Theme::instance()->getSprite(SPRITE_TYPE_MISC, "Arrow");
      if(pType != NULL) {
	_RenderAlphaBlendedSectionSP(pType->getTexture(),p1+C,p2+C,p3+C,p4+C);      
      }
    }
        
    /* Messages */
    if(i_scene != NULL) {
      for(unsigned int i=0;i<i_scene->getGameMessage().size();i++) {
        GameMessage *pMsg = i_scene->getGameMessage()[i];
	FontManager* v_fm = GameApp::instance()->getDrawLib()->getFontMedium();
	FontGlyph* v_fg = v_fm->getGlyph(pMsg->Text);
	v_fm->printString(v_fg,
			  (int)(GameApp::instance()->getDrawLib()->getDispWidth()/2 - v_fg->realWidth()/2),
			  (int)(pMsg->Pos[1]*GameApp::instance()->getDrawLib()->getDispHeight()),
			  MAKE_COLOR(255,255,255,pMsg->nAlpha), true);
      }
    }
  }
  
  /*===========================================================================
  Sprite rendering main
  ===========================================================================*/
void GameRenderer::_RenderSprites(MotoGame* i_scene, bool bForeground,bool bBackground) {
    Entity *pEnt;

    AABB screenBigger;
    Vector2f screenMin = m_screenBBox.getBMin();
    Vector2f screenMax = m_screenBBox.getBMax();
    /* to avoid sprites being clipped out of the screen,
       we draw also the nearest one */
#define ENTITY_OFFSET 5.0f
    screenBigger.addPointToAABB2f(screenMin.x-ENTITY_OFFSET,
				  screenMin.y-ENTITY_OFFSET);
    screenBigger.addPointToAABB2f(screenMax.x+ENTITY_OFFSET,
				  screenMax.y+ENTITY_OFFSET);

    /* DONE::display only visible entities */
    std::vector<Entity*> Entities = i_scene->getCollisionHandler()->getEntitiesNearPosition(screenBigger);

    for(unsigned int i=0; i<Entities.size(); i++) {
      pEnt = Entities[i];

      try {

      switch(pEnt->Speciality()) {
        case ET_NONE:
          /* Middleground? (not foreground, not background) */
          if(pEnt->Z() == 0.0f && !bForeground && !bBackground) {
            _RenderSprite(i_scene, pEnt);  
          } 
          else {
            /* In front? */
            if(pEnt->Z() > 0.0f && bForeground) {
              _RenderSprite(i_scene, pEnt);
            } 
            else {
              /* Those in back? */
              if(pEnt->Z() < 0.0f && bBackground) {
                _RenderSprite(i_scene, pEnt);
              }
            }
          }
          break;
      default:
          if(!bForeground && !bBackground) {
	    switch(pEnt->Speciality()) {
	    case ET_MAKEWIN:
	      _RenderSprite(i_scene, pEnt, m_sizeMultOfEntitiesWhichMakeWin);
	      break;
	    case ET_ISTOTAKE:
	      _RenderSprite(i_scene, pEnt, m_sizeMultOfEntitiesToTake);
	      break;
	    default:	      
	      _RenderSprite(i_scene, pEnt);
	    }
          }
          break;
      }
      } catch(Exception &e) {
	i_scene->gameMessage("Unable to render a sprite", true);
      }

    }
}

  /*===========================================================================
  Render a sprite
  ===========================================================================*/
void GameRenderer::_RenderSprite(MotoGame* i_scene, Entity *pSprite, float i_sizeMult) {  
    Sprite* v_spriteType;
    AnimationSprite* v_animationSpriteType;
    DecorationSprite* v_decorationSpriteType;
    float v_centerX = 0.0f;
    float v_centerY = 0.0f;
    float v_width   = 0.0f;
    float v_height  = 0.0f;
    std::string v_sprite_type;

    if(XMSession::instance()->ugly() == false) {
      switch(pSprite->Speciality()) {
      case ET_KILL:
	v_sprite_type = i_scene->getLevelSrc()->SpriteForWecker();
        break;
      case ET_MAKEWIN:
	v_sprite_type = i_scene->getLevelSrc()->SpriteForFlower();
        break;
      case ET_ISTOTAKE:
	v_sprite_type = i_scene->getLevelSrc()->SpriteForStrawberry();
        break;
      default:
	v_sprite_type = pSprite->SpriteName();
      }

      /* search the sprite as an animation */
      v_animationSpriteType = (AnimationSprite*) Theme::instance()->getSprite(SPRITE_TYPE_ANIMATION, v_sprite_type);
      /* if the sprite is not an animation, it's perhaps a decoration */
      if(v_animationSpriteType != NULL) {
        v_spriteType = v_animationSpriteType;
        v_centerX = v_animationSpriteType->getCenterX();
        v_centerY = v_animationSpriteType->getCenterY();

        if(pSprite->Width() > 0.0) {
          v_width  = pSprite->Width();
          v_height = pSprite->Height();
          v_centerX += (pSprite->Width() -v_animationSpriteType->getWidth())  / 2.0;
          v_centerY += (pSprite->Height()-v_animationSpriteType->getHeight()) / 2.0;   
        } else {
          v_width  = v_animationSpriteType->getWidth();
          v_height = v_animationSpriteType->getHeight();
        }
      } else {
        v_decorationSpriteType = (DecorationSprite*) Theme::instance()->getSprite(SPRITE_TYPE_DECORATION, v_sprite_type);
        v_spriteType = v_decorationSpriteType;

        if(v_decorationSpriteType != NULL) {
          v_centerX = v_decorationSpriteType->getCenterX();
          v_centerY = v_decorationSpriteType->getCenterY();

          if(pSprite->Width()  > 0.0) {
            v_width  = pSprite->Width();
            v_height = pSprite->Height();
            /* adjust */
            v_centerX += (pSprite->Width() -v_decorationSpriteType->getWidth())  / 2.0;
            v_centerY += (pSprite->Height()-v_decorationSpriteType->getHeight()) / 2.0;
          } else {
            /* use the theme values */
            v_width  = v_decorationSpriteType->getWidth();
            v_height = v_decorationSpriteType->getHeight();
          }
        }
      }

      if(i_sizeMult != 1.0) {
	v_centerX -= (v_width  - (v_width  * i_sizeMult)) / 2.0;
	v_centerY -= (v_height - (v_height * i_sizeMult)) / 2.0;
	v_width  *= i_sizeMult;
	v_height *= i_sizeMult;
      }	

      if(v_spriteType != NULL) {
        /* Draw it */
        Vector2f p[4];
        
	p[0] = Vector2f(0.0    , 0.0);
	p[1] = Vector2f(v_width, 0.0);
	p[2] = Vector2f(v_width, v_height);
	p[3] = Vector2f(0.0    , v_height);

	/* positionne according the the center */
	p[0] -= Vector2f(v_centerX, v_centerY);
	p[1] -= Vector2f(v_centerX, v_centerY);
	p[2] -= Vector2f(v_centerX, v_centerY);
	p[3] -= Vector2f(v_centerX, v_centerY);

	/* apply rotation */
	if(pSprite->DrawAngle() != 0.0) { /* generally not nice to test a float and 0.0
					 but i will be false in the majority of the time
				       */
	  float beta;
	  float v_ray;
	  
	  for(unsigned int i=0; i<4; i++) {
	    v_ray = sqrt((p[i].x*p[i].x) + (p[i].y*p[i].y));
	    beta = 0.0;

	    if(p[i].x >= 0.0 && p[i].y >= 0.0) {
	      beta = acosf(p[i].x / v_ray);
	    } else if(p[i].x < 0.0 && p[i].y >= 0.0) {
	      beta = acosf(p[i].y / v_ray) + M_PI / 2.0;
	    } else if(p[i].x < 0.0 && p[i].y < 0.0) {
	      beta = acosf(-p[i].x / v_ray) + M_PI;
	    } else {
	      beta = acosf(-p[i].y / v_ray) - M_PI / 2.0;
	    }
	    
	    p[i].x = (cosf(pSprite->DrawAngle() + beta) * v_ray);
	    p[i].y = (sinf(pSprite->DrawAngle() + beta) * v_ray);
	  }
	  //pSprite->setDrawAngle(pSprite->DrawAngle() + 0.01);
	}

	/* reversed ? */
	if(pSprite->DrawReversed()) {
	  Vector2f v_tmp;
	  v_tmp = p[0];
	  p[0] = p[1];
	  p[1] = v_tmp;
	  v_tmp = p[2];
	  p[2] = p[3];
	  p[3] = v_tmp;
	} 

	/* vector to the good position */
	p[0] += pSprite->DynamicPosition();
	p[1] += pSprite->DynamicPosition();
	p[2] += pSprite->DynamicPosition();
	p[3] += pSprite->DynamicPosition();

        if(v_spriteType->getBlendMode() == SPRITE_BLENDMODE_ADDITIVE) {
          _RenderAdditiveBlendedSection(v_spriteType->getTexture(),p[0],p[1],p[2],p[3]);      
        }
        else {
#ifdef ENABLE_OPENGL
          glEnable(GL_ALPHA_TEST);
          glAlphaFunc(GL_GEQUAL,0.5f);      
#endif
          _RenderAlphaBlendedSection(v_spriteType->getTexture(),p[0],p[1],p[2],p[3]);      
#ifdef ENABLE_OPENGL
          glDisable(GL_ALPHA_TEST);
#endif
        }
      }    
    }
    /* If this is debug-mode, also draw entity's area of effect */
    if(XMSession::instance()->debug() || XMSession::instance()->testTheme() || XMSession::instance()->ugly()) {
      Vector2f C = pSprite->DynamicPosition();
      Color v_color;
      
      switch(pSprite->Speciality()) {
      case ET_KILL:
        v_color = MAKE_COLOR(80,255,255,255); /* Fix: color changed a bit so it's easier to spot */
        break;
      case ET_MAKEWIN:
        v_color = MAKE_COLOR(255,255,0,255); /* Fix: color not same as blocks */
        break;
      case ET_ISTOTAKE:
        v_color = MAKE_COLOR(255,0,0,255);
        break;
      default:
        v_color = MAKE_COLOR(50,50,50,255); /* Fix: hard-to-see color because of entity's insignificance */
        break;
      }

      _RenderCircle(20, v_color, C, pSprite->Size() * i_sizeMult);
    }
  }
     
  /*===========================================================================
  Blocks (dynamic)
  ===========================================================================*/
void GameRenderer::_RenderDynamicBlocks(MotoGame* i_scene, bool bBackground) {
    /* FIX::display only visible dyn blocks */
    std::vector<Block *> Blocks = i_scene->getCollisionHandler()->getDynBlocksNearPosition(m_screenBBox);

    /* sort blocks on their texture */
    std::sort(Blocks.begin(), Blocks.end(), AscendingTextureSort());

    if(XMSession::instance()->ugly() == false) {
      for(unsigned int i=0; i<Blocks.size(); i++) {
	/* Are we rendering background blocks or what? */
	if(Blocks[i]->isBackground() != bBackground)
	  continue;

	Block* block = Blocks[i];
	/* Build rotation matrix for block */
	float fR[4];
	float rotation = block->DynamicRotation();
	fR[0] =  cosf(rotation);
	fR[2] =  sinf(rotation);
	fR[1] = -fR[2];
	fR[3] =  fR[0];

	Vector2f dynRotCenter = block->DynamicRotationCenter();
	Vector2f dynPos       = block->DynamicPosition();
	int geom = block->getGeom();

	if(GameApp::instance()->getDrawLib()->getBackend() == DrawLib::backend_OpenGl) {
#ifdef ENABLE_OPENGL
	  glEnableClientState(GL_VERTEX_ARRAY);
	  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	  /* we're working with modelview matrix*/
	  glPushMatrix();

	  glTranslatef(dynPos.x, dynPos.y, 0);
	  if(rotation != 0.0){
	    glTranslatef(dynRotCenter.x, dynRotCenter.y, 0);
	    glRotatef(rad2deg(rotation), 0, 0, 1);
	    glTranslatef(-dynRotCenter.x, -dynRotCenter.y, 0);
	  }

	  GameApp::instance()->getDrawLib()->setTexture(m_DynamicGeoms[geom]->pTexture, BLEND_MODE_A);
	  GameApp::instance()->getDrawLib()->setColorRGB(255, 255, 255);

	  /* VBO optimized? */
	  if(GameApp::instance()->getDrawLib()->useVBOs()) {
	    for(unsigned int j=0;j<m_DynamicGeoms[geom]->Polys.size();j++) {          
	      GeomPoly *pPoly = m_DynamicGeoms[geom]->Polys[j];

	      ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nVertexBufferID);
	      glVertexPointer(2,GL_FLOAT,0,(char *)NULL);

	      ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nTexCoordBufferID);
	      glTexCoordPointer(2,GL_FLOAT,0,(char *)NULL);

	      glDrawArrays(GL_POLYGON,0,pPoly->nNumVertices);
	    }      
	  } else {
	    for(unsigned int j=0;j<m_DynamicGeoms[geom]->Polys.size();j++) {          
	      GeomPoly *pPoly = m_DynamicGeoms[geom]->Polys[j];
	      glVertexPointer(2,   GL_FLOAT, 0, pPoly->pVertices);
	      glTexCoordPointer(2, GL_FLOAT, 0, pPoly->pTexCoords);
	      glDrawArrays(GL_POLYGON, 0, pPoly->nNumVertices);
	    }
	  }

	  _RenderBlockEdges(block);

	  glPopMatrix();

	  glDisableClientState(GL_VERTEX_ARRAY);
	  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
	} else if(GameApp::instance()->getDrawLib()->getBackend() == DrawLib::backend_SdlGFX){

	  if(m_DynamicGeoms[geom]->Polys.size() > 0) {
	    GameApp::instance()->getDrawLib()->setTexture(m_DynamicGeoms[geom]->pTexture,BLEND_MODE_A);
	    GameApp::instance()->getDrawLib()->setColorRGB(255,255,255);

	    for(unsigned int j=0;j<m_DynamicGeoms[geom]->Polys.size();j++) {          
	      GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
	      for(unsigned int k=0;k<m_DynamicGeoms[geom]->Polys[j]->nNumVertices;k++) {
		Vector2f vertex = Vector2f(m_DynamicGeoms[geom]->Polys[j]->pVertices[k].x,
					   m_DynamicGeoms[geom]->Polys[j]->pVertices[k].y);
		/* transform vertex */
		Vector2f transVertex = Vector2f((vertex.x-dynRotCenter.x)*fR[0] + (vertex.y-dynRotCenter.y)*fR[1],
						(vertex.x-dynRotCenter.x)*fR[2] + (vertex.y-dynRotCenter.y)*fR[3]);
		transVertex += dynPos + dynRotCenter;
		
		GameApp::instance()->getDrawLib()->glTexCoord(m_DynamicGeoms[geom]->Polys[j]->pTexCoords[k].x,
						      m_DynamicGeoms[geom]->Polys[j]->pTexCoords[k].y);
		GameApp::instance()->getDrawLib()->glVertex(transVertex.x, transVertex.y);
	      }
	      GameApp::instance()->getDrawLib()->endDrawKeepProperties();
	    }
	    GameApp::instance()->getDrawLib()->removePropertiesAfterEnd();
	  }

	}
      }
      if(GameApp::instance()->getDrawLib()->getBackend() == DrawLib::backend_SdlGFX){
	/* Render all special edges (if quality!=low) */
	if(XMSession::instance()->gameGraphics() != GFX_LOW) {
	  for(unsigned int i=0;i<Blocks.size();i++) {
	    if(Blocks[i]->isBackground() == bBackground){
	      _RenderBlockEdges(Blocks[i]);
	    }
	  }
	}
      }
    }

    if(XMSession::instance()->ugly() || XMSession::instance()->uglyOver()) {
      for(unsigned int i=0; i<Blocks.size(); i++) {
	/* Are we rendering background blocks or what? */
	if(Blocks[i]->isBackground() != bBackground)
	  continue;

	/* Build rotation matrix for block */
	float fR[4];
	float rotation = Blocks[i]->DynamicRotation();
	fR[0] =  cosf(rotation);
	fR[2] =  sinf(rotation);
	fR[1] = -fR[2];
	fR[3] =  fR[0];

	Vector2f dynRotCenter = Blocks[i]->DynamicRotationCenter();
	Vector2f dynPos       = Blocks[i]->DynamicPosition();

	GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_LOOP);
	GameApp::instance()->getDrawLib()->setColorRGB(255,255,255);

        for(unsigned int j=0;j<Blocks[i]->Vertices().size();j++) {
	  Vector2f vertex = Blocks[i]->Vertices()[j]->Position();
	  /* transform vertex */
	  Vector2f transVertex = Vector2f((vertex.x-dynRotCenter.x)*fR[0] + (vertex.y-dynRotCenter.y)*fR[1],
					  (vertex.x-dynRotCenter.x)*fR[2] + (vertex.y-dynRotCenter.y)*fR[3]);
	  transVertex += dynPos + dynRotCenter;

          GameApp::instance()->getDrawLib()->glVertex(transVertex.x, transVertex.y);
        }
	GameApp::instance()->getDrawLib()->endDraw();
      }
    }

  }

  void GameRenderer::_RenderBlock(Block* block)
  {
    int geom = block->getGeom();
    GameApp::instance()->getDrawLib()->setTexture(m_StaticGeoms[geom]->pTexture, BLEND_MODE_A);
    GameApp::instance()->getDrawLib()->setColorRGB(255, 255, 255);

    if(GameApp::instance()->getDrawLib()->getBackend() == DrawLib::backend_OpenGl) {
#ifdef ENABLE_OPENGL
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);

      /* VBO optimized? */
      if(GameApp::instance()->getDrawLib()->useVBOs()) {
	for(unsigned int j=0;j<m_StaticGeoms[geom]->Polys.size();j++) {          
	  GeomPoly *pPoly = m_StaticGeoms[geom]->Polys[j];

	  ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nVertexBufferID);
	  glVertexPointer(2,GL_FLOAT,0,(char *)NULL);

	  ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nTexCoordBufferID);
	  glTexCoordPointer(2,GL_FLOAT,0,(char *)NULL);

	  glDrawArrays(GL_POLYGON,0,pPoly->nNumVertices);
	}      
      } else {
	for(unsigned int j=0;j<m_StaticGeoms[geom]->Polys.size();j++) {          
	  GeomPoly *pPoly = m_StaticGeoms[geom]->Polys[j];
	  glVertexPointer(2,   GL_FLOAT, 0, pPoly->pVertices);
	  glTexCoordPointer(2, GL_FLOAT, 0, pPoly->pTexCoords);
	  glDrawArrays(GL_POLYGON, 0, pPoly->nNumVertices);
	}
      }

      _RenderBlockEdges(block);

      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
    } else if(GameApp::instance()->getDrawLib()->getBackend() == DrawLib::backend_SdlGFX){

      for(unsigned int j=0;j<m_StaticGeoms[geom]->Polys.size();j++) {
	GameApp::instance()->getDrawLib()->setTexture(m_StaticGeoms[geom]->pTexture,BLEND_MODE_A);
	GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
	GameApp::instance()->getDrawLib()->setColorRGB(255,255,255);
	for(unsigned int k=0;k<m_StaticGeoms[geom]->Polys[j]->nNumVertices;k++) {
	  GameApp::instance()->getDrawLib()->glTexCoord(m_StaticGeoms[geom]->Polys[j]->pTexCoords[k].x,
						m_StaticGeoms[geom]->Polys[j]->pTexCoords[k].y);
	  GameApp::instance()->getDrawLib()->glVertex(m_StaticGeoms[geom]->Polys[j]->pVertices[k].x,
					      m_StaticGeoms[geom]->Polys[j]->pVertices[k].y);
	}
	GameApp::instance()->getDrawLib()->endDraw();
      }
    }
  }

void GameRenderer::_RenderBlockEdges(Block* pBlock)
{
  if(GameApp::instance()->getDrawLib()->getBackend() == DrawLib::backend_OpenGl) {
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    for(unsigned int i=0; i<pBlock->getEdgeGeoms().size(); i++){
      int geom = pBlock->getEdgeGeoms()[i];
      GameApp::instance()->getDrawLib()->setTexture(m_edgeGeoms[geom]->pTexture, BLEND_MODE_A);
      GameApp::instance()->getDrawLib()->setColorRGB(255, 255, 255);

      /* VBO optimized? */
      if(GameApp::instance()->getDrawLib()->useVBOs()) {
	for(unsigned int j=0;j<m_edgeGeoms[geom]->Polys.size();j++) {          
	  GeomPoly *pPoly = m_edgeGeoms[geom]->Polys[j];

	  ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nVertexBufferID);
	  glVertexPointer(2,GL_FLOAT,0,(char *)NULL);

	  ((DrawLibOpenGL*)GameApp::instance()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nTexCoordBufferID);
	  glTexCoordPointer(2,GL_FLOAT,0,(char *)NULL);

	  glDrawArrays(GL_QUADS, 0, pPoly->nNumVertices);
	}      
      } else {
	for(unsigned int j=0;j<m_edgeGeoms[geom]->Polys.size();j++) {          
	  GeomPoly *pPoly = m_edgeGeoms[geom]->Polys[j];
	  glVertexPointer(2,   GL_FLOAT, 0, pPoly->pVertices);
	  glTexCoordPointer(2, GL_FLOAT, 0, pPoly->pTexCoords);
	  glDrawArrays(GL_QUADS, 0, pPoly->nNumVertices);
	}
      }
    }
    //    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  } else if(GameApp::instance()->getDrawLib()->getBackend() == DrawLib::backend_SdlGFX){
    // SDLGFX::TODO
  }
}

  /*===========================================================================
  Blocks (static)
  ===========================================================================*/
  void GameRenderer::_RenderBlocks(MotoGame* i_scene) {

    for(int layer=-1; layer<=0; layer++){
      std::vector<Block *> Blocks;

      /* Render all non-background blocks */
      Blocks = i_scene->getCollisionHandler()->getStaticBlocksNearPosition(m_screenBBox, layer);

      /* sort blocks on their texture */
      std::sort(Blocks.begin(), Blocks.end(), AscendingTextureSort());

      /* Ugly mode? */
      if(XMSession::instance()->ugly() == false) {
	/* Render all non-background blocks */
	/* Static geoms... */
	for(unsigned int i=0;i<Blocks.size();i++) {
	  if(Blocks[i]->isBackground() == false) {
	    _RenderBlock(Blocks[i]);
	  }
	}
	if(GameApp::instance()->getDrawLib()->getBackend() == DrawLib::backend_SdlGFX){
	  /* Render all special edges (if quality!=low) */
	  if(XMSession::instance()->gameGraphics() != GFX_LOW) {
	    for(unsigned int i=0;i<Blocks.size();i++) {
	      if(Blocks[i]->isBackground() == false) {
		_RenderBlockEdges(Blocks[i]);
	      }
	    }
	  }
	}
      }

      if(XMSession::instance()->ugly()) {
	for(unsigned int i=0;i<Blocks.size();i++) {
	  if(Blocks[i]->isBackground() == false) {
	    GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_LOOP);
	    GameApp::instance()->getDrawLib()->setColorRGB(255,255,255);
	    for(unsigned int j=0;j<Blocks[i]->Vertices().size();j++) {
	      GameApp::instance()->getDrawLib()->glVertex(Blocks[i]->Vertices()[j]->Position().x + Blocks[i]->DynamicPosition().x,
						  Blocks[i]->Vertices()[j]->Position().y + Blocks[i]->DynamicPosition().y);
	    }
	    GameApp::instance()->getDrawLib()->endDraw();
	  }
	}
      }

      if(XMSession::instance()->uglyOver()) {
	for(unsigned int i=0; i<Blocks.size(); i++) {
	  for(unsigned int j=0; j<Blocks[i]->ConvexBlocks().size(); j++) {

	    GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_LOOP);

	    if(Blocks[i]->isBackground() == false) {
	      GameApp::instance()->getDrawLib()->setColorRGB(255,255,0);
	    } else {
	      GameApp::instance()->getDrawLib()->setColorRGB(255,255,255);
	    }
	    for(unsigned int k=0; k<Blocks[i]->ConvexBlocks()[j]->Vertices().size(); k++) {
	      GameApp::instance()->getDrawLib()->glVertex(Blocks[i]->ConvexBlocks()[j]->Vertices()[k]->Position().x + Blocks[i]->DynamicPosition().x,
						  Blocks[i]->ConvexBlocks()[j]->Vertices()[k]->Position().y + Blocks[i]->DynamicPosition().y);
	    }
	    GameApp::instance()->getDrawLib()->endDraw();

	  }
	}
      }
    }
  }

  void GameRenderer::_RenderZone(Zone *i_zone) {
    ZonePrim *v_prim;
    ZonePrimBox *v_primbox;

    for(unsigned int i=0; i<i_zone->Prims().size(); i++) {
      v_prim = i_zone->Prims()[i];
      if(v_prim->Type() == LZPT_BOX) {
	v_primbox = static_cast<ZonePrimBox*>(v_prim);
	_RenderRectangle(Vector2f(v_primbox->Left(),  v_primbox->Top()),
			 Vector2f(v_primbox->Right(), v_primbox->Bottom()),
			 MAKE_COLOR(255, 0, 0, 255));
      }
    }
  }

  /*===========================================================================
  Sky.
  ===========================================================================*/
void GameRenderer::_RenderSky(MotoGame* i_scene, float i_zoom, float i_offset, const TColor& i_color,
																float i_driftZoom, const TColor& i_driftColor, bool i_drifted) {
  TextureSprite* pType;
  float fDrift = 0.0;
  float uZoom = 1.0 / i_zoom;
  float uDriftZoom = 1.0 / i_driftZoom;

  if(XMSession::instance()->gameGraphics() != GFX_HIGH) {
    i_drifted = false;
  }
  
  pType = (TextureSprite*) Theme::instance()->getSprite(SPRITE_TYPE_TEXTURE,
							      i_scene->getLevelSrc()->Sky()->Texture());
  
  if(pType != NULL) {
    if(i_drifted) {
      GameApp::instance()->getDrawLib()->setTexture(pType->getTexture(), BLEND_MODE_A);
    }
    
    GameApp::instance()->getDrawLib()->setTexture(pType->getTexture(),BLEND_MODE_NONE);
    GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
    GameApp::instance()->getDrawLib()->setColorRGBA(i_color.Red() , i_color.Green(), i_color.Blue(), i_color.Alpha());
    
    if(i_drifted) {
      fDrift = GameApp::instance()->getXMTime() / 25.0f;
    }
    
    GameApp::instance()->getDrawLib()->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset+ fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset);
    GameApp::instance()->getDrawLib()->glVertexSP(0,0);
    GameApp::instance()->getDrawLib()->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset+uZoom+ fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset);
    GameApp::instance()->getDrawLib()->glVertexSP(GameApp::instance()->getDrawLib()->getDispWidth(),0);
    GameApp::instance()->getDrawLib()->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset+uZoom+ fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset+uZoom);
    GameApp::instance()->getDrawLib()->glVertexSP(GameApp::instance()->getDrawLib()->getDispWidth(),GameApp::instance()->getDrawLib()->getDispHeight());
    GameApp::instance()->getDrawLib()->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset+ fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset+uZoom);
    GameApp::instance()->getDrawLib()->glVertexSP(0,GameApp::instance()->getDrawLib()->getDispHeight());
    GameApp::instance()->getDrawLib()->endDraw();
    
    if(i_drifted) {
      GameApp::instance()->getDrawLib()->setTexture(pType->getTexture(),BLEND_MODE_B);
      GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
      GameApp::instance()->getDrawLib()->setColorRGBA(i_driftColor.Red(), i_driftColor.Green(), i_driftColor.Blue(), i_driftColor.Alpha());
      fDrift = GameApp::instance()->getXMTime() / 15.0f;
      GameApp::instance()->getDrawLib()->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset + fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset);
      GameApp::instance()->getDrawLib()->glVertexSP(0,0);
      GameApp::instance()->getDrawLib()->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset+uDriftZoom + fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset);
      GameApp::instance()->getDrawLib()->glVertexSP(GameApp::instance()->getDrawLib()->getDispWidth(),0);
      GameApp::instance()->getDrawLib()->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset+uDriftZoom + fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset+uDriftZoom);
      GameApp::instance()->getDrawLib()->glVertexSP(GameApp::instance()->getDrawLib()->getDispWidth(),GameApp::instance()->getDrawLib()->getDispHeight());
      GameApp::instance()->getDrawLib()->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset + fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset+uDriftZoom);
      GameApp::instance()->getDrawLib()->glVertexSP(0,GameApp::instance()->getDrawLib()->getDispHeight());
      GameApp::instance()->getDrawLib()->endDraw();
    }
  } else {
    Logger::Log(std::string("** Invalid sky " + i_scene->getLevelSrc()->Sky()->Texture()).c_str());
    GameApp::instance()->getDrawLib()->clearGraphics();
  } 
 }

  /*===========================================================================
  And background rendering
  ===========================================================================*/
  void GameRenderer::_RenderBackground(MotoGame* i_scene) { 
    /* Render STATIC background blocks */
    std::vector<Block *> Blocks = i_scene->getCollisionHandler()->getStaticBlocksNearPosition(m_screenBBox);

    /* sort blocks on their texture */
    std::sort(Blocks.begin(), Blocks.end(), AscendingTextureSort());

    for(unsigned int i=0;i<Blocks.size();i++) {
      if(Blocks[i]->isBackground() == true) {
	_RenderBlock(Blocks[i]);
      }
    }

    if(GameApp::instance()->getDrawLib()->getBackend() == DrawLib::backend_SdlGFX){
      /* Render all special edges (if quality != low) */
      if(XMSession::instance()->gameGraphics() != GFX_LOW) {
	for(unsigned int i=0;i<Blocks.size();i++) {
	  if(Blocks[i]->isBackground() == true) {
	    _RenderBlockEdges(Blocks[i]);
	  }
	}
      }
    }
  }

void GameRenderer::_RenderLayer(MotoGame* i_scene, int layer) {
    if(GameApp::instance()->getDrawLib()->getBackend() != DrawLib::backend_OpenGl) {
      return;
    }

    Vector2f layerOffset = i_scene->getLevelSrc()->getLayerOffset(layer);

    AABB bbox;
    // layers with almost the same x,y offsets than the main layers
    // are drawn with it. the others are drawn using default zoom
    if(layerOffset.x > 0.90 && layerOffset.x < 1.10
       && layerOffset.y > 0.90 && layerOffset.y < 1.10) {
      setCameraTransformations(i_scene->getCamera(), m_xScale, m_yScale);
      bbox = m_screenBBox;
    } else {
      setCameraTransformations(i_scene->getCamera(),
			       m_xScaleDefault, m_yScaleDefault);
      bbox = m_layersBBox;
    }

    /* get bounding box in the layer depending on its offset */
    Vector2f size = bbox.getBMax() - bbox.getBMin();

    Vector2f levelLeftTop = Vector2f(i_scene->getLevelSrc()->LeftLimit(),
				     i_scene->getLevelSrc()->TopLimit());

    Vector2f levelViewLeftTop = Vector2f(bbox.getBMin().x,
					 bbox.getBMin().y+size.y);

    Vector2f originalTranslateVector = levelViewLeftTop - levelLeftTop;
    Vector2f translationInLayer = originalTranslateVector * layerOffset;
    Vector2f translateVector = originalTranslateVector - translationInLayer;

    AABB layerBBox;
    layerBBox.addPointToAABB2f(levelLeftTop+translationInLayer);
    layerBBox.addPointToAABB2f(levelLeftTop.x + translationInLayer.x + size.x,
			       levelLeftTop.y + translationInLayer.y - size.y);

    std::vector<Block *> Blocks = i_scene->getCollisionHandler()->getBlocksNearPositionInLayer(layerBBox, layer);
    /* sort blocks on their texture */
    std::sort(Blocks.begin(), Blocks.end(), AscendingTextureSort());

#ifdef ENABLE_OPENGL
    glPushMatrix();
    glTranslatef(translateVector.x, translateVector.y, 0);

    for(unsigned int i=0; i<Blocks.size(); i++) {
      Block* block = Blocks[i];

      _RenderBlock(block);
    }
    glPopMatrix();
#endif
  }

void GameRenderer::_RenderLayers(MotoGame* i_scene, bool renderFront) { 
    /* Render background level blocks */
    int nbLayer = i_scene->getLevelSrc()->getNumberLayer();
    for(int layer=0; layer<nbLayer; layer++){
      if(i_scene->getLevelSrc()->isLayerFront(layer) == renderFront){
	_RenderLayer(i_scene, layer);
      }
    }
  }
  
  void GameRenderer::shutdown(void) {
    /* Free overlay */
    m_Overlay.cleanUp();
  }  

  /*===========================================================================
  Debug info. Note how this is leaked into the void and nobody cares :) 
  ===========================================================================*/
  void GameRenderer::_RenderDebugInfo(void) {
    for(unsigned int i=0;i<m_DebugInfo.size();i++) {
      if(m_DebugInfo[i]->Type == "@WHITEPOLYGONS") {
	GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_LOOP);
	GameApp::instance()->getDrawLib()->setColorRGB(255,255,255);
        for(unsigned int j=0;j<m_DebugInfo[i]->Args.size()/2;j++) {
          float x = atof(m_DebugInfo[i]->Args[j*2].c_str());
          float y = atof(m_DebugInfo[i]->Args[j*2+1].c_str());
          GameApp::instance()->getDrawLib()->glVertexSP(400 + x*10,300 - y*10);
        }
	GameApp::instance()->getDrawLib()->endDraw();
      }
      else if(m_DebugInfo[i]->Type == "@REDLINES") {
	GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
	GameApp::instance()->getDrawLib()->setColorRGB(255,0,0);
        for(unsigned int j=0;j<m_DebugInfo[i]->Args.size()/2;j++) {
          float x = atof(m_DebugInfo[i]->Args[j*2].c_str());
          float y = atof(m_DebugInfo[i]->Args[j*2+1].c_str());
          GameApp::instance()->getDrawLib()->glVertexSP(400 + x*10,300 - y*10);
        }
	GameApp::instance()->getDrawLib()->endDraw();
      }
      else if(m_DebugInfo[i]->Type == "@GREENLINES") {
	GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
	GameApp::instance()->getDrawLib()->setColorRGB(0,255,0);
        for(unsigned int j=0;j<m_DebugInfo[i]->Args.size()/2;j++) {
          float x = atof(m_DebugInfo[i]->Args[j*2].c_str());
          float y = atof(m_DebugInfo[i]->Args[j*2+1].c_str());
          GameApp::instance()->getDrawLib()->glVertexSP(400 + x*10,300 - y*10);
        }
	GameApp::instance()->getDrawLib()->endDraw();
      }
    }
  }
  
  void GameRenderer::loadDebugInfo(std::string File) {
    FileHandle *pfh = FS::openIFile(File);
    if(pfh != NULL) {
      std::string Line;
      std::string Type = "";
      while(FS::readNextLine(pfh,Line)) {
        if(!Line.empty() && Line[0] != '#') {
          if(Line[0] == '@') {
            Type = Line;
          }
          else {
            GraphDebugInfo *p = new GraphDebugInfo;
            m_DebugInfo.push_back(p);
            p->Type = Type;
            while(1) {
              int k = Line.find_first_of(" ");
              if(k<0) {
                p->Args.push_back(Line);
                break;
              }
              else {
                std::string s = Line.substr(0,k);
                Line = Line.substr(k+1,Line.length()-k);
                p->Args.push_back(s);
              }
            }
          }
        }
      }
      FS::closeFile(pfh);
    }
  }
  
  /*===========================================================================
  Replay stuff
  ===========================================================================*/
  void GameRenderer::showReplayHelp(float p_speed, bool bAllowRewind) {
    if(bAllowRewind) {
      if(p_speed >= 10.0) {
	m_replayHelp = GAMETEXT_REPLAYHELPTEXT(std::string(">> 10"));
      } else if(p_speed <= -10.0) {
	m_replayHelp = GAMETEXT_REPLAYHELPTEXT(std::string("<<-10"));
      } else {
	char v_speed_str[5 + 1];
	sprintf(v_speed_str, "% .2f", p_speed);
	m_replayHelp = GAMETEXT_REPLAYHELPTEXT(std::string(v_speed_str));
      }
    } else {
      if(p_speed >= 10.0) {
	m_replayHelp = GAMETEXT_REPLAYHELPTEXTNOREWIND(std::string(">> 10"));
      } else if(p_speed <= -10.0) {
	m_replayHelp = GAMETEXT_REPLAYHELPTEXTNOREWIND(std::string("<<-10"));
      } else {
	char v_speed_str[256];
	sprintf(v_speed_str, "% .2f", p_speed);
	m_replayHelp = GAMETEXT_REPLAYHELPTEXTNOREWIND(std::string(v_speed_str));
      }
    }
  }

  void GameRenderer::hideReplayHelp() {
    m_replayHelp = "";
  }
  
  /*===========================================================================
  In-game text rendering
  ===========================================================================*/
  /* 
                     |pi0
                     |pi1
                     |pi2
                     |pi3
       -------------------
       m0 m4 m8  m12 |po0
       m1 m5 m9  m13 |po1
       m2 m6 m10 m14 |po2
       m3 m7 m11 m15 |po3      
  */
  #define MULT_GL_MATRIX(pi,po,m) { \
    po[0] = m[0]*pi[0] + m[4]*pi[1] + m[8]*pi[2] + m[12]*pi[3]; \
    po[1] = m[1]*pi[0] + m[5]*pi[1] + m[9]*pi[2] + m[13]*pi[3]; \
    po[2] = m[2]*pi[0] + m[6]*pi[1] + m[10]*pi[2] + m[14]*pi[3]; \
    po[3] = m[3]*pi[0] + m[7]*pi[1] + m[11]*pi[2] + m[15]*pi[3]; \
  }    
  
  void GameRenderer::_RenderInGameText(Vector2f P,const std::string &Text,Color c) {
#ifdef ENABLE_OPENGL
    //keesj:todo i have no idea what is actualy going on here
    /* Perform manual transformation of world coordinates into screen
       coordinates */
    GLfloat fPoint[4],fTemp[4];
    GLfloat fModelView[16];
    GLfloat fProj[16];
    
    glGetFloatv(GL_MODELVIEW_MATRIX,fModelView);    
    glGetFloatv(GL_PROJECTION_MATRIX,fProj);
    
    fPoint[0] = P.x;
    fPoint[1] = P.y;
    fPoint[2] = 0.0f; /* no z please */
    fPoint[3] = 1.0f; /* homogenous 4-D coords */
    
    MULT_GL_MATRIX(fPoint,fTemp,fModelView);
    MULT_GL_MATRIX(fTemp,fPoint,fProj);
    float x = (fPoint[0] / fPoint[3] + 1.0f) / 2.0f;
    float y = 1.0f - (fPoint[1] / fPoint[3] + 1.0f) / 2.0f;
#else
    //keesj:TODO: I really have no clue what this function is about
    float x = (P.x / 1.0f + 1.0f) / 2.0f;
    float y = 1.0f - (P.y / 1.0f + 1.0f) / 2.0f;
#endif
    
    
    if(x > 0.0f && x < 1.0f && y > 0.0f && y < 1.0f) {    
      /* Map to viewport */
      float vx = ((float)GameApp::instance()->getDrawLib()->getDispWidth() * x);
      float vy = ((float)GameApp::instance()->getDrawLib()->getDispHeight() * y);
#ifdef ENABLE_OPENGL
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      glOrtho(0,GameApp::instance()->getDrawLib()->getDispWidth(),0,GameApp::instance()->getDrawLib()->getDispHeight(),-1,1);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
#endif
      
      FontManager* v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
      FontGlyph* v_fg = v_fm->getGlyph(Text);
      v_fm->printString(v_fg, (int)vx, (int)vy, c, true);

#ifdef ENABLE_OPENGL
      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
#endif
      
    }
  }

  /*===========================================================================
  Rendering helpers
  ===========================================================================*/
  void GameRenderer::_RenderAlphaBlendedSection(Texture *pTexture,
                                                const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3,
						const TColor&  i_filterColor) {
    GameApp::instance()->getDrawLib()->drawImage(p3, p2, p1, p0, pTexture,
						 MAKE_COLOR(i_filterColor.Red(), i_filterColor.Green(), i_filterColor.Blue(), 255));
  }
  
  void GameRenderer::_RenderAdditiveBlendedSection(Texture *pTexture,
                                                   const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3) {
    GameApp::instance()->getDrawLib()->drawImage(p0, p1, p2, p3, pTexture, MAKE_COLOR(255, 255, 255, 255), false, BLEND_MODE_B);
  }
  
  /* Screen-space version of the above */
  void GameRenderer::_RenderAlphaBlendedSectionSP(Texture *pTexture,
                                                  const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3) {
    GameApp::instance()->getDrawLib()->drawImage(p3, p2, p1, p0, pTexture,
						 MAKE_COLOR(255, 255, 255, 255), true);
  }
  
  void GameRenderer::_RenderRectangle(const Vector2f& i_p1, const Vector2f& i_p2, const Color& i_color) {
    GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
    GameApp::instance()->getDrawLib()->setColor(i_color);
    GameApp::instance()->getDrawLib()->glVertex(i_p1);
    GameApp::instance()->getDrawLib()->glVertex(Vector2f(i_p2.x, i_p1.y));
    GameApp::instance()->getDrawLib()->glVertex(i_p2);
    GameApp::instance()->getDrawLib()->glVertex(Vector2f(i_p1.x, i_p2.y));
    GameApp::instance()->getDrawLib()->glVertex(i_p1);
    GameApp::instance()->getDrawLib()->endDraw();
  }

  void GameRenderer::_RenderCircle(unsigned int nSteps,Color CircleColor,const Vector2f &C,float fRadius) {
    GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_LOOP);
    GameApp::instance()->getDrawLib()->setColor(CircleColor);
    for(unsigned int i=0;i<nSteps;i++) {
      float r = (3.14159f * 2.0f * (float)i)/ (float)nSteps;            
      GameApp::instance()->getDrawLib()->glVertex( Vector2f(C.x + fRadius*sinf(r),C.y + fRadius*cosf(r)) );
    }      
    GameApp::instance()->getDrawLib()->endDraw();
  }

  float GameRenderer::SizeMultOfEntitiesToTake() const {
    return m_sizeMultOfEntitiesToTake;
  }

  float GameRenderer::SizeMultOfEntitiesWhichMakeWin() const {
    return m_sizeMultOfEntitiesWhichMakeWin;
  }

  void GameRenderer::setSizeMultOfEntitiesToTake(float i_sizeMult) {
    m_sizeMultOfEntitiesToTake = i_sizeMult;
  }

  void GameRenderer::setSizeMultOfEntitiesWhichMakeWin(float i_sizeMult) {
    m_sizeMultOfEntitiesWhichMakeWin = i_sizeMult;
  }

  bool GameRenderer::showMinimap() const {
    return m_showMinimap;
  }

void GameRenderer::setShowTimePanel(bool i_value) {
  m_showTimePanel = i_value;
}

  bool GameRenderer::showEngineCounter() const {
    return m_showEngineCounter;
  }

  void GameRenderer::setShowMinimap(bool i_value) {
    m_showMinimap = i_value;
  }

  void GameRenderer::setShowEngineCounter(bool i_value) {
    m_showEngineCounter = i_value;
  }

  void GameRenderer::switchFollow(MotoGame* i_scene) {
    if(i_scene->getCamera()->getPlayerToFollow() == NULL) return;

    /* search into the player */
    for(unsigned i=0; i<i_scene->Players().size(); i++) {
      if(i_scene->Players()[i] == i_scene->getCamera()->getPlayerToFollow()) {
	if(i<i_scene->Players().size()-1) {
	  i_scene->getCamera()->setPlayerToFollow(i_scene->Players()[i+1]);
	} else {
	  if(i_scene->Ghosts().size() > 0) {
	    i_scene->getCamera()->setPlayerToFollow(i_scene->Ghosts()[0]);
	  } else {
	    i_scene->getCamera()->setPlayerToFollow(i_scene->Players()[0]);
	  }
	}
	return;
      }
    }

    /* search into the ghost */
    for(unsigned i=0; i<i_scene->Ghosts().size(); i++) {
      if(i_scene->Ghosts()[i] == i_scene->getCamera()->getPlayerToFollow()) {
	if(i<i_scene->Ghosts().size()-1) {
	  i_scene->getCamera()->setPlayerToFollow(i_scene->Ghosts()[i+1]);
	} else {
	  if(i_scene->Players().size() > 0) {
	    i_scene->getCamera()->setPlayerToFollow(i_scene->Players()[0]);
	  } else {
	    i_scene->getCamera()->setPlayerToFollow(i_scene->Ghosts()[0]);
	  }
	}
	return;
      }
    }
  }


void GameRenderer::renderTimePanel(MotoGame* i_scene) {
  int x = 0;
  int y = 0;
  FontGlyph* v_fg;

  // do not render it if it's the autozoom camera or ...
  if(i_scene->isAutoZoomCamera() == true){
    return;
  }

  unsigned int width  = GameApp::instance()->getDrawLib()->getDispWidth();
  unsigned int height = GameApp::instance()->getDrawLib()->getDispHeight();

  Biker* pBiker = i_scene->getCamera()->getPlayerToFollow();

  /* render game time */
  FontManager* v_fm = GameApp::instance()->getDrawLib()->getFontMedium();

  if(pBiker != NULL) {
    if(pBiker->isDead()) {
      v_fg = v_fm->getGlyph(formatTime(pBiker->deadTime()));
    } else {
      if(pBiker->isFinished()) {
	v_fg = v_fm->getGlyph(formatTime(pBiker->finishTime()));
      } else {
	v_fg = v_fm->getGlyph(formatTime(i_scene->getTime()));
      }
    }
  } else {
    v_fg = v_fm->getGlyph(formatTime(i_scene->getTime()));
  }

  switch(i_scene->getCurrentCamera()) {
  case 0:
    x=0; y=0;
    break;
  case 1:
    if(i_scene->getNumberCameras() == 2) {
      x=0; y=height/2;
    } else {
	x=width/2; y=0;
    }
    break;
  case 2:
    x=0; y=height/2;
    break;
  case 3:
    x=width/2; y=height/2;
    break;
  }

  v_fm->printString(v_fg,
		    x, y,
		    MAKE_COLOR(255,255,255,255), true);

  /* next things must be rendered only by the first camera */
  if(i_scene->getCurrentCamera() != 0)
    return;

  v_fm = GameApp::instance()->getDrawLib()->getFontSmall();

  v_fg = v_fm->getGlyph(m_bestTime);
  v_fm->printString(v_fg, x, y+28, MAKE_COLOR(255,255,255,255), true);

  v_fg = v_fm->getGlyph(m_worldRecordTime);
  v_fm->printString(v_fg, x, y+48, MAKE_COLOR(255,255,255,255), true);
}

void GameRenderer::renderReplayHelpMessage(MotoGame* i_scene) {
  /* next things must be rendered only the first camera */
  if(i_scene->getCurrentCamera() != 0)
    return;

  FontManager* v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
  FontGlyph* v_fg = v_fm->getGlyph(m_replayHelp);
  v_fm->printString(v_fg,
		    GameApp::instance()->getDrawLib()->getDispWidth() - v_fg->realWidth(),
		    0,
		    MAKE_COLOR(255,255,255,255), true);
}

  void GameRenderer::_RenderParticleDraw(Vector2f P,Texture *pTexture,float fSize,float fAngle, TColor c) {
    /* Render single particle */
    if(pTexture == NULL) return;

    m_nParticlesRendered++;

    Vector2f C = P;
    Vector2f p1,p2,p3,p4;
    p1 = Vector2f(1,0); p1.rotateXY(fAngle);
    p2 = Vector2f(1,0); p2.rotateXY(90+fAngle);
    p3 = Vector2f(1,0); p3.rotateXY(180+fAngle);
    p4 = Vector2f(1,0); p4.rotateXY(270+fAngle);
    
    p1 = C + p1 * fSize;
    p2 = C + p2 * fSize;
    p3 = C + p3 * fSize;
    p4 = C + p4 * fSize;


    GameApp::instance()->getDrawLib()->drawImageTextureSet(p1, p2, p3, p4, MAKE_COLOR(c.Red(), c.Green(), c.Blue(), c.Alpha()), false, true);
  }

void GameRenderer::_RenderParticle(MotoGame* i_scene, ParticlesSource *i_source) {

    if(i_source->SpriteName() == "Star") {

      AnimationSprite *pStarAnimation;
      pStarAnimation = (AnimationSprite*) Theme::instance()
      ->getSprite(SPRITE_TYPE_ANIMATION, i_scene->getLevelSrc()->SpriteForStar());

      if(pStarAnimation != NULL) {

	GameApp::instance()->getDrawLib()->setTexture(pStarAnimation->getTexture(),BLEND_MODE_A);	
	for(unsigned j = 0; j < i_source->Particles().size(); j++) {
	  _RenderParticleDraw(i_source->Particles()[j]->DynamicPosition(),
			      pStarAnimation->getTexture(),
			      pStarAnimation->getWidth(),
			      i_source->Particles()[j]->Angle(),
			      i_source->Particles()[j]->Color());
	}
	GameApp::instance()->getDrawLib()->removePropertiesAfterEnd();
      } else {
	DecorationSprite *pStarDecoration;

	/* search as a simple decoration, not nice, crappy crappy */
	pStarDecoration = (DecorationSprite*) Theme::instance()
	->getSprite(SPRITE_TYPE_DECORATION, i_scene->getLevelSrc()->SpriteForStar());
	if(pStarDecoration != NULL) {
	
	  GameApp::instance()->getDrawLib()->setTexture(pStarDecoration->getTexture(),BLEND_MODE_A);
	  for(unsigned j = 0; j < i_source->Particles().size(); j++) {
	    _RenderParticleDraw(i_source->Particles()[j]->DynamicPosition(),
				pStarDecoration->getTexture(),
				pStarDecoration->getWidth(),
				i_source->Particles()[j]->Angle(),
				i_source->Particles()[j]->Color());
	  }
	  GameApp::instance()->getDrawLib()->removePropertiesAfterEnd();
	}
      }
    } else if(i_source->SpriteName() == "Fire") {
      EffectSprite* pFireType = (EffectSprite*) Theme::instance()
      ->getSprite(SPRITE_TYPE_EFFECT, "Fire1");

      if(pFireType != NULL) {
	GameApp::instance()->getDrawLib()->setTexture(pFireType->getTexture(),BLEND_MODE_A);
	for(unsigned j = 0; j < i_source->Particles().size(); j++) {
	  _RenderParticleDraw(i_source->Particles()[j]->DynamicPosition(),
			      pFireType->getTexture(),
			      i_source->Particles()[j]->Size(),
			      i_source->Particles()[j]->Angle(),
			      i_source->Particles()[j]->Color());
	}
	GameApp::instance()->getDrawLib()->removePropertiesAfterEnd();
      }
    } else if(i_source->SpriteName() == "Smoke") {
      EffectSprite* pSmoke1Type = (EffectSprite*) Theme::instance()
      ->getSprite(SPRITE_TYPE_EFFECT, "Smoke1");
      EffectSprite* pSmoke2Type = (EffectSprite*) Theme::instance()
      ->getSprite(SPRITE_TYPE_EFFECT, "Smoke2");

      if(pSmoke1Type != NULL && pSmoke2Type != NULL) {
	if(i_source->Particles().size() > 0) {
	  if(i_source->Particles()[0]->SpriteName() == "Smoke1") {
	    GameApp::instance()->getDrawLib()->setTexture(pSmoke1Type->getTexture(),BLEND_MODE_A);
	  } else if(i_source->Particles()[0]->SpriteName() == "Smoke2") {
	    GameApp::instance()->getDrawLib()->setTexture(pSmoke1Type->getTexture(),BLEND_MODE_A);
	  }
	}
	for(unsigned j = 0; j < i_source->Particles().size(); j++) {
	  if(i_source->Particles()[j]->SpriteName() == "Smoke1") {
	    _RenderParticleDraw(i_source->Particles()[j]->DynamicPosition(),
				pSmoke1Type->getTexture(),
				i_source->Particles()[j]->Size(),
				i_source->Particles()[j]->Angle(),
				i_source->Particles()[j]->Color());
	  } else if(i_source->Particles()[j]->SpriteName() == "Smoke2") {
	    _RenderParticleDraw(i_source->Particles()[j]->DynamicPosition(),
				pSmoke2Type->getTexture(),
				i_source->Particles()[j]->Size(),
				i_source->Particles()[j]->Angle(),
				i_source->Particles()[j]->Color());
	  }
	}
	GameApp::instance()->getDrawLib()->removePropertiesAfterEnd();
      }
    } else if(i_source->SpriteName() == "Debris1") {
      EffectSprite* pDebrisType = (EffectSprite*) Theme::instance()
      ->getSprite(SPRITE_TYPE_EFFECT, "Debris1");

      if(pDebrisType != NULL) {
	GameApp::instance()->getDrawLib()->setTexture(pDebrisType->getTexture(),BLEND_MODE_A);
	for(unsigned j = 0; j < i_source->Particles().size(); j++) {
	  if(i_source->Particles()[j]->SpriteName() == "Debris1") {
	    _RenderParticleDraw(i_source->Particles()[j]->DynamicPosition(),
				pDebrisType->getTexture(),
				i_source->Particles()[j]->Size(),
				i_source->Particles()[j]->Angle(),
				i_source->Particles()[j]->Color());
	  }
	}
	GameApp::instance()->getDrawLib()->removePropertiesAfterEnd();
      }
    }
  }
  
void GameRenderer::_RenderParticles(MotoGame* i_scene, bool bFront) {
    AABB screenBigger;
    Vector2f screenMin = m_screenBBox.getBMin();
    Vector2f screenMax = m_screenBBox.getBMax();
    /* to avoid sprites being clipped out of the screen,
       we draw also the nearest one */
#define ENTITY_OFFSET 5.0f
    screenBigger.addPointToAABB2f(screenMin.x-ENTITY_OFFSET,
				  screenMin.y-ENTITY_OFFSET);
    screenBigger.addPointToAABB2f(screenMax.x+ENTITY_OFFSET,
				  screenMax.y+ENTITY_OFFSET);

    try {
      std::vector<Entity*> Entities = i_scene->getCollisionHandler()->getEntitiesNearPosition(screenBigger);
      for(unsigned int i = 0; i < Entities.size(); i++) {
	Entity* v_entity = Entities[i];
	if(v_entity->Speciality() == ET_PARTICLES_SOURCE) {
	  if((v_entity->Z() >= 0.0) == bFront) {
	    _RenderParticle(i_scene, (ParticlesSource*) v_entity);
	  }
	}
      }
      
      for(unsigned int i = 0; i < i_scene->getLevelSrc()->EntitiesExterns().size(); i++) {
	Entity* v_entity = i_scene->getLevelSrc()->EntitiesExterns()[i];
	if(v_entity->Speciality() == ET_PARTICLES_SOURCE) {
	  if((v_entity->Z() >= 0.0) == bFront) {
	    _RenderParticle(i_scene, (ParticlesSource*) v_entity);
	  }
	}
      }
    } catch(Exception &e) {
      i_scene->gameMessage("Unable to render particles", true);      
    }
}

  void GameRenderer::renderBodyPart(const Vector2f& i_from, const Vector2f& i_to,
				    float i_c11, float i_c12,
				    float i_c21, float i_c22,
				    float i_c31, float i_c32,
				    float i_c41, float i_c42,
				    Sprite *i_sprite,
				    const TColor& i_filterColor,
				    DriveDir i_direction,
				    int i_90_rotation
				    ) {
    Texture *pTexture;
    Vector2f Sv;
    Vector2f p0, p1, p2, p3;

    if(i_sprite == NULL) return;
    pTexture = i_sprite->getTexture(false, false, FM_LINEAR); // FM_LINEAR
    if(pTexture == NULL) return;

    Sv = i_from - i_to;
    Sv.normalize();

    if(i_direction == DD_RIGHT) {
      p0 = i_from + Vector2f(-Sv.y, Sv.x) * i_c11 + Sv * i_c12;
      p1 = i_to   + Vector2f(-Sv.y, Sv.x) * i_c21 + Sv * i_c22;
      p2 = i_to   - Vector2f(-Sv.y, Sv.x) * i_c31 + Sv * i_c32;
      p3 = i_from - Vector2f(-Sv.y, Sv.x) * i_c41 + Sv * i_c42;
    } else {
      p0 = i_from - Vector2f(-Sv.y, Sv.x) * i_c11 + Sv * i_c12;
      p1 = i_to   - Vector2f(-Sv.y, Sv.x) * i_c21 + Sv * i_c22;
      p2 = i_to   + Vector2f(-Sv.y, Sv.x) * i_c31 + Sv * i_c32;
      p3 = i_from + Vector2f(-Sv.y, Sv.x) * i_c41 + Sv * i_c42;
    }

    switch(i_90_rotation) {
      case 0:
      _RenderAlphaBlendedSection(pTexture, p1, p2, p3, p0, i_filterColor);
      break;
      case 1:
      _RenderAlphaBlendedSection(pTexture, p0, p1, p2, p3, i_filterColor);
      break;
      case 2:
      _RenderAlphaBlendedSection(pTexture, p3, p2, p1, p0, i_filterColor);
      break;
    }
  }

  /*===========================================================================
  Rendering of the bike
  ===========================================================================*/
  void GameRenderer::_RenderBike(BikeState *pBike, BikeParameters *pBikeParms, BikerTheme *p_theme, bool i_renderBikeFront,
				 const TColor&  i_filterColor, const TColor&  i_filterUglyColor) {
    Sprite *pSprite;
    Texture *pTexture;

    /* Render bike */
    Vector2f p0,p1,p2,p3,o0,o1,o2,o3;
    Vector2f C;
    Vector2f Sv,Rc,Fc;

    /* Draw front wheel */
    /* Ugly mode? */
    if(XMSession::instance()->ugly()) {
      o0 = Vector2f(-pBikeParms->WR,0);
      o1 = Vector2f(0,pBikeParms->WR);
      o2 = Vector2f(pBikeParms->WR,0);
      o3 = Vector2f(0,-pBikeParms->WR);
    }
    else {
      o0 = Vector2f(-pBikeParms->WR,pBikeParms->WR);
      o1 = Vector2f(pBikeParms->WR,pBikeParms->WR);
      o2 = Vector2f(pBikeParms->WR,-pBikeParms->WR);
      o3 = Vector2f(-pBikeParms->WR,-pBikeParms->WR);
    }
    p0 = Vector2f(o0.x*pBike->fFrontWheelRot[0] + o0.y*pBike->fFrontWheelRot[1],
                  o0.x*pBike->fFrontWheelRot[2] + o0.y*pBike->fFrontWheelRot[3]);
    p1 = Vector2f(o1.x*pBike->fFrontWheelRot[0] + o1.y*pBike->fFrontWheelRot[1],
                  o1.x*pBike->fFrontWheelRot[2] + o1.y*pBike->fFrontWheelRot[3]);
    p2 = Vector2f(o2.x*pBike->fFrontWheelRot[0] + o2.y*pBike->fFrontWheelRot[1],
                  o2.x*pBike->fFrontWheelRot[2] + o2.y*pBike->fFrontWheelRot[3]);
    p3 = Vector2f(o3.x*pBike->fFrontWheelRot[0] + o3.y*pBike->fFrontWheelRot[1],
                  o3.x*pBike->fFrontWheelRot[2] + o3.y*pBike->fFrontWheelRot[3]);
    
    C = pBike->FrontWheelP;
    Fc = (p0 + p1 + p2 + p3) * 0.25f + C;
    
    /* Ugly mode? */

    if(XMSession::instance()->ugly() == false) {
      pSprite = p_theme->getWheel();
      if(pSprite != NULL) {
	pTexture = pSprite->getTexture(false, false, FM_LINEAR);
	if(pTexture != NULL) {
	  _RenderAlphaBlendedSection(pTexture,p0+C,p1+C,p2+C,p3+C);
	}
      }
    }

    if(XMSession::instance()->ugly() || XMSession::instance()->testTheme()) {
      GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
      GameApp::instance()->getDrawLib()->setColor(p_theme->getUglyWheelColor());
      GameApp::instance()->getDrawLib()->glVertex(p0+C);    
      GameApp::instance()->getDrawLib()->glVertex(p2+C);
      GameApp::instance()->getDrawLib()->endDraw();
      GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
      GameApp::instance()->getDrawLib()->setColor(p_theme->getUglyWheelColor());
      GameApp::instance()->getDrawLib()->glVertex(p1+C);
      GameApp::instance()->getDrawLib()->glVertex(p3+C);
      GameApp::instance()->getDrawLib()->endDraw();
      _RenderCircle(16,p_theme->getUglyWheelColor(),C,pBikeParms->WR);
    }

    /* Draw rear wheel */        
    /* Ugly mode? */
    if(XMSession::instance()->ugly()) {
      o0 = Vector2f(-pBikeParms->WR,0);
      o1 = Vector2f(0,pBikeParms->WR);
      o2 = Vector2f(pBikeParms->WR,0);
      o3 = Vector2f(0,-pBikeParms->WR);
    }
    else {
      o0 = Vector2f(-pBikeParms->WR,pBikeParms->WR);
      o1 = Vector2f(pBikeParms->WR,pBikeParms->WR);
      o2 = Vector2f(pBikeParms->WR,-pBikeParms->WR);
      o3 = Vector2f(-pBikeParms->WR,-pBikeParms->WR);
    }
    p0 = Vector2f(o0.x*pBike->fRearWheelRot[0] + o0.y*pBike->fRearWheelRot[1],
                  o0.x*pBike->fRearWheelRot[2] + o0.y*pBike->fRearWheelRot[3]);
    p1 = Vector2f(o1.x*pBike->fRearWheelRot[0] + o1.y*pBike->fRearWheelRot[1],
                  o1.x*pBike->fRearWheelRot[2] + o1.y*pBike->fRearWheelRot[3]);
    p2 = Vector2f(o2.x*pBike->fRearWheelRot[0] + o2.y*pBike->fRearWheelRot[1],
                  o2.x*pBike->fRearWheelRot[2] + o2.y*pBike->fRearWheelRot[3]);
    p3 = Vector2f(o3.x*pBike->fRearWheelRot[0] + o3.y*pBike->fRearWheelRot[1],
                  o3.x*pBike->fRearWheelRot[2] + o3.y*pBike->fRearWheelRot[3]);
    
    C = pBike->RearWheelP;
    Rc = (p0 + p1 + p2 + p3) * 0.25f + C;
    
    /* Ugly mode? */
    if(XMSession::instance()->ugly() == false) {
      pSprite = p_theme->getWheel();
      if(pSprite != NULL) {
	pTexture = pSprite->getTexture(false, false, FM_LINEAR);
	if(pTexture != NULL) {
	  _RenderAlphaBlendedSection(pTexture,p0+C,p1+C,p2+C,p3+C);
	}
      }
    }

    if(XMSession::instance()->ugly() || XMSession::instance()->testTheme()) {
      GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
      GameApp::instance()->getDrawLib()->setColor(p_theme->getUglyWheelColor());
      GameApp::instance()->getDrawLib()->glVertex(p0+C);    
      GameApp::instance()->getDrawLib()->glVertex(p2+C);
      GameApp::instance()->getDrawLib()->endDraw();
      GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
      GameApp::instance()->getDrawLib()->setColor(p_theme->getUglyWheelColor());
      GameApp::instance()->getDrawLib()->glVertex(p1+C);
      GameApp::instance()->getDrawLib()->glVertex(p3+C);
      GameApp::instance()->getDrawLib()->endDraw();
      _RenderCircle(16,p_theme->getUglyWheelColor(),C,pBikeParms->WR);
    }

    if(!XMSession::instance()->ugly()) {
      /* Draw swing arm */
      if(pBike->Dir == DD_RIGHT) {       
        Sv = pBike->SwingAnchorP - Rc;
        Sv.normalize();
        p0 = pBike->RearWheelP + Vector2f(-Sv.y,Sv.x)*0.07f - Sv*0.08f;
        p1 = pBike->SwingAnchorP + Vector2f(-Sv.y,Sv.x)*0.07f;
        p2 = pBike->SwingAnchorP - Vector2f(-Sv.y,Sv.x)*0.07f;
        p3 = pBike->RearWheelP - Vector2f(-Sv.y,Sv.x)*0.07f - Sv*0.08f;
      }
      else {
        Sv = pBike->SwingAnchor2P - Fc;
        Sv.normalize();
        p0 = pBike->FrontWheelP + Vector2f(-Sv.y,Sv.x)*0.07f - Sv*0.08f;
        p1 = pBike->SwingAnchor2P + Vector2f(-Sv.y,Sv.x)*0.07f;
        p2 = pBike->SwingAnchor2P - Vector2f(-Sv.y,Sv.x)*0.07f;
        p3 = pBike->FrontWheelP - Vector2f(-Sv.y,Sv.x)*0.07f - Sv*0.08f;
      }        

      pSprite = p_theme->getRear();
      if(pSprite != NULL) {
	pTexture = pSprite->getTexture(false, false, FM_LINEAR);
	if(pTexture != NULL) {
	  _RenderAlphaBlendedSection(pTexture,p0,p1,p2,p3);
	}
      }

      /* Draw front suspension */
      if(pBike->Dir == DD_RIGHT) {
        Sv = pBike->FrontAnchorP - Fc;
        Sv.normalize();         
        p0 = pBike->FrontWheelP + Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.05f;
        p1 = pBike->FrontAnchorP + Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.35f;
        p2 = pBike->FrontAnchorP - Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.35f;
        p3 = pBike->FrontWheelP - Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.05f;
      }
      else {
        Sv = pBike->FrontAnchor2P - Rc;
        Sv.normalize();         
        p0 = pBike->RearWheelP + Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.05f;
        p1 = pBike->FrontAnchor2P + Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.35f;
        p2 = pBike->FrontAnchor2P - Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.35f;
        p3 = pBike->RearWheelP - Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.05f;
      }

      if(i_renderBikeFront) {
	pSprite = p_theme->getFront();
	if(pSprite != NULL) {
	  pTexture = pSprite->getTexture(false, false, FM_LINEAR);
	  if(pTexture != NULL) {
	    _RenderAlphaBlendedSection(pTexture,p3,p0,p1,p2);
	  }
	}    
      }  

      /* Draw body/frame */
      o0 = Vector2f(-1,0.5);
      o1 = Vector2f(1,0.5);
      o2 = Vector2f(1,-0.5);
      o3 = Vector2f(-1,-0.5);
      p0 = Vector2f(o0.x*pBike->fFrameRot[0] + o0.y*pBike->fFrameRot[1],
                    o0.x*pBike->fFrameRot[2] + o0.y*pBike->fFrameRot[3]);
      p1 = Vector2f(o1.x*pBike->fFrameRot[0] + o1.y*pBike->fFrameRot[1],
                    o1.x*pBike->fFrameRot[2] + o1.y*pBike->fFrameRot[3]);
      p2 = Vector2f(o2.x*pBike->fFrameRot[0] + o2.y*pBike->fFrameRot[1],
                    o2.x*pBike->fFrameRot[2] + o2.y*pBike->fFrameRot[3]);
      p3 = Vector2f(o3.x*pBike->fFrameRot[0] + o3.y*pBike->fFrameRot[1],
                    o3.x*pBike->fFrameRot[2] + o3.y*pBike->fFrameRot[3]);
      
      C = pBike->CenterP; 

      pSprite = p_theme->getBody();
      if(pSprite != NULL) {
	pTexture = pSprite->getTexture(false, false, FM_LINEAR);
	if(pTexture != NULL) {
	  if(pBike->Dir == DD_RIGHT) {
	    _RenderAlphaBlendedSection(pTexture,p3+C,p2+C,p1+C,p0+C, i_filterColor);
	  } else {
	    _RenderAlphaBlendedSection(pTexture,p2+C,p3+C,p0+C,p1+C, i_filterColor);
	  }
	}
      }
    }

      /* Draw rider */
    if(XMSession::instance()->ugly() == false) { 
      /* torso */
      renderBodyPart(pBike->Dir == DD_RIGHT ? pBike->ShoulderP  : pBike->Shoulder2P,
		     pBike->Dir == DD_RIGHT ? pBike->LowerBodyP : pBike->LowerBody2P,
		     0.24, 0.46,
		     0.24, -0.1,
		     0.24, -0.1,
		     0.24, 0.46,
		     p_theme->getTorso(),
		     i_filterColor,
		     pBike->Dir
		     );

      /* upper leg */
      renderBodyPart(pBike->Dir == DD_RIGHT ? pBike->LowerBodyP : pBike->LowerBody2P,
		     pBike->Dir == DD_RIGHT ? pBike->KneeP      : pBike->Knee2P,
		     0.20, 0.14,
		     0.15, 0.00,
		     0.15, 0.00,
		     0.10, 0.14,
		     p_theme->getUpperLeg(),
		     i_filterColor,
		     pBike->Dir, 1
		     );

      /* lower leg */
      renderBodyPart(pBike->Dir == DD_RIGHT ? pBike->KneeP : pBike->Knee2P,
		     pBike->Dir == DD_RIGHT ? pBike->FootP : pBike->Foot2P,
		     0.23, 0.01,
		     0.20, 0.00,
		     0.20, 0.00,
		     0.23, 0.10,
		     p_theme->getLowerLeg(),
		     i_filterColor,
		     pBike->Dir
		     );

      /* upper arm */
      renderBodyPart(pBike->Dir == DD_RIGHT ? pBike->ShoulderP : pBike->Shoulder2P,
		     pBike->Dir == DD_RIGHT ? pBike->ElbowP    : pBike->Elbow2P,
		     0.12, 0.09,
		     0.12, -0.05,
		     0.10, -0.05,
		     0.10, 0.09,
		     p_theme->getUpperArm(),
		     i_filterColor,
		     pBike->Dir
		     );

      /* lower arm */
      renderBodyPart(pBike->Dir == DD_RIGHT ? pBike->ElbowP : pBike->Elbow2P,
		     pBike->Dir == DD_RIGHT ? pBike->HandP  : pBike->Hand2P,
		     0.12, 0.09,
		     0.12, -0.05,
		     0.10, -0.05,
		     0.10, 0.09,
		     p_theme->getLowerArm(),
		     i_filterColor,
		     pBike->Dir, 2
		     );

    }

    if(pBike->Dir == DD_RIGHT) {
      if(XMSession::instance()->ugly() || XMSession::instance()->testTheme()) {
        /* Draw it ugly */
	GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
	GameApp::instance()->getDrawLib()->setColor(MAKE_COLOR(i_filterUglyColor.Red(),
						       i_filterUglyColor.Green(),
						       i_filterUglyColor.Blue(),
						       i_filterUglyColor.Alpha()));
        GameApp::instance()->getDrawLib()->glVertex(pBike->FootP);
        GameApp::instance()->getDrawLib()->glVertex(pBike->KneeP);
        GameApp::instance()->getDrawLib()->glVertex(pBike->LowerBodyP);
        GameApp::instance()->getDrawLib()->glVertex(pBike->ShoulderP);
        GameApp::instance()->getDrawLib()->glVertex(pBike->ElbowP);
        GameApp::instance()->getDrawLib()->glVertex(pBike->HandP);
	GameApp::instance()->getDrawLib()->endDraw();
        _RenderCircle(10, MAKE_COLOR(i_filterUglyColor.Red(),
				     i_filterUglyColor.Green(),
				     i_filterUglyColor.Blue(),
				     i_filterUglyColor.Alpha()),pBike->HeadP,pBikeParms->fHeadSize);
      }
    }
    else if(pBike->Dir == DD_LEFT) {
      if(XMSession::instance()->ugly() || XMSession::instance()->testTheme()) {
        /* Draw it ugly */
	GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
	GameApp::instance()->getDrawLib()->setColor(MAKE_COLOR(i_filterUglyColor.Red(),
						       i_filterUglyColor.Green(),
						       i_filterUglyColor.Blue(),
						       i_filterUglyColor.Alpha()));
        GameApp::instance()->getDrawLib()->glVertex(pBike->Foot2P);
        GameApp::instance()->getDrawLib()->glVertex(pBike->Knee2P);
        GameApp::instance()->getDrawLib()->glVertex(pBike->LowerBody2P);
        GameApp::instance()->getDrawLib()->glVertex(pBike->Shoulder2P);
        GameApp::instance()->getDrawLib()->glVertex(pBike->Elbow2P);
        GameApp::instance()->getDrawLib()->glVertex(pBike->Hand2P);
        GameApp::instance()->getDrawLib()->endDraw();
        _RenderCircle(10, MAKE_COLOR(i_filterUglyColor.Red(),
				     i_filterUglyColor.Green(),
				     i_filterUglyColor.Blue(),
				     i_filterUglyColor.Alpha()), pBike->Head2P,pBikeParms->fHeadSize);
      }
    }   

		// draw the center
    if(XMSession::instance()->debug()) {
      _RenderCircle(10, MAKE_COLOR(255, 0, 0, 255), pBike->CenterP, 0.2);
    }
  }

  void GameRenderer::_DrawRotatedMarker(Vector2f Pos,dReal *pfRot) {
    Vector2f p0,p1,p2,p3,o0,o1,o2,o3;
    Vector2f C = Pos;

    o0 = Vector2f(-0.1,0.1);
    o1 = Vector2f(0.1,0.1);
    o2 = Vector2f(0.1,-0.1);
    o3 = Vector2f(-0.1,-0.1);
    
    if(pfRot != NULL) {
      p0 = Vector2f(o0.x*pfRot[0*4+0] + o0.y*pfRot[0*4+1],
                    o0.x*pfRot[1*4+0] + o0.y*pfRot[1*4+1]);
      p1 = Vector2f(o1.x*pfRot[0*4+0] + o1.y*pfRot[0*4+1],
                    o1.x*pfRot[1*4+0] + o1.y*pfRot[1*4+1]);
      p2 = Vector2f(o2.x*pfRot[0*4+0] + o2.y*pfRot[0*4+1],
                    o2.x*pfRot[1*4+0] + o2.y*pfRot[1*4+1]);
      p3 = Vector2f(o3.x*pfRot[0*4+0] + o3.y*pfRot[0*4+1],
                    o3.x*pfRot[1*4+0] + o3.y*pfRot[1*4+1]);
    }
    else {
      p0 = o0;
      p1 = o1;
      p2 = o2;
      p3 = o3;
    }

    GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
    GameApp::instance()->getDrawLib()->setColorRGB(255,255,255);
    GameApp::instance()->getDrawLib()->glVertex(p0+C);    
    GameApp::instance()->getDrawLib()->glVertex(p2+C);
    GameApp::instance()->getDrawLib()->endDraw();
    GameApp::instance()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
    GameApp::instance()->getDrawLib()->setColorRGB(255,255,255);
    GameApp::instance()->getDrawLib()->glVertex(p1+C);    
    GameApp::instance()->getDrawLib()->glVertex(p3+C);
    GameApp::instance()->getDrawLib()->endDraw();
  }

void GameRenderer::setCameraTransformations(Camera* pCamera, float xScale, float yScale)
{
  /* Perform scaling/translation */    
  pCamera->setCamera3d();
  GameApp::instance()->getDrawLib()->setScale(xScale, yScale);
  if(pCamera->isMirrored() == true){
    GameApp::instance()->getDrawLib()->setMirrorY();
  }

  GameApp::instance()->getDrawLib()->setRotateZ(m_rotationAngleForTheFrame);
  GameApp::instance()->getDrawLib()->setTranslate(-pCamera->getCameraPositionX(), -pCamera->getCameraPositionY());
}

void GameRenderer::calculateCameraScaleAndScreenAABB(Camera* pCamera, AABB& bbox)
{
  bbox.reset();

  m_xScale = pCamera->getCurrentZoom() * ((float)pCamera->getDispHeight()) / pCamera->getDispWidth();
  m_yScale = pCamera->getCurrentZoom();

  // depends on zoom
  float xCamOffset = 1.0 / m_xScale;
  float yCamOffset = 1.0 / m_yScale;

  Vector2f v1(pCamera->getCameraPositionX()-xCamOffset, pCamera->getCameraPositionY()-yCamOffset);
  Vector2f v2(pCamera->getCameraPositionX()+xCamOffset, pCamera->getCameraPositionY()+yCamOffset);

  bbox.addPointToAABB2f(v1);
  bbox.addPointToAABB2f(v2);
}
