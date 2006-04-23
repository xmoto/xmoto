/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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

/* file contributed by Nicolas Adenis-Lamarre */

#ifndef WEBHIGHSCORES
#define WEBHIGHSCORES

#include <string>

#if defined(_MSC_VER)
  #include <vector>
#endif

#define DEFAULT_WEBHIGHSCORES_URL      "http://aspegic500.free.fr/xmoto_highscores/highscores.xml"
#define DEFAULT_WEBHIGHSCORES_FILENAME "webhighscores.xml"
#define DEFAULT_WEBHIGHSCORES_TIMEOUT  15

#define DEFAULT_WEBLEVELS_URL      "http://aspegic500.free.fr/xmoto_highscores/levels.xml"
#define DEFAULT_WEBLEVELS_FILENAME "weblevels.xml"
#define DEFAULT_WEBLEVELS_DIR "downloaded"

#if !defined(_MSC_VER)
  #ifdef __GNUC__
  #if (__GNUC__ >= 3)
  #include <ext/hash_map>
  namespace HashNamespace=__gnu_cxx;
  #else
  #include <hash_map>
  #define HashNamespace std
  #endif
  #else // #ifdef __GNUC__
  #include <hash_map>
  #if (_MSC_VER >= 1300)
  namespace HashNamespace=stdext;
  #else
  namespace HashNamespace=std;
  #endif
  #endif
#endif

#if !defined(_MSC_VER)
  struct highscore_str {
    bool operator()(const char* s1, const char* s2) {
      return strcmp(s1, s2) == 0;
    }
  };
#endif

class FSWeb {
 public:
  static void downloadFile(const std::string &p_local_file, const std::string &p_web_file);
 private:
  static size_t writeData(void *ptr, size_t size, size_t nmemb, FILE *stream);
};

class WebHighscore {
 public:
  WebHighscore(std::string p_levelId,
	       std::string p_playerName,
	       std::string p_time,
	       std::string p_rplUrl);
  ~WebHighscore();
  void download(); /* throws exceptions */

  std::string getPlayerName() const;
  std::string getTime() const;
  std::string getLevelId() const;

 private:
  std::string m_playerName;
  std::string m_time;
  std::string m_levelId;
  std::string m_rplUrl;
  std::string m_rplFilename;
};

class WebHighscores {
 public:
  WebHighscores();
  ~WebHighscores();

  void update(); /* throws exceptions */
  void upgrade();

  /* return NULL if no data found */
  WebHighscore* getHighscoreFromLevel(const std::string &p_levelId);
  void setWebsiteURL(std::string p_webhighscores_url);

 private:
  #if defined(_MSC_VER)
    std::vector<WebHighscore *> m_webhighscores;
  #else
    HashNamespace::hash_map<const char*, WebHighscore*, HashNamespace::hash<const char*>, highscore_str> m_webhighscores;
  #endif
  
  std::string m_userFilename;
  std::string m_webhighscores_url;

  void fillHash();
  void cleanHash();
};

class WebLevels {
 public:
  static void upgrade(); /* throws exceptions */
};

#endif /* WEBHIGHSCORES */

