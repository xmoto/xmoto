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
  void GameRenderer::_RenderBike(BikeState *pBike, BikeParameters *pBikeParms, BikerTheme *p_theme, bool i_renderBikeFront) {
    Sprite *pSprite;
    Texture *pTexture;

    /* Render bike */
    Vector2f p0,p1,p2,p3,o0,o1,o2,o3;
    Vector2f C;
    Vector2f Sv,Rc,Fc;

    /* Draw front wheel */
    /* Ugly mode? */
    if(m_bUglyMode) {
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

    if(m_bUglyMode == false) {
      pSprite = p_theme->getWheel();
      if(pSprite != NULL) {
	pTexture = pSprite->getTexture();
	if(pTexture != NULL) {
	  _RenderAlphaBlendedSection(pTexture,p0+C,p1+C,p2+C,p3+C);
	}
      }
    }

    if(m_bUglyMode || m_bTestThemeMode) {
      getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
      getParent()->getDrawLib()->setColor(p_theme->getUglyWheelColor());
      getParent()->getDrawLib()->glVertex(p0+C);    
      getParent()->getDrawLib()->glVertex(p2+C);
      getParent()->getDrawLib()->endDraw();
      getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
      getParent()->getDrawLib()->setColor(p_theme->getUglyWheelColor());
      getParent()->getDrawLib()->glVertex(p1+C);
      getParent()->getDrawLib()->glVertex(p3+C);
      getParent()->getDrawLib()->endDraw();
      _RenderCircle(16,p_theme->getUglyWheelColor(),C,pBikeParms->WR);
    }

    /* Draw rear wheel */        
    /* Ugly mode? */
    if(m_bUglyMode) {
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
    if(m_bUglyMode == false) {
      pSprite = p_theme->getWheel();
      if(pSprite != NULL) {
	pTexture = pSprite->getTexture();
	if(pTexture != NULL) {
	  _RenderAlphaBlendedSection(p_theme->getWheel()->getTexture(),p0+C,p1+C,p2+C,p3+C);
	}
      }
    }

    if(m_bUglyMode || m_bTestThemeMode) {
      getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
      getParent()->getDrawLib()->setColor(p_theme->getUglyWheelColor());
      getParent()->getDrawLib()->glVertex(p0+C);    
      getParent()->getDrawLib()->glVertex(p2+C);
      getParent()->getDrawLib()->endDraw();
      getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
      getParent()->getDrawLib()->setColor(p_theme->getUglyWheelColor());
      getParent()->getDrawLib()->glVertex(p1+C);
      getParent()->getDrawLib()->glVertex(p3+C);
      getParent()->getDrawLib()->endDraw();
      _RenderCircle(16,p_theme->getUglyWheelColor(),C,pBikeParms->WR);
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

      pSprite = p_theme->getRear();
      if(pSprite != NULL) {
	pTexture = pSprite->getTexture();
	if(pTexture != NULL) {
	  _RenderAlphaBlendedSection(pTexture,p0,p1,p2,p3);
	}
      }

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

      if(i_renderBikeFront) {
	pSprite = p_theme->getFront();
	if(pSprite != NULL) {
	  pTexture = pSprite->getTexture();
	  if(pTexture != NULL) {
	    _RenderAlphaBlendedSection(pTexture,p3,p0,p1,p2);
	  }
	}    
      }  

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
      
      C = pBike->CenterP; 

      pSprite = p_theme->getBody();
      if(pSprite != NULL) {
	pTexture = pSprite->getTexture();
	if(pTexture != NULL) {
	  if(pBike->Dir == DD_RIGHT) {
	    _RenderAlphaBlendedSection(pTexture,p3+C,p2+C,p1+C,p0+C);
	  } else {
	    _RenderAlphaBlendedSection(pTexture,p2+C,p3+C,p0+C,p1+C);
	  }
	}
      }
    }

    /* Draw rider */        
    if(pBike->Dir == DD_RIGHT) {
      if(m_bUglyMode == false) {    
        /* Draw rider torso */
        Sv = pBike->ShoulderP - pBike->LowerBodyP;
        Sv.normalize();         
        p0 = pBike->ShoulderP + Vector2f(-Sv.y,Sv.x)*0.24f + Sv*0.46f;
        p1 = pBike->LowerBodyP + Vector2f(-Sv.y,Sv.x)*0.24f - Sv*0.1f;
        p2 = pBike->LowerBodyP - Vector2f(-Sv.y,Sv.x)*0.24f - Sv*0.1f;
        p3 = pBike->ShoulderP - Vector2f(-Sv.y,Sv.x)*0.24f + Sv*0.46f;

	      pSprite = p_theme->getTorso();
	      if(pSprite != NULL) {
	        pTexture = pSprite->getTexture();
	        if(pTexture != NULL) {
	          _RenderAlphaBlendedSection(pTexture,p1,p2,p3,p0);
	        }
	      }        

        /* Draw rider upper leg */
        Sv = pBike->LowerBodyP - pBike->KneeP;
        Sv.normalize();         
        p0 = pBike->LowerBodyP + Vector2f(-Sv.y,Sv.x)*0.20f + Sv*0.14f;
        p1 = pBike->KneeP + Vector2f(-Sv.y,Sv.x)*0.15f + Sv*0.0f;
        p2 = pBike->KneeP - Vector2f(-Sv.y,Sv.x)*0.15f + Sv*0.0f;
        p3 = pBike->LowerBodyP - Vector2f(-Sv.y,Sv.x)*0.1f + Sv*0.14f;

	      pSprite = p_theme->getUpperLeg();
	      if(pSprite != NULL) {
	        pTexture = pSprite->getTexture();
	        if(pTexture != NULL) {
	          _RenderAlphaBlendedSection(pTexture,p0,p1,p2,p3);
	        }
	      }        

        /* Draw rider lower leg */
        Sv = pBike->KneeP - pBike->FootP;
        Sv.normalize();         
        p0 = pBike->KneeP + Vector2f(-Sv.y,Sv.x)*0.23f + Sv*0.01f;
        p1 = pBike->FootP + Vector2f(-Sv.y,Sv.x)*0.2f;
        p2 = pBike->FootP - Vector2f(-Sv.y,Sv.x)*0.2f;
        p3 = pBike->KneeP - Vector2f(-Sv.y,Sv.x)*0.23f + Sv*0.1f;

	      pSprite = p_theme->getLowerLeg();
	      if(pSprite != NULL) {
	        pTexture = pSprite->getTexture();
	        if(pTexture != NULL) {
	          _RenderAlphaBlendedSection(pTexture,p1,p2,p3,p0);        
	        }
	      }

        /* Draw rider upper arm */
        Sv = pBike->ShoulderP - pBike->ElbowP;
        Sv.normalize();         
        p0 = pBike->ShoulderP + Vector2f(-Sv.y,Sv.x)*0.12f;
        p1 = pBike->ElbowP + Vector2f(-Sv.y,Sv.x)*0.12f - Sv*0.05f;
        p2 = pBike->ElbowP - Vector2f(-Sv.y,Sv.x)*0.10f - Sv*0.05f;
        p3 = pBike->ShoulderP - Vector2f(-Sv.y,Sv.x)*0.10f;

	      pSprite = p_theme->getUpperArm();
	      if(pSprite != NULL) {
	        pTexture = pSprite->getTexture();
	        if(pTexture != NULL) {
	          _RenderAlphaBlendedSection(pTexture,p1,p2,p3,p0);
	        }
	      }        

        /* Draw rider lower arm */
        Sv = pBike->ElbowP - pBike->HandP;
        Sv.normalize();         
        p0 = pBike->ElbowP + Vector2f(-Sv.y,Sv.x)*0.12f + Sv*0.09f;
        p1 = pBike->HandP + Vector2f(-Sv.y,Sv.x)*0.12f - Sv*0.05f;
        p2 = pBike->HandP - Vector2f(-Sv.y,Sv.x)*0.10f - Sv*0.05f;
        p3 = pBike->ElbowP - Vector2f(-Sv.y,Sv.x)*0.10f + Sv*0.09f;
        
	      pSprite = p_theme->getLowerArm();
	      if(pSprite != NULL) {
	        pTexture = pSprite->getTexture();
	        if(pTexture != NULL) {
	          _RenderAlphaBlendedSection(pTexture,p3,p2,p1,p0);
	        }
	      }        
      }

      if(m_bUglyMode || m_bTestThemeMode) {
        /* Draw it ugly */
	getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
	getParent()->getDrawLib()->setColor(p_theme->getUglyRiderColor());
        getParent()->getDrawLib()->glVertex(pBike->FootP);
        getParent()->getDrawLib()->glVertex(pBike->KneeP);
        getParent()->getDrawLib()->glVertex(pBike->LowerBodyP);
        getParent()->getDrawLib()->glVertex(pBike->ShoulderP);
        getParent()->getDrawLib()->glVertex(pBike->ElbowP);
        getParent()->getDrawLib()->glVertex(pBike->HandP);
	getParent()->getDrawLib()->endDraw();
        _RenderCircle(10,p_theme->getUglyRiderColor(),pBike->HeadP,pBikeParms->fHeadSize);
      }
    }
    else if(pBike->Dir == DD_LEFT) {
      if(m_bUglyMode == false) {    
        /* Draw rider torso */
        Sv = pBike->Shoulder2P - pBike->LowerBody2P;
        Sv.normalize();         
        p0 = pBike->Shoulder2P + Vector2f(-Sv.y,Sv.x)*0.24f + Sv*0.46f;
        p1 = pBike->LowerBody2P + Vector2f(-Sv.y,Sv.x)*0.24f - Sv*0.1f;
        p2 = pBike->LowerBody2P - Vector2f(-Sv.y,Sv.x)*0.24f - Sv*0.1f;
        p3 = pBike->Shoulder2P - Vector2f(-Sv.y,Sv.x)*0.24f + Sv*0.46f;

	      pSprite = p_theme->getTorso();
	      if(pSprite != NULL) {
	        pTexture = pSprite->getTexture();
	        if(pTexture != NULL) {
	          _RenderAlphaBlendedSection(pTexture,p2,p1,p0,p3);        
	        }
	      }                

        /* Draw rider upper leg */
        Sv = pBike->LowerBody2P - pBike->Knee2P;
        Sv.normalize();         
        p0 = pBike->LowerBody2P + Vector2f(-Sv.y,Sv.x)*0.20f + Sv*0.14f;
        p1 = pBike->Knee2P + Vector2f(-Sv.y,Sv.x)*0.15f + Sv*0.0f;
        p2 = pBike->Knee2P - Vector2f(-Sv.y,Sv.x)*0.15f + Sv*0.0f;
        p3 = pBike->LowerBody2P - Vector2f(-Sv.y,Sv.x)*0.1f + Sv*0.14f;

	      pSprite = p_theme->getUpperLeg();
	      if(pSprite != NULL) {
	        pTexture = pSprite->getTexture();
	        if(pTexture != NULL) {
	          _RenderAlphaBlendedSection(pTexture,p3,p2,p1,p0);        
	        }
	      }

        /* Draw rider lower leg */
        Sv = pBike->Knee2P - pBike->Foot2P;
        Sv.normalize();         
        p0 = pBike->Knee2P + Vector2f(-Sv.y,Sv.x)*0.23f + Sv*0.01f;
        p1 = pBike->Foot2P + Vector2f(-Sv.y,Sv.x)*0.2f;
        p2 = pBike->Foot2P - Vector2f(-Sv.y,Sv.x)*0.2f;
        p3 = pBike->Knee2P - Vector2f(-Sv.y,Sv.x)*0.23f + Sv*0.1f;

	      pSprite = p_theme->getLowerLeg();
	      if(pSprite != NULL) {
	        pTexture = pSprite->getTexture();
	        if(pTexture != NULL) {
	          _RenderAlphaBlendedSection(pTexture,p2,p1,p0,p3);        
	        }
	      }

        /* Draw rider upper arm */
        Sv = pBike->Shoulder2P - pBike->Elbow2P;
        Sv.normalize();         
        p0 = pBike->Shoulder2P + Vector2f(-Sv.y,Sv.x)*0.12f;
        p1 = pBike->Elbow2P + Vector2f(-Sv.y,Sv.x)*0.12f - Sv*0.05f;
        p2 = pBike->Elbow2P - Vector2f(-Sv.y,Sv.x)*0.10f - Sv*0.05f;
        p3 = pBike->Shoulder2P - Vector2f(-Sv.y,Sv.x)*0.10f;

	      pSprite = p_theme->getUpperArm();
	      if(pSprite != NULL) {
	        pTexture = pSprite->getTexture();
	        if(pTexture != NULL) {
	          _RenderAlphaBlendedSection(pTexture,p2,p1,p0,p3);        
	        }
	      }        

        /* Draw rider lower arm */
        Sv = pBike->Elbow2P - pBike->Hand2P;
        Sv.normalize();         
        p0 = pBike->Elbow2P + Vector2f(-Sv.y,Sv.x)*0.12f + Sv*0.09f;
        p1 = pBike->Hand2P + Vector2f(-Sv.y,Sv.x)*0.12f - Sv*0.05f;
        p2 = pBike->Hand2P - Vector2f(-Sv.y,Sv.x)*0.10f - Sv*0.05f;
        p3 = pBike->Elbow2P - Vector2f(-Sv.y,Sv.x)*0.10f + Sv*0.09f;

	      pSprite = p_theme->getLowerArm();
	      if(pSprite != NULL) {
	        pTexture = pSprite->getTexture();
	        if(pTexture != NULL) {
	          _RenderAlphaBlendedSection(pTexture,p0,p1,p2,p3);        
	        }
	      }
      }

      if(m_bUglyMode || m_bTestThemeMode) {
        /* Draw it ugly */
	getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
	getParent()->getDrawLib()->setColor(p_theme->getUglyRiderColor());
        getParent()->getDrawLib()->glVertex(pBike->Foot2P);
        getParent()->getDrawLib()->glVertex(pBike->Knee2P);
        getParent()->getDrawLib()->glVertex(pBike->LowerBody2P);
        getParent()->getDrawLib()->glVertex(pBike->Shoulder2P);
        getParent()->getDrawLib()->glVertex(pBike->Elbow2P);
        getParent()->getDrawLib()->glVertex(pBike->Hand2P);
        getParent()->getDrawLib()->endDraw();
        _RenderCircle(10,p_theme->getUglyRiderColor(),pBike->Head2P,pBikeParms->fHeadSize);
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

    getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
    getParent()->getDrawLib()->setColorRGB(255,255,255);
    getParent()->getDrawLib()->glVertex(p0+C);    
    getParent()->getDrawLib()->glVertex(p2+C);
    getParent()->getDrawLib()->endDraw();
    getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
    getParent()->getDrawLib()->setColorRGB(255,255,255);
    getParent()->getDrawLib()->glVertex(p1+C);    
    getParent()->getDrawLib()->glVertex(p3+C);
    getParent()->getDrawLib()->endDraw();
  }
  
}

