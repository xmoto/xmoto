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

#include "SendVoteThread.h"
#include "common/WWW.h"
#include "common/XMSession.h"
#include "db/xmDatabase.h"
#include "helpers/Log.h"
#include "helpers/Random.h"
#include "xmoto/GameText.h"

#define VOTE_ASK_FREQUENCY 10 // /1000 = 1%
#define PLAYED_ENOUGH_TIME 3000 // at least 30 seconds

SendVoteThread::SendVoteThread(const std::string &i_idlevel,
                               const std::string &i_difficulty_value,
                               const std::string &i_quality_value,
                               bool i_adminMode,
                               const std::string &i_id_profile,
                               const std::string &i_password)
  : XMThread("SVT") {
  m_idlevel = i_idlevel;
  m_difficulty_value = i_difficulty_value;
  m_quality_value = i_quality_value;
  m_adminMode = i_adminMode;
  m_id_profile = i_id_profile;
  m_password = i_password;
}

SendVoteThread::~SendVoteThread() {}

bool SendVoteThread::hasPlayedEnough(xmDatabase *pDb,
                                     const std::string &i_id_level) {
  char **v_result;
  unsigned int nrow;
  int v_playedTime;

  v_result =
    pDb->readDB("SELECT IFNULL(SUM(playedTime), 0) "
                "FROM stats_profiles_levels "
                "WHERE id_profile = \"" +
                  xmDatabase::protectString(XMSession::instance()->profile()) +
                  "\" "
                  "AND id_level=\"" +
                  xmDatabase::protectString(i_id_level) +
                  "\" "
                  "GROUP BY id_profile, id_level;",
                nrow);
  if (nrow != 1) {
    /* should not happend */
    pDb->read_DB_free(v_result);
    return false;
  }

  v_playedTime = atoi(pDb->getResult(v_result, 1, 0, 0));
  pDb->read_DB_free(v_result);

  return v_playedTime > PLAYED_ENOUGH_TIME;
}

bool SendVoteThread::isToPropose(xmDatabase *pDb,
                                 const std::string &i_id_level) {
  int v_rand;

  // in admin mode, you must ask it explicitly
  if (XMSession::instance()->adminMode()) {
    return false;
  }

  if (XMSession::instance()->www() == false ||
      XMSession::instance()->webForms() == false) {
    return false;
  }

  // ask only sometimes
  v_rand = (int)(1001.0 * (rand() / (RAND_MAX + 1.0))); // 101 according to
  // manpage to get a
  // number between 0 and
  // 1000
  if (v_rand > VOTE_ASK_FREQUENCY) {
    return false;
  }

  // must be a weblevels
  if (pDb->isOnTheWeb(i_id_level) == false) {
    return false;
  }

  // must be a weblevels
  if (pDb->isWebVoteLocked(i_id_level)) {
    return false;
  }

  // if the vote has already be done, don't revote
  if (pDb->isVoted(XMSession::instance()->profile(), i_id_level)) {
    return false;
  }

  // check that the level has been played a minimum
  if (hasPlayedEnough(pDb, i_id_level) == false) {
    return false;
  }

  return true;
}

int SendVoteThread::realThreadFunction() {
  bool v_msg_status_ok;

  setThreadCurrentOperation(GAMETEXT_SENDING_VOTE);
  setThreadProgress(0);

  try {
    FSWeb::sendVote(m_idlevel,
                    m_difficulty_value,
                    m_quality_value,
                    m_adminMode,
                    m_id_profile,
                    m_password,
                    DEFAULT_SENDVOTE_URL,
                    this,
                    XMSession::instance()->proxySettings(),
                    v_msg_status_ok,
                    m_msg);
    if (v_msg_status_ok == false) {
      return 1;
    }
    m_pDb->markAsVoted(XMSession::instance()->profile(), m_idlevel);
  } catch (Exception &e) {
    m_msg = GAMETEXT_UPLOAD_ERROR + std::string("\n") + e.getMsg();
    LogWarning("%s", e.getMsg().c_str());
    return 1;
  }
  setThreadProgress(100);

  return 0;
}

std::string SendVoteThread::getMsg() const {
  return m_msg;
}

void SendVoteThread::setTaskProgress(float p_percent) {
  setThreadProgress((int)p_percent);
}
