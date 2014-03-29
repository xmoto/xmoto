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

#include "VXml.h"
#include "VFileIO.h"
#include "helpers/VExcept.h"
#include "helpers/Log.h"

XMLDocument::XMLDocument() {
    m_doc = NULL;
    xmlSetGenericErrorFunc(this, XMLDocument::XMLDocumentErrorFunc);
}

XMLDocument::~XMLDocument() {
  if(m_doc != NULL) {
    xmlFreeDoc(m_doc);
  }
}

void XMLDocument::clean() {
  xmlCleanupParser();
}

void XMLDocument::XMLDocumentErrorFunc(void* ctx, const char* msg, ...) {
  XMLDocument* v_xml = (XMLDocument*) ctx;
  va_list List;
  char cBuf[2048];

  va_start(List,msg);
  vsnprintf(cBuf, 2048, msg, List);
  va_end(List);

  LogError(cBuf);
}

void XMLDocument::readFromFile(FileDataType i_fdt, std::string File, bool i_includeCurrentDir) {
  /* Clean up if a doc already is loaded */
  if(m_doc != NULL) {
    xmlFreeDoc(m_doc);
    m_doc = NULL;
  }

  // directly open the file
  if(XMFS::doesRealFileOrDirectoryExists(File)) {
    m_doc = xmlParseFile(File.c_str());
  } else {
    FileHandle *pfh;
    std::string v_xmlstr;

    pfh = XMFS::openIFile(i_fdt, File, i_includeCurrentDir);
    if(pfh==NULL) {
      throw Exception("failed to load XML " + File);
    }
    v_xmlstr = XMFS::readFileToEnd(pfh);
    XMFS::closeFile(pfh);

    m_doc = xmlParseMemory(v_xmlstr.c_str(), v_xmlstr.length());
  }

  if (m_doc == NULL) {
    throw Exception("failed to load XML " + File);
  }
}

xmlNodePtr XMLDocument::getRootNode(const char* rootNameToCheck) {
  char*     vc = (char*) rootNameToCheck;
  xmlChar* xvc = (xmlChar*) vc;

  xmlNodePtr r = xmlDocGetRootElement(m_doc);
  if(xvc != NULL) {
    if(r == NULL) {
      throw Exception("Invalid root name");
    } else if(xmlStrcmp(r->name, xvc) != 0) {
      throw Exception("Invalid root name");
    }
  }
  return r;
}

xmlNodePtr XMLDocument::subElement(xmlNodePtr node, const char* name) {
  xmlNodePtr child;
  char*    cname = (char*)     name;
  xmlChar* xname = (xmlChar*) cname;

  // search he children
  if(node->children == NULL) {
    return NULL;
  }
  child = node->children;

  // is it the direct child ?
  if(child->type == XML_ELEMENT_NODE) {
    if(xmlStrcmp(child->name, xname) == 0) {
      return child;
    }
  }

  return nextElement(child, name); // go throw the child level to find a browser
}

xmlNodePtr XMLDocument::nextElement(xmlNodePtr node, const char* name) {
  xmlNodePtr n;
  char*    cname = (char*)     name;
  xmlChar* xname = (xmlChar*) cname;

  n = node->next;
  while(n != NULL) {
    if(n->type == XML_ELEMENT_NODE) {
      if(xmlStrcmp(n->name, name == NULL ? node->name : xname) == 0) { // searched node ?
	return n;
      }
    }
    n = n->next;
  }

  // nothing found
  return NULL;
}

std::string XMLDocument::getOption(xmlNodePtr node, const char* name, std::string Default) {
  char* v = (char *) name;
  return getOption(node, (xmlChar*) v, Default);
}

std::string XMLDocument::getOption(xmlNodePtr node, xmlChar* name, std::string Default) {
  xmlChar* value;
  std::string res;

  value = xmlGetProp(node, name);
  if(value == NULL) {
    return Default;
  }
  res = std::string((char*) value);
  xmlFree(value);

  return res; 
}

std::string XMLDocument::getElementText(xmlNodePtr node) {
  xmlChar* value;
  std::string res;

  // get the value
  value = xmlNodeGetContent(node);
  if(value == NULL) {
    return "";
  }
  res = std::string((char*) value);
  xmlFree(value);

  return res;
}

std::string XMLDocument::str2xmlstr(std::string str) {
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
