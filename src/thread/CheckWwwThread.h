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

#ifndef __CHECKWWWTHREAD_H__
#define __CHECKWWWTHREAD_H__

#include "XMThread.h"
#include "common/WWWAppInterface.h"

class WebRoom;
class WebLevels;

class CheckWwwThread
  : public XMThread
  , public WWWAppInterface {
public:
  // when forceUpdate is false, use the values in xmsession
  // to know which part has to be updated
  CheckWwwThread(bool forceUpdate = false);
  virtual ~CheckWwwThread();
  std::string getMsg() const;

  void setTaskProgress(float p_percent);

  virtual int realThreadFunction();

  static bool isNeeded(); // return false if it's not necessary to run it

private:
  void updateWebHighscores(const std::string &i_id_room);
  void upgradeWebHighscores(const std::string &i_id_room);
  void updateWebLevels();

  std::string m_msg;
  bool m_forceUpdate;
  WebRoom *m_pWebRoom;
  WebLevels *m_pWebLevels;
  bool m_realHighscoresUpdate;
};

#endif
