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

#ifndef __POLYDRAW_H__
#define __POLYDRAW_H__

  class PolyDraw {
  public:
    struct EdgeValue {
      int x;			/* Interpolated x-value (screen) */
      int u;			/* Interpolated u-value (texture) */
      int v;			/* Interpolated v-value (texture) */
    };

    /* Construction/destruction */
     PolyDraw(SDL_Surface * pBuf = NULL);
    ~PolyDraw();

    /* Methods */
    void setRenderBuffer(SDL_Surface * pBuf);
    void render(int *pnScreenVerts, int *pnTexVerts, int nNumCorners,
		SDL_Surface * pTexture);

  private:
    /* Data */
     SDL_Surface * m_pBuf;
    EdgeValue *m_pLeftEdge, *m_pRightEdge;

    /* Helpers */
    void _ReleaseEdgeBuffers(void);
    void _LerpEdge(int y1, int y2, EdgeValue * pEdgeBuf,
		   int x1, int x2, int u1, int u2, int v1, int v2);

    void _RenderHLine(int y, EdgeValue * pLeft, EdgeValue * pRight,
		      SDL_Surface * pTexture, int nWSq, int nHSq,
		      int nPitchSq);
    int _TexSq(int n);
  };

#endif
