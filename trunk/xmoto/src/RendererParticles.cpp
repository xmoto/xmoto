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
 *  In-game rendering - particles
 */
#include "VXml.h"
#include "VFileIO.h"
#include "MotoGame.h"
#include "Renderer.h"
#include "PhysSettings.h"

namespace vapp {

  /*===========================================================================
  Particles, rendering and management
  ===========================================================================*/
  Particle *GameRenderer::spawnParticle(ParticleType Type,Vector2f Pos,Vector2f Vel,float fLifeTime) {
    MotoGame *pGame = getGameObject();

    if(pGame != NULL) {
      Particle *p = new Particle;
      p->Type = Type;
      p->Pos = Pos;
      p->Vel = Vel;     
      p->fSpawnTime = pGame->getTime();
      p->fKillTime = pGame->getTime() + fLifeTime;
      p->Acc = Vector2f(0,0);
      p->fAng = 0;
      p->fAngAcc = 0;
      p->fAngVel = 0;
      m_Particles.push_back(p);
      
      int cc;
      switch(Type) {
        case PT_DEBRIS:
          cc = (int)randomNum(0,250);
          p->DebrisTint = MAKE_COLOR(cc,cc,cc,255);
          p->Vel *= randomNum(1.5,6);
          p->Vel.x += randomNum(-0.2,0.2);
          p->Vel.y += randomNum(-0.2,0.2);
          p->fAngVel = randomNum(-60,60);
          p->fDebrisSize = randomNum(0.02f,0.05f);
          break;
          
        case PT_SMOKE1:
        case PT_SMOKE2:
          cc = (int)randomNum(0,50);
          p->SmokeColor = MAKE_COLOR(cc,cc,cc,255);
          p->fSmokeSize = randomNum(0,0.2);
          p->fAngVel = randomNum(-60,60);
          p->fKillTime += 10000;
          break;
        
        case PT_FIRE:
          p->FireColor = MAKE_COLOR(255,255,200,255);
          p->fFireSize = 0.1f;
          p->fKillTime += 10000;
          p->fFireSeed = randomNum(0,100);
          break;
      }
      
      return p;
    }
    
    return NULL;
  }  

  void GameRenderer::_UpdateParticles(float fTimeStep) {
    MotoGame *pGame = getGameObject();
    
    if(m_Quality != GQ_HIGH) {
      /* only particles at high quality setting */
      return;
    }

    if(pGame != NULL) {
      /* Look for particle sources */
      {
        for(int i=0;i<pGame->getEntities().size();i++) {
          if(pGame->getEntities()[i]->Type == ET_PARTICLESOURCE && 
            pGame->getTime()>pGame->getEntities()[i]->fNextParticleTime) {
            
            if(pGame->getEntities()[i]->ParticleType == "Smoke") {
              /* Generate smoke */
              if(randomNum(0,1) < 0.5)
                spawnParticle(PT_SMOKE1,pGame->getEntities()[i]->Pos,Vector2f(randomNum(-0.6,0.6),randomNum(0.2,0.6)),0);
              else
                spawnParticle(PT_SMOKE2,pGame->getEntities()[i]->Pos,Vector2f(randomNum(-0.6,0.6),randomNum(0.2,0.6)),0);
              pGame->getEntities()[i]->fNextParticleTime = pGame->getTime() + randomNum(0.2,0.5f);
            }
            else if(pGame->getEntities()[i]->ParticleType == "Fire") {
              for(int k=0;k<10;k++) {
                /* Generate fire */
                spawnParticle(PT_FIRE,pGame->getEntities()[i]->Pos,Vector2f(randomNum(-1,1),randomNum(0.1,0.3)),0);
                pGame->getEntities()[i]->fNextParticleTime = pGame->getTime() + randomNum(0.05,0.1f);              
              }
            }
          }
        }
      }
    
      /* For each particle */
      int i=0;
      while(1) {
        if(i >= m_Particles.size()) break;
        bool bKillParticle = false;
        
        /* Do general handling of it */
        m_Particles[i]->Vel += m_Particles[i]->Acc * fTimeStep;
        m_Particles[i]->Pos += m_Particles[i]->Vel * fTimeStep;
        m_Particles[i]->fAngVel += m_Particles[i]->fAngAcc * fTimeStep;
        m_Particles[i]->fAng += m_Particles[i]->fAngVel * fTimeStep;
        
        if(pGame->getTime() > m_Particles[i]->fKillTime)
          bKillParticle = true;
        
        /* Do type dependant handling of it */
        float a,c,c2;
        
        switch(m_Particles[i]->Type) {
          case PT_DEBRIS:
            m_Particles[i]->Acc = pGame->getGravity() * (-1.5f / PHYS_WORLD_GRAV);
            c=GET_RED(m_Particles[i]->DebrisTint);
            a=GET_ALPHA(m_Particles[i]->DebrisTint);
            a -= 120.0f * fTimeStep;
            if(a<0) { 
              a=0;                       
              bKillParticle = true;
            }
            m_Particles[i]->DebrisTint = MAKE_COLOR((int)c,(int)c,(int)c,(int)a);
            break;
          case PT_SMOKE1:
          case PT_SMOKE2:
            m_Particles[i]->fSmokeSize += fTimeStep * 1.3f; /* grow */
            m_Particles[i]->Acc = Vector2f(0.2,0.5);  /* accelerate upwards */
            c=GET_RED(m_Particles[i]->SmokeColor);
            c += randomNum(40,50) * fTimeStep;
            a=GET_ALPHA(m_Particles[i]->SmokeColor);
            a -= 60.0f * fTimeStep;
            if(c>255) c=255;
            if(a<0) { 
              a=0;                       
              bKillParticle = true;
            }
            else
              m_Particles[i]->SmokeColor = MAKE_COLOR((int)c,(int)c,(int)c,(int)a);
            break;
            
          case PT_FIRE:
            c=GET_GREEN(m_Particles[i]->FireColor);
            c -= randomNum(190,210) * fTimeStep;
            if(c<0) c=0;
            c2=GET_BLUE(m_Particles[i]->FireColor);
            c2 -= randomNum(400,400) * fTimeStep;
            if(c2<0) c2=0;
            a=GET_ALPHA(m_Particles[i]->FireColor);
            a -= 180.0f * fTimeStep;
            if(a<0) { 
              a=0;                       
              bKillParticle = true;
            }
            else
              m_Particles[i]->FireColor = MAKE_COLOR(255,(int)c,(int)c2,(int)a);
            
            m_Particles[i]->Vel.x = sin((pGame->getTime() - 
                                         m_Particles[i]->fSpawnTime + m_Particles[i]->fFireSeed)*randomNum(5,15)) * 0.8f
                                      +
                                    sin((pGame->getTime()-m_Particles[i]->fFireSeed) * 10) * 0.3;
            m_Particles[i]->Acc.y = 2;
            break;
        }
        
        /* Show we kill it? */
        if(bKillParticle) {
          delete m_Particles[i];
          m_Particles.erase(m_Particles.begin() + i);
        }
        else i++;
      }
    }
  }  
  
  void GameRenderer::_RenderParticle(Vector2f P,Texture *pTexture,float fSize,float fAngle,Color c) {
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
    glColor4ub(GET_RED(c),GET_GREEN(c),GET_BLUE(c),GET_ALPHA(c));
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
  
  void GameRenderer::_RenderParticles(bool bFront) {
    /* Render all particles */
    for(int i=0;i<m_Particles.size();i++) {
      if(m_Particles[i]->bFront == bFront) {
        switch(m_Particles[i]->Type) {
          case PT_SMOKE1:
            _RenderParticle(m_Particles[i]->Pos,m_pSmoke1,m_Particles[i]->fSmokeSize,
                            m_Particles[i]->fAng,m_Particles[i]->SmokeColor);
            break;
          case PT_SMOKE2:
            _RenderParticle(m_Particles[i]->Pos,m_pSmoke2,m_Particles[i]->fSmokeSize,
                            m_Particles[i]->fAng,m_Particles[i]->SmokeColor);
            break;
          case PT_FIRE:
            _RenderParticle(m_Particles[i]->Pos,m_pFire1,m_Particles[i]->fFireSize,
                            m_Particles[i]->fAng,m_Particles[i]->FireColor);
            break;
          case PT_DEBRIS:
            _RenderParticle(m_Particles[i]->Pos,m_pDirt1,m_Particles[i]->fDebrisSize,
                            m_Particles[i]->fAng,m_Particles[i]->DebrisTint);
            break;
        }
      }
    }
  }

};

