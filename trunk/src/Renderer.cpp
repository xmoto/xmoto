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

/* 
 *  In-game rendering
 */
#include "VXml.h"
#include "VFileIO.h"
#include "MotoGame.h"
#include "Renderer.h"
#include "GameText.h"

namespace vapp {

  /* to sort blocks on their texture */
  struct AscendingTextureSort {
    bool operator() (Block* b1, Block* b2) {
      return b1->Texture() < b2->Texture();
    }
  };

  /*===========================================================================
  Called to prepare renderer for new level
  ===========================================================================*/
  void GameRenderer::prepareForNewLevel(bool bCreditsMode) {
    m_fCurrentHorizontalScrollShift = 0.0f;
    m_fCurrentVerticalScrollShift = 0.0f;
    m_previous_driver_dir  = DD_LEFT;    
    m_recenter_camera_fast = true;
    
    m_bCreditsMode = bCreditsMode;

    m_screenBBox.reset();

    m_fNextGhostInfoUpdate = 0.0f;
    m_nGhostInfoTrans      = 255;
        
    /* Optimize scene */
    std::vector<Block *> Blocks = getGameObject()->getLevelSrc()->Blocks();
    int nVertexBytes = 0;
  
    for(int i=0; i<Blocks.size(); i++) {

      bool dynamicBlock = false;
      std::vector<Geom *>* pGeoms;
      if(Blocks[i]->isDynamic() == true){
	dynamicBlock = true;
	pGeoms = &m_DynamicGeoms;
      }
      else{
	pGeoms = &m_StaticGeoms;
      }

      std::vector<ConvexBlock *> ConvexBlocks = Blocks[i]->ConvexBlocks();
      Vector2f Center;
      if(dynamicBlock == true){
      	Center.x = 0.0;
      	Center.y = 0.0;
      }
      else{
	Center = Blocks[i]->DynamicPosition();
      }
      Sprite* pSprite = getParent()->getTheme()->getSprite(SPRITE_TYPE_TEXTURE,
							   Blocks[i]->Texture());
      Texture *pTexture = NULL;

      if(pSprite != NULL) {
	try {
	  pTexture = pSprite->getTexture();
	} catch(Exception &e) {
	  Log("** Warning ** : Texture '%s' not found!",
	      Blocks[i]->Texture().c_str());
	  getGameObject()->gameMessage(GAMETEXT_MISSINGTEXTURES,true);   
	}
      } else {
	Log("** Warning ** : Texture '%s' not found!",
	    Blocks[i]->Texture().c_str());
	getGameObject()->gameMessage(GAMETEXT_MISSINGTEXTURES,true);          
      }

      Geom* pSuitableGeom = new Geom;
      pSuitableGeom->pTexture = pTexture;
      int geomIndex = pGeoms->size();
      pGeoms->push_back(pSuitableGeom);
      Blocks[i]->setGeom(geomIndex);

      for(int j=0; j<ConvexBlocks.size(); j++) {
        GeomPoly *pPoly = new GeomPoly;
        pSuitableGeom->Polys.push_back(pPoly);
        
        pPoly->nNumVertices = ConvexBlocks[j]->Vertices().size();
        pPoly->pVertices    = new GeomCoord[ pPoly->nNumVertices ];
        pPoly->pTexCoords   = new GeomCoord[ pPoly->nNumVertices ];
        
        for(int k=0; k<pPoly->nNumVertices; k++) {
          pPoly->pVertices[k].x = Center.x + ConvexBlocks[j]->Vertices()[k]->Position().x;
          pPoly->pVertices[k].y = Center.y + ConvexBlocks[j]->Vertices()[k]->Position().y;
          pPoly->pTexCoords[k].x = ConvexBlocks[j]->Vertices()[k]->TexturePosition().x;
          pPoly->pTexCoords[k].y = ConvexBlocks[j]->Vertices()[k]->TexturePosition().y;
        }          
        
        nVertexBytes += pPoly->nNumVertices * ( 4 * sizeof(float) );
#ifdef ENABLE_OPENGL        
        /* Use VBO optimization? */
	if(getParent()->getDrawLib()->useVBOs()) {
	  /* Copy static coordinates unto video memory */
	  ((DrawLibOpenGL*)getParent()->getDrawLib())->glGenBuffersARB(1, (GLuint *) &pPoly->nVertexBufferID);
	  ((DrawLibOpenGL*)getParent()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB,pPoly->nVertexBufferID);
	  ((DrawLibOpenGL*)getParent()->getDrawLib())->glBufferDataARB(GL_ARRAY_BUFFER_ARB,pPoly->nNumVertices*2*sizeof(float),(void *)pPoly->pVertices,GL_STATIC_DRAW_ARB);

	  ((DrawLibOpenGL*)getParent()->getDrawLib())->glGenBuffersARB(1, (GLuint *) &pPoly->nTexCoordBufferID);
	  ((DrawLibOpenGL*)getParent()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB,pPoly->nTexCoordBufferID);
	  ((DrawLibOpenGL*)getParent()->getDrawLib())->glBufferDataARB(GL_ARRAY_BUFFER_ARB,pPoly->nNumVertices*2*sizeof(float),(void *)pPoly->pTexCoords,GL_STATIC_DRAW_ARB);
	}
#endif
      }

#ifdef ENABLE_OPENGL
#if 0
      int displayList = glGenLists(1);
      glNewList(displayList, GL_COMPILE);
      Blocks[i]->setDisplayList(displayList);

      BlockVertex* v_blockVertexA;
      BlockVertex* v_blockVertexB;
      Vector2f vertexPos;
      //glPushAttrib(GL_CURRENT_BIT|GL_TEXTURE_BIT);
      glPushAttrib(GL_ALL_ATTRIB_BITS);

      /* create display list for edge rendering */
      for(int j=0; j<Blocks[i]->Vertices().size(); j++) {  
	v_blockVertexA = Blocks[i]->Vertices()[j];
	if(v_blockVertexA->EdgeEffect() != "") {
	  v_blockVertexB = Blocks[i]->Vertices()[(j+1) % Blocks[i]->Vertices().size()];

	  /* link A to B */
	  float fXScale,fDepth;
	  EdgeEffectSprite* pType;
	  pType = (EdgeEffectSprite*)getParent()->getTheme()->getSprite(SPRITE_TYPE_EDGEEFFECT, v_blockVertexA->EdgeEffect());

	  if(pType != NULL) {
	    fXScale = pType->getScale();
	    fDepth  = pType->getDepth();
                 
	    glEnable(GL_TEXTURE_2D);
	    glEnable(GL_BLEND);
	    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	    glBindTexture(GL_TEXTURE_2D, pType->getTexture()->nID);	    
	    glBegin(GL_POLYGON);
	    glColor4ub(255, 255, 255, 255);

	    glTexCoord2f((Center.x + v_blockVertexA->Position().x)*fXScale, 0.01);
	    vertexPos = v_blockVertexA->Position() + Center;
	    glVertex2f(vertexPos.x, vertexPos.y);

	    glTexCoord2f((Center.x + v_blockVertexB->Position().x)*fXScale, 0.01);
	    getParent()->getDrawLib()->glVertex(v_blockVertexB->Position() + Center);
	    glVertex2f(vertexPos.x, vertexPos.y);

	    glTexCoord2f((Center.x + v_blockVertexB->Position().x)*fXScale, 0.99);
	    vertexPos = v_blockVertexB->Position() + Center + Vector2f(0,-fDepth);
	    glVertex2f(vertexPos.x, vertexPos.y);

	    glTexCoord2f((Center.x + v_blockVertexA->Position().x)*fXScale, 0.99);
	    vertexPos = v_blockVertexA->Position() + Center + Vector2f(0,-fDepth);
	    glVertex2f(vertexPos.x, vertexPos.y);

	    glEnd();
	    glDisable(GL_TEXTURE_2D);
	    glDisable(GL_BLEND);
	  }
	}
      }

      glPopAttrib();
      glEndList();
#endif
#endif

      
    }
    
    setScroll(false);

    Log("Number of static  geoms: %d", m_StaticGeoms.size());
    Log("Number of dynamic geoms: %d", m_DynamicGeoms.size());
    Log("GL: %d kB vertex buffers",    nVertexBytes/1024);
  }

  /*===========================================================================
  Called when we don't want to play the level anymore
  ===========================================================================*/
  void GameRenderer::_deleteGeoms(std::vector<Geom *>& geom)
  {
    /* Clean up optimized scene */
    for(int i=0;i<geom.size();i++) { 
      for(int j=0;j<geom[i]->Polys.size();j++) { 
#ifdef ENABLE_OPENGL
        if(geom[i]->Polys[j]->nVertexBufferID) {
          ((DrawLibOpenGL*)getParent()->getDrawLib())->glDeleteBuffersARB(1, (GLuint *) &geom[i]->Polys[j]->nVertexBufferID);
          ((DrawLibOpenGL*)getParent()->getDrawLib())->glDeleteBuffersARB(1, (GLuint *) &geom[i]->Polys[j]->nTexCoordBufferID);
        }
#endif
      
        delete [] geom[i]->Polys[j]->pTexCoords;
        delete [] geom[i]->Polys[j]->pVertices;
        delete geom[i]->Polys[j];
      }
      delete geom[i];
    }
    geom.clear();
  }
  
  void GameRenderer::unprepareForNewLevel(void) {
    if(m_pInGameStats)
      m_pInGameStats->showWindow(false);
    _deleteGeoms(m_StaticGeoms);
    _deleteGeoms(m_DynamicGeoms);
  }

  void GameRenderer::renderEngineCounter(int x,int y,int nWidth,int nHeight, float pSpeed) {

// coords of then center ; make it dynamic would be nice
#define ENGINECOUNTER_CENTERX      192.0
#define ENGINECOUNTER_CENTERY      206.0
#define ENGINECOUNTER_RADIUS       150.0
#define ENGINECOUNTER_PICTURE_SIZE 256.0
#define ENGINECOUNTER_CORRECT_X    0.0
#define ENGINECOUNTER_CORRECT_Y    -30.0
#define ENGINECOUNTER_MAX_DIFF     1.0

    float pSpeed_eff;

    /* don't make line too nasty */
    if(m_previousEngineSpeed < 0.0) {
      pSpeed_eff = pSpeed;
    } else {
      if( labs(pSpeed - m_previousEngineSpeed) > ENGINECOUNTER_MAX_DIFF) {
  if(pSpeed - m_previousEngineSpeed > 0) {
    pSpeed_eff = m_previousEngineSpeed + ENGINECOUNTER_MAX_DIFF;
  } else {
    pSpeed_eff = m_previousEngineSpeed - ENGINECOUNTER_MAX_DIFF;
  }
      } else {
  pSpeed_eff = pSpeed;
      }
    }
    m_previousEngineSpeed = pSpeed_eff;

    Sprite *pSprite;
    Texture *pTexture;
    Vector2f p0, p1, p2, p3;
    Vector2f pcenter, pdest;
    float coefw = 1.0 / ENGINECOUNTER_PICTURE_SIZE * nWidth;
    float coefh = 1.0 / ENGINECOUNTER_PICTURE_SIZE * nHeight;

    p0 = Vector2f(x,        getParent()->getDrawLib()->getDispHeight()-y-nHeight);
    p1 = Vector2f(x+nWidth, getParent()->getDrawLib()->getDispHeight()-y-nHeight);
    p2 = Vector2f(x+nWidth, getParent()->getDrawLib()->getDispHeight()-y);
    p3 = Vector2f(x,        getParent()->getDrawLib()->getDispHeight()-y);

    pcenter = p3 + Vector2f(ENGINECOUNTER_CENTERX   * coefw,
          - ENGINECOUNTER_CENTERY * coefh); 
    pdest    = pcenter
    + Vector2f(ENGINECOUNTER_CORRECT_X * coefw,
         ENGINECOUNTER_CORRECT_Y * coefh)
    + Vector2f(-cosf(pSpeed_eff / 360.0 * (2.0 * 3.14159))
         * (ENGINECOUNTER_RADIUS) * coefw,
         sinf(pSpeed_eff / 360.0  * (2.0 * 3.14159))
         * (ENGINECOUNTER_RADIUS) * coefh
         );


    pSprite = (MiscSprite*) getParent()->getTheme()->getSprite(SPRITE_TYPE_MISC, "EngineCounter");
    if(pSprite != NULL) {
      pTexture = pSprite->getTexture();
      if(pTexture != NULL) {
      _RenderAlphaBlendedSection(pTexture, p0, p1, p2, p3);
  getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
  getParent()->getDrawLib()->setColorRGB(255,50,50);
  getParent()->getDrawLib()->glVertex(pcenter);
  getParent()->getDrawLib()->glVertex(pdest);
  getParent()->getDrawLib()->endDraw();
      }
    }
  }
    
  /*===========================================================================
  Minimap rendering
  ===========================================================================*/
  #define MINIMAPZOOM 5.0f
  #define MINIMAPALPHA 128
  #define MINIVERTEX(Px,Py) \
    getParent()->getDrawLib()->glVertexSP(x + nWidth/2 + (float)(Px - getCameraPositionX())*MINIMAPZOOM, \
                          y + nHeight/2 - (float)(Py - getCameraPositionY())*MINIMAPZOOM);    

  void GameRenderer::renderMiniMap(int x,int y,int nWidth,int nHeight) {
    getParent()->getDrawLib()->drawBox(Vector2f(x,y),Vector2f(x+nWidth,y+nHeight),1,
				       MAKE_COLOR(0,0,0,MINIMAPALPHA),
				       MAKE_COLOR(255,255,255,MINIMAPALPHA));
    getParent()->scissorGraphics(x+1,y+1,nWidth-2,nHeight-2);
#ifdef ENABLE_OPENGL
    glEnable(GL_SCISSOR_TEST);
    glLoadIdentity();
#endif

    /* get minimap AABB in level space
      input:  position on the screen (in the minimap area)
      output: position in the level
    */
#define MAP_TO_LEVEL_X(mapX) ((mapX) - x - nWidth/2)/MINIMAPZOOM  + getCameraPositionX()
#define MAP_TO_LEVEL_Y(mapY) ((mapY) - y - nHeight/2)/MINIMAPZOOM + getCameraPositionY()
    AABB mapBBox;
    mapBBox.addPointToAABB2f(MAP_TO_LEVEL_X(x), MAP_TO_LEVEL_Y(y));
    mapBBox.addPointToAABB2f(MAP_TO_LEVEL_X(x+nWidth), MAP_TO_LEVEL_Y(y+nHeight));

    /* TOFIX::Draw the static blocks only once in a texture, and reuse it after */
    /* Render blocks */
    MotoGame *pGame = getGameObject();
    std::vector<Block*> Blocks;

    for(int layer=-1; layer<=0; layer++){
      Blocks = getGameObject()->getCollisionHandler()->getStaticBlocksNearPosition(mapBBox, layer);
      for(int i=0; i<Blocks.size(); i++) {

	/* Don't draw background blocks neither dynamic ones */
	if(Blocks[i]->isBackground() == false && Blocks[i]->getLayer() == -1) {
	  std::vector<ConvexBlock *> ConvexBlocks = Blocks[i]->ConvexBlocks();
	  for(int j=0; j<ConvexBlocks.size(); j++) {
	    Vector2f Center = ConvexBlocks[j]->SourceBlock()->DynamicPosition(); 	 

	    getParent()->getDrawLib()->startDraw(DRAW_MODE_POLYGON); 	 
	    getParent()->getDrawLib()->setColorRGB(128,128,128);
	    /* TOFIX::what's THAT ??!? -->> put all the vertices in a vector and draw them in one opengl call ! */
	    for(int k=0; k<ConvexBlocks[j]->Vertices().size(); k++) { 	 
	      Vector2f P = Center + ConvexBlocks[j]->Vertices()[k]->Position(); 	 
	      MINIVERTEX(P.x,P.y); 	 
	    } 	 
	    getParent()->getDrawLib()->endDraw();
	  }
	}
      }
    }

    /* Render dynamic blocks */
    /* display only visible dyn blocks */
    Blocks = pGame->getCollisionHandler()->getDynBlocksNearPosition(mapBBox);

    /* TOFIX::do not calculate this again. (already done in Block.cpp) */
    for(int i=0; i<Blocks.size(); i++) {
      std::vector<ConvexBlock *> ConvexBlocks = Blocks[i]->ConvexBlocks();
      for(int j=0; j<ConvexBlocks.size(); j++) {
	getParent()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
	getParent()->getDrawLib()->setColorRGB(128,128,128);
	/* Build rotation matrix for block */
	float fR[4];
	fR[0] = cos(Blocks[i]->DynamicRotation()); fR[1] = -sin(Blocks[i]->DynamicRotation());
	fR[2] = sin(Blocks[i]->DynamicRotation()); fR[3] = cos(Blocks[i]->DynamicRotation());
	for(int k=0; k<ConvexBlocks[j]->Vertices().size(); k++) {
	  ConvexBlockVertex *pVertex = ConvexBlocks[j]->Vertices()[k];
	  
	  /* Transform vertex */
	  Vector2f Tv = Vector2f((pVertex->Position().x-Blocks[i]->DynamicRotationCenter().x) * fR[0] + (pVertex->Position().y-Blocks[i]->DynamicRotationCenter().y) * fR[1],
				 (pVertex->Position().x-Blocks[i]->DynamicRotationCenter().x) * fR[2] + (pVertex->Position().y-Blocks[i]->DynamicRotationCenter().y) * fR[3]);
	  Tv += Blocks[i]->DynamicPosition() + Blocks[i]->DynamicRotationCenter();
	  
	  MINIVERTEX(Tv.x,Tv.y);
	}
	
	getParent()->getDrawLib()->endDraw();
      }
    }


    /*
      input: position in the level
      output: position on the screen (draw in the minimap area)
    */
#define LEVEL_TO_SCREEN_X(elemPosX) (x + nWidth/2  + (float)((elemPosX) - getCameraPositionX()) * MINIMAPZOOM)
#define LEVEL_TO_SCREEN_Y(elemPosY) (y + nHeight/2 - (float)((elemPosY) - getCameraPositionY()) * MINIMAPZOOM)

    for(unsigned int i=0; i<getGameObject()->Players().size(); i++) {
      Vector2f bikePos(LEVEL_TO_SCREEN_X(getGameObject()->Players()[i]->getState()->CenterP.x),
		       LEVEL_TO_SCREEN_Y(getGameObject()->Players()[i]->getState()->CenterP.y));
      getParent()->getDrawLib()->drawCircle(bikePos, 3, 0, MAKE_COLOR(255,255,255,255), 0);
    }
    
    /* Render ghost position too? */
    for(unsigned int i=0; i<getGameObject()->Ghosts().size(); i++) {
      Ghost* v_ghost = getGameObject()->Ghosts()[i];

      Vector2f ghostPos(LEVEL_TO_SCREEN_X(v_ghost->getState()->CenterP.x),
			LEVEL_TO_SCREEN_Y(v_ghost->getState()->CenterP.y));
      getParent()->getDrawLib()->drawCircle(ghostPos, 3, 0, MAKE_COLOR(96,96,150,255), 0);
    }

    /* FIX::display only visible entities */
    std::vector<Entity*> Entities = pGame->getCollisionHandler()->getEntitiesNearPosition(mapBBox);

    for(int i=0;i<Entities.size();i++) {
      Vector2f entityPos(LEVEL_TO_SCREEN_X(Entities[i]->DynamicPosition().x),
			 LEVEL_TO_SCREEN_Y(Entities[i]->DynamicPosition().y));
      if(Entities[i]->DoesMakeWin()) {
        getParent()->getDrawLib()->drawCircle(entityPos, 3, 0, MAKE_COLOR(255,0,255,255), 0);
      }
      else if(Entities[i]->IsToTake()) {
        getParent()->getDrawLib()->drawCircle(entityPos, 3, 0, MAKE_COLOR(255,0,0,255), 0);
      }
    }
    
#ifdef ENABLE_OPENGL
    glDisable(GL_SCISSOR_TEST);
#endif
    //keesj:todo replace with setClipRect(NULL) in drawlib
    getParent()->scissorGraphics(0,0,getParent()->getDrawLib()->getDispWidth(),getParent()->getDrawLib()->getDispHeight());
  }
  
  void GameRenderer::zoom(float p_f) {
    m_fScale += p_f;
    if(m_fScale < 0) {
      m_fScale = 0;
    }
  }

  void GameRenderer::initZoom() {
    m_fScale = ZOOM_DEFAULT;
  }

  void GameRenderer::setCameraPosition(float px, float py) {
    m_cameraOffsetX = m_Scroll.x + px;
    m_cameraOffsetY = m_Scroll.y + py;
  }

  void GameRenderer::moveCamera(float px, float py) {
    m_cameraOffsetX += px;
    m_cameraOffsetY += py;
  }

  void GameRenderer::initCameraPosition() {
    m_cameraOffsetX = CAMERA_OFFSETX_DEFAULT;
    m_cameraOffsetY = CAMERA_OFFSETY_DEFAULT;
  }

  void GameRenderer::initCamera() {
    initCameraPosition();
    initZoom();
  }

  float GameRenderer::getCameraPositionX() {
    return -m_Scroll.x + m_cameraOffsetX;
  }
  
  float GameRenderer::getCameraPositionY() {
    return -m_Scroll.y + m_cameraOffsetY;
  }

  void GameRenderer::guessDesiredCameraPosition(float &p_fDesiredHorizontalScrollShift,
            float &p_fDesiredVerticalScrollShift) {

    float normal_hoffset = 4.0;
    float normal_voffset = 2.0;             
    p_fDesiredHorizontalScrollShift = 0.0;
    p_fDesiredVerticalScrollShift   = 0.0;

    if(m_playerToFollow == NULL) {
      return;
    }

    p_fDesiredHorizontalScrollShift = getGameObject()->getGravity().y * normal_hoffset / 9.81;
    if(m_playerToFollow->getState()->Dir == DD_LEFT) {
      p_fDesiredHorizontalScrollShift *= -1;
    }

    p_fDesiredVerticalScrollShift = getGameObject()->getGravity().x * normal_voffset / 9.81;
    if(m_playerToFollow->getState()->Dir == DD_RIGHT) {
      p_fDesiredVerticalScrollShift *= -1;
    }

    /* allow maximum and maximum */
    if(p_fDesiredHorizontalScrollShift > normal_hoffset) {
      p_fDesiredHorizontalScrollShift = normal_hoffset;
    }
    if(p_fDesiredHorizontalScrollShift < -normal_hoffset) {
      p_fDesiredHorizontalScrollShift = -normal_hoffset;
    }
    if(p_fDesiredVerticalScrollShift > normal_voffset) {
      p_fDesiredVerticalScrollShift = normal_voffset;
    }
    if(p_fDesiredVerticalScrollShift < -normal_voffset) {
      p_fDesiredVerticalScrollShift = -normal_voffset;
    }
  }

  void GameRenderer::_RenderGhost(Biker* i_ghost, int i) {
    /* Render ghost - ugly mode? */
    if(m_bUglyMode) {
      _RenderBike(i_ghost->getState(), &(i_ghost->getState()->Parameters()), m_theme->getGhostTheme());
    } else {
      /* No not ugly, fancy! Render into overlay? */      
      if(m_bGhostMotionBlur && getParent()->getDrawLib()->useFBOs()) {
	m_Overlay.beginRendering();
	m_Overlay.fade(0.15);
      }
      _RenderBike(i_ghost->getState(), &(i_ghost->getState()->Parameters()), m_theme->getGhostTheme());
	  
      if(m_bGhostMotionBlur && getParent()->getDrawLib()->useFBOs()) {
	m_Overlay.endRendering();
	m_Overlay.present();
      }
	  
      if(i_ghost->getDescription() != "") {
	if(m_nGhostInfoTrans > 0 && m_displayGhostInformation) {
	  _RenderInGameText(i_ghost->getState()->CenterP + Vector2f(i*3.0,-1.5f),
			    i_ghost->getDescription(),
			    MAKE_COLOR(255,255,255,m_nGhostInfoTrans));
	}
      }
    }
  }

  /*===========================================================================
  Main rendering function
  ===========================================================================*/
  void GameRenderer::render(bool bIsPaused) {
    /* Update time */
    m_pInGameStats->showWindow(!m_bCreditsMode);
    m_pPlayTime->setCaption(m_bCreditsMode?"":getParent()->formatTime(getGameObject()->getTime()));
    
    m_fZoom = 60.0f;    
    setScroll(true);
 
#ifdef ENABLE_OPENGL
    glLoadIdentity();
#endif

    /* calculate screen AABB to show only visible entities and dyn blocks */
    m_screenBBox.reset();

    float xScale = m_fScale * ((float)getParent()->getDrawLib()->getDispHeight()) / getParent()->getDrawLib()->getDispWidth();
    float yScale = m_fScale;
    // depends on zoom
    float xCamOffset=1.0/xScale;
    float yCamOffset=1.0/yScale;

    Vector2f v1(getCameraPositionX()-xCamOffset, getCameraPositionY()-yCamOffset);
    Vector2f v2(getCameraPositionX()+xCamOffset, getCameraPositionY()+yCamOffset);

    m_screenBBox.addPointToAABB2f(v1);
    m_screenBBox.addPointToAABB2f(v2);

    /* SKY! */
    if(!m_bUglyMode)
    _RenderSky(getGameObject()->getLevelSrc()->Sky().Zoom(),
	       getGameObject()->getLevelSrc()->Sky().Offset(),
	       getGameObject()->getLevelSrc()->Sky().TextureColor(),
	       getGameObject()->getLevelSrc()->Sky().DriftZoom(),
	       getGameObject()->getLevelSrc()->Sky().DriftTextureColor(),
	       getGameObject()->getLevelSrc()->Sky().Drifted());
    
#ifdef ENABLE_OPENGL
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
#endif

    /* Perform scaling/translation */    
    getParent()->getDrawLib()->setScale(xScale, yScale);
    //glRotatef(getGameObject()->getTime()*100,0,0,1); /* Uncomment this line if you want to vomit :) */
    getParent()->getDrawLib()->setTranslate(-getCameraPositionX(), -getCameraPositionY());

    if(m_Quality != GQ_LOW && !m_bUglyMode) {
      /* background level blocks */
      _RenderLayers(false);

      /* Background blocks */
      _RenderDynamicBlocks(true);
      _RenderBackground();
      
      /* ... then render background sprites ... */      
    }
    _RenderSprites(false,true);

    if(m_Quality == GQ_HIGH && !m_bUglyMode) {
      /* Render particles (back!) */    
      _RenderParticles(false);
    }

    /* ... covered by blocks ... */
    _RenderDynamicBlocks(false);
    _RenderBlocks();

    /* ... then render "middleground" sprites ... */
    _RenderSprites(false,false);

    /* ghosts */
    bool v_found = false;
    int v_found_i = 0;
    for(unsigned int i=0; i<getGameObject()->Ghosts().size(); i++) {
      Ghost* v_ghost = getGameObject()->Ghosts()[i];
      if(v_ghost != m_playerToFollow) {
	_RenderGhost(v_ghost, i);
      } else {
	v_found = true;
	v_found_i = i;
      }
    }
    /* draw the player to follow over the others */
    if(v_found) {
      _RenderGhost(getGameObject()->Ghosts()[v_found_i], v_found_i);
    }

    /* ... followed by the bike ... */
    v_found = false;
    for(unsigned int i=0; i<getGameObject()->Players().size(); i++) {
      PlayerBiker* v_player = getGameObject()->Players()[i];
      if(v_player != m_playerToFollow) {
	_RenderBike(v_player->getState(),
		    &(v_player->getState()->Parameters()),
		    m_theme->getPlayerTheme(),
		    v_player->getRenderBikeFront());
      } else {
	v_found = true;
      }
    }
    if(v_found) {
      _RenderBike(m_playerToFollow->getState(),
		  &(m_playerToFollow->getState()->Parameters()),
		  m_theme->getPlayerTheme(),
		  m_playerToFollow->getRenderBikeFront());
    }

    /* ghost information */
    if(getGameObject()->getTime() > m_fNextGhostInfoUpdate) {
      if(m_nGhostInfoTrans > 0) {
	if(m_fNextGhostInfoUpdate > 1.5f) {
	  m_nGhostInfoTrans-=16;
	}
	m_fNextGhostInfoUpdate += 0.025f;
      }
    }
    
    if(m_Quality == GQ_HIGH && !m_bUglyMode) {
      /* Render particles (front!) */    
      _RenderParticles();
    }
    
    /* ... and finally the foreground sprites! */
    _RenderSprites(true,false);

    /* and finally finally, front layers */
    if(m_Quality != GQ_LOW && !m_bUglyMode) {
      _RenderLayers(true);
    }

    if(isDebug()) {
      /* Draw some collision handling debug info */
      CollisionSystem *pc = getGameObject()->getCollisionHandler();
      for(int i=0;i<pc->m_CheckedLines.size();i++) {
        getParent()->getDrawLib()->setLineWidth(3);
	getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
	getParent()->getDrawLib()->setColorRGB(255,0,0);
        getParent()->getDrawLib()->glVertex(pc->m_CheckedLines[i]->x1,pc->m_CheckedLines[i]->y1);
        getParent()->getDrawLib()->glVertex(pc->m_CheckedLines[i]->x2,pc->m_CheckedLines[i]->y2);
	getParent()->getDrawLib()->endDraw();
        getParent()->getDrawLib()->setLineWidth(2);
      }
      for(int i=0;i<pc->m_CheckedCells.size();i++) {
	getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_LOOP);
	getParent()->getDrawLib()->setColorRGB(255,0,0);

        getParent()->getDrawLib()->glVertex(pc->m_CheckedCells[i].x1,pc->m_CheckedCells[i].y1);
        getParent()->getDrawLib()->glVertex(pc->m_CheckedCells[i].x2,pc->m_CheckedCells[i].y1);
        getParent()->getDrawLib()->glVertex(pc->m_CheckedCells[i].x2,pc->m_CheckedCells[i].y2);
        getParent()->getDrawLib()->glVertex(pc->m_CheckedCells[i].x1,pc->m_CheckedCells[i].y2);
	getParent()->getDrawLib()->endDraw();
      }
      for(int i=0;i<pc->m_CheckedLinesW.size();i++) {
        getParent()->getDrawLib()->setLineWidth(1);
	getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
	getParent()->getDrawLib()->setColorRGB(0,255,0);
        getParent()->getDrawLib()->glVertex(pc->m_CheckedLinesW[i]->x1,pc->m_CheckedLinesW[i]->y1);
        getParent()->getDrawLib()->glVertex(pc->m_CheckedLinesW[i]->x2,pc->m_CheckedLinesW[i]->y2);
	getParent()->getDrawLib()->endDraw();
        getParent()->getDrawLib()->setLineWidth(1);
      }
      for(int i=0;i<pc->m_CheckedCellsW.size();i++) {
	getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_LOOP);
	getParent()->getDrawLib()->setColorRGB(0,255,0);
        getParent()->getDrawLib()->glVertex(pc->m_CheckedCellsW[i].x1,pc->m_CheckedCellsW[i].y1);
        getParent()->getDrawLib()->glVertex(pc->m_CheckedCellsW[i].x2,pc->m_CheckedCellsW[i].y1);
        getParent()->getDrawLib()->glVertex(pc->m_CheckedCellsW[i].x2,pc->m_CheckedCellsW[i].y2);
        getParent()->getDrawLib()->glVertex(pc->m_CheckedCellsW[i].x1,pc->m_CheckedCellsW[i].y2);
	getParent()->getDrawLib()->endDraw();
      }

      std::vector<Entity*>& v = pc->getCheckedEntities();
      for(int i=0; i<v.size(); i++) {
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

#ifdef ENABLE_OPENGL
    glLoadIdentity();
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,getParent()->getDrawLib()->getDispWidth(),0,getParent()->getDrawLib()->getDispHeight(),-1,1);
    glMatrixMode(GL_MODELVIEW);
#endif

    if(!m_bCreditsMode) {
      /* And then the game messages */
      _RenderGameMessages();            
      
      /* If there's strawberries in the level, tell the user how many there's left */
      _RenderGameStatus();
    }

    /* minimap + counter */
    if(m_playerToFollow != NULL) {
      if(showMinimap() && m_bCreditsMode == false) {
    	if(m_playerToFollow->getState()->Dir == DD_LEFT && showEngineCounter() == false) {
	  renderMiniMap(getParent()->getDrawLib()->getDispWidth()-150,
			getParent()->getDrawLib()->getDispHeight()-100,150,100);
	} else {
	  renderMiniMap(0,getParent()->getDrawLib()->getDispHeight()-100,150,100);
	}
      }
      if(showEngineCounter() && m_bUglyMode == false) {
    	renderEngineCounter(getParent()->getDrawLib()->getDispWidth()-128,
			    getParent()->getDrawLib()->getDispHeight()-128,128,128,
    			    m_playerToFollow->getBikeEngineSpeed());
      }
    }

  }

  void GameRenderer::setScroll(bool isSmooth) {
    float v_move_camera_max;
    float v_fDesiredHorizontalScrollShift = 0.0;
    float v_fDesiredVerticalScrollShift   = 0.0;

    if(m_playerToFollow == NULL) {
      return;
    }

    /* determine if the camera must move fastly */
    /* it must go faster if the player change of sense */
    if(m_previous_driver_dir != m_playerToFollow->getState()->Dir) {
      m_recenter_camera_fast = true;
    }
    m_previous_driver_dir = m_playerToFollow->getState()->Dir;

    if(m_recenter_camera_fast) {
      v_move_camera_max = 0.1;
    } else {
      v_move_camera_max = 0.01;
    }

    /* Determine scroll */
    m_Scroll = -m_playerToFollow->getState()->CenterP;

    /* Driving direction? */
    guessDesiredCameraPosition(v_fDesiredHorizontalScrollShift, v_fDesiredVerticalScrollShift);
    
    if(fabs(v_fDesiredHorizontalScrollShift - m_fCurrentHorizontalScrollShift)
       < v_move_camera_max) {
      /* remove fast move once the camera is set correctly */
      m_recenter_camera_fast = false;
    }
    
    if(v_fDesiredHorizontalScrollShift != m_fCurrentHorizontalScrollShift) {
      float d = v_fDesiredHorizontalScrollShift - m_fCurrentHorizontalScrollShift;
      if(fabs(d)<v_move_camera_max || isSmooth == false) {
        m_fCurrentHorizontalScrollShift = v_fDesiredHorizontalScrollShift;
      }
      else if(d < 0.0f) {
        m_fCurrentHorizontalScrollShift -= v_move_camera_max * m_fSpeedMultiply;
      }
      else if(d > 0.0f) {
        m_fCurrentHorizontalScrollShift += v_move_camera_max * m_fSpeedMultiply;
      }
    }
    
    if(v_fDesiredVerticalScrollShift != m_fCurrentVerticalScrollShift) {
      float d = v_fDesiredVerticalScrollShift - m_fCurrentVerticalScrollShift;
      if(fabs(d)<0.01f || isSmooth == false) {
        m_fCurrentVerticalScrollShift = v_fDesiredVerticalScrollShift;
      }
      else if(d < 0.0f) {
        m_fCurrentVerticalScrollShift -= 0.01f * m_fSpeedMultiply;
      }
      else if(d > 0.0f) {
        m_fCurrentVerticalScrollShift += 0.01f * m_fSpeedMultiply;
      }
    }
    
    m_Scroll += Vector2f(m_fCurrentHorizontalScrollShift,
                         m_fCurrentVerticalScrollShift);
  }

  /*===========================================================================
  Game status rendering
  ===========================================================================*/
  void GameRenderer::_RenderGameStatus(void) {
    Sprite* pType = NULL;
    MotoGame *pGame = getGameObject();

    float x1 = 115;
    float y1 = -2;
    float x2 = 90;
    float y2 = 23;

    int nStrawberriesLeft = pGame->getLevelSrc()->countToTakeEntities();
    int nQuantity = 0;

    if(getParent()->isUglyMode() == false) {
      pType = getParent()->getTheme()->getSprite(SPRITE_TYPE_ANIMATION, getGameObject()->getLevelSrc()->SpriteForFlower());
      if(pType == NULL) {
	pType = getParent()->getTheme()->getSprite(SPRITE_TYPE_DECORATION, getGameObject()->getLevelSrc()->SpriteForFlower());
      }
    }
    
    if(nStrawberriesLeft > 0) {
      if(getParent()->isUglyMode() == false) {
	pType = getParent()->getTheme()->getSprite(SPRITE_TYPE_ANIMATION, getGameObject()->getLevelSrc()->SpriteForStrawberry());
      if(pType == NULL) {
	pType = getParent()->getTheme()->getSprite(SPRITE_TYPE_DECORATION, getGameObject()->getLevelSrc()->SpriteForStrawberry());
      }
      }
      nQuantity = nStrawberriesLeft;
    }
            
    if(pType != NULL) {    
      _RenderAlphaBlendedSectionSP(pType->getTexture(),Vector2f(x2,y2),Vector2f(x1,y2),Vector2f(x1,y1),Vector2f(x2,y1));
    }

    if(nQuantity > 0) {
      int tx1,ty1,tx2,ty2;
      char cBuf[256];    
      sprintf(cBuf,"%d",nQuantity);

      UIFont *v_font = getSmallFont();
      if(v_font != NULL) {
  UITextDraw::getTextExt(v_font,cBuf,&tx1,&ty1,&tx2,&ty2);
      }      

      /* Now for some evil special-case adjustments */
      int nAdjust = 0;
      if(nQuantity == 1) nAdjust = -3;
      if(nQuantity > 9) nAdjust = -2;
      
      /* Draw text */
      if(v_font != NULL) {
  UITextDraw::printRaw(v_font,nAdjust + (x1+x2)/2 - (tx2-tx1)/2,y2-(ty2-ty1)+3,cBuf,MAKE_COLOR(255,255,0,255));
      }
    }
  }
  
  /*===========================================================================
  Game message rendering
  ===========================================================================*/
  void GameRenderer::_RenderGameMessages(void) {
    MotoGame *pGame = getGameObject();

    /* Arrow messages */
    ArrowPointer *pArrow = &pGame->getArrowPointer();
    if(pArrow->nArrowPointerMode != 0) {
      Vector2f C;
      if(pArrow->nArrowPointerMode == 1) {          
        C=Vector2f(getParent()->getDrawLib()->getDispWidth()/2 + (float)(pArrow->ArrowPointerPos.x - getCameraPositionX())*m_fZoom,
                  getParent()->getDrawLib()->getDispHeight()/2 - (float)(pArrow->ArrowPointerPos.y - getCameraPositionY())*m_fZoom);      
      }
      else if(pArrow->nArrowPointerMode == 2) {          
        C.x=(getParent()->getDrawLib()->getDispWidth() * pArrow->ArrowPointerPos.x) / 800.0f;
        C.y=(getParent()->getDrawLib()->getDispHeight() * pArrow->ArrowPointerPos.y) / 600.0f;
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
      pType = (MiscSprite*) getParent()->getTheme()->getSprite(SPRITE_TYPE_MISC, "Arrow");
      if(pType != NULL) {
  _RenderAlphaBlendedSectionSP(pType->getTexture(),p1+C,p2+C,p3+C,p4+C);      
      }
    }
        
    /* Messages */
    if(pGame != NULL) {
      for(int i=0;i<pGame->getGameMessage().size();i++) {
        GameMessage *pMsg = pGame->getGameMessage()[i];
        int x1,y1,x2,y2;

  UIFont *v_font = getMediumFont();
  if(v_font != NULL) {
    UITextDraw::getTextExt(v_font,pMsg->Text,&x1,&y1,&x2,&y2);
    UITextDraw::printRaw(v_font,getParent()->getDrawLib()->getDispWidth()/2 - (x2-x1)/2,pMsg->Pos[1]*getParent()->getDrawLib()->getDispHeight(),pMsg->Text,MAKE_COLOR(255,255,255,pMsg->nAlpha));
  }
      }
    }
  }
  
  /*===========================================================================
  Sprite rendering main
  ===========================================================================*/
  void GameRenderer::_RenderSprites(bool bForeground,bool bBackground) {
    MotoGame *pGame = getGameObject();
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
    std::vector<Entity*> Entities = getGameObject()->getCollisionHandler()->getEntitiesNearPosition(screenBigger);

    int nbNearEntities = Entities.size();
    int nbTotalEntities = getGameObject()->getLevelSrc()->Entities().size();

    //printf("draw %d entities on %d\n", nbNearEntities, nbTotalEntities);

    for(int i=0;i<Entities.size();i++) {
      pEnt = Entities[i];

      switch(pEnt->Speciality()) {
        case ET_NONE:
          /* Middleground? (not foreground, not background) */
          if(pEnt->Z() == 0.0f && !bForeground && !bBackground) {
            _RenderSprite(pEnt);  
          } 
          else {
            /* In front? */
            if(pEnt->Z() > 0.0f && bForeground) {
              _RenderSprite(pEnt);
            } 
            else {
              /* Those in back? */
              if(pEnt->Z() < 0.0f && bBackground) {
                _RenderSprite(pEnt);
              }
            }
          }
          break;
      default:
          if(!bForeground && !bBackground) {
	    switch(pEnt->Speciality()) {
	    case ET_MAKEWIN:
	      _RenderSprite(pEnt, m_sizeMultOfEntitiesWhichMakeWin);
	      break;
	    case ET_ISTOTAKE:
	      _RenderSprite(pEnt, m_sizeMultOfEntitiesToTake);
	      break;
	    default:	      
	      _RenderSprite(pEnt);
	    }
          }
          break;
      }
    }
  }

  /*===========================================================================
  Render a sprite
  ===========================================================================*/
  void GameRenderer::_RenderSprite(Entity *pSprite, float i_sizeMult) {  
    Sprite* v_spriteType;
    AnimationSprite* v_animationSpriteType;
    DecorationSprite* v_decorationSpriteType;
    float v_centerX;
    float v_centerY;
    float v_width;
    float v_height;
    std::string v_sprite_type;

    if(m_bUglyMode == false) {
      switch(pSprite->Speciality()) {
      case ET_NONE:
        v_sprite_type = pSprite->SpriteName();
        break;
      case ET_KILL:
	v_sprite_type = getGameObject()->getLevelSrc()->SpriteForWecker();
        break;
      case ET_MAKEWIN:
	v_sprite_type = getGameObject()->getLevelSrc()->SpriteForFlower();
        break;
      case ET_ISTOTAKE:
	v_sprite_type = getGameObject()->getLevelSrc()->SpriteForStrawberry();
        break;
      }

      /* search the sprite as an animation */
      v_animationSpriteType = (AnimationSprite*) getParent()->getTheme()->getSprite(SPRITE_TYPE_ANIMATION, v_sprite_type);
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
        v_decorationSpriteType = (DecorationSprite*) getParent()->getTheme()->getSprite(SPRITE_TYPE_DECORATION, v_sprite_type);
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
	  
	  for(int i=0; i<4; i++) {

	    v_ray = sqrt((p[i].x*p[i].x) + (p[i].y*p[i].y));
	    beta = 0.0;

	    if(p[i].x >= 0.0 && p[i].y >= 0.0) {
	      beta = acos(p[i].x / v_ray);
	    } else if(p[i].x < 0.0 && p[i].y >= 0.0) {
	      beta = acos(p[i].y / v_ray) + M_PI / 2.0;
	    } else if(p[i].x < 0.0 && p[i].y < 0.0) {
	      beta = acos(-p[i].x / v_ray) + M_PI;
	    } else {
	      beta = acos(-p[i].y / v_ray) - M_PI / 2.0;
	    }
	    
	    p[i].x = cos(pSprite->DrawAngle() + beta) * v_ray;
	    p[i].y = sin(pSprite->DrawAngle() + beta) * v_ray;
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
    if(isDebug() || m_bTestThemeMode || m_bUglyMode) {
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
  void GameRenderer::_RenderDynamicBlocks(bool bBackground) {
    MotoGame *pGame = getGameObject();

    /* FIX::display only visible dyn blocks */
    std::vector<Block *> Blocks = getGameObject()->getCollisionHandler()->getDynBlocksNearPosition(m_screenBBox);

    /* sort blocks on their texture */
    std::sort(Blocks.begin(), Blocks.end(), AscendingTextureSort());

    if(m_bUglyMode) {

      for(int i=0; i<Blocks.size(); i++) {
	/* Are we rendering background blocks or what? */
	if(Blocks[i]->isBackground() != bBackground)
	  continue;

	/* Build rotation matrix for block */
	float fR[4];
	float rotation = Blocks[i]->DynamicRotation();
	fR[0] =  cos(rotation);
	fR[2] =  sin(rotation);
	fR[1] = -fR[2];
	fR[3] =  fR[0];

	Vector2f dynRotCenter = Blocks[i]->DynamicRotationCenter();
	Vector2f dynPos       = Blocks[i]->DynamicPosition();

	getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_LOOP);
	getParent()->getDrawLib()->setColorRGB(255,255,255);

        for(int j=0;j<Blocks[i]->Vertices().size();j++) {
	  Vector2f vertex = Blocks[i]->Vertices()[j]->Position();
	  /* transform vertex */
	  Vector2f transVertex = Vector2f((vertex.x-dynRotCenter.x)*fR[0] + (vertex.y-dynRotCenter.y)*fR[1],
					  (vertex.x-dynRotCenter.x)*fR[2] + (vertex.y-dynRotCenter.y)*fR[3]);
	  transVertex += dynPos + dynRotCenter;

          getParent()->getDrawLib()->glVertex(transVertex.x, transVertex.y);
        }
	getParent()->getDrawLib()->endDraw();
      }
    }
    else {
      for(int i=0; i<Blocks.size(); i++) {
	/* Are we rendering background blocks or what? */
	if(Blocks[i]->isBackground() != bBackground)
	  continue;

	Block* block = Blocks[i];
	/* Build rotation matrix for block */
	float fR[4];
	float rotation = block->DynamicRotation();
	fR[0] =  cos(rotation);
	fR[2] =  sin(rotation);
	fR[1] = -fR[2];
	fR[3] =  fR[0];

	Vector2f dynRotCenter = block->DynamicRotationCenter();
	Vector2f dynPos       = block->DynamicPosition();
	int geom = block->getGeom();

// 57.295779524 = 180/pi
#define rad2deg(x) ((x)*57.295779524)

	if(getParent()->getDrawLib()->getBackend() == DrawLib::backend_OpenGl) {
#ifdef ENABLE_OPENGL
	  glEnableClientState(GL_VERTEX_ARRAY);
	  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	  /* we're working with modelview matrix*/
	  glMatrixMode(GL_MODELVIEW);
	  glPushMatrix();

	  glTranslatef(dynPos.x, dynPos.y, 0);
	  if(rotation != 0.0){
	    glTranslatef(dynRotCenter.x, dynRotCenter.y, 0);
	    glRotatef(rad2deg(rotation), 0, 0, 1);
	    glTranslatef(-dynRotCenter.x, -dynRotCenter.y, 0);
	  }

	  getParent()->getDrawLib()->setTexture(m_DynamicGeoms[geom]->pTexture, BLEND_MODE_NONE);
	  getParent()->getDrawLib()->setColorRGB(255, 255, 255);

	  /* VBO optimized? */
	  if(getParent()->getDrawLib()->useVBOs()) {
	    for(int j=0;j<m_DynamicGeoms[geom]->Polys.size();j++) {          
	      GeomPoly *pPoly = m_DynamicGeoms[geom]->Polys[j];

	      ((DrawLibOpenGL*)getParent()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nVertexBufferID);
	      glVertexPointer(2,GL_FLOAT,0,(char *)NULL);

	      ((DrawLibOpenGL*)getParent()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nTexCoordBufferID);
	      glTexCoordPointer(2,GL_FLOAT,0,(char *)NULL);

	      glDrawArrays(GL_POLYGON,0,pPoly->nNumVertices);
	    }      
	  } else {
	    for(int j=0;j<m_DynamicGeoms[geom]->Polys.size();j++) {          
	      GeomPoly *pPoly = m_DynamicGeoms[geom]->Polys[j];
	      glVertexPointer(2,   GL_FLOAT, 0, pPoly->pVertices);
	      glTexCoordPointer(2, GL_FLOAT, 0, pPoly->pTexCoords);
	      glDrawArrays(GL_POLYGON, 0, pPoly->nNumVertices);
	    }
	  }

	  glPopMatrix();

	  glDisableClientState(GL_VERTEX_ARRAY);
	  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
	} else if(getParent()->getDrawLib()->getBackend() == DrawLib::backend_SdlGFX){

	  for(int j=0;j<m_DynamicGeoms[geom]->Polys.size();j++) {          
	    getParent()->getDrawLib()->setTexture(m_DynamicGeoms[geom]->pTexture,BLEND_MODE_NONE);
	    getParent()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
	    getParent()->getDrawLib()->setColorRGB(255,255,255);

	    for(int k=0;k<m_DynamicGeoms[geom]->Polys[j]->nNumVertices;k++) {
	      Vector2f vertex = Vector2f(m_DynamicGeoms[geom]->Polys[j]->pVertices[k].x,
					 m_DynamicGeoms[geom]->Polys[j]->pVertices[k].y);
	      /* transform vertex */
	      Vector2f transVertex = Vector2f((vertex.x-dynRotCenter.x)*fR[0] + (vertex.y-dynRotCenter.y)*fR[1],
					      (vertex.x-dynRotCenter.x)*fR[2] + (vertex.y-dynRotCenter.y)*fR[3]);
	      transVertex += dynPos + dynRotCenter;

	      getParent()->getDrawLib()->glTexCoord(m_DynamicGeoms[geom]->Polys[j]->pTexCoords[k].x,
						    m_DynamicGeoms[geom]->Polys[j]->pTexCoords[k].y);
	      getParent()->getDrawLib()->glVertex(transVertex.x, transVertex.y);
	    }
	    getParent()->getDrawLib()->endDraw();
	  }

	}
      }

      /* Render all special edges (if quality!=low) */
      if(m_Quality != GQ_LOW) {
	for(int i=0;i<Blocks.size();i++) {
	  if(Blocks[i]->isBackground() == bBackground){
	    _RenderBlockEdges(Blocks[i]);
	  }
	}
      }

    }
  }

  void GameRenderer::_RenderBlock(Block* block)
  {
    int geom = block->getGeom();
    getParent()->getDrawLib()->setTexture(m_StaticGeoms[geom]->pTexture, BLEND_MODE_NONE);
    getParent()->getDrawLib()->setColorRGB(255, 255, 255);

    if(getParent()->getDrawLib()->getBackend() == DrawLib::backend_OpenGl) {
#ifdef ENABLE_OPENGL
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);

      /* VBO optimized? */
      if(getParent()->getDrawLib()->useVBOs()) {
	for(int j=0;j<m_StaticGeoms[geom]->Polys.size();j++) {          
	  GeomPoly *pPoly = m_StaticGeoms[geom]->Polys[j];

	  ((DrawLibOpenGL*)getParent()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nVertexBufferID);
	  glVertexPointer(2,GL_FLOAT,0,(char *)NULL);

	  ((DrawLibOpenGL*)getParent()->getDrawLib())->glBindBufferARB(GL_ARRAY_BUFFER_ARB, pPoly->nTexCoordBufferID);
	  glTexCoordPointer(2,GL_FLOAT,0,(char *)NULL);

	  glDrawArrays(GL_POLYGON,0,pPoly->nNumVertices);
	}      
      } else {
	for(int j=0;j<m_StaticGeoms[geom]->Polys.size();j++) {          
	  GeomPoly *pPoly = m_StaticGeoms[geom]->Polys[j];
	  glVertexPointer(2,   GL_FLOAT, 0, pPoly->pVertices);
	  glTexCoordPointer(2, GL_FLOAT, 0, pPoly->pTexCoords);
	  glDrawArrays(GL_POLYGON, 0, pPoly->nNumVertices);
	}
      }

      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
    } else if(getParent()->getDrawLib()->getBackend() == DrawLib::backend_SdlGFX){

      for(int j=0;j<m_StaticGeoms[geom]->Polys.size();j++) {          
	getParent()->getDrawLib()->setTexture(m_StaticGeoms[geom]->pTexture,BLEND_MODE_NONE);
	getParent()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
	getParent()->getDrawLib()->setColorRGB(255,255,255);
	for(int k=0;k<m_StaticGeoms[geom]->Polys[j]->nNumVertices;k++) {
	  getParent()->getDrawLib()->glTexCoord(m_StaticGeoms[geom]->Polys[j]->pTexCoords[k].x,
						m_StaticGeoms[geom]->Polys[j]->pTexCoords[k].y);
	  getParent()->getDrawLib()->glVertex(m_StaticGeoms[geom]->Polys[j]->pVertices[k].x,
					      m_StaticGeoms[geom]->Polys[j]->pVertices[k].y);
	}
	getParent()->getDrawLib()->endDraw();
      }
    }
  }

  void GameRenderer::_RenderBlockEdges(Block* block)
  {
    BlockVertex* v_blockVertexA;
    BlockVertex* v_blockVertexB;

    //      glCallList(block->getDisplayList());
    /* FIXME::is the edge of a dyn block rotated with the dyn block ?? */
    for(int j=0; j<block->Vertices().size(); j++) {  
      v_blockVertexA = block->Vertices()[j];
      if(v_blockVertexA->EdgeEffect() != "") {
	v_blockVertexB = block->Vertices()[(j+1) % block->Vertices().size()];

	Vector2f vAPos = v_blockVertexA->Position();
	Vector2f vBPos = v_blockVertexB->Position();

	/* link A to B */
	float fXScale,fDepth;
	EdgeEffectSprite* pType;
	pType = (EdgeEffectSprite*)getParent()->getTheme()->getSprite(SPRITE_TYPE_EDGEEFFECT, v_blockVertexA->EdgeEffect());

	if(pType != NULL) {
	  fXScale = pType->getScale();
	  fDepth  = pType->getDepth();
                 
	  getParent()->getDrawLib()->setTexture(pType->getTexture(),BLEND_MODE_A);
	  getParent()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
	  getParent()->getDrawLib()->setColorRGB(255,255,255);

	  getParent()->getDrawLib()->glTexCoord((block->DynamicPosition().x+vAPos.x)*fXScale,0.01);
	  getParent()->getDrawLib()->glVertex(vAPos + Vector2f(block->DynamicPosition().x,block->DynamicPosition().y));
	  getParent()->getDrawLib()->glTexCoord((block->DynamicPosition().x+vBPos.x)*fXScale,0.01);
	  getParent()->getDrawLib()->glVertex(vBPos + Vector2f(block->DynamicPosition().x,block->DynamicPosition().y));
	  getParent()->getDrawLib()->glTexCoord((block->DynamicPosition().x+vBPos.x)*fXScale,0.99);
	  getParent()->getDrawLib()->glVertex(vBPos + Vector2f(block->DynamicPosition().x,block->DynamicPosition().y) + Vector2f(0,-fDepth));
	  getParent()->getDrawLib()->glTexCoord((block->DynamicPosition().x+vAPos.x)*fXScale,0.99);
	  getParent()->getDrawLib()->glVertex(vAPos + Vector2f(block->DynamicPosition().x,block->DynamicPosition().y) + Vector2f(0,-fDepth));

	  getParent()->getDrawLib()->endDraw();
	}
      }
    }
  }

  /*===========================================================================
  Blocks (static)
  ===========================================================================*/
  void GameRenderer::_RenderBlocks(void) {
    MotoGame *pGame = getGameObject();

    for(int layer=-1; layer<=0; layer++){
      std::vector<Block *> Blocks;

      /* Render all non-background blocks */
      Blocks = getGameObject()->getCollisionHandler()->getStaticBlocksNearPosition(m_screenBBox, layer);

      /* sort blocks on their texture */
      std::sort(Blocks.begin(), Blocks.end(), AscendingTextureSort());

      /* Ugly mode? */
      if(m_bUglyMode) {
	for(int i=0;i<Blocks.size();i++) {
	  if(Blocks[i]->isBackground() == false) {
	    getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_LOOP);
	    getParent()->getDrawLib()->setColorRGB(255,255,255);
	    for(int j=0;j<Blocks[i]->Vertices().size();j++) {
	      getParent()->getDrawLib()->glVertex(Blocks[i]->Vertices()[j]->Position().x + Blocks[i]->DynamicPosition().x,
						  Blocks[i]->Vertices()[j]->Position().y + Blocks[i]->DynamicPosition().y);
	    }
	    getParent()->getDrawLib()->endDraw();
	  }
	}
      }
      else {
	/* Render all non-background blocks */
	/* Static geoms... */
	for(int i=0;i<Blocks.size();i++) {
	  if(Blocks[i]->isBackground() == false) {
	    _RenderBlock(Blocks[i]);
	  }
	}

	/* Render all special edges (if quality!=low) */
	if(m_Quality != GQ_LOW) {
	  for(int i=0;i<Blocks.size();i++) {
	    if(Blocks[i]->isBackground() == false) {
	    _RenderBlockEdges(Blocks[i]);
	    }
	  }
	}
      }
    }
  }

  /*===========================================================================
  Sky.
  ===========================================================================*/
  void GameRenderer::_RenderSky(float i_zoom, float i_offset, const TColor& i_color,
				float i_driftZoom, const TColor& i_driftColor, bool i_drifted) {
  MotoGame *pGame = getGameObject();
  TextureSprite* pType;
  float fDrift = 0.0;
  float uZoom = 1.0 / i_zoom;
  float uDriftZoom = 1.0 / i_driftZoom;

  if(m_Quality != GQ_HIGH) {
    i_drifted = false;
  }
  
  pType = (TextureSprite*) getParent()->getTheme()->getSprite(SPRITE_TYPE_TEXTURE,
							      pGame->getLevelSrc()->Sky().Texture());
  
  if(pType != NULL) {
    if(i_drifted) {
      getParent()->getDrawLib()->setTexture(pType->getTexture(), BLEND_MODE_A);
    }
    
    getParent()->getDrawLib()->setTexture(pType->getTexture(),BLEND_MODE_NONE);
    getParent()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
    getParent()->getDrawLib()->setColorRGBA(i_color.Red() , i_color.Green(), i_color.Blue(), i_color.Alpha());
    
    if(i_drifted) {
      fDrift = getParent()->getRealTime() / 25.0f;
    }
    
    getParent()->getDrawLib()->glTexCoord(getCameraPositionX()*i_offset+ fDrift,-getCameraPositionY()*i_offset);
    getParent()->getDrawLib()->glVertexSP(0,0);
    getParent()->getDrawLib()->glTexCoord(getCameraPositionX()*i_offset+uZoom+ fDrift,-getCameraPositionY()*i_offset);
    getParent()->getDrawLib()->glVertexSP(getParent()->getDrawLib()->getDispWidth(),0);
    getParent()->getDrawLib()->glTexCoord(getCameraPositionX()*i_offset+uZoom+ fDrift,-getCameraPositionY()*i_offset+uZoom);
    getParent()->getDrawLib()->glVertexSP(getParent()->getDrawLib()->getDispWidth(),getParent()->getDrawLib()->getDispHeight());
    getParent()->getDrawLib()->glTexCoord(getCameraPositionX()*i_offset+ fDrift,-getCameraPositionY()*i_offset+uZoom);
    getParent()->getDrawLib()->glVertexSP(0,getParent()->getDrawLib()->getDispHeight());
    getParent()->getDrawLib()->endDraw();
    
    if(i_drifted) {
      getParent()->getDrawLib()->setTexture(pType->getTexture(),BLEND_MODE_B);
      getParent()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
      getParent()->getDrawLib()->setColorRGBA(i_driftColor.Red(), i_driftColor.Green(), i_driftColor.Blue(), i_driftColor.Alpha());
      fDrift = getParent()->getRealTime() / 15.0f;
      getParent()->getDrawLib()->glTexCoord(getCameraPositionX()*i_offset + fDrift,-getCameraPositionY()*i_offset);
      getParent()->getDrawLib()->glVertexSP(0,0);
      getParent()->getDrawLib()->glTexCoord(getCameraPositionX()*i_offset+uDriftZoom + fDrift,-getCameraPositionY()*i_offset);
      getParent()->getDrawLib()->glVertexSP(getParent()->getDrawLib()->getDispWidth(),0);
      getParent()->getDrawLib()->glTexCoord(getCameraPositionX()*i_offset+uDriftZoom + fDrift,-getCameraPositionY()*i_offset+uDriftZoom);
      getParent()->getDrawLib()->glVertexSP(getParent()->getDrawLib()->getDispWidth(),getParent()->getDrawLib()->getDispHeight());
      getParent()->getDrawLib()->glTexCoord(getCameraPositionX()*i_offset + fDrift,-getCameraPositionY()*i_offset+uDriftZoom);
      getParent()->getDrawLib()->glVertexSP(0,getParent()->getDrawLib()->getDispHeight());
      getParent()->getDrawLib()->endDraw();
    }
  } else {
    Log(std::string("** Invalid sky " + pGame->getLevelSrc()->Sky().Texture()).c_str());
    getParent()->getDrawLib()->clearGraphics();
  } 
 }

  /*===========================================================================
  And background rendering
  ===========================================================================*/
  void GameRenderer::_RenderBackground(void) { 
    MotoGame *pGame = getGameObject();

    /* Render STATIC background blocks */
    std::vector<Block *> Blocks = getGameObject()->getCollisionHandler()->getStaticBlocksNearPosition(m_screenBBox);

    /* sort blocks on their texture */
    std::sort(Blocks.begin(), Blocks.end(), AscendingTextureSort());

    for(int i=0;i<Blocks.size();i++) {
      if(Blocks[i]->isBackground() == true) {
	_RenderBlock(Blocks[i]);
      }
    }

    /* Render all special edges (if quality != low) */
    if(m_Quality != GQ_LOW) {
      for(int i=0;i<Blocks.size();i++) {
	if(Blocks[i]->isBackground() == true) {
	  _RenderBlockEdges(Blocks[i]);
	}
      }
    }
  }

  void GameRenderer::_RenderLayer(int layer) {
    if(getParent()->getDrawLib()->getBackend() != DrawLib::backend_OpenGl) {
      return;
    }

    Vector2f layerOffset = getGameObject()->getLevelSrc()->getLayerOffset(layer);

    /* get bounding box in the layer depending on its offset */
    Vector2f size = m_screenBBox.getBMax() - m_screenBBox.getBMin();

    Vector2f levelLeftTop = Vector2f(getGameObject()->getLevelSrc()->LeftLimit(),
				     getGameObject()->getLevelSrc()->TopLimit());

    Vector2f levelViewLeftTop = Vector2f(m_screenBBox.getBMin().x,
					 m_screenBBox.getBMin().y+size.y);

    Vector2f originalTranslateVector = levelViewLeftTop - levelLeftTop;
    Vector2f translationInLayer = originalTranslateVector * layerOffset;
    Vector2f translateVector = originalTranslateVector - translationInLayer;

    AABB layerBBox;
    layerBBox.addPointToAABB2f(levelLeftTop+translationInLayer);
    layerBBox.addPointToAABB2f(levelLeftTop.x + translationInLayer.x + size.x,
			       levelLeftTop.y + translationInLayer.y - size.y);

    std::vector<Block *> Blocks = getGameObject()->getCollisionHandler()->getBlocksNearPositionInLayer(layerBBox, layer);
    /* sort blocks on their texture */
    std::sort(Blocks.begin(), Blocks.end(), AscendingTextureSort());

#ifdef ENABLE_OPENGL
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(translateVector.x, translateVector.y, 0);

    for(int i=0; i<Blocks.size(); i++) {
      Block* block = Blocks[i];
      int geom = block->getGeom();

      _RenderBlock(block);
    }
    /* Render all special edges (if quality!=low) */
    if(m_Quality != GQ_LOW) {
      for(int i=0;i<Blocks.size();i++) {
	_RenderBlockEdges(Blocks[i]);
      }
    }
    glPopMatrix();
#endif
  }

  void GameRenderer::_RenderLayers(bool renderFront) { 
    /* Render background level blocks */
    int nbLayer = getGameObject()->getLevelSrc()->getNumberLayer();
    for(int layer=0; layer<nbLayer; layer++){
      if(getGameObject()->getLevelSrc()->isLayerFront(layer) == renderFront){
	_RenderLayer(layer);
      }
    }
  }

  /*===========================================================================
  Helpers
  ===========================================================================*/
  void GameRenderer::_DbgText(Vector2f P,std::string Text,Color c) {
    Vector2f Sp = Vector2f(getParent()->getDrawLib()->getDispWidth()/2 + (float)(P.x - getCameraPositionX())*m_fZoom,
                           getParent()->getDrawLib()->getDispHeight()/2 - (float)(P.y - getCameraPositionY())*m_fZoom) -
                  Vector2f(getParent()->getDrawLib()->getTextWidth(Text)/2.0f,getParent()->getDrawLib()->getTextHeight(Text)/2.0f);
    getParent()->getDrawLib()->drawText(Sp,Text,0,c,true);
  }   

  /*===========================================================================
  Free stuff
  ===========================================================================*/
  void GameRenderer::_Free(void) {      
  }
  
  void GameRenderer::shutdown(void) {
    /* Free overlay */
    m_Overlay.cleanUp();
  }  

  /*===========================================================================
  Debug info. Note how this is leaked into the void and nobody cares :) 
  ===========================================================================*/
  void GameRenderer::_RenderDebugInfo(void) {
    for(int i=0;i<m_DebugInfo.size();i++) {
      if(m_DebugInfo[i]->Type == "@WHITEPOLYGONS") {
	getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_LOOP);
	getParent()->getDrawLib()->setColorRGB(255,255,255);
        for(int j=0;j<m_DebugInfo[i]->Args.size()/2;j++) {
          float x = atof(m_DebugInfo[i]->Args[j*2].c_str());
          float y = atof(m_DebugInfo[i]->Args[j*2+1].c_str());
          getParent()->getDrawLib()->glVertexSP(400 + x*10,300 - y*10);
        }
	getParent()->getDrawLib()->endDraw();
      }
      else if(m_DebugInfo[i]->Type == "@REDLINES") {
	getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
	getParent()->getDrawLib()->setColorRGB(255,0,0);
        for(int j=0;j<m_DebugInfo[i]->Args.size()/2;j++) {
          float x = atof(m_DebugInfo[i]->Args[j*2].c_str());
          float y = atof(m_DebugInfo[i]->Args[j*2+1].c_str());
          getParent()->getDrawLib()->glVertexSP(400 + x*10,300 - y*10);
        }
	getParent()->getDrawLib()->endDraw();
      }
      else if(m_DebugInfo[i]->Type == "@GREENLINES") {
	getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
	getParent()->getDrawLib()->setColorRGB(0,255,0);
        for(int j=0;j<m_DebugInfo[i]->Args.size()/2;j++) {
          float x = atof(m_DebugInfo[i]->Args[j*2].c_str());
          float y = atof(m_DebugInfo[i]->Args[j*2+1].c_str());
          getParent()->getDrawLib()->glVertexSP(400 + x*10,300 - y*10);
        }
	getParent()->getDrawLib()->endDraw();
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
        m_pReplayHelp->setCaption(GAMETEXT_REPLAYHELPTEXT(String(">> 10")));
      } 
      else if(p_speed <= -10.0) {
        m_pReplayHelp->setCaption(GAMETEXT_REPLAYHELPTEXT(String("<<-10")));
      } 
      else {
        char v_speed_str[5 + 1];
        sprintf(v_speed_str, "% .2f", p_speed);
        m_pReplayHelp->setCaption(GAMETEXT_REPLAYHELPTEXT(String(v_speed_str)));
      }
    } 
    else {
      if(p_speed >= 10.0) {
        m_pReplayHelp->setCaption(GAMETEXT_REPLAYHELPTEXTNOREWIND(String(">> 10")));
      } 
      else if(p_speed <= -10.0) {
        m_pReplayHelp->setCaption(GAMETEXT_REPLAYHELPTEXTNOREWIND(String("<<-10")));
      } 
      else {
        char v_speed_str[256];
        sprintf(v_speed_str, "% .2f", p_speed);
        m_pReplayHelp->setCaption(GAMETEXT_REPLAYHELPTEXTNOREWIND(String(v_speed_str)));
      }
    }
  }

  void GameRenderer::hideReplayHelp() {
    m_pReplayHelp->setCaption("");
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
      float vx = ((float)getParent()->getDrawLib()->getDispWidth() * x);
      float vy = ((float)getParent()->getDrawLib()->getDispHeight() * y);
#ifdef ENABLE_OPENGL
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      glOrtho(0,getParent()->getDrawLib()->getDispWidth(),0,getParent()->getDrawLib()->getDispHeight(),-1,1);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
#endif
      
      int nMinX,nMinY,nMaxX,nMaxY;
      if(m_pSFont != NULL) {
  UITextDraw::getTextExt(m_pSFont,Text,&nMinX,&nMinY,&nMaxX,&nMaxY);

  int nx = vx - (nMaxX - nMinX)/2;
  int ny = vy;
  UITextDraw::printRaw(m_pSFont,nx-1,ny-1,Text,MAKE_COLOR(0,0,0,GET_ALPHA(c)));
  UITextDraw::printRaw(m_pSFont,nx+1,ny-1,Text,MAKE_COLOR(0,0,0,GET_ALPHA(c)));
  UITextDraw::printRaw(m_pSFont,nx+1,ny+1,Text,MAKE_COLOR(0,0,0,GET_ALPHA(c)));
  UITextDraw::printRaw(m_pSFont,nx-1,ny+1,Text,MAKE_COLOR(0,0,0,GET_ALPHA(c)));
  UITextDraw::printRaw(m_pSFont,nx,ny,Text,c);
      } 

#ifdef ENABLE_OPENGL
      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
#endif
      
    }
  }
  
  void GameRenderer::showMsgNewPersonalHighscore(String p_save) {
    m_pInGameNewHighscore->showWindow(true);
    m_pNewHighscorePersonal_str->showWindow(true);
    m_pNewHighscoreBest_str->showWindow(false);
    if(p_save == "") {
      m_pNewHighscoreSave_str->showWindow(false);
    } else {
      m_pNewHighscoreSave_str->showWindow(true);
      m_pNewHighscoreSave_str->setCaption("(" + std::string(GAMETEXT_SAVE_AS) + " " + p_save + ")");
    }
  }
  
  void GameRenderer::showMsgNewBestHighscore(String p_save) {
    m_pInGameNewHighscore->showWindow(true);
    m_pNewHighscorePersonal_str->showWindow(false);
    m_pNewHighscoreBest_str->showWindow(true);
    if(p_save == "") {
      m_pNewHighscoreSave_str->showWindow(false);
    } else {
      m_pNewHighscoreSave_str->showWindow(true);
      m_pNewHighscoreSave_str->setCaption("(" + std::string(GAMETEXT_SAVE_AS) + " " + p_save + ")");
    }
  }

  void GameRenderer::hideMsgNewHighscore() {
    m_pInGameNewHighscore->showWindow(false);
  }

  /*===========================================================================
  Rendering helpers
  ===========================================================================*/
  void GameRenderer::_RenderAlphaBlendedSection(Texture *pTexture,
                                                const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3) {
    getParent()->getDrawLib()->setTexture(pTexture,BLEND_MODE_A);
    getParent()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
    getParent()->getDrawLib()->setColorRGB(255,255,255);
    getParent()->getDrawLib()->glTexCoord(0,1);
    getParent()->getDrawLib()->glVertex(p0.x,p0.y);
    getParent()->getDrawLib()->glTexCoord(1,1);
    getParent()->getDrawLib()->glVertex(p1.x,p1.y);
    getParent()->getDrawLib()->glTexCoord(1,0);
    getParent()->getDrawLib()->glVertex(p2.x,p2.y);
    getParent()->getDrawLib()->glTexCoord(0,0);
    getParent()->getDrawLib()->glVertex(p3.x,p3.y);
    getParent()->getDrawLib()->endDraw();
  }
  
  void GameRenderer::_RenderAdditiveBlendedSection(Texture *pTexture,
                                                   const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3) {
    getParent()->getDrawLib()->setTexture(pTexture,BLEND_MODE_B);
    getParent()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
    getParent()->getDrawLib()->setColorRGB(255,255,255);
    getParent()->getDrawLib()->glTexCoord(0,1);
    getParent()->getDrawLib()->glVertex(p0.x,p0.y);
    getParent()->getDrawLib()->glTexCoord(1,1);
    getParent()->getDrawLib()->glVertex(p1.x,p1.y);
    getParent()->getDrawLib()->glTexCoord(1,0);
    getParent()->getDrawLib()->glVertex(p2.x,p2.y);
    getParent()->getDrawLib()->glTexCoord(0,0);
    getParent()->getDrawLib()->glVertex(p3.x,p3.y);
    getParent()->getDrawLib()->endDraw();
  }
  
  /* Screen-space version of the above */
  void GameRenderer::_RenderAlphaBlendedSectionSP(Texture *pTexture,
                                                  const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3) {
    getParent()->getDrawLib()->setTexture(pTexture,BLEND_MODE_A);
    getParent()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
    getParent()->getDrawLib()->setColorRGB(255,255,255);
    getParent()->getDrawLib()->glTexCoord(0,1);
    getParent()->getDrawLib()->glVertexSP(p0.x,p0.y);
    getParent()->getDrawLib()->glTexCoord(1,1);
    getParent()->getDrawLib()->glVertexSP(p1.x,p1.y);
    getParent()->getDrawLib()->glTexCoord(1,0);
    getParent()->getDrawLib()->glVertexSP(p2.x,p2.y);
    getParent()->getDrawLib()->glTexCoord(0,0);
    getParent()->getDrawLib()->glVertexSP(p3.x,p3.y);
    getParent()->getDrawLib()->endDraw();
  }
  
  void GameRenderer::_RenderCircle(int nSteps,Color CircleColor,const Vector2f &C,float fRadius) {
    getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_LOOP);
    getParent()->getDrawLib()->setColor(CircleColor);
    for(int i=0;i<nSteps;i++) {
      float r = (3.14159f * 2.0f * (float)i)/ (float)nSteps;            
      getParent()->getDrawLib()->glVertex( Vector2f(C.x + fRadius*sin(r),C.y + fRadius*cos(r)) );
    }      
    getParent()->getDrawLib()->endDraw();
  }

  void GameRenderer::setTheme(Theme *p_theme) {
    m_theme = p_theme;
  }

  float GameRenderer::getCurrentZoom() {
    return m_fScale;
  }

  void GameRenderer::setZoom(float p_f) {
    m_fScale = p_f;
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

  void GameRenderer::setGhostDisplayInformation(bool i_display) {
    m_displayGhostInformation = i_display;
  }

  void GameRenderer::setPlayerToFollow(Biker* i_playerToFollow) {
    m_playerToFollow = i_playerToFollow;
  }

  bool GameRenderer::showMinimap() const {
    return m_showMinimap;
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

  void GameRenderer::switchFollow() {
    if(m_playerToFollow == NULL) return;

    /* search into the player */
    for(unsigned i=0; i<getGameObject()->Players().size(); i++) {
      if(getGameObject()->Players()[i] == m_playerToFollow) {
	if(i<getGameObject()->Players().size()-1) {
	  m_playerToFollow = getGameObject()->Players()[i+1];
	} else {
	  if(getGameObject()->Ghosts().size() > 0) {
	    m_playerToFollow = getGameObject()->Ghosts()[0];
	  } else {
	    m_playerToFollow = getGameObject()->Players()[0];
	  }
	}
	return;
      }
    }

    /* search into the ghost */
    for(unsigned i=0; i<getGameObject()->Ghosts().size(); i++) {
      if(getGameObject()->Ghosts()[i] == m_playerToFollow) {
	if(i<getGameObject()->Ghosts().size()-1) {
	  m_playerToFollow = getGameObject()->Ghosts()[i+1];
	} else {
	  if(getGameObject()->Players().size() > 0) {
	    m_playerToFollow = getGameObject()->Players()[0];
	  } else {
	    m_playerToFollow = getGameObject()->Ghosts()[0];
	  }
	}
	return;
      }
    }
  }
}
