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

#ifndef __XMARGS_H__
#define __XMARGS_H__

#include <string>

class XMArguments {
  public:
  XMArguments();
  void parse(int i_argc, char **i_argv);
  void help(const std::string& i_cmd);

  private:
  /* pack options */
  bool m_opt_pack;
  bool m_opt_unpack;
  std::string m_pack_Bin;
  std::string m_pack_Dir;
  bool m_opt_pack_NoList;

  /* graphics */
  bool m_opt_nogfx;
  bool m_opt_res;
  int m_res_dispWidth;
  int m_res_dispHeight;
  bool m_opt_bpp;
  int m_bpp_value;
  bool m_opt_fs;
  bool m_opt_win;
  bool m_opt_noext;
  bool m_opt_drawlib;
  std::string m_drawlib_lib;
  bool m_opt_ugly;

  /* web */
  bool m_opt_nowww;

  /* extra */
  bool m_opt_help;
  bool m_opt_verbose;
  bool m_opt_debug;
  bool m_opt_sqlTrace;
  bool m_opt_fps;
  
  /* replays */
  bool m_opt_replay;
  std::string m_replay_File;
  bool m_opt_listReplays;
  bool m_opt_replayInfos;
  std::string m_replayInfos_File;

  /* levels */
  bool m_opt_levelID;
  std::string m_levelID_Id;
  bool m_opt_levelFile;
  std::string m_levelFile_File;
  bool m_opt_listLevels;

  /* profile */
  bool m_opt_profile;
  std::string m_profile_value;

  /* game */
  bool m_opt_timedemo;
  bool m_opt_testTheme;
  bool m_opt_benchmark;
  bool m_opt_cleanCache;

};

#endif

