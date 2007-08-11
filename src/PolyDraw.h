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
