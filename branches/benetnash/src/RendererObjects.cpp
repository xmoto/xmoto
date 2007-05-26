/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

Game Objects rendering code 2007 Janek Polak (benetnash@mail.icpnet.pl)
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

#include "Renderer.h"
#include "xmscene/Bike.h"
#include "xmscene/BikeGhost.h"
#include "xmscene/BikePlayer.h"

namespace vapp {

void GameRenderer::RenderLevelBlock(Block* block) 
{
	if (block->isBackground() && (m_Quality == GQ_LOW || m_bUglyMode))
		return;
	
	if (!m_bUglyMode) {
		_RenderBlock(block);
		/* Render all special edges (if quality!=low) */
		if(m_Quality != GQ_LOW) {
			_RenderBlockEdges(block);
		}
	}
	if(m_bUglyMode || m_bUglyOverMode) {
		_RenderUglyBlock(block);
	}
}

/* currently not used */
void GameRenderer::RenderLevelLayerBlock(Block* block) 
{
#ifdef ENABLE_OPENGL
	if (m_bUglyMode || m_Quality != GQ_HIGH)
		return;
	
	if(getParent()->getDrawLib()->getBackend() != DrawLib::backend_OpenGl) {
		return;
	}
	
	int layer = block->getLayer();

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
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(translateVector.x, translateVector.y, 0);

		RenderLevelBlock(block);
	
	glPopMatrix();
#endif
}

void GameRenderer::RenderLevelDynamicBlock(Block* block) 
{
	if (block->isBackground() && (m_Quality == GQ_LOW || m_bUglyMode))
		return;
	
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

		/* we're working with modelview matrix*/
		glPushMatrix();

		glTranslatef(dynPos.x, dynPos.y, 0);
		if(rotation != 0.0){
			glTranslatef(dynRotCenter.x, dynRotCenter.y, 0);
			glRotatef(rad2deg(rotation), 0, 0, 1);
			glTranslatef(-dynRotCenter.x, -dynRotCenter.y, 0);
		}

		if (!m_bUglyMode) {
			_RenderBlock(block);
			/* Render all special edges (if quality!=low) */
			if(m_Quality != GQ_LOW) {
				/* unfortunately because _RenderBlockEdges included dynamic 
				position inside we have to delete  translation matrix*/
				glPushMatrix();
				glTranslatef(-dynPos.x, -dynPos.y, 0);
				_RenderBlockEdges(block);
				glPopMatrix();
			}
		}
		
		if(m_bUglyMode || m_bUglyOverMode) {
			if(block->isBackground() == false && block->isLayer() == false) {
				getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_LOOP);
				getParent()->getDrawLib()->setColorRGB(255,255,255);
				for(int j=0;j<block->Vertices().size();j++) {
					getParent()->getDrawLib()->glVertex(block->Vertices()[j]->Position().x,
							  block->Vertices()[j]->Position().y);
				}
				getParent()->getDrawLib()->endDraw();
			}
		}
				
		glPopMatrix();
#endif
		
	} else if(getParent()->getDrawLib()->getBackend() == DrawLib::backend_SdlGFX){
		if (m_bUglyMode == false) {
			for(int j=0;j<m_Geoms[geom]->Polys.size();j++) {          
				getParent()->getDrawLib()->setTexture(m_Geoms[geom]->pTexture,BLEND_MODE_NONE);
				getParent()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
				getParent()->getDrawLib()->setColorRGB(255,255,255);

				for(int k=0;k<m_Geoms[geom]->Polys[j]->nNumVertices;k++) {
					Vector2f vertex = Vector2f(m_Geoms[geom]->Polys[j]->pVertices[k].x,
											   m_Geoms[geom]->Polys[j]->pVertices[k].y);
					/* transform vertex */
					Vector2f transVertex = Vector2f((vertex.x-dynRotCenter.x)*fR[0] + (vertex.y-dynRotCenter.y)*fR[1],
							(vertex.x-dynRotCenter.x)*fR[2] + (vertex.y-dynRotCenter.y)*fR[3]);
					transVertex += dynPos + dynRotCenter;

					getParent()->getDrawLib()->glTexCoord(m_Geoms[geom]->Polys[j]->pTexCoords[k].x,
							  m_Geoms[geom]->Polys[j]->pTexCoords[k].y);
					getParent()->getDrawLib()->glVertex(transVertex.x, transVertex.y);
				}
				getParent()->getDrawLib()->endDraw();
			}
				
			/* Render all special edges (if quality!=low) */
			if(m_Quality != GQ_LOW) {
					_RenderBlockEdges(block);
			}
		}
					
		if((m_bUglyMode || m_bUglyOverMode) && block->isBackground() == false) { 	

			getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_LOOP);
			getParent()->getDrawLib()->setColorRGB(255,255,255);

			for(int j=0;j<block->Vertices().size();j++) {
				Vector2f vertex = block->Vertices()[j]->Position();
				/* transform vertex */
				Vector2f transVertex = Vector2f((vertex.x-dynRotCenter.x)*fR[0] + (vertex.y-dynRotCenter.y)*fR[1],
						(vertex.x-dynRotCenter.x)*fR[2] + (vertex.y-dynRotCenter.y)*fR[3]);
				transVertex += dynPos + dynRotCenter;

				getParent()->getDrawLib()->glVertex(transVertex.x, transVertex.y);
			}
			getParent()->getDrawLib()->endDraw();
		}	


	}
}

void GameRenderer::RenderLevelEntity(Entity* entity) 
{
	if (entity->isAlive() == false)
		return;
	switch(entity->Speciality()) {
		case ET_NONE:
			_RenderSprite(entity);  
			break;
		case ET_PARTICLES_SOURCE:
			if (m_Quality == GQ_HIGH && m_bUglyMode == false)
				_RenderParticle((ParticlesSource*) entity);
			break;
		default:
			switch(entity->Speciality()) {
					case ET_MAKEWIN:
						_RenderSprite(entity, m_sizeMultOfEntitiesWhichMakeWin);
						break;
					case ET_ISTOTAKE:
						_RenderSprite(entity, m_sizeMultOfEntitiesToTake);
						break;
					default:	      
						_RenderSprite(entity);
			}
			break;
	}
}


void GameRenderer::RenderLevelExternEntities() 
{
	for(unsigned int i = 0; i < getGameObject()->getLevelSrc()->EntitiesExterns().size(); i++) {
		Entity* v_entity = getGameObject()->getLevelSrc()->EntitiesExterns()[i];
		v_entity->render(this);
	}	
}


void GameRenderer::RenderLevelBikes()
{
	MotoGame* pGame   = getGameObject();
	Camera*   pCamera = pGame->getCamera();
	
	/* ghosts */
	bool v_found = false;
	int v_found_i = 0;
	for(unsigned int i=0; i<pGame->Ghosts().size(); i++) {
		Ghost* v_ghost = pGame->Ghosts()[i];
		if(v_ghost != pCamera->getPlayerToFollow()) {
			_RenderGhost(v_ghost, i);
		} else {
			v_found = true;
			v_found_i = i;
		}
	}
	/* draw the player to follow over the others */
	if(v_found) {
		_RenderGhost(pGame->Ghosts()[v_found_i], v_found_i);
	}
		
	/* ... followed by the bike ... */
	v_found = false;
	for(unsigned int i=0; i<pGame->Players().size(); i++) {
		Biker* v_player = pGame->Players()[i];
		if(v_player != pCamera->getPlayerToFollow()) {
			_RenderBike(v_player->getState(),
						&(v_player->getState()->Parameters()),
						v_player->getBikeTheme(),
						v_player->getRenderBikeFront(),
						v_player->getColorFilter(),
						v_player->getUglyColorFilter());
		} else {
			v_found = true;
		}
	}
	
	if(v_found) {
		_RenderBike(pCamera->getPlayerToFollow()->getState(),
					&(pCamera->getPlayerToFollow()->getState()->Parameters()),
					pCamera->getPlayerToFollow()->getBikeTheme(),
					pCamera->getPlayerToFollow()->getRenderBikeFront(),
					pCamera->getPlayerToFollow()->getColorFilter(),
					pCamera->getPlayerToFollow()->getUglyColorFilter());
	}
	
	/* ghost information */
	if(pGame->getTime() > m_fNextGhostInfoUpdate) {
		if(m_nGhostInfoTrans > 0) {
			if(m_fNextGhostInfoUpdate > 1.5f) {
				m_nGhostInfoTrans-=16;
			}
			m_fNextGhostInfoUpdate += 0.025f;
		}
	}
}

}
