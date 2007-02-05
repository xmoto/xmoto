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
 
vapp::LevelSrc* UILevelList::getSelectedLevel() {
  vapp::LevelSrc *pLevelSrc = NULL;
  if(!isBranchHidden() && getSelected()>=0) {
    if(!getEntries().empty()) {
      vapp::UIListEntry *pEntry = getEntries()[getSelected()];
      pLevelSrc = reinterpret_cast<vapp::LevelSrc *>(pEntry->pvUser);        
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

void UILevelList::addLevel(vapp::LevelSrc *pLevel,
			   vapp::PlayerProfile *p_player,
			   vapp::PlayerData *p_profile,
#if defined(SUPPORT_WEBACCESS) 
			   WebRoom *p_pWebHighscores,
#endif
			   std::string p_prefix) {
  std::string Name,File;
      
  if(pLevel->getLevelInfo()->Name != "") Name = pLevel->getLevelInfo()->Name;
  else Name = "???";

  if(pLevel->getFileName() != "") File = vapp::FS::getFileBaseName(pLevel->getFileName());
  else File = "???";

  vapp::UIListEntry *pEntry = NULL;
  vapp::PlayerTimeEntry *pTimeEntry = NULL;
      
  pEntry = addEntry(p_prefix + Name,reinterpret_cast<void *>(pLevel));
  
  /* Add times to list entry */
  if(pEntry != NULL) {
    pTimeEntry = p_profile->getBestPlayerTime(p_player->PlayerName, pLevel->getID());
      
    if(pTimeEntry != NULL)
      pEntry->Text.push_back(vapp::App::formatTime(pTimeEntry->fFinishTime));
    else
      pEntry->Text.push_back("--:--:--");

#if defined(SUPPORT_WEBACCESS)
    if(p_pWebHighscores != NULL) {    
      WebHighscore *pWH = p_pWebHighscores->getHighscoreFromLevel(pLevel->getID());
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


