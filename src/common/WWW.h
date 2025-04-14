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

/* file contributed by Nicolas Adenis-Lamarre */

#ifndef __WWW_H__
#define __WWW_H__

#include "BuildConfig.h"

#include <curl/curl.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "Theme.h"
#include "VFileIO_types.h"
#include "WWWAppInterface.h"
#include "XMBuild.h"
#include "XMSession.h"

class ThemeChoice;
class WebRoom;
class xmDatabase;

#define DEFAULT_SITE_URL "https://" WEBSITE_DOMAIN

#define DEFAULT_TRANSFER_TIMEOUT 240
#define DEFAULT_TRANSFER_CONNECT_TIMEOUT 15

#define DEFAULT_WEBHIGHSCORES_URL DEFAULT_SITE_URL "/highscores.xml"
#define DEFAULT_WEBLEVELS_URL DEFAULT_SITE_URL "/levels.xml"
#define DEFAULT_UPLOADDBSYNC_URL DEFAULT_SITE_URL "/tools/UploadDbSync.php"
#define DEFAULT_WEBTHEMES_URL DEFAULT_SITE_URL "/themes.xml"
#define DEFAULT_WEBTHEMES_SPRITESURLBASE DEFAULT_SITE_URL "/sprites"
#define DEFAULT_UPLOADREPLAY_URL DEFAULT_SITE_URL "/tools/UploadReplay.php"
#define DEFAULT_SENDVOTE_URL DEFAULT_SITE_URL "/tools/SendVote.php"
#define DEFAULT_SENDREPORT_URL DEFAULT_SITE_URL "/tools/SendReport.php"
#define DEFAULT_WEBROOMS_URL DEFAULT_SITE_URL "/rooms.xml"

#define DEFAULT_WEBHIGHSCORES_FILENAME_PREFIX "webhighscores"
#define DEFAULT_WEBLEVELS_FILENAME "weblevels.xml"
#define DEFAULT_WEBLEVELS_DIR "downloaded"
#define DEFAULT_WEBTHEMES_FILENAME "webthemes.xml"
#define DEFAULT_WEBROOMS_FILENAME "webrooms.xml"
#define DEFAULT_WEBROOM_NAME "WR"

#define WWW_AGENT ("xmoto-" + XMBuild::getVersionString(true))

#if CURL_AT_LEAST_VERSION(7, 56, 0)
#define USE_CURL_MIME_API 1
#define HTTP_FORM_TYPE CURLOPT_MIMEPOST
#else
#define USE_CURL_MIME_API 0
#define HTTP_FORM_TYPE CURLOPT_HTTPPOST
#endif

class HTTPForm {
public:
  HTTPForm(CURL *curl);
  ~HTTPForm();

#if USE_CURL_MIME_API
  const curl_mime *const handle() const { return m_mime; }
#else
  const curl_httppost *const handle() const { return m_post; }
#endif

  void addData(const std::string &name, const std::string &data);
  void addFile(const std::string &name, const std::string &filename);

private:
  void add(const std::string &name, const std::string &data, bool file);

private:
  CURL *m_curl;

#if USE_CURL_MIME_API
  curl_mime *m_mime;
#else
  curl_httppost *m_post;
  curl_httppost *m_last;
#endif
};

struct f_curl_download_data {
  WWWAppInterface *v_WebApp;
  int v_nb_files_to_download;
  int v_nb_files_performed;
};

struct f_curl_upload_data {
  WWWAppInterface *v_WebApp;
};

using ProgressCallback = int (*)(void *clientp,
                                 curl_off_t dltotal,
                                 curl_off_t dlnow,
                                 curl_off_t ultotal,
                                 curl_off_t ulnow);

class FSWeb {
public:
  static void downloadFile(const std::string &p_local_file,
                           const std::string &p_web_file,
                           ProgressCallback progressCallback,
                           void *p_data,
                           const ProxySettings *p_proxy_settings);

  static void downloadFileBz2(const std::string &p_local_file,
                              const std::string &p_web_file,
                              ProgressCallback progressCallback,
                              void *p_data,
                              const ProxySettings *p_proxy_settings);

  static void downloadFileBz2UsingMd5(const std::string &p_local_file,
                                      const std::string &p_web_file,
                                      ProgressCallback progressCallback,
                                      void *p_data,
                                      const ProxySettings *p_proxy_settings);

  static void uploadReplay(const std::string &p_replayFilename,
                           const std::string &p_id_room,
                           const std::string &p_login,
                           const std::string &p_password,
                           const std::string &p_url_to_transfer,
                           WWWAppInterface *p_WebApp,
                           const ProxySettings *p_proxy_settings,
                           bool &p_msg_status,
                           std::string &p_msg);

  static void uploadDbSync(const std::string &p_dbSyncFilename,
                           const std::string &p_login,
                           const std::string &p_password,
                           const std::string &p_siteKey,
                           int p_dbSyncServer,
                           const std::string &p_url_to_transfer,
                           WWWAppInterface *p_WebApp,
                           const ProxySettings *p_proxy_settings,
                           bool &p_msg_status,
                           std::string &p_msg,
                           const std::string &p_answerFile);

  static void sendVote(const std::string &p_id_level,
                       const std::string &p_difficulty_value,
                       const std::string &p_quality_value,
                       bool p_adminMode,
                       const std::string &p_id_profile,
                       const std::string &p_password,
                       const std::string &p_url_to_transfer,
                       WWWAppInterface *p_WebApp,
                       const ProxySettings *p_proxy_settings,
                       bool &p_msg_status,
                       std::string &p_msg);

  static void sendReport(const std::string &p_reportauthor,
                         const std::string &p_reportmsg,
                         const std::string &p_url_to_transfer,
                         WWWAppInterface *p_WebApp,
                         const ProxySettings *p_proxy_settings,
                         bool &p_msg_status,
                         std::string &p_msg);

  static int f_curl_progress_callback_upload(void *clientp,
                                             curl_off_t dltotal,
                                             curl_off_t dlnow,
                                             curl_off_t ultotal,
                                             curl_off_t ulnow);

  static int f_curl_progress_callback_download(void *clientp,
                                               curl_off_t dltotal,
                                               curl_off_t dlnow,
                                               curl_off_t ultotal,
                                               curl_off_t ulnow);

private:
  static size_t writeData(void *ptr, size_t size, size_t nmemb, FILE *stream);
  static void uploadAnalyseMsg(const std::string &p_key,
                               const std::string &p_filename,
                               bool &p_msg_status_ok,
                               std::string &p_msg);

  static CURLcode performPostCurl(CURL *p_curl,
                                  HTTPForm &form,
                                  const std::string &p_url_to_transfer,
                                  FILE *p_destinationFile,
                                  WWWAppInterface *p_WebApp,
                                  const ProxySettings *p_proxy_settings);
};

class WebRoom {
public:
  WebRoom(WWWAppInterface *p_WebRoomApp);
  ~WebRoom();

  void update(const std::string &i_id_room); /* throws exceptions */
  void upgrade(const std::string &i_id_room,
               xmDatabase *i_db); /* throws exceptions */

  /* return NULL if no data found */
  void setProxy(const ProxySettings *pProxySettings);

  // only required for update and upgrade
  void setHighscoresUrl(const std::string &i_webhighscores_url);

  void downloadReplay(const std::string &i_url);
  bool downloadReplayExists(const std::string &i_url);

private:
  WWWAppInterface *m_WebRoomApp;
  std::string m_userFilename_prefix;
  std::string m_webhighscores_url;
  const ProxySettings *m_proxy_settings;
};

class WebLevels {
public:
  WebLevels(WWWAppInterface *p_WebLevelApp);
  ~WebLevels();

  /* check for new levels to download */
  void update(xmDatabase *i_db); /* throws exceptions */

  /* download new levels */
  void upgrade(
    xmDatabase *i_db,
    int i_nb_levels = -1 /* -1 for all levels */); /* throws exceptions */

  /* Get names of new files downloaded OK */
  const std::vector<std::string> &getNewDownloadedLevels();

  /* Get names of updated files downloaded OK*/
  const std::vector<std::string> &getUpdatedDownloadedLevels();

  /* Set URL */
  void setWebsiteInfos(const std::string &p_url,
                       const ProxySettings *p_proxy_settings);

  static std::string getDestinationFile(std::string p_url);

  /* return the number of level files required to download for an upgrade */
  int nbLevelsToGet(xmDatabase *i_db) const;

private:
  WWWAppInterface *m_WebLevelApp;

  std::vector<std::string>
    m_webLevelsNewDownloadedOK; /* file names of those levels
which where downloaded OK (so we can load them right away) and which are new */
  std::vector<std::string> m_webLevelsUpdatedDownloadedOK;

  const ProxySettings *m_proxy_settings;

  std::string m_levels_url;

  std::string getXmlFileName();
  void downloadXml(); /* throw exceptions */
  static std::string getDestinationDir();
  void createDestinationDirIfRequired();
};

class WebThemes {
public:
  static void updateTheme(xmDatabase *i_pDb,
                          const std::string &i_id_theme,
                          WWWAppInterface *i_WebLevelApp);
  static void updateThemeList(xmDatabase *i_pDb,
                              WWWAppInterface *i_WebLevelApp);
  static bool isUpdatable(xmDatabase *i_pDb, const std::string &i_id_theme);
};

class XMSync {
public:
  static void syncUp(xmDatabase *i_pDb,
                     const ProxySettings *pProxySettings,
                     const std::string &i_sitekey,
                     const std::string &i_profile);
  static void syncDown();
};

#endif /* WEBSTUFFS */
