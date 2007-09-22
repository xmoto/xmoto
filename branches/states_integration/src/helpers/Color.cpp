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

#include "Color.h"

TColor::TColor(int i_red, int i_green, int i_blue, int i_alpha) {
  m_red   = i_red;
  m_green = i_green;
  m_blue  = i_blue;
  m_alpha = i_alpha;
}

TColor::~TColor() {
}

TColor::TColor(const TColor& i_color) {
  m_red   = i_color.m_red;
  m_green = i_color.m_green;
  m_blue  = i_color.m_blue;
  m_alpha = i_color.m_alpha;
}

int TColor::Red() const {
  return m_red;
}

int TColor::Green() const {
  return m_green;
}

int TColor::Blue() const {
  return m_blue;
}

int TColor::Alpha() const {
  return m_alpha;
}

void TColor::setRed(int i_red) {
  m_red = i_red;
}

void TColor::setGreen(int i_green) {
  m_green = i_green;
}

void TColor::setBlue(int i_blue) {
  m_blue = i_blue;
}

void TColor::setAlpha(int i_alpha) {
  m_alpha = i_alpha;
}
