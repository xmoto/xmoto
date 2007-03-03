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
         std::string Caption,
         int nWidth,int nHeight)
    :UIList(pParent, x, y, Caption, nWidth, nHeight) {
      addColumn(GAMETEXT_LEVEL, getPosition().nWidth - 175);
      addColumn(std::string(GAMETEXT_TIME) + ":",80);  
#if defined(SUPPORT_WEBACCESS)    
      addColumn("WR:",80);  
#endif
    }

UILevelList::~UILevelList() {
}
 
Level* UILevelList::getSelectedLevel() {
  Level *pLevelSrc = NULL;
  if(!isBranchHidden() && getSelected()>=0) {
    if(!getEntries().empty()) {
      vapp::UIListEntry *pEntry = getEntries()[getSelected()];
      pLevelSrc = reinterpret_cast<Level *>(pEntry->pvUser);        
    }
  }

  return pLevelSrc;
}

void UILevelList::hideBestTime() {
  setHideColumn(1);
}

void UILevelList::hideRoomBestTime() {
  setHideColumn(2);
}

void UILevelList::addLevel(Level *pLevel,
         vapp::PlayerProfile *p_player,
         vapp::PlayerData *p_profile,
#if defined(SUPPORT_WEBACCESS) 
         WebRoom *p_pWebHighscores,
#endif
         std::string p_prefix) {
  std::string Name,File;
      
  if(pLevel->Name() != "") Name = pLevel->Name();
  else Name = "???";

  if(pLevel->FileName() != "") File = vapp::FS::getFileBaseName(pLevel->FileName());
  else File = "???";

  vapp::UIListEntry *pEntry = NULL;
  vapp::PlayerTimeEntry *pTimeEntry = NULL;
      
  pEntry = addEntry(p_prefix + Name,reinterpret_cast<void *>(pLevel));
  
  /* Add times to list entry */
  if(pEntry != NULL) {
    pTimeEntry = p_profile->getBestPlayerTime(p_player->PlayerName, pLevel->Id());
      
    if(pTimeEntry != NULL)
      pEntry->Text.push_back(vapp::App::formatTime(pTimeEntry->fFinishTime));
    else
      pEntry->Text.push_back("--:--:--");

#if defined(SUPPORT_WEBACCESS)
    if(p_pWebHighscores != NULL) {    
      WebHighscore *pWH = p_pWebHighscores->getHighscoreFromLevel(pLevel->Id());
      if(pWH != NULL)
  pEntry->Text.push_back(pWH->getTime());
      else
  pEntry->Text.push_back(GAMETEXT_WORLDRECORDNA);
    }
    else
      pEntry->Text.push_back(GAMETEXT_WORLDRECORDNA);
#endif          
  }
}  

void UILevelList::updateLevelsInformations(vapp::PlayerProfile *p_player,
					   vapp::PlayerData *p_profile
#if defined(SUPPORT_WEBACCESS) 
					   , WebRoom *p_pWebHighscores
#endif
					   ) {
  Level *pLevel;
  vapp::UIListEntry *pEntry;
  vapp::PlayerTimeEntry *pTimeEntry = NULL;
  
  for(int i=0; i<getEntries().size(); i++) {
    pEntry = getEntries()[i];
    pLevel = static_cast<Level*>(pEntry->pvUser);

    /* Add times to list entry */
    pTimeEntry = p_profile->getBestPlayerTime(p_player->PlayerName, pLevel->Id());
    
    if(pTimeEntry != NULL)
      pEntry->Text[1] = vapp::App::formatTime(pTimeEntry->fFinishTime);
    else
      pEntry->Text[1] = "--:--:--";
    
#if defined(SUPPORT_WEBACCESS)
    if(p_pWebHighscores != NULL) {    
      WebHighscore *pWH = p_pWebHighscores->getHighscoreFromLevel(pLevel->Id());
      if(pWH != NULL)
	pEntry->Text[2] = pWH->getTime();
      else
	pEntry->Text[2] = GAMETEXT_WORLDRECORDNA;
    }
    else
     pEntry->Text[2] = GAMETEXT_WORLDRECORDNA;
#endif     
  }
}

UIPackTree::UIPackTree(UIWindow *pParent,
		       int x, int y,
		       std::string Caption,
		       int nWidth, int nHeight)
  : vapp::UIList(pParent, x, y, Caption, nWidth, nHeight) {
  addColumn(GAMETEXT_LEVELPACK, getPosition().nWidth-150, CONTEXTHELP_LEVELPACK);
  addColumn(GAMETEXT_NUMLEVELS, 150, CONTEXTHELP_LEVELPACKNUMLEVELS);
}

UIPackTree::~UIPackTree() {
}

void UIPackTree::addPack(LevelsPack* i_levelsPack,
			 std::string i_categorie,
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
    /* Make lowercase before comparing */
    std::string LCo = i_levelsPack->Name();
    for(int i=0; i<LCo.length(); i++)  
      LCo[i] = tolower(LCo[i]);

    int position;
    for(position = found+1; position<getEntries().size(); position++) {
      /* next categorie => break */
      if(getEntries()[position]->pvUser == NULL) {
	break;
      }
      /* must be done after the next entry ? */
      std::string LC = getEntries()[position]->Text[0];
      for(int i=0;i<LC.length();i++)  
	LC[i] = tolower(LC[i]);
      if(LCo < LC) {
	break;
      }
    }
    p = addEntry(i_levelsPack->Name(), i_levelsPack, position);
  } else {
    /* the categorie doesn't exist, add the categorie and the entry */
    c = addEntry(i_categorie);
    c->bUseOwnProperties  = true;
    c->ownTextColor       = MAKE_COLOR(220,150,50,255);
    c->ownSelectedColor   = MAKE_COLOR(50,50,70,255);
    c->ownUnSelectedColor = MAKE_COLOR(50,50,70,255);
    c->ownXOffset         = 30;
    c->ownYOffset         = 2;
    p = addEntry(i_levelsPack->Name(), i_levelsPack);
  }

  std::ostringstream v_level_nb;
  v_level_nb << i_nbFinishedLevels;
  v_level_nb << "/";
  v_level_nb << i_nbLevels;
  p->Text.push_back(v_level_nb.str());
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

void UIPackTree::setSelectedPackByName(std::string i_levelsPackName) {
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

