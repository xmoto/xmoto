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
 *  Collision detection.
 */
#include "Collision.h"
#include "PhysSettings.h"

/* 
   Prior to version 0.1.11, the far largest time sink in the game was the 
   collision detection. Yup, it was actually just brute force search for 
   collisions! :-O
   Additionally there also was some problems with players flying through 
   blocks due to high speed.
   This new collision system does the following:
   
    * Provides a general (neater) collision detection interface.
    * Collision is not only done on single sample points, but on a line between
      last sample position and the current. (no falling through the ground)
    * Static level geometry is stored in grid-structure for faster access. 
*/

namespace vapp {

  #define CD_MIN(a,b)             ((a)>(b)?(b):(a))
  #define CD_MAX(a,b)             ((a)<(b)?(b):(a))
  
  #define CD_MIN_CELL_SIZE        2.0f
  #define CD_MAX_GRID_SIZE        16
  #define CD_EPSILON              0.01f

  /*===========================================================================
  Reset collision system
  ===========================================================================*/
  void CollisionSystem::reset(void) {
    /* Free everything */
    if(m_pGrid != NULL) {
      delete [] m_pGrid;
      m_pGrid = NULL;
    }
    
    for(int i=0;i<m_Lines.size();i++) {
      delete m_Lines[i];
    }
    m_Lines.clear();
    
    m_ExternalDynamicLines.clear();
  }
 
  /*===========================================================================
  Add external dynamic line to the system
  ===========================================================================*/
  void CollisionSystem::addExternalDynamicLine(Line *pLine) {
    m_ExternalDynamicLines.push_back(pLine);
  }
 
  /*===========================================================================
  Set dimensions of system
  ===========================================================================*/
  void CollisionSystem::setDims(float fMinX,float fMinY,float fMaxX,float fMaxY) {
    /* Find suitable grid properties - first horizontal */
    int nTryGridWidth = CD_MAX_GRID_SIZE;
    do {
      float fCellWidth = (fMaxX - fMinX) / (float)nTryGridWidth;
      if(fCellWidth >= CD_MIN_CELL_SIZE) {
        m_fCellWidth = fCellWidth;
        m_nGridWidth = nTryGridWidth;
        break;
      }
      nTryGridWidth--;
    } while(nTryGridWidth > 0);
    
    if(nTryGridWidth == 0) {
      m_fCellWidth = fMaxX - fMinX;
      m_nGridWidth = 1;
    }
    
    /* Then vertical */
    int nTryGridHeight = CD_MAX_GRID_SIZE;
    do {
      float fCellHeight = (fMaxY - fMinY) / (float)nTryGridHeight;
      if(fCellHeight >= CD_MIN_CELL_SIZE) {
        m_fCellHeight = fCellHeight;
        m_nGridHeight = nTryGridHeight;
        break;
      }
      nTryGridHeight--;
    } while(nTryGridHeight > 0);
    
    if(nTryGridHeight == 0) {
      m_fCellHeight = fMaxY - fMinY;
      m_nGridHeight = 1;
    }
    
    /* Set bounding box */
    m_fMinX = fMinX;
    m_fMinY = fMinY;
    m_fMaxX = fMaxX;
    m_fMaxY = fMaxY;
    
    /* Allocate grid structure */
    m_pGrid = new GridCell[m_nGridWidth * m_nGridHeight];
    
    //printf("%dx%d grid width %.2fx%.2f cells\n",
    //       m_nGridWidth,m_nGridHeight,m_fCellWidth,m_fCellHeight);
  }
 
  /*===========================================================================
  Add blocking line to collision system
  ===========================================================================*/
  void CollisionSystem::defineLine(float x1,float y1,float x2,float y2) {
    /* Define bounding box of line */
    float fLineMinX = CD_MIN(x1,x2);
    float fLineMinY = CD_MIN(y1,y2);
    float fLineMaxX = CD_MAX(x1,x2);
    float fLineMaxY = CD_MAX(y1,y2);        
        
    /* Add line */
    Line *pNewLine = new Line;
    pNewLine->x1 = x1;
    pNewLine->y1 = y1;
    pNewLine->x2 = x2;
    pNewLine->y2 = y2;
    m_Lines.push_back(pNewLine);   
    
    /* Calculate cell coordinates */
    int nMinCX = (int)floor(((fLineMinX - m_fMinX - CD_EPSILON) * (float)m_nGridWidth) / (m_fMaxX - m_fMinX));
    int nMinCY = (int)floor(((fLineMinY - m_fMinY - CD_EPSILON) * (float)m_nGridHeight) / (m_fMaxY - m_fMinY));
    int nMaxCX = (int)floor(((fLineMaxX - m_fMinX + CD_EPSILON) * (float)m_nGridWidth) / (m_fMaxX - m_fMinX));
    int nMaxCY = (int)floor(((fLineMaxY - m_fMinY + CD_EPSILON) * (float)m_nGridHeight) / (m_fMaxY - m_fMinY));
    
    if(nMinCX < 0) nMinCX = 0;
    if(nMinCY < 0) nMinCY = 0;
    if(nMaxCX > m_nGridWidth-1) nMaxCX = m_nGridWidth-1;
    if(nMaxCY > m_nGridHeight-1) nMaxCY = m_nGridHeight-1;
    
    /* Add line to touched cells */
    for(int cx=nMinCX;cx<=nMaxCX;cx++) {
      for(int cy=nMinCY;cy<=nMaxCY;cy++) {
        int i = cx + cy*m_nGridWidth;
        
        m_pGrid[i].Lines.push_back(pNewLine);        
      }
    }
    
    /* TODO: instead of just adding line to all cells which are touched 
             box-box wise, do a more precise touch-check between the line and
             the cell box */
  }
  
  /*===========================================================================
  Boolean check of collision between line and system
  ===========================================================================*/
  bool CollisionSystem::checkLine(float x1,float y1,float x2,float y2) {
    return false;
  }
  
  /*===========================================================================
  Boolean check of collision between circle and system
  ===========================================================================*/
  bool CollisionSystem::_CheckCircleAndLine(Line *pLine,float x,float y,float r) {
    /* Is circle "behind" the line? */
    float vx = pLine->x2 - pLine->x1;
    float vy = pLine->y2 - pLine->y1;
    float enx = -vy;
    float eny = vx;
    if(enx*x + eny*y < enx*pLine->x1 + eny*pLine->y1) {
      /* Yes it is, can't touch */
      return false;
    }          

    /* Too small? */
    if(fabs(vx) < 0.0001f && fabs(vy) < 0.0001f) {
      return false;
    }
    
    /* Is line endings inside the circle? */
    float dx1 = pLine->x1 - x;
    float dy1 = pLine->y1 - y;
    if(dx1*dx1 + dy1*dy1 <= r*r) {
      /* We have a touch! */
      return true;
    }

    float dx2 = pLine->x2 - x;
    float dy2 = pLine->y2 - y;
    if(dx2*dx2 + dy2*dy2 <= r*r) {
      /* We have a touch! */
      return true;
    }
              
    /* Final check */		  
		Vector2f T1,T2;
		int n = intersectLineCircle2f(Vector2f(x,y),r,Vector2f(pLine->x1,pLine->y1),
		                              Vector2f(pLine->x2,pLine->y2),T1,T2);
    if(n>0) return true;		                                    
    return false;
  }

  bool CollisionSystem::checkCircle(float x,float y,float r) {
    /* Calculate bounding box of circle */
    float fMinX = x - r;
    float fMinY = y - r;
    float fMaxX = x + r;
    float fMaxY = y + r;
  
    /* Calculate cell coordinates */
    int nMinCX = (int)floor(((fMinX - m_fMinX - CD_EPSILON) * (float)m_nGridWidth) / (m_fMaxX - m_fMinX));
    int nMinCY = (int)floor(((fMinY - m_fMinY - CD_EPSILON) * (float)m_nGridHeight) / (m_fMaxY - m_fMinY));
    int nMaxCX = (int)floor(((fMaxX - m_fMinX + CD_EPSILON) * (float)m_nGridWidth) / (m_fMaxX - m_fMinX));
    int nMaxCY = (int)floor(((fMaxY - m_fMinY + CD_EPSILON) * (float)m_nGridHeight) / (m_fMaxY - m_fMinY));
    
    if(nMinCX < 0) nMinCX = 0;
    if(nMinCY < 0) nMinCY = 0;
    if(nMaxCX > m_nGridWidth-1) nMaxCX = m_nGridWidth-1;
    if(nMaxCY > m_nGridHeight-1) nMaxCY = m_nGridHeight-1;        
    
    if(m_bDebugFlag) {
      m_CheckedLines.clear();
      m_CheckedCells.clear();
    }
    
    /* Check all external dynamic lines first... */
    for(int k=0;k<m_ExternalDynamicLines.size();k++) {
      if(_CheckCircleAndLine(m_ExternalDynamicLines[k],x,y,r))
        return true;
    }    
    
    /* For each cell we might have touched something in... */
    for(int cx=nMinCX;cx<=nMaxCX;cx++) {
      for(int cy=nMinCY;cy<=nMaxCY;cy++) {
        int i = cx + cy*m_nGridWidth;
        
        if(m_bDebugFlag) {
          Line CellBox;
          CellBox.x1 = m_fMinX + m_fCellWidth * (float)cx;
          CellBox.y1 = m_fMinY + m_fCellHeight * (float)cy;
          CellBox.x2 = m_fMinX + m_fCellWidth * (float)(cx+1);
          CellBox.y2 = m_fMinY + m_fCellHeight * (float)(cy+1);
          m_CheckedCells.push_back(CellBox);
        }
        
        /* Empty? That would be nice */
        if(m_pGrid[i].Lines.empty()) continue;
        
        /* TODO: currently we will probably check the same lines several times each... AVOID THAT! */
        
        /* Check all lines in cell */
        for(int j=0;j<m_pGrid[i].Lines.size();j++) {  
          if(m_bDebugFlag)
            m_CheckedLines.push_back(m_pGrid[i].Lines[j]);
            
          if(_CheckCircleAndLine(m_pGrid[i].Lines[j],x,y,r))
            return true;
        }
      }
    }

    /* Woo, nothing touched */
    return false;
  }
  
  /*===========================================================================
  Boolean check of collision between box and system (fast, not precise)
  ===========================================================================*/
  bool CollisionSystem::checkBoxFast(float fMinX,float fMinY,float fMaxX,float fMaxY) {
    /* Calculate cell coordinates */
    int nMinCX = (int)floor(((fMinX - m_fMinX - CD_EPSILON) * (float)m_nGridWidth) / (m_fMaxX - m_fMinX));
    int nMinCY = (int)floor(((fMinY - m_fMinY - CD_EPSILON) * (float)m_nGridHeight) / (m_fMaxY - m_fMinY));
    int nMaxCX = (int)floor(((fMaxX - m_fMinX + CD_EPSILON) * (float)m_nGridWidth) / (m_fMaxX - m_fMinX));
    int nMaxCY = (int)floor(((fMaxY - m_fMinY + CD_EPSILON) * (float)m_nGridHeight) / (m_fMaxY - m_fMinY));
    
    if(nMinCX < 0) nMinCX = 0;
    if(nMinCY < 0) nMinCY = 0;
    if(nMaxCX > m_nGridWidth-1) nMaxCX = m_nGridWidth-1;
    if(nMaxCY > m_nGridHeight-1) nMaxCY = m_nGridHeight-1;
    
    /* Check touched cells for emptyness */
    for(int cx=nMinCX;cx<=nMaxCX;cx++) {
      for(int cy=nMinCY;cy<=nMaxCY;cy++) {
        int i = cx + cy*m_nGridWidth;
        
        if(m_pGrid[i].Lines.empty()) return true; /* damn, we might touch something */
      }
    }

    /* None of the touched cells contain any geometry */
    return false;
  }
      
  /*===========================================================================
  Boolean check of collision between circle-path and system
  ===========================================================================*/
  bool CollisionSystem::checkCirclePath(float x1,float y1,float x2,float y2,float r) {
    return false;
  }
      
  /*===========================================================================
  Calculate collision between line and system
  ===========================================================================*/
  int CollisionSystem::collideLine(float x1,float y1,float x2,float y2,dContact *pContacts,int nMaxC) {
    int nNumC = 0;
  
    /* Calculate bounding box of line */
    float fMinX = CD_MIN(x1,x2);
    float fMinY = CD_MIN(y1,y2);
    float fMaxX = CD_MAX(x1,x2);
    float fMaxY = CD_MAX(y1,y2);
  
    /* Calculate cell coordinates */
    int nMinCX = (int)floor(((fMinX - m_fMinX - CD_EPSILON) * (float)m_nGridWidth) / (m_fMaxX - m_fMinX));
    int nMinCY = (int)floor(((fMinY - m_fMinY - CD_EPSILON) * (float)m_nGridHeight) / (m_fMaxY - m_fMinY));
    int nMaxCX = (int)floor(((fMaxX - m_fMinX + CD_EPSILON) * (float)m_nGridWidth) / (m_fMaxX - m_fMinX));
    int nMaxCY = (int)floor(((fMaxY - m_fMinY + CD_EPSILON) * (float)m_nGridHeight) / (m_fMaxY - m_fMinY));
    
    if(nMinCX < 0) nMinCX = 0;
    if(nMinCY < 0) nMinCY = 0;
    if(nMaxCX > m_nGridWidth-1) nMaxCX = m_nGridWidth-1;
    if(nMaxCY > m_nGridHeight-1) nMaxCY = m_nGridHeight-1;        

    /* First check dynamic lines */
    for(int k=0;k<m_ExternalDynamicLines.size();k++) {
      /* <TODO: avoid code duplication here> */
      
      /* Is the beginning "behind" the line? */
      float vx = m_ExternalDynamicLines[k]->x2 - m_ExternalDynamicLines[k]->x1;
      float vy = m_ExternalDynamicLines[k]->y2 - m_ExternalDynamicLines[k]->y1;
      float enx = -vy;
      float eny = vx;
      if(enx*x1 + eny*y1 < enx*m_ExternalDynamicLines[k]->x1 + eny*m_ExternalDynamicLines[k]->y1) {
        /* Yes it is, can't touch */
        continue;
      }

      /* Too small? */
      if(fabs(vx) < 0.0001f && fabs(vy) < 0.0001f) {
        continue;
      }
      
      /* Try calculating intersection point */
      Vector2f T;
      int n = intersectLineLine2f(Vector2f(x1,y1),Vector2f(x2,y2),
                                  Vector2f(m_ExternalDynamicLines[k]->x1,m_ExternalDynamicLines[k]->y1),
                                  Vector2f(m_ExternalDynamicLines[k]->x2,m_ExternalDynamicLines[k]->y2),T);
      if(n > 0) {
        dContact c;
        Vector2f W = Vector2f(vx,vy);
        W.normalize();

        _SetWheelContactParams(&c,T,W,0.0f);                                           
        int nOldC = nNumC;
        nNumC = _AddContactToList(pContacts,nNumC,&c,nMaxC);                         
        if(nNumC != nOldC) m_bDynamicTouched = true;
      }                                      

      /* </TODO: avoid code duplication here> */
    }              

    /* For each cell we might have touched something in... */
    for(int cx=nMinCX;cx<=nMaxCX;cx++) {
      for(int cy=nMinCY;cy<=nMaxCY;cy++) {
        int i = cx + cy*m_nGridWidth;
            
        /* Empty? That would be nice */
        if(m_pGrid[i].Lines.empty()) continue;
        
        /* TODO: currently we will probably check the same lines several times each... AVOID THAT! */
        
        /* Check all lines in cell */
        for(int j=0;j<m_pGrid[i].Lines.size();j++) {
          /* Is the beginning "behind" the line? */
          float vx = m_pGrid[i].Lines[j]->x2 - m_pGrid[i].Lines[j]->x1;
          float vy = m_pGrid[i].Lines[j]->y2 - m_pGrid[i].Lines[j]->y1;
          float enx = -vy;
          float eny = vx;
          if(enx*x1 + eny*y1 < enx*m_pGrid[i].Lines[j]->x1 + eny*m_pGrid[i].Lines[j]->y1) {
            /* Yes it is, can't touch */
            continue;
          }

          /* Too small? */
          if(fabs(vx) < 0.0001f && fabs(vy) < 0.0001f) {
            continue;
          }
          
          /* Try calculating intersection point */
          Vector2f T;
          int n = intersectLineLine2f(Vector2f(x1,y1),Vector2f(x2,y2),
                                      Vector2f(m_pGrid[i].Lines[j]->x1,m_pGrid[i].Lines[j]->y1),
                                      Vector2f(m_pGrid[i].Lines[j]->x2,m_pGrid[i].Lines[j]->y2),T);
          if(n > 0) {
            dContact c;
            Vector2f W = Vector2f(vx,vy);
            W.normalize();

            _SetWheelContactParams(&c,T,W,0.0f);                                   
            nNumC = _AddContactToList(pContacts,nNumC,&c,nMaxC);                 
          }                                      
        }          
      }
    }

    /* Woo, nothing touched */
    return nNumC;
  }

  /*===========================================================================
  Calculate precise intersections between circle and geometry, if any
  ===========================================================================*/
  int CollisionSystem::_CollideCircleAndLine(Line *pLine,float x,float y,float r,dContact *pContacts,int nOldNumC,int nMaxC) {
    int nNumC = nOldNumC;
  
    /* Is circle "behind" the line? */
    float vx = pLine->x2 - pLine->x1;
    float vy = pLine->y2 - pLine->y1;
    float enx = -vy;
    float eny = vx;
    if(enx*x + eny*y < enx*pLine->x1 + eny*pLine->y1) {
      /* Yes it is, can't touch */
      return nNumC;
    }

    /* Too small? */
    if(fabs(vx) < 0.0001f && fabs(vy) < 0.0001f) {
      return nNumC;
    }

    if(m_bDebugFlag)
      m_CheckedLinesW.push_back(pLine);

    /* Is line endings inside the circle? */
    float dx1 = pLine->x1 - x;
    float dy1 = pLine->y1 - y;
    if(sqrt(dx1*dx1 + dy1*dy1) <= r + 0.0001f) {
      /* We have a touch! */
      dContact c;
      Vector2f W = Vector2f(-dx1,-dy1);
      W.normalize();
      
      //W.x =enx;
      //W.y = eny;
      //W.normalize();

//      float fDepth = _CalculateCircleLineDepth(Vector2f(x,y),r,Vector2f(pLine->x1,pLine->y1),Vector2f(pLine->x2,pLine->y2));
      double fDepth = _CalculateDepth(Vector2f(x,y),r,Vector2f(pLine->x1,pLine->y1));
      _SetWheelContactParams(&c,Vector2f(pLine->x1,pLine->y1),
                              W,fDepth);                                   
      nNumC = _AddContactToList(pContacts,nNumC,&c,nMaxC);            
      //return nNumC;
    }

    float dx2 = pLine->x2 - x;
    float dy2 = pLine->y2 - y;
    if(sqrt(dx2*dx2 + dy2*dy2) <= r + 0.0001f) {
      /* We have a touch! */            
      dContact c;
      Vector2f W = Vector2f(-dx2,-dy2);
      W.normalize();

      //W.x =enx;
      //W.y = eny;
      //W.normalize();

//      float fDepth = _CalculateCircleLineDepth(Vector2f(x,y),r,Vector2f(pLine->x1,pLine->y1),Vector2f(pLine->x2,pLine->y2));
      double fDepth = _CalculateDepth(Vector2f(x,y),r,Vector2f(pLine->x2,pLine->y2));
      _SetWheelContactParams(&c,Vector2f(pLine->x2,pLine->y2),
                              W,fDepth);                                   
      nNumC = _AddContactToList(pContacts,nNumC,&c,nMaxC);            
      //return nNumC;
    }

    /* Calculate intersection */
		Vector2f T1,T2;
		int n = intersectLineCircle2f(Vector2f(x,y),r,Vector2f(pLine->x1,pLine->y1),
		                              Vector2f(pLine->x2,pLine->y2),T1,T2);
    if(n>0) {
      dContact c;
      Vector2f W = Vector2f(enx,eny);
      W.normalize();
      
      //_SetWheelContactParams(&c,T1,W,_CalculateDepth(Vector2f(x,y),r,T1));                                   
      double fDepth = _CalculateCircleLineDepth(Vector2f(x,y),r,Vector2f(pLine->x1,pLine->y1),Vector2f(pLine->x2,pLine->y2));
      //fDepth = 0.0f;
      //fDepth *= 0.2;
      //static int xxx = 0,yyy = 0;		                             
      //xxx++;    
      //if((xxx % 200) == 0) 
      //  printf("%f\n",fDepth);
        
      _SetWheelContactParams(&c,T1,W,fDepth); 
      nNumC = _AddContactToList(pContacts,nNumC,&c,nMaxC);
        
      if(n>1) {
        _SetWheelContactParams(&c,T2,W,fDepth); 
//        _SetWheelContactParams(&c,T2,W,_CalculateDepth(Vector2f(x,y),r,T2));                                   
        nNumC = _AddContactToList(pContacts,nNumC,&c,nMaxC);
      }
    }
    
    return nNumC;
  }
  
  int CollisionSystem::collideCircle(float x,float y,float r,dContact *pContacts,int nMaxC) {
    int nNumC = 0;
  
    /* Calculate bounding box of circle */
    float fMinX = x - r;
    float fMinY = y - r;
    float fMaxX = x + r;
    float fMaxY = y + r;
  
    /* Calculate cell coordinates */
    int nMinCX = (int)floor(((fMinX - m_fMinX - CD_EPSILON) * (float)m_nGridWidth) / (m_fMaxX - m_fMinX));
    int nMinCY = (int)floor(((fMinY - m_fMinY - CD_EPSILON) * (float)m_nGridHeight) / (m_fMaxY - m_fMinY));
    int nMaxCX = (int)floor(((fMaxX - m_fMinX + CD_EPSILON) * (float)m_nGridWidth) / (m_fMaxX - m_fMinX));
    int nMaxCY = (int)floor(((fMaxY - m_fMinY + CD_EPSILON) * (float)m_nGridHeight) / (m_fMaxY - m_fMinY));
    
    if(nMinCX < 0) nMinCX = 0;
    if(nMinCY < 0) nMinCY = 0;
    if(nMaxCX > m_nGridWidth-1) nMaxCX = m_nGridWidth-1;
    if(nMaxCY > m_nGridHeight-1) nMaxCY = m_nGridHeight-1;        

    if(m_bDebugFlag) {
      m_CheckedLinesW.clear();
      m_CheckedCellsW.clear();
    }
    
    /* Collide against dynamic lines */
    for(int k=0;k<m_ExternalDynamicLines.size();k++) {
      int nOldC = nNumC;
      nNumC = _CollideCircleAndLine(m_ExternalDynamicLines[k],x,y,r,pContacts,nNumC,nMaxC);
//      printf("<%d  %d  %d>\n",nNumC,nOldC,m_ExternalDynamicLines.size());
      if(nOldC != nNumC)
        m_bDynamicTouched = true;
    }
    
    /* For each cell we might have touched something in... */
    for(int cx=nMinCX;cx<=nMaxCX;cx++) {
      for(int cy=nMinCY;cy<=nMaxCY;cy++) {
        int i = cx + cy*m_nGridWidth;

        if(m_bDebugFlag) {
          Line CellBox;
          CellBox.x1 = m_fMinX + m_fCellWidth * (float)cx;
          CellBox.y1 = m_fMinY + m_fCellHeight * (float)cy;
          CellBox.x2 = m_fMinX + m_fCellWidth * (float)(cx+1);
          CellBox.y2 = m_fMinY + m_fCellHeight * (float)(cy+1);
          m_CheckedCellsW.push_back(CellBox);
        }
            
        /* Empty? That would be nice */
        if(m_pGrid[i].Lines.empty()) continue;
        
        /* TODO: currently we will probably check the same lines several times each... AVOID THAT! */
        
        /* Check all lines in cell */
        for(int j=0;j<m_pGrid[i].Lines.size();j++) {
          nNumC = _CollideCircleAndLine(m_pGrid[i].Lines[j],x,y,r,pContacts,nNumC,nMaxC);
        }          
      }
    }

    /* Woo, nothing touched */
    return nNumC;
  }
  
  /*===========================================================================
  Calculate precise intersections between circle-path and geometry, if any
  ===========================================================================*/
  int CollisionSystem::collideCirclePath(float x1,float y1,float x2,float y2,float r,float *cx,float *cy,int nMaxC) {
    return 0;
  }

  /*===========================================================================
  Collection of stats
  ===========================================================================*/
  void CollisionSystem::getStats(CollisionSystemStats *p) {
    p->nTotalLines = m_Lines.size();
    p->nGridWidth = m_nGridWidth;
    p->nGridHeight = m_nGridHeight;
    p->fCellWidth = m_fCellWidth;
    p->fCellHeight = m_fCellHeight;
    
    int nEmpty = 0;
    for(int i=0;i<m_nGridWidth*m_nGridHeight;i++) {
      if(m_pGrid[i].Lines.empty()) nEmpty++;
    }
    
    p->fPercentageOfEmptyCells = (100.0f * (float)nEmpty) / (float)(m_nGridWidth*m_nGridHeight);
  }

  /*===========================================================================
  Helpers 
  ===========================================================================*/
  void CollisionSystem::_SetWheelContactParams(dContact *pc,const Vector2f &Pos,const Vector2f &NormalT,double fDepth) {
    memset(pc,0,sizeof(dContact));
    Vector2f Normal = NormalT;
    Normal.normalize();
    if(fDepth < 0.01f) fDepth = 0.0f;
    pc->geom.depth = fDepth;
    
    //printf("%f \n",pc->geom.depth);
    pc->geom.normal[0] = Normal.x;
    pc->geom.normal[1] = Normal.y;
    pc->geom.pos[0] = Pos.x;
    pc->geom.pos[1] = Pos.y;
    //pc->surface.mu = dInfinity;
    pc->surface.mu = PHYS_WHEEL_GRIP;
    //pc->surface.mu = 0.9; //.7; //8;
    //pc->surface.bounce = 0.3;
    pc->surface.soft_erp = PHYS_WHEEL_ERP;     
    pc->surface.soft_cfm = PHYS_WHEEL_CFM; 
    pc->surface.mode = dContactApprox1_1 | dContactSlip1;
  }  

  double CollisionSystem::_CalculateDepth(const Vector2f &Cp,float Cr,Vector2f P) {
    double fDist = (Cp - P).length();
    double fDepth = Cr - fDist;
    if(fDepth<0.0f) fDepth=0.0f;
    return fDepth;
  }

  double CollisionSystem::_CalculateCircleLineDepth(const Vector2f &Cp,float Cr,Vector2f P1,Vector2f P2) {
    Vector2f N;
    N.x = P2.y - P1.y;
    N.y = -(P2.x - P1.x);
    Vector2f R = P1 - Cp;
        
    if(N.length() > 0.0f) {     
      N.normalize(); 
      double f = R.x * N.x + R.y * N.y;         
      //printf("[%f]\n",f);   
      return Cr - fabs(f);
    }
    return 0.0f;
  }
  
  int CollisionSystem::_AddContactToList(dContact *pContacts,int nNumContacts,dContact *pc,int nMaxContacts) {
    int i;    
    if(nNumContacts == nMaxContacts) return nNumContacts;
    
    for(int i=0;i<nNumContacts;i++) {
      if(fabs(pContacts[i].geom.pos[0] - pc->geom.pos[0]) < 0.1f &&
         fabs(pContacts[i].geom.pos[1] - pc->geom.pos[1]) < 0.1f) return nNumContacts;
    }
    //printf(" [%f %f]  [%f %f]\n",pc->geom.pos[0],pc->geom.pos[1],
      //pc->geom.normal[0],pc->geom.normal[1]);
    memcpy(&pContacts[nNumContacts],pc,sizeof(dContact));
    nNumContacts++;
    return nNumContacts;
  }
  
};
