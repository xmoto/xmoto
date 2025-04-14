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

typedef unsigned int Color;

#define MAKE_COLOR(red, green, blue, alpha)                          \
  ((Color)((alpha) | (((Color)blue) << 8) | (((Color)green) << 16) | \
           (((Color)red) << 24)))

#define GET_ALPHA(color) ((Color)((color) & 0xff))
#define GET_BLUE(color) ((Color)(((color) & 0xff00) >> 8))
#define GET_GREEN(color) ((Color)(((color) & 0xff0000) >> 16))
#define GET_RED(color) ((Color)(((color) & 0xff000000) >> 24))

#define SET_ALPHA(color, alpha) \
  ((Color)(((color) & 0xffffff00) | ((alpha) & 0xff)))
#define SET_BLUE(color, blue) \
  ((Color)(((color) & 0xffff00ff) | ((blue) & 0xff) << 8))
#define SET_GREEN(color, green) \
  ((Color)(((color) & 0xff00ffff) | ((green) & 0xff) << 16))
#define SET_RED(color, red) \
  ((Color)(((color) & 0x00ffffff) | ((red) & 0xff) << 24))

#define INVERT_COLOR(color)          \
  MAKE_COLOR(255 - GET_RED(color),   \
             255 - GET_GREEN(color), \
             255 - GET_BLUE(color),  \
             GET_ALPHA(color))

class TColor {
public:
  inline TColor(int i_red = 255,
                int i_green = 255,
                int i_blue = 255,
                int i_alpha = 0) {
    m_color = MAKE_COLOR(i_red, i_green, i_blue, i_alpha);
  }

  inline int Red() const { return GET_RED(m_color); }
  inline int Green() const { return GET_GREEN(m_color); }
  inline int Blue() const { return GET_BLUE(m_color); }
  inline int Alpha() const { return GET_ALPHA(m_color); }

  inline void setRed(int i_red) { m_color = SET_RED(m_color, i_red); }
  inline void setGreen(int i_green) { m_color = SET_GREEN(m_color, i_green); }
  inline void setBlue(int i_blue) { m_color = SET_BLUE(m_color, i_blue); }
  inline void setAlpha(int i_alpha) { m_color = SET_ALPHA(m_color, i_alpha); }

  inline void setColor(Color c) { m_color = c; }
  inline Color getColor() const { return m_color; };

private:
  Color m_color;
};

#endif /* __COLOR_H__ */
