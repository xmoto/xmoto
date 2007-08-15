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
 *  In-game rendering - particles
 */
#include "VXml.h"
#include "VFileIO.h"
#include "xmscene/Scene.h"
#include "Renderer.h"
#include "PhysSettings.h"
#include "xmscene/BasicSceneStructs.h"
#include "VDraw.h"
#include "Game.h"

  void GameRenderer::_RenderParticleDraw(Vector2f P,Texture *pTexture,float fSize,float fAngle, TColor c) {
    /* Render single particle */
    if(pTexture == NULL) return;

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
    
    getParent()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
    //convert the TColor to a Color 
    getParent()->getDrawLib()->setColor(MAKE_COLOR(c.Red(), c.Green(), c.Blue(), c.Alpha()));
    getParent()->getDrawLib()->glTexCoord(0.01, 0.01);
    getParent()->getDrawLib()->glVertex(p1);
    getParent()->getDrawLib()->glTexCoord(0.99, 0.01);
    getParent()->getDrawLib()->glVertex(p2);
    getParent()->getDrawLib()->glTexCoord(0.99, 0.99);
    getParent()->getDrawLib()->glVertex(p3);
    getParent()->getDrawLib()->glTexCoord(0.01, 0.99);
    getParent()->getDrawLib()->glVertex(p4);
    getParent()->getDrawLib()->endDrawKeepProperties();
  }

  void GameRenderer::_RenderParticle(ParticlesSource *i_source) {

    if(i_source->SpriteName() == "Star") {

      AnimationSprite *pStarAnimation;
      pStarAnimation = (AnimationSprite*) getParent()->getTheme()
      ->getSprite(SPRITE_TYPE_ANIMATION, getGameObject()->getLevelSrc()->SpriteForStar());

      if(pStarAnimation != NULL) {

	getParent()->getDrawLib()->setTexture(pStarAnimation->getTexture(),BLEND_MODE_A);	
	for(unsigned j = 0; j < i_source->Particles().size(); j++) {
	  _RenderParticleDraw(i_source->Particles()[j]->DynamicPosition(),
			      pStarAnimation->getTexture(),
			      pStarAnimation->getWidth(),
			      i_source->Particles()[j]->Angle(),
			      i_source->Particles()[j]->Color());
	}
	getParent()->getDrawLib()->removePropertiesAfterEnd();
      } else {
	DecorationSprite *pStarDecoration;

	/* search as a simple decoration, not nice, crappy crappy */
	pStarDecoration = (DecorationSprite*) getParent()->getTheme()
	->getSprite(SPRITE_TYPE_DECORATION, getGameObject()->getLevelSrc()->SpriteForStar());
	if(pStarDecoration != NULL) {
	
	  getParent()->getDrawLib()->setTexture(pStarDecoration->getTexture(),BLEND_MODE_A);
	  for(unsigned j = 0; j < i_source->Particles().size(); j++) {
	    _RenderParticleDraw(i_source->Particles()[j]->DynamicPosition(),
				pStarDecoration->getTexture(),
				pStarDecoration->getWidth(),
				i_source->Particles()[j]->Angle(),
				i_source->Particles()[j]->Color());
	  }
	  getParent()->getDrawLib()->removePropertiesAfterEnd();
	}
      }
    } else if(i_source->SpriteName() == "Fire") {
      EffectSprite* pFireType = (EffectSprite*) getParent()->getTheme()
      ->getSprite(SPRITE_TYPE_EFFECT, "Fire1");

      if(pFireType != NULL) {
	getParent()->getDrawLib()->setTexture(pFireType->getTexture(),BLEND_MODE_A);
	for(unsigned j = 0; j < i_source->Particles().size(); j++) {
	  _RenderParticleDraw(i_source->Particles()[j]->DynamicPosition(),
			      pFireType->getTexture(),
			      i_source->Particles()[j]->Size(),
			      i_source->Particles()[j]->Angle(),
			      i_source->Particles()[j]->Color());
	}
	getParent()->getDrawLib()->removePropertiesAfterEnd();
      }
    } else if(i_source->SpriteName() == "Smoke") {
      EffectSprite* pSmoke1Type = (EffectSprite*) getParent()->getTheme()
      ->getSprite(SPRITE_TYPE_EFFECT, "Smoke1");
      EffectSprite* pSmoke2Type = (EffectSprite*) getParent()->getTheme()
      ->getSprite(SPRITE_TYPE_EFFECT, "Smoke2");

      if(pSmoke1Type != NULL && pSmoke2Type != NULL) {
	if(i_source->Particles().size() > 0) {
	  if(i_source->Particles()[0]->SpriteName() == "Smoke1") {
	    getParent()->getDrawLib()->setTexture(pSmoke1Type->getTexture(),BLEND_MODE_A);
	  } else if(i_source->Particles()[0]->SpriteName() == "Smoke2") {
	    getParent()->getDrawLib()->setTexture(pSmoke1Type->getTexture(),BLEND_MODE_A);
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
	getParent()->getDrawLib()->removePropertiesAfterEnd();
      }
    } else if(i_source->SpriteName() == "Debris1") {
      EffectSprite* pDebrisType = (EffectSprite*) getParent()->getTheme()
      ->getSprite(SPRITE_TYPE_EFFECT, "Debris1");

      if(pDebrisType != NULL) {
	getParent()->getDrawLib()->setTexture(pDebrisType->getTexture(),BLEND_MODE_A);
	for(unsigned j = 0; j < i_source->Particles().size(); j++) {
	  if(i_source->Particles()[j]->SpriteName() == "Debris1") {
	    _RenderParticleDraw(i_source->Particles()[j]->DynamicPosition(),
				pDebrisType->getTexture(),
				i_source->Particles()[j]->Size(),
				i_source->Particles()[j]->Angle(),
				i_source->Particles()[j]->Color());
	  }
	}
	getParent()->getDrawLib()->removePropertiesAfterEnd();
      }
    }
  }
  
  void GameRenderer::_RenderParticles(bool bFront) {
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

    std::vector<Entity*> Entities = getGameObject()->getCollisionHandler()->getEntitiesNearPosition(screenBigger);
    for(unsigned int i = 0; i < Entities.size(); i++) {
      Entity* v_entity = Entities[i];
      if(v_entity->Speciality() == ET_PARTICLES_SOURCE) {
	_RenderParticle((ParticlesSource*) v_entity);
      }
    }

    for(unsigned int i = 0; i < getGameObject()->getLevelSrc()->EntitiesExterns().size(); i++) {
      Entity* v_entity = getGameObject()->getLevelSrc()->EntitiesExterns()[i];
      if(v_entity->Speciality() == ET_PARTICLES_SOURCE) {
	_RenderParticle((ParticlesSource*) v_entity);
      }
    }
  }

