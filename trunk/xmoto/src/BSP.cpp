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
 *  Convex polygon generation using BSP trees.
 */
#include "Game.h"
#include "BSP.h"
#include "VFileIO.h"

namespace vapp {

  /*===========================================================================
  Add input line definition
  ===========================================================================*/
  void BSP::addLineDef(Vector2f P0,Vector2f P1) {
    BSPLine *pLine = new BSPLine;
    
    pLine->P0 = P0;
    pLine->P1 = P1;
    _ComputeNormal( pLine );
    
    m_Lines.push_back( pLine );
  }

  /*===========================================================================
  2D BSP class... creates convex hulls from a set of arbitrary edge definitions
  ===========================================================================*/
  std::vector<BSPPoly *> &BSP::compute(void) {
    /* Start by creating the root polygon - i.e. the quad that covers
       the entire region enclosed by the input linedefs */
    AABB GlobalBox;

    for(int i=0;i<m_Lines.size();i++) {
      GlobalBox.addPointToAABB2f(m_Lines[i]->P0);
      GlobalBox.addPointToAABB2f(m_Lines[i]->P1);
    }
    
    BSPPoly RootPoly;
    Vector2f GlobalMin = GlobalBox.getBMin();
    Vector2f GlobalMax = GlobalBox.getBMax();
    RootPoly.Vertices.push_back( new BSPVertex( Vector2f(GlobalMin.x,GlobalMin.y),Vector2f(0,0) ) ); 
    RootPoly.Vertices.push_back( new BSPVertex( Vector2f(GlobalMin.x,GlobalMax.y),Vector2f(0,0) ) ); 
    RootPoly.Vertices.push_back( new BSPVertex( Vector2f(GlobalMax.x,GlobalMax.y),Vector2f(0,0) ) ); 
    RootPoly.Vertices.push_back( new BSPVertex( Vector2f(GlobalMax.x,GlobalMin.y),Vector2f(0,0) ) ); 
    
    RootPoly.pTexture = NULL;
    
    /* Start the recursion */
    _Recurse(&RootPoly,m_Lines);
    
    /* Return polygon list */
    return m_Polys;
  }

  /*===========================================================================
  Recursive generation
  ===========================================================================*/
  void BSP::_Recurse(BSPPoly *pSubSpace,std::vector<BSPLine *> &Lines) {  
    /* Find best splitter */
    //printf("FIND BEST SPLITTER: %08x\n",&Lines);
    BSPLine *pBestSplitter = _FindBestSplitter(Lines);
    
    if(pBestSplitter == NULL) {
      /* We've found a final convex hull! :D  stop here -- add it to output
         (after cutting it of course) */
      BSPPoly *pPoly = _CopyPoly(new BSPPoly,pSubSpace);
        
      /* Cut the polygon by each lines */
      //for(int i=0;i<1;i++) { 
//      printf("HULL!\n");
      for(int i=0;i<Lines.size();i++) {
        /* Define splitting plane */
        BSPLine Splitter;
        Splitter.P0 = Lines[i]->P0;
        Splitter.P1 = Lines[i]->P1;
        Splitter.Normal = Lines[i]->Normal;
        
      /*  printf("SPLITTER: [%f,%f]->[%f,%f] n=%f %f\n",Splitter.P0.x,Splitter.P0.y
          ,Splitter.P1.x,Splitter.P1.y,Splitter.Normal.x,Splitter.Normal.y);*/
        
        /* Cut */
        BSPPoly *pTempPoly = new BSPPoly;
        _SplitPoly(pPoly,NULL,pTempPoly,&Splitter);
        delete pPoly;
        pPoly = pTempPoly;
      }
      
      if(pPoly->Vertices.size() > 0)
        m_Polys.push_back( pPoly );
      else {
      /*  printf("Lines: %d   (subspace size: %d)\n",Lines.size(),pSubSpace->Vertices.size());
        for(int i=0;i<Lines.size();i++)
          printf("    (%f,%f)  ->  (%f,%f)\n",Lines[i]->P0.x,Lines[i]->P0.y,Lines[i]->P1.x,Lines[i]->P1.y);*/
      
        delete pPoly;
        Log("** Warning ** : BSP::_Recurse() - empty final polygon ignored");
        
        m_nNumErrors++;
      }
    }
    else {
      /* Split the mess */
    //  printf("(subdividing)\n");
      std::vector<BSPLine *> Front,Back;      
      _SplitLines(Lines,Front,Back,pBestSplitter,NULL,NULL,NULL,false);
      
      /* Also split the convex subspace */
      BSPPoly FrontSpace,BackSpace;
      _SplitPoly(pSubSpace,&FrontSpace,&BackSpace,pBestSplitter);
      
      //if(FrontSpace.Vertices.empty()) {
      //  printf("FRONTSPACE EMPTY!\n");
      //  printf("  (subspace size=%d)\n",pSubSpace->Vertices.size());
      //  printf("  (subspace:\n");
      //  for(int i=0;i<pSubSpace->Vertices.size();i++) {
      //    printf("   [%f %f]\n",pSubSpace->Vertices[i]->P.x,pSubSpace->Vertices[i]->P.y);
      //  }
      //  printf("  )\n");
      //  printf("  (splitter: %f %f -> %f %f (of %d lines))\n",pBestSplitter->P0.x,pBestSplitter->P0.y,
      //                                          pBestSplitter->P1.x,pBestSplitter->P1.y,Lines.size());
      //  for(int i=0;i<Lines.size();i++) {
      //    printf("%f %f %f %f\n",Lines[i]->P0.x,Lines[i]->P0.y,Lines[i]->P1.x,Lines[i]->P1.y);
      //  }
      //}
      //else if(BackSpace.Vertices.empty()) {
      //  printf("******* BACKSPACE EMPTY! *******\n");
      //}
      
      /* Continue recursion */
      _Recurse(&FrontSpace,Front);
      _Recurse(&BackSpace,Back);            
      
      /* Clean up */
      for(int i=0;i<Front.size();i++)
        delete Front[i];
      for(int i=0;i<Back.size();i++)
        delete Back[i];
    }
  }

  /*===========================================================================
  Split polygon
  ===========================================================================*/
  void BSP::_SplitPoly(BSPPoly *pPoly,BSPPoly *pFront,BSPPoly *pBack,BSPLine *pLine) {
    /* Split the given convex polygon to produce two new convex polygons */
    enum SplitPolyRel {
      SPR_ON_PLANE,SPR_IN_FRONT,SPR_IN_BACK
    };

    /* Empty? */
    if(pPoly->Vertices.empty()) {
      Log("** Warning ** : BSP::_SplitPoly() - empty polygon encountered");
      m_nNumErrors++;
      return;
    }
    
    /* Look at each corner of the polygon -- how does each of them relate to
       the plane? */
    std::vector<int> Rels;
    int nNumInFront=0,nNumInBack=0,nNumOnPlane=0;
    
    for(int i=0;i<pPoly->Vertices.size();i++) {
      double d = pLine->Normal.dot(pLine->P0 - pPoly->Vertices[i]->P);
      //printf("%f ",d);

      if(fabs(d) < 0.00001f) {Rels.push_back( SPR_ON_PLANE ); nNumOnPlane++;}
      else if(d < 0.0f) {Rels.push_back( SPR_IN_BACK ); nNumInBack++;}
      else {Rels.push_back( SPR_IN_FRONT ); nNumInFront++;}
    }
    
        
//    printf("\n[front=%d back=%d onplane=%d\n\n",nNumInFront,nNumInBack,nNumOnPlane);
    
    /* Do we need a split, or can we draw a simple conclusion to this madness? */
    if(nNumInBack==0 && nNumInFront==0) {
      /* Everything is on the line */                 
      Log("** Warning ** : BSP::_SplitPoly() - polygon fully plane aligned");
      m_nNumErrors++;
      //printf("   %d verts :\n",pPoly->Vertices.size());
      //for(int i=0;i<pPoly->Vertices.size();i++) {
      //  printf("     [%f %f]\n",pPoly->Vertices[i]->P.x,pPoly->Vertices[i]->P.y);
      //}
    }
    else if(nNumInBack==0 && nNumInFront>0) {
      /* Polygon can be regarded as being in front */
      if(pFront != NULL)
        _CopyPoly(pFront,pPoly);
    }
    else if(nNumInBack>0 && nNumInFront==0) {
      /* Polygon can be regarded as being behind the line */
      if(pBack != NULL)
        _CopyPoly(pBack,pPoly);
    }
    else {
      /* We need to divide the polygon */
      for(int i=0;i<pPoly->Vertices.size();i++) {
        bool bSplit = false;
      
        /* Next vertex? */
        int j=i+1;
        if(j==pPoly->Vertices.size()) j=0;
      
        /* Which sides should we add this corner to? */
        if(Rels[i] == SPR_ON_PLANE) {
          /* Both */
          BSPVertex *pVertex;
          if(pFront) {
            pFront->Vertices.push_back(pVertex = new BSPVertex);
            pVertex->P = pPoly->Vertices[i]->P;
          }
          if(pBack) {
            pBack->Vertices.push_back(pVertex = new BSPVertex);
            pVertex->P = pPoly->Vertices[i]->P;
          }
        }
        else if(Rels[i] == SPR_IN_FRONT) {
          /* In front */
          BSPVertex *pVertex;
          if(pFront) {
            pFront->Vertices.push_back(pVertex = new BSPVertex);
            pVertex->P = pPoly->Vertices[i]->P;
          }
          
          if(Rels[j] == SPR_IN_BACK)
            bSplit = true;
        }
        else if(Rels[i] == SPR_IN_BACK) {
          /* In back */
          BSPVertex *pVertex;
          if(pBack) {
            pBack->Vertices.push_back(pVertex = new BSPVertex);
            pVertex->P = pPoly->Vertices[i]->P;
          }
          
          if(Rels[j] == SPR_IN_FRONT)
            bSplit = true;
        }
        
        /* Check split? */
        if(bSplit) {        
          /* Calculate */
          Vector2f v = pPoly->Vertices[j]->P - pPoly->Vertices[i]->P;
          float den = v.dot(pLine->Normal);
          if(den == 0.0f) { 
            /* This should REALLY not be the case... warning! */
            Log("** Warning ** : BSP::_SplitPoly() - impossible case (1)");
            m_nNumErrors++;
            
            /* Now it's best simply to ignore this */
            continue;
          }
          
          float t = -(pLine->Normal.dot(pPoly->Vertices[i]->P - pLine->P0)) / den;

          if(t>-0.0001f && t<1.0001f) {
            BSPVertex *pt = pPoly->Vertices[i];
            Vector2f Sect = pt->P + v*t;            
            
            /* We have now calculated the intersection point.
               Add it to both front/back */
            BSPVertex *pVertex;
            if(pFront) {
              pFront->Vertices.push_back(pVertex = new BSPVertex);
              pVertex->P = Sect;
            }
            if(pBack) {
              pBack->Vertices.push_back(pVertex = new BSPVertex);
              pVertex->P = Sect;
            }
          }            
        }
      }
    }
  }
  
  /*===========================================================================
  Split lines
  ===========================================================================*/
  void BSP::_SplitLines(std::vector<BSPLine *> &Lines,std::vector<BSPLine *> &Front,std::vector<BSPLine *> &Back,
                        BSPLine *pLine,int *pnNumFront,int *pnNumBack,int *pnNumSplits,bool bProbe) {
    enum SplitLineRel {
      SLR_ON_PLANE,SLR_IN_FRONT,SLR_IN_BACK
    };
                        
    /* Try splitting all the lines -- and collect a bunch of stats about it */
    for(int i=0;i<Lines.size();i++) {
      /* Look at this... determined the signed point-plane distance for each ends of the line to split */
      double d0 = pLine->Normal.dot(pLine->P0 - Lines[i]->P0);
      double d1 = pLine->Normal.dot(pLine->P0 - Lines[i]->P1);
      
      /* Now decide how it relates to the splitter */
      SplitLineRel r0,r1;

      if(fabs(d0) < 0.0001f) r0=SLR_ON_PLANE;
      else if(d0 < 0.0f) r0=SLR_IN_BACK;
      else r0=SLR_IN_FRONT;

      if(fabs(d1) < 0.0001f) r1=SLR_ON_PLANE;
      else if(d1 < 0.0f) r1=SLR_IN_BACK;
      else r1=SLR_IN_FRONT;
      
      /* If we are lucky we don't need to perform the split */
      if((r0==SLR_IN_FRONT && r1!=SLR_IN_BACK) ||
         (r1==SLR_IN_FRONT && r0!=SLR_IN_BACK)) {
        /* The entire line can be regarded as being IN FRONT */
        if(bProbe) {
          if(pnNumFront) *pnNumFront=*pnNumFront + 1;
        }
        else {
          Front.push_back( _CopyLine(Lines[i]) );
        }
      }
      else if((r0==SLR_IN_BACK && r1!=SLR_IN_FRONT) ||
              (r1==SLR_IN_BACK && r0!=SLR_IN_FRONT)) {
        /* The entire line can be regarded as being IN BACK */
        if(bProbe) {
          if(pnNumBack) *pnNumBack=*pnNumBack + 1;
        }
        else {
          Back.push_back( _CopyLine(Lines[i]) );
        }
      }
      else if(r0==SLR_ON_PLANE && r1==SLR_ON_PLANE) {
        /* The line is approximately on the plane... find out which way it faces */
        if(Lines[i]->Normal.almostEqual(pLine->Normal)) {
//          printf("ON PLANE!!  [%f %f]  [%f %f]\n",Lines[i]->Normal.x,Lines[i]->Normal.y,pLine->Normal.x,pLine->Normal.y);
          /* In back then */
          if(bProbe) {
            if(pnNumBack) *pnNumBack=*pnNumBack + 1;
          }
          else {
            Back.push_back( _CopyLine(Lines[i]) );
          }
        }
        else {
          /* In front then */
          if(bProbe) {
            if(pnNumFront) *pnNumFront=*pnNumFront + 1;
          }
          else {
            Front.push_back( _CopyLine(Lines[i]) );
          }
        }
      }
      else {
        /* We need to perform a split :( */
        if(bProbe) {
          if(pnNumFront) *pnNumFront=*pnNumFront + 1;
          if(pnNumBack) *pnNumBack=*pnNumBack + 1;
          if(pnNumSplits) *pnNumSplits=*pnNumSplits + 1;
        }
        else {
          Vector2f v = Lines[i]->P1 - Lines[i]->P0;
          float den = v.dot(pLine->Normal);
          if(den == 0.0f) { 
            /* This should REALLY not be the case... warning! */
            Log("** Warning ** : BSP::_SplitLines() - impossible case (1)");
            m_nNumErrors++;
            
            /* Now it's best simply to ignore this */
            continue;
          }
          
          double t = -(pLine->Normal.dot(Lines[i]->P0 - pLine->P0)) / den;
          
          if(t>-0.0001f && t<1.0001f) {
            Vector2f Sect = Lines[i]->P0 + v*t;            
            
            BSPLine *pNewLine;
            
            Front.push_back( pNewLine = new BSPLine );
            if(r0 == SLR_IN_FRONT) {
              pNewLine->P0 = Lines[i]->P0;
              pNewLine->P1 = Sect;
            }
            else {
              pNewLine->P0 = Sect;
              pNewLine->P1 = Lines[i]->P1;
            }
            _ComputeNormal(pNewLine);

            Back.push_back( pNewLine = new BSPLine );
            if(r0 == SLR_IN_FRONT) {
              pNewLine->P0 = Sect;
              pNewLine->P1 = Lines[i]->P1;
            }
            else {
              pNewLine->P0 = Lines[i]->P0;
              pNewLine->P1 = Sect;
            }            
            _ComputeNormal(pNewLine);
          }
          else {
            /* Another thing we should just ignore */
            Log("** Warning ** : BSP::_SplitLines() - impossible case (2)");
            m_nNumErrors++;
            
            continue;
          }         
        }
      }
    }                      
  }                        
  
  /*===========================================================================
  Find best splitter in line soup
  ===========================================================================*/
  BSPLine *BSP::_FindBestSplitter(std::vector<BSPLine *> &Lines) {
    std::vector<BSPLine *> Dummy1,Dummy2;
    
    int nBestScore = 10000000;
    BSPLine *pBest = NULL;
    int nBest = -1;
    
    /* Try splitting all lines with each of the lines */
//    printf("findbest\n");
    for(int i=0;i<Lines.size();i++) {
      int nNumFront=0,nNumBack=0,nNumSplits=0,nScore=0;      
      _SplitLines(Lines,Dummy1,Dummy2,Lines[i],&nNumFront,&nNumBack,&nNumSplits,true);
  //      printf("LINE%d:   f:%d  b:%d  s:%d\n",i,nNumFront,nNumBack,nNumSplits);
      
      /* Only qualify if both front and back is larger than 0 */
      if(nNumFront>0 && nNumBack>0) {      
        /* Compute the score (smaller the better) */
        nScore = abs(nNumBack - nNumFront) + nNumSplits*2;
        if(nScore < nBestScore) {
          pBest = Lines[i];
          nBest = i;
          nBestScore = nScore;
        }
      }
    }
//    printf("    best = %d\n",nBest);
    
    /* OK */
    return pBest;
  }

  /*===========================================================================
  Clean up
  ===========================================================================*/
  void BSP::_Cleanup(void) {
    /* Free input & output data */
    for(int i=0;i<m_Lines.size();i++)
      delete m_Lines[i];

    for(int i=0;i<m_Polys.size();i++)
      delete m_Polys[i];
  }

  /*===========================================================================
  Update line "normal"
  ===========================================================================*/
  void BSP::_ComputeNormal(BSPLine *pLine) {
    Vector2f v = pLine->P1 - pLine->P0;
    pLine->Normal.x = v.y;
    pLine->Normal.y = -v.x;
    pLine->Normal.normalize();
  }
   
  /*===========================================================================
  Update line axis-aligned bounding box
  ===========================================================================*/
  void BSP::_UpdateAABB(Vector2f &P,Vector2f &Min,Vector2f &Max) {
    if(P.x < Min.x) Min.x = P.x;
    if(P.x > Max.x) Max.x = P.x;
    if(P.y < Min.y) Min.y = P.y;
    if(P.y > Max.y) Max.y = P.y;
  }
  
  /*===========================================================================
  Update line axis-aligned bounding box
  ===========================================================================*/
  BSPPoly::~BSPPoly() {
    /* Voilala -- BSPPolys can clean up after themselves! */
    for(int i=0;i<Vertices.size();i++)
      delete Vertices[i];
  }
  
  /*===========================================================================
  Copy a BSP line
  ===========================================================================*/
  BSPLine *BSP::_CopyLine(BSPLine *pSrc) {
    BSPLine *pDst = new BSPLine;
    pDst->Normal = pSrc->Normal;
    pDst->P0 = pSrc->P0;
    pDst->P1 = pSrc->P1;
    return pDst;
  }

  /*===========================================================================
  Copy a BSP polygon
  ===========================================================================*/
  BSPPoly *BSP::_CopyPoly(BSPPoly *pDst,BSPPoly *pSrc) {
    for(int i=0;i<pSrc->Vertices.size();i++)
      pDst->Vertices.push_back( new BSPVertex( pSrc->Vertices[i]->P,pSrc->Vertices[i]->T ) );       
    
    pDst->pTexture = pSrc->pTexture;
    
    return pDst;
  }
}

