/* file contributed by Nicolas Adenis-Lamarre */

#include "BuildConfig.h"

/* I (rasmus) made these (among others) changes to the files:  
    - File names changed to WWW.cpp and WWW.h
    - vapp::FS::getReplaysDir(), not vapp::FS::getReplayDir()
    - vapp::FS::getLevelsDir(), not vapp::FS::getLevelDir()
    - Interface class in WWWAppInterface.h (also renamed class to WWWAppInterface
    - USE_HASH_MAP specifies whether hash_map or (stupid) vector should be used. 
      Still too lazy to implement hash_map on windows :)
    - no mkdir() in vs.net
    - remove() also a bit different in vs.net
    - added cleanHash() to WebRoom destructor
 */

#include "WWW.h"
#include "VApp.h"
#include "VFileIO.h"
#include "helpers/VExcept.h"
#include "VXml.h"
#include <curl/curl.h>

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
#include "md5sum/md5file.h"
#include "db/xmDatabase.h"

struct f_curl_download_data {
  vapp::WWWAppInterface *v_WebApp;
  int v_nb_files_to_download;
  int v_nb_files_performed;
};

struct f_curl_upload_data {
  vapp::WWWAppInterface *v_WebApp;
};

ProxySettings::ProxySettings() {
  m_server       = "";
  m_port         = -1;
  m_type         = CURLPROXY_HTTP;
  m_authUser     = "";
  m_authPassword = "";
}

void ProxySettings::setServer(std::string p_server) {
  m_server = p_server;
}

void ProxySettings::setPort(long p_port) {
  m_port = p_port;
}

void ProxySettings::setType(long p_type) {
  m_type = p_type;
}

void ProxySettings::setAuthentification(std::string p_user, std::string p_password) {
  m_authUser     = p_user;
  m_authPassword = p_password;
}

void ProxySettings::setDefaultServer() {
  m_server = "";
}

void ProxySettings::setDefaultPort() {
  m_port = -1;
}

void ProxySettings::setDefaultType() {
  m_type = CURLPROXY_HTTP;
}

void ProxySettings::setDefaultAuthentification() {
  m_authUser     = "";
  m_authPassword = "";
}

std::string ProxySettings::getServer() const {
  return m_server;
}

long ProxySettings::getPort() const {
  return m_port;
}

long ProxySettings::getType() const {
  return m_type;
}

std::string ProxySettings::getAuthentificationUser() const {
  return m_authUser;
}

std::string ProxySettings::getAuthentificationPassword() const {
  return m_authPassword;
}

bool ProxySettings::useDefaultServer() const {
  return m_server == "";
}

bool ProxySettings::useDefaultPort() const {
  return m_port == -1;
}

bool ProxySettings::useDefaultAuthentification() const {
  return m_authUser == "";
}

void WebRoom::downloadReplay(const std::string& i_url) {
  std::string i_rplFilename = vapp::FS::getReplaysDir()
    + "/" 
    + vapp::FS::getFileBaseName(i_url) 
    + ".rpl";

  FSWeb::downloadFile(i_rplFilename, i_url, NULL, NULL, m_proxy_settings);
}

WebRoom::WebRoom(const ProxySettings *p_proxy_settings) {
  std::string v_userDir;
  v_userDir = vapp::FS::getUserDir();
  m_userFilename =  v_userDir 
    + "/" 
    + DEFAULT_WEBHIGHSCORES_FILENAME;
  m_webhighscores_url = DEFAULT_WEBHIGHSCORES_URL;

  m_proxy_settings = p_proxy_settings;
}

WebRoom::~WebRoom() {
}

std::string WebRoom::getRoomId() const {
  return m_roomId;
}

void WebRoom::setWebsiteInfos(const std::string& i_id_room, const std::string& i_webhighscores_url) {
  m_webhighscores_url = i_webhighscores_url;
  m_roomId = i_id_room;
}

void WebRoom::update() {
  /* download xml file */
  FSWeb::downloadFileBz2UsingMd5(m_userFilename,
         m_webhighscores_url,
         NULL,
         NULL,
         m_proxy_settings);
}

void WebRoom::upgrade(xmDatabase *i_db) {
  i_db->webhighscores_updateDB(m_userFilename, m_webhighscores_url);
}

size_t FSWeb::writeData(void *ptr, size_t size, size_t nmemb, FILE *stream) {
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
    if(vapp::FS::isFileReadable(p_local_file) == true) {
      std::string v_md5Local  = md5file(p_local_file);
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

  vapp::Verbose(std::string("downloading " + 
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
  if(v_curl == false) {
    fclose(v_destinationFile);
    throw Exception("error : unable to init curl"); 
  }

  curl_easy_setopt(v_curl, CURLOPT_URL, p_web_file.c_str());
  curl_easy_setopt(v_curl, CURLOPT_WRITEDATA, v_destinationFile);
  curl_easy_setopt(v_curl, CURLOPT_WRITEFUNCTION, FSWeb::writeData);
  curl_easy_setopt(v_curl, CURLOPT_TIMEOUT, DEFAULT_TRANSFERT_TIMEOUT);
  curl_easy_setopt(v_curl, CURLOPT_CONNECTTIMEOUT, DEFAULT_TRANSFERT_CONNECT_TIMEOUT);
  curl_easy_setopt(v_curl, CURLOPT_USERAGENT,  v_www_agent.c_str());
  curl_easy_setopt(v_curl, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt(v_curl, CURLOPT_FOLLOWLOCATION, 1);

  /* set proxy settings */
  if(p_proxy_settings != NULL) {
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
      vapp::Verbose(std::string("set proxy ->" + v_proxy_server).c_str());
      curl_easy_setopt(v_curl, CURLOPT_PROXY, v_proxy_server.c_str());
    }
    
    if(p_proxy_settings->useDefaultPort() == false) {
      curl_easy_setopt(v_curl, CURLOPT_PROXYPORT, p_proxy_settings->getPort());
    }
    
    curl_easy_setopt(v_curl, CURLOPT_PROXYTYPE, p_proxy_settings->getType());
   
    if(p_proxy_settings->useDefaultAuthentification() == false) {
      vapp::Verbose(std::string("set proxy authentification ->" + v_proxy_auth_str).c_str());
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
    remove(v_local_file_tmp.c_str());
    throw Exception("error : unable to download file "
        + p_web_file);  
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

void FSWeb::uploadReplay(std::string p_replayFilename,
       std::string p_id_room,
       std::string p_login,
       std::string p_password,
       std::string p_url_to_transfert,
       vapp::WWWAppInterface *p_WebApp,
       const ProxySettings *p_proxy_settings,
       bool &p_msg_status,
       std::string &p_msg) {
  CURL *v_curl;
  CURLcode v_res;
  std::string v_proxy_server;
  std::string v_proxy_auth_str;
  std::string v_www_agent = WWW_AGENT;
  f_curl_upload_data v_data;

  FILE *v_destinationFile;
  std::string v_local_file;
  v_local_file = vapp::FS::getUserDir() + "/" + DEFAULT_REPLAYUPLOAD_MSGFILE;

  //printf("%s\n", v_local_file.c_str());

  struct curl_httppost *v_post, *v_last;

  vapp::Verbose(std::string("Uploading replay " + p_replayFilename).c_str());

  /* open the file */
  if( (v_destinationFile = fopen(v_local_file.c_str(), "wb")) == false) {
    throw Exception("error : unable to open output file " DEFAULT_REPLAYUPLOAD_MSGFILE);
  }
      
  v_curl = curl_easy_init();
  if(v_curl == false) {
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
  curl_easy_setopt(v_curl, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt(v_curl, CURLOPT_FOLLOWLOCATION, 1);

  /* set proxy settings */
  if(p_proxy_settings != NULL) {
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
      vapp::Verbose(std::string("set proxy ->" + v_proxy_server).c_str());
      curl_easy_setopt(v_curl, CURLOPT_PROXY, v_proxy_server.c_str());
    }
    
    if(p_proxy_settings->useDefaultPort() == false) {
      curl_easy_setopt(v_curl, CURLOPT_PROXYPORT, p_proxy_settings->getPort());
    }
    
    curl_easy_setopt(v_curl, CURLOPT_PROXYTYPE, p_proxy_settings->getType());
   
    if(p_proxy_settings->useDefaultAuthentification() == false) {
      vapp::Verbose(std::string("set proxy authentification ->" + v_proxy_auth_str).c_str());
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

  /* CURLE_ABORTED_BY_CALLBACK is not considered as an error */
  if(v_res != CURLE_ABORTED_BY_CALLBACK) {
    if(v_res != CURLE_OK) {
      curl_easy_cleanup(v_curl);
      remove(v_local_file.c_str());
      throw Exception("error : unable to perform curl");  
    }
  }
  curl_easy_cleanup(v_curl);

  /* analyse de la réponse */
  if(v_res == CURLE_ABORTED_BY_CALLBACK) {
    p_msg_status = 0;
    p_msg = "Upload aborted\nYou will be able to validate it later ;-)";
  } else {
    uploadReplayAnalyseMsg(v_local_file, p_msg_status, p_msg);
  }

  remove(v_local_file.c_str());
}

void FSWeb::uploadReplayAnalyseMsg(std::string p_filename,
           bool &p_msg_status_ok,
           std::string &p_msg) {
  vapp::XMLDocument v_Xml;
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
  v_XmlDataElement = v_XmlData->FirstChildElement("xmoto_uploadReplayResult");
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

WebLevels::WebLevels(vapp::WWWAppInterface *p_WebLevelApp,
         const ProxySettings *p_proxy_settings) {
  m_WebLevelApp    = p_WebLevelApp;
  m_proxy_settings = p_proxy_settings;
  m_levels_url     = DEFAULT_WEBLEVELS_URL;
}

WebLevels::~WebLevels() {
}

std::string WebLevels::getXmlFileName() {
  return vapp::FS::getUserDir() + "/" + DEFAULT_WEBLEVELS_FILENAME;
}

void WebLevels::downloadXml() {
  FSWeb::downloadFileBz2UsingMd5(getXmlFileName(),
         m_levels_url,
         NULL,
         NULL,
         m_proxy_settings);
}

std::string WebLevels::getDestinationDir() {
  return vapp::FS::getLevelsDir() + "/" + DEFAULT_WEBLEVELS_DIR;
}

void WebLevels::createDestinationDirIfRequired() {
  std::string v_destination_dir = getDestinationDir();

  if(vapp::FS::isDir(v_destination_dir) == false) {
    /* no mkdir() with microsoft C */
    vapp::FS::mkArborescence(v_destination_dir + "/file");
  }
}

std::string WebLevels::getDestinationFile(std::string p_url) {
  return getDestinationDir() + "/" + vapp::FS::getFileBaseName(p_url) + ".lvl";
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
  if(data->v_WebApp->isCancelAsSoonAsPossible()) {
    return 1;
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

  data->v_WebApp->setTaskProgress(real_percentage_already_done + real_percentage_of_current_file);

  return 0;
}

int WebLevels::nbLevelsToGet(xmDatabase *i_db) const {
  char **v_result;
  int nrow;
  v_result = i_db->readDB("SELECT a.name FROM weblevels AS a "
			  "LEFT OUTER JOIN levels AS b ON a.id_level=b.id_level "
			  "WHERE b.id_level IS NULL OR a.checkSum <> b.checkSum;",
			  nrow);
  i_db->read_DB_free(v_result);
  return nrow;
}

void WebLevels::upgrade(xmDatabase *i_db) {
  char **v_result;
  int nrow;
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
      if(m_WebLevelApp->isCancelAsSoonAsPossible()) break;

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
	  if(vapp::FS::isInUserDir(v_filePath)) {
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
      m_WebLevelApp->readEvents(); 
    }
  } catch(Exception &e) {
    i_db->read_DB_free(v_result);
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

WebTheme::WebTheme(std::string pName, std::string pUrl, std::string pSum) {
  m_name = pName;
  m_url  = pUrl;
  m_sum  = pSum;
}

WebTheme::~WebTheme() {
}

std::string WebTheme::getName() const {
  return m_name;
}

std::string WebTheme::getUrl() const {
  return m_url;
}

std::string WebTheme::getSum() const {
  return m_sum;
}

WebThemes::WebThemes(vapp::WWWAppInterface *p_WebApp,
         const ProxySettings *p_proxy_settings) {
  m_proxy_settings = p_proxy_settings;
  m_themes_url     = DEFAULT_WEBTHEMES_URL;
  m_themes_urlBase = DEFAULT_WEBTHEMES_SPRITESURLBASE;
  m_WebApp = p_WebApp;
}

WebThemes::~WebThemes() {
  clean();
}

void WebThemes::update() {
  FSWeb::downloadFileBz2UsingMd5(getXmlFileName(),
         m_themes_url,
         NULL,
         NULL,
         m_proxy_settings);
}

void WebThemes::upgrade() {
  clean();
  extractThemesAvailableFromXml();
}

std::string WebThemes::getXmlFileName() {
  return vapp::FS::getUserDir() + "/" + DEFAULT_WEBTHEMES_FILENAME;
}

void WebThemes::extractThemesAvailableFromXml() {
  vapp::XMLDocument v_webTXml;
  TiXmlDocument *v_webTXmlData;
  TiXmlElement *v_webTXmlDataElement;
  const char *pc;
  std::string v_themeName, v_url, v_MD5sum_web;
  
  v_webTXml.readFromFile(getXmlFileName());
  v_webTXmlData = v_webTXml.getLowLevelAccess();

  if(v_webTXmlData == NULL) {
    throw Exception("error : unable to analyze xml theme file");
  }

  v_webTXmlDataElement = v_webTXmlData->FirstChildElement("xmoto_themes");
  
  if(v_webTXmlDataElement == NULL) {
    throw Exception("error : unable to analyze xml theme file");
  }

  TiXmlElement *pVarElem = v_webTXmlDataElement->FirstChildElement("theme");
  while(pVarElem != NULL) {
    
    pc = pVarElem->Attribute("name");
    if(pc != NULL) {
      v_themeName = pc;
  
      pc = pVarElem->Attribute("url");
      if(pc != NULL) {
  v_url = pc;  
    
  pc = pVarElem->Attribute("sum");
  if(pc != NULL) {
    v_MD5sum_web = pc;

    m_availableThemes.push_back(new WebTheme(v_themeName, v_url, v_MD5sum_web));
      
  }
      }
      pVarElem = pVarElem->NextSiblingElement("theme");
    }
  }
}

void WebThemes::clean() {
  for(unsigned int i=0;i<m_availableThemes.size();i++) {
    delete m_availableThemes[i];
  }    

  m_availableThemes.clear();
}

const std::vector<WebTheme*> &WebThemes::getAvailableThemes() {
  return m_availableThemes;
}

bool WebThemes::isUpgradable(ThemeChoice *p_themeChoice) {
  for(unsigned int i=0; i<m_availableThemes.size(); i++) {
    if(m_availableThemes[i]->getName() == p_themeChoice->ThemeName()) {
      return true;
    }
  }
  return false;
}

void WebThemes::upgrade(ThemeChoice *p_themeChoice) {
  std::string v_destinationFile;
  std::string v_sourceFile;
  WebTheme *v_associated_webtheme;
  f_curl_download_data v_data;
  
  /* no update required */
  if(p_themeChoice->getHosted() && p_themeChoice->getRequireUpdate() == false) {
    // do nothing : it give a possibility to download file removed by mistake
  }

  /* find the associated webtheme */
  v_associated_webtheme = NULL;
  for(unsigned int i=0; i<m_availableThemes.size(); i++) {
    if(m_availableThemes[i]->getName() == p_themeChoice->ThemeName()) {
      v_associated_webtheme = m_availableThemes[i];
    }
  }

  if(v_associated_webtheme == NULL) {
    throw Exception("error : unable to find this theme on the web");
  }
  // theme avaible on the web get it

  /* the destination file must be in the user dir */
  if(vapp::FS::isInUserDir(p_themeChoice->ThemeFile())) {
    v_destinationFile = p_themeChoice->ThemeFile();
  }

  if(v_destinationFile == "") {
    /* determine destination file */
    v_destinationFile = 
      vapp::FS::getUserDir() + "/" + THEMES_DIRECTORY + "/" + 
      vapp::FS::getFileBaseName(v_associated_webtheme->getUrl()) + ".xml";
  }    
  
  /* download the theme file */
  vapp::FS::mkArborescence(v_destinationFile);
  FSWeb::downloadFileBz2(v_destinationFile,
       v_associated_webtheme->getUrl(),
       NULL,
       NULL,
       m_proxy_settings);

  if(v_destinationFile == "") {
    throw Exception("error : unable to determine theme destination file");
  }

  /* download all the files required */
  Theme *v_theme = new Theme();
  std::vector<std::string> *v_required_files;
  v_theme->load(v_destinationFile);
  v_required_files = v_theme->getRequiredFiles();

  int v_nb_files_to_download = 0;
  for(unsigned int i=0; i<v_required_files->size(); i++) {
    if(vapp::FS::fileExists((*v_required_files)[i]) == false) {
      v_nb_files_to_download++;
    }
  }
  if(v_nb_files_to_download != 0) {

    int v_nb_files_performed  = 0;
    
    v_data.v_WebApp = m_WebApp;
    v_data.v_nb_files_to_download = v_nb_files_to_download;

    unsigned int i = 0;
    while(i<v_required_files->size() && m_WebApp->isCancelAsSoonAsPossible() == false) {
      // download v_required_files[i]     
      v_destinationFile = vapp::FS::getUserDir() + std::string("/") + (*v_required_files)[i];
      v_sourceFile = m_themes_urlBase + 
  std::string("/") + (*v_required_files)[i];
      
      if(vapp::FS::fileExists((*v_required_files)[i]) == false) {
  v_data.v_nb_files_performed = v_nb_files_performed;
  
  float v_percentage = (((float)v_nb_files_performed) * 100.0) / ((float)v_nb_files_to_download);
  m_WebApp->setTaskProgress(v_percentage);
  m_WebApp->setBeingDownloadedInformation((*v_required_files)[i], true);

  vapp::FS::mkArborescence(v_destinationFile);

  FSWeb::downloadFile(v_destinationFile,
          v_sourceFile,
          FSWeb::f_curl_progress_callback_download,
          &v_data,
          m_proxy_settings);
  
  v_nb_files_performed++;
      }
      m_WebApp->readEvents();
      i++;
    }
    m_WebApp->setTaskProgress(100.0);
  }

  delete v_theme;

  p_themeChoice->setRequireUpdate(false);
  p_themeChoice->setHosted(true);
}

WebRooms::WebRooms(const ProxySettings *p_proxy_settings) {
  m_proxy_settings = p_proxy_settings;
}

WebRooms::~WebRooms() {
}

void WebRooms::update() {
  FSWeb::downloadFileBz2UsingMd5(getXmlFileName(),
         m_rooms_url,
         NULL,
         NULL,
         m_proxy_settings);
}

std::string WebRooms::getXmlFileName() {
  return vapp::FS::getUserDir() + "/" + DEFAULT_WEBROOMS_FILENAME;
}

void WebRooms::upgrade(xmDatabase *i_db) {
  i_db->webrooms_updateDB(getXmlFileName());
}
