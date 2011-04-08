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

#ifndef __SKYAPPARENCE_H__
#define __SKYAPPARENCE_H__

#include <string>
#include "../helpers/Color.h"
  
  class SkyApparence {
    public:
	SkyApparence();
    ~SkyApparence();
    
    std::string Texture() const;
    std::string BlendTexture() const;
    float Zoom() const;
    float Offset() const;
    const TColor& TextureColor() const;
    bool Drifted() const;
    float DriftZoom() const;
    const TColor& DriftTextureColor() const;
    
    void setTexture(std::string i_texture);
    void setBlendTexture(std::string i_texture);
    void setZoom(float i_zoom);
    void setOffset(float i_offset);
    void setTextureColor(const TColor& i_color);
    void setDrifted(bool i_drifted);
    void setDriftZoom(float i_driftZoom);
    void setDriftTextureColor(const TColor& i_driftedColor);
    
    void setOldXmotoValuesFromTextureName();
    void reInit();
    
    private:
    std::string                 m_texture;
    std::string                 m_blendTexture;
    float  			m_zoom;
    float  			m_offset;
    TColor 			m_color;
    float  			m_driftZoom;
    TColor 			m_driftColor;
    bool   			m_drifted;
  };

#endif
