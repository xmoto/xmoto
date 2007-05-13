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

#include "GUIXMoto.h"
#include "GameText.h"
#include "VFileIO.h"
#include "VApp.h"

UILevelList::UILevelList(UIWindow *pParent,
			 int x,int y,
			 const std::string& Caption,
			 int nWidth,int nHeight)
  :UIList(pParent, x, y, Caption, nWidth, nHeight) {
    addColumn(GAMETEXT_LEVEL, getPosition().nWidth - 175);
    addColumn(std::string(GAMETEXT_TIME) + ":",80);  
    addColumn("WR:",80);  
}

UILevelList::~UILevelList() {
  clear();
}

std::string UILevelList::getLevel(int n) {
  if(getEntries().empty() == false) {
    vapp::UIListEntry *pEntry = getEntries()[n];
    return *(reinterpret_cast<std::string *>(pEntry->pvUser));
  }
  return "";
}

std::string UILevelList::getSelectedLevel() {
  if(!isBranchHidden() && getSelected()>=0) {
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
  for(int i=0; i<getEntries().size(); i++) {
    delete getEntries()[i]->pvUser;
  }
  UIList::clear();
}

void UILevelList::addLevel(const std::string& i_id_level,
			   const std::string& i_name,
			   float i_playerHighscore,
			   float i_roomHighscore,
			   const std::string& i_prefix) {
  std::string v_name;

  if(i_name != "") v_name = i_name;
  else v_name = "???";

  vapp::UIListEntry *pEntry = NULL;
  pEntry = addEntry(i_prefix + v_name, reinterpret_cast<void *>(new String(i_id_level)));
  
  /* Add times to list entry */
  if(pEntry != NULL) {
    if(i_playerHighscore < 0.0) {
      pEntry->Text.push_back("--:--:--");
    } else {
      pEntry->Text.push_back(vapp::App::formatTime(i_playerHighscore));
    }

    if(i_roomHighscore < 0.0) {
      pEntry->Text.push_back(GAMETEXT_WORLDRECORDNA);
    } else {
      pEntry->Text.push_back(vapp::App::formatTime(i_roomHighscore));
    }
  }
}  

void UILevelList::updateLevel(const std::string& i_id_level, float i_playerHighscore) {
  for(unsigned int i=0; i<getEntries().size(); i++) {
    if(*(reinterpret_cast<std::string *>(getEntries()[i]->pvUser)) == i_id_level) {
      getEntries()[i]->Text[1] = vapp::App::formatTime(i_playerHighscore);
    }
  }
}

UIPackTree::UIPackTree(UIWindow *pParent,
		       int x, int y,
		       const std::string& Caption,
		       int nWidth, int nHeight)
  : vapp::UIList(pParent, x, y, Caption, nWidth, nHeight) {
  addColumn(GAMETEXT_LEVELPACK, getPosition().nWidth-150, CONTEXTHELP_LEVELPACK);
  addColumn(GAMETEXT_NUMLEVELS, 150, CONTEXTHELP_LEVELPACKNUMLEVELS);
}

UIPackTree::~UIPackTree() {
}

void UIPackTree::addPack(LevelsPack* i_levelsPack,
			 const std::string& i_categorie,
			 int i_nbFinishedLevels,
			 int i_nbLevels) {
  vapp::UIListEntry *c, *p;

  /* looking the categorie */
  int found = -1;
  for(int i=0; i<getEntries().size(); i++) {
    if(getEntries()[i]->pvUser == NULL) {
      if(getEntries()[i]->Text[0] == i_categorie) {
	found = i;
	break;
      }
    }
  }

  /* the categorie exists, add the entry */
  if(found != -1) {
    /* sort */
    int position;
    for(position = found+1; position<getEntries().size(); position++) {
      /* next categorie => break */
      if(getEntries()[position]->pvUser == NULL) {
	break;
      }
    }
    p = addEntry(i_levelsPack->Name(), i_levelsPack, position);
  } else {
    /* the categorie doesn't exist, add the categorie and the entry */
    c = addEntry(i_categorie);
    c->bUseOwnProperties  = true;
    c->ownTextColor       = MAKE_COLOR(207,204,71,255);
    c->ownSelectedColor   = MAKE_COLOR(4,0,87,255);
    c->ownUnSelectedColor = MAKE_COLOR(4,0,87,255);
    c->ownXOffset         = 30;
    c->ownYOffset         = 0;
    p = addEntry(i_levelsPack->Name(), i_levelsPack);
  }

  std::ostringstream v_level_nb;
  v_level_nb << i_nbFinishedLevels;
  v_level_nb << "/";
  v_level_nb << i_nbLevels;
  p->Text.push_back(v_level_nb.str());
}

void UIPackTree::updatePack(LevelsPack* i_levelsPack,
			    int i_nbFinishedLevels,
			    int i_nbLevels) {
  for(unsigned int i=0; i<getEntries().size(); i++) {
    if(getEntries()[i]->pvUser != NULL) {
      if(i_levelsPack == getEntries()[i]->pvUser) {
	  std::ostringstream v_level_nb;
	  v_level_nb << i_nbFinishedLevels;
	  v_level_nb << "/";
	  v_level_nb << i_nbLevels;
	  getEntries()[i]->Text[1] = v_level_nb.str();
      }
    }
  }
}

LevelsPack* UIPackTree::getSelectedPack() {
  if(getSelected() >= 0 && getSelected() < getEntries().size()) {
    vapp::UIListEntry *pEntry = getEntries()[getSelected()];
    if(pEntry->pvUser != NULL) {
      return (LevelsPack*) pEntry->pvUser;
    }
  }
  return NULL;
}

void UIPackTree::setSelectedPackByName(const std::string& i_levelsPackName) {
  int nPack = 0;
  for(int i=0; i<getEntries().size(); i++) {
    if(getEntries()[i]->pvUser != NULL) {
      if(getEntries()[i]->Text[0] == i_levelsPackName) {
	nPack = i;
	break;
      }
    }
  }
  setRealSelected(nPack);
}

std::string UIPackTree::subContextHelp(int x,int y) {
  int n;
  LevelsPack *v_levelPack;

  if(getColumnAtPosition(x, y) != 0) {
    return vapp::UIList::subContextHelp(x, y);
  }

  n = getRowAtPosition(x, y);
  if(n == -1) {
    return "";
  }

  vapp::UIListEntry *pEntry = getEntries()[n];
  if(pEntry->pvUser == NULL) {
    return "";
  }

  v_levelPack = (LevelsPack*) pEntry->pvUser;
  return v_levelPack->Description();
}
