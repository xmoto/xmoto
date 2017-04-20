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

#include "XMDemo.h"
#include "common/VXml.h"
#include "common/VFileIO.h"
#include "common/WWW.h"
#include "helpers/VExcept.h"

XMDemo::XMDemo(const std::string i_demoFile) {
    XMLDocument v_xml;
    xmlNodePtr  v_xmlElt;

    v_xml.readFromFile(FDT_DATA, i_demoFile, true);

    v_xmlElt = v_xml.getRootNode("demo");
    if(v_xmlElt == NULL) {
      throw Exception("failed to load demo XML " + i_demoFile);
    }

    /* Get URLs */
    m_levelUrl = XMLDocument::getOption(v_xmlElt, "level_url");
    if(m_levelUrl == "") throw Exception("no level URL specified in XML");

    m_replayUrl = XMLDocument::getOption(v_xmlElt, "replay_url");
    if(m_replayUrl == "") throw Exception("no replay URL specified in XML");

    m_levelFile  = XMFS::getUserDir(FDT_CACHE) + "/demo/" + XMFS::getFileBaseName(m_levelUrl) + ".lvl";
    m_replayFile = XMFS::getUserDir(FDT_CACHE) + "/demo/" + XMFS::getFileBaseName(m_replayUrl) + ".rpl";

    XMFS::mkArborescence(m_levelFile);
    XMFS::mkArborescence(m_replayFile);
}

XMDemo::~XMDemo() {
}

std::string XMDemo::levelFile() const {
  return m_levelFile;
}

std::string XMDemo::replayFile() const {
  return m_replayFile;
}

void XMDemo::getLevel(ProxySettings* i_proxy) {
  FSWeb::downloadFile(m_levelFile, m_levelUrl, NULL, NULL, i_proxy);
}

void XMDemo::getReplay(ProxySettings* i_proxy) {
  FSWeb::downloadFile(m_replayFile, m_replayUrl, NULL, NULL, i_proxy);
}

void XMDemo::destroyFiles() {
  XMFS::deleteFile(m_levelFile);
  XMFS::deleteFile(m_replayFile);
}
