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

/* 
 *  XML support through the tinyxml library.
 */
#include "VXml.h"
#include "VFileIO.h"
#include "CRCHash.h"
#include "helpers/Log.h"

  /*===========================================================================
  Load XML document from disk
  ===========================================================================*/
void XMLDocument::readFromFile(FileDataType i_fdt, std::string File,unsigned long *pnCRC32, bool i_includeCurrentDir) {
    /* Clean up if a doc already is loaded */
    if(m_pXML) delete m_pXML;
    
    /* Load */
    std::string Line,Doc = "";    
    
    FileHandle *pfh = XMFS::openIFile(i_fdt, File, i_includeCurrentDir);
    if(pfh==NULL) return;

    m_pXML = new TiXmlDocument;
    
    while(XMFS::readNextLine(pfh,Line)) {
      if(Line.length() > 0) {
        Doc.append(Line);
        Doc.append("\n");
      }
    }    
    XMFS::closeFile(pfh);
    
    /* Are we just going to need the CRC? */
    if(pnCRC32 != NULL) {
      *pnCRC32 = CRC32::computeCRC32((const unsigned char *)Doc.c_str(),Doc.length());
    }
    else {
      /* Parse XML */
      m_pXML->Parse(Doc.c_str());
      if(m_pXML->Error()) {
        LogWarning("XML-parsing error in '%s' : %s",File.c_str(),m_pXML->ErrorDesc());
      }
    }
  }
      
  /*===========================================================================
  Save XML document from disk
  ===========================================================================*/
  void XMLDocument::writeToFile(FileDataType i_fdt, std::string File) {
    /* Anything? */
    if(!m_pXML) return;
    FileHandle *pfh = XMFS::openOFile(i_fdt, File);
    if(pfh==NULL || pfh->Type!=FHT_STDIO) return;
    m_pXML->Print(pfh->fp);
    XMFS::closeFile(pfh);
  }

  std::string XML::str2xmlstr(std::string str) {
    std::string v_res = "";
    for(unsigned int i=0; i<str.length(); i++) {
      switch(str[i]) {
      case '&':
  v_res.append("&amp;");
  break;
      case '<':
  v_res.append("&lt;");
  break;
      case '>':
  v_res.append("&gt;");
  break;
      case '\"':
  v_res.append("&quot;");
  break;
      case '\'':
  v_res.append("&apos;");
  break;
      default:
  char c[2] = {str[i], '\0'};
  v_res.append(c);
      }
    }
    return v_res;
  }
    

  /*===========================================================================
    Find element option/attribute
    ===========================================================================*/
  std::string XML::getOption(TiXmlElement *pElem,std::string Name,std::string Default) {
    const char *pc = pElem->Attribute(Name.c_str());
    if(pc == NULL) return Default;
    return std::string(pc);
  }

  /*===========================================================================
    Find element in document object model
    ===========================================================================*/
  TiXmlElement* XML::findElement(XMLDocument& i_source, TiXmlElement *pRoot,const std::string &Name) {
    TiXmlElement *pFirst;
    
    /* Find out where to start */
    if(pRoot == NULL) pFirst = i_source.getLowLevelAccess()->FirstChildElement();
    else pFirst = pRoot->FirstChildElement();
    
    /* Scan through elements */
    for(TiXmlElement *pElem = pFirst;pElem!=NULL;pElem=pElem->NextSiblingElement()) {
      if(!strcmp(pElem->Value(),Name.c_str())) {
        /* This is the one */
        return pElem;
      }
      
      /* Recurse */
      if(pElem->FirstChildElement() != NULL) {
        TiXmlElement *pRet = findElement(i_source, pElem,Name);
        if(pRet != NULL) return pRet;
      }
    }
    
    /* Found nothing */
    return NULL;
  }


/*===========================================================================
  Get formatted element text
  ===========================================================================*/
  std::string XML::getElementText(XMLDocument& i_source, TiXmlElement *pRoot,std::string Name) {
    TiXmlElement *pFirst;
    std::string Text = "";
    
    /* Find out where to start */
    if(pRoot == NULL) pFirst = i_source.getLowLevelAccess()->FirstChildElement();
    else pFirst = pRoot->FirstChildElement();
    
    /* Scan through elements */
    for(TiXmlElement *pElem = pFirst;pElem!=NULL;pElem=pElem->NextSiblingElement()) {
      if(!strcmp(pElem->Value(),Name.c_str())) {
        /* This is the one -- format text*/
        for(TiXmlNode *pNode = pElem->FirstChild();pNode!=NULL;pNode=pNode->NextSibling()) {
          if(pNode->Type() == TiXmlNode::TEXT) {
            /* Ohh... text. Append it */
            appendText(Text,std::string(pNode->Value()));
          }
          else if(pNode->Type() == TiXmlNode::ELEMENT) {
            /* Hmm, some kind of formatting element */
            if(!strcmp(pNode->Value(),"br")) {
              /* HTML-style line break */
              Text.append( "\n" );              
            }
          }
        }
        if(Text.length() > 0) {
          if(Text[ Text.length()-1 ] == ' ') {
            Text = Text.substr(0,Text.length()-1);
          }
        }
        return Text;
      }
    }
    
    /* Nothing */
    return "";
  }

  /*===========================================================================
    Append. yeah.
    ===========================================================================*/
  void XML::appendText(std::string &Text,const std::string &Append) {
    const char *pc = Append.c_str();
    int i=0;
    std::string A = "";
    char c[2];
    
    while(1) {
      if(pc[i] == '\0' || pc[i] == ' ' || pc[i] == '\t') {
        if(A != "") {
          Text.append( A );
          Text.append( " " );
          A = "";
        }
        if(pc[i] == '\0') break;
    }
      else {
        c[0] = pc[i];
        c[1] = '\0';
        A.append( c );
      }
      
      i++;
    }
  }  

