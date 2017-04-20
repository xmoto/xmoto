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
#include "common/VFileIO.h"
#include "common/Image.h"
#include "drawlib/DrawLib.h"
#include "Game.h"
#include "helpers/Log.h"

VideoRecorder::VideoRecorder(const std::string& i_videoName, int i_division, int i_frameRate) {
  	std::string v_listFile;

    m_name      = i_videoName;
    m_division  = i_division;
    m_framerate = i_frameRate;
    m_nbFrames  = 0;

    LogInfo("New video recorder: name=%s, division=%i, frame rate=%i", i_videoName.c_str(), i_division, i_frameRate);

    m_directory = XMFS::getUserDir(FDT_DATA) + "/Videos/" + i_videoName;
		v_listFile = m_directory + "/pictures.lst";

    if(XMFS::isDir(m_directory)) {
			throw Exception("Video directory already exists");
		}

		XMFS::mkArborescenceDir(m_directory);

    LogInfo("Video recording:");
    LogInfo("transcode -i %s -x imlist,null -y xvid,null -f %i -g %ix%i --use_rgb -z -o %s/%s.avi -H 0 # -w 500",
  	v_listFile.c_str(),
		m_framerate,
		GameApp::instance()->getDrawLib()->getDispWidth()/i_division,
		GameApp::instance()->getDrawLib()->getDispHeight()/i_division,
  	std::string(XMFS::getUserDir(FDT_DATA) + "/Videos").c_str(),
  	m_name.c_str()
		);

		m_fd = fopen(v_listFile.c_str(), "w");
		if(m_fd == NULL) {
			throw Exception("Unable to open file " + v_listFile);
		}
}

VideoRecorder::~VideoRecorder() {
	fclose(m_fd);
}

void VideoRecorder::read(int i_time) {
  Img *pShot;
  std::string v_frameName;
  char vName[9];

  if( (i_time/100.0) * ((float) m_framerate) < ((float)m_nbFrames)) {
    return;
  }

  // frame name
  snprintf(vName, 9, "%08i", m_nbFrames);
  v_frameName = m_directory + "/" + std::string(vName) + ".jpg";

  // take the screenshot
  pShot = GameApp::instance()->getDrawLib()->grabScreen(m_division); 
  pShot->saveFile(v_frameName);
  delete pShot;

	fprintf(m_fd, "%s\n", v_frameName.c_str());
  m_nbFrames++;
}
