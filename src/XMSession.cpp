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

#include "XMSession.h"
#include "XMArgs.h"
#include "UserConfig.h"

XMSession::XMSession() {
  /* default config */
  m_verbose          = false;
  m_useGraphics      = true;
  m_resolutionWidth  = 800;
  m_resolutionHeight = 600;
  m_bpp              = 32;
  m_windowed         = false;
  m_glExts           = true;
  m_drawlib          = "OPENGL";
  m_www              = true;
}

void XMSession::load(const XMArguments* i_xmargs) {
  if(i_xmargs->isOptVerbose()) {
    m_verbose = true;
  }
  if(i_xmargs->isOptNoGfx()) {
    m_useGraphics = false;
  }
  if(i_xmargs->isOptRes()) {
    m_resolutionWidth  = i_xmargs->getOpt_res_dispWidth();
    m_resolutionHeight = i_xmargs->getOpt_res_dispHeight();
  }

  if(i_xmargs->isOptBpp()) {
    m_bpp = i_xmargs->getOpt_bpp_value();
  }

  if(i_xmargs->isOptWindowed()) {
    m_windowed = true;
  }

  if(i_xmargs->isOptFs()) {
    m_windowed = false;
  }

  if(i_xmargs->isOptNoExts()) {
    m_glExts = false;
  }

  if(i_xmargs->isOptDrawlib()) {
    m_drawlib = i_xmargs->getOpt_drawlib_lib();
  }

  if(i_xmargs->isOptNoWWW()) {
    m_www = false;
  }

}

void XMSession::load(UserConfig* m_Config) {
  m_resolutionWidth  = m_Config->getInteger("DisplayWidth");
  m_resolutionHeight = m_Config->getInteger("DisplayHeight");
  m_bpp              = m_Config->getInteger("DisplayBPP");
  m_windowed         = m_Config->getBool("DisplayWindowed");
  m_drawlib          = m_Config->getString("DrawLib");
  m_www              = m_Config->getBool("WebHighscores");
}

bool XMSession::isVerbose() const {
  return m_verbose;
}

bool XMSession::useGraphics() const {
  return m_useGraphics;
}

void XMSession::setUseGraphics(bool i_value) {
  m_useGraphics = i_value;
}

int XMSession::resolutionWidth() const {
  return m_resolutionWidth;
}

int XMSession::resolutionHeight() const {
  return m_resolutionHeight;
}

int XMSession::bpp() const {
  return m_bpp;
}

int XMSession::windowed() const {
  return m_windowed;
}

bool XMSession::glExts() const {
  return m_glExts;
}

std::string XMSession::drawlib() const {
  return m_drawlib;
}

bool XMSession::www() const {
  return m_www;
}

void XMSession::setWWW(bool i_value) {
  m_www = i_value;
}
