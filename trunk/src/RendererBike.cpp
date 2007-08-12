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
 *  In-game rendering - the bike
 */
#include "VXml.h"
#include "VFileIO.h"
#include "xmscene/Scene.h"
#include "Renderer.h"
#include "VDraw.h"

  void GameRenderer::renderBodyPart(const Vector2f& i_from, const Vector2f& i_to,
				    float i_c11, float i_c12,
				    float i_c21, float i_c22,
				    float i_c31, float i_c32,
				    float i_c41, float i_c42,
				    Sprite *i_sprite,
				    const TColor& i_filterColor,
				    DriveDir i_direction,
				    int i_90_rotation
				    ) {
    Texture *pTexture;
    Vector2f Sv;
    Vector2f p0, p1, p2, p3;

    if(i_sprite == NULL) return;
    pTexture = i_sprite->getTexture(false, false, FM_LINEAR); // FM_LINEAR
    if(pTexture == NULL) return;

    Sv = i_from - i_to;
    Sv.normalize();

    if(i_direction == DD_RIGHT) {
      p0 = i_from + Vector2f(-Sv.y, Sv.x) * i_c11 + Sv * i_c12;
      p1 = i_to   + Vector2f(-Sv.y, Sv.x) * i_c21 + Sv * i_c22;
      p2 = i_to   - Vector2f(-Sv.y, Sv.x) * i_c31 + Sv * i_c32;
      p3 = i_from - Vector2f(-Sv.y, Sv.x) * i_c41 + Sv * i_c42;
    } else {
      p0 = i_from - Vector2f(-Sv.y, Sv.x) * i_c11 + Sv * i_c12;
      p1 = i_to   - Vector2f(-Sv.y, Sv.x) * i_c21 + Sv * i_c22;
      p2 = i_to   + Vector2f(-Sv.y, Sv.x) * i_c31 + Sv * i_c32;
      p3 = i_from + Vector2f(-Sv.y, Sv.x) * i_c41 + Sv * i_c42;
    }

    switch(i_90_rotation) {
      case 0:
      _RenderAlphaBlendedSection(pTexture, p1, p2, p3, p0, i_filterColor);
      break;
      case 1:
      _RenderAlphaBlendedSection(pTexture, p0, p1, p2, p3, i_filterColor);
      break;
      case 2:
      _RenderAlphaBlendedSection(pTexture, p3, p2, p1, p0, i_filterColor);
      break;
    }
  }

  /*===========================================================================
  Rendering of the bike
  ===========================================================================*/
  void GameRenderer::_RenderBike(BikeState *pBike, BikeParameters *pBikeParms, BikerTheme *p_theme, bool i_renderBikeFront,
				 const TColor&  i_filterColor, const TColor&  i_filterUglyColor) {
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
	pTexture = pSprite->getTexture(false, false, FM_LINEAR);
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
	pTexture = pSprite->getTexture(false, false, FM_LINEAR);
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
	pTexture = pSprite->getTexture(false, false, FM_LINEAR);
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
	  pTexture = pSprite->getTexture(false, false, FM_LINEAR);
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
	pTexture = pSprite->getTexture(false, false, FM_LINEAR);
	if(pTexture != NULL) {
	  if(pBike->Dir == DD_RIGHT) {
	    _RenderAlphaBlendedSection(pTexture,p3+C,p2+C,p1+C,p0+C, i_filterColor);
	  } else {
	    _RenderAlphaBlendedSection(pTexture,p2+C,p3+C,p0+C,p1+C, i_filterColor);
	  }
	}
      }
    }

      /* Draw rider */
    if(m_bUglyMode == false) { 
      /* torso */
      renderBodyPart(pBike->Dir == DD_RIGHT ? pBike->ShoulderP  : pBike->Shoulder2P,
		     pBike->Dir == DD_RIGHT ? pBike->LowerBodyP : pBike->LowerBody2P,
		     0.24, 0.46,
		     0.24, -0.1,
		     0.24, -0.1,
		     0.24, 0.46,
		     p_theme->getTorso(),
		     i_filterColor,
		     pBike->Dir
		     );

      /* upper leg */
      renderBodyPart(pBike->Dir == DD_RIGHT ? pBike->LowerBodyP : pBike->LowerBody2P,
		     pBike->Dir == DD_RIGHT ? pBike->KneeP      : pBike->Knee2P,
		     0.20, 0.14,
		     0.15, 0.00,
		     0.15, 0.00,
		     0.10, 0.14,
		     p_theme->getUpperLeg(),
		     i_filterColor,
		     pBike->Dir, 1
		     );

      /* lower leg */
      renderBodyPart(pBike->Dir == DD_RIGHT ? pBike->KneeP : pBike->Knee2P,
		     pBike->Dir == DD_RIGHT ? pBike->FootP : pBike->Foot2P,
		     0.23, 0.01,
		     0.20, 0.00,
		     0.20, 0.00,
		     0.23, 0.10,
		     p_theme->getLowerLeg(),
		     i_filterColor,
		     pBike->Dir
		     );

      /* upper arm */
      renderBodyPart(pBike->Dir == DD_RIGHT ? pBike->ShoulderP : pBike->Shoulder2P,
		     pBike->Dir == DD_RIGHT ? pBike->ElbowP    : pBike->Elbow2P,
		     0.12, 0.09,
		     0.12, -0.05,
		     0.10, -0.05,
		     0.10, 0.09,
		     p_theme->getUpperArm(),
		     i_filterColor,
		     pBike->Dir
		     );

      /* lower arm */
      renderBodyPart(pBike->Dir == DD_RIGHT ? pBike->ElbowP : pBike->Elbow2P,
		     pBike->Dir == DD_RIGHT ? pBike->HandP  : pBike->Hand2P,
		     0.12, 0.09,
		     0.12, -0.05,
		     0.10, -0.05,
		     0.10, 0.09,
		     p_theme->getLowerArm(),
		     i_filterColor,
		     pBike->Dir, 2
		     );

    }

    if(pBike->Dir == DD_RIGHT) {
      if(m_bUglyMode || m_bTestThemeMode) {
        /* Draw it ugly */
	getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
	getParent()->getDrawLib()->setColor(MAKE_COLOR(i_filterUglyColor.Red(),
						       i_filterUglyColor.Green(),
						       i_filterUglyColor.Blue(),
						       i_filterUglyColor.Alpha()));
        getParent()->getDrawLib()->glVertex(pBike->FootP);
        getParent()->getDrawLib()->glVertex(pBike->KneeP);
        getParent()->getDrawLib()->glVertex(pBike->LowerBodyP);
        getParent()->getDrawLib()->glVertex(pBike->ShoulderP);
        getParent()->getDrawLib()->glVertex(pBike->ElbowP);
        getParent()->getDrawLib()->glVertex(pBike->HandP);
	getParent()->getDrawLib()->endDraw();
        _RenderCircle(10, MAKE_COLOR(i_filterUglyColor.Red(),
				     i_filterUglyColor.Green(),
				     i_filterUglyColor.Blue(),
				     i_filterUglyColor.Alpha()),pBike->HeadP,pBikeParms->fHeadSize);
      }
    }
    else if(pBike->Dir == DD_LEFT) {
      if(m_bUglyMode || m_bTestThemeMode) {
        /* Draw it ugly */
	getParent()->getDrawLib()->startDraw(DRAW_MODE_LINE_STRIP);
	getParent()->getDrawLib()->setColor(MAKE_COLOR(i_filterUglyColor.Red(),
						       i_filterUglyColor.Green(),
						       i_filterUglyColor.Blue(),
						       i_filterUglyColor.Alpha()));
        getParent()->getDrawLib()->glVertex(pBike->Foot2P);
        getParent()->getDrawLib()->glVertex(pBike->Knee2P);
        getParent()->getDrawLib()->glVertex(pBike->LowerBody2P);
        getParent()->getDrawLib()->glVertex(pBike->Shoulder2P);
        getParent()->getDrawLib()->glVertex(pBike->Elbow2P);
        getParent()->getDrawLib()->glVertex(pBike->Hand2P);
        getParent()->getDrawLib()->endDraw();
        _RenderCircle(10, MAKE_COLOR(i_filterUglyColor.Red(),
				     i_filterUglyColor.Green(),
				     i_filterUglyColor.Blue(),
				     i_filterUglyColor.Alpha()), pBike->Head2P,pBikeParms->fHeadSize);
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

