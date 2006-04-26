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
    - added cleanHash() to WebHighscores destructor
 */

#include "WWW.h"
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

WebHighscore::WebHighscore(std::string p_levelId,
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
}

WebHighscore::~WebHighscore() {
}

void WebHighscore::download() {
  FSWeb::downloadFile(m_rplFilename, m_rplUrl, NULL, NULL, m_proxy_settings);
}

std::string WebHighscore::getPlayerName() const {
  return m_playerName;
}

std::string WebHighscore::getLevelId() const {
  return m_levelId;
}

std::string WebHighscore::getTime() const {
  return m_time;
}

WebHighscores::WebHighscores(const ProxySettings *p_proxy_settings) {
  std::string v_userDir;
  v_userDir = vapp::FS::getUserDir();
  m_userFilename =  v_userDir 
    + "/" 
    + DEFAULT_WEBHIGHSCORES_FILENAME;
  m_webhighscores_url = DEFAULT_WEBHIGHSCORES_URL;

  m_proxy_settings = p_proxy_settings;
}

WebHighscores::~WebHighscores() {
  cleanHash();
}

void WebHighscores::setWebsiteURL(std::string p_webhighscores_url) {
  m_webhighscores_url = p_webhighscores_url;
}

WebHighscore* WebHighscores::getHighscoreFromLevel(const std::string &p_levelId) {
  #if defined(USE_HASH_MAP)
    return m_webhighscores[p_levelId.c_str()];
  #else
    for(int i=0;i<m_webhighscores.size();i++)
      if(m_webhighscores[i]->getLevelId() == p_levelId) return m_webhighscores[i];
    return NULL;  
  #endif
}

void WebHighscores::cleanHash() {
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

void WebHighscores::fillHash() {
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
  v_webHSXmlDataElement = v_webHSXmlData->FirstChildElement("xmoto_worldrecords");

  if(v_webHSXmlDataElement != NULL) {
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
      
      wh = new WebHighscore(v_levelId, v_playerName, v_time, v_rplUrl, m_proxy_settings);
      
      #if defined(USE_HASH_MAP)
        m_webhighscores[wh->getLevelId().c_str()] = wh;
      #else
        m_webhighscores.push_back(wh);
      #endif      
    }
  }
}

void WebHighscores::update() {
  /* download xml file */
  FSWeb::downloadFile(m_userFilename, m_webhighscores_url, NULL, NULL, m_proxy_settings);
}

void WebHighscores::upgrade() {
  fillHash();
}

size_t FSWeb::writeData(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  return fwrite (ptr, size, nmemb, stream);
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

  /* open the file */
  if( (v_destinationFile = fopen(v_local_file_tmp.c_str(), "w")) == false) {
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

  /* set proxy settings */
  if(p_proxy_settings != NULL) {
    if(p_proxy_settings->useDefaultServer() == false) {
      curl_easy_setopt(v_curl, CURLOPT_PROXY, p_proxy_settings->getServer().c_str());
    }
    
    if(p_proxy_settings->useDefaultPort() == false) {
      curl_easy_setopt(v_curl, CURLOPT_PROXYPORT, p_proxy_settings->getPort());
    }
    
    curl_easy_setopt(v_curl, CURLOPT_PROXYTYPE, p_proxy_settings->getType());
   
    if(p_proxy_settings->useDefaultAuthentification() == false) {
      std::string v_proxy_auth_str =
	p_proxy_settings->getAuthentificationUser()
	+ " "
	+ p_proxy_settings->getAuthentificationPassword();
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

WebLevels::WebLevels(vapp::WWWAppInterface *p_WebLevelApp,
		     const ProxySettings *p_proxy_settings) {
  m_WebLevelApp    = p_WebLevelApp;
  m_proxy_settings = p_proxy_settings;
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
  FSWeb::downloadFile(getXmlFileName(),
		      DEFAULT_WEBLEVELS_URL,
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
  std::string v_levelId, v_levelName, v_url;
  
  v_webLXml.readFromFile(getXmlFileName());
  v_webLXmlData = v_webLXml.getLowLevelAccess();
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
	  
	  /* if it doesn't exist */
	  if(m_WebLevelApp->doesLevelExist(v_levelId) == false) {
	    m_webLevels.push_back(new WebLevel(v_levelId, v_levelName, v_url));
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
  downloadXml();
  extractLevelsToDownloadFromXml();
}

struct f_curl_download_data {
  vapp::WWWAppInterface *v_WebLevelApp;
  int v_nb_levels_to_download;
  int v_nb_levels_downloaded;
};

int WebLevels::f_curl_progress_callback(void *clientp,
					double dltotal,
					double dlnow,
					double ultotal,
					double ulnow) {
  f_curl_download_data *data = ((f_curl_download_data*) clientp);
  float real_percentage_already_done;
  float real_percentage_of_current_file;

  /* cancel if it's wanted */
  if(data->v_WebLevelApp->isCancelAsSoonAsPossible()) {
    return 1;
  }

  real_percentage_already_done = 
    (((float)data->v_nb_levels_downloaded) * 100.0) 
    / ((float)data->v_nb_levels_to_download);

  /* we can't make confiance to the web server information */
  real_percentage_of_current_file = 0.0;
  if(dltotal > 0.0) {
    if((dlnow / dltotal) >= 0.0 && (dlnow / dltotal) <= 1.0) {
      real_percentage_of_current_file = 
	(dlnow * 100.0) 
	/ (((float)data->v_nb_levels_to_download) * dltotal);
    }
  }

  data->v_WebLevelApp->setTaskProgress(real_percentage_already_done + real_percentage_of_current_file);

  return 0;
}

void WebLevels::upgrade() {
  std::vector<WebLevel*>::iterator it;
  f_curl_download_data v_data;

  createDestinationDirIfRequired();

  int v_nb_levels_to_download = m_webLevels.size();
  int v_nb_levels_downloaded  = 0;

  v_data.v_WebLevelApp = m_WebLevelApp;
  v_data.v_nb_levels_to_download = v_nb_levels_to_download;

  /* download levels */
  it = m_webLevels.begin();
  while(it != m_webLevels.end() && m_WebLevelApp->isCancelAsSoonAsPossible() == false) {
    std::string v_url = (*it)->getUrl();
    float v_percentage = (((float)v_nb_levels_downloaded) * 100.0) / ((float)v_nb_levels_to_download);

    m_WebLevelApp->setTaskProgress(v_percentage);
    m_WebLevelApp->setBeingDownloadedLevel((*it)->getName());

    v_data.v_nb_levels_downloaded = v_nb_levels_downloaded;
    std::string v_destFile = getDestinationFile(v_url);
    
    FSWeb::downloadFile(v_destFile,
			v_url,
			f_curl_progress_callback,
			&v_data,
			m_proxy_settings);
			
    m_webLevelsDownloadedOK.push_back(v_destFile);
    v_nb_levels_downloaded++;
    m_WebLevelApp->readEvents(); 
    it++;
  }

  m_WebLevelApp->setTaskProgress(100.0);
}

void WebLevels::getUpdateInfo(int *pnUBytes,int *pnULevels) {
  if(pnULevels != NULL) *pnULevels = m_webLevels.size();
  if(pnUBytes != NULL) *pnUBytes = -1;
}

const std::vector<std::string> &WebLevels::getDownloadedLevels(void) {
  return m_webLevelsDownloadedOK;
}

#endif

