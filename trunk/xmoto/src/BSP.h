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

#ifndef __BSP_H__
#define __BSP_H__

#include "VCommon.h"
#include "VApp.h"
#include "VMath.h"
#include "LevelSrc.h"
#include "VTexture.h"

namespace vapp {

	/*===========================================================================
	Types
  ===========================================================================*/
  struct BSPLine {
    Vector2f P0,P1;                   /* Line */
    Vector2f Normal;                  /* Linenormal (meaningless, but hey :P)*/
  };  

	/*===========================================================================
	Convex polygon vertex
  ===========================================================================*/
  struct BSPVertex {
    BSPVertex() {}
    BSPVertex(const Vector2f &iP,const Vector2f &iT,std::string iEdgeEffect="") : 
      P(iP), T(iT) {}    
    Vector2f P;                       /* Position */
    Vector2f T;                       /* Texture coordinates */
  };

	/*===========================================================================
	Convex polygon
  ===========================================================================*/
  struct BSPPoly {
    BSPPoly() {
      pTexture = NULL;
    }
    ~BSPPoly();                       /* Destructor */      
    void debugOutput(void) {
      //printf("POLY %X with %d vertices:\n",this,Vertices.size());
      //for(int i=0;i<Vertices.size();i++)
      //  printf(" <%f, %f>\n",Vertices[i]->P.x,Vertices[i]->P.y);
      //printf("\n");
    }
    std::vector<BSPVertex *> Vertices; /* Vertices */
    Texture *pTexture;                /* Texture */
  };

	/*===========================================================================
	BSP class
  ===========================================================================*/
  class BSP {
    public:
      BSP() {m_nNumErrors=0;}
      ~BSP() {_Cleanup();}
    
      /* Methods */
      void addLineDef(Vector2f P0,Vector2f P1);
      std::vector<BSPPoly *> &compute(void);            
      
      int getNumErrors(void) {return m_nNumErrors;}
      
    private:
      /* Data */
      std::vector<BSPLine *> m_Lines;     /* Input data set */
      std::vector<BSPPoly *> m_Polys;     /* Output data set */
      
      int m_nNumErrors;                   /* Number of errors found */
      
      /* Helpers */
      void _Cleanup(void);
      void _ComputeNormal(BSPLine *pLine);     
      void _UpdateAABB(Vector2f &P,Vector2f &Min,Vector2f &Max); 
      void _Recurse(BSPPoly *pSubSpace,std::vector<BSPLine *> &Lines);
      void _SplitPoly(BSPPoly *pPoly,BSPPoly *pFront,BSPPoly *pBack,BSPLine *pLine);
      void _SplitLines(std::vector<BSPLine *> &Lines,std::vector<BSPLine *> &Front,std::vector<BSPLine *> &Back,
                            BSPLine *pLine,int *pnNumFront,int *pnNumBack,int *pnNumSplits,bool bProbe);
      BSPLine *_FindBestSplitter(std::vector<BSPLine *> &Lines);
      BSPLine *_CopyLine(BSPLine *pSrc);
      BSPPoly *_CopyPoly(BSPPoly *pDst,BSPPoly *pSrc);      
  };

}

#endif
