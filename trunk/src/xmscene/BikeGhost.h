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

#ifndef __BIKEGHOST_H__
#define __BIKEGHOST_H__

#include "Bike.h"

class Ghost : public Biker {
 public:
  Ghost(std::string i_replayFile, bool i_isActiv, Theme *i_theme, BikerTheme* i_bikerTheme);
  ~Ghost();

  std::string playerName();
  std::string levelId();
  float getFinishTime();
  void initLastToTakeEntities(Level* i_level);
  void updateDiffToPlayer(std::vector<float> &i_lastToTakeEntities);
  float diffToPlayer() const;
  virtual void updateToTime(float i_time, float i_timeStep,
			    vapp::CollisionSystem *i_collisionSystem, Vector2f i_gravity,
			    vapp::MotoGame *i_motogame);
  void setInfo(std::string i_info);
  std::string getDescription() const;
  bool getRenderBikeFront();
  float getBikeEngineSpeed();

 private:
  vapp::Replay* m_replay;
  std::vector<float> m_lastToTakeEntities;
  float m_diffToPlayer; /* time diff between the ghost and the player */
  std::string m_info;
  bool m_isActiv;

  /* because we have not the real one, but the one before and the one after */
  SerializedBikeState m_previous_ghostBikeState;
  SerializedBikeState m_next_ghostBikeState;

  void execReplayEvents(float i_time, vapp::MotoGame *i_motogame);
};

#endif
