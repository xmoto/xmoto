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
  
  /*===========================================================================
  Optimizing
  ===========================================================================*/
  std::vector<StaticGeom *> GameRenderer::_FindGeomsByTexture(Texture *pTex) {
    std::vector<StaticGeom *> RetVal;
    
    /* Find all geoms with this texture */
    for(int i=0;i<m_Geoms.size();i++) {
      if(m_Geoms[i]->pTexture == pTex) RetVal.push_back(m_Geoms[i]);
    }
    return RetVal;
  }
  
  /*===========================================================================
  Called to prepare renderer for new level
  ===========================================================================*/
  void GameRenderer::prepareForNewLevel(void) {
//    printf("PREPARE!!\n");
    m_fCurrentHorizontalScrollShift = 0.0f;
    m_fDesiredHorizontalScrollShift = 0.0f;
    m_fNextParticleUpdate = 0.0f;
    
    #if defined(ALLOW_GHOST)
      /* Set initial ghost information position */
      if(getGameObject()->isGhostActive()) {
        m_GhostInfoPos = getGameObject()->getGhostBikeState()->CenterP + Vector2f(0,-1.5f);
        m_GhostInfoVel = Vector2f(0,0);

        m_fNextGhostInfoUpdate = 0.0f;
        m_nGhostInfoTrans = 255;
        
        if(m_pGhostReplay != NULL) {
          m_GhostInfoString = std::string(GAMETEXT_GHOSTOF) + m_pGhostReplay->getPlayerName() +
                              std::string("\n(") + m_ReplayDesc + std::string(")") +
                              std::string("\n(") + getParent()->formatTime(m_pGhostReplay->getFinishTime()) + std::string(")");
        }
        
        if(m_ReplayDesc == "")
          m_nGhostInfoTrans = 0;
      }
    #endif
        
    /* Optimize scene */
    std::vector<ConvexBlock *> &Blocks = getGameObject()->getBlocks();
    int nVertexBytes = 0;
  
    for(int i=0;i<Blocks.size();i++) {
      Vector2f Center;
      Sprite *pSprite;
      Texture *pTexture;
      GLuint GLName = 0;

      pTexture = NULL;
      
      if(Blocks[i]->pSrcBlock && (Blocks[i]->pSrcBlock->bBackground || Blocks[i]->pSrcBlock->bDynamic)) continue;

      if(Blocks[i]->pSrcBlock) {
        Center = Vector2f(Blocks[i]->pSrcBlock->fPosX,Blocks[i]->pSrcBlock->fPosY);

	pSprite = getParent()->m_theme.getSprite(SPRITE_TYPE_TEXTURE,
						 Blocks[i]->pSrcBlock->Texture);
        if(pSprite != NULL) {
	  try {
	    pTexture = pSprite->getTexture();
	    GLName = pTexture->nID;
	  } catch(Exception &e) {
	    Log("** Warning ** : Texture '%s' not found!",Blocks[i]->pSrcBlock->Texture.c_str());
	    getGameObject()->gameMessage(GAMETEXT_MISSINGTEXTURES,true);   
	  }
	} else {
          Log("** Warning ** : Texture '%s' not found!",Blocks[i]->pSrcBlock->Texture.c_str());
          getGameObject()->gameMessage(GAMETEXT_MISSINGTEXTURES,true);          
        }
      }
      else {
	pSprite = getParent()->m_theme.getSprite(SPRITE_TYPE_TEXTURE, "default");
	if(pSprite != NULL) {
	  pTexture = pSprite->getTexture();
	}

        if(pTexture != NULL) {
	  GLName = pTexture->nID;
	}
      }
      
      /* TODO: introduce non-static geoms and handle them differently */
      
      /* Define its box */
      Vector2f PBoxMin = (Blocks[i]->Vertices[0]->P + Center);
      Vector2f PBoxMax = (Blocks[i]->Vertices[0]->P + Center);      
      for(int j=0;j<Blocks[i]->Vertices.size();j++)
        addPointToAABB2f(PBoxMin,PBoxMax,(Blocks[i]->Vertices[j]->P + Center));
      
      /* Look at our list of static geoms, see if we can find a matching texture */     
      std::vector<StaticGeom *> GeomList = _FindGeomsByTexture(pTexture);
      
      /* Go through them, to see if we can find a suitable geom */
      StaticGeom *pSuitableGeom = NULL;
      for(int j=0;j<GeomList.size();j++) {
        /* Right then... if we add this polygon to this geom, how large will its 
           bounding box be? */
        Vector2f BoxMin = GeomList[j]->Min,BoxMax = GeomList[j]->Max;
        if(PBoxMin.x < BoxMin.x) BoxMin.x = PBoxMin.x;
        if(PBoxMin.y < BoxMin.y) BoxMin.y = PBoxMin.y;
        if(PBoxMax.x > BoxMax.x) BoxMax.x = PBoxMax.x;
        if(PBoxMax.y > BoxMax.y) BoxMax.y = PBoxMax.y;
        
        /* Too large? */
        if( (BoxMax.x - BoxMin.x) < 10.0f && (BoxMax.y - BoxMin.y) < 10.0f ) {
          /* Nope, use this */
          pSuitableGeom = GeomList[j];
          pSuitableGeom->Min = BoxMin;
          pSuitableGeom->Max = BoxMax;
          break;
        }
      }
      
      /* Did we get something? */
      if(pSuitableGeom == NULL) {
        /* No. Allocate new */
        pSuitableGeom = new StaticGeom;
        pSuitableGeom->Max = PBoxMax;
        pSuitableGeom->Min = PBoxMin;
        pSuitableGeom->pTexture = pTexture;
        m_Geoms.push_back(pSuitableGeom);
      }
      
      /* Nice, add polygon */
      StaticGeomPoly *pPoly = new StaticGeomPoly;
      pSuitableGeom->Polys.push_back(pPoly);
      
      pPoly->nNumVertices = Blocks[i]->Vertices.size();
      pPoly->pVertices = new StaticGeomCoord[ pPoly->nNumVertices ];
      pPoly->pTexCoords = new StaticGeomCoord[ pPoly->nNumVertices ];
      
      for(int j=0;j<pPoly->nNumVertices;j++) {
        pPoly->pVertices[j].x = Center.x + Blocks[i]->Vertices[j]->P.x;
        pPoly->pVertices[j].y = Center.y + Blocks[i]->Vertices[j]->P.y;        
        pPoly->pTexCoords[j].x = Blocks[i]->Vertices[j]->T.x; //(Center.x+Blocks[i]->Vertices[j]->P.x) * 0.25;
        pPoly->pTexCoords[j].y = Blocks[i]->Vertices[j]->T.y; //(Center.y+Blocks[i]->Vertices[j]->P.y) * 0.25;
      }          
      
      nVertexBytes += pPoly->nNumVertices * ( 4 * sizeof(float) );
      
      /* Use VBO optimization? */
      if(getParent()->useVBOs()) {
        /* Copy static coordinates unto video memory */
        getParent()->glGenBuffersARB(1, (GLuint *) &pPoly->nVertexBufferID);
        getParent()->glBindBufferARB(GL_ARRAY_BUFFER_ARB,pPoly->nVertexBufferID);
        getParent()->glBufferDataARB(GL_ARRAY_BUFFER_ARB,pPoly->nNumVertices*2*sizeof(float),(void *)pPoly->pVertices,GL_STATIC_DRAW_ARB);
        
        getParent()->glGenBuffersARB(1, (GLuint *) &pPoly->nTexCoordBufferID);
        getParent()->glBindBufferARB(GL_ARRAY_BUFFER_ARB,pPoly->nTexCoordBufferID);
        getParent()->glBufferDataARB(GL_ARRAY_BUFFER_ARB,pPoly->nNumVertices*2*sizeof(float),(void *)pPoly->pTexCoords,GL_STATIC_DRAW_ARB);
      }
    }        
    
    Log("Number of optimized geoms: %d",m_Geoms.size());
    Log("GL: %d kB vertex buffers",nVertexBytes/1024);
  }

  /*===========================================================================
  Called when we don't want to play the level anymore
  ===========================================================================*/
  void GameRenderer::unprepareForNewLevel(void) {
    if(m_pInGameStats)
      m_pInGameStats->showWindow(false);
    
    /* Free any particles left */
    for(int i=0;i<m_Particles.size();i++)
      delete m_Particles[i];    
    m_Particles.clear();
    
    /* Clean up optimized scene */
    for(int i=0;i<m_Geoms.size();i++) { 
      for(int j=0;j<m_Geoms[i]->Polys.size();j++) { 
        if(m_Geoms[i]->Polys[j]->nVertexBufferID) {
          getParent()->glDeleteBuffersARB(1, (GLuint *) &m_Geoms[i]->Polys[j]->nVertexBufferID);
          getParent()->glDeleteBuffersARB(1, (GLuint *) &m_Geoms[i]->Polys[j]->nTexCoordBufferID);
        }
      
        delete [] m_Geoms[i]->Polys[j]->pTexCoords;
        delete [] m_Geoms[i]->Polys[j]->pVertices;
        delete m_Geoms[i]->Polys[j];
      }
      delete m_Geoms[i];
    }
    m_Geoms.clear();
  }  

  /*===========================================================================
  Minimap rendering
  ===========================================================================*/
  #define MINIMAPZOOM 5.0f
  #define MINIMAPALPHA 128
  #define MINIVERTEX(Px,Py) \
    getParent()->glVertex(x + nWidth/2 + (float)(Px + m_Scroll.x)*MINIMAPZOOM, \
                          y + nHeight/2 - (float)(Py + m_Scroll.y)*MINIMAPZOOM);    

  void GameRenderer::renderMiniMap(int x,int y,int nWidth,int nHeight) {
    getParent()->drawBox(Vector2f(x,y),Vector2f(x+nWidth,y+nHeight),1,
                         MAKE_COLOR(0,0,0,MINIMAPALPHA),MAKE_COLOR(255,255,255,MINIMAPALPHA));
    getParent()->scissorGraphics(x+1,y+1,nWidth-2,nHeight-2);
    glEnable(GL_SCISSOR_TEST);
    glLoadIdentity();
        
    MotoGame *pGame = getGameObject();
    std::vector<ConvexBlock *> &Blocks = pGame->getBlocks();

    /* Render non-dynamic blocks */
    for(int i=0;i<Blocks.size();i++) {
      Vector2f Center;

      if(Blocks[i]->pSrcBlock) {
        if(Blocks[i]->pSrcBlock->bBackground || Blocks[i]->pSrcBlock->bDynamic) continue;
        Center = Vector2f(Blocks[i]->pSrcBlock->fPosX,Blocks[i]->pSrcBlock->fPosY);
      }

      glBegin(GL_POLYGON);
      glColor3f(0.5,0.5,0.5);
      for(int j=0;j<Blocks[i]->Vertices.size();j++) {
        Vector2f P = Center + Blocks[i]->Vertices[j]->P;
        
        MINIVERTEX(P.x,P.y);                  
      }
      glEnd();
    }    
    
    std::vector<DynamicBlock *> &DynBlocks = pGame->getDynBlocks();

    /* Render dynamic blocks */
    for(int i=0;i<DynBlocks.size();i++) {
      DynamicBlock *pDB = DynBlocks[i];
      
      /* Don't draw background blocks */
      if(pDB->bBackground) continue;
      
			/* Build rotation matrix for block */
			float fR[4]; 
			fR[0] = cos(pDB->fRotation); fR[1] = -sin(pDB->fRotation);
			fR[2] = sin(pDB->fRotation); fR[3] = cos(pDB->fRotation);

      for(int j=0;j<pDB->ConvexBlocks.size();j++) {
        ConvexBlock *pCB = pDB->ConvexBlocks[j];
  
        glBegin(GL_POLYGON);
        glColor3f(0.5,0.5,0.5);
        for(int k=0;k<pCB->Vertices.size();k++) {
          ConvexBlockVertex *pVertex = pCB->Vertices[k];
        
				  /* Transform vertex */
				  Vector2f Tv = Vector2f(pVertex->P.x * fR[0] + pVertex->P.y * fR[1],
				                          pVertex->P.x * fR[2] + pVertex->P.y * fR[3]);
          Tv += pDB->Position;
            				  
				  /* Put vertex */
				  glTexCoord2f(pVertex->T.x,pVertex->T.y);				  
          MINIVERTEX(Tv.x,Tv.y);                  
        }
        glEnd();        
      }
    }
    
    getParent()->drawCircle(Vector2f(x + nWidth/2 + (float)(pGame->getBikeState()->CenterP.x + m_Scroll.x)*MINIMAPZOOM,
                                     y + nHeight/2 - (float)(pGame->getBikeState()->CenterP.y + m_Scroll.y)*MINIMAPZOOM),
                            3,0,MAKE_COLOR(255,255,255,255),0);

    #if defined(ALLOW_GHOST)
      /* Render ghost position too? */
      if(getGameObject()->isGhostActive()) {
        getParent()->drawCircle(Vector2f(x + nWidth/2 + (float)(pGame->getGhostBikeState()->CenterP.x + m_Scroll.x)*MINIMAPZOOM,
                                        y + nHeight/2 - (float)(pGame->getGhostBikeState()->CenterP.y + m_Scroll.y)*MINIMAPZOOM),
                                3,0,MAKE_COLOR(96,96,150,255),0);
      }
    #endif
                            
    for(int i=0;i<pGame->getEntities().size();i++) {
      if(pGame->getEntities()[i]->Type == ET_ENDOFLEVEL) {
        getParent()->drawCircle(Vector2f(x + nWidth/2 + (float)(pGame->getEntities()[i]->Pos.x + m_Scroll.x)*MINIMAPZOOM,
                                        y + nHeight/2 - (float)(pGame->getEntities()[i]->Pos.y + m_Scroll.y)*MINIMAPZOOM),
                                3,0,MAKE_COLOR(255,0,255,255),0);
        
      }
      else if(pGame->getEntities()[i]->Type == ET_STRAWBERRY) {
        getParent()->drawCircle(Vector2f(x + nWidth/2 + (float)(pGame->getEntities()[i]->Pos.x + m_Scroll.x)*MINIMAPZOOM,
                                        y + nHeight/2 - (float)(pGame->getEntities()[i]->Pos.y + m_Scroll.y)*MINIMAPZOOM),
                                3,0,MAKE_COLOR(255,0,0,255),0);
        
      }
    }
        
    glDisable(GL_SCISSOR_TEST);
    getParent()->scissorGraphics(0,0,getParent()->getDispWidth(),getParent()->getDispHeight());
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

  void GameRenderer::moveCamera(float px, float py) {
    m_cameraOffsetX += px;
    m_cameraOffsetY += py;
  }

  void GameRenderer::initCamera() {
    m_cameraOffsetX = CAMERA_OFFSETX_DEFAULT;
    m_cameraOffsetY = CAMERA_OFFSETY_DEFAULT;
  }

  /*===========================================================================
  Main rendering function
  ===========================================================================*/
  void GameRenderer::render(void) {
    /* Update time */    
    m_pInGameStats->showWindow(true);
    m_pPlayTime->setCaption(getParent()->formatTime(getGameObject()->getTime()));

    /* Prepare for rendering frame */    
    if(getGameObject()->getTime() > m_fNextParticleUpdate) {
      _UpdateParticles(0.025f);
      m_fNextParticleUpdate = getGameObject()->getTime() + 0.025f; 
    }
    
    m_Scroll = -getGameObject()->getBikeState()->CenterP; /* Determine scroll */    
    m_fZoom = 60.0f;    
    
    /* Driving direction? */
    if(getGameObject()->getBikeState()->Dir == DD_RIGHT) {
      m_fDesiredHorizontalScrollShift = -4;
    }
    else if(getGameObject()->getBikeState()->Dir == DD_LEFT) {
      m_fDesiredHorizontalScrollShift = 4;
    }
    
    if(m_fDesiredHorizontalScrollShift != m_fCurrentHorizontalScrollShift) {
      float d = m_fDesiredHorizontalScrollShift - m_fCurrentHorizontalScrollShift;
      if(fabs(d)<0.25f) {
        m_fCurrentHorizontalScrollShift = m_fDesiredHorizontalScrollShift;
      }
      else if(d < 0.0f) {
        m_fCurrentHorizontalScrollShift -= 0.1f * m_fSpeedMultiply;
      }
      else if(d > 0.0f) {
        m_fCurrentHorizontalScrollShift += 0.1f * m_fSpeedMultiply;
      }
    }

    m_Scroll += Vector2f(m_fCurrentHorizontalScrollShift,0.0f);

    glLoadIdentity();

    /* SKY! */
    if(!m_bUglyMode)
			_RenderSky();

    /* Perform scaling/translation */    
    glScalef(m_fScale * ((float)getParent()->getDispHeight()) / getParent()->getDispWidth(), m_fScale,1);
    //glRotatef(getGameObject()->getTime()*100,0,0,1); /* Uncomment this line if you want to vomit :) */
    glTranslatef(m_Scroll.x - m_cameraOffsetX, m_Scroll.y - m_cameraOffsetY, 0);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
      
    if(m_Quality != GQ_LOW && !m_bUglyMode) {
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
    
    /* ... the entities ... */
    //_RenderEntities();
    
#if defined(ALLOW_GHOST)
    if(getGameObject()->isGhostActive()) {
      /* Render ghost - ugly mode? */
      if(m_bUglyMode) {
        _RenderBike(getGameObject()->getGhostBikeState(), getGameObject()->getBikeParams(), m_theme->getGhostTheme());
      }    
      else {
        /* No not ugly, fancy! Render into overlay? */      
        if(m_bGhostMotionBlur && getParent()->useFBOs()) {
          m_Overlay.beginRendering();
          m_Overlay.fade(0.15);
        }
        _RenderBike(getGameObject()->getGhostBikeState(), getGameObject()->getBikeParams(), m_theme->getGhostTheme());
        
        if(m_bGhostMotionBlur && getParent()->useFBOs()) {
          GLuint nOverlayTextureID = m_Overlay.endRendering();
          m_Overlay.present();
        }
        
        if(m_nGhostInfoTrans > 0) {
          _RenderInGameText(m_GhostInfoPos,m_GhostInfoString,MAKE_COLOR(255,255,255,m_nGhostInfoTrans));
          if(getGameObject()->getTime() > m_fNextGhostInfoUpdate) {
            if(getGameObject()->getTime() - m_fNextGhostInfoUpdate > 0.05f) {
              if(getGameObject()->getTime() > 3.0f) m_nGhostInfoTrans=0;
              
              m_GhostInfoPos = getGameObject()->getGhostBikeState()->CenterP + Vector2f(0,-1.5);
            }
            else {
              if(getGameObject()->getTime() > 3.0f) m_nGhostInfoTrans-=16;
              
              m_GhostInfoVel = ((getGameObject()->getGhostBikeState()->CenterP + Vector2f(0,-1.5)) - m_GhostInfoPos) * 0.2f;
              m_GhostInfoPos += m_GhostInfoVel;
            }
            m_fNextGhostInfoUpdate = getGameObject()->getTime() + 0.025f;        
          }
        }      
      }
    }
#endif

    /* ... followed by the bike ... */
    _RenderBike(getGameObject()->getBikeState(), getGameObject()->getBikeParams(), m_theme->getPlayerTheme());
    
    if(m_Quality == GQ_HIGH && !m_bUglyMode) {
      /* Render particles (front!) */    
      _RenderParticles();
    }
    
    /* ... and finally the foreground sprites! */
    _RenderSprites(true,false);
    
    //glBegin(GL_LINE_STRIP);
    //glColor3f(1,1,1);
    //glVertex2f(getGameObject()->m_PrevFrontWheelP.x,getGameObject()->m_PrevFrontWheelP.y);
    //glVertex2f(getGameObject()->getBikeState()->FrontWheelP.x,getGameObject()->getBikeState()->FrontWheelP.y);
    //glEnd();
    //    
    //glBegin(GL_LINE_STRIP);
    //glColor3f(1,1,1);
    //glVertex2f(getGameObject()->m_PrevRearWheelP.x,getGameObject()->m_PrevRearWheelP.y);
    //glVertex2f(getGameObject()->getBikeState()->RearWheelP.x,getGameObject()->getBikeState()->RearWheelP.y);
    //glEnd();
        
    if(isDebug()) {
      /* Draw some collision handling debug info */
      CollisionSystem *pc = getGameObject()->getCollisionHandler();
      for(int i=0;i<pc->m_CheckedLines.size();i++) {
        glLineWidth(3);
 			  glBegin(GL_LINE_STRIP);
  		  glColor3f(1,0,0);
  		  glVertex2f(pc->m_CheckedLines[i]->x1,pc->m_CheckedLines[i]->y1);
  		  glVertex2f(pc->m_CheckedLines[i]->x2,pc->m_CheckedLines[i]->y2);
  		  glEnd();
        glLineWidth(2);
      }
      for(int i=0;i<pc->m_CheckedCells.size();i++) {
        glBegin(GL_LINE_LOOP);
        glColor3f(1,0,0);
        glVertex2f(pc->m_CheckedCells[i].x1,pc->m_CheckedCells[i].y1);
        glVertex2f(pc->m_CheckedCells[i].x2,pc->m_CheckedCells[i].y1);
        glVertex2f(pc->m_CheckedCells[i].x2,pc->m_CheckedCells[i].y2);
        glVertex2f(pc->m_CheckedCells[i].x1,pc->m_CheckedCells[i].y2);
        glEnd();
      }
      for(int i=0;i<pc->m_CheckedLinesW.size();i++) {
        glLineWidth(1);
 			  glBegin(GL_LINE_STRIP);
  		  glColor3f(0,1,0);
  		  glVertex2f(pc->m_CheckedLinesW[i]->x1,pc->m_CheckedLinesW[i]->y1);
  		  glVertex2f(pc->m_CheckedLinesW[i]->x2,pc->m_CheckedLinesW[i]->y2);
  		  glEnd();
        glLineWidth(1);
      }
      for(int i=0;i<pc->m_CheckedCellsW.size();i++) {
        glBegin(GL_LINE_LOOP);
        glColor3f(0,1,0);
        glVertex2f(pc->m_CheckedCellsW[i].x1,pc->m_CheckedCellsW[i].y1);
        glVertex2f(pc->m_CheckedCellsW[i].x2,pc->m_CheckedCellsW[i].y1);
        glVertex2f(pc->m_CheckedCellsW[i].x2,pc->m_CheckedCellsW[i].y2);
        glVertex2f(pc->m_CheckedCellsW[i].x1,pc->m_CheckedCellsW[i].y2);
        glEnd();
      }
    }
    
    //const std::vector<LineSoup *> &LSoups = getGameObject()->getCollisionHandler()->getSoups();
    //for(int i=0;i<LSoups.size();i++) {
    //  glLineWidth(3);
    //  for(int j=0;j<LSoups[i]->cNumLines;j++) {
  		//	glBegin(GL_LINE_STRIP);
	  	//	glColor3f(LSoups[i]->r,LSoups[i]->g,LSoups[i]->b);
	  	//	glVertex2f(LSoups[i]->Lines[j].x1,LSoups[i]->Lines[j].y1);
	  	//	glVertex2f(LSoups[i]->Lines[j].x2,LSoups[i]->Lines[j].y2);
	  	//	glEnd();
    //  }
    //  glLineWidth(1);
    //  glBegin(GL_LINE_LOOP);
  		//glColor3f(LSoups[i]->r,LSoups[i]->g,LSoups[i]->b);
  		//glVertex2f(LSoups[i]->Min.x,LSoups[i]->Min.y);
  		//glVertex2f(LSoups[i]->Max.x,LSoups[i]->Min.y);
  		//glVertex2f(LSoups[i]->Max.x,LSoups[i]->Max.y);
  		//glVertex2f(LSoups[i]->Min.x,LSoups[i]->Max.y);
    //  glEnd();
    //}
        
    /* Hmm, in debug-mode we'd also like to see the zones and stuff */
    if(isDebug()) {    
      /* Render debug info */
      _RenderDebugInfo();

      /* Zones */
      /* TODO: port this to use new transform */
      //for(int k=0;k<getGameObject()->getLevelSrc()->getZoneList().size();k++) {
      //  LevelZone *pZone = getGameObject()->getLevelSrc()->getZoneList()[k];
      //  
      //  for(int u=0;u<pZone->Prims.size();u++) {
      //    if(pZone->Prims[u]->Type == LZPT_BOX) {
      //      glBegin(GL_LINE_LOOP);
      //      glColor3f(0,1,0);
      //      getParent()->glVertex( getParent()->getDispWidth()/2 + (pZone->Prims[u]->fLeft + m_Scroll.x)*60.0f,
      //                getParent()->getDispHeight()/2 - (pZone->Prims[u]->fTop + m_Scroll.y)*60.0f );
      //      getParent()->glVertex( getParent()->getDispWidth()/2 + (pZone->Prims[u]->fRight + m_Scroll.x)*60.0f,
      //                getParent()->getDispHeight()/2 - (pZone->Prims[u]->fTop + m_Scroll.y)*60.0f );
      //      getParent()->glVertex( getParent()->getDispWidth()/2 + (pZone->Prims[u]->fRight + m_Scroll.x)*60.0f,
      //                getParent()->getDispHeight()/2 - (pZone->Prims[u]->fBottom + m_Scroll.y)*60.0f );
      //      getParent()->glVertex( getParent()->getDispWidth()/2 + (pZone->Prims[u]->fLeft + m_Scroll.x)*60.0f,
      //                getParent()->getDispHeight()/2 - (pZone->Prims[u]->fBottom + m_Scroll.y)*60.0f );
      //      glEnd();
      //      
      //      Vector2f TP(getParent()->getDispWidth()/2 + (pZone->Prims[u]->fLeft + m_Scroll.x)*60.0f,
      //                getParent()->getDispHeight()/2 - (pZone->Prims[u]->fTop + m_Scroll.y)*60.0f);
      //      float th=12,tw=pZone->ID.length()*8;
      //      if(TP.x < 0) TP.x=0;
      //      if(TP.y < 0) TP.y=0;
      //      if(TP.x > getParent()->getDispWidth()-pZone->ID.length()*8) TP.x=getParent()->getDispWidth()-pZone->ID.length()*8;
      //      if(TP.y > getParent()->getDispHeight()-12) TP.y=getParent()->getDispHeight()-12;
      //      if(TP.x > getParent()->getDispWidth()/2 + (pZone->Prims[u]->fRight + m_Scroll.x)*60.0f) continue;
      //      if(TP.y > getParent()->getDispHeight()/2 - (pZone->Prims[u]->fBottom + m_Scroll.y)*60.0f) continue;
      //      if(TP.x + tw < getParent()->getDispWidth()/2 + (pZone->Prims[u]->fLeft + m_Scroll.x)*60.0f) continue;
      //      if(TP.y + th < getParent()->getDispHeight()/2 - (pZone->Prims[u]->fTop + m_Scroll.y)*60.0f) continue;
      //      getParent()->drawText(TP,pZone->ID,0,MAKE_COLOR(0,255,0,255),true);               
      //    }
      //  }
      //}
    }        

    glLoadIdentity();
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,getParent()->getDispWidth(),0,getParent()->getDispHeight(),-1,1);
    glMatrixMode(GL_MODELVIEW);

    /* And then the game messages */
    _RenderGameMessages();            
    
    /* If there's strawberries in the level, tell the user how many there's left */
    _RenderGameStatus();
  }

  /*===========================================================================
  Game status rendering
  ===========================================================================*/
  void GameRenderer::_RenderGameStatus(void) {
    AnimationSprite* pType;
    MotoGame *pGame = getGameObject();

    int nStrawberriesLeft = pGame->countEntitiesByType(ET_STRAWBERRY);
    int nQuantity = 0;
    pType = (AnimationSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_ANIMATION, "Flower");
    
    if(nStrawberriesLeft > 0) {
      pType = (AnimationSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_ANIMATION, "Strawberry");
      nQuantity = nStrawberriesLeft;
    }
            
    float x1 = 90;
    float y1 = -2;
    float x2 = 115;
    float y2 = 23;

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
        C=Vector2f(getParent()->getDispWidth()/2 + (float)(pArrow->ArrowPointerPos.x + m_Scroll.x)*m_fZoom,
                  getParent()->getDispHeight()/2 - (float)(pArrow->ArrowPointerPos.y + m_Scroll.y)*m_fZoom);      
      }
      else if(pArrow->nArrowPointerMode == 2) {          
        C.x=(getParent()->getDispWidth() * pArrow->ArrowPointerPos.x) / 800.0f;
        C.y=(getParent()->getDispHeight() * pArrow->ArrowPointerPos.y) / 600.0f;
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
      pType = (MiscSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_MISC, "Arrow");
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
	  UITextDraw::printRaw(v_font,400 - (x2-x1)/2,pMsg->Pos[1]*600,pMsg->Text,MAKE_COLOR(255,255,255,pMsg->nAlpha));
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

    for(int i=0;i<pGame->getEntities().size();i++) {
      pEnt = pGame->getEntities()[i];

      switch(pEnt->Type) {
        case ET_SPRITE:
	        /* Middleground? (not foreground, not background) */
	        if(pEnt->fSpriteZ == 0.0f && !bForeground && !bBackground) {
	          _RenderSprite(pEnt);	
	        } 
	        else {
	          /* In front? */
	          if(pEnt->fSpriteZ > 0.0f && bForeground) {
	            _RenderSprite(pEnt);
	          } 
	          else {
	            /* Those in back? */
	            if(pEnt->fSpriteZ < 0.0f && bBackground) {
	              _RenderSprite(pEnt);
	            }
	          }
	        }
	        break;
        case ET_WRECKER:
        case ET_ENDOFLEVEL:
        case ET_STRAWBERRY:
	        if(!bForeground && !bBackground) {
	          _RenderSprite(pEnt);
	        }
	        break;
      }
    }
  }

  /*===========================================================================
  Render a sprite
  ===========================================================================*/
  void GameRenderer::_RenderSprite(Entity *pSprite) {  
    Sprite* v_spriteType;
    AnimationSprite* v_animationSpriteType;
    DecorationSprite* v_decorationSpriteType;
    float v_centerX;
    float v_centerY;
    float v_width;
    float v_height;
    std::string v_sprite_type;

    switch(pSprite->Type) {
    case ET_SPRITE:
      v_sprite_type = pSprite->SpriteType;
      break;
    case ET_WRECKER:
      v_sprite_type = "Wrecker";
      break;
    case ET_ENDOFLEVEL:
      v_sprite_type = "Flower";
      break;
    case ET_STRAWBERRY:
      v_sprite_type = "Strawberry";
      break;
    }

    /* search the sprite as an animation */
    v_animationSpriteType = (AnimationSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_ANIMATION, v_sprite_type);
    /* if the sprite is not an animation, it's perhaps a decoration */
    if(v_animationSpriteType != NULL) {
      v_spriteType = v_animationSpriteType;
      v_centerX = v_animationSpriteType->getCenterX();
      v_centerY = v_animationSpriteType->getCenterY();
      v_width   = v_animationSpriteType->getWidth();
      v_height  = v_animationSpriteType->getHeight();
    } else {
      v_decorationSpriteType = (DecorationSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_DECORATION, v_sprite_type);
      v_spriteType = v_decorationSpriteType;
      if(v_decorationSpriteType != NULL) {
	v_centerX = v_decorationSpriteType->getCenterX();
	v_centerY = v_decorationSpriteType->getCenterY();
	v_width   = v_decorationSpriteType->getWidth();
	v_height  = v_decorationSpriteType->getHeight();
      }
    }

    if(v_spriteType != NULL) {
      /* Draw it */
      Vector2f p0,p1,p2,p3;
      
      p0 = Vector2f(pSprite->Pos.x,pSprite->Pos.y) +
      Vector2f(-v_centerX,-v_centerY);
      p1 = Vector2f(pSprite->Pos.x+v_width,pSprite->Pos.y) +
      Vector2f(-v_centerX,-v_centerY);
      p2 = Vector2f(pSprite->Pos.x+v_width,pSprite->Pos.y+v_height) +
      Vector2f(-v_centerX,-v_centerY);
      p3 = Vector2f(pSprite->Pos.x,pSprite->Pos.y+v_height) +
      Vector2f(-v_centerX,-v_centerY);
            
      if(v_spriteType->getBlendMode() == SPRITE_BLENDMODE_ADDITIVE) {
        _RenderAdditiveBlendedSection(v_spriteType->getTexture(),p0,p1,p2,p3);      
      }
      else {
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GEQUAL,0.5f);      
        _RenderAlphaBlendedSection(v_spriteType->getTexture(),p0,p1,p2,p3);      
        glDisable(GL_ALPHA_TEST);
      }
    
      /* If this is debug-mode, also draw entity's area of effect */
      if(isDebug()) {
        Vector2f C = Vector2f( getParent()->getDispWidth()/2 + (float)(pSprite->Pos.x + m_Scroll.x)*m_fZoom,
                               getParent()->getDispHeight()/2 - (float)(pSprite->Pos.y + m_Scroll.y)*m_fZoom );
        Vector2f Cr = Vector2f( getParent()->getDispWidth()/2 + (float)((pSprite->Pos.x + pSprite->fSize) + m_Scroll.x)*m_fZoom,
                                getParent()->getDispHeight()/2 - (float)(pSprite->Pos.y + m_Scroll.y)*m_fZoom );
        float r = (C-Cr).length();
        getParent()->drawCircle(C,r,1,0,MAKE_COLOR(0,255,0,255));
      }
    }
  }
     
  /*===========================================================================
  Blocks (dynamic)
  ===========================================================================*/
  void GameRenderer::_RenderDynamicBlocks(bool bBackground) {
    MotoGame *pGame = getGameObject();

		/* Render all dynamic blocks */
	  std::vector<DynamicBlock *> &Blocks = getGameObject()->getDynBlocks();

		for(int i=0;i<Blocks.size();i++) {						
		  /* Are we rendering background blocks or what? */
		  if(Blocks[i]->bBackground != bBackground) continue;
		  
			/* Build rotation matrix for block */
			float fR[4]; 
			fR[0] = cos(Blocks[i]->fRotation); fR[1] = -sin(Blocks[i]->fRotation);
			fR[2] = sin(Blocks[i]->fRotation); fR[3] = cos(Blocks[i]->fRotation);
			
			/* Determine texture... this is so ingredibly ugly... TODO: no string lookups here */
			Texture *pTexture = NULL;
			if(!m_bUglyMode) {
			  Sprite *pSprite;
			  pSprite = getParent()->m_theme.getSprite(SPRITE_TYPE_TEXTURE, Blocks[i]->pSrcBlock->Texture);
			  if(pSprite != NULL) {
			    pTexture = pSprite->getTexture();
			  }
			}
			GLuint GLName = 0;
			if(pTexture != NULL) GLName = pTexture->nID;

			for(int j=0;j<Blocks[i]->ConvexBlocks.size();j++) {				
			  if(!m_bUglyMode) {
  		    glBindTexture(GL_TEXTURE_2D,GLName);				  				  
				  glEnable(GL_TEXTURE_2D);      
				  glBegin(GL_POLYGON);
				}
				else
  				glBegin(GL_LINE_LOOP);
				
				glColor3f(1,1,1);				  
				for(int k=0;k<Blocks[i]->ConvexBlocks[j]->Vertices.size();k++) {				    
				  ConvexBlockVertex *pVertex = Blocks[i]->ConvexBlocks[j]->Vertices[k];

				  /* Transform vertex */
				  Vector2f Tv = Vector2f(pVertex->P.x * fR[0] + pVertex->P.y * fR[1],
				                          pVertex->P.x * fR[2] + pVertex->P.y * fR[3]);
          Tv += Blocks[i]->Position;				                          
            				  
				  /* Put vertex */
				  if(!m_bUglyMode) glTexCoord2f(pVertex->T.x,pVertex->T.y);
				  
				  glVertex2f(Tv.x,Tv.y);
				}
				glEnd();	            
				
				if(!m_bUglyMode) glDisable(GL_TEXTURE_2D);
			}
		}		  
  }
       
  /*===========================================================================
  Blocks (static)
  ===========================================================================*/
  void GameRenderer::_RenderBlocks(void) {
    MotoGame *pGame = getGameObject();

		/* Ugly mode? */
		if(m_bUglyMode) {
			/* Render all non-background blocks */
	    std::vector<LevelBlock *> &Blocks = getGameObject()->getLevelSrc()->getBlockList();
	
			for(int i=0;i<Blocks.size();i++) {
				if(!Blocks[i]->bBackground && !Blocks[i]->bDynamic) {
					glBegin(GL_LINE_LOOP);
					glColor3f(1,1,1);
					for(int j=0;j<Blocks[i]->Vertices.size();j++) {
						glVertex2f(Blocks[i]->Vertices[j]->fX + Blocks[i]->fPosX,
						           Blocks[i]->Vertices[j]->fY + Blocks[i]->fPosY);
					}
					glEnd();
				}
			}
			
			/* Also render the level limits */
			glBegin(GL_LINE_LOOP);
			glColor3f(1,1,1);
			glVertex2f(getGameObject()->getLevelSrc()->getLeftLimit(),getGameObject()->getLevelSrc()->getTopLimit());
			glVertex2f(getGameObject()->getLevelSrc()->getRightLimit(),getGameObject()->getLevelSrc()->getTopLimit());
			glVertex2f(getGameObject()->getLevelSrc()->getRightLimit(),getGameObject()->getLevelSrc()->getBottomLimit());
			glVertex2f(getGameObject()->getLevelSrc()->getLeftLimit(),getGameObject()->getLevelSrc()->getBottomLimit());
			glEnd();
		}
		else {
			/* Render all non-background blocks */
			/* Static geoms... */
			for(int i=0;i<m_Geoms.size();i++) {
			  if(m_Geoms[i]->pTexture != NULL) {
			    glBindTexture(GL_TEXTURE_2D,m_Geoms[i]->pTexture->nID);
			  } else {
			    glBindTexture(GL_TEXTURE_2D,0);	/* no texture */
			  }			    

				glEnable(GL_TEXTURE_2D);      
				glColor3f(1,1,1);
	      
				/* VBO optimized? */
				if(getParent()->useVBOs()) {
					for(int j=0;j<m_Geoms[i]->Polys.size();j++) {          
						StaticGeomPoly *pPoly = m_Geoms[i]->Polys[j];
						getParent()->glBindBufferARB(GL_ARRAY_BUFFER_ARB,pPoly->nVertexBufferID);
						glVertexPointer(2,GL_FLOAT,0,(char *)NULL);
						getParent()->glBindBufferARB(GL_ARRAY_BUFFER_ARB,pPoly->nTexCoordBufferID);
						glTexCoordPointer(2,GL_FLOAT,0,(char *)NULL);
						glDrawArrays(GL_POLYGON,0,pPoly->nNumVertices);
					}      
				}
				else {
					for(int j=0;j<m_Geoms[i]->Polys.size();j++) {          
						glBegin(GL_POLYGON);
						glColor3f(1,1,1);
						for(int k=0;k<m_Geoms[i]->Polys[j]->nNumVertices;k++) {
							glTexCoord2f(m_Geoms[i]->Polys[j]->pTexCoords[k].x,m_Geoms[i]->Polys[j]->pTexCoords[k].y);
							glVertex2f(m_Geoms[i]->Polys[j]->pVertices[k].x,m_Geoms[i]->Polys[j]->pVertices[k].y);
						}
						glEnd();
					}
				}
	            
				glDisable(GL_TEXTURE_2D);
			}
	    
			/* Render all special edges (if quality!=low) */
			if(m_Quality != GQ_LOW) {
				for(int i=0;i<pGame->getOverlayEdges().size();i++) {
					OverlayEdge *pEdge = pGame->getOverlayEdges()[i];
	        
					switch(pEdge->Effect) {
						case EE_GRASS:
						case EE_GRASSALT:
						case EE_BLUEBRICKS:
						case EE_GRAYBRICKS:
						case EE_REDBRICKS: {
						    GLuint GLName = 0;
						    float fXScale,fDepth;
						    if(pEdge->Effect == EE_GRASS) {						    
						      EffectSprite* pType;
						      pType = (EffectSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_EFFECT, "EdgeGrass1");
						      if(pType != NULL) {
							      GLName = pType->getTexture()->nID;
							      fXScale = 0.5f;
							      fDepth = 0.3;
						      }
						    }
						    else if(pEdge->Effect == EE_GRASSALT) {						    
						      EffectSprite* pType;
						      pType = (EffectSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_EFFECT, "EdgeGrassAlt1");
						      if(pType != NULL) {
							      GLName = pType->getTexture()->nID;
							      fXScale = 0.5f;
							      fDepth = 0.6;
						      }
						    }
						    else if(pEdge->Effect == EE_REDBRICKS) {						    
						      EffectSprite* pType;
						      pType = (EffectSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_EFFECT, "EdgeRedBricks1");
						      if(pType != NULL) {
							      GLName = pType->getTexture()->nID;
							      fXScale = 0.8f;
							      fDepth = 0.3;
						      }
						    }						
						    else if(pEdge->Effect == EE_GRAYBRICKS) {
						      EffectSprite* pType;
						      pType = (EffectSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_EFFECT, "EdgeGrayBricks1");
						      if(pType != NULL) {
							      GLName = pType->getTexture()->nID;
							      fXScale = 0.8f;
							      fDepth = 0.3;
						      }
						    }
						    else if(pEdge->Effect == EE_BLUEBRICKS) {
						      EffectSprite* pType;
						      pType = (EffectSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_EFFECT, "EdgeBlueBricks1");
						      if(pType != NULL) {
							      GLName = pType->getTexture()->nID;
							      fXScale = 0.8f;
							      fDepth = 0.3;
						      }
						    }
						
							  if(GLName != 0) {
								  glEnable(GL_BLEND); 
								  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
								  glBindTexture(GL_TEXTURE_2D,GLName);
								  glEnable(GL_TEXTURE_2D);
								  glBegin(GL_POLYGON);
								  glColor3f(1,1,1);
								  glTexCoord2f((pEdge->pSrcBlock->fPosX+pEdge->P1.x)*fXScale,0.01);
								  _Vertex(pEdge->P1 + Vector2f(pEdge->pSrcBlock->fPosX,pEdge->pSrcBlock->fPosY));
								  glTexCoord2f((pEdge->pSrcBlock->fPosX+pEdge->P2.x)*fXScale,0.01);
								  _Vertex(pEdge->P2 + Vector2f(pEdge->pSrcBlock->fPosX,pEdge->pSrcBlock->fPosY));
								  glTexCoord2f((pEdge->pSrcBlock->fPosX+pEdge->P2.x)*fXScale,0.99);
								  _Vertex(pEdge->P2 + Vector2f(pEdge->pSrcBlock->fPosX,pEdge->pSrcBlock->fPosY) + Vector2f(0,-fDepth));
								  glTexCoord2f((pEdge->pSrcBlock->fPosX+pEdge->P1.x)*fXScale,0.99);
								  _Vertex(pEdge->P1 + Vector2f(pEdge->pSrcBlock->fPosX,pEdge->pSrcBlock->fPosY) + Vector2f(0,-fDepth));
								  glEnd();
								  glDisable(GL_TEXTURE_2D);
								  glDisable(GL_BLEND);
							  }
							}
							break;
					}
				}
			}
		}
  }  

  /*===========================================================================
  Sky.
  ===========================================================================*/
  void GameRenderer::_RenderSky(void) {
    MotoGame *pGame = getGameObject();
    EffectSprite* pType;

    /* Render sky - but which? */
    const std::string &SkyName = pGame->getLevelSrc()->getLevelInfo()->Sky;
    if(SkyName == "" || SkyName == "sky1") {
      pType = (EffectSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_EFFECT, "Sky1");

      if(pType != NULL) {
        glBindTexture(GL_TEXTURE_2D, pType->getTexture()->nID);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_POLYGON);   
        glColor3f(1,1,1);   
        glTexCoord2f(-m_Scroll.x*0.015,m_Scroll.y*0.015);
        getParent()->glVertex(0,0);
        glTexCoord2f(-m_Scroll.x*0.015+0.5,m_Scroll.y*0.015);
        getParent()->glVertex(getParent()->getDispWidth(),0);
        glTexCoord2f(-m_Scroll.x*0.015+0.5,m_Scroll.y*0.015+0.5);
        getParent()->glVertex(getParent()->getDispWidth(),getParent()->getDispHeight());
        glTexCoord2f(-m_Scroll.x*0.015,m_Scroll.y*0.015+0.5);
        getParent()->glVertex(0,getParent()->getDispHeight());
        glEnd();
        glDisable(GL_TEXTURE_2D); 
      }   
    }
    else if(SkyName == "sky2") {
      pType = (EffectSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_EFFECT, "Sky2");

      if(pType != NULL) {
        glBindTexture(GL_TEXTURE_2D, pType->getTexture()->nID);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_POLYGON);   
        glColor3f(1,1,1);   
        glTexCoord2f(-m_Scroll.x*0.015,m_Scroll.y*0.015);
        getParent()->glVertex(0,0);
        glTexCoord2f(-m_Scroll.x*0.015+0.65,m_Scroll.y*0.015);
        getParent()->glVertex(getParent()->getDispWidth(),0);
        glTexCoord2f(-m_Scroll.x*0.015+0.65,m_Scroll.y*0.015+0.65);
        getParent()->glVertex(getParent()->getDispWidth(),getParent()->getDispHeight());
        glTexCoord2f(-m_Scroll.x*0.015,m_Scroll.y*0.015+0.65);
        getParent()->glVertex(0,getParent()->getDispHeight());
        glEnd();
        glDisable(GL_TEXTURE_2D); 

	pType = (EffectSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_EFFECT, "Sky2Drift");
	if(pType != NULL && m_Quality == GQ_HIGH) {
          glBindTexture(GL_TEXTURE_2D,pType->getTexture()->nID);
          glEnable(GL_TEXTURE_2D);
          glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
          glEnable(GL_BLEND);

          glBegin(GL_POLYGON);   
          glColor4f(0.5,0.5,0.5,0.5f);   
          float fDrift = getParent()->getRealTime() / 25.0f;
          glTexCoord2f(-m_Scroll.x*0.015 + fDrift,m_Scroll.y*0.015);
          getParent()->glVertex(0,0);
          glTexCoord2f(-m_Scroll.x*0.015+0.65 + fDrift,m_Scroll.y*0.015);
          getParent()->glVertex(getParent()->getDispWidth(),0);
          glTexCoord2f(-m_Scroll.x*0.015+0.65 + fDrift,m_Scroll.y*0.015+0.65);
          getParent()->glVertex(getParent()->getDispWidth(),getParent()->getDispHeight());
          glTexCoord2f(-m_Scroll.x*0.015 + fDrift,m_Scroll.y*0.015+0.65);
          getParent()->glVertex(0,getParent()->getDispHeight());
          glEnd();

          glBlendFunc(GL_ONE,GL_ONE);

          glBegin(GL_POLYGON);   
          glColor3f(1,0.5,0.5);   
          fDrift = getParent()->getRealTime() / 15.0f;
          glTexCoord2f(-m_Scroll.x*0.015 + fDrift,m_Scroll.y*0.015);
          getParent()->glVertex(0,0);
          glTexCoord2f(-m_Scroll.x*0.015+0.85 + fDrift,m_Scroll.y*0.015);
          getParent()->glVertex(getParent()->getDispWidth(),0);
          glTexCoord2f(-m_Scroll.x*0.015+0.85 + fDrift,m_Scroll.y*0.015+0.85);
          getParent()->glVertex(getParent()->getDispWidth(),getParent()->getDispHeight());
          glTexCoord2f(-m_Scroll.x*0.015 + fDrift,m_Scroll.y*0.015+0.85);
          getParent()->glVertex(0,getParent()->getDispHeight());
          glEnd();

          glDisable(GL_BLEND);
          glDisable(GL_TEXTURE_2D); 
        }
      }   
    }
  }  
  /*===========================================================================
  And background rendering
  ===========================================================================*/
  void GameRenderer::_RenderBackground(void) { 
    MotoGame *pGame = getGameObject();
    
    /* Render background blocks */
    std::vector<ConvexBlock *> &Blocks = pGame->getBlocks();

    for(int i=0;i<Blocks.size();i++) {
      Vector2f Center;
      Texture *pTexture;
      GLuint GLName = 0;
      
      if(!Blocks[i]->pSrcBlock || Blocks[i]->pSrcBlock->bDynamic || !Blocks[i]->pSrcBlock->bBackground) continue;

      /* Main body */      
      Center = Vector2f(Blocks[i]->pSrcBlock->fPosX,Blocks[i]->pSrcBlock->fPosY);
      Sprite *pSprite;
      pSprite = getParent()->m_theme.getSprite(SPRITE_TYPE_TEXTURE, Blocks[i]->pSrcBlock->Texture);

      if(pSprite != NULL) {
	try {
	  pTexture = pSprite->getTexture();
	} catch(Exception &e) {
	  pTexture = NULL;
	}
      } else {
	pTexture = NULL;
      }
      if(pTexture != NULL) {GLName = pTexture->nID;}
    
      glBindTexture(GL_TEXTURE_2D,GLName);
      glEnable(GL_TEXTURE_2D);
      glBegin(GL_POLYGON);
      glColor3f(1,1,1);
      for(int j=0;j<Blocks[i]->Vertices.size();j++) {
        glTexCoord2f((Center.x+Blocks[i]->Vertices[j]->P.x) * 0.25,
                      (Center.y+Blocks[i]->Vertices[j]->P.y) * 0.25);
        _Vertex( Center + Blocks[i]->Vertices[j]->P );                  
      }
      glEnd();
      glDisable(GL_TEXTURE_2D);
    }        
  }

  /*===========================================================================
  Helpers
  ===========================================================================*/
  void GameRenderer::_Vertex(Vector2f P) {
    glVertex2f(P.x,P.y);
  }
  
  void GameRenderer::_DbgText(Vector2f P,std::string Text,Color c) {
    Vector2f Sp = Vector2f(getParent()->getDispWidth()/2 + (float)(P.x + m_Scroll.x)*m_fZoom,
                           getParent()->getDispHeight()/2 - (float)(P.y + m_Scroll.y)*m_fZoom) -
                  Vector2f(getParent()->getTextWidth(Text)/2.0f,getParent()->getTextHeight(Text)/2.0f);
    getParent()->drawText(Sp,Text,0,c,true);
  }   

  /*===========================================================================
  Free stuff
  ===========================================================================*/
  void GameRenderer::_Free(void) {      
    /* Free any particles left */
    for(int i=0;i<m_Particles.size();i++)
      delete m_Particles[i];
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
        glBegin(GL_LINE_LOOP);
        glColor3f(1,1,1);
        for(int j=0;j<m_DebugInfo[i]->Args.size()/2;j++) {
          float x = atof(m_DebugInfo[i]->Args[j*2].c_str());
          float y = atof(m_DebugInfo[i]->Args[j*2+1].c_str());
          getParent()->glVertex(400 + x*10,300 - y*10);
        }
        glEnd();
      }
      else if(m_DebugInfo[i]->Type == "@REDLINES") {
        glBegin(GL_LINE_STRIP);
        glColor3f(1,0,0);
        for(int j=0;j<m_DebugInfo[i]->Args.size()/2;j++) {
          float x = atof(m_DebugInfo[i]->Args[j*2].c_str());
          float y = atof(m_DebugInfo[i]->Args[j*2+1].c_str());
          getParent()->glVertex(400 + x*10,300 - y*10);
        }
        glEnd();
      }
      else if(m_DebugInfo[i]->Type == "@GREENLINES") {
        glBegin(GL_LINE_STRIP);
        glColor3f(0,1,0);
        for(int j=0;j<m_DebugInfo[i]->Args.size()/2;j++) {
          float x = atof(m_DebugInfo[i]->Args[j*2].c_str());
          float y = atof(m_DebugInfo[i]->Args[j*2+1].c_str());
          getParent()->glVertex(400 + x*10,300 - y*10);
        }
        glEnd();
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
    
    if(x > 0.0f && x < 1.0f && y > 0.0f && y < 1.0f) {    
      /* Map to viewport */
      float vx = ((float)getParent()->getDispWidth() * x);
      float vy = ((float)getParent()->getDispHeight() * y);

      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      glOrtho(0,getParent()->getDispWidth(),0,getParent()->getDispHeight(),-1,1);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      
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

      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      
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
      m_pNewHighscoreSave_str->setCaption("(Saved as " + p_save + ")");
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
      m_pNewHighscoreSave_str->setCaption("(Saved as " + p_save + ")");
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
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
    glBindTexture(GL_TEXTURE_2D,pTexture->nID);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_POLYGON);
    glColor3f(1,1,1);
    glTexCoord2f(0,1);
    glVertex2f(p0.x,p0.y);
    glTexCoord2f(1,1);
    glVertex2f(p1.x,p1.y);
    glTexCoord2f(1,0);
    glVertex2f(p2.x,p2.y);
    glTexCoord2f(0,0);
    glVertex2f(p3.x,p3.y);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
  }
  
  void GameRenderer::_RenderAdditiveBlendedSection(Texture *pTexture,
                                                   const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3) {
    glEnable(GL_BLEND); 
    glBlendFunc(GL_ONE,GL_ONE);         
    glBindTexture(GL_TEXTURE_2D,pTexture->nID);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_POLYGON);
    glColor3f(1,1,1);
    glTexCoord2f(0,1);
    glVertex2f(p0.x,p0.y);
    glTexCoord2f(1,1);
    glVertex2f(p1.x,p1.y);
    glTexCoord2f(1,0);
    glVertex2f(p2.x,p2.y);
    glTexCoord2f(0,0);
    glVertex2f(p3.x,p3.y);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
  }
  
  /* Screen-space version of the above */
  void GameRenderer::_RenderAlphaBlendedSectionSP(Texture *pTexture,
                                                  const Vector2f &p0,const Vector2f &p1,const Vector2f &p2,const Vector2f &p3) {
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
    glBindTexture(GL_TEXTURE_2D,pTexture->nID);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_POLYGON);
    glColor3f(1,1,1);
    glTexCoord2f(0,1);
    getParent()->glVertex(p0.x,p0.y);
    glTexCoord2f(1,1);
    getParent()->glVertex(p1.x,p1.y);
    glTexCoord2f(1,0);
    getParent()->glVertex(p2.x,p2.y);
    glTexCoord2f(0,0);
    getParent()->glVertex(p3.x,p3.y);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
  }
  
  void GameRenderer::_RenderCircle(int nSteps,Color CircleColor,const Vector2f &C,float fRadius) {
    glBegin(GL_LINE_LOOP);              
    glColor3ub(GET_RED(CircleColor),GET_GREEN(CircleColor),GET_BLUE(CircleColor));
    for(int i=0;i<nSteps;i++) {
      float r = (3.14159f * 2.0f * (float)i)/ (float)nSteps;            
      _Vertex( Vector2f(C.x + fRadius*sin(r),C.y + fRadius*cos(r)) );
    }      
    glEnd();
  }

  void GameRenderer::setTheme(Theme *p_theme) {
    m_theme = p_theme;
  }

};
