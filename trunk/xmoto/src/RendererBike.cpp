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
 *  In-game rendering - the bike
 */
#include "VXml.h"
#include "VFileIO.h"
#include "MotoGame.h"
#include "Renderer.h"

namespace vapp {

  /*===========================================================================
  Rendering of the bike
  ===========================================================================*/
  void GameRenderer::_RenderBike(BikeState *pBike, TextureTheme *p_theme) {
    /* Render bike */
    Vector2f p0,p1,p2,p3,o0,o1,o2,o3;
    Vector2f C;
    Vector2f Sv,Rc,Fc;
      
    /* Draw front wheel */
    /* Ugly mode? */
    if(m_bUglyMode) {
      o0 = Vector2f(-pBike->pParams->WR,0);
      o1 = Vector2f(0,pBike->pParams->WR);
      o2 = Vector2f(pBike->pParams->WR,0);
      o3 = Vector2f(0,-pBike->pParams->WR);
    }
    else {
      o0 = Vector2f(-pBike->pParams->WR,pBike->pParams->WR);
      o1 = Vector2f(pBike->pParams->WR,pBike->pParams->WR);
      o2 = Vector2f(pBike->pParams->WR,-pBike->pParams->WR);
      o3 = Vector2f(-pBike->pParams->WR,-pBike->pParams->WR);
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
    if(m_bUglyMode) {
      glBegin(GL_LINE_STRIP);
      glColor3f(1,0,0);
      _Vertex(p0+C);    
      _Vertex(p2+C);
      glEnd();
      glBegin(GL_LINE_STRIP);
      glColor3f(1,0,0);
      _Vertex(p1+C);
      _Vertex(p3+C);
      glEnd();
      int nSteps = 16;
      glBegin(GL_LINE_LOOP);              
      glColor3f(1,0,0);
      for(int i=0;i<nSteps;i++) {
        float r = (3.14159f * 2.0f * (float)i)/ (float)nSteps;            
        _Vertex( Vector2f(C.x + pBike->pParams->WR*sin(r),C.y + pBike->pParams->WR*cos(r)) );
      }      
      glEnd();
    }
    else {
      glEnable(GL_BLEND); 
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
      glBindTexture(GL_TEXTURE_2D,p_theme->BikeWheel->nID);
      glEnable(GL_TEXTURE_2D);
      glBegin(GL_POLYGON);
      glColor3f(1,1,1);
      glTexCoord2f(0,1);
      _Vertex(p0+C);    
      glTexCoord2f(1,1);
      _Vertex(p1+C);
      glTexCoord2f(1,0);
      _Vertex(p2+C);
      glTexCoord2f(0,0);
      _Vertex(p3+C);
      glEnd();
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_BLEND);
    }

    /* Draw rear wheel */        
    /* Ugly mode? */
    if(m_bUglyMode) {
      o0 = Vector2f(-pBike->pParams->WR,0);
      o1 = Vector2f(0,pBike->pParams->WR);
      o2 = Vector2f(pBike->pParams->WR,0);
      o3 = Vector2f(0,-pBike->pParams->WR);
    }
    else {
      o0 = Vector2f(-pBike->pParams->WR,pBike->pParams->WR);
      o1 = Vector2f(pBike->pParams->WR,pBike->pParams->WR);
      o2 = Vector2f(pBike->pParams->WR,-pBike->pParams->WR);
      o3 = Vector2f(-pBike->pParams->WR,-pBike->pParams->WR);
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
    if(m_bUglyMode) {
      glBegin(GL_LINE_STRIP);
      glColor3f(1,0,0);
      _Vertex(p0+C);    
      _Vertex(p2+C);
      glEnd();
      glBegin(GL_LINE_STRIP);
      glColor3f(1,0,0);
      _Vertex(p1+C);
      _Vertex(p3+C);
      glEnd();
      int nSteps = 16;
      glBegin(GL_LINE_LOOP);              
      glColor3f(1,0,0);
      for(int i=0;i<nSteps;i++) {
        float r = (3.14159f * 2.0f * (float)i)/ (float)nSteps;            
        _Vertex( Vector2f(C.x + pBike->pParams->WR*sin(r),C.y + pBike->pParams->WR*cos(r)) );
      }      
      glEnd();
    }
    else {
      glEnable(GL_BLEND); 
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
      glBindTexture(GL_TEXTURE_2D,p_theme->BikeWheel->nID);
      glEnable(GL_TEXTURE_2D);
      glBegin(GL_POLYGON);
      glColor3f(1,1,1);
      glTexCoord2f(0,1);
      _Vertex(p0+C);    
      glTexCoord2f(1,1);
      _Vertex(p1+C);
      glTexCoord2f(1,0);
      _Vertex(p2+C);
      glTexCoord2f(0,0);
      _Vertex(p3+C);
      glEnd();
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_BLEND);
    }

    if(!m_bUglyMode) {
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

      glEnable(GL_BLEND); 
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
      glBindTexture(GL_TEXTURE_2D,p_theme->BikeRear->nID);
      glEnable(GL_TEXTURE_2D);
      glBegin(GL_POLYGON);
      glColor3f(1,1,1);
      glTexCoord2f(0,0);
      _Vertex(p0);    
      glTexCoord2f(1,0);
      _Vertex(p1);
      glTexCoord2f(1,1);
      _Vertex(p2);
      glTexCoord2f(0,1);
      _Vertex(p3);
      glEnd();
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_BLEND);        
   
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
      
      glEnable(GL_BLEND); 
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
      glBindTexture(GL_TEXTURE_2D,p_theme->BikeFront->nID);
      glEnable(GL_TEXTURE_2D);
      glBegin(GL_POLYGON);
      glColor3f(1,1,1);
      glTexCoord2f(1,0);
      _Vertex(p0);    
      glTexCoord2f(1,1);
      _Vertex(p1);
      glTexCoord2f(0,1);
      _Vertex(p2);
      glTexCoord2f(0,0);
      _Vertex(p3);
      glEnd();
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_BLEND);        

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
      
      C = pBike->CenterP; //Vector2f(pBike->pfFramePos[0],pBike->pfFramePos[1]);        
      
      glEnable(GL_BLEND); 
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
      glBindTexture(GL_TEXTURE_2D,p_theme->BikeBody->nID);
      glEnable(GL_TEXTURE_2D);
      glBegin(GL_POLYGON);
      glColor3f(1,1,1);
      if(pBike->Dir == DD_RIGHT) {
        glTexCoord2f(0,0);
        _Vertex(p0+C);    
        glTexCoord2f(1,0);
        _Vertex(p1+C);
        glTexCoord2f(1,1);
        _Vertex(p2+C);
        glTexCoord2f(0,1);
        _Vertex(p3+C);
      }
      else {
        glTexCoord2f(1,0);
        _Vertex(p0+C);    
        glTexCoord2f(0,0);
        _Vertex(p1+C);
        glTexCoord2f(0,1);
        _Vertex(p2+C);
        glTexCoord2f(1,1);
        _Vertex(p3+C);
      }
      glEnd();
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_BLEND);
    }

    /* Draw rider */        
    if(pBike->Dir == DD_RIGHT) {
      if(m_bUglyMode) {
        /* Draw it ugly */
        glBegin(GL_LINE_STRIP);
        glColor3f(0,1,0);
        _Vertex(pBike->FootP);
        _Vertex(pBike->KneeP);
        _Vertex(pBike->LowerBodyP);
        _Vertex(pBike->ShoulderP);
        _Vertex(pBike->ElbowP);
        _Vertex(pBike->HandP);
        glEnd();
        int nSteps = 10;
        glBegin(GL_LINE_LOOP);              
        glColor3f(0,1,0);
        for(int i=0;i<nSteps;i++) {
          float r = (3.14159f * 2.0f * (float)i)/ (float)nSteps;            
          _Vertex( Vector2f(pBike->HeadP.x + pBike->pParams->fHeadSize*sin(r),
                            pBike->HeadP.y + pBike->pParams->fHeadSize*cos(r)) );
        }      
        glEnd();
      }
      else {      
        /* Draw rider torso */
        Sv = pBike->ShoulderP - pBike->LowerBodyP;
        Sv.normalize();         
        p0 = pBike->ShoulderP + Vector2f(-Sv.y,Sv.x)*0.24f + Sv*0.46f;
        p1 = pBike->LowerBodyP + Vector2f(-Sv.y,Sv.x)*0.24f - Sv*0.1f;
        p2 = pBike->LowerBodyP - Vector2f(-Sv.y,Sv.x)*0.24f - Sv*0.1f;
        p3 = pBike->ShoulderP - Vector2f(-Sv.y,Sv.x)*0.24f + Sv*0.46f;

        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
        glBindTexture(GL_TEXTURE_2D,p_theme->RiderTorso->nID);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_POLYGON);
        glColor3f(1,1,1);
        glTexCoord2f(0,0);
        _Vertex(p0);    
        glTexCoord2f(0,1);
        _Vertex(p1);
        glTexCoord2f(1,1);
        _Vertex(p2);
        glTexCoord2f(1,0);
        _Vertex(p3);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);        
        
        /* Draw rider upper arm */
        Sv = pBike->ShoulderP - pBike->ElbowP;
        Sv.normalize();         
        p0 = pBike->ShoulderP + Vector2f(-Sv.y,Sv.x)*0.12f;
        p1 = pBike->ElbowP + Vector2f(-Sv.y,Sv.x)*0.12f - Sv*0.05f;
        p2 = pBike->ElbowP - Vector2f(-Sv.y,Sv.x)*0.10f - Sv*0.05f;
        p3 = pBike->ShoulderP - Vector2f(-Sv.y,Sv.x)*0.10f;
        
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
        glBindTexture(GL_TEXTURE_2D,p_theme->RiderUpperArm->nID);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_POLYGON);
        glColor3f(1,1,1);
        glTexCoord2f(0,0);
        _Vertex(p0);    
        glTexCoord2f(0,1);
        _Vertex(p1);
        glTexCoord2f(1,1);
        _Vertex(p2);
        glTexCoord2f(1,0);
        _Vertex(p3);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);        

        /* Draw rider lower arm */
        Sv = pBike->ElbowP - pBike->HandP;
        Sv.normalize();         
        p0 = pBike->ElbowP + Vector2f(-Sv.y,Sv.x)*0.12f + Sv*0.09f;
        p1 = pBike->HandP + Vector2f(-Sv.y,Sv.x)*0.12f - Sv*0.05f;
        p2 = pBike->HandP - Vector2f(-Sv.y,Sv.x)*0.10f - Sv*0.05f;
        p3 = pBike->ElbowP - Vector2f(-Sv.y,Sv.x)*0.10f + Sv*0.09f;
        
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
        glBindTexture(GL_TEXTURE_2D,p_theme->RiderLowerArm->nID);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_POLYGON);
        glColor3f(1,1,1);
        glTexCoord2f(0,0);
        _Vertex(p0);    
        glTexCoord2f(1,0);
        _Vertex(p1);
        glTexCoord2f(1,1);
        _Vertex(p2);
        glTexCoord2f(0,1);
        _Vertex(p3);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);        
        
        /* Draw rider upper leg */
        Sv = pBike->LowerBodyP - pBike->KneeP;
        Sv.normalize();         
        p0 = pBike->LowerBodyP + Vector2f(-Sv.y,Sv.x)*0.20f + Sv*0.14f;
        p1 = pBike->KneeP + Vector2f(-Sv.y,Sv.x)*0.15f + Sv*0.0f;
        p2 = pBike->KneeP - Vector2f(-Sv.y,Sv.x)*0.15f + Sv*0.0f;
        p3 = pBike->LowerBodyP - Vector2f(-Sv.y,Sv.x)*0.1f + Sv*0.14f;
        
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
        glBindTexture(GL_TEXTURE_2D,p_theme->RiderUpperLeg->nID);
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

        /* Draw rider lower leg */
        Sv = pBike->KneeP - pBike->FootP;
        Sv.normalize();         
        p0 = pBike->KneeP + Vector2f(-Sv.y,Sv.x)*0.23f + Sv*0.01f;
        p1 = pBike->FootP + Vector2f(-Sv.y,Sv.x)*0.2f;
        p2 = pBike->FootP - Vector2f(-Sv.y,Sv.x)*0.2f;
        p3 = pBike->KneeP - Vector2f(-Sv.y,Sv.x)*0.23f + Sv*0.1f;
        
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
        glBindTexture(GL_TEXTURE_2D,p_theme->RiderLowerLeg->nID);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_POLYGON);
        glColor3f(1,1,1);
        glTexCoord2f(0,0);
        _Vertex(p0);    
        glTexCoord2f(0,1);
        _Vertex(p1);
        glTexCoord2f(1,1);
        _Vertex(p2);
        glTexCoord2f(1,0);
        _Vertex(p3);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);        
      }
    }
    else if(pBike->Dir == DD_LEFT) {
      if(m_bUglyMode) {
        /* Draw it ugly */
        glBegin(GL_LINE_STRIP);
        glColor3f(0,1,0);
        _Vertex(pBike->Foot2P);
        _Vertex(pBike->Knee2P);
        _Vertex(pBike->LowerBody2P);
        _Vertex(pBike->Shoulder2P);
        _Vertex(pBike->Elbow2P);
        _Vertex(pBike->Hand2P);
        glEnd();
        int nSteps = 10;
        glBegin(GL_LINE_LOOP);              
        glColor3f(0,1,0);
        for(int i=0;i<nSteps;i++) {
          float r = (3.14159f * 2.0f * (float)i)/ (float)nSteps;            
          _Vertex( Vector2f(pBike->Head2P.x + pBike->pParams->fHeadSize*sin(r),
                            pBike->Head2P.y + pBike->pParams->fHeadSize*cos(r)) );
        }      
        glEnd();
      }
      else {      
        /* Draw rider torso */
        Sv = pBike->Shoulder2P - pBike->LowerBody2P;
        Sv.normalize();         
        p0 = pBike->Shoulder2P + Vector2f(-Sv.y,Sv.x)*0.24f + Sv*0.46f;
        p1 = pBike->LowerBody2P + Vector2f(-Sv.y,Sv.x)*0.24f - Sv*0.1f;
        p2 = pBike->LowerBody2P - Vector2f(-Sv.y,Sv.x)*0.24f - Sv*0.1f;
        p3 = pBike->Shoulder2P - Vector2f(-Sv.y,Sv.x)*0.24f + Sv*0.46f;
        
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
        glBindTexture(GL_TEXTURE_2D,p_theme->RiderTorso->nID);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_POLYGON);
        glColor3f(1,1,1);
        glTexCoord2f(1,0);
        _Vertex(p0);    
        glTexCoord2f(1,1);
        _Vertex(p1);
        glTexCoord2f(0,1);
        _Vertex(p2);
        glTexCoord2f(0,0);
        _Vertex(p3);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);        
        
        /* Draw rider upper arm */
        Sv = pBike->Shoulder2P - pBike->Elbow2P;
        Sv.normalize();         
        p0 = pBike->Shoulder2P + Vector2f(-Sv.y,Sv.x)*0.12f;
        p1 = pBike->Elbow2P + Vector2f(-Sv.y,Sv.x)*0.12f - Sv*0.05f;
        p2 = pBike->Elbow2P - Vector2f(-Sv.y,Sv.x)*0.10f - Sv*0.05f;
        p3 = pBike->Shoulder2P - Vector2f(-Sv.y,Sv.x)*0.10f;
        
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
        glBindTexture(GL_TEXTURE_2D,p_theme->RiderUpperArm->nID);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_POLYGON);
        glColor3f(1,1,1);
        glTexCoord2f(1,0);
        _Vertex(p0);    
        glTexCoord2f(1,1);
        _Vertex(p1);
        glTexCoord2f(0,1);
        _Vertex(p2);
        glTexCoord2f(0,0);
        _Vertex(p3);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);        

        /* Draw rider lower arm */
        Sv = pBike->Elbow2P - pBike->Hand2P;
        Sv.normalize();         
        p0 = pBike->Elbow2P + Vector2f(-Sv.y,Sv.x)*0.12f + Sv*0.09f;
        p1 = pBike->Hand2P + Vector2f(-Sv.y,Sv.x)*0.12f - Sv*0.05f;
        p2 = pBike->Hand2P - Vector2f(-Sv.y,Sv.x)*0.10f - Sv*0.05f;
        p3 = pBike->Elbow2P - Vector2f(-Sv.y,Sv.x)*0.10f + Sv*0.09f;
        
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
        glBindTexture(GL_TEXTURE_2D,p_theme->RiderLowerArm->nID);
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

        /* Draw rider upper leg */
        Sv = pBike->LowerBody2P - pBike->Knee2P;
        Sv.normalize();         
        p0 = pBike->LowerBody2P + Vector2f(-Sv.y,Sv.x)*0.20f + Sv*0.14f;
        p1 = pBike->Knee2P + Vector2f(-Sv.y,Sv.x)*0.15f + Sv*0.0f;
        p2 = pBike->Knee2P - Vector2f(-Sv.y,Sv.x)*0.15f + Sv*0.0f;
        p3 = pBike->LowerBody2P - Vector2f(-Sv.y,Sv.x)*0.1f + Sv*0.14f;
        
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
        glBindTexture(GL_TEXTURE_2D,p_theme->RiderUpperLeg->nID);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_POLYGON);
        glColor3f(1,1,1);
        glTexCoord2f(0,0);
        _Vertex(p0);    
        glTexCoord2f(1,0);
        _Vertex(p1);
        glTexCoord2f(1,1);
        _Vertex(p2);
        glTexCoord2f(0,1);
        _Vertex(p3);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);        

        /* Draw rider lower leg */
        Sv = pBike->Knee2P - pBike->Foot2P;
        Sv.normalize();         
        p0 = pBike->Knee2P + Vector2f(-Sv.y,Sv.x)*0.23f + Sv*0.01f;
        p1 = pBike->Foot2P + Vector2f(-Sv.y,Sv.x)*0.2f;
        p2 = pBike->Foot2P - Vector2f(-Sv.y,Sv.x)*0.2f;
        p3 = pBike->Knee2P - Vector2f(-Sv.y,Sv.x)*0.23f + Sv*0.1f;
        
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);         
        glBindTexture(GL_TEXTURE_2D,p_theme->RiderLowerLeg->nID);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_POLYGON);
        glColor3f(1,1,1);
        glTexCoord2f(1,0);
        _Vertex(p0);    
        glTexCoord2f(1,1);
        _Vertex(p1);
        glTexCoord2f(0,1);
        _Vertex(p2);
        glTexCoord2f(0,0);
        _Vertex(p3);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);     
      }
    }   
    
    /* New wheel-spin particles? */
    if(getGameObject()->isWheelSpinning()) {
      //glBegin(GL_LINE_STRIP);      
      //glColor3f(1,0.5,0.2);
      //_Vertex(getGameObject()->getWheelSpinPoint() - Vector2f(0.5,0));
      //_Vertex(getGameObject()->getWheelSpinPoint() + Vector2f(0.5,0));
      //glEnd();
      //glBegin(GL_LINE_STRIP);      
      //glColor3f(1,0.5,0.2);
      //_Vertex(getGameObject()->getWheelSpinPoint() - Vector2f(0,0.5));
      //_Vertex(getGameObject()->getWheelSpinPoint() + Vector2f(0,0.5));
      //glEnd();      
      //glBegin(GL_LINE_STRIP);      
      //glColor3f(0,1,0);
      //_Vertex(getGameObject()->getWheelSpinPoint());
      //_Vertex(getGameObject()->getWheelSpinPoint() + getGameObject()->getWheelSpinDir());
      //glEnd();      

      if(randomNum(0,1) < 0.8f) {
        Particle *pNewParticle = spawnParticle(PT_DEBRIS,getGameObject()->getWheelSpinPoint(),
                                              getGameObject()->getWheelSpinDir(),4);
        pNewParticle->bFront = false;                                              
      }                                             
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

    glBegin(GL_LINE_STRIP);
    glColor3f(1,1,1);
    _Vertex(p0+C);    
    _Vertex(p2+C);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glColor3f(1,1,1);
    _Vertex(p1+C);    
    _Vertex(p3+C);
    glEnd();
  }
  
};

