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

#ifndef __WWWAPPINTERFACE_H__
#define __WWWAPPINTERFACE_H__

#include "BuildConfig.h"

#if defined(SUPPORT_WEBACCESS)
#include <string>

namespace vapp {

  class WWWAppInterface {
  public:  
    WWWAppInterface() {m_cancel_as_soon_as_possible=false;}
    virtual ~WWWAppInterface() {}
    
    /* Task management */
    virtual void setTaskProgress(float p_percent) = 0;

    /* p_isNew is true if it's a new level, false if it's just an update */
    virtual void setBeingDownloadedLevel(const std::string &p_LevelName,bool p_isNew=true) = 0;

    virtual void readEvents() = 0;
    
    /* Level tools */
    virtual bool doesLevelExist(const std::string &p_LevelId) = 0;
  
    /* return the level file or "" if you won't the level be updated (if required) */
    virtual std::string levelPathForUpdate(const std::string &p_LevelId) = 0;

    /* return the CRC32 sum or "" if can't determine it */
    virtual std::string levelCRC32Sum(const std::string &p_LevelId) = 0;
    
    /* return the MD5 sum or "" not available */
    //virtual std::string levelMD5Sum(const std::string &LevelID) = 0;

    /* Data interface */
    void setCancelAsSoonAsPossible() {m_cancel_as_soon_as_possible = true;}
    void clearCancelAsSoonAsPossible() {m_cancel_as_soon_as_possible = false;}
    bool isCancelAsSoonAsPossible() {return m_cancel_as_soon_as_possible;}

  private:
    /* Data */
    bool m_cancel_as_soon_as_possible;
  };

};

#endif

#endif

