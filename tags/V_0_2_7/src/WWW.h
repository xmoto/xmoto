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
#include <stdio.h>

#if !defined(WIN32) && !defined(__APPLE__) && !defined(__MACH__)
  //#define USE_HASH_MAP // removed because it seems to segfault i don't know why when i refresh levels using F5 and quit
#endif

#include "WWWAppInterface.h"
#include "Theme.h"
class ThemeChoice;

#define DEFAULT_WEBHIGHSCORES_URL         "http://xmoto.tuxfamily.org/highscores.xml"
#define DEFAULT_WEBHIGHSCORES_FILENAME    "webhighscores.xml"
#define DEFAULT_TRANSFERT_TIMEOUT         240
#define DEFAULT_TRANSFERT_CONNECT_TIMEOUT 7
#define DEFAULT_WEBLEVELS_URL             "http://xmoto.tuxfamily.org/levels.xml"
#define DEFAULT_WEBLEVELS_FILENAME        "weblevels.xml"
#define DEFAULT_WEBLEVELS_DIR             "downloaded"
#define DEFAULT_WEBTHEMES_URL             "http://xmoto.tuxfamily.org/themes.xml"
#define DEFAULT_WEBTHEMES_FILENAME        "webthemes.xml"
#define DEFAULT_WEBTHEMES_SPRITESURLBASE  "http://xmoto.tuxfamily.org/sprites"
#define DEFAULT_UPLOADREPLAY_URL          "http://xmoto.tuxfamily.org/tools/UploadReplay.php"
#define DEFAULT_REPLAYUPLOAD_MSGFILE      "UploadReplayMsg.xml"
#define DEFAULT_WEBROOMS_URL              "http://xmoto.tuxfamily.org/rooms.xml"
#define DEFAULT_WEBROOMS_FILENAME         "webrooms.xml"
#define DEFAULT_WEBROOM_ID                "1"
#define DEFAULT_WEBROOM_NAME              "WR"

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
  namespace HashNamespace=std;
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
			   int (*curl_progress_callback_download)(void *clientp,
								  double dltotal,
								  double dlnow,
								  double ultotal,
								  double ulnow),
			   void *p_data,
			   const ProxySettings *p_proxy_settings);

  static void downloadFileBz2(const std::string &p_local_file,
			      const std::string &p_web_file,
			      int (*curl_progress_callback_download)(void *clientp,
								     double dltotal,
								     double dlnow,
								     double ultotal,
								     double ulnow),
			      void *p_data,
			      const ProxySettings *p_proxy_settings);

  static void downloadFileBz2UsingMd5(const std::string &p_local_file,
				      const std::string &p_web_file,
				      int (*curl_progress_callback_download)(void *clientp,
									     double dltotal,
									     double dlnow,
									     double ultotal,
									     double ulnow),
				      void *p_data,
				      const ProxySettings *p_proxy_settings);

  static void uploadReplay(std::string p_replayFilename,
			   std::string p_id_room,
			   std::string p_login,
			   std::string p_password,
			   std::string p_url_to_transfert,
			   vapp::WWWAppInterface *p_WebApp,
			   const ProxySettings *p_proxy_settings,
			   bool &p_msg_status,
			   std::string &p_msg);

  static int f_curl_progress_callback_upload(void *clientp,
					     double dltotal,
					     double dlnow,
					     double ultotal,
					     double ulnow);

  static int f_curl_progress_callback_download(void *clientp,
					       double dltotal,
					       double dlnow,
					       double ultotal,
					       double ulnow);

 private:
  static size_t writeData(void *ptr, size_t size, size_t nmemb, FILE *stream);
  static void   uploadReplayAnalyseMsg(std::string p_filename,
				       bool &p_msg_status_ok,
				       std::string &p_msg);
};

class WebHighscore {
 public:
  WebHighscore(WebRoom *p_room,
	       std::string p_levelId,
	       std::string p_playerName,
	       int p_day, int p_month, int p_year,
	       std::string p_date,
	       std::string p_rplUrl,
	       const ProxySettings *p_proxy_settings);
  ~WebHighscore();
  void download(); /* throws exceptions */
  /* return the path to the replay, even if it's not downloaded */
  std::string getReplayName();

  std::string getPlayerName() const;
  std::string getTime() const;
  int getDateYear()  const;
  int getDateMonth() const;
  int getDateDay()   const;
  float       getFTime() const; /* throws exceptions */
  std::string getLevelId() const;
  WebRoom* getRoom() const;

 private:
  std::string m_playerName;
  std::string m_time;
  int m_day, m_month, m_year;
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

class WebRoomInfos {
 public:
  WebRoomInfos(std::string p_id,
	       std::string p_name,
	       std::string p_urlHighscores);
  ~WebRoomInfos();

  std::string getId();
  std::string getName();
  std::string getUrlHighscores();

 private:
  std::string m_id;
  std::string m_name;
  std::string m_urlHighscore;
};

class WebRooms {
 public:
  WebRooms(const ProxySettings *p_proxy_settings);
  ~WebRooms();

  /* check for new rooms */
  void update(); /* throws exceptions */

  /* fill the list of avaible rooms ; does not required an internet connexion */
  void upgrade();

  void setURL(const std::string &p_url) {m_rooms_url = p_url;}
  const std::vector<WebRoomInfos*> &getAvailableRooms();

 private:
  const ProxySettings *m_proxy_settings;
  std::vector<WebRoomInfos*> m_availableRooms;
  std::string m_rooms_url;

  void clean();
  std::string getXmlFileName();
};

class WebLevel {
public:
  WebLevel(std::string p_id, std::string p_name, std::string p_url, float p_webDifficulty, float p_webQuality);
  std::string getId() const;
  std::string getName() const;
  std::string getUrl() const;
  float getDifficulty() const;
  float getQuality() const;

  /* true if the level should be updated from the website but exists */  
  bool requireUpdate() const; /* false by default */
  void setRequireUpdate(bool p_require_update);

  bool requireDownload() const; /* false by default */
  void setRequireDownload(bool p_require_download);

  /* if level require update, these methods have a sense */
  void setCurrentPath(std::string p_current_path);
  std::string getCurrentPath() const;

private:
  std::string m_id;
  std::string m_name;
  std::string m_url;
  bool m_require_update;
  bool m_require_download;
  std::string m_current_path;
  float m_webDifficulty;
  float m_webQuality;
};

class WebLevels {
 public:
  WebLevels(vapp::WWWAppInterface *p_WebLevelApp,
	    const ProxySettings *p_proxy_settings);
  ~WebLevels();

  /* check for new levels to download */
  void update(bool i_enableWeb = true); /* throws exceptions */

  /* download new levels */
  void upgrade(); /* throws exceptions */
  
  /* after update() you might want to know how much is going to be downloaded */
  void getUpdateInfo(int *pnUBytes,int *pnULevels);
  
  /* Get names of new files downloaded OK */
  const std::vector<std::string> &getNewDownloadedLevels();
  
  /* Get names of updated files downloaded OK*/
  const std::vector<std::string> &getUpdatedDownloadedLevels();

  /* Get IDs of updated levels downloaded OK*/
  const std::vector<std::string> &getUpdatedDownloadedLevelIds();
  
  /* Get levels informations */
  const std::vector<WebLevel*> &getLevels();

  /* Set URL */
  void setURL(const std::string &p_url) {m_levels_url = p_url;}

  bool exists(const std::string p_id);

  static std::string getDestinationFile(std::string p_url);

  int nbLevelsToGet() const; /* return the number of level files required to download for an upgrade */

 private:
  vapp::WWWAppInterface *m_WebLevelApp;
  std::vector<WebLevel*> m_webLevels;
  std::vector<std::string> m_webLevelsNewDownloadedOK; /* file names of those levels 
           which where downloaded OK (so we can load them right away) and which are new */
  std::vector<std::string> m_webLevelsUpdatedDownloadedOK;
  std::vector<std::string> m_webLevelsIdsUpdatedDownloadedOK;

  const ProxySettings *m_proxy_settings;
  
  std::string m_levels_url;

  std::string getXmlFileName();
  void downloadXml(); /* throw exceptions */
  static std::string getDestinationDir();
  void createDestinationDirIfRequired();
  void extractLevelsToDownloadFromXml(); /* throw exceptions */				      
};

class WebTheme {
 public:
  WebTheme(std::string pName, std::string pUrl, std::string pSum);
  ~WebTheme();
  std::string getName() const;
  std::string getUrl() const;
  std::string getSum() const;

 private:
  std::string m_name;
  std::string m_url;
  std::string m_sum;
};

class WebThemes {
 public:
  WebThemes(vapp::WWWAppInterface *p_WebApp,
	    const ProxySettings *p_proxy_settings);
  ~WebThemes();

  /* check for new themes to download */
  void update(); /* throws exceptions */

  /* fill the list of avaible theme ; does not required an internet connexion */
  void upgrade();

  /* download a theme or just update it */
  void upgrade(ThemeChoice *p_themeChoice);
  bool isUpgradable(ThemeChoice *p_themeChoice);

  const std::vector<WebTheme*> &getAvailableThemes();

  void setURL(const std::string &p_url) {m_themes_url = p_url;}
  void setURLBase(const std::string &p_urlBase) {m_themes_urlBase = p_urlBase;}

 private:
  std::string getXmlFileName();
  void extractThemesAvailableFromXml();
  void clean();

  vapp::WWWAppInterface *m_WebApp;
  std::string m_themes_url;
  std::string m_themes_urlBase;
  const ProxySettings *m_proxy_settings;
  std::vector<WebTheme*> m_availableThemes;
};

#endif

#endif /* WEBSTUFFS */

