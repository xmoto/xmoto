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

#ifndef __SENDVOTETHREAD_H__
#define __SENDVOTETHREAD_H__

#include "XMThread.h"
#include "common/WWWAppInterface.h"

class xmDatabase;

class SendVoteThread
  : public XMThread
  , public WWWAppInterface {
public:
  SendVoteThread(const std::string &i_idlevel,
                 const std::string &i_difficulty_value,
                 const std::string &i_quality_value,
                 bool i_adminMode,
                 const std::string &i_id_profile,
                 const std::string &i_password);
  virtual ~SendVoteThread();
  std::string getMsg() const;

  void setTaskProgress(float p_percent);

  virtual int realThreadFunction();

  static bool isToPropose(xmDatabase *pDb,
                          const std::string &i_id_level); // return false if
  // it's not necessary
  // to run it

private:
  std::string m_msg;
  std::string m_idlevel, m_difficulty_value, m_quality_value;
  bool m_adminMode;
  std::string m_id_profile;
  std::string m_password;

  static bool hasPlayedEnough(xmDatabase *pDb, const std::string &i_id_level);
};

#endif
