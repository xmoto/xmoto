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

#ifndef __WWW_H__
#define __WWW_H__

#include "BuildConfig.h"
#if defined(SUPPORT_WEBACCESS)

#include <string>
#include <vector>

#if !defined(_MSC_VER) && !defined(__APPLE__) && !defined(__MACH__)
  #define USE_HASH_MAP
#endif

#include "WWWAppInterface.h"

#define DEFAULT_WEBHIGHSCORES_URL         "http://xmoto.free.fr/highscores.xml"
#define DEFAULT_WEBHIGHSCORES_FILENAME    "webhighscores.xml"
#define DEFAULT_TRANSFERT_TIMEOUT         240
#define DEFAULT_TRANSFERT_CONNECT_TIMEOUT 15
#define DEFAULT_WEBLEVELS_URL             "http://xmoto.free.fr/levels.xml"
#define DEFAULT_WEBLEVELS_FILENAME        "weblevels.xml"
#define DEFAULT_WEBLEVELS_DIR             "downloaded"
#define DEFAULT_WEBTHEMES_FILENAME        "webthemes.xml"

#define WWW_AGENT ("xmoto-" + vapp::App::getVersionString())

#if defined(USE_HASH_MAP)
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

  struct highscore_str {
    bool operator()(const char* s1, const char* s2) {
      return strcmp(s1, s2) == 0;
    }
  };
#endif

class WebRoom;

class ProxySettings {
 public:
  ProxySettings();
  void setServer(std::string p_server);
  void setPort(long p_port);
  void setType(long p_type); /* CURLPROXY_HTTP OR CURLPROXY_SOCKS5 */
  void setAuthentification(std::string p_user, std::string p_password);

  void setDefaultServer();
  void setDefaultPort();
  void setDefaultType();
  void setDefaultAuthentification();

  std::string getServer() const;
  long getPort() const;
  long getType() const; /* CURLPROXY_HTTP OR CURLPROXY_SOCKS5 */
  std::string getAuthentificationUser() const;
  std::string getAuthentificationPassword() const;

  /* default means : curl try to find default values (no proxy, or environment vars) */
  bool useDefaultServer() const;
  bool useDefaultPort() const;
  bool useDefaultAuthentification() const;

 private:
  std::string m_server;
  long m_port;
  long m_type;
  std::string m_authUser;
  std::string m_authPassword;
};

class FSWeb {
 public:
  static void downloadFile(const std::string &p_local_file,
			   const std::string &p_web_file,
			   int (*curl_progress_callback)(void *clientp,
							 double dltotal,
							 double dlnow,
							 double ultotal,
							 double ulnow),
			   void *p_data,
			   const ProxySettings *p_proxy_settings);

  static void downloadFileBz2(const std::string &p_local_file,
			      const std::string &p_web_file,
			      int (*curl_progress_callback)(void *clientp,
							    double dltotal,
							    double dlnow,
							    double ultotal,
							    double ulnow),
			      void *p_data,
			      const ProxySettings *p_proxy_settings);

  static void downloadFileBz2UsingMd5(const std::string &p_local_file,
				      const std::string &p_web_file,
				      int (*curl_progress_callback)(void *clientp,
								    double dltotal,
								    double dlnow,
								    double ultotal,
								    double ulnow),
				      void *p_data,
				      const ProxySettings *p_proxy_settings);

 private:
  static size_t writeData(void *ptr, size_t size, size_t nmemb, FILE *stream);
};

class WebHighscore {
 public:
  WebHighscore(WebRoom *p_room,
	       std::string p_levelId,
	       std::string p_playerName,
	       std::string p_time,
	       std::string p_rplUrl,
	       const ProxySettings *p_proxy_settings);
  ~WebHighscore();
  void download(); /* throws exceptions */
  /* return the path to the replay, even if it's not downloaded */
  std::string getReplayName();

  std::string getPlayerName() const;
  std::string getTime() const;
  float       getFTime() const; /* throws exceptions */
  std::string getLevelId() const;
  WebRoom* getRoom() const;

 private:
  std::string m_playerName;
  std::string m_time;
  std::string m_levelId;
  std::string m_rplUrl;
  std::string m_rplFilename;
  const ProxySettings *m_proxy_settings;
  WebRoom *m_room;
};

class WebRoom {
 public:
  WebRoom(const ProxySettings *p_proxy_settings);
  ~WebRoom();

  void update(); /* throws exceptions */
  void upgrade(); /* throws exceptions */

  /* return NULL if no data found */
  WebHighscore* getHighscoreFromLevel(const std::string &p_levelId);
  void setWebsiteURL(std::string p_webhighscores_url);
  std::string getRoomName() const;

 private:
  #if defined(USE_HASH_MAP)
    HashNamespace::hash_map<const char*, WebHighscore*, HashNamespace::hash<const char*>, highscore_str> m_webhighscores;
  #else
    std::vector<WebHighscore *> m_webhighscores;
  #endif
  std::string m_userFilename;
  std::string m_webhighscores_url;
  std::string m_roomName;
  const ProxySettings *m_proxy_settings;

  void fillHash();
  void cleanHash();
};

class WebLevel {
public:
  WebLevel(std::string p_id, std::string p_name, std::string p_url);
  std::string getId() const;
  std::string getName() const;
  std::string getUrl() const;

  /* true if the level should be updated from the website but exists */  
  bool requireUpdate() const; /* false by default */
  void setRequireUpdate(bool p_require_update);

  /* if level require update, these methods have a sense */
  void setCurrentPath(std::string p_current_path);
  std::string getCurrentPath() const;

private:
  std::string m_id;
  std::string m_name;
  std::string m_url;
  bool m_require_update;
  std::string m_current_path;
};

class WebLevels {
 public:
  WebLevels(vapp::WWWAppInterface *p_WebLevelApp,
	    const ProxySettings *p_proxy_settings);
  ~WebLevels();

  /* check for new levels to download */
  void update(); /* throws exceptions */

  /* download new levels */
  void upgrade(); /* throws exceptions */
  
  /* after update() you might want to know how much is going to be downloaded */
  void getUpdateInfo(int *pnUBytes,int *pnULevels);
  
  /* Get names of new files downloaded OK */
  const std::vector<std::string> &getNewDownloadedLevels();
  
  /* Get names of updated files downloaded OK*/
  const std::vector<std::string> &getUpdatedDownloadedLevels();

  /* Get IDs of updated levels downloaded OK*/
  const std::vector<std::string> &getUpdatedDownloadedLevelIDs();
  
  /* Set URL */
  void setURL(const std::string &p_url) {m_levels_url = p_url;}

 private:
  vapp::WWWAppInterface *m_WebLevelApp;
  std::vector<WebLevel*> m_webLevels;
  std::vector<std::string> m_webLevelsNewDownloadedOK; /* file names of those levels 
           which where downloaded OK (so we can load them right away) and which are new */
  std::vector<std::string> m_webLevelsUpdatedDownloadedOK;

  const ProxySettings *m_proxy_settings;
  
  std::string m_levels_url;

  std::string getXmlFileName();
  void downloadXml(); /* throw exceptions */
  std::string getDestinationDir();
  std::string getDestinationFile(std::string p_url);
  void createDestinationDirIfRequired();
  void extractLevelsToDownloadFromXml(); /* throw exceptions */
  static int f_curl_progress_callback(void *clientp,
				      double dltotal,
				      double dlnow,
				      double ultotal,
				      double ulnow);  				      
};

class WebTheme {
 public:
  WebTheme(std::string pName, std::string pUrl);
  ~WebTheme();
  std::string getName() const;
  std::string getUrl() const;

 private:
  std::string m_name;
  std::string m_url;
};

class WebThemes {
 public:
  WebThemes(const ProxySettings *p_proxy_settings);
  ~WebThemes();

  /* check for new themes to download */
  void update(); /* throws exceptions */

  /* fill the list of avaible theme ; does not required an internet connexion */
  void upgrade();

  const std::vector<WebTheme*> &getAvailableThemes();

 private:
  std::string getXmlFileName();
  void extractThemesAvailableFromXml();
  void clean();

  const ProxySettings *m_proxy_settings;
  std::vector<WebTheme*> m_availableThemes;
};

#endif

#endif /* WEBSTUFFS */

