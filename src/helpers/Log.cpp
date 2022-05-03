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
#include "Time.h"
#include "VExcept.h"
#include "common/VFileIO.h"

#include "assert.h"
#include <cstdio>
#include <stdarg.h>

bool Logger::m_isInitialized = false;
bool Logger::m_activ = true;
bool Logger::m_verbose = false;
FILE *Logger::m_fd = NULL;
std::string Logger::m_logName;

void Logger::init() {
  assert(XMFS::isInitialized());

  m_verbose = false;

  m_logName = iso8601Date() + ".log";

  auto logDir = XMFS::getUserDir(FDT_CACHE) + "/logs";
  auto logFile = logDir + "/" + m_logName;
  XMFS::mkArborescenceDir(logDir);

  m_fd = fopen(logFile.c_str(), "w");
  if (m_fd == NULL) {
    throw Exception("Unable to open log file");
  }

  auto latestFile = "logs/latest.log";

  if (XMFS::fileExists(FDT_CACHE, latestFile))
    XMFS::deleteFile(FDT_CACHE, latestFile);

  XMFS::mklink(logFile, XMFS::getUserDir(FDT_CACHE) + "/" + latestFile);

  m_isInitialized = true;
}

void Logger::uninit() {
  fclose(m_fd);
  m_isInitialized = false;
}

bool Logger::isInitialized() {
  return m_isInitialized;
}

bool Logger::isVerbose() {
  return m_verbose;
}

void Logger::setVerbose(bool i_value) {
  m_verbose = i_value;
}

void Logger::setActiv(bool i_value) {
  m_activ = i_value;
}

void Logger::LogRaw(const std::string &s) {
  if (m_activ == false) {
    return;
  }

  fprintf(m_fd, "%s\n", s.c_str());
  fflush(m_fd);

  if (m_verbose) {
    printf("%s\n", s.c_str());
  }
}

void Logger::LogLevelMsg(LogLevel i_level, const char *pcFmt, ...) {
  va_list List;
  char cBuf[4096];
  va_start(List, pcFmt);
  vsnprintf(cBuf, 4096, pcFmt, List);
  va_end(List);

  switch (i_level) {
    case LOG_ERROR:
      LogRaw(std::string("** Error ** : ") + cBuf);
      break;
    case LOG_WARNING:
      LogRaw(std::string("** Warning ** : ") + cBuf);
      break;
    case LOG_INFO:
      LogRaw(cBuf);
      break;
    case LOG_DEBUG:
      LogRaw(std::string("** Debug ** : ") + cBuf);
      break;
  }
}

void Logger::LogData(void *data, unsigned int len) {
  if (m_activ == false) {
    return;
  }

  fprintf(m_fd, "=== Packet [%u]: ===\n", len);
  fflush(m_fd);
  fwrite(data, len, 1, m_fd);
  fprintf(m_fd, "====================\n");
}

void Logger::deleteLegacyLog() {
  if (XMFS::fileExists(FDT_CACHE, "xmoto.log"))
    XMFS::deleteFile(FDT_CACHE, "xmoto.log");
}

int Logger::deleteOldFiles() {
  int deleted = 0;
  std::vector<std::string> files = XMFS::findPhysFiles(FDT_CACHE, "logs/*.log");
  for (auto &file : files) {
    file = XMFS::getFileBaseName(file, true);

    if (file == m_logName || file == "latest.log")
      continue;

    XMFS::deleteFile(FDT_CACHE, "logs/" + file);
    deleted++;
  }

  return deleted;
}
