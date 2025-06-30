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

#include "GUIXMoto.h"
#include "common/VFileIO.h"
#include "drawlib/DrawLib.h"
#include "helpers/Text.h"
#include "xmoto/GameText.h"
#include "xmoto/LevelsManager.h"
#include <sstream>

#define UIQUICKSTART_BORDER 25

UILevelList::UILevelList(UIWindow *pParent,
                         int x,
                         int y,
                         const std::string &Caption,
                         int nWidth,
                         int nHeight)
  : UIList(pParent, x, y, Caption, nWidth, nHeight) {
  addColumn(GAMETEXT_LEVEL, getPosition().nWidth - 175);
  addColumn(std::string(GAMETEXT_TIME) + ":", 80, GAMETEXT_YOURBESTTIME);
  addColumn(std::string(GAMETEXT_ROOM) + ":", 80, GAMETEXT_HIGHSCOREOFTHEROOM);
}

UILevelList::~UILevelList() {
  clear();
}

std::string UILevelList::getLevel(int n) {
  if (getEntries().empty() == false) {
    UIListEntry *pEntry = getEntries()[n];
    return *(reinterpret_cast<std::string *>(pEntry->pvUser));
  }
  return "";
}

std::string UILevelList::getSelectedLevel() {
  if (!isBranchHidden() && getSelected() >= 0) {
    return getLevel(getSelected());
  }

  return "";
}

void UILevelList::hideBestTime() {
  setHideColumn(1);
}

void UILevelList::hideRoomBestTime() {
  setHideColumn(2);
}

void UILevelList::clear() {
  for (unsigned int i = 0; i < getEntries().size(); i++) {
    delete ((std::string *)getEntries()[i]->pvUser);
  }
  UIList::clear();
}

void UILevelList::addLevel(const std::string &i_id_level,
                           const std::string &i_name,
                           int i_playerHighscore,
                           int i_roomHighscore,
                           const std::string &i_prefix) {
  std::string v_name;

  if (i_name != "")
    v_name = i_name;
  else
    v_name = "???";

  UIListEntry *pEntry = NULL;
  pEntry = addEntry(i_prefix + v_name,
                    reinterpret_cast<void *>(new std::string(i_id_level)));

  /* Add times to list entry */
  if (pEntry != NULL) {
    if (i_playerHighscore < 0) {
      pEntry->Text.push_back(GAMETEXT_HIGHSCORE_NONE);
    } else {
      pEntry->Text.push_back(formatTime(i_playerHighscore));
    }

    if (i_roomHighscore < 0) {
      pEntry->Text.push_back(GAMETEXT_HIGHSCORE_NONE);
    } else {
      pEntry->Text.push_back(formatTime(i_roomHighscore));
    }
  }
}

void UILevelList::updateLevel(const std::string &i_id_level,
                              int i_playerHighscore) {
  for (unsigned int i = 0; i < getEntries().size(); i++) {
    if (*(reinterpret_cast<std::string *>(getEntries()[i]->pvUser)) ==
        i_id_level) {
      getEntries()[i]->Text[1] = formatTime(i_playerHighscore);
    }
  }
}

std::string UILevelList::determineNextLevel(const std::string &i_id_level) {
  for (unsigned int i = 0; i < getEntries().size() - 1; i++) {
    if ((*((std::string *)getEntries()[i]->pvUser)) == i_id_level) {
      return *((std::string *)getEntries()[i + 1]->pvUser);
    }
  }
  return *((std::string *)getEntries()[0]->pvUser);
}

std::string UILevelList::determinePreviousLevel(const std::string &i_id_level) {
  for (unsigned int i = 1; i < getEntries().size(); i++) {
    if ((*((std::string *)getEntries()[i]->pvUser)) == i_id_level) {
      return *((std::string *)getEntries()[i - 1]->pvUser);
    }
  }
  return *((std::string *)getEntries()[getEntries().size() - 1]->pvUser);
}

UIPackTree::UIPackTree(UIWindow *pParent,
                       int x,
                       int y,
                       const std::string &Caption,
                       int nWidth,
                       int nHeight)
  : UIList(pParent, x, y, Caption, nWidth, nHeight) {
  addColumn(
    GAMETEXT_LEVELPACK, getPosition().nWidth - 150, CONTEXTHELP_LEVELPACK);
  addColumn(GAMETEXT_NUMLEVELS, 150, CONTEXTHELP_LEVELPACKNUMLEVELS);
}

UIPackTree::~UIPackTree() {}

void UIPackTree::addPack(LevelsPack *i_levelsPack,
                         const std::string &i_categorie,
                         int i_nbFinishedLevels,
                         int i_nbLevels) {
  UIListEntry *c, *p;

  /* looking the categorie */
  int found = -1;
  for (unsigned int i = 0; i < getEntries().size(); i++) {
    if (getEntries()[i]->pvUser == NULL) {
      if (getEntries()[i]->Text[0] == i_categorie) {
        found = i;
        break;
      }
    }
  }

  /* the categorie exists, add the entry */
  if (found != -1) {
    /* sort */
    unsigned int position = found + 1;
    for (; position < getEntries().size(); position++) {
      /* next categorie => break */
      if (getEntries()[position]->pvUser == NULL) {
        break;
      }
    }
    p = addEntry(i_levelsPack->Name(), i_levelsPack, position);
  } else {
    /* the categorie doesn't exist, add the categorie and the entry */
    c = addEntry(i_categorie);
    c->bUseOwnProperties = true;
    c->ownTextColor = MAKE_COLOR(207, 204, 71, 255);
    c->ownSelectedColor = MAKE_COLOR(4, 0, 87, 255);
    c->ownUnSelectedColor = MAKE_COLOR(4, 0, 87, 255);
    c->ownXOffset = 30;
    c->ownYOffset = 0;
    p = addEntry(i_levelsPack->Name(), i_levelsPack);
  }

  std::ostringstream v_level_nb;

  if (i_nbFinishedLevels != -1 && i_nbLevels != -1) {
    v_level_nb << i_nbFinishedLevels;
    v_level_nb << "/";
    v_level_nb << i_nbLevels;
  } else {
    v_level_nb << "?";
  }

  p->Text.push_back(v_level_nb.str());
  p->bFiltered = i_nbLevels == 0; // filter packs with 0 levels
  checkForFilteredEntries();
}

void UIPackTree::updatePack(LevelsPack *i_levelsPack,
                            int i_nbFinishedLevels,
                            int i_nbLevels) {
  for (unsigned int i = 0; i < getEntries().size(); i++) {
    if (getEntries()[i]->pvUser != NULL) {
      if (i_levelsPack == getEntries()[i]->pvUser) {
        std::ostringstream v_level_nb;

        if (i_nbFinishedLevels != -1 && i_nbLevels != -1) {
          v_level_nb << i_nbFinishedLevels;
          v_level_nb << "/";
          v_level_nb << i_nbLevels;
        } else {
          v_level_nb << "?";
        }
        getEntries()[i]->Text[1] = v_level_nb.str();
        getEntries()[i]->bFiltered = (i_nbLevels == 0);
      }
    }
  }
  checkForFilteredEntries();
}

LevelsPack *UIPackTree::getSelectedPack() {
  if (getSelected() >= 0 && getSelected() < getEntries().size()) {
    UIListEntry *pEntry = getEntries()[getSelected()];
    if (pEntry->pvUser != NULL) {
      return (LevelsPack *)pEntry->pvUser;
    }
  }
  return NULL;
}

void UIPackTree::setSelectedPackByName(const std::string &i_levelsPackName) {
  int nPack = 0;
  for (unsigned int i = 0; i < getEntries().size(); i++) {
    if (getEntries()[i]->pvUser != NULL) {
      if (getEntries()[i]->Text[0] == i_levelsPackName) {
        nPack = i;
        break;
      }
    }
  }
  setRealSelected(nPack);
}

std::string UIPackTree::subContextHelp(int x, int y) {
  int n;
  LevelsPack *v_levelPack;

  if (getColumnAtPosition(x, y) != 0) {
    return UIList::subContextHelp(x, y);
  }

  n = getRowAtPosition(x, y);
  if (n == -1) {
    return "";
  }

  UIListEntry *pEntry = getEntries()[n];
  if (pEntry->pvUser == NULL) {
    return "";
  }

  v_levelPack = (LevelsPack *)pEntry->pvUser;
  return v_levelPack->Description();
}

UIQuickStartButton::UIQuickStartButton(UIWindow *pParent,
                                       int x,
                                       int y,
                                       std::string Caption,
                                       int nWidth,
                                       int nHeight,
                                       unsigned int i_qualityMIN,
                                       unsigned int i_difficultyMIN,
                                       unsigned int i_qualityMAX,
                                       unsigned int i_difficultyMAX)
  : UIButtonDrawn(pParent,
                  "RoundButtonUnpressed",
                  "RoundButtonPressed",
                  "RoundButtonHover",
                  x,
                  y,
                  Caption,
                  nWidth,
                  nHeight) {
  Sprite *v_sprite;
  m_uncheckedTex = m_qualityTex = m_difficultyTex = NULL;

  v_sprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "qsChoiceUnchecked");
  if (v_sprite != NULL) {
    m_uncheckedTex = v_sprite->getTexture();
  }

  v_sprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "qsChoiceQuality");
  if (v_sprite != NULL) {
    m_qualityTex = v_sprite->getTexture();
  }

  v_sprite = Theme::instance()->getSprite(SPRITE_TYPE_UI, "qsChoiceDifficulty");
  if (v_sprite != NULL) {
    m_difficultyTex = v_sprite->getTexture();
  }

  setBorder(UIQUICKSTART_BORDER);
  m_qualityMIN = i_qualityMIN;
  m_difficultyMIN = i_difficultyMIN;
  m_qualityMAX = i_qualityMAX;
  m_difficultyMAX = i_difficultyMAX;

  m_hasChanged = false;
}

UIQuickStartButton::~UIQuickStartButton() {}

bool UIQuickStartButton::hasChanged() {
  return m_hasChanged;
}

void UIQuickStartButton::setHasChanged(bool i_value) {
  m_hasChanged = i_value;
}

void UIQuickStartButton::paint() {
  Vector2i v_point;
  Vector2i v_center = Vector2i(getAbsPosX() + getPosition().nWidth / 2,
                               getAbsPosY() + getPosition().nHeight / 2);
  unsigned int v_ray = getPosition().nWidth / 2 - UIQUICKSTART_BORDER;

  if (isUglyMode()) {
    m_drawLib->drawCircle(Vector2f((float)v_center.x, (float)v_center.y),
                          (float)v_ray,
                          1.0,
                          MAKE_COLOR(160, 40, 40, 255),
                          MAKE_COLOR(160, 40, 40, 255));
  }
  UIButtonDrawn::paint();

  /* draw the quality stars */
  const unsigned int numberStar = 5;
  for (unsigned int i = 0; i < numberStar; i++) {
    v_point = getQualityPoint(v_center, v_ray, i);

    if (i >= m_qualityMIN - 1 && i < m_qualityMAX) {
      if (isUglyMode()) {
        m_drawLib->drawCircle(Vector2f((float)v_point.x, (float)v_point.y),
                              UIQUICKSTART_BORDER / 2,
                              1.0,
                              MAKE_COLOR(0, 255, 0, 255),
                              MAKE_COLOR(0, 255, 0, 255));
      } else {
        m_drawLib->drawImage(
          Vector2f(v_point.x, v_point.y) -
            Vector2f(UIQUICKSTART_BORDER / 2, UIQUICKSTART_BORDER / 2),
          Vector2f(v_point.x, v_point.y) +
            Vector2f(UIQUICKSTART_BORDER / 2, UIQUICKSTART_BORDER / 2),
          m_qualityTex,
          0xFFFFFFFF,
          true);
      }
    } else {
      if (isUglyMode()) {
        m_drawLib->drawCircle(Vector2f((float)v_point.x, (float)v_point.y),
                              UIQUICKSTART_BORDER / 2,
                              1.0,
                              0,
                              MAKE_COLOR(0, 255, 0, 255));
      } else {
        m_drawLib->drawImage(
          Vector2f(v_point.x, v_point.y) -
            Vector2f(UIQUICKSTART_BORDER / 2, UIQUICKSTART_BORDER / 2),
          Vector2f(v_point.x, v_point.y) +
            Vector2f(UIQUICKSTART_BORDER / 2, UIQUICKSTART_BORDER / 2),
          m_uncheckedTex,
          0xFFFFFFFF,
          true);
      }
    }
  }

  /* draw the difficulty stars */
  for (unsigned int i = 0; i < 5; i++) {
    v_point = getDifficultyPoint(v_center, v_ray, i);

    if (i >= m_difficultyMIN - 1 && i < m_difficultyMAX) {
      if (isUglyMode()) {
        m_drawLib->drawCircle(Vector2f((float)v_point.x, (float)v_point.y),
                              UIQUICKSTART_BORDER / 2,
                              1.0,
                              MAKE_COLOR(255, 0, 0, 255),
                              MAKE_COLOR(255, 0, 0, 255));
      } else {
        m_drawLib->drawImage(
          Vector2f(v_point.x, v_point.y) -
            Vector2f(UIQUICKSTART_BORDER / 2, UIQUICKSTART_BORDER / 2),
          Vector2f(v_point.x, v_point.y) +
            Vector2f(UIQUICKSTART_BORDER / 2, UIQUICKSTART_BORDER / 2),
          m_difficultyTex,
          0xFFFFFFFF,
          true);
      }
    } else {
      if (isUglyMode()) {
        m_drawLib->drawCircle(Vector2f((float)v_point.x, (float)v_point.y),
                              UIQUICKSTART_BORDER / 2,
                              1.0,
                              0,
                              MAKE_COLOR(255, 0, 0, 255));
      } else {
        m_drawLib->drawImage(
          Vector2f(v_point.x, v_point.y) -
            Vector2f(UIQUICKSTART_BORDER / 2, UIQUICKSTART_BORDER / 2),
          Vector2f(v_point.x, v_point.y) +
            Vector2f(UIQUICKSTART_BORDER / 2, UIQUICKSTART_BORDER / 2),
          m_uncheckedTex,
          0xFFFFFFFF,
          true);
      }
    }
  }
}

void UIQuickStartButton::mouseLDown(int x, int y) {
  // check as a circle, i don't know how to do for an elipse

  if (isXYInCircle(
        x,
        y,
        Vector2i(getPosition().nWidth / 2, getPosition().nHeight / 2),
        getPosition().nWidth / 2 - UIQUICKSTART_BORDER)) {
    UIButtonDrawn::mouseLDown(x, y);
  } else {
    Vector2i v_center =
      Vector2i(getPosition().nWidth / 2, getPosition().nHeight / 2);
    unsigned int v_ray = getPosition().nWidth / 2 - UIQUICKSTART_BORDER;

    /* check quality the stars */
    for (unsigned int i = 0; i < 5; i++) {
      if (isXYInCircle(x,
                       y,
                       getQualityPoint(v_center, v_ray, i),
                       UIQUICKSTART_BORDER / 2)) {
        if (i + 1 < m_qualityMIN) {
          m_hasChanged = true;
          m_qualityMIN = i + 1;
        } else {
          if (i + 1 > m_qualityMAX) {
            m_hasChanged = true;
            m_qualityMAX = i + 1;
          } else {
            m_hasChanged = true;
            m_qualityMIN = m_qualityMAX = i + 1;
          }
        }
      }
    }

    /* check difficulty the stars */
    for (unsigned int i = 0; i < 5; i++) {
      if (isXYInCircle(x,
                       y,
                       getDifficultyPoint(v_center, v_ray, i),
                       UIQUICKSTART_BORDER / 2)) {
        if (i + 1 < m_difficultyMIN) {
          m_hasChanged = true;
          m_difficultyMIN = i + 1;
        } else {
          if (i + 1 > m_difficultyMAX) {
            m_hasChanged = true;
            m_difficultyMAX = i + 1;
          } else {
            m_hasChanged = true;
            m_difficultyMIN = m_difficultyMAX = i + 1;
          }
        }
      }
    }
  }
}

int UIQuickStartButton::getQualityMIN() const {
  return m_qualityMIN;
}

int UIQuickStartButton::getQualityMAX() const {
  return m_qualityMAX;
}

int UIQuickStartButton::getDifficultyMIN() const {
  return m_difficultyMIN;
}

int UIQuickStartButton::getDifficultyMAX() const {
  return m_difficultyMAX;
}

Vector2i UIQuickStartButton::getQualityPoint(const Vector2i &i_center,
                                             unsigned int i_ray,
                                             unsigned int i_value) {
  float v_angle;
  Vector2i v_point;

  v_angle = (i_value * (M_PI / 8.0)) - M_PI / 2.0 + M_PI / 10.0;
  v_point = Vector2i((int)(cos(v_angle) * (i_ray + UIQUICKSTART_BORDER / 2)),
                     (int)(sin(v_angle) * (i_ray + UIQUICKSTART_BORDER / 2)));
  return i_center - v_point;
}

Vector2i UIQuickStartButton::getDifficultyPoint(const Vector2i &i_center,
                                                unsigned int i_ray,
                                                unsigned int i_value) {
  float v_angle;
  Vector2i v_point;

  v_angle = (i_value * (-M_PI / 8.0)) - M_PI / 2.0 - M_PI / 10.0;
  v_point = Vector2i((int)(cos(v_angle) * (i_ray + UIQUICKSTART_BORDER / 2)),
                     (int)(sin(v_angle) * (i_ray + UIQUICKSTART_BORDER / 2)));
  return i_center - v_point;
}

bool UIQuickStartButton::isXYInCircle(int x,
                                      int y,
                                      Vector2i i_center,
                                      unsigned int v_ray) {
  float v_length;
  v_length = sqrt(((float)(x - i_center.x)) * ((float)(x - i_center.x)) +
                  ((float)(y - i_center.y)) * ((float)(y - i_center.y)));
  return v_length <= ((float)v_ray);
}

std::string UIQuickStartButton::subContextHelp(int x, int y) {
  // check as a circle, i don't know how to do for an elipse
  if (isXYInCircle(
        x,
        y,
        Vector2i(getPosition().nWidth / 2, getPosition().nHeight / 2),
        getPosition().nWidth / 2 - UIQUICKSTART_BORDER)) {
    return CONTEXTHELP_QUICKSTART;
  } else {
    if (x < getPosition().nWidth / 2) {
      return CONTEXTHELP_QUICKSTART_QUALITY;
    } else {
      return CONTEXTHELP_QUICKSTART_DIFFICULTY;
    }
  }
}
