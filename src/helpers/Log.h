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

enum LogLevel { LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG };

// using ## gnu extension to allow empty args list
#define LogError(format, ...) \
  Logger::LogLevelMsg(LOG_ERROR, format, ##__VA_ARGS__);
#define LogWarning(format, ...) \
  Logger::LogLevelMsg(LOG_WARNING, format, ##__VA_ARGS__);
#define LogInfo(format, ...) \
  Logger::LogLevelMsg(LOG_INFO, format, ##__VA_ARGS__);

// a class using LogDebug must be aware of XMSession
#define LogDebug(format, ...)                              \
  if (XMSession::instance()->debug()) {                    \
    Logger::LogLevelMsg(LOG_DEBUG, format, ##__VA_ARGS__); \
  }

class Logger {
public:
  static void init(const std::string &i_logFile);
  static void uninit();
  static bool isInitialized();

  static bool isVerbose();
  static void setVerbose(bool i_value);
  static void setActiv(bool i_value);
  static void LogLevelMsg(LogLevel i_level, const char *pcFmt, ...);
  static void LogData(void *data, unsigned int len);

private:
  static bool m_isInitialized;
  static bool m_verbose;
  static bool m_activ;
  static FILE *m_fd;

  static void LogRaw(const std::string &s);
};

#endif
