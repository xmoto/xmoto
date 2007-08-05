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

#include "XMArgs.h"
#include "helpers/VExcept.h"
#include "XMBuild.h"

XMArguments::XMArguments() {
  m_opt_pack   	    = false;
  m_opt_unpack 	    = false;
  m_opt_pack_NoList = false;
  m_opt_nogfx       = false;
  m_opt_res 	    = false;
  m_res_dispWidth   = 0;
  m_res_dispHeight  = 0;
  m_opt_bpp         = false;
  m_bpp_value 	    = 0;
  m_opt_fs          = false;
  m_opt_win         = false;
  m_opt_noext       = false;
  m_opt_drawlib     = false;
  m_opt_ugly        = false;
  m_opt_nowww       = false;
  m_opt_help        = false;
  m_opt_verbose     = false;
  m_opt_debug       = false;
  m_opt_sqlTrace    = false;
  m_opt_fps         = false;
  m_opt_replay      = false;
  m_opt_listReplays = false;
  m_opt_replayInfos = false;
  m_opt_levelID     = false;
  m_opt_levelFile   = false;
  m_opt_listLevels  = false;
  m_opt_profile     = false;
  m_opt_timedemo    = false;
  m_opt_testTheme   = false;
  m_opt_benchmark   = false;
  m_opt_cleanCache  = false;
}

void XMArguments::parse(int i_argc, char **i_argv) {
  //throw Exception("To do");
}

void XMArguments::help(const std::string& i_cmd) {
  printf("X-Moto %s\n", XMBuild::getVersionString().c_str());
  printf("%s\n", XMBuild::getCopyRight().c_str());
  printf("usage:  %s {options}\n"
	 "options:\n", i_cmd.c_str());
        
  printf("\t-res WIDTHxHEIGHT\n\t\tSpecifies display resolution to use.\n");
  printf("\t-bpp BITS\n\t\tTry to use this display color bit depth.\n");
  printf("\t-fs\n\t\tForces fullscreen mode.\n");
  printf("\t-win\n\t\tForces windowed mode.\n");
  printf("\t-nogfx\n\t\tDon't show any graphical elements.\n");
  printf("\t-q\n\t\tDon't print messages to screen, and don't save them in the log.\n");
  printf("\t-v\n\t\tBe verbose.\n");
  printf("\t-noexts\n\t\tDon't use any OpenGL extensions.\n");
  printf("\t-nowww\n\t\tDon't allow xmoto to connect on the web.\n");
  //If both sdlgfx and opengl are available give the user the posibility
  //to select the draw library
#ifdef ENABLE_SDLGFX
#ifdef ENABLE_OPENGL
  printf("\t-drawlib [OPENGL|SDLGFX]\n\t\tSelect the draw library to use.\n");
#endif
#endif
}
