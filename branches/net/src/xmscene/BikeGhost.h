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

#ifndef __BIKEGHOST_H__
#define __BIKEGHOST_H__

#include "Bike.h"

class Ghost : public Biker {
  public:
  Ghost(PhysicsSettings* i_physicsSettings,
	Theme *i_theme, BikerTheme* i_bikerTheme,
	const TColor& i_colorFilter,
	const TColor& i_uglyColorFilter);
  ~Ghost();

  virtual void updateDiffToPlayer(std::vector<float> &i_lastToTakeEntities) = 0;
  float diffToPlayer() const;
  virtual bool getRenderBikeFront();
  void setInfo(const std::string& i_info);

  protected:
  float m_diffToPlayer; /* time diff between the ghost and the player */
  std::string m_info;
};

class FileGhost : public Ghost {
 public:
  FileGhost(std::string i_replayFile,
	PhysicsSettings* i_physicsSettings,
	bool i_isActiv, Theme *i_theme,
	BikerTheme* i_bikerTheme,
	const TColor& i_colorFilter,
	const TColor& i_uglyColorFilter);
  ~FileGhost();

  std::string playerName();
  std::string levelId();
  int getFinishTime();
  void initLastToTakeEntities(Level* i_level);
  virtual void updateDiffToPlayer(std::vector<float> &i_lastToTakeEntities);
  virtual void updateToTime(int i_time, int i_timeStep,
			    CollisionSystem *i_collisionSystem, Vector2f i_gravity,
			    MotoGame *i_motogame);
  std::string getQuickDescription() const;
  std::string getDescription() const;
  float getBikeEngineSpeed();
  float getBikeLinearVel();
  double getAngle();

  virtual void initToPosition(Vector2f i_position, DriveDir i_direction, Vector2f i_gravity);

 protected:
  Replay* m_replay;

 private:
  std::vector<float> m_lastToTakeEntities;
  bool m_isActiv;
  double m_linearVelocity;
  bool m_teleportationOccured; // true if the teleportation occured since the last update

 /* because we have not the real one, but the one before and the one after */
 std::vector<BikeState*> m_ghostBikeStates;

 void execReplayEvents(int i_time, MotoGame *i_motogame);
};

class NetGhost : public Ghost {
 public:
  NetGhost(PhysicsSettings* i_physicsSettings,
	   Theme *i_theme, BikerTheme* i_bikerTheme,
	   const TColor& i_colorFilter,
	   const TColor& i_uglyColorFilter);
  ~NetGhost();

  virtual void updateDiffToPlayer(std::vector<float> &i_lastToTakeEntities);

  std::string getQuickDescription() const;
  std::string getDescription() const;
  float getBikeEngineSpeed();
  float getBikeLinearVel();
  double getAngle();
 
 private:
  
};

#endif