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
 *  In-game rendering - particles
 */
#include "VXml.h"
#include "VFileIO.h"
#include "MotoGame.h"
#include "Renderer.h"
#include "PhysSettings.h"
#include "xmscene/BasicSceneStructs.h"

namespace vapp {

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
    
    getParent()->getDrawLib()->setTexture(pTexture,BLEND_MODE_A);
    getParent()->getDrawLib()->startDraw(DRAW_MODE_POLYGON);
    //convert the TColor to a Color 
    getParent()->getDrawLib()->setColor(MAKE_COLOR(c.Red(), c.Green(), c.Blue(), c.Alpha()));
    getParent()->getDrawLib()->glTexCoord(0,0);
    getParent()->getDrawLib()->glVertex(p1);
    getParent()->getDrawLib()->glTexCoord(1,0);
    getParent()->getDrawLib()->glVertex(p2);
    getParent()->getDrawLib()->glTexCoord(1,1);
    getParent()->getDrawLib()->glVertex(p3);
    getParent()->getDrawLib()->glTexCoord(0,1);
    getParent()->getDrawLib()->glVertex(p4);
    getParent()->getDrawLib()->endDraw();
  }

  void GameRenderer::_RenderParticle(ParticlesSource *i_source) {
    AnimationSprite *pStarAnim = (AnimationSprite *)getParent()->getTheme()->getSprite(SPRITE_TYPE_ANIMATION,"Star");
    EffectSprite* pFireType = (EffectSprite*) getParent()->getTheme()->getSprite(SPRITE_TYPE_EFFECT, "Fire1");
    EffectSprite* pSmoke1Type = (EffectSprite*) getParent()->getTheme()->getSprite(SPRITE_TYPE_EFFECT, "Smoke1");
    EffectSprite* pSmoke2Type = (EffectSprite*) getParent()->getTheme()->getSprite(SPRITE_TYPE_EFFECT, "Smoke2");
    
    EffectSprite* pDebrisType = (EffectSprite*) getParent()->getTheme()->getSprite(SPRITE_TYPE_EFFECT, "Debris1");

    if(i_source->SpriteName() == "Star") {
      if(pStarAnim != NULL) {
	
	for(unsigned j = 0; j < i_source->Particles().size(); j++) {
	  _RenderParticleDraw(i_source->Particles()[j]->DynamicPosition(),
			      pStarAnim->getTexture(),
			      pStarAnim->getWidth(),
			      i_source->Particles()[j]->Angle(),
			      i_source->Particles()[j]->Color());
	}
      }
    } else if(i_source->SpriteName() == "Fire") {
      if(pFireType != NULL) {
	for(unsigned j = 0; j < i_source->Particles().size(); j++) {
	  _RenderParticleDraw(i_source->Particles()[j]->DynamicPosition(),
			      pFireType->getTexture(),
			      i_source->Particles()[j]->Size(),
			      i_source->Particles()[j]->Angle(),
			      i_source->Particles()[j]->Color());
	}
      }
    } else if(i_source->SpriteName() == "Smoke") {
      if(pSmoke1Type != NULL && pSmoke2Type != NULL) {
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
      }
    } else if(i_source->SpriteName() == "Debris1") {
      if(pDebrisType != NULL) {
	for(unsigned j = 0; j < i_source->Particles().size(); j++) {
	  if(i_source->Particles()[j]->SpriteName() == "Debris1") {
	    _RenderParticleDraw(i_source->Particles()[j]->DynamicPosition(),
				pDebrisType->getTexture(),
				i_source->Particles()[j]->Size(),
				i_source->Particles()[j]->Angle(),
				i_source->Particles()[j]->Color());
	  }
	}
      }
    }
  }
  
  void GameRenderer::_RenderParticles(bool bFront) {
    for(unsigned int i = 0; i < getGameObject()->getLevelSrc()->Entities().size(); i++) {
      Entity* v_entity = getGameObject()->getLevelSrc()->Entities()[i];
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

}

