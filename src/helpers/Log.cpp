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
#include "assert.h"
#include <stdarg.h>

bool  Logger::m_isInitialized = false;
bool  Logger::m_verbose       = false;
FILE* Logger::m_fd            = NULL;

void Logger::init(const std::string& i_logFile) {
  assert(FS::isInitialized());

  m_verbose = false;

  if(FS::fileExists(i_logFile)) {
    FS::deleteFile(i_logFile);
  }

  m_fd = fopen(i_logFile.c_str(), "w");
  if(m_fd == NULL) {
    throw Exception("Unable to open log file");
  }

  m_isInitialized = true;
}

void Logger::uninit() {
  fclose(m_fd);
  m_isInitialized = false;
}

bool Logger::isInitialized() {
  return m_isInitialized;
}

void Logger::setVerbose(bool i_value) {
  m_verbose = i_value;
}

void Logger::LogRaw(const std::string &s) {
  fprintf(m_fd, "%s\n", s.c_str());
  fflush(m_fd);
  
  if(m_verbose) {
    printf("%s\n", s.c_str());
  }
}

void Logger::LogLevelMsg(LogLevel i_level, const char *pcFmt, ...) {
  va_list List;
  char cBuf[4096];
  va_start(List, pcFmt);
  vsnprintf(cBuf, 4096, pcFmt, List);
  va_end(List);
  
  switch(i_level) {
  case LOG_ERROR:
    LogRaw(std::string("** Error ** : ")   + cBuf);
    break;
  case LOG_WARNING:
    LogRaw(std::string("** Warning ** : ") + cBuf);
    break;
  case LOG_INFO:
    LogRaw(cBuf);
    break;
  case LOG_DEBUG:
    LogRaw(std::string("** Debug ** : ")   + cBuf);
    break;
  }

}
