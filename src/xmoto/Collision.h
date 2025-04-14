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

#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "common/VCommon.h"
#include "helpers/VMath.h"
#include <ode/ode.h>
#include <vector>

class Block;
class Entity;
class Zone;
class CollisionSystem;
class PhysicsSettings;
struct GridCell;

/*===========================================================================
Structs
===========================================================================*/
/* Line */
struct Line {
  float x1, y1, x2, y2;
  float fGrip;
};

template<class T>
struct ColElement {
  T *id;
  /* in order to remove efficiently an element from the grid,
     we need to know in which cells it is */
  /* if gridCells.size() == 0, then it means that the element is not in
     the level boundaries (moved out by a script for example) */
  std::vector<int> gridCells;
  /* as an element can be in more than one cell,
     we need to tell if an element has already be visited
  */
  int curCheck;
};

template<class T>
class ElementHandler {
public:
  typedef struct {
    std::vector<struct ColElement<T> *> ColElements;
  } GridCell;

  /* The element must have a method getAABB() */
  struct ColElement<T> *addElement(T *id);
  void removeElement(T *id);
  void moveElement(T *id);
  void moveElement(struct ColElement<T> *pColElem);
  std::vector<T *> &getElementsNearPosition(AABB &BBox);

  ElementHandler() {
    m_pGrid = NULL;
    m_bDebugFlag = false;
    m_curCheck = 0;
    reset();
  }
  void reset();
  void setDims(Vector2f min,
               Vector2f max,
               int gridWidth,
               int gridHeight,
               float Xoffset = 1.0,
               float Yoffset = 1.0);

  void setDebug(bool b) { m_bDebugFlag = b; }
  std::vector<T *> &getCheckedElements() { return m_CheckedElements; }

private:
  std::vector<struct ColElement<T> *> m_ColElements;
  /* level dimensions */
  Vector2f m_min, m_max;
  /* grid dimensions */
  int m_gridWidth, m_gridHeight;
  /* grid for the elements */
  GridCell *m_pGrid;

  /* The current checking pass */
  int m_curCheck;

  bool m_bDebugFlag;
  std::vector<T *> m_CheckedElements;

  // the vector returned by getElementsNearPosition
  std::vector<T *> m_returnedElements;

  /* helpers */
  struct ColElement<T> *_getColElement(T *id);
  void _addColElementInCells(struct ColElement<T> *pColElem);
  struct ColElement<T> *_getAndRemoveColElement(T *id);
  void _removeColElementFromCells(struct ColElement<T> *pColElem);

  // precalculated values
  float m_widthDivisor;
  float m_heightDivisor;
};

/* Grid cell */
struct GridCell {
  std::vector<Line *> Lines;
};

/* Stats */
struct CollisionSystemStats {
  int nGridWidth, nGridHeight;
  float fCellWidth, fCellHeight;
  float fPercentageOfEmptyCells;
  int nTotalLines;
};

/*===========================================================================
Collision detection class
===========================================================================*/
class CollisionSystem {
public:
  CollisionSystem();
  ~CollisionSystem();

  /* Methods */
  void reset(void);
  void setDims(float fMinX,
               float fMinY,
               float fMaxX,
               float fMaxY,
               unsigned int numberBackgroundLayers,
               std::vector<Vector2f> &layerOffsets);
  void defineLine(float x1, float y1, float x2, float y2, float grip);

  bool checkLine(float x1, float y1, float x2, float y2);
  bool checkCircle(float x, float y, float r);
  bool checkBoxFast(float fMinX, float fMinY, float fMaxX, float fMaxY);

  int collideLine(float x1,
                  float y1,
                  float x2,
                  float y2,
                  dContact *pContacts,
                  int nMaxC,
                  PhysicsSettings *i_physicsSettings);
  int collideCircle(float x,
                    float y,
                    float r,
                    dContact *pContacts,
                    int nMaxC,
                    PhysicsSettings *i_physicsSettings);
  int collideCirclePath(float x1,
                        float y1,
                        float x2,
                        float y2,
                        float r,
                        float *cx,
                        float *cy,
                        int nMaxC);

  void getStats(CollisionSystemStats *p);
  void setDebug(bool b) {
    m_bDebugFlag = b;
    m_entitiesHandler.setDebug(b);
  }

  /* check this to see if we can remove this two functions */
  void clearDynamicTouched(void) { m_bDynamicTouched = false; }
  bool isDynamicTouched(void) { return m_bDynamicTouched; }

  /* Debug information, evil and public... only updated if the debug flag is
   * specified */
  std::vector<Line *> m_CheckedLines;
  std::vector<Line *> m_CheckedLinesW;
  std::vector<Line> m_CheckedCells;
  std::vector<Line> m_CheckedCellsW;
  std::vector<Entity *> &getCheckedEntities() {
    return m_entitiesHandler.getCheckedElements();
  }

  /* Adding zones, dyn blocks and entities to the collision system */
  /* In order to use space partionning with them */
  void addEntity(Entity *id);
  void removeEntity(Entity *id);
  void moveEntity(Entity *id);
  std::vector<Entity *> &getEntitiesNearPosition(AABB &BBox);

  /* TODO::zones
  void addZone(Zone* id);
  void removeZone(Zone* id);
  void moveZone(Zone* id);
  std::vector<Zone*> getZonesNearPosition(AABB& BBox);
  */

  struct ColElement<Block> *addDynBlock(Block *id);
  void removeDynBlock(Block *id);
  void moveDynBlock(Block *id);
  std::vector<Block *> &getDynBlocksNearPosition(AABB &BBox);

  /* -1 for actual static block layer, other value (0) for the second static
   * layer */
  void addStaticBlock(Block *id, bool inFrontLayer = false);
  std::vector<Block *> &getStaticBlocksNearPosition(AABB &BBox, int layer = -1);

  void addBlockInLayer(Block *id, int layer);
  std::vector<Block *> &getBlocksNearPositionInLayer(AABB &BBox, int layer);

private:
  /* Data */
  float m_fMinX, m_fMinY, m_fMaxX, m_fMaxY;
  std::vector<Line *> m_Lines;

  ElementHandler<Entity> m_entitiesHandler;
  ElementHandler<Block> m_dynBlocksHandler;
  /* TODO::zones
  ElementHandler<Zone>   m_zonesHandler;
  */
  ElementHandler<Block> m_staticBlocksHandler;
  ElementHandler<Block> m_staticBlocksHandlerSecondLayer;
  std::vector<ElementHandler<Block> *> m_layerBlocksHandlers;

  bool m_bDebugFlag;

  float m_fCellWidth, m_fCellHeight;
  int m_nGridWidth, m_nGridHeight;

  GridCell *m_pGrid;

  bool m_bDynamicTouched;

  /* Helpers */
  bool _CheckCircleAndLine(Line *pLine, float x, float y, float r);
  int _CollideCircleAndLine(Line *pLine,
                            float x,
                            float y,
                            float r,
                            dContact *pContacts,
                            int nOldNumC,
                            int nMaxC,
                            float fGrip,
                            PhysicsSettings *i_physicsSettings);
  void _SetWheelContactParams(dContact *pc,
                              const Vector2f &Pos,
                              const Vector2f &NormalT,
                              double fDepth,
                              float fGrip,
                              PhysicsSettings *i_physicsSettings);
  double _CalculateDepth(const Vector2f &Cp, float Cr, Vector2f P);
  double _CalculateCircleLineDepth(const Vector2f &Cp,
                                   float Cr,
                                   Vector2f P1,
                                   Vector2f P2);
  int _AddContactToList(dContact *pContacts,
                        int nNumContacts,
                        dContact *pc,
                        int nMaxContacts);
};

#endif
