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

#include "Log.h"
#include "VExcept.h"
#include "../VFileIO.h"

bool  Logger::m_verbose = false;
FILE* Logger::m_fd      = NULL;

void Logger::init(const std::string& i_logFile) {
  m_verbose = false;

  if(vapp::FS::fileExists(i_logFile)) {
    vapp::FS::deleteFile(i_logFile);
  }

  m_fd = fopen(i_logFile.c_str(), "w");
  if(m_fd == NULL) {
    throw Exception("Unable to open log file");
  }
}

void Logger::uninit() {
  fclose(m_fd);
}

void Logger::setVerbose(bool i_value) {
  m_verbose = i_value;
}

void Logger::LogRaw(const std::string &s) {
  fprintf(m_fd, "%s\n", s.c_str());
  
  if(m_verbose) {
    printf("%s\n", s.c_str());
  }
}
  
void Logger::Log(const char *pcFmt, ...) {
  va_list List;
  char cBuf[1024];
  va_start(List, pcFmt);
  vsnprintf(cBuf, 1024, pcFmt, List);
  va_end(List);
  
  LogRaw(cBuf);    
}
