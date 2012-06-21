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

#ifndef __TRAINER_H__
#define __TRAINER_H__

#include "helpers/VMath.h"
#include "helpers/Singleton.h"
#include <vector>


class Trainer : public Singleton<Trainer> {
  friend class Singleton<Trainer>;

private:
  Trainer();
  ~Trainer();

public:
  bool isRestorePositionAvailable( std::string levelId );
  int getCurrentRestoreIndex();
  int getMaxRestoreIndex();
  Vector2f getCurrentRestorePosition( std::string levelId );
  Vector2f getPreviousRestorePosition( std::string levelId );
  Vector2f getNextRestorePosition( std::string levelId );
  void storePosition( std::string levelId, Vector2f pos );

  void resetTrainerUse();
  bool trainerHasBeenUsed();

private:
  void changeLevel( std::string toLevelId );

private:
  std::string m_forLevelId;  // restore positions are invalidated on level change
  std::vector<Vector2f> m_storePos;
  int m_storePosReadIndex;
  bool m_trainerHasBeenUsed;

};

#endif
