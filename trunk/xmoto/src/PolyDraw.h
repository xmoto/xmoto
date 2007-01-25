#ifndef __POLYDRAW_H__
#define __POLYDRAW_H__

class PolyDraw {
  public:  
    /* Types */
    struct Buffer {
      int nWidth,nHeight;       /* Width and height of buffer */
      int nPitch;               /* Size of a horizontal line (in bytes) */
      char *pcData;             /* Pointer to pixel data */
      int nPixelSize;           /* Size of a pixel (in bytes) */
      int nPixelAlign;          /* Pixels are aligned to occupy this number of bytes */
    };
    
    struct EdgeValue {
      int x;                    /* Interpolated x-value (screen) */
      int u;                    /* Interpolated u-value (texture) */
      int v;                    /* Interpolated v-value (texture) */
    };
    
    /* Construction/destruction */
    PolyDraw(Buffer *pBuf = NULL);
    ~PolyDraw();    
  
    /* Methods */
    void setRenderBuffer(Buffer *pBuf);
    void render(int *pnScreenVerts,int *pnTexVerts,int nNumCorners,Buffer *pTexture);
                                               
  private:
    /* Data */
    Buffer *m_pBuf;
    EdgeValue *m_pLeftEdge,*m_pRightEdge;
    
    /* Helpers */
    void _ReleaseEdgeBuffers(void);
    void _LerpEdge(int y1,int y2,EdgeValue *pEdgeBuf,
                   int x1,int x2,int u1,int u2,int v1,int v2);
    
    void _RenderHLine(int y,EdgeValue *pLeft,EdgeValue *pRight,Buffer *pTexture,int nWSq,int nHSq,int nPitchSq,int nASq);
    void _RenderHLine16(int y,EdgeValue *pLeft,EdgeValue *pRight,Buffer *pTexture,int nWSq,int nHSq,int nPitchSq);          

    int _TexSq(int n);
};

#endif
