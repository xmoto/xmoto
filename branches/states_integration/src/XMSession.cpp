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
  m_verbose          	     = false;
  m_useGraphics      	     = true;
  m_resolutionWidth  	     = 800;
  m_resolutionHeight 	     = 600;
  m_bpp              	     = 32;
  m_windowed         	     = false;
  m_glExts           	     = true;
  m_drawlib          	     = "OPENGL";
  m_www              	     = true;
  m_benchmark 	     	     = false;
  m_debug     	     	     = false;
  m_sqlTrace  	     	     = false;
  m_profile   	     	     = "";
  m_gdebug    	     	     = false;
  m_timedemo  	     	     = false;
  m_fps       	     	     = false;
  m_ugly      	     	     = false;
  m_uglyOver         	     = false;
  m_testTheme 	     	     = false;
  m_ghostStrategy_MYBEST     = true;
  m_ghostStrategy_THEBEST    = false;
  m_ghostStrategy_BESTOFROOM = false;
  m_autosaveHighscoreReplays = true;
  m_enableGhosts             = true;
  m_enableEngineSound        = true;
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

  if(i_xmargs->isOptBenchmark()) {
    m_benchmark = true;
  }

  if(i_xmargs->isOptDebug()) {
    m_debug = true;
  }

  if(i_xmargs->isOptSqlTrace()) {
    m_sqlTrace = true;
  }

  if(i_xmargs->isOptProfile()) {
    m_profile = i_xmargs->getOpt_profile_value();
  }

  if(i_xmargs->isOptGDebug()) {
    m_gdebug = true;
    m_gdebug_file = i_xmargs->getOpt_gdebug_file();
  }

  if(i_xmargs->isOptTimedemo()) {
    m_timedemo = true;
  }

  if(i_xmargs->isOptFps()) {
    m_fps = true;
  }

  if(i_xmargs->isOptUgly()) {
    m_ugly = true;
  }

  if(i_xmargs->isOptTestTheme()) {
    m_testTheme = true;
  }

}

void XMSession::load(UserConfig* m_Config) {
  m_resolutionWidth  	     = m_Config->getInteger("DisplayWidth");
  m_resolutionHeight 	     = m_Config->getInteger("DisplayHeight");
  m_bpp              	     = m_Config->getInteger("DisplayBPP");
  m_windowed         	     = m_Config->getBool("DisplayWindowed");
  m_drawlib          	     = m_Config->getString("DrawLib");
  m_www              	     = m_Config->getBool("WebHighscores");
  m_profile          	     = m_Config->getString("DefaultProfile");
  m_ghostStrategy_MYBEST     = m_Config->getBool("GhostStrategy_MYBEST");
  m_ghostStrategy_THEBEST    = m_Config->getBool("GhostStrategy_THEBEST");
  m_ghostStrategy_BESTOFROOM = m_Config->getBool("GhostStrategy_BESTOFROOM");
  m_autosaveHighscoreReplays = m_Config->getBool("AutosaveHighscoreReplays");
  m_enableGhosts             = m_Config->getBool("EnableGhost");
  m_enableEngineSound        = m_Config->getBool("EngineSoundEnable");
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

bool XMSession::windowed() const {
  return m_windowed;
}

void XMSession::setResolutionWidth(int i_value) {
  m_resolutionWidth = i_value;
}

void XMSession::setResolutionHeight(int i_value) {
  m_resolutionHeight = i_value;
}

void XMSession::setBpp(int i_value) {
  m_bpp = i_value;
}

void XMSession::setWindowed(bool i_value) {
  m_windowed = i_value;
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

bool XMSession::benchmark() const {
  return m_benchmark;
}

bool XMSession::debug() const {
  return m_debug;
}

bool XMSession::sqlTrace() const {
  return m_sqlTrace;
}

std::string XMSession::profile() const {
  return m_profile;
}

void XMSession::setProfile(const std::string& i_profile) {
  m_profile = i_profile;
}

bool XMSession::gDebug() const {
  return m_gdebug;
}

std::string XMSession::gDebugFile() const {
  return m_gdebug_file;
}

bool XMSession::timedemo() const {
  return m_timedemo;
}

bool XMSession::fps() const {
  return m_fps;
}

void XMSession::setFps(bool i_value) {
  m_fps = i_value;
}

bool XMSession::ugly() const {
  return m_ugly;
}

bool XMSession::uglyOver() const {
  return m_uglyOver;
}

bool XMSession::testTheme() const {
  return m_testTheme;
}

void XMSession::setUgly(bool i_value) {
  m_ugly = i_value;
}

void XMSession::setUglyOver(bool i_value) {
  m_uglyOver = i_value;
}

void XMSession::setTestTheme(bool i_value) {
  m_testTheme = i_value;
}

bool XMSession::ghostStrategy_MYBEST() const {
  return m_ghostStrategy_MYBEST;
}

void XMSession::setGhostStrategy_MYBEST(bool i_value) {
  m_ghostStrategy_MYBEST = i_value;
}

bool XMSession::ghostStrategy_THEBEST() const {
  return m_ghostStrategy_THEBEST;
}

void XMSession::setGhostStrategy_THEBEST(bool i_value) {
  m_ghostStrategy_THEBEST = i_value;
}

bool XMSession::ghostStrategy_BESTOFROOM() const {
  return m_ghostStrategy_BESTOFROOM;
}

void XMSession::setGhostStrategy_BESTOFROOM(bool i_value) {
  m_ghostStrategy_BESTOFROOM = i_value;
}

bool XMSession::autosaveHighscoreReplays() const {
  return m_autosaveHighscoreReplays;
}

void XMSession::setAutosaveHighscoreReplays(bool i_value) {
  m_autosaveHighscoreReplays = i_value;
}

void XMSession::setEnableGhosts(bool i_value) {
  m_enableGhosts = i_value;
}
 
bool XMSession::enableGhosts() const {
  return m_enableGhosts;
}

void XMSession::setEnableEngineSound(bool i_value) {
  m_enableEngineSound = i_value;
}

bool XMSession::enableEngineSound() const {
  return m_enableEngineSound;
}
