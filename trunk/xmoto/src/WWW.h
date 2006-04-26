/* file contributed by Nicolas Adenis-Lamarre */

#ifndef __WWW_H__
#define __WWW_H__

#include "BuildConfig.h"
#if defined(SUPPORT_WEBACCESS)

#include <string>
#include <vector>

#if !defined(_MSC_VER)
  #define USE_HASH_MAP
#endif

#include "WWWAppInterface.h"

#define DEFAULT_WEBHIGHSCORES_URL         "http://aspegic500.free.fr/xmoto_highscores/highscores.xml"
#define DEFAULT_WEBHIGHSCORES_FILENAME    "webhighscores.xml"
#define DEFAULT_TRANSFERT_TIMEOUT         240
#define DEFAULT_TRANSFERT_CONNECT_TIMEOUT 20
#define DEFAULT_WEBLEVELS_URL             "http://aspegic500.free.fr/xmoto_highscores/levels.xml"
#define DEFAULT_WEBLEVELS_FILENAME        "weblevels.xml"
#define DEFAULT_WEBLEVELS_DIR             "downloaded"

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
 private:
  static size_t writeData(void *ptr, size_t size, size_t nmemb, FILE *stream);
};

class WebHighscore {
 public:
  WebHighscore(std::string p_levelId,
	       std::string p_playerName,
	       std::string p_time,
	       std::string p_rplUrl,
	       const ProxySettings *p_proxy_settings);
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
  const ProxySettings *m_proxy_settings;
};

class WebHighscores {
 public:
  WebHighscores(const ProxySettings *p_proxy_settings);
  ~WebHighscores();

  void update(); /* throws exceptions */
  void upgrade();

  /* return NULL if no data found */
  WebHighscore* getHighscoreFromLevel(const std::string &p_levelId);
  void setWebsiteURL(std::string p_webhighscores_url);

 private:
  #if defined(USE_HASH_MAP)
    HashNamespace::hash_map<const char*, WebHighscore*, HashNamespace::hash<const char*>, highscore_str> m_webhighscores;
  #else
    std::vector<WebHighscore *> m_webhighscores;
  #endif
  std::string m_userFilename;
  std::string m_webhighscores_url;
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
private:
  std::string m_id;
  std::string m_name;
  std::string m_url;
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
  
  /* Get names of files downloaded OK */
  const std::vector<std::string> &getDownloadedLevels(void);
  
 private:
  vapp::WWWAppInterface *m_WebLevelApp;
  std::vector<WebLevel*> m_webLevels;
  std::vector<std::string> m_webLevelsDownloadedOK; /* file names of those levels 
           which where downloaded OK (so we can load them right away) */
  const ProxySettings *m_proxy_settings;

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

#endif

#endif /* WEBSTUFFS */

