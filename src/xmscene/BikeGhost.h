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
  Ghost(PhysicsSettings *i_physicsSettings,
        bool i_engineSound,
        Theme *i_theme,
        BikerTheme *i_bikerTheme,
        const TColor &i_colorFilter,
        const TColor &i_uglyColorFilter);
  ~Ghost();

  virtual bool diffToPlayerAvailable() const { return true; };
  virtual void updateDiffToPlayer(std::vector<float> &i_lastToTakeEntities) = 0;
  virtual bool isNetGhost() const { return false; };
  float diffToPlayer() const;
  virtual bool getRenderBikeFront();
  void setInfo(const std::string &i_info);

  void setReference(bool i_value);
  bool isReference() const;

protected:
  float m_diffToPlayer; /* time diff between the ghost and the player */
  std::string m_info;
  bool m_reference;
};

class FileGhost : public Ghost {
public:
  FileGhost(std::string i_replayFile,
            PhysicsSettings *i_physicsSettings,
            bool i_isActiv,
            bool i_engineSound,
            Theme *i_theme,
            BikerTheme *i_bikerTheme,
            const TColor &i_colorFilter,
            const TColor &i_uglyColorFilter);
  ~FileGhost();

  std::string playerName();
  std::string levelId();
  int getFinishTime();
  void initLastToTakeEntities(Level *i_level);
  virtual void updateDiffToPlayer(std::vector<float> &i_lastToTakeEntities);
  virtual void updateToTime(int i_time,
                            int i_timeStep,
                            CollisionSystem *i_collisionSystem,
                            Vector2f i_gravity,
                            Scene *i_motogame);
  virtual bool isNetGhost() const { return false; };
  std::string getDescription() const;
  std::string getQuickDescription() const;
  std::string getVeryQuickDescription() const;
  float getBikeEngineSpeed();
  float getBikeLinearVel();
  float getTorsoVelocity();
  double getAngle();
  Replay *getReplay() { return m_replay; };

  virtual void initToPosition(Vector2f i_position,
                              DriveDir i_direction,
                              Vector2f i_gravity);

protected:
  Replay *m_replay;

private:
  std::vector<float> m_lastToTakeEntities;
  bool m_isActiv;
  double m_linearVelocity;
  bool m_teleportationOccurred; // true if the teleportation occurred since the
  // last update

  /* because we have not the real one, but the one before and the one after */
  std::vector<BikeState *> m_ghostBikeStates;

  void execReplayEvents(int i_time, Scene *i_motogame);

  // stuff needed for GhostTrail
  PhysicsSettings *m_pyhsicsSettings;
};

class ReplayBiker : public FileGhost {
public:
  ReplayBiker(std::string i_replayFile,
              PhysicsSettings *i_physicsSettings,
              bool i_engineSound,
              Theme *i_theme,
              BikerTheme *i_bikerTheme);
  std::string getQuickDescription() const;
  std::string getVeryQuickDescription() const;

private:
};

class NetGhost : public Ghost {
public:
  NetGhost(PhysicsSettings *i_physicsSettings,
           bool i_engineSound,
           Theme *i_theme,
           BikerTheme *i_bikerTheme,
           const TColor &i_colorFilter,
           const TColor &i_uglyColorFilter);
  ~NetGhost();

  virtual bool diffToPlayerAvailable() const { return false; };
  virtual void updateDiffToPlayer(std::vector<float> &i_lastToTakeEntities);
  virtual bool isNetGhost() const { return true; };
  std::string getDescription() const;
  std::string getQuickDescription() const;
  std::string getVeryQuickDescription() const;
  float getBikeEngineSpeed();
  float getBikeLinearVel();
  float getTorsoVelocity();
  double getAngle();

private:
};
#endif
