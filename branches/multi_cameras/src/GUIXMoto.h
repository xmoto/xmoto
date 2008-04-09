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

#ifndef __GUIXMOTO_H__
#define __GUIXMOTO_H__

#include "GUI.h"
#include "xmscene/Level.h"
#include "WWW.h"
#include "LevelsManager.h"
  
class UILevelList : public vapp::UIList {
  public:
  UILevelList(UIWindow *pParent,
	      int x = 0,int y = 0,
	      const std::string& Caption="",
	      int nWidth = 0,int nHeight = 0);
  ~UILevelList();
  
  std::string getLevel(int n);
  std::string getSelectedLevel();
  void addLevel(const std::string& i_id_level,
		const std::string& i_name,
		float i_playerHighscore, // negativ if no one
		float i_roomHighscore,   // negativ if no one
		const std::string& i_prefix = "");
  virtual void clear();
  void updateLevel(const std::string& i_id_level, float i_playerHighscore);

  void hideBestTime();
  void hideRoomBestTime();
  
  private:
};

class UIPackTree : public vapp::UIList {
 public:
  UIPackTree(UIWindow *pParent,
	     int x = 0,int y = 0,
	     const std::string& Caption="",
	     int nWidth = 0,int nHeight = 0);
  ~UIPackTree();

  void addPack(LevelsPack* i_levelsPack,
	       const std::string& i_categorie,
	       int i_nbFinishedLevels,
	       int i_nbLevels);
  LevelsPack* getSelectedPack();

  void setSelectedPackByName(const std::string& i_levelsPackName);

  virtual std::string subContextHelp(int x,int y);

 private:
};


#endif /* __GUIXMOTO_H__ */