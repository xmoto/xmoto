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

#ifndef __XMDATABASE_H__
#define __XMDATABASE_H__

#include <sqlite3.h>
#include <string>
#include "xmDatabaseUpdateInterface.h"

namespace vapp {
  class UIWindow;
  class UIFont;
}

class xmDatabase {

  public:
  xmDatabase(std::string i_dbFile, XmDatabaseUpdateInterface *i_interface = NULL);
  ~xmDatabase();


  /* stats */
  void stats_levelCompleted(const std::string &PlayerName,
			    const std::string &LevelID,
			    float fPlayTime);
  void stats_died(const std::string &PlayerName,
		  const std::string &LevelID,
		  float fPlayTime);
  void stats_abortedLevel(const std::string &PlayerName,
			  const std::string &LevelID,
			  float fPlayTime);
  void stats_levelRestarted(const std::string &PlayerName,
			    const std::string &LevelID,
			    float fPlayTime);
  void stats_xmotoStarted(const std::string &PlayerName);
  
  vapp::UIWindow *stats_generateReport(const std::string &PlayerName, vapp::UIWindow *pParent,
				       int x, int y, int nWidth, int nHeight, vapp::UIFont *pFont);

  private:
  sqlite3 *m_db;

  /* function used to synchronise with the last xmoto version */
  int  getXmDbVersion();
  void upgrateXmDbToVersion(int i_fromVersion, XmDatabaseUpdateInterface *i_interface = NULL);
  void updateXmDbVersion(int i_newVersion);

  /* trace */
  static void sqlTrace(void*, const char* sql);

  /* i_sql must be of form select count() from */
  bool checkKey(std::string i_sql);
  void simpleSql(std::string i_sql);

  /* conversion of old files */
  void updateDB_stats(XmDatabaseUpdateInterface *i_interface = NULL); /* statistics */
  bool stats_checkKeyExists_stats_profiles(std::string i_profile);
  bool stats_checkKeyExists_stats_profiles_levels(std::string i_profile, std::string i_level);

  void debugResult(char **i_result, int ncolumn, int nrow);
};

#endif
