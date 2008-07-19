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

#ifndef __LOG_H__
#define __LOG_H__

#include <string>

#define LOG_ERROR       1
#define LOG_WARNING     2
#define LOG_INFO        3
#define LOG_DEBUG       4

#define LogError(format, ...)                                        Logger::LogLevel(LOG_ERROR,   format, ## __VA_ARGS__);
#define LogWarning(format, ...)                                      Logger::LogLevel(LOG_WARNING, format, ## __VA_ARGS__);
#define LogInfo(format, ...)                                         Logger::LogLevel(LOG_INFO,    format, ## __VA_ARGS__);

// a class using LogDebug must be aware of XMSession
#define LogDebug(format, ...)   if(XMSession::instance()->debug()) { Logger::LogLevel(LOG_DEBUG,   format, ## __VA_ARGS__); }

class Logger {
  public:
  static void init(const std::string& i_logFile);
  static void uninit();
  static bool isInitialized();

  static void setVerbose(bool i_value);
  static void LogLevel(int i_level, const char *pcFmt, ...);

  private:
  static bool  m_isInitialized;
  static bool  m_verbose;
  static FILE* m_fd;

  static void LogRaw(const std::string &s);
};

#endif
