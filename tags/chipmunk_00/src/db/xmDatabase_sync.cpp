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

#include "xmDatabase.h"
#include "VFileIO.h"
#include "VXml.h"
#include "helpers/VExcept.h"

void xmDatabase::sync_buildServerFile(const std::string& i_outFile, const std::string& i_sitekey, const std::string& i_profile) {
    char **v_result;
    unsigned int nrow;
    std::string v_res;
    char v_line[2048];

    FileHandle *pfh = FS::openOFile(i_outFile);

    if(pfh == NULL) {
      throw Exception("Unable to open " + i_outFile);
    }

    FS::writeLine(pfh,"<?xml version=\"1.0\" encoding=\"utf-8\"?>");

    /* header */
    v_result = readDB("SELECT nbStarts, since "
		      "FROM stats_profiles "
		      "WHERE sitekey=\"" + protectString(i_sitekey) + "\" AND id_profile=\"" + protectString(i_profile) + "\";", nrow);
    if(nrow != 1) {
      read_DB_free(v_result);
      throw Exception("Unable to retrieve informations");
    }
    snprintf(v_line, 2048,
	     "<xmoto_sync fileformat=\"1\" sitekey=\"%s\" profile=\"%s\" nbStarts=\"%i\" since=\"%s\">",
	     XML::str2xmlstr(i_sitekey).c_str(), XML::str2xmlstr(i_profile).c_str(),
	     atoi(getResult(v_result, 2, 0, 0)), getResult(v_result, 2, 0, 1));
    FS::writeLine(pfh, v_line);
    read_DB_free(v_result);
 
    /* stats_levels */
    FS::writeLine(pfh, "<stats_levels>");
    v_result = readDB("SELECT id_level, nbPlayed, nbDied, nbCompleted, nbRestarted, playedTime, last_play_date "
		      "FROM stats_profiles_levels "
		      "WHERE sitekey=\"" + protectString(i_sitekey) + "\" AND id_profile=\"" + protectString(i_profile) + "\" "
		      "AND synchronized=0;", nrow);
    for(unsigned int i=0; i<nrow; i++) {
      snprintf(v_line, 2048, "<stats_level id_level=\"%s\" nbPlayed=\"%i\" nbDied=\"%i\" nbCompleted=\"%i\" nbRestarted=\"%i\" playedTime=\"%i\" last_play_date=\"%s\" />",
	       XML::str2xmlstr(getResult(v_result, 7, i, 0)).c_str(),
	       atoi(getResult(v_result, 7, i, 1)), atoi(getResult(v_result, 7, i, 2)),
	       atoi(getResult(v_result, 7, i, 3)), atoi(getResult(v_result, 7, i, 4)),
	       atoi(getResult(v_result, 7, i, 5)), getResult(v_result, 7, i, 6) == NULL ? "" : getResult(v_result, 7, i, 6));
      FS::writeLine(pfh, v_line);
    }
    FS::writeLine(pfh, "</stats_levels>");

    /* stats_completedLevels */
    FS::writeLine(pfh, "<stats_completedLevels>");
    v_result = readDB("SELECT id_level, timeStamp, finishTime "
		      "FROM profile_completedLevels "
		      "WHERE sitekey=\"" + protectString(i_sitekey) + "\" AND id_profile=\"" + protectString(i_profile) + "\" "
		      "AND synchronized=0;", nrow);
    for(unsigned int i=0; i<nrow; i++) {
      snprintf(v_line, 2048, "<stats_completedLevel id_level=\"%s\" timeStamp=\"%s\" finishTime=\"%i\" />",
	       XML::str2xmlstr(getResult(v_result, 3, i, 0)).c_str(),
	       getResult(v_result, 3, i, 1), atoi(getResult(v_result, 3, i, 2)));
      FS::writeLine(pfh, v_line);
    }
    FS::writeLine(pfh, "</stats_completedLevels>");

    FS::writeLine(pfh, "</xmoto_sync>");

    FS::closeFile(pfh);
  }
