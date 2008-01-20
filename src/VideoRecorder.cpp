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

#include "VideoRecorder.h"
#include "VFileIO.h"
#include "Image.h"
#include "drawlib/DrawLib.h"
#include "Game.h"
#include "helpers/Log.h"

VideoRecorder::VideoRecorder(const std::string& i_videoName, int i_division, int i_frameRate) {
    m_name      = i_videoName;
    m_division  = i_division;
    m_framerate = i_frameRate;
    m_lastRead  = -1.0;
    m_nbFrames  = 0;

    m_directory = FS::getUserDir() + "/Videos/" + i_videoName;

    if(FS::isDir(m_directory) == false) {
      FS::mkArborescenceDir(m_directory);
    }

    Logger::Log("Video recording:");
    Logger::Log("find %s | sort > /tmp/list.lst && transcode -i /tmp/list.lst -x imlist,null -y xvid,null -f %i -g %ix%i --use_rgb -z -o %s/movie.avi -H 0",
		m_directory.c_str(),
		m_framerate,
		GameApp::instance()->getDrawLib()->getDispWidth()/i_division,
		GameApp::instance()->getDrawLib()->getDispHeight()/i_division,
		std::string(FS::getUserDir() + "/Videos/").c_str()
		);

}

VideoRecorder::~VideoRecorder() {
}

void VideoRecorder::read(float i_time) {
  Img *pShot;
  std::string v_frameName;
  char vName[9];

  if( (m_lastRead < 0.0 || i_time - m_lastRead >= 1.0/((float)m_framerate)) == false) {
    return;
  }
  m_lastRead = i_time;

  // frame name
  snprintf(vName, 9, "%08i", m_nbFrames);
  v_frameName = std::string(vName) + ".jpg";

  // take the screenshot
  pShot = GameApp::instance()->getDrawLib()->grabScreen(m_division); 
  pShot->saveFile(m_directory + "/" + v_frameName);
  delete pShot;
  m_nbFrames++;
}
