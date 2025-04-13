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

#ifndef __GUIXMOTO_H__
#define __GUIXMOTO_H__

#include "../basic/GUI.h"
#include "helpers/VMath.h"
#include "xmoto/VirtualLevelsList.h"

class LevelsPack;

class UILevelList
  : public UIList
  , public VirtualLevelsList {
public:
  UILevelList(UIWindow *pParent,
              int x = 0,
              int y = 0,
              const std::string &Caption = "",
              int nWidth = 0,
              int nHeight = 0);
  ~UILevelList();

  std::string getLevel(int n);
  std::string getSelectedLevel();
  void addLevel(const std::string &i_id_level,
                const std::string &i_name,
                int i_playerHighscore, // negativ if no one
                int i_roomHighscore, // negativ if no one
                const std::string &i_prefix = "");
  virtual void clear();
  void updateLevel(const std::string &i_id_level, int i_playerHighscore);

  std::string determineNextLevel(const std::string &i_id_level);
  std::string determinePreviousLevel(const std::string &i_id_level);

  void hideBestTime();
  void hideRoomBestTime();

private:
};

class UIPackTree : public UIList {
public:
  UIPackTree(UIWindow *pParent,
             int x = 0,
             int y = 0,
             const std::string &Caption = "",
             int nWidth = 0,
             int nHeight = 0);
  ~UIPackTree();

  void addPack(LevelsPack *i_levelsPack,
               const std::string &i_categorie,
               int i_nbFinishedLevels,
               int i_nbLevels);
  LevelsPack *getSelectedPack();

  void updatePack(LevelsPack *i_levelsPack,
                  int i_nbFinishedLevels,
                  int i_nbLevels);

  void setSelectedPackByName(const std::string &i_levelsPackName);

  virtual std::string subContextHelp(int x, int y);

private:
};

class UIQuickStartButton : public UIButtonDrawn {
public:
  UIQuickStartButton(UIWindow *pParent,
                     int x = 0,
                     int y = 0,
                     std::string Caption = "",
                     int nWidth = 0,
                     int nHeight = 0,
                     unsigned int i_qualityMIN = 0,
                     unsigned int i_difficultyMIN = 0,
                     unsigned int i_qualityMAX = 0,
                     unsigned int i_difficultyMAX = 0);
  ~UIQuickStartButton();

  virtual void paint();
  virtual void mouseLDown(int x, int y);
  int getQualityMIN() const;
  int getDifficultyMIN() const;
  int getQualityMAX() const;
  int getDifficultyMAX() const;
  virtual std::string subContextHelp(int x, int y);
  bool hasChanged(); // indicate whether the button values quality or difficulty
  // changed
  void setHasChanged(bool i_value);

private:
  Texture *m_uncheckedTex, *m_qualityTex, *m_difficultyTex;
  unsigned int m_qualityMIN, m_difficultyMIN;
  unsigned int m_qualityMAX, m_difficultyMAX;
  bool m_hasChanged;

  Vector2i getQualityPoint(const Vector2i &i_center,
                           unsigned int i_ray,
                           unsigned int i_value);
  Vector2i getDifficultyPoint(const Vector2i &i_center,
                              unsigned int i_ray,
                              unsigned int i_value);

  bool isXYInCircle(int x, int y, Vector2i i_center, unsigned int v_ray);
};

#endif /* __GUIXMOTO_H__ */
