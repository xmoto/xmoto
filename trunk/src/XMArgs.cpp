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
  m_opt_pack   	      = false;
  m_opt_unpack 	      = false;
  m_unpack_noList     = false;
  m_opt_nogfx         = false;
  m_opt_res 	      = false;
  m_res_dispWidth     = 0;
  m_res_dispHeight    = 0;
  m_opt_bpp           = false;
  m_bpp_value 	      = 0;
  m_opt_fs            = false;
  m_opt_win           = false;
  m_opt_noexts        = false;
  m_opt_drawlib       = false;
  m_opt_ugly          = false;
  m_opt_nowww         = false;
  m_opt_help          = false;
  m_opt_verbose       = false;
  m_opt_debug         = false;
  m_opt_sqlTrace      = false;
  m_opt_fps           = false;
  m_opt_replay        = false;
  m_opt_listReplays   = false;
  m_opt_replayInfos   = false;
  m_opt_levelID       = false;
  m_opt_levelFile     = false;
  m_opt_listLevels    = false;
  m_opt_profile       = false;
  m_opt_timedemo      = false;
  m_opt_testTheme     = false;
  m_opt_benchmark     = false;
  m_opt_cleanCache    = false;
}

void XMArguments::parse(int i_argc, char **i_argv) {
  std::string v_opt;
  std::string v_arg;

  int i = 1;
  while(i<i_argc) {
    v_opt = i_argv[i];
    
    if(v_opt == "-pack") {
      m_opt_pack = true;
      if(i+1 < i_argc) {
	m_pack_bin = i_argv[i+1];
	i++;
      }
      if(i+1 < i_argc) {
	m_pack_dir = i_argv[i+1];
	i++;
      }
    } else if(v_opt == "-unpack") {
      m_opt_unpack = true;
      if(i+1 < i_argc) {
	m_unpack_bin = i_argv[i+1];
	i++;
      }
      if(i+1 < i_argc) {
	m_unpack_dir = i_argv[i+1];
	i++;
      }
      if(i+1 < i_argc) {
	v_arg = i_argv[i+1];
	if(v_arg != "no_list") {
	  throw Exception("only no_list is allow here");
	}
	m_unpack_noList = true;
	i++;
      }      
    } else if(v_opt == "-nogfx") {
      m_opt_nogfx = true;
    } else if(v_opt == "-v") {
      m_opt_verbose = true;
    } else if(v_opt == "-nogfx") {
      m_opt_nogfx = true;
    } else if(v_opt == "-res") {
      m_opt_res = true;
      if(i+1 >= i_argc) {
	throw SyntaxError("missing resolution");
      }
      sscanf(i_argv[i+1], "%ix%i", &m_res_dispWidth, &m_res_dispHeight);
      i++;
    } else if(v_opt == "-bpp") {
      m_opt_bpp = true;
      if(i+1 >= i_argc) {
	throw SyntaxError("missing bit depth");
      }
      m_bpp_value = atoi(i_argv[i+1]);
      i++;
    } else if(v_opt == "-win") {
      m_opt_win = true;
    } else if(v_opt == "-fs") {
      m_opt_fs = true;
    } else if(v_opt == "-noexts") {
      m_opt_noexts = true;
    } else if(v_opt == "-drawlib") {
      m_opt_drawlib = true;
      if(i+1 >= i_argc) {
	throw SyntaxError("missing drawlib name");
      }
      m_drawlib_lib = i_argv[i+1];
      i++;
    } else if(v_opt == "-h" || v_opt == "-?" || v_opt == "--help" || v_opt == "-help" ) {
      m_opt_help = true;
    } else if(v_opt == "-nowww") {
      m_opt_nowww = true;
    }  else {
      //throw Exception("Invalid option \"" + v_opt + "\"");
    }

    i++;
  }

  //throw Exception("To do");
}

//      else if(!strcmp(ppcArgs[i],"-noexts")) {
//	m_useGlExtension = false;
//      }
//      else if(!strcmp(ppcArgs[i],"-nowww")) {
//        m_bNoWWW = true;
//      } else if(!strcmp(ppcArgs[i],"-drawlib")) {
//        if(i+1 == nNumArgs) {
//          throw SyntaxError("missing drawlib");
//	}
//	m_CmdDrawLibName = ppcArgs[i+1];
//        i++;
//      } else if(!strcmp(ppcArgs[i],"-h") || !strcmp(ppcArgs[i],"-?") ||
//              !strcmp(ppcArgs[i],"--help") || !strcmp(ppcArgs[i],"-help")) {
//        printf("%s (Version %s)\n",m_AppName.c_str(), XMBuild::getVersionString().c_str());
//
//        helpUserArgs();
//        printf("\n");
//        
//	/* mark that we want to quit the application */
//	m_bQuit = true;
//        return;
//      }
//      else {
//        /* Add it to argument vector */
//        UserArgs.push_back(ppcArgs[i]);
//      }
//    }

bool XMArguments::isOptPack() const {
  return m_opt_pack;
}

std::string XMArguments::getOpt_pack_bin() const {
  return m_pack_bin;
}

std::string XMArguments::getOpt_pack_dir() const {
  return m_pack_dir;
}

bool XMArguments::isOptUnPack() const {
  return m_opt_unpack;
}

std::string XMArguments::getOpt_unpack_bin() const {
  return m_unpack_bin;
}

std::string XMArguments::getOpt_unpack_dir() const {
  return m_unpack_dir;
}

bool XMArguments::getOpt_unpack_noList() const {
  return m_unpack_noList;
}

bool XMArguments::isOptVerbose() const {
  return m_opt_verbose;
}

bool XMArguments::isOptNoGfx() const {
  return m_opt_nogfx;
}

bool XMArguments::isOptRes() const {
  return m_opt_res;
}

int XMArguments::getOpt_res_dispWidth() const {
  return m_res_dispWidth;
}

int XMArguments::getOpt_res_dispHeight() const {
  return m_res_dispHeight;
}

bool XMArguments::isOptBpp() const {
  return m_opt_bpp;
}

int XMArguments::getOpt_bpp_value() const {
  return m_bpp_value;
}

bool XMArguments::isOptWindowed() const {
  return m_opt_win;
}

bool XMArguments::isOptFs() const {
  return m_opt_fs;
}

bool XMArguments::isOptNoExts() const {
  return m_opt_noexts;
}

bool XMArguments::isOptDrawlib() const {
  return m_opt_drawlib;
}

std::string XMArguments::getOpt_drawlib_lib() const {
  return m_drawlib_lib;
}

bool XMArguments::isOptHelp() const {
  return m_opt_help;
}

bool XMArguments::isOptNoWWW() const {
  return m_opt_nowww;
}

void XMArguments::help(const std::string& i_cmd) {
  printf("X-Moto %s\n", XMBuild::getVersionString().c_str());
  printf("%s\n", XMBuild::getCopyRight().c_str());
  printf("usage:  %s {options}\n"
	 "options:\n", i_cmd.c_str());
        
  printf("\t-level ID\n\t\tStart playing the given level right away.\n");
  printf("\t-replay NAME\n\t\tPlayback replay with the given name.\n");    
  printf("\t-debug\n\t\tEnable debug mode.\n");
  printf("\t-profile NAME\n\t\tUse this player profile.\n");
  printf("\t-listlevels\n\t\tOutputs a list of all installed levels.\n");
  printf("\t-listreplays\n\t\tOutputs a list of all replays.\n");
  printf("\t-timedemo\n\t\tNo delaying, maximum framerate.\n");
  printf("\t-fps\n\t\tDisplay framerate.\n");
  printf("\t-ugly\n\t\tEnable 'ugly' mode, suitable for computers without\n");
  printf("\t\ta good OpenGL-enabled video card.\n");
  printf("\t-testTheme\n\t\tDisplay forms around the theme to check it.\n");
  printf("\t-benchmark\n\t\tOnly meaningful when combined with -replay\n");
  printf("\t\tand -timedemo. Useful to determine the graphics\n");
  printf("\t\tperformance.\n");
  printf("\t-cleancache\n\t\tDeletes the content of the level cache.\n");
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
