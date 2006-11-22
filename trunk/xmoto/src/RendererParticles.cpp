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
    
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
    glBindTexture(GL_TEXTURE_2D,pTexture->nID);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_POLYGON);
    glColor4ub(c.Red(), c.Green(), c.Blue(), c.Alpha());
    glTexCoord2f(0,0);
    _Vertex(p1);
    glTexCoord2f(1,0);
    _Vertex(p2);
    glTexCoord2f(1,1);
    _Vertex(p3);
    glTexCoord2f(0,1);
    _Vertex(p4);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);            
  }

  void GameRenderer::_RenderParticle(ParticlesSource *i_source) {
    AnimationSprite *pStarAnim = (AnimationSprite *)getParent()->m_theme.getSprite(SPRITE_TYPE_ANIMATION,"Star");
    EffectSprite* pFireType = (EffectSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_EFFECT, "Fire1");
    EffectSprite* pSmoke1Type = (EffectSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_EFFECT, "Smoke1");
    EffectSprite* pSmoke2Type = (EffectSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_EFFECT, "Smoke2");
    
    EffectSprite* pDebrisType = (EffectSprite*) getParent()->m_theme.getSprite(SPRITE_TYPE_EFFECT, "Debris1");

    if(i_source->SpriteName() == "Star") {
      if(pStarAnim != NULL) {
	
	for(unsigned j = 0; j < i_source->Particles().size(); j++) {
	  _RenderParticleDraw(i_source->Particles()[j]->Position(),
			      pStarAnim->getTexture(),
			      pStarAnim->getWidth(),
			      i_source->Particles()[j]->Angle(),
			      i_source->Particles()[j]->Color());
	}
      }
    } else if(i_source->SpriteName() == "Fire") {
      if(pFireType != NULL) {
	for(unsigned j = 0; j < i_source->Particles().size(); j++) {
	  _RenderParticleDraw(i_source->Particles()[j]->Position(),
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
	    _RenderParticleDraw(i_source->Particles()[j]->Position(),
				pSmoke1Type->getTexture(),
				i_source->Particles()[j]->Size(),
				i_source->Particles()[j]->Angle(),
				i_source->Particles()[j]->Color());
	  } else if(i_source->Particles()[j]->SpriteName() == "Smoke2") {
	    _RenderParticleDraw(i_source->Particles()[j]->Position(),
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
	    _RenderParticleDraw(i_source->Particles()[j]->Position(),
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
      if(v_entity->Type() == ET_PARTICLESOURCE) {
	_RenderParticle((ParticlesSource*) v_entity);
      }
    }

    for(unsigned int i = 0; i < getGameObject()->getLevelSrc()->EntitiesExterns().size(); i++) {
      Entity* v_entity = getGameObject()->getLevelSrc()->EntitiesExterns()[i];
      if(v_entity->Type() == ET_PARTICLESOURCE) {
	_RenderParticle((ParticlesSource*) v_entity);
      }
    }
  }

}

