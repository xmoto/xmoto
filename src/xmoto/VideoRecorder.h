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

#define VR_DEFAULT_FRAMERATE 20
#define VR_DEFAULT_DIVISION 2

#include <string>

class VideoRecorder {
public:
  VideoRecorder(const std::string &i_videoName,
                int i_division = VR_DEFAULT_DIVISION,
                int i_frameRate = VR_DEFAULT_FRAMERATE);
  ~VideoRecorder();

  void read(int i_time);

private:
  std::string m_name;
  int m_division;
  std::string m_directory;
  int m_framerate;
  int m_nbFrames;

  FILE *m_fd;
};
