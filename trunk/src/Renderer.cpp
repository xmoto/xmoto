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
#include "xmscene/GhostTrail.h"
#include "helpers/Log.h"
#include "helpers/Text.h"
#include "helpers/Random.h"
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
#include <sstream>
#include "states/GameState.h"
#include "SysMessage.h"

#define ABS(x) ((x) > 0.0 ? (x) : -(x))
#define SIGNE(x) ((x) >= 0.0 ? 1.0 : -1.0)

#ifdef ENABLE_OPENGL
#include "drawlib/DrawLibOpenGL.h"
#endif

#define GHOST_INFO_DURATION  3.0
#define GHOST_INFO_FADE_TIME 0.5
#define GHOST_INFO_INSCREEN_MARGE 2.0

#define SUPRESS_LOGERROR true

//for ScreenShadowing
#define MENU_SHADING_TIME 0.3
#define MENU_SHADING_VALUE 150

//for ghost trail rendering
#define GT_RENDER_SCALE 0.2
#define GT_TRAIL_COLOR 255
//how ugly to draw depending on the camerazoom
#define GT_GFX_LOW_RATIO 0.35
#define GT_GFX_MED_RATIO 0.25
#define GT_GFX_HI_RATIO 0.10
//how much uglier in uglymode
#define GT_UGLY_MODE_MULTIPLYER 2

  /* to sort blocks on their texture */
  struct AscendingTextureSort {
    bool operator() (Block* b1, Block* b2) {
      return b1->getTexture() < b2->getTexture();
    }
  };

  /* to sort particle sources on their type */
  struct AscendingParticleSourceSort {
    bool operator() (ParticlesSource* p1, ParticlesSource* p2) {
      return p1->getType() < p2->getType();
    }
  };

  /* to sort entities on their sprite */
  struct AscendingEntitySort {
    bool operator() (Entity* e1, Entity* e2) {
      Sprite* pSprite1 = e1->getSprite();
      Sprite* pSprite2 = e2->getSprite();
      return (pSprite1 ? pSprite1->getOrder() : 0) < (pSprite2 ? pSprite2->getOrder() : 0);
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
  m_currentEdgeBlendColor = DEFAULT_EDGE_BLENDCOLOR;
  m_currentEdgeSprite = NULL;
  m_curRegistrationStage = 0;
  m_showGhostsText = true;
  m_graphicsLevel = GFX_HIGH;
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
    m_arrowSprite = NULL;
  }

  /*===========================================================================
  Called to prepare renderer for new level
  ===========================================================================*/
void GameRenderer::prepareForNewLevel(Universe* i_universe) {
  // level of the first world
  Level* v_level;
  int n_sameSceneAs;

  // set the graphical level on time by level in case it changes while playing
  m_graphicsLevel = XMSession::instance()->gameGraphics();

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

  m_sizeMultOfEntitiesToTake       = 1.0;
  m_sizeMultOfEntitiesWhichMakeWin = 1.0;

  beginTexturesRegistration();

  /* Optimize scene */
  for(unsigned int u=0; u<i_universe->getScenes().size(); u++) {
    v_level = i_universe->getScenes()[u]->getLevelSrc();

    LogInfo("Loading level %s", v_level->Name().c_str());

    n_sameSceneAs = -1;
    // set to the universe which has the same level to init geoms
    // only 1 time if the level is loaded several times
    for(unsigned int v=0; v<u; v++) {
      if(i_universe->getScenes()[u]->getLevelSrc()->Id() == i_universe->getScenes()[v]->getLevelSrc()->Id()) {
	n_sameSceneAs = v;
	break;
      }
    }

    std::vector<Block*>& Blocks = v_level->Blocks();
    int nVertexBytes = 0;

    bool v_loadLayers = m_graphicsLevel == GFX_HIGH;
    bool v_loadBackgroundBlocks = m_graphicsLevel != GFX_LOW;

    for(unsigned int i=0; i<Blocks.size(); i++) {
      /* do not load into the graphic card blocks which won't be
	 displayed. On ati card with free driver, levels like green
	 hill zone act 2 doesn't work if there's too much vertex loaded */
      if(v_loadLayers == false && Blocks[i]->getLayer() != -1)
	continue;
      if(v_loadBackgroundBlocks == false && Blocks[i]->isBackground() == true)
	continue;

      nVertexBytes += loadBlock(Blocks[i], i_universe, u, n_sameSceneAs, i);
    }

    LogInfo("GL: %d kB vertex buffers", nVertexBytes/1024);

    // load sprites textures
    std::vector<Entity*>& entities = v_level->Entities();
    std::vector<Entity*>::const_iterator it = entities.begin();

    while(it != entities.end()){
      // loadSpriteTextures is implement in the base class Entity
      // so we can't access to child class re-implementation
      // with an Entity pointer
      if((*it)->Speciality() == ET_PARTICLES_SOURCE
	 && ((ParticlesSource*)(*it))->getType() == Smoke)
	((ParticlesSourceSmoke*)(*it))->loadSpriteTextures();
      else
	(*it)->loadSpriteTextures();

      ++it;
    }

    // sprites remplacement stored in level and the sky
    TextureSprite* skySprite = (TextureSprite*) Theme::instance()->getSprite(SPRITE_TYPE_TEXTURE, v_level->Sky()->Texture());
    if(skySprite != NULL)
      skySprite->loadTextures();
    else{
      LogDebug("skySprite is NULL [%s]", v_level->Sky()->Texture().c_str());
    }
    TextureSprite* skySprite2 =(TextureSprite*) Theme::instance()->getSprite(SPRITE_TYPE_TEXTURE, v_level->Sky()->BlendTexture());
    if(skySprite2 != NULL)
      skySprite2->loadTextures();
    else{
      LogDebug("skySprite2 is NULL [%s]", v_level->Sky()->BlendTexture().c_str());
    }    

    AnimationSprite* v_sprite = NULL;
    v_sprite = (AnimationSprite*)v_level->wreckerSprite();
    if(v_sprite != NULL)
      v_sprite->loadTextures();
    v_sprite = (AnimationSprite*)v_level->flowerSprite();
    if(v_sprite != NULL)
      v_sprite->loadTextures();
    v_sprite = (AnimationSprite*)v_level->strawberrySprite();
    if(v_sprite != NULL)
      v_sprite->loadTextures();
    v_sprite = (AnimationSprite*)v_level->starSprite();
    if(v_sprite != NULL)
      v_sprite->loadTextures();

    // debris sprite
    EffectSprite* pDebrisType;
    pDebrisType = (EffectSprite*) Theme::instance()->getSprite(SPRITE_TYPE_EFFECT, "Debris1");
    if(pDebrisType != NULL)
      pDebrisType->loadTextures();

    // and now the arrow sprite stored in the renderer
    m_arrowSprite = (MiscSprite*)Theme::instance()->getSprite(SPRITE_TYPE_MISC, "Arrow");
    m_arrowSprite->loadTextures();
  }
  
  endTexturesRegistration();
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
      i_universe->getScenes()[j]->getCamera()->initTrailCam(i_universe->getScenes()[j]);
    }
  }
}

int GameRenderer::loadBlock(Block* pBlock,
			    Universe* i_universe,
			    unsigned int currentScene,
			    int sameSceneAs,
			    int blockIndex)
{
  Scene* pScene  = i_universe->getScenes()[currentScene];
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

  Texture* pTexture = loadTexture(pBlock->getTexture());
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
				Scene* pScene)
{
  std::vector<ConvexBlock *> ConvexBlocks = pBlock->ConvexBlocks();
  int nVertexBytes  = 0;
  Geom* pSuitableGeom = new Geom;
  pSuitableGeom->pTexture = pTexture;
;
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

int GameRenderer::loadBlockEdge(Block* pBlock, Vector2f Center, Scene* pScene)
{
  int nVertexBytes  = 0;
  if(m_graphicsLevel != GFX_LOW){
    m_currentEdgeEffect = "";
    m_currentEdgeBlendColor = DEFAULT_EDGE_BLENDCOLOR;
    m_currentEdgeSprite = NULL;
    m_currentEdgeMaterialScale = DEFAULT_EDGE_SCALE;
    m_currentEdgeMaterialDepth = DEFAULT_EDGE_DEPTH;

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
	std::string v_edgeMaterialTextureName = pBlock->getEdgeMaterialTexture(firstVertex->EdgeEffect());
	if(v_edgeMaterialTextureName == "") {
	  pType = (EdgeEffectSprite*)Theme::instance()->getSprite(SPRITE_TYPE_EDGEEFFECT, firstVertex->EdgeEffect());
	}
	else {   // edge material defined then
	   pType = (EdgeEffectSprite*)Theme::instance()->getSprite(SPRITE_TYPE_EDGEEFFECT, v_edgeMaterialTextureName);
        }
         
	if(pType == NULL) {
	  LogWarning("Invalid edge effect %s", firstVertex->EdgeEffect().c_str());
	  useOld = false;
	}
	else{
	  Vector2f a1, b1, b2, a2, c1, c2;
	  m_currentEdgeSprite = pType;
	  m_currentEdgeEffect = firstVertex->EdgeEffect();
	  calculateEdgePosition(pBlock,
				lastVertex, firstVertex, secondVertex,
				Center,
				a1, b1, b2, a2, c1, c2,
				oldC2, oldB2, useOld, false, swapDone);
	  oldC2 = c2;
	  oldB2 = b2;
	  useOld = true;
	  m_currentEdgeBlendColor = pBlock->getEdgeMaterialColor(firstVertex->EdgeEffect());
	  m_currentEdgeMaterialScale = pBlock->getEdgeMaterialScale(firstVertex->EdgeEffect());
	  m_currentEdgeMaterialDepth = pBlock->getEdgeMaterialDepth(firstVertex->EdgeEffect());
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


      //check if edge texture is in material or pure
      std::string v_edgeMaterialTextureName = pBlock->getEdgeMaterialTexture(edgeEffect);;
      Texture* pTexture = NULL;
      if(v_edgeMaterialTextureName == ""){  // the edge effect is then probably pure oldschool. lets load it then
        pTexture = loadTextureEdge(edgeEffect);
      }
      else {  // we seem to have a material defined.
        pTexture = loadTextureEdge(v_edgeMaterialTextureName);    
      }
      if(pTexture == NULL) {
	  pScene->gameMessage(GAMETEXT_MISSINGTEXTURES, true);
	  useOld = false;
	  continue;
	}
     
      if(edgeEffect != m_currentEdgeEffect) {   //if a new edge effect texture occurs, load it
        EdgeEffectSprite* pType = NULL;
        if(v_edgeMaterialTextureName == "") {  //no material defined then
	  pType = (EdgeEffectSprite*)Theme::instance()->getSprite(SPRITE_TYPE_EDGEEFFECT, edgeEffect);
	  if(pType == NULL) {
	    LogWarning("Invalid edge effect %s", edgeEffect.c_str());
	    useOld = false;
	    continue;
	  }
  	  m_currentEdgeBlendColor = DEFAULT_EDGE_BLENDCOLOR;
	  m_currentEdgeMaterialScale = DEFAULT_EDGE_SCALE; 
          m_currentEdgeMaterialDepth = DEFAULT_EDGE_DEPTH; 
	}
	else {  // seems we ve got a material!
	  pType = (EdgeEffectSprite*)Theme::instance()->getSprite(SPRITE_TYPE_EDGEEFFECT, v_edgeMaterialTextureName);
	  if(pType == NULL) {
	    LogWarning("Invalid edge material %s", edgeEffect.c_str());
	    useOld = false;
	    continue;
	  }
	  m_currentEdgeBlendColor = pBlock->getEdgeMaterialColor(edgeEffect);
	  m_currentEdgeMaterialScale = pBlock->getEdgeMaterialScale(edgeEffect);
          m_currentEdgeMaterialDepth = pBlock->getEdgeMaterialDepth(edgeEffect);
        }
        
        m_currentEdgeSprite = pType;
  	m_currentEdgeEffect = edgeEffect;
      }
      
      // if a geom for current edge effect exists, get its index number
      int geomIndex = edgeGeomExists(pBlock, edgeEffect); 
      if(geomIndex < 0){
	// create a new one
	Geom* pGeom = new Geom;
	geomIndex = m_edgeGeoms.size(); 
	
	m_edgeGeoms.push_back(pGeom);
	pBlock->addEdgeGeom(geomIndex);
	pGeom->pTexture = pTexture;
	pGeom->material = edgeEffect;
	GeomPoly *pPoly = new GeomPoly;
	pGeom->Polys.push_back(pPoly);
      }
      Geom*     pGeom = m_edgeGeoms[geomIndex];

      GeomPoly* pPoly = pGeom->Polys[0];

      pGeom->edgeBlendColor = m_currentEdgeBlendColor;

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
  /*get scale value from inksmoto material, if value is set there, if not (-1), get value from theme */
  float fDepth = m_currentEdgeMaterialDepth; 
  if(fDepth == DEFAULT_EDGE_DEPTH) fDepth = m_currentEdgeSprite->getDepth();
  
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
  float fXScale = m_currentEdgeMaterialScale; //look if theres a inksmoto material scale defined
  if(fXScale == DEFAULT_EDGE_SCALE) fXScale = m_currentEdgeSprite->getScale();

  switch(pBlock->getEdgeDrawMethod()){
  case Block::inside:
  case Block::outside:{
    Vector2f N1(-B1.y+A1.y, B1.x-A1.x);

    if(N1.x == 0.0 && N1.y == 0.0){
      LogWarning("normal is null for block %s vertex (%f,%f)", pBlock->Id().c_str(), A1.x, A1.y);
    }

    N1.normalize();

    ua1.x = A1.x*fXScale*N1.y - A1.y*fXScale*N1.x;
    ub1.x = B1.x*fXScale*N1.y - B1.y*fXScale*N1.x;

    Vector2f N2(-B2.y+A2.y, B2.x-A2.x);

    if(N2.x == 0.0 && N2.y == 0.0){
      LogWarning("normal is null for block %s vertex (%f,%f)", pBlock->Id().c_str(), A2.x, A2.y);
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
      LogWarning("Texture '%s' not found!", textureName.c_str());
    }
  } else {
    LogWarning("Texture '%s' not found!", textureName.c_str());
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
        LogWarning("Edge Texture '%s' not found!", textureName.c_str());
    }
  } else {
      LogWarning("Edge Texture '%s' not found!", textureName.c_str());
  }

  return pTexture;
}

int GameRenderer::edgeGeomExists(Block* pBlock, std::string material)
{
  std::vector<int>& edgeGeoms = pBlock->getEdgeGeoms();

  for(unsigned int i=0; i<edgeGeoms.size(); i++){
    if(m_edgeGeoms[edgeGeoms[i]]->material == material)
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
    DrawLib* pDrawlib = GameApp::instance()->getDrawLib();

    p0 = Vector2f(x,        pDrawlib->getDispHeight()-y-nHeight);
    p1 = Vector2f(x+nWidth, pDrawlib->getDispHeight()-y-nHeight);
    p2 = Vector2f(x+nWidth, pDrawlib->getDispHeight()-y);
    p3 = Vector2f(x,        pDrawlib->getDispHeight()-y);

    pSprite = (MiscSprite*) Theme::instance()->getSprite(SPRITE_TYPE_MISC, "EngineCounter");
    if(pSprite != NULL) {
      pTexture = pSprite->getTexture();
      if(pTexture != NULL) {
	_RenderAlphaBlendedSection(pTexture, p0, p1, p2, p3);

	pDrawlib->setTexture(NULL, BLEND_MODE_NONE);

	pDrawlib->setColorRGB(255,50,50);
	renderEngineCounterNeedle(nWidth, nHeight, p3, pSpeed_eff);
	pDrawlib->setColorRGB(50,50,255);
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

    DrawLib* pDrawlib = GameApp::instance()->getDrawLib();
    pDrawlib->startDraw(DRAW_MODE_POLYGON);
    pDrawlib->glVertex(pdest);
    pDrawlib->glVertex(pcenterl);
    pDrawlib->glVertex(pbottom);
    pDrawlib->glVertex(pcenterr);
    pDrawlib->endDraw();
  }

  /*========================================================
   * Calculate the new position of a point according to the
   * animation when changing the direction
   * ========================================================*/
  Vector2f GameRenderer::calculateChangeDirPosition(Biker* i_biker, const Vector2f i_p){
      BikeState* pBike = i_biker->getState();
      Vector2f C = i_biker->getState()->CenterP;
      Vector2f s1, s2, p;

      p = i_p - C;

      s1 = Vector2f(0.0,1.0);
      s1 = Vector2f( s1.x*pBike->fFrameRot[0] + s1.y*pBike->fFrameRot[1], 
                     s1.x*pBike->fFrameRot[2] + s1.y*pBike->fFrameRot[3]); 
      s1.normalize();
      
      s2 = p - s1 * (p.x*s1.x + p.y*s1.y);
      p = p + s2 * (2.0 * (i_biker->changeDirPer() - 1.0)) + C;

      return p;
  }

  
  /*===========================================================================
  Minimap rendering
  ===========================================================================*/
#define MINIMAPZOOM 5.0f
  #define MINIMAPALPHA 128
  #define MINIMAPALPHA_BACK 212
  #define MINIVERTEX(Px,Py) \
    pDrawlib->glVertexSP(x + nWidth/2  + (float)(Px - cameraPosX)*MINIMAPZOOM, \
                         y + nHeight/2 - (float)(Py - cameraPosY)*MINIMAPZOOM);    

void GameRenderer::renderMiniMap(Scene* i_scene, int x,int y,int nWidth,int nHeight) {
    DrawLib* pDrawlib = GameApp::instance()->getDrawLib();
    Camera*  pCamera  = i_scene->getCamera();
    Biker*   pBiker   = pCamera->getPlayerToFollow();

    // do not render it if it's the autozoom camera (in multi), or the player is dead (in multi), or no player is followed
    if(i_scene->isAutoZoomCamera() == true
       || (pBiker == NULL || (pBiker != NULL && pBiker->isDead() == true))){
      return;
    }

    float cameraPosX = pCamera->getCameraPositionX();
    float cameraPosY = pCamera->getCameraPositionY();

    pDrawlib->drawBox(Vector2f(x,y),Vector2f(x+nWidth,y+nHeight),1,
				       MAKE_COLOR(0,0,0,MINIMAPALPHA_BACK),
				       MAKE_COLOR(255,255,255,MINIMAPALPHA));
    // the scissor zone is in the screen coordinates
    Vector2i bottomLeft = pCamera->getDispBottomLeft();

    unsigned int y_translate = bottomLeft.y/2;
    if((unsigned int)bottomLeft.y != pDrawlib->getDispHeight() || XMSession::instance()->multiNbPlayers() == 1){
      y_translate = 0;
    }
    pDrawlib->setClipRect(bottomLeft.x + x+1,
			 y+1 - y_translate,
			 nWidth-2,nHeight-2);

#ifdef ENABLE_OPENGL
    glEnable(GL_SCISSOR_TEST);
    glLoadIdentity();
#endif

    if(pCamera->isMirrored() == true){
      pDrawlib->setMirrorY();
    }

    /* get minimap AABB in level space
      input:  position on the screen (in the minimap area)
      output: position in the level
    */
#define MAP_TO_LEVEL_X(mapX) ((mapX) - x - nWidth/2)/MINIMAPZOOM  + cameraPosX
#define MAP_TO_LEVEL_Y(mapY) ((mapY) - y - nHeight/2)/MINIMAPZOOM + cameraPosY
    AABB mapBBox;
    mapBBox.addPointToAABB2f(MAP_TO_LEVEL_X(x), MAP_TO_LEVEL_Y(y));
    mapBBox.addPointToAABB2f(MAP_TO_LEVEL_X(x+nWidth), MAP_TO_LEVEL_Y(y+nHeight));

    if(pCamera->isMirrored() == true){
      // nice value coming out from my hat
      cameraPosX += 30.0;
    }

    /* TOFIX::Draw the static blocks only once in a texture, and reuse it after */
    /* Render blocks */
    std::vector<Block*> Blocks;

    pDrawlib->setTexture(NULL, BLEND_MODE_NONE);

    for(int layer=-1; layer<=0; layer++){
      Blocks = i_scene->getCollisionHandler()->getStaticBlocksNearPosition(mapBBox, layer);
      for(unsigned int i=0; i<Blocks.size(); i++) {

	/* Don't draw background blocks neither dynamic ones */
	if(Blocks[i]->isBackground() == false && Blocks[i]->getLayer() == -1) {
	  std::vector<ConvexBlock *> ConvexBlocks = Blocks[i]->ConvexBlocks();
	  for(unsigned int j=0; j<ConvexBlocks.size(); j++) {
	    Vector2f Center = ConvexBlocks[j]->SourceBlock()->DynamicPosition(); 	 

	    pDrawlib->startDraw(DRAW_MODE_POLYGON); 	 
	    pDrawlib->setColorRGB(168,168,168);
	    /* TOFIX::what's THAT ??!? -->> put all the vertices in a vector and draw them in one opengl call ! */
	    for(unsigned int k=0; k<ConvexBlocks[j]->Vertices().size(); k++) { 	 
	      Vector2f P = Center + ConvexBlocks[j]->Vertices()[k]->Position(); 	 
	      MINIVERTEX(P.x,P.y);
	    } 	 
	    pDrawlib->endDraw();
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
	  pDrawlib->startDraw(DRAW_MODE_POLYGON);
	  pDrawlib->setColorRGB(168,168,168);
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
	  
	  pDrawlib->endDraw();
	}
      }
    }


    /*
      input: position in the level
      output: position on the screen (draw in the minimap area)
    */
#define LEVEL_TO_SCREEN_X(elemPosX) (x + nWidth/2  + (float)((elemPosX) - cameraPosX) * MINIMAPZOOM)
#define LEVEL_TO_SCREEN_Y(elemPosY) (y + nHeight/2 - (float)((elemPosY) - cameraPosY) * MINIMAPZOOM)

    for(unsigned int i=0; i<i_scene->Players().size(); i++) {
      Vector2f bikePos(LEVEL_TO_SCREEN_X(i_scene->Players()[i]->getState()->CenterP.x),
		       LEVEL_TO_SCREEN_Y(i_scene->Players()[i]->getState()->CenterP.y));
      pDrawlib->drawCircle(bikePos, 3, 0, MAKE_COLOR(255,238,104,255), 0);
    }
    
    /* Render ghost position too? */
    for(unsigned int i=0; i<i_scene->Ghosts().size(); i++) {
      Ghost* v_ghost = i_scene->Ghosts()[i];

      Vector2f ghostPos(LEVEL_TO_SCREEN_X(v_ghost->getState()->CenterP.x),
			LEVEL_TO_SCREEN_Y(v_ghost->getState()->CenterP.y));
      pDrawlib->drawCircle(ghostPos, 3, 0, MAKE_COLOR(96,96,150,255), 0);
    }

    /* FIX::display only visible entities */
    std::vector<Entity*> Entities = i_scene->getCollisionHandler()->getEntitiesNearPosition(mapBBox);

    for(unsigned int i=0;i<Entities.size();i++) {
      Vector2f entityPos(LEVEL_TO_SCREEN_X(Entities[i]->DynamicPosition().x),
			 LEVEL_TO_SCREEN_Y(Entities[i]->DynamicPosition().y));
      if(Entities[i]->DoesMakeWin()) {
        pDrawlib->drawCircle(entityPos, 3, 0, MAKE_COLOR(255,0,255,255), 0);
      }
      else if(Entities[i]->IsToTake()) {
        pDrawlib->drawCircle(entityPos, 3, 0, MAKE_COLOR(255,0,0,255), 0);
      }
      else if(Entities[i]->DoesKill()) {
        pDrawlib->drawCircle(entityPos, 3, 0, MAKE_COLOR(26,26,188,255), 0);
      }
      else if(Entities[i]->isScripted()) {
        pDrawlib->drawCircle(entityPos, 3, 0, MAKE_COLOR(230,226,100,255), 0);
      }
    }
    
#ifdef ENABLE_OPENGL
    glDisable(GL_SCISSOR_TEST);
#endif
    //keesj:todo replace with setClipRect(NULL) in drawlib
    pDrawlib->setClipRect(0,0,pDrawlib->getDispWidth(),pDrawlib->getDispHeight());
  }

void GameRenderer::_RenderGhost(Scene* i_scene, Biker* i_ghost, int i, float i_textOffset) {
  float v_diffInfoTextTime;
  int v_textTrans;

  if(m_screenBBox.getBMin().x+GHOST_INFO_INSCREEN_MARGE < i_ghost->getState()->CenterP.x &&
     m_screenBBox.getBMax().x-GHOST_INFO_INSCREEN_MARGE > i_ghost->getState()->CenterP.x &&
     m_screenBBox.getBMin().y+GHOST_INFO_INSCREEN_MARGE < i_ghost->getState()->CenterP.y &&
     m_screenBBox.getBMax().y-GHOST_INFO_INSCREEN_MARGE > i_ghost->getState()->CenterP.y) {
    i_scene->getCamera()->setGhostIn(i);
  } else if(m_screenBBox.getBMin().x-GHOST_INFO_INSCREEN_MARGE > i_ghost->getState()->CenterP.x ||
	    m_screenBBox.getBMax().x+GHOST_INFO_INSCREEN_MARGE < i_ghost->getState()->CenterP.x ||
            m_screenBBox.getBMin().y-GHOST_INFO_INSCREEN_MARGE > i_ghost->getState()->CenterP.y ||
            m_screenBBox.getBMax().y+GHOST_INFO_INSCREEN_MARGE < i_ghost->getState()->CenterP.y) {
    i_scene->getCamera()->setGhostOut(i);
  }

  /* Render ghost - ugly mode? */
  if(XMSession::instance()->ugly() == false) {
    if(XMSession::instance()->hideGhosts() == false) { /* ghosts can be hidden, but don't hide text */
      /* No not ugly, fancy! Render into overlay? */      
      if(XMSession::instance()->ghostMotionBlur() && i_ghost->getBikeTheme()->getGhostEffect()
	 && m_allowGhostEffect) {
	m_Overlay.beginRendering();
	m_Overlay.fade(0.15);
      }

      try {
	_RenderBike(i_ghost, true,
		    i_ghost->getColorFilter(), i_ghost->getUglyColorFilter());
      } catch(Exception &e) {
	i_scene->gameMessage("Unable to render the ghost", true, 50);
      }

      if(XMSession::instance()->ghostMotionBlur() && i_ghost->getBikeTheme()->getGhostEffect()
	 && m_allowGhostEffect) {
	m_Overlay.endRendering();
	m_Overlay.present();
      }
    }
 
    /* ghost description */
    if(i_ghost->getDescription() != "" && showGhostsText()) {
      if(XMSession::instance()->showGhostsInfos()) {
	if(i_scene->getCamera()->isGhostIn(i)) {
	  v_diffInfoTextTime = GameApp::getXMTime() - i_scene->getCamera()->getGhostLastIn(i);

	  if(v_diffInfoTextTime < GHOST_INFO_FADE_TIME + GHOST_INFO_DURATION + GHOST_INFO_FADE_TIME) {

	    if(v_diffInfoTextTime < GHOST_INFO_FADE_TIME) {
	      v_textTrans = (int)(((v_diffInfoTextTime-GHOST_INFO_FADE_TIME)*255) / GHOST_INFO_FADE_TIME);
	    }
	    else if(v_diffInfoTextTime >= GHOST_INFO_FADE_TIME &&
		    v_diffInfoTextTime < GHOST_INFO_FADE_TIME + GHOST_INFO_DURATION) {
	      v_textTrans = 255;
	    } else {
	      v_textTrans = 255 - ((int)(((v_diffInfoTextTime-GHOST_INFO_FADE_TIME-GHOST_INFO_DURATION)*255) / GHOST_INFO_FADE_TIME));
	    }
	    
	    _RenderInGameText(i_ghost->getState()->CenterP + Vector2f(i_textOffset, -1.0f),
			      i_ghost->getDescription(),
			      MAKE_COLOR(255,255,255, v_textTrans), 0.5);
	  }
	}
      }
    }

    /* ghost arrow indication */
    if(XMSession::instance()->showBikersArrows()) {
      displayArrowIndication(i_ghost, &m_screenBBox);
    }
  }
    
  if(XMSession::instance()->ugly()) {
    if(XMSession::instance()->hideGhosts() == false) { /* ghosts can be hidden, but don't hide text */
      _RenderBike(i_ghost,
		  true,
		  i_ghost->getColorFilter(), i_ghost->getUglyColorFilter());
    }
  }
}

void GameRenderer::_RenderGhostTrail(Scene* i_scene, AABB* i_screenBBox, float i_scale) {
  //get trail data
  GhostTrail* v_ghostTrail = i_scene->getGhostTrail(); 
  if(v_ghostTrail == 0){
    return;
  }
  std::vector<Vector2f>* v_ghostTrailData = v_ghostTrail->getGhostTrailData();
  
  //setup colors and declare vars
  TColor c = TColor(255,255,0,255);
  float fSize=0.0,v_last_size=0.0,v_xdiff,v_ydiff;
  int lines_drawn=0; //for debug mode
  
  //calculate nice quality
  int v_offset=0;
  float v_cZoom=i_scene->getCamera()->getCurrentZoom();
  switch(m_graphicsLevel){ //calculate nice step value
    case GFX_LOW:    
      v_offset=GT_GFX_LOW_RATIO/v_cZoom;
      break;//low gfx mode! 
    case GFX_MEDIUM: 
      v_offset=GT_GFX_MED_RATIO/v_cZoom;
      break;//medium gfx mode! 
    case GFX_HIGH:   
      v_offset= GT_GFX_HI_RATIO/v_cZoom;
      break;//high gfx mode! 
  }
  if(v_offset<1){  //no infinite loop, thank you.
    v_offset=1;
  }
  
  //grab rendering device
  DrawLib* pDrawlib = GameApp::instance()->getDrawLib(); 
  if(pDrawlib==0){
    return;
  }
  //draw nice or ugly?
  if(XMSession::instance()->ugly()) {
    v_offset=v_offset*GT_UGLY_MODE_MULTIPLYER; //make it more uglier
  }
  //draw the lines!
  for(unsigned int i=0; i < (*v_ghostTrailData).size(); i=i+v_offset) {
      
      if (!(i>0)){
        v_last_size=0.1;
        continue;
      }
      //get speed/size
      v_xdiff=(*v_ghostTrailData)[i-v_offset].x - (*v_ghostTrailData)[i].x;
      if(v_xdiff<0.0){
        v_xdiff=-v_xdiff;
      }
      v_ydiff=(*v_ghostTrailData)[i-v_offset].y - (*v_ghostTrailData)[i].y;
      if(v_ydiff<0.0){
        v_ydiff=-v_ydiff;
      }
      fSize=sqrt(pow(v_xdiff,2)+pow(v_ydiff,2))/v_offset*2.0;	
      if(fSize>10.0){   //if the size (=speed) is more than 10 then skip, you can't go that fast ;-) 		
        v_last_size=0.1f; 
        continue;
      }
      //max and min sizes
      if(fSize>1.0f) {   
        fSize=1.0f;
      } 
      if(fSize<0.1f){
        fSize=0.1f;
      }
      fSize=v_last_size*0.94+fSize*(1.0-0.94); //interpolate, to make it nice and smooth
      
      //we need to check that the line is inside the screen, why to draw 2000-5000 lines when we can draw ~100 by skipping invisible ones.
      Vector2f scr_max = i_screenBBox->getBMax();
      Vector2f scr_min = i_screenBBox->getBMin();
      Vector2f point = (*v_ghostTrailData)[i];
      
      if(scr_max.x < point.x - v_xdiff*2.5 ||
         scr_min.x > point.x + v_xdiff*2.5 ||
         scr_max.y < point.y - v_ydiff*2.5 ||
         scr_min.y > point.y + v_ydiff*2.5) {
        v_last_size=fSize;
        continue;
      }

      //set fancy colours
      c.setGreen(GT_TRAIL_COLOR - fSize * GT_TRAIL_COLOR);
      
      //render at last!
      pDrawlib->DrawLine((*v_ghostTrailData)[i-v_offset], 		   //start pos
			 (*v_ghostTrailData)[i],   			   //end pos
			 MAKE_COLOR(c.Red(),c.Green(),c.Blue(),c.Alpha()), //color
			 (XMSession::instance()->ugly() ? v_last_size*GT_RENDER_SCALE*i_scale/2 : v_last_size*GT_RENDER_SCALE*i_scale), //start size
			 (XMSession::instance()->ugly() ? fSize*GT_RENDER_SCALE*i_scale/2 : fSize*GT_RENDER_SCALE*i_scale),		//end size
			 (i_scale>0.4)); //only if scale is big enough     //toggle rounded ends
			
      lines_drawn++;
      v_last_size=fSize;
  }
  //print amount of lines drawn and the camera's zoom value, usefull for testing.
  if( XMSession::instance()->debug() ) {
    std::stringstream out; out << lines_drawn;
    std::stringstream out2; out2 << v_cZoom;
    i_scene->gameMessage("drawed "+out.str()+" lines. Zoom is "+out2.str()+".", true, 50);
    _RenderCircle(20,MAKE_COLOR(10,10,255,255),i_scene->Cameras()[0]->getTrailCamAimPos(), 0.25);
    _RenderCircle(20,MAKE_COLOR(255,10,10,255),i_scene->Cameras()[0]->getNearestPointOnTrailPos(), 0.25);	  
  }
}

void GameRenderer::displayArrowIndication(Biker* i_biker, AABB *i_screenBBox) {
  Vector2f v_arrowPoint;
  float v_arrowAngle;
  float v_spriteSize;
  float v_spriteSizeMin = 0.2;
  float v_spriteSizeMax = 0.7;
  float v_spriteSizeLimitMin = 5.0; //
  float v_spriteSizeLimitMax = 60.0; // if the distance if more than 40m, arrow will no more be reduced and will be v_spriteSizeMin
  float v_spriteOffset = 0.5; // don't display the arrow exactly at the border
  float v_infoOffset = 1.0; // don't display the name exactly at the border
  float v_bikerOutMarge = 1.5; // don't display the arrow if the biker is almost on the screen
  float v_arrowAngleDeg;
  std::ostringstream v_distanceStr;

  Vector2f p1(1,0), p2(1,0), p3(1,0), p4(1,0);
  AABBSide v_side;
  float v_distance; // distance between the center of the camera and the biker
  float a, b;
  Vector2f v_infoPosition;

  // display the arrow only if the biker if far enough of the screen
  if(i_screenBBox->getBMin().x - v_bikerOutMarge > i_biker->getState()->CenterP.x ||
     i_screenBBox->getBMax().x + v_bikerOutMarge < i_biker->getState()->CenterP.x ||
     i_screenBBox->getBMin().y - v_bikerOutMarge > i_biker->getState()->CenterP.y ||
     i_screenBBox->getBMax().y + v_bikerOutMarge < i_biker->getState()->CenterP.y
     ) {
	
    // display the arrow only if the biker in not on the screen
    if(getBikerDirection(i_biker, i_screenBBox, &v_arrowPoint, &v_arrowAngle, &v_side)) {
	  
      a = i_biker->getState()->CenterP.x - (i_screenBBox->getBMin().x + (i_screenBBox->getBMax().x - i_screenBBox->getBMin().x)/2.0);
      b = i_biker->getState()->CenterP.y - (i_screenBBox->getBMin().y + (i_screenBBox->getBMax().y - i_screenBBox->getBMin().y)/2.0);

      v_distance = sqrt(a*a + b*b);
      v_distanceStr << ((int)v_distance) << "m";
  
      if(v_distance < v_spriteSizeLimitMin) {
	v_spriteSize = v_spriteSizeMax;
      } else if(v_distance > v_spriteSizeLimitMax) {
	v_spriteSize = v_spriteSizeMin;
      } else {
	v_spriteSize = v_spriteSizeMax - (((v_distance - v_spriteSizeLimitMin) / (v_spriteSizeLimitMax - v_spriteSizeLimitMin)) * v_spriteSizeMax);
	if(v_spriteSize < v_spriteSizeMin) {
	  v_spriteSize = v_spriteSizeMin;
	}
      }
	  
      v_arrowAngleDeg = (v_arrowAngle * 180) / M_PI - 45.0;
      p1.rotateXY(v_arrowAngleDeg);
      p2.rotateXY(90+v_arrowAngleDeg);
      p3.rotateXY(180+v_arrowAngleDeg);
      p4.rotateXY(270+v_arrowAngleDeg);
	  
      p1 = p1 * v_spriteSize;
      p2 = p2 * v_spriteSize;
      p3 = p3 * v_spriteSize;
      p4 = p4 * v_spriteSize;
	  
      v_infoPosition = v_arrowPoint;

      // arrow
      if(v_arrowPoint.x > i_screenBBox->getBMax().x - v_spriteOffset) {
	v_arrowPoint.x = i_screenBBox->getBMax().x - v_spriteOffset;
      }
	  
      if(v_arrowPoint.x < i_screenBBox->getBMin().x + v_spriteOffset) {
	v_arrowPoint.x = i_screenBBox->getBMin().x + v_spriteOffset;
      }
	  
      if(v_arrowPoint.y > i_screenBBox->getBMax().y - v_spriteOffset) {
	v_arrowPoint.y = i_screenBBox->getBMax().y - v_spriteOffset;
      }
	  
      if(v_arrowPoint.y < i_screenBBox->getBMin().y + v_spriteOffset) {
	v_arrowPoint.y = i_screenBBox->getBMin().y + v_spriteOffset;
      }
	  
      // info
      if(v_infoPosition.x > i_screenBBox->getBMax().x - v_infoOffset) {
	v_infoPosition.x = i_screenBBox->getBMax().x - v_infoOffset;
      }
	  
      if(v_infoPosition.x < i_screenBBox->getBMin().x + v_infoOffset) {
	v_infoPosition.x = i_screenBBox->getBMin().x + v_infoOffset;
      }
	  
      if(v_infoPosition.y > i_screenBBox->getBMax().y - v_infoOffset) {
	v_infoPosition.y = i_screenBBox->getBMax().y - v_infoOffset;
      }
	  
      if(v_infoPosition.y < i_screenBBox->getBMin().y + v_infoOffset) {
	v_infoPosition.y = i_screenBBox->getBMin().y + v_infoOffset;
      }

      if(m_arrowSprite != NULL) {
	_RenderAlphaBlendedSection(m_arrowSprite->getTexture(), p1+v_arrowPoint, p2+v_arrowPoint, p3+v_arrowPoint, p4+v_arrowPoint);
	_RenderInGameText(v_infoPosition, i_biker->getVeryQuickDescription() + "\n" + v_distanceStr.str(),
			  MAKE_COLOR(255,255,255, 255), 0.5, 0.5);
      }
    }
  }
}

bool GameRenderer::getBikerDirection(Biker* i_biker, AABB* i_screenBBox,
				     Vector2f* o_arrowPoint, float* o_arrowAngle, AABBSide* o_side) {
  Vector2f v_centerPoint = Vector2f(i_screenBBox->getBMin().x + (i_screenBBox->getBMax().x-i_screenBBox->getBMin().x)/2.0,
				    i_screenBBox->getBMin().y + (i_screenBBox->getBMax().y-i_screenBBox->getBMin().y)/2.0);
  float a, b;
    
  if(i_screenBBox->lineTouchBorder(v_centerPoint, i_biker->getState()->CenterP, *o_arrowPoint, *o_side) == false) {
    return false;
  }

  if(*o_side == AABB_TOP || *o_side == AABB_BOTTOM) {
    a = (i_screenBBox->getBMax().y-i_screenBBox->getBMin().y)/2.0;
    b = o_arrowPoint->x - (i_screenBBox->getBMin().x + (i_screenBBox->getBMax().x-i_screenBBox->getBMin().x)/2.0);
  } else {
    a = (i_screenBBox->getBMax().x-i_screenBBox->getBMin().x)/2.0;
    b = o_arrowPoint->y - (i_screenBBox->getBMin().y + (i_screenBBox->getBMax().y-i_screenBBox->getBMin().y)/2.0);
  }

  switch(*o_side) {
  case AABB_TOP:
    if(b>=0) {
      *o_arrowAngle = atan(a/b) - M_PI/2;
    } else {
      *o_arrowAngle = atan(a/b) + M_PI/2;
    }
    break;
  case AABB_BOTTOM:
    if(b>=0) {
      *o_arrowAngle = atan(a/-b) - M_PI/2;
    } else {
      *o_arrowAngle = atan(a/-b) + M_PI/2;
    }
    break;
  case AABB_LEFT:
    if(b>=0) {
      *o_arrowAngle = atan(a/b);
    } else {
      *o_arrowAngle = atan(a/b) + M_PI;
    }
    break;
  case AABB_RIGHT:
    if(b>=0) {
      *o_arrowAngle = atan(a/-b);
    } else {
      *o_arrowAngle = atan(a/-b) + M_PI;
    }
    break;
  }

  return true;
}

int GameRenderer::nbParticlesRendered() const {
  return m_nParticlesRendered;
}

  /*===========================================================================
  Main rendering function
  ===========================================================================*/
  void GameRenderer::render(Scene* i_scene) {
    Camera*  pCamera  = i_scene->getCamera();
    DrawLib* pDrawlib = GameApp::instance()->getDrawLib();

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
      const SkyApparence* pSky = i_scene->getLevelSrc()->Sky();
      _RenderSky(i_scene,
		 pSky->Zoom(),
		 pSky->Offset(),
		 pSky->TextureColor(),
		 pSky->DriftZoom(),
		 pSky->DriftTextureColor(),
		 pSky->Drifted());
    }    

    if(m_graphicsLevel == GFX_HIGH && XMSession::instance()->ugly() == false) {
      /* background level blocks */
      _RenderLayers(i_scene, false);
    }

    // the layers may have change the scale transformation
    setCameraTransformations(pCamera, m_xScale, m_yScale);

    if(m_graphicsLevel != GFX_LOW && XMSession::instance()->ugly() == false) {
      /* Background blocks */
      _RenderDynamicBlocks(i_scene, true);
      _RenderBackground(i_scene);
      
      /* ... then render background sprites ... */      
    }
    
    _RenderSprites(i_scene, false,true);

    if(m_graphicsLevel == GFX_HIGH && XMSession::instance()->ugly() == false) {
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
	_RenderZone(i_scene->getLevelSrc()->Zones()[i], false);
      }
    }
    else if(XMSession::instance()->ugly() || m_graphicsLevel == GFX_LOW) {
      for(unsigned int i=0; i<i_scene->getLevelSrc()->Zones().size(); i++) {
	_RenderZone(i_scene->getLevelSrc()->Zones()[i], true);
      }
    }

    /* ghosts */
    bool v_found = false;
    int v_found_i = 0;
    float v_textOffset, v_found_textOffset = 0.0;

    for(unsigned int i=0; i<i_scene->Ghosts().size(); i++) {
      Ghost* v_ghost = i_scene->Ghosts()[i];
      v_textOffset = 0.0;
     
      for(unsigned int j=0; j<i; j++) {
	if(fabs(i_scene->Ghosts()[j]->getState()->CenterP.x - v_ghost->getState()->CenterP.x) < 2.0 &&
	   fabs(i_scene->Ghosts()[j]->getState()->CenterP.y - v_ghost->getState()->CenterP.y) < 2.0
	   ) {
	  v_textOffset += 2.0;
	}
      }

      if(v_ghost != pCamera->getPlayerToFollow()) {
	_RenderGhost(i_scene, v_ghost, i, v_textOffset);
        if(i_scene->getGhostTrail() != 0)
          if(XMSession::instance()->renderGhostTrail() || XMSession::instance()->renderGhostTrailTS()) { 
        	_RenderGhostTrail(i_scene, &m_screenBBox, m_sizeMultOfEntitiesToTake); 
          }
      } else {
	v_found = true;
	v_found_i = i;
	v_found_textOffset = v_textOffset;
      }
    }
    /* draw the player to follow over the others */
    if(v_found) {
      _RenderGhost(i_scene, i_scene->Ghosts()[v_found_i], v_found_i, v_found_textOffset);
      
    } 

    /* ... followed by the bike ... */
    v_found = false;
    for(unsigned int i=0; i<i_scene->Players().size(); i++) {
      Biker* v_player = i_scene->Players()[i];
      if(v_player != pCamera->getPlayerToFollow()) {
	try {
	  _RenderBike(v_player,
		      v_player->getRenderBikeFront(),
		      v_player->getColorFilter(),
		      v_player->getUglyColorFilter());
	  if(XMSession::instance()->showBikersArrows()) {
	    displayArrowIndication(v_player, &m_screenBBox);
	  }
	} catch(Exception &e) {
	  i_scene->gameMessage("Unable to render the biker", true, 50);
	}
      } else {
	v_found = true;
      }
    }
    if(v_found) {
      try {
	Biker* pBiker = pCamera->getPlayerToFollow();
	_RenderBike(pBiker,
		    pBiker->getRenderBikeFront(),
		    pBiker->getColorFilter(),
		    pBiker->getUglyColorFilter());

	if(XMSession::instance()->debug()) {
	  GameApp::instance()->getDrawLib()->setTexture(NULL, BLEND_MODE_NONE);
	  // render collision points
	  for(unsigned int j=0; j<pCamera->getPlayerToFollow()->CollisionPoints().size(); j++) {
	    _RenderCircle(16, MAKE_COLOR(255,255,0,255),pCamera->getPlayerToFollow()->CollisionPoints()[j], 0.02);
	  }
	}
      } catch(Exception &e) {
	i_scene->gameMessage("Unable to render the biker", true, 50);
      }
    }
    
    if(m_graphicsLevel == GFX_HIGH && XMSession::instance()->ugly() == false) {
      /* Render particles (front!) */    
      _RenderParticles(i_scene, true);
    }
    
    /* ... and finally the foreground sprites! */
    _RenderSprites(i_scene, true,false);

    /* and finally finally, front layers */
    if(m_graphicsLevel == GFX_HIGH && XMSession::instance()->ugly() == false) {
      _RenderLayers(i_scene, true);
    }

    // put it back
    setCameraTransformations(pCamera, m_xScale, m_yScale);

    if(XMSession::instance()->debug()) {
      GameApp::instance()->getDrawLib()->setTexture(NULL, BLEND_MODE_NONE);

      /* Draw some collision handling debug info */
      CollisionSystem *pc = i_scene->getCollisionHandler();
      for(unsigned int i=0;i<pc->m_CheckedLines.size();i++) {
        pDrawlib->setLineWidth(3);
	pDrawlib->startDraw(DRAW_MODE_LINE_STRIP);
	pDrawlib->setColorRGB(255,0,0);
        pDrawlib->glVertex(pc->m_CheckedLines[i]->x1,pc->m_CheckedLines[i]->y1);
        pDrawlib->glVertex(pc->m_CheckedLines[i]->x2,pc->m_CheckedLines[i]->y2);
	pDrawlib->endDraw();
        pDrawlib->setLineWidth(2);
      }
      for(unsigned int i=0;i<pc->m_CheckedCells.size();i++) {
	pDrawlib->startDraw(DRAW_MODE_LINE_LOOP);
	pDrawlib->setColorRGB(255,0,0);

        pDrawlib->glVertex(pc->m_CheckedCells[i].x1,pc->m_CheckedCells[i].y1);
        pDrawlib->glVertex(pc->m_CheckedCells[i].x2,pc->m_CheckedCells[i].y1);
        pDrawlib->glVertex(pc->m_CheckedCells[i].x2,pc->m_CheckedCells[i].y2);
        pDrawlib->glVertex(pc->m_CheckedCells[i].x1,pc->m_CheckedCells[i].y2);
	pDrawlib->endDraw();
      }
      for(unsigned int i=0;i<pc->m_CheckedLinesW.size();i++) {
        pDrawlib->setLineWidth(1);
	pDrawlib->startDraw(DRAW_MODE_LINE_STRIP);
	pDrawlib->setColorRGB(0,255,0);
        pDrawlib->glVertex(pc->m_CheckedLinesW[i]->x1,pc->m_CheckedLinesW[i]->y1);
        pDrawlib->glVertex(pc->m_CheckedLinesW[i]->x2,pc->m_CheckedLinesW[i]->y2);
	pDrawlib->endDraw();
        pDrawlib->setLineWidth(1);
      }
      for(unsigned int i=0;i<pc->m_CheckedCellsW.size();i++) {
	pDrawlib->startDraw(DRAW_MODE_LINE_LOOP);
	pDrawlib->setColorRGB(0,255,0);
        pDrawlib->glVertex(pc->m_CheckedCellsW[i].x1,pc->m_CheckedCellsW[i].y1);
        pDrawlib->glVertex(pc->m_CheckedCellsW[i].x2,pc->m_CheckedCellsW[i].y1);
        pDrawlib->glVertex(pc->m_CheckedCellsW[i].x2,pc->m_CheckedCellsW[i].y2);
        pDrawlib->glVertex(pc->m_CheckedCellsW[i].x1,pc->m_CheckedCellsW[i].y2);
	pDrawlib->endDraw();
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
	  v_color = MAKE_COLOR(255,0,255,255); /* Fix: color not same as blocks */
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

      // render joints
      std::vector<Joint*>& joints = i_scene->getLevelSrc()->Joints();
      for(unsigned int i=0; i<joints.size(); i++) {
	_RenderCircle(16, MAKE_COLOR(151, 0, 255, 255),
		      joints[i]->DynamicPosition(), 0.2f);
      }

      /* Render debug info */
      _RenderDebugInfo();
      
      pDrawlib->setLineWidth(3);
      GameApp::instance()->getDrawLib()->setTexture(NULL, BLEND_MODE_NONE);
      pDrawlib->startDraw(DRAW_MODE_LINE_LOOP);
      pDrawlib->setColorRGB(0,0,255);
      pDrawlib->glVertex(m_screenBBox.getBMin().x,m_screenBBox.getBMin().y);
      pDrawlib->glVertex(m_screenBBox.getBMin().x,m_screenBBox.getBMax().y);
      pDrawlib->glVertex(m_screenBBox.getBMax().x,m_screenBBox.getBMax().y);
      pDrawlib->glVertex(m_screenBBox.getBMax().x,m_screenBBox.getBMin().y);
      pDrawlib->endDraw();
      pDrawlib->setLineWidth(2);
  
    }

    pCamera->setCamera2d();

    /* minimap + counter */
    if(pCamera->getPlayerToFollow() != NULL) {
      if(showMinimap()) {
        int multiPlayerScale = 0;
        if(XMSession::instance()->multiNbPlayers() > 2) {  //small small screen then
          multiPlayerScale = 2;
        }
	renderMiniMap(i_scene, 0, pDrawlib->getDispHeight() - (pDrawlib->getDispHeight()/(6+multiPlayerScale)),
		      pDrawlib->getDispWidth()/(5+multiPlayerScale), pDrawlib->getDispHeight()/(6+multiPlayerScale));
      }
      if(showEngineCounter()
	 && XMSession::instance()->ugly() == false
	 && i_scene->getNumberCameras() == 1) {
	int v_engineCounterSize = pDrawlib->getDispWidth()/7;
	renderEngineCounter(pDrawlib->getDispWidth()- v_engineCounterSize,
			    pDrawlib->getDispHeight()-v_engineCounterSize,
			    v_engineCounterSize, v_engineCounterSize,
			    pCamera->getPlayerToFollow()->getBikeEngineSpeed(),
			    pCamera->getPlayerToFollow()->getBikeLinearVel());
      }
    }

    pDrawlib->getMenuCamera()->setCamera2d();
    
    if(m_showTimePanel) {
      renderTimePanel(i_scene);
      /* If there's strawberries in the level, tell the user how many there's left */
      _RenderGameStatus(i_scene);
    }

    renderReplayHelpMessage(i_scene);

    /* And then the game messages beyond shadow layer*/
    _RenderGameMessages(i_scene,false);

    pCamera->setCamera2d();
    
    _RenderScreenShadow(i_scene);

    /* And then the game messages over shadow layer*/
    pDrawlib->getMenuCamera()->setCamera2d();
    
    // _RenderGameMessages(i_scene,true);  => got moved to stateScene, to have non-covered multiplayer message display

    /* Render Level Name at bottom of screen, when died or pause */
    FontManager* v_fm = pDrawlib->getFontMedium();
    FontGlyph* v_fg = v_fm->getGlyph(i_scene->getInfos());
    v_fm->printString(v_fg,
		      5,
		      pDrawlib->getDispHeight() - v_fg->realHeight() - 2,
		      MAKE_COLOR(255,255,255,255), 0.0, true);
  }

  /*===========================================================================
  Game status rendering
  ===========================================================================*/
  void GameRenderer::_RenderGameStatus(Scene* i_scene) {
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
    Vector2i bottomLeft = i_scene->getCamera()->getDispBottomLeft();

    // adapt to the current camera
    float x1_cam = x1 + bottomLeft.x;
    float x2_cam = x2 + bottomLeft.x;
    float y1_cam = y1;
    float y2_cam = y2;

    //seek correct drawing positions
    unsigned int height=GameApp::instance()->getDrawLib()->getDispHeight();
    if((unsigned int) bottomLeft.y != height) {
      int y=height/2;
      y1_cam += y;
      y2_cam += y;
    }

    if(XMSession::instance()->ugly() == false) {
      pType = i_scene->getLevelSrc()->flowerSprite();
    }
    
    if(nStrawberriesLeft > 0) {
      if(XMSession::instance()->ugly() == false) {
	pType = i_scene->getLevelSrc()->strawberrySprite();
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
      snprintf(cBuf, 256, "%d", nQuantity);

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
  void GameRenderer::_RenderGameMessages(Scene* i_scene, bool renderOverShadow) {
    float v_fZoom = 60.0f;
    DrawLib* pDrawlib = GameApp::instance()->getDrawLib();

    /* Arrow messages */
    ArrowPointer *pArrow = &(i_scene->getArrowPointer());
    if(pArrow->nArrowPointerMode != 0) {
      Vector2f C;
      if(pArrow->nArrowPointerMode == 1) {          
        C=Vector2f(pDrawlib->getDispWidth()/2 + (float)(pArrow->ArrowPointerPos.x - i_scene->getCamera()->getCameraPositionX())*v_fZoom,
                  pDrawlib->getDispHeight()/2 - (float)(pArrow->ArrowPointerPos.y - i_scene->getCamera()->getCameraPositionY())*v_fZoom);      
      }
      else if(pArrow->nArrowPointerMode == 2) {          
        C.x=(pDrawlib->getDispWidth() * pArrow->ArrowPointerPos.x) / 800.0f;
        C.y=(pDrawlib->getDispHeight() * pArrow->ArrowPointerPos.y) / 600.0f;
      }

      Vector2f p1(1,0), p2(1,0), p3(1,0), p4(1,0);
      float arrowAngle = pArrow->fArrowPointerAngle;
      p1.rotateXY(arrowAngle);
      p2.rotateXY(90+arrowAngle);
      p3.rotateXY(180+arrowAngle);
      p4.rotateXY(270+arrowAngle);

      p1 = p1 * 50.0f;
      p2 = p2 * 50.0f;
      p3 = p3 * 50.0f;
      p4 = p4 * 50.0f;

      if(m_arrowSprite != NULL) {
	_RenderAlphaBlendedSectionSP(m_arrowSprite->getTexture(),p1+C,p2+C,p3+C,p4+C);      
      }
    }
        
    /* Messages */
    if(i_scene != NULL) {
      for(unsigned int i=0;i<i_scene->getGameMessage().size();i++) {
        GameMessage *pMsg = i_scene->getGameMessage()[i];
        FontManager* v_fm = NULL;
        FontGlyph* v_fg = NULL;
        unsigned int width  = pDrawlib->getDispWidth();
        unsigned int height = pDrawlib->getDispHeight();
        int numPlayers = XMSession::instance()->multiNbPlayers();
        Vector2i bottomLeft = i_scene->getCamera()->getDispBottomLeft();        
        int posX = 0; 
        int posY = 0;

        #define GET_FONT_SMALL  v_fm = pDrawlib->getFontSmall(); \
	                        v_fg = v_fm->getGlyph(pMsg->Text);
        #define GET_FONT_MEDIUM v_fm = pDrawlib->getFontMedium(); \
        			v_fg = v_fm->getGlyph(pMsg->Text);

        if( (pMsg->msgType == gameMsg && renderOverShadow) ||
            (pMsg->msgType == gameTime && numPlayers == 1) ) {
          GET_FONT_MEDIUM    
          posX = int(width/2 - v_fg->realWidth()/2);        
          posY = int(pMsg->Pos[1] * height);
          v_fm->printString(v_fg, posX, posY, MAKE_COLOR(255,255,255,pMsg->nAlpha), 0.0, true);
	} 
	else {
          switch(pMsg->msgType) {
            case gameMsg:
              continue;
	    case levelID:
	      //put text to higher position
	      if(renderOverShadow) {
	        GET_FONT_MEDIUM
                posX = int(width/2 - v_fg->realWidth()/2);
	        posY = int(pMsg->Pos[1] * height - height/6);
	        
	        if(XMSession::instance()->ugly() == false) {
	        pDrawlib->drawBox(Vector2f(posX- 10,posY- 5),
			        Vector2f(posX + v_fg->realWidth() +10, posY+33),
			        1,MAKE_COLOR(0,0,0,pMsg->nAlpha/2),MAKE_COLOR(255,244,176,pMsg->nAlpha));
	        }
	      }
	      else continue;
	      break;
	    case scripted:
	      if(renderOverShadow == false) {
	      //scripted text for display below the bike
	        if(numPlayers == 1) {
	          GET_FONT_MEDIUM
	          posX = int(width/2 - v_fg->realWidth()/2);
	          posY = int(pMsg->Pos[1] * height + height/5);
	        }
	        else{
	          GET_FONT_SMALL
	      	  if(numPlayers > 1) {
                    if((unsigned int) bottomLeft.x != 0 && numPlayers > 2) {
                      posX = int(width/2 + width/4 - v_fg->realWidth()/2);
                    }
                    else if(numPlayers > 2) {
                      posX = int(width/4 - v_fg->realWidth()/2);    
                    }
                    else {
                      posX = int(width/2 - v_fg->realWidth()/2);
                    }        
                
                    if((unsigned int) bottomLeft.y != height) {
                      posY = int( pMsg->Pos[1] * (height/2)  + height/2 + height/10);
	            }
	            else {
	              posY = int( pMsg->Pos[1] * (height/2) + height/10);
	            }     
	          }
                }
	        if(XMSession::instance()->ugly() == false) {
	          pDrawlib->drawBox(Vector2f(posX- 15,posY- 1),
		    	            Vector2f(posX + v_fg->realWidth() +15 , posY+v_fg->realHeight()+ 2),
			            0,MAKE_COLOR(0,0,0,int(pMsg->nAlpha/2)));
	        }
	      }
	      else continue;
	      break;  
	    case gameTime:
	      //especially in multiplayer: render in according center size  
              if(numPlayers == 1) {
	        GET_FONT_MEDIUM
	        posX = int(width/2 - v_fg->realWidth()/2);
	      }
	      else{
	        GET_FONT_SMALL
	      }
	       
              if(numPlayers > 1) {
                if((unsigned int) bottomLeft.x != 0 && numPlayers > 2) {
                  posX = int(width/2 + width/4 - v_fg->realWidth()/2);
                }
                else if(numPlayers > 2) {
                  posX = int(width/4 - v_fg->realWidth()/2);    
                }        
            
                if((unsigned int) bottomLeft.y != height) {
                  posY = int( pMsg->Pos[1] * (height/2)  + height/2 - height/10);
	        }
	        else {
	          posY = int( pMsg->Pos[1] * (height/2) - height/10);
	        }     
	      }	      
	      break;   
	  }
	  v_fm->printString(v_fg, posX, posY, MAKE_COLOR(255,255,255,pMsg->nAlpha), 0.0, true);
	}

      }
    }
  }
  
  /*===========================================================================
  Sprite rendering main
  ===========================================================================*/
void GameRenderer::_RenderSprites(Scene* i_scene, bool bForeground,bool bBackground) {
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
    unsigned int size = Entities.size();

    if(size == 0)
      return;

    std::sort(Entities.begin(), Entities.end(),
	      AscendingEntitySort());

    for(unsigned int i=0; i<size; i++) {
      pEnt = Entities[i];

      try {

      switch(pEnt->Speciality()) {
      case ET_NONE:
	/* Middleground? (not foreground, not background) */
	if(pEnt->Z() == 0.0f && !bForeground && !bBackground && ( (m_graphicsLevel == GFX_HIGH && !XMSession::instance()->ugly()) || pEnt->isScripted()) ) {
	  _RenderSprite(i_scene, pEnt);  
	} 
	else {
	  /* In front? */
	  if(pEnt->Z() > 0.0f && bForeground && ( (m_graphicsLevel == GFX_HIGH && !XMSession::instance()->ugly()) || pEnt->isScripted())) {
	    _RenderSprite(i_scene, pEnt);
	  } 
	  else {
	    /* Those in back? */
	    if(pEnt->Z() < 0.0f && bBackground && ( (m_graphicsLevel == GFX_HIGH && !XMSession::instance()->ugly()) || pEnt->isScripted())) {
	      _RenderSprite(i_scene, pEnt);
	    }
	  }
	}
	break;
      case ET_PARTICLES_SOURCE:
	// do not render particles this way
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
void GameRenderer::_RenderSprite(Scene* i_scene, Entity *pEntity, float i_sizeMult) {  
    AnimationSprite* v_sprite = NULL;
    float v_centerX = 0.0f;
    float v_centerY = 0.0f;
    float v_width   = 0.0f;
    float v_height  = 0.0f;
    std::string v_spriteName = "";

    if(XMSession::instance()->ugly() == false) {
      switch(pEntity->Speciality()) {
      case ET_KILL:
	v_spriteName = i_scene->getLevelSrc()->SpriteForWecker();
	v_sprite     = (AnimationSprite*)i_scene->getLevelSrc()->wreckerSprite();
        break;
      case ET_MAKEWIN:
	v_spriteName = i_scene->getLevelSrc()->SpriteForFlower();
	v_sprite     = (AnimationSprite*)i_scene->getLevelSrc()->flowerSprite();
        break;
      case ET_ISTOTAKE:
	v_spriteName = i_scene->getLevelSrc()->SpriteForStrawberry();
	v_sprite     = (AnimationSprite*)i_scene->getLevelSrc()->strawberrySprite();
        break;
      default:
	v_spriteName = pEntity->SpriteName();
      }

      if(v_sprite == NULL){
	v_sprite = (AnimationSprite*)pEntity->getSprite();
      }

      if(v_sprite != NULL) {
	v_centerX = v_sprite->getCenterX();
	v_centerY = v_sprite->getCenterY();

	if(pEntity->Width() > 0.0) {
	  v_width  = pEntity->Width();
	  v_height = pEntity->Height();
	  v_centerX += (pEntity->Width() -v_sprite->getWidth())  / 2.0;
	  v_centerY += (pEntity->Height()-v_sprite->getHeight()) / 2.0;   
	} else {
	  v_width  = v_sprite->getWidth();
	  v_height = v_sprite->getHeight();
	}
      }

      if(i_sizeMult != 1.0) {
	v_centerX -= (v_width  - (v_width  * i_sizeMult)) / 2.0;
	v_centerY -= (v_height - (v_height * i_sizeMult)) / 2.0;
	v_width  *= i_sizeMult;
	v_height *= i_sizeMult;
      }	

      if(v_sprite != NULL) {
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
	if(pEntity->DrawAngle() != 0.0) { /* generally not nice to test a float and 0.0
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
	    
	    p[i].x = (cosf(pEntity->DrawAngle() + beta) * v_ray);
	    p[i].y = (sinf(pEntity->DrawAngle() + beta) * v_ray);
	  }
	  //pEntity->setDrawAngle(pEntity->DrawAngle() + 0.01);
	}

	/* reversed ? */
	if(pEntity->DrawReversed()) {
	  Vector2f v_tmp;
	  v_tmp = p[0];
	  p[0] = p[1];
	  p[1] = v_tmp;
	  v_tmp = p[2];
	  p[2] = p[3];
	  p[3] = v_tmp;
	} 

	/* vector to the good position */
	p[0] += pEntity->DynamicPosition();
	p[1] += pEntity->DynamicPosition();
	p[2] += pEntity->DynamicPosition();
	p[3] += pEntity->DynamicPosition();

        if(v_sprite->getBlendMode() == SPRITE_BLENDMODE_ADDITIVE) {
          _RenderAdditiveBlendedSection(v_sprite->getTexture(),p[0],p[1],p[2],p[3]);
        }
        else {
#ifdef ENABLE_OPENGL
          glEnable(GL_ALPHA_TEST);
          glAlphaFunc(GL_GEQUAL,0.5f);
#endif
          _RenderAlphaBlendedSection(v_sprite->getTexture(),p[0],p[1],p[2],p[3]);
#ifdef ENABLE_OPENGL
          glDisable(GL_ALPHA_TEST);
#endif
        }
      }    
    }
    /* If this is debug-mode, also draw entity's area of effect */
    if(XMSession::instance()->debug()
       || XMSession::instance()->testTheme()
       || XMSession::instance()->ugly()) {
      Vector2f C = pEntity->DynamicPosition();
      Color v_color;
      
      switch(pEntity->Speciality()) {
      case ET_KILL:
        v_color = MAKE_COLOR(80,255,255,255); /* Fix: color changed a bit so it's easier to spot */
        break;
      case ET_MAKEWIN:
        v_color = MAKE_COLOR(255,0,255,255); /* Fix: color not same as blocks */
        break;
      case ET_ISTOTAKE:
        v_color = MAKE_COLOR(255,0,0,255);
        break;
      case ET_ISSTART:
        v_color = MAKE_COLOR(0,0,0,0);  /* we dont wanna have start position displayed in ugly mode */
        break;
      default:
        v_color = MAKE_COLOR(255,255,90,255); /* then only the entitys used in scripts get displayed */
        break;
      }

      GameApp::instance()->getDrawLib()->setTexture(NULL, BLEND_MODE_NONE);
      _RenderCircle(20, v_color, C, pEntity->Size() * i_sizeMult);
    }
  }
     
  /*===========================================================================
  Blocks (dynamic)
  ===========================================================================*/
void GameRenderer::_RenderDynamicBlocks(Scene* i_scene, bool bBackground) {
    DrawLib* pDrawlib = GameApp::instance()->getDrawLib();

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

	if(pDrawlib->getBackend() == DrawLib::backend_OpenGl) {
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

	  pDrawlib->setTexture(m_DynamicGeoms[geom]->pTexture, BLEND_MODE_A);
	  
	  /* set flashy blendColor */
	  TColor v_blendColor =  TColor(block->getBlendColor());
	  pDrawlib->setColorRGBA(v_blendColor.Red(), v_blendColor.Green(), v_blendColor.Blue(), v_blendColor.Alpha());


	  /* VBO optimized? */
	  if(pDrawlib->useVBOs()) {
	    for(unsigned int j=0;j<m_DynamicGeoms[geom]->Polys.size();j++) {          
	      GeomPoly *pPoly = m_DynamicGeoms[geom]->Polys[j];

	      ((DrawLibOpenGL*)pDrawlib)->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nVertexBufferID);
	      glVertexPointer(2,GL_FLOAT,0,(char *)NULL);

	      ((DrawLibOpenGL*)pDrawlib)->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nTexCoordBufferID);
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
	} else if(pDrawlib->getBackend() == DrawLib::backend_SdlGFX){

	  if(m_DynamicGeoms[geom]->Polys.size() > 0) {
	    pDrawlib->setTexture(m_DynamicGeoms[geom]->pTexture,BLEND_MODE_A);
	    pDrawlib->setColorRGB(255,255,255);

	    for(unsigned int j=0;j<m_DynamicGeoms[geom]->Polys.size();j++) {          
	      pDrawlib->startDraw(DRAW_MODE_POLYGON);
	      for(unsigned int k=0;k<m_DynamicGeoms[geom]->Polys[j]->nNumVertices;k++) {
		Vector2f vertex = Vector2f(m_DynamicGeoms[geom]->Polys[j]->pVertices[k].x,
					   m_DynamicGeoms[geom]->Polys[j]->pVertices[k].y);
		/* transform vertex */
		Vector2f transVertex = Vector2f((vertex.x-dynRotCenter.x)*fR[0] + (vertex.y-dynRotCenter.y)*fR[1],
						(vertex.x-dynRotCenter.x)*fR[2] + (vertex.y-dynRotCenter.y)*fR[3]);
		transVertex += dynPos + dynRotCenter;
		
		pDrawlib->glTexCoord(m_DynamicGeoms[geom]->Polys[j]->pTexCoords[k].x,
						      m_DynamicGeoms[geom]->Polys[j]->pTexCoords[k].y);
		pDrawlib->glVertex(transVertex.x, transVertex.y);
	      }
	      pDrawlib->endDrawKeepProperties();
	    }
	    pDrawlib->removePropertiesAfterEnd();
	  }

	}
      }
      if(pDrawlib->getBackend() == DrawLib::backend_SdlGFX){
	/* Render all special edges (if quality!=low) */
	if(m_graphicsLevel != GFX_LOW) {
	  for(unsigned int i=0;i<Blocks.size();i++) {
	    if(Blocks[i]->isBackground() == bBackground){
	      _RenderBlockEdges(Blocks[i]);
	    }
	  }
	}
      }
    }

    if(XMSession::instance()->ugly() || XMSession::instance()->uglyOver()) {
      GameApp::instance()->getDrawLib()->setTexture(NULL, BLEND_MODE_NONE);
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

	pDrawlib->startDraw(DRAW_MODE_LINE_LOOP);
	pDrawlib->setColorRGB(255,255,255);

        for(unsigned int j=0;j<Blocks[i]->Vertices().size();j++) {
	  Vector2f vertex = Blocks[i]->Vertices()[j]->Position();
	  /* transform vertex */
	  Vector2f transVertex = Vector2f((vertex.x-dynRotCenter.x)*fR[0] + (vertex.y-dynRotCenter.y)*fR[1],
					  (vertex.x-dynRotCenter.x)*fR[2] + (vertex.y-dynRotCenter.y)*fR[3]);
	  transVertex += dynPos + dynRotCenter;

          pDrawlib->glVertex(transVertex.x, transVertex.y);
        }
	pDrawlib->endDraw();
      }
    }

    if(XMSession::instance()->debug() == true) {
      GameApp::instance()->getDrawLib()->setTexture(NULL, BLEND_MODE_NONE);
      for(unsigned int i=0; i<Blocks.size(); i++) {
	if(Blocks[i]->isBackground() != bBackground)
	  continue;
      
	pDrawlib->startDraw(DRAW_MODE_LINE_LOOP);
	pDrawlib->setColorRGB(0, 0, 255);

	AABB aabb = Blocks[i]->getAABB();

	pDrawlib->glVertex(aabb.getBMin().x, aabb.getBMin().y);
	pDrawlib->glVertex(aabb.getBMin().x, aabb.getBMax().y);
	pDrawlib->glVertex(aabb.getBMax().x, aabb.getBMax().y);
	pDrawlib->glVertex(aabb.getBMax().x, aabb.getBMin().y);

	pDrawlib->endDraw();

	_RenderCircle(16, MAKE_COLOR(0, 0, 255, 255),
		      Blocks[i]->getBCircle().getCenter(),
		      Blocks[i]->getBCircle().getRadius());
      }
    }
  }

  void GameRenderer::_RenderBlock(Block* block)
  {
    DrawLib* pDrawlib = GameApp::instance()->getDrawLib();
    int geom = block->getGeom();
    pDrawlib->setTexture(m_StaticGeoms[geom]->pTexture, BLEND_MODE_A);
    
    /* set flashy blendColor */
    TColor v_blendColor =  TColor(block->getBlendColor());
    pDrawlib->setColorRGBA(v_blendColor.Red(), v_blendColor.Green(), v_blendColor.Blue(), v_blendColor.Alpha());

    if(pDrawlib->getBackend() == DrawLib::backend_OpenGl) {
#ifdef ENABLE_OPENGL
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);

      /* VBO optimized? */
      if(pDrawlib->useVBOs()) {
	for(unsigned int j=0;j<m_StaticGeoms[geom]->Polys.size();j++) {          
	  GeomPoly *pPoly = m_StaticGeoms[geom]->Polys[j];

	  ((DrawLibOpenGL*)pDrawlib)->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nVertexBufferID);
	  glVertexPointer(2,GL_FLOAT,0,(char *)NULL);

	  ((DrawLibOpenGL*)pDrawlib)->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nTexCoordBufferID);
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
    } else if(pDrawlib->getBackend() == DrawLib::backend_SdlGFX){

      for(unsigned int j=0;j<m_StaticGeoms[geom]->Polys.size();j++) {
	pDrawlib->setTexture(m_StaticGeoms[geom]->pTexture,BLEND_MODE_A);
	pDrawlib->startDraw(DRAW_MODE_POLYGON);
	pDrawlib->setColorRGB(255,255,255);
	for(unsigned int k=0;k<m_StaticGeoms[geom]->Polys[j]->nNumVertices;k++) {
	  pDrawlib->glTexCoord(m_StaticGeoms[geom]->Polys[j]->pTexCoords[k].x,
						m_StaticGeoms[geom]->Polys[j]->pTexCoords[k].y);
	  pDrawlib->glVertex(m_StaticGeoms[geom]->Polys[j]->pVertices[k].x,
					      m_StaticGeoms[geom]->Polys[j]->pVertices[k].y);
	}
	pDrawlib->endDraw();
      }
    }
  }

void GameRenderer::_RenderBlockEdges(Block* pBlock)
{
  DrawLib* pDrawlib = GameApp::instance()->getDrawLib();
  if(pDrawlib->getBackend() == DrawLib::backend_OpenGl) {
    for(unsigned int i=pBlock->getEdgeGeoms().size(); i>0; i--){
      int geom = pBlock->getEdgeGeoms()[i-1];
      pDrawlib->setTexture(m_edgeGeoms[geom]->pTexture, BLEND_MODE_A);
      
      TColor v_blendColor = m_edgeGeoms[geom]->edgeBlendColor;
      pDrawlib->setColorRGBA(v_blendColor.Red(), v_blendColor.Green(), v_blendColor.Blue(), v_blendColor.Alpha());

      /* VBO optimized? */
      if(pDrawlib->useVBOs()) {
	for(unsigned int j=0;j<m_edgeGeoms[geom]->Polys.size();j++) {          
	  GeomPoly *pPoly = m_edgeGeoms[geom]->Polys[j];

	  ((DrawLibOpenGL*)pDrawlib)->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nVertexBufferID);
	  glVertexPointer(2,GL_FLOAT,0,(char *)NULL);

	  ((DrawLibOpenGL*)pDrawlib)->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nTexCoordBufferID);
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
  } else if(pDrawlib->getBackend() == DrawLib::backend_SdlGFX){
    // SDLGFX::TODO
  }
}

  /*===========================================================================
  Blocks (static)
  ===========================================================================*/
  void GameRenderer::_RenderBlocks(Scene* i_scene) {
    DrawLib* pDrawlib = GameApp::instance()->getDrawLib();

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
	if(pDrawlib->getBackend() == DrawLib::backend_SdlGFX){
	  /* Render all special edges (if quality!=low) */
	  if(m_graphicsLevel != GFX_LOW) {
	    for(unsigned int i=0;i<Blocks.size();i++) {
	      if(Blocks[i]->isBackground() == false) {
		_RenderBlockEdges(Blocks[i]);
	      }
	    }
	  }
	}
      }

      if(XMSession::instance()->ugly()) {
	GameApp::instance()->getDrawLib()->setTexture(NULL, BLEND_MODE_NONE);
	for(unsigned int i=0;i<Blocks.size();i++) {
	  if(Blocks[i]->isBackground() == false) {
	    pDrawlib->startDraw(DRAW_MODE_LINE_LOOP);
	    pDrawlib->setColorRGB(255,255,255);
	    for(unsigned int j=0;j<Blocks[i]->Vertices().size();j++) {
	      pDrawlib->glVertex(Blocks[i]->Vertices()[j]->Position().x + Blocks[i]->DynamicPosition().x,
				 Blocks[i]->Vertices()[j]->Position().y + Blocks[i]->DynamicPosition().y);
	    }
	    pDrawlib->endDraw();
	  }
	}
      }

      if(XMSession::instance()->uglyOver()) {
	GameApp::instance()->getDrawLib()->setTexture(NULL, BLEND_MODE_NONE);
	for(unsigned int i=0; i<Blocks.size(); i++) {
	  for(unsigned int j=0; j<Blocks[i]->ConvexBlocks().size(); j++) {

	    pDrawlib->startDraw(DRAW_MODE_LINE_LOOP);

	    if(Blocks[i]->isBackground() == false) {
	      pDrawlib->setColorRGB(255,255,0);
	    } else {
	      pDrawlib->setColorRGB(255,255,255);
	    }
	    for(unsigned int k=0; k<Blocks[i]->ConvexBlocks()[j]->Vertices().size(); k++) {
	      pDrawlib->glVertex(Blocks[i]->ConvexBlocks()[j]->Vertices()[k]->Position().x + Blocks[i]->DynamicPosition().x,
						  Blocks[i]->ConvexBlocks()[j]->Vertices()[k]->Position().y + Blocks[i]->DynamicPosition().y);
	    }
	    pDrawlib->endDraw();

	  }
	}
      }
    }
  }

  void GameRenderer::_RenderZone(Zone *i_zone, bool i_renderOnlyDeathZone) {
    ZonePrim *v_prim;
    ZonePrimBox *v_primbox;

    GameApp::instance()->getDrawLib()->setTexture(NULL, BLEND_MODE_NONE);

    if(i_renderOnlyDeathZone && !(i_zone->isDeathZone())) {
      return;
    }
    
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
void GameRenderer::_RenderSky(Scene* i_scene, float i_zoom, float i_offset, const TColor& i_color,
			      float i_driftZoom, const TColor& i_driftColor, bool i_drifted) {
  DrawLib* pDrawlib = GameApp::instance()->getDrawLib();
  TextureSprite* pType;
  TextureSprite* pType2;
  float fDrift = 0.0;
  float uZoom = 1.0 / i_zoom;
  float uDriftZoom = 1.0 / i_driftZoom;

  if(m_graphicsLevel != GFX_HIGH) {
    i_drifted = false;
  }

  pType = (TextureSprite*) Theme::instance()->getSprite(SPRITE_TYPE_TEXTURE,
							i_scene->getLevelSrc()->Sky()->Texture());
  pType2= (TextureSprite*) Theme::instance()->getSprite(SPRITE_TYPE_TEXTURE,
                                                        i_scene->getLevelSrc()->Sky()->BlendTexture());
  
  
  if(pType != NULL && pType2 != NULL) {
    if(i_drifted) {
      pDrawlib->setTexture(pType->getTexture(), BLEND_MODE_A);
//      pDrawlib->setTexture(pType->getTexture(), BLEND_MODE_B);
    }
    
    pDrawlib->setTexture(pType->getTexture(),BLEND_MODE_NONE);
    pDrawlib->startDraw(DRAW_MODE_POLYGON);
    pDrawlib->setColorRGBA(i_color.Red() , i_color.Green(), i_color.Blue(), i_color.Alpha());
    
    if(i_drifted) {
      fDrift = GameApp::instance()->getXMTime() / 25.0f;
    }
    
    pDrawlib->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset+ fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset);
    pDrawlib->glVertexSP(0,0);
    pDrawlib->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset+uZoom+ fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset);
    pDrawlib->glVertexSP(pDrawlib->getDispWidth(),0);
    pDrawlib->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset+uZoom+ fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset+uZoom);
    pDrawlib->glVertexSP(pDrawlib->getDispWidth(),pDrawlib->getDispHeight());
    pDrawlib->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset+ fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset+uZoom);
    pDrawlib->glVertexSP(0,pDrawlib->getDispHeight());
    pDrawlib->endDraw();
    
    if(i_drifted) {
      pDrawlib->setTexture(pType2->getTexture(),BLEND_MODE_B);
      pDrawlib->startDraw(DRAW_MODE_POLYGON);
      pDrawlib->setColorRGBA(i_driftColor.Red(), i_driftColor.Green(), i_driftColor.Blue(), i_driftColor.Alpha());
      fDrift = GameApp::instance()->getXMTime() / 15.0f;
      pDrawlib->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset + fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset);
      pDrawlib->glVertexSP(0,0);
      pDrawlib->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset+uDriftZoom + fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset);
      pDrawlib->glVertexSP(pDrawlib->getDispWidth(),0);
      pDrawlib->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset+uDriftZoom + fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset+uDriftZoom);
      pDrawlib->glVertexSP(pDrawlib->getDispWidth(),pDrawlib->getDispHeight());
      pDrawlib->glTexCoord(i_scene->getCamera()->getCameraPositionX()*i_offset + fDrift,-i_scene->getCamera()->getCameraPositionY()*i_offset+uDriftZoom);
      pDrawlib->glVertexSP(0,pDrawlib->getDispHeight());
      pDrawlib->endDraw();
    }
  } else {
    LogWarning(std::string("Invalid sky " + i_scene->getLevelSrc()->Sky()->Texture()).c_str());
    pDrawlib->clearGraphics();
  }
 }

  /*===========================================================================
  And background rendering
  ===========================================================================*/
  void GameRenderer::_RenderBackground(Scene* i_scene) { 
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
      if(m_graphicsLevel != GFX_LOW) {
	for(unsigned int i=0;i<Blocks.size();i++) {
	  if(Blocks[i]->isBackground() == true) {
	    _RenderBlockEdges(Blocks[i]);
	  }
	}
      }
    }
  }

void GameRenderer::_RenderLayer(Scene* i_scene, int layer) {
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

void GameRenderer::_RenderLayers(Scene* i_scene, bool renderFront) { 
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
    DrawLib* pDrawlib = GameApp::instance()->getDrawLib();
    pDrawlib->setTexture(NULL, BLEND_MODE_NONE);

    for(unsigned int i=0;i<m_DebugInfo.size();i++) {
      if(m_DebugInfo[i]->Type == "@WHITEPOLYGONS") {
	pDrawlib->startDraw(DRAW_MODE_LINE_LOOP);
	pDrawlib->setColorRGB(255,255,255);
        for(unsigned int j=0;j<m_DebugInfo[i]->Args.size()/2;j++) {
          float x = atof(m_DebugInfo[i]->Args[j*2].c_str());
          float y = atof(m_DebugInfo[i]->Args[j*2+1].c_str());
          pDrawlib->glVertexSP(400 + x*10,300 - y*10);
        }
	pDrawlib->endDraw();
      }
      else if(m_DebugInfo[i]->Type == "@REDLINES") {
	pDrawlib->startDraw(DRAW_MODE_LINE_STRIP);
	pDrawlib->setColorRGB(255,0,0);
        for(unsigned int j=0;j<m_DebugInfo[i]->Args.size()/2;j++) {
          float x = atof(m_DebugInfo[i]->Args[j*2].c_str());
          float y = atof(m_DebugInfo[i]->Args[j*2+1].c_str());
          pDrawlib->glVertexSP(400 + x*10,300 - y*10);
        }
	pDrawlib->endDraw();
      }
      else if(m_DebugInfo[i]->Type == "@GREENLINES") {
	pDrawlib->startDraw(DRAW_MODE_LINE_STRIP);
	pDrawlib->setColorRGB(0,255,0);
        for(unsigned int j=0;j<m_DebugInfo[i]->Args.size()/2;j++) {
          float x = atof(m_DebugInfo[i]->Args[j*2].c_str());
          float y = atof(m_DebugInfo[i]->Args[j*2+1].c_str());
          pDrawlib->glVertexSP(400 + x*10,300 - y*10);
        }
	pDrawlib->endDraw();
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
	m_replayHelp_l = GAMETEXT_REPLAYHELPTEXT_L;
	m_replayHelp_r = GAMETEXT_REPLAYHELPTEXT_R(std::string("> 10"));
      } else if(p_speed <= -10.0) {
	m_replayHelp_l = GAMETEXT_REPLAYHELPTEXT_L;
	m_replayHelp_r = GAMETEXT_REPLAYHELPTEXT_R(std::string("< -10"));
      } else {
	char v_speed_str[5 + 1];
	snprintf(v_speed_str, 5+1, "% .2f", p_speed);
	m_replayHelp_l = GAMETEXT_REPLAYHELPTEXT_L;
	m_replayHelp_r = GAMETEXT_REPLAYHELPTEXT_R(std::string(v_speed_str));
      }
    } else {
      if(p_speed >= 10.0) {
	m_replayHelp_l = GAMETEXT_REPLAYHELPTEXTNOREWIND_L;
	m_replayHelp_r = GAMETEXT_REPLAYHELPTEXTNOREWIND_R(std::string("> 10"));
      } else if(p_speed <= -10.0) {
	m_replayHelp_l = GAMETEXT_REPLAYHELPTEXTNOREWIND_L;
	m_replayHelp_r = GAMETEXT_REPLAYHELPTEXTNOREWIND_R(std::string("< -10"));
      } else {
	char v_speed_str[256];
	snprintf(v_speed_str, 256, "% .2f", p_speed);
	m_replayHelp_l = GAMETEXT_REPLAYHELPTEXTNOREWIND_L;
	m_replayHelp_r = GAMETEXT_REPLAYHELPTEXTNOREWIND_R(std::string(v_speed_str));
      }
    }
  }

  void GameRenderer::hideReplayHelp() {
    m_replayHelp_l = "";
    m_replayHelp_r = "";
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
  
void GameRenderer::_RenderInGameText(Vector2f P,const std::string &Text,Color c, float i_xcentering, float i_ycentering) {
    DrawLib* pDrawlib = GameApp::instance()->getDrawLib();
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
      float vx = ((float)pDrawlib->getDispWidth() * x);
      float vy = ((float)pDrawlib->getDispHeight() * y);
#ifdef ENABLE_OPENGL
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      glOrtho(0,pDrawlib->getDispWidth(),0,pDrawlib->getDispHeight(),-1,1);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
#endif
      
      FontManager* v_fm = pDrawlib->getFontSmall();
      FontGlyph* v_fg = v_fm->getGlyph(Text);
      v_fm->printString(v_fg, (int)(vx - (v_fg->realWidth() * i_xcentering)), (int)(vy - (v_fg->realHeight() * i_ycentering)), c, 0.0, true);

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
    DrawLib* pDrawlib = GameApp::instance()->getDrawLib();
    pDrawlib->startDraw(DRAW_MODE_LINE_STRIP);
    pDrawlib->setColor(i_color);
    pDrawlib->glVertex(i_p1);
    pDrawlib->glVertex(Vector2f(i_p2.x, i_p1.y));
    pDrawlib->glVertex(i_p2);
    pDrawlib->glVertex(Vector2f(i_p1.x, i_p2.y));
    pDrawlib->glVertex(i_p1);
    pDrawlib->endDraw();
  }

  void GameRenderer::_RenderCircle(unsigned int nSteps,Color CircleColor,const Vector2f &C,float fRadius) {
    DrawLib* pDrawlib = GameApp::instance()->getDrawLib();
    pDrawlib->startDraw(DRAW_MODE_LINE_LOOP);
    pDrawlib->setColor(CircleColor);
    for(unsigned int i=0;i<nSteps;i++) {
      float r = (3.14159f * 2.0f * (float)i)/ (float)nSteps;            
      pDrawlib->glVertex( Vector2f(C.x + fRadius*sinf(r),C.y + fRadius*cosf(r)) );
    }      
    pDrawlib->endDraw();
  }
  
  void GameRenderer::_RenderScreenShadow(Scene* i_scene) {
    DrawLib* pDrawLib = GameApp::instance()->getDrawLib();
    Camera* i_camera = i_scene->getCamera();
    
    /* Screen shadow creates the dark overlay, which is put onto the camera on player death.
       For Multiplayer a special handling is needed, so multiple queries must be done */
    
    bool v_doShade, v_doShadeAnim;
    float v_nShadeTime;
      
    if(m_doShade_global) {
      v_doShade = m_doShade_global;
      v_doShadeAnim = m_doShadeAnim_global;
      v_nShadeTime = m_nShadeTime_global;
      if(XMSession::instance()->multiNbPlayers() > 1 && i_camera->getPlayerToFollow()->isDead() && 
        GameApp::getXMTime() > i_camera->getShadeTime()+MENU_SHADING_TIME ) {
        v_doShadeAnim = false;
      }
    }
    else {
      v_doShade = i_camera->getDoShade();
      v_doShadeAnim = i_camera->getDoShadeAnim();
      v_nShadeTime = i_camera->getShadeTime();
    }

    // shade
    if(XMSession::instance()->ugly() == false && v_doShade) {
      float v_currentTime = GameApp::getXMTime();
      int   v_nShade;
    
      if(v_currentTime - v_nShadeTime < MENU_SHADING_TIME && v_doShadeAnim) {
        v_nShade = (int ) ((v_currentTime - v_nShadeTime) * (MENU_SHADING_VALUE / MENU_SHADING_TIME));
      } else {
        v_nShade = MENU_SHADING_VALUE;
      }
      pDrawLib->drawBox(Vector2f(0.0,0.0),
                        Vector2f(float(pDrawLib->getDispWidth()), //pDrawlib returns camera specific values, as set in render()
			         float(pDrawLib->getDispHeight())), 
		        0, MAKE_COLOR(0,0,0, v_nShade));  
    }
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

void GameRenderer::setShowGhostsText(bool i_value) {
  m_showGhostsText = i_value;
}

bool GameRenderer::showGhostsText() const {
  return m_showGhostsText;
}

  void GameRenderer::switchFollow(Scene* i_scene) {
    Camera*  pCamera  = i_scene->getCamera();

    if(pCamera->getPlayerToFollow() == NULL)
      return;

    std::vector<Biker*>& players = i_scene->Players();
    std::vector<Ghost*>& ghosts = i_scene->Ghosts();
    unsigned int sizePlayers = players.size();
    unsigned int sizeGhosts  = ghosts.size();

    /* search into the player */
    for(unsigned i=0; i<sizePlayers; i++) {
      if(players[i] == pCamera->getPlayerToFollow()) {
	if(i < sizePlayers-1) {
	  pCamera->setPlayerToFollow(players[i+1]);
	} else {
	  if(sizeGhosts > 0) {
	    pCamera->setPlayerToFollow(ghosts[0]);
	  } else {
	    pCamera->setPlayerToFollow(players[0]);
	  }
	}
	return;
      }
    }

    /* search into the ghost */
    for(unsigned i=0; i<sizeGhosts; i++) {
      if(ghosts[i] == pCamera->getPlayerToFollow()) {
	if(i < sizeGhosts-1) {
	  pCamera->setPlayerToFollow(ghosts[i+1]);
	} else {
	  if(sizePlayers > 0) {
	    pCamera->setPlayerToFollow(players[0]);
	  } else {
	    pCamera->setPlayerToFollow(ghosts[0]);
	  }
	}
	return;
      }
    }
  }


void GameRenderer::renderTimePanel(Scene* i_scene) {
  int x = 0;
  int y = 0;
  FontGlyph* v_fg;

  // do not render it if it's the autozoom camera or ...
  if(i_scene->isAutoZoomCamera() == true){
    return;
  }

  DrawLib* pDrawlib = GameApp::instance()->getDrawLib();
  unsigned int width  = pDrawlib->getDispWidth();
  unsigned int height = pDrawlib->getDispHeight();

  Biker* pBiker = i_scene->getCamera()->getPlayerToFollow();
  Vector2i bottomLeft = i_scene->getCamera()->getDispBottomLeft();

  /* render game time */
  FontManager* v_fm = pDrawlib->getFontMedium();

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

  //seek correct drawing positions
  if((unsigned int) bottomLeft.x != 0) {
      x=width/2; 
  }
  if((unsigned int) bottomLeft.y != height) {
      y=height/2;
  }

  v_fm->printString(v_fg,
		    x,y,
		    MAKE_COLOR(255,255,255,255), -1.0, true);

  /* next things must be rendered only by the first camera */
  if((unsigned int) bottomLeft.x != 0 || (unsigned int) bottomLeft.y != height) {
    return;
  }
  
  v_fm = pDrawlib->getFontSmall();

  v_fg = v_fm->getGlyph(m_bestTime);
  v_fm->printString(v_fg, x, y+28, MAKE_COLOR(255,255,255,255), -1.0, true);

  v_fg = v_fm->getGlyph(m_worldRecordTime);
  v_fm->printString(v_fg, x, y+48, MAKE_COLOR(255,255,255,255), -1.0, true);
}

void GameRenderer::renderReplayHelpMessage(Scene* i_scene) {
  /* next things must be rendered only the first camera */
  if(i_scene->getCurrentCamera() != 0)
    return;

  FontManager* v_fm = GameApp::instance()->getDrawLib()->getFontSmall();
  FontGlyph* v_fg_l = v_fm->getGlyph(m_replayHelp_l);
  FontGlyph* v_fg_r = v_fm->getGlyph(m_replayHelp_r);
  
  // a nice Box
  int v_displayWidth = GameApp::instance()->getDrawLib()->getDispWidth(),
      posX = v_displayWidth - v_fg_r->realWidth(),
      posY = 5;
  // but just in case there is replayHelp displayed
  if(m_replayHelp_l != "") {  
    Vector2f A(posX - v_fg_l->realWidth() - 20, posY - 5), B(v_displayWidth, v_fg_r->realHeight() + posY + 7);
    GameApp::instance()->getDrawLib()->drawBox(A,B,1,MAKE_COLOR(0,0,0,118),MAKE_COLOR(255,255,255,255));
  }
  v_fm->printString(v_fg_l,
		    posX - v_fg_l->realWidth() - 12, //give some space between left and right part
		    posY,
		    MAKE_COLOR(255,255,255,255), -1.0, true);
  
  v_fm->printString(v_fg_r,
		    posX - 7, //give some space to right side of screen
		    posY,
		    MAKE_COLOR(255,255,255,255), -1.0, true);
}

void GameRenderer::_RenderParticleDraw(Vector2f position, Texture* pTexture, float fSize, float fAngle, TColor c)
{
  /* Render single particle */
  m_nParticlesRendered++;

  Vector2f p1(1,0), p2(1,0), p3(1,0), p4(1,0);
  p1.rotateXY(fAngle);
  p2.rotateXY(90+fAngle);
  p3.rotateXY(180+fAngle);
  p4.rotateXY(270+fAngle);

  p1 = position + p1 * fSize;
  p2 = position + p2 * fSize;
  p3 = position + p3 * fSize;
  p4 = position + p4 * fSize;

  GameApp::instance()->getDrawLib()->drawImageTextureSet(p1, p2, p3, p4, MAKE_COLOR(c.Red(), c.Green(), c.Blue(), c.Alpha()), false, true);
}

void GameRenderer::_RenderParticle(Scene* i_scene, ParticlesSource* i_source, unsigned int sprite)
{
  std::vector<EntityParticle*>& v_particles = i_source->Particles();
  unsigned int size = v_particles.size();

  for(unsigned j = 0; j < size; j++) {
    EntityParticle* v_particle = v_particles[j];
    if(v_particle->spriteIndex() == sprite) {
      _RenderParticleDraw(v_particle->DynamicPosition(),
			  NULL,
			  v_particle->Size(),
			  v_particle->Angle(),
			  v_particle->Color());
    }
  }
}
  
void GameRenderer::_RenderParticles(Scene* i_scene, bool bFront) {
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
      std::vector<Entity*> Entities         = i_scene->getCollisionHandler()->getEntitiesNearPosition(screenBigger);
      std::vector<Entity*> ExternalEntities = i_scene->getLevelSrc()->EntitiesExterns();
      std::vector<Entity*>* allEntities[]   = {&Entities, &ExternalEntities, NULL};

      for(unsigned int j=0; allEntities[j] != NULL; j++){
	std::vector<ParticlesSource*> particleSources;
	unsigned int size = allEntities[j]->size();

	// keep only particle sources
	for(unsigned int i = 0; i < size; i++) {
	  Entity* v_entity = (*(allEntities[j]))[i];
	  if(v_entity->Speciality() == ET_PARTICLES_SOURCE
	     && (v_entity->Z() >= 0.0) == bFront) {
	    particleSources.push_back((ParticlesSource*)v_entity);
	  }
	}
	// sort by particle type
	std::sort(particleSources.begin(), particleSources.end(),
		  AscendingParticleSourceSort());

	// draw them
	unsigned int index = 0;
	size = particleSources.size();

	if(index < size && particleSources[index]->getType() == Smoke) {
	  // smoke1
	  EffectSprite* pSmoke1Type = (EffectSprite*) ((ParticlesSourceSmoke*)particleSources[index])->getSprite(0);
	  if(pSmoke1Type != NULL) {
	    GameApp::instance()->getDrawLib()->setTexture(pSmoke1Type->getTexture(), BLEND_MODE_A);

	    for(; index < size; index++) {
	      if(particleSources[index]->getType() != Smoke)
		break;
	      _RenderParticle(i_scene, particleSources[index]);
	    }
	  } else {
	    for(; index < size; index++) {
	      if(particleSources[index]->getType() != Smoke)
		break;
	    }
	  }

	  index = 0;
	  // smoke2
	  EffectSprite* pSmoke2Type = (EffectSprite*) ((ParticlesSourceSmoke*)particleSources[index])->getSprite(1);
	  if(pSmoke2Type != NULL) {
	    GameApp::instance()->getDrawLib()->setTexture(pSmoke2Type->getTexture(), BLEND_MODE_A);

	    for(; index < size; index++) {
	      if(particleSources[index]->getType() != Smoke)
		break;
	      _RenderParticle(i_scene, particleSources[index], 1);
	    }
	  } else {
	    for(; index < size; index++) {
	      if(particleSources[index]->getType() != Smoke)
		break;
	    }
	  }
	}

	if(index < size && particleSources[index]->getType() == Fire) {
	  // fire
	  EffectSprite* pFireType = (EffectSprite*) particleSources[index]->getSprite();
	  if(pFireType != NULL) {
	    GameApp::instance()->getDrawLib()->setTexture(pFireType->getTexture(), BLEND_MODE_A);

	    for(; index < size; index++) {
	      if(particleSources[index]->getType() != Fire)
		break;
	      _RenderParticle(i_scene, particleSources[index]);
	    }
	  } else {
	    for(; index < size; index++) {
	      if(particleSources[index]->getType() != Fire)
		break;
	    }	
	  }
	}

	if(index < size && particleSources[index]->getType() == Star) {
	  // star
	  AnimationSprite* pStarAnimation = (AnimationSprite*)i_scene->getLevelSrc()->starSprite();
	  if(pStarAnimation != NULL) {
	    GameApp::instance()->getDrawLib()->setTexture(pStarAnimation->getTexture(), BLEND_MODE_A);	

	    for(; index < size; index++) {
	      if(particleSources[index]->getType() != Star)
		break;
	      _RenderParticle(i_scene, particleSources[index]);
	    }
	  } else {
	    for(; index < size; index++) {
	      if(particleSources[index]->getType() != Star)
		break;
	    }	
	  }
	}

	if(index < size && particleSources[index]->getType() == Debris) {
	  // debris
	  EffectSprite* pDebrisType;
	  pDebrisType = (EffectSprite*) particleSources[index]->getSprite();
	  if(pDebrisType == NULL)
	    pDebrisType = (EffectSprite*) Theme::instance()->getSprite(SPRITE_TYPE_EFFECT, "Debris1");
	  if(pDebrisType != NULL) {
	    GameApp::instance()->getDrawLib()->setTexture(pDebrisType->getTexture(), BLEND_MODE_A);

	    for(; index < size; index++) {
	      if(particleSources[index]->getType() != Debris)
		break;
	      _RenderParticle(i_scene, particleSources[index]);
	    }
	  }
	}

	if(index < size && particleSources[index]->getType() == Sparkle) {
	  // sparkle
	  EffectSprite* pSparkleType = (EffectSprite*) particleSources[index]->getSprite();
	  if(pSparkleType != NULL) {
	    GameApp::instance()->getDrawLib()->setTexture(pSparkleType->getTexture(), BLEND_MODE_A);

	    for(; index < size; index++) {
	      if(particleSources[index]->getType() != Sparkle)
		break;
	      _RenderParticle(i_scene, particleSources[index]);
	    }
	  } else {
	    for(; index < size; index++) {
	      if(particleSources[index]->getType() != Sparkle)
		break;
	    }	
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
				    Sprite* i_sprite,
				    const TColor& i_filterColor,
				    Biker* i_biker,
				    int i_90_rotation
				    ) {
    Texture *pTexture;
    Vector2f Sv;
    Vector2f p0, p1, p2, p3;

    if(i_sprite == NULL)
      return;
    pTexture = i_sprite->getTexture(false, false, FM_LINEAR); // FM_LINEAR
    if(pTexture == NULL)
      return;

    Sv = i_from - i_to;
    Sv.normalize();

    if(i_biker->getState()->Dir == DD_RIGHT) {
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

    p0 = calculateChangeDirPosition(i_biker,p0);
    p1 = calculateChangeDirPosition(i_biker,p1);
    p2 = calculateChangeDirPosition(i_biker,p2);
    p3 = calculateChangeDirPosition(i_biker,p3);
    
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
  void GameRenderer::_RenderBike(Biker* i_biker, bool i_renderBikeFront,
				 const TColor&  i_filterColor, const TColor&  i_filterUglyColor) {
    BikeState*      pBike      = i_biker->getState();
    BikeParameters* pBikeParms = pBike->Parameters();
    BikerTheme*     p_theme    = i_biker->getBikeTheme();

    Sprite*  pSprite;
    Texture* pTexture;
    DrawLib* pDrawlib = GameApp::instance()->getDrawLib();

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
      pDrawlib->setTexture(NULL, BLEND_MODE_NONE);
      pDrawlib->startDraw(DRAW_MODE_LINE_STRIP);
      pDrawlib->setColor(p_theme->getUglyWheelColor());
      pDrawlib->glVertex(p0+C);    
      pDrawlib->glVertex(p2+C);
      pDrawlib->endDraw();
      pDrawlib->startDraw(DRAW_MODE_LINE_STRIP);
      pDrawlib->setColor(p_theme->getUglyWheelColor());
      pDrawlib->glVertex(p1+C);
      pDrawlib->glVertex(p3+C);
      pDrawlib->endDraw();
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
      pDrawlib->setTexture(NULL, BLEND_MODE_NONE);
      pDrawlib->startDraw(DRAW_MODE_LINE_STRIP);
      pDrawlib->setColor(p_theme->getUglyWheelColor());
      pDrawlib->glVertex(p0+C);    
      pDrawlib->glVertex(p2+C);
      pDrawlib->endDraw();
      pDrawlib->startDraw(DRAW_MODE_LINE_STRIP);
      pDrawlib->setColor(p_theme->getUglyWheelColor());
      pDrawlib->glVertex(p1+C);
      pDrawlib->glVertex(p3+C);
      pDrawlib->endDraw();
      _RenderCircle(16,p_theme->getUglyWheelColor(),C,pBikeParms->WR);
    }

    if(!XMSession::instance()->ugly()) {
      /* Draw swing arm */
      if(pBike->Dir == DD_RIGHT) {       
        Sv = pBike->SwingAnchorP - Rc;
	    Sv.normalize();
        
        p0 = pBike->RearWheelP + Vector2f(-Sv.y,Sv.x)*0.07f - Sv*0.08f ;
        p1 = pBike->SwingAnchorP + Vector2f(-Sv.y,Sv.x)*0.07f ;
        p2 = pBike->SwingAnchorP - Vector2f(-Sv.y,Sv.x)*0.07f ;
        p3 = pBike->RearWheelP - Vector2f(-Sv.y,Sv.x)*0.07f - Sv*0.08f ;
      }
      else {
        Sv = pBike->SwingAnchor2P - Fc;
	    Sv.normalize();
        
        p0 = pBike->FrontWheelP + Vector2f(-Sv.y,Sv.x)*0.07f - Sv*0.08f;
        p1 = pBike->SwingAnchor2P + Vector2f(-Sv.y,Sv.x)*0.07f;
        p2 = pBike->SwingAnchor2P - Vector2f(-Sv.y,Sv.x)*0.07f;
        p3 = pBike->FrontWheelP - Vector2f(-Sv.y,Sv.x)*0.07f - Sv*0.08f;
      }        
      
      p0 = calculateChangeDirPosition(i_biker,p0);
      p1 = calculateChangeDirPosition(i_biker,p1);
      p2 = calculateChangeDirPosition(i_biker,p2);
      p3 = calculateChangeDirPosition(i_biker,p3);

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
       
        p0 = pBike->FrontWheelP + Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.05f ;
        p1 = pBike->FrontAnchorP + Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.35f ;
        p2 = pBike->FrontAnchorP - Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.35f ;
        p3 = pBike->FrontWheelP - Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.05f ;
      }
      else {
        Sv = pBike->FrontAnchor2P - Rc;
	    Sv.normalize();
      
        p0 = pBike->RearWheelP + Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.05f ;
        p1 = pBike->FrontAnchor2P + Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.35f ;
        p2 = pBike->FrontAnchor2P - Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.35f ;
        p3 = pBike->RearWheelP - Vector2f(-Sv.y,Sv.x)*0.04f - Sv*0.05f ;
      }
      
      p0 = calculateChangeDirPosition(i_biker,p0);
      p1 = calculateChangeDirPosition(i_biker,p1);
      p2 = calculateChangeDirPosition(i_biker,p2);
      p3 = calculateChangeDirPosition(i_biker,p3);

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
      
      o0 = Vector2f(-1.0, 0.5);
      o1 = Vector2f(1.0, 0.5);
      o2 = Vector2f(1.0,-0.5);
      o3 = Vector2f(-1.0,-0.5);
      C = pBike->CenterP; 
      
      p0 = C + Vector2f(o0.x*pBike->fFrameRot[0] + o0.y*pBike->fFrameRot[1],
                        o0.x*pBike->fFrameRot[2] + o0.y*pBike->fFrameRot[3]);
      p1 = C + Vector2f(o1.x*pBike->fFrameRot[0] + o1.y*pBike->fFrameRot[1],
                        o1.x*pBike->fFrameRot[2] + o1.y*pBike->fFrameRot[3]);
      p2 = C + Vector2f(o2.x*pBike->fFrameRot[0] + o2.y*pBike->fFrameRot[1],
                        o2.x*pBike->fFrameRot[2] + o2.y*pBike->fFrameRot[3]);
      p3 = C + Vector2f(o3.x*pBike->fFrameRot[0] + o3.y*pBike->fFrameRot[1],
                        o3.x*pBike->fFrameRot[2] + o3.y*pBike->fFrameRot[3]);
      
      p0 = calculateChangeDirPosition(i_biker,p0);
      p1 = calculateChangeDirPosition(i_biker,p1);
      p2 = calculateChangeDirPosition(i_biker,p2);
      p3 = calculateChangeDirPosition(i_biker,p3);

      pSprite = p_theme->getBody();
      if(pSprite != NULL) {
	    pTexture = pSprite->getTexture(false, false, FM_LINEAR);
	      if(pTexture != NULL) {
	        if(pBike->Dir == DD_RIGHT) {
	          _RenderAlphaBlendedSection(pTexture,p3,p2,p1,p0, i_filterColor);
	        } else {
	          _RenderAlphaBlendedSection(pTexture,p2,p3,p0,p1, i_filterColor);
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
		     i_biker
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
		     i_biker, 1
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
		     i_biker
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
		     i_biker
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
		     i_biker, 2
		     );

    }

    if(pBike->Dir == DD_RIGHT) {
      if(XMSession::instance()->ugly() || XMSession::instance()->testTheme()) {
        /* Draw it ugly */
	pDrawlib->setTexture(NULL, BLEND_MODE_NONE);
	pDrawlib->startDraw(DRAW_MODE_LINE_STRIP);
	pDrawlib->setColor(i_filterUglyColor.getColor());
        pDrawlib->glVertex(pBike->FootP);
        pDrawlib->glVertex(pBike->KneeP);
        pDrawlib->glVertex(pBike->LowerBodyP);
        pDrawlib->glVertex(pBike->ShoulderP);
        pDrawlib->glVertex(pBike->ElbowP);
        pDrawlib->glVertex(pBike->HandP);
	pDrawlib->endDraw();
        _RenderCircle(10, i_filterUglyColor.getColor(),
		      pBike->HeadP, pBikeParms->fHeadSize);
      }
    }
    else if(pBike->Dir == DD_LEFT) {
      if(XMSession::instance()->ugly() || XMSession::instance()->testTheme()) {
        /* Draw it ugly */
	pDrawlib->setTexture(NULL, BLEND_MODE_NONE);
	pDrawlib->startDraw(DRAW_MODE_LINE_STRIP);
	pDrawlib->setColor(i_filterUglyColor.getColor());
        pDrawlib->glVertex(pBike->Foot2P);
        pDrawlib->glVertex(pBike->Knee2P);
        pDrawlib->glVertex(pBike->LowerBody2P);
        pDrawlib->glVertex(pBike->Shoulder2P);
        pDrawlib->glVertex(pBike->Elbow2P);
        pDrawlib->glVertex(pBike->Hand2P);
        pDrawlib->endDraw();
        _RenderCircle(10, i_filterUglyColor.getColor(),
		      pBike->Head2P,pBikeParms->fHeadSize);
      }
    }   

		// draw the center
    if(XMSession::instance()->debug()) {
      pDrawlib->setTexture(NULL, BLEND_MODE_NONE);
      _RenderCircle(10, MAKE_COLOR(255, 0, 0, 255), pBike->CenterP, 0.2);
    }
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

  m_xScale = pCamera->getCurrentZoom() * ((float)pCamera->getDispHeight()) / pCamera->getDispWidth();  //sets correct screen aspect ratio
  m_yScale = pCamera->getCurrentZoom();

  // depends on zoom
  float xCamOffset = 1.0 / m_xScale;
  float yCamOffset = 1.0 / m_yScale;

  //screenBBox must be transformed by the angle of the camera
  float a=pCamera->rotationAngle();
  if(a<0){  //prevent negative angles, which are supplied by various getCamera* functions
    a=-a;
  }
  float r=(sqrt(pow(xCamOffset,2)+pow(yCamOffset,2))); //radius of screen edge remains constant, no matter which angle!
  float alpha=asin(yCamOffset/r);
  float alpha2=asin(-yCamOffset/r);
  float newYCamOffset1=sin(alpha+a) *r;
  float newXCamOffset1=cos(alpha+a) *r;
  float newYCamOffset2=sin(alpha2+a) *r;
  float newXCamOffset2=cos(alpha2+a) *r;
  
  float newXCamOffset=newXCamOffset1, 
        newYCamOffset=newYCamOffset2; 
  if(a<PI/2 || (a > PI && a < 1.5*PI)) {
    newXCamOffset=-newXCamOffset2;
    newYCamOffset=-newYCamOffset1;
  }
  Vector2f v1=Vector2f(pCamera->getCameraPositionX()-newXCamOffset, pCamera->getCameraPositionY()-newYCamOffset);
  Vector2f v2=Vector2f(pCamera->getCameraPositionX()+newXCamOffset, pCamera->getCameraPositionY()+newYCamOffset);

  bbox.addPointToAABB2f(v1);
  bbox.addPointToAABB2f(v2);
}

void GameRenderer::beginTexturesRegistration()
{
  // as we will remove textures, the current texture in drawlib can
  // became invalid -> set it to NULL
  GameApp::instance()->getDrawLib()->setTexture(NULL, BLEND_MODE_NONE);

  m_curRegistrationStage++;

  // zero is for persistent textures
  if(m_curRegistrationStage == PERSISTANT)
    m_curRegistrationStage++;

  if(XMSession::instance()->debug() == true) {
    LogDebug("---Begin texture registration---");
    std::vector<Texture*> textures = Theme::instance()->getTextureManager()->getTextures();
    std::vector<Texture*>::iterator it = textures.begin();
    while(it != textures.end()){
      if((*it)->curRegistrationStage != PERSISTANT)
	LogDebug("  begin %s %d [%x]", (*it)->Name.c_str(), (*it)->curRegistrationStage, (*it));
      ++it;
    }
    LogDebug("---Begin texture registration---");
  }
}

void GameRenderer::endTexturesRegistration()
{
  // remove not used textures
  std::vector<Texture*> textures = Theme::instance()->getTextureManager()->getTextures();
  std::vector<Texture*>::iterator it = textures.begin();

  while(it != textures.end()){
    // zero is for persistent textures
    if((*it)->curRegistrationStage != PERSISTANT)
      if((*it)->curRegistrationStage != m_curRegistrationStage){
	LogDebug("remove texture [%s] [%x]", (*it)->Name.c_str(), (*it));

	(*it)->invalidateSpritesTexture();

	Theme::instance()->getTextureManager()->destroyTexture((*it));
	it = textures.erase(it);
      } else
	++it;
    else
      ++it;
  }

  if(XMSession::instance()->debug() == true) {
    LogDebug("---End texture registration---");
    it = textures.begin();
    while(it != textures.end()){
      if((*it)->curRegistrationStage != PERSISTANT)
	LogDebug("  end %s %d [%x]", (*it)->Name.c_str(), (*it)->curRegistrationStage, (*it));
      ++it;
    }
    LogDebug("---End texture registration---");
  }
}

unsigned int GameRenderer::currentRegistrationStage() const
{
  return m_curRegistrationStage;
}

void GameRenderer::setScreenShade(bool i_doShade_global, bool i_doShadeAnim_global, float i_shadeTime_global) 
{ 
  m_doShade_global = i_doShade_global; 
  m_doShadeAnim_global = i_doShadeAnim_global;
  m_nShadeTime_global = i_shadeTime_global; 
}

void GameRenderer::renderGameMessages(Scene* i_scene) {
  /* this is implemented as a non-private function to make game message callable from elsewhere
     (needed for multiplayer, where cams may cover screenwide text displays) */
  _RenderGameMessages(i_scene, true);
}
