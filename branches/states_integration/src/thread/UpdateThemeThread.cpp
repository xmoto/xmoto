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

#include "UpdateThemeThread.h"
#include "helpers/Log.h"
#include "GameText.h"
#include "Game.h"
#include "WWW.h"
#include "XMSession.h"
#include "states/StateManager.h"

UpdateThemeThread::UpdateThemeThread(const std::string& i_id_theme)
  : XMThread()
{
  m_id_theme = i_id_theme;
}

UpdateThemeThread::~UpdateThemeThread()
{
}

int UpdateThemeThread::realThreadFunction()
{
  setThreadCurrentOperation(GAMETEXT_DLTHEME);
  setThreadProgress(0);

  try {
    Logger::Log("WWW: Downloading a theme...");

    std::string v_destinationFile;
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

    v_data.v_WebApp = this;

    /* download even if the the theme is uptodate
       it give a possibility to download file removed by mistake
    */
    v_result = m_pDb->readDB("SELECT a.fileUrl, b.filepath "
			     "FROM webthemes AS a LEFT OUTER JOIN themes AS b "
			     "ON a.id_theme = b.id_theme "
			     "WHERE a.id_theme=\"" + xmDatabase::protectString(m_id_theme) + "\";",
			     nrow);
    if(nrow != 1) {
      m_pDb->read_DB_free(v_result);
      return 1;
    }

    v_fileUrl = m_pDb->getResult(v_result, 2, 0, 0);
    if(m_pDb->getResult(v_result, 2, 0, 1) != NULL) {
      v_filePath = m_pDb->getResult(v_result, 2, 0, 1);
      v_onDisk = true;
    }  
    m_pDb->read_DB_free(v_result);

    // theme avaible on the web get it

    /* the destination file must be in the user dir */
    if(v_filePath != "") {
      if(FS::isInUserDir(v_filePath)) {
	v_destinationFile = v_filePath;
      }
    }

    if(v_destinationFile == "") {
      /* determine destination file */
      v_destinationFile = 
	FS::getUserDir() + "/" + THEMES_DIRECTORY + "/" + 
	FS::getFileBaseName(v_fileUrl) + ".xml";
    }
  
    v_themeFile = v_destinationFile;

    /* download the theme file */
    FS::mkArborescence(v_destinationFile);
    FSWeb::downloadFileBz2(v_destinationFile, v_fileUrl, NULL, NULL, m_pGame->getProxySettings());
    
    /* download all the files required */
    Theme *v_theme = new Theme();
    std::vector<ThemeFile> *v_required_files;
    v_theme->load(v_destinationFile);
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
      
      v_data.v_nb_files_to_download = v_nb_files_to_download;
      
      unsigned int i = 0;
      while(i<v_required_files->size()) {
	// download v_required_files[i]     
	v_destinationFile = FS::getUserDir() + std::string("/") + (*v_required_files)[i].filepath;
	v_sourceFile = m_pGame->getSession()->webThemesURLBase() + std::string("/") + (*v_required_files)[i].filepath;
	
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
	  setThreadProgress(v_percentage);
	  setThreadCurrentMicroOperation((*v_required_files)[i].filepath);
	  
	  FS::mkArborescence(v_destinationFile);
	  
	  FSWeb::downloadFile(v_destinationFile, v_sourceFile, FSWeb::f_curl_progress_callback_download, &v_data,
			      m_pGame->getProxySettings());
	  
	  v_nb_files_performed++;
	}
	i++;
      }

      setThreadProgress(100);
    }   
    
    delete v_theme;
    
    if(v_onDisk) {
      m_pDb->themes_update(m_id_theme, v_themeFile);
    } else {
      m_pDb->themes_add(m_id_theme, v_themeFile);
    }
 
    m_pGame->getStateManager()->sendSynchronousMessage("UPDATE_THEMES_LISTS");

 } catch(Exception &e) {
    Logger::Log("** Warning ** : Failed to update theme %s", m_id_theme.c_str());
    return 1;
  }

  return 0;
}

void UpdateThemeThread::setTaskProgress(float p_percent) {
  setThreadProgress(p_percent);
}
