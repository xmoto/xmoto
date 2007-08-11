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

#ifndef __COLOR_H__
#define __COLOR_H__

class TColor {
  public:
  TColor(int i_red = 255, int i_green = 255, int i_blue = 255, int i_alpha = 0);
  TColor(const TColor& i_color);
  ~TColor();

  int Red()   const;
  int Green() const;
  int Blue()  const;
  int Alpha() const;
  void setRed(int i_red);
  void setGreen(int i_green);
  void setBlue(int i_blue);
  void setAlpha(int i_alpha);

  private:
  int m_red, m_green, m_blue, m_alpha;
};

#endif /* __COLOR_H__ */
