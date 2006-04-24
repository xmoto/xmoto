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

#ifndef __WEBHSAPPINTERFACE_H__
#define __WEBHSAPPINTERFACE_H__

#include "BuildConfig.h"

#if defined(SUPPORT_WEBHIGHSCORES)
#include <string>

namespace vapp {

  enum WebHSTask {
    TASK_DOWNLOADING_LEVELS,
    TASK_DOWNLOADING_REPLAYS,
    TASK_DOWNLOADING_HIGHSCORES,
    /* more? */
  };

  class WebHSAppInterface {
    public:
      virtual ~WebHSAppInterface() {}
      
      /* Task management */
      virtual void beginTask(WebHSTask Task) = 0;
      virtual void setTaskProgress(float fPercent) = 0;
      virtual void endTask(void) = 0;
      
      virtual void setBeingDownloadedLevel(const std::string &LevelName) = 0;
      virtual void readEvents(void) = 0;
      
      /* Level tools */
      virtual bool doesLevelExist(const std::string &LevelID) = 0; 
      
  };

};

#endif

#endif

