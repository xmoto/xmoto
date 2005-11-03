/*=============================================================================
XMOTO
Copyright (C) 2005 Rasmus Neckelmann (neckelmann@gmail.com)

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
        
    /* Optimize scene */
    std::vector<ConvexBlock *> &Blocks = getGameObject()->getBlocks();
    int nVertexBytes = 0;
  
    for(int i=0;i<Blocks.size();i++) {
      Vector2f Center;
      Texture *pTexture;
      GLuint GLName = 0;
      
      if(Blocks[i]->pSrcBlock && Blocks[i]->pSrcBlock->bBackground) continue;

      if(Blocks[i]->pSrcBlock) {
        Center = Vector2f(Blocks[i]->pSrcBlock->fPosX,Blocks[i]->pSrcBlock->fPosY);
        pTexture = getParent()->TexMan.getTexture(Blocks[i]->pSrcBlock->Texture);
        if(pTexture != NULL) {GLName = pTexture->nID;}
      }
      else {
        pTexture = getParent()->TexMan.getTexture("default");
        if(pTexture != NULL) {GLName = pTexture->nID;}
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
        pPoly->pTexCoords[j].x = (Center.x+Blocks[i]->Vertices[j]->P.x) * 0.25;
        pPoly->pTexCoords[j].y = (Center.y+Blocks[i]->Vertices[j]->P.y) * 0.25;
      }          
      
      nVertexBytes += pPoly->nNumVertices * ( 4 * sizeof(float) );
      
      /* Use VBO optimization? */
      if(getParent()->useVBOs()) {
        /* Copy static coordinates unto video memory */
        getParent()->glGenBuffersARB(1,&pPoly->nVertexBufferID);
        getParent()->glBindBufferARB(GL_ARRAY_BUFFER_ARB,pPoly->nVertexBufferID);
        getParent()->glBufferDataARB(GL_ARRAY_BUFFER_ARB,pPoly->nNumVertices*2*sizeof(float),(void *)pPoly->pVertices,GL_STATIC_DRAW_ARB);
        
        getParent()->glGenBuffersARB(1,&pPoly->nTexCoordBufferID);
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
          getParent()->glDeleteBuffersARB(1,&m_Geoms[i]->Polys[j]->nVertexBufferID);
          getParent()->glDeleteBuffersARB(1,&m_Geoms[i]->Polys[j]->nTexCoordBufferID);
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
        
    MotoGame *pGame = getGameObject();
    std::vector<ConvexBlock *> &Blocks = pGame->getBlocks();

    for(int i=0;i<Blocks.size();i++) {
      Vector2f Center;

      if(Blocks[i]->pSrcBlock) {
        if(Blocks[i]->pSrcBlock->bBackground) continue;
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
    
    getParent()->drawCircle(Vector2f(x + nWidth/2 + (float)(pGame->getBikeState()->CenterP.x + m_Scroll.x)*MINIMAPZOOM,
                                     y + nHeight/2 - (float)(pGame->getBikeState()->CenterP.y + m_Scroll.y)*MINIMAPZOOM),
                            3,0,MAKE_COLOR(255,255,255,255),0);
                            
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

  /*===========================================================================
  Main rendering function
  ===========================================================================*/
  void GameRenderer::render(void) {  
    /* Update time */    
    m_pInGameStats->showWindow(true);
    m_pPlayTime->setCaption(getParent()->formatTime(getGameObject()->getTime()));
  
    /* Prepare for rendering frame */
    _UpdateAnimations();
    
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

    /* SKY! */
    if(!m_bUglyMode)
			_RenderSky();

    /* Perform scaling/translation */
    float fScale = 0.195f;
    glScalef(fScale * ((float)getParent()->getDispHeight()) / getParent()->getDispWidth(),fScale,1);
    glTranslatef(m_Scroll.x,m_Scroll.y,0);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
      
    if(m_Quality != GQ_LOW && !m_bUglyMode) {
      /* Background blocks */
      _RenderBackground();
      
      /* ... then render background sprites ... */
      _RenderSprites(false,true);
    }

    if(m_Quality == GQ_HIGH && !m_bUglyMode) {
      /* Render particles (back!) */    
      _RenderParticles(false);
    }
        
    /* ... covered by blocks ... */
    _RenderBlocks();
    
    /* ... the entities ... */
    _RenderEntities();

    /* ... followed by the bike ... */
    _RenderBike();
    
    if(m_Quality == GQ_HIGH && !m_bUglyMode) {
      /* Render particles (front!) */    
      _RenderParticles();
    }
    
    if(m_Quality != GQ_LOW && !m_bUglyMode) {
      /* ... and finally the foreground sprites! */
      _RenderSprites(true,false);
    }
        
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
        C=pArrow->ArrowPointerPos;
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
      
      glEnable(GL_BLEND); 
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
      glBindTexture(GL_TEXTURE_2D,m_pArrowTexture->nID);
      glEnable(GL_TEXTURE_2D);
      glBegin(GL_POLYGON);
      glColor3f(1,1,1);
      glTexCoord2f(0,0);
      getParent()->glVertex((C+p1).x,(C+p1).y);
      glTexCoord2f(1,0);
      getParent()->glVertex((C+p2).x,(C+p2).y);
      glTexCoord2f(1,1);
      getParent()->glVertex((C+p3).x,(C+p3).y);
      glTexCoord2f(0,1);
      getParent()->glVertex((C+p4).x,(C+p4).y);
      glEnd();
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_BLEND);        
    }
        
    /* Messages */
    if(pGame != NULL) {
      for(int i=0;i<pGame->getGameMessage().size();i++) {
        GameMessage *pMsg = pGame->getGameMessage()[i];
        int x1,y1,x2,y2;
        UITextDraw::getTextExt(getMediumFont(),pMsg->Text,&x1,&y1,&x2,&y2);
        UITextDraw::printRaw(getMediumFont(),400 - (x2-x1)/2,pMsg->Pos[1]*600,pMsg->Text,MAKE_COLOR(255,255,255,pMsg->nAlpha));
      } 
    }
  }
  
  /*===========================================================================
  Sprite rendering main
  ===========================================================================*/
  void GameRenderer::_RenderSprites(bool bForeground,bool bBackground) { 
    MotoGame *pGame = getGameObject();
  
    /* In front? */
    if(bForeground) {
      for(int i=0;i<pGame->getFSprites().size();i++)
        _RenderSprite(pGame->getFSprites()[i]);
    }

    /* Those in back? */
    if(bBackground) {
      for(int i=0;i<pGame->getBSprites().size();i++)
        _RenderSprite(pGame->getBSprites()[i]);
    }
  }

  /*===========================================================================
  Render a sprite
  ===========================================================================*/
  void GameRenderer::_RenderSprite(Entity *pSprite) {    
    /* TODO: make type fetching faster */
    SpriteType *pType = _GetSpriteTypeByName(pSprite->SpriteType);
    if(pType != NULL) {
      /* Get texture */
      GLuint GLName = pType->pTexture->nID;      
    
      /* Draw it */
      Vector2f p0,p1,p2,p3;
      
      p0 = Vector2f(pSprite->Pos.x,pSprite->Pos.y) +
           Vector2f(-pType->Center.x,-pType->Center.y);
      p1 = Vector2f(pSprite->Pos.x+pType->Size.x,pSprite->Pos.y) +
           Vector2f(-pType->Center.x,-pType->Center.y);
      p2 = Vector2f(pSprite->Pos.x+pType->Size.x,pSprite->Pos.y+pType->Size.y) +
           Vector2f(-pType->Center.x,-pType->Center.y);
      p3 = Vector2f(pSprite->Pos.x,pSprite->Pos.y+pType->Size.y) +
           Vector2f(-pType->Center.x,-pType->Center.y);
            
      glEnable(GL_BLEND); 
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
      glBindTexture(GL_TEXTURE_2D,GLName);
      glEnable(GL_TEXTURE_2D);
      glBegin(GL_POLYGON);   
      glColor3f(1,1,1);   
      glTexCoord2f(0,1);
      _Vertex(p0);
      glTexCoord2f(1,1);
      _Vertex(p1);
      glTexCoord2f(1,0);
      _Vertex(p2);
      glTexCoord2f(0,0);
      _Vertex(p3);
      glEnd();
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_BLEND);

      /* Debug mode? */
      /* TODO: port to new transform */
      //if(isDebug()) {
      //  /* Sprite marker */
      //  glBegin(GL_LINE_STRIP);      
      //  glColor3f(1,0.5,0.2);
      //  _Vertex(pSprite->Pos - Vector2f(0.5,0));
      //  _Vertex(pSprite->Pos + Vector2f(0.5,0));
      //  glEnd();
      //  glBegin(GL_LINE_STRIP);      
      //  glColor3f(1,0.5,0.2);
      //  _Vertex(pSprite->Pos - Vector2f(0,0.5));
      //  _Vertex(pSprite->Pos + Vector2f(0,0.5));
      //  glEnd();
      //  
      //  _DbgText(pSprite->Pos,pSprite->ID,MAKE_COLOR(255,128,51,255));
      //}
    }
  }
       
  /*===========================================================================
  Blocks
  ===========================================================================*/
  void GameRenderer::_RenderBlocks(void) {
    MotoGame *pGame = getGameObject();
    float cols[1000];
    int dd=0;
    for(dd=0;dd<1000;dd++) cols[dd]=1.0f;

		/* Ugly mode? */
		if(m_bUglyMode) {
			/* Render all non-background blocks */
	    std::vector<LevelBlock *> &Blocks = getGameObject()->getLevelSrc()->getBlockList();
	
			for(int i=0;i<Blocks.size();i++) {
				if(!Blocks[i]->bBackground) {
					glBegin(GL_LINE_LOOP);
					glColor3f(1,1,1);
					for(int j=0;j<Blocks[i]->Vertices.size();j++) {
						glVertex2f(Blocks[i]->Vertices[j]->fX + Blocks[i]->fPosX,
						           Blocks[i]->Vertices[j]->fY + Blocks[i]->fPosY);
					}
					glEnd();
				}
			}
		}
		else {
			/* Render all non-background blocks */
			/* Static geoms... */
			for(int i=0;i<m_Geoms.size();i++) {
				glBindTexture(GL_TEXTURE_2D,m_Geoms[i]->pTexture->nID);
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
							if(m_pEdgeGrass1 != NULL) {
								glEnable(GL_BLEND); 
								glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
								glBindTexture(GL_TEXTURE_2D,m_pEdgeGrass1->nID);
								glEnable(GL_TEXTURE_2D);
								glBegin(GL_POLYGON);
								glColor3f(1,1,1);
								glTexCoord2f((pEdge->pSrcBlock->fPosX+pEdge->P1.x)*0.5,0.01);
								_Vertex(pEdge->P1 + Vector2f(pEdge->pSrcBlock->fPosX,pEdge->pSrcBlock->fPosY));
								glTexCoord2f((pEdge->pSrcBlock->fPosX+pEdge->P2.x)*0.5,0.01);
								_Vertex(pEdge->P2 + Vector2f(pEdge->pSrcBlock->fPosX,pEdge->pSrcBlock->fPosY));
								glTexCoord2f((pEdge->pSrcBlock->fPosX+pEdge->P2.x)*0.5,0.99);
								_Vertex(pEdge->P2 + Vector2f(pEdge->pSrcBlock->fPosX,pEdge->pSrcBlock->fPosY) + Vector2f(0,-0.3));
								glTexCoord2f((pEdge->pSrcBlock->fPosX+pEdge->P1.x)*0.5,0.99);
								_Vertex(pEdge->P1 + Vector2f(pEdge->pSrcBlock->fPosX,pEdge->pSrcBlock->fPosY) + Vector2f(0,-0.3));
								glEnd();
								glDisable(GL_TEXTURE_2D);
								glDisable(GL_BLEND);
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

    /* Render sky - but which? */
    const std::string &SkyName = pGame->getLevelSrc()->getLevelInfo()->Sky;
//    printf("[%s]\n",SkyName.c_str());
    if(SkyName == "" || SkyName == "sky1") {
      if(m_pSkyTexture1 != NULL) {
        glBindTexture(GL_TEXTURE_2D,m_pSkyTexture1->nID);
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
      if(m_pSkyTexture2 != NULL) {
        glBindTexture(GL_TEXTURE_2D,m_pSkyTexture2->nID);
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

        if(m_pSkyTexture2Drift != NULL && m_Quality == GQ_HIGH) {
          glBindTexture(GL_TEXTURE_2D,m_pSkyTexture2Drift->nID);
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
      
      if(!Blocks[i]->pSrcBlock || !Blocks[i]->pSrcBlock->bBackground) continue;

      /* Main body */      
      Center = Vector2f(Blocks[i]->pSrcBlock->fPosX,Blocks[i]->pSrcBlock->fPosY);
      pTexture = getParent()->TexMan.getTexture(Blocks[i]->pSrcBlock->Texture);
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
  And general entity rendering
  ===========================================================================*/
  void GameRenderer::_RenderEntities(void) {
    MotoGame *pGame = getGameObject();

    /* Render all entities */
    std::vector<Entity *> &Entities = pGame->getEntities();    
        
    for(int i=0;i<Entities.size();i++) {
      Entity *pEntity = Entities[i];
      
      if(pEntity->Type == ET_STRAWBERRY) {
        /* Draw strawberry */
        _DrawAnimation(pEntity->Pos,m_pStrawberryAnim);
      }
      else if(pEntity->Type == ET_ENDOFLEVEL) {
        /* Draw end-of-level flower */
        _DrawAnimation(pEntity->Pos,m_pFlowerAnim);
      }
      else if(pEntity->Type == ET_WRECKER) {
        /* Draw nasty wrecker */
        _DrawAnimation(pEntity->Pos,m_pWreckerAnim);
      }
      
      /* If this is debug-mode, also draw entity's area of effect */
      if(isDebug()) {
        Vector2f C = Vector2f( getParent()->getDispWidth()/2 + (float)(pEntity->Pos.x + m_Scroll.x)*m_fZoom,
                               getParent()->getDispHeight()/2 - (float)(pEntity->Pos.y + m_Scroll.y)*m_fZoom );
        Vector2f Cr = Vector2f( getParent()->getDispWidth()/2 + (float)((pEntity->Pos.x + pEntity->fSize) + m_Scroll.x)*m_fZoom,
                                getParent()->getDispHeight()/2 - (float)(pEntity->Pos.y + m_Scroll.y)*m_fZoom );
        float r = (C-Cr).length();
        getParent()->drawCircle(C,r,1,0,MAKE_COLOR(0,255,0,255));
      }
    }
  }

  /*===========================================================================
  Helpers
  ===========================================================================*/
  void GameRenderer::_Vertex(Vector2f P) {
    //getParent()->glVertex( getParent()->getDispWidth()/2 + (float)(P.x + m_Scroll.x)*m_fZoom,
    //                       getParent()->getDispHeight()/2 - (float)(P.y + m_Scroll.y)*m_fZoom );
    glVertex2f(P.x,P.y);
  }
  
  SpriteType *GameRenderer::_GetSpriteTypeByName(std::string Name) {
    for(int i=0;i<m_nNumSpriteTypes;i++)
      if(m_SpriteTypes[i].Name == Name) return &m_SpriteTypes[i];
    return NULL;
  }
  
  void GameRenderer::_DbgText(Vector2f P,std::string Text,Color c) {
    Vector2f Sp = Vector2f(getParent()->getDispWidth()/2 + (float)(P.x + m_Scroll.x)*m_fZoom,
                           getParent()->getDispHeight()/2 - (float)(P.y + m_Scroll.y)*m_fZoom) -
                  Vector2f(getParent()->getTextWidth(Text)/2.0f,getParent()->getTextHeight(Text)/2.0f);
    getParent()->drawText(Sp,Text,0,c,true);
  }   
  
  /*===========================================================================
  Animations
  ===========================================================================*/
  Animation *GameRenderer::_GetAnimationByName(std::string Name) {
    for(int i=0;i<m_Anims.size();i++)
      if(m_Anims[i]->Name == Name) return m_Anims[i];
    return NULL;
  }
  
  void GameRenderer::_UpdateAnimations(void) {
    for(int i=0;i<m_Anims.size();i++) {
      Animation *pAnim = m_Anims[i];
    
      /* Next frame? */
      if(getParent()->getRealTime() > pAnim->fFrameTime+pAnim->m_Frames[pAnim->m_nCurFrame]->fDelay) {
        pAnim->fFrameTime=getParent()->getRealTime();
        pAnim->m_nCurFrame++;
        if(pAnim->m_nCurFrame == pAnim->m_Frames.size()) pAnim->m_nCurFrame = 0;      
      }
    }
  }    

  void GameRenderer::_DrawAnimation(Vector2f Pos,Animation *pAnim) {
    AnimationFrame *pFrame = pAnim->m_Frames[pAnim->m_nCurFrame];
    
    /* Get texture */
    GLuint GLName = pFrame->pTexture->nID;      
  
    /* Draw it */
    Vector2f p0,p1,p2,p3;
    
    p0 = Vector2f(Pos.x,Pos.y) +
          Vector2f(-pFrame->Center.x,-pFrame->Center.y);
    p1 = Vector2f(Pos.x+pFrame->Size.x,Pos.y) +
          Vector2f(-pFrame->Center.x,-pFrame->Center.y);
    p2 = Vector2f(Pos.x+pFrame->Size.x,Pos.y+pFrame->Size.y) +
          Vector2f(-pFrame->Center.x,-pFrame->Center.y);
    p3 = Vector2f(Pos.x,Pos.y+pFrame->Size.y) +
          Vector2f(-pFrame->Center.x,-pFrame->Center.y);
          
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
    glBindTexture(GL_TEXTURE_2D,GLName);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_POLYGON);   
    glColor3f(1,1,1);   
    glTexCoord2f(0,1);
    _Vertex(p0);
    glTexCoord2f(1,1);
    _Vertex(p1);
    glTexCoord2f(1,0);
    _Vertex(p2);
    glTexCoord2f(0,0);
    _Vertex(p3);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);    
  }

  /*===========================================================================
  Free stuff
  ===========================================================================*/
  void GameRenderer::_Free(void) {
    /* Free animations */
    for(int i=0;i<m_Anims.size();i++) { 
      for(int j=0;j<m_Anims[i]->m_Frames.size();j++) 
        delete m_Anims[i]->m_Frames[j];
      delete m_Anims[i];
    }
    
    /* Free any particles left */
    for(int i=0;i<m_Particles.size();i++)
      delete m_Particles[i];
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
  
};

