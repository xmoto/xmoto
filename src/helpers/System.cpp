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
#include "Log.h"
#include "VExcept.h"
#include "common/VCommon.h"
#include "include/xm_SDL.h"
#include <sstream>

#if !(defined(WIN32) || defined(__APPLE__))
#include <sys/types.h>
#include <unistd.h>
#endif

std::vector<std::string> *System::getDisplayModes(int windowed) {
  std::vector<std::string> *strModes = new std::vector<std::string>({
    /* Always include these in the modes */
    "800 X 600", "1024 X 768", "1280 X 1024", "1600 X 1200",
  });

  /* Always use the fullscreen flags to be sure to
     always get a result (no any modes available like in windowed) */
  //nFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN;

  const int displayIndex = 0;
  int displayModeCount = 0;
  if ((displayModeCount = SDL_GetNumDisplayModes(displayIndex)) < 1) {
    throw Exception("getDisplayModes: No display modes found.");
  }
  std::vector<SDL_DisplayMode> modes(displayModeCount);

  for (int modeIndex = 0; modeIndex < displayModeCount; ++modeIndex) {
    SDL_DisplayMode mode;
    if (SDL_GetDisplayMode(displayIndex, modeIndex, &mode) != 0) {
      throw Exception("getDisplayModes: SDL_GetDisplayMode failed: "
          + std::string(SDL_GetError()));
    }
    modes[modeIndex] = mode;
  }

  /* Get available fullscreen/hardware modes */
  //sdlModes = SDL_ListModes(NULL, nFlags);

  /* Check is there are any modes available */
  /*
  if (sdlModes == (SDL_Rect **)0) {
    LogWarning("No display modes available.");
    throw Exception("getDisplayModes : No modes available.");
  }
  */

  /* Create a string-list of the display modes */
  for (auto &mode : modes) {
    char tmp[128];

    /* Menus don't fit under 800x600 */
    if (mode.w < 800 || mode.h < 600)
      continue;

    snprintf(tmp, 126, "%d X %d", mode.w, mode.h);
    tmp[127] = '\0';

    /* Only add if not a duplicate */
    bool isDuplicate = false;
    for (auto &mode : *strModes) {
      if (!strncmp(tmp, mode.c_str(), mode.length())) {
        isDuplicate = true;
        break;
      }
    }
    if (!isDuplicate) {
      strModes->push_back(tmp);
    }
  }

  return strModes;
}

std::string System::getMemoryInfo() {
  std::string v_res;

// note that apple don't know getline
#if defined(WIN32) || defined(__APPLE__)
  return "No available information";
#else
  // grep -E '^Vm' /proc/2532/status
  // VmSize:	  403576 kB
  // VmLck:	       0 kB
  // VmRSS:	  140996 kB
  // VmData:	  285376 kB
  // VmStk:	     212 kB
  // VmExe:	      48 kB
  // VmLib:	   53016 kB
  // VmPTE:	     508 kB
  // VmSwap:	       0 kB

  pid_t v_pid;
  std::ostringstream v_pid_str;
  std::string v_memfile;
  FILE *fd;
  char *v_line;
  std::string v_str, v_var, v_value;
  size_t v_len;
  ssize_t v_read;
  // size_t n;

  v_pid = getpid();
  v_pid_str << v_pid;
  v_memfile = "/proc/" + v_pid_str.str() + "/status";

  if ((fd = fopen(v_memfile.c_str(), "r")) == NULL) {
    return "No available information";
  }

  v_line = NULL;
  v_len = 0;
  while ((v_read = getline(&v_line, &v_len, fd)) != -1) {
    v_str = v_line;
    if (v_str.substr(0, 2) == "Vm") {
      v_res += v_str;
    }

    //    // search the : delimiter
    //    if( (n = v_str.find(":")) != std::string::npos) {
    //      v_var   = v_str.substr(0, n);
    //      n++; // to remove the :
    //      // remove unnecessary spaces used in this file
    //      while(n < v_str.size() && (v_str[n] == '\t' || v_str[n] == ' ')) {
    //	n++;
    //      }
    //      if(n < v_str.size()) {
    //	// a string and a non empty value is found
    //	v_value = v_str.substr(n, v_str.length() - n -1 /* remove the \n */);
    //	printf("+var=%s+val=%s+\n", v_var.c_str(), v_value.c_str());
    //      }
    //    }
  }
  if (v_line != NULL) {
    free(v_line);
  }

  fclose(fd);
#endif

  return v_res;
}
