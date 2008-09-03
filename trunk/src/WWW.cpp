/* file contributed by Nicolas Adenis-Lamarre */

#include "BuildConfig.h"

#include "WWW.h"
#include "Game.h"
#include "VFileIO.h"
#include "helpers/VExcept.h"
#include "helpers/Log.h"
#include "VXml.h"
#include <curl/curl.h>
#include "GameText.h"

#ifdef WIN32
  #include <io.h>
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <dirent.h>
#endif
#include <sys/stat.h>
#include <vector>
#include "helpers/FileCompression.h"
#include "db/xmDatabase.h"
#include "md5sum/md5file.h"

#define DEFAULT_REPLAYUPLOAD_MSGFILE "UploadReplayMsg.xml"

void WebRoom::downloadReplay(const std::string& i_url) {
  std::string i_rplFilename = FS::getReplaysDir()
    + "/" 
    + FS::getFileBaseName(i_url) 
    + ".rpl";

  /* download xml file */
  f_curl_download_data v_data;

  v_data.v_WebApp               = m_WebRoomApp;
  v_data.v_nb_files_performed   = 0;
  v_data.v_nb_files_to_download = 1;

  FSWeb::downloadFile(i_rplFilename, i_url,
		      FSWeb::f_curl_progress_callback_download,
		      &v_data, m_proxy_settings);
}

WebRoom::WebRoom(WWWAppInterface* p_WebRoomApp)
{
  m_userFilename_prefix = FS::getUserDir()
    + "/" 
    + DEFAULT_WEBHIGHSCORES_FILENAME_PREFIX;
  m_webhighscores_url = DEFAULT_WEBHIGHSCORES_URL;
  m_WebRoomApp        = p_WebRoomApp;
  m_proxy_settings    = NULL;
}

WebRoom::~WebRoom()
{
}

std::string WebRoom::getRoomId() const
{
  return m_roomId;
}

void WebRoom::setWebsiteInfos(const std::string& i_id_room,
			      const std::string& i_webhighscores_url,
			      const ProxySettings* pProxySettings)
{
  m_webhighscores_url = i_webhighscores_url;
  m_roomId            = i_id_room;
  m_proxy_settings    = pProxySettings;
}

/* use the id of the room so that if several people share the same room, it doesn't need to recheck if md5 is ok */
void WebRoom::update(const std::string& i_id_room)
{
  /* download xml file */
  f_curl_download_data v_data;

  v_data.v_WebApp               = m_WebRoomApp;
  v_data.v_nb_files_performed   = 0;
  v_data.v_nb_files_to_download = 1;

  FSWeb::downloadFileBz2UsingMd5(m_userFilename_prefix + i_id_room + ".xml",
				 m_webhighscores_url,
				 FSWeb::f_curl_progress_callback_download,
				 &v_data,
				 m_proxy_settings);
}

void WebRoom::upgrade(const std::string& i_id_room, xmDatabase *i_db)
{
  i_db->webhighscores_updateDB(m_userFilename_prefix + i_id_room + ".xml", m_webhighscores_url);
}

size_t FSWeb::writeData(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  return fwrite (ptr, size, nmemb, stream);
}

void FSWeb::downloadFileBz2(const std::string &p_local_file,
          const std::string &p_web_file,
          int (*curl_progress_callback_download)(void *clientp,
                   double dltotal,
                   double dlnow,
                   double ultotal,
                   double ulnow),
          void *p_data,
          const ProxySettings *p_proxy_settings) {
  std::string v_bzFile = p_local_file + ".bz2";

  /* remove in case it already exists */
  remove(v_bzFile.c_str());
  downloadFile(v_bzFile,
         p_web_file + ".bz2",
         curl_progress_callback_download,
         p_data,
         p_proxy_settings
         );

  try {
    FileCompression::bunzip2(v_bzFile, p_local_file);
    remove(v_bzFile.c_str());
  } catch(Exception &e) {
    remove(v_bzFile.c_str());
    throw e;
  }
}

void FSWeb::downloadFileBz2UsingMd5(const std::string &p_local_file,
            const std::string &p_web_file,
            int (*curl_progress_callback_download)(void *clientp,
                     double dltotal,
                     double dlnow,
                     double ultotal,
                     double ulnow),
            void *p_data,
            const ProxySettings *p_proxy_settings) {
  bool require_dwd = true;

  try {
    if(FS::isFileReadable(p_local_file) == true) {
      std::string v_md5Local  = FS::md5sum(p_local_file);
      if(v_md5Local != "") {
	std::string v_md5File = p_local_file + ".md5";
  
	/* remove in case it already exists */
	remove(v_md5File.c_str());
	downloadFile(v_md5File,
		     p_web_file + ".md5",
		     NULL, /* nothing for this */
		     NULL,
		     p_proxy_settings
		     );
	
	std::string v_md5Web = md5Contents(v_md5File);
	remove(v_md5File.c_str());
	if(v_md5Web != "") {
	  require_dwd = v_md5Web != v_md5Local;
	}
      }
    }
  } catch(Exception &e) {
    /* no pb, just dwd */
  }

  if(require_dwd) {
    FSWeb::downloadFileBz2(p_local_file,
         p_web_file,
         curl_progress_callback_download,
         p_data,
         p_proxy_settings);
  }
}

void FSWeb::downloadFile(const std::string &p_local_file,
			 const std::string &p_web_file,
			 int (*curl_progress_callback_download)(void *clientp,
								double dltotal,
								double dlnow,
								double ultotal,
								double ulnow),
			 void *p_data,
			 const ProxySettings *p_proxy_settings
			 ) {
  
  LogInfo(std::string("downloading " + 
			  p_web_file     +
			  " to "         +
			  p_local_file
			  ).c_str()
	      );
  
  CURL *v_curl;
  CURLcode res;
  FILE *v_destinationFile;

  std::string v_local_file_tmp = p_local_file + ".part";

  std::string v_proxy_server;
  std::string v_proxy_auth_str;
  std::string v_www_agent = WWW_AGENT;

  /* open the file */
  if( (v_destinationFile = fopen(v_local_file_tmp.c_str(), "wb")) == false) {
    throw Exception("error : unable to open output file " 
        + v_local_file_tmp);
  }

  /* download the file */
  v_curl = curl_easy_init();
  if(v_curl == NULL) {
    fclose(v_destinationFile);
    throw Exception("error : unable to init curl"); 
  }

  curl_easy_setopt(v_curl, CURLOPT_URL, p_web_file.c_str());
  curl_easy_setopt(v_curl, CURLOPT_WRITEDATA, v_destinationFile);
  curl_easy_setopt(v_curl, CURLOPT_WRITEFUNCTION, FSWeb::writeData);
  curl_easy_setopt(v_curl, CURLOPT_TIMEOUT, DEFAULT_TRANSFERT_TIMEOUT);
  curl_easy_setopt(v_curl, CURLOPT_CONNECTTIMEOUT, DEFAULT_TRANSFERT_CONNECT_TIMEOUT);
  curl_easy_setopt(v_curl, CURLOPT_USERAGENT,  v_www_agent.c_str());
  curl_easy_setopt(v_curl, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(v_curl, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt(v_curl, CURLOPT_FOLLOWLOCATION, 1);

  /* set proxy settings */
  if(p_proxy_settings != NULL && p_proxy_settings->getTypeStr() != "") {
    /* v_proxy_server because 
       after call to 
       curl_easy_setopt(v_curl, CURLOPT_PROXY, p_proxy_settings->getServer().c_str());
       result is destroyed on call to curl_easy_perform
    */
    v_proxy_server = p_proxy_settings->getServer();
    v_proxy_auth_str =
      p_proxy_settings->getAuthentificationUser()
      + ":"
      + p_proxy_settings->getAuthentificationPassword();

    if(p_proxy_settings->useDefaultServer() == false) {
      curl_easy_setopt(v_curl, CURLOPT_PROXY, v_proxy_server.c_str());
    }
    
    if(p_proxy_settings->useDefaultPort() == false) {
      curl_easy_setopt(v_curl, CURLOPT_PROXYPORT, p_proxy_settings->getPort());
    }
    
    curl_easy_setopt(v_curl, CURLOPT_PROXYTYPE, p_proxy_settings->getType());
   
    if(p_proxy_settings->useDefaultAuthentification() == false) {
      curl_easy_setopt(v_curl, CURLOPT_PROXYUSERPWD,
           v_proxy_auth_str.c_str());
    }
  }
  /* ***** */

  if(curl_progress_callback_download != NULL) {
    curl_easy_setopt(v_curl, CURLOPT_NOPROGRESS, false);
    curl_easy_setopt(v_curl, CURLOPT_PROGRESSFUNCTION, curl_progress_callback_download);
    curl_easy_setopt(v_curl, CURLOPT_PROGRESSDATA, p_data);
  }

  res = curl_easy_perform(v_curl);
  curl_easy_cleanup(v_curl);
  fclose(v_destinationFile);

  /* this is not considered as an error */
  if(res == CURLE_ABORTED_BY_CALLBACK) {
    remove(v_local_file_tmp.c_str());
    return;
  }

  if(res != CURLE_OK) {
    char v_err[256];

    remove(v_local_file_tmp.c_str());

    snprintf(v_err, 256, "error : unable to download file %s (curl[%i]: %s)",
	     p_web_file.c_str(), res, curl_easy_strerror(res));
    throw Exception(v_err);
  }

  /* replace file */
  /* On windows you can't rename FILE1 to FILE2 if FILE2 already exists...
     So we need to delete it first. */
  remove(p_local_file.c_str());
  if(rename(v_local_file_tmp.c_str(), p_local_file.c_str()) != 0) {
    remove(v_local_file_tmp.c_str());
    throw Exception("error : unable to write output file " 
        + p_local_file);
  }
}

void FSWeb::uploadReplay(const std::string& p_replayFilename,
			 const std::string& p_id_room,
			 const std::string& p_login,
			 const std::string& p_password,
			 const std::string& p_url_to_transfert,
			 WWWAppInterface *p_WebApp,
			 const ProxySettings *p_proxy_settings,
			 bool &p_msg_status,
			 std::string &p_msg) {
  CURL *v_curl;
  CURLcode v_res;
  std::string v_proxy_server;
  std::string v_proxy_auth_str;
  std::string v_www_agent = WWW_AGENT;
  f_curl_upload_data v_data;
  struct curl_slist *v_headers;
  std::string v_accept_language;

  FILE *v_destinationFile;
  std::string v_local_file;
  v_local_file = FS::getUserDir() + "/" + DEFAULT_REPLAYUPLOAD_MSGFILE;

  struct curl_httppost *v_post, *v_last;

  LogInfo(std::string("Uploading replay " + p_replayFilename).c_str());

  /* open the file */
  if( (v_destinationFile = fopen(v_local_file.c_str(), "wb")) == false) {
    throw Exception("error : unable to open output file " DEFAULT_REPLAYUPLOAD_MSGFILE);
  }
      
  v_curl = curl_easy_init();
  if(v_curl == NULL) {
    fclose(v_destinationFile);
    remove(v_local_file.c_str());
    throw Exception("error : unable to init curl"); 
  }

  curl_easy_setopt(v_curl, CURLOPT_URL, p_url_to_transfert.c_str());

  v_post = NULL;
  v_last = NULL;
  
  curl_formadd(&v_post, &v_last, CURLFORM_COPYNAME, "id_room",
         CURLFORM_PTRCONTENTS, p_id_room.c_str(), CURLFORM_END);

  curl_formadd(&v_post, &v_last, CURLFORM_COPYNAME, "login",
         CURLFORM_PTRCONTENTS, p_login.c_str(), CURLFORM_END);

  curl_formadd(&v_post, &v_last, CURLFORM_COPYNAME, "password",
         CURLFORM_PTRCONTENTS, p_password.c_str(), CURLFORM_END);

  curl_formadd(&v_post, &v_last, CURLFORM_COPYNAME, "replay",
         CURLFORM_FILE, p_replayFilename.c_str(), CURLFORM_END);

  curl_easy_setopt(v_curl, CURLOPT_HTTPPOST, v_post);

  curl_easy_setopt(v_curl, CURLOPT_TIMEOUT, DEFAULT_TRANSFERT_TIMEOUT);
  curl_easy_setopt(v_curl, CURLOPT_CONNECTTIMEOUT, DEFAULT_TRANSFERT_CONNECT_TIMEOUT);
  curl_easy_setopt(v_curl, CURLOPT_WRITEDATA, v_destinationFile);
  curl_easy_setopt(v_curl, CURLOPT_WRITEFUNCTION, FSWeb::writeData);
  curl_easy_setopt(v_curl, CURLOPT_USERAGENT,  v_www_agent.c_str());
  curl_easy_setopt(v_curl, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(v_curl, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt(v_curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(v_curl, CURLOPT_ENCODING, "gzip,deflate");

  v_headers = NULL;
  v_accept_language = "Accept-Language: " + std::string(WEB_LANGUAGE);
  v_headers = curl_slist_append(v_headers, v_accept_language.c_str()); 
  curl_easy_setopt(v_curl, CURLOPT_HTTPHEADER, v_headers);

  /* set proxy settings */
  if(p_proxy_settings != NULL && p_proxy_settings->getTypeStr() != "") {
    /* v_proxy_server because 
       after call to 
       curl_easy_setopt(v_curl, CURLOPT_PROXY, p_proxy_settings->getServer().c_str());
       result is destroyed on call to curl_easy_perform
    */
    v_proxy_server = p_proxy_settings->getServer();
    v_proxy_auth_str =
      p_proxy_settings->getAuthentificationUser()
      + ":"
      + p_proxy_settings->getAuthentificationPassword();

    if(p_proxy_settings->useDefaultServer() == false) {
      curl_easy_setopt(v_curl, CURLOPT_PROXY, v_proxy_server.c_str());
    }
    
    if(p_proxy_settings->useDefaultPort() == false) {
      curl_easy_setopt(v_curl, CURLOPT_PROXYPORT, p_proxy_settings->getPort());
    }
    
    curl_easy_setopt(v_curl, CURLOPT_PROXYTYPE, p_proxy_settings->getType());
   
    if(p_proxy_settings->useDefaultAuthentification() == false) {
      curl_easy_setopt(v_curl, CURLOPT_PROXYUSERPWD,
           v_proxy_auth_str.c_str());
    }
  }
  /* ***** */

  if(p_WebApp != NULL) {
    v_data.v_WebApp = p_WebApp;

    curl_easy_setopt(v_curl, CURLOPT_NOPROGRESS, false);
    curl_easy_setopt(v_curl, CURLOPT_PROGRESSFUNCTION, FSWeb::f_curl_progress_callback_upload);
    curl_easy_setopt(v_curl, CURLOPT_PROGRESSDATA, &v_data);
  }

  v_res = curl_easy_perform(v_curl);

  fclose(v_destinationFile);

  /* free the headers */
  curl_slist_free_all(v_headers);


  /* CURLE_ABORTED_BY_CALLBACK is not considered as an error */
  if(v_res != CURLE_ABORTED_BY_CALLBACK) {

    if(v_res != CURLE_OK) {
      char v_err[256];
      
      curl_easy_cleanup(v_curl);
      remove(v_local_file.c_str());
      
      snprintf(v_err, 256, "error : unable to perform curl (curl[%i]: %s)",
	       v_res, curl_easy_strerror(v_res));
      
      throw Exception(v_err);
    }
  }
  curl_easy_cleanup(v_curl);

  /* analyse de la réponse */
  if(v_res == CURLE_ABORTED_BY_CALLBACK) {
    p_msg_status = 0;
    p_msg = "Upload aborted\nYou will be able to validate it later ;-)";
  } else {
    uploadAnalyseMsg("xmoto_uploadReplayResult", v_local_file, p_msg_status, p_msg);
  }

  if(XMSession::instance()->debug() == false) {
    remove(v_local_file.c_str());
  }
}

void FSWeb::uploadAnalyseMsg(const std::string& p_key,
			     const std::string& p_filename,
			     bool &p_msg_status_ok,
			     std::string &p_msg) {
  XMLDocument v_Xml;
  TiXmlDocument *v_XmlData;
  TiXmlElement *v_XmlDataElement;
  TiXmlElement *v_XmlDataElementMsg;
  std::string v_success;
  const char *pc;
  TiXmlNode * pChild;
  
  /* open the file */
  v_Xml.readFromFile(p_filename);   
  v_XmlData = v_Xml.getLowLevelAccess();
  
  if(v_XmlData == NULL) {
    throw Exception("unable to analyze xml file result");
  }
  
  /* read the res and msg */
  v_XmlDataElement = v_XmlData->FirstChildElement(p_key.c_str());
  if(v_XmlDataElement == NULL) {
    throw Exception("unable to analyze xml file result");
  }

  pc = v_XmlDataElement->Attribute("success");
  if(pc == NULL) {
    throw Exception("unable to analyze xml file result");
  }
  v_success = pc;
  p_msg_status_ok = v_success == "1";

  v_XmlDataElementMsg = v_XmlDataElement->FirstChildElement("message");
  if(v_XmlDataElementMsg == NULL) {
    throw Exception("unable to analyze xml file result");
  }

  pChild = v_XmlDataElementMsg->FirstChild();
  if(pChild == NULL) {
    throw Exception("unable to analyze xml file result");
  }

  if(pChild->ToText() == NULL) {
    throw Exception("unable to analyze xml file result");
  }

  pc = pChild->ToText()->Value();
  if(pc == NULL) {
    throw Exception("unable to analyze xml file result");
  }
  p_msg = pc;
}

/* could be factorized with uploadReplay */
void FSWeb::uploadDbSync(const std::string& p_dbSyncFilename,
			 const std::string& p_login,
			 const std::string& p_password,
			 const std::string& p_siteKey,
			 int p_dbSyncServer,
			 const std::string& p_url_to_transfert,
			 WWWAppInterface *p_WebApp,
			 const ProxySettings *p_proxy_settings,
			 bool &p_msg_status,
			 std::string& p_msg, 
			 const std::string& p_answerFile) {
  CURL *v_curl;
  CURLcode v_res;
  std::string v_proxy_server;
  std::string v_proxy_auth_str;
  std::string v_www_agent = WWW_AGENT;
  f_curl_upload_data v_data;
  struct curl_slist *v_headers;
  std::string v_accept_language;

  FILE *v_destinationFile;

  struct curl_httppost *v_post, *v_last;

  // std::ostringstream v_dbSyncServerStr;
  // curl seems not to like .str().c_str()
  char v_syncStr[256];

  LogInfo(std::string("Uploading dbsync " + p_dbSyncFilename + " to " + p_url_to_transfert).c_str());

  /* open the file */
  if( (v_destinationFile = fopen(p_answerFile.c_str(), "wb")) == false) {
    throw Exception("error : unable to open output file " + p_answerFile);
  }
      
  v_curl = curl_easy_init();
  if(v_curl == NULL) {
    fclose(v_destinationFile);
    remove(p_answerFile.c_str());
    throw Exception("error : unable to init curl"); 
  }

  curl_easy_setopt(v_curl, CURLOPT_URL, p_url_to_transfert.c_str());

  v_post = NULL;
  v_last = NULL;

  snprintf(v_syncStr, 256, "%i", p_dbSyncServer);

  curl_formadd(&v_post, &v_last, CURLFORM_COPYNAME, "login",
	       CURLFORM_PTRCONTENTS, p_login.c_str(), CURLFORM_END);

  curl_formadd(&v_post, &v_last, CURLFORM_COPYNAME, "password",
	       CURLFORM_PTRCONTENTS, p_password.c_str(), CURLFORM_END);

  curl_formadd(&v_post, &v_last, CURLFORM_COPYNAME, "downSiteKey",
	       CURLFORM_PTRCONTENTS, p_siteKey.c_str(), CURLFORM_END);

  curl_formadd(&v_post, &v_last, CURLFORM_COPYNAME, "downDbSync",
	       CURLFORM_PTRCONTENTS, v_syncStr, CURLFORM_END);

  curl_formadd(&v_post, &v_last, CURLFORM_COPYNAME, "dbSync",
	       CURLFORM_FILE, p_dbSyncFilename.c_str(), CURLFORM_END);

  curl_easy_setopt(v_curl, CURLOPT_HTTPPOST, v_post);

  curl_easy_setopt(v_curl, CURLOPT_TIMEOUT, DEFAULT_TRANSFERT_TIMEOUT);
  curl_easy_setopt(v_curl, CURLOPT_CONNECTTIMEOUT, DEFAULT_TRANSFERT_CONNECT_TIMEOUT);
  curl_easy_setopt(v_curl, CURLOPT_WRITEDATA, v_destinationFile);
  curl_easy_setopt(v_curl, CURLOPT_WRITEFUNCTION, FSWeb::writeData);
  curl_easy_setopt(v_curl, CURLOPT_USERAGENT,  v_www_agent.c_str());
  curl_easy_setopt(v_curl, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(v_curl, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt(v_curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(v_curl, CURLOPT_ENCODING, "gzip,deflate");

  v_headers = NULL;
  v_accept_language = "Accept-Language: " + std::string(WEB_LANGUAGE);
  v_headers = curl_slist_append(v_headers, v_accept_language.c_str()); 
  curl_easy_setopt(v_curl, CURLOPT_HTTPHEADER, v_headers);

  /* set proxy settings */
  if(p_proxy_settings != NULL && p_proxy_settings->getTypeStr() != "") {
    /* v_proxy_server because 
       after call to 
       curl_easy_setopt(v_curl, CURLOPT_PROXY, p_proxy_settings->getServer().c_str());
       result is destroyed on call to curl_easy_perform
    */
    v_proxy_server = p_proxy_settings->getServer();
    v_proxy_auth_str =
      p_proxy_settings->getAuthentificationUser()
      + ":"
      + p_proxy_settings->getAuthentificationPassword();

    if(p_proxy_settings->useDefaultServer() == false) {
      curl_easy_setopt(v_curl, CURLOPT_PROXY, v_proxy_server.c_str());
    }
    
    if(p_proxy_settings->useDefaultPort() == false) {
      curl_easy_setopt(v_curl, CURLOPT_PROXYPORT, p_proxy_settings->getPort());
    }
    
    curl_easy_setopt(v_curl, CURLOPT_PROXYTYPE, p_proxy_settings->getType());
   
    if(p_proxy_settings->useDefaultAuthentification() == false) {
      curl_easy_setopt(v_curl, CURLOPT_PROXYUSERPWD,
           v_proxy_auth_str.c_str());
    }
  }
  /* ***** */

  if(p_WebApp != NULL) {
    v_data.v_WebApp = p_WebApp;

    curl_easy_setopt(v_curl, CURLOPT_NOPROGRESS, false);
    curl_easy_setopt(v_curl, CURLOPT_PROGRESSFUNCTION, FSWeb::f_curl_progress_callback_upload);
    curl_easy_setopt(v_curl, CURLOPT_PROGRESSDATA, &v_data);
  }

  v_res = curl_easy_perform(v_curl);

  fclose(v_destinationFile);

  /* free the headers */
  curl_slist_free_all(v_headers);


  /* CURLE_ABORTED_BY_CALLBACK is not considered as an error */
  if(v_res != CURLE_ABORTED_BY_CALLBACK) {

    if(v_res != CURLE_OK) {
      char v_err[256];
      
      curl_easy_cleanup(v_curl);

      if(XMSession::instance()->debug() == false) {
	remove(p_answerFile.c_str());
      }      

      snprintf(v_err, 256, "error : unable to perform curl (curl[%i]: %s)",
	       v_res, curl_easy_strerror(v_res));
      
      throw Exception(v_err);
    }
  }
  curl_easy_cleanup(v_curl);

  /* analyse de la réponse */
  if(v_res == CURLE_ABORTED_BY_CALLBACK) {
    p_msg_status = 0;
    p_msg = "DbSync aborted";
  } else {
    uploadAnalyseMsg("xmoto_uploadDbSyncResult", p_answerFile, p_msg_status, p_msg);
  }

  // don't remove the answer file
  if(XMSession::instance()->debug() == false) {
    remove(p_answerFile.c_str());
  }
}

WebLevels::WebLevels(WWWAppInterface *p_WebLevelApp) {
  m_WebLevelApp    = p_WebLevelApp;
  m_proxy_settings = NULL;
  m_levels_url     = DEFAULT_WEBLEVELS_URL;
}

WebLevels::~WebLevels() {
}

void WebLevels::setWebsiteInfos(const std::string &p_url, const ProxySettings* p_proxy_settings)
{
  m_levels_url     = p_url;
  m_proxy_settings = p_proxy_settings;
}

std::string WebLevels::getXmlFileName() {
  return FS::getUserDir() + "/" + DEFAULT_WEBLEVELS_FILENAME;
}

void WebLevels::downloadXml()
{
  f_curl_download_data v_data;

  v_data.v_WebApp               = m_WebLevelApp;
  v_data.v_nb_files_performed   = 0;
  v_data.v_nb_files_to_download = 1;

  FSWeb::downloadFileBz2UsingMd5(getXmlFileName(),
				 m_levels_url,
				 FSWeb::f_curl_progress_callback_download,
				 &v_data,
				 m_proxy_settings);
}

std::string WebLevels::getDestinationDir() {
  return FS::getLevelsDir() + "/" + DEFAULT_WEBLEVELS_DIR;
}

void WebLevels::createDestinationDirIfRequired() {
  std::string v_destination_dir = getDestinationDir();

  if(FS::isDir(v_destination_dir) == false) {
    /* no mkdir() with microsoft C */
    FS::mkArborescence(v_destination_dir + "/file");
  }
}

std::string WebLevels::getDestinationFile(std::string p_url) {
  return getDestinationDir() + "/" + FS::getFileBaseName(p_url) + ".lvl";
}

void WebLevels::update(xmDatabase *i_db) {
  downloadXml();
  i_db->weblevels_updateDB(getXmlFileName());
}

int FSWeb::f_curl_progress_callback_upload(void *clientp,
             double dltotal,
             double dlnow,
             double ultotal,
             double ulnow) {
  f_curl_download_data *data = ((f_curl_download_data*) clientp);
  float v_percentage;

  /* cancel if it's wanted */
  if(data->v_WebApp->isCancelAsSoonAsPossible()) {
    return 1;
  }

  /* we can't make trust to the web server information */
  v_percentage = 0.0;
  if(ultotal > 0.0) {
    if((ulnow / ultotal) >= 0.0 && (ulnow / ultotal) <= 1.0) {
      v_percentage = (ulnow * 100.0) /ultotal;
    }
  }

  data->v_WebApp->setTaskProgress(v_percentage);

  return 0;
}

int FSWeb::f_curl_progress_callback_download(void *clientp,
               double dltotal,
               double dlnow,
               double ultotal,
               double ulnow) {
  f_curl_download_data *data = ((f_curl_download_data*) clientp);
  float real_percentage_already_done;
  float real_percentage_of_current_file;

  /* cancel if it's wanted */
  if(data->v_WebApp != NULL) {
    if(data->v_WebApp->isCancelAsSoonAsPossible()) {
      return 1;
    }
  }

  real_percentage_already_done = 
    (((float)data->v_nb_files_performed) * 100.0) 
    / ((float)data->v_nb_files_to_download);

  /* we can't make trust to the web server information */
  real_percentage_of_current_file = 0.0;
  if(dltotal > 0.0) {
    if((dlnow / dltotal) >= 0.0 && (dlnow / dltotal) <= 1.0) {
      real_percentage_of_current_file = 
  (dlnow * 100.0) 
  / (((float)data->v_nb_files_to_download) * dltotal);
    }
  }

  if(data->v_WebApp != NULL) {
    data->v_WebApp->setTaskProgress(real_percentage_already_done + real_percentage_of_current_file);
  }

  return 0;
}

int WebLevels::nbLevelsToGet(xmDatabase *i_db) const {
  return i_db->levels_nbLevelsToDownload();
}

void WebLevels::upgrade(xmDatabase *i_db) {
  char **v_result = NULL;
  unsigned int nrow;
  std::string v_levelId;
  std::string v_levelName;
  std::string v_urlFile;
  bool        v_isAnUpdate;
  std::string v_filePath;
  f_curl_download_data v_data;

  int v_nb_levels_to_download;
  int v_nb_levels_performed  = 0;
  float v_percentage;
  bool to_download;

  createDestinationDirIfRequired();

  v_result = i_db->readDB("SELECT a.id_level, a.name, a.fileUrl, b.filepath "
			  "FROM weblevels AS a "
			  "LEFT OUTER JOIN levels AS b ON a.id_level=b.id_level "
			  "WHERE b.id_level IS NULL OR a.checkSum <> b.checkSum;",
			  nrow);
  v_nb_levels_to_download = nrow;
  v_data.v_WebApp = m_WebLevelApp;
  v_data.v_nb_files_to_download = v_nb_levels_to_download;

  m_webLevelsNewDownloadedOK.clear();
  m_webLevelsUpdatedDownloadedOK.clear();

  try {
    /* download levels */
    for(unsigned int i=0; i<nrow; i++) {
      if(m_WebLevelApp != NULL) {
	if(m_WebLevelApp->isCancelAsSoonAsPossible()) {
	  break;
	}
      }

      v_levelId    = i_db->getResult(v_result, 4, i, 0);
      v_levelName  = i_db->getResult(v_result, 4, i, 1);
      v_urlFile    = i_db->getResult(v_result, 4, i, 2);
      v_isAnUpdate = i_db->getResult(v_result, 4, i, 3) != NULL;
      if(v_isAnUpdate) {
	v_filePath = i_db->getResult(v_result, 4, i, 3);
      }
      v_percentage = (((float)v_nb_levels_performed) * 100.0) / ((float)v_nb_levels_to_download);
	
      m_WebLevelApp->setTaskProgress(v_percentage);
      m_WebLevelApp->setBeingDownloadedInformation(v_levelName, v_isAnUpdate);
      
      /* should the level be updated */
      to_download = true;
      if(v_isAnUpdate) {
	/* does the user want to update the level ? */
	to_download = m_WebLevelApp->shouldLevelBeUpdated(v_levelId);
      }
      
      if(to_download) {
	v_data.v_nb_files_performed = v_nb_levels_performed;
	std::string v_destFile;
	
	if(v_isAnUpdate) {
	  if(FS::isInUserDir(v_filePath)) {
	    v_destFile = v_filePath;
	  } else {
	    v_destFile = WebLevels::getDestinationFile(v_urlFile);
	  }
	} else {
	  v_destFile = getDestinationFile(v_urlFile);
	}
	
	FSWeb::downloadFileBz2(v_destFile,
			       v_urlFile,
			       FSWeb::f_curl_progress_callback_download,
			       &v_data,
			       m_proxy_settings);
	
	if(v_isAnUpdate) {
	  m_webLevelsUpdatedDownloadedOK.push_back(v_destFile);
	} else {
	  m_webLevelsNewDownloadedOK.push_back(v_destFile);
	}
      }  
      
      v_nb_levels_performed++;
    }
  } catch(Exception &e) {
    i_db->read_DB_free(v_result);

    if(m_WebLevelApp != NULL) { // this is not an error in this case
      if(m_WebLevelApp->isCancelAsSoonAsPossible()) {
	return;
      }
    }

    throw e;
  }
  m_WebLevelApp->setTaskProgress(100.0);
  i_db->read_DB_free(v_result);
}

const std::vector<std::string> &WebLevels::getNewDownloadedLevels(void) {
  return m_webLevelsNewDownloadedOK;
}

const std::vector<std::string> &WebLevels::getUpdatedDownloadedLevels(void) {
  return m_webLevelsUpdatedDownloadedOK;
}

void WebThemes::updateTheme(xmDatabase* i_pDb, const std::string& i_id_theme, WWWAppInterface* i_WebLevelApp) {
  bool i_askThreadToEnd = false;

  if(i_WebLevelApp != NULL) {
    if(i_WebLevelApp->isCancelAsSoonAsPossible()) {
      return;
    }
  }

  try {
    LogInfo("WWW: Downloading a theme...");
    
    std::string v_destinationFile, v_destinationFileXML, v_destinationFileXML_tmp;
    std::string v_sourceFile;
    f_curl_download_data v_data;
    char **v_result;
    unsigned int nrow;
    std::string v_fileUrl;
    std::string v_filePath;
    std::string v_themeFile;
    bool v_onDisk = false;
    std::string v_md5Local;
    std::string v_md5Dist;
    bool v_all_downloaded = false;

    v_data.v_WebApp = i_WebLevelApp;
    
    /* download even if the the theme is uptodate
       it give a possibility to download file removed by mistake
    */
    v_result = i_pDb->readDB("SELECT a.fileUrl, b.filepath "
			     "FROM webthemes AS a LEFT OUTER JOIN themes AS b "
			     "ON a.id_theme = b.id_theme "
			     "WHERE a.id_theme=\"" + xmDatabase::protectString(i_id_theme) + "\";",
			     nrow);
    if(nrow != 1) {
      i_pDb->read_DB_free(v_result);
      throw Exception("Unable to found the theme on the website");
    }
    
    v_fileUrl = i_pDb->getResult(v_result, 2, 0, 0);
    if(i_pDb->getResult(v_result, 2, 0, 1) != NULL) {
      v_filePath = i_pDb->getResult(v_result, 2, 0, 1);
      v_onDisk = true;
    }  
    i_pDb->read_DB_free(v_result);
    
    // theme avaible on the web get it
    
    /* the destination file must be in the user dir */
    if(v_filePath != "") {
      if(FS::isInUserDir(v_filePath)) {
	v_destinationFileXML = v_filePath;
      }
    }

    if(v_destinationFileXML == "") {
      /* determine destination file */
      v_destinationFileXML = 
	FS::getUserDir() + "/" + THEMES_DIRECTORY + "/" + 
	FS::getFileBaseName(v_fileUrl) + ".xml";
    }
  
    v_themeFile = v_destinationFileXML;
    
    /* download the theme file */
    v_data.v_nb_files_performed   = 0;
    v_data.v_nb_files_to_download = 1;
    FS::mkArborescence(v_destinationFileXML);
    v_destinationFileXML_tmp = v_destinationFileXML + ".tmp";
    FSWeb::downloadFileBz2(v_destinationFileXML_tmp, v_fileUrl, FSWeb::f_curl_progress_callback_download, &v_data, XMSession::instance()->proxySettings());
    
    if(i_WebLevelApp != NULL) {
      if(i_WebLevelApp->isCancelAsSoonAsPossible()) {
	remove(v_destinationFileXML_tmp.c_str());
	return;
      }
    }

    /* download all the files required */
    Theme *v_theme = Theme::instance();
    std::vector<ThemeFile> *v_required_files;
    v_theme->load(v_destinationFileXML_tmp);
    v_required_files = v_theme->getRequiredFiles();

    // all files must be checked for md5sum
    int v_nb_files_to_download = 0;
    for(unsigned int i=0; i<v_required_files->size(); i++) {
      if(FS::fileExists((*v_required_files)[i].filepath) == false) {
	v_nb_files_to_download++;
      } else {
	v_md5Local = FS::md5sum((*v_required_files)[i].filepath);
	v_md5Dist  = (*v_required_files)[i].filemd5;
	if(v_md5Local != v_md5Dist && v_md5Dist != "") {
	  v_nb_files_to_download++;
	}
      }
    }
    
    if(v_nb_files_to_download != 0) {
      int v_nb_files_performed  = 0;
      
      v_data.v_nb_files_performed   = 0;
      v_data.v_nb_files_to_download = v_nb_files_to_download;
      
      unsigned int i = 0;
      if(i_WebLevelApp != NULL) {
	if(i_WebLevelApp->isCancelAsSoonAsPossible()) {
	  i_askThreadToEnd = true;
	}
      }

      while(i<v_required_files->size() && i_askThreadToEnd == false) {
	// download v_required_files[i]     
	v_destinationFile = FS::getUserDir() + std::string("/") + (*v_required_files)[i].filepath;
	v_sourceFile = XMSession::instance()->webThemesURLBase() + std::string("/") + (*v_required_files)[i].filepath;
	
	/* check md5 sums */
	v_md5Local = v_md5Dist = "";
	if(FS::fileExists((*v_required_files)[i].filepath) == true) {
	  v_md5Local = FS::md5sum((*v_required_files)[i].filepath);
	  v_md5Dist  = (*v_required_files)[i].filemd5;
	}
	
	/* if v_md5Dist == "", don't download ; it's a manually adding */
	if(FS::fileExists((*v_required_files)[i].filepath) == false || (v_md5Local != v_md5Dist && v_md5Dist != "")) {
	  v_data.v_nb_files_performed = v_nb_files_performed;
	  
	  float v_percentage = (((float)v_nb_files_performed) * 100.0) / ((float)v_nb_files_to_download);
	  if(i_WebLevelApp != NULL) {
	    i_WebLevelApp->setTaskProgress(v_percentage);
	    i_WebLevelApp->setBeingDownloadedInformation((*v_required_files)[i].filepath);
	  }

	  FS::mkArborescence(v_destinationFile);
	  
	  FSWeb::downloadFile(v_destinationFile, v_sourceFile, FSWeb::f_curl_progress_callback_download, &v_data,
			      XMSession::instance()->proxySettings());
	  
	  v_nb_files_performed++;
	}
	i++;

	if(i_WebLevelApp != NULL) {
	  if(i_WebLevelApp->isCancelAsSoonAsPossible()) {
	    i_askThreadToEnd = true;
	  }
	}
      }
      v_all_downloaded = i == v_required_files->size();

      if(i_WebLevelApp != NULL) {
	i_WebLevelApp->setTaskProgress(100);
      }
    } else {
      v_all_downloaded = true;
    }

    /* full downloading, put the xml */
    if(v_all_downloaded) {
      remove(v_destinationFileXML.c_str());
      if(rename(v_destinationFileXML_tmp.c_str(), v_destinationFileXML.c_str()) != 0) {
	remove(v_destinationFileXML_tmp.c_str());
	throw Exception("Failed to update the theme");
	return;
      } else {
	if(v_onDisk) {
	  i_pDb->themes_update(i_id_theme, v_themeFile);
	} else {
	  i_pDb->themes_add(i_id_theme, v_themeFile);
	}
      }
    } else {
      remove(v_destinationFileXML_tmp.c_str());
    }

  } catch(Exception &e) {
    if(i_WebLevelApp != NULL) {
      if(i_WebLevelApp->isCancelAsSoonAsPossible()) {
	return;
      }
    }
    
    LogWarning("Failed to update theme %s (%s)", i_id_theme.c_str(), e.getMsg().c_str());
    throw Exception("Failed to update the theme");
    return;
  }
  
}

void WebThemes::updateThemeList(xmDatabase* i_pDb, WWWAppInterface* i_WebLevelApp) {
  std::string v_destinationFile = FS::getUserDir() + "/" + DEFAULT_WEBTHEMES_FILENAME;
  f_curl_download_data v_data;
  
  if(i_WebLevelApp != NULL) {
    if(i_WebLevelApp->isCancelAsSoonAsPossible()) {
      return;
    }
  }

  LogInfo("WWW: Checking for new or updated themes...");
  v_data.v_WebApp = i_WebLevelApp;
  v_data.v_nb_files_performed   = 0;
  v_data.v_nb_files_to_download = 1;
  FSWeb::downloadFileBz2UsingMd5(v_destinationFile, XMSession::instance()->webThemesURL(),
				 FSWeb::f_curl_progress_callback_download, &v_data, XMSession::instance()->proxySettings());
  i_pDb->webthemes_updateDB(v_destinationFile);
}

bool WebThemes::isUpdatable(xmDatabase* i_pDb, const std::string& i_id_theme) {
  char **v_result;
  unsigned int nrow;

  v_result = i_pDb->readDB("SELECT id_theme FROM webthemes WHERE id_theme=\"" + xmDatabase::protectString(i_id_theme) + "\";",
			   nrow);
  if(nrow == 1) {
    i_pDb->read_DB_free(v_result);
    return true;
  }
  i_pDb->read_DB_free(v_result);
  return false;
}

void XMSync::syncUp(xmDatabase* i_pDb, const ProxySettings* pProxySettings,
		    const std::string& i_sitekey, const std::string& i_profile) {
  // sync_buildServerFile(const std::string& i_outFile, const std::string& i_sitekey, const std::string& i_profile)
}

void XMSync::syncDown() {
}
