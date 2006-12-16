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

#ifndef __CREDITS_H__
#define __CREDITS_H__

#include "VCommon.h"
#include "GUI.h"

namespace vapp {

  /*===========================================================================
  Credits class
  ===========================================================================*/
  class Credits {
    public:
      /* Types */
      struct Entry {
        std::string Left,Right;
        int nLeftWidth,nRightWidth;
      };
    
      /* Construction/destruction */
      Credits();
      ~Credits();
      
      /* Methods */
      void init(float fBackgroundReplayLength,float fFadeInLength,float fFadeOutLength,const char *pcCredits);
      void render(float fTime);
      bool isFinished(void);
      
    private:
      /* Data */
      std::vector<Entry *> m_Entries;
      int m_nWidestLeft,m_nWidestRight;
      float m_fTime,m_fReplayLength,m_fFadeIn,m_fFadeOut;
      bool m_bBlackBackground;
      UIFont *m_pFont;
      App *m_pApp;
      bool m_bFinished;
      
      /* Helpers */
      int _GetStringWidth(const std::string &s);
  };
  
}


#endif
