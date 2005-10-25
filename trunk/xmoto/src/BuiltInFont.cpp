/*=============================================================================
XMOTO
Copyright (C) 2005 Rasmus Neckelmann (neckelmann@gmail.com)

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
 *  The built-in font is a obscurely encoded 8x12 terminal-like type, useful
 *  for debugging and stuff like that. The code here simply decodes it. 
 */
#define DEFINEFONTDATA
#include "BuiltInFont.h"

/*=============================================================================
One might argue that this compression is totally useless... but I spent a 
couple of hours of complete boredom building a bit-level huffman/rle 
compression tree manually. Why? Because I was tired of coding. Enjoy ;)
=============================================================================*/

void CBuiltInFont::_DecodeData(void) {
  /* Obscure, uncommented code goes here... :D */
  unsigned char sm[]={1,2,102,202,3,4,108,5,6,7,8,9,106,10,101,103,203,
                      104,204,105,11,12,205,13,201,107,14,206,208,207};
  unsigned char *t = new unsigned char [32768];
  unsigned char *ps = g_cBuiltInFontData;
  int is = sizeof(g_cBuiltInFontData) - 1;
  int ib = ((is-1)<<3) + *(ps++);
  int j,i,k=0,r=0,s=0;  
  for(i=0;i<ib;i++) {
    int bb = i>>3, bi = i&7;    
    s = sm[ps[bb] & (1<<bi) ? 1 + (s<<1) : s<<1]; /* what does this statement do? hehe */    
    if(s > 200) {
      for(j=0;j<s-200;j++) t[r++] = 0xff;
      s = 0;
    }
    else if(s > 100) {
      for(j=0;j<s-100;j++) t[r++] = 0x00;
      s = 0;
    }
  }
  for(i=0;i<18;i++) t[r++] = 0x00; 
  m_pcData=t;
}
