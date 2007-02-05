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
#include "PlayerData.h"
#include "WWW.h"
  
  class UILevelList : public vapp::UIList {
    public:
    UILevelList(UIWindow *pParent,
		int x = 0,int y = 0,
		std::string Caption="",
		int nWidth = 0,int nHeight = 0);
    ~UILevelList();
    
    Level* getSelectedLevel();
    void addLevel(Level *pLevel,
		  vapp::PlayerProfile *p_player,
		  vapp::PlayerData *p_profile,
#if defined(SUPPORT_WEBACCESS) 
		  WebRoom *p_pWebHighscores,
#endif
		  std::string p_prefix = "");

    void hideBestTime();
    void hideRoomBestTime();
    
    private:
  };

#endif /* __GUIXMOTO_H__ */
