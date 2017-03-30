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

#include "SkyApparence.h"

  SkyApparence::SkyApparence() {
    reInit();
  }

SkyApparence::~SkyApparence() {
}

void SkyApparence::reInit() {
    m_texture     = "sky1";
    m_blendTexture= m_texture;
    m_zoom        = 1.0;
    m_offset      = 0.015;
    m_driftZoom   = 1.0;
    m_drifted     = false;
    m_color.setRed(255);
    m_color.setGreen(255);
    m_color.setBlue(255);
    m_driftColor.setAlpha(255);
    m_driftColor.setRed(255);
    m_driftColor.setGreen(255);
    m_driftColor.setBlue(255);
    m_driftColor.setAlpha(255);
}

void SkyApparence::setOldXmotoValuesFromTextureName() {

  if(m_texture == "sky1") {
    m_zoom        = 2.0;
    m_offset      = 0.015;
    m_driftZoom   = 1.0;
    m_drifted     = false;
    m_color.setRed(255);
    m_color.setGreen(255);
    m_color.setBlue(255);
    m_color.setAlpha(255);
  }

  if(m_texture == "sky2") {
    m_zoom        = 1.53;
    m_offset      = 0.015;
    m_driftZoom   = 1.0;
    m_drifted     = false;
    m_color.setRed(255);
    m_color.setGreen(255);
    m_color.setBlue(255);
    m_color.setAlpha(255);
    return;
  }

  if(m_texture == "Sky2Drift") {
    m_zoom        = 1.53;
    m_offset      = 0.015;
    m_driftZoom   = 1.17;
    m_drifted     = true;
    m_color.setRed(128);
    m_color.setGreen(128);
    m_color.setBlue(128);
    m_color.setAlpha(128);
    m_driftColor.setRed(255);
    m_driftColor.setGreen(128);
    m_driftColor.setBlue(128);
    m_driftColor.setAlpha(128);
    return;
  }

  return;
}

std::string SkyApparence::Texture() const {
  return m_texture;
}

std::string SkyApparence::BlendTexture() const {
  return m_blendTexture;
}

float SkyApparence::Zoom() const {
  return m_zoom;
}

float SkyApparence::Offset() const {
  return m_offset;
}

const TColor& SkyApparence::TextureColor() const {
  return m_color;
}

bool SkyApparence::Drifted() const {
  return m_drifted;
}

float SkyApparence::DriftZoom() const {
  return m_driftZoom;
}

const TColor& SkyApparence::DriftTextureColor() const {
  return m_driftColor;
}


void SkyApparence::setTexture(std::string i_texture) {
  m_texture = i_texture;
}

void SkyApparence::setBlendTexture(std::string i_texture) {
  m_blendTexture = i_texture;
}

void SkyApparence::setZoom(float i_zoom) {
  m_zoom = i_zoom;
}

void SkyApparence::setOffset(float i_offset) {
  m_offset = i_offset;
}

void SkyApparence::setTextureColor(const TColor& i_color) {
  m_color = i_color;
}

void SkyApparence::setDrifted(bool i_drifted) {
  m_drifted = i_drifted;
}

void SkyApparence::setDriftZoom(float i_driftZoom) {
  m_driftZoom = i_driftZoom;
}

void SkyApparence::setDriftTextureColor(const TColor& i_driftedColor) {
  m_driftColor = i_driftedColor;
}
