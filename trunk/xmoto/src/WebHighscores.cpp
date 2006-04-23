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

#ifdef WIN32
  #include <io.h>
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <dirent.h>
#endif
#include <sys/stat.h>

#include <curl/curl.h>

#include "WebHighscores.h"
#include "VFileIO.h"
#include "VExcept.h"
#include "VXml.h"

WebHighscore::WebHighscore(std::string p_levelId,
			   std::string p_playerName,
			   std::string p_time,
			   std::string p_rplUrl) {
  m_playerName  = p_playerName;
  m_time        = p_time;
  m_levelId     = p_levelId;
  m_rplUrl      = p_rplUrl;
  
  /* TODO: vapp::FS::getReplayDir() won't work here, FS not init at this point */
  //m_rplFilename = vapp::FS::getReplayDir()
  //  + "/" 
  //  + vapp::FS::getFileBaseName(m_rplUrl) 
  //  + ".rpl";
}


WebHighscore::~WebHighscore() {
}

void WebHighscore::download() {
  FSWeb::downloadFile(m_rplFilename, m_rplUrl);
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

WebHighscores::WebHighscores() {
  std::string v_userDir;
  v_userDir = vapp::FS::getUserDir();
  m_userFilename =  v_userDir 
    + "/" 
    + DEFAULT_WEBHIGHSCORES_FILENAME;
  m_webhighscores_url = DEFAULT_WEBHIGHSCORES_URL;
}

WebHighscores::~WebHighscores() {
  /* clean hash table */
  cleanHash(); /*(rasmus) 2006-04-23*/
}

void WebHighscores::setWebsiteURL(std::string p_webhighscores_url) {
  m_webhighscores_url = p_webhighscores_url;
}

WebHighscore* WebHighscores::getHighscoreFromLevel(const std::string &p_levelId) {
  #if defined(_MSC_VER)
    for(int i=0;i<m_webhighscores.size();i++)
      if(m_webhighscores[i]->getLevelId() == p_levelId) return m_webhighscores[i];
    return NULL;
  #else
    return m_webhighscores[p_levelId.c_str()];
  #endif  
}

void WebHighscores::cleanHash() {
  #if defined(_MSC_VER)
    for(int i=0;i<m_webhighscores.size();i++)
      delete m_webhighscores[i];
  #else
    HashNamespace::hash_map<const char*, WebHighscore*, HashNamespace::hash<const char*>, highscore_str, std::allocator<WebHighscore*> >::iterator it;

    it = m_webhighscores.begin();
    while(it != m_webhighscores.end()) {
      /* can be NULL, because search on unexisting value create an entry */
      if(it->second != NULL) { 
        delete it->second; /* delete highscore */
      }
      it++;
    } 
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
      
      wh = new WebHighscore(v_levelId, v_playerName, v_time, v_rplUrl);
      
      #if defined(_MSC_VER)
        m_webhighscores.push_back(wh);
      #else
        m_webhighscores[wh->getLevelId().c_str()] = wh;
      #endif
    }
  }
}

void WebHighscores::update() {
  /* download xml file */
  FSWeb::downloadFile(m_userFilename, m_webhighscores_url);
}

void WebHighscores::upgrade() {
  fillHash();
}

size_t FSWeb::writeData(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  return fwrite (ptr, size, nmemb, stream);
}

void FSWeb::downloadFile(const std::string &p_local_file, const std::string &p_web_file) {
  CURL *v_curl;
  CURLcode res;
  FILE *v_destinationFile;

  /* open the file */
  if( (v_destinationFile = fopen(p_local_file.c_str(), "w")) == false) {
    throw vapp::Exception("error : unable to open output file " 
			  + p_local_file);
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
  curl_easy_setopt(v_curl, CURLOPT_TIMEOUT, DEFAULT_WEBHIGHSCORES_TIMEOUT);

  res = curl_easy_perform(v_curl);

  if(res != 0) {
    curl_easy_cleanup(v_curl); /* (rasmus) 2006-04-23: not sure if this is necesary, but 
                                                       it's better to be safe than sorry :P */
    fclose(v_destinationFile);
    throw vapp::Exception("error : unable to download file "
			  + p_web_file);	
  }

  curl_easy_cleanup(v_curl);
  fclose(v_destinationFile);
}

void WebLevels::upgrade() {
  std::string v_levels_filename = vapp::FS::getUserDir()
    + "/" 
    + DEFAULT_WEBLEVELS_FILENAME;

  /* download xml file */
  FSWeb::downloadFile(v_levels_filename, DEFAULT_WEBLEVELS_URL);

  /* create destination directory */
  std::string v_destination_dir;
  v_destination_dir = vapp::FS::getLevelsDir() + "/" + DEFAULT_WEBLEVELS_DIR;
  if(vapp::FS::isDir(v_destination_dir) == false) {
    if(vapp::FS::mkDir(v_destination_dir.c_str()) != 0) { /* drwx------ */
      throw vapp::Exception("error : unable to make directory "
			    + v_destination_dir);	
    }
  }

  /* for each levels */
  vapp::XMLDocument v_webLXml;
  TiXmlDocument *v_webLXmlData;
  TiXmlElement *v_webLXmlDataElement;
  const char *pc;
  std::string v_levelId, v_url;

  v_webLXml.readFromFile(v_levels_filename);
  v_webLXmlData = v_webLXml.getLowLevelAccess();
  v_webLXmlDataElement = v_webLXmlData->FirstChildElement("xmoto_levels");

  if(v_webLXmlDataElement != NULL) {
    for(TiXmlElement *pVarElem = v_webLXmlDataElement->FirstChildElement("level");
	pVarElem!=NULL;
	pVarElem = pVarElem->NextSiblingElement("level")
	) {

      pc = pVarElem->Attribute("level_id");
      if(pc == NULL) { continue; }
      v_levelId = pc;

      pc = pVarElem->Attribute("url");
      if(pc == NULL) { continue; }
      v_url = pc;	 

      /* if it doesn't exist */
      // if levelExists(v_levelId) {
      /* download it ! */
      std::string v_levelFilename;
 //     v_levelFilename = v_destination_dir 
	//+ "/" 
	//+ vapp::FS::getFileBaseName(v_url) 
	//+ ".lvl";
 //     FSWeb::downloadFile(v_levelFilename, v_url); 
      //}
    }
  }
}

