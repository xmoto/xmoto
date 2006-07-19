/* file contributed by Nicolas Adenis-Lamarre */

#include "BuildConfig.h"
#if defined(SUPPORT_WEBACCESS)

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
#include "VExcept.h"
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
#include "compression/FileCompression.h"
#include "md5sum/md5file.h"

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

WebHighscore::WebHighscore(WebRoom *p_room,
			   std::string p_levelId,
			   std::string p_playerName,
			   std::string p_time,
			   std::string p_rplUrl,
			   const ProxySettings *p_proxy_settings) {
  m_playerName  = p_playerName;
  m_time        = p_time;
  m_levelId     = p_levelId;
  m_rplUrl      = p_rplUrl;
  m_rplFilename = vapp::FS::getReplaysDir()
    + "/" 
    + vapp::FS::getFileBaseName(m_rplUrl) 
    + ".rpl";

  m_proxy_settings = p_proxy_settings;
  m_room = p_room;
}

WebHighscore::~WebHighscore() {
}

void WebHighscore::download() {
  FSWeb::downloadFile(m_rplFilename, m_rplUrl, NULL, NULL, m_proxy_settings);
}

std::string WebHighscore::getReplayName() {
  return vapp::FS::getFileBaseName(m_rplFilename);
}

std::string WebHighscore::getPlayerName() const {
  return m_playerName;
}

std::string WebHighscore::getLevelId() const {
  return m_levelId;
}

std::string WebHighscore::getTime() const {
  char cTime[256];
  int n1=0,n2=0,n3=0;
  
  sscanf(m_time.c_str(),"%d:%d:%d",&n1,&n2,&n3);
  sprintf(cTime,"%02d:%02d:%02d",n1,n2,n3);
  
  return cTime;
}

float WebHighscore::getFTime() const {
  std::string::size_type v_pos;
  std::string v_min, v_sec, v_hun;
  std::string v_rest;
  float v_fmin, v_fsec, v_fhun;

  v_rest = m_time;

  /* search min */
  v_pos = v_rest.find(":", 0);
  if(v_pos == std::string::npos || v_pos == 0) {
    throw vapp::Exception("error : invalid time in webhighscore");
  }
  v_min = v_rest.substr(0, v_pos);
  if(v_pos == v_rest.length() -1) {
    throw vapp::Exception("error : invalid time in webhighscore");
  }
  v_rest = v_rest.substr(v_pos+1, v_rest.length() -v_pos -1);

  /* search sec */
  v_pos = v_rest.find(":", 0);
  if(v_pos == std::string::npos || v_pos == 0) {
    throw vapp::Exception("error : invalid time in webhighscore");
  }
  v_sec = v_rest.substr(0, v_pos);
  if(v_pos == v_rest.length() -1) {
    throw vapp::Exception("error : invalid time in webhighscore");
  }
  v_rest = v_rest.substr(v_pos+1, v_rest.length() -v_pos -1);

  /* search hun */
  v_hun = v_rest;

  v_fmin = atof(v_min.c_str());
  v_fsec = atof(v_sec.c_str());
  v_fhun = atof(v_hun.c_str());

  return v_fmin * 60.0 + v_fsec + v_fhun / 100.0;
}

WebRoom* WebHighscore::getRoom() const {
  return m_room;
}

WebRoom::WebRoom(const ProxySettings *p_proxy_settings) {
  std::string v_userDir;
  v_userDir = vapp::FS::getUserDir();
  m_userFilename =  v_userDir 
    + "/" 
    + DEFAULT_WEBHIGHSCORES_FILENAME;
  m_webhighscores_url = DEFAULT_WEBHIGHSCORES_URL;

  m_proxy_settings = p_proxy_settings;
  m_roomName = "";
}

WebRoom::~WebRoom() {
  cleanHash();
}

std::string WebRoom::getRoomName() const {
  return m_roomName;
}

void WebRoom::setWebsiteURL(std::string p_webhighscores_url) {
  m_webhighscores_url = p_webhighscores_url;
}

WebHighscore* WebRoom::getHighscoreFromLevel(const std::string &p_levelId) {
  #if defined(USE_HASH_MAP)
    return m_webhighscores[p_levelId.c_str()];
  #else
    for(int i=0;i<m_webhighscores.size();i++)
      if(m_webhighscores[i]->getLevelId() == p_levelId) return m_webhighscores[i];
    return NULL;  
  #endif
}

void WebRoom::cleanHash() {
  #if defined(USE_HASH_MAP)
    HashNamespace::hash_map<const char*, WebHighscore*, HashNamespace::hash<const char*>, highscore_str, std::allocator<WebHighscore*> >::iterator it;

    it = m_webhighscores.begin();
    while(it != m_webhighscores.end()) {
      /* can be NULL, because search on unexisting value create an entry */
      if(it->second != NULL) { 
        delete it->second; /* delete highscore */
      }
      it++;
    } 
  #else
    for(int i=0;i<m_webhighscores.size();i++)
      delete m_webhighscores[i];
  #endif

  /* clear hash table */
  m_webhighscores.clear();
}

void WebRoom::fillHash() {
  /* clean hash table */
  cleanHash();

  /* fillHashTable */
  vapp::XMLDocument v_webHSXml;
  TiXmlDocument *v_webHSXmlData;
  TiXmlElement *v_webHSXmlDataElement;
  const char *pc;
  std::string v_levelId, v_playerName, v_time, v_rplUrl;
  WebHighscore *wh;

  v_webHSXml.readFromFile(m_userFilename);
  v_webHSXmlData = v_webHSXml.getLowLevelAccess();

  if(v_webHSXmlData == NULL) {
    throw vapp::Exception("error : unable analyse xml highscore file");
  }

  v_webHSXmlDataElement = v_webHSXmlData->FirstChildElement("xmoto_worldrecords");

  if(v_webHSXmlDataElement != NULL) {
    /* get Room name */
    pc = v_webHSXmlDataElement->Attribute("roomname");
    if(pc != NULL) {
      m_roomName = pc;
    }

    for(TiXmlElement *pVarElem = v_webHSXmlDataElement->FirstChildElement("worldrecord");
	pVarElem!=NULL;
	pVarElem = pVarElem->NextSiblingElement("worldrecord")
	) {

      pc = pVarElem->Attribute("level_id");
      if(pc == NULL) { continue; }
      v_levelId = pc;

      pc = pVarElem->Attribute("player");
      if(pc == NULL) { continue; }
      v_playerName = pc;	 

      pc = pVarElem->Attribute("time");
      if(pc == NULL) { continue; }
      v_time = pc;	 

      pc = pVarElem->Attribute("replay");
      if(pc == NULL) { continue; }
      v_rplUrl = pc;
      
      wh = new WebHighscore(this, v_levelId, v_playerName, v_time, v_rplUrl, m_proxy_settings);
      
      #if defined(USE_HASH_MAP)
        m_webhighscores[wh->getLevelId().c_str()] = wh;
      #else
        m_webhighscores.push_back(wh);
      #endif      
    }
  }
}

void WebRoom::update() {
  /* download xml file */
  FSWeb::downloadFileBz2UsingMd5(m_userFilename,
				 m_webhighscores_url,
				 NULL,
				 NULL,
				 m_proxy_settings);
}

void WebRoom::upgrade() {
  fillHash();
}

size_t FSWeb::writeData(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  return fwrite (ptr, size, nmemb, stream);
}

void FSWeb::downloadFileBz2(const std::string &p_local_file,
			    const std::string &p_web_file,
			    int (*curl_progress_callback)(void *clientp,
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
	       curl_progress_callback,
	       p_data,
	       p_proxy_settings
	       );

  try {
    FileCompression::bunzip2(v_bzFile, p_local_file);
    remove(v_bzFile.c_str());
  } catch(vapp::Exception &e) {
    remove(v_bzFile.c_str());
    throw e;
  }
}

void FSWeb::downloadFileBz2UsingMd5(const std::string &p_local_file,
				    const std::string &p_web_file,
				    int (*curl_progress_callback)(void *clientp,
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
  } catch(vapp::Exception &e) {
    /* no pb, just dwd */
  }
    
  if(require_dwd) {
    FSWeb::downloadFileBz2(p_local_file,
			   p_web_file,
			   curl_progress_callback,
			   p_data,
			   p_proxy_settings);
  }
}

void FSWeb::downloadFile(const std::string &p_local_file,
			 const std::string &p_web_file,
			 int (*curl_progress_callback)(void *clientp,
						       double dltotal,
						       double dlnow,
						       double ultotal,
						       double ulnow),
			 void *p_data,
			 const ProxySettings *p_proxy_settings
			 ) {

  CURL *v_curl;
  CURLcode res;
  FILE *v_destinationFile;

  std::string v_local_file_tmp = p_local_file + ".part";

  std::string v_proxy_server;
  std::string v_proxy_auth_str;
  std::string v_www_agent = WWW_AGENT;

  /* open the file */
  if( (v_destinationFile = fopen(v_local_file_tmp.c_str(), "wb")) == false) {
    throw vapp::Exception("error : unable to open output file " 
			  + v_local_file_tmp);
  }

  /* download the file */
  v_curl = curl_easy_init();
  if(v_curl == false) {
    fclose(v_destinationFile);
    throw vapp::Exception("error : unable to init curl");	
  }

  curl_easy_setopt(v_curl, CURLOPT_URL, p_web_file.c_str());
  curl_easy_setopt(v_curl, CURLOPT_WRITEDATA, v_destinationFile);
  curl_easy_setopt(v_curl, CURLOPT_WRITEFUNCTION, FSWeb::writeData);
  curl_easy_setopt(v_curl, CURLOPT_TIMEOUT, DEFAULT_TRANSFERT_TIMEOUT);
  curl_easy_setopt(v_curl, CURLOPT_CONNECTTIMEOUT, DEFAULT_TRANSFERT_CONNECT_TIMEOUT);
  curl_easy_setopt(v_curl, CURLOPT_USERAGENT,  v_www_agent.c_str());
  curl_easy_setopt(v_curl, CURLOPT_FAILONERROR, 1);

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
      + " "
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

  if(curl_progress_callback != NULL) {
    curl_easy_setopt(v_curl, CURLOPT_NOPROGRESS, false);
    curl_easy_setopt(v_curl, CURLOPT_PROGRESSFUNCTION, curl_progress_callback);
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
    throw vapp::Exception("error : unable to download file "
			  + p_web_file);	
  }

  /* replace file */
  /* On windows you can't rename FILE1 to FILE2 if FILE2 already exists...
     So we need to delete it first. */
  remove(p_local_file.c_str());
  if(rename(v_local_file_tmp.c_str(), p_local_file.c_str()) != 0) {
    remove(v_local_file_tmp.c_str());
    throw vapp::Exception("error : unable to write output file " 
			  + p_local_file);
  }
}

WebLevel::WebLevel(std::string p_id, std::string p_name, std::string p_url) {
  m_id   = p_id;
  m_name = p_name;
  m_url  = p_url;
  m_require_update = false;
}

std::string WebLevel::getId() const {
  return m_id;
}

std::string WebLevel::getName() const {
  return m_name;
}

std::string WebLevel::getUrl() const {
  return m_url;
}

bool WebLevel::requireUpdate() const {
  return m_require_update;
}

void WebLevel::setRequireUpdate(bool p_require_update) {
  m_require_update = p_require_update;
}

void WebLevel::setCurrentPath(std::string p_current_path) {
  m_current_path = p_current_path;
}

std::string WebLevel::getCurrentPath() const {
  return m_current_path;
}

WebLevels::WebLevels(vapp::WWWAppInterface *p_WebLevelApp,
		     const ProxySettings *p_proxy_settings) {
  m_WebLevelApp    = p_WebLevelApp;
  m_proxy_settings = p_proxy_settings;
  m_levels_url     = DEFAULT_WEBLEVELS_URL;
}

WebLevels::~WebLevels() {
  /* delete levels information */
  std::vector<WebLevel*>::iterator it; 
  for(it = m_webLevels.begin(); it != m_webLevels.end(); it++) {
    delete *it;
  }
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
    if(vapp::FS::mkDir(v_destination_dir.c_str()) != 0) { /* drwx------ */
      throw vapp::Exception("error : unable to make directory "
			    + v_destination_dir);	
    }
  }
}

void WebLevels::extractLevelsToDownloadFromXml() {
  vapp::XMLDocument v_webLXml;
  TiXmlDocument *v_webLXmlData;
  TiXmlElement *v_webLXmlDataElement;
  const char *pc;
  std::string v_levelId, v_levelName, v_url, v_MD5sum_web;
  
  v_webLXml.readFromFile(getXmlFileName());
  v_webLXmlData = v_webLXml.getLowLevelAccess();

  if(v_webLXmlData == NULL) {
    throw vapp::Exception("error : unable analyse xml level file");
  }

  v_webLXmlDataElement = v_webLXmlData->FirstChildElement("xmoto_levels");
  
  if(v_webLXmlDataElement == NULL) {
    throw vapp::Exception("error : unable analyse xml level file");
  }

  TiXmlElement *pVarElem = v_webLXmlDataElement->FirstChildElement("level");
  while(pVarElem != NULL) {
    
    pc = pVarElem->Attribute("level_id");
    if(pc != NULL) {
      v_levelId = pc;
	
      pc = pVarElem->Attribute("name");
      if(pc != NULL) {
	v_levelName = pc;
	
	pc = pVarElem->Attribute("url");
	if(pc != NULL) {
	  v_url = pc;	 
	  
	  pc = pVarElem->Attribute("sum");
	  if(pc != NULL) {
	    v_MD5sum_web = pc;	

	    /* if it doesn't exist */
	    if(m_WebLevelApp->doesLevelExist(v_levelId) == false) {
	      m_webLevels.push_back(new WebLevel(v_levelId, v_levelName, v_url));
	    } else { /* or it md5sum if different */
	      if(m_WebLevelApp->levelPathForUpdate(v_levelId) != "") {
		if(m_WebLevelApp->levelMD5Sum(v_levelId) != v_MD5sum_web) {
		  WebLevel *v_lvl = new WebLevel(v_levelId, v_levelName, v_url);
		  v_lvl->setCurrentPath(m_WebLevelApp->levelPathForUpdate(v_levelId));
		  v_lvl->setRequireUpdate(true);
		  m_webLevels.push_back(v_lvl);
		}
	      }
	    }
	  }
	}
      }
      pVarElem = pVarElem->NextSiblingElement("level");
    }
  }
}

std::string WebLevels::getDestinationFile(std::string p_url) {
  return getDestinationDir() + "/" + vapp::FS::getFileBaseName(p_url) + ".lvl";
}

void WebLevels::update() {
  /* delete levels information */
  std::vector<WebLevel*>::iterator it; 
  for(it = m_webLevels.begin(); it != m_webLevels.end(); it++) {
    delete *it;
  }
  m_webLevels.clear();

  downloadXml();
  extractLevelsToDownloadFromXml();
}

struct f_curl_download_data {
  vapp::WWWAppInterface *v_WebApp;
  int v_nb_files_to_download;
  int v_nb_files_performed;
};

int FSWeb::f_curl_progress_callback(void *clientp,
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

  /* we can't make confiance to the web server information */
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

void WebLevels::upgrade() {
  std::vector<WebLevel*>::iterator it;
  f_curl_download_data v_data;
  bool to_download;

  createDestinationDirIfRequired();

  int v_nb_levels_to_download = m_webLevels.size();
  int v_nb_levels_performed  = 0;

  v_data.v_WebApp = m_WebLevelApp;
  v_data.v_nb_files_to_download = v_nb_levels_to_download;

  /* download levels */
  it = m_webLevels.begin();
  while(it != m_webLevels.end() && m_WebLevelApp->isCancelAsSoonAsPossible() == false) {
    std::string v_url = (*it)->getUrl();
    float v_percentage = (((float)v_nb_levels_performed) * 100.0) / ((float)v_nb_levels_to_download);

    m_WebLevelApp->setTaskProgress(v_percentage);
    m_WebLevelApp->setBeingDownloadedInformation((*it)->getName(), (*it)->requireUpdate());

    /* should the level be updated */
    to_download = true;
    if((*it)->requireUpdate()) {
      /* does the user want to update the level ? */
      to_download = m_WebLevelApp->shouldLevelBeUpdated((*it)->getId());
    }

    if(to_download) {
      v_data.v_nb_files_performed = v_nb_levels_performed;
      std::string v_destFile;
      
      if((*it)->requireUpdate()) {
	v_destFile = m_WebLevelApp->levelPathForUpdate((*it)->getId());
      } else {
	v_destFile = getDestinationFile(v_url);
      }
      
      FSWeb::downloadFileBz2(v_destFile,
			     v_url,
			     FSWeb::f_curl_progress_callback,
			     &v_data,
			     m_proxy_settings);
      
      if((*it)->requireUpdate()) {
	m_webLevelsUpdatedDownloadedOK.push_back(v_destFile);
      } else {
	m_webLevelsNewDownloadedOK.push_back(v_destFile);
      }
    }    

    v_nb_levels_performed++;
    m_WebLevelApp->readEvents(); 
    it++;
  }

  m_WebLevelApp->setTaskProgress(100.0);
}

void WebLevels::getUpdateInfo(int *pnUBytes,int *pnULevels) {
  if(pnULevels != NULL) *pnULevels = m_webLevels.size();
  if(pnUBytes != NULL) *pnUBytes = -1;
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
    throw vapp::Exception("error : unable analyse xml theme file");
  }

  v_webTXmlDataElement = v_webTXmlData->FirstChildElement("xmoto_themes");
  
  if(v_webTXmlDataElement == NULL) {
    throw vapp::Exception("error : unable analyse xml theme file");
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
  for(int i=0;i<m_availableThemes.size();i++) {
    delete m_availableThemes[i];
  }    

  m_availableThemes.clear();
}

const std::vector<WebTheme*> &WebThemes::getAvailableThemes() {
  return m_availableThemes;
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
  for(int i=0; i<m_availableThemes.size(); i++) {
    if(m_availableThemes[i]->getName() == p_themeChoice->ThemeName()) {
      v_associated_webtheme = m_availableThemes[i];
    }
  }

  /* the destination file must be in the user dir */
  if(vapp::FS::isInUserDir(p_themeChoice->ThemeFile())) {
    v_destinationFile = p_themeChoice->ThemeFile();
  }

  // theme avaible on the web get it
  if(v_associated_webtheme != NULL) {

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
  }

  if(v_destinationFile == "") {
    throw vapp::Exception("error : unable to determine theme destination file");
  }

  /* download all the files required */
  Theme *v_theme = new Theme();
  std::vector<std::string> *v_required_files;
  v_theme->load(v_destinationFile);
  v_required_files = v_theme->getRequiredFiles();

  int v_nb_files_to_download = 0;
  for(int i=0; i<v_required_files->size(); i++) {
    if(vapp::FS::fileExists((*v_required_files)[i]) == false) {
      v_nb_files_to_download++;
    }
  }
  if(v_nb_files_to_download != 0) {

    int v_nb_files_performed  = 0;
    
    v_data.v_WebApp = m_WebApp;
    v_data.v_nb_files_to_download = v_nb_files_to_download;

    int i = 0;
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
			    FSWeb::f_curl_progress_callback,
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

#endif
