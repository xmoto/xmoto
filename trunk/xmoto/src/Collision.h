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

#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "VCommon.h"
#include "VApp.h"
#include "xmscene/Entity.h"

namespace vapp {

  class CollisionSystem;

	/*===========================================================================
	Structs
  ===========================================================================*/
  /* Line */
  struct Line {
    float x1,y1,x2,y2;
    float fGrip;
  };

  /* Entity */
  struct ColEntity {
    float x, y;
    Entity* id;
    /* in order to remove efficiently an entity from the grid,
       we need to know in which cell it is */
    /* if gridCell == -1, then it means that the entity is not in
       the level boundaries (moved out by a script for example) */
    int gridCell;
  };

  /* Grid cell */
  struct GridCell {
    std::vector<Line *> Lines;
    std::vector<ColEntity*> Entities;
  };

  /* Stats */
  struct CollisionSystemStats {
    int nGridWidth,nGridHeight;
    float fCellWidth,fCellHeight;
    float fPercentageOfEmptyCells;
    int nTotalLines;
  };

	/*===========================================================================
	Collision detection class
  ===========================================================================*/
  class CollisionSystem {
    public:
      CollisionSystem() {m_pGrid = NULL; m_bDebugFlag=false;}
      ~CollisionSystem() {reset();}
    
      /* Methods */
      void reset(void);
      void setDims(float fMinX,float fMinY,float fMaxX,float fMaxY);
      void defineLine(float x1,float y1,float x2,float y2, float grip);
      void addExternalDynamicLine(Line *pLine);
      
      bool checkLine(float x1,float y1,float x2,float y2);
      bool checkCircle(float x,float y,float r);
      bool checkBoxFast(float fMinX,float fMinY,float fMaxX,float fMaxY);
      bool checkCirclePath(float x1,float y1,float x2,float y2,float r);
      
      int collideLine(float x1,float y1,float x2,float y2,dContact *pContacts,int nMaxC);
      int collideCircle(float x,float y,float r,dContact *pContacts,int nMaxC);
      int collideCirclePath(float x1,float y1,float x2,float y2,float r,float *cx,float *cy,int nMaxC);
      
      void getStats(CollisionSystemStats *p);
      void setDebug(bool b) {m_bDebugFlag = b;}
      
      void clearDynamicTouched(void) {m_bDynamicTouched = false;}
      bool isDynamicTouched(void) {return m_bDynamicTouched;}

      /* Debug information, evil and public... only updated if the debug flag is specified */
      std::vector<Line *> m_CheckedLines;
      std::vector<Line *> m_CheckedLinesW;
      std::vector<Line> m_CheckedCells;
      std::vector<Line> m_CheckedCellsW;
      std::vector<Entity*> m_CheckedEntities;

      /* Adding zones and entities to the collision system */
      /* In order to use space partionning with them */
      void addEntity(Entity* id, float x, float y);
      void removeEntity(Entity* id);
      void moveEntity(Entity* id, float newX, float newY);
      std::vector<Entity*> getEntitiesNearPosition(float xmin, float ymin,
						   float xmax, float ymax);

    private:
      /* Data */
      float m_fMinX,m_fMinY,m_fMaxX,m_fMaxY;
      std::vector<Line *> m_Lines;      
      std::vector<Line *> m_ExternalDynamicLines; /* list NOT managed by this class */
      std::vector<ColEntity*> m_Entities;
      
      bool m_bDebugFlag;
      
      float m_fCellWidth,m_fCellHeight;
      int m_nGridWidth,m_nGridHeight;
      
      GridCell *m_pGrid;
      
      bool m_bDynamicTouched;
      
      /* Helpers */
      bool _CheckCircleAndLine(Line *pLine,float x,float y,float r);
      int _CollideCircleAndLine(Line *pLine,float x,float y,float r,dContact *pContacts,int nOldNumC,int nMaxC, float fGrip);
      void _SetWheelContactParams(dContact *pc,const Vector2f &Pos,const Vector2f &NormalT,double fDepth, float fGrip);
      double _CalculateDepth(const Vector2f &Cp,float Cr,Vector2f P);
      double _CalculateCircleLineDepth(const Vector2f &Cp,float Cr,Vector2f P1,Vector2f P2);
      int _AddContactToList(dContact *pContacts,int nNumContacts,dContact *pc,int nMaxContacts);

      ColEntity* _getEntity(Entity* id);
      ColEntity* _getAndRemoveEntity(Entity* id);
      void _removeEntityFromCell(ColEntity* pEnt);
      void _addEntityInCell(ColEntity* pEnt, float x, float y);
  };

}

#endif

