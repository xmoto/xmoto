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

#ifndef __VDRAW_H__
#define __VDRAW_H__

#include "VCommon.h"

#include "VMath.h"
#include "VTexture.h"

namespace vapp {
  /*===========================================================================
  Class with various drawing functions
  ===========================================================================*/
  class DrawLib {
    public:
      /* Methods - low-level */
      void glVertex(float x,float y);
      void screenProjVertex(float *x,float *y);      
      
      /* Methods - primitives */
      void drawCircle(const Vector2f &Center,float fRadius,float fBorder=1.0f,Color Back=0,Color Front=-1);
      void drawBox(const Vector2f &A,const Vector2f &B,float fBorder=1.0f,Color Back=0,Color Front=-1);      
      void drawImage(const Vector2f &A,const Vector2f &B,Texture *pTexture,Color Tint=-1);
      
      /* Methods - text */
      void drawText(const Vector2f &Pos,std::string Text,Color Back=0,Color Front=-1,bool bEdge=false);
      int getTextWidth(std::string Text);
      int getTextHeight(std::string Text);
        
    protected:
    
      /* Protected methods */
      void setDrawDims(int nActualW,int nActualH,int w,int h) {m_nDrawWidth=w; m_nDrawHeight=h;
                                                               m_nActualWidth=nActualW; m_nActualHeight=nActualH;}
      void initLib(TextureManager *pTextureManager);
      void uninitLib(TextureManager *pTextureManager);
    
    private:
      /* Data */
      int m_nDrawWidth,m_nDrawHeight;     
      int m_nActualWidth,m_nActualHeight;     
      Texture *m_pDefaultFontTexture;             
      
      /* Public helper methods */
      void _InitTextRendering(TextureManager *pTextureManager);
      void _UninitTextRendering(TextureManager *pTextureManager);
  };
  
};

#endif

