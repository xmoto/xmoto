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

/* 
 *  Game main.
 */
#include <stdio.h>
#include <stdlib.h>

#include "Game.h"
#include "VFileIO.h"

using namespace vapp;

/*===========================================================================
SDL main entry point
===========================================================================*/
#if defined(_MSC_VER)
int SDL_main(int nNumArgs,char **ppcArgs) {
#else
int main(int nNumArgs,char **ppcArgs) {
#endif
  /* Start application */
  try {  
    /* Init file system stuff */
    FS::init( "xmoto" );
    
    /* Setup basic info */
    GameApp Game;
    
    Game.setAppName(std::string("X-Moto"));
    Game.setAppCommand(std::string("xmoto"));
    Game.setCopyrightInfo(std::string("(C) Copyright 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)"));
    Game.run(nNumArgs,ppcArgs);
  }
  catch (Exception &e) {
    FS::writeLog(std::string("Exception: ") + e.getMsg());
  
    printf("fatal exception : %s\n",e.getMsg().c_str());        
    SDL_Quit(); /* make sure SDL shuts down gracefully */

    #if defined(WIN32)
      char cBuf[1024];
      sprintf(cBuf,"Fatal exception occured: %s\n"
                   "Consult the file xmoto.log for more information about what\n"
                   "might has occured.\n",e.getMsg().c_str());                    
      MessageBox(NULL,cBuf,"X-Moto Error",MB_OK|MB_ICONERROR);
    #endif
  }
  return 0;
}

