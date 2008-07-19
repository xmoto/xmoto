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

#include "System.h"
#include "VCommon.h"
#include "Log.h"
#include "VExcept.h"

std::vector<std::string>* System::getDisplayModes(int windowed) {
    std::vector<std::string>* modes = new std::vector<std::string>;
    SDL_Rect **sdl_modes;
    int i, nFlags;
    
    /* Always use the fullscreen flags to be sure to
       always get a result (no any modes available like in windowed) */
    nFlags = SDL_OPENGL | SDL_FULLSCREEN;
    
    /* Get available fullscreen/hardware modes */
    sdl_modes = SDL_ListModes(NULL, nFlags);
    
    /* Check is there are any modes available */
    if(sdl_modes == (SDL_Rect **)0){
       LogInfo("** Warning ** : No display modes available.");
      throw Exception("getDisplayModes : No modes available.");
    }
    
    /* Always include these to modes */
    modes->push_back("800 X 600");
    modes->push_back("1024 X 768");
    modes->push_back("1280 X 1024");
    modes->push_back("1600 X 1200");
    
    /* Print valid modes */
    //Log("Available Modes :");
    
    for(i=0; sdl_modes[i]; i++){
      char tmp[128];
      
      /* Menus don't fit under 800x600 */
      if(sdl_modes[i]->w < 800 || sdl_modes[i]->h < 600)
	continue;
      
      snprintf(tmp, 126, "%d X %d",
	       sdl_modes[i]->w,
	       sdl_modes[i]->h);
      tmp[127] = '\0';
      
      /* Only single */
      bool findDouble = false;
      //Log("size: %d", modes->size());
      for(unsigned int j=0; j<modes->size(); j++)
	if(!strcmp(tmp, (*modes)[j].c_str())){
	  findDouble = true;
	  break;
	}
      
      if(!findDouble){
	modes->push_back(tmp);
	//Log("  %s", tmp);
      }
    }
    
    return modes;
}
